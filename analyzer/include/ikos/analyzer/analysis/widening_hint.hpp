/*******************************************************************************
 *
 * \file
 * \brief Widening hint analysis
 *
 * Author: Thomas Bailleux
 *
 * Contributor: Maxime Arthaud
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

#include <ikos/analyzer/analysis/context.hpp>

namespace ikos {
namespace analyzer {

/// \brief Widening hint analysis
///
/// This analysis is intended to be used before the computation of any fixpoint.
///
/// It detect widening hints to help the fixpoint computation. It basically
/// iterates on the cycles in the code and looks for constants which could be
/// involved in the loop guard.
///
/// For instance, if we have the following code:
///
/// \code{.c}
/// for (int i = 0; i < 10; i++) {
///   do_something();
/// }
/// \endcode
///
/// It will mark the constant '10' as a widening hint.
class WideningHintAnalysis {
private:
  /// \brief Analysis context
  Context& _ctx;

public:
  /// \brief Constructor
  explicit WideningHintAnalysis(Context& ctx);

  /// \brief No copy constructor
  WideningHintAnalysis(const WideningHintAnalysis&) = delete;

  /// \brief No move constructor
  WideningHintAnalysis(WideningHintAnalysis&&) = delete;

  /// \brief No copy assignment operator
  WideningHintAnalysis& operator=(const WideningHintAnalysis&) = delete;

  /// \brief No move assignment operator
  WideningHintAnalysis& operator=(WideningHintAnalysis&&) = delete;

  /// \brief Destructor
  ~WideningHintAnalysis();

  /// \brief Run the analysis
  void run();

private:
  /// \brief Run the analysis on the given function
  void run(ar::Function*);

}; // end class WideningHintAnalysis

} // end namespace analyzer
} // end namespace ikos
