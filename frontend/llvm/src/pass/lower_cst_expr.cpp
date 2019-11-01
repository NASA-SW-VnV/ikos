/*******************************************************************************
 *
 * \file
 * \brief Implementation of the LowerCstExprPass
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
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

#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/Statistic.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/Pass.h>

#include <ikos/core/support/assert.hpp>

#include <ikos/frontend/llvm/pass.hpp>

#define DEBUG_TYPE "lower-cst-expr"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace llvm;

STATISTIC(TotalLowered, "Number of lowered constant expressions");

namespace {

/// \brief Lower Constant Expression Pass
///
/// Note that this pass does not lower constant aggregates that contain
/// constant expressions. This would require more work, because a constant
/// aggregate can only have constant operands.
struct LowerCstExprPass final : public FunctionPass {
  static char ID; // Pass identification

  LowerCstExprPass() : FunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage&) const override {
    // All analyses are invalidated
  }

  bool runOnFunction(Function& F) override {
    SmallPtrSet< Instruction*, 8 > worklist;

    for (auto it = inst_begin(F), et = inst_end(F); it != et; ++it) {
      Instruction* inst = &*it;

      if (has_cst_expr(inst)) {
        worklist.insert(inst);
      }
    }

    bool change = !worklist.empty();

    while (!worklist.empty()) {
      Instruction* inst = *worklist.begin();
      worklist.erase(inst);

      if (auto phi = dyn_cast< PHINode >(inst)) {
        for (unsigned i = 0; i < phi->getNumIncomingValues(); ++i) {
          Value* incoming_value = phi->getIncomingValue(i);

          if (auto cst_expr = dyn_cast< ConstantExpr >(incoming_value)) {
            BasicBlock* incoming_block = phi->getIncomingBlock(i);
            Instruction* insert_loc = incoming_block->getTerminator();
            Instruction* new_inst = lower_cst_expr(cst_expr, insert_loc);

            for (unsigned j = i; j < phi->getNumIncomingValues(); j++) {
              if (phi->getIncomingValue(j) == incoming_value &&
                  phi->getIncomingBlock(j) == incoming_block) {
                phi->setIncomingValue(j, new_inst);
              }
            }

            worklist.insert(new_inst);
          }
        }
      } else {
        for (unsigned i = 0; i < inst->getNumOperands(); ++i) {
          Value* operand = inst->getOperand(i);
          if (auto cst_expr = dyn_cast< ConstantExpr >(operand)) {
            Instruction* new_inst = lower_cst_expr(cst_expr, inst);
            inst->replaceUsesOfWith(cst_expr, new_inst);
            worklist.insert(new_inst);
          }
        }
      }
    }

    return change;
  }

  /// \brief Return true if the given instruction has constant expression
  /// operands
  static bool has_cst_expr(Instruction* inst) {
    if (isa< LandingPadInst >(inst)) {
      // Skip landingpad (especially the catch clause)
      // It has to be the first instruction in the basic block, so we won't be
      // able to insert instructions before it to lower a constant expression.
      return false;
    }
    for (auto it = inst->op_begin(), et = inst->op_end(); it != et; ++it) {
      if (isa< ConstantExpr >(*it)) {
        return true;
      }
    }
    return false;
  }

  /// \brief Lower the given constant expression
  static Instruction* lower_cst_expr(ConstantExpr* cst_expr,
                                     Instruction* insertion_loc) {
    Instruction* new_inst = cst_expr->getAsInstruction();
    ikos_assert_msg(new_inst, "Unhandled constant expression");
    new_inst->setDebugLoc(insertion_loc->getDebugLoc());
    new_inst->insertBefore(insertion_loc);
    TotalLowered++;
    return new_inst;
  }

}; // end struct LowerCstExprPass

} // end anonymous namespace

char LowerCstExprPass::ID = 0;

INITIALIZE_PASS(LowerCstExprPass,
                "lower-cst-expr",
                "Lower constant expressions to instructions",
                false,
                false);

FunctionPass* ikos::frontend::pass::create_lower_cst_expr_pass() {
  return new LowerCstExprPass();
}
