/*******************************************************************************
 *
 * This class inlines dynamically functions.
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *               Clement Decoodt
 *
 * Contact: ikos@lists.nasa.gov
 *
 * It inlines direct calls as well as indirect calls if they can be
 * resolved at compile time. It returns top when the callee is a recursive
 * call.
 *
 * Notices:
 *
 * Copyright (c) 2011-2017 United States Government as represented by the
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

/// Inliner of function calls.
///
/// The inlining of a function is done dynamically by matching formal
/// and actual parameters (sym_exec::match_up), creating a new CFG
/// for the callee, analyzing recursively the callee (FunctionAnalyzer) and
/// after the callee returns by simulating call-by-ref
/// (sym_exec::match_down) and updating the return value at the call
/// site. The inlining also supports function pointers by resolving
/// first the set of possible callees and joining the results.

template < typename FunctionAnalyzer, typename AbsDomain >
class inline_sym_exec_call
    : public sym_exec_call< FunctionAnalyzer, AbsDomain > {
private:
  typedef num_sym_exec< AbsDomain,
                        number_t /*Integer*/,
                        ikos::dummy_number /*Float*/
                        >
      sym_exec_t;
  typedef sym_exec_call< FunctionAnalyzer, AbsDomain > sym_exec_call_t;
  typedef typename sym_exec_call_t::sym_exec_call_ptr_t sym_exec_call_ptr_t;
  typedef typename sym_exec_call_t::function_names_t function_names_t;

  typedef inline_sym_exec_call< FunctionAnalyzer, AbsDomain >
      inline_sym_exec_call_t;
  typedef typename std::shared_ptr< inline_sym_exec_call_t >
      inline_sym_exec_call_ptr_t;

  typedef std::unique_ptr< FunctionAnalyzer > function_analyzer_ptr_t;
  typedef boost::container::flat_map< std::string, function_analyzer_ptr_t >
      callee_map_t;
  typedef std::unordered_map< index64_t, callee_map_t > call_map_t;

private:
  AbsDomain _exit_inv; // invariant at the end of the callee

  boost::optional< Operand_ref > _exit_op; // exit statement

  call_map_t _call_map; // map of function calls

public:
  inline_sym_exec_call(TrackedPrecision prec_level)
      : sym_exec_call_t(prec_level),
        _exit_inv(AbsDomain::bottom()),
        _exit_op(),
        _call_map() {}

  AbsDomain call(
      context& ctx,
      FunctionAnalyzer& caller,
      Call_ref call_stmt,
      AbsDomain caller_inv,
      bool convergence_achieved,
      bool is_context_stable,
      std::string call_context,
      function_names_t analyzed_functions,
      std::shared_ptr< sym_exec_warning_callback > warning_callback = nullptr) {
    if (ikos::exc_domain_traits::is_normal_flow_bottom(caller_inv)) {
      if (warning_callback) {
        json_dict info = {{"type", "unreachable"}};
        warning_callback->warning("call",
                                  "unreachable function call",
                                  call_stmt,
                                  info);
      }

      return caller_inv;
    }

    // update call context
    std::string caller_s = caller.func_name();
    location loc = ar::getSrcLoc(call_stmt);
    boost::optional< Internal_Variable_ref > lhs =
        ar::getReturnValue(call_stmt);
    OpRange actual_params = ar::getArguments(call_stmt);

    call_context += "/" + caller_s + "@" + std::to_string(loc.line) + "@" +
                    std::to_string(loc.column) + "@" +
                    std::to_string(ar::getUID(call_stmt));

    /*
     * Collect potential callees
     */

    std::vector< memloc_t > callees;

    if (ar::isIndirectCall(call_stmt)) {
      // indirect call through a function pointer
      scalar_lit_t ptr = ctx.lit_factory()[ar::getIndirectCallVar(call_stmt)];

      if (!ptr.is_pointer_var()) {
        throw analyzer_error(
            "inline_sym_exec_call: unexpected operand to call statement");
      }

      // reduction between value and pointer analysis: refine the set
      // of potential callees
      std::pair< PointerInfo::ptr_set_t, ikos::z_interval > ptr_info =
          ctx.pointer_info()[ptr.var()];

      ikos::ptr_domain_traits::refine_addrs(caller_inv,
                                            ptr.var(),
                                            ptr_info.first);

      if (!ikos::ptr_domain_traits::is_unknown_addr(caller_inv, ptr.var())) {
        ikos::discrete_domain< memloc_t > addrs_set =
            ikos::ptr_domain_traits::addrs_set< AbsDomain,
                                                memloc_t >(caller_inv,
                                                           ptr.var());
        for (auto it = addrs_set.begin(); it != addrs_set.end(); ++it) {
          callees.push_back(*it);
        }
      } else {
        // No points-to information
        if (warning_callback) {
          json_dict info = {{"type", "no-points-to"},
                            {"pointer", ptr.var()->name()}};
          warning_callback->warning("call",
                                    "indirect call cannot be resolved "
                                    "(analysis unsound if function has side "
                                    "effects)",
                                    call_stmt,
                                    info);
        }

        // ASSUMPTION: the callee has no side effects.
        // Just set lhs and all actual parameters of pointer type to TOP.
        sym_exec_t sym_exec(caller_inv,
                            ctx.var_factory(),
                            ctx.mem_factory(),
                            ctx.lit_factory(),
                            ctx.arch(),
                            ctx.prec_level(),
                            ctx.pointer_info(),
                            varname_set_t(),
                            varname_set_t(),
                            warning_callback);
        sym_exec.exec_external_call(lhs, call_stmt, "", actual_params);
        return sym_exec.inv();
      }
    } else { // direct call
      callees.push_back(ctx.mem_factory().get_function(
          arbos::ar::ar_internal::getFunctionAddr(call_stmt)));
    }

    assert(!callees.empty());

    /*
     * Compute the post invariant
     */

    // map from callee (function name) to function_analyzer_ptr_t
    callee_map_t& callee_map = _call_map[call_stmt.getUID()];

    // by default, propagate the exception states
    AbsDomain post = caller_inv;
    ikos::exc_domain_traits::set_normal_flow_bottom(post);

    bool resolved = false;

    json_list points_to_info;

    for (std::vector< memloc_t >::iterator it = callees.begin();
         it != callees.end();
         ++it) {
      std::ostringstream buf;
      ikos::index_traits< memory_location* >::write(buf, *it);
      const std::string& callee_s = buf.str();

      json_dict fun_info = {{"function", callee_s}};

      // Check if the callee is a function
      if (!isa< function_memory_location >(*it)) {
        // Not a call to a function memory location, emit a warning
        fun_info.put("type", "not-function");
        points_to_info.add(fun_info);
        continue;
      }

      boost::optional< Function_ref > callee = ctx[callee_s];

      if (!callee || ar::isExternal(*callee)) {
        // ASSUMPTION: if the function code is not available, treat it as
        // function call that has no side effects.
        sym_exec_t sym_exec(caller_inv,
                            ctx.var_factory(),
                            ctx.mem_factory(),
                            ctx.lit_factory(),
                            ctx.arch(),
                            ctx.prec_level(),
                            ctx.pointer_info(),
                            varname_set_t(),
                            varname_set_t(),
                            warning_callback);
        sym_exec.exec_external_call(lhs, call_stmt, callee_s, actual_params);
        post = post | sym_exec.inv();

        resolved = true;
        fun_info.put("type", "extern");
        points_to_info.add(fun_info);
      } else {
        bool is_va_arg = ar::isVarargs(*callee);
        IvRange formal_params = ar::getFormalParams(*callee);

        if ((!is_va_arg && actual_params.size() != formal_params.size()) ||
            (is_va_arg && actual_params.size() < formal_params.size())) {
          // ASSUMPTION: all function calls have been checked by the compiler
          // and are well-formed. In that case, it means this function cannot be
          // called and that it is just an imprecision of the pointer analysis.
          fun_info.put("type", "wrong-signature");
          points_to_info.add(fun_info);
          continue;
        }

        if (is_recursive(callee_s, analyzed_functions)) {
          if (warning_callback) {
            json_dict info = {{"type", "recursive"}, {"function", callee_s}};
            warning_callback->warning("call",
                                      "skipping safely recursive call to " +
                                          demangle(callee_s),
                                      call_stmt,
                                      info);
          }

          // TODO: we can be more precise by making top only lhs of call_stmt,
          // actual parameters of pointer type and any global variable
          // that might be touched by the recursive function.
          sym_exec_t sym_exec(caller_inv,
                              ctx.var_factory(),
                              ctx.mem_factory(),
                              ctx.lit_factory(),
                              ctx.arch(),
                              ctx.prec_level(),
                              ctx.pointer_info(),
                              varname_set_t(),
                              varname_set_t(),
                              warning_callback);
          sym_exec.exec_unknown_call(lhs, actual_params);
          return sym_exec.inv();
        }

        arbos_cfg callee_cfg = ctx[*callee];

        // match actual with formal parameters
        AbsDomain callee_entry_inv;
        try {
          callee_entry_inv = propagate_down(ctx,
                                            caller_inv,
                                            formal_params,
                                            actual_params,
                                            is_va_arg);
        } catch (type_error&) {
          fun_info.put("type", "wrong-signature");
          points_to_info.add(fun_info);
          continue;
        }

        // do not propagate exceptions from the caller to the callee
        ikos::exc_domain_traits::ignore_exceptions(callee_entry_inv);

        /*
         * Analyze recursively the callee
         */

        sym_exec_call_ptr_t callee_it = nullptr;

        if (is_context_stable && convergence_achieved) {
          // use the previously computed fix-point
          function_analyzer_ptr_t& callee_analyzer = callee_map.at(callee_s);

          // run checks
          callee_analyzer->check();

          callee_it = callee_analyzer->call_semantic();
        } else {
          // erase the previous fix-point
          callee_map.erase(callee_s);

          // compute a new fix-point on the callee
          function_names_t callee_analyzed_functions = analyzed_functions;
          callee_analyzed_functions.insert(callee_s);

          callee_it = sym_exec_call_ptr_t(
              new inline_sym_exec_call_t(this->_prec_level));

          function_analyzer_ptr_t callee_analyzer =
              std::make_unique< FunctionAnalyzer >(callee_cfg,
                                                   ctx,
                                                   callee_it,
                                                   is_context_stable &&
                                                       convergence_achieved,
                                                   caller,
                                                   call_context,
                                                   callee_analyzed_functions);

          if (!convergence_achieved) {
            std::cout << "*** Analyzing function: " << demangle(callee_s)
                      << std::endl;
          }

          // run analysis on callee
          callee_analyzer->run(callee_entry_inv);

          // insert in the callee map
          callee_map.emplace(callee_s, std::move(callee_analyzer));
        }

        inline_sym_exec_call_ptr_t inliner =
            std::static_pointer_cast< inline_sym_exec_call_t >(callee_it);
        AbsDomain callee_exit_inv = inliner->_exit_inv;
        boost::optional< Operand_ref > ret_val = inliner->_exit_op;

        if (ikos::exc_domain_traits::is_normal_flow_bottom(callee_exit_inv)) {
          post = post | callee_exit_inv; // collect the exception states

          resolved = true;
          fun_info.put("type", "internal");
          points_to_info.add(fun_info);
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

        try {
          post = post | propagate_up(ctx,
                                     callee_exit_inv,
                                     out_actuals,
                                     lhs,
                                     out_formals,
                                     ret_val,
                                     is_va_arg);
        } catch (type_error&) {
          fun_info.put("type", "wrong-signature");
          points_to_info.add(fun_info);
          continue;
        }

        resolved = true;
        fun_info.put("type", "internal");
        points_to_info.add(fun_info);
      }
    }

    if (!resolved) {
      // the points-to analysis was not precise enough.
      if (warning_callback) {
        json_dict info = {{"type", "no-match"}, {"points-to", points_to_info}};
        warning_callback->warning("call",
                                  "indirect call cannot be resolved (analysis "
                                  "unsound if function has side effects)",
                                  call_stmt,
                                  info);
      }

      // ASSUMPTION: the callee has no side effects.
      // Just set lhs and all actual parameters of pointer type to TOP.
      sym_exec_t sym_exec(caller_inv,
                          ctx.var_factory(),
                          ctx.mem_factory(),
                          ctx.lit_factory(),
                          ctx.arch(),
                          ctx.prec_level(),
                          ctx.pointer_info(),
                          varname_set_t(),
                          varname_set_t(),
                          warning_callback);
      sym_exec.exec_external_call(lhs, call_stmt, "", actual_params);
      return sym_exec.inv();
    }

    if (warning_callback) {
      json_dict info = {{"type", "ok"}, {"points-to", points_to_info}};
      warning_callback->warning("call",
                                "analyzing function call",
                                call_stmt,
                                info);
    }

    return post;
  }

  void ret(context& ctx,
           FunctionAnalyzer& analyzer,
           Return_Value_ref s,
           AbsDomain pre) {
    // Assume the code has only one return statement.
    // This is enforced by the UnifyFunctionExitNodesPass llvm pass.
    // Note that functions might not have a return statement.
    assert(!_exit_op || _exit_op == ar::getReturnValue(s));
    _exit_op = ar::getReturnValue(s);
  }

  void exit(context& ctx, FunctionAnalyzer& analyzer, AbsDomain inv) {
    // deallocate local variables
    sym_exec_t sym_exec(inv,
                        ctx.var_factory(),
                        ctx.mem_factory(),
                        ctx.lit_factory(),
                        ctx.arch(),
                        ctx.prec_level(),
                        ctx.pointer_info());
    sym_exec.deallocate_local_vars(ar::getLocalVars(analyzer.function()));
    _exit_inv = sym_exec.inv();
  }

private:
  inline bool is_recursive(const std::string& function_name,
                           const function_names_t& visited) {
    return visited.find(function_name) != visited.end();
  }

  AbsDomain propagate_down(context& ctx,
                           AbsDomain caller_inv,
                           IvRange formals,
                           OpRange actuals,
                           bool is_va_arg) {
    sym_exec_t sym_exec(caller_inv,
                        ctx.var_factory(),
                        ctx.mem_factory(),
                        ctx.lit_factory(),
                        ctx.arch(),
                        ctx.prec_level(),
                        ctx.pointer_info());
    sym_exec.match_down(formals, actuals, is_va_arg);
    return sym_exec.inv();
  }

  AbsDomain propagate_up(context& ctx,
                         AbsDomain callee,
                         OpRange actuals,
                         boost::optional< Internal_Variable_ref > lhs_cs_ret,
                         IvRange formals,
                         boost::optional< Operand_ref > callee_ret,
                         bool is_va_arg) {
    sym_exec_t sym_exec(callee,
                        ctx.var_factory(),
                        ctx.mem_factory(),
                        ctx.lit_factory(),
                        ctx.arch(),
                        ctx.prec_level(),
                        ctx.pointer_info());
    sym_exec.match_up(actuals, lhs_cs_ret, formals, callee_ret, is_va_arg);
    return sym_exec.inv();
  }
};

} // end namespace analyzer

#endif // ANALYZER_INLINER_HPP
