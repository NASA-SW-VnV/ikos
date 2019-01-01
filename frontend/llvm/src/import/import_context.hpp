/*******************************************************************************
 *
 * \file
 * \brief Define the ImportContext
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

#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/context.hpp>
#include <ikos/ar/semantic/data_layout.hpp>

#include <ikos/frontend/llvm/import.hpp>

namespace ikos {
namespace frontend {
namespace import {

// forward declarations
class TypeImporter;
class LibraryFunctionImporter;
class ConstantImporter;
class BundleImporter;

/// \brief ImportContext
///
/// Hold all the objects and information required during the translation.
struct ImportContext {
public:
  using ImportOptions = Importer::ImportOptions;

public:
  /// \brief Import options
  const ImportOptions opts;

  /// \brief Imported LLVM module
  llvm::Module& module;

  /// \brief AR bundle
  ar::Bundle* bundle;

  /// \brief Imported LLVM context
  llvm::LLVMContext& llvm_context;

  /// \brief AR context
  ar::Context& ar_context;

  /// \brief Imported LLVM data layout
  const llvm::DataLayout& llvm_data_layout;

  /// \brief AR data layout
  const ar::DataLayout& ar_data_layout;

  /// \brief Helper to translate LLVM types to AR types
  TypeImporter* type_imp;

  /// \brief Helper class to find known library functions
  LibraryFunctionImporter* lib_fun_imp;

  /// \brief Helper class to translate LLVM constants to AR values
  ConstantImporter* constant_imp;

  /// \brief Helper class to translate global values and functions
  BundleImporter* bundle_imp;

public:
  /// \brief Create an ImportContext
  ImportContext(llvm::Module& module_, ar::Bundle* bundle_, ImportOptions opts_)
      : opts(opts_),
        module(module_),
        bundle(bundle_),
        llvm_context(module_.getContext()),
        ar_context(bundle_->context()),
        llvm_data_layout(module_.getDataLayout()),
        ar_data_layout(bundle_->data_layout()),
        type_imp(nullptr),
        lib_fun_imp(nullptr),
        constant_imp(nullptr),
        bundle_imp(nullptr) {}

  void set_type_importer(TypeImporter& type_imp_) {
    this->type_imp = &type_imp_;
  }

  void set_library_function_importer(LibraryFunctionImporter& lib_fun_imp_) {
    this->lib_fun_imp = &lib_fun_imp_;
  }

  void set_constant_importer(ConstantImporter& constant_imp_) {
    this->constant_imp = &constant_imp_;
  }

  void set_bundle_importer(BundleImporter& bundle_imp_) {
    this->bundle_imp = &bundle_imp_;
  }

}; // end struct ImportContext

} // end namespace import
} // end namespace frontend
} // end namespace ikos
