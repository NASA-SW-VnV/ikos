/******************************************************************************
 *
 * \file
 * \brief Pointer abstractions
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

#include <unordered_map>

#include <ikos/core/value/machine_int/interval.hpp>
#include <ikos/core/value/pointer/pointer.hpp>
#include <ikos/core/value/pointer/points_to_set.hpp>

#include <ikos/ar/semantic/data_layout.hpp>

#include <ikos/analyzer/analysis/memory_location.hpp>
#include <ikos/analyzer/analysis/variable.hpp>

namespace ikos {
namespace analyzer {

/// \brief Set of memory locations
using PointsToSet = core::PointsToSet< MemoryLocation* >;

/// \brief Machine integer interval
using MachineIntInterval = core::machine_int::Interval;

/// \brief Pointer abstraction using a points-to set and an interval
using PointerAbsValue = core::PointerAbsValue< MemoryLocation* >;

/// \brief Hold pointer information
class PointerInfo {
private:
  /// \brief Map from variable to pointer value
  using PointerMap = std::unordered_map< Variable*, PointerAbsValue >;

private:
  /// \brief Map from variables to pointer abstract values
  PointerMap _map;

  /// \brief Data layout
  const ar::DataLayout& _data_layout;

public:
  /// \brief Constructor
  explicit PointerInfo(const ar::DataLayout& data_layout);

  /// \brief No copy constructor
  PointerInfo(const PointerInfo&) = delete;

  /// \brief No move constructor
  PointerInfo(PointerInfo&&) = delete;

  /// \brief No copy assignment operator
  PointerInfo& operator=(const PointerInfo&) = delete;

  /// \brief No move assignment operator
  PointerInfo& operator=(PointerInfo&&) = delete;

  /// \brief Destructor
  ~PointerInfo();

  /// \brief Clear all information
  void clear();

  /// \brief Return information about the given pointer
  PointerAbsValue get(Variable* v) const;

  /// \brief Insert an information about a pointer
  void insert(Variable* v, const PointerAbsValue&);

  /// \brief Dump the pointer constraints, for debugging purpose
  void dump(std::ostream&) const;

}; // end class PointerInfo

} // end namespace analyzer
} // end namespace ikos
