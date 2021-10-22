/*******************************************************************************
 *
 * \file
 * \brief Standard domain of Difference-Bound Matrices
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

#include <boost/iterator/transform_iterator.hpp>

#include <ikos/core/domain/numeric/abstract_domain.hpp>
#include <ikos/core/domain/numeric/linear_interval_solver.hpp>
#include <ikos/core/number/bound.hpp>
#include <ikos/core/support/assert.hpp>
#include <ikos/core/value/numeric/congruence.hpp>
#include <ikos/core/value/numeric/interval.hpp>
#include <ikos/core/value/numeric/interval_congruence.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief Difference-Bound Matrices abstract domain
template < typename Number,
           typename VariableRef,
           std::size_t MaxReductionCycles = 10 >
class DBM final : public numeric::AbstractDomain<
                      Number,
                      VariableRef,
                      DBM< Number, VariableRef, MaxReductionCycles > > {
public:
  using BoundT = Bound< Number >;
  using IntervalT = Interval< Number >;
  using CongruenceT = Congruence< Number >;
  using IntervalCongruenceT = IntervalCongruence< Number >;
  using VariableExprT = VariableExpression< Number, VariableRef >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;
  using LinearConstraintSystemT = LinearConstraintSystem< Number, VariableRef >;

private:
  /// \brief Index of a variable in the matrix
  using MatrixIndex = unsigned;

  // \brief Map from variable to index
  using VarIndexMap = boost::container::flat_map< VariableRef, MatrixIndex >;

  /// \brief Solver
  using LinearIntervalSolverT =
      LinearIntervalSolver< Number, VariableRef, DBM >;

  /// \brief Parent
  using Parent = numeric::AbstractDomain< Number, VariableRef, DBM >;

  class Matrix {
  private:
    std::vector< BoundT > _matrix;
    MatrixIndex _num_vars = 0; // size of the matrix

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

    /// \brief Return the number of variables in the matrix
    MatrixIndex num_vars() const { return this->_num_vars; }

    /// \brief Return the element (i, j)
    const BoundT& operator()(MatrixIndex i, MatrixIndex j) const {
      ikos_assert_msg(i < this->_num_vars && j < this->_num_vars,
                      "ouf of bounds matrix access");
      return this->_matrix[this->_num_vars * i + j];
    }

    /// \brief Return the element (i, j)
    BoundT& operator()(MatrixIndex i, MatrixIndex j) {
      ikos_assert_msg(i < this->_num_vars && j < this->_num_vars,
                      "ouf of bounds matrix access");
      return this->_matrix[this->_num_vars * i + j];
    }

    /// \brief Clear the matrix
    void clear() {
      this->_num_vars = 0;
      this->_matrix.clear();
    }

    /// \brief Clear and resize the matrix
    void clear_resize(MatrixIndex num_vars) {
      this->_num_vars = num_vars;
      this->_matrix.clear();
      this->_matrix.resize(num_vars * num_vars, BoundT::plus_infinity());
    }

    /// \brief Resize the matrix to handle a new variable
    ///
    /// \returns the index of the new variable
    MatrixIndex add_variable() {
      if (this->_num_vars == 0) {
        this->_num_vars = 2;
        this->_matrix.resize(this->_num_vars * this->_num_vars,
                             BoundT::plus_infinity());
      } else {
        std::vector< BoundT > new_matrix((this->_num_vars + 1) *
                                             (this->_num_vars + 1),
                                         BoundT::plus_infinity());

        for (MatrixIndex i = 0; i < this->_num_vars; i++) {
          for (MatrixIndex j = 0; j < this->_num_vars; j++) {
            new_matrix[(this->_num_vars + 1) * i + j] =
                std::move(this->_matrix[this->_num_vars * i + j]);
          }
        }

        std::swap(this->_matrix, new_matrix);
        this->_num_vars++;
      }

      return this->_num_vars - 1;
    }

    /// \brief Apply Floyd-Warshall algorithm to normalize the matrix
    void normalize() {
      const MatrixIndex n = this->_num_vars;

      for (MatrixIndex i = 0; i < n; i++) {
        this->_matrix[n * i + i] = BoundT(0);
      }

      for (MatrixIndex k = 0; k < n; k++) {
        for (MatrixIndex i = 0; i < n; i++) {
          for (MatrixIndex j = 0; j < n; j++) {
            this->_matrix[n * i + j] =
                min(this->_matrix[n * i + j],
                    this->_matrix[n * i + k] + this->_matrix[n * k + j]);
          }
        }
      }
    }

    /// \brief Return true if the matrix has a negative cycle
    ///
    /// Precondition: matrix is normalized
    bool has_negative_cycle() const {
      for (MatrixIndex i = 0; i < this->_num_vars; i++) {
        if (this->operator()(i, i) < BoundT(0)) {
          return true;
        }
      }

      return false;
    }

    /// \brief Return true if the matrix only contains +oo
    bool all_plus_infinity() const {
      for (MatrixIndex i = 0; i < this->_num_vars; i++) {
        for (MatrixIndex j = 0; j < this->_num_vars; j++) {
          if (i != j && !this->operator()(i, j).is_plus_infinity()) {
            return false;
          }
        }
      }
      return true;
    }

    /// \brief Return true if all M[i, j] and M[j, i] are +oo, for all j
    bool all_plus_infinity(MatrixIndex i) const {
      for (MatrixIndex j = 0; j < this->_num_vars; j++) {
        if (i == j) {
          continue;
        }
        if (!this->operator()(i, j).is_plus_infinity() ||
            !this->operator()(j, i).is_plus_infinity()) {
          return false;
        }
      }

      return true;
    }

    /// \brief Print the matrix, for debugging purpose
    void dump(std::ostream& o) const {
      for (MatrixIndex i = 0; i < this->_num_vars; i++) {
        for (MatrixIndex j = 0; j < this->_num_vars; j++) {
          o << "M[" << i << ", " << j << "] = " << this->operator()(i, j)
            << "; ";
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

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top abstract value
  explicit DBM(TopTag) : _is_bottom(false), _is_normalized(true) {}

  /// \brief Create the bottom abstract value
  explicit DBM(BottomTag) : _is_bottom(true), _is_normalized(true) {}

public:
  /// \brief Create the top abstract value
  static DBM top() { return DBM(TopTag{}); }

  /// \brief Create the bottom abstract value
  static DBM bottom() { return DBM(BottomTag{}); }

  /// \brief Copy constructor
  DBM(const DBM&) = default;

  /// \brief Move constructor
  DBM(DBM&&) = default;

  /// \brief Copy assignment operator
  DBM& operator=(const DBM&) = default;

  /// \brief Move assignment operator
  DBM& operator=(DBM&&) = default;

  /// \brief Destructor
  ~DBM() override = default;

  void normalize() override {
    if (this->_is_normalized) {
      return;
    }

    if (this->_is_bottom) {
      this->set_to_bottom();
      return;
    }

    // Floyd-Warshall algorithm
    this->_matrix.normalize();

    // Check for negative cycle
    if (this->_matrix.has_negative_cycle()) {
      this->set_to_bottom();
      return;
    }

    this->_is_normalized = true;
  }

  bool is_bottom() const override {
    if (this->_is_normalized) {
      return this->_is_bottom;
    } else if (this->_is_bottom) {
      return true;
    } else {
      // Very inefficient, make sure this is not in a hot path
      Matrix tmp = this->_matrix;
      tmp.normalize();
      return tmp.has_negative_cycle();
    }
  }

  bool is_top() const override {
    return !this->_is_bottom && this->_matrix.all_plus_infinity();
  }

  void set_to_bottom() override {
    this->_is_bottom = true;
    this->_is_normalized = true;
    this->_matrix.clear();
    this->_var_index_map.clear();
  }

  void set_to_top() override {
    this->_is_bottom = false;
    this->_is_normalized = true;
    this->_matrix.clear();
    this->_var_index_map.clear();
  }

private:
  /// \brief Return a normalized copy
  DBM normalize_copy() const {
    // Very inefficient, make sure this is not in a hot path
    DBM tmp = *this;
    tmp.normalize();
    return tmp;
  }

public:
  bool leq(const DBM& other) const override {
    // Requires normalization
    if (!this->_is_normalized) {
      return this->normalize_copy().leq(other);
    }
    if (!other._is_normalized) {
      return this->leq(other.normalize_copy());
    }

    ikos_assert(this->_is_normalized);
    ikos_assert(other._is_normalized);

    if (this->_is_bottom) {
      return true;
    } else if (other._is_bottom) {
      return false;
    }

    std::vector< std::pair< MatrixIndex, MatrixIndex > > vars;
    vars.reserve(this->_var_index_map.size());

    // Iterate over this->_var_index_map and other._var_index_map in parallel
    // This is possible because var_index_map is sorted.
    for (auto l = this->_var_index_map.begin(),
              r = other._var_index_map.begin();
         r != other._var_index_map.end();) {
      if (l == this->_var_index_map.end() || r->first < l->first) {
        // Variable in `other` but not in `this`
        if (!other._matrix.all_plus_infinity(r->second)) {
          return false;
        }
        ++r;
      } else if (l->first < r->first) {
        // Variable in `this` but not in `other`, this is fine.
        ++l;
      } else {
        vars.emplace_back(l->second, r->second);
        ++l;
        ++r;
      }
    }

    // Check if this->_matrix(i, j) <= other._matrix(i, j)
    for (const auto& i : vars) {
      // special variable 0
      if (!(this->_matrix(i.first, 0) <= other._matrix(i.second, 0)) ||
          !(this->_matrix(0, i.first) <= other._matrix(0, i.second))) {
        return false;
      }

      for (const auto& j : vars) {
        if (!(this->_matrix(i.first, j.first) <=
              other._matrix(i.second, j.second))) {
          return false;
        }
      }
    }

    return true;
  }

  bool equals(const DBM& other) const override {
    return this->leq(other) && other.leq(*this);
  }

private:
  /// \brief Apply a pointwise binary operator
  template < typename BinaryOperator >
  DBM pointwise_binary_op(const DBM& other, const BinaryOperator& op) const {
    // Result dbm
    auto dbm = DBM::top();

    // Marker for an invalid index
    const MatrixIndex none = std::numeric_limits< MatrixIndex >::max();

    // Build index map
    MatrixIndex next_index = 1;
    std::vector< std::pair< MatrixIndex, MatrixIndex > > vars;
    vars.reserve(this->_var_index_map.size());

    for (auto l = this->_var_index_map.begin(),
              r = other._var_index_map.begin();
         l != this->_var_index_map.end() || r != other._var_index_map.end();) {
      if (l == this->_var_index_map.end() ||
          (r != other._var_index_map.end() && r->first < l->first)) {
        // Variable in `other` but not in `this`
        if (op.meet_semantic()) {
          dbm._var_index_map.emplace_hint(dbm._var_index_map.end(),
                                          r->first,
                                          next_index++);
          vars.emplace_back(none, r->second);
        }
        ++r;
      } else if (r == other._var_index_map.end() ||
                 (l != this->_var_index_map.end() && l->first < r->first)) {
        // Variable in `this` but not in `other`
        if (op.meet_semantic()) {
          dbm._var_index_map.emplace_hint(dbm._var_index_map.end(),
                                          l->first,
                                          next_index++);
          vars.emplace_back(l->second, none);
        }
        l++;
      } else {
        ikos_assert(l->first == r->first);
        dbm._var_index_map.emplace_hint(dbm._var_index_map.end(),
                                        l->first,
                                        next_index++);
        vars.emplace_back(l->second, r->second);
        l++;
        r++;
      }
    }

    // Allocate memory for the result matrix
    dbm._matrix.clear_resize(static_cast< MatrixIndex >(vars.size() + 1));

    // Compute the result matrix
    for (std::size_t i_index = 0; i_index < vars.size(); ++i_index) {
      const auto& i = vars[i_index];
      const auto i_res = static_cast< MatrixIndex >(i_index + 1);

      if (op.meet_semantic() && i.second == none) {
        dbm._matrix(i_res, 0) = op(this->_matrix(i.first, 0));
        dbm._matrix(0, i_res) = op(this->_matrix(0, i.first));
      } else if (op.meet_semantic() && i.first == none) {
        dbm._matrix(i_res, 0) = op(other._matrix(i.second, 0));
        dbm._matrix(0, i_res) = op(other._matrix(0, i.second));
      } else {
        dbm._matrix(i_res, 0) =
            op(this->_matrix(i.first, 0), other._matrix(i.second, 0));
        dbm._matrix(0, i_res) =
            op(this->_matrix(0, i.first), other._matrix(0, i.second));
      }

      for (std::size_t j_index = 0; j_index < vars.size(); ++j_index) {
        const auto& j = vars[j_index];
        const auto j_res = static_cast< MatrixIndex >(j_index + 1);

        if (i_res == j_res) {
          dbm._matrix(i_res, i_res) = BoundT(0);
        } else if (op.meet_semantic() && (i.first == none || j.first == none) &&
                   (i.second == none || j.second == none)) {
          dbm._matrix(i_res, j_res) = op();
        } else if (op.meet_semantic() &&
                   (i.second == none || j.second == none)) {
          dbm._matrix(i_res, j_res) = op(this->_matrix(i.first, j.first));
        } else if (op.meet_semantic() && (i.first == none || j.first == none)) {
          dbm._matrix(i_res, j_res) = op(other._matrix(i.second, j.second));
        } else {
          dbm._matrix(i_res, j_res) = op(this->_matrix(i.first, j.first),
                                         other._matrix(i.second, j.second));
        }
      }
    }

    dbm._is_normalized = false;
    return dbm;
  }

  struct JoinOperator {
    bool meet_semantic() const { return false; }

    BoundT operator()() const { return BoundT::plus_infinity(); }

    BoundT operator()(const BoundT&) const { return BoundT::plus_infinity(); }

    BoundT operator()(const BoundT& x, const BoundT& y) const {
      return max(x, y);
    }
  };

  struct WideningOperator {
    bool meet_semantic() const { return false; }

    BoundT operator()() const { return BoundT::plus_infinity(); }

    BoundT operator()(const BoundT&) const { return BoundT::plus_infinity(); }

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

    bool meet_semantic() const { return false; }

    BoundT operator()() const { return BoundT::plus_infinity(); }

    BoundT operator()(const BoundT&) const { return BoundT::plus_infinity(); }

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

  struct MeetOperator {
    bool meet_semantic() const { return true; }

    BoundT operator()() const { return BoundT::plus_infinity(); }

    BoundT operator()(const BoundT& x) const { return x; }

    BoundT operator()(const BoundT& x, const BoundT& y) const {
      return min(x, y);
    }
  };

  struct NarrowingOperator {
    bool meet_semantic() const { return true; }

    BoundT operator()() const { return BoundT::plus_infinity(); }

    BoundT operator()(const BoundT& x) const { return x; }

    BoundT operator()(const BoundT& x, const BoundT& y) const {
      if (x.is_plus_infinity()) {
        return y;
      } else {
        return x;
      }
    }
  };

  struct NarrowingThresholdOperator {
    BoundT threshold;

    explicit NarrowingThresholdOperator(const Number& threshold_)
        : threshold(threshold_) {}

    bool meet_semantic() const { return true; }

    BoundT operator()() const { return BoundT::plus_infinity(); }

    BoundT operator()(const BoundT& x) const { return x; }

    BoundT operator()(const BoundT& x, const BoundT& y) const {
      if (x.is_plus_infinity() || x == threshold) {
        return y;
      } else {
        return x;
      }
    }
  };

public:
  void join_with(DBM&& other) override {
    // Requires normalization
    this->normalize();
    other.normalize();

    ikos_assert(this->_is_normalized);
    ikos_assert(other._is_normalized);

    if (this->_is_bottom) {
      this->operator=(std::move(other));
    } else if (other._is_bottom) {
      return;
    } else {
      DBM dbm = this->pointwise_binary_op(other, JoinOperator{});
      dbm._is_normalized = true; // The join is normalized by construction
      this->operator=(std::move(dbm));
    }
  }

  void join_with(const DBM& other) override {
    // Requires normalization
    this->normalize();
    if (!other._is_normalized) {
      this->join_with(other.normalize_copy());
      return;
    }

    ikos_assert(this->_is_normalized);
    ikos_assert(other._is_normalized);

    if (this->_is_bottom) {
      this->operator=(other);
    } else if (other._is_bottom) {
      return;
    } else {
      DBM dbm = this->pointwise_binary_op(other, JoinOperator{});
      dbm._is_normalized = true; // The join is normalized by construction
      this->operator=(std::move(dbm));
    }
  }

  DBM join(const DBM& other) const override {
    // Requires normalization
    if (!this->_is_normalized) {
      return this->normalize_copy().join(other);
    } else if (!other._is_normalized) {
      return this->join(other.normalize_copy());
    }

    ikos_assert(this->_is_normalized);
    ikos_assert(other._is_normalized);

    if (this->_is_bottom) {
      return other;
    } else if (other._is_bottom) {
      return *this;
    } else {
      DBM dbm = this->pointwise_binary_op(other, JoinOperator{});
      dbm._is_normalized = true; // The join is normalized by construction
      return dbm;
    }
  }

  void widen_with(const DBM& other) override {
    this->operator=(this->widening(other));
  }

  DBM widening(const DBM& other) const override {
    // Requires the normalization of the right hand side.
    // The left hand side should not be normalized.
    if (!other._is_normalized) {
      return this->widening(other.normalize_copy());
    }

    ikos_assert(other._is_normalized);

    if (this->_is_bottom) {
      return other;
    } else if (other._is_bottom) {
      return *this;
    } else {
      return this->pointwise_binary_op(other, WideningOperator{});
    }
  }

  DBM widening_threshold(const DBM& other,
                         const Number& threshold) const override {
    // Requires the normalization of the right hand side.
    // The left hand side should not be normalized.
    if (!other._is_normalized) {
      return this->widening_threshold(other.normalize_copy(), threshold);
    }

    ikos_assert(other._is_normalized);

    if (this->_is_bottom) {
      return other;
    } else if (other._is_bottom) {
      return *this;
    } else {
      return this->pointwise_binary_op(other,
                                       WideningThresholdOperator{threshold});
    }
  }

  void widen_threshold_with(const DBM& other,
                            const Number& threshold) override {
    this->operator=(this->widening_threshold(other, threshold));
  }

  DBM meet(const DBM& other) const override {
    // Requires normalization
    if (!this->_is_normalized) {
      return this->normalize_copy().meet(other);
    } else if (!other._is_normalized) {
      return this->meet(other.normalize_copy());
    }

    ikos_assert(this->_is_normalized);
    ikos_assert(other._is_normalized);

    if (this->_is_bottom || other._is_bottom) {
      return bottom();
    } else {
      return this->pointwise_binary_op(other, MeetOperator{});
    }
  }

  void meet_with(const DBM& other) override {
    this->operator=(this->meet(other));
  }

  DBM narrowing(const DBM& other) const override {
    // Requires normalization
    if (!this->_is_normalized) {
      return this->normalize_copy().narrowing(other);
    } else if (!other._is_normalized) {
      return this->narrowing(other.normalize_copy());
    }

    ikos_assert(this->_is_normalized);
    ikos_assert(other._is_normalized);

    if (this->_is_bottom || other._is_bottom) {
      return bottom();
    } else {
      return this->pointwise_binary_op(other, NarrowingOperator{});
    }
  }

  void narrow_with(const DBM& other) override {
    this->operator=(this->narrowing(other));
  }

  DBM narrowing_threshold(const DBM& other,
                          const Number& threshold) const override {
    // Requires normalization
    if (!this->_is_normalized) {
      return this->normalize_copy().narrowing_threshold(other, threshold);
    } else if (!other._is_normalized) {
      return this->narrowing_threshold(other.normalize_copy(), threshold);
    }

    ikos_assert(this->_is_normalized);
    ikos_assert(other._is_normalized);

    if (this->_is_bottom || other._is_bottom) {
      return bottom();
    } else {
      return this->pointwise_binary_op(other,
                                       NarrowingThresholdOperator{threshold});
    }
  }

  void narrow_threshold_with(const DBM& other,
                             const Number& threshold) override {
    this->operator=(this->narrowing_threshold(other, threshold));
  }

private:
  /// \brief Get the index of variable x in _matrix
  ///
  /// Create a new one if not found
  MatrixIndex var_index(VariableRef x) {
    if (this->_matrix.num_vars() == 0) {
      MatrixIndex i = this->_matrix.add_variable();
      this->_var_index_map.emplace(x, i);
      return i;
    }

    auto it = this->_var_index_map.find(x);
    if (it != this->_var_index_map.end()) {
      return it->second;
    }

    // Look for an unused index in _matrix
    std::vector< bool > is_used(this->_matrix.num_vars(), false);

    is_used[0] = true;
    for (const auto& p : this->_var_index_map) {
      is_used[p.second] = true;
    }

    auto unused_index = std::find(is_used.begin(), is_used.end(), false);
    if (unused_index == is_used.end()) {
      // No unused index found, we resize the matrix
      MatrixIndex i = this->_matrix.add_variable();
      this->_var_index_map.emplace(x, i);
      return i;
    } else {
      MatrixIndex i = static_cast< MatrixIndex >(
          std::distance(is_used.begin(), unused_index));
      this->_var_index_map.emplace(x, i);
      return i;
    }
  }

  /// \brief Add constraint v_i - v_j <= c
  void add_constraint(MatrixIndex i, MatrixIndex j, const BoundT& c) {
    const BoundT& w = this->_matrix(j, i);
    if (c < w) {
      this->_matrix(j, i) = c;
      this->_is_normalized = false;
    }
  }

  /// \brief Add constraint v_i - v_j <= c
  void add_constraint(MatrixIndex i, MatrixIndex j, const Number& c) {
    this->add_constraint(i, j, BoundT(c));
  }

  /// \brief Add constraint v_i - v_j <= c
  void add_constraint(MatrixIndex i, MatrixIndex j, int c) {
    this->add_constraint(i, j, BoundT(c));
  }

  /// \brief Apply v_i = v_i + c
  void increment(MatrixIndex i, const BoundT& c) {
    if (c == BoundT(0)) {
      return;
    }

    for (MatrixIndex j = 0; j < this->_matrix.num_vars(); j++) {
      if (i != j) {
        this->_matrix(i, j) -= c;
        this->_matrix(j, i) += c;
      }
    }

    this->_is_normalized = false;
  }

  /// \brief Apply v_i = v_i + c
  void increment(MatrixIndex i, const Number& c) {
    this->increment(i, BoundT(c));
  }

public:
  void assign(VariableRef x, int n) override { this->assign(x, Number(n)); }

  void assign(VariableRef x, const Number& n) override {
    if (this->_is_bottom) {
      return;
    }

    MatrixIndex i = this->var_index(x);
    this->forget(i);
    this->add_constraint(i, 0, n);
    this->add_constraint(0, i, -n);
  }

  void assign(VariableRef x, VariableRef y) override {
    if (this->_is_bottom) {
      return;
    }

    if (x == y) {
      return;
    }

    MatrixIndex i = this->var_index(x);
    MatrixIndex j = this->var_index(y);
    this->forget(i);
    this->add_constraint(i, j, 0);
    this->add_constraint(j, i, 0);
  }

  void assign(VariableRef x, const LinearExpressionT& e) override {
    // Does not require normalization

    if (this->_is_bottom) {
      return;
    }

    if (e.is_constant()) { // x = c
      MatrixIndex i = this->var_index(x);
      this->forget(i);
      this->add_constraint(i, 0, e.constant());
      this->add_constraint(0, i, -e.constant());
      return;
    }

    if (e.num_terms() == 1 && e.begin()->second == 1) { // x = y + c
      MatrixIndex i = this->var_index(x);
      VariableRef y = e.begin()->first;
      const Number& c = e.constant();

      if (x == y) { // x = x + c
        this->increment(i, c);
      } else {
        MatrixIndex j = this->var_index(y);
        this->forget(i);
        this->add_constraint(i, j, c);
        this->add_constraint(j, i, -c);
      }
      return;
    }

    // Projection using intervals, requires normalization
    this->normalize();

    if (this->_is_bottom) {
      return;
    }

    this->set(x, this->to_interval(e));
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             VariableRef z) override {
    // Requires normalization
    this->normalize();

    if (this->_is_bottom) {
      return;
    }

    IntervalT v_y = this->to_interval(y);
    IntervalT v_z = this->to_interval(z);

    if (v_z.singleton()) {
      this->apply(op, x, y, *v_z.singleton());
    } else if (v_y.singleton()) {
      this->apply(op, x, *v_y.singleton(), z);
    } else {
      this->set(x, apply_bin_operator(op, v_y, v_z));
    }
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             const Number& z) override {
    // Does not require normalization

    if (this->_is_bottom) {
      return;
    }

    switch (op) {
      case BinaryOperator::Add: {
        MatrixIndex i = this->var_index(x);
        if (x == y) { // x = x + z
          this->increment(i, z);
        } else { // x = y + z
          MatrixIndex j = this->var_index(y);
          this->forget(i);
          this->add_constraint(i, j, z);
          this->add_constraint(j, i, -z);
        }
      } break;
      case BinaryOperator::Sub: {
        MatrixIndex i = this->var_index(x);
        if (x == y) { // x = x - z
          this->increment(i, -z);
        } else { // x = y - z
          MatrixIndex j = this->var_index(y);
          this->forget(i);
          this->add_constraint(i, j, -z);
          this->add_constraint(j, i, z);
        }
      } break;
      case BinaryOperator::Mul: {
        if (z == 1) { // x = y
          if (x == y) {
            return;
          }
          MatrixIndex i = this->var_index(x);
          MatrixIndex j = this->var_index(y);
          this->forget(i);
          this->add_constraint(i, j, 0);
          this->add_constraint(j, i, 0);
        } else {
          // Requires normalization
          this->normalize();

          if (this->_is_bottom) {
            return;
          }

          this->set(x, this->to_interval(y) * IntervalT(z));
        }
      } break;
      case BinaryOperator::Div: {
        if (z == 1) { // x = y
          if (x == y) {
            return;
          }
          MatrixIndex i = this->var_index(x);
          MatrixIndex j = this->var_index(y);
          this->forget(i);
          this->add_constraint(i, j, 0);
          this->add_constraint(j, i, 0);
        } else {
          // Requires normalization
          this->normalize();

          if (this->_is_bottom) {
            return;
          }

          this->set(x, this->to_interval(y) / IntervalT(z));
        }
      } break;
      case BinaryOperator::Mod: {
        if (z == 0) {
          this->set_to_bottom();
          return;
        }

        // Requires normalization
        this->normalize();

        if (this->_is_bottom) {
          return;
        }

        IntervalT v_y = this->to_interval(y);
        boost::optional< Number > n = v_y.mod_to_sub(z);

        if (n) {
          // Equivalent to x = y - n
          MatrixIndex i = this->var_index(x);
          if (x == y) { // x = x - n
            this->increment(i, -(*n));
          } else { // x = y - n
            MatrixIndex j = this->var_index(y);
            this->forget(i);
            this->add_constraint(i, j, -(*n));
            this->add_constraint(j, i, *n);
          }
        } else {
          this->set(x, IntervalT(BoundT(0), BoundT(abs(z) - 1)));

          // If y < abs(z) then x >= y
          if (v_y.ub() < BoundT(abs(z))) {
            MatrixIndex i = this->var_index(x);
            MatrixIndex j = this->var_index(y);
            this->add_constraint(j, i, BoundT(0));
          }

          // If y >= -abs(z) then x <= y + abs(z)
          if (v_y.lb() >= BoundT(-abs(z))) {
            MatrixIndex i = this->var_index(x);
            MatrixIndex j = this->var_index(y);
            this->add_constraint(i, j, BoundT(abs(z)));
          }
        }
      } break;
      case BinaryOperator::Rem:
      case BinaryOperator::Shl:
      case BinaryOperator::Shr:
      case BinaryOperator::And:
      case BinaryOperator::Or:
      case BinaryOperator::Xor: {
        // Requires normalization
        this->normalize();

        if (this->_is_bottom) {
          return;
        }

        this->set(x,
                  apply_bin_operator(op, this->to_interval(y), IntervalT(z)));
      } break;
    }
  }

  void apply(BinaryOperator op,
             VariableRef x,
             const Number& y,
             VariableRef z) override {
    // Does not require normalization

    if (this->_is_bottom) {
      return;
    }

    switch (op) {
      case BinaryOperator::Add: {
        MatrixIndex i = this->var_index(x);
        if (x == z) { // x = y + x
          this->increment(i, y);
        } else { // x = y + z
          MatrixIndex j = this->var_index(z);
          this->forget(i);
          this->add_constraint(i, j, y);
          this->add_constraint(j, i, -y);
        }
      } break;
      case BinaryOperator::Sub: {
        // Requires normalization
        this->normalize();

        if (this->_is_bottom) {
          return;
        }

        this->set(x, IntervalT(y) - this->to_interval(z));
      } break;
      case BinaryOperator::Mul: {
        if (y == 1) { // x = z
          if (x == z) {
            return;
          }
          MatrixIndex i = this->var_index(x);
          MatrixIndex j = this->var_index(z);
          this->forget(i);
          this->add_constraint(i, j, 0);
          this->add_constraint(j, i, 0);
        } else {
          // Requires normalization
          this->normalize();

          if (this->_is_bottom) {
            return;
          }

          this->set(x, IntervalT(y) * this->to_interval(z));
        }
      } break;
      case BinaryOperator::Div:
      case BinaryOperator::Rem:
      case BinaryOperator::Mod:
      case BinaryOperator::Shl:
      case BinaryOperator::Shr:
      case BinaryOperator::And:
      case BinaryOperator::Or:
      case BinaryOperator::Xor: {
        // Requires normalization
        this->normalize();

        if (this->_is_bottom) {
          return;
        }

        this->set(x,
                  apply_bin_operator(op, IntervalT(y), this->to_interval(z)));
      } break;
    }
  }

  void add(const LinearConstraintT& cst) override {
    // Does not require normalization

    if (this->_is_bottom) {
      return;
    }

    if (cst.num_terms() == 0) {
      if (cst.is_contradiction()) {
        this->set_to_bottom();
      }
      return;
    }

    auto it = cst.begin();
    auto it2 = ++cst.begin();
    MatrixIndex i;
    MatrixIndex j;
    const Number& c = cst.constant();

    if (cst.num_terms() == 1 && it->second == 1) {
      i = this->var_index(it->first);
      j = 0;
    } else if (cst.num_terms() == 1 && it->second == -1) {
      i = 0;
      j = this->var_index(it->first);
    } else if (cst.num_terms() == 2 && it->second == 1 && it2->second == -1) {
      i = this->var_index(it->first);
      j = this->var_index(it2->first);
    } else if (cst.num_terms() == 2 && it->second == -1 && it2->second == 1) {
      i = this->var_index(it2->first);
      j = this->var_index(it->first);
    } else {
      // Use the linear interval solver
      this->normalize();

      if (this->_is_bottom) {
        return;
      }

      LinearIntervalSolverT solver(MaxReductionCycles);
      solver.add(cst);
      solver.run(*this);
      return;
    }

    if (cst.is_inequality()) {
      this->add_constraint(i, j, c);
    } else if (cst.is_equality()) {
      this->add_constraint(i, j, c);
      this->add_constraint(j, i, -c);
    } else {
      // Use the linear interval solver
      this->normalize();

      if (this->_is_bottom) {
        return;
      }

      LinearIntervalSolverT solver(MaxReductionCycles);
      solver.add(cst);
      solver.run(*this);
    }
  }

  void add(const LinearConstraintSystemT& csts) override {
    if (this->_is_bottom) {
      return;
    }

    LinearIntervalSolverT solver(MaxReductionCycles);

    for (const LinearConstraintT& cst : csts) {
      // Process each constraint
      if (cst.num_terms() == 0) {
        if (cst.is_contradiction()) {
          this->set_to_bottom();
          return;
        }
      } else if (cst.is_inequality() || cst.is_equality()) {
        auto it = cst.begin();
        auto it2 = ++cst.begin();
        MatrixIndex i;
        MatrixIndex j;
        const Number& c = cst.constant();

        if (cst.num_terms() == 1 && it->second == 1) {
          i = this->var_index(it->first);
          j = 0;
        } else if (cst.num_terms() == 1 && it->second == -1) {
          i = 0;
          j = this->var_index(it->first);
        } else if (cst.num_terms() == 2 && it->second == 1 &&
                   it2->second == -1) {
          i = this->var_index(it->first);
          j = this->var_index(it2->first);
        } else if (cst.num_terms() == 2 && it->second == -1 &&
                   it2->second == 1) {
          i = this->var_index(it2->first);
          j = this->var_index(it->first);
        } else {
          solver.add(cst);
          continue;
        }

        if (cst.is_inequality()) {
          this->add_constraint(i, j, c);
        } else {
          this->add_constraint(i, j, c);
          this->add_constraint(j, i, -c);
        }
      } else {
        solver.add(cst);
      }
    }

    if (!solver.empty()) {
      // Use the linear interval solver
      this->normalize();

      if (this->_is_bottom) {
        return;
      }

      solver.run(*this);
    }
  }

  void set(VariableRef x, const IntervalT& value) override {
    if (this->_is_bottom) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      MatrixIndex i = this->var_index(x);
      this->forget(i);
      this->add_constraint(i, 0, value.ub());
      this->add_constraint(0, i, -value.lb());
    }
  }

  void set(VariableRef x, const CongruenceT& value) override {
    if (this->_is_bottom) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      MatrixIndex i = this->var_index(x);
      this->forget(i);
      boost::optional< Number > n = value.singleton();
      if (n) {
        this->add_constraint(i, 0, *n);
        this->add_constraint(0, i, -*n);
      }
    }
  }

  void set(VariableRef x, const IntervalCongruenceT& value) override {
    this->set(x, value.interval());
  }

  void refine(VariableRef x, const IntervalT& value) override {
    if (this->_is_bottom) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      MatrixIndex i = this->var_index(x);
      this->add_constraint(i, 0, value.ub());
      this->add_constraint(0, i, -value.lb());
    }
  }

  void refine(VariableRef x, const CongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      IntervalCongruenceT iv(this->to_interval(x), value);
      this->refine(x, iv.interval());
    }
  }

  void refine(VariableRef x, const IntervalCongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      IntervalCongruenceT iv(this->to_interval(x));
      iv.meet_with(value);
      this->refine(x, iv.interval());
    }
  }

private:
  /// \brief Forget all informations about variable k
  void forget(MatrixIndex k) {
    // Use informations about k to improve all constraints
    // Not necessary if already normalized
    if (!this->_is_normalized) {
      for (MatrixIndex i = 0; i < this->_matrix.num_vars(); i++) {
        const BoundT& w_i_k = this->_matrix(i, k);

        if (w_i_k + this->_matrix(k, i) < BoundT(0)) {
          this->_is_bottom = true;
          return;
        }

        for (MatrixIndex j = 0; j < this->_matrix.num_vars(); j++) {
          if (i != k && j != k && i != j) {
            this->_matrix(i, j) =
                min(this->_matrix(i, j), w_i_k + this->_matrix(k, j));
          }
        }
      }
    }

    for (MatrixIndex i = 0; i < this->_matrix.num_vars(); i++) {
      this->_matrix(i, k) = BoundT::plus_infinity();
      this->_matrix(k, i) = BoundT::plus_infinity();
    }
    this->_matrix(k, k) = BoundT(0);

    this->_is_normalized = false;
  }

public:
  void forget(VariableRef x) override {
    if (this->_is_bottom) {
      return;
    }

    auto it = this->_var_index_map.find(x);
    if (it != this->_var_index_map.end()) {
      this->forget(it->second);
      this->_var_index_map.erase(it);
    }
  }

private:
  struct GetVar {
    const VariableRef& operator()(
        const std::pair< VariableRef, MatrixIndex >& p) const {
      return p.first;
    }
  };

public:
  /// \brief Iterator over a list of variables
  using VariableIterator =
      boost::transform_iterator< GetVar, typename VarIndexMap::const_iterator >;

  /// \brief Begin iterator over the list of variables
  VariableIterator var_begin() const {
    return boost::make_transform_iterator(this->_var_index_map.cbegin(),
                                          GetVar());
  }

  /// \brief End iterator over the list of variables
  VariableIterator var_end() const {
    return boost::make_transform_iterator(this->_var_index_map.cend(),
                                          GetVar());
  }

  IntervalT to_interval(VariableRef x) const override {
    if (this->_is_bottom) {
      return IntervalT::bottom();
    } else {
      auto it = this->_var_index_map.find(x);

      if (it == this->_var_index_map.cend()) {
        return IntervalT::top();
      } else {
        return IntervalT(-this->_matrix(it->second, 0),
                         this->_matrix(0, it->second));
      }
    }
  }

  IntervalT to_interval(const LinearExpressionT& e) const override {
    // TODO(marthaud): provide a better result for e = x - y
    return Parent::to_interval(e);
  }

  CongruenceT to_congruence(VariableRef x) const override {
    if (this->_is_bottom) {
      return CongruenceT::bottom();
    } else {
      boost::optional< Number > n = this->to_interval(x).singleton();
      if (n) {
        return CongruenceT(*n);
      } else {
        return CongruenceT::top();
      }
    }
  }

  CongruenceT to_congruence(const LinearExpressionT& e) const override {
    return Parent::to_congruence(e);
  }

  IntervalCongruenceT to_interval_congruence(VariableRef x) const override {
    return IntervalCongruenceT(this->to_interval(x));
  }

  IntervalCongruenceT to_interval_congruence(
      const LinearExpressionT& e) const override {
    // TODO(marthaud): provide a better result for e = x - y
    return Parent::to_interval_congruence(e);
  }

  LinearConstraintSystemT to_linear_constraint_system() const override {
    if (this->_is_bottom) {
      return LinearConstraintSystemT(LinearConstraintT::contradiction());
    }

    LinearConstraintSystemT csts;
    for (auto it = this->_var_index_map.begin(),
              et = this->_var_index_map.end();
         it != et;
         ++it) {
      csts.add(within_interval(it->first,
                               IntervalT(-this->_matrix(it->second, 0),
                                         this->_matrix(0, it->second))));

      for (auto it2 = it + 1; it2 != et; ++it2) {
        csts.add(
            within_interval(VariableExprT(it->first) -
                                VariableExprT(it2->first),
                            IntervalT(-this->_matrix(it->second, it2->second),
                                      this->_matrix(it2->second, it->second))));
      }
    }

    return csts;
  }

  void dump(std::ostream& o) const override {
    this->to_linear_constraint_system().dump(o);
  }

  static std::string name() { return "dbm"; }

}; // end class DBM

} // end namespace numeric
} // end namespace core
} // end namespace ikos
