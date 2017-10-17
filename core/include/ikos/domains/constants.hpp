/**************************************************************************/ /**
 *
 * \file
 * \brief Standard domain of constants.
 *
 * Author: Arnaud J. Venet
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

#ifndef IKOS_CONSTANTS_HPP
#define IKOS_CONSTANTS_HPP

#include <boost/optional.hpp>

#include <ikos/common/types.hpp>
#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/domains/bitwise_operators_api.hpp>
#include <ikos/domains/division_operators_api.hpp>
#include <ikos/domains/intervals.hpp>
#include <ikos/domains/numerical_domains_api.hpp>
#include <ikos/domains/separate_domains.hpp>
#include <ikos/value/constant.hpp>

namespace ikos {

/// \brief Constant abstract domain
template < typename Number,
           typename VariableName,
           std::size_t max_reduction_cycles = 10 >
class constant_domain : public abstract_domain,
                        public numerical_domain< Number, VariableName >,
                        public bitwise_operators< Number, VariableName >,
                        public division_operators< Number, VariableName > {
public:
  typedef constant< Number > constant_t;
  typedef constant_domain< Number, VariableName > constant_domain_t;
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;

private:
  typedef separate_domain< VariableName, constant_t > separate_domain_t;
  typedef interval< Number > interval_t;
  typedef interval_domain< Number, VariableName > interval_domain_t;
  typedef typename linear_constraint_system_t::variable_set_t variable_set_t;

public:
  typedef typename separate_domain_t::iterator iterator;

private:
  separate_domain_t _env;

private:
  constant_domain(separate_domain_t env) : _env(env) {}

public:
  static constant_domain_t top() {
    return constant_domain(separate_domain_t::top());
  }

  static constant_domain_t bottom() {
    return constant_domain(separate_domain_t::bottom());
  }

public:
  constant_domain() : _env(separate_domain_t::top()) {}

  constant_domain(const constant_domain_t& e) : _env(e._env) {}

  constant_domain_t& operator=(constant_domain_t e) {
    this->_env = e._env;
    return *this;
  }

  iterator begin() { return this->_env.begin(); }

  iterator end() { return this->_env.end(); }

  bool is_bottom() { return this->_env.is_bottom(); }

  bool is_top() { return this->_env.is_top(); }

  bool operator<=(constant_domain_t e) { return this->_env <= e._env; }

  bool operator==(constant_domain_t e) { return this->_env == e._env; }

  constant_domain_t operator|(constant_domain_t e) {
    return this->_env | e._env;
  }

  constant_domain_t operator||(constant_domain_t e) {
    return this->_env || e._env;
  }

  constant_domain_t join_loop(constant_domain_t e) {
    return this->operator|(e);
  }

  constant_domain_t join_iter(constant_domain_t e) {
    return this->operator|(e);
  }

  constant_domain_t operator&(constant_domain_t e) {
    return this->_env & e._env;
  }

  constant_domain_t operator&&(constant_domain_t e) {
    return this->_env && e._env;
  }

  void set(VariableName v, constant_t i) { this->_env.set(v, i); }

  void set(VariableName v, Number n) { this->_env.set(v, constant_t(n)); }

  void operator-=(VariableName v) { forget_num(v); }

  void forget(VariableName v) { forget_num(v); }

  void forget_num(VariableName v) { this->_env -= v; }

  template < typename Iterator >
  void forget(Iterator begin, Iterator end) {
    forget_num(begin, end);
  }

  template < typename Iterator >
  void forget_num(Iterator begin, Iterator end) {
    for (auto it = begin; it != end; ++it) {
      this->_env -= *it;
    }
  }

  constant_t operator[](VariableName v) { return _env[v]; }

  constant_t operator[](linear_expression_t expr) {
    constant_t r(expr.constant());
    for (typename linear_expression_t::iterator it = expr.begin();
         it != expr.end();
         ++it) {
      constant_t c(it->first);
      r += c * this->_env[it->second.name()];
    }
    return r;
  }

  void operator+=(linear_constraint_system_t csts) { this->add(csts); }

  void add(linear_constraint_system_t csts,
           std::size_t threshold = max_reduction_cycles) {
    if (!this->is_bottom()) {
      interval_domain_t e;
      variable_set_t vars = csts.variables();
      for (typename variable_set_t::iterator it = vars.begin();
           it != vars.end();
           ++it) {
        VariableName n = it->name();
        constant_t c = this->operator[](n);
        boost::optional< Number > v = c.number();
        if (v) {
          e.set(n, *v);
        }
      }
      e.add(csts, threshold);
      for (typename variable_set_t::iterator it = vars.begin();
           it != vars.end();
           ++it) {
        VariableName n = it->name();
        interval_t i = e[n];
        if (i.is_bottom()) {
          this->set(n, constant_t::bottom());
          return;
        } else {
          boost::optional< Number > v = i.singleton();
          if (v) {
            this->set(n, *v);
          }
        }
      }
    }
  }

  constant_domain_t operator+(linear_constraint_system_t csts) {
    constant_domain_t e(this->_env);
    e += csts;
    return e;
  }

  void assign(VariableName x, linear_expression_t e) {
    constant_t r = e.constant();
    for (typename linear_expression_t::iterator it = e.begin(); it != e.end();
         ++it) {
      r += it->first * this->_env[it->second.name()];
    }
    this->_env.set(x, r);
  }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    constant_t yi = this->_env[y];
    constant_t zi = this->_env[z];
    constant_t xi = constant_t::bottom();

    switch (op) {
      case OP_ADDITION: {
        xi = yi + zi;
        break;
      }
      case OP_SUBTRACTION: {
        xi = yi - zi;
        break;
      }
      case OP_MULTIPLICATION: {
        xi = yi * zi;
        break;
      }
      case OP_DIVISION: {
        xi = yi / zi;
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }
    this->_env.set(x, xi);
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    constant_t yi = this->_env[y];
    constant_t zi(k);
    constant_t xi = constant_t::bottom();

    switch (op) {
      case OP_ADDITION: {
        xi = yi + zi;
        break;
      }
      case OP_SUBTRACTION: {
        xi = yi - zi;
        break;
      }
      case OP_MULTIPLICATION: {
        xi = yi * zi;
        break;
      }
      case OP_DIVISION: {
        xi = yi / zi;
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }
    this->_env.set(x, xi);
  }

  // bitwise_operators_api

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             uint64_t from,
             uint64_t to) {
    constant_t yi = this->_env[y];
    constant_t xi = constant_t::bottom();

    switch (op) {
      case OP_TRUNC: {
        xi = yi.Trunc(from, to);
        break;
      }
      case OP_ZEXT: {
        xi = yi.ZExt(from, to);
        break;
      }
      case OP_SEXT: {
        xi = yi.SExt(from, to);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }
    this->_env.set(x, xi);
  }

  void apply(conv_operation_t op,
             VariableName x,
             Number k,
             uint64_t from,
             uint64_t to) {
    constant_t yi(k);
    constant_t xi = constant_t::bottom();

    switch (op) {
      case OP_TRUNC: {
        xi = yi.Trunc(from, to);
        break;
      }
      case OP_ZEXT: {
        xi = yi.ZExt(from, to);
        break;
      }
      case OP_SEXT: {
        xi = yi.SExt(from, to);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }
    this->_env.set(x, xi);
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    constant_t yi = this->_env[y];
    constant_t zi = this->_env[z];
    constant_t xi = constant_t::bottom();

    switch (op) {
      case OP_AND: {
        xi = yi.And(zi);
        break;
      }
      case OP_OR: {
        xi = yi.Or(zi);
        break;
      }
      case OP_XOR: {
        xi = yi.Xor(zi);
        break;
      }
      case OP_SHL: {
        xi = yi.Shl(zi);
        break;
      }
      case OP_LSHR: {
        xi = yi.LShr(zi);
        break;
      }
      case OP_ASHR: {
        xi = yi.AShr(zi);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }
    this->_env.set(x, xi);
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    constant_t yi = this->_env[y];
    constant_t zi(k);
    constant_t xi = constant_t::bottom();

    switch (op) {
      case OP_AND: {
        xi = yi.And(zi);
        break;
      }
      case OP_OR: {
        xi = yi.Or(zi);
        break;
      }
      case OP_XOR: {
        xi = yi.Xor(zi);
        break;
      }
      case OP_SHL: {
        xi = yi.Shl(zi);
        break;
      }
      case OP_LSHR: {
        xi = yi.LShr(zi);
        break;
      }
      case OP_ASHR: {
        xi = yi.AShr(zi);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }
    this->_env.set(x, xi);
  }

  // division_operators_api

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    constant_t yi = this->_env[y];
    constant_t zi = this->_env[z];
    constant_t xi = constant_t::bottom();

    switch (op) {
      case OP_SDIV: {
        xi = yi / zi;
        break;
      }
      case OP_UDIV: {
        xi = yi.UDiv(zi);
        break;
      }
      case OP_SREM: {
        xi = yi.SRem(zi);
        break;
      }
      case OP_UREM: {
        xi = yi.URem(zi);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }
    this->_env.set(x, xi);
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    constant_t yi = this->_env[y];
    constant_t zi(k);
    constant_t xi = constant_t::bottom();

    switch (op) {
      case OP_SDIV: {
        xi = yi / zi;
        break;
      }
      case OP_UDIV: {
        xi = yi.UDiv(zi);
        break;
      }
      case OP_SREM: {
        xi = yi.SRem(zi);
        break;
      }
      case OP_UREM: {
        xi = yi.URem(zi);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }
    this->_env.set(x, xi);
  }

  void write(std::ostream& o) { this->_env.write(o); }

  linear_constraint_system_t to_linear_constraint_system() {
    linear_constraint_system_t csts;

    if (is_bottom()) {
      csts += linear_constraint_t::contradiction();
      return csts;
    }

    for (iterator it = this->_env.begin(); it != this->_env.end(); ++it) {
      VariableName v = it->first;
      constant_t c = it->second;
      boost::optional< Number > n = c.number();
      if (n) {
        csts += linear_constraint_t(variable_t(v) == *n);
      }
    }
    return csts;
  }

  static std::string domain_name() { return "Constants"; }

}; // end class constant_domain

namespace num_domain_traits {
namespace detail {

template < typename Number,
           typename VariableName,
           std::size_t max_reduction_cycles >
struct var_to_interval_impl<
    constant_domain< Number, VariableName, max_reduction_cycles > > {
  inline interval< Number > operator()(
      constant_domain< Number, VariableName, max_reduction_cycles >& inv,
      VariableName v,
      bool /*normalize*/) {
    constant< Number > c = inv[v];

    if (c.is_bottom()) {
      return interval< Number >::bottom();
    } else if (c.is_top()) {
      return interval< Number >::top();
    } else {
      return interval< Number >(*c.number());
    }
  }
};

template < typename Number,
           typename VariableName,
           std::size_t max_reduction_cycles >
struct from_interval_impl<
    constant_domain< Number, VariableName, max_reduction_cycles > > {
  inline void operator()(
      constant_domain< Number, VariableName, max_reduction_cycles >& inv,
      VariableName v,
      interval< Number > i) {
    if (i.is_bottom()) {
      inv.set(v, constant< Number >::bottom());
    } else if (!i.singleton()) {
      inv.set(v, constant< Number >::top());
    } else {
      inv.set(v, constant< Number >(*i.singleton()));
    }
  }
};

} // end namespace detail
} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_CONSTANTS_HPP
