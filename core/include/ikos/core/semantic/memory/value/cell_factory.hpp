/*******************************************************************************
 *
 * \file
 * \brief Generic API for creating variables representing memory cells
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

#include <ikos/core/number/machine_int.hpp>
#include <ikos/core/support/mpl.hpp>

namespace ikos {
namespace core {
namespace memory {

/// \brief Traits for creating cell variables
///
/// Requirements:
///
/// CellFactoryRef has a noexcept copy constructor
/// CellFactoryRef has a noexcept move constructor
/// CellFactoryRef has a noexcept copy assignment operator
/// CellFactoryRef has a noexcept move assignment operator
///
/// Elements to provide:
///
/// static VariableRef cell(CellFactoryRef factory,
///                         MemoryLocationRef base,
///                         const MachineInt& offset,
///                         const MachineInt& size,
///                         Signedness sign)
///   Get or create the cell with the given base address, offset and size
///   If a new cell is created, it will have the given signedness
template < typename VariableRef,
           typename MemoryLocationRef,
           typename CellFactoryRef >
struct CellFactoryTraits {};

/// \brief Check if a type implements CellFactoryTraits
template <
    typename VariableRef,
    typename MemoryLocationRef,
    typename CellFactoryRef,
    typename CellFactoryTrait =
        CellFactoryTraits< VariableRef, MemoryLocationRef, CellFactoryRef >,
    typename = void >
struct IsCellFactory : std::false_type {};

template < typename VariableRef,
           typename MemoryLocationRef,
           typename CellFactoryRef,
           typename CellFactoryTrait >
struct IsCellFactory<
    VariableRef,
    MemoryLocationRef,
    CellFactoryRef,
    CellFactoryTrait,
    std::enable_if_t< conjunction<
        std::is_nothrow_copy_constructible< CellFactoryRef >,
        std::is_nothrow_move_constructible< CellFactoryRef >,
        std::is_nothrow_copy_assignable< CellFactoryRef >,
        std::is_nothrow_move_assignable< CellFactoryRef >,
        std::is_same<
            VariableRef,
            decltype(CellFactoryTrait::cell(std::declval< CellFactoryRef >(),
                                            std::declval< MemoryLocationRef >(),
                                            std::declval< const MachineInt& >(),
                                            std::declval< const MachineInt& >(),
                                            std::declval< Signedness >())) > >::
                          value > > : std::true_type {};

} // end namespace memory
} // end namespace core
} // end namespace ikos
