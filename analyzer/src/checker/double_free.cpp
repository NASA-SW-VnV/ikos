/*******************************************************************************
 *
 * \file
 * \brief Double free checker implementation
 *
 * Author: Thomas Bailleux
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2018-2019 United States Government as represented by the
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

#include <ikos/analyzer/checker/double_free.hpp>
#include <ikos/analyzer/support/cast.hpp>

namespace ikos {
namespace analyzer {

DoubleFreeChecker::DoubleFreeChecker(Context& ctx) : Checker(ctx) {}

CheckerName DoubleFreeChecker::name() const {
  return CheckerName::DoubleFree;
}

const char* DoubleFreeChecker::description() const {
  return "Double free checker";
}

void DoubleFreeChecker::check(ar::Statement* stmt,
                              const value::AbstractDomain& inv,
                              CallContext* call_context) {
  if (auto call = dyn_cast< ar::IntrinsicCall >(stmt)) {
    if (call->intrinsic_id() == ar::Intrinsic::LibcFree ||
        call->intrinsic_id() == ar::Intrinsic::LibcppDelete ||
        call->intrinsic_id() == ar::Intrinsic::LibcppDeleteArray ||
        call->intrinsic_id() == ar::Intrinsic::LibcppFreeException) {
      CheckResult check = this->check_double_free(call, inv);
      this->display_invariant(check.result, stmt, inv);
      this->_checks.insert(check.kind,
                           CheckerName::DoubleFree,
                           check.result,
                           stmt,
                           call_context,
                           check.operands,
                           check.info);
    }
  }
}

DoubleFreeChecker::CheckResult DoubleFreeChecker::check_double_free(
    ar::IntrinsicCall* stmt, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (this->display_double_free_check(Result::Unreachable, stmt)) {
      out() << std::endl;
    }
    return {CheckKind::Unreachable, Result::Unreachable, {}, {}};
  }

  ikos_assert(stmt->num_arguments() == 1);

  const auto operand = stmt->argument(0);
  const ScalarLit& ptr = this->_lit_factory.get_scalar(operand);

  if (ptr.is_undefined() ||
      (ptr.is_pointer_var() &&
       inv.normal().uninitialized().is_uninitialized(ptr.var()))) {
    if (this->display_double_free_check(Result::Error, stmt)) {
      out() << ": undefined operand" << std::endl;
    }
    return {CheckKind::UninitializedVariable, Result::Error, {operand}, {}};
  }

  if (ptr.is_null() ||
      (ptr.is_pointer_var() && inv.normal().nullity().is_null(ptr.var()))) {
    if (this->display_double_free_check(Result::Ok, stmt)) {
      out() << ": safe call to free with NULL value" << std::endl;
    }
    return {CheckKind::Free, Result::Ok, {operand}, {}};
  }

  PointsToSet addrs = inv.normal().pointers().points_to(ptr.var());

  if (addrs.is_empty()) {
    if (this->display_double_free_check(Result::Error, stmt)) {
      out() << ": empty points-to set for pointer";
    }
    return {CheckKind::InvalidPointerDereference, Result::Error, {operand}, {}};
  } else if (addrs.is_top()) {
    if (this->display_double_free_check(Result::Warning, stmt)) {
      out() << ": no points-to information for pointer" << std::endl;
    }
    return {CheckKind::IgnoredFree, Result::Warning, {operand}, {}};
  }

  bool all_error = true;
  bool all_ok = true;

  JsonDict info;
  JsonList points_to_info;

  for (const auto& addr : addrs) {
    JsonDict block_info = {
        {"id", _ctx.output_db->memory_locations.insert(addr)}};
    auto result = this->check_memory_location_free(stmt, inv, addr);
    block_info.put("status", static_cast< int >(result));

    if (result == Result::Ok) {
      all_error = false;
    } else if (result == Result::Warning) {
      all_error = false;
      all_ok = false;
    } else {
      all_ok = false;
    }
    points_to_info.add(block_info);
  }

  info.put("points_to", points_to_info);

  if (all_error) {
    // Unsafe
    return {CheckKind::Free, Result::Error, {operand}, info};
  } else if (all_ok) {
    // Safe
    return {CheckKind::Free, Result::Ok, {operand}, {}};
  } else {
    // Warning
    return {CheckKind::Free, Result::Warning, {operand}, info};
  }
}

Result DoubleFreeChecker::check_memory_location_free(
    ar::IntrinsicCall* stmt,
    const value::AbstractDomain& inv,
    MemoryLocation* addr) {
  if (isa< DynAllocMemoryLocation >(addr)) {
    auto lifetime = inv.normal().lifetime().get(addr);
    if (lifetime.is_deallocated()) {
      // This is a double free
      if (this->display_double_free_check(Result::Error, stmt, addr)) {
        out() << ": double free" << std::endl;
      }
      return Result::Error;
    } else if (lifetime.is_top()) {
      // A double free could be possible
      if (this->display_double_free_check(Result::Warning, stmt, addr)) {
        out() << ": possible double free" << std::endl;
      }
      return Result::Warning;
    } else {
      // Safe
      if (this->display_double_free_check(Result::Ok, stmt, addr)) {
        out() << ": safe call to free()" << std::endl;
      }
      return Result::Ok;
    }
  } else {
    // This is a free() call on something which isn't a dynamic allocated
    // memory.
    // This is an error
    if (this->display_double_free_check(Result::Error, stmt, addr)) {
      out() << ": free() called on a non-dynamic allocated memory" << std::endl;
    }
    return Result::Error;
  }
}

bool DoubleFreeChecker::display_double_free_check(
    Result result, ar::IntrinsicCall* stmt) const {
  if (this->display_check(result, stmt)) {
    out() << "check_dfa(";
    stmt->dump(out());
    out() << ")";
    return true;
  }
  return false;
}

bool DoubleFreeChecker::display_double_free_check(Result result,
                                                  ar::IntrinsicCall* stmt,
                                                  MemoryLocation* addr) const {
  if (this->display_check(result, stmt)) {
    out() << "check_dfa(";
    stmt->dump(out());
    out() << ", addr=";
    addr->dump(out());
    out() << ")";
    return true;
  }
  return false;
}

} // end namespace analyzer
} // namespace ikos
