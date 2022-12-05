/*******************************************************************************
 *
 * \file
 * \brief IntervalCongruence class for machine integers
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

#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/value/machine_int/congruence.hpp>
#include <ikos/core/value/machine_int/interval.hpp>
#include <ikos/core/value/numeric/congruence.hpp>
#include <ikos/core/value/numeric/interval_congruence.hpp>

namespace ikos {
namespace core {
namespace machine_int {

/// \brief Machine integer Interval-Congruence abstract value
///
/// This is implemented as a pair of interval and congruence abstract value.
class IntervalCongruence final
    : public core::AbstractDomain< IntervalCongruence > {
private:
  /// Use a pair (machine_int::Interval, numeric::ZCongruence)
  ///
  /// machine_int::Congruence is just a wrapper for numeric::ZCongruence.
  /// For performance reasons, it is better to use a numeric::ZCongruence here,
  /// This way the normalization is only done once.

public:
  /// \brief Interval on unlimited precision integers
  using ZInterval = numeric::Interval< ZNumber >;

  /// \brief Congruence on unlimited precision integers
  using ZCongruence = numeric::Congruence< ZNumber >;

  /// \brief Interval-congruence on unlimited precision integers
  using ZIntervalCongruence = numeric::IntervalCongruence< ZNumber >;

private:
  Interval _i;
  ZCongruence _c;

  /// \brief Reduce the interval-congruence
  ///
  /// Let (i, c) be a pair of interval and congruence
  /// if (c.is_bottom() || i.is_bottom()) (bottom(), bottom());
  /// if (c = 0Z+a and a notin i)         (bottom(), bottom());
  /// if (c = 0Z+a)                       ([a,a]   , c);
  /// if (i=[a,b] and R(c,a) > L(c,b))    (bottom(), bottom());
  /// if (i=[a,b] and R(c,a) = L(c,b))    ([R(c,a), R(c,b)], R(c,a))
  /// else (i=[a,b])                      ([R(c,a), L(c,b)], c);
  void reduce() {
    if (this->_c.is_bottom()) {
      this->_i.set_to_bottom();
      return;
    }

    if (this->_i.is_bottom()) {
      this->_c.set_to_bottom();
      return;
    }

    if (this->_c.is_top()) {
      // congruence is top
      boost::optional< MachineInt > n = this->_i.singleton();

      if (n) {
        // interval is a singleton
        this->_c = ZCongruence(n->to_z_number());
      }
      return;
    }

    ZNumber lb = this->_i.lb().to_z_number();
    ZNumber ub = this->_i.ub().to_z_number();

    if (this->_c.modulus() == 0) {
      // congruence is a singleton, refine the interval
      if (!(lb <= this->_c.residue() && this->_c.residue() <= ub)) {
        this->set_to_bottom();
      } else {
        this->_i = Interval(
            MachineInt(this->_c.residue(), this->bit_width(), this->sign()));
      }
      return;
    }

    // refine lower and upper bounds of the interval using congruences
    ikos_assert(this->_c.modulus() > 0);

    ZNumber x = Congruence::r(this->_c, lb);
    ZNumber y = Congruence::l(this->_c, ub);

    if (x > y) {
      this->set_to_bottom();
    } else if (x == y) {
      this->_i = Interval(MachineInt(x, this->bit_width(), this->sign()));
      this->_c = ZCongruence(x);
    } else {
      this->_i = Interval(MachineInt(x, this->bit_width(), this->sign()),
                          MachineInt(y, this->bit_width(), this->sign()));
    }
  }

  /// \brief Wrap a congruence in Z to the given bit-width and signedness
  static ZCongruence wrap(const ZCongruence& c,
                          uint64_t bit_width,
                          Signedness sign) {
    return Congruence::wrap(c, bit_width, sign);
  }

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top interval-congruence for the given bit-width and
  /// signedness
  IntervalCongruence(TopTag, uint64_t bit_width, Signedness sign)
      : _i(Interval::top(bit_width, sign)), _c(ZCongruence::top()) {}

  /// \brief Create the bottom interval-congruence for the given bit-width and
  /// signedness
  IntervalCongruence(BottomTag, uint64_t bit_width, Signedness sign)
      : _i(Interval::bottom(bit_width, sign)), _c(ZCongruence::bottom()) {}

public:
  /// \brief Create the top interval-congruence for the given bit-width and
  /// signedness
  static IntervalCongruence top(uint64_t bit_width, Signedness sign) {
    return IntervalCongruence(TopTag{}, bit_width, sign);
  }

  /// \brief Create the bottom interval-congruence for the given bit-width and
  /// signedness
  static IntervalCongruence bottom(uint64_t bit_width, Signedness sign) {
    return IntervalCongruence(BottomTag{}, bit_width, sign);
  }

  /// \brief Create the interval-congruence ([n, n], 0Z+n)
  explicit IntervalCongruence(const MachineInt& n)
      : _i(n), _c(n.to_z_number()) {}

  /// \brief Create the interval-congruence (i, c)
  IntervalCongruence(const Interval& i, const Congruence& c)
      : _i(i), _c(c.to_z_congruence()) {
    assert_compatible(i, c);
    this->reduce();
  }

  /// \brief Create the interval-congruence (i, c)
  IntervalCongruence(Interval i, ZCongruence c)
      : _i(std::move(i)), _c(std::move(c)) {
    this->reduce();
  }

  /// \brief Create the interval-congruence (i, T)
  explicit IntervalCongruence(Interval i)
      : _i(std::move(i)), _c(ZCongruence::top()) {
    this->reduce();
  }

  /// \brief Create the interval-congruence (T, c)
  explicit IntervalCongruence(const Congruence& c)
      : _i(Interval::top(c.bit_width(), c.sign())), _c(c.to_z_congruence()) {
    this->reduce();
  }

  /// \brief Copy constructor
  IntervalCongruence(const IntervalCongruence&) = default;

  /// \brief Move constructor
  IntervalCongruence(IntervalCongruence&&) = default;

  /// \brief Copy assignment operator
  IntervalCongruence& operator=(const IntervalCongruence&) = default;

  /// \brief Move assignment operator
  IntervalCongruence& operator=(IntervalCongruence&&) noexcept = default;

  /// \brief Destructor
  ~IntervalCongruence() override = default;

  /// \brief Return the bit width of the interval-congruence
  uint64_t bit_width() const { return this->_i.bit_width(); }

  /// \brief Return the signedness the interval-congruence
  Signedness sign() const { return this->_i.sign(); }

  bool is_signed() const { return this->sign() == Signed; }
  bool is_unsigned() const { return this->sign() == Unsigned; }

  /// \brief Return the interval
  const Interval& interval() const { return this->_i; }

  /// \brief Return the congruence
  Congruence congruence() const {
    return Congruence(this->_c,
                      this->bit_width(),
                      this->sign(),
                      Congruence::NormalizedTag{});
  }

  /// \brief Return the lower bound
  const MachineInt& lb() const {
    ikos_assert(!this->is_bottom());
    return this->_i.lb();
  }

  /// \brief Return the upper bound
  const MachineInt& ub() const {
    ikos_assert(!this->is_bottom());
    return this->_i.ub();
  }

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

  bool is_bottom() const override {
    return this->_c.is_bottom(); // Correct because of reduction
  }

  bool is_top() const override {
    return this->_i.is_top() && this->_c.is_top();
  }

  /// \brief Return true if the interval-congruence is 0
  bool is_zero() const {
    return this->_i.is_zero(); // Correct because of normalization
  }

  void set_to_bottom() override {
    this->_i.set_to_bottom();
    this->_c.set_to_bottom();
  }

  void set_to_top() override {
    this->_i.set_to_top();
    this->_c.set_to_top();
  }

  bool leq(const IntervalCongruence& other) const override {
    assert_compatible(*this, other);
    return this->_i.leq(other._i) && this->_c.leq(other._c);
  }

  bool equals(const IntervalCongruence& other) const override {
    assert_compatible(*this, other);
    return this->_i.equals(other._i) && this->_c.equals(other._c);
  }

  IntervalCongruence join(const IntervalCongruence& other) const override {
    assert_compatible(*this, other);
    return IntervalCongruence(this->_i.join(other._i), this->_c.join(other._c));
  }

  void join_with(const IntervalCongruence& other) override {
    assert_compatible(*this, other);
    this->_i.join_with(other._i);
    this->_c.join_with(other._c);
    this->reduce();
  }

  IntervalCongruence widening(const IntervalCongruence& other) const override {
    assert_compatible(*this, other);
    return IntervalCongruence(this->_i.widening(other._i),
                              this->_c.widening(other._c));
  }

  void widen_with(const IntervalCongruence& other) override {
    assert_compatible(*this, other);
    this->_i.widen_with(other._i);
    this->_c.widen_with(other._c);
    this->reduce();
  }

  IntervalCongruence widening_threshold(const IntervalCongruence& other,
                                        const MachineInt& threshold) const {
    assert_compatible(*this, other);
    return IntervalCongruence(this->_i.widening_threshold(other._i, threshold),
                              this->_c.widening(other._c));
  }

  void widen_threshold_with(const IntervalCongruence& other,
                            const MachineInt& threshold) {
    assert_compatible(*this, other);
    this->_i.widen_threshold_with(other._i, threshold);
    this->_c.widen_with(other._c);
    this->reduce();
  }

  IntervalCongruence meet(const IntervalCongruence& other) const override {
    assert_compatible(*this, other);
    return IntervalCongruence(this->_i.meet(other._i), this->_c.meet(other._c));
  }

  void meet_with(const IntervalCongruence& other) override {
    assert_compatible(*this, other);
    this->_i.meet_with(other._i);
    this->_c.meet_with(other._c);
    this->reduce();
  }

  IntervalCongruence narrowing(const IntervalCongruence& other) const override {
    assert_compatible(*this, other);
    return IntervalCongruence(this->_i.narrowing(other._i),
                              this->_c.narrowing(other._c));
  }

  void narrow_with(const IntervalCongruence& other) override {
    assert_compatible(*this, other);
    this->_i.narrow_with(other._i);
    this->_c.narrow_with(other._c);
    this->reduce();
  }

  IntervalCongruence narrowing_threshold(const IntervalCongruence& other,
                                         const MachineInt& threshold) const {
    assert_compatible(*this, other);
    return IntervalCongruence(this->_i.narrowing_threshold(other._i, threshold),
                              this->_c.narrowing(other._c));
  }

  void narrow_threshold_with(const IntervalCongruence& other,
                             const MachineInt& threshold) {
    assert_compatible(*this, other);
    this->_i.narrow_threshold_with(other._i, threshold);
    this->_c.narrow_with(other._c);
    this->reduce();
  }

  /// \name Unary Operators
  /// @{

  /// \brief Truncate the interval-congruence to the given bit width
  IntervalCongruence trunc(uint64_t bit_width) const {
    ikos_assert(this->bit_width() > bit_width);
    return IntervalCongruence(this->_i.trunc(bit_width),
                              wrap(this->_c, bit_width, this->sign()));
  }

  /// \brief Extend the interval-congruence to the given bit width
  IntervalCongruence ext(uint64_t bit_width) const {
    ikos_assert(this->bit_width() < bit_width);
    return IntervalCongruence(this->_i.ext(bit_width), this->_c);
  }

  /// \brief Change the interval-congruence sign (bitcast)
  IntervalCongruence sign_cast(Signedness sign) const {
    ikos_assert(this->sign() != sign);
    return IntervalCongruence(this->_i.sign_cast(sign),
                              wrap(this->_c, this->bit_width(), sign));
  }

  /// \brief Cast the interval-congruence to the given bit width and sign
  ///
  /// This is equivalent to trunc()/ext() + sign_cast() (in this specific order)
  IntervalCongruence cast(uint64_t bit_width, Signedness sign) const {
    return IntervalCongruence(this->_i.cast(bit_width, sign),
                              wrap(this->_c, bit_width, sign));
  }

  /// @}
  /// \name Conversion Functions
  /// @{

  /// \brief Return the interval as a ZInterval
  ZInterval to_z_interval() const { return this->_i.to_z_interval(); }

  /// \brief Return the congruence as a ZCongruence
  const ZCongruence& to_z_congruence() const { return this->_c; }

  /// \brief Return the interval-congruence as a ZIntervalCongruence
  ZIntervalCongruence to_z_interval_congruence() const {
    return ZIntervalCongruence(this->_i.to_z_interval(), this->_c);
  }

  struct WrapTag {};
  struct TruncTag {};

  /// \brief Convert a ZIntervalCongruence to a machine integer
  /// interval-congruence
  ///
  /// Note that it wraps.
  ///
  /// For instance:
  ///   from_z_congruence([255, 256], 1Z, 8, Unsigned, WrapTag{}) = Top
  static IntervalCongruence from_z_interval_congruence(
      const ZIntervalCongruence& ic,
      uint64_t bit_width,
      Signedness sign,
      WrapTag) {
    if (ic.is_bottom()) {
      return bottom(bit_width, sign);
    }

    return IntervalCongruence(Interval::from_z_interval(ic.interval(),
                                                        bit_width,
                                                        sign,
                                                        Interval::WrapTag{}),
                              wrap(ic.congruence(), bit_width, sign));
  }

  /// \brief Convert a ZIntervalCongruence to a machine integer
  /// interval-congruence
  ///
  /// Note that it truncates.
  ///
  /// For instance:
  ///   from_z_congruence([255, 256], 1Z, 8, Unsigned, WrapTag{}) = [255, 255]
  static IntervalCongruence from_z_interval_congruence(
      const ZIntervalCongruence& ic,
      uint64_t bit_width,
      Signedness sign,
      TruncTag) {
    if (ic.is_bottom()) {
      return bottom(bit_width, sign);
    }

    return IntervalCongruence(Interval::from_z_interval(ic.interval(),
                                                        bit_width,
                                                        sign,
                                                        Interval::TruncTag{}),
                              ic.congruence());
  }

  /// @}

  /// \brief If the interval-congruence is a singleton, return its value,
  /// otherwise return boost::none
  boost::optional< MachineInt > singleton() const {
    return this->_i.singleton(); // correct because of normalization
  }

  /// \brief Return true if the interval-congruence contains n
  bool contains(const MachineInt& n) const {
    return this->_i.contains(n) && this->_c.contains(n.to_z_number());
  }

  void dump(std::ostream& o) const override {
    o << "(";
    this->_i.dump(o);
    o << ", ";
    this->_c.dump(o);
    o << ")";
  }

  static std::string name() { return "interval-congruence"; }

  // Friends

  friend IntervalCongruence add(const IntervalCongruence& lhs,
                                const IntervalCongruence& rhs);

  friend IntervalCongruence add_no_wrap(const IntervalCongruence& lhs,
                                        const IntervalCongruence& rhs);

  friend IntervalCongruence sub(const IntervalCongruence& lhs,
                                const IntervalCongruence& rhs);

  friend IntervalCongruence sub_no_wrap(const IntervalCongruence& lhs,
                                        const IntervalCongruence& rhs);

  friend IntervalCongruence mul(const IntervalCongruence& lhs,
                                const IntervalCongruence& rhs);

  friend IntervalCongruence mul_no_wrap(const IntervalCongruence& lhs,
                                        const IntervalCongruence& rhs);

  friend IntervalCongruence div(const IntervalCongruence& lhs,
                                const IntervalCongruence& rhs);

  friend IntervalCongruence div_exact(const IntervalCongruence& lhs,
                                      const IntervalCongruence& rhs);

  friend IntervalCongruence rem(const IntervalCongruence& lhs,
                                const IntervalCongruence& rhs);

}; // end class IntervalCongruence

/// \name Binary Operators
/// @{

/// \brief Addition with wrapping
inline IntervalCongruence add(const IntervalCongruence& lhs,
                              const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return IntervalCongruence(add(lhs._i, rhs._i),
                              IntervalCongruence::wrap(lhs._c + rhs._c,
                                                       lhs.bit_width(),
                                                       lhs.sign()));
  }
}

/// \brief Addition without wrapping
inline IntervalCongruence add_no_wrap(const IntervalCongruence& lhs,
                                      const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return IntervalCongruence(add_no_wrap(lhs._i, rhs._i), lhs._c + rhs._c);
  }
}

/// \brief Substraction with wrapping
inline IntervalCongruence sub(const IntervalCongruence& lhs,
                              const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return IntervalCongruence(sub(lhs._i, rhs._i),
                              IntervalCongruence::wrap(lhs._c - rhs._c,
                                                       lhs.bit_width(),
                                                       lhs.sign()));
  }
}

/// \brief Substraction without wrapping
inline IntervalCongruence sub_no_wrap(const IntervalCongruence& lhs,
                                      const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return IntervalCongruence(sub_no_wrap(lhs._i, rhs._i), lhs._c - rhs._c);
  }
}

/// \brief Multiplication with wrapping
inline IntervalCongruence mul(const IntervalCongruence& lhs,
                              const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return IntervalCongruence(mul(lhs._i, rhs._i),
                              IntervalCongruence::wrap(lhs._c * rhs._c,
                                                       lhs.bit_width(),
                                                       lhs.sign()));
  }
}

/// \brief Multiplication without wrapping
inline IntervalCongruence mul_no_wrap(const IntervalCongruence& lhs,
                                      const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return IntervalCongruence(mul_no_wrap(lhs._i, rhs._i), lhs._c * rhs._c);
  }
}

/// \brief Division
inline IntervalCongruence div(const IntervalCongruence& lhs,
                              const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    // overflow is undefined behavior
    return IntervalCongruence(div(lhs._i, rhs._i), lhs._c / rhs._c);
  }
}

/// \brief Exact division
inline IntervalCongruence div_exact(const IntervalCongruence& lhs,
                                    const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    // overflow is undefined behavior
    return IntervalCongruence(div_exact(lhs._i, rhs._i), lhs._c / rhs._c);
  }
}

/// \brief Remainder
inline IntervalCongruence rem(const IntervalCongruence& lhs,
                              const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return IntervalCongruence(rem(lhs._i, rhs._i), lhs._c % rhs._c);
  }
}

/// \brief Left shift with wrapping
///
/// The right hand side has to be between 0 and bit_width - 1
inline IntervalCongruence shl(const IntervalCongruence& lhs,
                              const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return IntervalCongruence(shl(lhs.interval(), rhs.interval()),
                              shl(lhs.congruence(), rhs.congruence()));
  }
}

/// \brief Left shift without wrapping
///
/// The right hand side has to be between 0 and bit_width - 1
inline IntervalCongruence shl_no_wrap(const IntervalCongruence& lhs,
                                      const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return IntervalCongruence(shl_no_wrap(lhs.interval(), rhs.interval()),
                              shl_no_wrap(lhs.congruence(), rhs.congruence()));
  }
}

/// \brief Logical shift right
///
/// The right hand side has to be between 0 and bit_width - 1
inline IntervalCongruence lshr(const IntervalCongruence& lhs,
                               const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return IntervalCongruence(lshr(lhs.interval(), rhs.interval()),
                              lshr(lhs.congruence(), rhs.congruence()));
  }
}

/// \brief Exact logical shift right
///
/// The right hand side has to be between 0 and bit_width - 1
inline IntervalCongruence lshr_exact(const IntervalCongruence& lhs,
                                     const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return IntervalCongruence(lshr_exact(lhs.interval(), rhs.interval()),
                              lshr_exact(lhs.congruence(), rhs.congruence()));
  }
}

/// \brief Arithmetic shift right
///
/// The right hand side has to be between 0 and bit_width - 1
inline IntervalCongruence ashr(const IntervalCongruence& lhs,
                               const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return IntervalCongruence(ashr(lhs.interval(), rhs.interval()),
                              ashr(lhs.congruence(), rhs.congruence()));
  }
}

/// \brief Exact arithmetic shift right
///
/// The right hand side has to be between 0 and bit_width - 1
inline IntervalCongruence ashr_exact(const IntervalCongruence& lhs,
                                     const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return IntervalCongruence(ashr_exact(lhs.interval(), rhs.interval()),
                              ashr_exact(lhs.congruence(), rhs.congruence()));
  }
}

/// \brief Bitwise AND
inline IntervalCongruence and_(const IntervalCongruence& lhs,
                               const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return IntervalCongruence(and_(lhs.interval(), rhs.interval()),
                              and_(lhs.congruence(), rhs.congruence()));
  }
}

/// \brief Bitwise OR
inline IntervalCongruence or_(const IntervalCongruence& lhs,
                              const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return IntervalCongruence(or_(lhs.interval(), rhs.interval()),
                              or_(lhs.congruence(), rhs.congruence()));
  }
}

/// \brief Bitwise XOR
inline IntervalCongruence xor_(const IntervalCongruence& lhs,
                               const IntervalCongruence& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return IntervalCongruence(xor_(lhs.interval(), rhs.interval()),
                              xor_(lhs.congruence(), rhs.congruence()));
  }
}

/// @}
/// \name Input / Output
/// @{

/// \brief Write an interval-congruence on a stream
inline std::ostream& operator<<(std::ostream& o, const IntervalCongruence& iv) {
  iv.dump(o);
  return o;
}

/// @}

} // end namespace machine_int
} // end namespace core
} // end namespace ikos
