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

#include <ikos/ar/verify/type.hpp>

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
    CheckResult check = this->check_null(stmt, load->operand(), inv);
    this->display_invariant(check.result, stmt, inv);
    this->_checks.insert(check.kind,
                         CheckerName::NullPointerDereference,
                         check.result,
                         stmt,
                         call_context,
                         check.operands);
  } else if (auto store = dyn_cast< ar::Store >(stmt)) {
    CheckResult check = this->check_null(stmt, store->pointer(), inv);
    this->display_invariant(check.result, stmt, inv);
    this->_checks.insert(check.kind,
                         CheckerName::NullPointerDereference,
                         check.result,
                         stmt,
                         call_context,
                         check.operands);
  } else if (auto call = dyn_cast< ar::CallBase >(stmt)) {
    std::vector< CheckResult > checks = this->check_call(call, inv);
    for (const auto& check : checks) {
      this->display_invariant(check.result, stmt, inv);
      this->_checks.insert(check.kind,
                           CheckerName::NullPointerDereference,
                           check.result,
                           stmt,
                           call_context,
                           check.operands);
    }
  }
}

std::vector< NullDereferenceChecker::CheckResult > NullDereferenceChecker::
    check_call(ar::CallBase* call, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg = this->display_null_check(Result::Unreachable, call)) {
      *msg << "\n";
    }
    return {{CheckKind::Unreachable, Result::Unreachable, {}}};
  }

  const ScalarLit& called = this->_lit_factory.get_scalar(call->called());

  // Check uninitialized

  if (called.is_undefined() ||
      (called.is_pointer_var() &&
       inv.normal().uninit_is_uninitialized(called.var()))) {
    // Undefined call pointer operand
    if (auto msg =
            this->display_null_check(Result::Error, call, call->called())) {
      *msg << ": undefined call pointer operand\n";
    }
    return {
        {CheckKind::UninitializedVariable, Result::Error, {call->called()}}};
  }

  // Check null pointer dereference

  if (called.is_null() ||
      (called.is_pointer_var() && inv.normal().nullity_is_null(called.var()))) {
    // Null call pointer operand
    if (auto msg =
            this->display_null_check(Result::Error, call, call->called())) {
      *msg << ": null call pointer operand\n";
    }
    return {
        {CheckKind::NullPointerDereference, Result::Error, {call->called()}}};
  }

  // Collect potential callees
  auto callees = PointsToSet::bottom();

  if (auto cst = dyn_cast< ar::FunctionPointerConstant >(call->called())) {
    callees = {_ctx.mem_factory->get_function(cst->function())};
  } else if (isa< ar::InlineAssemblyConstant >(call->called())) {
    // call to inline assembly
    if (auto msg = this->display_null_check(Result::Ok, call, call->called())) {
      *msg << ": call to inline assembly\n";
    }
    return {{CheckKind::FunctionCallInlineAssembly, Result::Ok, {}}};
  } else if (auto gv = dyn_cast< ar::GlobalVariable >(call->called())) {
    callees = {_ctx.mem_factory->get_global(gv)};
  } else if (auto lv = dyn_cast< ar::LocalVariable >(call->called())) {
    callees = {_ctx.mem_factory->get_local(lv)};
  } else if (isa< ar::InternalVariable >(call->called())) {
    // Indirect call through a function pointer
    callees = inv.normal().pointer_to_points_to(called.var());
  } else {
    log::error("unexpected call pointer operand");
    return {{CheckKind::UnexpectedOperand, Result::Error, {call->called()}}};
  }

  // Check callees
  ikos_assert(!callees.is_bottom());

  if (callees.is_empty()) {
    // Invalid pointer dereference
    if (auto msg =
            this->display_null_check(Result::Error, call, call->called())) {
      *msg << ": points-to set of function pointer is empty\n";
    }
    return {{CheckKind::InvalidPointerDereference,
             Result::Error,
             {call->called()}}};
  }

  std::vector< CheckResult > checks = {
      this->check_null(call, call->called(), inv)};

  if (callees.is_top()) {
    // No points-to set
    if (auto msg =
            this->display_null_check(Result::Warning, call, call->called())) {
      *msg << ": no points-to set for function pointer\n";
    }
    checks.push_back({CheckKind::UnknownFunctionCallPointer,
                      Result::Warning,
                      {call->called()}});
    return checks;
  }

  for (MemoryLocation* addr : callees) {
    if (!isa< FunctionMemoryLocation >(addr)) {
      // Not a call to a function memory location
      continue;
    }

    ar::Function* callee = cast< FunctionMemoryLocation >(addr)->function();

    if (!ar::TypeVerifier::is_valid_call(call, callee->type())) {
      // Ill-formed function call
      continue;
    }

    if (callee->is_intrinsic()) {
      for (const auto& check : this->check_intrinsic_call(call, callee, inv)) {
        checks.push_back(check);
      }
    }
  }

  return checks;
}

std::vector< NullDereferenceChecker::CheckResult > NullDereferenceChecker::
    check_intrinsic_call(ar::CallBase* call,
                         ar::Function* fun,
                         const value::AbstractDomain& inv) {
  switch (fun->intrinsic_id()) {
    case ar::Intrinsic::MemoryCopy:
    case ar::Intrinsic::MemoryMove: {
      return {this->check_null(call, call->argument(0), inv),
              this->check_null(call, call->argument(1), inv)};
    }
    case ar::Intrinsic::MemorySet: {
      return {this->check_null(call, call->argument(0), inv)};
    }
    case ar::Intrinsic::VarArgStart:
    case ar::Intrinsic::VarArgEnd:
    case ar::Intrinsic::VarArgGet: {
      return {this->check_null(call, call->argument(0), inv)};
    }
    case ar::Intrinsic::VarArgCopy: {
      return {this->check_null(call, call->argument(0), inv),
              this->check_null(call, call->argument(1), inv)};
    }
    case ar::Intrinsic::StackSave:
    case ar::Intrinsic::StackRestore:
    case ar::Intrinsic::LifetimeStart:
    case ar::Intrinsic::LifetimeEnd:
    case ar::Intrinsic::EhTypeidFor:
    case ar::Intrinsic::Trap: {
      return {};
    }
    // <ikos/analyzer/intrinsic.h>
    case ar::Intrinsic::IkosAssert:
    case ar::Intrinsic::IkosAssume:
    case ar::Intrinsic::IkosNonDet:
    case ar::Intrinsic::IkosCounterInit:
    case ar::Intrinsic::IkosCounterIncr: {
      return {};
    }
    case ar::Intrinsic::IkosCheckMemAccess:
    case ar::Intrinsic::IkosCheckStringAccess:
    case ar::Intrinsic::IkosAssumeMemSize:
    case ar::Intrinsic::IkosForgetMemory:
    case ar::Intrinsic::IkosAbstractMemory:
    case ar::Intrinsic::IkosWatchMemory: {
      return {this->check_null(call, call->argument(0), inv)};
    }
    case ar::Intrinsic::IkosPartitioningVar:
    case ar::Intrinsic::IkosPartitioningJoin:
    case ar::Intrinsic::IkosPartitioningDisable:
    case ar::Intrinsic::IkosPrintInvariant:
    case ar::Intrinsic::IkosPrintValues: {
      return {};
    }
    // <stdlib.h>
    case ar::Intrinsic::LibcMalloc:
    case ar::Intrinsic::LibcCalloc:
    case ar::Intrinsic::LibcValloc:
    case ar::Intrinsic::LibcAlignedAlloc:
    case ar::Intrinsic::LibcRealloc:
    case ar::Intrinsic::LibcFree:
    case ar::Intrinsic::LibcAbs:
    case ar::Intrinsic::LibcRand:
    case ar::Intrinsic::LibcSrand:
    case ar::Intrinsic::LibcExit:
    case ar::Intrinsic::LibcAbort: {
      return {};
    }
    // <errno.h>
    case ar::Intrinsic::LibcErrnoLocation: {
      return {};
    }
    // <fcntl.h>
    case ar::Intrinsic::LibcOpen: {
      return {this->check_null(call, call->argument(0), inv)};
    }
    // <unistd.h>
    case ar::Intrinsic::LibcClose: {
      return {};
    }
    case ar::Intrinsic::LibcRead: {
      return {this->check_null(call, call->argument(1), inv)};
    }
    case ar::Intrinsic::LibcWrite: {
      return {this->check_null(call, call->argument(1), inv)};
    }
    // <stdio.h>
    case ar::Intrinsic::LibcGets: {
      return {this->check_null(call, call->argument(0), inv)};
    }
    case ar::Intrinsic::LibcFgets: {
      return {this->check_null(call, call->argument(0), inv),
              this->check_null(call, call->argument(2), inv)};
    }
    case ar::Intrinsic::LibcGetc:
    case ar::Intrinsic::LibcFgetc: {
      return {this->check_null(call, call->argument(0), inv)};
    }
    case ar::Intrinsic::LibcGetchar: {
      return {};
    }
    case ar::Intrinsic::LibcPuts: {
      return {this->check_null(call, call->argument(0), inv)};
    }
    case ar::Intrinsic::LibcFputs: {
      return {this->check_null(call, call->argument(0), inv),
              this->check_null(call, call->argument(1), inv)};
    }
    case ar::Intrinsic::LibcPutc:
    case ar::Intrinsic::LibcFputc: {
      return {this->check_null(call, call->argument(1), inv)};
    }
    case ar::Intrinsic::LibcPrintf: {
      return {this->check_null(call, call->argument(0), inv)};
    }
    case ar::Intrinsic::LibcFprintf: {
      return {this->check_null(call, call->argument(0), inv),
              this->check_null(call, call->argument(1), inv)};
    }
    case ar::Intrinsic::LibcSprintf: {
      return {this->check_null(call, call->argument(0), inv),
              this->check_null(call, call->argument(1), inv)};
    }
    case ar::Intrinsic::LibcSnprintf: {
      std::vector< CheckResult > checks = {};

      // Calling snprintf with zero bufsz and null pointer buffer can be used
      // to determine the buffer size needed to contain the output. That case
      // is allowed. Otherwise, check first argument.
      auto bufsz = this->_lit_factory.get_scalar(call->argument(1));
      if (!(bufsz.is_machine_int() && bufsz.machine_int().is_zero())) {
        checks.push_back(this->check_null(call, call->argument(0), inv));
      }

      checks.push_back(this->check_null(call, call->argument(2), inv));

      return checks;
    }
    case ar::Intrinsic::LibcScanf:
    case ar::Intrinsic::LibcFscanf:
    case ar::Intrinsic::LibcSscanf: {
      std::vector< CheckResult > checks;
      std::transform(call->arg_begin(),
                     call->arg_end(),
                     std::back_inserter(checks),
                     [=](ar::Value* arg) {
                       return this->check_null(call, arg, inv);
                     });
      return checks;
    }
    case ar::Intrinsic::LibcFopen: {
      return {this->check_null(call, call->argument(0), inv),
              this->check_null(call, call->argument(1), inv)};
    }
    case ar::Intrinsic::LibcFclose: {
      return {this->check_null(call, call->argument(0), inv)};
    }
    case ar::Intrinsic::LibcFflush: {
      return {this->check_null(call, call->argument(0), inv)};
    }
    // <string.h>
    case ar::Intrinsic::LibcStrlen:
    case ar::Intrinsic::LibcStrnlen: {
      return {this->check_null(call, call->argument(0), inv)};
    }
    case ar::Intrinsic::LibcStrcpy:
    case ar::Intrinsic::LibcStrncpy: {
      return {this->check_null(call, call->argument(0), inv),
              this->check_null(call, call->argument(1), inv)};
    }
    case ar::Intrinsic::LibcStrcat:
    case ar::Intrinsic::LibcStrncat: {
      return {this->check_null(call, call->argument(0), inv),
              this->check_null(call, call->argument(1), inv)};
    }
    case ar::Intrinsic::LibcStrcmp:
    case ar::Intrinsic::LibcStrncmp: {
      return {this->check_null(call, call->argument(0), inv),
              this->check_null(call, call->argument(1), inv)};
    }
    case ar::Intrinsic::LibcStrstr: {
      return {this->check_null(call, call->argument(0), inv),
              this->check_null(call, call->argument(1), inv)};
    }
    case ar::Intrinsic::LibcStrchr: {
      return {this->check_null(call, call->argument(0), inv)};
    }
    case ar::Intrinsic::LibcStrdup:
    case ar::Intrinsic::LibcStrndup: {
      return {this->check_null(call, call->argument(0), inv)};
    }
    case ar::Intrinsic::LibcStrcpyCheck:
    case ar::Intrinsic::LibcMemoryCopyCheck:
    case ar::Intrinsic::LibcMemoryMoveCheck: {
      return {this->check_null(call, call->argument(0), inv),
              this->check_null(call, call->argument(1), inv)};
    }
    case ar::Intrinsic::LibcMemorySetCheck: {
      return {this->check_null(call, call->argument(0), inv)};
    }
    case ar::Intrinsic::LibcStrcatCheck: {
      return {this->check_null(call, call->argument(0), inv),
              this->check_null(call, call->argument(1), inv)};
    }
    case ar::Intrinsic::LibcppNew:
    case ar::Intrinsic::LibcppNewArray:
    case ar::Intrinsic::LibcppDelete:
    case ar::Intrinsic::LibcppDeleteArray:
    case ar::Intrinsic::LibcppAllocateException:
    case ar::Intrinsic::LibcppFreeException:
    case ar::Intrinsic::LibcppThrow:
    case ar::Intrinsic::LibcppBeginCatch:
    case ar::Intrinsic::LibcppEndCatch: {
      return {};
    }
    default: {
      ikos_unreachable("unreachable");
    }
  }
}

NullDereferenceChecker::CheckResult NullDereferenceChecker::check_null(
    ar::Statement* stmt, ar::Value* operand, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg =
            this->display_null_check(Result::Unreachable, stmt, operand)) {
      *msg << "\n";
    }
    return {CheckKind::Unreachable, Result::Unreachable, {}};
  }

  const ScalarLit& ptr = this->_lit_factory.get_scalar(operand);

  if (ptr.is_undefined() || (ptr.is_pointer_var() &&
                             inv.normal().uninit_is_uninitialized(ptr.var()))) {
    // Undefined operand
    if (auto msg = this->display_null_check(Result::Error, stmt, operand)) {
      *msg << ": undefined operand\n";
    }
    return {CheckKind::UninitializedVariable, Result::Error, {operand}};
  }

  if (ptr.is_null()) {
    // Null operand
    if (auto msg = this->display_null_check(Result::Error, stmt, operand)) {
      *msg << ": null operand\n";
    }
    return {CheckKind::NullPointerDereference, Result::Error, {operand}};
  }

  if (!ptr.is_pointer_var()) {
    log::error("unexpected pointer operand");
    return {CheckKind::UnexpectedOperand, Result::Error, {operand}};
  }

  if (isa< ar::LocalVariable >(operand)) {
    // Local variable
    if (auto msg = this->display_null_check(Result::Ok, stmt, operand)) {
      *msg << ": dereferencing a local variable\n";
    }
    return {CheckKind::NullPointerDereference, Result::Ok, {operand}};
  } else if (isa< ar::GlobalVariable >(operand)) {
    // Global variable
    if (auto msg = this->display_null_check(Result::Ok, stmt, operand)) {
      *msg << ": dereferencing a global variable\n";
    }
    return {CheckKind::NullPointerDereference, Result::Ok, {operand}};
  } else if (isa< ar::InlineAssemblyConstant >(operand)) {
    // Inline Assembly
    if (auto msg = this->display_null_check(Result::Ok, stmt, operand)) {
      *msg << ": dereferencing an inline assembly\n";
    }
    return {CheckKind::NullPointerDereference, Result::Ok, {operand}};
  } else if (isa< ar::FunctionPointerConstant >(operand)) {
    // Function pointer constant
    if (auto msg = this->display_null_check(Result::Ok, stmt, operand)) {
      *msg << ": dereferencing a function pointer\n";
    }
    return {CheckKind::NullPointerDereference, Result::Ok, {operand}};
  }

  core::Nullity nullity = inv.normal().nullity_to_nullity(ptr.var());
  if (nullity.is_null()) {
    // Pointer is definitely null
    if (auto msg = this->display_null_check(Result::Error, stmt, operand)) {
      *msg << ": pointer is null\n";
    }
    return {CheckKind::NullPointerDereference, Result::Error, {operand}};
  } else if (nullity.is_non_null()) {
    // Pointer is definitely non-null
    if (auto msg = this->display_null_check(Result::Ok, stmt, operand)) {
      *msg << ": pointer is non null\n";
    }
    return {CheckKind::NullPointerDereference, Result::Ok, {operand}};
  } else {
    // Pointer may be null
    if (auto msg = this->display_null_check(Result::Warning, stmt, operand)) {
      *msg << ": pointer may be null\n";
    }
    return {CheckKind::NullPointerDereference, Result::Warning, {operand}};
  }
}

llvm::Optional< LogMessage > NullDereferenceChecker::display_null_check(
    Result result, ar::Statement* stmt) const {
  auto msg = this->display_check(result, stmt);
  if (msg) {
    *msg << "check_null_dereference(";
    stmt->dump(msg->stream());
    *msg << ")";
  }
  return msg;
}

llvm::Optional< LogMessage > NullDereferenceChecker::display_null_check(
    Result result, ar::Statement* stmt, ar::Value* operand) const {
  auto msg = this->display_check(result, stmt);
  if (msg) {
    *msg << "check_null_dereference(";
    operand->dump(msg->stream());
    *msg << ")";
  }
  return msg;
}

} // end namespace analyzer
} // end namespace ikos
