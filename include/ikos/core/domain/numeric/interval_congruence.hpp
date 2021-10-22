/*******************************************************************************
 *
 * \file
 * \brief Reduced product of intervals and congruences
 *
 * The reduce operator based on "Static Analysis of Arithmetical
 * Congruences" by P. Granger published in International Journal of
 * Computer Mathematics, 1989.
 *
 * Author: Jorge A. Navas Laserna
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
#include <ikos/core/domain/numeric/equality_congruence_solver.hpp>
#include <ikos/core/domain/numeric/linear_interval_solver.hpp>
#include <ikos/core/domain/numeric/separate_domain.hpp>
#include <ikos/core/value/numeric/interval_congruence.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief Reduced product of intervals and congruences
template < typename Number,
           typename VariableRef,
           std::size_t MaxReductionCycles = 10 >
class IntervalCongruenceDomain final
    : public numeric::AbstractDomain<
          Number,
          VariableRef,
          IntervalCongruenceDomain< Number,
                                    VariableRef,
                                    MaxReductionCycles > > {
public:
  using IntervalT = Interval< Number >;
  using CongruenceT = Congruence< Number >;
  using IntervalCongruenceT = IntervalCongruence< Number >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;
  using LinearConstraintSystemT = LinearConstraintSystem< Number, VariableRef >;

private:
  using SeparateDomainT =
      SeparateDomain< Number, VariableRef, IntervalCongruenceT >;
  using LinearIntervalSolverT =
      LinearIntervalSolver< Number, VariableRef, IntervalCongruenceDomain >;
  using EqualityCongruenceSolverT =
      EqualityCongruenceSolver< Number, VariableRef, IntervalCongruenceDomain >;

public:
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

  /// \brief Begin iterator over the pairs (variable, interval_congruence)
  Iterator begin() const { return this->_inv.begin(); }

  /// \brief End iterator over the pairs (variable, interval_congruence)
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

  void join_loop_with(const IntervalCongruenceDomain& other) override {
    this->_inv.join_loop_with(other._inv);
  }

  void join_iter_with(const IntervalCongruenceDomain& other) override {
    this->_inv.join_iter_with(other._inv);
  }

  void widen_with(const IntervalCongruenceDomain& other) override {
    this->_inv.widen_with(other._inv);
  }

  void widen_threshold_with(const IntervalCongruenceDomain& other,
                            const Number& threshold) override {
    this->_inv.widen_threshold_with(other._inv, threshold);
  }

  void meet_with(const IntervalCongruenceDomain& other) override {
    this->_inv.meet_with(other._inv);
  }

  void narrow_with(const IntervalCongruenceDomain& other) override {
    this->_inv.narrow_with(other._inv);
  }

  void narrow_threshold_with(const IntervalCongruenceDomain& other,
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

    LinearIntervalSolverT int_solver(MaxReductionCycles);
    int_solver.add(cst);
    int_solver.run(*this);

    EqualityCongruenceSolverT cong_solver(MaxReductionCycles);
    cong_solver.add(cst);
    cong_solver.run(*this);
  }

  void add(const LinearConstraintSystemT& csts) override {
    if (this->is_bottom()) {
      return;
    }

    LinearIntervalSolverT int_solver(MaxReductionCycles);
    int_solver.add(csts);
    int_solver.run(*this);

    EqualityCongruenceSolverT cong_solver(MaxReductionCycles);
    cong_solver.add(csts);
    cong_solver.run(*this);
  }

  void set(VariableRef x, const IntervalCongruenceT& value) override {
    this->_inv.set(x, value);
  }

  void set(VariableRef x, const IntervalT& value) override {
    this->set(x, IntervalCongruenceT(value));
  }

  void set(VariableRef x, const CongruenceT& value) override {
    this->set(x, IntervalCongruenceT(value));
  }

  void refine(VariableRef x, const IntervalCongruenceT& value) override {
    this->_inv.refine(x, value);
  }

  void refine(VariableRef x, const IntervalT& value) override {
    this->refine(x, IntervalCongruenceT(value));
  }

  void refine(VariableRef x, const CongruenceT& value) override {
    this->refine(x, IntervalCongruenceT(value));
  }

  void forget(VariableRef x) override { this->_inv.forget(x); }

  IntervalCongruenceT to_interval_congruence(VariableRef x) const override {
    return this->_inv.get(x);
  }

  IntervalCongruenceT to_interval_congruence(
      const LinearExpressionT& e) const override {
    return this->_inv.project(e);
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

  LinearConstraintSystemT to_linear_constraint_system() const override {
    if (this->is_bottom()) {
      return LinearConstraintSystemT(LinearConstraintT::contradiction());
    }

    LinearConstraintSystemT csts;
    for (auto it = this->begin(), et = this->end(); it != et; ++it) {
      csts.add(within_interval(it->first, it->second.interval()));
    }

    return csts;
  }

  void dump(std::ostream& o) const override { this->_inv.dump(o); }

  static std::string name() {
    return "reduced product of intervals and congruences";
  }

}; // end class IntervalCongruenceDomain

} // end namespace numeric
} // end namespace core
} // end namespace ikos
