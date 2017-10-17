/**************************************************************************/ /**
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

#ifndef IKOS_WTO_HPP
#define IKOS_WTO_HPP

#include <deque>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/container/slist.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <ikos/common/types.hpp>
#include <ikos/domains/intervals.hpp>

namespace ikos {

template < typename NodeName, typename CFG >
class wto;

template < typename NodeName, typename CFG >
class wto_vertex;

template < typename NodeName, typename CFG >
class wto_cycle;

template < typename NodeName, typename CFG >
class wto_component_visitor;

template < typename NodeName, typename CFG >
class wto_nesting : public writeable {
  friend class wto< NodeName, CFG >;
  friend class wto_vertex< NodeName, CFG >;
  friend class wto_cycle< NodeName, CFG >;

public:
  typedef wto_nesting< NodeName, CFG > wto_nesting_t;

private:
  typedef std::vector< NodeName > node_list_t;
  typedef std::shared_ptr< node_list_t > node_list_ptr;

private:
  node_list_ptr _nodes;

public:
  class iterator : public boost::iterator_facade< iterator,
                                                  NodeName&,
                                                  boost::forward_traversal_tag,
                                                  NodeName& > {
    friend class boost::iterator_core_access;

  private:
    typename node_list_t::iterator _it;
    node_list_ptr _l;

  public:
    iterator(node_list_ptr l, bool b) : _it(b ? l->begin() : l->end()), _l(l) {}

  private:
    void increment() { ++(this->_it); }

    bool equal(const iterator& other) const {
      return this->_l == other._l && this->_it == other._it;
    }

    NodeName& dereference() const {
      if (this->_it != this->_l->end()) {
        return *(this->_it);
      } else {
        throw logic_error(
            "WTO nesting: trying to dereference an empty iterator");
      }
    }

  }; // end class iterator

private:
  wto_nesting(node_list_ptr l) : _nodes(node_list_ptr(new node_list_t(*l))) {}

  int compare(wto_nesting_t& other) {
    iterator this_it = this->begin(), other_it = other.begin();
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
  wto_nesting() : _nodes(node_list_ptr(new node_list_t)) {}

  void operator+=(NodeName n) {
    this->_nodes = node_list_ptr(new node_list_t(*(this->_nodes)));
    this->_nodes->push_back(n);
  }

  wto_nesting_t operator+(NodeName n) {
    wto_nesting_t res(this->_nodes);
    res._nodes->push_back(n);
    return res;
  }

  iterator begin() { return iterator(this->_nodes, true); }

  iterator end() { return iterator(this->_nodes, false); }

  wto_nesting_t operator^(wto_nesting_t other) {
    wto_nesting_t res;
    for (iterator this_it = this->begin(), other_it = other.begin();
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

  bool operator<=(wto_nesting_t other) { return this->compare(other) <= 0; }

  bool operator==(wto_nesting_t other) { return this->compare(other) == 0; }

  bool operator>=(wto_nesting_t other) {
    return this->operator<=(other, *this);
  }

  bool operator>(wto_nesting_t other) { return this->compare(other) == 1; }

  void write(std::ostream& o) {
    o << "[";
    for (iterator it = this->begin(); it != this->end();) {
      NodeName n = *it;
      o << n;
      ++it;
      if (it != this->end()) {
        o << ", ";
      }
    }
    o << "]";
  }

}; // end class nesting

template < typename NodeName, typename CFG >
class wto_component : public writeable {
public:
  typedef wto_nesting< NodeName, CFG > wto_nesting_t;

public:
  virtual void accept(wto_component_visitor< NodeName, CFG >*) = 0;
  virtual ~wto_component() {}

}; // end class wto_component

template < typename NodeName, typename CFG >
class wto_vertex : public wto_component< NodeName, CFG > {
  friend class wto< NodeName, CFG >;

private:
  NodeName _node;

private:
  wto_vertex(NodeName node) : _node(node) {}

public:
  NodeName node() { return this->_node; }

  void accept(wto_component_visitor< NodeName, CFG >* v) { v->visit(*this); }

  void write(std::ostream& o) { o << this->_node; }

}; // end class wto_vertex

template < typename NodeName, typename CFG >
class wto_cycle : public wto_component< NodeName, CFG > {
  friend class wto< NodeName, CFG >;

public:
  typedef wto_component< NodeName, CFG > wto_component_t;

private:
  typedef std::shared_ptr< wto_component_t > wto_component_ptr;
  typedef boost::container::slist< wto_component_ptr > wto_component_list_t;
  typedef std::shared_ptr< wto_component_list_t > wto_component_list_ptr;

private:
  NodeName _head;
  wto_component_list_ptr _wto_components;

private:
  wto_cycle(NodeName head, wto_component_list_ptr wto_components)
      : _head(head), _wto_components(wto_components) {}

public:
  class iterator : public boost::iterator_facade< iterator,
                                                  wto_component_t&,
                                                  boost::forward_traversal_tag,
                                                  wto_component_t& > {
    friend class boost::iterator_core_access;

  private:
    typename wto_component_list_t::iterator _it;
    wto_component_list_ptr _l;

  public:
    iterator(wto_component_list_ptr l, bool b)
        : _it(b ? l->begin() : l->end()), _l(l) {}

  private:
    void increment() { ++(this->_it); }

    bool equal(const iterator& other) const {
      return this->_l == other._l && this->_it == other._it;
    }

    wto_component_t& dereference() const {
      if (this->_it != this->_l->end()) {
        return **(this->_it);
      } else {
        throw logic_error("WTO cycle: trying to dereference an empty iterator");
      }
    }

  }; // end class iterator

public:
  NodeName head() { return this->_head; }

  void accept(wto_component_visitor< NodeName, CFG >* v) { v->visit(*this); }

  iterator begin() { return iterator(this->_wto_components, true); }

  iterator end() { return iterator(this->_wto_components, false); }

  void write(std::ostream& o) {
    o << "(" << this->_head;
    if (!this->_wto_components->empty()) {
      o << " ";
      for (iterator it = this->begin(); it != this->end();) {
        wto_component_t& c = *it;
        o << c;
        ++it;
        if (it != this->end()) {
          o << " ";
        }
      }
    }
    o << ")";
  }

}; // end class wto_cycle

template < typename NodeName, typename CFG >
class wto_component_visitor {
public:
  typedef wto_vertex< NodeName, CFG > wto_vertex_t;
  typedef wto_cycle< NodeName, CFG > wto_cycle_t;

public:
  virtual void visit(wto_vertex_t&) = 0;
  virtual void visit(wto_cycle_t&) = 0;
  virtual ~wto_component_visitor() {}

}; // end class wto_component_visitor

template < typename NodeName, typename CFG >
class wto : public writeable {
public:
  typedef wto_nesting< NodeName, CFG > wto_nesting_t;
  typedef wto_component< NodeName, CFG > wto_component_t;
  typedef wto_vertex< NodeName, CFG > wto_vertex_t;
  typedef wto_cycle< NodeName, CFG > wto_cycle_t;
  typedef wto< NodeName, CFG > wto_t;

private:
  typedef std::shared_ptr< wto_component_t > wto_component_ptr;
  typedef std::shared_ptr< wto_vertex_t > wto_vertex_ptr;
  typedef std::shared_ptr< wto_cycle_t > wto_cycle_ptr;
  typedef boost::container::slist< wto_component_ptr > wto_component_list_t;
  typedef std::shared_ptr< wto_component_list_t > wto_component_list_ptr;
  typedef bound< z_number > dfn_t;
  typedef std::unordered_map< NodeName, dfn_t > dfn_table_t;
  typedef std::shared_ptr< dfn_table_t > dfn_table_ptr;
  typedef std::deque< NodeName > stack_t;
  typedef std::shared_ptr< stack_t > stack_ptr;
  typedef std::unordered_map< NodeName, wto_nesting_t > nesting_table_t;
  typedef std::shared_ptr< nesting_table_t > nesting_table_ptr;

private:
  wto_component_list_ptr _wto_components;
  dfn_table_ptr _dfn_table;
  dfn_t _num;
  stack_ptr _stack;
  nesting_table_ptr _nesting_table;

private:
  class nesting_builder : public wto_component_visitor< NodeName, CFG > {
  public:
    typedef wto_vertex< NodeName, CFG > wto_vertex_t;
    typedef wto_cycle< NodeName, CFG > wto_cycle_t;

  private:
    wto_nesting_t _nesting;
    nesting_table_ptr _nesting_table;

  public:
    nesting_builder(nesting_table_ptr nesting_table)
        : _nesting_table(nesting_table) {}

    void visit(wto_cycle_t& cycle) {
      NodeName head = cycle.head();
      wto_nesting_t previous_nesting = this->_nesting;
      this->_nesting_table->insert(std::make_pair(head, this->_nesting));
      this->_nesting += head;
      for (typename wto_cycle_t::iterator it = cycle.begin(); it != cycle.end();
           ++it) {
        it->accept(this);
      }
      this->_nesting = previous_nesting;
    }

    void visit(wto_vertex_t& vertex) {
      this->_nesting_table->insert(
          std::make_pair(vertex.node(), this->_nesting));
    }

  }; // end class nesting_builder

private:
  dfn_t get_dfn(NodeName n) {
    typename dfn_table_t::iterator it = this->_dfn_table->find(n);
    if (it == this->_dfn_table->end()) {
      return 0;
    } else {
      return it->second;
    }
  }

  void set_dfn(NodeName n, dfn_t dfn) {
    std::pair< typename dfn_table_t::iterator, bool > res =
        this->_dfn_table->insert(std::make_pair(n, dfn));
    if (!res.second) {
      (res.first)->second = dfn;
    }
  }

  NodeName pop() {
    if (this->_stack->empty()) {
      throw logic_error("WTO computation: empty stack");
    } else {
      NodeName top = this->_stack->back();
      this->_stack->pop_back();
      return top;
    }
  }

  void push(NodeName n) { this->_stack->push_back(n); }

  wto_cycle_ptr component(CFG& cfg, NodeName vertex) {
    wto_component_list_ptr partition(new wto_component_list_t);
    typename CFG::node_collection_t next_nodes = cfg.next_nodes(vertex);
    for (typename CFG::node_collection_t::iterator it = next_nodes.begin();
         it != next_nodes.end();
         ++it) {
      NodeName succ = *it;
      if (this->get_dfn(succ) == 0) {
        this->visit(cfg, succ, partition);
      }
    }
    return wto_cycle_ptr(new wto_cycle_t(vertex, partition));
  }

  dfn_t visit(CFG& cfg, NodeName vertex, wto_component_list_ptr partition) {
    dfn_t head = 0, min = 0;
    bool loop;

    this->push(vertex);
    this->_num += 1;
    head = this->_num;
    this->set_dfn(vertex, head);
    loop = false;
    typename CFG::node_collection_t next_nodes = cfg.next_nodes(vertex);
    for (typename CFG::node_collection_t::iterator it = next_nodes.begin();
         it != next_nodes.end();
         ++it) {
      NodeName succ = *it;
      dfn_t succ_dfn = this->get_dfn(succ);
      if (succ_dfn == 0) {
        min = this->visit(cfg, succ, partition);
      } else {
        min = succ_dfn;
      }
      if (min <= head) {
        head = min;
        loop = true;
      }
    }
    if (head == this->get_dfn(vertex)) {
      this->set_dfn(vertex, dfn_t::plus_infinity());
      NodeName element = this->pop();
      if (loop) {
        while (!(element == vertex)) {
          this->set_dfn(element, 0);
          element = this->pop();
        }
        partition->push_front(
            std::static_pointer_cast< wto_component_t, wto_cycle_t >(
                this->component(cfg, vertex)));
      } else {
        partition->push_front(
            std::static_pointer_cast< wto_component_t, wto_vertex_t >(
                wto_vertex_ptr(new wto_vertex_t(vertex))));
      }
    }
    return head;
  }

  void build_nesting() {
    nesting_builder builder(this->_nesting_table);
    for (iterator it = this->begin(); it != this->end(); ++it) {
      it->accept(&builder);
    }
  }

public:
  class iterator : public boost::iterator_facade< iterator,
                                                  wto_component_t&,
                                                  boost::forward_traversal_tag,
                                                  wto_component_t& > {
    friend class boost::iterator_core_access;

  private:
    typename wto_component_list_t::iterator _it;
    wto_component_list_ptr _l;

  public:
    iterator(wto_component_list_ptr l, bool b)
        : _it(b ? l->begin() : l->end()), _l(l) {}

  private:
    void increment() { ++(this->_it); }

    bool equal(const iterator& other) const {
      return this->_l == other._l && this->_it == other._it;
    }

    wto_component_t& dereference() const {
      if (this->_it != this->_l->end()) {
        return **(this->_it);
      } else {
        throw logic_error("WTO: trying to dereference an empty iterator");
      }
    }

  }; // end class iterator

public:
  wto(CFG& cfg)
      : _wto_components(wto_component_list_ptr(new wto_component_list_t)),
        _dfn_table(dfn_table_ptr(new dfn_table_t)),
        _num(0),
        _stack(stack_ptr(new stack_t)),
        _nesting_table(nesting_table_ptr(new nesting_table_t)) {
    this->visit(cfg, cfg.entry(), this->_wto_components);
    this->_dfn_table.reset();
    this->_stack.reset();
    this->build_nesting();
  }

  wto(const wto_t& other)
      : _wto_components(other._wto_components),
        _nesting_table(other._nesting_table) {}

  wto_t& operator=(wto_t other) {
    this->_wto_components = other._wto_components;
    this->_nesting_table = other._nesting_table;
    return *this;
  }

  iterator begin() { return iterator(this->_wto_components, true); }

  iterator end() { return iterator(this->_wto_components, false); }

  wto_nesting_t nesting(NodeName n) {
    typename nesting_table_t::iterator it = this->_nesting_table->find(n);
    if (it == this->_nesting_table->end()) {
      std::ostringstream buf;
      buf << "WTO nesting: node " << n << " not found";
      throw logic_error(buf.str());
    } else {
      return it->second;
    }
  }

  void accept(wto_component_visitor< NodeName, CFG >* v) {
    for (iterator it = this->begin(); it != this->end(); ++it) {
      it->accept(v);
    }
  }

  void write(std::ostream& o) {
    for (iterator it = this->begin(); it != this->end();) {
      wto_component_t& c = *it;
      o << c;
      ++it;
      if (it != this->end()) {
        o << " ";
      }
    }
  }

}; // end class wto

} // end namespace ikos

#endif // IKOS_WTO_HPP
