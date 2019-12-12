/******************************************************************************
 *
 * \file
 * \brief Generic API for abstract domains keeping track of memory location
 * lifetimes
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

#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/semantic/memory_location.hpp>
#include <ikos/core/value/lifetime.hpp>

namespace ikos {
namespace core {
namespace lifetime {

/// \brief Base class for abstract domains keeping track of memory location
/// lifetimes
template < typename MemoryLocationRef, typename Derived >
class AbstractDomain : public core::AbstractDomain< Derived > {
public:
  static_assert(core::IsMemoryLocation< MemoryLocationRef >::value,
                "MemoryLocationRef does not meet the requirements for memory "
                "location types");

public:
  /// \brief Assign `m = allocated`
  virtual void assign_allocated(MemoryLocationRef m) = 0;

  /// \brief Assign `m = deallocated`
  virtual void assign_deallocated(MemoryLocationRef m) = 0;

  /// \brief Add the constraint `m == allocated`
  virtual void assert_allocated(MemoryLocationRef m) = 0;

  /// \brief Add the constraint `m == deallocated`
  virtual void assert_deallocated(MemoryLocationRef m) = 0;

  /// \brief Return true if `m` is allocated, otherwise false
  virtual bool is_allocated(MemoryLocationRef m) const = 0;

  /// \brief Return true if `m` is deallocated, otherwise false
  virtual bool is_deallocated(MemoryLocationRef m) const = 0;

  /// \brief Set the lifetime of a memory location
  virtual void set(MemoryLocationRef m, const Lifetime& value) = 0;

  /// \brief Forget the lifetime of a memory location
  virtual void forget(MemoryLocationRef m) = 0;

  /// \brief Get the lifetime value for the given memory location
  virtual Lifetime get(MemoryLocationRef m) const = 0;

}; // end class AbstractDomain

/// \brief Check if a type is a lifetime abstract domain
template < typename T, typename MemoryLocationRef >
struct IsAbstractDomain
    : std::is_base_of< lifetime::AbstractDomain< MemoryLocationRef, T >, T > {};

} // end namespace lifetime
} // end namespace core
} // end namespace ikos
