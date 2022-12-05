/*******************************************************************************
 *
 * \file
 * \brief Type definitions
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
#include <vector>

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/context.hpp>
#include <ikos/ar/support/assert.hpp>
#include <ikos/ar/support/cast.hpp>
#include <ikos/ar/support/number.hpp>

namespace ikos {
namespace ar {

/// \brief Floating point semantic
enum FloatSemantic { Half, Float, Double, X86_FP80, FP128, PPC_FP128 };

/// \brief Base class for types
///
/// Types are immutable. Once created, they are never changed.
///
/// Note that only one instance of each type is ever created (except for
/// StructType and OpaqueType). Thus, you can compare two types using a
/// pointer comparison.
///
/// To enforce this, each type should be created using the static method get().
class Type {
public:
  enum TypeKind {
    VoidKind,
    _BeginScalarKind,
    IntegerKind,
    FloatKind,
    PointerKind,
    _EndScalarKind,
    _BeginAggregateKind,
    StructKind,
    _BeginSequentialKind,
    ArrayKind,
    VectorKind,
    _EndSequentialKind,
    OpaqueKind,
    _EndAggregateKind,
    FunctionKind
  };

protected:
  // Kind of type
  TypeKind _kind;

protected:
  /// \brief Protected constructor
  explicit Type(TypeKind kind);

public:
  /// \brief No copy constructor
  Type(const Type&) = delete;

  /// \brief No move constructor
  Type(Type&&) = delete;

  /// \brief No copy assignment operator
  Type& operator=(const Type&) = delete;

  /// \brief No move assignment operator
  Type& operator=(Type&&) = delete;

  /// \brief Destructor
  virtual ~Type();

  /// \brief Get the kind of type
  TypeKind kind() const { return this->_kind; }

  /// \brief Dump the type for debugging purpose
  virtual void dump(std::ostream&) const = 0;

public:
  /// \brief Is it a void type?
  bool is_void() const { return this->_kind == VoidKind; }

  /// \brief Is it a scalar type?
  bool is_scalar() const {
    return this->_kind >= _BeginScalarKind && this->_kind <= _EndScalarKind;
  }

  /// \brief Is it an integer type?
  bool is_integer() const { return this->_kind == IntegerKind; }

  /// \brief Is it an unsigned integer type?
  bool is_unsigned_integer() const;

  /// \brief Is it a signed integer type?
  bool is_signed_integer() const;

  /// \brief Is it a float type?
  bool is_float() const { return this->_kind == FloatKind; }

  /// \brief Is it a pointer type?
  bool is_pointer() const { return this->_kind == PointerKind; }

  /// \brief Is it an aggregate type?
  bool is_aggregate() const {
    return this->_kind >= _BeginAggregateKind &&
           this->_kind <= _EndAggregateKind;
  }

  /// \brief Is it a structure type?
  bool is_struct() const { return this->_kind == StructKind; }

  /// \brief Is it a sequential type?
  bool is_sequential() const {
    return this->_kind >= _BeginSequentialKind &&
           this->_kind <= _EndSequentialKind;
  }

  /// \brief Is it an array type?
  bool is_array() const { return this->_kind == ArrayKind; }

  /// \brief Is it a vector type?
  bool is_vector() const { return this->_kind == VectorKind; }

  /// \brief Is it an opaque type?
  bool is_opaque() const { return this->_kind == OpaqueKind; }

  /// \brief Is it a function type?
  bool is_function() const { return this->_kind == FunctionKind; }

  /// \brief Is it a primitive type?
  ///
  /// A primitive type has a fixed bit-width and is target-independent. This is
  /// either an integer, a floating point or a vector of integers or floating
  /// points.
  bool is_primitive() const;

  /// \brief Return the bit width of a primitive type
  ///
  /// Returns zero for non-primitive type.
  uint64_t primitive_bit_width() const;

protected:
  /// \brief Get the context implementation
  ///
  /// Helper for derived types
  static ContextImpl& ctx_impl(Context& ctx);

}; // end class Type

/// \brief Void type
///
/// Represents the void type in C. Mainly used for return type of functions.
class VoidType final : public Type {
private:
  /// \brief Private constructor
  VoidType();

public:
  /// \brief Static constructor
  static VoidType* get(Context& ctx);

  /// \brief Dump the type for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Type* t) { return t->kind() == VoidKind; }

  // friends
  friend class ContextImpl;

}; // end class VoidType

/// \brief Base class for scalar types
class ScalarType : public Type {
protected:
  /// \brief Protected constructor
  explicit ScalarType(TypeKind kind);

public:
  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Type* t) {
    return t->kind() >= _BeginScalarKind && t->kind() <= _EndScalarKind;
  }

}; // end class ScalarType

/// \brief Integer type
///
/// Represents any integer type, with its bit-width and sign.
class IntegerType final : public ScalarType {
private:
  // Bit width
  uint64_t _bit_width;

  // Sign
  Signedness _sign;

private:
  /// \brief Private constructor
  IntegerType(uint64_t bit_width, Signedness sign);

public:
  /// \brief Static constructor
  static IntegerType* get(Context& ctx, uint64_t bit_width, Signedness sign);

  /// \brief Get the signed 1 bit integer type
  static IntegerType* si1(Context& ctx);

  /// \brief Get the unsigned 1 bit integer type
  static IntegerType* ui1(Context& ctx);

  /// \brief Get the signed 8 bits integer type
  static IntegerType* si8(Context& ctx);

  /// \brief Get the unsigned 8 bits integer type
  static IntegerType* ui8(Context& ctx);

  /// \brief Get the signed 32 bits integer type
  static IntegerType* si32(Context& ctx);

  /// \brief Get the unsigned 32 bits integer type
  static IntegerType* ui32(Context& ctx);

  /// \brief Get the signed 64 bits integer type
  static IntegerType* si64(Context& ctx);

  /// \brief Get the unsigned 64 bits integer type
  static IntegerType* ui64(Context& ctx);

  /// \brief Get the unsigned integer type that has the bit-width of a pointer
  static IntegerType* size_type(Bundle* bundle);

  /// \brief Get the signed integer type that has the bit-width of a pointer
  static IntegerType* ssize_type(Bundle* bundle);

  /// \brief Get the bit width
  uint64_t bit_width() const { return this->_bit_width; }

  /// \brief Get the sign
  Signedness sign() const { return this->_sign; }

  bool is_unsigned() const { return this->_sign == Unsigned; }
  bool is_signed() const { return this->_sign == Signed; }

  /// \brief Get the minimum integer represented by this type
  MachineInt min_value() const;

  /// \brief Get the maximum integer represented by this type
  MachineInt max_value() const;

  /// \brief Dump the type for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Type* t) { return t->kind() == IntegerKind; }

  // friends
  friend class ContextImpl;

}; // end class IntegerType

/// \brief Floating point type
///
/// Represents any floating point type, with its bit-width and semantic.
///
/// See `FloatSemantic`.
class FloatType final : public ScalarType {
private:
  // Bit width
  uint64_t _bit_width;

  // Float semantic
  FloatSemantic _float_sem;

private:
  /// \brief Private constructor
  FloatType(uint64_t bit_width, FloatSemantic float_sem);

public:
  /// \brief Static constructor
  static FloatType* get(Context& ctx, FloatSemantic float_sem);

  /// \brief Get the bit width
  uint64_t bit_width() const { return this->_bit_width; }

  /// \brief Get the float semantic
  FloatSemantic float_semantic() const { return this->_float_sem; }

  /// \brief Dump the type for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Type* t) { return t->kind() == FloatKind; }

  // friends
  friend class ContextImpl;

}; // end class FloatType

/// \brief Pointer type
///
/// Represents any pointer type.
class PointerType final : public ScalarType {
private:
  // Pointee type
  Type* _pointee;

private:
  /// \brief Private constructor
  explicit PointerType(Type* pointee);

public:
  /// \brief Static constructor
  static PointerType* get(Context& ctx, Type* pointee);

  /// \brief Get the pointee type
  Type* pointee() const { return this->_pointee; }

  /// \brief Dump the type for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Type* t) { return t->kind() == PointerKind; }

  // friends
  friend class ContextImpl;

}; // end class PointerType

/// \brief Base class for aggregate types
///
/// Base class for StructType, ArrayType, VectorType and OpaqueType.
class AggregateType : public Type {
protected:
  /// \brief Protected constructor
  explicit AggregateType(TypeKind kind);

public:
  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Type* t) {
    return t->kind() >= _BeginAggregateKind && t->kind() <= _EndAggregateKind;
  }

}; // end class AggregateType

/// \brief Structure type
///
/// Structure types might not be unique, so a pointer comparison might not work.
///
/// Structure types might also be recursive (a structure can have a pointer
/// to itself).
class StructType final : public AggregateType {
public:
  /// \brief Structure field
  struct Field {
    // Offset, in bytes
    ZNumber offset;

    // Type
    Type* type;
  };

  /// \brief Type of the layout
  ///
  /// List of fields, ordered by offset
  using Layout = std::vector< Field >;

  /// \brief Iterator over the fields of the structure
  using FieldIterator = Layout::const_iterator;

  /// \brief Reverse iterator over the fields of the structure
  using FieldReverseIterator = Layout::const_reverse_iterator;

private:
  // Layout of the structure
  Layout _layout;

  // Is the structure packed?
  bool _packed;

private:
  /// \brief Private constructor
  StructType(Layout layout, bool packed);

  /// \brief Private constructor
  explicit StructType(bool packed);

public:
  /// \brief Static constructor
  ///
  /// Always return a new instance
  static StructType* create(Context& ctx, Layout layout, bool packed);

  /// \brief Static constructor
  ///
  /// Always return a new instance
  static StructType* create(Context& ctx, bool packed);

  /// \brief Set the layout
  void set_layout(Layout layout);

  /// \brief Set the packed attribute
  void set_packed(bool packed);

  /// \brief Get the layout
  const Layout& layout() const { return this->_layout; }

  /// \brief Begin iterator over the fields of the structure
  FieldIterator field_begin() const { return this->_layout.cbegin(); }

  /// \brief End iterator over the fields of the structure
  FieldIterator field_end() const { return this->_layout.cend(); }

  /// \brief Begin reverse iterator over the fields of the structure
  FieldReverseIterator field_rbegin() const { return this->_layout.crbegin(); }

  /// \brief End reverse iterator over the fields of the structure
  FieldReverseIterator field_rend() const { return this->_layout.crend(); }

  /// \brief Is the structure empty?
  bool empty() const { return this->_layout.empty(); }

  /// \brief Get the number of fields
  std::size_t num_fields() const { return this->_layout.size(); }

  /// \brief Is the structure packed?
  bool packed() const { return this->_packed; }

  /// \brief Dump the type for debugging purpose
  ///
  /// Note: it might produce an infinite loop on recursive types
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Type* t) { return t->kind() == StructKind; }

}; // end class StructType

/// \brief Base class for sequential types, such as ArrayType and VectorType
class SequentialType : public AggregateType {
protected:
  // Element type
  Type* _element_type;

  // Number of elements
  ZNumber _num_elements;

protected:
  /// \brief Private constructor
  SequentialType(TypeKind kind, Type* element_type, ZNumber num_element);

public:
  /// \brief Get the element type
  Type* element_type() const { return this->_element_type; }

  /// \brief Get the number of elements
  ZNumber num_elements() const { return this->_num_elements; }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Type* t) {
    return t->kind() >= _BeginSequentialKind && t->kind() <= _EndSequentialKind;
  }

}; // end class SequentialType

/// \brief Array type
///
/// Represents the array type.
///
/// The layout in memory includes padding so that each element is properly
/// aligned. This is the equivalent of arrays in C.
class ArrayType final : public SequentialType {
private:
  /// \brief Private constructor
  ArrayType(Type* element_type, ZNumber num_element);

public:
  /// \brief Static constructor
  static ArrayType* get(Context& ctx,
                        Type* element_type,
                        const ZNumber& num_element);

  /// \brief Dump the type for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Type* t) { return t->kind() == ArrayKind; }

  // friends
  friend class ContextImpl;

}; // end class ArrayType

/// \brief Vector type
///
/// Represents the vector type.
///
/// This is similar to ArrayType, but can only hold scalars and does not add
/// padding between elements. This is mostly used to represent return types of C
/// function returning small structures.
class VectorType final : public SequentialType {
private:
  /// \brief Private constructor
  VectorType(ScalarType* element_type, ZNumber num_element);

public:
  /// \brief Static constructor
  static VectorType* get(Context& ctx,
                         ScalarType* element_type,
                         const ZNumber& num_element);

  /// \brief Get the element type
  ScalarType* element_type() const {
    return cast< ScalarType >(this->_element_type);
  }

  /// \brief Dump the type for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Type* t) { return t->kind() == VectorKind; }

  // friends
  friend class ContextImpl;

}; // end class VectorType

/// \brief Opaque type
///
/// Represents an opaque type. Mainly used to represent a forward declared
/// structure in C.
//
/// Opaque types might not be unique, so a pointer comparison might not work.
class OpaqueType final : public AggregateType {
private:
  /// \brief Private constructor
  OpaqueType();

public:
  /// \brief Static constructor
  ///
  /// Always return a new instance
  static OpaqueType* create(Context& ctx);

  /// \brief Get the libc FILE opaque type
  static OpaqueType* libc_file_type(Context& ctx);

  /// \brief Dump the type for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Type* t) { return t->kind() == OpaqueKind; }

  // friends
  friend class ContextImpl;

}; // end class OpaqueType

/// \brief Function type
///
/// Represents a function type, with its return type and parameter types.
class FunctionType final : public Type {
public:
  /// \brief Type for a list of parameter types
  using ParamTypes = std::vector< Type* >;

  /// \brief Iterator over the list of parameter types
  using ParamIterator = ParamTypes::const_iterator;

private:
  // Return type
  Type* _return_type;

  // List of parameter types
  ParamTypes _param_types;

  // Is it a variable argument function call?
  bool _is_var_arg;

private:
  /// \brief Private constructor
  FunctionType(Type* return_type,
               const ParamTypes& param_types,
               bool is_var_arg);

public:
  /// \brief Static constructor
  static FunctionType* get(Context& ctx,
                           Type* return_type,
                           const ParamTypes& param_types,
                           bool is_var_arg);

  /// \brief Get the returned type
  Type* return_type() const { return this->_return_type; }

  /// \brief Get the list of parameter types
  const ParamTypes& param_types() const { return this->_param_types; }

  /// \brief Get the i-th parameter type
  ar::Type* param_type(std::size_t i) const {
    ikos_assert_msg(i < this->_param_types.size(), "invalid index");
    return this->_param_types[i];
  }

  /// \brief Begin iterator over the list of parameter types
  ParamIterator param_begin() const { return this->_param_types.begin(); }

  /// \brief End iterator over the list of parameter types
  ParamIterator param_end() const { return this->_param_types.end(); }

  /// \brief Get the number of parameters
  std::size_t num_parameters() const { return this->_param_types.size(); }

  /// \brief Is it a variable argument (var-arg) function call?
  bool is_var_arg() const { return this->_is_var_arg; }

  /// \brief Dump the type for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Type* t) { return t->kind() == FunctionKind; }

  // friends
  friend class ContextImpl;

}; // end class FunctionType

} // end namespace ar
} // end namespace ikos
