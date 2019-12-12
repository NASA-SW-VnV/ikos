/*******************************************************************************
 *
 * \file
 * \brief Optimized version of the DBM abstract domain using variable packing
 *
 * The idea is to put variables together in the same equivalence class only if
 * we can express the relation between them using DBM.
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

#include <ikos/core/domain/numeric/abstract_domain.hpp>
#include <ikos/core/domain/numeric/dbm.hpp>
#include <ikos/core/domain/numeric/linear_interval_solver.hpp>
#include <ikos/core/domain/numeric/var_packing_domain.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief DBM abstract domain using variable packing
template < typename Number,
           typename VariableRef,
           std::size_t MaxReductionCycles = 10 >
class VarPackingDBM final
    : public numeric::AbstractDomain<
          Number,
          VariableRef,
          VarPackingDBM< Number, VariableRef, MaxReductionCycles > > {
public:
  using IntervalT = Interval< Number >;
  using CongruenceT = Congruence< Number >;
  using IntervalCongruenceT = IntervalCongruence< Number >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;
  using LinearConstraintSystemT = LinearConstraintSystem< Number, VariableRef >;

public:
  template < typename, typename, std::size_t >
  friend class VarPackingDBMCongruence;

private:
  using VarPackingDomainT =
      VarPackingDomain< Number,
                        VariableRef,
                        DBM< Number, VariableRef, MaxReductionCycles > >;
  using LinearIntervalSolverT =
      LinearIntervalSolver< Number, VariableRef, VarPackingDBM >;

private:
  VarPackingDomainT _inv;

private:
  /// \brief Private constructor
  explicit VarPackingDBM(VarPackingDomainT inv) : _inv(std::move(inv)) {}

public:
  /// \brief Create the top abstract value
  static VarPackingDBM top() { return VarPackingDBM(VarPackingDomainT::top()); }

  /// \brief Create the bottom abstract value
  static VarPackingDBM bottom() {
    return VarPackingDBM(VarPackingDomainT::bottom());
  }

  /// \brief Copy constructor
  VarPackingDBM(const VarPackingDBM&) = default;

  /// \brief Move constructor
  VarPackingDBM(VarPackingDBM&&) = default;

  /// \brief Copy assignment operator
  VarPackingDBM& operator=(const VarPackingDBM&) = default;

  /// \brief Move assignment operator
  VarPackingDBM& operator=(VarPackingDBM&&) = default;

  /// \brief Destructor
  ~VarPackingDBM() override = default;

  void normalize() override { this->_inv.normalize(); }

  bool is_bottom() const override { return this->_inv.is_bottom(); }

  bool is_top() const override { return this->_inv.is_top(); }

  void set_to_bottom() override { this->_inv.set_to_bottom(); }

  void set_to_top() override { this->_inv.set_to_top(); }

  bool leq(const VarPackingDBM& other) const override {
    return this->_inv.leq(other._inv);
  }

  bool equals(const VarPackingDBM& other) const override {
    return this->_inv.equals(other._inv);
  }

  void join_with(VarPackingDBM&& other) override {
    this->_inv.join_with(std::move(other._inv));
  }

  void join_with(const VarPackingDBM& other) override {
    this->_inv.join_with(other._inv);
  }

  void join_loop_with(VarPackingDBM&& other) override {
    this->_inv.join_loop_with(std::move(other._inv));
  }

  void join_loop_with(const VarPackingDBM& other) override {
    this->_inv.join_loop_with(other._inv);
  }

  void join_iter_with(VarPackingDBM&& other) override {
    this->_inv.join_iter_with(std::move(other._inv));
  }

  void join_iter_with(const VarPackingDBM& other) override {
    this->_inv.join_iter_with(other._inv);
  }

  void widen_with(const VarPackingDBM& other) override {
    this->_inv.widen_with(other._inv);
  }

  void widen_threshold_with(const VarPackingDBM& other,
                            const Number& threshold) override {
    this->_inv.widen_threshold_with(other._inv, threshold);
  }

  void meet_with(const VarPackingDBM& other) override {
    this->_inv.meet_with(other._inv);
  }

  void narrow_with(const VarPackingDBM& other) override {
    this->_inv.narrow_with(other._inv);
  }

  void narrow_threshold_with(const VarPackingDBM& other,
                             const Number& threshold) override {
    this->_inv.narrow_threshold_with(other._inv, threshold);
  }

  VarPackingDBM join(const VarPackingDBM& other) const override {
    return VarPackingDBM(this->_inv.join(other._inv));
  }

  VarPackingDBM join_loop(const VarPackingDBM& other) const override {
    return VarPackingDBM(this->_inv.join_loop(other._inv));
  }

  VarPackingDBM join_iter(const VarPackingDBM& other) const override {
    return VarPackingDBM(this->_inv.join_iter(other._inv));
  }

  VarPackingDBM widening(const VarPackingDBM& other) const override {
    return VarPackingDBM(this->_inv.widening(other._inv));
  }

  VarPackingDBM widening_threshold(const VarPackingDBM& other,
                                   const Number& threshold) const override {
    return VarPackingDBM(this->_inv.widening_threshold(other._inv, threshold));
  }

  VarPackingDBM meet(const VarPackingDBM& other) const override {
    return VarPackingDBM(this->_inv.meet(other._inv));
  }

  VarPackingDBM narrowing(const VarPackingDBM& other) const override {
    return VarPackingDBM(this->_inv.narrowing(other._inv));
  }

  VarPackingDBM narrowing_threshold(const VarPackingDBM& other,
                                    const Number& threshold) const override {
    return VarPackingDBM(this->_inv.narrowing_threshold(other._inv, threshold));
  }

  void assign(VariableRef x, int n) override { this->_inv.assign(x, n); }

  void assign(VariableRef x, const Number& n) override {
    this->_inv.assign(x, n);
  }

  void assign(VariableRef x, VariableRef y) override {
    this->_inv.assign(x, y);
  }

  void assign(VariableRef x, const LinearExpressionT& e) override {
    if (this->_inv._is_bottom) {
      return;
    }

    if (e.is_constant() || (e.num_terms() == 1 && e.begin()->second == 1)) {
      this->_inv.assign(x, e);
    } else {
      // Projection using intervals
      this->_inv.set(x, this->_inv.to_interval(e));
    }
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             VariableRef z) override {
    if (this->_inv.is_bottom()) {
      return;
    }

    IntervalT v_y = this->_inv.to_interval(y);
    IntervalT v_z = this->_inv.to_interval(z);

    if (v_y.singleton()) {
      this->apply(op, x, *v_y.singleton(), z);
    } else if (v_z.singleton()) {
      this->apply(op, x, y, *v_z.singleton());
    } else {
      this->_inv.set(x, apply_bin_operator(op, v_y, v_z));
    }
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             const Number& z) override {
    if (this->_inv.is_bottom()) {
      return;
    }

    if (op == BinaryOperator::Add || op == BinaryOperator::Sub ||
        (op == BinaryOperator::Mul && z == 1) ||
        (op == BinaryOperator::Div && z == 1) || op == BinaryOperator::Mod) {
      this->_inv.apply(op, x, y, z);
      return;
    }

    this->_inv.set(x,
                   apply_bin_operator(op,
                                      this->_inv.to_interval(y),
                                      IntervalT(z)));
  }

  void apply(BinaryOperator op,
             VariableRef x,
             const Number& y,
             VariableRef z) override {
    if (this->_inv.is_bottom()) {
      return;
    }

    if (op == BinaryOperator::Add || (op == BinaryOperator::Mul && y == 1)) {
      this->_inv.apply(op, x, y, z);
      return;
    }

    this->_inv.set(x,
                   apply_bin_operator(op,
                                      IntervalT(y),
                                      this->_inv.to_interval(z)));
  }

  void add(const LinearConstraintT& cst) override {
    if (this->_inv._is_bottom) {
      return;
    }

    if (cst.num_terms() == 0) {
      if (cst.is_contradiction()) {
        this->set_to_bottom();
      }
      return;
    }

    auto it = cst.begin();
    auto it2 = ++cst.begin();

    if ((cst.is_inequality() || cst.is_equality()) &&
        ((cst.num_terms() == 1 && it->second == 1) ||
         (cst.num_terms() == 1 && it->second == -1) ||
         (cst.num_terms() == 2 && it->second == 1 && it2->second == -1) ||
         (cst.num_terms() == 2 && it->second == -1 && it2->second == 1))) {
      this->_inv.add(cst);
    } else {
      if (this->_inv.is_bottom()) {
        return;
      }

      LinearIntervalSolverT solver(MaxReductionCycles);
      solver.add(cst);
      solver.run(*this);
    }
  }

  void add(const LinearConstraintSystemT& csts) override {
    if (this->_inv._is_bottom) {
      return;
    }

    LinearIntervalSolverT solver(MaxReductionCycles);

    for (const LinearConstraintT& cst : csts) {
      // process each constraint
      if (cst.num_terms() == 0) {
        if (cst.is_contradiction()) {
          this->set_to_bottom();
          return;
        }
      } else if (cst.is_inequality() || cst.is_equality()) {
        auto it = cst.begin();
        auto it2 = ++cst.begin();

        if ((cst.num_terms() == 1 && it->second == 1) ||
            (cst.num_terms() == 1 && it->second == -1) ||
            (cst.num_terms() == 2 && it->second == 1 && it2->second == -1) ||
            (cst.num_terms() == 2 && it->second == -1 && it2->second == 1)) {
          this->_inv.add(cst);
        } else {
          solver.add(cst);
        }
      } else {
        solver.add(cst);
      }
    }

    if (!solver.empty()) {
      if (this->_inv.is_bottom()) {
        return;
      }

      solver.run(*this);
    }
  }

  void set(VariableRef x, const IntervalT& value) override {
    this->_inv.set(x, value);
  }

  void set(VariableRef x, const CongruenceT& value) override {
    this->_inv.set(x, value);
  }

  void set(VariableRef x, const IntervalCongruenceT& value) override {
    this->_inv.set(x, value);
  }

  void refine(VariableRef x, const IntervalT& value) override {
    this->_inv.refine(x, value);
  }

  void refine(VariableRef x, const CongruenceT& value) override {
    this->_inv.refine(x, value);
  }

  void refine(VariableRef x, const IntervalCongruenceT& value) override {
    this->_inv.refine(x, value);
  }

  void forget(VariableRef x) override { this->_inv.forget(x); }

  IntervalT to_interval(VariableRef x) const override {
    return this->_inv.to_interval(x);
  }

  IntervalT to_interval(const LinearExpressionT& e) const override {
    return this->_inv.to_interval(e);
  }

  CongruenceT to_congruence(VariableRef x) const override {
    return this->_inv.to_congruence(x);
  }

  CongruenceT to_congruence(const LinearExpressionT& e) const override {
    return this->_inv.to_congruence(e);
  }

  IntervalCongruenceT to_interval_congruence(VariableRef x) const override {
    return this->_inv.to_interval_congruence(x);
  }

  IntervalCongruenceT to_interval_congruence(
      const LinearExpressionT& e) const override {
    return this->_inv.to_interval_congruence(e);
  }

  LinearConstraintSystemT to_linear_constraint_system() const override {
    return this->_inv.to_linear_constraint_system();
  }

  void dump(std::ostream& o) const override { this->_inv.dump(o); }

  static std::string name() { return "DBM with variable packing"; }

}; // end class VarPackingDBM

} // end namespace numeric
} // end namespace core
} // end namespace ikos
