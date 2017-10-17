/*******************************************************************************
 *
 * Null dereference checker.
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

#ifndef ANALYZER_NULL_DEREFERENCE_CHECKER_HPP
#define ANALYZER_NULL_DEREFERENCE_CHECKER_HPP

#include <ikos/domains/nullity_domains_api.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/checkers/checker_api.hpp>

namespace analyzer {

using namespace arbos;

template < class AbsDomain >
class null_dereference_checker : public checker< AbsDomain > {
private:
  typedef checker< AbsDomain > checker_t;

public:
  null_dereference_checker(context& ctx,
                           results_table_t& results_table,
                           display_settings display_invariants,
                           display_settings display_checks)
      : checker_t(ctx, results_table, display_invariants, display_checks) {}

  virtual const char* name() { return "nullity"; }
  virtual const char* description() { return "Null dereference checker"; }

  virtual void check(Store_ref store,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(store);
    boost::optional< Function_ref > fun = ar::getParentFunction(store);
    Operand_ref ptr = ar::getPointer(store);
    check_null_dereference(ptr, inv, call_context, fun, loc, ar::getUID(store));
  }

  virtual void check(Load_ref load,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(load);
    boost::optional< Function_ref > fun = ar::getParentFunction(load);
    Operand_ref ptr = ar::getPointer(load);
    check_null_dereference(ptr, inv, call_context, fun, loc, ar::getUID(load));
  }

  virtual void check(MemCpy_ref memcpy,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(memcpy);
    boost::optional< Function_ref > fun = ar::getParentFunction(memcpy);
    Operand_ref ptr_src = ar::getSource(memcpy);
    Operand_ref ptr_dest = ar::getTarget(memcpy);
    check_null_dereference(ptr_src,
                           inv,
                           call_context,
                           fun,
                           loc,
                           ar::getUID(memcpy));
    check_null_dereference(ptr_dest,
                           inv,
                           call_context,
                           fun,
                           loc,
                           ar::getUID(memcpy));
  }

  virtual void check(MemMove_ref memmove,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(memmove);
    boost::optional< Function_ref > fun = ar::getParentFunction(memmove);
    Operand_ref ptr_src = ar::getSource(memmove);
    Operand_ref ptr_dest = ar::getTarget(memmove);
    check_null_dereference(ptr_src,
                           inv,
                           call_context,
                           fun,
                           loc,
                           ar::getUID(memmove));
    check_null_dereference(ptr_dest,
                           inv,
                           call_context,
                           fun,
                           loc,
                           ar::getUID(memmove));
  }

  virtual void check(MemSet_ref memset,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(memset);
    boost::optional< Function_ref > fun = ar::getParentFunction(memset);
    Operand_ref ptr = ar::getBase(memset);
    check_null_dereference(ptr,
                           inv,
                           call_context,
                           fun,
                           loc,
                           ar::getUID(memset));
  }

  virtual void check(Call_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    if (ar::isDirectCall(stmt)) {
      location loc = ar::getSrcLoc(stmt);
      boost::optional< Function_ref > curr_fun = ar::getParentFunction(stmt);
      std::string fun_name = ar::getFunctionName(stmt);
      OpRange arguments = ar::getArguments(stmt);

      if ((fun_name == "strlen" && arguments.size() == 1) ||
          (fun_name == "strnlen" && arguments.size() == 2)) {
        check_null_dereference(arguments[0],
                               inv,
                               call_context,
                               curr_fun,
                               loc,
                               ar::getUID(stmt));
      } else if ((fun_name == "strcpy" && arguments.size() == 2) ||
                 (fun_name == "strncpy" && arguments.size() == 3) ||
                 (fun_name == "strcat" && arguments.size() == 2) ||
                 (fun_name == "strncat" && arguments.size() == 3)) {
        check_null_dereference(arguments[0],
                               inv,
                               call_context,
                               curr_fun,
                               loc,
                               ar::getUID(stmt));
        check_null_dereference(arguments[1],
                               inv,
                               call_context,
                               curr_fun,
                               loc,
                               ar::getUID(stmt));
      }
    }
  }

  virtual void check(Invoke_ref s,
                     AbsDomain inv,
                     const std::string& call_context) {
    check(ar::getFunctionCall(s), inv, call_context);
  }

private:
  inline bool display_check(analysis_result result,
                            const scalar_lit_t& ptr,
                            const location& loc) {
    if (checker_t::display_check(result, loc)) {
      std::cerr << " check_null_dereference(" << ptr << ")";
      return true;
    }
    return false;
  }

  void check_null_dereference(Operand_ref ptr_op,
                              AbsDomain inv,
                              const std::string& call_context,
                              const boost::optional< Function_ref >& function,
                              const location& loc,
                              unsigned long stmt_uid) {
    LiteralFactory& lfac = this->_context.lit_factory();
    scalar_lit_t ptr = lfac[ptr_op];
    analysis_result result = WARNING;
    json_dict info;

    if (ptr.is_var()) {
      info.put("name", ptr.var()->name());
    }

    if (ikos::exc_domain_traits::is_normal_flow_bottom(inv)) {
      result = UNREACHABLE;
      info.clear();

      if (display_check(result, ptr, loc)) {
        std::cerr << std::endl;
      }
    } else if (ar::isAllocaVar(ptr_op)) {
      result = OK;
      info.put("type", "local-var");

      if (display_check(result, ptr, loc)) {
        std::cerr << std::endl;
      }
    } else if (ar::isGlobalVar(ptr_op)) {
      result = OK;
      info.put("type", "global-var");

      if (display_check(result, ptr, loc)) {
        std::cerr << std::endl;
      }
    } else if (ar::isFunctionPointer(ptr_op)) {
      result = ERROR;
      info.put("type", "function-pointer");

      if (display_check(result, ptr, loc)) {
        std::cerr << ": dereferencing a function pointer" << std::endl;
      }
    } else {
      if (ptr.is_null()) {
        result = ERROR;
        info.put("type", "null-constant");

        if (display_check(result, ptr, loc)) {
          std::cerr << ": ptr is null" << std::endl;
        }
      } else if (ptr.is_undefined()) {
        result = ERROR;
        info.put("type", "undefined-constant");

        if (display_check(result, ptr, loc)) {
          std::cerr << ": ptr is uninitialized" << std::endl;
        }
      } else if (ptr.is_pointer_var()) {
        info.put("type", "var");

        if (ikos::null_domain_traits::is_null(inv, ptr.var())) {
          result = ERROR;

          if (display_check(result, ptr, loc)) {
            std::cerr << ": ptr is null" << std::endl;
          }
        } else if (ikos::null_domain_traits::is_non_null(inv, ptr.var())) {
          result = OK;

          if (display_check(result, ptr, loc)) {
            std::cerr << ": ptr is non null" << std::endl;
          }
        } else {
          result = WARNING;

          if (display_check(result, ptr, loc)) {
            std::cerr << ": ptr may be null" << std::endl;
          }
        }
      } else {
        throw analyzer_error("null_dereference_checker: unexpected operand");
      }
    }

    this->display_invariant(result, inv, loc);
    this->_results
        .write("nullity", result, call_context, function, loc, stmt_uid, info);
  }

}; // end class null_dereference_checker

} // end namespace analyzer

#endif // ANALYZER_NULL_DEREFERENCE_CHECKER_HPP
