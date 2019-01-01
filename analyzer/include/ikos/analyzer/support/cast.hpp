/*******************************************************************************
 *
 * \file
 * \brief Casting definitions (isa, cast, dyn_cast)
 *
 * This header includes:
 *   * ikos/core/support/cast.hpp
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017-2019 United States Government as represented by the
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

#include <ikos/core/support/cast.hpp>

namespace ikos {
namespace analyzer {

/// \brief Check if the parameter is an instance of the template type argument
///
/// Used like this:
///
/// \code{.cpp}
/// if (isa< GlobalVariable >(v)) { ... }
/// \endcode
using core::isa;

/// \brief Return the argument parameter cast to the specified type
///
/// This casting operator asserts that the type is correct. It does not allow
/// a null argument.
///
/// Used like this:
///
/// \code{.cpp}
/// Instruction* inst = cast< Instruction >(val)
/// \endcode
using core::cast;

/// \brief Return the argument parameter cast to the specified type
///
/// This is equivalent to cast< X >, except that a null value is allowed.
using core::cast_or_null;

/// \brief Return the argument parameter cast to the specified type
///
/// This casting operator returns null if the argument is of the wrong type, so
/// it can used to test for a type as well as cast if successful.
///
/// Used like this:
///
/// \code{.cpp}
/// if (Instruction* inst = dyn_cast< Instruction >(val)) { ... }
/// \endcode
using core::dyn_cast;

/// \brief Return the argument parameter cast to the specified type
///
/// This is equivalent to dyn_cast< X >, except that a null value is allowed.
using core::dyn_cast_or_null;

} // end namespace analyzer
} // end namespace ikos
