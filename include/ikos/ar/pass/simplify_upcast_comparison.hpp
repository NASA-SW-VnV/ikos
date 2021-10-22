/*******************************************************************************
 *
 * \file
 * \brief Pass to simplify upcasts before a comparison.
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

#include <boost/optional.hpp>
#include <ikos/ar/pass/pass.hpp>
#include <ikos/ar/semantic/statement.hpp>
#include <ikos/core/value/machine_int/interval.hpp>
#include <ikos/core/value/numeric/interval.hpp>

namespace ikos {
namespace ar {

/// \brief Simplify upcasts that happen before a comparison
///
/// This pass adds assertions after comparisons on upcasts. The goal is to
/// help the numerical analysis in ikos.
///
/// For instance:
///
///       [ si32 %1 = sext %0 ]
///              /    \ 
///  [ %1 silt 10 ]  [ %1 sige 10 ]
///
/// Will become:
///
///       [ si32 %1 = sext %0 ]
///              /    \ 
///  [ %1 silt 10 ]  [ %1 sige 10 ]
///  [ %0 silt 10 ]  [ %0 sige 10 ]
class SimplifyUpcastComparisonPass final : public CodePass {
private:
  using IntInterval = core::machine_int::Interval;
  using ZInterval = core::numeric::Interval< ZNumber >;

public:
  /// \brief Default constructor
  SimplifyUpcastComparisonPass() = default;

  /// \brief Get the pass name
  const char* name() const override;

  /// \brief Get the pass description
  const char* description() const override;

private:
  /// \brief Run the pass on the given Code
  ///
  /// Return true if the code has been updated
  bool run_on_code(Code*) override;

  /// \brief Run the pass on the given statement
  ///
  /// Return true if we should backtrace that statement
  bool run_on_statement(Statement*);

  /// \brief Backtrace a comparison statement
  ///
  /// Return true if the code has been updated
  bool run_on_comparison(Comparison*, IntegerConstant*, Value*);

  /// \brief Run the pass on the upcast associated to a comparison
  ///
  /// Return true if this if the upcast we want
  boost::optional< IntegerConstant* > run_on_upcast(Context&,
                                                    Value*,
                                                    IntegerConstant*);

  /// \brief Insert an assert for simplifying an upcast comparison
  void insert_assert(
      Comparison*, Value*, IntegerConstant*, IntegerConstant*, bool);

}; // end class SimplifyUpcastComparisonPass

} // end namespace ar
} // end namespace ikos
