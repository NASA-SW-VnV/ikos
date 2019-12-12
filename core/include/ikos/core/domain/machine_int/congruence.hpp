/*******************************************************************************
 *
 * \file
 * \brief Standard domain of congruences for machine integers
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
#include <ikos/core/value/machine_int/congruence.hpp>

namespace ikos {
namespace core {
namespace machine_int {

/// \brief Machine integer congruence abstract domain
template < typename VariableRef >
class CongruenceDomain final
    : public machine_int::AbstractDomain< VariableRef,
                                          CongruenceDomain< VariableRef > > {
private:
  using Parent = machine_int::AbstractDomain< VariableRef,
                                              CongruenceDomain< VariableRef > >;
  using SeparateDomainT =
      machine_int::SeparateDomain< VariableRef, Congruence >;

public:
  using LinearExpressionT = LinearExpression< MachineInt, VariableRef >;
  using Iterator = typename SeparateDomainT::Iterator;

private:
  SeparateDomainT _inv;

private:
  /// \brief Private constructor
  explicit CongruenceDomain(SeparateDomainT inv) : _inv(std::move(inv)) {}

public:
  /// \brief Create the top abstract value
  static CongruenceDomain top() {
    return CongruenceDomain(SeparateDomainT::top());
  }

  /// \brief Create the bottom abstract value
  static CongruenceDomain bottom() {
    return CongruenceDomain(SeparateDomainT::bottom());
  }

  /// \brief Copy constructor
  CongruenceDomain(const CongruenceDomain&) noexcept = default;

  /// \brief Move constructor
  CongruenceDomain(CongruenceDomain&&) noexcept = default;

  /// \brief Copy assignment operator
  CongruenceDomain& operator=(const CongruenceDomain&) noexcept = default;

  /// \brief Move assignment operator
  CongruenceDomain& operator=(CongruenceDomain&&) noexcept = default;

  /// \brief Destructor
  ~CongruenceDomain() override = default;

  /// \brief Begin iterator over the pairs (variable, congruence)
  Iterator begin() const { return this->_inv.begin(); }

  /// \brief End iterator over the pairs (variable, congruence)
  Iterator end() const { return this->_inv.end(); }

  void normalize() override {}

  bool is_bottom() const override { return this->_inv.is_bottom(); }

  bool is_top() const override { return this->_inv.is_top(); }

  void set_to_bottom() override { this->_inv.set_to_bottom(); }

  void set_to_top() override { this->_inv.set_to_top(); }

  bool leq(const CongruenceDomain& other) const override {
    return this->_inv.leq(other._inv);
  }

  bool equals(const CongruenceDomain& other) const override {
    return this->_inv.equals(other._inv);
  }

  void join_with(const CongruenceDomain& other) override {
    this->_inv.join_with(other._inv);
  }

  void widen_with(const CongruenceDomain& other) override {
    this->_inv.widen_with(other._inv);
  }

  void widen_threshold_with(const CongruenceDomain& other,
                            const MachineInt& threshold) override {
    this->_inv.widen_threshold_with(other._inv, threshold);
  }

  void meet_with(const CongruenceDomain& other) override {
    this->_inv.meet_with(other._inv);
  }

  void narrow_with(const CongruenceDomain& other) override {
    this->_inv.narrow_with(other._inv);
  }

  void narrow_threshold_with(const CongruenceDomain& other,
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

    Congruence xc = this->_inv.get(x);
    Congruence yc = this->_inv.get(y);

    switch (pred) {
      case Predicate::EQ: {
        Congruence c = xc.meet(yc);
        this->_inv.set(x, c);
        this->_inv.set(y, c);
      } break;
      case Predicate::NE: {
        if (x == y) {
          this->set_to_bottom();
          return;
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
      } break;
      case Predicate::LE: {
        // nothing
      } break;
    }
  }

  void add(Predicate pred, VariableRef x, const MachineInt& y) override {
    if (this->is_bottom()) {
      return;
    }

    Congruence xc = this->_inv.get(x);
    Congruence yc(y);

    switch (pred) {
      case Predicate::EQ: {
        this->_inv.set(x, xc.meet(yc));
      } break;
      case Predicate::NE: {
        // nothing
      } break;
      case Predicate::GT: {
        // nothing
      } break;
      case Predicate::GE: {
        // nothing
      } break;
      case Predicate::LT: {
        // nothing
      } break;
      case Predicate::LE: {
        // nothing
      } break;
    }
  }

  void add(Predicate pred, const MachineInt& x, VariableRef y) override {
    Parent::add(pred, x, y);
  }

  void set(VariableRef x, const Interval& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      boost::optional< MachineInt > n = value.singleton();
      if (n) {
        this->_inv.set(x, Congruence(*n));
      } else {
        this->forget(x);
      }
    }
  }

  void set(VariableRef x, const Congruence& value) override {
    this->_inv.set(x, value);
  }

  void set(VariableRef x, const IntervalCongruence& value) override {
    this->_inv.set(x, value.congruence());
  }

  void refine(VariableRef x, const Interval& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      IntervalCongruence iv(value, this->_inv.get(x));
      this->_inv.set(x, iv.congruence());
    }
  }

  void refine(VariableRef x, const Congruence& value) override {
    this->_inv.refine(x, value);
  }

  void refine(VariableRef x, const IntervalCongruence& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      IntervalCongruence iv = IntervalCongruence(this->_inv.get(x)).meet(value);
      this->_inv.set(x, iv.congruence());
    }
  }

  void forget(VariableRef x) override { this->_inv.forget(x); }

  Interval to_interval(VariableRef x) const override {
    return this->to_interval_congruence(x).interval();
  }

  Interval to_interval(const LinearExpressionT& e) const override {
    return this->to_interval_congruence(e).interval();
  }

  Congruence to_congruence(VariableRef x) const override {
    return this->_inv.get(x);
  }

  Congruence to_congruence(const LinearExpressionT& e) const override {
    return this->_inv.project(e);
  }

  IntervalCongruence to_interval_congruence(VariableRef x) const override {
    return IntervalCongruence(this->_inv.get(x));
  }

  IntervalCongruence to_interval_congruence(
      const LinearExpressionT& e) const override {
    return IntervalCongruence(this->_inv.project(e));
  }

  void dump(std::ostream& o) const override { return this->_inv.dump(o); }

  static std::string name() { return "congruence domain"; }

}; // end class CongruenceDomain

} // end namespace machine_int
} // end namespace core
} // end namespace ikos
