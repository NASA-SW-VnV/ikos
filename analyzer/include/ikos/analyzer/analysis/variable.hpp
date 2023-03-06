/*******************************************************************************
 *
 * \file
 * \brief Variable types and VariableFactory used by the analyses
 *
 * Author: Clement Decoodt
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

#include <memory>
#include <string>
#include <unordered_map>

#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>

#include <ikos/core/semantic/dumpable.hpp>
#include <ikos/core/semantic/indexable.hpp>
#include <ikos/core/semantic/machine_int/variable.hpp>
#include <ikos/core/semantic/memory/value/cell_factory.hpp>
#include <ikos/core/semantic/memory/value/cell_variable.hpp>
#include <ikos/core/semantic/scalar/variable.hpp>
#include <ikos/core/semantic/variable.hpp>

#include <ikos/ar/semantic/function.hpp>
#include <ikos/ar/semantic/type.hpp>
#include <ikos/ar/semantic/value.hpp>

#include <ikos/analyzer/analysis/memory_location.hpp>
#include <ikos/analyzer/support/cast.hpp>
#include <ikos/analyzer/support/number.hpp>

namespace ikos {
namespace analyzer {

/// \brief Represents a variable, with a reference on the AR representation
///
/// Variable is an abstract class, and implementation of the Variable generic
/// object in the core.
///
/// See ikos/core/semantic/variable.hpp
/// It must be indexable (see ikos/core/semantic/indexable.hpp)
/// It must be dumpable (see ikos/core/semantic/dumpable.hpp)
/// It must implement machine_int::VariableTraits,
///   see ikos/core/semantic/machine_int/variable.hpp
/// It must implement scalar::VariableTraits,
///   see ikos/core/semantic/scalar/variable.hpp
/// It must implement memory::CellVariableTraits,
///   see ikos/core/semantic/memory/value/cell_variable.hpp
///
/// Variable has a 'kind', which is the type of variable it represents.
/// The 'kind' is an element of VariableKind.
///
/// Variable* should be passed everywhere. The factory that creates the Variable
/// owns the pointer, so you don't need to free it.
class Variable {
public:
  /// \brief Kind of the variable
  enum VariableKind {
    _BeginArVariableKind,
    LocalVariableKind,
    GlobalVariableKind,
    InternalVariableKind,
    InlineAssemblyPointerVariableKind,
    FunctionPointerVariableKind,
    _EndArVariableKind,
    _BeginShadowVariableKind,
    CellVariableKind,
    OffsetVariableKind,
    AllocSizeVariableKind,
    ReturnVariableKind,
    NamedShadowVariableKind,
    UnnamedShadowVariableKind,
    _EndShadowVariableKind,
  };

protected:
  /// \brief Kind of the variable
  VariableKind _kind;

  /// \brief Type of the variable
  ar::Type* _type;

  /// \brief The offset variable, or nullptr if it is not a pointer
  std::unique_ptr< Variable > _offset_var;

protected:
  /// \brief Protected constructor
  Variable(VariableKind kind, ar::Type* type);

public:
  /// \brief No copy constructor
  Variable(const Variable&) = delete;

  /// \brief No move constructor
  Variable(Variable&&) = delete;

  /// \brief No copy assignment operator
  Variable& operator=(const Variable&) = delete;

  /// \brief No move assignment operator
  Variable& operator=(Variable&&) = delete;

  /// \brief Destructor
  virtual ~Variable();

  /// \brief Return the kind of the object
  VariableKind kind() const { return this->_kind; }

  /// \brief Return the type of the variable
  ar::Type* type() const { return this->_type; }

  /// \brief Return the offset variable, or nullptr if it is not a pointer
  Variable* offset_var() const { return this->_offset_var.get(); }

  /// \brief Set the offset variable
  void set_offset_var(std::unique_ptr< Variable > offset_var) {
    this->_offset_var = std::move(offset_var);
  }

  /// \brief Dump the variable, for debugging purpose
  virtual void dump(std::ostream&) const = 0;

}; // end class Variable

/// \brief Local variable
class LocalVariable final : public Variable {
protected:
  /// \brief AR Local Variable
  ar::LocalVariable* _var;

public:
  /// \brief Default constructor
  explicit LocalVariable(ar::LocalVariable* var);

  /// \brief Get the ar::LocalVariable*
  ar::LocalVariable* local_var() const { return this->_var; }

  /// \brief Dump the variable, for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Variable* v) {
    return v->kind() == LocalVariableKind;
  }

}; // end class LocalVariableKind

/// \brief Global variable
class GlobalVariable final : public Variable {
protected:
  /// \brief AR Global Variable
  ar::GlobalVariable* _var;

public:
  /// \brief Default constructor
  explicit GlobalVariable(ar::GlobalVariable* var);

  /// \brief Get the ar::GlobalVariable*
  ar::GlobalVariable* global_var() const { return this->_var; }

  /// \brief Dump the variable, for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Variable* v) {
    return v->kind() == GlobalVariableKind;
  }

}; // end class GlobalVariable

/// \brief Internal variable
class InternalVariable final : public Variable {
protected:
  /// \brief AR Internal Variable
  ar::InternalVariable* _var;

public:
  /// \brief Default constructor
  explicit InternalVariable(ar::InternalVariable* var);

  /// \brief Get the ar::InternalVariable*
  ar::InternalVariable* internal_var() const { return this->_var; }

  /// \brief Dump the variable, for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Variable* v) {
    return v->kind() == InternalVariableKind;
  }

}; // end class InternalVariable

/// \brief Inline assembly pointer variable
class InlineAssemblyPointerVariable final : public Variable {
private:
  /// \brief Inline Assembly
  ar::InlineAssemblyConstant* _inline_asm;

public:
  /// \brief Default constructor
  explicit InlineAssemblyPointerVariable(
      ar::InlineAssemblyConstant* inline_asm);

  /// \brief Get the ar::InlineAssemblyConstant*
  ar::InlineAssemblyConstant* inline_asm() const { return this->_inline_asm; }

  /// \brief Dump the variable, for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Variable* v) {
    return v->kind() == InlineAssemblyPointerVariableKind;
  }

}; // end class InlineAssemblyPointerVariable

/// \brief Function pointer variable
class FunctionPointerVariable final : public Variable {
private:
  /// \brief Function
  ar::Function* _fun;

public:
  /// \brief Default constructor
  explicit FunctionPointerVariable(ar::Function* fun);

  /// \brief Get the ar::Function*
  ar::Function* function() const { return this->_fun; }

  /// \brief Dump the variable, for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Variable* v) {
    return v->kind() == FunctionPointerVariableKind;
  }

}; // end class FunctionPointerVariable

/// \brief Cell variable
///
/// Symbolic variable representing the value at a specific memory location
class CellVariable final : public Variable {
private:
  /// \brief Base address
  MemoryLocation* _address;

  /// \brief Offset (in bytes)
  MachineInt _offset;

  /// \brief Size (in bytes)
  MachineInt _size;

public:
  /// \brief Default constructor
  CellVariable(ar::Type* type,
               MemoryLocation* address,
               MachineInt offset,
               MachineInt size);

  /// \brief Get the MemoryLocation* the cell refers to
  MemoryLocation* address() const { return this->_address; }

  /// \brief Get the offset of the cell
  const MachineInt& offset() const { return this->_offset; }

  /// \brief Get the size of the cell
  const MachineInt& size() const { return this->_size; }

  /// \brief Dump the variable, for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Variable* v) {
    return v->kind() == CellVariableKind;
  }

}; // end class CellVariable

/// \brief Offset variable
///
/// Symbolic variable representing the offset of a pointer variable
class OffsetVariable final : public Variable {
private:
  /// \brief Parent variable
  Variable* _pointer;

public:
  /// \brief Default constructor
  OffsetVariable(ar::Type* type, Variable* pointer);

  /// \brief Get the pointer variable
  Variable* pointer() const { return this->_pointer; }

  /// \brief Dump the variable, for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Variable* v) {
    return v->kind() == OffsetVariableKind;
  }

}; // end class OffsetVariable

/// \brief Allocation size variable
///
/// Symbolic variable representing the size of a memory location
class AllocSizeVariable final : public Variable {
private:
  /// \brief Base address
  MemoryLocation* _address;

public:
  /// \brief Default constructor
  AllocSizeVariable(ar::Type* type, MemoryLocation* address);

  /// \brief Get the base address
  MemoryLocation* address() const { return this->_address; }

  /// \brief Dump the variable, for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Variable* v) {
    return v->kind() == AllocSizeVariableKind;
  }

}; // end class AllocSizeVariable

/// \brief Return variable of a function
///
/// Symbolic variable representing the return variable of a function
class ReturnVariable final : public Variable {
private:
  /// \brief Function
  ar::Function* _fun;

public:
  /// \brief Default constructor
  explicit ReturnVariable(ar::Function* fun);

  /// \brief Get the ar::Function*
  ar::Function* function() const { return this->_fun; }

  /// \brief Dump the variable, for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Variable* v) {
    return v->kind() == ReturnVariableKind;
  }

}; // end class ReturnVariable

/// \brief Named shadow variable
///
/// Variable with a unique name, used to represents values unrelated to AR.
class NamedShadowVariable final : public Variable {
private:
  std::string _name;

public:
  /// \brief Default constructor
  NamedShadowVariable(ar::Type* type, std::string name);

  /// \brief Get the variable name
  const std::string& name() const { return this->_name; }

  /// \brief Dump the variable, for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Variable* v) {
    return v->kind() == NamedShadowVariableKind;
  }

}; // end class NamedShadowVariable

/// \brief Unnamed shadow variable
///
/// Temporary unnamed variable, used to represents values unrelated to AR.
class UnnamedShadowVariable final : public Variable {
private:
  std::size_t _id;

public:
  /// \brief Default constructor
  UnnamedShadowVariable(ar::Type* type, std::size_t id);

  /// \brief Dump the variable, for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Variable* v) {
    return v->kind() == UnnamedShadowVariableKind;
  }

}; // end class UnnamedShadowVariable

/// \brief Management of variables
class VariableFactory {
private:
  /// \brief Hash function for _cell_map
  struct CellMapKeyHash {
    std::size_t operator()(
        const std::tuple< MemoryLocation*, MachineInt, MachineInt >& k) const {
      std::size_t hash = 0;
      boost::hash_combine(hash, std::get< 0 >(k));
      boost::hash_combine(hash, std::get< 1 >(k));
      boost::hash_combine(hash, std::get< 2 >(k));
      return hash;
    }
  };

private:
  /// \brief The AR context
  ar::Context& _ar_context;

  /// \brief Represents the type of an offset or a size (similar to std::size_t)
  ///
  /// This is an unsigned integer with the bit-width of a pointer
  ar::IntegerType* _size_type;

  boost::shared_mutex _local_variable_mutex;

  llvm::DenseMap< ar::LocalVariable*, std::unique_ptr< LocalVariable > >
      _local_variable_map;

  boost::shared_mutex _global_variable_mutex;

  llvm::DenseMap< ar::GlobalVariable*, std::unique_ptr< GlobalVariable > >
      _global_variable_map;

  boost::shared_mutex _internal_variable_mutex;

  llvm::DenseMap< ar::InternalVariable*, std::unique_ptr< InternalVariable > >
      _internal_variable_map;

  boost::shared_mutex _inline_asm_pointer_mutex;

  llvm::DenseMap< ar::InlineAssemblyConstant*,
                  std::unique_ptr< InlineAssemblyPointerVariable > >
      _inline_asm_pointer_map;

  boost::shared_mutex _function_pointer_mutex;

  llvm::DenseMap< ar::Function*, std::unique_ptr< FunctionPointerVariable > >
      _function_pointer_map;

  boost::shared_mutex _cell_mutex;

  std::unordered_map< std::tuple< MemoryLocation*, MachineInt, MachineInt >,
                      std::unique_ptr< CellVariable >,
                      CellMapKeyHash >
      _cell_map;

  boost::shared_mutex _alloc_size_mutex;

  llvm::DenseMap< MemoryLocation*, std::unique_ptr< AllocSizeVariable > >
      _alloc_size_map;

  boost::shared_mutex _return_variable_mutex;

  llvm::DenseMap< ar::Function*, std::unique_ptr< ReturnVariable > >
      _return_variable_map;

  boost::shared_mutex _named_shadow_variable_mutex;

  llvm::StringMap< std::unique_ptr< NamedShadowVariable > >
      _named_shadow_variable_map;

  boost::mutex _unnamed_shadow_variable_mutex;

  std::vector< std::unique_ptr< UnnamedShadowVariable > >
      _unnamed_shadow_variable_vec;

public:
  /// \brief Constructor
  explicit VariableFactory(ar::Bundle* bundle);

  /// \brief No copy constructor
  VariableFactory(const VariableFactory&) = delete;

  /// \brief No move constructor
  VariableFactory(VariableFactory&&) = delete;

  /// \brief No copy assignment operator
  VariableFactory& operator=(const VariableFactory&) = delete;

  /// \brief No move assignment operator
  VariableFactory& operator=(VariableFactory&&) = delete;

  /// \brief Destructor
  ~VariableFactory();

public:
  /// \brief Get or Create a LocalVariable
  LocalVariable* get_local(ar::LocalVariable* var);

  /// \brief Get or Create a GlobalVariable
  GlobalVariable* get_global(ar::GlobalVariable* var);

  /// \brief Get or Create an InternalVariable
  InternalVariable* get_internal(ar::InternalVariable* var);

  /// \brief Get or Create a InlineAssemblyPointerVariable
  InlineAssemblyPointerVariable* get_asm_ptr(ar::InlineAssemblyConstant* cst);

  /// \brief Get or Create a FunctionPointerVariable
  FunctionPointerVariable* get_function_ptr(ar::Function* fun);

  /// \brief Get or Create a FunctionPointerVariable
  FunctionPointerVariable* get_function_ptr(ar::FunctionPointerConstant* cst);

  /// \brief Get or Create a CellVariable
  CellVariable* get_cell(MemoryLocation* address,
                         const MachineInt& offset,
                         const MachineInt& size,
                         Signedness sign);

  /// \brief Get or Create an AllocSizeVariable
  AllocSizeVariable* get_alloc_size(MemoryLocation* address);

  /// \brief Get or Create a ReturnVariable
  ReturnVariable* get_return(ar::Function* fun);

  /// \brief Get or Create a NamedShadowVariable
  NamedShadowVariable* get_named_shadow(ar::Type* type, llvm::StringRef name);

  /// \brief Create a new UnnamedShadowVariable
  UnnamedShadowVariable* create_unnamed_shadow(ar::Type* type);

}; // end class VariableFactory

} // end namespace analyzer
} // end namespace ikos

namespace ikos {
namespace core {

/// \brief Implement IndexableTraits for Variable*
///
/// The index of Variable* is the address of the pointer.
template <>
struct IndexableTraits< analyzer::Variable* > {
  static Index index(const analyzer::Variable* v) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast< Index >(v);
  }
};

/// \brief Implement DumpableTraits for Variable*
template <>
struct DumpableTraits< analyzer::Variable* > {
  static void dump(std::ostream& o, const analyzer::Variable* v) { v->dump(o); }
};

namespace machine_int {

/// \brief Implement machine_int::VariableTraits for Variable*
template <>
struct VariableTraits< analyzer::Variable* > {
  /// \brief Return the bit width of the given variable
  static uint64_t bit_width(const analyzer::Variable* v) {
    ikos_assert_msg(v->type()->is_integer(), "variable is not an integer");
    return ar::cast< ar::IntegerType >(v->type())->bit_width();
  }

  /// \brief Return the sign of the given variable
  static Signedness sign(const analyzer::Variable* v) {
    ikos_assert_msg(v->type()->is_integer(), "variable is not an integer");
    return ar::cast< ar::IntegerType >(v->type())->sign();
  }
};

} // end namespace machine_int

namespace scalar {

/// \brief Implement scalar::VariableTraits for Variable*
template <>
struct VariableTraits< analyzer::Variable* > {
  /// \brief Return true if the given variable is a machine integer variable
  static bool is_int(const analyzer::Variable* v) {
    return !analyzer::isa< analyzer::CellVariable >(v) &&
           v->type()->is_integer();
  }

  /// \brief Return true if the given variable is a floating point variable
  static bool is_float(const analyzer::Variable* v) {
    return !analyzer::isa< analyzer::CellVariable >(v) && v->type()->is_float();
  }

  /// \brief Return true if the given variable is a pointer variable
  static bool is_pointer(const analyzer::Variable* v) {
    // Aggregate variables are treated as pointers for the analysis
    return !analyzer::isa< analyzer::CellVariable >(v) &&
           (v->type()->is_pointer() || v->type()->is_aggregate());
  }

  /// \brief Return true if the given variable is a dynamically typed variable
  static bool is_dynamic(const analyzer::Variable* v) {
    return analyzer::isa< analyzer::CellVariable >(v);
  }

  /// \brief Return the machine integer offset variable of the given pointer
  static analyzer::Variable* offset_var(const analyzer::Variable* v) {
    ikos_assert_msg(v->offset_var() != nullptr, "variable is not a pointer");
    return v->offset_var();
  }
};

} // end namespace scalar

namespace memory {

/// \brief Implement memory::CellVariableTraits for Variable*
template <>
struct CellVariableTraits< analyzer::Variable*, analyzer::MemoryLocation* > {
  /// \brief Return true if the given variable is a memory cell variable
  static bool is_cell(const analyzer::Variable* v) {
    return analyzer::isa< analyzer::CellVariable >(v);
  }

  /// \brief Return the base memory location of the given cell
  static analyzer::MemoryLocation* base(const analyzer::Variable* v) {
    return analyzer::cast< analyzer::CellVariable >(v)->address();
  }

  /// \brief Return the offset of the given cell
  static const MachineInt& offset(const analyzer::Variable* v) {
    return analyzer::cast< analyzer::CellVariable >(v)->offset();
  }

  /// \brief Return the size of the given cell
  static const MachineInt& size(const analyzer::Variable* v) {
    return analyzer::cast< analyzer::CellVariable >(v)->size();
  }
};

/// \brief Implement memory::CellFactoryTraits for VariableFactory
template <>
struct CellFactoryTraits< analyzer::Variable*,
                          analyzer::MemoryLocation*,
                          analyzer::VariableFactory* > {
  /// \brief Get or create the cell with the given base address, offset and size
  static analyzer::Variable* cell(analyzer::VariableFactory* vfac,
                                  analyzer::MemoryLocation* base,
                                  const MachineInt& offset,
                                  const MachineInt& size,
                                  Signedness sign) {
    return vfac->get_cell(base, offset, size, sign);
  }
};

} // end namespace memory

} // end namespace core
} // end namespace ikos
