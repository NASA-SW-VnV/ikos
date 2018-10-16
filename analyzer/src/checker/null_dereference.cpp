/*******************************************************************************
 *
 * \file
 * \brief Null dereference checker implementation
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
#include <ikos/analyzer/checker/null_dereference.hpp>
#include <ikos/analyzer/support/cast.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {

NullDereferenceChecker::NullDereferenceChecker(Context& ctx) : Checker(ctx) {}

CheckerName NullDereferenceChecker::name() const {
  return CheckerName::NullPointerDereference;
}

const char* NullDereferenceChecker::description() const {
  return "Null dereference checker";
}

void NullDereferenceChecker::check(ar::Statement* stmt,
                                   const value::AbstractDomain& inv,
                                   CallContext* call_context) {
  if (auto load = dyn_cast< ar::Load >(stmt)) {
    this->check_null(stmt, load->operand(), inv, call_context);
  }
  if (auto store = dyn_cast< ar::Store >(stmt)) {
    this->check_null(stmt, store->pointer(), inv, call_context);
  }
  if (auto call = dyn_cast< ar::CallBase >(stmt)) {
    this->check_null(stmt, call->called(), inv, call_context);
  }
  if (auto call = dyn_cast< ar::IntrinsicCall >(stmt)) {
    ar::Function* fun = call->called_function();

    switch (fun->intrinsic_id()) {
      case ar::Intrinsic::MemoryCopy:
      case ar::Intrinsic::MemoryMove: {
        this->check_null(stmt, call->argument(0), inv, call_context);
        this->check_null(stmt, call->argument(1), inv, call_context);
      } break;
      case ar::Intrinsic::MemorySet: {
        this->check_null(stmt, call->argument(0), inv, call_context);
      } break;
      case ar::Intrinsic::LibcStrlen:
      case ar::Intrinsic::LibcStrnlen: {
        this->check_null(stmt, call->argument(0), inv, call_context);
      } break;
      case ar::Intrinsic::LibcStrcpy:
      case ar::Intrinsic::LibcStrncpy: {
        this->check_null(stmt, call->argument(0), inv, call_context);
        this->check_null(stmt, call->argument(1), inv, call_context);
      } break;
      case ar::Intrinsic::LibcStrcat:
      case ar::Intrinsic::LibcStrncat: {
        this->check_null(stmt, call->argument(0), inv, call_context);
        this->check_null(stmt, call->argument(1), inv, call_context);
      } break;
      default: { break; }
    }
  }
}

void NullDereferenceChecker::check_null(ar::Statement* stmt,
                                        ar::Value* operand,
                                        const value::AbstractDomain& inv,
                                        CallContext* call_context) {
  CheckResult check = this->check_null(stmt, operand, inv);
  this->display_invariant(check.result, stmt, inv);
  this->_checks.insert(check.kind,
                       CheckerName::NullPointerDereference,
                       check.result,
                       stmt,
                       call_context,
                       std::array< ar::Value*, 1 >{{operand}});
}

NullDereferenceChecker::CheckResult NullDereferenceChecker::check_null(
    ar::Statement* stmt, ar::Value* operand, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (this->display_null_check(Result::Unreachable, stmt, operand)) {
      out() << std::endl;
    }
    return {CheckKind::Unreachable, Result::Unreachable};
  }

  const ScalarLit& ptr = this->_lit_factory.get_scalar(operand);

  if (ptr.is_undefined() ||
      (ptr.is_pointer_var() &&
       inv.normal().uninitialized().is_uninitialized(ptr.var()))) {
    // Undefined operand
    if (this->display_null_check(Result::Error, stmt, operand)) {
      out() << ": undefined operand" << std::endl;
    }
    return {CheckKind::UninitializedVariable, Result::Error};
  }

  if (ptr.is_null()) {
    // Null operand
    if (this->display_null_check(Result::Error, stmt, operand)) {
      out() << ": null operand" << std::endl;
    }
    return {CheckKind::NullPointerDereference, Result::Error};
  }

  if (!ptr.is_pointer_var()) {
    log::error("unexpected pointer operand");
    return {CheckKind::UnexpectedOperand, Result::Error};
  }

  if (isa< ar::LocalVariable >(operand)) {
    // Local variable
    if (this->display_null_check(Result::Ok, stmt, operand)) {
      out() << ": dereferencing a local variable" << std::endl;
    }
    return {CheckKind::NullPointerDereference, Result::Ok};
  } else if (isa< ar::GlobalVariable >(operand)) {
    // Global variable
    if (this->display_null_check(Result::Ok, stmt, operand)) {
      out() << ": dereferencing a global variable" << std::endl;
    }
    return {CheckKind::NullPointerDereference, Result::Ok};
  } else if (isa< ar::InlineAssemblyConstant >(operand)) {
    // Inline Assembly
    if (this->display_null_check(Result::Ok, stmt, operand)) {
      out() << ": dereferencing an inline assembly" << std::endl;
    }
    return {CheckKind::NullPointerDereference, Result::Ok};
  } else if (isa< ar::FunctionPointerConstant >(operand)) {
    // Function pointer constant
    if (this->display_null_check(Result::Ok, stmt, operand)) {
      out() << ": dereferencing a function pointer" << std::endl;
    }
    return {CheckKind::NullPointerDereference, Result::Ok};
  }

  core::Nullity null_val = inv.normal().nullity().get(ptr.var());
  if (null_val.is_null()) {
    // Pointer is definitely null
    if (this->display_null_check(Result::Error, stmt, operand)) {
      out() << ": pointer is null" << std::endl;
    }
    return {CheckKind::NullPointerDereference, Result::Error};
  } else if (null_val.is_non_null()) {
    // Pointer is definitely non-null
    if (this->display_null_check(Result::Ok, stmt, operand)) {
      out() << ": pointer is non null" << std::endl;
    }
    return {CheckKind::NullPointerDereference, Result::Ok};
  } else {
    // Pointer may be null
    if (this->display_null_check(Result::Warning, stmt, operand)) {
      out() << ": pointer may be null" << std::endl;
    }
    return {CheckKind::NullPointerDereference, Result::Warning};
  }
}

bool NullDereferenceChecker::display_null_check(Result result,
                                                ar::Statement* stmt,
                                                ar::Value* operand) const {
  if (this->display_check(result, stmt)) {
    out() << "check_null_dereference(";
    operand->dump(out());
    out() << ")";
    return true;
  }
  return false;
}

} // end namespace analyzer
} // end namespace ikos
