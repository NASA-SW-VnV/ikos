/*******************************************************************************
 *
 * Add a loop counter within every loop.
 *
 * This pass adds an initialization statement that sets the counter to zero
 * in all basic blocks before the loop, and then adds a statement that
 * increments the counter by one within the loop.
 *
 * Authors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017 United States Government as represented by the
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

#include <ikos/iterators/wto.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/ar-wrapper/cfg.hpp>
#include <analyzer/ar-wrapper/wrapper.hpp>

//#define DEBUG

namespace arbos {
namespace passes {

// Wrapper of AR_Function that follows ikos::cfg interface
class FunctionCFG {
public:
  typedef BBRange node_collection_t;

private:
  Function_ref _function;

public:
  FunctionCFG(Function_ref function) : _function(function) {}

  Basic_Block_ref entry() { return ar::getEntryBlock(ar::getBody(_function)); }

  Basic_Block_ref get_node(Basic_Block_ref b) { return b; }

  node_collection_t next_nodes(Basic_Block_ref b) { return ar::getSuccs(b); }

  node_collection_t prev_nodes(Basic_Block_ref b) { return ar::getPreds(b); }

}; // end class FunctionCFG

// Iterate over all cycles in the control flow graph
class loop_iterator
    : public ikos::wto_component_visitor< Basic_Block_ref, FunctionCFG > {
private:
  typedef ikos::wto_vertex< Basic_Block_ref, FunctionCFG > wto_vertex_t;
  typedef ikos::wto_cycle< Basic_Block_ref, FunctionCFG > wto_cycle_t;
  typedef std::unordered_set< Basic_Block_ref > basic_block_set_t;

private:
  // Function
  Function_ref _function;

  // Set of basic blocks in the current cycle
  basic_block_set_t _blocks;

public:
  loop_iterator(Function_ref function) : _function(function), _blocks() {}

  void visit(wto_vertex_t& vertex) { this->_blocks.insert(vertex.node()); }

  void visit(wto_cycle_t& cycle) {
    basic_block_set_t current_blocks = this->_blocks;

    // collect all basic blocks within the cycle
    this->_blocks.clear();
    this->_blocks.insert(cycle.head());

    for (auto it = cycle.begin(); it != cycle.end(); ++it) {
      it->accept(this);
    }

    this->add_loop_counter(cycle, this->_blocks);

    // update _blocks
    this->_blocks.insert(current_blocks.begin(), current_blocks.end());
  }

  void add_loop_counter(wto_cycle_t& cycle, basic_block_set_t& blocks) {
#ifdef DEBUG
    std::cout << "** Cycle {";
    for (auto it = _blocks.begin(); it != _blocks.end(); ++it) {
      if (it != _blocks.begin()) {
        std::cout << ", ";
      }
      std::cout << (*it)->getNameId();
    }
    std::cout << "}" << std::endl;
#endif

    // find a source location
    Source_Location_ref src_loc = first_source_location(cycle.head());

    // create the loop counter variable
    Integer_Type_ref var_type = Integer_Type::create(64, 64, sign_unsigned);
    Internal_Variable_ref var =
        Internal_Variable::create(var_type,
                                  "shadow." + _function->getFunctionId() + "." +
                                      cycle.head()->getNameId() + ".ctn");

    // add initialization statement in parent blocks that aren't in the cycle
    std::size_t blocks_incoming_edge = 0;

    for (auto bb = blocks.begin(); bb != blocks.end(); ++bb) {
      BBRange preds = ar::getPreds(*bb);
      bool has_incoming_edge = false;

      for (auto pred = preds.begin(); pred != preds.end(); ++pred) {
        if (blocks.find(*pred) == blocks.end()) {
          has_incoming_edge = true;
          Statement_ref stmt =
              Assignment::create(var,
                                 Cst_Operand::create(
                                     Integer_Constant::create(var_type, 0)),
                                 src_loc);
          (*pred)->pushBack(stmt);
        }
      }

      if (has_incoming_edge) {
        blocks_incoming_edge++;
      }
    }

    if (blocks_incoming_edge > 1) {
      // Cycle with multiple blocks having incoming edges from outside of
      // the cycle.
      // This pattern is unusual. We cannot know where to put the increment
      // statement, so just abort silently.
      return;
    }

    // add an increment statement in incoming blocks of the head
    // that are in the cycle
    BBRange preds = ar::getPreds(cycle.head());
    for (auto pred = preds.begin(); pred != preds.end(); ++pred) {
      if (blocks.find(*pred) != blocks.end()) {
        Statement_ref stmt =
            Arith_Op::create(add,
                             var,
                             Var_Operand::create(var),
                             Cst_Operand::create(
                                 Integer_Constant::create(var_type, 1)),
                             src_loc);
        (*pred)->pushBack(stmt);
      }
    }
  }

  static Source_Location_ref first_source_location(Basic_Block_ref entry) {
    std::deque< Basic_Block_ref > worklist;
    std::vector< Basic_Block_ref > done;

    // breadth-first search from the entry
    worklist.push_back(entry);

    while (!worklist.empty()) {
      Basic_Block_ref bb = worklist.front();
      worklist.pop_front();

      StmtRange stmts = bb->getStatements();
      for (const Statement_ref& stmt : stmts) {
        if (stmt->getSourceLocation() != Null_ref &&
            stmt->getSourceLocation()->getLineNumber() != -1 &&
            stmt->getSourceLocation()->getColumnNumber() != -1) {
          return stmt->getSourceLocation();
        }
      }

      // could not find any source location, look further
      done.push_back(bb);
      auto succs = bb->getNextBlocks();
      for (auto it = succs.first; it != succs.second; ++it) {
        if (std::find(done.begin(), done.end(), *it) == done.end()) {
          worklist.push_back(*it);
        }
      }
    }

    return Null_ref;
  }

}; // end class loop_iterator

class AddLoopCountersPass : public Pass {
private:
  typedef ikos::wto< Basic_Block_ref, FunctionCFG > wto_t;

public:
  AddLoopCountersPass()
      : Pass("add-loop-counters", "Add a loop counter within every loop") {}

  void execute(Bundle_ref bundle) {
    FuncRange functions = ar::getFunctions(bundle);
    for (auto it = functions.begin(); it != functions.end(); ++it) {
      execute(*it);
    }
  }

  void execute(Function_ref f) {
    // compute the weak topological order
    FunctionCFG cfg(f);
    wto_t wto(cfg);

    // add loop counters in every cycle
    loop_iterator it(f);
    wto.accept(&it);
  }

}; // end class AddLoopCountersPass

} // end namespace passes
} // end namespace arbos

extern "C" arbos::Pass* init() {
  return new arbos::passes::AddLoopCountersPass();
}
