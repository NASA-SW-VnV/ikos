/*******************************************************************************
 *
 * \file
 * \brief Numerical abstract domain adapter for machine integers
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

#include <type_traits>

#include <ikos/core/domain/machine_int/abstract_domain.hpp>
#include <ikos/core/domain/numeric/abstract_domain.hpp>
#include <ikos/core/number/machine_int.hpp>
#include <ikos/core/number/z_number.hpp>
#include <ikos/core/semantic/machine_int/variable.hpp>
#include <ikos/core/support/assert.hpp>

namespace ikos {
namespace core {
namespace machine_int {

/// \brief Numerical abstract domain adapter for machine integers
///
/// This is a wrapper to use a `numeric::AbstractDomain` as a
/// `machine_int::AbstractDomain`.
template < typename VariableRef, typename NumDomain >
class NumericDomainAdapter final
    : public machine_int::AbstractDomain<
          VariableRef,
          NumericDomainAdapter< VariableRef, NumDomain > > {
public:
  static_assert(
      numeric::IsAbstractDomain< NumDomain, ZNumber, VariableRef >::value,
      "NumDomain must implement numeric::AbstractDomain");

private:
  using Parent = machine_int::AbstractDomain<
      VariableRef,
      NumericDomainAdapter< VariableRef, NumDomain > >;
  using VariableTrait = machine_int::VariableTraits< VariableRef >;

public:
  using MachIntVariableExpression =
      VariableExpression< MachineInt, VariableRef >;
  using MachIntLinearExpression = LinearExpression< MachineInt, VariableRef >;
  using MachIntUnaryOperator = machine_int::UnaryOperator;
  using MachIntBinaryOperator = machine_int::BinaryOperator;
  using MachIntPredicate = machine_int::Predicate;
  using MachIntInterval = machine_int::Interval;
  using MachIntCongruence = machine_int::Congruence;
  using MachIntIntervalCongruence = machine_int::IntervalCongruence;

  using ZBinaryOperator = numeric::BinaryOperator;
  using ZVariableExpression = VariableExpression< ZNumber, VariableRef >;
  using ZLinearExpression = LinearExpression< ZNumber, VariableRef >;
  using ZInterval = numeric::Interval< ZNumber >;
  using ZCongruence = numeric::Congruence< ZNumber >;
  using ZIntervalCongruence = numeric::IntervalCongruence< ZNumber >;

private:
  NumDomain _inv;

public:
  /// \brief Create an abstract value from the given numeric abstract domain
  explicit NumericDomainAdapter(NumDomain inv) : _inv(std::move(inv)) {}

  /// \brief Copy constructor
  NumericDomainAdapter(const NumericDomainAdapter&) noexcept(
      std::is_nothrow_copy_constructible< NumDomain >::value) = default;

  /// \brief Move constructor
  NumericDomainAdapter(NumericDomainAdapter&&) noexcept(
      std::is_nothrow_move_constructible< NumDomain >::value) = default;

  /// \brief Copy assignment operator
  NumericDomainAdapter& operator=(const NumericDomainAdapter&) noexcept(
      std::is_nothrow_copy_assignable< NumDomain >::value) = default;

  /// \brief Move assignment operator
  NumericDomainAdapter& operator=(NumericDomainAdapter&&) noexcept(
      std::is_nothrow_move_assignable< NumDomain >::value) = default;

  /// \brief Destructor
  ~NumericDomainAdapter() override = default;

  void normalize() override { this->_inv.normalize(); }

  bool is_bottom() const override { return this->_inv.is_bottom(); }

  bool is_top() const override { return this->_inv.is_top(); }

  void set_to_bottom() override { this->_inv.set_to_bottom(); }

  void set_to_top() override { this->_inv.set_to_top(); }

  bool leq(const NumericDomainAdapter& other) const override {
    return this->_inv.leq(other._inv);
  }

  bool equals(const NumericDomainAdapter& other) const override {
    return this->_inv.equals(other._inv);
  }

  void join_with(NumericDomainAdapter&& other) override {
    this->_inv.join_with(std::move(other._inv));
  }

  void join_with(const NumericDomainAdapter& other) override {
    this->_inv.join_with(other._inv);
  }

  void join_loop_with(NumericDomainAdapter&& other) override {
    this->_inv.join_loop_with(std::move(other._inv));
  }

  void join_loop_with(const NumericDomainAdapter& other) override {
    this->_inv.join_loop_with(other._inv);
  }

  void join_iter_with(NumericDomainAdapter&& other) override {
    this->_inv.join_iter_with(std::move(other._inv));
  }

  void join_iter_with(const NumericDomainAdapter& other) override {
    this->_inv.join_iter_with(other._inv);
  }

  void widen_with(const NumericDomainAdapter& other) override {
    this->_inv.widen_with(other._inv);
  }

  void widen_threshold_with(const NumericDomainAdapter& other,
                            const MachineInt& threshold) override {
    this->_inv.widen_threshold_with(other._inv, threshold.to_z_number());
  }

  void meet_with(const NumericDomainAdapter& other) override {
    this->_inv.meet_with(other._inv);
  }

  void narrow_with(const NumericDomainAdapter& other) override {
    this->_inv.narrow_with(other._inv);
  }

  void narrow_threshold_with(const NumericDomainAdapter& other,
                             const MachineInt& threshold) override {
    this->_inv.narrow_threshold_with(other._inv, threshold.to_z_number());
  }

  NumericDomainAdapter join(const NumericDomainAdapter& other) const override {
    return NumericDomainAdapter(this->_inv.join(other._inv));
  }

  NumericDomainAdapter join_loop(
      const NumericDomainAdapter& other) const override {
    return NumericDomainAdapter(this->_inv.join_loop(other._inv));
  }

  NumericDomainAdapter join_iter(
      const NumericDomainAdapter& other) const override {
    return NumericDomainAdapter(this->_inv.join_iter(other._inv));
  }

  NumericDomainAdapter widening(
      const NumericDomainAdapter& other) const override {
    return NumericDomainAdapter(this->_inv.widening(other._inv));
  }

  NumericDomainAdapter widening_threshold(
      const NumericDomainAdapter& other,
      const MachineInt& threshold) const override {
    return NumericDomainAdapter(
        this->_inv.widening_threshold(other._inv, threshold.to_z_number()));
  }

  NumericDomainAdapter meet(const NumericDomainAdapter& other) const override {
    return NumericDomainAdapter(this->_inv.meet(other._inv));
  }

  NumericDomainAdapter narrowing(
      const NumericDomainAdapter& other) const override {
    return NumericDomainAdapter(this->_inv.narrowing(other._inv));
  }

  NumericDomainAdapter narrowing_threshold(
      const NumericDomainAdapter& other,
      const MachineInt& threshold) const override {
    return NumericDomainAdapter(
        this->_inv.narrowing_threshold(other._inv, threshold.to_z_number()));
  }

private:
  /// \brief Return 2**n
  static ZNumber power_of_2(uint64_t n) { return ZNumber(1) << n; }

  /// \brief Return 2**n
  static ZNumber power_of_2(const ZNumber& n) { return ZNumber(1) << n; }

  /// \brief Convert a linear expression on machine integers to a linear
  /// expression on integers
  static ZLinearExpression to_z_linear_expression(
      const MachIntLinearExpression& e) {
    ZLinearExpression r(e.constant().to_z_number());
    for (const auto& term : e) {
      r.add(term.second.to_z_number(), term.first);
    }
    return r;
  }

  /// \brief Wrap the given variable
  void wrap(VariableRef x) {
    this->wrap(x, x, VariableTrait::bit_width(x), VariableTrait::sign(x));
  }

  /// \brief Apply `x = wrap(y, bit-width, sign)`
  ///
  /// Apply the wrap-around semantic for machine integers of the given bit-width
  /// and sign.
  void wrap(VariableRef x, VariableRef y, uint64_t bit_width, Signedness sign) {
    if (sign == Signed) {
      ZNumber n = power_of_2(bit_width);
      ZNumber m = power_of_2(bit_width - 1);
      // x = ((y + m) mod n) - m
      this->_inv.apply(ZBinaryOperator::Add, x, y, m);
      this->_inv.apply(ZBinaryOperator::Mod, x, x, n);
      this->_inv.apply(ZBinaryOperator::Sub, x, x, m);
    } else if (sign == Unsigned) {
      // x = y mod n
      ZNumber n = power_of_2(bit_width);
      this->_inv.apply(ZBinaryOperator::Mod, x, y, n);
    } else {
      ikos_unreachable("unreachable");
    }
  }

  /// \brief Truncate the given variable
  void trunc(VariableRef x) {
    this->trunc(x, VariableTrait::bit_width(x), VariableTrait::sign(x));
  }

  /// \brief Apply `x = trunc(x, bit-width, sign)`
  ///
  /// This truncate the value of `x`, basically ignoring overflows.
  void trunc(VariableRef x, uint64_t bit_width, Signedness sign) {
    this->_inv.refine(x, MachIntInterval::top(bit_width, sign).to_z_interval());
  }

public:
  void assign(VariableRef x, const MachineInt& n) override {
    this->_inv.assign(x, n.to_z_number());
  }

  void assign(VariableRef x, VariableRef y) override {
    this->_inv.assign(x, y);
  }

  void assign(VariableRef x, const MachIntLinearExpression& e) override {
    this->_inv.assign(x, to_z_linear_expression(e));
    this->wrap(x);
  }

  void apply(MachIntUnaryOperator op, VariableRef x, VariableRef y) override {
    this->trunc(y); // add bound information on y

    switch (op) {
      case MachIntUnaryOperator::Trunc: {
        this->wrap(x, y, VariableTrait::bit_width(x), VariableTrait::sign(x));
      } break;
      case MachIntUnaryOperator::Ext: {
        this->_inv.assign(x, y);
      } break;
      case MachIntUnaryOperator::SignCast: {
        this->wrap(x, y, VariableTrait::bit_width(x), VariableTrait::sign(x));
      } break;
      case MachIntUnaryOperator::Cast: {
        this->wrap(x, y, VariableTrait::bit_width(x), VariableTrait::sign(x));
      } break;
      default: {
        ikos_unreachable("unreachable");
      }
    }
  }

  void apply(MachIntBinaryOperator op,
             VariableRef x,
             VariableRef y,
             VariableRef z) override {
    switch (op) {
      case MachIntBinaryOperator::Add: {
        this->_inv.apply(ZBinaryOperator::Add, x, y, z);
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::AddNoWrap: {
        this->_inv.apply(ZBinaryOperator::Add, x, y, z);
        this->trunc(x);
      } break;
      case MachIntBinaryOperator::Sub: {
        this->_inv.apply(ZBinaryOperator::Sub, x, y, z);
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::SubNoWrap: {
        this->_inv.apply(ZBinaryOperator::Sub, x, y, z);
        this->trunc(x);
      } break;
      case MachIntBinaryOperator::Mul: {
        this->_inv.apply(ZBinaryOperator::Mul, x, y, z);
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::MulNoWrap: {
        this->_inv.apply(ZBinaryOperator::Mul, x, y, z);
        this->trunc(x);
      } break;
      case MachIntBinaryOperator::Div: {
        this->_inv.apply(ZBinaryOperator::Div, x, y, z);
        this->trunc(x); // overflow is undefined behavior
      } break;
      case MachIntBinaryOperator::DivExact: {
        this->apply(MachIntBinaryOperator::Div, x, y, z); // sound
      } break;
      case MachIntBinaryOperator::Rem: {
        this->_inv.apply(ZBinaryOperator::Rem, x, y, z);
        this->trunc(x);
      } break;
      case MachIntBinaryOperator::Shl: {
        // z has to be between [0, bit_width - 1]
        uint64_t bit_width = VariableTrait::bit_width(x);
        this->_inv.refine(z,
                          ZInterval(ZBound(0), ZBound(ZNumber(bit_width - 1))));

        this->_inv.apply(ZBinaryOperator::Shl, x, y, z);
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::ShlNoWrap: {
        // z has to be between [0, bit_width - 1]
        uint64_t bit_width = VariableTrait::bit_width(x);
        this->_inv.refine(z,
                          ZInterval(ZBound(0), ZBound(ZNumber(bit_width - 1))));

        this->_inv.apply(ZBinaryOperator::Shl, x, y, z);
        this->trunc(x);
      } break;
      case MachIntBinaryOperator::LShr: {
        // z has to be between [0, bit_width - 1]
        uint64_t bit_width = VariableTrait::bit_width(x);
        this->_inv.refine(z,
                          ZInterval(ZBound(0), ZBound(ZNumber(bit_width - 1))));

        if (VariableTrait::sign(x) == Signed) {
          if (x == z) {
            this->forget(x);
            return;
          }
          this->wrap(x, y, bit_width, Unsigned);
          this->_inv.apply(ZBinaryOperator::Shr, x, x, z);
          this->wrap(x, x, bit_width, Signed);
        } else {
          this->_inv.apply(ZBinaryOperator::Shr, x, y, z);
        }
      } break;
      case MachIntBinaryOperator::LShrExact: {
        this->apply(MachIntBinaryOperator::LShr, x, y, z); // sound
      } break;
      case MachIntBinaryOperator::AShr: {
        // z has to be between [0, bit_width - 1]
        uint64_t bit_width = VariableTrait::bit_width(x);
        this->_inv.refine(z,
                          ZInterval(ZBound(0), ZBound(ZNumber(bit_width - 1))));

        if (VariableTrait::sign(x) == Signed) {
          this->_inv.apply(ZBinaryOperator::Shr, x, y, z);
        } else {
          if (x == z) {
            this->forget(x);
            return;
          }
          this->wrap(x, y, bit_width, Signed);
          this->_inv.apply(ZBinaryOperator::Shr, x, x, z);
          this->wrap(x, x, bit_width, Unsigned);
        }
      } break;
      case MachIntBinaryOperator::AShrExact: {
        this->apply(MachIntBinaryOperator::AShr, x, y, z); // sound
      } break;
      case MachIntBinaryOperator::And: {
        this->_inv.apply(ZBinaryOperator::And, x, y, z);
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::Or: {
        this->_inv.apply(ZBinaryOperator::Or, x, y, z);
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::Xor: {
        this->_inv.apply(ZBinaryOperator::Xor, x, y, z);
        this->wrap(x);
      } break;
      default: {
        ikos_unreachable("unreachable");
      }
    }
  }

  void apply(MachIntBinaryOperator op,
             VariableRef x,
             VariableRef y,
             const MachineInt& z) override {
    switch (op) {
      case MachIntBinaryOperator::Add: {
        this->_inv.apply(ZBinaryOperator::Add, x, y, z.to_z_number());
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::AddNoWrap: {
        this->_inv.apply(ZBinaryOperator::Add, x, y, z.to_z_number());
        this->trunc(x);
      } break;
      case MachIntBinaryOperator::Sub: {
        this->_inv.apply(ZBinaryOperator::Sub, x, y, z.to_z_number());
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::SubNoWrap: {
        this->_inv.apply(ZBinaryOperator::Sub, x, y, z.to_z_number());
        this->trunc(x);
      } break;
      case MachIntBinaryOperator::Mul: {
        this->_inv.apply(ZBinaryOperator::Mul, x, y, z.to_z_number());
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::MulNoWrap: {
        this->_inv.apply(ZBinaryOperator::Mul, x, y, z.to_z_number());
        this->trunc(x);
      } break;
      case MachIntBinaryOperator::Div: {
        this->_inv.apply(ZBinaryOperator::Div, x, y, z.to_z_number());
        this->trunc(x); // overflow is undefined behavior
      } break;
      case MachIntBinaryOperator::DivExact: {
        this->apply(MachIntBinaryOperator::Div, x, y, z); // sound
      } break;
      case MachIntBinaryOperator::Rem: {
        this->_inv.apply(ZBinaryOperator::Rem, x, y, z.to_z_number());
        this->trunc(x);
      } break;
      case MachIntBinaryOperator::Shl: {
        // z has to be between [0, bit_width - 1]
        uint64_t bit_width = VariableTrait::bit_width(x);
        ZNumber zz = z.to_z_number();
        if (!(0 <= zz && zz < bit_width)) {
          this->set_to_bottom();
          return;
        }

        this->_inv.apply(ZBinaryOperator::Shl, x, y, zz);
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::ShlNoWrap: {
        // z has to be between [0, bit_width - 1]
        uint64_t bit_width = VariableTrait::bit_width(x);
        ZNumber zz = z.to_z_number();
        if (!(0 <= zz && zz < bit_width)) {
          this->set_to_bottom();
          return;
        }

        this->_inv.apply(ZBinaryOperator::Shl, x, y, zz);
        this->trunc(x);
      } break;
      case MachIntBinaryOperator::LShr: {
        // z has to be between [0, bit_width - 1]
        uint64_t bit_width = VariableTrait::bit_width(x);
        ZNumber zz = z.to_z_number();
        if (!(0 <= zz && zz < bit_width)) {
          this->set_to_bottom();
          return;
        }

        if (VariableTrait::sign(x) == Signed) {
          this->wrap(x, y, bit_width, Unsigned);
          this->_inv.apply(ZBinaryOperator::Shr, x, x, zz);
          this->wrap(x, x, bit_width, Signed);
        } else {
          this->_inv.apply(ZBinaryOperator::Shr, x, y, zz);
        }
      } break;
      case MachIntBinaryOperator::LShrExact: {
        this->apply(MachIntBinaryOperator::LShr, x, y, z); // sound
      } break;
      case MachIntBinaryOperator::AShr: {
        // z has to be between [0, bit_width - 1]
        uint64_t bit_width = VariableTrait::bit_width(x);
        ZNumber zz = z.to_z_number();
        if (!(0 <= zz && zz < bit_width)) {
          this->set_to_bottom();
          return;
        }

        if (VariableTrait::sign(x) == Signed) {
          this->_inv.apply(ZBinaryOperator::Shr, x, y, zz);
        } else {
          this->wrap(x, y, bit_width, Signed);
          this->_inv.apply(ZBinaryOperator::Shr, x, x, zz);
          this->wrap(x, x, bit_width, Unsigned);
        }
      } break;
      case MachIntBinaryOperator::AShrExact: {
        this->apply(MachIntBinaryOperator::AShr, x, y, z); // sound
      } break;
      case MachIntBinaryOperator::And: {
        this->_inv.apply(ZBinaryOperator::And, x, y, z.to_z_number());
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::Or: {
        this->_inv.apply(ZBinaryOperator::Or, x, y, z.to_z_number());
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::Xor: {
        this->_inv.apply(ZBinaryOperator::Xor, x, y, z.to_z_number());
        this->wrap(x);
      } break;
      default: {
        ikos_unreachable("unreachable");
      }
    }
  }

  void apply(MachIntBinaryOperator op,
             VariableRef x,
             const MachineInt& y,
             VariableRef z) override {
    switch (op) {
      case MachIntBinaryOperator::Add: {
        this->_inv.apply(ZBinaryOperator::Add, x, y.to_z_number(), z);
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::AddNoWrap: {
        this->_inv.apply(ZBinaryOperator::Add, x, y.to_z_number(), z);
        this->trunc(x);
      } break;
      case MachIntBinaryOperator::Sub: {
        this->_inv.apply(ZBinaryOperator::Sub, x, y.to_z_number(), z);
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::SubNoWrap: {
        this->_inv.apply(ZBinaryOperator::Sub, x, y.to_z_number(), z);
        this->trunc(x);
      } break;
      case MachIntBinaryOperator::Mul: {
        this->_inv.apply(ZBinaryOperator::Mul, x, y.to_z_number(), z);
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::MulNoWrap: {
        this->_inv.apply(ZBinaryOperator::Mul, x, y.to_z_number(), z);
        this->trunc(x);
      } break;
      case MachIntBinaryOperator::Div: {
        this->_inv.apply(ZBinaryOperator::Div, x, y.to_z_number(), z);
        this->trunc(x); // overflow is undefined behavior
      } break;
      case MachIntBinaryOperator::DivExact: {
        this->apply(MachIntBinaryOperator::Div, x, y, z); // sound
      } break;
      case MachIntBinaryOperator::Rem: {
        this->_inv.apply(ZBinaryOperator::Rem, x, y.to_z_number(), z);
        this->trunc(x);
      } break;
      case MachIntBinaryOperator::Shl: {
        // z has to be between [0, bit_width - 1]
        uint64_t bit_width = VariableTrait::bit_width(x);
        this->_inv.refine(z,
                          ZInterval(ZBound(0), ZBound(ZNumber(bit_width - 1))));

        this->_inv.apply(ZBinaryOperator::Shl, x, y.to_z_number(), z);
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::ShlNoWrap: {
        // z has to be between [0, bit_width - 1]
        uint64_t bit_width = VariableTrait::bit_width(x);
        this->_inv.refine(z,
                          ZInterval(ZBound(0), ZBound(ZNumber(bit_width - 1))));

        this->_inv.apply(ZBinaryOperator::Shl, x, y.to_z_number(), z);
        this->trunc(x);
      } break;
      case MachIntBinaryOperator::LShr: {
        // z has to be between [0, bit_width - 1]
        uint64_t bit_width = VariableTrait::bit_width(x);
        this->_inv.refine(z,
                          ZInterval(ZBound(0), ZBound(ZNumber(bit_width - 1))));

        if (VariableTrait::sign(x) == Signed) {
          this->_inv.apply(ZBinaryOperator::Shr,
                           x,
                           y.sign_cast(Unsigned).to_z_number(),
                           z);
          this->wrap(x, x, bit_width, Signed);
        } else {
          this->_inv.apply(ZBinaryOperator::Shr, x, y.to_z_number(), z);
        }
      } break;
      case MachIntBinaryOperator::LShrExact: {
        this->apply(MachIntBinaryOperator::LShr, x, y, z); // sound
      } break;
      case MachIntBinaryOperator::AShr: {
        // z has to be between [0, bit_width - 1]
        uint64_t bit_width = VariableTrait::bit_width(x);
        this->_inv.refine(z,
                          ZInterval(ZBound(0), ZBound(ZNumber(bit_width - 1))));

        if (VariableTrait::sign(x) == Signed) {
          this->_inv.apply(ZBinaryOperator::Shr, x, y.to_z_number(), z);
        } else {
          this->_inv.apply(ZBinaryOperator::Shr,
                           x,
                           y.sign_cast(Signed).to_z_number(),
                           z);
          this->wrap(x, x, bit_width, Unsigned);
        }
      } break;
      case MachIntBinaryOperator::AShrExact: {
        this->apply(MachIntBinaryOperator::AShr, x, y, z); // sound
      } break;
      case MachIntBinaryOperator::And: {
        this->_inv.apply(ZBinaryOperator::And, x, y.to_z_number(), z);
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::Or: {
        this->_inv.apply(ZBinaryOperator::Or, x, y.to_z_number(), z);
        this->wrap(x);
      } break;
      case MachIntBinaryOperator::Xor: {
        this->_inv.apply(ZBinaryOperator::Xor, x, y.to_z_number(), z);
        this->wrap(x);
      } break;
      default: {
        ikos_unreachable("unreachable");
      }
    }
  }

  void add(MachIntPredicate pred, VariableRef x, VariableRef y) override {
    switch (pred) {
      case MachIntPredicate::EQ: {
        this->_inv.add(ZVariableExpression(x) == ZVariableExpression(y));
      } break;
      case MachIntPredicate::NE: {
        this->_inv.add(ZVariableExpression(x) != ZVariableExpression(y));
      } break;
      case MachIntPredicate::GT: {
        this->_inv.add(ZVariableExpression(x) >= ZVariableExpression(y) + 1);
      } break;
      case MachIntPredicate::GE: {
        this->_inv.add(ZVariableExpression(x) >= ZVariableExpression(y));
      } break;
      case MachIntPredicate::LT: {
        this->_inv.add(ZVariableExpression(x) <= ZVariableExpression(y) - 1);
      } break;
      case MachIntPredicate::LE: {
        this->_inv.add(ZVariableExpression(x) <= ZVariableExpression(y));
      } break;
      default: {
        ikos_unreachable("unreachable");
      }
    }
  }

  void add(MachIntPredicate pred, VariableRef x, const MachineInt& y) override {
    switch (pred) {
      case MachIntPredicate::EQ: {
        this->_inv.add(ZVariableExpression(x) == y.to_z_number());
      } break;
      case MachIntPredicate::NE: {
        this->_inv.add(ZVariableExpression(x) != y.to_z_number());
      } break;
      case MachIntPredicate::GT: {
        this->_inv.add(ZVariableExpression(x) >= y.to_z_number() + 1);
      } break;
      case MachIntPredicate::GE: {
        this->_inv.add(ZVariableExpression(x) >= y.to_z_number());
      } break;
      case MachIntPredicate::LT: {
        this->_inv.add(ZVariableExpression(x) <= y.to_z_number() - 1);
      } break;
      case MachIntPredicate::LE: {
        this->_inv.add(ZVariableExpression(x) <= y.to_z_number());
      } break;
      default: {
        ikos_unreachable("unreachable");
      }
    }
  }

  void add(MachIntPredicate pred, const MachineInt& x, VariableRef y) override {
    Parent::add(pred, x, y);
  }

  void set(VariableRef x, const MachIntInterval& value) override {
    this->_inv.set(x, value.to_z_interval());
  }

  void set(VariableRef x, const MachIntCongruence& value) override {
    this->_inv.set(x, value.to_z_congruence());
  }

  void set(VariableRef x, const MachIntIntervalCongruence& value) override {
    this->_inv.set(x, value.to_z_interval_congruence());
  }

  void refine(VariableRef x, const MachIntInterval& value) override {
    this->_inv.refine(x, value.to_z_interval());
  }

  void refine(VariableRef x, const MachIntCongruence& value) override {
    this->_inv.refine(x, value.to_z_congruence());
  }

  void refine(VariableRef x, const MachIntIntervalCongruence& value) override {
    this->_inv.refine(x, value.to_z_interval_congruence());
  }

  void forget(VariableRef x) override { this->_inv.forget(x); }

  MachIntInterval to_interval(VariableRef x) const override {
    return MachIntInterval::from_z_interval(this->_inv.to_interval(x),
                                            VariableTrait::bit_width(x),
                                            VariableTrait::sign(x),
                                            MachIntInterval::TruncTag{});
  }

  MachIntInterval to_interval(const MachIntLinearExpression& e) const override {
    return MachIntInterval::from_z_interval(this->_inv.to_interval(
                                                to_z_linear_expression(e)),
                                            e.constant().bit_width(),
                                            e.constant().sign(),
                                            MachIntInterval::WrapTag{});
  }

  MachIntCongruence to_congruence(VariableRef x) const override {
    return MachIntCongruence::from_z_congruence(this->_inv.to_congruence(x),
                                                VariableTrait::bit_width(x),
                                                VariableTrait::sign(x),
                                                MachIntCongruence::TruncTag{});
  }

  MachIntCongruence to_congruence(
      const MachIntLinearExpression& e) const override {
    return MachIntCongruence::from_z_congruence(this->_inv.to_congruence(
                                                    to_z_linear_expression(e)),
                                                e.constant().bit_width(),
                                                e.constant().sign(),
                                                MachIntCongruence::WrapTag{});
  }

  MachIntIntervalCongruence to_interval_congruence(
      VariableRef x) const override {
    return MachIntIntervalCongruence::
        from_z_interval_congruence(this->_inv.to_interval_congruence(x),
                                   VariableTrait::bit_width(x),
                                   VariableTrait::sign(x),
                                   MachIntIntervalCongruence::TruncTag{});
  }

  MachIntIntervalCongruence to_interval_congruence(
      const MachIntLinearExpression& e) const override {
    return MachIntIntervalCongruence::
        from_z_interval_congruence(this->_inv.to_interval_congruence(
                                       to_z_linear_expression(e)),
                                   e.constant().bit_width(),
                                   e.constant().sign(),
                                   MachIntIntervalCongruence::WrapTag{});
  }

  /// \name Non-negative loop counter abstract domain methods
  /// @{

  void counter_mark(VariableRef x) override { this->_inv.counter_mark(x); }

  void counter_unmark(VariableRef x) override { this->_inv.counter_unmark(x); }

  void counter_init(VariableRef x, const MachineInt& c) override {
    this->_inv.counter_init(x, c.to_z_number());
  }

  void counter_incr(VariableRef x, const MachineInt& k) override {
    this->_inv.counter_incr(x, k.to_z_number());
  }

  void counter_forget(VariableRef x) override { this->_inv.counter_forget(x); }

  /// @}

  void dump(std::ostream& o) const override { return this->_inv.dump(o); }

  static std::string name() { return "adapter of " + NumDomain::name(); }

}; // end class NumericDomainAdapter

} // end namespace machine_int
} // end namespace core
} // end namespace ikos
