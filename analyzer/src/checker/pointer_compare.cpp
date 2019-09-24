/*******************************************************************************
 *
 * \file
 * \brief Pointer compare checker implementation
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

#include <ikos/analyzer/analysis/literal.hpp>
#include <ikos/analyzer/checker/pointer_compare.hpp>
#include <ikos/analyzer/support/cast.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {

PointerCompareChecker::PointerCompareChecker(Context& ctx) : Checker(ctx) {}

CheckerName PointerCompareChecker::name() const {
  return CheckerName::PointerCompare;
}

const char* PointerCompareChecker::description() const {
  return "Pointer compare checker";
}

void PointerCompareChecker::check(ar::Statement* stmt,
                                  const value::AbstractDomain& inv,
                                  CallContext* call_context) {
  if (auto cmp = dyn_cast< ar::Comparison >(stmt)) {
    if (cmp->is_pointer_predicate() &&
        cmp->predicate() != ar::Comparison::PEQ &&
        cmp->predicate() != ar::Comparison::PNE) {
      CheckResult check = this->check_pointer_compare(cmp, inv);
      this->display_invariant(check.result, stmt, inv);
      this->_checks.insert(check.kind,
                           CheckerName::PointerCompare,
                           check.result,
                           stmt,
                           call_context,
                           check.operands,
                           check.info);
    }
  }
}

PointerCompareChecker::CheckResult PointerCompareChecker::check_pointer_compare(
    ar::Comparison* stmt, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg =
            this->display_pointer_compare_check(Result::Unreachable, stmt)) {
      *msg << "\n";
    }
    return {CheckKind::Unreachable, Result::Unreachable, {}, {}};
  }

  const ScalarLit& left_ptr = this->_lit_factory.get_scalar(stmt->left());
  const ScalarLit& right_ptr = this->_lit_factory.get_scalar(stmt->right());

  // Check uninitialized operands

  if (left_ptr.is_undefined() ||
      (left_ptr.is_pointer_var() &&
       inv.normal().uninit_is_uninitialized(left_ptr.var()))) {
    if (auto msg = this->display_pointer_compare_check(Result::Error, stmt)) {
      *msg << ": undefined left operand\n";
    }
    return {CheckKind::UninitializedVariable,
            Result::Error,
            {stmt->left()},
            {}};
  } else if (right_ptr.is_undefined() ||
             (right_ptr.is_pointer_var() &&
              inv.normal().uninit_is_uninitialized(right_ptr.var()))) {
    if (auto msg = this->display_pointer_compare_check(Result::Error, stmt)) {
      *msg << ": undefined right operand\n";
    }
    return {CheckKind::UninitializedVariable,
            Result::Error,
            {stmt->right()},
            {}};
  }

  // Check for null operands

  if (left_ptr.is_null() || (left_ptr.is_pointer_var() &&
                             inv.normal().nullity_is_null(left_ptr.var()))) {
    if (auto msg = this->display_pointer_compare_check(Result::Error, stmt)) {
      *msg << ": null left operand\n";
    }
    return {CheckKind::NullPointerComparison,
            Result::Error,
            {stmt->left()},
            {}};
  } else if (right_ptr.is_null() ||
             (right_ptr.is_pointer_var() &&
              inv.normal().nullity_is_null(right_ptr.var()))) {
    if (auto msg = this->display_pointer_compare_check(Result::Error, stmt)) {
      *msg << ": null right operand\n";
    }
    return {CheckKind::NullPointerComparison,
            Result::Error,
            {stmt->right()},
            {}};
  }

  if (!left_ptr.is_pointer_var()) {
    log::error("unexpected operand to comparison");
    return {CheckKind::UnexpectedOperand, Result::Error, {stmt->left()}, {}};
  } else if (!right_ptr.is_pointer_var()) {
    log::error("unexpected operand to comparison");
    return {CheckKind::UnexpectedOperand, Result::Error, {stmt->right()}, {}};
  }

  auto left_addrs = PointsToSet::bottom();
  if (auto gv = dyn_cast< ar::GlobalVariable >(stmt->left())) {
    left_addrs = {_ctx.mem_factory->get_global(gv)};
  } else if (auto cst = dyn_cast< ar::FunctionPointerConstant >(stmt->left())) {
    left_addrs = {_ctx.mem_factory->get_function(cst->function())};
  } else {
    left_addrs = inv.normal().pointer_to_points_to(left_ptr.var());
  }

  auto right_addrs = PointsToSet::bottom();
  if (auto gv = dyn_cast< ar::GlobalVariable >(stmt->right())) {
    right_addrs = {_ctx.mem_factory->get_global(gv)};
  } else if (auto cst =
                 dyn_cast< ar::FunctionPointerConstant >(stmt->right())) {
    right_addrs = {_ctx.mem_factory->get_function(cst->function())};
  } else {
    right_addrs = inv.normal().pointer_to_points_to(right_ptr.var());
  }

  if (left_addrs.is_empty()) {
    if (auto msg = this->display_pointer_compare_check(Result::Error, stmt)) {
      *msg << ": empty points-to set for left operand\n";
    }
    return {CheckKind::InvalidPointerComparison,
            Result::Error,
            {stmt->left()},
            {}};
  } else if (right_addrs.is_empty()) {
    if (auto msg = this->display_pointer_compare_check(Result::Error, stmt)) {
      *msg << ": empty points-to set for right operand\n";
    }
    return {CheckKind::InvalidPointerComparison,
            Result::Error,
            {stmt->right()},
            {}};
  }

  PointsToSet join_addrs = left_addrs.join(right_addrs);
  PointsToSet meet_addrs = left_addrs.meet(right_addrs);

  Result result;
  if (meet_addrs.is_empty()) {
    result = Result::Error;
  } else if (join_addrs.is_top() || join_addrs.size() > 1) {
    result = Result::Warning;
  } else {
    result = Result::Ok;
  }

  if (auto msg = this->display_pointer_compare_check(result, stmt)) {
    *msg << ": left addresses=" << left_addrs
         << " right addresses=" << right_addrs << "\n";
  }

  JsonDict info;
  if (result != Result::Ok) {
    if (left_addrs.is_set()) {
      JsonList left_points_to;
      for (MemoryLocation* mem_loc : left_addrs) {
        left_points_to.add(_ctx.output_db->memory_locations.insert(mem_loc));
      }
      info.put("left_points_to", left_points_to);
    } else {
      ikos_assert(left_addrs.is_top());
    }

    if (right_addrs.is_set()) {
      JsonList right_points_to;
      for (MemoryLocation* mem_loc : right_addrs) {
        right_points_to.add(_ctx.output_db->memory_locations.insert(mem_loc));
      }
      info.put("right_points_to", right_points_to);
    } else {
      ikos_assert(right_addrs.is_top());
    }
  }

  return {CheckKind::PointerComparison,
          result,
          {stmt->left(), stmt->right()},
          info};
}

llvm::Optional< LogMessage > PointerCompareChecker::
    display_pointer_compare_check(Result result, ar::Comparison* stmt) const {
  auto msg = this->display_check(result, stmt);
  if (msg) {
    *msg << "check_pcmp(";
    stmt->dump(msg->stream());
    *msg << ")";
  }
  return msg;
}

} // end namespace analyzer
} // end namespace ikos
