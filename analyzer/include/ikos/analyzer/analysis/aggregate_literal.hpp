/*******************************************************************************
 *
 * \file
 * \brief Data structures for aggregates (struct, array).
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017-2019 United States Government as represented by the
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

#pragma once

#include <vector>

#include <boost/variant.hpp>
#include <boost/version.hpp>

#include <ikos/core/literal.hpp>
#include <ikos/core/semantic/dumpable.hpp>

#include <ikos/analyzer/exception.hpp>
#include <ikos/analyzer/support/number.hpp>

namespace ikos {
namespace analyzer {

/// \brief Literal for aggregate types (struct, array)
template < typename VariableRef, typename MemoryLocationRef >
class AggregateLiteral {
public:
  /// \brief Scalar literal type
  using ScalarLiteral = core::Literal< VariableRef, MemoryLocationRef >;

  /// \brief Field of an aggregate
  struct Field {
    MachineInt offset;
    ScalarLiteral value;
    MachineInt size;

    bool operator==(const Field& o) const {
      return offset == o.offset && value == o.value && size == o.size;
    }
  };

  /// \brief List of fields
  using Fields = std::vector< Field >;

private:
  /// \brief Constant aggregate literal
  struct CstLit {
    Fields fields;

    bool operator==(const CstLit& o) const { return fields == o.fields; }
  };

  /// \brief Zero aggregate literal
  struct ZeroLit {
    bool operator==(const ZeroLit&) const { return true; }
  };

  /// \brief Undefined aggregate literal
  struct UndefinedLit {
    bool operator==(const UndefinedLit&) const { return true; }
  };

  /// \brief Variable aggregate literal
  struct VarLit {
    VariableRef var;

    bool operator==(const VarLit& o) const { return var == o.var; }
  };

private:
  /// \brief Union type for aggregate literals
  using Lit = boost::variant< CstLit, ZeroLit, UndefinedLit, VarLit >;

private:
  /// \brief Literal value
  Lit _lit;

  /// \brief Size (in bytes)
  MachineInt _size;

private:
  AggregateLiteral(Lit lit, MachineInt size)
      : _lit(std::move(lit)), _size(std::move(size)) {}

public:
  /// \brief No default constructor
  AggregateLiteral() = delete;

  /// \brief Copy constructor
  AggregateLiteral(const AggregateLiteral&) = default;

  /// \brief Move constructor
  AggregateLiteral(AggregateLiteral&&) = default;

  /// \brief Copy assignment operator
  AggregateLiteral& operator=(const AggregateLiteral&) = default;

  /// \brief Move assignment operator
  AggregateLiteral& operator=(AggregateLiteral&&) = default;

  /// \brief Destructor
  ~AggregateLiteral() = default;

  /// \brief Comparison of aggregate literals
  bool operator==(const AggregateLiteral& other) const {
    return this->_lit == other._lit && this->_size == other._size;
  }

  /// \brief Comparison of aggregate literals
  bool operator!=(const AggregateLiteral& other) const {
    return !this->operator==(other);
  }

  /// \brief Create a constant aggregate literal
  static AggregateLiteral cst(Fields fields, MachineInt size) {
    return AggregateLiteral(Lit(CstLit{fields}), size);
  }

  /// \brief Create a zero-initialized aggregate literal
  static AggregateLiteral zero(MachineInt size) {
    return AggregateLiteral(Lit(ZeroLit{}), size);
  }

  /// \brief Create an undefined aggregate literal
  static AggregateLiteral undefined(MachineInt size) {
    return AggregateLiteral(Lit(UndefinedLit{}), size);
  }

  /// \brief Create a variable aggregate literal
  static AggregateLiteral var(VariableRef v, MachineInt size) {
    return AggregateLiteral(Lit(VarLit{v}), size);
  }

  /// \brief Return the size of the literal, in bytes
  const MachineInt& size() const { return this->_size; }

private:
  /// \brief Visitor that checks if the given literal has the given type
  template < typename LiteralType >
  struct IsType : public boost::static_visitor< bool > {
    template < typename T >
    bool operator()(const T&) const {
      return std::is_same< T, LiteralType >::value;
    }
  };

public:
  /// \brief Return true if it's a constant aggregate literal
  bool is_cst() const {
    return boost::apply_visitor(IsType< CstLit >(), this->_lit);
  }

  /// \brief Return true if it's a zero-initialized aggregate literal
  bool is_zero() const {
    return boost::apply_visitor(IsType< ZeroLit >(), this->_lit);
  }

  /// \brief Return true if it's an undefined aggregate literal
  bool is_undefined() const {
    return boost::apply_visitor(IsType< UndefinedLit >(), this->_lit);
  }

  /// \brief Return true if it's a variable aggregate literal
  bool is_var() const {
    return boost::apply_visitor(IsType< VarLit >(), this->_lit);
  }

private:
  /// \brief Visitor that returns the fields
  struct GetFields : public boost::static_visitor< const Fields& > {
    const Fields& operator()(const CstLit& lit) const { return lit.fields; }

    const Fields& operator()(const ZeroLit&) const {
      ikos_unreachable("trying to call fields() on literal zero");
    }

    const Fields& operator()(const UndefinedLit&) const {
      ikos_unreachable("trying to call fields() on literal undefined");
    }

    const Fields& operator()(const VarLit&) const {
      ikos_unreachable("trying to call fields() on a variable");
    }
  };

public:
  /// \brief Get the set of fields
  const Fields& fields() const {
#if BOOST_VERSION == 105800
    // Workaround for https://svn.boost.org/trac10/ticket/11285
    GetFields vis;
    return this->_lit.apply_visitor(vis);
#else
    return boost::apply_visitor(GetFields(), this->_lit);
#endif
  }

private:
  /// \brief Visitor that returns the variable
  struct GetVar : public boost::static_visitor< VariableRef > {
    VariableRef operator()(const CstLit&) const {
      ikos_unreachable("trying to call var() on a constant");
    }

    VariableRef operator()(const ZeroLit&) const {
      ikos_unreachable("trying to call var() on literal zero");
    }

    VariableRef operator()(const UndefinedLit&) const {
      ikos_unreachable("trying to call var() on literal undefined");
    }

    VariableRef operator()(const VarLit& lit) const { return lit.var; }
  };

public:
  /// \brief Get the variable
  VariableRef var() const {
    return boost::apply_visitor(GetVar(), this->_lit);
  }

public:
  /// \brief Aggregate literal visitor
  ///
  /// Visitors should implement the following methods:
  ///
  /// R cst(const Fields& fields, const MachineInt& size);
  /// R zero(const MachineInt& size);
  /// R undefined(const MachineInt& size);
  /// R var(VariableRef variable, const MachineInt& size);
  template < typename R = void >
  class Visitor {
  public:
    using ResultType = R;

  }; // end class Visitor

private:
  template < typename Visitor >
  struct VariantVisitor
      : public boost::static_visitor< typename Visitor::ResultType > {
    using ResultType = typename Visitor::ResultType;

    Visitor& v;
    const MachineInt& size;

    VariantVisitor(Visitor& v_, const MachineInt& size_) : v(v_), size(size_) {}

    ResultType operator()(const CstLit& lit) { return v.cst(lit.fields, size); }

    ResultType operator()(const ZeroLit&) { return v.zero(size); }

    ResultType operator()(const UndefinedLit&) { return v.undefined(size); }

    ResultType operator()(const VarLit& lit) { return v.var(lit.var, size); }
  };

  template < typename Visitor >
  struct ConstVariantVisitor
      : public boost::static_visitor< typename Visitor::ResultType > {
    using ResultType = typename Visitor::ResultType;

    const Visitor& v;
    const MachineInt& size;

    ConstVariantVisitor(const Visitor& v_, const MachineInt& size_)
        : v(v_), size(size_) {}

    ResultType operator()(const CstLit& lit) const {
      return v.cst(lit.fields, size);
    }

    ResultType operator()(const ZeroLit&) const { return v.zero(size); }

    ResultType operator()(const UndefinedLit&) const {
      return v.undefined(size);
    }

    ResultType operator()(const VarLit& lit) const {
      return v.var(lit.var, size);
    }
  };

public:
  /// \brief Visit the aggregate literal
  template < typename Visitor,
             typename = typename std::enable_if_t< std::is_base_of<
                 AggregateLiteral::Visitor< typename Visitor::ResultType >,
                 Visitor >::value > >
  typename Visitor::ResultType apply_visitor(Visitor& v) const {
    VariantVisitor< Visitor > vis(v);
    return boost::apply_visitor(vis, this->_lit);
  }

  /// \brief Visit the aggregate literal
  template < typename Visitor,
             typename = typename std::enable_if_t< std::is_base_of<
                 AggregateLiteral::Visitor< typename Visitor::ResultType >,
                 Visitor >::value > >
  typename Visitor::ResultType apply_visitor(const Visitor& v) const {
    ConstVariantVisitor< Visitor > vis(v);
    return boost::apply_visitor(vis, this->_lit);
  }

private:
  /// \brief Visitor that dumps the literal on a stream
  struct Dumper : public boost::static_visitor<> {
    std::ostream& o;

    explicit Dumper(std::ostream& _o) : o(_o) {}

    void operator()(const CstLit& lit) const {
      o << "cst_aggregate{fields=";
      for (auto it = lit.fields.begin(), et = lit.fields.end(); it != et;) {
        o << "(offset=" << it->offset << ", value=" << it->value
          << ", size=" << it->size << ")";
        if (++it != et) {
          o << ", ";
        }
      }
    }

    void operator()(const ZeroLit&) const { o << "zero_aggregate{"; }

    void operator()(const UndefinedLit&) const { o << "undefined_aggregate{"; }

    void operator()(const VarLit& lit) const {
      o << "var_aggregate{var=";
      core::DumpableTraits< VariableRef >::dump(o, lit.var);
    }
  };

public:
  /// \brief Dump the literal on a stream, for debugging purpose
  void dump(std::ostream& o) const {
    Dumper dumper(o);
    boost::apply_visitor(dumper, this->_lit);
    o << ", size=" << this->_size << "}";
  }

}; // end class AggregateLiteral

/// \brief Write an aggregate literal on a stream
template < typename VariableRef, typename MemoryLocationRef >
std::ostream& operator<<(
    std::ostream& o,
    const AggregateLiteral< VariableRef, MemoryLocationRef >& l) {
  l.dump(o);
  return o;
}

} // end namespace analyzer
} // end namespace ikos
