/*******************************************************************************
 *
 * \file
 * \brief Generic API for indexing objects
 *
 * Author: Maxime Arthaud
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

#include <cstddef>
#include <cstdint>

#include <ikos/core/support/mpl.hpp>

namespace ikos {
namespace core {

/// \brief Type for unique indexes
///
/// This unsigned type should be capable of holding a pointer.
using Index = std::uintptr_t;

/// \brief Traits for indexable objects
///
/// IndexableTraits is a structure of traits for indexable objects. These
/// objects are used e.g. by the patricia tree for organizing the objects in the
/// tree.
///
/// An indexable object must provide:
///
/// static Index index(const T&)
///   Return a unique index
///
/// The trait has to be specialized for each specific type.
///
/// The traits can be used as:
/// \code{.cpp}
///   Index index = IndexableTraits< VariableRef >::index(my_variable_ref);
/// \endcode
template < typename T >
struct IndexableTraits {};

/// \brief IndexableTraits implementation for Index
template <>
struct IndexableTraits< Index > {
  static Index index(Index idx) { return idx; }
};

/// \brief Check if a type implements IndexableTraits
template < typename T,
           typename IndexableTrait = IndexableTraits< T >,
           typename = void >
struct IsIndexable : std::false_type {};

template < typename T, typename IndexableTrait >
struct IsIndexable<
    T,
    IndexableTrait,
    // Check if IndexableTrait has: index(const T&) -> Index
    std::enable_if_t< std::is_same< Index,
                                    decltype(IndexableTrait::index(
                                        std::declval< T >())) >::value > >
    : std::true_type {};

} // end namespace core
} // end namespace ikos
