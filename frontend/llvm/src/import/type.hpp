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

/// \brief Helper class to translate types with a given signedness
class TypeWithSignImporter {
private:
  // AR context
  ar::Context& _context;

  // LLVM data layout
  const llvm::DataLayout& _llvm_data_layout;

  // AR data layout
  const ar::DataLayout& _ar_data_layout;

  // Current call depth of translation
  //
  // This is used to know if some types (e.g, structures) are still being build.
  unsigned _translation_depth;

  // Map from LLVM type + signedness to AR type
  llvm::DenseMap< std::pair< llvm::Type*, ar::Signedness >, ar::Type* > _map;

public:
  /// \brief Public constructor
  explicit TypeWithSignImporter(ImportContext& ctx);

  /// \brief No default constructor
  TypeWithSignImporter() = delete;

  /// \brief No copy constructor
  TypeWithSignImporter(const TypeWithSignImporter&) = delete;

  /// \brief No move constructor
  TypeWithSignImporter(TypeWithSignImporter&&) = delete;

  /// \brief No copy assignment operator
  TypeWithSignImporter& operator=(const TypeWithSignImporter&) = delete;

  /// \brief No move assignment operator
  TypeWithSignImporter& operator=(TypeWithSignImporter&&) = delete;

  /// \brief Destructor
  ~TypeWithSignImporter();

  /// \brief Translate a pair (llvm::Type*, ar::Signedness) into an ar::Type
  ///
  /// \param type LLVM type
  /// \param preferred Preferred signedness
  ///
  /// \throws ImportError for unsupported types
  ar::Type* translate_type(llvm::Type* type, ar::Signedness preferred);

private:
  /// \brief Store the translation result from (llvm::Type*, ar::Signedness) to
  /// ar::Type*
  void store_translation(llvm::Type*, ar::Signedness, ar::Type*);

  /// \brief Check that the llvm::Type and ar::Type have the same size
  void sanity_check_size(llvm::Type*, ar::Type*);

  /// \brief Translate a llvm::Type* into an ar::VoidType
  ar::VoidType* translate_void_type(llvm::Type*, ar::Signedness);

  /// \brief Translate a llvm::Type* into an ar::IntegerType
  ar::IntegerType* translate_integer_type(llvm::Type*, ar::Signedness);

  /// \brief Translate a llvm::Type* into an ar::FloatType
  ar::FloatType* translate_floating_point_type(llvm::Type*, ar::Signedness);

  /// \brief Translate a llvm::Type* into an ar::PointerType
  ar::PointerType* translate_pointer_type(llvm::Type*, ar::Signedness);

  /// \brief Translate a llvm::Type* into an ar::ArrayType
  ar::ArrayType* translate_array_type(llvm::Type*, ar::Signedness);

  /// \brief Translate a llvm::Type* into an ar::VectorType
  ar::VectorType* translate_vector_type(llvm::Type*, ar::Signedness);

  /// \brief Translate a llvm::Type* into an ar::StructType
  ar::Type* translate_struct_type(llvm::Type*, ar::Signedness);

  /// \brief Translate a llvm::Type* into an ar::FunctionType
  ar::FunctionType* translate_function_type(llvm::Type*, ar::Signedness);

}; // end class TypeWithSignImporter

/// \brief Helper class to translate types with debug info
class TypeWithDebugInfoImporter {
private:
  // AR context
  ar::Context& _context;

  // LLVM data layout
  const llvm::DataLayout& _llvm_data_layout;

  // AR data layout
  const ar::DataLayout& _ar_data_layout;

  // Is C one of the input source languages?
  bool _is_c;

  // Is C++ one of the input source languages?
  bool _is_cpp;

  // Current call depth of translation
  //
  // This is used to know if some types (e.g, structures) are still being build.
  unsigned _translation_depth;

  // Helper class to translate types with a given signedness
  TypeWithSignImporter& _type_sign_imp;

  // Parent importer, or nullptr
  //
  // This contains a map with already translated types
  const TypeWithDebugInfoImporter* _parent;

  // Map from LLVM type + LLVM debug info to AR type
  //
  // This map might have incomplete types during the translation because of
  // recursive types (struct or union). If a mismatch occurs, this map should be
  // discarded.
  llvm::DenseMap< std::pair< llvm::Type*, llvm::DIType* >, ar::Type* > _map;

private:
  /// \brief Private constructor
  TypeWithDebugInfoImporter(ar::Context& context,
                            const llvm::DataLayout& llvm_data_layout,
                            const ar::DataLayout& ar_data_layout,
                            bool is_c,
                            bool is_cpp,
                            unsigned translation_depth,
                            TypeWithSignImporter& type_sign_imp,
                            const TypeWithDebugInfoImporter* parent);

public:
  /// \brief Public constructor
  explicit TypeWithDebugInfoImporter(ImportContext& ctx,
                                     TypeWithSignImporter& type_sign_imp);

  /// \brief No default constructor
  TypeWithDebugInfoImporter() = delete;

private:
  /// \brief Private copy constructor
  TypeWithDebugInfoImporter(const TypeWithDebugInfoImporter&) = default;

public:
  /// \brief Move constructor
  TypeWithDebugInfoImporter(TypeWithDebugInfoImporter&&) = default;

  /// \brief No copy assignment operator
  TypeWithDebugInfoImporter& operator=(const TypeWithDebugInfoImporter&) =
      delete;

  /// \brief No move assignment operator
  TypeWithDebugInfoImporter& operator=(TypeWithDebugInfoImporter&&) = delete;

  /// \brief Destructor
  ~TypeWithDebugInfoImporter();

  /// \brief Translate a pair (llvm::Type*, llvm::DIType*) into an ar::Type
  ///
  /// \param type LLVM type
  /// \param di_type LLVM Debug Info type
  ///
  /// \throws TypeDebugInfoMismatch for a mismatch between debug info and type
  /// \throws ImportError for unsupported types
  ar::Type* translate_type(llvm::Type* type, llvm::DIType* di_type);

  /// \brief Create a child
  TypeWithDebugInfoImporter fork() const;

  /// \brief Join with a child, saving the internal translation map
  void join(const TypeWithDebugInfoImporter&);

private:
  /// \brief Store the translation result from (llvm::Type*, llvm::DIType*)
  /// to ar::Type*
  void store_translation(llvm::Type*, llvm::DIType*, ar::Type*);

  /// \brief Check that the llvm::Type and ar::Type have the same size
  void sanity_check_size(llvm::Type*, ar::Type*);

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

  /// \brief Translate (llvm::DISubroutineType*, llvm::Type*) into an ar::Type
  ar::Type* translate_subroutine_di_type(llvm::DISubroutineType*, llvm::Type*);

public:
  /// \brief Translate a llvm::Function's type with debug info into an ar::Type*
  ///
  /// This is similar to translate_subroutine_di_type() but it takes the
  /// llvm::Function* rather than the llvm::Type*, to handle special attributes
  /// on arguments (e.g, byval).
  ///
  /// \param fun LLVM function
  /// \param di_type LLVM Debug Info type
  ///
  /// \throws TypeDebugInfoMismatch for a mismatch between debug info and type
  /// \throws ImportError for unsupported types
  ar::FunctionType* translate_function_di_type(llvm::Function* fun,
                                               llvm::DISubroutineType* di_type);

}; // end class TypeWithDebugInfoImporter

/// \brief Helper class to check if a LLVM type matches an AR type
class TypeMatcher {
private:
  // LLVM data layout
  const llvm::DataLayout& _llvm_data_layout;

public:
  /// \brief Public constructor
  explicit TypeMatcher(ImportContext& ctx);

  /// \brief No default constructor
  TypeMatcher() = delete;

  /// \brief No copy constructor
  TypeMatcher(const TypeMatcher&) = delete;

  /// \brief No move constructor
  TypeMatcher(TypeMatcher&&) = delete;

  /// \brief No copy assignment operator
  TypeMatcher& operator=(const TypeMatcher&) = delete;

  /// \brief No move assignment operator
  TypeMatcher& operator=(TypeMatcher&&) = delete;

  /// \brief Destructor
  ~TypeMatcher();

  /// \brief Return true if the llvm::Type matches the ar::Type
  ///
  /// \throws ImportError for unsupported types
  bool match_type(llvm::Type*, ar::Type*);

private:
  using ARTypeSet =
      boost::container::flat_set< std::pair< llvm::Type*, ar::Type* > >;

  /// \brief Check whether a llvm::Type matches an ar::Type
  bool match_type(llvm::Type*, ar::Type*, ARTypeSet);

  /// \brief Check whether a llvm::IntegerType matches an ar::Type
  bool match_integer_type(llvm::Type*, ar::Type*);

  /// \brief Check whether a llvm::Type matches an ar::Type
  bool match_floating_point_type(llvm::Type*, ar::Type*);

  /// \brief Check whether a llvm::PointerType matches an ar::Type
  bool match_pointer_type(llvm::Type*, ar::Type*, ARTypeSet);

  /// \brief Check whether a llvm::ArrayType matches an ar::Type
  bool match_array_type(llvm::Type*, ar::Type*, ARTypeSet);

  /// \brief Check whether a llvm::VectorType matches an ar::Type
  bool match_vector_type(llvm::Type*, ar::Type*, ARTypeSet);

  /// \brief Check whether a llvm::StructType matches an ar::Type
  bool match_struct_type(llvm::Type*, ar::Type*, ARTypeSet);

  /// \brief Check whether a llvm::FunctionType matches an ar::Type
  bool match_function_type(llvm::Type*, ar::Type*, ARTypeSet);

public:
  /// \brief Check whether an extern llvm::FunctionType matches an
  /// ar::FunctionType
  ///
  /// This is similar to match_type, but it accepts to match even if a parameter
  /// is a pointer to a structure on one hand, and a pointer to an opaque type
  /// on the other hand.
  ///
  /// The opaque type is used to model libc parameters such as FILE.
  ///
  /// \throws ImportError for unsupported types
  bool match_extern_function_type(llvm::FunctionType*, ar::FunctionType*);

private:
  /// \brief Check whether a llvm::Type matches an ar::Type
  ///
  /// See `match_extern_function_type`
  bool match_extern_function_param_type(llvm::Type*, ar::Type*);

}; // end class TypeMatcher

/// \brief Helper class to translate types
class TypeImporter {
private:
  // Helper class to translate types with a given signedness
  TypeWithSignImporter _type_sign_imp;

  // Helper class to translate types with debug info
  TypeWithDebugInfoImporter _type_di_imp;

  // Helper class to check if a LLVM type matches an AR type
  TypeMatcher _type_match;

public:
  /// \brief Public constructor
  explicit TypeImporter(ImportContext& ctx);

  /// \brief Translate a pair (llvm::Type*, ar::Signedness) into an ar::Type
  ///
  /// \param type LLVM type
  /// \param preferred Preferred signedness
  ///
  /// \throws ImportError for unsupported types
  ar::Type* translate_type(llvm::Type* type, ar::Signedness preferred);

  /// \brief Translate a pair (llvm::Type*, llvm::DIType*) into an ar::Type
  ///
  /// \param type LLVM type
  /// \param di_type LLVM Debug Info type
  ///
  /// \throws TypeDebugInfoMismatch for a mismatch between debug info and type
  /// \throws ImportError for unsupported types
  ar::Type* translate_type(llvm::Type* type, llvm::DIType* di_type);

  /// \brief Translate a llvm::Function's type with debug info into an ar::Type*
  ///
  /// This is similar to translate_subroutine_di_type() but it takes the
  /// llvm::Function* rather than the llvm::Type*, to handle special attributes
  /// on arguments (e.g, byval).
  ///
  /// \param fun LLVM function
  /// \param di_type LLVM Debug Info type
  ///
  /// \throws TypeDebugInfoMismatch for a mismatch between debug info and type
  /// \throws ImportError for unsupported types
  ar::FunctionType* translate_function_type(llvm::Function* fun,
                                            llvm::DISubroutineType* di_type);

  /// \brief Return true if the llvm::Type matches the ar::Type
  ///
  /// \throws ImportError for unsupported types
  bool match_type(llvm::Type*, ar::Type*);

  /// \brief Check whether an extern llvm::FunctionType matches an
  /// ar::FunctionType
  ///
  /// This is similar to match_type, but it accepts to match even if a parameter
  /// is a pointer to a structure on one hand, and a pointer to an opaque type
  /// on the other hand.
  ///
  /// The opaque type is used to model libc parameters such as FILE.
  ///
  /// \throws ImportError for unsupported types
  bool match_extern_function_type(llvm::FunctionType*, ar::FunctionType*);

}; // end class TypeImporter

} // end namespace import
} // end namespace frontend
} // end namespace ikos
