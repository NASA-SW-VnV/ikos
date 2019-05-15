/*******************************************************************************
 *
 * \file
 * \brief Dynamic inlining call semantic
 *
 * Author: Maxime Arthaud
 *
 * Contributors: Jorge A. Navas
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

#pragma once

#include <memory>

#include <boost/container/flat_map.hpp>

#include <llvm/ADT/DenseMap.h>

#include <ikos/ar/semantic/function.hpp>
#include <ikos/ar/verify/type.hpp>

#include <ikos/analyzer/analysis/execution_engine/engine.hpp>
#include <ikos/analyzer/analysis/execution_engine/numerical.hpp>
#include <ikos/analyzer/analysis/pointer/value.hpp>
#include <ikos/analyzer/util/demangle.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {

/// \brief Inliner of function calls.
///
/// The inlining of a function is done dynamically by matching formal and actual
/// parameters and analyzing recursively the callee (FunctionFixpoint) and after
/// the callee returns by simulating call-by-ref and updating the return value
/// at the call site. The inlining also supports function pointers by resolving
/// first the set of possible callees and joining the results.
template < typename FunctionAnalyzer, typename AbstractDomain >
class InlineCallExecutionEngine final : public CallExecutionEngine {
public:
  using InlineCallExecutionEngineT =
      InlineCallExecutionEngine< FunctionAnalyzer, AbstractDomain >;
  using NumericalExecutionEngineT = NumericalExecutionEngine< AbstractDomain >;

private:
  /// \brief Map from callee function to FunctionAnalyzer
  using CalleeMap =
      boost::container::flat_map< ar::Function*,
                                  std::unique_ptr< FunctionAnalyzer > >;

  /// \brief Map from call statement to CalleeMap
  using CallMap = llvm::DenseMap< ar::CallBase*, CalleeMap >;

private:
  /// \brief Analysis context
  Context& _ctx;

  /// \brief Numerical execution engine
  NumericalExecutionEngineT& _engine;

  /// \brief Function analyzer of the caller
  const FunctionAnalyzer& _caller;

  /// \brief Invariant at the end of the function
  AbstractDomain _exit_inv;

  /// \brief Return statement, or null
  ar::ReturnValue* _return_stmt;

  /// \brief Store previously-computed fixpoints on callees
  CallMap _calls_cache;

  /// \brief True if the calling context is stable
  bool _context_stable;

  /// \brief True if the fixpoint of the current function is reached
  bool _convergence_achieved;

  /// \brief True to check properties on the callees
  bool _check_callees;

public:
  /// \brief Constructor
  InlineCallExecutionEngine(Context& ctx,
                            NumericalExecutionEngineT& engine,
                            const FunctionAnalyzer& caller,
                            bool context_stable,
                            bool convergence_achieved)
      : _ctx(ctx),
        _engine(engine),
        _caller(caller),
        _exit_inv(AbstractDomain::bottom()),
        _return_stmt(nullptr),
        _context_stable(context_stable),
        _convergence_achieved(convergence_achieved),
        _check_callees(false) {}

  /// \brief Mark that the calling context is stable
  void mark_context_stable() { this->_context_stable = true; }

  /// \brief Mark that the reached the fixpoint
  void mark_convergence_achieved() { this->_convergence_achieved = true; }

  /// \brief Mark to check the callees
  void mark_check_callees() { this->_check_callees = true; }

  /// \brief Return the exit invariant, or bottom
  const AbstractDomain& exit_invariant() const { return this->_exit_inv; }

  /// \brief Return the return statement, or null
  ar::ReturnValue* return_stmt() const { return this->_return_stmt; }

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
    this->_exit_inv = this->_engine.inv();
  }

  /// \brief Execute a ReturnValue statement
  void exec(ar::ReturnValue* s) override {
    ikos_assert_msg(this->_return_stmt == nullptr || this->_return_stmt == s,
                    "input code has more than one return statement");
    this->_return_stmt = s;
  }

  /// \brief Execute a Call statement
  void exec(ar::Call* s) override {
    // execute the call base statement
    this->exec(cast< ar::CallBase >(s));

    // exceptions aren't caught, propagate them
    this->inv().merge_caught_in_propagated_exceptions();
  }

  /// \brief Execute an Invoke statement
  void exec(ar::Invoke* s) override {
    // execute the call base statement
    this->exec(cast< ar::CallBase >(s));

    // Exceptions are caught.
    // Nothing to do here.
    // see NumericalExecutionEngine::exec_edge()
  }

private:
  /// \brief Return a non-const reference on the current invariant
  AbstractDomain& inv() { return this->_engine.inv(); }

  /// \brief Execute any call statement
  void exec(ar::CallBase* call) {
    if (this->inv().is_normal_flow_bottom()) {
      return;
    }

    //
    // Collect potential callees
    //
    PointsToSet callees;
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

      if (this->inv().normal().uninitialized().is_uninitialized(ptr_var) ||
          this->inv().normal().nullity().is_null(ptr_var)) {
        // null/undefined dereference
        this->inv().set_normal_flow_to_bottom();
        return;
      }

      // Reduction between value and pointer analysis: refine the set of
      // potential callees
      const PointerInfo* pointer_info = this->_engine.pointer_info();
      if (pointer_info != nullptr) {
        PointsToSet points_to = pointer_info->get(ptr_var).points_to();

        // Pointer analysis and value analysis can be inconsistent
        if (!points_to.is_bottom() && !points_to.is_top()) {
          this->inv().normal().pointers().refine(ptr_var, points_to);
        }
      }

      // Get the callees
      callees = this->inv().normal().pointers().points_to(ptr_var);
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
    AbstractDomain post(this->inv());
    post.set_normal_flow_to_bottom();

    // For each callee
    for (MemoryLocation* mem : callees) {
      // Check if the callee is a function
      if (!isa< FunctionMemoryLocation >(mem)) {
        // Not a call to a function memory location
        continue;
      }

      ar::Function* callee = cast< FunctionMemoryLocation >(mem)->function();

      if (!ar::TypeVerifier::is_valid_call(call, callee->type())) {
        // Ill-formed function call
        // This could be because of an imprecision of the pointer analysis.
        continue;
      }

      if (callee->is_declaration()) {
        // ASSUMPTION: if this is a call to an extern non-intrinsic function,
        // treat it as a function call that has no side effects.
        NumericalExecutionEngineT engine(this->_engine.fork());
        engine.inv().ignore_exceptions();
        engine.exec_extern_call(call, callee);
        engine.inv().merge_propagated_in_caught_exceptions();
        post.join_with(engine.inv());
        continue;
      }
      ikos_assert(callee->is_definition());

      if (this->_caller.is_currently_analyzed(callee)) {
        // TODO(jnavas): we can be more precise by making top only lhs of
        // call_stmt, actual parameters of pointer type and any global variable
        // that might be touched by the recursive function.
        this->_engine.exec_unknown_intern_call(call);
        return;
      }

      NumericalExecutionEngineT engine(this->_engine.fork());

      // Do not propagate exceptions from the caller to the callee
      engine.inv().ignore_exceptions();

      // Assign parameters
      engine.match_down(call, callee);

      //
      // Analyze recursively the callee
      //

      std::unique_ptr< FunctionAnalyzer > callee_analyzer = nullptr;

      if (this->_convergence_achieved && _ctx.opts.use_fixpoint_cache) {
        // Use the previously computed fix-point
        callee_analyzer = std::move(this->_calls_cache[call][callee]);

        if (this->_context_stable) {
          // Calling context is stable
          callee_analyzer->mark_context_stable();
        }
      } else {
        if (_ctx.opts.use_fixpoint_cache) {
          // Erase the previous fix-point on the callee
          this->_calls_cache[call][callee].reset();
        }

        // Create a fixpoint on the callee
        callee_analyzer = std::make_unique<
            FunctionAnalyzer >(_ctx,
                               _caller,
                               call,
                               callee,
                               this->_context_stable &&
                                   this->_convergence_achieved);

        // Run analysis on callee
        log::debug("Analyzing function '" + demangle(callee->name()) + "'");
        callee_analyzer->run(engine.inv());
      }

      if (this->_check_callees) {
        // Run the checks on the callee
        callee_analyzer->run_checks();
      }

      // Return statement in the callee, or null
      ar::ReturnValue* return_stmt = callee_analyzer->return_stmt();

      engine.set_inv(callee_analyzer->exit_invariant());

      if (_ctx.opts.use_fixpoint_cache) {
        // Save the fix-point for later
        this->_calls_cache[call][callee] = std::move(callee_analyzer);
      } else {
        // Delete the callee fix-point
        callee_analyzer.reset();
      }

      // Merge exceptions in caught_exceptions, in case it's an invoke
      engine.inv().merge_propagated_in_caught_exceptions();

      if (engine.inv().is_normal_flow_bottom()) {
        post.join_with(engine.inv()); // collect the exception states
        continue;
      }

      engine.match_up(call, return_stmt);
      post.join_with(engine.inv());
    }

    this->_engine.set_inv(std::move(post));
  }

}; // end class InlineCallExecutionEngine

} // end namespace analyzer
} // end namespace ikos
