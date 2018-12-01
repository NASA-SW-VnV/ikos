/*******************************************************************************
 *
 * \file
 * \brief Interprocedural value analysis implementation
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2018 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Disclaimers:
 *
 * No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF
 * ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS,
 * ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE
 * ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO
 * THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN
 * ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS,
 * RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS
 * RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY
 * DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE,
 * IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."
 *
 * Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST
 * THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL
 * AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS
 * IN ANY LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH
 * USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM,
 * RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD
 * HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS,
 * AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.
 * RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE,
 * UNILATERAL TERMINATION OF THIS AGREEMENT.
 *
 ******************************************************************************/

#include <memory>
#include <vector>

#include <ikos/core/fixpoint/fwd_fixpoint_iterator.hpp>
#include <ikos/core/support/compiler.hpp>

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/type.hpp>
#include <ikos/ar/semantic/value.hpp>

#include <ikos/analyzer/analysis/execution_engine/engine.hpp>
#include <ikos/analyzer/analysis/execution_engine/inliner.hpp>
#include <ikos/analyzer/analysis/execution_engine/numerical.hpp>
#include <ikos/analyzer/analysis/fixpoint_profile.hpp>
#include <ikos/analyzer/analysis/pointer/pointer.hpp>
#include <ikos/analyzer/analysis/value/abstract_domain.hpp>
#include <ikos/analyzer/analysis/value/interprocedural.hpp>
#include <ikos/analyzer/analysis/value/machine_int_domain.hpp>
#include <ikos/analyzer/analysis/variable.hpp>
#include <ikos/analyzer/checker/checker.hpp>
#include <ikos/analyzer/support/cast.hpp>
#include <ikos/analyzer/util/demangle.hpp>
#include <ikos/analyzer/util/log.hpp>
#include <ikos/analyzer/util/timer.hpp>

namespace ikos {
namespace analyzer {

InterproceduralValueAnalysis::InterproceduralValueAnalysis(Context& ctx)
    : _ctx(ctx) {}

InterproceduralValueAnalysis::~InterproceduralValueAnalysis() = default;

namespace {

using namespace value;

/// \brief Return true if the given global variable should be initialized,
/// according to the given policy
bool is_initialized(ar::GlobalVariable* gv, GlobalsInitPolicy policy) {
  switch (policy) {
    case GlobalsInitPolicy::All: {
      // Initialize all global variables
      return true;
    }
    case GlobalsInitPolicy::SkipBigArrays: {
      // Initialize all global variables except arrays with more than 100
      // elements
      ar::Type* type = gv->type()->pointee();
      return !isa< ar::ArrayType >(type) ||
             cast< ar::ArrayType >(type)->num_elements() <= 100;
    }
    case GlobalsInitPolicy::SkipStrings: {
      // Initialize all global variables except strings ([n x si8]*)
      ar::Type* type = gv->type()->pointee();
      return !isa< ar::ArrayType >(type) ||
             cast< ar::ArrayType >(type)->element_type() !=
                 ar::IntegerType::si8(gv->context());
    }
    case GlobalsInitPolicy::None: {
      // Do not initialize any global variable
      return false;
    }
    default: { ikos_unreachable("unreachable"); }
  }
}

/// \brief Call execution engine for global variable initializer
class GlobalVarCallExecutionEngine final : public CallExecutionEngine {
public:
  ikos_attribute_unused void exec_exit(ar::Function* /*fun*/) override {}

  ikos_attribute_noreturn void exec(ar::Call* /*call*/) override {
    ikos_unreachable("call statement in global variable initializer");
  }

  ikos_attribute_noreturn void exec(ar::Invoke* /*invoke*/) override {
    ikos_unreachable("invoke statement in global variable initializer");
  }

  ikos_attribute_noreturn void exec(ar::ReturnValue* /*ret*/) override {
    ikos_unreachable("return statement in global variable initializer");
  }
};

/// \brief Return the list of pair (function, priority) for arrays
/// ar.global_ctors or ar.global_dtors, given the global variable
std::vector< std::pair< ar::Function*, MachineInt > > global_cdtors(
    ar::GlobalVariable* gv) {
  if (gv == nullptr || gv->is_declaration()) {
    return {};
  }

  ar::BasicBlock* bb = gv->initializer()->entry_block();

  if (bb->empty() || !isa< ar::Store >(bb->back())) {
    return {};
  }

  auto store = cast< ar::Store >(bb->back());

  if (store->pointer() != gv || !isa< ar::ArrayConstant >(store->value())) {
    return {};
  }

  auto cst = cast< ar::ArrayConstant >(store->value());
  std::vector< std::pair< ar::Function*, MachineInt > > entries;

  for (ar::Value* element : cst->values()) {
    if (!isa< ar::StructConstant >(element)) {
      continue;
    }

    auto e = cast< ar::StructConstant >(element);

    if (e->num_fields() != 3) {
      continue;
    }

    auto it = e->field_begin();
    ar::Value* fst = it->second;
    ++it;
    ar::Value* snd = it->second;

    if (!isa< ar::IntegerConstant >(fst) ||
        !isa< ar::FunctionPointerConstant >(snd)) {
      continue;
    }

    const MachineInt& priority = cast< ar::IntegerConstant >(fst)->value();
    ar::Function* fun = cast< ar::FunctionPointerConstant >(snd)->function();
    entries.emplace_back(fun, priority);
  }

  return entries;
}

/// \brief Return the global constructors, in call order
std::vector< std::pair< ar::Function*, MachineInt > > global_ctors(
    ar::GlobalVariable* gv) {
  auto entries = global_cdtors(gv);
  std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
    return a.second < b.second;
  });
  return entries;
}

/// \brief Return the global destructors, in call order
std::vector< std::pair< ar::Function*, MachineInt > > global_dtors(
    ar::GlobalVariable* gv) {
  auto entries = global_cdtors(gv);
  std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
    return a.second > b.second;
  });
  return entries;
}

/// \brief Fixpoint on a global variable initializer
class GlobalVarInitializerFixpoint final
    : public core::InterleavedFwdFixpointIterator< ar::Code*, AbstractDomain > {
private:
  /// \brief Parent class
  using FwdFixpointIterator =
      core::InterleavedFwdFixpointIterator< ar::Code*, AbstractDomain >;

  /// \brief Numerical execution engine
  using NumericalExecutionEngineT = NumericalExecutionEngine< AbstractDomain >;

private:
  /// \brief Global variable
  ar::GlobalVariable* _gv;

  /// \brief Analysis context
  Context& _ctx;

  /// \brief Empty call context
  CallContext* _empty_call_context;

public:
  /// \brief Constructor
  GlobalVarInitializerFixpoint(Context& ctx, ar::GlobalVariable* gv)
      : FwdFixpointIterator(gv->initializer()),
        _gv(gv),
        _ctx(ctx),
        _empty_call_context(ctx.call_context_factory->get_empty()) {}

  /// \brief Propagate the invariant through the basic block
  AbstractDomain analyze_node(ar::BasicBlock* bb, AbstractDomain pre) override {
    NumericalExecutionEngineT exec_engine(std::move(pre),
                                          _ctx,
                                          this->_empty_call_context,
                                          /* precision = */ _ctx.opts.precision,
                                          /* liveness = */ _ctx.liveness,
                                          /* pointer_info = */ _ctx.pointer ==
                                                  nullptr
                                              ? nullptr
                                              : &_ctx.pointer->results());
    GlobalVarCallExecutionEngine call_exec_engine;
    exec_engine.exec_enter(bb);
    for (ar::Statement* stmt : *bb) {
      transfer_function(exec_engine, call_exec_engine, stmt);
    }
    exec_engine.exec_leave(bb);
    return std::move(exec_engine.inv());
  }

  /// \brief Propagate the invariant through an edge
  AbstractDomain analyze_edge(ar::BasicBlock* src,
                              ar::BasicBlock* dest,
                              AbstractDomain pre) override {
    NumericalExecutionEngineT exec_engine(std::move(pre),
                                          _ctx,
                                          this->_empty_call_context,
                                          /* precision = */ _ctx.opts.precision,
                                          /* liveness = */ _ctx.liveness,
                                          /* pointer_info = */ _ctx.pointer ==
                                                  nullptr
                                              ? nullptr
                                              : &_ctx.pointer->results());
    exec_engine.exec_edge(src, dest);
    return std::move(exec_engine.inv());
  }

  /// \brief Process the computed abstract value for a node
  void process_pre(ar::BasicBlock* /*bb*/,
                   const AbstractDomain& /*pre*/) override {}

  /// \brief Process the computed abstract value for a node
  void process_post(ar::BasicBlock* /*bb*/,
                    const AbstractDomain& /*post*/) override {}

  /// \brief Compute the fixpoint
  void run(AbstractDomain inv) {
    // Allocate memory for the global variable
    NumericalExecutionEngineT exec_engine(std::move(inv),
                                          _ctx,
                                          this->_empty_call_context,
                                          /* precision = */ _ctx.opts.precision,
                                          /* liveness = */ _ctx.liveness,
                                          /* pointer_info = */ _ctx.pointer ==
                                                  nullptr
                                              ? nullptr
                                              : &_ctx.pointer->results());
    exec_engine
        .allocate_memory(_ctx.var_factory->get_global(_gv),
                         _ctx.mem_factory->get_global(_gv),
                         core::Nullity::non_null(),
                         core::Uninitialized::initialized(),
                         core::Lifetime::top(),
                         NumericalExecutionEngineT::MemoryInitialValue::Zero);

    // Compute the fixpoint
    FwdFixpointIterator::run(std::move(exec_engine.inv()));
  }

  /// \brief Return the invariant at the end of the exit node
  const AbstractDomain& exit_invariant() const {
    ar::Code* code = this->cfg();
    ikos_assert_msg(code->has_exit_block(), "initializer without exit block");
    return this->post(code->exit_block());
  }

}; // end class GlobalVarInitializerFixpoint

/// \brief Fixpoint on a function body
class FunctionFixpoint final
    : public core::InterleavedFwdFixpointIterator< ar::Code*, AbstractDomain > {
private:
  /// \brief Parent class
  using FwdFixpointIterator =
      core::InterleavedFwdFixpointIterator< ar::Code*, AbstractDomain >;

  /// \brief Numerical execution engine
  using NumericalExecutionEngineT = NumericalExecutionEngine< AbstractDomain >;

  /// \brief Inliner
  using InlineCallExecutionEngineT =
      InlineCallExecutionEngine< FunctionFixpoint, AbstractDomain >;

private:
  /// \brief Analyzed function
  ar::Function* _function;

  /// \brief Current call context
  CallContext* _call_context;

  /// \brief Machine integer abstract domain
  MachineIntDomainOption _machine_int_domain;

  /// \brief Fixpoint profile
  boost::optional< const FixpointProfile& > _profile;

  /// \brief Function currently analyzed
  ///
  /// This is used to avoid cycles.
  std::vector< ar::Function* > _analyzed_functions;

  /// \brief List of property checks to run
  const std::vector< std::unique_ptr< Checker > >& _checkers;

  /// \brief Numerical execution engine
  NumericalExecutionEngineT _exec_engine;

  /// \brief Call execution engine
  InlineCallExecutionEngineT _call_exec_engine;

public:
  /// \brief Constructor for an entry point
  ///
  /// \param ctx Analysis context
  /// \param checkers List of checkers to run
  /// \param entry_point Function to analyze
  FunctionFixpoint(Context& ctx,
                   const std::vector< std::unique_ptr< Checker > >& checkers,
                   ar::Function* entry_point)
      : FwdFixpointIterator(entry_point->body()),
        _function(entry_point),
        _call_context(ctx.call_context_factory->get_empty()),
        _machine_int_domain(ctx.opts.machine_int_domain),
        _profile(ctx.fixpoint_profiler == nullptr
                     ? boost::none
                     : ctx.fixpoint_profiler->profile(entry_point)),
        _analyzed_functions{entry_point},
        _checkers(checkers),
        _exec_engine(AbstractDomain::bottom(),
                     ctx,
                     this->_call_context,
                     /* precision = */ ctx.opts.precision,
                     /* liveness = */ ctx.liveness,
                     /* pointer_info = */ ctx.pointer == nullptr
                         ? nullptr
                         : &ctx.pointer->results()),
        _call_exec_engine(ctx,
                          _exec_engine,
                          *this,
                          /* context_stable = */ true,
                          /* convergence_achieved = */ false) {}

  /// \brief Constructor for a callee
  ///
  /// \param ctx Analysis context
  /// \param caller Parent function fixpoint
  /// \param call Call statement
  /// \param callee Called function
  /// \param context_stable Is the calling context stable (fixpoint reached)?
  FunctionFixpoint(Context& ctx,
                   const FunctionFixpoint& caller,
                   ar::CallBase* call,
                   ar::Function* callee,
                   bool context_stable)
      : FwdFixpointIterator(callee->body()),
        _function(callee),
        _call_context(
            ctx.call_context_factory->get_context(caller._call_context, call)),
        _machine_int_domain(ctx.opts.machine_int_domain),
        _profile(ctx.fixpoint_profiler == nullptr
                     ? boost::none
                     : ctx.fixpoint_profiler->profile(callee)),
        _analyzed_functions(caller._analyzed_functions),
        _checkers(caller._checkers),
        _exec_engine(AbstractDomain::bottom(),
                     ctx,
                     this->_call_context,
                     /* precision = */ ctx.opts.precision,
                     /* liveness = */ ctx.liveness,
                     /* pointer_info = */ ctx.pointer == nullptr
                         ? nullptr
                         : &ctx.pointer->results()),
        _call_exec_engine(ctx,
                          _exec_engine,
                          *this,
                          /* context_stable = */ context_stable,
                          /* convergence_achieved = */ false) {
    this->_analyzed_functions.push_back(callee);
  }

  /// \brief Compute the fixpoint
  void run(AbstractDomain inv) {
    FwdFixpointIterator::run(std::move(inv));
    this->_call_exec_engine.mark_convergence_achieved();
  }

  /// \brief Extrapolate the new state after an increasing iteration
  AbstractDomain extrapolate(ar::BasicBlock* head,
                             unsigned iteration,
                             AbstractDomain before,
                             AbstractDomain after) override {
    if (iteration <= 1) {
      before.join_iter_with(after);
      return before;
    }
    if (iteration == 2 && this->_profile) {
      if (auto threshold = this->_profile->widening_hint(head)) {
        before.widen_threshold_with(after, *threshold);
        return before;
      }
    }
    before.widen_with(after);
    return before;
  }

  /// \brief Check if the decreasing iterations fixpoint is reached
  bool is_decreasing_iterations_fixpoint(const AbstractDomain& before,
                                         const AbstractDomain& after) override {
    if (machine_int_domain_option_has_narrowing(this->_machine_int_domain)) {
      return before.leq(after);
    } else {
      return true; // stop after the first decreasing iteration
    }
  }

  /// \brief Propagate the invariant through the basic block
  AbstractDomain analyze_node(ar::BasicBlock* bb, AbstractDomain pre) override {
    this->_exec_engine.set_inv(std::move(pre));
    this->_exec_engine.exec_enter(bb);
    for (ar::Statement* stmt : *bb) {
      transfer_function(this->_exec_engine, this->_call_exec_engine, stmt);
    }
    this->_exec_engine.exec_leave(bb);
    return std::move(this->_exec_engine.inv());
  }

  /// \brief Propagate the invariant through an edge
  AbstractDomain analyze_edge(ar::BasicBlock* src,
                              ar::BasicBlock* dest,
                              AbstractDomain pre) override {
    this->_exec_engine.set_inv(std::move(pre));
    this->_exec_engine.exec_edge(src, dest);
    return std::move(this->_exec_engine.inv());
  }

  /// \brief Process the computed abstract value for a node
  void process_pre(ar::BasicBlock* /*bb*/,
                   const AbstractDomain& /*pre*/) override {}

  /// \brief Process the computed abstract value for a node
  void process_post(ar::BasicBlock* bb, const AbstractDomain& post) override {
    if (this->_function->body()->exit_block_or_null() == bb) {
      this->_exec_engine.set_inv(post);
      this->_call_exec_engine.exec_exit(this->_function);
    }
  }

  /// \brief Run the checks with the previously computed fix-point
  void run_checks() {
    for (const auto& checker : this->_checkers) {
      checker->enter(this->_function, this->_call_context);
    }

    // Check the function body
    for (ar::BasicBlock* bb : *this->cfg()) {
      this->_exec_engine.set_inv(this->pre(bb));
      this->_exec_engine.exec_enter(bb);
      for (const auto& checker : this->_checkers) {
        checker->enter(bb, this->_exec_engine.inv(), this->_call_context);
      }

      for (ar::Statement* stmt : *bb) {
        // Check the statement if it's related to an llvm instruction
        if (stmt->has_frontend()) {
          for (const auto& checker : this->_checkers) {
            checker->check(stmt, this->_exec_engine.inv(), this->_call_context);
          }
        }

        // Propagate
        transfer_function(this->_exec_engine, this->_call_exec_engine, stmt);
      }

      for (const auto& checker : this->_checkers) {
        checker->leave(bb, this->_exec_engine.inv(), this->_call_context);
      }
      this->_exec_engine.exec_leave(bb);
    }

    for (const auto& checker : this->_checkers) {
      checker->leave(this->_function, this->_call_context);
    }

    // Clear the invariants
    this->clear();

    // Run the checks on the callees
    this->_call_exec_engine.run_checks();

    // Clear the list of callees
    this->_call_exec_engine.clear();
  }

  /// \name Helpers for InlineCallExecutionEngine
  /// @{

  /// \brief Mark that the calling context is stable
  void mark_context_stable() { this->_call_exec_engine.mark_context_stable(); }

  /// \brief Return the inline call execution engine
  const InlineCallExecutionEngineT& inliner() const {
    return this->_call_exec_engine;
  }

  /// \brief Return true if the given function is currently analyzed
  bool is_currently_analyzed(ar::Function* fun) const {
    return std::find(this->_analyzed_functions.begin(),
                     this->_analyzed_functions.end(),
                     fun) != this->_analyzed_functions.end();
  }

  /// @}

  /// \brief Return the exit invariant, or bottom
  const AbstractDomain& exit_invariant() const {
    return this->_call_exec_engine.exit_invariant();
  }

}; // end class FunctionFixpoint

/// \brief Return the initial invariant
AbstractDomain init_invariant(MachineIntDomainOption machine_int_domain) {
  return AbstractDomain(
      /*normal=*/
      MemoryAbstractDomain(PointerAbstractDomain(make_top_machine_int_domain(
                                                     machine_int_domain),
                                                 NullityAbstractDomain::top()),
                           UninitializedAbstractDomain::top(),
                           LifetimeAbstractDomain::top()),
      /*caught_exceptions=*/MemoryAbstractDomain::bottom(),
      /*propagated_exceptions=*/MemoryAbstractDomain::bottom());
}

/// \brief Initialize argc and argv
AbstractDomain init_main_invariant(Context& ctx,
                                   ar::Function* main,
                                   AbstractDomain inv) {
  auto argc = ctx.lit_factory->get_scalar(main->param(0));
  auto argv = ctx.lit_factory->get_scalar(main->param(1));

  if (!argc.is_machine_int_var()) {
    log::warning("Unexpected type for first argument of main");
    return inv;
  }
  if (!argv.is_pointer_var()) {
    log::warning("Unexpected type for second argument of main");
    return inv;
  }

  // Set argc
  auto argc_type = cast< ar::IntegerType >(main->param(0)->type());
  if (ctx.opts.argc) {
    // Add `argc = ctx.opts.argc`
    inv.normal().integers().assign(argc.var(),
                                   MachineInt(*ctx.opts.argc,
                                              argc_type->bit_width(),
                                              argc_type->sign()));
  } else {
    // Add `argc >= 0`
    inv.normal().integers().add(core::machine_int::Predicate::GE,
                                argc.var(),
                                MachineInt::zero(argc_type->bit_width(),
                                                 argc_type->sign()));
  }
  inv.normal().uninitialized().set(argc.var(),
                                   core::Uninitialized::initialized());

  // Set argv
  ArgvMemoryLocation* argv_mem_loc = ctx.mem_factory->get_argv();
  inv.normal().pointers().assign_address(argv.var(),
                                         argv_mem_loc,
                                         core::Nullity::non_null());
  inv.normal().uninitialized().set(argv.var(),
                                   core::Uninitialized::initialized());

  if (ctx.opts.argc) {
    // Add size of argv array
    const auto& dl = ctx.bundle->data_layout();
    uint64_t pointer_size = dl.pointers.bit_width / 8;
    uint64_t argv_size =
        pointer_size * (static_cast< uint64_t >(*ctx.opts.argc) + 1u);
    Variable* alloc_size_var = ctx.var_factory->get_alloc_size(argv_mem_loc);
    inv.normal().integers().assign(alloc_size_var,
                                   MachineInt(argv_size,
                                              dl.pointers.bit_width,
                                              Unsigned));
  }

  return inv;
}

} // end anonymous namespace

void InterproceduralValueAnalysis::run() {
  // Bundle
  ar::Bundle* bundle = _ctx.bundle;

  // Create checkers
  std::vector< std::unique_ptr< Checker > > checkers;
  for (CheckerName name : _ctx.opts.analyses) {
    checkers.emplace_back(make_checker(_ctx, name));
  }

  // Initial invariant
  value::AbstractDomain init_inv = init_invariant(_ctx.opts.machine_int_domain);

  // Initialize global variables
  log::debug("Computing global variable static initialization");
  for (auto it = bundle->global_begin(), et = bundle->global_end(); it != et;
       ++it) {
    ar::GlobalVariable* gv = *it;
    if (gv->is_definition() &&
        is_initialized(gv, _ctx.opts.globals_init_policy)) {
      log::debug("Initializing global variable @" + gv->name());
      GlobalVarInitializerFixpoint fixpoint(_ctx, gv);
      fixpoint.run(init_inv);
      init_inv = fixpoint.exit_invariant();
    }
  }

  if (_ctx.opts.display_invariants == DisplayOption::All) {
    log::out() << "Invariant after global variable static initialization:\n";
    init_inv.dump(log::out());
    log::out() << std::endl;
  }

  // Call constructors
  ar::GlobalVariable* gv_ctors = bundle->global_or_null("ar.global_ctors");
  if (gv_ctors != nullptr) {
    log::info("Computing global variable dynamic initialization");

    std::vector< std::pair< ar::Function*, MachineInt > > ctors =
        global_ctors(gv_ctors);
    for (const auto& entry : ctors) {
      ar::Function* ctor = entry.first;

      if (ctor->is_declaration()) {
        log::error("Global constructor " + ctor->name() + " is extern");
        continue;
      }

      FunctionFixpoint fixpoint(_ctx, checkers, ctor);

      {
        log::info("Analyzing global constructor: " + demangle(ctor->name()));
        ScopeTimerDatabase t(_ctx.output_db->times,
                             "ikos-analyzer.value." + ctor->name());
        fixpoint.run(init_inv);
      }

      {
        log::info(
            "Checking properties and writing results for global constructor: " +
            demangle(ctor->name()));
        ScopeTimerDatabase t(_ctx.output_db->times,
                             "ikos-analyzer.check." + ctor->name());
        fixpoint.run_checks();
      }

      init_inv = fixpoint.exit_invariant();
    }

    if (_ctx.opts.display_invariants == DisplayOption::All) {
      log::out() << "Invariant after global variable dynamic initialization:\n";
      init_inv.dump(log::out());
      log::out() << std::endl;
    }
  }

  // Analyze each entry point
  for (ar::Function* entry_point : _ctx.opts.entry_points) {
    if (entry_point->is_declaration()) {
      log::error("Entry point " + entry_point->name() + " is extern");
      continue;
    }

    // Entry point initial invariant
    value::AbstractDomain entry_inv = value::AbstractDomain::bottom();

    if (std::find(_ctx.opts.no_init_globals.begin(),
                  _ctx.opts.no_init_globals.end(),
                  entry_point) == _ctx.opts.no_init_globals.end()) {
      // Use invariant with initialized global variables
      entry_inv = init_inv;
    } else {
      // Default invariant
      entry_inv = init_invariant(_ctx.opts.machine_int_domain);
    }

    if (entry_point->name() == "main" && entry_point->num_parameters() >= 2) {
      entry_inv = init_main_invariant(_ctx, entry_point, entry_inv);
    }

    FunctionFixpoint fixpoint(_ctx, checkers, entry_point);

    {
      log::info("Analyzing entry point: " + demangle(entry_point->name()));
      ScopeTimerDatabase t(_ctx.output_db->times,
                           "ikos-analyzer.value." + entry_point->name());
      fixpoint.run(entry_inv);
    }

    {
      log::info("Checking properties and writing results for entry point: " +
                demangle(entry_point->name()));
      ScopeTimerDatabase t(_ctx.output_db->times,
                           "ikos-analyzer.check." + entry_point->name());
      fixpoint.run_checks();
    }
  }

  // Call destructors
  ar::GlobalVariable* gv_dtors = bundle->global_or_null("ar.global_dtors");
  if (gv_dtors != nullptr) {
    log::info("Analyzing global destructors");

    std::vector< std::pair< ar::Function*, MachineInt > > dtors =
        global_dtors(gv_dtors);
    for (const auto& entry : dtors) {
      ar::Function* dtor = entry.first;

      if (dtor->is_declaration()) {
        log::error("Global destructor " + dtor->name() + " is extern");
        continue;
      }

      FunctionFixpoint fixpoint(_ctx, checkers, dtor);

      {
        log::info("Analyzing global destructor: " + demangle(dtor->name()));
        ScopeTimerDatabase t(_ctx.output_db->times,
                             "ikos-analyzer.value." + dtor->name());
        // Note: We currently analyze destructors with the initial invariant
        fixpoint.run(init_inv);
      }

      {
        log::info(
            "Checking properties and writing results for global destructor: " +
            demangle(dtor->name()));
        ScopeTimerDatabase t(_ctx.output_db->times,
                             "ikos-analyzer.check." + dtor->name());
        fixpoint.run_checks();
      }

      init_inv = fixpoint.exit_invariant();
    }
  }

  // Insert all functions in the database
  for (auto it = bundle->function_begin(), et = bundle->function_end();
       it != et;
       ++it) {
    _ctx.output_db->functions.insert(*it);
  }
}

} // end namespace analyzer
} // end namespace ikos
