/*******************************************************************************
 *
 * \file
 * \brief Congruence solver for linear constraints equalities
 *
 * Author: Alexandre C. D. Wimmers
 *
 * Contributors:
 *   * Jorge A. Navas
 *   * Maxime Arthaud
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

#include <functional>
#include <vector>

#include <boost/container/flat_set.hpp>

#include <ikos/core/linear_constraint.hpp>
#include <ikos/core/linear_expression.hpp>
#include <ikos/core/value/numeric/congruence.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief Equality congruence solver
///
/// Note that the solver does not own the linear constraints.
///
/// TODO(awimmers): Check correctness of the solver. Granger provides a sound
/// and more precise solver for equality linear congruences (see Theorem 4.4).
template < typename Number, typename VariableRef, typename NumAbstractDomain >
class EqualityCongruenceSolver {
private:
  using CongruenceT = Congruence< Number >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;
  using LinearConstraintSystemT = LinearConstraintSystem< Number, VariableRef >;
  using LinearConstraintRef = std::reference_wrapper< const LinearConstraintT >;
  using LinearConstraintSystemRef =
      std::reference_wrapper< const LinearConstraintSystemT >;

private:
  using ConstraintSet = std::vector< LinearConstraintRef >;
  using VariableSet = boost::container::flat_set< VariableRef >;

private:
  std::size_t _max_cycles;
  std::size_t _op_count = 0;
  bool _is_contradiction = false;
  ConstraintSet _csts;
  VariableSet _refined_variables;

private:
  struct BottomFound {};

  /// \brief Refine the abstract value for the given variable v
  void refine(VariableRef v, const CongruenceT& c, NumAbstractDomain& inv) {
    CongruenceT old_c = inv.to_congruence(v);
    CongruenceT new_c = old_c.meet(c);
    if (new_c.is_bottom()) {
      throw BottomFound();
    }
    if (old_c != new_c) {
      inv.refine(v, new_c);
      this->_refined_variables.insert(v);
      ++this->_op_count;
    }
  }

  /// \brief Compute the residual of a linear constraint
  CongruenceT compute_residual(const LinearConstraintT& cst,
                               VariableRef pivot,
                               const NumAbstractDomain& inv) {
    CongruenceT residual(cst.constant());
    for (const auto& term : cst) {
      if (term.first != pivot) {
        residual -= CongruenceT(term.second) * inv.to_congruence(term.first);
        ++this->_op_count;
      }
    }
    return residual;
  }

  void propagate(const LinearConstraintT& cst, NumAbstractDomain& inv) {
    // Inequations (>=, <=, >, and <) do not work well with
    // congruences because for any number n there is always x and y
    // \in gamma(aZ+b) such that n < x and n > y.
    //
    // The only cases we can catch is when all the expressions
    // are constants. We do not bother because any product
    // with intervals or constants should get those cases.
    if (!cst.is_equality()) {
      return;
    }

    for (const auto& term : cst) {
      const Number& c = term.second;
      VariableRef pivot = term.first;

      CongruenceT rhs =
          this->compute_residual(cst, pivot, inv) / CongruenceT(c);
      this->refine(pivot, rhs, inv);
    }
  }

  /// \brief Solve a linear constraint system
  void solve_system(NumAbstractDomain& inv) {
    std::size_t cycle = 0;
    do {
      ++cycle;
      this->_refined_variables.clear();
      for (const LinearConstraintT& cst : this->_csts) {
        this->propagate(cst, inv);
      }
    } while (!this->_refined_variables.empty() && cycle <= this->_max_cycles);
  }

public:
  /// \brief Constructor
  explicit EqualityCongruenceSolver(std::size_t max_cycles)
      : _max_cycles(max_cycles) {}

  /// \brief Add a constraint
  ///
  /// Warning: the linear constraint should outlive the solver
  void add(LinearConstraintRef cst) {
    if (cst.get().is_contradiction()) {
      this->_is_contradiction = true;
    } else if (cst.get().is_tautology()) {
      return;
    } else {
      this->_csts.push_back(cst);
    }
  }

  /// \brief Add a system of constraint
  ///
  /// Warning: the linear constrainst system should outlive the solver
  void add(LinearConstraintSystemRef csts) {
    for (const LinearConstraintT& cst : csts.get()) {
      if (cst.is_contradiction()) {
        this->_is_contradiction = true;
        return;
      } else if (cst.is_tautology()) {
        continue;
      } else {
        this->_csts.push_back(cst);
      }
    }
  }

  /// \brief Return the number of linear constraints
  std::size_t size() const { return this->_csts.size(); }

  /// \brief Return true if the solver is empty
  bool empty() const { return this->_csts.empty(); }

  /// \brief Solve the system and refine the given invariant
  void run(NumAbstractDomain& inv) {
    if (this->_is_contradiction) {
      inv.set_to_bottom();
      return;
    }

    if (this->empty()) {
      return;
    }

    try {
      this->solve_system(inv);
    } catch (BottomFound&) {
      inv.set_to_bottom();
    }
  }

}; // end class EqualityCongruenceSolver

} // end namespace numeric
} // end namespace core
} // end namespace ikos
