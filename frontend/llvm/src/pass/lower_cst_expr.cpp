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

#include <ikos/frontend/llvm/pass.hpp>

#define DEBUG_TYPE "lower-cst-expr"

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

  void getAnalysisUsage(AnalysisUsage& AU) const override {
    AU.setPreservesAll();
  }

  bool runOnFunction(Function& F) override {
    SmallPtrSet< Instruction*, 8 > Worklist;

    for (auto I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      Instruction* Inst = &*I;

      if (hasCstExpr(Inst)) {
        Worklist.insert(Inst);
      }
    }

    bool change = !Worklist.empty();

    while (!Worklist.empty()) {
      Instruction* Inst = *Worklist.begin();
      Worklist.erase(Inst);

      if (auto PHI = dyn_cast< PHINode >(Inst)) {
        for (unsigned i = 0; i < PHI->getNumIncomingValues(); ++i) {
          Value* IncomingValue = PHI->getIncomingValue(i);

          if (auto CstExpr = dyn_cast< ConstantExpr >(IncomingValue)) {
            BasicBlock* IncomingBlock = PHI->getIncomingBlock(i);
            Instruction* InsertLoc = IncomingBlock->getTerminator();
            Instruction* NewInst = lowerCstExpr(CstExpr, InsertLoc);

            for (unsigned j = i; j < PHI->getNumIncomingValues(); j++) {
              if (PHI->getIncomingValue(j) == IncomingValue &&
                  PHI->getIncomingBlock(j) == IncomingBlock) {
                PHI->setIncomingValue(j, NewInst);
              }
            }

            Worklist.insert(NewInst);
          }
        }
      } else {
        for (unsigned i = 0; i < Inst->getNumOperands(); ++i) {
          Value* Operand = Inst->getOperand(i);
          if (auto CstExpr = dyn_cast< ConstantExpr >(Operand)) {
            Instruction* NewInst = lowerCstExpr(CstExpr, Inst);
            Inst->replaceUsesOfWith(CstExpr, NewInst);
            Worklist.insert(NewInst);
          }
        }
      }
    }

    return change;
  }

  /// \brief Return true if the given instruction has constant expression
  /// operands
  static bool hasCstExpr(Instruction* Inst) {
    if (isa< LandingPadInst >(Inst)) {
      // Skip landingpad (especially the catch clause)
      // It has to be the first instruction in the basic block, so we won't be
      // able to insert instructions before it to lower a constant expression.
      return false;
    }
    for (auto I = Inst->op_begin(), E = Inst->op_end(); I != E; ++I) {
      if (isa< ConstantExpr >(*I)) {
        return true;
      }
    }
    return false;
  }

  /// \brief Lower the given constant expression
  static Instruction* lowerCstExpr(ConstantExpr* CstExpr,
                                   Instruction* InsertionLoc) {
    Instruction* NewInst = CstExpr->getAsInstruction();
    assert(NewInst && "Unhandled constant expression");
    NewInst->setDebugLoc(InsertionLoc->getDebugLoc());
    NewInst->insertBefore(InsertionLoc);
    TotalLowered++;
    return NewInst;
  }

}; // end struct LowerCstExprPass

} // end anonymous namespace

char LowerCstExprPass::ID = 0;

INITIALIZE_PASS(LowerCstExprPass,
                "lower-cst-expr",
                "Lower constant expressions to instructions",
                false,
                false);

FunctionPass* ikos::frontend::pass::createLowerCstExprPass() {
  return new LowerCstExprPass();
}
