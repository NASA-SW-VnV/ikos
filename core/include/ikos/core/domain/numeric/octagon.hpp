/*******************************************************************************
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

#include <vector>

#include <boost/container/flat_map.hpp>
#include <boost/optional.hpp>

#include <ikos/core/domain/numeric/abstract_domain.hpp>
#include <ikos/core/domain/numeric/interval.hpp>
#include <ikos/core/number/bound.hpp>
#include <ikos/core/support/assert.hpp>
#include <ikos/core/value/numeric/interval.hpp>

//#define VERBOSE

namespace ikos {
namespace core {
namespace numeric {

/// \brief Octagon abstract domain
///
/// Warning: The memory of this domain is managed in a brute force
/// fashion (O(n^2)) as it is intended to be used for small sets of
/// variables (~10-20).
///
/// XXX: The implementation has several issues (segfaults, unsoundness, etc.).
/// Prefer the APRON octagon domain if necessary.
template < typename Number, typename VariableRef >
class Octagon final
    : public numeric::AbstractDomain< Number,
                                      VariableRef,
                                      Octagon< Number, VariableRef > > {
public:
  using BoundT = Bound< Number >;
  using IntervalT = Interval< Number >;
  using CongruenceT = Congruence< Number >;
  using IntervalCongruenceT = IntervalCongruence< Number >;
  using VariableExprT = VariableExpression< Number, VariableRef >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;
  using LinearConstraintSystemT = LinearConstraintSystem< Number, VariableRef >;
  using IntervalDomainT = IntervalDomain< Number, VariableRef >;

private:
  /// \brief Index of a variable in the matrix
  using MatrixIndex = std::size_t;

  // \brief Map from variable to index
  using VarIndexMap = boost::container::flat_map< VariableRef, MatrixIndex >;

  using Parent = numeric::AbstractDomain< Number, VariableRef, Octagon >;

private:
  class Matrix {
  private:
    // Matrix represented as single vector for performance.
    // Using a standard vector< vector<> > results in significant
    // degradation of performance.

  private:
    std::vector< BoundT > _matrix;
    MatrixIndex _num_var = 0; // size of the matrix

  public:
    /// \brief Create an empty matrix
    Matrix() = default;

    /// \brief Copy constructor
    Matrix(const Matrix&) = default;

    /// \brief Move constructor
    Matrix(Matrix&&) = default;

    /// \brief Copy assignment operator
    Matrix& operator=(const Matrix&) = default;

    /// \brief Move assignment operator
    Matrix& operator=(Matrix&&) = default;

    /// \brief Destructor
    ~Matrix() = default;

    /// \brief Return the number of variables
    MatrixIndex size() const { return this->_num_var; }

    /// \brief Resize the matrix
    ///
    /// \param new_size number of contained variables
    void resize(MatrixIndex new_size) {
      if (this->_num_var >= new_size) {
        // Does not currently support downsizing.
        return;
      }

      this->_matrix.resize(4 * new_size * new_size, BoundT::plus_infinity());

      for (MatrixIndex j = 2 * this->_num_var; j > 0; --j) {
        for (MatrixIndex i = 2 * this->_num_var; i > 0; --i) {
          std::swap(this->_matrix[2 * new_size * (j - 1) + (i - 1)],
                    this->_matrix[2 * _num_var * (j - 1) + (i - 1)]);
        }
      }

      this->_num_var = new_size;
    }

    /// \brief Downsize the matrix
    void operator-=(MatrixIndex k) {
      ikos_assert_msg(k >= 1 && k <= this->_num_var,
                      "invalid valuewrong value");

      if (this->_num_var == 0) {
        return;
      }

      MatrixIndex new_size = this->_num_var - 1;
      std::vector< BoundT > new_matrix;
      new_matrix.resize(4 * new_size * new_size, BoundT::plus_infinity());

      for (MatrixIndex j = 2 * this->_num_var; j > 0; --j) {
        if (j == 2 * k || j == 2 * k - 1) {
          continue;
        }

        for (MatrixIndex i = 2 * this->_num_var; i > 0; --i) {
          if (i == 2 * k || i == 2 * k - 1) {
            continue;
          }

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

    /// \brief Clear the matrix
    void clear() {
      this->_num_var = 0;
      this->_matrix.clear();
    }

    BoundT& operator()(MatrixIndex i, MatrixIndex j) {
      // Accesses the matrix as one-based and in column-major order
      // so as to match DBM representations.
      ikos_assert_msg(i >= 1 && j >= 1 && i <= 2 * this->_num_var &&
                          j <= 2 * this->_num_var,
                      "out of bounds matrix access");
      return this->_matrix[2 * this->_num_var * (j - 1) + (i - 1)];
    }

    const BoundT& operator()(MatrixIndex i, MatrixIndex j) const {
      // Accesses the matrix as one-based and in column-major order
      // so as to match DBM representations.
      ikos_assert_msg(i >= 1 && j >= 1 && i <= 2 * this->_num_var &&
                          j <= 2 * this->_num_var,
                      "out of bounds matrix access");
      return this->_matrix[2 * this->_num_var * (j - 1) + (i - 1)];
    }

    /// \brief Print the matrix, for debugging purpose
    void dump(std::ostream& o) const {
      for (MatrixIndex i = 1; i <= 2 * this->_num_var; i++) {
        for (MatrixIndex j = 1; j <= 2 * this->_num_var; j++) {
          o << "M[" << i << ", " << j << "] = " << this->operator()(i, j)
            << ";";
        }
        o << "\n";
      }
    }

  }; // end class Matrix

private:
  bool _is_bottom;
  bool _is_normalized;
  Matrix _matrix;
  VarIndexMap _var_index_map;

  // IMPORTANT: Treat this as a vector of booleans.
  std::vector< unsigned char > _norm_vector;

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top abstract value
  explicit Octagon(TopTag) : _is_bottom(false), _is_normalized(true) {}

  /// \brief Create the bottom abstract value
  explicit Octagon(BottomTag) : _is_bottom(true), _is_normalized(true) {}

public:
  /// \brief Create the top abstract value
  static Octagon top() { return Octagon(TopTag{}); }

  /// \brief Create the bottom abstract value
  static Octagon bottom() { return Octagon(BottomTag{}); }

  /// \brief Copy constructor
  Octagon(const Octagon&) = default;

  /// \brief Move constructor
  Octagon(Octagon&&) = default;

  /// \brief Copy assignment operator
  Octagon& operator=(const Octagon&) = default;

  /// \brief Move assignment operator
  Octagon& operator=(Octagon&&) = default;

  /// \brief Destructor
  ~Octagon() override = default;

private:
  /// \brief Helper for normalization
  static BoundT c(const BoundT& a,
                  const BoundT& b,
                  const BoundT& c,
                  const BoundT& d,
                  const BoundT& e) {
    // Separating the min()s yields 20-25% increases in performance.
    return min(a, min(b, min(c, min(d, e))));
  }

  /// \brief Set whether the matrix is normalized or not
  void set_normalized(bool b) {
    this->_is_normalized = b;
    for (auto it = _norm_vector.begin(); it != _norm_vector.end(); ++it) {
      *it = b ? 1 : 0;
    }
  }

  /// \brief Resize the octagon
  void resize() {
    this->_matrix.resize(this->_var_index_map.size());
    this->_norm_vector.resize(this->_var_index_map.size(), 0);
  }

  /// \brief Compute the strong closure algorithm
  ///
  /// TODO(marthaud): This is not thread-safe.
  void unsafe_normalize() const {
    if (this->_is_normalized) {
      return;
    }

    auto self = const_cast< Octagon* >(this);

    if (this->_is_bottom) {
      self->_is_normalized = true;
      return;
    }

    const MatrixIndex num_var = this->_matrix.size();

    for (MatrixIndex k = 1; k <= num_var; ++k) {
      if (this->_norm_vector[k - 1]) {
        continue;
      }

      for (MatrixIndex i = 1; i <= 2 * num_var; ++i) {
        for (MatrixIndex j = 1; j <= 2 * num_var; ++j) {
          // to ensure the "closed" property
          self->_matrix(i, j) =
              c(this->_matrix(i, j),
                this->_matrix(i, 2 * k - 1) + this->_matrix(2 * k - 1, j),
                this->_matrix(i, 2 * k) + this->_matrix(2 * k, j),
                this->_matrix(i, 2 * k - 1) + this->_matrix(2 * k - 1, 2 * k) +
                    this->_matrix(2 * k, j),
                this->_matrix(i, 2 * k) + this->_matrix(2 * k, 2 * k - 1) +
                    this->_matrix(2 * k - 1, j));
        }
      }

      // to ensure for all i,j: m_ij <= (m_i+i- + m_j-j+)/2
      for (MatrixIndex i = 1; i <= 2 * num_var; ++i) {
        for (MatrixIndex j = 1; j <= 2 * num_var; ++j) {
          self->_matrix(i, j) = min(this->_matrix(i, j),
                                    (this->_matrix(i, i + 2 * (i % 2) - 1) +
                                     this->_matrix(j + 2 * (j % 2) - 1, j)) /
                                        BoundT(2));
        }
      }

      self->_norm_vector[k - 1] = 1;
    }

    // Check for negative cycle
    for (MatrixIndex i = 1; i <= 2 * num_var; ++i) {
      if (this->_matrix(i, i) < BoundT(0)) {
        self->_is_bottom = true;
        self->_is_normalized = true;
        return;
      }
      self->_matrix(i, i) = BoundT(0);
    }

    self->_is_normalized = true;
  }

public:
  void normalize() override { this->unsafe_normalize(); }

  bool is_bottom() const override {
    this->unsafe_normalize();
    return this->_is_bottom;
  }

  bool is_top() const override {
    // TODO(marthaud): This is not correct.
    return !this->_var_index_map.size() && !this->is_bottom();
  }

  void set_to_bottom() override {
    this->_is_bottom = true;
    this->_is_normalized = true;
    this->_matrix.clear();
    this->_var_index_map.clear();
    this->_norm_vector.clear();
  }

  void set_to_top() override {
    this->_is_bottom = false;
    this->_is_normalized = true;
    this->_matrix.clear();
    this->_var_index_map.clear();
    this->_norm_vector.clear();
  }

  bool leq(const Octagon& other) const override {
    // Require normalization of the left operand
    this->unsafe_normalize();

    if (this->_is_bottom) {
      return true;
    }

    if (other._is_bottom) {
      return false;
    }

    // Used to construct a list of variables that appear in both octagons.
    VarIndexMap temp;

    MatrixIndex i1;
    MatrixIndex j1;
    MatrixIndex i2;
    MatrixIndex j2;
    for (auto ito = other._var_index_map.begin();
         ito != other._var_index_map.end();
         ++ito) {
      i2 = ito->second;
      if (this->_var_index_map.find(ito->first) == this->_var_index_map.end()) {
        if (!other._matrix(2 * i2 - 1, 2 * i2).is_infinite() ||
            !other._matrix(2 * i2, 2 * i2 - 1).is_infinite()) {
          // Case: Variable exists and is finite in _other but
          // does not exist in _this.
          return false;
        }
      } else {
        temp.emplace(ito->first, 0);
      }
    }

    for (auto it = temp.begin(); it != temp.end(); ++it) {
      // Case: Variable exists in both `this` and `other`
      i1 = this->_var_index_map.find(it->first)->second;
      i2 = other._var_index_map.find(it->first)->second;

      if (!(this->_matrix(2 * i1 - 1, 2 * i1 - 1) <=
            other._matrix(2 * i2 - 1, 2 * i2 - 1)) ||
          !(this->_matrix(2 * i1 - 1, 2 * i1) <=
            other._matrix(2 * i2 - 1, 2 * i2)) ||
          !(this->_matrix(2 * i1, 2 * i1 - 1) <=
            other._matrix(2 * i2, 2 * i2 - 1)) ||
          !(this->_matrix(2 * i1, 2 * i1) <= other._matrix(2 * i2, 2 * i2))) {
        return false;
      }

      for (auto it2 = it + 1; it2 != temp.end(); ++it2) {
        j1 = this->_var_index_map.find(it2->first)->second;
        j2 = other._var_index_map.find(it2->first)->second;

        if (!(this->_matrix(2 * i1 - 1, 2 * j1 - 1) <=
              other._matrix(2 * i2 - 1, 2 * j2 - 1)) ||
            !(this->_matrix(2 * i1 - 1, 2 * j1) <=
              other._matrix(2 * i2 - 1, 2 * j2)) ||
            !(this->_matrix(2 * i1, 2 * j1 - 1) <=
              other._matrix(2 * i2, 2 * j2 - 1)) ||
            !(this->_matrix(2 * i1, 2 * j1) <= other._matrix(2 * i2, 2 * j2))) {
          return false;
        }
        if (!(this->_matrix(2 * j1 - 1, 2 * i1 - 1) <=
              other._matrix(2 * j2 - 1, 2 * i2 - 1)) ||
            !(this->_matrix(2 * j1 - 1, 2 * i1) <=
              other._matrix(2 * j2 - 1, 2 * i2)) ||
            !(this->_matrix(2 * j1, 2 * i1 - 1) <=
              other._matrix(2 * j2, 2 * i2 - 1)) ||
            !(this->_matrix(2 * j1, 2 * i1) <= other._matrix(2 * j2, 2 * i2))) {
          return false;
        }
      }
    }

    return true;
  }

  bool equals(const Octagon& other) const override {
    return this->leq(other) && other.leq(*this);
  }

private:
  /// \brief Apply a pointwise binary operator
  template < typename BinaryOperator >
  static Octagon pointwise_binary_op(const Octagon& o1,
                                     const Octagon& o2,
                                     const BinaryOperator& op) {
    auto n = Octagon::top();

    // Set intersection of the two maps
    for (auto it = o1._var_index_map.begin(); it != o1._var_index_map.end();
         ++it) {
      if (o2._var_index_map.find(it->first) != o2._var_index_map.end()) {
        n._var_index_map.emplace(it->first, n._var_index_map.size() + 1);
      }
    }

    if (n._var_index_map.empty()) {
      return top();
    }

    n.resize();

    MatrixIndex i1 = 0;
    MatrixIndex i2 = 0;
    MatrixIndex i3 = 0;
    MatrixIndex j1 = 0;
    MatrixIndex j2 = 0;
    MatrixIndex j3 = 0;
    for (auto it = n._var_index_map.begin(); it != n._var_index_map.end();
         ++it) {
      // Finds the union of each 2x2 identity matrix.
      i1 = o1._var_index_map.find(it->first)->second;
      i2 = o2._var_index_map.find(it->first)->second;
      i3 = it->second;

      n._matrix(2 * i3 - 1, 2 * i3 - 1) =
          op(o1._matrix(2 * i1 - 1, 2 * i1 - 1),
             o2._matrix(2 * i2 - 1, 2 * i2 - 1));
      n._matrix(2 * i3 - 1, 2 * i3) =
          op(o1._matrix(2 * i1 - 1, 2 * i1), o2._matrix(2 * i2 - 1, 2 * i2));
      n._matrix(2 * i3, 2 * i3 - 1) =
          op(o1._matrix(2 * i1, 2 * i1 - 1), o2._matrix(2 * i2, 2 * i2 - 1));
      n._matrix(2 * i3, 2 * i3) =
          op(o1._matrix(2 * i1, 2 * i1), o2._matrix(2 * i2, 2 * i2));

      for (auto it2 = it + 1; it2 != n._var_index_map.end(); ++it2) {
        // Finds the union of each pair of 2x2 relational matrices.
        j1 = o1._var_index_map.find(it2->first)->second;
        j2 = o2._var_index_map.find(it2->first)->second;
        j3 = it2->second;

        n._matrix(2 * i3 - 1, 2 * j3 - 1) =
            op(o1._matrix(2 * i1 - 1, 2 * j1 - 1),
               o2._matrix(2 * i2 - 1, 2 * j2 - 1));
        n._matrix(2 * i3 - 1, 2 * j3) =
            op(o1._matrix(2 * i1 - 1, 2 * j1), o2._matrix(2 * i2 - 1, 2 * j2));
        n._matrix(2 * i3, 2 * j3 - 1) =
            op(o1._matrix(2 * i1, 2 * j1 - 1), o2._matrix(2 * i2, 2 * j2 - 1));
        n._matrix(2 * i3, 2 * j3) =
            op(o1._matrix(2 * i1, 2 * j1), o2._matrix(2 * i2, 2 * j2));

        n._matrix(2 * j3 - 1, 2 * i3 - 1) =
            op(o1._matrix(2 * j1 - 1, 2 * i1 - 1),
               o2._matrix(2 * j2 - 1, 2 * i2 - 1));
        n._matrix(2 * j3 - 1, 2 * i3) =
            op(o1._matrix(2 * j1 - 1, 2 * i1), o2._matrix(2 * j2 - 1, 2 * i2));
        n._matrix(2 * j3, 2 * i3 - 1) =
            op(o1._matrix(2 * j1, 2 * i1 - 1), o2._matrix(2 * j2, 2 * i2 - 1));
        n._matrix(2 * j3, 2 * i3) =
            op(o1._matrix(2 * j1, 2 * i1), o2._matrix(2 * j2, 2 * i2));
      }
    }

    return n;
  }

  struct JoinOperator {
    BoundT operator()(const BoundT& x, const BoundT& y) const {
      return max(x, y);
    }
  };

  struct WideningOperator {
    BoundT operator()(const BoundT& x, const BoundT& y) const {
      if (y <= x) {
        return x;
      } else {
        return BoundT::plus_infinity();
      }
    }
  };

  struct WideningThresholdOperator {
    BoundT threshold;

    explicit WideningThresholdOperator(const Number& threshold_)
        : threshold(threshold_) {}

    BoundT operator()(const BoundT& x, const BoundT& y) const {
      if (y <= x) {
        return x;
      } else if (threshold >= y) {
        return threshold;
      } else {
        return BoundT::plus_infinity();
      }
    }
  };

public:
  Octagon join(const Octagon& other) const override {
    // Requires normalization
    this->unsafe_normalize();
    other.unsafe_normalize();

    if (this->_is_bottom) {
      return other;
    } else if (other._is_bottom) {
      return *this;
    } else {
      Octagon oct = this->pointwise_binary_op(*this, other, JoinOperator{});
      oct.set_normalized(true); // Returned matrix is normalized
      return oct;
    }
  }

  void join_with(const Octagon& other) override {
    this->operator=(this->join(other));
  }

  Octagon widening(const Octagon& other) const override {
    // The left operand of the widenning cannot be closed, otherwise
    // termination is not ensured. However, if the right operand is
    // close precision may be improved.
    other.unsafe_normalize();

    if (this->_is_bottom) {
      return other;
    } else if (other._is_bottom) {
      return *this;
    } else {
      Octagon oct = this->pointwise_binary_op(*this, other, WideningOperator{});
      oct.set_normalized(false); // Returned matrix is not normalized
      return oct;
    }
  }

  void widen_with(const Octagon& other) override {
    this->operator=(this->widening(other));
  }

  Octagon widening_threshold(const Octagon& other,
                             const Number& threshold) const override {
    // The left operand of the widenning cannot be closed, otherwise
    // termination is not ensured. However, if the right operand is
    // close precision may be improved.
    other.unsafe_normalize();

    if (this->_is_bottom) {
      return other;
    } else if (other._is_bottom) {
      return *this;
    } else {
      Octagon oct =
          this->pointwise_binary_op(*this,
                                    other,
                                    WideningThresholdOperator{threshold});
      oct.set_normalized(false); // Returned matrix is not normalized
      return oct;
    }
  }

  void widen_threshold_with(const Octagon& other,
                            const Number& threshold) override {
    this->operator=(this->widening_threshold(other, threshold));
  }

  Octagon meet(const Octagon& other) const override {
    // Does not require normalization of any of the two operands
    if (this->_is_bottom || other._is_bottom) {
      return bottom();
    } else {
      auto n = Octagon::top();

      // Set union of the two maps
      for (auto it = this->_var_index_map.begin();
           it != this->_var_index_map.end();
           ++it) {
        n._var_index_map.emplace(it->first, n._var_index_map.size() + 1);
      }
      for (auto it = other._var_index_map.begin();
           it != other._var_index_map.end();
           ++it) {
        n._var_index_map.emplace(it->first, n._var_index_map.size() + 1);
      }

      if (n._var_index_map.empty()) {
        return top();
      }

      n.resize();
      n.set_normalized(false);

      typename VarIndexMap::const_iterator testi1;
      typename VarIndexMap::const_iterator testi2;
      typename VarIndexMap::const_iterator testj1;
      typename VarIndexMap::const_iterator testj2;
      MatrixIndex i1 = 0;
      MatrixIndex i2 = 0;
      MatrixIndex i3 = 0;
      MatrixIndex j1 = 0;
      MatrixIndex j2 = 0;
      MatrixIndex j3 = 0;
      for (auto it = n._var_index_map.begin(); it != n._var_index_map.end();
           ++it) {
        // Finds the intersection on each 2x2 identity matrix.
        testi1 = this->_var_index_map.find(it->first);
        testi2 = other._var_index_map.find(it->first);
        i3 = it->second;

        if (testi1 == this->_var_index_map.end()) {
          i1 = 0;
        } else {
          i1 = testi1->second;
        }
        if (testi2 == other._var_index_map.end()) {
          i2 = 0;
        } else {
          i2 = testi2->second;
        }

        n._matrix(2 * i3 - 1, 2 * i3 - 1) =
            (!i1) ? other._matrix(2 * i2 - 1, 2 * i2 - 1)
                  : ((!i2) ? this->_matrix(2 * i1 - 1, 2 * i1 - 1)
                           : min(this->_matrix(2 * i1 - 1, 2 * i1 - 1),
                                 other._matrix(2 * i2 - 1, 2 * i2 - 1)));
        n._matrix(2 * i3 - 1, 2 * i3) =
            (!i1) ? other._matrix(2 * i2 - 1, 2 * i2)
                  : ((!i2) ? this->_matrix(2 * i1 - 1, 2 * i1)
                           : min(this->_matrix(2 * i1 - 1, 2 * i1),
                                 other._matrix(2 * i2 - 1, 2 * i2)));
        n._matrix(2 * i3, 2 * i3 - 1) =
            (!i1) ? other._matrix(2 * i2, 2 * i2 - 1)
                  : ((!i2) ? this->_matrix(2 * i1, 2 * i1 - 1)
                           : min(this->_matrix(2 * i1, 2 * i1 - 1),
                                 other._matrix(2 * i2, 2 * i2 - 1)));
        n._matrix(2 * i3, 2 * i3) =
            (!i1) ? other._matrix(2 * i2, 2 * i2)
                  : ((!i2) ? this->_matrix(2 * i1, 2 * i1)
                           : min(this->_matrix(2 * i1, 2 * i1),
                                 other._matrix(2 * i2, 2 * i2)));

        for (auto it2 = it + 1; it2 != n._var_index_map.end(); ++it2) {
          // Finds the intersection of each pair of 2x2 relational matrices.
          testj1 = this->_var_index_map.find(it2->first);
          testj2 = other._var_index_map.find(it2->first);
          j3 = it2->second;

          if (testj1 == this->_var_index_map.end()) {
            j1 = 0;
          } else {
            j1 = testj1->second;
          }
          if (testj2 == other._var_index_map.end()) {
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

          n._matrix(2 * i3 - 1, 2 * j3 - 1) =
              (!i1 || !j1)
                  ? other._matrix(2 * i2 - 1, 2 * j2 - 1)
                  : ((!i2 || !j2) ? this->_matrix(2 * i1 - 1, 2 * j1 - 1)
                                  : min(this->_matrix(2 * i1 - 1, 2 * j1 - 1),
                                        other._matrix(2 * i2 - 1, 2 * j2 - 1)));
          n._matrix(2 * i3 - 1, 2 * j3) =
              (!i1 || !j1)
                  ? other._matrix(2 * i2 - 1, 2 * j2)
                  : ((!i2 || !j2) ? this->_matrix(2 * i1 - 1, 2 * j1)
                                  : min(this->_matrix(2 * i1 - 1, 2 * j1),
                                        other._matrix(2 * i2 - 1, 2 * j2)));
          n._matrix(2 * i3, 2 * j3 - 1) =
              (!i1 || !j1)
                  ? other._matrix(2 * i2, 2 * j2 - 1)
                  : ((!i2 || !j2) ? this->_matrix(2 * i1, 2 * j1 - 1)
                                  : min(this->_matrix(2 * i1, 2 * j1 - 1),
                                        other._matrix(2 * i2, 2 * j2 - 1)));
          n._matrix(2 * i3, 2 * j3) =
              (!i1 || !j1)
                  ? other._matrix(2 * i2, 2 * j2)
                  : ((!i2 || !j2) ? this->_matrix(2 * i1, 2 * j1)
                                  : min(this->_matrix(2 * i1, 2 * j1),
                                        other._matrix(2 * i2, 2 * j2)));

          n._matrix(2 * j3 - 1, 2 * i3 - 1) =
              (!i1 || !j1)
                  ? other._matrix(2 * j2 - 1, 2 * i2 - 1)
                  : ((!i2 || !j2) ? this->_matrix(2 * j1 - 1, 2 * i1 - 1)
                                  : min(this->_matrix(2 * j1 - 1, 2 * i1 - 1),
                                        other._matrix(2 * j2 - 1, 2 * i2 - 1)));
          n._matrix(2 * j3 - 1, 2 * i3) =
              (!i1 || !j1)
                  ? other._matrix(2 * j2 - 1, 2 * i2)
                  : ((!i2 || !j2) ? this->_matrix(2 * j1 - 1, 2 * i1)
                                  : min(this->_matrix(2 * j1 - 1, 2 * i1),
                                        other._matrix(2 * j2 - 1, 2 * i2)));
          n._matrix(2 * j3, 2 * i3 - 1) =
              (!i1 || !j1)
                  ? other._matrix(2 * j2, 2 * i2 - 1)
                  : ((!i2 || !j2) ? this->_matrix(2 * j1, 2 * i1 - 1)
                                  : min(this->_matrix(2 * j1, 2 * i1 - 1),
                                        other._matrix(2 * j2, 2 * i2 - 1)));
          n._matrix(2 * j3, 2 * i3) =
              (!i1 || !j1)
                  ? other._matrix(2 * j2, 2 * i2)
                  : ((!i2 || !j2) ? this->_matrix(2 * j1, 2 * i1)
                                  : min(this->_matrix(2 * j1, 2 * i1),
                                        other._matrix(2 * j2, 2 * i2)));
        }
      }

      return n;
    }
  }

  void meet_with(const Octagon& other) override {
    this->operator=(this->meet(other));
  }

  Octagon narrowing(const Octagon& other) const override {
    // Does not require normalization of any of the two operands
    if (this->_is_bottom || other._is_bottom) {
      return bottom();
    } else {
      auto n = Octagon::top();

      // Set union of the two maps
      for (auto it = this->_var_index_map.begin();
           it != this->_var_index_map.end();
           ++it) {
        n._var_index_map.emplace(it->first, n._var_index_map.size() + 1);
      }
      for (auto it = other._var_index_map.begin();
           it != other._var_index_map.end();
           ++it) {
        n._var_index_map.emplace(it->first, n._var_index_map.size() + 1);
      }
      if (n._var_index_map.empty()) {
        return top();
      }

      n.resize();
      n.set_normalized(false);

      typename VarIndexMap::const_iterator testi1;
      typename VarIndexMap::const_iterator testi2;
      typename VarIndexMap::const_iterator testj1;
      typename VarIndexMap::const_iterator testj2;
      MatrixIndex i1 = 0;
      MatrixIndex i2 = 0;
      MatrixIndex i3 = 0;
      MatrixIndex j1 = 0;
      MatrixIndex j2 = 0;
      MatrixIndex j3 = 0;
      for (auto it = n._var_index_map.begin(); it != n._var_index_map.end();
           ++it) {
        // Finds the narrowing on each 2x2 identity matrix.
        testi1 = this->_var_index_map.find(it->first);
        testi2 = other._var_index_map.find(it->first);
        i3 = it->second;

        if (testi1 == this->_var_index_map.end()) {
          i1 = 0;
        } else {
          i1 = testi1->second;
        }
        if (testi2 == other._var_index_map.end()) {
          i2 = 0;
        } else {
          i2 = testi2->second;
        }

        n._matrix(2 * i3 - 1, 2 * i3 - 1) =
            (!i2)
                ? this->_matrix(2 * i1 - 1, 2 * i1 - 1)
                : ((!i1 || this->_matrix(2 * i1 - 1, 2 * i1 - 1).is_infinite())
                       ? other._matrix(2 * i2 - 1, 2 * i2 - 1)
                       : this->_matrix(2 * i1 - 1, 2 * i1 - 1));
        n._matrix(2 * i3 - 1, 2 * i3) =
            (!i2) ? this->_matrix(2 * i1 - 1, 2 * i1)
                  : ((!i1 || this->_matrix(2 * i1 - 1, 2 * i1).is_infinite())
                         ? other._matrix(2 * i2 - 1, 2 * i2)
                         : this->_matrix(2 * i1 - 1, 2 * i1));
        n._matrix(2 * i3, 2 * i3 - 1) =
            (!i2) ? this->_matrix(2 * i1, 2 * i1 - 1)
                  : ((!i1 || this->_matrix(2 * i1, 2 * i1 - 1).is_infinite())
                         ? other._matrix(2 * i2, 2 * i2 - 1)
                         : this->_matrix(2 * i1, 2 * i1 - 1));
        n._matrix(2 * i3, 2 * i3) =
            (!i2) ? this->_matrix(2 * i1, 2 * i1)
                  : ((!i1 || this->_matrix(2 * i1, 2 * i1).is_infinite())
                         ? other._matrix(2 * i2, 2 * i2)
                         : this->_matrix(2 * i1, 2 * i1));

        for (auto it2 = it + 1; it2 != n._var_index_map.end(); ++it2) {
          // Finds the narrowing of each pair of 2x2 relational matrices.
          testj1 = this->_var_index_map.find(it2->first);
          testj2 = other._var_index_map.find(it2->first);
          j3 = it2->second;

          if (testj1 == this->_var_index_map.end()) {
            j1 = 0;
          } else {
            j1 = testj1->second;
          }
          if (testj2 == other._var_index_map.end()) {
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

          n._matrix(2 * i3 - 1, 2 * j3 - 1) =
              (!i2 || !j2)
                  ? this->_matrix(2 * i1 - 1, 2 * j1 - 1)
                  : ((!i1 || !j1 ||
                      this->_matrix(2 * i1 - 1, 2 * j1 - 1).is_infinite())
                         ? other._matrix(2 * i2 - 1, 2 * j2 - 1)
                         : this->_matrix(2 * i1 - 1, 2 * j1 - 1));
          n._matrix(2 * i3 - 1, 2 * j3) =
              (!i2 || !j2) ? this->_matrix(2 * i1 - 1, 2 * j1)
                           : ((!i1 || !j1 ||
                               this->_matrix(2 * i1 - 1, 2 * j1).is_infinite())
                                  ? other._matrix(2 * i2 - 1, 2 * j2)
                                  : this->_matrix(2 * i1 - 1, 2 * j1));
          n._matrix(2 * i3, 2 * j3 - 1) =
              (!i2 || !j2) ? this->_matrix(2 * i1, 2 * j1 - 1)
                           : ((!i1 || !j1 ||
                               this->_matrix(2 * i1, 2 * j1 - 1).is_infinite())
                                  ? other._matrix(2 * i2, 2 * j2 - 1)
                                  : this->_matrix(2 * i1, 2 * j1 - 1));
          n._matrix(2 * i3, 2 * j3) =
              (!i2 || !j2)
                  ? this->_matrix(2 * i1, 2 * j1)
                  : ((!i1 || !j1 || this->_matrix(2 * i1, 2 * j1).is_infinite())
                         ? other._matrix(2 * i2, 2 * j2)
                         : this->_matrix(2 * i1, 2 * j1));

          n._matrix(2 * j3 - 1, 2 * i3 - 1) =
              (!i2 || !j2)
                  ? this->_matrix(2 * j1 - 1, 2 * i1 - 1)
                  : ((!i1 || !j1 ||
                      this->_matrix(2 * j1 - 1, 2 * i1 - 1).is_infinite())
                         ? other._matrix(2 * j2 - 1, 2 * i2 - 1)
                         : this->_matrix(2 * j1 - 1, 2 * i1 - 1));
          n._matrix(2 * j3 - 1, 2 * i3) =
              (!i2 || !j2) ? this->_matrix(2 * j1 - 1, 2 * i1)
                           : ((!i1 || !j1 ||
                               this->_matrix(2 * j1 - 1, 2 * i1).is_infinite())
                                  ? other._matrix(2 * j2 - 1, 2 * i2)
                                  : this->_matrix(2 * j1 - 1, 2 * i1));
          n._matrix(2 * j3, 2 * i3 - 1) =
              (!i2 || !j2) ? this->_matrix(2 * j1, 2 * i1 - 1)
                           : ((!i1 || !j1 ||
                               this->_matrix(2 * j1, 2 * i1 - 1).is_infinite())
                                  ? other._matrix(2 * j2, 2 * i2 - 1)
                                  : this->_matrix(2 * j1, 2 * i1 - 1));
          n._matrix(2 * j3, 2 * i3) =
              (!i2 || !j2)
                  ? this->_matrix(2 * j1, 2 * i1)
                  : ((!i1 || !j1 || this->_matrix(2 * j1, 2 * i1).is_infinite())
                         ? other._matrix(2 * j2, 2 * i2)
                         : this->_matrix(2 * j1, 2 * i1));
        }
      }

      return n;
    }
  }

  void narrow_with(const Octagon& other) override {
    this->operator=(this->narrowing(other));
  }

  Octagon narrowing_threshold(const Octagon& other,
                              const Number& /*threshold*/) const override {
    // TODO(marthaud): Implement
    return this->narrowing(other);
  }

  void narrow_threshold_with(const Octagon& other,
                             const Number& /*threshold*/) override {
    // TODO(marthaud): Implement
    this->narrow_with(other);
  }

private:
  /// \brief Abstract the variable
  boost::optional< typename VarIndexMap::iterator > abstract(VariableRef v) {
    // Requires normalization.
    auto it = this->_var_index_map.find(v);
    if (it != this->_var_index_map.end()) {
      this->unsafe_normalize();
      MatrixIndex n = it->second;
      MatrixIndex odd = 2 * n - 1;
      MatrixIndex even = 2 * n;
      MatrixIndex size = 2 * this->_matrix.size();
      for (MatrixIndex idx = 1; idx <= size; ++idx) {
        this->_matrix(idx, odd) = BoundT::plus_infinity();
        this->_matrix(idx, even) = BoundT::plus_infinity();
        this->_matrix(odd, idx) = BoundT::plus_infinity();
        this->_matrix(even, idx) = BoundT::plus_infinity();
      }
      this->_matrix(odd, odd) = BoundT(0);
      this->_matrix(even, even) = BoundT(0);
      return boost::optional< typename VarIndexMap::iterator >(it);
    }
    // Maintains normalization.
    return boost::none;
  }

  void apply_constraint(MatrixIndex var, bool is_positive, BoundT constraint) {
    // Application of single variable octagonal constraints.
    constraint *= BoundT(2);
    if (is_positive) { // 2*v1 <= constraint
      this->_matrix(2 * var, 2 * var - 1) =
          min(this->_matrix(2 * var, 2 * var - 1), constraint);
    } else { // 2*v1 >= constraint
      this->_matrix(2 * var - 1, 2 * var) =
          min(this->_matrix(2 * var - 1, 2 * var), constraint);
    }

    if (this->_matrix(2 * var, 2 * var - 1) <
        -this->_matrix(2 * var - 1, 2 * var)) {
      this->set_to_bottom();
    }
  }

  void apply_constraint(MatrixIndex i,
                        MatrixIndex j,
                        bool is1_positive,
                        bool is2_positive,
                        const BoundT& constraint) {
    // Application of double variable octagonal constraints.
    if (is1_positive && is2_positive) { // v1 + v2 <= constraint
      this->_matrix(2 * j, 2 * i - 1) =
          min(this->_matrix(2 * j, 2 * i - 1), constraint);
      this->_matrix(2 * i, 2 * j - 1) =
          min(this->_matrix(2 * i, 2 * j - 1), constraint);
    } else if (is1_positive && !is2_positive) { // v1 - v2 <= constraint
      this->_matrix(2 * j - 1, 2 * i - 1) =
          min(this->_matrix(2 * j - 1, 2 * i - 1), constraint);
      this->_matrix(2 * i, 2 * j) =
          min(this->_matrix(2 * i, 2 * j), constraint);
    } else if (!is1_positive && is2_positive) { // v2 - v1 <= constraint
      this->_matrix(2 * i - 1, 2 * j - 1) =
          min(this->_matrix(2 * i - 1, 2 * j - 1), constraint);
      this->_matrix(2 * j, 2 * i) =
          min(this->_matrix(2 * j, 2 * i), constraint);
    } else if (!is1_positive && !is2_positive) { // -v1 - v2 <= constraint
      this->_matrix(2 * j - 1, 2 * i) =
          min(this->_matrix(2 * j - 1, 2 * i), constraint);
      this->_matrix(2 * i - 1, 2 * j) =
          min(this->_matrix(2 * i - 1, 2 * j), constraint);
    }
    if (this->_matrix(2 * j, 2 * i - 1) < -this->_matrix(2 * j - 1, 2 * i) ||
        this->_matrix(2 * j - 1, 2 * i - 1) <
            -this->_matrix(2 * i - 1, 2 * j - 1)) {
      this->set_to_bottom();
    }
  }

public:
  void assign(VariableRef x, int n) override {
    this->assign(x, LinearExpressionT(n));
  }

  void assign(VariableRef x, const Number& n) override {
    this->assign(x, LinearExpressionT(n));
  }

  void assign(VariableRef x, VariableRef y) override {
    this->assign(x, LinearExpressionT(y));
  }

  void assign(VariableRef x, const LinearExpressionT& e) override {
    if (this->_is_bottom) {
      return;
    }

    boost::optional< VariableRef > v = e.variable();
    if (v && *v == x) {
      return;
    }

    // add x in the matrix if not found
    auto it = this->_var_index_map.find(x);
    MatrixIndex i;
    if (it == this->_var_index_map.end()) {
      i = this->_var_index_map.emplace(x, this->_var_index_map.size() + 1)
              .first->second;
      this->resize();
    } else {
      i = it->second;
    }

    this->abstract(x); // call unsafe_normalize()

    if (e.is_constant()) {
      this->apply_constraint(i, true, BoundT(e.constant())); // adding  x <= c
      this->apply_constraint(i,
                             false,
                             BoundT(-e.constant())); // adding -x <= -c
    } else if (v) {
      VariableRef y = *v;
      auto itz = this->_var_index_map.find(y);
      if (itz == this->_var_index_map.end()) {
        return; // x has been already abstracted
      }
      MatrixIndex j = itz->second;
      this->apply_constraint(i, j, true, false, BoundT(0));
      this->apply_constraint(i, j, false, true, BoundT(0));
    } else {
      // Projection using intervals, requires normalization
      this->unsafe_normalize();

      if (this->_is_bottom) {
        return;
      }

      this->set(x, this->to_interval(e));
    }

    this->set_normalized(false);
  }

private:
  void add_var(VariableRef x,
               MatrixIndex i,
               MatrixIndex j,
               const BoundT& lb,
               const BoundT& ub,
               bool op_eq) {
    if (lb.is_minus_infinity() && ub.is_plus_infinity()) {
      this->abstract(x);
      return;
    }

    if (op_eq) {
      for (MatrixIndex j_idx = 1; j_idx <= 2 * this->_matrix.size(); ++j_idx) {
        if (j_idx != 2 * j && j_idx != 2 * j - 1) {
          this->_matrix(2 * j - 1, j_idx) -= lb;
          this->_matrix(2 * j, j_idx) += ub;
        }
      }

      for (MatrixIndex i_idx = 1; i_idx <= 2 * _matrix.size(); ++i_idx) {
        if (i_idx != 2 * j && i_idx != 2 * j - 1) {
          this->_matrix(i_idx, 2 * j) -= lb;
          this->_matrix(i_idx, 2 * j - 1) += ub;
        }
      }

      this->_matrix(2 * j - 1, 2 * j) -= BoundT(2) * lb;
      this->_matrix(2 * j, 2 * j - 1) += BoundT(2) * ub;
    } else {
      this->abstract(x);
      this->_matrix(2 * j - 1, 2 * i - 1) = -lb;
      this->_matrix(2 * i, 2 * j) = -lb;
      this->_matrix(2 * i - 1, 2 * j - 1) = ub;
      this->_matrix(2 * j, 2 * i) = ub;
    }
  }

  void subtract_var(VariableRef x,
                    MatrixIndex i,
                    MatrixIndex j,
                    const BoundT& lb,
                    const BoundT& ub,
                    bool op_eq) {
    this->add_var(x, i, j, -ub, -lb, op_eq);
  }

  void multiply_var(VariableRef x,
                    MatrixIndex i,
                    MatrixIndex j,
                    const BoundT& lb,
                    const BoundT& ub,
                    bool op_eq) {
    if (op_eq) {
      BoundT t1 = this->_matrix(2 * j - 1, 2 * j);
      BoundT t2 = this->_matrix(2 * j, 2 * j - 1);
      this->abstract(x);
      BoundT ll = (t1 / BoundT(-2)) * lb;
      BoundT lu = (t1 / BoundT(-2)) * ub;
      BoundT ul = (t2 / BoundT(2)) * lb;
      BoundT uu = (t2 / BoundT(2)) * ub;
      this->_matrix(2 * j - 1, 2 * j) = min(ll, lu, ul, uu) * BoundT(-2);
      this->_matrix(2 * j, 2 * j - 1) = max(ll, lu, ul, uu) * BoundT(2);
    } else {
      BoundT t1 = -this->_matrix(2 * i - 1, 2 * i);
      BoundT t2 = this->_matrix(2 * i, 2 * i - 1);
      this->abstract(x);
      BoundT ll = (t1 / BoundT(-2)) * lb;
      BoundT lu = (t1 / BoundT(-2)) * ub;
      BoundT ul = (t2 / BoundT(2)) * lb;
      BoundT uu = (t2 / BoundT(2)) * ub;
      this->_matrix(2 * j - 1, 2 * j) = min(ll, lu, ul, uu) * BoundT(-2);
      this->_matrix(2 * j, 2 * j - 1) = max(ll, lu, ul, uu) * BoundT(2);
    }
  }

  void divide_var(VariableRef x,
                  MatrixIndex i,
                  MatrixIndex j,
                  const BoundT& _lb,
                  const BoundT& _ub,
                  bool op_eq) {
    IntervalT trim_intv = trim_bound(IntervalT(_lb, _ub), BoundT(0));

    if (trim_intv.is_bottom()) {
      // definite division by zero
      this->set_to_bottom();
      return;
    }

    IntervalT zero(Number(0));
    if (zero.leq(trim_intv)) {
      this->abstract(x);
      return;
    }
    const BoundT& lb = trim_intv.lb();
    const BoundT& ub = trim_intv.ub();
    if (op_eq) {
      BoundT t1 = this->_matrix(2 * j - 1, 2 * j);
      BoundT t2 = this->_matrix(2 * j, 2 * j - 1);
      this->abstract(x);
      BoundT ll = (t1 / BoundT(-2)) / lb;
      BoundT lu = (t1 / BoundT(-2)) / ub;
      BoundT ul = (t2 / BoundT(2)) / lb;
      BoundT uu = (t2 / BoundT(2)) / ub;
      this->_matrix(2 * j - 1, 2 * j) = min(ll, lu, ul, uu) * BoundT(-2);
      this->_matrix(2 * j, 2 * j - 1) = max(ll, lu, ul, uu) * BoundT(2);
    } else {
      BoundT t1 = -this->_matrix(2 * i - 1, 2 * i);
      BoundT t2 = this->_matrix(2 * i, 2 * i - 1);
      this->abstract(x);
      BoundT ll = (t1 / BoundT(-2)) / lb;
      BoundT lu = (t1 / BoundT(-2)) / ub;
      BoundT ul = (t2 / BoundT(2)) / lb;
      BoundT uu = (t2 / BoundT(2)) / ub;
      this->_matrix(2 * j - 1, 2 * j) = min(ll, lu, ul, uu) * BoundT(-2);
      this->_matrix(2 * j, 2 * j - 1) = max(ll, lu, ul, uu) * BoundT(2);
    }
  }

  /// \brief x = y op [lb, ub]
  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             const BoundT& lb,
             const BoundT& ub) {
    // Requires normalization.

    // add x in the DBM if not found
    if (this->_var_index_map.find(x) == this->_var_index_map.end()) {
      this->_var_index_map.emplace(x, this->_var_index_map.size() + 1);
      this->resize();
    }

    if (this->_var_index_map.find(y) == this->_var_index_map.end()) {
      this->abstract(x);
      return;
    }

    MatrixIndex i = this->_var_index_map.find(x)->second;
    MatrixIndex j = this->_var_index_map.find(y)->second;
    this->unsafe_normalize();

    switch (op) {
      case BinaryOperator::Add: {
        this->add_var(x, i, j, lb, ub, i == j);
      } break;
      case BinaryOperator::Sub: {
        this->subtract_var(x, i, j, lb, ub, i == j);
      } break;
      case BinaryOperator::Mul: {
        this->multiply_var(x, i, j, lb, ub, i == j);
      } break;
      case BinaryOperator::Div: {
        this->divide_var(x, i, j, lb, ub, i == j);
      } break;
      case BinaryOperator::Mod:
      case BinaryOperator::Rem:
      case BinaryOperator::Shl:
      case BinaryOperator::Shr:
      case BinaryOperator::And:
      case BinaryOperator::Or:
      case BinaryOperator::Xor: {
        this->set(x,
                  apply_bin_operator(op,
                                     this->to_interval(y),
                                     IntervalT(lb, ub)));
      } break;
      default: {
        ikos_unreachable("invalid operation");
      }
    }

    this->_norm_vector[i - 1] = 0;
    // Result is not normalized.
  }

public:
  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             VariableRef z) override {
    // Requires normalization.

    typename VarIndexMap::iterator itz = this->_var_index_map.find(z);
    if (itz == this->_var_index_map.end()) {
      this->abstract(x);
      return;
    }
    MatrixIndex n = itz->second;

    if (x != y) {
      this->assign(x, y);
      this->apply(op,
                  x,
                  x,
                  this->_matrix(2 * n - 1, 2 * n) / BoundT(-2),
                  this->_matrix(2 * n, 2 * n - 1) / BoundT(2));
    } else {
      this->apply(op,
                  x,
                  y,
                  this->_matrix(2 * n - 1, 2 * n) / BoundT(-2),
                  this->_matrix(2 * n, 2 * n - 1) / BoundT(2));
    }
    // Sets state to not normalized.
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             const Number& z) override {
    // Requires normalization.

    if (x != y) {
      this->assign(x, y);
      this->apply(op, x, x, BoundT(z), BoundT(z));
    } else {
      this->apply(op, x, y, BoundT(z), BoundT(z));
    }
    // Sets state to not normalized.
  }

  void apply(BinaryOperator op,
             VariableRef x,
             const Number& y,
             VariableRef z) override {
    // Requires normalization.

    if (x != z) {
      this->assign(x, y);
      this->apply(op, x, x, z);
    } else {
      this->set(x, apply_bin_operator(op, IntervalT(y), this->to_interval(z)));
    }
    // Sets state to not normalized.
  }

private:
  /// \brief Check satisfiability of cst using intervals
  ///
  /// Only to be used if cst is too hard for octagons
  bool check_sat(const LinearConstraintT& cst) {
    auto inv = IntervalDomainT::top();
    this->unsafe_normalize();
    for (const auto& term : cst) {
      inv.set(term.first, this->to_interval(term.first));
    }
    inv.add(cst);
    return !inv.is_bottom();
  }

public:
  void add(const LinearConstraintT& cst) override {
    // Does not require normalization.
    if (this->_is_bottom) {
      return;
    }

    bool v1 = false;
    bool v2 = false;
    bool is1_positive = false;
    bool is2_positive = false;
    MatrixIndex i = 0;
    MatrixIndex j = 0;
    for (const auto& term : cst) {
      if (!v1) {
        // Calculates and loads information for the first variable.
        if (term.second == -1) {
          is1_positive = false;
        } else if (term.second == 1) {
          is1_positive = true;
        } else {
          ikos_unreachable(
              "expr contains unexpected coefficient (accepted values are -1, "
              "0, and 1).");
        }

        i = this->_var_index_map.emplace(term.first, _var_index_map.size() + 1)
                .first->second;
        v1 = true;
      } else if (!v2) {
        // Calculates and loads information for the second variable,
        // if it exists.
        if (term.second == -1) {
          is2_positive = false;
        } else if (term.second == 1) {
          is2_positive = true;
        } else {
          ikos_unreachable(
              "expr contains unexpected coefficient (accepted values are -1, "
              "0, and 1).");
        }

        j = this->_var_index_map.emplace(term.first, _var_index_map.size() + 1)
                .first->second;
        v2 = true;
      } else {
        ikos_unreachable("constraint is not an octagon constraint");
      }
    }
    if (!v1) {
      if (cst.is_contradiction()) {
        this->set_to_bottom();
      }
      return;
    }
    this->resize();
    BoundT constant(cst.constant());
    BoundT neg_constant(-cst.constant());

    if (cst.is_inequality()) { // Applies inequality constraints in the form of
                               // octagonal constraints.
      if (v1 && !v2) {
        this->apply_constraint(i, is1_positive, constant);
      } else /*if(v1 && v2)*/ {
        this->apply_constraint(i, j, is1_positive, is2_positive, constant);
      }
    } else if (cst.is_equality()) { // Applies equality constraints as two
                                    // octagonal constraints.
      if (v1 && !v2) {
        this->apply_constraint(i, is1_positive, constant);
        this->apply_constraint(i, !is1_positive, neg_constant);
      } else /*if(v1 && v2)*/ {
        this->apply_constraint(i, j, is1_positive, is2_positive, constant);
        this->apply_constraint(i,
                               j,
                               !is1_positive,
                               !is2_positive,
                               neg_constant);
      }
    } else if (cst.is_disequation()) {
      // we use intervals to reason about disequations
      if (!this->check_sat(cst)) {
        this->set_to_bottom();
        return;
      }
    }
    this->_is_normalized = false;
  }

  void add(const LinearConstraintSystemT& csts) override {
    // Does not require normalization.
    for (const LinearConstraintT& cst : csts) {
      this->add(cst);
    }
  } // Sets state to not normalized.

  void set(VariableRef x, const IntervalT& value) override {
    if (this->_is_bottom) {
      return;
    }
    if (value.is_bottom()) {
      this->set_to_bottom();
      return;
    }
    // add x in the matrix if not found
    auto it = this->_var_index_map.find(x);
    MatrixIndex idx;
    if (it == this->_var_index_map.end()) {
      idx = this->_var_index_map.emplace(x, this->_var_index_map.size() + 1)
                .first->second;
      this->resize();
    } else {
      idx = it->second;
    }
    this->abstract(x);                               // normalize
    this->apply_constraint(idx, true, value.ub());   // x <= ub
    this->apply_constraint(idx, false, -value.lb()); // -x <= -lb
  }

  void set(VariableRef x, const CongruenceT& value) override {
    if (this->_is_bottom) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      boost::optional< Number > n = value.singleton();
      if (n) {
        this->assign(x, *n);
      } else {
        this->forget(x);
      }
    }
  }

  void set(VariableRef x, const IntervalCongruenceT& value) override {
    this->set(x, value.interval());
  }

  void refine(VariableRef /*x*/, const IntervalT& /*value*/) override {
    // TODO(marthaud)
  }

  void refine(VariableRef /*x*/, const CongruenceT& /*value*/) override {
    // TODO(marthaud)
  }

  void refine(VariableRef /*x*/,
              const IntervalCongruenceT& /*value*/) override {
    // TODO(marthaud)
  }

  void forget(VariableRef x) override {
    if (boost::optional< typename VarIndexMap::iterator > it =
            this->abstract(x)) {
      MatrixIndex n = (*it)->second;
      this->_matrix -= n;
      this->_var_index_map.erase(*it);
      // update the values in _var_index_map
      for (auto itz = this->_var_index_map.begin();
           itz != this->_var_index_map.end();
           ++itz) {
        if (itz->second > n) {
          this->_var_index_map[itz->first]--;
        }
      }
      this->_norm_vector.resize(this->_var_index_map.size(), 0);
      this->_is_normalized = false;
    }
  }

  IntervalT to_interval(VariableRef x) const override {
    // projection requires normalization.
    auto it = this->_var_index_map.find(x);
    if (it == this->_var_index_map.end()) {
      return IntervalT::top();
    } else {
      MatrixIndex idx = it->second;
      return IntervalT(this->_matrix(2 * idx - 1, 2 * idx) / BoundT(-2),
                       this->_matrix(2 * idx, 2 * idx - 1) / BoundT(2));
    }
  }

  IntervalT to_interval(const LinearExpressionT& e) const override {
    return Parent::to_interval(e);
  }

  CongruenceT to_congruence(VariableRef) const override {
    return CongruenceT::top();
  }

  CongruenceT to_congruence(const LinearExpressionT& /*e*/) const override {
    return CongruenceT::top();
  }

  IntervalCongruenceT to_interval_congruence(VariableRef x) const override {
    return IntervalCongruenceT(this->to_interval(x));
  }

  IntervalCongruenceT to_interval_congruence(
      const LinearExpressionT& e) const override {
    return Parent::to_interval_congruence(e);
  }

  LinearConstraintSystemT to_linear_constraint_system() const override {
    this->unsafe_normalize();

    if (this->is_bottom()) {
      return LinearConstraintSystemT{LinearConstraintT::contradiction()};
    }

    LinearConstraintSystemT csts;

    BoundT lb(0);
    BoundT rb(0);
    MatrixIndex idx1 = 0;
    MatrixIndex idx2 = 0;
    for (auto it = this->_var_index_map.begin();
         it != this->_var_index_map.end();
         ++it) {
      idx1 = it->second;
      lb = this->_matrix(2 * idx1 - 1, 2 * idx1) / BoundT(-2);
      rb = this->_matrix(2 * idx1, 2 * idx1 - 1) / BoundT(2);
      // lb <= v <= rb
      csts.add(within_interval(it->first, IntervalT(lb, rb)));

      for (auto it2 = it + 1; it2 != this->_var_index_map.end(); ++it2) {
        idx2 = it2->second;

        // v1 - v2
        lb = -this->_matrix(2 * idx2, 2 * idx1);
        rb = this->_matrix(2 * idx2 - 1, 2 * idx1 - 1);
        csts.add(within_interval(VariableExprT(it->first) -
                                     VariableExprT(it2->first),
                                 IntervalT(lb, rb)));

        // v1 + v2
        lb = -this->_matrix(2 * idx2 - 1, 2 * idx1);
        rb = this->_matrix(2 * idx2, 2 * idx1 - 1);
        csts.add(within_interval(VariableExprT(it->first) +
                                     VariableExprT(it2->first),
                                 IntervalT(lb, rb)));
      }
    }

    return csts;
  }

  void dump(std::ostream& o) const override {
#ifdef VERBOSE
    /// For debugging purposes
    /// print internal datastructures
    o << "DBM:\n{";
    for (auto it = this->_var_index_map.begin();
         it != this->_var_index_map.end();
         ++it) {
      MatrixIndex i_plus = (2 * it->second);
      MatrixIndex i_minus = (2 * it->second) - 1;
      o << it->first << "-"
        << " -> " << i_minus << ";";
      o << it->first << "+"
        << " -> " << i_plus << ";";
    }
    o << "}\n";
    this->_matrix.dump(o);
    o << "\n";
#else
    this->to_linear_constraint_system().dump(o);
#endif
  }

  static std::string name() {
    return "octagon";
  }

}; // end class Octagon

} // end namespace numeric
} // end namespace core
} // end namespace ikos
