/*******************************************************************************
 *
 * \file
 * \brief Polymorphic memory abstract domain
 *
 * The PolymorphicDomain is a memory abstract domain whose behavior depends on
 * the abstract domain it is constructed with. It allows the use of different
 * abstract domains at runtime.
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

#include <memory>

#include <ikos/core/domain/memory/abstract_domain.hpp>
#include <ikos/core/support/assert.hpp>
#include <ikos/core/support/mpl.hpp>

namespace ikos {
namespace core {
namespace memory {

/// \brief Polymorphic memory abstract domain
///
/// The PolymorphicDomain is a memory abstract domain whose behavior depends on
/// the abstract domain it is constructed with. It allows the use of different
/// abstract domains at runtime.
template < typename VariableRef, typename MemoryLocationRef >
class PolymorphicDomain final
    : public memory::AbstractDomain<
          VariableRef,
          MemoryLocationRef,
          PolymorphicDomain< VariableRef, MemoryLocationRef > > {
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
  using LiteralT = Literal< VariableRef, MemoryLocationRef >;

private:
  /// Type erasure idiom
  ///
  /// This is commonly known as the 'type erasure idiom', used to implement
  /// std::any.

  class PolymorphicBase {
  public:
    /// \brief Default constructor
    PolymorphicBase() = default;

    /// \brief No copy constructor
    PolymorphicBase(const PolymorphicBase&) = delete;

    /// \brief No move constructor
    PolymorphicBase(PolymorphicBase&&) = delete;

    /// \brief No copy assignment operator
    PolymorphicBase& operator=(const PolymorphicBase&) = delete;

    /// \brief No move assignment operator
    PolymorphicBase& operator=(PolymorphicBase&&) = delete;

    /// \brief Destructor
    virtual ~PolymorphicBase() = default;

    /// \brief Clone the abstract value
    virtual std::unique_ptr< PolymorphicBase > clone() const = 0;

    /// \name Core abstract domain methods
    /// @{

    /// \brief Normalize the abstract value
    virtual void normalize() = 0;

    /// \brief Check if the abstract value is bottom
    virtual bool is_bottom() const = 0;

    /// \brief Check if the abstract value is top
    virtual bool is_top() const = 0;

    /// \brief Set the abstract value to bottom
    virtual void set_to_bottom() = 0;

    /// \brief Set the abstract value to top
    virtual void set_to_top() = 0;

    /// \brief Partial order comparison
    virtual bool leq(const PolymorphicBase& other) const = 0;

    /// \brief Equality comparison
    virtual bool equals(const PolymorphicBase& other) const = 0;

    /// \brief Perform the union of two abstract values
    virtual void join_with(PolymorphicBase&& other) = 0;

    /// \brief Perform the union of two abstract values
    virtual void join_with(const PolymorphicBase& other) = 0;

    /// \brief Perform a union on a loop head
    virtual void join_loop_with(PolymorphicBase&& other) = 0;

    /// \brief Perform a union on a loop head
    virtual void join_loop_with(const PolymorphicBase& other) = 0;

    /// \brief Perform a union on two consecutive iterations of a fix-point
    /// algorithm
    virtual void join_iter_with(PolymorphicBase&& other) = 0;

    /// \brief Perform a union on two consecutive iterations of a fix-point
    /// algorithm
    virtual void join_iter_with(const PolymorphicBase& other) = 0;

    /// \brief Perform the widening of two abstract values
    virtual void widen_with(const PolymorphicBase& other) = 0;

    /// \brief Perform the widening of two abstract values with a threshold
    virtual void widen_threshold_with(const PolymorphicBase& other,
                                      const MachineInt& threshold) = 0;

    /// \brief Perform the intersection of two abstract values
    virtual void meet_with(const PolymorphicBase& other) = 0;

    /// \brief Perform the narrowing of two abstract values
    virtual void narrow_with(const PolymorphicBase& other) = 0;

    /// \brief Perform the narrowing of two abstract values with a threshold
    virtual void narrow_threshold_with(const PolymorphicBase& other,
                                       const MachineInt& threshold) = 0;

    /// \brief Perform the union of two abstract values
    virtual std::unique_ptr< PolymorphicBase > join(
        const PolymorphicBase& other) const = 0;

    /// \brief Perform a union on a loop head
    virtual std::unique_ptr< PolymorphicBase > join_loop(
        const PolymorphicBase& other) const = 0;

    /// \brief Perform a union on two consecutive iterations of a fix-point
    /// algorithm
    virtual std::unique_ptr< PolymorphicBase > join_iter(
        const PolymorphicBase& other) const = 0;

    /// \brief Perform the widening of two abstract values
    virtual std::unique_ptr< PolymorphicBase > widening(
        const PolymorphicBase& other) const = 0;

    /// \brief Perform the widening of two abstract values with a threshold
    virtual std::unique_ptr< PolymorphicBase > widening_threshold(
        const PolymorphicBase& other, const MachineInt& threshold) const = 0;

    /// \brief Perform the intersection of two abstract values
    virtual std::unique_ptr< PolymorphicBase > meet(
        const PolymorphicBase& other) const = 0;

    /// \brief Perform the narrowing of two abstract values
    virtual std::unique_ptr< PolymorphicBase > narrowing(
        const PolymorphicBase& other) const = 0;

    /// \brief Perform the narrowing of two abstract values with a threshold
    virtual std::unique_ptr< PolymorphicBase > narrowing_threshold(
        const PolymorphicBase& other, const MachineInt& threshold) const = 0;

    /// @}
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
    virtual void int_apply(IntUnaryOperator op,
                           VariableRef x,
                           VariableRef y) = 0;

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
    /// Return an overapproximation of the value of `x` as an
    /// interval-congruence
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
    virtual void pointer_assign(VariableRef p,
                                VariableRef q,
                                VariableRef o) = 0;

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

    /// \brief Refine the pointer `p` with the given set of addresses and
    /// offsets
    virtual void pointer_refine(VariableRef p,
                                const PointsToSetT& addrs,
                                const IntInterval& offset) = 0;

    /// \brief Refine the pointer `p` with the given pointer abstract value
    virtual void pointer_refine(VariableRef p,
                                const PointerAbsValueT& value) = 0;

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

    /// \brief Assign a dynamically typed variable to a dynamically typed
    /// variable
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
    /// \name Memory abstract domain methods
    /// @{

    /// \brief Perform the memory write `*p = v`
    ///
    /// \param p The pointer variable
    /// \param v The stored value
    /// \param size The stored size, in bytes (for instance, 4 for a int)
    virtual void mem_write(VariableRef p,
                           const LiteralT& v,
                           const MachineInt& size) = 0;

    /// \brief Perform the memory read `x = *p`
    ///
    /// \param x The result variable
    /// \param p The pointer variable
    /// \param size The read size, in bytes (for instance, 4 for a int)
    virtual void mem_read(const LiteralT& x,
                          VariableRef p,
                          const MachineInt& size) = 0;

    /// \brief Perform the memory copy `memcpy(dest, src, size)`
    ///
    /// \param dest The destination pointer variable
    /// \param src The source pointer variable
    /// \param size The number of bytes copied, as a literal
    ///
    /// Notes:
    ///   If `dst` and `src` overlap, as the behavior in C is undefined, the
    ///   memory contents is set to top.
    virtual void mem_copy(VariableRef dest,
                          VariableRef src,
                          const LiteralT& size) = 0;

    /// \brief Perform the memory set `memset(dest, value, size)`
    ///
    /// \param dest The destination pointer variable
    /// \param value The byte value, as a literal
    /// \param size The number of written bytes, as a literal
    virtual void mem_set(VariableRef dest,
                         const LiteralT& value,
                         const LiteralT& size) = 0;

    /// \brief Forget all memory contents
    virtual void mem_forget_all() = 0;

    /// \brief Forget the memory contents at the given memory location
    virtual void mem_forget(MemoryLocationRef addr) = 0;

    /// \brief Forget the memory contents in range
    /// `[addr + offset, addr + offset + size - 1]`
    ///
    /// \param addr The memory location
    /// \param offset The offset as a machine integer interval
    /// \param size The size in bytes
    virtual void mem_forget(MemoryLocationRef addr,
                            const IntInterval& offset,
                            const MachineInt& size) = 0;

    /// \brief Forget the memory contents in range
    /// `[addr + range.lb(), addr + range.ub()]`
    ///
    /// \param addr The memory location
    /// \param range The byte range as a machine integer interval
    virtual void mem_forget(MemoryLocationRef addr,
                            const IntInterval& range) = 0;

    /// \brief Forget the memory contents accessible through pointer `p`
    virtual void mem_forget_reachable(VariableRef p) = 0;

    /// \brief Forget the memory contents in range `[p, p + size - 1]`
    ///
    /// Forget all memory contents that can be accessible through pointer `p`
    /// and that overlap with `[p.offset, ..., p.offset + size - 1]`
    //
    /// \param p The pointer variable
    /// \param size The size in bytes
    virtual void mem_forget_reachable(VariableRef p,
                                      const MachineInt& size) = 0;

    /// \brief Abstract the memory contents reachable through pointer `p`
    ///
    /// Abstract all memory contents that can be accessible through pointer `p`.
    /// Suppose it contains random bytes, and no valid pointers (unlike
    /// forget_reachable_mem).
    virtual void mem_abstract_reachable(VariableRef p) = 0;

    /// \brief Abstract the memory contents in range `[p, p + size - 1]`
    ///
    /// Abstract all memory contents that can be accessible through pointer `p`
    /// and that overlap with `[p.offset, ..., p.offset + size - 1]`. Suppose it
    /// contains random bytes, and no valid pointers (unlike
    /// forget_reachable_mem).
    ///
    /// \param p The pointer variable
    /// \param size The size in bytes
    virtual void mem_abstract_reachable(VariableRef p,
                                        const MachineInt& size) = 0;

    /// \brief Set the memory contents accessible through pointer `p` to zero
    virtual void mem_zero_reachable(VariableRef p) = 0;

    /// \brief Set the memory contents accessible through pointer `p` to
    /// uninitialized
    virtual void mem_uninitialize_reachable(VariableRef p) = 0;

    /// @}
    /// \name Lifetime abstract domain methods
    /// @{

    /// \brief Assign `m = allocated`
    virtual void lifetime_assign_allocated(MemoryLocationRef m) = 0;

    /// \brief Assign `m = deallocated`
    virtual void lifetime_assign_deallocated(MemoryLocationRef m) = 0;

    /// \brief Add the constraint `m == allocated`
    virtual void lifetime_assert_allocated(MemoryLocationRef m) = 0;

    /// \brief Add the constraint `m == deallocated`
    virtual void lifetime_assert_deallocated(MemoryLocationRef m) = 0;

    /// \brief Forget the lifetime of a memory location
    virtual void lifetime_forget(MemoryLocationRef m) = 0;

    /// \brief Set the lifetime of a memory location
    virtual void lifetime_set(MemoryLocationRef m, Lifetime value) = 0;

    /// \brief Get the lifetime value for the given memory location
    virtual Lifetime lifetime_to_lifetime(MemoryLocationRef m) const = 0;

    /// @}
    /// \name Partitioning abstract domain methods
    /// @{

    /// \brief Partition the abstract value according to the given variable
    virtual void partitioning_set_variable(VariableRef x) = 0;

    /// \brief Return the current partitioning variable, or boost::none
    virtual boost::optional< VariableRef > partitioning_variable() const = 0;

    /// \brief Join the current partitions
    virtual void partitioning_join() = 0;

    /// \brief Disable the current partitioning
    virtual void partitioning_disable() = 0;

    /// @}

    /// \brief Dump the abstract value, for debugging purpose
    virtual void dump(std::ostream&) const = 0;

  }; // end class PolymorphicBase

private:
  template < typename RuntimeDomain >
  class PolymorphicDerived final : public PolymorphicBase {
  public:
    static_assert(memory::IsAbstractDomain< RuntimeDomain,
                                            VariableRef,
                                            MemoryLocationRef >::value,
                  "RuntimeDomain must implement memory::AbstractDomain");

  private:
    using PolymorphicDerivedT = PolymorphicDerived< RuntimeDomain >;

  private:
    RuntimeDomain _inv;

  public:
    /// \brief Create an abstract value
    explicit PolymorphicDerived(RuntimeDomain inv) : _inv(std::move(inv)) {}

    std::unique_ptr< PolymorphicBase > clone() const override {
      return std::make_unique< PolymorphicDerivedT >(this->_inv);
    }

    /// \name Core abstract domain methods
    /// @{

    void normalize() override { return this->_inv.normalize(); }

    bool is_bottom() const override { return this->_inv.is_bottom(); }

    bool is_top() const override { return this->_inv.is_top(); }

    void set_to_bottom() override { this->_inv.set_to_bottom(); }

    void set_to_top() override { this->_inv.set_to_top(); }

    /// \brief Check if the parameter has the same runtime domain
    bool is_compatible(const PolymorphicBase& other) const {
      return dynamic_cast< const PolymorphicDerivedT* >(&other) != nullptr;
    }

    /// \brief Assert that the parameter has the same runtime domain
    void assert_compatible(const PolymorphicBase& other) const {
      ikos_assert_msg(this->is_compatible(other),
                      "incompatible runtime abstract domains");
      ikos_ignore(other);
    }

    bool leq(const PolymorphicBase& other) const override {
      this->assert_compatible(other);
      return this->_inv.leq(
          static_cast< const PolymorphicDerivedT& >(other)._inv);
    }

    bool equals(const PolymorphicBase& other) const override {
      this->assert_compatible(other);
      return this->_inv.equals(
          static_cast< const PolymorphicDerivedT& >(other)._inv);
    }

    void join_with(PolymorphicBase&& other) override {
      this->assert_compatible(other);
      this->_inv.join_with(
          std::move(static_cast< PolymorphicDerivedT& >(other)._inv));
    }

    void join_with(const PolymorphicBase& other) override {
      this->assert_compatible(other);
      this->_inv.join_with(
          static_cast< const PolymorphicDerivedT& >(other)._inv);
    }

    void join_loop_with(PolymorphicBase&& other) override {
      this->assert_compatible(other);
      this->_inv.join_loop_with(
          std::move(static_cast< PolymorphicDerivedT& >(other)._inv));
    }

    void join_loop_with(const PolymorphicBase& other) override {
      this->assert_compatible(other);
      this->_inv.join_loop_with(
          static_cast< const PolymorphicDerivedT& >(other)._inv);
    }

    void join_iter_with(PolymorphicBase&& other) override {
      this->assert_compatible(other);
      this->_inv.join_iter_with(
          std::move(static_cast< PolymorphicDerivedT& >(other)._inv));
    }

    void join_iter_with(const PolymorphicBase& other) override {
      this->assert_compatible(other);
      this->_inv.join_iter_with(
          static_cast< const PolymorphicDerivedT& >(other)._inv);
    }

    void widen_with(const PolymorphicBase& other) override {
      this->assert_compatible(other);
      this->_inv.widen_with(
          static_cast< const PolymorphicDerivedT& >(other)._inv);
    }

    void widen_threshold_with(const PolymorphicBase& other,
                              const MachineInt& threshold) override {
      this->assert_compatible(other);
      this->_inv.widen_threshold_with(static_cast< const PolymorphicDerivedT& >(
                                          other)
                                          ._inv,
                                      threshold);
    }

    void meet_with(const PolymorphicBase& other) override {
      this->assert_compatible(other);
      this->_inv.meet_with(
          static_cast< const PolymorphicDerivedT& >(other)._inv);
    }

    void narrow_with(const PolymorphicBase& other) override {
      this->assert_compatible(other);
      this->_inv.narrow_with(
          static_cast< const PolymorphicDerivedT& >(other)._inv);
    }

    void narrow_threshold_with(const PolymorphicBase& other,
                               const MachineInt& threshold) override {
      this->assert_compatible(other);
      this->_inv
          .narrow_threshold_with(static_cast< const PolymorphicDerivedT& >(
                                     other)
                                     ._inv,
                                 threshold);
    }

    std::unique_ptr< PolymorphicBase > join(
        const PolymorphicBase& other) const override {
      this->assert_compatible(other);
      return std::make_unique< PolymorphicDerivedT >(this->_inv.join(
          static_cast< const PolymorphicDerivedT& >(other)._inv));
    }

    std::unique_ptr< PolymorphicBase > join_loop(
        const PolymorphicBase& other) const override {
      this->assert_compatible(other);
      return std::make_unique< PolymorphicDerivedT >(this->_inv.join_loop(
          static_cast< const PolymorphicDerivedT& >(other)._inv));
    }

    std::unique_ptr< PolymorphicBase > join_iter(
        const PolymorphicBase& other) const override {
      this->assert_compatible(other);
      return std::make_unique< PolymorphicDerivedT >(this->_inv.join_iter(
          static_cast< const PolymorphicDerivedT& >(other)._inv));
    }

    std::unique_ptr< PolymorphicBase > widening(
        const PolymorphicBase& other) const override {
      this->assert_compatible(other);
      return std::make_unique< PolymorphicDerivedT >(this->_inv.widening(
          static_cast< const PolymorphicDerivedT& >(other)._inv));
    }

    std::unique_ptr< PolymorphicBase > widening_threshold(
        const PolymorphicBase& other,
        const MachineInt& threshold) const override {
      this->assert_compatible(other);
      return std::make_unique< PolymorphicDerivedT >(
          this->_inv
              .widening_threshold(static_cast< const PolymorphicDerivedT& >(
                                      other)
                                      ._inv,
                                  threshold));
    }

    std::unique_ptr< PolymorphicBase > meet(
        const PolymorphicBase& other) const override {
      this->assert_compatible(other);
      return std::make_unique< PolymorphicDerivedT >(this->_inv.meet(
          static_cast< const PolymorphicDerivedT& >(other)._inv));
    }

    std::unique_ptr< PolymorphicBase > narrowing(
        const PolymorphicBase& other) const override {
      this->assert_compatible(other);
      return std::make_unique< PolymorphicDerivedT >(this->_inv.narrowing(
          static_cast< const PolymorphicDerivedT& >(other)._inv));
    }

    std::unique_ptr< PolymorphicBase > narrowing_threshold(
        const PolymorphicBase& other,
        const MachineInt& threshold) const override {
      this->assert_compatible(other);
      return std::make_unique< PolymorphicDerivedT >(
          this->_inv
              .narrowing_threshold(static_cast< const PolymorphicDerivedT& >(
                                       other)
                                       ._inv,
                                   threshold));
    }

    /// @}
    /// \name Uninitialized abstract domain methods
    /// @{

    void uninit_assert_initialized(VariableRef x) override {
      this->_inv.uninit_assert_initialized(x);
    }

    bool uninit_is_initialized(VariableRef x) const override {
      return this->_inv.uninit_is_initialized(x);
    }

    bool uninit_is_uninitialized(VariableRef x) const override {
      return this->_inv.uninit_is_uninitialized(x);
    }

    void uninit_refine(VariableRef x, Uninitialized value) override {
      this->_inv.uninit_refine(x, value);
    }

    Uninitialized uninit_to_uninitialized(VariableRef x) const override {
      return this->_inv.uninit_to_uninitialized(x);
    }

    /// @}
    /// \name Machine integer abstract domain methods
    /// @{

    void int_assign(VariableRef x, const MachineInt& n) override {
      this->_inv.int_assign(x, n);
    }

    void int_assign_undef(VariableRef x) override {
      this->_inv.int_assign_undef(x);
    }

    void int_assign_nondet(VariableRef x) override {
      this->_inv.int_assign_nondet(x);
    }

    void int_assign(VariableRef x, VariableRef y) override {
      this->_inv.int_assign(x, y);
    }

    void int_assign(VariableRef x, const IntLinearExpression& e) override {
      this->_inv.int_assign(x, e);
    }

    void int_apply(IntUnaryOperator op, VariableRef x, VariableRef y) override {
      this->_inv.int_apply(op, x, y);
    }

    void int_apply(IntBinaryOperator op,
                   VariableRef x,
                   VariableRef y,
                   VariableRef z) override {
      this->_inv.int_apply(op, x, y, z);
    }

    void int_apply(IntBinaryOperator op,
                   VariableRef x,
                   VariableRef y,
                   const MachineInt& z) override {
      this->_inv.int_apply(op, x, y, z);
    }

    void int_apply(IntBinaryOperator op,
                   VariableRef x,
                   const MachineInt& y,
                   VariableRef z) override {
      this->_inv.int_apply(op, x, y, z);
    }

    void int_add(IntPredicate pred, VariableRef x, VariableRef y) override {
      this->_inv.int_add(pred, x, y);
    }

    void int_add(IntPredicate pred,
                 VariableRef x,
                 const MachineInt& y) override {
      this->_inv.int_add(pred, x, y);
    }

    void int_add(IntPredicate pred,
                 const MachineInt& x,
                 VariableRef y) override {
      this->_inv.int_add(pred, x, y);
    }

    void int_set(VariableRef x, const IntInterval& value) override {
      this->_inv.int_set(x, value);
    }

    void int_set(VariableRef x, const IntCongruence& value) override {
      this->_inv.int_set(x, value);
    }

    void int_set(VariableRef x, const IntIntervalCongruence& value) override {
      this->_inv.int_set(x, value);
    }

    void int_refine(VariableRef x, const IntInterval& value) override {
      this->_inv.int_refine(x, value);
    }

    void int_refine(VariableRef x, const IntCongruence& value) override {
      this->_inv.int_refine(x, value);
    }

    void int_refine(VariableRef x,
                    const IntIntervalCongruence& value) override {
      this->_inv.int_refine(x, value);
    }

    void int_forget(VariableRef x) override { this->_inv.int_forget(x); }

    IntInterval int_to_interval(VariableRef x) const override {
      return this->_inv.int_to_interval(x);
    }

    IntInterval int_to_interval(const IntLinearExpression& e) const override {
      return this->_inv.int_to_interval(e);
    }

    IntCongruence int_to_congruence(VariableRef x) const override {
      return this->_inv.int_to_congruence(x);
    }

    IntCongruence int_to_congruence(
        const IntLinearExpression& e) const override {
      return this->_inv.int_to_congruence(e);
    }

    IntIntervalCongruence int_to_interval_congruence(
        VariableRef x) const override {
      return this->_inv.int_to_interval_congruence(x);
    }

    IntIntervalCongruence int_to_interval_congruence(
        const IntLinearExpression& e) const override {
      return this->_inv.int_to_interval_congruence(e);
    }

    /// @}
    /// \name Non-negative loop counter abstract domain methods
    /// @{

    void counter_mark(VariableRef x) override { this->_inv.counter_mark(x); }

    void counter_unmark(VariableRef x) override {
      this->_inv.counter_unmark(x);
    }

    void counter_init(VariableRef x, const MachineInt& c) override {
      this->_inv.counter_init(x, c);
    }

    void counter_incr(VariableRef x, const MachineInt& k) override {
      this->_inv.counter_incr(x, k);
    }

    void counter_forget(VariableRef x) override {
      this->_inv.counter_forget(x);
    }

    /// @}
    /// \name Floating point abstract domain methods
    /// @{

    void float_assign_undef(VariableRef x) override {
      this->_inv.float_assign_undef(x);
    }

    void float_assign_nondet(VariableRef x) override {
      this->_inv.float_assign_nondet(x);
    }

    void float_assign(VariableRef x, VariableRef y) override {
      this->_inv.float_assign(x, y);
    }

    void float_forget(VariableRef x) override { this->_inv.float_forget(x); }

    /// @}
    /// \name Nullity abstract domain methods
    /// @{

    void nullity_assert_null(VariableRef p) override {
      this->_inv.nullity_assert_null(p);
    }

    void nullity_assert_non_null(VariableRef p) override {
      this->_inv.nullity_assert_non_null(p);
    }

    bool nullity_is_null(VariableRef p) const override {
      return this->_inv.nullity_is_null(p);
    }

    bool nullity_is_non_null(VariableRef p) const override {
      return this->_inv.nullity_is_non_null(p);
    }

    void nullity_set(VariableRef p, Nullity value) override {
      this->_inv.nullity_set(p, value);
    }

    void nullity_refine(VariableRef p, Nullity value) override {
      this->_inv.nullity_refine(p, value);
    }

    Nullity nullity_to_nullity(VariableRef p) const override {
      return this->_inv.nullity_to_nullity(p);
    }

    /// @}
    /// \name Pointer abstract domain methods
    /// @{

    void pointer_assign(VariableRef p,
                        MemoryLocationRef addr,
                        Nullity nullity) override {
      this->_inv.pointer_assign(p, addr, nullity);
    }

    void pointer_assign_null(VariableRef p) override {
      this->_inv.pointer_assign_null(p);
    }

    void pointer_assign_undef(VariableRef p) override {
      this->_inv.pointer_assign_undef(p);
    }

    void pointer_assign_nondet(VariableRef p) override {
      this->_inv.pointer_assign_nondet(p);
    }

    void pointer_assign(VariableRef p, VariableRef q) override {
      this->_inv.pointer_assign(p, q);
    }

    void pointer_assign(VariableRef p, VariableRef q, VariableRef o) override {
      this->_inv.pointer_assign(p, q, o);
    }

    void pointer_assign(VariableRef p,
                        VariableRef q,
                        const MachineInt& o) override {
      this->_inv.pointer_assign(p, q, o);
    }

    void pointer_assign(VariableRef p,
                        VariableRef q,
                        const IntLinearExpression& o) override {
      this->_inv.pointer_assign(p, q, o);
    }

    void pointer_add(PointerPredicate pred,
                     VariableRef p,
                     VariableRef q) override {
      this->_inv.pointer_add(pred, p, q);
    }

    void pointer_refine(VariableRef p, const PointsToSetT& addrs) override {
      this->_inv.pointer_refine(p, addrs);
    }

    void pointer_refine(VariableRef p,
                        const PointsToSetT& addrs,
                        const IntInterval& offset) override {
      this->_inv.pointer_refine(p, addrs, offset);
    }

    void pointer_refine(VariableRef p, const PointerAbsValueT& value) override {
      this->_inv.pointer_refine(p, value);
    }

    void pointer_refine(VariableRef p, const PointerSetT& set) override {
      this->_inv.pointer_refine(p, set);
    }

    void pointer_offset_to_int(VariableRef x, VariableRef p) override {
      this->_inv.pointer_offset_to_int(x, p);
    }

    IntInterval pointer_offset_to_interval(VariableRef p) const override {
      return this->_inv.pointer_offset_to_interval(p);
    }

    IntCongruence pointer_offset_to_congruence(VariableRef p) const override {
      return this->_inv.pointer_offset_to_congruence(p);
    }

    IntIntervalCongruence pointer_offset_to_interval_congruence(
        VariableRef p) const override {
      return this->_inv.pointer_offset_to_interval_congruence(p);
    }

    PointsToSetT pointer_to_points_to(VariableRef p) const override {
      return this->_inv.pointer_to_points_to(p);
    }

    PointerAbsValueT pointer_to_pointer(VariableRef p) const override {
      return this->_inv.pointer_to_pointer(p);
    }

    void pointer_forget_offset(VariableRef p) override {
      this->_inv.pointer_forget_offset(p);
    }

    void pointer_forget(VariableRef p) override {
      this->_inv.pointer_forget(p);
    }

    /// @}
    /// \name Dynamically typed variables abstract domain methods
    /// @{

    void dynamic_assign(VariableRef x, VariableRef y) override {
      this->_inv.dynamic_assign(x, y);
    }

    void dynamic_write_undef(VariableRef x) override {
      this->_inv.dynamic_write_undef(x);
    }

    void dynamic_write_nondet(VariableRef x) override {
      this->_inv.dynamic_write_nondet(x);
    }

    void dynamic_write_int(VariableRef x, const MachineInt& n) override {
      this->_inv.dynamic_write_int(x, n);
    }

    void dynamic_write_nondet_int(VariableRef x) override {
      this->_inv.dynamic_write_nondet_int(x);
    }

    void dynamic_write_int(VariableRef x, VariableRef y) override {
      this->_inv.dynamic_write_int(x, y);
    }

    void dynamic_write_nondet_float(VariableRef x) override {
      this->_inv.dynamic_write_nondet_float(x);
    }

    void dynamic_write_null(VariableRef x) override {
      this->_inv.dynamic_write_null(x);
    }

    void dynamic_write_pointer(VariableRef x,
                               MemoryLocationRef addr,
                               Nullity nullity) override {
      this->_inv.dynamic_write_pointer(x, addr, nullity);
    }

    void dynamic_write_pointer(VariableRef x, VariableRef y) override {
      this->_inv.dynamic_write_pointer(x, y);
    }

    void dynamic_read_int(VariableRef x, VariableRef y) override {
      this->_inv.dynamic_read_int(x, y);
    }

    void dynamic_read_pointer(VariableRef x, VariableRef y) override {
      this->_inv.dynamic_read_pointer(x, y);
    }

    bool dynamic_is_zero(VariableRef x) const override {
      return this->_inv.dynamic_is_zero(x);
    }

    bool dynamic_is_null(VariableRef x) const override {
      return this->_inv.dynamic_is_null(x);
    }

    void dynamic_forget(VariableRef x) override {
      this->_inv.dynamic_forget(x);
    }

    /// @}
    /// \name Scalar abstract domain methods
    /// @{

    void scalar_assign_undef(VariableRef x) override {
      this->_inv.scalar_assign_undef(x);
    }

    void scalar_assign_nondet(VariableRef x) override {
      this->_inv.scalar_assign_nondet(x);
    }

    void scalar_pointer_to_int(VariableRef x,
                               VariableRef p,
                               MemoryLocationRef absolute_zero) override {
      this->_inv.scalar_pointer_to_int(x, p, absolute_zero);
    }

    void scalar_int_to_pointer(VariableRef p,
                               VariableRef x,
                               MemoryLocationRef absolute_zero) override {
      this->_inv.scalar_int_to_pointer(p, x, absolute_zero);
    }

    void scalar_forget(VariableRef x) override { this->_inv.scalar_forget(x); }

    /// @}
    /// \name Memory abstract domain methods
    /// @{

    void mem_write(VariableRef p,
                   const LiteralT& v,
                   const MachineInt& size) override {
      this->_inv.mem_write(p, v, size);
    }

    void mem_read(const LiteralT& x,
                  VariableRef p,
                  const MachineInt& size) override {
      this->_inv.mem_read(x, p, size);
    }

    void mem_copy(VariableRef dest,
                  VariableRef src,
                  const LiteralT& size) override {
      this->_inv.mem_copy(dest, src, size);
    }

    void mem_set(VariableRef dest,
                 const LiteralT& value,
                 const LiteralT& size) override {
      this->_inv.mem_set(dest, value, size);
    }

    void mem_forget_all() override { this->_inv.mem_forget_all(); }

    void mem_forget(MemoryLocationRef addr) override {
      this->_inv.mem_forget(addr);
    }

    void mem_forget(MemoryLocationRef addr,
                    const IntInterval& offset,
                    const MachineInt& size) override {
      this->_inv.mem_forget(addr, offset, size);
    }

    void mem_forget(MemoryLocationRef addr, const IntInterval& range) override {
      this->_inv.mem_forget(addr, range);
    }

    void mem_forget_reachable(VariableRef p) override {
      this->_inv.mem_forget_reachable(p);
    }

    void mem_forget_reachable(VariableRef p, const MachineInt& size) override {
      this->_inv.mem_forget_reachable(p, size);
    }

    void mem_abstract_reachable(VariableRef p) override {
      this->_inv.mem_abstract_reachable(p);
    }

    void mem_abstract_reachable(VariableRef p,
                                const MachineInt& size) override {
      this->_inv.mem_abstract_reachable(p, size);
    }

    void mem_zero_reachable(VariableRef p) override {
      this->_inv.mem_zero_reachable(p);
    }

    void mem_uninitialize_reachable(VariableRef p) override {
      this->_inv.mem_uninitialize_reachable(p);
    }

    /// @}
    /// \name Lifetime abstract domain methods
    /// @{

    void lifetime_assign_allocated(MemoryLocationRef m) override {
      this->_inv.lifetime_assign_allocated(m);
    }

    void lifetime_assign_deallocated(MemoryLocationRef m) override {
      this->_inv.lifetime_assign_deallocated(m);
    }

    void lifetime_assert_allocated(MemoryLocationRef m) override {
      this->_inv.lifetime_assert_allocated(m);
    }

    void lifetime_assert_deallocated(MemoryLocationRef m) override {
      this->_inv.lifetime_assert_deallocated(m);
    }

    void lifetime_forget(MemoryLocationRef m) override {
      this->_inv.lifetime_forget(m);
    }

    void lifetime_set(MemoryLocationRef m, Lifetime value) override {
      this->_inv.lifetime_set(m, value);
    }

    Lifetime lifetime_to_lifetime(MemoryLocationRef m) const override {
      return this->_inv.lifetime_to_lifetime(m);
    }

    /// @}
    /// \name Partitioning abstract domain methods
    /// @{

    void partitioning_set_variable(VariableRef x) override {
      this->_inv.partitioning_set_variable(x);
    }

    boost::optional< VariableRef > partitioning_variable() const override {
      return this->_inv.partitioning_variable();
    }

    void partitioning_join() override { this->_inv.partitioning_join(); }

    void partitioning_disable() override { this->_inv.partitioning_disable(); }

    /// @}

    void dump(std::ostream& o) const override { this->_inv.dump(o); }

  }; // end class PolymorphicDerived

private:
  /// \brief Pointer on the polymorphic base class
  std::unique_ptr< PolymorphicBase > _ptr;

private:
  /// \brief Constructor
  explicit PolymorphicDomain(std::unique_ptr< PolymorphicBase > ptr)
      : _ptr(std::move(ptr)) {}

public:
  /// \brief Create a polymorphic domain with the given abstract value
  template < typename RuntimeDomain >
  explicit PolymorphicDomain(RuntimeDomain inv)
      : _ptr(std::make_unique<
             PolymorphicDerived< remove_cvref_t< RuntimeDomain > > >(
            std::move(inv))) {}

  /// \brief Copy constructor
  PolymorphicDomain(const PolymorphicDomain& other)
      : _ptr(other._ptr->clone()) {}

  /// \brief Move constructor
  PolymorphicDomain(PolymorphicDomain&&) noexcept = default;

  /// \brief Copy assignment operator
  PolymorphicDomain& operator=(const PolymorphicDomain& other) {
    this->_ptr = other._ptr->clone();
    return *this;
  }

  /// \brief Move assignment operator
  PolymorphicDomain& operator=(PolymorphicDomain&&) noexcept = default;

  /// \brief Destructor
  ~PolymorphicDomain() override = default;

  /// \name Core abstract domain methods
  /// @{

  void normalize() override { this->_ptr->normalize(); }

  bool is_bottom() const override { return this->_ptr->is_bottom(); }

  bool is_top() const override { return this->_ptr->is_top(); }

  void set_to_bottom() override { this->_ptr->set_to_bottom(); }

  void set_to_top() override { this->_ptr->set_to_top(); }

  bool leq(const PolymorphicDomain& other) const override {
    return this->_ptr->leq(*other._ptr);
  }

  bool equals(const PolymorphicDomain& other) const override {
    return this->_ptr->equals(*other._ptr);
  }

  void join_with(PolymorphicDomain&& other) override {
    this->_ptr->join_with(std::move(*other._ptr));
  }

  void join_with(const PolymorphicDomain& other) override {
    this->_ptr->join_with(*other._ptr);
  }

  void join_loop_with(PolymorphicDomain&& other) override {
    this->_ptr->join_loop_with(std::move(*other._ptr));
  }

  void join_loop_with(const PolymorphicDomain& other) override {
    this->_ptr->join_loop_with(*other._ptr);
  }

  void join_iter_with(PolymorphicDomain&& other) override {
    this->_ptr->join_iter_with(std::move(*other._ptr));
  }

  void join_iter_with(const PolymorphicDomain& other) override {
    this->_ptr->join_iter_with(*other._ptr);
  }

  void widen_with(const PolymorphicDomain& other) override {
    this->_ptr->widen_with(*other._ptr);
  }

  void widen_threshold_with(const PolymorphicDomain& other,
                            const MachineInt& threshold) override {
    this->_ptr->widen_threshold_with(*other._ptr, threshold);
  }

  void meet_with(const PolymorphicDomain& other) override {
    this->_ptr->meet_with(*other._ptr);
  }

  void narrow_with(const PolymorphicDomain& other) override {
    this->_ptr->narrow_with(*other._ptr);
  }

  void narrow_threshold_with(const PolymorphicDomain& other,
                             const MachineInt& threshold) override {
    this->_ptr->narrow_threshold_with(*other._ptr, threshold);
  }

  PolymorphicDomain join(const PolymorphicDomain& other) const override {
    return PolymorphicDomain(this->_ptr->join(*other._ptr));
  }

  PolymorphicDomain join_loop(const PolymorphicDomain& other) const override {
    return PolymorphicDomain(this->_ptr->join_loop(*other._ptr));
  }

  PolymorphicDomain join_iter(const PolymorphicDomain& other) const override {
    return PolymorphicDomain(this->_ptr->join_iter(*other._ptr));
  }

  PolymorphicDomain widening(const PolymorphicDomain& other) const override {
    return PolymorphicDomain(this->_ptr->widening(*other._ptr));
  }

  PolymorphicDomain widening_threshold(
      const PolymorphicDomain& other,
      const MachineInt& threshold) const override {
    return PolymorphicDomain(
        this->_ptr->widening_threshold(*other._ptr, threshold));
  }

  PolymorphicDomain meet(const PolymorphicDomain& other) const override {
    return PolymorphicDomain(this->_ptr->meet(*other._ptr));
  }

  PolymorphicDomain narrowing(const PolymorphicDomain& other) const override {
    return PolymorphicDomain(this->_ptr->narrowing(*other._ptr));
  }

  PolymorphicDomain narrowing_threshold(
      const PolymorphicDomain& other,
      const MachineInt& threshold) const override {
    return PolymorphicDomain(
        this->_ptr->narrowing_threshold(*other._ptr, threshold));
  }

  /// @}
  /// \name Uninitialized abstract domain methods
  /// @{

  void uninit_assert_initialized(VariableRef x) override {
    this->_ptr->uninit_assert_initialized(x);
  }

  bool uninit_is_initialized(VariableRef x) const override {
    return this->_ptr->uninit_is_initialized(x);
  }

  bool uninit_is_uninitialized(VariableRef x) const override {
    return this->_ptr->uninit_is_uninitialized(x);
  }

  void uninit_refine(VariableRef x, Uninitialized value) override {
    this->_ptr->uninit_refine(x, value);
  }

  Uninitialized uninit_to_uninitialized(VariableRef x) const override {
    return this->_ptr->uninit_to_uninitialized(x);
  }

  /// @}
  /// \name Machine integer abstract domain methods
  /// @{

  void int_assign(VariableRef x, const MachineInt& n) override {
    this->_ptr->int_assign(x, n);
  }

  void int_assign_undef(VariableRef x) override {
    this->_ptr->int_assign_undef(x);
  }

  void int_assign_nondet(VariableRef x) override {
    this->_ptr->int_assign_nondet(x);
  }

  void int_assign(VariableRef x, VariableRef y) override {
    this->_ptr->int_assign(x, y);
  }

  void int_assign(VariableRef x, const IntLinearExpression& e) override {
    this->_ptr->int_assign(x, e);
  }

  void int_apply(IntUnaryOperator op, VariableRef x, VariableRef y) override {
    this->_ptr->int_apply(op, x, y);
  }

  void int_apply(IntBinaryOperator op,
                 VariableRef x,
                 VariableRef y,
                 VariableRef z) override {
    this->_ptr->int_apply(op, x, y, z);
  }

  void int_apply(IntBinaryOperator op,
                 VariableRef x,
                 VariableRef y,
                 const MachineInt& z) override {
    this->_ptr->int_apply(op, x, y, z);
  }

  void int_apply(IntBinaryOperator op,
                 VariableRef x,
                 const MachineInt& y,
                 VariableRef z) override {
    this->_ptr->int_apply(op, x, y, z);
  }

  void int_add(IntPredicate pred, VariableRef x, VariableRef y) override {
    this->_ptr->int_add(pred, x, y);
  }

  void int_add(IntPredicate pred, VariableRef x, const MachineInt& y) override {
    this->_ptr->int_add(pred, x, y);
  }

  void int_add(IntPredicate pred, const MachineInt& x, VariableRef y) override {
    this->_ptr->int_add(pred, x, y);
  }

  void int_set(VariableRef x, const IntInterval& value) override {
    this->_ptr->int_set(x, value);
  }

  void int_set(VariableRef x, const IntCongruence& value) override {
    this->_ptr->int_set(x, value);
  }

  void int_set(VariableRef x, const IntIntervalCongruence& value) override {
    this->_ptr->int_set(x, value);
  }

  void int_refine(VariableRef x, const IntInterval& value) override {
    this->_ptr->int_refine(x, value);
  }

  void int_refine(VariableRef x, const IntCongruence& value) override {
    this->_ptr->int_refine(x, value);
  }

  void int_refine(VariableRef x, const IntIntervalCongruence& value) override {
    this->_ptr->int_refine(x, value);
  }

  void int_forget(VariableRef x) override { this->_ptr->int_forget(x); }

  IntInterval int_to_interval(VariableRef x) const override {
    return this->_ptr->int_to_interval(x);
  }

  IntInterval int_to_interval(const IntLinearExpression& e) const override {
    return this->_ptr->int_to_interval(e);
  }

  IntCongruence int_to_congruence(VariableRef x) const override {
    return this->_ptr->int_to_congruence(x);
  }

  IntCongruence int_to_congruence(const IntLinearExpression& e) const override {
    return this->_ptr->int_to_congruence(e);
  }

  IntIntervalCongruence int_to_interval_congruence(
      VariableRef x) const override {
    return this->_ptr->int_to_interval_congruence(x);
  }

  IntIntervalCongruence int_to_interval_congruence(
      const IntLinearExpression& e) const override {
    return this->_ptr->int_to_interval_congruence(e);
  }

  /// @}
  /// \name Non-negative loop counter abstract domain methods
  /// @{

  void counter_mark(VariableRef x) override { this->_ptr->counter_mark(x); }

  void counter_unmark(VariableRef x) override { this->_ptr->counter_unmark(x); }

  void counter_init(VariableRef x, const MachineInt& c) override {
    this->_ptr->counter_init(x, c);
  }

  void counter_incr(VariableRef x, const MachineInt& k) override {
    this->_ptr->counter_incr(x, k);
  }

  void counter_forget(VariableRef x) override { this->_ptr->counter_forget(x); }

  /// @}
  /// \name Floating point abstract domain methods
  /// @{

  void float_assign_undef(VariableRef x) override {
    this->_ptr->float_assign_undef(x);
  }

  void float_assign_nondet(VariableRef x) override {
    this->_ptr->float_assign_nondet(x);
  }

  void float_assign(VariableRef x, VariableRef y) override {
    this->_ptr->float_assign(x, y);
  }

  void float_forget(VariableRef x) override { this->_ptr->float_forget(x); }

  /// @}
  /// \name Nullity abstract domain methods
  /// @{

  void nullity_assert_null(VariableRef p) override {
    this->_ptr->nullity_assert_null(p);
  }

  void nullity_assert_non_null(VariableRef p) override {
    this->_ptr->nullity_assert_non_null(p);
  }

  bool nullity_is_null(VariableRef p) const override {
    return this->_ptr->nullity_is_null(p);
  }

  bool nullity_is_non_null(VariableRef p) const override {
    return this->_ptr->nullity_is_non_null(p);
  }

  void nullity_set(VariableRef p, Nullity value) override {
    this->_ptr->nullity_set(p, value);
  }

  void nullity_refine(VariableRef p, Nullity value) override {
    this->_ptr->nullity_refine(p, value);
  }

  Nullity nullity_to_nullity(VariableRef p) const override {
    return this->_ptr->nullity_to_nullity(p);
  }

  /// @}
  /// \name Pointer abstract domain methods
  /// @{

  void pointer_assign(VariableRef p,
                      MemoryLocationRef addr,
                      Nullity nullity) override {
    this->_ptr->pointer_assign(p, addr, nullity);
  }

  void pointer_assign_null(VariableRef p) override {
    this->_ptr->pointer_assign_null(p);
  }

  void pointer_assign_undef(VariableRef p) override {
    this->_ptr->pointer_assign_undef(p);
  }

  void pointer_assign_nondet(VariableRef p) override {
    this->_ptr->pointer_assign_nondet(p);
  }

  void pointer_assign(VariableRef p, VariableRef q) override {
    this->_ptr->pointer_assign(p, q);
  }

  void pointer_assign(VariableRef p, VariableRef q, VariableRef o) override {
    this->_ptr->pointer_assign(p, q, o);
  }

  void pointer_assign(VariableRef p,
                      VariableRef q,
                      const MachineInt& o) override {
    this->_ptr->pointer_assign(p, q, o);
  }

  void pointer_assign(VariableRef p,
                      VariableRef q,
                      const IntLinearExpression& o) override {
    this->_ptr->pointer_assign(p, q, o);
  }

  void pointer_add(PointerPredicate pred,
                   VariableRef p,
                   VariableRef q) override {
    this->_ptr->pointer_add(pred, p, q);
  }

  void pointer_refine(VariableRef p, const PointsToSetT& addrs) override {
    this->_ptr->pointer_refine(p, addrs);
  }

  void pointer_refine(VariableRef p,
                      const PointsToSetT& addrs,
                      const IntInterval& offset) override {
    this->_ptr->pointer_refine(p, addrs, offset);
  }

  void pointer_refine(VariableRef p, const PointerAbsValueT& value) override {
    this->_ptr->pointer_refine(p, value);
  }

  void pointer_refine(VariableRef p, const PointerSetT& set) override {
    this->_ptr->pointer_refine(p, set);
  }

  void pointer_offset_to_int(VariableRef x, VariableRef p) override {
    this->_ptr->pointer_offset_to_int(x, p);
  }

  IntInterval pointer_offset_to_interval(VariableRef p) const override {
    return this->_ptr->pointer_offset_to_interval(p);
  }

  IntCongruence pointer_offset_to_congruence(VariableRef p) const override {
    return this->_ptr->pointer_offset_to_congruence(p);
  }

  IntIntervalCongruence pointer_offset_to_interval_congruence(
      VariableRef p) const override {
    return this->_ptr->pointer_offset_to_interval_congruence(p);
  }

  PointsToSetT pointer_to_points_to(VariableRef p) const override {
    return this->_ptr->pointer_to_points_to(p);
  }

  PointerAbsValueT pointer_to_pointer(VariableRef p) const override {
    return this->_ptr->pointer_to_pointer(p);
  }

  void pointer_forget_offset(VariableRef p) override {
    this->_ptr->pointer_forget_offset(p);
  }

  void pointer_forget(VariableRef p) override { this->_ptr->pointer_forget(p); }

  /// @}
  /// \name Dynamically typed variables abstract domain methods
  /// @{

  void dynamic_assign(VariableRef x, VariableRef y) override {
    this->_ptr->dynamic_assign(x, y);
  }

  void dynamic_write_undef(VariableRef x) override {
    this->_ptr->dynamic_write_undef(x);
  }

  void dynamic_write_nondet(VariableRef x) override {
    this->_ptr->dynamic_write_nondet(x);
  }

  void dynamic_write_int(VariableRef x, const MachineInt& n) override {
    this->_ptr->dynamic_write_int(x, n);
  }

  void dynamic_write_nondet_int(VariableRef x) override {
    this->_ptr->dynamic_write_nondet_int(x);
  }

  void dynamic_write_int(VariableRef x, VariableRef y) override {
    this->_ptr->dynamic_write_int(x, y);
  }

  void dynamic_write_nondet_float(VariableRef x) override {
    this->_ptr->dynamic_write_nondet_float(x);
  }

  void dynamic_write_null(VariableRef x) override {
    this->_ptr->dynamic_write_null(x);
  }

  void dynamic_write_pointer(VariableRef x,
                             MemoryLocationRef addr,
                             Nullity nullity) override {
    this->_ptr->dynamic_write_pointer(x, addr, nullity);
  }

  void dynamic_write_pointer(VariableRef x, VariableRef y) override {
    this->_ptr->dynamic_write_pointer(x, y);
  }

  void dynamic_read_int(VariableRef x, VariableRef y) override {
    this->_ptr->dynamic_read_int(x, y);
  }

  void dynamic_read_pointer(VariableRef x, VariableRef y) override {
    this->_ptr->dynamic_read_pointer(x, y);
  }

  bool dynamic_is_zero(VariableRef x) const override {
    return this->_ptr->dynamic_is_zero(x);
  }

  bool dynamic_is_null(VariableRef x) const override {
    return this->_ptr->dynamic_is_null(x);
  }

  void dynamic_forget(VariableRef x) override { this->_ptr->dynamic_forget(x); }

  /// @}
  /// \name Scalar abstract domain methods
  /// @{

  void scalar_assign_undef(VariableRef x) override {
    this->_ptr->scalar_assign_undef(x);
  }

  void scalar_assign_nondet(VariableRef x) override {
    this->_ptr->scalar_assign_nondet(x);
  }

  void scalar_pointer_to_int(VariableRef x,
                             VariableRef p,
                             MemoryLocationRef absolute_zero) override {
    this->_ptr->scalar_pointer_to_int(x, p, absolute_zero);
  }

  void scalar_int_to_pointer(VariableRef p,
                             VariableRef x,
                             MemoryLocationRef absolute_zero) override {
    this->_ptr->scalar_int_to_pointer(p, x, absolute_zero);
  }

  void scalar_forget(VariableRef x) override { this->_ptr->scalar_forget(x); }

  /// @}
  /// \name Memory abstract domain methods
  /// @{

  void mem_write(VariableRef p,
                 const LiteralT& v,
                 const MachineInt& size) override {
    this->_ptr->mem_write(p, v, size);
  }

  void mem_read(const LiteralT& x,
                VariableRef p,
                const MachineInt& size) override {
    this->_ptr->mem_read(x, p, size);
  }

  void mem_copy(VariableRef dest,
                VariableRef src,
                const LiteralT& size) override {
    this->_ptr->mem_copy(dest, src, size);
  }

  void mem_set(VariableRef dest,
               const LiteralT& value,
               const LiteralT& size) override {
    this->_ptr->mem_set(dest, value, size);
  }

  void mem_forget_all() override { this->_ptr->mem_forget_all(); }

  void mem_forget(MemoryLocationRef addr) override {
    this->_ptr->mem_forget(addr);
  }

  void mem_forget(MemoryLocationRef addr,
                  const IntInterval& offset,
                  const MachineInt& size) override {
    this->_ptr->mem_forget(addr, offset, size);
  }

  void mem_forget(MemoryLocationRef addr, const IntInterval& range) override {
    this->_ptr->mem_forget(addr, range);
  }

  void mem_forget_reachable(VariableRef p) override {
    this->_ptr->mem_forget_reachable(p);
  }

  void mem_forget_reachable(VariableRef p, const MachineInt& size) override {
    this->_ptr->mem_forget_reachable(p, size);
  }

  void mem_abstract_reachable(VariableRef p) override {
    this->_ptr->mem_abstract_reachable(p);
  }

  void mem_abstract_reachable(VariableRef p, const MachineInt& size) override {
    this->_ptr->mem_abstract_reachable(p, size);
  }

  void mem_zero_reachable(VariableRef p) override {
    this->_ptr->mem_zero_reachable(p);
  }

  void mem_uninitialize_reachable(VariableRef p) override {
    this->_ptr->mem_uninitialize_reachable(p);
  }

  /// @}
  /// \name Lifetime abstract domain methods
  /// @{

  void lifetime_assign_allocated(MemoryLocationRef m) override {
    this->_ptr->lifetime_assign_allocated(m);
  }

  void lifetime_assign_deallocated(MemoryLocationRef m) override {
    this->_ptr->lifetime_assign_deallocated(m);
  }

  void lifetime_assert_allocated(MemoryLocationRef m) override {
    this->_ptr->lifetime_assert_allocated(m);
  }

  void lifetime_assert_deallocated(MemoryLocationRef m) override {
    this->_ptr->lifetime_assert_deallocated(m);
  }

  void lifetime_forget(MemoryLocationRef m) override {
    this->_ptr->lifetime_forget(m);
  }

  void lifetime_set(MemoryLocationRef m, Lifetime value) override {
    this->_ptr->lifetime_set(m, value);
  }

  Lifetime lifetime_to_lifetime(MemoryLocationRef m) const override {
    return this->_ptr->lifetime_to_lifetime(m);
  }

  /// @}
  /// \name Partitioning abstract domain methods
  /// @{

  void partitioning_set_variable(VariableRef x) override {
    this->_ptr->partitioning_set_variable(x);
  }

  boost::optional< VariableRef > partitioning_variable() const override {
    return this->_ptr->partitioning_variable();
  }

  void partitioning_join() override { this->_ptr->partitioning_join(); }

  void partitioning_disable() override { this->_ptr->partitioning_disable(); }

  /// @}

  void dump(std::ostream& o) const override { this->_ptr->dump(o); }

  static std::string name() { return "polymorphic domain"; }

}; // end class PolymorphicDomain

} // end namespace memory
} // end namespace core
} // end namespace ikos
