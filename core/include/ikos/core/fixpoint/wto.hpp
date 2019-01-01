/*******************************************************************************
 *
 * \file
 * \brief Construction and management of weak topological orderings (WTOs).
 *
 * The construction of weak topological orderings is based on F. Bourdoncle's
 * paper: "Efficient chaotic iteration strategies with widenings", Formal
 * Methods in Programming and Their Applications, 1993, pages 128-141.
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

#include <deque>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/container/slist.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <ikos/core/number/bound.hpp>
#include <ikos/core/semantic/dumpable.hpp>
#include <ikos/core/semantic/graph.hpp>

namespace ikos {
namespace core {

template < typename GraphRef, typename GraphTrait >
class Wto;

template < typename GraphRef, typename GraphTrait >
class WtoVertex;

template < typename GraphRef, typename GraphTrait >
class WtoCycle;

template < typename GraphRef, typename GraphTrait >
class WtoComponentVisitor;

template < typename GraphRef, typename GraphTrait >
class WtoNesting {
  friend class Wto< GraphRef, GraphTrait >;
  friend class WtoVertex< GraphRef, GraphTrait >;
  friend class WtoCycle< GraphRef, GraphTrait >;

public:
  using NodeRef = typename GraphTrait::NodeRef;

private:
  using NodeList = std::vector< NodeRef >;
  using NodeListPtr = std::shared_ptr< NodeList >;

private:
  NodeListPtr _nodes;

public:
  class Iterator : public boost::iterator_facade< Iterator,
                                                  NodeRef,
                                                  boost::forward_traversal_tag,
                                                  NodeRef > {
    friend class boost::iterator_core_access;

  private:
    typename NodeList::const_iterator _it;
    NodeListPtr _l;

  public:
    Iterator(const NodeListPtr& l, bool b)
        : _it(b ? l->begin() : l->end()), _l(l) {}

  private:
    void increment() { ++this->_it; }

    bool equal(const Iterator& other) const {
      return this->_l == other._l && this->_it == other._it;
    }

    NodeRef dereference() const {
      ikos_assert_msg(this->_it != this->_l->end(),
                      "trying to dereference an empty iterator");
      return *this->_it;
    }

  }; // end class Iterator

private:
  explicit WtoNesting(const NodeListPtr& l)
      : _nodes(std::make_shared< NodeList >(*l)) {}

  int compare(const WtoNesting& other) const {
    Iterator this_it = this->begin(), other_it = other.begin();
    while (this_it != this->end()) {
      if (other_it == other.end()) {
        return 1;
      } else if (*this_it == *other_it) {
        ++this_it;
        ++other_it;
      } else {
        return 2; // Nestings are not comparable
      }
    }
    if (other_it == other.end()) {
      return 0;
    } else {
      return -1;
    }
  }

public:
  WtoNesting() : _nodes(std::make_shared< NodeList >()) {}

  void operator+=(NodeRef n) {
    this->_nodes = std::make_shared< NodeList >(*this->_nodes);
    this->_nodes->push_back(n);
  }

  WtoNesting operator+(NodeRef n) const {
    WtoNesting res(this->_nodes);
    res._nodes->push_back(n);
    return res;
  }

  Iterator begin() const { return Iterator(this->_nodes, true); }

  Iterator end() const { return Iterator(this->_nodes, false); }

  WtoNesting operator^(const WtoNesting& other) const {
    WtoNesting res;
    for (Iterator this_it = this->begin(), other_it = other.begin();
         this_it != this->end() && other_it != other.end();
         ++this_it, ++other_it) {
      if (*this_it == *other_it) {
        res._nodes->push_back(*this_it);
      } else {
        break;
      }
    }
    return res;
  }

  bool operator<=(const WtoNesting& other) const {
    return this->compare(other) <= 0;
  }

  bool operator==(const WtoNesting& other) const {
    return this->compare(other) == 0;
  }

  bool operator>=(const WtoNesting& other) const {
    return this->operator<=(other, *this);
  }

  bool operator>(const WtoNesting& other) const {
    return this->compare(other) == 1;
  }

  void dump(std::ostream& o) const {
    o << "[";
    for (Iterator it = this->begin(); it != this->end();) {
      DumpableTraits< NodeRef >::dump(o, *it);
      ++it;
      if (it != this->end()) {
        o << ", ";
      }
    }
    o << "]";
  }

}; // end class WtoNesting

/// \brief Base class for components of a weak topological order
///
/// This is either a vertex or a cycle.
template < typename GraphRef, typename GraphTrait = GraphTraits< GraphRef > >
class WtoComponent {
public:
  WtoComponent() = default;

  WtoComponent(const WtoComponent&) = default;

  WtoComponent(WtoComponent&&) = default;

  WtoComponent& operator=(const WtoComponent&) = default;

  WtoComponent& operator=(WtoComponent&&) = default;

  virtual void accept(WtoComponentVisitor< GraphRef, GraphTrait >&) const = 0;

  virtual ~WtoComponent() = default;

}; // end class WtoComponent

template < typename GraphRef, typename GraphTrait = GraphTraits< GraphRef > >
class WtoVertex final : public WtoComponent< GraphRef, GraphTrait > {
  friend class Wto< GraphRef, GraphTrait >;

public:
  using NodeRef = typename GraphTrait::NodeRef;

private:
  NodeRef _node;

private:
  /// \brief Tag to call the private constructor
  struct PrivateCtor {};

public:
  WtoVertex(NodeRef node, PrivateCtor) : _node(node) {}

public:
  NodeRef node() const { return this->_node; }

  void accept(WtoComponentVisitor< GraphRef, GraphTrait >& v) const override {
    v.visit(*this);
  }

  void dump(std::ostream& o) const {
    DumpableTraits< NodeRef >::dump(o, this->_node);
  }

}; // end class WtoVertex

template < typename GraphRef, typename GraphTrait = GraphTraits< GraphRef > >
class WtoCycle final : public WtoComponent< GraphRef, GraphTrait > {
  friend class Wto< GraphRef, GraphTrait >;

public:
  using NodeRef = typename GraphTrait::NodeRef;
  using WtoComponentT = WtoComponent< GraphRef, GraphTrait >;

private:
  using WtoComponentPtr = std::shared_ptr< WtoComponentT >;
  using WtoComponentList = boost::container::slist< WtoComponentPtr >;
  using WtoComponentListPtr = std::shared_ptr< WtoComponentList >;

private:
  NodeRef _head;
  WtoComponentListPtr _components;

private:
  /// \brief Tag to call the private constructor
  struct PrivateCtor {};

public:
  WtoCycle(NodeRef head, WtoComponentListPtr components, PrivateCtor)
      : _head(head), _components(std::move(components)) {}

public:
  class Iterator : public boost::iterator_facade< Iterator,
                                                  const WtoComponentT&,
                                                  boost::forward_traversal_tag,
                                                  const WtoComponentT& > {
    friend class boost::iterator_core_access;

  private:
    typename WtoComponentList::const_iterator _it;
    WtoComponentListPtr _l;

  public:
    Iterator(const WtoComponentListPtr& l, bool b)
        : _it(b ? l->begin() : l->end()), _l(l) {}

  private:
    void increment() { ++this->_it; }

    bool equal(const Iterator& other) const {
      return this->_l == other._l && this->_it == other._it;
    }

    const WtoComponentT& dereference() const {
      ikos_assert_msg(this->_it != this->_l->end(),
                      "trying to dereference an empty iterator");
      return **this->_it;
    }

  }; // end class Iterator

public:
  NodeRef head() const { return this->_head; }

  void accept(WtoComponentVisitor< GraphRef, GraphTrait >& v) const override {
    v.visit(*this);
  }

  Iterator begin() const { return Iterator(this->_components, true); }

  Iterator end() const { return Iterator(this->_components, false); }

  void dump(std::ostream& o) const {
    o << "(";
    DumpableTraits< NodeRef >::dump(o, this->_head);
    if (!this->_components->empty()) {
      o << " ";
      for (Iterator it = this->begin(); it != this->end();) {
        it->dump(o);
        ++it;
        if (it != this->end()) {
          o << " ";
        }
      }
    }
    o << ")";
  }

}; // end class WtoCycle

template < typename GraphRef, typename GraphTrait = GraphTraits< GraphRef > >
class WtoComponentVisitor {
public:
  using WtoVertexT = WtoVertex< GraphRef, GraphTrait >;
  using WtoCycleT = WtoCycle< GraphRef, GraphTrait >;

public:
  WtoComponentVisitor() = default;

  WtoComponentVisitor(const WtoComponentVisitor&) = default;

  WtoComponentVisitor(WtoComponentVisitor&&) = default;

  WtoComponentVisitor& operator=(const WtoComponentVisitor&) = default;

  WtoComponentVisitor& operator=(WtoComponentVisitor&&) = default;

  virtual void visit(const WtoVertexT&) = 0;

  virtual void visit(const WtoCycleT&) = 0;

  virtual ~WtoComponentVisitor() = default;

}; // end class WtoComponentVisitor

/// \brief Weak Topological Ordering
template < typename GraphRef, typename GraphTrait = GraphTraits< GraphRef > >
class Wto {
public:
  static_assert(IsGraph< GraphRef, GraphTrait >::value,
                "GraphRef does not implement GraphTraits");

public:
  using NodeRef = typename GraphTrait::NodeRef;
  using WtoNestingT = WtoNesting< GraphRef, GraphTrait >;
  using WtoComponentT = WtoComponent< GraphRef, GraphTrait >;
  using WtoVertexT = WtoVertex< GraphRef, GraphTrait >;
  using WtoCycleT = WtoCycle< GraphRef, GraphTrait >;

private:
  using WtoComponentPtr = std::shared_ptr< WtoComponentT >;
  using WtoVertexPtr = std::shared_ptr< WtoVertexT >;
  using WtoCyclePtr = std::shared_ptr< WtoCycleT >;
  using WtoComponentList = boost::container::slist< WtoComponentPtr >;
  using WtoComponentListPtr = std::shared_ptr< WtoComponentList >;
  using Dfn = Bound< ZNumber >;
  using DfnTable = std::unordered_map< NodeRef, Dfn >;
  using DfnTablePtr = std::shared_ptr< DfnTable >;
  using Stack = std::deque< NodeRef >;
  using StackPtr = std::shared_ptr< Stack >;
  using NestingTable = std::unordered_map< NodeRef, WtoNestingT >;
  using NestingTablePtr = std::shared_ptr< NestingTable >;

private:
  WtoComponentListPtr _components;
  DfnTablePtr _dfn_table;
  Dfn _num;
  StackPtr _stack;
  NestingTablePtr _nesting_table;

private:
  class NestingBuilder final
      : public WtoComponentVisitor< GraphRef, GraphTrait > {
  public:
    using WtoVertexT = WtoVertex< GraphRef, GraphTrait >;
    using WtoCycleT = WtoCycle< GraphRef, GraphTrait >;

  private:
    WtoNestingT _nesting;
    NestingTablePtr _nesting_table;

  public:
    explicit NestingBuilder(NestingTablePtr nesting_table)
        : _nesting_table(std::move(nesting_table)) {}

    void visit(const WtoCycleT& cycle) override {
      NodeRef head = cycle.head();
      WtoNestingT previous_nesting = this->_nesting;
      this->_nesting_table->insert(std::make_pair(head, this->_nesting));
      this->_nesting += head;
      for (auto it = cycle.begin(); it != cycle.end(); ++it) {
        it->accept(*this);
      }
      this->_nesting = previous_nesting;
    }

    void visit(const WtoVertexT& vertex) override {
      this->_nesting_table->insert(
          std::make_pair(vertex.node(), this->_nesting));
    }

  }; // end class NestingBuilder

private:
  Dfn dfn(NodeRef n) const {
    auto it = this->_dfn_table->find(n);
    if (it == this->_dfn_table->end()) {
      return Dfn(0);
    } else {
      return it->second;
    }
  }

  void set_dfn(NodeRef n, const Dfn& dfn) {
    std::pair< typename DfnTable::iterator, bool > res =
        this->_dfn_table->insert(std::make_pair(n, dfn));
    if (!res.second) {
      (res.first)->second = dfn;
    }
  }

  NodeRef pop() {
    ikos_assert_msg(!this->_stack->empty(), "empty stack");
    NodeRef top = this->_stack->back();
    this->_stack->pop_back();
    return top;
  }

  void push(NodeRef n) { this->_stack->push_back(n); }

  WtoCyclePtr component(GraphRef cfg, NodeRef vertex) {
    WtoComponentListPtr partition = std::make_shared< WtoComponentList >();
    for (auto it = GraphTrait::successor_begin(vertex),
              et = GraphTrait::successor_end(vertex);
         it != et;
         ++it) {
      NodeRef succ = *it;
      if (this->dfn(succ) == Dfn(0)) {
        this->visit(cfg, succ, partition);
      }
    }
    return std::make_shared< WtoCycleT >(vertex,
                                         partition,
                                         typename WtoCycleT::PrivateCtor());
  }

  Dfn visit(GraphRef cfg,
            NodeRef vertex,
            const WtoComponentListPtr& partition) {
    Dfn head(0), min(0);
    bool loop;

    this->push(vertex);
    this->_num += Dfn(1);
    head = this->_num;
    this->set_dfn(vertex, head);
    loop = false;
    for (auto it = GraphTrait::successor_begin(vertex),
              et = GraphTrait::successor_end(vertex);
         it != et;
         ++it) {
      NodeRef succ = *it;
      Dfn succ_dfn = this->dfn(succ);
      if (succ_dfn == Dfn(0)) {
        min = this->visit(cfg, succ, partition);
      } else {
        min = succ_dfn;
      }
      if (min <= head) {
        head = min;
        loop = true;
      }
    }
    if (head == this->dfn(vertex)) {
      this->set_dfn(vertex, Dfn::plus_infinity());
      NodeRef element = this->pop();
      if (loop) {
        while (element != vertex) {
          this->set_dfn(element, Dfn(0));
          element = this->pop();
        }
        partition->push_front(
            std::static_pointer_cast< WtoComponentT, WtoCycleT >(
                this->component(cfg, vertex)));
      } else {
        partition->push_front(
            std::static_pointer_cast< WtoComponentT, WtoVertexT >(
                std::make_shared<
                    WtoVertexT >(vertex, typename WtoVertexT::PrivateCtor())));
      }
    }
    return head;
  }

  void build_nesting() {
    NestingBuilder builder(this->_nesting_table);
    for (Iterator it = this->begin(); it != this->end(); ++it) {
      it->accept(builder);
    }
  }

public:
  class Iterator : public boost::iterator_facade< Iterator,
                                                  const WtoComponentT&,
                                                  boost::forward_traversal_tag,
                                                  const WtoComponentT& > {
    friend class boost::iterator_core_access;

  private:
    typename WtoComponentList::const_iterator _it;
    WtoComponentListPtr _l;

  public:
    Iterator(const WtoComponentListPtr& l, bool b)
        : _it(b ? l->begin() : l->end()), _l(l) {}

  private:
    void increment() { ++this->_it; }

    bool equal(const Iterator& other) const {
      return this->_l == other._l && this->_it == other._it;
    }

    const WtoComponentT& dereference() const {
      ikos_assert_msg(this->_it != this->_l->end(),
                      "trying to dereference an empty iterator");
      return **this->_it;
    }

  }; // end class Iterator

public:
  /// \brief Compute the weak topological order of the given graph
  explicit Wto(GraphRef cfg)
      : _components(std::make_shared< WtoComponentList >()),
        _dfn_table(std::make_shared< DfnTable >()),
        _num(0),
        _stack(std::make_shared< Stack >()),
        _nesting_table(std::make_shared< NestingTable >()) {
    this->visit(cfg, GraphTrait::entry(cfg), this->_components);
    this->_dfn_table.reset();
    this->_stack.reset();
    this->build_nesting();
  }

  /// \brief Copy constructor
  Wto(const Wto& other)
      : _components(other._components), _nesting_table(other._nesting_table) {}

  /// \brief Move constructor
  Wto(Wto&& other)
      : _components(std::move(other._components)),
        _nesting_table(std::move(other._nesting_table)) {}

  /// \brief Copy assignment operator
  Wto& operator=(const Wto& other) {
    this->_components = other._components;
    this->_nesting_table = other._nesting_table;
    return *this;
  }

  /// \brief Move assignment operator
  Wto& operator=(Wto&& other) {
    this->_components = std::move(other._components);
    this->_nesting_table = std::move(other._nesting_table);
    return *this;
  }

  /// \brief Destructor
  ~Wto() = default;

  Iterator begin() const { return Iterator(this->_components, true); }

  Iterator end() const { return Iterator(this->_components, false); }

  WtoNestingT nesting(NodeRef n) const {
    auto it = this->_nesting_table->find(n);
    ikos_assert_msg(it != this->_nesting_table->end(), "node not found");
    return it->second;
  }

  void accept(WtoComponentVisitor< GraphRef, GraphTrait >& v) {
    for (Iterator it = this->begin(); it != this->end(); ++it) {
      it->accept(v);
    }
  }

  void dump(std::ostream& o) const {
    for (Iterator it = this->begin(); it != this->end();) {
      it->dump(o);
      ++it;
      if (it != this->end()) {
        o << " ";
      }
    }
  }

}; // end class Wto

} // end namespace core
} // end namespace ikos
