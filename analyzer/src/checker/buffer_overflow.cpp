/*******************************************************************************
 *
 * \file
 * \brief Buffer overflow checker implementation
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

#include <ikos/analyzer/checker/buffer_overflow.hpp>
#include <ikos/analyzer/json/helper.hpp>
#include <ikos/analyzer/support/cast.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {

BufferOverflowChecker::BufferOverflowChecker(Context& ctx)
    : Checker(ctx),
      _ar_context(ctx.bundle->context()),
      _data_layout(ctx.bundle->data_layout()),
      _size_type(ar::IntegerType::size_type(ctx.bundle)),
      _size_zero(
          ar::IntegerConstant::get(this->_ar_context, this->_size_type, 0)),
      _size_one(
          ar::IntegerConstant::get(this->_ar_context, this->_size_type, 1)) {}

CheckerName BufferOverflowChecker::name() const {
  return CheckerName::BufferOverflow;
}

const char* BufferOverflowChecker::description() const {
  return "Buffer overflow checker";
}

void BufferOverflowChecker::check(ar::Statement* stmt,
                                  const value::AbstractDomain& inv,
                                  CallContext* call_context) {
  if (auto load = dyn_cast< ar::Load >(stmt)) {
    CheckResult check =
        this->check_mem_access(load,
                               load->operand(),
                               this->store_size(load->result()->type()),
                               /* if_null = */ Result::Error,
                               inv);
    this->display_invariant(check.result, stmt, inv);
    this->_checks.insert(check.kind,
                         CheckerName::BufferOverflow,
                         check.result,
                         stmt,
                         call_context,
                         check.operands,
                         check.info);
  } else if (auto store = dyn_cast< ar::Store >(stmt)) {
    CheckResult check =
        this->check_mem_access(store,
                               store->pointer(),
                               this->store_size(store->value()->type()),
                               /* if_null = */ Result::Error,
                               inv);
    this->display_invariant(check.result, stmt, inv);
    this->_checks.insert(check.kind,
                         CheckerName::BufferOverflow,
                         check.result,
                         stmt,
                         call_context,
                         check.operands,
                         check.info);
  } else if (auto call = dyn_cast< ar::CallBase >(stmt)) {
    std::vector< CheckResult > checks = this->check_call(call, inv);
    for (const auto& check : checks) {
      this->display_invariant(check.result, stmt, inv);
      this->_checks.insert(check.kind,
                           CheckerName::BufferOverflow,
                           check.result,
                           stmt,
                           call_context,
                           check.operands,
                           check.info);
    }
  }
}

std::vector< BufferOverflowChecker::CheckResult > BufferOverflowChecker::
    check_call(ar::CallBase* call, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg = this->display_mem_access_check(Result::Unreachable, call)) {
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
    if (auto msg = this->display_mem_access_check(Result::Error, call)) {
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
    if (auto msg = this->display_mem_access_check(Result::Error, call)) {
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
    if (auto msg = this->display_mem_access_check(Result::Ok, call)) {
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
    if (auto msg = this->display_mem_access_check(Result::Error, call)) {
      *msg << ": points-to set of function pointer is empty\n";
    }
    return {{CheckKind::InvalidPointerDereference,
             Result::Error,
             {call->called()},
             {}}};
  } else if (callees.is_top()) {
    // No points-to set
    if (auto msg = this->display_mem_access_check(Result::Warning, call)) {
      *msg << ": no points-to set for function pointer\n";
    }
    return {{CheckKind::UnknownFunctionCallPointer,
             Result::Warning,
             {call->called()},
             {}}};
  }

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
    }

    if (callee->is_intrinsic()) {
      for (const auto& check : this->check_intrinsic_call(call, callee, inv)) {
        checks.push_back(check);
      }
    }
  }

  return checks;
}

std::vector< BufferOverflowChecker::CheckResult > BufferOverflowChecker::
    check_intrinsic_call(ar::CallBase* call,
                         ar::Function* fun,
                         const value::AbstractDomain& inv) {
  switch (fun->intrinsic_id()) {
    case ar::Intrinsic::MemoryCopy:
    case ar::Intrinsic::MemoryMove: {
      return {this->check_mem_access(call,
                                     call->argument(1),
                                     call->argument(2),
                                     /* if_null = */ Result::Error,
                                     inv),
              this->check_mem_access(call,
                                     call->argument(0),
                                     call->argument(2),
                                     /* if_null = */ Result::Error,
                                     inv)};
    }
    case ar::Intrinsic::MemorySet: {
      return {this->check_mem_access(call,
                                     call->argument(0),
                                     call->argument(2),
                                     /* if_null = */ Result::Error,
                                     inv)};
    }
    case ar::Intrinsic::VarArgStart:
    case ar::Intrinsic::VarArgEnd:
    case ar::Intrinsic::VarArgGet: {
      return {this->check_va_list_access(call, call->argument(0), inv)};
    }
    case ar::Intrinsic::VarArgCopy: {
      return {this->check_va_list_access(call, call->argument(0), inv),
              this->check_va_list_access(call, call->argument(1), inv)};
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
    case ar::Intrinsic::IkosCheckMemAccess: {
      return {this->check_mem_access(call,
                                     call->argument(0),
                                     call->argument(1),
                                     /* if_null = */ Result::Error,
                                     inv)};
    }
    case ar::Intrinsic::IkosCheckStringAccess: {
      return {this->check_string_access(call,
                                        call->argument(0),
                                        /* if_null = */ Result::Error,
                                        inv)};
    }
    case ar::Intrinsic::IkosAssumeMemSize: {
      return {};
    }
    case ar::Intrinsic::IkosForgetMemory:
    case ar::Intrinsic::IkosAbstractMemory: {
      return {this->check_mem_access(call,
                                     call->argument(0),
                                     call->argument(1),
                                     /* if_null = */ Result::Error,
                                     inv)};
    }
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
      return {this->check_realloc(call, call->argument(0), inv)};
    }
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
      return {this->check_string_access(call,
                                        call->argument(0),
                                        /* if_null = */ Result::Error,
                                        inv)};
    }
    // <unistd.h>
    case ar::Intrinsic::LibcClose: {
      return {};
    }
    case ar::Intrinsic::LibcRead: {
      return {this->check_mem_access(call,
                                     call->argument(1),
                                     call->argument(2),
                                     /* if_null = */ Result::Error,
                                     inv)};
    }
    case ar::Intrinsic::LibcWrite: {
      return {this->check_mem_access(call,
                                     call->argument(1),
                                     call->argument(2),
                                     /* if_null = */ Result::Error,
                                     inv)};
    }
    // <stdio.h>
    case ar::Intrinsic::LibcGets: {
      return {{CheckKind::BufferOverflowGets,
               Result::Error,
               {call->argument(0)},
               {}}};
    }
    case ar::Intrinsic::LibcFgets: {
      return {this->check_mem_access(call,
                                     call->argument(0),
                                     call->argument(1),
                                     /* if_null = */ Result::Error,
                                     inv),
              this->check_file_access(call,
                                      call->argument(2),
                                      /* if_null = */ Result::Error,
                                      inv)};
    }
    case ar::Intrinsic::LibcGetc:
    case ar::Intrinsic::LibcFgetc: {
      return {this->check_file_access(call,
                                      call->argument(0),
                                      /* if_null = */ Result::Error,
                                      inv)};
    }
    case ar::Intrinsic::LibcGetchar: {
      return {};
    }
    case ar::Intrinsic::LibcPuts: {
      return {this->check_string_access(call,
                                        call->argument(0),
                                        /* if_null = */ Result::Error,
                                        inv)};
    }
    case ar::Intrinsic::LibcFputs: {
      return {this->check_string_access(call,
                                        call->argument(0),
                                        /* if_null = */ Result::Error,
                                        inv),
              this->check_file_access(call,
                                      call->argument(1),
                                      /* if_null = */ Result::Error,
                                      inv)};
    }
    case ar::Intrinsic::LibcPutc:
    case ar::Intrinsic::LibcFputc: {
      return {this->check_file_access(call,
                                      call->argument(1),
                                      /* if_null = */ Result::Error,
                                      inv)};
    }
    case ar::Intrinsic::LibcPrintf: {
      std::vector< CheckResult > checks = {
          this->check_string_access(call,
                                    call->argument(0),
                                    /* if_null = */ Result::Error,
                                    inv)};
      for (auto it = call->arg_begin() + 1, et = call->arg_end(); it != et;
           ++it) {
        ar::Value* arg = *it;
        if (arg->type()->is_pointer()) {
          checks.push_back(this->check_string_access(call,
                                                     arg,
                                                     /* if_null = */ Result::Ok,
                                                     inv));
        }
      }
      return checks;
    }
    case ar::Intrinsic::LibcFprintf: {
      std::vector< CheckResult > checks =
          {this->check_file_access(call,
                                   call->argument(0),
                                   /* if_null = */ Result::Error,
                                   inv),
           this->check_string_access(call,
                                     call->argument(1),
                                     /* if_null = */ Result::Error,
                                     inv)};
      for (auto it = call->arg_begin() + 2, et = call->arg_end(); it != et;
           ++it) {
        ar::Value* arg = *it;
        if (arg->type()->is_pointer()) {
          checks.push_back(this->check_string_access(call,
                                                     arg,
                                                     /* if_null = */ Result::Ok,
                                                     inv));
        }
      }
      return checks;
    }
    case ar::Intrinsic::LibcSprintf: {
      std::vector< CheckResult > checks =
          {this->check_string_access(call,
                                     call->argument(0),
                                     /* if_null = */ Result::Error,
                                     inv),
           this->check_string_access(call,
                                     call->argument(1),
                                     /* if_null = */ Result::Error,
                                     inv)};
      for (auto it = call->arg_begin() + 2, et = call->arg_end(); it != et;
           ++it) {
        ar::Value* arg = *it;
        if (arg->type()->is_pointer()) {
          checks.push_back(this->check_string_access(call,
                                                     arg,
                                                     /* if_null = */ Result::Ok,
                                                     inv));
        }
      }
      return checks;
    }
    case ar::Intrinsic::LibcSnprintf: {
      std::vector< CheckResult > checks = {};

      // Calling snprintf with zero bufsz and null pointer buffer can be used
      // to determine the buffer size needed to contain the output. That case
      // is allowed. Otherwise, check first argument.
      auto bufsz = this->_lit_factory.get_scalar(call->argument(1));
      if (!(bufsz.is_machine_int() && bufsz.machine_int().is_zero())) {
        checks.push_back(this->check_mem_access(call,
                                                call->argument(0),
                                                call->argument(1),
                                                /* if_null = */ Result::Error,
                                                inv));
      }

      checks.push_back(this->check_string_access(call,
                                                 call->argument(2),
                                                 /* if_null = */ Result::Error,
                                                 inv));

      for (auto it = call->arg_begin() + 3, et = call->arg_end(); it != et;
           ++it) {
        ar::Value* arg = *it;
        if (arg->type()->is_pointer()) {
          checks.push_back(this->check_string_access(call,
                                                     arg,
                                                     /* if_null = */ Result::Ok,
                                                     inv));
        }
      }
      return checks;
    }
    case ar::Intrinsic::LibcScanf: {
      std::vector< CheckResult > checks = {
          this->check_string_access(call,
                                    call->argument(0),
                                    /* if_null = */ Result::Error,
                                    inv)};
      for (auto it = call->arg_begin() + 1, et = call->arg_end(); it != et;
           ++it) {
        ar::Value* arg = *it;
        ikos_assert(arg->type()->is_pointer());
        checks.push_back(
            this->check_string_access(call,
                                      arg,
                                      /* if_null = */ Result::Error,
                                      inv));
      }
      return checks;
    }
    case ar::Intrinsic::LibcFscanf: {
      std::vector< CheckResult > checks = {
          this->check_file_access(call,
                                  call->argument(0),
                                  /* if_null = */ Result::Error,
                                  inv),
          this->check_string_access(call,
                                    call->argument(1),
                                    /* if_null = */ Result::Error,
                                    inv),
      };
      for (auto it = call->arg_begin() + 2, et = call->arg_end(); it != et;
           ++it) {
        ar::Value* arg = *it;
        ikos_assert(arg->type()->is_pointer());
        checks.push_back(
            this->check_string_access(call,
                                      arg,
                                      /* if_null = */ Result::Error,
                                      inv));
      }
      return checks;
    }
    case ar::Intrinsic::LibcSscanf: {
      std::vector< CheckResult > checks = {
          this->check_string_access(call,
                                    call->argument(0),
                                    /* if_null = */ Result::Error,
                                    inv),
          this->check_string_access(call,
                                    call->argument(1),
                                    /* if_null = */ Result::Error,
                                    inv),
      };
      for (auto it = call->arg_begin() + 2, et = call->arg_end(); it != et;
           ++it) {
        ar::Value* arg = *it;
        ikos_assert(arg->type()->is_pointer());
        checks.push_back(
            this->check_string_access(call,
                                      arg,
                                      /* if_null = */ Result::Error,
                                      inv));
      }
      return checks;
    }
    case ar::Intrinsic::LibcFopen: {
      return {this->check_string_access(call,
                                        call->argument(0),
                                        /* if_null = */ Result::Error,
                                        inv),
              this->check_string_access(call,
                                        call->argument(1),
                                        /* if_null = */ Result::Error,
                                        inv)};
    }
    case ar::Intrinsic::LibcFclose:
    case ar::Intrinsic::LibcFflush: {
      return {this->check_file_access(call,
                                      call->argument(0),
                                      /* if_null = */ Result::Error,
                                      inv)};
    }
    // <string.h>
    case ar::Intrinsic::LibcStrlen: {
      return {this->check_string_access(call,
                                        call->argument(0),
                                        /* if_null = */ Result::Error,
                                        inv)};
    }
    case ar::Intrinsic::LibcStrnlen: {
      return {this->check_mem_access(call,
                                     call->argument(0),
                                     call->argument(1),
                                     /* if_null = */ Result::Error,
                                     inv)};
    }
    case ar::Intrinsic::LibcStrcpy: {
      return {this->check_string_access(call,
                                        call->argument(0),
                                        /* if_null = */ Result::Error,
                                        inv),
              this->check_string_access(call,
                                        call->argument(1),
                                        /* if_null = */ Result::Error,
                                        inv)};
    }
    case ar::Intrinsic::LibcStrncpy: {
      return {this->check_string_access(call,
                                        call->argument(0),
                                        /* if_null = */ Result::Error,
                                        inv),
              this->check_string_access(call,
                                        call->argument(1),
                                        /* max_access_size = */
                                        call->argument(2),
                                        /* if_null = */ Result::Error,
                                        inv)};
    }
    case ar::Intrinsic::LibcStrcat: {
      return {this->check_string_access(call,
                                        call->argument(0),
                                        /* if_null = */ Result::Error,
                                        inv),
              this->check_string_access(call,
                                        call->argument(1),
                                        /* if_null = */ Result::Error,
                                        inv)};
    }
    case ar::Intrinsic::LibcStrncat: {
      return {this->check_string_access(call,
                                        call->argument(0),
                                        /* if_null = */ Result::Error,
                                        inv),
              this->check_string_access(call,
                                        call->argument(1),
                                        /* max_access_size = */
                                        call->argument(2),
                                        /* if_null = */ Result::Error,
                                        inv)};
    }
    case ar::Intrinsic::LibcStrcmp: {
      return {this->check_string_access(call,
                                        call->argument(0),
                                        /* if_null = */ Result::Error,
                                        inv),
              this->check_string_access(call,
                                        call->argument(1),
                                        /* if_null = */ Result::Error,
                                        inv)};
    }
    case ar::Intrinsic::LibcStrncmp: {
      return {this->check_string_access(call,
                                        call->argument(0),
                                        /* max_access_size = */
                                        call->argument(2),
                                        /* if_null = */ Result::Error,
                                        inv),
              this->check_string_access(call,
                                        call->argument(1),
                                        /* max_access_size = */
                                        call->argument(2),
                                        /* if_null = */ Result::Error,
                                        inv)};
    }
    case ar::Intrinsic::LibcStrstr: {
      return {this->check_string_access(call,
                                        call->argument(0),
                                        /* if_null = */ Result::Error,
                                        inv),
              this->check_string_access(call,
                                        call->argument(1),
                                        /* if_null = */ Result::Error,
                                        inv)};
    }
    case ar::Intrinsic::LibcStrchr: {
      return {this->check_string_access(call,
                                        call->argument(0),
                                        /* if_null = */ Result::Error,
                                        inv)};
    }
    case ar::Intrinsic::LibcStrdup: {
      return {this->check_string_access(call,
                                        call->argument(0),
                                        /* if_null = */ Result::Error,
                                        inv)};
    }
    case ar::Intrinsic::LibcStrndup: {
      return {this->check_mem_access(call,
                                     call->argument(0),
                                     call->argument(1),
                                     /* if_null = */ Result::Error,
                                     inv)};
    }
    case ar::Intrinsic::LibcStrcpyCheck: {
      return {this->check_mem_access(call,
                                     call->argument(0),
                                     call->argument(2),
                                     /* if_null = */ Result::Error,
                                     inv),
              this->check_mem_access(call,
                                     call->argument(1),
                                     call->argument(2),
                                     /* if_null = */ Result::Error,
                                     inv)};
    }
    case ar::Intrinsic::LibcMemoryCopyCheck:
    case ar::Intrinsic::LibcMemoryMoveCheck: {
      return {this->check_mem_access(call,
                                     call->argument(1),
                                     call->argument(2),
                                     /* if_null = */ Result::Error,
                                     inv),
              this->check_mem_access(call,
                                     call->argument(0),
                                     call->argument(2),
                                     /* if_null = */ Result::Error,
                                     inv)};
    }
    case ar::Intrinsic::LibcMemorySetCheck: {
      return {this->check_mem_access(call,
                                     call->argument(0),
                                     call->argument(2),
                                     /* if_null = */ Result::Error,
                                     inv)};
    }
    case ar::Intrinsic::LibcStrcatCheck: {
      return {this->check_string_access(call,
                                        call->argument(0),
                                        /* if_null = */ Result::Error,
                                        inv),
              this->check_string_access(call,
                                        call->argument(1),
                                        /* if_null = */ Result::Error,
                                        inv)};
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

BufferOverflowChecker::CheckResult BufferOverflowChecker::check_mem_access(
    ar::Statement* stmt,
    ar::Value* pointer,
    ar::Value* access_size,
    Result if_null,
    value::AbstractDomain inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg = this->display_mem_access_check(Result::Unreachable,
                                                  stmt,
                                                  pointer,
                                                  access_size)) {
      *msg << "\n";
    }
    return {CheckKind::Unreachable, Result::Unreachable, {}, {}};
  }

  const ScalarLit& ptr = this->_lit_factory.get_scalar(pointer);
  const ScalarLit& size = this->_lit_factory.get_scalar(access_size);

  // Check uninitialized

  if (ptr.is_undefined() || (ptr.is_pointer_var() &&
                             inv.normal().uninit_is_uninitialized(ptr.var()))) {
    // Undefined pointer operand
    if (auto msg = this->display_mem_access_check(Result::Error,
                                                  stmt,
                                                  pointer,
                                                  access_size)) {
      *msg << ": undefined pointer operand\n";
    }
    return {CheckKind::UninitializedVariable, Result::Error, {pointer}, {}};
  }

  if (size.is_undefined() ||
      (size.is_machine_int_var() &&
       inv.normal().uninit_is_uninitialized(size.var()))) {
    // Undefined size operand
    if (auto msg = this->display_mem_access_check(Result::Error,
                                                  stmt,
                                                  pointer,
                                                  access_size)) {
      *msg << ": undefined size operand\n";
    }
    return {CheckKind::UninitializedVariable, Result::Error, {access_size}, {}};
  }

  // Check null pointer dereference

  if (ptr.is_null() ||
      (ptr.is_pointer_var() && inv.normal().nullity_is_null(ptr.var()))) {
    // Null pointer operand
    if (auto msg = this->display_mem_access_check(if_null,
                                                  stmt,
                                                  pointer,
                                                  access_size)) {
      *msg << ": null pointer dereference\n";
    }
    return {CheckKind::NullPointerDereference, if_null, {pointer}, {}};
  }

  // Check unexpected operand
  if (!ptr.is_pointer_var()) {
    log::error("unexpected pointer operand");
    return {CheckKind::UnexpectedOperand, Result::Error, {pointer}, {}};
  }
  if (!size.is_machine_int() && !size.is_machine_int_var()) {
    log::error("unexpected size operand");
    return {CheckKind::UnexpectedOperand, Result::Error, {access_size}, {}};
  }

  // Initialize global variable pointer and function pointer
  this->init_global_ptr(inv, pointer);

  // Points-to set of the pointer
  PointsToSet addrs = inv.normal().pointer_to_points_to(ptr.var());

  if (addrs.is_empty()) {
    // Pointer is invalid
    if (auto msg = this->display_mem_access_check(Result::Error,
                                                  stmt,
                                                  pointer,
                                                  access_size)) {
      *msg << ": empty points-to set for pointer\n";
    }
    return {CheckKind::InvalidPointerDereference, Result::Error, {pointer}, {}};
  } else if (addrs.is_top()) {
    // Unknown points-to set
    if (auto msg = this->display_mem_access_check(Result::Warning,
                                                  stmt,
                                                  pointer,
                                                  access_size)) {
      *msg << ": no points-to information for pointer\n";
    }
    return {CheckKind::UnknownMemoryAccess, Result::Warning, {pointer}, {}};
  }

  JsonDict info;
  JsonList points_to_info;

  IntInterval offset_intv = inv.normal().pointer_offset_to_interval(ptr.var());
  info.put("offset", to_json(offset_intv));

  auto size_intv = IntInterval::bottom(1, Signed);
  if (size.is_machine_int_var()) {
    size_intv = inv.normal().int_to_interval(size.var());
  } else if (size.is_machine_int()) {
    size_intv = IntInterval(size.machine_int());
  } else {
    ikos_unreachable("unexpected access size");
  }
  info.put("access_size", to_json(size_intv));

  // Variable representing the pointer offset
  Variable* offset_var = ptr.var()->offset_var();
  inv.normal().pointer_offset_to_int(offset_var, ptr.var());

  // Add a shadow variable `offset_plus_size = offset + access_size`
  Variable* offset_plus_size =
      _ctx.var_factory->get_named_shadow(this->_size_type,
                                         "shadow.offset_plus_size");

  if (access_size->type() == this->_size_type) {
    if (size.is_machine_int_var()) {
      inv.normal().int_apply(IntBinaryOperator::Add,
                             offset_plus_size,
                             offset_var,
                             size.var());
    } else if (size.is_machine_int()) {
      inv.normal().int_apply(IntBinaryOperator::Add,
                             offset_plus_size,
                             offset_var,
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
                             offset_var);
    } else if (size.is_machine_int()) {
      inv.normal()
          .int_apply(IntBinaryOperator::Add,
                     offset_plus_size,
                     offset_var,
                     size.machine_int().cast(this->_size_type->bit_width(),
                                             ar::Unsigned));
    } else {
      ikos_unreachable("unexpected access size");
    }
  }

  inv.normal().normalize();

  if (auto element_size =
          this->is_array_access(stmt, inv, offset_intv, addrs)) {
    info.put("array_element_size", *element_size);
  }

  // Are all the points-to in/valid
  bool all_valid = true;
  bool all_invalid = true;

  for (auto addr : addrs) {
    AllocSizeVariable* size_var = _ctx.var_factory->get_alloc_size(addr);
    this->init_global_alloc_size(inv, addr, size_var);

    // Add block info
    JsonDict block_info = {
        {"id", _ctx.output_db->memory_locations.insert(addr)}};

    // Perform analysis
    auto check = this->check_memory_location_access(stmt,
                                                    pointer,
                                                    access_size,
                                                    inv,
                                                    addr,
                                                    size_var,
                                                    offset_var,
                                                    offset_plus_size,
                                                    offset_intv,
                                                    block_info);

    block_info.put("status", static_cast< int >(check.result));
    block_info.put("kind", static_cast< int >(check.kind));

    if (check.result == Result::Error) {
      all_valid = false;
    } else if (check.result == Result::Warning) {
      all_valid = false;
      all_invalid = false;
    } else {
      all_invalid = false;
    }

    points_to_info.add(block_info);
  }

  info.put("points_to", points_to_info);

  if (all_invalid) {
    return {CheckKind::BufferOverflow,
            Result::Error,
            {pointer, access_size},
            info};
  } else if (!all_valid) {
    return {CheckKind::BufferOverflow,
            Result::Warning,
            {pointer, access_size},
            info};
  } else {
    return {CheckKind::BufferOverflow, Result::Ok, {pointer, access_size}, {}};
  }
}

BufferOverflowChecker::MemoryLocationCheckResult BufferOverflowChecker::
    check_memory_location_access(ar::Statement* stmt,
                                 ar::Value* pointer,
                                 ar::Value* access_size,
                                 const value::AbstractDomain& inv,
                                 MemoryLocation* addr,
                                 AllocSizeVariable* size_var,
                                 Variable* offset_var,
                                 Variable* offset_plus_size,
                                 const IntInterval& offset_intv,
                                 JsonDict& block_info) {
  if (isa< FunctionMemoryLocation >(addr)) {
    // Try to dereference a function pointer, this is an error
    if (auto msg = this->display_mem_access_check(Result::Error,
                                                  stmt,
                                                  pointer,
                                                  access_size,
                                                  addr)) {
      *msg << ": dereferencing a function pointer\n";
    }
    return {BufferOverflowCheckKind::Function, Result::Error};
  }

  if (isa< DynAllocMemoryLocation >(addr)) {
    // Dynamic allocated memory location
    // Check for use after free

    auto lifetime = inv.normal().lifetime_to_lifetime(addr);

    if (lifetime.is_deallocated()) {
      // Use after free
      if (auto msg = this->display_mem_access_check(Result::Error,
                                                    stmt,
                                                    pointer,
                                                    access_size,
                                                    addr)) {
        *msg << ": use after free\n";
      }
      return {BufferOverflowCheckKind::UseAfterFree, Result::Error};
    } else if (lifetime.is_top()) {
      // Possible use after free
      if (auto msg = this->display_mem_access_check(Result::Warning,
                                                    stmt,
                                                    pointer,
                                                    access_size,
                                                    addr)) {
        *msg << ": possible use after free\n";
      }
      return {BufferOverflowCheckKind::UseAfterFree, Result::Warning};
    } else {
      ikos_assert(lifetime.is_allocated());
    }
  }

  if (isa< LocalMemoryLocation >(addr)) {
    // Stack memory location
    // Check for dangling stack pointer

    auto lifetime = inv.normal().lifetime_to_lifetime(addr);

    if (lifetime.is_deallocated()) {
      // Access to a dangling stack pointer
      if (auto msg = this->display_mem_access_check(Result::Error,
                                                    stmt,
                                                    pointer,
                                                    access_size,
                                                    addr)) {
        *msg << ": access to a dangling stack pointer\n";
      }
      return {BufferOverflowCheckKind::UseAfterReturn, Result::Error};
    } else if (lifetime.is_top()) {
      // Possible access to a dangling stack pointer
      if (auto msg = this->display_mem_access_check(Result::Warning,
                                                    stmt,
                                                    pointer,
                                                    access_size,
                                                    addr)) {
        *msg << ": possible access to a dangling stack pointer\n";
      }
      return {BufferOverflowCheckKind::UseAfterReturn, Result::Warning};
    } else {
      ikos_assert(lifetime.is_allocated());
    }
  }

  if (isa< AbsoluteZeroMemoryLocation >(addr)) {
    // Checks: hardware addresses

    // Compute the writable interval for offset o ([o, o + access_size])
    auto offset_plus_size_intv = inv.normal().int_to_interval(offset_plus_size);
    auto one = IntInterval(MachineInt(1, offset_intv.bit_width(), Unsigned));
    auto last_byte_offset_intv = sub_no_wrap(offset_plus_size_intv, one);
    auto writable_interval = last_byte_offset_intv.join(offset_intv);

    if (_ctx.opts.hardware_addresses.geq(writable_interval)) {
      // The offset_var is completely included in an hardware address range
      // specified by the user, so we're Ok
      if (auto msg = this->display_mem_access_check(Result::Ok,
                                                    stmt,
                                                    pointer,
                                                    access_size,
                                                    addr)) {
        *msg << "[hardware addresses]: ∀o ∈ offset, o <= ";
        access_size->dump(msg->stream());
        *msg << " && o + access_size <= ";
        access_size->dump(msg->stream());
        *msg << "\n";
      }
      return {BufferOverflowCheckKind::HardwareAddresses, Result::Ok};
    } else if (_ctx.opts.hardware_addresses.is_meet_bottom(offset_intv) ||
               _ctx.opts.hardware_addresses.is_meet_bottom(
                   last_byte_offset_intv)) {
      // The offset_var isn't included in an hardware address range at all.
      // This is an error
      if (auto msg = this->display_mem_access_check(Result::Error,
                                                    stmt,
                                                    pointer,
                                                    access_size,
                                                    addr)) {
        *msg << "[hardware addresses]: ∀o ∈ offset, o > ";
        access_size->dump(msg->stream());
        *msg << " || o + access_size > ";
        access_size->dump(msg->stream());
        *msg << "\n";
      }
      return {BufferOverflowCheckKind::HardwareAddresses, Result::Error};
    } else {
      // The offset_var isn't completely included in an hardware address range
      // specified by the user, so it could overflow somewhere
      // This is a warning
      if (auto msg = this->display_mem_access_check(Result::Warning,
                                                    stmt,
                                                    pointer,
                                                    access_size,
                                                    addr)) {
        *msg << "[hardware addresses]: ∃o ∈ offset, o > ";
        access_size->dump(msg->stream());
        *msg << " || o + access_size > ";
        access_size->dump(msg->stream());
        *msg << "\n";
      }
      return {BufferOverflowCheckKind::HardwareAddresses, Result::Warning};
    }
  }

  // add `size` (min, max) to block_info
  IntInterval size_intv = inv.normal().int_to_interval(size_var);
  block_info.put("size", to_json(size_intv));

  // add `offset + access_size - size` (min, max) to block_info
  auto zero = MachineInt(0, this->_data_layout.pointers.bit_width, Unsigned);
  auto one = MachineInt(1, this->_data_layout.pointers.bit_width, Unsigned);
  auto expr = IntLinearExpression(zero);
  expr.add(one, offset_plus_size);
  expr.add(-one, size_var);
  IntInterval diff_intv = inv.normal().int_to_interval(expr);
  block_info.put("diff", to_json(diff_intv));

  // Checks: `offset > mem_size || offset + access_size > mem_size`
  value::AbstractDomain tmp1 = inv;
  tmp1.normal().int_add(IntPredicate::GT, offset_var, size_var);
  tmp1.normal().normalize();

  value::AbstractDomain tmp2 = inv;
  tmp2.normal().int_add(IntPredicate::GT, offset_plus_size, size_var);
  tmp2.normal().normalize();

  bool is_bottom = tmp1.is_normal_flow_bottom() && tmp2.is_normal_flow_bottom();

  if (is_bottom) {
    // offset_var <= size_var and offset_plus_size <= size_var, so we're
    // safe here
    if (auto msg = this->display_mem_access_check(Result::Ok,
                                                  stmt,
                                                  pointer,
                                                  access_size,
                                                  addr)) {
      *msg << ": ∀o ∈ offset, o <= ";
      access_size->dump(msg->stream());
      *msg << " && o + access_size <= ";
      access_size->dump(msg->stream());
      *msg << "\n";
    }
    return {BufferOverflowCheckKind::OutOfBound, Result::Ok};
  }

  // Check: `offset <= mem_size && offset + access_size <= mem_size`
  value::AbstractDomain tmp3 = inv;
  tmp3.normal().int_add(IntPredicate::LE, offset_var, size_var);
  tmp3.normal().int_add(IntPredicate::LE, offset_plus_size, size_var);
  tmp3.normal().normalize();
  is_bottom = tmp3.is_normal_flow_bottom();

  if (is_bottom) {
    if (auto msg = this->display_mem_access_check(Result::Error,
                                                  stmt,
                                                  pointer,
                                                  access_size,
                                                  addr)) {
      *msg << ": ∀o ∈ offset, o > ";
      access_size->dump(msg->stream());
      *msg << " || o + access_size > ";
      access_size->dump(msg->stream());
      *msg << "\n";
    }
    return {BufferOverflowCheckKind::OutOfBound, Result::Error};
  } else {
    if (auto msg = this->display_mem_access_check(Result::Warning,
                                                  stmt,
                                                  pointer,
                                                  access_size,
                                                  addr)) {
      *msg << ": ∃o ∈ offset, o <= ";
      access_size->dump(msg->stream());
      *msg << " && o + access_size <= ";
      access_size->dump(msg->stream());
      *msg << "\n";
    }
    return {BufferOverflowCheckKind::OutOfBound, Result::Warning};
  }
}

BufferOverflowChecker::CheckResult BufferOverflowChecker::check_string_access(
    ar::Statement* stmt,
    ar::Value* pointer,
    Result if_null,
    const value::AbstractDomain& inv) {
  /// Notes:
  ///
  /// Since we do not keep track of null-terminated string lengths, we cannot
  /// prove if a string access is safe. Thus, we use one for the access size,
  /// checking only if the first byte is accessible.
  ///
  /// ASSUMPTION: If the first byte of a string is accessible, the string is
  /// well-formed.
  ///
  /// TODO(marthaud): Improve checks for strings.
  return this->check_mem_access(stmt, pointer, this->_size_one, if_null, inv);
}

BufferOverflowChecker::CheckResult BufferOverflowChecker::check_string_access(
    ar::Statement* stmt,
    ar::Value* pointer,
    ar::Value* max_access_size,
    Result if_null,
    const value::AbstractDomain& inv) {
  /// Notes:
  ///
  /// Since we do not keep track of null-terminated string lengths, we cannot
  /// prove if a string access is safe. Thus, we use one for the access size,
  /// checking only if the first byte is accessible.
  ///
  /// ASSUMPTION: If the first byte of a string is accessible, the string is
  /// well-formed.
  ///
  /// TODO(marthaud): Improve checks for strings.
  ikos_ignore(max_access_size);
  return this->check_mem_access(stmt, pointer, this->_size_one, if_null, inv);
}

BufferOverflowChecker::CheckResult BufferOverflowChecker::check_va_list_access(
    ar::Statement* stmt, ar::Value* pointer, const value::AbstractDomain& inv) {
  /// Notes:
  ///
  /// Since we do not have the size of `va_list`, we cannot prove if a `va_list`
  /// is safe. Thus, we use zero for the access size.
  ///
  /// ASSUMPTION: calls to `va_start`, `va_end`, `va_arg` and `va_copy` are
  /// memory safe.
  return this->check_mem_access(stmt,
                                pointer,
                                this->_size_zero,
                                /* if_null = */ Result::Error,
                                inv);
}

BufferOverflowChecker::CheckResult BufferOverflowChecker::check_realloc(
    ar::CallBase* call, ar::Value* pointer, const value::AbstractDomain& inv) {
  /// Notes:
  ///
  /// Since `realloc` knows the size of the allocated memory block, we use zero
  /// for the access size.
  ///
  /// ASSUMPTION: calls to `realloc` are memory safe.
  ///
  /// TODO(marthaud): Add checks that the pointer offset is zero.
  /// TODO(marthaud): Add checks that the pointer was dynamically allocated.
  return this->check_mem_access(call,
                                pointer,
                                this->_size_zero,
                                /* if_null = */ Result::Ok,
                                inv);
}

BufferOverflowChecker::CheckResult BufferOverflowChecker::check_file_access(
    ar::Statement* stmt,
    ar::Value* pointer,
    Result if_null,
    const value::AbstractDomain& inv) {
  /// Notes:
  ///
  /// Since we do not have the size of `FILE*`, we cannot prove if a `FILE*`
  /// is safe. Thus, we use zero for the access size.
  ///
  /// ASSUMPTION: accesses of `FILE*` by libc functions are memory safe.
  return this->check_mem_access(stmt, pointer, this->_size_zero, if_null, inv);
}

ar::IntegerConstant* BufferOverflowChecker::store_size(ar::Type* type) const {
  return ar::IntegerConstant::get(this->_ar_context,
                                  this->_size_type,
                                  MachineInt(this->_data_layout
                                                 .store_size_in_bytes(type),
                                             this->_size_type->bit_width(),
                                             this->_size_type->sign()));
}

void BufferOverflowChecker::init_global_ptr(value::AbstractDomain& inv,
                                            ar::Value* value) const {
  if (auto gv = dyn_cast< ar::GlobalVariable >(value)) {
    Variable* ptr = _ctx.var_factory->get_global(gv);
    MemoryLocation* addr = _ctx.mem_factory->get_global(gv);
    inv.normal().pointer_assign(ptr, addr, core::Nullity::non_null());
    inv.normal().normalize();
  } else if (auto cst = dyn_cast< ar::FunctionPointerConstant >(value)) {
    auto fun = cst->function();
    Variable* ptr = _ctx.var_factory->get_function_ptr(fun);
    MemoryLocation* addr = _ctx.mem_factory->get_function(fun);
    inv.normal().pointer_assign(ptr, addr, core::Nullity::non_null());
    inv.normal().normalize();
  }
}

void BufferOverflowChecker::init_global_alloc_size(
    value::AbstractDomain& inv,
    MemoryLocation* addr,
    AllocSizeVariable* size_var) const {
  if (auto gv = dyn_cast< GlobalMemoryLocation >(addr)) {
    MachineInt size(this->_data_layout.store_size_in_bytes(
                        gv->global_var()->type()->pointee()),
                    this->_data_layout.pointers.bit_width,
                    Unsigned);
    inv.normal().int_assign(size_var, size);
    inv.normal().normalize();
  } else if (isa< FunctionMemoryLocation >(addr)) {
    MachineInt size(0, this->_data_layout.pointers.bit_width, Unsigned);
    inv.normal().int_assign(size_var, size);
    inv.normal().normalize();
  } else if (isa< LibcErrnoMemoryLocation >(addr)) {
    MachineInt size(4, this->_data_layout.pointers.bit_width, Unsigned);
    inv.normal().int_assign(size_var, size);
    inv.normal().normalize();
  }
}

/// \brief Check whether an interval is a multiple of a number
static bool is_multiple(const core::machine_int::Interval& interval,
                        const MachineInt& n) {
  return (mod(interval.lb(), n).is_zero() || interval.lb().is_min()) &&
         (mod(interval.ub(), n).is_zero() || interval.ub().is_max());
}

boost::optional< MachineInt > BufferOverflowChecker::is_array_access(
    ar::Statement* stmt,
    const value::AbstractDomain& inv,
    const IntInterval& offset_intv,
    const PointsToSet& addrs) const {
  // Use heuristics to determine if it is an array access
  ar::Type* access_type = nullptr;

  // Load or Store
  if (auto load = dyn_cast< ar::Load >(stmt)) {
    access_type = load->result()->type();
  } else if (auto store = dyn_cast< ar::Store >(stmt)) {
    access_type = store->value()->type();
  } else {
    return boost::none;
  }

  MachineInt element_size(this->_data_layout.store_size_in_bytes(access_type),
                          this->_size_type->bit_width(),
                          this->_size_type->sign());

  // Offset is a multiple of the element size
  if (!is_multiple(offset_intv, element_size)) {
    return boost::none;
  }

  if (!std::all_of(addrs.begin(), addrs.end(), [&](MemoryLocation* addr) {
        if (auto local = dyn_cast< LocalMemoryLocation >(addr)) {
          // Local variable with an array type
          auto type = local->local_var()->type()->pointee();
          return type->is_array() &&
                 cast< ar::ArrayType >(type)->element_type() == access_type;
        } else if (auto global = dyn_cast< GlobalMemoryLocation >(addr)) {
          // Global variable with an array type
          auto type = global->global_var()->type()->pointee();
          return type->is_array() &&
                 cast< ar::ArrayType >(type)->element_type() == access_type;
        } else if (auto dyn_alloc = dyn_cast< DynAllocMemoryLocation >(addr)) {
          AllocSizeVariable* size_var = _ctx.var_factory->get_alloc_size(addr);
          IntInterval size_intv = inv.normal().int_to_interval(size_var);

          // At least >= 2 elements
          if (size_intv.ub() <= element_size) {
            return false;
          }

          // Size is a multiple of the element size
          if (!is_multiple(size_intv, element_size)) {
            return false;
          }

          // Check if the next instruction is a bitcast to `access_type*`
          auto call = dyn_alloc->call();

          auto next_stmt = call->next_statement();
          if (next_stmt == nullptr) {
            return false;
          }

          auto unary = dyn_cast< ar::UnaryOperation >(next_stmt);
          return unary != nullptr && unary->operand() == call->result() &&
                 unary->op() == ar::UnaryOperation::Bitcast &&
                 unary->result()->type()->is_pointer() &&
                 cast< ar::PointerType >(unary->result()->type())->pointee() ==
                     access_type;
        }

        return false;
      })) {
    return boost::none;
  }

  return std::move(element_size);
}

llvm::Optional< LogMessage > BufferOverflowChecker::display_mem_access_check(
    Result result, ar::Statement* stmt) const {
  auto msg = this->display_check(result, stmt);
  if (msg) {
    *msg << "check_mem_access(";
    stmt->dump(msg->stream());
    *msg << ")";
  }
  return msg;
}

llvm::Optional< LogMessage > BufferOverflowChecker::display_mem_access_check(
    Result result,
    ar::Statement* stmt,
    ar::Value* pointer,
    ar::Value* access_size) const {
  auto msg = this->display_check(result, stmt);
  if (msg) {
    *msg << "check_mem_access(pointer=";
    pointer->dump(msg->stream());
    *msg << ", access_size=";
    access_size->dump(msg->stream());
    *msg << ")";
  }
  return msg;
}

llvm::Optional< LogMessage > BufferOverflowChecker::display_mem_access_check(
    Result result,
    ar::Statement* stmt,
    ar::Value* pointer,
    ar::Value* access_size,
    MemoryLocation* addr) const {
  auto msg = this->display_check(result, stmt);
  if (msg) {
    *msg << "check_mem_access(pointer=";
    pointer->dump(msg->stream());
    *msg << ", access_size=";
    access_size->dump(msg->stream());
    *msg << ", address=";
    addr->dump(msg->stream());
    *msg << ")";
  }
  return msg;
}

} // end namespace analyzer
} // end namespace ikos
