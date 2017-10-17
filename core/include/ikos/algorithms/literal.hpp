/**************************************************************************/ /**
 *
 * \file
 * \brief Data structures for literals.
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

#ifndef IKOS_LITERAL_HPP
#define IKOS_LITERAL_HPP

#include <type_traits>

#include <boost/variant.hpp>

#include <ikos/common/types.hpp>
#include <ikos/number/support.hpp>

namespace ikos {

/// \brief Class to represent a literal
///
/// A literal is either a constant (integer, floating point, null, undefined) or
/// a variable (integer, floating point, pointer).
template < typename Integer, typename Float, typename VariableName >
class literal : public writeable {
public:
  typedef Integer integer_t;
  typedef Float floating_point_t;
  typedef VariableName variable_name_t;
  typedef literal< Integer, Float, VariableName > literal_t;

private:
  struct integer_lit {
    Integer value;

    bool operator==(const integer_lit& o) const { return value == o.value; }
  };

  struct floating_point_lit {
    Float value;

    bool operator==(const floating_point_lit& o) const {
      return value == o.value;
    }
  };

  struct undefined_lit {
    bool operator==(const undefined_lit&) const { return true; }
  };

  struct null_lit {
    bool operator==(const null_lit&) const { return true; }
  };

  struct integer_var_lit {
    VariableName var;

    bool operator==(const integer_var_lit& o) const { return var == o.var; }
  };

  struct floating_point_var_lit {
    VariableName var;

    bool operator==(const floating_point_var_lit& o) const {
      return var == o.var;
    }
  };

  struct pointer_var_lit {
    VariableName var;

    bool operator==(const pointer_var_lit& o) const { return var == o.var; }
  };

  typedef boost::variant< integer_lit,
                          floating_point_lit,
                          undefined_lit,
                          null_lit,
                          integer_var_lit,
                          floating_point_var_lit,
                          pointer_var_lit > lit_t;

private:
  lit_t _lit;

private:
  explicit literal(const lit_t& lit) : _lit(lit) {}

public:
  literal() = delete;

  literal(const literal_t&) = default;

  literal_t& operator=(const literal_t&) = default;

  bool operator==(const literal_t& o) const { return _lit == o._lit; }

  bool operator!=(const literal_t& o) const { return _lit != o._lit; }

public:
  static literal_t integer(Integer v) { return literal(lit_t(integer_lit{v})); }

  template < typename N,
             class = std::enable_if_t< is_supported_integral< N >::value > >
  static literal_t integer(N n) {
    return literal(lit_t(integer_lit{Integer(n)}));
  }

  static literal_t floating_point(Float v) {
    return literal(lit_t(floating_point_lit{v}));
  }

  static literal_t undefined() { return literal(lit_t(undefined_lit{})); }

  static literal_t null() { return literal(lit_t(null_lit{})); }

  static literal_t integer_var(VariableName v) {
    return literal(lit_t(integer_var_lit{v}));
  }

  static literal_t floating_point_var(VariableName v) {
    return literal(lit_t(floating_point_var_lit{v}));
  }

  static literal_t pointer_var(VariableName v) {
    return literal(lit_t(pointer_var_lit{v}));
  }

private:
  template < typename LiteralType >
  struct is_type : public boost::static_visitor< bool > {
    template < typename T >
    bool operator()(const T&) const {
      return std::is_same< T, LiteralType >::value;
    }
  };

public:
  bool is_integer() const {
    return boost::apply_visitor(is_type< integer_lit >(), _lit);
  }

  bool is_floating_point() const {
    return boost::apply_visitor(is_type< floating_point_lit >(), _lit);
  }

  bool is_undefined() const {
    return boost::apply_visitor(is_type< undefined_lit >(), _lit);
  }

  bool is_null() const {
    return boost::apply_visitor(is_type< null_lit >(), _lit);
  }

  bool is_integer_var() const {
    return boost::apply_visitor(is_type< integer_var_lit >(), _lit);
  }

  bool is_floating_point_var() const {
    return boost::apply_visitor(is_type< floating_point_var_lit >(), _lit);
  }

  bool is_pointer_var() const {
    return boost::apply_visitor(is_type< pointer_var_lit >(), _lit);
  }

  // for convenience
  bool is_cst() const {
    return is_integer() || is_floating_point() || is_null() || is_undefined();
  }

  // for convenience
  bool is_var() const {
    return is_integer_var() || is_floating_point_var() || is_pointer_var();
  }

private:
  struct get_integer : public boost::static_visitor< Integer > {
    Integer operator()(const integer_lit& lit) const { return lit.value; }

    Integer operator()(const floating_point_lit&) const {
      throw logic_error(
          "literal: trying to call integer() on a floating point");
    }

    Integer operator()(const undefined_lit&) const {
      throw logic_error(
          "literal: trying to call integer() on literal undefined");
    }

    Integer operator()(const null_lit&) const {
      throw logic_error("literal: trying to call integer() on literal null");
    }

    Integer operator()(const integer_var_lit&) const {
      throw logic_error(
          "literal: trying to call integer() on an integer variable");
    }

    Integer operator()(const floating_point_var_lit&) const {
      throw logic_error(
          "literal: trying to call integer() on a floating point variable");
    }

    Integer operator()(const pointer_var_lit&) const {
      throw logic_error(
          "literal: trying to call integer() on a pointer variable");
    }

  }; // end struct get_integer

public:
  Integer integer() const { return boost::apply_visitor(get_integer(), _lit); }

private:
  struct get_floating_point : public boost::static_visitor< Float > {
    Float operator()(const integer_lit&) const {
      throw logic_error(
          "literal: trying to call floating_point() on an integer");
    }

    Float operator()(const floating_point_lit& lit) const { return lit.value; }

    Float operator()(const undefined_lit&) const {
      throw logic_error(
          "literal: trying to call floating_point() on literal undefined");
    }

    Float operator()(const null_lit&) const {
      throw logic_error(
          "literal: trying to call floating_point() on literal null");
    }

    Float operator()(const integer_var_lit&) const {
      throw logic_error(
          "literal: trying to call floating_point() on an integer variable");
    }

    Float operator()(const floating_point_var_lit&) const {
      throw logic_error(
          "literal: trying to call floating_point() on a floating point "
          "variable");
    }

    Float operator()(const pointer_var_lit&) const {
      throw logic_error(
          "literal: trying to call floating_point() on a pointer variable");
    }

  }; // end struct get_floating_point

public:
  Float floating_point() const {
    return boost::apply_visitor(get_floating_point(), _lit);
  }

private:
  struct get_var : public boost::static_visitor< VariableName > {
    VariableName operator()(const integer_lit&) const {
      throw logic_error("literal: trying to call var() on an integer");
    }

    VariableName operator()(const floating_point_lit&) const {
      throw logic_error("literal: trying to call var() on a floating point");
    }

    VariableName operator()(const undefined_lit&) const {
      throw logic_error("literal: trying to call var() on literal undefined");
    }

    VariableName operator()(const null_lit&) const {
      throw logic_error("literal: trying to call var() on literal null");
    }

    VariableName operator()(const integer_var_lit& lit) const {
      return lit.var;
    }

    VariableName operator()(const floating_point_var_lit& lit) const {
      return lit.var;
    }

    VariableName operator()(const pointer_var_lit& lit) const {
      return lit.var;
    }

  }; // end struct get_var

public:
  VariableName var() const { return boost::apply_visitor(get_var(), _lit); }

public:
  /// \brief Visitor for literals
  template < typename R = void >
  class visitor {
  public:
    typedef R result_type;

    // visitors should implement the following methods:
    //
    // R integer(Integer);
    // R floating_point(Float);
    // R undefined();
    // R null();
    // R integer_var(VariableName);
    // R floating_point_var(VariableName);
    // R pointer_var(VariableName);

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

    variant_visitor(Visitor& _v) : v(_v) {}

    result_type operator()(const integer_lit& lit) {
      return v.integer(lit.value);
    }

    result_type operator()(const floating_point_lit& lit) {
      return v.floating_point(lit.value);
    }

    result_type operator()(const undefined_lit&) { return v.undefined(); }

    result_type operator()(const null_lit&) { return v.null(); }

    result_type operator()(const integer_var_lit& lit) {
      return v.integer_var(lit.var);
    }

    result_type operator()(const floating_point_var_lit& lit) {
      return v.floating_point_var(lit.var);
    }

    result_type operator()(const pointer_var_lit& lit) {
      return v.pointer_var(lit.var);
    }

  }; // end struct variant_visitor

  template < typename Visitor >
  struct const_variant_visitor
      : public boost::static_visitor< typename Visitor::result_type > {
    typedef typename Visitor::result_type result_type;

    const Visitor& v;

    const_variant_visitor(const Visitor& _v) : v(_v) {}

    result_type operator()(const integer_lit& lit) const {
      return v.integer(lit.value);
    }

    result_type operator()(const floating_point_lit& lit) const {
      return v.floating_point(lit.value);
    }

    result_type operator()(const undefined_lit&) const { return v.undefined(); }

    result_type operator()(const null_lit&) const { return v.null(); }

    result_type operator()(const integer_var_lit& lit) const {
      return v.integer_var(lit.var);
    }

    result_type operator()(const floating_point_var_lit& lit) const {
      return v.floating_point_var(lit.var);
    }

    result_type operator()(const pointer_var_lit& lit) const {
      return v.pointer_var(lit.var);
    }

  }; // end struct const_variant_visitor

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

    void operator()(integer_lit lit) { o << "integer{" << lit.value << "}"; }

    void operator()(floating_point_lit lit) {
      o << "floating_point{" << lit.value << "}";
    }

    void operator()(undefined_lit) { o << "undefined"; }

    void operator()(null_lit) { o << "null"; }

    void operator()(integer_var_lit lit) {
      o << "integer_var{";
      index_traits< VariableName >::write(o, lit.var);
      o << "}";
    }

    void operator()(floating_point_var_lit lit) {
      o << "floating_point_var{";
      index_traits< VariableName >::write(o, lit.var);
      o << "}";
    }

    void operator()(pointer_var_lit lit) {
      o << "pointer_var{";
      index_traits< VariableName >::write(o, lit.var);
      o << "}";
    }

  }; // end struct writer

public:
  void write(std::ostream& o) {
    writer w(o);
    boost::apply_visitor(w, _lit);
  }

}; // end class literal

template < typename Integer, typename Float, typename VariableName >
std::ostream& operator<<(std::ostream& o,
                         literal< Integer, Float, VariableName > l) {
  l.write(o);
  return o;
}

} // end namespace ikos

#endif // IKOS_LITERAL_HPP
