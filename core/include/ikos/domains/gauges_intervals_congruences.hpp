/**************************************************************************/ /**
 *
 * \file
 * \brief Reduced product of gauges, intervals and congruences.
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017 United States Government as represented by the
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

#ifndef IKOS_GAUGES_INTERVALS_CONGRUENCES_HPP
#define IKOS_GAUGES_INTERVALS_CONGRUENCES_HPP

#include <ikos/domains/gauges.hpp>
#include <ikos/domains/intervals_congruences.hpp>
#include <ikos/value/congruence.hpp>
#include <ikos/value/interval.hpp>

namespace ikos {

/// \brief Reduced product of gauges, intervals and congruences.
template < typename Number, typename VariableName, int TypeSize = -1 >
class gauge_interval_congruence_domain
    : public abstract_domain,
      public numerical_domain< Number, VariableName >,
      public bitwise_operators< Number, VariableName >,
      public division_operators< Number, VariableName >,
      public counter_domain< Number, VariableName > {
public:
  typedef variable< Number, VariableName > variable_t;
  typedef patricia_tree_set< variable_t > variable_set_t;
  typedef gauge_bound< Number, VariableName > gauge_bound_t;
  typedef gauge< Number, VariableName > gauge_t;
  typedef interval< Number > interval_t;
  typedef congruence< Number > congruence_t;
  typedef congruence< Number, TypeSize > congruence_t;
  typedef interval_congruence< Number, TypeSize > interval_congruence_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;
  typedef gauge_interval_congruence_domain< Number, VariableName, TypeSize >
      gauge_interval_congruence_domain_t;

private:
  typedef gauge_domain< Number, VariableName > gauge_domain_t;
  typedef interval_domain< Number, VariableName > interval_domain_t;
  typedef congruence_domain< Number, VariableName, TypeSize >
      congruence_domain_t;
  typedef numerical_domain_product3< Number,
                                     VariableName,
                                     gauge_domain_t,
                                     interval_domain_t,
                                     congruence_domain_t > domain_product3_t;

private:
  domain_product3_t _product;

private:
  gauge_interval_congruence_domain(const domain_product3_t& product)
      : _product(product) {}

  gauge_interval_congruence_domain(const gauge_domain_t& first,
                                   const interval_domain_t& second,
                                   const congruence_domain_t& third)
      : _product(first, second, third) {}

  void reduce_variable(const VariableName& v) {
    if (this->is_bottom())
      return;

    interval_t i = this->first().to_interval(v) & this->second()[v];
    congruence_t c = this->third()[v];
    interval_congruence_t val(i, c);

    if (val.is_bottom()) {
      *this = bottom();
    } else {
      if (val.first() != i) {
        this->second().set(v, val.first());
      }
      if (val.second() != c) {
        this->third().set(v, val.second());
      }
    }
  }

  template < typename Iterator >
  void reduce_variables(Iterator first, Iterator last) {
    for (Iterator it = first; !is_bottom() && it != last; ++it) {
      this->reduce_variable(*it);
    }
  }

  void reduce_variables(variable_set_t variables) {
    for (typename variable_set_t::iterator it = variables.begin();
         !is_bottom() && it != variables.end();
         ++it) {
      this->reduce_variable((*it).name());
    }
  }

public:
  static gauge_interval_congruence_domain_t top() {
    return gauge_interval_congruence_domain_t(domain_product3_t::top());
  }

  static gauge_interval_congruence_domain_t bottom() {
    return gauge_interval_congruence_domain_t(domain_product3_t::bottom());
  }

public:
  gauge_interval_congruence_domain() : _product() {}

  gauge_interval_congruence_domain(
      const gauge_interval_congruence_domain_t& other) = default;

  gauge_interval_congruence_domain_t& operator=(
      const gauge_interval_congruence_domain_t& other) = default;

  bool is_bottom() { return this->_product.is_bottom(); }

  bool is_top() { return this->_product.is_top(); }

  gauge_domain_t& first() { return this->_product.first(); }

  interval_domain_t& second() { return this->_product.second(); }

  congruence_domain_t& third() { return this->_product.third(); }

  bool operator<=(gauge_interval_congruence_domain_t other) {
    return this->_product <= other._product;
  }

  bool operator==(gauge_interval_congruence_domain_t other) {
    return this->_product == other._product;
  }

  gauge_interval_congruence_domain_t operator|(
      gauge_interval_congruence_domain_t other) {
    return gauge_interval_congruence_domain_t(this->_product | other._product);
  }

  gauge_interval_congruence_domain_t operator||(
      gauge_interval_congruence_domain_t other) {
    return gauge_interval_congruence_domain_t(this->_product || other._product);
  }

  gauge_interval_congruence_domain_t join_loop(
      gauge_interval_congruence_domain_t other) {
    return gauge_interval_congruence_domain_t(
        this->_product.join_loop(other._product));
  }

  gauge_interval_congruence_domain_t join_iter(
      gauge_interval_congruence_domain_t other) {
    return gauge_interval_congruence_domain_t(
        this->_product.join_iter(other._product));
  }

  gauge_interval_congruence_domain_t operator&(
      gauge_interval_congruence_domain_t other) {
    return gauge_interval_congruence_domain_t(this->_product & other._product);
  }

  gauge_interval_congruence_domain_t operator&&(
      gauge_interval_congruence_domain_t other) {
    return gauge_interval_congruence_domain_t(this->_product && other._product);
  }

  congruence_t to_congruence(VariableName v) {
    if (this->is_bottom()) {
      return congruence_t::bottom();
    } else {
      interval_t i = this->first().to_interval(v) & this->second()[v];
      congruence_t c = this->third()[v];
      interval_congruence_t val(i, c);
      return val.second();
    }
  }

  interval_t to_interval(VariableName v) {
    if (this->is_bottom()) {
      return interval_t::bottom();
    } else {
      interval_t i = this->first().to_interval(v) & this->second()[v];
      congruence_t c = this->third()[v];
      interval_congruence_t val(i, c);
      return val.first();
    }
  }

  interval_t to_interval(linear_expression_t e) {
    if (this->is_bottom()) {
      return interval_t::bottom();
    } else {
      interval_t i = this->first().to_interval(e) & this->second()[e];
      congruence_t c = this->third()[e];
      interval_congruence_t val(i, c);
      return val.first();
    }
  }

  void set_interval(VariableName x, interval_t value) {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      *this = bottom();
    } else {
      interval_congruence< Number > ic(value);
      this->first().set_interval(x, ic.first());
      this->second().set(x, ic.first());
      this->third().set(x, ic.second());
    }
  }

  void operator+=(linear_constraint_system_t csts) {
    this->_product += csts;
    this->reduce_variables(csts.variables());
  }

  void assign(VariableName x, linear_expression_t e) {
    this->_product.assign(x, e);
    this->reduce_variable(x);
  }

  void operator-=(VariableName v) { this->_product -= v; }

  void forget(VariableName v) { this->_product.forget(v); }

  void forget_num(VariableName v) { this->_product.forget_num(v); }

  template < typename Iterator >
  void forget(Iterator begin, Iterator end) {
    this->_product.forget(begin, end);
  }

  template < typename Iterator >
  void forget_num(Iterator begin, Iterator end) {
    this->_product.forget_num(begin, end);
  }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    this->_product.apply(op, x, y, z);
    this->reduce_variable(x);
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    this->_product.apply(op, x, y, k);
    this->reduce_variable(x);
  }

  // bitwise_operators_api

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             uint64_t from,
             uint64_t to) {
    this->_product.apply(op, x, y, from, to);
    this->reduce_variable(x);
  }

  void apply(conv_operation_t op,
             VariableName x,
             Number k,
             uint64_t from,
             uint64_t to) {
    this->_product.apply(op, x, k, from, to);
    this->reduce_variable(x);
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    this->_product.apply(op, x, y, z);
    this->reduce_variable(x);
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    this->_product.apply(op, x, y, k);
    this->reduce_variable(x);
  }

  // division_operators_api

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    this->_product.apply(op, x, y, z);
    this->reduce_variable(x);
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    this->_product.apply(op, x, y, k);
    this->reduce_variable(x);
  }

  // counter_domain

  void mark_counter(VariableName x) { this->first().mark_counter(x); }

  void unmark_counter(VariableName x) { this->first().unmark_counter(x); }

  void init_counter(VariableName x, Number c) {
    if (this->is_bottom()) {
      return;
    }

    this->first().init_counter(x, c);
    this->second().assign(x, c);
    this->third().assign(x, c);
  }

  void incr_counter(VariableName x, Number c) {
    if (this->is_bottom()) {
      return;
    }

    this->first().incr_counter(x, c);
    this->second().assign(x, variable_t(x) + c);
    this->third().assign(x, variable_t(x) + c);
  }

  void forget_counter(VariableName x) {
    if (this->is_bottom()) {
      return;
    }

    this->first().forget_counter(x);
    this->second().forget_num(x);
    this->third().forget_num(x);
  }

  void write(std::ostream& o) { this->_product.write(o); }

  linear_constraint_system_t to_linear_constraint_system() {
    linear_constraint_system_t csts;

    if (this->is_bottom()) {
      csts += linear_constraint_t::contradiction();
      return csts;
    }

    csts += this->first().to_linear_constraint_system();
    csts += this->second().to_linear_constraint_system();
    return csts;
  }

  static std::string domain_name() {
    return "Gauges + Intervals + Congruences";
  }

}; // end class gauge_interval_congruence_domain

namespace num_domain_traits {
namespace detail {

template < typename Number, typename VariableName, int TypeSize >
struct var_to_interval_impl<
    gauge_interval_congruence_domain< Number, VariableName, TypeSize > > {
  inline interval< Number > operator()(
      gauge_interval_congruence_domain< Number, VariableName, TypeSize >& inv,
      VariableName v,
      bool /*normalize*/) {
    return inv.to_interval(v);
  }
};

template < typename Number, typename VariableName, int TypeSize >
struct lin_expr_to_interval_impl<
    gauge_interval_congruence_domain< Number, VariableName, TypeSize > > {
  inline interval< Number > operator()(
      gauge_interval_congruence_domain< Number, VariableName, TypeSize >& inv,
      linear_expression< Number, VariableName > e,
      bool /*normalize*/) {
    return inv.to_interval(e);
  }
};

template < typename Number, typename VariableName, int TypeSize >
struct from_interval_impl<
    gauge_interval_congruence_domain< Number, VariableName, TypeSize > > {
  inline void operator()(
      gauge_interval_congruence_domain< Number, VariableName, TypeSize >& inv,
      VariableName v,
      interval< Number > i) {
    inv.set_interval(v, i);
  }
};

} // end namespace detail
} // end namespace num_domain_traits
} // end namespace ikos

#endif // IKOS_GAUGES_INTERVALS_CONGRUENCES_HPP
