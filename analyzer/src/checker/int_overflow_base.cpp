/*******************************************************************************
 *
 * \file
 * \brief Base for integer overflow checker implementation
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
#include <ikos/analyzer/checker/signed_int_overflow.hpp>
#include <ikos/analyzer/json/helper.hpp>
#include <ikos/analyzer/support/cast.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {

/// \brief Return a text representation of the given binary operator
static char op_char(ar::BinaryOperation::Operator op) {
  switch (op) {
    case ar::BinaryOperation::SAdd:
    case ar::BinaryOperation::UAdd:
      return '+';
    case ar::BinaryOperation::SSub:
    case ar::BinaryOperation::USub:
      return '-';
    case ar::BinaryOperation::SMul:
    case ar::BinaryOperation::UMul:
      return '*';
    case ar::BinaryOperation::SDiv:
    case ar::BinaryOperation::UDiv:
      return '/';
    case ar::BinaryOperation::SRem:
    case ar::BinaryOperation::URem:
      return '%';
    default:
      ikos_unreachable("unexpected binary operator");
  }
}

IntOverflowCheckerBase::IntOverflowCheckerBase(Context& ctx) : Checker(ctx) {}

void IntOverflowCheckerBase::check_integer_overflow(
    ar::BinaryOperation* stmt,
    const value::AbstractDomain& inv,
    CallContext* call_context) {
  llvm::SmallVector< CheckResult, 2 > checks =
      this->check_integer_overflow(stmt, inv);
  for (const auto& check : checks) {
    this->display_invariant(check.result, stmt, inv);
    this->_checks.insert(check.kind,
                         this->name(),
                         check.result,
                         stmt,
                         call_context,
                         check.operands,
                         check.info);
  }
}

llvm::SmallVector< IntOverflowCheckerBase::CheckResult, 2 >
IntOverflowCheckerBase::check_integer_overflow(
    ar::BinaryOperation* stmt, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg =
            this->display_int_overflow_check(Result::Unreachable, stmt)) {
      *msg << "\n";
    }
    return {{CheckKind::Unreachable, Result::Unreachable, {}, {}}};
  }

  if (stmt->result()->type()->is_vector()) {
    return {}; // TODO(marthaud): Support checks on vector operations
  }

  const ScalarLit& left_lit = this->_lit_factory.get_scalar(stmt->left());
  const ScalarLit& right_lit = this->_lit_factory.get_scalar(stmt->right());

  auto left_interval = IntInterval::bottom(1, Signed);
  auto right_interval = IntInterval::bottom(1, Signed);

  if (left_lit.is_undefined() ||
      (left_lit.is_machine_int_var() &&
       inv.normal().uninit_is_uninitialized(left_lit.var()))) {
    // Undefined operand
    if (auto msg = this->display_int_overflow_check(Result::Error, stmt)) {
      *msg << ": undefined left operand\n";
    }
    return {
        {CheckKind::UninitializedVariable, Result::Error, {stmt->left()}, {}}};
  } else if (left_lit.is_machine_int()) {
    left_interval = IntInterval(left_lit.machine_int());
  } else if (left_lit.is_machine_int_var()) {
    left_interval = inv.normal().int_to_interval(left_lit.var());
  } else {
    log::error("unexpected operand to binary operation");
    return {{CheckKind::UnexpectedOperand, Result::Error, {stmt->left()}, {}}};
  }

  if (right_lit.is_undefined() ||
      (right_lit.is_machine_int_var() &&
       inv.normal().uninit_is_uninitialized(right_lit.var()))) {
    // Undefined operand
    if (auto msg = this->display_int_overflow_check(Result::Error, stmt)) {
      *msg << ": undefined right operand\n";
    }
    return {
        {CheckKind::UninitializedVariable, Result::Error, {stmt->right()}, {}}};
  } else if (right_lit.is_machine_int()) {
    right_interval = IntInterval(right_lit.machine_int());
  } else if (right_lit.is_machine_int_var()) {
    right_interval = inv.normal().int_to_interval(right_lit.var());
  } else {
    log::error("unexpected operand to binary operation");
    return {{CheckKind::UnexpectedOperand, Result::Error, {stmt->right()}, {}}};
  }

  auto result_interval = ZInterval::bottom();

  // Computes final interval, depending on the binary operator
  if (stmt->op() == ar::BinaryOperation::SAdd ||
      stmt->op() == ar::BinaryOperation::UAdd) {
    result_interval =
        left_interval.to_z_interval() + right_interval.to_z_interval();
  } else if (stmt->op() == ar::BinaryOperation::SSub ||
             stmt->op() == ar::BinaryOperation::USub) {
    result_interval =
        left_interval.to_z_interval() - right_interval.to_z_interval();
  } else if (stmt->op() == ar::BinaryOperation::SMul ||
             stmt->op() == ar::BinaryOperation::UMul) {
    result_interval =
        left_interval.to_z_interval() * right_interval.to_z_interval();
  } else if (stmt->op() == ar::BinaryOperation::SDiv ||
             stmt->op() == ar::BinaryOperation::UDiv) {
    result_interval =
        left_interval.to_z_interval() / right_interval.to_z_interval();
  } else if (stmt->op() == ar::BinaryOperation::URem ||
             stmt->op() == ar::BinaryOperation::SRem) {
    // signed remainder INT_MIN % -1 is undefined behavior
    // we're using division to check this case, because
    // we're checking INT_MIN / -1
    result_interval =
        left_interval.to_z_interval() / right_interval.to_z_interval();
  } else {
    ikos_unreachable("unexpected operator");
  }

  // No result because of division by zero
  if (result_interval.is_bottom()) {
    if (auto msg = this->display_int_overflow_check(Result::Error, stmt)) {
      *msg << ": division by zero\n";
    }
    return {{CheckKind::DivisionByZero, Result::Error, {stmt->right()}, {}}};
  }

  auto type = cast< ar::IntegerType >(stmt->result()->type());
  ZBound max(MachineInt::max(type->bit_width(), type->sign()).to_z_number());
  ZBound min(MachineInt::min(type->bit_width(), type->sign()).to_z_number());

  const ZBound& lb = result_interval.lb();
  const ZBound& ub = result_interval.ub();

  Result result_underflow;
  Result result_overflow;

  JsonDict info;
  info.put("left", to_json(left_interval));
  info.put("right", to_json(right_interval));

  if (lb > max) {
    result_underflow = Result::Ok;
    result_overflow = Result::Error;
    if (auto msg = this->display_int_overflow_check(Result::Error, stmt)) {
      *msg << ": ∀ a, b ∈ left x right, left " << op_char(stmt->op())
           << " right > INT_MAX\n";
    }
  } else if (ub < min) {
    result_underflow = Result::Error;
    result_overflow = Result::Ok;
    if (auto msg = this->display_int_overflow_check(Result::Error, stmt)) {
      *msg << ": ∀ a, b ∈ left x right, left " << op_char(stmt->op())
           << " right < INT_MIN\n";
    }
  } else {
    result_underflow = (lb < min) ? Result::Warning : Result::Ok;
    result_overflow = (ub > max) ? Result::Warning : Result::Ok;
    if (auto msg = this->display_int_overflow_check(result_underflow, stmt)) {
      *msg << " [underflow]: ";
      if (result_underflow == Result::Warning) {
        *msg << "lower_bound < min\n";
      } else {
        *msg << "lower_bound >= min\n";
      }
    }
    if (auto msg = this->display_int_overflow_check(result_overflow, stmt)) {
      *msg << " [overflow]: ";
      if (result_overflow == Result::Warning) {
        *msg << "upper_bound > max\n";
      } else {
        *msg << "upper_bound <= max\n";
      }
    }
  }

  return {{this->underflow_check_kind(),
           result_underflow,
           {stmt->left(), stmt->right()},
           ((result_underflow != Result::Ok) ? info : JsonDict())},
          {this->overflow_check_kind(),
           result_overflow,
           {stmt->left(), stmt->right()},
           ((result_overflow != Result::Ok) ? info : JsonDict())}};
}

llvm::Optional< LogMessage > IntOverflowCheckerBase::display_int_overflow_check(
    Result result, ar::BinaryOperation* stmt) const {
  auto msg = this->display_check(result, stmt);
  if (msg) {
    *msg << "check_" << this->short_name() << "(";
    stmt->dump(msg->stream());
    *msg << ")";
  }
  return msg;
}

} // end namespace analyzer
} // end namespace ikos
