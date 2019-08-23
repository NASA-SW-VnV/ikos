/*******************************************************************************
 *
 * \file
 * \brief Fixpoint parameters
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

#include <memory>

#include <boost/optional.hpp>

#include <llvm/ADT/DenseMap.h>

#include <ikos/ar/semantic/code.hpp>

#include <ikos/analyzer/analysis/option.hpp>

namespace ikos {
namespace analyzer {

/// \brief Widening hints for a control flow graph
class WideningHints {
private:
  /// \brief Map of widening hints associated to a given cycle head
  using Map = llvm::DenseMap< ar::BasicBlock*, MachineInt >;

private:
  Map _map;

public:
  /// \brief Iterator over the widening hints
  using Iterator = Map::const_iterator;

public:
  /// \brief Constructor
  WideningHints() = default;

  /// \brief Copy constructor
  WideningHints(const WideningHints&) = default;

  /// \brief Move constructor
  WideningHints(WideningHints&&) = default;

  /// \brief Copy assignment operator
  WideningHints& operator=(const WideningHints&) = default;

  /// \brief Move assignment operator
  WideningHints& operator=(WideningHints&&) = default;

  /// \brief Destructor
  ~WideningHints() = default;

  /// \brief Return the widening hint for the given cycle head, if any
  boost::optional< const MachineInt& > get(ar::BasicBlock* head) const;

  /// \brief Add a widening hint for the given cycle head
  void add(ar::BasicBlock* head, const MachineInt& hint);

  /// \brief Begin iterator over the list of widening hints
  Iterator begin() const { return this->_map.begin(); }

  /// \brief End iterator over the list of widening hints
  Iterator end() const { return this->_map.end(); }

}; // end class WideningHints

/// \brief Fixpoint parameters for a control flow graph
class CodeFixpointParameters {
public:
  /// \brief Strategy for the increasing iterations (before reaching a fixpoint)
  WideningStrategy widening_strategy;

  /// \brief Strategy for the decreasing iterations (after reaching a fixpoint)
  NarrowingStrategy narrowing_strategy;

  /// \brief Number of loop iterations before applying the widening strategy
  unsigned widening_delay;

  /// \brief Number of loop iterations between each widening
  unsigned widening_period;

  /// \brief Fixed number of narrowing iterations to perform
  ///
  /// boost::none to perform narrowing iterations until convergence
  boost::optional< unsigned > narrowing_iterations;

  /// \brief Widening hints
  WideningHints widening_hints;

public:
  /// \brief Constructor
  CodeFixpointParameters(WideningStrategy widening_strategy_,
                         NarrowingStrategy narrowing_strategy_,
                         unsigned widening_delay_,
                         unsigned widening_period_,
                         boost::optional< unsigned > narrowing_iterations_);

  /// \brief Copy constructor
  CodeFixpointParameters(const CodeFixpointParameters&) = default;

  /// \brief Move constructor
  CodeFixpointParameters(CodeFixpointParameters&&) = default;

  /// \brief Copy assignment operator
  CodeFixpointParameters& operator=(const CodeFixpointParameters&) = default;

  /// \brief Move assignment operator
  CodeFixpointParameters& operator=(CodeFixpointParameters&&) = default;

  /// \brief Destructor
  ~CodeFixpointParameters() = default;

}; // end class CodeFixpointParameters

/// \brief Fixpoint parameters for the whole program
class FixpointParameters {
private:
  /// \brief Default fixpoint parameters
  CodeFixpointParameters _default_params;

  /// \brief Map that associates a function to fixpoint parameters
  llvm::DenseMap< ar::Function*, std::unique_ptr< CodeFixpointParameters > >
      _map;

public:
  /// \brief Constructor
  explicit FixpointParameters(const AnalysisOptions& opts);

  /// \brief No copy constructor
  FixpointParameters(const FixpointParameters&) = delete;

  /// \brief No move constructor
  FixpointParameters(FixpointParameters&&) = delete;

  /// \brief No copy assignment operator
  FixpointParameters& operator=(const FixpointParameters&) = delete;

  /// \brief No move assignment operator
  FixpointParameters& operator=(FixpointParameters&&) = delete;

  /// \brief Destructor
  ~FixpointParameters();

  /// \brief Default fixpoint parameters for a control flow graph
  const CodeFixpointParameters& default_params() const {
    return this->_default_params;
  }

  /// \brief Get or create the fixpoint parameters for the given function
  CodeFixpointParameters& get(ar::Function*);

  /// \brief Get the fixpoint parameters for the given function
  const CodeFixpointParameters& get(ar::Function*) const;

  /// \brief Dump the fixpoint parameters, for debugging purpose
  void dump(std::ostream& o) const;

}; // end class FixpointParameters

} // end namespace analyzer
} // end namespace ikos
