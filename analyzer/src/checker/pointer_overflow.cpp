/*******************************************************************************
 *
 * \file
 * \brief Pointer overflow checker implementation
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
#include <ikos/analyzer/checker/pointer_overflow.hpp>
#include <ikos/analyzer/support/cast.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {

PointerOverflowChecker::PointerOverflowChecker(Context& ctx)
    : Checker(ctx), _data_layout(ctx.bundle->data_layout()) {}

CheckerName PointerOverflowChecker::name() const {
  return CheckerName::PointerOverflow;
}

const char* PointerOverflowChecker::description() const {
  return "Pointer overflow checker";
}

void PointerOverflowChecker::check(ar::Statement* stmt,
                                   const value::AbstractDomain& inv,
                                   CallContext* call_context) {
  if (auto bin = dyn_cast< ar::PointerShift >(stmt)) {
    CheckResult check = this->check_pointer_overflow(bin, inv);
    this->display_invariant(check.result, stmt, inv);
    this->_checks.insert(check.kind,
                         CheckerName::PointerOverflow,
                         check.result,
                         stmt,
                         call_context,
                         check.operands);
  }
}

PointerOverflowChecker::CheckResult PointerOverflowChecker::
    check_pointer_overflow(ar::PointerShift* stmt,
                           const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg =
            this->display_pointer_overflow_check(Result::Unreachable, stmt)) {
      *msg << "\n";
    }
    return {CheckKind::Unreachable, Result::Unreachable, {}};
  }

  const ScalarLit& base = this->_lit_factory.get_scalar(stmt->pointer());

  if (base.is_undefined() ||
      (base.is_pointer_var() &&
       inv.normal().uninit_is_uninitialized(base.var()))) {
    if (auto msg = this->display_pointer_overflow_check(Result::Error, stmt)) {
      *msg << ": undefined base operand\n";
    }
    return {CheckKind::UninitializedVariable, Result::Error, {stmt->pointer()}};
  }

  auto base_interval = ZInterval::bottom();
  if (isa< ar::NullConstant >(stmt->pointer()) ||
      isa< ar::GlobalVariable >(stmt->pointer()) ||
      isa< ar::LocalVariable >(stmt->pointer()) ||
      isa< ar::FunctionPointerConstant >(stmt->pointer())) {
    base_interval = ZInterval(0);
  } else if (isa< ar::InternalVariable >(stmt->pointer())) {
    base_interval =
        inv.normal().pointer_offset_to_interval(base.var()).to_z_interval();
  } else {
    log::error("unexpected operand to ptrshift");
    return {CheckKind::UnexpectedOperand, Result::Error, {stmt->pointer()}};
  }

  Result result = Result::Ok;
  auto top = IntInterval::top(this->_data_layout.pointers.bit_width, Unsigned)
                 .to_z_interval();
  ZBound max(MachineInt::max(this->_data_layout.pointers.bit_width, Unsigned)
                 .to_z_number());

  for (auto it = stmt->term_begin(), et = stmt->term_end();
       it != et && result != Result::Error;
       it++) {
    auto term = *it;
    auto factor_interval = ZInterval(term.first.to_z_number());
    const ScalarLit& offset = this->_lit_factory.get_scalar(term.second);
    auto offset_interval = ZInterval::bottom();

    if (offset.is_undefined() ||
        (offset.is_machine_int_var() &&
         inv.normal().uninit_is_uninitialized(offset.var()))) {
      if (auto msg =
              this->display_pointer_overflow_check(Result::Error, stmt)) {
        *msg << ": undefined operand\n";
      }
      return {CheckKind::UninitializedVariable, Result::Error, {term.second}};
    } else if (offset.is_machine_int()) {
      offset_interval = ZInterval(offset.machine_int().to_z_number());
    } else if (offset.is_machine_int_var()) {
      offset_interval =
          inv.normal().int_to_interval(offset.var()).to_z_interval();
    } else {
      log::error("unexpected operand to ptrshift");
      return {CheckKind::UnexpectedOperand, Result::Error, {term.second}};
    }

    base_interval += factor_interval * offset_interval;

    if (!base_interval.leq(top)) {
      // possible overflow
      if (base_interval.lb() <= max) {
        result = Result::Warning;
        if (auto msg = this->display_pointer_overflow_check(result, stmt)) {
          *msg << ": ∃ p ∈ base_interval | p > " << max << "\n";
        }
      } else {
        result = Result::Error;
        if (auto msg = this->display_pointer_overflow_check(result, stmt)) {
          *msg << ": ∀ p ∈ base_interval, p > " << max << "\n";
        }
        break;
      }
    } else {
      if (auto msg = this->display_pointer_overflow_check(result, stmt)) {
        *msg << ": ∀ p ∈ base_interval, p < " << max << "\n";
      }
    }
  }

  return {CheckKind::PointerOverflow,
          result,
          {stmt->op_begin(), stmt->op_end()}};
}

llvm::Optional< LogMessage > PointerOverflowChecker::
    display_pointer_overflow_check(Result result,
                                   ar::PointerShift* stmt) const {
  auto msg = this->display_check(result, stmt);
  if (msg) {
    *msg << "check_poa(";
    stmt->dump(msg->stream());
    *msg << ")";
  }
  return msg;
}

} // end namespace analyzer
} // end namespace ikos
