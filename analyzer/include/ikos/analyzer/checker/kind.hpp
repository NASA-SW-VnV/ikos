/*******************************************************************************
 *
 * \file
 * \brief Check kind
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2018-2019 United States Government as represented by the
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

namespace ikos {
namespace analyzer {

/// \brief List of checks
enum class CheckKind {
  /// \brief Check for an unreachable statement
  Unreachable,

  /// \brief Check for an unexpected operand of a statement
  UnexpectedOperand,

  /// \brief Check for an uninitialized variable of a statement
  UninitializedVariable,

  /// \brief Check for an assertion
  Assert,

  /// \brief Check for a division by zero
  DivisionByZero,

  /// \brief Check for an invalid shift count
  ShiftCount,

  _BeginIntOverflow,

  /// \brief Check for an underflow on a signed integer operation
  SignedIntUnderflow,

  /// \brief Check for an overflow on a signed integer operation
  SignedIntOverflow,

  /// \brief Check for an underflow on an unsigned integer operation
  UnsignedIntUnderflow,

  /// \brief Check for an overflow on an unsigned integer operation
  UnsignedIntOverflow,

  _EndIntOverflow,

  /// \brief Check for a memory access on a null pointer
  NullPointerDereference,

  /// \brief Check for a null pointer comparison
  NullPointerComparison,

  /// \brief Check for a pointer comparison on an invalid pointer (points-to set
  /// is empty)
  InvalidPointerComparison,

  /// \brief Check for an invalid pointer comparison
  PointerComparison,

  /// \brief Check for a pointer computation overflow
  PointerOverflow,

  /// \brief Check for a memory access on an invalid pointer (points-to set is
  /// empty)
  InvalidPointerDereference,

  /// \brief Check for an unknown memory access (points-to set is top)
  UnknownMemoryAccess,

  /// \brief Check for the alignment of a pointer on a memory access
  UnalignedPointer,

  _BeginBufferOverflow,

  /// \brief Check for a call to gets()
  BufferOverflowGets,

  /// \brief Check for a buffer overflow
  BufferOverflow,

  _EndBufferOverflow,

  _BeginSoundness,

  /// \brief Ignored memory store on an unknown pointer
  IgnoredStore,

  /// \brief Ignored memory copy on an unknown pointer
  IgnoredMemoryCopy,

  /// \brief Ignored memory move on an unknown pointer
  IgnoredMemoryMove,

  /// \brief Ignored memory set on an unknown pointer
  IgnoredMemorySet,

  /// \brief Ignored free on an unknown pointer
  IgnoredFree,

  /// \brief Ignored side effect of a call with an unknown pointer parameter
  IgnoredCallSideEffectOnPointerParameter,

  /// \brief Ignored side effect of a function call
  IgnoredCallSideEffect,

  /// \brief Check for a recursive function call
  RecursiveFunctionCall,

  _EndSoundness,

  _BeginFunctionCall,

  /// \brief Check for a function call to inline assembly
  FunctionCallInlineAssembly,

  /// \brief Check for a function call on an unknown pointer
  UnknownFunctionCallPointer,

  /// \brief Check a function call
  FunctionCall,

  _EndFunctionCall,

  /// \brief Check for a memory deallocation (e.g, free)
  Free,

};

} // end namespace analyzer
} // end namespace ikos
