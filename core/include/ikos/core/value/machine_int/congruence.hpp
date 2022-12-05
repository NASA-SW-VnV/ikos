/*******************************************************************************
 *
 * \file
 * \brief Machine integer congruence abstract value
 *
 * Bitwise extensions based on Stefan Bygde's paper: Static WCET analysis based
 * on abstract interpretation and counting of elements, Vasteras : School of
 * Innovation, Design and Engineering, Malardalen University (2010)
 *
 * Author: Maxime Arthaud
 *
 * Contributors:
 *   * Jorge A. Navas
 *   * Alexandre C. D. Wimmers
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

#include <boost/optional.hpp>

#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/number/machine_int.hpp>
#include <ikos/core/number/z_number.hpp>
#include <ikos/core/value/numeric/congruence.hpp>

namespace ikos {
namespace core {
namespace machine_int {

/// \brief Machine integer congruence abstract value
///
/// This is either bottom, or aZ + b.
///
/// Note that top is represented as 1Z + 0.
class Congruence final : public core::AbstractDomain< Congruence > {
private:
  /// Use the congruence on unlimited precision integers (ZNumber) internally
  ///
  /// Writing a congruence class using MachineInt appears to be very hard,
  /// because of overflows and the limited bit-width for the modulus.

private:
  /// \brief Congruence on unlimited precision integers
  using ZCongruence = numeric::ZCongruence;

private:
  ZCongruence _c;
  uint64_t _bit_width;
  Signedness _sign;

private:
  /// \brief Return 2**n
  static ZNumber power_of_2(uint64_t n) { return ZNumber(1) << n; }

  /// \brief Return 2**n
  static ZNumber power_of_2(const ZNumber& n) { return ZNumber(1) << n; }

  /// \brief r(c, a) is the smallest element of c greater or equal than a
  static ZNumber r(const ZCongruence& c, const ZNumber& a) {
    ikos_assert(!c.is_bottom() && c.modulus() != 0);
    return a + mod(c.residue() - a, c.modulus());
  }

  /// \brief l(c, a) is the greatest element of c smaller or equal than a
  static ZNumber l(const ZCongruence& c, const ZNumber& a) {
    ikos_assert(!c.is_bottom() && c.modulus() != 0);
    return a - mod(a - c.residue(), c.modulus());
  }

  /// \brief Reduce the congruence
  void reduce() {
    if (this->_c.is_bottom()) {
      return;
    }

    if (this->_c.is_top()) {
      return;
    }

    // lb, ub = [Int::min, Int::max]
    ZNumber lb;
    ZNumber ub;
    if (this->is_signed()) {
      lb = -power_of_2(this->_bit_width - 1);
      ub = power_of_2(this->_bit_width - 1) - 1;
    } else {
      lb = 0;
      ub = power_of_2(this->_bit_width) - 1;
    }

    if (this->_c.modulus() == 0) {
      // 0Z + b
      if (!(lb <= this->_c.residue() && this->_c.residue() <= ub)) {
        this->set_to_bottom();
      }
      return;
    }

    // aZ + b with a != 0
    ikos_assert(this->_c.modulus() > 0);

    ZNumber x = r(this->_c, lb);
    ZNumber y = l(this->_c, ub);

    if (x > y) {
      this->_c.set_to_bottom();
    } else if (x == y) {
      this->_c = ZCongruence(x);
    }
  }

  /// \brief Wrap a congruence in Z to the given bit-width and signedness
  static ZCongruence wrap(const ZCongruence& c,
                          uint64_t bit_width,
                          Signedness sign) {
    if (c.is_bottom()) {
      return ZCongruence::bottom();
    } else if (sign == Signed) {
      ZNumber n = power_of_2(bit_width);
      ZNumber m = power_of_2(bit_width - 1);
      return mod(c + m, ZCongruence(n)) - m;
    } else {
      ZNumber n = power_of_2(bit_width);
      return mod(c, ZCongruence(n));
    }
  }

  /// \brief Weakening operator
  ///
  /// \returns weakened version of *this, does not modify *this. Returned
  /// modulus is guaranteed to be a power of 2.
  static ZCongruence weaken(ZCongruence c, uint64_t bit_width) {
    if (c.is_bottom()) {
      return c;
    } else if (c.modulus() == 0) {
      return c;
    } else {
      return ZCongruence(gcd(c.modulus(), power_of_2(bit_width)), c.residue());
    }
  }

private:
  struct TopTag {};
  struct BottomTag {};
  struct NormalizedTag {};

  /// \brief Create the top congruence for the given bit-width and signedness
  Congruence(TopTag, uint64_t bit_width, Signedness sign)
      : _c(ZCongruence::top()), _bit_width(bit_width), _sign(sign) {}

  /// \brief Create the bottom congruence for the given bit-width and signedness
  Congruence(BottomTag, uint64_t bit_width, Signedness sign)
      : _c(ZCongruence::bottom()), _bit_width(bit_width), _sign(sign) {}

  /// \brief Create the machine integer congruence from a congruence in Z
  Congruence(ZCongruence c, uint64_t bit_width, Signedness sign, NormalizedTag)
      : _c(std::move(c)), _bit_width(bit_width), _sign(sign) {}

public:
  /// \brief Create the top congruence for the given bit-width and signedness
  static Congruence top(uint64_t bit_width, Signedness sign) {
    return Congruence(TopTag{}, bit_width, sign);
  }

  /// \brief Create the bottom congruence for the given bit-width and signedness
  static Congruence bottom(uint64_t bit_width, Signedness sign) {
    return Congruence(BottomTag{}, bit_width, sign);
  }

  /// \brief Create the congruence 0Z + n
  explicit Congruence(const MachineInt& n)
      : _c(n.to_z_number()), _bit_width(n.bit_width()), _sign(n.sign()) {}

  /// \brief Create the congruence aZ + b
  Congruence(const MachineInt& a, const MachineInt& b)
      : _c(a.to_z_number(), b.to_z_number()),
        _bit_width(b.bit_width()),
        _sign(b.sign()) {
    assert_compatible(a, b);
    this->reduce();
  }

  /// \brief Create the congruence aZ + b
  Congruence(ZNumber a, ZNumber b, uint64_t bit_width, Signedness sign)
      : _c(std::move(a), std::move(b)), _bit_width(bit_width), _sign(sign) {
    this->reduce();
  }

  /// \brief Create the machine integer congruence from a congruence in Z
  Congruence(ZCongruence c, uint64_t bit_width, Signedness sign)
      : _c(std::move(c)), _bit_width(bit_width), _sign(sign) {
    this->reduce();
  }

  /// \brief Copy constructor
  Congruence(const Congruence&) = default;

  /// \brief Move constructor
  Congruence(Congruence&&) = default;

  /// \brief Copy assignment operator
  Congruence& operator=(const Congruence&) = default;

  /// \brief Move assignment operator
  Congruence& operator=(Congruence&&) noexcept = default;

  /// \brief Destructor
  ~Congruence() override = default;

  /// \brief Return the bit width of the congruence
  uint64_t bit_width() const { return this->_bit_width; }

  /// \brief Return the signedness (Signed or Unsigned) of the congruence
  Signedness sign() const { return this->_sign; }

  bool is_signed() const { return this->_sign == Signed; }
  bool is_unsigned() const { return this->_sign == Unsigned; }

  /// \brief Return the modulus
  const ZNumber& modulus() const {
    ikos_assert(!this->is_bottom());
    return this->_c.modulus();
  }

  /// \brief Return the residue
  const ZNumber& residue() const {
    ikos_assert(!this->is_bottom());
    return this->_c.residue();
  }

  void normalize() override {
    // Already performed by the reduction
  }

  bool is_bottom() const override { return this->_c.is_bottom(); }

  bool is_top() const override { return this->_c.is_top(); }

  /// \brief Return true if the congruence is 0Z + 0
  bool is_zero() const { return this->_c.is_zero(); }

  void set_to_bottom() override { this->_c.set_to_bottom(); }

  void set_to_top() override { this->_c.set_to_top(); }

  bool leq(const Congruence& other) const override {
    assert_compatible(*this, other);
    return this->_c.leq(other._c);
  }

  bool equals(const Congruence& other) const override {
    assert_compatible(*this, other);
    return this->_c.equals(other._c);
  }

  Congruence join(const Congruence& other) const override {
    assert_compatible(*this, other);
    return Congruence(this->_c.join(other._c), this->_bit_width, this->_sign);
  }

  void join_with(const Congruence& other) override {
    assert_compatible(*this, other);
    this->_c.join_with(other._c);
    this->reduce();
  }

  Congruence widening(const Congruence& other) const override {
    // equivalent to join, domain is flat
    return this->join(other);
  }

  void widen_with(const Congruence& other) override {
    // equivalent to join, domain is flat
    this->join_with(other);
  }

  Congruence widening_threshold(const Congruence& other,
                                const MachineInt& /*threshold*/) const {
    // equivalent to join, domain is flat
    return this->join(other);
  }

  void widen_threshold_with(const Congruence& other,
                            const MachineInt& /*threshold*/) {
    // equivalent to join, domain is flat
    this->join_with(other);
  }

  Congruence meet(const Congruence& other) const override {
    assert_compatible(*this, other);
    return Congruence(this->_c.meet(other._c), this->_bit_width, this->_sign);
  }

  void meet_with(const Congruence& other) override {
    assert_compatible(*this, other);
    this->_c.meet_with(other._c);
    this->reduce();
  }

  Congruence narrowing(const Congruence& other) const override {
    // equivalent to meet, domain is flat
    return this->meet(other);
  }

  void narrow_with(const Congruence& other) override {
    // equivalent to meet, domain is flat
    this->meet_with(other);
  }

  Congruence narrowing_threshold(const Congruence& other,
                                 const MachineInt& /*threshold*/) const {
    // equivalent to meet, domain is flat
    return this->meet(other);
  }

  void narrow_threshold_with(const Congruence& other,
                             const MachineInt& /*threshold*/) {
    // equivalent to meet, domain is flat
    this->meet_with(other);
  }

  /// \name Unary Operators
  /// @{

  /// \brief Truncate the machine integer congruence to the given bit width
  Congruence trunc(uint64_t bit_width) const {
    ikos_assert(this->bit_width() > bit_width);
    return Congruence(wrap(this->_c, bit_width, this->_sign),
                      bit_width,
                      this->_sign);
  }

  /// \brief Extend the machine integer congruence to the given bit width
  Congruence ext(uint64_t bit_width) const {
    ikos_assert(this->bit_width() < bit_width);
    return Congruence(this->_c, bit_width, this->_sign, NormalizedTag{});
  }

  /// \brief Change the machine integer congruence sign (bitcast)
  Congruence sign_cast(Signedness sign) const {
    ikos_assert(this->sign() != sign);
    return Congruence(wrap(this->_c, this->_bit_width, sign),
                      this->_bit_width,
                      sign);
  }

  /// \brief Cast the machine integer congruence to the given bit width and sign
  ///
  /// This is equivalent to trunc()/ext() + sign_cast() (in this specific order)
  Congruence cast(uint64_t bit_width, Signedness sign) const {
    return Congruence(wrap(this->_c, bit_width, sign), bit_width, sign);
  }

  /// \brief Bitwise not operator
  Congruence not_() const {
    if (this->is_bottom()) {
      return *this;
    } else if (this->is_signed()) {
      return Congruence(this->_c.modulus(),
                        -this->_c.residue() - 1,
                        this->_bit_width,
                        this->_sign);
    } else {
      return Congruence(this->_c.modulus(),
                        power_of_2(this->_bit_width) - this->_c.residue() - 1,
                        this->_bit_width,
                        this->_sign);
    }
  }

  /// @}
  /// \name Conversion Functions
  /// @{

  /// \brief Return the machine integer congruence as a ZCongruence
  const ZCongruence& to_z_congruence() const { return this->_c; }

  struct WrapTag {};
  struct TruncTag {};

  /// \brief Convert a ZCongruence to a machine integer congruence
  ///
  /// Note: it truncates integers
  ///
  /// For instance:
  ///   from_z_congruence(ZCongruence(256), 8, Unsigned, TruncTag{}) = Bottom
  static Congruence from_z_congruence(ZCongruence c,
                                      uint64_t bit_width,
                                      Signedness sign,
                                      TruncTag) {
    return Congruence(std::move(c), bit_width, sign);
  }

  /// \brief Convert a ZCongruence to a machine integer congruence
  ///
  /// Note: it wraps integers
  ///
  /// For instance:
  ///   from_z_congruence(ZCongruence(256), 8, Unsigned, WrapTag{}) = 0Z + 0
  static Congruence from_z_congruence(const ZCongruence& c,
                                      uint64_t bit_width,
                                      Signedness sign,
                                      WrapTag) {
    return Congruence(wrap(c, bit_width, sign), bit_width, sign);
  }

  /// @}

  /// \brief If the congruence is 0Z + n, return n, otherwise return boost::none
  boost::optional< MachineInt > singleton() const {
    if (!this->is_bottom() && this->_c.modulus() == 0) {
      return MachineInt(this->_c.residue(), this->_bit_width, this->_sign);
    } else {
      return boost::none;
    }
  }

  /// \brief Return true if the congruence contains n
  bool contains(const MachineInt& n) const {
    return this->_c.contains(n.to_z_number());
  }

  void dump(std::ostream& o) const override { this->_c.dump(o); }

  static std::string name() { return "congruence"; }

  // Friends

  friend Congruence add(const Congruence& lhs, const Congruence& rhs);

  friend Congruence add_no_wrap(const Congruence& lhs, const Congruence& rhs);

  friend Congruence sub(const Congruence& lhs, const Congruence& rhs);

  friend Congruence sub_no_wrap(const Congruence& lhs, const Congruence& rhs);

  friend Congruence mul(const Congruence& lhs, const Congruence& rhs);

  friend Congruence mul_no_wrap(const Congruence& lhs, const Congruence& rhs);

  friend Congruence div(const Congruence& lhs, const Congruence& rhs);

  friend Congruence rem(const Congruence& lhs, const Congruence& rhs);

  friend Congruence shl(const Congruence& lhs, const Congruence& rhs);

  friend Congruence shl_no_wrap(const Congruence& lhs, const Congruence& rhs);

  friend Congruence lshr(const Congruence& lhs, const Congruence& rhs);

  friend Congruence ashr(const Congruence& lhs, const Congruence& rhs);

  friend Congruence and_(const Congruence& lhs, const Congruence& rhs);

  friend class IntervalCongruence;

}; // end class Congruence

/// \name Binary Operators
/// @{

/// \brief Addition with wrapping
inline Congruence add(const Congruence& lhs, const Congruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return Congruence(Congruence::wrap(lhs._c + rhs._c,
                                       lhs._bit_width,
                                       lhs._sign),
                      lhs._bit_width,
                      lhs._sign);
  }
}

/// \brief Addition without wrapping
inline Congruence add_no_wrap(const Congruence& lhs, const Congruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return Congruence(lhs._c + rhs._c, lhs._bit_width, lhs._sign);
  }
}

/// \brief Substraction with wrapping
inline Congruence sub(const Congruence& lhs, const Congruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return Congruence(Congruence::wrap(lhs._c - rhs._c,
                                       lhs._bit_width,
                                       lhs._sign),
                      lhs._bit_width,
                      lhs._sign);
  }
}

/// \brief Substraction without wrapping
inline Congruence sub_no_wrap(const Congruence& lhs, const Congruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return Congruence(lhs._c - rhs._c, lhs._bit_width, lhs._sign);
  }
}

/// \brief Multiplication with wrapping
inline Congruence mul(const Congruence& lhs, const Congruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return Congruence(Congruence::wrap(lhs._c * rhs._c,
                                       lhs._bit_width,
                                       lhs._sign),
                      lhs._bit_width,
                      lhs._sign);
  }
}

/// \brief Multiplication without wrapping
inline Congruence mul_no_wrap(const Congruence& lhs, const Congruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return Congruence(lhs._c * rhs._c, lhs._bit_width, lhs._sign);
  }
}

/// \brief Division
inline Congruence div(const Congruence& lhs, const Congruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    // overflow is undefined behavior
    return Congruence(lhs._c / rhs._c, lhs._bit_width, lhs._sign);
  }
}

/// \brief Exact division
inline Congruence div_exact(const Congruence& lhs, const Congruence& rhs) {
  return div(lhs, rhs);
}

/// \brief Remainder
inline Congruence rem(const Congruence& lhs, const Congruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return Congruence(lhs._c % rhs._c, lhs._bit_width, lhs._sign);
  }
}

/// \brief Left shift with wrapping
///
/// The right hand side has to be between 0 and bit_width - 1
inline Congruence shl(const Congruence& lhs, const Congruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    if (rhs._c.modulus() == 0) {
      // aZ + b << 0Z + b'
      const ZNumber& b = rhs._c.residue();

      if (!(0 <= b && b < lhs._bit_width)) {
        // Invalid operand
        return Congruence::bottom(lhs._bit_width, lhs._sign);
      }

      // aZ + b << 0Z + b' = (a*2^b')Z + b*2^b'
      ZNumber x = Congruence::power_of_2(b);
      numeric::ZCongruence c(lhs._c.modulus() * x, lhs._c.residue() * x);
      return Congruence(Congruence::wrap(c, lhs._bit_width, lhs._sign),
                        lhs._bit_width,
                        lhs._sign);
    }

    // aZ + b << a'Z + b' = (gcd(a, b * (2^a' - 1)))*(2^b')Z + b*(2^b')
    ZNumber x = Congruence::power_of_2(rhs._c.residue());
    ZNumber y = Congruence::power_of_2(rhs._c.modulus());
    ZNumber a = gcd(lhs._c.modulus(), lhs._c.residue() * (y - 1)) * x;
    ZNumber b = lhs._c.residue() * x;
    numeric::ZCongruence c(a, b);
    return Congruence(Congruence::wrap(c, lhs._bit_width, lhs._sign),
                      lhs._bit_width,
                      lhs._sign);
  }
}

/// \brief Left shift without wrapping
///
/// The right hand side has to be between 0 and bit_width - 1
inline Congruence shl_no_wrap(const Congruence& lhs, const Congruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    if (rhs._c.modulus() == 0) {
      // aZ + b << 0Z + b'
      const ZNumber& b = rhs._c.residue();

      if (!(0 <= b && b < lhs._bit_width)) {
        // Invalid operand
        return Congruence::bottom(lhs._bit_width, lhs._sign);
      }

      // aZ + b << 0Z + b' = (a*2^b')Z + b*2^b'
      ZNumber x = Congruence::power_of_2(b);
      numeric::ZCongruence c(lhs._c.modulus() * x, lhs._c.residue() * x);
      return Congruence(c, lhs._bit_width, lhs._sign);
    }

    // aZ + b << a'Z + b' = (gcd(a, b * (2^a' - 1)))*(2^b')Z + b*(2^b')
    ZNumber x = Congruence::power_of_2(rhs._c.residue());
    ZNumber y = Congruence::power_of_2(rhs._c.modulus());
    ZNumber a = gcd(lhs._c.modulus(), lhs._c.residue() * (y - 1)) * x;
    ZNumber b = lhs._c.residue() * x;
    numeric::ZCongruence c(a, b);
    return Congruence(c, lhs._bit_width, lhs._sign);
  }
}

/// \brief Logical shift right
///
/// The right hand side has to be between 0 and bit_width - 1
inline Congruence lshr(const Congruence& lhs, const Congruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    if (rhs._c.modulus() == 0) {
      // aZ + b >> 0Z + b'
      const ZNumber& b = rhs._c.residue();

      if (!(0 <= b && b < lhs._bit_width)) {
        // Invalid operand
        return Congruence::bottom(lhs._bit_width, lhs._sign);
      }
    }

    if (lhs._c.is_zero() || rhs._c.is_zero()) {
      // 0 >> aZ + b
      // aZ + b >> 0
      return lhs;
    }

    if (lhs._c.modulus() == 0 && rhs._c.modulus() == 0) {
      // 0Z + b >> 0Z + b'
      return Congruence(
          lshr(MachineInt(lhs._c.residue(), lhs._bit_width, lhs._sign),
               MachineInt(rhs._c.residue(), lhs._bit_width, lhs._sign)));
    }

    return Congruence::top(lhs._bit_width, lhs._sign);
  }
}

/// \brief Exact logical shift right
///
/// The right hand side has to be between 0 and bit_width - 1
inline Congruence lshr_exact(const Congruence& lhs, const Congruence& rhs) {
  return lshr(lhs, rhs);
}

/// \brief Arithmetic shift right
///
/// The right hand side has to be between 0 and bit_width - 1
inline Congruence ashr(const Congruence& lhs, const Congruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    if (rhs._c.modulus() == 0) {
      // aZ + b >> 0Z + b'
      const ZNumber& b = rhs._c.residue();

      if (!(0 <= b && b < lhs._bit_width)) {
        // Invalid operand
        return Congruence::bottom(lhs._bit_width, lhs._sign);
      }
    }

    if (lhs._c.is_zero() || rhs._c.is_zero()) {
      // 0 >> aZ + b
      // aZ + b >> 0
      return lhs;
    }

    if (lhs._c.modulus() == 0 && rhs._c.modulus() == 0) {
      // 0Z + b >> 0Z + b'
      return Congruence(
          ashr(MachineInt(lhs._c.residue(), lhs._bit_width, lhs._sign),
               MachineInt(rhs._c.residue(), lhs._bit_width, lhs._sign)));
    }

    return Congruence::top(lhs._bit_width, lhs._sign);
  }
}

/// \brief Exact arithmetic shift right
///
/// The right hand side has to be between 0 and bit_width - 1
inline Congruence ashr_exact(const Congruence& lhs, const Congruence& rhs) {
  return ashr(lhs, rhs);
}

/// \brief Bitwise AND
inline Congruence and_(const Congruence& lhs, const Congruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    // First, both numbers are approximated to power of 2
    numeric::ZCongruence x = Congruence::weaken(lhs._c, lhs._bit_width);
    numeric::ZCongruence y = Congruence::weaken(rhs._c, lhs._bit_width);

    if (x.modulus() == 0 && y.modulus() == 0) {
      // 0Z + b & 0Z + b'
      return Congruence(
          and_(MachineInt(x.residue(), lhs._bit_width, lhs._sign),
               MachineInt(y.residue(), lhs._bit_width, lhs._sign)));
    } else if (x.modulus() == 0) {
      // 0Z + b & gcd(2^n,a')Z + b'
      ZNumber b = mod(x.residue(), Congruence::power_of_2(lhs._bit_width));

      if (b < y.modulus()) {
        return Congruence(
            and_(MachineInt(x.residue(), lhs._bit_width, lhs._sign),
                 MachineInt(y.residue(), lhs._bit_width, lhs._sign)));
      } else {
        return Congruence(y.modulus(),
                          b & y.residue(),
                          lhs._bit_width,
                          lhs._sign);
      }
    } else if (y.modulus() == 0) {
      // Symmetric
      return and_(rhs, lhs);
    } else {
      return Congruence(min(x.modulus(), y.modulus()),
                        x.residue() & y.residue(),
                        lhs._bit_width,
                        lhs._sign);
    }
  }
}

/// \brief Bitwise OR
inline Congruence or_(const Congruence& lhs, const Congruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return and_(lhs.not_(), rhs.not_()).not_();
  }
}

/// \brief Bitwise XOR
inline Congruence xor_(const Congruence& lhs, const Congruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return or_(and_(lhs, rhs.not_()), and_(lhs.not_(), rhs));
  }
}

/// @}
/// \name Input / Output
/// @{

/// \brief Write a congruence on a stream
inline std::ostream& operator<<(std::ostream& o, const Congruence& congruence) {
  congruence.dump(o);
  return o;
}

/// @}

} // end namespace machine_int
} // end namespace core
} // end namespace ikos
