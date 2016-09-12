/*******************************************************************************
 *
 * Optimized version of the DBM abstract domain using variable packing.
 * The idea is to put variables together in the same equivalence class only if
 * we can express the relation between them using DBM.
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

#ifndef IKOS_VAR_PACKING_DBM_HPP
#define IKOS_VAR_PACKING_DBM_HPP

#include <ikos/domains/dbm.hpp>
#include <ikos/domains/var_packing_domains.hpp>
#include <ikos/domains/abstract_domains_api.hpp>

namespace ikos {

template < typename Number, typename VariableName >
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
  typedef interval_domain< Number, VariableName > interval_domain_t;
  typedef var_packing_dbm< Number, VariableName > var_packing_dbm_t;

  template < typename A, typename B, std::size_t C, int D >
  friend class var_packing_dbm_congruence;

private:
  typedef var_packing_domain< Number,
                              VariableName,
                              dbm< Number, VariableName > >
      var_packing_domain_t;

private:
  var_packing_domain_t _domain;

private:
  var_packing_dbm(const var_packing_domain_t& domain) : _domain(domain) {}

  // Check the satisfiability of a linear constraint using intervals
  bool satisfies(linear_constraint_t cst) {
    interval_domain_t inv;
    for (typename linear_expression_t::iterator it = cst.begin();
         it != cst.end();
         ++it) {
      inv.set(it->second.name(), _domain.to_interval(it->second.name()));
    }
    inv += cst;
    return !inv.is_bottom();
  }

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

  // convert an interval_domain to a var_packing_dbm
  explicit var_packing_dbm(interval_domain_t o) : _domain(o) {}

  // convert a var_packing_dbm to an interval_domain
  interval_domain_t get_interval_domain() {
    return _domain.get_interval_domain();
  }

  bool operator<=(var_packing_dbm_t o) { return _domain <= o._domain; }

  var_packing_dbm_t operator|(var_packing_dbm_t o) {
    return var_packing_dbm_t(_domain | o._domain);
  }

  var_packing_dbm_t operator&(var_packing_dbm_t o) {
    return var_packing_dbm_t(_domain & o._domain);
  }

  var_packing_dbm_t operator||(var_packing_dbm_t o) {
    return var_packing_dbm_t(_domain || o._domain);
  }

  var_packing_dbm_t operator&&(var_packing_dbm_t o) {
    return var_packing_dbm_t(_domain && o._domain);
  }

  void operator-=(VariableName v) { _domain -= v; }

  void operator-=(variable_set_t variables) { _domain -= variables; }

  void operator+=(linear_constraint_t cst) {
    if (_domain._is_bottom)
      return;

    if (cst.size() == 0) {
      _domain += cst;
      return;
    }

    typename linear_expression_t::iterator it = cst.begin();
    typename linear_expression_t::iterator it2 = ++cst.begin();

    if ((cst.size() == 1 && it->first == 1) ||
        (cst.size() == 1 && it->first == -1) ||
        (cst.size() == 2 && it->first == 1 && it2->first == -1) ||
        (cst.size() == 2 && it->first == -1 && it2->first == 1)) {
      _domain += cst;
    } else {
      // all other cases are not handled by the dbm domain
      if (!satisfies(cst))
        _domain.set_bottom();
    }
  }

  void operator+=(linear_constraint_system_t cst) {
    for (typename linear_constraint_system_t::iterator it = cst.begin();
         it != cst.end();
         ++it) {
      this->operator+=(*it);
    }
  }

  interval_t operator[](VariableName x) { return _domain[x]; }

  void set(VariableName x, interval_t intv) { _domain.set(x, intv); }

  void assign(VariableName x, linear_expression_t e) {
    if (_domain._is_bottom)
      return;

    if (e.is_constant() || (e.size() == 1 && e.begin()->first == 1)) {
      _domain.assign(x, e);
    } else {
      // Projection using intervals
      _domain.set(x, _domain.to_interval(e));
    }
  }

  // x = y op z
  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    if (_domain.is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = _domain.to_interval(y);
    interval_t v_z = _domain.to_interval(z);

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
      default: { throw ikos_error("unreachable"); }
    }

    _domain.set(x, v_x);
  }

  // x = y op k
  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    if (_domain.is_bottom())
      return;

    if (op == OP_ADDITION || op == OP_SUBTRACTION ||
        (op == OP_MULTIPLICATION && k == 1) || (op == OP_DIVISION && k == 1)) {
      _domain.apply(op, x, y, k);
    } else if (op == OP_MULTIPLICATION) {
      _domain.set(x, _domain.to_interval(y) * k);
    } else if (op == OP_DIVISION) {
      _domain.set(x, _domain.to_interval(y) / k);
    } else {
      throw ikos_error("unreachable");
    }
  }

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             unsigned width) {
    if (_domain.is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = _domain.to_interval(y);

    switch (op) {
      case OP_TRUNC: {
        v_x = v_y.Trunc(width);
        break;
      }
      case OP_ZEXT: {
        v_x = v_y.ZExt(width);
        break;
      }
      case OP_SEXT: {
        v_x = v_y.SExt(width);
        break;
      }
      default: { throw ikos_error("unreachable"); }
    }

    _domain.set(x, v_x);
  }

  void apply(conv_operation_t op, VariableName x, Number k, unsigned width) {
    if (_domain.is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y(k);

    switch (op) {
      case OP_TRUNC: {
        v_x = v_y.Trunc(width);
        break;
      }
      case OP_ZEXT: {
        v_x = v_y.ZExt(width);
        break;
      }
      case OP_SEXT: {
        v_x = v_y.SExt(width);
        break;
      }
      default: { throw ikos_error("unreachable"); }
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
    interval_t v_y = _domain.to_interval(y);
    interval_t v_z = _domain.to_interval(z);

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
      default: { throw ikos_error("unreachable"); }
    }

    _domain.set(x, v_x);
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    if (_domain.is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = _domain.to_interval(y);
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
      default: { throw ikos_error("unreachable"); }
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
    interval_t v_y = _domain.to_interval(y);
    interval_t v_z = _domain.to_interval(z);

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
      default: { throw ikos_error("unreachable"); }
    }

    _domain.set(x, v_x);
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    if (_domain.is_bottom())
      return;

    interval_t v_x = interval_t::top();
    interval_t v_y = _domain.to_interval(y);
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
      default: { throw ikos_error("unreachable"); }
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
  var_packing_dbm< Number, VariableName > operator()(
      interval_domain< Number, VariableName > inv) {
    return var_packing_dbm< Number, VariableName >(inv);
  }
};

template < typename Number, typename VariableName >
struct convert_impl< var_packing_dbm< Number, VariableName >,
                     interval_domain< Number, VariableName > > {
  interval_domain< Number, VariableName > operator()(
      var_packing_dbm< Number, VariableName > inv) {
    return inv.get_interval_domain();
  }
};

} // end namespace detail
} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_VAR_PACKING_DBM_HPP
