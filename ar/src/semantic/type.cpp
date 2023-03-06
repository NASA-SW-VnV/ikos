/*******************************************************************************
 *
 * \file
 * \brief Implementation of types
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

#include <ikos/ar/semantic/context.hpp>
#include <ikos/ar/semantic/type.hpp>
#include <ikos/ar/support/assert.hpp>
#include <ikos/ar/support/cast.hpp>

#include "context_impl.hpp"

namespace ikos {
namespace ar {

// Type

Type::Type(TypeKind kind) : _kind(kind) {}

Type::~Type() = default;

bool Type::is_unsigned_integer() const {
  return isa< IntegerType >(this) && cast< IntegerType >(this)->is_unsigned();
}

bool Type::is_signed_integer() const {
  return isa< IntegerType >(this) && cast< IntegerType >(this)->is_signed();
}

bool Type::is_primitive() const {
  switch (this->_kind) {
    case IntegerKind:
      return true;
    case FloatKind:
      return true;
    case VectorKind: {
      Type* element = cast< VectorType >(this)->element_type();
      return element->is_integer() || element->is_float();
    }
    default:
      return false;
  }
}

uint64_t Type::primitive_bit_width() const {
  switch (this->_kind) {
    case IntegerKind:
      return cast< IntegerType >(this)->bit_width();
    case FloatKind:
      return cast< FloatType >(this)->bit_width();
    case VectorKind: {
      auto vector = cast< VectorType >(this);
      return vector->num_elements().to< uint64_t >() *
             vector->element_type()->primitive_bit_width();
    }
    default:
      return 0;
  }
}

ContextImpl& Type::ctx_impl(Context& ctx) {
  return *(ctx._impl);
}

// VoidType

VoidType::VoidType() : Type(VoidKind) {}

VoidType* VoidType::get(Context& ctx) {
  return ctx_impl(ctx).void_type();
}

void VoidType::dump(std::ostream& o) const {
  o << "void";
}

// ScalarType

ScalarType::ScalarType(TypeKind kind) : Type(kind) {}

// IntegerType

IntegerType::IntegerType(uint64_t bit_width, Signedness sign)
    : ScalarType(IntegerKind), _bit_width(bit_width), _sign(sign) {
  ikos_assert_msg(bit_width >= 1, "invalid bit width");
}

IntegerType* IntegerType::get(Context& ctx,
                              uint64_t bit_width,
                              Signedness sign) {
  if (bit_width == 1) {
    if (sign == Unsigned) {
      return ctx_impl(ctx).ui1_type();
    } else {
      return ctx_impl(ctx).si1_type();
    }
  } else if (bit_width == 8) {
    if (sign == Unsigned) {
      return ctx_impl(ctx).ui8_type();
    } else {
      return ctx_impl(ctx).si8_type();
    }
  } else if (bit_width == 16) {
    if (sign == Unsigned) {
      return ctx_impl(ctx).ui16_type();
    } else {
      return ctx_impl(ctx).si16_type();
    }
  } else if (bit_width == 32) {
    if (sign == Unsigned) {
      return ctx_impl(ctx).ui32_type();
    } else {
      return ctx_impl(ctx).si32_type();
    }
  } else if (bit_width == 64) {
    if (sign == Unsigned) {
      return ctx_impl(ctx).ui64_type();
    } else {
      return ctx_impl(ctx).si64_type();
    }
  } else {
    return ctx_impl(ctx).integer_type(bit_width, sign);
  }
}

IntegerType* IntegerType::si1(Context& ctx) {
  return ctx_impl(ctx).si1_type();
}

IntegerType* IntegerType::ui1(Context& ctx) {
  return ctx_impl(ctx).ui1_type();
}

IntegerType* IntegerType::si8(Context& ctx) {
  return ctx_impl(ctx).si8_type();
}

IntegerType* IntegerType::ui8(Context& ctx) {
  return ctx_impl(ctx).ui8_type();
}

IntegerType* IntegerType::si32(Context& ctx) {
  return ctx_impl(ctx).si32_type();
}

IntegerType* IntegerType::ui32(Context& ctx) {
  return ctx_impl(ctx).ui32_type();
}

IntegerType* IntegerType::si64(Context& ctx) {
  return ctx_impl(ctx).si64_type();
}

IntegerType* IntegerType::ui64(Context& ctx) {
  return ctx_impl(ctx).ui64_type();
}

IntegerType* IntegerType::size_type(Bundle* bundle) {
  return IntegerType::get(bundle->context(),
                          static_cast< uint64_t >(
                              bundle->data_layout().pointers.bit_width),
                          Unsigned);
}

IntegerType* IntegerType::ssize_type(Bundle* bundle) {
  return IntegerType::get(bundle->context(),
                          static_cast< uint64_t >(
                              bundle->data_layout().pointers.bit_width),
                          Signed);
}

MachineInt IntegerType::min_value() const {
  return MachineInt::min(this->_bit_width, this->_sign);
}

MachineInt IntegerType::max_value() const {
  return MachineInt::max(this->_bit_width, this->_sign);
}

void IntegerType::dump(std::ostream& o) const {
  if (this->is_unsigned()) {
    o << "ui";
  } else {
    o << "si";
  }
  o << this->_bit_width;
}

// FloatType

FloatType::FloatType(uint64_t bit_width, FloatSemantic float_sem)
    : ScalarType(FloatKind), _bit_width(bit_width), _float_sem(float_sem) {
  ikos_assert_msg(bit_width >= 1, "invalid bit width");
}

FloatType* FloatType::get(Context& ctx, FloatSemantic float_sem) {
  switch (float_sem) {
    case Half:
      return ctx_impl(ctx).half_type();
    case Float:
      return ctx_impl(ctx).float_type();
    case Double:
      return ctx_impl(ctx).double_type();
    case X86_FP80:
      return ctx_impl(ctx).x86_fp80_type();
    case FP128:
      return ctx_impl(ctx).fp128_type();
    case PPC_FP128:
      return ctx_impl(ctx).ppc_fp128_type();
    default:
      ikos_unreachable("unknown float semantic");
  }
}

void FloatType::dump(std::ostream& o) const {
  switch (this->_float_sem) {
    case Half:
      o << "half";
      break;
    case Float:
      o << "float";
      break;
    case Double:
      o << "double";
      break;
    case X86_FP80:
      o << "x86_fp80";
      break;
    case FP128:
      o << "fp128";
      break;
    case PPC_FP128:
      o << "ppc_fp128";
      break;
    default:
      ikos_unreachable("unknown float semantic");
  }
}

// PointerType

PointerType::PointerType(Type* pointee)
    : ScalarType(PointerKind), _pointee(pointee) {
  ikos_assert_msg(pointee, "pointee type is null");
}

PointerType* PointerType::get(Context& ctx, Type* pointee) {
  return ctx_impl(ctx).pointer_type(pointee);
}

void PointerType::dump(std::ostream& o) const {
  this->_pointee->dump(o);
  o << "*";
}

// AggregateType

AggregateType::AggregateType(TypeKind kind) : Type(kind) {}

// StructType

StructType::StructType(Layout layout, bool packed)
    : AggregateType(StructKind), _layout(std::move(layout)), _packed(packed) {
  ikos_assert_msg(std::is_sorted(this->_layout.begin(),
                                 this->_layout.end(),
                                 [](const Field& a, const Field& b) {
                                   return a.offset < b.offset;
                                 }),
                  "layout is not sorted by offset");
}

StructType::StructType(bool packed)
    : AggregateType(StructKind), _packed(packed) {}

StructType* StructType::create(Context& ctx, Layout layout, bool packed) {
  auto t = new StructType(std::move(layout), packed);
  ctx_impl(ctx).add_type(std::unique_ptr< Type >(t));
  return t;
}

StructType* StructType::create(Context& ctx, bool packed) {
  auto type = std::unique_ptr< StructType >(new StructType(packed));
  auto ptr = type.get();
  ctx_impl(ctx).add_type(std::move(type));
  return ptr;
}

void StructType::set_layout(Layout layout) {
  ikos_assert_msg(std::is_sorted(layout.begin(),
                                 layout.end(),
                                 [](const Field& a, const Field& b) {
                                   return a.offset < b.offset;
                                 }),
                  "layout is not sorted by offset");
  this->_layout = std::move(layout);
}

void StructType::set_packed(bool packed) {
  this->_packed = packed;
}

void StructType::dump(std::ostream& o) const {
  if (this->_packed) {
    o << "<";
  }
  o << "{";
  for (auto it = this->_layout.cbegin(), et = this->_layout.cend(); it != et;) {
    o << it->offset << ": ";
    it->type->dump(o);
    ++it;
    if (it != et) {
      o << ", ";
    }
  }
  o << "}";
  if (this->_packed) {
    o << ">";
  }
}

// SequentialType

SequentialType::SequentialType(TypeKind kind,
                               Type* element_type,
                               ZNumber num_element)
    : AggregateType(kind),
      _element_type(element_type),
      _num_elements(std::move(num_element)) {
  ikos_assert_msg(this->_element_type, "element type is null");
  ikos_assert_msg(this->_num_elements >= 0, "invalid number of elements");
}

// ArrayType

ArrayType::ArrayType(Type* element_type, ZNumber num_element)
    : SequentialType(ArrayKind, element_type, std::move(num_element)) {}

ArrayType* ArrayType::get(Context& ctx,
                          Type* element_type,
                          const ZNumber& num_element) {
  return ctx_impl(ctx).array_type(element_type, num_element);
}

void ArrayType::dump(std::ostream& o) const {
  o << "[" << this->num_elements() << " x ";
  this->element_type()->dump(o);
  o << "]";
}

// VectorType

VectorType::VectorType(ScalarType* element_type, ZNumber num_element)
    : SequentialType(VectorKind, element_type, std::move(num_element)) {}

VectorType* VectorType::get(Context& ctx,
                            ScalarType* element_type,
                            const ZNumber& num_element) {
  return ctx_impl(ctx).vector_type(element_type, num_element);
}

void VectorType::dump(std::ostream& o) const {
  o << "<" << this->num_elements() << " x ";
  this->element_type()->dump(o);
  o << ">";
}

// OpaqueType

OpaqueType::OpaqueType() : AggregateType(OpaqueKind) {}

OpaqueType* OpaqueType::create(Context& ctx) {
  auto type = std::unique_ptr< OpaqueType >(new OpaqueType());
  auto ptr = type.get();
  ctx_impl(ctx).add_type(std::move(type));
  return ptr;
}

OpaqueType* OpaqueType::libc_file_type(Context& ctx) {
  return ctx_impl(ctx).libc_file_type();
}

void OpaqueType::dump(std::ostream& o) const {
  o << "opaque";
}

// FunctionType

FunctionType::FunctionType(Type* return_type,
                           const ParamTypes& param_types,
                           bool is_var_arg)
    : Type(FunctionKind),
      _return_type(return_type),
      _param_types(param_types),
      _is_var_arg(is_var_arg) {
  ikos_assert_msg(return_type, "return type is null");
  ikos_assert_msg(std::all_of(param_types.begin(),
                              param_types.end(),
                              [](Type* p) { return p; }),
                  "one parameter type is null");
}

FunctionType* FunctionType::get(Context& ctx,
                                Type* return_type,
                                const ParamTypes& param_types,
                                bool is_var_arg) {
  return ctx_impl(ctx).function_type(return_type, param_types, is_var_arg);
}

void FunctionType::dump(std::ostream& o) const {
  this->_return_type->dump(o);
  o << " (";
  for (auto it = this->_param_types.cbegin(), et = this->_param_types.cend();
       it != et;) {
    (*it)->dump(o);
    ++it;
    if (it != et) {
      o << ", ";
    }
  }
  if (this->_is_var_arg) {
    if (!this->_param_types.empty()) {
      o << ", ";
    }
    o << "...";
  }
  o << ")";
}

} // end namespace ar
} // end namespace ikos
