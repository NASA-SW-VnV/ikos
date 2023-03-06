/*******************************************************************************
 *
 * \file
 * \brief Functions to create various ikos-specific llvm passes
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017-2019 United States Government as represented by the
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

#include <llvm/Pass.h>
#include <llvm/PassRegistry.h>

namespace ikos {
namespace frontend {
namespace pass {

/// \brief Lower constant expressions into instructions
llvm::FunctionPass* create_lower_cst_expr_pass();

/// \brief Lower select instructions
///
/// Lower select instructions to three new basic blocks and a phi instruction.
llvm::FunctionPass* create_lower_select_pass();

/// \brief Mark all internal functions with the AlwaysInline attribute
llvm::ModulePass* create_mark_internal_inline_pass();

/// \brief Name all unnamed values
llvm::ModulePass* create_name_values_pass();

/// \brief Remove printf-like function calls
llvm::FunctionPass* create_remove_printf_calls_pass();

/// \brief Remove blocks that are not reachable, including dead cycles
llvm::FunctionPass* create_remove_unreachable_blocks_pass();

/// \brief Initialize all passes linked into the ikos-pp library
void initialize_ikos_passes(llvm::PassRegistry&);

} // end namespace pass
} // end namespace frontend
} // end namespace ikos

namespace llvm {

/// \brief Initialize the LowerCstExprPass
void initializeLowerCstExprPassPass(llvm::PassRegistry&);

/// \brief Initialize the LowerSelectPass
void initializeLowerSelectPassPass(llvm::PassRegistry&);

/// \brief Initialize the MarkInternalInlinePass
void initializeMarkInternalInlinePassPass(llvm::PassRegistry&);

/// \brief Initialize the NameValuesPass
void initializeNameValuesPassPass(llvm::PassRegistry&);

/// \brief Initialize the RemovePrintfCallsPass
void initializeRemovePrintfCallsPassPass(llvm::PassRegistry&);

/// \brief Initialize the RemoveUnreachableBlocksPass
void initializeRemoveUnreachableBlocksPassPass(llvm::PassRegistry&);

} // end namespace llvm
