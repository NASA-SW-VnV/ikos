/**************************************************************************/ /**
 *
 * \file
 * \brief Standard domain of Difference-Bound Matrices.
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Based on Antoine Mine's paper: A New Numerical Abstract Domain Based on
 * Difference-Bound Matrices, in PADO, 155-172, 2001.
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

#ifndef IKOS_DBM_HPP
#define IKOS_DBM_HPP

#include <ikos/common/types.hpp>
#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/domains/bitwise_operators_api.hpp>
#include <ikos/domains/division_operators_api.hpp>
#include <ikos/domains/intervals.hpp>
#include <ikos/domains/numerical_domains_api.hpp>
#include <ikos/value/congruence.hpp>

namespace ikos {

/// \brief Difference-Bound Matrices abstract domain
template < typename Number,
           typename VariableName,
           std::size_t max_reduction_cycles = 10 >
class dbm : public abstract_domain,
            public numerical_domain< Number, VariableName >,
            public bitwise_operators< Number, VariableName >,
            public division_operators< Number, VariableName > {
public:
  typedef bound< Number > bound_t;
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;
  typedef patricia_tree_set< variable_t > variable_set_t;
  typedef interval< Number > interval_t;
  typedef congruence< Number > congruence_t;
  typedef interval_domain< Number, VariableName > interval_domain_t;
  typedef dbm< Number, VariableName, max_reduction_cycles > dbm_t;

private:
  class dbmatrix : public writeable {
  private:
    std::vector< bound_t > _matrix; // Matrix of size _num_var * _num_var
    std::size_t _num_var;

  public:
    dbmatrix() : _num_var(0) {}

    dbmatrix(const dbmatrix& o) : _matrix(o._matrix), _num_var(o._num_var) {}

    dbmatrix& operator=(const dbmatrix& o) {
      _matrix = o._matrix;
      _num_var = o._num_var;
      return *this;
    }

    std::size_t num_var() const { return _num_var; }

    bound_t operator()(unsigned int i, unsigned int j) const {
      assert(i < _num_var && j < _num_var && "ouf of bounds error");
      return _matrix[_num_var * i + j];
    }

    bound_t& operator()(unsigned int i, unsigned int j) {
      assert(i < _num_var && j < _num_var && "ouf of bounds error");
      return _matrix[_num_var * i + j];
    }

    void clear() {
      _matrix.clear();
      _num_var = 0;
    }

    /// \brief Clear and resize the matrix
    void clear_resize(std::size_t num_var) {
      _matrix.clear();
      _matrix.resize(num_var * num_var, bound_t::plus_infinity());
      _num_var = num_var;
    }

    /// \brief Resize the matrix to manage a new variable.
    ///
    /// \returns the index of the new variable.
    unsigned int add_variable() {
      if (_num_var == 0) {
        _num_var = 2;
        _matrix.resize(_num_var * _num_var, bound_t::plus_infinity());
      } else {
        std::vector< bound_t > new_matrix((_num_var + 1) * (_num_var + 1),
                                          bound_t::plus_infinity());

        for (unsigned int i = 0; i < _num_var; i++) {
          for (unsigned int j = 0; j < _num_var; j++) {
            new_matrix[(_num_var + 1) * i + j] = _matrix[_num_var * i + j];
          }
        }

        std::swap(_matrix, new_matrix);
        _num_var++;
      }

      return _num_var - 1;
    }

    void write(std::ostream& o) {
      for (unsigned int i = 0; i < _num_var; i++) {
        for (unsigned int j = 0; j < _num_var; j++) {
          o << "M[" << i << ", " << j << "] = " << this->operator()(i, j)
            << "; ";
        }
        o << std::endl;
      }
    }

  }; // end class dbmatrix

private:
  typedef linear_interval_solver< Number, VariableName, dbm_t > solver_t;
  typedef boost::container::flat_map< VariableName, unsigned int >
      var_indexes_t;

private:
  bool _is_bottom;
  bool _is_normalized;
  dbmatrix _matrix;
  var_indexes_t _var_indexes;

private:
  dbm(bool is_top) : _is_bottom(!is_top), _is_normalized(true) {}

  void set_bottom() {
    _is_bottom = true;
    _is_normalized = true;
    _matrix.clear();
    _var_indexes.clear();
  }

public:
  void normalize() {
    if (_is_normalized)
      return;

    if (_is_bottom) {
      set_bottom();
      _is_normalized = true;
      return;
    }

    /* Floyd-Warshall algorithm */

    for (unsigned int i = 0; i < _matrix.num_var(); i++) {
      _matrix(i, i) = 0;
    }

    for (unsigned int k = 0; k < _matrix.num_var(); k++) {
      for (unsigned int i = 0; i < _matrix.num_var(); i++) {
        for (unsigned int j = 0; j < _matrix.num_var(); j++) {
          _matrix(i, j) =
              bound_t::min(_matrix(i, j), _matrix(i, k) + _matrix(k, j));
        }
      }
    }

    // Check for negative cycle
    for (unsigned int i = 0; i < _matrix.num_var(); i++) {
      if (_matrix(i, i) < 0) {
        set_bottom();
        _is_normalized = true;
        return;
      }
    }

    _is_normalized = true;
  }

private:
  /// \brief Get the index of variable x in _matrix
  ///
  /// Create a new one if not found
  unsigned int var_index(const VariableName& x) {
    if (_matrix.num_var() == 0) {
      return _var_indexes[x] = _matrix.add_variable();
    }

    typename var_indexes_t::iterator it = _var_indexes.find(x);
    if (it == _var_indexes.end()) {
      // look for an unused index in _matrix
      std::vector< bool > is_used(_matrix.num_var(), false);

      is_used[0] = true;
      for (typename var_indexes_t::iterator it = _var_indexes.begin();
           it != _var_indexes.end();
           ++it) {
        is_used[it->second] = true;
      }

      std::vector< bool >::const_iterator unused_index =
          std::find(is_used.begin(), is_used.end(), false);
      if (unused_index ==
          is_used.end()) { // no unused index found, we resize the matrix
        return _var_indexes[x] = _matrix.add_variable();
      } else {
        return _var_indexes[x] = unused_index - is_used.begin();
      }
    } else {
      return it->second;
    }
  }

  /// \brief Forget all informations about variable k
  void forget(unsigned int k) {
    // Use informations about k to improve all constraints
    if (!_is_normalized) { // Useless if already normalized
      for (unsigned int i = 0; i < _matrix.num_var(); i++) {
        bound_t w_i_k = _matrix(i, k);

        for (unsigned int j = 0; j < _matrix.num_var(); j++) {
          if (i != k && j != k) {
            _matrix(i, j) = bound_t::min(_matrix(i, j), w_i_k + _matrix(k, j));
          }
        }
      }
    }

    for (unsigned int i = 0; i < _matrix.num_var(); i++) {
      _matrix(i, k) = _matrix(k, i) = bound_t::plus_infinity();
    }
    _matrix(k, k) = 0;

    _is_normalized = false;
  }

  /// \brief Add constraint v_i - v_j <= c
  void add_constraint(unsigned int i, unsigned int j, bound_t c) {
    bound_t w = _matrix(j, i);
    if (c < w) {
      _matrix(j, i) = c;
      _is_normalized = false;
    }
  }

  /// \brief Apply v_i = v_i + c
  void increment(unsigned int i, Number c) {
    if (c == 0)
      return;

    for (unsigned int j = 0; j < _matrix.num_var(); j++) {
      if (i != j) {
        _matrix(i, j) = _matrix(i, j) - c;
        _matrix(j, i) = _matrix(j, i) + c;
      }
    }

    _is_normalized = false;
  }

  template < typename op_t >
  dbm_t pointwise_binary_op(const dbm_t& o) const {
    op_t op;
    dbm_t dbm;
    variable_set_t variables = this->variables() | o.variables();
    dbm._matrix.clear_resize(variables.size() + 1);

    // Assign indexes
    for (typename variable_set_t::iterator it = variables.begin();
         it != variables.end();
         ++it) {
      dbm._var_indexes.insert(
          typename var_indexes_t::value_type((*it).name(),
                                             dbm._var_indexes.size() + 1));
    }

    // Compute dbm._matrix
    for (typename var_indexes_t::iterator it = dbm._var_indexes.begin();
         it != dbm._var_indexes.end();
         ++it) {
      const VariableName& var_i = it->first;
      unsigned int this_i =
          this->_var_indexes.find(var_i) == this->_var_indexes.end()
              ? 0
              : this->_var_indexes.at(var_i);
      unsigned int o_i = o._var_indexes.find(var_i) == o._var_indexes.end()
                             ? 0
                             : o._var_indexes.at(var_i);
      unsigned int i = it->second;

      // special variable 0
      if (this_i > 0) {
        if (o_i > 0) {
          dbm._matrix(i, 0) = op(this->_matrix(this_i, 0), o._matrix(o_i, 0));
          dbm._matrix(0, i) = op(this->_matrix(0, this_i), o._matrix(0, o_i));
        } else {
          dbm._matrix(i, 0) = op(this->_matrix(this_i, 0));
          dbm._matrix(0, i) = op(this->_matrix(0, this_i));
        }
      } else {
        if (o_i > 0) {
          dbm._matrix(i, 0) = op(o._matrix(o_i, 0));
          dbm._matrix(0, i) = op(o._matrix(0, o_i));
        } else {
          dbm._matrix(i, 0) = op();
          dbm._matrix(0, i) = op();
        }
      }

      for (typename var_indexes_t::iterator it2 = dbm._var_indexes.begin();
           it2 != dbm._var_indexes.end();
           ++it2) {
        const VariableName& var_j = it2->first;
        unsigned int this_j =
            this->_var_indexes.find(var_j) == this->_var_indexes.end()
                ? 0
                : this->_var_indexes.at(var_j);
        unsigned int o_j = o._var_indexes.find(var_j) == o._var_indexes.end()
                               ? 0
                               : o._var_indexes.at(var_j);
        unsigned int j = it2->second;

        if (this_i > 0 && this_j > 0) { // _matrix(this_i, this_j) exists
          if (o_i > 0 && o_j > 0) {     // _matrix(o_i, o_j) exists
            dbm._matrix(i, j) =
                op(this->_matrix(this_i, this_j), o._matrix(o_i, o_j));
          } else {
            dbm._matrix(i, j) = op(this->_matrix(this_i, this_j));
          }
        } else {                    // _matrix(this_i, this_j) doesn't exist
          if (o_i > 0 && o_j > 0) { // _matrix(o_i, o_j) exists
            dbm._matrix(i, j) = op(o._matrix(o_i, o_j));
          } else {
            dbm._matrix(i, j) = op();
          }
        }
      }
    }

    for (unsigned int i = 0; i < dbm._matrix.num_var(); i++) {
      dbm._matrix(i, i) = 0;
    }

    dbm._is_normalized = false;
    return dbm;
  }

  // Helpers for lattice operations

  struct join_op {
    // Default value
    bound_t operator()() { return bound_t::plus_infinity(); }

    bound_t operator()(bound_t) { return bound_t::plus_infinity(); }

    bound_t operator()(bound_t v1, bound_t v2) { return bound_t::max(v1, v2); }
  };

  struct meet_op {
    // Default value
    bound_t operator()() { return bound_t::plus_infinity(); }

    bound_t operator()(bound_t v) { return v; }

    bound_t operator()(bound_t v1, bound_t v2) { return bound_t::min(v1, v2); }
  };

  struct widening_op {
    // Default value
    bound_t operator()() { return bound_t::plus_infinity(); }

    bound_t operator()(bound_t v) { return bound_t::plus_infinity(); }

    bound_t operator()(bound_t v1, bound_t v2) {
      if (v2 <= v1) {
        return v1;
      } else {
        return bound_t::plus_infinity();
      }
    }
  };

  struct narrowing_op {
    // Default value
    bound_t operator()() { return bound_t::plus_infinity(); }

    bound_t operator()(bound_t v) { return v; }

    bound_t operator()(bound_t v1, bound_t v2) {
      if (v1.is_plus_infinity()) {
        return v2;
      } else {
        return v1;
      }
    }
  };

public:
  static dbm_t top() { return dbm(true); }

  static dbm_t bottom() { return dbm(false); }

public:
  dbm() : _is_bottom(false), _is_normalized(true) {}

  dbm(const dbm_t& o)
      : _is_bottom(o._is_bottom),
        _is_normalized(o._is_normalized),
        _matrix(o._matrix),
        _var_indexes(o._var_indexes) {}

  dbm_t& operator=(const dbm_t& o) {
    _is_bottom = o._is_bottom;
    _is_normalized = o._is_normalized;
    _matrix = o._matrix;
    _var_indexes = o._var_indexes;
    return *this;
  }

  bool is_bottom() {
    normalize();
    return _is_bottom;
  }

  bool is_top() {
    // Does not require normalization

    if (_is_bottom) {
      return false;
    }

    // Check that the matrix contains only +oo
    for (typename var_indexes_t::iterator it = _var_indexes.begin();
         it != _var_indexes.end();) {
      unsigned int i = it->second;

      for (unsigned int j = 0; j < _matrix.num_var(); j++) {
        if (i != j &&
            !(_matrix(i, j).is_plus_infinity() &&
              _matrix(j, i).is_plus_infinity())) {
          return false;
        }
      }

      it = _var_indexes.erase(it);
    }

    _matrix.clear();
    _var_indexes.clear();
    _is_normalized = true;
    return true;
  }

  variable_set_t variables() const {
    variable_set_t variables;
    for (typename var_indexes_t::const_iterator it = _var_indexes.cbegin();
         it != _var_indexes.cend();
         ++it) {
      variables += it->first;
    }
    return variables;
  }

  bool operator<=(dbm_t o) {
    // Requires normalization
    normalize();
    o.normalize();

    if (is_bottom()) {
      return true;
    } else if (o.is_bottom()) {
      return false;
    } else {
      variable_set_t vars;

      /* For each variable v in o, we check if we have it in our list.
       * We don't care about variables in our list that are not in o. */
      for (typename var_indexes_t::iterator it = o._var_indexes.begin();
           it != o._var_indexes.end();) {
        if (_var_indexes.find(it->first) == _var_indexes.end()) {
          // tricky case: we need to check if we have any information on v
          unsigned int i = it->second;

          for (unsigned int j = 0; j < o._matrix.num_var(); j++) {
            if (i != j && (!o._matrix(i, j).is_plus_infinity() ||
                           !o._matrix(j, i).is_plus_infinity())) {
              return false;
            }
          }

          it = o._var_indexes.erase(it);
        } else {
          vars += it->first;
          ++it;
        }
      }

      // Check if _matrix(i, j) <= o._matrix(i, j)
      for (typename variable_set_t::iterator it = vars.begin();
           it != vars.end();
           ++it) {
        unsigned int this_i = this->_var_indexes[(*it).name()];
        unsigned int o_i = o._var_indexes[(*it).name()];

        // special variable 0
        if (o._matrix(o_i, 0) < _matrix(this_i, 0) ||
            o._matrix(0, o_i) < _matrix(0, this_i)) {
          return false;
        }

        for (typename variable_set_t::iterator it2 = vars.begin();
             it2 != vars.end();
             ++it2) {
          unsigned int this_j = this->_var_indexes[(*it2).name()];
          unsigned int o_j = o._var_indexes[(*it2).name()];

          if (o._matrix(o_i, o_j) < _matrix(this_i, this_j)) {
            return false;
          }
        }
      }

      return true;
    }
  }

  /// \brief Join
  dbm_t operator|(dbm_t o) {
    // Requires normalization
    normalize();
    o.normalize();

    if (is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      dbm_t dbm = pointwise_binary_op< join_op >(o);
      dbm._is_normalized = true; // The join is normalized by construction
      return dbm;
    }
  }

  /// \brief Widening
  dbm_t operator||(dbm_t o) const {
    // Requires the normalization of the right operand.
    // The left operand (this) should not be normalized.
    o.normalize();

    if (_is_bottom) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      return pointwise_binary_op< widening_op >(o);
    }
  }

  /// \brief Join on a loop head
  dbm_t join_loop(dbm_t o) { return this->operator|(o); }

  /// \brief Join on two consecutive iterations
  dbm_t join_iter(dbm_t o) { return this->operator|(o); }

  /// \brief Meet
  dbm_t operator&(dbm_t o) {
    // Requires normalization
    normalize();
    o.normalize();

    if (this->is_bottom() || o.is_bottom()) {
      return dbm_t::bottom();
    } else {
      return pointwise_binary_op< meet_op >(o);
    }
  }

  /// \brief Narrowing
  dbm_t operator&&(dbm_t o) {
    // Requires normalization
    normalize();
    o.normalize();

    if (this->is_bottom() || o.is_bottom()) {
      return dbm_t::bottom();
    } else {
      return pointwise_binary_op< narrowing_op >(o);
    }
  }

  void assign(VariableName x, linear_expression_t e) {
    // Does not require normalization

    if (_is_bottom)
      return;

    unsigned int i = var_index(x);

    if (e.is_constant()) { // x = c
      forget(i);
      add_constraint(i, 0, e.constant());
      add_constraint(0, i, -e.constant());
    } else if (e.size() == 1 && e.begin()->first == 1) { // x = y + c
      const VariableName& y = e.begin()->second.name();
      Number c = e.constant();

      if (x == y) { // x = x + c
        increment(i, c);
      } else {
        unsigned int j = var_index(y);

        forget(i);
        add_constraint(i, j, c);
        add_constraint(j, i, -c);
      }
    } else {
      // Projection using intervals, requires normalization
      normalize();

      if (is_bottom())
        return;

      interval_t value = to_interval(e);

      forget(i);
      add_constraint(i, 0, value.ub());
      add_constraint(0, i, -value.lb());
    }
  }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    // Requires normalization
    normalize();

    if (is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = to_interval(y);
    interval_t v_z = to_interval(z);

    switch (op) {
      case OP_ADDITION: {
        if (v_z.singleton()) { // x = y + c
          apply(op, x, y, *v_z.singleton());
          return;
        } else if (v_y.singleton()) { // x = c + z
          apply(op, x, z, *v_y.singleton());
          return;
        }
        v_x = v_y + v_z;
        break;
      }
      case OP_SUBTRACTION: {
        if (v_z.singleton()) { // x = y - c
          apply(op, x, y, *v_z.singleton());
          return;
        }
        v_x = v_y - v_z;
        break;
      }
      case OP_MULTIPLICATION: {
        if (v_z.singleton()) { // x = y * c
          apply(op, x, y, *v_z.singleton());
          return;
        } else if (v_y.singleton()) { // x = c * z
          apply(op, x, z, *v_y.singleton());
          return;
        }
        v_x = v_y * v_z;
        break;
      }
      case OP_DIVISION: {
        if (v_z.singleton()) { // x = y / c
          apply(op, x, y, *v_z.singleton());
          return;
        }
        v_x = v_y / v_z;
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }

    unsigned int i = var_index(x);
    forget(i);
    add_constraint(i, 0, v_x.ub());
    add_constraint(0, i, -v_x.lb());
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    // Does not require normalization

    if (_is_bottom)
      return;

    unsigned int i = var_index(x);

    switch (op) {
      case OP_ADDITION: {
        if (x == y) { // x = x + k
          increment(i, k);
        } else { // x = y + k
          unsigned int j = var_index(y);

          forget(i);
          add_constraint(i, j, k);
          add_constraint(j, i, -k);
        }
        break;
      }
      case OP_SUBTRACTION: {
        if (x == y) { // x = x - k
          increment(i, -k);
        } else { // x = y - k
          unsigned int j = var_index(y);

          forget(i);
          add_constraint(i, j, -k);
          add_constraint(j, i, k);
        }
        break;
      }
      case OP_MULTIPLICATION: {
        if (k == 1) { // x = y
          if (x == y)
            return;
          unsigned int j = var_index(y);

          forget(i);
          add_constraint(i, j, 0);
          add_constraint(j, i, 0);
        } else {
          // Requires normalization
          normalize();

          if (is_bottom())
            return;

          interval_t v_x = to_interval(y) * k;
          forget(i);
          add_constraint(i, 0, v_x.ub());
          add_constraint(0, i, -v_x.lb());
        }
        break;
      }
      case OP_DIVISION: {
        if (k == 1) { // x = y
          if (x == y)
            return;
          unsigned int j = var_index(y);

          forget(i);
          add_constraint(i, j, 0);
          add_constraint(j, i, 0);
        } else {
          // Requires normalization
          normalize();

          if (is_bottom())
            return;

          interval_t v_x = to_interval(y) / k;
          forget(i);
          add_constraint(i, 0, v_x.ub());
          add_constraint(0, i, -v_x.lb());
        }
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }
  }

  void operator-=(VariableName v) { forget_num(v); }

  void forget(VariableName v) { forget_num(v); }

  void forget_num(VariableName v) {
    if (_is_bottom)
      return;

    typename var_indexes_t::iterator it = _var_indexes.find(v);
    if (it != _var_indexes.end()) {
      forget(it->second);
      _var_indexes.erase(it);
    }
  }

  template < typename Iterator >
  void forget(Iterator begin, Iterator end) {
    forget_num(begin, end);
  }

  template < typename Iterator >
  void forget_num(Iterator begin, Iterator end) {
    for (auto it = begin; it != end; ++it) {
      this->operator-=(*it);
    }
  }

  void operator+=(linear_constraint_t cst) {
    // Does not require normalization

    if (_is_bottom)
      return;

    if (cst.size() == 0) {
      if (cst.is_contradiction()) {
        set_bottom();
      }
      return;
    }

    typename linear_expression_t::iterator it = cst.begin();
    typename linear_expression_t::iterator it2 = ++cst.begin();
    unsigned int i, j;
    Number c = cst.constant();

    if (cst.size() == 1 && it->first == 1) {
      i = var_index(it->second.name());
      j = 0;
    } else if (cst.size() == 1 && it->first == -1) {
      i = 0;
      j = var_index(it->second.name());
    } else if (cst.size() == 2 && it->first == 1 && it2->first == -1) {
      i = var_index(it->second.name());
      j = var_index(it2->second.name());
    } else if (cst.size() == 2 && it->first == -1 && it2->first == 1) {
      i = var_index(it2->second.name());
      j = var_index(it->second.name());
    } else {
      // use the linear interval solver
      normalize();

      if (is_bottom())
        return;

      linear_constraint_system_t csts;
      csts += cst;
      solver_t solver(csts, max_reduction_cycles);
      solver.run(*this);
      return;
    }

    if (cst.is_inequality()) {
      add_constraint(i, j, c);
    } else if (cst.is_equality()) {
      add_constraint(i, j, c);
      add_constraint(j, i, -c);
    } else {
      // use the linear interval solver
      normalize();

      if (is_bottom())
        return;

      linear_constraint_system_t csts;
      csts += cst;
      solver_t solver(csts, max_reduction_cycles);
      solver.run(*this);
    }
  }

  void operator+=(linear_constraint_system_t csts) {
    if (_is_bottom)
      return;

    linear_constraint_system_t remaining_csts;

    for (linear_constraint_t cst : csts) {
      // process each constraint
      if (cst.size() == 0) {
        if (cst.is_contradiction()) {
          set_bottom();
          return;
        }
      } else if (cst.is_inequality() || cst.is_equality()) {
        typename linear_expression_t::iterator it = cst.begin();
        typename linear_expression_t::iterator it2 = ++cst.begin();
        unsigned int i, j;
        Number c = cst.constant();

        if (cst.size() == 1 && it->first == 1) {
          i = var_index(it->second.name());
          j = 0;
        } else if (cst.size() == 1 && it->first == -1) {
          i = 0;
          j = var_index(it->second.name());
        } else if (cst.size() == 2 && it->first == 1 && it2->first == -1) {
          i = var_index(it->second.name());
          j = var_index(it2->second.name());
        } else if (cst.size() == 2 && it->first == -1 && it2->first == 1) {
          i = var_index(it2->second.name());
          j = var_index(it->second.name());
        } else {
          remaining_csts += cst;
          continue;
        }

        if (cst.is_inequality()) {
          add_constraint(i, j, c);
        } else {
          add_constraint(i, j, c);
          add_constraint(j, i, -c);
        }
      } else {
        remaining_csts += cst;
      }
    }

    if (!remaining_csts.empty()) {
      // use the linear interval solver
      normalize();

      if (is_bottom())
        return;

      solver_t solver(remaining_csts, max_reduction_cycles);
      solver.run(*this);
    }
  }

  congruence_t to_congruence(VariableName x) { return congruence_t::top(); }

  interval_t to_interval(VariableName x, bool normalize = false) {
    if (normalize) {
      this->normalize();
    }

    if (_is_bottom) {
      return interval_t::bottom();
    } else {
      typename var_indexes_t::const_iterator it = _var_indexes.find(x);

      if (it == _var_indexes.cend()) {
        return interval_t::top();
      } else {
        return interval_t(-_matrix(it->second, 0), _matrix(0, it->second));
      }
    }
  }

  interval_t to_interval(linear_expression_t e, bool normalize = false) {
    if (normalize) {
      this->normalize();
    }

    if (_is_bottom) {
      return interval_t::bottom();
    } else {
      interval_t result(e.constant());

      for (typename linear_expression_t::iterator it = e.begin(); it != e.end();
           ++it) {
        interval_t c(it->first);
        result += c * to_interval(it->second.name());
      }

      return result;
    }
  }

  void set(VariableName x, interval_t intv) {
    if (_is_bottom)
      return;

    if (intv.is_bottom()) {
      set_bottom();
      return;
    }

    unsigned int i = var_index(x);
    forget(i);
    add_constraint(i, 0, intv.ub());
    add_constraint(0, i, -intv.lb());
  }

  void refine(VariableName x, interval_t intv) {
    if (_is_bottom)
      return;

    if (intv.is_bottom()) {
      set_bottom();
      return;
    }

    unsigned int i = var_index(x);
    add_constraint(i, 0, intv.ub());
    add_constraint(0, i, -intv.lb());
  }

  // bitwise_operators_api

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             uint64_t from,
             uint64_t to) {
    // Requires normalization
    normalize();

    if (is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = to_interval(y);

    switch (op) {
      case OP_TRUNC: {
        v_x = v_y.Trunc(from, to);
        break;
      }
      case OP_ZEXT: {
        v_x = v_y.ZExt(from, to);
        break;
      }
      case OP_SEXT: {
        v_x = v_y.SExt(from, to);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }

    unsigned int i = var_index(x);
    forget(i);
    add_constraint(i, 0, v_x.ub());
    add_constraint(0, i, -v_x.lb());
  }

  void apply(conv_operation_t op,
             VariableName x,
             Number k,
             uint64_t from,
             uint64_t to) {
    // Does not require normalization

    if (_is_bottom)
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y(k);

    switch (op) {
      case OP_TRUNC: {
        v_x = v_y.Trunc(from, to);
        break;
      }
      case OP_ZEXT: {
        v_x = v_y.ZExt(from, to);
        break;
      }
      case OP_SEXT: {
        v_x = v_y.SExt(from, to);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }

    unsigned int i = var_index(x);
    forget(i);
    add_constraint(i, 0, v_x.ub());
    add_constraint(0, i, -v_x.lb());
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    // Requires normalization
    normalize();

    if (is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = to_interval(y);
    interval_t v_z = to_interval(z);

    switch (op) {
      case OP_AND: {
        v_x = v_y.And(v_z);
        break;
      }
      case OP_OR: {
        v_x = v_y.Or(v_z);
        break;
      }
      case OP_XOR: {
        v_x = v_y.Xor(v_z);
        break;
      }
      case OP_SHL: {
        v_x = v_y.Shl(v_z);
        break;
      }
      case OP_LSHR: {
        v_x = v_y.LShr(v_z);
        break;
      }
      case OP_ASHR: {
        v_x = v_y.AShr(v_z);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }

    unsigned int i = var_index(x);
    forget(i);
    add_constraint(i, 0, v_x.ub());
    add_constraint(0, i, -v_x.lb());
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    // Requires normalization
    normalize();

    if (is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = to_interval(y);
    interval_t v_z(k);

    switch (op) {
      case OP_AND: {
        v_x = v_y.And(v_z);
        break;
      }
      case OP_OR: {
        v_x = v_y.Or(v_z);
        break;
      }
      case OP_XOR: {
        v_x = v_y.Xor(v_z);
        break;
      }
      case OP_SHL: {
        v_x = v_y.Shl(v_z);
        break;
      }
      case OP_LSHR: {
        v_x = v_y.LShr(v_z);
        break;
      }
      case OP_ASHR: {
        v_x = v_y.AShr(v_z);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }

    unsigned int i = var_index(x);
    forget(i);
    add_constraint(i, 0, v_x.ub());
    add_constraint(0, i, -v_x.lb());
  }

  // division_operators_api

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    // Requires normalization
    normalize();

    if (is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = to_interval(y);
    interval_t v_z = to_interval(z);

    switch (op) {
      case OP_SDIV: {
        v_x = v_y / v_z;
        break;
      }
      case OP_UDIV: {
        v_x = v_y.UDiv(v_z);
        break;
      }
      case OP_SREM: {
        v_x = v_y.SRem(v_z);
        break;
      }
      case OP_UREM: {
        v_x = v_y.URem(v_z);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }

    unsigned int i = var_index(x);
    forget(i);
    add_constraint(i, 0, v_x.ub());
    add_constraint(0, i, -v_x.lb());
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    // Requires normalization
    normalize();

    if (is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = to_interval(y);
    interval_t v_z(k);

    switch (op) {
      case OP_SDIV: {
        v_x = v_y / v_z;
        break;
      }
      case OP_UDIV: {
        v_x = v_y.UDiv(v_z);
        break;
      }
      case OP_SREM: {
        v_x = v_y.SRem(v_z);
        break;
      }
      case OP_UREM: {
        v_x = v_y.URem(v_z);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }

    unsigned int i = var_index(x);
    forget(i);
    add_constraint(i, 0, v_x.ub());
    add_constraint(0, i, -v_x.lb());
  }

  linear_constraint_system_t to_linear_constraint_system() {
    normalize();
    linear_constraint_system_t csts;

    if (is_bottom()) {
      csts += linear_constraint_t::contradiction();
      return csts;
    }

    for (typename var_indexes_t::iterator it = _var_indexes.begin();
         it != _var_indexes.end();
         ++it) {
      variable_t var_i(it->first);
      unsigned int i = it->second;
      csts += within_interval(var_i, interval_t(-_matrix(i, 0), _matrix(0, i)));

      typename var_indexes_t::iterator it2 = it;
      ++it2;
      for (; it2 != _var_indexes.end(); ++it2) {
        variable_t var_j(it2->first);
        unsigned int j = it2->second;
        csts += within_interval(var_i - var_j,
                                interval_t(-_matrix(i, j), _matrix(j, i)));
      }
    }

    return csts;
  }

  void write(std::ostream& o) {
    linear_constraint_system_t csts = to_linear_constraint_system();
    o << csts;
  }

  static std::string domain_name() { return "DBM"; }

}; // end class dbm

namespace num_domain_traits {
namespace detail {

template < typename Number, typename VariableName >
struct normalize_impl< dbm< Number, VariableName > > {
  inline void operator()(dbm< Number, VariableName >& inv) { inv.normalize(); }
};

template < typename Number, typename VariableName >
struct var_to_interval_impl< dbm< Number, VariableName > > {
  inline interval< Number > operator()(dbm< Number, VariableName >& inv,
                                       VariableName v,
                                       bool normalize) {
    return inv.to_interval(v, normalize);
  }
};

template < typename Number, typename VariableName >
struct lin_expr_to_interval_impl< dbm< Number, VariableName > > {
  inline interval< Number > operator()(
      dbm< Number, VariableName >& inv,
      linear_expression< Number, VariableName > e,
      bool normalize) {
    return inv.to_interval(e, normalize);
  }
};

} // end namespace detail
} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_DBM_HPP
