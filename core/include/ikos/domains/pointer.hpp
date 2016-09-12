/*******************************************************************************
 *
 * Implementation of a pointer domain.
 *
 * Author: Maxime Arthaud
 *         Jorge A. Navas
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2016 United States Government as represented by the
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

#ifndef IKOS_POINTER_HPP
#define IKOS_POINTER_HPP

#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/domains/bitwise_operators_api.hpp>
#include <ikos/domains/division_operators_api.hpp>
#include <ikos/domains/numerical_domains_api.hpp>
#include <ikos/domains/pointer_domains_api.hpp>
#include <ikos/domains/separate_domains.hpp>

namespace ikos {

// The pointer domain can express integer scalar (modelled directly in
// ScalarDomain) and pointer variables. For each pointer p, it keeps
// track of its address as well as its offset. The offset of p is also
// modelled directly by the numerical abstraction ScalarDomain. The
// address of p is modelled by keeping track of all possible memory
// objects (e.g., &'s and malloc's) to which p may point to.
//
// Notes:
//
// - This domain can only model pointers to integer or floating point
//   variables and it does not model pointers with multiple levels of
//   indirection. However, unlike traditional pointer analysis this
//   domain is flow-sensitive.
//
// - It represents the points-to set (all possible addresses of a
//   pointer) using the discrete domain. Thus, we cannot distinguish
//   between bottom and the empty set. The empty set is useful since
//   it can tell us whether a variable is definitely null. One simple
//   solution is to have a special memory object denoting null (e.g.,
//   $null). For simplicity, we do not do it because we combine later
//   this domain with the nullity domain so that can still keep track
//   of that information.
template < typename ScalarDomain >
class pointer_domain_impl
    : public abstract_domain,
      public numerical_domain< typename ScalarDomain::number_t,
                               typename ScalarDomain::variable_name_t >,
      public bitwise_operators< typename ScalarDomain::number_t,
                                typename ScalarDomain::variable_name_t >,
      public division_operators< typename ScalarDomain::number_t,
                                 typename ScalarDomain::variable_name_t >,
      public pointer_domain< typename ScalarDomain::number_t,
                             typename ScalarDomain::variable_name_t > {
private:
  typedef typename ScalarDomain::number_t Number;
  typedef typename ScalarDomain::variable_name_t VariableName;

public:
  typedef Number number_t;
  typedef VariableName variable_name_t;
  typedef interval< Number > interval_t;
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;
  typedef Number offset_number_t;
  typedef discrete_domain< VariableName > points_to_set_t;
  typedef pointer_domain_impl< ScalarDomain > pointer_domain_t;

private:
  typedef separate_domain< VariableName, points_to_set_t > points_to_map_t;

private:
  bool _is_bottom;
  points_to_map_t _ptr_map; // map pointer variables to set of addresses
  ScalarDomain _inv;        // scalar and pointer offset variables

private:
  pointer_domain_impl(bool is_bottom)
      : _is_bottom(is_bottom),
        _ptr_map(is_bottom ? points_to_map_t::bottom()
                           : points_to_map_t::top()),
        _inv(is_bottom ? ScalarDomain::bottom() : ScalarDomain::top()) {}

  pointer_domain_impl(const points_to_map_t& ptr_map, const ScalarDomain& inv)
      : _is_bottom(false), _ptr_map(ptr_map), _inv(inv) {}

public:
  static pointer_domain_t top() { return pointer_domain_impl(false); }

  static pointer_domain_t bottom() { return pointer_domain_impl(true); }

public:
  pointer_domain_impl()
      : _is_bottom(false),
        _ptr_map(points_to_map_t::top()),
        _inv(ScalarDomain::top()) {}

  pointer_domain_impl(const pointer_domain_t& o)
      : _is_bottom(o._is_bottom), _ptr_map(o._ptr_map), _inv(o._inv) {}

  pointer_domain_t& operator=(const pointer_domain_t& o) {
    this->_is_bottom = o._is_bottom;
    this->_ptr_map = o._ptr_map;
    this->_inv = o._inv;
    return *this;
  }

  bool is_bottom() {
    if (!_is_bottom) {
      _is_bottom = _ptr_map.is_bottom() || _inv.is_bottom();
      if (_is_bottom) {
        _ptr_map = points_to_map_t::bottom();
        _inv = ScalarDomain::bottom();
      }
    }
    return _is_bottom;
  }

  bool is_top() { return _ptr_map.is_top() && _inv.is_top(); }

  bool operator<=(pointer_domain_t o) {
    if (is_bottom()) {
      return true;
    } else if (o.is_bottom()) {
      return false;
    } else {
      return _ptr_map <= o._ptr_map && _inv <= o._inv;
    }
  }

  pointer_domain_t operator|(pointer_domain_t o) {
    if (is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      return pointer_domain_t(_ptr_map | o._ptr_map, _inv | o._inv);
    }
  }

  pointer_domain_t operator||(pointer_domain_t o) {
    if (is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      return pointer_domain_t(_ptr_map | o._ptr_map, _inv || o._inv);
    }
  }

  pointer_domain_t operator&(pointer_domain_t o) {
    if (is_bottom() || o.is_bottom()) {
      return bottom();
    } else {
      return pointer_domain_t(_ptr_map & o._ptr_map, _inv & o._inv);
    }
  }

  pointer_domain_t operator&&(pointer_domain_t o) {
    if (is_bottom() || o.is_bottom()) {
      return bottom();
    } else {
      return pointer_domain_t(_ptr_map & o._ptr_map, _inv && o._inv);
    }
  }

  void operator-=(VariableName v) {
    // if v is a scalar variable
    _inv -= v;

    // if v is a pointer
    _ptr_map -= v;
    _inv -= this->offset_var(v);
  }

  /*
   * Implement pointer_domain
   */

  void assign_object(VariableName p, VariableName obj) {
    if (is_bottom())
      return;

    _ptr_map.set(p, points_to_set_t(obj));
    _inv.assign(this->offset_var(p), Number(0));
  }

  void assign_pointer(VariableName p, VariableName q) {
    if (is_bottom())
      return;

    _ptr_map.set(p, _ptr_map[q]);
    _inv.assign(this->offset_var(p), variable_t(this->offset_var(q)));
  }

  void assign_pointer(VariableName p, VariableName q, VariableName o) {
    if (is_bottom())
      return;

    _ptr_map.set(p, _ptr_map[q]);
    _inv.assign(this->offset_var(p),
                variable_t(this->offset_var(q)) + variable_t(o));
  }

  void assign_pointer(VariableName p, VariableName q, Number o) {
    if (is_bottom())
      return;

    _ptr_map.set(p, _ptr_map[q]);
    _inv.assign(this->offset_var(p), variable_t(this->offset_var(q)) + o);
  }

  void assert_pointer(bool equality, VariableName p, VariableName q) {
    if (is_bottom())
      return;

    points_to_set_t addrs_p = _ptr_map[p];
    points_to_set_t addrs_q = _ptr_map[q];

    if (equality) { // p == q
      points_to_set_t addrs_pq = addrs_p & addrs_q;

      if (addrs_pq.is_bottom()) {
        *this = bottom();
        return;
      }

      // p and q's points-to sets
      _ptr_map.set(p, addrs_pq);
      _ptr_map.set(q, addrs_pq);

      // p and q's offsets
      _inv +=
          (variable_t(this->offset_var(p)) == variable_t(this->offset_var(q)));
    } else { // p != q
      if (!addrs_p.is_top() && !addrs_p.is_top() && addrs_p.size() == 1 &&
          addrs_p == addrs_p) {
        // p and q's offsets
        _inv += (variable_t(this->offset_var(p)) !=
                 variable_t(this->offset_var(q)));
      }
    }

    if (_inv.is_bottom()) {
      *this = bottom();
      return;
    }
  }

  void refine_addrs(VariableName p, points_to_set_t addrs) {
    if (is_bottom())
      return;

    _ptr_map.set(p, _ptr_map[p] & addrs);

    if (_ptr_map.is_bottom()) {
      *this = bottom();
    }
  }

  void refine_addrs_offset(VariableName p,
                           points_to_set_t addrs,
                           interval_t offset) {
    if (is_bottom())
      return;

    _ptr_map.set(p, _ptr_map[p] & addrs);

    linear_constraint_system_t csts;
    if (offset.ub().is_finite()) {
      csts += (variable_t(this->offset_var(p)) <= *offset.ub().number());
    }
    if (offset.lb().is_finite()) {
      csts += (variable_t(this->offset_var(p)) >= *offset.lb().number());
    }
    _inv += csts;

    if (_ptr_map.is_bottom() || _inv.is_bottom()) {
      *this = bottom();
    }
  }

  bool is_unknown_addr(VariableName p) {
    if (is_bottom()) {
      throw ikos_error(
          "pointer domain: trying to call is_unknown_addr() on bottom");
    } else {
      return _ptr_map[p].is_top();
    }
  }

  points_to_set_t addrs_set(VariableName p) {
    if (is_bottom()) {
      throw ikos_error("pointer domain: trying to call addrs_set() on bottom");
    } else {
      return _ptr_map[p];
    }
  }

  /*
   * Implement numerical_domain
   */

  void assign(VariableName x, linear_expression_t e) { _inv.assign(x, e); }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    _inv.apply(op, x, y, z);
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    _inv.apply(op, x, y, k);
  }

  void operator+=(linear_constraint_t cst) { _inv += cst; }

  void operator+=(linear_constraint_system_t csts) { _inv += csts; }

  linear_constraint_system_t to_linear_constraint_system() {
    return _inv.to_linear_constraint_system();
  }

  /*
   * Helpers for num_domain_traits
   */

  void normalize() { num_domain_traits::normalize(_inv); }

  template < typename ScalarDomainTo >
  pointer_domain_impl< ScalarDomainTo > convert() {
    if (is_bottom()) {
      return pointer_domain_impl< ScalarDomainTo >::bottom();
    } else {
      return pointer_domain_impl<
          ScalarDomainTo >(_ptr_map,
                           num_domain_traits::convert< ScalarDomain,
                                                       ScalarDomainTo >(_inv));
    }
  }

  interval_t to_interval(VariableName v) {
    return num_domain_traits::to_interval(_inv, v);
  }

  interval_t to_interval(linear_expression_t e) {
    return num_domain_traits::to_interval(_inv, e);
  }

  void from_interval(VariableName v, interval_t i) {
    num_domain_traits::from_interval(_inv, v, i);
  }

  /*
   * Implement bitwise_operators
   */

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             unsigned width) {
    _inv.apply(op, x, y, width);
  }

  void apply(conv_operation_t op, VariableName x, Number k, unsigned width) {
    _inv.apply(op, x, k, width);
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    _inv.apply(op, x, y, z);
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    _inv.apply(op, x, y, k);
  }

  /*
   * Implement division_operators
   */

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    _inv.apply(op, x, y, z);
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    _inv.apply(op, x, y, k);
  }

  void write(std::ostream& o) {
    if (is_bottom()) {
      o << "_|_";
    } else {
      o << "(" << _ptr_map << ", " << _inv << ")";
    }
  }

  static std::string domain_name() {
    return "Pointer of " + ScalarDomain::domain_name();
  }

  // required for convert()
  template < typename Any >
  friend class pointer_domain_impl;

}; // end class pointer_domain_impl

namespace num_domain_traits {
namespace detail {

template < typename ScalarDomain >
struct normalize_impl< pointer_domain_impl< ScalarDomain > > {
  void operator()(pointer_domain_impl< ScalarDomain >& inv) { inv.normalize(); }
};

template < typename ScalarDomainFrom, typename ScalarDomainTo >
struct convert_impl< pointer_domain_impl< ScalarDomainFrom >,
                     pointer_domain_impl< ScalarDomainTo > > {
  pointer_domain_impl< ScalarDomainTo > operator()(
      pointer_domain_impl< ScalarDomainFrom > inv) {
    return inv.template convert< ScalarDomainTo >();
  }
};

template < typename ScalarDomain >
struct var_to_interval_impl< pointer_domain_impl< ScalarDomain > > {
  interval< typename ScalarDomain::number_t > operator()(
      pointer_domain_impl< ScalarDomain >& inv,
      typename ScalarDomain::variable_name_t v) {
    return inv.to_interval(v);
  }
};

template < typename ScalarDomain >
struct lin_expr_to_interval_impl< pointer_domain_impl< ScalarDomain > > {
  interval< typename ScalarDomain::number_t > operator()(
      pointer_domain_impl< ScalarDomain >& inv,
      typename ScalarDomain::linear_expression_t e) {
    return inv.to_interval(e);
  }
};

template < typename ScalarDomain >
struct from_interval_impl< pointer_domain_impl< ScalarDomain > > {
  void operator()(pointer_domain_impl< ScalarDomain >& inv,
                  typename ScalarDomain::variable_name_t v,
                  interval< typename ScalarDomain::number_t > i) {
    inv.from_interval(v, i);
  }
};

} // end namespace detail
} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_POINTER_HPP
