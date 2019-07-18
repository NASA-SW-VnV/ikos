/******************************************************************************
 *
 * \file
 * \brief Pointer analysis
 *
 * This pass is intended to be used as a pre-step for other analyses.
 * It computes first intra-procedurally numerical invariants for each function.
 * Then, it generates nonuniform points-to constraints in a flow-insensitive
 * manner, solves them, and finally stores the results for subsequent analyses.
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
#include <ikos/analyzer/analysis/pointer/function.hpp>
#include <ikos/analyzer/analysis/pointer/value.hpp>

namespace ikos {
namespace analyzer {

/// \brief Pointer analysis for a whole bundle
///
/// This pass is intended to be used as a pre-step for other analyses. It
/// computes first intra-procedurally numerical invariants for each function.
/// Then, it generates nonuniform points-to constraints in a flow-insensitive
/// manner, solves them, and finally stores the results for subsequent analyses.
class PointerAnalysis {
private:
  /// \brief Analysis context
  Context& _ctx;

  /// \brief Previously computed function pointer analysis
  const FunctionPointerAnalysis& _function_pointer;

  /// \brief Pointer information
  PointerInfo _info;

public:
  /// \brief Constructor
  PointerAnalysis(Context& ctx,
                  const FunctionPointerAnalysis& function_pointer);

  /// \brief No copy constructor
  PointerAnalysis(const PointerAnalysis&) = delete;

  /// \brief No move constructor
  PointerAnalysis(PointerAnalysis&&) = delete;

  /// \brief No copy assignment operator
  PointerAnalysis& operator=(const PointerAnalysis&) = delete;

  /// \brief No move assignment operator
  PointerAnalysis& operator=(PointerAnalysis&&) = delete;

  /// \brief Destructor
  ~PointerAnalysis();

  /// \brief Run the analysis
  void run();

  /// \brief Dump the pointer analysis results, for debugging purpose
  void dump(std::ostream& o) const;

  /// \brief Return the result of the analysis
  const PointerInfo& results() const { return this->_info; }

}; // end class PointerAnalysis

} // end namespace analyzer
} // end namespace ikos
