/**************************************************************************/ /**
 *
 * \file
 * \brief Standard domain of octagons.
 *
 * Based on Antoine Mine's paper: The Octagon Abstract Domain, in
 * Higher-Order and Symbolic Computation, 19(1): 31-100 (2006)
 *
 * Author: Alexandre C. D. Wimmers
 *
 * Contributors: Jorge A. Navas
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

#ifndef IKOS_OCTAGONS_HPP
#define IKOS_OCTAGONS_HPP

#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

#include <boost/container/flat_map.hpp>
#include <boost/optional.hpp>

#include <ikos/algorithms/linear_constraints.hpp>
#include <ikos/common/types.hpp>
#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/domains/bitwise_operators_api.hpp>
#include <ikos/domains/division_operators_api.hpp>
#include <ikos/domains/numerical_domains_api.hpp>
#include <ikos/value/congruence.hpp>
#include <ikos/value/interval.hpp>

// #define VERBOSE

namespace ikos {

/// \brief Octagon abstract domain
///
/// Warning: The memory of this domain is managed in a brute force
/// fashion (O(n^2)) as it is intended to be used for small sets of
/// variables (~10-20).
template < typename Number, typename VariableName >
class octagon : public abstract_domain,
                public numerical_domain< Number, VariableName >,
                public bitwise_operators< Number, VariableName >,
                public division_operators< Number, VariableName > {
private:
  typedef bound< Number > bound_t;
  typedef interval_domain< Number, VariableName > intervals_t;

  class dbmatrix : public writeable {
  private:
    // Matrix represented as single vector due to performance issues,
    // using a standard vector<vector > results in significant
    // degradation of performance.
    std::vector< bound_t > _matrix;
    std::size_t _num_var;

  public:
    dbmatrix() : _num_var(0) {} // Size of 0 represents top.

    dbmatrix(const dbmatrix& other)
        : _matrix(other._matrix), _num_var(other._num_var) {}

    dbmatrix operator=(dbmatrix other) {
      this->_num_var = other._num_var;
      this->_matrix = other._matrix;
      return *this;
    }

    void resize(size_t new_size) { // Expected size value to represent number of
                                   // contained variables.
      if (this->_num_var >=
          new_size) { // Does not currently support downsizing.
        return;
      }

      bound_t infinite("+oo");
      this->_matrix.resize(4 * new_size * new_size, infinite);
      for (size_t j = 2 * this->_num_var; j > 0; --j) {
        for (size_t i = 2 * this->_num_var; i > 0; --i) {
          std::swap(this->_matrix[2 * new_size * (j - 1) + (i - 1)],
                    this->_matrix[2 * _num_var * (j - 1) + (i - 1)]);
        }
      }

      this->_num_var = new_size;
    }

    // downsize the matrix
    void operator-=(unsigned int k) {
      if (!(k >= 1 && k <= this->_num_var)) {
        assert(false && "dbmatrix::operator-= received a wrong value");
      }

      if (this->_num_var == 0)
        return;

      std::size_t new_size = this->_num_var - 1;
      bound_t infinite("+oo");
      std::vector< bound_t > new_matrix;
      new_matrix.resize(4 * new_size * new_size, infinite);

      for (std::size_t j = 2 * this->_num_var; j > 0; --j) {
        if (j == 2 * k || j == 2 * k - 1)
          continue;

        for (std::size_t i = 2 * this->_num_var; i > 0; --i) {
          if (i == 2 * k || i == 2 * k - 1)
            continue;

          if (i > 2 * k) {
            if (j > 2 * k) {
              std::swap(new_matrix[2 * new_size * (j - 3) + (i - 3)],
                        this->_matrix[2 * _num_var * (j - 1) + (i - 1)]);
            } else { // j < 2*k-1
              std::swap(new_matrix[2 * new_size * (j - 1) + (i - 3)],
                        this->_matrix[2 * _num_var * (j - 1) + (i - 1)]);
            }
          } else { // i < 2*k-1
            if (j > 2 * k) {
              std::swap(new_matrix[2 * new_size * (j - 3) + (i - 1)],
                        this->_matrix[2 * _num_var * (j - 1) + (i - 1)]);
            } else { // j < 2*k-1
              std::swap(new_matrix[2 * new_size * (j - 1) + (i - 1)],
                        this->_matrix[2 * _num_var * (j - 1) + (i - 1)]);
            }
          }
        }
      }
      std::swap(this->_matrix, new_matrix);
      this->_num_var = new_size;
    }

    std::size_t size() { // Returned size indicates number of variables.
      return this->_num_var;
    }

    bound_t& operator()(unsigned int i, unsigned int j) {
      // Accesses the matrix as one-based and in column-major order
      // so as to match DBM representations.
      if (i < 1 || j < 1 || i > 2 * this->_num_var || j > 2 * this->_num_var) {
        std::cerr << "Error: out-of-bounds error accessing internal octagon "
                  << "matrix at [" << i << ", " << j
                  << "] where 2n = " << 2 * _num_var << ".";
        assert(false);
      }
      return this->_matrix[2 * this->_num_var * (j - 1) + (i - 1)];
    }

    void write(std::ostream& o) {
      for (unsigned int i = 1; i <= 2 * this->_num_var; i++) {
        for (unsigned int j = 1; j <= 2 * this->_num_var; j++) {
          bound_t val = this->operator()(i, j);
          o << "M[" << i << "," << j << "]=" << val << ";";
        }
        o << std::endl;
      }
    }

  }; // end class dbmatrix

public:
  typedef variable< Number, VariableName > variable_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef octagon< Number, VariableName > octagon_t;
  typedef interval< Number > interval_t;
  typedef congruence< Number > congruence_t;
  typedef interval_domain< Number, VariableName > interval_domain_t;

private:
  typedef dbmatrix matrix_t;
  typedef boost::container::flat_map< variable_t, size_t > map_t;
  typedef typename map_t::value_type value_type;
  typedef unsigned char BOOL;

private:
  bool _is_bottom;
  matrix_t _dbm;
  map_t _map;
  bool _is_normalized;
  std::vector< BOOL >
      _norm_vector; // IMPORTANT: Treat this as a vector of booleans.

  void set_to_bottom() { this->_is_bottom = true; }

private:
  octagon(bool is_top) : _is_bottom(!is_top), _is_normalized(true) {}

  void add_var(VariableName x,
               unsigned int i,
               unsigned int j,
               bound_t lb,
               bound_t ub,
               bool op_eq) {
    if (lb.is_minus_infinity() && ub.is_plus_infinity()) {
      abstract(x);
      return;
    }

    if (op_eq) {
      for (std::size_t j_idx = 1; j_idx <= 2 * _dbm.size(); ++j_idx) {
        if (j_idx != 2 * j && j_idx != 2 * j - 1) {
          _dbm(2 * j - 1, j_idx) -= lb;
          _dbm(2 * j, j_idx) += ub;
        }
      }

      for (std::size_t i_idx = 1; i_idx <= 2 * _dbm.size(); ++i_idx) {
        if (i_idx != 2 * j && i_idx != 2 * j - 1) {
          _dbm(i_idx, 2 * j) -= lb;
          _dbm(i_idx, 2 * j - 1) += ub;
        }
      }

      _dbm(2 * j - 1, 2 * j) -= bound_t(2) * lb;
      _dbm(2 * j, 2 * j - 1) += bound_t(2) * ub;
    } else {
      abstract(x);
      _dbm(2 * j - 1, 2 * i - 1) = -lb;
      _dbm(2 * i, 2 * j) = -lb;
      _dbm(2 * i - 1, 2 * j - 1) = ub;
      _dbm(2 * j, 2 * i) = ub;
    }
  }

  void subtract_var(VariableName x,
                    unsigned int i,
                    unsigned int j,
                    bound_t lb,
                    bound_t ub,
                    bool op_eq) {
    add_var(x, i, j, -ub, -lb, op_eq);
  }

  void multiply_var(VariableName x,
                    unsigned int i,
                    unsigned int j,
                    bound_t lb,
                    bound_t ub,
                    bool op_eq) {
    if (op_eq) {
      bound_t t1(_dbm(2 * j - 1, 2 * j)), t2(_dbm(2 * j, 2 * j - 1));
      abstract(x);
      bound_t ll = (t1 / -2) * lb;
      bound_t lu = (t1 / -2) * ub;
      bound_t ul = (t2 / 2) * lb;
      bound_t uu = (t2 / 2) * ub;
      _dbm(2 * j - 1, 2 * j) = bound_t::min(ll, lu, ul, uu) * -2;
      _dbm(2 * j, 2 * j - 1) = bound_t::max(ll, lu, ul, uu) * 2;
    } else {
      bound_t t1(_dbm(2 * i - 1, 2 * i).operator-()),
          t2(_dbm(2 * i, 2 * i - 1));
      abstract(x);
      bound_t ll = (t1 / -2) * lb;
      bound_t lu = (t1 / -2) * ub;
      bound_t ul = (t2 / 2) * lb;
      bound_t uu = (t2 / 2) * ub;
      _dbm(2 * j - 1, 2 * j) = bound_t::min(ll, lu, ul, uu) * -2;
      _dbm(2 * j, 2 * j - 1) = bound_t::max(ll, lu, ul, uu) * 2;
    }
  }

  void divide_var(VariableName x,
                  unsigned int i,
                  unsigned int j,
                  bound_t _lb,
                  bound_t _ub,
                  bool op_eq) {
    interval_t trim_intv =
        ikos::intervals_impl::trim_bound(interval_t(_lb, _ub), Number(0));

    if (trim_intv.is_bottom()) {
      // definite division by zero
      set_to_bottom();
      return;
    }

    interval_t zero(Number(0));
    if (zero <= trim_intv) {
      abstract(x);
      return;
    }
    bound_t lb = trim_intv.lb();
    bound_t ub = trim_intv.ub();
    if (op_eq) {
      bound_t t1(_dbm(2 * j - 1, 2 * j)), t2(_dbm(2 * j, 2 * j - 1));
      abstract(x);
      bound_t ll = (t1 / -2) / lb;
      bound_t lu = (t1 / -2) / ub;
      bound_t ul = (t2 / 2) / lb;
      bound_t uu = (t2 / 2) / ub;
      _dbm(2 * j - 1, 2 * j) = bound_t::min(ll, lu, ul, uu) * -2;
      _dbm(2 * j, 2 * j - 1) = bound_t::max(ll, lu, ul, uu) * 2;
    } else {
      bound_t t1(_dbm(2 * i - 1, 2 * i).operator-()),
          t2(_dbm(2 * i, 2 * i - 1));
      abstract(x);
      bound_t ll = (t1 / -2) / lb;
      bound_t lu = (t1 / -2) / ub;
      bound_t ul = (t2 / 2) / lb;
      bound_t uu = (t2 / 2) / ub;
      _dbm(2 * j - 1, 2 * j) = bound_t::min(ll, lu, ul, uu) * -2;
      _dbm(2 * j, 2 * j - 1) = bound_t::max(ll, lu, ul, uu) * 2;
    }
  }

  /// \brief x = y op [lb, ub]
  void apply(
      operation_t op, VariableName x, VariableName y, bound_t lb, bound_t ub) {
    // Requires normalization.

    // add x in the DBM if not found
    if (this->_map.find(x) == this->_map.end()) {
      this->_map.insert(value_type(x, this->_map.size() + 1));
      this->resize();
    }

    if (this->_map.find(y) == this->_map.end()) {
      this->abstract(x);
      return;
    }

    unsigned int i(_map.find(x)->second), j(_map.find(y)->second);
    normalize();

    switch (op) {
      case OP_ADDITION: {
        add_var(x, i, j, lb, ub, i == j);
        break;
      }
      case OP_SUBTRACTION: {
        subtract_var(x, i, j, lb, ub, i == j);
        break;
      }
      case OP_MULTIPLICATION: {
        multiply_var(x, i, j, lb, ub, i == j);
        break;
      }
      case OP_DIVISION: {
        divide_var(x, i, j, lb, ub, i == j);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }
    _norm_vector.at(i - 1) = 0;
    // Result is not normalized.
  }

  void apply_constraint(unsigned int var,
                        bool is_positive,
                        bound_t constraint) {
    // Application of single variable octagonal constraints.
    constraint *= 2;
    if (is_positive) { // 2*v1 <= constraint
      _dbm(2 * var, 2 * var - 1) =
          bound_t::min(_dbm(2 * var, 2 * var - 1), constraint);
    } else { // 2*v1 >= constraint
      _dbm(2 * var - 1, 2 * var) =
          bound_t::min(_dbm(2 * var - 1, 2 * var), constraint);
    }

    if (_dbm(2 * var, 2 * var - 1) < _dbm(2 * var - 1, 2 * var).operator-()) {
      this->operator=(bottom());
    }
  }

  void apply_constraint(unsigned int i,
                        unsigned int j,
                        bool is1_positive,
                        bool is2_positive,
                        bound_t constraint) {
    // Application of double variable octagonal constraints.
    if (is1_positive && is2_positive) { // v1 + v2 <= constraint
      _dbm(2 * j, 2 * i - 1) = bound_t::min(_dbm(2 * j, 2 * i - 1), constraint);
      _dbm(2 * i, 2 * j - 1) = bound_t::min(_dbm(2 * i, 2 * j - 1), constraint);
    } else if (is1_positive && !is2_positive) { // v1 - v2 <= constraint
      _dbm(2 * j - 1, 2 * i - 1) =
          bound_t::min(_dbm(2 * j - 1, 2 * i - 1), constraint);
      _dbm(2 * i, 2 * j) = bound_t::min(_dbm(2 * i, 2 * j), constraint);
    } else if (!is1_positive && is2_positive) { // v2 - v1 <= constraint
      _dbm(2 * i - 1, 2 * j - 1) =
          bound_t::min(_dbm(2 * i - 1, 2 * j - 1), constraint);
      _dbm(2 * j, 2 * i) = bound_t::min(_dbm(2 * j, 2 * i), constraint);
    } else if (!is1_positive && !is2_positive) { // -v1 - v2 <= constraint
      _dbm(2 * j - 1, 2 * i) = bound_t::min(_dbm(2 * j - 1, 2 * i), constraint);
      _dbm(2 * i - 1, 2 * j) = bound_t::min(_dbm(2 * i - 1, 2 * j), constraint);
    }
    if (_dbm(2 * j, 2 * i - 1) < _dbm(2 * j - 1, 2 * i).operator-() ||
        _dbm(2 * j - 1, 2 * i - 1) < _dbm(2 * i - 1, 2 * j - 1).operator-()) {
      this->operator=(bottom());
    }
  }

  /// \brief Check satisfiability of cst using intervals
  ///
  /// Only to be used if cst is too hard for octagons
  bool check_sat(linear_constraint_t cst) {
    typename linear_constraint_t::variable_set_t vars = cst.variables();
    intervals_t inv;
    normalize();
    for (typename linear_constraint_t::variable_set_t::iterator it =
             vars.begin();
         it != vars.end();
         ++it) {
      inv.set(it->name(), to_interval(it->name(), false));
    }
    inv += cst;
    return !inv.is_bottom();
  }

  congruence_t to_congruence(VariableName x) { return congruence_t::top(); }

  interval_t to_interval(VariableName x, bool requires_normalization) {
    // projection requires normalization.
    typename map_t::iterator it(_map.find(x));
    if (it == _map.end()) {
      return interval_t::top();
    } else {
      unsigned int idx(it->second);
      if (requires_normalization)
        normalize();
      return interval_t(_dbm(2 * idx - 1, 2 * idx).operator/(-2),
                        _dbm(2 * idx, 2 * idx - 1).operator/(2));
    }
  } // Maintains normalization.

public:
  void set(VariableName x, interval_t intv) {
    // add x in the matrix if not found
    typename map_t::iterator it = this->_map.find(x);
    unsigned int idx;
    if (it == this->_map.end()) {
      idx =
          this->_map.insert(value_type(x, this->_map.size() + 1)).first->second;
      this->resize();
    } else {
      idx = it->second;
    }
    this->abstract(x);                              // normalize
    this->apply_constraint(idx, true, intv.ub());   // x <= ub
    this->apply_constraint(idx, false, -intv.lb()); // -x <= -lb
  }

private:
  void resize() {
    _dbm.resize(_map.size());
    _norm_vector.resize(_map.size(), 0);
  }

  void is_normalized(bool b) {
    _is_normalized = b;
    for (std::vector< unsigned char >::iterator it = _norm_vector.begin();
         it != _norm_vector.end();
         ++it) {
      *it = b ? 1 : 0;
    }
  }

  /// \brief Helper for normalization
  inline bound_t C(const bound_t& a,
                   const bound_t& b,
                   const bound_t& c,
                   const bound_t& d,
                   const bound_t& e) {
    // Separating the bound_t::min()s yields 20-25% increases in performance.
    return bound_t::min(a,
                        bound_t::min(b, bound_t::min(c, bound_t::min(d, e))));
  }

  // Helpers for lattice operations
  struct join_op {
    bound_t operator()(bound_t v1, bound_t v2) { return bound_t::max(v1, v2); }
  };

  struct widening_op {
    bound_t operator()(bound_t v1, bound_t v2) {
      bound_t infinite("+oo");
      return (v1 >= v2) ? v1 : infinite;
    }
  };

  template < typename op_t >
  octagon_t pointwise_binary_op(octagon_t o1, octagon_t o2) {
    octagon_t n;
    // Set intersection of the two maps
    for (typename map_t::iterator it = o1._map.begin(); it != o1._map.end();
         ++it) {
      if (o2._map.find(it->first) != o2._map.end()) {
        n._map.insert(value_type(it->first, n._map.size() + 1));
      }
    }
    if (n._map.size() == 0) {
      return top();
    }
    n.resize();
    n.is_normalized(true);

    unsigned int i1, i2, i3, j1, j2, j3;
    i1 = i2 = i3 = j1 = j2 = j3 = 0;
    for (typename map_t::iterator it = n._map.begin(); it != n._map.end();
         ++it) {
      // Finds the union of each 2x2 identity matrix.
      i1 = o1._map.find(it->first)->second;
      i2 = o2._map.find(it->first)->second;
      i3 = it->second;

      op_t op;
      n._dbm(2 * i3 - 1, 2 * i3 - 1) =
          op(o1._dbm(2 * i1 - 1, 2 * i1 - 1), o2._dbm(2 * i2 - 1, 2 * i2 - 1));
      n._dbm(2 * i3 - 1, 2 * i3) =
          op(o1._dbm(2 * i1 - 1, 2 * i1), o2._dbm(2 * i2 - 1, 2 * i2));
      n._dbm(2 * i3, 2 * i3 - 1) =
          op(o1._dbm(2 * i1, 2 * i1 - 1), o2._dbm(2 * i2, 2 * i2 - 1));
      n._dbm(2 * i3, 2 * i3) =
          op(o1._dbm(2 * i1, 2 * i1), o2._dbm(2 * i2, 2 * i2));

      for (typename map_t::iterator it2 = it + 1; it2 != n._map.end(); ++it2) {
        // Finds the union of each pair of 2x2 relational matrices.
        j1 = o1._map.find(it2->first)->second;
        j2 = o2._map.find(it2->first)->second;
        j3 = it2->second;

        n._dbm(2 * i3 - 1, 2 * j3 - 1) = op(o1._dbm(2 * i1 - 1, 2 * j1 - 1),
                                            o2._dbm(2 * i2 - 1, 2 * j2 - 1));
        n._dbm(2 * i3 - 1, 2 * j3) =
            op(o1._dbm(2 * i1 - 1, 2 * j1), o2._dbm(2 * i2 - 1, 2 * j2));
        n._dbm(2 * i3, 2 * j3 - 1) =
            op(o1._dbm(2 * i1, 2 * j1 - 1), o2._dbm(2 * i2, 2 * j2 - 1));
        n._dbm(2 * i3, 2 * j3) =
            op(o1._dbm(2 * i1, 2 * j1), o2._dbm(2 * i2, 2 * j2));

        n._dbm(2 * j3 - 1, 2 * i3 - 1) = op(o1._dbm(2 * j1 - 1, 2 * i1 - 1),
                                            o2._dbm(2 * j2 - 1, 2 * i2 - 1));
        n._dbm(2 * j3 - 1, 2 * i3) =
            op(o1._dbm(2 * j1 - 1, 2 * i1), o2._dbm(2 * j2 - 1, 2 * i2));
        n._dbm(2 * j3, 2 * i3 - 1) =
            op(o1._dbm(2 * j1, 2 * i1 - 1), o2._dbm(2 * j2, 2 * i2 - 1));
        n._dbm(2 * j3, 2 * i3) =
            op(o1._dbm(2 * j1, 2 * i1), o2._dbm(2 * j2, 2 * i2));
      }
    }
    return n;
  }

  // helper for debugging
  interval_domain_t to_intervals() {
    // Requires normalization.
    if (this->_is_bottom) {
      return interval_domain_t::bottom();
    } else {
      interval_domain_t itv = interval_domain_t::top();
      // we normalize just once
      normalize();
      for (typename map_t::iterator it = _map.begin(); it != _map.end(); ++it) {
        itv.set(it->first.name(), to_interval(it->first.name(), false));
      }
      return itv;
    }
  }

  /// \brief Abstract the variable
  boost::optional< typename map_t::iterator > abstract(variable_t v) {
    // Requires normalization.
    typename map_t::iterator it(_map.find(v));
    if (it != _map.end()) {
      normalize();
      std::size_t n = it->second;
      std::size_t odd = 2 * n - 1;
      std::size_t even = 2 * n;
      bound_t infinite("+oo"), zero(0);
      std::size_t size = 2 * _dbm.size();
      for (unsigned int idx = 1; idx <= size; ++idx) {
        _dbm(idx, odd) = infinite;
        _dbm(idx, even) = infinite;
        _dbm(odd, idx) = infinite;
        _dbm(even, idx) = infinite;
      }
      _dbm(odd, odd) = zero;
      _dbm(even, even) = zero;
      return boost::optional< typename map_t::iterator >(it);
    }
    return boost::optional< typename map_t::iterator >();
  } // Maintains normalization.

public:
  static octagon_t top() { return octagon(true); }

  static octagon_t bottom() { return octagon(false); }

public:
  /// \brief Constructs top octagon, represented by a size of 0.
  octagon() : _is_bottom(false), _is_normalized(true) {}

  octagon(const octagon_t& o)
      : _is_bottom(o._is_bottom),
        _dbm(o._dbm),
        _map(o._map),
        _is_normalized(o._is_normalized),
        _norm_vector(o._norm_vector) {}

  octagon_t operator=(octagon_t o) {
    _is_bottom = o.is_bottom();
    _dbm = o._dbm;
    _map = o._map;
    _is_normalized = o._is_normalized;
    _norm_vector = o._norm_vector;
    return *this;
  }

  bool is_bottom() { return _is_bottom; }

  bool is_top() { return !_map.size() && !is_bottom(); }

  /// \brief Compute the strong closure algorithm
  void normalize() {
    if (_is_normalized)
      return;

    is_normalized(false);

    std::size_t num_var(_dbm.size());
    bound_t zero(0);
    for (std::size_t k = 1; k <= num_var; ++k) {
      if (_norm_vector.at(k - 1) == 0) {
        for (std::size_t i = 1; i <= 2 * num_var; ++i) {
          for (std::size_t j = 1; j <= 2 * num_var; ++j) {
            // to ensure the "closed" property
            _dbm(i, j) =
                C(_dbm(i, j),
                  _dbm(i, 2 * k - 1) + _dbm(2 * k - 1, j),
                  _dbm(i, 2 * k) + _dbm(2 * k, j),
                  _dbm(i, 2 * k - 1) + _dbm(2 * k - 1, 2 * k) + _dbm(2 * k, j),
                  _dbm(i, 2 * k) + _dbm(2 * k, 2 * k - 1) + _dbm(2 * k - 1, j));
          }
        }
        // to ensure for all i,j: m_ij <= (m_i+i- + m_j-j+)/2
        for (size_t i = 1; i <= 2 * num_var; ++i) {
          for (size_t j = 1; j <= 2 * num_var; ++j) {
            _dbm(i, j) = bound_t::min(_dbm(i, j),
                                      (_dbm(i, i + 2 * (i % 2) - 1) +
                                       _dbm(j + 2 * (j % 2) - 1, j)) /
                                          Number(2));
          }
        }
        _norm_vector.at(k - 1) = 1;
      }
    }
    // negative cycle?
    for (size_t i = 1; i <= 2 * num_var; ++i) {
      if (_dbm(i, i) < 0) {
        this->operator=(bottom());
        return;
      }
      _dbm(i, i) = zero;
    }
    _is_normalized = true;
  }

  bool operator<=(octagon_t o) {
    // Require normalization of the first argument
    this->normalize();

    if (is_bottom()) {
      return true;
    } else if (o.is_bottom()) {
      return false;
    } else {
      map_t _temp; // Used to construct a list of variables that appear in both
                   // octagons.
      unsigned int i1, j1, i2, j2;
      for (typename map_t::iterator ito = o._map.begin(); ito != o._map.end();
           ++ito) {
        i2 = ito->second;
        if (_map.find(ito->first) == _map.end()) {
          if (!o._dbm(2 * i2 - 1, 2 * i2).is_infinite() ||
              !o._dbm(2 * i2, 2 * i2 - 1).is_infinite()) {
            // Case: Variable exists and is finite in _other but
            // does not exist in _this.
            return false;
          }
        } else {
          _temp.insert(value_type(ito->first, 0));
        }
      }

      for (typename map_t::iterator it = _temp.begin(); it != _temp.end();
           ++it) {
        // Case: Variable exists in both _this and _other.
        i1 = _map.find(it->first)->second;
        i2 = o._map.find(it->first)->second;

        if (!(_dbm(2 * i1 - 1, 2 * i1 - 1) <= o._dbm(2 * i2 - 1, 2 * i2 - 1)) ||
            !(_dbm(2 * i1 - 1, 2 * i1) <= o._dbm(2 * i2 - 1, 2 * i2)) ||
            !(_dbm(2 * i1, 2 * i1 - 1) <= o._dbm(2 * i2, 2 * i2 - 1)) ||
            !(_dbm(2 * i1, 2 * i1) <= o._dbm(2 * i2, 2 * i2))) {
          return false;
        }
        for (typename map_t::iterator it2 = it + 1; it2 != _temp.end(); ++it2) {
          j1 = _map.find(it2->first)->second;
          j2 = o._map.find(it2->first)->second;

          if (!(_dbm(2 * i1 - 1, 2 * j1 - 1) <=
                o._dbm(2 * i2 - 1, 2 * j2 - 1)) ||
              !(_dbm(2 * i1 - 1, 2 * j1) <= o._dbm(2 * i2 - 1, 2 * j2)) ||
              !(_dbm(2 * i1, 2 * j1 - 1) <= o._dbm(2 * i2, 2 * j2 - 1)) ||
              !(_dbm(2 * i1, 2 * j1) <= o._dbm(2 * i2, 2 * j2))) {
            return false;
          }
          if (!(_dbm(2 * j1 - 1, 2 * i1 - 1) <=
                o._dbm(2 * j2 - 1, 2 * i2 - 1)) ||
              !(_dbm(2 * j1 - 1, 2 * i1) <= o._dbm(2 * j2 - 1, 2 * i2)) ||
              !(_dbm(2 * j1, 2 * i1 - 1) <= o._dbm(2 * j2, 2 * i2 - 1)) ||
              !(_dbm(2 * j1, 2 * i1) <= o._dbm(2 * j2, 2 * i2))) {
            return false;
          }
        }
      }
      return true;
    }
  } // Maintains normalization.

  /// \brief Join
  octagon_t operator|(octagon_t o) {
    // Requires normalization of both operands
    normalize();
    o.normalize();
    if (is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      return pointwise_binary_op< join_op >(*this, o);
    }
  } // Returned matrix is normalized.

  /// \brief Widening
  octagon_t operator||(octagon_t o) {
    // The left operand of the widenning cannot be closed, otherwise
    // termination is not ensured. However, if the right operand is
    // close precision may be improved.
    o.normalize();
    if (is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      return pointwise_binary_op< widening_op >(*this, o);
    }
  } // Returned matrix is not normalized.

  octagon_t join_loop(octagon_t o) { return this->operator|(o); }

  octagon_t join_iter(octagon_t o) { return this->operator|(o); }

  /// \brief Meet
  octagon_t operator&(octagon_t o) {
    // Does not require normalization of any of the two operands
    if (is_bottom() || o.is_bottom()) {
      return bottom();
    } else {
      octagon_t n;
      // Set union of the two maps
      for (typename map_t::iterator it = _map.begin(); it != _map.end(); ++it) {
        n._map.insert(value_type(it->first, n._map.size() + 1));
      }
      for (typename map_t::iterator it = o._map.begin(); it != o._map.end();
           ++it) {
        n._map.insert(value_type(it->first, n._map.size() + 1));
      }
      if (n._map.size() == 0) {
        return top();
      }
      n.resize();
      n.is_normalized(false);

      typename map_t::iterator testi1, testi2, testj1, testj2;
      unsigned int i1, i2, i3, j1, j2, j3;
      i1 = i2 = i3 = j1 = j2 = j3 = 0;
      for (typename map_t::iterator it = n._map.begin(); it != n._map.end();
           ++it) {
        // Finds the intersection on each 2x2 identity matrix.
        testi1 = _map.find(it->first);
        testi2 = o._map.find(it->first);
        i3 = it->second;

        if (testi1 == _map.end()) {
          i1 = 0;
        } else {
          i1 = testi1->second;
        }
        if (testi2 == o._map.end()) {
          i2 = 0;
        } else {
          i2 = testi2->second;
        }

        n._dbm(2 * i3 - 1, 2 * i3 - 1) =
            (!i1) ? o._dbm(2 * i2 - 1, 2 * i2 - 1)
                  : ((!i2) ? _dbm(2 * i1 - 1, 2 * i1 - 1)
                           : bound_t::min(_dbm(2 * i1 - 1, 2 * i1 - 1),
                                          o._dbm(2 * i2 - 1, 2 * i2 - 1)));
        n._dbm(2 * i3 - 1, 2 * i3) =
            (!i1) ? o._dbm(2 * i2 - 1, 2 * i2)
                  : ((!i2) ? _dbm(2 * i1 - 1, 2 * i1)
                           : bound_t::min(_dbm(2 * i1 - 1, 2 * i1),
                                          o._dbm(2 * i2 - 1, 2 * i2)));
        n._dbm(2 * i3, 2 * i3 - 1) =
            (!i1) ? o._dbm(2 * i2, 2 * i2 - 1)
                  : ((!i2) ? _dbm(2 * i1, 2 * i1 - 1)
                           : bound_t::min(_dbm(2 * i1, 2 * i1 - 1),
                                          o._dbm(2 * i2, 2 * i2 - 1)));
        n._dbm(2 * i3, 2 * i3) =
            (!i1) ? o._dbm(2 * i2, 2 * i2)
                  : ((!i2) ? _dbm(2 * i1, 2 * i1)
                           : bound_t::min(_dbm(2 * i1, 2 * i1),
                                          o._dbm(2 * i2, 2 * i2)));

        for (typename map_t::iterator it2 = it + 1; it2 != n._map.end();
             ++it2) {
          // Finds the intersection of each pair of 2x2 relational matrices.
          testj1 = _map.find(it2->first);
          testj2 = o._map.find(it2->first);
          j3 = it2->second;

          if (testj1 == _map.end()) {
            j1 = 0;
          } else {
            j1 = testj1->second;
          }
          if (testj2 == o._map.end()) {
            j2 = 0;
          } else {
            j2 = testj2->second;
          }

          if (((i1 && !j1) || (!i1 && j1)) && ((i2 && !j2) || (!i2 && j2))) {
            continue;
          }
          if (i1 > j1) {
            std::swap(i1, j1);
          }
          if (i2 > j2) {
            std::swap(i2, j2);
          }

          n._dbm(2 * i3 - 1, 2 * j3 - 1) =
              (!i1 || !j1)
                  ? o._dbm(2 * i2 - 1, 2 * j2 - 1)
                  : ((!i2 || !j2)
                         ? _dbm(2 * i1 - 1, 2 * j1 - 1)
                         : bound_t::min(_dbm(2 * i1 - 1, 2 * j1 - 1),
                                        o._dbm(2 * i2 - 1, 2 * j2 - 1)));
          n._dbm(2 * i3 - 1, 2 * j3) =
              (!i1 || !j1)
                  ? o._dbm(2 * i2 - 1, 2 * j2)
                  : ((!i2 || !j2) ? _dbm(2 * i1 - 1, 2 * j1)
                                  : bound_t::min(_dbm(2 * i1 - 1, 2 * j1),
                                                 o._dbm(2 * i2 - 1, 2 * j2)));
          n._dbm(2 * i3, 2 * j3 - 1) =
              (!i1 || !j1)
                  ? o._dbm(2 * i2, 2 * j2 - 1)
                  : ((!i2 || !j2) ? _dbm(2 * i1, 2 * j1 - 1)
                                  : bound_t::min(_dbm(2 * i1, 2 * j1 - 1),
                                                 o._dbm(2 * i2, 2 * j2 - 1)));
          n._dbm(2 * i3, 2 * j3) =
              (!i1 || !j1)
                  ? o._dbm(2 * i2, 2 * j2)
                  : ((!i2 || !j2) ? _dbm(2 * i1, 2 * j1)
                                  : bound_t::min(_dbm(2 * i1, 2 * j1),
                                                 o._dbm(2 * i2, 2 * j2)));

          n._dbm(2 * j3 - 1, 2 * i3 - 1) =
              (!i1 || !j1)
                  ? o._dbm(2 * j2 - 1, 2 * i2 - 1)
                  : ((!i2 || !j2)
                         ? _dbm(2 * j1 - 1, 2 * i1 - 1)
                         : bound_t::min(_dbm(2 * j1 - 1, 2 * i1 - 1),
                                        o._dbm(2 * j2 - 1, 2 * i2 - 1)));
          n._dbm(2 * j3 - 1, 2 * i3) =
              (!i1 || !j1)
                  ? o._dbm(2 * j2 - 1, 2 * i2)
                  : ((!i2 || !j2) ? _dbm(2 * j1 - 1, 2 * i1)
                                  : bound_t::min(_dbm(2 * j1 - 1, 2 * i1),
                                                 o._dbm(2 * j2 - 1, 2 * i2)));
          n._dbm(2 * j3, 2 * i3 - 1) =
              (!i1 || !j1)
                  ? o._dbm(2 * j2, 2 * i2 - 1)
                  : ((!i2 || !j2) ? _dbm(2 * j1, 2 * i1 - 1)
                                  : bound_t::min(_dbm(2 * j1, 2 * i1 - 1),
                                                 o._dbm(2 * j2, 2 * i2 - 1)));
          n._dbm(2 * j3, 2 * i3) =
              (!i1 || !j1)
                  ? o._dbm(2 * j2, 2 * i2)
                  : ((!i2 || !j2) ? _dbm(2 * j1, 2 * i1)
                                  : bound_t::min(_dbm(2 * j1, 2 * i1),
                                                 o._dbm(2 * j2, 2 * i2)));
        }
      }

      return n;
    }
  } // Returned matrix is not normalized.

  /// \brief Narrowing
  octagon_t operator&&(octagon_t o) {
    // Does not require normalization of any of the two operands
    if (is_bottom() || o.is_bottom()) {
      return bottom();
    } else {
      octagon_t n;
      // Set union of the two maps
      for (typename map_t::iterator it = _map.begin(); it != _map.end(); ++it) {
        n._map.insert(value_type(it->first, n._map.size() + 1));
      }
      for (typename map_t::iterator it = o._map.begin(); it != o._map.end();
           ++it) {
        n._map.insert(value_type(it->first, n._map.size() + 1));
      }
      if (n._map.size() == 0) {
        return top();
      }
      n.resize();
      n.is_normalized(false);

      typename map_t::iterator testi1, testi2, testj1, testj2;
      unsigned int i1, i2, i3, j1, j2, j3;
      i1 = i2 = i3 = j1 = j2 = j3 = 0;
      for (typename map_t::iterator it = n._map.begin(); it != n._map.end();
           ++it) {
        // Finds the narrowing on each 2x2 identity matrix.
        testi1 = _map.find(it->first);
        testi2 = o._map.find(it->first);
        i3 = it->second;

        if (testi1 == _map.end()) {
          i1 = 0;
        } else {
          i1 = testi1->second;
        }
        if (testi2 == o._map.end()) {
          i2 = 0;
        } else {
          i2 = testi2->second;
        }

        n._dbm(2 * i3 - 1, 2 * i3 - 1) =
            (!i2) ? _dbm(2 * i1 - 1, 2 * i1 - 1)
                  : ((!i1 || _dbm(2 * i1 - 1, 2 * i1 - 1).is_infinite())
                         ? o._dbm(2 * i2 - 1, 2 * i2 - 1)
                         : _dbm(2 * i1 - 1, 2 * i1 - 1));
        n._dbm(2 * i3 - 1, 2 * i3) =
            (!i2) ? _dbm(2 * i1 - 1, 2 * i1)
                  : ((!i1 || _dbm(2 * i1 - 1, 2 * i1).is_infinite())
                         ? o._dbm(2 * i2 - 1, 2 * i2)
                         : _dbm(2 * i1 - 1, 2 * i1));
        n._dbm(2 * i3, 2 * i3 - 1) =
            (!i2) ? _dbm(2 * i1, 2 * i1 - 1)
                  : ((!i1 || _dbm(2 * i1, 2 * i1 - 1).is_infinite())
                         ? o._dbm(2 * i2, 2 * i2 - 1)
                         : _dbm(2 * i1, 2 * i1 - 1));
        n._dbm(2 * i3, 2 * i3) =
            (!i2) ? _dbm(2 * i1, 2 * i1)
                  : ((!i1 || _dbm(2 * i1, 2 * i1).is_infinite())
                         ? o._dbm(2 * i2, 2 * i2)
                         : _dbm(2 * i1, 2 * i1));

        for (typename map_t::iterator it2 = it + 1; it2 != n._map.end();
             ++it2) {
          // Finds the narrowing of each pair of 2x2 relational matrices.
          testj1 = _map.find(it2->first);
          testj2 = o._map.find(it2->first);
          j3 = it2->second;

          if (testj1 == _map.end()) {
            j1 = 0;
          } else {
            j1 = testj1->second;
          }
          if (testj2 == o._map.end()) {
            j2 = 0;
          } else {
            j2 = testj2->second;
          }

          if (((i1 && !j1) || (!i1 && j1)) && ((i2 && !j2) || (!i2 && j2))) {
            continue;
          }
          if (i1 > j1) {
            std::swap(i1, j1);
          }
          if (i2 > j2) {
            std::swap(i2, j2);
          }

          n._dbm(2 * i3 - 1, 2 * j3 - 1) =
              (!i2 || !j2)
                  ? _dbm(2 * i1 - 1, 2 * j1 - 1)
                  : ((!i1 || !j1 || _dbm(2 * i1 - 1, 2 * j1 - 1).is_infinite())
                         ? o._dbm(2 * i2 - 1, 2 * j2 - 1)
                         : _dbm(2 * i1 - 1, 2 * j1 - 1));
          n._dbm(2 * i3 - 1, 2 * j3) =
              (!i2 || !j2)
                  ? _dbm(2 * i1 - 1, 2 * j1)
                  : ((!i1 || !j1 || _dbm(2 * i1 - 1, 2 * j1).is_infinite())
                         ? o._dbm(2 * i2 - 1, 2 * j2)
                         : _dbm(2 * i1 - 1, 2 * j1));
          n._dbm(2 * i3, 2 * j3 - 1) =
              (!i2 || !j2)
                  ? _dbm(2 * i1, 2 * j1 - 1)
                  : ((!i1 || !j1 || _dbm(2 * i1, 2 * j1 - 1).is_infinite())
                         ? o._dbm(2 * i2, 2 * j2 - 1)
                         : _dbm(2 * i1, 2 * j1 - 1));
          n._dbm(2 * i3, 2 * j3) =
              (!i2 || !j2) ? _dbm(2 * i1, 2 * j1)
                           : ((!i1 || !j1 || _dbm(2 * i1, 2 * j1).is_infinite())
                                  ? o._dbm(2 * i2, 2 * j2)
                                  : _dbm(2 * i1, 2 * j1));

          n._dbm(2 * j3 - 1, 2 * i3 - 1) =
              (!i2 || !j2)
                  ? _dbm(2 * j1 - 1, 2 * i1 - 1)
                  : ((!i1 || !j1 || _dbm(2 * j1 - 1, 2 * i1 - 1).is_infinite())
                         ? o._dbm(2 * j2 - 1, 2 * i2 - 1)
                         : _dbm(2 * j1 - 1, 2 * i1 - 1));
          n._dbm(2 * j3 - 1, 2 * i3) =
              (!i2 || !j2)
                  ? _dbm(2 * j1 - 1, 2 * i1)
                  : ((!i1 || !j1 || _dbm(2 * j1 - 1, 2 * i1).is_infinite())
                         ? o._dbm(2 * j2 - 1, 2 * i2)
                         : _dbm(2 * j1 - 1, 2 * i1));
          n._dbm(2 * j3, 2 * i3 - 1) =
              (!i2 || !j2)
                  ? _dbm(2 * j1, 2 * i1 - 1)
                  : ((!i1 || !j1 || _dbm(2 * j1, 2 * i1 - 1).is_infinite())
                         ? o._dbm(2 * j2, 2 * i2 - 1)
                         : _dbm(2 * j1, 2 * i1 - 1));
          n._dbm(2 * j3, 2 * i3) =
              (!i2 || !j2) ? _dbm(2 * j1, 2 * i1)
                           : ((!i1 || !j1 || _dbm(2 * j1, 2 * i1).is_infinite())
                                  ? o._dbm(2 * j2, 2 * i2)
                                  : _dbm(2 * j1, 2 * i1));
        }
      }

      return n;
    }
  } // Returned matrix is not normalized.

  void operator-=(VariableName v) { forget_num(v); }

  void forget(VariableName v) { forget_num(v); }

  void forget_num(VariableName v) {
    if (boost::optional< typename map_t::iterator > it = this->abstract(v)) {
      size_t n = (*it)->second;
      this->_dbm -= n;
      this->_map.erase(*it);
      // update the values in _map
      for (typename map_t::iterator itz = this->_map.begin();
           itz != this->_map.end();
           ++itz) {
        if (itz->second > n) {
          this->_map[itz->first]--;
        }
      }
      this->_norm_vector.resize(this->_map.size(), 0);
      this->_is_normalized = false;
    }
  }

  template < typename Iterator >
  void forget(Iterator begin, Iterator end) {
    forget_num(begin, end);
  }

  template < typename Iterator >
  void forget_num(Iterator begin, Iterator end) {
    for (auto it = begin; it != end; ++it) {
      forget_num(*it);
    }
  }

  void assign(VariableName x, linear_expression_t e) {
    if (this->is_bottom())
      return;

    boost::optional< variable_t > v = e.get_variable();
    if (v && ((*v).name() == x))
      return;

    // add x in the matrix if not found
    typename map_t::iterator it = this->_map.find(x);
    unsigned int i;
    if (it == this->_map.end()) {
      i = this->_map.insert(value_type(x, this->_map.size() + 1)).first->second;
      this->resize();
    } else {
      i = it->second;
    }

    this->abstract(x); // call normalize()

    if (e.is_constant()) {
      this->apply_constraint(i, true, bound_t(e.constant())); // adding  x <=  c
      this->apply_constraint(i,
                             false,
                             bound_t(-(e.constant()))); // adding -x <= -c
    } else if (v) {
      VariableName y = (*v).name();
      typename map_t::iterator itz = this->_map.find(y);
      if (itz == this->_map.end()) {
        return; // x has been already abstracted
      }
      unsigned int j = itz->second;
      this->apply_constraint(i, j, true, false, bound_t(0));
      this->apply_constraint(i, j, false, true, bound_t(0));
    } else {
      assert(false &&
             "only supports constant or variable on the rhs of assignment");
    }

    this->is_normalized(false);
  }

  // Apply operations to variables.

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    // Requires normalization.

    typename map_t::iterator itz(_map.find(z));
    if (itz == this->_map.end()) {
      this->abstract(x);
      return;
    }
    unsigned int n(itz->second);

    if (!(x == y)) {
      assign(x, linear_expression_t(y));
      apply(op,
            x,
            x,
            _dbm(2 * n - 1, 2 * n).operator/(-2),
            _dbm(2 * n, 2 * n - 1).operator/(2));
    } else {
      apply(op,
            x,
            y,
            _dbm(2 * n - 1, 2 * n).operator/(-2),
            _dbm(2 * n, 2 * n - 1).operator/(2));
    }
    // Sets state to not normalized.
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    // Requires normalization.

    if (!(x == y)) {
      assign(x, linear_expression_t(y));
      apply(op, x, x, bound_t(k), bound_t(k));
    } else {
      apply(op, x, y, bound_t(k), bound_t(k));
    }
    // Sets state to not normalized.
  }

  void operator+=(linear_constraint_t cst) {
    // Does not require normalization.
    if (this->is_bottom()) {
      return;
    }
    bool v1, v2, is1_positive, is2_positive;
    v1 = v2 = is1_positive = is2_positive = false;
    unsigned int i, j;
    i = j = 0;
    for (typename linear_expression_t::iterator it = cst.expression().begin();
         it != cst.expression().end();
         ++it) {
      if (!v1) {
        // Calculates and loads information for the first variable.
        if (it->first == -1) {
          is1_positive = false;
        } else if (it->first == 1) {
          is1_positive = true;
        } else {
          assert(false &&
                 "expr contains unexpected coefficient (accepted values are "
                 "-1, 0, and 1).");
        }

        i = _map.insert(value_type(it->second, _map.size() + 1)).first->second;
        v1 = true;
      } else if (!v2) {
        // Calculates and loads information for the second variable,
        // if it exists.
        if (it->first == -1) {
          is2_positive = false;
        } else if (it->first == 1) {
          is2_positive = true;
        } else {
          assert(false &&
                 "expr contains unexpected coefficient (accepted values are "
                 "-1, 0, and 1).");
        }

        j = _map.insert(value_type(it->second, _map.size() + 1)).first->second;
        v2 = true;
      } else {
        std::cerr << cst << " is not an octagon constraint (> 2 variables).";
        assert(false);
      }
    }
    if (!v1) {
      if ((cst.is_inequality() && cst.constant() >= 0) ||
          (cst.is_equality() && cst.constant() == 0)) {
        return;
      }
      std::cerr << cst << " contains no variables.";
      assert(false);
    }
    resize();
    bound_t constant(cst.constant()), neg_constant(-(cst.constant()));

    if (cst.is_inequality()) { // Applies inequality constraints in the form of
                               // octagonal constraints.
      if (v1 && !v2) {
        apply_constraint(i, is1_positive, constant);
      } else /*if(v1 && v2)*/ {
        apply_constraint(i, j, is1_positive, is2_positive, constant);
      }
    } else if (cst.is_equality()) { // Applies equality constraints as two
                                    // octagonal constraints.
      if (v1 && !v2) {
        apply_constraint(i, is1_positive, constant);
        apply_constraint(i, !is1_positive, neg_constant);
      } else /*if(v1 && v2)*/ {
        apply_constraint(i, j, is1_positive, is2_positive, constant);
        apply_constraint(i, j, !is1_positive, !is2_positive, neg_constant);
      }
    } else if (cst.is_disequation()) {
      // we use intervals to reason about disequations
      if (!check_sat(cst))
        this->operator=(bottom());
    }
    _is_normalized = false;
  } // Sets state to not normalized.

  void operator+=(
      linear_constraint_system_t cst) { // Does not require normalization.
    for (typename linear_constraint_system_t::iterator it = cst.begin();
         it != cst.end();
         ++it) {
      this->operator+=(*it);
    }
  } // Sets state to not normalized.

  interval_t operator[](VariableName x) { return to_interval(x, true); }

  // bitwise_operators_api

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             uint64_t from,
             uint64_t to) {
    // since reasoning about infinite precision we simply assign and
    // ignore the width.
    // FIXME: unsound
    assign(x, linear_expression_t(y));
  }

  void apply(conv_operation_t op,
             VariableName x,
             Number k,
             uint64_t from,
             uint64_t to) {
    // since reasoning about infinite precision we simply assign
    // and ignore the width.
    // FIXME: unsound
    assign(x, k);
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    // Convert to intervals and perform the operation
    interval_t yi = this->operator[](y);
    interval_t zi = this->operator[](z);
    interval_t xi = interval_t::bottom();

    switch (op) {
      case OP_AND: {
        xi = yi.And(zi);
        break;
      }
      case OP_OR: {
        xi = yi.Or(zi);
        break;
      }
      case OP_XOR: {
        xi = yi.Xor(zi);
        break;
      }
      case OP_SHL: {
        xi = yi.Shl(zi);
        break;
      }
      case OP_LSHR: {
        xi = yi.LShr(zi);
        break;
      }
      case OP_ASHR: {
        xi = yi.AShr(zi);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }
    this->set(x, xi);
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    // Convert to intervals and perform the operation
    interval_t yi = this->operator[](y);
    interval_t zi(k);
    interval_t xi = interval_t::bottom();

    switch (op) {
      case OP_AND: {
        xi = yi.And(zi);
        break;
      }
      case OP_OR: {
        xi = yi.Or(zi);
        break;
      }
      case OP_XOR: {
        xi = yi.Xor(zi);
        break;
      }
      case OP_SHL: {
        xi = yi.Shl(zi);
        break;
      }
      case OP_LSHR: {
        xi = yi.LShr(zi);
        break;
      }
      case OP_ASHR: {
        xi = yi.AShr(zi);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }
    this->set(x, xi);
  }

  // division_operators_api

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    if (op == OP_SDIV) {
      apply(OP_DIVISION, x, y, z);
    } else {
      // Convert to intervals and perform the operation
      interval_t yi = this->operator[](y);
      interval_t zi = this->operator[](z);
      interval_t xi = interval_t::bottom();

      switch (op) {
        case OP_UDIV: {
          xi = yi.UDiv(zi);
          break;
        }
        case OP_SREM: {
          xi = yi.SRem(zi);
          break;
        }
        case OP_UREM: {
          xi = yi.URem(zi);
          break;
        }
        default: { ikos_unreachable("invalid operation"); }
      }
      this->set(x, xi);
    }
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    if (op == OP_SDIV) {
      apply(OP_DIVISION, x, y, k);
    } else {
      // Convert to intervals and perform the operation
      interval_t yi = this->operator[](y);
      interval_t zi(k);
      interval_t xi = interval_t::bottom();

      switch (op) {
        case OP_UDIV: {
          xi = yi.UDiv(zi);
          break;
        }
        case OP_SREM: {
          xi = yi.SRem(zi);
          break;
        }
        case OP_UREM: {
          xi = yi.URem(zi);
          break;
        }
        default: { ikos_unreachable("invalid operation"); }
      }
      this->set(x, xi);
    }
  }

  linear_constraint_system_t to_linear_constraint_system() {
    normalize();
    linear_constraint_system_t csts;

    if (is_bottom()) {
      csts += linear_constraint_t::contradiction();
      return csts;
    }

    bound_t lb(0), rb(0);
    unsigned int idx1(0), idx2(0);
    for (typename map_t::iterator it = _map.begin(); it != _map.end(); ++it) {
      idx1 = it->second;
      lb = _dbm(2 * idx1 - 1, 2 * idx1).operator/(-2);
      rb = _dbm(2 * idx1, 2 * idx1 - 1).operator/(2);
      // lb <= v <= rb
      csts += within_interval(it->first, interval_t(lb, rb));

      for (typename map_t::iterator it2 = it + 1; it2 != _map.end(); ++it2) {
        idx2 = it2->second;

        // v1 - v2
        lb = _dbm(2 * idx2, 2 * idx1).operator-();
        rb = _dbm(2 * idx2 - 1, 2 * idx1 - 1);
        csts += within_interval(variable_t(it->first) - variable_t(it2->first),
                                interval_t(lb, rb));

        // v1 + v2
        lb = _dbm(2 * idx2 - 1, 2 * idx1).operator-();
        rb = _dbm(2 * idx2, 2 * idx1 - 1);
        csts += within_interval(variable_t(it->first) + variable_t(it2->first),
                                interval_t(lb, rb));
      }
    }

    return csts;
  }

  void write(std::ostream& o) {
    linear_constraint_system_t csts = to_linear_constraint_system();
    o << csts;
#ifdef VERBOSE
    /// For debugging purposes
    { // print intervals
      interval_domain_t intervals = to_intervals();
      std::cerr << intervals;
    }
    { // print internal datastructures
      std::cerr << endl << "DBM: " << endl;
      std::cerr << "{";
      for (typename map_t::iterator it = _map.begin(); it != _map.end(); ++it) {
        unsigned int i_plus = (2 * it->second);
        unsigned int i_minus = (2 * it->second) - 1;
        std::cerr << it->first << "-"
                  << " -> " << i_minus << ";";
        std::cerr << it->first << "+"
                  << " -> " << i_plus << ";";
      }
      std::cerr << "}" << endl;
      std::cerr << _dbm << endl;
    }
#endif
  } // Maintains normalization.

  static std::string domain_name() { return "Octagons"; }

}; // end class octagon

namespace num_domain_traits {
namespace detail {

template < typename Number, typename VariableName >
struct normalize_impl< octagon< Number, VariableName > > {
  inline void operator()(octagon< Number, VariableName >& inv) {
    inv.normalize();
  }
};

} // end namespace detail
} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_OCTAGONS_HPP
