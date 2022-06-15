/*******************************************************************************
 *
 * \file
 * \brief Uninitialized variable checker implementation
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

#include <ikos/analyzer/checker/uninitialized_variable.hpp>
#include <ikos/analyzer/support/cast.hpp>

namespace ikos {
namespace analyzer {

UninitializedVariableChecker::UninitializedVariableChecker(Context& ctx)
    : Checker(ctx) {}

CheckerName UninitializedVariableChecker::name() const {
  return CheckerName::UninitializedVariable;
}

const char* UninitializedVariableChecker::description() const {
  return "Uninitialized variable checker";
}

void UninitializedVariableChecker::check(ar::Statement* stmt,
                                         const value::AbstractDomain& inv,
                                         CallContext* call_context) {
  if (inv.is_normal_flow_bottom()) {
    // Statement is unreachable
    // No checks
    return;
  }

  if (isa< ar::Assignment >(stmt)) {
    // Assignment propagate the undefinedness
    // No checks
    return;
  }

  // Exempt "And" and "Or" if one operand is constant.
  if (ar::BinaryOperation* inst = dyn_cast< ar::BinaryOperation >(stmt)) {
    auto op = inst->op();
    switch (op) {
      case ar::BinaryOperation::Operator::UAnd:
      case ar::BinaryOperation::Operator::UOr:
      case ar::BinaryOperation::Operator::SAnd:
      case ar::BinaryOperation::Operator::SOr: {
        auto left = inst->left();
        auto right = inst->right();
        if (left->is_integer_constant() || right->is_integer_constant()) {
          return;
        }
        break;
      }
      default: {
        break;
      }
    }
  }

  if (auto insert = dyn_cast< ar::InsertElement >(stmt)) {
    // InsertElement accepts undefined aggregate operands
    this->check_initialized(stmt, insert->offset(), inv, call_context);
    this->check_initialized(stmt, insert->element(), inv, call_context);
    return;
  }

  // Check each operand
  for (auto it = stmt->op_begin(), et = stmt->op_end(); it != et; ++it) {
    this->check_initialized(stmt, *it, inv, call_context);
  }
}

void UninitializedVariableChecker::check_initialized(
    ar::Statement* stmt,
    ar::Value* operand,
    const value::AbstractDomain& inv,
    CallContext* call_context) {
  if (auto result = this->check_initialized(operand, inv)) {
    this->display_initialized_check(*result, stmt, operand);
    this->display_invariant(*result, stmt, inv);
    this->_checks.insert(CheckKind::UninitializedVariable,
                         CheckerName::UninitializedVariable,
                         *result,
                         stmt,
                         call_context,
                         std::array< ar::Value*, 1 >{{operand}});
  }
}

boost::optional< Result > UninitializedVariableChecker::check_initialized(
    ar::Value* operand, const value::AbstractDomain& inv) {
  if (isa< ar::UndefinedConstant >(operand)) {
    return Result::Error;
  } else if (isa< ar::IntegerConstant >(operand)) {
    return boost::none;
  } else if (isa< ar::FloatConstant >(operand)) {
    return boost::none;
  } else if (isa< ar::NullConstant >(operand)) {
    return boost::none;
  } else if (isa< ar::StructConstant >(operand)) {
    return boost::none;
  } else if (isa< ar::ArrayConstant >(operand)) {
    return boost::none;
  } else if (isa< ar::VectorConstant >(operand)) {
    return boost::none;
  } else if (isa< ar::AggregateZeroConstant >(operand)) {
    return boost::none;
  } else if (isa< ar::FunctionPointerConstant >(operand)) {
    return Result::Ok;
  } else if (isa< ar::InlineAssemblyConstant >(operand)) {
    return Result::Ok;
  } else if (isa< ar::GlobalVariable >(operand)) {
    return Result::Ok;
  } else if (isa< ar::LocalVariable >(operand)) {
    return Result::Ok;
  } else if (auto iv = dyn_cast< ar::InternalVariable >(operand)) {
    Variable* var = _ctx.var_factory->get_internal(iv);
    core::Uninitialized uninit = inv.normal().uninit_to_uninitialized(var);

    if (uninit.is_uninitialized()) {
      return Result::Error;
    } else if (uninit.is_initialized()) {
      return Result::Ok;
    } else {
      return Result::Warning;
    }
  } else {
    ikos_unreachable("unreachable");
  }
}

void UninitializedVariableChecker::display_initialized_check(
    Result result, ar::Statement* stmt, ar::Value* operand) const {
  if (auto msg = this->display_check(result, stmt)) {
    *msg << "check_initialized(";
    operand->dump(msg->stream());
    *msg << ")\n";
  }
}

} // end namespace analyzer
} // end namespace ikos
