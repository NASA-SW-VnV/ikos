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
  typedef typename analysis_db::db_ptr db_ptr_t;

public:
  null_dereference_checker(context& ctx,
                           db_ptr_t db,
                           display_settings display_invariants,
                           display_settings display_checks)
      : checker_t(ctx, db, display_invariants, display_checks) {}

  virtual const char* name() { return "nullity"; }
  virtual const char* description() { return "Null dereference checker"; }

  virtual void check(Store_ref store,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(store);
    Operand_ref ptr = ar::getPointer(store);
    check_null_dereference(ptr, inv, call_context, loc, ar::getUID(store));
  }

  virtual void check(Load_ref load,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(load);
    Operand_ref ptr = ar::getPointer(load);
    check_null_dereference(ptr, inv, call_context, loc, ar::getUID(load));
  }

  virtual void check(MemCpy_ref memcpy,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(memcpy);
    Operand_ref ptr_src = ar::getSource(memcpy);
    Operand_ref ptr_dest = ar::getSource(memcpy);
    check_null_dereference(ptr_src, inv, call_context, loc, ar::getUID(memcpy));
    check_null_dereference(ptr_dest,
                           inv,
                           call_context,
                           loc,
                           ar::getUID(memcpy));
  }

  virtual void check(MemMove_ref memmove,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(memmove);
    Operand_ref ptr_src = ar::getSource(memmove);
    Operand_ref ptr_dest = ar::getSource(memmove);
    check_null_dereference(ptr_src,
                           inv,
                           call_context,
                           loc,
                           ar::getUID(memmove));
    check_null_dereference(ptr_dest,
                           inv,
                           call_context,
                           loc,
                           ar::getUID(memmove));
  }

  virtual void check(MemSet_ref memset,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(memset);
    Operand_ref ptr = ar::getBase(memset);
    check_null_dereference(ptr, inv, call_context, loc, ar::getUID(memset));
  }

  virtual void check(Call_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    if (ar::isDirectCall(stmt)) {
      location loc = ar::getSrcLoc(stmt);
      std::string fun_name = ar::getFunctionName(stmt);
      OpRange arguments = ar::getArguments(stmt);

      if ((fun_name == "strlen" && arguments.size() == 1) ||
          (fun_name == "strnlen" && arguments.size() == 2)) {
        check_null_dereference(arguments[0],
                               inv,
                               call_context,
                               loc,
                               ar::getUID(stmt));
      } else if ((fun_name == "strcpy" && arguments.size() == 2) ||
                 (fun_name == "strncpy" && arguments.size() == 3) ||
                 (fun_name == "strcat" && arguments.size() == 2) ||
                 (fun_name == "strncat" && arguments.size() == 3)) {
        check_null_dereference(arguments[0],
                               inv,
                               call_context,
                               loc,
                               ar::getUID(stmt));
        check_null_dereference(arguments[1],
                               inv,
                               call_context,
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
  void check_null_dereference(Operand_ref ptr,
                              AbsDomain inv,
                              const std::string& call_context,
                              const location& loc,
                              unsigned long stmt_uid) {
    // this is always a non-null dereference
    if (ar::isGlobalVar(ptr) || ar::isAllocaVar(ptr))
      return;

    if (exc_domain_traits::is_normal_flow_bottom(inv)) {
      if (this->display_check(UNREACHABLE)) {
        std::cout << location_to_string(loc)
                  << ": [unreachable] check_null_dereference(" << ptr << ")"
                  << std::endl;
      }
      if (this->display_invariant(UNREACHABLE)) {
        std::cout << location_to_string(loc) << ": Invariant:" << std::endl
                  << inv << std::endl;
      }

      this->_db->write("nullity",
                       call_context,
                       loc.file,
                       loc.line,
                       loc.column,
                       stmt_uid,
                       "unreachable");
      return;
    }

    LiteralFactory& lfac = this->_context.lit_factory();
    Literal ptr_lit = lfac[ptr];
    analysis_result res = WARNING;

    if (ptr_lit.is_var()) {
      varname_t ptr_var = ptr_lit.get_var();

      if (null_domain_traits::is_null(inv, ptr_var)) {
        if (this->display_check(ERR)) {
          std::cout << location_to_string(loc)
                    << ": [error] check_null_dereference(" << ptr
                    << "): ptr is null" << std::endl;
        }

        res = ERR;
      } else if (null_domain_traits::is_non_null(inv, ptr_var)) {
        if (this->display_check(OK)) {
          std::cout << location_to_string(loc)
                    << ": [ok] check_null_dereference(" << ptr
                    << "): ptr is non null" << std::endl;
        }

        res = OK;
      } else {
        if (this->display_check(WARNING)) {
          std::cout << location_to_string(loc)
                    << ": [warning] check_null_dereference(" << ptr
                    << "): ptr may be null" << std::endl;
        }

        res = WARNING;
      }
    } else if (ptr_lit.is_null_cst() || ptr_lit.is_undefined_cst()) {
      if (this->display_check(ERR)) {
        std::cout << location_to_string(loc)
                  << ": [error] check_null_dereference(" << ptr
                  << "): ptr is null or undefined" << std::endl;
      }

      res = ERR;
    } else {
      assert(false && "unexpected operand");
    }

    if (this->display_invariant(res)) {
      std::cout << location_to_string(loc) << ": Invariant:" << std::endl
                << inv << std::endl;
    }
    this->_db->write("nullity",
                     call_context,
                     loc.file,
                     loc.line,
                     loc.column,
                     stmt_uid,
                     tostr(res));
  }
}; // end class null_dereference_checker

} // end namespace analyzer

#endif // ANALYZER_NULL_DEREFERENCE_CHECKER_HPP
