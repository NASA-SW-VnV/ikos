/*******************************************************************************
 *
 * \file
 * \brief Translate AR operands to a symbolic representation
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2019 United States Government as represented by the
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

#include <unordered_map>

#include <boost/thread/shared_mutex.hpp>
#include <boost/variant.hpp>
#include <boost/version.hpp>

#include <ikos/core/literal.hpp>

#include <ikos/ar/semantic/data_layout.hpp>
#include <ikos/ar/semantic/value.hpp>

#include <ikos/analyzer/analysis/aggregate_literal.hpp>
#include <ikos/analyzer/analysis/memory_location.hpp>
#include <ikos/analyzer/analysis/variable.hpp>
#include <ikos/analyzer/exception.hpp>
#include <ikos/analyzer/support/number.hpp>

namespace ikos {
namespace analyzer {

/// \brief Scalar literal
using ScalarLit = core::Literal< Variable*, MemoryLocation* >;

/// \brief Aggregate literal
using AggregateLit = AggregateLiteral< Variable*, MemoryLocation* >;

/// \brief Any literal
class Literal {
private:
  /// \brief Union type for scalar and aggregate literals
  using Lit = boost::variant< ScalarLit, AggregateLit >;

private:
  Lit _lit;

public:
  /// Create a scalar literal
  explicit Literal(ScalarLit lit) : _lit(std::move(lit)) {}

  /// \brief Create an aggregate literal
  explicit Literal(AggregateLit lit) : _lit(std::move(lit)) {}

  /// \brief Copy constructor
  Literal(const Literal&) = default;

  /// \brief Move constructor
  Literal(Literal&&) = default;

  /// \brief Copy assignment operator
  Literal& operator=(const Literal&) = default;

  /// \brief Move assignment operator
  Literal& operator=(Literal&&) = default;

  /// \brief Destructor
  ~Literal() = default;

  /// \brief Comparison of literals
  bool operator==(const Literal& other) const {
    return this->_lit == other._lit;
  }

  /// \brief Comparison of literals
  bool operator!=(const Literal& other) const {
    return !this->operator==(other);
  }

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
  /// \brief Return true if it's a scalar literal
  bool is_scalar() const {
    return boost::apply_visitor(IsType< ScalarLit >(), this->_lit);
  }

  /// \brief Return true if it's an aggregate literal
  bool is_aggregate() const {
    return boost::apply_visitor(IsType< AggregateLit >(), this->_lit);
  }

private:
  /// \brief Visitor that returns the scalar literal
  struct GetScalarLit : public boost::static_visitor< const ScalarLit& > {
    const ScalarLit& operator()(const ScalarLit& lit) const { return lit; }

    const ScalarLit& operator()(const AggregateLit&) const {
      ikos_unreachable("trying to call scalar() on an aggregate literal");
    }
  };

public:
  /// \brief Return the scalar literal
  const ScalarLit& scalar() const {
#if BOOST_VERSION == 105800
    // Workaround for https://svn.boost.org/trac10/ticket/11285
    GetScalarLit vis;
    return this->_lit.apply_visitor(vis);
#else
    return boost::apply_visitor(GetScalarLit(), this->_lit);
#endif
  }

private:
  /// \brief Visitor that returns the aggregate literal
  struct GetAggregateLit : public boost::static_visitor< const AggregateLit& > {
    const AggregateLit& operator()(const ScalarLit&) const {
      ikos_unreachable("trying to call aggregate() on a scalar literal");
    }

    const AggregateLit& operator()(const AggregateLit& lit) const {
      return lit;
    }
  };

public:
  /// \brief Return the aggregate literal
  const AggregateLit& aggregate() const {
#if BOOST_VERSION == 105800
    // Workaround for https://svn.boost.org/trac10/ticket/11285
    GetAggregateLit vis;
    return this->_lit.apply_visitor(vis);
#else
    return boost::apply_visitor(GetAggregateLit(), this->_lit);
#endif
  }

private:
  /// \brief Visitor that returns true if the literal is a variable
  struct IsVariableLit : public boost::static_visitor< bool > {
    bool operator()(const ScalarLit& lit) const { return lit.is_var(); }

    bool operator()(const AggregateLit& lit) const { return lit.is_var(); }
  };

public:
  // \brief Return true if the literal is a variable
  bool is_var() const {
    return boost::apply_visitor(IsVariableLit(), this->_lit);
  }

private:
  /// \brief Visitor that returns the variable
  struct GetVariable : public boost::static_visitor< Variable* > {
    Variable* operator()(const ScalarLit& lit) const { return lit.var(); }

    Variable* operator()(const AggregateLit& lit) const { return lit.var(); }
  };

public:
  /// \brief Get the variable
  Variable* var() const {
    return boost::apply_visitor(GetVariable(), this->_lit);
  }

public:
  /// \brief Literal visitor
  ///
  /// Visitors should implement the following methods:
  ///
  /// R operator()(const ScalarLit&);
  /// R operator()(const AggregateLit&);
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

    explicit VariantVisitor(Visitor& v_) : v(v_) {}

    ResultType operator()(const ScalarLit& lit) { return v(lit); }

    ResultType operator()(const AggregateLit& lit) { return v(lit); }
  };

  template < typename Visitor >
  struct ConstVariantVisitor
      : public boost::static_visitor< typename Visitor::ResultType > {
    using ResultType = typename Visitor::ResultType;

    const Visitor& v;

    explicit ConstVariantVisitor(const Visitor& v_) : v(v_) {}

    ResultType operator()(const ScalarLit& lit) const { return v(lit); }

    ResultType operator()(const AggregateLit& lit) const { return v(lit); }
  };

public:
  /// \brief Visit the literal
  template < typename Visitor,
             typename = typename std::enable_if_t< std::is_base_of<
                 Literal::Visitor< typename Visitor::ResultType >,
                 Visitor >::value > >
  typename Visitor::ResultType apply_visitor(Visitor& v) const {
    VariantVisitor< Visitor > vis(v);
    return boost::apply_visitor(vis, this->_lit);
  }

  /// \brief Visit the literal
  template < typename Visitor,
             typename = typename std::enable_if_t< std::is_base_of<
                 Literal::Visitor< typename Visitor::ResultType >,
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

    void operator()(const ScalarLit& lit) const { lit.dump(o); }

    void operator()(const AggregateLit& lit) const { lit.dump(o); }
  };

public:
  /// \brief Dump the literal on a stream, for debugging purpose
  void dump(std::ostream& o) const {
    boost::apply_visitor(Dumper(o), this->_lit);
  }

}; // end class Literal

/// \brief Write a literal on a stream
inline std::ostream& operator<<(std::ostream& o, const Literal& l) {
  l.dump(o);
  return o;
}

/// \brief Exception for unexpected literals
///
/// This is raised when calling lfac[value] and an AggregateLit is found
class UnexpectedLiteralError : public LogicError {
public:
  /// \brief Constructor
  explicit UnexpectedLiteralError(const std::string& msg) : LogicError(msg) {}

}; // end class UnexpectedLiteralError

class ScalarLiteralError : public UnexpectedLiteralError {
private:
  const ScalarLit& _lit;

public:
  explicit ScalarLiteralError(const ScalarLit& lit)
      : UnexpectedLiteralError("literal factory: unexpected scalar literal"),
        _lit(lit) {}

  const ScalarLit& lit() const { return this->_lit; }

}; // end class ScalarLiteralError

class AggregateLiteralError : public UnexpectedLiteralError {
private:
  const AggregateLit& _lit;

public:
  explicit AggregateLiteralError(const AggregateLit& lit)
      : UnexpectedLiteralError("literal factory: unexpected aggregate literal"),
        _lit(lit) {}

  const AggregateLit& lit() const { return _lit; }

}; // end class AggregateLiteralError

/// \brief Create literals from AR values
class LiteralFactory {
private:
  /// \brief Map from ar::Value* to Literal
  ///
  /// Must be a data structure that does not invalidate references on
  /// insertions.
  using Map = std::unordered_map< ar::Value*, Literal >;

private:
  /// \brief Mutex
  boost::shared_mutex _mutex;

  /// \brief Variable factory
  VariableFactory& _vfac;

  /// \brief Data layout
  const ar::DataLayout& _data_layout;

  /// \brief Map from ar::Value* to Literal
  Map _map;

public:
  /// \brief Constructor
  LiteralFactory(VariableFactory& vfac, const ar::DataLayout& data_layout);

  /// \brief No copy constructor
  LiteralFactory(const LiteralFactory&) = delete;

  /// \brief No move constructor
  LiteralFactory(LiteralFactory&&) = delete;

  /// \brief No copy assignment operator
  LiteralFactory& operator=(const LiteralFactory&) = delete;

  /// \brief No move assignment operator
  LiteralFactory& operator=(LiteralFactory&&) = delete;

  /// \brief Destructor
  ~LiteralFactory();

  /// \brief Translate an ar::Value* into a scalar literal
  ///
  /// \throw AggregateLiteralError or VoidVarLiteralError
  const ScalarLit& get_scalar(ar::Value* value);

  /// \brief Translate an ar::Value* into an aggregate literal
  ///
  /// \throw ScalarLiteralError or VoidVarLiteralError
  const AggregateLit& get_aggregate(ar::Value* value);

  /// \brief Translate an ar::Value* into a literal
  ///
  /// This also adds the translation into the cache
  const Literal& get(ar::Value* value);

private:
  /// \brief Translate an ar::Value* into a Literal
  Literal create_literal(ar::Value* value);

}; // end class LiteralFactory

} // end namespace analyzer
} // end namespace ikos
