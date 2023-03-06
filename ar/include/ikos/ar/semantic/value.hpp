/*******************************************************************************
 *
 * \file
 * \brief Value definitions (variables, constants, etc.)
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

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/context.hpp>
#include <ikos/ar/semantic/type.hpp>
#include <ikos/ar/support/assert.hpp>
#include <ikos/ar/support/cast.hpp>
#include <ikos/ar/support/number.hpp>
#include <ikos/ar/support/traceable.hpp>

namespace ikos {
namespace ar {

// forward declaration
class Code;
class Function;

/// \brief Base class for values (ie., constants and variables)
///
/// Note that only one instance of each value is ever created. Thus, you can
/// compare two values using a pointer comparison.
///
/// To enforce this, each value should be created using the static method get().
class Value {
public:
  enum ValueKind {
    _BeginConstantKind,
    UndefinedConstantKind,
    IntegerConstantKind,
    FloatConstantKind,
    NullConstantKind,
    StructConstantKind,
    _BeginSequentialConstantKind,
    ArrayConstantKind,
    VectorConstantKind,
    _EndSequentialConstantKind,
    AggregateZeroConstantKind,
    FunctionPointerConstantKind,
    InlineAssemblyConstantKind,
    _EndConstantKind,
    _BeginVariableKind,
    GlobalVariableKind,
    LocalVariableKind,
    InternalVariableKind,
    _EndVariableKind
  };

protected:
  // Kind of value
  ValueKind _kind;

  // Type
  Type* _type;

protected:
  /// \brief Protected constructor
  Value(ValueKind kind, Type* type);

public:
  /// \brief No copy constructor
  Value(const Value&) = delete;

  /// \brief No move constructor
  Value(Value&&) = delete;

  /// \brief No copy assignment operator
  Value& operator=(const Value&) = delete;

  /// \brief No move assignment operator
  Value& operator=(Value&&) = delete;

  /// \brief Destructor
  virtual ~Value();

  /// \brief Get the kind of value
  ValueKind kind() const { return this->_kind; }

  /// \brief Get the type
  Type* type() const { return this->_type; }

  /// \brief Dump the value for debugging purpose
  virtual void dump(std::ostream&) const = 0;

public:
  /// \brief Is it a constant?
  bool is_constant() const {
    return this->_kind >= _BeginConstantKind && this->_kind <= _EndConstantKind;
  }

  /// \brief Is it an undefined constant?
  bool is_undefined_constant() const {
    return this->_kind == UndefinedConstantKind;
  }

  /// \brief Is it an integer constant?
  bool is_integer_constant() const {
    return this->_kind == IntegerConstantKind;
  }

  /// \brief Is it a float constant?
  bool is_float_constant() const { return this->_kind == FloatConstantKind; }

  /// \brief Is it a null constant?
  bool is_null_constant() const { return this->_kind == NullConstantKind; }

  /// \brief Is it a constant structure?
  bool is_struct_constant() const { return this->_kind == StructConstantKind; }

  /// \brief Is it a sequential constant?
  bool is_sequential_constant() const {
    return this->_kind >= _BeginSequentialConstantKind &&
           this->_kind <= _EndSequentialConstantKind;
  }

  /// \brief Is it a constant array?
  bool is_array_constant() const { return this->_kind == ArrayConstantKind; }

  /// \brief Is it a constant vector?
  bool is_vector_constant() const { return this->_kind == VectorConstantKind; }

  /// \brief Is it a constant aggregate zero?
  bool is_aggregate_zero_constant() const {
    return this->_kind == AggregateZeroConstantKind;
  }

  /// \brief Is it a constant function pointer?
  bool is_function_pointer_constant() const {
    return this->_kind == FunctionPointerConstantKind;
  }

  /// \brief Is it an inline assembly?
  bool is_inline_assembly_constant() const {
    return this->_kind == InlineAssemblyConstantKind;
  }

  /// \brief Is it a variable?
  bool is_variable() const {
    return this->_kind >= _BeginVariableKind && this->_kind <= _EndVariableKind;
  }

  /// \brief Is it a global variable?
  bool is_global_variable() const { return this->_kind == GlobalVariableKind; }

  /// \brief Is it a local variable?
  bool is_local_variable() const { return this->_kind == LocalVariableKind; }

  /// \brief Is it an internal variable?
  bool is_internal_variable() const {
    return this->_kind == InternalVariableKind;
  }

protected:
  /// \brief Get the context implementation
  ///
  /// Helper for derived values
  static ContextImpl& ctx_impl(Context& ctx);

}; // end class Value

/// \brief Base class for constants
class Constant : public Value {
protected:
  /// \brief Protected constructor
  Constant(ValueKind kind, Type* type);

public:
  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Value* v) {
    return v->kind() >= _BeginConstantKind && v->kind() <= _EndConstantKind;
  }

}; // end class Constant

/// \brief Undefined constant
class UndefinedConstant final : public Constant {
private:
  /// \brief Private constructor
  explicit UndefinedConstant(Type* type);

public:
  /// \brief Static constructor
  static UndefinedConstant* get(Context& ctx, Type* type);

  /// \brief Dump the value for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Value* v) {
    return v->kind() == UndefinedConstantKind;
  }

  // friends
  friend class ContextImpl;

}; // end class UndefinedConstant

/// \brief Integer constant
class IntegerConstant final : public Constant {
private:
  // Normalized value
  MachineInt _value;

private:
  /// \brief Private constructor
  IntegerConstant(IntegerType* type, MachineInt value);

public:
  /// \brief Static constructor
  static IntegerConstant* get(Context& ctx,
                              IntegerType* type,
                              const MachineInt& value);

  /// \brief Static constructor
  static IntegerConstant* get(Context& ctx, IntegerType* type, int value);

  /// \brief Get the type
  IntegerType* type() const { return cast< IntegerType >(this->_type); }

  /// \brief Get the value
  const MachineInt& value() const { return this->_value; }

  /// \brief Dump the value for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Value* v) {
    return v->kind() == IntegerConstantKind;
  }

  // friends
  friend class ContextImpl;

}; // end class IntegerConstant

/// \brief Floating point constant
class FloatConstant final : public Constant {
private:
  // Decimal representation, in scientific notation
  // TODO(marthaud): use a better representation (see llvm::APFloat for example)
  // Note: QNumber cannot be used (no representation for +Inf, -Inf, NaN, etc.)
  std::string _value;

private:
  /// \brief Private constructor
  FloatConstant(FloatType* type, std::string value);

public:
  /// \brief Static constructor
  ///
  /// \param value Decimal representation, in scientific notation
  static FloatConstant* get(Context& ctx,
                            FloatType* type,
                            const std::string& value);

  /// \brief Static constructor
  ///
  /// \param value Decimal representation, in scientific notation
  static FloatConstant* get(Context& ctx, FloatType* type, const char* value);

  /// \brief Get the type
  FloatType* type() const { return cast< FloatType >(this->_type); }

  /// \brief Get the decimal representation, in scientific notation
  const std::string& value() const { return this->_value; }

  /// \brief Dump the value for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Value* v) { return v->kind() == FloatConstantKind; }

  // friends
  friend class ContextImpl;

}; // end class FloatConstant

/// \brief Null constant
class NullConstant final : public Constant {
private:
  /// \brief Private constructor
  explicit NullConstant(PointerType* type);

public:
  /// \brief Static constructor
  static NullConstant* get(Context& ctx, PointerType* type);

  /// \brief Get the type
  PointerType* type() const { return cast< PointerType >(this->_type); }

  /// \brief Dump the value for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Value* v) { return v->kind() == NullConstantKind; }

  // friends
  friend class ContextImpl;

}; // end class NullConstant

/// \brief Constant structure
class StructConstant final : public Constant {
public:
  /// \brief Structure field
  struct Field {
    // Offset, in bytes
    ZNumber offset;

    // Value
    Value* value;

    bool operator<(const Field& o) const {
      return offset < o.offset || (offset == o.offset && value < o.value);
    }
  };

  /// \brief Type of the value container
  ///
  /// List of fields, ordered by offset
  using Values = std::vector< Field >;

  /// \brief Iterator over the fields of the structure
  using FieldIterator = Values::const_iterator;

  /// \brief Reverse iterator over the fields of the structure
  using FieldReverseIterator = Values::const_reverse_iterator;

private:
  // Values in the structure
  Values _values;

private:
  /// \brief Private constructor
  StructConstant(StructType* type, Values values);

public:
  /// \brief Static constructor
  static StructConstant* get(Context& ctx,
                             StructType* type,
                             const Values& values);

  /// \brief Get the type
  StructType* type() const { return cast< StructType >(this->_type); }

  /// \brief Get the values
  const Values& values() const { return this->_values; }

  /// \brief Begin iterator over the fields of the structure
  FieldIterator field_begin() const { return this->_values.cbegin(); }

  /// \brief End iterator over the fields of the structure
  FieldIterator field_end() const { return this->_values.cend(); }

  /// \brief Begin reverse iterator over the fields of the structure
  FieldReverseIterator field_rbegin() const { return this->_values.crbegin(); }

  /// \brief End reverse iterator over the fields of the structure
  FieldReverseIterator field_rend() const { return this->_values.crend(); }

  /// \brief Get the number of fields
  std::size_t num_fields() const { return this->_values.size(); }

  /// \brief Dump the value for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Value* v) {
    return v->kind() == StructConstantKind;
  }

  // friends
  friend class ContextImpl;

}; // end class StructConstant

/// \brief Base class for sequential constants,
/// such as ArrayConstant and VectorConstant
class SequentialConstant : public Constant {
public:
  /// \brief Type of the value container
  using Values = std::vector< Value* >;

  /// \brief Iterator over the elements of the array
  using ElementIterator = Values::const_iterator;

private:
  // Values
  Values _values;

protected:
  /// \brief Private constructor
  SequentialConstant(ValueKind kind,
                     SequentialType* type,
                     const Values& values);

public:
  /// \brief Get the type
  SequentialType* type() const { return cast< SequentialType >(this->_type); }

  /// \brief Get the values
  const Values& values() const { return this->_values; }

  /// \brief Begin iterator over the elements of the sequence
  ElementIterator element_begin() const { return this->_values.cbegin(); }

  /// \brief End iterator over the elements of the sequence
  ElementIterator element_end() const { return this->_values.cend(); }

  /// \brief Get the number of elements
  std::size_t num_elements() const { return this->_values.size(); }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Value* v) {
    return v->kind() >= _BeginSequentialConstantKind &&
           v->kind() <= _EndSequentialConstantKind;
  }

}; // end class SequentialConstant

/// \brief Constant array
class ArrayConstant final : public SequentialConstant {
private:
  /// \brief Private constructor
  ArrayConstant(ArrayType* type, const Values& values);

public:
  /// \brief Static constructor
  static ArrayConstant* get(Context& ctx,
                            ArrayType* type,
                            const Values& values);

  /// \brief Get the type
  ArrayType* type() const { return cast< ArrayType >(this->_type); }

  /// \brief Dump the value for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Value* v) { return v->kind() == ArrayConstantKind; }

  // friends
  friend class ContextImpl;

}; // end class ArrayConstant

/// \brief Constant vector
class VectorConstant final : public SequentialConstant {
private:
  /// \brief Private constructor
  VectorConstant(VectorType* type, const Values& values);

public:
  /// \brief Static constructor
  static VectorConstant* get(Context& ctx,
                             VectorType* type,
                             const Values& values);

  /// \brief Get the type
  VectorType* type() const { return cast< VectorType >(this->_type); }

  /// \brief Dump the value for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Value* v) {
    return v->kind() == VectorConstantKind;
  }

  // friends
  friend class ContextImpl;

}; // end class VectorConstant

/// \brief Constant aggregate full of zeros
class AggregateZeroConstant final : public Constant {
private:
  /// \brief Private constructor
  explicit AggregateZeroConstant(AggregateType* type);

public:
  /// \brief Static constructor
  static AggregateZeroConstant* get(Context& ctx, AggregateType* type);

  /// \brief Get the type
  AggregateType* type() const { return cast< AggregateType >(this->_type); }

  /// \brief Dump the value for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Value* v) {
    return v->kind() == AggregateZeroConstantKind;
  }

  // friends
  friend class ContextImpl;

}; // end class AggregateZeroConstant

/// \brief Constant function pointer
class FunctionPointerConstant final : public Constant {
private:
  // Function
  Function* _function;

private:
  /// \brief Private constructor
  FunctionPointerConstant(PointerType* type, Function* function);

public:
  /// \brief Static constructor
  static FunctionPointerConstant* get(Context& ctx, Function* function);

  /// \brief Get the type
  PointerType* type() const { return cast< PointerType >(this->_type); }

  /// \brief Get the function
  Function* function() const { return this->_function; }

  /// \brief Dump the value for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Value* v) {
    return v->kind() == FunctionPointerConstantKind;
  }

  // friends
  friend class ContextImpl;

}; // end class FunctionPointerConstant

/// \brief Inline Assembly code
class InlineAssemblyConstant final : public Constant {
private:
  // Assembly code
  std::string _code;

private:
  /// \brief Private constructor
  InlineAssemblyConstant(PointerType* type, std::string code);

public:
  /// \brief Static constructor
  static InlineAssemblyConstant* get(Context& ctx,
                                     PointerType* type,
                                     const std::string& code);

  /// \brief Get the type
  PointerType* type() const { return cast< PointerType >(this->_type); }

  /// \brief Get the assembly code as a string
  const std::string& code() const { return this->_code; }

  /// \brief Dump the value for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Value* v) {
    return v->kind() == InlineAssemblyConstantKind;
  }

  // friends
  friend class ContextImpl;

}; // end class InlineAssemblyConstant

/// \brief Base class for variables
class Variable : public Value, public Traceable {
protected:
  // Name (optional)
  std::string _name;

protected:
  /// \brief Protected constructor
  Variable(ValueKind kind, Type* type);

public:
  /// \brief Does the variable have a name?
  bool has_name() const { return !this->_name.empty(); }

  /// \brief Get the name
  const std::string& name() const {
    ikos_assert_msg(!this->_name.empty(), "variable has no name");
    return this->_name;
  }

  /// \brief Get the name
  ///
  /// Returns an empty string if the variable has no name.
  const std::string& name_or_empty() const { return this->_name; }

  /// \brief Set the name
  virtual void set_name(std::string name);

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Value* v) {
    return v->kind() >= _BeginVariableKind && v->kind() <= _EndVariableKind;
  }

}; // end class Variable

/// \brief Global variable
class GlobalVariable final : public Variable {
private:
  // Parent bundle
  Bundle* _parent;

  // Initializer (if definition)
  std::unique_ptr< Code > _initializer;

  // Alignment of the global variable, in bytes (0 if unspecified)
  uint64_t _alignment;

private:
  /// \brief Private constructor
  GlobalVariable(Bundle* bundle,
                 PointerType* type,
                 std::string name,
                 bool is_definition,
                 uint64_t alignment);

public:
  /// \brief Static constructor
  ///
  /// \param bundle Parent bundle
  /// \param type Type of the global variable
  /// \param name Name of the global variable
  /// \param is_definition True if it is a definition, false otherwise
  /// \param alignment Explicit alignment in bytes, or 0 if unspecified
  static GlobalVariable* create(Bundle* bundle,
                                PointerType* type,
                                std::string name,
                                bool is_definition,
                                uint64_t alignment);

  /// \brief Get the parent context
  Context& context() const { return this->bundle()->context(); }

  /// \brief Get parent bundle
  Bundle* bundle() const { return this->_parent; }

  /// \brief Get the type
  PointerType* type() const { return cast< PointerType >(this->_type); }

  /// \brief Set the name
  void set_name(std::string new_name) override;

  /// \brief Is this a declaration?
  bool is_declaration() const { return this->_initializer == nullptr; }

  /// \brief Is this a definition?
  bool is_definition() const { return this->_initializer != nullptr; }

  /// \brief Get the global variable initializer code
  Code* initializer() const {
    ikos_assert_msg(this->_initializer, "global variable is a declaration");
    return this->_initializer.get();
  }

  /// \brief Get the global variable initializer code, or null
  Code* initializer_or_null() const { return this->_initializer.get(); }

  /// \brief Get the alignment of the global variable in memory, in bytes
  ///
  /// Returns 0 if the alignment is unspecified.
  uint64_t alignment() const { return this->_alignment; }

  /// \brief Return true if the global variable has a specified alignment
  bool has_alignment() const { return this->alignment() > 0; }

  /// \brief Dump the value for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Value* v) {
    return v->kind() == GlobalVariableKind;
  }

}; // end class GlobalVariable

/// \brief Local variable
class LocalVariable final : public Variable {
private:
  // Parent function
  Function* _parent;

  // Alignment of the local variable, in bytes (0 if unspecified)
  uint64_t _alignment;

private:
  /// \brief Private constructor
  LocalVariable(Function* function, PointerType* type, uint64_t alignment);

public:
  /// \brief Static constructor
  ///
  /// \param function Parent function
  /// \param type Type of the local variable
  /// \param alignment Explicit alignment in bytes, or 0 if unspecified
  static LocalVariable* create(Function* function,
                               PointerType* type,
                               uint64_t alignment);

  /// \brief Get the parent context
  Context& context() const;

  /// \brief Get the parent bundle
  Bundle* bundle() const;

  /// \brief Get parent function
  Function* function() const { return this->_parent; }

  /// \brief Get the type
  PointerType* type() const { return cast< PointerType >(this->_type); }

  /// \brief Get the alignment of the local variable in memory, in bytes
  ///
  /// Returns 0 if the alignment is unspecified.
  uint64_t alignment() const { return this->_alignment; }

  /// \brief Return true if the local variable has a specified alignment
  bool has_alignment() const { return this->alignment() > 0; }

  /// \brief Dump the value for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Value* v) { return v->kind() == LocalVariableKind; }

}; // end class LocalVariable

class InternalVariable final : public Variable {
private:
  // Parent code
  Code* _parent;

private:
  /// \brief Private constructor
  InternalVariable(Code* code, Type* type);

public:
  /// \brief Static constructor
  static InternalVariable* create(Code* code, Type* type);

  /// \brief Get the parent context
  Context& context() const;

  /// \brief Get the parent bundle
  Bundle* bundle() const;

  /// \brief Get parent code
  Code* code() const { return this->_parent; }

  /// \brief Dump the value for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Value* v) {
    return v->kind() == InternalVariableKind;
  }

}; // end class InternalVariable

} // end namespace ar
} // end namespace ikos
