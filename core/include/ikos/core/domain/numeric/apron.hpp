/*******************************************************************************
 *
 * \file
 * \brief Wrapper for the APRON library
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

#include <algorithm>
#include <memory>
#include <mutex>
#include <vector>

#include <ap_global0.h>
#include <ap_pkgrid.h>
#include <ap_ppl.h>
#include <box.h>
#include <oct.h>
#include <pk.h>
#include <pkeq.h>

#include <ikos/core/adt/patricia_tree/map.hpp>
#include <ikos/core/domain/numeric/abstract_domain.hpp>
#include <ikos/core/linear_constraint.hpp>
#include <ikos/core/linear_expression.hpp>
#include <ikos/core/number.hpp>
#include <ikos/core/support/assert.hpp>
#include <ikos/core/value/numeric/congruence.hpp>
#include <ikos/core/value/numeric/interval.hpp>

namespace ikos {
namespace core {
namespace numeric {

namespace apron {

/// \brief Create a binary expression
template < typename Number >
inline ap_texpr0_t* binop_expr(ap_texpr_op_t, ap_texpr0_t*, ap_texpr0_t*);

template <>
inline ap_texpr0_t* binop_expr< ZNumber >(ap_texpr_op_t op,
                                          ap_texpr0_t* l,
                                          ap_texpr0_t* r) {
  return ap_texpr0_binop(op, l, r, AP_RTYPE_INT, AP_RDIR_ZERO);
}

template <>
inline ap_texpr0_t* binop_expr< QNumber >(ap_texpr_op_t op,
                                          ap_texpr0_t* l,
                                          ap_texpr0_t* r) {
  return ap_texpr0_binop(op, l, r, AP_RTYPE_REAL, AP_RDIR_NEAREST);
}

/// \brief Conversion from ikos::ZNumber to ap_scalar_t*
inline ap_scalar_t* to_ap_scalar(const ZNumber& n) {
  mpq_class e(n.mpz());
  return ap_scalar_alloc_set_mpq(e.get_mpq_t());
}

/// \brief Conversion from ikos::QNumber to ap_scalar_t*
inline ap_scalar_t* to_ap_scalar(const QNumber& n) {
  mpq_class e(n.mpq());
  return ap_scalar_alloc_set_mpq(e.get_mpq_t());
}

/// \brief Conversion from ikos::ZNumber to ap_texpr0_t*
inline ap_texpr0_t* to_ap_expr(const ZNumber& n) {
  mpq_class e(n.mpz());
  return ap_texpr0_cst_scalar_mpq(e.get_mpq_t());
}

/// \brief Conversion from ikos::QNumber to ap_texpr0_t*
inline ap_texpr0_t* to_ap_expr(const QNumber& q) {
  mpq_class e(q.mpq());
  return ap_texpr0_cst_scalar_mpq(e.get_mpq_t());
}

/// \brief Conversion from ap_scalar_t* to ikos::ZNumber/QNumber
template < typename Number >
inline Number to_ikos_number(ap_scalar_t*, bool round_upper);

template <>
inline ZNumber to_ikos_number(ap_scalar_t* scalar, bool round_upper) {
  ikos_assert(ap_scalar_infty(scalar) == 0);
  ikos_assert(scalar->discr == AP_SCALAR_MPQ);

  QNumber q(mpq_class(scalar->val.mpq));
  if (round_upper) {
    return q.round_to_upper();
  } else {
    return q.round_to_lower();
  }
}

template <>
inline QNumber to_ikos_number(ap_scalar_t* scalar, bool /*round_upper*/) {
  ikos_assert(ap_scalar_infty(scalar) == 0);
  ikos_assert(scalar->discr == AP_SCALAR_MPQ);

  return QNumber(mpq_class(scalar->val.mpq));
}

/// \brief Conversion from ap_coeff_t* to ikos::ZNumber/QNumber
template < typename Number >
inline Number to_ikos_number(ap_coeff_t* coeff, bool round_upper) {
  ikos_assert(coeff->discr == AP_COEFF_SCALAR);

  return to_ikos_number< Number >(coeff->val.scalar, round_upper);
}

/// \brief Conversion from ap_scalar_t* to ikos::bound< Number >
template < typename Number >
inline Bound< Number > to_ikos_bound(ap_scalar_t* scalar, bool round_upper) {
  using BoundT = Bound< Number >;

  if (ap_scalar_infty(scalar) == -1) {
    return BoundT::minus_infinity();
  } else if (ap_scalar_infty(scalar) == 1) {
    return BoundT::plus_infinity();
  } else {
    return BoundT(to_ikos_number< Number >(scalar, round_upper));
  }
}

/// \brief Conversion from ap_interval_t* to ikos::interval< Number >
template < typename Number >
inline Interval< Number > to_ikos_interval(ap_interval_t* intv) {
  using IntervalT = Interval< Number >;

  if (ap_interval_is_top(intv)) {
    return IntervalT::top();
  }

  if (ap_interval_is_bottom(intv)) {
    return IntervalT::bottom();
  }

  return IntervalT(to_ikos_bound< Number >(intv->inf, true),
                   to_ikos_bound< Number >(intv->sup, false));
}

/// \brief Available abstract domains
enum Domain {
  Interval,
  Octagon,
  PolkaPolyhedra,
  PolkaLinearEqualities,
  PplPolyhedra,
  PplLinearCongruences,
  PkgridPolyhedraLinCongruences,
};

inline const char* domain_name(Domain d) {
  switch (d) {
    case Interval:
      return "APRON Intervals";
    case Octagon:
      return "APRON Octagons";
    case PolkaPolyhedra:
      return "APRON NewPolka Convex Polyhedra";
    case PolkaLinearEqualities:
      return "APRON NewPolka Linear Equalities";
    case PplPolyhedra:
      return "APRON PPL Convex Polyhedra";
    case PplLinearCongruences:
      return "APRON PPL Linear Congruences";
    case PkgridPolyhedraLinCongruences:
      return "APRON Reduced Product of NewPolka Convex Polyhedra and PPL "
             "Linear Congruences";
    default:
      ikos_unreachable("unexpected domain");
  }
}

inline ap_manager_t* alloc_domain_manager(Domain d) {
  switch (d) {
    case Interval:
      return box_manager_alloc();
    case Octagon:
      return oct_manager_alloc();
    case PolkaPolyhedra:
      return pk_manager_alloc(false);
    case PolkaLinearEqualities:
      return pkeq_manager_alloc();
    case PplPolyhedra:
      return ap_ppl_poly_manager_alloc(false);
    case PplLinearCongruences:
      return ap_ppl_grid_manager_alloc();
    case PkgridPolyhedraLinCongruences:
      return ap_pkgrid_manager_alloc(pk_manager_alloc(false),
                                     ap_ppl_grid_manager_alloc());
    default:
      ikos_unreachable("unexpected domain");
  }
}

} // end namespace apron

/// \brief Wrapper for APRON abstract domains
template < apron::Domain Domain, typename Number, typename VariableRef >
class ApronDomain final : public numeric::AbstractDomain<
                              Number,
                              VariableRef,
                              ApronDomain< Domain, Number, VariableRef > > {
public:
  using BoundT = Bound< Number >;
  using IntervalT = Interval< Number >;
  using CongruenceT = Congruence< Number >;
  using IntervalCongruenceT = IntervalCongruence< Number >;
  using VariableExprT = VariableExpression< Number, VariableRef >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;
  using LinearConstraintSystemT = LinearConstraintSystem< Number, VariableRef >;

private:
  using VariableMap = PatriciaTreeMap< VariableRef, ap_dim_t >;
  using Parent = numeric::AbstractDomain< Number, VariableRef, ApronDomain >;

  /// \brief Deleter for ap_abstract0_t*
  struct InvDeleter {
    void operator()(ap_abstract0_t* inv) { ap_abstract0_free(manager(), inv); }
  };

  /// \brief Wrapper for ap_abstract0_t
  using InvPtr = std::unique_ptr< ap_abstract0_t, InvDeleter >;

private:
  mutable std::mutex _mutex;
  InvPtr _inv;
  VariableMap _var_map;

private:
  /// \brief Get the manager for the given apron domain
  static ap_manager_t* manager() {
    // Initialized at first call
    static ap_manager_t* Man = apron::alloc_domain_manager(Domain);
    return Man;
  }

  /*
   * Dimension utils
   */

  /// \brief Return the size of an ap_abstract0_t
  static std::size_t dimension(ap_abstract0_t* inv) {
    return ap_abstract0_dimension(manager(), inv).intdim;
  }

  /// \brief Return a dimension change that adds a number of dimensions
  static ap_dimchange_t* add_dimensions(ap_abstract0_t* inv, std::size_t dims) {
    ikos_assert(dims > 0);

    ap_dimchange_t* dimchange = ap_dimchange_alloc(dims, 0);
    for (std::size_t i = 0; i < dims; i++) {
      // Add dimension at the end
      dimchange->dim[i] = static_cast< ap_dim_t >(dimension(inv));
    }

    return dimchange;
  }

  /// \brief Return a dimension change that removes the given dimensions of an
  /// ap_abstract0_t
  static ap_dimchange_t* remove_dimensions(
      ap_abstract0_t* /*inv*/, const std::vector< ap_dim_t >& dims) {
    ikos_assert(!dims.empty());
    ikos_assert(std::is_sorted(dims.begin(), dims.end()));

    // Make sure that the removed dimensions are in ascending order
    ap_dimchange_t* dimchange = ap_dimchange_alloc(dims.size(), 0);
    for (std::size_t i = 0; i < dims.size(); i++) {
      // Remove dimension dims[i] and shift to the left all the dimensions
      // greater than dims[i]
      dimchange->dim[i] = dims[i];
    }

    return dimchange;
  }

  /*
   * Variable dimension utils
   */

  /// \brief Get the dimension associated to a variable
  boost::optional< const ap_dim_t& > var_dim(VariableRef v) const {
    return this->_var_map.at(v);
  }

  /// \brief Get the dimension associated to a variable, or create one
  ap_dim_t var_dim_insert(VariableRef v) {
    boost::optional< const ap_dim_t& > dim = this->_var_map.at(v);

    if (dim) {
      return *dim;
    }

    auto new_dim = static_cast< ap_dim_t >(this->_var_map.size());
    this->_var_map.insert_or_assign(v, new_dim);
    ap_dimchange_t* dimchange = add_dimensions(this->_inv.get(), 1);
    ap_abstract0_add_dimensions(manager(),
                                true,
                                this->_inv.get(),
                                dimchange,
                                false);
    ap_dimchange_free(dimchange);
    return new_dim;
  }

  /*
   * Abstract operator utils
   */

  /// \brief Merge two variable maps, updating the associated abstract values
  static VariableMap merge_var_maps(const VariableMap& lhs_var_map,
                                    ap_abstract0_t* lhs_inv,
                                    const VariableMap& rhs_var_map,
                                    ap_abstract0_t* rhs_inv) {
    ikos_assert(lhs_var_map.size() == dimension(lhs_inv));
    ikos_assert(rhs_var_map.size() == dimension(rhs_inv));

    // Build a result variable map, based on lhs_var_map
    VariableMap result = lhs_var_map;

    // Add variables from rhs_var_map to the result variable map
    for (auto it = rhs_var_map.begin(), et = rhs_var_map.end(); it != et;
         ++it) {
      VariableRef rhs_v = it->first;
      boost::optional< const ap_dim_t& > lhs_dim = lhs_var_map.at(rhs_v);

      if (!lhs_dim) {
        auto dim = static_cast< ap_dim_t >(result.size());
        result.insert_or_assign(rhs_v, dim);
      }
    }

    // Add the necessary dimensions to lhs_inv and rhs_inv
    if (result.size() > lhs_var_map.size()) {
      ap_dimchange_t* dimchange =
          add_dimensions(lhs_inv, result.size() - lhs_var_map.size());
      ap_abstract0_add_dimensions(manager(), true, lhs_inv, dimchange, false);
      ap_dimchange_free(dimchange);
    }
    if (result.size() > rhs_var_map.size()) {
      ap_dimchange_t* dimchange =
          add_dimensions(rhs_inv, result.size() - rhs_var_map.size());
      ap_abstract0_add_dimensions(manager(), true, rhs_inv, dimchange, false);
      ap_dimchange_free(dimchange);
    }

    ikos_assert(result.size() == dimension(lhs_inv));
    ikos_assert(result.size() == dimension(rhs_inv));

    // Build and apply the permutation map for rhs_inv
    ap_dimperm_t* rhs_perm = build_perm_map(rhs_var_map, result);
    ap_abstract0_permute_dimensions(manager(), true, rhs_inv, rhs_perm);
    ap_dimperm_free(rhs_perm);

    ikos_assert(result.size() == dimension(lhs_inv));
    ikos_assert(result.size() == dimension(rhs_inv));

    return result;
  }

  /// \brief Return a permutation map from the old to the new map
  static ap_dimperm_t* build_perm_map(const VariableMap& old_map,
                                      const VariableMap& new_map) {
    std::size_t n = new_map.size();
    ap_dimperm_t* perm = ap_dimperm_alloc(n);
    std::vector< bool > index_assigned(n, false);
    std::vector< bool > value_assigned(n, false);

    for (auto it = old_map.begin(), et = old_map.end(); it != et; ++it) {
      boost::optional< const ap_dim_t& > dim = new_map.at(it->first);
      ikos_assert(dim);

      perm->dim[it->second] = *dim;
      index_assigned[it->second] = true;
      value_assigned[*dim] = true;
    }

    ap_dim_t counter = 0;
    for (std::size_t i = 0; i < n; i++) {
      if (index_assigned[i]) {
        continue;
      }

      while (value_assigned[counter]) {
        counter++;
      }

      perm->dim[i] = counter;
      counter++;
    }

    return perm;
  }

  /// \brief Return the narrowing of the given invariants
  static ap_abstract0_t* apron_narrowing(ap_abstract0_t* a, ap_abstract0_t* b) {
    if (Domain == apron::Octagon) {
      return ap_abstract0_oct_narrowing(manager(), a, b);
    } else {
      // by default, use meet
      return ap_abstract0_meet(manager(), false, a, b);
    }
  }

  /// \brief Conversion from VariableRef to ap_texpr0_t*
  ap_texpr0_t* to_ap_expr(VariableRef v) {
    return ap_texpr0_dim(this->var_dim_insert(v));
  }

  /// \brief Conversion from LinearExpression to ap_texpr0_t*
  ap_texpr0_t* to_ap_expr(const LinearExpressionT& e) {
    ap_texpr0_t* r = apron::to_ap_expr(e.constant());

    for (auto it = e.begin(), et = e.end(); it != et; ++it) {
      ap_texpr0_t* term =
          apron::binop_expr< Number >(AP_TEXPR_MUL,
                                      apron::to_ap_expr(it->second),
                                      this->to_ap_expr(it->first));
      r = apron::binop_expr< Number >(AP_TEXPR_ADD, r, term);
    }

    return r;
  }

  /// \brief Conversion from LinearConstraint to ap_tcons0_t
  ap_tcons0_t to_ap_constraint(const LinearConstraintT& cst) {
    const LinearExpressionT& exp = cst.expression();

    if (cst.is_equality()) {
      return ap_tcons0_make(AP_CONS_EQ, this->to_ap_expr(exp), nullptr);
    } else if (cst.is_inequality()) {
      return ap_tcons0_make(AP_CONS_SUPEQ, this->to_ap_expr(-exp), nullptr);
    } else {
      return ap_tcons0_make(AP_CONS_DISEQ, this->to_ap_expr(exp), nullptr);
    }
  }

  /// \brief Conversion from ap_linexpr0_t* to LinearExpression
  LinearExpressionT to_ikos_linear_expression(ap_linexpr0_t* expr) const {
    ikos_assert(ap_linexpr0_is_linear(expr));

    ap_coeff_t* coeff = ap_linexpr0_cstref(expr);
    LinearExpressionT e(apron::to_ikos_number< Number >(coeff, false));

    for (auto it = _var_map.begin(), et = _var_map.end(); it != et; ++it) {
      coeff = ap_linexpr0_coeffref(expr, it->second);

      if (ap_coeff_zero(coeff)) {
        continue;
      }

      e.add(apron::to_ikos_number< Number >(coeff, false), it->first);
    }

    return e;
  }

  /// \brief Conversion from ap_lincons0_t to LinearConstraint
  LinearConstraintT to_ikos_linear_constraint(const ap_lincons0_t& cst) const {
    ikos_assert(cst.scalar == nullptr);

    LinearExpressionT e = this->to_ikos_linear_expression(cst.linexpr0);

    switch (cst.constyp) {
      case AP_CONS_EQ:
        return LinearConstraintT(std::move(e), LinearConstraintT::Equality);
      case AP_CONS_SUPEQ:
        return LinearConstraintT(-std::move(e), LinearConstraintT::Inequality);
      case AP_CONS_SUP:
        return LinearConstraintT(-std::move(e) + 1,
                                 LinearConstraintT::Inequality);
      case AP_CONS_DISEQ:
        return LinearConstraintT(std::move(e), LinearConstraintT::Disequation);
      case AP_CONS_EQMOD:
      default:
        ikos_unreachable("unexpected linear constraint");
    }
  }

  /// \brief Return true if the variable mapping is the same
  bool same_var_map(const ApronDomain& other) const {
    return dimension(this->_inv.get()) == dimension(other._inv.get()) &&
           this->_var_map.size() == other._var_map.size() &&
           this->_var_map.equals(other._var_map,
                                 [](ap_dim_t x, ap_dim_t y) { return x == y; });
  }

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Private constructor
  ApronDomain(InvPtr inv, VariableMap var_map)
      : _inv(std::move(inv)), _var_map(std::move(var_map)) {}

  /// \brief Create the top abstract value
  explicit ApronDomain(TopTag) : _inv(ap_abstract0_top(manager(), 0, 0)) {}

  /// \brief Create the bottom abstract value
  explicit ApronDomain(BottomTag)
      : _inv(ap_abstract0_bottom(manager(), 0, 0)) {}

public:
  /// \brief Create the top abstract value
  static ApronDomain top() { return ApronDomain(TopTag{}); }

  /// \brief Create the bottom abstract value
  static ApronDomain bottom() { return ApronDomain(BottomTag{}); }

  /// \brief Copy constructor
  ApronDomain(const ApronDomain& other) {
    std::lock_guard< std::mutex > lock(other._mutex);
    this->_inv = InvPtr(ap_abstract0_copy(manager(), other._inv.get()));
    this->_var_map = other._var_map;
  }

  /// \brief Move constructor
  ApronDomain(ApronDomain&& other) noexcept
      : _inv(std::move(other._inv)), _var_map(std::move(other._var_map)) {}

  /// \brief Copy assignment operator
  ApronDomain& operator=(const ApronDomain& other) {
    std::lock_guard< std::mutex > lock(other._mutex);
    this->_inv = InvPtr(ap_abstract0_copy(manager(), other._inv.get()));
    this->_var_map = other._var_map;
    return *this;
  }

  /// \brief Move assignment operator
  ApronDomain& operator=(ApronDomain&& other) noexcept {
    this->_inv = std::move(other._inv);
    this->_var_map = std::move(other._var_map);
    return *this;
  }

  /// \brief Destructor
  ~ApronDomain() override = default;

  void normalize() override {
    std::lock_guard< std::mutex > lock(this->_mutex);
    ap_abstract0_canonicalize(manager(), this->_inv.get());
  }

  bool is_bottom() const override {
    std::lock_guard< std::mutex > lock(this->_mutex);
    return ap_abstract0_is_bottom(manager(), this->_inv.get());
  }

  bool is_top() const override {
    std::lock_guard< std::mutex > lock(this->_mutex);
    return ap_abstract0_is_top(manager(), this->_inv.get());
  }

  void set_to_bottom() override {
    this->_inv = InvPtr(ap_abstract0_bottom(manager(), 0, 0));
    this->_var_map.clear();
  }

  void set_to_top() override {
    this->_inv = InvPtr(ap_abstract0_top(manager(), 0, 0));
    this->_var_map.clear();
  }

  bool leq(const ApronDomain& other) const override {
    if (this == &other) {
      return true;
    }

    std::lock(this->_mutex, other._mutex);
    std::lock_guard< std::mutex > lock_this(this->_mutex, std::adopt_lock);
    std::lock_guard< std::mutex > lock_other(other._mutex, std::adopt_lock);

    if (ap_abstract0_is_bottom(manager(), this->_inv.get())) {
      return true;
    } else if (ap_abstract0_is_bottom(manager(), other._inv.get())) {
      return false;
    } else if (this->same_var_map(other)) {
      return ap_abstract0_is_leq(manager(), this->_inv.get(), other._inv.get());
    } else {
      InvPtr lhs = InvPtr(ap_abstract0_copy(manager(), this->_inv.get()));
      InvPtr rhs = InvPtr(ap_abstract0_copy(manager(), other._inv.get()));
      merge_var_maps(this->_var_map, lhs.get(), other._var_map, rhs.get());
      return ap_abstract0_is_leq(manager(), lhs.get(), rhs.get());
    }
  }

  bool equals(const ApronDomain& other) const override {
    if (this == &other) {
      return true;
    }

    std::lock(this->_mutex, other._mutex);
    std::lock_guard< std::mutex > lock_this(this->_mutex, std::adopt_lock);
    std::lock_guard< std::mutex > lock_other(other._mutex, std::adopt_lock);

    if (ap_abstract0_is_bottom(manager(), this->_inv.get())) {
      return ap_abstract0_is_bottom(manager(), other._inv.get());
    } else if (ap_abstract0_is_bottom(manager(), other._inv.get())) {
      return false;
    } else if (this->same_var_map(other)) {
      return ap_abstract0_is_eq(manager(), this->_inv.get(), other._inv.get());
    } else {
      InvPtr lhs = InvPtr(ap_abstract0_copy(manager(), this->_inv.get()));
      InvPtr rhs = InvPtr(ap_abstract0_copy(manager(), other._inv.get()));
      merge_var_maps(this->_var_map, lhs.get(), other._var_map, rhs.get());
      return ap_abstract0_is_eq(manager(), lhs.get(), rhs.get());
    }
  }

  void join_with(ApronDomain&& other) override {
    if (this == &other) {
      return;
    }

    std::lock(this->_mutex, other._mutex);
    std::lock_guard< std::mutex > lock_this(this->_mutex, std::adopt_lock);
    std::lock_guard< std::mutex > lock_other(other._mutex, std::adopt_lock);

    if (ap_abstract0_is_bottom(manager(), this->_inv.get())) {
      this->_inv = std::move(other._inv);
      this->_var_map = std::move(other._var_map);
    } else if (ap_abstract0_is_bottom(manager(), other._inv.get())) {
      return;
    } else if (this->same_var_map(other)) {
      ap_abstract0_join(manager(), true, this->_inv.get(), other._inv.get());
    } else {
      this->_var_map = merge_var_maps(this->_var_map,
                                      this->_inv.get(),
                                      other._var_map,
                                      other._inv.get());
      ap_abstract0_join(manager(), true, this->_inv.get(), other._inv.get());
    }
  }

  void join_with(const ApronDomain& other) override {
    if (this == &other) {
      return;
    }

    std::lock(this->_mutex, other._mutex);
    std::lock_guard< std::mutex > lock_this(this->_mutex, std::adopt_lock);
    std::lock_guard< std::mutex > lock_other(other._mutex, std::adopt_lock);

    if (ap_abstract0_is_bottom(manager(), this->_inv.get())) {
      this->_inv = InvPtr(ap_abstract0_copy(manager(), other._inv.get()));
      this->_var_map = other._var_map;
    } else if (ap_abstract0_is_bottom(manager(), other._inv.get())) {
      return;
    } else if (this->same_var_map(other)) {
      ap_abstract0_join(manager(), true, this->_inv.get(), other._inv.get());
    } else {
      InvPtr rhs = InvPtr(ap_abstract0_copy(manager(), other._inv.get()));
      this->_var_map = merge_var_maps(this->_var_map,
                                      this->_inv.get(),
                                      other._var_map,
                                      rhs.get());
      ap_abstract0_join(manager(), true, this->_inv.get(), rhs.get());
    }
  }

  ApronDomain join(const ApronDomain& other) const override {
    if (this == &other) {
      return *this;
    }

    std::lock(this->_mutex, other._mutex);
    std::lock_guard< std::mutex > lock_this(this->_mutex, std::adopt_lock);
    std::lock_guard< std::mutex > lock_other(other._mutex, std::adopt_lock);

    if (ap_abstract0_is_bottom(manager(), this->_inv.get())) {
      return ApronDomain(InvPtr(ap_abstract0_copy(manager(), other._inv.get())),
                         other._var_map);
    } else if (ap_abstract0_is_bottom(manager(), other._inv.get())) {
      return ApronDomain(InvPtr(ap_abstract0_copy(manager(), this->_inv.get())),
                         this->_var_map);
    } else if (this->same_var_map(other)) {
      return ApronDomain(InvPtr(ap_abstract0_join(manager(),
                                                  false,
                                                  this->_inv.get(),
                                                  other._inv.get())),
                         this->_var_map);
    } else {
      InvPtr lhs = InvPtr(ap_abstract0_copy(manager(), this->_inv.get()));
      InvPtr rhs = InvPtr(ap_abstract0_copy(manager(), other._inv.get()));
      VariableMap var_map =
          merge_var_maps(this->_var_map, lhs.get(), other._var_map, rhs.get());
      ap_abstract0_join(manager(), true, lhs.get(), rhs.get());
      return ApronDomain(std::move(lhs), var_map);
    }
  }

  ApronDomain widening(const ApronDomain& other) const override {
    if (this == &other) {
      return *this;
    }

    std::lock(this->_mutex, other._mutex);
    std::lock_guard< std::mutex > lock_this(this->_mutex, std::adopt_lock);
    std::lock_guard< std::mutex > lock_other(other._mutex, std::adopt_lock);

    if (ap_abstract0_is_bottom(manager(), this->_inv.get())) {
      return ApronDomain(InvPtr(ap_abstract0_copy(manager(), other._inv.get())),
                         other._var_map);
    } else if (ap_abstract0_is_bottom(manager(), other._inv.get())) {
      return ApronDomain(InvPtr(ap_abstract0_copy(manager(), this->_inv.get())),
                         this->_var_map);
    } else if (this->same_var_map(other)) {
      return ApronDomain(InvPtr(ap_abstract0_widening(manager(),
                                                      this->_inv.get(),
                                                      other._inv.get())),
                         this->_var_map);
    } else {
      InvPtr lhs = InvPtr(ap_abstract0_copy(manager(), this->_inv.get()));
      InvPtr rhs = InvPtr(ap_abstract0_copy(manager(), other._inv.get()));
      VariableMap var_map =
          merge_var_maps(this->_var_map, lhs.get(), other._var_map, rhs.get());
      return ApronDomain(InvPtr(ap_abstract0_widening(manager(),
                                                      lhs.get(),
                                                      rhs.get())),
                         var_map);
    }
  }

  void widen_with(const ApronDomain& other) override {
    this->operator=(this->widening(other));
  }

  ApronDomain widening_threshold(const ApronDomain& other,
                                 const Number& /*threshold*/) const override {
    return this->widening(other);
  }

  void widen_threshold_with(const ApronDomain& other,
                            const Number& /*threshold*/) override {
    this->widen_with(other);
  }

  ApronDomain meet(const ApronDomain& other) const override {
    if (this == &other) {
      return *this;
    }

    std::lock(this->_mutex, other._mutex);
    std::lock_guard< std::mutex > lock_this(this->_mutex, std::adopt_lock);
    std::lock_guard< std::mutex > lock_other(other._mutex, std::adopt_lock);

    if (ap_abstract0_is_bottom(manager(), this->_inv.get()) ||
        ap_abstract0_is_bottom(manager(), other._inv.get())) {
      return bottom();
    } else if (this->same_var_map(other)) {
      return ApronDomain(InvPtr(ap_abstract0_meet(manager(),
                                                  false,
                                                  this->_inv.get(),
                                                  other._inv.get())),
                         this->_var_map);
    } else {
      InvPtr lhs = InvPtr(ap_abstract0_copy(manager(), this->_inv.get()));
      InvPtr rhs = InvPtr(ap_abstract0_copy(manager(), other._inv.get()));
      VariableMap var_map =
          merge_var_maps(this->_var_map, lhs.get(), other._var_map, rhs.get());
      ap_abstract0_meet(manager(), true, lhs.get(), rhs.get());
      return ApronDomain(std::move(lhs), var_map);
    }
  }

  void meet_with(const ApronDomain& other) override {
    this->operator=(this->meet(other));
  }

  ApronDomain narrowing(const ApronDomain& other) const override {
    if (this == &other) {
      return *this;
    }

    std::lock(this->_mutex, other._mutex);
    std::lock_guard< std::mutex > lock_this(this->_mutex, std::adopt_lock);
    std::lock_guard< std::mutex > lock_other(other._mutex, std::adopt_lock);

    if (ap_abstract0_is_bottom(manager(), this->_inv.get()) ||
        ap_abstract0_is_bottom(manager(), other._inv.get())) {
      return bottom();
    } else if (this->same_var_map(other)) {
      return ApronDomain(InvPtr(apron_narrowing(this->_inv.get(),
                                                other._inv.get())),
                         this->_var_map);
    } else {
      InvPtr lhs = InvPtr(ap_abstract0_copy(manager(), this->_inv.get()));
      InvPtr rhs = InvPtr(ap_abstract0_copy(manager(), other._inv.get()));
      VariableMap var_map =
          merge_var_maps(this->_var_map, lhs.get(), other._var_map, rhs.get());
      return ApronDomain(InvPtr(apron_narrowing(lhs.get(), rhs.get())),
                         var_map);
    }
  }

  void narrow_with(const ApronDomain& other) override {
    this->operator=(this->narrowing(other));
  }

  ApronDomain narrowing_threshold(const ApronDomain& other,
                                  const Number& /*threshold*/) const override {
    return this->narrowing(other);
  }

  void narrow_threshold_with(const ApronDomain& other,
                             const Number& /*threshold*/) override {
    this->narrow_with(other);
  }

  void assign(VariableRef x, int n) override {
    this->assign(x, LinearExpressionT(n));
  }

  void assign(VariableRef x, const Number& n) override {
    this->assign(x, LinearExpressionT(n));
  }

  void assign(VariableRef x, VariableRef y) override {
    this->assign(x, LinearExpressionT(y));
  }

  void assign(VariableRef x, const LinearExpressionT& e) override {
    std::lock_guard< std::mutex > lock(this->_mutex);

    if (ap_abstract0_is_bottom(manager(), this->_inv.get())) {
      return;
    }

    ap_texpr0_t* t = this->to_ap_expr(e);
    ap_dim_t v_dim = this->var_dim_insert(x);
    ap_abstract0_assign_texpr(manager(),
                              true,
                              this->_inv.get(),
                              v_dim,
                              t,
                              nullptr);
    ap_texpr0_free(t);
  }

private:
  /// \brief Return true if the operator is supported
  static bool is_supported(BinaryOperator op) {
    switch (op) {
      case BinaryOperator::Add:
      case BinaryOperator::Sub:
      case BinaryOperator::Mul:
      case BinaryOperator::Div:
      case BinaryOperator::Rem:
        return true;
      default:
        return false;
    }
  }

  /// \brief Apply `x = left op right`
  void apply(BinaryOperator op,
             VariableRef x,
             ap_texpr0_t* left,
             ap_texpr0_t* right) {
    ap_texpr0_t* t;

    switch (op) {
      case BinaryOperator::Add: {
        t = apron::binop_expr< Number >(AP_TEXPR_ADD, left, right);
      } break;
      case BinaryOperator::Sub: {
        t = apron::binop_expr< Number >(AP_TEXPR_SUB, left, right);
      } break;
      case BinaryOperator::Mul: {
        t = apron::binop_expr< Number >(AP_TEXPR_MUL, left, right);
      } break;
      case BinaryOperator::Div: {
        t = apron::binop_expr< Number >(AP_TEXPR_DIV, left, right);
      } break;
      case BinaryOperator::Rem: {
        // XXX(marthaud): AP_TEXPR_MOD is actually a signed remainder..
        t = apron::binop_expr< Number >(AP_TEXPR_MOD, left, right);
      } break;
      default: {
        ikos_unreachable("unsupported operator");
      }
    }

    ap_dim_t x_dim = this->var_dim_insert(x);
    ap_abstract0_assign_texpr(manager(),
                              true,
                              this->_inv.get(),
                              x_dim,
                              t,
                              nullptr);
    ap_texpr0_free(t);
  }

public:
  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             VariableRef z) override {
    if (this->is_bottom()) {
      return;
    }

    if (is_supported(op)) {
      std::lock_guard< std::mutex > lock(this->_mutex);
      this->apply(op, x, this->to_ap_expr(y), this->to_ap_expr(z));
    } else {
      this->set(x,
                apply_bin_operator(op,
                                   this->to_interval(y),
                                   this->to_interval(z)));
    }
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             const Number& z) override {
    if (this->is_bottom()) {
      return;
    }

    if (is_supported(op)) {
      std::lock_guard< std::mutex > lock(this->_mutex);
      this->apply(op, x, this->to_ap_expr(y), apron::to_ap_expr(z));
    } else if (op == BinaryOperator::Mod) {
      // Optimized version, because mod is heavily used on machine integers
      if (z == 0) {
        this->set_to_bottom();
        return;
      }

      IntervalT v_y = this->to_interval(y);
      boost::optional< Number > n = v_y.mod_to_sub(z);

      if (n) {
        // Equivalent to x = y - n
        this->apply(BinaryOperator::Sub, x, y, *n);
      } else {
        this->set(x, IntervalT(BoundT(0), BoundT(abs(z) - 1)));
      }
    } else {
      this->set(x, apply_bin_operator(op, this->to_interval(y), IntervalT(z)));
    }
  }

  void apply(BinaryOperator op,
             VariableRef x,
             const Number& y,
             VariableRef z) override {
    if (this->is_bottom()) {
      return;
    }

    if (is_supported(op)) {
      std::lock_guard< std::mutex > lock(this->_mutex);
      this->apply(op, x, apron::to_ap_expr(y), this->to_ap_expr(z));
    } else {
      this->set(x, apply_bin_operator(op, IntervalT(y), this->to_interval(z)));
    }
  }

  void add(const LinearConstraintT& cst) override {
    this->add(LinearConstraintSystemT{cst});
  }

  void add(const LinearConstraintSystemT& csts) override {
    std::lock_guard< std::mutex > lock(this->_mutex);

    if (csts.empty()) {
      return;
    }

    if (ap_abstract0_is_bottom(manager(), this->_inv.get())) {
      return;
    }

    ap_tcons0_array_t ap_csts = ap_tcons0_array_make(csts.size());

    std::size_t i = 0;
    for (const LinearConstraintT& cst : csts) {
      ap_csts.p[i++] = this->to_ap_constraint(cst);
    }

    ap_abstract0_meet_tcons_array(manager(), true, this->_inv.get(), &ap_csts);

    // Improve the precision
    for (i = 0; i < csts.size() &&
                !ap_abstract0_is_bottom(manager(), this->_inv.get());
         i++) {
      // Check satisfiability of ap_csts.p[i]
      ap_tcons0_t& cst = ap_csts.p[i];
      ap_interval_t* ap_intv =
          ap_abstract0_bound_texpr(manager(), this->_inv.get(), cst.texpr0);
      IntervalT intv = apron::to_ikos_interval< Number >(ap_intv);

      if (intv.is_bottom() ||
          (cst.constyp == AP_CONS_EQ && !intv.contains(0)) ||
          (cst.constyp == AP_CONS_SUPEQ && intv.ub() < BoundT(0)) ||
          (cst.constyp == AP_CONS_DISEQ && intv == IntervalT(0))) {
        // Cst is not satisfiable
        this->set_to_bottom();
      }

      ap_interval_free(ap_intv);
    }

    ap_tcons0_array_clear(&ap_csts);
  }

  void set(VariableRef x, const IntervalT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->forget(x);
      this->refine(x, value);
    }
  }

  void set(VariableRef x, const CongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->forget(x);
      this->refine(x, value);
    }
  }

  void set(VariableRef x, const IntervalCongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->forget(x);
      this->refine(x, value.interval());
      this->refine(x, value.congruence());
    }
  }

  void refine(VariableRef x, const IntervalT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else if (value.is_top()) {
      return;
    } else {
      this->add(within_interval(x, value));
    }
  }

  void refine(VariableRef x, const CongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else if (value.is_top()) {
      return;
    } else if (value.singleton()) {
      this->add(VariableExprT(x) == *value.singleton());
    } else {
      std::lock_guard< std::mutex > lock(this->_mutex);
      ap_tcons0_array_t csts = ap_tcons0_array_make(1);
      csts.p[0] =
          ap_tcons0_make(AP_CONS_EQMOD,
                         this->to_ap_expr(VariableExprT(x) - value.residue()),
                         apron::to_ap_scalar(value.modulus()));
      ap_abstract0_meet_tcons_array(manager(), true, this->_inv.get(), &csts);
      ap_tcons0_array_clear(&csts);
    }
  }

  void refine(VariableRef x, const IntervalCongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->refine(x, value.interval());
      this->refine(x, value.congruence());
    }
  }

  void forget(VariableRef x) override {
    std::lock_guard< std::mutex > lock(this->_mutex);
    boost::optional< const ap_dim_t& > has_dim = this->var_dim(x);

    if (!has_dim) {
      return;
    }

    ap_dim_t dim = *has_dim;
    std::vector< ap_dim_t > vector_dims{dim};
    ap_abstract0_forget_array(manager(),
                              true,
                              this->_inv.get(),
                              &vector_dims[0],
                              vector_dims.size(),
                              false);
    ap_dimchange_t* dimchange =
        remove_dimensions(this->_inv.get(), vector_dims);
    ap_abstract0_remove_dimensions(manager(),
                                   true,
                                   this->_inv.get(),
                                   dimchange);
    ap_dimchange_free(dimchange);
    this->_var_map.transform([dim](VariableRef, ap_dim_t d) {
      if (d < dim) {
        return boost::optional< ap_dim_t >(d);
      } else if (d == dim) {
        return boost::optional< ap_dim_t >(boost::none);
      } else { // d > dim
        return boost::optional< ap_dim_t >(d - 1);
      }
    });
    ikos_assert(this->_var_map.size() == dimension(this->_inv.get()));
  }

  IntervalT to_interval(VariableRef x) const override {
    std::lock_guard< std::mutex > lock(this->_mutex);

    if (ap_abstract0_is_bottom(manager(), this->_inv.get())) {
      return IntervalT::bottom();
    }

    boost::optional< const ap_dim_t& > dim = this->var_dim(x);

    if (!dim) {
      return IntervalT::top();
    }

    ap_interval_t* intv =
        ap_abstract0_bound_dimension(manager(), this->_inv.get(), *dim);
    IntervalT r = apron::to_ikos_interval< Number >(intv);
    ap_interval_free(intv);
    return r;
  }

  IntervalT to_interval(const LinearExpressionT& e) const override {
    return Parent::to_interval(e);
  }

  CongruenceT to_congruence(VariableRef) const override {
    if (this->is_bottom()) {
      return CongruenceT::bottom();
    }

    return CongruenceT::top();
  }

  CongruenceT to_congruence(const LinearExpressionT& e) const override {
    return Parent::to_congruence(e);
  }

  IntervalCongruenceT to_interval_congruence(VariableRef x) const override {
    if (this->is_bottom()) {
      return IntervalCongruenceT::bottom();
    }

    return IntervalCongruenceT(this->to_interval(x), this->to_congruence(x));
  }

  IntervalCongruenceT to_interval_congruence(
      const LinearExpressionT& e) const override {
    if (this->is_bottom()) {
      return IntervalCongruenceT::bottom();
    }

    return IntervalCongruenceT(this->to_interval(e), this->to_congruence(e));
  }

  LinearConstraintSystemT to_linear_constraint_system() const override {
    std::lock_guard< std::mutex > lock(this->_mutex);

    if (ap_abstract0_is_bottom(manager(), this->_inv.get())) {
      return LinearConstraintSystemT(LinearConstraintT::contradiction());
    }

    LinearConstraintSystemT csts;
    ap_lincons0_array_t ap_csts =
        ap_abstract0_to_lincons_array(manager(), this->_inv.get());
    for (std::size_t i = 0; i < ap_csts.size; i++) {
      ap_lincons0_t& ap_cst = ap_csts.p[i];

      if (ap_cst.constyp == AP_CONS_EQMOD) {
        // ikos::LinearConstraint does not support modular equality
        continue;
      }

      csts.add(this->to_ikos_linear_constraint(ap_cst));
    }

    ap_lincons0_array_clear(&ap_csts);
    return csts;
  }

  void dump(std::ostream& o) const override {
    std::lock_guard< std::mutex > lock(this->_mutex);

    if (ap_abstract0_is_bottom(manager(), this->_inv.get())) {
      o << "⊥";
      return;
    }

#if 1
    o << "{";

    ap_lincons0_array_t ap_csts =
        ap_abstract0_to_lincons_array(manager(), this->_inv.get());
    for (unsigned i = 0; i < ap_csts.size;) {
      ap_lincons0_t& ap_cst = ap_csts.p[i];

      if (ap_cst.constyp == AP_CONS_EQMOD) {
        // ikos::LinearConstraint does not support modular equality
        ikos_assert(ap_cst.scalar != nullptr);

        LinearExpressionT expr =
            this->to_ikos_linear_expression(ap_cst.linexpr0);
        Number mod = apron::to_ikos_number< Number >(ap_cst.scalar, false);
        o << expr << " = 0 mod " << mod;
      } else {
        LinearConstraintT cst = this->to_ikos_linear_constraint(ap_cst);
        o << cst;
      }

      i++;
      if (i < ap_csts.size) {
        o << "; ";
      }
    }

    ap_lincons0_array_clear(&ap_csts);
    o << "}";
#else
    // Only for debugging purpose
    // This is less generic since it needs a FILE* (here, we use stdout)
    o << "(";
    this->_var_map.dump(o);
    o << ",{\n" << std::flush;

    fflush(stdout);
    ap_abstract0_fprint(stdout, manager(), this->_inv.get(), nullptr);
    fflush(stdout);

    o << "})";
#endif
  }

  static std::string name() {
    return apron::domain_name(Domain);
  }

}; // end class ApronDomain

} // end namespace numeric
} // end namespace core
} // end namespace ikos
