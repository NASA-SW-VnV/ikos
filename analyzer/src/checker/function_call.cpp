/*******************************************************************************
 *
 * \file
 * \brief Function call checker implementation
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

#include <ikos/analyzer/checker/function_call.hpp>
#include <ikos/analyzer/support/cast.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {

FunctionCallChecker::FunctionCallChecker(Context& ctx) : Checker(ctx) {}

CheckerName FunctionCallChecker::name() const {
  return CheckerName::FunctionCall;
}

const char* FunctionCallChecker::description() const {
  return "Function call checker";
}

void FunctionCallChecker::check(ar::Statement* stmt,
                                const value::AbstractDomain& inv,
                                CallContext* call_context) {
  if (auto call = dyn_cast< ar::CallBase >(stmt)) {
    CheckResult check = this->check_call(call, inv);
    this->display_invariant(check.result, stmt, inv);
    this->_checks.insert(check.kind,
                         CheckerName::FunctionCall,
                         check.result,
                         stmt,
                         call_context,
                         check.operands,
                         check.info);
  }
}

FunctionCallChecker::CheckResult FunctionCallChecker::check_call(
    ar::CallBase* call, const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg = this->display_call_check(Result::Unreachable, call)) {
      *msg << "\n";
    }
    return {CheckKind::Unreachable, Result::Unreachable, {}, {}};
  }

  const ScalarLit& called = this->_lit_factory.get_scalar(call->called());

  // Check uninitialized

  if (called.is_undefined() ||
      (called.is_pointer_var() &&
       inv.normal().uninit_is_uninitialized(called.var()))) {
    // Undefined call pointer operand
    if (auto msg = this->display_call_check(Result::Error, call)) {
      *msg << ": undefined call pointer operand\n";
    }
    return {CheckKind::UninitializedVariable,
            Result::Error,
            {call->called()},
            {}};
  }

  // Check null pointer dereference

  if (called.is_null() ||
      (called.is_pointer_var() && inv.normal().nullity_is_null(called.var()))) {
    // Null call pointer operand
    if (auto msg = this->display_call_check(Result::Error, call)) {
      *msg << ": null call pointer operand\n";
    }
    return {CheckKind::NullPointerDereference,
            Result::Error,
            {call->called()},
            {}};
  }

  // Collect potential callees
  auto callees = PointsToSet::bottom();

  if (auto cst = dyn_cast< ar::FunctionPointerConstant >(call->called())) {
    callees = {_ctx.mem_factory->get_function(cst->function())};
  } else if (isa< ar::InlineAssemblyConstant >(call->called())) {
    // call to inline assembly
    if (auto msg = this->display_call_check(Result::Ok, call)) {
      *msg << ": call to inline assembly\n";
    }
    return {CheckKind::FunctionCallInlineAssembly, Result::Ok, {}, {}};
  } else if (auto gv = dyn_cast< ar::GlobalVariable >(call->called())) {
    callees = {_ctx.mem_factory->get_global(gv)};
  } else if (auto lv = dyn_cast< ar::LocalVariable >(call->called())) {
    callees = {_ctx.mem_factory->get_local(lv)};
  } else if (isa< ar::InternalVariable >(call->called())) {
    // Indirect call through a function pointer
    callees = inv.normal().pointer_to_points_to(called.var());
  } else {
    log::error("unexpected call pointer operand");
    return {CheckKind::UnexpectedOperand, Result::Error, {call->called()}, {}};
  }

  // Check callees
  ikos_assert(!callees.is_bottom());

  if (callees.is_empty()) {
    // Invalid pointer dereference
    if (auto msg = this->display_call_check(Result::Error, call)) {
      *msg << ": points-to set of function pointer is empty\n";
    }
    return {CheckKind::InvalidPointerDereference,
            Result::Error,
            {call->called()},
            {}};
  } else if (callees.is_top()) {
    // No points-to set
    if (auto msg = this->display_call_check(Result::Warning, call)) {
      *msg << ": no points-to set for function pointer\n";
    }
    return {CheckKind::UnknownFunctionCallPointer,
            Result::Warning,
            {call->called()},
            {}};
  }

  // Are all the callees valid/invalid
  bool all_valid = true;
  bool all_invalid = true;

  JsonList points_to_info;

  for (MemoryLocation* addr : callees) {
    JsonDict block_info = {
        {"id", _ctx.output_db->memory_locations.insert(addr)}};

    if (!isa< FunctionMemoryLocation >(addr)) {
      // Not a call to a function memory location, emit a warning
      if (auto msg = this->display_call_check(Result::Error, call)) {
        *msg << ": potential call to ";
        addr->dump(msg->stream());
        *msg << ", which is not a function\n";
      }

      block_info.put("kind",
                     static_cast< int >(FunctionCallCheckKind::NotFunction));
      all_valid = false;
    } else {
      ar::Function* callee = cast< FunctionMemoryLocation >(addr)->function();
      block_info.put("fun_id", _ctx.output_db->functions.insert(callee));

      if (!ar::TypeVerifier::is_valid_call(call, callee->type())) {
        // Ill-formed function call
        // This could be because of an imprecision of the pointer analysis.
        if (auto msg = this->display_call_check(Result::Error, call)) {
          *msg << ": potential call to " << callee->name()
               << ", wrong signature\n";
        }

        block_info.put("kind",
                       static_cast< int >(
                           FunctionCallCheckKind::WrongSignature));
        all_valid = false;
      } else {
        if (auto msg = this->display_call_check(Result::Ok, call)) {
          *msg << ": potential call to " << callee->name() << "\n";
        }

        block_info.put("kind", static_cast< int >(FunctionCallCheckKind::Ok));
        all_invalid = false;
      }
    }

    points_to_info.add(block_info);
  }

  JsonDict info;
  info.put("points_to", points_to_info);

  if (all_invalid) {
    return {CheckKind::FunctionCall, Result::Error, {call->called()}, info};
  } else if (!all_valid) {
    return {CheckKind::FunctionCall, Result::Warning, {call->called()}, info};
  } else {
    return {CheckKind::FunctionCall, Result::Ok, {call->called()}, info};
  }
}

llvm::Optional< LogMessage > FunctionCallChecker::display_call_check(
    Result result, ar::CallBase* call) const {
  auto msg = this->display_check(result, call);
  if (msg) {
    *msg << "check_call(";
    call->dump(msg->stream());
    *msg << ")";
  }
  return msg;
}

} // end namespace analyzer
} // end namespace ikos
