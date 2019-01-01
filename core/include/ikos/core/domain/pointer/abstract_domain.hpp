/*******************************************************************************
 *
 * \file
 * \brief Generic API for pointer abstract domains
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

#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/domain/machine_int/abstract_domain.hpp>
#include <ikos/core/domain/nullity/abstract_domain.hpp>
#include <ikos/core/domain/pointer/operator.hpp>
#include <ikos/core/semantic/memory_location.hpp>
#include <ikos/core/semantic/pointer/variable.hpp>
#include <ikos/core/semantic/variable.hpp>
#include <ikos/core/value/pointer/pointer.hpp>
#include <ikos/core/value/pointer/pointer_set.hpp>
#include <ikos/core/value/pointer/points_to_set.hpp>

namespace ikos {
namespace core {
namespace pointer {

/// \brief Base class for pointer abstract domains
///
/// Note that it holds the following underlying abstract domains:
///   * `MachineIntDomain` for machine integers: to express invariants on
///     offsets and machine integer variables.
///   * `NullityDomain`: to express nullity of pointers
template < typename VariableRef,
           typename MemoryLocationRef,
           typename MachineIntDomain,
           typename NullityDomain,
           typename Derived >
class AbstractDomain : public core::AbstractDomain< Derived > {
public:
  static_assert(
      core::IsVariable< VariableRef >::value,
      "VariableRef does not meet the requirements for variable types");
  static_assert(pointer::IsVariable< VariableRef >::value,
                "VariableRef must implement pointer::VariableTraits");
  static_assert(core::IsMemoryLocation< MemoryLocationRef >::value,
                "MemoryLocationRef does not meet the requirements for memory "
                "location types");
  static_assert(
      machine_int::IsAbstractDomain< MachineIntDomain, VariableRef >::value,
      "MachineIntDomain must implement machine_int::AbstractDomain");
  static_assert(nullity::IsAbstractDomain< NullityDomain, VariableRef >::value,
                "NullityDomain must implement nullity::AbstractDomain");

public:
  using LinearExpressionT = LinearExpression< MachineInt, VariableRef >;
  using PointsToSetT = PointsToSet< MemoryLocationRef >;
  using PointerAbsValueT = PointerAbsValue< MemoryLocationRef >;
  using PointerSetT = PointerSet< MemoryLocationRef >;

public:
  /// \brief Provide access to the underlying machine integer abstract domain
  virtual MachineIntDomain& integers() = 0;

  /// \brief Provide access to the underlying machine integer abstract domain
  virtual const MachineIntDomain& integers() const = 0;

  /// \brief Provide access to the underlying nullity abstract domain
  virtual NullityDomain& nullity() = 0;

  /// \brief Provide access to the underlying nullity abstract domain
  virtual const NullityDomain& nullity() const = 0;

  /// \brief Perform the widening of two abstract values with a threshold
  virtual void widen_threshold_with(const Derived& other,
                                    const MachineInt& threshold) = 0;

  /// \brief Perform the widening of two abstract values with a threshold
  virtual Derived widening_threshold(const Derived& other,
                                     const MachineInt& threshold) const {
    Derived tmp(static_cast< const Derived& >(*this));
    tmp.widen_threshold_with(other, threshold);
    return tmp;
  }

  /// \brief Assign `p` to an address (i.e, memory location)
  ///
  /// This is equivalent to `p = &x` or `p = malloc()`
  virtual void assign_address(VariableRef p,
                              MemoryLocationRef addr,
                              Nullity nullity) = 0;

  /// \brief Assign `p` to null
  ///
  /// This is equivalent to `p = null`
  virtual void assign_null(VariableRef p) = 0;

  /// \brief Assign `p` to undefined
  ///
  /// This is equivalent to `p = undef`
  virtual void assign_undef(VariableRef p) = 0;

  /// \brief Assign `p = q`
  virtual void assign(VariableRef p, VariableRef q) = 0;

  /// \brief Assign `p = q + o`
  virtual void assign(VariableRef p, VariableRef q, VariableRef o) = 0;

  /// \brief Assign `p = q + o`
  virtual void assign(VariableRef p, VariableRef q, const MachineInt& o) = 0;

  /// \brief Assign `p = q + o`
  virtual void assign(VariableRef p,
                      VariableRef q,
                      const LinearExpressionT& o) = 0;

  /// \brief Add the constraint `p == null`
  virtual void assert_null(VariableRef p) = 0;

  /// \brief Add the constraint `p != null`
  virtual void assert_non_null(VariableRef p) = 0;

  // \brief Add the constraint `p pred q`
  virtual void add(Predicate pred, VariableRef p, VariableRef q) = 0;

  /// \brief Refine the pointer `p` with the given set of addresses
  virtual void refine(VariableRef p, const PointsToSetT& addrs) = 0;

  /// \brief Refine the pointer `p` with the given set of addresses and offsets
  virtual void refine(VariableRef p,
                      const PointsToSetT& addrs,
                      const machine_int::Interval& offset) = 0;

  /// \brief Refine the pointer `p` with the given pointer abstract value
  virtual void refine(VariableRef p, const PointerAbsValueT& value) = 0;

  /// \brief Refine the pointer `p` with the given pointer set
  virtual void refine(VariableRef p, const PointerSetT& set) = 0;

  /// \brief Return the points-to set of `p`
  virtual PointsToSetT points_to(VariableRef p) const = 0;

  /// \brief Return the offset variable associated to `p`
  VariableRef offset_var(VariableRef p) const {
    return pointer::VariableTraits< VariableRef >::offset_var(p);
  }

  /// \brief Return the pointer abstract value of `p`
  virtual PointerAbsValueT get(VariableRef p) const = 0;

  /// \brief Forget a pointer variable
  virtual void forget(VariableRef p) = 0;

  /// \brief Normalize the abstract value
  virtual void normalize() const = 0;

}; // end class AbstractDomain

/// \brief Check if a type is a pointer abstract domain
template < typename T,
           typename VariableRef,
           typename MemoryLocationRef,
           typename MachineIntDomain,
           typename NullityDomain >
struct IsAbstractDomain
    : std::is_base_of< pointer::AbstractDomain< VariableRef,
                                                MemoryLocationRef,
                                                MachineIntDomain,
                                                NullityDomain,
                                                T >,
                       T > {};

} // end namespace pointer
} // end namespace core
} // end namespace ikos
