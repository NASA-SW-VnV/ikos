/*******************************************************************************
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
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

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "passes/name_values.hpp"

namespace ikos_pp {

using namespace llvm;

char NameValues::ID = 0;

bool NameValues::runOnModule(Module& M) {
  for (Module::global_iterator GI = M.global_begin(), E = M.global_end();
       GI != E;
       ++GI) {
    runOnGlobal(*GI);
  }
  for (Module::iterator FI = M.begin(), E = M.end(); FI != E; ++FI) {
    runOnFunction(*FI);
  }
  return false;
}

bool NameValues::runOnGlobal(GlobalVariable& G) {
  StringRef name = G.getName();
  if (!name.startswith(".") && !name.startswith("llvm.") &&
      name.find('.') != StringRef::npos) {
    G.setName("." + name);
  }
  return false;
}

bool NameValues::runOnFunction(Function& F) {
  std::string prefix = F.getName().str() + ".";

  unsigned int arg_id = 1;
  for (Function::arg_iterator AI = F.arg_begin(), AE = F.arg_end(); AI != AE;
       ++AI, ++arg_id) {
    Argument& A = *AI;

    if (!A.hasName() && !A.getType()->isVoidTy()) {
      AI->setName("arg_" + std::to_string(arg_id));
    }
    if (!A.getType()->isVoidTy() && !A.getName().startswith(prefix)) {
      AI->setName(prefix + A.getName());
    }
  }

  unsigned int block_id = 1;
  unsigned int inst_id = 1;
  for (Function::iterator BI = F.begin(), BE = F.end(); BI != BE;
       ++BI, ++block_id) {
    BasicBlock& BB = *BI;

    if (!BB.hasName()) {
      BB.setName("bb_" + std::to_string(block_id));
    }

    for (BasicBlock::iterator II = BB.begin(), IE = BB.end(); II != IE;
         ++II, ++inst_id) {
      Instruction& I = *II;

      if (!I.hasName() && !(I.getType()->isVoidTy())) {
        I.setName("_" + std::to_string(inst_id));
      }
      if (!I.getType()->isVoidTy() && !I.getName().startswith(prefix)) {
        I.setName(prefix + I.getName());
      }
    }
  }

  return false;
}

} // end namespace ikos_pp

static llvm::RegisterPass< ikos_pp::NameValues > X("name-values",
                                                   "Names all unnamed values");
