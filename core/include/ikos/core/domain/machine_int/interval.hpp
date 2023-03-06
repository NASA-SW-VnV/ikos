/*******************************************************************************
 *
 * \file
 * \brief Standard domain of intervals for machine integers
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
#include <ikos/core/domain/machine_int/separate_domain.hpp>
#include <ikos/core/value/machine_int/interval.hpp>

namespace ikos {
namespace core {
namespace machine_int {

/// \brief Trim the bounds of the given interval
inline Interval trim_bound(const Interval& i, const MachineInt& b) {
  assert_compatible(i, b);
  ikos_assert(!i.is_bottom());

  if (i.lb() == b) {
    if (b.is_max()) {
      return Interval::bottom(b.bit_width(), b.sign());
    } else {
      return Interval(b + MachineInt(1, b.bit_width(), b.sign()), i.ub());
    }
  } else if (i.ub() == b) {
    ikos_assert(!b.is_min());
    return Interval(i.lb(), b - MachineInt(1, b.bit_width(), b.sign()));
  } else {
    return i;
  }
}

/// \brief Machine integer interval abstract domain
template < typename VariableRef >
class IntervalDomain final
    : public machine_int::AbstractDomain< VariableRef,
                                          IntervalDomain< VariableRef > > {
private:
  using Parent =
      machine_int::AbstractDomain< VariableRef, IntervalDomain< VariableRef > >;
  using SeparateDomainT = machine_int::SeparateDomain< VariableRef, Interval >;
  using VariableTrait = machine_int::VariableTraits< VariableRef >;

public:
  using LinearExpressionT = LinearExpression< MachineInt, VariableRef >;
  using Iterator = typename SeparateDomainT::Iterator;

private:
  SeparateDomainT _inv;

private:
  /// \brief Private constructor
  explicit IntervalDomain(SeparateDomainT inv) : _inv(std::move(inv)) {}

public:
  /// \brief Create the top abstract value
  static IntervalDomain top() { return IntervalDomain(SeparateDomainT::top()); }

  /// \brief Create the bottom abstract value
  static IntervalDomain bottom() {
    return IntervalDomain(SeparateDomainT::bottom());
  }

  /// \brief Copy constructor
  IntervalDomain(const IntervalDomain&) noexcept = default;

  /// \brief Move constructor
  IntervalDomain(IntervalDomain&&) noexcept = default;

  /// \brief Copy assignment operator
  IntervalDomain& operator=(const IntervalDomain&) noexcept = default;

  /// \brief Move assignment operator
  IntervalDomain& operator=(IntervalDomain&&) noexcept = default;

  /// \brief Destructor
  ~IntervalDomain() override = default;

  /// \brief Begin iterator over the pairs (variable, interval)
  Iterator begin() const { return this->_inv.begin(); }

  /// \brief End iterator over the pairs (variable, interval)
  Iterator end() const { return this->_inv.end(); }

  void normalize() override {}

  bool is_bottom() const override { return this->_inv.is_bottom(); }

  bool is_top() const override { return this->_inv.is_top(); }

  void set_to_bottom() override { this->_inv.set_to_bottom(); }

  void set_to_top() override { this->_inv.set_to_top(); }

  bool leq(const IntervalDomain& other) const override {
    return this->_inv.leq(other._inv);
  }

  bool equals(const IntervalDomain& other) const override {
    return this->_inv.equals(other._inv);
  }

  void join_with(const IntervalDomain& other) override {
    this->_inv.join_with(other._inv);
  }

  void widen_with(const IntervalDomain& other) override {
    this->_inv.widen_with(other._inv);
  }

  void widen_threshold_with(const IntervalDomain& other,
                            const MachineInt& threshold) override {
    this->_inv.widen_threshold_with(other._inv, threshold);
  }

  void meet_with(const IntervalDomain& other) override {
    this->_inv.meet_with(other._inv);
  }

  void narrow_with(const IntervalDomain& other) override {
    this->_inv.narrow_with(other._inv);
  }

  void narrow_threshold_with(const IntervalDomain& other,
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

    Interval xi = this->_inv.get(x);
    Interval yi = this->_inv.get(y);

    switch (pred) {
      case Predicate::EQ: {
        Interval i = xi.meet(yi);
        this->_inv.set(x, i);
        this->_inv.set(y, i);
      } break;
      case Predicate::NE: {
        if (x == y) {
          this->set_to_bottom();
          return;
        }
        if (xi.singleton()) {
          this->_inv.set(y, trim_bound(yi, *xi.singleton()));
        }
        if (yi.singleton()) {
          this->_inv.set(x, trim_bound(xi, *yi.singleton()));
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
        if (yi.ub().is_min() || xi.lb().is_max()) {
          this->set_to_bottom();
          return;
        }
        MachineInt int_min = MachineInt::min(xi.bit_width(), xi.sign());
        MachineInt int_max = MachineInt::max(xi.bit_width(), xi.sign());
        MachineInt one(1, xi.bit_width(), xi.sign());
        this->_inv.refine(x, Interval(int_min, yi.ub() - one));
        this->_inv.refine(y, Interval(xi.lb() + one, int_max));
      } break;
      case Predicate::LE: {
        this->_inv.refine(x, yi.lower_half_line());
        this->_inv.refine(y, xi.upper_half_line());
      } break;
    }
  }

  void add(Predicate pred, VariableRef x, const MachineInt& y) override {
    if (this->is_bottom()) {
      return;
    }

    Interval xi = this->_inv.get(x);
    Interval yi(y);

    switch (pred) {
      case Predicate::EQ: {
        Interval i = xi.meet(yi);
        this->_inv.set(x, i);
      } break;
      case Predicate::NE: {
        this->_inv.set(x, trim_bound(xi, y));
      } break;
      case Predicate::GT: {
        if (y.is_max()) {
          this->set_to_bottom();
          return;
        }
        MachineInt int_max = MachineInt::max(xi.bit_width(), xi.sign());
        MachineInt one(1, xi.bit_width(), xi.sign());
        this->_inv.refine(x, Interval(y + one, int_max));
      } break;
      case Predicate::GE: {
        this->_inv.refine(x, yi.upper_half_line());
      } break;
      case Predicate::LT: {
        if (y.is_min()) {
          this->set_to_bottom();
          return;
        }
        MachineInt int_min = MachineInt::min(xi.bit_width(), xi.sign());
        MachineInt one(1, xi.bit_width(), xi.sign());
        this->_inv.refine(x, Interval(int_min, y - one));
      } break;
      case Predicate::LE: {
        this->_inv.refine(x, yi.lower_half_line());
      } break;
    }
  }

  void add(Predicate pred, const MachineInt& x, VariableRef y) override {
    Parent::add(pred, x, y);
  }

  void set(VariableRef x, const Interval& value) override {
    this->_inv.set(x, value);
  }

  void set(VariableRef x, const Congruence& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      boost::optional< MachineInt > n = value.singleton();
      if (n) {
        this->_inv.set(x, Interval(*n));
      } else {
        this->forget(x);
      }
    }
  }

  void set(VariableRef x, const IntervalCongruence& value) override {
    this->_inv.set(x, value.interval());
  }

  void refine(VariableRef x, const Interval& value) override {
    this->_inv.refine(x, value);
  }

  void refine(VariableRef x, const Congruence& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      IntervalCongruence iv(this->_inv.get(x), value);
      this->_inv.set(x, iv.interval());
    }
  }

  void refine(VariableRef x, const IntervalCongruence& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      IntervalCongruence iv = IntervalCongruence(this->_inv.get(x)).meet(value);
      this->_inv.set(x, iv.interval());
    }
  }

  void forget(VariableRef x) override { this->_inv.forget(x); }

  Interval to_interval(VariableRef x) const override {
    return this->_inv.get(x);
  }

  Interval to_interval(const LinearExpressionT& e) const override {
    return this->_inv.project(e);
  }

  Congruence to_congruence(VariableRef x) const override {
    return this->to_interval_congruence(x).congruence();
  }

  Congruence to_congruence(const LinearExpressionT& e) const override {
    return this->to_interval_congruence(e).congruence();
  }

  IntervalCongruence to_interval_congruence(VariableRef x) const override {
    return IntervalCongruence(this->_inv.get(x));
  }

  IntervalCongruence to_interval_congruence(
      const LinearExpressionT& e) const override {
    // Result type
    uint64_t bit_width = e.constant().bit_width();
    Signedness sign = e.constant().sign();

    if (this->is_bottom()) {
      return IntervalCongruence::bottom(bit_width, sign);
    }

    IntervalCongruence r(e.constant());
    for (const auto& term : e) {
      r = machine_int::add(r,
                           machine_int::mul(IntervalCongruence(term.second),
                                            IntervalCongruence(
                                                this->_inv.get(term.first)
                                                    .cast(bit_width, sign))));
    }
    return r;
  }

  void dump(std::ostream& o) const override { return this->_inv.dump(o); }

  static std::string name() { return "interval domain"; }

}; // end class IntervalDomain

} // end namespace machine_int
} // end namespace core
} // end namespace ikos
