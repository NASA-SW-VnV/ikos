/*******************************************************************************
 *
 * \file
 * \brief Implementation of UnifyExitNodesPass
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017-2019 United States Government as represented by the
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

#include <ikos/core/adt/small_vector.hpp>

#include <ikos/ar/pass/unify_exit_nodes.hpp>

namespace ikos {
namespace ar {

const char* UnifyExitNodesPass::name() const {
  return "unify-exit-nodes";
}

const char* UnifyExitNodesPass::description() const {
  return "Unify exit blocks";
}

bool UnifyExitNodesPass::run_on_code(Code* code) {
  core::SmallVector< BasicBlock*, 3 > exit_nodes;

  // Gather exit blocks

  if (code->has_exit_block()) {
    exit_nodes.push_back(code->exit_block());
  }
  if (code->has_unreachable_block()) {
    exit_nodes.push_back(code->unreachable_block());
  }
  if (code->has_ehresume_block()) {
    exit_nodes.push_back(code->ehresume_block());
  }

  // Merge exit nodes

  if (exit_nodes.empty()) {
    return false;
  } else if (exit_nodes.size() == 1) {
    code->set_exit_block(exit_nodes[0]);
    return true;
  } else {
    BasicBlock* unified_exit = BasicBlock::create(code);
    unified_exit->set_name("unified-exit");

    for (BasicBlock* bb : exit_nodes) {
      bb->add_successor(unified_exit);
    }

    // Update exit block, leave the rest unchanged
    code->set_exit_block(unified_exit);

    return true;
  }
}

} // end namespace ar
} // end namespace ikos
