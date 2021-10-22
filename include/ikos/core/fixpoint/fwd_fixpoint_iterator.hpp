/*******************************************************************************
 *
 * \file
 * \brief Forward fixpoint iterators of varying complexity and precision.
 *
 * The interleaved fixpoint iterator is described in G. Amato and F. Scozzari's
 * paper: Localizing widening and narrowing. In Proceedings of SAS 2013,
 * pages 25-42. LNCS 7935, 2013.
 *
 * Author: Arnaud J. Venet
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2019 United States Government as represented by the
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
#include <unordered_map>
#include <utility>

#include <ikos/core/fixpoint/fixpoint_iterator.hpp>
#include <ikos/core/fixpoint/wto.hpp>

namespace ikos {
namespace core {

namespace interleaved_fwd_fixpoint_iterator_impl {

template < typename GraphRef, typename AbstractValue, typename GraphTrait >
class WtoIterator;

template < typename GraphRef, typename AbstractValue, typename GraphTrait >
class WtoProcessor;

} // end namespace interleaved_fwd_fixpoint_iterator_impl

/// \brief Interleaved forward fixpoint iterator
///
/// This class computes a fixpoint on a control flow graph.
template < typename GraphRef,
           typename AbstractValue,
           typename GraphTrait = GraphTraits< GraphRef > >
class InterleavedFwdFixpointIterator
    : public ForwardFixpointIterator< GraphRef, AbstractValue, GraphTrait > {
  friend class interleaved_fwd_fixpoint_iterator_impl::
      WtoIterator< GraphRef, AbstractValue, GraphTrait >;

private:
  using NodeRef = typename GraphTrait::NodeRef;
  using InvariantTable = std::unordered_map< NodeRef, AbstractValue >;
  using WtoT = Wto< GraphRef, GraphTrait >;
  using WtoIterator = interleaved_fwd_fixpoint_iterator_impl::
      WtoIterator< GraphRef, AbstractValue, GraphTrait >;
  using WtoProcessor = interleaved_fwd_fixpoint_iterator_impl::
      WtoProcessor< GraphRef, AbstractValue, GraphTrait >;

private:
  GraphRef _cfg;
  WtoT _wto;
  AbstractValue _bottom;
  InvariantTable _pre;
  InvariantTable _post;
  bool _converged;

public:
  /// \brief Create an interleaved forward fixpoint iterator
  ///
  /// \param cfg The control flow graph
  /// \param bottom The bottom abstract value
  InterleavedFwdFixpointIterator(GraphRef cfg, AbstractValue bottom)
      : _cfg(cfg), _wto(cfg), _bottom(std::move(bottom)), _converged(false) {}

  /// \brief No copy constructor
  InterleavedFwdFixpointIterator(const InterleavedFwdFixpointIterator&) =
      delete;

  /// \brief Move constructor
  InterleavedFwdFixpointIterator(InterleavedFwdFixpointIterator&&) = default;

  /// \brief No copy assignment operator
  InterleavedFwdFixpointIterator& operator=(
      const InterleavedFwdFixpointIterator&) = delete;

  /// \brief Move assignment operator
  InterleavedFwdFixpointIterator& operator=(InterleavedFwdFixpointIterator&&) =
      default;

  /// \brief Return the control flow graph
  GraphRef cfg() const override { return this->_cfg; }

  /// \brief Return the weak topological order of the graph
  const WtoT& wto() const { return this->_wto; }

  /// \brief Return the bottom abstract value
  const AbstractValue& bottom() const { return this->_bottom; }

  /// \brief Return true if the fixpoint is reached
  bool converged() const override { return this->_converged; }

private:
  /// \brief Set the invariant for the given node
  void set(InvariantTable& table, NodeRef node, AbstractValue inv) const {
    inv.normalize();

    auto it = table.find(node);
    if (it != table.end()) {
      it->second = std::move(inv);
    } else {
      table.emplace(node, std::move(inv));
    }
  }

  /// \brief Set the pre invariant for the given node
  void set_pre(NodeRef node, AbstractValue inv) {
    this->set(this->_pre, node, std::move(inv));
  }

  /// \brief Set the post invariant for the given node
  void set_post(NodeRef node, AbstractValue inv) {
    this->set(this->_post, node, std::move(inv));
  }

  /// \brief Return the invariant for the given node
  const AbstractValue& get(const InvariantTable& table, NodeRef node) const {
    auto it = table.find(node);
    if (it != table.end()) {
      return it->second;
    } else {
      return this->_bottom;
    }
  }

public:
  /// \brief Return the pre invariant for the given node
  const AbstractValue& pre(NodeRef node) const override {
    return this->get(this->_pre, node);
  }

  /// \brief Return the post invariant for the given node
  const AbstractValue& post(NodeRef node) const override {
    return this->get(this->_post, node);
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

  /// \brief Notify the beginning of the analysis of a cycle
  ///
  /// This method is called before analyzing a cycle.
  virtual void notify_enter_cycle(NodeRef head) { ikos_ignore(head); }

  /// \brief Notify the beginning of an iteration on a cycle
  ///
  /// This method is called for each iteration on a cycle.
  virtual void notify_cycle_iteration(NodeRef head,
                                      unsigned iteration,
                                      FixpointIterationKind kind) {
    ikos_ignore(head);
    ikos_ignore(iteration);
    ikos_ignore(kind);
  }

  /// \brief Notify the end of the analysis of a cycle
  ///
  /// This method is called after reaching a fixpoint on a cycle.
  virtual void notify_leave_cycle(NodeRef head) { ikos_ignore(head); }

  /// \brief Compute the fixpoint with the given initial abstract value
  void run(AbstractValue init) override {
    this->clear();
    this->set_pre(GraphTrait::entry(this->_cfg), std::move(init));

    // Compute the fixpoint
    WtoIterator iterator(*this);
    this->_wto.accept(iterator);
    this->_converged = true;

    // Call process_pre/process_post methods
    WtoProcessor processor(*this);
    this->_wto.accept(processor);
  }

  /// \brief Clear the pre invariants
  void clear_pre() { this->_pre.clear(); }

  /// \brief Clear the post invariants
  void clear_post() { this->_post.clear(); }

  /// \brief Clear the current fixpoint
  void clear() override {
    this->_converged = false;
    this->_pre.clear();
    this->_post.clear();
  }

  /// \brief Destructor
  ~InterleavedFwdFixpointIterator() override = default;

}; // end class InterleavedFwdFixpointIterator

namespace interleaved_fwd_fixpoint_iterator_impl {

template < typename GraphRef, typename AbstractValue, typename GraphTrait >
class WtoIterator final : public WtoComponentVisitor< GraphRef, GraphTrait > {
public:
  using InterleavedIterator =
      InterleavedFwdFixpointIterator< GraphRef, AbstractValue, GraphTrait >;
  using NodeRef = typename GraphTrait::NodeRef;
  using WtoVertexT = WtoVertex< GraphRef, GraphTrait >;
  using WtoCycleT = WtoCycle< GraphRef, GraphTrait >;
  using WtoT = Wto< GraphRef, GraphTrait >;
  using WtoNestingT = WtoNesting< GraphRef, GraphTrait >;

private:
  /// \brief Fixpoint engine
  InterleavedIterator& _iterator;

  /// \brief Graph entry point
  NodeRef _entry;

public:
  explicit WtoIterator(InterleavedIterator& iterator)
      : _iterator(iterator), _entry(GraphTrait::entry(iterator.cfg())) {}

  void visit(const WtoVertexT& vertex) override {
    NodeRef node = vertex.node();
    AbstractValue pre = this->_iterator.bottom();

    // Use the invariant for the entry point
    if (node == this->_entry) {
      pre = this->_iterator.pre(node);
    }

    // Collect invariants from incoming edges
    for (auto it = GraphTrait::predecessor_begin(node),
              et = GraphTrait::predecessor_end(node);
         it != et;
         ++it) {
      NodeRef pred = *it;
      pre.join_with(
          this->_iterator.analyze_edge(pred, node, this->_iterator.post(pred)));
    }

    pre.normalize();
    this->_iterator.set_pre(node, pre);
    this->_iterator.set_post(node, this->_iterator.analyze_node(node, pre));
  }

  void visit(const WtoCycleT& cycle) override {
    NodeRef head = cycle.head();
    AbstractValue pre = this->_iterator.bottom();
    const WtoNestingT& cycle_nesting = this->_iterator.wto().nesting(head);

    this->_iterator.notify_enter_cycle(head);

    // Collect invariants from incoming edges
    for (auto it = GraphTrait::predecessor_begin(head),
              et = GraphTrait::predecessor_end(head);
         it != et;
         ++it) {
      NodeRef pred = *it;
      if (this->_iterator.wto().nesting(pred) <= cycle_nesting) {
        pre.join_with(this->_iterator.analyze_edge(pred,
                                                   head,
                                                   this->_iterator.post(pred)));
      }
    }

    // Fixpoint iterations
    FixpointIterationKind kind = FixpointIterationKind::Increasing;
    for (unsigned iteration = 1;; ++iteration) {
      this->_iterator.notify_cycle_iteration(head, iteration, kind);
      pre.normalize();
      this->_iterator.set_pre(head, pre);
      this->_iterator.set_post(head, this->_iterator.analyze_node(head, pre));

      for (auto it = cycle.begin(), et = cycle.end(); it != et; ++it) {
        it->accept(*this);
      }

      // Invariant from the head of the loop
      AbstractValue new_pre_in = this->_iterator.bottom();

      // Invariant from the tail of the loop
      AbstractValue new_pre_back = this->_iterator.bottom();

      for (auto it = GraphTrait::predecessor_begin(head),
                et = GraphTrait::predecessor_end(head);
           it != et;
           ++it) {
        NodeRef pred = *it;
        AbstractValue inv =
            this->_iterator.analyze_edge(pred,
                                         head,
                                         this->_iterator.post(pred));
        if (this->_iterator.wto().nesting(pred) <= cycle_nesting) {
          new_pre_in.join_with(std::move(inv));
        } else {
          new_pre_back.join_with(std::move(inv));
        }
      }

      new_pre_in.join_loop_with(std::move(new_pre_back));
      AbstractValue new_pre(std::move(new_pre_in));
      new_pre.normalize();

      if (kind == FixpointIterationKind::Increasing) {
        // Increasing iteration with widening
        AbstractValue inv =
            this->_iterator.extrapolate(head, iteration, pre, new_pre);
        inv.normalize();
        if (this->_iterator.is_increasing_iterations_fixpoint(head,
                                                              iteration,
                                                              pre,
                                                              inv)) {
          // Post-fixpoint reached
          // Use this iteration as a decreasing iteration
          kind = FixpointIterationKind::Decreasing;
          iteration = 1;
        } else {
          pre = std::move(inv);
        }
      }

      if (kind == FixpointIterationKind::Decreasing) {
        // Decreasing iteration with narrowing
        AbstractValue inv =
            this->_iterator.refine(head, iteration, pre, new_pre);
        inv.normalize();
        if (this->_iterator.is_decreasing_iterations_fixpoint(head,
                                                              iteration,
                                                              pre,
                                                              inv)) {
          // No more refinement possible
          this->_iterator.set_pre(head, std::move(inv));
          break;
        } else {
          pre = std::move(inv);
        }
      }
    }

    this->_iterator.notify_leave_cycle(head);
  }

}; // end class WtoIterator

template < typename GraphRef, typename AbstractValue, typename GraphTrait >
class WtoProcessor final : public WtoComponentVisitor< GraphRef, GraphTrait > {
public:
  using InterleavedIterator =
      InterleavedFwdFixpointIterator< GraphRef, AbstractValue, GraphTrait >;
  using NodeRef = typename GraphTrait::NodeRef;
  using WtoVertexT = WtoVertex< GraphRef, GraphTrait >;
  using WtoCycleT = WtoCycle< GraphRef, GraphTrait >;

private:
  InterleavedIterator& _iterator;

public:
  explicit WtoProcessor(InterleavedIterator& iterator) : _iterator(iterator) {}

  void visit(const WtoVertexT& vertex) override {
    NodeRef node = vertex.node();
    this->_iterator.process_pre(node, this->_iterator.pre(node));
    this->_iterator.process_post(node, this->_iterator.post(node));
  }

  void visit(const WtoCycleT& cycle) override {
    NodeRef head = cycle.head();
    this->_iterator.process_pre(head, this->_iterator.pre(head));
    this->_iterator.process_post(head, this->_iterator.post(head));

    for (auto it = cycle.begin(), et = cycle.end(); it != et; ++it) {
      it->accept(*this);
    }
  }

}; // end class WtoProcessor

} // end namespace interleaved_fwd_fixpoint_iterator_impl

} // end namespace core
} // end namespace ikos
