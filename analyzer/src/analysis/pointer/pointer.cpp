/******************************************************************************
 *
 * \file
 * \brief Implementation of the pointer analysis
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *               Clement Decoodt
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2019 United States Government as represented by the
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

#include <ikos/core/domain/exception/exception.hpp>
#include <ikos/core/domain/lifetime/dummy.hpp>
#include <ikos/core/domain/machine_int/interval.hpp>
#include <ikos/core/domain/memory/dummy.hpp>
#include <ikos/core/domain/nullity/dummy.hpp>
#include <ikos/core/domain/pointer/dummy.hpp>
#include <ikos/core/domain/uninitialized/dummy.hpp>
#include <ikos/core/fixpoint/fwd_fixpoint_iterator.hpp>

#include <ikos/analyzer/analysis/execution_engine/context_insensitive.hpp>
#include <ikos/analyzer/analysis/execution_engine/engine.hpp>
#include <ikos/analyzer/analysis/execution_engine/numerical.hpp>
#include <ikos/analyzer/analysis/fixpoint_profile.hpp>
#include <ikos/analyzer/analysis/pointer/constraint.hpp>
#include <ikos/analyzer/analysis/pointer/function.hpp>
#include <ikos/analyzer/analysis/pointer/pointer.hpp>
#include <ikos/analyzer/analysis/pointer/value.hpp>
#include <ikos/analyzer/util/demangle.hpp>
#include <ikos/analyzer/util/log.hpp>
#include <ikos/analyzer/util/progress.hpp>

namespace ikos {
namespace analyzer {

PointerAnalysis::PointerAnalysis(
    Context& ctx, const FunctionPointerAnalysis& function_pointer)
    : _ctx(ctx),
      _function_pointer(function_pointer),
      _info(ctx.bundle->data_layout()) {}

PointerAnalysis::~PointerAnalysis() = default;

namespace {

/// \brief Numerical abstract domain for the intra-procedural pointer analysis
using MachineIntAbstractDomain = core::machine_int::IntervalDomain< Variable* >;

/// \brief Pointer abstract domain for the intra-procedural pointer analysis
using PointerAbstractDomain =
    core::pointer::DummyDomain< Variable*,
                                MemoryLocation*,
                                MachineIntAbstractDomain,
                                core::nullity::DummyDomain< Variable* > >;

/// \brief Memory abstract domain for the intra-procedural pointer analysis
using MemoryAbstractDomain =
    core::memory::DummyDomain< Variable*,
                               MemoryLocation*,
                               VariableFactory,
                               MachineIntAbstractDomain,
                               core::nullity::DummyDomain< Variable* >,
                               PointerAbstractDomain,
                               core::uninitialized::DummyDomain< Variable* >,
                               core::lifetime::DummyDomain< MemoryLocation* > >;

/// \brief Abstract domain for the intra-procedural pointer analysis
using AbstractDomain = core::exception::ExceptionDomain< MemoryAbstractDomain >;

/// \brief Numerical invariants on an ar::Code
class NumericalCodeInvariants final
    : public core::InterleavedFwdFixpointIterator< ar::Code*, AbstractDomain > {
public:
  using AbstractDomainT = AbstractDomain;

private:
  /// \brief Parent class
  using FwdFixpointIterator =
      core::InterleavedFwdFixpointIterator< ar::Code*, AbstractDomainT >;

private:
  /// \brief Analysis context
  Context& _ctx;

  /// \brief Empty call context
  CallContext* _empty_call_context;

  /// \brief Previously computed function pointer analysis
  const FunctionPointerAnalysis& _function_pointer;

  /// \brief Fixpoint profile
  boost::optional< const FixpointProfile& > _profile;

public:
  /// \brief Constructor
  NumericalCodeInvariants(Context& ctx,
                          const FunctionPointerAnalysis& function_pointer,
                          ar::Code* code)
      : FwdFixpointIterator(code),
        _ctx(ctx),
        _empty_call_context(ctx.call_context_factory->get_empty()),
        _function_pointer(function_pointer),
        _profile(ctx.fixpoint_profiler != nullptr && code->is_function_body()
                     ? ctx.fixpoint_profiler->profile(code->function())
                     : boost::none) {}

  /// \brief Compute an intra-procedural fixpoint on the given code
  void run() { FwdFixpointIterator::run(AbstractDomainT::top_no_exceptions()); }

  /// \brief Extrapolate the new state after an increasing iteration
  AbstractDomainT extrapolate(ar::BasicBlock* head,
                              unsigned iteration,
                              AbstractDomainT before,
                              AbstractDomainT after) override {
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

  /// \brief Propagate the invariant through the basic block
  AbstractDomainT analyze_node(ar::BasicBlock* bb,
                               AbstractDomainT pre) override {
    NumericalExecutionEngine< AbstractDomainT >
        exec_engine(std::move(pre),
                    _ctx,
                    this->_empty_call_context,
                    /* precision = */ Precision::Register,
                    /* liveness = */ _ctx.liveness,
                    /* pointer_info = */ &_function_pointer.results());
    ContextInsensitiveCallExecutionEngine< AbstractDomainT > call_exec_engine(
        exec_engine);
    exec_engine.exec_enter(bb);
    for (ar::Statement* stmt : *bb) {
      transfer_function(exec_engine, call_exec_engine, stmt);
    }
    exec_engine.exec_leave(bb);
    return std::move(exec_engine.inv());
  }

  /// \brief Propagate the invariant through an edge
  AbstractDomainT analyze_edge(ar::BasicBlock* src,
                               ar::BasicBlock* dest,
                               AbstractDomainT pre) override {
    NumericalExecutionEngine< AbstractDomainT >
        exec_engine(std::move(pre),
                    _ctx,
                    this->_empty_call_context,
                    /* precision = */ Precision::Register,
                    /* liveness = */ _ctx.liveness,
                    /* pointer_info = */ &_function_pointer.results());
    exec_engine.exec_edge(src, dest);
    return std::move(exec_engine.inv());
  }

  /// \brief Process the computed abstract value for a node
  void process_pre(ar::BasicBlock* /*bb*/,
                   const AbstractDomainT& /*pre*/) override {}

  /// \brief Process the computed abstract value for a node
  void process_post(ar::BasicBlock* /*bb*/,
                    const AbstractDomainT& /*post*/) override {}

  /// \brief Get the invariant at the entry of the given basic block
  AbstractDomainT entry(ar::BasicBlock* bb) const {
    NumericalExecutionEngine< AbstractDomainT >
        exec_engine(this->pre(bb),
                    _ctx,
                    this->_empty_call_context,
                    /* precision = */ Precision::Register,
                    /* liveness = */ _ctx.liveness,
                    /* pointer_info = */ &_function_pointer.results());
    exec_engine.exec_enter(bb);
    return std::move(exec_engine.inv());
  }

  /// \brief Execute the given statement with the given invariant
  AbstractDomainT analyze_statement(ar::Statement* stmt,
                                    AbstractDomainT pre) const {
    NumericalExecutionEngine< AbstractDomainT >
        exec_engine(std::move(pre),
                    _ctx,
                    this->_empty_call_context,
                    /* precision = */ Precision::Register,
                    /* liveness = */ _ctx.liveness,
                    /* pointer_info = */ &_function_pointer.results());
    ContextInsensitiveCallExecutionEngine< AbstractDomainT > call_exec_engine(
        exec_engine);
    transfer_function(exec_engine, call_exec_engine, stmt);
    return std::move(exec_engine.inv());
  }
};

} // end anonymous namespace

void PointerAnalysis::run() {
  ar::Bundle* bundle = _ctx.bundle;

  // Setup a progress logger
  std::unique_ptr< ProgressLogger > progress =
      make_progress_logger(_ctx.opts.progress,
                           LogLevel::Info,
                           /* num_tasks = */
                           2 * std::count_if(bundle->global_begin(),
                                             bundle->global_end(),
                                             [](ar::GlobalVariable* gv) {
                                               return gv->is_definition();
                                             }) +
                               2 * std::count_if(bundle->function_begin(),
                                                 bundle->function_end(),
                                                 [](ar::Function* fun) {
                                                   return fun->is_definition();
                                                 }) +
                               1);
  ScopeLogger scope(*progress);

  log::debug("Generating pointer constraints");
  PointerConstraints constraints(bundle->data_layout());

  PointerConstraintsGenerator< NumericalCodeInvariants >
      visitor(_ctx, constraints, &_function_pointer.results());

  for (auto it = bundle->global_begin(), et = bundle->global_end(); it != et;
       ++it) {
    ar::GlobalVariable* gv = *it;
    if (gv->is_definition()) {
      progress->start_task(
          "Generating intra-procedural numerical invariant for initializer of "
          "global variable '" +
          demangle(gv->name()) + "'");
      NumericalCodeInvariants invariants(_ctx,
                                         _function_pointer,
                                         gv->initializer());
      invariants.run();

      progress->start_task(
          "Generating pointer constraints for initializer of global variable "
          "'" +
          demangle(gv->name()) + "'");
      visitor.process_global_var_def(gv, invariants);
    } else {
      visitor.process_global_var_decl(gv);
    }
  }

  for (auto it = bundle->function_begin(), et = bundle->function_end();
       it != et;
       ++it) {
    ar::Function* fun = *it;
    if (fun->is_definition()) {
      progress->start_task(
          "Generating intra-procedural numerical invariant for function '" +
          demangle(fun->name()) + "'");
      NumericalCodeInvariants invariants(_ctx, _function_pointer, fun->body());
      invariants.run();

      progress->start_task("Generating pointer constraints for function '" +
                           demangle(fun->name()) + "'");
      visitor.process_function_def(fun, invariants);
    } else {
      visitor.process_function_decl(fun);
    }
  }

  log::debug("Solving pointer constraints");
  progress->start_task("Solving pointer constraints");
  constraints.solve();

  // Save information
  constraints.results(this->_info);
}

/// \brief Dump the pointer analysis results, for debugging purpose
void PointerAnalysis::dump(std::ostream& o) const {
  o << "Pointer analysis results:\n";
  this->_info.dump(o);
}

} // end namespace analyzer
} // end namespace ikos
