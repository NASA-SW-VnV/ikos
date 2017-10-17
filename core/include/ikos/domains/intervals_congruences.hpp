/**************************************************************************/ /**
 *
 * \file
 * \brief Reduced product of intervals and congruences.
 *
 * The reduce operator based on "Static Analysis of Arithmetical
 * Congruences" by P. Granger published in International Journal of
 * Computer Mathematics, 1989.
 *
 * Author: Jorge A. Navas Laserna
 *
 * Contributors: Maxime Arthaud
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

#ifndef IKOS_INTERVALS_CONGRUENCES_HPP
#define IKOS_INTERVALS_CONGRUENCES_HPP

#include <iostream>

#include <ikos/common/types.hpp>
#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/domains/bitwise_operators_api.hpp>
#include <ikos/domains/division_operators_api.hpp>
#include <ikos/domains/domain_products.hpp>
#include <ikos/domains/intervals.hpp>
#include <ikos/domains/congruences.hpp>
#include <ikos/domains/numerical_domains_api.hpp>
#include <ikos/value/congruence.hpp>
#include <ikos/value/interval.hpp>
#include <ikos/value/interval_congruence.hpp>

namespace ikos {

/// \brief Reduced product of intervals and congruences
template < typename Number, typename VariableName, int TypeSize = -1 >
class interval_congruence_domain
    : public abstract_domain,
      public numerical_domain< Number, VariableName >,
      public bitwise_operators< Number, VariableName >,
      public division_operators< Number, VariableName > {
public:
  // note that this is assuming that all variables have the same
  // bit width for the congruence domain which is unrealistic.
  typedef interval_congruence< Number, TypeSize > interval_congruence_t;
  typedef interval_congruence_domain< Number, VariableName, TypeSize >
      interval_congruence_domain_t;

public:
  typedef variable< Number, VariableName > variable_t;
  typedef patricia_tree_set< variable_t > variable_set_t;
  typedef interval< Number > interval_t;
  typedef congruence< Number, TypeSize > congruence_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;

private:
  typedef interval_domain< Number, VariableName > interval_domain_t;
  typedef congruence_domain< Number, VariableName, TypeSize >
      congruence_domain_t;
  typedef numerical_domain_product2< Number,
                                     VariableName,
                                     interval_domain_t,
                                     congruence_domain_t > domain_product2_t;

private:
  domain_product2_t _product;

private:
  interval_congruence_domain(const domain_product2_t& product)
      : _product(product) {}

  void reduce_variable(const VariableName& v) {
    if (is_bottom())
      return;

    interval_t i = this->_product.first()[v];
    congruence_t c = this->_product.second()[v];
    interval_congruence_t val(i, c);

    if (val.is_bottom()) {
      *this = bottom();
    } else {
      if (val.first() != i) {
        this->_product.first().set(v, val.first());
      }
      if (val.second() != c) {
        this->_product.second().set(v, val.second());
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
  static interval_congruence_domain_t top() {
    return interval_congruence_domain_t(domain_product2_t::top());
  }

  static interval_congruence_domain_t bottom() {
    return interval_congruence_domain_t(domain_product2_t::bottom());
  }

public:
  interval_congruence_domain() : _product() {}

  interval_congruence_domain(const interval_congruence_domain_t& other)
      : _product(other._product) {}

  interval_congruence_domain(const interval_domain_t& first,
                             const congruence_domain_t& second)
      : _product(first, second) {}

  interval_congruence_domain_t& operator=(
      const interval_congruence_domain_t& other) {
    this->_product = other._product;
    return *this;
  }

  bool is_bottom() { return this->_product.is_bottom(); }

  bool is_top() { return this->_product.is_top(); }

  interval_domain_t& first() { return this->_product.first(); }

  congruence_domain_t& second() { return this->_product.second(); }

  bool operator<=(interval_congruence_domain_t other) {
    return this->_product <= other._product;
  }

  bool operator==(interval_congruence_domain_t other) {
    return this->_product == other._product;
  }

  interval_congruence_domain_t operator|(interval_congruence_domain_t other) {
    return interval_congruence_domain_t(this->_product | other._product);
  }

  interval_congruence_domain_t operator||(interval_congruence_domain_t other) {
    return interval_congruence_domain_t(this->_product || other._product);
  }

  interval_congruence_domain_t join_loop(interval_congruence_domain_t other) {
    return interval_congruence_domain_t(
        this->_product.join_loop(other._product));
  }

  interval_congruence_domain_t join_iter(interval_congruence_domain_t other) {
    return interval_congruence_domain_t(
        this->_product.join_iter(other._product));
  }

  interval_congruence_domain_t operator&(interval_congruence_domain_t other) {
    return interval_congruence_domain_t(this->_product & other._product);
  }

  interval_congruence_domain_t operator&&(interval_congruence_domain_t other) {
    return interval_congruence_domain_t(this->_product && other._product);
  }

  /// \brief Set an abstract value for variable v
  ///
  /// Precondition: x is already reduced
  void set(VariableName v, interval_congruence_t x) {
    this->_product.first().set(v, x.first());
    this->_product.second().set(v, x.second());
  }

  interval_congruence_t operator[](VariableName v) {
    return interval_congruence_t(this->_product.first()[v],
                                 this->_product.second()[v]);
  }

  congruence_t to_congruence(VariableName v) {
    return num_domain_traits::to_congruence(this->_product.second(), v);
  }

  void operator+=(linear_constraint_system_t csts) {
    this->_product += csts;
    this->reduce_variables(csts.variables());
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

  void assign(VariableName x, linear_expression_t e) {
    this->_product.assign(x, e);
    this->reduce_variable(x);
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

  linear_constraint_system_t to_linear_constraint_system() {
    return this->_product.first().to_linear_constraint_system();
  }

  void write(std::ostream& o) { this->_product.write(o); }

  static std::string domain_name() { return "Intervals + Congruences"; }

}; // end class interval_congruence_domain

namespace num_domain_traits {
namespace detail {

template < typename Number, typename VariableName, int TypeSize >
struct var_to_interval_impl<
    interval_congruence_domain< Number, VariableName, TypeSize > > {
  inline interval< Number > operator()(
      interval_congruence_domain< Number, VariableName, TypeSize >& inv,
      VariableName v,
      bool /*normalize*/) {
    return inv[v].first();
  }
};

template < typename Number, typename VariableName, int TypeSize >
struct from_interval_impl<
    interval_congruence_domain< Number, VariableName, TypeSize > > {
  inline void operator()(
      interval_congruence_domain< Number, VariableName, TypeSize >& inv,
      VariableName v,
      interval< Number > i) {
    interval_congruence< Number > ic(i);
    inv.set(v, ic);
  }
};

} // end namespace detail
} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_INTERVALS_CONGRUENCES_HPP
