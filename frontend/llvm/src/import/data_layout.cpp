/*******************************************************************************
 *
 * \file
 * \brief Translate LLVM data layout into AR data layout
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

#include "data_layout.hpp"

namespace ikos {
namespace frontend {
namespace import {

static ar::DataLayoutInfo translate_data_layout_info(
    const llvm::DataLayout& data_layout, llvm::Type* type) {
  uint64_t bit_width = type->getPrimitiveSizeInBits();
  uint64_t abi_alignment = data_layout.getABITypeAlignment(type);
  uint64_t pref_alignment = data_layout.getPrefTypeAlignment(type);
  return {bit_width, abi_alignment, pref_alignment};
}

std::unique_ptr< ar::DataLayout > translate_data_layout(
    const llvm::DataLayout& llvm_data_layout, llvm::LLVMContext& ctx) {
  // Translate endianness
  ar::Endianness endianness =
      llvm_data_layout.isLittleEndian() ? ar::LittleEndian : ar::BigEndian;

  // Translate pointer size and alignments
  ar::DataLayoutInfo
      pointers(llvm_data_layout.getPointerSizeInBits(),
               llvm_data_layout.getPointerABIAlignment(0).value(),
               llvm_data_layout.getPointerPrefAlignment().value());

  // Create ar::DataLayout
  std::unique_ptr< ar::DataLayout > ar_data_layout =
      ar::DataLayout::create(endianness, pointers);

  // Set alignments of integers
  for (unsigned bit_width : std::array< unsigned, 5 >{{1, 8, 16, 32, 64}}) {
    ar_data_layout->set_integer_alignment(
        translate_data_layout_info(llvm_data_layout,
                                   llvm::IntegerType::get(ctx, bit_width)));
  }

  // Set alignments of floating points
  ar_data_layout->set_float_alignment(
      translate_data_layout_info(llvm_data_layout, llvm::Type::getHalfTy(ctx)));
  ar_data_layout->set_float_alignment(
      translate_data_layout_info(llvm_data_layout,
                                 llvm::Type::getFloatTy(ctx)));
  ar_data_layout->set_float_alignment(
      translate_data_layout_info(llvm_data_layout,
                                 llvm::Type::getDoubleTy(ctx)));
  ar_data_layout->set_float_alignment(
      translate_data_layout_info(llvm_data_layout,
                                 llvm::Type::getX86_FP80Ty(ctx)));
  ar_data_layout->set_float_alignment(
      translate_data_layout_info(llvm_data_layout,
                                 llvm::Type::getFP128Ty(ctx)));
  ar_data_layout->set_float_alignment(
      translate_data_layout_info(llvm_data_layout,
                                 llvm::Type::getPPC_FP128Ty(ctx)));

  return ar_data_layout;
}

} // end namespace import
} // end namespace frontend
} // end namespace ikos
