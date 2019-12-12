/*******************************************************************************
 *
 * \file
 * \brief Memory watch checker implementation
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2019 United States Government as represented by the
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

#include <ikos/analyzer/checker/memory_watch.hpp>
#include <ikos/analyzer/support/cast.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {

MemoryWatchChecker::MemoryWatchChecker(Context& ctx)
    : Checker(ctx),
      _ar_context(ctx.bundle->context()),
      _data_layout(ctx.bundle->data_layout()),
      _size_type(ar::IntegerType::size_type(ctx.bundle)) {
  ar::Type* void_ty = ar::VoidType::get(this->_ar_context);
  ar::Type* void_ptr_ty = ar::PointerType::get(this->_ar_context, void_ty);
  this->_watch_mem_ptr =
      ctx.var_factory->get_named_shadow(void_ptr_ty, "shadow.watch_mem.ptr");
  this->_watch_mem_size =
      ctx.var_factory->get_named_shadow(this->_size_type,
                                        "shadow.watch_mem.size");
}

CheckerName MemoryWatchChecker::name() const {
  return CheckerName::MemoryWatch;
}

const char* MemoryWatchChecker::description() const {
  return "Memory watch checker";
}

void MemoryWatchChecker::check(ar::Statement* stmt,
                               const value::AbstractDomain& inv,
                               CallContext* call_context) {
  if (auto store = dyn_cast< ar::Store >(stmt)) {
    this->check_store(store, inv);
  } else if (auto call = dyn_cast< ar::CallBase >(stmt)) {
    this->check_call(call, inv, call_context);
  }
}

void MemoryWatchChecker::check_store(ar::Store* store,
                                     const value::AbstractDomain& inv) {
  this->check_mem_write(store,
                        store->pointer(),
                        this->store_size(store->value()->type()),
                        inv);
}

void MemoryWatchChecker::check_call(ar::CallBase* call,
                                    const value::AbstractDomain& inv,
                                    CallContext* call_context) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    return;
  }

  const ScalarLit& called = this->_lit_factory.get_scalar(call->called());

  if (called.is_undefined() ||
      (called.is_pointer_var() &&
       inv.normal().uninit_is_uninitialized(called.var()))) {
    // Undefined call pointer operand
    return;
  }

  if (called.is_null() ||
      (called.is_pointer_var() && inv.normal().nullity_is_null(called.var()))) {
    // Null call pointer operand
    return;
  }

  // Collect potential callees
  auto callees = PointsToSet::bottom();

  if (auto cst = dyn_cast< ar::FunctionPointerConstant >(call->called())) {
    callees = {_ctx.mem_factory->get_function(cst->function())};
  } else if (isa< ar::InlineAssemblyConstant >(call->called())) {
    // Call to assembly
    this->check_unknown_extern_call(call, inv);
    return;
  } else if (isa< ar::GlobalVariable >(call->called())) {
    // Call to global variable: error
    return;
  } else if (isa< ar::LocalVariable >(call->called())) {
    // Call to local variable: error
    return;
  } else if (isa< ar::InternalVariable >(call->called())) {
    // Indirect call through a function pointer
    callees = inv.normal().pointer_to_points_to(called.var());
  } else {
    log::error("unexpected call pointer operand");
    return;
  }

  // Check callees
  ikos_assert(!callees.is_bottom());

  if (callees.is_empty()) {
    // Invalid pointer dereference
    return;
  } else if (callees.is_top()) {
    // No points-to information
    this->check_unknown_extern_call(call, inv);
    return;
  }

  ar::Function* caller = call->code()->function_or_null();

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
      this->check_recursive_call(call, callee, inv);
    } else if (callee->is_intrinsic()) {
      this->check_intrinsic_call(call, callee, inv);
    } else if (callee->is_declaration()) {
      this->check_unknown_extern_call(call, inv);
    }
  }
}

void MemoryWatchChecker::check_recursive_call(
    ar::CallBase* call,
    ar::Function* /*fun*/,
    const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    return;
  }

  // Watched addresses
  PointsToSet watch_addrs =
      inv.normal().pointer_to_points_to(this->_watch_mem_ptr);

  if (watch_addrs.is_empty() || watch_addrs.is_top()) {
    // Not watching anything, __ikos_watch_mem was not called
    return;
  }

  LogMessage msg = log::msg();
  this->display_stmt_location(msg, call);
  msg << "potential memory write at a watched memory location\n";
}

void MemoryWatchChecker::check_intrinsic_call(
    ar::CallBase* call, ar::Function* fun, const value::AbstractDomain& inv) {
  switch (fun->intrinsic_id()) {
    case ar::Intrinsic::MemoryCopy:
    case ar::Intrinsic::MemoryMove:
    case ar::Intrinsic::MemorySet: {
      this->check_mem_write(call, call->argument(0), call->argument(2), inv);
    } break;
    case ar::Intrinsic::VarArgStart:
    case ar::Intrinsic::VarArgEnd:
    case ar::Intrinsic::VarArgGet:
    case ar::Intrinsic::VarArgCopy: {
      this->check_unknown_call(call,
                               inv,
                               /* may_write_params = */ true,
                               /* ignore_unknown_write = */ true,
                               /* may_write_globals = */ false,
                               /* may_throw_exc = */ false);
    } break;
    case ar::Intrinsic::StackSave:
    case ar::Intrinsic::StackRestore:
    case ar::Intrinsic::LifetimeStart:
    case ar::Intrinsic::LifetimeEnd:
    case ar::Intrinsic::EhTypeidFor:
    case ar::Intrinsic::Trap: {
    } break;
    // <ikos/analyzer/intrinsic.h>
    case ar::Intrinsic::IkosAssert:
    case ar::Intrinsic::IkosAssume:
    case ar::Intrinsic::IkosNonDet:
    case ar::Intrinsic::IkosCounterInit:
    case ar::Intrinsic::IkosCounterIncr:
    case ar::Intrinsic::IkosCheckMemAccess:
    case ar::Intrinsic::IkosCheckStringAccess:
    case ar::Intrinsic::IkosAssumeMemSize: {
    } break;
    case ar::Intrinsic::IkosForgetMemory:
    case ar::Intrinsic::IkosAbstractMemory: {
      this->check_mem_write(call, call->argument(0), call->argument(1), inv);
    } break;
    case ar::Intrinsic::IkosWatchMemory:
    case ar::Intrinsic::IkosPartitioningVar:
    case ar::Intrinsic::IkosPartitioningJoin:
    case ar::Intrinsic::IkosPartitioningDisable:
    case ar::Intrinsic::IkosPrintInvariant:
    case ar::Intrinsic::IkosPrintValues: {
    } break;
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
    } break;
    // <errno.h>
    case ar::Intrinsic::LibcErrnoLocation: {
    } break;
    // <fcntl.h>
    case ar::Intrinsic::LibcOpen: {
    } break;
    // <unistd.h>
    case ar::Intrinsic::LibcClose: {
    } break;
    case ar::Intrinsic::LibcRead: {
      this->check_mem_write(call, call->argument(1), call->argument(2), inv);
    } break;
    case ar::Intrinsic::LibcWrite: {
    } break;
    // <stdio.h>
    case ar::Intrinsic::LibcGets: {
      this->check_mem_write(call, call->argument(0), inv);
    } break;
    case ar::Intrinsic::LibcFgets: {
      this->check_mem_write(call, call->argument(0), call->argument(1), inv);
    } break;
    case ar::Intrinsic::LibcGetc:
    case ar::Intrinsic::LibcFgetc:
    case ar::Intrinsic::LibcGetchar:
    case ar::Intrinsic::LibcPuts:
    case ar::Intrinsic::LibcFputs:
    case ar::Intrinsic::LibcPutc:
    case ar::Intrinsic::LibcFputc:
    case ar::Intrinsic::LibcPrintf:
    case ar::Intrinsic::LibcFprintf: {
    } break;
    case ar::Intrinsic::LibcSprintf: {
      this->check_mem_write(call, call->argument(0), inv);
    } break;
    case ar::Intrinsic::LibcSnprintf: {
      this->check_mem_write(call, call->argument(0), call->argument(1), inv);
    } break;
    case ar::Intrinsic::LibcScanf: {
      this->check_unknown_call(call,
                               inv,
                               /* may_write_params = */ true,
                               /* ignore_unknown_write = */ true,
                               /* may_write_globals = */ false,
                               /* may_throw_exc = */ false);
    } break;
    case ar::Intrinsic::LibcFscanf: {
      this->check_unknown_call(call,
                               inv,
                               /* may_write_params = */ true,
                               /* ignore_unknown_write = */ true,
                               /* may_write_globals = */ false,
                               /* may_throw_exc = */ false);
    } break;
    case ar::Intrinsic::LibcSscanf: {
      this->check_unknown_call(call,
                               inv,
                               /* may_write_params = */ true,
                               /* ignore_unknown_write = */ true,
                               /* may_write_globals = */ false,
                               /* may_throw_exc = */ false);
    } break;
    case ar::Intrinsic::LibcFopen:
    case ar::Intrinsic::LibcFclose:
    case ar::Intrinsic::LibcFflush: {
    } break;
    // <string.h>
    case ar::Intrinsic::LibcStrlen:
    case ar::Intrinsic::LibcStrnlen: {
    } break;
    case ar::Intrinsic::LibcStrcpy: {
      this->check_mem_write(call, call->argument(0), inv);
    } break;
    case ar::Intrinsic::LibcStrncpy: {
      this->check_mem_write(call, call->argument(0), call->argument(2), inv);
    } break;
    case ar::Intrinsic::LibcStrcat: {
      this->check_mem_write(call, call->argument(0), inv);
    } break;
    case ar::Intrinsic::LibcStrncat: {
      this->check_mem_write(call, call->argument(0), inv);
    } break;
    case ar::Intrinsic::LibcStrcmp:
    case ar::Intrinsic::LibcStrncmp:
    case ar::Intrinsic::LibcStrstr:
    case ar::Intrinsic::LibcStrchr:
    case ar::Intrinsic::LibcStrdup:
    case ar::Intrinsic::LibcStrndup: {
    } break;
    case ar::Intrinsic::LibcStrcpyCheck: {
      this->check_mem_write(call, call->argument(0), inv);
    } break;
    case ar::Intrinsic::LibcMemoryCopyCheck:
    case ar::Intrinsic::LibcMemoryMoveCheck:
    case ar::Intrinsic::LibcMemorySetCheck: {
      this->check_mem_write(call, call->argument(0), call->argument(2), inv);
    } break;
    case ar::Intrinsic::LibcStrcatCheck: {
      this->check_mem_write(call, call->argument(0), inv);
    } break;
    case ar::Intrinsic::LibcppNew:
    case ar::Intrinsic::LibcppNewArray:
    case ar::Intrinsic::LibcppDelete:
    case ar::Intrinsic::LibcppDeleteArray:
    case ar::Intrinsic::LibcppAllocateException:
    case ar::Intrinsic::LibcppFreeException:
    case ar::Intrinsic::LibcppThrow:
    case ar::Intrinsic::LibcppBeginCatch:
    case ar::Intrinsic::LibcppEndCatch: {
    } break;
    default: {
      ikos_unreachable("unreachable");
    }
  }
}

void MemoryWatchChecker::check_unknown_extern_call(
    ar::CallBase* call, const value::AbstractDomain& inv) {
  this->check_unknown_call(call,
                           inv,
                           /* may_write_params = */ true,
                           /* ignore_unknown_write = */ true,
                           /* may_write_globals = */ false,
                           /* may_throw_exc = */ true);
}

void MemoryWatchChecker::check_unknown_call(ar::CallBase* call,
                                            value::AbstractDomain inv,
                                            bool may_write_params,
                                            bool ignore_unknown_write,
                                            bool may_write_globals,
                                            bool /*may_throw_exc*/) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    return;
  }

  // Watched addresses
  PointsToSet watch_addrs =
      inv.normal().pointer_to_points_to(this->_watch_mem_ptr);

  if (watch_addrs.is_empty() || watch_addrs.is_top()) {
    // Not watching anything, __ikos_watch_mem was not called
    return;
  }

  if (may_write_globals) {
    // May write on global variables
    LogMessage msg = log::msg();
    this->display_stmt_location(msg, call);
    msg << "potential memory write at a watched memory location\n";
    return;
  }

  if (!may_write_params) {
    // Won't write on pointer parameters
    return;
  }

  for (auto it = call->arg_begin(), et = call->arg_end(); it != et; ++it) {
    ar::Value* arg = *it;

    if (!isa< ar::InternalVariable >(arg) ||
        !isa< ar::PointerType >(arg->type())) {
      continue;
    }

    auto iv = cast< ar::InternalVariable >(arg);
    Variable* ptr = _ctx.var_factory->get_internal(iv);

    this->init_global_ptr(inv, arg);

    if (inv.normal().nullity_is_null(ptr)) {
      // Null pointer parameter
      continue;
    }

    // Points-to set of the pointer
    PointsToSet addrs = inv.normal().pointer_to_points_to(ptr);

    if (addrs.is_empty()) {
      // Pointer is invalid
      continue;
    } else if (addrs.is_top()) {
      if (ignore_unknown_write) {
        // Ignored side effect on the memory
        continue;
      } else {
        LogMessage msg = log::msg();
        this->display_stmt_location(msg, call);
        msg << "potential memory write at a watched memory location\n";
        return;
      }
    } else if (addrs.meet(watch_addrs).is_empty()) {
      // Not writing on a watched address
      continue;
    } else {
      LogMessage msg = log::msg();
      this->display_stmt_location(msg, call);
      msg << "potential memory write at a watched memory location\n";
      return;
    }
  }
}

void MemoryWatchChecker::check_mem_write(ar::Statement* stmt,
                                         ar::Value* pointer,
                                         ar::Value* access_size,
                                         value::AbstractDomain inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    return;
  }

  // Watched addresses
  PointsToSet watch_addrs =
      inv.normal().pointer_to_points_to(this->_watch_mem_ptr);

  if (watch_addrs.is_empty() || watch_addrs.is_top()) {
    // Not watching anything, __ikos_watch_mem was not called
    return;
  }

  const ScalarLit& ptr = this->_lit_factory.get_scalar(pointer);
  const ScalarLit& size = this->_lit_factory.get_scalar(access_size);

  if (ptr.is_undefined() || (ptr.is_pointer_var() &&
                             inv.normal().uninit_is_uninitialized(ptr.var()))) {
    // Undefined pointer operand
    return;
  }

  if (size.is_undefined() ||
      (size.is_machine_int_var() &&
       inv.normal().uninit_is_uninitialized(size.var()))) {
    // Undefined pointer operand
    return;
  }

  if (ptr.is_null() ||
      (ptr.is_pointer_var() && inv.normal().nullity_is_null(ptr.var()))) {
    // Null pointer operand
    return;
  }

  // Check unexpected operand
  if (!ptr.is_pointer_var()) {
    log::error("unexpected pointer operand");
    return;
  }
  if (!size.is_machine_int() && !size.is_machine_int_var()) {
    log::error("unexpected size operand");
    return;
  }

  // Initialize global variable pointer and function pointer
  this->init_global_ptr(inv, pointer);

  // Points-to set of the pointer
  PointsToSet addrs = inv.normal().pointer_to_points_to(ptr.var());

  if (addrs.is_empty()) {
    // Pointer is invalid
    return;
  } else if (addrs.is_top()) {
    // Ignored memory write
    return;
  } else if (addrs.meet(watch_addrs).is_empty()) {
    // Not writing on a watched address
    return;
  }

  // Variable representing the pointer offset
  Variable* offset = ptr.var()->offset_var();
  inv.normal().pointer_offset_to_int(offset, ptr.var());

  // Add a shadow variable `offset_plus_size = offset + access_size`
  Variable* offset_plus_size =
      _ctx.var_factory->get_named_shadow(this->_size_type,
                                         "shadow.offset_plus_size");

  if (access_size->type() == this->_size_type) {
    if (size.is_machine_int_var()) {
      inv.normal().int_apply(IntBinaryOperator::Add,
                             offset_plus_size,
                             offset,
                             size.var());
    } else if (size.is_machine_int()) {
      inv.normal().int_apply(IntBinaryOperator::Add,
                             offset_plus_size,
                             offset,
                             size.machine_int());
    } else {
      ikos_unreachable("unexpected access size");
    }
  } else {
    // This happens in LibcFgets for instance
    if (size.is_machine_int_var()) {
      inv.normal().int_apply(IntUnaryOperator::Cast,
                             offset_plus_size,
                             size.var());
      inv.normal().int_apply(IntBinaryOperator::Add,
                             offset_plus_size,
                             offset_plus_size,
                             offset);
    } else if (size.is_machine_int()) {
      inv.normal()
          .int_apply(IntBinaryOperator::Add,
                     offset_plus_size,
                     offset,
                     size.machine_int().cast(this->_size_type->bit_width(),
                                             ar::Unsigned));
    } else {
      ikos_unreachable("unexpected access size");
    }
  }

  // Variable representing the watched pointer offset
  Variable* watch_offset = this->_watch_mem_ptr->offset_var();
  inv.normal().pointer_offset_to_int(watch_offset, this->_watch_mem_ptr);

  // Add a shadow variable `watch_offset_plus_size = watch_offset + watch_size`
  Variable* watch_offset_plus_size =
      _ctx.var_factory->get_named_shadow(this->_size_type,
                                         "shadow.watch_offset_plus_size");
  inv.normal().int_apply(IntBinaryOperator::Add,
                         watch_offset_plus_size,
                         watch_offset,
                         this->_watch_mem_size);

  // Add a shadow variable that represents an offset in bytes
  Variable* x = _ctx.var_factory->get_named_shadow(this->_size_type,
                                                   "shadow.watch_mem.x");

  // Check if it might overlap
  // Check if there exists an integer x such that:
  // x is in [offset, offset + size - 1]
  // x is in [watch_offset, watch_offset + watch_size - 1]
  value::AbstractDomain tmp1 = inv;
  tmp1.normal().int_add(IntPredicate::GE, x, offset);
  tmp1.normal().int_add(IntPredicate::LT, x, offset_plus_size);
  tmp1.normal().int_add(IntPredicate::GE, x, watch_offset);
  tmp1.normal().int_add(IntPredicate::LT, x, watch_offset_plus_size);
  tmp1.normal().normalize();

  if (tmp1.is_normal_flow_bottom()) {
    // Overlap not possible
    return;
  }

  // Check if it always overlaps

  // Check if all offset + size - 1 >= watch_offset
  value::AbstractDomain tmp2 = inv;
  tmp2.normal().int_add(IntPredicate::LE, offset_plus_size, watch_offset);
  tmp2.normal().normalize();

  // Check if all offset < watch_offset + watch_size
  value::AbstractDomain tmp3 = inv;
  tmp3.normal().int_add(IntPredicate::GE, offset, watch_offset_plus_size);
  tmp3.normal().normalize();

  LogMessage msg = log::msg();
  this->display_stmt_location(msg, stmt);

  if (tmp2.is_normal_flow_bottom() && tmp3.is_normal_flow_bottom()) {
    // Always overlaps
    msg << "memory write at a watched memory location\n";
  } else {
    // Might overlap
    msg << "potential memory write at a watched memory location\n";
  }
}

void MemoryWatchChecker::check_mem_write(ar::Statement* stmt,
                                         ar::Value* pointer,
                                         value::AbstractDomain inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    return;
  }

  // Watched addresses
  PointsToSet watch_addrs =
      inv.normal().pointer_to_points_to(this->_watch_mem_ptr);

  if (watch_addrs.is_empty() || watch_addrs.is_top()) {
    // Not watching anything, __ikos_watch_mem was not called
    return;
  }

  const ScalarLit& ptr = this->_lit_factory.get_scalar(pointer);

  if (ptr.is_undefined() || (ptr.is_pointer_var() &&
                             inv.normal().uninit_is_uninitialized(ptr.var()))) {
    // Undefined pointer operand
    return;
  }

  if (ptr.is_null() ||
      (ptr.is_pointer_var() && inv.normal().nullity_is_null(ptr.var()))) {
    // Null pointer operand
    return;
  }

  // Check unexpected operand
  if (!ptr.is_pointer_var()) {
    log::error("unexpected pointer operand");
    return;
  }

  // Initialize global variable pointer and function pointer
  this->init_global_ptr(inv, pointer);

  // Points-to set of the pointer
  PointsToSet addrs = inv.normal().pointer_to_points_to(ptr.var());

  if (addrs.is_empty()) {
    // Pointer is invalid
    return;
  } else if (addrs.is_top()) {
    // Ignored memory write
    return;
  } else if (addrs.meet(watch_addrs).is_empty()) {
    // Not writing on a watched address
    return;
  } else {
    LogMessage msg = log::msg();
    this->display_stmt_location(msg, stmt);
    msg << "potential memory write at a watched memory location\n";
  }
}

void MemoryWatchChecker::init_global_ptr(value::AbstractDomain& inv,
                                         ar::Value* value) const {
  if (auto gv = dyn_cast< ar::GlobalVariable >(value)) {
    Variable* ptr = _ctx.var_factory->get_global(gv);
    MemoryLocation* addr = _ctx.mem_factory->get_global(gv);
    inv.normal().pointer_assign(ptr, addr, core::Nullity::non_null());
  } else if (auto cst = dyn_cast< ar::FunctionPointerConstant >(value)) {
    auto fun = cst->function();
    Variable* ptr = _ctx.var_factory->get_function_ptr(fun);
    MemoryLocation* addr = _ctx.mem_factory->get_function(fun);
    inv.normal().pointer_assign(ptr, addr, core::Nullity::non_null());
  }
}

ar::IntegerConstant* MemoryWatchChecker::store_size(ar::Type* type) const {
  return ar::IntegerConstant::get(this->_ar_context,
                                  this->_size_type,
                                  MachineInt(this->_data_layout
                                                 .store_size_in_bytes(type),
                                             this->_size_type->bit_width(),
                                             this->_size_type->sign()));
}

} // end namespace analyzer
} // end namespace ikos
