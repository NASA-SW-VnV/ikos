/*******************************************************************************
 *
 * \file
 * \brief Machine integer interval class
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

#include <boost/optional.hpp>

#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/number/machine_int.hpp>
#include <ikos/core/value/numeric/interval.hpp>

namespace ikos {
namespace core {
namespace machine_int {

/// \brief Machine integer interval abstract value
class Interval final : public core::AbstractDomain< Interval > {
private:
  using ZBound = Bound< ZNumber >;
  using ZInterval = numeric::Interval< ZNumber >;

private:
  // Lower bound
  MachineInt _lb;

  // Upper bound
  MachineInt _ub;

  // Invariant: is_bottom() <=> _lb > _ub

public:
  /// \brief Create the top interval for the given bit-width and signedness
  static Interval top(uint64_t bit_width, Signedness sign) {
    return Interval(MachineInt::min(bit_width, sign),
                    MachineInt::max(bit_width, sign));
  }

  /// \brief Create the bottom interval for the given bit-width and signedness
  static Interval bottom(uint64_t bit_width, Signedness sign) {
    return Interval(MachineInt::max(bit_width, sign),
                    MachineInt::min(bit_width, sign));
  }

  /// \brief Create the interval [n, n]
  explicit Interval(const MachineInt& n) : _lb(n), _ub(n) {}

  /// \brief Create the interval [lb, ub]
  ///
  /// If lb > ub, the returned interval is bottom.
  Interval(MachineInt lb, MachineInt ub)
      : _lb(std::move(lb)), _ub(std::move(ub)) {
    assert_compatible(this->_lb, this->_ub);
  }

  /// \brief Copy constructor
  Interval(const Interval&) = default;

  /// \brief Move constructor
  Interval(Interval&&) noexcept = default;

  /// \brief Copy assignment operator
  Interval& operator=(const Interval&) = default;

  /// \brief Move assignment operator
  Interval& operator=(Interval&&) noexcept = default;

  /// \brief Destructor
  ~Interval() override = default;

  /// \brief Return the bit width of the interval
  uint64_t bit_width() const { return this->_lb.bit_width(); }

  /// \brief Return the signedness (Signed or Unsigned) of the interval
  Signedness sign() const { return this->_lb.sign(); }

  /// \brief Return the lower bound
  const MachineInt& lb() const {
    ikos_assert(!this->is_bottom());
    return this->_lb;
  }

  /// \brief Return the upper bound
  const MachineInt& ub() const {
    ikos_assert(!this->is_bottom());
    return this->_ub;
  }

  void normalize() override {}

  bool is_bottom() const override { return this->_lb > this->_ub; }

  bool is_top() const override {
    return this->_lb.is_min() && this->_ub.is_max();
  }

  /// \brief Return true if the interval is [0, 0]
  bool is_zero() const { return this->_lb.is_zero() && this->_ub.is_zero(); }

  void set_to_bottom() override {
    this->_lb.set_max();
    this->_ub.set_min();
  }

  void set_to_top() override {
    this->_lb.set_min();
    this->_ub.set_max();
  }

  /// \brief Return the interval [MachineInt::min, ub]
  Interval lower_half_line() const {
    ikos_assert(!this->is_bottom());
    return Interval(MachineInt::min(this->bit_width(), this->sign()),
                    this->_ub);
  }

  /// \brief Return the interval [lb, MachineInt::max]
  Interval upper_half_line() const {
    ikos_assert(!this->is_bottom());
    return Interval(this->_lb,
                    MachineInt::max(this->bit_width(), this->sign()));
  }

  bool leq(const Interval& other) const override {
    assert_compatible(*this, other);
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      return other._lb <= this->_lb && this->_ub <= other._ub;
    }
  }

  bool equals(const Interval& other) const override {
    assert_compatible(*this, other);
    if (this->is_bottom()) {
      return other.is_bottom();
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_lb == other._lb && this->_ub == other._ub;
    }
  }

  Interval join(const Interval& other) const override {
    assert_compatible(*this, other);
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
    assert_compatible(*this, other);
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return Interval(other._lb < this->_lb
                          ? MachineInt::min(this->bit_width(), this->sign())
                          : this->_lb,
                      this->_ub < other._ub
                          ? MachineInt::max(this->bit_width(), this->sign())
                          : this->_ub);
    }
  }

  void widen_with(const Interval& other) override {
    this->operator=(this->widening(other));
  }

  Interval widening_threshold(const Interval& other,
                              const MachineInt& threshold) const {
    assert_compatible(*this, other);
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      MachineInt th = threshold.cast(this->bit_width(), this->sign());

      MachineInt lb = this->_lb;
      if (other._lb < this->_lb) {
        if (th <= other._lb) {
          lb = th;
        } else {
          lb = MachineInt::min(this->bit_width(), this->sign());
        }
      }

      MachineInt ub = this->_ub;
      if (other._ub > this->_ub) {
        if (th >= other._ub) {
          ub = th;
        } else {
          ub = MachineInt::max(this->bit_width(), this->sign());
        }
      }
      return Interval(lb, ub);
    }
  }

  void widen_threshold_with(const Interval& other,
                            const MachineInt& threshold) {
    this->operator=(this->widening_threshold(other, threshold));
  }

  Interval meet(const Interval& other) const override {
    assert_compatible(*this, other);
    if (this->is_bottom()) {
      return *this;
    } else if (other.is_bottom()) {
      return other;
    } else {
      return Interval(max(this->_lb, other._lb), min(this->_ub, other._ub));
    }
  }

  void meet_with(const Interval& other) override {
    this->operator=(this->meet(other));
  }

  Interval narrowing(const Interval& other) const override {
    assert_compatible(*this, other);
    if (this->is_bottom()) {
      return *this;
    } else if (other.is_bottom()) {
      return other;
    } else {
      return Interval(this->_lb.is_min() ? other._lb : this->_lb,
                      this->_ub.is_max() ? other._ub : this->_ub);
    }
  }

  void narrow_with(const Interval& other) override {
    this->operator=(this->narrowing(other));
  }

  Interval narrowing_threshold(const Interval& other,
                               const MachineInt& threshold) const {
    assert_compatible(*this, other);
    if (this->is_bottom()) {
      return *this;
    } else if (other.is_bottom()) {
      return other;
    } else {
      MachineInt th = threshold.cast(this->bit_width(), this->sign());

      return Interval(this->_lb.is_min() || this->_lb == th ? other._lb
                                                            : this->_lb,
                      this->_ub.is_max() || this->_ub == th ? other._ub
                                                            : this->_ub);
    }
  }

  void narrow_threshold_with(const Interval& other,
                             const MachineInt& threshold) {
    this->operator=(this->narrowing_threshold(other, threshold));
  }

  /// \name Unary Operators
  /// @{

  /// \brief Truncate the machine integer interval to the given bit width
  Interval trunc(uint64_t bit_width) const {
    ikos_assert(this->bit_width() > bit_width);
    if (this->is_bottom()) {
      return bottom(bit_width, this->sign());
    } else if (this->_lb == this->_ub) {
      return Interval(this->_lb.trunc(bit_width));
    } else {
      // For unsigned integers, check that the first (m-n) bits match.
      // For signed integers, check the first (m-n+1) bits: if the result sign
      // bit is different, return top.
      MachineInt n(this->sign() == Unsigned ? bit_width : (bit_width - 1),
                   this->bit_width(),
                   this->sign());
      if (lshr(this->_lb, n) == lshr(this->_ub, n)) {
        // Identical upper bits
        return Interval(this->_lb.trunc(bit_width), this->_ub.trunc(bit_width));
      } else {
        return Interval::top(bit_width, this->sign());
      }
    }
  }

  /// \brief Extend the machine integer interval to the given bit width
  Interval ext(uint64_t bit_width) const {
    ikos_assert(this->bit_width() < bit_width);
    if (this->is_bottom()) {
      return bottom(bit_width, this->sign());
    } else {
      return Interval(this->_lb.ext(bit_width), this->_ub.ext(bit_width));
    }
  }

  /// \brief Change the machine integer interval sign (bitcast)
  Interval sign_cast(Signedness sign) const {
    ikos_assert(this->sign() != sign);
    if (this->is_bottom()) {
      return bottom(this->bit_width(), sign);
    } else if (this->_lb == this->_ub) {
      return Interval(this->_lb.sign_cast(sign));
    } else {
      if (this->_lb.high_bit() == this->_ub.high_bit()) {
        // Identical high bit
        MachineInt lb = this->_lb.sign_cast(sign);
        MachineInt ub = this->_ub.sign_cast(sign);
        return (lb <= ub) ? Interval(lb, ub) : Interval(ub, lb);
      } else {
        return Interval::top(this->bit_width(), sign);
      }
    }
  }

  /// \brief Cast the machine integer interval to the given bit width and sign
  ///
  /// This is equivalent to trunc()/ext() + sign_cast() (in this specific order)
  Interval cast(uint64_t bit_width, Signedness sign) const {
    if (this->bit_width() == bit_width) {
      if (this->sign() == sign) {
        return *this;
      } else {
        return this->sign_cast(sign);
      }
    } else if (this->bit_width() < bit_width) {
      if (this->sign() == sign) {
        return this->ext(bit_width);
      } else {
        return this->ext(bit_width).sign_cast(sign);
      }
    } else {
      if (this->sign() == sign) {
        return this->trunc(bit_width);
      } else {
        return this->trunc(bit_width).sign_cast(sign);
      }
    }
  }

  /// @}
  /// \name Conversion Functions
  /// @{

  /// \brief Return the machine integer interval as a ZInterval
  ZInterval to_z_interval() const {
    if (this->is_bottom()) {
      return ZInterval::bottom();
    } else {
      return ZInterval(ZBound(this->_lb.to_z_number()),
                       ZBound(this->_ub.to_z_number()));
    }
  }

  struct WrapTag {};
  struct TruncTag {};

  /// \brief Convert a ZInterval to a machine integer interval
  ///
  /// Note that it wraps the interval.
  ///
  /// For instance:
  ///   from_z_interval([255, 256], 8, Unsigned, WrapTag{}) = Top
  static Interval from_z_interval(const ZInterval& i,
                                  uint64_t bit_width,
                                  Signedness sign,
                                  WrapTag) {
    if (i.is_bottom()) {
      return bottom(bit_width, sign);
    }

    const ZBound& lb = i.lb();
    const ZBound& ub = i.ub();

    if (lb.is_infinite() || ub.is_infinite()) {
      return top(bit_width, sign);
    }

    ZNumber z_lb = *lb.number();
    ZNumber z_ub = *ub.number();

    MachineInt i_lb(z_lb, bit_width, sign);
    MachineInt i_ub(z_ub, bit_width, sign);

    if (i_ub.to_z_number() - i_lb.to_z_number() == z_ub - z_lb) {
      return Interval(i_lb, i_ub);
    }

    return top(bit_width, sign);
  }

  /// \brief Convert a ZInterval to a machine integer interval
  ///
  /// Note that it truncates the interval.
  ///
  /// For instance:
  ///  from_z_interval([255, 256], 8, Unsigned, TruncTag{}) = [255, 255]
  static Interval from_z_interval(const ZInterval& i,
                                  uint64_t bit_width,
                                  Signedness sign,
                                  TruncTag) {
    if (i.is_bottom()) {
      return bottom(bit_width, sign);
    }

    ZInterval j = i.meet(Interval::top(bit_width, sign).to_z_interval());

    if (j.is_bottom()) {
      return bottom(bit_width, sign);
    }

    return Interval(MachineInt(*j.lb().number(), bit_width, sign),
                    MachineInt(*j.ub().number(), bit_width, sign));
  }

  /// @}

  /// \brief If the interval is a singleton [n, n], return n, otherwise return
  /// boost::none
  boost::optional< MachineInt > singleton() const {
    if (this->_lb == this->_ub) {
      return this->_lb;
    } else {
      return boost::none;
    }
  }

  /// \brief Return true if the interval contains n
  bool contains(const MachineInt& n) const {
    assert_compatible(this->_lb, n);
    if (this->is_bottom()) {
      return false;
    } else {
      return this->_lb <= n && n <= this->_ub;
    }
  }

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else if (this->is_top()) {
      o << "T";
    } else {
      o << "[" << this->_lb << ", " << this->_ub << "]";
    }
  }

  static std::string name() { return "interval"; }

  // Friends

  friend Interval rem(const Interval& lhs, const Interval& rhs);

  friend Interval shl(const Interval& lhs, const Interval& rhs);

  friend Interval shl_no_wrap(const Interval& lhs, const Interval& rhs);

  friend Interval lshr(const Interval& lhs, const Interval& rhs);

  friend Interval ashr(const Interval& lhs, const Interval& rhs);

  friend Interval and_(const Interval& lhs, const Interval& rhs);

  friend Interval or_(const Interval& lhs, const Interval& rhs);

  friend Interval xor_(const Interval& lhs, const Interval& rhs);

}; // end class Interval

/// \name Binary Operators
/// @{

/// \brief Addition with wrapping
inline Interval add(const Interval& lhs, const Interval& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return Interval::from_z_interval(lhs.to_z_interval() + rhs.to_z_interval(),
                                     lhs.bit_width(),
                                     lhs.sign(),
                                     Interval::WrapTag{});
  }
}

/// \brief Addition without wrapping
inline Interval add_no_wrap(const Interval& lhs, const Interval& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return Interval::from_z_interval(lhs.to_z_interval() + rhs.to_z_interval(),
                                     lhs.bit_width(),
                                     lhs.sign(),
                                     Interval::TruncTag{});
  }
}

/// \brief Substraction with wrapping
inline Interval sub(const Interval& lhs, const Interval& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return Interval::from_z_interval(lhs.to_z_interval() - rhs.to_z_interval(),
                                     lhs.bit_width(),
                                     lhs.sign(),
                                     Interval::WrapTag{});
  }
}

/// \brief Substraction without wrapping
inline Interval sub_no_wrap(const Interval& lhs, const Interval& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return Interval::from_z_interval(lhs.to_z_interval() - rhs.to_z_interval(),
                                     lhs.bit_width(),
                                     lhs.sign(),
                                     Interval::TruncTag{});
  }
}

/// \brief Multiplication with wrapping
inline Interval mul(const Interval& lhs, const Interval& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return Interval::from_z_interval(lhs.to_z_interval() * rhs.to_z_interval(),
                                     lhs.bit_width(),
                                     lhs.sign(),
                                     Interval::WrapTag{});
  }
}

/// \brief Multiplication without wrapping
inline Interval mul_no_wrap(const Interval& lhs, const Interval& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    return Interval::from_z_interval(lhs.to_z_interval() * rhs.to_z_interval(),
                                     lhs.bit_width(),
                                     lhs.sign(),
                                     Interval::TruncTag{});
  }
}

/// \brief Division
inline Interval div(const Interval& lhs, const Interval& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    // overflow is undefined behavior
    return Interval::from_z_interval(lhs.to_z_interval() / rhs.to_z_interval(),
                                     lhs.bit_width(),
                                     lhs.sign(),
                                     Interval::TruncTag{});
  }
}

/// \brief Exact division
inline Interval div_exact(const Interval& lhs, const Interval& rhs) {
  return div(lhs, rhs);
}

/// \brief Remainder
inline Interval rem(const Interval& lhs, const Interval& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    boost::optional< MachineInt > n = lhs.singleton();
    boost::optional< MachineInt > d = rhs.singleton();

    if (d && (*d).is_zero()) {
      // [a, b] % 0 = ⊥
      return Interval::bottom(lhs.bit_width(), lhs.sign());
    } else if (n && d) {
      // [n, n] % [d, d] = [n % d, n % d]
      return Interval(rem(*n, *d));
    } else if (lhs.sign() == Signed) {
      // Signed remainder
      // [a, b] % [c, d] <= max(abs(a), abs(b))
      // [a, b] % [c, d] <= max(abs(c), abs(d)) - 1
      MachineInt zero = MachineInt::zero(lhs.bit_width(), lhs.sign());
      MachineInt one(1, lhs.bit_width(), lhs.sign());
      MachineInt max_int = MachineInt::max(lhs.bit_width(), lhs.sign());

      // Check .is_min() to prevent overflows on abs()
      MachineInt n_ub =
          lhs._lb.is_min() ? max_int : max(abs(lhs._lb), abs(lhs._ub));
      MachineInt d_ub =
          rhs._lb.is_min() ? max_int : max(abs(rhs._lb), abs(rhs._ub)) - one;
      MachineInt ub = min(n_ub, d_ub);
      ikos_assert(ub.is_non_negative());

      if (lhs._lb.is_negative()) {
        if (lhs._ub.is_strictly_positive()) {
          return Interval(-ub, ub);
        } else {
          return Interval(-ub, zero);
        }
      } else {
        return Interval(zero, ub);
      }
    } else {
      // Unsigned remainder
      // [a, b] % [c, d] <= b
      // [a, b] % [c, d] <= d - 1
      // [a, b] % [c, d] >= 0
      MachineInt zero = MachineInt::zero(lhs.bit_width(), lhs.sign());
      MachineInt one(1, lhs.bit_width(), lhs.sign());
      MachineInt ub = min(lhs._ub, rhs._ub - one);
      return Interval(zero, ub);
    }
  }
}

/// \brief Left shift with wrapping
///
/// The right hand side has to be between 0 and bit_width - 1
inline Interval shl(const Interval& lhs, const Interval& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    // meet(rhs, [0, bit_width-1])
    Interval constraint(MachineInt::zero(lhs.bit_width(), lhs.sign()),
                        MachineInt(lhs.bit_width() - 1,
                                   lhs.bit_width(),
                                   lhs.sign()));
    Interval shift = rhs.meet(constraint);

    if (shift.is_bottom()) {
      // Invalid operand
      return shift;
    }

    // [a, b] << [c, d] = [a, b] * [1 << c, 1 << d]
    numeric::ZInterval coeff(ZBound(1 << shift._lb.to_z_number()),
                             ZBound(1 << shift._ub.to_z_number()));
    return Interval::from_z_interval(lhs.to_z_interval() * coeff,
                                     lhs.bit_width(),
                                     lhs.sign(),
                                     Interval::WrapTag{});
  }
}

/// \brief Left shift without wrapping
///
/// The right hand side has to be between 0 and bit_width - 1
inline Interval shl_no_wrap(const Interval& lhs, const Interval& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    // meet(rhs, [0, bit_width-1])
    Interval constraint(MachineInt::zero(lhs.bit_width(), lhs.sign()),
                        MachineInt(lhs.bit_width() - 1,
                                   lhs.bit_width(),
                                   lhs.sign()));
    Interval shift = rhs.meet(constraint);

    if (shift.is_bottom()) {
      // Invalid operand
      return shift;
    }

    // [a, b] << [c, d] = [a, b] * [1 << c, 1 << d]
    numeric::ZInterval coeff(ZBound(1 << shift._lb.to_z_number()),
                             ZBound(1 << shift._ub.to_z_number()));
    return Interval::from_z_interval(lhs.to_z_interval() * coeff,
                                     lhs.bit_width(),
                                     lhs.sign(),
                                     Interval::TruncTag{});
  }
}

/// \brief Logical shift right
///
/// The right hand side has to be between 0 and bit_width - 1
inline Interval lshr(const Interval& lhs, const Interval& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    // meet(rhs, [0, bit_width-1])
    Interval constraint(MachineInt::zero(lhs.bit_width(), lhs.sign()),
                        MachineInt(lhs.bit_width() - 1,
                                   lhs.bit_width(),
                                   lhs.sign()));
    Interval shift = rhs.meet(constraint);

    if (shift.is_bottom()) {
      // Invalid operand
      return shift;
    }

    if (lhs.sign() == Signed) {
      // Signed logical shift right
      return lshr(lhs.sign_cast(Unsigned), shift.sign_cast(Unsigned))
          .sign_cast(Signed);
    } else {
      // Unsigned logical shift right
      // [a, b] >> [c, d] \in [a >> d, b >> c]
      return Interval(lshr(lhs._lb, shift._ub), lshr(lhs._ub, shift._lb));
    }
  }
}

/// \brief Exact logical shift right
///
/// The right hand side has to be between 0 and bit_width - 1
inline Interval lshr_exact(const Interval& lhs, const Interval& rhs) {
  return lshr(lhs, rhs);
}

/// \brief Arithmetic shift right
///
/// The right hand side has to be between 0 and bit_width - 1
inline Interval ashr(const Interval& lhs, const Interval& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    // meet(rhs, [0, bit_width-1])
    Interval constraint(MachineInt::zero(lhs.bit_width(), lhs.sign()),
                        MachineInt(lhs.bit_width() - 1,
                                   lhs.bit_width(),
                                   lhs.sign()));
    Interval shift = rhs.meet(constraint);

    if (shift.is_bottom()) {
      // Invalid operand
      return shift;
    }

    if (lhs.sign() == Unsigned) {
      // Unsigned arithmetic shift right
      return ashr(lhs.sign_cast(Signed), shift.sign_cast(Signed))
          .sign_cast(Unsigned);
    } else {
      // Signed arithmetic shift right
      if (lhs.contains(MachineInt::zero(lhs.bit_width(), lhs.sign()))) {
        Interval l(lhs._lb, MachineInt(-1, lhs.bit_width(), lhs.sign()));
        Interval u(MachineInt(1, lhs.bit_width(), lhs.sign()), lhs._ub);
        return ashr(l, shift)
            .join(ashr(u, shift))
            .join(Interval(MachineInt::zero(lhs.bit_width(), lhs.sign())));
      } else {
        // [a, b] >> [c, d] \in [min(a >> c, a >> d, b >> c, b >> d),
        //                       max(a >> c, a >> d, b >> c, b >> d)]
        MachineInt ll = ashr(lhs._lb, shift._lb);
        MachineInt lu = ashr(lhs._lb, shift._ub);
        MachineInt ul = ashr(lhs._ub, shift._lb);
        MachineInt uu = ashr(lhs._ub, shift._ub);
        return Interval(min(min(min(ll, lu), ul), uu),
                        max(max(max(ll, lu), ul), uu));
      }
    }
  }
}

/// \brief Exact arithmetic shift right
///
/// The right hand side has to be between 0 and bit_width - 1
inline Interval ashr_exact(const Interval& lhs, const Interval& rhs) {
  return ashr(lhs, rhs);
}

/// \brief Bitwise AND
inline Interval and_(const Interval& lhs, const Interval& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    boost::optional< MachineInt > l = lhs.singleton();
    boost::optional< MachineInt > r = rhs.singleton();

    if (l && r) {
      // [l, l] & [r, r] = [l & r, l & r]
      return Interval(and_(*l, *r));
    } else if ((l && (*l).is_zero()) || (r && (*r).is_zero())) {
      // [a, b] & 0 = 0
      return Interval(MachineInt::zero(lhs.bit_width(), lhs.sign()));
    } else if (l && (*l).all_ones()) {
      // 0b11..11 & [a, b] = [a, b]
      return rhs;
    } else if (r && (*r).all_ones()) {
      // [a, b] & 0b11..11 = [a, b]
      return lhs;
    } else if (lhs.sign() == Signed) {
      // Signed AND
      return and_(lhs.sign_cast(Unsigned), rhs.sign_cast(Unsigned))
          .sign_cast(Signed);
    } else {
      // Unsigned AND
      // [a, b] & [c, d] <= [c, d] & b <= b
      // [a, b] & [c, d] <= [a, b] & d <= d
      // [a, b] & [c, d] >= 0
      MachineInt zero = MachineInt::zero(lhs.bit_width(), lhs.sign());
      MachineInt ub = min(lhs._ub, rhs._ub);
      return Interval(zero, ub);
    }
  }
}

/// \brief Bitwise OR
inline Interval or_(const Interval& lhs, const Interval& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    boost::optional< MachineInt > l = lhs.singleton();
    boost::optional< MachineInt > r = rhs.singleton();

    if (l && r) {
      // [l, l] | [r, r] = [l | r, l | r]
      return Interval(or_(*l, *r));
    } else if ((l && (*l).all_ones()) || (r && (*r).all_ones())) {
      // 0b11..11 | [a, b] = 0b11..11
      return Interval(MachineInt::all_ones(lhs.bit_width(), lhs.sign()));
    } else if (l && (*l).is_zero()) {
      // 0 | [a, b] = [a, b]
      return rhs;
    } else if (r && (*r).is_zero()) {
      // [a, b] | 0 = [a, b]
      return lhs;
    } else if (lhs.sign() == Signed) {
      // Signed OR
      return or_(lhs.sign_cast(Unsigned), rhs.sign_cast(Unsigned))
          .sign_cast(Signed);
    } else {
      // Unsigned OR
      uint64_t lead_zeros =
          std::min(lhs._ub.leading_zeros(), rhs._ub.leading_zeros());
      ikos_assert(lead_zeros < lhs.bit_width());
      MachineInt zero = MachineInt::zero(lhs.bit_width(), lhs.sign());
      MachineInt ub = lshr(MachineInt::all_ones(lhs.bit_width(), lhs.sign()),
                           MachineInt(lead_zeros, lhs.bit_width(), lhs.sign()));
      return Interval(zero, ub);
    }
  }
}

/// \brief Bitwise XOR
inline Interval xor_(const Interval& lhs, const Interval& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom()) {
    return lhs;
  } else if (rhs.is_bottom()) {
    return rhs;
  } else {
    boost::optional< MachineInt > l = lhs.singleton();
    boost::optional< MachineInt > r = rhs.singleton();

    if (l && r) {
      // [l, l] ^ [r, r] = [l ^ r, l ^ r]
      return Interval(xor_(*l, *r));
    } else if (l && (*l).is_zero()) {
      // 0 ^ [a, b] = [a, b]
      return rhs;
    } else if (r && (*r).is_zero()) {
      // [a, b] ^ 0 = [a, b]
      return lhs;
    } else if (lhs.sign() == Signed) {
      // Signed XOR
      return xor_(lhs.sign_cast(Unsigned), rhs.sign_cast(Unsigned))
          .sign_cast(Signed);
    } else {
      // Unsigned XOR
      uint64_t lead_zeros =
          std::min(lhs._ub.leading_zeros(), rhs._ub.leading_zeros());
      ikos_assert(lead_zeros < lhs.bit_width());
      MachineInt zero = MachineInt::zero(lhs.bit_width(), lhs.sign());
      MachineInt ub = lshr(MachineInt::all_ones(lhs.bit_width(), lhs.sign()),
                           MachineInt(lead_zeros, lhs.bit_width(), lhs.sign()));
      return Interval(zero, ub);
    }
  }
}

/// @}
/// \name Input / Output
/// @{

/// \brief Write an interval on a stream
inline std::ostream& operator<<(std::ostream& o, const Interval& interval) {
  interval.dump(o);
  return o;
}

/// @}

} // end namespace machine_int
} // end namespace core
} // end namespace ikos
