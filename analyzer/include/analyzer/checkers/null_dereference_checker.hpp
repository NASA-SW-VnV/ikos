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

#include <analyzer/analysis/common.hpp>
#include <analyzer/checkers/checker_api.hpp>
#include <analyzer/analysis/num_sym_exec.hpp>

namespace analyzer {

using namespace arbos;

template < class AbsDomain >
class null_dereference_checker : public checker< AbsDomain > {
private:
  typedef checker< AbsDomain > checker_t;
  typedef typename analysis_db::db_ptr db_ptr_t;

public:
  null_dereference_checker(context& ctx, db_ptr_t db) : checker_t(ctx, db) {}

  virtual const char* name() { return "nullity"; }
  virtual const char* description() { return "Null dereference checker"; }

  virtual void check(Store_ref store,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(store);
    Operand_ref ptr = ar::getPointer(store);
    check_null_dereference(ptr, inv, call_context, loc);
  }

  virtual void check(Load_ref load,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(load);
    Operand_ref ptr = ar::getPointer(load);
    check_null_dereference(ptr, inv, call_context, loc);
  }

  virtual void check(MemCpy_ref memcpy,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(memcpy);
    Operand_ref ptr_src = ar::getSource(memcpy);
    check_null_dereference(ptr_src, inv, call_context, loc);
    Operand_ref ptr_dest = ar::getSource(memcpy);
    check_null_dereference(ptr_dest, inv, call_context, loc);
  }

  virtual void check(MemMove_ref memmove,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(memmove);
    Operand_ref ptr_src = ar::getSource(memmove);
    check_null_dereference(ptr_src, inv, call_context, loc);
    Operand_ref ptr_dest = ar::getSource(memmove);
    check_null_dereference(ptr_dest, inv, call_context, loc);
  }

  virtual void check(MemSet_ref memset,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(memset);
    Operand_ref ptr = ar::getBase(memset);
    check_null_dereference(ptr, inv, call_context, loc);
  }

private:
  void check_null_dereference(Operand_ref ptr,
                              AbsDomain inv,
                              const std::string& call_context,
                              location loc) {
    // this is always a non-null dereference
    if (ar::isGlobalVar(ptr) || ar::isAllocaVar(ptr))
      return;

    if (inv.is_bottom()) {
      this->_db->write("nullity",
                       call_context,
                       loc.first,
                       loc.second,
                       "unreachable");
      return;
    }

    LiteralFactory& lfac = this->_context.lit_factory();
    Literal ptr_lit = lfac[ptr];
    varname_t ptr_var = ptr_lit.get_var();

    analysis_result res = WARNING;
    if (ptr_lit.is_null_cst() || ptr_lit.is_undefined_cst() ||
        value_domain_impl::is_null(inv, ptr_var)) {
      res = ERR;
    } else if (value_domain_impl::is_non_null(inv, ptr_var)) {
      res = OK;
    }

    if (res == OK) {
      this->_db->write("nullity", call_context, loc.first, loc.second, "ok");
#ifdef DISPLAY_CHECKS
      std::cout << "safe"
                << "|" << loc.first << "|" << loc.second << std::endl;
#endif
    } else if (res == ERR) {
      this->_db->write("nullity", call_context, loc.first, loc.second, "error");
#ifdef DISPLAY_CHECKS
      std::cout << "unsafe"
                << "|" << loc.first << "|" << loc.second << std::endl;
      std::cout << "Null dereference " << ptr << "\n\t" << inv << std::endl;
#endif
    } else {
      this->_db->write("nullity",
                       call_context,
                       loc.first,
                       loc.second,
                       "warning");
#ifdef DISPLAY_CHECKS
      std::cout << "warning"
                << "|" << loc.first << "|" << loc.second << std::endl;
      std::cout << "May null dereference " << ptr << "\n\t" << inv << std::endl;
#endif
    }
  }
}; // end class null_dereference_checker

} // end namespace analyzer

#endif // ANALYZER_NULL_DEREFERENCE_CHECKER_HPP
