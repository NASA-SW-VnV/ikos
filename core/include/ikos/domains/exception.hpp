/*******************************************************************************
 *
 * Implementation of an abstract domain that keeps track of exceptions.
 *
 * Author: Maxime Arthaud
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

#ifndef IKOS_EXCEPTION_DOMAIN_HPP
#define IKOS_EXCEPTION_DOMAIN_HPP

#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/domains/bitwise_operators_api.hpp>
#include <ikos/domains/division_operators_api.hpp>
#include <ikos/domains/memory_domains_api.hpp>
#include <ikos/domains/nullity_domains_api.hpp>
#include <ikos/domains/numerical_domains_api.hpp>
#include <ikos/domains/pointer_domains_api.hpp>
#include <ikos/domains/uninitialized_domains_api.hpp>
#include <ikos/domains/exception_domains_api.hpp>

namespace ikos {

template < typename AbsDomain >
class exception_domain_impl
    : public abstract_domain,
      public numerical_domain< typename AbsDomain::number_t,
                               typename AbsDomain::variable_name_t >,
      public bitwise_operators< typename AbsDomain::number_t,
                                typename AbsDomain::variable_name_t >,
      public division_operators< typename AbsDomain::number_t,
                                 typename AbsDomain::variable_name_t >,
      public pointer_domain< typename AbsDomain::number_t,
                             typename AbsDomain::variable_name_t >,
      public uninitialized_domain< typename AbsDomain::variable_name_t >,
      public nullity_domain< typename AbsDomain::variable_name_t >,
      public memory_domain< typename AbsDomain::number_t,
                            dummy_number,
                            typename AbsDomain::variable_name_t >,
      public exception_domain< typename AbsDomain::variable_name_t > {
private:
  typedef typename AbsDomain::number_t Number;
  typedef typename AbsDomain::variable_name_t VariableName;

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
  typedef literal< Number, dummy_number, VariableName > literal_t;
  typedef AbsDomain abs_domain_t;
  typedef exception_domain_impl< AbsDomain > exception_domain_t;

private:
  // Represents the normal execution flow state
  AbsDomain _normal;

  // Represents the union of all possible states of the program where
  // an exception has been thrown
  AbsDomain _exception;

  // Most methods update _normal, as _exception will be treated when we hit
  // a catch() {}

private:
  exception_domain_impl(const AbsDomain& n, const AbsDomain& e)
      : _normal(n), _exception(e) {}

public:
  static exception_domain_t top() {
    return exception_domain_t(AbsDomain::top(), AbsDomain::top());
  }

  static exception_domain_t top_no_exception() {
    return exception_domain_t(AbsDomain::top(), AbsDomain::bottom());
  }

  static exception_domain_t bottom() {
    return exception_domain_t(AbsDomain::bottom(), AbsDomain::bottom());
  }

public:
  exception_domain_impl()
      : _normal(AbsDomain::top()), _exception(AbsDomain::top()) {}

  exception_domain_impl(const exception_domain_t& o)
      : _normal(o._normal), _exception(o._exception) {}

  exception_domain_t& operator=(const exception_domain_t& o) {
    _normal = o._normal;
    _exception = o._exception;
    return *this;
  }

  bool is_bottom() { return _normal.is_bottom() && _exception.is_bottom(); }

  bool is_normal_flow_bottom() { return _normal.is_bottom(); }

  bool is_pending_exceptions_bottom() { return _exception.is_bottom(); }

  bool is_top() { return _normal.is_top() && _exception.is_top(); }

  bool is_normal_flow_top() { return _normal.is_top(); }

  bool is_pending_exceptions_top() { return _exception.is_top(); }

  bool operator<=(exception_domain_t o) {
    return _normal <= o._normal && _exception <= o._exception;
  }

  exception_domain_t operator|(exception_domain_t o) {
    return exception_domain_t(_normal | o._normal, _exception | o._exception);
  }

  exception_domain_t operator||(exception_domain_t o) {
    return exception_domain_t(_normal || o._normal, _exception || o._exception);
  }

  exception_domain_t operator&(exception_domain_t o) {
    return exception_domain_t(_normal & o._normal, _exception & o._exception);
  }

  exception_domain_t operator&&(exception_domain_t o) {
    return exception_domain_t(_normal && o._normal, _exception && o._exception);
  }

  void operator-=(VariableName v) { _normal -= v; }

  /*
   * Implement numerical_domain
   */

  void assign(VariableName x, linear_expression_t e) { _normal.assign(x, e); }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    _normal.apply(op, x, y, z);
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    _normal.apply(op, x, y, k);
  }

  void operator+=(linear_constraint_t cst) { _normal += cst; }

  void operator+=(linear_constraint_system_t csts) { _normal += csts; }

  linear_constraint_system_t to_linear_constraint_system() {
    return _normal.to_linear_constraint_system();
  }

  /*
   * Helpers for num_domain_traits
   */

  void normalize() {
    num_domain_traits::normalize(_normal);
    num_domain_traits::normalize(_exception);
  }

  interval_t to_interval(VariableName v) {
    return num_domain_traits::to_interval(_normal, v);
  }

  interval_t to_interval(linear_expression_t e) {
    return num_domain_traits::to_interval(_normal, e);
  }

  void from_interval(VariableName v, interval_t i) {
    return num_domain_traits::from_interval(_normal, v, i);
  }

  /*
   * Implement bitwise_operators
   */

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             unsigned width) {
    _normal.apply(op, x, y, width);
  }

  void apply(conv_operation_t op, VariableName x, Number k, unsigned width) {
    _normal.apply(op, x, k, width);
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    _normal.apply(op, x, y, z);
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    _normal.apply(op, x, y, k);
  }

  /*
   * Implement division_operators
   */

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    _normal.apply(op, x, y, z);
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    _normal.apply(op, x, y, k);
  }

  /*
   * Implement pointer_domain
   */

  void assign_object(VariableName p, VariableName obj) {
    ptr_domain_traits::assign_object(_normal, p, obj);
  }

  void assign_pointer(VariableName p, VariableName q) {
    ptr_domain_traits::assign_pointer(_normal, p, q);
  }

  void assign_pointer(VariableName p, VariableName q, VariableName o) {
    ptr_domain_traits::assign_pointer(_normal, p, q, o);
  }

  void assign_pointer(VariableName p, VariableName q, Number o) {
    ptr_domain_traits::assign_pointer(_normal, p, q, o);
  }

  void assert_pointer(bool equality, VariableName p, VariableName q) {
    ptr_domain_traits::assert_pointer(_normal, equality, p, q);
  }

  void refine_addrs(VariableName p, points_to_set_t addrs) {
    ptr_domain_traits::refine_addrs(_normal, p, addrs);
  }

  void refine_addrs_offset(VariableName p,
                           points_to_set_t addrs,
                           interval_t offset) {
    ptr_domain_traits::refine_addrs_offset(_normal, p, addrs, offset);
  }

  bool is_unknown_addr(VariableName p) {
    return ptr_domain_traits::is_unknown_addr(_normal, p);
  }

  points_to_set_t addrs_set(VariableName p) {
    return ptr_domain_traits::addrs_set(_normal, p);
  }

  VariableName offset_var(VariableName p) {
    return ptr_domain_traits::offset_var(_normal, p);
  }

  /*
   * Implement uninitialized_domain
   */

  void make_initialized(VariableName v) {
    uninit_domain_traits::make_initialized(_normal, v);
  }

  void make_uninitialized(VariableName v) {
    uninit_domain_traits::make_uninitialized(_normal, v);
  }

  void assign_uninitialized(VariableName x, VariableName y) {
    uninit_domain_traits::assign_uninitialized(_normal, x, y);
  }

  void assign_uninitialized(VariableName x, VariableName y, VariableName z) {
    uninit_domain_traits::assign_uninitialized(_normal, x, y, z);
  }

  void assign_uninitialized(VariableName x,
                            const std::vector< VariableName >& y) {
    uninit_domain_traits::assign_uninitialized(_normal, x, y);
  }

  bool is_initialized(VariableName v) {
    return uninit_domain_traits::is_initialized(_normal, v);
  }

  bool is_uninitialized(VariableName v) {
    return uninit_domain_traits::is_uninitialized(_normal, v);
  }

  /*
   * Implement nullity_domain
   */

  void make_null(VariableName v) { null_domain_traits::make_null(_normal, v); }

  void make_non_null(VariableName v) {
    null_domain_traits::make_non_null(_normal, v);
  }

  void assign_nullity(VariableName x, VariableName y) {
    null_domain_traits::assign_nullity(_normal, x, y);
  }

  void assert_null(VariableName v) {
    null_domain_traits::assert_null(_normal, v);
  }

  void assert_non_null(VariableName v) {
    null_domain_traits::assert_non_null(_normal, v);
  }

  void assert_nullity(bool equality, VariableName x, VariableName y) {
    null_domain_traits::assert_nullity(_normal, equality, x, y);
  }

  bool is_null(VariableName v) {
    return null_domain_traits::is_null(_normal, v);
  }

  bool is_non_null(VariableName v) {
    return null_domain_traits::is_non_null(_normal, v);
  }

  /*
   * Implement memory_domain
   */

  void mem_write(VariableName p, literal_t v, Number size) {
    mem_domain_traits::mem_write(_normal, p, v, size);
  }

  void mem_read(literal_t x, VariableName p, Number size) {
    mem_domain_traits::mem_read(_normal, x, p, size);
  }

  void mem_copy(VariableName dest, VariableName src, linear_expression_t size) {
    mem_domain_traits::mem_copy(_normal, dest, src, size);
  }

  void mem_set(VariableName dest,
               linear_expression_t value,
               linear_expression_t size) {
    mem_domain_traits::mem_set(_normal, dest, value, size);
  }

  void forget_mem_surface(VariableName p) {
    mem_domain_traits::forget_mem_surface(_normal, p);
  }

  void forget_mem_contents(VariableName p) {
    mem_domain_traits::forget_mem_contents(_normal, p);
  }

  void forget_mem_contents(VariableName p, Number size) {
    mem_domain_traits::forget_mem_contents(_normal, p, size);
  }

  /*
   * Implement exception_domain
   */

  void set_normal_flow_bottom() { _normal = AbsDomain::bottom(); }

  void set_pending_exceptions_top() { _exception = AbsDomain::top(); }

  void enter_catch(VariableName exc) {
    _normal = _exception;
    _exception = AbsDomain::bottom();
    _normal -= exc;
  }

  void ignore_exceptions() { _exception = AbsDomain::bottom(); }

  void throw_exception(VariableName exc,
                       VariableName tinfo,
                       boost::optional< VariableName > dest) {
    _exception = _exception | _normal;
    _normal = AbsDomain::bottom();
  }

  void resume_exception(VariableName exc) {
    _exception = _exception | _normal;
    _normal = AbsDomain::bottom();
  }

  void write(std::ostream& o) {
    o << "(normal=";
    _normal.write(o);
    o << ", exception=";
    _exception.write(o);
    o << ")";
  }

  static std::string domain_name() {
    return "Exception of " + AbsDomain::domain_name();
  }

}; // end class exception_domain_impl

namespace num_domain_traits {
namespace detail {

template < typename AbsDomain >
struct normalize_impl< exception_domain_impl< AbsDomain > > {
  void operator()(exception_domain_impl< AbsDomain >& inv) { inv.normalize(); }
};

template < typename AbsDomain >
struct var_to_interval_impl< exception_domain_impl< AbsDomain > > {
  interval< typename AbsDomain::number_t > operator()(
      exception_domain_impl< AbsDomain >& inv,
      typename AbsDomain::variable_name_t v) {
    return inv.to_interval(v);
  }
};

template < typename AbsDomain >
struct lin_expr_to_interval_impl< exception_domain_impl< AbsDomain > > {
  interval< typename AbsDomain::number_t > operator()(
      exception_domain_impl< AbsDomain >& inv,
      typename AbsDomain::linear_expression_t e) {
    return inv.to_interval(e);
  }
};

template < typename AbsDomain >
struct from_interval_impl< exception_domain_impl< AbsDomain > > {
  void operator()(exception_domain_impl< AbsDomain >& inv,
                  typename AbsDomain::variable_name_t v,
                  interval< typename AbsDomain::number_t > i) {
    inv.from_interval(v, i);
  }
};

} // end namespace detail
} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_EXCEPTION_DOMAIN_HPP
