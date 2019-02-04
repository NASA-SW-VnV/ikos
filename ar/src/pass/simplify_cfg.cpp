/*******************************************************************************
 *
 * \file
 * \brief Implementation of SimplifyCFGPass
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

#include <ikos/ar/pass/simplify_cfg.hpp>
#include <ikos/ar/semantic/statement.hpp>

namespace ikos {
namespace ar {

/// \brief Merge the given basic block with its child, if possible
static bool merge_single_block(Code* code, BasicBlock* bb) {
  if (bb->num_successors() != 1) {
    return false;
  }

  BasicBlock* child = *bb->successor_begin();

  if (bb == child || child->num_predecessors() != 1 ||
      child->is_successor(child) || code->entry_block() == child) {
    return false;
  }

  // Move the statements from child to bb
  std::vector< std::unique_ptr< Statement > > stmts;
  stmts.reserve(child->num_statements());

  while (!child->empty()) {
    stmts.push_back(child->pop_back());
  }
  while (!stmts.empty()) {
    bb->push_back(std::move(stmts.back()));
    stmts.pop_back();
  }

  // Update edges
  bb->clear_successors();
  for (auto it = child->successor_begin(), et = child->successor_end();
       it != et;
       ++it) {
    bb->add_successor(*it);
  }
  child->clear_predecessors();
  child->clear_successors();
  // child will be removed later, see remove_unreachable_blocks()

  // Update exit block
  if (code->exit_block_or_null() == child) {
    code->set_exit_block(bb);
  }

  // Update traceability
  if (!bb->has_frontend()) {
    bb->set_frontend(*child);
  }

  return true;
}

/// \brief Merge basic blocks whenever possible
static bool merge_single_blocks(Code* code) {
  bool change = false;
  bool done = false;

  while (!done) {
    done = true;

    for (auto it = code->begin(), et = code->end(); it != et; ++it) {
      while (merge_single_block(code, *it)) {
        change = true;
        done = false;
      }
    }
  }

  return change;
}

/// \brief Remove unreachable basic blocks
static bool remove_unreachable_blocks(Code* code) {
  bool change = false;
  bool done = false;

  while (!done) {
    std::vector< BasicBlock* > to_remove;
    done = true;

    for (auto it = code->begin(), et = code->end(); it != et; ++it) {
      BasicBlock* bb = *it;

      if (bb->num_predecessors() == 0 && code->entry_block() != bb) {
        to_remove.push_back(bb);
      }
    }

    for (BasicBlock* bb : to_remove) {
      code->erase_basic_block(bb);
      change = true;
      done = false;
    }
  }

  return change;
}

const char* SimplifyCFGPass::name() const {
  return "simplify-cfg";
}

const char* SimplifyCFGPass::description() const {
  return "Simplify the Control Flow Graphs";
}

bool SimplifyCFGPass::run_on_code(Code* code) {
  bool change = false;
  change |= merge_single_blocks(code);
  change |= remove_unreachable_blocks(code);
  return change;
}

} // end namespace ar
} // end namespace ikos
