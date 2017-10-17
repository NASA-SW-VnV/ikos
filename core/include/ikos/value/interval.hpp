/**************************************************************************/ /**
 *
 * \file
 * \brief interval value for interval domain.
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

#ifndef IKOS_VALUE_INTERVAL_HPP
#define IKOS_VALUE_INTERVAL_HPP

#include <iosfwd>
#include <string>

#include <boost/optional.hpp>

#include <ikos/algorithms/linear_constraints.hpp>
#include <ikos/common/types.hpp>
#include <ikos/number.hpp>

namespace ikos {

template < typename Number >
class bound;

/// \brief Bound
///
/// This is either -oo, +oo or a number.
template < typename Number >
class bound {
  template < typename Any >
  friend class bound;

public:
  typedef bound< Number > bound_t;

private:
  bool _is_infinite;
  Number _n;

private:
  bound();

  bound(bool is_infinite, Number n) : _is_infinite(is_infinite), _n(n) {
    if (is_infinite) {
      if (n > 0) {
        this->_n = 1;
      } else {
        this->_n = -1;
      }
    }
  }

  bound(bool is_infinite, int n) : _is_infinite(is_infinite), _n(n) {
    if (is_infinite) {
      if (n > 0) {
        this->_n = 1;
      } else {
        this->_n = -1;
      }
    }
  }

public:
  static bound_t min(bound_t x, bound_t y) { return x.operator<=(y) ? x : y; }

  static bound_t min(bound_t x, bound_t y, bound_t z) {
    return min(x, min(y, z));
  }

  static bound_t min(bound_t x, bound_t y, bound_t z, bound_t t) {
    return min(x, min(y, z, t));
  }

  static bound_t max(bound_t x, bound_t y) { return x.operator<=(y) ? y : x; }

  static bound_t max(bound_t x, bound_t y, bound_t z) {
    return max(x, max(y, z));
  }

  static bound_t max(bound_t x, bound_t y, bound_t z, bound_t t) {
    return max(x, max(y, z, t));
  }

  static bound_t plus_infinity() { return bound_t(true, 1); }

  static bound_t minus_infinity() { return bound_t(true, -1); }

public:
  template < typename N,
             class = std::enable_if_t< is_supported_integral< N >::value > >
  bound(N n) : _is_infinite(false), _n(n) {}

  bound(const std::string& s) : _n(1) {
    if (s == "+oo") {
      this->_is_infinite = true;
    } else if (s == "-oo") {
      this->_is_infinite = true;
      this->_n = -1;
    } else {
      this->_is_infinite = false;
      this->_n = Number::from_string(s);
    }
  }

  bound(Number n) : _is_infinite(false), _n(n) {}

  bound(const bound_t& o) : _is_infinite(o._is_infinite), _n(o._n) {}

  bound_t& operator=(bound_t o) {
    this->_is_infinite = o._is_infinite;
    this->_n = o._n;
    return *this;
  }

  bool is_infinite() const { return this->_is_infinite; }

  bool is_finite() const { return !this->_is_infinite; }

  bool is_plus_infinity() const { return this->is_infinite() && this->_n > 0; }

  bool is_minus_infinity() const { return this->is_infinite() && this->_n < 0; }

  bound_t operator-() const { return bound_t(this->_is_infinite, -this->_n); }

  bound_t operator+(bound_t x) const {
    if (this->is_finite() && x.is_finite()) {
      return bound_t(false, this->_n + x._n);
    } else if (this->is_finite() && x.is_infinite()) {
      return x;
    } else if (this->is_infinite() && x.is_finite()) {
      return *this;
    } else if (this->_n == x._n) {
      return *this;
    } else {
      throw logic_error("bound: undefined operation -oo + +oo");
    }
  }

  bound_t& operator+=(bound_t x) { return this->operator=(this->operator+(x)); }

  bound_t operator-(bound_t x) const { return this->operator+(x.operator-()); }

  bound_t& operator-=(bound_t x) { return this->operator=(this->operator-(x)); }

  bound_t operator*(bound_t x) const {
    if (this->_n == 0 || x._n == 0) {
      return bound_t(false, 0);
    } else {
      return bound_t(this->_is_infinite || x._is_infinite, this->_n * x._n);
    }
  }

  bound_t& operator*=(bound_t x) { return this->operator=(this->operator*(x)); }

  bound_t operator/(bound_t x) const {
    if (x._n == 0) {
      throw logic_error("bound: division by zero");
    } else if (this->is_finite() && x.is_finite()) {
      return bound_t(false, _n / x._n);
    } else if (this->is_finite() && x.is_infinite()) {
      if (this->_n > 0) {
        return x;
      } else if (this->_n == 0) {
        return *this;
      } else {
        return x.operator-();
      }
    } else if (this->is_infinite() && x.is_finite()) {
      if (x._n > 0) {
        return *this;
      } else {
        return this->operator-();
      }
    } else {
      return bound_t(true, this->_n * x._n);
    }
  }

  bound_t& operator/=(bound_t x) { return this->operator=(this->operator/(x)); }

  bool operator<(bound_t x) const { return !this->operator>=(x); }

  bool operator>(bound_t x) const { return !this->operator<=(x); }

  bool operator==(bound_t x) const {
    return this->_is_infinite == x._is_infinite && this->_n == x._n;
  }

  bool operator!=(bound_t x) const { return !this->operator==(x); }

  /* operator<= and operator>= use a somewhat optimized implementation.
   * results include up to 20% improvements in performance in the octagon
   * domain over a more naive implementation.
   */
  bool operator<=(bound_t x) const {
    if (this->_is_infinite xor x._is_infinite) {
      if (this->_is_infinite) {
        return this->_n < 0;
      }
      return x._n > 0;
    }
    return this->_n <= x._n;
  }

  bool operator>=(bound_t x) const {
    if (this->_is_infinite xor x._is_infinite) {
      if (this->_is_infinite) {
        return this->_n > 0;
      }
      return x._n < 0;
    }
    return this->_n >= x._n;
  }

  bound_t abs() const {
    if (this->operator>=(0)) {
      return *this;
    } else {
      return this->operator-();
    }
  }

  boost::optional< Number > number() const {
    if (this->is_infinite()) {
      return boost::optional< Number >();
    } else {
      return boost::optional< Number >(this->_n);
    }
  }

  void write(std::ostream& o) {
    if (this->is_plus_infinity()) {
      o << "+oo";
    } else if (this->is_minus_infinity()) {
      o << "-oo";
    } else {
      o << this->_n;
    }
  }

}; // end class bound

template < typename Number >
inline std::ostream& operator<<(std::ostream& o, bound< Number > b) {
  b.write(o);
  return o;
}

typedef bound< z_number > z_bound;
typedef bound< q_number > q_bound;

template < typename Number >
class interval;

/// \brief Interval
///
/// This is implemented as a pair of bound
template < typename Number >
class interval {
public:
  typedef bound< Number > bound_t;
  typedef interval< Number > interval_t;

private:
  bound_t _lb;
  bound_t _ub;

public:
  static interval_t top() {
    return interval_t(bound_t::minus_infinity(), bound_t::plus_infinity());
  }

  static interval_t bottom() { return interval_t(); }

private:
  interval() : _lb(0), _ub(-1) {}

  static Number abs(Number x) { return x < 0 ? -x : x; }

  static Number max(Number x, Number y) { return x <= y ? y : x; }

  static Number min(Number x, Number y) { return x < y ? x : y; }

public:
  interval(bound_t lb, bound_t ub) : _lb(lb), _ub(ub) {
    if (lb > ub) {
      this->_lb = 0;
      this->_ub = -1;
    }
  }

  interval(bound_t b) : _lb(b), _ub(b) {
    if (b.is_infinite()) {
      this->_lb = 0;
      this->_ub = -1;
    }
  }

  interval(Number n) : _lb(n), _ub(n) {}

  interval(const std::string& b) : _lb(b), _ub(b) {
    if (this->_lb.is_infinite()) {
      this->_lb = 0;
      this->_ub = -1;
    }
  }

  interval(const interval_t& i) : _lb(i._lb), _ub(i._ub) {}

  interval_t& operator=(interval_t i) {
    this->_lb = i._lb;
    this->_ub = i._ub;
    return *this;
  }

  bound_t lb() const { return this->_lb; }

  bound_t ub() const { return this->_ub; }

  bool is_bottom() const { return this->_lb > this->_ub; }

  bool is_top() const {
    return this->_lb.is_infinite() && this->_ub.is_infinite();
  }

  interval_t lower_half_line() const {
    return interval_t(bound_t::minus_infinity(), this->_ub);
  }

  interval_t upper_half_line() const {
    return interval_t(this->_lb, bound_t::plus_infinity());
  }

  bool operator==(interval_t x) const {
    if (is_bottom()) {
      return x.is_bottom();
    } else {
      return (this->_lb == x._lb) && (this->_ub == x._ub);
    }
  }

  bool operator!=(interval_t x) const { return !this->operator==(x); }

  bool operator<=(interval_t x) const {
    if (this->is_bottom()) {
      return true;
    } else if (x.is_bottom()) {
      return false;
    } else {
      return (x._lb <= this->_lb) && (this->_ub <= x._ub);
    }
  }

  interval_t operator|(interval_t x) const {
    if (this->is_bottom()) {
      return x;
    } else if (x.is_bottom()) {
      return *this;
    } else {
      return interval_t(bound_t::min(this->_lb, x._lb),
                        bound_t::max(this->_ub, x._ub));
    }
  }

  interval_t operator&(interval_t x) const {
    if (this->is_bottom() || x.is_bottom()) {
      return this->bottom();
    } else {
      return interval_t(bound_t::max(this->_lb, x._lb),
                        bound_t::min(this->_ub, x._ub));
    }
  }

  interval_t operator||(interval_t x) const {
    if (this->is_bottom()) {
      return x;
    } else if (x.is_bottom()) {
      return *this;
    } else {
      return interval_t(x._lb < this->_lb ? bound_t::minus_infinity()
                                          : this->_lb,
                        this->_ub < x._ub ? bound_t::plus_infinity()
                                          : this->_ub);
    }
  }

  interval_t operator&&(interval_t x) const {
    if (this->is_bottom() || x.is_bottom()) {
      return this->bottom();
    } else {
      return interval_t(this->_lb.is_infinite() && x._lb.is_finite()
                            ? x._lb
                            : this->_lb,
                        this->_ub.is_infinite() && x._ub.is_finite()
                            ? x._ub
                            : this->_ub);
    }
  }

  interval_t operator+(interval_t x) const {
    if (this->is_bottom() || x.is_bottom()) {
      return this->bottom();
    } else {
      return interval_t(this->_lb + x._lb, this->_ub + x._ub);
    }
  }

  interval_t& operator+=(interval_t x) {
    return this->operator=(this->operator+(x));
  }

  interval_t operator-() const {
    if (this->is_bottom()) {
      return this->bottom();
    } else {
      return interval_t(-this->_ub, -this->_lb);
    }
  }

  interval_t operator-(interval_t x) const {
    if (this->is_bottom() || x.is_bottom()) {
      return this->bottom();
    } else {
      return interval_t(this->_lb - x._ub, this->_ub - x._lb);
    }
  }

  interval_t& operator-=(interval_t x) {
    return this->operator=(this->operator-(x));
  }

  interval_t operator*(interval_t x) const {
    if (this->is_bottom() || x.is_bottom()) {
      return this->bottom();
    } else {
      bound_t ll = this->_lb * x._lb;
      bound_t lu = this->_lb * x._ub;
      bound_t ul = this->_ub * x._lb;
      bound_t uu = this->_ub * x._ub;
      return interval_t(bound_t::min(ll, lu, ul, uu),
                        bound_t::max(ll, lu, ul, uu));
    }
  }

  interval_t& operator*=(interval_t x) {
    return this->operator=(this->operator*(x));
  }

  interval_t operator/(interval_t x) const;

  interval_t& operator/=(interval_t x) {
    return this->operator=(this->operator/(x));
  }

  boost::optional< Number > singleton() const {
    if (!this->is_bottom() && this->_lb == this->_ub) {
      return this->_lb.number();
    } else {
      return boost::optional< Number >();
    }
  }

  bool operator[](Number n) const {
    if (this->is_bottom()) {
      return false;
    } else {
      bound_t b(n);
      return (this->_lb <= b) && (b <= this->_ub);
    }
  }

  template < typename N,
             class = std::enable_if_t< is_supported_integral< N >::value > >
  bool operator[](N n) const {
    if (this->is_bottom()) {
      return false;
    } else {
      bound_t b(n);
      return (this->_lb <= b) && (b <= this->_ub);
    }
  }

  void write(std::ostream& o) {
    if (is_bottom()) {
      o << "_|_";
    } else {
      o << "[" << _lb << ", " << _ub << "]";
    }
  }

private:
  interval_t unknown_unary_op() const {
    if (this->is_bottom()) {
      return this->bottom();
    } else {
      return this->top();
    }
  }

  interval_t unknown_binary_op(interval_t x) const {
    if (this->is_bottom() || x.is_bottom()) {
      return this->bottom();
    } else {
      return this->top();
    }
  }

public:
  // division and remainder operations

  interval_t UDiv(interval_t x) const { return this->unknown_binary_op(x); }

  interval_t SRem(interval_t x) const { return this->unknown_binary_op(x); }

  interval_t URem(interval_t x) const { return this->unknown_binary_op(x); }

  // bitwise operations

  interval_t Trunc(uint64_t /* from */, uint64_t /* to */) const {
    return this->unknown_unary_op();
  }

  interval_t ZExt(uint64_t /* from */, uint64_t /* to */) const {
    return this->unknown_unary_op();
  }

  interval_t SExt(uint64_t /* from */, uint64_t /* to */) const {
    return this->unknown_unary_op();
  }

  interval_t And(interval_t x) const { return this->unknown_binary_op(x); }

  interval_t Or(interval_t x) const { return this->unknown_binary_op(x); }

  interval_t Xor(interval_t x) const { return this->unknown_binary_op(x); }

  interval_t Shl(interval_t x) const { return this->unknown_binary_op(x); }

  interval_t LShr(interval_t x) const { return this->unknown_binary_op(x); }

  interval_t AShr(interval_t x) const { return this->unknown_binary_op(x); }

}; // end class interval

template <>
inline interval< q_number > interval< q_number >::operator/(
    interval< q_number > x) const {
  if (this->is_bottom() || x.is_bottom()) {
    return this->bottom();
  } else {
    boost::optional< q_number > d = x.singleton();
    if (d && *d == 0) {
      // [_, _] / 0 = _|_
      return this->bottom();
    } else if (x[0]) {
      boost::optional< q_number > n = this->singleton();
      if (n && *n == 0) {
        // 0 / [_, _] = 0
        return interval_t(q_number(0));
      } else {
        return this->top();
      }
    } else {
      bound_t ll = this->_lb / x._lb;
      bound_t lu = this->_lb / x._ub;
      bound_t ul = this->_ub / x._lb;
      bound_t uu = this->_ub / x._ub;
      return interval_t(bound_t::min(ll, lu, ul, uu),
                        bound_t::max(ll, lu, ul, uu));
    }
  }
}

template <>
inline interval< z_number > interval< z_number >::operator/(
    interval< z_number > x) const {
  if (this->is_bottom() || x.is_bottom()) {
    return this->bottom();
  } else {
    if (x[0]) {
      interval_t l(x._lb, z_bound(-1));
      interval_t u(z_bound(1), x._ub);
      return (this->operator/(l) | this->operator/(u));
    } else if (this->operator[](0)) {
      interval_t l(this->_lb, z_bound(-1));
      interval_t u(z_bound(1), this->_ub);
      return ((l / x) | (u / x) | interval_t(z_number(0)));
    } else {
      // Neither the dividend nor the divisor contains 0
      interval_t a =
          (this->_ub < 0)
              ? (*this + ((x._ub < 0) ? (x + interval_t(z_number(1)))
                                      : (interval_t(z_number(1)) - x)))
              : *this;
      bound_t ll = a._lb / x._lb;
      bound_t lu = a._lb / x._ub;
      bound_t ul = a._ub / x._lb;
      bound_t uu = a._ub / x._ub;
      return interval_t(bound_t::min(ll, lu, ul, uu),
                        bound_t::max(ll, lu, ul, uu));
    }
  }
}

template <>
inline interval< z_number > interval< z_number >::SRem(
    interval< z_number > x) const {
  if (this->is_bottom() || x.is_bottom()) {
    return this->bottom();
  } else {
    boost::optional< z_number > n = this->singleton();
    boost::optional< z_number > d = x.singleton();
    if (d && *d == 0) {
      // [_, _] / 0 = _|_
      return this->bottom();
    } else if (n && d) {
      return interval_t(*n % *d);
    } else {
      // Note that the sign of the divisor does not matter
      z_bound n_ub = z_bound::max(this->_lb.abs(), this->_ub.abs());
      z_bound d_ub = z_bound::max(x._lb.abs(), x._ub.abs());
      z_bound ub = z_bound::min(n_ub, d_ub - 1);
      if (this->_lb < 0) {
        if (this->_ub > 0) {
          return interval_t(-ub, ub);
        } else {
          return interval_t(-ub, 0);
        }
      } else {
        return interval_t(0, ub);
      }
    }
  }
}

template <>
inline interval< z_number > interval< z_number >::URem(
    interval< z_number > x) const {
  if (this->is_bottom() || x.is_bottom()) {
    return this->bottom();
  } else {
    boost::optional< z_number > n = this->singleton();
    boost::optional< z_number > d = x.singleton();
    if (d && *d == 0) {
      // [_, _] / 0 = _|_
      return this->bottom();
    } else if (this->_lb < 0 && x._lb < 0) {
      return this->top();
    } else if (this->_lb < 0) {
      return interval_t(0, x._ub - 1);
    } else if (x._lb < 0) {
      return interval_t(0, this->_ub);
    } else if (n && d) {
      return interval_t(*n % *d);
    } else {
      return interval_t(0, z_bound::min(this->_ub, x._ub - 1));
    }
  }
}

template <>
inline interval< z_number > interval< z_number >::Trunc(uint64_t from,
                                                        uint64_t to) const {
  assert(from > to && "invalid trunc");
  if (this->is_bottom()) {
    return this->bottom();
  } else {
    boost::optional< z_number > n = this->singleton();
    z_number unsigned_max = (z_number(1) << to) - 1;
    z_number signed_min = -(z_number(1) << (to - 1));
    if (n) {
      return interval_t(*n & unsigned_max);
    } else if (this->_lb >= signed_min && this->_ub <= unsigned_max) {
      return *this;
    } else {
      return interval_t(signed_min, unsigned_max);
    }
  }
}

template <>
inline interval< z_number > interval< z_number >::ZExt(uint64_t from,
                                                       uint64_t to) const {
  assert(from < to && "invalid zext");
  if (this->is_bottom()) {
    return this->bottom();
  } else {
    z_number unsigned_max = (z_number(1) << from) - 1;
    z_number signed_min = -(z_number(1) << (from - 1));
    if (this->_lb >= 0 && this->_ub <= unsigned_max) { // positive
      return *this;
    } else if (this->_lb >= signed_min && this->_ub < 0) { // negative
      return interval_t(*this->_lb.number() + (z_number(1) << from),
                        *this->_ub.number() + (z_number(1) << from));
    } else {
      return interval_t(0, unsigned_max);
    }
  }
}

template <>
inline interval< z_number > interval< z_number >::SExt(uint64_t from,
                                                       uint64_t to) const {
  assert(from < to && "invalid sext");
  if (this->is_bottom()) {
    return this->bottom();
  } else {
    z_number unsigned_max = (z_number(1) << from) - 1;
    z_number signed_min = -(z_number(1) << (from - 1));
    if (this->_lb >= signed_min && this->_ub <= unsigned_max) {
      return *this;
    } else {
      return interval_t(signed_min, unsigned_max);
    }
  }
}

template <>
inline interval< z_number > interval< z_number >::And(
    interval< z_number > x) const {
  if (this->is_bottom() || x.is_bottom()) {
    return this->bottom();
  } else {
    boost::optional< z_number > l = this->singleton();
    boost::optional< z_number > r = x.singleton();
    if (l && r) {
      return interval_t(*l & *r);
    } else if ((l && *l == 0) || (r && *r == 0)) {
      return interval_t(z_number(0));
    } else if (l && *l == -1) {
      return x;
    } else if (r && *r == -1) {
      return *this;
    } else {
      if (this->_lb >= 0 && x._lb >= 0) {
        return interval_t(0, z_bound::min(this->_ub, x._ub));
      } else if (this->_lb >= 0) {
        return interval_t(0, this->_ub);
      } else if (x._lb >= 0) {
        return interval_t(0, x._ub);
      } else {
        return this->top();
      }
    }
  }
}

template <>
inline interval< z_number > interval< z_number >::Or(
    interval< z_number > x) const {
  if (this->is_bottom() || x.is_bottom()) {
    return this->bottom();
  } else {
    boost::optional< z_number > l = this->singleton();
    boost::optional< z_number > r = x.singleton();
    if (l && r) {
      return interval_t(*l | *r);
    } else if ((l && *l == -1) || (r && *r == -1)) {
      return interval_t(z_number(-1));
    } else if (l && *l == 0) {
      return x;
    } else if (r && *r == 0) {
      return *this;
    } else if (this->_lb >= 0 && x._lb >= 0) {
      boost::optional< z_number > l_ub = this->_ub.number();
      boost::optional< z_number > r_ub = x._ub.number();
      if (l_ub && r_ub) {
        return interval_t(0, std::max(*l_ub, *r_ub).fill_ones());
      } else {
        return interval_t(0, bound_t::plus_infinity());
      }
    } else {
      return this->top();
    }
  }
}

template <>
inline interval< z_number > interval< z_number >::Xor(
    interval< z_number > x) const {
  if (this->is_bottom() || x.is_bottom()) {
    return this->bottom();
  } else {
    boost::optional< z_number > l = this->singleton();
    boost::optional< z_number > r = x.singleton();
    if (l && r) {
      return interval_t(*l ^ *r);
    } else if (l && *l == 0) {
      return x;
    } else if (r && *r == 0) {
      return *this;
    } else if (this->_lb >= 0 && x._lb >= 0) {
      boost::optional< z_number > l_ub = this->_ub.number();
      boost::optional< z_number > r_ub = x._ub.number();
      if (l_ub && r_ub) {
        return interval_t(0, std::max(*l_ub, *r_ub).fill_ones());
      } else {
        return interval_t(0, bound_t::plus_infinity());
      }
    } else {
      return this->top();
    }
  }
}

template <>
inline interval< z_number > interval< z_number >::Shl(
    interval< z_number > x) const {
  if (this->is_bottom() || x.is_bottom()) {
    return this->bottom();
  } else {
    boost::optional< z_number > shift = x.singleton();
    if (shift) {
      z_number factor(1);
      for (int i = 0; (*shift) > i; i++) {
        factor <<= 1;
      }
      return (*this) * factor;
    } else {
      return this->top();
    }
  }
}

template <>
inline interval< z_number > interval< z_number >::LShr(
    interval< z_number > x) const {
  if (this->is_bottom() || x.is_bottom()) {
    return this->bottom();
  } else {
    boost::optional< z_number > shift = x.singleton();
    if (this->_lb >= 0 && this->_ub.is_finite() && shift) {
      z_number lb = *(this->_lb.number());
      z_number ub = *(this->_ub.number());
      return interval< z_number >(lb >> *shift, ub >> *shift);
    } else {
      return this->top();
    }
  }
}

template < typename Number >
inline interval< Number > operator+(Number c, interval< Number > x) {
  return interval< Number >(c) + x;
}

template < typename Number >
inline interval< Number > operator+(interval< Number > x, Number c) {
  return x + interval< Number >(c);
}

template < typename Number >
inline interval< Number > operator*(Number c, interval< Number > x) {
  return interval< Number >(c) * x;
}

template < typename Number >
inline interval< Number > operator*(interval< Number > x, Number c) {
  return x * interval< Number >(c);
}

template < typename Number >
inline interval< Number > operator/(Number c, interval< Number > x) {
  return interval< Number >(c) / x;
}

template < typename Number >
inline interval< Number > operator/(interval< Number > x, Number c) {
  return x / interval< Number >(c);
}

template < typename Number >
inline interval< Number > operator-(Number c, interval< Number > x) {
  return interval< Number >(c) - x;
}

template < typename Number >
inline interval< Number > operator-(interval< Number > x, Number c) {
  return x - interval< Number >(c);
}

template < typename Number >
inline std::ostream& operator<<(std::ostream& o, interval< Number > i) {
  i.write(o);
  return o;
}

/// \returns the constraint system: i.lower_bound <= e <= i.upper_bound
template < typename Number, typename VariableName >
inline linear_constraint_system< Number, VariableName > within_interval(
    linear_expression< Number, VariableName > e, interval< Number > i) {
  linear_constraint_system< Number, VariableName > csts;

  if (i.is_bottom()) {
    csts += linear_constraint< Number, VariableName >::contradiction();
  } else {
    boost::optional< Number > lb = i.lb().number();
    boost::optional< Number > ub = i.ub().number();

    if (lb && ub && *lb == *ub) {
      csts += (e == *ub);
    } else {
      if (lb) {
        csts += (e >= *lb);
      }
      if (ub) {
        csts += (e <= *ub);
      }
    }
  }

  return csts;
}

/// \returns the constraint system: i.lower_bound <= v <= i.upper_bound
template < typename Number, typename VariableName >
inline linear_constraint_system< Number, VariableName > within_interval(
    variable< Number, VariableName > v, interval< Number > i) {
  return within_interval(linear_expression< Number, VariableName >(v), i);
}

/// \returns the constraint system: i.lower_bound <= v <= i.upper_bound
template < typename Number, typename VariableName >
inline linear_constraint_system< Number, VariableName > within_interval(
    VariableName v, interval< Number > i) {
  return within_interval(variable< Number, VariableName >(v), i);
}

typedef interval< z_number > z_interval;
typedef interval< q_number > q_interval;

} // end namespace ikos

#endif // IKOS_VALUE_INTERVAL_HPP
