/*******************************************************************************
 *
 * Convert ARBOS operands to a symbolic ARBOS-independent
 * representation.
 *
 * Author: Jorge A. Navas
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

#ifndef ANALYZER_LITERAL_HPP
#define ANALYZER_LITERAL_HPP

#include <tuple>
#include <unordered_map>
#include <utility>

#include <boost/optional.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/ar-wrapper/wrapper.hpp>

namespace analyzer {

using namespace arbos;

class LiteralFactory;
class VariableFactory;

//! A literal is either a variable, a (integer) number or a constant.
class Literal {
  friend class LiteralFactory;

  //! An integer constant is marked as a number. Floating point (fp)
  //  numbers are considered as uninterpreted constants since analyses
  //  are not expected to reason about fp numbers. We specially keep
  //  track of undefined and null constants.

  typedef enum { Undefined, Null, Float, Other } cst_kind_t;
  typedef boost::optional< std::string > var_t;
  typedef boost::optional< cst_kind_t > cst_t;
  typedef boost::optional< std::pair< std::string, unsigned > >
      num_t; //! A number is pair of
             // a string (value) and size
             // in bits.

  //  Note that we could have a virtual class Literal and then
  //  subclasses for Variable, Cst, and Number. For efficiency, we
  //  prefer not to do that.

  typedef std::tuple< var_t, num_t, cst_t > type;

  type _lit;
  VariableFactory& _vfac;

  type make_var(std::string v) {
    return std::make_tuple(var_t(v), num_t(), cst_t());
  }

  type make_num(std::string val, unsigned size) {
    return std::make_tuple(var_t(), num_t(std::make_pair(val, size)), cst_t());
  }

  type make_undefined_constant() {
    return std::make_tuple(var_t(), num_t(), cst_t(Undefined));
  }

  type make_null_constant() {
    return std::make_tuple(var_t(), num_t(), cst_t(Null));
  }

  type make_float_constant() {
    return std::make_tuple(var_t(), num_t(), cst_t(Float));
  }

  type make_constant() {
    return std::make_tuple(var_t(), num_t(), cst_t(Other));
  }

  type lookup(const Operand_ref& o) {
    if (ar::ar_internal::is_var_operand(o)) {
      Var_Operand_ref v = node_cast< Var_Operand >(o);
      return make_var(ar::getName(v));
    }

    assert(ar::ar_internal::is_cst_operand(o));
    Cst_Operand_ref cst_operand = node_cast< Cst_Operand >(o);
    Constant_ref cst = ar::getConstant(cst_operand);

    if (ar::ar_internal::is_var_addr_constant(cst)) {
      Var_Addr_Constant_ref var_cst = node_cast< Var_Addr_Constant >(cst);
      return make_var(ar::getName(var_cst));
    }

    if (ar::ar_internal::is_function_addr_constant(cst)) {
      Function_Addr_Constant_ref faddr =
          node_cast< Function_Addr_Constant >(cst);
      return make_var(ar::getFunctionName(faddr));
    }

    if (ar::ar_internal::is_int_constant(cst)) {
      Integer_Constant_ref n = node_cast< Integer_Constant >(cst);
      return make_num(ar::getZValueStr(n), ar::getSize(n));
    }

    if (ar::ar_internal::is_float_constant(cst)) {
      return make_float_constant();
    }

    if (ar::ar_internal::is_undefined_constant(cst)) {
      return make_undefined_constant();
    }

    if (ar::ar_internal::is_null_constant(cst)) {
      return make_null_constant();
    }

    // otherwise, we just convert it to a generic constant
    return make_constant();
  }

  type lookup(const Local_Variable_ref& lv) {
    Variable_ref v = node_cast< Variable >(lv);
    return make_var(ar::getName(v));
  }

  type lookup(const Internal_Variable_ref& v) {
    return make_var(ar::getName(v));
  }

  //! Constructors

  Literal(VariableFactory& vfac)
      : _lit(std::make_tuple(var_t(), num_t(), cst_t(Undefined))),
        _vfac(vfac) {}

  Literal(Operand_ref op, VariableFactory& vfac)
      : _lit(lookup(op)), _vfac(vfac) {}

  Literal(Local_Variable_ref var, VariableFactory& vfac)
      : _lit(lookup(var)), _vfac(vfac) {}

  Literal(Internal_Variable_ref var, VariableFactory& vfac)
      : _lit(lookup(var)), _vfac(vfac) {}

  Literal(uint64_t val, VariableFactory& vfac)
      : _lit(make_num(std::to_string(val), -1)), _vfac(vfac) {}

  // Do not implement
  Literal& operator=(const Literal& other);

public:
  template < typename Number >
  static Number make_num(uint64_t n, unsigned int bitwidth = 32) {
#if 1
    return Number(n);
#else
    return Number(n, bitwidth);
#endif
  }

  template < typename Number >
  static bool is_zero(Number n) {
    return n == Number(n);
  }

public:
  Literal(const Literal& other) : _lit(other._lit), _vfac(other._vfac) {}

  bool is_var() const {
    var_t _var;
    num_t _num;
    cst_t _cst;
    std::tie(_var, _num, _cst) = _lit;
    return _var;
  }

  bool is_num() const {
    var_t _var;
    num_t _num;
    cst_t _cst;
    std::tie(_var, _num, _cst) = _lit;
    return _num;
  }

  varname_t get_var() const {
    var_t _var;
    num_t _num;
    cst_t _cst;
    std::tie(_var, _num, _cst) = _lit;
    assert(_var);
    return this->_vfac[*_var];
  }

  std::string get_var_str() const {
    var_t _var;
    num_t _num;
    cst_t _cst;
    std::tie(_var, _num, _cst) = _lit;
    assert(_var);
    return *_var;
  }

  template < typename Number >
  Number get_num() const {
    var_t _var;
    num_t _num;
    cst_t _cst;
    std::tie(_var, _num, _cst) = _lit;
    assert(_num);
#if 1
    return Number((*_num).first);
#else
    return Number((*_num).first, (*_num).second);
#endif
  }

  bool is_cst() const {
    var_t _var;
    num_t _num;
    cst_t _cst;
    std::tie(_var, _num, _cst) = _lit;
    return _cst;
  }

  bool is_float_cst() const {
    var_t _var;
    num_t _num;
    cst_t _cst;
    std::tie(_var, _num, _cst) = _lit;
    return (_cst && (*_cst == Float));
  }

  bool is_undefined_cst() const {
    var_t _var;
    num_t _num;
    cst_t _cst;
    std::tie(_var, _num, _cst) = _lit;
    return (_cst && (*_cst == Undefined));
  }

  bool is_null_cst() const {
    var_t _var;
    num_t _num;
    cst_t _cst;
    std::tie(_var, _num, _cst) = _lit;
    return (_cst && (*_cst == Null));
  }

  std::ostream& dump(std::ostream& o) const {
    var_t var;
    num_t num;
    cst_t cst;
    std::tie(var, num, cst) = _lit;
    o << "(";
    if (var)
      o << *var;
    else
      o << "_";
    o << ",";
    if (num)
      o << (*num).first;
    else
      o << "_";
    o << ",";
    if (cst) {
      switch (*cst) {
        case Undefined:
          o << "$undefined";
          break;
        case Null:
          o << "$null";
          break;
        case Float:
          o << "$float";
          break;
        default:
          o << "$unknown_cst";
          break;
      }
    } else
      o << "_";
    o << ")";

    return o;
  }

}; // end class Literal

//! A factory of literals
class LiteralFactory : public boost::noncopyable {
  typedef std::unordered_map< size_t, Literal > map_t;
  typedef map_t::iterator iterator;
  typedef map_t::value_type binding_t;

  map_t _map;
  VariableFactory& _vfac;

  template < typename T >
  Literal& lookup(T e) {
    iterator it = _map.find(hash_value(e));
    if (it == _map.end()) {
      Literal lit(e, _vfac);
      std::pair< iterator, bool > res =
          _map.insert(binding_t(hash_value(e), lit));
      return (res.first)->second;
    } else
      return it->second;
  }

public:
  LiteralFactory(VariableFactory& vfac) : _vfac(vfac) {}

  Literal& operator[](Operand_ref op) { return lookup(op); }
  Literal& operator[](Local_Variable_ref v) { return lookup(v); }
  Literal& operator[](Internal_Variable_ref v) { return lookup(v); }
  Literal operator[](uint64_t v) { return Literal(v, _vfac); }

  std::ostream& dump(std::ostream& o) {
    for (iterator it = _map.begin(); it != _map.end(); ++it) {
      o << it->first << " --> ";
      it->second.dump(o);
      o << "\n";
    }
    return o;
  }
};

} // end namespace analyzer

#endif // ANALYZER_LITERAL_HPP
