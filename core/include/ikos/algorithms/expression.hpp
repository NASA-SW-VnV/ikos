/**************************************************************************/ /**
 *
 * \file
 * \brief Class for arbitrary (non-linear) expressions.
 *
 * ikos uses mostly `linear_expression`, which is more efficient, where
 * `expression` is more generic.
 *
 * Author: Jorge A. Navas
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

#ifndef IKOS_EXPRESSION_HPP
#define IKOS_EXPRESSION_HPP

#include <memory>

#include <ikos/algorithms/patricia_trees.hpp>

namespace ikos {

template < typename VariableName, typename Number >
class expression_visitor;

/// \brief Base class for expression
template < typename VariableName, typename Number >
class _expression : public writeable {
public:
  typedef _expression< VariableName, Number > _expression_t;
  typedef std::shared_ptr< _expression_t > _expression_ptr;
  typedef patricia_tree_set< VariableName > variable_set_t;

public:
  typedef std::shared_ptr< expression_visitor< VariableName, Number > >
      expression_visitor_ptr;

protected:
  _expression() {}

public:
  virtual ~_expression() {}

public:
  virtual void write(std::ostream& o) = 0;
  virtual variable_set_t variables() = 0;
  virtual void accept(expression_visitor_ptr visitor) = 0;

}; // end class _expression

/// \brief Number expression
template < typename VariableName, typename Number >
class expression_number : public _expression< VariableName, Number > {
public:
  typedef typename _expression< VariableName, Number >::expression_visitor_ptr
      expression_visitor_ptr;

private:
  typedef typename _expression< VariableName, Number >::variable_set_t
      variable_set_t;

private:
  Number _n;

public:
  expression_number(Number n) : _expression< VariableName, Number >(), _n(n) {}

  void write(std::ostream& o) { o << this->_n; }

  variable_set_t variables() {
    variable_set_t vars;
    return vars;
  }

  void accept(expression_visitor_ptr visitor) { visitor->visit(*this); }

}; // end class expression_number

/// \brief Variable expression
template < typename VariableName, typename Number >
class expression_variable : public _expression< VariableName, Number > {
public:
  typedef typename _expression< VariableName, Number >::expression_visitor_ptr
      expression_visitor_ptr;

private:
  typedef typename _expression< VariableName, Number >::variable_set_t
      variable_set_t;

private:
  VariableName _v;

public:
  expression_variable(VariableName v)
      : _expression< VariableName, Number >(), _v(v) {}

  void write(std::ostream& o) { o << this->_v; }

  VariableName name() { return this->_v; }

  variable_set_t variables() {
    variable_set_t vars;
    vars += this->_v;
    return vars;
  }

  void accept(expression_visitor_ptr visitor) { visitor->visit(*this); }

}; // end class expression_variable

typedef enum {
  add,
  sub,
  mul,
  udiv,
  sdiv,
  urem,
  srem,
  eq,
  ne,
  ult,
  ule,
  slt,
  sle,
  fadd,
  fsub,
  fmul,
  fdiv,
  frem,
  oeq,
  olt,
  ole,
  one,
  ord,
  ueq,
  _ult,
  _ule,
  une,
  uno,
  _shl,
  _lshr,
  _ashr,
  _and,
  _or,
  _xor
} expression_op_t;

/// \brief Binary operation of expressions
template < typename VariableName, typename Number >
class expression_binary_op : public _expression< VariableName, Number > {
public:
  typedef typename _expression< VariableName, Number >::expression_visitor_ptr
      expression_visitor_ptr;

private:
  typedef typename _expression< VariableName, Number >::_expression_ptr
      _expression_ptr;
  typedef typename _expression< VariableName, Number >::variable_set_t
      variable_set_t;

private:
  _expression_ptr _left;
  _expression_ptr _right;
  expression_op_t _op;

public:
  expression_binary_op(expression_op_t op,
                       _expression_ptr left,
                       _expression_ptr right)
      : _expression< VariableName, Number >(),
        _left(left),
        _right(right),
        _op(op) {}

  variable_set_t variables() {
    variable_set_t vars;
    vars = this->_left->variables() | this->_right->variables();
    return vars;
  }

  expression_op_t op() { return this->_op; }

  virtual void accept(expression_visitor_ptr visitor) {
    this->_left->accept(visitor);
    visitor->visit(*this);
    this->_right->accept(visitor);
  }

  void write(std::ostream& o) {
    switch (this->_op) {
      case add:
        o << *(this->_left) << " + " << *(this->_right);
        break;
      case sub:
        o << *(this->_left) << " - " << *(this->_right);
        break;
      case mul:
        o << *(this->_left) << " * " << *(this->_right);
        break;
      case sdiv:
        o << *(this->_left) << " / " << *(this->_right);
        break;
      case udiv:
        o << *(this->_left) << " / " << *(this->_right);
        break;
      default:
        ikos_unreachable("unexpected operation");
    }
  }
}; // end expression_binary_op

/// \brief Class for arbitrary expression
template < typename VariableName, typename Number >
class expression : public writeable {
public:
  typedef expression< VariableName, Number > expression_t;

public:
  typedef typename _expression< VariableName, Number >::variable_set_t
      variable_set_t;

private:
  typedef expression_variable< VariableName, Number > expression_var_t;
  typedef expression_number< VariableName, Number > expression_number_t;
  typedef expression_binary_op< VariableName, Number > expression_binary_op_t;

private:
  typedef typename _expression< VariableName, Number >::_expression_ptr
      _expression_ptr;

private:
  _expression_ptr _ptr;

private:
  expression(_expression_ptr ptr) : _ptr(ptr) {}

public:
  static expression_t variable(VariableName v) {
    return expression_t(_expression_ptr(new expression_var_t(v)));
  }

  static expression_t number(Number n) {
    return expression_t(_expression_ptr(new expression_number_t(n)));
  }

public:
  expression_t combine(expression_op_t op, const expression_t& other) {
    return expression_t(_expression_ptr(
        new expression_binary_op_t(op, this->_ptr, other._ptr)));
  }

  variable_set_t variables() { return this->_ptr->variables(); }

  void accept(
      std::shared_ptr< expression_visitor< VariableName, Number > > visitor) {
    this->_ptr->accept(visitor);
  }

  void write(std::ostream& o) { this->_ptr->write(o); }

}; // end class expression

template < typename VariableName, typename Number >
inline expression< VariableName, Number > operator+(
    expression< VariableName, Number > x,
    expression< VariableName, Number > y) {
  return x.combine(add, y);
}

template < typename VariableName, typename Number >
inline expression< VariableName, Number > operator-(
    expression< VariableName, Number > x,
    expression< VariableName, Number > y) {
  return x.combine(sub, y);
}

template < typename VariableName, typename Number >
inline expression< VariableName, Number > operator*(
    expression< VariableName, Number > x,
    expression< VariableName, Number > y) {
  return x.combine(mul, y);
}

template < typename VariableName, typename Number >
inline expression< VariableName, Number > operator/(
    expression< VariableName, Number > x,
    expression< VariableName, Number > y) {
  return x.combine(sdiv, y);
}

/// \brief Expression visitor
template < typename VariableName, typename Number >
class expression_visitor {
private:
  template < typename Any1, typename Any2 >
  friend class expression;

private:
  typedef _expression< VariableName, Number > _expression_t;

public:
  typedef expression< VariableName, Number > expression_t;

  typedef expression_binary_op< VariableName, Number > expression_binary_op_t;
  typedef expression_variable< VariableName, Number > expression_variable_t;
  typedef expression_number< VariableName, Number > expression_number_t;

public:
  virtual ~expression_visitor() {}
  virtual void visit(expression_t&) = 0;
  virtual void visit(expression_binary_op_t&) = 0;
  virtual void visit(expression_variable_t&) = 0;
  virtual void visit(expression_number_t&) = 0;

private:
  void visit(_expression_t& /*exp*/) {}

}; // end class expression_visitor

} // end namespace

#endif // IKOS_EXPRESSION_HPP
