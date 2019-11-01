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

#include <ikos/core/support/assert.hpp>

#include <ikos/frontend/llvm/pass.hpp>

#define DEBUG_TYPE "lower-select"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace llvm;

STATISTIC(TotalLowered, "Number of lowered select instructions");

namespace {

struct LowerSelectPass final : public FunctionPass {
  static char ID; // Pass identification

  LowerSelectPass() : FunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage&) const override {
    // All analyses are invalidated
  }

  bool runOnFunction(Function& f) override {
    SmallVector< SelectInst*, 8 > worklist;

    // Initialization of the worklist with all select instructions within
    // the function
    for (auto it = inst_begin(f), et = inst_end(f); it != et; ++it) {
      Instruction* inst = &*it;

      if (auto si = dyn_cast< SelectInst >(inst)) {
        if (!(si->getCondition()->getType()->isIntegerTy(1))) {
          // note that the flag can be a vector of Boolean
          dbgs() << "We only lower a select if the flag is Boolean.\n";
          ikos_unreachable("unexpected select");
        }
        worklist.push_back(si);
      }
    }

    bool change = !worklist.empty();

    for (SelectInst* si : worklist) {
      process_select_inst(si);
    }

    return change;
  }

  /// \brief Lower the select instruction into three new blocks.
  void process_select_inst(SelectInst* si) {
    BasicBlock* current_block = si->getParent();
    Function* f = current_block->getParent();
    Value* flag = si->getCondition();

    // This splits a basic block into two at the specified instruction.
    // All instructions BEFORE the specified iterator stay as part of
    // the original basic block, an unconditional branch is added to
    // the original BB, and the rest of the instructions in the BB are
    // moved to the new BB, including the old terminator.
    // IMPORTANT: this function invalidates the specified iterator.
    // IMPORTANT: note that the select instructions goes to after_select
    // Also note that this doesn't preserve any passes. To split blocks
    // while keeping loop information consistent, use the SplitBlock
    // utility function.
    BasicBlock* after_select = current_block->splitBasicBlock(si);
    BasicBlock* true_block =
        BasicBlock::Create(f->getContext(), "", f, after_select);
    BasicBlock* false_block =
        BasicBlock::Create(f->getContext(), "", f, after_select);

    if (current_block->hasName()) {
      after_select->setName(current_block->getName() + ".AfterSelect");
      true_block->setName(current_block->getName() + ".TrueSelect");
      false_block->setName(current_block->getName() + ".FalseSelect");
    }

    // Wire true_block and false_block to after_select via unconditional branch
    BranchInst* true_br = BranchInst::Create(after_select, true_block);
    BranchInst* false_br = BranchInst::Create(after_select, false_block);
    true_br->setDebugLoc(si->getDebugLoc());
    false_br->setDebugLoc(si->getDebugLoc());

    // Replace the unconditional branch added by splitBasicBlock
    // with a conditional branch splitting on flag **at the end** of
    // current_block
    current_block->getTerminator()->eraseFromParent();
    BranchInst* br =
        BranchInst::Create(true_block, false_block, flag, current_block);
    br->setDebugLoc(si->getDebugLoc());

    // Insert a phi node just before the select instruction.
    PHINode* phi = PHINode::Create(si->getOperand(1)->getType(),
                                   si->getNumOperands(),
                                   "",
                                   si);
    phi->addIncoming(si->getOperand(1), true_block);
    phi->addIncoming(si->getOperand(2), false_block);
    if (si->hasName()) {
      phi->setName(si->getName() + ".phi");
    }
    phi->setDebugLoc(si->getDebugLoc());

    // Make sure any users of the select is now an user of the phi node.
    si->replaceAllUsesWith(phi);

    // Finally we remove the select instruction
    si->eraseFromParent();

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

FunctionPass* ikos::frontend::pass::create_lower_select_pass() {
  return new LowerSelectPass();
}
