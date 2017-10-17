/**************************************************************************/ /**
 *
 * \file
 * \brief Unlimited precision integers
 *
 * Implementation of big numbers based on GMP, the GNU Multiple Precision
 * Arithmetic Library (http://gmplib.org).
 *
 * Author: Maxime Arthaud
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

#ifndef IKOS_NUMBER_Z_NUMBER_HPP
#define IKOS_NUMBER_Z_NUMBER_HPP

#include <functional>
#include <iosfwd>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include <gmpxx.h>

#include <boost/functional/hash.hpp>

#include <ikos/assert.hpp>
#include <ikos/number/exception.hpp>
#include <ikos/number/support.hpp>

namespace ikos {

namespace detail {

/// \brief Helper to handle all supported integral types with GMP
template < typename T >
struct mpz_adapter {
  T operator()(T n) { return n; }
};

/// \brief Helper to handle unsigned long long
template < bool >
struct mpz_adapter_ull;

template <>
struct mpz_adapter_ull< true > {
  unsigned long operator()(unsigned long long n) {
    return static_cast< unsigned long >(n);
  }
};

template <>
struct mpz_adapter_ull< false > {
  mpz_class operator()(unsigned long long n) {
    static_assert(sizeof(unsigned long long) == 8, "unexpected size");
    static_assert(sizeof(unsigned int) == 4, "unexpected size");
    mpz_class r(static_cast< unsigned int >(n >> 32));
    r <<= 32;
    r += static_cast< unsigned int >(n);
    return r;
  }
};

template <>
struct mpz_adapter< unsigned long long >
    : public mpz_adapter_ull< sizeof(unsigned long long) ==
                              sizeof(unsigned long) > {};

/// \brief Helper to handle long long
template < bool >
struct mpz_adapter_ll;

template <>
struct mpz_adapter_ll< true > {
  long operator()(long long n) { return static_cast< long >(n); }
};

template <>
struct mpz_adapter_ll< false > {
  mpz_class operator()(long long n) {
    static_assert(sizeof(long long) == 8, "unexpected size");
    static_assert(sizeof(int) == 4, "unexpected size");
    mpz_class r(static_cast< int >(n >> 32));
    r <<= 32;
    r += static_cast< unsigned int >(n);
    return r;
  }
};

template <>
struct mpz_adapter< long long >
    : public mpz_adapter_ll< sizeof(long long) == sizeof(long) > {};

} // end namespace detail

/// \brief Class for unlimited precision integers
class z_number {
private:
  mpz_class _n;

public:
  /// \brief Create a z_number from a string representation
  ///
  /// Interpret the null-terminated string str in the given base.
  ///
  /// If the string contains unsuitable characters for the given base, throw
  /// an exception number_error.
  ///
  /// The base may vary from 2 to 36.
  static z_number from_string(const char* str, int base = 10) {
    try {
      return z_number(mpz_class(str, base));
    } catch (std::invalid_argument&) {
      std::ostringstream buf;
      buf << "z_number: invalid conversion from string '" << str << "'";
      throw number_error(buf.str());
    }
  }

  /// \brief Create a z_number from a string representation
  ///
  /// Interpret the string str in the given base.
  ///
  /// If the string contains unsuitable characters for the given base, throw
  /// an exception number_error.
  ///
  /// The base may vary from 2 to 36.
  static z_number from_string(const std::string& str, int base = 10) {
    try {
      return z_number(mpz_class(str, base));
    } catch (std::invalid_argument&) {
      std::ostringstream buf;
      buf << "z_number: invalid conversion from string '" << str << "'";
      throw number_error(buf.str());
    }
  }

  /// \name Constructors
  /// @{

  /// \brief Default constructor that creates a z_number equals to 0
  z_number() = default;

  /// \brief Copy constructor
  z_number(const z_number&) = default;

  /// \brief Move constructor
  z_number(z_number&&) = default;

  /// \brief Create a z_number from a mpz_class
  explicit z_number(const mpz_class& n) : _n(n) {}

  /// \brief Create a z_number from a mpz_class
  explicit z_number(mpz_class&& n) : _n(std::move(n)) {}

  /// \brief Create a z_number from an integral type
  template < typename T,
             class = std::enable_if_t< is_supported_integral< T >::value > >
  explicit z_number(T n)
      : _n(detail::mpz_adapter< T >()(n)) {}

  /// @}
  /// \name Assignment Operators
  /// @{

  /// \brief Copy assignment
  z_number& operator=(const z_number&) = default;

  /// \brief Move assignment
  z_number& operator=(z_number&&) noexcept = default;

  /// \brief Assignment for integral types
  template < typename T,
             class = std::enable_if_t< is_supported_integral< T >::value > >
  z_number& operator=(T n) {
    this->_n = detail::mpz_adapter< T >()(n);
    return *this;
  }

  /// \brief Addition assignment
  z_number& operator+=(const z_number& x) {
    this->_n += x._n;
    return *this;
  }

  /// \brief Addition assignment with integral types
  template < typename T,
             class = std::enable_if_t< is_supported_integral< T >::value > >
  z_number& operator+=(T x) {
    this->_n += detail::mpz_adapter< T >()(x);
    return *this;
  }

  /// \brief Subtraction assignment
  z_number& operator-=(const z_number& x) {
    this->_n -= x._n;
    return *this;
  }

  /// \brief Subtraction assignment with integral types
  template < typename T,
             class = std::enable_if_t< is_supported_integral< T >::value > >
  z_number& operator-=(T x) {
    this->_n -= detail::mpz_adapter< T >()(x);
    return *this;
  }

  /// \brief Multiplication assignment
  z_number& operator*=(const z_number& x) {
    this->_n *= x._n;
    return *this;
  }

  /// \brief Multiplication assignment with integral types
  template < typename T,
             class = std::enable_if_t< is_supported_integral< T >::value > >
  z_number& operator*=(T x) {
    this->_n *= detail::mpz_adapter< T >()(x);
    return *this;
  }

  /// \brief Integer division assignment
  z_number& operator/=(const z_number& x) {
    ikos_assert_msg(x._n != 0, "division by zero");
    this->_n /= x._n;
    return *this;
  }

  /// \brief Integer division assignment with integral types
  template < typename T,
             class = std::enable_if_t< is_supported_integral< T >::value > >
  z_number& operator/=(T x) {
    ikos_assert_msg(x != 0, "division by zero");
    this->_n /= detail::mpz_adapter< T >()(x);
    return *this;
  }

  /// \brief Remainder assignment
  ///
  /// The result r will satisfy: this = q * abs(x) + r with 0 <= abs(r) < abs(x)
  ///
  /// The sign of x is ignored, and the result will have the same sign as this.
  z_number& operator%=(const z_number& x) {
    ikos_assert_msg(x._n != 0, "division by zero");
    this->_n %= x._n;
    return *this;
  }

  /// \brief Remainder assignment with integral types
  ///
  /// The result r will satisfy: this = q * abs(x) + r with 0 <= abs(r) < abs(x)
  ///
  /// The sign of x is ignored, and the result will have the same sign as this.
  template < typename T,
             class = std::enable_if_t< is_supported_integral< T >::value > >
  z_number& operator%=(T x) {
    ikos_assert_msg(x != 0, "division by zero");
    this->_n %= detail::mpz_adapter< T >()(x);
    return *this;
  }

  /// \brief Bitwise AND assignment
  z_number& operator&=(const z_number& x) {
    this->_n &= x._n;
    return *this;
  }

  /// \brief Bitwise AND assignment with integral types
  template < typename T,
             class = std::enable_if_t< is_supported_integral< T >::value > >
  z_number& operator&=(T x) {
    this->_n &= detail::mpz_adapter< T >()(x);
    return *this;
  }

  /// \brief Bitwise OR assignment
  z_number& operator|=(const z_number& x) {
    this->_n |= x._n;
    return *this;
  }

  /// \brief Bitwise OR assignment with integral types
  template < typename T,
             class = std::enable_if_t< is_supported_integral< T >::value > >
  z_number& operator|=(T x) {
    this->_n |= detail::mpz_adapter< T >()(x);
    return *this;
  }

  /// \brief Bitwise XOR assignment
  z_number& operator^=(const z_number& x) {
    this->_n ^= x._n;
    return *this;
  }

  /// \brief Bitwise XOR assignment with integral types
  template < typename T,
             class = std::enable_if_t< is_supported_integral< T >::value > >
  z_number& operator^=(T x) {
    this->_n ^= detail::mpz_adapter< T >()(x);
    return *this;
  }

  /// \brief Left binary shift assignment
  ///
  /// This is undefined if x isn't between 0 and 2**32 - 1
  z_number& operator<<=(const z_number& x) {
    ikos_assert_msg(x._n >= 0, "shift count is negative");
    ikos_assert_msg(x._n.fits_ulong_p(), "shift count is too big");
    this->_n <<= x._n.get_ui();
    return *this;
  }

  /// \brief Left binary shift assignment with integral types
  ///
  /// This is undefined if x isn't between 0 and 2**32 - 1
  template < typename T,
             class = std::enable_if_t< is_supported_integral< T >::value > >
  z_number& operator<<=(T x) {
    ikos_assert_msg(x >= 0, "shift count is negative");
    this->_n <<= static_cast< unsigned long int >(x);
    return *this;
  }

  /// \brief Right binary shift
  ///
  /// This is undefined if x isn't between 0 and 2**32 - 1
  z_number& operator>>=(const z_number& x) {
    ikos_assert_msg(x._n >= 0, "shift count is negative");
    ikos_assert_msg(x._n.fits_ulong_p(), "shift count is too big");
    this->_n >>= x._n.get_ui();
    return *this;
  }

  /// \brief Right binary shift with integral types
  ///
  /// This is undefined if x isn't between 0 and 2**32 - 1
  template < typename T,
             class = std::enable_if_t< is_supported_integral< T >::value > >
  z_number& operator>>=(T x) {
    ikos_assert_msg(x >= 0, "shift count is negative");
    this->_n >>= static_cast< unsigned long int >(x);
    return *this;
  }

  /// @}
  /// \name Unary Operators
  /// @{

  /// \brief Unary plus
  z_number operator+() const { return *this; }

  /// \brief Prefix increment
  z_number& operator++() {
    ++this->_n;
    return *this;
  }

  /// \brief Postfix increment
  z_number operator++(int) {
    z_number r(*this);
    ++this->_n;
    return r;
  }

  /// \brief Unary minus
  z_number operator-() const { return z_number(-this->_n); }

  /// \brief Prefix decrement
  z_number& operator--() {
    --this->_n;
    return *this;
  }

  /// \brief Postfix decrement
  z_number operator--(int) {
    z_number r(*this);
    --this->_n;
    return r;
  }

  /// @}
  /// \name Value generators
  /// @{

  /// \brief Fill the number with one bits
  ///
  /// \returns the smallest n such that n = 2^m - 1 >= this
  z_number fill_ones() const {
    ikos_assert(this->_n >= 0);

    if (this->_n == 0) {
      return z_number(0);
    }

    mpz_class result(1);
    while (result < this->_n) {
      result <<= 1;
      result += 1;
    }
    return z_number(result);
  }

  /// @}
  /// \name Conversion Functions
  /// @{

  /// \brief Get the internal representation
  const mpz_class& mpz() const { return this->_n; }

  /// \brief Return a string of the z_number in the given base
  ///
  /// The base can vary from 2 to 36, or from -2 to -36
  std::string str(int base = 10) const { return this->_n.get_str(base); }

  /// @}

  friend std::istream& operator>>(std::istream& i, z_number& n);

  friend class q_number;

}; // end class z_number

/// \name Binary Operators
/// @{

/// \brief Addition
inline z_number operator+(const z_number& lhs, const z_number& rhs) {
  return z_number(lhs.mpz() + rhs.mpz());
}

/// \brief Addition with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator+(const z_number& lhs, T rhs) {
  return z_number(lhs.mpz() + detail::mpz_adapter< T >()(rhs));
}

/// \brief Addition with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator+(T lhs, const z_number& rhs) {
  return z_number(detail::mpz_adapter< T >()(lhs) + rhs.mpz());
}

/// \brief Subtraction
inline z_number operator-(const z_number& lhs, const z_number& rhs) {
  return z_number(lhs.mpz() - rhs.mpz());
}

/// \brief Subtraction with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator-(const z_number& lhs, T rhs) {
  return z_number(lhs.mpz() - detail::mpz_adapter< T >()(rhs));
}

/// \brief Subtraction with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator-(T lhs, const z_number& rhs) {
  return z_number(detail::mpz_adapter< T >()(lhs) - rhs.mpz());
}

/// \brief Multiplication
inline z_number operator*(const z_number& lhs, const z_number& rhs) {
  return z_number(lhs.mpz() * rhs.mpz());
}

/// \brief Multiplication with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator*(const z_number& lhs, T rhs) {
  return z_number(lhs.mpz() * detail::mpz_adapter< T >()(rhs));
}

/// \brief Multiplication with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator*(T lhs, const z_number& rhs) {
  return z_number(detail::mpz_adapter< T >()(lhs) * rhs.mpz());
}

/// \brief Integer division
inline z_number operator/(const z_number& lhs, const z_number& rhs) {
  ikos_assert_msg(rhs.mpz() != 0, "division by zero");
  return z_number(lhs.mpz() / rhs.mpz());
}

/// \brief Integer division with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator/(const z_number& lhs, T rhs) {
  ikos_assert_msg(rhs != 0, "division by zero");
  return z_number(lhs.mpz() / detail::mpz_adapter< T >()(rhs));
}

/// \brief Integer division with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator/(T lhs, const z_number& rhs) {
  ikos_assert_msg(rhs.mpz() != 0, "division by zero");
  return z_number(detail::mpz_adapter< T >()(lhs) / rhs.mpz());
}

/// \brief Remainder
inline z_number operator%(const z_number& lhs, const z_number& rhs) {
  ikos_assert_msg(rhs.mpz() != 0, "division by zero");
  return z_number(lhs.mpz() % rhs.mpz());
}

/// \brief Remainder with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator%(const z_number& lhs, T rhs) {
  ikos_assert_msg(rhs != 0, "division by zero");
  return z_number(lhs.mpz() % detail::mpz_adapter< T >()(rhs));
}

/// \brief Remainder with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator%(T lhs, const z_number& rhs) {
  ikos_assert_msg(rhs.mpz() != 0, "division by zero");
  return z_number(detail::mpz_adapter< T >()(lhs) % rhs.mpz());
}

/// \brief Bitwise AND
inline z_number operator&(const z_number& lhs, const z_number& rhs) {
  return z_number(lhs.mpz() & rhs.mpz());
}

/// \brief Bitwise AND with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator&(const z_number& lhs, T rhs) {
  return z_number(lhs.mpz() & detail::mpz_adapter< T >()(rhs));
}

/// \brief Bitwise AND with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator&(T lhs, const z_number& rhs) {
  return z_number(detail::mpz_adapter< T >()(lhs) & rhs.mpz());
}

/// \brief Bitwise OR
inline z_number operator|(const z_number& lhs, const z_number& rhs) {
  return z_number(lhs.mpz() | rhs.mpz());
}

/// \brief Bitwise OR with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator|(const z_number& lhs, T rhs) {
  return z_number(lhs.mpz() | detail::mpz_adapter< T >()(rhs));
}

/// \brief Bitwise OR with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator|(T lhs, const z_number& rhs) {
  return z_number(detail::mpz_adapter< T >()(lhs) | rhs.mpz());
}

/// \brief Bitwise XOR
inline z_number operator^(const z_number& lhs, const z_number& rhs) {
  return z_number(lhs.mpz() ^ rhs.mpz());
}

/// \brief Bitwise XOR with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator^(const z_number& lhs, T rhs) {
  return z_number(lhs.mpz() ^ detail::mpz_adapter< T >()(rhs));
}

/// \brief Bitwise XOR with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator^(T lhs, const z_number& rhs) {
  return z_number(detail::mpz_adapter< T >()(lhs) ^ rhs.mpz());
}

/// \brief Left binary shift
inline z_number
operator<<(const z_number& lhs, const z_number& rhs) {
  ikos_assert_msg(rhs.mpz() >= 0, "shift count is negative");
  ikos_assert_msg(rhs.mpz().fits_ulong_p(), "shift count is too big");
  return z_number(lhs.mpz() << rhs.mpz().get_ui());
}

/// \brief Left binary shift with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator<<(const z_number& lhs, T rhs) {
  ikos_assert_msg(rhs >= 0, "shift count is negative");
  return z_number(lhs.mpz() << static_cast< unsigned long int >(rhs));
}

/// \brief Left binary shift with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator<<(T lhs, const z_number& rhs) {
  z_number r(lhs);
  r <<= rhs;
  return r;
}

/// \brief Right binary shift
inline z_number operator>>(const z_number& lhs, const z_number& rhs) {
  ikos_assert_msg(rhs.mpz() >= 0, "shift count is negative");
  ikos_assert_msg(rhs.mpz().fits_ulong_p(), "shift count is too big");
  return z_number(lhs.mpz() >> rhs.mpz().get_ui());
}

/// \brief Right binary shift with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator>>(const z_number& lhs, T rhs) {
  ikos_assert_msg(rhs >= 0, "shift count is negative");
  return z_number(lhs.mpz() >> static_cast< unsigned long int >(rhs));
}

/// \brief Right binary shift with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline z_number operator>>(T lhs, const z_number& rhs) {
  z_number r(lhs);
  r >>= rhs;
  return r;
}

/// @}
/// \name Comparison Operators
/// @{

/// \brief Equality operator
inline bool operator==(const z_number& lhs, const z_number& rhs) {
  return lhs.mpz() == rhs.mpz();
}

/// \brief Equality operator with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline bool operator==(const z_number& lhs, T rhs) {
  return lhs.mpz() == detail::mpz_adapter< T >()(rhs);
}

/// \brief Equality operator with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline bool operator==(T lhs, const z_number& rhs) {
  return detail::mpz_adapter< T >()(lhs) == rhs.mpz();
}

/// \brief Inequality operator
inline bool operator!=(const z_number& lhs, const z_number& rhs) {
  return lhs.mpz() != rhs.mpz();
}

/// \brief Inequality operator with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline bool operator!=(const z_number& lhs, T rhs) {
  return lhs.mpz() != detail::mpz_adapter< T >()(rhs);
}

/// \brief Inequality operator with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline bool operator!=(T lhs, const z_number& rhs) {
  return detail::mpz_adapter< T >()(lhs) != rhs.mpz();
}

/// \brief Less than comparison
inline bool operator<(const z_number& lhs, const z_number& rhs) {
  return lhs.mpz() < rhs.mpz();
}

/// \brief Less than comparison with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline bool operator<(const z_number& lhs, T rhs) {
  return lhs.mpz() < detail::mpz_adapter< T >()(rhs);
}

/// \brief Less than comparison with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline bool operator<(T lhs, const z_number& rhs) {
  return detail::mpz_adapter< T >()(lhs) < rhs.mpz();
}

/// \brief Less or equal comparison
inline bool operator<=(const z_number& lhs, const z_number& rhs) {
  return lhs.mpz() <= rhs.mpz();
}

/// \brief Less or equal comparison with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline bool operator<=(const z_number& lhs, T rhs) {
  return lhs.mpz() <= detail::mpz_adapter< T >()(rhs);
}

/// \brief Less or equal comparison with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline bool operator<=(T lhs, const z_number& rhs) {
  return detail::mpz_adapter< T >()(lhs) <= rhs.mpz();
}

/// \brief Greater than comparison
inline bool operator>(const z_number& lhs, const z_number& rhs) {
  return lhs.mpz() > rhs.mpz();
}

/// \brief Greater than comparison with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline bool operator>(const z_number& lhs, T rhs) {
  return lhs.mpz() > detail::mpz_adapter< T >()(rhs);
}

/// \brief Greater than comparison with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline bool operator>(T lhs, const z_number& rhs) {
  return detail::mpz_adapter< T >()(lhs) > rhs.mpz();
}

/// \brief Greater or equal comparison
inline bool operator>=(const z_number& lhs, const z_number& rhs) {
  return lhs.mpz() >= rhs.mpz();
}

/// \brief Greater or equal comparison with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline bool operator>=(const z_number& lhs, T rhs) {
  return lhs.mpz() >= detail::mpz_adapter< T >()(rhs);
}

/// \brief Greater or equal comparison with integral types
template < typename T,
           class = std::enable_if_t< is_supported_integral< T >::value > >
inline bool operator>=(T lhs, const z_number& rhs) {
  return detail::mpz_adapter< T >()(lhs) >= rhs.mpz();
}

/// @}
/// \name Input / Output
/// @{

/// \brief Write a z_number on a stream, in base 10
inline std::ostream& operator<<(std::ostream& o, const z_number& n) {
  o << n.mpz();
  return o;
}

/// \brief Read a z_number from a stream, in base 10
inline std::istream& operator>>(std::istream& i, z_number& n) {
  i >> n._n;
  return i;
}

/// @}

} // end namespace ikos

namespace std {

/// \brief Hash for z_number
template <>
struct hash< ikos::z_number > {
  std::size_t operator()(const ikos::z_number& n) const {
    const mpz_class& m = n.mpz();
    std::size_t result = 0;
    boost::hash_combine(result, m.get_mpz_t()[0]._mp_size);
    for (int i = 0; i < std::abs(m.get_mpz_t()[0]._mp_size); ++i) {
      boost::hash_combine(result, m.get_mpz_t()[0]._mp_d[i]);
    }
    return result;
  }
};

} // end namespace std

#endif // IKOS_NUMBER_Z_NUMBER_HPP
