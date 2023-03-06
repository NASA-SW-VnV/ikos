/*******************************************************************************
 *
 * \file
 * \brief Interleaved concurrent forward fixpoint iterator
 *
 * The interleaved concurrent fixpoint iterator is described in Sung Kook Kim's,
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

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <tbb/cache_aligned_allocator.h>
#include <tbb/parallel_for_each.h>

#include <ikos/core/fixpoint/fixpoint_iterator.hpp>
#include <ikos/core/fixpoint/wpo.hpp>

namespace ikos {
namespace core {

/// \brief Interleaved concurrent forward fixpoint iterator
///
/// This class computes a fixpoint on a control flow graph.
template < typename GraphRef,
           typename AbstractValue,
           typename GraphTrait = GraphTraits< GraphRef > >
class InterleavedConcurrentFwdFixpointIterator
    : public ForwardFixpointIterator< GraphRef, AbstractValue, GraphTrait > {
private:
  using NodeRef = typename GraphTrait::NodeRef;
  using WpoT = Wpo< GraphRef, GraphTrait >;
  using WpoNodeT = WpoNode< GraphRef, GraphTrait >;
  using WpoNodeKind = typename WpoNodeT::Kind;
  using WpoIndex = std::size_t;

private:
  class WorkNode;

private:
  GraphRef _cfg;
  WpoT _wpo;
  AbstractValue _bottom;
  NodeRef _entry;
  std::vector< WorkNode, tbb::cache_aligned_allocator< WorkNode > > _work_nodes;
  std::unordered_map< NodeRef, WorkNode* > _node_to_work;
  bool _converged;

public:
  /// \brief Create an interleaved concurrent forward fixpoint iterator
  ///
  /// \param cfg The control flow graph
  /// \param bottom The bottom abstract value
  explicit InterleavedConcurrentFwdFixpointIterator(GraphRef cfg,
                                                    AbstractValue bottom)
      : _cfg(cfg),
        _wpo(cfg),
        _bottom(std::move(bottom)),
        _entry(GraphTrait::entry(cfg)),
        _converged(false) {}

  /// \brief No copy constructor
  InterleavedConcurrentFwdFixpointIterator(
      const InterleavedConcurrentFwdFixpointIterator&) = delete;

  /// \brief Move constructor
  InterleavedConcurrentFwdFixpointIterator(
      InterleavedConcurrentFwdFixpointIterator&&) = default;

  /// \brief No copy assignment operator
  InterleavedConcurrentFwdFixpointIterator& operator=(
      const InterleavedConcurrentFwdFixpointIterator&) = delete;

  /// \brief Move assignment operator
  InterleavedConcurrentFwdFixpointIterator& operator=(
      InterleavedConcurrentFwdFixpointIterator&&) = default;

  /// \brief Return the control flow graph
  GraphRef cfg() const override { return this->_cfg; }

  /// \brief Return the weak partial order of the graph
  const WpoT& wpo() const { return this->_wpo; }

  /// \brief Return the bottom abstract value
  const AbstractValue& bottom() const { return this->_bottom; }

  /// \brief Return the entry node of the graph
  NodeRef entry() const { return this->_entry; }

  /// \brief Return true if the fixpoint is reached
  bool converged() const override { return this->_converged; }

  /// \brief Return the pre invariant for the given node
  const AbstractValue& pre(NodeRef node) const override {
    auto it = this->_node_to_work.find(node);
    if (it != this->_node_to_work.end()) {
      return it->second->pre();
    } else {
      return this->_bottom;
    }
  }

  /// \brief Return the post invariant for the given node
  const AbstractValue& post(NodeRef node) const override {
    auto it = this->_node_to_work.find(node);
    if (it != this->_node_to_work.end()) {
      return it->second->post();
    } else {
      return this->_bottom;
    }
  }

  /// \brief Extrapolate the new state after an increasing iteration
  ///
  /// This is called after each iteration of a cycle, until the fixpoint is
  /// reached. In order to converge, the widening operator must be applied.
  /// This method gives the user the ability to use different widening
  /// strategies.
  ///
  /// By default, it applies a join for the first iteration, and then the
  /// widening until it reaches the fixpoint.
  ///
  /// \param head Head of the cycle
  /// \param iteration Iteration number
  /// \param before Abstract value before the iteration
  /// \param after Abstract value after the iteration
  virtual AbstractValue extrapolate(NodeRef head,
                                    unsigned iteration,
                                    const AbstractValue& before,
                                    const AbstractValue& after) {
    ikos_ignore(head);

    if (iteration <= 1) {
      return before.join_iter(after);
    } else {
      return before.widening(after);
    }
  }

  /// \brief Check if the increasing iterations fixpoint is reached
  ///
  /// \param head Head of the cycle
  /// \param iteration Iteration number
  /// \param before Abstract value before the iteration
  /// \param after Abstract value after the iteration
  virtual bool is_increasing_iterations_fixpoint(NodeRef head,
                                                 unsigned iteration,
                                                 const AbstractValue& before,
                                                 const AbstractValue& after) {
    ikos_ignore(head);
    ikos_ignore(iteration);

    return after.leq(before);
  }

  /// \brief Refine the new state after a decreasing iteration
  ///
  /// This is called after each iteration of a cycle, until the post fixpoint
  /// is reached. In order to converge, the narrowing operator must be applied.
  /// This method gives the user the ability to use different narrowing
  /// strategies.
  ///
  /// By default, it applies the narrowing until it reaches the post fixpoint.
  ///
  /// \param head Head of the cycle
  /// \param iteration Iteration number
  /// \param before Abstract value before the iteration
  /// \param after Abstract value after the iteration
  virtual AbstractValue refine(NodeRef head,
                               unsigned iteration,
                               const AbstractValue& before,
                               const AbstractValue& after) {
    ikos_ignore(head);
    ikos_ignore(iteration);

    return before.narrowing(after);
  }

  /// \brief Check if the decreasing iterations fixpoint is reached
  ///
  /// \param head Head of the cycle
  /// \param iteration Iteration number
  /// \param before Abstract value before the iteration
  /// \param after Abstract value after the iteration
  virtual bool is_decreasing_iterations_fixpoint(NodeRef head,
                                                 unsigned iteration,
                                                 const AbstractValue& before,
                                                 const AbstractValue& after) {
    ikos_ignore(head);
    ikos_ignore(iteration);

    return before.leq(after);
  }

private:
  /// \brief Represents a work node
  ///
  /// This is associated to a weak partial order node and a graph node
  class WorkNode {
  private:
    using WorkNodeVector =
        std::vector< WorkNode*, tbb::cache_aligned_allocator< WorkNode* > >;

  private:
    std::mutex _mutex;
    WpoNodeKind _kind;
    NodeRef _node;
    WpoIndex _index;
    InterleavedConcurrentFwdFixpointIterator& _iterator;

    // Reference count of number of inputs that are not yet updated
    std::atomic< std::size_t > _ref_count;
    WorkNodeVector _successors;

    // For head nodes
    FixpointIterationKind _iteration_kind;
    unsigned _iteration_count;

    // For plain and head nodes
    WorkNodeVector _predecessors;
    std::mutex _post_mutex;
    AbstractValue _pre;
    AbstractValue _post;

    // For exit nodes
    WorkNode* _head;

  public:
    /// \brief Constructor
    WorkNode(WpoNodeKind kind,
             NodeRef node,
             WpoIndex index,
             InterleavedConcurrentFwdFixpointIterator& iterator,
             std::size_t ref_count,
             AbstractValue pre,
             AbstractValue post)
        : _kind(kind),
          _node(node),
          _index(index),
          _iterator(iterator),
          _ref_count(ref_count),
          _iteration_kind(FixpointIterationKind::Increasing),
          _iteration_count(0),
          _pre(std::move(pre)),
          _post(std::move(post)),
          _head(nullptr) {
      this->_pre.normalize();
    }

    /// \brief Copy constructor
    ///
    /// Required by old versions of TBB
    WorkNode(const WorkNode& other)
        : _kind(other._kind),
          _node(other._node),
          _index(other._index),
          _iterator(other._iterator),
          _ref_count(other._ref_count.load()),
          _successors(other._successors),
          _iteration_kind(other._iteration_kind),
          _iteration_count(other._iteration_count),
          _predecessors(other._predecessors),
          _pre(other._pre),
          _post(other._post),
          _head(other._head) {}

    /// \brief Move constructor
    WorkNode(WorkNode&& other)
        : _kind(other._kind),
          _node(other._node),
          _index(other._index),
          _iterator(other._iterator),
          _ref_count(other._ref_count.load()),
          _successors(std::move(other._successors)),
          _iteration_kind(other._iteration_kind),
          _iteration_count(other._iteration_count),
          _predecessors(std::move(other._predecessors)),
          _pre(std::move(other._pre)),
          _post(std::move(other._post)),
          _head(other._head) {}

    /// \brief No copy assignment operator
    WorkNode& operator=(const WorkNode&) = delete;

    /// \brief No move assignment operator
    WorkNode& operator=(WorkNode&&) = delete;

    /// \brief Destructor
    ~WorkNode() = default;

    /// \brief Return the node kind
    WpoNodeKind kind() const { return this->_kind; }

    /// \brief Return the graph node
    NodeRef node() const { return this->_node; }

    /// \brief Set the head of the given exit node
    void set_head(WorkNode* work_node) {
      ikos_assert(work_node != nullptr);
      ikos_assert(this->_kind == WpoNodeKind::Exit);
      this->_head = work_node;
    }

    /// \brief Add a successor work node
    void add_successor(WorkNode* work_node) {
      ikos_assert(work_node != nullptr);
      this->_successors.push_back(work_node);
    }

    /// \brief Add a predecessor work node
    void add_predecessor(WorkNode* work_node) {
      ikos_assert(work_node != nullptr);
      ikos_assert(this->_kind != WpoNodeKind::Exit);
      this->_predecessors.push_back(work_node);
    }

    /// \brief Return the pre invariant
    const AbstractValue& pre() const {
      ikos_assert(this->_kind != WpoNodeKind::Exit);
      return this->_pre;
    }

    /// \brief Return the post invariant
    const AbstractValue& post() const {
      ikos_assert(this->_kind != WpoNodeKind::Exit);
      return this->_post;
    }

    /// \brief Update the node
    const WorkNodeVector& update() {
      std::lock_guard< std::mutex > lock(this->_mutex);
      switch (_kind) {
        case WpoNodeKind::Plain:
          return this->update_plain();
        case WpoNodeKind::Head:
          return this->update_head();
        case WpoNodeKind::Exit:
          return this->update_exit();
        default:
          ikos_unreachable("unexpected kind");
      }
    }

    /// \brief Decrement the reference counter
    std::size_t decr_ref_count() { return --this->_ref_count; }

  private:
    /// \brief Reset the reference counter
    void reset_ref_count() {
      this->_ref_count =
          this->_iterator.wpo().num_predecessors_reducible(this->_index);
    }

    /// \brief Thread-safe read access to the post invariant
    AbstractValue get_post() {
      std::lock_guard< std::mutex > lock(this->_post_mutex);
      return AbstractValue(this->_post);
    }

    /// \brief Thread-safe write access to the post invariant
    void set_post(AbstractValue post) {
      post.normalize();
      std::lock_guard< std::mutex > lock(this->_post_mutex);
      this->_post = std::move(post);
    }

    const WorkNodeVector& update_plain() {
      ikos_assert(this->_kind == WpoNodeKind::Plain);

      // The assumption is that the pre for entry node has already been
      // initialized, and the entry node will be processed only once.
      if (this->_node != this->_iterator.entry()) {
        this->_pre = this->_iterator.bottom();
      }

      // Collect invariants from incoming edges
      for (WorkNode* pred : this->_predecessors) {
        this->_pre.join_with(this->_iterator.analyze_edge(pred->_node,
                                                          this->_node,
                                                          pred->get_post()));
      }

      this->_pre.normalize();
      this->set_post(this->_iterator.analyze_node(this->_node, this->_pre));
      this->reset_ref_count();
      return this->_successors;
    }

    /// \brief Update a head node
    const WorkNodeVector& update_head() {
      ikos_assert(this->_kind == WpoNodeKind::Head);

      // Initialization
      if (this->_iteration_count == 0) {
        // Collect invariants from incoming edges
        for (WorkNode* pred : this->_predecessors) {
          if (!this->_iterator.wpo().is_back_edge(this->_node, pred->_node)) {
            this->_pre.join_with(
                this->_iterator.analyze_edge(pred->_node,
                                             this->_node,
                                             pred->get_post()));
          }
        }

        this->_pre.normalize();
        this->_iteration_count++;
      }

      this->set_post(this->_iterator.analyze_node(this->_node, this->_pre));
      return this->_successors;
    }

    /// \brief Update a exit node
    const WorkNodeVector& update_exit() {
      ikos_assert(this->_kind == WpoNodeKind::Exit);

      bool converged = this->_head->update_head_backedge();
      if (converged) {
        this->reset_ref_count();
        this->handle_irreducible();
        return this->_successors;
      } else {
        this->reset_ref_count();
        return this->_head->update_head();
      }
    }

    /// \brief Returns true if the loop converged, false otherwise
    bool update_head_backedge() {
      ikos_assert(this->_kind == WpoNodeKind::Head);

      // Invariant from the head of the loop
      AbstractValue new_pre_in = this->_iterator.bottom();

      // Invariant from the tail of the loop
      AbstractValue new_pre_back = this->_iterator.bottom();

      for (WorkNode* pred : this->_predecessors) {
        if (!this->_iterator.wpo().is_back_edge(this->_node, pred->_node)) {
          new_pre_in.join_with(this->_iterator.analyze_edge(pred->_node,
                                                            this->_node,
                                                            pred->get_post()));
        } else {
          new_pre_back.join_with(
              this->_iterator.analyze_edge(pred->_node,
                                           this->_node,
                                           pred->get_post()));
        }
      }

      new_pre_in.join_loop_with(std::move(new_pre_back));
      AbstractValue new_pre(std::move(new_pre_in));
      new_pre.normalize();

      if (this->_iteration_kind == FixpointIterationKind::Increasing) {
        // Increasing iteration with widening
        AbstractValue inv = this->_iterator.extrapolate(this->_node,
                                                        this->_iteration_count,
                                                        this->_pre,
                                                        new_pre);
        inv.normalize();
        if (this->_iterator
                .is_increasing_iterations_fixpoint(this->_node,
                                                   this->_iteration_count,
                                                   this->_pre,
                                                   inv)) {
          // Post-fixpoint reached
          // Use this iteration as a decreasing iteration
          this->_iteration_kind = FixpointIterationKind::Decreasing;
          this->_iteration_count = 1;
        } else {
          this->_pre = std::move(inv);
          this->_iteration_count++;
          return false; // Not converged
        }
      }

      if (this->_iteration_kind == FixpointIterationKind::Decreasing) {
        // Decreasing iteration with narrowing
        AbstractValue inv = this->_iterator.refine(this->_node,
                                                   this->_iteration_count,
                                                   this->_pre,
                                                   new_pre);
        inv.normalize();
        if (this->_iterator
                .is_decreasing_iterations_fixpoint(this->_node,
                                                   this->_iteration_count,
                                                   this->_pre,
                                                   inv)) {
          // No more refinement possible
          this->_pre = std::move(inv);
          this->_iteration_kind = FixpointIterationKind::Increasing;
          this->_iteration_count = 0;
          this->reset_ref_count();
          return true; // Converged
        } else {
          this->_pre = std::move(inv);
          this->_iteration_count++;
          return false; // Not converged
        }
      }

      ikos_unreachable("unreachable");
    }

    /// \brief Handle irreducible edges
    void handle_irreducible() {
      ikos_assert(this->_kind == WpoNodeKind::Exit);

      for (const auto& p : this->_iterator.wpo().irreducibles(this->_index)) {
        this->_iterator._work_nodes[p.first]._ref_count += p.second;
      }
    }

  }; // end class WorkNode

  /// \brief Worker on a node for tbb::parallel_for_each
  class Worker {
  public:
    // Required by tbb::parallel_for_each
    using argument_type = WorkNode*;

  public:
    /// \brief Constructor
    Worker() = default;

    /// \brief No copy constructor
    Worker(const Worker&) = delete;

    /// \brief No move constructor
    Worker(Worker&&) = delete;

    /// \brief No copy assignment operator
    Worker& operator=(const Worker&) = delete;

    /// \brief No move assignment operator
    Worker& operator=(Worker&&) = delete;

    /// \brief Destructor
    ~Worker() = default;

    /// \brief Process a work node
    void operator()(WorkNode* work_node,
                    tbb::feeder< argument_type >& feeder) const {
      const auto& successors = work_node->update();

      for (WorkNode* successor : successors) {
        if (successor->decr_ref_count() == 0) {
          feeder.add(successor);
        }
      }
    }
  };

public:
  /// \brief Compute the fixpoint with the given initial abstract value
  void run(AbstractValue init) override {
    std::size_t size = this->_wpo.size();

    // Clear the fixpoint
    this->clear();

    // Build the work nodes
    this->_work_nodes.reserve(size);

    for (std::size_t idx = 0; idx < size; idx++) {
      WpoNodeKind kind = this->_wpo.kind(idx);
      NodeRef node = this->_wpo.node(idx);
      AbstractValue pre = this->_bottom;

      if (node == this->_entry && kind != WpoNodeKind::Exit) {
        pre = std::move(init);
      }

      this->_work_nodes.push_back(WorkNode(
          /* kind = */ kind,
          /* node = */ node,
          /* index = */ idx,
          /* iterator = */ *this,
          /* ref_count = */ this->_wpo.num_predecessors(idx),
          /* pre = */ std::move(pre),
          /* post = */ this->_bottom));
    }

    // Build the node to work-node map
    for (std::size_t idx = 0; idx < size; idx++) {
      WorkNode& work_node = this->_work_nodes[idx];

      if (work_node.kind() != WpoNodeKind::Exit) {
        this->_node_to_work.emplace(work_node.node(), &work_node);
      }
    }

    // Link the work nodes (predecessors/successors)
    for (std::size_t idx = 0; idx < size; idx++) {
      WorkNode& work_node = this->_work_nodes[idx];

      for (std::size_t succ : this->_wpo.successors(idx)) {
        work_node.add_successor(&this->_work_nodes[succ]);
      }

      if (work_node.kind() == WpoNodeKind::Exit) {
        work_node.set_head(&this->_work_nodes[this->_wpo.head_of_exit(idx)]);
      } else {
        NodeRef node = work_node.node();
        for (auto it = GraphTrait::predecessor_begin(node),
                  et = GraphTrait::predecessor_end(node);
             it != et;
             ++it) {
          work_node.add_predecessor(this->_node_to_work[*it]);
        }
      }
    }

    // Run the analysis
    std::array< WorkNode*, 1 > root = {this->_node_to_work[this->_entry]};
    tbb::parallel_for_each(std::begin(root), std::end(root), Worker());
    this->_converged = true;

    // Call process_pre/process_post methods
    for (WorkNode& work_node : this->_work_nodes) {
      if (work_node.kind() != WpoNodeKind::Exit) {
        NodeRef node = work_node.node();
        this->process_pre(node, work_node.pre());
        this->process_post(node, work_node.post());
      }
    }
  }

  /// \brief Clear the current fixpoint
  void clear() override {
    this->_converged = false;
    this->_work_nodes.clear();
    this->_node_to_work.clear();
  }

  /// \brief Destructor
  ~InterleavedConcurrentFwdFixpointIterator() override = default;

}; // end class InterleavedConcurrentFwdFixpointIterator

} // end namespace core
} // end namespace ikos
