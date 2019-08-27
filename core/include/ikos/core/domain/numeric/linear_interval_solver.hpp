/*******************************************************************************
 *
 * \file
 * \brief Interval solver for linear constraints
 *
 * The resolution of a system of linear constraints over the domain of intervals
 * is based on W. Harvey & P. J. Stuckey's paper: Improving linear constraint
 * propagation by changing constraint representation, in Constraints,
 * 8(2):173–207, 2003.
 *
 * Author: Arnaud J. Venet
 *
 * Contributors:
 *   * Alexandre C. D. Wimmers
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

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>

#include <ikos/core/linear_constraint.hpp>
#include <ikos/core/linear_expression.hpp>
#include <ikos/core/value/numeric/interval.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief Trim the bounds of the given interval
inline ZInterval trim_bound(const ZInterval& i, const ZBound& b) {
  ikos_assert(!i.is_bottom());
  if (i.lb() == b) {
    return ZInterval(b + ZBound(1), i.ub());
  } else if (i.ub() == b) {
    return ZInterval(i.lb(), b - ZBound(1));
  } else {
    return i;
  }
}

/// \brief Trim the bounds of the given interval
inline QInterval trim_bound(const QInterval& i, const QBound&) {
  // No refinement possible for disequations over rational numbers
  return i;
}

/// \brief Linear interval solver
///
/// Note that the solver does not own the linear constraints.
template < typename Number, typename VariableRef, typename NumAbstractDomain >
class LinearIntervalSolver {
private:
  static const std::size_t LargeSystemCstThreshold = 3;

  /// \brief Cost of one propagation cycle for a dense 3x3 system of constraints
  static const std::size_t LargeSystemOpThreshold = 27;

private:
  using BoundT = Bound< Number >;
  using IntervalT = Interval< Number >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;
  using LinearConstraintSystemT = LinearConstraintSystem< Number, VariableRef >;
  using LinearConstraintRef = std::reference_wrapper< const LinearConstraintT >;
  using LinearConstraintSystemRef =
      std::reference_wrapper< const LinearConstraintSystemT >;

private:
  using ConstraintSet = std::vector< LinearConstraintRef >;
  using TriggerTable = boost::container::flat_map< VariableRef, ConstraintSet >;
  using VariableSet = boost::container::flat_set< VariableRef >;

private:
  std::size_t _max_cycles;
  std::size_t _op_per_cycle = 0;
  std::size_t _max_op = 0;
  std::size_t _op_count = 0;
  bool _is_contradiction = false;
  bool _is_large_system = false;
  ConstraintSet _csts;
  TriggerTable _trigger_table;
  VariableSet _refined_variables;

private:
  struct BottomFound {};

  /// \brief Refine the abstract value for the given variable v
  void refine(VariableRef v, const IntervalT& i, NumAbstractDomain& inv) {
    IntervalT old_i = inv.to_interval(v);
    IntervalT new_i = old_i.meet(i);
    if (new_i.is_bottom()) {
      throw BottomFound();
    }
    if (old_i != new_i) {
      inv.refine(v, new_i);
      this->_refined_variables.insert(v);
      ++this->_op_count;
    }
  }

  /// \brief Compute the residual of a linear constraint
  IntervalT compute_residual(const LinearConstraintT& cst,
                             VariableRef pivot,
                             const NumAbstractDomain& inv) {
    IntervalT residual(cst.constant());
    for (const auto& term : cst) {
      if (term.first != pivot) {
        residual -= IntervalT(term.second) * inv.to_interval(term.first);
        ++this->_op_count;
      }
    }
    return residual;
  }

  void propagate(const LinearConstraintT& cst, NumAbstractDomain& inv) {
    for (const auto& term : cst) {
      const Number& c = term.second;
      VariableRef pivot = term.first;

      IntervalT rhs = this->compute_residual(cst, pivot, inv) / IntervalT(c);
      if (cst.is_equality()) {
        this->refine(pivot, rhs, inv);
      } else if (cst.is_inequality()) {
        if (c > 0) {
          this->refine(pivot, rhs.lower_half_line(), inv);
        } else {
          this->refine(pivot, rhs.upper_half_line(), inv);
        }
      } else {
        // cst is a disequation
        boost::optional< Number > k = rhs.singleton();
        if (k) {
          IntervalT old_i = inv.to_interval(pivot);
          IntervalT new_i = trim_bound(old_i, BoundT(*k));
          if (new_i.is_bottom()) {
            throw BottomFound();
          }
          if (old_i != new_i) {
            inv.refine(pivot, new_i);
            this->_refined_variables.insert(pivot);
          }
          ++this->_op_count;
        }
      }
    }
  }

  void build_trigger_table() {
    // Build the trigger table
    for (LinearConstraintRef cst : this->_csts) {
      for (const auto& term : cst.get()) {
        this->_trigger_table[term.first].push_back(cst);
      }
    }
  }

  /// \brief Slove a large linear constraint system
  void solve_large_system(NumAbstractDomain& inv) {
    this->_op_count = 0;
    this->_refined_variables.clear();
    for (const LinearConstraintT& cst : this->_csts) {
      this->propagate(cst, inv);
    }
    do {
      VariableSet vars_to_process(this->_refined_variables);
      this->_refined_variables.clear();
      for (VariableRef var : vars_to_process) {
        for (const LinearConstraintT& cst : this->_trigger_table[var]) {
          this->propagate(cst, inv);
        }
      }
    } while (!this->_refined_variables.empty() &&
             this->_op_count <= this->_max_op);
  }

  /// \brief Slove a small linear constraint system
  void solve_small_system(NumAbstractDomain& inv) {
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
  explicit LinearIntervalSolver(std::size_t max_cycles)
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

      // cost of one reduction step on the constraint in terms
      // of accesses to the interval collection
      this->_op_per_cycle += cst.get().num_terms() * cst.get().num_terms();
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

        // cost of one reduction step on the constraint in terms
        // of accesses to the interval collection
        this->_op_per_cycle += cst.num_terms() * cst.num_terms();
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

    this->_max_op = this->_op_per_cycle * this->_max_cycles;

    this->_is_large_system = this->_csts.size() > LargeSystemCstThreshold ||
                             this->_op_per_cycle > LargeSystemOpThreshold;

    if (this->_is_large_system) {
      this->build_trigger_table();
    }

    try {
      if (this->_is_large_system) {
        this->solve_large_system(inv);
      } else {
        this->solve_small_system(inv);
      }
    } catch (BottomFound&) {
      inv.set_to_bottom();
    }
  }

}; // end class LinearIntervalSolver

} // end namespace numeric
} // end namespace core
} // end namespace ikos
