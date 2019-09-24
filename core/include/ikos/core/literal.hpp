/*******************************************************************************
 *
 * \file
 * \brief Data structures for literals
 *
 * Author: Maxime Arthaud
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

#include <type_traits>

#include <boost/variant.hpp>
#include <boost/version.hpp>

#include <ikos/core/number/dummy_number.hpp>
#include <ikos/core/number/machine_int.hpp>
#include <ikos/core/semantic/dumpable.hpp>
#include <ikos/core/semantic/memory_location.hpp>
#include <ikos/core/semantic/variable.hpp>

namespace ikos {
namespace core {

/// \brief Class to represent a literal
///
/// A literal is either a:
///   * Constant machine integer
///   * Constant floating point
///   * Constant memory location
///   * Null
///   * Undefined
///   * Machine integer variable
///   * Floating point variable
///   * Pointer variable
template < typename VariableRef, typename MemoryLocationRef >
class Literal {
public:
  static_assert(
      core::IsVariable< VariableRef >::value,
      "VariableRef does not meet the requirements for variable types");
  static_assert(core::IsMemoryLocation< MemoryLocationRef >::value,
                "MemoryLocationRef does not meet the requirements for memory "
                "location types");

private:
  struct MachineIntLit {
    MachineInt value;

    bool operator==(const MachineIntLit& o) const { return value == o.value; }
  };

  struct FloatingPointLit {
    // TODO(marthaud): Add a class to represent floating points
    DummyNumber value;

    bool operator==(const FloatingPointLit&) const { return true; }
  };

  struct MemoryLocationLit {
    MemoryLocationRef value;

    bool operator==(const MemoryLocationLit& o) const {
      return value == o.value;
    }
  };

  struct NullLit {
    bool operator==(const NullLit&) const { return true; }
  };

  struct UndefinedLit {
    bool operator==(const UndefinedLit&) const { return true; }
  };

  struct MachineIntVarLit {
    VariableRef var;

    bool operator==(const MachineIntVarLit& o) const { return var == o.var; }
  };

  struct FloatingPointVarLit {
    VariableRef var;

    bool operator==(const FloatingPointVarLit& o) const { return var == o.var; }
  };

  struct PointerVarLit {
    VariableRef var;

    bool operator==(const PointerVarLit& o) const { return var == o.var; }
  };

private:
  using Lit = boost::variant< MachineIntLit,
                              FloatingPointLit,
                              MemoryLocationLit,
                              NullLit,
                              UndefinedLit,
                              MachineIntVarLit,
                              FloatingPointVarLit,
                              PointerVarLit >;

private:
  Lit _lit;

private:
  /// \brief Private constructor
  explicit Literal(Lit lit) : _lit(std::move(lit)) {}

public:
  /// \brief No default constructor
  Literal() = delete;

  /// \brief Copy constructor
  Literal(const Literal&) = default;

  /// \brief Move constructor
  Literal(Literal&&) noexcept = default;

  /// \brief Copy assignment operator
  Literal& operator=(const Literal&) = default;

  /// \brief Move assignment operator
  Literal& operator=(Literal&&) noexcept = default;

  /// \brief Destructor
  ~Literal() = default;

  bool operator==(const Literal& other) const {
    return this->_lit == other._lit;
  }

  bool operator!=(const Literal& other) const {
    return !this->operator==(other);
  }

  /// \brief Create a constant machine integer literal
  static Literal machine_int(MachineInt v) {
    return Literal(Lit(MachineIntLit{std::move(v)}));
  }

  /// \brief Create a constant floating point literal
  static Literal floating_point(DummyNumber) {
    return Literal(Lit(FloatingPointLit{DummyNumber{}}));
  }

  /// \brief Create a constant memory location literal
  static Literal memory_location(MemoryLocationRef v) {
    return Literal(Lit(MemoryLocationLit{v}));
  }

  /// \brief Create the null literal
  static Literal null() { return Literal(Lit(NullLit{})); }

  /// \brief Create the undefined literal
  static Literal undefined() { return Literal(Lit(UndefinedLit{})); }

  /// \brief Create a machine integer variable literal
  static Literal machine_int_var(VariableRef v) {
    return Literal(Lit(MachineIntVarLit{v}));
  }

  /// \brief Create a floating point variable literal
  static Literal floating_point_var(VariableRef v) {
    return Literal(Lit(FloatingPointVarLit{v}));
  }

  /// \brief Create a pointer variable literal
  static Literal pointer_var(VariableRef v) {
    return Literal(Lit(PointerVarLit{v}));
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
  /// \brief Return true if the literal is a constant machine integer
  bool is_machine_int() const {
    return boost::apply_visitor(IsType< MachineIntLit >(), this->_lit);
  }

  /// \brief Return true if the literal is a constant floating point
  bool is_floating_point() const {
    return boost::apply_visitor(IsType< FloatingPointLit >(), this->_lit);
  }

  /// \brief Return true if the literal is a constant memory location
  bool is_memory_location() const {
    return boost::apply_visitor(IsType< MemoryLocationLit >(), this->_lit);
  }

  /// \brief Return true if the literal is the null constant
  bool is_null() const {
    return boost::apply_visitor(IsType< NullLit >(), this->_lit);
  }

  /// \brief Return true if the literal is the undefined constant
  bool is_undefined() const {
    return boost::apply_visitor(IsType< UndefinedLit >(), this->_lit);
  }

  /// \brief Return true if the literal is a machine integer variable
  bool is_machine_int_var() const {
    return boost::apply_visitor(IsType< MachineIntVarLit >(), this->_lit);
  }

  /// \brief Return true if the literal is a floating point variable
  bool is_floating_point_var() const {
    return boost::apply_visitor(IsType< FloatingPointVarLit >(), this->_lit);
  }

  /// \brief Return true if the literal is a pointer variable
  bool is_pointer_var() const {
    return boost::apply_visitor(IsType< PointerVarLit >(), this->_lit);
  }

private:
  /// \brief Visitor that returns true if the literal is a constant
  struct IsConstant : public boost::static_visitor< bool > {
    bool operator()(const MachineIntLit&) const { return true; }

    bool operator()(const FloatingPointLit&) const { return true; }

    bool operator()(const MemoryLocationLit&) const { return true; }

    bool operator()(const NullLit&) const { return true; }

    bool operator()(const UndefinedLit&) const { return true; }

    bool operator()(const MachineIntVarLit&) const { return false; }

    bool operator()(const FloatingPointVarLit&) const { return false; }

    bool operator()(const PointerVarLit&) const { return false; }
  };

public:
  /// \brief Return true if the literal is a constant
  bool is_cst() const { return boost::apply_visitor(IsConstant(), this->_lit); }

private:
  /// \brief Visitor that returns true if the literal is a variable
  struct IsVariable : public boost::static_visitor< bool > {
    bool operator()(const MachineIntLit&) const { return false; }

    bool operator()(const FloatingPointLit&) const { return false; }

    bool operator()(const MemoryLocationLit&) const { return false; }

    bool operator()(const NullLit&) const { return false; }

    bool operator()(const UndefinedLit&) const { return false; }

    bool operator()(const MachineIntVarLit&) const { return true; }

    bool operator()(const FloatingPointVarLit&) const { return true; }

    bool operator()(const PointerVarLit&) const { return true; }
  };

public:
  // \brief Return true if the literal is a variable
  bool is_var() const { return boost::apply_visitor(IsVariable(), this->_lit); }

private:
  /// \brief Visitor that returns the machine integer
  struct GetMachineInt : public boost::static_visitor< const MachineInt& > {
    const MachineInt& operator()(const MachineIntLit& lit) const {
      return lit.value;
    }

    const MachineInt& operator()(const FloatingPointLit&) const {
      ikos_unreachable("trying to call machine_int() on a floating point");
    }

    const MachineInt& operator()(const MemoryLocationLit&) const {
      ikos_unreachable("trying to call machine_int() on a memory location");
    }

    const MachineInt& operator()(const NullLit&) const {
      ikos_unreachable("trying to call machine_int() on null");
    }

    const MachineInt& operator()(const UndefinedLit&) const {
      ikos_unreachable("trying to call machine_int() on undefined");
    }

    const MachineInt& operator()(const MachineIntVarLit&) const {
      ikos_unreachable(
          "trying to call machine_int() on a machine integer variable");
    }

    const MachineInt& operator()(const FloatingPointVarLit&) const {
      ikos_unreachable(
          "trying to call machine_int() on a floating point variable");
    }

    const MachineInt& operator()(const PointerVarLit&) const {
      ikos_unreachable("trying to call machine_int() on a pointer variable");
    }
  };

public:
  /// \brief Get the machine integer
  const MachineInt& machine_int() const {
#if BOOST_VERSION == 105800
    // workaround for https://svn.boost.org/trac10/ticket/11285
    GetMachineInt vis;
    return this->_lit.apply_visitor(vis);
#else
    return boost::apply_visitor(GetMachineInt(), this->_lit);
#endif
  }

private:
  /// \brief Visitor that returns the floating point
  struct GetFloatingPoint : public boost::static_visitor< const DummyNumber& > {
    const DummyNumber& operator()(const MachineIntLit&) const {
      ikos_unreachable("trying to call floating_point() on a machine integer");
    }

    const DummyNumber& operator()(const FloatingPointLit& lit) const {
      return lit.value;
    }

    const DummyNumber& operator()(const MemoryLocationLit&) const {
      ikos_unreachable("trying to call floating_point() on a memory location");
    }

    const DummyNumber& operator()(const NullLit&) const {
      ikos_unreachable("trying to call floating_point() on null");
    }

    const DummyNumber& operator()(const UndefinedLit&) const {
      ikos_unreachable("trying to call floating_point() on undefined");
    }

    const DummyNumber& operator()(const MachineIntVarLit&) const {
      ikos_unreachable(
          "trying to call floating_point() on a machine integer variable");
    }

    const DummyNumber& operator()(const FloatingPointVarLit&) const {
      ikos_unreachable(
          "trying to call floating_point() on a floating point variable");
    }

    const DummyNumber& operator()(const PointerVarLit&) const {
      ikos_unreachable("trying to call floating_point() on a pointer variable");
    }
  };

public:
  /// \brief Get the floating point
  const DummyNumber& floating_point() const {
    return boost::apply_visitor(GetFloatingPoint(), this->_lit);
  }

private:
  /// \brief Visitor that returns the memory location
  struct GetMemoryLocation : public boost::static_visitor< MemoryLocationRef > {
    MemoryLocationRef operator()(const MachineIntLit&) const {
      ikos_unreachable("trying to call memory_location() on a machine integer");
    }

    MemoryLocationRef operator()(const FloatingPointLit&) const {
      ikos_unreachable("trying to call memory_location() on a floating point");
    }

    MemoryLocationRef operator()(const MemoryLocationLit& lit) const {
      return lit.value;
    }

    MemoryLocationRef operator()(const NullLit&) const {
      ikos_unreachable("trying to call memory_location() on null");
    }

    MemoryLocationRef operator()(const UndefinedLit&) const {
      ikos_unreachable("trying to call memory_location() on undefined");
    }

    MemoryLocationRef operator()(const MachineIntVarLit&) const {
      ikos_unreachable(
          "trying to call memory_location() on a machine integer variable");
    }

    MemoryLocationRef operator()(const FloatingPointVarLit&) const {
      ikos_unreachable(
          "trying to call memory_location() on a floating point variable");
    }

    MemoryLocationRef operator()(const PointerVarLit&) const {
      ikos_unreachable(
          "trying to call memory_location() on a pointer variable");
    }
  };

public:
  /// \brief Get the memory location
  MemoryLocationRef memory_location() const {
    return boost::apply_visitor(GetMemoryLocation(), this->_lit);
  }

private:
  /// \brief Visitor that returns the variable
  struct GetVariable : public boost::static_visitor< VariableRef > {
    VariableRef operator()(const MachineIntLit&) const {
      ikos_unreachable("trying to call var() on a machine integer");
    }

    VariableRef operator()(const FloatingPointLit&) const {
      ikos_unreachable("trying to call var() on a floating point");
    }

    VariableRef operator()(const MemoryLocationLit&) const {
      ikos_unreachable("trying to call var() on a memory location");
    }

    VariableRef operator()(const NullLit&) const {
      ikos_unreachable("trying to call var() on null");
    }

    VariableRef operator()(const UndefinedLit&) const {
      ikos_unreachable("trying to call var() on undefined");
    }

    VariableRef operator()(const MachineIntVarLit& lit) const {
      return lit.var;
    }

    VariableRef operator()(const FloatingPointVarLit& lit) const {
      return lit.var;
    }

    VariableRef operator()(const PointerVarLit& lit) const { return lit.var; }
  };

public:
  /// \brief Get the variable
  VariableRef var() const {
    return boost::apply_visitor(GetVariable(), this->_lit);
  }

public:
  /// \brief Base class for visitors of literals
  ///
  /// Visitors should implement the following methods:
  ///
  /// R machine_int(const MachineInt&);
  /// R floating_point(const DummyNumber&);
  /// R memory_location(MemoryLocationRef);
  /// R null();
  /// R undefined();
  /// R machine_int_var(VariableRef);
  /// R floating_point_var(VariableRef);
  /// R pointer_var(VariableRef);
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

    /// \brief Constructor
    explicit VariantVisitor(Visitor& _v) : v(_v) {}

    ResultType operator()(const MachineIntLit& lit) {
      return v.machine_int(lit.value);
    }

    ResultType operator()(const FloatingPointLit& lit) {
      return v.floating_point(lit.value);
    }

    ResultType operator()(const MemoryLocationLit& lit) {
      return v.memory_location(lit.value);
    }

    ResultType operator()(const NullLit&) { return v.null(); }

    ResultType operator()(const UndefinedLit&) { return v.undefined(); }

    ResultType operator()(const MachineIntVarLit& lit) {
      return v.machine_int_var(lit.var);
    }

    ResultType operator()(const FloatingPointVarLit& lit) {
      return v.floating_point_var(lit.var);
    }

    ResultType operator()(const PointerVarLit& lit) {
      return v.pointer_var(lit.var);
    }

  }; // end struct VariantVisitor

  template < typename Visitor >
  struct ConstVariantVisitor
      : public boost::static_visitor< typename Visitor::ResultType > {
    using ResultType = typename Visitor::ResultType;

    const Visitor& v;

    /// \brief Constructor
    explicit ConstVariantVisitor(const Visitor& _v) : v(_v) {}

    ResultType operator()(const MachineIntLit& lit) const {
      return v.machine_int(lit.value);
    }

    ResultType operator()(const FloatingPointLit& lit) const {
      return v.floating_point(lit.value);
    }

    ResultType operator()(const MemoryLocationLit& lit) const {
      return v.memory_location(lit.value);
    }

    ResultType operator()(const NullLit&) const { return v.null(); }

    ResultType operator()(const UndefinedLit&) const { return v.undefined(); }

    ResultType operator()(const MachineIntVarLit& lit) const {
      return v.machine_int_var(lit.var);
    }

    ResultType operator()(const FloatingPointVarLit& lit) const {
      return v.floating_point_var(lit.var);
    }

    ResultType operator()(const PointerVarLit& lit) const {
      return v.pointer_var(lit.var);
    }

  }; // end struct ConstVariantVisitor

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

    void operator()(const MachineIntLit& lit) {
      o << "machine_int{" << lit.value << "}";
    }

    void operator()(const FloatingPointLit& lit) {
      o << "floating_point{" << lit.value << "}";
    }

    void operator()(const MemoryLocationLit& lit) {
      o << "memory_location{";
      DumpableTraits< MemoryLocationRef >::dump(o, lit.value);
      o << "}";
    }

    void operator()(const NullLit&) { o << "null"; }

    void operator()(const UndefinedLit&) { o << "undefined"; }

    void operator()(const MachineIntVarLit& lit) {
      o << "machine_int_var{";
      DumpableTraits< VariableRef >::dump(o, lit.var);
      o << "}";
    }

    void operator()(const FloatingPointVarLit& lit) {
      o << "floating_point_var{";
      DumpableTraits< VariableRef >::dump(o, lit.var);
      o << "}";
    }

    void operator()(const PointerVarLit& lit) {
      o << "pointer_var{";
      DumpableTraits< VariableRef >::dump(o, lit.var);
      o << "}";
    }
  };

public:
  /// \brief Dump the literal on a stream
  void dump(std::ostream& o) const {
    Dumper dumper(o);
    boost::apply_visitor(dumper, this->_lit);
  }

}; // end class Literal

/// \brief Write a literal on a stream
template < typename VariableRef, typename MemoryLocationRef >
std::ostream& operator<<(std::ostream& o,
                         const Literal< VariableRef, MemoryLocationRef >& l) {
  l.dump(o);
  return o;
}

} // end namespace core
} // end namespace ikos
