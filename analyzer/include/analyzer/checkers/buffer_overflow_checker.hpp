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

#ifndef ANALYZER_BUFFER_OVERFLOW_CHECKER_HPP
#define ANALYZER_BUFFER_OVERFLOW_CHECKER_HPP

#include <analyzer/analysis/common.hpp>
#include <analyzer/analysis/num_sym_exec.hpp>
#include <analyzer/checkers/checker_api.hpp>

namespace analyzer {

using namespace arbos;

template < class AbsDomain >
class buffer_overflow_checker : public checker< AbsDomain > {
private:
  typedef checker< AbsDomain > checker_t;
  typedef typename analysis_db::db_ptr db_ptr_t;

  typedef typename AbsDomain::variable_t variable_t;
  typedef typename AbsDomain::number_t number_t;
  typedef typename AbsDomain::linear_expression_t linear_expression_t;

public:
  buffer_overflow_checker(context& ctx,
                          db_ptr_t db,
                          display_settings display_invariants,
                          display_settings display_checks)
      : checker_t(ctx, db, display_invariants, display_checks) {}

  virtual const char* name() { return "boa"; }
  virtual const char* description() { return "Buffer overflow checker"; }

  virtual void check(Store_ref store,
                     AbsDomain inv,
                     const std::string& call_context) {
    Operand_ref store_pointer = ar::getPointer(store);

    // Skip if writing to an integer/floating point global
    // variable or a local variable whose address has been taken
    // since it's always safe
    if (!ar::isGlobalVar(store_pointer) && !ar::isAllocaVar(store_pointer)) {
      location loc = ar::getSrcLoc(store);
      LiteralFactory& lfac = this->_context.lit_factory();
      uint64_t store_size =
          ar::getSize(ar::getPointeeType(ar::getType(store_pointer)));
      check_mem_access(lfac[store_pointer],
                       lfac[store_size],
                       inv,
                       call_context,
                       loc,
                       ar::getUID(store));
    }
  }

  virtual void check(Load_ref load,
                     AbsDomain inv,
                     const std::string& call_context) {
    Operand_ref load_pointer = ar::getPointer(load);

    // Skip if reading to an integer/floating point global
    // variable or a local variable whose address has been taken
    // since it's always safe
    if (!ar::isGlobalVar(load_pointer) && !ar::isAllocaVar(load_pointer)) {
      location loc = ar::getSrcLoc(load);
      LiteralFactory& lfac = this->_context.lit_factory();
      uint64_t load_size =
          ar::getSize(ar::getPointeeType(ar::getType(load_pointer)));
      check_mem_access(lfac[load_pointer],
                       lfac[load_size],
                       inv,
                       call_context,
                       loc,
                       ar::getUID(load));
    }
  }

  virtual void check(MemCpy_ref memcpy,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(memcpy);
    LiteralFactory& lfac = this->_context.lit_factory();
    Literal dest = lfac[ar::getTarget(memcpy)];
    Literal src = lfac[ar::getSource(memcpy)];
    Literal len = lfac[ar::getLen(memcpy)];

    // Both dest and src are already allocated in memory so we need to
    // check that their offsets are in-bounds.
    check_mem_access(src, len, inv, call_context, loc, ar::getUID(memcpy));
    check_mem_access(dest, len, inv, call_context, loc, ar::getUID(memcpy));
  }

  virtual void check(MemMove_ref memmove,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(memmove);
    LiteralFactory& lfac = this->_context.lit_factory();
    Literal dest = lfac[ar::getTarget(memmove)];
    Literal src = lfac[ar::getSource(memmove)];
    Literal len = lfac[ar::getLen(memmove)];

    // Both dest and src are already allocated in memory so we need to
    // check that their offsets are in-bounds.
    check_mem_access(src, len, inv, call_context, loc, ar::getUID(memmove));
    check_mem_access(dest, len, inv, call_context, loc, ar::getUID(memmove));
  }

  virtual void check(MemSet_ref memset,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(memset);
    LiteralFactory& lfac = this->_context.lit_factory();
    Literal dest = lfac[ar::getBase(memset)];
    Literal len = lfac[ar::getLen(memset)];

    // dest is already allocated in memory so we need to check
    // that its offset is in-bounds.
    check_mem_access(dest, len, inv, call_context, loc, ar::getUID(memset));
  }

  virtual void check(Call_ref call,
                     AbsDomain inv,
                     const std::string& call_context) {
    if (ar::isDirectCall(call)) {
      location loc = ar::getSrcLoc(call);
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
        Literal s = lfac[arguments[0]];
        check_mem_access(s, lfac[1], inv, call_context, loc, ar::getUID(call));
      } else if (fun_name == "strnlen" && arguments.size() == 2) {
        Literal s = lfac[arguments[0]];
        Literal n = lfac[arguments[1]];
        if (is_greater_equal(n, 1, inv)) {
          check_mem_access(s,
                           lfac[1],
                           inv,
                           call_context,
                           loc,
                           ar::getUID(call));
        }
      } else if (fun_name == "strcpy" && arguments.size() == 2) {
        Literal dest = lfac[arguments[0]];
        Literal src = lfac[arguments[1]];
        check_mem_access(dest,
                         lfac[1],
                         inv,
                         call_context,
                         loc,
                         ar::getUID(call));
        check_mem_access(src,
                         lfac[1],
                         inv,
                         call_context,
                         loc,
                         ar::getUID(call));
        check_strcpy(dest, src, inv, call_context, loc, ar::getUID(call));
      } else if (fun_name == "strncpy" && arguments.size() == 3) {
        Literal dest = lfac[arguments[0]];
        Literal src = lfac[arguments[1]];
        Literal n = lfac[arguments[2]];
        if (is_greater_equal(n, 1, inv)) {
          check_mem_access(dest,
                           lfac[1],
                           inv,
                           call_context,
                           loc,
                           ar::getUID(call));
          check_mem_access(src,
                           lfac[1],
                           inv,
                           call_context,
                           loc,
                           ar::getUID(call));
        }
        // TODO: check_strncpy
      } else if (fun_name == "strcat" && arguments.size() == 2) {
        Literal s1 = lfac[arguments[0]];
        Literal s2 = lfac[arguments[1]];
        check_mem_access(s1, lfac[1], inv, call_context, loc, ar::getUID(call));
        check_mem_access(s2, lfac[1], inv, call_context, loc, ar::getUID(call));
      } else if (fun_name == "strncat" && arguments.size() == 3) {
        Literal s1 = lfac[arguments[0]];
        Literal s2 = lfac[arguments[1]];
        Literal n = lfac[arguments[2]];
        if (is_greater_equal(n, 1, inv)) {
          check_mem_access(s1,
                           lfac[1],
                           inv,
                           call_context,
                           loc,
                           ar::getUID(call));
          check_mem_access(s2,
                           lfac[1],
                           inv,
                           call_context,
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
  static bool is_greater_equal(const Literal& lit, number_t n, AbsDomain inv) {
    if (lit.is_undefined_cst()) {
      return false;
    } else if (lit.is_num()) {
      return lit.get_num< number_t >() >= n;
    } else if (lit.is_var()) {
      if (exc_domain_traits::is_normal_flow_bottom(inv))
        return false;

      inv += (variable_t(lit.get_var()) <= n - 1);
      return exc_domain_traits::is_normal_flow_bottom(inv);
    } else {
      assert(false && "unreachable");
    }
  }

  /*
   * Write the results of a memory access check (buffer overflow + underflow) in
   * the database.
   */
  inline void write_result(analysis_result overflow_result,
                           analysis_result underflow_result,
                           const std::string& call_context,
                           const location& loc,
                           unsigned long stmt_uid) {
    this->_db->write("overflow",
                     call_context,
                     loc.file,
                     loc.line,
                     loc.column,
                     stmt_uid,
                     tostr(overflow_result));
    this->_db->write("underflow",
                     call_context,
                     loc.file,
                     loc.line,
                     loc.column,
                     stmt_uid,
                     tostr(underflow_result));
  }

  /*
   * Check a memory access (read/write) for overflow/underflow.
   *
   * Arguments:
   *   pointer: The pointer variable, as a literal
   *   access_size: The read/written size (in bytes), as a literal
   *   inv: The invariant, as an abstract value
   *   call_context: The calling context
   *   loc: The source location
   *   stmt_uid: The UID of the AR_Statement
   *
   * Description:
   *   The method checks that the memory access is valid and writes the result
   *   in the database.
   */
  void check_mem_access(const Literal& pointer,
                        const Literal& access_size,
                        AbsDomain inv,
                        const std::string& call_context,
                        const location& loc,
                        unsigned long stmt_uid) {
    if (pointer.is_undefined_cst() || pointer.is_null_cst() ||
        access_size.is_undefined_cst()) {
      if (this->display_check(ERR)) {
        std::cout << location_to_string(loc)
                  << ": [error] check_mem_access(ptr=";
        pointer.dump(std::cout);
        std::cout << ", access_size=";
        access_size.dump(std::cout);
        std::cout << "): ";
        if (pointer.is_undefined_cst()) {
          std::cout << "ptr is undefined" << std::endl;
        } else if (pointer.is_null_cst()) {
          std::cout << "ptr is null" << std::endl;
        } else {
          std::cout << "access_size is undefined" << std::endl;
        }
      }
      if (this->display_invariant(ERR)) {
        std::cout << location_to_string(loc) << ": Invariant:" << std::endl
                  << inv << std::endl;
      }

      write_result(ERR, ERR, call_context, loc, stmt_uid);
      return;
    }

    assert(pointer.is_var());
    assert(access_size.is_var() || access_size.is_num());

    varname_t pointer_var = pointer.get_var();
    linear_expression_t offset_expr = variable_t(inv.offset_var(pointer_var));
    linear_expression_t access_size_expr =
        access_size.is_var()
            ? linear_expression_t(access_size.get_var())
            : linear_expression_t(access_size.get_num< ikos::z_number >());

    analysis_result overflow_result =
        check_overflow(pointer_var, offset_expr, access_size_expr, inv, loc);
    analysis_result underflow_result =
        check_underflow(pointer_var, offset_expr, access_size_expr, inv, loc);

    if (this->display_invariant(overflow_result) ||
        this->display_invariant(underflow_result)) {
      std::cout << location_to_string(loc) << ": Invariant:" << std::endl
                << inv << std::endl;
    }

    write_result(overflow_result,
                 underflow_result,
                 call_context,
                 loc,
                 stmt_uid);
  }

  /*
   * Check a memory access (read/write) for overflow
   *
   * Arguments:
   *   pointer: The pointer variable
   *   offset: A linear expression for the offset of the pointer (in bytes)
   *   access_size: A linear expression for the read/written size (in bytes)
   *   inv: The invariant, as an abstract value
   *   loc: The source location
   *
   * Returns:
   *   The analysis result (OK, WARNING, ERR, UNREACHABLE)
   *
   * Description:
   *   The method checks that the memory access to
   *   [offset, offset + access_size - 1] is valid.
   *
   *   It checks the following property:
   *     addrs_set(pointer) != TOP &&
   *     ∀a ∈ addrs_set(pointer), ∀o ∈ offset, o + access_size <= a.size
   */
  analysis_result check_overflow(varname_t pointer,
                                 linear_expression_t offset,
                                 linear_expression_t access_size,
                                 AbsDomain inv,
                                 const location& loc) {
    if (exc_domain_traits::is_normal_flow_bottom(inv)) {
      if (this->display_check(UNREACHABLE)) {
        std::cout << location_to_string(loc)
                  << ": [unreachable] check_overflow(ptr=" << pointer
                  << ", offset=" << offset << ", access_size=" << access_size
                  << ")" << std::endl;
      }

      return UNREACHABLE;
    }

    if (ptr_domain_traits::is_unknown_addr(inv, pointer)) {
      if (this->display_check(WARNING)) {
        std::cout << location_to_string(loc)
                  << ": [warning] check_overflow(ptr=" << pointer
                  << ", offset=" << offset << ", access_size=" << access_size
                  << "): no points-to information for " << pointer << std::endl;
      }

      return WARNING;
    } else {
      ikos::discrete_domain< varname_t > addrs_set =
          ptr_domain_traits::addrs_set(inv, pointer);
      assert(!addrs_set.is_top());
      bool all_valid = true;
      bool all_invalid = true;

      for (auto it = addrs_set.begin(); it != addrs_set.end(); ++it) {
        varname_t size_var = num_sym_exec_impl::get_shadow_size(*it);

        if (all_valid) {
          AbsDomain tmp(inv);
          tmp += (offset + access_size - 1 >= variable_t(size_var));
          bool is_bottom = exc_domain_traits::is_normal_flow_bottom(tmp);

          if (is_bottom && this->display_check(OK)) {
            std::cout << location_to_string(loc)
                      << ": [ok] check_overflow(ptr=" << pointer
                      << ", offset=" << offset
                      << ", access_size=" << access_size
                      << "): ∀o ∈ offset, o + access_size <= " << size_var
                      << std::endl;
          } else if (!is_bottom && this->display_check(WARNING)) {
            std::cout << location_to_string(loc)
                      << ": [warning] check_overflow(ptr=" << pointer
                      << ", offset=" << offset
                      << ", access_size=" << access_size
                      << "): ∃o ∈ offset, o + access_size > " << size_var
                      << std::endl;
          }

          all_valid = all_valid && is_bottom;
          all_invalid = all_invalid && !is_bottom;
        }

        if (all_invalid) {
          AbsDomain tmp(inv);
          tmp += (offset + access_size <= variable_t(size_var));
          bool is_bottom = exc_domain_traits::is_normal_flow_bottom(tmp);

          if (!is_bottom && this->display_check(OK)) {
            std::cout << location_to_string(loc)
                      << ": [ok] check_overflow(ptr=" << pointer
                      << ", offset=" << offset
                      << ", access_size=" << access_size
                      << "): ∃o ∈ offset, o + access_size <= " << size_var
                      << std::endl;
          } else if (is_bottom && this->display_check(WARNING)) {
            std::cout << location_to_string(loc)
                      << ": [warning|error] check_overflow(ptr=" << pointer
                      << ", offset=" << offset
                      << ", access_size=" << access_size
                      << "): ∀o ∈ offset, o + access_size > " << size_var
                      << std::endl;
          }

          all_valid = all_valid && !is_bottom;
          all_invalid = all_invalid && is_bottom;
        }
      }

      if (all_invalid) {
        return ERR;
      } else if (!all_valid) {
        return WARNING;
      } else {
        return OK;
      }
    }
  }

  /*
   * Check a memory access (read/write) for underflow
   *
   * Arguments:
   *   pointer: The pointer variable
   *   offset: A linear expression for the offset of the pointer (in bytes)
   *   access_size: A linear expression for the read/written size (in bytes)
   *   inv: The invariant, as an abstract value
   *   loc: The source location
   *
   * Returns:
   *   The analysis result (OK, WARNING, ERR, UNREACHABLE)
   *
   * Description:
   *   The method checks that the memory access to
   *   [offset, offset + access_size - 1] is valid.
   *
   *   It checks the following property:
   *     addrs_set(pointer) != TOP && ∀o ∈ offset, o >= 0
   */
  analysis_result check_underflow(varname_t pointer,
                                  linear_expression_t offset,
                                  linear_expression_t access_size,
                                  AbsDomain inv,
                                  const location& loc) {
    if (exc_domain_traits::is_normal_flow_bottom(inv)) {
      if (this->display_check(UNREACHABLE)) {
        std::cout << location_to_string(loc)
                  << ": [unreachable] check_underflow(ptr=" << pointer
                  << ", offset=" << offset << ", access_size=" << access_size
                  << ")" << std::endl;
      }

      return UNREACHABLE;
    }

    if (ptr_domain_traits::is_unknown_addr(inv, pointer)) {
      if (this->display_check(WARNING)) {
        std::cout << location_to_string(loc)
                  << ": [warning] check_underflow(ptr=" << pointer
                  << ", offset=" << offset << ", access_size=" << access_size
                  << "): no points-to information for " << pointer << std::endl;
      }

      return WARNING;
    } else {
      AbsDomain tmp(inv);
      tmp += (offset <= -1);
      if (!exc_domain_traits::is_normal_flow_bottom(tmp)) { // warning or error
        inv += (offset >= 0);
        if (exc_domain_traits::is_normal_flow_bottom(inv)) {
          if (this->display_check(ERR)) {
            std::cout << location_to_string(loc)
                      << ": [error] check_underflow(ptr=" << pointer
                      << ", offset=" << offset
                      << ", access_size=" << access_size
                      << "): ∀o ∈ offset, o < 0" << std::endl;
          }

          return ERR;
        } else {
          if (this->display_check(WARNING)) {
            std::cout << location_to_string(loc)
                      << ": [warning] check_underflow(ptr=" << pointer
                      << ", offset=" << offset
                      << ", access_size=" << access_size
                      << "): ∃o ∈ offset, o < 0" << std::endl;
          }

          return WARNING;
        }
      } else { // safe
        if (this->display_check(OK)) {
          std::cout << location_to_string(loc)
                    << ": [ok] check_underflow(ptr=" << pointer
                    << ", offset=" << offset << ", access_size=" << access_size
                    << "): ∀o ∈ offset, o >= 0" << std::endl;
        }

        return OK;
      }
    }
  }

  /*
   * Check a string copy for overflow
   *
   * Arguments:
   *   dest: The destination pointer
   *   src: The source pointer
   *   inv: The invariant, as an abstract value
   *   call_context: The calling context
   *   loc: The source location
   *   stmt_uid: The UID of the AR_Statement
   */
  void check_strcpy(Literal dest,
                    Literal src,
                    AbsDomain inv,
                    const std::string& call_context,
                    const location& loc,
                    unsigned long stmt_uid) {
    analysis_result overflow_result = OK;

    if (dest.is_undefined_cst() || dest.is_null_cst() ||
        src.is_undefined_cst() || src.is_null_cst()) {
      if (this->display_check(ERR)) {
        std::cout << location_to_string(loc) << ": [error] check_strcpy(dest=";
        dest.dump(std::cout);
        std::cout << ", src=";
        src.dump(std::cout);
        std::cout << "): ";
        if (dest.is_undefined_cst()) {
          std::cout << "dest is undefined" << std::endl;
        } else if (dest.is_null_cst()) {
          std::cout << "dest is null" << std::endl;
        } else if (src.is_undefined_cst()) {
          std::cout << "src is undefined" << std::endl;
        } else {
          std::cout << "src is null" << std::endl;
        }
      }

      overflow_result = ERR;
    } else {
      assert(dest.is_var());
      assert(src.is_var());

      if (exc_domain_traits::is_normal_flow_bottom(inv)) {
        if (this->display_check(UNREACHABLE)) {
          std::cout << location_to_string(loc)
                    << ": [unreachable] check_strcpy(dest=";
          dest.dump(std::cout);
          std::cout << ", src=";
          src.dump(std::cout);
          std::cout << ")";
        }

        overflow_result = UNREACHABLE;
      } else if (ptr_domain_traits::is_unknown_addr(inv, dest.get_var())) {
        if (this->display_check(WARNING)) {
          std::cout << location_to_string(loc)
                    << ": [error] check_strcpy(dest=";
          dest.dump(std::cout);
          std::cout << ", src=";
          src.dump(std::cout);
          std::cout << "): no points-to information for dest" << std::endl;
        }

        overflow_result = WARNING;
      } else if (ptr_domain_traits::is_unknown_addr(inv, src.get_var())) {
        if (this->display_check(WARNING)) {
          std::cout << location_to_string(loc)
                    << ": [error] check_strcpy(dest=";
          dest.dump(std::cout);
          std::cout << ", src=";
          src.dump(std::cout);
          std::cout << "): no points-to information for src" << std::endl;
        }

        overflow_result = WARNING;
      } else {
        ikos::discrete_domain< varname_t > dest_addrs_set =
            ptr_domain_traits::addrs_set(inv, dest.get_var());
        ikos::discrete_domain< varname_t > src_addrs_set =
            ptr_domain_traits::addrs_set(inv, src.get_var());
        bool all_valid = true;

        for (auto dest_it = dest_addrs_set.begin();
             dest_it != dest_addrs_set.end();
             dest_it++) {
          varname_t dest_size = num_sym_exec_impl::get_shadow_size(*dest_it);
          varname_t dest_offset = inv.offset_var(dest.get_var());
          linear_expression_t max_space_available =
              variable_t(dest_size) - variable_t(dest_offset);

          for (auto src_it = src_addrs_set.begin();
               src_it != src_addrs_set.end();
               src_it++) {
            varname_t src_size = num_sym_exec_impl::get_shadow_size(*src_it);
            varname_t src_offset = inv.offset_var(src.get_var());
            linear_expression_t max_space_needed =
                variable_t(src_size) - variable_t(src_offset);

            AbsDomain tmp(inv);
            tmp += (max_space_needed >= max_space_available + 1);
            bool is_bottom = exc_domain_traits::is_normal_flow_bottom(tmp);

            if (is_bottom && this->display_check(OK)) {
              std::cout << location_to_string(loc) << ": [ok] strcpy(dest=";
              dest.dump(std::cout);
              std::cout << ", src=";
              src.dump(std::cout);
              std::cout << "): ∀(s, d) ∈ src.offset x dest.offset, " << src_size
                        << " - s <= " << dest_size << " - d" << std::endl;
            } else if (!is_bottom && this->display_check(WARNING)) {
              std::cout << location_to_string(loc)
                        << ": [warning] strcpy(dest=";
              dest.dump(std::cout);
              std::cout << ", src=";
              src.dump(std::cout);
              std::cout << "): ∃(s, d) ∈ src.offset x dest.offset, " << src_size
                        << " - s > " << dest_size << " - d" << std::endl;
            }

            all_valid = all_valid && is_bottom;
          }
        }

        if (all_valid) {
          overflow_result = OK;
        } else {
          overflow_result = WARNING;
        }
      }
    }

    if (this->display_invariant(overflow_result)) {
      std::cout << location_to_string(loc) << ": Invariant:" << std::endl
                << inv << std::endl;
    }

    this->_db->write("overflow",
                     call_context,
                     loc.file,
                     loc.line,
                     loc.column,
                     stmt_uid,
                     tostr(overflow_result));
  }

}; // end class buffer_overflow_checker

} // end namespace analyzer

#endif // ANALYZER_BUFFER_OVERFLOW_CHECKER_HPP
