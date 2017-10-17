/**************************************************************************/ /**
 *
 * \file
 * \brief Standard domain of intervals.
 *
 * The resolution of a system of linear constraints over the domain of intervals
 * is based on W. Harvey & P. J. Stuckey's paper: Improving linear constraint
 * propagation by changing constraint representation, in Constraints,
 * 8(2):173–207, 2003.
 *
 * Author: Arnaud J. Venet
 *
 * Contributors:
 *   * Alexandre C. D. Wimmers
 *   * Maxime Arthaud
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

#ifndef IKOS_INTERVALS_HPP
#define IKOS_INTERVALS_HPP

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/optional.hpp>

#include <ikos/algorithms/linear_constraints.hpp>
#include <ikos/common/types.hpp>
#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/domains/bitwise_operators_api.hpp>
#include <ikos/domains/division_operators_api.hpp>
#include <ikos/domains/numerical_domains_api.hpp>
#include <ikos/domains/separate_domains.hpp>
#include <ikos/number.hpp>
#include <ikos/value/congruence.hpp>
#include <ikos/value/interval.hpp>

namespace ikos {

namespace intervals_impl {

template < typename Number >
inline interval< Number > trim_bound(interval< Number > i, Number c);

template <>
inline z_interval trim_bound(z_interval i, z_number c) {
  if (i.lb() == c) {
    return z_interval(c + 1, i.ub());
  } else if (i.ub() == c) {
    return z_interval(i.lb(), c - 1);
  } else {
    return i;
  }
}

template <>
inline q_interval trim_bound(q_interval i, q_number /* c */) {
  // No refinement possible for disequations over rational numbers
  return i;
}

} // end namespace intervals_impl

/// \brief Linear interval solver
template < typename Number, typename VariableName, typename NumDomain >
class linear_interval_solver {
public:
  typedef interval< Number > interval_t;
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;

private:
  typedef std::vector< linear_constraint_t > cst_table_t;
  typedef std::unordered_set< unsigned int > uint_set_t;
  typedef std::unordered_map< VariableName, uint_set_t > trigger_table_t;
  typedef typename linear_constraint_t::variable_set_t variable_set_t;

private:
  class bottom_found {};

private:
  std::size_t _max_cycles;
  std::size_t _max_op;
  bool _is_contradiction;
  bool _is_large_system;
  cst_table_t _cst_table;
  trigger_table_t _trigger_table;
  variable_set_t _refined_variables;
  std::size_t _op_count;

private:
  static const std::size_t _large_system_cst_threshold = 3;
  // cost of one propagation cycle for a dense 3x3 system of constraints
  static const std::size_t _large_system_op_threshold = 27;

private:
  void refine(variable_t v, interval_t i, NumDomain& env) {
    VariableName n = v.name();
    interval_t old_i = num_domain_traits::to_interval(env, n, false);
    interval_t new_i = old_i & i;
    if (new_i.is_bottom()) {
      throw bottom_found();
    }
    if (old_i != new_i) {
      num_domain_traits::refine_interval(env, n, new_i);
      this->_refined_variables += v;
      ++(this->_op_count);
    }
  }

  interval_t compute_residual(linear_constraint_t cst,
                              variable_t pivot,
                              NumDomain& env) {
    interval_t residual(cst.constant());
    for (typename linear_constraint_t::iterator it = cst.begin();
         it != cst.end();
         ++it) {
      variable_t v = it->second;
      if (index_traits< variable_t >::index(v) !=
          index_traits< variable_t >::index(pivot)) {
        residual =
            residual -
            (it->first * num_domain_traits::to_interval(env, v.name(), false));
        ++(this->_op_count);
      }
    }
    return residual;
  }

  void propagate(linear_constraint_t cst, NumDomain& env) {
    for (typename linear_constraint_t::iterator it = cst.begin();
         it != cst.end();
         ++it) {
      Number c = it->first;
      variable_t pivot = it->second;
      interval_t rhs = this->compute_residual(cst, pivot, env) / interval_t(c);
      if (cst.is_equality()) {
        this->refine(pivot, rhs, env);
      } else if (cst.is_inequality()) {
        if (c > 0) {
          refine(pivot, rhs.lower_half_line(), env);
        } else {
          refine(pivot, rhs.upper_half_line(), env);
        }
      } else {
        // cst is a disequation
        boost::optional< Number > c = rhs.singleton();
        if (c) {
          interval_t old_i =
              num_domain_traits::to_interval(env, pivot.name(), false);
          interval_t new_i = intervals_impl::trim_bound< Number >(old_i, *c);
          if (new_i.is_bottom()) {
            throw bottom_found();
          }
          if (old_i != new_i) {
            num_domain_traits::refine_interval(env, pivot.name(), new_i);
            this->_refined_variables += pivot;
          }
          ++(this->_op_count);
        }
      }
    }
  }

  void solve_large_system(NumDomain& env) {
    this->_op_count = 0;
    this->_refined_variables.clear();
    for (typename cst_table_t::iterator it = this->_cst_table.begin();
         it != this->_cst_table.end();
         ++it) {
      this->propagate(*it, env);
    }
    do {
      variable_set_t vars_to_process(this->_refined_variables);
      this->_refined_variables.clear();
      for (typename variable_set_t::iterator it = vars_to_process.begin();
           it != vars_to_process.end();
           ++it) {
        uint_set_t& csts = this->_trigger_table[it->name()];
        for (typename uint_set_t::iterator cst_it = csts.begin();
             cst_it != csts.end();
             ++cst_it) {
          this->propagate(this->_cst_table.at(*cst_it), env);
        }
      }
    } while (!this->_refined_variables.empty() &&
             this->_op_count <= this->_max_op);
  }

  void solve_small_system(NumDomain& env) {
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
  linear_interval_solver(linear_constraint_system_t csts,
                         std::size_t max_cycles)
      : _max_cycles(max_cycles),
        _is_contradiction(false),
        _is_large_system(false),
        _op_count(0) {
    std::size_t op_per_cycle = 0;
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
        std::size_t cst_size = cst.size();
        this->_cst_table.push_back(cst);
        // cost of one reduction step on the constraint in terms
        // of accesses to the interval collection
        op_per_cycle += cst_size * cst_size;
      }
    }

    this->_is_large_system =
        (this->_cst_table.size() > _large_system_cst_threshold) ||
        (op_per_cycle > _large_system_op_threshold);

    if (!this->_is_contradiction && this->_is_large_system) {
      this->_max_op = op_per_cycle * max_cycles;
      for (unsigned int i = 0; i < this->_cst_table.size(); ++i) {
        linear_constraint_t cst = this->_cst_table.at(i);
        variable_set_t vars = cst.variables();
        for (typename variable_set_t::iterator it = vars.begin();
             it != vars.end();
             ++it) {
          this->_trigger_table[it->name()].insert(i);
        }
      }
    }
  }

  void run(NumDomain& env) {
    if (this->_is_contradiction) {
      env = NumDomain::bottom();
    } else {
      try {
        if (this->_is_large_system) {
          this->solve_large_system(env);
        } else {
          this->solve_small_system(env);
        }
      } catch (bottom_found& e) {
        env = NumDomain::bottom();
      }
    }
  }

}; // end class linear_interval_solver

/// \brief Interval abstract domain
template < typename Number,
           typename VariableName,
           std::size_t max_reduction_cycles = 10 >
class interval_domain : public abstract_domain,
                        public numerical_domain< Number, VariableName >,
                        public bitwise_operators< Number, VariableName >,
                        public division_operators< Number, VariableName > {
public:
  typedef interval< Number > interval_t;
  typedef congruence< Number > congruence_t;
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;
  typedef interval_domain< Number, VariableName > interval_domain_t;

private:
  typedef separate_domain< VariableName, interval_t > separate_domain_t;
  typedef linear_interval_solver< Number, VariableName, interval_domain_t >
      solver_t;

public:
  typedef typename separate_domain_t::iterator iterator;

private:
  separate_domain_t _env;

private:
  interval_domain(separate_domain_t env) : _env(env) {}

public:
  static interval_domain_t top() {
    return interval_domain(separate_domain_t::top());
  }

  static interval_domain_t bottom() {
    return interval_domain(separate_domain_t::bottom());
  }

public:
  interval_domain() : _env(separate_domain_t::top()) {}

  interval_domain(const interval_domain_t& e) : _env(e._env) {}

  interval_domain_t& operator=(interval_domain_t e) {
    this->_env = e._env;
    return *this;
  }

  iterator begin() { return this->_env.begin(); }

  iterator end() { return this->_env.end(); }

  bool is_bottom() { return this->_env.is_bottom(); }

  bool is_top() { return this->_env.is_top(); }

  bool operator<=(interval_domain_t e) { return this->_env <= e._env; }

  interval_domain_t operator|(interval_domain_t e) {
    return this->_env | e._env;
  }

  interval_domain_t operator||(interval_domain_t e) {
    return this->_env || e._env;
  }

  interval_domain_t join_loop(interval_domain_t e) {
    return this->operator|(e);
  }

  interval_domain_t join_iter(interval_domain_t e) {
    return this->operator|(e);
  }

  interval_domain_t operator&(interval_domain_t e) {
    return this->_env & e._env;
  }

  interval_domain_t operator&&(interval_domain_t e) {
    return this->_env && e._env;
  }

  void set(VariableName v, interval_t i) { this->_env.set(v, i); }

  void refine(VariableName v, interval_t i) {
    this->_env.set(v, i & this->_env[v]);
  }

  void set(VariableName v, Number n) { this->_env.set(v, interval_t(n)); }

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

  congruence_t to_congruence(VariableName v) {
    if (this->is_bottom()) {
      return congruence_t::bottom();
    }

    interval_t var_interval = this->_env[v];
    if (var_interval.singleton())
      return congruence_t(Number(0), *var_interval.singleton());
    else {
      return congruence_t::top();
    }
  }

  interval_t operator[](VariableName v) { return this->_env[v]; }

  interval_t operator[](linear_expression_t expr) {
    interval_t r(expr.constant());
    for (typename linear_expression_t::iterator it = expr.begin();
         it != expr.end();
         ++it) {
      interval_t c(it->first);
      r += c * this->_env[it->second.name()];
    }
    return r;
  }

  void operator+=(linear_constraint_system_t csts) { this->add(csts); }

  void add(linear_constraint_system_t csts,
           std::size_t threshold = max_reduction_cycles) {
    if (!this->is_bottom()) {
      solver_t solver(csts, threshold);
      solver.run(*this);
    }
  }

  interval_domain_t operator+(linear_constraint_system_t csts) {
    interval_domain_t e(this->_env);
    e += csts;
    return e;
  }

  void assign(VariableName x, linear_expression_t e) {
    interval_t r = e.constant();
    for (typename linear_expression_t::iterator it = e.begin(); it != e.end();
         ++it) {
      r += it->first * this->_env[it->second.name()];
    }
    this->_env.set(x, r);
  }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    interval_t yi = this->_env[y];
    interval_t zi = this->_env[z];
    interval_t xi = interval_t::bottom();

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
    interval_t yi = this->_env[y];
    interval_t zi(k);
    interval_t xi = interval_t::bottom();

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
    interval_t yi = this->_env[y];
    interval_t xi = interval_t::bottom();

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
    interval_t yi(k);
    interval_t xi = interval_t::bottom();

    switch (op) {
      case OP_TRUNC:
        xi = yi.Trunc(from, to);
        break;
      case OP_ZEXT:
        xi = yi.ZExt(from, to);
        break;
      case OP_SEXT:
        xi = yi.SExt(from, to);
        break;
      default:
        ikos_unreachable("invalid operation");
    }
    this->_env.set(x, xi);
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    interval_t yi = this->_env[y];
    interval_t zi = this->_env[z];
    interval_t xi = interval_t::bottom();

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
    interval_t yi = this->_env[y];
    interval_t zi(k);
    interval_t xi = interval_t::bottom();

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
    interval_t yi = this->_env[y];
    interval_t zi = this->_env[z];
    interval_t xi = interval_t::bottom();

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
    interval_t yi = this->_env[y];
    interval_t zi(k);
    interval_t xi = interval_t::bottom();

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
      csts += within_interval(it->first, it->second);
    }

    return csts;
  }

  static std::string domain_name() { return "Intervals"; }

}; // end class interval_domain

} // end namespace ikos

#endif // IKOS_INTERVALS_HPP
