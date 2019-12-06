/*******************************************************************************
 *
 * \file
 * \brief Function fixpoint cache for dynamic inlining
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2019 United States Government as represented by the
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
#include <mutex>

#include <boost/container/flat_map.hpp>

#include <llvm/ADT/DenseMap.h>

#include <ikos/ar/semantic/function.hpp>
#include <ikos/ar/semantic/statement.hpp>

namespace ikos {
namespace analyzer {

/// \brief Function fixpoint cache for dynamic inlining
///
/// This class stores computed function fixpoints for a given call site
template < typename FunctionFixpoint, typename AbstractDomain >
class FixpointCache {
private:
  /// \brief Map from callee function to FunctionFixpoint
  using CalleeMap =
      boost::container::flat_map< ar::Function*,
                                  std::unique_ptr< FunctionFixpoint > >;

  /// \brief Map from call statement to CalleeMap
  using CallMap = llvm::DenseMap< ar::CallBase*, CalleeMap >;

private:
  std::mutex _mutex;
  CallMap _call_map;

public:
  /// \brief Constructor
  FixpointCache() = default;

  /// \brief No copy constructor
  FixpointCache(const FixpointCache&) = delete;

  /// \brief No move constructor
  FixpointCache(FixpointCache&&) = delete;

  /// \brief No copy assignment operator
  FixpointCache& operator=(const FixpointCache&) = delete;

  /// \brief No move assignment operator
  FixpointCache& operator=(FixpointCache&&) = delete;

  /// \brief Destructor
  ~FixpointCache() = default;

  /// \brief Try to fetch a fixpoint for a given call site and callee function
  std::unique_ptr< FunctionFixpoint > try_fetch(ar::CallBase* call,
                                                ar::Function* callee) {
    std::lock_guard< std::mutex > lock(this->_mutex);
    auto call_it = this->_call_map.find(call);
    if (call_it == this->_call_map.end()) {
      return nullptr;
    }
    CalleeMap& callee_map = call_it->second;
    auto callee_it = callee_map.find(callee);
    if (callee_it == callee_map.end()) {
      return nullptr;
    }
    return std::move(callee_it->second);
  }

  /// \brief Erase the fixpoint for a given call site and callee function
  void erase(ar::CallBase* call, ar::Function* callee) {
    std::lock_guard< std::mutex > lock(this->_mutex);
    auto call_it = this->_call_map.find(call);
    if (call_it == this->_call_map.end()) {
      return;
    }
    CalleeMap& callee_map = call_it->second;
    auto callee_it = callee_map.find(callee);
    if (callee_it == callee_map.end()) {
      return;
    }
    callee_it->second.reset();
  }

  /// \brief Store a fixpoint for a given call site and callee function
  void store(ar::CallBase* call,
             ar::Function* callee,
             std::unique_ptr< FunctionFixpoint > fixpoint) {
    std::lock_guard< std::mutex > lock(this->_mutex);
    this->_call_map[call][callee] = std::move(fixpoint);
  }

}; // end class FixpointCache

} // end namespace analyzer
} // end namespace ikos
