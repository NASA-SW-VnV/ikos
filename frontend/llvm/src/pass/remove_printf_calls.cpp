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

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace llvm;

STATISTIC(NumKilled, "Number of printf-like calls removed");

namespace {

struct RemovePrintfCallsPass final : public FunctionPass {
  static char ID; // Pass identification

  RemovePrintfCallsPass() : FunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage&) const override {
    // All analyses are invalidated
  }

  bool runOnFunction(Function& f) override {
    SmallVector< CallInst*, 8 > to_erase;

    for (auto it = inst_begin(f), et = inst_end(f); it != et; ++it) {
      Instruction* inst = &*it;

      // looking for empty users
      if (!inst->use_empty()) {
        continue;
      }

      if (auto ci = dyn_cast< CallInst >(inst)) {
        Function* called = ci->getCalledFunction();

        if (called == nullptr) {
          continue;
        }

        if (called->isDeclaration()) {
          if (called->getName() == "fprintf" || called->getName() == "printf" ||
              called->getName() == "fputc" || called->getName() == "putc" ||
              called->getName() == "fputs" || called->getName() == "puts" ||
              called->getName() == "putchar" || called->getName() == "fwrite" ||
              called->getName() == "write") {
            to_erase.push_back(ci);
            ++NumKilled;
          }
        }
      }
    }

    for (CallInst* ci : to_erase) {
      ci->eraseFromParent();
    }

    return !to_erase.empty();
  }

}; // end struct RemovePrintfCallsPass

} // end anonymous namespace

char RemovePrintfCallsPass::ID = 0;

INITIALIZE_PASS(RemovePrintfCallsPass,
                "remove-printf-calls",
                "Remove printf-like calls",
                false,
                false);

FunctionPass* ikos::frontend::pass::create_remove_printf_calls_pass() {
  return new RemovePrintfCallsPass();
}
