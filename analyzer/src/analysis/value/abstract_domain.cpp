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

#include <ikos/core/domain/lifetime/separate_domain.hpp>
#include <ikos/core/domain/memory/partitioning.hpp>
#include <ikos/core/domain/memory/value.hpp>
#include <ikos/core/domain/nullity/separate_domain.hpp>
#include <ikos/core/domain/scalar/composite.hpp>
#include <ikos/core/domain/uninitialized/separate_domain.hpp>

#include <ikos/analyzer/analysis/value/abstract_domain.hpp>
#include <ikos/analyzer/analysis/value/machine_int_domain.hpp>

namespace ikos {
namespace analyzer {
namespace value {

namespace {

/// \brief Uninitialized abstract domain
using UninitializedAbstractDomain =
    core::uninitialized::SeparateDomain< Variable* >;

/// \brief Nullity abstract domain
using NullityAbstractDomain = core::nullity::SeparateDomain< Variable* >;

/// \brief Scalar abstract domain
using ScalarAbstractDomain =
    core::scalar::CompositeDomain< Variable*,
                                   MemoryLocation*,
                                   UninitializedAbstractDomain,
                                   MachineIntAbstractDomain,
                                   NullityAbstractDomain >;

/// \brief Lifetime abstract domain
using LifetimeAbstractDomain =
    core::lifetime::SeparateDomain< MemoryLocation* >;

/// \brief Value abstract domain
using ValueAbstractDomain = core::memory::ValueDomain< Variable*,
                                                       MemoryLocation*,
                                                       VariableFactory*,
                                                       ScalarAbstractDomain,
                                                       LifetimeAbstractDomain >;

/// \brief Partitioning abstract domain
using PartitioningAbstractDomain = core::memory::
    PartitioningDomain< Variable*, MemoryLocation*, ValueAbstractDomain >;

/// \brief Create the bottom memory abstract value
MemoryAbstractDomain make_bottom_memory_abstract_value(Context& ctx) {
  auto inv = ValueAbstractDomain(
      ctx.var_factory,
      ScalarAbstractDomain(UninitializedAbstractDomain::bottom(),
                           make_bottom_machine_int_abstract_value(
                               ctx.opts.machine_int_domain),
                           NullityAbstractDomain::bottom()),
      LifetimeAbstractDomain::bottom());

  if (ctx.opts.use_partitioning_domain) {
    return MemoryAbstractDomain(PartitioningAbstractDomain(inv));
  } else {
    return MemoryAbstractDomain(inv);
  }
}

/// \brief Create the top memory abstract value
MemoryAbstractDomain make_top_memory_abstract_value(Context& ctx) {
  auto inv = ValueAbstractDomain(
      ctx.var_factory,
      ScalarAbstractDomain(UninitializedAbstractDomain::top(),
                           make_top_machine_int_abstract_value(
                               ctx.opts.machine_int_domain),
                           NullityAbstractDomain::top()),
      LifetimeAbstractDomain::top());

  if (ctx.opts.use_partitioning_domain) {
    return MemoryAbstractDomain(PartitioningAbstractDomain(inv));
  } else {
    return MemoryAbstractDomain(inv);
  }
}

} // end anonymous namespace

AbstractDomain make_bottom_abstract_value(Context& ctx) {
  return AbstractDomain(/* normal = */
                        make_bottom_memory_abstract_value(ctx),
                        /* caught_exceptions = */
                        make_bottom_memory_abstract_value(ctx),
                        /* propagated_exceptions = */
                        make_bottom_memory_abstract_value(ctx));
}

AbstractDomain make_initial_abstract_value(Context& ctx) {
  return AbstractDomain(/* normal = */
                        make_top_memory_abstract_value(ctx),
                        /* caught_exceptions = */
                        make_bottom_memory_abstract_value(ctx),
                        /* propagated_exceptions = */
                        make_bottom_memory_abstract_value(ctx));
}

} // end namespace value
} // end namespace analyzer
} // end namespace ikos
