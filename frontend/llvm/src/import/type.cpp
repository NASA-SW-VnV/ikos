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

// Fix for libstdc++, see https://bugs.llvm.org/show_bug.cgi?id=35755
#ifdef __GLIBCXX__

namespace std {

template <>
struct iterator_traits< llvm::DITypeRefArray::iterator > {
  using difference_type = std::ptrdiff_t;
  using value_type = llvm::DITypeRef;
  using pointer = void;
  using reference = llvm::DITypeRef;
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

TypeImporter::TypeImporter(ImportContext& ctx)
    : _context(ctx.ar_context),
      _llvm_data_layout(ctx.llvm_data_layout),
      _ar_data_layout(ctx.ar_data_layout),
      _translation_depth(0) {
  // Get all source languages
  llvm::SmallSet< llvm::dwarf::SourceLanguage, 2 > languages;

  for (auto it = ctx.module.debug_compile_units_begin(),
            et = ctx.module.debug_compile_units_end();
       it != et;
       ++it) {
    unsigned lang = it->getSourceLanguage();
    languages.insert(static_cast< dwarf::SourceLanguage >(lang));
  }

  _is_c = languages.count(dwarf::DW_LANG_C) != 0 ||
          languages.count(dwarf::DW_LANG_C89) != 0 ||
          languages.count(dwarf::DW_LANG_C99) != 0 ||
          languages.count(dwarf::DW_LANG_C11) != 0;

  _is_cpp = languages.count(dwarf::DW_LANG_C_plus_plus) != 0 ||
            languages.count(dwarf::DW_LANG_C_plus_plus_03) != 0 ||
            languages.count(dwarf::DW_LANG_C_plus_plus_11) != 0 ||
            languages.count(dwarf::DW_LANG_C_plus_plus_14) != 0;
}

ar::Type* TypeImporter::translate_di_type(llvm::DIType* di_type,
                                          llvm::Type* llvm_type) {
  auto it = this->_di_types.find({di_type, llvm_type});

  if (it != this->_di_types.end()) {
    return it->second;
  }

  if (di_type == nullptr) {
    return this->translate_null_di_type(llvm_type);
  } else if (di_type->isForwardDecl()) {
    return this->translate_forward_decl_di_type(di_type, llvm_type);
  } else if (auto basic_type = llvm::dyn_cast< llvm::DIBasicType >(di_type)) {
    return this->translate_basic_di_type(basic_type, llvm_type);
  } else if (auto derived_type =
                 llvm::dyn_cast< llvm::DIDerivedType >(di_type)) {
    return this->translate_derived_di_type(derived_type, llvm_type);
  } else if (auto composite_type =
                 llvm::dyn_cast< llvm::DICompositeType >(di_type)) {
    return this->translate_composite_di_type(composite_type, llvm_type);
  } else if (auto subroutine_type =
                 llvm::dyn_cast< llvm::DISubroutineType >(di_type)) {
    return this->translate_subroutine_di_type(subroutine_type, llvm_type);
  } else {
    throw ImportError("unsupported llvm DIType");
  }
}

void TypeImporter::store_translation(llvm::DIType* di_type,
                                     llvm::Type* llvm_type,
                                     ar::Type* ar_type) {
  ikos_assert_msg(ar_type, "result is null");
  this->_di_types.try_emplace({di_type, llvm_type}, ar_type);
}

ar::Type* TypeImporter::translate_null_di_type(llvm::Type* llvm_type) {
  ar::Type* ar_type = nullptr;

  if (llvm_type->isVoidTy()) {
    ar_type = ar::VoidType::get(this->_context);
  } else if (llvm_type->isIntegerTy(8)) {
    ar_type = ar::IntegerType::get(this->_context, 8, ar::Signed);
  } else {
    throw ImportError("unexpected llvm type with no debug information");
  }

  this->store_translation(nullptr, llvm_type, ar_type);
  return ar_type;
}

ar::Type* TypeImporter::translate_forward_decl_di_type(llvm::DIType* di_type,
                                                       llvm::Type* llvm_type) {
  // In this case, guess the type using llvm::Type*
  ar::Type* ar_type = this->translate_type(llvm_type, ar::Signed);
  this->store_translation(di_type, llvm_type, ar_type);
  return ar_type;
}

ar::Type* TypeImporter::translate_basic_di_type(llvm::DIBasicType* di_type,
                                                llvm::Type* llvm_type) {
  ar::Type* ar_type = nullptr;

  if (di_type->getEncoding() != 0) {
    auto encoding = static_cast< dwarf::TypeKind >(di_type->getEncoding());

    switch (encoding) {
      case dwarf::DW_ATE_signed:
      case dwarf::DW_ATE_unsigned:
      case dwarf::DW_ATE_signed_char:
      case dwarf::DW_ATE_unsigned_char: {
        check_import(llvm_type->isIntegerTy(),
                     "llvm DIBasicType with integer encoding, but llvm type is "
                     "not an integer type");
        auto int_type = llvm::cast< llvm::IntegerType >(llvm_type);
        check_import(di_type->getSizeInBits() == int_type->getBitWidth(),
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
        check_import(llvm_type->isIntegerTy(),
                     "llvm DIBasicType with boolean encoding, but llvm type is "
                     "not an integer type");
        auto int_type = llvm::cast< llvm::IntegerType >(llvm_type);
        check_import(di_type->getSizeInBits() == int_type->getBitWidth() ||
                         (di_type->getSizeInBits() == 8 &&
                          int_type->getBitWidth() == 1),
                     "llvm DIBasicType with boolean encoding and llvm integer "
                     "type have a different bit-width");
        ar_type = ar::IntegerType::get(this->_context,
                                       int_type->getBitWidth(),
                                       ar::Unsigned);
      } break;
      case dwarf::DW_ATE_float: {
        check_import(llvm_type->isFloatingPointTy(),
                     "llvm DIBasicType with float encoding, but llvm type is "
                     "not a floating point type");
        check_import(di_type->getSizeInBits() ==
                             llvm_type->getPrimitiveSizeInBits() ||
                         (di_type->getSizeInBits() == 128 &&
                          llvm_type->isX86_FP80Ty()),
                     "llvm DIBasicType with float encoding and llvm floating "
                     "point type have a different bit-width");

        if (llvm_type->isHalfTy()) {
          ar_type = ar::FloatType::get(this->_context, ar::Half);
        } else if (llvm_type->isFloatTy()) {
          ar_type = ar::FloatType::get(this->_context, ar::Float);
        } else if (llvm_type->isDoubleTy()) {
          ar_type = ar::FloatType::get(this->_context, ar::Double);
        } else if (llvm_type->isX86_FP80Ty()) {
          ar_type = ar::FloatType::get(this->_context, ar::X86_FP80);
        } else if (llvm_type->isFP128Ty()) {
          ar_type = ar::FloatType::get(this->_context, ar::FP128);
        } else if (llvm_type->isPPC_FP128Ty()) {
          ar_type = ar::FloatType::get(this->_context, ar::PPC_FP128);
        } else {
          throw ImportError("unsupported llvm floating point type");
        }
      } break;
      case dwarf::DW_ATE_complex_float: {
        check_import(llvm_type->isStructTy(),
                     "llvm DIBasicType with complex encoding, but llvm type is "
                     "not a structure type");
        auto struct_type = llvm::cast< llvm::StructType >(llvm_type);
        check_import(struct_type->getNumElements() == 2 &&
                         struct_type->getElementType(0) ==
                             struct_type->getElementType(1) &&
                         struct_type->getElementType(0)->isFloatingPointTy(),
                     "llvm DIBasicType with complex encoding, but llvm "
                     "structure type does not contain 2 floating points");
        check_import(di_type->getSizeInBits() ==
                         2 * struct_type->getElementType(0)
                                 ->getPrimitiveSizeInBits(),
                     "llvm DIBasicType with complex encoding and llvm "
                     "structure type have a different bit-width");
        ar_type = this->translate_type(llvm_type, ar::Signed);
      } break;
      default: {
        throw ImportError("unsupported dwarf encoding for llvm DIBasicType");
      }
    }
  } else if (di_type->getTag() != 0) {
    auto tag = static_cast< dwarf::Tag >(di_type->getTag());

    if (tag == dwarf::DW_TAG_unspecified_type &&
        di_type->getName() == "decltype(nullptr)") {
      check_import(llvm_type->isPointerTy() &&
                       llvm::cast< llvm::PointerType >(llvm_type)
                           ->getElementType()
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

  this->store_translation(di_type, llvm_type, ar_type);
  return ar_type;
}

ar::Type* TypeImporter::translate_derived_di_type(llvm::DIDerivedType* di_type,
                                                  llvm::Type* llvm_type) {
  if (di_type->getTag() == dwarf::DW_TAG_typedef ||
      di_type->getTag() == dwarf::DW_TAG_const_type ||
      di_type->getTag() == dwarf::DW_TAG_volatile_type ||
      di_type->getTag() == dwarf::DW_TAG_restrict_type ||
      di_type->getTag() == dwarf::DW_TAG_atomic_type ||
      di_type->getTag() == dwarf::DW_TAG_ptr_to_member_type) {
    return this->translate_qualified_di_type(di_type, llvm_type);
  } else if (di_type->getTag() == dwarf::DW_TAG_pointer_type) {
    return this->translate_pointer_di_type(di_type, llvm_type);
  } else if (di_type->getTag() == dwarf::DW_TAG_reference_type ||
             di_type->getTag() == dwarf::DW_TAG_rvalue_reference_type) {
    return this->translate_reference_di_type(di_type, llvm_type);
  } else {
    throw ImportError("unsupported dwarf tag for llvm DIDerivedType");
  }
}

ar::Type* TypeImporter::translate_qualified_di_type(
    llvm::DIDerivedType* di_type, llvm::Type* llvm_type) {
  auto qualified_type =
      llvm::cast_or_null< llvm::DIType >(di_type->getRawBaseType());

  ar::Type* ar_type = this->translate_di_type(qualified_type, llvm_type);
  this->store_translation(di_type, llvm_type, ar_type);
  return ar_type;
}

ar::PointerType* TypeImporter::translate_pointer_di_type(
    llvm::DIDerivedType* di_type, llvm::Type* llvm_type) {
  check_import(llvm_type->isPointerTy(),
               "llvm DIDerivedType with pointer tag, but llvm type is not a "
               "pointer type");
  check_import(di_type->getSizeInBits() ==
                   this->_llvm_data_layout.getPointerSizeInBits(),
               "llvm DIDerivedType with pointer tag and llvm pointer type have "
               "a different bit-width");
  auto ptr_type = llvm::cast< llvm::PointerType >(llvm_type);
  llvm::Type* pointee_type = ptr_type->getElementType();
  auto di_pointee_type =
      llvm::cast_or_null< llvm::DIType >(di_type->getRawBaseType());

  ar::Type* ar_pointee_type =
      this->translate_di_type(di_pointee_type, pointee_type);
  ar::PointerType* ar_type =
      ar::PointerType::get(this->_context, ar_pointee_type);
  this->store_translation(di_type, llvm_type, ar_type);
  return ar_type;
}

ar::PointerType* TypeImporter::translate_reference_di_type(
    llvm::DIDerivedType* di_type, llvm::Type* llvm_type) {
  check_import(llvm_type->isPointerTy(),
               "llvm DIDerivedType with reference tag, but llvm type is not a "
               "pointer type");
  check_import(di_type->getSizeInBits() == 0 ||
                   di_type->getSizeInBits() ==
                       this->_llvm_data_layout.getPointerSizeInBits(),
               "llvm DIDerivedType with reference tag and llvm pointer type "
               "have a different bit-width");
  auto ptr_type = llvm::cast< llvm::PointerType >(llvm_type);
  llvm::Type* pointee_type = ptr_type->getElementType();
  auto di_referred_type =
      llvm::cast_or_null< llvm::DIType >(di_type->getRawBaseType());

  ar::Type* ar_pointee_type =
      this->translate_di_type(di_referred_type, pointee_type);
  ar::PointerType* ar_type =
      ar::PointerType::get(this->_context, ar_pointee_type);
  this->store_translation(di_type, llvm_type, ar_type);
  return ar_type;
}

ar::Type* TypeImporter::translate_composite_di_type(
    llvm::DICompositeType* di_type, llvm::Type* llvm_type) {
  auto tag = static_cast< dwarf::Tag >(di_type->getTag());

  if (tag == dwarf::DW_TAG_array_type) {
    return this->translate_array_di_type(di_type, llvm_type);
  } else if (tag == dwarf::DW_TAG_structure_type ||
             tag == dwarf::DW_TAG_class_type) {
    return this->translate_struct_di_type(di_type, llvm_type);
  } else if (tag == dwarf::DW_TAG_union_type) {
    return this->translate_union_di_type(di_type, llvm_type);
  } else if (tag == dwarf::DW_TAG_enumeration_type) {
    return this->translate_enum_di_type(di_type, llvm_type);
  } else {
    throw ImportError("unsupported dwarf tag for llvm DICompositeType");
  }
}

ar::Type* TypeImporter::translate_array_di_type(llvm::DICompositeType* di_type,
                                                llvm::Type* llvm_type) {
  // Check that the elements of di_type and the llvm::Type* match,
  // and get the final element Type
  llvm::SmallVector< llvm::Type*, 3 > llvm_elements;
  llvm::Type* current_element = llvm_type;

  // True if the previous element has count = -1
  bool prev_no_count = false;

  for (auto it = di_type->getElements().begin(),
            et = di_type->getElements().end();
       it != et;
       ++it) {
    llvm::DINode* di_element = *it;
    check_import(llvm::isa< llvm::DISubrange >(di_element),
                 "unsupported element in llvm DICompositeType with array tag");
    auto subrange = llvm::cast< llvm::DISubrange >(di_element);
    auto count = subrange->getCount();

    if (count.is< llvm::ConstantInt* >() &&
        !count.get< llvm::ConstantInt* >()->isMinusOne()) {
      auto count_int = count.get< llvm::ConstantInt* >();
      check_import(!count_int->isNegative(),
                   "unexpected negative count for llvm DICompositeType with "
                   "array tag");

      if (current_element->isArrayTy()) {
        auto array_type = llvm::cast< llvm::ArrayType >(current_element);
        check_import(array_type->getNumElements() == count_int->getZExtValue(),
                     "llvm DICompositeType with array tag and llvm array type "
                     "have a different number of elements");

        llvm_elements.push_back(array_type);
        current_element = array_type->getElementType();
      } else if (current_element->isVectorTy()) {
        auto vector_type = llvm::cast< llvm::VectorType >(current_element);
        check_import(vector_type->getNumElements() == count_int->getZExtValue(),
                     "llvm DICompositeType with array tag and llvm vector type "
                     "have a different number of elements");

        llvm_elements.push_back(vector_type);
        current_element = vector_type->getElementType();
      } else if (current_element->isStructTy()) {
        auto struct_type = llvm::cast< llvm::StructType >(current_element);
        check_import(!struct_type->isOpaque(),
                     "llvm DICompositeType with array tag, but llvm structure "
                     "type is opaque");
        check_import(struct_type->isPacked(),
                     "llvm DICompositeType with array tag, but llvm structure "
                     "type is not packed");
        check_import(struct_type->getNumElements() == count_int->getZExtValue(),
                     "llvm DICompositeType with array tag and llvm structure "
                     "type have a different number of elements");
        check_import(!count_int->isZero(),
                     "llvm DICompositeType with array tag, but llvm structure "
                     "type is empty");
        check_import(std::next(it) == et,
                     "llvm DICompositeType with array tag, but llvm structure "
                     "type is not the last element");

        llvm_elements.push_back(struct_type);
        current_element = struct_type->getElementType(0);
      } else {
        throw ImportError(
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
        current_element = ptr_type->getElementType();
        prev_no_count = true;
      } else if (current_element->isArrayTy()) {
        auto array_type = llvm::cast< llvm::ArrayType >(current_element);
        check_import(array_type->getNumElements() == 0,
                     "llvm DICompositeType with array tag and count -1, but "
                     "llvm array type is not empty");

        llvm_elements.push_back(array_type);
        current_element = array_type->getElementType();
        prev_no_count = false;
      } else {
        throw ImportError(
            "llvm DICompositeType with array tag and count -1, but llvm type "
            "is not a pointer or array type");
      }
    }
  }

  llvm::Type* final_element = current_element;
  auto di_final_element =
      llvm::cast_or_null< llvm::DIType >(di_type->getRawBaseType());

  // Build the ar::Type
  ar::Type* ar_type = this->translate_di_type(di_final_element, final_element);

  for (auto it = llvm_elements.rbegin(), et = llvm_elements.rend(); it != et;
       ++it) {
    if (auto array_type = llvm::dyn_cast< llvm::ArrayType >(*it)) {
      ar_type = ar::ArrayType::get(this->_context,
                                   ar_type,
                                   ar::ZNumber(array_type->getNumElements()));
    } else if (auto vector_type = llvm::dyn_cast< llvm::VectorType >(*it)) {
      ar_type = ar::VectorType::get(this->_context,
                                    ar_type,
                                    ar::ZNumber(vector_type->getNumElements()));
    } else if (auto struct_type = llvm::dyn_cast< llvm::StructType >(*it)) {
      // This is the last element of di_type->getElements()
      ar::StructType::Layout ar_layout;
      const llvm::StructLayout* struct_layout =
          this->_llvm_data_layout.getStructLayout(struct_type);

      for (unsigned i = 0; i < struct_type->getNumElements(); i++) {
        ar::ZNumber offset(struct_layout->getElementOffset(i));
        ar_type = this->translate_di_type(di_final_element,
                                          struct_type->getElementType(i));
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

  this->store_translation(di_type, llvm_type, ar_type);
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
               "unexpected null pointer in llvm DICompositeType with structure "
               "or class tag");
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

ar::StructType* TypeImporter::translate_struct_di_type(
    llvm::DICompositeType* di_type, llvm::Type* llvm_type) {
  check_import(llvm_type->isStructTy(),
               "llvm DICompositeType with structure or class tag, but llvm "
               "type is not a structure type");
  auto struct_type = llvm::cast< llvm::StructType >(llvm_type);

  check_import(!struct_type->isOpaque(),
               "unexpected opaque llvm structure type");
  check_import(struct_type->isSized(),
               "unexpected unsized llvm structure type");

  const llvm::StructLayout* struct_layout =
      this->_llvm_data_layout.getStructLayout(struct_type);
  check_import(llvm::alignTo(di_type->getSizeInBits(),
                             static_cast< uint64_t >(
                                 struct_layout->getAlignment()) *
                                 8) == struct_layout->getSizeInBits(),
               "llvm DICompositeType and llvm structure type have a different "
               "bit-width");

  // Structures can be recursive, so create it now, with an empty layout
  ar::StructType* ar_type =
      ar::StructType::create(this->_context, struct_type->isPacked());
  this->store_translation(di_type, llvm_type, ar_type);
  this->_translation_depth++; // recursive type, not complete yet

  // Create an empty layout
  ar::StructType::Layout ar_layout;
  ar_layout.reserve(struct_type->getNumElements());

  // Collect debug info members
  check_import(di_type->getRawElements() != nullptr,
               "unexpected null pointer in llvm DICompositeType with structure "
               "or class tag");
  std::vector< llvm::DIDerivedType* > di_members;
  di_members.reserve(di_type->getElements().size());

  for (llvm::DINode* di_member : di_type->getElements()) {
    // Skip methods and static members
    if (llvm::isa< llvm::DISubprogram >(di_member) ||
        llvm::cast< llvm::DIType >(di_member)->isStaticMember()) {
      continue;
    }

    check_import(llvm::isa< llvm::DIDerivedType >(di_member),
                 "unexpected element in llvm DICompositeType");
    di_members.push_back(llvm::cast< llvm::DIDerivedType >(di_member));
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
        this->_llvm_data_layout.getTypeStoreSize(element_type));

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
        throw ImportError(
            "structure member of llvm DICompositeType does not match any "
            "member in llvm structure type");
      }
    }

    if (di_matching_members.size() == 1 && !is_bit_field) {
      // Only one DIDerivedType matches the structure member
      llvm::DIDerivedType* di_member = di_matching_members[0];
      ar::ZNumber di_offset_bits(di_member->getOffsetInBits());
      check_import(di_offset_bits == element_offset_bytes * 8,
                   "llvm DIDerivedType with member or class tag and llvm type "
                   "have a different offset");
      auto di_member_base =
          llvm::cast< llvm::DIType >(di_member->getRawBaseType());

      ar::Type* ar_element_type =
          this->translate_di_type(di_member_base, element_type);
      ar_layout.push_back({element_offset_bytes, ar_element_type});
    } else if (!di_matching_members.empty() && is_bit_field) {
      // One or more DIDerivedType matches the structure member, and
      // One of them has a DIFlagBitField.
      check_import(element_type->isIntegerTy() ||
                       (element_type->isArrayTy() &&
                        llvm::cast< llvm::ArrayType >(element_type)
                            ->getElementType()
                            ->isIntegerTy(8)),
                   "llvm structure member matches several bit-field llvm "
                   "DIDerivedType, but llvm type is not iX or [X x i8]");

      ar::Type* ar_element_type =
          this->translate_type(element_type, ar::Unsigned);
      ar_layout.push_back({element_offset_bytes, ar_element_type});
    } else if (!di_matching_members.empty()) {
      throw ImportError(
          "llvm structure member matches several llvm DIDerivedType, and none "
          "of them has a bitfield flag");
    } else {
      // di_matching_members is empty
      if (has_virtual_bases && element_type->isStructTy()) {
        ar::Type* ar_element_type =
            this->translate_type(element_type, ar::Signed);
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
            this->translate_type(element_type, ar::Signed);
        ar_layout.push_back({element_offset_bytes, ar_element_type});
      } else {
        throw ImportError(
            "no matching llvm DIDerivedType for llvm structure member");
      }
    }
  }

  check_import(has_no_member(di_member_it, di_members.end()),
               "no matching llvm structure member for llvm DIDerivedType");

  ar_type->set_layout(ar_layout);
  this->_translation_depth--;
  this->sanity_check_size(struct_type, ar_type);
  return ar_type;
}

ar::Type* TypeImporter::translate_union_di_type(llvm::DICompositeType* di_type,
                                                llvm::Type* llvm_type) {
  check_import(di_type->getRawElements() != nullptr,
               "unexpected null pointer in llvm DICompositeType with union "
               "tag");
  llvm::DINodeArray di_members = di_type->getElements();

  check_import(llvm_type->isStructTy(),
               "llvm DICompositeType with union tag, but llvm type is not a "
               "structure type");
  auto struct_type = llvm::cast< llvm::StructType >(llvm_type);

  check_import(!struct_type->isOpaque(),
               "unexpected opaque llvm structure type");
  check_import(struct_type->isSized(),
               "unexpected unsized llvm structure type");

  if (struct_type->getNumElements() == 0) {
    // Empty union
    check_import(di_members.begin() == di_members.end(),
                 "empty llvm structure type, but llvm DICompositeType with "
                 "union tag is not empty");

    ar::StructType* ar_type =
        ar::StructType::create(this->_context, struct_type->isPacked());
    this->store_translation(di_type, llvm_type, ar_type);
    return ar_type;
  }

  check_import(struct_type->getNumElements() <= 2,
               "llvm DICompositeType with union tag, but llvm structure type "
               "has more than 2 members");

  llvm::Type* inner_type = struct_type->getElementType(0);
  llvm::Type* padding_type = nullptr;
  if (struct_type->getNumElements() == 2) {
    padding_type = struct_type->getElementType(1);
    check_import(padding_type->isArrayTy() &&
                     llvm::cast< llvm::ArrayType >(padding_type)
                         ->getElementType()
                         ->isIntegerTy(8),
                 "llvm DICompositeType with union tag, but the second member "
                 "of the llvm structure type does not correspond to padding");
  }

  const llvm::StructLayout* struct_layout =
      this->_llvm_data_layout.getStructLayout(struct_type);
  check_import(llvm::alignTo(di_type->getSizeInBits(),
                             static_cast< uint64_t >(
                                 struct_layout->getAlignment()) *
                                 8) == struct_layout->getSizeInBits(),
               "llvm DICompositeType and llvm structure type have a different "
               "bit-width");

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
      ar::Type* ar_type = this->translate_type(struct_type, ar::Signed);
      this->sanity_check_size(struct_type, ar_type);
      return ar_type;
    }

    if (this->match_di_type(di_member_type, inner_type)) {
      // Structures can be recursive, so create it now, with an empty layout
      ar::StructType* ar_type =
          ar::StructType::create(this->_context, struct_type->isPacked());
      this->store_translation(di_type, llvm_type, ar_type);
      this->_translation_depth++; // recursive type, not complete yet

      ar::StructType::Layout ar_layout;

      // inner type
      ar::Type* ar_inner_type =
          this->translate_di_type(di_member_type, inner_type);
      ar_layout.push_back({ar::ZNumber(0), ar_inner_type});

      // padding type, [n x i8]
      if (padding_type != nullptr) {
        ar::Type* ar_padding_type =
            this->translate_type(padding_type, ar::Signed);
        ar::ZNumber padding_offset_bytes(struct_layout->getElementOffset(1));
        ar_layout.push_back({padding_offset_bytes, ar_padding_type});
      }

      ar_type->set_layout(ar_layout);
      this->_translation_depth--;
      this->sanity_check_size(struct_type, ar_type);
      return ar_type;
    }
  }

  throw ImportError("could not translate llvm DICompositeType with union tag");
}

ar::Type* TypeImporter::translate_enum_di_type(llvm::DICompositeType* di_type,
                                               llvm::Type* llvm_type) {
  check_import(llvm_type->isIntegerTy(),
               "llvm DICompositeType with enumeration tag, but llvm type is "
               "not an integer type");
  auto int_type = llvm::cast< llvm::IntegerType >(llvm_type);
  check_import(di_type->getSizeInBits() == int_type->getBitWidth(),
               "llvm DICompositeType with enumeration tag and llvm integer "
               "type have a different bit-width");
  ar::Type* ar_type = ar::IntegerType::get(this->_context,
                                           int_type->getBitWidth(),
                                           ar::Unsigned);
  this->store_translation(di_type, llvm_type, ar_type);
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
               ->getElementType()
               ->isStructTy() &&
           snd->isPointerTy() &&
           llvm::cast< llvm::PointerType >(snd)
               ->getElementType()
               ->isPointerTy() &&
           llvm::cast< llvm::PointerType >(
               llvm::cast< llvm::PointerType >(snd)->getElementType())
               ->getElementType()
               ->isIntegerTy(8);
  }
  return false;
}

ar::Type* TypeImporter::translate_subroutine_di_type(
    llvm::DISubroutineType* di_type, llvm::Type* llvm_type) {
  if (ikos_unlikely(llvm_type->isStructTy())) {
    // This should be very rare, but we can have a function pointer type
    // translated into a pointer on an empty structure.
    auto struct_type = llvm::cast< llvm::StructType >(llvm_type);
    check_import(!struct_type->isOpaque() && !struct_type->isPacked() &&
                     struct_type->getNumElements() == 0,
                 "llvm DISubroutineType, but llvm type is a non-empty "
                 "structure type");
    ar::StructType* ar_type = ar::StructType::create(this->_context, false);
    this->store_translation(di_type, llvm_type, ar_type);
    this->sanity_check_size(struct_type, ar_type);
    return ar_type;
  }

  check_import(llvm_type->isFunctionTy(),
               "llvm DISubroutineType, but llvm type is not a function type");
  auto fun_type = llvm::cast< llvm::FunctionType >(llvm_type);
  llvm::DITypeRefArray di_params = di_type->getTypeArray();

  if (di_params.size() == 0) {
    // Empty debug info parameters
    check_import(fun_type->getReturnType()->isVoidTy() &&
                     fun_type->getNumParams() == 0 && !fun_type->isVarArg(),
                 "llvm DISubroutineType type array is empty but llvm function "
                 "type is not void(*)()");
    ar::FunctionType* ar_type =
        ar::FunctionType::get(this->_context,
                              ar::VoidType::get(this->_context),
                              ar::FunctionType::ParamTypes(),
                              false);
    this->store_translation(di_type, llvm_type, ar_type);
    return ar_type;
  }

  // Function attributes
  bool var_arg = fun_type->isVarArg();

  // Return type
  llvm::DIType* di_ret_type = di_params[0].resolve();
  llvm::Type* llvm_ret_type = fun_type->getReturnType();
  ar::Type* ar_ret_type = nullptr;

  // Parameters
  auto param_it = fun_type->param_begin(), param_et = fun_type->param_end();
  auto di_param_it = di_params.begin(), di_param_et = di_params.end();

  // Return type
  if (llvm_ret_type->isVoidTy() &&
      remove_di_qualifiers(di_ret_type) != nullptr) {
    // Structure returned as first parameter (see sret attribute)
    ar_ret_type = ar::VoidType::get(this->_context);
  } else if (this->match_di_type(di_ret_type, llvm_ret_type)) {
    ar_ret_type = this->translate_di_type(di_ret_type, llvm_ret_type);
    di_param_it++;
  } else {
    ar_ret_type = this->translate_type(llvm_ret_type, ar::Signed);
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
        std::distance(param_it, param_et) == 1) {
      // Last parameter of a constructor of a structure or class
      // with virtual inheritance, such as void f(struct.F*, i8**)
      ar::Type* ar_param = this->translate_type(param_type, ar::Signed);
      ar_params.push_back(ar_param);
      break;
    }

    check_import(di_param_it != di_param_et,
                 "llvm DISubroutineType and llvm function type have a "
                 "different number of parameters [1]");
    llvm::DIType* di_param_type = (*di_param_it).resolve();
    di_param_type = remove_di_qualifiers(di_param_type);
    check_import(di_param_type != nullptr,
                 "unexpected null pointer in parameters of llvm "
                 "DISubroutineType");

    if (split_struct != nullptr) {
      // Current di_param_type is a structure split into several arguments
      ar::Type* ar_param = this->translate_type(param_type, ar::Signed);
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
          llvm::Type* pointee_param_type = ptr_param_type->getElementType();
          if (pointee_param_type->isStructTy() &&
              this->match_di_type(di_param_type, pointee_param_type)) {
            // Structure passed by pointer (see byval attribute)
            ar::Type* ar_pointee_param =
                this->translate_di_type(di_param_type, pointee_param_type);
            ar::Type* ar_param =
                ar::PointerType::get(this->_context, ar_pointee_param);
            ar_params.push_back(ar_param);
            ++di_param_it;
            continue;
          }
        }

        if (!param_type->isStructTy()) {
          // Structure split into several arguments
          ar::Type* ar_param = this->translate_type(param_type, ar::Signed);
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
    ar::Type* ar_param = this->translate_di_type(di_param_type, param_type);
    ar_params.push_back(ar_param);
    ++di_param_it;
  }

  check_import(split_struct == nullptr,
               "llvm DISubroutineType and llvm function type have a different "
               "number of parameters [2]");
  check_import(di_param_it == di_param_et ||
                   std::all_of(di_param_it,
                               di_param_et,
                               [](llvm::DITypeRef di_param) {
                                 return di_param.resolve() == nullptr;
                               }),
               "llvm DISubroutineType and llvm function type have a different "
               "number of parameters [3]");

  ar::FunctionType* ar_type =
      ar::FunctionType::get(this->_context, ar_ret_type, ar_params, var_arg);
  this->store_translation(di_type, llvm_type, ar_type);
  return ar_type;
}

ar::FunctionType* TypeImporter::translate_function_di_type(
    llvm::DISubroutineType* di_type, llvm::Function* fun) {
  check_import(di_type != nullptr,
               "unexpected null pointer for llvm DISubroutineType of llvm "
               "function");

  return ar::cast< ar::FunctionType >(
      this->translate_subroutine_di_type(di_type, fun->getFunctionType()));
}

bool TypeImporter::match_di_type(llvm::DIType* di_type, llvm::Type* type) {
  return this->match_di_type(di_type, type, DITypeSet{});
}

bool TypeImporter::match_di_type(llvm::DIType* di_type,
                                 llvm::Type* type,
                                 DITypeSet seen) {
  if (di_type == nullptr) {
    return this->match_null_di_type(type);
  } else if (di_type->isForwardDecl()) {
    return true;
  } else if (auto basic_type = llvm::dyn_cast< llvm::DIBasicType >(di_type)) {
    return this->match_basic_di_type(basic_type, type);
  } else if (auto derived_type =
                 llvm::dyn_cast< llvm::DIDerivedType >(di_type)) {
    return this->match_derived_di_type(derived_type, type, seen);
  } else if (auto composite_type =
                 llvm::dyn_cast< llvm::DICompositeType >(di_type)) {
    return this->match_composite_di_type(composite_type, type, seen);
  } else if (auto subroutine_type =
                 llvm::dyn_cast< llvm::DISubroutineType >(di_type)) {
    return this->match_subroutine_di_type(subroutine_type, type, seen);
  } else {
    throw ImportError("unsupported llvm DIType");
  }
}

bool TypeImporter::match_null_di_type(llvm::Type* type) {
  return type->isVoidTy() || type->isIntegerTy(8);
}

bool TypeImporter::match_basic_di_type(llvm::DIBasicType* di_type,
                                       llvm::Type* type) {
  if (di_type->getEncoding() != 0) {
    auto encoding = static_cast< dwarf::TypeKind >(di_type->getEncoding());

    switch (encoding) {
      case dwarf::DW_ATE_signed:
      case dwarf::DW_ATE_unsigned:
      case dwarf::DW_ATE_signed_char:
      case dwarf::DW_ATE_unsigned_char: {
        return type->isIntegerTy() &&
               di_type->getSizeInBits() ==
                   llvm::cast< llvm::IntegerType >(type)->getBitWidth();
      }
      case dwarf::DW_ATE_boolean: {
        return type->isIntegerTy() &&
               (di_type->getSizeInBits() ==
                    llvm::cast< llvm::IntegerType >(type)->getBitWidth() ||
                (di_type->getSizeInBits() == 8 &&
                 llvm::cast< llvm::IntegerType >(type)->getBitWidth() == 1));
      }
      case dwarf::DW_ATE_float: {
        return type->isFloatingPointTy() &&
               (di_type->getSizeInBits() == type->getPrimitiveSizeInBits() ||
                (di_type->getSizeInBits() == 128 && type->isX86_FP80Ty()));
      }
      case dwarf::DW_ATE_complex_float: {
        if (!type->isStructTy()) {
          return false;
        }
        auto struct_type = llvm::cast< llvm::StructType >(type);
        return struct_type->getNumElements() == 2 &&
               struct_type->getElementType(0) ==
                   struct_type->getElementType(1) &&
               struct_type->getElementType(0)->isFloatingPointTy() &&
               di_type->getSizeInBits() ==
                   2 * struct_type->getElementType(0)->getPrimitiveSizeInBits();
      }
      default:
        throw ImportError("unsupported dwarf encoding for llvm DIBasicType");
    }
  } else if (di_type->getTag() != 0) {
    auto tag = static_cast< dwarf::Tag >(di_type->getTag());

    if (tag == dwarf::DW_TAG_unspecified_type &&
        di_type->getName() == "decltype(nullptr)") {
      return type->isPointerTy() && llvm::cast< llvm::PointerType >(type)
                                        ->getElementType()
                                        ->isIntegerTy(8);
    } else {
      throw ImportError("unsupported dwarf tag for llvm DIBasicType");
    }
  } else {
    throw ImportError("unexpected llvm DIBasicType without tag and encoding");
  }
}

bool TypeImporter::match_derived_di_type(llvm::DIDerivedType* di_type,
                                         llvm::Type* type,
                                         DITypeSet seen) {
  if (di_type->getTag() == dwarf::DW_TAG_typedef ||
      di_type->getTag() == dwarf::DW_TAG_const_type ||
      di_type->getTag() == dwarf::DW_TAG_volatile_type ||
      di_type->getTag() == dwarf::DW_TAG_restrict_type ||
      di_type->getTag() == dwarf::DW_TAG_atomic_type ||
      di_type->getTag() == dwarf::DW_TAG_ptr_to_member_type) {
    return this->match_qualified_di_type(di_type, type, seen);
  } else if (di_type->getTag() == dwarf::DW_TAG_pointer_type) {
    return this->match_pointer_di_type(di_type, type, seen);
  } else if (di_type->getTag() == dwarf::DW_TAG_reference_type ||
             di_type->getTag() == dwarf::DW_TAG_rvalue_reference_type) {
    return this->match_reference_di_type(di_type, type, seen);
  } else {
    throw ImportError("unsupported dwarf tag for llvm DIDerivedType");
  }
}

bool TypeImporter::match_qualified_di_type(llvm::DIDerivedType* di_type,
                                           llvm::Type* type,
                                           DITypeSet seen) {
  auto qualified_type =
      llvm::cast_or_null< llvm::DIType >(di_type->getRawBaseType());
  return this->match_di_type(qualified_type, type, seen);
}

bool TypeImporter::match_pointer_di_type(llvm::DIDerivedType* di_type,
                                         llvm::Type* type,
                                         DITypeSet seen) {
  if (!type->isPointerTy()) {
    return false;
  }
  if (di_type->getSizeInBits() !=
      this->_llvm_data_layout.getPointerSizeInBits()) {
    return false;
  }
  auto ptr_type = llvm::cast< llvm::PointerType >(type);
  llvm::Type* pointee_type = ptr_type->getElementType();
  auto di_pointee_type =
      llvm::cast_or_null< llvm::DIType >(di_type->getRawBaseType());
  return this->match_di_type(di_pointee_type, pointee_type, seen);
}

bool TypeImporter::match_reference_di_type(llvm::DIDerivedType* di_type,
                                           llvm::Type* type,
                                           DITypeSet seen) {
  if (!type->isPointerTy()) {
    return false;
  }
  if (di_type->getSizeInBits() != 0 &&
      di_type->getSizeInBits() !=
          this->_llvm_data_layout.getPointerSizeInBits()) {
    return false;
  }
  auto ptr_type = llvm::cast< llvm::PointerType >(type);
  llvm::Type* pointee_type = ptr_type->getElementType();
  auto di_referred_type =
      llvm::cast_or_null< llvm::DIType >(di_type->getRawBaseType());
  return this->match_di_type(di_referred_type, pointee_type, seen);
}

bool TypeImporter::match_composite_di_type(llvm::DICompositeType* di_type,
                                           llvm::Type* type,
                                           DITypeSet seen) {
  auto tag = static_cast< dwarf::Tag >(di_type->getTag());

  if (tag == dwarf::DW_TAG_array_type) {
    return this->match_array_di_type(di_type, type, seen);
  } else if (tag == dwarf::DW_TAG_structure_type ||
             tag == dwarf::DW_TAG_class_type) {
    return this->match_struct_di_type(di_type, type, seen);
  } else if (tag == dwarf::DW_TAG_union_type) {
    return this->match_union_di_type(di_type, type, seen);
  } else if (tag == dwarf::DW_TAG_enumeration_type) {
    return this->match_enum_di_type(di_type, type);
  } else {
    throw ImportError("unsupported dwarf tag for llvm DICompositeType");
  }
}

bool TypeImporter::match_array_di_type(llvm::DICompositeType* di_type,
                                       llvm::Type* type,
                                       DITypeSet seen) {
  llvm::Type* current_type = type;

  // True if the previous element has count = -1
  bool prev_no_count = false;

  for (auto it = di_type->getElements().begin(),
            et = di_type->getElements().end();
       it != et;
       ++it) {
    llvm::DINode* di_element = *it;
    check_import(llvm::isa< llvm::DISubrange >(di_element),
                 "unexpected element in llvm DICompositeType with array tag");
    auto subrange = llvm::cast< llvm::DISubrange >(di_element);
    auto count = subrange->getCount();

    if (count.is< llvm::ConstantInt* >() &&
        !count.get< llvm::ConstantInt* >()->isMinusOne()) {
      auto count_int = count.get< llvm::ConstantInt* >();
      check_import(!count_int->isNegative(),
                   "unexpected negative count for llvm DICompositeType with "
                   "array tag");

      if (current_type->isArrayTy()) {
        auto array_type = llvm::cast< llvm::ArrayType >(current_type);

        if (array_type->getNumElements() != count_int->getZExtValue()) {
          return false;
        }

        current_type = array_type->getElementType();
      } else if (current_type->isVectorTy()) {
        auto vector_type = llvm::cast< llvm::VectorType >(current_type);

        if (vector_type->getNumElements() != count_int->getZExtValue()) {
          return false;
        }

        current_type = vector_type->getElementType();
      } else if (current_type->isStructTy()) {
        auto struct_type = llvm::cast< llvm::StructType >(current_type);

        if (struct_type->isOpaque() || !struct_type->isPacked() ||
            struct_type->getNumElements() != count_int->getZExtValue() ||
            count_int->isZero() || std::next(it) != et) {
          return false;
        }

        auto di_final_type =
            llvm::cast_or_null< llvm::DIType >(di_type->getRawBaseType());
        llvm::SmallPtrSet< llvm::Type*, 2 > matches;

        for (llvm::Type* element : struct_type->elements()) {
          if (matches.insert(element).second &&
              !this->match_di_type(di_final_type, element, seen)) {
            return false;
          }
        }

        current_type = struct_type->getElementType(0);
      } else {
        return false;
      }
      prev_no_count = false;
    } else {
      if (prev_no_count) {
        // Type T[][] is flattened into T*, so skip this element
        continue;
      } else if (current_type->isPointerTy()) {
        auto ptr_type = llvm::cast< llvm::PointerType >(current_type);

        current_type = ptr_type->getElementType();
        prev_no_count = true;
      } else if (current_type->isArrayTy()) {
        auto array_type = llvm::cast< llvm::ArrayType >(current_type);
        if (array_type->getArrayNumElements() > 0) {
          return false;
        }

        current_type = array_type->getElementType();
        prev_no_count = false;
      } else {
        return false;
      }
    }
  }

  llvm::Type* final_type = current_type;
  auto di_final_type =
      llvm::cast_or_null< llvm::DIType >(di_type->getRawBaseType());

  return this->match_di_type(di_final_type, final_type, seen);
}

bool TypeImporter::match_struct_di_type(llvm::DICompositeType* di_type,
                                        llvm::Type* type,
                                        DITypeSet seen) {
  if (!type->isStructTy()) {
    return false;
  }
  auto struct_type = llvm::cast< llvm::StructType >(type);

  if (struct_type->isOpaque() || !struct_type->isSized()) {
    return false;
  }

  const llvm::StructLayout* struct_layout =
      this->_llvm_data_layout.getStructLayout(struct_type);
  if (llvm::alignTo(di_type->getSizeInBits(),
                    static_cast< uint64_t >(struct_layout->getAlignment()) *
                        8) != struct_layout->getSizeInBits()) {
    return false;
  }

  // Avoid infinite recursion
  auto p = seen.insert({di_type, type});
  if (!p.second) {
    return true; // already processing
  }

  // Collect debug info members
  check_import(di_type->getRawElements() != nullptr,
               "unexpected null pointer in llvm DICompositeType with structure "
               "or class tag");
  std::vector< llvm::DIDerivedType* > di_members;
  di_members.reserve(di_type->getElements().size());

  for (llvm::DINode* di_member : di_type->getElements()) {
    // Skip methods and static members
    if (llvm::isa< llvm::DISubprogram >(di_member) ||
        llvm::cast< llvm::DIType >(di_member)->isStaticMember()) {
      continue;
    }

    check_import(llvm::isa< llvm::DIDerivedType >(di_member),
                 "unsupported element in llvm DICompositeType");
    di_members.push_back(llvm::cast< llvm::DIDerivedType >(di_member));
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
        this->_llvm_data_layout.getTypeStoreSize(element_type));

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
        return false;
      }
    }

    if (di_matching_members.size() == 1 && !is_bit_field) {
      // Only one DIDerivedType matches the structure member
      llvm::DIDerivedType* di_member = di_matching_members[0];
      ar::ZNumber di_offset_bits(di_member->getOffsetInBits());

      if (di_offset_bits != element_offset_bytes * 8) {
        return false;
      }

      auto di_member_base =
          llvm::cast< llvm::DIType >(di_member->getRawBaseType());

      if (!this->match_di_type(di_member_base, element_type, seen)) {
        return false;
      }
    } else if (!di_matching_members.empty() && is_bit_field) {
      // One or more DIDerivedType matches the structure member, and
      // One of them has a DIFlagBitField.
      if (!element_type->isIntegerTy() &&
          !(element_type->isArrayTy() &&
            llvm::cast< llvm::ArrayType >(element_type)
                ->getElementType()
                ->isIntegerTy(8))) {
        return false;
      }
    } else if (!di_matching_members.empty()) {
      return false;
    } else {
      if (has_virtual_bases && element_type->isStructTy()) {
        continue;
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
        continue;
      } else {
        return false;
      }
    }
  }

  return has_no_member(di_member_it, di_members.end());
}

bool TypeImporter::match_union_di_type(llvm::DICompositeType* di_type,
                                       llvm::Type* type,
                                       DITypeSet seen) {
  check_import(di_type->getRawElements() != nullptr,
               "unexpected null pointer in llvm DICompositeType with union "
               "tag");
  llvm::DINodeArray di_members = di_type->getElements();

  if (!type->isStructTy()) {
    return false;
  }
  auto struct_type = llvm::cast< llvm::StructType >(type);

  if (struct_type->isOpaque() || !struct_type->isSized()) {
    return false;
  }

  if (struct_type->getNumElements() == 0) {
    // Empty union
    return di_members.begin() == di_members.end();
  }

  if (struct_type->getNumElements() > 2) {
    return false;
  }

  llvm::Type* inner_type = struct_type->getElementType(0);
  if (struct_type->getNumElements() == 2) {
    llvm::Type* padding_type = struct_type->getElementType(1);
    if (!(padding_type->isArrayTy() &&
          llvm::cast< llvm::ArrayType >(padding_type)
              ->getElementType()
              ->isIntegerTy(8))) {
      return false;
    }
  }

  const llvm::StructLayout* struct_layout =
      this->_llvm_data_layout.getStructLayout(struct_type);
  if (llvm::alignTo(di_type->getSizeInBits(),
                    static_cast< uint64_t >(struct_layout->getAlignment()) *
                        8) != struct_layout->getSizeInBits()) {
    return false;
  }

  // Check if a member matches the llvm::StructType
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
      return true;
    }

    if (this->match_di_type(di_member_type, inner_type, seen)) {
      return true;
    }
  }

  return false; // does not match
}

bool TypeImporter::match_enum_di_type(llvm::DICompositeType* di_type,
                                      llvm::Type* type) {
  return type->isIntegerTy() &&
         di_type->getSizeInBits() ==
             llvm::cast< llvm::IntegerType >(type)->getBitWidth();
}

bool TypeImporter::match_subroutine_di_type(llvm::DISubroutineType* di_type,
                                            llvm::Type* type,
                                            DITypeSet seen) {
  if (ikos_unlikely(type->isStructTy())) {
    // This should be very rare, but we can have a function pointer type
    // translated into a pointer on an empty structure.
    auto struct_type = llvm::cast< llvm::StructType >(type);
    return !struct_type->isOpaque() && !struct_type->isPacked() &&
           struct_type->getNumElements() == 0;
  }

  if (!type->isFunctionTy()) {
    return false;
  }
  auto fun_type = llvm::cast< llvm::FunctionType >(type);
  llvm::DITypeRefArray di_params = di_type->getTypeArray();

  if (di_params.size() == 0) {
    // Empty debug info parameters
    return fun_type->getReturnType()->isVoidTy() &&
           fun_type->getNumParams() == 0 && !fun_type->isVarArg();
  }

  // Return type
  llvm::DIType* di_ret_type = di_params[0].resolve();
  llvm::Type* ret_type = fun_type->getReturnType();

  // Parameters
  auto param_it = fun_type->param_begin(), param_et = fun_type->param_end();
  auto di_param_it = di_params.begin(), di_param_et = di_params.end();

  // Return type
  if (!(ret_type->isVoidTy() && remove_di_qualifiers(di_ret_type) != nullptr)) {
    // No checks on return type, because of ABI
    di_param_it++;
  }

  // Parameters

  // If a structure is split into several fields
  llvm::StructType* split_struct = nullptr;
  llvm::SmallVector< llvm::Type*, 4 > split_struct_elements;

  for (; param_it != param_et; ++param_it) {
    llvm::Type* param_type = *param_it;

    if (di_param_it == di_param_et && this->_is_cpp &&
        is_constructor_with_virtual_base(fun_type) &&
        std::distance(param_it, param_et) == 1) {
      // Last parameter of a constructor of a structure or class
      // with virtual inheritance, such as void f(struct.F*, i8**)
      break;
    }

    if (di_param_it == di_param_et) {
      return false;
    }
    llvm::DIType* di_param_type = (*di_param_it).resolve();
    di_param_type = remove_di_qualifiers(di_param_type);
    if (di_param_type == nullptr) {
      return false;
    }

    if (split_struct != nullptr) {
      // Current di_param_type is a structure split into several arguments
      // No checks on this parameter, because of ABI
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
        // Debug info parameter is a structure or class

        if (auto ptr_param_type =
                llvm::dyn_cast< llvm::PointerType >(param_type)) {
          llvm::Type* pointee_param_type = ptr_param_type->getElementType();
          if (pointee_param_type->isStructTy() &&
              this->match_di_type(di_param_type, pointee_param_type, seen)) {
            // Structure passed by pointer (see byval attribute)
            ++di_param_it;
            continue;
          }
        }

        if (!param_type->isStructTy()) {
          // Structure split into several arguments
          // No checks on this parameter, because of ABI
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
    if (!this->match_di_type(di_param_type, param_type, seen)) {
      return false;
    }
    ++di_param_it;
  }

  if (split_struct != nullptr) {
    return false;
  }
  if (!(di_param_it == di_param_et ||
        std::all_of(di_param_it, di_param_et, [](llvm::DITypeRef di_param) {
          return di_param.resolve() == nullptr;
        }))) {
    return false;
  }

  return true;
}

ar::Type* TypeImporter::translate_type(llvm::Type* type,
                                       ar::Signedness preferred) {
  auto it = this->_types.find({type, preferred});

  if (it != this->_types.end()) {
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

ar::VoidType* TypeImporter::translate_void_type(llvm::Type* type,
                                                ar::Signedness preferred) {
  ar::VoidType* ar_type = ar::VoidType::get(this->_context);
  this->_types.try_emplace({type, preferred}, ar_type);
  return ar_type;
}

ar::IntegerType* TypeImporter::translate_integer_type(
    llvm::Type* type, ar::Signedness preferred) {
  auto int_type = llvm::cast< llvm::IntegerType >(type);
  ar::IntegerType* ar_type =
      ar::IntegerType::get(this->_context, int_type->getBitWidth(), preferred);
  this->_types.try_emplace({type, preferred}, ar_type);
  return ar_type;
}

ar::FloatType* TypeImporter::translate_floating_point_type(
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

  this->_types.try_emplace({type, preferred}, ar_type);
  return ar_type;
}

ar::PointerType* TypeImporter::translate_pointer_type(
    llvm::Type* type, ar::Signedness preferred) {
  auto ptr_type = llvm::cast< llvm::PointerType >(type);
  ar::Type* ar_pointee_type =
      this->translate_type(ptr_type->getElementType(), preferred);
  ar::PointerType* ar_type =
      ar::PointerType::get(this->_context, ar_pointee_type);
  this->_types.try_emplace({type, preferred}, ar_type);
  return ar_type;
}

ar::ArrayType* TypeImporter::translate_array_type(llvm::Type* type,
                                                  ar::Signedness preferred) {
  auto array_type = llvm::cast< llvm::ArrayType >(type);
  ar::Type* ar_element_type =
      this->translate_type(array_type->getElementType(), preferred);
  ar::ArrayType* ar_type =
      ar::ArrayType::get(this->_context,
                         ar_element_type,
                         ar::ZNumber(array_type->getNumElements()));
  this->_types.try_emplace({type, preferred}, ar_type);
  this->sanity_check_size(type, ar_type);
  return ar_type;
}

ar::VectorType* TypeImporter::translate_vector_type(llvm::Type* type,
                                                    ar::Signedness preferred) {
  auto vector_type = llvm::cast< llvm::VectorType >(type);
  ar::Type* ar_element_type =
      this->translate_type(vector_type->getElementType(), preferred);
  ar::VectorType* ar_type =
      ar::VectorType::get(this->_context,
                          ar_element_type,
                          ar::ZNumber(vector_type->getNumElements()));
  this->_types.try_emplace({type, preferred}, ar_type);
  this->sanity_check_size(type, ar_type);
  return ar_type;
}

ar::Type* TypeImporter::translate_struct_type(llvm::Type* type,
                                              ar::Signedness preferred) {
  auto struct_type = llvm::cast< llvm::StructType >(type);

  if (struct_type->isOpaque()) {
    ar::OpaqueType* ar_type = ar::OpaqueType::create(this->_context);
    this->_types.try_emplace({type, ar::Signed}, ar_type);
    this->_types.try_emplace({type, ar::Unsigned}, ar_type);
    return ar_type;
  }

  // Structures can be recursive, so create it now, with an empty layout
  ar::StructType* ar_type =
      ar::StructType::create(this->_context, struct_type->isPacked());
  this->_types.try_emplace({type, preferred}, ar_type);
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

ar::FunctionType* TypeImporter::translate_function_type(
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
  this->_types.try_emplace({type, preferred}, ar_type);
  return ar_type;
}

bool TypeImporter::match_ar_type(llvm::Type* llvm_type, ar::Type* ar_type) {
  return this->match_ar_type(llvm_type, ar_type, ARTypeSet{});
}

bool TypeImporter::match_ar_type(llvm::Type* llvm_type,
                                 ar::Type* ar_type,
                                 ARTypeSet seen) {
  if (llvm_type->isVoidTy()) {
    return ar_type->is_void();
  } else if (llvm_type->isIntegerTy()) {
    return this->match_integer_ar_type(llvm_type, ar_type);
  } else if (llvm_type->isFloatingPointTy()) {
    return this->match_floating_point_ar_type(llvm_type, ar_type);
  } else if (llvm_type->isPointerTy()) {
    return this->match_pointer_ar_type(llvm_type, ar_type, seen);
  } else if (llvm_type->isArrayTy()) {
    return this->match_array_ar_type(llvm_type, ar_type, seen);
  } else if (llvm_type->isVectorTy()) {
    return this->match_vector_ar_type(llvm_type, ar_type, seen);
  } else if (llvm_type->isStructTy()) {
    return this->match_struct_ar_type(llvm_type, ar_type, seen);
  } else if (llvm_type->isFunctionTy()) {
    return this->match_function_ar_type(llvm_type, ar_type, seen);
  } else {
    throw ImportError("unsupported llvm type");
  }
}

bool TypeImporter::match_integer_ar_type(llvm::Type* llvm_type,
                                         ar::Type* ar_type) {
  return ar_type->is_integer() &&
         llvm::cast< llvm::IntegerType >(llvm_type)->getBitWidth() ==
             ar::cast< ar::IntegerType >(ar_type)->bit_width();
}

bool TypeImporter::match_floating_point_ar_type(llvm::Type* llvm_type,
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

bool TypeImporter::match_pointer_ar_type(llvm::Type* llvm_type,
                                         ar::Type* ar_type,
                                         ARTypeSet seen) {
  if (!ar_type->is_pointer()) {
    return false;
  }
  auto llvm_pointee_type =
      llvm::cast< llvm::PointerType >(llvm_type)->getElementType();
  auto ar_pointee_type = ar::cast< ar::PointerType >(ar_type)->pointee();

  return this->match_ar_type(llvm_pointee_type, ar_pointee_type, seen);
}

bool TypeImporter::match_array_ar_type(llvm::Type* llvm_type,
                                       ar::Type* ar_type,
                                       ARTypeSet seen) {
  if (!ar_type->is_array()) {
    return false;
  }
  auto llvm_array_type = llvm::cast< llvm::ArrayType >(llvm_type);
  auto ar_array_type = ar::cast< ar::ArrayType >(ar_type);

  return llvm_array_type->getNumElements() == ar_array_type->num_elements() &&
         this->match_ar_type(llvm_array_type->getElementType(),
                             ar_array_type->element_type(),
                             seen);
}

bool TypeImporter::match_vector_ar_type(llvm::Type* llvm_type,
                                        ar::Type* ar_type,
                                        ARTypeSet seen) {
  if (!ar_type->is_vector()) {
    return false;
  }
  auto llvm_vec_type = llvm::cast< llvm::VectorType >(llvm_type);
  auto ar_vec_type = ar::cast< ar::VectorType >(ar_type);

  return llvm_vec_type->getNumElements() == ar_vec_type->num_elements() &&
         this->match_ar_type(llvm_vec_type->getElementType(),
                             ar_vec_type->element_type(),
                             seen);
}

bool TypeImporter::match_struct_ar_type(llvm::Type* llvm_type,
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
        !this->match_ar_type(llvm_element_type, it->type, seen)) {
      return false;
    }
  }

  return true;
}

bool TypeImporter::match_function_ar_type(llvm::Type* llvm_type,
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

  if (!this->match_ar_type(llvm_fun_type->getReturnType(),
                           ar_fun_type->return_type(),
                           seen)) {
    return false;
  }

  return std::equal(llvm_fun_type->param_begin(),
                    llvm_fun_type->param_end(),
                    ar_fun_type->param_begin(),
                    [&](llvm::Type* a, ar::Type* b) {
                      return this->match_ar_type(a, b, seen);
                    });
}

bool TypeImporter::match_extern_function_type(llvm::FunctionType* llvm_type,
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

bool TypeImporter::match_extern_function_param_type(llvm::Type* llvm_type,
                                                    ar::Type* ar_type) {
  if (llvm_type->isPointerTy()) {
    // Allow `{}*` to match with `opaque*`
    if (!ar_type->is_pointer()) {
      return false;
    }
    auto llvm_pointee_type =
        llvm::cast< llvm::PointerType >(llvm_type)->getElementType();
    auto ar_pointee_type = ar::cast< ar::PointerType >(ar_type)->pointee();

    return (llvm_pointee_type->isStructTy() && ar_pointee_type->is_opaque()) ||
           this->match_ar_type(llvm_pointee_type, ar_pointee_type);
  } else {
    return this->match_ar_type(llvm_type, ar_type);
  }
}

void TypeImporter::sanity_check_size(llvm::Type* llvm_type, ar::Type* ar_type) {
  if (this->_translation_depth > 0) {
    // Disable checks, some types are not complete yet.
    return;
  }
  check_import(this->_llvm_data_layout.getTypeSizeInBits(llvm_type) >=
                   this->_ar_data_layout.size_in_bits(ar_type),
               "llvm type size in bits is smaller than ar type size");
  check_import(this->_llvm_data_layout.getTypeAllocSize(llvm_type) ==
                   this->_ar_data_layout.alloc_size_in_bytes(ar_type),
               "llvm type and ar type alloc size are different");
}

} // end namespace import
} // end namespace frontend
} // end namespace ikos
