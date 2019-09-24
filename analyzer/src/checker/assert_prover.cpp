/*******************************************************************************
 *
 * \file
 * \brief Assertion prover checker implementation
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2019 United States Government as represented by the
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

#include <ikos/analyzer/checker/assert_prover.hpp>
#include <ikos/analyzer/support/cast.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {

AssertProverChecker::AssertProverChecker(Context& ctx) : Checker(ctx) {}

CheckerName AssertProverChecker::name() const {
  return CheckerName::AssertProver;
}

const char* AssertProverChecker::description() const {
  return "Assertion prover checker";
}

void AssertProverChecker::check(ar::Statement* stmt,
                                const value::AbstractDomain& inv,
                                CallContext* call_context) {
  if (auto call = dyn_cast< ar::IntrinsicCall >(stmt)) {
    ar::Function* fun = call->called_function();

    if (fun->intrinsic_id() == ar::Intrinsic::IkosAssert) {
      CheckResult check = this->check_assert(call, inv);
      this->display_invariant(check.result, call, inv);
      this->_checks.insert(check.kind,
                           CheckerName::AssertProver,
                           check.result,
                           stmt,
                           call_context,
                           std::array< ar::Value*, 1 >{{call->argument(0)}});
    }
  }
}

AssertProverChecker::CheckResult AssertProverChecker::check_assert(
    ar::IntrinsicCall* call, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg = this->display_assert_check(Result::Unreachable, call)) {
      *msg << "\n";
    }
    return {CheckKind::Unreachable, Result::Unreachable};
  }

  const ScalarLit& cond = this->_lit_factory.get_scalar(call->argument(0));

  if (cond.is_undefined() ||
      (cond.is_machine_int_var() &&
       inv.normal().uninit_is_uninitialized(cond.var()))) {
    // Undefined operand
    if (auto msg = this->display_assert_check(Result::Error, call)) {
      *msg << ": undefined operand\n";
    }
    return {CheckKind::UninitializedVariable, Result::Error};
  }

  auto flag = IntInterval::bottom(32, Unsigned);
  if (cond.is_machine_int()) {
    flag = IntInterval(cond.machine_int());
  } else if (cond.is_machine_int_var()) {
    flag = inv.normal().int_to_interval(cond.var());
  } else {
    log::error("unexpected argument to __ikos_assert()");
    return {CheckKind::UnexpectedOperand, Result::Error};
  }

  boost::optional< MachineInt > v = flag.singleton();

  if (v && (*v).is_zero()) {
    // The condition is definitely 0
    if (auto msg = this->display_assert_check(Result::Error, call)) {
      *msg << ": ∀x ∈ " << cond << ", x == 0\n";
    }
    return {CheckKind::Assert, Result::Error};
  } else if (flag.contains(MachineInt::zero(32, Unsigned))) {
    // The condition may be 0
    if (auto msg = this->display_assert_check(Result::Warning, call)) {
      *msg << ": (∃x ∈ " << cond << ", x == 0) and (∃x ∈ " << cond
           << ", x != 0)\n";
    }
    return {CheckKind::Assert, Result::Warning};
  } else {
    // The condition cannot be 0
    if (auto msg = this->display_assert_check(Result::Ok, call)) {
      *msg << ": ∀x ∈ " << cond << ", x != 0\n";
    }
    return {CheckKind::Assert, Result::Ok};
  }
}

llvm::Optional< LogMessage > AssertProverChecker::display_assert_check(
    Result result, ar::IntrinsicCall* call) const {
  auto msg = this->display_check(result, call);
  if (msg) {
    *msg << "__ikos_assert(";
    call->argument(0)->dump(msg->stream());
    *msg << ")";
  }
  return msg;
}

} // end namespace analyzer
} // end namespace ikos
