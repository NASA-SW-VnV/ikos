/*******************************************************************************
 *
 * \file
 * \brief Unlimited precision rationals
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
#include <ikos/core/number/z_number.hpp>
#include <ikos/core/support/assert.hpp>

namespace ikos {
namespace core {

/// \brief Checks wether T is a supported integral type or ZNumber
template < typename T >
struct IsSupportedIntegralOrZNumber : public IsSupportedIntegral< T > {};

template <>
struct IsSupportedIntegralOrZNumber< const ZNumber& > : public std::true_type {
};

namespace detail {

/// \brief Helper that returns either an integral type or a mpz_class
template < typename T >
struct ZNumberAdapter : public MpzAdapter< T > {};

template <>
struct ZNumberAdapter< const ZNumber& > {
  const mpz_class& operator()(const ZNumber& n) { return n.mpz(); }
};

} // end namespace detail

/// \brief Class for unlimited precision rationals
class QNumber {
private:
  mpq_class _n;

public:
  /// \brief Create a QNumber from a string representation
  ///
  /// Interpret the null-terminated string `str` in the given base.
  ///
  /// If the string contains unsuitable characters for the given base, throw
  /// an exception NumberError.
  ///
  /// The base may vary from 2 to 36.
  static QNumber from_string(const char* str, int base = 10) {
    try {
      return QNumber(mpq_class(str, base));
    } catch (std::invalid_argument&) {
      std::ostringstream buf;
      buf << "QNumber: invalid conversion from string '" << str << "'";
      throw NumberError(buf.str());
    }
  }

  /// \brief Create a QNumber from a string representation
  ///
  /// Interpret the string `str` in the given base.
  ///
  /// If the string contains unsuitable characters for the given base, throw
  /// an exception NumberError.
  ///
  /// The base may vary from 2 to 36.
  static QNumber from_string(const std::string& str, int base = 10) {
    try {
      return QNumber(mpq_class(str, base));
    } catch (std::invalid_argument&) {
      std::ostringstream buf;
      buf << "QNumber: invalid conversion from string '" << str << "'";
      throw NumberError(buf.str());
    }
  }

  /// \name Constructors
  /// @{

  /// \brief Default constructor that creates a QNumber equals to 0
  QNumber() = default;

  /// \brief Copy constructor
  QNumber(const QNumber&) = default;

  /// \brief Move constructor
  QNumber(QNumber&&) = default;

  /// \brief Create a QNumber from a ZNumber
  explicit QNumber(const ZNumber& n) : _n(n._n) {}

  /// \brief Create a QNumber from a ZNumber
  explicit QNumber(ZNumber&& n) : _n(std::move(n)._n) {}

  /// \brief Create a QNumber from an integral type
  template < typename N,
             class = std::enable_if_t< IsSupportedIntegral< N >::value > >
  explicit QNumber(N n) : _n(detail::MpzAdapter< N >()(n)) {}

  /// \brief Create a QNumber from a mpq_class
  explicit QNumber(const mpq_class& n) : _n(n) {
    ikos_assert_msg(this->_n.get_den() != 0, "denominator is zero");
    this->_n.canonicalize();
  }

  /// \brief Create a QNumber from a mpq_class
  explicit QNumber(mpq_class&& n) : _n(std::move(n)) {
    ikos_assert_msg(this->_n.get_den() != 0, "denominator is zero");
    this->_n.canonicalize();
  }

  /// \brief Create a QNumber from a numerator and a denominator
  template < typename N,
             typename D,
             class = std::enable_if_t< IsSupportedIntegral< N >::value &&
                                       IsSupportedIntegral< D >::value > >
  explicit QNumber(N n, D d)
      : _n(detail::MpzAdapter< N >()(n), detail::MpzAdapter< D >()(d)) {
    ikos_assert_msg(this->_n.get_den() != 0, "denominator is zero");
    this->_n.canonicalize();
  }

  /// \brief Create a QNumber from a numerator and a denominator
  explicit QNumber(const ZNumber& n, const ZNumber& d) : _n(n._n, d._n) {
    ikos_assert_msg(this->_n.get_den() != 0, "denominator is zero");
    this->_n.canonicalize();
  }

  /// \brief Create a QNumber from a numerator and a denominator
  explicit QNumber(ZNumber&& n, ZNumber&& d)
      : _n(std::move(n)._n, std::move(d)._n) {
    ikos_assert_msg(this->_n.get_den() != 0, "denominator is zero");
    this->_n.canonicalize();
  }

  struct NormalizedTag {};

  /// \brief Create a QNumber from a normalized mpq_class
  QNumber(const mpq_class& n, NormalizedTag) : _n(n) {}

  /// \brief Create a QNumber from a normalized mpq_class
  QNumber(mpq_class&& n, NormalizedTag) : _n(std::move(n)) {}

  /// \brief Destructor
  ~QNumber() = default;

  /// @}
  /// \name Assignment Operators
  /// @{

  /// \brief Copy assignment
  QNumber& operator=(const QNumber&) = default;

  /// \brief Move assignment
  QNumber& operator=(QNumber&&) noexcept = default;

  /// \brief Assignment for ZNumber
  QNumber& operator=(const ZNumber& n) {
    this->_n = n._n;
    return *this;
  }

  /// \brief Assignment for ZNumber
  QNumber& operator=(ZNumber&& n) noexcept {
    this->_n = std::move(n._n);
    return *this;
  }

  /// \brief Assignment for integral types
  template < typename N,
             typename = std::enable_if_t< IsSupportedIntegral< N >::value > >
  QNumber& operator=(N n) {
    this->_n = detail::MpzAdapter< N >()(n);
    return *this;
  }

  /// \brief Addition assignment
  QNumber& operator+=(const QNumber& x) {
    this->_n += x._n;
    return *this;
  }

  /// \brief Addition assignment with integral types or ZNumber
  template <
      typename N,
      class = std::enable_if_t< IsSupportedIntegralOrZNumber< N >::value > >
  QNumber& operator+=(N x) {
    this->_n += detail::ZNumberAdapter< N >()(x);
    return *this;
  }

  /// \brief Subtraction assignment
  QNumber& operator-=(const QNumber& x) {
    this->_n -= x._n;
    return *this;
  }

  /// \brief Subtraction assignment with integral types or ZNumber
  template <
      typename N,
      class = std::enable_if_t< IsSupportedIntegralOrZNumber< N >::value > >
  QNumber& operator-=(N x) {
    this->_n -= detail::ZNumberAdapter< N >()(x);
    return *this;
  }

  /// \brief Multiplication assignment
  QNumber& operator*=(const QNumber& x) {
    this->_n *= x._n;
    return *this;
  }

  /// \brief Multiplication assignment with integral types or ZNumber
  template <
      typename N,
      class = std::enable_if_t< IsSupportedIntegralOrZNumber< N >::value > >
  QNumber& operator*=(N x) {
    this->_n *= detail::ZNumberAdapter< N >()(x);
    return *this;
  }

  /// \brief Division assignment
  QNumber& operator/=(const QNumber& x) {
    ikos_assert_msg(x._n != 0, "division by zero");
    this->_n /= x._n;
    return *this;
  }

  /// \brief Division assignment with integral types or ZNumber
  template <
      typename N,
      class = std::enable_if_t< IsSupportedIntegralOrZNumber< N >::value > >
  QNumber& operator/=(N x) {
    ikos_assert_msg(x != 0, "division by zero");
    this->_n /= detail::ZNumberAdapter< N >()(x);
    return *this;
  }

  /// @}
  /// \name Unary Operators
  /// @{

  /// \brief Unary plus
  const QNumber& operator+() const { return *this; }

  /// \brief Prefix increment
  QNumber& operator++() {
    ++this->_n;
    return *this;
  }

  /// \brief Postfix increment
  const QNumber operator++(int) {
    QNumber r(*this);
    ++this->_n;
    return r;
  }

  /// \brief Unary minus
  const QNumber operator-() const { return QNumber(-this->_n); }

  /// \brief Prefix decrement
  QNumber& operator--() {
    --this->_n;
    return *this;
  }

  /// \brief Postfix decrement
  const QNumber operator--(int) {
    QNumber r(*this);
    --this->_n;
    return r;
  }

  /// @}
  /// \name Value generators
  /// @{

  /// \brief Get the numerator
  ZNumber numerator() const { return ZNumber(this->_n.get_num()); }

  /// \brief Get the denominator
  ZNumber denominator() const { return ZNumber(this->_n.get_den()); }

  /// \brief Round to upper integer
  ZNumber round_to_upper() const {
    const mpz_class& num = this->_n.get_num();
    const mpz_class& den = this->_n.get_den();
    ZNumber q(num / den);
    ZNumber r(num % den);
    if (r == 0 || this->_n < 0) {
      return q;
    } else {
      return q + 1;
    }
  }

  /// \brief Round to lower integer
  ZNumber round_to_lower() const {
    const mpz_class& num = this->_n.get_num();
    const mpz_class& den = this->_n.get_den();
    ZNumber q(num / den);
    ZNumber r(num % den);
    if (r == 0 || this->_n > 0) {
      return q;
    } else {
      return q - 1;
    }
  }

  /// @}
  /// \name Conversion Functions
  /// @{

  /// \brief Get the internal representation
  const mpq_class& mpq() const { return this->_n; }

  /// \brief Return a string of the QNumber in the given base
  ///
  /// The base can vary from 2 to 36, or from -2 to -36
  std::string str(int base = 10) const { return this->_n.get_str(base); }

  /// @}

  friend std::istream& operator>>(std::istream& i, QNumber& n);

}; // end class QNumber

/// \name Binary Operators
/// @{

/// \brief Addition
inline QNumber operator+(const QNumber& lhs, const QNumber& rhs) {
  return QNumber(lhs.mpq() + rhs.mpq(), QNumber::NormalizedTag{});
}

/// \brief Addition with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline QNumber operator+(const QNumber& lhs, T rhs) {
  return QNumber(lhs.mpq() + detail::ZNumberAdapter< T >()(rhs),
                 QNumber::NormalizedTag{});
}

/// \brief Addition with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline QNumber operator+(T lhs, const QNumber& rhs) {
  return QNumber(detail::ZNumberAdapter< T >()(lhs) + rhs.mpq(),
                 QNumber::NormalizedTag{});
}

/// \brief Subtraction
inline QNumber operator-(const QNumber& lhs, const QNumber& rhs) {
  return QNumber(lhs.mpq() - rhs.mpq(), QNumber::NormalizedTag{});
}

/// \brief Subtraction with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline QNumber operator-(const QNumber& lhs, T rhs) {
  return QNumber(lhs.mpq() - detail::ZNumberAdapter< T >()(rhs),
                 QNumber::NormalizedTag{});
}

/// \brief Subtraction with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline QNumber operator-(T lhs, const QNumber& rhs) {
  return QNumber(detail::ZNumberAdapter< T >()(lhs) - rhs.mpq(),
                 QNumber::NormalizedTag{});
}

/// \brief Multiplication
inline QNumber operator*(const QNumber& lhs, const QNumber& rhs) {
  return QNumber(lhs.mpq() * rhs.mpq(), QNumber::NormalizedTag{});
}

/// \brief Multiplication with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline QNumber operator*(const QNumber& lhs, T rhs) {
  return QNumber(lhs.mpq() * detail::ZNumberAdapter< T >()(rhs),
                 QNumber::NormalizedTag{});
}

/// \brief Multiplication with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline QNumber operator*(T lhs, const QNumber& rhs) {
  return QNumber(detail::ZNumberAdapter< T >()(lhs) * rhs.mpq(),
                 QNumber::NormalizedTag{});
}

/// \brief Division
inline QNumber operator/(const QNumber& lhs, const QNumber& rhs) {
  ikos_assert_msg(rhs.mpq() != 0, "division by zero");
  return QNumber(lhs.mpq() / rhs.mpq(), QNumber::NormalizedTag{});
}

/// \brief Division with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline QNumber operator/(const QNumber& lhs, T rhs) {
  ikos_assert_msg(rhs != 0, "division by zero");
  return QNumber(lhs.mpq() / detail::ZNumberAdapter< T >()(rhs),
                 QNumber::NormalizedTag{});
}

/// \brief Division with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline QNumber operator/(T lhs, const QNumber& rhs) {
  ikos_assert_msg(rhs.mpq() != 0, "division by zero");
  return QNumber(detail::ZNumberAdapter< T >()(lhs) / rhs.mpq(),
                 QNumber::NormalizedTag{});
}

/// @}
/// \name Comparison Operators
/// @{

/// \brief Equality operator
inline bool operator==(const QNumber& lhs, const QNumber& rhs) {
  return lhs.mpq() == rhs.mpq();
}

/// \brief Equality operator with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline bool operator==(const QNumber& lhs, T rhs) {
  return lhs.mpq() == detail::ZNumberAdapter< T >()(rhs);
}

/// \brief Equality operator with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline bool operator==(T lhs, const QNumber& rhs) {
  return detail::ZNumberAdapter< T >()(lhs) == rhs.mpq();
}

/// \brief Inequality operator
inline bool operator!=(const QNumber& lhs, const QNumber& rhs) {
  return lhs.mpq() != rhs.mpq();
}

/// \brief Inequality operator with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline bool operator!=(const QNumber& lhs, T rhs) {
  return lhs.mpq() != detail::ZNumberAdapter< T >()(rhs);
}

/// \brief Inequality operator with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline bool operator!=(T lhs, const QNumber& rhs) {
  return detail::ZNumberAdapter< T >()(lhs) != rhs.mpq();
}

/// \brief Less than comparison
inline bool operator<(const QNumber& lhs, const QNumber& rhs) {
  return lhs.mpq() < rhs.mpq();
}

/// \brief Less than comparison with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline bool operator<(const QNumber& lhs, T rhs) {
  return lhs.mpq() < detail::ZNumberAdapter< T >()(rhs);
}

/// \brief Less than comparison with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline bool operator<(T lhs, const QNumber& rhs) {
  return detail::ZNumberAdapter< T >()(lhs) < rhs.mpq();
}

/// \brief Less or equal comparison
inline bool operator<=(const QNumber& lhs, const QNumber& rhs) {
  return lhs.mpq() <= rhs.mpq();
}

/// \brief Less or equal comparison with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline bool operator<=(const QNumber& lhs, T rhs) {
  return lhs.mpq() <= detail::ZNumberAdapter< T >()(rhs);
}

/// \brief Less or equal comparison with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline bool operator<=(T lhs, const QNumber& rhs) {
  return detail::ZNumberAdapter< T >()(lhs) <= rhs.mpq();
}

/// \brief Greater than comparison
inline bool operator>(const QNumber& lhs, const QNumber& rhs) {
  return lhs.mpq() > rhs.mpq();
}

/// \brief Greater than comparison with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline bool operator>(const QNumber& lhs, T rhs) {
  return lhs.mpq() > detail::ZNumberAdapter< T >()(rhs);
}

/// \brief Greater than comparison with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline bool operator>(T lhs, const QNumber& rhs) {
  return detail::ZNumberAdapter< T >()(lhs) > rhs.mpq();
}

/// \brief Greater or equal comparison
inline bool operator>=(const QNumber& lhs, const QNumber& rhs) {
  return lhs.mpq() >= rhs.mpq();
}

/// \brief Greater or equal comparison with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline bool operator>=(const QNumber& lhs, T rhs) {
  return lhs.mpq() >= detail::ZNumberAdapter< T >()(rhs);
}

/// \brief Greater or equal comparison with integral types or ZNumber
template <
    typename T,
    class = std::enable_if_t< IsSupportedIntegralOrZNumber< T >::value > >
inline bool operator>=(T lhs, const QNumber& rhs) {
  return detail::ZNumberAdapter< T >()(lhs) >= rhs.mpq();
}

/// @}
/// \name Utility functions (min, max, and abs)
/// @{

/// \brief Return the smaller of the given numbers
inline const QNumber& min(const QNumber& a, const QNumber& b) {
  return (a < b) ? a : b;
}

/// \brief Return the smaller of the given numbers
inline const QNumber& min(const QNumber& a,
                          const QNumber& b,
                          const QNumber& c) {
  return min(min(a, b), c);
}

/// \brief Return the smaller of the given numbers
inline const QNumber& min(const QNumber& a,
                          const QNumber& b,
                          const QNumber& c,
                          const QNumber& d) {
  return min(min(min(a, b), c), d);
}

/// \brief Return the greater of the given numbers
inline const QNumber& max(const QNumber& a, const QNumber& b) {
  return (a < b) ? b : a;
}

/// \brief Return the greater of the given numbers
inline const QNumber& max(const QNumber& a,
                          const QNumber& b,
                          const QNumber& c) {
  return max(max(a, b), c);
}

/// \brief Return the greater of the given numbers
inline const QNumber& max(const QNumber& a,
                          const QNumber& b,
                          const QNumber& c,
                          const QNumber& d) {
  return max(max(max(a, b), c), d);
}

/// \brief Return the absolute value of the given number
inline QNumber abs(const QNumber& n) {
  return QNumber(abs(n.mpq()));
}

/// @}
/// \name Input / Output
/// @{

/// \brief Write a QNumber on a stream, in base 10
inline std::ostream& operator<<(std::ostream& o, const QNumber& n) {
  o << n.mpq();
  return o;
}

/// \brief Read a QNumber from a stream, in base 10
inline std::istream& operator>>(std::istream& i, QNumber& n) {
  i >> n._n;
  return i;
}

/// @}

/// \brief Return the hash of a QNumber
inline std::size_t hash_value(const QNumber& n) {
  const mpq_class& m = n.mpq();
  std::size_t result = 0;
  boost::hash_combine(result, m.get_num_mpz_t()[0]._mp_size);
  for (int i = 0, e = std::abs(m.get_num_mpz_t()[0]._mp_size); i < e; ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    boost::hash_combine(result, m.get_num_mpz_t()[0]._mp_d[i]);
  }
  boost::hash_combine(result, m.get_den_mpz_t()[0]._mp_size);
  for (int i = 0, e = std::abs(m.get_den_mpz_t()[0]._mp_size); i < e; ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    boost::hash_combine(result, m.get_den_mpz_t()[0]._mp_d[i]);
  }
  return result;
}

} // end namespace core
} // end namespace ikos

namespace std {

/// \brief Hash for QNumber
template <>
struct hash< ikos::core::QNumber > {
  std::size_t operator()(const ikos::core::QNumber& n) const {
    return ikos::core::hash_value(n);
  }
};

} // end namespace std
