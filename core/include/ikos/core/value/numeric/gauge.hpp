/*******************************************************************************
 *
 * \file
 * \brief GaugeBound and Gauge classes
 *
 * Based on Arnaud Venet's paper: The Gauge Domain: Scalable Analysis of
 * Linear Inequality Invariants, in CAV, 129-154, 2012.
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017-2019 United States Government as represented by the
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

#include <type_traits>

#include <boost/optional.hpp>

#include <ikos/core/adt/patricia_tree/map.hpp>
#include <ikos/core/linear_expression.hpp>
#include <ikos/core/number/bound.hpp>
#include <ikos/core/support/assert.hpp>
#include <ikos/core/value/numeric/constant.hpp>
#include <ikos/core/value/numeric/interval.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief Gauge bound
///
/// This is either -oo, +oo or a linear expression of the non-negative loop
/// counters (e.g, `1 + 2*i + 3*j`)
template < typename Number, typename VariableRef >
class GaugeBound {
public:
  using BoundT = Bound< Number >;
  using VariableExpressionT = VariableExpression< Number, VariableRef >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;

private:
  using Coefficients = PatriciaTreeMap< VariableRef, Number >;

private:
  bool _is_infinite;
  Number _cst;
  Coefficients _coeffs;

  /* Invariants:
   * _is_infinite => _cst in {-1, 1} and _coeffs.empty()
   */

private:
  /// \brief Private constructor
  GaugeBound(bool is_infinite, Number cst, Coefficients coeffs)
      : _is_infinite(is_infinite),
        _cst(std::move(cst)),
        _coeffs(std::move(coeffs)) {
    if (this->_is_infinite) {
      this->_coeffs.clear();
      this->_cst = (this->_cst >= 0) ? 1 : -1;
    }
  }

  /// \brief Private constructor
  GaugeBound(bool is_infinite, int cst, Coefficients coeffs)
      : _is_infinite(is_infinite), _cst(cst), _coeffs(std::move(coeffs)) {
    if (this->_is_infinite) {
      this->_coeffs.clear();
      this->_cst = (cst >= 0) ? 1 : -1;
    }
  }

public:
  /// \brief Return +oo
  static GaugeBound plus_infinity() {
    return GaugeBound(true, 1, Coefficients());
  }

  /// \brief Return -oo
  static GaugeBound minus_infinity() {
    return GaugeBound(true, -1, Coefficients());
  }

  /// \brief Create the gauge bound `n`
  explicit GaugeBound(int n) : _is_infinite(false), _cst(n) {}

  /// \brief Create the gauge bound `n`
  explicit GaugeBound(Number n) : _is_infinite(false), _cst(std::move(n)) {}

  /// \brief Create the gauge bound `1*v`
  explicit GaugeBound(VariableRef v) : _is_infinite(false), _cst(0) {
    this->_coeffs.insert_or_assign(v, Number(1));
  }

  /// \brief Create a gauge bound from bound `b`
  explicit GaugeBound(const BoundT& b) : _is_infinite(b.is_infinite()) {
    if (b.is_plus_infinity()) {
      this->_cst = 1;
    } else if (b.is_minus_infinity()) {
      this->_cst = -1;
    } else {
      this->_cst = *b.number();
    }
  }

  /// \brief Create the gauge bound `n*v`
  GaugeBound(int n, VariableRef v) : _is_infinite(false), _cst(0) {
    this->_coeffs.insert_or_assign(v, Number(n));
  }

  /// \brief Create the gauge bound `n*v`
  GaugeBound(const Number& n, VariableRef v) : _is_infinite(false), _cst(0) {
    this->_coeffs.insert_or_assign(v, n);
  }

  /// \brief Copy constructor
  GaugeBound(const GaugeBound&) noexcept(
      std::is_nothrow_copy_constructible< Number >::value) = default;

  /// \brief Move constructor
  GaugeBound(GaugeBound&&) noexcept(
      std::is_nothrow_move_constructible< Number >::value) = default;

  /// \brief Assign a number
  GaugeBound& operator=(int n) {
    this->_is_infinite = false;
    this->_cst = n;
    this->_coeffs.clear();
    return *this;
  }

  /// \brief Assign a number
  GaugeBound& operator=(Number n) {
    this->_is_infinite = false;
    this->_n = std::move(n);
    this->_coeffs.clear();
    return *this;
  }

  /// \brief Copy assignment operator
  GaugeBound& operator=(const GaugeBound&) noexcept(
      std::is_nothrow_copy_assignable< Number >::value) = default;

  /// \brief Move assignment operator
  GaugeBound& operator=(GaugeBound&&) noexcept(
      std::is_nothrow_move_assignable< Number >::value) = default;

  /// \brief Destructor
  ~GaugeBound() = default;

  /// \brief Return true if the bound is infinite
  bool is_infinite() const { return this->_is_infinite; }

  /// \brief Return true if the bound is finite
  bool is_finite() const { return !this->_is_infinite; }

  /// \brief Return true if the bound is plus infinity
  bool is_plus_infinity() const {
    return this->_is_infinite && this->_cst == 1;
  }

  /// \brief Return true if the bound is minus infinity
  bool is_minus_infinity() const {
    return this->_is_infinite && this->_cst == -1;
  }

  /// \brief Return true if the bound is a constant
  bool is_constant() const {
    return !this->_is_infinite && this->_coeffs.empty();
  }

  /// \brief Return the constant
  const Number& constant() const {
    ikos_assert(this->is_finite());
    return this->_cst;
  }

  /// \brief Return the number of coefficients
  std::size_t num_coeffs() const {
    ikos_assert(this->is_finite());
    return this->_coeffs.size();
  }

  /// \brief Return the coefficient for the variable `v`
  Number coeff(VariableRef v) const {
    ikos_assert(this->is_finite());
    boost::optional< const Number& > c = this->_coeffs.at(v);
    if (c) {
      return *c;
    } else {
      return Number(0);
    }
  }

  /// \brief Return the sum of the coefficients
  Number sum_coeffs() const {
    ikos_assert(this->is_finite());
    Number r(0);
    for (auto it = this->_coeffs.begin(), et = this->_coeffs.end(); it != et;
         ++it) {
      r += it->second;
    }
    return r;
  }

  /// \brief Return the gauge bound as a numeric bound or boost::none
  boost::optional< BoundT > to_bound() const {
    if (this->_is_infinite) {
      if (this->_cst == 1) {
        return BoundT::plus_infinity();
      } else {
        return BoundT::minus_infinity();
      }
    } else if (this->_coeffs.empty()) {
      return BoundT(this->_cst);
    } else {
      return boost::none;
    }
  }

private:
  /// \brief Apply a binary operator on coefficients
  template < typename BinaryOp >
  static typename BinaryOp::ResultType apply_binary_operation(
      const Coefficients& c1, const Coefficients& c2, const BinaryOp& op) {
    return c1.binary_operation(c2, op);
  }

  /// \brief Apply an unary operator on coefficients
  template < typename UnaryOp >
  static Coefficients apply_unary_operation(Coefficients c, const UnaryOp& op) {
    c.transform(op);
    return c;
  }

  /// \brief Join separated coefficient trees
  static Coefficients join_separated(const Coefficients& c1,
                                     const Coefficients& c2) {
    return c1.join(c2, [](const Number&, const Number&) {
      ikos_unreachable("trees are not separated");
      return boost::optional< Number >(boost::none);
    });
  }

private:
  /// \brief Add binary operator
  struct NumberAdd {
    boost::optional< Number > operator()(const Number& x,
                                         const Number& y) const {
      return x + y;
    }
  };

  /// \brief Negate binary operator
  struct GaugeBoundNeg {
    boost::optional< Number > operator()(VariableRef, const Number& x) const {
      return -x;
    }
  };

public:
  /// \brief Unary minus
  GaugeBound operator-() const {
    return GaugeBound(this->_is_infinite,
                      -this->_cst,
                      apply_unary_operation(this->_coeffs, GaugeBoundNeg{}));
  }

private:
  /// \brief Add binary operator
  struct GaugeBoundAdd {
    using ResultType = Coefficients;

    bool has_equals() const { return false; }

    Coefficients equals(const Coefficients&) const {
      ikos_unreachable("unreachable");
      return Coefficients();
    }

    Coefficients left(const Coefficients& c) const { return c; }

    Coefficients right(const Coefficients& c) const { return c; }

    Coefficients left_with_right_leaf(Coefficients c,
                                      VariableRef v,
                                      const Number& n) const {
      c.update_or_insert(NumberAdd{}, v, n);
      return c;
    }

    Coefficients right_with_left_leaf(Coefficients c,
                                      VariableRef v,
                                      const Number& n) const {
      c.update_or_insert(NumberAdd{}, v, n);
      return c;
    }

    Coefficients merge(const Coefficients& l, const Coefficients& r) const {
      return join_separated(l, r);
    }
  };

public:
  /// \brief Add a number
  GaugeBound operator+(const Number& n) const {
    if (this->is_finite()) {
      return GaugeBound(false, this->_cst + n, this->_coeffs);
    } else {
      return *this;
    }
  }

  /// \brief Add gauge bounds
  GaugeBound operator+(const GaugeBound& other) const {
    if (this->is_finite() && other.is_finite()) {
      return GaugeBound(false,
                        this->_cst + other._cst,
                        apply_binary_operation(this->_coeffs,
                                               other._coeffs,
                                               GaugeBoundAdd{}));
    } else if (this->is_finite() && other.is_infinite()) {
      return other;
    } else if (this->is_infinite() && other.is_finite()) {
      return *this;
    } else if (this->_cst == other._cst) {
      return *this;
    } else {
      ikos_unreachable("undefined operation -oo + +oo");
    }
  }

  /// \brief Add gauge bounds
  GaugeBound& operator+=(const GaugeBound& other) {
    return this->operator=(this->operator+(other));
  }

private:
  /// \brief Sub binary operator
  struct GaugeBoundSub {
    using ResultType = Coefficients;

    bool has_equals() const { return true; }

    Coefficients equals(const Coefficients&) const {
      return Coefficients(); // x - x
    }

    Coefficients left(const Coefficients& c) const { return c; }

    Coefficients right(const Coefficients& c) const {
      return apply_unary_operation(c, GaugeBoundNeg{});
    }

    Coefficients left_with_right_leaf(Coefficients c,
                                      VariableRef v,
                                      const Number& n) const {
      c.update_or_insert(NumberAdd{}, v, -n);
      return c;
    }

    Coefficients right_with_left_leaf(Coefficients c,
                                      VariableRef v,
                                      const Number& n) const {
      c.transform(GaugeBoundNeg{});
      c.update_or_insert(NumberAdd{}, v, n);
      return c;
    }

    Coefficients merge(const Coefficients& l, const Coefficients& r) const {
      return join_separated(l, r);
    }
  };

public:
  /// \brief Substract a number
  GaugeBound operator-(const Number& n) const {
    if (this->is_finite()) {
      return GaugeBound(false, this->_cst - n, this->_coeffs);
    } else {
      return *this;
    }
  }

  /// \brief Substract bounds
  GaugeBound operator-(const GaugeBound& other) const {
    if (this->is_finite() && other.is_finite()) {
      return GaugeBound(false,
                        this->_cst - other._cst,
                        apply_binary_operation(this->_coeffs,
                                               other._coeffs,
                                               GaugeBoundSub{}));
    } else if (this->is_finite() && other.is_infinite()) {
      return other.operator-();
    } else if (this->is_infinite() && other.is_finite()) {
      return *this;
    } else if (this->_cst == -other._cst) {
      return *this;
    } else {
      ikos_unreachable("undefined operation +oo - +oo");
    }
  }

  /// \brief Substract bounds
  GaugeBound& operator-=(const GaugeBound& other) {
    return this->operator=(this->operator-(other));
  }

private:
  /// \brief Multiplication by a constant
  struct GaugeBoundMul {
    Number c;

    boost::optional< Number > operator()(VariableRef, const Number& x) const {
      return x * c;
    }
  };

public:
  /// \brief Multiply by a number
  GaugeBound operator*(const Number& c) const {
    if (c == 0) {
      return GaugeBound(0);
    } else if (this->is_finite()) {
      return GaugeBound(false,
                        this->_cst * c,
                        apply_unary_operation(this->_coeffs, GaugeBoundMul{c}));
    } else {
      return GaugeBound(true, this->_cst * c, Coefficients());
    }
  }

  /// \brief Multiply by a constant
  GaugeBound& operator*=(const Number& c) {
    return this->operator=(this->operator*(c));
  }

private:
  /// \brief Compare bounds
  struct GaugeBoundLeq {
    using ResultType = bool;

    bool has_equals() const { return true; }

    bool equals(const Coefficients&) const {
      return true; // x <= x
    }

    bool left(const Coefficients& c) const {
      return std::all_of(c.begin(), c.end(), [](const auto& binding) {
        return binding.second <= 0;
      });
    }

    bool right(const Coefficients& c) const {
      return std::all_of(c.begin(), c.end(), [](const auto& binding) {
        return 0 <= binding.second;
      });
    }

    bool left_with_right_leaf(const Coefficients& c,
                              VariableRef v,
                              const Number& n) const {
      if (!c.at(v)) {
        if (!(0 <= n)) {
          return false;
        }
      }
      return std::all_of(c.begin(), c.end(), [v, n](const auto& binding) {
        if (binding.first == v) {
          return binding.second <= n;
        } else {
          return binding.second <= 0;
        }
      });
    }

    bool right_with_left_leaf(const Coefficients& c,
                              VariableRef v,
                              const Number& n) const {
      if (!c.at(v)) {
        if (!(n <= 0)) {
          return false;
        }
      }
      return std::all_of(c.begin(), c.end(), [v, n](const auto& binding) {
        if (binding.first == v) {
          return n <= binding.second;
        } else {
          return 0 <= binding.second;
        }
      });
    }

    bool merge(bool l, bool r) const { return l && r; }
  };

  /// \brief Compare bounds
  struct GaugeBoundGeq {
    using ResultType = bool;

    bool has_equals() const { return true; }

    bool equals(const Coefficients&) const {
      return true; // x >= x
    }

    bool left(const Coefficients& c) const {
      return std::all_of(c.begin(), c.end(), [](const auto& binding) {
        return binding.second >= 0;
      });
    }

    bool right(const Coefficients& c) const {
      return std::all_of(c.begin(), c.end(), [](const auto& binding) {
        return 0 >= binding.second;
      });
    }

    bool left_with_right_leaf(const Coefficients& c,
                              VariableRef v,
                              const Number& n) const {
      if (!c.at(v)) {
        if (!(0 >= n)) {
          return false;
        }
      }
      return std::all_of(c.begin(), c.end(), [v, n](const auto& binding) {
        if (binding.first == v) {
          return binding.second >= n;
        } else {
          return binding.second >= 0;
        }
      });
    }

    bool right_with_left_leaf(const Coefficients& c,
                              VariableRef v,
                              const Number& n) const {
      if (!c.at(v)) {
        if (!(n >= 0)) {
          return false;
        }
      }
      return std::all_of(c.begin(), c.end(), [v, n](const auto& binding) {
        if (binding.first == v) {
          return n >= binding.second;
        } else {
          return 0 >= binding.second;
        }
      });
    }

    bool merge(bool l, bool r) const { return l && r; }
  };

  /// \brief Compare bounds
  struct GaugeBoundEq {
    using ResultType = bool;

    bool has_equals() const { return true; }

    bool equals(const Coefficients&) const {
      return true; // x == x
    }

    bool left(const Coefficients& c) const {
      return std::all_of(c.begin(), c.end(), [](const auto& binding) {
        return binding.second == 0;
      });
    }

    bool right(const Coefficients& c) const {
      return std::all_of(c.begin(), c.end(), [](const auto& binding) {
        return 0 == binding.second;
      });
    }

    bool left_with_right_leaf(const Coefficients& c,
                              VariableRef v,
                              const Number& n) const {
      if (!c.at(v)) {
        if (n != 0) {
          return false;
        }
      }
      return std::all_of(c.begin(), c.end(), [v, n](const auto& binding) {
        if (binding.first == v) {
          return binding.second == n;
        } else {
          return binding.second == 0;
        }
      });
    }

    bool right_with_left_leaf(const Coefficients& c,
                              VariableRef v,
                              const Number& n) const {
      if (!c.at(v)) {
        if (n != 0) {
          return false;
        }
      }
      return std::all_of(c.begin(), c.end(), [v, n](const auto& binding) {
        if (binding.first == v) {
          return n == binding.second;
        } else {
          return 0 == binding.second;
        }
      });
    }

    bool merge(bool l, bool r) const { return l && r; }
  };

public:
  /// \brief Lower or equal comparison
  bool operator<=(const GaugeBound& other) const {
    if (this->_is_infinite && other._is_infinite) {
      return this->_cst <= other._cst;
    } else if (this->_is_infinite) {
      return this->_cst < 0;
    } else if (other._is_infinite) {
      return other._cst > 0;
    } else {
      return this->_cst <= other._cst &&
             apply_binary_operation(this->_coeffs,
                                    other._coeffs,
                                    GaugeBoundLeq{});
    }
  }

  /// \brief Greater or equal comparison
  bool operator>=(const GaugeBound& other) const {
    if (this->_is_infinite && other._is_infinite) {
      return this->_cst >= other._cst;
    } else if (this->_is_infinite) {
      return this->_cst > 0;
    } else if (other._is_infinite) {
      return other._cst < 0;
    } else {
      return this->_cst >= other._cst &&
             apply_binary_operation(this->_coeffs,
                                    other._coeffs,
                                    GaugeBoundGeq{});
    }
  }

  /// \brief Lower comparison
  bool operator<(const GaugeBound& other) const {
    return !this->operator>=(other);
  }

  /// \brief Greater comparison
  bool operator>(const GaugeBound& other) const {
    return !this->operator<=(other);
  }

  /// \brief Equal comparison
  bool operator==(const GaugeBound& other) const {
    return this->_is_infinite == other._is_infinite &&
           this->_cst == other._cst &&
           apply_binary_operation(this->_coeffs, other._coeffs, GaugeBoundEq{});
  }

  /// \brief Not equal comparison
  bool operator!=(const GaugeBound& other) const {
    return !this->operator==(other);
  }

private:
  /// \brief Min with 0
  struct MinZero {
    boost::optional< Number > operator()(VariableRef, const Number& x) const {
      return min(x, Number(0));
    }
  };

  /// \brief Min with 0 if it is not `u`
  struct MinZeroIfNot {
    VariableRef u;

    boost::optional< Number > operator()(VariableRef v, const Number& x) const {
      if (u == v) {
        return x;
      }
      return min(x, Number(0));
    }
  };

  /// \brief Min binary operator
  struct GaugeBoundMin {
    using ResultType = Coefficients;

    bool has_equals() const { return true; }

    Coefficients equals(const Coefficients& c) const {
      return c; // min(x, x)
    }

    Coefficients left(const Coefficients& c) const {
      return apply_unary_operation(c, MinZero{});
    }

    Coefficients right(const Coefficients& c) const {
      return apply_unary_operation(c, MinZero{});
    }

    Coefficients tree_with_leaf(Coefficients c,
                                VariableRef v,
                                const Number& x) const {
      auto y = c.at(v);
      if (y) {
        c.insert_or_assign(v, min(x, *y));
      } else {
        c.insert_or_assign(v, min(x, Number(0)));
      }
      return apply_unary_operation(c, MinZeroIfNot{v});
    }

    Coefficients left_with_right_leaf(Coefficients c,
                                      VariableRef v,
                                      const Number& x) const {
      return tree_with_leaf(c, v, x);
    }

    Coefficients right_with_left_leaf(Coefficients c,
                                      VariableRef v,
                                      const Number& x) const {
      return tree_with_leaf(c, v, x);
    }

    Coefficients merge(const Coefficients& l, const Coefficients& r) const {
      return join_separated(l, r);
    }
  };

public:
  /// \brief Return the min of the given bounds, as defined in the paper
  friend GaugeBound min(const GaugeBound& x, const GaugeBound& y) {
    if (x.is_infinite() || y.is_infinite()) {
      return (x <= y) ? x : y;
    } else {
      return GaugeBound(false,
                        min(x._cst, y._cst),
                        apply_binary_operation(x._coeffs,
                                               y._coeffs,
                                               GaugeBoundMin{}));
    }
  }

  /// \brief Return the min of the given bounds, as defined in the paper
  friend GaugeBound min(const GaugeBound& x,
                        const GaugeBound& y,
                        const GaugeBound& z) {
    return min(x, min(y, z));
  }

  /// \brief Return the min of the given bounds, as defined in the paper
  friend GaugeBound min(const GaugeBound& x,
                        const GaugeBound& y,
                        const GaugeBound& z,
                        const GaugeBound& t) {
    return min(min(x, y), min(z, t));
  }

private:
  /// \brief Max with 0
  struct MaxZero {
    boost::optional< Number > operator()(VariableRef, const Number& x) const {
      return max(x, Number(0));
    }
  };

  /// \brief Max with 0 if it is not `u`
  struct MaxZeroIfNot {
    VariableRef u;

    boost::optional< Number > operator()(VariableRef v, const Number& x) const {
      if (u == v) {
        return x;
      }
      return max(x, Number(0));
    }
  };

  /// \brief Max binary operator
  struct GaugeBoundMax {
    using ResultType = Coefficients;

    bool has_equals() const { return true; }

    Coefficients equals(const Coefficients& c) const { return c; }

    Coefficients left(const Coefficients& c) const {
      return apply_unary_operation(c, MaxZero{});
    }

    Coefficients right(const Coefficients& c) const {
      return apply_unary_operation(c, MaxZero{});
    }

    Coefficients tree_with_leaf(Coefficients c,
                                VariableRef v,
                                const Number& x) const {
      auto y = c.at(v);
      if (y) {
        c.insert_or_assign(v, max(x, *y));
      } else {
        c.insert_or_assign(v, max(x, Number(0)));
      }
      return apply_unary_operation(c, MaxZeroIfNot{v});
    }

    Coefficients left_with_right_leaf(Coefficients c,
                                      VariableRef v,
                                      const Number& x) const {
      return tree_with_leaf(c, v, x);
    }

    Coefficients right_with_left_leaf(Coefficients c,
                                      VariableRef v,
                                      const Number& x) const {
      return tree_with_leaf(c, v, x);
    }

    Coefficients merge(const Coefficients& l, const Coefficients& r) const {
      return join_separated(l, r);
    }
  };

public:
  /// \brief Return the max of the given bounds, as defined in the paper
  friend GaugeBound max(const GaugeBound& x, const GaugeBound& y) {
    if (x.is_infinite() || y.is_infinite()) {
      return (x <= y) ? y : x;
    } else {
      return GaugeBound(false,
                        max(x._cst, y._cst),
                        apply_binary_operation(x._coeffs,
                                               y._coeffs,
                                               GaugeBoundMax{}));
    }
  }

  /// \brief Return the max of the given bounds, as defined in the paper
  friend GaugeBound max(const GaugeBound& x,
                        const GaugeBound& y,
                        const GaugeBound& z) {
    return max(x, max(y, z));
  }

  /// \brief Return the max of the given bounds, as defined in the paper
  friend GaugeBound max(const GaugeBound& x,
                        const GaugeBound& y,
                        const GaugeBound& z,
                        const GaugeBound& t) {
    return max(max(x, y), max(z, t));
  }

  /// \brief Return the gauge bound as a linear expression
  LinearExpressionT lin_expr() const {
    ikos_assert(this->is_finite());
    LinearExpressionT e(this->_cst);
    for (auto it = this->_coeffs.begin(), et = this->_coeffs.end(); it != et;
         ++it) {
      e.add(it->second, it->first);
    }
    return e;
  }

  /// \brief Write the gauge bound on a stream
  void dump(std::ostream& o) const {
    if (this->is_plus_infinity()) {
      o << "+oo";
    } else if (this->is_minus_infinity()) {
      o << "-oo";
    } else {
      bool first = true;
      for (auto it = this->_coeffs.begin(), et = this->_coeffs.end(); it != et;
           ++it) {
        const Number& c = it->second;
        const VariableRef& v = it->first;
        if (c == 0) {
          continue;
        } else if (c > 0 && !first) {
          o << "+";
        }
        if (c == -1) {
          o << "-";
        } else if (c != 1) {
          o << c;
        }
        DumpableTraits< VariableRef >::dump(o, v);
        first = false;
      }
      if (this->_cst > 0 && !first) {
        o << "+";
      }
      if (this->_cst != 0 || first) {
        o << this->_cst;
      }
    }
  }

}; // end class GaugeBound

/// \brief Write a bound on a stream
template < typename Number, typename VariableRef >
inline std::ostream& operator<<(
    std::ostream& o, const GaugeBound< Number, VariableRef >& bound) {
  bound.dump(o);
  return o;
}

/// \brief Gauge abstract value
///
/// This is implemented as a pair of gauge bounds
template < typename Number, typename VariableRef >
class Gauge final
    : public core::AbstractDomain< Gauge< Number, VariableRef > > {
public:
  using BoundT = Bound< Number >;
  using GaugeBoundT = GaugeBound< Number, VariableRef >;
  using ConstantT = Constant< Number >;
  using IntervalT = Interval< Number >;

private:
  // Lower bound
  GaugeBoundT _lb;

  // Upper bound
  GaugeBoundT _ub;

  // Invariant: _lb > _ub <=> _lb = 1 && _ub = 0 <=> is_bottom()

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top gauge [-oo, +oo]
  explicit Gauge(TopTag)
      : _lb(GaugeBoundT::minus_infinity()), _ub(GaugeBoundT::plus_infinity()) {}

  /// \brief Create the bottom gauge
  explicit Gauge(BottomTag) : _lb(1), _ub(0) {}

public:
  /// \brief Create the gauge [-oo, +oo]
  static Gauge top() { return Gauge(TopTag{}); }

  /// \brief Create the bottom gauge
  static Gauge bottom() { return Gauge(BottomTag{}); }

  /// \brief Create the gauge [n, n]
  explicit Gauge(int n) : _lb(n), _ub(n) {}

  /// \brief Create the gauge [n, n]
  explicit Gauge(const Number& n) : _lb(n), _ub(n) {}

  /// \brief Create the gauge [b, b]
  explicit Gauge(const GaugeBoundT& b) : _lb(b), _ub(b) {
    ikos_assert(!b.is_infinite());
  }

  /// \brief Create the gauge [lb, ub]
  ///
  /// If lb > ub, the gauge is bottom.
  Gauge(GaugeBoundT lb, GaugeBoundT ub)
      : _lb(std::move(lb)), _ub(std::move(ub)) {
    if (this->_lb > this->_ub) {
      this->_lb = 1;
      this->_ub = 0;
    }
  }

  /// \brief Create a gauge from an interval
  explicit Gauge(const IntervalT& i) : _lb(1), _ub(0) {
    if (!i.is_bottom()) {
      this->_lb = GaugeBoundT(i.lb());
      this->_ub = GaugeBoundT(i.ub());
    }
  }

  /// \brief Copy constructor
  Gauge(const Gauge&) noexcept(
      std::is_nothrow_copy_constructible< Number >::value) = default;

  /// \brief Move constructor
  Gauge(Gauge&&) noexcept(std::is_nothrow_move_constructible< Number >::value) =
      default;

  /// \brief Copy assignment operator
  Gauge& operator=(const Gauge&) noexcept(
      std::is_nothrow_copy_assignable< Number >::value) = default;

  /// \brief Move assignment operator
  Gauge& operator=(Gauge&&) noexcept(
      std::is_nothrow_move_assignable< Number >::value) = default;

  /// \brief Destructor
  ~Gauge() override = default;

  /// \brief Return the lower bound
  const GaugeBoundT& lb() const {
    ikos_assert(!this->is_bottom());
    return this->_lb;
  }

  /// \brief Return the upper bound
  const GaugeBoundT& ub() const {
    ikos_assert(!this->is_bottom());
    return this->_ub;
  }

  void normalize() override {}

  bool is_bottom() const override { return this->_lb > this->_ub; }

  bool is_top() const override {
    return this->_lb.is_infinite() && this->_ub.is_infinite();
  }

  void set_to_bottom() override {
    this->_lb = 1;
    this->_ub = 0;
  }

  void set_to_top() override {
    this->_lb = GaugeBoundT::minus_infinity();
    this->_ub = GaugeBoundT::plus_infinity();
  }

  bool leq(const Gauge& other) const override {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      return other._lb <= this->_lb && this->_ub <= other._ub;
    }
  }

  bool equals(const Gauge& other) const override {
    if (is_bottom()) {
      return other.is_bottom();
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_lb == other._lb && this->_ub == other._ub;
    }
  }

  Gauge join(const Gauge& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return Gauge(min(this->_lb, other._lb), max(this->_ub, other._ub));
    }
  }

  void join_with(const Gauge& other) override {
    this->operator=(this->join(other));
  }

  /// \brief Meet
  ///
  /// Implementation based on X. Wu, L. Chen and J. Wang's paper: An Abstract
  /// Domain to Infer Symbolic Ranges over Nonnegative Parameters.
  ///
  /// It uses a heuristic because, in general, the greatest lower bound
  /// cannot be defined.
  Gauge meet(const Gauge& other) const override {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else {
      // lower bound
      GaugeBoundT lb = GaugeBoundT::minus_infinity();
      if (other._lb <= this->_lb) {
        lb = this->_lb;
      } else if (this->_lb <= other._lb) {
        lb = other._lb;
      } else {
        // no order, use a heuristic
        lb = (this->_lb.constant() + this->_lb.sum_coeffs() >=
              other._lb.constant() + other._lb.sum_coeffs())
                 ? this->_lb
                 : other._lb;
      }

      // upper bound
      GaugeBoundT ub = GaugeBoundT::plus_infinity();
      if (this->_ub <= other._ub) {
        ub = this->_ub;
      } else if (other._ub <= this->_ub) {
        ub = other._ub;
      } else {
        // no order, use a heuristic
        ub = (this->_ub.constant() + this->_ub.sum_coeffs() <=
              other._ub.constant() + other._ub.sum_coeffs())
                 ? this->_ub
                 : other._ub;
      }

      if (lb.is_infinite() || ub.is_infinite() ||
          (lb.is_constant() && ub.is_constant())) {
        // in that case, it is safe to use [lb, ub]
        return Gauge(lb, ub);
      } else {
        // use max(lb, ub) so that the gauge is not empty
        return Gauge(lb, max(lb, ub));
      }
    }
  }

  void meet_with(const Gauge& other) override {
    this->operator=(this->meet(other));
  }

  /// \brief Unary minus
  Gauge operator-() const {
    if (this->is_bottom()) {
      return bottom();
    } else {
      return Gauge(-this->_ub, -this->_lb);
    }
  }

  /// \brief Add gauges
  Gauge operator+(const Gauge& other) const {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else {
      return Gauge(this->_lb + other._lb, this->_ub + other._ub);
    }
  }

  /// \brief Add gauges
  Gauge& operator+=(const Gauge& other) {
    return this->operator=(this->operator+(other));
  }

  /// \brief Substract gauges
  Gauge operator-(const Gauge& other) const {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else {
      return Gauge(this->_lb - other._ub, this->_ub - other._lb);
    }
  }

  /// \brief Substract gauges
  Gauge& operator-=(const Gauge& other) {
    return this->operator=(this->operator-(other));
  }

  /// \brief Multiply by a constant
  Gauge operator*(const Number& c) const {
    if (this->is_bottom()) {
      return bottom();
    } else if (c >= 0) {
      return Gauge(this->_lb * c, this->_ub * c);
    } else {
      return Gauge(this->_ub * c, this->_lb * c);
    }
  }

  /// \brief Multiply by an interval
  Gauge operator*(const IntervalT& i) const {
    if (this->is_bottom() || i.is_bottom()) {
      return bottom();
    } else if (i.is_top()) {
      return top();
    } else if (i.lb().is_infinite()) {
      // i = [-oo, u]
      Number u = *i.ub().number();
      if (this->_lb >= GaugeBoundT(0)) {
        return Gauge(GaugeBoundT::minus_infinity(),
                     max(this->_lb * u, this->_ub * u));
      } else if (this->_ub <= GaugeBoundT(0)) {
        return Gauge(min(this->_lb * u, this->_ub * u),
                     GaugeBoundT::plus_infinity());
      } else {
        return top();
      }
    } else if (i.ub().is_infinite()) {
      // i = [l, +oo]
      Number l = *i.lb().number();
      if (this->_lb >= GaugeBoundT(0)) {
        return Gauge(min(this->_lb * l, this->_ub * l),
                     GaugeBoundT::plus_infinity());
      } else if (this->_ub <= GaugeBoundT(0)) {
        return Gauge(GaugeBoundT::minus_infinity(),
                     max(this->_lb * l, this->_ub * l));
      } else {
        return top();
      }
    } else {
      Number l = *i.lb().number();
      Number u = *i.ub().number();
      GaugeBoundT ll = this->_lb * l;
      GaugeBoundT lu = this->_lb * u;
      GaugeBoundT ul = this->_ub * l;
      GaugeBoundT uu = this->_ub * u;
      return Gauge(min(ll, lu, ul, uu), max(ll, lu, ul, uu));
    }
  }

  /// \brief Forget a counter variable
  Gauge forget(VariableRef v) const {
    if (!this->is_bottom() &&
        ((this->_lb.is_finite() && this->_lb.coeff(v) != 0) ||
         (this->_ub.is_finite() && this->_ub.coeff(v) != 0))) {
      return top();
    } else {
      return *this;
    }
  }

  /// \brief Coalesce operation, as described in the paper
  ///
  /// Note that the definition of coalesce() in Arnaud's paper is wrong because
  /// coefficients can be negative.
  Gauge coalesce(VariableRef v, const Number& l, const BoundT& u) const {
    if (this->is_bottom()) {
      return bottom();
    } else {
      GaugeBoundT lb = this->_lb;
      if (this->_lb.is_finite()) {
        Number lb_v = this->_lb.coeff(v);
        if (lb_v > 0) {
          lb = this->_lb + lb_v * l + GaugeBoundT(-lb_v, v);
        } else if (lb_v < 0) {
          if (u.is_finite()) {
            lb = this->_lb + lb_v * (*u.number()) + GaugeBoundT(-lb_v, v);
          } else {
            lb = GaugeBoundT::minus_infinity();
          }
        }
      }

      GaugeBoundT ub = this->_ub;
      if (this->_ub.is_finite()) {
        Number ub_v = this->_ub.coeff(v);
        if (ub_v > 0) {
          if (u.is_finite()) {
            ub = this->_ub + ub_v * (*u.number()) + GaugeBoundT(-ub_v, v);
          } else {
            ub = GaugeBoundT::plus_infinity();
          }
        } else if (ub_v < 0) {
          ub = this->_ub + ub_v * l + GaugeBoundT(-ub_v, v);
        }
      }

      return Gauge(lb, ub);
    }
  }

  /// \brief Increment counter, as described in the paper
  ///
  /// Note that there is a typo in the paper, it should be:
  /// [min(a_0 - k*a_j, b_0 - k*b_j) + ..; max(a_0 - k*a_j, b_0 - k*_bj) + ...]
  Gauge counter_incr(VariableRef v, const Number& k) const {
    if (this->is_bottom()) {
      return bottom();
    } else if (this->_lb.is_finite() && this->_ub.is_finite()) {
      Number lb_new_cst = this->_lb.constant() - k * this->_lb.coeff(v);
      Number ub_new_cst = this->_ub.constant() - k * this->_ub.coeff(v);
      return Gauge(this->_lb - this->_lb.constant() +
                       min(lb_new_cst, ub_new_cst),
                   this->_ub - this->_ub.constant() +
                       max(lb_new_cst, ub_new_cst));
    } else if (this->_lb.is_finite()) {
      return Gauge(this->_lb - k * this->_lb.coeff(v),
                   GaugeBoundT::plus_infinity());
    } else if (this->_ub.is_finite()) {
      return Gauge(GaugeBoundT::minus_infinity(),
                   this->_ub - k * this->_ub.coeff(v));
    } else {
      return top();
    }
  }

private:
  /// \brief Division with rounding towards -oo
  static Number div_round_low(Number num, Number den) {
    if (den < 0) {
      num = -num;
      den = -den;
    }
    Number q = num / den;
    Number r = num % den;
    if (r == 0 || num > 0) {
      return q;
    } else {
      return q - 1;
    }
  }

  /// \brief Division with rounding towards +oo
  static Number div_round_up(Number num, Number den) {
    if (den < 0) {
      num = -num;
      den = -den;
    }
    Number q = num / den;
    Number r = num % den;
    if (r == 0 || num < 0) {
      return q;
    } else {
      return q + 1;
    }
  }

public:
  /// \brief Widening by linear interpolation
  ///
  /// \f$G \nabla^{k}_{u,v} G'\f$ in Arnaud's paper
  Gauge widening_interpol(const Gauge& other,
                          VariableRef k,
                          const Number& u,
                          const ConstantT& v) const {
    ikos_assert(ConstantT(u) != v);

    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      bool lb_is_finite = this->_lb.is_finite() && other._lb.is_finite();
      bool ub_is_finite = this->_ub.is_finite() && other._ub.is_finite();

      Number lb_slope;
      Number ub_slope;
      if (v.is_number()) {
        Number v_num = *v.number();

        if (lb_is_finite) {
          lb_slope =
              div_round_low((other._lb.constant() +
                             other._lb.coeff(k) * v_num) -
                                (this->_lb.constant() + this->_lb.coeff(k) * u),
                            v_num - u);
        }
        if (ub_is_finite) {
          ub_slope =
              div_round_up((other._ub.constant() + other._ub.coeff(k) * v_num) -
                               (this->_ub.constant() + this->_ub.coeff(k) * u),
                           v_num - u);
        }
      } else {
        if (lb_is_finite) {
          lb_slope = other._lb.coeff(k);
        }
        if (ub_is_finite) {
          ub_slope = other._ub.coeff(k);
        }
      }

      Number lb_new_cst;
      Number ub_new_cst;
      if (lb_is_finite) {
        lb_new_cst =
            this->_lb.constant() + this->_lb.coeff(k) * u - lb_slope * u;

        if (v.is_top()) {
          lb_new_cst = min(lb_new_cst, other._lb.constant());
        }
      }
      if (ub_is_finite) {
        ub_new_cst =
            this->_ub.constant() + this->_ub.coeff(k) * u - ub_slope * u;

        if (v.is_top()) {
          ub_new_cst = max(ub_new_cst, other._ub.constant());
        }
      }

      GaugeBoundT lb = min(this->_lb, other._lb);
      GaugeBoundT ub = max(this->_ub, other._ub);

      if (lb_is_finite && ub_is_finite) {
        lb += GaugeBoundT(min(lb_new_cst, ub_new_cst) - lb.constant()) +
              GaugeBoundT(min(lb_slope, ub_slope) - lb.coeff(k), k);
        ub += GaugeBoundT(max(lb_new_cst, ub_new_cst) - ub.constant()) +
              GaugeBoundT(max(lb_slope, ub_slope) - ub.coeff(k), k);
      } else if (lb_is_finite) {
        lb += GaugeBoundT(lb_new_cst - lb.constant()) +
              GaugeBoundT(lb_slope - lb.coeff(k), k);
      } else if (ub_is_finite) {
        ub += GaugeBoundT(ub_new_cst - ub.constant()) +
              GaugeBoundT(ub_slope - ub.coeff(k), k);
      }

      return Gauge(lb, ub);
    }
  }

  /// \brief Interval-like widening
  ///
  /// \f$\nabla_{I}\f$ in Arnaud's paper
  Gauge widening_interval(const Gauge& other) const {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return Gauge(this->_lb <= other._lb ? this->_lb
                                          : GaugeBoundT::minus_infinity(),
                   other._ub <= this->_ub ? this->_ub
                                          : GaugeBoundT::plus_infinity());
    }
  }

  /// \brief Interval-like widening with a threshold
  Gauge widening_interval_threshold(const Gauge& other,
                                    const Number& threshold) const {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      GaugeBoundT th = GaugeBoundT(threshold);

      GaugeBoundT lb = this->_lb;
      if (other._lb < this->_lb) {
        if (th <= other._lb) {
          lb = th;
        } else {
          lb = GaugeBoundT::minus_infinity();
        }
      }

      GaugeBoundT ub = this->_ub;
      if (other._ub > this->_ub) {
        if (th >= other._ub) {
          ub = th;
        } else {
          ub = GaugeBoundT::plus_infinity();
        }
      }

      return Gauge(lb, ub);
    }
  }

  /// \brief Widening
  ///
  /// Default to interval-like widening
  Gauge widening(const Gauge& other) const override {
    return this->widening_interval(other);
  }

  void widen_with(const Gauge& other) override {
    this->operator=(this->widening(other));
  }

  Gauge narrowing(const Gauge& other) const override {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else {
      GaugeBoundT lb = this->_lb.is_infinite() ? other._lb : this->_lb;
      GaugeBoundT ub = this->_ub.is_infinite() ? other._ub : this->_ub;

      if (lb.is_infinite() || ub.is_infinite() ||
          (lb.is_constant() && ub.is_constant())) {
        // in that case, it is safe to use [lb, ub]
        return Gauge(lb, ub);
      } else {
        // use max(lb, ub) so that the gauge is not empty
        return Gauge(lb, max(lb, ub));
      }
    }
  }

  void narrow_with(const Gauge& other) override {
    this->operator=(this->narrowing(other));
  }

  /// \brief Interval-like narrowing with a threshold
  Gauge narrowing_interval_threshold(const Gauge& other,
                                     const Number& threshold) const {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else {
      GaugeBoundT th = GaugeBoundT(threshold);

      GaugeBoundT lb =
          this->_lb.is_infinite() || this->_lb == th ? other._lb : this->_lb;
      GaugeBoundT ub =
          this->_ub.is_infinite() || this->_ub == th ? other._ub : this->_ub;

      if (lb.is_infinite() || ub.is_infinite() ||
          (lb.is_constant() && ub.is_constant())) {
        // in that case, it is safe to use [lb, ub]
        return Gauge(lb, ub);
      } else {
        // use max(lb, ub) so that the gauge is not empty
        return Gauge(lb, max(lb, ub));
      }
    }
  }

  /// \brief If the gauge is a singleton [n, n], return n, otherwise return
  /// boost::none
  boost::optional< GaugeBoundT > singleton() const {
    if (!this->is_bottom() && this->_lb == this->_ub) {
      return this->_lb;
    } else {
      return boost::none;
    }
  }

  /// \brief Convert the gauge to an interval
  ///
  /// Return boost::none if one of the bound contains a variable.
  boost::optional< IntervalT > interval() const {
    if (this->is_bottom()) {
      return IntervalT::bottom();
    } else {
      boost::optional< BoundT > lb = this->_lb.to_bound();
      boost::optional< BoundT > ub = this->_ub.to_bound();
      if (lb && ub) {
        return IntervalT(*lb, *ub);
      } else {
        return boost::none;
      }
    }
  }

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else {
      o << "[" << this->_lb << ", " << this->_ub << "]";
    }
  }

  static std::string name() { return "gauge"; }

}; // end class Gauge

/// \brief Write a gauge on a stream
template < typename Number, typename VariableRef >
inline std::ostream& operator<<(std::ostream& o,
                                const Gauge< Number, VariableRef >& gauge) {
  gauge.dump(o);
  return o;
}

} // end namespace numeric
} // end namespace core
} // end namespace ikos
