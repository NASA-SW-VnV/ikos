/*******************************************************************************
 *
 * \file
 * \brief Requirements for MemoryLocationRef types
 *
 * Author: Maxime Arthaud
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

#include <ikos/core/semantic/dumpable.hpp>
#include <ikos/core/semantic/indexable.hpp>
#include <ikos/core/support/mpl.hpp>

namespace ikos {
namespace core {

/// \brief Check if the given type meets the requirements for memory location
/// types
///
/// Requirements:
///
/// MemoryLocationRef has a noexcept copy constructor
/// MemoryLocationRef has a noexcept move constructor
/// MemoryLocationRef has a noexcept copy assignment operator
/// MemoryLocationRef has a noexcept move assignment operator
///
/// bool operator==(MemoryLocationRef x, MemoryLocationRef y)
///   Return true if x and y refers to the same memory location
///
/// bool operator<(MemoryLocationref x, MemoryLocationref y)
///   Return true if index(x) < index(y)
///
/// MemoryLocationRef implements DumpableTraits
///
/// MemoryLocationRef implements IndexableTraits
///
/// The MemoryLocationRef type should be cheap to copy.
template < typename MemoryLocationRef >
struct IsMemoryLocation
    : conjunction< std::is_nothrow_copy_constructible< MemoryLocationRef >,
                   std::is_nothrow_move_constructible< MemoryLocationRef >,
                   std::is_nothrow_copy_assignable< MemoryLocationRef >,
                   std::is_nothrow_move_assignable< MemoryLocationRef >,
                   supports_equality< MemoryLocationRef, MemoryLocationRef >,
                   supports_less_than< MemoryLocationRef, MemoryLocationRef >,
                   IsDumpable< MemoryLocationRef >,
                   IsIndexable< MemoryLocationRef > > {};

} // end namespace core
} // end namespace ikos
