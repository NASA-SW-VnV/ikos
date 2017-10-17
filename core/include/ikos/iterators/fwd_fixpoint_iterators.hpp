/**************************************************************************/ /**
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

#ifndef IKOS_FWD_FIXPOINT_ITERATORS_HPP
#define IKOS_FWD_FIXPOINT_ITERATORS_HPP

#include <iostream>
#include <unordered_map>
#include <utility>

#include <ikos/common/types.hpp>
#include <ikos/iterators/fixpoint_iterators_api.hpp>
#include <ikos/iterators/wto.hpp>

namespace ikos {

namespace interleaved_fwd_fixpoint_iterator_impl {

template < typename NodeName, typename CFG, typename AbstractValue >
class wto_iterator;

template < typename NodeName, typename CFG, typename AbstractValue >
class wto_processor;

} // end namespace interleaved_fwd_fixpoint_iterator_impl

template < typename NodeName, typename CFG, typename AbstractValue >
class interleaved_fwd_fixpoint_iterator
    : public forward_fixpoint_iterator< NodeName, CFG, AbstractValue > {
  friend class interleaved_fwd_fixpoint_iterator_impl::
      wto_iterator< NodeName, CFG, AbstractValue >;

private:
  typedef std::unordered_map< NodeName, AbstractValue > invariant_table_t;
  typedef std::shared_ptr< invariant_table_t > invariant_table_ptr;
  typedef wto< NodeName, CFG > wto_t;
  typedef interleaved_fwd_fixpoint_iterator_impl::
      wto_iterator< NodeName, CFG, AbstractValue > wto_iterator_t;
  typedef interleaved_fwd_fixpoint_iterator_impl::
      wto_processor< NodeName, CFG, AbstractValue > wto_processor_t;

private:
  CFG _cfg;
  wto_t _wto;
  invariant_table_ptr _pre, _post;

private:
  void set(invariant_table_ptr table, NodeName node, AbstractValue v) {
    std::pair< typename invariant_table_t::iterator, bool > res =
        table->insert(std::make_pair(node, v));
    if (!res.second) {
      (res.first)->second = v;
    }
  }

  void set_pre(NodeName node, AbstractValue v) {
    this->set(this->_pre, node, v);
  }

  void set_post(NodeName node, AbstractValue v) {
    this->set(this->_post, node, v);
  }

  AbstractValue get(invariant_table_ptr table, NodeName n) {
    typename invariant_table_t::iterator it = table->find(n);
    if (it != table->end()) {
      return it->second;
    } else {
      return AbstractValue::bottom();
    }
  }

public:
  interleaved_fwd_fixpoint_iterator(CFG cfg)
      : _cfg(cfg),
        _wto(cfg),
        _pre(invariant_table_ptr(new invariant_table_t)),
        _post(invariant_table_ptr(new invariant_table_t)) {}

  CFG get_cfg() { return this->_cfg; }

  wto_t get_wto() { return this->_wto; }

  AbstractValue get_pre(NodeName node) { return this->get(this->_pre, node); }

  AbstractValue get_post(NodeName node) { return this->get(this->_post, node); }

  virtual AbstractValue extrapolate(NodeName /* node */,
                                    unsigned int iteration,
                                    AbstractValue before,
                                    AbstractValue after) {
    if (iteration <= 1) {
      return before.join_iter(after);
    } else {
      return before || after;
    }
  }

  virtual AbstractValue refine(NodeName /* node */,
                               unsigned int iteration,
                               AbstractValue before,
                               AbstractValue after) {
    if (iteration <= 1) {
      return before & after;
    } else {
      return before && after;
    }
  }

  void run(AbstractValue init) {
    this->set_pre(this->_cfg.entry(), init);
    wto_iterator_t iterator(this);
    this->_wto.accept(&iterator);
    wto_processor_t processor(this);
    this->_wto.accept(&processor);
  }

  virtual ~interleaved_fwd_fixpoint_iterator() {}

}; // end class interleaved_fwd_fixpoint_iterator

namespace interleaved_fwd_fixpoint_iterator_impl {

template < typename NodeName, typename CFG, typename AbstractValue >
class wto_iterator : public wto_component_visitor< NodeName, CFG > {
public:
  typedef interleaved_fwd_fixpoint_iterator< NodeName, CFG, AbstractValue >
      interleaved_iterator_t;
  typedef wto_vertex< NodeName, CFG > wto_vertex_t;
  typedef wto_cycle< NodeName, CFG > wto_cycle_t;
  typedef wto< NodeName, CFG > wto_t;
  typedef typename wto_t::wto_nesting_t wto_nesting_t;
  typedef typename CFG::node_collection_t node_collection_t;

private:
  interleaved_iterator_t* _iterator;

public:
  wto_iterator(interleaved_iterator_t* iterator) : _iterator(iterator) {}

  void visit(wto_vertex_t& vertex) {
    AbstractValue pre;
    NodeName node = vertex.node();
    if (node == this->_iterator->get_cfg().entry()) {
      pre = this->_iterator->get_pre(node);
    } else {
      node_collection_t prev_nodes = this->_iterator->_cfg.prev_nodes(node);
      pre = AbstractValue::bottom();
      for (typename node_collection_t::iterator it = prev_nodes.begin();
           it != prev_nodes.end();
           ++it) {
        NodeName prev = *it;
        pre = pre | this->_iterator->get_post(prev);
      }
      this->_iterator->set_pre(node, pre);
    }
    this->_iterator->set_post(node, this->_iterator->analyze(node, pre));
  }

  void visit(wto_cycle_t& cycle) {
    NodeName head = cycle.head();
    wto_nesting_t cycle_nesting = this->_iterator->_wto.nesting(head);
    node_collection_t prev_nodes = this->_iterator->_cfg.prev_nodes(head);
    AbstractValue pre = AbstractValue::bottom();
    for (typename node_collection_t::iterator it = prev_nodes.begin();
         it != prev_nodes.end();
         ++it) {
      NodeName prev = *it;
      if (!(this->_iterator->_wto.nesting(prev) > cycle_nesting)) {
        pre = pre | this->_iterator->get_post(prev);
      }
    }
    for (unsigned int iteration = 1;; ++iteration) {
      // Increasing iteration sequence with widening
      this->_iterator->set_pre(head, pre);
      this->_iterator->set_post(head, this->_iterator->analyze(head, pre));
      for (typename wto_cycle_t::iterator it = cycle.begin(); it != cycle.end();
           ++it) {
        it->accept(this);
      }
      // invariant from the head of the loop
      AbstractValue new_pre_in = AbstractValue::bottom();
      for (typename node_collection_t::iterator it = prev_nodes.begin();
           it != prev_nodes.end();
           ++it) {
        NodeName prev = *it;
        if (!(this->_iterator->_wto.nesting(prev) > cycle_nesting)) {
          new_pre_in = new_pre_in | this->_iterator->get_post(prev);
        }
      }
      // invariant from the tail of the loop
      AbstractValue new_pre_back = AbstractValue::bottom();
      for (typename node_collection_t::iterator it = prev_nodes.begin();
           it != prev_nodes.end();
           ++it) {
        NodeName prev = *it;
        if (this->_iterator->_wto.nesting(prev) > cycle_nesting) {
          new_pre_back = new_pre_back | this->_iterator->get_post(prev);
        }
      }
      AbstractValue new_pre = new_pre_in.join_loop(new_pre_back);
      if (new_pre <= pre) {
        // Post-fixpoint reached
        this->_iterator->set_pre(head, new_pre);
        pre = new_pre;
        break;
      } else {
        pre = this->_iterator->extrapolate(head, iteration, pre, new_pre);
      }
    }
    for (unsigned int iteration = 1;; ++iteration) {
      // Decreasing iteration sequence with narrowing
      this->_iterator->set_post(head, this->_iterator->analyze(head, pre));
      for (typename wto_cycle_t::iterator it = cycle.begin(); it != cycle.end();
           ++it) {
        it->accept(this);
      }
      // invariant from the head of the loop
      AbstractValue new_pre_in = AbstractValue::bottom();
      for (typename node_collection_t::iterator it = prev_nodes.begin();
           it != prev_nodes.end();
           ++it) {
        NodeName prev = *it;
        if (!(this->_iterator->_wto.nesting(prev) > cycle_nesting)) {
          new_pre_in = new_pre_in | this->_iterator->get_post(prev);
        }
      }
      // invariant from the tail of the loop
      AbstractValue new_pre_back = AbstractValue::bottom();
      for (typename node_collection_t::iterator it = prev_nodes.begin();
           it != prev_nodes.end();
           ++it) {
        NodeName prev = *it;
        if (this->_iterator->_wto.nesting(prev) > cycle_nesting) {
          new_pre_back = new_pre_back | this->_iterator->get_post(prev);
        }
      }
      AbstractValue new_pre = new_pre_in.join_loop(new_pre_back);
      if (pre <= new_pre) {
        // No more refinement possible (pre == new_pre)
        break;
      } else {
        pre = this->_iterator->refine(head, iteration, pre, new_pre);
        this->_iterator->set_pre(head, pre);
      }
    }
  }

}; // end class wto_iterator

template < typename NodeName, typename CFG, typename AbstractValue >
class wto_processor : public wto_component_visitor< NodeName, CFG > {
public:
  typedef interleaved_fwd_fixpoint_iterator< NodeName, CFG, AbstractValue >
      interleaved_iterator_t;
  typedef wto_vertex< NodeName, CFG > wto_vertex_t;
  typedef wto_cycle< NodeName, CFG > wto_cycle_t;

private:
  interleaved_iterator_t* _iterator;

public:
  wto_processor(interleaved_iterator_t* iterator) : _iterator(iterator) {}

  void visit(wto_vertex_t& vertex) {
    NodeName node = vertex.node();
    this->_iterator->process_pre(node, this->_iterator->get_pre(node));
    this->_iterator->process_post(node, this->_iterator->get_post(node));
  }

  void visit(wto_cycle_t& cycle) {
    NodeName head = cycle.head();
    this->_iterator->process_pre(head, this->_iterator->get_pre(head));
    this->_iterator->process_post(head, this->_iterator->get_post(head));
    for (typename wto_cycle_t::iterator it = cycle.begin(); it != cycle.end();
         ++it) {
      it->accept(this);
    }
  }

}; // end class wto_processor

} // end namespace interleaved_fwd_fixpoint_iterator_impl

} // end namespace ikos

#endif // IKOS_FWD_FIXPOINT_ITERATORS
