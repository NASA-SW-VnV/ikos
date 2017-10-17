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

#define DEBUG_TYPE "remove-printf-like-calls"

#include <vector>

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"

#include "passes/remove_printf_calls.hpp"

STATISTIC(NumKilled, "Number of printf-like calls removed");

namespace ikos_pp {

using namespace llvm;

char RemovePrintfCalls::ID = 0;

bool RemovePrintfCalls::runOnFunction(Function& F) {
  // std::forward_list<CallInst*> toerase;
  std::vector< CallInst* > toerase;

  for (Function::iterator b = F.begin(), be = F.end(); b != be; ++b)
    for (BasicBlock::iterator it = b->begin(), ie = b->end(); it != ie; ++it) {
      User* u = &(*it);

      // -- looking for empty users
      if (!u->use_empty())
        continue;

      if (CallInst* ci = dyn_cast< CallInst >(u)) {
        Function* f = ci->getCalledFunction();
        if (f == NULL)
          continue;

        if (f->isDeclaration()) {
          if ((f->getName() == "printf") || (f->getName() == "fprintf") ||
              (f->getName() == "puts") || (f->getName() == "write")) {
            toerase.push_back(ci);
            ++NumKilled;
          }
        }
      }
    }

  for (std::vector< CallInst* >::iterator it = toerase.begin();
       it != toerase.end();
       ++it) {
    CallInst* ci = *it;
    ci->eraseFromParent();
  }

  return !toerase.empty();
}
}

static llvm::RegisterPass< ikos_pp::RemovePrintfCalls > X(
    "remove-printf-calls", "Remove print-like calls");
