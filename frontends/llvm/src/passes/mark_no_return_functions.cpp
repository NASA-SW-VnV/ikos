/*******************************************************************************
 *
 * Mark as unreachable any non side-effect function that does not
 * return.
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

#define DEBUG_TYPE "mark-fn-with-no-exit-point-unreachable"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

#include "passes/mark_no_return_functions.hpp"

STATISTIC(NumNoReturn, "Number of functions without exit point");

namespace ikos_pp {

using namespace llvm;

char MarkNoReturnFunctions::ID = 0;

bool MarkNoReturnFunctions::runOnModule(llvm::Module& M) {
  bool change = false;
  for (Module::iterator F = M.begin(); F != M.end(); F++) {
    if (F->isDeclaration())
      continue;

    UnifyFunctionExitNodes* UFEN = &getAnalysis< UnifyFunctionExitNodes >(*F);
    if (UFEN->getReturnBlock())
      continue;

    bool mayHaveSideEffects = false;
    for (inst_iterator It = inst_begin(&*F), Et = inst_end(&*F); It != Et; ++It)
      mayHaveSideEffects |= It->mayHaveSideEffects();

    if (mayHaveSideEffects)
      continue;

    BasicBlock& entry = F->getEntryBlock();
    if (entry.begin() != entry.end()) {
      Instruction& I = *(entry.begin());
      llvm::changeToUnreachable(&I, false);
      NumNoReturn++;
      change = true;
    }
  }
  return change;
}

void MarkNoReturnFunctions::getAnalysisUsage(llvm::AnalysisUsage& AU) const {
  AU.setPreservesAll();
  AU.addRequired< llvm::UnifyFunctionExitNodes >();
}

} // end namespace ikos_pp

static llvm::RegisterPass< ikos_pp::MarkNoReturnFunctions > X(
    "mark-noret-functions",
    "Mark as unreachable all side-effect-free functions without exit point",
    false,
    false);
