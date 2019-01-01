/*******************************************************************************
 *
 * \file
 * \brief Abstract domain for the value analysis
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

#include <ikos/analyzer/analysis/value/machine_int_domain.hpp>
#include <ikos/core/domain/exception/exception.hpp>
#include <ikos/core/domain/lifetime/lifetime.hpp>
#include <ikos/core/domain/memory/value.hpp>
#include <ikos/core/domain/nullity/nullity.hpp>
#include <ikos/core/domain/pointer/pointer.hpp>
#include <ikos/core/domain/uninitialized/uninitialized.hpp>

#include <ikos/analyzer/analysis/memory_location.hpp>
#include <ikos/analyzer/analysis/variable.hpp>

namespace ikos {
namespace analyzer {
namespace value {

/// \brief Nullity abstract domain for the value analysis
using NullityAbstractDomain = core::nullity::NullityDomain< Variable* >;

/// \brief Pointer abstract domain for the value analysis
using PointerAbstractDomain =
    core::pointer::PointerDomain< Variable*,
                                  MemoryLocation*,
                                  MachineIntAbstractDomain,
                                  NullityAbstractDomain >;

/// \brief Uninitialized abstract domain for the value analysis
using UninitializedAbstractDomain =
    core::uninitialized::UninitializedDomain< Variable* >;

/// \brief Lifetime abstract domain for the value analysis
using LifetimeAbstractDomain =
    core::lifetime::LifetimeDomain< MemoryLocation* >;

/// \brief Memory abstract domain for the value analysis
using MemoryAbstractDomain =
    core::memory::ValueDomain< Variable*,
                               MemoryLocation*,
                               VariableFactory,
                               MachineIntAbstractDomain,
                               NullityAbstractDomain,
                               PointerAbstractDomain,
                               UninitializedAbstractDomain,
                               LifetimeAbstractDomain >;

/// \brief Abstract domain for the value analysis
using AbstractDomain = core::exception::ExceptionDomain< MemoryAbstractDomain >;

} // end namespace value
} // end namespace analyzer
} // end namespace ikos
