/*******************************************************************************
 *
 * \file
 * \brief Standard domain of constants
 *
 * Author: Arnaud J. Venet
 *
 * Contributors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2019 United States Government as represented by the
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

#include <ikos/core/domain/numeric/abstract_domain.hpp>
#include <ikos/core/domain/numeric/interval.hpp>
#include <ikos/core/domain/numeric/separate_domain.hpp>
#include <ikos/core/value/numeric/congruence.hpp>
#include <ikos/core/value/numeric/constant.hpp>
#include <ikos/core/value/numeric/interval.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief Constant abstract domain
template < typename Number,
           typename VariableRef,
           std::size_t MaxReductionCycles = 10 >
class ConstantDomain final
    : public numeric::AbstractDomain<
          Number,
          VariableRef,
          ConstantDomain< Number, VariableRef, MaxReductionCycles > > {
public:
  using ConstantT = Constant< Number >;
  using IntervalT = Interval< Number >;
  using IntervalCongruenceT = IntervalCongruence< Number >;
  using CongruenceT = Congruence< Number >;
  using VariableExprT = VariableExpression< Number, VariableRef >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;
  using LinearConstraintSystemT = LinearConstraintSystem< Number, VariableRef >;

private:
  using SeparateDomainT = SeparateDomain< Number, VariableRef, ConstantT >;
  using Parent = numeric::AbstractDomain< Number, VariableRef, ConstantDomain >;

public:
  using Iterator = typename SeparateDomainT::Iterator;

private:
  SeparateDomainT _inv;

private:
  /// \brief Private constructor
  explicit ConstantDomain(SeparateDomainT inv) : _inv(std::move(inv)) {}

public:
  /// \brief Create the top abstract value
  static ConstantDomain top() { return ConstantDomain(SeparateDomainT::top()); }

  /// \brief Create the bottom abstract value
  static ConstantDomain bottom() {
    return ConstantDomain(SeparateDomainT::bottom());
  }

  /// \brief Copy constructor
  ConstantDomain(const ConstantDomain&) noexcept = default;

  /// \brief Move constructor
  ConstantDomain(ConstantDomain&&) noexcept = default;

  /// \brief Copy assignment operator
  ConstantDomain& operator=(const ConstantDomain&) noexcept = default;

  /// \brief Move assignment operator
  ConstantDomain& operator=(ConstantDomain&&) noexcept = default;

  /// \brief Destructor
  ~ConstantDomain() override = default;

  /// \brief Begin iterator over the pairs (variable, constant)
  Iterator begin() const { return this->_inv.begin(); }

  /// \brief End iterator over the pairs (variable, constant)
  Iterator end() const { return this->_inv.end(); }

  void normalize() override {}

  bool is_bottom() const override { return this->_inv.is_bottom(); }

  bool is_top() const override { return this->_inv.is_top(); }

  void set_to_bottom() override { this->_inv.set_to_bottom(); }

  void set_to_top() override { this->_inv.set_to_top(); }

  bool leq(const ConstantDomain& other) const override {
    return this->_inv.leq(other._inv);
  }

  bool equals(const ConstantDomain& other) const override {
    return this->_inv.equals(other._inv);
  }

  void join_with(const ConstantDomain& other) override {
    this->_inv.join_with(other._inv);
  }

  void join_loop_with(const ConstantDomain& other) override {
    this->_inv.join_loop_with(other._inv);
  }

  void join_iter_with(const ConstantDomain& other) override {
    this->_inv.join_iter_with(other._inv);
  }

  void widen_with(const ConstantDomain& other) override {
    this->_inv.widen_with(other._inv);
  }

  void widen_threshold_with(const ConstantDomain& other,
                            const Number& threshold) override {
    this->_inv.widen_threshold_with(other._inv, threshold);
  }

  void meet_with(const ConstantDomain& other) override {
    this->_inv.meet_with(other._inv);
  }

  void narrow_with(const ConstantDomain& other) override {
    this->_inv.narrow_with(other._inv);
  }

  void narrow_threshold_with(const ConstantDomain& other,
                             const Number& threshold) override {
    this->_inv.narrow_threshold_with(other._inv, threshold);
  }

  void assign(VariableRef x, int n) override { this->_inv.assign(x, n); }

  void assign(VariableRef x, const Number& n) override {
    this->_inv.assign(x, n);
  }

  void assign(VariableRef x, VariableRef y) override {
    this->_inv.assign(x, y);
  }

  void assign(VariableRef x, const LinearExpressionT& e) override {
    this->_inv.assign(x, e);
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
             const Number& z) override {
    this->_inv.apply(op, x, y, z);
  }

  void apply(BinaryOperator op,
             VariableRef x,
             const Number& y,
             VariableRef z) override {
    this->_inv.apply(op, x, y, z);
  }

  void add(const LinearConstraintT& cst) override {
    if (this->is_bottom()) {
      return;
    }

    auto e = IntervalDomain< Number, VariableRef, MaxReductionCycles >::top();
    for (const auto& term : cst) {
      VariableRef x = term.first;
      boost::optional< Number > n = this->_inv.get(x).number();
      if (n) {
        e.assign(x, *n);
      }
    }

    e.add(cst);

    if (e.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    for (auto it = e.begin(), et = e.end(); it != et; ++it) {
      boost::optional< Number > n = it->second.singleton();
      if (n) {
        this->_inv.set(it->first, ConstantT(*n));
      }
    }
  }

  void add(const LinearConstraintSystemT& csts) override {
    if (this->is_bottom()) {
      return;
    }

    auto e = IntervalDomain< Number, VariableRef, MaxReductionCycles >::top();
    for (VariableRef x : csts.variables()) {
      boost::optional< Number > n = this->_inv.get(x).number();
      if (n) {
        e.assign(x, *n);
      }
    }

    e.add(csts);

    if (e.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    for (auto it = e.begin(), et = e.end(); it != et; ++it) {
      boost::optional< Number > n = it->second.singleton();
      if (n) {
        this->_inv.set(it->first, ConstantT(*n));
      }
    }
  }

  void set(VariableRef x, const ConstantT& value) { this->_inv.set(x, value); }

  void set(VariableRef x, const IntervalT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      boost::optional< Number > n = value.singleton();
      if (n) {
        this->_inv.set(x, ConstantT(*n));
      } else {
        this->_inv.forget(x);
      }
    }
  }

  void set(VariableRef x, const CongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      boost::optional< Number > n = value.singleton();
      if (n) {
        this->_inv.set(x, ConstantT(*n));
      } else {
        this->_inv.forget(x);
      }
    }
  }

  void set(VariableRef x, const IntervalCongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      boost::optional< Number > n = value.singleton();
      if (n) {
        this->_inv.set(x, ConstantT(*n));
      } else {
        this->_inv.forget(x);
      }
    }
  }

  void refine(VariableRef x, const ConstantT& value) {
    this->_inv.refine(x, value);
  }

  void refine(VariableRef x, const IntervalT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      IntervalT i(this->to_interval(x).meet(value));
      if (i.is_bottom()) {
        this->set_to_bottom();
      } else if (i.singleton()) {
        this->_inv.set(x, ConstantT(*i.singleton()));
      }
    }
  }

  void refine(VariableRef x, const CongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      CongruenceT c(this->to_congruence(x).meet(value));
      if (c.is_bottom()) {
        this->set_to_bottom();
      } else if (c.singleton()) {
        this->_inv.set(x, ConstantT(*c.singleton()));
      }
    }
  }

  void refine(VariableRef x, const IntervalCongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      IntervalCongruenceT ic(this->to_interval_congruence(x).meet(value));
      if (ic.is_bottom()) {
        this->set_to_bottom();
      } else if (ic.singleton()) {
        this->_inv.set(x, ConstantT(*ic.singleton()));
      }
    }
  }

  void forget(VariableRef x) override { this->_inv.forget(x); }

  ConstantT to_constant(VariableRef x) const { return this->_inv.get(x); }

  ConstantT to_constant(const LinearExpressionT& e) const {
    return this->_inv.project(e);
  }

  IntervalT to_interval(VariableRef x) const override {
    if (this->is_bottom()) {
      return IntervalT::bottom();
    }

    boost::optional< Number > n = this->_inv.get(x).singleton();
    if (n) {
      return IntervalT(*n);
    } else {
      return IntervalT::top();
    }
  }

  IntervalT to_interval(const LinearExpressionT& e) const override {
    return Parent::to_interval(e);
  }

  CongruenceT to_congruence(VariableRef x) const override {
    if (this->is_bottom()) {
      return CongruenceT::bottom();
    }

    boost::optional< Number > n = this->_inv.get(x).singleton();
    if (n) {
      return CongruenceT(*n);
    } else {
      return CongruenceT::top();
    }
  }

  CongruenceT to_congruence(const LinearExpressionT& e) const override {
    return Parent::to_congruence(e);
  }

  IntervalCongruenceT to_interval_congruence(VariableRef x) const override {
    if (this->is_bottom()) {
      return IntervalCongruenceT::bottom();
    }

    boost::optional< Number > n = this->_inv.get(x).singleton();
    if (n) {
      return IntervalCongruenceT(*n);
    } else {
      return IntervalCongruenceT::top();
    }
  }

  IntervalCongruenceT to_interval_congruence(
      const LinearExpressionT& e) const override {
    return Parent::to_interval_congruence(e);
  }

  LinearConstraintSystemT to_linear_constraint_system() const override {
    if (this->is_bottom()) {
      return LinearConstraintSystemT(LinearConstraintT::contradiction());
    }

    LinearConstraintSystemT csts;
    for (auto it = this->begin(), et = this->end(); it != et; ++it) {
      boost::optional< Number > n = it->second.number();
      if (n) {
        csts.add(VariableExprT(it->first) == *n);
      }
    }

    return csts;
  }

  void dump(std::ostream& o) const override { this->_inv.dump(o); }

  static std::string name() { return "constant domain"; }

}; // end class ConstantDomain

} // end namespace numeric
} // end namespace core
} // end namespace ikos
