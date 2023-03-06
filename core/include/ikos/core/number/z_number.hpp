/*******************************************************************************
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

#include <functional>
#include <iosfwd>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include <gmpxx.h>

#include <boost/functional/hash.hpp>

#include <ikos/core/number/exception.hpp>
#include <ikos/core/number/supported_integral.hpp>
#include <ikos/core/support/assert.hpp>

namespace ikos {
namespace core {

namespace detail {

/// \brief Helper to handle all supported integral types with GMP
template < typename T >
struct MpzAdapter {
  T operator()(T n) { return n; }
};

/// \brief Helper to handle unsigned long long
template < bool >
struct MpzAdapterUnsignedLongLong;

template <>
struct MpzAdapterUnsignedLongLong< true > {
  unsigned long operator()(unsigned long long n) {
    return static_cast< unsigned long >(n);
  }
};

template <>
struct MpzAdapterUnsignedLongLong< false > {
  mpz_class operator()(unsigned long long n) {
    static_assert(sizeof(unsigned long long) == 8, "unexpected size");
    static_assert(sizeof(unsigned int) == 4, "unexpected size");
    mpz_class r(static_cast< unsigned int >(n >> 32U));
    r <<= 32;
    r += static_cast< unsigned int >(n & 0xFFFFFFFFU);
    return r;
  }
};

template <>
struct MpzAdapter< unsigned long long >
    : public MpzAdapterUnsignedLongLong< sizeof(unsigned long long) ==
                                         sizeof(unsigned long) > {};

/// \brief Helper to handle long long
template < bool >
struct MpzAdapterLongLong;

template <>
struct MpzAdapterLongLong< true > {
  long operator()(long long n) { return static_cast< long >(n); }
};

template <>
struct MpzAdapterLongLong< false > {
  mpz_class operator()(long long n) {
    static_assert(sizeof(long long) == 8, "unexpected size");
    static_assert(sizeof(int) == 4, "unexpected size");
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    mpz_class r(static_cast< int >(n >> 32));
    r <<= 32;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    r += static_cast< unsigned int >(n & 0xFFFFFFFF);
    return r;
  }
};

template <>
struct MpzAdapter< long long >
    : public MpzAdapterLongLong< sizeof(long long) == sizeof(long) > {};

/// \brief Helper to check if a mpz_class fits in the given integer type
template < typename T >
struct MpzFits;

template <>
struct MpzFits< unsigned int > {
  bool operator()(const mpz_class& n) { return n.fits_uint_p(); }
};

template <>
struct MpzFits< int > {
  bool operator()(const mpz_class& n) { return n.fits_sint_p(); }
};

template <>
struct MpzFits< unsigned long > {
  bool operator()(const mpz_class& n) { return n.fits_ulong_p(); }
};

template <>
struct MpzFits< long > {
  bool operator()(const mpz_class& n) { return n.fits_slong_p(); }
};

/// \brief Helper to handle unsigned long long
template < bool >
struct MpzFitsUnsignedLongLong;

template <>
struct MpzFitsUnsignedLongLong< true > {
  bool operator()(const mpz_class& n) { return n.fits_ulong_p(); }
};

template <>
struct MpzFitsUnsignedLongLong< false > {
  bool operator()(const mpz_class& n) {
    static_assert(sizeof(unsigned long long) == 8, "unexpected size");
    static_assert(sizeof(unsigned int) == 4, "unexpected size");
    return mpz_sgn(n.get_mpz_t()) >= 0 &&
           mpz_sizeinbase(n.get_mpz_t(), 2) <= 64;
  }
};

template <>
struct MpzFits< unsigned long long >
    : public MpzFitsUnsignedLongLong< sizeof(unsigned long long) ==
                                      sizeof(unsigned long) > {};

/// \brief Helper to handle long long
template < bool >
struct MpzFitsLongLong;

template <>
struct MpzFitsLongLong< true > {
  bool operator()(const mpz_class& n) { return n.fits_slong_p(); }
};

template <>
struct MpzFitsLongLong< false > {
  bool operator()(const mpz_class& n) {
    static_assert(sizeof(long long) == 8, "unexpected size");
    static_assert(sizeof(int) == 4, "unexpected size");
    if (mpz_sgn(n.get_mpz_t()) >= 0) {
      return mpz_sizeinbase(n.get_mpz_t(), 2) <= 63;
    } else {
      mpz_class m(-n - 1);
      return mpz_sizeinbase(m.get_mpz_t(), 2) <= 63;
    }
  }
};

template <>
struct MpzFits< long long >
    : public MpzFitsLongLong< sizeof(long long) == sizeof(long) > {};

/// \brief Helper to convert a mpz_class to the given integer type
template < typename T >
struct MpzTo;

template <>
struct MpzTo< unsigned int > {
  unsigned int operator()(const mpz_class& n) {
    return static_cast< unsigned int >(n.get_ui());
  }
};

template <>
struct MpzTo< int > {
  int operator()(const mpz_class& n) { return static_cast< int >(n.get_si()); }
};

template <>
struct MpzTo< unsigned long > {
  unsigned long operator()(const mpz_class& n) { return n.get_ui(); }
};

template <>
struct MpzTo< long > {
  long operator()(const mpz_class& n) { return n.get_si(); }
};

/// \brief Helper to handle unsigned long long
template < bool >
struct MpzToUnsignedLongLong;

template <>
struct MpzToUnsignedLongLong< true > {
  unsigned long long operator()(const mpz_class& n) {
    return static_cast< unsigned long long >(n.get_ui());
  }
};

template <>
struct MpzToUnsignedLongLong< false > {
  unsigned long long operator()(const mpz_class& n) {
    static_assert(sizeof(unsigned long long) == 8, "unexpected size");
    static_assert(sizeof(unsigned int) == 4, "unexpected size");
    auto hi = static_cast< unsigned long long >(mpz_class(n >> 32).get_ui());
    auto lo =
        static_cast< unsigned long long >(mpz_class(n & 0xFFFFFFFF).get_ui());
    return (hi << 32U) + lo;
  }
};

template <>
struct MpzTo< unsigned long long >
    : public MpzToUnsignedLongLong< sizeof(unsigned long long) ==
                                    sizeof(unsigned long) > {};

/// \brief Helper to handle long long
template < bool >
struct MpzToLongLong;

template <>
struct MpzToLongLong< true > {
  long long operator()(const mpz_class& n) {
    return static_cast< long long >(n.get_si());
  }
};

template <>
struct MpzToLongLong< false > {
  long long operator()(const mpz_class& n) {
    static_assert(sizeof(long long) == 8, "unexpected size");
    static_assert(sizeof(int) == 4, "unexpected size");
    auto hi = static_cast< long long >(mpz_class(n >> 32).get_si());
    auto lo = static_cast< long long >(mpz_class(n & 0xFFFFFFFF).get_ui());
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return (hi << 32) + lo;
  }
};

template <>
struct MpzTo< long long >
    : public MpzToLongLong< sizeof(long long) == sizeof(long) > {};

} // end namespace detail

/// \brief Class for unlimited precision integers
class ZNumber {
private:
  mpz_class _n;

public:
  /// \brief Create a ZNumber from a string representation
  ///
  /// Interpret the null-terminated string `str` in the given base.
  ///
  /// If the string contains unsuitable characters for the given base, throw
  /// an exception NumberError.
  ///
  /// The base may vary from 2 to 36.
  static ZNumber from_string(const char* str, int base = 10) {
    try {
      return ZNumber(mpz_class(str, base));
    } catch (std::invalid_argument&) {
      std::ostringstream buf;
      buf << "ZNumber: invalid conversion from string '" << str << "'";
      throw NumberError(buf.str());
    }
  }

  /// \brief Create a ZNumber from a string representation
  ///
  /// Interpret the string `str` in the given base.
  ///
  /// If the string contains unsuitable characters for the given base, throw
  /// an exception NumberError.
  ///
  /// The base may vary from 2 to 36.
  static ZNumber from_string(const std::string& str, int base = 10) {
    try {
      return ZNumber(mpz_class(str, base));
    } catch (std::invalid_argument&) {
      std::ostringstream buf;
      buf << "ZNumber: invalid conversion from string '" << str << "'";
      throw NumberError(buf.str());
    }
  }

  /// \name Constructors
  /// @{

  /// \brief Default constructor that creates a ZNumber equals to 0
  ZNumber() = default;

  /// \brief Copy constructor
  ZNumber(const ZNumber&) = default;

  /// \brief Move constructor
  // TODO(marthaud): Add noexcept when mpz_class(mpz_class&&) becomes noexcept
  ZNumber(ZNumber&&) = default;

  /// \brief Create a ZNumber from a mpz_class
  explicit ZNumber(const mpz_class& n) : _n(n) {}

  /// \brief Create a ZNumber from a mpz_class
  explicit ZNumber(mpz_class&& n) : _n(std::move(n)) {}

  /// \brief Create a ZNumber from an integral type
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  explicit ZNumber(T n) : _n(detail::MpzAdapter< T >()(n)) {}

  /// \brief Destructor
  ~ZNumber() = default;

  /// @}
  /// \name Assignment Operators
  /// @{

  /// \brief Copy assignment
  ZNumber& operator=(const ZNumber&) = default;

  /// \brief Move assignment
  ZNumber& operator=(ZNumber&&) noexcept = default;

  /// \brief Assignment for integral types
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  ZNumber& operator=(T n) {
    this->_n = detail::MpzAdapter< T >()(n);
    return *this;
  }

  /// \brief Addition assignment
  ZNumber& operator+=(const ZNumber& x) {
    this->_n += x._n;
    return *this;
  }

  /// \brief Addition assignment with integral types
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  ZNumber& operator+=(T x) {
    this->_n += detail::MpzAdapter< T >()(x);
    return *this;
  }

  /// \brief Subtraction assignment
  ZNumber& operator-=(const ZNumber& x) {
    this->_n -= x._n;
    return *this;
  }

  /// \brief Subtraction assignment with integral types
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  ZNumber& operator-=(T x) {
    this->_n -= detail::MpzAdapter< T >()(x);
    return *this;
  }

  /// \brief Multiplication assignment
  ZNumber& operator*=(const ZNumber& x) {
    this->_n *= x._n;
    return *this;
  }

  /// \brief Multiplication assignment with integral types
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  ZNumber& operator*=(T x) {
    this->_n *= detail::MpzAdapter< T >()(x);
    return *this;
  }

  /// \brief Integer division assignment
  ///
  /// Integer division with rounding towards zero.
  ZNumber& operator/=(const ZNumber& x) {
    ikos_assert_msg(x._n != 0, "division by zero");
    this->_n /= x._n;
    return *this;
  }

  /// \brief Integer division assignment with integral types
  ///
  /// Integer division with rounding towards zero.
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  ZNumber& operator/=(T x) {
    ikos_assert_msg(x != 0, "division by zero");
    this->_n /= detail::MpzAdapter< T >()(x);
    return *this;
  }

  /// \brief Remainder assignment
  ///
  /// The result `r` will satisfy:
  /// `this = q * abs(x) + r` with `0 <= abs(r) < abs(x)`
  ///
  /// The sign of `x` is ignored, and the result will have the same sign as
  /// `this`.
  ZNumber& operator%=(const ZNumber& x) {
    ikos_assert_msg(x._n != 0, "division by zero");
    this->_n %= x._n;
    return *this;
  }

  /// \brief Remainder assignment with integral types
  ///
  /// The result `r` will satisfy:
  /// `this = q * abs(x) + r` with `0 <= abs(r) < abs(x)`
  ///
  /// The sign of `x` is ignored, and the result will have the same sign as
  /// `this`.
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  ZNumber& operator%=(T x) {
    ikos_assert_msg(x != 0, "division by zero");
    this->_n %= detail::MpzAdapter< T >()(x);
    return *this;
  }

  /// \brief Bitwise AND assignment
  ZNumber& operator&=(const ZNumber& x) {
    this->_n &= x._n;
    return *this;
  }

  /// \brief Bitwise AND assignment with integral types
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  ZNumber& operator&=(T x) {
    this->_n &= detail::MpzAdapter< T >()(x);
    return *this;
  }

  /// \brief Bitwise OR assignment
  ZNumber& operator|=(const ZNumber& x) {
    this->_n |= x._n;
    return *this;
  }

  /// \brief Bitwise OR assignment with integral types
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  ZNumber& operator|=(T x) {
    this->_n |= detail::MpzAdapter< T >()(x);
    return *this;
  }

  /// \brief Bitwise XOR assignment
  ZNumber& operator^=(const ZNumber& x) {
    this->_n ^= x._n;
    return *this;
  }

  /// \brief Bitwise XOR assignment with integral types
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  ZNumber& operator^=(T x) {
    this->_n ^= detail::MpzAdapter< T >()(x);
    return *this;
  }

  /// \brief Left binary shift assignment
  ///
  /// This is undefined if `x` isn't between 0 and 2**32 - 1
  ZNumber& operator<<=(const ZNumber& x) {
    ikos_assert_msg(x._n >= 0, "shift count is negative");
    ikos_assert_msg(x._n.fits_ulong_p(), "shift count is too big");
    this->_n <<= x._n.get_ui();
    return *this;
  }

  /// \brief Left binary shift assignment with integral types
  ///
  /// This is undefined if `x` isn't between 0 and 2**32 - 1
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  ZNumber& operator<<=(T x) {
    ikos_assert_msg(x >= 0, "shift count is negative");
    this->_n <<= static_cast< unsigned long int >(x);
    return *this;
  }

  /// \brief Right binary shift
  ///
  /// This is undefined if `x` isn't between 0 and 2**32 - 1
  ZNumber& operator>>=(const ZNumber& x) {
    ikos_assert_msg(x._n >= 0, "shift count is negative");
    ikos_assert_msg(x._n.fits_ulong_p(), "shift count is too big");
    this->_n >>= x._n.get_ui();
    return *this;
  }

  /// \brief Right binary shift with integral types
  ///
  /// This is undefined if `x` isn't between 0 and 2**32 - 1
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  ZNumber& operator>>=(T x) {
    ikos_assert_msg(x >= 0, "shift count is negative");
    this->_n >>= static_cast< unsigned long int >(x);
    return *this;
  }

  /// @}
  /// \name Unary Operators
  /// @{

  /// \brief Unary plus
  const ZNumber& operator+() const { return *this; }

  /// \brief Prefix increment
  ZNumber& operator++() {
    ++this->_n;
    return *this;
  }

  /// \brief Postfix increment
  const ZNumber operator++(int) {
    ZNumber r(*this);
    ++this->_n;
    return r;
  }

  /// \brief Unary minus
  const ZNumber operator-() const { return ZNumber(-this->_n); }

  /// \brief Prefix decrement
  ZNumber& operator--() {
    --this->_n;
    return *this;
  }

  /// \brief Postfix decrement
  const ZNumber operator--(int) {
    ZNumber r(*this);
    --this->_n;
    return r;
  }

  /// \brief Return the next power of 2 greater or equal to this number
  ///
  /// This is undefined for negative numbers.
  ZNumber next_power_of_2() const {
    ikos_assert(this->_n >= 0);

    if (this->_n <= 1) {
      return ZNumber(1);
    }

    ZNumber n(this->_n - 1);
    return ZNumber(mpz_class(1) << n.size_in_bits());
  }

  /// @}
  /// \name Value tests
  /// @{

  /// \brief Return the number of trailing '0' bits
  ///
  /// This is undefined if the number is 0.
  uint64_t trailing_zeros() const {
    ikos_assert(this->_n != 0);
    return mpz_scan1(this->_n.get_mpz_t(), 0);
  }

  /// \brief Return the number of trailing '1' bits
  ///
  /// This is undefined if the number is -1.
  uint64_t trailing_ones() const {
    ikos_assert(this->_n != -1);
    return mpz_scan0(this->_n.get_mpz_t(), 0);
  }

  /// \brief Return the number of bits
  ///
  /// The sign is ignored.
  uint64_t size_in_bits() const {
    return mpz_sizeinbase(this->_n.get_mpz_t(), 2);
  }

  /// @}
  /// \name Conversion Functions
  /// @{

  /// \brief Get the internal representation
  const mpz_class& mpz() const { return this->_n; }

  /// \brief Return true if the number fits in the given integer type
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  bool fits() const {
    return detail::MpzFits< T >()(this->_n);
  }

  /// \brief Return the number as the given integer type
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  T to() const {
    ikos_assert_msg(detail::MpzFits< T >()(this->_n), "does not fit");
    return detail::MpzTo< T >()(this->_n);
  }

  /// \brief Return a string representation of the ZNumber in the given base
  ///
  /// The base can vary from 2 to 36, or from -2 to -36
  std::string str(int base = 10) const { return this->_n.get_str(base); }

  /// @}

  friend ZNumber mod(const ZNumber&, const ZNumber&);

  friend ZNumber gcd(const ZNumber&, const ZNumber&);

  friend ZNumber lcm(const ZNumber&, const ZNumber&);

  friend void gcd_extended(
      const ZNumber&, const ZNumber&, ZNumber&, ZNumber&, ZNumber&);

  friend std::istream& operator>>(std::istream& i, ZNumber& n);

  friend class QNumber;

  friend ZNumber single_mask(const ZNumber& size);

  friend ZNumber double_mask(const ZNumber& low, const ZNumber& high);

  friend ZNumber make_clipped_mask(const ZNumber& low,
                                   const ZNumber& size,
                                   const ZNumber& lower_clip,
                                   const ZNumber& size_clip);

}; // end class ZNumber

/// \name Binary Operators
/// @{

/// \brief Addition
inline ZNumber operator+(const ZNumber& lhs, const ZNumber& rhs) {
  return ZNumber(lhs.mpz() + rhs.mpz());
}

/// \brief Addition with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator+(const ZNumber& lhs, T rhs) {
  return ZNumber(lhs.mpz() + detail::MpzAdapter< T >()(rhs));
}

/// \brief Addition with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator+(T lhs, const ZNumber& rhs) {
  return ZNumber(detail::MpzAdapter< T >()(lhs) + rhs.mpz());
}

/// \brief Subtraction
inline ZNumber operator-(const ZNumber& lhs, const ZNumber& rhs) {
  return ZNumber(lhs.mpz() - rhs.mpz());
}

/// \brief Subtraction with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator-(const ZNumber& lhs, T rhs) {
  return ZNumber(lhs.mpz() - detail::MpzAdapter< T >()(rhs));
}

/// \brief Subtraction with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator-(T lhs, const ZNumber& rhs) {
  return ZNumber(detail::MpzAdapter< T >()(lhs) - rhs.mpz());
}

/// \brief Multiplication
inline ZNumber operator*(const ZNumber& lhs, const ZNumber& rhs) {
  return ZNumber(lhs.mpz() * rhs.mpz());
}

/// \brief Multiplication with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator*(const ZNumber& lhs, T rhs) {
  return ZNumber(lhs.mpz() * detail::MpzAdapter< T >()(rhs));
}

/// \brief Multiplication with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator*(T lhs, const ZNumber& rhs) {
  return ZNumber(detail::MpzAdapter< T >()(lhs) * rhs.mpz());
}

/// \brief Integer division
///
/// Integer division with rounding towards zero.
inline ZNumber operator/(const ZNumber& lhs, const ZNumber& rhs) {
  ikos_assert_msg(rhs.mpz() != 0, "division by zero");
  return ZNumber(lhs.mpz() / rhs.mpz());
}

/// \brief Integer division with integral types
///
/// Integer division with rounding towards zero.
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator/(const ZNumber& lhs, T rhs) {
  ikos_assert_msg(rhs != 0, "division by zero");
  return ZNumber(lhs.mpz() / detail::MpzAdapter< T >()(rhs));
}

/// \brief Integer division with integral types
///
/// Integer division with rounding towards zero.
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator/(T lhs, const ZNumber& rhs) {
  ikos_assert_msg(rhs.mpz() != 0, "division by zero");
  return ZNumber(detail::MpzAdapter< T >()(lhs) / rhs.mpz());
}

/// \brief Remainder
///
/// The result `r` will satisfy:
/// `lhs = q * abs(rhs) + r` with `0 <= abs(r) < abs(rhs)`
///
/// The sign of `rhs` is ignored, and the result will have the same sign as
/// `lhs`.
inline ZNumber operator%(const ZNumber& lhs, const ZNumber& rhs) {
  ikos_assert_msg(rhs.mpz() != 0, "division by zero");
  return ZNumber(lhs.mpz() % rhs.mpz());
}

/// \brief Remainder with integral types
///
/// The result `r` will satisfy:
/// `lhs = q * abs(rhs) + r` with `0 <= abs(r) < abs(rhs)`
///
/// The sign of `rhs` is ignored, and the result will have the same sign as
/// `lhs`.
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator%(const ZNumber& lhs, T rhs) {
  ikos_assert_msg(rhs != 0, "division by zero");
  return ZNumber(lhs.mpz() % detail::MpzAdapter< T >()(rhs));
}

/// \brief Remainder with integral types
///
/// The result `r` will satisfy:
/// `lhs = q * abs(rhs) + r` with `0 <= abs(r) < abs(rhs)`
///
/// The sign of `rhs` is ignored, and the result will have the same sign as
/// `lhs`.
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator%(T lhs, const ZNumber& rhs) {
  ikos_assert_msg(rhs.mpz() != 0, "division by zero");
  return ZNumber(detail::MpzAdapter< T >()(lhs) % rhs.mpz());
}

/// \brief Bitwise AND
inline ZNumber operator&(const ZNumber& lhs, const ZNumber& rhs) {
  return ZNumber(lhs.mpz() & rhs.mpz());
}

/// \brief Bitwise AND with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator&(const ZNumber& lhs, T rhs) {
  return ZNumber(lhs.mpz() & detail::MpzAdapter< T >()(rhs));
}

/// \brief Bitwise AND with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator&(T lhs, const ZNumber& rhs) {
  return ZNumber(detail::MpzAdapter< T >()(lhs) & rhs.mpz());
}

/// \brief Bitwise OR
inline ZNumber operator|(const ZNumber& lhs, const ZNumber& rhs) {
  return ZNumber(lhs.mpz() | rhs.mpz());
}

/// \brief Bitwise OR with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator|(const ZNumber& lhs, T rhs) {
  return ZNumber(lhs.mpz() | detail::MpzAdapter< T >()(rhs));
}

/// \brief Bitwise OR with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator|(T lhs, const ZNumber& rhs) {
  return ZNumber(detail::MpzAdapter< T >()(lhs) | rhs.mpz());
}

/// \brief Bitwise XOR
inline ZNumber operator^(const ZNumber& lhs, const ZNumber& rhs) {
  return ZNumber(lhs.mpz() ^ rhs.mpz());
}

/// \brief Bitwise XOR with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator^(const ZNumber& lhs, T rhs) {
  return ZNumber(lhs.mpz() ^ detail::MpzAdapter< T >()(rhs));
}

/// \brief Bitwise XOR with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator^(T lhs, const ZNumber& rhs) {
  return ZNumber(detail::MpzAdapter< T >()(lhs) ^ rhs.mpz());
}

/// \brief Left binary shift
///
/// This is undefined if `rhs` isn't between 0 and 2**32 - 1
inline ZNumber operator<<(const ZNumber& lhs, const ZNumber& rhs) {
  ikos_assert_msg(rhs.mpz() >= 0, "shift count is negative");
  ikos_assert_msg(rhs.mpz().fits_ulong_p(), "shift count is too big");
  return ZNumber(lhs.mpz() << rhs.mpz().get_ui());
}

/// \brief Left binary shift with integral types
///
/// This is undefined if `rhs` isn't between 0 and 2**32 - 1
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator<<(const ZNumber& lhs, T rhs) {
  ikos_assert_msg(rhs >= 0, "shift count is negative");
  return ZNumber(lhs.mpz() << static_cast< unsigned long int >(rhs));
}

/// \brief Left binary shift with integral types
///
/// This is undefined if `rhs` isn't between 0 and 2**32 - 1
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator<<(T lhs, const ZNumber& rhs) {
  ZNumber r(lhs);
  r <<= rhs;
  return r;
}

/// \brief Right binary shift
///
/// This is undefined if `rhs` isn't between 0 and 2**32 - 1
inline ZNumber operator>>(const ZNumber& lhs, const ZNumber& rhs) {
  ikos_assert_msg(rhs.mpz() >= 0, "shift count is negative");
  ikos_assert_msg(rhs.mpz().fits_ulong_p(), "shift count is too big");
  return ZNumber(lhs.mpz() >> rhs.mpz().get_ui());
}

/// \brief Right binary shift with integral types
///
/// This is undefined if `rhs` isn't between 0 and 2**32 - 1
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator>>(const ZNumber& lhs, T rhs) {
  ikos_assert_msg(rhs >= 0, "shift count is negative");
  return ZNumber(lhs.mpz() >> static_cast< unsigned long int >(rhs));
}

/// \brief Right binary shift with integral types
///
/// This is undefined if `rhs` isn't between 0 and 2**32 - 1
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline ZNumber operator>>(T lhs, const ZNumber& rhs) {
  ZNumber r(lhs);
  r >>= rhs;
  return r;
}

/// @}
/// \name Comparison Operators
/// @{

/// \brief Equality operator
inline bool operator==(const ZNumber& lhs, const ZNumber& rhs) {
  return lhs.mpz() == rhs.mpz();
}

/// \brief Equality operator with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator==(const ZNumber& lhs, T rhs) {
  return lhs.mpz() == detail::MpzAdapter< T >()(rhs);
}

/// \brief Equality operator with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator==(T lhs, const ZNumber& rhs) {
  return detail::MpzAdapter< T >()(lhs) == rhs.mpz();
}

/// \brief Inequality operator
inline bool operator!=(const ZNumber& lhs, const ZNumber& rhs) {
  return lhs.mpz() != rhs.mpz();
}

/// \brief Inequality operator with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator!=(const ZNumber& lhs, T rhs) {
  return lhs.mpz() != detail::MpzAdapter< T >()(rhs);
}

/// \brief Inequality operator with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator!=(T lhs, const ZNumber& rhs) {
  return detail::MpzAdapter< T >()(lhs) != rhs.mpz();
}

/// \brief Less than comparison
inline bool operator<(const ZNumber& lhs, const ZNumber& rhs) {
  return lhs.mpz() < rhs.mpz();
}

/// \brief Less than comparison with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator<(const ZNumber& lhs, T rhs) {
  return lhs.mpz() < detail::MpzAdapter< T >()(rhs);
}

/// \brief Less than comparison with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator<(T lhs, const ZNumber& rhs) {
  return detail::MpzAdapter< T >()(lhs) < rhs.mpz();
}

/// \brief Less or equal comparison
inline bool operator<=(const ZNumber& lhs, const ZNumber& rhs) {
  return lhs.mpz() <= rhs.mpz();
}

/// \brief Less or equal comparison with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator<=(const ZNumber& lhs, T rhs) {
  return lhs.mpz() <= detail::MpzAdapter< T >()(rhs);
}

/// \brief Less or equal comparison with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator<=(T lhs, const ZNumber& rhs) {
  return detail::MpzAdapter< T >()(lhs) <= rhs.mpz();
}

/// \brief Greater than comparison
inline bool operator>(const ZNumber& lhs, const ZNumber& rhs) {
  return lhs.mpz() > rhs.mpz();
}

/// \brief Greater than comparison with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator>(const ZNumber& lhs, T rhs) {
  return lhs.mpz() > detail::MpzAdapter< T >()(rhs);
}

/// \brief Greater than comparison with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator>(T lhs, const ZNumber& rhs) {
  return detail::MpzAdapter< T >()(lhs) > rhs.mpz();
}

/// \brief Greater or equal comparison
inline bool operator>=(const ZNumber& lhs, const ZNumber& rhs) {
  return lhs.mpz() >= rhs.mpz();
}

/// \brief Greater or equal comparison with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator>=(const ZNumber& lhs, T rhs) {
  return lhs.mpz() >= detail::MpzAdapter< T >()(rhs);
}

/// \brief Greater or equal comparison with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator>=(T lhs, const ZNumber& rhs) {
  return detail::MpzAdapter< T >()(lhs) >= rhs.mpz();
}

/// @}
/// \name Utility functions (min, max, abs, gcd, lcm, etc.)
/// @{

/// \brief Return the smaller of the given numbers
inline const ZNumber& min(const ZNumber& a, const ZNumber& b) {
  return (a < b) ? a : b;
}

/// \brief Return the smaller of the given numbers
inline const ZNumber& min(const ZNumber& a,
                          const ZNumber& b,
                          const ZNumber& c) {
  return min(min(a, b), c);
}

/// \brief Return the smaller of the given numbers
inline const ZNumber& min(const ZNumber& a,
                          const ZNumber& b,
                          const ZNumber& c,
                          const ZNumber& d) {
  return min(min(min(a, b), c), d);
}

/// \brief Return the greater of the given numbers
inline const ZNumber& max(const ZNumber& a, const ZNumber& b) {
  return (a < b) ? b : a;
}

/// \brief Return the greater of the given numbers
inline const ZNumber& max(const ZNumber& a,
                          const ZNumber& b,
                          const ZNumber& c) {
  return max(max(a, b), c);
}

/// \brief Return the greater of the given numbers
inline const ZNumber& max(const ZNumber& a,
                          const ZNumber& b,
                          const ZNumber& c,
                          const ZNumber& d) {
  return max(max(max(a, b), c), d);
}

/// \brief Return `n mod d`
///
/// The result `r` will satisfy:
/// `n = q * abs(d) + r` with `0 <= r < abs(d)`
///
/// The sign of `d` is ignored, and the result is always non-negative.
inline ZNumber mod(const ZNumber& n, const ZNumber& d) {
  ikos_assert_msg(d.mpz() != 0, "division by zero");
  ZNumber r;
  mpz_mod(r._n.get_mpz_t(), n._n.get_mpz_t(), d._n.get_mpz_t());
  return r;
}

/// \brief Return the absolute value of the given number
inline ZNumber abs(const ZNumber& n) {
  return ZNumber(abs(n.mpz()));
}

/// \brief Return the greatest common divisor of the given numbers
///
/// The result is always positive even if one or both input operands are
/// negative. Except if both inputs are zero; then this function defines
/// `gcd(0, 0) = 0`.
inline ZNumber gcd(const ZNumber& a, const ZNumber& b) {
  ZNumber r;
  mpz_gcd(r._n.get_mpz_t(), a._n.get_mpz_t(), b._n.get_mpz_t());
  return r;
}

/// \brief Return the greatest common divisor of the given numbers
inline ZNumber gcd(const ZNumber& a, const ZNumber& b, const ZNumber& c) {
  return gcd(gcd(a, b), c);
}

/// \brief Return the least common multiple of the given numbers
inline ZNumber lcm(const ZNumber& a, const ZNumber& b) {
  ZNumber r;
  mpz_lcm(r._n.get_mpz_t(), a._n.get_mpz_t(), b._n.get_mpz_t());
  return r;
}

/// \brief Run Euclid's algorithm
///
/// Compute `g = gcd(a, b)` and `u`, `v` such that `g = a*u + b*v`
//
/// The value in `g` is always positive, even if one or both of `a` and `b` are
/// negative (or zero if both inputs are zero).
inline void gcd_extended(
    const ZNumber& a, const ZNumber& b, ZNumber& g, ZNumber& u, ZNumber& v) {
  mpz_gcdext(g._n.get_mpz_t(),
             u._n.get_mpz_t(),
             v._n.get_mpz_t(),
             a._n.get_mpz_t(),
             b._n.get_mpz_t());
}

/// @}
/// \name Input / Output
/// @{

/// \brief Write a ZNumber on a stream, in base 10
inline std::ostream& operator<<(std::ostream& o, const ZNumber& n) {
  o << n.mpz();
  return o;
}

/// \brief Read a ZNumber from a stream, in base 10
inline std::istream& operator>>(std::istream& i, ZNumber& n) {
  i >> n._n;
  return i;
}

/// @}

/// \brief Return the hash of a ZNumber
inline std::size_t hash_value(const ZNumber& n) {
  const mpz_class& m = n.mpz();
  std::size_t result = 0;
  boost::hash_combine(result, m.get_mpz_t()[0]._mp_size);
  for (int i = 0, e = std::abs(m.get_mpz_t()[0]._mp_size); i < e; ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    boost::hash_combine(result, m.get_mpz_t()[0]._mp_d[i]);
  }
  return result;
}

/// \brief Return a mask with size bits starting at 0.
inline ZNumber single_mask(const ZNumber& size) {
  ZNumber pow2 = ZNumber(1) << size;
  ZNumber mask = pow2 - 1;
  return mask;
}

/// \brief Return a mask with bits [low, high).
inline ZNumber double_mask(const ZNumber& low, const ZNumber& high) {
  ZNumber high_mask = single_mask(high);
  ZNumber low_mask = single_mask(low);
  ZNumber result = high_mask - low_mask;
  return result;
}

/// \brief return a bit mask with bits in the range
/// [0, size_clip), with bits taken from the intersection of
/// [low, low + size) with [low_clip, low_clip + size_clip)
/// with the result shifted by low_clip to fit in the
/// range [0, size_clip).
inline ZNumber make_clipped_mask(const ZNumber& low,
                                 const ZNumber& size,
                                 const ZNumber& lower_clip,
                                 const ZNumber& size_clip) {
  ZNumber upper_clip = lower_clip + size_clip;
  ZNumber upper = low + size;
  if ((low >= upper_clip) || (upper <= lower_clip)) {
    // No overlap.
    return ZNumber(0);
  }
  // Because of the above, we assert
  // (low < upper_clip) && (upper > lower_clip).

  ZNumber shifted_low = low - lower_clip;
  ZNumber shifted_size = size;
  if (shifted_low < 0) {
    shifted_size = shifted_size + shifted_low;
    // shifted_size = size + shifted_low
    //              = (upper - low) + (low - lower_clip)
    //              = upper - lower_clip
    //              > 0   because of the above test.
    shifted_low = 0;
  }
  ZNumber shifted_upper = upper - lower_clip;
  // By the above shifted_upper > 0.
  // But we do not want it to exceed size_clip.
  if (shifted_upper > size_clip) {
    shifted_upper = size_clip;
  }

  ZNumber result = double_mask(shifted_low, shifted_upper);
  return result;
}

} // end namespace core
} // end namespace ikos

namespace std {

/// \brief std::hash implementation for ZNumber
template <>
struct hash< ikos::core::ZNumber > {
  std::size_t operator()(const ikos::core::ZNumber& n) const {
    return ikos::core::hash_value(n);
  }
};

} // end namespace std
