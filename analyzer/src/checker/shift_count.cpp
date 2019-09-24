/*******************************************************************************
 *
 * \file
 * \brief Shift count checker implementation
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
#include <ikos/analyzer/checker/shift_count.hpp>
#include <ikos/analyzer/json/helper.hpp>
#include <ikos/analyzer/support/cast.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {

ShiftCountChecker::ShiftCountChecker(Context& ctx) : Checker(ctx) {}

CheckerName ShiftCountChecker::name() const {
  return CheckerName::ShiftCount;
}

const char* ShiftCountChecker::description() const {
  return "Shift count checker";
}

void ShiftCountChecker::check(ar::Statement* stmt,
                              const value::AbstractDomain& inv,
                              CallContext* call_context) {
  if (auto bin = dyn_cast< ar::BinaryOperation >(stmt)) {
    if (bin->op() == ar::BinaryOperation::SShl ||
        bin->op() == ar::BinaryOperation::UShl ||
        bin->op() == ar::BinaryOperation::SLShr ||
        bin->op() == ar::BinaryOperation::ULShr ||
        bin->op() == ar::BinaryOperation::SAShr ||
        bin->op() == ar::BinaryOperation::UAShr) {
      CheckResult check = this->check_shift_count(bin, inv);
      this->display_invariant(check.result, stmt, inv);
      this->_checks.insert(check.kind,
                           CheckerName::ShiftCount,
                           check.result,
                           stmt,
                           call_context,
                           std::array< ar::Value*, 1 >{{bin->right()}},
                           check.info);
    }
  }
}

ShiftCountChecker::CheckResult ShiftCountChecker::check_shift_count(
    ar::BinaryOperation* stmt, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg = this->display_shift_count_check(Result::Unreachable, stmt)) {
      *msg << "\n";
    }
    return {CheckKind::Unreachable, Result::Unreachable, {}};
  }

  const ScalarLit& shift_count = this->_lit_factory.get_scalar(stmt->right());

  auto shift_count_interval = IntInterval::bottom(1, Signed);
  if (shift_count.is_undefined() ||
      (shift_count.is_machine_int_var() &&
       inv.normal().uninit_is_uninitialized(shift_count.var()))) {
    if (auto msg = this->display_shift_count_check(Result::Error, stmt)) {
      *msg << ": undefined shift count\n";
    }
    return {CheckKind::UninitializedVariable, Result::Error, {}};
  } else if (shift_count.is_machine_int()) {
    shift_count_interval = IntInterval(shift_count.machine_int());
  } else if (shift_count.is_machine_int_var()) {
    shift_count_interval = inv.normal().int_to_interval(shift_count.var());
  } else {
    log::error("unexpected shit count operand");
    return {CheckKind::UnexpectedOperand, Result::Error, {}};
  }

  auto type = cast< ar::IntegerType >(stmt->result()->type());
  MachineInt zero = MachineInt::zero(type->bit_width(), type->sign());
  MachineInt limit =
      MachineInt(type->bit_width() - 1, type->bit_width(), type->sign());

  if (shift_count_interval.ub() < zero) {
    if (auto msg = this->display_shift_count_check(Result::Error, stmt)) {
      *msg << ": ∀c ∈ shift_count, c < 0\n";
    }
    return {CheckKind::ShiftCount,
            Result::Error,
            to_json(shift_count_interval)};
  } else if (shift_count_interval.lb() < zero) {
    if (auto msg = this->display_shift_count_check(Result::Warning, stmt)) {
      *msg << ": ∃c ∈ shift_count, c < 0\n";
    }
    return {CheckKind::ShiftCount,
            Result::Warning,
            to_json(shift_count_interval)};
  } else if (shift_count_interval.lb() > limit) {
    if (auto msg = this->display_shift_count_check(Result::Error, stmt)) {
      *msg << ": ∀c ∈ shift_count, c >= " << type->bit_width() << "\n";
    }
    return {CheckKind::ShiftCount,
            Result::Error,
            to_json(shift_count_interval)};
  } else if (shift_count_interval.ub() > limit) {
    if (auto msg = this->display_shift_count_check(Result::Warning, stmt)) {
      *msg << ": ∃c ∈ shift_count, c >= " << type->bit_width() << "\n";
    }
    return {CheckKind::ShiftCount,
            Result::Warning,
            to_json(shift_count_interval)};
  } else {
    if (auto msg = this->display_shift_count_check(Result::Ok, stmt)) {
      *msg << ": ∀c ∈ shift_count, 0 <= c < " << type->bit_width() << "\n";
    }
    return {CheckKind::ShiftCount, Result::Ok, {}};
  }
}

llvm::Optional< LogMessage > ShiftCountChecker::display_shift_count_check(
    Result result, ar::BinaryOperation* stmt) const {
  auto msg = this->display_check(result, stmt);
  if (msg) {
    *msg << "check_shc(";
    stmt->dump(msg->stream());
    *msg << ")";
  }
  return msg;
}

} // end namespace analyzer
} // end namespace ikos
