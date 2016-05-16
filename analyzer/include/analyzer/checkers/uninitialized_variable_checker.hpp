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

#ifndef ANALYZER_UNINITIALIZED_VARIABLE_CHECKER_HPP
#define ANALYZER_UNINITIALIZED_VARIABLE_CHECKER_HPP

#include <vector>

#include <analyzer/analysis/common.hpp>
#include <analyzer/checkers/checker_api.hpp>
#include <analyzer/analysis/num_sym_exec.hpp>

namespace analyzer {

using namespace arbos;

template < class AbsDomain >
class uninitialized_variable_checker : public checker< AbsDomain > {
private:
  typedef checker< AbsDomain > checker_t;
  typedef typename analysis_db::db_ptr db_ptr_t;

public:
  uninitialized_variable_checker(context& ctx, db_ptr_t db)
      : checker_t(ctx, db) {}

  virtual const char* name() { return "uva"; }
  virtual const char* description() { return "Uninitialized variable checker"; }

  virtual void check(Arith_Op_ref s,
                     AbsDomain inv,
                     const std::string& call_context) {
    check_binary_stmt(s,
                      inv,
                      call_context); // check the two operands of the rhs
  }

  virtual void check(Integer_Comparison_ref s,
                     AbsDomain inv,
                     const std::string& call_context) {
    check_binary_stmt(s, inv, call_context); // check the two operands
  }

  virtual void check(Bitwise_Op_ref s,
                     AbsDomain inv,
                     const std::string& call_context) {
    check_binary_stmt(s,
                      inv,
                      call_context); // check the two operands of the rhs
  }

  virtual void check(Conv_Op_ref s,
                     AbsDomain inv,
                     const std::string& call_context) {
    check_unary_stmt(s, inv, call_context); // check the rhs
  }

  virtual void check(Assignment_ref s,
                     AbsDomain inv,
                     const std::string& call_context) {
    check_unary_stmt(s, inv, call_context); // check the rhs
  }

  virtual void check(Pointer_Shift_ref s,
                     AbsDomain inv,
                     const std::string& call_context) {
    // Both result and base of s are pointers so they are
    // ignored. Check only offset.
    std::vector< varname_t > used;
    if (is_checkable(ar::getOffset(s))) {
      Literal l = this->_context.lit_factory()[ar::getOffset(s)];
      assert(l.is_var());
      used.push_back(l.get_var());
    }
    check(inv, call_context, ar::getSrcLoc(s), used.begin(), used.end());
  }

  virtual void check(Call_ref s,
                     AbsDomain inv,
                     const std::string& call_context) {
    // Check all arguments and ignore the return value
    std::vector< varname_t > used;
    OpRange args = ar::getArguments(s);
    for (OpRange::iterator it = args.begin(); it != args.end(); ++it) {
      if (is_checkable(*it)) {
        Literal l = this->_context.lit_factory()[*it];
        assert(l.is_var());
        used.push_back(l.get_var());
      }
    }
    check(inv, call_context, ar::getSrcLoc(s), used.begin(), used.end());
  }

  virtual void check(Invoke_ref s,
                     AbsDomain inv,
                     const std::string& call_context) {
    check(ar::getFunctionCall(s), inv, call_context);
  }

  virtual void check(MemCpy_ref s,
                     AbsDomain inv,
                     const std::string& call_context) {
    // Both source and target are pointers so they are
    // ignored. Check length.
    std::vector< varname_t > used;
    if (is_checkable(ar::getLen(s))) {
      Literal l = this->_context.lit_factory()[ar::getLen(s)];
      assert(l.is_var());
      used.push_back(l.get_var());
    }
    check(inv, call_context, ar::getSrcLoc(s), used.begin(), used.end());
  }

  virtual void check(MemMove_ref s,
                     AbsDomain inv,
                     const std::string& call_context) {
    // Both source and target are pointers so they are
    // ignored. check only length.
    std::vector< varname_t > used;
    if (is_checkable(ar::getLen(s))) {
      Literal l = this->_context.lit_factory()[ar::getLen(s)];
      assert(l.is_var());
      used.push_back(l.get_var());
    }
    check(inv, call_context, ar::getSrcLoc(s), used.begin(), used.end());
  }

  virtual void check(MemSet_ref s,
                     AbsDomain inv,
                     const std::string& call_context) {
    // Base is a pointer so it is ignored. check value and length.
    std::vector< varname_t > used;
    if (is_checkable(ar::getValue(s))) {
      Literal l = this->_context.lit_factory()[ar::getValue(s)];
      assert(l.is_var());
      used.push_back(l.get_var());
    }
    if (is_checkable(ar::getLen(s))) {
      Literal l = this->_context.lit_factory()[ar::getLen(s)];
      assert(l.is_var());
      used.push_back(l.get_var());
    }
    check(inv, call_context, ar::getSrcLoc(s), used.begin(), used.end());
  }

  virtual void check(Return_Value_ref s,
                     AbsDomain inv,
                     const std::string& call_context) {
    // Check the return value (if any)
    std::vector< varname_t > used;
    boost::optional< Operand_ref > r = ar::getReturnValue(s);
    if (r) {
      if (is_checkable(*r)) {
        Literal l = this->_context.lit_factory()[*r];
        assert(l.is_var());
        used.push_back(l.get_var());
      }
      check(inv, call_context, ar::getSrcLoc(s), used.begin(), used.end());
    }
  }

private:
  //! return true if o is an integer scalar
  static bool is_checkable(const Operand_ref& o) {
    return ar::isIntegerRegVar(o);
  }

  //! return true if v is an integer scalar
  static bool is_checkable(const Internal_Variable_ref& v) {
    return !ar::isPointer(v) && ar::isInteger(v);
  }

  template < typename Iterator >
  void check(AbsDomain inv,
             const std::string& call_context,
             location loc,
             Iterator begin,
             Iterator end) {
    // {begin ... end-1} is a range of varname_t elements

    /// Check whether each variable is definitely initialized.
    if (std::distance(begin, end) > 0) {
      if (inv.is_bottom()) {
        this->_db->write("uva",
                         call_context,
                         loc.first,
                         loc.second,
                         "unreachable");
      } else {
        bool safe = true;
        bool unsafe = false;

        for (Iterator it = begin; it != end; ++it) {
          varname_t v = *it;
          safe &= value_domain_impl::is_initialized(inv, v);
          unsafe |= value_domain_impl::is_uninitialized(inv, v);

#ifdef DISPLAY_CHECKS
          if (!value_domain_impl::is_initialized(inv, v) &&
              !value_domain_impl::is_uninitialized(inv, v)) {
            std::cout << v << " may be uninitialized." << std::endl;
          }
          if (unsafe) {
            std::cout << v << " is uninitialized" << std::endl;
          }
#endif
        }
        if (safe) {
          this->_db->write("uva", call_context, loc.first, loc.second, "ok");
#ifdef DISPLAY_CHECKS
          std::cout << "safe"
                    << "|" << loc.first << "|" << loc.second << std::endl;
#endif
        } else if (unsafe) {
          this->_db->write("uva", call_context, loc.first, loc.second, "error");
#ifdef DISPLAY_CHECKS
          std::cout << "unsafe"
                    << "|" << loc.first << "|" << loc.second << std::endl;
#endif
        } else {
          this->_db->write("uva",
                           call_context,
                           loc.first,
                           loc.second,
                           "warning");
#ifdef DISPLAY_CHECKS
          std::cout << "warning"
                    << "|" << loc.first << "|" << loc.second << std::endl;
#endif
        }
      }
    }
  }

  template < typename Binary_Statement_ref >
  void check_binary_stmt(Binary_Statement_ref s,
                         AbsDomain inv,
                         const std::string& call_context) {
    std::vector< varname_t > used;
    if (is_checkable(ar::getLeftOp(s))) {
      Literal l = this->_context.lit_factory()[ar::getLeftOp(s)];
      assert(l.is_var());
      used.push_back(l.get_var());
    }
    if (is_checkable(ar::getRightOp(s))) {
      Literal l = this->_context.lit_factory()[ar::getRightOp(s)];
      assert(l.is_var());
      used.push_back(l.get_var());
    }

    check(inv, call_context, ar::getSrcLoc(s), used.begin(), used.end());
  }

  template < typename Unary_Statement_ref >
  void check_unary_stmt(Unary_Statement_ref s,
                        AbsDomain inv,
                        const std::string& call_context) {
    std::vector< varname_t > used;
    if (is_checkable(ar::getRightOp(s))) {
      Literal l = this->_context.lit_factory()[ar::getRightOp(s)];
      assert(l.is_var());
      used.push_back(l.get_var());
    }

    check(inv, call_context, ar::getSrcLoc(s), used.begin(), used.end());
  }
}; // end class uninitialized_variable_checker

} // end namespace analyzer

#endif // ANALYZER_UNINITIALIZED_VARIABLE_CHECKER_HPP
