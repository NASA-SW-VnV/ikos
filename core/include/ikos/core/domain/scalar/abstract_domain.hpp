/*******************************************************************************
 *
 * \file
 * \brief Generic API for scalar abstract domains
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

#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/domain/machine_int/operator.hpp>
#include <ikos/core/domain/pointer/operator.hpp>
#include <ikos/core/linear_expression.hpp>
#include <ikos/core/number/machine_int.hpp>
#include <ikos/core/semantic/machine_int/variable.hpp>
#include <ikos/core/semantic/memory_location.hpp>
#include <ikos/core/semantic/scalar/variable.hpp>
#include <ikos/core/semantic/variable.hpp>
#include <ikos/core/value/machine_int/congruence.hpp>
#include <ikos/core/value/machine_int/interval.hpp>
#include <ikos/core/value/machine_int/interval_congruence.hpp>
#include <ikos/core/value/nullity.hpp>
#include <ikos/core/value/pointer/pointer.hpp>
#include <ikos/core/value/pointer/pointer_set.hpp>
#include <ikos/core/value/pointer/points_to_set.hpp>
#include <ikos/core/value/uninitialized.hpp>

namespace ikos {
namespace core {
namespace scalar {

/// \brief Base class for scalar abstract domains
///
/// A scalar is either a machine integer, a floating point or a pointer.
template < typename VariableRef, typename MemoryLocationRef, typename Derived >
class AbstractDomain : public core::AbstractDomain< Derived > {
public:
  static_assert(
      core::IsVariable< VariableRef >::value,
      "VariableRef does not meet the requirements for variable types");
  static_assert(machine_int::IsVariable< VariableRef >::value,
                "VariableRef must implement machine_int::VariableTraits");
  static_assert(scalar::IsVariable< VariableRef >::value,
                "VariableRef must implement scalar::VariableTraits");
  static_assert(core::IsMemoryLocation< MemoryLocationRef >::value,
                "MemoryLocationRef does not meet the requirements for memory "
                "location types");

public:
  using IntUnaryOperator = machine_int::UnaryOperator;
  using IntBinaryOperator = machine_int::BinaryOperator;
  using IntPredicate = machine_int::Predicate;
  using IntLinearExpression = LinearExpression< MachineInt, VariableRef >;
  using IntInterval = machine_int::Interval;
  using IntCongruence = machine_int::Congruence;
  using IntIntervalCongruence = machine_int::IntervalCongruence;
  using PointerPredicate = pointer::Predicate;
  using PointsToSetT = PointsToSet< MemoryLocationRef >;
  using PointerAbsValueT = PointerAbsValue< MemoryLocationRef >;
  using PointerSetT = PointerSet< MemoryLocationRef >;

public:
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

  /// \brief Perform the narrowing of two abstract values with a threshold
  virtual void narrow_threshold_with(const Derived& other,
                                     const MachineInt& threshold) = 0;

  /// \brief Perform the narrowing of two abstract values with a threshold
  virtual Derived narrowing_threshold(const Derived& other,
                                      const MachineInt& threshold) const {
    Derived tmp(static_cast< const Derived& >(*this));
    tmp.narrow_threshold_with(other, threshold);
    return tmp;
  }

  /// \name Uninitialized abstract domain methods
  /// @{

  /// \brief Add the constraint `x == initialized`
  virtual void uninit_assert_initialized(VariableRef x) = 0;

  /// \brief Return true if `x` is initialized, otherwise false
  virtual bool uninit_is_initialized(VariableRef x) const = 0;

  /// \brief Return true if `x` is uninitialized, otherwise false
  virtual bool uninit_is_uninitialized(VariableRef x) const = 0;

  /// \brief Refine the uninitialized value of a variable
  virtual void uninit_refine(VariableRef x, Uninitialized value) = 0;

  /// \brief Get the uninitialized value for the given variable
  virtual Uninitialized uninit_to_uninitialized(VariableRef x) const = 0;

  /// @}
  /// \name Machine integer abstract domain methods
  /// @{

  /// \brief Assign `x = n`
  virtual void int_assign(VariableRef x, const MachineInt& n) = 0;

  /// \brief Assign `x = undefined`
  virtual void int_assign_undef(VariableRef x) = 0;

  /// \brief Assign `x` to a non deterministic integer
  virtual void int_assign_nondet(VariableRef x) = 0;

  /// \brief Assign `x = y`
  virtual void int_assign(VariableRef x, VariableRef y) = 0;

  /// \brief Assign `x = e`
  ///
  /// Note that it wraps on integer overflow.
  /// Note that it will automatically cast variables to the type of `x`.
  virtual void int_assign(VariableRef x, const IntLinearExpression& e) = 0;

  /// \brief Apply `x = op y`
  virtual void int_apply(IntUnaryOperator op, VariableRef x, VariableRef y) = 0;

  /// \brief Apply `x = y op z`
  virtual void int_apply(IntBinaryOperator op,
                         VariableRef x,
                         VariableRef y,
                         VariableRef z) = 0;

  /// \brief Apply `x = y op z`
  virtual void int_apply(IntBinaryOperator op,
                         VariableRef x,
                         VariableRef y,
                         const MachineInt& z) = 0;

  /// \brief Apply `x = y op z`
  virtual void int_apply(IntBinaryOperator op,
                         VariableRef x,
                         const MachineInt& y,
                         VariableRef z) = 0;

  // \brief Add the constraint `x pred y`
  virtual void int_add(IntPredicate pred, VariableRef x, VariableRef y) = 0;

  // \brief Add the constraint `x pred y`
  virtual void int_add(IntPredicate pred,
                       VariableRef x,
                       const MachineInt& y) = 0;

  // \brief Add the constraint `x pred y`
  virtual void int_add(IntPredicate pred,
                       const MachineInt& x,
                       VariableRef y) = 0;

  /// \brief Set the interval value of a variable
  virtual void int_set(VariableRef x, const IntInterval& value) = 0;

  /// \brief Set the congruence value of a variable
  virtual void int_set(VariableRef x, const IntCongruence& value) = 0;

  /// \brief Set the interval-congruence value of a variable
  virtual void int_set(VariableRef x, const IntIntervalCongruence& value) = 0;

  /// \brief Refine the value of a variable with an interval
  virtual void int_refine(VariableRef x, const IntInterval& value) = 0;

  /// \brief Refine the value of a variable with a congruence
  virtual void int_refine(VariableRef x, const IntCongruence& value) = 0;

  /// \brief Refine the value of a variable with an interval-congruence
  virtual void int_refine(VariableRef x,
                          const IntIntervalCongruence& value) = 0;

  /// \brief Forget an integer variable
  virtual void int_forget(VariableRef x) = 0;

  /// \brief Projection to an interval
  ///
  /// Return an overapproximation of the value of `x` as an interval
  virtual IntInterval int_to_interval(VariableRef x) const = 0;

  /// \brief Projection to an interval
  ///
  /// Return an overapproximation of the linear expression `e` as an interval
  ///
  /// Note that it wraps on integer overflow.
  /// Note that it will automatically cast variables to the type of
  /// `e.constant()`.
  virtual IntInterval int_to_interval(const IntLinearExpression& e) const = 0;

  /// \brief Projection to a congruence
  ///
  /// Return an overapproximation of the value of `x` as a congruence
  virtual IntCongruence int_to_congruence(VariableRef x) const = 0;

  /// \brief Projection to a congruence
  ///
  /// Return an overapproximation of the linear expression `e` as a congruence
  ///
  /// Note that it wraps on integer overflow.
  /// Note that it will automatically cast variables to the type of
  /// `e.constant()`.
  virtual IntCongruence int_to_congruence(
      const IntLinearExpression& e) const = 0;

  /// \brief Projection to an interval-congruence
  ///
  /// Return an overapproximation of the value of `x` as an interval-congruence
  virtual IntIntervalCongruence int_to_interval_congruence(
      VariableRef x) const = 0;

  /// \brief Projection to an interval-congruence
  ///
  /// Return an overapproximation of the linear expression `e` as an
  /// interval-congruence
  ///
  /// Note that it wraps on integer overflow.
  /// Note that it will automatically cast variables to the type of
  /// `e.constant()`.
  virtual IntIntervalCongruence int_to_interval_congruence(
      const IntLinearExpression& e) const = 0;

  /// @}
  /// \name Non-negative loop counter abstract domain methods
  /// @{

  /// \brief Mark the variable `x` as a non-negative loop counter
  virtual void counter_mark(VariableRef x) = 0;

  /// \brief Mark the variable `x` as a normal variable, without losing
  /// information
  virtual void counter_unmark(VariableRef x) = 0;

  /// \brief Initialize a non-negative loop counter: `x = c`
  ///
  /// Precondition: `c >= 0`
  virtual void counter_init(VariableRef x, const MachineInt& c) = 0;

  /// \brief Increment a non-negative loop counter counter: `x += k`
  ///
  /// Precondition: `k >= 0`
  virtual void counter_incr(VariableRef x, const MachineInt& k) = 0;

  /// \brief Forget a non-negative loop counter
  virtual void counter_forget(VariableRef x) = 0;

  /// @}
  /// \name Floating point abstract domain methods
  /// @{

  /// \brief Assign `x = undefined`
  virtual void float_assign_undef(VariableRef x) = 0;

  /// \brief Assign `x` to a non deterministic floating point
  virtual void float_assign_nondet(VariableRef x) = 0;

  /// \brief Assign `x = y`
  virtual void float_assign(VariableRef x, VariableRef y) = 0;

  /// \brief Forget a floating point variable
  virtual void float_forget(VariableRef x) = 0;

  /// @}
  /// \name Nullity abstract domain methods
  /// @{

  /// \brief Add the constraint `p == null`
  virtual void nullity_assert_null(VariableRef p) = 0;

  /// \brief Add the constraint `p != null`
  virtual void nullity_assert_non_null(VariableRef p) = 0;

  /// \brief Return true if `p` is null, otherwise false
  virtual bool nullity_is_null(VariableRef p) const = 0;

  /// \brief Return true if `p` is non null, otherwise false
  virtual bool nullity_is_non_null(VariableRef p) const = 0;

  /// \brief Set the nullity of the pointer `p` to the given value
  virtual void nullity_set(VariableRef p, Nullity value) = 0;

  /// \brief Refine the pointer `p` with the given nullity value
  virtual void nullity_refine(VariableRef p, Nullity value) = 0;

  /// \brief Get the nullity value for the given variable
  virtual Nullity nullity_to_nullity(VariableRef p) const = 0;

  /// @}
  /// \name Pointer abstract domain methods
  /// @{

  /// \brief Assign `p` to an address (i.e, memory location)
  ///
  /// This is equivalent to `p = &x` or `p = malloc()`
  virtual void pointer_assign(VariableRef p,
                              MemoryLocationRef addr,
                              Nullity nullity) = 0;

  /// \brief Assign `p = null`
  virtual void pointer_assign_null(VariableRef p) = 0;

  /// \brief Assign `p = undefined`
  virtual void pointer_assign_undef(VariableRef p) = 0;

  /// \brief Assign `p` to a non deterministic pointer
  virtual void pointer_assign_nondet(VariableRef p) = 0;

  /// \brief Assign `p = q`
  virtual void pointer_assign(VariableRef p, VariableRef q) = 0;

  /// \brief Assign `p = q + o`
  virtual void pointer_assign(VariableRef p, VariableRef q, VariableRef o) = 0;

  /// \brief Assign `p = q + o`
  virtual void pointer_assign(VariableRef p,
                              VariableRef q,
                              const MachineInt& o) = 0;

  /// \brief Assign `p = q + o`
  virtual void pointer_assign(VariableRef p,
                              VariableRef q,
                              const IntLinearExpression& o) = 0;

  // \brief Add the constraint `p pred q`
  virtual void pointer_add(PointerPredicate pred,
                           VariableRef p,
                           VariableRef q) = 0;

  /// \brief Refine the pointer `p` with the given set of addresses
  virtual void pointer_refine(VariableRef p, const PointsToSetT& addrs) = 0;

  /// \brief Refine the pointer `p` with the given set of addresses and offsets
  virtual void pointer_refine(VariableRef p,
                              const PointsToSetT& addrs,
                              const IntInterval& offset) = 0;

  /// \brief Refine the pointer `p` with the given pointer abstract value
  virtual void pointer_refine(VariableRef p, const PointerAbsValueT& value) = 0;

  /// \brief Refine the pointer `p` with the given pointer set
  virtual void pointer_refine(VariableRef p, const PointerSetT& set) = 0;

  /// \brief Assign `x = offset-of p`
  virtual void pointer_offset_to_int(VariableRef x, VariableRef p) = 0;

  /// \brief Project the offset of the pointer `p` to an interval
  virtual IntInterval pointer_offset_to_interval(VariableRef p) const = 0;

  /// \brief Project the offset of the pointer `p` to a congruence
  virtual IntCongruence pointer_offset_to_congruence(VariableRef p) const = 0;

  /// \brief Project the offset of the pointer `p` to an interval-congruence
  virtual IntIntervalCongruence pointer_offset_to_interval_congruence(
      VariableRef p) const = 0;

  /// \brief Return the points-to set of `p`
  virtual PointsToSetT pointer_to_points_to(VariableRef p) const = 0;

  /// \brief Return the pointer abstract value of `p`
  virtual PointerAbsValueT pointer_to_pointer(VariableRef p) const = 0;

  /// \brief Forget the offset of a pointer variable
  virtual void pointer_forget_offset(VariableRef p) = 0;

  /// \brief Forget a pointer variable
  virtual void pointer_forget(VariableRef p) = 0;

  /// @}
  /// \name Dynamically typed variables abstract domain methods
  /// @{

  /// \brief Assign a dynamically typed variable to a dynamically typed variable
  virtual void dynamic_assign(VariableRef x, VariableRef y) = 0;

  /// \brief Write undefined to a dynamically typed variable
  virtual void dynamic_write_undef(VariableRef x) = 0;

  /// \brief Write a non deterministic value to a dynamically typed variable
  virtual void dynamic_write_nondet(VariableRef x) = 0;

  /// \brief Write an integer to a dynamically typed variable
  virtual void dynamic_write_int(VariableRef x, const MachineInt& n) = 0;

  /// \brief Write a non deterministic integer to a dynamically typed variable
  virtual void dynamic_write_nondet_int(VariableRef x) = 0;

  /// \brief Write an integer variable to a dynamically typed variable
  virtual void dynamic_write_int(VariableRef x, VariableRef y) = 0;

  /// \brief Write a non deterministic float to a dynamically typed variable
  virtual void dynamic_write_nondet_float(VariableRef x) = 0;

  /// \brief Write null to a dynamically typed variable
  virtual void dynamic_write_null(VariableRef x) = 0;

  /// \brief Write a pointer to a dynamically typed variable
  virtual void dynamic_write_pointer(VariableRef x,
                                     MemoryLocationRef addr,
                                     Nullity nullity) = 0;

  /// \brief Write a pointer variable to a dynamically typed variable
  virtual void dynamic_write_pointer(VariableRef x, VariableRef y) = 0;

  /// \brief Read an integer variable from a dynamically typed variable
  virtual void dynamic_read_int(VariableRef x, VariableRef y) = 0;

  /// \brief Read a pointer variable from a dynamically typed variable
  virtual void dynamic_read_pointer(VariableRef x, VariableRef y) = 0;

  /// \brief Return true if `x` is the integer zero
  virtual bool dynamic_is_zero(VariableRef x) const = 0;

  /// \brief Return true if `x` is the null pointer
  virtual bool dynamic_is_null(VariableRef x) const = 0;

  /// \brief Forget a dynamically typed variable
  virtual void dynamic_forget(VariableRef x) = 0;

  /// @}
  /// \name Scalar abstract domain methods
  /// @{

  /// \brief Assign `x = undefined`
  virtual void scalar_assign_undef(VariableRef x) = 0;

  /// \brief Assign `x` to a non deterministic value
  virtual void scalar_assign_nondet(VariableRef x) = 0;

  /// \brief Assign `x = ptr-to-int p`
  virtual void scalar_pointer_to_int(VariableRef x,
                                     VariableRef p,
                                     MemoryLocationRef absolute_zero) = 0;

  /// \brief Assign `p = int-to-ptr x`
  virtual void scalar_int_to_pointer(VariableRef p,
                                     VariableRef x,
                                     MemoryLocationRef absolute_zero) = 0;

  /// \brief Forget a scalar variable
  virtual void scalar_forget(VariableRef x) = 0;

  /// @}

}; // end class AbstractDomain

/// \brief Check if a type is a scalar abstract domain
template < typename T, typename VariableRef, typename MemoryLocationRef >
struct IsAbstractDomain
    : std::is_base_of<
          scalar::AbstractDomain< VariableRef, MemoryLocationRef, T >,
          T > {};

} // end namespace scalar
} // end namespace core
} // end namespace ikos
