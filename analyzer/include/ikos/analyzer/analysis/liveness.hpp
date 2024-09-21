/*******************************************************************************
 *
 * \file
 * \brief Liveness variable analysis
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
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

#include <iosfwd>
#include <vector>
#include <cstdint>

#include <boost/optional.hpp>

#include <llvm/ADT/DenseMap.h>

#include <ikos/analyzer/analysis/context.hpp>
#include <ikos/analyzer/analysis/variable.hpp>

namespace ikos {
namespace analyzer {

/// \brief Compute liveness of variables for a whole bundle
class LivenessAnalysis {
public:
  /// \brief List of variables
  using VariableRefList = std::vector< Variable* >;

private:
  /// \brief Map from basic block to a list of variables
  using VariableRefMap = llvm::DenseMap< ar::BasicBlock*, VariableRefList >;

private:
  /// \brief Analysis context
  Context& _ctx;

  /// \brief List of live variables at the entry of a basic block
  VariableRefMap _live_at_entry_map;

  /// \brief List of dead variables at the end of a basic block
  VariableRefMap _dead_at_end_map;

public:
  /// \brief Constructor
  explicit LivenessAnalysis(Context& ctx);

  /// \brief No copy constructor
  LivenessAnalysis(const LivenessAnalysis&) = delete;

  /// \brief No move constructor
  LivenessAnalysis(LivenessAnalysis&&) = delete;

  /// \brief No copy assignment operator
  LivenessAnalysis& operator=(const LivenessAnalysis&) = delete;

  /// \brief No move assignment operator
  LivenessAnalysis& operator=(LivenessAnalysis&&) = delete;

  /// \brief Destructor
  ~LivenessAnalysis();

  /// \brief Return a list of live variables at the entry of the given block
  ///
  /// Returns boost::none if we have no information
  boost::optional< const VariableRefList& > live_at_entry(
      ar::BasicBlock* bb) const;

  /// \brief Return a list of dead variables at the end of the given block
  ///
  /// Returns boost::none if we have no information
  boost::optional< const VariableRefList& > dead_at_end(
      ar::BasicBlock* bb) const;

  /// \brief Run the analysis
  void run();

private:
  /// \brief Run the analysis on the given code
  void run(ar::Code* code);

public:
  /// \brief Dump the liveness analysis results, for debugging purpose
  void dump(std::ostream& o) const;

private:
  /// \brief Dump the liveness analysis results for the given code
  void dump(std::ostream& o, ar::Code* code) const;

  /// \brief Dump a VariableRefList
  static void dump(std::ostream& o, const VariableRefList& vars);

}; // end class LivenessAnalysis

} // end namespace analyzer
} // end namespace ikos
