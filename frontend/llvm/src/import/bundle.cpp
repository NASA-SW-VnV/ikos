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

#include <ikos/core/support/assert.hpp>
#include <iostream>

#include <ikos/ar/semantic/statement.hpp>

#include <ikos/frontend/llvm/import/exception.hpp>

#include "bundle.hpp"
#include "constant.hpp"
#include "function.hpp"
#include "library_function.hpp"
#include "type.hpp"

namespace ikos {
namespace frontend {
namespace import {

ar::GlobalVariable* BundleImporter::translate_global_variable(
    llvm::GlobalVariable* gv) {
  auto it = this->_globals.find(gv);

  if (it != this->_globals.end()) {
    return it->second;
  }

  // Build the ar::GlobalVariable

  std::string name;
  if (gv->hasName()) {
    name = gv->getName().str();
  } else {
    name = this->_bundle->find_available_name("__unnamed_global_var");
  }

  // Special names for intrinsic global variables (such as llvm.global_ctors)
  if (name.rfind("llvm.", 0) == 0) {
    name = "ar." + name.substr(5);
  }

  llvm::PointerType* type = gv->getType();

  // Extract the DWARF type from debug information
  llvm::SmallVector< llvm::DIGlobalVariableExpression*, 1 > dbgs;
  gv->getDebugInfo(dbgs);

  ar::Type* ar_pointee_type = nullptr;
  if (dbgs.empty()) {
    // No debug info

    // Prefer signed integers, because most global variables without
    // debug information are strings (const char*)
    ar_pointee_type =
        _ctx.type_imp->translate_type(type->getPointerElementType(),
                                      ar::Signed);
  } else {
    // Use debug information to build the exact type
    llvm::DIGlobalVariable* di_gv = dbgs[0]->getVariable();
    auto di_type = llvm::cast_or_null< llvm::DIType >(di_gv->getRawType());

    try {
      ar_pointee_type =
          _ctx.type_imp->translate_type(type->getPointerElementType(), di_type);
    } catch (const TypeDebugInfoMismatch&) {
      if (!this->_allow_debug_info_mismatch) {
        throw;
      }
      ar_pointee_type =
          _ctx.type_imp->translate_type(type->getPointerElementType(),
                                        ar::Signed);
    }
  }

  // Create the ar::GlobalVariable
  ikos_assert(ar_pointee_type);
  ar::PointerType* ar_type =
      ar::PointerType::get(this->_context, ar_pointee_type);
  ar::GlobalVariable* ar_gv =
      ar::GlobalVariable::create(this->_bundle,
                                 ar_type,
                                 name,
                                 /*is_definition = */ !gv->isDeclaration(),
                                 gv->getAlignment());
  ar_gv->set_frontend(gv);
  this->_globals.try_emplace(gv, ar_gv);
  return ar_gv;
}

ar::Code* BundleImporter::translate_global_variable_initializer(
    llvm::GlobalVariable* gv) {
  ar::GlobalVariable* ar_gv = this->translate_global_variable(gv);
  ikos_assert(ar_gv->is_definition());

  // Initialize the ar::Code initializer
  ar::Code* init = ar_gv->initializer();
  ar::BasicBlock* bb = ar::BasicBlock::create(init);
  init->set_entry_block(bb);
  init->set_exit_block(bb);

  // Translate the llvm::Constant
  ar::Value* cst =
      _ctx.constant_imp->translate_constant(gv->getInitializer(),
                                            ar_gv->type()->pointee(),
                                            bb);

  // Insert statement *gv = cst
  bb->push_back(ar::Store::create(ar_gv, cst, 1, false));

  return init;
}

// This is used in the case where there is no debug information.
ar::Function* BundleImporter::translate_internal_function(llvm::Function* fun) {
  ikos_assert(!fun->isDeclaration());

  ar::Function* ar_fun = nullptr;

  // Use int for the type since it is more common than unsigned in C
  llvm::FunctionType* type = fun->getFunctionType();

  auto ar_type = ar::cast< ar::FunctionType >(
      _ctx.type_imp->translate_type(type, ar::Signed));

  ar_fun = ar::Function::create(this->_bundle,
                                ar_type,
                                fun->getName().str(),
                                /*is_definition = */ true);

  ikos_assert(ar_fun);
  return ar_fun;
}

ar::Function* BundleImporter::translate_function(llvm::Function* fun) {
  auto it = this->_functions.find(fun);

  if (it != this->_functions.end()) {
    return it->second;
  }

  // Build the ar function

  if (!fun->hasName()) {
    throw ImportError("llvm function has no name");
  }

  // Extract the DWARF type from debug information
  llvm::DISubprogram* dbg = fun->getSubprogram();

  ar::Function* ar_fun = nullptr;
  if (dbg != nullptr) {
    // Debug information available
    ar_fun = this->translate_function_di(fun, dbg);
  } else if (fun->isDeclaration()) {
    // No debug information on external function
    ar_fun = this->translate_extern_function(fun);
  } else if (this->is_clang_generated_function(fun)) {
    // Auto-generated by clang
    ar_fun = this->translate_clang_generated_function(fun);
  } else {
    // No debug information on internal function
    ar_fun = this->translate_internal_function(fun);
  }

  if (ar_fun != nullptr) {
    ar_fun->set_frontend(fun);
  }
  this->_functions.try_emplace(fun, ar_fun);
  return ar_fun;
}

ar::Function* BundleImporter::translate_function_di(llvm::Function* fun,
                                                    llvm::DISubprogram* dbg) {
  ikos_assert_msg(dbg != nullptr, "no debug info");
  ikos_assert_msg(!fun->isIntrinsic(), "unexpected intrinsic with debug info");

  // Use debug information to build the exact type
  llvm::DISubroutineType* di_type = dbg->getType();

  // Translate the function type
  ar::FunctionType* type = nullptr;

  try {
    type = _ctx.type_imp->translate_function_type(fun, di_type);
  } catch (const TypeDebugInfoMismatch&) {
    if (!this->_allow_debug_info_mismatch) {
      throw;
    }
    type = ar::cast< ar::FunctionType >(
        _ctx.type_imp->translate_type(fun->getFunctionType(), ar::Signed));
  }

  // Create the ar::Function
  return ar::Function::create(this->_bundle,
                              type,
                              fun->getName().str(),
                              /*is_definition = */ !fun->isDeclaration());
}

ar::Function* BundleImporter::translate_extern_function(llvm::Function* fun) {
  ikos_assert(fun->isDeclaration());

  ar::Function* ar_fun = nullptr;

  llvm::Intrinsic::ID id = fun->getIntrinsicID();

  // Not translated
  if (this->ignore_intrinsic(id)) {
    return nullptr;
  }

  // Translate an intrinsic function
  if (ar_fun == nullptr && fun->isIntrinsic()) {
    ar_fun = this->translate_intrinsic_function(fun, id);
  }

  // Translate known library functions (e.g, malloc, printf, etc.)
  if (ar_fun == nullptr) {
    ar_fun = this->translate_library_function(fun);
  }

  if (ar_fun == nullptr) {
    // Otherwise, just prefer signed integers,
    // because int is more common than unsigned in C
    llvm::FunctionType* type = fun->getFunctionType();

    auto ar_type = ar::cast< ar::FunctionType >(
        _ctx.type_imp->translate_type(type, ar::Signed));

    ar_fun = ar::Function::create(this->_bundle,
                                  ar_type,
                                  fun->getName().str(),
                                  /*is_definition = */ false);
  }

  ikos_assert(ar_fun);
  return ar_fun;
}

bool BundleImporter::ignore_intrinsic(llvm::Intrinsic::ID id) {
  return id == llvm::Intrinsic::dbg_value ||
         id == llvm::Intrinsic::dbg_declare ||
         id == llvm::Intrinsic::dbg_label || id == llvm::Intrinsic::prefetch;
}

ar::Function* BundleImporter::translate_intrinsic_function(
    llvm::Function* fun, llvm::Intrinsic::ID id) {
  ar::Function* ar_fun = nullptr;

  if (id == llvm::Intrinsic::memcpy) {
    ar_fun = this->_bundle->intrinsic_function(ar::Intrinsic::MemoryCopy);
  } else if (id == llvm::Intrinsic::memmove) {
    ar_fun = this->_bundle->intrinsic_function(ar::Intrinsic::MemoryMove);
  } else if (id == llvm::Intrinsic::memset) {
    ar_fun = this->_bundle->intrinsic_function(ar::Intrinsic::MemorySet);
  } else if (id == llvm::Intrinsic::vastart) {
    ar_fun = this->_bundle->intrinsic_function(ar::Intrinsic::VarArgStart);
  } else if (id == llvm::Intrinsic::vaend) {
    ar_fun = this->_bundle->intrinsic_function(ar::Intrinsic::VarArgEnd);
  } else if (id == llvm::Intrinsic::vacopy) {
    ar_fun = this->_bundle->intrinsic_function(ar::Intrinsic::VarArgCopy);
  } else if (id == llvm::Intrinsic::stacksave) {
    ar_fun = this->_bundle->intrinsic_function(ar::Intrinsic::StackSave);
  } else if (id == llvm::Intrinsic::stackrestore) {
    ar_fun = this->_bundle->intrinsic_function(ar::Intrinsic::StackRestore);
  } else if (id == llvm::Intrinsic::lifetime_start) {
    ar_fun = this->_bundle->intrinsic_function(ar::Intrinsic::LifetimeStart);
  } else if (id == llvm::Intrinsic::lifetime_end) {
    ar_fun = this->_bundle->intrinsic_function(ar::Intrinsic::LifetimeEnd);
  } else if (id == llvm::Intrinsic::eh_typeid_for) {
    ar_fun = this->_bundle->intrinsic_function(ar::Intrinsic::EhTypeidFor);
  } else if (id == llvm::Intrinsic::trap) {
    ar_fun = this->_bundle->intrinsic_function(ar::Intrinsic::Trap);
  } else {
    // No equivalent AR intrinsic, translate into a normal external function
    ar_fun = nullptr;
  }

  // Sanity check, should never happen
  // Skip for memcpy, memmove and memset because of the alignment parameter
  if (id != llvm::Intrinsic::memcpy && id != llvm::Intrinsic::memmove &&
      id != llvm::Intrinsic::memset && ar_fun != nullptr &&
      !_ctx.type_imp->match_extern_function_type(fun->getFunctionType(),
                                                 ar_fun->type())) {
    std::ostringstream buf;
    buf << "llvm intrinsic " << fun->getName().str() << " and ar intrinsic "
        << ar_fun->name() << " have a different type";
    throw ImportError(buf.str());
  }

  return ar_fun;
}

ar::Function* BundleImporter::translate_library_function(llvm::Function* fun) {
  ar::Function* ar_fun = _ctx.lib_fun_imp->function(fun->getName());

  // Sanity check, can happen if the user uses C/C++ standard library names
  if (ar_fun != nullptr &&
      !_ctx.type_imp->match_extern_function_type(fun->getFunctionType(),
                                                 ar_fun->type())) {
    std::ostringstream buf;

    if (ar_fun->is_ikos_intrinsic()) {
      buf << "function definition of " << fun->getName().str()
          << " does not match the expected ikos intrinsic definition";
    } else if (ar_fun->is_libc_intrinsic()) {
      buf << "function definition of " << fun->getName().str()
          << " does not match the expected C Standard Library definition";
    } else if (ar_fun->is_libcpp_intrinsic()) {
      buf << "function definition of " << fun->getName().str()
          << " does not match the expected C++ Standard Library definition";
    } else {
      buf << "llvm function " << fun->getName().str() << " and ar intrinsic "
          << ar_fun->name() << " have a different type";
    }
    std::cerr << "Warning: " << buf.str() << "\n";
    std::cerr << "LLVM function declaration\n";
    std::cerr << "ikos ar expected function declaration\n";
    std::cerr << "Expected signature will be ignored.\n";
    ar_fun = nullptr;
  }

  return ar_fun;
}

bool BundleImporter::is_clang_generated_function(llvm::Function* fun) {
  if (fun->isDeclaration()) {
    return false;
  }

  if (fun->getName() == "__clang_call_terminate") {
    // Terminate function
    return true;
  }

  if (fun->getName().startswith("_ZTW")) {
    // Thread-local wrapper function
    return true;
  }

  return false;
}

ar::Function* BundleImporter::translate_clang_generated_function(
    llvm::Function* fun) {
  llvm::FunctionType* type = fun->getFunctionType();

  // Translate the function type
  auto ar_type = ar::cast< ar::FunctionType >(
      _ctx.type_imp->translate_type(type, ar::Signed));

  return ar::Function::create(this->_bundle,
                              ar_type,
                              fun->getName().str(),
                              /*is_definition = */ !fun->isDeclaration());
}

ar::Code* BundleImporter::translate_function_body(llvm::Function* fun) {
  ar::Function* ar_fun = this->translate_function(fun);
  ikos_assert(ar_fun->is_definition());

  FunctionImporter function_imp(_ctx, fun, ar_fun);
  return function_imp.translate_body();
}

} // end namespace import
} // end namespace frontend
} // end namespace ikos
