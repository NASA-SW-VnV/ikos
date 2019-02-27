/*******************************************************************************
 *
 * \file
 * \brief Base for integer overflow checker
 *
 * Author: Thomas Bailleux
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

#include <llvm/ADT/SmallVector.h>

#include <ikos/analyzer/checker/checker.hpp>

namespace ikos {
namespace analyzer {

/// \brief Base for integer overflow checker
class IntOverflowCheckerBase : public Checker {
private:
  using IntInterval = core::machine_int::Interval;
  using ZInterval = core::numeric::Interval< ZNumber >;
  using ZBound = core::Bound< ZNumber >;

public:
  /// \brief Constructor
  explicit IntOverflowCheckerBase(Context& ctx);

protected:
  /// \brief Check an integer overflow and insert the checks in the database
  void check_integer_overflow(ar::BinaryOperation* stmt,
                              const value::AbstractDomain& inv,
                              CallContext* call_context);

private:
  /// \brief Check result
  struct CheckResult {
    CheckKind kind;
    Result result;
    llvm::SmallVector< ar::Value*, 2 > operands;
    JsonDict info;
  };

  /// \brief Check an integer overflow
  llvm::SmallVector< CheckResult, 2 > check_integer_overflow(
      ar::BinaryOperation* stmt, const value::AbstractDomain& inv);

private:
  /// \brief Display info about the check
  llvm::Optional< LogMessage > display_int_overflow_check(
      Result result, ar::BinaryOperation* stmt) const;

  /// \brief CheckKind for integer underflow
  virtual CheckKind underflow_check_kind() const = 0;

  /// \brief CheckKind for integer overflow
  virtual CheckKind overflow_check_kind() const = 0;

}; // end class IntOverflowCheckerBase

} // end namespace analyzer
} // end namespace ikos
