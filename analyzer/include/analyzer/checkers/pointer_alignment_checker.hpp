/*******************************************************************************
 *
 * Pointer Alignment checker.
 *
 * Author: Clement Decoodt
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

#ifndef ANALYZER_POINTER_ALIGNMENT_CHECKER_HPP
#define ANALYZER_POINTER_ALIGNMENT_CHECKER_HPP

#include <analyzer/analysis/common.hpp>
#include <analyzer/analysis/memory_location.hpp>
#include <analyzer/analysis/num_sym_exec.hpp>
#include <analyzer/analysis/variable_name.hpp>
#include <analyzer/cast.hpp>
#include <analyzer/checkers/checker_api.hpp>

namespace analyzer {

using namespace arbos;

template < class AbsDomain >
class pointer_alignment_checker : public checker< AbsDomain > {
private:
  typedef checker< AbsDomain > checker_t;

public:
  pointer_alignment_checker(context& ctx,
                            results_table_t& results_table,
                            display_settings display_invariants,
                            display_settings display_checks)
      : checker_t(ctx, results_table, display_invariants, display_checks) {}

  virtual const char* name() { return "upa"; }
  virtual const char* description() { return "pointer alignment checker"; }

  //! Check for Store statement
  //  A store instruction contains a pointer with an alignment property
  virtual void check(Store_ref store,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(store);
    boost::optional< Function_ref > fun = ar::getParentFunction(store);
    Operand_ref ptr = ar::getPointer(store);
    boost::optional< z_number > alignment_req = ar::getAlignment(store);
    check_pointer_alignment(ptr,
                            inv,
                            alignment_req,
                            call_context,
                            fun,
                            loc,
                            ar::getUID(store));
  }

  //! Check for Load statement
  //  A load instruction contains a pointer with an alignment property
  virtual void check(Load_ref load,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(load);
    boost::optional< Function_ref > fun = ar::getParentFunction(load);
    Operand_ref ptr = ar::getPointer(load);
    boost::optional< z_number > alignment_req = ar::getAlignment(load);
    check_pointer_alignment(ptr,
                            inv,
                            alignment_req,
                            call_context,
                            fun,
                            loc,
                            ar::getUID(load));
  }

  //! Check for MemCpy statement
  //  A memcpy instruction contains src and destination pointers with an
  //  alignment property for both of them
  virtual void check(MemCpy_ref memcpy,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(memcpy);
    boost::optional< Function_ref > fun = ar::getParentFunction(memcpy);
    Operand_ref ptr_src = ar::getSource(memcpy);
    Operand_ref ptr_dest = ar::getTarget(memcpy);
    boost::optional< z_number > alignment_req = ar::getAlignment(memcpy);
    check_pointer_alignment(ptr_src,
                            inv,
                            alignment_req,
                            call_context,
                            fun,
                            loc,
                            ar::getUID(memcpy));
    check_pointer_alignment(ptr_dest,
                            inv,
                            alignment_req,
                            call_context,
                            fun,
                            loc,
                            ar::getUID(memcpy));
  }

  //! Check for MemMove statement
  //  A memmove instruction contains src and destination pointers with an
  //  alignment property for both of them
  virtual void check(MemMove_ref memmove,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(memmove);
    boost::optional< Function_ref > fun = ar::getParentFunction(memmove);
    Operand_ref ptr_src = ar::getSource(memmove);
    Operand_ref ptr_dest = ar::getTarget(memmove);
    boost::optional< z_number > alignment_req = ar::getAlignment(memmove);
    check_pointer_alignment(ptr_src,
                            inv,
                            alignment_req,
                            call_context,
                            fun,
                            loc,
                            ar::getUID(memmove));
    check_pointer_alignment(ptr_dest,
                            inv,
                            alignment_req,
                            call_context,
                            fun,
                            loc,
                            ar::getUID(memmove));
  }

  //! Check for MemSet statement
  //  A memset instruction contains a destination pointer with an alignment
  //  property
  virtual void check(MemSet_ref memset,
                     AbsDomain inv,
                     const std::string& call_context) {
    location loc = ar::getSrcLoc(memset);
    boost::optional< Function_ref > fun = ar::getParentFunction(memset);
    Operand_ref ptr = ar::getBase(memset);
    boost::optional< z_number > alignment_req = ar::getAlignment(memset);
    check_pointer_alignment(ptr,
                            inv,
                            alignment_req,
                            call_context,
                            fun,
                            loc,
                            ar::getUID(memset));
  }

private:
  inline bool display_check(analysis_result result,
                            const scalar_lit_t& ptr,
                            const location& loc) {
    if (checker_t::display_check(result, loc)) {
      std::cerr << " check_pointer_alignment(" << ptr << ")";
      return true;
    }
    return false;
  }

  analysis_result check_memloc_alignment(
      memloc_t memloc,
      const ikos::congruence< number_t >& offset_congr,
      const ikos::congruence< number_t >& alignment_req_congr,
      json_dict& block_info) {
    // Get the alignment of the memory_location
    bool pto_in_alignment_req = false;
    bool alignment_req_in_pto = true;

    if (local_memory_location* local_memloc =
            dyn_cast< local_memory_location >(memloc)) {
      ikos::congruence< number_t >
          local_alignment(ikos::z_number(
                              static_cast< mpz_class >(*ar::getAlignment(
                                  local_memloc->local_variable()))),
                          ikos::z_number(0));

      pto_in_alignment_req =
          local_alignment + offset_congr <= alignment_req_congr;
      alignment_req_in_pto =
          alignment_req_congr <= local_alignment + offset_congr;

#ifdef DEBUG
      std::cerr << "local memory location" << std::endl;
      std::cerr << local_alignment << " + " << offset_congr
                << " <= " << alignment_req_congr
                << " == " << pto_in_alignment_req << std::endl;
      std::cerr << alignment_req_congr << " <= " << local_alignment << " + "
                << offset_congr << " == " << alignment_req_in_pto << std::endl;
#endif

      block_info.put("memloc-type", "local");
      std::ostringstream buf;
      buf << local_alignment;
      block_info.put("congruence", buf.str());

    } else if (global_memory_location* global_memloc =
                   dyn_cast< global_memory_location >(memloc)) {
      ikos::congruence< number_t >
          global_alignment(ikos::z_number(
                               static_cast< mpz_class >(*ar::getAlignment(
                                   global_memloc->global_variable()))),
                           ikos::z_number(0));

      pto_in_alignment_req =
          global_alignment + offset_congr <= alignment_req_congr;
      alignment_req_in_pto =
          alignment_req_congr <= global_alignment + offset_congr;

#ifdef DEBUG
      std::cerr << "global memory location" << std::endl;
      std::cerr << global_alignment << " + " << offset_congr
                << " <= " << alignment_req_congr
                << " == " << pto_in_alignment_req << std::endl;
      std::cerr << alignment_req_congr << " <= " << global_alignment << " + "
                << offset_congr << " == " << alignment_req_in_pto << std::endl;
#endif

      block_info.put("memloc-type", "global");
      std::ostringstream buf;
      buf << global_alignment;
      block_info.put("congruence", buf.str());

    } else if (fixed_address_memory_location* fixed_addr_memloc =
                   dyn_cast< fixed_address_memory_location >(memloc)) {
      ikos::congruence< number_t >
          fixed_addr_alignment(ikos::z_number(0), fixed_addr_memloc->addr());

      pto_in_alignment_req =
          fixed_addr_alignment + offset_congr <= alignment_req_congr;
      alignment_req_in_pto =
          alignment_req_congr <= fixed_addr_alignment + offset_congr;

#ifdef DEBUG
      std::cerr << "fixed address memory location" << std::endl;
      std::cerr << fixed_addr_alignment << " + " << offset_congr
                << " <= " << alignment_req_congr
                << " == " << pto_in_alignment_req << std::endl;
      std::cerr << alignment_req_congr << " <= " << fixed_addr_alignment
                << " + " << offset_congr << " == " << alignment_req_in_pto
                << std::endl;
#endif

      block_info.put("memloc-type", "fixed-addr");
      std::ostringstream buf;
      buf << fixed_addr_alignment;
      block_info.put("congruence", buf.str());

    } else if (isa< function_memory_location >(memloc)) {
      block_info.put("memloc-type", "function");
      return ERROR;

    } else if (isa< dyn_alloc_memory_location >(memloc)) {
      block_info.put("memloc-type", "dyn-alloc");

      // We suppose a dynamic allocation like malloc always returns the best
      // possible alignment for every type, even for vectors
      pto_in_alignment_req = offset_congr <= alignment_req_congr;
      alignment_req_in_pto = alignment_req_congr <= offset_congr;

#ifdef DEBUG
      std::cerr << "dynamic alloc memory location" << std::endl;
      std::cerr << offset_congr << " <= " << alignment_req_congr
                << " == " << pto_in_alignment_req << std::endl;
      std::cerr << alignment_req_congr << " <= " << offset_congr
                << " == " << alignment_req_in_pto << std::endl;
#endif

    } else if (isa< va_arg_memory_location >(memloc)) {
      block_info.put("memloc-type", "va-arg");

      pto_in_alignment_req = offset_congr <= alignment_req_congr;
      alignment_req_in_pto = alignment_req_congr <= offset_congr;

#ifdef DEBUG
      std::cerr << "va arg memory location" << std::endl;
      std::cerr << offset_congr << " <= " << alignment_req_congr
                << " == " << pto_in_alignment_req << std::endl;
      std::cerr << alignment_req_congr << " <= " << offset_congr
                << " == " << alignment_req_in_pto << std::endl;
#endif

    } else if (isa< aggregate_memory_location >(memloc)) {
      block_info.put("memloc-type", "aggregate");
      return ERROR;
    }

    // - If the alignment is in the requirement, we are sure the requirement
    // is met
    // - If the alignment contains the requirement (meaning the requirement is
    // in the alignment), this may be a precision issue
    // - Otherwise, there is no common part between the found alignment and
    // the requirement, thus we are sure the requirement is not met
    if (pto_in_alignment_req) {
      return OK;
    } else if (alignment_req_in_pto) {
      return WARNING;
    } else {
      return ERROR;
    }
  }

  //! Check the alignment property of the pointer in ptr_op
  //  The Operand_ref must be a pointer
  void check_pointer_alignment(Operand_ref ptr_op,
                               AbsDomain inv,
                               const boost::optional< z_number >& alignment_req,
                               const std::string& call_context,
                               const boost::optional< Function_ref >& function,
                               const location& loc,
                               unsigned long stmt_uid) {
    LiteralFactory& lfac = this->_context.lit_factory();

    // Get the pointer variable_name
    scalar_lit_t ptr = lfac[ptr_op];

    analysis_result result = WARNING;
    json_dict info;

    // Check for unreachable statement
    if (ikos::exc_domain_traits::is_normal_flow_bottom(inv)) {
      result = UNREACHABLE;
      info.clear();

      if (display_check(result, ptr, loc)) {
        std::cerr << std::endl;
      }
    } else if (ptr.is_null()) {
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
      // Pointer checks
    } else if (ptr.is_pointer_var()) {
      variable_name* ptr_var = ptr.var();
      info.put("name", ptr_var->name());

      // Check for nullptr
      if (ikos::null_domain_traits::is_null(inv, ptr_var)) {
        result = ERROR;
        info.put("type", "null-pointer");

        if (display_check(result, ptr, loc)) {
          std::cerr << ": ptr is null" << std::endl;
        }
      } else if (ikos::uninit_domain_traits::is_uninitialized(inv, ptr_var)) {
        result = ERROR;
        info.put("type", "undefined-pointer");

        if (display_check(result, ptr, loc)) {
          std::cerr << ": ptr is uninitialized" << std::endl;
        }
        // No alignment requirement found, or always-safe alignment requirement
      } else if (!alignment_req || *alignment_req == 1) {
        result = OK;
        info.put("type", "no-requirement");

        if (display_check(result, ptr, loc)) {
          std::cerr << ": pointer alignment always safe" << std::endl;
        }
      } else if (ar::isGlobalVar(ptr_op)) {
        info.put("type", "var");
        auto alignment =
            ikos::z_number(static_cast< mpz_class >(*alignment_req));
        ikos::congruence< number_t > alignment_req_congr(alignment,
                                                         ikos::z_number(0));
        auto global_var = ar::getGlobalVariable(ptr_op);
        ikos_assert(global_var);
        ikos::congruence< number_t >
            global_alignment(ikos::z_number(static_cast< mpz_class >(
                                 *ar::getAlignment(*global_var))),
                             ikos::z_number(0));

        auto pto_in_alignment_req = global_alignment <= alignment_req_congr;
        auto alignment_req_in_pto = alignment_req_congr <= global_alignment;

#ifdef DEBUG
        std::cerr << "global variable" << std::endl;
        std::cerr << global_alignment << " <= " << alignment_req_congr
                  << " == " << pto_in_alignment_req << std::endl;
        std::cerr << alignment_req_congr << " <= " << global_alignment
                  << " == " << alignment_req_in_pto << std::endl;
#endif

        if (pto_in_alignment_req) {
          result = OK;
          if (display_check(result, ptr, loc)) {
            std::cerr << ": global variable correctly aligned" << std::endl;
          }
        } else if (alignment_req_in_pto) {
          result = WARNING;
          if (display_check(result, ptr, loc)) {
            std::cerr << ": global variable might be unaligned" << std::endl;
          }
        } else {
          result = ERROR;
          if (display_check(result, ptr, loc)) {
            std::cerr << ": global variable is unaligned" << std::endl;
          }
        }

        // No information on pointer
      } else if (ikos::ptr_domain_traits::is_unknown_addr(inv, ptr_var)) {
        result = WARNING;
        info.put("type", "no-points-to");

        if (display_check(result, ptr, loc)) {
          std::cerr << ": no points-to information for " << ptr_var->str()
                    << std::endl;
        }
      } else {
        info.put("type", "var");
        ikos::discrete_domain< memloc_t > points_to_set =
            ikos::ptr_domain_traits::addrs_set< AbsDomain, memloc_t >(inv,
                                                                      ptr_var);
        assert(!points_to_set.is_top());

        auto alignment =
            ikos::z_number(static_cast< mpz_class >(*alignment_req));
        ikos::congruence< number_t > alignment_req_congr(alignment,
                                                         ikos::z_number(0));
        json_list points_to_info;
        varname_t ptr_offset_var = inv.offset_var(ptr_var);
        typename ikos::congruence< number_t > offset_congr =
            ikos::num_domain_traits::to_congruence(inv, ptr_offset_var);

        std::ostringstream buf_req;
        buf_req << alignment_req_congr;
        info.put("requirement", buf_req.str());

        std::ostringstream buf_offset;
        buf_offset << offset_congr;
        info.put("offset", buf_offset.str());

        // Are all the points_to in/valid
        bool all_valid = true;
        bool all_invalid = true;

        // The goal is to check the following property:
        //   points_to_set(ptr) != TOP &&
        //   ∀a ∈ points_to_set(ptr), ∀o ∈ offset, a + o ≡ 0 [alignment_req]
        //   (eq. to a + o % alignment_req == 0)
        //
        // Iterate through the points-to set of the pointer
        // - If every memory location has an alignment greater or equal to the
        // requirement, then it's OK.
        // - If every memory location is unaligned based on the requirement,
        // set to ERROR state.
        // - Otherwise, it's a WARNING

        for (auto it = points_to_set.begin(); it != points_to_set.end(); ++it) {
          // Add info to json
          json_dict block_info = {{"addr", (*it)->str()}};

          // Is the points_to correctly aligned?
          analysis_result is_correctly_aligned =
              check_memloc_alignment(*it,
                                     offset_congr,
                                     alignment_req_congr,
                                     block_info);

          if (is_correctly_aligned == OK) {
            all_invalid = false;

            if (display_check(is_correctly_aligned, ptr, loc)) {
              std::cerr << ": memory location (" << (*it)->str()
                        << ") with offset (" << offset_congr
                        << ") is correctly aligned" << std::endl;
            }
          } else if (is_correctly_aligned == ERROR) {
            all_valid = false;

            if (display_check(is_correctly_aligned, ptr, loc)) {
              std::cerr << ": memory location (" << (*it)->str()
                        << ") with offset (" << offset_congr << ") is unaligned"
                        << std::endl;
            }
          } else {
            all_valid = false;
            all_invalid = false;

            if (display_check(is_correctly_aligned, ptr, loc)) {
              std::cerr << ": memory location (" << (*it)->str()
                        << ") with offset (" << offset_congr
                        << ") may be unaligned" << std::endl;
            }
          }

          points_to_info.add(block_info);
        }

        info.put("points-to", points_to_info);

        if (all_valid) {
          result = OK;
          if (display_check(result, ptr, loc)) {
            std::cerr << ": pointer is aligned" << std::endl;
          }
        } else if (all_invalid) {
          result = ERROR;
          if (display_check(result, ptr, loc)) {
            std::cerr << ": pointer is unaligned" << std::endl;
          }
        } else {
          result = WARNING;
          if (display_check(result, ptr, loc)) {
            std::cerr << ": pointer may be unaligned" << std::endl;
          }
        }
      }
    } else {
      throw analyzer_error("pointer_alignment_checker: unexpected operand");
    }

    this->display_invariant(result, inv, loc);
    this->_results
        .write("upa", result, call_context, function, loc, stmt_uid, info);
  }

}; // end class pointer_alignment_checker

} // end namespace analyzer

#endif // ANALYZER_POINTER_ALIGNMENT_CHECKER_HPP
