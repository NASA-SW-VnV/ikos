/*******************************************************************************
 *
 * \file
 * \brief Checker names
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

#pragma once

#include <ikos/analyzer/support/assert.hpp>

namespace ikos {
namespace analyzer {

/// \brief List of available checkers
enum class CheckerName {
  BufferOverflow,
  DivisionByZero,
  NullPointerDereference,
  AssertProver,
  UnalignedPointer,
  UninitializedVariable,
  SignedIntOverflow,
  UnsignedIntOverflow,
  ShiftCount,
  PointerOverflow,
  PointerCompare,
  Soundness,
  FunctionCall,
  DeadCode,
  DoubleFree,
  Debug,
  MemoryWatch,
};

/// \brief Return the long name of the given checker
inline const char* checker_long_name(CheckerName checker) {
  switch (checker) {
    case CheckerName::BufferOverflow:
      return "Buffer Overflow Analysis";
    case CheckerName::DivisionByZero:
      return "Division by Zero Analysis";
    case CheckerName::NullPointerDereference:
      return "Null Pointer Dereference Analysis";
    case CheckerName::AssertProver:
      return "Assertion Prover";
    case CheckerName::UnalignedPointer:
      return "Unaligned Pointer Analysis";
    case CheckerName::UninitializedVariable:
      return "Uninitialized Variable Analysis";
    case CheckerName::SignedIntOverflow:
      return "Signed Integer Overflow Analysis";
    case CheckerName::UnsignedIntOverflow:
      return "Unsigned Integer Overflow Analysis";
    case CheckerName::ShiftCount:
      return "Shift Count Analysis";
    case CheckerName::PointerOverflow:
      return "Pointer Overflow Analysis";
    case CheckerName::PointerCompare:
      return "Pointer Comparison Analysis";
    case CheckerName::Soundness:
      return "Soundness Analysis";
    case CheckerName::FunctionCall:
      return "Function Call Analysis";
    case CheckerName::DeadCode:
      return "Dead Code Analysis";
    case CheckerName::DoubleFree:
      return "Double Free Analysis";
    case CheckerName::Debug:
      return "Debugger";
    case CheckerName::MemoryWatch:
      return "Memory Watcher";
    default: {
      ikos_unreachable("unreachable");
    }
  }
}

/// \brief Return the short name of the given checker
inline const char* checker_short_name(CheckerName checker) {
  switch (checker) {
    case CheckerName::BufferOverflow:
      return "boa";
    case CheckerName::DivisionByZero:
      return "dbz";
    case CheckerName::NullPointerDereference:
      return "nullity";
    case CheckerName::AssertProver:
      return "prover";
    case CheckerName::UnalignedPointer:
      return "upa";
    case CheckerName::UninitializedVariable:
      return "uva";
    case CheckerName::SignedIntOverflow:
      return "sio";
    case CheckerName::UnsignedIntOverflow:
      return "uio";
    case CheckerName::ShiftCount:
      return "shc";
    case CheckerName::PointerOverflow:
      return "poa";
    case CheckerName::PointerCompare:
      return "pcmp";
    case CheckerName::Soundness:
      return "sound";
    case CheckerName::FunctionCall:
      return "fca";
    case CheckerName::DeadCode:
      return "dca";
    case CheckerName::DoubleFree:
      return "dfa";
    case CheckerName::Debug:
      return "dbg";
    case CheckerName::MemoryWatch:
      return "watch";
    default: {
      ikos_unreachable("unreachable");
    }
  }
}

} // end namespace analyzer
} // end namespace ikos
