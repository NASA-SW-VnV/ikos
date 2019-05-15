/*******************************************************************************
 *
 * \file
 * \brief Base class for property checker and make_checker implementation
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

#include <ikos/analyzer/checker/assert_prover.hpp>
#include <ikos/analyzer/checker/buffer_overflow.hpp>
#include <ikos/analyzer/checker/checker.hpp>
#include <ikos/analyzer/checker/dead_code.hpp>
#include <ikos/analyzer/checker/debug.hpp>
#include <ikos/analyzer/checker/division_by_zero.hpp>
#include <ikos/analyzer/checker/double_free.hpp>
#include <ikos/analyzer/checker/function_call.hpp>
#include <ikos/analyzer/checker/memory_watch.hpp>
#include <ikos/analyzer/checker/null_dereference.hpp>
#include <ikos/analyzer/checker/pointer_alignment.hpp>
#include <ikos/analyzer/checker/pointer_compare.hpp>
#include <ikos/analyzer/checker/pointer_overflow.hpp>
#include <ikos/analyzer/checker/shift_count.hpp>
#include <ikos/analyzer/checker/signed_int_overflow.hpp>
#include <ikos/analyzer/checker/soundness.hpp>
#include <ikos/analyzer/checker/uninitialized_variable.hpp>
#include <ikos/analyzer/checker/unsigned_int_overflow.hpp>
#include <ikos/analyzer/util/color.hpp>
#include <ikos/analyzer/util/source_location.hpp>

namespace ikos {
namespace analyzer {

// Checker

void Checker::display_stmt_location(LogMessage& msg, ar::Statement* s) const {
  msg << color::bold() << source_location_string(s, _ctx.wd) << ": "
      << color::off();
}

void Checker::display_result(LogMessage& msg, Result result) const {
  switch (result) {
    case Result::Ok: {
      msg << "[" << color::bold_green() << "ok" << color::off() << "] ";
    } break;
    case Result::Warning: {
      msg << "[" << color::bold_yellow() << "warning" << color::off() << "] ";
    } break;
    case Result::Error: {
      msg << "[" << color::bold_red() << "error" << color::off() << "] ";
    } break;
    case Result::Unreachable: {
      msg << "[" << color::bold_magenta() << "unreachable" << color::off()
          << "] ";
    } break;
    default: {
      ikos_unreachable("unreachable");
    }
  }
}

// make_checker

std::unique_ptr< Checker > make_checker(Context& ctx, CheckerName name) {
  switch (name) {
    case CheckerName::BufferOverflow:
      return std::make_unique< BufferOverflowChecker >(ctx);
    case CheckerName::DivisionByZero:
      return std::make_unique< DivisionByZeroChecker >(ctx);
    case CheckerName::NullPointerDereference:
      return std::make_unique< NullDereferenceChecker >(ctx);
    case CheckerName::AssertProver:
      return std::make_unique< AssertProverChecker >(ctx);
    case CheckerName::UnalignedPointer:
      return std::make_unique< PointerAlignmentChecker >(ctx);
    case CheckerName::UninitializedVariable:
      return std::make_unique< UninitializedVariableChecker >(ctx);
    case CheckerName::SignedIntOverflow:
      return std::make_unique< SignedIntOverflowChecker >(ctx);
    case CheckerName::UnsignedIntOverflow:
      return std::make_unique< UnsignedIntOverflowChecker >(ctx);
    case CheckerName::ShiftCount:
      return std::make_unique< ShiftCountChecker >(ctx);
    case CheckerName::PointerOverflow:
      return std::make_unique< PointerOverflowChecker >(ctx);
    case CheckerName::PointerCompare:
      return std::make_unique< PointerCompareChecker >(ctx);
    case CheckerName::Soundness:
      return std::make_unique< SoundnessChecker >(ctx);
    case CheckerName::FunctionCall:
      return std::make_unique< FunctionCallChecker >(ctx);
    case CheckerName::DeadCode:
      return std::make_unique< DeadCodeChecker >(ctx);
    case CheckerName::DoubleFree:
      return std::make_unique< DoubleFreeChecker >(ctx);
    case CheckerName::Debug:
      return std::make_unique< DebugChecker >(ctx);
    case CheckerName::MemoryWatch:
      return std::make_unique< MemoryWatchChecker >(ctx);
    default:
      ikos_unreachable("unreachable");
  }
}

} // end namespace analyzer
} // end namespace ikos
