/*******************************************************************************
 *
 * \file
 * \brief Implementation of the MarkNoReturnFunctionPass
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

#include <llvm/ADT/Statistic.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/Pass.h>
#include <llvm/Transforms/Utils/Local.h>
#include <llvm/Transforms/Utils/UnifyFunctionExitNodes.h>

#include <ikos/frontend/llvm/pass.hpp>

#define DEBUG_TYPE "mark-no-return-function"

using namespace llvm;

STATISTIC(NumNoReturn, "Number of functions without exit point");

namespace {

struct MarkNoReturnFunctionPass final : public ModulePass {
  static char ID; // Pass identification

  MarkNoReturnFunctionPass() : ModulePass(ID) {}

  void getAnalysisUsage(AnalysisUsage& AU) const override {
    AU.setPreservesAll();
    AU.addRequired< UnifyFunctionExitNodes >();
  }

  bool runOnModule(Module& M) override {
    bool change = false;
    for (Function& F : M) {
      change = runOnFunction(F) || change;
    }
    return change;
  }

  bool runOnFunction(Function& F) {
    if (F.isDeclaration()) {
      return false;
    }

    UnifyFunctionExitNodes* UFEN = &getAnalysis< UnifyFunctionExitNodes >(F);
    if (UFEN->getReturnBlock() != nullptr) {
      return false;
    }

    bool mayHaveSideEffects = false;
    for (auto I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      mayHaveSideEffects = mayHaveSideEffects || I->mayHaveSideEffects();
    }

    if (mayHaveSideEffects) {
      return false;
    }

    BasicBlock& Entry = F.getEntryBlock();
    if (Entry.begin() != Entry.end()) {
      Instruction& I = *(Entry.begin());
      changeToUnreachable(&I, false);
      NumNoReturn++;
      return true;
    }

    return false;
  }

}; // end struct MarkNoReturnFunctionPass

} // end anonymous namespace

char MarkNoReturnFunctionPass::ID = 0;

INITIALIZE_PASS(
    MarkNoReturnFunctionPass,
    "mark-no-return-function",
    "Mark as unreachable all side-effect-free functions without exit point",
    false,
    false);

ModulePass* ikos::frontend::pass::createMarkNoReturnFunctionPass() {
  return new MarkNoReturnFunctionPass();
}
