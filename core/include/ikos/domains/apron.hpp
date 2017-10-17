/**************************************************************************/ /**
 *
 * \file
 * \brief Wrapper for the APRON library.
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2017 United States Government as represented by the
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

#ifndef IKOS_APRON_HPP
#define IKOS_APRON_HPP

#include <algorithm>
#include <memory>
#include <vector>

#include "ap_global0.h"
#include "ap_pkgrid.h"
#include "ap_ppl.h"
#include "box.h"
#include "oct.h"
#include "pk.h"
#include "pkeq.h"

#include <ikos/algorithms/linear_constraints.hpp>
#include <ikos/common/types.hpp>
#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/domains/bitwise_operators_api.hpp>
#include <ikos/domains/division_operators_api.hpp>
#include <ikos/domains/numerical_domains_api.hpp>
#include <ikos/number.hpp>
#include <ikos/value/congruence.hpp>
#include <ikos/value/interval.hpp>

namespace ikos {

namespace apron {

/// \brief Wrapper for ap_abstract0_t*
typedef std::shared_ptr< ap_abstract0_t > inv_ptr_t;

/// \brief Deleter for inv_ptr_t
struct inv_deleter {
  void operator()(ap_abstract0_t* inv) {
    ap_abstract0_free(ap_abstract0_manager(inv), inv);
  }
};

/// \brief Create a inv_ptr_t from a ap_abstract0_t*
inline inv_ptr_t inv_ptr(ap_abstract0_t* inv) {
  return std::shared_ptr< ap_abstract0_t >(inv, inv_deleter());
}

/// \returns the size of a ap_abstract0_t
inline std::size_t dims(ap_abstract0_t* inv) {
  return _ap_abstract0_dimension(inv).intdim;
}

/// \brief Add some dimensions to a ap_abstract0_t
inv_ptr_t add_dimensions(ap_abstract0_t* inv, std::size_t dims) {
  assert(dims > 0);

  ap_dimchange_t* dimchange = ap_dimchange_alloc(dims, 0);
  for (std::size_t i = 0; i < dims; i++) {
    dimchange->dim[i] = apron::dims(inv); // add dimension at the end
  }

  ap_manager_t* manager = ap_abstract0_manager(inv);
  inv_ptr_t r = inv_ptr(
      ap_abstract0_add_dimensions(manager, false, inv, dimchange, false));
  ap_dimchange_free(dimchange);
  return r;
}

/// \brief Remove some dimensions of a ap_abstract0_t
inv_ptr_t remove_dimensions(ap_abstract0_t* inv,
                            const std::vector< ap_dim_t >& dims) {
  assert(!dims.empty());
  assert(std::is_sorted(dims.begin(), dims.end()));

  // make sure that the removing dimensions are in ascending order

  ap_dimchange_t* dimchange = ap_dimchange_alloc(dims.size(), 0);
  for (std::size_t i = 0; i < dims.size(); i++) {
    // remove dimension dims[i] and shift to the left all the dimensions greater
    // than dims[i]
    dimchange->dim[i] = dims[i];
  }

  ap_manager_t* manager = ap_abstract0_manager(inv);
  inv_ptr_t r =
      inv_ptr(ap_abstract0_remove_dimensions(manager, false, inv, dimchange));
  ap_dimchange_free(dimchange);
  return r;
}

/// \brief Create a binary expression
inline ap_texpr0_t* binop_expr(ap_texpr_op_t op,
                               ap_texpr0_t* l,
                               ap_texpr0_t* r) {
  return ap_texpr0_binop(op, l, r, AP_RTYPE_INT, AP_RDIR_NEAREST);
}

/// \brief Conversion from ikos::z_number/q_number to ap_texpr0_t*
template < typename Number >
ap_texpr0_t* to_ap_expr(Number);

template <>
ap_texpr0_t* to_ap_expr(z_number n) {
  mpq_class e(n.mpz());
  return ap_texpr0_cst_scalar_mpq(e.get_mpq_t());
}

template <>
ap_texpr0_t* to_ap_expr(q_number q) {
  mpq_class e(q.mpq());
  return ap_texpr0_cst_scalar_mpq(e.get_mpq_t());
}

/// \brief Conversion from ap_scalar_t* to ikos::z_number/q_number
template < typename Number >
Number to_ikos_number(ap_scalar_t*, bool round_upper);

template <>
z_number to_ikos_number(ap_scalar_t* scalar, bool round_upper) {
  assert(ap_scalar_infty(scalar) == 0);
  assert(scalar->discr == AP_SCALAR_MPQ);

  q_number q(mpq_class(scalar->val.mpq));
  if (round_upper) {
    return q.round_to_upper();
  } else {
    return q.round_to_lower();
  }
}

template <>
q_number to_ikos_number(ap_scalar_t* scalar, bool /*round_upper*/) {
  assert(ap_scalar_infty(scalar) == 0);
  assert(scalar->discr == AP_SCALAR_MPQ);

  return q_number(mpq_class(scalar->val.mpq));
}

/// \brief Conversion from ap_coeff_t* to ikos::z_number/q_number
template < typename Number >
Number to_ikos_number(ap_coeff_t* coeff, bool round_upper) {
  assert(coeff->discr == AP_COEFF_SCALAR);

  return to_ikos_number< Number >(coeff->val.scalar, round_upper);
}

/// \brief Conversion from ap_scalar_t* to ikos::bound< Number >
template < typename Number >
bound< Number > to_ikos_bound(ap_scalar_t* scalar, bool round_upper) {
  typedef bound< Number > bound_t;

  if (ap_scalar_infty(scalar) == -1) {
    return bound_t::minus_infinity();
  } else if (ap_scalar_infty(scalar) == 1) {
    return bound_t::plus_infinity();
  } else {
    return bound_t(to_ikos_number< Number >(scalar, round_upper));
  }
}

/// \brief Conversion from ap_interval_t* to ikos::interval< Number >
template < typename Number >
interval< Number > to_ikos_interval(ap_interval_t* intv) {
  typedef interval< Number > interval_t;

  if (ap_interval_is_top(intv)) {
    return interval_t::top();
  }

  if (ap_interval_is_bottom(intv)) {
    return interval_t::bottom();
  }

  return interval_t(to_ikos_bound< Number >(intv->inf, true),
                    to_ikos_bound< Number >(intv->sup, false));
}

/// \brief Available abstract domains
typedef enum {
  interval,
  octagon,
  polka_polyhedra,
  polka_linear_equalities,
  ppl_polyhedra,
  ppl_linear_congruences,
  pkgrid_polyhedra_lin_congruences,
} domain_t;

const char* domain_name(domain_t d) {
  switch (d) {
    case interval:
      return "APRON Intervals";
    case octagon:
      return "APRON Octagons";
    case polka_polyhedra:
      return "APRON NewPolka Convex Polyhedra";
    case polka_linear_equalities:
      return "APRON NewPolka Linear Equalities";
    case ppl_polyhedra:
      return "APRON PPL Convex Polyhedra";
    case ppl_linear_congruences:
      return "APRON PPL Linear Congruences";
    case pkgrid_polyhedra_lin_congruences:
      return "APRON Reduced Product of NewPolka Convex Polyhedra and PPL "
             "Linear Congruences";
    default:
      ikos_unreachable("unexpected domain");
  }
}

ap_manager_t* alloc_domain_manager(domain_t d) {
  switch (d) {
    case interval:
      return box_manager_alloc();
    case octagon:
      return oct_manager_alloc();
    case polka_polyhedra:
      return pk_manager_alloc(false);
    case polka_linear_equalities:
      return pkeq_manager_alloc();
    case ppl_polyhedra:
      return ap_ppl_poly_manager_alloc(false);
    case ppl_linear_congruences:
      return ap_ppl_grid_manager_alloc();
    case pkgrid_polyhedra_lin_congruences:
      return ap_pkgrid_manager_alloc(pk_manager_alloc(false),
                                     ap_ppl_grid_manager_alloc());
    default:
      ikos_unreachable("unexpected domain");
  }
}

ap_abstract0_t* domain_narrowing(domain_t d,
                                 ap_manager_t* manager,
                                 ap_abstract0_t* a,
                                 ap_abstract0_t* b) {
  if (d == octagon) {
    return ap_abstract0_oct_narrowing(manager, a, b);
  } else {
    // by default, use meet
    return ap_abstract0_meet(manager, false, a, b);
  }
}

} // end namespace apron

/// \brief Wrapper for APRON abstract domains
template < apron::domain_t ApronDomain, typename Number, typename VariableName >
class apron_domain : public abstract_domain,
                     public numerical_domain< Number, VariableName >,
                     public bitwise_operators< Number, VariableName >,
                     public division_operators< Number, VariableName > {
public:
  typedef bound< Number > bound_t;
  typedef interval< Number > interval_t;
  typedef congruence< Number > congruence_t;
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;
  typedef apron_domain< ApronDomain, Number, VariableName > apron_domain_t;

private:
  typedef patricia_tree< VariableName, ap_dim_t > variable_map_t;

private:
  apron::inv_ptr_t _inv;
  variable_map_t _var_map;

private:
  static ap_manager_t* manager() {
    static ap_manager_t* man = nullptr;

    if (!man) {
      man = apron::alloc_domain_manager(ApronDomain);
    }

    return man;
  }

private:
  /// \brief Get the dimension associated to a variable
  inline boost::optional< ap_dim_t > var_dim(VariableName v) const {
    return _var_map.lookup(v);
  }

  /// \brief Get the dimension associated to a variable, or create one
  inline ap_dim_t var_dim_insert(VariableName v) {
    boost::optional< ap_dim_t > dim = _var_map.lookup(v);

    if (dim) {
      return *dim;
    } else {
      ap_dim_t new_dim = _var_map.size();
      _inv = apron::add_dimensions(&*_inv, 1);
      _var_map.insert(v, new_dim);
      assert(_var_map.size() == apron::dims(&*_inv));
      return new_dim;
    }
  }

  /// \brief Unary operator for variable_map_t that removes a dimension
  class remove_dim_op : public variable_map_t::unary_op_t {
  private:
    ap_dim_t _dim; // dimension to be removed

  public:
    remove_dim_op(ap_dim_t dim) : _dim(dim) {}

    boost::optional< ap_dim_t > apply(ap_dim_t dim) {
      if (dim < _dim) {
        return boost::optional< ap_dim_t >(dim);
      } else if (dim == _dim) {
        return boost::optional< ap_dim_t >();
      } else { // dim > _dim
        return boost::optional< ap_dim_t >(dim - 1);
      }
    }
  };

  /// \brief Unary operator for variable_map_t that removes a list of dimensions
  class remove_dims_op : public variable_map_t::unary_op_t {
  private:
    const std::vector< ap_dim_t >& _dims; // dimensions to be removed

  public:
    remove_dims_op(const std::vector< ap_dim_t >& dims) : _dims(dims) {
      assert(std::is_sorted(_dims.begin(), _dims.end()));
    }

    boost::optional< ap_dim_t > apply(ap_dim_t dim) {
      std::vector< ap_dim_t >::const_iterator it =
          std::lower_bound(_dims.begin(), _dims.end(), dim);

      if (it == _dims.end()) {
        return boost::optional< ap_dim_t >(dim - _dims.size());
      }

      if (*it == dim) {
        return boost::optional< ap_dim_t >();
      }

      if (it == _dims.begin()) {
        return boost::optional< ap_dim_t >(dim);
      }

      return boost::optional< ap_dim_t >(dim -
                                         std::distance(_dims.begin(), it));
    }
  };

  /// \brief Merge two variable maps, updating the associated abstract values
  static variable_map_t merge_var_maps(const variable_map_t& var_map_x,
                                       apron::inv_ptr_t& inv_x,
                                       const variable_map_t& var_map_y,
                                       apron::inv_ptr_t& inv_y) {
    assert(var_map_x.size() == apron::dims(&*inv_x));
    assert(var_map_y.size() == apron::dims(&*inv_y));

    // build a result variable map, based on var_map_x
    variable_map_t result_var_map(var_map_x);

    // (optimization) check if the variable mappings are equal
    bool equal = (var_map_x.size() == var_map_y.size());

    // add variables from var_map_y to the result variable map
    for (typename variable_map_t::iterator it = var_map_y.begin();
         it != var_map_y.end();
         ++it) {
      const VariableName& v_y = it->first;
      const ap_dim_t& dim_y = it->second;
      boost::optional< ap_dim_t > dim_x = var_map_x.lookup(v_y);
      equal = equal && dim_x && *dim_x == dim_y;

      if (!dim_x) {
        ap_dim_t dim = result_var_map.size();
        result_var_map.insert(v_y, dim);
      }
    }

    if (equal) { // var_map_x == var_map_y
      return result_var_map;
    }

    // add the necessary dimensions to inv_x and inv_y
    if (result_var_map.size() > var_map_x.size()) {
      inv_x = apron::add_dimensions(&*inv_x,
                                    result_var_map.size() - var_map_x.size());
    }
    if (result_var_map.size() > var_map_y.size()) {
      inv_y = apron::add_dimensions(&*inv_y,
                                    result_var_map.size() - var_map_y.size());
    }

    assert(result_var_map.size() == apron::dims(&*inv_x));
    assert(result_var_map.size() == apron::dims(&*inv_y));

    // build and apply the permutation map for inv_y
    ap_dimperm_t* perm_y = build_perm_map(var_map_y, result_var_map);
    inv_y = apron::inv_ptr(
        ap_abstract0_permute_dimensions(manager(), false, &*inv_y, perm_y));
    ap_dimperm_free(perm_y);

    assert(result_var_map.size() == apron::dims(&*inv_x));
    assert(result_var_map.size() == apron::dims(&*inv_y));

    return result_var_map;
  }

  static ap_dimperm_t* build_perm_map(const variable_map_t& old_map,
                                      const variable_map_t& new_map) {
    std::size_t n = new_map.size();
    ap_dimperm_t* perm = ap_dimperm_alloc(n);
    std::vector< bool > index_assigned(n, false);
    std::vector< bool > value_assigned(n, false);

    for (typename variable_map_t::iterator it = old_map.begin();
         it != old_map.end();
         ++it) {
      boost::optional< ap_dim_t > dim = new_map.lookup(it->first);
      assert(dim);

      perm->dim[it->second] = *dim;
      index_assigned[it->second] = true;
      value_assigned[*dim] = true;
    }

    ap_dim_t counter = 0;
    for (ap_dim_t i = 0; i < n; i++) {
      if (index_assigned[i])
        continue;

      while (value_assigned[counter])
        counter++;

      perm->dim[i] = counter;
      counter++;
    }

    return perm;
  }

  /// \brief Conversion from VariableName to ap_texpr0_t*
  ap_texpr0_t* to_ap_expr(VariableName v) {
    return ap_texpr0_dim(var_dim_insert(v));
  }

  /// \brief Conversion from linear_expression_t to ap_texpr0_t*
  ap_texpr0_t* to_ap_expr(linear_expression_t e) {
    ap_texpr0_t* r = apron::to_ap_expr(e.constant());

    for (typename linear_expression_t::iterator it = e.begin(); it != e.end();
         ++it) {
      ap_texpr0_t* term = apron::binop_expr(AP_TEXPR_MUL,
                                            apron::to_ap_expr(it->first),
                                            to_ap_expr(it->second.name()));
      r = apron::binop_expr(AP_TEXPR_ADD, r, term);
    }

    return r;
  }

  /// \brief Conversion from linear_constraint_t to ap_tcons0_t
  ap_tcons0_t to_ap_constraint(linear_constraint_t cst) {
    linear_expression_t exp = cst.expression();

    if (cst.is_equality()) {
      return ap_tcons0_make(AP_CONS_EQ, to_ap_expr(exp), nullptr);
    } else if (cst.is_inequality()) {
      return ap_tcons0_make(AP_CONS_SUPEQ, to_ap_expr(-exp), nullptr);
    } else {
      return ap_tcons0_make(AP_CONS_DISEQ, to_ap_expr(exp), nullptr);
    }
  }

  /// \brief Conversion from ap_linexpr0_t* to linear_expression_t
  linear_expression_t to_ikos_linear_expression(ap_linexpr0_t* expr) const {
    assert(ap_linexpr0_is_linear(expr));

    ap_coeff_t* coeff = ap_linexpr0_cstref(expr);
    linear_expression_t e(apron::to_ikos_number< Number >(coeff, false));

    for (typename variable_map_t::iterator it = _var_map.begin();
         it != _var_map.end();
         ++it) {
      coeff = ap_linexpr0_coeffref(expr, it->second);

      if (ap_coeff_zero(coeff))
        continue;

      e = e + (variable_t(it->first) *
               apron::to_ikos_number< Number >(coeff, false));
    }

    return e;
  }

  /// \brief Conversion from ap_lincons0_t to linear_constraint_t
  linear_constraint_t to_ikos_linear_constraint(
      const ap_lincons0_t& cst) const {
    assert(cst.constyp != AP_CONS_EQMOD);
    assert(cst.scalar == nullptr);

    linear_expression_t e = to_ikos_linear_expression(cst.linexpr0);

    switch (cst.constyp) {
      case AP_CONS_EQ:
        return linear_constraint_t(e, linear_constraint_t::kind_t::EQUALITY);
      case AP_CONS_SUPEQ:
        return linear_constraint_t(-e, linear_constraint_t::kind_t::INEQUALITY);
      case AP_CONS_SUP:
        return linear_constraint_t(-e + 1,
                                   linear_constraint_t::kind_t::INEQUALITY);
      case AP_CONS_DISEQ:
        return linear_constraint_t(e, linear_constraint_t::kind_t::DISEQUATION);
      default:
        ikos_unreachable("unexpected linear constraint");
    }
  }

private:
  apron_domain(apron::inv_ptr_t inv, variable_map_t var_map)
      : _inv(inv), _var_map(var_map) {}

  apron_domain(bool top)
      : _inv(apron::inv_ptr(top ? ap_abstract0_top(manager(), 0, 0)
                                : ap_abstract0_bottom(manager(), 0, 0))),
        _var_map() {}

public:
  static apron_domain_t top() { return apron_domain_t(true); }
  static apron_domain_t bottom() { return apron_domain_t(false); }

  apron_domain()
      : _inv(apron::inv_ptr(ap_abstract0_top(manager(), 0, 0))), _var_map() {}

  apron_domain(const apron_domain_t& o) : _inv(o._inv), _var_map(o._var_map) {}

  apron_domain& operator=(const apron_domain_t& o) {
    _inv = o._inv;
    _var_map = o._var_map;
    return *this;
  }

  bool is_bottom() { return ap_abstract0_is_bottom(manager(), &*_inv); }
  bool is_top() { return ap_abstract0_is_top(manager(), &*_inv); }

  bool operator<=(apron_domain_t o) {
    if (is_bottom()) {
      return true;
    } else if (o.is_bottom()) {
      return false;
    } else if (o.is_top()) {
      return true;
    } else if (is_top()) {
      return false;
    } else {
      apron::inv_ptr_t inv_x(_inv);
      apron::inv_ptr_t inv_y(o._inv);
      merge_var_maps(_var_map, inv_x, o._var_map, inv_y);
      return ap_abstract0_is_leq(manager(), &*inv_x, &*inv_y);
    }
  }

  bool operator==(apron_domain_t o) {
    if (is_bottom()) {
      return o.is_bottom();
    } else if (o.is_bottom()) {
      return false;
    } else if (is_top()) {
      return o.is_top();
    } else if (o.is_top()) {
      return false;
    } else {
      apron::inv_ptr_t inv_x(_inv);
      apron::inv_ptr_t inv_y(o._inv);
      merge_var_maps(_var_map, inv_x, o._var_map, inv_y);
      return ap_abstract0_is_eq(manager(), &*inv_x, &*inv_y);
    }
  }

  apron_domain_t operator|(apron_domain_t o) {
    if (is_bottom() || o.is_top()) {
      return o;
    } else if (is_top() || o.is_bottom()) {
      return *this;
    } else {
      apron::inv_ptr_t inv_x(_inv);
      apron::inv_ptr_t inv_y(o._inv);
      variable_map_t var_map =
          merge_var_maps(_var_map, inv_x, o._var_map, inv_y);
      apron::inv_ptr_t inv =
          apron::inv_ptr(ap_abstract0_join(manager(), false, &*inv_x, &*inv_y));
      return apron_domain_t(inv, var_map);
    }
  }

  apron_domain_t operator&(apron_domain_t o) {
    if (is_bottom() || o.is_bottom()) {
      return bottom();
    } else if (is_top()) {
      return o;
    } else if (o.is_top()) {
      return *this;
    } else {
      apron::inv_ptr_t inv_x(_inv);
      apron::inv_ptr_t inv_y(o._inv);
      variable_map_t var_map =
          merge_var_maps(_var_map, inv_x, o._var_map, inv_y);
      apron::inv_ptr_t inv =
          apron::inv_ptr(ap_abstract0_meet(manager(), false, &*inv_x, &*inv_y));
      return apron_domain_t(inv, var_map);
    }
  }

  apron_domain_t operator||(apron_domain_t o) {
    if (is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      apron::inv_ptr_t inv_x(_inv);
      apron::inv_ptr_t inv_y(o._inv);
      variable_map_t var_map =
          merge_var_maps(_var_map, inv_x, o._var_map, inv_y);
      apron::inv_ptr_t inv =
          apron::inv_ptr(ap_abstract0_widening(manager(), &*inv_x, &*inv_y));
      return apron_domain_t(inv, var_map);
    }
  }

  apron_domain_t operator&&(apron_domain_t o) {
    if (is_bottom() || o.is_bottom()) {
      return bottom();
    } else if (is_top()) {
      return o;
    } else if (o.is_top()) {
      return *this;
    } else {
      apron::inv_ptr_t inv_x(_inv);
      apron::inv_ptr_t inv_y(o._inv);
      variable_map_t var_map =
          merge_var_maps(_var_map, inv_x, o._var_map, inv_y);
      apron::inv_ptr_t inv = apron::inv_ptr(
          apron::domain_narrowing(ApronDomain, manager(), &*inv_x, &*inv_y));
      return apron_domain_t(inv, var_map);
    }
  }

  apron_domain_t join_loop(apron_domain_t o) { return this->operator|(o); }

  apron_domain_t join_iter(apron_domain_t o) { return this->operator|(o); }

  void set(VariableName v, interval_t intv) {
    if (is_bottom())
      return;

    if (intv.is_bottom()) {
      *this = bottom();
      return;
    }

    this->operator-=(v);
    this->operator+=(within_interval(v, intv));
  }

  interval_t operator[](VariableName v) {
    if (is_bottom()) {
      return interval_t::bottom();
    }

    boost::optional< ap_dim_t > dim = var_dim(v);

    if (dim) {
      ap_interval_t* intv =
          ap_abstract0_bound_dimension(manager(), &*_inv, *dim);
      interval_t r = apron::to_ikos_interval< Number >(intv);
      ap_interval_free(intv);
      return r;
    } else {
      return interval_t::top();
    }
  }

  congruence_t to_congruence(VariableName v) {
    if (is_bottom()) {
      return congruence_t::bottom();
    }

    return congruence_t::top();
  }

  void operator-=(VariableName v) { forget_num(v); }

  void forget(VariableName v) { forget_num(v); }

  void forget_num(VariableName v) {
    boost::optional< ap_dim_t > dim = var_dim(v);

    if (dim) {
      std::vector< ap_dim_t > vector_dims(1, *dim);
      _inv = apron::inv_ptr(ap_abstract0_forget_array(manager(),
                                                      false,
                                                      &*_inv,
                                                      &vector_dims[0],
                                                      vector_dims.size(),
                                                      false));
      _inv = apron::remove_dimensions(&*_inv, vector_dims);
      remove_dim_op op(*dim);
      _var_map.transform(op);
      assert(_var_map.size() == apron::dims(&*_inv));
    }
  }

  template < typename Iterator >
  void forget(Iterator begin, Iterator end) {
    forget_num(begin, end);
  }

  template < typename Iterator >
  void forget_num(Iterator begin, Iterator end) {
    std::vector< ap_dim_t > vector_dims;

    // gather all dimensions
    for (auto it = begin; it != end; ++it) {
      boost::optional< ap_dim_t > dim = var_dim(*it);

      if (dim) {
        vector_dims.push_back(*dim);
      }
    }

    if (vector_dims.empty())
      return;

    // sort the dimensions into ascending order
    std::sort(vector_dims.begin(), vector_dims.end());

    // forget and remove the dimensions
    _inv = apron::inv_ptr(ap_abstract0_forget_array(manager(),
                                                    false,
                                                    &*_inv,
                                                    &vector_dims[0],
                                                    vector_dims.size(),
                                                    false));
    _inv = apron::remove_dimensions(&*_inv, vector_dims);

    // update the variable mapping
    remove_dims_op op(vector_dims);
    _var_map.transform(op);

    assert(_var_map.size() == apron::dims(&*_inv));
  }

  void operator+=(linear_constraint_system_t csts) {
    if (csts.size() == 0)
      return;

    if (is_bottom())
      return;

    ap_tcons0_array_t ap_csts = ap_tcons0_array_make(csts.size());

    unsigned int i = 0;
    for (typename linear_constraint_system_t::iterator it = csts.begin();
         it != csts.end();
         ++it) {
      ap_csts.p[i++] = to_ap_constraint(*it);
    }

    _inv = apron::inv_ptr(
        ap_abstract0_meet_tcons_array(manager(), false, &*_inv, &ap_csts));

    // this step allows to improve the precision
    for (i = 0; i < csts.size() && !is_bottom(); i++) {
      // check satisfiability of ap_csts.p[i]
      ap_tcons0_t& cst = ap_csts.p[i];
      ap_interval_t* ap_intv =
          ap_abstract0_bound_texpr(manager(), &*_inv, cst.texpr0);
      interval_t intv = apron::to_ikos_interval< Number >(ap_intv);

      if (intv.is_bottom() || (cst.constyp == AP_CONS_EQ && !intv[0]) ||
          (cst.constyp == AP_CONS_SUPEQ && intv.ub() < bound_t(0)) ||
          (cst.constyp == AP_CONS_DISEQ && intv == interval_t(0, 0))) {
        // cst is not satisfiable
        *this = bottom();
      }

      ap_interval_free(ap_intv);
    }

    ap_tcons0_array_clear(&ap_csts);
  }

  void assign(VariableName v, linear_expression_t e) {
    if (is_bottom())
      return;

    ap_texpr0_t* t = to_ap_expr(e);
    ap_dim_t v_dim = var_dim_insert(v);
    _inv = apron::inv_ptr(
        ap_abstract0_assign_texpr(manager(), false, &*_inv, v_dim, t, nullptr));
    ap_texpr0_free(t);
  }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    if (is_bottom())
      return;

    ap_texpr0_t* left = to_ap_expr(y);
    ap_texpr0_t* right = to_ap_expr(z);
    ap_texpr0_t* t;

    switch (op) {
      case OP_ADDITION: {
        t = apron::binop_expr(AP_TEXPR_ADD, left, right);
        break;
      }
      case OP_SUBTRACTION: {
        t = apron::binop_expr(AP_TEXPR_SUB, left, right);
        break;
      }
      case OP_MULTIPLICATION: {
        t = apron::binop_expr(AP_TEXPR_MUL, left, right);
        break;
      }
      case OP_DIVISION: {
        t = apron::binop_expr(AP_TEXPR_DIV, left, right);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }

    ap_dim_t x_dim = var_dim_insert(x);
    _inv = apron::inv_ptr(
        ap_abstract0_assign_texpr(manager(), false, &*_inv, x_dim, t, nullptr));
    ap_texpr0_free(t);
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    if (is_bottom())
      return;

    ap_texpr0_t* left = to_ap_expr(y);
    ap_texpr0_t* right = apron::to_ap_expr(k);
    ap_texpr0_t* t;

    switch (op) {
      case OP_ADDITION: {
        t = apron::binop_expr(AP_TEXPR_ADD, left, right);
        break;
      }
      case OP_SUBTRACTION: {
        t = apron::binop_expr(AP_TEXPR_SUB, left, right);
        break;
      }
      case OP_MULTIPLICATION: {
        t = apron::binop_expr(AP_TEXPR_MUL, left, right);
        break;
      }
      case OP_DIVISION: {
        t = apron::binop_expr(AP_TEXPR_DIV, left, right);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }

    ap_dim_t x_dim = var_dim_insert(x);
    _inv = apron::inv_ptr(
        ap_abstract0_assign_texpr(manager(), false, &*_inv, x_dim, t, nullptr));
    ap_texpr0_free(t);
  }

  void apply(conv_operation_t /*op*/,
             VariableName x,
             VariableName y,
             uint64_t /*from*/,
             uint64_t /*to*/) {
    // since reasoning about infinite precision we simply assign and ignore the
    // width
    // FIXME: unsound
    assign(x, linear_expression_t(y));
  }

  void apply(conv_operation_t /*op*/,
             VariableName x,
             Number k,
             uint64_t /*from*/,
             uint64_t /*to*/) {
    // since reasoning about infinite precision we simply assign and ignore the
    // width
    // FIXME: unsound
    assign(x, linear_expression_t(k));
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    if (is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = this->operator[](y);
    interval_t v_z = this->operator[](z);

    switch (op) {
      case OP_AND: {
        v_x = v_y.And(v_z);
        break;
      }
      case OP_OR: {
        v_x = v_y.Or(v_z);
        break;
      }
      case OP_XOR: {
        v_x = v_y.Xor(v_z);
        break;
      }
      case OP_SHL: {
        v_x = v_y.Shl(v_z);
        break;
      }
      case OP_LSHR: {
        v_x = v_y.LShr(v_z);
        break;
      }
      case OP_ASHR: {
        v_x = v_y.AShr(v_z);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }

    set(x, v_x);
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    if (is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = this->operator[](y);
    interval_t v_z(k);

    switch (op) {
      case OP_AND: {
        v_x = v_y.And(v_z);
        break;
      }
      case OP_OR: {
        v_x = v_y.Or(v_z);
        break;
      }
      case OP_XOR: {
        v_x = v_y.Xor(v_z);
        break;
      }
      case OP_SHL: {
        v_x = v_y.Shl(v_z);
        break;
      }
      case OP_LSHR: {
        v_x = v_y.LShr(v_z);
        break;
      }
      case OP_ASHR: {
        v_x = v_y.AShr(v_z);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }

    set(x, v_x);
  }

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    if (op == OP_SDIV) {
      apply(OP_DIVISION, x, y, z);
    } else {
      if (is_bottom())
        return;

      interval_t v_x = interval_t::top();
      interval_t v_y = this->operator[](y);
      interval_t v_z = this->operator[](z);

      switch (op) {
        case OP_UDIV: {
          v_x = v_y.UDiv(v_z);
          break;
        }
        case OP_SREM: {
          v_x = v_y.SRem(v_z);
          break;
        }
        case OP_UREM: {
          v_x = v_y.URem(v_z);
          break;
        }
        default: { ikos_unreachable("invalid operation"); }
      }

      set(x, v_x);
    }
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    if (op == OP_SDIV) {
      apply(OP_DIVISION, x, y, k);
    } else {
      if (is_bottom())
        return;

      interval_t v_x = interval_t::top();
      interval_t v_y = this->operator[](y);
      interval_t v_z(k);

      switch (op) {
        case OP_UDIV: {
          v_x = v_y.UDiv(v_z);
          break;
        }
        case OP_SREM: {
          v_x = v_y.SRem(v_z);
          break;
        }
        case OP_UREM: {
          v_x = v_y.URem(v_z);
          break;
        }
        default: { ikos_unreachable("invalid operation"); }
      }

      set(x, v_x);
    }
  }

  void normalize() { ap_abstract0_canonicalize(manager(), &*_inv); }

  linear_constraint_system_t to_linear_constraint_system() {
    normalize();
    linear_constraint_system_t csts;

    if (is_bottom()) {
      csts += linear_constraint_t::contradiction();
      return csts;
    }

    ap_lincons0_array_t ap_csts =
        ap_abstract0_to_lincons_array(manager(), &*_inv);
    for (unsigned int i = 0; i < ap_csts.size; i++) {
      ap_lincons0_t& ap_cst = ap_csts.p[i];

      if (ap_cst.constyp == AP_CONS_EQMOD) {
        // ikos::linear_constraint does not support modular equality
        continue;
      }

      csts += to_ikos_linear_constraint(ap_cst);
    }

    ap_lincons0_array_clear(&ap_csts);
    return csts;
  }

  void write(std::ostream& o) {
    normalize();

    if (is_bottom()) {
      o << "_|_";
    } else {
#if 1
      o << "{";

      ap_lincons0_array_t ap_csts =
          ap_abstract0_to_lincons_array(manager(), &*_inv);
      for (unsigned int i = 0; i < ap_csts.size;) {
        ap_lincons0_t& ap_cst = ap_csts.p[i];

        if (ap_cst.constyp == AP_CONS_EQMOD) {
          // ikos::linear_constraint does not support modular equality
          assert(ap_cst.scalar != nullptr);

          linear_expression_t expr = to_ikos_linear_expression(ap_cst.linexpr0);
          Number mod = apron::to_ikos_number< Number >(ap_cst.scalar, false);
          o << expr << " = 0 mod " << mod;
        } else {
          linear_constraint_t cst = to_ikos_linear_constraint(ap_cst);
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
      o << "({";
      for (typename variable_map_t::iterator it = _var_map.begin();
           it != _var_map.end();) {
        VariableName key = it->first;
        index_traits::write< Key >(o, key);
        o << " -> " << it->second;
        ++it;
        if (it != _var_map.end()) {
          o << "; ";
        }
      }
      o << "},{" << std::endl;

      fflush(stdout);
      ap_abstract0_fprint(stdout, manager(), &*_inv, nullptr);
      fflush(stdout);

      o << "})";
#endif
    }
  }

  static std::string domain_name() { return apron::domain_name(ApronDomain); }

}; // end class apron_domain

namespace num_domain_traits {
namespace detail {

template < apron::domain_t ApronDomain, typename Number, typename VariableName >
struct normalize_impl< apron_domain< ApronDomain, Number, VariableName > > {
  inline void operator()(
      apron_domain< ApronDomain, Number, VariableName >& inv) {
    inv.normalize();
  }
};

} // end namespace detail
} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_APRON_HPP
