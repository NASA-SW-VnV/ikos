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
    this->check_mem_access(store,
                           store->pointer(),
                           CheckKind::IgnoredStore,
                           inv,
                           call_context);
  } else if (auto memcpy = dyn_cast< ar::MemoryCopy >(stmt)) {
    this->check_mem_access(memcpy,
                           memcpy->destination(),
                           CheckKind::IgnoredMemoryCopy,
                           inv,
                           call_context);
  } else if (auto memmove = dyn_cast< ar::MemoryMove >(stmt)) {
    this->check_mem_access(memmove,
                           memmove->destination(),
                           CheckKind::IgnoredMemoryMove,
                           inv,
                           call_context);
  } else if (auto memset = dyn_cast< ar::MemorySet >(stmt)) {
    this->check_mem_access(memset,
                           memset->pointer(),
                           CheckKind::IgnoredMemorySet,
                           inv,
                           call_context);
  } else {
    if (auto call = dyn_cast< ar::IntrinsicCall >(stmt)) {
      if (call->intrinsic_id() == ar::Intrinsic::LibcFree ||
          call->intrinsic_id() == ar::Intrinsic::LibcppDelete ||
          call->intrinsic_id() == ar::Intrinsic::LibcppDeleteArray ||
          call->intrinsic_id() == ar::Intrinsic::LibcppFreeException) {
        if (auto check = this->check_free(call, inv)) {
          this->display_invariant(check->result, stmt, inv);
          this->_checks.insert(check->kind,
                               CheckerName::Soundness,
                               check->result,
                               stmt,
                               call_context,
                               check->operands);
        }
      }
    }
    if (auto call = dyn_cast< ar::CallBase >(stmt)) {
      auto checks = this->check_call(call, inv);
      for (const auto& check : checks) {
        this->display_invariant(check.result, stmt, inv);
        this->_checks.insert(check.kind,
                             CheckerName::Soundness,
                             check.result,
                             stmt,
                             call_context,
                             check.operands);
      }
    }
  }
}

void SoundnessChecker::check_mem_access(ar::Statement* stmt,
                                        ar::Value* pointer,
                                        CheckKind access_kind,
                                        const value::AbstractDomain& inv,
                                        CallContext* call_context) {
  if (auto check = this->check_mem_access(stmt, pointer, access_kind, inv)) {
    this->display_invariant(check->result, stmt, inv);
    this->_checks.insert(check->kind,
                         CheckerName::Soundness,
                         check->result,
                         stmt,
                         call_context,
                         check->operands);
  }
}

boost::optional< SoundnessChecker::CheckResult > SoundnessChecker::
    check_mem_access(ar::Statement* stmt,
                     ar::Value* pointer,
                     CheckKind access_kind,
                     const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (this->display_soundness_check(Result::Unreachable, stmt)) {
      out() << std::endl;
    }
    return {{CheckKind::Unreachable, Result::Unreachable, {}}};
  }

  const ScalarLit& ptr = this->_lit_factory.get_scalar(pointer);

  // Check uninitialized
  if (ptr.is_undefined() ||
      (ptr.is_pointer_var() &&
       inv.normal().uninitialized().is_uninitialized(ptr.var()))) {
    // Undefined pointer operand
    if (this->display_soundness_check(Result::Error, stmt)) {
      out() << ": undefined pointer operand" << std::endl;
    }
    return {{CheckKind::UninitializedVariable, Result::Error, {pointer}}};
  }

  // Check null pointer dereference
  if (ptr.is_null() ||
      (ptr.is_pointer_var() && inv.normal().nullity().is_null(ptr.var()))) {
    // Null pointer operand
    if (this->display_soundness_check(Result::Error, stmt)) {
      out() << ": null pointer dereference" << std::endl;
    }
    return {{CheckKind::NullPointerDereference, Result::Error, {pointer}}};
  }

  // Check unexpected operand
  if (!ptr.is_pointer_var()) {
    log::error("unexpected pointer operand");
    return {{CheckKind::UnexpectedOperand, Result::Error, {pointer}}};
  }

  if (isa< ar::GlobalVariable >(pointer) ||
      isa< ar::FunctionPointerConstant >(pointer)) {
    // Points-to set is a singleton
    return boost::none;
  }

  // Points-to set of the pointer
  PointsToSet addrs = inv.normal().pointers().points_to(ptr.var());

  if (addrs.is_empty()) {
    // Pointer is invalid
    if (this->display_soundness_check(Result::Error, stmt)) {
      out() << ": empty points-to set for pointer" << std::endl;
    }
    return {{CheckKind::InvalidPointerDereference, Result::Error, {pointer}}};
  }

  if (addrs.is_top()) {
    // Ignored memory access because points-to set is top
    if (this->display_soundness_check(Result::Warning, stmt)) {
      out() << ": ignored memory access because points-to set is top"
            << std::endl;
    }
    return {{access_kind, Result::Warning, {pointer}}};
  }

  return boost::none;
}

boost::optional< SoundnessChecker::CheckResult > SoundnessChecker::check_free(
    ar::IntrinsicCall* call, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (this->display_soundness_check(Result::Unreachable, call)) {
      out() << std::endl;
    }
    return {{CheckKind::Unreachable, Result::Unreachable, {}}};
  }

  ikos_assert(call->num_arguments() == 1);

  auto pointer = call->argument(0);
  const ScalarLit& ptr = this->_lit_factory.get_scalar(pointer);

  // Check uninitialized
  if (ptr.is_undefined() ||
      (ptr.is_pointer_var() &&
       inv.normal().uninitialized().is_uninitialized(ptr.var()))) {
    // Undefined pointer operand
    if (this->display_soundness_check(Result::Error, call)) {
      out() << ": undefined pointer operand" << std::endl;
    }
    return {{CheckKind::UninitializedVariable, Result::Error, {pointer}}};
  }

  // Check null pointer dereference
  if (ptr.is_null() ||
      (ptr.is_pointer_var() && inv.normal().nullity().is_null(ptr.var()))) {
    // Null pointer argument, safe
    if (this->display_soundness_check(Result::Error, call)) {
      out() << ": safe call to free with NULL value" << std::endl;
    }
    return {{CheckKind::Free, Result::Ok, {pointer}}};
  }

  // Check unexpected operand
  if (!ptr.is_pointer_var()) {
    log::error("unexpected pointer operand");
    return {{CheckKind::UnexpectedOperand, Result::Error, {pointer}}};
  }

  if (isa< ar::GlobalVariable >(pointer) ||
      isa< ar::FunctionPointerConstant >(pointer)) {
    // Points-to set is a singleton
    return boost::none;
  }

  // Points-to set of the pointer
  PointsToSet addrs = inv.normal().pointers().points_to(ptr.var());

  if (addrs.is_top()) {
    // Ignored memory access because points-to set is top
    if (this->display_soundness_check(Result::Warning, call)) {
      out() << ": ignored memory deallocation because points-to set is top"
            << std::endl;
    }
    return {{CheckKind::IgnoredFree, Result::Warning, {pointer}}};
  }

  return boost::none;
}

std::vector< SoundnessChecker::CheckResult > SoundnessChecker::check_call(
    ar::CallBase* call, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (this->display_soundness_check(Result::Unreachable, call)) {
      out() << std::endl;
    }
    return {{CheckKind::Unreachable, Result::Unreachable, {}}};
  }

  const ScalarLit& called = this->_lit_factory.get_scalar(call->called());

  // Check uninitialized

  if (called.is_undefined() ||
      (called.is_pointer_var() &&
       inv.normal().uninitialized().is_uninitialized(called.var()))) {
    // Undefined call pointer operand
    if (this->display_soundness_check(Result::Error, call)) {
      out() << ": undefined call pointer operand" << std::endl;
    }
    return {{
        CheckKind::UninitializedVariable,
        Result::Error,
        {call->called()},
    }};
  }

  // Check null pointer dereference

  if (called.is_null() || (called.is_pointer_var() &&
                           inv.normal().nullity().is_null(called.var()))) {
    // Null call pointer operand
    if (this->display_soundness_check(Result::Error, call)) {
      out() << ": null call pointer operand" << std::endl;
    }
    return {
        {CheckKind::NullPointerDereference, Result::Error, {call->called()}}};
  }

  // Collect potential callees
  PointsToSet callees;

  if (auto cst = dyn_cast< ar::FunctionPointerConstant >(call->called())) {
    callees = {_ctx.mem_factory->get_function(cst->function())};
  } else if (isa< ar::InlineAssemblyConstant >(call->called())) {
    // call to inline assembly
    if (this->display_soundness_check(Result::Ok, call)) {
      out() << ": call to inline assembly" << std::endl;
    }
    return {{CheckKind::FunctionCallInlineAssembly, Result::Ok, {}}};
  } else if (auto gv = dyn_cast< ar::GlobalVariable >(call->called())) {
    callees = {_ctx.mem_factory->get_global(gv)};
  } else if (auto lv = dyn_cast< ar::LocalVariable >(call->called())) {
    callees = {_ctx.mem_factory->get_local(lv)};
  } else if (isa< ar::InternalVariable >(call->called())) {
    // Indirect call through a function pointer
    callees = inv.normal().pointers().points_to(called.var());
  } else {
    log::error("unexpected call pointer operand");
    return {{CheckKind::UnexpectedOperand, Result::Error, {call->called()}}};
  }

  // Check callees
  ikos_assert(!callees.is_bottom());
  if (callees.is_empty()) {
    // Invalid pointer dereference
    if (this->display_soundness_check(Result::Error, call)) {
      out() << ": points-to set of function pointer is empty" << std::endl;
    }
    return {{CheckKind::InvalidPointerDereference,
             Result::Error,
             {call->called()}}};
  } else if (callees.is_top()) {
    // No points-to set
    if (this->display_soundness_check(Result::Warning, call)) {
      out() << ": no points-to set for function pointer" << std::endl;
    }
    return {{CheckKind::UnknownFunctionCallPointer,
             Result::Warning,
             {call->called()}}};
  }

  // Is it calling an unknown extern function
  bool calls_unknown_extern_function =
      std::any_of(callees.begin(), callees.end(), [call](MemoryLocation* addr) {
        if (!isa< FunctionMemoryLocation >(addr)) {
          return false;
        }
        ar::Function* callee = cast< FunctionMemoryLocation >(addr)->function();
        return callee->is_declaration() &&
               ar::TypeVerifier::is_valid_call(call, callee->type()) &&
               !is_known_extern_function(callee);
      });

  if (!calls_unknown_extern_function) {
    return {}; // sound function call
  }

  std::vector< CheckResult > checks;

  for (auto it = call->arg_begin(), et = call->arg_end(); it != et; ++it) {
    ar::Value* arg = *it;
    if (isa< ar::InternalVariable >(arg) && arg->type()->is_pointer()) {
      const ScalarLit& ptr = this->_lit_factory.get_scalar(arg);
      ikos_assert(ptr.is_pointer_var());

      if (!inv.normal().uninitialized().is_uninitialized(ptr.var()) &&
          !inv.normal().nullity().is_null(ptr.var()) &&
          inv.normal().pointers().points_to(ptr.var()).is_top()) {
        // Ignored side effect on the memory because points-to set is top
        if (this->display_soundness_check(Result::Warning, call)) {
          out() << ": ignored call side effect on pointer ";
          arg->dump(out());
          out() << " because points-to set is top" << std::endl;
        }
        checks.push_back(
            {CheckKind::IgnoredCallSideEffect, Result::Warning, {arg}});
      }
    }
  }

  return checks;
}

bool SoundnessChecker::is_known_extern_function(ar::Function* fun) {
  ikos_assert(fun->is_declaration());

  if (!fun->is_intrinsic()) {
    return false;
  }

  // This needs to be up to date with
  // NumericalExecutionEngine::exec_intrinsic_call()
  switch (fun->intrinsic_id()) {
    case ar::Intrinsic::MemoryCopy:
    case ar::Intrinsic::MemoryMove:
    case ar::Intrinsic::MemorySet:
    case ar::Intrinsic::LibcMalloc:
    case ar::Intrinsic::LibcCalloc:
    case ar::Intrinsic::LibcppNew:
    case ar::Intrinsic::LibcppNewArray:
    case ar::Intrinsic::LibcppAllocateException:
    case ar::Intrinsic::LibcFree:
    case ar::Intrinsic::LibcppDelete:
    case ar::Intrinsic::LibcppDeleteArray:
    case ar::Intrinsic::LibcppFreeException:
    case ar::Intrinsic::LibcRead:
    case ar::Intrinsic::LibcppThrow:
    case ar::Intrinsic::LibcppBeginCatch:
    case ar::Intrinsic::LibcStrlen:
    case ar::Intrinsic::LibcStrnlen:
    case ar::Intrinsic::LibcStrcpy:
    case ar::Intrinsic::LibcStrncpy:
    case ar::Intrinsic::LibcStrcat:
    case ar::Intrinsic::LibcStrncat:
    case ar::Intrinsic::IkosAssert:
    case ar::Intrinsic::IkosAssume:
    case ar::Intrinsic::IkosPrintInvariant:
    case ar::Intrinsic::IkosPrintValues:
    case ar::Intrinsic::IkosNonDetSi32:
    case ar::Intrinsic::IkosNonDetUi32:
    case ar::Intrinsic::IkosCounterInit:
    case ar::Intrinsic::IkosCounterIncr:
      return true;
    default:
      return false;
  }
}

bool SoundnessChecker::display_soundness_check(Result result,
                                               ar::Statement* stmt) const {
  if (this->display_check(result, stmt)) {
    out() << "check_soundness(";
    stmt->dump(out());
    out() << ")";
    return true;
  }
  return false;
}

} // end namespace analyzer
} // end namespace ikos
