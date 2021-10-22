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
 * Contributor: Maxime Arthaud
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
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/container/slist.hpp>
#include <boost/iterator/transform_iterator.hpp>

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

/// \brief Helper for sequential containers of unique_ptr
template < typename T >
struct SeqExposeConstRef {
  const T& operator()(const std::unique_ptr< T >& p) const { return *p; }
};

/// \brief Represents the nesting of a node
///
/// The nesting of a node is the list of cycles containing the node, from
/// the outermost to the innermost.
template < typename GraphRef, typename GraphTrait >
class WtoNesting {
public:
  using NodeRef = typename GraphTrait::NodeRef;

private:
  using NodeList = std::vector< NodeRef >;

public:
  using Iterator = typename NodeList::const_iterator;

private:
  NodeList _nodes;

public:
  /// \brief Constructor
  WtoNesting() = default;

  /// \brief Copy constructor
  WtoNesting(const WtoNesting&) = default;

  /// \brief Move constructor
  WtoNesting(WtoNesting&&) = default;

  /// \brief Copy assignment operator
  WtoNesting& operator=(const WtoNesting&) = default;

  /// \brief Move assignment operator
  WtoNesting& operator=(WtoNesting&&) = default;

  /// \brief Destructor
  ~WtoNesting() = default;

  /// \brief Add a cycle head in the nesting
  void add(NodeRef head) { this->_nodes.push_back(head); }

  /// \brief Begin iterator over the head of cycles
  Iterator begin() const { return this->_nodes.cbegin(); }

  /// \brief End iterator over the head of cycles
  Iterator end() const { return this->_nodes.cend(); }

  /// \brief Return the common prefix of the given nestings
  WtoNesting operator^(const WtoNesting& other) const {
    WtoNesting res;
    for (auto this_it = this->begin(), other_it = other.begin();
         this_it != this->end() && other_it != other.end();
         ++this_it, ++other_it) {
      if (*this_it == *other_it) {
        res.add(*this_it);
      } else {
        break;
      }
    }
    return res;
  }

private:
  /// \brief Compare the given nestings
  int compare(const WtoNesting& other) const {
    if (this == &other) {
      return 0; // equals
    }

    auto this_it = this->begin();
    auto other_it = other.begin();
    while (this_it != this->end()) {
      if (other_it == other.end()) {
        return 1; // `this` is nested within `other`
      } else if (*this_it == *other_it) {
        ++this_it;
        ++other_it;
      } else {
        return 2; // not comparable
      }
    }
    if (other_it == other.end()) {
      return 0; // equals
    } else {
      return -1; // `other` is nested within `this`
    }
  }

public:
  bool operator<(const WtoNesting& other) const {
    return this->compare(other) == -1;
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

  /// \brief Dump the nesting, for debugging purpose
  void dump(std::ostream& o) const {
    o << "[";
    for (auto it = this->begin(), et = this->end(); it != et;) {
      DumpableTraits< NodeRef >::dump(o, *it);
      ++it;
      if (it != et) {
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
  /// \brief Default constructor
  WtoComponent() = default;

  /// \brief Copy constructor
  WtoComponent(const WtoComponent&) noexcept = default;

  /// \brief Move constructor
  WtoComponent(WtoComponent&&) noexcept = default;

  /// \brief Copy assignment operator
  WtoComponent& operator=(const WtoComponent&) noexcept = default;

  /// \brief Move assignment operator
  WtoComponent& operator=(WtoComponent&&) noexcept = default;

  /// \brief Accept the given visitor
  virtual void accept(WtoComponentVisitor< GraphRef, GraphTrait >&) const = 0;

  /// \brief Destructor
  virtual ~WtoComponent() = default;

}; // end class WtoComponent

/// \brief Represents a vertex
template < typename GraphRef, typename GraphTrait = GraphTraits< GraphRef > >
class WtoVertex final : public WtoComponent< GraphRef, GraphTrait > {
public:
  using NodeRef = typename GraphTrait::NodeRef;

private:
  NodeRef _node;

public:
  /// \brief Constructor
  explicit WtoVertex(NodeRef node) : _node(node) {}

  /// \brief Return the graph node
  NodeRef node() const { return this->_node; }

  /// \brief Accept the given visitor
  void accept(WtoComponentVisitor< GraphRef, GraphTrait >& v) const override {
    v.visit(*this);
  }

  /// \brief Dump the vertex, for debugging purpose
  void dump(std::ostream& o) const {
    DumpableTraits< NodeRef >::dump(o, this->_node);
  }

}; // end class WtoVertex

/// \brief Represents a cycle
template < typename GraphRef, typename GraphTrait = GraphTraits< GraphRef > >
class WtoCycle final : public WtoComponent< GraphRef, GraphTrait > {
public:
  using NodeRef = typename GraphTrait::NodeRef;
  using WtoComponentT = WtoComponent< GraphRef, GraphTrait >;

private:
  using WtoComponentPtr = std::unique_ptr< WtoComponentT >;
  using WtoComponentList = boost::container::slist< WtoComponentPtr >;

public:
  /// \brief Iterator over the components
  using Iterator =
      boost::transform_iterator< SeqExposeConstRef< WtoComponentT >,
                                 typename WtoComponentList::const_iterator >;

private:
  /// \brief Head of the cycle
  NodeRef _head;

  /// \brief List of components
  WtoComponentList _components;

public:
  /// \brief Constructor
  WtoCycle(NodeRef head, WtoComponentList components)
      : _head(head), _components(std::move(components)) {}

  /// \brief Return the head of the cycle
  NodeRef head() const { return this->_head; }

  /// \brief Begin iterator over the components
  Iterator begin() const {
    return boost::make_transform_iterator(this->_components.cbegin(),
                                          SeqExposeConstRef< WtoComponentT >());
  }

  /// \brief End iterator over the components
  Iterator end() const {
    return boost::make_transform_iterator(this->_components.cend(),
                                          SeqExposeConstRef< WtoComponentT >());
  }

  /// \brief Accept the given visitor
  void accept(WtoComponentVisitor< GraphRef, GraphTrait >& v) const override {
    v.visit(*this);
  }

  /// \brief Dump the cycle, for debugging purpose
  void dump(std::ostream& o) const {
    o << "(";
    DumpableTraits< NodeRef >::dump(o, this->_head);
    for (const auto& c : this->_components) {
      o << " ";
      c->dump(o);
    }
    o << ")";
  }

}; // end class WtoCycle

/// \brief Weak topological order visitor
template < typename GraphRef, typename GraphTrait = GraphTraits< GraphRef > >
class WtoComponentVisitor {
public:
  using WtoVertexT = WtoVertex< GraphRef, GraphTrait >;
  using WtoCycleT = WtoCycle< GraphRef, GraphTrait >;

public:
  /// \brief Default constructor
  WtoComponentVisitor() = default;

  /// \brief Copy constructor
  WtoComponentVisitor(const WtoComponentVisitor&) noexcept = default;

  /// \brief Move constructor
  WtoComponentVisitor(WtoComponentVisitor&&) noexcept = default;

  /// \brief Copy assignment operator
  WtoComponentVisitor& operator=(const WtoComponentVisitor&) noexcept = default;

  /// \brief Move assignment operator
  WtoComponentVisitor& operator=(WtoComponentVisitor&&) noexcept = default;

  /// \brief Visit the given vertex
  virtual void visit(const WtoVertexT&) = 0;

  /// \brief Visit the given cycle
  virtual void visit(const WtoCycleT&) = 0;

  /// \brief Destructor
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
  using WtoComponentPtr = std::unique_ptr< WtoComponentT >;
  using WtoComponentList = boost::container::slist< WtoComponentPtr >;
  using Dfn = Bound< ZNumber >;
  using DfnTable = std::unordered_map< NodeRef, Dfn >;
  using Stack = std::vector< NodeRef >;
  using WtoNestingPtr = std::shared_ptr< WtoNestingT >;
  using NestingTable = std::unordered_map< NodeRef, WtoNestingPtr >;

public:
  /// \brief Iterator over the components
  using Iterator =
      boost::transform_iterator< SeqExposeConstRef< WtoComponentT >,
                                 typename WtoComponentList::const_iterator >;

private:
  WtoComponentList _components;
  NestingTable _nesting_table;
  DfnTable _dfn_table;
  Dfn _num;
  Stack _stack;

private:
  /// \brief Visitor to build the nestings of each node
  class NestingBuilder final
      : public WtoComponentVisitor< GraphRef, GraphTrait > {
  private:
    WtoNestingPtr _nesting;
    NestingTable& _nesting_table;

  public:
    explicit NestingBuilder(NestingTable& nesting_table)
        : _nesting(std::make_shared< WtoNestingT >()),
          _nesting_table(nesting_table) {}

    void visit(const WtoCycleT& cycle) override {
      NodeRef head = cycle.head();
      WtoNestingPtr previous_nesting = this->_nesting;
      this->_nesting_table.insert(std::make_pair(head, this->_nesting));
      this->_nesting = std::make_shared< WtoNestingT >(*this->_nesting);
      this->_nesting->add(head);
      for (auto it = cycle.begin(), et = cycle.end(); it != et; ++it) {
        it->accept(*this);
      }
      this->_nesting = previous_nesting;
    }

    void visit(const WtoVertexT& vertex) override {
      this->_nesting_table.insert(
          std::make_pair(vertex.node(), this->_nesting));
    }

  }; // end class NestingBuilder

private:
  /// \brief Return the depth-first number of the given node
  Dfn dfn(NodeRef n) const {
    auto it = this->_dfn_table.find(n);
    if (it != this->_dfn_table.end()) {
      return it->second;
    } else {
      return Dfn(0);
    }
  }

  /// \brief Set the depth-first number of the given node
  void set_dfn(NodeRef n, const Dfn& dfn) {
    auto res = this->_dfn_table.insert(std::make_pair(n, dfn));
    if (!res.second) {
      (res.first)->second = dfn;
    }
  }

  /// \brief Pop a node from the stack
  NodeRef pop() {
    ikos_assert_msg(!this->_stack.empty(), "empty stack");
    NodeRef top = this->_stack.back();
    this->_stack.pop_back();
    return top;
  }

  /// \brief Push a node on the stack
  void push(NodeRef n) { this->_stack.push_back(n); }

  /// \brief Create the cycle component for the given vertex
  WtoComponentPtr component(NodeRef vertex) {
    WtoComponentList partition;
    for (auto it = GraphTrait::successor_begin(vertex),
              et = GraphTrait::successor_end(vertex);
         it != et;
         ++it) {
      NodeRef succ = *it;
      if (this->dfn(succ) == Dfn(0)) {
        this->visit(succ, partition);
      }
    }
    return std::make_unique< WtoCycleT >(vertex, std::move(partition));
  }

  /// \brief Visit the given node
  ///
  /// Algorithm to build a weak topological order of a graph
  Dfn visit(NodeRef vertex, WtoComponentList& partition) {
    Dfn head(0);
    Dfn min(0);
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
        min = this->visit(succ, partition);
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
        partition.push_front(this->component(vertex));
      } else {
        partition.push_front(std::make_unique< WtoVertexT >(vertex));
      }
    }
    return head;
  }

  /// \brief Build the nesting table
  void build_nesting() {
    NestingBuilder builder(this->_nesting_table);
    for (auto it = this->begin(), et = this->end(); it != et; ++it) {
      it->accept(builder);
    }
  }

public:
  /// \brief Compute the weak topological order of the given graph
  explicit Wto(GraphRef cfg) : _num(0) {
    this->visit(GraphTrait::entry(cfg), this->_components);
    this->_dfn_table.clear();
    this->_stack.clear();
    this->build_nesting();
  }

  /// \brief No copy constructor
  Wto(const Wto& other) = delete;

  /// \brief Move constructor
  Wto(Wto&& other) = default;

  /// \brief No copy assignment operator
  Wto& operator=(const Wto& other) = delete;

  /// \brief Move assignment operator
  Wto& operator=(Wto&& other) = default;

  /// \brief Destructor
  ~Wto() = default;

  /// \brief Begin iterator over the components
  Iterator begin() const {
    return boost::make_transform_iterator(this->_components.cbegin(),
                                          SeqExposeConstRef< WtoComponentT >());
  }

  /// \brief End iterator over the components
  Iterator end() const {
    return boost::make_transform_iterator(this->_components.cend(),
                                          SeqExposeConstRef< WtoComponentT >());
  }

  /// \brief Return the nesting of the given node
  const WtoNestingT& nesting(NodeRef n) const {
    auto it = this->_nesting_table.find(n);
    ikos_assert_msg(it != this->_nesting_table.end(), "node not found");
    return *(it->second);
  }

  /// \brief Accept the given visitor
  void accept(WtoComponentVisitor< GraphRef, GraphTrait >& v) {
    for (const auto& c : this->_components) {
      c->accept(v);
    }
  }

  /// \brief Dump the order, for debugging purpose
  void dump(std::ostream& o) const {
    for (auto it = this->begin(), et = this->end(); it != et;) {
      it->dump(o);
      ++it;
      if (it != et) {
        o << " ";
      }
    }
  }

}; // end class Wto

} // end namespace core
} // end namespace ikos
