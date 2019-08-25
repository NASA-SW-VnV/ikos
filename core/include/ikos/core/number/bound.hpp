/*******************************************************************************
 *
 * \file
 * \brief Bound class (+oo, -oo or a number)
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

#include <ikos/core/exception.hpp>
#include <ikos/core/number/q_number.hpp>
#include <ikos/core/number/z_number.hpp>
#include <ikos/core/support/assert.hpp>

namespace ikos {
namespace core {

/// \brief Bound
///
/// This is either -oo, +oo or a number.
template < typename Number >
class Bound {
private:
  bool _is_infinite;
  Number _n;

  // Invariant: _is_infinite => (_n == 1 or _n == -1)

private:
  /// \brief Private constructor
  Bound(bool is_infinite, int n) : _is_infinite(is_infinite), _n(n) {
    this->normalize();
  }

  /// \brief Private constructor
  Bound(bool is_infinite, Number n)
      : _is_infinite(is_infinite), _n(std::move(n)) {
    this->normalize();
  }

  /// \brief Normalize the bound
  void normalize() {
    if (this->_is_infinite) {
      this->_n = (this->_n >= 0) ? 1 : -1;
    }
  }

public:
  /// \brief No default constructor
  Bound() = delete;

  /// \brief Create a bound
  explicit Bound(int n) : _is_infinite(false), _n(n) {}

  /// \brief Create a bound
  explicit Bound(Number n) : _is_infinite(false), _n(std::move(n)) {}

  /// \brief Copy constructor
  Bound(const Bound&) noexcept(
      std::is_nothrow_copy_constructible< Number >::value) = default;

  /// \brief Move constructor
  Bound(Bound&&) noexcept(std::is_nothrow_move_constructible< Number >::value) =
      default;

  /// \brief Assign a number
  Bound& operator=(int n) {
    this->_is_infinite = false;
    this->_n = n;
    return *this;
  }

  /// \brief Assign a number
  Bound& operator=(Number n) {
    this->_is_infinite = false;
    this->_n = std::move(n);
    return *this;
  }

  /// \brief Copy assignment operator
  Bound& operator=(const Bound&) noexcept(
      std::is_nothrow_copy_assignable< Number >::value) = default;

  /// \brief Move assignment operator
  Bound& operator=(Bound&&) noexcept(
      std::is_nothrow_move_assignable< Number >::value) = default;

  /// \brief Destructor
  ~Bound() = default;

public:
  /// \brief Return +oo
  static Bound plus_infinity() { return Bound(true, 1); }

  /// \brief Return -oo
  static Bound minus_infinity() { return Bound(true, -1); }

public:
  /// \brief Return true if the bound is infinite
  bool is_infinite() const { return this->_is_infinite; }

  /// \brief Return true if the bound is finite
  bool is_finite() const { return !this->_is_infinite; }

  /// \brief Return true if the bound is plus infinity
  bool is_plus_infinity() const { return this->is_infinite() && this->_n == 1; }

  /// \brief Return true if the bound is minus infinity
  bool is_minus_infinity() const {
    return this->is_infinite() && this->_n == -1;
  }

  /// \brief Return ture if the bound is zero
  bool is_zero() const { return this->_n == 0; }

  /// \brief Unary minus
  Bound operator-() const { return Bound(this->_is_infinite, -this->_n); }

  /// \brief Add a bound
  void operator+=(const Bound& other) {
    if (this->is_finite() && other.is_finite()) {
      this->_n += other._n;
    } else if (this->is_finite() && other.is_infinite()) {
      this->operator=(other);
    } else if (this->is_infinite() && other.is_finite()) {
      return;
    } else if (this->_n == other._n) {
      return;
    } else {
      ikos_unreachable("undefined operation +oo + -oo");
    }
  }

  /// \brief Substract a bound
  void operator-=(const Bound& other) {
    if (this->is_finite() && other.is_finite()) {
      this->_n -= other._n;
    } else if (this->is_finite() && other.is_infinite()) {
      this->operator=(Bound(true, -other._n));
    } else if (this->is_infinite() && other.is_finite()) {
      return;
    } else if (this->_n != other._n) {
      return;
    } else {
      ikos_unreachable("undefined operation +oo - +oo");
    }
  }

  /// \brief Multiply by a bound
  void operator*=(const Bound& other) {
    if (this->is_zero()) {
      return;
    } else if (other.is_zero()) {
      this->operator=(other);
    } else {
      this->_n *= other._n;
      this->_is_infinite = (this->_is_infinite || other._is_infinite);
      this->normalize();
    }
  }

  /// \brief Lower or equal comparison
  ///
  /// This is an optimized implementation.
  bool leq(const Bound& other) const {
    if (this->_is_infinite xor other._is_infinite) {
      if (this->_is_infinite) {
        return this->_n == -1;
      } else {
        return other._n == 1;
      }
    }
    return this->_n <= other._n;
  }

  /// \brief Greater or equal comparison
  ///
  /// This is an optimized implementation.
  bool geq(const Bound& other) const {
    if (this->_is_infinite xor other._is_infinite) {
      if (this->_is_infinite) {
        return this->_n == 1;
      } else {
        return other._n == -1;
      }
    }
    return this->_n >= other._n;
  }

  /// \brief Equality comparison
  bool equals(const Bound& other) const {
    return this->_is_infinite == other._is_infinite && this->_n == other._n;
  }

  /// \brief Return the number, or boost::none if the bound is infinite
  boost::optional< Number > number() const {
    if (this->is_infinite()) {
      return boost::none;
    } else {
      return this->_n;
    }
  }

  /// \brief Write the bound on a stream
  void dump(std::ostream& o) const {
    if (this->is_plus_infinity()) {
      o << "+oo";
    } else if (this->is_minus_infinity()) {
      o << "-oo";
    } else {
      o << this->_n;
    }
  }

  // Friends

  template < typename T >
  friend Bound< T > operator+(const Bound< T >& lhs, const Bound< T >& rhs);

  template < typename T >
  friend Bound< T > operator-(const Bound< T >& lhs, const Bound< T >& rhs);

  template < typename T >
  friend Bound< T > operator*(const Bound< T >& lhs, const Bound< T >& rhs);

  template < typename T >
  friend Bound< T > operator/(const Bound< T >& lhs, const Bound< T >& rhs);

  friend Bound< ZNumber > operator<<(const Bound< ZNumber >& lhs,
                                     const Bound< ZNumber >& rhs);

  friend Bound< ZNumber > operator>>(const Bound< ZNumber >& lhs,
                                     const Bound< ZNumber >& rhs);

}; // end class Bound

/// \brief Add bounds
template < typename Number >
inline Bound< Number > operator+(const Bound< Number >& lhs,
                                 const Bound< Number >& rhs) {
  using BoundT = Bound< Number >;

  if (lhs.is_finite() && rhs.is_finite()) {
    return BoundT(false, lhs._n + rhs._n);
  } else if (lhs.is_finite() && rhs.is_infinite()) {
    return rhs;
  } else if (lhs.is_infinite() && rhs.is_finite()) {
    return lhs;
  } else if (lhs._n == rhs._n) {
    return lhs;
  } else {
    ikos_unreachable("undefined operation +oo + -oo");
  }
}

/// \brief Substract bounds
template < typename Number >
inline Bound< Number > operator-(const Bound< Number >& lhs,
                                 const Bound< Number >& rhs) {
  using BoundT = Bound< Number >;

  if (lhs.is_finite() && rhs.is_finite()) {
    return BoundT(false, lhs._n - rhs._n);
  } else if (lhs.is_finite() && rhs.is_infinite()) {
    return BoundT(true, -rhs._n);
  } else if (lhs.is_infinite() && rhs.is_finite()) {
    return lhs;
  } else if (lhs._n != rhs._n) {
    return lhs;
  } else {
    ikos_unreachable("undefined operation +oo - +oo");
  }
}

/// \brief Multiply bounds
template < typename Number >
inline Bound< Number > operator*(const Bound< Number >& lhs,
                                 const Bound< Number >& rhs) {
  using BoundT = Bound< Number >;

  if (lhs.is_zero() || rhs.is_zero()) {
    return BoundT(false, 0);
  } else {
    return BoundT(lhs._is_infinite || rhs._is_infinite, lhs._n * rhs._n);
  }
}

/// \brief Divide bounds
template < typename Number >
inline Bound< Number > operator/(const Bound< Number >& lhs,
                                 const Bound< Number >& rhs) {
  using BoundT = Bound< Number >;

  if (rhs.is_zero()) {
    ikos_unreachable("division by zero");
  } else if (lhs.is_finite() && rhs.is_finite()) {
    return BoundT(false, lhs._n / rhs._n);
  } else if (lhs.is_finite() && rhs.is_infinite()) {
    return BoundT(0);
  } else if (lhs.is_infinite() && rhs.is_finite()) {
    if (rhs._n >= 0) {
      return lhs;
    } else {
      return -lhs;
    }
  } else {
    return BoundT(true, lhs._n * rhs._n);
  }
}

/// \brief Left binary shift of bounds
inline Bound< ZNumber > operator<<(const Bound< ZNumber >& lhs,
                                   const Bound< ZNumber >& rhs) {
  using BoundT = Bound< ZNumber >;
  ikos_assert_msg(rhs.geq(BoundT(0)), "right hand side is negative");

  if (lhs.is_zero()) {
    return lhs;
  } else if (lhs.is_infinite()) {
    return lhs;
  } else if (rhs.is_infinite()) {
    return BoundT(true, lhs._n);
  } else {
    return BoundT(lhs._n << rhs._n);
  }
}

/// \brief Right binary shift of bounds
inline Bound< ZNumber > operator>>(const Bound< ZNumber >& lhs,
                                   const Bound< ZNumber >& rhs) {
  using BoundT = Bound< ZNumber >;
  ikos_assert_msg(rhs.geq(BoundT(0)), "right hand side is negative");

  if (lhs.is_zero()) {
    return lhs;
  } else if (lhs.is_infinite()) {
    return lhs;
  } else if (rhs.is_infinite()) {
    return BoundT((lhs._n >= 0) ? 0 : -1);
  } else {
    return BoundT(lhs._n >> rhs._n);
  }
}

template < typename Number >
inline bool operator<=(const Bound< Number >& lhs, const Bound< Number >& rhs) {
  return lhs.leq(rhs);
}

template < typename Number >
inline bool operator>=(const Bound< Number >& lhs, const Bound< Number >& rhs) {
  return lhs.geq(rhs);
}

template < typename Number >
inline bool operator<(const Bound< Number >& lhs, const Bound< Number >& rhs) {
  return !lhs.geq(rhs);
}

template < typename Number >
inline bool operator>(const Bound< Number >& lhs, const Bound< Number >& rhs) {
  return !lhs.leq(rhs);
}

template < typename Number >
inline bool operator==(const Bound< Number >& lhs, const Bound< Number >& rhs) {
  return lhs.equals(rhs);
}

template < typename Number >
inline bool operator!=(const Bound< Number >& lhs, const Bound< Number >& rhs) {
  return !lhs.equals(rhs);
}

/// \brief Return the smaller of the given bound
template < typename Number >
inline const Bound< Number >& min(const Bound< Number >& a,
                                  const Bound< Number >& b) {
  return (a < b) ? a : b;
}

/// \brief Return the smaller of the given bound
template < typename Number >
inline const Bound< Number >& min(const Bound< Number >& a,
                                  const Bound< Number >& b,
                                  const Bound< Number >& c) {
  return min(min(a, b), c);
}

/// \brief Return the smaller of the given bound
template < typename Number >
inline const Bound< Number >& min(const Bound< Number >& a,
                                  const Bound< Number >& b,
                                  const Bound< Number >& c,
                                  const Bound< Number >& d) {
  return min(min(min(a, b), c), d);
}

/// \brief Return the greater of the given bound
template < typename Number >
inline const Bound< Number >& max(const Bound< Number >& a,
                                  const Bound< Number >& b) {
  return (a < b) ? b : a;
}

/// \brief Return the greater of the given bound
template < typename Number >
inline const Bound< Number >& max(const Bound< Number >& a,
                                  const Bound< Number >& b,
                                  const Bound< Number >& c) {
  return max(max(a, b), c);
}

/// \brief Return the greater of the given bound
template < typename Number >
inline const Bound< Number >& max(const Bound< Number >& a,
                                  const Bound< Number >& b,
                                  const Bound< Number >& c,
                                  const Bound< Number >& d) {
  return max(max(max(a, b), c), d);
}

/// \brief Return the absolute value of the given bound
template < typename Number >
inline Bound< Number > abs(const Bound< Number >& b) {
  return (b >= Bound< Number >(0)) ? b : -b;
}

/// \brief Write a bound on a stream
template < typename Number >
inline std::ostream& operator<<(std::ostream& o, const Bound< Number >& bound) {
  bound.dump(o);
  return o;
}

/// \brief Bound on unlimited precision integers
using ZBound = Bound< ZNumber >;

/// \brief Bound on unlimited precision rationals
using QBound = Bound< QNumber >;

} // end namespace core
} // end namespace ikos
