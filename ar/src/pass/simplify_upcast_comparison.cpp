/*******************************************************************************
 *
 * \file
 * \brief Implementation of SimplifyUpcastComparisonPass
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

#include <ikos/ar/pass/simplify_upcast_comparison.hpp>
#include <ikos/ar/semantic/statement.hpp>

namespace ikos {
namespace ar {

const char* SimplifyUpcastComparisonPass::name() const {
  return "simplify-upcast-comparison";
}

const char* SimplifyUpcastComparisonPass::description() const {
  return "Simplify upcast comparison";
}

bool SimplifyUpcastComparisonPass::run_on_code(Code* code) {
  bool change = false;

  for (BasicBlock* bb : *code) {
    if (!bb->empty()) {
      change = this->run_on_statement(bb->front()) || change;
    }
  }

  return change;
}

bool SimplifyUpcastComparisonPass::run_on_statement(Statement* stmt) {
  if (auto cmp = dyn_cast< Comparison >(stmt)) {
    if (cmp->is_integer_predicate()) {
      if (cmp->left()->is_integer_constant() &&
          cmp->right()->is_internal_variable()) {
        return this->run_on_comparison(cmp,
                                       cast< IntegerConstant >(cmp->left()),
                                       cmp->right());
      } else if (cmp->left()->is_internal_variable() &&
                 cmp->right()->is_integer_constant()) {
        return this->run_on_comparison(cmp,
                                       cast< IntegerConstant >(cmp->right()),
                                       cmp->left());
      }
    }
  }

  return false;
}

bool SimplifyUpcastComparisonPass::run_on_comparison(Comparison* cmp,
                                                     IntegerConstant* constant,
                                                     Value* var) {
  BasicBlock* bb = cmp->parent();
  auto it = bb->rend();
  auto et = bb->rend();

  // track variable if bitcast occurs
  Value* tracked_variable = var;
  bool bitcast = false;
  UnaryOperation* upcast = nullptr;

  // backtrace and find the upcast
  while (true) {
    // beginning of a block
    if (it == et) {
      if (bb->num_predecessors() == 1) {
        // move to the parent
        bb = *bb->predecessor_begin();
        it = bb->rbegin();
        et = bb->rend();
        continue;
      } else {
        return false;
      }
    }

    auto stmt = *it;

    if (stmt->result_or_null() == tracked_variable) {
      if (!isa< UnaryOperation >(stmt)) {
        return false;
      }
      upcast = cast< UnaryOperation >(stmt);
      if (!bitcast && upcast->op() == UnaryOperation::Bitcast) {
        tracked_variable = upcast->operand();
        upcast = nullptr;
        bitcast = true;
      } else if (!bitcast && (upcast->op() == UnaryOperation::SExt ||
                              upcast->op() == UnaryOperation::ZExt)) {
        break;
      } else if (bitcast && upcast->op() == UnaryOperation::ZExt) {
        break;
      } else {
        return false;
      }
    }

    it++;
  }

  if (auto new_constant =
          this->run_on_upcast(cmp->context(), upcast->operand(), constant)) {
    // add assert
    this->insert_assert(cmp,
                        upcast->operand(),
                        constant,
                        *new_constant,
                        bitcast);
    return true;
  } else {
    return false;
  }
}

boost::optional< IntegerConstant* > SimplifyUpcastComparisonPass::run_on_upcast(
    Context& context, Value* tracked_variable, IntegerConstant* constant) {
  auto type = cast< IntegerType >(tracked_variable->type());

  ZInterval top =
      IntInterval::top(type->bit_width(), type->sign()).to_z_interval();
  ZNumber zvalue = constant->value().to_z_number();

  // if the constant is superior to the maximum value of the original type
  // (before the cast), then this is an inifinite loop
  if (top.contains(zvalue)) {
    return IntegerConstant::get(context,
                                IntegerType::get(context,
                                                 type->bit_width(),
                                                 type->sign()),
                                constant->value().cast(type->bit_width(),
                                                       type->sign()));
  } else {
    return boost::none;
  }
}

void SimplifyUpcastComparisonPass::insert_assert(Comparison* cmp,
                                                 Value* tracked_variable,
                                                 IntegerConstant* constant,
                                                 IntegerConstant* new_constant,
                                                 bool bitcast) {
  Value* left;
  Value* right;
  if (cmp->left() == constant) {
    left = new_constant;
    right = tracked_variable;
  } else {
    left = tracked_variable;
    right = new_constant;
  }
  Comparison::Predicate predicate = cmp->predicate();
  if (bitcast) {
    switch (cmp->predicate()) {
      case Comparison::UIGT:
        predicate = Comparison::SIGT;
        break;
      case Comparison::UIGE:
        predicate = Comparison::SIGE;
        break;
      case Comparison::UILT:
        predicate = Comparison::SILT;
        break;
      case Comparison::UILE:
        predicate = Comparison::SILE;
        break;

      case Comparison::SIGT:
        predicate = Comparison::UIGT;
        break;
      case Comparison::SIGE:
        predicate = Comparison::UIGE;
        break;
      case Comparison::SILT:
        predicate = Comparison::UILT;
        break;
      case Comparison::SILE:
        predicate = Comparison::UILE;
        break;

      case Comparison::UIEQ:
        predicate = Comparison::SIEQ;
        break;
      case Comparison::UINE:
        predicate = Comparison::SINE;
        break;
      case Comparison::SIEQ:
        predicate = Comparison::UIEQ;
        break;
      case Comparison::SINE:
        predicate = Comparison::UINE;
        break;
      default:
        ikos_unreachable("unreachable");
    }
  }
  auto c = Comparison::create(predicate, left, right);
  auto bb = cmp->parent();
  auto it = bb->begin();
  bb->insert_after(it, std::move(c));
}

} // end namespace ar
} // end namespace ikos
