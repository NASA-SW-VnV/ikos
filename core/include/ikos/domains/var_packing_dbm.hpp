/**************************************************************************/ /**
 *
 * \file
 * \brief Optimized version of the DBM abstract domain using variable packing.
 *
 * The idea is to put variables together in the same equivalence class only if
 * we can express the relation between them using DBM.
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

#ifndef IKOS_VAR_PACKING_DBM_HPP
#define IKOS_VAR_PACKING_DBM_HPP

#include <ikos/domains/dbm.hpp>
#include <ikos/domains/var_packing_domains.hpp>
#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/value/congruence.hpp>
#include <ikos/value/interval.hpp>

namespace ikos {

/// \brief DBM abstract domain using variable packing
template < typename Number,
           typename VariableName,
           std::size_t max_reduction_cycles = 10 >
class var_packing_dbm : public abstract_domain,
                        public numerical_domain< Number, VariableName >,
                        public bitwise_operators< Number, VariableName >,
                        public division_operators< Number, VariableName > {
public:
  typedef variable< Number, VariableName > variable_t;
  typedef patricia_tree_set< variable_t > variable_set_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;
  typedef interval< Number > interval_t;
  typedef congruence< Number > congruence_t;
  typedef interval_domain< Number, VariableName > interval_domain_t;
  typedef var_packing_dbm< Number, VariableName, max_reduction_cycles >
      var_packing_dbm_t;

  template < typename A, typename B, std::size_t C, int D >
  friend class var_packing_dbm_congruence;

private:
  typedef linear_interval_solver< Number, VariableName, var_packing_dbm_t >
      solver_t;
  typedef var_packing_domain<
      Number,
      VariableName,
      dbm< Number, VariableName, max_reduction_cycles > > var_packing_domain_t;

private:
  var_packing_domain_t _domain;

private:
  var_packing_dbm(const var_packing_domain_t& domain) : _domain(domain) {}

public:
  static var_packing_dbm_t top() {
    return var_packing_dbm_t(var_packing_domain_t::top());
  }

  static var_packing_dbm_t bottom() {
    return var_packing_dbm_t(var_packing_domain_t::bottom());
  }

public:
  var_packing_dbm() : _domain() {}

  var_packing_dbm(const var_packing_dbm_t& o) : _domain(o._domain) {}

  var_packing_dbm_t& operator=(const var_packing_dbm_t& o) {
    _domain = o._domain;
    return *this;
  }

  bool is_bottom() { return _domain.is_bottom(); }

  bool is_top() { return _domain.is_top(); }

  variable_set_t variables() const { return _domain.variables(); }

  /// \brief Convert an interval_domain to a var_packing_dbm
  explicit var_packing_dbm(interval_domain_t o) : _domain(o) {}

  /// \brief Convert a var_packing_dbm to an interval_domain
  interval_domain_t get_interval_domain() {
    return _domain.get_interval_domain();
  }

  bool operator<=(var_packing_dbm_t o) { return _domain <= o._domain; }

  var_packing_dbm_t operator|(var_packing_dbm_t o) {
    return var_packing_dbm_t(_domain | o._domain);
  }

  var_packing_dbm_t operator||(var_packing_dbm_t o) {
    return var_packing_dbm_t(_domain || o._domain);
  }

  var_packing_dbm_t join_loop(var_packing_dbm_t o) {
    return var_packing_dbm_t(_domain.join_loop(o._domain));
  }

  var_packing_dbm_t join_iter(var_packing_dbm_t o) {
    return var_packing_dbm_t(_domain.join_iter(o._domain));
  }

  var_packing_dbm_t operator&(var_packing_dbm_t o) {
    return var_packing_dbm_t(_domain & o._domain);
  }

  var_packing_dbm_t operator&&(var_packing_dbm_t o) {
    return var_packing_dbm_t(_domain && o._domain);
  }

  void operator-=(VariableName v) { forget_num(v); }

  void forget(VariableName v) { forget_num(v); }

  void forget_num(VariableName v) { _domain.forget_num(v); }

  template < typename Iterator >
  void forget(Iterator begin, Iterator end) {
    forget_num(begin, end);
  }

  template < typename Iterator >
  void forget_num(Iterator begin, Iterator end) {
    _domain.forget_num(begin, end);
  }

  void operator+=(linear_constraint_t cst) {
    if (_domain._is_bottom)
      return;

    if (cst.size() == 0) {
      if (cst.is_contradiction()) {
        _domain.set_bottom();
      }
      return;
    }

    typename linear_expression_t::iterator it = cst.begin();
    typename linear_expression_t::iterator it2 = ++cst.begin();

    if ((cst.is_inequality() || cst.is_equality()) &&
        ((cst.size() == 1 && it->first == 1) ||
         (cst.size() == 1 && it->first == -1) ||
         (cst.size() == 2 && it->first == 1 && it2->first == -1) ||
         (cst.size() == 2 && it->first == -1 && it2->first == 1))) {
      _domain += cst;
    } else {
      _domain.normalize();

      if (is_bottom())
        return;

      linear_constraint_system_t csts;
      csts += cst;
      solver_t solver(csts, max_reduction_cycles);
      solver.run(*this);
    }
  }

  void operator+=(linear_constraint_system_t csts) {
    if (_domain._is_bottom)
      return;

    linear_constraint_system_t remaining_csts;

    for (linear_constraint_t cst : csts) {
      // process each constraint
      if (cst.size() == 0) {
        if (cst.is_contradiction()) {
          _domain.set_bottom();
          return;
        }
      } else if (cst.is_inequality() || cst.is_equality()) {
        typename linear_expression_t::iterator it = cst.begin();
        typename linear_expression_t::iterator it2 = ++cst.begin();

        if ((cst.size() == 1 && it->first == 1) ||
            (cst.size() == 1 && it->first == -1) ||
            (cst.size() == 2 && it->first == 1 && it2->first == -1) ||
            (cst.size() == 2 && it->first == -1 && it2->first == 1)) {
          this->operator+=(cst);
        } else {
          remaining_csts += cst;
        }
      } else {
        remaining_csts += cst;
      }
    }

    if (!remaining_csts.empty()) {
      _domain.normalize();

      if (is_bottom())
        return;

      solver_t solver(remaining_csts, max_reduction_cycles);
      solver.run(*this);
    }
  }

  congruence_t to_congruence(VariableName x) {
    return _domain.to_congruence(x);
  }

  interval_t to_interval(VariableName x, bool normalize) {
    return _domain.to_interval(x, normalize);
  }

  interval_t to_interval(linear_expression_t e, bool normalize) {
    return _domain.to_interval(e, normalize);
  }

  void set(VariableName x, interval_t intv) { _domain.set(x, intv); }

  void refine(VariableName x, interval_t intv) { _domain.refine(x, intv); }

  void assign(VariableName x, linear_expression_t e) {
    if (_domain._is_bottom)
      return;

    if (e.is_constant() || (e.size() == 1 && e.begin()->first == 1)) {
      _domain.assign(x, e);
    } else {
      // Projection using intervals
      _domain.set(x, _domain.to_interval(e, true));
    }
  }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    if (_domain.is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = _domain.to_interval(y, true);
    interval_t v_z = _domain.to_interval(z, true);

    switch (op) {
      case OP_ADDITION: {
        if (v_z.singleton()) { // x = y + c
          apply(op, x, y, *v_z.singleton());
          return;
        } else if (v_y.singleton()) { // x = c + z
          apply(op, x, z, *v_y.singleton());
          return;
        }
        v_x = v_y + v_z;
        break;
      }
      case OP_SUBTRACTION: {
        if (v_z.singleton()) { // x = y - c
          apply(op, x, y, *v_z.singleton());
          return;
        }
        v_x = v_y - v_z;
        break;
      }
      case OP_MULTIPLICATION: {
        if (v_z.singleton()) { // x = y * c
          apply(op, x, y, *v_z.singleton());
          return;
        } else if (v_y.singleton()) { // x = c * z
          apply(op, x, z, *v_y.singleton());
          return;
        }
        v_x = v_y * v_z;
        break;
      }
      case OP_DIVISION: {
        if (v_z.singleton()) { // x = y / c
          apply(op, x, y, *v_z.singleton());
          return;
        }
        v_x = v_y / v_z;
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }

    _domain.set(x, v_x);
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    if (_domain.is_bottom())
      return;

    if (op == OP_ADDITION || op == OP_SUBTRACTION ||
        (op == OP_MULTIPLICATION && k == 1) || (op == OP_DIVISION && k == 1)) {
      _domain.apply(op, x, y, k);
    } else if (op == OP_MULTIPLICATION) {
      _domain.set(x, _domain.to_interval(y, true) * k);
    } else if (op == OP_DIVISION) {
      _domain.set(x, _domain.to_interval(y, true) / k);
    } else {
      ikos_unreachable("invalid operation");
    }
  }

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             uint64_t from,
             uint64_t to) {
    if (_domain.is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = _domain.to_interval(y, true);

    switch (op) {
      case OP_TRUNC: {
        v_x = v_y.Trunc(from, to);
        break;
      }
      case OP_ZEXT: {
        v_x = v_y.ZExt(from, to);
        break;
      }
      case OP_SEXT: {
        v_x = v_y.SExt(from, to);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }

    _domain.set(x, v_x);
  }

  void apply(conv_operation_t op,
             VariableName x,
             Number k,
             uint64_t from,
             uint64_t to) {
    if (_domain.is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y(k);

    switch (op) {
      case OP_TRUNC: {
        v_x = v_y.Trunc(from, to);
        break;
      }
      case OP_ZEXT: {
        v_x = v_y.ZExt(from, to);
        break;
      }
      case OP_SEXT: {
        v_x = v_y.SExt(from, to);
        break;
      }
      default: { ikos_unreachable("invalid operation"); }
    }

    _domain.set(x, v_x);
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    if (_domain.is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = _domain.to_interval(y, true);
    interval_t v_z = _domain.to_interval(z, true);

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

    _domain.set(x, v_x);
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    if (_domain.is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = _domain.to_interval(y, true);
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

    _domain.set(x, v_x);
  }

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    if (_domain.is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = _domain.to_interval(y, true);
    interval_t v_z = _domain.to_interval(z, true);

    switch (op) {
      case OP_SDIV: {
        v_x = v_y / v_z;
        break;
      }
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

    _domain.set(x, v_x);
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    if (_domain.is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = _domain.to_interval(y, true);
    interval_t v_z(k);

    switch (op) {
      case OP_SDIV: {
        v_x = v_y / v_z;
        break;
      }
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

    _domain.set(x, v_x);
  }

  linear_constraint_system_t to_linear_constraint_system() {
    return _domain.to_linear_constraint_system();
  }

  void write(std::ostream& o) { _domain.write(o); }

  static std::string domain_name() { return "DBM with Variable Packing"; }

}; // end class var_packing_dbm

namespace num_domain_traits {
namespace detail {

template < typename Number, typename VariableName >
struct convert_impl< interval_domain< Number, VariableName >,
                     var_packing_dbm< Number, VariableName > > {
  inline var_packing_dbm< Number, VariableName > operator()(
      interval_domain< Number, VariableName > inv) {
    return var_packing_dbm< Number, VariableName >(inv);
  }
};

template < typename Number, typename VariableName >
struct convert_impl< var_packing_dbm< Number, VariableName >,
                     interval_domain< Number, VariableName > > {
  inline interval_domain< Number, VariableName > operator()(
      var_packing_dbm< Number, VariableName > inv) {
    return inv.get_interval_domain();
  }
};

template < typename Number, typename VariableName >
struct var_to_interval_impl< var_packing_dbm< Number, VariableName > > {
  inline interval< Number > operator()(
      var_packing_dbm< Number, VariableName >& inv,
      VariableName v,
      bool normalize) {
    return inv.to_interval(v, normalize);
  }
};

template < typename Number, typename VariableName >
struct lin_expr_to_interval_impl< var_packing_dbm< Number, VariableName > > {
  inline interval< Number > operator()(
      var_packing_dbm< Number, VariableName >& inv,
      linear_expression< Number, VariableName > e,
      bool normalize) {
    return inv.to_interval(e, normalize);
  }
};

} // end namespace detail
} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_VAR_PACKING_DBM_HPP
