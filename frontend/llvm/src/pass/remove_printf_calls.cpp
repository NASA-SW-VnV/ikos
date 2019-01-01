/*******************************************************************************
 *
 * \file
 * \brief Implementation of the RemovePrintfCallsPass
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
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/User.h>
#include <llvm/IR/Value.h>

#include <ikos/frontend/llvm/pass.hpp>

#define DEBUG_TYPE "remove-printf-call"

using namespace llvm;

STATISTIC(NumKilled, "Number of printf-like calls removed");

namespace {

struct RemovePrintfCallsPass final : public FunctionPass {
  static char ID; // Pass identification

  RemovePrintfCallsPass() : FunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage& /*AU*/) const override {}

  bool runOnFunction(Function& F) override {
    SmallVector< CallInst*, 8 > ToErase;

    for (auto I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      Instruction* Inst = &*I;

      // looking for empty users
      if (!Inst->use_empty()) {
        continue;
      }

      if (auto CI = dyn_cast< CallInst >(Inst)) {
        Function* Called = CI->getCalledFunction();

        if (Called == nullptr) {
          continue;
        }

        if (Called->isDeclaration()) {
          if (Called->getName() == "fprintf" || Called->getName() == "printf" ||
              Called->getName() == "fputc" || Called->getName() == "putc" ||
              Called->getName() == "fputs" || Called->getName() == "puts" ||
              Called->getName() == "putchar" || Called->getName() == "fwrite" ||
              Called->getName() == "write") {
            ToErase.push_back(CI);
            ++NumKilled;
          }
        }
      }
    }

    for (CallInst* CI : ToErase) {
      CI->eraseFromParent();
    }

    return !ToErase.empty();
  }

}; // end struct RemovePrintfCallsPass

} // end anonymous namespace

char RemovePrintfCallsPass::ID = 0;

INITIALIZE_PASS(RemovePrintfCallsPass,
                "remove-printf-calls",
                "Remove printf-like calls",
                false,
                false);

FunctionPass* ikos::frontend::pass::createRemovePrintfCallsPass() {
  return new RemovePrintfCallsPass();
}
