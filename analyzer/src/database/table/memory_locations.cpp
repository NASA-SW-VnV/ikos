/*******************************************************************************
 *
 * \file
 * \brief MemoryLocationsTable implementation
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2018 United States Government as represented by the
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

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/Transforms/Utils/Local.h>

#include <ikos/analyzer/database/table/memory_locations.hpp>
#include <ikos/analyzer/database/table/operands.hpp>
#include <ikos/analyzer/support/cast.hpp>
#include <ikos/analyzer/util/demangle.hpp>

namespace ikos {
namespace analyzer {

MemoryLocationsTable::MemoryLocationsTable(sqlite::DbConnection& db,
                                           FunctionsTable& functions,
                                           StatementsTable& statements,
                                           CallContextsTable& call_contexts)
    : DatabaseTable(db,
                    "memory_locations",
                    {{"id", sqlite::DbColumnType::Integer},
                     {"kind", sqlite::DbColumnType::Integer},
                     {"info", sqlite::DbColumnType::Text}},
                    {}),
      _functions(functions),
      _statements(statements),
      _call_contexts(call_contexts),
      _row(db, "memory_locations", 3) {}

sqlite::DbInt64 MemoryLocationsTable::insert(MemoryLocation* mem_loc) {
  ikos_assert(mem_loc != nullptr);

  auto it = this->_map.find(mem_loc);
  if (it != this->_map.end()) {
    return it->second;
  }

  sqlite::DbInt64 id = this->_last_insert_id++;

  this->_row << id;
  this->_row << static_cast< sqlite::DbInt64 >(mem_loc->kind());
  JsonDict info = this->info(mem_loc);
  if (!info.empty()) {
    this->_row << info.str();
  } else {
    this->_row << sqlite::null;
  }
  this->_row << sqlite::end_row;

  this->_map.try_emplace(mem_loc, id);
  return id;
}

JsonDict MemoryLocationsTable::info(MemoryLocation* mem_loc) {
  if (auto local_mem_loc = dyn_cast< LocalMemoryLocation >(mem_loc)) {
    ar::LocalVariable* lv = local_mem_loc->local_var();
    ikos_assert(lv->has_frontend());
    auto value = lv->frontend< llvm::Value >();
    auto alloca = llvm::cast< llvm::AllocaInst >(value);

    // Check for llvm.dbg.declare and llvm.dbg.addr
    llvm::TinyPtrVector< llvm::DbgInfoIntrinsic* > dbg_addrs =
        llvm::FindDbgAddrUses(alloca);

    if (!dbg_addrs.empty()) {
      llvm::DILocalVariable* di_var = dbg_addrs.front()->getVariable();
      return {{"name", di_var->getName()}};
    }

    // Check for llvm.dbg.value
    llvm::SmallVector< llvm::DbgValueInst*, 1 > dbg_values;
    llvm::findDbgValues(dbg_values, value);

    if (!dbg_values.empty()) {
      llvm::DILocalVariable* di_var = dbg_values.front()->getVariable();
      return {{"name", di_var->getName()}};
    }

    // Last chance, use ar variable name
    if (lv->has_name()) {
      return {{"name", lv->name()}};
    }

    return {};
  } else if (auto global_mem_loc = dyn_cast< GlobalMemoryLocation >(mem_loc)) {
    ar::GlobalVariable* gv = global_mem_loc->global_var();
    ikos_assert(gv->has_frontend());
    auto llvm_gv = gv->frontend< llvm::GlobalVariable >();

    // Check for debug info
    llvm::SmallVector< llvm::DIGlobalVariableExpression*, 1 > dbgs;
    llvm_gv->getDebugInfo(dbgs);

    if (!dbgs.empty()) {
      llvm::DIGlobalVariable* di_gv = dbgs[0]->getVariable();
      return {{"name", di_gv->getName()}};
    }

    // If it's a constant (e.g, a string)
    if (llvm_gv->isConstant() && llvm_gv->hasInitializer()) {
      return {{"cst", OperandsTable::repr(llvm_gv->getInitializer())}};
    }

    // Last chance, use ar variable name
    const std::string& name = gv->name();
    if (is_mangled(name)) {
      return {{"name", name}, {"demangle", demangle(name)}};
    } else {
      return {{"name", name}};
    }
  } else if (auto fun_mem_loc = dyn_cast< FunctionMemoryLocation >(mem_loc)) {
    ar::Function* fun = fun_mem_loc->function();
    return {{"id", this->_functions.insert(fun)}};
  } else if (isa< AggregateMemoryLocation >(mem_loc)) {
    return {};
  } else if (isa< VaArgMemoryLocation >(mem_loc)) {
    return {};
  } else if (isa< AbsoluteZeroMemoryLocation >(mem_loc)) {
    return {};
  } else if (isa< ArgvMemoryLocation >(mem_loc)) {
    return {};
  } else if (auto dyn_alloc_mem_loc =
                 dyn_cast< DynAllocMemoryLocation >(mem_loc)) {
    ar::CallBase* call = dyn_alloc_mem_loc->call();
    CallContext* context = dyn_alloc_mem_loc->context();
    return {{"call_id", this->_statements.insert(call)},
            {"context_id", this->_call_contexts.insert(context)}};
  } else {
    ikos_unreachable("unexpected memory location");
  }
}

} // end namespace analyzer
} // end namespace ikos
