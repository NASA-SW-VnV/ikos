/*******************************************************************************
 *
 * Lower select instructions to three new basic blocks and a phi
 * instruction.
 *
 * Author: Jorge A. Navas
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2017 United States Government as represented by the
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

#define DEBUG_TYPE "lowerselect"

#include <vector>

#include "passes/lower_select.hpp"

STATISTIC(totalLowered, "Number of Lowered Select Instructions");

namespace ikos_pp {

using namespace llvm;

bool LowerSelect::runOnFunction(Function& F) {
  std::vector< SelectInst* > worklist;
  bool modified = false;
  // Initialization of the worklist with all select instructions from
  // the function
  for (inst_iterator It = inst_begin(F), E = inst_end(F); It != E; ++It) {
    Instruction* inst = &*It;
    if (SelectInst* SI = dyn_cast< SelectInst >(inst)) {
      if (!(SI->getCondition()->getType()->isIntegerTy(1))) {
        dbgs() << "We only lower a select if the flag is Boolean.\n";
        // note that the flag can be a vector of Boolean
        assert(false);
      }
      worklist.push_back(SI);
    }
  }

  while (!worklist.empty()) {
    modified = true;
    SelectInst* SI = worklist.back();
    worklist.pop_back();
    processSelectInst(SI);
  }

  return modified;
}

// Lower the select instruction into three new blocks.
void LowerSelect::processSelectInst(SelectInst* SI) {
  BasicBlock* curBlk = SI->getParent();
  Function* F = curBlk->getParent();
  Value* Flag = SI->getCondition();

  /// This splits a basic block into two at the specified instruction.
  /// All instructions BEFORE the specified iterator stay as part of
  /// the original basic block, an unconditional branch is added to
  /// the original BB, and the rest of the instructions in the BB are
  /// moved to the new BB, including the old terminator.
  /// IMPORTANT: this function invalidates the specified iterator.
  /// IMPORTANT: note that the select instructions goes to afterSelect
  /// Also note that this doesn't preserve any passes. To split blocks
  /// while keeping loop information consistent, use the SplitBlock
  /// utility function.
  BasicBlock* afterSelect =
      curBlk->splitBasicBlock(SI, curBlk->getName() + "LowerSelect");

  BasicBlock* trueBlock =
      BasicBlock::Create(F->getContext(), "TrueLowerSelect", F, afterSelect);
  BasicBlock* falseBlock =
      BasicBlock::Create(F->getContext(), "FalseLowerSelect", F, afterSelect);

  /// Wire trueBlock and falseblock to afterSelect via unconditional
  /// branch
  BranchInst::Create(afterSelect, trueBlock);
  BranchInst::Create(afterSelect, falseBlock);

  /// Replace the the unconditional branch added by splitBasicBlock
  /// with a conditional branch splitting on Flag **at the end** of
  /// curBlk
  curBlk->getTerminator()->eraseFromParent();
  BranchInst::Create(trueBlock, falseBlock, Flag, curBlk);

  // Insert a phi node just before the select instruction.
  PHINode* PHI = PHINode::Create(SI->getOperand(1)->getType(),
                                 SI->getNumOperands(),
                                 "PHILowerSelect",
                                 SI);
  PHI->addIncoming(SI->getOperand(1), trueBlock);
  PHI->addIncoming(SI->getOperand(2), falseBlock);

  // Make sure any users of the select is now an user of the phi node.
  SI->replaceAllUsesWith(PHI);

  // Finally we remove the select instruction
  SI->eraseFromParent();

  totalLowered++;
}

// Identifier for the pass
char LowerSelect::ID = 0;

} // end namespace ikos_pp

// Registration of the pass
static llvm::RegisterPass< ikos_pp::LowerSelect > X(
    "lowerselect", "Lower select instructions to branches", false, false);
