/**************************************************************************/ /**
 *
 * \file
 * \brief Standard domain of numerical congruences extended with bitwise
 * operations.
 *
 * Bitwise extensions based on Stefan Bygde's paper: Static WCET
 * analysis based on abstract interpretation and counting of elements,
 * Vasteras : School of Innovation, Design and Engineering, Malardalen
 * University (2010)
 *
 * Comments:
 * - Assume that no overflow/underflow can occur.
 * - Bitwise operations are sound approximations for both signed and
 *   unsigned interpretations of bit strings.
 * - If TypeSize == -1 then the domain assumes that all variables have
 *   unlimited precision
 * - If TypeSize > -1 the domain assumes that all variables will
 *   have the same bit width TypeSize.
 *
 * Author: Alexandre C. D. Wimmers
 *
 * Contributors: Jorge A. Navas
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

#ifndef IKOS_CONGRUENCES_HPP
#define IKOS_CONGRUENCES_HPP

#include <iostream>
#include <vector>

#include <ikos/common/types.hpp>
#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/domains/bitwise_operators_api.hpp>
#include <ikos/domains/division_operators_api.hpp>
#include <ikos/domains/numerical_domains_api.hpp>
#include <ikos/domains/separate_domains.hpp>
#include <ikos/value/congruence.hpp>
#include <ikos/value/interval.hpp>

namespace ikos {

template < typename Number,
           typename VariableName,
           typename CongruenceCollection,
           int TypeSize = -1 >
class equality_congruence_solver {
  // TODO: check correctness of the solver. Granger provides a sound
  // and more precise solver for equality linear congruences (see
  // Theorem 4.4).
private:
  typedef congruence< Number, TypeSize > congruence_t;
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;

private:
  typedef std::vector< linear_constraint_t > cst_table_t;
  typedef typename linear_constraint_t::variable_set_t variable_set_t;

private:
  class bottom_found {};

private:
  std::size_t _max_cycles;
  bool _is_contradiction;
  cst_table_t _cst_table;
  variable_set_t _refined_variables;
  std::size_t _op_count;

private:
  void refine(variable_t v, congruence_t i, CongruenceCollection& env) {
    VariableName n = v.name();
    congruence_t old_i = env[n];
    congruence_t new_i = old_i & i;
    if (new_i.is_bottom()) {
      throw bottom_found();
    }
    if (old_i != new_i) {
      env.set(n, new_i);
      this->_refined_variables += v;
      ++(this->_op_count);
    }
  }

  congruence_t compute_residual(linear_constraint_t cst,
                                variable_t pivot,
                                CongruenceCollection& env) {
    congruence_t residual(cst.constant());
    for (typename linear_constraint_t::iterator it = cst.begin();
         it != cst.end();
         ++it) {
      variable_t v = it->second;
      if (index_traits< variable_t >::index(v) !=
          index_traits< variable_t >::index(pivot)) {
        residual = residual - (it->first * env[v.name()]);
        ++(this->_op_count);
      }
    }
    return residual;
  }

  void propagate(linear_constraint_t cst, CongruenceCollection& env) {
    for (typename linear_constraint_t::iterator it = cst.begin();
         it != cst.end();
         ++it) {
      Number c = it->first;
      variable_t pivot = it->second;
      congruence_t rhs =
          this->compute_residual(cst, pivot, env) / congruence_t(c);

      if (cst.is_equality()) {
        this->refine(pivot, rhs, env);
      } else if (cst.is_inequality()) {
        // Inequations (>=, <=, >, and <) do not work well with
        // congruences because for any number n there is always x and y
        // \in gamma(aZ+b) such that n < x and n > y.
        //
        // The only cases we can catch is when all the expressions
        // are constants. We do not bother because any product
        // with intervals or constants should get those cases.
        continue;
      } else {
        // TODO: cst is a disequation
      }
    }
  }

  void solve_system(CongruenceCollection& env) {
    std::size_t cycle = 0;
    do {
      ++cycle;
      this->_refined_variables.clear();
      for (typename cst_table_t::iterator it = this->_cst_table.begin();
           it != this->_cst_table.end();
           ++it) {
        this->propagate(*it, env);
      }
    } while (!this->_refined_variables.empty() && cycle <= this->_max_cycles);
  }

public:
  equality_congruence_solver(linear_constraint_system_t csts,
                             std::size_t max_cycles)
      : _max_cycles(max_cycles), _is_contradiction(false) {
    for (typename linear_constraint_system_t::iterator it = csts.begin();
         it != csts.end();
         ++it) {
      linear_constraint_t cst = *it;
      if (cst.is_contradiction()) {
        this->_is_contradiction = true;
        return;
      } else if (cst.is_tautology()) {
        continue;
      } else {
        this->_cst_table.push_back(cst);
      }
    }
  }

  void run(CongruenceCollection& env) {
    if (this->_is_contradiction) {
      env.set_to_bottom();
    } else {
      try {
        this->solve_system(env);
      } catch (bottom_found& e) {
        env.set_to_bottom();
      }
    }
  }

}; // end class equality_congruence_solver

/// \brief Congruence abstract domain
///
/// TypeSize = -1 means unlimited precision
template < typename Number, typename VariableName, int TypeSize = -1 >
class congruence_domain : public abstract_domain,
                          public numerical_domain< Number, VariableName >,
                          public bitwise_operators< Number, VariableName >,
                          public division_operators< Number, VariableName > {
public:
  typedef congruence< Number, TypeSize > congruence_t;
  typedef congruence_domain< Number, VariableName, TypeSize >
      congruence_domain_t;

public:
  // note that this is assuming that all variables have the same bit
  // width which is unrealistic.
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;

private:
  typedef separate_domain< VariableName, congruence_t > separate_domain_t;
  typedef equality_congruence_solver< Number,
                                      VariableName,
                                      separate_domain_t,
                                      TypeSize > solver_t;

public:
  typedef typename separate_domain_t::iterator iterator;

private:
  separate_domain_t _env;

private:
  congruence_domain(separate_domain_t env) : _env(env) {}

public:
  static congruence_domain_t top() {
    return congruence_domain(separate_domain_t::top());
  }

  static congruence_domain_t bottom() {
    return congruence_domain(separate_domain_t::bottom());
  }

public:
  congruence_domain() : _env(separate_domain_t::top()) {}

  congruence_domain(const congruence_domain_t& e) : _env(e._env) {}

  congruence_domain_t& operator=(congruence_domain_t e) {
    this->_env = e._env;
    return *this;
  }

  iterator begin() { return this->_env.begin(); }

  iterator end() { return this->_env.end(); }

  bool is_bottom() { return this->_env.is_bottom(); }

  bool is_top() { return this->_env.is_top(); }

  bool operator<=(congruence_domain_t e) { return this->_env <= e._env; }

  congruence_domain_t operator|(congruence_domain_t e) {
    return this->_env | e._env;
  }

  congruence_domain_t operator||(congruence_domain_t e) {
    return this->_env || e._env;
  }

  congruence_domain_t join_loop(congruence_domain_t e) {
    return this->operator|(e);
  }

  congruence_domain_t join_iter(congruence_domain_t e) {
    return this->operator|(e);
  }

  congruence_domain_t operator&(congruence_domain_t e) {
    return this->_env & e._env;
  }

  congruence_domain_t operator&&(congruence_domain_t e) {
    return this->_env && e._env;
  }

  void set(VariableName v, congruence_t i) { this->_env.set(v, i); }

  void set(VariableName v, Number n) { this->_env.set(v, congruence_t(n)); }

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

  congruence_t operator[](VariableName v) { return this->_env[v]; }

  congruence_t to_congruence(VariableName v) { return this->_env[v]; }

  congruence_t operator[](linear_expression_t expr) {
    congruence_t r(expr.constant());
    for (typename linear_expression_t::iterator it = expr.begin();
         it != expr.end();
         ++it) {
      congruence_t c(it->first);
      r = r + (c * this->_env[it->second.name()]);
    }
    return r;
  }

  void add(linear_constraint_system_t csts) {
    const std::size_t threshold = 10;
    if (!this->is_bottom()) {
      solver_t solver(csts, threshold);
      solver.run(this->_env);
    }
  }

  void operator+=(linear_constraint_system_t csts) { this->add(csts); }

  congruence_domain_t operator+(linear_constraint_system_t csts) {
    congruence_domain_t e(this->_env);
    e += csts;
    return e;
  }

  void assign(VariableName x, linear_expression_t e) {
    congruence_t r = e.constant();
    for (typename linear_expression_t::iterator it = e.begin(); it != e.end();
         ++it) {
      r = r + (it->first * this->_env[it->second.name()]);
    }
    this->_env.set(x, r);
  }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    congruence_t yi = this->_env[y];
    congruence_t zi = this->_env[z];
    congruence_t xi = congruence_t::bottom();

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
      case OP_DIVISION: { // signed division
        xi = yi / zi;
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }
    this->_env.set(x, xi);
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    congruence_t yi = this->_env[y];
    congruence_t zi(k);
    congruence_t xi = congruence_t::bottom();

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
      case OP_DIVISION: { // signed division
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
    congruence_t yi = this->_env[y];
    congruence_t xi = congruence_t::bottom();

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
    congruence_t yi(k);
    congruence_t xi = congruence_t::bottom();

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
    congruence_t yi = this->_env[y];
    congruence_t zi = this->_env[z];
    congruence_t xi = congruence_t::bottom();

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
    congruence_t yi = this->_env[y];
    congruence_t zi(k);
    congruence_t xi = congruence_t::bottom();

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
    congruence_t yi = this->_env[y];
    congruence_t zi = this->_env[z];
    congruence_t xi = congruence_t::bottom();

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
    congruence_t yi = this->_env[y];
    congruence_t zi(k);
    congruence_t xi = congruence_t::bottom();

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
      congruence_t c = it->second;
      boost::optional< Number > n = c.singleton();
      if (n) {
        csts += linear_constraint_t(variable_t(v) == *n);
      }
    }
    return csts;
  }

  static std::string domain_name() { return "Congruences"; }

}; // end class congruence_domain

namespace num_domain_traits {

namespace detail {

template < typename Number, typename VariableName, int TypeSize >
struct var_to_interval_impl<
    congruence_domain< Number, VariableName, TypeSize > > {
  inline interval< Number > operator()(
      congruence_domain< Number, VariableName, TypeSize >& inv,
      VariableName v,
      bool /*normalize*/) {
    congruence< Number > c = inv[v];

    if (c.is_bottom()) {
      return interval< Number >::bottom();
    } else if (!c.singleton()) {
      return interval< Number >::top();
    } else {
      return interval< Number >(*c.singleton());
    }
  }
};

template < typename Number, typename VariableName, int TypeSize >
struct from_interval_impl<
    congruence_domain< Number, VariableName, TypeSize > > {
  inline void operator()(
      congruence_domain< Number, VariableName, TypeSize >& inv,
      VariableName v,
      interval< Number > i) {
    if (i.is_bottom()) {
      inv.set(v, congruence< Number >::bottom());
    } else if (!i.singleton()) {
      inv.set(v, congruence< Number >::top());
    } else {
      inv.set(v, congruence< Number >(*i.singleton()));
    }
  }
};

} // end namespace detail
} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_CONGRUENCES_HPP
