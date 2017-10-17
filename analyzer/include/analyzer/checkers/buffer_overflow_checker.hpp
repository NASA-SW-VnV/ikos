/*******************************************************************************
 *
 * Buffer overflow checker.
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

#ifndef ANALYZER_BUFFER_OVERFLOW_CHECKER_HPP
#define ANALYZER_BUFFER_OVERFLOW_CHECKER_HPP

#include <analyzer/analysis/common.hpp>
#include <analyzer/analysis/num_sym_exec.hpp>
#include <analyzer/analysis/variable_name.hpp>
#include <analyzer/checkers/checker_api.hpp>

namespace analyzer {

using namespace arbos;

template < class AbsDomain >
class buffer_overflow_checker : public checker< AbsDomain > {
private:
  typedef checker< AbsDomain > checker_t;

  typedef typename AbsDomain::variable_t variable_t;
  typedef typename AbsDomain::number_t number_t;
  typedef typename ikos::interval< number_t > interval_t;
  typedef typename AbsDomain::linear_expression_t linear_expression_t;

public:
  buffer_overflow_checker(context& ctx,
                          results_table_t& results_table,
                          display_settings display_invariants,
                          display_settings display_checks)
      : checker_t(ctx, results_table, display_invariants, display_checks) {}

  virtual const char* name() { return "boa"; }
  virtual const char* description() { return "Buffer overflow checker"; }

  virtual void check(Store_ref store,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(store);
    boost::optional< Function_ref > fun = ar::getParentFunction(store);
    Operand_ref ptr = ar::getPointer(store);
    number_t store_size = ar::getSize(ar::getPointeeType(ar::getType(ptr)));
    check_mem_access(ptr,
                     scalar_lit_t::integer(store_size),
                     inv,
                     call_context,
                     fun,
                     loc,
                     ar::getUID(store));
  }

  virtual void check(Load_ref load,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(load);
    Operand_ref ptr = ar::getPointer(load);
    boost::optional< Function_ref > fun = ar::getParentFunction(load);
    number_t load_size = ar::getSize(ar::getPointeeType(ar::getType(ptr)));
    check_mem_access(ptr,
                     scalar_lit_t::integer(load_size),
                     inv,
                     call_context,
                     fun,
                     loc,
                     ar::getUID(load));
  }

  virtual void check(MemCpy_ref memcpy,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(memcpy);
    boost::optional< Function_ref > fun = ar::getParentFunction(memcpy);
    LiteralFactory& lfac = this->_context.lit_factory();
    Operand_ref dest_op = ar::getTarget(memcpy);
    Operand_ref src_op = ar::getSource(memcpy);
    scalar_lit_t len = lfac[ar::getLen(memcpy)];
    check_mem_access(dest_op,
                     len,
                     inv,
                     call_context,
                     fun,
                     loc,
                     ar::getUID(memcpy));
    check_mem_access(src_op,
                     len,
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
    LiteralFactory& lfac = this->_context.lit_factory();
    Operand_ref dest_op = ar::getTarget(memmove);
    Operand_ref src_op = ar::getSource(memmove);
    scalar_lit_t len = lfac[ar::getLen(memmove)];
    check_mem_access(src_op,
                     len,
                     inv,
                     call_context,
                     fun,
                     loc,
                     ar::getUID(memmove));
    check_mem_access(dest_op,
                     len,
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
    LiteralFactory& lfac = this->_context.lit_factory();
    Operand_ref dest_op = ar::getBase(memset);
    scalar_lit_t len = lfac[ar::getLen(memset)];
    check_mem_access(dest_op,
                     len,
                     inv,
                     call_context,
                     fun,
                     loc,
                     ar::getUID(memset));
  }

  virtual void check(Call_ref call,
                     AbsDomain inv,
                     const std::string& call_context) {
    if (ar::isDirectCall(call)) {
      location loc = ar::getSrcLoc(call);
      boost::optional< Function_ref > curr_fun = ar::getParentFunction(call);
      LiteralFactory& lfac = this->_context.lit_factory();
      std::string fun_name = ar::getFunctionName(call);
      OpRange arguments = ar::getArguments(call);

      /*
       * IKOS does not keep track of the string length (which is different from
       * the allocated size), thus it is hard to check if these function calls
       * are safe or not.
       *
       * In most cases here, we just check if the first byte is accessible.
       */
      if (fun_name == "strlen" && arguments.size() == 1) {
        check_mem_access(arguments[0],
                         scalar_lit_t::integer(1),
                         inv,
                         call_context,
                         curr_fun,
                         loc,
                         ar::getUID(call));
      } else if (fun_name == "strnlen" && arguments.size() == 2) {
        scalar_lit_t n = lfac[arguments[1]];
        if (is_greater_equal(n, 1, inv)) {
          check_mem_access(arguments[0],
                           scalar_lit_t::integer(1),
                           inv,
                           call_context,
                           curr_fun,
                           loc,
                           ar::getUID(call));
        }
      } else if (fun_name == "strcpy" && arguments.size() == 2) {
        check_mem_access(arguments[0],
                         scalar_lit_t::integer(1),
                         inv,
                         call_context,
                         curr_fun,
                         loc,
                         ar::getUID(call));
        check_mem_access(arguments[1],
                         scalar_lit_t::integer(1),
                         inv,
                         call_context,
                         curr_fun,
                         loc,
                         ar::getUID(call));
        check_strcpy(arguments[0],
                     arguments[1],
                     inv,
                     call_context,
                     curr_fun,
                     loc,
                     ar::getUID(call));
      } else if (fun_name == "strncpy" && arguments.size() == 3) {
        scalar_lit_t n = lfac[arguments[2]];
        if (is_greater_equal(n, 1, inv)) {
          check_mem_access(arguments[0],
                           scalar_lit_t::integer(1),
                           inv,
                           call_context,
                           curr_fun,
                           loc,
                           ar::getUID(call));
          check_mem_access(arguments[1],
                           scalar_lit_t::integer(1),
                           inv,
                           call_context,
                           curr_fun,
                           loc,
                           ar::getUID(call));
        }
        // TODO: check_strncpy
      } else if (fun_name == "strcat" && arguments.size() == 2) {
        check_mem_access(arguments[0],
                         scalar_lit_t::integer(1),
                         inv,
                         call_context,
                         curr_fun,
                         loc,
                         ar::getUID(call));
        check_mem_access(arguments[1],
                         scalar_lit_t::integer(1),
                         inv,
                         call_context,
                         curr_fun,
                         loc,
                         ar::getUID(call));
      } else if (fun_name == "strncat" && arguments.size() == 3) {
        scalar_lit_t n = lfac[arguments[2]];
        if (is_greater_equal(n, 1, inv)) {
          check_mem_access(arguments[0],
                           scalar_lit_t::integer(1),
                           inv,
                           call_context,
                           curr_fun,
                           loc,
                           ar::getUID(call));
          check_mem_access(arguments[1],
                           scalar_lit_t::integer(1),
                           inv,
                           call_context,
                           curr_fun,
                           loc,
                           ar::getUID(call));
        }
      }
    }
  }

  virtual void check(Invoke_ref s,
                     AbsDomain inv,
                     const std::string& call_context) {
    check(ar::getFunctionCall(s), inv, call_context);
  }

private:
  // Return true if lit >= n
  static bool is_greater_equal(const scalar_lit_t& lit, int n, AbsDomain inv) {
    if (lit.is_undefined()) {
      return false;
    } else if (lit.is_integer()) {
      return lit.integer() >= n;
    } else if (lit.is_integer_var()) {
      if (ikos::exc_domain_traits::is_normal_flow_bottom(inv))
        return false;

      inv += (variable_t(lit.var()) <= n - 1);
      return ikos::exc_domain_traits::is_normal_flow_bottom(inv);
    } else {
      throw analyzer_error("unreachable");
    }
  }

  inline bool display_mem_access_check(analysis_result result,
                                       const scalar_lit_t& ptr,
                                       const scalar_lit_t& access_size,
                                       const location& loc) {
    if (checker_t::display_check(result, loc)) {
      std::cerr << " check_mem_access(ptr=" << ptr
                << ", access_size=" << access_size << ")";
      return true;
    }
    return false;
  }

  /*
   * Check a memory access (read/write) for overflow/underflow.
   *
   * Arguments:
   *   ptr_op: The pointer operand
   *   access_size: The read/written size (in bytes), as a literal
   *   inv: The invariant, as an abstract value
   *   call_context: The calling context
   *   function: The current function
   *   loc: The source location
   *   stmt_uid: The UID of the AR_Statement
   *
   * Description:
   *   The method checks that the memory access is valid and writes the result
   *   in the database.
   */
  void check_mem_access(Operand_ref ptr_op,
                        const scalar_lit_t& access_size,
                        AbsDomain inv,
                        const std::string& call_context,
                        const boost::optional< Function_ref > function,
                        const location& loc,
                        unsigned long stmt_uid) {
    LiteralFactory& lfac = this->_context.lit_factory();
    scalar_lit_t ptr = lfac[ptr_op];
    analysis_result overflow_result = WARNING, underflow_result = WARNING;
    json_dict overflow_info, underflow_info;

    // add info
    if (ptr.is_pointer_var()) {
      overflow_info.put("pointer", ptr.var()->name());
    }
    if (access_size.is_integer_var()) {
      overflow_info.put("access-size", access_size.var()->name());
    } else if (access_size.is_integer()) {
      overflow_info.put("access-size", access_size.integer());
    } else if (!access_size.is_undefined()) {
      throw analyzer_error("buffer_overflow_checker: unexpected acces size");
    }
    underflow_info = overflow_info;

    if (ikos::exc_domain_traits::is_normal_flow_bottom(inv)) {
      overflow_result = underflow_result = UNREACHABLE;
      overflow_info.clear();
      underflow_info.clear();

      if (display_mem_access_check(overflow_result, ptr, access_size, loc)) {
        std::cerr << std::endl;
      }
    } else if (ar::isGlobalVar(ptr_op)) {
      overflow_result = underflow_result = OK;
      overflow_info.put("type", "global-var");
      underflow_info = overflow_info;

      if (display_mem_access_check(overflow_result, ptr, access_size, loc)) {
        std::cerr << ": ptr is a pointer on a global variable" << std::endl;
      }
    } else if (ar::isFunctionPointer(ptr_op)) {
      overflow_result = underflow_result = ERROR;
      overflow_info.put("type", "function-pointer");
      underflow_info = overflow_info;

      if (display_mem_access_check(overflow_result, ptr, access_size, loc)) {
        std::cerr << ": ptr is a function pointer" << std::endl;
      }
    } else {
      if (ptr.is_null() ||
          (ptr.is_pointer_var() &&
           ikos::null_domain_traits::is_null(inv, ptr.var()))) {
        overflow_result = underflow_result = ERROR;
        if (ptr.is_null()) {
          overflow_info.put("type", "null-constant-pointer");
        } else {
          overflow_info.put("type", "null-var-pointer");
        }
        underflow_info = overflow_info;

        if (display_mem_access_check(overflow_result, ptr, access_size, loc)) {
          std::cerr << ": ptr is null" << std::endl;
        }
      } else if (ptr.is_undefined() ||
                 (ptr.is_pointer_var() &&
                  ikos::uninit_domain_traits::is_uninitialized(inv,
                                                               ptr.var()))) {
        overflow_result = underflow_result = ERROR;
        if (ptr.is_undefined()) {
          overflow_info.put("type", "undefined-constant-pointer");
        } else {
          overflow_info.put("type", "undefined-var-pointer");
        }
        underflow_info = overflow_info;

        if (display_mem_access_check(overflow_result, ptr, access_size, loc)) {
          std::cerr << ": ptr is uninitialized" << std::endl;
        }
      } else if (access_size.is_undefined() ||
                 (access_size.is_integer_var() &&
                  ikos::uninit_domain_traits::is_uninitialized(inv,
                                                               access_size
                                                                   .var()))) {
        overflow_result = underflow_result = ERROR;
        if (access_size.is_undefined()) {
          overflow_info.put("type", "undefined-constant-access-size");
        } else {
          overflow_info.put("type", "undefined-var-access-size");
        }

        underflow_info = overflow_info;

        if (display_mem_access_check(overflow_result, ptr, access_size, loc)) {
          std::cerr << ": access_size is uninitialized" << std::endl;
        }
      } else {
        if (!ptr.is_pointer_var()) {
          throw analyzer_error(
              "buffer_overflow_checker: pointer is not a pointer variable");
        }

        linear_expression_t offset_expr = variable_t(inv.offset_var(ptr.var()));
        linear_expression_t access_size_expr =
            access_size.is_integer_var()
                ? linear_expression_t(access_size.var())
                : linear_expression_t(access_size.integer());

        overflow_result = check_overflow(ptr.var(),
                                         offset_expr,
                                         access_size_expr,
                                         inv,
                                         loc,
                                         overflow_info);
        underflow_result = check_underflow(ptr.var(),
                                           offset_expr,
                                           access_size_expr,
                                           inv,
                                           loc,
                                           underflow_info);
      }
    }

    if (this->display_invariant(overflow_result) ||
        this->display_invariant(underflow_result)) {
      std::cerr << location_to_string(loc) << ": Invariant:" << std::endl
                << inv << std::endl;
    }

    this->_results.write("overflow",
                         overflow_result,
                         call_context,
                         function,
                         loc,
                         stmt_uid,
                         overflow_info);
    this->_results.write("underflow",
                         underflow_result,
                         call_context,
                         function,
                         loc,
                         stmt_uid,
                         underflow_info);
  }

  inline bool display_overflow_check(analysis_result result,
                                     varname_t ptr,
                                     linear_expression_t offset,
                                     linear_expression_t access_size,
                                     const location& loc) {
    if (checker_t::display_check(result, loc)) {
      std::cerr << " check_overflow(ptr=" << ptr->str() << ", offset=" << offset
                << ", access_size=" << access_size << ")";
      return true;
    }
    return false;
  }

  /*
   * Check a memory access (read/write) for overflow
   *
   * Arguments:
   *   ptr: The pointer variable
   *   offset: A linear expression for the offset of the pointer (in bytes)
   *   access_size: A linear expression for the read/written size (in bytes)
   *   inv: The invariant, as an abstract value
   *   loc: The source location
   *
   * Returns:
   *   The analysis result (OK, WARNING, ERROR, UNREACHABLE)
   *
   * Description:
   *   The method checks that the memory access to
   *   [offset, offset + access_size - 1] is valid.
   *
   *   It checks the following property:
   *     addrs_set(ptr) != TOP &&
   *     ∀a ∈ addrs_set(ptr), ∀o ∈ offset, o + access_size <= a.size
   */
  analysis_result check_overflow(varname_t ptr,
                                 linear_expression_t offset,
                                 linear_expression_t access_size,
                                 AbsDomain inv,
                                 const location& loc,
                                 json_dict& info) {
    if (ikos::ptr_domain_traits::is_unknown_addr(inv, ptr)) {
      if (display_overflow_check(WARNING, ptr, offset, access_size, loc)) {
        std::cerr << ": no points-to information for " << ptr->str()
                  << std::endl;
      }

      info.put("type", "no-points-to");
      return WARNING;
    } else {
      ikos::discrete_domain< memloc_t > addrs_set =
          ikos::ptr_domain_traits::addrs_set< AbsDomain, memloc_t >(inv, ptr);
      assert(!addrs_set.is_top());
      bool all_valid = true;
      bool all_invalid = true;
      json_list points_to_info;

      for (auto it = addrs_set.begin(); it != addrs_set.end(); ++it) {
        varname_t size_var =
            sym_exec_traits::alloc_size_var(this->_context.var_factory(), *it);

        if (all_valid) {
          AbsDomain tmp(inv);
          tmp += (offset + access_size - 1 >= variable_t(size_var));
          bool is_bottom = ikos::exc_domain_traits::is_normal_flow_bottom(tmp);

          // display checks
          if (is_bottom &&
              display_overflow_check(OK, ptr, offset, access_size, loc)) {
            std::cerr << ": ∀o ∈ offset, o + access_size <= " << size_var->str()
                      << std::endl;
          } else if (!is_bottom && display_overflow_check(WARNING,
                                                          ptr,
                                                          offset,
                                                          access_size,
                                                          loc)) {
            std::cerr << ": ∃o ∈ offset, o + access_size > " << size_var->str()
                      << std::endl;
          }

          all_valid = all_valid && is_bottom;
          all_invalid = all_invalid && !is_bottom;
        }

        if (all_invalid) {
          AbsDomain tmp(inv);
          tmp += (offset + access_size <= variable_t(size_var));
          bool is_bottom = ikos::exc_domain_traits::is_normal_flow_bottom(tmp);

          // display checks
          if (!is_bottom &&
              display_overflow_check(OK, ptr, offset, access_size, loc)) {
            std::cerr << ": ∃o ∈ offset, o + access_size <= " << size_var
                      << std::endl;
          } else if (is_bottom && display_overflow_check(ERROR,
                                                         ptr,
                                                         offset,
                                                         access_size,
                                                         loc)) {
            std::cerr << ": ∀o ∈ offset, o + access_size > " << size_var
                      << std::endl;
          }

          all_valid = all_valid && !is_bottom;
          all_invalid = all_invalid && is_bottom;
        }

        // add block info
        json_dict block_info = {{"addr", (*it)->str()}};

        // add size (min, max) to block_info
        interval_t size_int =
            ikos::num_domain_traits::to_interval(inv, size_var);
        json_dict_put(block_info, "size_min", size_int.lb());
        json_dict_put(block_info, "size_max", size_int.ub());

        // add offset + access_size - size (min, max) to block_info
        interval_t diff_int =
            ikos::num_domain_traits::to_interval(inv,
                                                 offset + access_size -
                                                     size_var);
        json_dict_put(block_info, "diff_min", diff_int.lb());
        json_dict_put(block_info, "diff_max", diff_int.ub());

        points_to_info.add(block_info);
      }

      info.put("type", "var");
      info.put("points-to", points_to_info);

      if (all_invalid) {
        return ERROR;
      } else if (!all_valid) {
        return WARNING;
      } else {
        return OK;
      }
    }
  }

  inline bool display_underflow_check(analysis_result result,
                                      varname_t ptr,
                                      linear_expression_t offset,
                                      linear_expression_t access_size,
                                      const location& loc) {
    if (checker_t::display_check(result, loc)) {
      std::cerr << " check_underflow(ptr=" << ptr->str()
                << ", offset=" << offset << ", access_size=" << access_size
                << ")";
      return true;
    }
    return false;
  }

  /*
   * Check a memory access (read/write) for underflow
   *
   * Arguments:
   *   ptr: The pointer variable
   *   offset: A linear expression for the offset of the pointer (in bytes)
   *   access_size: A linear expression for the read/written size (in bytes)
   *   inv: The invariant, as an abstract value
   *   loc: The source location
   *
   * Returns:
   *   The analysis result (OK, WARNING, ERROR, UNREACHABLE)
   *
   * Description:
   *   The method checks that the memory access to
   *   [offset, offset + access_size - 1] is valid.
   *
   *   It checks the following property:
   *     addrs_set(ptr) != TOP && ∀o ∈ offset, o >= 0
   */
  analysis_result check_underflow(varname_t ptr,
                                  linear_expression_t offset,
                                  linear_expression_t access_size,
                                  AbsDomain inv,
                                  const location& loc,
                                  json_dict& info) {
    if (ikos::ptr_domain_traits::is_unknown_addr(inv, ptr)) {
      if (display_underflow_check(WARNING, ptr, offset, access_size, loc)) {
        std::cerr << ": no points-to information for " << ptr->str()
                  << std::endl;
      }

      info.put("type", "no-points-to");
      return WARNING;
    } else {
      info.put("type", "var");

      // add offset (min, max) to info dict
      interval_t offset_int = ikos::num_domain_traits::to_interval(inv, offset);
      json_dict_put(info, "offset_min", offset_int.lb());
      json_dict_put(info, "offset_max", offset_int.ub());

      // check
      AbsDomain tmp(inv);
      tmp += (offset <= -1);
      if (!ikos::exc_domain_traits::is_normal_flow_bottom(tmp)) {
        // warning or error
        inv += (offset >= 0);
        if (ikos::exc_domain_traits::is_normal_flow_bottom(inv)) {
          if (display_underflow_check(ERROR, ptr, offset, access_size, loc)) {
            std::cerr << ": ∀o ∈ offset, o < 0" << std::endl;
          }

          return ERROR;
        } else {
          if (display_underflow_check(WARNING, ptr, offset, access_size, loc)) {
            std::cerr << ": ∃o ∈ offset, o < 0" << std::endl;
          }

          return WARNING;
        }
      } else { // safe
        if (display_underflow_check(OK, ptr, offset, access_size, loc)) {
          std::cerr << ": ∀o ∈ offset, o >= 0" << std::endl;
        }

        return OK;
      }
    }
  }

  inline bool display_strcpy_check(analysis_result result,
                                   const scalar_lit_t& dest,
                                   const scalar_lit_t& src,
                                   const location& loc) {
    if (checker_t::display_check(result, loc)) {
      std::cerr << " check_strcpy(dest=" << dest << ", src=" << src << ")";
      return true;
    }
    return false;
  }

  /*
   * Check a string copy for overflow
   *
   * Arguments:
   *   dest_op: The destination operand
   *   src_op: The source operand
   *   inv: The invariant, as an abstract value
   *   call_context: The calling context
   *   function: The current function
   *   loc: The source location
   *   stmt_uid: The UID of the AR_Statement
   */
  void check_strcpy(Operand_ref dest_op,
                    Operand_ref src_op,
                    AbsDomain inv,
                    const std::string& call_context,
                    const boost::optional< Function_ref > function,
                    const location& loc,
                    unsigned long stmt_uid) {
    LiteralFactory& lfac = this->_context.lit_factory();
    scalar_lit_t dest = lfac[dest_op];
    scalar_lit_t src = lfac[src_op];
    analysis_result overflow_result = OK;
    json_dict info;

    if (ikos::exc_domain_traits::is_normal_flow_bottom(inv)) {
      overflow_result = UNREACHABLE;

      if (display_strcpy_check(overflow_result, dest, src, loc)) {
        std::cerr << std::endl;
      }
    } else if (ar::isFunctionPointer(dest_op)) {
      overflow_result = ERROR;
      info.put("type", "function-pointer");
      info.put("pointer", dest.var()->name());

      if (display_strcpy_check(overflow_result, dest, src, loc)) {
        std::cerr << ": dest is a function pointer" << std::endl;
      }
    } else if (ar::isFunctionPointer(src_op)) {
      overflow_result = ERROR;
      info.put("type", "function-pointer");
      info.put("pointer", src.var()->name());

      if (display_strcpy_check(overflow_result, dest, src, loc)) {
        std::cerr << ": src is a function pointer" << std::endl;
      }
    } else {
      if (dest.is_null() ||
          (dest.is_pointer_var() &&
           ikos::null_domain_traits::is_null(inv, dest.var()))) {
        overflow_result = ERROR;
        if (dest.is_null()) {
          info.put("type", "null-constant-pointer");
        } else {
          info.put("type", "null-var-pointer");
          info.put("pointer", dest.var()->name());
        }

        if (display_strcpy_check(overflow_result, dest, src, loc)) {
          std::cerr << ": dest is null" << std::endl;
        }
      } else if (dest.is_undefined() ||
                 (dest.is_pointer_var() &&
                  ikos::uninit_domain_traits::is_uninitialized(inv,
                                                               dest.var()))) {
        overflow_result = ERROR;
        if (dest.is_undefined()) {
          info.put("type", "undefined-constant-pointer");
        } else {
          info.put("type", "undefined-var-pointer");
          info.put("pointer", dest.var()->name());
        }

        if (display_strcpy_check(overflow_result, dest, src, loc)) {
          std::cerr << ": dest is uninitialized" << std::endl;
        }
      } else if (src.is_null() ||
                 (src.is_pointer_var() &&
                  ikos::null_domain_traits::is_null(inv, src.var()))) {
        overflow_result = ERROR;
        if (src.is_null()) {
          info.put("type", "null-constant-pointer");
        } else {
          info.put("type", "null-var-pointer");
          info.put("pointer", src.var()->name());
        }

        if (display_strcpy_check(overflow_result, dest, src, loc)) {
          std::cerr << ": src is null" << std::endl;
        }
      } else if (src.is_undefined() ||
                 (src.is_pointer_var() &&
                  ikos::uninit_domain_traits::is_uninitialized(inv,
                                                               src.var()))) {
        overflow_result = ERROR;
        if (src.is_undefined()) {
          info.put("type", "undefined-constant-pointer");
        } else {
          info.put("type", "undefined-var-pointer");
          info.put("pointer", src.var()->name());
        }

        if (display_strcpy_check(overflow_result, dest, src, loc)) {
          std::cerr << ": src is uninitialized" << std::endl;
        }
      } else if (dest.is_pointer_var() && src.is_pointer_var()) {
        // both dest and src are pointer variables
        if (ikos::ptr_domain_traits::is_unknown_addr(inv, dest.var())) {
          overflow_result = WARNING;
          info.put("type", "no-points-to");
          info.put("pointer", dest.var()->name());

          if (display_strcpy_check(overflow_result, dest, src, loc)) {
            std::cerr << ": no points-to information for dest" << std::endl;
          }
        } else if (ikos::ptr_domain_traits::is_unknown_addr(inv, src.var())) {
          overflow_result = WARNING;
          info.put("type", "no-points-to");
          info.put("pointer", src.var()->name());

          if (display_strcpy_check(overflow_result, dest, src, loc)) {
            std::cerr << ": no points-to information for src" << std::endl;
          }
        } else {
          ikos::discrete_domain< memloc_t > dest_addrs_set =
              ikos::ptr_domain_traits::addrs_set< AbsDomain,
                                                  memloc_t >(inv, dest.var());
          ikos::discrete_domain< memloc_t > src_addrs_set =
              ikos::ptr_domain_traits::addrs_set< AbsDomain,
                                                  memloc_t >(inv, src.var());
          bool all_valid = true;
          auto& vfac = this->_context.var_factory();

          for (auto dest_it = dest_addrs_set.begin();
               dest_it != dest_addrs_set.end();
               dest_it++) {
            varname_t dest_size =
                sym_exec_traits::alloc_size_var(vfac, *dest_it);
            varname_t dest_offset = inv.offset_var(dest.var());
            linear_expression_t max_space_available =
                variable_t(dest_size) - variable_t(dest_offset);

            for (auto src_it = src_addrs_set.begin();
                 src_it != src_addrs_set.end();
                 src_it++) {
              varname_t src_size =
                  sym_exec_traits::alloc_size_var(vfac, *src_it);
              varname_t src_offset = inv.offset_var(src.var());
              linear_expression_t max_space_needed =
                  variable_t(src_size) - variable_t(src_offset);

              AbsDomain tmp(inv);
              tmp += (max_space_needed >= max_space_available + 1);
              bool is_bottom =
                  ikos::exc_domain_traits::is_normal_flow_bottom(tmp);

              if (is_bottom && display_strcpy_check(OK, dest, src, loc)) {
                std::cerr << ": ∀(s, d) ∈ src.offset x dest.offset, "
                          << src_size << " - s <= " << dest_size << " - d"
                          << std::endl;
              } else if (!is_bottom &&
                         display_strcpy_check(WARNING, dest, src, loc)) {
                std::cerr << ": ∃(s, d) ∈ src.offset x dest.offset, "
                          << src_size << " - s > " << dest_size << " - d"
                          << std::endl;
              }

              all_valid = all_valid && is_bottom;
            }
          }

          info.put("type", "strcpy");
          info.put("src", src.var()->name());
          info.put("dest", dest.var()->name());

          if (all_valid) {
            overflow_result = OK;
          } else {
            overflow_result = WARNING;
          }
        }
      } else {
        throw analyzer_error(
            "buffer_overflow_checker: unexpected operands to strcpy");
      }
    }

    this->display_invariant(overflow_result, inv, loc);
    this->_results.write("overflow",
                         overflow_result,
                         call_context,
                         function,
                         loc,
                         stmt_uid,
                         info);
  }

}; // end class buffer_overflow_checker

} // end namespace analyzer

#endif // ANALYZER_BUFFER_OVERFLOW_CHECKER_HPP
