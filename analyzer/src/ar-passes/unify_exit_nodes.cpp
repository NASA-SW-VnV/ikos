/*******************************************************************************
 *
 * Unify Exit Nodes pass.
 *
 * This pass creates an extra basic block to unify all exit nodes (exit,
 * unreachable, ehresume). This is necessary for the backward fixpoint iterator
 * in the analyzer.
 *
 * Authors: Maxime Arthaud
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

#include <analyzer/ar-wrapper/cfg.hpp>
#include <analyzer/ar-wrapper/wrapper.hpp>

namespace arbos {
namespace passes {

class UnifyExitNodesPass : public Pass {
public:
  UnifyExitNodesPass() : Pass("unify-exit-nodes", "Unify exit nodes") {}
  virtual ~UnifyExitNodesPass() {}

  virtual void execute(Bundle_ref bundle) {
    FuncRange functions = ar::getFunctions(bundle);
    for (auto it = functions.begin(); it != functions.end(); ++it) {
      execute(*it);
    }
  }

  void execute(Function_ref f) {
    std::vector< Basic_Block_ref > exit_nodes;
    Code_ref body = ar::getBody(f);

    /*
     * Collect all exit nodes (exit, unreachable, resume)
     */

    boost::optional< Basic_Block_ref > exit = ar::getExitBlock(body);
    if (exit) {
      exit_nodes.push_back(*exit);
    }

    boost::optional< Basic_Block_ref > unreachable =
        ar::getUnreachableBlock(body);
    if (unreachable) {
      exit_nodes.push_back(*unreachable);
    }

    boost::optional< Basic_Block_ref > ehresume = ar::getEHResumeBlock(body);
    if (ehresume) {
      exit_nodes.push_back(*ehresume);
    }

    /*
     * Merge exit nodes
     */
    Basic_Block_ref unified_exit = Null_ref;

    if (exit_nodes.empty())
      return;

    if (exit_nodes.size() == 1) {
      unified_exit = exit_nodes.front();
    } else {
      unified_exit = AR_Basic_Block::create("_unified_exit");
      body->addBasicBlock(unified_exit);

      for (std::vector< Basic_Block_ref >::iterator it = exit_nodes.begin();
           it != exit_nodes.end();
           ++it) {
        (*it)->addNextBasicBlock(unified_exit);
      }
    }

    assert(!ar::ar_internal::is_null_ref(unified_exit));
    body->setExitBlock(unified_exit);
    body->setUnreachableBlock(Null_ref);
    body->setEHResumeBlock(Null_ref);
  }
}; // end class UnifyExitNodesPass

} // end namespace passes
} // end namespace arbos

extern "C" arbos::Pass* init() {
  return new arbos::passes::UnifyExitNodesPass();
}
