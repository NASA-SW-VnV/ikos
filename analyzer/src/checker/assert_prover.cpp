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
 * Copyright (c) 2011-2018 United States Government as represented by the
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

    switch (fun->intrinsic_id()) {
      case ar::Intrinsic::IkosAssert: {
        CheckResult check = this->check_assert(call, inv);
        this->display_invariant(check.result, call, inv);
        this->_checks.insert(check.kind,
                             CheckerName::AssertProver,
                             check.result,
                             stmt,
                             call_context,
                             std::array< ar::Value*, 1 >{{call->argument(0)}});
      } break;
      case ar::Intrinsic::IkosPrintInvariant: {
        this->exec_print_invariant(call, inv);
      } break;
      case ar::Intrinsic::IkosPrintValues: {
        this->exec_print_values(call, inv);
      } break;
      default: {
        break;
      }
    }
  }
}

AssertProverChecker::CheckResult AssertProverChecker::check_assert(
    ar::IntrinsicCall* call, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (this->display_assert_check(Result::Unreachable, call)) {
      out() << std::endl;
    }
    return {CheckKind::Unreachable, Result::Unreachable};
  }

  const ScalarLit& cond = this->_lit_factory.get_scalar(call->argument(0));

  if (cond.is_undefined() ||
      (cond.is_machine_int_var() &&
       inv.normal().uninitialized().is_uninitialized(cond.var()))) {
    // Undefined operand
    if (this->display_assert_check(Result::Error, call)) {
      out() << ": undefined operand" << std::endl;
    }
    return {CheckKind::UninitializedVariable, Result::Error};
  }

  IntInterval flag;
  if (cond.is_machine_int()) {
    flag = IntInterval(cond.machine_int());
  } else if (cond.is_machine_int_var()) {
    flag = inv.normal().integers().to_interval(cond.var());
  } else {
    log::error("unexpected parameter to __ikos_assert()");
    return {CheckKind::UnexpectedOperand, Result::Error};
  }

  boost::optional< MachineInt > v = flag.singleton();

  if (v && (*v).is_zero()) {
    // The condition is definitely 0
    if (this->display_assert_check(Result::Error, call)) {
      out() << ": ∀x ∈ " << cond << ", x == 0" << std::endl;
    }
    return {CheckKind::Assert, Result::Error};
  } else if (flag.contains(MachineInt(0, flag.bit_width(), flag.sign()))) {
    // The condition may be 0
    if (this->display_assert_check(Result::Warning, call)) {
      out() << ": (∃x ∈ " << cond << ", x == 0) and (∃x ∈ " << cond
            << ", x != 0)" << std::endl;
    }
    return {CheckKind::Assert, Result::Warning};
  } else {
    // The condition cannot be 0
    if (this->display_assert_check(Result::Ok, call)) {
      out() << ": ∀x ∈ " << cond << ", x != 0" << std::endl;
    }
    return {CheckKind::Assert, Result::Ok};
  }
}

void AssertProverChecker::exec_print_invariant(
    ar::IntrinsicCall* call, const value::AbstractDomain& inv) {
  this->display_stmt_location(call);
  out() << "__ikos_print_invariant():\n";
  inv.dump(out());
  out() << std::endl;
}

void AssertProverChecker::exec_print_values(ar::IntrinsicCall* call,
                                            const value::AbstractDomain& inv) {
  this->display_stmt_location(call);
  out() << "__ikos_print_values(";
  for (auto it = call->arg_begin(), et = call->arg_end(); it != et;) {
    (*it)->dump(out());
    ++it;
    if (it != et) {
      out() << ", ";
    }
  }
  out() << "):\n";

  if (inv.is_normal_flow_bottom()) {
    out() << "Invariant: ";
    inv.dump(out());
    out() << std::endl;
  } else {
    for (auto it = call->arg_begin(), et = call->arg_end(); it != et; ++it) {
      const ScalarLit& v = this->_lit_factory.get_scalar(*it);

      if (v.is_machine_int_var()) {
        out() << "\t";
        v.var()->dump(out());
        out() << " -> " << inv.normal().integers().to_interval(v.var()) << "\n";
      } else if (v.is_floating_point_var()) {
        // ignored for now
      } else if (v.is_pointer_var()) {
        // points-to
        PointsToSet points_to = inv.normal().pointers().points_to(v.var());
        out() << "\t";
        v.var()->dump(out());
        out() << " -> ";
        points_to.dump(out());
        out() << std::endl;

        // offset
        Variable* offset_var = inv.normal().pointers().offset_var(v.var());
        out() << "\t";
        offset_var->dump(out());
        out() << " -> " << inv.normal().integers().to_interval(offset_var)
              << "\n";

        // nullity
        Nullity nullity_val = inv.normal().nullity().get(v.var());
        out() << "\t";
        v.var()->dump(out());
        if (nullity_val.is_null()) {
          out() << " is null\n";
        } else if (nullity_val.is_non_null()) {
          out() << " is non-null\n";
        } else {
          out() << " may be null\n";
        }
      } else {
        log::error("__ikos_print_values() operand is not a variable");
        continue;
      }

      // initialized (available for all variables)
      Uninitialized uninit_val = inv.normal().uninitialized().get(v.var());
      out() << "\t";
      v.var()->dump(out());
      if (uninit_val.is_uninitialized()) {
        out() << " is uninitialized" << std::endl;
      } else if (uninit_val.is_initialized()) {
        out() << " is initialized" << std::endl;
      } else {
        out() << " may be uninitialized" << std::endl;
      }
    }
  }
}

bool AssertProverChecker::display_assert_check(Result result,
                                               ar::IntrinsicCall* call) const {
  if (this->display_check(result, call)) {
    out() << "__ikos_assert(";
    call->argument(0)->dump(out());
    out() << ")";
    return true;
  }
  return false;
}

} // end namespace analyzer
} // end namespace ikos
