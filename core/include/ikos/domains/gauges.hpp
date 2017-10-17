/**************************************************************************/ /**
 *
 * \file
 * \brief Implementation of the Gauge Domain.
 *
 * Based on Arnaud Venet's paper: The Gauge Domain: Scalable Analysis of
 * Linear Inequality Invariants, in CAV, 129-154, 2012.
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

#ifndef IKOS_GAUGES_HPP
#define IKOS_GAUGES_HPP

#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/domains/bitwise_operators_api.hpp>
#include <ikos/domains/constants.hpp>
#include <ikos/domains/counter_domains_api.hpp>
#include <ikos/domains/division_operators_api.hpp>
#include <ikos/domains/numerical_domains_api.hpp>
#include <ikos/value/congruence.hpp>
#include <ikos/value/interval.hpp>

namespace ikos {

/// \brief Gauge bound
///
/// This is either -oo, +oo or a linear expression of the non-negative loop
/// counters.
template < typename Number, typename VariableName >
class gauge_bound {
public:
  typedef bound< Number > bound_t;
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef gauge_bound< Number, VariableName > gauge_bound_t;

private:
  typedef patricia_tree< VariableName, Number > coefficients_t;
  typedef typename coefficients_t::unary_op_t unary_op_t;
  typedef
      typename coefficients_t::generic_partial_order_t generic_partial_order_t;
  typedef typename coefficients_t::generic_binary_op_t generic_binary_op_t;

private:
  bool _is_infinite;
  Number _cst;
  coefficients_t _coeffs;

  /* Invariants:
   * _is_infinite => _cst in {-1, 1} and _coeffs.empty()
   */

private:
  gauge_bound() = delete;

  gauge_bound(bool is_infinite, Number cst, coefficients_t coeffs)
      : _is_infinite(is_infinite), _cst(cst), _coeffs(coeffs) {
    if (is_infinite) {
      this->_coeffs.clear();
      if (cst > 0) {
        this->_cst = 1;
      } else {
        this->_cst = -1;
      }
    }
  }

  gauge_bound(bool is_infinite, int cst, coefficients_t coeffs)
      : _is_infinite(is_infinite), _cst(cst), _coeffs(coeffs) {
    if (is_infinite) {
      this->_coeffs.clear();
      if (cst > 0) {
        this->_cst = 1;
      } else {
        this->_cst = -1;
      }
    }
  }

public:
  static gauge_bound_t plus_infinity() {
    return gauge_bound_t(true, 1, coefficients_t());
  }

  static gauge_bound_t minus_infinity() {
    return gauge_bound_t(true, -1, coefficients_t());
  }

public:
  gauge_bound(Number cst) : _is_infinite(false), _cst(cst), _coeffs() {}

  template < typename N,
             class = std::enable_if_t< is_supported_integral< N >::value > >
  gauge_bound(N cst)
      : _is_infinite(false), _cst(cst), _coeffs() {}

  explicit gauge_bound(VariableName v)
      : _is_infinite(false), _cst(0), _coeffs() {
    this->_coeffs.insert(v, Number(1));
  }

  explicit gauge_bound(bound_t b)
      : _is_infinite(b.is_infinite()), _cst(0), _coeffs() {
    if (b.is_plus_infinity()) {
      this->_cst = 1;
    } else if (b.is_minus_infinity()) {
      this->_cst = -1;
    } else {
      this->_cst = *b.number();
    }
  }

  gauge_bound(Number n, VariableName v)
      : _is_infinite(false), _cst(0), _coeffs() {
    this->_coeffs.insert(v, n);
  }

  template < typename N,
             class = std::enable_if_t< is_supported_integral< N >::value > >
  gauge_bound(N n, VariableName v)
      : _is_infinite(false), _cst(0), _coeffs() {
    this->_coeffs.insert(v, Number(n));
  }

  gauge_bound(const gauge_bound_t&) = default;

  gauge_bound_t& operator=(const gauge_bound_t&) = default;

  bool is_infinite() const { return this->_is_infinite; }

  bool is_finite() const { return !this->_is_infinite; }

  bool is_plus_infinity() const { return this->_is_infinite && this->_cst > 0; }

  bool is_minus_infinity() const {
    return this->_is_infinite && this->_cst < 0;
  }

  bool is_constant() const {
    return !this->_is_infinite && this->_coeffs.empty();
  }

  Number constant() const {
    if (this->_is_infinite) {
      throw logic_error("gauge_bound: trying to call constant() on +oo/-oo");
    } else {
      return this->_cst;
    }
  }

  std::size_t num_coeffs() const {
    if (this->_is_infinite) {
      throw logic_error("gauge_bound: trying to call num_coeffs() on +oo/-oo");
    } else {
      return this->_coeffs.size();
    }
  }

  Number operator[](VariableName v) const {
    if (this->_is_infinite) {
      throw logic_error("gauge_bound: trying to call operator[] on +oo/-oo");
    } else {
      boost::optional< Number > c = this->_coeffs.lookup(v);
      if (c) {
        return *c;
      } else {
        return Number(0);
      }
    }
  }

  Number sum_coeffs() const {
    if (this->_is_infinite) {
      throw logic_error("gauge_bound: trying to call sum_coeffs() on +oo/-oo");
    } else {
      Number r(0);
      for (auto it = this->_coeffs.begin(); it != this->_coeffs.end(); ++it) {
        r += it->second;
      }
      return r;
    }
  }

  boost::optional< bound_t > num_bound() const {
    if (this->_is_infinite) {
      if (this->_cst > 0) {
        return boost::optional< bound_t >(bound_t::plus_infinity());
      } else {
        return boost::optional< bound_t >(bound_t::minus_infinity());
      }
    } else if (this->_coeffs.empty()) {
      return boost::optional< bound_t >(bound_t(this->_cst));
    } else {
      return boost::optional< bound_t >();
    }
  }

private:
  static coefficients_t apply_operation(generic_binary_op_t& op,
                                        coefficients_t c1,
                                        coefficients_t c2) {
    c1.generic_merge_with(c2, op);
    return c1;
  }

  static coefficients_t apply_operation(unary_op_t& op, coefficients_t c) {
    c.transform(op);
    return c;
  }

private:
  class gauge_bound_neg : public unary_op_t {
    boost::optional< Number > apply(Number x) {
      return boost::optional< Number >(-x);
    }
  }; // end class gauge_bound_neg

public:
  gauge_bound_t operator-() const {
    gauge_bound_neg op;
    return gauge_bound_t(this->_is_infinite,
                         -this->_cst,
                         apply_operation(op, this->_coeffs));
  }

private:
  class gauge_bound_add : public generic_binary_op_t {
    boost::optional< Number > apply(Number x, Number y) {
      return boost::optional< Number >(x + y);
    }

    boost::optional< Number > apply_left(Number x) {
      return boost::optional< Number >(x);
    }

    boost::optional< Number > apply_right(Number x) {
      return boost::optional< Number >(x);
    }
  }; // end class gauge_bound_add

public:
  gauge_bound_t operator+(gauge_bound_t g) const {
    if (this->is_finite() && g.is_finite()) {
      gauge_bound_add op;
      return gauge_bound_t(false,
                           this->_cst + g._cst,
                           apply_operation(op, this->_coeffs, g._coeffs));
    } else if (this->is_finite() && g.is_infinite()) {
      return g;
    } else if (this->is_infinite() && g.is_finite()) {
      return *this;
    } else if (this->_cst == g._cst) {
      return *this;
    } else {
      throw logic_error("gauge_bound: undefined operation -oo + +oo");
    }
  }

  gauge_bound_t& operator+=(gauge_bound_t g) {
    return this->operator=(this->operator+(g));
  }

private:
  class gauge_bound_sub : public generic_binary_op_t {
    boost::optional< Number > apply(Number x, Number y) {
      return boost::optional< Number >(x - y);
    }

    boost::optional< Number > apply_left(Number x) {
      return boost::optional< Number >(x);
    }

    boost::optional< Number > apply_right(Number x) {
      return boost::optional< Number >(-x);
    }
  }; // end class gauge_bound_sub

public:
  gauge_bound_t operator-(gauge_bound_t g) const {
    if (this->is_finite() && g.is_finite()) {
      gauge_bound_sub op;
      return gauge_bound_t(false,
                           this->_cst - g._cst,
                           apply_operation(op, this->_coeffs, g._coeffs));
    } else if (this->is_finite() && g.is_infinite()) {
      return g.operator-();
    } else if (this->is_infinite() && g.is_finite()) {
      return *this;
    } else if (this->_cst == -g._cst) {
      return *this;
    } else {
      throw logic_error("gauge_bound: undefined operation +oo - +oo");
    }
  }

  gauge_bound_t& operator-=(gauge_bound_t g) {
    return this->operator=(this->operator-(g));
  }

private:
  class gauge_bound_mul : public unary_op_t {
  private:
    Number _c;

  public:
    gauge_bound_mul(Number c) : _c(c) {}

    boost::optional< Number > apply(Number x) {
      return boost::optional< Number >(x * _c);
    }
  }; // end class gauge_bound_mul

public:
  gauge_bound_t operator*(Number c) const {
    if (c == 0) {
      return 0;
    } else if (this->is_finite()) {
      gauge_bound_mul op(c);
      return gauge_bound_t(false,
                           this->_cst * c,
                           apply_operation(op, this->_coeffs));
    } else {
      return gauge_bound_t(true, this->_cst * c, coefficients_t());
    }
  }

  gauge_bound_t& operator*=(Number c) {
    return this->operator=(this->operator*(c));
  }

private:
  class gauge_bound_le : public generic_partial_order_t {
    bool leq(Number x, Number y) { return x <= y; }

    bool leq_left(Number x) { return x <= 0; }

    bool leq_right(Number x) { return 0 <= x; }
  }; // end class gauge_bound_le

  class gauge_bound_ge : public generic_partial_order_t {
    bool leq(Number x, Number y) { return x >= y; }

    bool leq_left(Number x) { return x >= 0; }

    bool leq_right(Number x) { return 0 >= x; }
  }; // end class gauge_bound_ge

public:
  bool operator<=(gauge_bound_t g) const {
    if (this->_is_infinite && g._is_infinite) {
      return this->_cst <= g._cst;
    } else if (this->_is_infinite) {
      return this->_cst < 0;
    } else if (g._is_infinite) {
      return g._cst > 0;
    } else {
      gauge_bound_le le;
      return this->_cst <= g._cst && this->_coeffs.generic_leq(g._coeffs, le);
    }
  }

  bool operator>=(gauge_bound_t g) const {
    if (this->_is_infinite && g._is_infinite) {
      return this->_cst >= g._cst;
    } else if (this->_is_infinite) {
      return this->_cst > 0;
    } else if (g._is_infinite) {
      return g._cst < 0;
    } else {
      gauge_bound_ge ge;
      return this->_cst >= g._cst && this->_coeffs.generic_leq(g._coeffs, ge);
    }
  }

  bool operator==(gauge_bound_t g) const {
    return this->operator<=(g) && g.operator<=(*this);
  }

  bool operator!=(gauge_bound_t g) const { return !this->operator==(g); }

private:
  static Number max(Number x, Number y) { return x <= y ? y : x; }

  static Number min(Number x, Number y) { return x <= y ? x : y; }

private:
  class gauge_bound_min : public generic_binary_op_t {
    boost::optional< Number > apply(Number x, Number y) {
      return boost::optional< Number >(min(x, y));
    }

    boost::optional< Number > apply_left(Number x) {
      return boost::optional< Number >(min(x, Number(0)));
    }

    boost::optional< Number > apply_right(Number x) {
      return boost::optional< Number >(min(Number(0), x));
    }
  }; // end class gauge_bound_min

public:
  static gauge_bound_t min(gauge_bound_t x, gauge_bound_t y) {
    if (x.is_infinite() || y.is_infinite()) {
      return x.operator<=(y) ? x : y;
    } else {
      gauge_bound_min op;
      return gauge_bound_t(false,
                           min(x._cst, y._cst),
                           apply_operation(op, x._coeffs, y._coeffs));
    }
  }

  static gauge_bound_t min(gauge_bound_t x, gauge_bound_t y, gauge_bound_t z) {
    return min(x, min(y, z));
  }

  static gauge_bound_t min(gauge_bound_t x,
                           gauge_bound_t y,
                           gauge_bound_t z,
                           gauge_bound_t t) {
    return min(min(x, y), min(z, t));
  }

private:
  class gauge_bound_max : public generic_binary_op_t {
    boost::optional< Number > apply(Number x, Number y) {
      return boost::optional< Number >(max(x, y));
    }

    boost::optional< Number > apply_left(Number x) {
      return boost::optional< Number >(max(x, Number(0)));
    }

    boost::optional< Number > apply_right(Number x) {
      return boost::optional< Number >(max(Number(0), x));
    }
  }; // end class gauge_bound_max

public:
  static gauge_bound_t max(gauge_bound_t x, gauge_bound_t y) {
    if (x.is_infinite() || y.is_infinite()) {
      return x.operator<=(y) ? y : x;
    } else {
      gauge_bound_max op;
      return gauge_bound_t(false,
                           max(x._cst, y._cst),
                           apply_operation(op, x._coeffs, y._coeffs));
    }
  }

  static gauge_bound_t max(gauge_bound_t x, gauge_bound_t y, gauge_bound_t z) {
    return max(x, max(y, z));
  }

  static gauge_bound_t max(gauge_bound_t x,
                           gauge_bound_t y,
                           gauge_bound_t z,
                           gauge_bound_t t) {
    return max(max(x, y), max(z, t));
  }

  linear_expression_t lin_expr() const {
    if (this->_is_infinite) {
      throw logic_error("gauge_bound: trying to call lin_expr() on +oo/-oo");
    } else {
      linear_expression_t e(this->_cst);
      for (auto it = this->_coeffs.begin(); it != this->_coeffs.end(); ++it) {
        e = e + it->second * variable_t(it->first);
      }
      return e;
    }
  }

  void write(std::ostream& o) const {
    if (this->is_plus_infinity()) {
      o << "+oo";
    } else if (this->is_minus_infinity()) {
      o << "-oo";
    } else {
      bool first = true;
      for (typename coefficients_t::iterator it = this->_coeffs.begin();
           it != this->_coeffs.end();
           ++it) {
        Number c = it->second;
        VariableName v = it->first;
        if (c == 0) {
          continue;
        } else if (c > 0 && !first) {
          o << "+";
        }
        if (c == -1) {
          o << "-";
        } else if (c != 1) {
          o << c;
        }
        o << v;
        first = false;
      }
      if (this->_cst > 0 && !first) {
        o << "+";
      }
      if (this->_cst != 0 || first) {
        o << this->_cst;
      }
    }
  }

}; // end class gauge_bound

template < typename Number, typename VariableName >
inline gauge_bound< Number, VariableName > operator*(
    Number c, const gauge_bound< Number, VariableName >& g) {
  return g * c;
}

template < typename Number, typename VariableName >
inline std::ostream& operator<<(std::ostream& o,
                                const gauge_bound< Number, VariableName >& g) {
  g.write(o);
  return o;
}

/// \brief Gauge abstract value
///
/// This is implemented as a pair of gauge_bound
template < typename Number, typename VariableName >
class gauge {
public:
  typedef gauge_bound< Number, VariableName > gauge_bound_t;
  typedef gauge< Number, VariableName > gauge_t;
  typedef bound< Number > bound_t;
  typedef constant< Number > constant_t;
  typedef interval< Number > interval_t;

private:
  gauge_bound_t _lb;
  gauge_bound_t _ub;

  /* Invariants:
   * _lb > _ub => _lb = 0 and _ub = -1
   */

private:
  gauge() = delete;

private:
  static Number max(Number x, Number y) { return x <= y ? y : x; }

  static Number min(Number x, Number y) { return x <= y ? x : y; }

  static Number div_round_low(Number num, Number den) {
    if (den < 0) {
      num = -num;
      den = -den;
    }
    Number q = num / den;
    Number r = num % den;
    if (r == 0 || num > 0) {
      return q;
    } else {
      return q - 1;
    }
  }

  static Number div_round_up(Number num, Number den) {
    if (den < 0) {
      num = -num;
      den = -den;
    }
    Number q = num / den;
    Number r = num % den;
    if (r == 0 || num < 0) {
      return q;
    } else {
      return q + 1;
    }
  }

public:
  static gauge_t top() {
    return gauge_t(gauge_bound_t::minus_infinity(),
                   gauge_bound_t::plus_infinity());
  }

  static gauge_t bottom() { return gauge_t(0, -1); }

public:
  gauge(gauge_bound_t lb, gauge_bound_t ub) : _lb(lb), _ub(ub) {
    if (!(lb <= ub)) {
      this->_lb = 0;
      this->_ub = -1;
    }
  }

  explicit gauge(gauge_bound_t b) : _lb(b), _ub(b) {
    if (b.is_infinite()) {
      this->_lb = 0;
      this->_ub = -1;
    }
  }

  explicit gauge(Number cst) : _lb(cst), _ub(cst) {}

  explicit gauge(interval_t i) : _lb(0), _ub(-1) {
    if (!i.is_bottom()) {
      this->_lb = gauge_bound_t(i.lb());
      this->_ub = gauge_bound_t(i.ub());
    }
  }

  gauge(const gauge_t&) = default;

  gauge_t& operator=(const gauge_t&) = default;

  gauge_bound_t lb() const { return this->_lb; }

  gauge_bound_t ub() const { return this->_ub; }

  bool is_bottom() const { return !(this->_lb <= this->_ub); }

  bool is_top() const {
    return this->_lb.is_infinite() && this->_ub.is_infinite();
  }

  boost::optional< gauge_bound_t > singleton() const {
    if (!this->is_bottom() && this->_lb == this->_ub) {
      return boost::optional< gauge_bound_t >(this->_lb);
    } else {
      return boost::optional< gauge_bound_t >();
    }
  }

  boost::optional< interval_t > interval() const {
    if (this->is_bottom()) {
      return boost::optional< interval_t >(interval_t::bottom());
    } else {
      boost::optional< bound_t > lb = this->_lb.num_bound();
      boost::optional< bound_t > ub = this->_ub.num_bound();
      if (lb && ub) {
        return boost::optional< interval_t >(interval_t(*lb, *ub));
      } else {
        return boost::optional< interval_t >();
      }
    }
  }

  bool operator<=(gauge_t g) const {
    if (this->is_bottom()) {
      return true;
    } else if (g.is_bottom()) {
      return false;
    } else {
      return (g._lb <= this->_lb) && (this->_ub <= g._ub);
    }
  }

  bool operator==(gauge_t g) const {
    if (is_bottom()) {
      return g.is_bottom();
    } else {
      return (this->_lb == g._lb) && (this->_ub == g._ub);
    }
  }

  bool operator!=(gauge_t g) const { return !this->operator==(g); }

  gauge_t operator|(gauge_t g) const {
    if (this->is_bottom()) {
      return g;
    } else if (g.is_bottom()) {
      return *this;
    } else {
      return gauge_t(gauge_bound_t::min(this->_lb, g._lb),
                     gauge_bound_t::max(this->_ub, g._ub));
    }
  }

  /// \brief Meet
  ///
  /// Implementation based on X. Wu, L. Chen and J. Wang's paper: An Abstract
  /// Domain to Infer Symbolic Ranges over Nonnegative Parameters.
  ///
  /// It uses a heuristic because, in general, the greatest lower bound
  /// cannot be defined.
  gauge_t operator&(gauge_t g) const {
    if (this->is_bottom() || g.is_bottom()) {
      return gauge_t::bottom();
    } else {
      // lower bound
      gauge_bound_t lb = gauge_bound_t::minus_infinity();
      if (g._lb <= this->_lb) {
        lb = this->_lb;
      } else if (this->_lb <= g._lb) {
        lb = g._lb;
      } else {
        // no order, use a heuristic
        lb = (this->_lb.constant() + this->_lb.sum_coeffs() >=
              g._lb.constant() + g._lb.sum_coeffs())
                 ? this->_lb
                 : g._lb;
      }

      // upper bound
      gauge_bound_t ub = gauge_bound_t::plus_infinity();
      if (this->_ub <= g._ub) {
        ub = this->_ub;
      } else if (g._ub <= this->_ub) {
        ub = g._ub;
      } else {
        // no order, use a heuristic
        ub = (this->_ub.constant() + this->_ub.sum_coeffs() <=
              g._ub.constant() + g._ub.sum_coeffs())
                 ? this->_ub
                 : g._ub;
      }

      if (lb.is_infinite() || ub.is_infinite() ||
          (lb.is_constant() && ub.is_constant())) {
        // in that case, it is safe to use [lb, ub]
        return gauge_t(lb, ub);
      } else {
        // use max(lb, ub) so that the gauge is not empty
        return gauge_t(lb, gauge_bound_t::max(lb, ub));
      }
    }
  }

  gauge_t operator&&(gauge_t g) const {
    if (this->is_bottom() || g.is_bottom()) {
      return gauge_t::bottom();
    } else {
      gauge_bound_t lb =
          this->_lb.is_infinite() && g._lb.is_finite() ? g._lb : this->_lb;
      gauge_bound_t ub =
          this->_ub.is_infinite() && g._ub.is_finite() ? g._ub : this->_ub;

      if (lb.is_infinite() || ub.is_infinite() ||
          (lb.is_constant() && ub.is_constant())) {
        // in that case, it is safe to use [lb, ub]
        return gauge_t(lb, ub);
      } else {
        // use max(lb, ub) so that the gauge is not empty
        return gauge_t(lb, gauge_bound_t::max(lb, ub));
      }
    }
  }

  gauge_t operator-() const {
    if (this->is_bottom()) {
      return this->bottom();
    } else {
      return gauge_t(-this->_ub, -this->_lb);
    }
  }

  gauge_t operator+(gauge_t g) const {
    if (this->is_bottom() || g.is_bottom()) {
      return this->bottom();
    } else {
      return gauge_t(this->_lb + g._lb, this->_ub + g._ub);
    }
  }

  gauge_t& operator+=(gauge_t g) { return this->operator=(this->operator+(g)); }

  gauge_t operator-(gauge_t g) const {
    if (this->is_bottom() || g.is_bottom()) {
      return this->bottom();
    } else {
      return gauge_t(this->_lb - g._ub, this->_ub - g._lb);
    }
  }

  gauge_t& operator-=(gauge_t g) { return this->operator=(this->operator-(g)); }

  gauge_t operator*(Number c) const {
    if (this->is_bottom()) {
      return this->bottom();
    } else if (c >= 0) {
      return gauge_t(c * this->_lb, c * this->_ub);
    } else {
      return gauge_t(c * this->_ub, c * this->_lb);
    }
  }

  gauge_t operator*(interval_t i) const {
    if (this->is_bottom() || i.is_bottom()) {
      return this->bottom();
    } else if (i.is_top()) {
      return gauge_t::top();
    } else if (i.lb().is_infinite()) {
      // i = [-oo, u]
      Number u = *i.ub().number();
      if (this->_lb >= 0) {
        return gauge_t(gauge_bound_t::minus_infinity(),
                       gauge_bound_t::max(this->_lb * u, this->_ub * u));
      } else if (this->_ub <= 0) {
        return gauge_t(gauge_bound_t::min(this->_lb * u, this->_ub * u),
                       gauge_bound_t::plus_infinity());
      } else {
        return gauge_t::top();
      }
    } else if (i.ub().is_infinite()) {
      // i = [l, +oo]
      Number l = *i.lb().number();
      if (this->_lb >= 0) {
        return gauge_t(gauge_bound_t::min(this->_lb * l, this->_ub * l),
                       gauge_bound_t::plus_infinity());
      } else if (this->_ub <= 0) {
        return gauge_t(gauge_bound_t::minus_infinity(),
                       gauge_bound_t::max(this->_lb * l, this->_ub * l));
      } else {
        return gauge_t::top();
      }
    } else {
      Number l = *i.lb().number();
      Number u = *i.ub().number();
      gauge_bound_t ll = this->_lb * l;
      gauge_bound_t lu = this->_lb * u;
      gauge_bound_t ul = this->_ub * l;
      gauge_bound_t uu = this->_ub * u;
      return gauge_t(gauge_bound_t::min(ll, lu, ul, uu),
                     gauge_bound_t::max(ll, lu, ul, uu));
    }
  }

  /// \brief Forget a counter variable
  gauge_t forget(VariableName v) const {
    if (!this->is_bottom() && ((this->_lb.is_finite() && this->_lb[v] != 0) ||
                               (this->_ub.is_finite() && this->_ub[v] != 0))) {
      return this->top();
    } else {
      return *this;
    }
  }

  gauge_t coalesce(VariableName v, Number l, bound_t u) const {
    if (this->is_bottom()) {
      return this->bottom();
    } else {
      // note: the definition of coalesce() in Arnaud's paper is wrong because
      // coefficients can be negative
      gauge_bound_t lb = this->_lb;
      if (this->_lb.is_finite()) {
        Number lb_v = this->_lb[v];
        if (lb_v > 0) {
          lb = this->_lb + lb_v * l + gauge_bound_t(-lb_v, v);
        } else if (lb_v < 0) {
          if (u.is_finite()) {
            lb = this->_lb + lb_v * (*u.number()) + gauge_bound_t(-lb_v, v);
          } else {
            lb = gauge_bound_t::minus_infinity();
          }
        }
      }
      gauge_bound_t ub = this->_ub;
      if (this->_ub.is_finite()) {
        Number ub_v = this->_ub[v];
        if (ub_v > 0) {
          if (u.is_finite()) {
            ub = this->_ub + ub_v * (*u.number()) + gauge_bound_t(-ub_v, v);
          } else {
            ub = gauge_bound_t::plus_infinity();
          }
        } else if (ub_v < 0) {
          ub = this->_ub + ub_v * l + gauge_bound_t(-ub_v, v);
        }
      }
      return gauge_t(lb, ub);
    }
  }

  gauge_t incr_counter(VariableName v, Number k) const {
    // note: there is a typo in the paper, it should be:
    // [min(a_0 - k*a_j, b_0 - k*b_j) + ..; max(a_0 - k*a_j, b_0 - k*_bj) + ...]
    if (this->is_bottom()) {
      return *this;
    } else if (this->_lb.is_finite() && this->_ub.is_finite()) {
      Number lb_new_cst = this->_lb.constant() - k * this->_lb[v];
      Number ub_new_cst = this->_ub.constant() - k * this->_ub[v];
      return gauge_t(this->_lb - this->_lb.constant() +
                         min(lb_new_cst, ub_new_cst),
                     this->_ub - this->_ub.constant() +
                         max(lb_new_cst, ub_new_cst));
    } else if (this->_lb.is_finite()) {
      return gauge_t(this->_lb - k * this->_lb[v],
                     gauge_bound_t::plus_infinity());
    } else if (this->_ub.is_finite()) {
      return gauge_t(gauge_bound_t::minus_infinity(),
                     this->_ub - k * this->_ub[v]);
    } else {
      return *this;
    }
  }

  /// \brief Widening by linear interpolation
  ///
  /// \f$G \nabla^{k}_{u,v} G'\f$ in Arnaud's paper
  gauge_t widen_interpol(gauge_t g,
                         VariableName k,
                         Number u,
                         constant_t v) const {
    ikos_assert(constant_t(u) != v);
    if (this->is_bottom()) {
      return g;
    } else if (g.is_bottom()) {
      return *this;
    } else {
      bool lb_is_finite = this->_lb.is_finite() && g._lb.is_finite();
      bool ub_is_finite = this->_ub.is_finite() && g._ub.is_finite();

      Number lb_slope, ub_slope;
      if (v.is_number()) {
        Number v_num = *v.number();

        if (lb_is_finite) {
          lb_slope =
              div_round_low((g._lb.constant() + g._lb[k] * v_num) -
                                (this->_lb.constant() + this->_lb[k] * u),
                            v_num - u);
        }
        if (ub_is_finite) {
          ub_slope = div_round_up((g._ub.constant() + g._ub[k] * v_num) -
                                      (this->_ub.constant() + this->_ub[k] * u),
                                  v_num - u);
        }
      } else {
        if (lb_is_finite) {
          lb_slope = g._lb[k];
        }
        if (ub_is_finite) {
          ub_slope = g._ub[k];
        }
      }

      Number lb_new_cst, ub_new_cst;
      if (lb_is_finite) {
        lb_new_cst = this->_lb.constant() + this->_lb[k] * u - lb_slope * u;

        if (v.is_top()) {
          lb_new_cst = min(lb_new_cst, g._lb.constant());
        }
      }
      if (ub_is_finite) {
        ub_new_cst = this->_ub.constant() + this->_ub[k] * u - ub_slope * u;

        if (v.is_top()) {
          ub_new_cst = max(ub_new_cst, g._ub.constant());
        }
      }

      gauge_bound_t lb = gauge_bound_t::min(this->_lb, g._lb);
      gauge_bound_t ub = gauge_bound_t::max(this->_ub, g._ub);

      if (lb_is_finite && ub_is_finite) {
        lb += gauge_bound_t(min(lb_new_cst, ub_new_cst) - lb.constant()) +
              gauge_bound_t(min(lb_slope, ub_slope) - lb[k], k);
        ub += gauge_bound_t(max(lb_new_cst, ub_new_cst) - ub.constant()) +
              gauge_bound_t(max(lb_slope, ub_slope) - ub[k], k);
      } else if (lb_is_finite) {
        lb += gauge_bound_t(lb_new_cst - lb.constant()) +
              gauge_bound_t(lb_slope - lb[k], k);
      } else if (ub_is_finite) {
        ub += gauge_bound_t(ub_new_cst - ub.constant()) +
              gauge_bound_t(ub_slope - ub[k], k);
      }

      return gauge_t(lb, ub);
    }
  }

  /// \brief Interval-like widening
  ///
  /// \f$\nabla_{I}\f$ in Arnaud's paper
  gauge_t widen_interval(gauge_t g) const {
    if (this->is_bottom()) {
      return g;
    } else if (g.is_bottom()) {
      return *this;
    } else {
      return gauge_t(this->_lb <= g._lb ? this->_lb
                                        : gauge_bound_t::minus_infinity(),
                     g._ub <= this->_ub ? this->_ub
                                        : gauge_bound_t::plus_infinity());
    }
  }

  void write(std::ostream& o) const {
    if (is_bottom()) {
      o << "_|_";
    } else {
      o << "[" << this->_lb << ", " << this->_ub << "]";
    }
  }

}; // end class gauge

template < typename Number, typename VariableName >
inline gauge< Number, VariableName > operator*(
    Number c, const gauge< Number, VariableName >& g) {
  return g * c;
}

template < typename Number, typename VariableName >
inline std::ostream& operator<<(std::ostream& o,
                                const gauge< Number, VariableName >& g) {
  g.write(o);
  return o;
}

/// \brief Gauge semilattice
template < typename Number, typename VariableName >
class gauge_semilattice : public writeable {
public:
  typedef gauge_bound< Number, VariableName > gauge_bound_t;
  typedef gauge< Number, VariableName > gauge_t;
  typedef bound< Number > bound_t;
  typedef constant< Number > constant_t;
  typedef interval< Number > interval_t;
  typedef variable< Number, VariableName > variable_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;
  typedef gauge_semilattice< Number, VariableName > gauge_semilattice_t;

private:
  typedef patricia_tree< VariableName, gauge_t > patricia_tree_t;
  typedef typename patricia_tree_t::partial_order_t partial_order_t;
  typedef typename patricia_tree_t::unary_op_t unary_op_t;
  typedef typename patricia_tree_t::binary_op_t binary_op_t;

private:
  bool _is_bottom;
  patricia_tree_t _tree;

  /* Invariants:
   * _is_bottom => _tree.empty()
   * for v in _tree: _tree.lookup(v) != gauge_t::top()
   * for v in _tree: _tree.lookup(v) != gauge_t::bottom()
   */

private:
  gauge_semilattice(patricia_tree_t tree) : _is_bottom(false), _tree(tree) {}

  gauge_semilattice(bool is_bottom) : _is_bottom(is_bottom) {}

public:
  static gauge_semilattice_t top() { return gauge_semilattice_t(false); }

  static gauge_semilattice_t bottom() { return gauge_semilattice_t(true); }

private:
  static patricia_tree_t apply_operation(binary_op_t& o,
                                         patricia_tree_t t1,
                                         patricia_tree_t t2) {
    t1.merge_with(t2, o);
    return t1;
  }

public:
  gauge_semilattice() : _is_bottom(false) {}

  gauge_semilattice(const gauge_semilattice_t&) = default;

  gauge_semilattice_t& operator=(const gauge_semilattice_t&) = default;

  bool is_bottom() const { return this->_is_bottom; }

  bool is_top() const { return !this->_is_bottom && this->_tree.empty(); }

private:
  class domain_po : public partial_order_t {
    bool leq(gauge_t x, gauge_t y) { return x <= y; }

    bool default_is_top() { return true; }
  }; // end class domain_po

public:
  bool operator<=(gauge_semilattice_t o) const {
    if (this->is_bottom()) {
      return true;
    } else if (o.is_bottom()) {
      return false;
    } else {
      domain_po po;
      return this->_tree.leq(o._tree, po);
    }
  }

  bool operator==(gauge_semilattice_t o) const {
    return this->operator<=(o) && o.operator<=(*this);
  }

private:
  class join_op : public binary_op_t {
    boost::optional< gauge_t > apply(gauge_t x, gauge_t y) {
      gauge_t z = x | y;
      if (z.is_top()) {
        return boost::optional< gauge_t >();
      } else {
        return boost::optional< gauge_t >(z);
      }
    };

    bool default_is_absorbing() { return true; }
  }; // end class join_op

public:
  gauge_semilattice_t operator|(gauge_semilattice_t o) const {
    if (this->is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      join_op op;
      return gauge_semilattice_t(apply_operation(op, this->_tree, o._tree));
    }
  }

private:
  class bottom_found {};

  class meet_op : public binary_op_t {
    boost::optional< gauge_t > apply(gauge_t x, gauge_t y) {
      gauge_t z = x & y;
      if (z.is_bottom()) {
        throw bottom_found();
      } else {
        return boost::optional< gauge_t >(z);
      }
    }

    bool default_is_absorbing() { return false; }
  }; // end class meet_op

public:
  gauge_semilattice_t operator&(gauge_semilattice_t o) const {
    if (this->is_bottom() || o.is_bottom()) {
      return gauge_semilattice_t::bottom();
    } else {
      try {
        meet_op op;
        return gauge_semilattice_t(apply_operation(op, this->_tree, o._tree));
      } catch (bottom_found&) {
        return gauge_semilattice_t::bottom();
      }
    }
  }

private:
  class widening_interval_op : public binary_op_t {
    boost::optional< gauge_t > apply(gauge_t x, gauge_t y) {
      gauge_t z = x.widen_interval(y);
      if (z.is_top()) {
        return boost::optional< gauge_t >();
      } else {
        return boost::optional< gauge_t >(z);
      }
    }

    bool default_is_absorbing() { return true; }
  }; // end class widening_interval_op

public:
  gauge_semilattice_t widen_interval(gauge_semilattice_t o) const {
    if (this->is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      widening_interval_op op;
      return gauge_semilattice_t(apply_operation(op, this->_tree, o._tree));
    }
  }

private:
  class widening_interpolation_op : public binary_op_t {
  private:
    VariableName _k;
    Number _u;
    constant_t _v;

  public:
    widening_interpolation_op(VariableName k, Number u, constant_t v)
        : _k(k), _u(u), _v(v) {}

    boost::optional< gauge_t > apply(gauge_t x, gauge_t y) {
      gauge_t z = x.widen_interpol(y, _k, _u, _v);
      if (z.is_top()) {
        return boost::optional< gauge_t >();
      } else {
        return boost::optional< gauge_t >(z);
      }
    }

    bool default_is_absorbing() { return true; }
  }; // end class widening_interpolation_op

public:
  gauge_semilattice_t widen_interpol(gauge_semilattice_t o,
                                     VariableName k,
                                     Number u,
                                     constant_t v) const {
    if (this->is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      widening_interpolation_op op(k, u, v);
      return gauge_semilattice_t(apply_operation(op, this->_tree, o._tree));
    }
  }

private:
  class narrowing_op : public binary_op_t {
    boost::optional< gauge_t > apply(gauge_t x, gauge_t y) {
      gauge_t z = x && y;
      if (z.is_bottom()) {
        throw bottom_found();
      } else {
        return boost::optional< gauge_t >(z);
      }
    }

    bool default_is_absorbing() { return false; }
  }; // end class narrowing_op

public:
  gauge_semilattice_t operator&&(gauge_semilattice_t o) const {
    if (this->is_bottom() || o.is_bottom()) {
      return gauge_semilattice_t::bottom();
    } else {
      try {
        narrowing_op op;
        return gauge_semilattice_t(apply_operation(op, this->_tree, o._tree));
      } catch (bottom_found&) {
        return gauge_semilattice_t::bottom();
      }
    }
  }

private:
  class incr_counter_op : public unary_op_t {
  private:
    VariableName _v;
    Number _k;

  public:
    incr_counter_op(VariableName v, Number k) : _v(v), _k(k) {}

    boost::optional< gauge_t > apply(gauge_t x) {
      gauge_t y = x.incr_counter(_v, _k);
      if (y.is_top()) {
        return boost::optional< gauge_t >();
      } else {
        return boost::optional< gauge_t >(y);
      }
    }
  }; // end class incr_counter_op

public:
  void incr_counter(VariableName v, Number k) {
    if (!this->is_bottom()) {
      incr_counter_op op(v, k);
      this->_tree.transform(op);
    }
  }

private:
  class coalesce_op : public unary_op_t {
  private:
    VariableName _v;
    Number _l;
    bound_t _u;

  public:
    coalesce_op(VariableName v, Number l, bound_t u) : _v(v), _l(l), _u(u) {}

    boost::optional< gauge_t > apply(gauge_t x) {
      gauge_t y = x.coalesce(_v, _l, _u);
      if (y.is_top()) {
        return boost::optional< gauge_t >();
      } else {
        return boost::optional< gauge_t >(y);
      }
    }
  }; // end class coalesce_op

public:
  void forget_counter(VariableName v, interval_t value = interval_t::top()) {
    if (this->is_bottom()) {
      return;
    }

    value = value & interval_t(0, bound_t::plus_infinity());
    ikos_assert(!value.is_bottom() && value.lb().is_finite());
    coalesce_op op(v, *value.lb().number(), value.ub());
    this->_tree.transform(op);
  }

  gauge_semilattice_t& operator-=(VariableName v) {
    if (!this->is_bottom()) {
      this->_tree.remove(v);
    }
    return *this;
  }

  void set(VariableName v, gauge_t g) {
    if (!this->is_bottom()) {
      if (g.is_bottom()) {
        this->_is_bottom = true;
        this->_tree.clear();
      } else if (g.is_top()) {
        this->_tree.remove(v);
      } else {
        this->_tree.insert(v, g);
      }
    }
  }

  gauge_t operator[](VariableName v) {
    if (this->is_bottom()) {
      return gauge_t::bottom();
    } else {
      boost::optional< gauge_t > g = this->_tree.lookup(v);
      if (g) {
        return *g;
      } else {
        return gauge_t::top();
      }
    }
  }

  linear_constraint_system_t to_linear_constraint_system() {
    linear_constraint_system_t csts;

    if (this->is_bottom()) {
      csts += linear_constraint_t::contradiction();
      return csts;
    }

    for (auto it = this->_tree.begin(); it != this->_tree.end(); ++it) {
      gauge_bound_t lb = it->second.lb(), ub = it->second.ub();
      if (lb.is_finite()) {
        csts += (lb.lin_expr() <= variable_t(it->first));
      }
      if (ub.is_finite()) {
        csts += (variable_t(it->first) <= ub.lin_expr());
      }
    }

    return csts;
  }

  void write(std::ostream& o) {
    if (this->is_bottom()) {
      o << "_|_";
    } else {
      o << "{";
      for (typename patricia_tree_t::iterator it = this->_tree.begin();
           it != this->_tree.end();) {
        VariableName v = it->first;
        gauge_t g = it->second;
        index_traits< VariableName >::write(o, v);
        o << " -> ";
        g.write(o);
        ++it;
        if (it != this->_tree.end()) {
          o << "; ";
        }
      }
      o << "}";
    }
  }

  static std::string domain_name() { return "Gauge Semilattice"; }

}; // end class gauge_semilattice

/// \brief Gauge abstract domain
template < typename Number, typename VariableName >
class gauge_domain : public abstract_domain,
                     public numerical_domain< Number, VariableName >,
                     public bitwise_operators< Number, VariableName >,
                     public division_operators< Number, VariableName >,
                     public counter_domain< Number, VariableName > {
public:
  typedef gauge_bound< Number, VariableName > gauge_bound_t;
  typedef gauge< Number, VariableName > gauge_t;
  typedef bound< Number > bound_t;
  typedef constant< Number > constant_t;
  typedef interval< Number > interval_t;
  typedef congruence< Number > congruence_t;
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;
  typedef constant_domain< Number, VariableName > section_domain_t;
  typedef gauge_semilattice< Number, VariableName > gauge_semilattice_t;
  typedef patricia_tree_set< VariableName > counter_set_t;
  typedef interval_domain< Number, VariableName > interval_domain_t;
  typedef gauge_domain< Number, VariableName > gauge_domain_t;

private:
  bool _is_bottom;
  section_domain_t _sections;
  gauge_semilattice_t _gauges;
  counter_set_t _counters;
  interval_domain_t _intervals;

  /* Invariants:
   * _is_bottom => _sections.is_bottom() &&
   *               _gauges.is_bottom() &&
   *               _counters.empty() &&
   *               _intervals.is_bottom()
   * !_is_bottom => !_sections.is_bottom() &&
   *                !_gauges.is_bottom() &&
   *                !_intervals.is_bottom()
   */

private:
  gauge_domain(const section_domain_t& sections,
               const gauge_semilattice_t& gauges,
               const counter_set_t& counters,
               const interval_domain_t& intervals)
      : _is_bottom(false),
        _sections(sections),
        _gauges(gauges),
        _counters(counters),
        _intervals(intervals) {
    this->canonicalize();
  }

  gauge_domain(bool is_bottom)
      : _is_bottom(is_bottom),
        _sections(is_bottom ? section_domain_t::bottom()
                            : section_domain_t::top()),
        _gauges(is_bottom ? gauge_semilattice_t::bottom()
                          : gauge_semilattice_t::top()),
        _counters(),
        _intervals(is_bottom ? interval_domain_t::bottom()
                             : interval_domain_t::top()) {}

private:
  void canonicalize() {
    if (!this->_is_bottom) {
      this->_is_bottom = this->_sections.is_bottom() ||
                         this->_gauges.is_bottom() ||
                         this->_intervals.is_bottom();
      if (this->_is_bottom) {
        this->_sections = section_domain_t::bottom();
        this->_gauges = gauge_semilattice_t::bottom();
        this->_counters.clear();
        this->_intervals = interval_domain_t::bottom();
      }
    }
  }

public:
  static gauge_domain_t top() { return gauge_domain_t(false); }

  static gauge_domain_t bottom() { return gauge_domain_t(true); }

public:
  gauge_domain()
      : _is_bottom(false),
        _sections(section_domain_t::top()),
        _gauges(gauge_semilattice_t::top()),
        _counters(),
        _intervals(interval_domain_t::top()) {}

  gauge_domain(const gauge_domain_t&) = default;

  gauge_domain_t& operator=(const gauge_domain_t&) = default;

  bool is_bottom() { return this->_is_bottom; }

  bool is_top() {
    return this->_sections.is_top() && this->_gauges.is_top() &&
           this->_counters.empty() && this->_intervals.is_top();
  }

  bool operator<=(gauge_domain_t other) {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_sections <= other._sections &&
             this->_gauges <= other._gauges &&
             other._counters <= this->_counters &&
             this->_intervals <= other._intervals;
    }
  }

  bool operator==(gauge_domain_t other) {
    return this->operator<=(other) && other.operator<=(*this);
  }

private:
  /// \brief Make sure `l` and `r` have the same set of counters
  static void uniformize_counters(gauge_domain_t& l, gauge_domain_t& r) {
    counter_set_t l_counters = l._counters;
    counter_set_t r_counters = r._counters;

    for (auto it = l_counters.begin(); it != l_counters.end(); ++it) {
      if (!r_counters[*it]) {
        l.unmark_counter(*it);
      }
    }
    for (auto it = r_counters.begin(); it != r_counters.end(); ++it) {
      if (!l_counters[*it]) {
        r.unmark_counter(*it);
      }
    }

    ikos_assert(l._counters == r._counters);
  }

public:
  gauge_domain_t operator|(gauge_domain_t other) {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      if (this->_counters == other._counters) {
        return gauge_domain_t(this->_sections | other._sections,
                              this->_gauges | other._gauges,
                              this->_counters & other._counters,
                              this->_intervals | other._intervals);
      } else {
        gauge_domain_t left = *this;
        uniformize_counters(left, other);
        return left.operator|(other);
      }
    }
  }

  gauge_domain_t operator||(gauge_domain_t other) {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      if (this->_counters == other._counters) {
        section_domain_t sections = this->_sections || other._sections;
        if (sections == this->_sections) {
          return gauge_domain_t(sections,
                                this->_gauges.widen_interval(other._gauges),
                                this->_counters & other._counters,
                                this->_intervals || other._intervals);
        } else {
          gauge_semilattice_t gauges = this->_gauges;
          for (auto it = this->_sections.begin(); it != this->_sections.end();
               ++it) {
            VariableName k = it->first;
            constant_t u = it->second;
            constant_t v = other._sections[k];
            if (u != v) {
              ikos_assert(u.is_number());
              gauges = gauges.widen_interpol(other._gauges, k, *u.number(), v);
            }
          }
          return gauge_domain_t(sections,
                                gauges,
                                this->_counters & other._counters,
                                this->_intervals || other._intervals);
        }
      } else {
        gauge_domain_t left = *this;
        uniformize_counters(left, other);
        return left.operator||(other);
      }
    }
  }

  gauge_domain_t join_loop(gauge_domain_t other) {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      if (this->_counters == other._counters) {
        section_domain_t sections = this->_sections | other._sections;
        if (sections == this->_sections) {
          return gauge_domain_t(sections,
                                this->_gauges | other._gauges,
                                this->_counters & other._counters,
                                this->_intervals | other._intervals);
        } else {
          gauge_semilattice_t gauges = this->_gauges;
          for (auto it = this->_sections.begin(); it != this->_sections.end();
               ++it) {
            VariableName k = it->first;
            constant_t u = it->second;
            constant_t v = other._sections[k];
            if (u != v) {
              ikos_assert(u.is_number());
              gauges = gauges.widen_interpol(other._gauges, k, *u.number(), v);
            }
          }
          return gauge_domain_t(sections,
                                gauges,
                                this->_counters & other._counters,
                                this->_intervals | other._intervals);
        }
      } else {
        gauge_domain_t left = *this;
        uniformize_counters(left, other);
        return left.join_loop(other);
      }
    }
  }

  gauge_domain_t join_iter(gauge_domain_t other) {
    return this->join_loop(other);
  }

  gauge_domain_t operator&(gauge_domain_t other) {
    if (this->is_bottom() || other.is_bottom()) {
      return gauge_domain_t::bottom();
    } else {
      if (this->_counters == other._counters) {
        return gauge_domain_t(this->_sections & other._sections,
                              this->_gauges & other._gauges,
                              this->_counters | other._counters,
                              this->_intervals & other._intervals);
      } else {
        gauge_domain_t left = *this;
        uniformize_counters(left, other);
        return left.operator&(other);
      }
    }
  }

  gauge_domain_t operator&&(gauge_domain_t other) {
    if (this->is_bottom() || other.is_bottom()) {
      return gauge_domain_t::bottom();
    } else {
      if (this->_counters == other._counters) {
        return gauge_domain_t(this->_sections && other._sections,
                              this->_gauges && other._gauges,
                              this->_counters | other._counters,
                              this->_intervals && other._intervals);
      } else {
        gauge_domain_t left = *this;
        uniformize_counters(left, other);
        return left.operator&&(other);
      }
    }
  }

  gauge_t operator[](VariableName v) {
    if (this->is_bottom()) {
      return gauge_t::bottom();
    } else if (this->_counters[v]) {
      return gauge_t(gauge_bound_t(v));
    } else {
      return this->_gauges[v];
    }
  }

  gauge_t operator[](linear_expression_t e) {
    if (this->is_bottom()) {
      return gauge_t::bottom();
    }

    gauge_t r(e.constant());
    for (typename linear_expression_t::iterator it = e.begin(); it != e.end();
         ++it) {
      Number c = it->first;
      VariableName v = it->second.name();
      if (this->_counters[v]) {
        r += gauge_t(gauge_bound_t(c, v));
      } else {
        r += c * this->_gauges[v];
      }
    }
    return r;
  }

  congruence_t to_congruence(VariableName v) { return congruence_t::top(); }

  interval_t to_interval(gauge_t g) {
    if (this->is_bottom() || g.is_bottom()) {
      return interval_t::bottom();
    } else {
      bound_t lb = g.lb().is_finite() ? this->_intervals[g.lb().lin_expr()].lb()
                                      : bound_t::minus_infinity();
      bound_t ub = g.ub().is_finite() ? this->_intervals[g.ub().lin_expr()].ub()
                                      : bound_t::plus_infinity();
      return interval_t(lb, ub);
    }
  }

  interval_t to_interval(VariableName v) {
    if (this->is_bottom()) {
      return interval_t::bottom();
    } else if (this->_counters[v]) {
      return this->_intervals[v];
    } else {
      return this->to_interval(this->_gauges[v]);
    }
  }

  interval_t to_interval(linear_expression_t e) {
    if (this->is_bottom()) {
      return interval_t::bottom();
    }

    interval_t r(e.constant());
    for (typename linear_expression_t::iterator it = e.begin(); it != e.end();
         ++it) {
      interval_t c(it->first);
      r += c * to_interval(it->second.name());
    }
    return r;
  }

  void set_interval(VariableName x, interval_t value) {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      *this = gauge_domain_t::bottom();
    } else {
      if (this->_counters[x]) {
        throw logic_error(
            "gauge_domain: trying to call set_interval() on a counter");
      } else {
        this->_gauges.set(x, gauge_t(value));
      }
    }
  }

private:
  void assign(VariableName x, gauge_t g) {
    if (this->is_bottom()) {
      return;
    } else if (g.is_bottom()) {
      *this = gauge_domain_t::bottom();
    } else if (this->_counters[x]) {
      // x := c or x := x + k
      boost::optional< gauge_bound_t > b = g.singleton();

      if (!b) {
        // The implementation of the join can give us a non-singleton interval.
        // For instance, you can get g = [1, i+1] with i -> [0, 1], we can
        // easily reduce it to g = [i+1, i+1]
        interval_t itv = this->to_interval(g);
        if (g.lb().is_finite() && this->_intervals[g.lb().lin_expr()] == itv) {
          b = g.lb();
        }
        if (g.ub().is_finite() && this->_intervals[g.ub().lin_expr()] == itv) {
          b = g.ub();
        }
      }

      if (!b) {
        throw logic_error("gauge_domain: unexpected assignment on a counter");
      } else if (b->is_constant() && b->constant() >= 0) { // x := c
        this->init_counter(x, b->constant());
      } else if (b->constant() >= 0 && b->num_coeffs() == 1 &&
                 (*b)[x] == 1) { // x := x + k
        this->incr_counter(x, b->constant());
      } else {
        throw logic_error("gauge_domain: unexpected assignment on a counter");
      }
    } else {
      this->_gauges.set(x, g);
    }
  }

public:
  /// \brief Assignment x := e
  void assign(VariableName x, linear_expression_t e) {
    this->assign(x, this->operator[](e));
  }

  /// \brief Mark the variable `x` as a non-negative loop counter
  void mark_counter(VariableName x) {
    if (this->is_bottom()) {
      return;
    }

    if (!this->_counters[x]) {
      interval_t itv = this->to_interval(x);
      ikos_assert(itv.lb() >= 0);

      if (itv.singleton()) {
        this->_sections.set(x, *itv.singleton());
      }
      this->_gauges -= x;
      this->_counters += x;
      this->_intervals.set(x, itv);
    }
  }

  /// \brief Mark the variable `x` as a normal variable,
  /// without losing information
  void unmark_counter(VariableName x) {
    if (this->is_bottom()) {
      return;
    }

    if (this->_counters[x]) {
      interval_t itv = this->_intervals[x];

      this->_sections -= x;
      this->_gauges.forget_counter(x, itv);
      this->_gauges.set(x, gauge_t(itv));
      this->_counters -= x;
      this->_intervals -= x;
    }
  }

  /// \brief Initialize a counter: x = c
  ///
  /// Precondition: c >= 0
  void init_counter(VariableName x, Number c) {
    ikos_assert(c >= 0);

    if (this->is_bottom()) {
      return;
    }

    if (this->_counters[x]) {
      this->_gauges.forget_counter(x, this->_intervals[x]);
    } else {
      this->_gauges -= x;
    }

    this->_sections.set(x, c);
    this->_counters += x;
    this->_intervals.set(x, c);
  }

  /// \brief Increment a counter: x += k
  ///
  /// Precondition: k >= 1
  void incr_counter(VariableName x, Number c) {
    ikos_assert(c >= 0);

    if (this->is_bottom()) {
      return;
    }

    this->_sections.assign(x, variable_t(x) + c);
    this->_gauges.incr_counter(x, c);
    this->_intervals.assign(x, variable_t(x) + c);
  }

  void forget_counter(VariableName x) {
    if (this->is_bottom()) {
      return;
    }

    this->_sections -= x;
    this->_gauges.forget_counter(x, this->_intervals[x]);
    this->_counters -= x;
    this->_intervals -= x;
  }

  void operator+=(linear_constraint_system_t csts) {
    if (this->is_bottom()) {
      return;
    }

    linear_constraint_system_t counters_csts;
    for (auto it = csts.begin(); it != csts.end(); ++it) {
      if (it->is_contradiction()) {
        *this = bottom();
        return;
      } else {
        gauge_t g = this->operator[](it->expression());

        if (it->is_inequality()) { // e <= 0
          if (g.lb().is_finite()) {
            counters_csts += (g.lb().lin_expr() <= 0);
          }
        } else if (it->is_equality()) { // e == 0
          if (g.lb().is_finite()) {
            counters_csts += (g.lb().lin_expr() <= 0);
          }
          if (g.ub().is_finite()) {
            counters_csts += (g.ub().lin_expr() >= 0);
          }
        } else if (it->is_disequation()) { // e != 0
          if (g.singleton()) {
            counters_csts += (g.lb().lin_expr() != 0);
          }
        } else {
          ikos_unreachable("unreachable");
        }
      }
    }
    this->_intervals += counters_csts;
    this->canonicalize();
  }

  void operator-=(VariableName v) { this->forget_num(v); }

  void forget(VariableName v) { this->forget_num(v); }

  void forget_num(VariableName v) {
    if (this->is_bottom()) {
      return;
    } else if (this->_counters[v]) {
      this->forget_counter(v);
    } else {
      this->_gauges -= v;
    }
  }

  template < typename Iterator >
  void forget(Iterator begin, Iterator end) {
    this->forget_num(begin, end);
  }

  template < typename Iterator >
  void forget_num(Iterator begin, Iterator end) {
    for (auto it = begin; it != end; ++it) {
      this->forget_num(*it);
    }
  }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    if (this->is_bottom()) {
      return;
    } else if (this->_counters[x] &&
               (op == OP_MULTIPLICATION || op == OP_DIVISION)) {
      throw logic_error("gauge_domain: unexpected assignment on a counter");
    } else {
      gauge_t yg = this->operator[](y);
      gauge_t zg = this->operator[](z);
      gauge_t xg = gauge_t::bottom();

      switch (op) {
        case OP_ADDITION: {
          xg = yg + zg;
          break;
        }
        case OP_SUBTRACTION: {
          xg = yg - zg;
          break;
        }
        case OP_MULTIPLICATION: {
          boost::optional< interval_t > yi = yg.interval();
          boost::optional< interval_t > zi = zg.interval();
          if (yi) {
            xg = zg * (*yi);
          } else if (zi) {
            xg = yg * (*zi);
          } else {
            // use interval representation
            xg = gauge_t(this->to_interval(y) * this->to_interval(z));
          }
          break;
        }
        case OP_DIVISION: {
          // use interval representation
          xg = gauge_t(this->to_interval(y) / this->to_interval(z));
          break;
        }
        default: { ikos_unreachable("invalid operation"); }
      }

      this->assign(x, xg);
    }
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    if (this->is_bottom()) {
      return;
    } else if (this->_counters[x] &&
               (op == OP_MULTIPLICATION || op == OP_DIVISION)) {
      throw logic_error("gauge_domain: unexpected assignment on a counter");
    } else {
      gauge_t yg = this->operator[](y);
      gauge_t xg = gauge_t::bottom();

      switch (op) {
        case OP_ADDITION: {
          xg = yg + gauge_t(k);
          break;
        }
        case OP_SUBTRACTION: {
          xg = yg - gauge_t(k);
          break;
        }
        case OP_MULTIPLICATION: {
          xg = yg * k;
          break;
        }
        case OP_DIVISION: {
          // use interval representation
          xg = gauge_t(this->to_interval(y) / k);
          break;
        }
        default: { ikos_unreachable("invalid operation"); }
      }

      this->assign(x, xg);
    }
  }

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             uint64_t from,
             uint64_t to) {
    if (this->is_bottom()) {
      return;
    } else {
      gauge_t yg = this->operator[](y);
      interval_t yi = this->to_interval(y);
      gauge_t xg = gauge_t::bottom();

      switch (op) {
        case OP_TRUNC: {
          Number unsigned_max = (Number(1) << Number(to)) - 1;
          Number signed_min = -(Number(1) << Number(to - 1));

          if (yi.lb() >= signed_min && yi.ub() <= unsigned_max) {
            xg = yg;
          } else {
            xg = gauge_t(yi.Trunc(from, to));
          }
          break;
        }
        case OP_ZEXT: {
          Number unsigned_max = (Number(1) << Number(from)) - 1;

          if (yi.lb() >= 0 && yi.ub() <= unsigned_max) {
            xg = yg;
          } else {
            xg = gauge_t(yi.ZExt(from, to));
          }
          break;
        }
        case OP_SEXT: {
          Number unsigned_max = (Number(1) << Number(from)) - 1;
          Number signed_min = -(Number(1) << Number(from - 1));

          if (yi.lb() >= signed_min && yi.ub() <= unsigned_max) {
            xg = yg;
          } else {
            xg = gauge_t(yi.SExt(from, to));
          }
          break;
        }
        default: { ikos_unreachable("invalid operation"); }
      }

      this->assign(x, xg);
    }
  }

  void apply(conv_operation_t op,
             VariableName x,
             Number k,
             uint64_t from,
             uint64_t to) {
    if (this->is_bottom()) {
      return;
    } else if (this->_counters[x]) {
      throw logic_error("gauge_domain: unexpected assignment on a counter");
    } else {
      // use interval representation
      interval_t yi(k);
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

      this->assign(x, gauge_t(xi));
    }
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    if (this->is_bottom()) {
      return;
    } else if (this->_counters[x]) {
      throw logic_error("gauge_domain: unexpected assignment on a counter");
    } else {
      // use interval representation
      interval_t yi = this->to_interval(y);
      interval_t zi = this->to_interval(z);
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

      this->assign(x, gauge_t(xi));
    }
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    if (this->is_bottom()) {
      return;
    } else if (this->_counters[x]) {
      throw logic_error("gauge_domain: unexpected assignment on a counter");
    } else {
      // use interval representation
      interval_t yi = this->to_interval(y);
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

      this->assign(x, gauge_t(xi));
    }
  }

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    if (this->is_bottom()) {
      return;
    } else if (this->_counters[x]) {
      throw logic_error("gauge_domain: unexpected assignment on a counter");
    } else {
      // use interval representation
      interval_t yi = this->to_interval(y);
      interval_t zi = this->to_interval(z);
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

      this->assign(x, gauge_t(xi));
    }
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    if (this->is_bottom()) {
      return;
    } else if (this->_counters[x]) {
      throw logic_error("gauge_domain: unexpected assignment on a counter");
    } else {
      // use interval representation
      interval_t yi = this->to_interval(y);
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

      this->assign(x, gauge_t(xi));
    }
  }

  void write(std::ostream& o) {
    if (this->is_bottom()) {
      o << "_|_";
    } else {
      o << "(sections=" << this->_sections << ", gauges=" << this->_gauges
        << ", counters=" << this->_counters
        << ", intervals=" << this->_intervals << ")";
    }
  }

  linear_constraint_system_t to_linear_constraint_system() {
    linear_constraint_system_t csts;

    if (this->is_bottom()) {
      csts += linear_constraint_t::contradiction();
      return csts;
    }

    csts += this->_gauges.to_linear_constraint_system();
    csts += this->_intervals.to_linear_constraint_system();
    return csts;
  }

  static std::string domain_name() { return "Gauge Domain"; }

}; // end class gauge_domain

namespace num_domain_traits {
namespace detail {

template < typename Number, typename VariableName >
struct var_to_interval_impl< gauge_domain< Number, VariableName > > {
  inline interval< Number > operator()(
      gauge_domain< Number, VariableName >& inv,
      VariableName v,
      bool /*normalize*/) {
    return inv.to_interval(v);
  }
};

template < typename Number, typename VariableName >
struct lin_expr_to_interval_impl< gauge_domain< Number, VariableName > > {
  inline interval< Number > operator()(
      gauge_domain< Number, VariableName >& inv,
      linear_expression< Number, VariableName > e,
      bool /*normalize*/) {
    return inv.to_interval(e);
  }
};

template < typename Number, typename VariableName >
struct from_interval_impl< gauge_domain< Number, VariableName > > {
  inline void operator()(gauge_domain< Number, VariableName >& inv,
                         VariableName v,
                         interval< Number > i) {
    inv.set_interval(v, i);
  }
};

} // end namespace detail
} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_GAUGES_HPP
