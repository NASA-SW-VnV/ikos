/*******************************************************************************
 *
 * \file
 * \brief Products of numerical abstract domains
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

#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/domain/domain_product.hpp>
#include <ikos/core/domain/numeric/abstract_domain.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief Domain product of 2 numerical abstract domains
template < typename Number,
           typename VariableRef,
           typename Domain1,
           typename Domain2 >
class DomainProduct2 final
    : public numeric::AbstractDomain<
          Number,
          VariableRef,
          DomainProduct2< Number, VariableRef, Domain1, Domain2 > > {
public:
  static_assert(
      numeric::IsAbstractDomain< Domain1, Number, VariableRef >::value,
      "Domain1 must implement numeric::AbstractDomain");
  static_assert(
      numeric::IsAbstractDomain< Domain2, Number, VariableRef >::value,
      "Domain2 must implement numeric::AbstractDomain");

public:
  using IntervalT = Interval< Number >;
  using CongruenceT = Congruence< Number >;
  using IntervalCongruenceT = IntervalCongruence< Number >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;
  using LinearConstraintSystemT = LinearConstraintSystem< Number, VariableRef >;

private:
  using Product = core::DomainProduct2< Domain1, Domain2 >;

private:
  Product _product;

private:
  /// \brief Private constructor
  explicit DomainProduct2(Product product) : _product(std::move(product)) {}

public:
  /// \brief Create the abstract value with the given abstract values
  DomainProduct2(Domain1 first, Domain2 second)
      : _product(std::move(first), std::move(second)) {}

  /// \brief Copy constructor
  DomainProduct2(const DomainProduct2&) noexcept(
      (std::is_nothrow_copy_constructible< Domain1 >::value) &&
      (std::is_nothrow_copy_constructible< Domain2 >::value)) = default;

  /// \brief Move constructor
  DomainProduct2(DomainProduct2&&) noexcept(
      (std::is_nothrow_move_constructible< Domain1 >::value) &&
      (std::is_nothrow_move_constructible< Domain2 >::value)) = default;

  /// \brief Copy assignment operator
  DomainProduct2& operator=(const DomainProduct2&) noexcept(
      (std::is_nothrow_copy_assignable< Domain1 >::value) &&
      (std::is_nothrow_copy_assignable< Domain2 >::value)) = default;

  /// \brief Move assignment operator
  DomainProduct2& operator=(DomainProduct2&&) noexcept(
      (std::is_nothrow_move_assignable< Domain1 >::value) &&
      (std::is_nothrow_move_assignable< Domain2 >::value)) = default;

  /// \brief Destructor
  ~DomainProduct2() override = default;

  /// \brief Return the first abstract value
  ///
  /// Note: does not normalize.
  const Domain1& first() const { return this->_product.first(); }

  /// \brief Return the first abstract value
  ///
  /// Note: does not normalize.
  Domain1& first() { return this->_product.first(); }

  /// \brief Return the second abstract value
  ///
  /// Note: does not normalize.
  const Domain2& second() const { return this->_product.second(); }

  /// \brief Return the second abstract value
  ///
  /// Note: does not normalize.
  Domain2& second() { return this->_product.second(); }

  void normalize() override { this->_product.normalize(); }

  bool is_bottom() const override { return this->_product.is_bottom(); }

  bool is_top() const override { return this->_product.is_top(); }

  void set_to_bottom() override { this->_product.set_to_bottom(); }

  void set_to_top() override { this->_product.set_to_top(); }

  bool leq(const DomainProduct2& other) const override {
    return this->_product.leq(other._product);
  }

  bool equals(const DomainProduct2& other) const override {
    return this->_product.equals(other._product);
  }

  void join_with(DomainProduct2&& other) override {
    this->_product.join_with(std::move(other._product));
  }

  void join_with(const DomainProduct2& other) override {
    this->_product.join_with(other._product);
  }

  void join_loop_with(DomainProduct2&& other) override {
    this->_product.join_loop_with(std::move(other._product));
  }

  void join_loop_with(const DomainProduct2& other) override {
    this->_product.join_loop_with(other._product);
  }

  void join_iter_with(DomainProduct2&& other) override {
    this->_product.join_iter_with(std::move(other._product));
  }

  void join_iter_with(const DomainProduct2& other) override {
    this->_product.join_iter_with(other._product);
  }

  void widen_with(const DomainProduct2& other) override {
    this->_product.widen_with(other._product);
  }

  void widen_threshold_with(const DomainProduct2& other,
                            const Number& threshold) override {
    this->normalize();
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->first().widen_threshold_with(other.first(), threshold);
      this->second().widen_threshold_with(other.second(), threshold);
    }
  }

  void meet_with(const DomainProduct2& other) override {
    this->_product.meet_with(other._product);
  }

  void narrow_with(const DomainProduct2& other) override {
    this->_product.narrow_with(other._product);
  }

  void narrow_threshold_with(const DomainProduct2& other,
                             const Number& threshold) override {
    this->normalize();
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->first().narrow_threshold_with(other.first(), threshold);
      this->second().narrow_threshold_with(other.second(), threshold);
    }
  }

  DomainProduct2 join(const DomainProduct2& other) const override {
    return DomainProduct2(this->_product.join(other._product));
  }

  DomainProduct2 join_loop(const DomainProduct2& other) const override {
    return DomainProduct2(this->_product.join_loop(other._product));
  }

  DomainProduct2 join_iter(const DomainProduct2& other) const override {
    return DomainProduct2(this->_product.join_iter(other._product));
  }

  DomainProduct2 widening(const DomainProduct2& other) const override {
    return DomainProduct2(this->_product.widening(other._product));
  }

  DomainProduct2 widening_threshold(const DomainProduct2& other,
                                    const Number& threshold) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return DomainProduct2(this->first().widening_threshold(other.first(),
                                                             threshold),
                            this->second().widening_threshold(other.second(),
                                                              threshold));
    }
  }

  DomainProduct2 meet(const DomainProduct2& other) const override {
    return DomainProduct2(this->_product.meet(other._product));
  }

  DomainProduct2 narrowing(const DomainProduct2& other) const override {
    return DomainProduct2(this->_product.narrowing(other._product));
  }

  DomainProduct2 narrowing_threshold(const DomainProduct2& other,
                                     const Number& threshold) const override {
    if (this->is_bottom()) {
      return *this;
    } else if (other.is_bottom()) {
      return other;
    } else {
      return DomainProduct2(this->first().narrowing_threshold(other.first(),
                                                              threshold),
                            this->second().narrowing_threshold(other.second(),
                                                               threshold));
    }
  }

  void assign(VariableRef x, int n) override {
    this->_product.first().assign(x, n);
    this->_product.second().assign(x, n);
  }

  void assign(VariableRef x, const Number& n) override {
    this->_product.first().assign(x, n);
    this->_product.second().assign(x, n);
  }

  void assign(VariableRef x, VariableRef y) override {
    this->_product.first().assign(x, y);
    this->_product.second().assign(x, y);
  }

  void assign(VariableRef x, const LinearExpressionT& e) override {
    this->_product.first().assign(x, e);
    this->_product.second().assign(x, e);
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             VariableRef z) override {
    this->_product.first().apply(op, x, y, z);
    this->_product.second().apply(op, x, y, z);
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             const Number& z) override {
    this->_product.first().apply(op, x, y, z);
    this->_product.second().apply(op, x, y, z);
  }

  void apply(BinaryOperator op,
             VariableRef x,
             const Number& y,
             VariableRef z) override {
    this->_product.first().apply(op, x, y, z);
    this->_product.second().apply(op, x, y, z);
  }

  void add(const LinearConstraintT& cst) override {
    this->_product.first().add(cst);
    this->_product.second().add(cst);
  }

  void add(const LinearConstraintSystemT& csts) override {
    this->_product.first().add(csts);
    this->_product.second().add(csts);
  }

  void set(VariableRef x, const IntervalT& value) override {
    this->_product.first().set(x, value);
    this->_product.second().set(x, value);
  }

  void set(VariableRef x, const CongruenceT& value) override {
    this->_product.first().set(x, value);
    this->_product.second().set(x, value);
  }

  void set(VariableRef x, const IntervalCongruenceT& value) override {
    this->_product.first().set(x, value);
    this->_product.second().set(x, value);
  }

  void refine(VariableRef x, const IntervalT& value) override {
    this->_product.first().refine(x, value);
    this->_product.second().refine(x, value);
  }

  void refine(VariableRef x, const CongruenceT& value) override {
    this->_product.first().refine(x, value);
    this->_product.second().refine(x, value);
  }

  void refine(VariableRef x, const IntervalCongruenceT& value) override {
    this->_product.first().refine(x, value);
    this->_product.second().refine(x, value);
  }

  void forget(VariableRef x) override {
    this->_product.first().forget(x);
    this->_product.second().forget(x);
  }

  IntervalT to_interval(VariableRef x) const override {
    IntervalT a = this->_product.first().to_interval(x);
    IntervalT b = this->_product.second().to_interval(x);
    return a.meet(b);
  }

  IntervalT to_interval(const LinearExpressionT& e) const override {
    IntervalT a = this->_product.first().to_interval(e);
    IntervalT b = this->_product.second().to_interval(e);
    return a.meet(b);
  }

  CongruenceT to_congruence(VariableRef x) const override {
    CongruenceT a = this->_product.first().to_congruence(x);
    CongruenceT b = this->_product.second().to_congruence(x);
    return a.meet(b);
  }

  CongruenceT to_congruence(const LinearExpressionT& e) const override {
    CongruenceT a = this->_product.first().to_congruence(e);
    CongruenceT b = this->_product.second().to_congruence(e);
    return a.meet(b);
  }

  IntervalCongruenceT to_interval_congruence(VariableRef x) const override {
    IntervalCongruenceT a = this->_product.first().to_interval_congruence(x);
    IntervalCongruenceT b = this->_product.second().to_interval_congruence(x);
    return a.meet(b);
  }

  IntervalCongruenceT to_interval_congruence(
      const LinearExpressionT& e) const override {
    IntervalCongruenceT a = this->_product.first().to_interval_congruence(e);
    IntervalCongruenceT b = this->_product.second().to_interval_congruence(e);
    return a.meet(b);
  }

  LinearConstraintSystemT to_linear_constraint_system() const override {
    if (this->is_bottom()) {
      return LinearConstraintSystemT(LinearConstraintT::contradiction());
    }

    LinearConstraintSystemT csts;
    csts = this->_product.first().to_linear_constraint_system();
    csts.add(this->_product.second().to_linear_constraint_system());
    return csts;
  }

  /// \name Non-negative loop counter abstract domain methods
  /// @{

  void counter_mark(VariableRef x) override {
    this->_product.first().counter_mark(x);
    this->_product.second().counter_mark(x);
  }

  void counter_unmark(VariableRef x) override {
    this->_product.first().counter_unmark(x);
    this->_product.second().counter_unmark(x);
  }

  void counter_init(VariableRef x, const Number& c) override {
    this->_product.first().counter_init(x, c);
    this->_product.second().counter_init(x, c);
  }

  void counter_incr(VariableRef x, const Number& k) override {
    this->_product.first().counter_incr(x, k);
    this->_product.second().counter_incr(x, k);
  }

  void counter_forget(VariableRef x) override {
    this->_product.first().counter_forget(x);
    this->_product.second().counter_forget(x);
  }

  /// @}

  void dump(std::ostream& o) const override { this->_product.dump(o); }

  static std::string name() {
    return "numerical product " + Domain1::name() + " x " + Domain2::name();
  }

}; // end class DomainProduct2

/// \brief Domain product of 3 numerical abstract domains
template < typename Number,
           typename VariableRef,
           typename Domain1,
           typename Domain2,
           typename Domain3 >
class DomainProduct3 final
    : public numeric::AbstractDomain<
          Number,
          VariableRef,
          DomainProduct3< Number, VariableRef, Domain1, Domain2, Domain3 > > {
public:
  static_assert(
      numeric::IsAbstractDomain< Domain1, Number, VariableRef >::value,
      "Domain1 must implement numeric::AbstractDomain");
  static_assert(
      numeric::IsAbstractDomain< Domain2, Number, VariableRef >::value,
      "Domain2 must implement numeric::AbstractDomain");
  static_assert(
      numeric::IsAbstractDomain< Domain3, Number, VariableRef >::value,
      "Domain3 must implement numeric::AbstractDomain");

public:
  using IntervalT = Interval< Number >;
  using CongruenceT = Congruence< Number >;
  using IntervalCongruenceT = IntervalCongruence< Number >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;
  using LinearConstraintSystemT = LinearConstraintSystem< Number, VariableRef >;

private:
  using Product12 = DomainProduct2< Number, VariableRef, Domain1, Domain2 >;
  using Product123 = DomainProduct2< Number, VariableRef, Product12, Domain3 >;

private:
  Product123 _product;

private:
  /// \brief Private constructor
  explicit DomainProduct3(Product123 product) : _product(std::move(product)) {}

public:
  /// \brief Create the top abstract value
  DomainProduct3() = default;

  /// \brief Create the abstract value with the given abstract values
  DomainProduct3(Domain1 first, Domain2 second, Domain3 third)
      : _product(Product12(std::move(first), std::move(second)),
                 std::move(third)) {}

  /// \brief Copy constructor
  DomainProduct3(const DomainProduct3&) noexcept(
      (std::is_nothrow_copy_constructible< Domain1 >::value) &&
      (std::is_nothrow_copy_constructible< Domain2 >::value) &&
      (std::is_nothrow_copy_constructible< Domain3 >::value)) = default;

  /// \brief Move constructor
  DomainProduct3(DomainProduct3&&) noexcept(
      (std::is_nothrow_move_constructible< Domain1 >::value) &&
      (std::is_nothrow_move_constructible< Domain2 >::value) &&
      (std::is_nothrow_move_constructible< Domain3 >::value)) = default;

  /// \brief Copy assignment operator
  DomainProduct3& operator=(const DomainProduct3&) noexcept(
      (std::is_nothrow_copy_assignable< Domain1 >::value) &&
      (std::is_nothrow_copy_assignable< Domain2 >::value) &&
      (std::is_nothrow_copy_assignable< Domain3 >::value)) = default;

  /// \brief Move assignment operator
  DomainProduct3& operator=(DomainProduct3&&) noexcept(
      (std::is_nothrow_move_assignable< Domain1 >::value) &&
      (std::is_nothrow_move_assignable< Domain2 >::value) &&
      (std::is_nothrow_move_assignable< Domain3 >::value)) = default;

  /// \brief Destructor
  ~DomainProduct3() override = default;

  /// \brief Create the top abstract value
  static DomainProduct3 top() { return DomainProduct3(Product123::top()); }

  /// \brief Create the bottom abstract value
  static DomainProduct3 bottom() {
    return DomainProduct3(Product123::bottom());
  }

  /// \brief Return the first abstract value
  ///
  /// Note: does not normalize.
  const Domain1& first() const { return this->_product.first().first(); }

  /// \brief Return the first abstract value
  ///
  /// Note: does not normalize.
  Domain1& first() { return this->_product.first().first(); }

  /// \brief Return the second abstract value
  ///
  /// Note: does not normalize.
  const Domain2& second() const { return this->_product.first().second(); }

  /// \brief Return the second abstract value
  ///
  /// Note: does not normalize.
  Domain2& second() { return this->_product.first().second(); }

  /// \brief Return the third abstract value
  ///
  /// Note: does not normalize.
  const Domain3& third() const { return this->_product.second(); }

  /// \brief Return the third abstract value
  ///
  /// Note: does not normalize.
  Domain3& third() { return this->_product.second(); }

  void normalize() override { this->_product.normalize(); }

  bool is_bottom() const override { return this->_product.is_bottom(); }

  bool is_top() const override { return this->_product.is_top(); }

  void set_to_bottom() override { this->_product.set_to_bottom(); }

  void set_to_top() override { this->_product.set_to_top(); }

  bool leq(const DomainProduct3& other) const override {
    return this->_product.leq(other._product);
  }

  bool equals(const DomainProduct3& other) const override {
    return this->_product.equals(other._product);
  }

  void join_with(DomainProduct3&& other) override {
    this->_product.join_with(std::move(other._product));
  }

  void join_with(const DomainProduct3& other) override {
    this->_product.join_with(other._product);
  }

  void join_loop_with(DomainProduct3&& other) override {
    this->_product.join_loop_with(std::move(other._product));
  }

  void join_loop_with(const DomainProduct3& other) override {
    this->_product.join_loop_with(other._product);
  }

  void join_iter_with(DomainProduct3&& other) override {
    this->_product.join_iter_with(std::move(other._product));
  }

  void join_iter_with(const DomainProduct3& other) override {
    this->_product.join_iter_with(other._product);
  }

  void widen_with(const DomainProduct3& other) override {
    this->_product.widen_with(other._product);
  }

  void widen_threshold_with(const DomainProduct3& other,
                            const Number& threshold) override {
    this->_product.widen_threshold_with(other._product, threshold);
  }

  void meet_with(const DomainProduct3& other) override {
    this->_product.meet_with(other._product);
  }

  void narrow_with(const DomainProduct3& other) override {
    this->_product.narrow_with(other._product);
  }

  void narrow_threshold_with(const DomainProduct3& other,
                             const Number& threshold) override {
    this->_product.narrow_threshold_with(other._product, threshold);
  }

  DomainProduct3 join(const DomainProduct3& other) const override {
    return DomainProduct3(this->_product.join(other._product));
  }

  DomainProduct3 join_loop(const DomainProduct3& other) const override {
    return DomainProduct3(this->_product.join_loop(other._product));
  }

  DomainProduct3 join_iter(const DomainProduct3& other) const override {
    return DomainProduct3(this->_product.join_iter(other._product));
  }

  DomainProduct3 widening(const DomainProduct3& other) const override {
    return DomainProduct3(this->_product.widening(other._product));
  }

  DomainProduct3 widening_threshold(const DomainProduct3& other,
                                    const Number& threshold) const override {
    return DomainProduct3(
        this->_product.widening_threshold(other._product, threshold));
  }

  DomainProduct3 meet(const DomainProduct3& other) const override {
    return DomainProduct3(this->_product.meet(other._product));
  }

  DomainProduct3 narrowing(const DomainProduct3& other) const override {
    return DomainProduct3(this->_product.narrowing(other._product));
  }

  DomainProduct3 narrowing_threshold(const DomainProduct3& other,
                                     const Number& threshold) const override {
    return DomainProduct3(
        this->_product.narrowing_threshold(other._product, threshold));
  }

  void assign(VariableRef x, int n) override { this->_product.assign(x, n); }

  void assign(VariableRef x, const Number& n) override {
    this->_product.assign(x, n);
  }

  void assign(VariableRef x, VariableRef y) override {
    this->_product.assign(x, y);
  }

  void assign(VariableRef x, const LinearExpressionT& e) override {
    this->_product.assign(x, e);
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             VariableRef z) override {
    this->_product.apply(op, x, y, z);
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             const Number& z) override {
    this->_product.apply(op, x, y, z);
  }

  void apply(BinaryOperator op,
             VariableRef x,
             const Number& y,
             VariableRef z) override {
    this->_product.apply(op, x, y, z);
  }

  void add(const LinearConstraintT& cst) override { this->_product.add(cst); }

  void add(const LinearConstraintSystemT& csts) override {
    this->_product.add(csts);
  }

  void set(VariableRef x, const IntervalT& value) override {
    this->_product.set(x, value);
  }

  void set(VariableRef x, const CongruenceT& value) override {
    this->_product.set(x, value);
  }

  void set(VariableRef x, const IntervalCongruenceT& value) override {
    this->_product.set(x, value);
  }

  void refine(VariableRef x, const IntervalT& value) override {
    this->_product.refine(x, value);
  }

  void refine(VariableRef x, const CongruenceT& value) override {
    this->_product.refine(x, value);
  }

  void refine(VariableRef x, const IntervalCongruenceT& value) override {
    this->_product.refine(x, value);
  }

  void forget(VariableRef x) override { this->_product.forget(x); }

  IntervalT to_interval(VariableRef x) const override {
    return this->_product.to_interval(x);
  }

  IntervalT to_interval(const LinearExpressionT& e) const override {
    return this->_product.to_interval(e);
  }

  CongruenceT to_congruence(VariableRef x) const override {
    return this->_product.to_congruence(x);
  }

  CongruenceT to_congruence(const LinearExpressionT& e) const override {
    return this->_product.to_congruence(e);
  }

  IntervalCongruenceT to_interval_congruence(VariableRef x) const override {
    return this->_product.to_interval_congruence(x);
  }

  IntervalCongruenceT to_interval_congruence(
      const LinearExpressionT& e) const override {
    return this->_product.to_interval_congruence(e);
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

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else {
      o << "(";
      this->first().dump(o);
      o << ", ";
      this->second().dump(o);
      o << ", ";
      this->third().dump(o);
      o << ")";
    }
  }

  static std::string name() {
    return "numerical product " + Domain1::name() + " x " + Domain2::name() +
           " x " + Domain3::name();
  }

}; // end class DomainProduct3

} // end namespace numeric
} // end namespace core
} // end namespace ikos
