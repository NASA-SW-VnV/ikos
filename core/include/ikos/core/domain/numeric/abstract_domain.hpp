/*******************************************************************************
 *
 * \file
 * \brief Generic API for numerical abstract domains
 *
 * Author: Maxime Arthaud
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
#include <ikos/core/domain/numeric/operator.hpp>
#include <ikos/core/linear_constraint.hpp>
#include <ikos/core/linear_expression.hpp>
#include <ikos/core/semantic/variable.hpp>
#include <ikos/core/value/numeric/congruence.hpp>
#include <ikos/core/value/numeric/interval.hpp>
#include <ikos/core/value/numeric/interval_congruence.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief Base class for numerical abstract domains
template < typename Number, typename VariableRef, typename Derived >
class AbstractDomain : public core::AbstractDomain< Derived > {
public:
  static_assert(
      IsVariable< VariableRef >::value,
      "VariableRef does not meet the requirements for variable types");

public:
  using IntervalT = Interval< Number >;
  using CongruenceT = Congruence< Number >;
  using IntervalCongruenceT = IntervalCongruence< Number >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;
  using LinearConstraintSystemT = LinearConstraintSystem< Number, VariableRef >;

public:
  /// \brief Perform the widening of two abstract values with a threshold
  virtual void widen_threshold_with(const Derived& other,
                                    const Number& threshold) = 0;

  /// \brief Perform the widening of two abstract values with a threshold
  virtual Derived widening_threshold(const Derived& other,
                                     const Number& threshold) const {
    Derived tmp(static_cast< const Derived& >(*this));
    tmp.widen_threshold_with(other, threshold);
    return tmp;
  }

  /// \brief Perform the narrowing of two abstract values with a threshold
  virtual void narrow_threshold_with(const Derived& other,
                                     const Number& threshold) = 0;

  /// \brief Perform the narrowing of two abstract values with a threshold
  virtual Derived narrowing_threshold(const Derived& other,
                                      const Number& threshold) const {
    Derived tmp(static_cast< const Derived& >(*this));
    tmp.narrow_threshold_with(other, threshold);
    return tmp;
  }

  /// \brief Assign `x = n`
  virtual void assign(VariableRef x, int n) = 0;

  /// \brief Assign `x = n`
  virtual void assign(VariableRef x, const Number& n) = 0;

  /// \brief Assign `x = y`
  virtual void assign(VariableRef x, VariableRef y) = 0;

  /// \brief Assign `x = e`
  virtual void assign(VariableRef x, const LinearExpressionT& e) = 0;

  /// \brief Apply `x = y op z`
  virtual void apply(BinaryOperator op,
                     VariableRef x,
                     VariableRef y,
                     VariableRef z) = 0;

  /// \brief Apply `x = y op z`
  virtual void apply(BinaryOperator op,
                     VariableRef x,
                     VariableRef y,
                     const Number& z) = 0;

  /// \brief Apply `x = y op z`
  virtual void apply(BinaryOperator op,
                     VariableRef x,
                     const Number& y,
                     VariableRef z) = 0;

  /// \brief Add a linear constraint
  virtual void add(const LinearConstraintT& cst) = 0;

  /// \brief Add a linear constraint system
  virtual void add(const LinearConstraintSystemT& csts) = 0;

  /// \brief Set the interval value of a variable
  virtual void set(VariableRef x, const IntervalT& value) = 0;

  /// \brief Set the congruence value of a variable
  virtual void set(VariableRef x, const CongruenceT& value) = 0;

  /// \brief Set the interval-congruence value of a variable
  virtual void set(VariableRef x, const IntervalCongruenceT& value) = 0;

  /// \brief Refine the value of a variable with an interval
  virtual void refine(VariableRef x, const IntervalT& value) = 0;

  /// \brief Refine the value of a variable with a congruence
  virtual void refine(VariableRef x, const CongruenceT& value) = 0;

  /// \brief Refine the value of a variable with an interval-congruence
  virtual void refine(VariableRef x, const IntervalCongruenceT& value) = 0;

  /// \brief Forget a numerical variable
  virtual void forget(VariableRef x) = 0;

  /// \brief Projection to an interval
  ///
  /// Return an overapproximation of the value of `x` as an interval
  virtual IntervalT to_interval(VariableRef x) const = 0;

  /// \brief Projection to an interval
  ///
  /// Return an overapproximation of the linear expression `e` as an interval
  virtual IntervalT to_interval(const LinearExpressionT& e) const {
    if (this->is_bottom()) {
      return IntervalT::bottom();
    }

    IntervalT r(e.constant());
    for (const auto& term : e) {
      r += IntervalT(term.second) * this->to_interval(term.first);
    }
    return r;
  }

  /// \brief Projection to a congruence
  ///
  /// Return an overapproximation of the value of `x` as a congruence
  virtual CongruenceT to_congruence(VariableRef x) const = 0;

  /// \brief Projection to a congruence
  ///
  /// Return an overapproximation of the linear expression `e` as a congruence
  virtual CongruenceT to_congruence(const LinearExpressionT& e) const {
    if (this->is_bottom()) {
      return CongruenceT::bottom();
    }

    CongruenceT r(e.constant());
    for (const auto& term : e) {
      r += CongruenceT(term.second) * this->to_congruence(term.first);
    }
    return r;
  }

  /// \brief Projection to an interval-congruence
  ///
  /// Return an overapproximation of the value of `x` as an interval-congruence
  virtual IntervalCongruenceT to_interval_congruence(VariableRef x) const = 0;

  /// \brief Projection to an interval-congruence
  ///
  /// Return an overapproximation of the linear expression `e` as an
  /// interval-congruence
  virtual IntervalCongruenceT to_interval_congruence(
      const LinearExpressionT& e) const {
    if (this->is_bottom()) {
      return IntervalCongruenceT::bottom();
    }

    IntervalCongruenceT r(e.constant());
    for (const auto& term : e) {
      r += IntervalCongruenceT(term.second) *
           this->to_interval_congruence(term.first);
    }
    return r;
  }

  /// \brief Return a set of linear constraints representing the abstract value
  virtual LinearConstraintSystemT to_linear_constraint_system() const = 0;

  /// \name Non-negative loop counter abstract domain methods
  /// @{

  /// \brief Mark the variable `x` as a non-negative loop counter
  virtual void counter_mark(VariableRef /*x*/) {}

  /// \brief Mark the variable `x` as a normal variable, without losing
  /// information
  virtual void counter_unmark(VariableRef /*x*/) {}

  /// \brief Initialize a non-negative loop counter: `x = c`
  ///
  /// Precondition: `c >= 0`
  virtual void counter_init(VariableRef x, const Number& c) {
    this->assign(x, c);
  }

  /// \brief Increment a non-negative loop counter counter: `x += k`
  ///
  /// Precondition: `k >= 0`
  virtual void counter_incr(VariableRef x, const Number& k) {
    this->apply(BinaryOperator::Add, x, x, k);
  }

  /// \brief Forget a non-negative loop counter
  virtual void counter_forget(VariableRef x) { this->forget(x); }

  /// @}

}; // end class AbstractDomain

/// \brief Check if a type is a numeric abstract domain
template < typename T, typename Number, typename VariableRef >
struct IsAbstractDomain
    : std::is_base_of< numeric::AbstractDomain< Number, VariableRef, T >, T > {
};

} // end namespace numeric
} // end namespace core
} // end namespace ikos
