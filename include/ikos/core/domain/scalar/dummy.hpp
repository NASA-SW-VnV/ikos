/*******************************************************************************
 *
 * \file
 * \brief A dummy scalar abstract domain
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

#include <ikos/core/domain/scalar/abstract_domain.hpp>

namespace ikos {
namespace core {
namespace scalar {

/// \brief Dummy scalar abstract domain
///
/// This class implements a scalar abstract domain that ignores all operations.
template < typename VariableRef, typename MemoryLocationRef >
class DummyDomain final : public scalar::AbstractDomain<
                              VariableRef,
                              MemoryLocationRef,
                              DummyDomain< VariableRef, MemoryLocationRef > > {
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

private:
  using IntVariableTrait = machine_int::VariableTraits< VariableRef >;
  using ScalarVariableTrait = scalar::VariableTraits< VariableRef >;

private:
  bool _is_bottom;

private:
  /// \brief Private constructor
  explicit DummyDomain(bool is_bottom) : _is_bottom(is_bottom) {}

public:
  /// \brief Create the top abstract value
  static DummyDomain top() { return DummyDomain(false); }

  /// \brief Create the bottom abstract value
  static DummyDomain bottom() { return DummyDomain(true); }

  /// \brief Copy constructor
  DummyDomain(const DummyDomain&) noexcept = default;

  /// \brief Move constructor
  DummyDomain(DummyDomain&&) noexcept = default;

  /// \brief Copy assignment operator
  DummyDomain& operator=(const DummyDomain&) noexcept = default;

  /// \brief Move assignment operator
  DummyDomain& operator=(DummyDomain&&) noexcept = default;

  /// \brief Destructor
  ~DummyDomain() override = default;

  /// \name Implement core abstract domain methods
  /// @{

  void normalize() override {}

  bool is_bottom() const override { return this->_is_bottom; }

  bool is_top() const override { return !this->_is_bottom; }

  void set_to_bottom() override { this->_is_bottom = true; }

  void set_to_top() override { this->_is_bottom = false; }

  bool leq(const DummyDomain& other) const override {
    return static_cast< int >(this->_is_bottom) >=
           static_cast< int >(other._is_bottom);
  }

  bool equals(const DummyDomain& other) const override {
    return this->_is_bottom == other._is_bottom;
  }

  void join_with(const DummyDomain& other) override {
    this->_is_bottom = (this->_is_bottom && other._is_bottom);
  }

  void widen_with(const DummyDomain& other) override { this->join_with(other); }

  void widen_threshold_with(const DummyDomain& other,
                            const MachineInt& /*threshold*/) override {
    this->join_with(other);
  }

  void meet_with(const DummyDomain& other) override {
    this->_is_bottom = (this->_is_bottom || other._is_bottom);
  }

  void narrow_with(const DummyDomain& other) override {
    this->meet_with(other);
  }

  void narrow_threshold_with(const DummyDomain& other,
                             const MachineInt& /*threshold*/) override {
    this->meet_with(other);
  }

  /// @}
  /// \name Implement uninitialized abstract domain methods
  /// @{

  void uninit_assert_initialized(VariableRef) override {}

  bool uninit_is_initialized(VariableRef) const override {
    return this->_is_bottom;
  }

  bool uninit_is_uninitialized(VariableRef) const override {
    return this->_is_bottom;
  }

  void uninit_refine(VariableRef, Uninitialized value) override {
    if (value.is_bottom()) {
      this->_is_bottom = true;
    }
  }

  Uninitialized uninit_to_uninitialized(VariableRef) const override {
    if (this->_is_bottom) {
      return Uninitialized::bottom();
    } else {
      return Uninitialized::top();
    }
  }

  /// @}
  /// \name Implement machine integer abstract domain methods
  /// @{

  void int_assign(VariableRef, const MachineInt&) override {}

  void int_assign_undef(VariableRef) override {}

  void int_assign_nondet(VariableRef) override {}

  void int_assign(VariableRef, VariableRef) override {}

  void int_assign(VariableRef, const IntLinearExpression&) override {}

  void int_apply(IntUnaryOperator, VariableRef, VariableRef) override {}

  void int_apply(IntBinaryOperator,
                 VariableRef,
                 VariableRef,
                 VariableRef) override {}

  void int_apply(IntBinaryOperator,
                 VariableRef,
                 VariableRef,
                 const MachineInt&) override {}

  void int_apply(IntBinaryOperator,
                 VariableRef,
                 const MachineInt&,
                 VariableRef) override {}

  void int_add(IntPredicate, VariableRef, VariableRef) override {}

  void int_add(IntPredicate, VariableRef, const MachineInt&) override {}

  void int_add(IntPredicate, const MachineInt&, VariableRef) override {}

  void int_set(VariableRef, const IntInterval& value) override {
    if (value.is_bottom()) {
      this->_is_bottom = true;
    }
  }

  void int_set(VariableRef, const IntCongruence& value) override {
    if (value.is_bottom()) {
      this->_is_bottom = true;
    }
  }

  void int_set(VariableRef, const IntIntervalCongruence& value) override {
    if (value.is_bottom()) {
      this->_is_bottom = true;
    }
  }

  void int_refine(VariableRef, const IntInterval& value) override {
    if (value.is_bottom()) {
      this->_is_bottom = true;
    }
  }

  void int_refine(VariableRef, const IntCongruence& value) override {
    if (value.is_bottom()) {
      this->_is_bottom = true;
    }
  }

  void int_refine(VariableRef, const IntIntervalCongruence& value) override {
    if (value.is_bottom()) {
      this->_is_bottom = true;
    }
  }

  void int_forget(VariableRef) override {}

  IntInterval int_to_interval(VariableRef x) const override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    if (this->_is_bottom) {
      return IntInterval::bottom(IntVariableTrait::bit_width(x),
                                 IntVariableTrait::sign(x));
    } else {
      return IntInterval::top(IntVariableTrait::bit_width(x),
                              IntVariableTrait::sign(x));
    }
  }

  IntInterval int_to_interval(const IntLinearExpression& e) const override {
    if (this->_is_bottom) {
      return IntInterval::bottom(e.constant().bit_width(), e.constant().sign());
    } else {
      return IntInterval::top(e.constant().bit_width(), e.constant().sign());
    }
  }

  IntCongruence int_to_congruence(VariableRef x) const override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    if (this->_is_bottom) {
      return IntCongruence::bottom(IntVariableTrait::bit_width(x),
                                   IntVariableTrait::sign(x));
    } else {
      return IntCongruence::top(IntVariableTrait::bit_width(x),
                                IntVariableTrait::sign(x));
    }
  }

  IntCongruence int_to_congruence(const IntLinearExpression& e) const override {
    if (this->_is_bottom) {
      return IntCongruence::bottom(e.constant().bit_width(),
                                   e.constant().sign());
    } else {
      return IntCongruence::top(e.constant().bit_width(), e.constant().sign());
    }
  }

  IntIntervalCongruence int_to_interval_congruence(
      VariableRef x) const override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    if (this->_is_bottom) {
      return IntIntervalCongruence::bottom(IntVariableTrait::bit_width(x),
                                           IntVariableTrait::sign(x));
    } else {
      return IntIntervalCongruence::top(IntVariableTrait::bit_width(x),
                                        IntVariableTrait::sign(x));
    }
  }

  IntIntervalCongruence int_to_interval_congruence(
      const IntLinearExpression& e) const override {
    if (this->_is_bottom) {
      return IntIntervalCongruence::bottom(e.constant().bit_width(),
                                           e.constant().sign());
    } else {
      return IntIntervalCongruence::top(e.constant().bit_width(),
                                        e.constant().sign());
    }
  }

  /// @}
  /// \name Implement non-negative loop counter abstract domain methods
  /// @{

  void counter_mark(VariableRef) override {}

  void counter_unmark(VariableRef) override {}

  void counter_init(VariableRef, const MachineInt&) override {}

  void counter_incr(VariableRef, const MachineInt&) override {}

  void counter_forget(VariableRef) override {}

  /// @}
  /// \name Implement floating point abstract domain methods
  /// @{

  void float_assign_undef(VariableRef) override {}

  void float_assign_nondet(VariableRef) override {}

  void float_assign(VariableRef, VariableRef) override {}

  void float_forget(VariableRef) override {}

  /// @}
  /// \name Implement nullity abstract domain methods
  /// @{

  void nullity_assert_null(VariableRef) override {}

  void nullity_assert_non_null(VariableRef) override {}

  bool nullity_is_null(VariableRef) const override { return this->_is_bottom; }

  bool nullity_is_non_null(VariableRef) const override {
    return this->_is_bottom;
  }

  void nullity_set(VariableRef, Nullity value) override {
    if (value.is_bottom()) {
      this->_is_bottom = true;
    }
  }

  void nullity_refine(VariableRef, Nullity value) override {
    if (value.is_bottom()) {
      this->_is_bottom = true;
    }
  }

  Nullity nullity_to_nullity(VariableRef) const override {
    if (this->_is_bottom) {
      return Nullity::bottom();
    } else {
      return Nullity::top();
    }
  }

  /// @}
  /// \name Implement pointer abstract domain methods
  /// @{

  void pointer_assign(VariableRef, MemoryLocationRef, Nullity) override {}

  void pointer_assign_null(VariableRef) override {}

  void pointer_assign_undef(VariableRef) override {}

  void pointer_assign_nondet(VariableRef) override {}

  void pointer_assign(VariableRef, VariableRef) override {}

  void pointer_assign(VariableRef, VariableRef, VariableRef) override {}

  void pointer_assign(VariableRef, VariableRef, const MachineInt&) override {}

  void pointer_assign(VariableRef,
                      VariableRef,
                      const IntLinearExpression&) override {}

  void pointer_add(PointerPredicate, VariableRef, VariableRef) override {}

  void pointer_refine(VariableRef, const PointsToSetT& addrs) override {
    if (addrs.is_bottom()) {
      this->_is_bottom = true;
    }
  }

  void pointer_refine(VariableRef,
                      const PointsToSetT& addrs,
                      const IntInterval& offset) override {
    if (addrs.is_bottom() || offset.is_bottom()) {
      this->_is_bottom = true;
    }
  }

  void pointer_refine(VariableRef, const PointerAbsValueT& value) override {
    if (value.is_bottom()) {
      this->_is_bottom = true;
    }
  }

  void pointer_refine(VariableRef, const PointerSetT& set) override {
    if (set.is_bottom()) {
      this->_is_bottom = true;
    }
  }

  void pointer_offset_to_int(VariableRef, VariableRef) override {}

  IntInterval pointer_offset_to_interval(VariableRef p) const override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    auto o = ScalarVariableTrait::offset_var(p);
    if (this->_is_bottom) {
      return IntInterval::bottom(IntVariableTrait::bit_width(o),
                                 IntVariableTrait::sign(o));
    } else {
      return IntInterval::top(IntVariableTrait::bit_width(o),
                              IntVariableTrait::sign(o));
    }
  }

  IntCongruence pointer_offset_to_congruence(VariableRef p) const override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    auto o = ScalarVariableTrait::offset_var(p);
    if (this->_is_bottom) {
      return IntCongruence::bottom(IntVariableTrait::bit_width(o),
                                   IntVariableTrait::sign(o));
    } else {
      return IntCongruence::top(IntVariableTrait::bit_width(o),
                                IntVariableTrait::sign(o));
    }
  }

  IntIntervalCongruence pointer_offset_to_interval_congruence(
      VariableRef p) const override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    auto o = ScalarVariableTrait::offset_var(p);
    if (this->_is_bottom) {
      return IntIntervalCongruence::bottom(IntVariableTrait::bit_width(o),
                                           IntVariableTrait::sign(o));
    } else {
      return IntIntervalCongruence::top(IntVariableTrait::bit_width(o),
                                        IntVariableTrait::sign(o));
    }
  }

  PointsToSetT pointer_to_points_to(VariableRef) const override {
    if (this->_is_bottom) {
      return PointsToSetT::bottom();
    } else {
      return PointsToSetT::top();
    }
  }

  PointerAbsValueT pointer_to_pointer(VariableRef p) const override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    auto o = ScalarVariableTrait::offset_var(p);
    if (this->_is_bottom) {
      return PointerAbsValueT::bottom(IntVariableTrait::bit_width(o),
                                      IntVariableTrait::sign(o));
    } else {
      return PointerAbsValueT::top(IntVariableTrait::bit_width(o),
                                   IntVariableTrait::sign(o));
    }
  }

  void pointer_forget_offset(VariableRef) override {}

  void pointer_forget(VariableRef) override {}

  /// @}
  /// \name Implement dynamically typed variables abstract domain methods
  /// @{

  void dynamic_assign(VariableRef, VariableRef) override {}

  void dynamic_write_undef(VariableRef) override {}

  void dynamic_write_nondet(VariableRef) override {}

  void dynamic_write_int(VariableRef, const MachineInt&) override {}

  void dynamic_write_nondet_int(VariableRef) override {}

  void dynamic_write_int(VariableRef, VariableRef) override {}

  void dynamic_write_nondet_float(VariableRef) override {}

  void dynamic_write_null(VariableRef) override {}

  void dynamic_write_pointer(VariableRef, MemoryLocationRef, Nullity) override {
  }

  void dynamic_write_pointer(VariableRef, VariableRef) override {}

  void dynamic_read_int(VariableRef, VariableRef) override {}

  void dynamic_read_pointer(VariableRef, VariableRef) override {}

  bool dynamic_is_zero(VariableRef) const override { return this->_is_bottom; }

  bool dynamic_is_null(VariableRef) const override { return this->_is_bottom; }

  void dynamic_forget(VariableRef) override {}

  /// @}
  /// \name Implement scalar abstract domain methods
  /// @{

  void scalar_assign_undef(VariableRef) override {}

  void scalar_assign_nondet(VariableRef) override {}

  void scalar_pointer_to_int(VariableRef,
                             VariableRef,
                             MemoryLocationRef) override {}

  virtual void scalar_int_to_pointer(VariableRef,
                                     VariableRef,
                                     MemoryLocationRef) override {}

  void scalar_forget(VariableRef) override {}

  /// @}

  void dump(std::ostream& o) const override {
    if (this->_is_bottom) {
      o << "⊥";
    } else {
      o << "T";
    }
  }

  static std::string name() { return "dummy scalar domain"; }

}; // end class DummyDomain

} // end namespace scalar
} // end namespace core
} // end namespace ikos
