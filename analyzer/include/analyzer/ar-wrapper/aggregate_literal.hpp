/*******************************************************************************
 *
 * Data structures for aggregates (struct, array).
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

#ifndef ANALYZER_AGGREGATE_LITERAL_HPP
#define ANALYZER_AGGREGATE_LITERAL_HPP

#include <vector>

#include <ikos/algorithms/literal.hpp>

#include <analyzer/analysis/common.hpp>

namespace analyzer {

template < typename Integer, typename Float, typename VariableName >
class aggregate_literal {
public:
  typedef Integer integer_t;
  typedef Float floating_point_t;
  typedef VariableName variable_name_t;
  typedef ikos::literal< Integer, Float, VariableName > scalar_literal_t;
  typedef aggregate_literal< Integer, Float, VariableName > aggregate_literal_t;

public:
  struct field_t {
    Integer offset;
    scalar_literal_t value;
    Integer size;
  };

  typedef std::vector< field_t > fields_t;

private:
  struct cst_lit {
    fields_t fields;

    bool operator==(const cst_lit& o) const { return fields == o.fields; }
  };

  struct undefined_lit {
    bool operator==(const undefined_lit&) const { return true; }
  };

  struct var_lit {
    VariableName var;

    bool operator==(const var_lit& o) const { return var == o.var; }
  };

  typedef boost::variant< cst_lit, undefined_lit, var_lit > lit_t;

private:
  lit_t _lit;
  Integer _size;

private:
  aggregate_literal(const lit_t& lit, Integer size) : _lit(lit), _size(size) {}

public:
  aggregate_literal() = delete;

  aggregate_literal(const aggregate_literal&) = default;

  aggregate_literal_t& operator=(const aggregate_literal_t&) = default;

  bool operator==(const aggregate_literal_t& o) const {
    return _lit == o._lit && _size == o._size;
  }

  bool operator!=(const aggregate_literal_t& o) const {
    return _lit != o._lit || _size != o._size;
  }

public:
  static aggregate_literal_t cst(const fields_t& fields, Integer size) {
    return aggregate_literal(lit_t(cst_lit{fields}), size);
  }

  static aggregate_literal_t undefined(Integer size) {
    return aggregate_literal(lit_t(undefined_lit{}), size);
  }

  static aggregate_literal_t var(VariableName v, Integer size) {
    return aggregate_literal(lit_t(var_lit{v}), size);
  }

public:
  Integer size() const { return _size; }

private:
  template < typename LiteralType >
  struct is_type : public boost::static_visitor< bool > {
    template < typename T >
    bool operator()(const T&) const {
      return std::is_same< T, LiteralType >::value;
    }
  };

public:
  bool is_cst() const {
    return boost::apply_visitor(is_type< cst_lit >(), _lit);
  }

  bool is_undefined() const {
    return boost::apply_visitor(is_type< undefined_lit >(), _lit);
  }

  bool is_var() const {
    return boost::apply_visitor(is_type< var_lit >(), _lit);
  }

private:
  struct get_fields : public boost::static_visitor< const fields_t& > {
    const fields_t& operator()(const cst_lit& lit) const { return lit.fields; }

    const fields_t& operator()(const undefined_lit&) const {
      throw analyzer_error(
          "aggregate_literal: trying to call fields() on literal undefined");
    }

    const fields_t& operator()(const var_lit&) const {
      throw analyzer_error(
          "aggregate_literal: trying to call fields() on a variable");
    }

  }; // end struct get_fields

public:
  const fields_t& fields() const {
#if BOOST_VERSION == 105800
    // workaround for https://svn.boost.org/trac10/ticket/11285
    get_fields vis;
    return _lit.apply_visitor(vis);
#else
    return boost::apply_visitor(get_fields(), _lit);
#endif
  }

private:
  struct get_var : public boost::static_visitor< VariableName > {
    VariableName operator()(const cst_lit&) const {
      throw analyzer_error(
          "aggregate_literal: trying to call var() on a constant");
    }

    VariableName operator()(const undefined_lit&) const {
      throw analyzer_error(
          "aggregate_literal: trying to call var() on literal undefined");
    }

    VariableName operator()(const var_lit& lit) const { return lit.var; }

  }; // end struct get_var

public:
  VariableName var() const { return boost::apply_visitor(get_var(), _lit); }

public:
  template < typename R = void >
  class visitor {
  public:
    typedef R result_type;

    /*
     * visitors should implement the following methods
     *
     * R cst(const fields_t& fields, Integer size);
     * R undefined(Integer size);
     * R var(VariableName name, Integer size);
     */

  protected: // for use as base class only
    visitor() = default;
    ~visitor() = default;

  }; // end class visitor

private:
  template < typename Visitor >
  struct variant_visitor
      : public boost::static_visitor< typename Visitor::result_type > {
    typedef typename Visitor::result_type result_type;

    Visitor& v;
    const Integer size;

    variant_visitor(Visitor& _v, Integer _size) : v(_v), size(_size) {}

    result_type operator()(const cst_lit& lit) {
      return v.cst(lit.fields, size);
    }

    result_type operator()(const undefined_lit&) { return v.undefined(size); }

    result_type operator()(const var_lit& lit) { return v.var(lit.var, size); }

  }; // end struct variant_visitor

  template < typename Visitor >
  struct const_variant_visitor
      : public boost::static_visitor< typename Visitor::result_type > {
    typedef typename Visitor::result_type result_type;

    const Visitor& v;
    const Integer size;

    const_variant_visitor(const Visitor& _v, Integer _size)
        : v(_v), size(_size) {}

    result_type operator()(const cst_lit& lit) const {
      return v.cst(lit.fields, size);
    }

    result_type operator()(const undefined_lit&) const {
      return v.undefined(size);
    }

    result_type operator()(const var_lit& lit) const {
      return v.var(lit.var, size);
    }

  }; // end struct const variant_visitor

public:
  template < typename Visitor,
             typename = typename std::enable_if<
                 std::is_base_of< visitor< typename Visitor::result_type >,
                                  Visitor >::value >::type >
  typename Visitor::result_type apply_visitor(Visitor& v) const {
    variant_visitor< Visitor > vis(v);
    return boost::apply_visitor(vis, _lit);
  }

  template < typename Visitor,
             typename = typename std::enable_if<
                 std::is_base_of< visitor< typename Visitor::result_type >,
                                  Visitor >::value >::type >
  typename Visitor::result_type apply_visitor(const Visitor& v) const {
    const_variant_visitor< Visitor > vis(v);
    return boost::apply_visitor(vis, _lit);
  }

private:
  struct writer : public boost::static_visitor<> {
    std::ostream& o;

    writer(std::ostream& _o) : o(_o) {}

    void operator()(const cst_lit& lit) const {
      o << "cst_aggregate{fields=";
      for (auto it = lit.fields.begin(); it != lit.fields.end();) {
        o << "(offset=" << it->offset << ", value=" << it->value
          << ", size=" << it->size << ")";
        if (++it != lit.fields.end()) {
          o << ", ";
        }
      }
    }

    void operator()(const undefined_lit&) const { o << "undefined_aggregate{"; }

    void operator()(const var_lit& lit) const {
      o << "var_aggregate{var=" << lit.var;
    }

  }; // end struct writer

public:
  void write(std::ostream& o) const {
    writer w(o);
    boost::apply_visitor(w, _lit);
    o << ", size=" << _size << "}";
  }

}; // end class aggregate_literal

template < typename Integer, typename Float, typename VariableName >
std::ostream& operator<<(
    std::ostream& o,
    const aggregate_literal< Integer, Float, VariableName >& l) {
  l.write(o);
  return o;
}

} // end namespace analyzer

#endif // ANALYZER_AGGREGATE_LITERAL_HPP
