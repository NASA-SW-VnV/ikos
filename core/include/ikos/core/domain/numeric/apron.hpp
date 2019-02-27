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

/// \brief Wrapper for ap_abstract0_t*
using InvPtr = std::shared_ptr< ap_abstract0_t >;

/// \brief Deleter for InvPtr
struct InvDeleter {
  void operator()(ap_abstract0_t* inv) {
    ap_abstract0_free(ap_abstract0_manager(inv), inv);
  }
};

/// \brief Create a InvPtr from a ap_abstract0_t*
inline InvPtr inv_ptr(ap_abstract0_t* inv) {
  return std::shared_ptr< ap_abstract0_t >(inv, InvDeleter());
}

/// \returns the size of a ap_abstract0_t
inline std::size_t dims(ap_abstract0_t* inv) {
  return ap_abstract0_dimension(ap_abstract0_manager(inv), inv).intdim;
}

/// \brief Add some dimensions to a ap_abstract0_t
inline InvPtr add_dimensions(ap_abstract0_t* inv, std::size_t dims) {
  ikos_assert(dims > 0);

  ap_dimchange_t* dimchange = ap_dimchange_alloc(dims, 0);
  for (std::size_t i = 0; i < dims; i++) {
    // add dimension at the end
    dimchange->dim[i] = static_cast< ap_dim_t >(apron::dims(inv));
  }

  ap_manager_t* manager = ap_abstract0_manager(inv);
  InvPtr r = inv_ptr(
      ap_abstract0_add_dimensions(manager, false, inv, dimchange, false));
  ap_dimchange_free(dimchange);
  return r;
}

/// \brief Remove some dimensions of a ap_abstract0_t
inline InvPtr remove_dimensions(ap_abstract0_t* inv,
                                const std::vector< ap_dim_t >& dims) {
  ikos_assert(!dims.empty());
  ikos_assert(std::is_sorted(dims.begin(), dims.end()));

  // make sure that the removing dimensions are in ascending order

  ap_dimchange_t* dimchange = ap_dimchange_alloc(dims.size(), 0);
  for (std::size_t i = 0; i < dims.size(); i++) {
    // remove dimension dims[i] and shift to the left all the dimensions greater
    // than dims[i]
    dimchange->dim[i] = dims[i];
  }

  ap_manager_t* manager = ap_abstract0_manager(inv);
  InvPtr r =
      inv_ptr(ap_abstract0_remove_dimensions(manager, false, inv, dimchange));
  ap_dimchange_free(dimchange);
  return r;
}

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

inline ap_abstract0_t* domain_narrowing(Domain d,
                                        ap_manager_t* manager,
                                        ap_abstract0_t* a,
                                        ap_abstract0_t* b) {
  if (d == Octagon) {
    return ap_abstract0_oct_narrowing(manager, a, b);
  } else {
    // by default, use meet
    return ap_abstract0_meet(manager, false, a, b);
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

private:
  apron::InvPtr _inv;
  VariableMap _var_map;

private:
  /// \brief Get the manager for the given apron domain
  static ap_manager_t* manager() {
    // Initialized at first call
    static ap_manager_t* man = apron::alloc_domain_manager(Domain);
    return man;
  }

  /// \brief Get the dimension associated to a variable
  boost::optional< const ap_dim_t& > var_dim(VariableRef v) const {
    return this->_var_map.at(v);
  }

  /// \brief Get the dimension associated to a variable, or create one
  ap_dim_t var_dim_insert(VariableRef v) {
    boost::optional< const ap_dim_t& > dim = var_dim(v);

    if (dim) {
      return *dim;
    } else {
      auto new_dim = static_cast< ap_dim_t >(this->_var_map.size());
      this->_inv = apron::add_dimensions(this->_inv.get(), 1);
      this->_var_map.insert_or_assign(v, new_dim);
      ikos_assert(this->_var_map.size() == apron::dims(this->_inv.get()));
      return new_dim;
    }
  }

  /// \brief Merge two variable maps, updating the associated abstract values
  static VariableMap merge_var_maps(const VariableMap& var_map_x,
                                    apron::InvPtr& inv_x,
                                    const VariableMap& var_map_y,
                                    apron::InvPtr& inv_y) {
    ikos_assert(var_map_x.size() == apron::dims(inv_x.get()));
    ikos_assert(var_map_y.size() == apron::dims(inv_y.get()));

    // build a result variable map, based on var_map_x
    VariableMap result_var_map(var_map_x);

    // (optimization) check if the variable mappings are equal
    bool equal = (var_map_x.size() == var_map_y.size());

    // add variables from var_map_y to the result variable map
    for (auto it = var_map_y.begin(); it != var_map_y.end(); ++it) {
      const VariableRef& v_y = it->first;
      const ap_dim_t& dim_y = it->second;
      boost::optional< const ap_dim_t& > dim_x = var_map_x.at(v_y);
      equal = equal && dim_x && *dim_x == dim_y;

      if (!dim_x) {
        auto dim = static_cast< ap_dim_t >(result_var_map.size());
        result_var_map.insert_or_assign(v_y, dim);
      }
    }

    if (equal) { // var_map_x == var_map_y
      return result_var_map;
    }

    // add the necessary dimensions to inv_x and inv_y
    if (result_var_map.size() > var_map_x.size()) {
      inv_x = apron::add_dimensions(inv_x.get(),
                                    result_var_map.size() - var_map_x.size());
    }
    if (result_var_map.size() > var_map_y.size()) {
      inv_y = apron::add_dimensions(inv_y.get(),
                                    result_var_map.size() - var_map_y.size());
    }

    ikos_assert(result_var_map.size() == apron::dims(inv_x.get()));
    ikos_assert(result_var_map.size() == apron::dims(inv_y.get()));

    // build and apply the permutation map for inv_y
    ap_dimperm_t* perm_y = build_perm_map(var_map_y, result_var_map);
    inv_y = apron::inv_ptr(
        ap_abstract0_permute_dimensions(manager(), false, inv_y.get(), perm_y));
    ap_dimperm_free(perm_y);

    ikos_assert(result_var_map.size() == apron::dims(inv_x.get()));
    ikos_assert(result_var_map.size() == apron::dims(inv_y.get()));

    return result_var_map;
  }

  static ap_dimperm_t* build_perm_map(const VariableMap& old_map,
                                      const VariableMap& new_map) {
    std::size_t n = new_map.size();
    ap_dimperm_t* perm = ap_dimperm_alloc(n);
    std::vector< bool > index_assigned(n, false);
    std::vector< bool > value_assigned(n, false);

    for (auto it = old_map.begin(); it != old_map.end(); ++it) {
      boost::optional< const ap_dim_t& > dim = new_map.at(it->first);
      ikos_assert(dim);

      perm->dim[it->second] = *dim;
      index_assigned[it->second] = true;
      value_assigned[*dim] = true;
    }

    ap_dim_t counter = 0;
    for (ap_dim_t i = 0; i < n; i++) {
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

  /// \brief Conversion from VariableRef to ap_texpr0_t*
  ap_texpr0_t* to_ap_expr(VariableRef v) {
    return ap_texpr0_dim(var_dim_insert(v));
  }

  /// \brief Conversion from LinearExpression to ap_texpr0_t*
  ap_texpr0_t* to_ap_expr(const LinearExpressionT& e) {
    ap_texpr0_t* r = apron::to_ap_expr(e.constant());

    for (auto it = e.begin(), et = e.end(); it != et; ++it) {
      ap_texpr0_t* term =
          apron::binop_expr< Number >(AP_TEXPR_MUL,
                                      apron::to_ap_expr(it->second),
                                      to_ap_expr(it->first));
      r = apron::binop_expr< Number >(AP_TEXPR_ADD, r, term);
    }

    return r;
  }

  /// \brief Conversion from LinearConstraint to ap_tcons0_t
  ap_tcons0_t to_ap_constraint(const LinearConstraintT& cst) {
    const LinearExpressionT& exp = cst.expression();

    if (cst.is_equality()) {
      return ap_tcons0_make(AP_CONS_EQ, to_ap_expr(exp), nullptr);
    } else if (cst.is_inequality()) {
      return ap_tcons0_make(AP_CONS_SUPEQ, to_ap_expr(-exp), nullptr);
    } else {
      return ap_tcons0_make(AP_CONS_DISEQ, to_ap_expr(exp), nullptr);
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

    LinearExpressionT e = to_ikos_linear_expression(cst.linexpr0);

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

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Private constructor
  ApronDomain(apron::InvPtr inv, VariableMap var_map)
      : _inv(std::move(inv)), _var_map(std::move(var_map)) {}

  /// \brief Create the top abstract value
  explicit ApronDomain(TopTag)
      : _inv(apron::inv_ptr(ap_abstract0_top(manager(), 0, 0))) {}

  /// \brief Create the bottom abstract value
  explicit ApronDomain(BottomTag)
      : _inv(apron::inv_ptr(ap_abstract0_bottom(manager(), 0, 0))) {}

public:
  /// \brief Create the top abstract value
  ApronDomain() : ApronDomain(TopTag{}) {}

  /// \brief Copy constructor
  ApronDomain(const ApronDomain&) = default;

  /// \brief Move constructor
  ApronDomain(ApronDomain&&) = default;

  /// \brief Copy assignment operator
  ApronDomain& operator=(const ApronDomain&) = default;

  /// \brief Move assignment operator
  ApronDomain& operator=(ApronDomain&&) = default;

  /// \brief Destructor
  ~ApronDomain() override = default;

  /// \brief Create the top abstract value
  static ApronDomain top() { return ApronDomain(TopTag{}); }

  /// \brief Create the bottom abstract value
  static ApronDomain bottom() { return ApronDomain(BottomTag{}); }

  bool is_bottom() const override {
    return ap_abstract0_is_bottom(manager(), this->_inv.get());
  }

  bool is_top() const override {
    return ap_abstract0_is_top(manager(), this->_inv.get());
  }

  void set_to_bottom() override { this->operator=(bottom()); }

  void set_to_top() override { this->operator=(top()); }

  bool leq(const ApronDomain& other) const override {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else if (other.is_top()) {
      return true;
    } else if (this->is_top()) {
      return false;
    } else {
      apron::InvPtr inv_x(this->_inv);
      apron::InvPtr inv_y(other._inv);
      merge_var_maps(this->_var_map, inv_x, other._var_map, inv_y);
      return ap_abstract0_is_leq(manager(), inv_x.get(), inv_y.get());
    }
  }

  bool equals(const ApronDomain& other) const override {
    if (this->is_bottom()) {
      return other.is_bottom();
    } else if (other.is_bottom()) {
      return false;
    } else if (this->is_top()) {
      return other.is_top();
    } else if (other.is_top()) {
      return false;
    } else {
      apron::InvPtr inv_x(this->_inv);
      apron::InvPtr inv_y(other._inv);
      merge_var_maps(this->_var_map, inv_x, other._var_map, inv_y);
      return ap_abstract0_is_eq(manager(), inv_x.get(), inv_y.get());
    }
  }

  ApronDomain join(const ApronDomain& other) const override {
    if (this->is_bottom() || other.is_top()) {
      return other;
    } else if (this->is_top() || other.is_bottom()) {
      return *this;
    } else {
      apron::InvPtr inv_x(this->_inv);
      apron::InvPtr inv_y(other._inv);
      VariableMap var_map =
          merge_var_maps(this->_var_map, inv_x, other._var_map, inv_y);
      apron::InvPtr inv = apron::inv_ptr(
          ap_abstract0_join(manager(), false, inv_x.get(), inv_y.get()));
      return ApronDomain(inv, var_map);
    }
  }

  void join_with(const ApronDomain& other) override {
    this->operator=(this->join(other));
  }

  ApronDomain widening(const ApronDomain& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      apron::InvPtr inv_x(this->_inv);
      apron::InvPtr inv_y(other._inv);
      VariableMap var_map =
          merge_var_maps(this->_var_map, inv_x, other._var_map, inv_y);
      apron::InvPtr inv = apron::inv_ptr(
          ap_abstract0_widening(manager(), inv_x.get(), inv_y.get()));
      return ApronDomain(inv, var_map);
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
                            const Number& threshold) override {
    this->operator=(this->widening_threshold(other, threshold));
  }

  ApronDomain meet(const ApronDomain& other) const override {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else if (this->is_top()) {
      return other;
    } else if (other.is_top()) {
      return *this;
    } else {
      apron::InvPtr inv_x(this->_inv);
      apron::InvPtr inv_y(other._inv);
      VariableMap var_map =
          merge_var_maps(this->_var_map, inv_x, other._var_map, inv_y);
      apron::InvPtr inv = apron::inv_ptr(
          ap_abstract0_meet(manager(), false, inv_x.get(), inv_y.get()));
      return ApronDomain(inv, var_map);
    }
  }

  void meet_with(const ApronDomain& other) override {
    this->operator=(this->meet(other));
  }

  ApronDomain narrowing(const ApronDomain& other) const override {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else if (this->is_top()) {
      return other;
    } else if (other.is_top()) {
      return *this;
    } else {
      apron::InvPtr inv_x(this->_inv);
      apron::InvPtr inv_y(other._inv);
      VariableMap var_map =
          merge_var_maps(this->_var_map, inv_x, other._var_map, inv_y);
      apron::InvPtr inv = apron::inv_ptr(
          apron::domain_narrowing(Domain, manager(), inv_x.get(), inv_y.get()));
      return ApronDomain(inv, var_map);
    }
  }

  void narrow_with(const ApronDomain& other) override {
    this->operator=(this->narrowing(other));
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
    if (this->is_bottom()) {
      return;
    }

    ap_texpr0_t* t = to_ap_expr(e);
    ap_dim_t v_dim = var_dim_insert(x);
    this->_inv = apron::inv_ptr(ap_abstract0_assign_texpr(manager(),
                                                          false,
                                                          this->_inv.get(),
                                                          v_dim,
                                                          t,
                                                          nullptr));
    ap_texpr0_free(t);
  }

private:
  bool is_supported(BinaryOperator op) {
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

    ap_dim_t x_dim = var_dim_insert(x);
    this->_inv = apron::inv_ptr(ap_abstract0_assign_texpr(manager(),
                                                          false,
                                                          this->_inv.get(),
                                                          x_dim,
                                                          t,
                                                          nullptr));
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

    if (this->is_supported(op)) {
      this->apply(op, x, to_ap_expr(y), to_ap_expr(z));
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

    if (this->is_supported(op)) {
      this->apply(op, x, to_ap_expr(y), apron::to_ap_expr(z));
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

    if (this->is_supported(op)) {
      this->apply(op, x, apron::to_ap_expr(y), to_ap_expr(z));
    } else {
      this->set(x, apply_bin_operator(op, IntervalT(y), this->to_interval(z)));
    }
  }

  void add(const LinearConstraintT& cst) override {
    this->add(LinearConstraintSystemT{cst});
  }

  void add(const LinearConstraintSystemT& csts) override {
    if (csts.empty()) {
      return;
    }

    if (this->is_bottom()) {
      return;
    }

    ap_tcons0_array_t ap_csts = ap_tcons0_array_make(csts.size());

    std::size_t i = 0;
    for (const LinearConstraintT& cst : csts) {
      ap_csts.p[i++] = to_ap_constraint(cst);
    }

    this->_inv = apron::inv_ptr(ap_abstract0_meet_tcons_array(manager(),
                                                              false,
                                                              this->_inv.get(),
                                                              &ap_csts));

    // this step allows to improve the precision
    for (i = 0; i < csts.size() && !this->is_bottom(); i++) {
      // check satisfiability of ap_csts.p[i]
      ap_tcons0_t& cst = ap_csts.p[i];
      ap_interval_t* ap_intv =
          ap_abstract0_bound_texpr(manager(), this->_inv.get(), cst.texpr0);
      IntervalT intv = apron::to_ikos_interval< Number >(ap_intv);

      if (intv.is_bottom() ||
          (cst.constyp == AP_CONS_EQ && !intv.contains(0)) ||
          (cst.constyp == AP_CONS_SUPEQ && intv.ub() < BoundT(0)) ||
          (cst.constyp == AP_CONS_DISEQ && intv == IntervalT(0))) {
        // cst is not satisfiable
        this->set_to_bottom();
        return;
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
      ap_tcons0_array_t csts = ap_tcons0_array_make(1);
      csts.p[0] = ap_tcons0_make(AP_CONS_EQMOD,
                                 to_ap_expr(VariableExprT(x) - value.residue()),
                                 apron::to_ap_scalar(value.modulus()));
      this->_inv =
          apron::inv_ptr(ap_abstract0_meet_tcons_array(manager(),
                                                       false,
                                                       this->_inv.get(),
                                                       &csts));
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
    boost::optional< const ap_dim_t& > has_dim = var_dim(x);

    if (!has_dim) {
      return;
    }

    ap_dim_t dim = *has_dim;
    std::vector< ap_dim_t > vector_dims{dim};
    this->_inv = apron::inv_ptr(ap_abstract0_forget_array(manager(),
                                                          false,
                                                          this->_inv.get(),
                                                          &vector_dims[0],
                                                          vector_dims.size(),
                                                          false));
    this->_inv = apron::remove_dimensions(this->_inv.get(), vector_dims);
    this->_var_map.transform([dim](VariableRef, ap_dim_t d) {
      if (d < dim) {
        return boost::optional< ap_dim_t >(d);
      } else if (d == dim) {
        return boost::optional< ap_dim_t >(boost::none);
      } else { // d > dim
        return boost::optional< ap_dim_t >(d - 1);
      }
    });
    ikos_assert(this->_var_map.size() == apron::dims(this->_inv.get()));
  }

  void normalize() const override {
    ap_abstract0_canonicalize(manager(), this->_inv.get());
  }

  IntervalT to_interval(VariableRef x) const override {
    if (this->is_bottom()) {
      return IntervalT::bottom();
    }

    boost::optional< const ap_dim_t& > dim = var_dim(x);

    if (dim) {
      ap_interval_t* intv =
          ap_abstract0_bound_dimension(manager(), this->_inv.get(), *dim);
      IntervalT r = apron::to_ikos_interval< Number >(intv);
      ap_interval_free(intv);
      return r;
    } else {
      return IntervalT::top();
    }
  }

  IntervalT to_interval(const LinearExpressionT& e) const override {
    return Parent::to_interval(e);
  }

  CongruenceT to_congruence(VariableRef) const override {
    if (is_bottom()) {
      return CongruenceT::bottom();
    }

    return CongruenceT::top();
  }

  CongruenceT to_congruence(const LinearExpressionT& e) const override {
    return Parent::to_congruence(e);
  }

  IntervalCongruenceT to_interval_congruence(VariableRef x) const override {
    if (is_bottom()) {
      return IntervalCongruenceT::bottom();
    }

    return IntervalCongruenceT(this->to_interval(x), this->to_congruence(x));
  }

  IntervalCongruenceT to_interval_congruence(
      const LinearExpressionT& e) const override {
    if (is_bottom()) {
      return IntervalCongruenceT::bottom();
    }

    return IntervalCongruenceT(this->to_interval(e), this->to_congruence(e));
  }

  LinearConstraintSystemT to_linear_constraint_system() const override {
    this->normalize();

    if (this->is_bottom()) {
      return LinearConstraintSystemT(LinearConstraintT::contradiction());
    }

    LinearConstraintSystemT csts;
    ap_lincons0_array_t ap_csts =
        ap_abstract0_to_lincons_array(manager(), this->_inv.get());
    for (unsigned i = 0; i < ap_csts.size; i++) {
      ap_lincons0_t& ap_cst = ap_csts.p[i];

      if (ap_cst.constyp == AP_CONS_EQMOD) {
        // ikos::LinearConstraint does not support modular equality
        continue;
      }

      csts.add(to_ikos_linear_constraint(ap_cst));
    }

    ap_lincons0_array_clear(&ap_csts);
    return csts;
  }

  void dump(std::ostream& o) const override {
    this->normalize();

    if (this->is_bottom()) {
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

        LinearExpressionT expr = to_ikos_linear_expression(ap_cst.linexpr0);
        Number mod = apron::to_ikos_number< Number >(ap_cst.scalar, false);
        o << expr << " = 0 mod " << mod;
      } else {
        LinearConstraintT cst = to_ikos_linear_constraint(ap_cst);
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

  static std::string name() { return apron::domain_name(Domain); }

}; // end class ApronDomain

} // end namespace numeric
} // end namespace core
} // end namespace ikos
