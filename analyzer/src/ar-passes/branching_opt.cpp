/*******************************************************************************
 *
 * Branching optimization pass.
 *
 * This pass tries to optimize the control flow graph of each function. The goal
 * is to make the analysis (using Abstract Interpretation) more precise. It
 * should also make it faster.
 *
 * We want to remove some specific shapes of the control flow graph, such as:
 *
 *                [ ENTRY ]
 *                  /   \
 *  [ assert(i < 10) ] [ assert(i >= 10) ]
 *  [     v0 = 1     ] [     v0 = 0      ]
 *                 \    /
 *                [  M  ]
 *                 /   \
 *  [ assert(cond) ] [ assert(not cond) ]
 *  [    v1 = 1    ] [     v1 = 0       ]
 *                 \    /
 *            [ v = v0 AND v1 ]
 *                 /    \
 *       [ assert(v) ] [ assert(not v) ]
 *             |             |
 *           [ A ]         [ B ]
 *
 * This pass will try to transform the control flow graph so that we get:
 *
 *                [       ENTRY       ]
 *                 /                 \
 *  [ assert(i < 10) ]       [ assert(i >= 10) ]
 *  [     v0 = 1     ]       [      v0 = 0     ]
 *  [  assert(cond)  ]              /    \
 *  [     v1 = 1     ] [ assert(cond) ] [ assert(not cond) ]
 *  [ v = v0 AND v1  ] [    v1 = 1    ] [     v1 = 0       ]
 *  [   assert(v)    ]             \      /
 *          |                   [ v = v0 AND v1 ]
 *        [ A ]                 [ assert(not v) ]
 *                                    |
 *                                  [ B ]
 *
 * Thus avoiding the merging node M.
 *
 * This is done by checking each path with a depth of 2, starting from each
 * possible node in the control flow graph. If we find an unreachable path,
 * depending on the shape of the graph we can remove some edges or clone
 * the merging node.
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

#include <unordered_set>
#include <vector>

#include <boost/functional/hash.hpp>

#include <ikos/domains/intervals.hpp>
#include <ikos/iterators/wto.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/analysis/memory_location.hpp>
#include <analyzer/analysis/num_sym_exec.hpp>
#include <analyzer/ar-wrapper/cfg.hpp>
#include <analyzer/ar-wrapper/transformations.hpp>
#include <analyzer/ar-wrapper/wrapper.hpp>

//#define DEBUG 1

namespace arbos {
namespace passes {

template < typename AbsNumDomain,
           typename VariableName,
           typename MemoryLocation,
           typename Number >
class TransferFunVisitor : public arbos_visitor_api {
  typedef analyzer::num_sym_exec< AbsNumDomain, Number, ikos::dummy_number >
      sym_exec_t;

  sym_exec_t _sym_exec;
  analyzer::VariableFactory& _vfac;
  analyzer::memory_factory& _mfac;

public:
  TransferFunVisitor(AbsNumDomain pre,
                     analyzer::VariableFactory& vfac,
                     analyzer::memory_factory& mfac,
                     analyzer::LiteralFactory& lfac,
                     const std::string& arch)
      : _sym_exec(pre, vfac, mfac, lfac, arch, analyzer::REG),
        _vfac(vfac),
        _mfac(mfac) {}

  AbsNumDomain inv() { return _sym_exec.inv(); }

  void visit_start(Basic_Block_ref b) { _sym_exec.exec_start(b); }
  void visit_end(Basic_Block_ref b) { _sym_exec.exec_end(b); }
  void visit(Arith_Op_ref s) { _sym_exec.exec(s); }
  void visit(Integer_Comparison_ref s) { _sym_exec.exec(s); }
  void visit(FP_Comparison_ref s) { _sym_exec.exec(s); }
  void visit(Bitwise_Op_ref s) { _sym_exec.exec(s); }
  void visit(FP_Op_ref s) { _sym_exec.exec(s); }
  void visit(Abstract_Variable_ref s) { _sym_exec.exec(s); }
  void visit(Unreachable_ref s) { _sym_exec.exec(s); }
  void visit(Assignment_ref s) { _sym_exec.exec(s); }
  void visit(Conv_Op_ref s) { _sym_exec.exec(s); }
  void visit(Pointer_Shift_ref s) { _sym_exec.exec(s); }
  void visit(Allocate_ref s) { _sym_exec.exec(s); }
  void visit(Store_ref s) { _sym_exec.exec(s); }
  void visit(Load_ref s) { _sym_exec.exec(s); }
  void visit(Insert_Element_ref s) { _sym_exec.exec(s); }
  void visit(Extract_Element_ref s) { _sym_exec.exec(s); }
  void visit(MemCpy_ref s) { _sym_exec.exec(s); }
  void visit(MemMove_ref s) { _sym_exec.exec(s); }
  void visit(MemSet_ref s) { _sym_exec.exec(s); }
  void visit(Landing_Pad_ref s) { _sym_exec.exec(s); }
  void visit(Resume_ref s) { _sym_exec.exec(s); }
  void visit(Abstract_Memory_ref s) { _sym_exec.exec(s); }
  void visit(Call_ref s) {
    _sym_exec.exec_unknown_call(ar::getReturnValue(s), ar::getArguments(s));
  }
  void visit(Invoke_ref s) { visit(ar::getFunctionCall(s)); }
  void visit(Return_Value_ref s) { _sym_exec.exec(s); }
  void visit(VA_Start_ref s) { _sym_exec.exec(s); }
  void visit(VA_End_ref s) { _sym_exec.exec(s); }
  void visit(VA_Arg_ref s) { _sym_exec.exec(s); }
  void visit(VA_Copy_ref s) { _sym_exec.exec(s); }
};

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
};

// Iterator on a weak topological order to find the heads of the nested cycles
// The head of a cycle is a node that represents the cycle. It is usually the
// node in the cycle that is reached first by the execution flow.
class mark_heads_wto_iterator
    : public ikos::wto_component_visitor< Basic_Block_ref, FunctionCFG > {
private:
  typedef ikos::wto_vertex< Basic_Block_ref, FunctionCFG > wto_vertex_t;
  typedef ikos::wto_cycle< Basic_Block_ref, FunctionCFG > wto_cycle_t;

private:
  std::unordered_set< index64_t >& _nested_cycles_heads;
  bool _is_nested;

public:
  mark_heads_wto_iterator(std::unordered_set< index64_t >& nested_cycles_heads)
      : _nested_cycles_heads(nested_cycles_heads), _is_nested(false) {}

  void visit(wto_vertex_t& /*vertex*/) {}

  void visit(wto_cycle_t& cycle) {
    if (_is_nested) {
      _nested_cycles_heads.insert(cycle.head().getUID());
    }

    bool parent_is_nested = _is_nested;
    _is_nested = true;

    for (typename wto_cycle_t::iterator it = cycle.begin(); it != cycle.end();
         ++it) {
      it->accept(this);
    }

    _is_nested = parent_is_nested;
  }
};

class BranchingOptPass : public Pass {
private:
  // Use abstract interpretation to find unreachable paths
  typedef analyzer::varname_t VariableName;
  typedef analyzer::memloc_t MemoryLocation;
  typedef ikos::z_number Number;
  typedef ikos::interval_domain< Number, VariableName > AbsNumDomain;
  typedef TransferFunVisitor< AbsNumDomain,
                              VariableName,
                              MemoryLocation,
                              Number >
      transfer_fun_visitor_t;

  typedef std::pair< index64_t, index64_t > edge_t;
  typedef ikos::wto< Basic_Block_ref, FunctionCFG > wto_t;

private:
  analyzer::VariableFactory _vfac;
  analyzer::memory_factory _mfac;
  analyzer::LiteralFactory _lfac;
  std::string _arch;

  // Keep track of already cloned and merged nodes, to avoid infinite loops
  std::unordered_set< index64_t > _cloned;
  std::unordered_set< edge_t, boost::hash< edge_t > > _merged;

  // Do not touch heads of nested cycles
  std::unordered_set< index64_t > _nested_cycles_heads;

public:
  BranchingOptPass()
      : Pass("branching-opt", "Optimize the Control Flow Graph"),
        _vfac(),
        _mfac(),
        _lfac(_vfac) {}

  void execute(Bundle_ref bundle) {
    try {
      _arch = ar::getTargetArch(bundle);
      FuncRange functions = ar::getFunctions(bundle);
      for (FuncRange::iterator it = functions.begin(); it != functions.end();
           ++it) {
        execute(*it);
      }
    } catch (analyzer::analyzer_error& e) {
      std::cerr << "analyzer error: " << e << std::endl;
      exit(EXIT_FAILURE);
    } catch (ikos::exception& e) {
      std::cerr << "ikos error: " << e.what() << std::endl;
      exit(EXIT_FAILURE);
    } catch (arbos::error& e) {
      std::cerr << "arbos error: " << e << std::endl;
      exit(EXIT_FAILURE);
    } catch (std::exception& e) {
      std::cerr << "system error: " << e.what() << std::endl;
      exit(EXIT_FAILURE);
    } catch (...) {
      std::cerr << "unknown error occurred" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  void execute(Function_ref f) {
#ifdef DEBUG
    std::cerr << "* Optimizing control flow graph of " << ar::getName(f)
              << std::endl;
#endif

    // Compute the Weak Topological Order
    FunctionCFG cfg(f);
    wto_t wto(cfg);

#ifdef DEBUG
    std::cerr << "** Weak topological ordering: " << wto << std::endl;
#endif

    // We want to keep the heads of the nested cycles, otherwise we might lose
    // precision during the analysis
    _nested_cycles_heads.clear();
    mark_heads_wto_iterator wto_it(_nested_cycles_heads);
    wto.accept(&wto_it);

#ifdef DEBUG
    std::cerr << "** Heads of nested cycles: [";
    for (auto it = _nested_cycles_heads.begin();
         it != _nested_cycles_heads.end();) {
      std::cerr << ar::getName(Basic_Block_ref(*it));
      ++it;
      if (it != _nested_cycles_heads.end()) {
        std::cerr << ", ";
      }
    }
    std::cerr << "]" << std::endl;
#endif

    bool changed = true;

    while (changed) {
      changed = false;
      BBRange blocks = ar::getBlocks(f);

      for (BBRange::iterator it = blocks.begin(); it != blocks.end(); ++it) {
        changed = changed || cleanup_node(*it);
      }

      if (changed)
        continue;

      for (BBRange::iterator it = blocks.begin(); it != blocks.end(); ++it) {
        changed = changed || prune_edges_from(*it);
      }
    }

    remove_unused_nodes(f);
  }

  bool prune_edges_from(Basic_Block_ref node) {
    if (has_side_effect(node))
      return false;

    BBRange succs = ar::getSuccs(node);
    bool changed = false;

    // Check path node -> child and paths node -> child -> X
    for (BBRange::iterator it = succs.begin(); it != succs.end(); ++it) {
      Basic_Block_ref child = *it;

      if (node == child || has_side_effect(child))
        continue;

      changed = changed || prune_edges_from(node, child);
    }

    if (changed) {
      cleanup_node(node);
    }

    return changed;
  }

  bool prune_edges_from(Basic_Block_ref node, Basic_Block_ref child) {
    // Check if node -> child is reachable
    std::shared_ptr< transfer_fun_visitor_t > vis =
        std::make_shared< transfer_fun_visitor_t >(AbsNumDomain::top(),
                                                   _vfac,
                                                   _mfac,
                                                   _lfac,
                                                   _arch);

    ar::accept(node, vis);
    ar::accept(child, vis);
    AbsNumDomain inv = vis->inv();

    if (inv.is_bottom()) {
#ifdef DEBUG
      std::cerr << "** Path " << ar::getName(node) << " -> "
                << ar::getName(child) << " unreachable. Removing edge."
                << std::endl;
#endif

      // Remove edge between node and child
      node->removeNextBasicBlock(child);

      cleanup_node(child);
      return true;
    }

    // Check one more depth ahead
    BBRange succs = ar::getSuccs(child);

    if (succs.empty()) {
      return false;
    }

    std::vector< bool > path_reachable(succs.size(), true);
    bool has_unreachable_path = false;
    bool all_unreachable_paths = true;

    for (unsigned int i = 0; i < succs.size(); i++) {
      // Check path node -> child -> succ
      Basic_Block_ref succ = succs[i];

      if (child == succ) {
        return false;
      }
      if (node == succ || has_side_effect(succ)) {
        all_unreachable_paths = false;
        continue;
      }

      vis = std::make_shared< transfer_fun_visitor_t >(inv,
                                                       _vfac,
                                                       _mfac,
                                                       _lfac,
                                                       _arch);
      ar::accept(succ, vis);

      if (vis->inv().is_bottom()) {
#ifdef DEBUG
        std::cerr << "*** Path " << ar::getName(node) << " -> "
                  << ar::getName(child) << " -> " << ar::getName(succ)
                  << " unreachable" << std::endl;
#endif

        path_reachable[i] = false;
        has_unreachable_path = true;
      } else {
        all_unreachable_paths = false;
      }
    }

    if (all_unreachable_paths) {
#ifdef DEBUG
      std::cerr << "** Path " << ar::getName(node) << " -> "
                << ar::getName(child) << " unnecessary because all paths "
                << ar::getName(node) << " -> " << ar::getName(child)
                << " -> * are unreachable. Removing edge." << std::endl;
#endif

      // Remove edge between node and child
      node->removeNextBasicBlock(child);

      cleanup_node(child);
      return true;
    } else if (has_unreachable_path) {
      if (ar::getPreds(child).size() == 1) {
        // Remove the edges between child and succs[i] if the path is
        // unreachable
        for (unsigned int i = 0; i < succs.size(); i++) {
          if (!path_reachable[i]) {
#ifdef DEBUG
            std::cerr << "** Path " << ar::getName(child) << " -> "
                      << ar::getName(succs[i]) << " unnecessary because path "
                      << ar::getName(node) << " -> " << ar::getName(child)
                      << " -> " << ar::getName(succs[i])
                      << " is unreachable. Removing edge." << std::endl;
#endif

            child->removeNextBasicBlock(succs[i]);

            cleanup_node(succs[i]);
          }
        }
      } else { // ar::getPreds(child).size() > 1
        // Check if this node has already been cloned
        if (_cloned.find(child->getUID()) != _cloned.end()) {
          return false;
        }

        // Do not clone heads of nested cycles
        if (_nested_cycles_heads.find(child->getUID()) !=
            _nested_cycles_heads.end()) {
          return false;
        }

#ifdef DEBUG
        std::cerr << "** Clone node " << ar::getName(child)
                  << " because some paths " << ar::getName(node) << " -> "
                  << ar::getName(child) << " -> X are unreachable."
                  << std::endl;
#endif

        // Create a new basic block
        std::string clone_name = next_available_node_name(child);
        Basic_Block_ref clone_child = AR_Basic_Block::create(clone_name);
        child->getContainingCode()->addBasicBlock(clone_child);

        // Mark the nodes as cloned
        _cloned.insert(child->getUID());
        _cloned.insert(clone_child->getUID());

        // Add all statements from child to clone_child
        StmtRange stmts = ar::getStatements(child);
        for (StmtRange::iterator it = stmts.begin(); it != stmts.end(); ++it) {
          transformations::addBackStatement(clone_child, (*it)->clone());
        }

        // Remove the edge from node to child
        node->removeNextBasicBlock(child);

        // Add an edge from node to clone_child
        node->addNextBasicBlock(clone_child);

        // Create the necessary edges from clone_child to succs
        for (unsigned int i = 0; i < succs.size(); i++) {
          if (path_reachable[i]) {
            clone_child->addNextBasicBlock(succs[i]);
          }
        }
      }

      cleanup_node(child);
      return true;
    }

    return false;
  }

  // Check if a node can be merged with its (only) child
  bool is_mergeable_with_child(Basic_Block_ref node) {
    BBRange succs = ar::getSuccs(node);

    if (succs.size() != 1)
      return false;

    Basic_Block_ref child = succs[0];
    succs = ar::getSuccs(child);

    // check that there is no loop
    return child != node && !child->isNextBlock(node) &&
           !child->isNextBlock(child) &&
           // check that it's not an entry/exit/unreachable/ehresume block
           node->getContainingCode()->getEntryBlock() != child &&
           node->getContainingCode()->getExitBlock() != child &&
           node->getContainingCode()->getUnreachableBlock() != child &&
           node->getContainingCode()->getEHResumeBlock() != child &&
           // check that it has not already been merged
           _merged.find(edge_t(node->getUID(), child->getUID())) ==
               _merged.end() &&
           // check that the child is not the head of a nested cycle
           _nested_cycles_heads.find(child->getUID()) ==
               _nested_cycles_heads.end() &&
           // prevent the entry block to have an edge to a head of a nested
           // cycle
           !(node->getContainingCode()->getEntryBlock() == node &&
             std::any_of(succs.begin(),
                         succs.end(),
                         [&](Basic_Block_ref b) {
                           return _nested_cycles_heads.find(b->getUID()) !=
                                  _nested_cycles_heads.end();
                         }));
  }

  void merge_with_child(Basic_Block_ref node) {
    BBRange succs = ar::getSuccs(node);
    Basic_Block_ref child = succs[0];

#ifdef DEBUG
    std::cerr << "** Merge " << ar::getName(node) << " with its child "
              << ar::getName(child) << std::endl;
#endif

    // Mark the nodes as merged
    _merged.insert(edge_t(node->getUID(), child->getUID()));

    // Move the statements from the child to the parent
    StmtRange stmts = ar::getStatements(child);
    for (StmtRange::iterator it = stmts.begin(); it != stmts.end(); ++it) {
      transformations::addBackStatement(node, (*it)->clone());
    }

    // Remove the edge between node and child
    node->removeNextBasicBlock(child);

    // Create the edges from node
    succs = ar::getSuccs(child);
    for (BBRange::iterator it = succs.begin(); it != succs.end(); ++it) {
      node->addNextBasicBlock(*it);
    }

    if (ar::getPreds(child).empty()) {
      remove_edges(child);
      // The child will be removed later in remove_unused_nodes()
    }
  }

  static void remove_edges(Basic_Block_ref node) {
#ifdef DEBUG
    std::cerr << "** Remove edges of " << ar::getName(node) << std::endl;
#endif

    BBRange preds = ar::getPreds(node);
    BBRange succs = ar::getSuccs(node);

    for (BBRange::iterator it = preds.begin(); it != preds.end(); ++it) {
      node->removePreviousBasicBlock(*it);
    }
    for (BBRange::iterator it = succs.begin(); it != succs.end(); ++it) {
      node->removeNextBasicBlock(*it);
    }
  }

  // Try to merge a node with its child and/or remove an unnecessary node
  bool cleanup_node(Basic_Block_ref node) {
    bool changed = false;

    while (is_mergeable_with_child(node) && !has_side_effect(node) &&
           !has_side_effect(ar::getSuccs(node)[0])) {
      merge_with_child(node);
      changed = true;
    }

    if (ar::getPreds(node).empty() && !ar::getSuccs(node).empty() &&
        node->getContainingCode()->getEntryBlock() != node) {
      remove_edges(node);
      changed = true;
    }

    return changed;
  }

  static void remove_unused_nodes(Function_ref f) {
    BBRange blocks = ar::getBlocks(f);

    for (BBRange::iterator it = blocks.begin(); it != blocks.end(); ++it) {
      if (ar::getPreds(*it).empty() && ar::getSuccs(*it).empty() &&
          f->getFunctionBody()->getEntryBlock() != *it) {
#ifdef DEBUG
        std::cerr << "** Remove node " << ar::getName(*it) << std::endl;
#endif
        f->getFunctionBody()->removeBasicBlock(*it);
      }
    }
  }

  static bool has_side_effect(Basic_Block_ref node) {
    StmtRange stmts = ar::getStatements(node);

    for (StmtRange::iterator it = stmts.begin(); it != stmts.end(); ++it) {
      if (!(ar::is_int_arith_stmt(*it) || ar::is_float_arith_stmt(*it) ||
            ar::is_int_cmp_stmt(*it) || ar::is_float_cmp_stmt(*it) ||
            ar::is_assignment_stmt(*it) || ar::is_conv_stmt(*it) ||
            ar::is_bitwise_stmt(*it) || ar::is_nop_stmt(*it) ||
            ar::is_pointer_shift_stmt(*it) || ar::is_allocate_stmt(*it))) {
        return true;
      }

      // consider that divisions have side effects (because they can raise a
      // runtime error)
      if (ar::is_int_arith_stmt(*it) &&
          (ar::getArithOp(node_cast< Arith_Op >(*it)) == udiv ||
           ar::getArithOp(node_cast< Arith_Op >(*it)) == sdiv)) {
        return true;
      }
    }

    return false;
  }

  static std::string next_available_node_name(Basic_Block_ref node) {
    BBRange blocks = ar::getBlocks(ar::getParent(node));

    for (std::size_t i = 2;; i++) {
      std::string next_name = ar::getName(node) + "_" + std::to_string(i);
      bool found = false;

      for (BBRange::iterator it = blocks.begin(); it != blocks.end(); ++it) {
        if (ar::getName(*it) == next_name) {
          found = true;
          break;
        }
      }

      if (!found) {
        return next_name;
      }
    }
  }

}; // end class BranchingOptPass

} // end namespace passes
} // end namespace arbos

extern "C" arbos::Pass* init() {
  return new arbos::passes::BranchingOptPass();
}
