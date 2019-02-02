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

#pragma once

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/code.hpp>
#include <ikos/ar/semantic/context.hpp>
#include <ikos/ar/semantic/statement.hpp>
#include <ikos/ar/semantic/type.hpp>
#include <ikos/ar/semantic/value.hpp>

#include "import_context.hpp"

namespace ikos {
namespace frontend {
namespace import {

/// \brief Helper class to translate constants
class ConstantImporter {
private:
  // Import context
  ImportContext& _ctx;

  // AR context
  ar::Context& _context;

  // AR bundle
  ar::Bundle* _bundle;

  // LLVM data layout
  const llvm::DataLayout& _llvm_data_layout;

  // Map from LLVM Constant + AR type to AR value
  llvm::DenseMap< std::pair< llvm::Constant*, ar::Type* >, ar::Value* >
      _constants;

public:
  /// \brief Public constructor
  explicit ConstantImporter(ImportContext& ctx)
      : _ctx(ctx),
        _context(ctx.ar_context),
        _bundle(ctx.bundle),
        _llvm_data_layout(ctx.llvm_data_layout) {}

public:
  /// \brief Translate a llvm::Constant into an ar::Value
  ///
  /// This will add statements at the end of the basic block if the
  /// llvm::Constant contains constant expressions (see llvm::ConstantExpr).
  ///
  /// Note: If non-null, the `type` parameter should be compatible with the
  /// constant: `TypeImporter::match_ar_type(cst->getType(), type)` should be
  /// true.
  ///
  /// \param cst The constant to translate
  /// \param type The required ar::Type, or null if no specific type is needed
  /// \param bb The basic block that will use the constant
  ar::Value* translate_constant(llvm::Constant* cst,
                                ar::Type* type,
                                ar::BasicBlock* bb);

private:
  /*
   * Implementation of translation of llvm::Constant
   */

  struct ConstantExpression {
    ar::InternalVariable* var;
    llvm::Constant* expr;
  };

  using ConstantExpressionList = llvm::SmallVectorImpl< ConstantExpression >;

private:
  /// \brief Translate a llvm::Constant into an ar::Value
  ///
  /// This also produces a list of llvm::ConstantExpr that needs to be
  /// translated into ar::Statement
  ///
  /// \param cst The constant to translate
  /// \param type The required ar::Type, or null if no specific type is needed
  /// \param bb The basic block that will use the constant
  /// \param exprs The list of llvm::ConstantExpr to handle later
  ar::Value* translate_constant(llvm::Constant* cst,
                                ar::Type* type,
                                ar::BasicBlock* bb,
                                ConstantExpressionList& exprs);

  /// \brief Translate a llvm::ConstantInt into an ar::IntegerConstant
  ar::IntegerConstant* translate_constant_int(llvm::ConstantInt* cst,
                                              ar::IntegerType* type);

  /// \brief Translate a llvm::ConstantFP into ar ar::FloatConstant
  ar::FloatConstant* translate_constant_fp(llvm::ConstantFP* cst,
                                           ar::FloatType* type);

  /// \brief Translate a llvm::ConstantPointerNull into an ar::NullConstant
  ar::NullConstant* translate_constant_ptr_null(llvm::ConstantPointerNull* cst,
                                                ar::PointerType* type);

  /// \brief Translate a llvm::UndefValue into an ar::UndefinedConstant
  ar::UndefinedConstant* translate_constant_undef(llvm::UndefValue* cst,
                                                  ar::Type* type);

  /// \brief Translate a llvm::ConstantAggregateZero into an
  /// ar::AggregateZeroConstant
  ar::AggregateZeroConstant* translate_constant_agg_zero(
      llvm::ConstantAggregateZero* cst, ar::AggregateType* type);

  /// \brief Translate a llvm::ConstantArray into an ar::ArrayConstant
  ar::ArrayConstant* translate_constant_array(llvm::ConstantArray* cst,
                                              ar::ArrayType* type,
                                              ar::BasicBlock* bb,
                                              ConstantExpressionList& exprs);

  /// \brief Translate a llvm::ConstantStruct into an ar::StructConstant
  ar::StructConstant* translate_constant_struct(llvm::ConstantStruct* cst,
                                                ar::StructType* type,
                                                ar::BasicBlock* bb,
                                                ConstantExpressionList& exprs);

  /// \brief Translate a llvm::ConstantVector into an ar::VectorConstant
  ar::VectorConstant* translate_constant_vector(llvm::ConstantVector* cst,
                                                ar::VectorType* type,
                                                ar::BasicBlock* bb,
                                                ConstantExpressionList& exprs);

  /// \brief Translate a llvm::ConstantDataArray into an ar::ArrayConstant
  ar::ArrayConstant* translate_constant_data_array(
      llvm::ConstantDataArray* cst,
      ar::ArrayType* type,
      ar::BasicBlock* bb,
      ConstantExpressionList& exprs);

  /// \brief Translate a llvm::ConstantDataVector into an ar::VectorConstant
  ar::VectorConstant* translate_constant_data_vector(
      llvm::ConstantDataVector* cst,
      ar::VectorType* type,
      ar::BasicBlock* bb,
      ConstantExpressionList& exprs);

  /// \brief Translate a llvm::GlobalAlias into an ar::Value
  ar::Value* translate_global_alias(llvm::GlobalAlias* cst,
                                    ar::Type* type,
                                    ar::BasicBlock* bb,
                                    ConstantExpressionList& exprs);

  /// \brief Translate a llvm::ConstantExpr* into a ar::InternalVariable
  ar::InternalVariable* translate_constant_expr_to_var(
      llvm::ConstantExpr* cst,
      ar::Type* type,
      ar::BasicBlock* bb,
      ConstantExpressionList& exprs);

  /// \brief Translate a llvm::GlobalVariable into an ar::Value
  ar::Value* translate_global_variable(llvm::GlobalVariable* gv,
                                       ar::Type* type,
                                       ar::BasicBlock* bb,
                                       ConstantExpressionList& exprs);

  /// \brief Translate a llvm::Function into an ar::Value
  ar::Value* translate_function(llvm::Function* fun,
                                ar::Type* type,
                                ar::BasicBlock* bb,
                                ConstantExpressionList& exprs);

private:
  /*
   * Implementation of translation of llvm::ConstantExpr
   */

  /// \brief Translate a llvm::ConstantExpr into an ar::Statement
  std::unique_ptr< ar::Statement > translate_constant_expr_to_stmt(
      ar::InternalVariable* result,
      llvm::Constant* cst,
      ar::BasicBlock* bb,
      ConstantExpressionList& exprs);

  /// \brief Create a cast from a llvm::GlobalVariable to an
  /// ar::InternalVariable
  std::unique_ptr< ar::UnaryOperation > translate_global_variable_cast(
      ar::InternalVariable* result, llvm::GlobalVariable* gv);

  /// \brief Create a cast from a llvm::Function to an ar::InternalVariable
  std::unique_ptr< ar::UnaryOperation > translate_function_ptr_cast(
      ar::InternalVariable* result, llvm::Function* fun);

  /// \brief Translate a llvm::ConstantExpr into an ar::PointerShift
  std::unique_ptr< ar::PointerShift > translate_getelementptr(
      ar::InternalVariable* result,
      llvm::GetElementPtrInst* gep,
      ar::BasicBlock* bb,
      ConstantExpressionList& exprs);

  /// \brief Translate a llvm::ConstantExpr into an ar::UnaryOperation
  std::unique_ptr< ar::UnaryOperation > translate_bitcast(
      ar::InternalVariable* result,
      llvm::BitCastInst* inst,
      ar::BasicBlock* bb,
      ConstantExpressionList& exprs);

  /// \brief Translate a llvm::ConstantExpr into an ar::UnaryOperation
  std::unique_ptr< ar::UnaryOperation > translate_inttoptr(
      ar::InternalVariable* result,
      llvm::IntToPtrInst* inst,
      ar::BasicBlock* bb,
      ConstantExpressionList& exprs);

  /// \brief Translate a llvm::ConstantExpr into an ar::UnaryOperation
  std::unique_ptr< ar::UnaryOperation > translate_ptrtoint(
      ar::InternalVariable* result,
      llvm::PtrToIntInst* inst,
      ar::BasicBlock* bb,
      ConstantExpressionList& exprs);

public:
  /// \brief Translate an integer llvm::Constant into an ar::Value and cast it
  /// to the given ar::IntegerType
  ///
  /// This will throw an exception if the constant is a llvm::ConstantExpr.
  ///
  /// \param cst The constant to translate
  /// \param type The required ar::IntegerType
  ar::Value* translate_cast_integer_constant(llvm::Constant* cst,
                                             ar::IntegerType* type);

}; // end class ConstantImporter

} // end namespace import
} // end namespace frontend
} // end namespace ikos
