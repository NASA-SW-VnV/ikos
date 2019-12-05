/*******************************************************************************
 *
 * \file
 * \brief Construction and management of weak partial orderings (WPOs).
 *
 * The construction of weak partial orderings is based on Sung Kook Kim's,
 * Arnaud J. Venet's, and Aditya V. Thakur's paper: "Deterministic Parallel
 * Fixpoint Computation", in POPL 2020.
 *
 * Author: Sung Kook Kim
 *
 * Contributor: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2019 United States Government as represented by the
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

#pragma once

#include <memory>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/pending/disjoint_sets.hpp>

#include <ikos/core/semantic/dumpable.hpp>
#include <ikos/core/semantic/graph.hpp>
#include <ikos/core/support/assert.hpp>

namespace ikos {
namespace core {

template < typename GraphRef, typename GraphTrait >
class Wpo;

template < typename GraphRef, typename GraphTrait >
class WpoNode;

namespace wpo_impl {

template < typename GraphRef, typename GraphTrait >
class WpoBuilder;

} // end namespace wpo_impl

/// \brief Node of a weak partial order
///
/// This is either head, plain, or exit.
template < typename GraphRef, typename GraphTrait = GraphTraits< GraphRef > >
class WpoNode final {
public:
  /// \brief Kind of the node
  enum class Kind { Plain, Head, Exit };

private:
  using NodeRef = typename GraphTrait::NodeRef;
  using WpoIndex = std::size_t;

private:
  /// \brief Graph node
  NodeRef _node;

  /// \brief Kind of node
  Kind _kind;

  /// \brief Successors of forward blue arrow
  std::vector< WpoIndex > _successors;

  /// \brief Predecessors of forward blue arrow
  std::vector< WpoIndex > _predecessors;

  /// \brief Sucessors of 'lifted' forward arrow
  /// This is meant to be used to construct the WTO
  std::vector< WpoIndex > _successors_lifted;

  /// \brief Backward arrow
  WpoIndex _head_or_exit;

  /// \brief Number of predecessors
  std::size_t _num_predecessors;

  /// \brief Number of reducible predecessors
  std::size_t _num_predecessors_reducible;

  /// \brief Postorder DFN (used to calculate priority of a node in scheduling)
  std::size_t _post_order;

  /// \brief Irreducible forward arrows for this exit's component (for exits
  /// only)
  std::unordered_map< WpoIndex, std::size_t > _irreducibles;

  /// \brief Size of the component
  std::size_t _size;

public:
  /// \brief Constructor
  WpoNode(NodeRef node, Kind kind, std::size_t size, std::size_t post_order)
      : _node(node),
        _kind(kind),
        _num_predecessors(0),
        _num_predecessors_reducible(0),
        _post_order(post_order),
        _size(size) {}

  /// \brief No copy constructor
  WpoNode(const WpoNode&) = delete;

  /// \brief Move constructor
  WpoNode(WpoNode&&) = default;

  /// \brief No copy assignment operator
  WpoNode& operator=(const WpoNode&) = delete;

  /// \brief No move assignment operator
  WpoNode& operator=(WpoNode&&) = delete;

  /// \brief Return the graph node
  NodeRef node() const { return this->_node; }

  /// \brief Return the kind of node
  Kind kind() const { return this->_kind; }

  /// \brief Check the kind of this node
  bool is_plain() const { return this->_kind == Kind::Plain; }
  bool is_head() const { return this->_kind == Kind::Head; }
  bool is_exit() const { return this->_kind == Kind::Exit; }

  /// \brief Return the successors
  const std::vector< WpoIndex >& successors() const {
    return this->_successors;
  }

  /// \brief Return the predecessors
  const std::vector< WpoIndex >& predecessors() const {
    return this->_predecessors;
  }

  /// \brief Return the successors of lifted forward arrows
  const std::vector< WpoIndex >& successors_lifted() const {
    return this->_successors_lifted;
  }

  /// \brief Return the irreducible forward arrows for this exit's component
  const std::unordered_map< WpoIndex, std::size_t >& irreducibles() const {
    ikos_assert_msg(this->_kind == Kind::Exit,
                    "trying to get irreducibles from non-exit");
    return this->_irreducibles;
  }

  /// \brief Return the size of the component
  std::size_t size() const { return this->_size; }

  /// \brief Return the number of predecessors
  std::size_t num_predecessors() const { return this->_num_predecessors; }

  /// \brief Return the number of reducible predecessors
  std::size_t num_predecessors_reducible() const {
    return this->_num_predecessors_reducible;
  }

  /// \brief Return the post order
  std::size_t post_order() const { return this->_post_order; }

  /// \brief Return the head of the exit node
  WpoIndex head() const {
    ikos_assert_msg(this->_kind == Kind::Exit,
                    "trying to get head from non-exit");
    return this->_head_or_exit;
  }

  /// \brief Return the exit of the head node
  WpoIndex exit() const {
    ikos_assert_msg(this->_kind == Kind::Head,
                    "trying to get exit from non-head");
    return this->_head_or_exit;
  }

private:
  /// \brief Add a successor
  void add_successor(WpoIndex idx) { this->_successors.push_back(idx); }

  /// \brief Add a predecessor
  void add_predecessor(WpoIndex idx) { this->_predecessors.push_back(idx); }

  /// \brief Check if the given node is a successor
  bool is_successor(WpoIndex idx) const {
    return std::find(this->_successors.begin(), this->_successors.end(), idx) !=
           this->_successors.end();
  }

  /// \brief Add a successor of lifted forward arrow
  void add_successor_lifted(WpoIndex idx) {
    this->_successors_lifted.push_back(idx);
  }

  /// \brief Check if the given node is a successor of lifted forward arrow
  bool is_successor_lifted(WpoIndex idx) const {
    return std::find(this->_successor_lifted.begin(),
                     this->_successor_lifted.end(),
                     idx) != this->_successor_lifted.end();
  }

  /// \brief Increment the number of irreducibles that is directing to idx
  void inc_irreducible(WpoIndex idx) {
    ikos_assert_msg(this->_kind == Kind::Exit,
                    "trying to access irreducibles from non-exit");
    this->_irreducibles[idx]++;
  }

  /// \brief Increment the number of predecessors
  void inc_num_predecessors() { this->_num_predecessors++; }

  /// \brief Increment the number of reducible predecessors
  void inc_num_predecessors_reducible() { this->_num_predecessors_reducible++; }

  /// \brief Set head or an exit
  void set_head_exit(WpoIndex idx) {
    ikos_assert_msg(this->_kind != Kind::Plain,
                    "trying to access head_or_exit from plain");
    this->_head_or_exit = idx;
  }

public:
  /// \brief Dump the node, for debugging purpose
  void dump(std::ostream& o, WpoIndex idx) const {
    o << "WpoNode:\n";
    o << "  index: " << idx << "\n";
    DumpableTraits< NodeRef >::dump(o, this->_node);
    o << "\n";
    switch (this->_kind) {
      case Kind::Plain: {
        o << "  kind: plain\n";
        break;
      }
      case Kind::Head: {
        o << "  kind: head\n";
        o << "  exit: " << this->_head_or_exit << "\n";
        break;
      }
      case Kind::Exit: {
        o << "  kind: exit\n";
        o << "  head: " << this->_head_or_exit << "\n";
        break;
      }
    }
    o << "  successors: ";
    for (auto succ : this->_successors) {
      o << succ << " ";
    }
    o << "\n";
    o << "  successors_lifted: ";
    for (auto succ : this->_successors_lifted) {
      o << succ << " ";
    }
    o << "\n";
    o << "  predecessors: ";
    for (auto pred : this->_predecessors) {
      o << pred << " ";
    }
    o << "\n";
    o << "  number of predecessors: " << this->_num_predecessors << "\n";
    o << "  number of reducible predecessors: "
      << this->_num_predecessors_reducible << "\n";
    o << "  post order: " << this->_post_order << "\n";
    o << "  irreducibles: ";
    for (const auto& p : this->_irreducibles) {
      o << p.first << "," << p.second << " ";
    }
    o << "\n";
    o << "  size: " << this->_size << "\n";
  }

public:
  template < typename T1, typename T2 >
  friend class wpo_impl::WpoBuilder;

}; // end class WpoNode

/// \brief Weak Partial Ordering
template < typename GraphRef, typename GraphTrait = GraphTraits< GraphRef > >
class Wpo {
public:
  static_assert(IsGraph< GraphRef, GraphTrait >::value,
                "GraphRef does not implement GraphTraits");

private:
  using NodeRef = typename GraphTrait::NodeRef;
  using WpoNodeT = WpoNode< GraphRef, GraphTrait >;
  using Kind = typename WpoNodeT::Kind;
  using WpoIndex = std::size_t;

private:
  // WPO nodes
  std::vector< WpoNodeT > _wpo_nodes;

  // Back predecessors
  std::unordered_map< NodeRef, std::unordered_set< NodeRef > >
      _back_predecessors;

public:
  /// \brief Compute the weak partial order of the given graph
  explicit Wpo(GraphRef cfg) {
    NodeRef root = GraphTrait::entry(cfg);

    if (GraphTrait::successor_begin(root) == GraphTrait::successor_end(root)) {
      this->_wpo_nodes.emplace_back(root, Kind::Plain, 1, 1);
      return;
    }

    wpo_impl::WpoBuilder< GraphRef, GraphTrait >
        builder(cfg, this->_wpo_nodes, this->_back_predecessors);
  }

  /// \brief No copy constructor
  Wpo(const Wpo& other) = delete;

  /// \brief No move constructor
  Wpo(Wpo&& other) = delete;

  /// \brief No copy assignment operator
  Wpo& operator=(const Wpo& other) = delete;

  /// \brief No move assignment operator
  Wpo& operator=(Wpo&& other) = delete;

  /// \brief Return the total number of nodes
  std::size_t size() const { return this->_wpo_nodes.size(); }

  /// \brief Return the entry node index
  WpoIndex entry() { return this->_wpo_nodes.size() - 1; }

  /// \brief Return the successors of a given node index
  const std::vector< WpoIndex >& successors(WpoIndex idx) const {
    return this->_wpo_nodes[idx].successors();
  }

  /// \brief Return the predecessors of a given node index
  const std::vector< WpoIndex >& predecessors(WpoIndex idx) const {
    return this->_wpo_nodes[idx].predecessors();
  }

  /// \brief Return the number of predecessors of a given node index
  std::size_t num_predecessors(WpoIndex idx) const {
    return this->_wpo_nodes[idx].num_predecessors();
  }

  /// \brief Return the number of reducible predecessors of a given node index
  std::size_t num_predecessors_reducible(WpoIndex idx) const {
    return this->_wpo_nodes[idx].num_predecessors_reducible();
  }

  /// \brief Return the post order of a given node index
  std::size_t post_order(WpoIndex idx) const {
    return this->_wpo_nodes[idx].post_order();
  }

  /// \brief Return the irreducibles for the exit's component
  const std::unordered_map< WpoIndex, std::size_t >& irreducibles(
      WpoIndex exit) const {
    return this->_wpo_nodes[exit].irreducibles();
  }

  /// \brief Return the head of the given exit node index
  WpoIndex head_of_exit(WpoIndex exit) const { return exit + 1; }

  /// \brief Return the exit of the given head node index
  WpoIndex exit_of_head(WpoIndex head) const { return head - 1; }

  /// \brief Return the node for the given index
  NodeRef node(WpoIndex idx) const { return this->_wpo_nodes[idx].node(); }

  /// \brief Return the kind of the given index
  Kind kind(WpoIndex idx) const { return this->_wpo_nodes[idx].kind(); }

  /// \brief Check the kind of this given node index
  bool is_plain(WpoIndex idx) const { return this->_wpo_nodes[idx].is_plain(); }
  bool is_head(WpoIndex idx) const { return this->_wpo_nodes[idx].is_head(); }
  bool is_exit(WpoIndex idx) const { return this->_wpo_nodes[idx].is_exit(); }

  /// \brief Checks whether a given edge is a backedge
  bool is_back_edge(NodeRef head, NodeRef pred) const {
    auto it = this->_back_predecessors.find(head);
    if (it == this->_back_predecessors.end()) {
      return false;
    } else {
      const auto& preds = it->second;
      return preds.find(pred) != preds.end();
    }
  }

  void dump(std::ostream& o) const {
    for (std::size_t idx = 0; idx < this->_wpo_nodes.size(); idx++) {
      o << "# ";
      this->_wpo_nodes[idx].dump(o, idx);
    }
  }

}; // end class Wpo

namespace wpo_impl {

template < typename GraphRef, typename GraphTrait = GraphTraits< GraphRef > >
class WpoBuilder {
private:
  using NodeRef = typename GraphTrait::NodeRef;
  using WpoNodeT = WpoNode< GraphRef, GraphTrait >;
  using WpoT = Wpo< GraphRef, GraphTrait >;
  using Kind = typename WpoNodeT::Kind;
  using WpoIndex = std::size_t;

  struct Edge {
    WpoIndex from;
    WpoIndex to;
  };

private:
  /// \brief Reference to the WPO nodes
  std::vector< WpoNodeT >& _wpo_nodes;

  /// \brief Reference to a map that contains back edges
  std::unordered_map< NodeRef, std::unordered_set< NodeRef > >&
      _back_predecessors;

  /// \brief Map a node to its DFN
  std::unordered_map< NodeRef, std::size_t > _node_to_dfn;

  /// \brief Map a node to its post DFN
  std::unordered_map< NodeRef, std::size_t > _node_to_post_dfn;

  /// \brief Map a DFN to a node
  std::vector< NodeRef > _dfn_to_node;

  /// \brief Maps DFN to DFNs of its back-edge predecessors
  std::vector< std::vector< std::size_t > > _back_predecessors_dfn;

  /// \brief Maps DFN to DFNs of its non-back-edge predecessors
  std::vector< std::vector< std::size_t > > _non_back_predecessors_dfn;

  /// \brief Maps DFN to cross/forward edges (DFN is the lowest common ancestor)
  std::unordered_map< std::size_t, std::vector< Edge > > _cross_forward_edges;

  /// \brief Next DFN
  std::size_t _next_dfn;

  /// \brief Next post DFN
  std::size_t _next_post_dfn;

  /// \brief Next node index
  std::size_t _next_idx;

  /// \brief Map DFN to index
  std::vector< std::size_t > _dfn_to_index;

public:
  /// \brief Constructor
  WpoBuilder(GraphRef cfg,
             std::vector< WpoNodeT >& wpo_nodes,
             std::unordered_map< NodeRef, std::unordered_set< NodeRef > >&
                 back_predecessors)
      : _wpo_nodes(wpo_nodes),
        _back_predecessors(back_predecessors),
        _next_dfn(1),
        _next_post_dfn(1),
        _next_idx(0) {
    this->construct_auxilary(cfg);
    this->construct_wpo();
  }

private:
  /// \brief Construct auxilary data-structures
  ///
  /// Performs DFS iteratively to classify the edges and find the lowest
  /// common ancestors of cross/forward edges.
  /// Nodes are identifed by their DFNs afterwards.
  void construct_auxilary(GraphRef cfg) {
    using RankMap = std::unordered_map< std::size_t, std::size_t >;
    using RankPropertyMap = boost::associative_property_map< RankMap >;
    using ParentMap = std::unordered_map< std::size_t, std::size_t >;
    using ParentPropertyMap = boost::associative_property_map< ParentMap >;

    struct Tuple {
      NodeRef node;
      bool finished;
      std::size_t pred_dfn;
    };

    std::stack< Tuple > stack;
    std::vector< bool > black;
    std::vector< std::size_t > ancestor;
    RankMap rank_map;
    ParentMap parent_map;
    RankPropertyMap rank_property_map(rank_map);
    ParentPropertyMap parent_property_map(parent_map);
    boost::disjoint_sets< RankPropertyMap, ParentPropertyMap >
        dsets(rank_property_map, parent_property_map);

    stack.push(Tuple{/* node = */ GraphTrait::entry(cfg),
                     /* finished = */ false,
                     /* pred = */ 0});

    while (!stack.empty()) {
      NodeRef node = stack.top().node;
      bool finished = stack.top().finished;
      std::size_t pred_dfn = stack.top().pred_dfn;
      stack.pop();

      if (finished) {
        // DFS is done for this node

        // Update the post DFN
        this->_node_to_post_dfn[node] = this->_next_post_dfn++;

        // Mark as visited
        std::size_t dfn = this->node_to_dfn(node);
        black[dfn] = true;

        dsets.union_set(dfn, pred_dfn);
        ancestor[dsets.find_set(pred_dfn)] = pred_dfn;
      } else if (this->node_to_dfn(node) != 0) {
        // Forward edge, can be ignored
        continue;
      } else {
        // Unvisited node
        std::size_t dfn = this->_next_dfn++;
        this->_dfn_to_node.push_back(node);
        this->_node_to_dfn[node] = dfn;

        black.resize(this->_next_dfn);
        this->_back_predecessors_dfn.resize(this->_next_dfn);
        this->_non_back_predecessors_dfn.resize(this->_next_dfn);

        // Lowest common ancestor
        dsets.make_set(dfn);
        ancestor.resize(this->_next_dfn);
        ancestor[dfn] = dfn;

        // This will be popped after its successors are visited
        stack.push(Tuple{/* node = */ node,
                         /* finished = */ true,
                         /* pred = */ pred_dfn});

        // Successors are visited in reverse order to match the WTO
        for (auto it = GraphTrait::successor_end(node),
                  et = GraphTrait::successor_begin(node);
             it != et;) {
          --it;
          NodeRef succ = *it;
          std::size_t succ_dfn = this->node_to_dfn(succ);
          if (succ_dfn == 0) {
            // Unvisited
            stack.push(Tuple{/* node = */ succ,
                             /* finished = */ false,
                             /* pred = */ dfn});
          } else if (black[succ_dfn]) {
            // Cross edge
            auto lca = ancestor[dsets.find_set(succ_dfn)];
            this->_cross_forward_edges[lca].push_back(Edge{dfn, succ_dfn});
          } else {
            // Back edge
            this->_back_predecessors_dfn[succ_dfn].push_back(dfn);
            this->_back_predecessors[succ].insert(node);
          }
        }

        if (pred_dfn != 0) {
          // Tree edge
          this->_non_back_predecessors_dfn[dfn].push_back(pred_dfn);
        }
      }
    }
  }

  void construct_wpo() {
    std::vector< std::size_t > rank(this->_next_dfn);
    std::vector< std::size_t > parent(this->_next_dfn);

    // A partition of vertices. Each subset is known to be strongly connected
    boost::disjoint_sets< std::size_t*, std::size_t* > dsets(&rank[0],
                                                             &parent[0]);

    // Maps representative of a set to the vertex with minimum DFN
    std::vector< std::size_t > rep(this->_next_dfn);

    // Maps a head to its exit
    std::vector< std::size_t > exit(this->_next_dfn);

    // Maps a head to its size of components
    std::vector< std::size_t > size(this->_next_dfn);

    // Maps a vertex to original non-back edges that now target the vertex
    std::vector< std::vector< Edge > > origin(this->_next_dfn);

    // Maps DFN to index
    this->_dfn_to_index.resize(2 * this->_next_dfn);

    std::size_t dfn = this->_next_dfn;

    // Initialization
    for (std::size_t v = 1; v < this->_next_dfn; v++) {
      dsets.make_set(v);
      rep[v] = v;
      exit[v] = v;
      for (std::size_t u : this->_non_back_predecessors_dfn[v]) {
        origin[v].push_back(Edge{u, v});
      }
    }

    // In reverse DFS order, build WPOs for SCCs bottom-up
    for (std::size_t h = this->_next_dfn - 1; h > 0; h--) {
      // Restore cross/fwd edges which has h as the LCA
      auto it = this->_cross_forward_edges.find(h);
      if (it != this->_cross_forward_edges.end()) {
        for (const Edge& edge : it->second) {
          std::size_t rep_to = rep[dsets.find_set(edge.to)];
          this->_non_back_predecessors_dfn[rep_to].push_back(edge.from);
          origin[rep_to].push_back(edge);
        }
      }

      // Find nested SCCs
      bool is_scc = false;
      std::unordered_set< std::size_t > exits_h;
      for (std::size_t v : this->_back_predecessors_dfn[h]) {
        if (v != h) {
          exits_h.insert(rep[dsets.find_set(v)]);
        } else {
          // Self-loop
          is_scc = true;
        }
      }
      if (!exits_h.empty()) {
        is_scc = true;
      }

      // Invariant: h \notin exits_h
      std::unordered_set< std::size_t > components_h(exits_h);
      std::vector< std::size_t > worklist_h(exits_h.begin(), exits_h.end());

      // Find components
      while (!worklist_h.empty()) {
        std::size_t v = worklist_h.back();
        worklist_h.pop_back();
        for (std::size_t u : this->_non_back_predecessors_dfn[v]) {
          std::size_t rep_u = rep[dsets.find_set(u)];
          if (components_h.find(rep_u) == components_h.end() && rep_u != h) {
            components_h.insert(rep_u);
            worklist_h.push_back(rep_u);
          }
        }
      }

      // Invariant: h \notin components_h

      // Found components & exits
      if (!is_scc) {
        size[h] = 1;
        this->add_wpo_node(h, this->dfn_to_node(h), Kind::Plain, /*size=*/1);
        continue;
      }

      // Invariant: wpo_nodes = ...::h
      // End

      // Size of this component is initialized to 2: head and exit.
      std::size_t size_h = 2;
      for (std::size_t v : components_h) {
        size_h += size[v];
      }
      size[h] = size_h;

      // Invariant: size_h = size[h] = number of all nodes in C_h.

      // New exit is added
      std::size_t x = dfn++;
      this->add_wpo_node(x, this->dfn_to_node(h), Kind::Exit, size_h);
      this->add_wpo_node(h, this->dfn_to_node(h), Kind::Head, size_h);
      this->set_head_exit(h, x);
      // Invariant: wpo_nodes = ...::x::h

      if (exits_h.empty()) {
        this->add_successor(/* from = */ h,
                            /* to = */ x,
                            /* exit = */ x,
                            /* irreducible = */ false);
      } else {
        for (std::size_t xx : exits_h) {
          this->add_successor(/* from = */ exit[xx],
                              /* to = */ x,
                              /* exit = */ x,
                              /* irreducible = */ false);
        }
      }
      // Invariant: Forward arrows to x are all constructed

      // Add forward arrow
      for (std::size_t v : components_h) {
        for (const Edge& edge : origin[v]) {
          std::size_t u = edge.from;
          std::size_t vv = edge.to;
          std::size_t x_u = exit[rep[dsets.find_set(u)]];
          std::size_t x_v = exit[v];
          this->add_successor(/* from = */ x_u,
                              /* to = */ vv,
                              /* exit = */ x_v,
                              /* irreducible = */ v != vv);

          // Invariant: u \cfgarrow vv, u \notin C_v, vv \in C_v, u,v \in C_h
          // Invariant: x_u \forwardarrow vv.
        }
      }

      for (std::size_t v : components_h) {
        dsets.union_set(v, h);
        rep[dsets.find_set(v)] = h;
      }

      exit[h] = x;
      // Invariant: exit[h] = h if C_h is trivial SCC, x_h if C_h is non-trivial
      // SCC
    }

    std::vector< std::size_t > top_levels;

    // Top level components
    for (std::size_t v = 1; v < this->_next_dfn; v++) {
      if (rep[dsets.find_set(v)] == v) {
        top_levels.push_back(v);
        for (const Edge& edge : origin[v]) {
          std::size_t u = edge.from;
          std::size_t vv = edge.to;
          std::size_t x_u = exit[rep[dsets.find_set(u)]];
          std::size_t x_v = exit[v];

          this->add_successor(/* from = */ x_u,
                              /* to = */ vv,
                              /* exit = */ x_v,
                              /* irreducible = */ v != vv);
        }
      }
    }
  }

  /// \brief Return the DFN for a given node
  std::size_t node_to_dfn(NodeRef n) const {
    auto it = this->_node_to_dfn.find(n);
    if (it != this->_node_to_dfn.end()) {
      return it->second;
    } else {
      return 0;
    }
  }

  /// \brief Return the node for a given DFN
  const NodeRef& dfn_to_node(std::size_t dfn) const {
    return this->_dfn_to_node.at(dfn - 1);
  }

  /// \brief Add a weak partial order node
  void add_wpo_node(std::size_t dfn,
                    NodeRef node,
                    Kind kind,
                    std::size_t size) {
    this->_dfn_to_index[dfn] = this->_next_idx++;
    this->_wpo_nodes.emplace_back(node,
                                  kind,
                                  size,
                                  this->_node_to_post_dfn[node]);
  }

  /// \brief Return the WPO node from a DFN
  WpoNodeT& dfn_to_wpo_node(std::size_t dfn) {
    return this->_wpo_nodes[this->_dfn_to_index[dfn]];
  }

  /// \brief Return the WPO node index from a DFN
  WpoIndex dfn_to_index(std::size_t dfn) { return this->_dfn_to_index[dfn]; }

  /// \brief Set the head and exit of a WPO node
  void set_head_exit(std::size_t h, std::size_t x) {
    auto idx = this->dfn_to_index(h);
    this->dfn_to_wpo_node(x).set_head_exit(idx);
    this->dfn_to_wpo_node(h).set_head_exit(idx - 1);
  }

  /// \brief Add a successor to a weak partial order node
  void add_successor(std::size_t from_dfn,
                     std::size_t to_dfn,
                     std::size_t exit_dfn,
                     bool irreducible) {
    WpoNodeT& from_node = this->dfn_to_wpo_node(from_dfn);
    WpoNodeT& to_node = this->dfn_to_wpo_node(to_dfn);
    std::size_t to_idx = this->dfn_to_index(to_dfn);

    if (from_node.is_successor(to_idx)) {
      return;
    }

    from_node.add_successor(to_idx);
    to_node.inc_num_predecessors();
    to_node.add_predecessor(this->dfn_to_index(from_dfn));

    if (irreducible) {
      // Irreducible arrow of exit's component
      this->dfn_to_wpo_node(exit_dfn).inc_irreducible(to_idx);
    } else {
      to_node.inc_num_predecessors_reducible();
    }
  }

}; // end class WpoBuilder

} // end namespace wpo_impl

} // end namespace core
} // end namespace ikos
