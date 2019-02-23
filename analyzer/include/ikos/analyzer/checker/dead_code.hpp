/*******************************************************************************
 *
 * \file
 * \brief Dead code checker
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

#include <llvm/ADT/DenseMap.h>

#include <ikos/analyzer/checker/checker.hpp>

namespace ikos {
namespace analyzer {

/// \brief Dead code checker
class DeadCodeChecker final : public Checker {
private:
  /// \brief Previous statement for each basic block currently analyzed
  ///
  /// Since we cannot get the previous statement of an `ar::Statement` in O(1),
  /// we use this map to store the previously analyzed statement.
  llvm::DenseMap< ar::BasicBlock*, ar::Statement* > _prev_stmts;

public:
  /// \brief Constructor
  explicit DeadCodeChecker(Context& ctx);

  /// \brief Get the checker name
  CheckerName name() const override;

  /// \brief Get the checker description
  const char* description() const override;

  /// \brief Check a statement
  void check(ar::Statement* stmt,
             const value::AbstractDomain& inv,
             CallContext* call_context) override;

private:
  /// \brief Return the previous statement
  ar::Statement* previous_statement(ar::Statement* stmt) const;

  /// \brief Save the current statement
  void save_current_statement(ar::Statement* stmt);

  /// \brief Return true if we need to skip the check for the given statement
  static bool skip_check(ar::Statement* stmt);

  /// \brief Return true if we need a check for the current statement,
  /// given the previous statement (or null) and the current basic block
  static bool needs_check(ar::Statement* prev_stmt, ar::BasicBlock* bb);

  /// \brief Dispay a dead code check, if requested
  void display_dead_code_check(Result result, ar::Statement* stmt) const;

}; // end class DeadCodeChecker

} // end namespace analyzer
} // end namespace ikos
