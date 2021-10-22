/*******************************************************************************
 *
 * \file
 * \brief A dummy memory abstract domain
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

#include <type_traits>

#include <ikos/core/domain/memory/abstract_domain.hpp>

namespace ikos {
namespace core {
namespace memory {

/// \brief Dummy memory abstract domain
///
/// This class implements a memory abstract domain on top of a scalar abstract
/// domain, ignoring all memory operations safely.
template < typename VariableRef,
           typename MemoryLocationRef,
           typename ScalarDomain >
class DummyDomain final
    : public memory::AbstractDomain<
          VariableRef,
          MemoryLocationRef,
          DummyDomain< VariableRef, MemoryLocationRef, ScalarDomain > > {
public:
  static_assert(scalar::IsAbstractDomain< ScalarDomain,
                                          VariableRef,
                                          MemoryLocationRef >::value,
                "ScalarDomain must implement scalar::AbstractDomain");

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
  using ScalarVariableTrait = scalar::VariableTraits< VariableRef >;

private:
  ScalarDomain _scalar;

public:
  /// \brief Create an abstract value with the given underlying abstract values
  DummyDomain(ScalarDomain scalar) : _scalar(std::move(scalar)) {}

  /// \brief Copy constructor
  DummyDomain(const DummyDomain&) noexcept(
      std::is_nothrow_copy_constructible< ScalarDomain >::value) = default;

  /// \brief Move constructor
  DummyDomain(DummyDomain&&) noexcept(
      std::is_nothrow_move_constructible< ScalarDomain >::value) = default;

  /// \brief Copy assignment operator
  DummyDomain& operator=(const DummyDomain&) noexcept(
      std::is_nothrow_copy_assignable< ScalarDomain >::value) = default;

  /// \brief Move assignment operator
  DummyDomain& operator=(DummyDomain&&) noexcept(
      std::is_nothrow_move_assignable< ScalarDomain >::value) = default;

  /// \brief Destructor
  ~DummyDomain() override = default;

  /// \name Implement core abstract domain methods
  /// @{

  void normalize() override { this->_scalar.normalize(); }

  bool is_bottom() const override { return this->_scalar.is_bottom(); }

  bool is_top() const override { return this->_scalar.is_top(); }

  void set_to_bottom() override { this->_scalar.set_to_bottom(); }

  void set_to_top() override { this->_scalar.set_to_top(); }

  bool leq(const DummyDomain& other) const override {
    return this->_scalar.leq(other._scalar);
  }

  bool equals(const DummyDomain& other) const override {
    return this->_scalar.equals(other._scalar);
  }

  void join_with(DummyDomain&& other) override {
    this->_scalar.join_with(std::move(other._scalar));
  }

  void join_with(const DummyDomain& other) override {
    this->_scalar.join_with(other._scalar);
  }

  void join_loop_with(DummyDomain&& other) override {
    this->_scalar.join_loop_with(std::move(other._scalar));
  }

  void join_loop_with(const DummyDomain& other) override {
    this->_scalar.join_loop_with(other._scalar);
  }

  void join_iter_with(DummyDomain&& other) override {
    this->_scalar.join_iter_with(std::move(other._scalar));
  }

  void join_iter_with(const DummyDomain& other) override {
    this->_scalar.join_iter_with(other._scalar);
  }

  void widen_with(const DummyDomain& other) override {
    this->_scalar.widen_with(other._scalar);
  }

  void widen_threshold_with(const DummyDomain& other,
                            const MachineInt& threshold) override {
    this->_scalar.widen_threshold_with(other._scalar, threshold);
  }

  void meet_with(const DummyDomain& other) override {
    this->_scalar.meet_with(other._scalar);
  }

  void narrow_with(const DummyDomain& other) override {
    this->_scalar.narrow_with(other._scalar);
  }

  void narrow_threshold_with(const DummyDomain& other,
                             const MachineInt& threshold) override {
    this->_scalar.narrow_threshold_with(other._scalar, threshold);
  }

  DummyDomain join(const DummyDomain& other) const override {
    return DummyDomain(this->_scalar.join(other._scalar));
  }

  DummyDomain join_loop(const DummyDomain& other) const override {
    return DummyDomain(this->_scalar.join_loop(other._scalar));
  }

  DummyDomain join_iter(const DummyDomain& other) const override {
    return DummyDomain(this->_scalar.join_iter(other._scalar));
  }

  DummyDomain widening(const DummyDomain& other) const override {
    return DummyDomain(this->_scalar.widening(other._scalar));
  }

  DummyDomain widening_threshold(const DummyDomain& other,
                                 const MachineInt& threshold) const override {
    return DummyDomain(
        this->_scalar.widening_threshold(other._scalar, threshold));
  }

  DummyDomain meet(const DummyDomain& other) const override {
    return DummyDomain(this->_scalar.meet(other._scalar));
  }

  DummyDomain narrowing(const DummyDomain& other) const override {
    return DummyDomain(this->_scalar.narrowing(other._scalar));
  }

  DummyDomain narrowing_threshold(const DummyDomain& other,
                                  const MachineInt& threshold) const override {
    return DummyDomain(
        this->_scalar.narrowing_threshold(other._scalar, threshold));
  }

  /// @}
  /// \name Implement uninitialized abstract domain methods
  /// @{

  void uninit_assert_initialized(VariableRef x) override {
    this->_scalar.uninit_assert_initialized(x);
  }

  bool uninit_is_initialized(VariableRef x) const override {
    return this->_scalar.uninit_is_initialized(x);
  }

  bool uninit_is_uninitialized(VariableRef x) const override {
    return this->_scalar.uninit_is_uninitialized(x);
  }

  void uninit_refine(VariableRef x, Uninitialized value) override {
    this->_scalar.uninit_refine(x, value);
  }

  Uninitialized uninit_to_uninitialized(VariableRef x) const override {
    return this->_scalar.uninit_to_uninitialized(x);
  }

  /// @}
  /// \name Implement machine integer abstract domain methods
  /// @{

  void int_assign(VariableRef x, const MachineInt& n) override {
    this->_scalar.int_assign(x, n);
  }

  void int_assign_undef(VariableRef x) override {
    this->_scalar.int_assign_undef(x);
  }

  void int_assign_nondet(VariableRef x) override {
    this->_scalar.int_assign_nondet(x);
  }

  void int_assign(VariableRef x, VariableRef y) override {
    this->_scalar.int_assign(x, y);
  }

  void int_assign(VariableRef x, const IntLinearExpression& e) override {
    this->_scalar.int_assign(x, e);
  }

  void int_apply(IntUnaryOperator op, VariableRef x, VariableRef y) override {
    this->_scalar.int_apply(op, x, y);
  }

  void int_apply(IntBinaryOperator op,
                 VariableRef x,
                 VariableRef y,
                 VariableRef z) override {
    this->_scalar.int_apply(op, x, y, z);
  }

  void int_apply(IntBinaryOperator op,
                 VariableRef x,
                 VariableRef y,
                 const MachineInt& z) override {
    this->_scalar.int_apply(op, x, y, z);
  }

  void int_apply(IntBinaryOperator op,
                 VariableRef x,
                 const MachineInt& y,
                 VariableRef z) override {
    this->_scalar.int_apply(op, x, y, z);
  }

  void int_add(IntPredicate pred, VariableRef x, VariableRef y) override {
    this->_scalar.int_add(pred, x, y);
  }

  void int_add(IntPredicate pred, VariableRef x, const MachineInt& y) override {
    this->_scalar.int_add(pred, x, y);
  }

  void int_add(IntPredicate pred, const MachineInt& x, VariableRef y) override {
    this->_scalar.int_add(pred, x, y);
  }

  void int_set(VariableRef x, const IntInterval& value) override {
    this->_scalar.int_set(x, value);
  }

  void int_set(VariableRef x, const IntCongruence& value) override {
    this->_scalar.int_set(x, value);
  }

  void int_set(VariableRef x, const IntIntervalCongruence& value) override {
    this->_scalar.int_set(x, value);
  }

  void int_refine(VariableRef x, const IntInterval& value) override {
    this->_scalar.int_refine(x, value);
  }

  void int_refine(VariableRef x, const IntCongruence& value) override {
    this->_scalar.int_refine(x, value);
  }

  void int_refine(VariableRef x, const IntIntervalCongruence& value) override {
    this->_scalar.int_refine(x, value);
  }

  void int_forget(VariableRef x) override { this->_scalar.int_forget(x); }

  IntInterval int_to_interval(VariableRef x) const override {
    return this->_scalar.int_to_interval(x);
  }

  IntInterval int_to_interval(const IntLinearExpression& e) const override {
    return this->_scalar.int_to_interval(e);
  }

  IntCongruence int_to_congruence(VariableRef x) const override {
    return this->_scalar.int_to_congruence(x);
  }

  IntCongruence int_to_congruence(const IntLinearExpression& e) const override {
    return this->_scalar.int_to_congruence(e);
  }

  IntIntervalCongruence int_to_interval_congruence(
      VariableRef x) const override {
    return this->_scalar.int_to_interval_congruence(x);
  }

  IntIntervalCongruence int_to_interval_congruence(
      const IntLinearExpression& e) const override {
    return this->_scalar.int_to_interval_congruence(e);
  }

  /// @}
  /// \name Implement non-negative loop counter abstract domain methods
  /// @{

  void counter_mark(VariableRef x) override { this->_scalar.counter_mark(x); }

  void counter_unmark(VariableRef x) override {
    this->_scalar.counter_unmark(x);
  }

  void counter_init(VariableRef x, const MachineInt& c) override {
    this->_scalar.counter_init(x, c);
  }

  void counter_incr(VariableRef x, const MachineInt& k) override {
    this->_scalar.counter_incr(x, k);
  }

  void counter_forget(VariableRef x) override {
    this->_scalar.counter_forget(x);
  }

  /// @}
  /// \name Implement floating point abstract domain methods
  /// @{

  void float_assign_undef(VariableRef x) override {
    this->_scalar.float_assign_undef(x);
  }

  void float_assign_nondet(VariableRef x) override {
    this->_scalar.float_assign_nondet(x);
  }

  void float_assign(VariableRef x, VariableRef y) override {
    this->_scalar.float_assign(x, y);
  }

  void float_forget(VariableRef x) override { this->_scalar.float_forget(x); }

  /// @}
  /// \name Implement nullity abstract domain methods
  /// @{

  void nullity_assert_null(VariableRef p) override {
    this->_scalar.nullity_assert_null(p);
  }

  void nullity_assert_non_null(VariableRef p) override {
    this->_scalar.nullity_assert_non_null(p);
  }

  bool nullity_is_null(VariableRef p) const override {
    return this->_scalar.nullity_is_null(p);
  }

  bool nullity_is_non_null(VariableRef p) const override {
    return this->_scalar.nullity_is_non_null(p);
  }

  void nullity_set(VariableRef p, Nullity value) override {
    this->_scalar.nullity_set(p, value);
  }

  void nullity_refine(VariableRef p, Nullity value) override {
    this->_scalar.nullity_refine(p, value);
  }

  Nullity nullity_to_nullity(VariableRef p) const override {
    return this->_scalar.nullity_to_nullity(p);
  }

  /// @}
  /// \name Implement pointer abstract domain methods
  /// @{

  void pointer_assign(VariableRef p,
                      MemoryLocationRef addr,
                      Nullity nullity) override {
    this->_scalar.pointer_assign(p, addr, nullity);
  }

  void pointer_assign_null(VariableRef p) override {
    this->_scalar.pointer_assign_null(p);
  }

  void pointer_assign_undef(VariableRef p) override {
    this->_scalar.pointer_assign_undef(p);
  }

  void pointer_assign_nondet(VariableRef p) override {
    this->_scalar.pointer_assign_nondet(p);
  }

  void pointer_assign(VariableRef p, VariableRef q) override {
    this->_scalar.pointer_assign(p, q);
  }

  void pointer_assign(VariableRef p, VariableRef q, VariableRef o) override {
    this->_scalar.pointer_assign(p, q, o);
  }

  void pointer_assign(VariableRef p,
                      VariableRef q,
                      const MachineInt& o) override {
    this->_scalar.pointer_assign(p, q, o);
  }

  void pointer_assign(VariableRef p,
                      VariableRef q,
                      const IntLinearExpression& o) override {
    this->_scalar.pointer_assign(p, q, o);
  }

  void pointer_add(PointerPredicate pred,
                   VariableRef p,
                   VariableRef q) override {
    this->_scalar.pointer_add(pred, p, q);
  }

  void pointer_refine(VariableRef p, const PointsToSetT& addrs) override {
    this->_scalar.pointer_refine(p, addrs);
  }

  void pointer_refine(VariableRef p,
                      const PointsToSetT& addrs,
                      const IntInterval& offset) override {
    this->_scalar.pointer_refine(p, addrs, offset);
  }

  void pointer_refine(VariableRef p, const PointerAbsValueT& value) override {
    this->_scalar.pointer_refine(p, value);
  }

  void pointer_refine(VariableRef p, const PointerSetT& set) override {
    this->_scalar.pointer_refine(p, set);
  }

  void pointer_offset_to_int(VariableRef x, VariableRef p) override {
    this->_scalar.pointer_offset_to_int(x, p);
  }

  IntInterval pointer_offset_to_interval(VariableRef p) const override {
    return this->_scalar.pointer_offset_to_interval(p);
  }

  IntCongruence pointer_offset_to_congruence(VariableRef p) const override {
    return this->_scalar.pointer_offset_to_congruence(p);
  }

  IntIntervalCongruence pointer_offset_to_interval_congruence(
      VariableRef p) const override {
    return this->_scalar.pointer_offset_to_interval_congruence(p);
  }

  PointsToSetT pointer_to_points_to(VariableRef p) const override {
    return this->_scalar.pointer_to_points_to(p);
  }

  PointerAbsValueT pointer_to_pointer(VariableRef p) const override {
    return this->_scalar.pointer_to_pointer(p);
  }

  void pointer_forget_offset(VariableRef p) override {
    this->_scalar.pointer_forget_offset(p);
  }

  void pointer_forget(VariableRef p) override {
    this->_scalar.pointer_forget(p);
  }

  /// @}
  /// \name Implement dynamically typed variables abstract domain methods
  /// @{

  void dynamic_assign(VariableRef x, VariableRef y) override {
    this->_scalar.dynamic_assign(x, y);
  }

  void dynamic_write_undef(VariableRef x) override {
    this->_scalar.dynamic_write_undef(x);
  }

  void dynamic_write_nondet(VariableRef x) override {
    this->_scalar.dynamic_write_nondet(x);
  }

  void dynamic_write_int(VariableRef x, const MachineInt& n) override {
    this->_scalar.dynamic_write_int(x, n);
  }

  void dynamic_write_nondet_int(VariableRef x) override {
    this->_scalar.dynamic_write_nondet_int(x);
  }

  void dynamic_write_int(VariableRef x, VariableRef y) override {
    this->_scalar.dynamic_write_int(x, y);
  }

  void dynamic_write_nondet_float(VariableRef x) override {
    this->_scalar.dynamic_write_nondet_float(x);
  }

  void dynamic_write_null(VariableRef x) override {
    this->_scalar.dynamic_write_null(x);
  }

  void dynamic_write_pointer(VariableRef x,
                             MemoryLocationRef addr,
                             Nullity nullity) override {
    this->_scalar.dynamic_write_pointer(x, addr, nullity);
  }

  void dynamic_write_pointer(VariableRef x, VariableRef y) override {
    this->_scalar.dynamic_write_pointer(x, y);
  }

  void dynamic_read_int(VariableRef x, VariableRef y) override {
    this->_scalar.dynamic_read_int(x, y);
  }

  void dynamic_read_pointer(VariableRef x, VariableRef y) override {
    this->_scalar.dynamic_read_pointer(x, y);
  }

  bool dynamic_is_zero(VariableRef x) const override {
    return this->_scalar.dynamic_is_zero(x);
  }

  bool dynamic_is_null(VariableRef x) const override {
    return this->_scalar.dynamic_is_null(x);
  }

  void dynamic_forget(VariableRef x) override {
    this->_scalar.dynamic_forget(x);
  }

  /// @}
  /// \name Implement scalar abstract domain methods
  /// @{

  void scalar_assign_undef(VariableRef x) override {
    this->_scalar.scalar_assign_undef(x);
  }

  void scalar_assign_nondet(VariableRef x) override {
    this->_scalar.scalar_assign_nondet(x);
  }

  void scalar_pointer_to_int(VariableRef x,
                             VariableRef p,
                             MemoryLocationRef absolute_zero) override {
    this->_scalar.scalar_pointer_to_int(x, p, absolute_zero);
  }

  void scalar_int_to_pointer(VariableRef p,
                             VariableRef x,
                             MemoryLocationRef absolute_zero) override {
    this->_scalar.scalar_int_to_pointer(p, x, absolute_zero);
  }

  void scalar_forget(VariableRef x) override { this->_scalar.scalar_forget(x); }

  /// @}
  /// \name Implement memory abstract domain methods
  /// @{

  void mem_write(VariableRef p,
                 const LiteralT& v,
                 const MachineInt& /*size*/) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    if (this->_scalar.is_bottom()) {
      return;
    }

    this->_scalar.nullity_assert_non_null(p);

    // Writing an uninitialized variable is an error
    // Note that writing the undefined constant is allowed
    if (v.is_var()) {
      this->_scalar.uninit_assert_initialized(v.var());
    }
  }

  void mem_read(const LiteralT& x,
                VariableRef p,
                const MachineInt& /*size*/) override {
    ikos_assert(x.is_var());
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    if (this->_scalar.is_bottom()) {
      return;
    }

    this->_scalar.nullity_assert_non_null(p);

    // Assign x to a non deterministic value
    // Reading uninitialized memory is an error
    // Therefore, the result of a read is always initialized
    this->_scalar.scalar_assign_nondet(x.var());
  }

  void mem_copy(VariableRef dest,
                VariableRef src,
                const LiteralT& size) override {
    ikos_assert(ScalarVariableTrait::is_pointer(dest));
    ikos_assert(ScalarVariableTrait::is_pointer(src));

    if (this->_scalar.is_bottom()) {
      return;
    }

    this->_scalar.nullity_assert_non_null(src);
    this->_scalar.nullity_assert_non_null(dest);

    if (size.is_undefined()) {
      this->_scalar.set_to_bottom();
      return;
    } else if (size.is_var()) {
      this->_scalar.uninit_assert_initialized(size.var());
    }
  }

  void mem_set(VariableRef dest,
               const LiteralT& value,
               const LiteralT& size) override {
    ikos_assert(ScalarVariableTrait::is_pointer(dest));

    if (this->_scalar.is_bottom()) {
      return;
    }

    this->_scalar.nullity_assert_non_null(dest);

    if (value.is_undefined()) {
      this->_scalar.set_to_bottom();
      return;
    } else if (value.is_var()) {
      this->_scalar.uninit_assert_initialized(value.var());
    }

    if (size.is_undefined()) {
      this->_scalar.set_to_bottom();
      return;
    } else if (size.is_var()) {
      this->_scalar.uninit_assert_initialized(size.var());
    }
  }

  void mem_forget_all() override {}

  void mem_forget(MemoryLocationRef) override {}

  void mem_forget(MemoryLocationRef,
                  const IntInterval&,
                  const MachineInt&) override {}

  void mem_forget(MemoryLocationRef, const IntInterval&) override {}

  void mem_forget_reachable(VariableRef p) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    this->_scalar.uninit_assert_initialized(p);
  }

  void mem_forget_reachable(VariableRef p,
                            const MachineInt& /*size*/) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    this->_scalar.uninit_assert_initialized(p);
  }

  void mem_abstract_reachable(VariableRef p) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    this->_scalar.uninit_assert_initialized(p);
  }

  void mem_abstract_reachable(VariableRef p,
                              const MachineInt& /*size*/) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    this->_scalar.uninit_assert_initialized(p);
  }

  void mem_zero_reachable(VariableRef p) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    this->_scalar.uninit_assert_initialized(p);
  }

  void mem_uninitialize_reachable(VariableRef p) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    this->_scalar.uninit_assert_initialized(p);
  }

  /// @}
  /// \name Lifetime abstract domain methods
  /// @{

  void lifetime_assign_allocated(MemoryLocationRef) override {}

  void lifetime_assign_deallocated(MemoryLocationRef) override {}

  void lifetime_assert_allocated(MemoryLocationRef) override {}

  void lifetime_assert_deallocated(MemoryLocationRef) override {}

  void lifetime_set(MemoryLocationRef, Lifetime value) override {
    if (value.is_bottom()) {
      this->_scalar.set_to_bottom();
    }
  }

  void lifetime_forget(MemoryLocationRef) override {}

  Lifetime lifetime_to_lifetime(MemoryLocationRef) const override {
    if (this->is_bottom()) {
      return Lifetime::bottom();
    } else {
      return Lifetime::top();
    }
  }

  /// @}
  /// \name Partitioning abstract domain methods
  /// @{

  void partitioning_set_variable(VariableRef) override {}

  boost::optional< VariableRef > partitioning_variable() const override {
    return boost::none;
  }

  void partitioning_join() override {}

  void partitioning_disable() override {}

  /// @}

  void dump(std::ostream& o) const override { this->_scalar.dump(o); }

  static std::string name() {
    return "dummy memory domain using " + ScalarDomain::name();
  }

}; // end class DummyDomain

} // end namespace memory
} // end namespace core
} // end namespace ikos
