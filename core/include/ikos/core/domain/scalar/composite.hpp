/*******************************************************************************
 *
 * \file
 * \brief Scalar abstract domain made from several underlying abstract domains
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

#include <ikos/core/domain/machine_int/abstract_domain.hpp>
#include <ikos/core/domain/nullity/abstract_domain.hpp>
#include <ikos/core/domain/scalar/abstract_domain.hpp>
#include <ikos/core/domain/separate_domain.hpp>
#include <ikos/core/domain/uninitialized/abstract_domain.hpp>

namespace ikos {
namespace core {
namespace scalar {

/// \brief Scalar abstract domain made from several underlying abstract domains
///
/// The composite domain is a scalar domain implemented on top of:
///   * An uninitialized abstract domain
///   * A machine integer abstract domain
///   * A nullity abstract domain
///
/// The composite domain can express invariants on machine integer variables,
/// floating point variables and pointer variables.
///
/// For each machine integer `x`, it keeps track of its value and whether it is
/// initialized or not.
///
/// For each floating point `x`, it keeps track of whether it is initialized or
/// not.
///
/// For each pointer `p`, it keep track of its address, its offset, its nullity
/// and whether it is initialized or not.
///
/// The offset of `p` is modelled directly by the underlying machine integer
/// abstract domain `MachineIntDomain` with the special variable
/// `offset_var(p)`.
///
/// The address of `p` is modelled by keeping track of all possible memory
/// locations (e.g., &'s and malloc's) to which `p` may point to, also called
/// the points-to set.
///
/// The nullity (null/non-null) of `p` is modelled by the underlying nullity
/// abstract domain `NullityDomain`.
///
/// The initialization of a variable `x` is modelled by the underlying
/// uninitialized abstract domain `UninitializedDomain`.
template < typename VariableRef,
           typename MemoryLocationRef,
           typename UninitializedDomain,
           typename MachineIntDomain,
           typename NullityDomain >
class CompositeDomain final
    : public scalar::AbstractDomain< VariableRef,
                                     MemoryLocationRef,
                                     CompositeDomain< VariableRef,
                                                      MemoryLocationRef,
                                                      UninitializedDomain,
                                                      MachineIntDomain,
                                                      NullityDomain > > {
public:
  static_assert(
      uninitialized::IsAbstractDomain< UninitializedDomain,
                                       VariableRef >::value,
      "UninitializedDomain must implement uninitialized::AbstractDomain");
  static_assert(
      machine_int::IsAbstractDomain< MachineIntDomain, VariableRef >::value,
      "MachineIntDomain must implement machine_int::AbstractDomain");
  static_assert(nullity::IsAbstractDomain< NullityDomain, VariableRef >::value,
                "NullityDomain must implement nullity::AbstractDomain");

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
  using PointsToMap = SeparateDomain< VariableRef, PointsToSetT >;
  using IntVariableTrait = machine_int::VariableTraits< VariableRef >;
  using ScalarVariableTrait = scalar::VariableTraits< VariableRef >;

private:
  /// \brief Underlying uninitialized abstract domains
  UninitializedDomain _uninitialized;

  /// \brief Underlying machine integer abstract domains
  MachineIntDomain _integer;

  /// \brief Underlying nullity abstract domains
  NullityDomain _nullity;

  /// \brief Map pointer variables to set of addresses
  PointsToMap _points_to_map;

private:
  /// \brief Constructor
  CompositeDomain(UninitializedDomain uninitialized,
                  MachineIntDomain integer,
                  NullityDomain nullity,
                  PointsToMap points_to_map)
      : _uninitialized(std::move(uninitialized)),
        _integer(std::move(integer)),
        _nullity(std::move(nullity)),
        _points_to_map(std::move(points_to_map)) {
    this->normalize();
  }

public:
  /// \brief Create an abstract value with the given underlying abstract values
  ///
  /// \param uninitialized The uninitialized abstract value
  /// \param integer The machine integer abstract value
  /// \param nullity The nullity abstract value
  CompositeDomain(UninitializedDomain uninitialized,
                  MachineIntDomain integer,
                  NullityDomain nullity)
      : _uninitialized(std::move(uninitialized)),
        _integer(std::move(integer)),
        _nullity(std::move(nullity)),
        _points_to_map(PointsToMap::top()) {
    this->normalize();
  }

  /// \brief Copy constructor
  CompositeDomain(const CompositeDomain&) noexcept(
      (std::is_nothrow_copy_constructible< UninitializedDomain >::value) &&
      (std::is_nothrow_copy_constructible< MachineIntDomain >::value) &&
      (std::is_nothrow_copy_constructible< NullityDomain >::value)) = default;

  /// \brief Move constructor
  CompositeDomain(CompositeDomain&&) noexcept(
      (std::is_nothrow_move_constructible< UninitializedDomain >::value) &&
      (std::is_nothrow_move_constructible< MachineIntDomain >::value) &&
      (std::is_nothrow_move_constructible< NullityDomain >::value)) = default;

  /// \brief Copy assignment operator
  CompositeDomain& operator=(const CompositeDomain&) noexcept(
      (std::is_nothrow_copy_assignable< UninitializedDomain >::value) &&
      (std::is_nothrow_copy_assignable< MachineIntDomain >::value) &&
      (std::is_nothrow_copy_assignable< NullityDomain >::value)) = default;

  /// \brief Move assignment operator
  CompositeDomain& operator=(CompositeDomain&&) noexcept(
      (std::is_nothrow_move_assignable< UninitializedDomain >::value) &&
      (std::is_nothrow_move_assignable< MachineIntDomain >::value) &&
      (std::is_nothrow_move_assignable< NullityDomain >::value)) = default;

  /// \brief Destructor
  ~CompositeDomain() override = default;

  /// \name Implement core abstract domain methods
  /// @{

  void normalize() override {
    this->_uninitialized.normalize();
    if (this->_uninitialized.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_nullity.normalize();
    if (this->_nullity.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_points_to_map.normalize();
    if (this->_points_to_map.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_integer.normalize();
    if (this->_integer.is_bottom()) {
      this->set_to_bottom();
      return;
    }
  }

private:
  /// \brief Return true if the abstract value is bottom
  ///
  /// This is not always correct since it doesn't check this->_integer
  bool is_bottom_fast() const { return this->_uninitialized.is_bottom(); }

public:
  bool is_bottom() const override {
    return this->_uninitialized.is_bottom() || this->_nullity.is_bottom() ||
           this->_points_to_map.is_bottom() || this->_integer.is_bottom();
  }

  bool is_top() const override {
    return this->_uninitialized.is_top() && this->_nullity.is_top() &&
           this->_points_to_map.is_top() && this->_integer.is_top();
  }

  void set_to_bottom() override {
    this->_uninitialized.set_to_bottom();
    this->_integer.set_to_bottom();
    this->_nullity.set_to_bottom();
    this->_points_to_map.set_to_bottom();
  }

  void set_to_top() override {
    this->_uninitialized.set_to_top();
    this->_integer.set_to_top();
    this->_nullity.set_to_top();
    this->_points_to_map.set_to_top();
  }

  bool leq(const CompositeDomain& other) const override {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_uninitialized.leq(other._uninitialized) &&
             this->_integer.leq(other._integer) &&
             this->_nullity.leq(other._nullity) &&
             this->_points_to_map.leq(other._points_to_map);
    }
  }

  bool equals(const CompositeDomain& other) const override {
    if (this->is_bottom()) {
      return other.is_bottom();
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_uninitialized.equals(other._uninitialized) &&
             this->_integer.equals(other._integer) &&
             this->_nullity.equals(other._nullity) &&
             this->_points_to_map.equals(other._points_to_map);
    }
  }

  void join_with(CompositeDomain&& other) override {
    this->normalize();
    other.normalize();
    if (this->is_bottom()) {
      this->operator=(std::move(other));
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_uninitialized.join_with(std::move(other._uninitialized));
      this->_integer.join_with(std::move(other._integer));
      this->_nullity.join_with(std::move(other._nullity));
      this->_points_to_map.join_with(std::move(other._points_to_map));
    }
  }

  void join_with(const CompositeDomain& other) override {
    this->normalize();
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_uninitialized.join_with(other._uninitialized);
      this->_integer.join_with(other._integer);
      this->_nullity.join_with(other._nullity);
      this->_points_to_map.join_with(other._points_to_map);
    }
  }

  void join_loop_with(CompositeDomain&& other) override {
    this->normalize();
    other.normalize();
    if (this->is_bottom()) {
      this->operator=(std::move(other));
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_uninitialized.join_loop_with(std::move(other._uninitialized));
      this->_integer.join_loop_with(std::move(other._integer));
      this->_nullity.join_loop_with(std::move(other._nullity));
      this->_points_to_map.join_loop_with(std::move(other._points_to_map));
    }
  }

  void join_loop_with(const CompositeDomain& other) override {
    this->normalize();
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_uninitialized.join_loop_with(other._uninitialized);
      this->_integer.join_loop_with(other._integer);
      this->_nullity.join_loop_with(other._nullity);
      this->_points_to_map.join_loop_with(other._points_to_map);
    }
  }

  void join_iter_with(CompositeDomain&& other) override {
    this->normalize();
    other.normalize();
    if (this->is_bottom()) {
      this->operator=(std::move(other));
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_uninitialized.join_iter_with(std::move(other._uninitialized));
      this->_integer.join_iter_with(std::move(other._integer));
      this->_nullity.join_iter_with(std::move(other._nullity));
      this->_points_to_map.join_iter_with(std::move(other._points_to_map));
    }
  }

  void join_iter_with(const CompositeDomain& other) override {
    this->normalize();
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_uninitialized.join_iter_with(other._uninitialized);
      this->_integer.join_iter_with(other._integer);
      this->_nullity.join_iter_with(other._nullity);
      this->_points_to_map.join_iter_with(other._points_to_map);
    }
  }

  void widen_with(const CompositeDomain& other) override {
    this->normalize();
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_uninitialized.widen_with(other._uninitialized);
      this->_integer.widen_with(other._integer);
      this->_nullity.widen_with(other._nullity);
      this->_points_to_map.widen_with(other._points_to_map);
    }
  }

  void widen_threshold_with(const CompositeDomain& other,
                            const MachineInt& threshold) override {
    this->normalize();
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_uninitialized.widen_with(other._uninitialized);
      this->_integer.widen_threshold_with(other._integer, threshold);
      this->_nullity.widen_with(other._nullity);
      this->_points_to_map.widen_with(other._points_to_map);
    }
  }

  void meet_with(const CompositeDomain& other) override {
    this->normalize();
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_uninitialized.meet_with(other._uninitialized);
      this->_integer.meet_with(other._integer);
      this->_nullity.meet_with(other._nullity);
      this->_points_to_map.meet_with(other._points_to_map);
    }
  }

  void narrow_with(const CompositeDomain& other) override {
    this->normalize();
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_uninitialized.narrow_with(other._uninitialized);
      this->_integer.narrow_with(other._integer);
      this->_nullity.narrow_with(other._nullity);
      this->_points_to_map.narrow_with(other._points_to_map);
    }
  }

  void narrow_threshold_with(const CompositeDomain& other,
                             const MachineInt& threshold) override {
    this->normalize();
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_uninitialized.narrow_with(other._uninitialized);
      this->_integer.narrow_threshold_with(other._integer, threshold);
      this->_nullity.narrow_with(other._nullity);
      this->_points_to_map.narrow_with(other._points_to_map);
    }
  }

  CompositeDomain join(const CompositeDomain& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return CompositeDomain(this->_uninitialized.join(other._uninitialized),
                             this->_integer.join(other._integer),
                             this->_nullity.join(other._nullity),
                             this->_points_to_map.join(other._points_to_map));
    }
  }

  CompositeDomain join_loop(const CompositeDomain& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return CompositeDomain(this->_uninitialized.join_loop(
                                 other._uninitialized),
                             this->_integer.join_loop(other._integer),
                             this->_nullity.join_loop(other._nullity),
                             this->_points_to_map.join_loop(
                                 other._points_to_map));
    }
  }

  CompositeDomain join_iter(const CompositeDomain& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return CompositeDomain(this->_uninitialized.join_iter(
                                 other._uninitialized),
                             this->_integer.join_iter(other._integer),
                             this->_nullity.join_iter(other._nullity),
                             this->_points_to_map.join_iter(
                                 other._points_to_map));
    }
  }

  CompositeDomain widening(const CompositeDomain& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return CompositeDomain(this->_uninitialized.widening(
                                 other._uninitialized),
                             this->_integer.widening(other._integer),
                             this->_nullity.widening(other._nullity),
                             this->_points_to_map.widening(
                                 other._points_to_map));
    }
  }

  CompositeDomain widening_threshold(
      const CompositeDomain& other,
      const MachineInt& threshold) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return CompositeDomain(this->_uninitialized.widening(
                                 other._uninitialized),
                             this->_integer.widening_threshold(other._integer,
                                                               threshold),
                             this->_nullity.widening(other._nullity),
                             this->_points_to_map.widening(
                                 other._points_to_map));
    }
  }

  CompositeDomain meet(const CompositeDomain& other) const override {
    if (this->is_bottom()) {
      return *this;
    } else if (other.is_bottom()) {
      return other;
    } else {
      return CompositeDomain(this->_uninitialized.meet(other._uninitialized),
                             this->_integer.meet(other._integer),
                             this->_nullity.meet(other._nullity),
                             this->_points_to_map.meet(other._points_to_map));
    }
  }

  CompositeDomain narrowing(const CompositeDomain& other) const override {
    if (this->is_bottom()) {
      return *this;
    } else if (other.is_bottom()) {
      return other;
    } else {
      return CompositeDomain(this->_uninitialized.narrowing(
                                 other._uninitialized),
                             this->_integer.narrowing(other._integer),
                             this->_nullity.narrowing(other._nullity),
                             this->_points_to_map.narrowing(
                                 other._points_to_map));
    }
  }

  CompositeDomain narrowing_threshold(
      const CompositeDomain& other,
      const MachineInt& threshold) const override {
    if (this->is_bottom()) {
      return *this;
    } else if (other.is_bottom()) {
      return other;
    } else {
      return CompositeDomain(this->_uninitialized.narrowing(
                                 other._uninitialized),
                             this->_integer.narrowing_threshold(other._integer,
                                                                threshold),
                             this->_nullity.narrowing(other._nullity),
                             this->_points_to_map.narrowing(
                                 other._points_to_map));
    }
  }

  /// @}
  /// \name Implement uninitialized abstract domain methods
  /// @{

  void uninit_assert_initialized(VariableRef x) override {
    this->_uninitialized.assert_initialized(x);
  }

  bool uninit_is_initialized(VariableRef x) const override {
    return this->_uninitialized.is_initialized(x);
  }

  bool uninit_is_uninitialized(VariableRef x) const override {
    return this->_uninitialized.is_uninitialized(x);
  }

  void uninit_refine(VariableRef x, Uninitialized value) override {
    this->_uninitialized.refine(x, value);
  }

  Uninitialized uninit_to_uninitialized(VariableRef x) const override {
    return this->_uninitialized.get(x);
  }

  /// @}
  /// \name Implement machine integer abstract domain methods
  /// @{

  void int_assign(VariableRef x, const MachineInt& n) override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign_initialized(x);
    this->_integer.assign(x, n);
  }

  void int_assign_undef(VariableRef x) override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign_uninitialized(x);
    this->_integer.forget(x);
  }

  void int_assign_nondet(VariableRef x) override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign_initialized(x);
    this->_integer.forget(x);
  }

  void int_assign(VariableRef x, VariableRef y) override {
    ikos_assert(ScalarVariableTrait::is_int(x));
    ikos_assert(ScalarVariableTrait::is_int(y));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign(x, y);
    this->_integer.assign(x, y);
  }

  void int_assign(VariableRef x, const IntLinearExpression& e) override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    if (this->is_bottom_fast()) {
      return;
    }

    for (const auto& term : e) {
      ikos_assert(ScalarVariableTrait::is_int(term.first));
      this->_uninitialized.assert_initialized(term.first);
    }

    if (this->_uninitialized.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_uninitialized.assign_initialized(x);
    this->_integer.assign(x, e);
  }

  void int_apply(IntUnaryOperator op, VariableRef x, VariableRef y) override {
    ikos_assert(ScalarVariableTrait::is_int(x));
    ikos_assert(ScalarVariableTrait::is_int(y));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assert_initialized(y);

    if (this->_uninitialized.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_uninitialized.assign_initialized(x);
    this->_integer.apply(op, x, y);
  }

  // \brief Assert that x is initialized (throw if not), but only if the
  // operation, op, is not logical "and" or "or" as these are used in
  // bitfield operations which may start with uninitialized memory.
  // Is only called if one of the operands is constant.
  void assert_initialized_if_not_and_or(IntBinaryOperator op, VariableRef x) {
    if ((op == IntBinaryOperator::And) || (op == IntBinaryOperator::Or)) {
      return;
    }

    this->_uninitialized.assert_initialized(x);
  }

  void int_apply(IntBinaryOperator op,
                 VariableRef x,
                 VariableRef y,
                 VariableRef z) override {
    ikos_assert(ScalarVariableTrait::is_int(x));
    ikos_assert(ScalarVariableTrait::is_int(y));
    ikos_assert(ScalarVariableTrait::is_int(z));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assert_initialized(y);
    this->_uninitialized.assert_initialized(z);

    if (this->_uninitialized.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_uninitialized.assign_initialized(x);
    this->_integer.apply(op, x, y, z);
  }

  void int_apply(IntBinaryOperator op,
                 VariableRef x,
                 VariableRef y,
                 const MachineInt& z) override {
    ikos_assert(ScalarVariableTrait::is_int(x));
    ikos_assert(ScalarVariableTrait::is_int(y));

    if (this->is_bottom_fast()) {
      return;
    }

    this->assert_initialized_if_not_and_or(op, y);

    if (this->_uninitialized.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_uninitialized.assign_initialized(x);
    this->_integer.apply(op, x, y, z);
  }

  void int_apply(IntBinaryOperator op,
                 VariableRef x,
                 const MachineInt& y,
                 VariableRef z) override {
    ikos_assert(ScalarVariableTrait::is_int(x));
    ikos_assert(ScalarVariableTrait::is_int(z));

    if (this->is_bottom_fast()) {
      return;
    }

    this->assert_initialized_if_not_and_or(op, z);

    if (this->_uninitialized.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_uninitialized.assign_initialized(x);
    this->_integer.apply(op, x, y, z);
  }

  void int_add(IntPredicate pred, VariableRef x, VariableRef y) override {
    ikos_assert(ScalarVariableTrait::is_int(x));
    ikos_assert(ScalarVariableTrait::is_int(y));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assert_initialized(x);
    this->_uninitialized.assert_initialized(y);

    if (this->_uninitialized.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_integer.add(pred, x, y);
  }

  void int_add(IntPredicate pred, VariableRef x, const MachineInt& y) override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assert_initialized(x);

    if (this->_uninitialized.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_integer.add(pred, x, y);
  }

  void int_add(IntPredicate pred, const MachineInt& x, VariableRef y) override {
    ikos_assert(ScalarVariableTrait::is_int(y));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assert_initialized(y);

    if (this->_uninitialized.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_integer.add(pred, x, y);
  }

  void int_set(VariableRef x, const IntInterval& value) override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign_initialized(x);
    this->_integer.set(x, value);
  }

  void int_set(VariableRef x, const IntCongruence& value) override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign_initialized(x);
    this->_integer.set(x, value);
  }

  void int_set(VariableRef x, const IntIntervalCongruence& value) override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign_initialized(x);
    this->_integer.set(x, value);
  }

  void int_refine(VariableRef x, const IntInterval& value) override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    this->_integer.refine(x, value);
  }

  void int_refine(VariableRef x, const IntCongruence& value) override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    this->_integer.refine(x, value);
  }

  void int_refine(VariableRef x, const IntIntervalCongruence& value) override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    this->_integer.refine(x, value);
  }

  void int_forget(VariableRef x) override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.forget(x);
    this->_integer.forget(x);
  }

  IntInterval int_to_interval(VariableRef x) const override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    return this->_integer.to_interval(x);
  }

  IntInterval int_to_interval(const IntLinearExpression& e) const override {
    return this->_integer.to_interval(e);
  }

  IntCongruence int_to_congruence(VariableRef x) const override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    return this->_integer.to_congruence(x);
  }

  IntCongruence int_to_congruence(const IntLinearExpression& e) const override {
    return this->_integer.to_congruence(e);
  }

  IntIntervalCongruence int_to_interval_congruence(
      VariableRef x) const override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    return this->_integer.to_interval_congruence(x);
  }

  IntIntervalCongruence int_to_interval_congruence(
      const IntLinearExpression& e) const override {
    return this->_integer.to_interval_congruence(e);
  }

  /// @}
  /// \name Implement non-negative loop counter abstract domain methods
  /// @{

  void counter_mark(VariableRef x) override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    this->_integer.counter_mark(x);
  }

  void counter_unmark(VariableRef x) override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    this->_integer.counter_unmark(x);
  }

  void counter_init(VariableRef x, const MachineInt& c) override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    this->_integer.counter_init(x, c);
  }

  void counter_incr(VariableRef x, const MachineInt& k) override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    this->_integer.counter_incr(x, k);
  }

  void counter_forget(VariableRef x) override {
    ikos_assert(ScalarVariableTrait::is_int(x));

    this->_integer.counter_forget(x);
  }

  /// @}
  /// \name Implement floating point abstract domain methods
  /// @{

  void float_assign_undef(VariableRef x) override {
    ikos_assert(ScalarVariableTrait::is_float(x));

    this->_uninitialized.assign_uninitialized(x);
  }

  void float_assign_nondet(VariableRef x) override {
    ikos_assert(ScalarVariableTrait::is_float(x));

    this->_uninitialized.assign_initialized(x);
  }

  void float_assign(VariableRef x, VariableRef y) override {
    ikos_assert(ScalarVariableTrait::is_float(x));
    ikos_assert(ScalarVariableTrait::is_float(y));

    this->_uninitialized.assign(x, y);
  }

  void float_forget(VariableRef x) override {
    ikos_assert(ScalarVariableTrait::is_float(x));

    this->_uninitialized.forget(x);
  }

  /// @}
  /// \name Implement nullity abstract domain methods
  /// @{

  void nullity_assert_null(VariableRef p) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assert_initialized(p);
    this->_nullity.assert_null(p);
    this->_points_to_map.refine(p, PointsToSetT::empty());
  }

  void nullity_assert_non_null(VariableRef p) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assert_initialized(p);
    this->_nullity.assert_non_null(p);

    if (this->_points_to_map.get(p).is_empty()) {
      this->set_to_bottom();
    }
  }

  bool nullity_is_null(VariableRef p) const override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    return this->_nullity.is_null(p);
  }

  bool nullity_is_non_null(VariableRef p) const override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    return this->_nullity.is_non_null(p);
  }

  void nullity_set(VariableRef p, Nullity value) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    return this->_nullity.set(p, value);
  }

  void nullity_refine(VariableRef p, Nullity value) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    return this->_nullity.refine(p, value);
  }

  Nullity nullity_to_nullity(VariableRef p) const override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    return this->_nullity.get(p);
  }

  /// @}
  /// \name Implement pointer abstract domain methods
  /// @{

  void pointer_assign(VariableRef p,
                      MemoryLocationRef addr,
                      Nullity nullity) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign_initialized(p);
    this->_nullity.set(p, nullity);
    this->_points_to_map.set(p, PointsToSetT{addr});
    VariableRef offset = ScalarVariableTrait::offset_var(p);
    this->_integer.assign(offset,
                          MachineInt::zero(IntVariableTrait::bit_width(offset),
                                           IntVariableTrait::sign(offset)));
  }

  void pointer_assign_null(VariableRef p) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign_initialized(p);
    this->_nullity.assign_null(p);
    this->_points_to_map.set(p, PointsToSetT::empty());
    VariableRef offset = ScalarVariableTrait::offset_var(p);
    this->_integer.assign(offset,
                          MachineInt::zero(IntVariableTrait::bit_width(offset),
                                           IntVariableTrait::sign(offset)));
  }

  void pointer_assign_undef(VariableRef p) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign_uninitialized(p);
    this->_nullity.forget(p);
    this->_points_to_map.set(p, PointsToSetT::empty());
    this->_integer.forget(ScalarVariableTrait::offset_var(p));
  }

  void pointer_assign_nondet(VariableRef p) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign_initialized(p);
    this->_nullity.forget(p);
    this->_points_to_map.forget(p);
    this->_integer.forget(ScalarVariableTrait::offset_var(p));
  }

  void pointer_assign(VariableRef p, VariableRef q) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));
    ikos_assert(ScalarVariableTrait::is_pointer(q));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign(p, q);
    this->_nullity.assign(p, q);
    this->_points_to_map.set(p, this->_points_to_map.get(q));
    this->_integer.assign(ScalarVariableTrait::offset_var(p),
                          ScalarVariableTrait::offset_var(q));
  }

  void pointer_assign(VariableRef p, VariableRef q, VariableRef o) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));
    ikos_assert(ScalarVariableTrait::is_pointer(q));
    ikos_assert(ScalarVariableTrait::is_int(o));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assert_initialized(q);
    this->_uninitialized.assert_initialized(o);

    if (this->_uninitialized.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_uninitialized.assign_initialized(p);
    this->_nullity.assign(p, q);
    this->_points_to_map.set(p, this->_points_to_map.get(q));
    this->_integer.apply(IntBinaryOperator::Add,
                         ScalarVariableTrait::offset_var(p),
                         ScalarVariableTrait::offset_var(q),
                         o);
  }

  void pointer_assign(VariableRef p,
                      VariableRef q,
                      const MachineInt& o) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));
    ikos_assert(ScalarVariableTrait::is_pointer(q));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assert_initialized(q);

    if (this->_uninitialized.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_uninitialized.assign_initialized(p);
    this->_nullity.assign(p, q);
    this->_points_to_map.set(p, this->_points_to_map.get(q));
    this->_integer.apply(IntBinaryOperator::Add,
                         ScalarVariableTrait::offset_var(p),
                         ScalarVariableTrait::offset_var(q),
                         o);
  }

  void pointer_assign(VariableRef p,
                      VariableRef q,
                      const IntLinearExpression& o) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));
    ikos_assert(ScalarVariableTrait::is_pointer(q));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assert_initialized(q);
    for (const auto& term : o) {
      ikos_assert(ScalarVariableTrait::is_int(term.first));
      this->_uninitialized.assert_initialized(term.first);
    }

    if (this->_uninitialized.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_uninitialized.assign_initialized(p);
    this->_nullity.assign(p, q);
    this->_points_to_map.set(p, this->_points_to_map.get(q));
    VariableRef offset_p = ScalarVariableTrait::offset_var(p);
    VariableRef offset_q = ScalarVariableTrait::offset_var(q);
    auto one = MachineInt(1,
                          IntVariableTrait::bit_width(offset_p),
                          IntVariableTrait::sign(offset_p));
    IntLinearExpression offset(o);
    offset.add(one, offset_q);
    this->_integer.assign(offset_p, offset);
  }

  void pointer_add(PointerPredicate pred,
                   VariableRef p,
                   VariableRef q) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));
    ikos_assert(ScalarVariableTrait::is_pointer(q));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assert_initialized(p);
    this->_uninitialized.assert_initialized(q);

    if (this->_uninitialized.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_nullity.add(pred, p, q);

    if (this->_nullity.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    PointsToSetT addrs_p = this->_points_to_map.get(p);
    PointsToSetT addrs_q = this->_points_to_map.get(q);
    Nullity nullity_p = this->_nullity.get(p);
    Nullity nullity_q = this->_nullity.get(q);

    switch (pred) {
      case PointerPredicate::EQ: {
        // p == q
        PointsToSetT addrs_pq = addrs_p.meet(addrs_q);

        if (addrs_pq.is_bottom() ||
            (addrs_pq.is_empty() && nullity_p.is_non_null())) {
          this->set_to_bottom();
          return;
        }

        // p and q's points-to sets
        this->_points_to_map.set(p, addrs_pq);
        this->_points_to_map.set(q, addrs_pq);

        // p and q's offsets
        this->_integer.add(IntPredicate::EQ,
                           ScalarVariableTrait::offset_var(p),
                           ScalarVariableTrait::offset_var(q));
      } break;
      case PointerPredicate::NE: {
        // p != q
        if (nullity_p.is_non_null() && nullity_q.is_non_null() &&
            addrs_p.singleton() && addrs_p == addrs_q) {
          // p and q's offsets
          this->_integer.add(IntPredicate::NE,
                             ScalarVariableTrait::offset_var(p),
                             ScalarVariableTrait::offset_var(q));
        }
      } break;
      case PointerPredicate::GT: {
        // p > q
        if (nullity_p.is_non_null() && nullity_q.is_non_null() &&
            addrs_p.singleton() && addrs_p == addrs_q) {
          // p and q's offsets
          this->_integer.add(IntPredicate::GT,
                             ScalarVariableTrait::offset_var(p),
                             ScalarVariableTrait::offset_var(q));
        }
      } break;
      case PointerPredicate::GE: {
        // p >= q
        if (nullity_p.is_non_null() && nullity_q.is_non_null() &&
            addrs_p.singleton() && addrs_p == addrs_q) {
          // p and q's offsets
          this->_integer.add(IntPredicate::GE,
                             ScalarVariableTrait::offset_var(p),
                             ScalarVariableTrait::offset_var(q));
        }
      } break;
      case PointerPredicate::LT: {
        // p < q
        if (nullity_p.is_non_null() && nullity_q.is_non_null() &&
            addrs_p.singleton() && addrs_p == addrs_q) {
          // p and q's offsets
          this->_integer.add(IntPredicate::LT,
                             ScalarVariableTrait::offset_var(p),
                             ScalarVariableTrait::offset_var(q));
        }
      } break;
      case PointerPredicate::LE: {
        // p <= q
        if (nullity_p.is_non_null() && nullity_q.is_non_null() &&
            addrs_p.singleton() && addrs_p == addrs_q) {
          // p and q's offsets
          this->_integer.add(IntPredicate::LE,
                             ScalarVariableTrait::offset_var(p),
                             ScalarVariableTrait::offset_var(q));
        }
      } break;
    }
  }

  void pointer_refine(VariableRef p, const PointsToSetT& addrs) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    this->_points_to_map.refine(p, addrs);
  }

  void pointer_refine(VariableRef p,
                      const PointsToSetT& addrs,
                      const IntInterval& offset) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_points_to_map.refine(p, addrs);
    this->_integer.refine(ScalarVariableTrait::offset_var(p), offset);
  }

  void pointer_refine(VariableRef p, const PointerAbsValueT& value) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    if (this->is_bottom_fast()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else if (value.is_uninitialized()) {
      this->_uninitialized.refine(p, Uninitialized::uninitialized());
      this->_points_to_map.refine(p, PointsToSetT::empty());
    } else if (value.is_null()) {
      this->_uninitialized.refine(p, Uninitialized::initialized());
      this->_nullity.refine(p, Nullity::null());
      this->_points_to_map.refine(p, PointsToSetT::empty());
    } else {
      this->_uninitialized.refine(p, value.uninitialized());
      this->_nullity.refine(p, value.nullity());
      this->_points_to_map.refine(p, value.points_to());
      this->_integer.refine(ScalarVariableTrait::offset_var(p), value.offset());
    }
  }

  void pointer_refine(VariableRef p, const PointerSetT& set) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    if (this->is_bottom_fast()) {
      return;
    } else if (set.is_bottom()) {
      this->set_to_bottom();
    } else if (set.points_to().is_empty()) {
      // The pointer set only contains null and uninitialized pointers
      this->_points_to_map.refine(p, PointsToSetT::empty());
    } else {
      this->_points_to_map.refine(p, set.points_to());
      this->_integer.refine(ScalarVariableTrait::offset_var(p), set.offsets());
    }
  }

  void pointer_offset_to_int(VariableRef x, VariableRef p) override {
    ikos_assert(ScalarVariableTrait::is_int(x));
    ikos_assert(ScalarVariableTrait::is_pointer(p));
    ikos_assert(
        IntVariableTrait::bit_width(x) ==
        IntVariableTrait::bit_width(ScalarVariableTrait::offset_var(p)));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assert_initialized(p);

    if (this->_uninitialized.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    VariableRef offset = ScalarVariableTrait::offset_var(p);

    this->_uninitialized.assign_initialized(x);
    if (x == offset) {
      return; // No-op
    } else if (IntVariableTrait::sign(x) == IntVariableTrait::sign(offset)) {
      this->_integer.assign(x, offset);
    } else {
      this->_integer.apply(IntUnaryOperator::SignCast, x, offset);
    }
  }

  IntInterval pointer_offset_to_interval(VariableRef p) const override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    return this->_integer.to_interval(ScalarVariableTrait::offset_var(p));
  }

  IntCongruence pointer_offset_to_congruence(VariableRef p) const override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    return this->_integer.to_congruence(ScalarVariableTrait::offset_var(p));
  }

  IntIntervalCongruence pointer_offset_to_interval_congruence(
      VariableRef p) const override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    return this->_integer.to_interval_congruence(
        ScalarVariableTrait::offset_var(p));
  }

  PointsToSetT pointer_to_points_to(VariableRef p) const override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    return this->_points_to_map.get(p);
  }

  PointerAbsValueT pointer_to_pointer(VariableRef p) const override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    return PointerAbsValueT(this->_uninitialized.get(p),
                            this->_nullity.get(p),
                            this->_points_to_map.get(p),
                            this->_integer.to_interval(
                                ScalarVariableTrait::offset_var(p)));
  }

  void pointer_forget_offset(VariableRef p) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    this->_integer.forget(ScalarVariableTrait::offset_var(p));
  }

  void pointer_forget(VariableRef p) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.forget(p);
    this->_nullity.forget(p);
    this->_points_to_map.forget(p);
    this->_integer.forget(ScalarVariableTrait::offset_var(p));
  }

  /// @}
  /// \name Implement dynamically typed variables abstract domain methods
  /// @{

  void dynamic_assign(VariableRef x, VariableRef y) override {
    ikos_assert(ScalarVariableTrait::is_dynamic(x));
    ikos_assert(ScalarVariableTrait::is_dynamic(y));
    ikos_assert(IntVariableTrait::bit_width(x) ==
                IntVariableTrait::bit_width(y));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign(x, y);
    if (IntVariableTrait::sign(x) == IntVariableTrait::sign(y)) {
      this->_integer.assign(x, y);
    } else {
      this->_integer.apply(IntUnaryOperator::SignCast, x, y);
    }
    this->_nullity.assign(x, y);
    this->_points_to_map.set(x, this->_points_to_map.get(y));
    this->_integer.assign(ScalarVariableTrait::offset_var(x),
                          ScalarVariableTrait::offset_var(y));
  }

  void dynamic_write_undef(VariableRef x) override {
    ikos_assert(ScalarVariableTrait::is_dynamic(x));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign_uninitialized(x);
    this->_integer.forget(x);
    this->_nullity.forget(x);
    this->_points_to_map.forget(x);
    this->_integer.forget(ScalarVariableTrait::offset_var(x));
  }

  void dynamic_write_nondet(VariableRef x) override {
    ikos_assert(ScalarVariableTrait::is_dynamic(x));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign_initialized(x);
    this->_integer.forget(x);
    this->_nullity.forget(x);
    this->_points_to_map.forget(x);
    this->_integer.forget(ScalarVariableTrait::offset_var(x));
  }

  void dynamic_write_int(VariableRef x, const MachineInt& n) override {
    ikos_assert(ScalarVariableTrait::is_dynamic(x));
    ikos_assert(IntVariableTrait::bit_width(x) == n.bit_width());

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign_initialized(x);
    if (IntVariableTrait::sign(x) == n.sign()) {
      this->_integer.assign(x, n);
    } else {
      this->_integer.assign(x, n.sign_cast(IntVariableTrait::sign(x)));
    }
    this->_nullity.forget(x);
    this->_points_to_map.forget(x);
    this->_integer.forget(ScalarVariableTrait::offset_var(x));
  }

  void dynamic_write_nondet_int(VariableRef x) override {
    ikos_assert(ScalarVariableTrait::is_dynamic(x));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign_initialized(x);
    this->_integer.forget(x);
    this->_nullity.forget(x);
    this->_points_to_map.forget(x);
    this->_integer.forget(ScalarVariableTrait::offset_var(x));
  }

  void dynamic_write_int(VariableRef x, VariableRef y) override {
    ikos_assert(ScalarVariableTrait::is_dynamic(x));
    ikos_assert(ScalarVariableTrait::is_int(y));
    ikos_assert(IntVariableTrait::bit_width(x) ==
                IntVariableTrait::bit_width(y));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign(x, y);
    if (IntVariableTrait::sign(x) == IntVariableTrait::sign(y)) {
      this->_integer.assign(x, y);
    } else {
      this->_integer.apply(IntUnaryOperator::SignCast, x, y);
    }
    this->_nullity.forget(x);
    this->_points_to_map.forget(x);
    this->_integer.forget(ScalarVariableTrait::offset_var(x));
  }

  void dynamic_write_nondet_float(VariableRef x) override {
    ikos_assert(ScalarVariableTrait::is_dynamic(x));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign_initialized(x);
    this->_integer.forget(x);
    this->_nullity.forget(x);
    this->_points_to_map.forget(x);
    this->_integer.forget(ScalarVariableTrait::offset_var(x));
  }

  void dynamic_write_null(VariableRef x) override {
    ikos_assert(ScalarVariableTrait::is_dynamic(x));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign_initialized(x);
    this->_integer.forget(x);
    this->_nullity.assign_null(x);
    this->_points_to_map.set(x, PointsToSetT::empty());
    VariableRef offset = ScalarVariableTrait::offset_var(x);
    this->_integer.assign(offset,
                          MachineInt::zero(IntVariableTrait::bit_width(offset),
                                           IntVariableTrait::sign(offset)));
  }

  void dynamic_write_pointer(VariableRef x,
                             MemoryLocationRef addr,
                             Nullity nullity) override {
    ikos_assert(ScalarVariableTrait::is_dynamic(x));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign_initialized(x);
    this->_integer.forget(x);
    this->_nullity.set(x, nullity);
    this->_points_to_map.set(x, PointsToSetT{addr});
    VariableRef offset = ScalarVariableTrait::offset_var(x);
    this->_integer.assign(offset,
                          MachineInt::zero(IntVariableTrait::bit_width(offset),
                                           IntVariableTrait::sign(offset)));
  }

  void dynamic_write_pointer(VariableRef x, VariableRef y) override {
    ikos_assert(ScalarVariableTrait::is_dynamic(x));
    ikos_assert(ScalarVariableTrait::is_pointer(y));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign(x, y);
    this->_integer.forget(x);
    this->_nullity.assign(x, y);
    this->_points_to_map.set(x, this->_points_to_map.get(y));
    this->_integer.assign(ScalarVariableTrait::offset_var(x),
                          ScalarVariableTrait::offset_var(y));
  }

  void dynamic_read_int(VariableRef x, VariableRef y) override {
    ikos_assert(ScalarVariableTrait::is_int(x));
    ikos_assert(ScalarVariableTrait::is_dynamic(y));
    ikos_assert(IntVariableTrait::bit_width(x) ==
                IntVariableTrait::bit_width(y));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign(x, y);
    if (IntVariableTrait::sign(x) == IntVariableTrait::sign(y)) {
      this->_integer.assign(x, y);
    } else {
      this->_integer.apply(IntUnaryOperator::SignCast, x, y);
    }
  }

  void dynamic_read_pointer(VariableRef x, VariableRef y) override {
    ikos_assert(ScalarVariableTrait::is_pointer(x));
    ikos_assert(ScalarVariableTrait::is_dynamic(y));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assign(x, y);
    this->_nullity.assign(x, y);
    this->_points_to_map.set(x, this->_points_to_map.get(y));
    this->_integer.assign(ScalarVariableTrait::offset_var(x),
                          ScalarVariableTrait::offset_var(y));
  }

  bool dynamic_is_zero(VariableRef x) const override {
    ikos_assert(ScalarVariableTrait::is_dynamic(x));

    IntInterval value = this->_integer.to_interval(x);
    return value.is_bottom() || value.is_zero();
  }

  bool dynamic_is_null(VariableRef x) const override {
    ikos_assert(ScalarVariableTrait::is_dynamic(x));

    return this->_nullity.is_null(x);
  }

  void dynamic_forget(VariableRef x) override {
    ikos_assert(ScalarVariableTrait::is_dynamic(x));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.forget(x);
    this->_integer.forget(x);
    this->_nullity.forget(x);
    this->_points_to_map.forget(x);
    this->_integer.forget(ScalarVariableTrait::offset_var(x));
  }

  /// @}
  /// \name Implement scalar abstract domain methods
  /// @{

  void scalar_assign_undef(VariableRef x) override {
    if (this->is_bottom_fast()) {
      return;
    } else if (ScalarVariableTrait::is_int(x)) {
      this->int_assign_undef(x);
    } else if (ScalarVariableTrait::is_float(x)) {
      this->float_assign_undef(x);
    } else if (ScalarVariableTrait::is_pointer(x)) {
      this->pointer_assign_undef(x);
    } else if (ScalarVariableTrait::is_dynamic(x)) {
      this->dynamic_write_undef(x);
    } else {
      ikos_unreachable("unexpected type");
    }
  }

  void scalar_assign_nondet(VariableRef x) override {
    if (this->is_bottom_fast()) {
      return;
    } else if (ScalarVariableTrait::is_int(x)) {
      this->int_assign_nondet(x);
    } else if (ScalarVariableTrait::is_float(x)) {
      this->float_assign_nondet(x);
    } else if (ScalarVariableTrait::is_pointer(x)) {
      this->pointer_assign_nondet(x);
    } else if (ScalarVariableTrait::is_dynamic(x)) {
      this->dynamic_write_nondet(x);
    } else {
      ikos_unreachable("unexpected type");
    }
  }

  void scalar_pointer_to_int(VariableRef x,
                             VariableRef p,
                             MemoryLocationRef absolute_zero) override {
    ikos_assert(ScalarVariableTrait::is_int(x));
    ikos_assert(ScalarVariableTrait::is_pointer(p));
    ikos_assert(
        IntVariableTrait::bit_width(x) ==
        IntVariableTrait::bit_width(ScalarVariableTrait::offset_var(p)));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assert_initialized(p);

    this->normalize();

    if (this->is_bottom()) {
      return;
    }

    this->_uninitialized.assign_initialized(x);
    if (this->_nullity.is_null(p)) {
      auto zero = MachineInt::zero(IntVariableTrait::bit_width(x),
                                   IntVariableTrait::sign(x));
      this->_integer.assign(x, zero);
    } else if (this->_points_to_map.get(p) == PointsToSetT{absolute_zero}) {
      VariableRef offset = ScalarVariableTrait::offset_var(p);
      if (IntVariableTrait::sign(x) == IntVariableTrait::sign(offset)) {
        this->_integer.assign(x, offset);
      } else {
        this->_integer.apply(IntUnaryOperator::SignCast, x, offset);
      }
    } else {
      this->_integer.forget(x);
    }
  }

  void scalar_int_to_pointer(VariableRef p,
                             VariableRef x,
                             MemoryLocationRef absolute_zero) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));
    ikos_assert(ScalarVariableTrait::is_int(x));
    ikos_assert(
        IntVariableTrait::bit_width(x) ==
        IntVariableTrait::bit_width(ScalarVariableTrait::offset_var(p)));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_uninitialized.assert_initialized(x);

    this->normalize();

    if (this->is_bottom()) {
      return;
    }

    IntIntervalCongruence value = this->_integer.to_interval_congruence(x);
    auto zero = MachineInt::zero(IntVariableTrait::bit_width(x),
                                 IntVariableTrait::sign(x));
    auto nullity = Nullity::top();
    if (value.contains(zero)) {
      if (value.singleton()) {
        nullity = Nullity::null();
      } else {
        nullity = Nullity::top();
      }
    } else {
      nullity = Nullity::non_null();
    }

    this->_uninitialized.assign_initialized(p);
    this->_nullity.set(p, nullity);
    this->_points_to_map.set(p, PointsToSetT{absolute_zero});
    VariableRef offset = ScalarVariableTrait::offset_var(p);
    if (IntVariableTrait::sign(offset) == IntVariableTrait::sign(x)) {
      this->_integer.assign(offset, x);
    } else {
      this->_integer.apply(IntUnaryOperator::SignCast, offset, x);
    }
  }

  void scalar_forget(VariableRef x) override {
    if (this->is_bottom_fast()) {
      return;
    } else if (ScalarVariableTrait::is_int(x)) {
      this->int_forget(x);
    } else if (ScalarVariableTrait::is_float(x)) {
      this->float_forget(x);
    } else if (ScalarVariableTrait::is_pointer(x)) {
      this->pointer_forget(x);
    } else if (ScalarVariableTrait::is_dynamic(x)) {
      this->dynamic_forget(x);
    } else {
      ikos_unreachable("unexpected type");
    }
  }

  /// @}

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else {
      o << "(";
      this->_uninitialized.dump(o);
      o << ", ";
      this->_integer.dump(o);
      o << ", ";
      this->_nullity.dump(o);
      o << ", ";
      this->_points_to_map.dump(o);
      o << ")";
    }
  }

  static std::string name() {
    return "composite domain using " + UninitializedDomain::name() + ", " +
           MachineIntDomain::name() + " and " + NullityDomain::name();
  }

}; // end class CompositeDomain

} // end namespace scalar
} // end namespace core
} // end namespace ikos
