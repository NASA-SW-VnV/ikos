/*******************************************************************************
 *
 * This class inlines dynamically functions.
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * It inlines direct calls as well as indirect calls if they can be
 * resolved at compile time. It returns top when the callee is:
 * - recursive call
 * - variable argument list call
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

#ifndef ANALYZER_INLINER_HPP
#define ANALYZER_INLINER_HPP

#include <analyzer/analysis/common.hpp>
#include <analyzer/analysis/context.hpp>
#include <analyzer/analysis/num_sym_exec.hpp>
#include <analyzer/analysis/sym_exec_api.hpp>
#include <analyzer/utils/demangle.hpp>

namespace analyzer {

using namespace arbos;

template < typename AbsDomain >
class sym_exec_call_params_matcher
    : public sym_exec_match_call_params< AbsDomain > {
  typedef num_sym_exec< AbsDomain,
                        varname_t /*VariableName*/,
                        number_t /*Number*/ > sym_exec_t;

public:
  AbsDomain propagate_down(IvRange formals,
                           AbsDomain callee,
                           OpRange actuals,
                           AbsDomain caller,
                           context& ctx) {
    sym_exec_t sym_exec(callee & caller,
                        ctx.var_factory(),
                        ctx.lit_factory(),
                        ctx.prec_level(),
                        ctx.pointer_info());
    sym_exec.match_down(formals, actuals);
    return sym_exec.inv();
  }

  AbsDomain propagate_up(OpRange actuals,
                         boost::optional< Internal_Variable_ref > lhs_cs_ret,
                         AbsDomain /*caller*/,
                         IvRange formals,
                         boost::optional< Operand_ref > callee_ret,
                         AbsDomain callee,
                         context& ctx) {
    sym_exec_t sym_exec(callee,
                        ctx.var_factory(),
                        ctx.lit_factory(),
                        ctx.prec_level(),
                        ctx.pointer_info());
    sym_exec.match_up(actuals, lhs_cs_ret, formals, callee_ret);
    return sym_exec.inv();
  }
};

/// Inliner of function calls.
///
/// The inlining of a function is done dynamically by matching formal
/// and actual parameters (sym_exec_call_params_matcher), creating a new CFG
/// for the callee, analyzing recursively the callee (FunctionAnalyzer) and
/// after the callee returns by simulating call-by-ref
/// (sym_exec_call_params_matcher) and updating the return value at the call
/// site. The inlining also supports function pointers by resolving
/// first the set of possible callees and joining the results.

template < typename FunctionAnalyzer, typename AbsDomain >
class inline_sym_exec_call
    : public sym_exec_call< FunctionAnalyzer, AbsDomain >,
      private sym_exec_call_params_matcher< AbsDomain > {
private:
  typedef num_sym_exec< AbsDomain,
                        varname_t /*VariableName*/,
                        number_t /*Number*/ > sym_exec_t;
  typedef sym_exec_call< FunctionAnalyzer, AbsDomain > sym_exec_call_t;
  typedef typename sym_exec_call_t::sym_exec_call_ptr_t sym_exec_call_ptr_t;
  typedef typename sym_exec_call_t::function_names_t function_names_t;

  typedef inline_sym_exec_call< FunctionAnalyzer, AbsDomain >
      inline_sym_exec_call_t;
  typedef typename std::shared_ptr< inline_sym_exec_call_t >
      inline_sym_exec_call_ptr_t;

private:
  AbsDomain _exit_inv; // invariant at the end of the callee
  boost::optional< Operand_ref > _exit_op;

public:
  inline_sym_exec_call(TrackedPrecision prec_level)
      : sym_exec_call_t(prec_level),
        sym_exec_call_params_matcher< AbsDomain >(),
        _exit_inv(AbsDomain::bottom()),
        _exit_op() {}

  AbsDomain call(context& ctx,
                 Call_ref call_stmt,
                 AbsDomain caller_inv,
                 bool convergence_achieved,
                 bool is_context_stable,
                 FunctionAnalyzer& caller,
                 std::string call_context,
                 function_names_t analyzed_functions) {
    if (exc_domain_traits::is_normal_flow_bottom(caller_inv)) {
      return caller_inv;
    }

    std::string caller_s = caller.function_name();
    location loc = ar::getSrcLoc(call_stmt);
    boost::optional< Internal_Variable_ref > lhs =
        ar::getReturnValue(call_stmt);
    OpRange actual_params = ar::getArguments(call_stmt);

    call_context += "/" + caller_s + "@" + std::to_string(loc.line) + "@" +
                    std::to_string(loc.column);

    /*
     * Collect potential callees
     */

    std::vector< std::string > callees;

    if (ar::isIndirectCall(call_stmt)) {
      // indirect call through a function pointer
      Literal fvar = ctx.lit_factory()[ar::getIndirectCallVar(call_stmt)];
      assert(fvar.is_var());

      // reduction between value and pointer analysis: refine the set
      // of potential callees
      std::pair< PointerInfo::ptr_set_t, ikos::z_interval > ptr_info =
          ctx.pointer_info()[fvar.get_var()];

      ptr_domain_traits::refine_addrs(caller_inv,
                                      fvar.get_var(),
                                      ptr_info.first);

      if (!ptr_domain_traits::is_unknown_addr(caller_inv, fvar.get_var())) {
        ikos::discrete_domain< varname_t > ptr_set =
            ptr_domain_traits::addrs_set(caller_inv, fvar.get_var());
        for (auto it = ptr_set.begin(); it != ptr_set.end(); ++it) {
          callees.push_back(it->name());
        }
      } else {
        // No points-to information
        std::cout
            << location_to_string(loc)
            << ": warning: indirect call cannot be resolved" << std::endl
            << "The analysis might be unsound if the function has side effects."
            << std::endl;

        // ASSUMPTION: the callee has no side effects.
        // Just set lhs and all actual parameters of pointer type to TOP.
        sym_exec_t sym_exec(caller_inv,
                            ctx.var_factory(),
                            ctx.lit_factory(),
                            ctx.prec_level(),
                            ctx.pointer_info());
        sym_exec.exec_external_call(lhs, "<unknown>", actual_params);
        return sym_exec.inv();
      }
    } else { // direct call
      callees.push_back(ar::getFunctionName(call_stmt));
    }

    assert(!callees.empty());

    /*
     * Compute the post invariant
     */

    // by default, propagate the exception states
    AbsDomain post = caller_inv;
    exc_domain_traits::set_normal_flow_bottom(post);

    bool resolved = false;

    for (std::vector< std::string >::iterator it = callees.begin();
         it != callees.end();
         ++it) {
      const std::string& callee_s = *it;
      boost::optional< Function_ref > callee = ctx[callee_s];

      if (!callee || ar::isExternal(*callee)) {
        // ASSUMPTION: if the function code is not available, treat it as
        // function call that has no side effects.
        sym_exec_t sym_exec(caller_inv,
                            ctx.var_factory(),
                            ctx.lit_factory(),
                            ctx.prec_level(),
                            ctx.pointer_info());
        sym_exec.exec_external_call(lhs, callee_s, actual_params);
        post = post | sym_exec.inv();
        resolved = true;
      } else {
        if (ar::isVarargs(*callee)) {
          std::cout << location_to_string(loc)
                    << ": warning: skipping safely function "
                    << demangle(callee_s)
                    << " with variable number of arguments." << std::endl;

          // TODO: we can be more precise by making top only lhs of call_stmt,
          // actual parameters of pointer type and any global variable
          // that might be touched by the recursive function.
          return AbsDomain::top();
        }

        IvRange formal_params = ar::getFormalParams(*callee);

        if (actual_params.size() != formal_params.size()) {
          // ASSUMPTION: all function calls have been checked by the compiler
          // and are well-formed. In that case, it means this function cannot be
          // called and that it is just an imprecision of the pointer analysis.
          continue;
        }

        if (is_recursive(callee_s, analyzed_functions)) {
          std::cout << location_to_string(loc)
                    << ": warning: skipping safely recursive call "
                    << demangle(callee_s) << std::endl;

          // TODO: we can be more precise by making top only lhs of call_stmt,
          // actual parameters of pointer type and any global variable
          // that might be touched by the recursive function.
          return AbsDomain::top();
        }

        resolved = true;
        arbos_cfg callee_cfg = ctx[*callee];

        // match actual with formal parameters
        AbsDomain callee_entry_inv = AbsDomain::top();
        callee_entry_inv = this->propagate_down(formal_params,
                                                callee_entry_inv,
                                                actual_params,
                                                caller_inv,
                                                ctx);

        /*
         * Analyze recursively the callee
         */

        analyzed_functions.insert(callee_s);

        sym_exec_call_ptr_t callee_it(
            new inline_sym_exec_call_t(this->_prec_level));

        FunctionAnalyzer callee_analyzer(callee_cfg,
                                         ctx,
                                         callee_it,
                                         is_context_stable &&
                                             convergence_achieved,
                                         caller,
                                         call_context,
                                         analyzed_functions);

        if (!convergence_achieved) {
          std::cout << "*** Analyzing function: " << demangle(callee_s)
                    << std::endl;
        }

        callee_analyzer.run(callee_entry_inv);

        inline_sym_exec_call_ptr_t inliner =
            std::static_pointer_cast< inline_sym_exec_call_t >(callee_it);
        AbsDomain callee_exit_inv = inliner->_exit_inv;
        boost::optional< Operand_ref > ret_val = inliner->_exit_op;

        if (exc_domain_traits::is_normal_flow_bottom(callee_exit_inv)) {
          post = post | callee_exit_inv; // collect the exception states
          continue;
        }

        // call-by-ref of pointers and propagation of return value
        OpRange out_actuals;
        IvRange out_formals;
        if (this->_prec_level >= MEM) {
          IvRange::iterator fIt = formal_params.begin();
          for (OpRange::iterator it = actual_params.begin(),
                                 et = actual_params.end();
               it != et;
               ++it, ++fIt) {
            if (ar::isPointer(*it) && ar::isRegVar(*it)) {
              out_actuals.push_back(*it);
              out_formals.push_back(*fIt);
            }
          }
        }

        post = post |
               this->propagate_up(out_actuals,
                                  lhs,
                                  caller_inv,
                                  out_formals,
                                  ret_val,
                                  callee_exit_inv,
                                  ctx);
      }
    }

    if (!resolved) {
      // the points-to analysis was not precise enough.
      std::cout
          << location_to_string(loc)
          << ": warning: indirect call cannot be resolved" << std::endl
          << "The analysis might be unsound if the function has side effects."
          << std::endl;

      // ASSUMPTION: the callee has no side effects.
      // Just set lhs and all actual parameters of pointer type to TOP.
      sym_exec_t sym_exec(caller_inv,
                          ctx.var_factory(),
                          ctx.lit_factory(),
                          ctx.prec_level(),
                          ctx.pointer_info());
      sym_exec.exec_external_call(lhs, "<unknown>", actual_params);
      return sym_exec.inv();
    }

    return post;
  }

  void ret(Return_Value_ref s, AbsDomain pre) {
    // Assume the code has only one return statement.
    // This is enforced by the UnifyFunctionExitNodesPass llvm pass
    assert(!_exit_op || _exit_op == ar::getReturnValue(s));
    _exit_op = ar::getReturnValue(s);
  }

  void exit(AbsDomain inv) { _exit_inv = inv; }

private:
  inline bool is_recursive(const std::string& function_name,
                           const function_names_t& visited) {
    return visited.find(function_name) != visited.end();
  }
};

} // end namespace analyzer

#endif // ANALYZER_INLINER_HPP
