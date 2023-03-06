/*******************************************************************************
 *
 * \file
 * \brief Pointer alignment checker
 *
 * Author: Clement Decoodt
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

#include <ikos/core/value/machine_int/congruence.hpp>

#include <ikos/analyzer/checker/checker.hpp>

namespace ikos {
namespace analyzer {

/// \brief Pointer alignment checker
class PointerAlignmentChecker final : public Checker {
private:
  using PointsToSet = core::PointsToSet< MemoryLocation* >;
  using Congruence = core::machine_int::Congruence;

private:
  /// \brief The data layout
  const ar::DataLayout& _data_layout;

public:
  /// \brief Constructor
  explicit PointerAlignmentChecker(Context& ctx);

  /// \brief Get the checker name
  CheckerName name() const override;

  /// \brief Get the checker description
  const char* description() const override;

  /// \brief Check a statement
  void check(ar::Statement* stmt,
             const value::AbstractDomain& inv,
             CallContext* call_context) override;

private:
  /// \brief Check a pointer alignment and insert the check in the database
  void check_alignment(ar::Statement* stmt,
                       ar::Value* operand,
                       uint64_t alignment_req,
                       const value::AbstractDomain& inv,
                       CallContext* call_context);

  /// \brief Check result
  struct CheckResult {
    CheckKind kind;
    Result result;
    JsonDict info;
  };

  /// \brief Check a pointer alignment and insert the check in the database
  CheckResult check_alignment(ar::Statement* stmt,
                              ar::Value* operand,
                              uint64_t alignment_req,
                              const value::AbstractDomain& inv);

  /// \brief Check the alignment of a memory location
  Result check_memory_location_alignment(MemoryLocation* memloc,
                                         const Congruence& offset_c,
                                         const Congruence& alignment_req_c,
                                         JsonDict& block_info);

  /// \brief Return the congruence aZ+b on pointer offsets
  Congruence to_congruence(uint64_t a, uint64_t b) const;

private:
  /// \brief Dispay the pointer alignment check, if requested
  llvm::Optional< LogMessage > display_alignment_check(
      Result result, ar::Statement* stmt, ar::Value* operand) const;

}; // end class PointerAlignmentChecker

} // end namespace analyzer
} // end namespace ikos
