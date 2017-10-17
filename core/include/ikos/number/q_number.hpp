/**************************************************************************/ /**
 *
 * \file
 * \brief Unlimited precision floating-point
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

#ifndef IKOS_NUMBER_Q_NUMBER_HPP
#define IKOS_NUMBER_Q_NUMBER_HPP

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
#include <ikos/number/z_number.hpp>

namespace ikos {

/// \brief Checks wether T is a supported integral type or z_number
template < typename T >
struct is_supported_integral_or_z_number : public is_supported_integral< T > {};

template <>
struct is_supported_integral_or_z_number< const z_number& >
    : public std::true_type {};

namespace detail {

/// \brief Helper that returns either an integral type or a mpz_class
template < typename T >
struct z_num_adapter : public mpz_adapter< T > {};

template <>
struct z_num_adapter< const z_number& > {
  const mpz_class& operator()(const z_number& n) { return n.mpz(); }
};

} // end namespace detail

/// \brief Class for unlimited precision floating-point
class q_number {
private:
  mpq_class _n;

public:
  /// \brief Create a q_number from a string representation
  ///
  /// Interpret the null-terminated string str in the given base.
  ///
  /// If the string contains unsuitable characters for the given base, throw
  /// an exception number_error.
  ///
  /// The base may vary from 2 to 36.
  static q_number from_string(const char* str, int base = 10) {
    try {
      return q_number(mpq_class(str, base));
    } catch (std::invalid_argument&) {
      std::ostringstream buf;
      buf << "q_number: invalid conversion from string '" << str << "'";
      throw number_error(buf.str());
    }
  }

  /// \brief Create a q_number from a string representation
  ///
  /// Interpret the string str in the given base.
  ///
  /// If the string contains unsuitable characters for the given base, throw
  /// an exception number_error.
  ///
  /// The base may vary from 2 to 36.
  static q_number from_string(const std::string& str, int base = 10) {
    try {
      return q_number(mpq_class(str, base));
    } catch (std::invalid_argument&) {
      std::ostringstream buf;
      buf << "q_number: invalid conversion from string '" << str << "'";
      throw number_error(buf.str());
    }
  }

  /// \name Constructors
  /// @{

  /// \brief Default constructor that creates a q_number equals to 0
  q_number() = default;

  /// \brief Copy constructor
  q_number(const q_number&) = default;

  /// \brief Move constructor
  q_number(q_number&&) = default;

  /// \brief Create a q_number from a z_number
  explicit q_number(const z_number& n) : _n(n._n) {}

  /// \brief Create a q_number from a z_number
  explicit q_number(z_number&& n) : _n(std::move(n._n)) {}

  /// \brief Create a q_number from an integral type
  template < typename N,
             class = std::enable_if_t< is_supported_integral< N >::value > >
  explicit q_number(N n)
      : _n(detail::mpz_adapter< N >()(n)) {}

  /// \brief Create a q_number from a mpq_class
  explicit q_number(const mpq_class& n) : _n(n) {
    ikos_assert_msg(this->_n.get_den() != 0, "denominator is zero");
    this->_n.canonicalize();
  }

  /// \brief Create a q_number from a mpq_class
  explicit q_number(mpq_class&& n) : _n(std::move(n)) {
    ikos_assert_msg(this->_n.get_den() != 0, "denominator is zero");
    this->_n.canonicalize();
  }

  /// \brief Create a q_number from a numerator and a denominator
  template < typename N,
             typename D,
             class = std::enable_if_t< is_supported_integral< N >::value &&
                                       is_supported_integral< D >::value > >
  explicit q_number(N n, D d)
      : _n(detail::mpz_adapter< N >()(n), detail::mpz_adapter< D >()(d)) {
    ikos_assert_msg(this->_n.get_den() != 0, "denominator is zero");
    this->_n.canonicalize();
  }

  /// \brief Create a q_number from a numerator and a denominator
  explicit q_number(const z_number& n, const z_number& d) : _n(n._n, d._n) {
    ikos_assert_msg(this->_n.get_den() != 0, "denominator is zero");
    this->_n.canonicalize();
  }

  /// \brief Create a q_number from a numerator and a denominator
  explicit q_number(z_number&& n, z_number&& d)
      : _n(std::move(n._n), std::move(d._n)) {
    ikos_assert_msg(this->_n.get_den() != 0, "denominator is zero");
    this->_n.canonicalize();
  }

private:
  struct tag_normalized_t {};

  /// \brief Private constructor for normalized mpq_class
  q_number(const mpq_class& n, tag_normalized_t) : _n(n) {}

  /// \brief Private constructor for normalized mpq_class
  q_number(mpq_class&& n, tag_normalized_t) : _n(std::move(n)) {}

public:
  /// @}
  /// \name Assignment Operators
  /// @{

  /// \brief Copy assignment
  q_number& operator=(const q_number&) = default;

  /// \brief Move assignment
  q_number& operator=(q_number&&) noexcept = default;

  /// \brief Assignment for z_number
  q_number& operator=(const z_number& n) {
    this->_n = n._n;
    return *this;
  }

  /// \brief Assignment for z_number
  q_number& operator=(z_number&& n) noexcept {
    this->_n = std::move(n._n);
    return *this;
  }

  /// \brief Assignment for integral types
  template < typename N,
             typename = std::enable_if_t< is_supported_integral< N >::value > >
  q_number& operator=(N n) {
    this->_n = detail::mpz_adapter< N >()(n);
    return *this;
  }

  /// \brief Addition assignment
  q_number& operator+=(const q_number& x) {
    this->_n += x._n;
    return *this;
  }

  /// \brief Addition assignment with integral types or z_number
  template < typename N,
             class = std::enable_if_t<
                 is_supported_integral_or_z_number< N >::value > >
  q_number& operator+=(N x) {
    this->_n += detail::z_num_adapter< N >()(x);
    return *this;
  }

  /// \brief Subtraction assignment
  q_number& operator-=(const q_number& x) {
    this->_n -= x._n;
    return *this;
  }

  /// \brief Subtraction assignment with integral types or z_number
  template < typename N,
             class = std::enable_if_t<
                 is_supported_integral_or_z_number< N >::value > >
  q_number& operator-=(N x) {
    this->_n -= detail::z_num_adapter< N >()(x);
    return *this;
  }

  /// \brief Multiplication assignment
  q_number& operator*=(const q_number& x) {
    this->_n *= x._n;
    return *this;
  }

  /// \brief Multiplication assignment with integral types or z_number
  template < typename N,
             class = std::enable_if_t<
                 is_supported_integral_or_z_number< N >::value > >
  q_number& operator*=(N x) {
    this->_n *= detail::z_num_adapter< N >()(x);
    return *this;
  }

  /// \brief Division assignment
  q_number& operator/=(const q_number& x) {
    ikos_assert_msg(x._n != 0, "division by zero");
    this->_n /= x._n;
    return *this;
  }

  /// \brief Division assignment with integral types or z_number
  template < typename N,
             class = std::enable_if_t<
                 is_supported_integral_or_z_number< N >::value > >
  q_number& operator/=(N x) {
    ikos_assert_msg(x != 0, "division by zero");
    this->_n /= detail::z_num_adapter< N >()(x);
    return *this;
  }

  /// @}
  /// \name Unary Operators
  /// @{

  /// \brief Unary plus
  q_number operator+() const { return *this; }

  /// \brief Prefix increment
  q_number& operator++() {
    ++this->_n;
    return *this;
  }

  /// \brief Postfix increment
  q_number operator++(int) {
    q_number r(*this);
    ++this->_n;
    return r;
  }

  /// \brief Unary minus
  q_number operator-() const { return q_number(-this->_n); }

  /// \brief Prefix decrement
  q_number& operator--() {
    --this->_n;
    return *this;
  }

  /// \brief Postfix decrement
  q_number operator--(int) {
    q_number r(*this);
    --this->_n;
    return r;
  }

  /// @}
  /// \name Value generators
  /// @{

  /// \brief Get the numerator
  z_number numerator() const { return z_number(this->_n.get_num()); }

  /// \brief Get the denominator
  z_number denominator() const { return z_number(this->_n.get_den()); }

  /// \brief Round to upper integer
  z_number round_to_upper() const {
    const mpz_class& num = this->_n.get_num();
    const mpz_class& den = this->_n.get_den();
    z_number q(num / den);
    z_number r(num % den);
    if (r == 0 || this->_n < 0) {
      return q;
    } else {
      return q + 1;
    }
  }

  /// \brief Round to lower integer
  z_number round_to_lower() const {
    const mpz_class& num = this->_n.get_num();
    const mpz_class& den = this->_n.get_den();
    z_number q(num / den);
    z_number r(num % den);
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

  /// \brief Return a string of the q_number in the given base
  ///
  /// The base can vary from 2 to 36, or from -2 to -36
  std::string str(int base = 10) const { return this->_n.get_str(base); }

  /// @}
  /// \name Binary Operators
  /// @{

  /// \brief Addition
  friend q_number operator+(const q_number& lhs, const q_number& rhs) {
    return q_number(lhs._n + rhs._n, q_number::tag_normalized_t{});
  }

  /// \brief Addition with integral types or z_number
  template < typename T,
             class = std::enable_if_t<
                 is_supported_integral_or_z_number< T >::value > >
  friend q_number operator+(const q_number& lhs, T rhs) {
    return q_number(lhs._n + detail::z_num_adapter< T >()(rhs),
                    q_number::tag_normalized_t{});
  }

  /// \brief Addition with integral types or z_number
  template < typename T,
             class = std::enable_if_t<
                 is_supported_integral_or_z_number< T >::value > >
  friend q_number operator+(T lhs, const q_number& rhs) {
    return q_number(detail::z_num_adapter< T >()(lhs) + rhs._n,
                    q_number::tag_normalized_t{});
  }

  /// \brief Subtraction
  friend q_number operator-(const q_number& lhs, const q_number& rhs) {
    return q_number(lhs._n - rhs._n, q_number::tag_normalized_t{});
  }

  /// \brief Subtraction with integral types or z_number
  template < typename T,
             class = std::enable_if_t<
                 is_supported_integral_or_z_number< T >::value > >
  friend q_number operator-(const q_number& lhs, T rhs) {
    return q_number(lhs._n - detail::z_num_adapter< T >()(rhs),
                    q_number::tag_normalized_t{});
  }

  /// \brief Subtraction with integral types or z_number
  template < typename T,
             class = std::enable_if_t<
                 is_supported_integral_or_z_number< T >::value > >
  friend q_number operator-(T lhs, const q_number& rhs) {
    return q_number(detail::z_num_adapter< T >()(lhs) - rhs._n,
                    q_number::tag_normalized_t{});
  }

  /// \brief Multiplication
  friend q_number operator*(const q_number& lhs, const q_number& rhs) {
    return q_number(lhs._n * rhs._n, q_number::tag_normalized_t{});
  }

  /// \brief Multiplication with integral types or z_number
  template < typename T,
             class = std::enable_if_t<
                 is_supported_integral_or_z_number< T >::value > >
  friend q_number operator*(const q_number& lhs, T rhs) {
    return q_number(lhs._n * detail::z_num_adapter< T >()(rhs),
                    q_number::tag_normalized_t{});
  }

  /// \brief Multiplication with integral types or z_number
  template < typename T,
             class = std::enable_if_t<
                 is_supported_integral_or_z_number< T >::value > >
  friend q_number operator*(T lhs, const q_number& rhs) {
    return q_number(detail::z_num_adapter< T >()(lhs) * rhs._n,
                    q_number::tag_normalized_t{});
  }

  /// \brief Division
  friend q_number operator/(const q_number& lhs, const q_number& rhs) {
    ikos_assert_msg(rhs._n != 0, "division by zero");
    return q_number(lhs._n / rhs._n, q_number::tag_normalized_t{});
  }

  /// \brief Division with integral types or z_number
  template < typename T,
             class = std::enable_if_t<
                 is_supported_integral_or_z_number< T >::value > >
  friend q_number operator/(const q_number& lhs, T rhs) {
    ikos_assert_msg(rhs != 0, "division by zero");
    return q_number(lhs._n / detail::z_num_adapter< T >()(rhs),
                    q_number::tag_normalized_t{});
  }

  /// \brief Division with integral types or z_number
  template < typename T,
             class = std::enable_if_t<
                 is_supported_integral_or_z_number< T >::value > >
  friend q_number operator/(T lhs, const q_number& rhs) {
    ikos_assert_msg(rhs._n != 0, "division by zero");
    return q_number(detail::z_num_adapter< T >()(lhs) / rhs._n,
                    q_number::tag_normalized_t{});
  }

  /// @}

  friend std::istream& operator>>(std::istream& i, q_number& n);

}; // end class q_number

/// \name Comparison Operators
/// @{

/// \brief Equality operator
inline bool operator==(const q_number& lhs, const q_number& rhs) {
  return lhs.mpq() == rhs.mpq();
}

/// \brief Equality operator with integral types or z_number
template <
    typename T,
    class = std::enable_if_t< is_supported_integral_or_z_number< T >::value > >
inline bool operator==(const q_number& lhs, T rhs) {
  return lhs.mpq() == detail::z_num_adapter< T >()(rhs);
}

/// \brief Equality operator with integral types or z_number
template <
    typename T,
    class = std::enable_if_t< is_supported_integral_or_z_number< T >::value > >
inline bool operator==(T lhs, const q_number& rhs) {
  return detail::z_num_adapter< T >()(lhs) == rhs.mpq();
}

/// \brief Inequality operator
inline bool operator!=(const q_number& lhs, const q_number& rhs) {
  return lhs.mpq() != rhs.mpq();
}

/// \brief Inequality operator with integral types or z_number
template <
    typename T,
    class = std::enable_if_t< is_supported_integral_or_z_number< T >::value > >
inline bool operator!=(const q_number& lhs, T rhs) {
  return lhs.mpq() != detail::z_num_adapter< T >()(rhs);
}

/// \brief Inequality operator with integral types or z_number
template <
    typename T,
    class = std::enable_if_t< is_supported_integral_or_z_number< T >::value > >
inline bool operator!=(T lhs, const q_number& rhs) {
  return detail::z_num_adapter< T >()(lhs) != rhs.mpq();
}

/// \brief Less than comparison
inline bool operator<(const q_number& lhs, const q_number& rhs) {
  return lhs.mpq() < rhs.mpq();
}

/// \brief Less than comparison with integral types or z_number
template <
    typename T,
    class = std::enable_if_t< is_supported_integral_or_z_number< T >::value > >
inline bool operator<(const q_number& lhs, T rhs) {
  return lhs.mpq() < detail::z_num_adapter< T >()(rhs);
}

/// \brief Less than comparison with integral types or z_number
template <
    typename T,
    class = std::enable_if_t< is_supported_integral_or_z_number< T >::value > >
inline bool operator<(T lhs, const q_number& rhs) {
  return detail::z_num_adapter< T >()(lhs) < rhs.mpq();
}

/// \brief Less or equal comparison
inline bool operator<=(const q_number& lhs, const q_number& rhs) {
  return lhs.mpq() <= rhs.mpq();
}

/// \brief Less or equal comparison with integral types or z_number
template <
    typename T,
    class = std::enable_if_t< is_supported_integral_or_z_number< T >::value > >
inline bool operator<=(const q_number& lhs, T rhs) {
  return lhs.mpq() <= detail::z_num_adapter< T >()(rhs);
}

/// \brief Less or equal comparison with integral types or z_number
template <
    typename T,
    class = std::enable_if_t< is_supported_integral_or_z_number< T >::value > >
inline bool operator<=(T lhs, const q_number& rhs) {
  return detail::z_num_adapter< T >()(lhs) <= rhs.mpq();
}

/// \brief Greater than comparison
inline bool operator>(const q_number& lhs, const q_number& rhs) {
  return lhs.mpq() > rhs.mpq();
}

/// \brief Greater than comparison with integral types or z_number
template <
    typename T,
    class = std::enable_if_t< is_supported_integral_or_z_number< T >::value > >
inline bool operator>(const q_number& lhs, T rhs) {
  return lhs.mpq() > detail::z_num_adapter< T >()(rhs);
}

/// \brief Greater than comparison with integral types or z_number
template <
    typename T,
    class = std::enable_if_t< is_supported_integral_or_z_number< T >::value > >
inline bool operator>(T lhs, const q_number& rhs) {
  return detail::z_num_adapter< T >()(lhs) > rhs.mpq();
}

/// \brief Greater or equal comparison
inline bool operator>=(const q_number& lhs, const q_number& rhs) {
  return lhs.mpq() >= rhs.mpq();
}

/// \brief Greater or equal comparison with integral types or z_number
template <
    typename T,
    class = std::enable_if_t< is_supported_integral_or_z_number< T >::value > >
inline bool operator>=(const q_number& lhs, T rhs) {
  return lhs.mpq() >= detail::z_num_adapter< T >()(rhs);
}

/// \brief Greater or equal comparison with integral types or z_number
template <
    typename T,
    class = std::enable_if_t< is_supported_integral_or_z_number< T >::value > >
inline bool operator>=(T lhs, const q_number& rhs) {
  return detail::z_num_adapter< T >()(lhs) >= rhs.mpq();
}

/// @}
/// \name Input / Output
/// @{

/// \brief Write a q_number on a stream, in base 10
inline std::ostream& operator<<(std::ostream& o, const q_number& n) {
  o << n.mpq();
  return o;
}

/// \brief Read a q_number from a stream, in base 10
inline std::istream& operator>>(std::istream& i, q_number& n) {
  i >> n._n;
  return i;
}

/// @}

} // end namespace ikos

namespace std {

/// \brief Hash for q_number
template <>
struct hash< ikos::q_number > {
  std::size_t operator()(const ikos::q_number& n) const {
    const mpq_class& m = n.mpq();
    std::size_t result = 0;
    boost::hash_combine(result, m.get_num_mpz_t()[0]._mp_size);
    for (int i = 0; i < std::abs(m.get_num_mpz_t()[0]._mp_size); ++i) {
      boost::hash_combine(result, m.get_num_mpz_t()[0]._mp_d[i]);
    }
    boost::hash_combine(result, m.get_den_mpz_t()[0]._mp_size);
    for (int i = 0; i < std::abs(m.get_den_mpz_t()[0]._mp_size); ++i) {
      boost::hash_combine(result, m.get_den_mpz_t()[0]._mp_d[i]);
    }
    return result;
  }
};

} // end namespace std

#endif // IKOS_NUMBER_Q_NUMBER_HPP
