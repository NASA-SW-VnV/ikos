/*******************************************************************************
 *
 * \file
 * \brief Machine integer class
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2018-2019 United States Government as represented by the
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

#include <ikos/core/number/compatibility.hpp>
#include <ikos/core/number/signedness.hpp>
#include <ikos/core/number/supported_integral.hpp>
#include <ikos/core/number/z_number.hpp>
#include <ikos/core/support/compiler.hpp>

namespace ikos {
namespace core {

/// \brief Class for arbitrary precision machine integers
class MachineInt {
private:
  /// If bit-width <= 64, store directly the integer,
  /// Otherwise use a pointer on a ZNumber.
  union {
    uint64_t i; /// Used to store the <= 64 bits integer value.
    ZNumber* p; /// Used to store the >64 bits integer value.
  } _n;
  uint64_t _bit_width;
  Signedness _sign;

private:
  /// \brief The biggest uint64_t
  static const uint64_t ONES = ~uint64_t(0);

private:
  /// \brief Return true if bit-width <= 64
  bool is_small() const { return ikos_likely(this->_bit_width <= 64); }

  /// \brief Return true if bit-width > 64
  bool is_large() const { return !this->is_small(); }

  /// \brief Return 2**n
  static ZNumber power_of_2(uint64_t n) { return ZNumber(1) << n; }

  /// \brief Return 2**n
  static ZNumber power_of_2(const ZNumber& n) { return ZNumber(1) << n; }

  /// \brief Normalize the machine integer
  void normalize() {
    if (this->is_small()) {
      // Mask out the high bits
      uint64_t mask = ONES >> (64 - this->_bit_width);
      this->_n.i &= mask;
    } else {
      if (*this->_n.p == 0) {
        // always normalized
        return;
      } else if (this->is_signed()) {
        // n needs to be within [-2**(n-1), 2**(n-1)-1]
        *this->_n.p = mod(*this->_n.p + power_of_2(this->_bit_width - 1),
                          power_of_2(this->_bit_width)) -
                      power_of_2(this->_bit_width - 1);
      } else {
        // n needs to be within [0, 2**n-1]
        *this->_n.p = mod(*this->_n.p, power_of_2(this->_bit_width));
      }
    }
  }

public:
  /// \name Constructors
  /// @{

  /// \brief Default constructor
  MachineInt() = delete;

  /// \brief Create a machine integer from an integral type
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  MachineInt(T n, uint64_t bit_width, Signedness sign)
      : _bit_width(bit_width), _sign(sign) {
    ikos_assert_msg(bit_width > 0, "invalid bit width");

    if (this->is_small()) {
      this->_n.i = static_cast< uint64_t >(n);
    } else {
      this->_n.p = new ZNumber(n);
    }

    this->normalize();
  }

  /// \brief Create a machine integer from a ZNumber
  MachineInt(const ZNumber& n, uint64_t bit_width, Signedness sign)
      : _bit_width(bit_width), _sign(sign) {
    ikos_assert_msg(bit_width > 0, "invalid bit width");

    if (this->is_small()) {
      ZNumber m = mod(n, power_of_2(this->_bit_width));
      this->_n.i = m.to< uint64_t >();
    } else {
      this->_n.p = new ZNumber(n);
    }

    this->normalize();
  }

private:
  struct NormalizedTag {};

  /// \brief Private constructor for normalized integers
  MachineInt(uint64_t n, uint64_t bit_width, Signedness sign, NormalizedTag)
      : _bit_width(bit_width), _sign(sign) {
    ikos_assert_msg(bit_width > 0, "invalid bit width");

    if (this->is_small()) {
      this->_n.i = n;
    } else {
      this->_n.p = new ZNumber(n);
    }
  }

  /// \brief Private constructor for normalized integers
  MachineInt(const ZNumber& n,
             uint64_t bit_width,
             Signedness sign,
             NormalizedTag)
      : _bit_width(bit_width), _sign(sign) {
    ikos_assert_msg(bit_width > 0, "invalid bit width");

    if (this->is_small()) {
      this->_n.i = n.to< uint64_t >();
    } else {
      this->_n.p = new ZNumber(n);
    }
  }

public:
  /// \brief Copy constructor
  MachineInt(const MachineInt& o) : _bit_width(o._bit_width), _sign(o._sign) {
    if (o.is_small()) {
      this->_n.i = o._n.i;
    } else {
      this->_n.p = new ZNumber(*o._n.p);
    }
  }

  /// \brief Move constructor
  MachineInt(MachineInt&& o) noexcept
      : _n(o._n), _bit_width(o._bit_width), _sign(o._sign) {
    o._bit_width = 0; // do not delete o._n.p
  }

  /// \brief Destructor
  ~MachineInt() {
    if (this->is_large()) {
      delete this->_n.p;
    }
  }

  /// \brief Create the minimum machine integer for the given bit width and sign
  static MachineInt min(uint64_t bit_width, Signedness sign) {
    ikos_assert_msg(bit_width > 0, "invalid bit width");

    if (sign == Signed) {
      if (ikos_likely(bit_width <= 64)) {
        return MachineInt(uint64_t(1) << (bit_width - 1),
                          bit_width,
                          sign,
                          NormalizedTag{});
      } else {
        return MachineInt(-power_of_2(bit_width - 1),
                          bit_width,
                          sign,
                          NormalizedTag{});
      }
    } else {
      return MachineInt(0, bit_width, sign, NormalizedTag{});
    }
  }

  /// \brief Create the maximum machine integer for the given bit width and sign
  static MachineInt max(uint64_t bit_width, Signedness sign) {
    ikos_assert_msg(bit_width > 0, "invalid bit width");

    if (sign == Signed) {
      if (bit_width == 1) {
        return MachineInt(0, bit_width, sign, NormalizedTag{});
      } else if (bit_width <= 64) {
        return MachineInt(ONES >> (65 - bit_width),
                          bit_width,
                          sign,
                          NormalizedTag{});
      } else {
        return MachineInt(power_of_2(bit_width - 1) - 1,
                          bit_width,
                          sign,
                          NormalizedTag{});
      }
    } else {
      if (ikos_likely(bit_width <= 64)) {
        return MachineInt(ONES >> (64 - bit_width),
                          bit_width,
                          sign,
                          NormalizedTag{});
      } else {
        return MachineInt(power_of_2(bit_width) - 1,
                          bit_width,
                          sign,
                          NormalizedTag{});
      }
    }
  }

  /// \brief Create the null machine integer for the given bit width and sign
  static MachineInt zero(uint64_t bit_width, Signedness sign) {
    ikos_assert_msg(bit_width > 0, "invalid bit width");

    return MachineInt(0, bit_width, sign, NormalizedTag{});
  }

  /// \brief Create the machine integer with all bits set to 1 for the given bit
  /// width and sign
  static MachineInt all_ones(uint64_t bit_width, Signedness sign) {
    ikos_assert_msg(bit_width > 0, "invalid bit width");

    if (ikos_likely(bit_width <= 64)) {
      return MachineInt(ONES >> (64 - bit_width),
                        bit_width,
                        sign,
                        NormalizedTag{});
    } else {
      if (sign == Signed) {
        return MachineInt(ZNumber(-1), bit_width, sign, NormalizedTag{});
      } else {
        return MachineInt(power_of_2(bit_width) - 1,
                          bit_width,
                          sign,
                          NormalizedTag{});
      }
    }
  }

  /// @}
  /// \name Assignment Operators
  /// @{

  /// \brief Copy assignment
  MachineInt& operator=(const MachineInt& o) {
    if (this == &o) {
      return *this;
    }

    if (this->is_small()) {
      if (o.is_small()) {
        this->_n.i = o._n.i;
      } else {
        this->_n.p = new ZNumber(*o._n.p);
      }
    } else {
      if (o.is_small()) {
        delete this->_n.p;
        this->_n.i = o._n.i;
      } else {
        *this->_n.p = *o._n.p;
      }
    }

    this->_bit_width = o._bit_width;
    this->_sign = o._sign;
    return *this;
  }

  /// \brief Move assignment
  MachineInt& operator=(MachineInt&& o) noexcept {
    if (this == &o) {
      return *this;
    }

    if (this->is_large()) {
      delete this->_n.p;
    }

    this->_n = o._n;
    this->_bit_width = o._bit_width;
    this->_sign = o._sign;
    o._bit_width = 0; // do not delete o._n.p
    return *this;
  }

  /// \brief Assignment for integral types
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  MachineInt& operator=(T n) {
    if (this->is_small()) {
      this->_n.i = static_cast< uint64_t >(n);
    } else {
      *this->_n.p = n;
    }
    this->normalize();
    return *this;
  }

  /// \brief Addition assignment
  MachineInt& operator+=(const MachineInt& x) {
    assert_compatible(*this, x);
    if (this->is_small()) {
      this->_n.i += x._n.i;
    } else {
      *this->_n.p += *x._n.p;
    }
    this->normalize();
    return *this;
  }

  /// \brief Subtraction assignment
  MachineInt& operator-=(const MachineInt& x) {
    assert_compatible(*this, x);
    if (this->is_small()) {
      this->_n.i -= x._n.i;
    } else {
      *this->_n.p -= *x._n.p;
    }
    this->normalize();
    return *this;
  }

  /// \brief Multiplication assignment
  MachineInt& operator*=(const MachineInt& x) {
    assert_compatible(*this, x);
    if (this->is_small()) {
      this->_n.i *= x._n.i;
    } else {
      *this->_n.p *= *x._n.p;
    }
    this->normalize();
    return *this;
  }

  /// \brief Division assignment
  MachineInt& operator/=(const MachineInt& x) {
    *this = div(*this, x);
    return *this;
  }

  /// \brief Remainder assignment
  MachineInt& operator%=(const MachineInt& x) {
    *this = rem(*this, x);
    return *this;
  }

  /// @}
  /// \name Value Characterization Functions
  /// @{

  /// \brief Return the bit width of the integer
  uint64_t bit_width() const { return this->_bit_width; }

  /// \brief Return the signedness (Signed or Unsigned) of the integer
  Signedness sign() const { return this->_sign; }

  bool is_signed() const { return this->_sign == Signed; }
  bool is_unsigned() const { return this->_sign == Unsigned; }

  /// @}
  /// \name Value tests
  /// @{

  /// \brief Return true if this is the minimum machine integer
  bool is_min() const {
    if (this->is_signed()) {
      if (this->is_small()) {
        return this->_n.i == (uint64_t(1) << (this->_bit_width - 1));
      } else {
        return *this->_n.p == -power_of_2(this->_bit_width - 1);
      }
    } else {
      return this->is_zero();
    }
  }

  /// \brief Return true if this is the maximum machine integer
  bool is_max() const {
    if (this->is_signed()) {
      if (this->bit_width() == 1) {
        return this->_n.i == 0;
      } else if (this->is_small()) {
        return this->_n.i == (ONES >> (65 - this->_bit_width));
      } else {
        return *this->_n.p == (power_of_2(this->_bit_width - 1) - 1);
      }
    } else {
      if (this->is_small()) {
        return this->_n.i == (ONES >> (64 - this->_bit_width));
      } else {
        return *this->_n.p == (power_of_2(this->_bit_width) - 1);
      }
    }
  }

  /// \brief Return true if the machine integer is 0
  bool is_zero() const {
    if (this->is_small()) {
      return this->_n.i == 0;
    } else {
      return *this->_n.p == 0;
    }
  }

  /// \brief Return the high bit (the sign bit) as a bool
  bool high_bit() const {
    if (this->is_small()) {
      return (this->_n.i >> (this->_bit_width - 1)) != 0;
    } else {
      if (this->is_signed()) {
        return *this->_n.p < 0;
      } else {
        return *this->_n.p >= power_of_2(this->_bit_width - 1);
      }
    }
  }

  /// \brief Return true if the machine integer is strictly smaller than 0
  ///
  /// Always false for unsigned integers
  bool is_negative() const {
    if (this->is_signed()) {
      return this->high_bit();
    } else {
      return false;
    }
  }

  /// \brief Return true if the machine integer is greater or equal to 0
  ///
  /// Always true for unsigned integers
  bool is_non_negative() const { return !this->is_negative(); }

  /// \brief Return true if the machine integer is strictly greater than 0
  bool is_strictly_positive() const {
    if (this->is_small()) {
      if (this->is_signed()) {
        return !this->high_bit() && this->_n.i != 0;
      } else {
        return this->_n.i != 0;
      }
    } else {
      return *this->_n.p > 0;
    }
  }

  /// \brief Return true if all bits are set
  bool all_ones() const {
    if (this->is_small()) {
      return this->_n.i == (ONES >> (64 - this->_bit_width));
    } else {
      if (this->is_signed()) {
        return *this->_n.p == -1;
      } else {
        return *this->_n.p == (power_of_2(this->_bit_width) - 1);
      }
    }
  }

private:
  /// \brief Return the number of leading '0' bits in a uint64_t
  static uint64_t leading_zeros(uint64_t n) {
    if (n == 0) {
      return 64;
    }

#if __has_builtin(__builtin_clzll) || IKOS_GNUC_PREREQ(4, 0, 0)
    // NOLINTNEXTLINE(google-runtime-int)
    if (std::is_same< uint64_t, unsigned long long >::value) {
      return static_cast< uint64_t >(__builtin_clzll(n));
    }
#endif

    // Use bisection
    uint64_t zeros = 0;
    for (uint64_t shift = 64U >> 1U; shift != 0U; shift >>= 1U) {
      uint64_t tmp = n >> shift;
      if (tmp != 0) {
        n = tmp;
      } else {
        zeros |= shift;
      }
    }
    return zeros;
  }

  /// \brief Return the number of leading '0' bits in a ZNumber
  static uint64_t leading_zeros(ZNumber n, uint64_t bit_width) {
    if (n == 0) {
      return bit_width;
    } else if (n < 0) {
      return 0;
    }

    // Use bisection
    uint64_t zeros = 0;
    for (uint64_t shift = bit_width >> 1U; shift != 0U; shift >>= 1U) {
      ZNumber tmp = n >> shift;
      if (tmp != 0) {
        n = tmp;
      } else {
        zeros |= shift;
      }
    }
    return zeros;
  }

  /// \brief Return the number of leading '1' bits in a uint64_t
  static uint64_t leading_ones(uint64_t n) {
    return leading_zeros(~n);
  }

  /// \brief Return the number of leading '1' bits in a ZNumber
  static uint64_t leading_ones(ZNumber n, uint64_t bit_width) {
    n = mod(n, power_of_2(bit_width));
    n = n ^ (power_of_2(bit_width) - 1);
    return leading_zeros(n, bit_width);
  }

public:
  /// \brief Return the number of leading '0' bits
  uint64_t leading_zeros() const {
    if (this->is_small()) {
      uint64_t unused_bits = 64 - this->_bit_width;
      return leading_zeros(this->_n.i) - unused_bits;
    } else {
      return leading_zeros(*this->_n.p, this->_bit_width);
    }
  }

  /// \brief Return the number of leading '1' bits
  uint64_t leading_ones() const {
    if (this->is_small()) {
      return leading_ones(this->_n.i << (64 - this->_bit_width));
    } else {
      return leading_ones(*this->_n.p, this->_bit_width);
    }
  }

private:
  /// Sign-extend the number in the bottom `bit-width` bits of `n` to a 64-bit
  /// signed integer
  static int64_t sign_extend_64(uint64_t n, uint64_t bit_width) {
    ikos_assert_msg(bit_width > 0 && bit_width <= 64, "invalid bit-width");

    // TODO(marthaud): this is implementation-defined.
    // See https://stackoverflow.com/a/7602006
    // See https://stackoverflow.com/a/13208789 for proper solution
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return int64_t(n << (64U - bit_width)) >> (64U - bit_width);
  }

public:
  /// @}
  /// \name Conversion Functions
  /// @{

  /// \brief Return the machine integer as a ZNumber
  ///
  /// If this is a signed integer, the result is within [-2**(n-1), 2**(n-1)-1]
  /// Otherwise, the result is within [0, 2**n-1]
  ZNumber to_z_number() const {
    if (this->is_small()) {
      if (this->is_signed()) {
        return ZNumber(sign_extend_64(this->_n.i, this->_bit_width));
      } else {
        return ZNumber(this->_n.i);
      }
    } else {
      return *this->_n.p;
    }
  }

  /// \brief Return true if the machine integer fits in the given integer type
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  bool fits() const {
    if (this->is_small()) {
      if (this->is_signed()) {
        int64_t n = sign_extend_64(this->_n.i, this->_bit_width);

        if (std::numeric_limits< T >::is_signed) {
          return int64_t(std::numeric_limits< T >::min()) <= n &&
                 n <= int64_t(std::numeric_limits< T >::max());
        } else {
          return int64_t(0) <= n &&
                 uint64_t(n) <= uint64_t(std::numeric_limits< T >::max());
        }
      } else {
        uint64_t n = this->_n.i;

        if (std::numeric_limits< T >::is_signed) {
          return n <= uint64_t(std::numeric_limits< T >::max());
        } else {
          return uint64_t(std::numeric_limits< T >::min()) <= n &&
                 n <= uint64_t(std::numeric_limits< T >::max());
        }
      }
    } else {
      return this->_n.p->fits< T >();
    }
  }

  /// \brief Return the machine integer as the given integer type
  template < typename T,
             class = std::enable_if_t< IsSupportedIntegral< T >::value > >
  T to() const {
    ikos_assert_msg(this->fits< T >(), "does not fit");

    if (this->is_small()) {
      if (this->is_signed()) {
        return static_cast< T >(sign_extend_64(this->_n.i, this->_bit_width));
      } else {
        return static_cast< T >(this->_n.i);
      }
    } else {
      return this->_n.p->to< T >();
    }
  }

  /// \brief Return a string representation of the integer in the given base
  ///
  /// The base can vary from 2 to 36, or from -2 to -36
  std::string str(int base = 10) const {
    if (this->is_small()) {
      if (this->is_signed()) {
        return std::to_string(sign_extend_64(this->_n.i, this->_bit_width));
      } else {
        return std::to_string(this->_n.i);
      }
    } else {
      return this->_n.p->str(base);
    }
  }

  /// @}
  /// \name Unary Operators
  /// @{

  // \brief Set the machine integer to the minimum integer
  void set_min() {
    if (this->is_signed()) {
      if (this->is_small()) {
        this->_n.i = uint64_t(1) << (this->_bit_width - 1);
      } else {
        *this->_n.p = -power_of_2(this->_bit_width - 1);
      }
    } else {
      this->set_zero();
    }
  }

  // \brief Set the machine integer to the maximum integer
  void set_max() {
    if (this->is_signed()) {
      if (this->_bit_width == 1) {
        this->_n.i = 0;
      } else if (this->is_small()) {
        this->_n.i = ONES >> (65 - this->_bit_width);
      } else {
        *this->_n.p = power_of_2(this->_bit_width - 1) - 1;
      }
    } else {
      if (this->is_small()) {
        this->_n.i = ONES >> (64 - this->_bit_width);
      } else {
        *this->_n.p = power_of_2(this->_bit_width) - 1;
      }
    }
  }

  // \brief Set the machine integer to zero
  void set_zero() {
    if (this->is_small()) {
      this->_n.i = 0;
    } else {
      *this->_n.p = 0;
    }
  }

  /// \brief Unary plus
  const MachineInt& operator+() const {
    return *this;
  }

  /// \brief Prefix increment
  MachineInt& operator++() {
    if (this->is_small()) {
      ++this->_n.i;
    } else {
      ++(*this->_n.p);
    }
    this->normalize();
    return *this;
  }

  /// \brief Postfix increment
  const MachineInt operator++(int) {
    MachineInt r(*this);
    if (this->is_small()) {
      ++this->_n.i;
    } else {
      ++(*this->_n.p);
    }
    this->normalize();
    return r;
  }

  /// \brief Unary minus
  const MachineInt operator-() const {
    if (this->is_small()) {
      return MachineInt((~this->_n.i) + 1, this->_bit_width, this->_sign);
    } else {
      return MachineInt(-(*this->_n.p), this->_bit_width, this->_sign);
    }
  }

  /// \brief Negate the machine integer
  void negate() {
    if (this->is_small()) {
      this->_n.i = (~this->_n.i) + 1;
    } else {
      *this->_n.p = -(*this->_n.p);
    }
    this->normalize();
  }

  /// \brief Bitwise not
  const MachineInt operator~() const {
    if (this->is_small()) {
      return MachineInt(~this->_n.i, this->_bit_width, this->_sign);
    } else {
      return MachineInt(-(*this->_n.p) - 1, this->_bit_width, this->_sign);
    }
  }

  /// \brief Flip all the bits
  ///
  /// This is the bitwise not operator.
  void flip_bits() {
    if (this->is_small()) {
      this->_n.i = ~this->_n.i;
    } else {
      *this->_n.p = -(*this->_n.p) - 1;
    }
    this->normalize();
  }

  /// \brief Prefix decrement
  MachineInt& operator--() {
    if (this->is_small()) {
      --this->_n.i;
    } else {
      --(*this->_n.p);
    }
    this->normalize();
    return *this;
  }

  /// \brief Postfix decrement
  const MachineInt operator--(int) {
    MachineInt r(*this);
    if (this->is_small()) {
      --this->_n.i;
    } else {
      --(*this->_n.p);
    }
    this->normalize();
    return r;
  }

  /// \brief Truncate the machine integer to the given bit width
  MachineInt trunc(uint64_t bit_width) const {
    ikos_assert(this->_bit_width > bit_width);

    if (this->is_small()) {
      return MachineInt(this->_n.i, bit_width, this->_sign);
    } else {
      return MachineInt(*this->_n.p, bit_width, this->_sign);
    }
  }

  /// \brief Extend the machine integer to the given bit width
  MachineInt ext(uint64_t bit_width) const {
    ikos_assert(this->_bit_width < bit_width);

    if (this->is_small()) {
      if (this->is_signed()) {
        return MachineInt(sign_extend_64(this->_n.i, this->_bit_width),
                          bit_width,
                          this->_sign);
      } else {
        return MachineInt(this->_n.i, bit_width, this->_sign, NormalizedTag{});
      }
    } else {
      return MachineInt(*this->_n.p, bit_width, this->_sign, NormalizedTag{});
    }
  }

  /// \brief Change the machine integer sign (bitcast)
  MachineInt sign_cast(Signedness sign) const {
    ikos_assert(this->_sign != sign);

    if (this->is_small()) {
      return MachineInt(this->_n.i, this->_bit_width, sign, NormalizedTag{});
    } else {
      return MachineInt(*this->_n.p, this->_bit_width, sign);
    }
  }

  /// \brief Cast the machine integer to the given bit width and sign
  ///
  /// This is equivalent to trunc()/ext() + sign_cast() (in this specific order)
  MachineInt cast(uint64_t bit_width, Signedness sign) const {
    if (this->is_small()) {
      if (this->is_signed()) {
        return MachineInt(sign_extend_64(this->_n.i, this->_bit_width),
                          bit_width,
                          sign);
      } else {
        return MachineInt(this->_n.i, bit_width, sign);
      }
    } else {
      return MachineInt(*this->_n.p, bit_width, sign);
    }
  }

  /// @}

private:
  // Helpers for binary operations

  /// \brief Check if an overflow occurred
  static bool check_overflow(const ZNumber& n,
                             uint64_t bit_width,
                             Signedness sign) {
    if (sign == Signed) {
      return n >= power_of_2(bit_width - 1);
    } else {
      return n >= power_of_2(bit_width);
    }
  }

  /// \brief Check if an underflow occurred
  static bool check_underflow(const ZNumber& n,
                              uint64_t bit_width,
                              Signedness sign) {
    if (sign == Signed) {
      return n < -power_of_2(bit_width - 1);
    } else {
      return n < 0;
    }
  }

public:
  // Friends

  friend MachineInt add(const MachineInt& lhs,
                        const MachineInt& rhs,
                        bool& overflow);

  friend MachineInt add(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt operator+(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt sub(const MachineInt& lhs,
                        const MachineInt& rhs,
                        bool& overflow);

  friend MachineInt sub(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt operator-(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt mul(const MachineInt& lhs,
                        const MachineInt& rhs,
                        bool& overflow);

  friend MachineInt mul(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt operator*(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt div(const MachineInt& lhs,
                        const MachineInt& rhs,
                        bool& overflow,
                        bool& exact);

  friend MachineInt div(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt operator/(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt rem(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt operator%(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt mod(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt shl(const MachineInt& lhs,
                        const MachineInt& rhs,
                        bool& overflow);

  friend MachineInt shl(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt lshr(const MachineInt& lhs,
                         const MachineInt& rhs,
                         bool& exact);

  friend MachineInt lshr(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt ashr(const MachineInt& lhs,
                         const MachineInt& rhs,
                         bool& exact);

  friend MachineInt ashr(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt and_(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt operator&(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt or_(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt operator|(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt xor_(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt operator^(const MachineInt& lhs, const MachineInt& rhs);

  friend MachineInt gcd(const MachineInt& a, const MachineInt& b);

  friend bool operator==(const MachineInt& lhs, const MachineInt& rhs);

  friend bool operator!=(const MachineInt& lhs, const MachineInt& rhs);

  friend bool operator<(const MachineInt& lhs, const MachineInt& rhs);

  friend bool operator<=(const MachineInt& lhs, const MachineInt& rhs);

  friend bool operator>(const MachineInt& lhs, const MachineInt& rhs);

  friend bool operator>=(const MachineInt& lhs, const MachineInt& rhs);

  friend std::ostream& operator<<(std::ostream& o, const MachineInt& n);

  friend std::size_t hash_value(const MachineInt&);

}; // end class MachineInt

/// \name Binary Operators
/// @{

/// \brief Assert that the shift count is between 0 and bit_width - 1
inline void assert_shift(const MachineInt& n, uint64_t bit_width) {
  ikos_assert_msg(n.is_non_negative(), "shift count is negative");
  ikos_assert_msg(n.to_z_number() < bit_width, "shift count is too big");
  ikos_ignore(n);
  ikos_ignore(bit_width);
}

/// \brief Addition
///
/// Returns the sum of the operands, with wrapping.
inline MachineInt add(const MachineInt& lhs,
                      const MachineInt& rhs,
                      bool& overflow) {
  assert_compatible(lhs, rhs);
  if (lhs.is_small()) {
    MachineInt result(lhs._n.i + rhs._n.i, lhs._bit_width, lhs._sign);
    if (lhs.is_signed()) {
      overflow = lhs.is_non_negative() == rhs.is_non_negative() &&
                 result.is_non_negative() != lhs.is_non_negative();
    } else {
      overflow = result < rhs;
    }
    return result;
  } else {
    ZNumber n = (*lhs._n.p) + (*rhs._n.p);
    overflow = MachineInt::check_overflow(n, lhs._bit_width, lhs._sign) ||
               MachineInt::check_underflow(n, lhs._bit_width, lhs._sign);
    return MachineInt(n, lhs._bit_width, lhs._sign);
  }
}

/// \brief Addition
///
/// Returns the sum of the operands, with wrapping.
inline MachineInt add(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_small()) {
    return MachineInt(lhs._n.i + rhs._n.i, lhs._bit_width, lhs._sign);
  } else {
    return MachineInt((*lhs._n.p) + (*rhs._n.p), lhs._bit_width, lhs._sign);
  }
}

/// \brief Addition
///
/// Returns the sum of the operands, with wrapping.
inline MachineInt operator+(const MachineInt& lhs, const MachineInt& rhs) {
  return add(lhs, rhs);
}

/// \brief Subtraction
///
/// Returns the difference of the operands, with wrapping.
inline MachineInt sub(const MachineInt& lhs,
                      const MachineInt& rhs,
                      bool& overflow) {
  assert_compatible(lhs, rhs);
  if (lhs.is_small()) {
    MachineInt result(lhs._n.i - rhs._n.i, lhs._bit_width, lhs._sign);
    if (lhs.is_signed()) {
      overflow = lhs.is_non_negative() != rhs.is_non_negative() &&
                 result.is_non_negative() != lhs.is_non_negative();
    } else {
      overflow = result > lhs;
    }
    return result;
  } else {
    ZNumber n = (*lhs._n.p) - (*rhs._n.p);
    overflow = MachineInt::check_overflow(n, lhs._bit_width, lhs._sign) ||
               MachineInt::check_underflow(n, lhs._bit_width, lhs._sign);
    return MachineInt(n, lhs._bit_width, lhs._sign);
  }
}

/// \brief Subtraction
///
/// Returns the difference of the operands, with wrapping.
inline MachineInt sub(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_small()) {
    return MachineInt(lhs._n.i - rhs._n.i, lhs._bit_width, lhs._sign);
  } else {
    return MachineInt((*lhs._n.p) - (*rhs._n.p), lhs._bit_width, lhs._sign);
  }
}

/// \brief Subtraction
///
/// Returns the difference of the operands, with wrapping.
inline MachineInt operator-(const MachineInt& lhs, const MachineInt& rhs) {
  return sub(lhs, rhs);
}

/// \brief Multiplication
///
/// Returns the product of the operands, with wrapping.
inline MachineInt mul(const MachineInt& lhs,
                      const MachineInt& rhs,
                      bool& overflow) {
  assert_compatible(lhs, rhs);
  if (lhs.is_small()) {
    MachineInt result(lhs._n.i * rhs._n.i, lhs._bit_width, lhs._sign);
    if (lhs.bit_width() == 1) {
      if (lhs.is_signed()) {
        overflow = (lhs.is_min() && rhs.is_min());
      } else {
        overflow = false;
      }
    } else {
      overflow = !lhs.is_zero() && !rhs.is_zero() &&
                 (div(result, rhs) != lhs || div(result, lhs) != rhs);
    }
    return result;
  } else {
    ZNumber n = (*lhs._n.p) * (*rhs._n.p);
    overflow = MachineInt::check_overflow(n, lhs._bit_width, lhs._sign) ||
               MachineInt::check_underflow(n, lhs._bit_width, lhs._sign);
    return MachineInt(n, lhs._bit_width, lhs._sign);
  }
}

/// \brief Multiplication
///
/// Returns the product of the operands, with wrapping.
inline MachineInt mul(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_small()) {
    return MachineInt(lhs._n.i * rhs._n.i, lhs._bit_width, lhs._sign);
  } else {
    return MachineInt((*lhs._n.p) * (*rhs._n.p), lhs._bit_width, lhs._sign);
  }
}

/// \brief Multiplication
///
/// Returns the product of the operands, with wrapping.
inline MachineInt operator*(const MachineInt& lhs, const MachineInt& rhs) {
  return mul(lhs, rhs);
}

/// \brief Division
///
/// Returns the quotient of the operands.
///
/// For unsigned integers, this is the unsigned division.
/// For signed integers, this is the signed division rounded towards zero, with
/// wrapping.
///
/// Division by zero is undefined behavior.
///
/// `overflow` is set to true when dividing INT_MIN / (-1) for signed integers.
/// `exact` is set to true if `lhs` is a multiple of `rhs`.
inline MachineInt div(const MachineInt& lhs,
                      const MachineInt& rhs,
                      bool& overflow,
                      bool& exact) {
  assert_compatible(lhs, rhs);
  ikos_assert_msg(!rhs.is_zero(), "division by zero");
  if (lhs.is_small()) {
    if (lhs.is_signed()) {
      // Signed division
      MachineInt abs_lhs = lhs.is_negative() ? -lhs : lhs;
      MachineInt abs_rhs = rhs.is_negative() ? -rhs : rhs;
      MachineInt result(abs_lhs._n.i / abs_rhs._n.i,
                        lhs._bit_width,
                        lhs._sign,
                        MachineInt::NormalizedTag{});
      if (lhs.is_negative() ^ rhs.is_negative()) {
        result.negate();
      }
      overflow = lhs.is_min() && rhs.all_ones();
      exact = (abs_lhs._n.i % abs_rhs._n.i) == 0;
      return result;
    } else {
      // Unsigned division
      MachineInt result(lhs._n.i / rhs._n.i,
                        lhs._bit_width,
                        lhs._sign,
                        MachineInt::NormalizedTag{});
      overflow = false;
      exact = (lhs._n.i % rhs._n.i) == 0;
      return result;
    }
  } else {
    ZNumber n = (*lhs._n.p) / (*rhs._n.p);
    overflow = MachineInt::check_overflow(n, lhs._bit_width, lhs._sign) ||
               MachineInt::check_underflow(n, lhs._bit_width, lhs._sign);
    exact = mod(*lhs._n.p, *rhs._n.p) == 0;
    return MachineInt(n, lhs._bit_width, lhs._sign);
  }
}

/// \brief Division
///
/// Returns the quotient of the operands.
///
/// For unsigned integers, this is the unsigned division.
/// For signed integers, this is the signed division rounded towards zero, with
/// wrapping.
///
/// Division by zero is undefined behavior.
inline MachineInt div(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  ikos_assert_msg(!rhs.is_zero(), "division by zero");
  if (lhs.is_small()) {
    if (lhs.is_signed()) {
      // Signed division
      MachineInt abs_lhs = lhs.is_negative() ? -lhs : lhs;
      MachineInt abs_rhs = rhs.is_negative() ? -rhs : rhs;
      MachineInt result(abs_lhs._n.i / abs_rhs._n.i,
                        lhs._bit_width,
                        lhs._sign,
                        MachineInt::NormalizedTag{});
      if (lhs.is_negative() ^ rhs.is_negative()) {
        result.negate();
      }
      return result;
    } else {
      // Unsigned division
      return MachineInt(lhs._n.i / rhs._n.i,
                        lhs._bit_width,
                        lhs._sign,
                        MachineInt::NormalizedTag{});
    }
  } else {
    return MachineInt((*lhs._n.p) / (*rhs._n.p), lhs._bit_width, lhs._sign);
  }
}

/// \brief Division
///
/// Returns the quotient of the operands.
///
/// For unsigned integers, this is the unsigned division.
/// For signed integers, this is the signed division rounded towards zero, with
/// wrapping.
///
/// Division by zero is undefined behavior.
inline MachineInt operator/(const MachineInt& lhs, const MachineInt& rhs) {
  return div(lhs, rhs);
}

/// \brief Remainder
///
/// Returns the remainder of the operands.
///
/// For unsigned integers, the result is the classic modulo operator.
/// For signed integers, the result is either zero or has the same sign as the
/// dividend. The sign of the divisor is ignored.
inline MachineInt rem(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  ikos_assert_msg(!rhs.is_zero(), "division by zero");
  if (lhs.is_small()) {
    if (lhs.is_signed()) {
      // Signed remainder
      MachineInt abs_lhs = lhs.is_negative() ? -lhs : lhs;
      MachineInt abs_rhs = rhs.is_negative() ? -rhs : rhs;
      MachineInt result(abs_lhs._n.i % abs_rhs._n.i,
                        lhs._bit_width,
                        lhs._sign,
                        MachineInt::NormalizedTag{});
      if (lhs.is_negative()) {
        result.negate();
      }
      return result;
    } else {
      // Unsigned remainder
      return MachineInt(lhs._n.i % rhs._n.i,
                        lhs._bit_width,
                        lhs._sign,
                        MachineInt::NormalizedTag{});
    }
  } else {
    return MachineInt((*lhs._n.p) % (*rhs._n.p), lhs._bit_width, lhs._sign);
  }
}

/// \brief Remainder
///
/// Returns the remainder of the operands.
///
/// For unsigned integers, the result is the classic modulo operator.
/// For signed integers, the result is either zero or has the same sign as the
/// dividend. The sign of the divisor is ignored.
inline MachineInt operator%(const MachineInt& lhs, const MachineInt& rhs) {
  return rem(lhs, rhs);
}

/// \brief Modulo
///
/// Returns the modulo of the operands.
/// The result is always non-negative. The sign of the divisor is ignored.
inline MachineInt mod(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  ikos_assert_msg(!rhs.is_zero(), "division by zero");
  if (lhs.is_small()) {
    if (lhs.is_signed()) {
      // Signed modulo
      MachineInt abs_lhs = lhs.is_negative() ? -lhs : lhs;
      MachineInt abs_rhs = rhs.is_negative() ? -rhs : rhs;
      MachineInt result(abs_lhs._n.i % abs_rhs._n.i,
                        lhs._bit_width,
                        lhs._sign,
                        MachineInt::NormalizedTag{});
      if (lhs.is_negative() && !result.is_zero()) {
        result = abs_rhs - result;
      }
      return result;
    } else {
      // Unsigned modulo
      return MachineInt(lhs._n.i % rhs._n.i,
                        lhs._bit_width,
                        lhs._sign,
                        MachineInt::NormalizedTag{});
    }
  } else {
    return MachineInt(mod(*lhs._n.p, *rhs._n.p), lhs._bit_width, lhs._sign);
  }
}

/// \brief Left shift
///
/// Returns the left binary shift of `lhs` by `rhs`, with wrapping.
///
/// For signed integers, `overflow` is set to true if it shifts out any bits
/// that disagree with the resultant sign bit.
/// For unsigned integers, `overflow` is set to true if it shifts out any
/// non-zero bits.
inline MachineInt shl(const MachineInt& lhs,
                      const MachineInt& rhs,
                      bool& overflow) {
  assert_compatible(lhs, rhs);
  assert_shift(rhs, lhs._bit_width);
  if (lhs.is_small()) {
    MachineInt result(lhs._n.i << rhs._n.i, lhs._bit_width, lhs._sign);
    if (lhs.is_signed()) {
      if (lhs.is_negative()) {
        overflow = (rhs._n.i >= lhs.leading_ones());
      } else {
        overflow = (rhs._n.i >= lhs.leading_zeros());
      }
    } else {
      overflow = (rhs._n.i > lhs.leading_zeros());
    }
    return result;
  } else {
    MachineInt result((*lhs._n.p) << (*rhs._n.p), lhs._bit_width, lhs._sign);
    if (lhs.is_signed()) {
      ZNumber u = mod(*lhs._n.p, MachineInt::power_of_2(lhs._bit_width));
      ZNumber shifted = u >> (lhs._bit_width - (*rhs._n.p));
      if ((*result._n.p) >= 0) { // resultant sign bit is 0
        overflow = (shifted != 0);
      } else { // resultant sign bit is 1
        overflow = (shifted != MachineInt::power_of_2(*rhs._n.p) - 1);
      }
    } else {
      overflow = ((*lhs._n.p) >> (lhs._bit_width - (*rhs._n.p))) != 0;
    }
    return result;
  }
}

/// \brief Left shift
///
/// Returns the left binary shift of `lhs` by `rhs`, with wrapping.
inline MachineInt shl(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  assert_shift(rhs, lhs._bit_width);
  if (lhs.is_small()) {
    return MachineInt(lhs._n.i << rhs._n.i, lhs._bit_width, lhs._sign);
  } else {
    return MachineInt((*lhs._n.p) << (*rhs._n.p), lhs._bit_width, lhs._sign);
  }
}

/// \brief Logical shift right
///
/// Returns the logical right binary shift of `lhs` by `rhs`.
/// The most significant bits of the result are filled with zero bits.
///
/// `exact` is set to false if it shifts out any non-zero bits.
inline MachineInt lshr(const MachineInt& lhs,
                       const MachineInt& rhs,
                       bool& exact) {
  assert_compatible(lhs, rhs);
  assert_shift(rhs, lhs._bit_width);
  if (lhs.is_small()) {
    exact = (rhs._n.i == 0) ||
            (lhs._n.i & (MachineInt::ONES >> (64 - rhs._n.i))) == 0;
    return MachineInt(lhs._n.i >> rhs._n.i,
                      lhs._bit_width,
                      lhs._sign,
                      MachineInt::NormalizedTag{});
  } else {
    exact = ((*lhs._n.p) & (MachineInt::power_of_2(*rhs._n.p) - 1)) == 0;
    if (lhs.is_signed()) {
      ZNumber u = mod(*lhs._n.p, MachineInt::power_of_2(lhs._bit_width));
      return MachineInt(u >> (*rhs._n.p), lhs._bit_width, lhs._sign);
    } else {
      return MachineInt((*lhs._n.p) >> (*rhs._n.p), lhs._bit_width, lhs._sign);
    }
  }
}

/// \brief Logical shift right
///
/// Returns the logical right binary shift of `lhs` by `rhs`.
/// The most significant bits of the result are filled with zero bits.
inline MachineInt lshr(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  assert_shift(rhs, lhs._bit_width);
  if (lhs.is_small()) {
    return MachineInt(lhs._n.i >> rhs._n.i,
                      lhs._bit_width,
                      lhs._sign,
                      MachineInt::NormalizedTag{});
  } else {
    if (lhs.is_signed()) {
      ZNumber u = mod(*lhs._n.p, MachineInt::power_of_2(lhs._bit_width));
      return MachineInt(u >> (*rhs._n.p), lhs._bit_width, lhs._sign);
    } else {
      return MachineInt((*lhs._n.p) >> (*rhs._n.p), lhs._bit_width, lhs._sign);
    }
  }
}

/// \brief Arithmetic shift right
///
/// Returns the arithmetic right binary shift of `lhs` by `rhs`.
/// The most significant bits of the result are filled with the sign bit of
/// `lhs`.
///
/// `exact` is set to false if it shifts out any non-zero bits.
inline MachineInt ashr(const MachineInt& lhs,
                       const MachineInt& rhs,
                       bool& exact) {
  assert_compatible(lhs, rhs);
  assert_shift(rhs, lhs._bit_width);
  if (lhs.is_small()) {
    exact = (rhs._n.i == 0) ||
            (lhs._n.i & (MachineInt::ONES >> (64 - rhs._n.i))) == 0;
    int64_t n = MachineInt::sign_extend_64(lhs._n.i, lhs._bit_width);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return MachineInt(n >> static_cast< int64_t >(rhs._n.i),
                      lhs._bit_width,
                      lhs._sign);
  } else {
    exact = ((*lhs._n.p) & (MachineInt::power_of_2(*rhs._n.p) - 1)) == 0;
    if (lhs.is_signed()) {
      return MachineInt((*lhs._n.p) >> (*rhs._n.p), lhs._bit_width, lhs._sign);
    } else {
      ZNumber s = mod((*lhs._n.p) + MachineInt::power_of_2(lhs._bit_width - 1),
                      MachineInt::power_of_2(lhs._bit_width)) -
                  MachineInt::power_of_2(lhs._bit_width - 1);
      return MachineInt(s >> (*rhs._n.p), lhs._bit_width, lhs._sign);
    }
  }
}

/// \brief Arithmetic shift right
///
/// Returns the arithmetic right binary shift of `lhs` by `rhs`.
/// The most significant bits of the result are filled with the sign bit of
/// `lhs`.
inline MachineInt ashr(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  assert_shift(rhs, lhs._bit_width);
  if (lhs.is_small()) {
    int64_t n = MachineInt::sign_extend_64(lhs._n.i, lhs._bit_width);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return MachineInt(n >> static_cast< int64_t >(rhs._n.i),
                      lhs._bit_width,
                      lhs._sign);
  } else {
    if (lhs.is_signed()) {
      return MachineInt((*lhs._n.p) >> (*rhs._n.p), lhs._bit_width, lhs._sign);
    } else {
      ZNumber s = mod((*lhs._n.p) + MachineInt::power_of_2(lhs._bit_width - 1),
                      MachineInt::power_of_2(lhs._bit_width)) -
                  MachineInt::power_of_2(lhs._bit_width - 1);
      return MachineInt(s >> (*rhs._n.p), lhs._bit_width, lhs._sign);
    }
  }
}

/// \brief Bitwise AND
///
/// Returns the bitwise AND of the operands.
inline MachineInt and_(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_small()) {
    return MachineInt(lhs._n.i & rhs._n.i,
                      lhs._bit_width,
                      lhs._sign,
                      MachineInt::NormalizedTag{});
  } else {
    return MachineInt((*lhs._n.p) & (*rhs._n.p), lhs._bit_width, lhs._sign);
  }
}

/// \brief Bitwise AND
///
/// Returns the bitwise AND of the operands.
inline MachineInt operator&(const MachineInt& lhs, const MachineInt& rhs) {
  return and_(lhs, rhs);
}

/// \brief Bitwise OR
///
/// Returns the bitwise OR of the operands.
inline MachineInt or_(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_small()) {
    return MachineInt(lhs._n.i | rhs._n.i,
                      lhs._bit_width,
                      lhs._sign,
                      MachineInt::NormalizedTag{});
  } else {
    return MachineInt((*lhs._n.p) | (*rhs._n.p), lhs._bit_width, lhs._sign);
  }
}

/// \brief Bitwise OR
///
/// Returns the bitwise OR of the operands.
inline MachineInt operator|(const MachineInt& lhs, const MachineInt& rhs) {
  return or_(lhs, rhs);
}

/// \brief Bitwise XOR
///
/// Returns the bitwise XOR of the operands.
inline MachineInt xor_(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_small()) {
    return MachineInt(lhs._n.i ^ rhs._n.i,
                      lhs._bit_width,
                      lhs._sign,
                      MachineInt::NormalizedTag{});
  } else {
    return MachineInt((*lhs._n.p) ^ (*rhs._n.p), lhs._bit_width, lhs._sign);
  }
}

/// \brief Bitwise XOR
///
/// Returns the bitwise XOR of the operands.
inline MachineInt operator^(const MachineInt& lhs, const MachineInt& rhs) {
  return xor_(lhs, rhs);
}

/// \brief Return the greatest common divisor of the given machine integers
inline MachineInt gcd(const MachineInt& a, const MachineInt& b) {
  assert_compatible(a, b);
  if (a.is_small()) {
    MachineInt abs_a = a.is_negative() ? -a : a;
    MachineInt abs_b = b.is_negative() ? -b : b;
    uint64_t u = abs_a._n.i;
    uint64_t v = abs_b._n.i;
    while (v != 0) {
      uint64_t r = u % v;
      u = v;
      v = r;
    }
    return MachineInt(u, a._bit_width, b._sign);
  } else {
    return MachineInt(gcd(*a._n.p, *b._n.p), a._bit_width, b._sign);
  }
}

/// \brief Return the greatest common divisor of the given machine integers
inline MachineInt gcd(const MachineInt& a,
                      const MachineInt& b,
                      const MachineInt& c) {
  return gcd(gcd(a, b), c);
}

/// @}
/// \name Comparison Operators
/// @{

/// \brief Equality operator
inline bool operator==(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_small()) {
    return lhs._n.i == rhs._n.i;
  } else {
    return (*lhs._n.p) == (*rhs._n.p);
  }
}

/// \brief Equality operator with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator==(const MachineInt& lhs, T rhs) {
  return lhs == MachineInt(rhs, lhs.bit_width(), lhs.sign());
}

/// \brief Equality operator with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator==(T lhs, const MachineInt& rhs) {
  return MachineInt(lhs, rhs.bit_width(), rhs.sign()) == rhs;
}

/// \brief Inequality operator
inline bool operator!=(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_small()) {
    return lhs._n.i != rhs._n.i;
  } else {
    return (*lhs._n.p) != (*rhs._n.p);
  }
}

/// \brief Inequality operator with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator!=(const MachineInt& lhs, T rhs) {
  return lhs != MachineInt(rhs, lhs.bit_width(), lhs.sign());
}

/// \brief Inequality operator with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator!=(T lhs, const MachineInt& rhs) {
  return MachineInt(lhs, rhs.bit_width(), rhs.sign()) != rhs;
}

/// \brief Less than comparison
inline bool operator<(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_small()) {
    if (lhs.is_signed()) {
      return MachineInt::sign_extend_64(lhs._n.i, lhs._bit_width) <
             MachineInt::sign_extend_64(rhs._n.i, rhs._bit_width);
    } else {
      return lhs._n.i < rhs._n.i;
    }
  } else {
    return (*lhs._n.p) < (*rhs._n.p);
  }
}

/// \brief Less than comparison with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator<(const MachineInt& lhs, T rhs) {
  return lhs < MachineInt(rhs, lhs.bit_width(), lhs.sign());
}

/// \brief Less than comparison with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator<(T lhs, const MachineInt& rhs) {
  return MachineInt(lhs, rhs.bit_width(), rhs.sign()) < rhs;
}

/// \brief Less or equal comparison
inline bool operator<=(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_small()) {
    if (lhs.is_signed()) {
      return MachineInt::sign_extend_64(lhs._n.i, lhs._bit_width) <=
             MachineInt::sign_extend_64(rhs._n.i, rhs._bit_width);
    } else {
      return lhs._n.i <= rhs._n.i;
    }
  } else {
    return (*lhs._n.p) <= (*rhs._n.p);
  }
}

/// \brief Less or equal comparison with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator<=(const MachineInt& lhs, T rhs) {
  return lhs <= MachineInt(rhs, lhs.bit_width(), lhs.sign());
}

/// \brief Less or equal comparison with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator<=(T lhs, const MachineInt& rhs) {
  return MachineInt(lhs, rhs.bit_width(), rhs.sign()) <= rhs;
}

/// \brief Greater than comparison
inline bool operator>(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_small()) {
    if (lhs.is_signed()) {
      return MachineInt::sign_extend_64(lhs._n.i, lhs._bit_width) >
             MachineInt::sign_extend_64(rhs._n.i, rhs._bit_width);
    } else {
      return lhs._n.i > rhs._n.i;
    }
  } else {
    return (*lhs._n.p) > (*rhs._n.p);
  }
}

/// \brief Greater than comparison with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator>(const MachineInt& lhs, T rhs) {
  return lhs > MachineInt(rhs, lhs.bit_width(), lhs.sign());
}

/// \brief Greater than comparison with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator>(T lhs, const MachineInt& rhs) {
  return MachineInt(lhs, rhs.bit_width(), rhs.sign()) > rhs;
}

/// \brief Greater or equal comparison
inline bool operator>=(const MachineInt& lhs, const MachineInt& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_small()) {
    if (lhs.is_signed()) {
      return MachineInt::sign_extend_64(lhs._n.i, lhs._bit_width) >=
             MachineInt::sign_extend_64(rhs._n.i, rhs._bit_width);
    } else {
      return lhs._n.i >= rhs._n.i;
    }
  } else {
    return (*lhs._n.p) >= (*rhs._n.p);
  }
}

/// \brief Greater or equal comparison with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator>=(const MachineInt& lhs, T rhs) {
  return lhs >= MachineInt(rhs, lhs.bit_width(), lhs.sign());
}

/// \brief Greater or equal comparison with integral types
template < typename T,
           class = std::enable_if_t< IsSupportedIntegral< T >::value > >
inline bool operator>=(T lhs, const MachineInt& rhs) {
  return MachineInt(lhs, rhs.bit_width(), rhs.sign()) >= rhs;
}

/// @}
/// \name Utility functions (min, max, etc.)
/// @{

/// \brief Return the smaller of the given machine integers
inline const MachineInt& min(const MachineInt& a, const MachineInt& b) {
  return (a < b) ? a : b;
}

/// \brief Return the greater of the given machine integers
inline const MachineInt& max(const MachineInt& a, const MachineInt& b) {
  return (a < b) ? b : a;
}

/// \brief Return the absolute value of the given machine integer
///
/// Note that it overflows for the minimum signed integer
inline MachineInt abs(const MachineInt& n) {
  return n.is_negative() ? -n : n;
}

/// @}
/// \name Input / Output
/// @{

/// \brief Write a machine integer on a stream
inline std::ostream& operator<<(std::ostream& o, const MachineInt& n) {
  if (n.is_small()) {
    if (n.is_signed()) {
      o << MachineInt::sign_extend_64(n._n.i, n._bit_width);
    } else {
      o << n._n.i;
    }
  } else {
    o << *n._n.p;
  }
  return o;
}

/// @}

/// \brief Return the hash of a MachineInt
inline std::size_t hash_value(const MachineInt& n) {
  std::size_t hash = 0;
  if (n.is_small()) {
    boost::hash_combine(hash, n._n.i);
  } else {
    boost::hash_combine(hash, *n._n.p);
  }
  boost::hash_combine(hash, n._bit_width);
  boost::hash_combine(hash, n._sign);
  return hash;
}

} // end namespace core
} // end namespace ikos

namespace std {

/// \brief Hash for MachineInt
template <>
struct hash< ikos::core::MachineInt > {
  std::size_t operator()(const ikos::core::MachineInt& n) const {
    return ikos::core::hash_value(n);
  }
};

} // end namespace std
