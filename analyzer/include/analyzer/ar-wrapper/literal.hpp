/*******************************************************************************
 *
 * Convert ARBOS operands to a symbolic ARBOS-independent
 * representation.
 *
 * Author: Jorge A. Navas
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

#ifndef ANALYZER_LITERAL_HPP
#define ANALYZER_LITERAL_HPP

#include <tuple>
#include <unordered_map>
#include <utility>

#include <boost/optional.hpp>

#include <ikos/algorithms/literal.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/ar-wrapper/aggregate_literal.hpp>
#include <analyzer/ar-wrapper/wrapper.hpp>

namespace analyzer {

using namespace arbos;

typedef ikos::literal< number_t, ikos::dummy_number, varname_t > scalar_lit_t;

typedef aggregate_literal< number_t, ikos::dummy_number, varname_t >
    aggregate_lit_t;

// Exception for unexpected literals.
// For instance, this is raised when you call lfac[operand]
// (i.e, lfac.lookup_scalar(operand)) and an aggregate_literal is found.
class unexpected_literal_error : public analyzer_error {
public:
  unexpected_literal_error(const std::string& msg) : analyzer_error(msg) {}

}; // end class unexpected_literal_error

class scalar_literal_error : public unexpected_literal_error {
private:
  scalar_lit_t _lit;

public:
  scalar_literal_error(const scalar_lit_t& lit)
      : unexpected_literal_error("literal factory: unexpected scalar literal"),
        _lit(lit) {}

  const scalar_lit_t& lit() const { return _lit; }

}; // end class scalar_literal_error

class aggregate_literal_error : public unexpected_literal_error {
private:
  aggregate_lit_t _lit;

public:
  aggregate_literal_error(const aggregate_lit_t& lit)
      : unexpected_literal_error(
            "literal factory: unexpected aggregate literal"),
        _lit(lit) {}

  const aggregate_lit_t& lit() const { return _lit; }

}; // end class aggregate_literal_error

class void_var_literal_error : public unexpected_literal_error {
private:
  varname_t _var;

public:
  void_var_literal_error(varname_t var)
      : unexpected_literal_error("literal factory: unexpected void variable"),
        _var(var) {}

  varname_t var() const { return _var; }

}; // end class void_var_literal_error

class LiteralFactory : public boost::noncopyable {
private:
  // special literal for void variables
  struct void_var_lit_t {
    varname_t var;
  };

  typedef boost::variant< scalar_lit_t, aggregate_lit_t, void_var_lit_t >
      literal_t;

  typedef std::unordered_map< std::size_t, literal_t > map_t;

private:
  VariableFactory& _vfac;
  map_t _map;

private:
  static number_t to_number(const arbos::z_number& n) {
    return number_t(static_cast< mpz_class >(n));
  }

  // Create a variable literal
  literal_t create_literal(varname_t name, Type_ref type) {
    if (ar::ar_internal::is_integer_type(type)) {
      return literal_t(scalar_lit_t::integer_var(name));
    } else if (ar::ar_internal::is_float_type(type)) {
      return literal_t(scalar_lit_t::floating_point_var(name));
    } else if (ar::ar_internal::is_pointer_type(type)) {
      return literal_t(scalar_lit_t::pointer_var(name));
    } else if (ar::ar_internal::is_void_type(type)) {
      return literal_t(void_var_lit_t{name});
    } else if (ar::ar_internal::is_struct_type(type) ||
               ar::ar_internal::is_array_type(type)) {
      return literal_t(aggregate_lit_t::var(name, ar::getSize(type)));
    } else {
      throw analyzer_error("literal factory: unexpected variable type");
    }
  }

  // Create a constant literal
  literal_t create_literal(Constant_ref cst) {
    if (ar::ar_internal::is_int_constant(cst)) {
      Integer_Constant_ref n = node_cast< Integer_Constant >(cst);
      return literal_t(scalar_lit_t::integer(to_number(ar::getZValue(n))));
    } else if (ar::ar_internal::is_float_constant(cst)) {
      return literal_t(scalar_lit_t::floating_point(ikos::dummy_number()));
    } else if (ar::ar_internal::is_undefined_constant(cst)) {
      Type_ref type = ar::getType(cst);
      if (ar::ar_internal::is_struct_type(type) ||
          ar::ar_internal::is_array_type(type)) {
        return literal_t(aggregate_lit_t::undefined(ar::getSize(type)));
      } else {
        return literal_t(scalar_lit_t::undefined());
      }
    } else if (ar::ar_internal::is_null_constant(cst)) {
      return literal_t(scalar_lit_t::null());
    } else if (ar::ar_internal::is_range_constant(cst)) {
      Range_Constant_ref range_cst = node_cast< Range_Constant >(cst);
      return create_literal(range_cst);
    } else if (ar::ar_internal::is_var_addr_constant(cst)) {
      Var_Addr_Constant_ref var_cst = node_cast< Var_Addr_Constant >(cst);
      Variable_ref var = var_cst->getVariable();
      if (arbos::ar::ar_internal::is_global_var(var)) {
        return create_literal(_vfac.get_global(
                                  node_cast< Global_Variable >(var)),
                              ar::getType(var_cst));
      } else if (arbos::ar::ar_internal::is_local_var(var)) {
        return create_literal(_vfac.get_local(node_cast< Local_Variable >(var)),
                              ar::getType(var_cst));
      } else {
        throw analyzer_error(
            "Var_Addr_Constant is neither a local nor a global variable");
      }
    } else if (ar::ar_internal::is_function_addr_constant(cst)) {
      Function_Addr_Constant_ref fun_addr =
          node_cast< Function_Addr_Constant >(cst);
      return literal_t(
          scalar_lit_t::pointer_var(_vfac.get_function_addr(fun_addr)));
    } else {
      throw analyzer_error("literal factory: unexpected constant operand");
    }
  }

  literal_t create_literal(Range_Constant_ref range) {
    aggregate_lit_t::fields_t fields;

    for (auto it = range->getValues().begin(); it != range->getValues().end();
         ++it) {
      literal_t value = create_literal(it->second);
      add_aggregate_field vis(fields,
                              to_number(it->first),
                              ar::getSize(ar::getType(it->second)));
      boost::apply_visitor(vis, value);
    }

    return literal_t(
        aggregate_lit_t::cst(fields, ar::getSize(ar::getType(range))));
  }

  struct add_aggregate_field : public boost::static_visitor<> {
    aggregate_lit_t::fields_t& fields;
    number_t offset;
    number_t size;

    add_aggregate_field(aggregate_lit_t::fields_t& _fields,
                        number_t _offset,
                        number_t _size)
        : fields(_fields), offset(_offset), size(_size) {}

    void operator()(const scalar_lit_t& scalar) {
      fields.push_back(aggregate_lit_t::field_t{offset, scalar, size});
    }

    void operator()(const aggregate_lit_t& aggregate) {
      if (aggregate.is_cst()) {
        for (auto it = aggregate.fields().begin();
             it != aggregate.fields().end();
             ++it) {
          fields.push_back(aggregate_lit_t::field_t{offset + it->offset,
                                                    it->value,
                                                    it->size});
        }
      } else if (aggregate.is_undefined()) {
        fields.push_back(
            aggregate_lit_t::field_t{offset, scalar_lit_t::undefined(), size});
      } else if (aggregate.is_var()) {
        throw analyzer_error(
            "literal factory: unexpected variable aggregate within a constant "
            "aggregate");
      } else {
        throw analyzer_error("unreachable");
      }
    }

    void operator()(const void_var_lit_t&) {
      // ignore
    }

  }; // end struct add_aggregate_field

  literal_t create_literal(Operand_ref op) {
    if (ar::ar_internal::is_var_operand(op)) {
      Var_Operand_ref v = node_cast< Var_Operand >(op);
      Internal_Variable_ref var = v->getInternalVariable();
      return create_literal(_vfac.get_internal(
                                node_cast< Internal_Variable >(var)),
                            ar::getType(v));
    } else if (ar::ar_internal::is_cst_operand(op)) {
      Cst_Operand_ref cst_operand = node_cast< Cst_Operand >(op);
      Constant_ref cst = ar::getConstant(cst_operand);
      return create_literal(cst);
    } else {
      throw analyzer_error("literal factory: unexpected operand");
    }
  }

  literal_t create_literal(Local_Variable_ref v) {
    return create_literal(_vfac.get_local(v), ar::getType(v));
  }

  literal_t create_literal(Internal_Variable_ref v) {
    return create_literal(_vfac.get_internal(v), ar::getType(v));
  }

  literal_t create_literal(Var_Addr_Constant_ref v) {
    Variable_ref var = v->getVariable();
    if (arbos::ar::ar_internal::is_global_var(var)) {
      return create_literal(_vfac.get_global(node_cast< Global_Variable >(var)),
                            ar::getType(v));
    } else if (arbos::ar::ar_internal::is_local_var(var)) {
      return create_literal(_vfac.get_local(node_cast< Local_Variable >(var)),
                            ar::getType(v));
    } else {
      throw analyzer_error(
          "Var_Addr_Constant is neither a local nor a global variable");
    }
  }

  template < typename T >
  const literal_t& lookup(T e) {
    map_t::iterator it = _map.find(hash_value(e));
    if (it == _map.end()) {
      std::pair< map_t::iterator, bool > res =
          _map.insert(map_t::value_type(hash_value(e), create_literal(e)));
      return (res.first)->second;
    } else {
      return it->second;
    }
  }

  struct get_scalar_lit : public boost::static_visitor< const scalar_lit_t& > {
    const scalar_lit_t& operator()(const scalar_lit_t& scalar) const {
      return scalar;
    }

    const scalar_lit_t& operator()(const aggregate_lit_t& aggregate) const {
      throw aggregate_literal_error(aggregate);
    }

    const scalar_lit_t& operator()(const void_var_lit_t& lit) const {
      throw void_var_literal_error(lit.var);
    }

  }; // end struct get_scalar_lit

  struct get_aggregate_lit
      : public boost::static_visitor< const aggregate_lit_t& > {
    const aggregate_lit_t& operator()(const scalar_lit_t& scalar) const {
      throw scalar_literal_error(scalar);
    }

    const aggregate_lit_t& operator()(const aggregate_lit_t& aggregate) const {
      return aggregate;
    }

    const aggregate_lit_t& operator()(const void_var_lit_t& lit) const {
      throw void_var_literal_error(lit.var);
    }

  }; // end struct get_aggregate_lit

public:
  LiteralFactory(VariableFactory& vfac) : _vfac(vfac) {}

  template < typename T >
  const scalar_lit_t& lookup_scalar(T e) {
#if BOOST_VERSION == 105800
    // workaround for https://svn.boost.org/trac10/ticket/11285
    const literal_t& lit = lookup(e);
    get_scalar_lit vis;
    return lit.apply_visitor(vis);
#else
    return boost::apply_visitor(get_scalar_lit(), lookup(e));
#endif
  }

  template < typename T >
  const aggregate_lit_t& lookup_aggregate(T e) {
#if BOOST_VERSION == 105800
    // workaround for https://svn.boost.org/trac10/ticket/11285
    const literal_t& lit = lookup(e);
    get_aggregate_lit vis;
    return lit.apply_visitor(vis);
#else
    return boost::apply_visitor(get_aggregate_lit(), lookup(e));
#endif
  }

  const scalar_lit_t& operator[](Operand_ref op) { return lookup_scalar(op); }

  const scalar_lit_t& operator[](Local_Variable_ref v) {
    return lookup_scalar(v);
  }

  const scalar_lit_t& operator[](Internal_Variable_ref v) {
    return lookup_scalar(v);
  }

  const scalar_lit_t& operator[](Var_Addr_Constant_ref v) {
    return lookup_scalar(v);
  }

}; // end class LiteralFactory

} // end namespace analyzer

#endif // ANALYZER_LITERAL_HPP
