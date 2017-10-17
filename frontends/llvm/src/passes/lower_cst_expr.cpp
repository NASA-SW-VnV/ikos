/*******************************************************************************
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

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/raw_ostream.h"

#include "passes/lower_cst_expr.hpp"

namespace ikos_pp {

using namespace llvm;

char LowerCstExprPass::ID = 0;

bool LowerCstExprPass::runOnModule(Module& M) {
  bool change = false;
  for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F)
    change |= runOnFunction(*F);
  return change;
}

bool LowerCstExprPass::runOnFunction(Function& F) {
  SmallPtrSet< Instruction*, 8 > worklist;
  typedef SmallPtrSet< Instruction*, 8 >::iterator worklist_it;

  for (inst_iterator It = inst_begin(F), E = inst_end(F); It != E; ++It) {
    Instruction* I = &*It;

    if (dyn_cast< LandingPadInst >(I))
      continue;

    for (unsigned int i = 0; i < I->getNumOperands(); ++i) {
      if (hasCstExpr(I->getOperand(i)))
        worklist.insert(I);
    }
  }

  bool change = !worklist.empty();

  while (!worklist.empty()) {
    worklist_it It = worklist.begin();
    Instruction* I = *It;
    worklist.erase(*It);

    if (PHINode* PHI = dyn_cast< PHINode >(I)) {
      for (unsigned int i = 0; i < PHI->getNumIncomingValues(); ++i) {
        Instruction* InsertLoc = PHI->getIncomingBlock(i)->getTerminator();
        assert(InsertLoc);

        if (ConstantExpr* CstExp = hasCstExpr(PHI->getIncomingValue(i))) {
          Instruction* NewInst = lowerCstExpr(CstExp, InsertLoc);
          for (unsigned int j = i; j < PHI->getNumIncomingValues(); j++) {
            if ((PHI->getIncomingValue(j) == PHI->getIncomingValue(i)) &&
                (PHI->getIncomingBlock(j) == PHI->getIncomingBlock(i))) {
              PHI->setIncomingValue(j, NewInst);
            }
          }
          worklist.insert(NewInst);
        }
      }
    } else {
      for (unsigned int i = 0; i < I->getNumOperands(); ++i) {
        if (ConstantExpr* CstExp = hasCstExpr(I->getOperand(i))) {
          Instruction* NewInst = lowerCstExpr(CstExp, I);
          I->replaceUsesOfWith(CstExp, NewInst);
          worklist.insert(NewInst);
        }
      }
    }
  }
  return change;
}

ConstantExpr* LowerCstExprPass::hasCstExpr(Value* V) {
  if (Constant* cst = dyn_cast< Constant >(V)) {
    if (ConstantExpr* ce = dyn_cast< ConstantExpr >(V))
      return ce;
    else {
      // for ConstantStruct, ConstantArray, etc, we need to check
      // recursively.
      for (unsigned u = 0; u < cst->getNumOperands(); ++u) {
        Use& p = cst->getOperandUse(u);
        if (ConstantExpr* cst_exp_i = hasCstExpr(p.get()))
          return cst_exp_i;
      }
    }
  }
  return NULL;
}

Instruction* LowerCstExprPass::lowerCstExpr(ConstantExpr* CstExp,
                                            Instruction* InsertionLoc) {
  Instruction* NewInst = CstExp->getAsInstruction();
  assert(NewInst && "unhandled constant expression");
  NewInst->setDebugLoc(InsertionLoc->getDebugLoc());
  InsertionLoc->getParent()->getInstList().insert(InsertionLoc->getIterator(),
                                                  NewInst);
  return NewInst;
}

} // end namespace ikos_pp

static llvm::RegisterPass< ikos_pp::LowerCstExprPass > X(
    "lowercstexp", "Lower constant expressions to instructions");
