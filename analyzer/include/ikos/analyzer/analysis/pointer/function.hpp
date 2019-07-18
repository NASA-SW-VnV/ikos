/******************************************************************************
 *
 * \file
 * \brief Function pointer analysis
 *
 * This pass is intended to be used as a pre-step for other analyses.
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *               Clement Decoodt
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

#include <ikos/analyzer/analysis/context.hpp>
#include <ikos/analyzer/analysis/pointer/value.hpp>

namespace ikos {
namespace analyzer {

/// \brief Compute points-to set of function pointers for a whole bundle
///
/// This pass is intended to be used as a pre-step for other analyses.
class FunctionPointerAnalysis {
private:
  /// \brief Analysis context
  Context& _ctx;

  /// \brief Pointer information
  PointerInfo _info;

public:
  /// \brief Constructor
  explicit FunctionPointerAnalysis(Context& ctx);

  /// \brief No copy constructor
  FunctionPointerAnalysis(const FunctionPointerAnalysis&) = delete;

  /// \brief No move constructor
  FunctionPointerAnalysis(FunctionPointerAnalysis&&) = delete;

  /// \brief No copy assignment operator
  FunctionPointerAnalysis& operator=(const FunctionPointerAnalysis&) = delete;

  /// \brief No move assignment operator
  FunctionPointerAnalysis& operator=(FunctionPointerAnalysis&&) = delete;

  /// \brief Destructor
  ~FunctionPointerAnalysis();

  /// \brief Run the analysis
  void run();

  /// \brief Dump the function pointer analysis results, for debugging purpose
  void dump(std::ostream& o) const;

  /// \brief Return the result of the analysis
  const PointerInfo& results() const { return this->_info; }

}; // end class FunctionPointerAnalysis

} // end namespace analyzer
} // end namespace ikos
