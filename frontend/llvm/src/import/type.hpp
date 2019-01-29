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

#pragma once

#include <boost/container/flat_set.hpp>

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallSet.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/context.hpp>
#include <ikos/ar/semantic/type.hpp>

#include "import_context.hpp"

namespace llvm {

template < typename T >
struct DenseMapInfo< std::pair< T, ikos::ar::Signedness > > {
  using FirstInfo = DenseMapInfo< T >;

  static inline std::pair< T, ikos::ar::Signedness > getEmptyKey() {
    return {FirstInfo::getEmptyKey(), ikos::ar::Signed};
  }

  static inline std::pair< T, ikos::ar::Signedness > getTombstoneKey() {
    return {FirstInfo::getTombstoneKey(), ikos::ar::Signed};
  }

  static unsigned getHashValue(const std::pair< T, ikos::ar::Signedness >& p) {
    return DenseMapInfo< std::pair< T, char > >::getHashValue(
        {p.first, static_cast< char >(p.second)});
  }

  static bool isEqual(const std::pair< T, ikos::ar::Signedness >& lhs,
                      const std::pair< T, ikos::ar::Signedness >& rhs) {
    return FirstInfo::isEqual(lhs.first, rhs.first) && lhs.second == rhs.second;
  }
};

} // end namespace llvm

namespace ikos {
namespace frontend {
namespace import {

/// \brief Helper class to translate types
class TypeImporter {
private:
  // AR context
  ar::Context& _context;

  // LLVM data layout
  const llvm::DataLayout& _llvm_data_layout;

  // AR data layout
  const ar::DataLayout& _ar_data_layout;

  // Map from debug info + llvm type to AR type
  llvm::DenseMap< std::pair< llvm::DIType*, llvm::Type* >, ar::Type* >
      _di_types;

  // Map from llvm type + signedness to AR type
  llvm::DenseMap< std::pair< llvm::Type*, ar::Signedness >, ar::Type* > _types;

  // Is C one of the input source languages?
  bool _is_c;

  // Is C++ one of the input source languages?
  bool _is_cpp;

  // Current call depth of translation
  //
  // This is used to know if some types (e.g, structures) are still being build.
  unsigned _translation_depth;

public:
  /// \brief Public constructor
  explicit TypeImporter(ImportContext& ctx);

  /// \brief Translate a pair (llvm::DIType*, llvm::Type*) into an ar::Type
  ar::Type* translate_di_type(llvm::DIType*, llvm::Type*);

private:
  /// \brief Store the translation result from (llvm::DIType*, llvm::Type*)
  /// to ar::Type*
  void store_translation(llvm::DIType*, llvm::Type*, ar::Type*);

  /// \brief Translate a llvm::Type* with no debug info into an ar::Type
  ar::Type* translate_null_di_type(llvm::Type*);

  /// \brief Translate (llvm::DIType*, llvm::Type*) into an ar::Type
  ar::Type* translate_forward_decl_di_type(llvm::DIType*, llvm::Type*);

  /// \brief Translate (llvm::DIBasicType*, llvm::Type*) into an ar::Type
  ar::Type* translate_basic_di_type(llvm::DIBasicType*, llvm::Type*);

  /// \brief Translate (llvm::DIDerivedType*, llvm::Type*) into an ar::Type
  ar::Type* translate_derived_di_type(llvm::DIDerivedType*, llvm::Type*);

  /// \brief Translate (llvm::DIDerivedType*, llvm::Type*) into an ar::Type
  ar::Type* translate_qualified_di_type(llvm::DIDerivedType*, llvm::Type*);

  /// \brief Translate (llvm::DIDerivedType*, llvm::Type*) into ar::PointerType
  ar::PointerType* translate_pointer_di_type(llvm::DIDerivedType*, llvm::Type*);

  /// \brief Translate (llvm::DIDerivedType*, llvm::Type*) into ar::PointerType
  ar::PointerType* translate_reference_di_type(llvm::DIDerivedType*,
                                               llvm::Type*);

  /// \brief Translate (llvm::DICompositeType*, llvm::Type*) into an ar::Type
  ar::Type* translate_composite_di_type(llvm::DICompositeType*, llvm::Type*);

  /// \brief Translate (llvm::DICompositeType*, llvm::Type*) into ar::Type
  ar::Type* translate_array_di_type(llvm::DICompositeType*, llvm::Type*);

  /// \brief Translate (llvm::DICompositeType*, llvm::Type*) into ar::StructType
  ar::StructType* translate_struct_di_type(llvm::DICompositeType*, llvm::Type*);

  /// \brief Translate (llvm::DICompositeType*, llvm::Type*) into an ar::Type
  ar::Type* translate_union_di_type(llvm::DICompositeType*, llvm::Type*);

  /// \brief Translate (llvm::DICompositeType*, llvm::Type*) into an ar::Type
  ar::Type* translate_enum_di_type(llvm::DICompositeType*, llvm::Type*);

  /// \brief Translate (llvm::DISubroutineType*, llvm::Type*) into an
  /// ar::Type
  ar::Type* translate_subroutine_di_type(llvm::DISubroutineType*, llvm::Type*);

public:
  /// \brief Translate a llvm::Function's type with debug info into an ar::Type*
  ///
  /// This is similar to translate_subroutine_di_type() but it takes the
  /// llvm::Function* rather than the llvm::Type*, to handle special attributes
  /// on arguments (e.g, byval).
  ar::FunctionType* translate_function_di_type(llvm::DISubroutineType*,
                                               llvm::Function*);

public:
  /// \brief Check whether a llvm::DIType matches a llvm::Type
  bool match_di_type(llvm::DIType*, llvm::Type*);

private:
  using DITypeSet =
      boost::container::flat_set< std::pair< llvm::DIType*, llvm::Type* > >;

  /// \brief Check whether a llvm::DIType matches a llvm::Type
  bool match_di_type(llvm::DIType*, llvm::Type*, DITypeSet);

  /// \brief Check whether a null llvm::DIType matches a llvm::Type
  bool match_null_di_type(llvm::Type*);

  /// \brief Check whether a llvm::DIBasicType matches a llvm::Type
  bool match_basic_di_type(llvm::DIBasicType*, llvm::Type*);

  /// \brief Check whether a llvm::DIDerivedType matches a llvm::Type
  bool match_derived_di_type(llvm::DIDerivedType*, llvm::Type*, DITypeSet);

  /// \brief Check whether a llvm::DIDerivedType matches a llvm::Type
  bool match_qualified_di_type(llvm::DIDerivedType*, llvm::Type*, DITypeSet);

  /// \brief Check whether a llvm::DIDerivedType matches a llvm::Type
  bool match_pointer_di_type(llvm::DIDerivedType*, llvm::Type*, DITypeSet);

  /// \brief Check whether a llvm::DIDerivedType matches a llvm::Type
  bool match_reference_di_type(llvm::DIDerivedType*, llvm::Type*, DITypeSet);

  /// \brief Check whether a llvm::DICompositeType matches a llvm::Type
  bool match_composite_di_type(llvm::DICompositeType*, llvm::Type*, DITypeSet);

  /// \brief Check whether a llvm::DICompositeType matches a llvm::Type
  bool match_array_di_type(llvm::DICompositeType*, llvm::Type*, DITypeSet);

  /// \brief Check whether a llvm::DICompositeType matches a llvm::Type
  bool match_struct_di_type(llvm::DICompositeType*, llvm::Type*, DITypeSet);

  /// \brief Check whether a llvm::DICompositeType matches a llvm::Type
  bool match_union_di_type(llvm::DICompositeType*, llvm::Type*, DITypeSet);

  /// \brief Check whether a llvm::DICompositeType matches a llvm::Type
  bool match_enum_di_type(llvm::DICompositeType*, llvm::Type*);

  /// \brief Check whether a llvm::DISubroutineType matches a llvm::Type
  bool match_subroutine_di_type(llvm::DISubroutineType*,
                                llvm::Type*,
                                DITypeSet);

public:
  /// \brief Translate a pair (llvm::Type*, ar::Signedness) into an ar::Type
  ///
  /// \param preferred Preferred signedness
  ar::Type* translate_type(llvm::Type*, ar::Signedness preferred);

private:
  /// \brief Translate a llvm::Type* into an ar::VoidType
  ar::VoidType* translate_void_type(llvm::Type*, ar::Signedness preferred);

  /// \brief Translate a llvm::Type* into an ar::IntegerType
  ar::IntegerType* translate_integer_type(llvm::Type*,
                                          ar::Signedness preferred);

  /// \brief Translate a llvm::Type* into an ar::FloatType
  ar::FloatType* translate_floating_point_type(llvm::Type*,
                                               ar::Signedness preferred);

  /// \brief Translate a llvm::Type* into an ar::PointerType
  ar::PointerType* translate_pointer_type(llvm::Type*,
                                          ar::Signedness preferred);

  /// \brief Translate a llvm::Type* into an ar::ArrayType
  ar::ArrayType* translate_array_type(llvm::Type*, ar::Signedness preferred);

  /// \brief Translate a llvm::Type* into an ar::VectorType
  ar::VectorType* translate_vector_type(llvm::Type*, ar::Signedness preferred);

  /// \brief Translate a llvm::Type* into an ar::StructType
  ar::Type* translate_struct_type(llvm::Type*, ar::Signedness preferred);

  /// \brief Translate a llvm::Type* into an ar::FunctionType
  ar::FunctionType* translate_function_type(llvm::Type*,
                                            ar::Signedness preferred);

public:
  /// \brief Check whether a llvm::Type matches an ar::Type
  bool match_ar_type(llvm::Type*, ar::Type*);

private:
  using ARTypeSet =
      boost::container::flat_set< std::pair< llvm::Type*, ar::Type* > >;

  /// \brief Check whether a llvm::Type matches an ar::Type
  bool match_ar_type(llvm::Type*, ar::Type*, ARTypeSet);

  /// \brief Check whether a llvm::IntegerType matches an ar::Type
  bool match_integer_ar_type(llvm::Type*, ar::Type*);

  /// \brief Check whether a llvm::Type matches an ar::Type
  bool match_floating_point_ar_type(llvm::Type*, ar::Type*);

  /// \brief Check whether a llvm::PointerType matches an ar::Type
  bool match_pointer_ar_type(llvm::Type*, ar::Type*, ARTypeSet);

  /// \brief Check whether a llvm::ArrayType matches an ar::Type
  bool match_array_ar_type(llvm::Type*, ar::Type*, ARTypeSet);

  /// \brief Check whether a llvm::VectorType matches an ar::Type
  bool match_vector_ar_type(llvm::Type*, ar::Type*, ARTypeSet);

  /// \brief Check whether a llvm::StructType matches an ar::Type
  bool match_struct_ar_type(llvm::Type*, ar::Type*, ARTypeSet);

  /// \brief Check whether a llvm::FunctionType matches an ar::Type
  bool match_function_ar_type(llvm::Type*, ar::Type*, ARTypeSet);

public:
  /// \brief Check whether an extern llvm::FunctionType matches an
  /// ar::FunctionType
  ///
  /// This is similar to match_ar_type, but it accepts to match even if a
  /// parameter is a pointer to a structure on one hand, and a pointer to an
  /// opaque type on the other hand.
  ///
  /// The opaque type is used to model libc parameters such as FILE.
  bool match_extern_function_type(llvm::FunctionType*, ar::FunctionType*);

private:
  /// \brief Check whether a llvm::Type matches an ar::Type
  ///
  /// See `match_extern_function_type`
  bool match_extern_function_param_type(llvm::Type*, ar::Type*);

public:
  /// \brief Check that the llvm::Type and ar::Type have a matching size
  void sanity_check_size(llvm::Type*, ar::Type*);

}; // end class TypeImporter

} // end namespace import
} // end namespace frontend
} // end namespace ikos
