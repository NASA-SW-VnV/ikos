/*******************************************************************************
 *
 * \file
 * \brief Implementation of the LowerSelectPass
 *
 * Author: Jorge A. Navas
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

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/Statistic.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/Pass.h>
#include <llvm/Support/Debug.h>

#include <ikos/frontend/llvm/pass.hpp>

#define DEBUG_TYPE "lower-select"

using namespace llvm;

STATISTIC(TotalLowered, "Number of lowered select instructions");

namespace {

struct LowerSelectPass final : public FunctionPass {
  static char ID; // Pass identification

  LowerSelectPass() : FunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage& /*AU*/) const override {}

  bool runOnFunction(Function& F) override {
    SmallVector< SelectInst*, 8 > Worklist;

    // Initialization of the worklist with all select instructions within
    // the function
    for (auto I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      Instruction* Inst = &*I;

      if (auto SI = dyn_cast< SelectInst >(Inst)) {
        if (!(SI->getCondition()->getType()->isIntegerTy(1))) {
          // note that the flag can be a vector of Boolean
          dbgs() << "We only lower a select if the flag is Boolean.\n";
          assert(false && "unexpected select");
        }
        Worklist.push_back(SI);
      }
    }

    bool change = !Worklist.empty();

    for (SelectInst* SI : Worklist) {
      processSelectInst(SI);
    }

    return change;
  }

  /// \brief Lower the select instruction into three new blocks.
  void processSelectInst(SelectInst* SI) {
    BasicBlock* CurrentBlock = SI->getParent();
    Function* F = CurrentBlock->getParent();
    Value* Flag = SI->getCondition();

    // This splits a basic block into two at the specified instruction.
    // All instructions BEFORE the specified iterator stay as part of
    // the original basic block, an unconditional branch is added to
    // the original BB, and the rest of the instructions in the BB are
    // moved to the new BB, including the old terminator.
    // IMPORTANT: this function invalidates the specified iterator.
    // IMPORTANT: note that the select instructions goes to AfterSelect
    // Also note that this doesn't preserve any passes. To split blocks
    // while keeping loop information consistent, use the SplitBlock
    // utility function.
    BasicBlock* AfterSelect = CurrentBlock->splitBasicBlock(SI);
    BasicBlock* TrueBlock =
        BasicBlock::Create(F->getContext(), "", F, AfterSelect);
    BasicBlock* FalseBlock =
        BasicBlock::Create(F->getContext(), "", F, AfterSelect);

    if (CurrentBlock->hasName()) {
      AfterSelect->setName(CurrentBlock->getName() + ".AfterSelect");
      TrueBlock->setName(CurrentBlock->getName() + ".TrueSelect");
      FalseBlock->setName(CurrentBlock->getName() + ".FalseSelect");
    }

    // Wire TrueBlock and FalseBlock to AfterSelect via unconditional branch
    BranchInst* TrueBr = BranchInst::Create(AfterSelect, TrueBlock);
    BranchInst* FalseBr = BranchInst::Create(AfterSelect, FalseBlock);
    TrueBr->setDebugLoc(SI->getDebugLoc());
    FalseBr->setDebugLoc(SI->getDebugLoc());

    // Replace the unconditional branch added by splitBasicBlock
    // with a conditional branch splitting on Flag **at the end** of
    // CurrentBlock
    CurrentBlock->getTerminator()->eraseFromParent();
    BranchInst* Br =
        BranchInst::Create(TrueBlock, FalseBlock, Flag, CurrentBlock);
    Br->setDebugLoc(SI->getDebugLoc());

    // Insert a phi node just before the select instruction.
    PHINode* PHI = PHINode::Create(SI->getOperand(1)->getType(),
                                   SI->getNumOperands(),
                                   "",
                                   SI);
    PHI->addIncoming(SI->getOperand(1), TrueBlock);
    PHI->addIncoming(SI->getOperand(2), FalseBlock);
    if (SI->hasName()) {
      PHI->setName(SI->getName() + ".phi");
    }
    PHI->setDebugLoc(SI->getDebugLoc());

    // Make sure any users of the select is now an user of the phi node.
    SI->replaceAllUsesWith(PHI);

    // Finally we remove the select instruction
    SI->eraseFromParent();

    TotalLowered++;
  }

}; // end struct LowerSelectPass

} // end anonymous namespace

char LowerSelectPass::ID = 0;

INITIALIZE_PASS(LowerSelectPass,
                "lower-select",
                "Lower select instructions to branches",
                false,
                false);

FunctionPass* ikos::frontend::pass::createLowerSelectPass() {
  return new LowerSelectPass();
}
