/*******************************************************************************
 *
 * \file
 * \brief Interval class
 *
 * Author: Arnaud J. Venet
 *
 * Contributors:
 *   * Alexandre C. D. Wimmers
 *   * Maxime Arthaud
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

#include <type_traits>

#include <boost/optional.hpp>

#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/linear_constraint.hpp>
#include <ikos/core/number.hpp>
#include <ikos/core/number/bound.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief Interval abstract value
///
/// Implemented as a pair of bounds
template < typename Number >
class Interval final : public core::AbstractDomain< Interval< Number > > {
public:
  using NumberT = Number;
  using BoundT = Bound< Number >;

private:
  // Lower bound
  BoundT _lb;

  // Upper bound
  BoundT _ub;

  // Invariant: is_bottom() <=> _lb = 1 && _ub = 0

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top interval [-oo, +oo]
  explicit Interval(TopTag)
      : _lb(BoundT::minus_infinity()), _ub(BoundT::plus_infinity()) {}

  /// \brief Create the bottom interval
  explicit Interval(BottomTag) : _lb(1), _ub(0) {}

public:
  /// \brief Create the interval [-oo, +oo]
  static Interval top() { return Interval(TopTag{}); }

  /// \brief Create the bottom interval
  static Interval bottom() { return Interval(BottomTag{}); }

  /// \brief Create the interval [n, n]
  explicit Interval(int n) : _lb(n), _ub(n) {}

  /// \brief Create the interval [n, n]
  explicit Interval(const Number& n) : _lb(n), _ub(n) {}

  /// \brief Create the interval [b, b]
  explicit Interval(const BoundT& b) : _lb(b), _ub(b) {
    ikos_assert(!b.is_infinite());
  }

  /// \brief Create the interval [lb, ub]
  ///
  /// If lb > ub, the interval is bottom.
  Interval(BoundT lb, BoundT ub) : _lb(std::move(lb)), _ub(std::move(ub)) {
    ikos_assert(this->_lb.is_finite() || this->_ub.is_finite() ||
                this->_lb != this->_ub);
    if (this->_lb > this->_ub) {
      this->_lb = 1;
      this->_ub = 0;
    }
  }

  /// \brief Copy constructor
  Interval(const Interval&) noexcept(
      std::is_nothrow_copy_constructible< Number >::value) = default;

  /// \brief Move constructor
  Interval(Interval&&) noexcept(
      std::is_nothrow_move_constructible< Number >::value) = default;

  /// \brief Copy assignment operator
  Interval& operator=(const Interval&) noexcept(
      std::is_nothrow_copy_assignable< Number >::value) = default;

  /// \brief Move assignment operator
  Interval& operator=(Interval&&) noexcept(
      std::is_nothrow_move_assignable< Number >::value) = default;

  /// \brief Destructor
  ~Interval() override = default;

  /// \brief Return the lower bound
  const BoundT& lb() const {
    ikos_assert(!this->is_bottom());
    return this->_lb;
  }

  /// \brief Return the upper bound
  const BoundT& ub() const {
    ikos_assert(!this->is_bottom());
    return this->_ub;
  }

  void normalize() override {}

  bool is_bottom() const override { return this->_lb > this->_ub; }

  bool is_top() const override {
    return this->_lb.is_infinite() && this->_ub.is_infinite();
  }

  /// \brief Return true if the interval is [0, 0]
  bool is_zero() const { return this->_lb == 0 && this->_ub == 0; }

  void set_to_bottom() override {
    this->_lb = 1;
    this->_ub = 0;
  }

  void set_to_top() override {
    this->_lb = BoundT::minus_infinity();
    this->_ub = BoundT::plus_infinity();
  }

  /// \brief Return the interval [-oo, ub]
  Interval lower_half_line() const {
    ikos_assert(!this->is_bottom());
    return Interval(BoundT::minus_infinity(), this->_ub);
  }

  /// \brief Return the interval [lb, +oo]
  Interval upper_half_line() const {
    ikos_assert(!this->is_bottom());
    return Interval(this->_lb, BoundT::plus_infinity());
  }

  bool leq(const Interval& other) const override {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      return other._lb <= this->_lb && this->_ub <= other._ub;
    }
  }

  bool equals(const Interval& other) const override {
    if (this->is_bottom()) {
      return other.is_bottom();
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_lb == other._lb && this->_ub == other._ub;
    }
  }

  Interval join(const Interval& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return Interval(min(this->_lb, other._lb), max(this->_ub, other._ub));
    }
  }

  void join_with(const Interval& other) override {
    this->operator=(this->join(other));
  }

  Interval widening(const Interval& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return Interval(other._lb < this->_lb ? BoundT::minus_infinity()
                                            : this->_lb,
                      this->_ub < other._ub ? BoundT::plus_infinity()
                                            : this->_ub);
    }
  }

  void widen_with(const Interval& other) override {
    this->operator=(this->widening(other));
  }

  Interval widening_threshold(const Interval& other,
                              const Number& threshold) const {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      BoundT th = BoundT(threshold);

      BoundT lb = this->_lb;
      if (other._lb < this->_lb) {
        if (th <= other._lb) {
          lb = th;
        } else {
          lb = BoundT::minus_infinity();
        }
      }

      BoundT ub = this->_ub;
      if (other._ub > this->_ub) {
        if (th >= other._ub) {
          ub = th;
        } else {
          ub = BoundT::plus_infinity();
        }
      }

      return Interval(lb, ub);
    }
  }

  void widen_threshold_with(const Interval& other, const Number& threshold) {
    this->operator=(this->widening_threshold(other, threshold));
  }

  Interval meet(const Interval& other) const override {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else {
      return Interval(max(this->_lb, other._lb), min(this->_ub, other._ub));
    }
  }

  void meet_with(const Interval& other) override {
    this->operator=(this->meet(other));
  }

  Interval narrowing(const Interval& other) const override {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else {
      return Interval(this->_lb.is_infinite() ? other._lb : this->_lb,
                      this->_ub.is_infinite() ? other._ub : this->_ub);
    }
  }

  void narrow_with(const Interval& other) override {
    this->operator=(this->narrowing(other));
  }

  Interval narrowing_threshold(const Interval& other,
                               const Number& threshold) const {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else {
      return Interval(this->_lb.is_infinite() || this->_lb == BoundT(threshold)
                          ? other._lb
                          : this->_lb,
                      this->_ub.is_infinite() || this->_ub == BoundT(threshold)
                          ? other._ub
                          : this->_ub);
    }
  }

  void narrow_threshold_with(const Interval& other, const Number& threshold) {
    this->operator=(this->narrowing_threshold(other, threshold));
  }

  /// \brief Unary minus
  Interval operator-() const {
    if (this->is_bottom()) {
      return bottom();
    } else {
      return Interval(-this->_ub, -this->_lb);
    }
  }

  /// \brief Add an interval
  void operator+=(const Interval& other) {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_lb += other._lb;
      this->_ub += other._ub;
    }
  }

  /// \brief Substract an interval
  void operator-=(const Interval& other) {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_lb -= other._ub;
      this->_ub -= other._lb;
    }
  }

  /// \brief Helper for mod([a, b], n)
  ///
  /// If mod([a, b], n) is equivalent to [a, b] - x, returns x
  /// Otherwise, returns boost::none.
  boost::optional< Number > mod_to_sub(const Number& n) {
    ikos_assert_msg(n != 0, "division by zero");

    if (this->_lb.is_infinite() || this->_ub.is_infinite()) {
      return boost::none;
    }

    ZNumber lb = *this->_lb.number();
    ZNumber ub = *this->_ub.number();

    ZNumber mod_lb = mod(lb, n);
    ZNumber mod_ub = mod(ub, n);

    if (mod_ub - mod_lb != ub - lb) {
      return boost::none;
    }

    return lb - mod_lb;
  }

  /// \brief If the interval is a singleton [n, n], return n, otherwise return
  /// boost::none
  boost::optional< Number > singleton() const {
    if (this->_lb.is_finite() && this->_lb == this->_ub) {
      return this->_lb.number();
    } else {
      return boost::none;
    }
  }

  /// \brief Return true if the interval contains n
  bool contains(int n) const { return this->contains(Number(n)); }

  /// \brief Return true if the interval contains n
  bool contains(Number n) const {
    if (this->is_bottom()) {
      return false;
    } else {
      BoundT b(std::move(n));
      return this->_lb <= b && b <= this->_ub;
    }
  }

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else {
      o << "[" << this->_lb << ", " << this->_ub << "]";
    }
  }

  static std::string name() { return "interval"; }

}; // end class Interval

/// \brief Add intervals
template < typename Number >
inline Interval< Number > operator+(const Interval< Number >& lhs,
                                    const Interval< Number >& rhs) {
  using IntervalT = Interval< Number >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return IntervalT::bottom();
  } else {
    return IntervalT(lhs.lb() + rhs.lb(), lhs.ub() + rhs.ub());
  }
}

/// \brief Substract intervals
template < typename Number >
inline Interval< Number > operator-(const Interval< Number >& lhs,
                                    const Interval< Number >& rhs) {
  using IntervalT = Interval< Number >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return IntervalT::bottom();
  } else {
    return IntervalT(lhs.lb() - rhs.ub(), lhs.ub() - rhs.lb());
  }
}

/// \brief Multiply intervals
template < typename Number >
inline Interval< Number > operator*(const Interval< Number >& lhs,
                                    const Interval< Number >& rhs) {
  using BoundT = Bound< Number >;
  using IntervalT = Interval< Number >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return IntervalT::bottom();
  } else {
    BoundT ll = lhs.lb() * rhs.lb();
    BoundT lu = lhs.lb() * rhs.ub();
    BoundT ul = lhs.ub() * rhs.lb();
    BoundT uu = lhs.ub() * rhs.ub();
    return IntervalT(min(ll, lu, ul, uu), max(ll, lu, ul, uu));
  }
}

/// \brief Divide intervals
inline Interval< ZNumber > operator/(const Interval< ZNumber >& lhs,
                                     const Interval< ZNumber >& rhs) {
  using BoundT = Bound< ZNumber >;
  using IntervalT = Interval< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return IntervalT::bottom();
  } else {
    if (rhs.contains(0)) {
      IntervalT l(rhs.lb(), BoundT(-1));
      IntervalT u(BoundT(1), rhs.ub());
      return (lhs / l).join(lhs / u);
    } else if (lhs.contains(0)) {
      IntervalT l(lhs.lb(), BoundT(-1));
      IntervalT u(BoundT(1), lhs.ub());
      return (l / rhs).join(u / rhs).join(IntervalT(0));
    } else {
      // Neither the dividend nor the divisor contains 0
      BoundT ll = lhs.lb() / rhs.lb();
      BoundT lu = lhs.lb() / rhs.ub();
      BoundT ul = lhs.ub() / rhs.lb();
      BoundT uu = lhs.ub() / rhs.ub();
      return IntervalT(min(ll, lu, ul, uu), max(ll, lu, ul, uu));
    }
  }
}

/// \brief Divide intervals
inline Interval< QNumber > operator/(const Interval< QNumber >& lhs,
                                     const Interval< QNumber >& rhs) {
  using BoundT = Bound< QNumber >;
  using IntervalT = Interval< QNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return IntervalT::bottom();
  } else {
    boost::optional< QNumber > d = rhs.singleton();
    if (d && *d == 0) {
      // [_, _] / 0 = ⊥
      return IntervalT::bottom();
    } else if (rhs.contains(0)) {
      boost::optional< QNumber > n = lhs.singleton();
      if (n && *n == 0) {
        // 0 / [_, _] = 0
        return IntervalT(0);
      } else {
        return IntervalT::top();
      }
    } else {
      BoundT ll = lhs.lb() / rhs.lb();
      BoundT lu = lhs.lb() / rhs.ub();
      BoundT ul = lhs.ub() / rhs.lb();
      BoundT uu = lhs.ub() / rhs.ub();
      return IntervalT(min(ll, lu, ul, uu), max(ll, lu, ul, uu));
    }
  }
}

/// \brief Remainder of intervals
inline Interval< ZNumber > operator%(const Interval< ZNumber >& lhs,
                                     const Interval< ZNumber >& rhs) {
  using BoundT = Bound< ZNumber >;
  using IntervalT = Interval< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return IntervalT::bottom();
  } else {
    boost::optional< ZNumber > n = lhs.singleton();
    boost::optional< ZNumber > d = rhs.singleton();

    if (d && *d == 0) {
      // [_, _] % 0 = ⊥
      return IntervalT::bottom();
    } else if (n && d) {
      // [n, n] % [d, d] = [n % d, n % d]
      return IntervalT(*n % *d);
    } else {
      // [a, b] % [c, d] <= max(abs(a), abs(b))
      // [a, b] % [c, d] <= max(abs(c), abs(d)) - 1
      BoundT zero(0);
      BoundT n_ub = max(abs(lhs.lb()), abs(lhs.ub()));
      BoundT d_ub = max(abs(rhs.lb()), abs(rhs.ub())) - BoundT(1);
      BoundT ub = min(n_ub, d_ub);

      if (lhs.lb() < zero) {
        if (lhs.ub() > zero) {
          return IntervalT(-ub, ub);
        } else {
          return IntervalT(-ub, zero);
        }
      } else {
        return IntervalT(zero, ub);
      }
    }
  }
}

/// \brief Modulo of intervals
inline Interval< ZNumber > mod(const Interval< ZNumber >& lhs,
                               const Interval< ZNumber >& rhs) {
  using BoundT = Bound< ZNumber >;
  using IntervalT = Interval< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return IntervalT::bottom();
  } else {
    boost::optional< ZNumber > n = lhs.singleton();
    boost::optional< ZNumber > d = rhs.singleton();

    if (d && *d == 0) {
      // mod([_, _], 0) = ⊥
      return IntervalT::bottom();
    } else if (n && d) {
      // mod([n, n], [d, d]) = [mod(n, d), mod(n, d)]
      return IntervalT(mod(*n, *d));
    } else if (d && lhs.lb().is_finite() && lhs.ub().is_finite()) {
      // mod([lb, ub], d)
      ZNumber lb = *lhs.lb().number();
      ZNumber ub = *lhs.ub().number();

      ZNumber mod_lb = mod(lb, *d);
      ZNumber mod_ub = mod(ub, *d);

      if (mod_ub - mod_lb == ub - lb) {
        return IntervalT(BoundT(mod_lb), BoundT(mod_ub));
      } else {
        return IntervalT(BoundT(0), BoundT(abs(*d) - 1));
      }
    } else {
      // mod([a, b], [c, d]) <= max(abs(c), abs(d)) - 1
      BoundT ub = max(abs(rhs.lb()), abs(rhs.ub())) - BoundT(1);
      return IntervalT(BoundT(0), ub);
    }
  }
}

/// \brief Left binary shift of intervals
inline Interval< ZNumber > operator<<(const Interval< ZNumber >& lhs,
                                      const Interval< ZNumber >& rhs) {
  using BoundT = Bound< ZNumber >;
  using IntervalT = Interval< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return IntervalT::bottom();
  } else {
    // rhs >= 0
    IntervalT shift = rhs.meet(IntervalT(BoundT(0), BoundT::plus_infinity()));

    if (shift.is_bottom()) {
      // Invalid operand
      return IntervalT::bottom();
    }

    // [a, b] << [c, d] = [a, b] * [1 << c, 1 << d]
    IntervalT coeff(BoundT(1 << *shift.lb().number()),
                    shift.ub().is_finite() ? BoundT(1 << *shift.ub().number())
                                           : BoundT::plus_infinity());
    return lhs * coeff;
  }
}

/// \brief Right binary shift of intervals
inline Interval< ZNumber > operator>>(const Interval< ZNumber >& lhs,
                                      const Interval< ZNumber >& rhs) {
  using BoundT = Bound< ZNumber >;
  using IntervalT = Interval< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return IntervalT::bottom();
  } else {
    // rhs >= 0
    IntervalT shift = rhs.meet(IntervalT(BoundT(0), BoundT::plus_infinity()));

    if (shift.is_bottom()) {
      // Invalid operand
      return IntervalT::bottom();
    }

    if (lhs.contains(0)) {
      IntervalT l(lhs.lb(), BoundT(-1));
      IntervalT u(BoundT(1), lhs.ub());
      return (l >> rhs).join(u >> rhs).join(IntervalT(0));
    } else {
      // [a, b] >> [c, d] \in [min(a >> c, a >> d, b >> c, b >> d),
      //                       max(a >> c, a >> d, b >> c, b >> d)]
      BoundT ll = lhs.lb() >> shift.lb();
      BoundT lu = lhs.lb() >> shift.ub();
      BoundT ul = lhs.ub() >> shift.lb();
      BoundT uu = lhs.ub() >> shift.ub();
      return IntervalT(min(ll, lu, ul, uu), max(ll, lu, ul, uu));
    }
  }
}

/// \brief Bitwise AND of intervals
inline Interval< ZNumber > operator&(const Interval< ZNumber >& lhs,
                                     const Interval< ZNumber >& rhs) {
  using BoundT = Bound< ZNumber >;
  using IntervalT = Interval< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return IntervalT::bottom();
  } else {
    boost::optional< ZNumber > l = lhs.singleton();
    boost::optional< ZNumber > r = rhs.singleton();

    if (l && r) {
      // [l, l] & [r, r] = [l & r, l & r]
      return IntervalT(*l & *r);
    } else if ((l && *l == 0) || (r && *r == 0)) {
      // [a, b] & 0 = 0
      return IntervalT(0);
    } else if (l && *l == -1) {
      // -1 & [a, b] = [a, b]
      return rhs;
    } else if (r && *r == -1) {
      // [a, b] & -1 = [a, b]
      return lhs;
    } else {
      if (lhs.lb() >= BoundT(0) && rhs.lb() >= BoundT(0)) {
        // [a, b] & [c, d] <= [c, d] & b <= b
        // [a, b] & [c, d] <= [a, b] & d <= d
        return IntervalT(BoundT(0), min(lhs.ub(), rhs.ub()));
      } else if (lhs.lb() >= BoundT(0)) {
        return IntervalT(BoundT(0), lhs.ub());
      } else if (rhs.lb() >= BoundT(0)) {
        return IntervalT(BoundT(0), rhs.ub());
      } else {
        return IntervalT::top();
      }
    }
  }
}

/// \brief Bitwise OR of intervals
inline Interval< ZNumber > operator|(const Interval< ZNumber >& lhs,
                                     const Interval< ZNumber >& rhs) {
  using BoundT = Bound< ZNumber >;
  using IntervalT = Interval< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return IntervalT::bottom();
  } else {
    boost::optional< ZNumber > l = lhs.singleton();
    boost::optional< ZNumber > r = rhs.singleton();

    if (l && r) {
      // [l, l] | [r, r] = [l | r, l | r]
      return IntervalT(*l | *r);
    } else if ((l && *l == -1) || (r && *r == -1)) {
      // -1 | [a, b] = -1
      return IntervalT(-1);
    } else if (l && *l == 0) {
      // 0 | [a, b] = [a, b]
      return rhs;
    } else if (r && *r == 0) {
      // [a, b] | 0 = [a, b]
      return lhs;
    } else {
      if (lhs.lb() >= BoundT(0) && lhs.ub().is_finite() &&
          rhs.lb() >= BoundT(0) && rhs.ub().is_finite()) {
        ZNumber m = max(*lhs.ub().number(), *rhs.ub().number());
        ZNumber ub = (m + 1).next_power_of_2() - 1;
        return IntervalT(BoundT(0), BoundT(ub));
      } else {
        return IntervalT::top();
      }
    }
  }
}

/// \brief Bitwise XOR of intervals
inline Interval< ZNumber > operator^(const Interval< ZNumber >& lhs,
                                     const Interval< ZNumber >& rhs) {
  using BoundT = Bound< ZNumber >;
  using IntervalT = Interval< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return IntervalT::bottom();
  } else {
    boost::optional< ZNumber > l = lhs.singleton();
    boost::optional< ZNumber > r = rhs.singleton();

    if (l && r) {
      // [l, l] ^ [r, r] = [l ^ r, l ^ r]
      return IntervalT(*l ^ *r);
    } else if (l && *l == 0) {
      // 0 ^ [a, b] = [a, b]
      return rhs;
    } else if (r && *r == 0) {
      // [a, b] ^ 0 = [a, b]
      return lhs;
    } else {
      if (lhs.lb() >= BoundT(0) && lhs.ub().is_finite() &&
          rhs.lb() >= BoundT(0) && rhs.ub().is_finite()) {
        ZNumber m = max(*lhs.ub().number(), *rhs.ub().number());
        ZNumber ub = (m + 1).next_power_of_2() - 1;
        return IntervalT(BoundT(0), BoundT(ub));
      } else {
        return IntervalT::top();
      }
    }
  }
}

/// \brief Write an interval on a stream
template < typename Number >
inline std::ostream& operator<<(std::ostream& o,
                                const Interval< Number >& interval) {
  interval.dump(o);
  return o;
}

/// \brief Return the constraint system: i.lb() <= e <= i.ub()
template < typename Number, typename VariableRef >
inline LinearConstraintSystem< Number, VariableRef > within_interval(
    const LinearExpression< Number, VariableRef >& e,
    const Interval< Number >& i) {
  LinearConstraintSystem< Number, VariableRef > csts;

  if (i.is_bottom()) {
    csts.add(LinearConstraint< Number, VariableRef >::contradiction());
  } else {
    boost::optional< Number > lb = i.lb().number();
    boost::optional< Number > ub = i.ub().number();

    if (lb && ub && *lb == *ub) {
      csts.add(e == *ub);
    } else {
      if (lb) {
        csts.add(e >= *lb);
      }
      if (ub) {
        csts.add(e <= *ub);
      }
    }
  }

  return csts;
}

/// \brief Return the constraint system: i.lb() <= v <= i.ub()
template < typename Number, typename VariableRef >
inline LinearConstraintSystem< Number, VariableRef > within_interval(
    VariableExpression< Number, VariableRef > v, const Interval< Number >& i) {
  return within_interval(LinearExpression< Number, VariableRef >(v), i);
}

/// \brief Return the constraint system: i.lb() <= v <= i.ub()
template < typename Number, typename VariableRef >
inline LinearConstraintSystem< Number, VariableRef > within_interval(
    VariableRef v, const Interval< Number >& i) {
  return within_interval(LinearExpression< Number, VariableRef >(v), i);
}

/// \brief Interval on unlimited precision integers
using ZInterval = Interval< ZNumber >;

/// \brief Interval on unlimited precision rationals
using QInterval = Interval< QNumber >;

} // end namespace numeric
} // end namespace core
} // end namespace ikos
