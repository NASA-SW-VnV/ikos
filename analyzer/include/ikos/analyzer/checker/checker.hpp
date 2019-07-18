/*******************************************************************************
 *
 * \file
 * \brief Base class for property checkers
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

#include <memory>

#include <llvm/ADT/Optional.h>

#include <ikos/ar/semantic/statement.hpp>

#include <ikos/analyzer/analysis/call_context.hpp>
#include <ikos/analyzer/analysis/context.hpp>
#include <ikos/analyzer/analysis/literal.hpp>
#include <ikos/analyzer/analysis/value/abstract_domain.hpp>
#include <ikos/analyzer/checker/name.hpp>
#include <ikos/analyzer/database/output.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {

/// \brief Base class for property checkers
class Checker {
protected:
  /// \brief Analysis context
  Context& _ctx;

  /// \brief Literal factory
  LiteralFactory& _lit_factory;

  /// \brief Checks table
  ChecksTable& _checks;

  /// \brief Option to display the invariants
  DisplayOption _display_invariants;

  /// \brief Option to display the checks
  DisplayOption _display_checks;

protected:
  /// \brief Constructor
  explicit Checker(Context& ctx)
      : _ctx(ctx),
        _lit_factory(*ctx.lit_factory),
        _checks(ctx.output_db->checks),
        _display_invariants(ctx.opts.display_invariants),
        _display_checks(ctx.opts.display_checks) {}

public:
  /// \brief No copy constructor
  Checker(const Checker&) = delete;

  /// \brief Move constructor
  Checker(Checker&&) noexcept = default;

  /// \brief No copy assignment operator
  Checker& operator=(const Checker&) = delete;

  /// \brief No move assignment operator
  Checker& operator=(Checker&&) = delete;

  /// \brief Destructor
  virtual ~Checker() = default;

  /// \brief Get the checker name
  virtual CheckerName name() const = 0;

  /// \brief Get the checker short name
  const char* short_name() const { return checker_short_name(this->name()); }

  /// \brief Get the checker long name
  const char* long_name() const { return checker_long_name(this->name()); }

  /// \brief Get the checker description
  virtual const char* description() const = 0;

  /// \brief Check a statement
  virtual void check(ar::Statement* stmt,
                     const value::AbstractDomain& inv,
                     CallContext* call_context) = 0;

protected:
  // Helpers to display checks and invariants

  /// \brief Return true if the invariant should be displayed
  inline bool display_invariant(Result result) const {
    return this->_display_invariants == DisplayOption::All ||
           (this->_display_invariants == DisplayOption::Fail &&
            (result == Result::Error || result == Result::Warning ||
             result == Result::Unreachable));
  }

  // \brief Display the invariant for the given statement, if requested
  ///
  /// \return a log message if the invariant should be displayed
  inline llvm::Optional< LogMessage > display_invariant(
      Result result,
      ar::Statement* stmt,
      const value::AbstractDomain& inv) const {
    if (this->display_invariant(result)) {
      LogMessage msg = log::msg();
      this->display_stmt_location(msg, stmt);
      msg << "Invariant:\n";
      inv.dump(msg.stream());
      msg << "\n";
      return std::move(msg);
    } else {
      return llvm::None;
    }
  }

  /// \brief Return true if the check should be displayed
  inline bool display_check(Result result) const {
    return this->_display_checks == DisplayOption::All ||
           (this->_display_checks == DisplayOption::Fail &&
            (result == Result::Error || result == Result::Warning ||
             result == Result::Unreachable));
  }

  /// \brief Display the check for the given statement, if requested
  ///
  /// \return a log message if the check should be displayed
  inline llvm::Optional< LogMessage > display_check(Result result,
                                                    ar::Statement* stmt) const {
    if (this->display_check(result)) {
      LogMessage msg = log::msg();
      this->display_stmt_location(msg, stmt);
      this->display_result(msg, result);
      return std::move(msg);
    } else {
      return llvm::None;
    }
  }

  /// \brief Display a statement location
  void display_stmt_location(LogMessage& msg, ar::Statement* s) const;

  /// \brief Display a check result
  void display_result(LogMessage& msg, Result result) const;

}; // end class Checker

/// \brief Create a checker, given its name
std::unique_ptr< Checker > make_checker(Context& ctx, CheckerName name);

} // end namespace analyzer
} // end namespace ikos
