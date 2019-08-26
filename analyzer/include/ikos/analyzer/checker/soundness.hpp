/*******************************************************************************
 *
 * \file
 * \brief Soundness checker
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

#include <vector>

#include <boost/optional.hpp>

#include <llvm/ADT/SmallVector.h>

#include <ikos/analyzer/checker/checker.hpp>

namespace ikos {
namespace analyzer {

/// \brief Soundness checker
///
/// Warn about any statement that could make the analysis unsound.
class SoundnessChecker final : public Checker {
private:
  using PointsToSet = core::PointsToSet< MemoryLocation* >;

public:
  /// \brief Constructor
  explicit SoundnessChecker(Context& ctx);

  /// \brief Get the checker name
  CheckerName name() const override;

  /// \brief Get the checker description
  const char* description() const override;

  /// \brief Check a statement
  void check(ar::Statement* stmt,
             const value::AbstractDomain& inv,
             CallContext* call_context) override;

private:
  /// \brief Check result
  struct CheckResult {
    CheckKind kind;
    Result result;
    llvm::SmallVector< ar::Value*, 2 > operands;
    JsonDict info;
  };

  /// \brief Check a function call
  std::vector< CheckResult > check_call(ar::CallBase* call,
                                        const value::AbstractDomain& inv,
                                        CallContext* call_context);

  /// \brief Check for a recursive function call
  ///
  /// Warn about RecursiveFunctionCall.
  CheckResult check_recursive_call(ar::CallBase* call,
                                   ar::Function* fun,
                                   const value::AbstractDomain& inv);

  /// \brief Check an intrinsic function call
  std::vector< CheckResult > check_intrinsic_call(
      ar::CallBase* call, ar::Function* fun, const value::AbstractDomain& inv);

  /// \brief Check a call to an unknown extern function
  ///
  /// Warn about IgnoredCallSideEffect.
  CheckResult check_unknown_extern_call(ar::CallBase* call,
                                        ar::Function* fun,
                                        const value::AbstractDomain& inv);

  /// \brief Check a memory write on an unknown pointer
  ///
  /// Check if the points-to set of the pointer is top.
  ///
  /// Warn about IgnoredStore, IgnoredMemoryCopy, IgnoredMemoryMove and
  /// IgnoredMemorySet.
  boost::optional< CheckResult > check_mem_write(
      ar::Statement* stmt,
      ar::Value* pointer,
      CheckKind access_kind,
      const value::AbstractDomain& inv);

  /// \brief Check a call for unknown pointer parameters
  ///
  /// Check if the points-to set of any given pointer is top.
  ///
  /// Warn about IgnoredCallSideEffectOnPointerParameter.
  std::vector< CheckResult > check_call_pointer_params(
      ar::CallBase* call,
      ar::Function* fun,
      const std::vector< ar::Value* >& pointers,
      const value::AbstractDomain& inv);

  /// \brief Check a call to free()
  ///
  /// Check if the points-to set of the given pointer is top.
  ///
  /// Warn about IgnoredFree.
  boost::optional< CheckResult > check_free(ar::CallBase* call,
                                            ar::Value* pointer,
                                            const value::AbstractDomain& inv);

  /// \brief Dispay a soundness check, if requested
  llvm::Optional< LogMessage > display_soundness_check(
      Result result, ar::Statement* stmt) const;

}; // end class SoundnessChecker

} // end namespace analyzer
} // end namespace ikos
