/*******************************************************************************
 *
 * \file
 * \brief Translate LLVM functions into AR functions
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

#include <boost/container/flat_map.hpp>

#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Module.h>

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/function.hpp>
#include <ikos/ar/semantic/statement.hpp>

#include "import_context.hpp"

namespace ikos {
namespace frontend {
namespace import {

/// \brief Represents the output of a llvm::BasicBlock
struct BasicBlockOutput {
  /// \brief AR basic block
  ar::BasicBlock* block;

  /// \brief LLVM successor
  llvm::BasicBlock* succ;

  /// \brief Constructor
  explicit BasicBlockOutput(ar::BasicBlock* block_)
      : block(block_), succ(nullptr) {}

  /// \brief Constructor
  BasicBlockOutput(ar::BasicBlock* block_, llvm::BasicBlock* succ_)
      : block(block_), succ(succ_) {}

}; // end struct BasicBlockOutput

/// \brief Represents the translation of a llvm::BasicBlock
struct BasicBlockTranslation {
  /// \brief LLVM basic block corresponding to this BasicBlockTranslation
  llvm::BasicBlock* source;

  /// \brief AR main basic block
  ///
  /// This basic block represents the llvm::BasicBlock in AR.
  ///
  /// Either it is the first basic block, or it is the successor of all the
  /// input basic blocks.
  ar::BasicBlock* main;

  /// \brief Map from LLVM predecessors to AR basic block
  ///
  /// This is only necessary to translate PHI nodes.
  ///
  /// These should be linked (set the predecessor) at the end of the
  /// translation.
  boost::container::flat_map< llvm::BasicBlock*, ar::BasicBlock* > inputs;

  /// \brief Internal basic blocks
  ///
  /// These are already linked (predecessors/successors) correctly.
  ///
  /// This list should be topologically ordered.
  std::vector< ar::BasicBlock* > internals;

  /// \brief Output basic blocks
  ///
  /// These should be linked (set the successor) at the end of the translation
  ///
  /// There is no specific order in this list.
  ///
  /// This should never be empty.
  std::vector< BasicBlockOutput > outputs;

  /// \brief Constructor
  BasicBlockTranslation(llvm::BasicBlock* source_, ar::BasicBlock* main_);

  /// \brief Get or create an input basic block for the given llvm BasicBlock
  ar::BasicBlock* input_basic_block(llvm::BasicBlock* bb);

  /// \brief Create an output basic block
  ar::BasicBlock* add_output_basic_block(ar::BasicBlock* ar_src,
                                         llvm::BasicBlock* llvm_dest = nullptr);

  /// \brief Merge all output basic blocks into one basic block
  void merge_outputs();

  /// \brief Add a statement in the basic block
  void add_statement(std::unique_ptr< ar::Statement > stmt);

  /// \brief Add a llvm::CmpInst translated into a variable and a comparison
  void add_comparison(ar::InternalVariable* var,
                      std::unique_ptr< ar::Comparison > cmp);

private:
  /// \brief Add a new basic block with statements: cmp; var = value;
  void add_comparison_output_bb(ar::BasicBlock* src,
                                std::unique_ptr< ar::Statement > cmp,
                                ar::InternalVariable* var,
                                bool value);

public:
  /// \brief Add an unconditional branching
  void add_unconditional_branching(llvm::BranchInst* br,
                                   llvm::BasicBlock* succ);

  /// \brief Add a conditional branching
  void add_conditional_branching(llvm::BranchInst* br,
                                 ar::InternalVariable* cond);

  /// \brief Add a non-deterministic branching
  void add_nondeterministic_branching(llvm::BranchInst* br);

private:
  /// \brief Add an output basic block with a "cond == value" statement
  void add_conditional_output_bb(llvm::BranchInst* br,
                                 ar::BasicBlock* src,
                                 llvm::BasicBlock* llvm_dest,
                                 ar::InternalVariable* cond,
                                 bool value);

public:
  /// \brief Add an invoke branching
  void add_invoke_branching(llvm::BasicBlock* normal_dest,
                            llvm::BasicBlock* exception_dest);

private:
  /// \brief Add an output basic block for an invoke statement
  void add_invoke_normal_output_bb(ar::BasicBlock* src,
                                   ar::Invoke* invoke,
                                   llvm::BasicBlock* normal_dest);

  /// \brief Add an output basic block for an invoke statement
  void add_invoke_exception_output_bb(ar::BasicBlock* src,
                                      ar::Invoke* invoke,
                                      llvm::BasicBlock* exception_dest);

}; // end struct BasicBlockTranslation

/// \brief Helper class to translate the body of a function
class FunctionImporter {
private:
  // Import context
  ImportContext& _ctx;

  // AR context
  ar::Context& _context;

  // AR bundle
  ar::Bundle* _bundle;

  // LLVM data layout
  const llvm::DataLayout& _llvm_data_layout;

  // LLVM function
  llvm::Function* _llvm_fun;

  // AR function
  ar::Function* _ar_fun;

  // AR body
  ar::Code* _body;

  // Map from LLVM Value to AR Variable
  llvm::DenseMap< llvm::Value*, ar::Variable* > _variables;

  // Map from LLVM BasicBlock to BasicBlockTranslation
  llvm::DenseMap< llvm::BasicBlock*, std::unique_ptr< BasicBlockTranslation > >
      _blocks;

  // Allow mismatch of LLVM types (llvm::Type) and Debug Info types
  // (llvm::DIType)
  //
  // See `Importer::ImportOption`.
  bool _allow_debug_info_mismatch;

public:
  /// \brief Public constructor
  FunctionImporter(ImportContext& ctx,
                   llvm::Function* llvm_fun,
                   ar::Function* ar_fun)
      : _ctx(ctx),
        _context(ctx.ar_context),
        _bundle(ctx.bundle),
        _llvm_data_layout(ctx.llvm_data_layout),
        _llvm_fun(llvm_fun),
        _ar_fun(ar_fun),
        _body(ar_fun->body()),
        _allow_debug_info_mismatch(
            ctx.opts.test(Importer::AllowMismatchDebugInfo)) {}

  /// \brief Translate the body of the function
  ar::Code* translate_body();

private:
  /// \brief Store the mapping between a llvm::Value and an ar::Variable
  ///
  /// It also assigns a nice name to the ar::Variable and set the frontend
  /// object using ar::Variable::set_frontend.
  void mark_variable_mapping(llvm::Value*, ar::Variable*);

  /// \brief Translate LLVM parameters into ar::InternalVariable
  void translate_parameters();

  /// \brief Translate control flow graph
  ///
  /// This calls:
  ///   * translate_basic_blocks()
  ///   * translate_phi_nodes()
  ///   * link_basic_blocks()
  void translate_control_flow_graph();

  /// \brief Translate all basic blocks
  void translate_basic_blocks();

  /// \brief Translate a llvm::BasicBlock
  void translate_basic_block(llvm::BasicBlock* bb);

  /// \brief Translate all phi nodes once basic blocks are translated
  void translate_phi_nodes();

  /// \brief Translate all phi nodes in `bb` once basic blocks are translated
  void translate_phi_nodes(BasicBlockTranslation* bb_translation,
                           llvm::BasicBlock* bb);

  /// \brief Link all basic blocks
  void link_basic_blocks();

  /// \brief Link the given basic block
  void link_basic_block(BasicBlockTranslation* bb_translation);

  /// \brief Unify the exit blocks
  void unify_exit_blocks();

  /// \brief Translate a llvm::Instruction
  void translate_instruction(BasicBlockTranslation* bb_translation,
                             llvm::Instruction* inst);

  /// \brief Translate a llvm::AllocaInst
  void translate_alloca(BasicBlockTranslation* bb_translation,
                        llvm::AllocaInst* alloca);

  /// \brief Translate a llvm::StoreInst
  void translate_store(BasicBlockTranslation* bb_translation,
                       llvm::StoreInst* store);

  /// \brief Translate a llvm::LoadInst
  void translate_load(BasicBlockTranslation* bb_translation,
                      llvm::LoadInst* load);

  /// \brief Translate a llvm::CallInst
  void translate_call(BasicBlockTranslation* bb_translation,
                      llvm::CallInst* call);

  /// \brief Translate a llvm::IntrinsicInst
  void translate_intrinsic_call(BasicBlockTranslation* bb_translation,
                                llvm::IntrinsicInst* call);

  /// \brief Translate a llvm::InvokeInst
  void translate_invoke(BasicBlockTranslation* bb_translation,
                        llvm::InvokeInst* invoke);

  /// \brief Translate a llvm::CallInst or llvm::InvokeInst
  template < typename CallInstType, typename CreateStmtFun >
  void translate_call_helper(BasicBlockTranslation* bb_translation,
                             CallInstType* call,
                             bool force_return_cast,
                             bool force_args_cast,
                             CreateStmtFun create_stmt);

  /// \brief Translate a llvm::BitCastInst
  void translate_bitcast(BasicBlockTranslation* bb_translation,
                         llvm::BitCastInst* bitcast);

  /// \brief Translate a llvm::CastInst
  void translate_cast(BasicBlockTranslation* bb_translation,
                      llvm::CastInst* cast);

  /// \brief Translate a llvm::GetElementPtrInst
  void translate_getelementptr(BasicBlockTranslation* bb_translation,
                               llvm::GetElementPtrInst* gep);

  /// \brief Translate a llvm::BinaryOperator
  void translate_binary_operator(BasicBlockTranslation* bb_translation,
                                 llvm::BinaryOperator* inst);

  /// \brief Translate a llvm::CmpInst
  void translate_cmp(BasicBlockTranslation* bb_translation, llvm::CmpInst* cmp);

  /// \brief Translate a llvm::BranchInst
  void translate_branch(BasicBlockTranslation* bb_translation,
                        llvm::BranchInst* br);

  /// \brief Translate a llvm::ReturnInst
  void translate_return(BasicBlockTranslation* bb_translation,
                        llvm::ReturnInst* ret);

  /// \brief Translate a llvm::PHINode
  ///
  /// This is called during the basic blocks translation, so not all
  /// basic blocks are translated yet.
  void translate_phi(BasicBlockTranslation* bb_translation, llvm::PHINode* phi);

  /// \brief Translate a llvm::PHINode
  ///
  /// This is called once every basic block has been translated.
  void translate_phi_late(BasicBlockTranslation* bb_translation,
                          llvm::PHINode* phi);

  /// \brief Translate a llvm::ExtractValueInst
  void translate_extractvalue(BasicBlockTranslation* bb_translation,
                              llvm::ExtractValueInst* inst);

  /// \brief Translate a llvm::InsertValueInst
  void translate_insertvalue(BasicBlockTranslation* bb_translation,
                             llvm::InsertValueInst* inst);

  /// \brief Translate and indexed type and a list of index into an offset
  ar::IntegerConstant* translate_indexes(
      llvm::Type* indexed_type,
      llvm::ExtractValueInst::idx_iterator begin,
      llvm::ExtractValueInst::idx_iterator end);

  /// \brief Translate a llvm::ExtractElementInst
  void translate_extractelement(BasicBlockTranslation* bb_translation,
                                llvm::ExtractElementInst* inst);

  /// \brief Translate a llvm::InsertElementInst
  void translate_insertelement(BasicBlockTranslation* bb_translation,
                               llvm::InsertElementInst* inst);

  /// \brief Translate a llvm::ShuffleVectorInst
  void translate_shufflevector(BasicBlockTranslation* bb_translation,
                               llvm::ShuffleVectorInst* inst);

  /// \brief Translate a llvm::UnreachableInst
  void translate_unreachable(BasicBlockTranslation* bb_translation,
                             llvm::UnreachableInst* unreachable);

  /// \brief Translate a llvm::LandingPadInst
  void translate_landingpad(BasicBlockTranslation* bb_translation,
                            llvm::LandingPadInst* landingpad);

  /// \brief Translate a llvm::ResumeInst
  void translate_resume(BasicBlockTranslation* bb_translation,
                        llvm::ResumeInst* resume);

  /// \brief Translate a llvm::Constant into an ar::Value
  ///
  /// This will add statements in the main basic block if the llvm::Constant
  /// contains constant expressions, or if a bitcast is required.
  ///
  /// Note: If non-null, the `type` parameter should be compatible with the
  /// constant: `TypeImporter::match_ar_type(cst->getType(), type)` should be
  /// true.
  ///
  /// \param bb_translation The current basic block translation
  /// \param cst The constant to translate
  /// \param type The required ar::Type, or null if no specific type is needed
  ar::Value* translate_constant(BasicBlockTranslation* bb_translation,
                                llvm::Constant* cst,
                                ar::Type* type);

  /// \brief Translate a llvm::Value into an ar::Value
  ///
  /// This will add a statement in the basic block if a bitcast is required.
  ///
  /// Note: If non-null, the `type` parameter should be compatible with the
  /// value: `TypeImporter::match_ar_type(value->getType(), type)` should be
  /// true.
  ///
  /// \param bb_translation The current basic block translation
  /// \param value The value to translate
  /// \param type The required ar::Type, or null if no specific type is needed
  ar::Value* translate_value(BasicBlockTranslation* bb_translation,
                             llvm::Value* value,
                             ar::Type* type);

  /// \brief Translate a llvm::InlineAsm into an ar::InlineAssemblyConstant
  ar::InlineAssemblyConstant* translate_inline_asm(llvm::InlineAsm* inline_asm,
                                                   ar::Type* type);

  /// \brief Add a bitcast from var->type() to type
  ar::InternalVariable* add_bitcast(BasicBlockTranslation* bb_translation,
                                    ar::Variable* var,
                                    ar::Type* type);

  /// \brief Add a bitcast from operand to result
  ar::InternalVariable* add_bitcast(BasicBlockTranslation* bb_translation,
                                    ar::InternalVariable* result,
                                    ar::Variable* operand);

  /// \brief Translate an integer llvm::Value into an ar::Value and cast it to
  /// the given ar::IntegerType
  ///
  /// This will add statements in the main basic block if casts are required.
  ///
  /// \param bb_translation The current basic block translation
  /// \param value The value to translate
  /// \param type The required ar::IntegerType
  ar::Value* translate_cast_integer_value(BasicBlockTranslation* bb_translation,
                                          llvm::Value* value,
                                          ar::IntegerType* type);

  /// \brief Add integer casts (trunc/ext/sign_cast) from var->type() to type
  ar::InternalVariable* add_integer_casts(BasicBlockTranslation* bb_translation,
                                          ar::Variable* var,
                                          ar::IntegerType* type);

  /*
   * Implementation of a heuristic algorithm to infer the type (with sign
   * information) of a llvm::Value
   */

  /// \brief Try to infer the correct type for the given `value` w.r.t the
  /// signedness
  ar::Type* infer_type(llvm::Value*);

  /// \brief Try to infer the correct type for the given `value` using llvm
  /// debug info
  ///
  /// Returns nullptr if the type could not be deduced
  ar::Type* infer_type_from_dbg(llvm::Value*);

  /// \brief Infer a default type, if no hints were found
  ar::Type* infer_default_type(llvm::Value*);

  /// \brief Hint for a type
  struct TypeHint {
    /// \brief The type
    ar::Type* type = nullptr;

    /// \brief The score corresponds to the reliability of the hint.
    /// A big number means the hint is very reliable.
    /// 0 means the hint should be ignored.
    unsigned score = 0;

    /// \brief Constructor
    TypeHint() = default;

    /// \brief Constructor
    TypeHint(ar::Type* type_, unsigned score_) : type(type_), score(score_) {}

    /// \brief Is it an empty hint?
    bool ignore() const { return this->score == 0; }

    /// \brief Update the score
    void set_score(unsigned new_score) {
      if (this->type != nullptr) {
        this->score = new_score;
      }
    }
  };

  /*
   * Get a hint about the type of a llvm::Value using one of its user
   */

  TypeHint infer_type_hint_use(llvm::Use&);

  TypeHint infer_type_hint_use_alloca(llvm::Use&, llvm::AllocaInst* alloca);

  TypeHint infer_type_hint_use_store(llvm::Use&, llvm::StoreInst* store);

  TypeHint infer_type_hint_use_load(llvm::Use&, llvm::LoadInst* load);

  TypeHint infer_type_hint_use_call(llvm::Use&, llvm::CallInst* call);

  TypeHint infer_type_hint_use_invoke(llvm::Use&, llvm::InvokeInst* invoke);

  template < typename CallInstType >
  TypeHint infer_type_hint_use_call_helper(llvm::Use&, CallInstType* call);

  TypeHint infer_type_hint_use_cast(llvm::Use&, llvm::CastInst* cast);

  TypeHint infer_type_hint_use_getelementptr(llvm::Use&,
                                             llvm::GetElementPtrInst* gep);

  TypeHint infer_type_hint_use_binary_operator(llvm::Use&,
                                               llvm::BinaryOperator* inst);

  TypeHint infer_type_hint_use_cmp(llvm::Use&, llvm::CmpInst* cmp);

  TypeHint infer_type_hint_use_branch(llvm::Use&, llvm::BranchInst* br);

  TypeHint infer_type_hint_use_return(llvm::Use&, llvm::ReturnInst* ret);

  TypeHint infer_type_hint_use_phi(llvm::Use&, llvm::PHINode* phi);

  /*
   * Get a hint about the type of an operand of a llvm::Instruction
   */

  TypeHint infer_type_hint_operand(llvm::Value*);

  TypeHint infer_type_hint_operand_global_variable(llvm::GlobalVariable* gv);

  TypeHint infer_type_hint_operand_function(llvm::Function* fun);

  TypeHint infer_type_hint_operand_instruction(llvm::Instruction* inst);

  TypeHint infer_type_hint_operand_argument(llvm::Argument* arg);

}; // end class FunctionImporter

} // end namespace import
} // end namespace frontend
} // end namespace ikos
