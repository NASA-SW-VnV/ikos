/*******************************************************************************
 *
 * \file
 * \brief Soundness checker implementation
 *
 * Author: Maxime Arthaud
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

#include <ikos/ar/verify/type.hpp>

#include <ikos/analyzer/checker/soundness.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {

SoundnessChecker::SoundnessChecker(Context& ctx) : Checker(ctx) {}

CheckerName SoundnessChecker::name() const {
  return CheckerName::Soundness;
}

const char* SoundnessChecker::description() const {
  return "Soundness checker";
}

void SoundnessChecker::check(ar::Statement* stmt,
                             const value::AbstractDomain& inv,
                             CallContext* call_context) {
  if (auto store = dyn_cast< ar::Store >(stmt)) {
    boost::optional< CheckResult > check =
        this->check_mem_write(store,
                              store->pointer(),
                              CheckKind::IgnoredStore,
                              inv);
    if (check) {
      this->display_invariant(check->result, stmt, inv);
      this->_checks.insert(check->kind,
                           CheckerName::Soundness,
                           check->result,
                           stmt,
                           call_context,
                           check->operands,
                           check->info);
    }
  } else if (auto call = dyn_cast< ar::CallBase >(stmt)) {
    std::vector< CheckResult > checks =
        this->check_call(call, inv, call_context);
    for (const auto& check : checks) {
      this->display_invariant(check.result, stmt, inv);
      this->_checks.insert(check.kind,
                           CheckerName::Soundness,
                           check.result,
                           stmt,
                           call_context,
                           check.operands,
                           check.info);
    }
  }
}

std::vector< SoundnessChecker::CheckResult > SoundnessChecker::check_call(
    ar::CallBase* call,
    const value::AbstractDomain& inv,
    CallContext* call_context) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg = this->display_soundness_check(Result::Unreachable, call)) {
      *msg << "\n";
    }
    return {{CheckKind::Unreachable, Result::Unreachable, {}, {}}};
  }

  const ScalarLit& called = this->_lit_factory.get_scalar(call->called());

  // Check uninitialized

  if (called.is_undefined() ||
      (called.is_pointer_var() &&
       inv.normal().uninit_is_uninitialized(called.var()))) {
    // Undefined call pointer operand
    if (auto msg = this->display_soundness_check(Result::Error, call)) {
      *msg << ": undefined call pointer operand\n";
    }
    return {{CheckKind::UninitializedVariable,
             Result::Error,
             {call->called()},
             {}}};
  }

  // Check null pointer dereference

  if (called.is_null() ||
      (called.is_pointer_var() && inv.normal().nullity_is_null(called.var()))) {
    // Null call pointer operand
    if (auto msg = this->display_soundness_check(Result::Error, call)) {
      *msg << ": null call pointer operand\n";
    }
    return {{CheckKind::NullPointerDereference,
             Result::Error,
             {call->called()},
             {}}};
  }

  // Collect potential callees
  auto callees = PointsToSet::bottom();

  if (auto cst = dyn_cast< ar::FunctionPointerConstant >(call->called())) {
    callees = {_ctx.mem_factory->get_function(cst->function())};
  } else if (isa< ar::InlineAssemblyConstant >(call->called())) {
    // call to inline assembly
    if (auto msg = this->display_soundness_check(Result::Ok, call)) {
      *msg << ": call to inline assembly\n";
    }
    return {{CheckKind::FunctionCallInlineAssembly, Result::Ok, {}, {}}};
  } else if (auto gv = dyn_cast< ar::GlobalVariable >(call->called())) {
    callees = {_ctx.mem_factory->get_global(gv)};
  } else if (auto lv = dyn_cast< ar::LocalVariable >(call->called())) {
    callees = {_ctx.mem_factory->get_local(lv)};
  } else if (isa< ar::InternalVariable >(call->called())) {
    // Indirect call through a function pointer
    callees = inv.normal().pointer_to_points_to(called.var());
  } else {
    log::error("unexpected call pointer operand");
    return {
        {CheckKind::UnexpectedOperand, Result::Error, {call->called()}, {}}};
  }

  // Check callees
  ikos_assert(!callees.is_bottom());

  if (callees.is_empty()) {
    // Invalid pointer dereference
    if (auto msg = this->display_soundness_check(Result::Error, call)) {
      *msg << ": points-to set of function pointer is empty\n";
    }
    return {{CheckKind::InvalidPointerDereference,
             Result::Error,
             {call->called()},
             {}}};
  } else if (callees.is_top()) {
    // No points-to set
    if (auto msg = this->display_soundness_check(Result::Warning, call)) {
      *msg << ": no points-to set for function pointer\n";
    }
    return {{CheckKind::UnknownFunctionCallPointer,
             Result::Warning,
             {call->called()},
             {}}};
  }

  ar::Function* caller = call->code()->function_or_null();

  std::vector< CheckResult > checks;

  for (MemoryLocation* addr : callees) {
    if (!isa< FunctionMemoryLocation >(addr)) {
      // Not a call to a function memory location
      continue;
    }

    ar::Function* callee = cast< FunctionMemoryLocation >(addr)->function();

    if (!ar::TypeVerifier::is_valid_call(call, callee->type())) {
      // Ill-formed function call
      continue;
    } else if (caller == callee || call_context->contains(callee)) {
      // Recursive function call
      checks.push_back(this->check_recursive_call(call, callee, inv));
    } else if (callee->is_intrinsic()) {
      for (const auto& check : this->check_intrinsic_call(call, callee, inv)) {
        checks.push_back(check);
      }
    } else if (callee->is_declaration()) {
      checks.push_back(this->check_unknown_extern_call(call, callee, inv));
    } else if (callee->is_definition()) {
      // This is sound
      continue;
    } else {
      ikos_unreachable("unreachable");
    }
  }

  return checks;
}

SoundnessChecker::CheckResult SoundnessChecker::check_recursive_call(
    ar::CallBase* call, ar::Function* fun, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg = this->display_soundness_check(Result::Unreachable, call)) {
      *msg << "\n";
    }
    return {CheckKind::Unreachable, Result::Unreachable, {}, {}};
  }

  if (auto msg = this->display_soundness_check(Result::Warning, call)) {
    *msg << ": call to a recursive function\n";
  }
  return {CheckKind::RecursiveFunctionCall,
          Result::Warning,
          {},
          JsonDict{{"fun_id", _ctx.output_db->functions.insert(fun)}}};
}

namespace {

/// \brief Convert a boost::optional to a std::vector
template < typename T >
inline std::vector< T > to_vector(boost::optional< T > check) {
  if (check) {
    return {std::move(*check)};
  } else {
    return {};
  }
}

} // end anonymous namespace

std::vector< SoundnessChecker::CheckResult > SoundnessChecker::
    check_intrinsic_call(ar::CallBase* call,
                         ar::Function* fun,
                         const value::AbstractDomain& inv) {
  switch (fun->intrinsic_id()) {
    case ar::Intrinsic::MemoryCopy: {
      return to_vector(this->check_mem_write(call,
                                             call->argument(0),
                                             CheckKind::IgnoredMemoryCopy,
                                             inv));
    }
    case ar::Intrinsic::MemoryMove: {
      return to_vector(this->check_mem_write(call,
                                             call->argument(0),
                                             CheckKind::IgnoredMemoryMove,
                                             inv));
    }
    case ar::Intrinsic::MemorySet: {
      return to_vector(this->check_mem_write(call,
                                             call->argument(0),
                                             CheckKind::IgnoredMemorySet,
                                             inv));
    }
    case ar::Intrinsic::VarArgStart:
    case ar::Intrinsic::VarArgEnd:
    case ar::Intrinsic::VarArgGet:
    case ar::Intrinsic::VarArgCopy: {
      return this->check_call_pointer_params(call,
                                             fun,
                                             {call->argument(0)},
                                             inv);
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
    case ar::Intrinsic::IkosCounterIncr:
    case ar::Intrinsic::IkosCheckMemAccess:
    case ar::Intrinsic::IkosCheckStringAccess:
    case ar::Intrinsic::IkosAssumeMemSize:
    case ar::Intrinsic::IkosForgetMemory:
    case ar::Intrinsic::IkosAbstractMemory:
    case ar::Intrinsic::IkosWatchMemory:
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
    case ar::Intrinsic::LibcAlignedAlloc: {
      return {};
    }
    case ar::Intrinsic::LibcRealloc: {
      return to_vector(this->check_free(call, call->argument(0), inv));
    }
    case ar::Intrinsic::LibcFree: {
      return to_vector(this->check_free(call, call->argument(0), inv));
    }
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
      return {};
    }
    // <unistd.h>
    case ar::Intrinsic::LibcClose: {
      return {};
    }
    case ar::Intrinsic::LibcRead: {
      return this->check_call_pointer_params(call,
                                             fun,
                                             {call->argument(1)},
                                             inv);
    }
    case ar::Intrinsic::LibcWrite: {
      return {};
    }
    // <stdio.h>
    case ar::Intrinsic::LibcGets: {
      return this->check_call_pointer_params(call,
                                             fun,
                                             {call->argument(0)},
                                             inv);
    }
    case ar::Intrinsic::LibcFgets: {
      return this->check_call_pointer_params(call,
                                             fun,
                                             {call->argument(0)},
                                             inv);
    }
    case ar::Intrinsic::LibcGetc:
    case ar::Intrinsic::LibcFgetc:
    case ar::Intrinsic::LibcGetchar:
    case ar::Intrinsic::LibcPuts:
    case ar::Intrinsic::LibcFputs:
    case ar::Intrinsic::LibcPutc:
    case ar::Intrinsic::LibcFputc:
    case ar::Intrinsic::LibcPrintf:
    case ar::Intrinsic::LibcFprintf: {
      return {};
    }
    case ar::Intrinsic::LibcSprintf: {
      return this->check_call_pointer_params(call,
                                             fun,
                                             {call->argument(0)},
                                             inv);
    }
    case ar::Intrinsic::LibcSnprintf: {
      return this->check_call_pointer_params(call,
                                             fun,
                                             {call->argument(0)},
                                             inv);
    }
    case ar::Intrinsic::LibcScanf: {
      return this->check_call_pointer_params(call,
                                             fun,
                                             {call->arg_begin() + 1,
                                              call->arg_end()},
                                             inv);
    }
    case ar::Intrinsic::LibcFscanf: {
      return this->check_call_pointer_params(call,
                                             fun,
                                             {call->arg_begin() + 2,
                                              call->arg_end()},
                                             inv);
    }
    case ar::Intrinsic::LibcSscanf: {
      return this->check_call_pointer_params(call,
                                             fun,
                                             {call->arg_begin() + 2,
                                              call->arg_end()},
                                             inv);
    }
    case ar::Intrinsic::LibcFopen: {
      return {};
    }
    case ar::Intrinsic::LibcFclose: {
      return to_vector(this->check_free(call, call->argument(0), inv));
    }
    case ar::Intrinsic::LibcFflush: {
      return {};
    }
    // <string.h>
    case ar::Intrinsic::LibcStrlen:
    case ar::Intrinsic::LibcStrnlen: {
      return {};
    }
    case ar::Intrinsic::LibcStrcpy:
    case ar::Intrinsic::LibcStrncpy:
    case ar::Intrinsic::LibcStrcat:
    case ar::Intrinsic::LibcStrncat: {
      return this->check_call_pointer_params(call,
                                             fun,
                                             {call->argument(0)},
                                             inv);
    }
    case ar::Intrinsic::LibcStrcmp:
    case ar::Intrinsic::LibcStrncmp:
    case ar::Intrinsic::LibcStrstr:
    case ar::Intrinsic::LibcStrchr:
    case ar::Intrinsic::LibcStrdup:
    case ar::Intrinsic::LibcStrndup: {
      return {};
    }
    case ar::Intrinsic::LibcStrcpyCheck: {
      return this->check_call_pointer_params(call,
                                             fun,
                                             {call->argument(0)},
                                             inv);
    }
    case ar::Intrinsic::LibcMemoryCopyCheck: {
      return to_vector(this->check_mem_write(call,
                                             call->argument(0),
                                             CheckKind::IgnoredMemoryCopy,
                                             inv));
    }
    case ar::Intrinsic::LibcMemoryMoveCheck: {
      return to_vector(this->check_mem_write(call,
                                             call->argument(0),
                                             CheckKind::IgnoredMemoryMove,
                                             inv));
    }
    case ar::Intrinsic::LibcMemorySetCheck: {
      return to_vector(this->check_mem_write(call,
                                             call->argument(0),
                                             CheckKind::IgnoredMemorySet,
                                             inv));
    }
    case ar::Intrinsic::LibcStrcatCheck: {
      return this->check_call_pointer_params(call,
                                             fun,
                                             {call->argument(0)},
                                             inv);
    }
    case ar::Intrinsic::LibcppNew:
    case ar::Intrinsic::LibcppNewArray: {
      return {};
    }
    case ar::Intrinsic::LibcppDelete:
    case ar::Intrinsic::LibcppDeleteArray: {
      return to_vector(this->check_free(call, call->argument(0), inv));
    }
    case ar::Intrinsic::LibcppAllocateException: {
      return {};
    }
    case ar::Intrinsic::LibcppFreeException: {
      return to_vector(this->check_free(call, call->argument(0), inv));
    }
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

SoundnessChecker::CheckResult SoundnessChecker::check_unknown_extern_call(
    ar::CallBase* call, ar::Function* fun, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg = this->display_soundness_check(Result::Unreachable, call)) {
      *msg << "\n";
    }
    return {CheckKind::Unreachable, Result::Unreachable, {}, {}};
  }

  if (auto msg = this->display_soundness_check(Result::Warning, call)) {
    *msg << ": call to an unknown extern function\n";
  }
  return {CheckKind::IgnoredCallSideEffect,
          Result::Warning,
          {},
          JsonDict{{"fun_id", _ctx.output_db->functions.insert(fun)}}};
}

boost::optional< SoundnessChecker::CheckResult > SoundnessChecker::
    check_mem_write(ar::Statement* stmt,
                    ar::Value* pointer,
                    CheckKind access_kind,
                    const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg = this->display_soundness_check(Result::Unreachable, stmt)) {
      *msg << "\n";
    }
    return {{CheckKind::Unreachable, Result::Unreachable, {}, {}}};
  }

  const ScalarLit& ptr = this->_lit_factory.get_scalar(pointer);

  // Check uninitialized
  if (ptr.is_undefined() || (ptr.is_pointer_var() &&
                             inv.normal().uninit_is_uninitialized(ptr.var()))) {
    // Undefined pointer operand
    if (auto msg = this->display_soundness_check(Result::Error, stmt)) {
      *msg << ": undefined pointer operand\n";
    }
    return {{CheckKind::UninitializedVariable, Result::Error, {pointer}, {}}};
  }

  // Check null pointer dereference
  if (ptr.is_null() ||
      (ptr.is_pointer_var() && inv.normal().nullity_is_null(ptr.var()))) {
    // Null pointer operand
    if (auto msg = this->display_soundness_check(Result::Error, stmt)) {
      *msg << ": null pointer dereference\n";
    }
    return {{CheckKind::NullPointerDereference, Result::Error, {pointer}, {}}};
  }

  // Check unexpected operand
  if (!ptr.is_pointer_var()) {
    log::error("unexpected pointer operand");
    return {{CheckKind::UnexpectedOperand, Result::Error, {pointer}, {}}};
  }

  if (isa< ar::GlobalVariable >(pointer) ||
      isa< ar::FunctionPointerConstant >(pointer)) {
    // Points-to set is a singleton
    return boost::none;
  }

  // Points-to set of the pointer
  PointsToSet addrs = inv.normal().pointer_to_points_to(ptr.var());

  if (addrs.is_empty()) {
    // Pointer is invalid
    if (auto msg = this->display_soundness_check(Result::Error, stmt)) {
      *msg << ": empty points-to set for pointer\n";
    }
    return {
        {CheckKind::InvalidPointerDereference, Result::Error, {pointer}, {}}};
  }

  if (addrs.is_top()) {
    // Ignored memory access because points-to set is top
    if (auto msg = this->display_soundness_check(Result::Warning, stmt)) {
      *msg << ": ignored memory write because points-to set is top\n";
    }
    return {{access_kind, Result::Warning, {pointer}, {}}};
  }

  return boost::none;
}

std::vector< SoundnessChecker::CheckResult > SoundnessChecker::
    check_call_pointer_params(ar::CallBase* call,
                              ar::Function* fun,
                              const std::vector< ar::Value* >& pointers,
                              const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg = this->display_soundness_check(Result::Unreachable, call)) {
      *msg << "\n";
    }
    return {{CheckKind::Unreachable, Result::Unreachable, {}, {}}};
  }

  std::vector< CheckResult > checks;

  // Check for unknown pointer arguments
  for (ar::Value* pointer : pointers) {
    ikos_assert(pointer->type()->is_pointer());

    const ScalarLit& ptr = this->_lit_factory.get_scalar(pointer);

    // Check uninitialized argument
    if (ptr.is_undefined() ||
        (ptr.is_pointer_var() &&
         inv.normal().uninit_is_uninitialized(ptr.var()))) {
      // Undefined pointer argument
      if (auto msg = this->display_soundness_check(Result::Error, call)) {
        *msg << ": undefined pointer argument\n";
      }
      return {{CheckKind::UninitializedVariable, Result::Error, {pointer}, {}}};
    }

    // Check null pointer argument
    if (ptr.is_null() ||
        (ptr.is_pointer_var() && inv.normal().nullity_is_null(ptr.var()))) {
      // This is sound
      continue;
    }

    // Check unexpected argument
    if (!ptr.is_pointer_var()) {
      log::error("unexpected pointer argument");
      return {{CheckKind::UnexpectedOperand, Result::Error, {pointer}, {}}};
    }

    if (isa< ar::GlobalVariable >(pointer) ||
        isa< ar::FunctionPointerConstant >(pointer)) {
      // Points-to set is a singleton
      // This is sound
      continue;
    }

    // Points-to set of the pointer
    PointsToSet addrs = inv.normal().pointer_to_points_to(ptr.var());

    if (addrs.is_empty()) {
      // Pointer is invalid
      if (auto msg = this->display_soundness_check(Result::Error, call)) {
        *msg << ": empty points-to set for pointer\n";
      }
      return {
          {CheckKind::InvalidPointerDereference, Result::Error, {pointer}, {}}};
    }

    if (addrs.is_top()) {
      // Ignored memory access because points-to set is top
      if (auto msg = this->display_soundness_check(Result::Warning, call)) {
        *msg << ": ignored call side effect on pointer ";
        pointer->dump(msg->stream());
        *msg << " because points-to set is top\n";
      }
      checks.push_back(
          {CheckKind::IgnoredCallSideEffectOnPointerParameter,
           Result::Warning,
           {pointer},
           JsonDict{{"fun_id", _ctx.output_db->functions.insert(fun)}}});
    }
  }

  return checks;
}

boost::optional< SoundnessChecker::CheckResult > SoundnessChecker::check_free(
    ar::CallBase* call, ar::Value* pointer, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg = this->display_soundness_check(Result::Unreachable, call)) {
      *msg << "\n";
    }
    return {{CheckKind::Unreachable, Result::Unreachable, {}, {}}};
  }

  const ScalarLit& ptr = this->_lit_factory.get_scalar(pointer);

  // Check uninitialized
  if (ptr.is_undefined() || (ptr.is_pointer_var() &&
                             inv.normal().uninit_is_uninitialized(ptr.var()))) {
    // Undefined pointer operand
    if (auto msg = this->display_soundness_check(Result::Error, call)) {
      *msg << ": undefined pointer operand\n";
    }
    return {{CheckKind::UninitializedVariable, Result::Error, {pointer}, {}}};
  }

  // Check null pointer dereference
  if (ptr.is_null() ||
      (ptr.is_pointer_var() && inv.normal().nullity_is_null(ptr.var()))) {
    // Null pointer argument, safe
    if (auto msg = this->display_soundness_check(Result::Ok, call)) {
      *msg << ": safe call to free with NULL value\n";
    }
    return {{CheckKind::Free, Result::Ok, {pointer}, {}}};
  }

  // Check unexpected operand
  if (!ptr.is_pointer_var()) {
    log::error("unexpected pointer operand");
    return {{CheckKind::UnexpectedOperand, Result::Error, {pointer}, {}}};
  }

  if (isa< ar::GlobalVariable >(pointer) ||
      isa< ar::FunctionPointerConstant >(pointer)) {
    // Points-to set is a singleton
    return boost::none;
  }

  // Points-to set of the pointer
  PointsToSet addrs = inv.normal().pointer_to_points_to(ptr.var());

  if (addrs.is_top()) {
    // Ignored memory deallocation because points-to set is top
    if (auto msg = this->display_soundness_check(Result::Warning, call)) {
      *msg << ": ignored memory deallocation because points-to set is top\n";
    }
    return {{CheckKind::IgnoredFree, Result::Warning, {pointer}, {}}};
  }

  return boost::none;
}

llvm::Optional< LogMessage > SoundnessChecker::display_soundness_check(
    Result result, ar::Statement* stmt) const {
  auto msg = this->display_check(result, stmt);
  if (msg) {
    *msg << "check_soundness(";
    stmt->dump(msg->stream());
    *msg << ")";
  }
  return msg;
}

} // end namespace analyzer
} // end namespace ikos
