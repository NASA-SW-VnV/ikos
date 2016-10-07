/*******************************************************************************
 *
 * Generic API for executing ARBOS statements.
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2016 United States Government as represented by the
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

#ifndef ANALYZER_SYM_EXEC_API_HPP
#define ANALYZER_SYM_EXEC_API_HPP

#include <unordered_set>

#include <boost/optional.hpp>

#include <ikos/domains/exception_domains_api.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/analysis/context.hpp>
#include <analyzer/analysis/liveness.hpp>
#include <analyzer/ar-wrapper/cfg.hpp>

namespace analyzer {

using namespace arbos;

// API for executing relevant ARBOS statements
template < typename AbsDomain >
class sym_exec {
public:
  virtual void exec_start(Basic_Block_ref b) = 0;
  virtual void exec_end(Basic_Block_ref b) = 0;
  virtual void exec(Arith_Op_ref stmt) = 0;
  virtual void exec(Integer_Comparison_ref stmt) = 0;
  virtual void exec(FP_Op_ref stmt) = 0;
  virtual void exec(FP_Comparison_ref stmt) = 0;
  virtual void exec(Assignment_ref stmt) = 0;
  virtual void exec(Conv_Op_ref stmt) = 0;
  virtual void exec(Bitwise_Op_ref stmt) = 0;
  virtual void exec(Store_ref stmt) = 0;
  virtual void exec(Load_ref stmt) = 0;
  virtual void exec(Pointer_Shift_ref stmt) = 0;
  virtual void exec(Abstract_Variable_ref stmt) = 0;
  virtual void exec(Abstract_Memory_ref stmt) = 0;
  virtual void exec(MemCpy_ref stmt) = 0;
  virtual void exec(MemMove_ref stmt) = 0;
  virtual void exec(MemSet_ref stmt) = 0;
  virtual void exec(Call_ref stmt) = 0; // only for external calls
  virtual void exec_external_call(boost::optional< Internal_Variable_ref > lhs,
                                  std::string fun_name,
                                  OpRange arguments) = 0;
  virtual void exec(Invoke_ref stmt) = 0;
  virtual void exec(Return_Value_ref) = 0;
  virtual void exec(Landing_Pad_ref stmt) = 0;
  virtual void exec(Resume_ref stmt) = 0;
  virtual void exec(Unreachable_ref stmt) = 0;

  /*
   * Assign the formal parameters to the actual parameters before a function
   * call.
   *
   * Arguments:
   *   formals: The list of formal parameters
   *   actuals: The list of actual parameters
   */
  virtual void match_down(IvRange formals, OpRange actuals) = 0;

  /*
   * Assign the return value after a function call.
   *
   * Arguments:
   *   actuals: The list of actual parameters
   *   lhs_ret: The result variable of the caller
   *   formals: The list of formal parameters
   *   callee_ret: The return statement operand of the callee
   */
  virtual void match_up(OpRange actuals,
                        boost::optional< Internal_Variable_ref > lhs_ret,
                        IvRange formals,
                        boost::optional< Operand_ref > callee_ret) = 0;

  virtual ~sym_exec() {}
};

// API to analyze internal function calls based on some
// inter-procedural strategy (e.g., inlining, summaries-based, etc).
// An internal function is one which is neither a declaration or
// external so its code is available for analysis.
template < typename FunctionAnalyzer, typename AbsDomain >
class sym_exec_call {
protected:
  TrackedPrecision _prec_level;

public:
  typedef sym_exec_call< FunctionAnalyzer, AbsDomain > sym_exec_call_t;
  typedef std::unordered_set< std::string > function_names_t;
  typedef std::shared_ptr< sym_exec_call_t > sym_exec_call_ptr_t;

public:
  sym_exec_call(TrackedPrecision level) : _prec_level(level) {}

  /*
   * Analyze a function call
   *
   * Arguments:
   *   ctx: The global context
   *   call_stmt: The AR call statement
   *   pre: The pre invariant
   *   convergence_achieved: true iff the fixpoint has been reached for the
   *                         current function
   *   is_context_stable: true iff the calling context is stable, ie. the
   *                      fixpoint has been reached for all calling functions
   *   caller: The function analyzer object of the caller
   *   call_context: The full call trace
   *   analyzed_functions: The set of already analyzed functions
   */
  virtual AbsDomain call(context& ctx,
                         Call_ref call_stmt,
                         AbsDomain pre,
                         bool convergence_achieved,
                         bool is_context_stable,
                         FunctionAnalyzer& caller,
                         std::string call_context,
                         function_names_t analyzed_functions) = 0;

  /*
   * Analyze a return statement
   *
   * Arguments:
   *   stmt: The return statement
   *   pre: The pre invariant
   */
  virtual void ret(Return_Value_ref stmt, AbsDomain pre) = 0;

  /*
   * Catch the invariant at the end of the function
   *
   * This is called whenever we reach the exit node. Note that this can be
   * different from the invariant at the return statement, if an exception
   * has been thrown for instance.
   */
  virtual void exit(AbsDomain inv) = 0;

  virtual ~sym_exec_call() {}
};

// A default implementation for sym_exec_call that performs
// context-insensitive analysis.
template < typename FunctionAnalyzer, typename AbsDomain >
class context_insensitive_sym_exec_call
    : public sym_exec_call< FunctionAnalyzer, AbsDomain > {
private:
  typedef sym_exec_call< FunctionAnalyzer, AbsDomain > sym_exec_call_t;
  typedef typename sym_exec_call_t::function_names_t function_names_t;

public:
  context_insensitive_sym_exec_call(TrackedPrecision prec_level)
      : sym_exec_call_t(prec_level) {}

  AbsDomain call(context& ctx, Call_ref call_stmt, AbsDomain pre) {
    if (ikos::exc_domain_traits::is_normal_flow_bottom(pre)) {
      return pre;
    }

    ikos::exc_domain_traits::set_pending_exceptions_top(pre);

    // abstract the lhs of the call site instruction
    if (ar::getReturnValue(call_stmt)) {
      pre -= ctx.var_factory()[ar::getName(*(ar::getReturnValue(call_stmt)))];
    }

    // abstract conservatively any memory location modified by the
    // callee.
    if (this->_prec_level >= MEM) {
      assert(false &&
             "Context insensitive analysis with memory not implemented");
    }

    return pre;
  }

  AbsDomain call(context& ctx,
                 Call_ref call_stmt,
                 AbsDomain pre,
                 bool /*convergence_achieved*/,
                 bool /*is_context_stable*/,
                 FunctionAnalyzer& /*caller*/,
                 std::string /*call_context*/,
                 function_names_t /*analyzed_functions*/) {
    return call(ctx, call_stmt, pre);
  }

  void ret(Return_Value_ref /*stmt*/, AbsDomain /*pre*/) {}

  void exit(AbsDomain /*pre*/) {}
};

// API for modelling matching of formal and actual parameters as well
// as simulating call-by-ref during a function call.
template < class AbsDomain >
class sym_exec_match_call_params {
public:
  // Match formal and actual parameters
  virtual AbsDomain propagate_down(IvRange formals,
                                   AbsDomain callee,
                                   OpRange actuals,
                                   AbsDomain caller,
                                   context& ctx) = 0;

  // Simulate call-by-reference and propagate return value
  virtual AbsDomain propagate_up(
      OpRange actuals,
      boost::optional< Internal_Variable_ref > lhs_cs,
      AbsDomain caller,
      IvRange formals,
      boost::optional< Operand_ref > callee_ret,
      AbsDomain callee,
      context& ctx) = 0;

  virtual ~sym_exec_match_call_params() {}
};

} // end namespace analyzer

#endif // ANALYZER_SYM_EXEC_API_HPP
