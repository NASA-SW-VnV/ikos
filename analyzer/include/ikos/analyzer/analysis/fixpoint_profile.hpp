/*******************************************************************************
 *
 * \file
 * \brief FixpointProfileAnalysis used by the analyses
 *
 * Author: Thomas Bailleux
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

#include <memory>

#include <boost/optional.hpp>

#include <llvm/ADT/DenseMap.h>

#include <ikos/core/number/machine_int.hpp>

#include <ikos/ar/semantic/code.hpp>

#include <ikos/analyzer/analysis/context.hpp>

namespace ikos {
namespace analyzer {

class FixpointProfile;

/// \brief The fixpoint profiler
///
/// This analysis is intended to be used before the computation of any fixpoint.
///
/// It generates a FixpointProfile for each function, providing widening hints
/// to help the fixpoint computation. It basically iterates on the cycles in the
/// code and looks for constants which could be involved in the loop guard.
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
class FixpointProfileAnalysis {
private:
  /// \brief Analysis context
  Context& _ctx;

  /// \brief Map that associates a function to a fixpoint profile
  llvm::DenseMap< ar::Function*, std::unique_ptr< FixpointProfile > > _map;

public:
  /// \brief Constructor
  FixpointProfileAnalysis(Context& ctx) : _ctx(ctx) {}

  /// \brief Deleted copy constructor
  FixpointProfileAnalysis(const FixpointProfileAnalysis&) = delete;

  /// \brief Deleted move constructor
  FixpointProfileAnalysis(FixpointProfileAnalysis&&) = delete;

  /// \brief Delete copy assignment operator
  FixpointProfileAnalysis& operator=(const FixpointProfileAnalysis&) = delete;

  /// \brief Delete move assignment operator
  FixpointProfileAnalysis& operator=(FixpointProfileAnalysis&&) = delete;

  /// \brief Run the analysis
  void run();

  /// \brief Dump the fixpoint profile analysis, for debugging purpose
  void dump(std::ostream& o) const;

  /// \brief Return the profile associated with a function
  boost::optional< const FixpointProfile& > profile(ar::Function*) const;

private:
  /// \brief Analyze a function
  std::unique_ptr< FixpointProfile > analyze_function(ar::Function*);

}; // end class FixpointProfileAnalysis

/// \brief A fixpoint profile
///
/// A fixpoint profile is associated with a function.
/// Inside a function, if there are some cycles, it will associate the head of
/// each cycle with a widening hint, if found.
class FixpointProfile {
private:
  /// \brief Function associated to the profile
  ar::Function* _function;

  /// \brief Map of widening hints associated to a given block
  llvm::DenseMap< ar::BasicBlock*, std::unique_ptr< core::MachineInt > >
      _widening_hints;

  /// \brief Constructor
  FixpointProfile(ar::Function* fun) : _function(fun) {}

public:
  /// \brief Return the function associated to the profile
  ar::Function* function() const { return this->_function; }

  /// \brief Return an widening hint for a given basic block, if exists
  boost::optional< const core::MachineInt& > widening_hint(
      ar::BasicBlock*) const;

  /// \brief Return true if there is at least one widening hint
  bool empty() const;

  /// \brief Dump the fixpoint profile, for debugging purpose
  void dump(std::ostream& o) const;

  friend FixpointProfileAnalysis;

}; // end class FixpointProfile

} // end namespace analyzer
} // end namespace ikos
