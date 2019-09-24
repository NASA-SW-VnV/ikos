/*******************************************************************************
 *
 * \file
 * \brief Generic API for scalar variables
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

#include <ikos/core/support/mpl.hpp>

namespace ikos {
namespace core {
namespace scalar {

/// \brief Traits for scalar variables
///
/// Elements to provide:
///
/// static bool is_int(VariableRef)
///   Return true if the given variable is a machine integer variable
///
/// static bool is_float(VariableRef)
///   Return true if the given variable is a floating point variable
///
/// static bool is_pointer(VariableRef)
///   Return true if the given variable is a pointer variable
///
/// static bool is_dynamic(VariableRef)
///   Return true if the given variable is a dynamically typed variable
///
/// static VariableRef offset_var(VariableRef)
///   Return the machine integer offset variable of the given pointer variable
template < typename VariableRef >
struct VariableTraits {};

/// \brief Check if a type implements VariableTraits
template < typename VariableRef,
           typename VariableTrait = VariableTraits< VariableRef >,
           typename = void >
struct IsVariable : std::false_type {};

template < typename VariableRef, typename VariableTrait >
struct IsVariable<
    VariableRef,
    VariableTrait,
    void_t< std::enable_if_t<
                std::is_same< bool,
                              decltype(VariableTrait::is_int(
                                  std::declval< VariableRef >())) >::value >,
            std::enable_if_t<
                std::is_same< bool,
                              decltype(VariableTrait::is_float(
                                  std::declval< VariableRef >())) >::value >,
            std::enable_if_t<
                std::is_same< bool,
                              decltype(VariableTrait::is_pointer(
                                  std::declval< VariableRef >())) >::value >,
            std::enable_if_t<
                std::is_same< bool,
                              decltype(VariableTrait::is_dynamic(
                                  std::declval< VariableRef >())) >::value >,
            std::enable_if_t<
                std::is_same< VariableRef,
                              decltype(VariableTrait::offset_var(
                                  std::declval< VariableRef >())) >::value > > >
    : std::true_type {};

} // end namespace scalar
} // end namespace core
} // end namespace ikos
