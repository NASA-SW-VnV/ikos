/*******************************************************************************
 *
 * \file
 * \brief Generate an AR bundle from a LLVM module
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

#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Module.h>

#include <ikos/core/support/assert.hpp>

#include <ikos/ar/semantic/bundle.hpp>

#include <ikos/frontend/llvm/import.hpp>

#include "bundle.hpp"
#include "constant.hpp"
#include "data_layout.hpp"
#include "import_context.hpp"
#include "library_function.hpp"
#include "type.hpp"

namespace ikos {
namespace frontend {
namespace import {

// hasDebugInfo

bool has_debug_info(llvm::Module& m) {
  return m.debug_compile_units_begin() != m.debug_compile_units_end();
}

// Importer

ar::Bundle* Importer::import(llvm::Module& module, ImportOptions opts) {
  // Create the data layout
  std::unique_ptr< ar::DataLayout > data_layout =
      translate_data_layout(module.getDataLayout(), module.getContext());

  // Create the bundle
  ar::Bundle* bundle = ar::Bundle::create(this->_context,
                                          std::move(data_layout),
                                          module.getTargetTriple());
  bundle->set_frontend(&module);

  // Create an ImportContext and Helper objects
  ImportContext ctx(module, bundle, opts);

  TypeImporter type_imp(ctx);
  ctx.set_type_importer(type_imp);

  LibraryFunctionImporter lib_fun_imp(ctx);
  ctx.set_library_function_importer(lib_fun_imp);

  ConstantImporter constant_imp(ctx);
  ctx.set_constant_importer(constant_imp);

  BundleImporter bundle_imp(ctx);
  ctx.set_bundle_importer(bundle_imp);

  // Create all the global variables (names and types only)
  for (auto it = module.global_begin(), et = module.global_end(); it != et;
       ++it) {
    llvm::GlobalVariable& gv = *it;
    bundle_imp.translate_global_variable(&gv);
  }

  // Create all the functions (names and types only)
  for (llvm::Function& fun : module) {
    bundle_imp.translate_function(&fun);
  }

  // Translate all global variable initializers
  for (auto it = module.global_begin(), et = module.global_end(); it != et;
       ++it) {
    llvm::GlobalVariable& gv = *it;
    if (!gv.isDeclaration()) {
      bundle_imp.translate_global_variable_initializer(&gv);
    }
  }

  // Translate all function bodies
  for (llvm::Function& fun : module) {
    if (!fun.isDeclaration()) {
      bundle_imp.translate_function_body(&fun);
    }
  }

  return bundle;
}

} // end namespace import
} // end namespace frontend
} // end namespace ikos
