/*******************************************************************************
 *
 * \file
 * \brief Translate LLVM constants into AR values
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

#include <llvm/ADT/SmallString.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GetElementPtrTypeIterator.h>
#include <llvm/IR/GlobalAlias.h>
#include <llvm/IR/GlobalVariable.h>

#include <ikos/ar/support/cast.hpp>

#include <ikos/frontend/llvm/import/exception.hpp>

#include "bundle.hpp"
#include "constant.hpp"
#include "type.hpp"

namespace ikos {
namespace frontend {
namespace import {

/// \brief Convert a llvm::APInt with the given signedness into an
/// ar::MachineInt
static ar::MachineInt to_machine_int(const llvm::APInt& n,
                                     ar::Signedness sign) {
  if (n.getBitWidth() <= 64) {
    if (sign == ar::Signed) {
      return ar::MachineInt(n.getSExtValue(), n.getBitWidth(), sign);
    } else {
      return ar::MachineInt(n.getZExtValue(), n.getBitWidth(), sign);
    }
  } else {
    llvm::SmallString< 16 > str;
    n.toString(str, /*radix = */ 10, /*signed = */ (sign == ar::Signed));
    return ar::MachineInt(ar::ZNumber::from_string(str.c_str(), /*base = */ 10),
                          n.getBitWidth(),
                          sign);
  }
}

ar::Value* ConstantImporter::translate_constant(llvm::Constant* cst,
                                                ar::Type* type,
                                                ar::BasicBlock* bb) {
  // List of constant expressions to handle
  llvm::SmallVector< ConstantExpression, 4 > exprs;

  // Translate the constant and fill exprs
  ar::Value* ar_cst = this->translate_constant(cst, type, bb, exprs);

  if (!exprs.empty()) {
    llvm::SmallVector< std::unique_ptr< ar::Statement >, 4 > stmts;

    // Build the statements
    for (std::size_t i = 0; i < exprs.size(); i++) {
      const ConstantExpression& cst_expr = exprs[i];
      std::unique_ptr< ar::Statement > stmt =
          this->translate_constant_expr_to_stmt(cst_expr.var,
                                                cst_expr.expr,
                                                bb,
                                                exprs);
      stmt->set_frontend< llvm::Value >(cst_expr.expr);
      stmts.push_back(std::move(stmt));
    }

    // Insert all the statements, in reverse order
    while (!stmts.empty()) {
      bb->push_back(std::move(stmts.back()));
      stmts.pop_back();
    }
  }

  return ar_cst;
}

ar::Value* ConstantImporter::translate_constant(llvm::Constant* cst,
                                                ar::Type* type,
                                                ar::BasicBlock* bb,
                                                ConstantExpressionList& exprs) {
  auto it = this->_constants.find({cst, type});

  if (it != this->_constants.end()) {
    return it->second;
  }

  ar::Value* ar_cst = nullptr;
  ar::Type* orig_type = type;

  if (llvm::isa< llvm::BlockAddress >(cst)) {
    throw ImportError("llvm blockaddress is not supported");
  } else if (llvm::isa< llvm::ConstantTokenNone >(cst)) {
    throw ImportError("llvm token 'none' is not supported");
  } else if (llvm::isa< llvm::GlobalIFunc >(cst)) {
    throw ImportError("indirect functions (ifunc) are not supported");
  } else if (auto gv_alias = llvm::dyn_cast< llvm::GlobalAlias >(cst)) {
    ar_cst = this->translate_global_alias(gv_alias, type, bb, exprs);
  } else if (auto gv = llvm::dyn_cast< llvm::GlobalVariable >(cst)) {
    ar_cst = this->translate_global_variable(gv, type, bb, exprs);
  } else if (auto fun = llvm::dyn_cast< llvm::Function >(cst)) {
    ar_cst = this->translate_function(fun, type, bb, exprs);
  } else {
    // If no specific type is needed, just use translate_type(cst->getType())
    if (type == nullptr) {
      type = _ctx.type_imp->translate_type(cst->getType(), ar::Signed);
    }

    if (auto cst_int = llvm::dyn_cast< llvm::ConstantInt >(cst)) {
      ar_cst = this->translate_constant_int(cst_int,
                                            ar::cast< ar::IntegerType >(type));
    } else if (auto cst_fp = llvm::dyn_cast< llvm::ConstantFP >(cst)) {
      ar_cst =
          this->translate_constant_fp(cst_fp, ar::cast< ar::FloatType >(type));
    } else if (auto cst_null =
                   llvm::dyn_cast< llvm::ConstantPointerNull >(cst)) {
      ar_cst =
          this->translate_constant_ptr_null(cst_null,
                                            ar::cast< ar::PointerType >(type));
    } else if (auto cst_undef = llvm::dyn_cast< llvm::UndefValue >(cst)) {
      ar_cst = this->translate_constant_undef(cst_undef, type);
    } else if (auto cst_agg_zero =
                   llvm::dyn_cast< llvm::ConstantAggregateZero >(cst)) {
      ar_cst = this->translate_constant_agg_zero(cst_agg_zero,
                                                 ar::cast< ar::AggregateType >(
                                                     type));
    } else if (auto cst_array = llvm::dyn_cast< llvm::ConstantArray >(cst)) {
      ar_cst = this->translate_constant_array(cst_array,
                                              ar::cast< ar::ArrayType >(type),
                                              bb,
                                              exprs);
    } else if (auto cst_struct = llvm::dyn_cast< llvm::ConstantStruct >(cst)) {
      ar_cst = this->translate_constant_struct(cst_struct,
                                               ar::cast< ar::StructType >(type),
                                               bb,
                                               exprs);
    } else if (auto cst_vector = llvm::dyn_cast< llvm::ConstantVector >(cst)) {
      ar_cst = this->translate_constant_vector(cst_vector,
                                               ar::cast< ar::VectorType >(type),
                                               bb,
                                               exprs);
    } else if (auto cst_data_array =
                   llvm::dyn_cast< llvm::ConstantDataArray >(cst)) {
      ar_cst =
          this->translate_constant_data_array(cst_data_array,
                                              ar::cast< ar::ArrayType >(type),
                                              bb,
                                              exprs);
    } else if (auto cst_data_vector =
                   llvm::dyn_cast< llvm::ConstantDataVector >(cst)) {
      ar_cst =
          this->translate_constant_data_vector(cst_data_vector,
                                               ar::cast< ar::VectorType >(type),
                                               bb,
                                               exprs);
    } else if (auto cst_expr = llvm::dyn_cast< llvm::ConstantExpr >(cst)) {
      ar_cst = this->translate_constant_expr_to_var(cst_expr, type, bb, exprs);
    } else {
      throw ImportError("unexpected llvm constant [1]");
    }
  }

  ikos_assert(ar_cst);
  if (exprs.empty()) {
    // only if the constant is free of llvm::ConstantExpr
    this->_constants.try_emplace({cst, orig_type}, ar_cst);
  }
  return ar_cst;
}

ar::IntegerConstant* ConstantImporter::translate_constant_int(
    llvm::ConstantInt* cst, ar::IntegerType* type) {
  ar::MachineInt n = to_machine_int(cst->getValue(), type->sign());
  return ar::IntegerConstant::get(this->_context, type, n);
}

ar::FloatConstant* ConstantImporter::translate_constant_fp(
    llvm::ConstantFP* cst, ar::FloatType* type) {
  const llvm::APFloat& f = cst->getValueAPF();
  llvm::SmallString< 16 > str;
  f.toString(str, /*FormatPrecision = */ 0, /*FormatMaxPadding = */ 0);
  return ar::FloatConstant::get(this->_context, type, str.c_str());
}

ar::NullConstant* ConstantImporter::translate_constant_ptr_null(
    llvm::ConstantPointerNull* /*cst*/, ar::PointerType* type) {
  return ar::NullConstant::get(this->_context, type);
}

ar::UndefinedConstant* ConstantImporter::translate_constant_undef(
    llvm::UndefValue* /*cst*/, ar::Type* type) {
  return ar::UndefinedConstant::get(this->_context, type);
}

ar::AggregateZeroConstant* ConstantImporter::translate_constant_agg_zero(
    llvm::ConstantAggregateZero* /*cst*/, ar::AggregateType* type) {
  return ar::AggregateZeroConstant::get(this->_context, type);
}

ar::ArrayConstant* ConstantImporter::translate_constant_array(
    llvm::ConstantArray* cst,
    ar::ArrayType* type,
    ar::BasicBlock* bb,
    ConstantExpressionList& exprs) {
  ar::ArrayConstant::Values values;
  ikos_assert(cst->getNumOperands() == type->num_elements());
  values.reserve(cst->getNumOperands());

  for (auto it = cst->op_begin(), et = cst->op_end(); it != et; ++it) {
    auto element = llvm::cast< llvm::Constant >(*it);
    ar::Value* ar_element =
        this->translate_constant(element, type->element_type(), bb, exprs);
    values.push_back(ar_element);
  }

  return ar::ArrayConstant::get(this->_context, type, values);
}

ar::StructConstant* ConstantImporter::translate_constant_struct(
    llvm::ConstantStruct* cst,
    ar::StructType* type,
    ar::BasicBlock* bb,
    ConstantExpressionList& exprs) {
  ar::StructConstant::Values fields;
  ikos_assert(cst->getNumOperands() == type->num_fields());
  fields.reserve(cst->getNumOperands());

  auto op_it = cst->op_begin();
  auto op_et = cst->op_end();
  auto type_it = type->field_begin();
  auto type_et = type->field_end();
  for (; op_it != op_et && type_it != type_et; ++op_it, ++type_it) {
    auto element = llvm::cast< llvm::Constant >(*op_it);
    ar::Value* ar_element =
        this->translate_constant(element, type_it->type, bb, exprs);
    fields.push_back({type_it->offset, ar_element});
  }

  return ar::StructConstant::get(this->_context, type, fields);
}

ar::VectorConstant* ConstantImporter::translate_constant_vector(
    llvm::ConstantVector* cst,
    ar::VectorType* type,
    ar::BasicBlock* bb,
    ConstantExpressionList& exprs) {
  ar::VectorConstant::Values values;
  values.reserve(cst->getNumOperands());

  for (auto it = cst->op_begin(), et = cst->op_end(); it != et; ++it) {
    auto element = llvm::cast< llvm::Constant >(*it);
    ar::Value* ar_element =
        this->translate_constant(element, type->element_type(), bb, exprs);
    values.push_back(ar_element);
  }

  return ar::VectorConstant::get(this->_context, type, values);
}

ar::ArrayConstant* ConstantImporter::translate_constant_data_array(
    llvm::ConstantDataArray* cst,
    ar::ArrayType* type,
    ar::BasicBlock* bb,
    ConstantExpressionList& exprs) {
  ar::ArrayConstant::Values values;
  ikos_assert(cst->getNumElements() == type->num_elements());
  values.reserve(cst->getNumElements());

  for (unsigned i = 0; i < cst->getNumElements(); i++) {
    llvm::Constant* element = cst->getElementAsConstant(i);
    ar::Value* ar_element =
        this->translate_constant(element, type->element_type(), bb, exprs);
    values.push_back(ar_element);
  }

  return ar::ArrayConstant::get(this->_context, type, values);
}

ar::VectorConstant* ConstantImporter::translate_constant_data_vector(
    llvm::ConstantDataVector* cst,
    ar::VectorType* type,
    ar::BasicBlock* bb,
    ConstantExpressionList& exprs) {
  ar::VectorConstant::Values values;
  ikos_assert(cst->getNumElements() == type->num_elements());
  values.reserve(cst->getNumElements());

  for (unsigned i = 0; i < cst->getNumElements(); i++) {
    llvm::Constant* element = cst->getElementAsConstant(i);
    ar::Value* ar_element =
        this->translate_constant(element, type->element_type(), bb, exprs);
    values.push_back(ar_element);
  }

  return ar::VectorConstant::get(this->_context, type, values);
}

ar::Value* ConstantImporter::translate_global_alias(
    llvm::GlobalAlias* cst,
    ar::Type* type,
    ar::BasicBlock* bb,
    ConstantExpressionList& exprs) {
  return this->translate_constant(cst->getAliasee(), type, bb, exprs);
}

ar::InternalVariable* ConstantImporter::translate_constant_expr_to_var(
    llvm::ConstantExpr* cst,
    ar::Type* type,
    ar::BasicBlock* bb,
    ConstantExpressionList& exprs) {
  // Create an internal variable containing the result of the ConstantExpr
  ar::InternalVariable* iv = ar::InternalVariable::create(bb->code(), type);

  // Set the origin of the variable
  iv->set_frontend< llvm::Value >(cst);

  // Add it in the list
  exprs.push_back({iv, cst});

  return iv;
}

ar::Value* ConstantImporter::translate_global_variable(
    llvm::GlobalVariable* gv,
    ar::Type* type,
    ar::BasicBlock* bb,
    ConstantExpressionList& exprs) {
  ar::GlobalVariable* ar_gv = _ctx.bundle_imp->translate_global_variable(gv);

  if (type == nullptr || ar_gv->type() == type) {
    return ar_gv;
  } else {
    // Add a cast from ar_gv->type() to type

    // Create an internal variable containing the result of the cast
    ar::InternalVariable* iv = ar::InternalVariable::create(bb->code(), type);

    // Set the origin of the variable
    iv->set_frontend< llvm::Value >(gv);

    // Add it in the list
    exprs.push_back({iv, gv});

    return iv;
  }
}

ar::Value* ConstantImporter::translate_function(llvm::Function* fun,
                                                ar::Type* type,
                                                ar::BasicBlock* bb,
                                                ConstantExpressionList& exprs) {
  ar::Function* ar_fun = _ctx.bundle_imp->translate_function(fun);
  ikos_assert(ar_fun != nullptr);

  ar::FunctionPointerConstant* ar_fun_ptr = ar_fun->pointer();

  if (type == nullptr || ar_fun_ptr->type() == type) {
    return ar_fun_ptr;
  } else {
    // Add a cast from ar_fun_ptr->type() to type

    // Create an internal variable containing the result of the cast
    ar::InternalVariable* iv = ar::InternalVariable::create(bb->code(), type);

    // Set the origin of the variable
    iv->set_frontend< llvm::Value >(fun);

    // Add it in the list
    exprs.push_back({iv, fun});

    return iv;
  }
}

std::unique_ptr< ar::Statement > ConstantImporter::
    translate_constant_expr_to_stmt(ar::InternalVariable* result,
                                    llvm::Constant* cst,
                                    ar::BasicBlock* bb,
                                    ConstantExpressionList& exprs) {
  if (auto gv = llvm::dyn_cast< llvm::GlobalVariable >(cst)) {
    return this->translate_global_variable_cast(result, gv);
  } else if (auto fun = llvm::dyn_cast< llvm::Function >(cst)) {
    return this->translate_function_ptr_cast(result, fun);
  } else if (auto expr = llvm::dyn_cast< llvm::ConstantExpr >(cst)) {
    // We only need to support constant expressions that appear in initializer
    // of global variables, because we assume the user lowered down
    // all constant expressions as instructions (using the lower-cst-expr pass)
    auto inst_deleter = [](llvm::Instruction* inst) { inst->deleteValue(); };
    std::unique_ptr< llvm::Instruction, decltype(inst_deleter) >
        inst(expr->getAsInstruction(), inst_deleter);

    if (auto gep = llvm::dyn_cast< llvm::GetElementPtrInst >(inst.get())) {
      return this->translate_getelementptr(result, gep, bb, exprs);
    } else if (auto bitcast = llvm::dyn_cast< llvm::BitCastInst >(inst.get())) {
      return this->translate_bitcast(result, bitcast, bb, exprs);
    } else if (auto inttoptr =
                   llvm::dyn_cast< llvm::IntToPtrInst >(inst.get())) {
      return this->translate_inttoptr(result, inttoptr, bb, exprs);
    } else if (auto ptrtoint =
                   llvm::dyn_cast< llvm::PtrToIntInst >(inst.get())) {
      return this->translate_ptrtoint(result, ptrtoint, bb, exprs);
    } else {
      throw ImportError("unexpected llvm constant expression");
    }
  } else {
    throw ImportError("unexpected llvm constant [2]");
  }
}

std::unique_ptr< ar::UnaryOperation > ConstantImporter::
    translate_global_variable_cast(ar::InternalVariable* result,
                                   llvm::GlobalVariable* gv) {
  // Create a cast statement from ar_gv->type() to result->type()
  ar::GlobalVariable* ar_gv = _ctx.bundle_imp->translate_global_variable(gv);
  return ar::UnaryOperation::create(ar::UnaryOperation::Bitcast, result, ar_gv);
}

std::unique_ptr< ar::UnaryOperation > ConstantImporter::
    translate_function_ptr_cast(ar::InternalVariable* result,
                                llvm::Function* fun) {
  // Create a cast statement from ar_fun_ptr->type() to result->type()
  ar::Function* ar_fun = _ctx.bundle_imp->translate_function(fun);
  ikos_assert(ar_fun != nullptr);

  ar::FunctionPointerConstant* ar_fun_ptr = ar_fun->pointer();

  return ar::UnaryOperation::create(ar::UnaryOperation::Bitcast,
                                    result,
                                    ar_fun_ptr);
}

std::unique_ptr< ar::PointerShift > ConstantImporter::translate_getelementptr(
    ar::InternalVariable* result,
    llvm::GetElementPtrInst* gep,
    ar::BasicBlock* bb,
    ConstantExpressionList& exprs) {
  // Translate base
  auto pointer = llvm::cast< llvm::Constant >(gep->getPointerOperand());
  ar::Value* ar_pointer = this->translate_constant(pointer, nullptr, bb, exprs);

  // Translate operands
  std::vector< ar::PointerShift::Term > terms;
  terms.reserve(gep->getNumOperands() - 1);

  ar::IntegerType* size_type = ar::IntegerType::size_type(this->_bundle);
  for (auto it = llvm::gep_type_begin(gep), et = llvm::gep_type_end(gep);
       it != et;
       ++it) {
    auto op = llvm::cast< llvm::Constant >(it.getOperand());

    if (llvm::StructType* struct_type = it.getStructTypeOrNull()) {
      // Shift to get a struct field
      llvm::APInt value = llvm::cast< llvm::ConstantInt >(op)->getValue();
      ikos_assert(value.getBitWidth() <= 64 &&
                  value.getZExtValue() <=
                      std::numeric_limits< unsigned >::max());
      auto uint_value = static_cast< unsigned >(value.getZExtValue());
      uint64_t offset = this->_llvm_data_layout.getStructLayout(struct_type)
                            ->getElementOffset(uint_value);

      ar::IntegerConstant* ar_op =
          ar::IntegerConstant::get(this->_context,
                                   size_type,
                                   ar::MachineInt(offset,
                                                  size_type->bit_width(),
                                                  size_type->sign()));
      terms.emplace_back(ar::MachineInt(1,
                                        size_type->bit_width(),
                                        size_type->sign()),
                         ar_op);
    } else {
      // Shift in a sequential type
      uint64_t size =
          this->_llvm_data_layout.getTypeAllocSize(it.getIndexedType());
      ar::Value* ar_op = this->translate_constant(op, nullptr, bb, exprs);
      terms.emplace_back(ar::MachineInt(size,
                                        size_type->bit_width(),
                                        size_type->sign()),
                         ar_op);
    }
  }

  return ar::PointerShift::create(result, ar_pointer, terms);
}

std::unique_ptr< ar::UnaryOperation > ConstantImporter::translate_bitcast(
    ar::InternalVariable* result,
    llvm::BitCastInst* inst,
    ar::BasicBlock* bb,
    ConstantExpressionList& exprs) {
  auto op = llvm::cast< llvm::Constant >(inst->getOperand(0));
  ar::Value* ar_op = this->translate_constant(op, nullptr, bb, exprs);
  return ar::UnaryOperation::create(ar::UnaryOperation::Bitcast, result, ar_op);
}

std::unique_ptr< ar::UnaryOperation > ConstantImporter::translate_inttoptr(
    ar::InternalVariable* result,
    llvm::IntToPtrInst* inst,
    ar::BasicBlock* bb,
    ConstantExpressionList& exprs) {
  auto op = llvm::cast< llvm::Constant >(inst->getOperand(0));
  ar::Value* ar_op = this->translate_constant(op, nullptr, bb, exprs);
  auto type = ar::cast< ar::IntegerType >(ar_op->type());
  return ar::UnaryOperation::create(type->is_signed()
                                        ? ar::UnaryOperation::SIToPtr
                                        : ar::UnaryOperation::UIToPtr,
                                    result,
                                    ar_op);
}

std::unique_ptr< ar::UnaryOperation > ConstantImporter::translate_ptrtoint(
    ar::InternalVariable* result,
    llvm::PtrToIntInst* inst,
    ar::BasicBlock* bb,
    ConstantExpressionList& exprs) {
  auto op = llvm::cast< llvm::Constant >(inst->getOperand(0));
  ar::Value* ar_op = this->translate_constant(op, nullptr, bb, exprs);
  auto type = ar::cast< ar::IntegerType >(result->type());
  return ar::UnaryOperation::create(type->is_signed()
                                        ? ar::UnaryOperation::PtrToSI
                                        : ar::UnaryOperation::PtrToUI,
                                    result,
                                    ar_op);
}

ar::Value* ConstantImporter::translate_cast_integer_constant(
    llvm::Constant* cst, ar::IntegerType* type) {
  ikos_assert(type != nullptr);

  auto it = this->_constants.find({cst, type});

  if (it != this->_constants.end()) {
    return it->second;
  }

  ar::Value* ar_cst = nullptr;

  if (auto cst_int = llvm::dyn_cast< llvm::ConstantInt >(cst)) {
    ar::MachineInt n = to_machine_int(cst_int->getValue(), type->sign());
    ar::MachineInt m = n.cast(type->bit_width(), type->sign());
    ar_cst = ar::IntegerConstant::get(this->_context, type, m);
  } else if (llvm::isa< llvm::UndefValue >(cst)) {
    ar_cst = ar::UndefinedConstant::get(this->_context, type);
  } else {
    throw ImportError("unexpected llvm constant [3]");
  }

  ikos_assert(ar_cst != nullptr);
  this->_constants.try_emplace({cst, type}, ar_cst);
  return ar_cst;
}

} // end namespace import
} // end namespace frontend
} // end namespace ikos
