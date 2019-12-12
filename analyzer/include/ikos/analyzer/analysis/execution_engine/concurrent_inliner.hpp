/*******************************************************************************
 *
 * \file
 * \brief Concurrent dynamic inlining call semantic
 *
 * Author: Sung Kook Kim
 *
 * Contributor: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2019 United States Government as represented by the
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

#pragma once

#include <memory>
#include <vector>

#include <tbb/blocked_range.h>
#include <tbb/parallel_reduce.h>

#include <llvm/ADT/Optional.h>

#include <ikos/ar/semantic/function.hpp>
#include <ikos/ar/semantic/statement.hpp>
#include <ikos/ar/verify/type.hpp>

#include <ikos/analyzer/analysis/execution_engine/engine.hpp>
#include <ikos/analyzer/analysis/execution_engine/fixpoint_cache.hpp>
#include <ikos/analyzer/analysis/execution_engine/numerical.hpp>
#include <ikos/analyzer/analysis/pointer/value.hpp>

namespace ikos {
namespace analyzer {

/// \brief Concurrent inliner of function calls
///
/// The inlining of a function is done dynamically by matching formal and actual
/// parameters and analyzing recursively the callee (FunctionFixpoint) and after
/// the callee returns by simulating call-by-ref and updating the return value
/// at the call site. The inlining also supports function pointers by resolving
/// first the set of possible callees and joining the results.
template < typename FunctionFixpoint, typename AbstractDomain >
class ConcurrentInlineCallExecutionEngine final : public CallExecutionEngine {
public:
  using InlineCallExecutionEngineT =
      ConcurrentInlineCallExecutionEngine< FunctionFixpoint, AbstractDomain >;
  using NumericalExecutionEngineT = NumericalExecutionEngine< AbstractDomain >;
  using FixpointCacheT = FixpointCache< FunctionFixpoint, AbstractDomain >;

private:
  /// \brief Analysis context
  Context& _ctx;

  /// \brief Numerical execution engine
  NumericalExecutionEngineT& _engine;

  /// \brief Function analyzer of the caller
  FunctionFixpoint& _caller;

  /// \brief Function fixpoint cache of callees
  FixpointCacheT& _callees_cache;

  /// \brief True to check properties on the callees
  bool _check_callees;

public:
  /// \brief Constructor
  ConcurrentInlineCallExecutionEngine(Context& ctx,
                                      NumericalExecutionEngineT& engine,
                                      FunctionFixpoint& caller,
                                      FixpointCacheT& callees_cache)
      : _ctx(ctx),
        _engine(engine),
        _caller(caller),
        _callees_cache(callees_cache),
        _check_callees(false) {}

  /// \brief Mark to check the callees
  void mark_check_callees() { this->_check_callees = true; }

  /// \brief Exit a function
  ///
  /// This is called whenever we reach the exit node (if there is one).
  ///
  /// Note that this is different from exec(ar::Returnvalue*) if there is
  /// exceptions. This can be used to catch the invariant, including pending
  /// exceptions.
  void exec_exit(ar::Function* fun) override {
    this->_engine.deallocate_local_variables(fun->local_variable_begin(),
                                             fun->local_variable_end());
    this->_caller.set_exit_invariant(this->_engine.inv());
  }

  /// \brief Execute a ReturnValue statement
  void exec(ar::ReturnValue* s) override { this->_caller.set_return_stmt(s); }

  /// \brief Execute a Call statement
  void exec(ar::Call* s) override {
    // Execute the call base statement
    this->exec(cast< ar::CallBase >(s));

    // Exceptions aren't caught, propagate them
    this->inv().merge_caught_in_propagated_exceptions();
  }

  /// \brief Execute an Invoke statement
  void exec(ar::Invoke* s) override {
    // Execute the call base statement
    this->exec(cast< ar::CallBase >(s));

    // Exceptions are caught.
    // Nothing to do here.
    // see NumericalExecutionEngine::exec_edge()
  }

private:
  /// \brief Return a non-const reference on the current invariant
  AbstractDomain& inv() { return this->_engine.inv(); }

  /// \brief Analysis on a callee
  class CalleeAnalysis {
  public:
    ar::Function* callee;
    std::unique_ptr< FunctionFixpoint > fixpoint;

  public:
    /// \brief Constructor
    explicit CalleeAnalysis(ar::Function* callee_)
        : callee(callee_), fixpoint(nullptr) {}

    /// \brief No copy constructor
    CalleeAnalysis(const CalleeAnalysis&) = delete;

    /// \brief Move constructor
    CalleeAnalysis(CalleeAnalysis&&) = default;

    /// \brief No copy assignment operator
    CalleeAnalysis& operator=(const CalleeAnalysis&) = delete;

    /// \brief No move assignment operator
    CalleeAnalysis& operator=(CalleeAnalysis&&) = delete;

    /// \brief Destructor
    ~CalleeAnalysis() = default;

  }; // end class CalleeAnalysis

  /// \brief Worker on a callee for tbb::parallel_reduce
  class CalleeWorker {
  private:
    /// \brief Analysis context
    Context& _ctx;

    /// \brief Caller numerical execution engine
    const NumericalExecutionEngineT& _engine;

    /// \brief Function analyzer of the caller
    FunctionFixpoint& _caller;

    /// \brief Function fixpoint cache of callees
    FixpointCacheT& _callees_cache;

    /// \brief Call statement
    ar::CallBase* _call;

    /// \brief Analyses on callees
    std::vector< CalleeAnalysis >& _callee_analyses;

    /// \brief Post invariant
    llvm::Optional< AbstractDomain > _post;

  public:
    /// \brief Constructor
    CalleeWorker(Context& ctx,
                 const NumericalExecutionEngineT& engine,
                 FunctionFixpoint& caller,
                 FixpointCacheT& callees_cache,
                 ar::CallBase* call,
                 std::vector< CalleeAnalysis >& callee_analyses,
                 AbstractDomain post)
        : _ctx(ctx),
          _engine(engine),
          _caller(caller),
          _callees_cache(callees_cache),
          _call(call),
          _callee_analyses(callee_analyses),
          _post(std::move(post)) {}

    /// \brief Split constructor
    CalleeWorker(CalleeWorker& parent, tbb::split)
        : _ctx(parent._ctx),
          _engine(parent._engine),
          _caller(parent._caller),
          _callees_cache(parent._callees_cache),
          _call(parent._call),
          _callee_analyses(parent._callee_analyses),
          _post(llvm::None) {}

    /// \brief No copy constructor
    CalleeWorker(const CalleeWorker&) = delete;

    /// \brief No move constructor
    CalleeWorker(CalleeWorker&&) = delete;

    /// \brief No copy assignment operator
    CalleeWorker& operator=(const CalleeWorker&) = delete;

    /// \brief No move assignment operator
    CalleeWorker& operator=(CalleeWorker&&) = delete;

    /// \brief Destructor
    ~CalleeWorker() = default;

    void operator()(const tbb::blocked_range< size_t >& r) {
      for (auto i = r.begin(); i != r.end(); i++) {
        this->operator()(this->_callee_analyses[i]);
      }
    }

    /// \brief Analyze a callee
    void operator()(CalleeAnalysis& analysis) {
      NumericalExecutionEngineT engine = this->_engine.fork();

      // Do not propagate exceptions from the caller to the callee
      engine.inv().ignore_exceptions();

      // Assign parameters
      engine.match_down(this->_call, analysis.callee);

      analysis.fixpoint = nullptr;

      if (_ctx.opts.use_fixpoint_cache && this->_caller.converged()) {
        // Try to fetch the previously computed fix-point
        analysis.fixpoint =
            this->_callees_cache.try_fetch(this->_call, analysis.callee);
      }

      if (analysis.fixpoint == nullptr) {
        if (_ctx.opts.use_fixpoint_cache) {
          // Erase the previous fix-point on the callee
          this->_callees_cache.erase(this->_call, analysis.callee);
        }

        // Create a fixpoint on the callee
        analysis.fixpoint =
            std::make_unique< FunctionFixpoint >(_ctx,
                                                 this->_caller,
                                                 this->_call,
                                                 analysis.callee);

        // Run analysis on callee
        analysis.fixpoint->run(std::move(engine.inv()));
      }

      // Return statement in the callee, or null
      ar::ReturnValue* return_stmt = analysis.fixpoint->return_stmt();

      engine.set_inv(analysis.fixpoint->exit_invariant());

      // Merge exceptions in caught_exceptions, in case it's an invoke
      engine.inv().merge_propagated_in_caught_exceptions();

      if (engine.inv().is_normal_flow_bottom()) {
        // Collect the exception states
        this->post_join(std::move(engine.inv()));
        return;
      }

      engine.match_up(this->_call, return_stmt);
      this->post_join(std::move(engine.inv()));
    }

    /// \brief Join two workers
    void join(CalleeWorker& other) {
      if (other._post) {
        this->post_join(std::move(*other._post));
      }
    }

    /// \brief Join the post invariant
    void post_join(AbstractDomain&& inv) {
      if (this->_post) {
        (*this->_post).join_with(std::move(inv));
      } else {
        this->_post = std::move(inv);
      }
    }

    /// \brief Join the post invariant
    void post_join(const AbstractDomain& inv) {
      if (this->_post) {
        (*this->_post).join_with(inv);
      } else {
        this->_post = inv;
      }
    }

    /// \brief Return the post invariant
    AbstractDomain& post() {
      ikos_assert(this->_post);
      return *this->_post;
    }

  }; // end class CalleeWorker

  /// \brief Execute any call statement
  void exec(ar::CallBase* call) {
    this->inv().normal().normalize();

    if (this->inv().is_normal_flow_bottom()) {
      return;
    }

    //
    // Collect potential callees
    //
    auto callees = PointsToSet::bottom();
    ar::Value* called = call->called();

    if (isa< ar::UndefinedConstant >(called)) {
      // Call on undefined pointer: error
      this->inv().set_normal_flow_to_bottom();
      return;
    } else if (isa< ar::NullConstant >(called)) {
      // Call on null pointer: error
      this->inv().set_normal_flow_to_bottom();
      return;
    } else if (auto cst = dyn_cast< ar::FunctionPointerConstant >(called)) {
      callees = {_ctx.mem_factory->get_function(cst->function())};
    } else if (isa< ar::InlineAssemblyConstant >(called)) {
      // Call to assembly
      this->_engine.exec_unknown_extern_call(call);
      return;
    } else if (isa< ar::GlobalVariable >(called)) {
      // Call to global variable: error
      this->inv().set_normal_flow_to_bottom();
      return;
    } else if (isa< ar::LocalVariable >(called)) {
      // Call to local variable: error
      this->inv().set_normal_flow_to_bottom();
      return;
    } else if (auto ptr = dyn_cast< ar::InternalVariable >(called)) {
      // Indirect call through a function pointer
      Variable* ptr_var = _ctx.var_factory->get_internal(ptr);

      // Assert `ptr != null`
      this->inv().normal().nullity_assert_non_null(ptr_var);

      // Reduction between value and pointer analysis
      const PointerInfo* pointer_info = this->_engine.pointer_info();
      if (pointer_info != nullptr) {
        PointsToSet points_to = pointer_info->get(ptr_var).points_to();

        // Pointer analysis and value analysis can be inconsistent
        if (!points_to.is_bottom() && !points_to.is_top()) {
          this->inv().normal().pointer_refine(ptr_var, points_to);
        }
      }

      this->inv().normal().normalize();

      if (this->inv().is_normal_flow_bottom()) {
        return;
      }

      // Get the callees
      callees = this->inv().normal().pointer_to_points_to(ptr_var);
    } else {
      ikos_unreachable("unexpected called operand");
    }

    //
    // Check callees
    //
    ikos_assert(!callees.is_bottom());
    if (callees.is_empty()) {
      // Invalid pointer dereference
      this->inv().set_normal_flow_to_bottom();
      return;
    } else if (callees.is_top()) {
      // No points-to information
      // ASSUMPTION: the callee has no side effects.
      // Just set lhs and all actual parameters of pointer type to TOP.
      this->_engine.exec_unknown_extern_call(call);
      return;
    }

    //
    // Compute the post invariant
    //

    // By default, propagate the exception states
    AbstractDomain post = this->inv();
    post.set_normal_flow_to_bottom();

    // Analyses on callees
    std::vector< CalleeAnalysis > callee_analyses;

    // For each callee
    for (MemoryLocation* mem : callees) {
      if (!isa< FunctionMemoryLocation >(mem)) {
        // Not a call to a function memory location
        continue;
      }

      ar::Function* callee = cast< FunctionMemoryLocation >(mem)->function();

      if (!ar::TypeVerifier::is_valid_call(call, callee->type())) {
        // Ill-formed function call
        //
        // This could be because of an imprecision of the pointer analysis.
        continue;
      }

      if (callee->is_declaration()) {
        // Call to an extern function
        //
        // ASSUMPTION: if this is a call to an extern non-intrinsic function,
        // treat it as a function call that has no side effects.
        NumericalExecutionEngineT engine = this->_engine.fork();
        engine.inv().ignore_exceptions();
        engine.exec_extern_call(call, callee);
        engine.inv().merge_propagated_in_caught_exceptions();
        post.join_with(std::move(engine.inv()));
        continue;
      }
      ikos_assert(callee->is_definition());

      if (this->_caller.function() == callee ||
          this->_caller.call_context()->contains(callee)) {
        // Recursive function call
        //
        // TODO(jnavas): we can be more precise by making top only lhs of
        // call_stmt, actual parameters of pointer type and any global variable
        // that might be touched by the recursive function.
        this->_engine.exec_unknown_intern_call(call);
        return;
      }

      //
      // Analyze recursively the callee
      //

      callee_analyses.push_back(CalleeAnalysis(callee));
    }

    CalleeWorker worker(this->_ctx,
                        this->_engine,
                        this->_caller,
                        this->_callees_cache,
                        call,
                        callee_analyses,
                        std::move(post));
    tbb::blocked_range< size_t > range(0, callee_analyses.size());
    tbb::parallel_reduce(range, worker);

    // Non-thread safe
    for (CalleeAnalysis& analysis : callee_analyses) {
      if (this->_check_callees) {
        // Run the checks on the callee
        analysis.fixpoint->run_checks();
      }

      if (_ctx.opts.use_fixpoint_cache) {
        // Save the fix-point for later
        this->_callees_cache.store(call,
                                   analysis.callee,
                                   std::move(analysis.fixpoint));
      }
    }

    this->_engine.set_inv(std::move(worker.post()));
  }

}; // end class ConcurrentInlineCallExecutionEngine

} // end namespace analyzer
} // end namespace ikos
