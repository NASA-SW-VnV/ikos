/*******************************************************************************
 *
 * \file
 * \brief Reduced product of DBM (using variable packing) and Congruences
 *
 * The implementation is based on Jorge's IntervalCongruenceDomain.
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
#include <ikos/core/domain/numeric/congruence.hpp>
#include <ikos/core/domain/numeric/domain_product.hpp>
#include <ikos/core/domain/numeric/var_packing_dbm.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief Reduced product of DBM (using variable packing) and congruences
template < typename Number,
           typename VariableRef,
           std::size_t MaxReductionCycles = 10 >
class VarPackingDBMCongruence final
    : public numeric::AbstractDomain<
          Number,
          VariableRef,
          VarPackingDBMCongruence< Number, VariableRef, MaxReductionCycles > > {
public:
  using IntervalT = Interval< Number >;
  using CongruenceT = Congruence< Number >;
  using IntervalCongruenceT = IntervalCongruence< Number >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;
  using LinearConstraintSystemT = LinearConstraintSystem< Number, VariableRef >;

private:
  using VarPackingDBMT =
      VarPackingDBM< Number, VariableRef, MaxReductionCycles >;
  using CongruenceDomainT =
      CongruenceDomain< Number, VariableRef, MaxReductionCycles >;
  using DomainProduct = numeric::
      DomainProduct2< Number, VariableRef, VarPackingDBMT, CongruenceDomainT >;

private:
  DomainProduct _product;

private:
  /// \brief Private constructor
  explicit VarPackingDBMCongruence(DomainProduct product)
      : _product(std::move(product)) {
    this->reduce();
  }

  /// \brief Reduce the equivalence class containing the variable `v`
  void reduce_equivalence_class(VariableRef v) {
    if (this->is_bottom()) {
      return;
    }

    auto& equiv_relation = this->_product.first()._inv._equiv_relation;

    if (!equiv_relation.contains(v)) {
      return;
    }

    auto& equiv_class = equiv_relation.find_equiv_class(v);

    std::shared_ptr< const DBM< Number, VariableRef > > domain_ptr =
        equiv_class.domain_ptr();

    bool change_dbm = true;
    bool change_congruence = true;

    while (change_dbm || change_congruence) {
      change_dbm = false;
      change_congruence = false;

      for (auto it = domain_ptr->var_begin(), et = domain_ptr->var_end();
           it != et;
           ++it) {
        VariableRef x = *it;
        IntervalT i = domain_ptr->to_interval(x);
        CongruenceT c = this->_product.second().to_congruence(x);
        IntervalCongruenceT val(i, c);

        if (val.is_bottom()) {
          this->set_to_bottom();
          return;
        }

        if (val.interval() != i) {
          DBM< Number, VariableRef > domain = *domain_ptr;
          domain.refine(x, val.interval());
          domain.normalize();
          if (domain.is_bottom()) {
            this->set_to_bottom();
            return;
          }
          domain_ptr = std::make_shared< const DBM< Number, VariableRef > >(
              std::move(domain));
          change_dbm = true;
        }

        if (val.congruence() != c) {
          this->_product.second().set(x, val.congruence());
          change_congruence = true;
        }
      }
    }

    equiv_class.set_domain_ptr(domain_ptr);
  }

  /// \brief Reduce the abstract value
  void reduce() {
    if (this->is_bottom()) {
      return;
    }

    auto& equiv_relation = this->_product.first()._inv._equiv_relation;

    for (const auto& equiv_class : equiv_relation) {
      this->reduce_equivalence_class(equiv_class.first);

      if (this->is_bottom()) {
        // iterators are invalidated, exit
        return;
      }
    }
  }

public:
  /// \brief Create the top abstract value
  static VarPackingDBMCongruence top() {
    return VarPackingDBMCongruence(
        DomainProduct(VarPackingDBMT::top(), CongruenceDomainT::top()));
  }

  /// \brief Create the bottom abstract value
  static VarPackingDBMCongruence bottom() {
    return VarPackingDBMCongruence(
        DomainProduct(VarPackingDBMT::bottom(), CongruenceDomainT::bottom()));
  }

  /// \brief Copy constructor
  VarPackingDBMCongruence(const VarPackingDBMCongruence&) = default;

  /// \brief Move constructor
  VarPackingDBMCongruence(VarPackingDBMCongruence&&) = default;

  /// \brief Copy assignment operator
  VarPackingDBMCongruence& operator=(const VarPackingDBMCongruence&) = default;

  /// \brief Move assignment operator
  VarPackingDBMCongruence& operator=(VarPackingDBMCongruence&&) = default;

  /// \brief Destructor
  ~VarPackingDBMCongruence() override = default;

  /// \brief Return the first abstract value
  ///
  /// Note: does not normalize.
  const VarPackingDBMT& first() const { return this->_product.first(); }

  /// \brief Return the first abstract value
  ///
  /// Note: does not normalize.
  VarPackingDBMT& first() { return this->_product.first(); }

  /// \brief Return the second abstract value
  ///
  /// Note: does not normalize.
  const CongruenceDomainT& second() const { return this->_product.second(); }

  /// \brief Return the second abstract value
  ///
  /// Note: does not normalize.
  CongruenceDomainT& second() { return this->_product.second(); }

  void normalize() override { this->_product.normalize(); }

  bool is_bottom() const override { return this->_product.is_bottom(); }

  bool is_top() const override { return this->_product.is_top(); }

  void set_to_bottom() override { this->_product.set_to_bottom(); }

  void set_to_top() override { this->_product.set_to_top(); }

  bool leq(const VarPackingDBMCongruence& other) const override {
    return this->_product.leq(other._product);
  }

  bool equals(const VarPackingDBMCongruence& other) const override {
    return this->_product.equals(other._product);
  }

  void join_with(VarPackingDBMCongruence&& other) override {
    this->_product.join_with(std::move(other._product));
    this->reduce();
  }

  void join_with(const VarPackingDBMCongruence& other) override {
    this->_product.join_with(other._product);
    this->reduce();
  }

  void join_loop_with(VarPackingDBMCongruence&& other) override {
    this->_product.join_loop_with(std::move(other._product));
    this->reduce();
  }

  void join_loop_with(const VarPackingDBMCongruence& other) override {
    this->_product.join_loop_with(other._product);
    this->reduce();
  }

  void join_iter_with(VarPackingDBMCongruence&& other) override {
    this->_product.join_iter_with(std::move(other._product));
    this->reduce();
  }

  void join_iter_with(const VarPackingDBMCongruence& other) override {
    this->_product.join_iter_with(other._product);
    this->reduce();
  }

  void widen_with(const VarPackingDBMCongruence& other) override {
    this->_product.widen_with(other._product);
    this->reduce();
  }

  void widen_threshold_with(const VarPackingDBMCongruence& other,
                            const Number& threshold) override {
    this->_product.widen_threshold_with(other._product, threshold);
    this->reduce();
  }

  void meet_with(const VarPackingDBMCongruence& other) override {
    this->_product.meet_with(other._product);
    this->reduce();
  }

  void narrow_with(const VarPackingDBMCongruence& other) override {
    this->_product.narrow_with(other._product);
    this->reduce();
  }

  void narrow_threshold_with(const VarPackingDBMCongruence& other,
                             const Number& threshold) override {
    this->_product.narrow_threshold_with(other._product, threshold);
    this->reduce();
  }

  VarPackingDBMCongruence join(
      const VarPackingDBMCongruence& other) const override {
    return VarPackingDBMCongruence(this->_product.join(other._product));
  }

  VarPackingDBMCongruence join_loop(
      const VarPackingDBMCongruence& other) const override {
    return VarPackingDBMCongruence(this->_product.join_loop(other._product));
  }

  VarPackingDBMCongruence join_iter(
      const VarPackingDBMCongruence& other) const override {
    return VarPackingDBMCongruence(this->_product.join_iter(other._product));
  }

  VarPackingDBMCongruence widening(
      const VarPackingDBMCongruence& other) const override {
    return VarPackingDBMCongruence(this->_product.widening(other._product));
  }

  VarPackingDBMCongruence widening_threshold(
      const VarPackingDBMCongruence& other,
      const Number& threshold) const override {
    return VarPackingDBMCongruence(
        this->_product.widening_threshold(other._product, threshold));
  }

  VarPackingDBMCongruence meet(
      const VarPackingDBMCongruence& other) const override {
    return VarPackingDBMCongruence(this->_product.meet(other._product));
  }

  VarPackingDBMCongruence narrowing(
      const VarPackingDBMCongruence& other) const override {
    return VarPackingDBMCongruence(this->_product.narrowing(other._product));
  }

  VarPackingDBMCongruence narrowing_threshold(
      const VarPackingDBMCongruence& other,
      const Number& threshold) const override {
    return VarPackingDBMCongruence(
        this->_product.narrowing_threshold(other._product, threshold));
  }

  void assign(VariableRef x, int n) override { this->_product.assign(x, n); }

  void assign(VariableRef x, const Number& n) override {
    this->_product.assign(x, n);
  }

  void assign(VariableRef x, VariableRef y) override {
    this->_product.assign(x, y);
    this->reduce_equivalence_class(x);
  }

  void assign(VariableRef x, const LinearExpressionT& e) override {
    this->_product.assign(x, e);
    this->reduce_equivalence_class(x);
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             VariableRef z) override {
    this->_product.apply(op, x, y, z);
    this->reduce_equivalence_class(x);
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             const Number& z) override {
    this->_product.apply(op, x, y, z);
    this->reduce_equivalence_class(x);
  }

  void apply(BinaryOperator op,
             VariableRef x,
             const Number& y,
             VariableRef z) override {
    this->_product.apply(op, x, y, z);
    this->reduce_equivalence_class(x);
  }

  void add(const LinearConstraintT& cst) override {
    this->_product.add(cst);

    if (this->is_bottom()) {
      return;
    }

    if (cst.num_terms() == 0) {
      return;
    }

    auto it = cst.begin();
    auto it2 = ++cst.begin();

    if ((cst.is_inequality() || cst.is_equality()) &&
        ((cst.num_terms() == 1 && it->second == 1) ||
         (cst.num_terms() == 1 && it->second == -1) ||
         (cst.num_terms() == 2 && it->second == 1 && it2->second == -1) ||
         (cst.num_terms() == 2 && it->second == -1 && it2->second == 1))) {
      // Variables are together in the same equivalence class
      this->reduce_equivalence_class(it->first);
    } else {
      for (const auto& term : cst) {
        this->reduce_equivalence_class(term.first);
      }
    }
  }

  void add(const LinearConstraintSystemT& csts) override {
    this->_product.add(csts);

    if (this->is_bottom()) {
      return;
    }

    for (const LinearConstraintT& cst : csts) {
      if (cst.num_terms() == 0) {
        continue;
      } else {
        auto it = cst.begin();
        auto it2 = ++cst.begin();

        if ((cst.is_inequality() || cst.is_equality()) &&
            ((cst.num_terms() == 1 && it->second == 1) ||
             (cst.num_terms() == 1 && it->second == -1) ||
             (cst.num_terms() == 2 && it->second == 1 && it2->second == -1) ||
             (cst.num_terms() == 2 && it->second == -1 && it2->second == 1))) {
          // Variables are together in the same equivalence class
          this->reduce_equivalence_class(it->first);
        } else {
          for (const auto& term : cst) {
            this->reduce_equivalence_class(term.first);
          }
        }
      }
    }
  }

  void set(VariableRef x, const IntervalT& value) override {
    this->_product.set(x, value);
  }

  void set(VariableRef x, const CongruenceT& value) override {
    this->_product.set(x, value);
  }

  void set(VariableRef x, const IntervalCongruenceT& value) override {
    this->_product.first().set(x, value.interval());
    this->_product.second().set(x, value.congruence());
  }

  void refine(VariableRef x, const IntervalT& value) override {
    this->_product.first().refine(x, value);
    this->reduce_equivalence_class(x);
  }

  void refine(VariableRef x, const CongruenceT& value) override {
    this->_product.second().refine(x, value);
    this->reduce_equivalence_class(x);
  }

  void refine(VariableRef x, const IntervalCongruenceT& value) override {
    this->_product.first().refine(x, value.interval());
    this->_product.second().refine(x, value.congruence());
    this->reduce_equivalence_class(x);
  }

  void forget(VariableRef x) override { this->_product.forget(x); }

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
    return IntervalCongruenceT(this->_product.first().to_interval(x),
                               this->_product.second().to_congruence(x));
  }

  IntervalCongruenceT to_interval_congruence(
      const LinearExpressionT& e) const override {
    return IntervalCongruenceT(this->_product.first().to_interval(e),
                               this->_product.second().to_congruence(e));
  }

  LinearConstraintSystemT to_linear_constraint_system() const override {
    return this->_product.to_linear_constraint_system();
  }

  void dump(std::ostream& o) const override { this->_product.dump(o); }

  static std::string name() {
    return "DBM with variable packing and congruences";
  }

}; // end class VarPackingDBMCongruence

} // end namespace numeric
} // end namespace core
} // end namespace ikos
