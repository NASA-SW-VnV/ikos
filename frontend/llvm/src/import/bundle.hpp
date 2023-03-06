/*******************************************************************************
 *
 * \file
 * \brief Translate a LLVM module and Debug Information into an AR bundle
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

#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Module.h>

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/context.hpp>
#include <ikos/ar/semantic/function.hpp>
#include <ikos/ar/semantic/type.hpp>
#include <ikos/ar/semantic/value.hpp>

#include "import_context.hpp"

namespace ikos {
namespace frontend {
namespace import {

/// \brief Helper class to translate global values and functions
class BundleImporter {
private:
  // Import context
  ImportContext& _ctx;

  // AR context
  ar::Context& _context;

  // AR bundle
  ar::Bundle* _bundle;

  // Map from LLVM global variables to AR global variables
  llvm::DenseMap< llvm::GlobalVariable*, ar::GlobalVariable* > _globals;

  // Map from LLVM functions to AR functions
  llvm::DenseMap< llvm::Function*, ar::Function* > _functions;

  // Allow mismatch of LLVM types (llvm::Type) and Debug Info types
  // (llvm::DIType)
  //
  // See `Importer::ImportOption`.
  bool _allow_debug_info_mismatch;

public:
  /// \brief Public constructor
  explicit BundleImporter(ImportContext& ctx)
      : _ctx(ctx),
        _context(ctx.ar_context),
        _bundle(ctx.bundle),
        _allow_debug_info_mismatch(
            ctx.opts.test(Importer::AllowMismatchDebugInfo)) {}

  /// \brief Translate a llvm::GlobalVariable* into an ar::GlobalVariable*
  ar::GlobalVariable* translate_global_variable(llvm::GlobalVariable*);

  /// \brief Translate the initializer of a llvm::GlobalVariable* into an
  /// ar::Code*
  ar::Code* translate_global_variable_initializer(llvm::GlobalVariable*);

  /// \brief Translate a llvm::Function* into an ar::Function*
  ///
  /// Returns nullptr for intrinsics that should not be translated (e.g,
  /// llvm.dbg.* functions)
  ar::Function* translate_function(llvm::Function*);

private:
  /// \brief Translate a llvm::Function* with debug info into an ar::Function*
  ar::Function* translate_function_di(llvm::Function*, llvm::DISubprogram*);

  /// \brief Translate an external llvm::Function* into an ar::Function*
  ///
  /// Returns nullptr for intrinsics that should not be translated (e.g,
  /// llvm.dbg.* functions)
  ar::Function* translate_extern_function(llvm::Function*);

  /// \brief Translate an internal llvm::Function* into an ar::Function*
  ///
  /// This is used when no debug information is available.
  ar::Function* translate_internal_function(llvm::Function*);

public:
  /// \brief Return true if the given intrinsic should not be translated
  bool ignore_intrinsic(llvm::Intrinsic::ID);

private:
  /// \brief Translate an intrinsic llvm::Function* into an ar::Function*
  ///
  /// Returns nullptr if the function cannot be translated into an AR intrinsic
  /// (such as llvm.fabs, etc.)
  ar::Function* translate_intrinsic_function(llvm::Function*,
                                             llvm::Intrinsic::ID);

  /// \brief Translate a library llvm::Function* into an ar::Function*
  ///
  /// Returns nullptr if the function is not a well-known library function
  ar::Function* translate_library_function(llvm::Function*);

  /// \brief Return true if the given llvm::Function* has been auto-generated by
  /// clang
  bool is_clang_generated_function(llvm::Function*);

  /// \brief Translate a llvm::Function* generated by clang into an
  /// ar::Function*
  ar::Function* translate_clang_generated_function(llvm::Function*);

public:
  /// \brief Translate the body of a llvm::Function* into an ar::Code*
  ar::Code* translate_function_body(llvm::Function*);

}; // end class BundleImporter

} // end namespace import
} // end namespace frontend
} // end namespace ikos
