/*******************************************************************************
 *
 * \file
 * \brief Double free checker implementation
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

#include <ikos/ar/verify/type.hpp>

#include <ikos/analyzer/checker/double_free.hpp>
#include <ikos/analyzer/support/cast.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {

DoubleFreeChecker::DoubleFreeChecker(Context& ctx) : Checker(ctx) {}

CheckerName DoubleFreeChecker::name() const {
  return CheckerName::DoubleFree;
}

const char* DoubleFreeChecker::description() const {
  return "Double free checker";
}

void DoubleFreeChecker::check(ar::Statement* stmt,
                              const value::AbstractDomain& inv,
                              CallContext* call_context) {
  if (auto call = dyn_cast< ar::CallBase >(stmt)) {
    std::vector< CheckResult > checks = this->check_call(call, inv);
    for (const auto& check : checks) {
      this->display_invariant(check.result, stmt, inv);
      this->_checks.insert(check.kind,
                           CheckerName::DoubleFree,
                           check.result,
                           stmt,
                           call_context,
                           check.operands,
                           check.info);
    }
  }
}

std::vector< DoubleFreeChecker::CheckResult > DoubleFreeChecker::check_call(
    ar::CallBase* call, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg = this->display_double_free_check(Result::Unreachable, call)) {
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
    if (auto msg = this->display_double_free_check(Result::Error, call)) {
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
    if (auto msg = this->display_double_free_check(Result::Error, call)) {
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
    if (auto msg = this->display_double_free_check(Result::Ok, call)) {
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
    if (auto msg = this->display_double_free_check(Result::Error, call)) {
      *msg << ": points-to set of function pointer is empty\n";
    }
    return {{CheckKind::InvalidPointerDereference,
             Result::Error,
             {call->called()},
             {}}};
  } else if (callees.is_top()) {
    // No points-to set
    if (auto msg = this->display_double_free_check(Result::Warning, call)) {
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
      boost::optional< CheckResult > check =
          this->check_intrinsic_call(call, callee, inv);
      if (check) {
        checks.push_back(*check);
      }
    }
  }

  return checks;
}

boost::optional< DoubleFreeChecker::CheckResult > DoubleFreeChecker::
    check_intrinsic_call(ar::CallBase* call,
                         ar::Function* fun,
                         const value::AbstractDomain& inv) {
  switch (fun->intrinsic_id()) {
    case ar::Intrinsic::LibcRealloc:
    case ar::Intrinsic::LibcFree:
    case ar::Intrinsic::LibcFclose:
    case ar::Intrinsic::LibcppDelete:
    case ar::Intrinsic::LibcppDeleteArray:
    case ar::Intrinsic::LibcppFreeException: {
      return this->check_double_free(call, call->argument(0), inv);
    }
    default: {
      return boost::none;
    }
  }
}

DoubleFreeChecker::CheckResult DoubleFreeChecker::check_double_free(
    ar::CallBase* call, ar::Value* pointer, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg = this->display_double_free_check(Result::Unreachable, call)) {
      *msg << "\n";
    }
    return {CheckKind::Unreachable, Result::Unreachable, {}, {}};
  }

  const ScalarLit& ptr = this->_lit_factory.get_scalar(pointer);

  if (ptr.is_undefined() || (ptr.is_pointer_var() &&
                             inv.normal().uninit_is_uninitialized(ptr.var()))) {
    if (auto msg = this->display_double_free_check(Result::Error, call)) {
      *msg << ": undefined operand\n";
    }
    return {CheckKind::UninitializedVariable, Result::Error, {pointer}, {}};
  }

  if (ptr.is_null() ||
      (ptr.is_pointer_var() && inv.normal().nullity_is_null(ptr.var()))) {
    if (auto msg = this->display_double_free_check(Result::Ok, call)) {
      *msg << ": safe call to free with NULL value\n";
    }
    return {CheckKind::Free, Result::Ok, {pointer}, {}};
  }

  PointsToSet addrs = inv.normal().pointer_to_points_to(ptr.var());

  if (addrs.is_empty()) {
    if (auto msg = this->display_double_free_check(Result::Error, call)) {
      *msg << ": empty points-to set for pointer\n";
    }
    return {CheckKind::InvalidPointerDereference, Result::Error, {pointer}, {}};
  } else if (addrs.is_top()) {
    if (auto msg = this->display_double_free_check(Result::Warning, call)) {
      *msg << ": no points-to information for pointer\n";
    }
    return {CheckKind::IgnoredFree, Result::Warning, {pointer}, {}};
  }

  bool all_error = true;
  bool all_ok = true;

  JsonDict info;
  JsonList points_to_info;

  for (const auto& addr : addrs) {
    JsonDict block_info = {
        {"id", _ctx.output_db->memory_locations.insert(addr)}};
    Result result = this->check_memory_location_free(call, inv, addr);
    block_info.put("status", static_cast< int >(result));

    if (result == Result::Ok) {
      all_error = false;
    } else if (result == Result::Warning) {
      all_error = false;
      all_ok = false;
    } else {
      all_ok = false;
    }
    points_to_info.add(block_info);
  }

  info.put("points_to", points_to_info);

  if (all_error) {
    // Unsafe
    return {CheckKind::Free, Result::Error, {pointer}, info};
  } else if (all_ok) {
    // Safe
    return {CheckKind::Free, Result::Ok, {pointer}, {}};
  } else {
    // Warning
    return {CheckKind::Free, Result::Warning, {pointer}, info};
  }
}

Result DoubleFreeChecker::check_memory_location_free(
    ar::CallBase* call,
    const value::AbstractDomain& inv,
    MemoryLocation* addr) {
  if (isa< DynAllocMemoryLocation >(addr)) {
    auto lifetime = inv.normal().lifetime_to_lifetime(addr);

    if (lifetime.is_deallocated()) {
      // This is a double free
      if (auto msg =
              this->display_double_free_check(Result::Error, call, addr)) {
        *msg << ": double free\n";
      }
      return Result::Error;
    } else if (lifetime.is_top()) {
      // A double free could be possible
      if (auto msg =
              this->display_double_free_check(Result::Warning, call, addr)) {
        *msg << ": possible double free\n";
      }
      return Result::Warning;
    } else {
      // Safe
      if (auto msg = this->display_double_free_check(Result::Ok, call, addr)) {
        *msg << ": safe call to free()\n";
      }
      return Result::Ok;
    }
  } else {
    // This is a free() call on something which isn't a dynamic allocated
    // memory.
    // This is an error
    if (auto msg = this->display_double_free_check(Result::Error, call, addr)) {
      *msg << ": free() called on a non-dynamic allocated memory\n";
    }
    return Result::Error;
  }
}

llvm::Optional< LogMessage > DoubleFreeChecker::display_double_free_check(
    Result result, ar::Statement* stmt) const {
  auto msg = this->display_check(result, stmt);
  if (msg) {
    *msg << "check_dfa(";
    stmt->dump(msg->stream());
    *msg << ")";
  }
  return msg;
}

llvm::Optional< LogMessage > DoubleFreeChecker::display_double_free_check(
    Result result, ar::CallBase* call, MemoryLocation* addr) const {
  auto msg = this->display_check(result, call);
  if (msg) {
    *msg << "check_dfa(";
    call->dump(msg->stream());
    *msg << ", addr=";
    addr->dump(msg->stream());
    *msg << ")";
  }
  return msg;
}

} // end namespace analyzer
} // end namespace ikos
