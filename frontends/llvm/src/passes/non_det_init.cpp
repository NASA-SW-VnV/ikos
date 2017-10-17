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

#define DEBUG_TYPE "nondet"

#include <list>
#include <map>

#include <boost/format.hpp>

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/raw_ostream.h"

STATISTIC(NumReplaced, "Number of undef made nondet");
STATISTIC(NumKilled, "Number of nondet calls killed");

namespace ikos_pp {

using namespace llvm;

class NondetInit : public ModulePass {
private:
  /** map for nondet functions */
  DenseMap< const Type*, Constant* > m_ndfn;
  Module* m;

  Constant* getNondetFn(Type* type) {
    Constant* res = m_ndfn[type];
    if (res == NULL) {
      res = m->getOrInsertFunction((boost::format("ikos_pp.nondet.%d") %
                                    m_ndfn.size())
                                       .str(),
                                   type,
                                   NULL);
      m_ndfn[type] = res;
    }
    return res;
  }

public:
  static char ID;

  NondetInit() : ModulePass(ID), m(NULL) {}

  virtual bool runOnModule(Module& M) {
    m = &M;
    bool Changed = false;

    // Iterate over all functions, basic blocks and instructions.
    for (Module::iterator FI = M.begin(), E = M.end(); FI != E; ++FI)
      Changed |= runOnFunction(*FI);

    return Changed;
  }

  virtual void releaseMemory() { m_ndfn.clear(); }

  bool runOnFunction(Function& F) {
    bool Changed = false;

    for (Function::iterator b_it = F.begin(), b_et = F.end(); b_it != b_et;
         ++b_it) {
      llvm::BasicBlock& b = *b_it;
      for (Value::use_iterator u_it = b.use_begin(), u_et = b.use_end();
           u_it != u_et;
           ++u_it) {
        User* u = u_it->getUser();
        // phi-node
        if (PHINode* phi = dyn_cast< PHINode >(u)) {
          for (unsigned i = 0; i < phi->getNumIncomingValues(); i++) {
            if (UndefValue* uv =
                    dyn_cast< UndefValue >(phi->getIncomingValue(i))) {
              Constant* ndf = getNondetFn(uv->getType());
              IRBuilder<> Builder(F.getContext());
              Builder.SetInsertPoint(&F.getEntryBlock(),
                                     F.getEntryBlock().begin());
              phi->setIncomingValue(i, Builder.CreateCall(ndf));
              ++NumReplaced;
              Changed = true;
            }
          }
          continue;
        }

        // -- the normal case
        for (unsigned i = 0; i < u->getNumOperands(); i++) {
          if (UndefValue* uv = dyn_cast< UndefValue >(u->getOperand(i))) {
            Constant* ndf = getNondetFn(uv->getType());
            IRBuilder<> Builder(F.getContext());
            Builder.SetInsertPoint(&F.getEntryBlock(),
                                   F.getEntryBlock().begin());
            u->setOperand(i, Builder.CreateCall(ndf));
            ++NumReplaced;
            Changed = true;
          }
        }
      }
    }
    return Changed;
  }

  virtual void getAnalysisUsage(AnalysisUsage& AU) const {
    AU.setPreservesAll();
  }
};

char NondetInit::ID = 0;

class KillUnusedNondet : public FunctionPass {
public:
  static char ID;
  KillUnusedNondet() : FunctionPass(ID) {}

  bool runOnFunction(Function& F) {
    // std::forward_list<CallInst*> toerase;
    std::list< CallInst* > toerase;

    for (Function::iterator b = F.begin(), be = F.end(); b != be; ++b)
      for (BasicBlock::iterator it = b->begin(), ie = b->end(); it != ie;
           ++it) {
        User* u = &(*it);
        // -- looking for empty users
        if (!u->use_empty())
          continue;

        if (CallInst* ci = dyn_cast< CallInst >(u)) {
          Function* f = ci->getCalledFunction();
          if (f == NULL)
            continue;

          if (f->getName().startswith("ikos_pp.nondet")) {
            toerase.push_front(ci);
            ++NumKilled;
          }
        }
      }

    for (std::list< CallInst* >::iterator it = toerase.begin();
         it != toerase.end();
         ++it) {
      CallInst* ci = *it;
      ci->eraseFromParent();
    }

    return !toerase.empty();
  }

  virtual void getAnalysisUsage(AnalysisUsage& AU) const {
    AU.setPreservesAll();
  }
};

char KillUnusedNondet::ID = 0;

llvm::Pass* createNondetInitPass() {
  return new NondetInit();
}
llvm::Pass* createDeadNondetElimPass() {
  return new KillUnusedNondet();
}

} // end namespace ikos_pp

static llvm::RegisterPass< ikos_pp::NondetInit > X(
    "nondet-init",
    "Non-deterministic initialization"
    "of all alloca.");

static llvm::RegisterPass< ikos_pp::KillUnusedNondet > Y(
    "kill-nondet", "Remove unused nondet calls.");
