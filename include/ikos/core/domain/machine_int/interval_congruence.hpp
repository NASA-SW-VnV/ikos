/*******************************************************************************
 *
 * \file
 * \brief Reduced product of intervals and congruences on machine integers
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

#include <ikos/core/domain/machine_int/abstract_domain.hpp>
#include <ikos/core/domain/machine_int/interval.hpp>
#include <ikos/core/domain/machine_int/separate_domain.hpp>
#include <ikos/core/value/machine_int/interval_congruence.hpp>

namespace ikos {
namespace core {
namespace machine_int {

/// \brief Trim the bounds of the given interval-congruence
inline IntervalCongruence trim_bound(const IntervalCongruence& ic,
                                     const MachineInt& b) {
  return IntervalCongruence(trim_bound(ic.interval(), b), ic.to_z_congruence());
}

/// \brief Reduced product of intervals and congruences on machine integers
template < typename VariableRef >
class IntervalCongruenceDomain final
    : public machine_int::AbstractDomain<
          VariableRef,
          IntervalCongruenceDomain< VariableRef > > {
private:
  using Parent =
      machine_int::AbstractDomain< VariableRef,
                                   IntervalCongruenceDomain< VariableRef > >;
  using SeparateDomainT =
      machine_int::SeparateDomain< VariableRef, IntervalCongruence >;

public:
  using LinearExpressionT = LinearExpression< MachineInt, VariableRef >;
  using Iterator = typename SeparateDomainT::Iterator;

private:
  SeparateDomainT _inv;

private:
  /// \brief Private constructor
  explicit IntervalCongruenceDomain(SeparateDomainT inv)
      : _inv(std::move(inv)) {}

public:
  /// \brief Create the top abstract value
  static IntervalCongruenceDomain top() {
    return IntervalCongruenceDomain(SeparateDomainT::top());
  }

  /// \brief Create the bottom abstract value
  static IntervalCongruenceDomain bottom() {
    return IntervalCongruenceDomain(SeparateDomainT::bottom());
  }

  /// \brief Copy constructor
  IntervalCongruenceDomain(const IntervalCongruenceDomain&) noexcept = default;

  /// \brief Move constructor
  IntervalCongruenceDomain(IntervalCongruenceDomain&&) noexcept = default;

  /// \brief Copy assignment operator
  IntervalCongruenceDomain& operator=(
      const IntervalCongruenceDomain&) noexcept = default;

  /// \brief Move assignment operator
  IntervalCongruenceDomain& operator=(IntervalCongruenceDomain&&) noexcept =
      default;

  /// \brief Destructor
  ~IntervalCongruenceDomain() override = default;

  /// \brief Begin iterator over the pairs (variable, interval-congruence)
  Iterator begin() const { return this->_inv.begin(); }

  /// \brief End iterator over the pairs (variable, interval-congruence)
  Iterator end() const { return this->_inv.end(); }

  void normalize() override {}

  bool is_bottom() const override { return this->_inv.is_bottom(); }

  bool is_top() const override { return this->_inv.is_top(); }

  void set_to_bottom() override { this->_inv.set_to_bottom(); }

  void set_to_top() override { this->_inv.set_to_top(); }

  bool leq(const IntervalCongruenceDomain& other) const override {
    return this->_inv.leq(other._inv);
  }

  bool equals(const IntervalCongruenceDomain& other) const override {
    return this->_inv.equals(other._inv);
  }

  void join_with(const IntervalCongruenceDomain& other) override {
    this->_inv.join_with(other._inv);
  }

  void widen_with(const IntervalCongruenceDomain& other) override {
    this->_inv.widen_with(other._inv);
  }

  void widen_threshold_with(const IntervalCongruenceDomain& other,
                            const MachineInt& threshold) override {
    this->_inv.widen_threshold_with(other._inv, threshold);
  }

  void meet_with(const IntervalCongruenceDomain& other) override {
    this->_inv.meet_with(other._inv);
  }

  void narrow_with(const IntervalCongruenceDomain& other) override {
    this->_inv.narrow_with(other._inv);
  }

  void narrow_threshold_with(const IntervalCongruenceDomain& other,
                             const MachineInt& threshold) override {
    this->_inv.narrow_threshold_with(other._inv, threshold);
  }

  void assign(VariableRef x, const MachineInt& n) override {
    this->_inv.assign(x, n);
  }

  void assign(VariableRef x, VariableRef y) override {
    this->_inv.assign(x, y);
  }

  void assign(VariableRef x, const LinearExpressionT& e) override {
    this->_inv.assign(x, e);
  }

  void apply(UnaryOperator op, VariableRef x, VariableRef y) override {
    this->_inv.apply(op, x, y);
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             VariableRef z) override {
    this->_inv.apply(op, x, y, z);
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             const MachineInt& z) override {
    this->_inv.apply(op, x, y, z);
  }

  void apply(BinaryOperator op,
             VariableRef x,
             const MachineInt& y,
             VariableRef z) override {
    this->_inv.apply(op, x, y, z);
  }

  void add(Predicate pred, VariableRef x, VariableRef y) override {
    if (this->is_bottom()) {
      return;
    }

    IntervalCongruence xic = this->_inv.get(x);
    IntervalCongruence yic = this->_inv.get(y);

    switch (pred) {
      case Predicate::EQ: {
        IntervalCongruence ic = xic.meet(yic);
        this->_inv.set(x, ic);
        this->_inv.set(y, ic);
      } break;
      case Predicate::NE: {
        if (x == y) {
          this->set_to_bottom();
          return;
        }
        if (xic.singleton()) {
          this->_inv.set(y, trim_bound(yic, *xic.singleton()));
        }
        if (yic.singleton()) {
          this->_inv.set(x, trim_bound(xic, *yic.singleton()));
        }
      } break;
      case Predicate::GT: {
        this->add(Predicate::LT, y, x);
      } break;
      case Predicate::GE: {
        this->add(Predicate::LE, y, x);
      } break;
      case Predicate::LT: {
        if (x == y) {
          this->set_to_bottom();
          return;
        }
        if (yic.ub().is_min() || xic.lb().is_max()) {
          this->set_to_bottom();
          return;
        }
        MachineInt int_min = MachineInt::min(xic.bit_width(), xic.sign());
        MachineInt int_max = MachineInt::max(xic.bit_width(), xic.sign());
        MachineInt one(1, xic.bit_width(), xic.sign());
        this->refine(x, Interval(int_min, yic.ub() - one));
        this->refine(y, Interval(xic.lb() + one, int_max));
      } break;
      case Predicate::LE: {
        this->refine(x, yic.interval().lower_half_line());
        this->refine(y, xic.interval().upper_half_line());
      } break;
    }
  }

  void add(Predicate pred, VariableRef x, const MachineInt& y) override {
    if (this->is_bottom()) {
      return;
    }

    IntervalCongruence xic = this->_inv.get(x);
    IntervalCongruence yic(y);

    switch (pred) {
      case Predicate::EQ: {
        IntervalCongruence ic = xic.meet(yic);
        this->_inv.set(x, ic);
      } break;
      case Predicate::NE: {
        this->_inv.set(x, trim_bound(xic, y));
      } break;
      case Predicate::GT: {
        if (y.is_max()) {
          this->set_to_bottom();
          return;
        }
        MachineInt int_max = MachineInt::max(xic.bit_width(), xic.sign());
        MachineInt one(1, xic.bit_width(), xic.sign());
        this->refine(x, Interval(y + one, int_max));
      } break;
      case Predicate::GE: {
        this->refine(x, yic.interval().upper_half_line());
      } break;
      case Predicate::LT: {
        if (y.is_min()) {
          this->set_to_bottom();
          return;
        }
        MachineInt int_min = MachineInt::min(xic.bit_width(), xic.sign());
        MachineInt one(1, xic.bit_width(), xic.sign());
        this->refine(x, Interval(int_min, y - one));
      } break;
      case Predicate::LE: {
        this->refine(x, yic.interval().lower_half_line());
      } break;
    }
  }

  void add(Predicate pred, const MachineInt& x, VariableRef y) override {
    Parent::add(pred, x, y);
  }

  void set(VariableRef x, const Interval& value) override {
    this->_inv.set(x, IntervalCongruence(value));
  }

  void set(VariableRef x, const Congruence& value) override {
    this->_inv.set(x, IntervalCongruence(value));
  }

  void set(VariableRef x, const IntervalCongruence& value) override {
    this->_inv.set(x, value);
  }

  void refine(VariableRef x, const Interval& value) override {
    this->_inv.refine(x, IntervalCongruence(value));
  }

  void refine(VariableRef x, const Congruence& value) override {
    this->_inv.refine(x, IntervalCongruence(value));
  }

  void refine(VariableRef x, const IntervalCongruence& value) override {
    this->_inv.refine(x, value);
  }

  void forget(VariableRef x) override { this->_inv.forget(x); }

  Interval to_interval(VariableRef x) const override {
    return this->_inv.get(x).interval();
  }

  Interval to_interval(const LinearExpressionT& e) const override {
    return this->_inv.project(e).interval();
  }

  Congruence to_congruence(VariableRef x) const override {
    return this->_inv.get(x).congruence();
  }

  Congruence to_congruence(const LinearExpressionT& e) const override {
    return this->_inv.project(e).congruence();
  }

  IntervalCongruence to_interval_congruence(VariableRef x) const override {
    return this->_inv.get(x);
  }

  IntervalCongruence to_interval_congruence(
      const LinearExpressionT& e) const override {
    return this->_inv.project(e);
  }

  void dump(std::ostream& o) const override { return this->_inv.dump(o); }

  static std::string name() { return "interval congruence domain"; }

}; // end class IntervalCongruenceDomain

} // end namespace machine_int
} // end namespace core
} // end namespace ikos
