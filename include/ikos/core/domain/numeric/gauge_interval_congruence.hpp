/*******************************************************************************
 *
 * \file
 * \brief Reduced product of gauges, intervals and congruences
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017-2019 United States Government as represented by the
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
#include <ikos/core/domain/numeric/congruence.hpp>
#include <ikos/core/domain/numeric/domain_product.hpp>
#include <ikos/core/domain/numeric/gauge.hpp>
#include <ikos/core/domain/numeric/interval.hpp>
#include <ikos/core/value/numeric/interval_congruence.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief Reduced product of gauges, intervals and congruences
template < typename Number,
           typename VariableRef,
           std::size_t MaxReductionCycles = 10 >
class GaugeIntervalCongruenceDomain final
    : public numeric::AbstractDomain<
          Number,
          VariableRef,
          GaugeIntervalCongruenceDomain< Number,
                                         VariableRef,
                                         MaxReductionCycles > > {
public:
  using GaugeT = Gauge< Number, VariableRef >;
  using IntervalT = Interval< Number >;
  using CongruenceT = Congruence< Number >;
  using IntervalCongruenceT = IntervalCongruence< Number >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;
  using LinearConstraintSystemT = LinearConstraintSystem< Number, VariableRef >;

private:
  using GaugeDomainT = GaugeDomain< Number, VariableRef >;
  using IntervalDomainT =
      IntervalDomain< Number, VariableRef, MaxReductionCycles >;
  using CongruenceDomainT =
      CongruenceDomain< Number, VariableRef, MaxReductionCycles >;
  using DomainProduct = numeric::DomainProduct3< Number,
                                                 VariableRef,
                                                 GaugeDomainT,
                                                 IntervalDomainT,
                                                 CongruenceDomainT >;

private:
  DomainProduct _product;

private:
  /// \brief Private constructor
  explicit GaugeIntervalCongruenceDomain(DomainProduct product)
      : _product(std::move(product)) {}

  /// \brief Reduce the information on variable `v`
  void reduce_variable(VariableRef v) {
    if (this->is_bottom()) {
      return;
    }

    IntervalT i = this->_product.first().to_interval(v);
    IntervalT j = this->_product.second().to_interval(v);
    CongruenceT c = this->_product.third().to_congruence(v);
    IntervalCongruenceT val(i.meet(j), c);

    if (val.is_bottom()) {
      this->set_to_bottom();
    } else {
      if (val.interval() != j) {
        this->_product.second().set(v, val.interval());
      }
      if (val.congruence() != c) {
        this->_product.third().set(v, val.congruence());
      }
    }
  }

public:
  /// \brief Create the top abstract value
  static GaugeIntervalCongruenceDomain top() {
    return GaugeIntervalCongruenceDomain(
        DomainProduct(GaugeDomainT::top(),
                      IntervalDomainT::top(),
                      CongruenceDomainT::top()));
  }

  /// \brief Create the bottom abstract value
  static GaugeIntervalCongruenceDomain bottom() {
    return GaugeIntervalCongruenceDomain(
        DomainProduct(GaugeDomainT::bottom(),
                      IntervalDomainT::bottom(),
                      CongruenceDomainT::bottom()));
  }

  /// \brief Copy constructor
  GaugeIntervalCongruenceDomain(const GaugeIntervalCongruenceDomain&) noexcept =
      default;

  /// \brief Move constructor
  GaugeIntervalCongruenceDomain(GaugeIntervalCongruenceDomain&&) noexcept =
      default;

  /// \brief Copy assignment operator
  GaugeIntervalCongruenceDomain& operator=(
      const GaugeIntervalCongruenceDomain&) noexcept = default;

  /// \brief Move assignment operator
  GaugeIntervalCongruenceDomain& operator=(
      GaugeIntervalCongruenceDomain&&) noexcept = default;

  /// \brief Destructor
  ~GaugeIntervalCongruenceDomain() override = default;

  /// \brief Return the first abstract value
  ///
  /// Note: does not normalize.
  const GaugeDomainT& first() const { return this->_product.first(); }

  /// \brief Return the first abstract value
  ///
  /// Note: does not normalize.
  GaugeDomainT& first() { return this->_product.first(); }

  /// \brief Return the second abstract value
  ///
  /// Note: does not normalize.
  const IntervalDomainT& second() const { return this->_product.second(); }

  /// \brief Return the second abstract value
  ///
  /// Note: does not normalize.
  IntervalDomainT& second() { return this->_product.second(); }

  /// \brief Return the third abstract value
  ///
  /// Note: does not normalize.
  const CongruenceDomainT& third() const { return this->_product.third(); }

  /// \brief Return the third abstract value
  ///
  /// Note: does not normalize.
  CongruenceDomainT& third() { return this->_product.third(); }

  void normalize() override { this->_product.normalize(); }

  bool is_bottom() const override { return this->_product.is_bottom(); }

  bool is_top() const override { return this->_product.is_top(); }

  void set_to_bottom() override { this->_product.set_to_bottom(); }

  void set_to_top() override { this->_product.set_to_top(); }

  bool leq(const GaugeIntervalCongruenceDomain& other) const override {
    return this->_product.leq(other._product);
  }

  bool equals(const GaugeIntervalCongruenceDomain& other) const override {
    return this->_product.equals(other._product);
  }

  void join_with(GaugeIntervalCongruenceDomain&& other) override {
    this->_product.join_with(std::move(other._product));
  }

  void join_with(const GaugeIntervalCongruenceDomain& other) override {
    this->_product.join_with(other._product);
  }

  void join_loop_with(GaugeIntervalCongruenceDomain&& other) override {
    this->_product.join_loop_with(std::move(other._product));
  }

  void join_loop_with(const GaugeIntervalCongruenceDomain& other) override {
    this->_product.join_loop_with(other._product);
  }

  void join_iter_with(GaugeIntervalCongruenceDomain&& other) override {
    this->_product.join_iter_with(std::move(other._product));
  }

  void join_iter_with(const GaugeIntervalCongruenceDomain& other) override {
    this->_product.join_iter_with(other._product);
  }

  void widen_with(const GaugeIntervalCongruenceDomain& other) override {
    this->_product.widen_with(other._product);
  }

  void widen_threshold_with(const GaugeIntervalCongruenceDomain& other,
                            const Number& threshold) override {
    this->_product.widen_threshold_with(other._product, threshold);
  }

  void meet_with(const GaugeIntervalCongruenceDomain& other) override {
    this->_product.meet_with(other._product);
  }

  void narrow_with(const GaugeIntervalCongruenceDomain& other) override {
    this->_product.narrow_with(other._product);
  }

  void narrow_threshold_with(const GaugeIntervalCongruenceDomain& other,
                             const Number& threshold) override {
    this->_product.narrow_threshold_with(other._product, threshold);
  }

  GaugeIntervalCongruenceDomain join(
      const GaugeIntervalCongruenceDomain& other) const override {
    return GaugeIntervalCongruenceDomain(this->_product.join(other._product));
  }

  GaugeIntervalCongruenceDomain join_loop(
      const GaugeIntervalCongruenceDomain& other) const override {
    return GaugeIntervalCongruenceDomain(
        this->_product.join_loop(other._product));
  }

  GaugeIntervalCongruenceDomain join_iter(
      const GaugeIntervalCongruenceDomain& other) const override {
    return GaugeIntervalCongruenceDomain(
        this->_product.join_iter(other._product));
  }

  GaugeIntervalCongruenceDomain widening(
      const GaugeIntervalCongruenceDomain& other) const override {
    return GaugeIntervalCongruenceDomain(
        this->_product.widening(other._product));
  }

  GaugeIntervalCongruenceDomain widening_threshold(
      const GaugeIntervalCongruenceDomain& other,
      const Number& threshold) const override {
    return GaugeIntervalCongruenceDomain(
        this->_product.widening_threshold(other._product, threshold));
  }

  GaugeIntervalCongruenceDomain meet(
      const GaugeIntervalCongruenceDomain& other) const override {
    return GaugeIntervalCongruenceDomain(this->_product.meet(other._product));
  }

  GaugeIntervalCongruenceDomain narrowing(
      const GaugeIntervalCongruenceDomain& other) const override {
    return GaugeIntervalCongruenceDomain(
        this->_product.narrowing(other._product));
  }

  GaugeIntervalCongruenceDomain narrowing_threshold(
      const GaugeIntervalCongruenceDomain& other,
      const Number& threshold) const override {
    return GaugeIntervalCongruenceDomain(
        this->_product.narrowing_threshold(other._product, threshold));
  }

  void assign(VariableRef x, int n) override { this->_product.assign(x, n); }

  void assign(VariableRef x, const Number& n) override {
    this->_product.assign(x, n);
  }

  void assign(VariableRef x, VariableRef y) override {
    this->_product.assign(x, y);
    this->reduce_variable(x);
  }

  void assign(VariableRef x, const LinearExpressionT& e) override {
    this->_product.assign(x, e);
    this->reduce_variable(x);
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             VariableRef z) override {
    this->_product.apply(op, x, y, z);
    this->reduce_variable(x);
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             const Number& z) override {
    this->_product.apply(op, x, y, z);
    this->reduce_variable(x);
  }

  void apply(BinaryOperator op,
             VariableRef x,
             const Number& y,
             VariableRef z) override {
    this->_product.apply(op, x, y, z);
    this->reduce_variable(x);
  }

  void add(const LinearConstraintT& cst) override {
    this->_product.add(cst);

    for (const auto& term : cst) {
      this->reduce_variable(term.first);
    }
  }

  void add(const LinearConstraintSystemT& csts) override {
    this->_product.add(csts);

    for (const LinearConstraintT& cst : csts) {
      for (const auto& term : cst) {
        this->reduce_variable(term.first);
      }
    }
  }

  void set(VariableRef x, const IntervalT& value) override {
    this->set(x, IntervalCongruenceT(value));
  }

  void set(VariableRef x, const CongruenceT& value) override {
    this->set(x, IntervalCongruenceT(value));
  }

  void set(VariableRef x, const IntervalCongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_product.first().set(x, value.interval());
      this->_product.second().set(x, value.interval());
      this->_product.third().set(x, value.congruence());
    }
  }

  void refine(VariableRef x, const IntervalT& value) override {
    this->refine(x, IntervalCongruenceT(value));
  }

  void refine(VariableRef x, const CongruenceT& value) override {
    this->refine(x, IntervalCongruenceT(value));
  }

  void refine(VariableRef x, const IntervalCongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_product.second().refine(x, value.interval());
      this->_product.third().refine(x, value.congruence());
      this->reduce_variable(x);
    }
  }

  void forget(VariableRef x) override { this->_product.forget(x); }

  GaugeT to_gauge(VariableRef x) const {
    return this->_product.first().to_gauge(x);
  }

  GaugeT to_gauge(const LinearExpressionT& e) const {
    return this->_product.first().to_gauge(e);
  }

  IntervalT to_interval(VariableRef x) const override {
    return this->to_interval_congruence(x).interval();
  }

  IntervalT to_interval(const LinearExpressionT& e) const override {
    return this->to_interval_congruence(e).interval();
  }

  CongruenceT to_congruence(VariableRef x) const override {
    return this->to_interval_congruence(x).congruence();
  }

  CongruenceT to_congruence(const LinearExpressionT& e) const override {
    return this->to_interval_congruence(e).congruence();
  }

  IntervalCongruenceT to_interval_congruence(VariableRef x) const override {
    if (this->is_bottom()) {
      return IntervalCongruenceT::bottom();
    } else {
      IntervalT i = this->_product.first().to_interval(x);
      i.meet_with(this->_product.second().to_interval(x));
      CongruenceT c = this->_product.third().to_congruence(x);
      return IntervalCongruenceT(i, c);
    }
  }

  IntervalCongruenceT to_interval_congruence(
      const LinearExpressionT& e) const override {
    if (this->is_bottom()) {
      return IntervalCongruenceT::bottom();
    } else {
      IntervalT i = this->_product.first().to_interval(e);
      i.meet_with(this->_product.second().to_interval(e));
      CongruenceT c = this->_product.third().to_congruence(e);
      return IntervalCongruenceT(i, c);
    }
  }

  LinearConstraintSystemT to_linear_constraint_system() const override {
    return this->_product.to_linear_constraint_system();
  }

  /// \name Non-negative loop counter abstract domain methods
  /// @{

  void counter_mark(VariableRef x) override { this->_product.counter_mark(x); }

  void counter_unmark(VariableRef x) override {
    this->_product.counter_unmark(x);
  }

  void counter_init(VariableRef x, const Number& c) override {
    this->_product.counter_init(x, c);
  }

  void counter_incr(VariableRef x, const Number& k) override {
    this->_product.counter_incr(x, k);
  }

  void counter_forget(VariableRef x) override {
    this->_product.counter_forget(x);
  }

  /// @}

  void dump(std::ostream& o) const override { this->_product.dump(o); }

  static std::string name() { return "gauge + interval + congruence domain"; }

}; // end class GaugeIntervalCongruenceDomain

} // end namespace numeric
} // end namespace core
} // end namespace ikos
