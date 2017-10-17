/*******************************************************************************
 *
 * Uninitialized variable checker.
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
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

#ifndef ANALYZER_UNINITIALIZED_VARIABLE_CHECKER_HPP
#define ANALYZER_UNINITIALIZED_VARIABLE_CHECKER_HPP

#include <array>
#include <vector>

#include <ikos/domains/uninitialized_domains_api.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/checkers/checker_api.hpp>

namespace analyzer {

using namespace arbos;

template < class AbsDomain >
class uninitialized_variable_checker : public checker< AbsDomain > {
private:
  typedef checker< AbsDomain > checker_t;

public:
  uninitialized_variable_checker(context& ctx,
                                 results_table_t& results_table,
                                 display_settings display_invariants,
                                 display_settings display_checks)
      : checker_t(ctx, results_table, display_invariants, display_checks) {}

  virtual const char* name() { return "uva"; }
  virtual const char* description() { return "Uninitialized variable checker"; }

  virtual void check(Arith_Op_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    check_binary_stmt(stmt, inv, call_context);
  }

  virtual void check(Integer_Comparison_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    check_binary_stmt(stmt, inv, call_context);
  }

  virtual void check(Bitwise_Op_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    check_binary_stmt(stmt, inv, call_context);
  }

  virtual void check(Conv_Op_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    check_unary_stmt(stmt, inv, call_context);
  }

  virtual void check(Assignment_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    check_unary_stmt(stmt, inv, call_context);
  }

  virtual void check(Pointer_Shift_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(stmt);
    boost::optional< Function_ref > fun = ar::getParentFunction(stmt);
    std::array< Operand_ref, 2 > ops = {
        {ar::getBase(stmt), ar::getOffset(stmt)}};
    check_initialized(ops.begin(),
                      ops.end(),
                      inv,
                      call_context,
                      fun,
                      loc,
                      ar::getUID(stmt));
  }

  virtual void check(Call_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    // Check all arguments and ignore the return value
    location loc = ar::getSrcLoc(stmt);
    boost::optional< Function_ref > fun = ar::getParentFunction(stmt);
    OpRange args = ar::getArguments(stmt);
    check_initialized(args.begin(),
                      args.end(),
                      inv,
                      call_context,
                      fun,
                      loc,
                      ar::getUID(stmt));
  }

  virtual void check(Invoke_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    check(ar::getFunctionCall(stmt), inv, call_context);
  }

  virtual void check(Store_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(stmt);
    boost::optional< Function_ref > fun = ar::getParentFunction(stmt);
    check_initialized(ar::getPointer(stmt),
                      inv,
                      call_context,
                      fun,
                      loc,
                      ar::getUID(stmt));
  }

  virtual void check(Load_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(stmt);
    boost::optional< Function_ref > fun = ar::getParentFunction(stmt);
    check_initialized(ar::getPointer(stmt),
                      inv,
                      call_context,
                      fun,
                      loc,
                      ar::getUID(stmt));
  }

  virtual void check(MemCpy_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(stmt);
    boost::optional< Function_ref > fun = ar::getParentFunction(stmt);
    std::array< Operand_ref, 3 > ops = {
        {ar::getTarget(stmt), ar::getSource(stmt), ar::getLen(stmt)}};
    check_initialized(ops.begin(),
                      ops.end(),
                      inv,
                      call_context,
                      fun,
                      loc,
                      ar::getUID(stmt));
  }

  virtual void check(MemMove_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(stmt);
    boost::optional< Function_ref > fun = ar::getParentFunction(stmt);
    std::array< Operand_ref, 3 > ops = {
        {ar::getTarget(stmt), ar::getSource(stmt), ar::getLen(stmt)}};
    check_initialized(ops.begin(),
                      ops.end(),
                      inv,
                      call_context,
                      fun,
                      loc,
                      ar::getUID(stmt));
  }

  virtual void check(MemSet_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(stmt);
    boost::optional< Function_ref > fun = ar::getParentFunction(stmt);
    std::array< Operand_ref, 3 > ops = {
        {ar::getBase(stmt), ar::getValue(stmt), ar::getLen(stmt)}};
    check_initialized(ops.begin(),
                      ops.end(),
                      inv,
                      call_context,
                      fun,
                      loc,
                      ar::getUID(stmt));
  }

  virtual void check(Return_Value_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    // Check the return value (if any)
    boost::optional< Operand_ref > r = ar::getReturnValue(stmt);
    if (r) {
      location loc = ar::getSrcLoc(stmt);
      boost::optional< Function_ref > fun = ar::getParentFunction(stmt);
      check_initialized(*r, inv, call_context, fun, loc, ar::getUID(stmt));
    }
  }

private:
  template < typename Binary_Statement_ref >
  void check_binary_stmt(Binary_Statement_ref stmt,
                         AbsDomain inv,
                         const std::string& call_context) {
    location loc = ar::getSrcLoc(stmt);
    boost::optional< Function_ref > fun = ar::getParentFunction(stmt);
    std::array< Operand_ref, 2 > ops = {
        {ar::getLeftOp(stmt), ar::getRightOp(stmt)}};
    check_initialized(ops.begin(),
                      ops.end(),
                      inv,
                      call_context,
                      fun,
                      loc,
                      ar::getUID(stmt));
  }

  template < typename Unary_Statement_ref >
  void check_unary_stmt(Unary_Statement_ref stmt,
                        AbsDomain inv,
                        const std::string& call_context) {
    location loc = ar::getSrcLoc(stmt);
    boost::optional< Function_ref > fun = ar::getParentFunction(stmt);
    check_initialized(ar::getRightOp(stmt),
                      inv,
                      call_context,
                      fun,
                      loc,
                      ar::getUID(stmt));
  }

  inline bool display_check(analysis_result result,
                            const scalar_lit_t& v,
                            const location& loc) {
    if (checker_t::display_check(result, loc)) {
      std::cerr << " check_initialized(" << v << ")";
      return true;
    }
    return false;
  }

  template < typename Iterator >
  void check_initialized(Iterator begin,
                         Iterator end,
                         AbsDomain inv,
                         const std::string& call_context,
                         const boost::optional< Function_ref >& function,
                         const location& loc,
                         unsigned long stmt_uid) {
    for (auto it = begin; it != end; ++it) {
      check_initialized(*it, inv, call_context, function, loc, stmt_uid);
    }
  }

  void check_initialized(Operand_ref op,
                         AbsDomain inv,
                         const std::string& call_context,
                         const boost::optional< Function_ref >& function,
                         const location& loc,
                         unsigned long stmt_uid) {
    LiteralFactory& lfac = this->_context.lit_factory();
    analysis_result result = WARNING;
    json_dict info;

    try {
      scalar_lit_t v = lfac[op];

      if (v.is_var()) {
        info.put("name", v.var()->name());
      }

      if (ikos::exc_domain_traits::is_normal_flow_bottom(inv)) {
        result = UNREACHABLE;
        info.clear();

        if (display_check(result, v, loc)) {
          std::cerr << std::endl;
        }
      } else if (ar::isAllocaVar(op)) {
        result = OK;
        info.put("type", "local-var");

        if (display_check(result, v, loc)) {
          std::cerr << std::endl;
        }
      } else if (ar::isGlobalVar(op)) {
        result = OK;
        info.put("type", "global-var");

        if (display_check(result, v, loc)) {
          std::cerr << std::endl;
        }
      } else if (ar::isFunctionPointer(op)) {
        result = OK;
        info.put("type", "function-pointer");

        if (display_check(result, v, loc)) {
          std::cerr << std::endl;
        }
      } else if (v.is_integer() || v.is_floating_point() || v.is_null()) {
        result = OK;
        info.put("type", "constant");

        if (display_check(result, v, loc)) {
          std::cerr << std::endl;
        }
      } else if (v.is_undefined()) {
        result = ERROR;
        info.put("type", "undefined-constant");

        if (display_check(result, v, loc)) {
          std::cerr << std::endl;
        }
      } else if (v.is_var()) {
        info.put("type", "var");

        if (ikos::uninit_domain_traits::is_uninitialized(inv, v.var())) {
          result = ERROR;

          if (display_check(result, v, loc)) {
            std::cerr << ": " << v.var()->name() << " is uninitialized"
                      << std::endl;
          }
        } else if (ikos::uninit_domain_traits::is_initialized(inv, v.var())) {
          result = OK;

          if (display_check(result, v, loc)) {
            std::cerr << ": " << v.var()->name() << " is initialized"
                      << std::endl;
          }
        } else {
          result = WARNING;

          if (display_check(result, v, loc)) {
            std::cerr << ": " << v.var()->name() << " may be uninitialized"
                      << std::endl;
          }
        }
      } else {
        throw analyzer_error(
            "uninitialized_variable_checker: unexpected operand");
      }
    } catch (aggregate_literal_error&) {
      const aggregate_lit_t& v = lfac.lookup_aggregate(op);
      result = OK; // do not report warnings/errors on structures/arrays

      if (v.is_cst() || v.is_undefined()) {
        info.put("type", "constant");
      } else if (v.is_var()) {
        info.put("type", "var");
        info.put("name", v.var()->name());
      } else {
        throw analyzer_error("unreachable");
      }
    } catch (void_var_literal_error& e) {
      result = OK;
      info.put("type", "var");
      info.put("name", e.var()->name());
    }

    this->display_invariant(result, inv, loc);
    this->_results
        .write("uva", result, call_context, function, loc, stmt_uid, info);
  }

}; // end class uninitialized_variable_checker

} // end namespace analyzer

#endif // ANALYZER_UNINITIALIZED_VARIABLE_CHECKER_HPP
