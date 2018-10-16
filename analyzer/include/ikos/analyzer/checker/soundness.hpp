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

#pragma once

#include <vector>

#include <boost/optional.hpp>

#include <llvm/ADT/SmallVector.h>

#include <ikos/analyzer/checker/checker.hpp>

namespace ikos {
namespace analyzer {

/// \brief Soundness checker
///
/// Warn about any statement that makes the analysis unsound
class SoundnessChecker final : public Checker {
private:
  using PointsToSet = core::PointsToSet< MemoryLocation* >;
  using IntInterval = core::machine_int::Interval;

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
  };

  /// \brief Check a memory access
  void check_mem_access(ar::Statement* stmt,
                        ar::Value* pointer,
                        CheckKind access_kind,
                        const value::AbstractDomain& inv,
                        CallContext* call_context);

  /// \brief Check a memory access
  boost::optional< CheckResult > check_mem_access(
      ar::Statement* stmt,
      ar::Value* pointer,
      CheckKind access_kind,
      const value::AbstractDomain& inv);

  /// \brief Check a call to free()
  ///
  /// Check for free() on an unknown pointer, i.e CheckKind::IgnoredFree
  boost::optional< CheckResult > check_free(ar::IntrinsicCall* call,
                                            const value::AbstractDomain& inv);

  /// \brief Check a call
  ///
  /// Check for calls to unknown extern function with an unknown pointer
  /// parameter, i.e CheckKind::IgnoredCallSideEffect
  std::vector< CheckResult > check_call(ar::CallBase* call,
                                        const value::AbstractDomain& inv);

  /// \brief Return true if the function is an known external function
  static bool is_known_extern_function(ar::Function* fun);

  /// \brief Dispay a soundness check, if requested
  bool display_soundness_check(Result result, ar::Statement* stmt) const;

}; // end class SoundnessChecker

} // end namespace analyzer
} // end namespace ikos
