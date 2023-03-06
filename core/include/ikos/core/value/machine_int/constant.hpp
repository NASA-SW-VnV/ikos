/*******************************************************************************
 *
 * \file
 * \brief Machine integer constant class
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

namespace ikos {
namespace core {
namespace machine_int {

/// \brief Machine integer constant abstract value
class Constant final : public core::AbstractDomain< Constant > {
private:
  enum Kind { BottomKind, TopKind, IntegerKind };

private:
  Kind _kind;
  MachineInt _n;

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top constant for the given bit-width and signedness
  Constant(TopTag, uint64_t bit_width, Signedness sign)
      : _kind(TopKind), _n(0, bit_width, sign) {}

  /// \brief Create the bottom constant for the given bit-width and signedness
  Constant(BottomTag, uint64_t bit_width, Signedness sign)
      : _kind(BottomKind), _n(0, bit_width, sign) {}

public:
  /// \brief Create the top constant for the given bit-width and signedness
  static Constant top(uint64_t bit_width, Signedness sign) {
    return Constant(TopTag{}, bit_width, sign);
  }

  /// \brief Create the bottom constant for the given bit-width and signedness
  static Constant bottom(uint64_t bit_width, Signedness sign) {
    return Constant(BottomTag{}, bit_width, sign);
  }

  /// \brief Create the constant n
  explicit Constant(MachineInt n) : _kind(IntegerKind), _n(std::move(n)) {}

  /// \brief Copy constructor
  Constant(const Constant&) = default;

  /// \brief Move constructor
  Constant(Constant&&) noexcept = default;

  /// \brief Copy assignment operator
  Constant& operator=(const Constant&) = default;

  /// \brief Move assignment operator
  Constant& operator=(Constant&&) noexcept = default;

  /// \brief Destructor
  ~Constant() override = default;

  /// \brief Return the bit width of the constant
  uint64_t bit_width() const { return this->_n.bit_width(); }

  /// \brief Return the signedness (Signed or Unsigned) of the constant
  Signedness sign() const { return this->_n.sign(); }

  void normalize() override {}

  bool is_bottom() const override { return this->_kind == BottomKind; }

  bool is_top() const override { return this->_kind == TopKind; }

  /// \brief Return true if the constant is an integer
  bool is_integer() const { return this->_kind == IntegerKind; }

  void set_to_bottom() override { this->_kind = BottomKind; }

  void set_to_top() override { this->_kind = TopKind; }

  bool leq(const Constant& other) const override {
    assert_compatible(*this, other);
    switch (this->_kind) {
      case BottomKind:
        return true;
      case TopKind:
        return other._kind == TopKind;
      case IntegerKind:
        return other._kind == TopKind ||
               (other._kind == IntegerKind && this->_n == other._n);
      default:
        ikos_unreachable("unreachable");
    }
  }

  bool equals(const Constant& other) const override {
    assert_compatible(*this, other);
    if (this->_kind == IntegerKind) {
      return other._kind == IntegerKind && this->_n == other._n;
    } else {
      return this->_kind == other._kind;
    }
  }

  Constant join(const Constant& other) const override {
    assert_compatible(*this, other);
    if (this->is_bottom() || other.is_top()) {
      return other;
    } else if (this->is_top() || other.is_bottom()) {
      return *this;
    } else if (this->_n == other._n) {
      return *this;
    } else {
      return top(this->bit_width(), this->sign());
    }
  }

  void join_with(const Constant& other) override {
    this->operator=(this->join(other));
  }

  Constant widening(const Constant& other) const override {
    // equivalent to join, domain is flat
    return this->join(other);
  }

  void widen_with(const Constant& other) override {
    // equivalent to join, domain is flat
    this->join_with(other);
  }

  Constant widening_threshold(const Constant& other,
                              const MachineInt& /*threshold*/) const {
    // equivalent to join, domain is flat
    return this->join(other);
  }

  void widen_threshold_with(const Constant& other,
                            const MachineInt& /*threshold*/) {
    // equivalent to join, domain is flat
    this->join_with(other);
  }

  Constant meet(const Constant& other) const override {
    assert_compatible(*this, other);
    if (this->is_bottom() || other.is_top()) {
      return *this;
    } else if (this->is_top() || other.is_bottom()) {
      return other;
    } else if (this->_n == other._n) {
      return *this;
    } else {
      return bottom(this->bit_width(), this->sign());
    }
  }

  void meet_with(const Constant& other) override {
    this->operator=(this->meet(other));
  }

  Constant narrowing(const Constant& other) const override {
    // equivalent to meet, domain is flat
    return this->meet(other);
  }

  void narrow_with(const Constant& other) override {
    // equivalent to meet, domain is flat
    this->meet_with(other);
  }

  Constant narrowing_threshold(const Constant& other,
                               const MachineInt& /*threshold*/) const {
    // equivalent to meet, domain is flat
    return this->meet(other);
  }

  void narrow_threshold_with(const Constant& other,
                             const MachineInt& /*threshold*/) {
    // equivalent to meet, domain is flat
    this->meet_with(other);
  }

  /// \name Unary Operators
  /// @{

  /// \brief Truncate the constant to the given bit width
  Constant trunc(uint64_t bit_width) {
    ikos_assert(this->bit_width() > bit_width);
    if (this->is_bottom()) {
      return bottom(bit_width, this->sign());
    } else if (this->is_top()) {
      return top(bit_width, this->sign());
    } else {
      return Constant(this->_n.trunc(bit_width));
    }
  }

  /// \brief Extend the constant to the given bit width
  Constant ext(uint64_t bit_width) {
    ikos_assert(this->bit_width() < bit_width);
    if (this->is_bottom()) {
      return bottom(bit_width, this->sign());
    } else if (this->is_top()) {
      return top(bit_width, this->sign());
    } else {
      return Constant(this->_n.ext(bit_width));
    }
  }

  /// \brief Change the constant sign (bitcast)
  Constant sign_cast(Signedness sign) const {
    ikos_assert(this->sign() != sign);
    if (this->is_bottom()) {
      return bottom(this->bit_width(), sign);
    } else if (this->is_top()) {
      return top(this->bit_width(), sign);
    } else {
      return Constant(this->_n.sign_cast(sign));
    }
  }

  /// \brief Cast the constant to the given bit width and sign
  Constant cast(uint64_t bit_width, Signedness sign) const {
    if (this->is_bottom()) {
      return bottom(bit_width, sign);
    } else if (this->is_top()) {
      return top(bit_width, sign);
    } else {
      return Constant(this->_n.cast(bit_width, sign));
    }
  }

  /// @}

  /// \brief If the constant in a number n, return n, otherwise return
  /// boost::none
  boost::optional< MachineInt > integer() const {
    if (this->is_integer()) {
      return this->_n;
    } else {
      return boost::none;
    }
  }

  /// \brief If the constant in a singleton n, return n, otherwise return
  /// boost::none
  boost::optional< MachineInt > singleton() const { return this->integer(); }

  /// \brief Return true if the constant contains n
  bool contains(const MachineInt& n) const {
    return this->is_top() || (this->is_integer() && this->_n == n);
  }

  void dump(std::ostream& o) const override {
    switch (this->_kind) {
      case BottomKind: {
        o << "⊥";
        break;
      }
      case TopKind: {
        o << "T";
        break;
      }
      case IntegerKind: {
        o << this->_n;
        break;
      }
    }
  }

  static std::string name() { return "constant"; }

  // Friends

  friend Constant add(const Constant& lhs, const Constant& rhs);

  friend Constant add_no_wrap(const Constant& lhs, const Constant& rhs);

  friend Constant sub(const Constant& lhs, const Constant& rhs);

  friend Constant sub_no_wrap(const Constant& lhs, const Constant& rhs);

  friend Constant mul(const Constant& lhs, const Constant& rhs);

  friend Constant mul_no_wrap(const Constant& lhs, const Constant& rhs);

  friend Constant div(const Constant& lhs, const Constant& rhs);

  friend Constant div_exact(const Constant& lhs, const Constant& rhs);

  friend Constant rem(const Constant& lhs, const Constant& rhs);

  friend Constant shl(const Constant& lhs, const Constant& rhs);

  friend Constant shl_no_wrap(const Constant& lhs, const Constant& rhs);

  friend Constant lshr(const Constant& lhs, const Constant& rhs);

  friend Constant lshr_exact(const Constant& lhs, const Constant& rhs);

  friend Constant ashr(const Constant& lhs, const Constant& rhs);

  friend Constant ashr_exact(const Constant& lhs, const Constant& rhs);

  friend Constant and_(const Constant& lhs, const Constant& rhs);

  friend Constant or_(const Constant& lhs, const Constant& rhs);

  friend Constant xor_(const Constant& lhs, const Constant& rhs);

}; // end class Constant

/// \name Binary Operators
/// @{

/// \brief Addition with wrapping
inline Constant add(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    return Constant(add(lhs._n, rhs._n));
  }
}

/// \brief Addition without wrapping
inline Constant add_no_wrap(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    bool overflow;
    MachineInt n = add(lhs._n, rhs._n, overflow);
    if (!overflow) {
      return Constant(n);
    } else {
      return Constant::bottom(lhs.bit_width(), lhs.sign());
    }
  }
}

/// \brief Substraction with wrapping
inline Constant sub(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    return Constant(sub(lhs._n, rhs._n));
  }
}

/// \brief Substraction without wrapping
inline Constant sub_no_wrap(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    bool overflow;
    MachineInt n = sub(lhs._n, rhs._n, overflow);
    if (!overflow) {
      return Constant(n);
    } else {
      return Constant::bottom(lhs.bit_width(), lhs.sign());
    }
  }
}

/// \brief Multiplication with wrapping
inline Constant mul(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if ((lhs.is_integer() && lhs._n.is_zero()) ||
             (rhs.is_integer() && rhs._n.is_zero())) {
    return Constant(MachineInt::zero(lhs.bit_width(), lhs.sign()));
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    return Constant(mul(lhs._n, rhs._n));
  }
}

/// \brief Multiplication without wrapping
inline Constant mul_no_wrap(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if ((lhs.is_integer() && lhs._n.is_zero()) ||
             (rhs.is_integer() && rhs._n.is_zero())) {
    return Constant(MachineInt::zero(lhs.bit_width(), lhs.sign()));
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    bool overflow;
    MachineInt n = mul(lhs._n, rhs._n, overflow);
    if (!overflow) {
      return Constant(n);
    } else {
      return Constant::bottom(lhs.bit_width(), lhs.sign());
    }
  }
}

/// \brief Division
inline Constant div(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (rhs.is_integer() && rhs._n.is_zero()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (lhs.is_integer() && lhs._n.is_zero()) {
    return Constant(MachineInt::zero(lhs.bit_width(), lhs.sign()));
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    bool overflow;
    bool exact;
    MachineInt n = div(lhs._n, rhs._n, overflow, exact);
    if (!overflow) {
      return Constant(n);
    } else {
      // overflow is considered undefined behavior
      return Constant::bottom(lhs.bit_width(), lhs.sign());
    }
  }
}

/// \brief Exact division
inline Constant div_exact(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (rhs.is_integer() && rhs._n.is_zero()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (lhs.is_integer() && lhs._n.is_zero()) {
    return Constant(MachineInt::zero(lhs.bit_width(), lhs.sign()));
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    bool overflow;
    bool exact;
    MachineInt n = div(lhs._n, rhs._n, overflow, exact);
    if (!overflow && exact) {
      return Constant(n);
    } else {
      // overflow or non-exact division are undefined behaviors
      return Constant::bottom(lhs.bit_width(), lhs.sign());
    }
  }
}

/// \brief Remainder
inline Constant rem(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (rhs.is_integer() && rhs._n.is_zero()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (lhs.is_integer() && lhs._n.is_zero()) {
    return Constant(MachineInt::zero(lhs.bit_width(), lhs.sign()));
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    return Constant(rem(lhs._n, rhs._n));
  }
}

/// \brief Left shift with wrapping
///
/// The right hand side has to be between 0 and bit_width - 1
inline Constant shl(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (rhs.is_integer() &&
             (rhs._n < MachineInt::zero(lhs.bit_width(), lhs.sign()) ||
              rhs._n > MachineInt(lhs.bit_width() - 1,
                                  lhs.bit_width(),
                                  lhs.sign()))) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (lhs.is_integer() && lhs._n.is_zero()) {
    return Constant(MachineInt::zero(lhs.bit_width(), lhs.sign()));
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    return Constant(shl(lhs._n, rhs._n));
  }
}

/// \brief Left shift without wrapping
///
/// The right hand side has to be between 0 and bit_width - 1
inline Constant shl_no_wrap(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (rhs.is_integer() &&
             (rhs._n < MachineInt::zero(lhs.bit_width(), lhs.sign()) ||
              rhs._n > MachineInt(lhs.bit_width() - 1,
                                  lhs.bit_width(),
                                  lhs.sign()))) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (lhs.is_integer() && lhs._n.is_zero()) {
    return Constant(MachineInt::zero(lhs.bit_width(), lhs.sign()));
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    bool overflow;
    MachineInt n = shl(lhs._n, rhs._n, overflow);
    if (!overflow) {
      return Constant(n);
    } else {
      return Constant::bottom(lhs.bit_width(), lhs.sign());
    }
  }
}

/// \brief Logical shift right
///
/// The right hand side has to be between 0 and bit_width - 1
inline Constant lshr(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (rhs.is_integer() &&
             (rhs._n < MachineInt::zero(lhs.bit_width(), lhs.sign()) ||
              rhs._n > MachineInt(lhs.bit_width() - 1,
                                  lhs.bit_width(),
                                  lhs.sign()))) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (lhs.is_integer() && lhs._n.is_zero()) {
    return Constant(MachineInt::zero(lhs.bit_width(), lhs.sign()));
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    return Constant(lshr(lhs._n, rhs._n));
  }
}

/// \brief Exact logical shift right
///
/// The right hand side has to be between 0 and bit_width - 1
inline Constant lshr_exact(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (rhs.is_integer() &&
             (rhs._n < MachineInt::zero(lhs.bit_width(), lhs.sign()) ||
              rhs._n > MachineInt(lhs.bit_width() - 1,
                                  lhs.bit_width(),
                                  lhs.sign()))) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (lhs.is_integer() && lhs._n.is_zero()) {
    return Constant(MachineInt::zero(lhs.bit_width(), lhs.sign()));
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    bool exact;
    MachineInt n = lshr(lhs._n, rhs._n, exact);
    if (exact) {
      return Constant(n);
    } else {
      return Constant::bottom(lhs.bit_width(), lhs.sign());
    }
  }
}

/// \brief Arithmetic shift right
///
/// The right hand side has to be between 0 and bit_width - 1
inline Constant ashr(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (rhs.is_integer() &&
             (rhs._n < MachineInt::zero(lhs.bit_width(), lhs.sign()) ||
              rhs._n > MachineInt(lhs.bit_width() - 1,
                                  lhs.bit_width(),
                                  lhs.sign()))) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (lhs.is_integer() && lhs._n.is_zero()) {
    return Constant(MachineInt::zero(lhs.bit_width(), lhs.sign()));
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    return Constant(ashr(lhs._n, rhs._n));
  }
}

/// \brief Exact arithmetic shift right
///
/// The right hand side has to be between 0 and bit_width - 1
inline Constant ashr_exact(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (rhs.is_integer() &&
             (rhs._n < MachineInt::zero(lhs.bit_width(), lhs.sign()) ||
              rhs._n > MachineInt(lhs.bit_width() - 1,
                                  lhs.bit_width(),
                                  lhs.sign()))) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (lhs.is_integer() && lhs._n.is_zero()) {
    return Constant(MachineInt::zero(lhs.bit_width(), lhs.sign()));
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    bool exact;
    MachineInt n = ashr(lhs._n, rhs._n, exact);
    if (exact) {
      return Constant(n);
    } else {
      return Constant::bottom(lhs.bit_width(), lhs.sign());
    }
  }
}

/// \brief Bitwise AND
inline Constant and_(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if ((lhs.is_integer() && lhs._n.is_zero()) ||
             (rhs.is_integer() && rhs._n.is_zero())) {
    return Constant(MachineInt::zero(lhs.bit_width(), lhs.sign()));
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    return Constant(and_(lhs._n, rhs._n));
  }
}

/// \brief Bitwise OR
inline Constant or_(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if ((lhs.is_integer() && lhs._n.all_ones()) ||
             (rhs.is_integer() && rhs._n.all_ones())) {
    return Constant(MachineInt::all_ones(lhs.bit_width(), lhs.sign()));
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    return Constant(or_(lhs._n, rhs._n));
  }
}

/// \brief Bitwise XOR
inline Constant xor_(const Constant& lhs, const Constant& rhs) {
  assert_compatible(lhs, rhs);
  if (lhs.is_bottom() || rhs.is_bottom()) {
    return Constant::bottom(lhs.bit_width(), lhs.sign());
  } else if (lhs.is_top() || rhs.is_top()) {
    return Constant::top(lhs.bit_width(), lhs.sign());
  } else {
    return Constant(xor_(lhs._n, rhs._n));
  }
}

/// @}
/// \name Input / Output
/// @{

/// \brief Write a constant on a stream
inline std::ostream& operator<<(std::ostream& o, const Constant& constant) {
  constant.dump(o);
  return o;
}

/// @}

} // end namespace machine_int
} // end namespace core
} // end namespace ikos
