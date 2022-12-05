/*******************************************************************************
 *
 * \file
 * \brief Translate LLVM types and Debug Information into AR types
 *
 * Author: Maxime Arthaud
 *         Nija Shi
 *         Arnaud Venet
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

#include <ikos/core/support/assert.hpp>

#include <ikos/ar/support/cast.hpp>

#include <ikos/frontend/llvm/import/exception.hpp>

#include "type.hpp"

#ifdef __GLIBCXX__
// Fix for libstdc++, see https://bugs.llvm.org/show_bug.cgi?id=35755

namespace std {

template <>
struct iterator_traits< llvm::DITypeRefArray::iterator > {
  using difference_type = std::ptrdiff_t;
  using value_type = llvm::DIType*;
  using pointer = void;
  using reference = llvm::DIType*;
  using iterator_category = std::input_iterator_tag;
};

template <>
struct iterator_traits< llvm::TypedMDOperandIterator< llvm::DINode > > {
  using difference_type = std::ptrdiff_t;
  using value_type = llvm::DINode*;
  using pointer = void;
  using reference = llvm::DINode*;
  using iterator_category = std::input_iterator_tag;
};

} // end namespace std

#endif

namespace ikos {
namespace frontend {
namespace import {

namespace dwarf = llvm::dwarf;

// TypeWithSignImporter

TypeWithSignImporter::TypeWithSignImporter(ImportContext& ctx)
    : _context(ctx.ar_context),
      _llvm_data_layout(ctx.llvm_data_layout),
      _ar_data_layout(ctx.ar_data_layout),
      _translation_depth(0) {}

TypeWithSignImporter::~TypeWithSignImporter() = default;

ar::Type* TypeWithSignImporter::translate_type(llvm::Type* type,
                                               ar::Signedness preferred) {
  auto it = this->_map.find({type, preferred});

  if (it != this->_map.end()) {
    return it->second;
  }

  if (type->isVoidTy()) {
    return this->translate_void_type(type, preferred);
  } else if (type->isIntegerTy()) {
    return this->translate_integer_type(type, preferred);
  } else if (type->isFloatingPointTy()) {
    return this->translate_floating_point_type(type, preferred);
  } else if (type->isPointerTy()) {
    return this->translate_pointer_type(type, preferred);
  } else if (type->isArrayTy()) {
    return this->translate_array_type(type, preferred);
  } else if (type->isVectorTy()) {
    return this->translate_vector_type(type, preferred);
  } else if (type->isStructTy()) {
    return this->translate_struct_type(type, preferred);
  } else if (type->isFunctionTy()) {
    return this->translate_function_type(type, preferred);
  } else {
    throw ImportError("unsupported llvm type");
  }
}

void TypeWithSignImporter::store_translation(llvm::Type* type,
                                             ar::Signedness preferred,
                                             ar::Type* ar_type) {
  ikos_assert_msg(type, "type is null");
  ikos_assert_msg(ar_type, "result is null");
  this->_map.try_emplace({type, preferred}, ar_type);
}

void TypeWithSignImporter::sanity_check_size(llvm::Type* llvm_type,
                                             ar::Type* ar_type) {
  if (this->_translation_depth > 0) {
    // Disable checks, some types are not complete yet.
    return;
  }

  check_import(this->_llvm_data_layout.getTypeSizeInBits(llvm_type)
                       .getFixedSize() >=
                   this->_ar_data_layout.size_in_bits(ar_type),
               "llvm type size in bits is smaller than ar type size");
  check_import(this->_llvm_data_layout.getTypeAllocSize(llvm_type)
                       .getFixedSize() ==
                   this->_ar_data_layout.alloc_size_in_bytes(ar_type),
               "llvm type and ar type alloc size are different");
}

ar::VoidType* TypeWithSignImporter::translate_void_type(
    llvm::Type* type, ar::Signedness preferred) {
  ar::VoidType* ar_type = ar::VoidType::get(this->_context);
  this->store_translation(type, preferred, ar_type);
  return ar_type;
}

ar::IntegerType* TypeWithSignImporter::translate_integer_type(
    llvm::Type* type, ar::Signedness preferred) {
  auto int_type = llvm::cast< llvm::IntegerType >(type);
  ar::IntegerType* ar_type =
      ar::IntegerType::get(this->_context, int_type->getBitWidth(), preferred);
  this->store_translation(type, preferred, ar_type);
  return ar_type;
}

ar::FloatType* TypeWithSignImporter::translate_floating_point_type(
    llvm::Type* type, ar::Signedness preferred) {
  ar::FloatType* ar_type = nullptr;

  if (type->isHalfTy()) {
    ar_type = ar::FloatType::get(this->_context, ar::Half);
  } else if (type->isFloatTy()) {
    ar_type = ar::FloatType::get(this->_context, ar::Float);
  } else if (type->isDoubleTy()) {
    ar_type = ar::FloatType::get(this->_context, ar::Double);
  } else if (type->isX86_FP80Ty()) {
    ar_type = ar::FloatType::get(this->_context, ar::X86_FP80);
  } else if (type->isFP128Ty()) {
    ar_type = ar::FloatType::get(this->_context, ar::FP128);
  } else if (type->isPPC_FP128Ty()) {
    ar_type = ar::FloatType::get(this->_context, ar::PPC_FP128);
  } else {
    throw ImportError("unsupported llvm floating point type");
  }

  this->store_translation(type, preferred, ar_type);
  return ar_type;
}

ar::PointerType* TypeWithSignImporter::translate_pointer_type(
    llvm::Type* type, ar::Signedness preferred) {
  auto ptr_type = llvm::cast< llvm::PointerType >(type);
  ar::Type* ar_pointee_type =
      this->translate_type(ptr_type->getPointerElementType(), preferred);
  ar::PointerType* ar_type =
      ar::PointerType::get(this->_context, ar_pointee_type);
  this->store_translation(type, preferred, ar_type);
  return ar_type;
}

ar::ArrayType* TypeWithSignImporter::translate_array_type(
    llvm::Type* type, ar::Signedness preferred) {
  auto array_type = llvm::cast< llvm::ArrayType >(type);
  ar::Type* ar_element_type =
      this->translate_type(array_type->getElementType(), preferred);
  ar::ArrayType* ar_type =
      ar::ArrayType::get(this->_context,
                         ar_element_type,
                         ar::ZNumber(array_type->getNumElements()));
  this->store_translation(type, preferred, ar_type);
  this->sanity_check_size(type, ar_type);
  return ar_type;
}

ar::VectorType* TypeWithSignImporter::translate_vector_type(
    llvm::Type* type, ar::Signedness preferred) {
  auto vector_type = llvm::cast< llvm::VectorType >(type);
  ar::Type* ar_element_type =
      this->translate_type(vector_type->getElementType(), preferred);
  ar::VectorType* ar_type =
      ar::VectorType::get(this->_context,
                          ar::cast< ar::ScalarType >(ar_element_type),
                          ar::ZNumber(
                              vector_type->getElementCount().getFixedValue()));
  this->store_translation(type, preferred, ar_type);
  this->sanity_check_size(type, ar_type);
  return ar_type;
}

ar::Type* TypeWithSignImporter::translate_struct_type(
    llvm::Type* type, ar::Signedness preferred) {
  auto struct_type = llvm::cast< llvm::StructType >(type);

  if (struct_type->isOpaque()) {
    ar::OpaqueType* ar_type = ar::OpaqueType::create(this->_context);
    this->store_translation(type, ar::Signed, ar_type);
    this->store_translation(type, ar::Unsigned, ar_type);
    return ar_type;
  }

  // Structures can be recursive, so create it now, with an empty layout
  ar::StructType* ar_type =
      ar::StructType::create(this->_context, struct_type->isPacked());
  this->store_translation(type, preferred, ar_type);
  this->_translation_depth++; // recursive type, not complete yet

  // Create the layout
  const llvm::StructLayout* struct_layout =
      this->_llvm_data_layout.getStructLayout(struct_type);
  ar::StructType::Layout ar_layout;
  ar_layout.reserve(struct_type->getNumElements());

  for (unsigned i = 0; i < struct_type->getNumElements(); i++) {
    llvm::Type* element_type = struct_type->getElementType(i);
    uint64_t element_offset = struct_layout->getElementOffset(i);
    ar::Type* ar_element_type = this->translate_type(element_type, preferred);
    ar_layout.push_back({ar::ZNumber(element_offset), ar_element_type});
  }

  ar_type->set_layout(ar_layout);
  this->_translation_depth--;
  this->sanity_check_size(struct_type, ar_type);
  return ar_type;
}

ar::FunctionType* TypeWithSignImporter::translate_function_type(
    llvm::Type* type, ar::Signedness preferred) {
  auto fun_type = llvm::cast< llvm::FunctionType >(type);

  // Return type
  ar::Type* ar_return_type =
      this->translate_type(fun_type->getReturnType(), preferred);

  // Parameters
  ar::FunctionType::ParamTypes ar_params;
  for (llvm::Type* param : fun_type->params()) {
    ar::Type* ar_param = this->translate_type(param, preferred);
    ar_params.push_back(ar_param);
  }

  ar::FunctionType* ar_type = ar::FunctionType::get(this->_context,
                                                    ar_return_type,
                                                    ar_params,
                                                    fun_type->isVarArg());
  this->store_translation(type, preferred, ar_type);
  return ar_type;
}

// TypeWithDebugInfoImporter

TypeWithDebugInfoImporter::TypeWithDebugInfoImporter(
    ar::Context& context,
    const llvm::DataLayout& llvm_data_layout,
    const ar::DataLayout& ar_data_layout,
    bool is_c,
    bool is_cpp,
    unsigned translation_depth,
    TypeWithSignImporter& type_sign_imp,
    const TypeWithDebugInfoImporter* parent)
    : _context(context),
      _llvm_data_layout(llvm_data_layout),
      _ar_data_layout(ar_data_layout),
      _is_c(is_c),
      _is_cpp(is_cpp),
      _translation_depth(translation_depth),
      _type_sign_imp(type_sign_imp),
      _parent(parent) {}

TypeWithDebugInfoImporter::TypeWithDebugInfoImporter(
    ImportContext& ctx, TypeWithSignImporter& type_sign_imp)
    : _context(ctx.ar_context),
      _llvm_data_layout(ctx.llvm_data_layout),
      _ar_data_layout(ctx.ar_data_layout),
      _translation_depth(0),
      _type_sign_imp(type_sign_imp),
      _parent(nullptr) {
  // Get all source languages
  llvm::SmallSet< llvm::dwarf::SourceLanguage, 2 > languages;

  for (auto it = ctx.module.debug_compile_units_begin(),
            et = ctx.module.debug_compile_units_end();
       it != et;
       ++it) {
    unsigned lang = it->getSourceLanguage();
    languages.insert(static_cast< dwarf::SourceLanguage >(lang));
  }

  this->_is_c = languages.count(dwarf::DW_LANG_C) != 0 ||
                languages.count(dwarf::DW_LANG_C89) != 0 ||
                languages.count(dwarf::DW_LANG_C99) != 0 ||
                languages.count(dwarf::DW_LANG_C11) != 0;

  this->_is_cpp = languages.count(dwarf::DW_LANG_C_plus_plus) != 0 ||
                  languages.count(dwarf::DW_LANG_C_plus_plus_03) != 0 ||
                  languages.count(dwarf::DW_LANG_C_plus_plus_11) != 0 ||
                  languages.count(dwarf::DW_LANG_C_plus_plus_14) != 0;
}

TypeWithDebugInfoImporter::~TypeWithDebugInfoImporter() = default;

ar::Type* TypeWithDebugInfoImporter::translate_type(llvm::Type* type,
                                                    llvm::DIType* di_type) {
  // Check if it is already translated in a parent
  const TypeWithDebugInfoImporter* parent = this->_parent;

  while (parent != nullptr) {
    auto it = parent->_map.find({type, di_type});

    if (it != parent->_map.end()) {
      return it->second;
    }

    parent = parent->_parent;
  }

  // Check if it is already translated here
  auto it = this->_map.find({type, di_type});

  if (it != this->_map.end()) {
    return it->second;
  }

  if (di_type == nullptr) {
    return this->translate_null_di_type(type);
  } else if (di_type->isForwardDecl()) {
    return this->translate_forward_decl_di_type(di_type, type);
  } else if (auto basic_type = llvm::dyn_cast< llvm::DIBasicType >(di_type)) {
    return this->translate_basic_di_type(basic_type, type);
  } else if (auto derived_type =
                 llvm::dyn_cast< llvm::DIDerivedType >(di_type)) {
    return this->translate_derived_di_type(derived_type, type);
  } else if (auto composite_type =
                 llvm::dyn_cast< llvm::DICompositeType >(di_type)) {
    return this->translate_composite_di_type(composite_type, type);
  } else if (auto subroutine_type =
                 llvm::dyn_cast< llvm::DISubroutineType >(di_type)) {
    return this->translate_subroutine_di_type(subroutine_type, type);
  } else {
    throw ImportError("unsupported llvm DIType");
  }
}

TypeWithDebugInfoImporter TypeWithDebugInfoImporter::fork() const {
  return TypeWithDebugInfoImporter(this->_context,
                                   this->_llvm_data_layout,
                                   this->_ar_data_layout,
                                   this->_is_c,
                                   this->_is_cpp,
                                   this->_translation_depth + 1,
                                   this->_type_sign_imp,
                                   this);
}

void TypeWithDebugInfoImporter::join(const TypeWithDebugInfoImporter& imp) {
  for (const auto& e : imp._map) {
    this->_map.try_emplace(e.first, e.second);
  }
}

void TypeWithDebugInfoImporter::store_translation(llvm::Type* type,
                                                  llvm::DIType* di_type,
                                                  ar::Type* ar_type) {
  ikos_assert_msg(type, "type is null");
  ikos_assert_msg(ar_type, "result is null");
  this->_map.try_emplace({type, di_type}, ar_type);
}

void TypeWithDebugInfoImporter::sanity_check_size(llvm::Type* llvm_type,
                                                  ar::Type* ar_type) {
  if (this->_translation_depth > 0) {
    // Disable checks, some types are not complete yet.
    return;
  }

  check_import(this->_llvm_data_layout.getTypeSizeInBits(llvm_type)
                       .getFixedSize() >=
                   this->_ar_data_layout.size_in_bits(ar_type),
               "llvm type size in bits is smaller than ar type size");
  check_import(this->_llvm_data_layout.getTypeAllocSize(llvm_type)
                       .getFixedSize() ==
                   this->_ar_data_layout.alloc_size_in_bytes(ar_type),
               "llvm type and ar type alloc size are different");
}

ar::Type* TypeWithDebugInfoImporter::translate_null_di_type(llvm::Type* type) {
  ar::Type* ar_type = nullptr;

  if (type->isVoidTy()) {
    ar_type = ar::VoidType::get(this->_context);
  } else if (type->isIntegerTy(8)) {
    ar_type = ar::IntegerType::get(this->_context, 8, ar::Signed);
  } else {
    throw TypeDebugInfoMismatch(
        "unexpected llvm type with no debug information");
  }

  this->store_translation(type, nullptr, ar_type);
  return ar_type;
}

ar::Type* TypeWithDebugInfoImporter::translate_forward_decl_di_type(
    llvm::DIType* di_type, llvm::Type* type) {
  // In this case, guess the type using llvm::Type*
  ar::Type* ar_type = this->_type_sign_imp.translate_type(type, ar::Signed);
  this->store_translation(type, di_type, ar_type);
  return ar_type;
}

ar::Type* TypeWithDebugInfoImporter::translate_basic_di_type(
    llvm::DIBasicType* di_type, llvm::Type* type) {
  ar::Type* ar_type = nullptr;

  if (di_type->getEncoding() != 0) {
    auto encoding = static_cast< dwarf::TypeKind >(di_type->getEncoding());

    switch (encoding) {
      case dwarf::DW_ATE_signed:
      case dwarf::DW_ATE_unsigned:
      case dwarf::DW_ATE_signed_char:
      case dwarf::DW_ATE_unsigned_char: {
        check_match(type->isIntegerTy(),
                    "llvm DIBasicType with integer encoding, but llvm type is "
                    "not an integer type");
        auto int_type = llvm::cast< llvm::IntegerType >(type);
        check_match(di_type->getSizeInBits() == int_type->getBitWidth(),
                    "llvm DIBasicType with integer encoding and llvm integer "
                    "type have a different bit-width");

        ar::Signedness sign = (encoding == dwarf::DW_ATE_unsigned ||
                               encoding == dwarf::DW_ATE_unsigned_char)
                                  ? ar::Unsigned
                                  : ar::Signed;
        ar_type =
            ar::IntegerType::get(this->_context, int_type->getBitWidth(), sign);
      } break;
      case dwarf::DW_ATE_boolean: {
        check_match(type->isIntegerTy(),
                    "llvm DIBasicType with boolean encoding, but llvm type is "
                    "not an integer type");
        auto int_type = llvm::cast< llvm::IntegerType >(type);
        check_match(di_type->getSizeInBits() == int_type->getBitWidth() ||
                        (di_type->getSizeInBits() == 8 &&
                         int_type->getBitWidth() == 1),
                    "llvm DIBasicType with boolean encoding and llvm integer "
                    "type have a different bit-width");
        ar_type = ar::IntegerType::get(this->_context,
                                       int_type->getBitWidth(),
                                       ar::Unsigned);
      } break;
      case dwarf::DW_ATE_UTF: {
        check_match(type->isIntegerTy(),
                    "llvm DIBasicType with UTF encoding, but llvm type is "
                    "not an integer type");
        auto int_type = llvm::cast< llvm::IntegerType >(type);
        check_match(di_type->getSizeInBits() == int_type->getBitWidth(),
                    "llvm DIBasicType with UTF encoding and llvm integer "
                    "type have a different bit-width");
        ar_type = ar::IntegerType::get(this->_context,
                                       int_type->getBitWidth(),
                                       ar::Unsigned);
      } break;
      case dwarf::DW_ATE_float: {
        check_match(type->isFloatingPointTy(),
                    "llvm DIBasicType with float encoding, but llvm type is "
                    "not a floating point type");
        check_match(di_type->getSizeInBits() ==
                            type->getPrimitiveSizeInBits() ||
                        (di_type->getSizeInBits() == 128 &&
                         type->isX86_FP80Ty()),
                    "llvm DIBasicType with float encoding and llvm floating "
                    "point type have a different bit-width");

        if (type->isHalfTy()) {
          ar_type = ar::FloatType::get(this->_context, ar::Half);
        } else if (type->isFloatTy()) {
          ar_type = ar::FloatType::get(this->_context, ar::Float);
        } else if (type->isDoubleTy()) {
          ar_type = ar::FloatType::get(this->_context, ar::Double);
        } else if (type->isX86_FP80Ty()) {
          ar_type = ar::FloatType::get(this->_context, ar::X86_FP80);
        } else if (type->isFP128Ty()) {
          ar_type = ar::FloatType::get(this->_context, ar::FP128);
        } else if (type->isPPC_FP128Ty()) {
          ar_type = ar::FloatType::get(this->_context, ar::PPC_FP128);
        } else {
          throw ImportError("unsupported llvm floating point type");
        }
      } break;
      case dwarf::DW_ATE_complex_float: {
        check_match(type->isStructTy(),
                    "llvm DIBasicType with complex encoding, but llvm type is "
                    "not a structure type");
        auto struct_type = llvm::cast< llvm::StructType >(type);
        check_match(struct_type->getNumElements() == 2 &&
                        struct_type->getElementType(0) ==
                            struct_type->getElementType(1) &&
                        struct_type->getElementType(0)->isFloatingPointTy(),
                    "llvm DIBasicType with complex encoding, but llvm "
                    "structure type does not contain 2 floating points");
        check_match(di_type->getSizeInBits() ==
                        2 * struct_type->getElementType(0)
                                ->getPrimitiveSizeInBits(),
                    "llvm DIBasicType with complex encoding and llvm structure "
                    "type have a different bit-width");
        ar_type = this->_type_sign_imp.translate_type(type, ar::Signed);
      } break;
      default: {
        throw ImportError("unsupported dwarf encoding for llvm DIBasicType");
      }
    }
  } else if (di_type->getTag() != 0) {
    auto tag = static_cast< dwarf::Tag >(di_type->getTag());

    if (tag == dwarf::DW_TAG_unspecified_type &&
        di_type->getName() == "decltype(nullptr)") {
      check_match(type->isPointerTy() && llvm::cast< llvm::PointerType >(type)
                                             ->getPointerElementType()
                                             ->isIntegerTy(8),
                  "unexpected llvm type for llvm DIBasicType with name "
                  "'decltype(nullptr)'");
      ar_type = ar::PointerType::get(this->_context,
                                     ar::IntegerType::si8(this->_context));
    } else {
      throw ImportError("unsupported dwarf tag for llvm DIBasicType");
    }
  } else {
    throw ImportError("unexpected llvm DIBasicType without tag and encoding");
  }

  this->store_translation(type, di_type, ar_type);
  return ar_type;
}

ar::Type* TypeWithDebugInfoImporter::translate_derived_di_type(
    llvm::DIDerivedType* di_type, llvm::Type* type) {
  if (di_type->getTag() == dwarf::DW_TAG_typedef ||
      di_type->getTag() == dwarf::DW_TAG_const_type ||
      di_type->getTag() == dwarf::DW_TAG_volatile_type ||
      di_type->getTag() == dwarf::DW_TAG_restrict_type ||
      di_type->getTag() == dwarf::DW_TAG_atomic_type ||
      di_type->getTag() == dwarf::DW_TAG_ptr_to_member_type) {
    return this->translate_qualified_di_type(di_type, type);
  } else if (di_type->getTag() == dwarf::DW_TAG_pointer_type) {
    return this->translate_pointer_di_type(di_type, type);
  } else if (di_type->getTag() == dwarf::DW_TAG_reference_type ||
             di_type->getTag() == dwarf::DW_TAG_rvalue_reference_type) {
    return this->translate_reference_di_type(di_type, type);
  } else {
    throw ImportError("unsupported dwarf tag for llvm DIDerivedType");
  }
}

ar::Type* TypeWithDebugInfoImporter::translate_qualified_di_type(
    llvm::DIDerivedType* di_type, llvm::Type* type) {
  auto qualified_type =
      llvm::cast_or_null< llvm::DIType >(di_type->getRawBaseType());

  ar::Type* ar_type = this->translate_type(type, qualified_type);
  this->store_translation(type, di_type, ar_type);
  return ar_type;
}

ar::PointerType* TypeWithDebugInfoImporter::translate_pointer_di_type(
    llvm::DIDerivedType* di_type, llvm::Type* type) {
  check_match(type->isPointerTy(),
              "llvm DIDerivedType with pointer tag, but llvm type is not a "
              "pointer type");
  check_match(di_type->getSizeInBits() ==
                  this->_llvm_data_layout.getPointerSizeInBits(),
              "llvm DIDerivedType with pointer tag and llvm pointer type have "
              "a different bit-width");
  auto ptr_type = llvm::cast< llvm::PointerType >(type);
  llvm::Type* pointee_type = ptr_type->getPointerElementType();
  auto di_pointee_type =
      llvm::cast_or_null< llvm::DIType >(di_type->getRawBaseType());

  ar::Type* ar_pointee_type =
      this->translate_type(pointee_type, di_pointee_type);
  ar::PointerType* ar_type =
      ar::PointerType::get(this->_context, ar_pointee_type);
  this->store_translation(type, di_type, ar_type);
  return ar_type;
}

ar::PointerType* TypeWithDebugInfoImporter::translate_reference_di_type(
    llvm::DIDerivedType* di_type, llvm::Type* type) {
  check_match(type->isPointerTy(),
              "llvm DIDerivedType with reference tag, but llvm type is not a "
              "pointer type");
  check_match(di_type->getSizeInBits() == 0 ||
                  di_type->getSizeInBits() ==
                      this->_llvm_data_layout.getPointerSizeInBits(),
              "llvm DIDerivedType with reference tag and llvm pointer type "
              "have a different bit-width");
  auto ptr_type = llvm::cast< llvm::PointerType >(type);
  llvm::Type* pointee_type = ptr_type->getPointerElementType();
  auto di_referred_type =
      llvm::cast_or_null< llvm::DIType >(di_type->getRawBaseType());

  ar::Type* ar_pointee_type =
      this->translate_type(pointee_type, di_referred_type);
  ar::PointerType* ar_type =
      ar::PointerType::get(this->_context, ar_pointee_type);
  this->store_translation(type, di_type, ar_type);
  return ar_type;
}

ar::Type* TypeWithDebugInfoImporter::translate_composite_di_type(
    llvm::DICompositeType* di_type, llvm::Type* type) {
  auto tag = static_cast< dwarf::Tag >(di_type->getTag());

  if (tag == dwarf::DW_TAG_array_type) {
    return this->translate_array_di_type(di_type, type);
  } else if (tag == dwarf::DW_TAG_structure_type ||
             tag == dwarf::DW_TAG_class_type) {
    return this->translate_struct_di_type(di_type, type);
  } else if (tag == dwarf::DW_TAG_union_type) {
    return this->translate_union_di_type(di_type, type);
  } else if (tag == dwarf::DW_TAG_enumeration_type) {
    return this->translate_enum_di_type(di_type, type);
  } else {
    throw ImportError("unsupported dwarf tag for llvm DICompositeType");
  }
}

ar::Type* TypeWithDebugInfoImporter::translate_array_di_type(
    llvm::DICompositeType* di_type, llvm::Type* type) {
  // Check that the elements of di_type and the llvm::Type* match,
  // and get the final element Type
  check_import(di_type->getRawElements() != nullptr,
               "unexpected null pointer for elements in llvm DICompositeType");
  llvm::DINodeArray di_elements = di_type->getElements();
  llvm::SmallVector< llvm::Type*, 3 > llvm_elements;
  llvm::Type* current_element = type;

  // True if the previous element has count = -1
  bool prev_no_count = false;

  for (auto it = di_elements.begin(), et = di_elements.end(); it != et; ++it) {
    llvm::DINode* di_element = *it;
    check_import(llvm::isa< llvm::DISubrange >(di_element),
                 "unsupported element in llvm DICompositeType with array tag");
    auto subrange = llvm::cast< llvm::DISubrange >(di_element);
    auto count = subrange->getCount();

    if (count.is< llvm::ConstantInt* >() != 0 &&
        !count.get< llvm::ConstantInt* >()->isMinusOne()) {
      auto count_int = count.get< llvm::ConstantInt* >();
      check_import(!count_int->isNegative(),
                   "unexpected negative count for llvm DICompositeType with "
                   "array tag");

      if (current_element->isArrayTy()) {
        auto array_type = llvm::cast< llvm::ArrayType >(current_element);
        check_match(array_type->getNumElements() == count_int->getZExtValue(),
                    "llvm DICompositeType with array tag and llvm array type "
                    "have a different number of elements");

        llvm_elements.push_back(array_type);
        current_element = array_type->getElementType();
      } else if (current_element->isVectorTy()) {
        auto vector_type = llvm::cast< llvm::VectorType >(current_element);
        check_match(vector_type->getElementCount().getFixedValue() ==
                        count_int->getZExtValue(),
                    "llvm DICompositeType with array tag and llvm vector type "
                    "have a different number of elements");

        llvm_elements.push_back(vector_type);
        current_element = vector_type->getElementType();
      } else if (current_element->isStructTy()) {
        auto struct_type = llvm::cast< llvm::StructType >(current_element);
        check_match(!struct_type->isOpaque(),
                    "llvm DICompositeType with array tag, but llvm structure "
                    "type is opaque");
        check_match(struct_type->isPacked(),
                    "llvm DICompositeType with array tag, but llvm structure "
                    "type is not packed");
        check_match(struct_type->getNumElements() == count_int->getZExtValue(),
                    "llvm DICompositeType with array tag and llvm structure "
                    "type have a different number of elements");
        check_match(!count_int->isZero(),
                    "llvm DICompositeType with array tag, but llvm structure "
                    "type is empty");
        check_match(std::next(it) == et,
                    "llvm DICompositeType with array tag, but llvm structure "
                    "type is not the last element");

        llvm_elements.push_back(struct_type);
        current_element = struct_type->getElementType(0);
      } else {
        throw TypeDebugInfoMismatch(
            "llvm DICompositeType with array tag and positive count, but llvm "
            "type is not an array, vector or structure type");
      }
      prev_no_count = false;
    } else {
      if (prev_no_count) {
        // Type T[][] is flattened into T*, so skip this element
        continue;
      } else if (current_element->isPointerTy()) {
        auto ptr_type = llvm::cast< llvm::PointerType >(current_element);

        llvm_elements.push_back(ptr_type);
        current_element = ptr_type->getPointerElementType();
        prev_no_count = true;
      } else if (current_element->isArrayTy()) {
        auto array_type = llvm::cast< llvm::ArrayType >(current_element);
        check_match(array_type->getNumElements() == 0,
                    "llvm DICompositeType with array tag and count -1, but "
                    "llvm array type is not empty");

        llvm_elements.push_back(array_type);
        current_element = array_type->getElementType();
        prev_no_count = false;
      } else {
        throw TypeDebugInfoMismatch(
            "llvm DICompositeType with array tag and count -1, but llvm type "
            "is not a pointer or array type");
      }
    }
  }

  llvm::Type* final_element = current_element;
  auto di_final_element =
      llvm::cast_or_null< llvm::DIType >(di_type->getRawBaseType());

  // Build the ar::Type
  ar::Type* ar_type = this->translate_type(final_element, di_final_element);

  for (auto it = llvm_elements.rbegin(), et = llvm_elements.rend(); it != et;
       ++it) {
    if (auto array_type = llvm::dyn_cast< llvm::ArrayType >(*it)) {
      ar_type = ar::ArrayType::get(this->_context,
                                   ar_type,
                                   ar::ZNumber(array_type->getNumElements()));
    } else if (auto vector_type = llvm::dyn_cast< llvm::VectorType >(*it)) {
      ar_type = ar::VectorType::get(this->_context,
                                    ar::cast< ar::ScalarType >(ar_type),
                                    ar::ZNumber(vector_type->getElementCount()
                                                    .getFixedValue()));
    } else if (auto struct_type = llvm::dyn_cast< llvm::StructType >(*it)) {
      // This is the last element of di_type->getElements()
      ar::StructType::Layout ar_layout;
      const llvm::StructLayout* struct_layout =
          this->_llvm_data_layout.getStructLayout(struct_type);

      for (unsigned i = 0; i < struct_type->getNumElements(); i++) {
        ar::ZNumber offset(struct_layout->getElementOffset(i));
        ar_type = this->translate_type(struct_type->getElementType(i),
                                       di_final_element);
        ar_layout.push_back({offset, ar_type});
      }

      ar_type = ar::StructType::create(this->_context, ar_layout, true);
    } else if (llvm::isa< llvm::PointerType >(*it)) {
      ar_type = ar::PointerType::get(this->_context, ar_type);
    } else {
      ikos_unreachable("unreachable");
    }

    this->sanity_check_size(*it, ar_type);
  }

  this->store_translation(type, di_type, ar_type);
  return ar_type;
}

/*
 * Helper functions for translate_struct_di_type
 */

/// \brief Remove any typedef/const/volatile/etc. qualifier
static llvm::DIType* remove_di_qualifiers(llvm::DIType* type) {
  while (type != nullptr && !type->isForwardDecl() &&
         llvm::isa< llvm::DIDerivedType >(type)) {
    auto derived_type = llvm::cast< llvm::DIDerivedType >(type);
    if (derived_type->getTag() == dwarf::DW_TAG_typedef ||
        derived_type->getTag() == dwarf::DW_TAG_const_type ||
        derived_type->getTag() == dwarf::DW_TAG_volatile_type ||
        derived_type->getTag() == dwarf::DW_TAG_restrict_type ||
        derived_type->getTag() == dwarf::DW_TAG_atomic_type) {
      type = llvm::cast_or_null< llvm::DIType >(derived_type->getRawBaseType());
    } else {
      break;
    }
  }
  return type;
}

/// \brief Return the parent class of a class, given the llvm::DIDerivedType*
static llvm::DICompositeType* get_composite_parent(
    llvm::DIDerivedType* di_member) {
  ikos_assert(di_member->getTag() == dwarf::DW_TAG_inheritance);

  check_import(di_member->getRawBaseType() != nullptr,
               "unexpected null pointer in member of llvm DICompositeType");
  auto di_member_base = llvm::cast< llvm::DIType >(di_member->getRawBaseType());
  di_member_base = remove_di_qualifiers(di_member_base);

  check_import(llvm::isa< llvm::DICompositeType >(di_member_base),
               "llvm DIDerivedType with inheritance tag, but the base type is "
               "not a DICompositeType");
  auto di_parent = llvm::cast< llvm::DICompositeType >(di_member_base);

  check_import(di_parent->getTag() == dwarf::DW_TAG_structure_type ||
                   di_parent->getTag() == dwarf::DW_TAG_class_type,
               "llvm DIDerivedType with inheritance tag, but the base type "
               "does not have structure or class tag");

  return di_parent;
}

// forward declaration
static bool is_empty_composite(llvm::DICompositeType* di_type);

/// \brief Return true if the given list has no non-static member field
template < typename Iterator >
static bool has_no_member(Iterator begin, Iterator end) {
  return std::all_of(begin, end, [](llvm::DINode* t) {
    if (llvm::isa< llvm::DISubprogram >(t)) {
      return true;
    } else if (llvm::cast< llvm::DIType >(t)->isStaticMember()) {
      return true;
    } else if (llvm::isa< llvm::DIDerivedType >(t)) {
      auto di_member = llvm::cast< llvm::DIDerivedType >(t);
      if (di_member->getTag() == dwarf::DW_TAG_member) {
        return false;
      } else if (di_member->getTag() == dwarf::DW_TAG_inheritance) {
        return is_empty_composite(get_composite_parent(di_member));
      } else {
        throw ImportError("unexpected tag for member of llvm DICompositeType");
      }
    } else {
      throw ImportError("unexpected element in llvm DICompositeType");
    }
  });
}

/// \brief Return true if the given llvm::DICompositeType* is an empty
/// structure or class (no non-static member field)
static bool is_empty_composite(llvm::DICompositeType* di_type) {
  if (di_type->isForwardDecl()) {
    return false; // assume it's not empty
  }
  check_import(di_type->getRawElements() != nullptr,
               "unexpected null pointer for elements in llvm DICompositeType");
  llvm::DINodeArray di_elements = di_type->getElements();
  return has_no_member(di_elements.begin(), di_elements.end());
}

enum FieldPosition {
  Before,
  Inside,
  Overlap,
  After,
};

/// \brief Position of B relative to A
static FieldPosition get_relative_position(const ar::ZNumber& a_offset,
                                           const ar::ZNumber& a_size,
                                           const ar::ZNumber& b_offset,
                                           const ar::ZNumber& b_size) {
  if (a_offset <= b_offset) {
    if ((a_size > 0 && b_offset < (a_offset + a_size)) ||
        (a_size == 0 && b_offset <= a_offset)) {
      if (b_offset + b_size <= a_offset + a_size) {
        return Inside;
      } else {
        return Overlap;
      }
    } else {
      return After;
    }
  } else {
    if (b_offset + b_size >= a_offset) {
      return Overlap;
    } else {
      return Before;
    }
  }
}

/// \brief Return true if the given llvm::Type is a base subobject
static bool is_base_subobject(llvm::DIDerivedType* di_member,
                              llvm::Type* member) {
  if (di_member->getTag() == dwarf::DW_TAG_inheritance &&
      llvm::isa< llvm::StructType >(member)) {
    auto struct_type = llvm::cast< llvm::StructType >(member);
    return struct_type->hasName() &&
           (struct_type->getName().startswith("struct.") ||
            struct_type->getName().startswith("class.")) &&
           struct_type->getName().endswith(".base");
  }
  return false;
}

ar::StructType* TypeWithDebugInfoImporter::translate_struct_di_type(
    llvm::DICompositeType* di_type, llvm::Type* type) {
  check_match(type->isStructTy(),
              "llvm DICompositeType with structure or class tag, but llvm type "
              "is not a structure type");
  auto struct_type = llvm::cast< llvm::StructType >(type);

  check_match(!struct_type->isOpaque(),
              "unexpected opaque llvm structure type");
  check_match(struct_type->isSized(), "unexpected unsized llvm structure type");

  const llvm::StructLayout* struct_layout =
      this->_llvm_data_layout.getStructLayout(struct_type);
  check_match(llvm::alignTo(di_type->getSizeInBits(),
                            static_cast< uint64_t >(
                                struct_layout->getAlignment().value()) *
                                8) == struct_layout->getSizeInBits(),
              "llvm DICompositeType and llvm structure type have a different "
              "bit-width");

  // Structures can be recursive, so create it now, with an empty layout
  ar::StructType* ar_type =
      ar::StructType::create(this->_context, struct_type->isPacked());
  this->store_translation(type, di_type, ar_type);
  this->_translation_depth++; // recursive type, not complete yet

  // Create an empty layout
  ar::StructType::Layout ar_layout;
  ar_layout.reserve(struct_type->getNumElements());

  // Collect debug info members
  check_import(di_type->getRawElements() != nullptr,
               "unexpected null pointer for elements in llvm DICompositeType");
  llvm::DINodeArray di_elements = di_type->getElements();
  std::vector< llvm::DIDerivedType* > di_members;
  di_members.reserve(di_elements.size());

  for (llvm::DINode* di_element : di_elements) {
    // Skip methods and static members
    if (llvm::isa< llvm::DISubprogram >(di_element) ||
        llvm::cast< llvm::DIType >(di_element)->isStaticMember()) {
      continue;
    }

    check_import(llvm::isa< llvm::DIDerivedType >(di_element),
                 "unexpected element in llvm DICompositeType");
    di_members.push_back(llvm::cast< llvm::DIDerivedType >(di_element));
  }

  // Sort debug info members by offset
  std::stable_sort(di_members.begin(),
                   di_members.end(),
                   [](llvm::DIDerivedType* a, llvm::DIDerivedType* b) {
                     return a->getOffsetInBits() < b->getOffsetInBits();
                   });

  // Iterate over the struct_type layout,
  // Matching members from debug info and llvm type
  llvm::SmallVector< llvm::DIDerivedType*, 3 > di_matching_members;
  bool has_virtual_bases = false;
  auto di_member_it = di_members.begin();

  for (unsigned i = 0; i < struct_type->getNumElements(); i++) {
    // llvm struct member
    llvm::Type* element_type = struct_type->getElementType(i);
    ar::ZNumber element_offset_bytes(struct_layout->getElementOffset(i));
    ar::ZNumber element_size_bytes(
        this->_llvm_data_layout.getTypeStoreSize(element_type).getFixedSize());

    // Find matching debug info
    di_matching_members.clear();

    // Field is a bit-field
    bool is_bit_field = false;

    while (di_member_it != di_members.end()) {
      llvm::DIDerivedType* di_member = *di_member_it;

      ar::ZNumber di_offset_bits(di_member->getOffsetInBits());
      ar::ZNumber di_size_bits;

      if (di_member->getTag() == dwarf::DW_TAG_member) {
        di_size_bits = di_member->getSizeInBits();
      } else if (di_member->getTag() == dwarf::DW_TAG_inheritance) {
        llvm::DICompositeType* di_member_base = get_composite_parent(di_member);
        di_size_bits = di_member_base->getSizeInBits();

        // Skip empty classes and structures in C++
        if (this->_is_cpp && is_empty_composite(di_member_base)) {
          ++di_member_it;
          continue;
        }

        // Handle virtual inheritance
        if (this->_is_cpp && di_member->isVirtual()) {
          has_virtual_bases = true;
          ++di_member_it;
          continue;
        }
      } else {
        throw ImportError("unsupported tag for member of llvm DICompositeType");
      }

      FieldPosition pos = get_relative_position(element_offset_bytes * 8,
                                                element_size_bytes * 8,
                                                di_offset_bits,
                                                di_size_bits);

      // Base subobject, for virtual inheritance and ABI issues
      if (this->_is_cpp && is_base_subobject(di_member, element_type)) {
        has_virtual_bases = true;
        ++di_member_it;
        break;
      }

      // Field is an empty structure in C
      // In this case, it overlaps with the next field
      if (this->_is_c && (pos == Inside || pos == Overlap) &&
          !di_matching_members.empty() && element_size_bytes == 0 &&
          element_offset_bytes * 8 == di_offset_bits &&
          (element_type->isStructTy() || element_type->isArrayTy())) {
        break;
      }

      // Field is an overlapping bit-field
      if (pos == Overlap && di_member->isBitField()) {
        is_bit_field = true;
        di_matching_members.push_back(di_member);
        ++di_member_it;
        continue;
      }

      if (pos == Inside) {
        is_bit_field = is_bit_field || di_member->isBitField();
        di_matching_members.push_back(di_member);
        ++di_member_it;
      } else if (pos == After) {
        break;
      } else {
        throw TypeDebugInfoMismatch(
            "structure member of llvm DICompositeType does not match any "
            "member in llvm structure type");
      }
    }

    if (di_matching_members.size() == 1 && !is_bit_field) {
      // Only one DIDerivedType matches the structure member
      llvm::DIDerivedType* di_member = di_matching_members[0];
      ar::ZNumber di_offset_bits(di_member->getOffsetInBits());
      check_match(di_offset_bits == element_offset_bytes * 8,
                  "llvm DIDerivedType with member or class tag and llvm type "
                  "have a different offset");
      auto di_member_base =
          llvm::cast< llvm::DIType >(di_member->getRawBaseType());

      ar::Type* ar_element_type =
          this->translate_type(element_type, di_member_base);
      ar_layout.push_back({element_offset_bytes, ar_element_type});
    } else if (!di_matching_members.empty() && is_bit_field) {
      // One or more DIDerivedType matches the structure member, and
      // One of them has a DIFlagBitField.
      check_match(element_type->isIntegerTy() ||
                      (element_type->isArrayTy() &&
                       llvm::cast< llvm::ArrayType >(element_type)
                           ->getElementType()
                           ->isIntegerTy(8)),
                  "llvm structure member matches several bit-field llvm "
                  "DIDerivedType, but llvm type is not iX or [X x i8]");

      ar::Type* ar_element_type =
          this->_type_sign_imp.translate_type(element_type, ar::Unsigned);
      ar_layout.push_back({element_offset_bytes, ar_element_type});
    } else if (!di_matching_members.empty()) {
      throw TypeDebugInfoMismatch(
          "llvm structure member matches several llvm DIDerivedType, and none "
          "of them has a bitfield flag");
    } else {
      // di_matching_members is empty
      if (has_virtual_bases && element_type->isStructTy()) {
        ar::Type* ar_element_type =
            this->_type_sign_imp.translate_type(element_type, ar::Signed);
        ar_layout.push_back({element_offset_bytes, ar_element_type});
      } else if (element_type->isIntegerTy(8) ||
                 (element_type->isArrayTy() &&
                  llvm::cast< llvm::ArrayType >(element_type)
                      ->getElementType()
                      ->isIntegerTy(8))) {
        // padding i8 or [n x i8]
        //
        // This is added by clang for different reasons:
        //   * Bit-fields
        //   * Packed structures
        //   * Alignment for inheritance
        //   * Empty structure in C++, translated into { i8 }
        ar::Type* ar_element_type =
            this->_type_sign_imp.translate_type(element_type, ar::Signed);
        ar_layout.push_back({element_offset_bytes, ar_element_type});
      } else {
        throw TypeDebugInfoMismatch(
            "no matching llvm DIDerivedType for llvm structure member");
      }
    }
  }

  check_match(has_no_member(di_member_it, di_members.end()),
              "no matching llvm structure member for llvm DIDerivedType");

  ar_type->set_layout(ar_layout);
  this->_translation_depth--;
  this->sanity_check_size(struct_type, ar_type);
  return ar_type;
}

ar::Type* TypeWithDebugInfoImporter::translate_union_di_type(
    llvm::DICompositeType* di_type, llvm::Type* type) {
  check_import(di_type->getRawElements() != nullptr,
               "unexpected null pointer for elements in llvm DICompositeType");
  llvm::DINodeArray di_members = di_type->getElements();

  check_match(type->isStructTy(),
              "llvm DICompositeType with union tag, but llvm type is not a "
              "structure type");
  auto struct_type = llvm::cast< llvm::StructType >(type);

  check_match(!struct_type->isOpaque(),
              "unexpected opaque llvm structure type");
  check_match(struct_type->isSized(), "unexpected unsized llvm structure type");

  if (struct_type->getNumElements() == 0) {
    // Empty union
    check_match(di_members.begin() == di_members.end(),
                "empty llvm structure type, but llvm DICompositeType with "
                "union tag is not empty");

    ar::StructType* ar_type =
        ar::StructType::create(this->_context, struct_type->isPacked());
    this->store_translation(type, di_type, ar_type);
    return ar_type;
  }

  check_match(struct_type->getNumElements() <= 2,
              "llvm DICompositeType with union tag, but llvm structure type "
              "has more than 2 members");

  llvm::Type* inner_type = struct_type->getElementType(0);
  llvm::Type* padding_type = nullptr;
  if (struct_type->getNumElements() == 2) {
    padding_type = struct_type->getElementType(1);
    check_match(padding_type->isArrayTy() &&
                    llvm::cast< llvm::ArrayType >(padding_type)
                        ->getElementType()
                        ->isIntegerTy(8),
                "llvm DICompositeType with union tag, but the second member of "
                "the llvm structure type does not correspond to padding");
  }

  const llvm::StructLayout* struct_layout =
      this->_llvm_data_layout.getStructLayout(struct_type);
  check_match(llvm::alignTo(di_type->getSizeInBits(),
                            static_cast< uint64_t >(
                                struct_layout->getAlignment().value()) *
                                8) == struct_layout->getSizeInBits(),
              "llvm DICompositeType and llvm structure type have a different "
              "bit-width");

  // Structures can be recursive, so create it now, with an empty layout
  ar::StructType* ar_type =
      ar::StructType::create(this->_context, struct_type->isPacked());

  // Find the first member matching the llvm::StructType
  for (llvm::DINode* di_member_node : di_members) {
    // Skip methods and static members
    if (llvm::isa< llvm::DISubprogram >(di_member_node) ||
        llvm::cast< llvm::DIType >(di_member_node)->isStaticMember()) {
      continue;
    }

    auto di_member = llvm::cast< llvm::DIDerivedType >(di_member_node);
    check_import(di_member->getTag() == dwarf::DW_TAG_member,
                 "unsupported tag for union member of llvm DICompositeType");
    check_import(di_member->getRawBaseType() != nullptr,
                 "unexpected null pointer in union member of llvm "
                 "DICompositeType");
    auto di_member_type =
        llvm::cast< llvm::DIType >(di_member->getRawBaseType());

    // Bit-field in union type
    if (di_member->isBitField() &&
        llvm::isa< llvm::DIBasicType >(di_member_type) &&
        inner_type->isIntegerTy() &&
        llvm::cast< llvm::IntegerType >(inner_type)->getBitWidth() ==
            di_member->getSizeInBits()) {
      ar_type = ar::cast< ar::StructType >(
          this->_type_sign_imp.translate_type(struct_type, ar::Signed));
      this->sanity_check_size(struct_type, ar_type);
      this->store_translation(type, di_type, ar_type);
      return ar_type;
    }

    try {
      // Try to translate inner_type with di_member_type
      TypeWithDebugInfoImporter imp = this->fork();
      imp.store_translation(type, di_type, ar_type);
      ar::Type* ar_inner_type = imp.translate_type(inner_type, di_member_type);
      this->join(imp);

      ar::StructType::Layout ar_layout;

      // inner type
      ar_layout.push_back({ar::ZNumber(0), ar_inner_type});

      // padding type, [n x i8]
      if (padding_type != nullptr) {
        ar::Type* ar_padding_type =
            this->_type_sign_imp.translate_type(padding_type, ar::Signed);
        ar::ZNumber padding_offset_bytes(struct_layout->getElementOffset(1));
        ar_layout.push_back({padding_offset_bytes, ar_padding_type});
      }

      ar_type->set_layout(ar_layout);
      this->sanity_check_size(struct_type, ar_type);
      return ar_type;
    } catch (const TypeDebugInfoMismatch&) {
    }
  }

  throw TypeDebugInfoMismatch(
      "could not translate llvm DICompositeType with union tag");
}

ar::Type* TypeWithDebugInfoImporter::translate_enum_di_type(
    llvm::DICompositeType* di_type, llvm::Type* type) {
  check_match(type->isIntegerTy(),
              "llvm DICompositeType with enumeration tag, but llvm type is "
              "not an integer type");
  auto int_type = llvm::cast< llvm::IntegerType >(type);
  check_match(di_type->getSizeInBits() == int_type->getBitWidth(),
              "llvm DICompositeType with enumeration tag and llvm integer "
              "type have a different bit-width");
  ar::Type* ar_type = ar::IntegerType::get(this->_context,
                                           int_type->getBitWidth(),
                                           ar::Unsigned);
  this->store_translation(type, di_type, ar_type);
  return ar_type;
}

/// \brief Return true if the function is a constructor of a structure or class
/// with virtual inheritance, such as void f(struct.F*, i8**)
static bool is_constructor_with_virtual_base(llvm::FunctionType* fun_type) {
  if (fun_type->getReturnType()->isVoidTy() && fun_type->getNumParams() == 2) {
    llvm::Type* fst = fun_type->getParamType(0);
    llvm::Type* snd = fun_type->getParamType(1);
    return fst->isPointerTy() &&
           llvm::cast< llvm::PointerType >(fst)
               ->getPointerElementType()
               ->isStructTy() &&
           snd->isPointerTy() &&
           llvm::cast< llvm::PointerType >(snd)
               ->getPointerElementType()
               ->isPointerTy() &&
           llvm::cast< llvm::PointerType >(
               llvm::cast< llvm::PointerType >(snd)->getPointerElementType())
               ->getPointerElementType()
               ->isIntegerTy(8);
  }
  return false;
}

ar::Type* TypeWithDebugInfoImporter::translate_subroutine_di_type(
    llvm::DISubroutineType* di_type, llvm::Type* type) {
  if (ikos_unlikely(type->isStructTy())) {
    // This should be very rare, but we can have a function pointer type
    // translated into a pointer on an empty structure.
    auto struct_type = llvm::cast< llvm::StructType >(type);
    check_match(!struct_type->isOpaque() && !struct_type->isPacked() &&
                    struct_type->getNumElements() == 0,
                "llvm DISubroutineType, but llvm type is a non-empty structure "
                "type");
    ar::StructType* ar_type = ar::StructType::create(this->_context, false);
    this->store_translation(type, di_type, ar_type);
    this->sanity_check_size(struct_type, ar_type);
    return ar_type;
  }

  check_match(type->isFunctionTy(),
              "llvm DISubroutineType, but llvm type is not a function type");
  auto fun_type = llvm::cast< llvm::FunctionType >(type);
  llvm::DITypeRefArray di_params = di_type->getTypeArray();

  if (di_params.size() == 0) {
    // Empty debug info parameters
    check_match(fun_type->getReturnType()->isVoidTy() &&
                    fun_type->getNumParams() == 0 && !fun_type->isVarArg(),
                "llvm DISubroutineType type array is empty but llvm function "
                "type is not void(*)()");
    ar::FunctionType* ar_type =
        ar::FunctionType::get(this->_context,
                              ar::VoidType::get(this->_context),
                              ar::FunctionType::ParamTypes(),
                              false);
    this->store_translation(type, di_type, ar_type);
    return ar_type;
  }

  // Function attributes
  bool var_arg = fun_type->isVarArg();

  // Return type
  llvm::DIType* di_ret_type = di_params[0];
  llvm::Type* ret_type = fun_type->getReturnType();
  ar::Type* ar_ret_type = nullptr;

  // Parameters
  auto param_it = fun_type->param_begin();
  auto param_et = fun_type->param_end();
  auto di_param_it = di_params.begin();
  auto di_param_et = di_params.end();

  // Return type
  if (ret_type->isVoidTy() && remove_di_qualifiers(di_ret_type) != nullptr) {
    // Structure returned as first parameter (see sret attribute)
    ar_ret_type = ar::VoidType::get(this->_context);
  } else {
    try {
      TypeWithDebugInfoImporter imp = this->fork();
      ar_ret_type = imp.translate_type(ret_type, di_ret_type);
      this->join(imp);
    } catch (const TypeDebugInfoMismatch&) {
      ar_ret_type = this->_type_sign_imp.translate_type(ret_type, ar::Signed);
    }
    di_param_it++;
  }

  // Parameters
  ar::FunctionType::ParamTypes ar_params;
  ar_params.reserve(fun_type->getNumParams());

  // If a structure is split into several fields
  llvm::StructType* split_struct = nullptr;
  llvm::SmallVector< llvm::Type*, 4 > split_struct_elements;

  for (; param_it != param_et; ++param_it) {
    llvm::Type* param_type = *param_it;

    if (di_param_it == di_param_et && this->_is_cpp &&
        is_constructor_with_virtual_base(fun_type) &&
        std::next(param_it) == param_et) {
      // Last parameter of a constructor of a structure or class
      // with virtual inheritance, such as void f(struct.F*, i8**)
      ar::Type* ar_param =
          this->_type_sign_imp.translate_type(param_type, ar::Signed);
      ar_params.push_back(ar_param);
      break;
    }

    check_match(di_param_it != di_param_et,
                "llvm DISubroutineType and llvm function type have a different "
                "number of parameters [1]");
    llvm::DIType* di_param_type = *di_param_it;
    di_param_type = remove_di_qualifiers(di_param_type);
    check_import(di_param_type != nullptr,
                 "unexpected null pointer in parameters of llvm "
                 "DISubroutineType");

    if (split_struct != nullptr) {
      // Current di_param_type is a structure split into several arguments
      ar::Type* ar_param =
          this->_type_sign_imp.translate_type(param_type, ar::Signed);
      ar_params.push_back(ar_param);

      split_struct_elements.push_back(param_type);
      split_struct = llvm::StructType::create(split_struct_elements);

      if (this->_llvm_data_layout.getTypeSizeInBits(split_struct) >=
          di_param_type->getSizeInBits()) {
        split_struct = nullptr;
        split_struct_elements.clear();
        ++di_param_it;
      }
      continue;
    }

    if (auto comp_di_param_type =
            llvm::dyn_cast< llvm::DICompositeType >(di_param_type)) {
      auto tag = static_cast< dwarf::Tag >(comp_di_param_type->getTag());

      if (tag == dwarf::DW_TAG_structure_type ||
          tag == dwarf::DW_TAG_class_type || tag == dwarf::DW_TAG_union_type) {
        // Debug info parameter is a structure, class or union

        if (auto ptr_param_type =
                llvm::dyn_cast< llvm::PointerType >(param_type)) {
          llvm::Type* pointee_param_type =
              ptr_param_type->getPointerElementType();
          if (pointee_param_type->isStructTy()) {
            try {
              // Structure passed by pointer (see byval attribute)
              TypeWithDebugInfoImporter imp = this->fork();
              ar::Type* ar_pointee_param =
                  imp.translate_type(pointee_param_type, di_param_type);
              this->join(imp);

              ar::Type* ar_param =
                  ar::PointerType::get(this->_context, ar_pointee_param);
              ar_params.push_back(ar_param);
              ++di_param_it;
              continue;
            } catch (const TypeDebugInfoMismatch&) {
            }
          }
        }

        if (!param_type->isStructTy()) {
          // Structure split into several arguments
          ar::Type* ar_param =
              this->_type_sign_imp.translate_type(param_type, ar::Signed);
          ar_params.push_back(ar_param);

          if (this->_llvm_data_layout.getTypeSizeInBits(param_type) >=
              di_param_type->getSizeInBits()) {
            ++di_param_it;
          } else {
            split_struct_elements.clear();
            split_struct_elements.push_back(param_type);
            split_struct = llvm::StructType::create(split_struct_elements);
          }
          continue;
        }
      }
    }

    // Otherwise
    ar::Type* ar_param = this->translate_type(param_type, di_param_type);
    ar_params.push_back(ar_param);
    ++di_param_it;
  }

  check_match(split_struct == nullptr,
              "llvm DISubroutineType and llvm function type have a different "
              "number of parameters [2]");
  // TODO: In c++, empty structures or classes can be entirely removed from the
  // interface. We do not currently handle it here.
  check_match(di_param_it == di_param_et ||
                  std::all_of(di_param_it,
                              di_param_et,
                              [](llvm::DIType* di_param) {
                                return di_param == nullptr;
                              }) ||
                  this->_is_cpp,
              "llvm DISubroutineType and llvm function type have a different "
              "number of parameters [3]");

  ar::FunctionType* ar_type =
      ar::FunctionType::get(this->_context, ar_ret_type, ar_params, var_arg);
  this->store_translation(type, di_type, ar_type);
  return ar_type;
}

ar::FunctionType* TypeWithDebugInfoImporter::translate_function_di_type(
    llvm::Function* fun, llvm::DISubroutineType* di_type) {
  check_import(di_type != nullptr,
               "unexpected null pointer for llvm DISubroutineType of llvm "
               "function");

  return ar::cast< ar::FunctionType >(
      this->translate_subroutine_di_type(di_type, fun->getFunctionType()));
}

TypeMatcher::TypeMatcher(ImportContext& ctx)
    : _llvm_data_layout(ctx.llvm_data_layout) {}

TypeMatcher::~TypeMatcher() = default;

bool TypeMatcher::match_type(llvm::Type* llvm_type, ar::Type* ar_type) {
  return this->match_type(llvm_type, ar_type, ARTypeSet{});
}

bool TypeMatcher::match_type(llvm::Type* llvm_type,
                             ar::Type* ar_type,
                             ARTypeSet seen) {
  if (llvm_type->isVoidTy()) {
    return ar_type->is_void();
  } else if (llvm_type->isIntegerTy()) {
    return this->match_integer_type(llvm_type, ar_type);
  } else if (llvm_type->isFloatingPointTy()) {
    return this->match_floating_point_type(llvm_type, ar_type);
  } else if (llvm_type->isPointerTy()) {
    return this->match_pointer_type(llvm_type, ar_type, std::move(seen));
  } else if (llvm_type->isArrayTy()) {
    return this->match_array_type(llvm_type, ar_type, std::move(seen));
  } else if (llvm_type->isVectorTy()) {
    return this->match_vector_type(llvm_type, ar_type, std::move(seen));
  } else if (llvm_type->isStructTy()) {
    return this->match_struct_type(llvm_type, ar_type, std::move(seen));
  } else if (llvm_type->isFunctionTy()) {
    return this->match_function_type(llvm_type, ar_type, std::move(seen));
  } else {
    throw ImportError("unsupported llvm type");
  }
}

bool TypeMatcher::match_integer_type(llvm::Type* llvm_type, ar::Type* ar_type) {
  return ar_type->is_integer() &&
         llvm::cast< llvm::IntegerType >(llvm_type)->getBitWidth() ==
             ar::cast< ar::IntegerType >(ar_type)->bit_width();
}

bool TypeMatcher::match_floating_point_type(llvm::Type* llvm_type,
                                            ar::Type* ar_type) {
  if (!ar_type->is_float()) {
    return false;
  }
  auto ar_float_type = ar::cast< ar::FloatType >(ar_type);

  if (llvm_type->isHalfTy()) {
    return ar_float_type->float_semantic() == ar::Half;
  } else if (llvm_type->isFloatTy()) {
    return ar_float_type->float_semantic() == ar::Float;
  } else if (llvm_type->isDoubleTy()) {
    return ar_float_type->float_semantic() == ar::Double;
  } else if (llvm_type->isX86_FP80Ty()) {
    return ar_float_type->float_semantic() == ar::X86_FP80;
  } else if (llvm_type->isFP128Ty()) {
    return ar_float_type->float_semantic() == ar::FP128;
  } else if (llvm_type->isPPC_FP128Ty()) {
    return ar_float_type->float_semantic() == ar::PPC_FP128;
  } else {
    throw ImportError("unsupported llvm floating point type");
  }
}

bool TypeMatcher::match_pointer_type(llvm::Type* llvm_type,
                                     ar::Type* ar_type,
                                     ARTypeSet seen) {
  if (!ar_type->is_pointer()) {
    return false;
  }
  auto llvm_pointee_type =
      llvm::cast< llvm::PointerType >(llvm_type)->getPointerElementType();
  auto ar_pointee_type = ar::cast< ar::PointerType >(ar_type)->pointee();

  return this->match_type(llvm_pointee_type, ar_pointee_type, std::move(seen));
}

bool TypeMatcher::match_array_type(llvm::Type* llvm_type,
                                   ar::Type* ar_type,
                                   ARTypeSet seen) {
  if (!ar_type->is_array()) {
    return false;
  }
  auto llvm_array_type = llvm::cast< llvm::ArrayType >(llvm_type);
  auto ar_array_type = ar::cast< ar::ArrayType >(ar_type);

  return llvm_array_type->getNumElements() == ar_array_type->num_elements() &&
         this->match_type(llvm_array_type->getElementType(),
                          ar_array_type->element_type(),
                          std::move(seen));
}

bool TypeMatcher::match_vector_type(llvm::Type* llvm_type,
                                    ar::Type* ar_type,
                                    ARTypeSet seen) {
  if (!ar_type->is_vector()) {
    return false;
  }
  auto llvm_vec_type = llvm::cast< llvm::VectorType >(llvm_type);
  auto ar_vec_type = ar::cast< ar::VectorType >(ar_type);

  return llvm_vec_type->getElementCount().getFixedValue() ==
             ar_vec_type->num_elements() &&
         this->match_type(llvm_vec_type->getElementType(),
                          ar_vec_type->element_type(),
                          std::move(seen));
}

bool TypeMatcher::match_struct_type(llvm::Type* llvm_type,
                                    ar::Type* ar_type,
                                    ARTypeSet seen) {
  auto llvm_struct_type = llvm::cast< llvm::StructType >(llvm_type);

  if (llvm_struct_type->isOpaque()) {
    return ar_type->is_opaque();
  }

  if (!ar_type->is_struct()) {
    return false;
  }
  auto ar_struct_type = ar::cast< ar::StructType >(ar_type);

  if (llvm_struct_type->isPacked() != ar_struct_type->packed() ||
      llvm_struct_type->getNumElements() != ar_struct_type->num_fields()) {
    return false;
  }

  // Avoid infinite recursion
  auto p = seen.insert({llvm_type, ar_type});
  if (!p.second) {
    return true; // already processing
  }

  const llvm::StructLayout* llvm_struct_layout =
      this->_llvm_data_layout.getStructLayout(llvm_struct_type);

  auto it = ar_struct_type->field_begin();
  for (unsigned i = 0; i < llvm_struct_type->getNumElements(); ++i, ++it) {
    llvm::Type* llvm_element_type = llvm_struct_type->getElementType(i);
    uint64_t llvm_element_offset = llvm_struct_layout->getElementOffset(i);
    if (llvm_element_offset != it->offset ||
        !this->match_type(llvm_element_type, it->type, seen)) {
      return false;
    }
  }

  return true;
}

bool TypeMatcher::match_function_type(llvm::Type* llvm_type,
                                      ar::Type* ar_type,
                                      ARTypeSet seen) {
  auto llvm_fun_type = llvm::cast< llvm::FunctionType >(llvm_type);

  if (!ar_type->is_function()) {
    return false;
  }
  auto ar_fun_type = ar::cast< ar::FunctionType >(ar_type);

  if (llvm_fun_type->isVarArg() != ar_fun_type->is_var_arg() ||
      llvm_fun_type->getNumParams() != ar_fun_type->num_parameters()) {
    return false;
  }

  if (!this->match_type(llvm_fun_type->getReturnType(),
                        ar_fun_type->return_type(),
                        seen)) {
    return false;
  }

  return std::equal(llvm_fun_type->param_begin(),
                    llvm_fun_type->param_end(),
                    ar_fun_type->param_begin(),
                    [&](llvm::Type* a, ar::Type* b) {
                      return this->match_type(a, b, seen);
                    });
}

bool TypeMatcher::match_extern_function_type(llvm::FunctionType* llvm_type,
                                             ar::FunctionType* ar_type) {
  if (llvm_type->isVarArg() != ar_type->is_var_arg() ||
      llvm_type->getNumParams() != ar_type->num_parameters()) {
    return false;
  }

  if (!this->match_extern_function_param_type(llvm_type->getReturnType(),
                                              ar_type->return_type())) {
    return false;
  }

  return std::equal(llvm_type->param_begin(),
                    llvm_type->param_end(),
                    ar_type->param_begin(),
                    [&](llvm::Type* a, ar::Type* b) {
                      return this->match_extern_function_param_type(a, b);
                    });
}

bool TypeMatcher::match_extern_function_param_type(llvm::Type* llvm_type,
                                                   ar::Type* ar_type) {
  if (llvm_type->isPointerTy()) {
    // Allow `{}*` to match with `opaque*`
    if (!ar_type->is_pointer()) {
      return false;
    }
    auto llvm_pointee_type =
        llvm::cast< llvm::PointerType >(llvm_type)->getPointerElementType();
    auto ar_pointee_type = ar::cast< ar::PointerType >(ar_type)->pointee();

    return (llvm_pointee_type->isStructTy() && ar_pointee_type->is_opaque()) ||
           this->match_type(llvm_pointee_type, ar_pointee_type);
  } else {
    return this->match_type(llvm_type, ar_type);
  }
}

TypeImporter::TypeImporter(ImportContext& ctx)
    : _type_sign_imp(ctx),
      _type_di_imp(ctx, this->_type_sign_imp),
      _type_match(ctx) {}

ar::Type* TypeImporter::translate_type(llvm::Type* type,
                                       ar::Signedness preferred) {
  return this->_type_sign_imp.translate_type(type, preferred);
}

ar::Type* TypeImporter::translate_type(llvm::Type* type,
                                       llvm::DIType* di_type) {
  TypeWithDebugInfoImporter imp = this->_type_di_imp.fork();
  ar::Type* ar_type = imp.translate_type(type, di_type);
  this->_type_di_imp.join(imp);
  return ar_type;
}

ar::FunctionType* TypeImporter::translate_function_type(
    llvm::Function* fun, llvm::DISubroutineType* di_type) {
  TypeWithDebugInfoImporter imp = this->_type_di_imp.fork();
  ar::FunctionType* ar_type = imp.translate_function_di_type(fun, di_type);
  this->_type_di_imp.join(imp);
  return ar_type;
}

bool TypeImporter::match_type(llvm::Type* llvm_type, ar::Type* ar_type) {
  return this->_type_match.match_type(llvm_type, ar_type);
}

bool TypeImporter::match_extern_function_type(llvm::FunctionType* llvm_type,
                                              ar::FunctionType* ar_type) {
  return this->_type_match.match_extern_function_type(llvm_type, ar_type);
}

} // end namespace import
} // end namespace frontend
} // end namespace ikos
