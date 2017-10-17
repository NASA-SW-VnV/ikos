/*******************************************************************************
 *
 * muAZ is a micro language for semantic modelling of arrays and
 * integer numbers.
 *
 * Author: Jorge A. Navas
 *
 * Contact: ikos@lists.nasa.gov
 *
 * The CFG can be also reversed which is useful for backward dataflow
 * analysis.
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

#ifndef ANALYZER_MUAZ_HPP
#define ANALYZER_MUAZ_HPP

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/flyweight.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <ikos/algorithms/linear_constraints.hpp>
#include <ikos/common/types.hpp>
#include <ikos/number/z_number.hpp>

// namespace boost
// {
//   // To use flyweight as boost::unordered_map keys
//   std::size_t hash_value(boost::flyweight<std::string> s)
//   {
//     return std::hash< std::string >()(s.get);
//   }
// }

namespace ikos {
namespace muaz {

template < typename VariableName, typename CheckPointName >
class statement_visitor;

template < typename VariableName, typename CheckPointName >
class statement : public writeable {
public:
  virtual void accept(statement_visitor< VariableName, CheckPointName >*) = 0;

  virtual ~statement() {}

}; // class statement

template < typename Number, typename VariableName, typename CheckPointName >
class binary_operation : public writeable {
public:
  typedef variable< Number, VariableName > variable_t;
  typedef binary_operation< Number, VariableName, CheckPointName >
      binary_operation_t;

private:
  variable_t _lhs;
  operation_t _op;
  variable_t _op1;
  variable_t _op2;

private:
  binary_operation();

public:
  binary_operation(variable_t lhs,
                   operation_t op,
                   variable_t op1,
                   variable_t op2)
      : _lhs(lhs), _op(op), _op1(op1), _op2(op2) {}

  variable_t lhs() { return this->_lhs; }

  operation_t operation() { return this->_op; }

  variable_t left_operand() { return this->_op1; }

  variable_t right_operand() { return this->_op2; }

  void write(std::ostream& o) {
    o << this->_lhs << " = " << this->_op1;
    switch (this->_op) {
      case OP_ADDITION: {
        o << "+";
        break;
      }
      case OP_MULTIPLICATION: {
        o << "*";
        break;
      }
      case OP_SUBTRACTION: {
        o << "-";
        break;
      }
      case OP_DIVISION: {
        o << "/";
        break;
      }
    }
    o << this->_op2;
  }

}; // class binary_operation

template < typename VariableName, typename CheckPointName >
class z_binary_operation : public statement< VariableName, CheckPointName > {
public:
  typedef variable< z_number, VariableName > variable_t;
  typedef binary_operation< z_number, VariableName, CheckPointName >
      z_binary_operation_t;

private:
  z_binary_operation_t _stmt;

private:
  z_binary_operation();

public:
  z_binary_operation(variable_t lhs,
                     operation_t op,
                     variable_t op1,
                     variable_t op2)
      : _stmt(lhs, op, op1, op2) {}

  void accept(statement_visitor< VariableName, CheckPointName >* v) {
    v->visit(this->_stmt);
  }

  void write(std::ostream& o) { this->_stmt.write(o); }

}; // class z_binary_operation

template < typename Number, typename VariableName, typename CheckPointName >
class linear_assignment : public writeable {
public:
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_assignment< Number, VariableName, CheckPointName >
      linear_assignment_t;

private:
  variable_t _lhs;
  linear_expression_t _rhs;

private:
  linear_assignment();

public:
  linear_assignment(variable_t lhs, linear_expression_t rhs)
      : _lhs(lhs), _rhs(rhs) {}

  variable_t lhs() { return this->_lhs; }

  linear_expression_t rhs() { return this->_rhs; }

  void write(std::ostream& o) { o << this->_lhs << " = " << this->_rhs; }

}; // class linear_assignment

template < typename VariableName, typename CheckPointName >
class z_linear_assignment : public statement< VariableName, CheckPointName > {
public:
  typedef variable< z_number, VariableName > variable_t;
  typedef linear_expression< z_number, VariableName > z_linear_expression_t;
  typedef linear_assignment< z_number, VariableName, CheckPointName >
      z_linear_assignment_t;

private:
  z_linear_assignment_t _stmt;

private:
  z_linear_assignment();

public:
  z_linear_assignment(variable_t lhs, z_linear_expression_t rhs)
      : _stmt(lhs, rhs) {}

  void accept(statement_visitor< VariableName, CheckPointName >* v) {
    v->visit(this->_stmt);
  }

  void write(std::ostream& o) { this->_stmt.write(o); }

}; // class z_linear_assignment

template < typename Number, typename VariableName, typename CheckPointName >
class linear_assertion : public writeable {
public:
  typedef variable< Number, VariableName > variable_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_assertion< Number, VariableName, CheckPointName >
      linear_assertion_t;

private:
  linear_constraint_t _cst;

private:
  linear_assertion();

public:
  linear_assertion(linear_constraint_t cst) : _cst(cst) {}

  linear_constraint_t constraint() { return this->_cst; }

  void write(std::ostream& o) { o << "assert(" << _cst << ")"; }

}; // class linear_assertion

template < typename VariableName, typename CheckPointName >
class z_linear_assertion : public statement< VariableName, CheckPointName > {
public:
  typedef variable< z_number, VariableName > variable_t;
  typedef linear_constraint< z_number, VariableName > z_linear_constraint_t;
  typedef linear_assertion< z_number, VariableName, CheckPointName >
      z_linear_assertion_t;

private:
  z_linear_assertion_t _stmt;

private:
  z_linear_assertion();

public:
  z_linear_assertion(z_linear_constraint_t cst) : _stmt(cst) {}

  void accept(statement_visitor< VariableName, CheckPointName >* v) {
    v->visit(this->_stmt);
  }

  void write(std::ostream& o) { this->_stmt.write(o); }

}; // class z_linear_assertion

template < typename VariableName, typename CheckPointName >
class checkpoint : public statement< VariableName, CheckPointName > {
private:
  CheckPointName _name;

public:
  typedef checkpoint< VariableName, CheckPointName > checkpoint_t;

public:
  checkpoint(CheckPointName name) : _name(name) {}

  CheckPointName name() { return this->_name; }

  void accept(statement_visitor< VariableName, CheckPointName >* v) {
    v->visit(*this);
  }

  void write(std::ostream& o) { o << "checkpoint " << this->_name; }

}; // class checkpoint

template < typename Number, typename VariableName, typename CheckPointName >
class array_write : public writeable {
public:
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef array_write< Number, VariableName, CheckPointName > array_write_t;

private:
  variable_t _array;
  linear_expression_t _index;
  linear_expression_t _val;

private:
  array_write();

public:
  array_write(variable_t array,
              linear_expression_t index,
              linear_expression_t val)
      : _array(array), _index(index), _val(val) {}

  variable_t array() { return this->_array; }

  linear_expression_t index() { return this->_index; }

  linear_expression_t val() { return this->_val; }

  void write(std::ostream& o) {
    o << this->_array << "[" << this->_index << "]"
      << " = " << this->_val;
  }

}; // class array_write

template < typename VariableName, typename CheckPointName >
class z_array_write : public statement< VariableName, CheckPointName > {
public:
  typedef variable< z_number, VariableName > variable_t;
  typedef linear_expression< z_number, VariableName > z_linear_expression_t;
  typedef array_write< z_number, VariableName, CheckPointName > z_array_write_t;

private:
  z_array_write_t _stmt;

private:
  z_array_write();

public:
  z_array_write(variable_t array,
                z_linear_expression_t index,
                z_linear_expression_t value)
      : _stmt(array, index, value) {}

  void accept(statement_visitor< VariableName, CheckPointName >* v) {
    v->visit(this->_stmt);
  }

  void write(std::ostream& o) { this->_stmt.write(o); }

}; // class z_array_write

template < typename Number, typename VariableName, typename CheckPointName >
class array_read : public writeable {
public:
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef array_read< Number, VariableName, CheckPointName > array_write_t;

private:
  variable_t _lhs;
  variable_t _array;
  linear_expression_t _index;

private:
  array_read();

public:
  array_read(variable_t lhs, variable_t array, linear_expression_t index)
      : _lhs(lhs), _array(array), _index(index) {}

  variable_t lhs() { return this->_lhs; }

  variable_t array() { return this->_array; }

  linear_expression_t index() { return this->_index; }

  void write(std::ostream& o) {
    o << this->_lhs << " = " << this->_array << "[" << this->_index << "]";
  }

}; // class array_read

template < typename VariableName, typename CheckPointName >
class z_array_read : public statement< VariableName, CheckPointName > {
public:
  typedef variable< z_number, VariableName > variable_t;
  typedef linear_expression< z_number, VariableName > z_linear_expression_t;
  typedef array_read< z_number, VariableName, CheckPointName > z_array_read_t;

private:
  z_array_read_t _stmt;

private:
  z_array_read();

public:
  z_array_read(variable_t lhs, variable_t array, z_linear_expression_t index)
      : _stmt(lhs, array, index) {}

  void accept(statement_visitor< VariableName, CheckPointName >* v) {
    v->visit(this->_stmt);
  }

  void write(std::ostream& o) { this->_stmt.write(o); }

}; // class z_array_read

template < typename VariableName, typename CheckPointName >
class muaz_cfg;

template < typename VariableName, typename CheckPointName >
class basic_block : public writeable {
  friend class muaz_cfg< VariableName, CheckPointName >;

public:
  typedef statement< VariableName, CheckPointName > statement_t;
  typedef basic_block< VariableName, CheckPointName > basic_block_t;
  // typedef boost::flyweight< std::string > identifier_t;
  typedef std::string identifier_t;
  typedef std::unordered_set< identifier_t > identifier_set_t;
  typedef std::shared_ptr< identifier_set_t > identifier_set_ptr;
  typedef variable< z_number, VariableName > z_variable_t;
  typedef linear_expression< z_number, VariableName > z_linear_expression_t;
  typedef linear_constraint< z_number, VariableName > z_linear_constraint_t;

private:
  typedef std::shared_ptr< basic_block_t > basic_block_ptr;
  typedef std::shared_ptr< statement_t > statement_ptr;
  typedef std::vector< statement_ptr > stmt_list_t;
  typedef std::shared_ptr< stmt_list_t > stmt_list_ptr;

  // Here the type of statements
  typedef z_binary_operation< VariableName, CheckPointName >
      z_binary_operation_t;
  typedef std::shared_ptr< z_binary_operation_t > z_binary_operation_ptr;
  typedef z_linear_assignment< VariableName, CheckPointName >
      z_linear_assignment_t;
  typedef std::shared_ptr< z_linear_assignment_t > z_linear_assignment_ptr;
  typedef z_linear_assertion< VariableName, CheckPointName >
      z_linear_assertion_t;
  typedef std::shared_ptr< z_linear_assertion_t > z_linear_assertion_ptr;
  typedef checkpoint< VariableName, CheckPointName > checkpoint_t;
  typedef std::shared_ptr< checkpoint_t > checkpoint_ptr;
  typedef z_array_write< VariableName, CheckPointName > z_array_write_t;
  typedef std::shared_ptr< z_array_write_t > z_array_write_ptr;
  typedef z_array_read< VariableName, CheckPointName > z_array_read_t;
  typedef std::shared_ptr< z_array_read_t > z_array_read_ptr;

private:
  identifier_t _name;
  stmt_list_ptr _stmts;
  identifier_set_ptr _prev, _next;

public:
  class iterator : public boost::iterator_facade< iterator,
                                                  statement_t&,
                                                  boost::forward_traversal_tag,
                                                  statement_t& > {
    friend class boost::iterator_core_access;

  private:
    typename stmt_list_t::iterator _it;
    stmt_list_ptr _l;

  public:
    iterator(stmt_list_ptr l, bool b) : _it(b ? l->begin() : l->end()), _l(l) {}

  private:
    void increment() { ++(this->_it); }

    bool equal(const iterator& other) const {
      return (this->_l == other._l && this->_it == other._it);
    }

    statement_t& dereference() const {
      if (this->_it != this->_l->end()) {
        return **(this->_it);
      } else {
        throw logic_error(
            "muAZ basic block: trying to dereference an empty iterator");
      }
    }

  }; // class iterator

private:
  basic_block();

  basic_block(identifier_t name)
      : _name(name),
        _stmts(stmt_list_ptr(new stmt_list_t)),
        _prev(identifier_set_ptr(new identifier_set_t)),
        _next(identifier_set_ptr(new identifier_set_t)) {}

public:
  basic_block(const basic_block_t& other)
      : _name(other._name),
        _stmts(stmt_list_ptr(new stmt_list_t(*(other._stmts)))),
        _prev(identifier_set_ptr(new identifier_set_t(*(other._prev)))),
        _next(identifier_set_ptr(new identifier_set_t(*(other._next)))) {}

private:
  void add(statement_ptr stmt) { this->_stmts->push_back(stmt); }

private:
  static basic_block_ptr make(identifier_t name) {
    return basic_block_ptr(new basic_block_t(name));
  }

public:
  identifier_t name() { return this->_name; }

  iterator begin() { return iterator(this->_stmts, true); }

  iterator end() { return iterator(this->_stmts, false); }

  size_t size() { return this->_stmts->size(); }

  identifier_set_ptr next_blocks() { return this->_next; }

  identifier_set_ptr prev_blocks() { return this->_prev; }

  void reverse() {
    std::swap(this->_prev, this->_next);
    std::reverse(this->_stmts->begin(), this->_stmts->end());
  }

  void operator>>(basic_block_t& b) {
    this->_next->insert(b._name);
    b._prev->insert(this->_name);
  }

  void write(std::ostream& o) {
    o << this->_name << ":" << std::endl;
    for (iterator it = this->begin(); it != this->end(); ++it) {
      statement_t& stmt = *it;
      o << "  " << stmt << ";" << std::endl;
    }
    o << "--> [";
    for (typename identifier_set_t::iterator it = this->_next->begin();
         it != this->_next->end();) {
      identifier_t id = *it;
      o << id;
      ++it;
      if (it != this->_next->end()) {
        o << ", ";
      }
    }
    o << "]" << std::endl;
  }

  void add(z_variable_t lhs, z_variable_t op1, z_variable_t op2) {
    this->add(std::static_pointer_cast< statement_t, z_binary_operation_t >(
        z_binary_operation_ptr(
            new z_binary_operation_t(lhs, OP_ADDITION, op1, op2))));
  }

  void sub(z_variable_t lhs, z_variable_t op1, z_variable_t op2) {
    this->add(std::static_pointer_cast< statement_t, z_binary_operation_t >(
        z_binary_operation_ptr(
            new z_binary_operation_t(lhs, OP_SUBTRACTION, op1, op2))));
  }

  void mul(z_variable_t lhs, z_variable_t op1, z_variable_t op2) {
    this->add(std::static_pointer_cast< statement_t, z_binary_operation_t >(
        z_binary_operation_ptr(
            new z_binary_operation_t(lhs, OP_MULTIPLICATION, op1, op2))));
  }

  void div(z_variable_t lhs, z_variable_t op1, z_variable_t op2) {
    this->add(std::static_pointer_cast< statement_t, z_binary_operation_t >(
        z_binary_operation_ptr(
            new z_binary_operation_t(lhs, OP_DIVISION, op1, op2))));
  }

  void assign(z_variable_t lhs, z_linear_expression_t rhs) {
    this->add(std::static_pointer_cast< statement_t, z_linear_assignment_t >(
        z_linear_assignment_ptr(new z_linear_assignment_t(lhs, rhs))));
  }

  void write(z_variable_t array,
             z_linear_expression_t index,
             z_linear_expression_t value) {
    this->add(std::static_pointer_cast< statement_t, z_array_write_t >(
        z_array_write_ptr(new z_array_write_t(array, index, value))));
  }

  void read(z_variable_t lhs, z_variable_t array, z_linear_expression_t index) {
    this->add(std::static_pointer_cast< statement_t, z_array_read_t >(
        z_array_read_ptr(new z_array_read_t(lhs, array, index))));
  }

  void assertion(z_linear_constraint_t cst) {
    this->add(std::static_pointer_cast< statement_t, z_linear_assertion_t >(
        z_linear_assertion_ptr(new z_linear_assertion_t(cst))));
  }

  void check(std::string name) {
    this->add(std::static_pointer_cast< statement_t, checkpoint_t >(
        checkpoint_ptr(new checkpoint_t(name))));
  }

}; // class basic_block

template < typename VariableName, typename CheckPointName >
class muaz_cfg : public writeable {
public:
  typedef statement< VariableName, CheckPointName > statement_t;
  typedef basic_block< VariableName, CheckPointName > basic_block_t;
  typedef typename basic_block_t::identifier_t basic_block_id_t;
  typedef collection< basic_block_id_t > node_collection_t;

private:
  typedef muaz_cfg< VariableName, CheckPointName > muaz_cfg_t;
  typedef std::shared_ptr< basic_block_t > basic_block_ptr;
  // std::unordered_map does not take flyweight's directly as keys
  // typedef std::unordered_map< basic_block_id_t, basic_block_ptr >
  // basic_block_map_t;
  typedef std::unordered_map< basic_block_id_t, basic_block_ptr >
      basic_block_map_t;
  typedef std::pair< basic_block_id_t, basic_block_ptr > binding_t;
  typedef std::shared_ptr< basic_block_map_t > basic_block_map_ptr;

private:
  basic_block_id_t _entry;
  basic_block_id_t _exit;
  basic_block_map_ptr _blocks;

public:
  class iterator : public boost::iterator_facade< iterator,
                                                  basic_block_t&,
                                                  boost::forward_traversal_tag,
                                                  basic_block_t& > {
    friend class boost::iterator_core_access;

  private:
    typedef std::vector< basic_block_id_t > id_list_t;
    typedef std::shared_ptr< id_list_t > id_list_ptr;
    typedef std::unordered_set< basic_block_id_t > id_set_t;
    typedef std::shared_ptr< id_set_t > id_set_ptr;

  private:
    basic_block_map_ptr _blocks;
    id_list_ptr _ids;
    typename id_list_t::iterator _it;
    id_set_ptr _seen_ids;
    bool _end;

  private:
    void scan(basic_block_id_t id) {
      if (this->_seen_ids->find(id) == this->_seen_ids->end()) {
        this->_seen_ids->insert(id);
        this->_ids->push_back(id);
        typename basic_block_t::identifier_set_ptr next_blocks =
            this->_blocks->find(id)->second->next_blocks();
        for (typename basic_block_t::identifier_set_t::iterator it =
                 next_blocks->begin();
             it != next_blocks->end();
             ++it) {
          this->scan(*it);
        }
      }
    }

  public:
    iterator(basic_block_id_t entry, basic_block_map_ptr blocks, bool b)
        : _blocks(blocks), _end(!b) {
      if (b) {
        this->_ids = id_list_ptr(new id_list_t);
        this->_seen_ids = id_set_ptr(new id_set_t);
        this->scan(entry);
        this->_seen_ids.reset();
        this->_it = this->_ids->begin();
      }
    }

  private:
    void increment() { ++(this->_it); }

    bool is_end() const {
      return (this->_end || (this->_ids && this->_it == this->_ids->end()));
    }

    bool equal(const iterator& other) const {
      return (this->is_end() && other.is_end());
    }

    basic_block_t& dereference() const {
      if (this->_it != this->_ids->end()) {
        return *(this->_blocks->find(*(this->_it))->second);
      } else {
        throw logic_error("muAZ cfg: trying to dereference an empty iterator");
      }
    }
  }; // class iterator

private:
  muaz_cfg();

public:
  muaz_cfg(std::string entry, std::string exit)
      : _entry(entry),
        _exit(exit),
        _blocks(basic_block_map_ptr(new basic_block_map_t)) {
    this->_blocks->insert(
        binding_t(this->_entry, basic_block_t::make(this->_entry)));
  }

  muaz_cfg(const muaz_cfg_t& other)
      : _entry(other._entry),
        _exit(other._exit),
        _blocks(basic_block_map_ptr(new basic_block_map_t)) {
    for (typename basic_block_map_t::iterator it = other._blocks->begin();
         it != other._blocks->end();
         ++it) {
      this->_blocks->insert(
          binding_t(it->first,
                    basic_block_ptr(new basic_block_t(*(it->second)))));
    }
  }

  basic_block_id_t entry() { return this->_entry; }

  basic_block_id_t exit() { return this->_exit; }

  // return true if the cfg is reversible
  bool reverse() {
    swap(this->_entry, this->_exit);
    for (typename basic_block_map_t::iterator it = this->_blocks->begin();
         it != this->_blocks->end();
         ++it) {
      it->second->reverse();
    }
    return true;
  }

  // for ikos fixpoint
  basic_block_t& get_node(basic_block_id_t name_id) {
    typename basic_block_map_t::iterator it = this->_blocks->find(name_id);
    if (it == this->_blocks->end()) {
      std::ostringstream s;
      s << "muAZ cfg: basic block " << name_id << " does not exist";
      throw logic_error(s.str());
    } else
      return *(it->second);
  }

  // for ikos fixpoint
  node_collection_t next_nodes(basic_block_id_t name_id) {
    basic_block_t& b = this->get_node(name_id);
    typename basic_block_t::identifier_set_ptr blocks = b.next_blocks();
    node_collection_t nodes;
    for (typename basic_block_t::identifier_set_t::iterator it =
             blocks->begin();
         it != blocks->end();
         ++it) {
      nodes += *it;
    }
    return nodes;
  }

  // for ikos fixpoint
  node_collection_t prev_nodes(basic_block_id_t name_id) {
    basic_block_t& b = this->get_node(name_id);
    typename basic_block_t::identifier_set_ptr blocks = b.prev_blocks();
    node_collection_t nodes;
    for (typename basic_block_t::identifier_set_t::iterator it =
             blocks->begin();
         it != blocks->end();
         ++it) {
      nodes += *it;
    }
    return nodes;
  }

  //  // for bgl
  // std::pair<typename std::vector<basic_block_id_t>::iterator,
  //           typename std::vector<basic_block_id_t>::iterator>
  // preds (basic_block_id_t bb_id)
  // {
  //   basic_block_t& bb = this->get_node(bb_id);
  //   typename basic_block_t::identifier_set_ptr blocks = bb.prev_blocks();
  //   std::vector<basic_block_id_t> nodes;
  //   for (typename basic_block_t::identifier_set_t::iterator it =
  //   blocks->begin(); it != blocks->end(); ++it)
  //     nodes.push_back (*it);
  //   return std::make_pair (nodes.begin (), nodes.end ());
  // }

  //  // for bgl
  // std::pair<typename std::vector<basic_block_id_t>::iterator,
  //           typename std::vector<basic_block_id_t>::iterator>
  // succs (basic_block_id_t bb_id)
  // {
  //   basic_block_t& bb = this->get_node(bb_id);
  //   typename basic_block_t::identifier_set_ptr blocks = bb.next_blocks();
  //   std::vector<basic_block_id_t> nodes;
  //   for (typename basic_block_t::identifier_set_t::iterator it =
  //   blocks->begin(); it != blocks->end(); ++it)
  //     nodes.push_back (*it);
  //   return std::make_pair (nodes.begin (), nodes.end ());
  // }

  // for bgl
  std::vector< basic_block_id_t > preds(basic_block_id_t bb_id) {
    basic_block_t& bb = this->get_node(bb_id);
    typename basic_block_t::identifier_set_ptr blocks = bb.prev_blocks();
    std::vector< basic_block_id_t > nodes;
    for (typename basic_block_t::identifier_set_t::iterator it =
             blocks->begin();
         it != blocks->end();
         ++it)
      nodes.push_back(*it);
    return nodes;
  }

  // for bgl
  std::vector< basic_block_id_t > succs(basic_block_id_t bb_id) {
    basic_block_t& bb = this->get_node(bb_id);
    typename basic_block_t::identifier_set_ptr blocks = bb.next_blocks();
    std::vector< basic_block_id_t > nodes;
    for (typename basic_block_t::identifier_set_t::iterator it =
             blocks->begin();
         it != blocks->end();
         ++it)
      nodes.push_back(*it);
    return nodes;
  }

  basic_block_t& insert_basic_block(std::string name) {
    basic_block_id_t name_id(name);
    typename basic_block_map_t::iterator it = this->_blocks->find(name_id);
    if (it == this->_blocks->end()) {
      basic_block_ptr block = basic_block_t::make(name_id);
      this->_blocks->insert(binding_t(name_id, block));
      return *block;
    } else
      return *(it->second);
  }

  iterator begin() { return iterator(this->_entry, this->_blocks, true); }

  iterator end() { return iterator(this->_entry, this->_blocks, false); }

  size_t size() { return this->_blocks->size(); }

  void write(std::ostream& o) {
    for (iterator it = this->begin(); it != this->end(); ++it)
      o << *it << std::endl;
  }

}; // class muaz_cfg

template < typename VariableName, typename CheckPointName >
class statement_visitor {
public:
  typedef binary_operation< z_number, VariableName, CheckPointName >
      z_binary_operation_t;
  typedef linear_assignment< z_number, VariableName, CheckPointName >
      z_linear_assignment_t;
  typedef linear_assertion< z_number, VariableName, CheckPointName >
      z_linear_assertion_t;
  typedef array_write< z_number, VariableName, CheckPointName > z_array_write_t;
  typedef array_read< z_number, VariableName, CheckPointName > z_array_read_t;
  typedef checkpoint< VariableName, CheckPointName > checkpoint_t;

public:
  virtual void visit(z_binary_operation_t&) = 0;

  virtual void visit(z_linear_assignment_t&) = 0;

  virtual void visit(z_linear_assertion_t&) = 0;

  virtual void visit(z_array_write_t&) = 0;

  virtual void visit(z_array_read_t&) = 0;

  virtual void visit(checkpoint_t&) = 0;

  virtual ~statement_visitor() {}

}; // class statement_visitor

} // namespace muaz
} // namespace ikos

#endif // ANALYZER_MUAZ_HPP
