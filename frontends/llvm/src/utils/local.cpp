#include "utils/local.hpp"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/Local.h"

#define DEBUG_TYPE "remove-unreachable-bb"

using namespace llvm;

STATISTIC(NumRemoved, "Number of unreachable basic blocks removed");

/// changeToUnreachable - Insert an unreachable instruction before the specified
/// instruction, making it and the rest of the code in the block dead.
void ikos_pp::changeToUnreachable(Instruction* I, bool UseLLVMTrap) {
  BasicBlock* BB = I->getParent();
  // Loop over all of the successors, removing BB's entry from any PHI
  // nodes.
  for (succ_iterator SI = succ_begin(BB), SE = succ_end(BB); SI != SE; ++SI)
    (*SI)->removePredecessor(BB);

  // Insert a call to llvm.trap right before this.  This turns the undefined
  // behavior into a hard fail instead of falling through into random code.
  if (UseLLVMTrap) {
    Function* TrapFn = Intrinsic::getDeclaration(BB->getParent()->getParent(),
                                                 Intrinsic::trap);
    CallInst* CallTrap = CallInst::Create(TrapFn, "", I);
    CallTrap->setDebugLoc(I->getDebugLoc());
  }
  new UnreachableInst(I->getContext(), I);

  // All instructions after this are dead.
#if (LLVM_VERSION_MAJOR == 3) && (LLVM_VERSION_MINOR == 7)
  BasicBlock::iterator BBI = I;
#else
  BasicBlock::iterator BBI = I->getIterator();
#endif
  BasicBlock::iterator BBE = BB->end();

  while (BBI != BBE) {
    if (!BBI->use_empty())
      BBI->replaceAllUsesWith(UndefValue::get(BBI->getType()));
    BB->getInstList().erase(BBI++);
  }
}

// /// changeToCall - Convert the specified invoke into a normal call.
// static void changeToCall(InvokeInst *II) {
//   SmallVector<Value*, 8> Args(II->op_begin(), II->op_end() - 3);
//   CallInst *NewCall = CallInst::Create(II->getCalledValue(), Args, "", II);
//   NewCall->takeName(II);
//   NewCall->setCallingConv(II->getCallingConv());
//   NewCall->setAttributes(II->getAttributes());
//   NewCall->setDebugLoc(II->getDebugLoc());
//   II->replaceAllUsesWith(NewCall);

//   // Follow the call by a branch to the normal destination.
//   BranchInst::Create(II->getNormalDest(), II);

//   // Update PHI nodes in the unwind destination
//   II->getUnwindDest()->removePredecessor(II->getParent());
//   II->eraseFromParent();
// }

static bool markAliveBlocks(BasicBlock* BB,
                            SmallPtrSet< BasicBlock*, 128 >& Reachable) {
  SmallVector< BasicBlock*, 128 > Worklist;
  Worklist.push_back(BB);
  Reachable.insert(BB);
  bool Changed = false;
  do {
    BB = Worklist.pop_back_val();

    // Do a quick scan of the basic block, turning any obviously unreachable
    // instructions into LLVM unreachable insts.  The instruction combining pass
    // canonicalizes unreachable insts into stores to null or undef.
    for (BasicBlock::iterator BBI = BB->begin(), E = BB->end(); BBI != E;
         ++BBI) {
      if (CallInst* CI = dyn_cast< CallInst >(BBI)) {
        if (CI->doesNotReturn()) {
          // If we found a call to a no-return function, insert an unreachable
          // instruction after it.  Make sure there isn't *already* one there
          // though.
          ++BBI;
          if (!isa< UnreachableInst >(BBI)) {
            // Don't insert a call to llvm.trap right before the unreachable.
            ikos_pp::changeToUnreachable(&*BBI, false);
            Changed = true;
          }
          break;
        }
      }

      // Store to undef and store to null are undefined and used to signal that
      // they should be changed to unreachable by passes that can't modify the
      // CFG.
      if (StoreInst* SI = dyn_cast< StoreInst >(BBI)) {
        // Don't touch volatile stores.
        if (SI->isVolatile())
          continue;

        Value* Ptr = SI->getOperand(1);

        if (isa< UndefValue >(Ptr) || (isa< ConstantPointerNull >(Ptr) &&
                                       SI->getPointerAddressSpace() == 0)) {
          ikos_pp::changeToUnreachable(SI, true);
          Changed = true;
          break;
        }
      }
    }

    // // Turn invokes that call 'nounwind' functions into ordinary calls.
    // if (InvokeInst *II = dyn_cast<InvokeInst>(BB->getTerminator())) {
    //   Value *Callee = II->getCalledValue();
    //   if (isa<ConstantPointerNull>(Callee) || isa<UndefValue>(Callee)) {
    //     changeToUnreachable(II, true);
    //     Changed = true;
    //   } else if (II->doesNotThrow()) {
    //     if (II->use_empty() && II->onlyReadsMemory()) {
    //       // jump to the normal destination branch.
    //       BranchInst::Create(II->getNormalDest(), II);
    //       II->getUnwindDest()->removePredecessor(II->getParent());
    //       II->eraseFromParent();
    //     } else
    //       changeToCall(II);
    //     Changed = true;
    //   }
    // }

    Changed |= ConstantFoldTerminator(BB);
    for (succ_iterator SI = succ_begin(BB), SE = succ_end(BB); SI != SE; ++SI)
      if (Reachable.insert(*SI).second)
        Worklist.push_back(*SI);
  } while (!Worklist.empty());
  return Changed;
}

/// removeUnreachableBlocksFromFn - Remove blocks that are not
/// reachable, even if they are in a dead cycle.  Return true if a
/// change was made, false otherwise.
bool ikos_pp::removeUnreachableBlocks(Function& F) {
  SmallPtrSet< BasicBlock*, 128 > Reachable;
  bool Changed = markAliveBlocks(&*F.begin(), Reachable);

  // If there are unreachable blocks in the CFG...
  if (Reachable.size() == F.size())
    return Changed;

  assert(Reachable.size() < F.size());
  NumRemoved += F.size() - Reachable.size();

  // Loop over all of the basic blocks that are not reachable, dropping all of
  // their internal references...
  for (Function::iterator BB = ++F.begin(), E = F.end(); BB != E; ++BB) {
    if (Reachable.count(&*BB))
      continue;

    for (succ_iterator SI = succ_begin(&*BB), SE = succ_end(&*BB); SI != SE;
         ++SI)
      if (Reachable.count(*SI))
        (*SI)->removePredecessor(&*BB);
    BB->dropAllReferences();
  }

  for (Function::iterator I = ++F.begin(); I != F.end();)
    if (!Reachable.count(&*I))
      I = F.getBasicBlockList().erase(I);
    else
      ++I;

  return true;
}
