/**************************************************************************/ /**
 *
 * \file
 * \brief Products of abstract domains.
 *
 * Author: Arnaud J. Venet
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

#ifndef IKOS_DOMAIN_PRODUCTS_HPP
#define IKOS_DOMAIN_PRODUCTS_HPP

#include <iostream>

#include <ikos/common/types.hpp>
#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/domains/bitwise_operators_api.hpp>
#include <ikos/domains/division_operators_api.hpp>
#include <ikos/domains/numerical_domains_api.hpp>

namespace ikos {

/// \brief Domain product of 2 abstract domains
template < typename Domain1, typename Domain2 >
class domain_product2 : public abstract_domain {
public:
  typedef domain_product2< Domain1, Domain2 > domain_product2_t;

private:
  bool _is_bottom;
  Domain1 _first;
  Domain2 _second;

public:
  static domain_product2_t top() {
    return domain_product2_t(Domain1::top(), Domain2::top());
  }

  static domain_product2_t bottom() {
    return domain_product2_t(Domain1::bottom(), Domain2::bottom());
  }

private:
  void canonicalize() {
    if (!this->_is_bottom) {
      this->_is_bottom = this->_first.is_bottom() || this->_second.is_bottom();
      if (this->_is_bottom) {
        this->_first = Domain1::bottom();
        this->_second = Domain2::bottom();
      }
    }
  }

public:
  domain_product2()
      : _is_bottom(false), _first(Domain1::top()), _second(Domain2::top()) {}

  domain_product2(Domain1 first, Domain2 second)
      : _is_bottom(false), _first(first), _second(second) {
    this->canonicalize();
  }

  domain_product2(const domain_product2_t& other)
      : _is_bottom(other._is_bottom),
        _first(other._first),
        _second(other._second) {}

  domain_product2_t& operator=(domain_product2_t other) {
    this->_is_bottom = other._is_bottom;
    this->_first = other._first;
    this->_second = other._second;
    return *this;
  }

  bool is_bottom() {
    this->canonicalize();
    return this->_is_bottom;
  }

  bool is_top() { return this->_first.is_top() && this->_second.is_top(); }

  Domain1& first() {
    this->canonicalize();
    return this->_first;
  }

  Domain2& second() {
    this->canonicalize();
    return this->_second;
  }

  bool operator<=(domain_product2_t other) {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      return (this->_first <= other._first) && (this->_second <= other._second);
    }
  }

  bool operator==(domain_product2_t other) {
    return (this->operator<=(other) && other.operator<=(*this));
  }

  domain_product2_t operator|(domain_product2_t other) {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return domain_product2_t(this->_first | other._first,
                               this->_second | other._second);
    }
  }

  domain_product2_t operator||(domain_product2_t other) {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return domain_product2_t(this->_first || other._first,
                               this->_second || other._second);
    }
  }

  domain_product2_t join_loop(domain_product2_t other) {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return domain_product2_t(this->_first.join_loop(other._first),
                               this->_second.join_loop(other._second));
    }
  }

  domain_product2_t join_iter(domain_product2_t other) {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return domain_product2_t(this->_first.join_iter(other._first),
                               this->_second.join_iter(other._second));
    }
  }

  domain_product2_t operator&(domain_product2_t other) {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else {
      return domain_product2_t(this->_first & other._first,
                               this->_second & other._second);
    }
  }

  domain_product2_t operator&&(domain_product2_t other) {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else {
      return domain_product2_t(this->_first && other._first,
                               this->_second && other._second);
    }
  }

  void write(std::ostream& o) {
    if (this->is_bottom()) {
      o << "_|_";
    } else {
      o << "(" << this->_first << ", " << this->_second << ")";
    }
  }

  static std::string domain_name() {
    return "Product " + Domain1::domain_name() + " x " + Domain2::domain_name();
  }

}; // end class domain_product2

/// \brief Domain product of 3 abstract domains
template < typename Domain1, typename Domain2, typename Domain3 >
class domain_product3 : public abstract_domain {
public:
  typedef domain_product3< Domain1, Domain2, Domain3 > domain_product3_t;

private:
  typedef domain_product2< Domain2, Domain3 > product23_t;
  typedef domain_product2< Domain1, product23_t > product123_t;

private:
  product123_t _product;

private:
  domain_product3(product123_t product) : _product(product) {}

public:
  static domain_product3_t top() {
    return domain_product3_t(product123_t::top());
  }

  static domain_product3_t bottom() {
    return domain_product3_t(product123_t::bottom());
  }

public:
  domain_product3() : _product() {}

  domain_product3(Domain1 first, Domain2 second, Domain3 third)
      : _product(first, product23_t(second, third)) {}

  domain_product3(const domain_product3_t& other) : _product(other._product) {}

  domain_product3_t& operator=(domain_product3_t other) {
    this->_product = other._product;
    return *this;
  }

  bool is_bottom() { return this->_product.is_bottom(); }

  bool is_top() { return this->_product.is_top(); }

  Domain1& first() { return this->_product.first(); }

  Domain2& second() { return this->_product.second().first(); }

  Domain3& third() { return this->_product.second().second(); }

  bool operator<=(domain_product3_t other) {
    return (this->_product <= other._product);
  }

  bool operator==(domain_product3_t other) {
    return (this->_product == other._product);
  }

  domain_product3_t operator|(domain_product3_t other) {
    return domain_product3_t(this->_product | other._product);
  }

  domain_product3_t operator||(domain_product3_t other) {
    return domain_product3_t(this->_product || other._product);
  }

  domain_product3_t join_loop(domain_product3_t other) {
    return domain_product3_t(this->_product.join_loop(other._product));
  }

  domain_product3_t join_iter(domain_product3_t other) {
    return domain_product3_t(this->_product.join_iter(other._product));
  }

  domain_product3_t operator&(domain_product3_t other) {
    return domain_product3_t(this->_product & other._product);
  }

  domain_product3_t operator&&(domain_product3_t other) {
    return domain_product3_t(this->_product && other._product);
  }

  void write(std::ostream& o) {
    if (this->is_bottom()) {
      o << "_|_";
    } else {
      o << "(" << this->first() << ", " << this->second() << ", "
        << this->third() << ")";
    }
  }

  static std::string domain_name() {
    return "Product " + Domain1::domain_name() + " x " +
           Domain2::domain_name() + " x " + Domain3::domain_name();
  }

}; // end class domain_product3

/// \brief Domain product of 2 numerical abstract domains
template < typename Number,
           typename VariableName,
           typename Domain1,
           typename Domain2 >
class numerical_domain_product2
    : public abstract_domain,
      public numerical_domain< Number, VariableName >,
      public bitwise_operators< Number, VariableName >,
      public division_operators< Number, VariableName > {
public:
  typedef numerical_domain_product2< Number, VariableName, Domain1, Domain2 >
      numerical_domain_product2_t;
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;

private:
  typedef domain_product2< Domain1, Domain2 > domain_product2_t;

private:
  domain_product2_t _product;

private:
  numerical_domain_product2(domain_product2_t product) : _product(product) {}

  void reduce() {
    if (this->_product.first().is_bottom() ||
        this->_product.second().is_bottom()) {
      _product = domain_product2_t::bottom();
    }
  }

public:
  static numerical_domain_product2_t top() {
    return numerical_domain_product2_t(domain_product2_t::top());
  }

  static numerical_domain_product2_t bottom() {
    return numerical_domain_product2_t(domain_product2_t::bottom());
  }

public:
  numerical_domain_product2() : _product() {}

  numerical_domain_product2(Domain1 first, Domain2 second)
      : _product(domain_product2_t(first, second)) {}

  numerical_domain_product2(const numerical_domain_product2_t& other)
      : _product(other._product) {}

  numerical_domain_product2_t& operator=(numerical_domain_product2_t other) {
    this->_product = other._product;
    return *this;
  }

  bool is_bottom() { return this->_product.is_bottom(); }

  bool is_top() { return this->_product.is_top(); }

  Domain1& first() { return this->_product.first(); }

  Domain2& second() { return this->_product.second(); }

  bool operator<=(numerical_domain_product2_t other) {
    return this->_product <= other._product;
  }

  bool operator==(numerical_domain_product2_t other) {
    return this->_product == other._product;
  }

  numerical_domain_product2_t operator|(numerical_domain_product2_t other) {
    return numerical_domain_product2_t(this->_product | other._product);
  }

  numerical_domain_product2_t operator||(numerical_domain_product2_t other) {
    return numerical_domain_product2_t(this->_product || other._product);
  }

  numerical_domain_product2_t join_loop(numerical_domain_product2_t other) {
    return numerical_domain_product2_t(
        this->_product.join_loop(other._product));
  }

  numerical_domain_product2_t join_iter(numerical_domain_product2_t other) {
    return numerical_domain_product2_t(
        this->_product.join_iter(other._product));
  }

  numerical_domain_product2_t operator&(numerical_domain_product2_t other) {
    return numerical_domain_product2_t(this->_product & other._product);
  }

  numerical_domain_product2_t operator&&(numerical_domain_product2_t other) {
    return numerical_domain_product2_t(this->_product && other._product);
  }

  void assign(VariableName x, linear_expression_t e) {
    this->_product.first().assign(x, e);
    this->_product.second().assign(x, e);
  }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    this->_product.first().apply(op, x, y, z);
    this->_product.second().apply(op, x, y, z);
    this->reduce();
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    this->_product.first().apply(op, x, y, k);
    this->_product.second().apply(op, x, y, k);
    this->reduce();
  }

  // bitwise_operators_api

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             uint64_t from,
             uint64_t to) {
    this->_product.first().apply(op, x, y, from, to);
    this->_product.second().apply(op, x, y, from, to);
    this->reduce();
  }

  void apply(conv_operation_t op,
             VariableName x,
             Number k,
             uint64_t from,
             uint64_t to) {
    this->_product.first().apply(op, x, k, from, to);
    this->_product.second().apply(op, x, k, from, to);
    this->reduce();
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    this->_product.first().apply(op, x, y, z);
    this->_product.second().apply(op, x, y, z);
    this->reduce();
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    this->_product.first().apply(op, x, y, k);
    this->_product.second().apply(op, x, y, k);
    this->reduce();
  }

  // division_operators_api

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    this->_product.first().apply(op, x, y, z);
    this->_product.second().apply(op, x, y, z);
    this->reduce();
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    this->_product.first().apply(op, x, y, k);
    this->_product.second().apply(op, x, y, k);
    this->reduce();
  }

  void operator+=(linear_constraint_system_t csts) {
    this->_product.first() += csts;
    this->_product.second() += csts;
    this->reduce();
  }

  void operator-=(VariableName v) {
    this->_product.first() -= v;
    this->_product.second() -= v;
  }

  void forget(VariableName v) {
    this->_product.first().forget(v);
    this->_product.second().forget(v);
  }

  void forget_num(VariableName v) {
    this->_product.first().forget_num(v);
    this->_product.second().forget_num(v);
  }

  template < typename Iterator >
  void forget(Iterator begin, Iterator end) {
    this->_product.first().forget(begin, end);
    this->_product.second().forget(begin, end);
  }

  template < typename Iterator >
  void forget_num(Iterator begin, Iterator end) {
    this->_product.first().forget_num(begin, end);
    this->_product.second().forget_num(begin, end);
  }

  linear_constraint_system_t to_linear_constraint_system() {
    linear_constraint_system_t csts;

    if (this->is_bottom()) {
      csts += linear_constraint_t::contradiction();
      return csts;
    }

    csts += this->_product.first().to_linear_constraint_system();
    csts += this->_product.second().to_linear_constraint_system();
    return csts;
  }

  void write(std::ostream& o) { this->_product.write(o); }

  static std::string domain_name() {
    return "Numerical Product " + Domain1::domain_name() + " x " +
           Domain2::domain_name();
  }

}; // end class numerical_domain_product2

/// \brief Domain product of 3 numerical abstract domains
template < typename Number,
           typename VariableName,
           typename Domain1,
           typename Domain2,
           typename Domain3 >
class numerical_domain_product3
    : public abstract_domain,
      public numerical_domain< Number, VariableName >,
      public bitwise_operators< Number, VariableName >,
      public division_operators< Number, VariableName > {
public:
  typedef numerical_domain_product3< Number,
                                     VariableName,
                                     Domain1,
                                     Domain2,
                                     Domain3 > numerical_domain_product3_t;
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;

private:
  typedef numerical_domain_product2< Number, VariableName, Domain2, Domain3 >
      product23_t;
  typedef numerical_domain_product2< Number,
                                     VariableName,
                                     Domain1,
                                     product23_t > product123_t;

private:
  product123_t _product;

private:
  numerical_domain_product3(product123_t product) : _product(product) {}

public:
  static numerical_domain_product3_t top() {
    return numerical_domain_product3_t(product123_t::top());
  }

  static numerical_domain_product3_t bottom() {
    return numerical_domain_product3_t(product123_t::bottom());
  }

public:
  numerical_domain_product3() : _product() {}

  numerical_domain_product3(Domain1 first, Domain2 second, Domain3 third)
      : _product(product123_t(first, product23_t(second, third))) {}

  numerical_domain_product3(const numerical_domain_product3_t& other)
      : _product(other._product) {}

  numerical_domain_product3_t& operator=(numerical_domain_product3_t other) {
    this->_product = other._product;
    return *this;
  }

  bool is_bottom() { return this->_product.is_bottom(); }

  bool is_top() { return this->_product.is_top(); }

  Domain1& first() { return this->_product.first(); }

  Domain2& second() { return this->_product.second().first(); }

  Domain3& third() { return this->_product.second().second(); }

  bool operator<=(numerical_domain_product3_t other) {
    return (this->_product <= other._product);
  }

  bool operator==(numerical_domain_product3_t other) {
    return (this->_product == other._product);
  }

  numerical_domain_product3_t operator|(numerical_domain_product3_t other) {
    return numerical_domain_product3_t(this->_product | other._product);
  }

  numerical_domain_product3_t operator||(numerical_domain_product3_t other) {
    return numerical_domain_product3_t(this->_product || other._product);
  }

  numerical_domain_product3_t join_loop(numerical_domain_product3_t other) {
    return numerical_domain_product3_t(
        this->_product.join_loop(other._product));
  }

  numerical_domain_product3_t join_iter(numerical_domain_product3_t other) {
    return numerical_domain_product3_t(
        this->_product.join_iter(other._product));
  }

  numerical_domain_product3_t operator&(numerical_domain_product3_t other) {
    return numerical_domain_product3_t(this->_product & other._product);
  }

  numerical_domain_product3_t operator&&(numerical_domain_product3_t other) {
    return numerical_domain_product3_t(this->_product && other._product);
  }

  void assign(VariableName x, linear_expression_t e) {
    this->_product.assign(x, e);
  }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    this->_product.apply(op, x, y, z);
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    this->_product.apply(op, x, y, k);
  }

  // bitwise_operators_api

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             uint64_t from,
             uint64_t to) {
    this->_product.apply(op, x, y, from, to);
  }

  void apply(conv_operation_t op,
             VariableName x,
             Number k,
             uint64_t from,
             uint64_t to) {
    this->_product.apply(op, x, k, from, to);
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    this->_product.apply(op, x, y, z);
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    this->_product.apply(op, x, y, k);
  }

  // division_operators_api

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    this->_product.apply(op, x, y, z);
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    this->_product.apply(op, x, y, k);
  }

  void operator+=(linear_constraint_system_t csts) { this->_product += csts; }

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

  linear_constraint_system_t to_linear_constraint_system() {
    return this->_product.to_linear_constraint_system();
  }

  void write(std::ostream& o) {
    if (this->is_bottom()) {
      o << "_|_";
    } else {
      o << "(" << this->first() << ", " << this->second() << ", "
        << this->third() << ")";
    }
  }

  static std::string domain_name() {
    return "Numerical Product " + Domain1::domain_name() + " x " +
           Domain2::domain_name() + " x " + Domain3::domain_name();
  }

}; // end class numerical_domain_product3

} // end namespace ikos

#endif // IKOS_DOMAIN_PRODUCTS_HPP
