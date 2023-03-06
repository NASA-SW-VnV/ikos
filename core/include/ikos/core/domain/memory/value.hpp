/*******************************************************************************
 *
 * \file
 * \brief A value abstract domain 'a la' Mine parameterized by a numerical
 * abstract domain.
 *
 * Based on the paper "Field-Sensitive Value Analysis of Embedded C
 * Programs with Union Types and Pointer Arithmetics" by A. Mine
 * (LCTES'06)
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *               Clement Decoodt
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

#include <type_traits>

#include <ikos/core/domain/lifetime/abstract_domain.hpp>
#include <ikos/core/domain/memory/abstract_domain.hpp>
#include <ikos/core/domain/memory/value/cell_set.hpp>
#include <ikos/core/domain/memory/value/mem_loc_to_cell_set.hpp>
#include <ikos/core/domain/memory/value/mem_loc_to_pointer_set.hpp>
#include <ikos/core/semantic/machine_int/variable.hpp>
#include <ikos/core/semantic/memory/value/cell_factory.hpp>
#include <ikos/core/semantic/memory/value/cell_variable.hpp>
#include <ikos/core/support/cast.hpp>

namespace ikos {
namespace core {
namespace memory {

/// \brief Value abstract domain
///
/// Memory abstraction consisting of cells and a scalar abstract domain.
///
/// This domain abstracts memory into a set of memory cells with
/// integer, float or pointer type following Mine's paper. If a cell is
/// of type integer this domain can model its value and whether it is
/// initialized or not. If the cell is of type float, the domain keeps
/// track of whether it is initialized or not. If the cell is of pointer
/// type this domain keeps track of its address, its offset, whether it is
/// null or not and whether it is initialized or not.
///
/// A memory cell is a triple `(base, offset, size)` modelling all bytes at
/// address `base`, starting at offset `offset` up to `offset + size - 1`. A
/// memory cell is represented by a variable implementing
/// `memory::CellVariableTraits`. The variable has a dynamic type. It is
/// either an integer of 8*size bits, a floating point of 8*size bits or a
/// pointer.
///
/// Note that offset variables should be unsigned.
template < typename VariableRef,
           typename MemoryLocationRef,
           typename CellFactoryRef,
           typename ScalarDomain,
           typename LifetimeDomain >
class ValueDomain final
    : public memory::AbstractDomain< VariableRef,
                                     MemoryLocationRef,
                                     ValueDomain< VariableRef,
                                                  MemoryLocationRef,
                                                  CellFactoryRef,
                                                  ScalarDomain,
                                                  LifetimeDomain > > {
public:
  static_assert(memory::IsCellVariable< VariableRef, MemoryLocationRef >::value,
                "VariableRef must implement memory::CellVariableTraits");
  static_assert(memory::IsCellFactory< VariableRef,
                                       MemoryLocationRef,
                                       CellFactoryRef >::value,
                "CellFactoryRef must implement memory::CellFactoryTraits");
  static_assert(scalar::IsAbstractDomain< ScalarDomain,
                                          VariableRef,
                                          MemoryLocationRef >::value,
                "ScalarDomain must implement scalar::AbstractDomain");
  static_assert(
      lifetime::IsAbstractDomain< LifetimeDomain, MemoryLocationRef >::value,
      "LifetimeDomain must implement lifetime::AbstractDomain");

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
  using CellSetT = CellSet< VariableRef >;
  using MemLocToCellSetT = MemLocToCellSet< MemoryLocationRef, VariableRef >;
  using MemLocToPointerSetT = MemLocToPointerSet< MemoryLocationRef >;
  using MachIntVariableTrait = machine_int::VariableTraits< VariableRef >;
  using ScalarVariableTrait = scalar::VariableTraits< VariableRef >;
  using CellVariableTrait =
      memory::CellVariableTraits< VariableRef, MemoryLocationRef >;
  using CellFactoryTrait = memory::
      CellFactoryTraits< VariableRef, MemoryLocationRef, CellFactoryRef >;

private:
  /// \brief Cell factory
  CellFactoryRef _cell_factory;

  /// \brief Underlying scalar domain
  ScalarDomain _scalar;

  /// \brief Map from memory location to set of cells
  MemLocToCellSetT _cells;

  /// \brief Map from memory location to set of pointers
  MemLocToPointerSetT _pointer_sets;

  /// \brief Underlying lifetime abstract domain
  LifetimeDomain _lifetime;

private:
  /// \brief Constructor
  ValueDomain(CellFactoryRef cell_factory,
              ScalarDomain scalar,
              MemLocToCellSetT cells,
              MemLocToPointerSetT pointer_sets,
              LifetimeDomain lifetime)
      : _cell_factory(std::move(cell_factory)),
        _scalar(std::move(scalar)),
        _cells(std::move(cells)),
        _pointer_sets(std::move(pointer_sets)),
        _lifetime(std::move(lifetime)) {
    this->normalize();
  }

public:
  /// \brief Create an abstract value with the given underlying abstract values
  ///
  /// \param cell_factory The cell factory
  /// \param scalar The scalar abstract value
  /// \param lifetime The lifetime abstract value
  ValueDomain(CellFactoryRef cell_factory,
              ScalarDomain scalar,
              LifetimeDomain lifetime)
      : _cell_factory(std::move(cell_factory)),
        _scalar(std::move(scalar)),
        _cells(MemLocToCellSetT::top()),
        _pointer_sets(MemLocToPointerSetT::top()),
        _lifetime(std::move(lifetime)) {
    this->normalize();
  }

  /// \brief Copy constructor
  ValueDomain(const ValueDomain&) noexcept(
      (std::is_nothrow_copy_constructible< ScalarDomain >::value) &&
      (std::is_nothrow_copy_constructible< LifetimeDomain >::value)) = default;

  /// \brief Move constructor
  ValueDomain(ValueDomain&&) noexcept(
      (std::is_nothrow_move_constructible< ScalarDomain >::value) &&
      (std::is_nothrow_move_constructible< LifetimeDomain >::value)) = default;

  /// \brief Copy assignment operator
  ValueDomain& operator=(const ValueDomain&) noexcept(
      (std::is_nothrow_copy_assignable< ScalarDomain >::value) &&
      (std::is_nothrow_copy_assignable< LifetimeDomain >::value)) = default;

  /// \brief Move assignment operator
  ValueDomain& operator=(ValueDomain&&) noexcept(
      (std::is_nothrow_move_assignable< ScalarDomain >::value) &&
      (std::is_nothrow_move_assignable< LifetimeDomain >::value)) = default;

  /// \brief Destructor
  ~ValueDomain() override = default;

  /// \name Implement core abstract domain methods
  /// @{

  void normalize() override {
    this->_lifetime.normalize();
    if (this->_lifetime.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_pointer_sets.normalize();
    if (this->_pointer_sets.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_cells.normalize();
    if (this->_cells.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_scalar.normalize();
    if (this->_scalar.is_bottom()) {
      this->set_to_bottom();
      return;
    }
  }

private:
  /// \brief Return true if the abstract value is bottom
  ///
  /// This is not always correct since it doesn't check this->_scalar
  bool is_bottom_fast() const { return this->_cells.is_bottom(); }

public:
  bool is_bottom() const override {
    return this->_lifetime.is_bottom() || this->_pointer_sets.is_bottom() ||
           this->_cells.is_bottom() || this->_scalar.is_bottom();
  }

  bool is_top() const override {
    return this->_lifetime.is_top() && this->_pointer_sets.is_top() &&
           this->_cells.is_top() && this->_scalar.is_top();
  }

  void set_to_bottom() override {
    this->_scalar.set_to_bottom();
    this->_cells.set_to_bottom();
    this->_pointer_sets.set_to_bottom();
    this->_lifetime.set_to_bottom();
  }

  void set_to_top() override {
    this->_scalar.set_to_top();
    this->_cells.set_to_top();
    this->_pointer_sets.set_to_top();
    this->_lifetime.set_to_top();
  }

  bool leq(const ValueDomain& other) const override {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_scalar.leq(other._scalar) &&
             this->_cells.leq(other._cells) &&
             this->_pointer_sets.leq(other._pointer_sets) &&
             this->_lifetime.leq(other._lifetime);
    }
  }

  bool equals(const ValueDomain& other) const override {
    if (this->is_bottom()) {
      return other.is_bottom();
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_scalar.equals(other._scalar) &&
             this->_cells.equals(other._cells) &&
             this->_pointer_sets.equals(other._pointer_sets) &&
             this->_lifetime.equals(other._lifetime);
    }
  }

  void join_with(ValueDomain&& other) override {
    this->normalize();
    other.normalize();
    if (this->is_bottom()) {
      this->operator=(std::move(other));
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_scalar.join_with(std::move(other._scalar));
      this->_cells.join_with(std::move(other._cells));
      this->_pointer_sets.join_with(std::move(other._pointer_sets));
      this->_lifetime.join_with(std::move(other._lifetime));
    }
  }

  void join_with(const ValueDomain& other) override {
    this->normalize();
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_scalar.join_with(other._scalar);
      this->_cells.join_with(other._cells);
      this->_pointer_sets.join_with(other._pointer_sets);
      this->_lifetime.join_with(other._lifetime);
    }
  }

  void join_loop_with(ValueDomain&& other) override {
    this->normalize();
    other.normalize();
    if (this->is_bottom()) {
      this->operator=(std::move(other));
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_scalar.join_loop_with(std::move(other._scalar));
      this->_cells.join_loop_with(std::move(other._cells));
      this->_pointer_sets.join_loop_with(std::move(other._pointer_sets));
      this->_lifetime.join_loop_with(std::move(other._lifetime));
    }
  }

  void join_loop_with(const ValueDomain& other) override {
    this->normalize();
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_scalar.join_loop_with(other._scalar);
      this->_cells.join_loop_with(other._cells);
      this->_pointer_sets.join_loop_with(other._pointer_sets);
      this->_lifetime.join_loop_with(other._lifetime);
    }
  }

  void join_iter_with(ValueDomain&& other) override {
    this->normalize();
    other.normalize();
    if (this->is_bottom()) {
      this->operator=(std::move(other));
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_scalar.join_iter_with(std::move(other._scalar));
      this->_cells.join_iter_with(std::move(other._cells));
      this->_pointer_sets.join_iter_with(std::move(other._pointer_sets));
      this->_lifetime.join_iter_with(std::move(other._lifetime));
    }
  }

  void join_iter_with(const ValueDomain& other) override {
    this->normalize();
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_scalar.join_iter_with(other._scalar);
      this->_cells.join_iter_with(other._cells);
      this->_pointer_sets.join_iter_with(other._pointer_sets);
      this->_lifetime.join_iter_with(other._lifetime);
    }
  }

  void widen_with(const ValueDomain& other) override {
    this->normalize();
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_scalar.widen_with(other._scalar);
      this->_cells.widen_with(other._cells);
      this->_pointer_sets.widen_with(other._pointer_sets);
      this->_lifetime.widen_with(other._lifetime);
    }
  }

  void widen_threshold_with(const ValueDomain& other,
                            const MachineInt& threshold) override {
    this->normalize();
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_scalar.widen_threshold_with(other._scalar, threshold);
      this->_cells.widen_with(other._cells);
      this->_pointer_sets.join_with(other._pointer_sets);
      this->_lifetime.widen_with(other._lifetime);
    }
  }

  void meet_with(const ValueDomain& other) override {
    this->normalize();
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_scalar.meet_with(other._scalar);
      this->_cells.meet_with(other._cells);
      this->_pointer_sets.meet_with(other._pointer_sets);
      this->_lifetime.meet_with(other._lifetime);
    }
  }

  void narrow_with(const ValueDomain& other) override {
    this->normalize();
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_scalar.narrow_with(other._scalar);
      this->_cells.narrow_with(other._cells);
      this->_pointer_sets.narrow_with(other._pointer_sets);
      this->_lifetime.narrow_with(other._lifetime);
    }
  }

  void narrow_threshold_with(const ValueDomain& other,
                             const MachineInt& threshold) override {
    this->normalize();
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_scalar.narrow_threshold_with(other._scalar, threshold);
      this->_cells.narrow_with(other._cells);
      this->_pointer_sets.narrow_with(other._pointer_sets);
      this->_lifetime.narrow_with(other._lifetime);
    }
  }

  ValueDomain join(const ValueDomain& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return ValueDomain(this->_cell_factory,
                         this->_scalar.join(other._scalar),
                         this->_cells.join(other._cells),
                         this->_pointer_sets.join(other._pointer_sets),
                         this->_lifetime.join(other._lifetime));
    }
  }

  ValueDomain join_loop(const ValueDomain& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return ValueDomain(this->_cell_factory,
                         this->_scalar.join_loop(other._scalar),
                         this->_cells.join_loop(other._cells),
                         this->_pointer_sets.join_loop(other._pointer_sets),
                         this->_lifetime.join_loop(other._lifetime));
    }
  }

  ValueDomain join_iter(const ValueDomain& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return ValueDomain(this->_cell_factory,
                         this->_scalar.join_iter(other._scalar),
                         this->_cells.join_iter(other._cells),
                         this->_pointer_sets.join_iter(other._pointer_sets),
                         this->_lifetime.join_iter(other._lifetime));
    }
  }

  ValueDomain widening(const ValueDomain& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return ValueDomain(this->_cell_factory,
                         this->_scalar.widening(other._scalar),
                         this->_cells.widening(other._cells),
                         this->_pointer_sets.widening(other._pointer_sets),
                         this->_lifetime.widening(other._lifetime));
    }
  }

  ValueDomain widening_threshold(const ValueDomain& other,
                                 const MachineInt& threshold) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return ValueDomain(this->_cell_factory,
                         this->_scalar.widening_threshold(other._scalar,
                                                          threshold),
                         this->_cells.widening(other._cells),
                         this->_pointer_sets.join(other._pointer_sets),
                         this->_lifetime.widening(other._lifetime));
    }
  }

  ValueDomain meet(const ValueDomain& other) const override {
    if (this->is_bottom()) {
      return *this;
    } else if (other.is_bottom()) {
      return other;
    } else {
      return ValueDomain(this->_cell_factory,
                         this->_scalar.meet(other._scalar),
                         this->_cells.meet(other._cells),
                         this->_pointer_sets.meet(other._pointer_sets),
                         this->_lifetime.meet(other._lifetime));
    }
  }

  ValueDomain narrowing(const ValueDomain& other) const override {
    if (this->is_bottom()) {
      return *this;
    } else if (other.is_bottom()) {
      return other;
    } else {
      return ValueDomain(this->_cell_factory,
                         this->_scalar.narrowing(other._scalar),
                         this->_cells.narrowing(other._cells),
                         this->_pointer_sets.narrowing(other._pointer_sets),
                         this->_lifetime.narrowing(other._lifetime));
    }
  }

  ValueDomain narrowing_threshold(const ValueDomain& other,
                                  const MachineInt& threshold) const override {
    if (this->is_bottom()) {
      return *this;
    } else if (other.is_bottom()) {
      return other;
    } else {
      return ValueDomain(this->_cell_factory,
                         this->_scalar.narrowing_threshold(other._scalar,
                                                           threshold),
                         this->_cells.narrowing(other._cells),
                         this->_pointer_sets.narrowing(other._pointer_sets),
                         this->_lifetime.narrowing(other._lifetime));
    }
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

private:
  /// \brief Preferred signedness for a cell created from a literal
  ///
  /// For machine integers, return the integer sign
  /// For floating points, return Signed
  /// For pointers, return Unsigned
  class LiteralPreferredSignedness
      : public LiteralT::template Visitor< Signedness > {
  public:
    LiteralPreferredSignedness() = default;

    Signedness machine_int(const MachineInt& i) const { return i.sign(); }

    Signedness floating_point(const DummyNumber&) const { return Signed; }

    Signedness memory_location(MemoryLocationRef) const { return Unsigned; }

    Signedness null() const { return Unsigned; }

    Signedness undefined() const { return Signed; }

    Signedness machine_int_var(VariableRef var) const {
      return MachIntVariableTrait::sign(var);
    }

    Signedness floating_point_var(VariableRef) const { return Signed; }

    Signedness pointer_var(VariableRef) const { return Unsigned; }

  }; // end class LiteralPreferredSignedness

  /// \brief Return the preferred signedness for a cell creating from the given
  /// literal
  Signedness preferred_cell_sign(const LiteralT& lit) const {
    return lit.apply_visitor(LiteralPreferredSignedness());
  }

  /// \brief Get or create the cell with the given base address, offset and size
  ///
  /// If a new cell is created, it will have the given signedness
  VariableRef make_cell(MemoryLocationRef base,
                        const MachineInt& offset,
                        const MachineInt& size,
                        Signedness sign) {
    ikos_assert(offset.sign() == Unsigned);
    ikos_assert(size.sign() == Unsigned);
    ikos_assert(size.is_strictly_positive());
    VariableRef c =
        CellFactoryTrait::cell(this->_cell_factory, base, offset, size, sign);
    ikos_assert(CellVariableTrait::is_cell(c));
    ikos_assert(MachIntVariableTrait::bit_width(c) ==
                size.to< uint64_t >() * 8);
    return c;
  }

  /// \brief Return the byte range for a given cell
  IntInterval cell_range(VariableRef cell) const {
    const MachineInt& offset = CellVariableTrait::offset(cell);
    const MachineInt& size = CellVariableTrait::size(cell);
    auto one = MachineInt(1, offset.bit_width(), Unsigned);
    return IntInterval(offset, offset + (size - one));
  }

  /// \brief Return true if the given cell overlaps with the given byte range
  bool cell_overlap(VariableRef cell, const IntInterval& range) const {
    IntInterval meet = this->cell_range(cell).meet(range);
    return !meet.is_bottom();
  }

  /// \brief Return true if the given cells overlap
  bool cell_overlap(VariableRef a, VariableRef b) const {
    return this->cell_overlap(a, this->cell_range(b));
  }

  /// \brief Return true if the memory write at `offset` of size `size`
  /// can update the given cell. Return false if the number of overlaps between
  /// the cell and the memory write is not exactly 1.
  ///
  /// For instance:
  ///   * If Cell{o,4,4}, offset=[0, 8], size=4: returns false
  ///   * If Cell{o,4,4}, offset=[4, 4], size=2: returns false
  ///   * If Cell{o,4,4}, offset=[0, 8], offset=4Z+0, size=4, returns true
  ///   * If Cell{o,4,4}, offset=[0, 8], offset=4Z+1, size=4, returns false
  bool cell_realizes_once(VariableRef cell,
                          IntIntervalCongruence offset,
                          const MachineInt& size) const {
    const MachineInt& cell_offset = CellVariableTrait::offset(cell);
    const MachineInt& cell_size = CellVariableTrait::size(cell);
    auto one = MachineInt(1, size.bit_width(), Unsigned);

    if (size != cell_size) {
      return false;
    }

    // Keep offsets that could 'touch' the cell
    bool overflow = false;

    // offset <= cell_offset + cell_size - 1
    MachineInt ub = add(cell_offset, cell_size - one, overflow);
    if (overflow) {
      ub.set_max();
    }

    // offset + size - 1 >= cell_offset
    MachineInt lb = sub(cell_offset, size - one, overflow);
    if (overflow) {
      lb.set_min();
    }

    offset.meet_with(IntIntervalCongruence(IntInterval(lb, ub)));

    return offset == IntIntervalCongruence(cell_offset);
  }

  /// \brief Create a new cell for a write, performing reduction if possible
  VariableRef write_realize_single_cell(MemoryLocationRef base,
                                        const MachineInt& offset,
                                        const MachineInt& size,
                                        Signedness sign) {
    VariableRef new_cell = this->make_cell(base, offset, size, sign);
    const CellSetT& cells = this->_cells.get(base);

    if (cells.is_empty()) {
      // No cell found for the base address
      this->_cells.set(base, CellSetT{new_cell});
      return new_cell;
    }

    CellSetT new_cells = cells;
    bool found = false;

    // Remove overlapping cells
    for (VariableRef cell : cells) {
      if (cell == new_cell) {
        found = true;
      } else if (this->cell_overlap(cell, new_cell)) {
        if (this->_scalar.uninit_is_uninitialized(cell)) {
          // Make new uninitialized cells for the parts not covered
          // by the new_cell.
          IntInterval new_interval = this->cell_range(new_cell);
          MachineInt new_lb = new_interval.lb();
          MachineInt new_ub = new_interval.ub();
          IntInterval un_interval = this->cell_range(cell);
          MachineInt un_lb = un_interval.lb();
          MachineInt un_ub = un_interval.ub();
          if (un_lb < new_lb) {
            MachineInt low_size = new_lb - un_lb;
            VariableRef low_cell = this->make_cell(base, un_lb, low_size, sign);
            this->_scalar.uninit_refine(low_cell,
                                        Uninitialized::uninitialized());
            new_cells.add(low_cell);
          }
          if (new_ub < un_ub) {
            MachineInt high_size = un_ub - new_ub;
            auto one = MachineInt(1, high_size.bit_width(), Unsigned);
            MachineInt high_lb = new_ub + one;
            VariableRef high_cell =
                this->make_cell(base, high_lb, high_size, sign);
            this->_scalar.uninit_refine(high_cell,
                                        Uninitialized::uninitialized());
            new_cells.add(high_cell);
          }
        }
        this->_scalar.dynamic_forget(cell);
        new_cells.remove(cell);
      }
    }

    if (!found) {
      new_cells.add(new_cell);
    }
    this->_cells.set(base, new_cells);
    return new_cell;
  }

  /// \brief Perform a write with an approximated offset.
  ///
  /// Returns a list of cells on which we should perform a weak update.
  std::vector< VariableRef > write_realize_range_cells(
      MemoryLocationRef base,
      const IntIntervalCongruence& offset,
      const MachineInt& size) {
    // Write byte range
    auto zero = MachineInt::zero(size.bit_width(), Unsigned);
    auto one = MachineInt(1, size.bit_width(), Unsigned);
    IntInterval range = add(offset.interval(), IntInterval(zero, size - one));

    // Current list of cells
    const CellSetT& cells = this->_cells.get(base);

    if (cells.is_empty()) {
      // no cell found for the base address
      return {};
    }

    CellSetT new_cells = cells;
    std::vector< VariableRef > updated_cells;

    for (VariableRef cell : cells) {
      if (this->cell_overlap(cell, range)) {
        if (this->cell_realizes_once(cell, offset, size)) {
          // This cell has only one way to be affected by the write statement
          updated_cells.push_back(cell);
        } else {
          this->_scalar.dynamic_forget(cell);
          new_cells.remove(cell);
        }
      }
    }

    this->_cells.set(base, new_cells);
    return updated_cells;
  }

  /// \brief Detect whether the read bits are initialized, uninitialized,
  /// for possibly uninitialized.
  ikos::core::Uninitialized is_read_uninitialized(MemoryLocationRef base,
                                                  const MachineInt& offset,
                                                  const MachineInt& size) {
    if (!size.fits< uint32_t >()) {
      // The ZNumber infrastructure cannot handle shifts by
      // more than the max 32-bit number, so give up in that case.
      // and assume possibly uninitialized.
      return ikos::core::Uninitialized::top();
    }

    // Use ZNumbers as bitvectors for bits [0,size) relative to the
    // low bound of the read (i.e. offset).
    ZNumber zoffset = offset.to_z_number();
    ZNumber zsize = size.to_z_number();
    ZNumber zupper = zoffset + zsize;
    ZNumber read_mask = make_clipped_mask(zoffset, zsize, zoffset, zsize);

    CellSetT cells = this->_cells.get(base);
    ZNumber initialized_coverage = ZNumber(0);
    ZNumber uninitialized_coverage = ZNumber(0);
    for (VariableRef cell : cells) {
      ZNumber other_offset = CellVariableTrait::offset(cell).to_z_number();
      ZNumber other_size = CellVariableTrait::size(cell).to_z_number();
      ZNumber cell_mask =
          make_clipped_mask(other_offset, other_size, zoffset, zsize);
      Uninitialized cell_uninit = this->uninit_to_uninitialized(cell);
      if (cell_uninit == ikos::core::Uninitialized::initialized()) {
        initialized_coverage |= cell_mask;
      } else if (cell_uninit == ikos::core::Uninitialized::uninitialized()) {
        uninitialized_coverage |= cell_mask;
      }
    }
    if (read_mask == initialized_coverage) {
      // The read bits are completely initialized
      return ikos::core::Uninitialized::initialized();
    } else if (read_mask == uninitialized_coverage) {
      // The read bits are all uninitialized
      return ikos::core::Uninitialized::uninitialized();
    }
    // The bits may or may not be initialized, so top.
    return ikos::core::Uninitialized::top();
  }

  /// \brief Create a new cell for a read
  VariableRef read_realize_single_cell(MemoryLocationRef base,
                                       const MachineInt& offset,
                                       const MachineInt& size,
                                       Signedness sign) {
    ikos::core::Uninitialized addr_uninit =
        is_read_uninitialized(base, offset, size);

    VariableRef new_cell = this->make_cell(base, offset, size, sign);
    this->_scalar.uninit_refine(new_cell, addr_uninit);
    CellSetT cells = this->_cells.get(base);
    cells.add(new_cell);
    this->_cells.set(base, cells);
    return new_cell;
  }

  /// \brief Assignment `var = literal`
  class LiteralWriter : public LiteralT::template Visitor<> {
  private:
    /// \brief Cell variable
    VariableRef _lhs;

    /// \brief Scalar abstract value
    ScalarDomain& _scalar;

  public:
    LiteralWriter(VariableRef lhs, ScalarDomain& scalar)
        : _lhs(lhs), _scalar(scalar) {}

    void machine_int(const MachineInt& rhs) {
      if (MachIntVariableTrait::bit_width(this->_lhs) == rhs.bit_width()) {
        this->_scalar.dynamic_write_int(this->_lhs, rhs);
      } else {
        this->_scalar.dynamic_write_nondet_int(this->_lhs);
      }
    }

    void floating_point(const DummyNumber&) {
      this->_scalar.dynamic_write_nondet_float(this->_lhs);
    }

    void memory_location(MemoryLocationRef addr) {
      this->_scalar.dynamic_write_pointer(this->_lhs,
                                          addr,
                                          Nullity::non_null());
    }

    void null() { this->_scalar.dynamic_write_null(this->_lhs); }

    void undefined() { this->_scalar.dynamic_write_undef(this->_lhs); }

    void machine_int_var(VariableRef rhs) {
      if (MachIntVariableTrait::bit_width(_lhs) ==
          MachIntVariableTrait::bit_width(rhs)) {
        this->_scalar.dynamic_write_int(this->_lhs, rhs);
      } else {
        this->_scalar.dynamic_write_nondet_int(this->_lhs);
      }
    }

    void floating_point_var(VariableRef /*rhs*/) {
      this->_scalar.dynamic_write_nondet_float(this->_lhs);
    }

    void pointer_var(VariableRef rhs) {
      this->_scalar.dynamic_write_pointer(this->_lhs, rhs);
    }

  }; // end class LiteralWriter

  /// \brief Assignment `literal = var`
  class LiteralReader : public LiteralT::template Visitor<> {
  private:
    /// \brief Cell variable
    VariableRef _rhs;

    /// \brief Scalar abstract value
    ScalarDomain& _scalar;

  public:
    LiteralReader(VariableRef rhs, ScalarDomain& scalar)
        : _rhs(rhs), _scalar(scalar) {}

    void machine_int(const MachineInt&) {
      ikos_unreachable("trying to assign a machine integer");
    }

    void floating_point(const DummyNumber&) {
      ikos_unreachable("trying to assign a floating point");
    }

    void memory_location(MemoryLocationRef) {
      ikos_unreachable("trying to assign a memory location");
    }

    void null() { ikos_unreachable("trying to assign to null"); }

    void undefined() { ikos_unreachable("trying to assign to undefined"); }

    void machine_int_var(VariableRef lhs) {
      // If the right hand side is a null pointer, assign the integer to zero
      // (implicit cast from pointer to int)
      if (this->_scalar.dynamic_is_null(this->_rhs)) {
        auto zero = MachineInt::zero(MachIntVariableTrait::bit_width(lhs),
                                     MachIntVariableTrait::sign(lhs));
        this->_scalar.int_assign(lhs, zero);
      } else if (MachIntVariableTrait::bit_width(lhs) ==
                 MachIntVariableTrait::bit_width(this->_rhs)) {
        this->_scalar.dynamic_read_int(lhs, this->_rhs);
      } else {
        this->_scalar.int_assign_nondet(lhs);
      }
    }

    void floating_point_var(VariableRef lhs) {
      this->_scalar.float_assign_nondet(lhs);
    }

    void pointer_var(VariableRef lhs) {
      // If the right hand side is the integer zero, assign the pointer to null
      // (implicit cast from int to pointer)
      if (this->_scalar.dynamic_is_zero(this->_rhs)) {
        this->_scalar.pointer_assign_null(lhs);
      } else {
        this->_scalar.dynamic_read_pointer(lhs, this->_rhs);
      }
    }

  }; // end class LiteralReader

  /// \brief Perform a strong update `lhs = rhs`
  void strong_update(VariableRef lhs, const LiteralT& rhs) {
    LiteralWriter v(lhs, this->_scalar);
    rhs.apply_visitor(v);
  }

  /// \brief Perform a weak update `lhs = rhs`
  void weak_update(VariableRef lhs, const LiteralT& rhs) {
    ScalarDomain scalar = this->_scalar;
    LiteralWriter v(lhs, scalar);
    rhs.apply_visitor(v);
    this->_scalar.join_with(std::move(scalar));
  }

  /// \brief Perform a strong update `lhs = rhs`
  void strong_update(const LiteralT& lhs, VariableRef rhs) {
    LiteralReader v(rhs, this->_scalar);
    lhs.apply_visitor(v);
  }

  /// \brief Perform a weak update `lhs = rhs`
  void weak_update(const LiteralT& lhs, VariableRef rhs) {
    ScalarDomain scalar = this->_scalar;
    LiteralReader v(rhs, scalar);
    lhs.apply_visitor(v);
    this->_scalar.join_with(std::move(scalar));
  }

public:
  void mem_write(VariableRef ptr,
                 const LiteralT& rhs,
                 const MachineInt& size) override {
    ikos_assert(ScalarVariableTrait::is_pointer(ptr));

    if (this->is_bottom_fast()) {
      return;
    }

    // Null/undefined pointer dereference
    this->_scalar.nullity_assert_non_null(ptr);

    // Writing an uninitialized variable is an error
    // Note that writing the undefined constant is allowed
    if (rhs.is_var()) {
      this->_scalar.uninit_assert_initialized(rhs.var());
    }

    this->_scalar.normalize();

    if (this->_scalar.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    // Memory locations pointed by the pointer
    PointsToSetT addrs = this->_scalar.pointer_to_points_to(ptr);

    if (addrs.is_empty()) {
      // Invalid dereference
      this->set_to_bottom();
      return;
    }

    if (size.is_zero()) {
      // Does nothing
      return;
    }

    if (addrs.is_top()) {
      this->mem_forget_all(); // Very conservative, but sound
      return;
    }

    //
    // Update memory cells
    //

    // Offset interval-congruence
    IntIntervalCongruence offset_ic =
        this->_scalar.pointer_offset_to_interval_congruence(ptr);
    ikos_assert(offset_ic.sign() == Unsigned);

    if (offset_ic.singleton()) {
      // The offset has one possible value.
      //
      // We can perform the usual reduction and update.
      MachineInt offset = *offset_ic.singleton();
      Signedness sign = this->preferred_cell_sign(rhs);

      for (MemoryLocationRef addr : addrs) {
        VariableRef cell =
            this->write_realize_single_cell(addr, offset, size, sign);

        if (addrs.size() == 1) {
          this->strong_update(cell, rhs);
        } else {
          this->weak_update(cell, rhs);
        }
      }
    } else {
      // The offset is a range.
      //
      // We check for all overlapping cells. If there is only one way to
      // write on that cell, we perform a weak update, otherwise we just
      // kill the cell.
      //
      // For instance, let's say offset=[0, 10], size=4 and we have a cell
      // C{o, 4, 4}.
      //
      // There are 7 ways to write on the cell:
      // C{o,1,4}, C{o,2,4}, ..., C{o,6,4} and C{o,7,4}.
      // Thus, we need to kill the cell.
      //
      // If we also have offset=4Z+0 from the underlying abstract domain, then
      // there is only one way: C{o,4,4} and thus we can perform a weak
      // update.

      for (MemoryLocationRef addr : addrs) {
        std::vector< VariableRef > cells =
            this->write_realize_range_cells(addr, offset_ic, size);
        for (VariableRef cell : cells) {
          this->weak_update(cell, rhs);
        }
      }
    }

    //
    // Update pointer sets
    //

    auto rhs_ptr = PointerAbsValueT::bottom(1, Unsigned);
    if (rhs.is_memory_location()) {
      rhs_ptr =
          PointerAbsValueT(Uninitialized::initialized(),
                           Nullity::non_null(),
                           PointsToSetT{rhs.memory_location()},
                           IntInterval(MachineInt::zero(offset_ic.bit_width(),
                                                        Unsigned)));
    } else if (rhs.is_pointer_var()) {
      rhs_ptr = this->_scalar.pointer_to_pointer(rhs.var());
    } else {
      // Right hand side is not a pointer, nothing else to do
      return;
    }

    for (MemoryLocationRef addr : addrs) {
      PointerSetT pointer_set =
          this->_pointer_sets.get(addr, offset_ic.bit_width(), Unsigned);
      pointer_set.add(rhs_ptr);
      this->_pointer_sets.set(addr, pointer_set);
    }
  }

  void mem_read(const LiteralT& lhs,
                VariableRef ptr,
                const MachineInt& size) override {
    ikos_assert(lhs.is_var());
    ikos_assert(ScalarVariableTrait::is_pointer(ptr));
    ikos_assert(size.is_strictly_positive());

    if (this->is_bottom_fast()) {
      return;
    }

    // Null/undefined pointer dereference
    this->_scalar.nullity_assert_non_null(ptr);

    this->_scalar.normalize();

    if (this->_scalar.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    // Memory locations pointed by the pointer
    PointsToSetT addrs = this->_scalar.pointer_to_points_to(ptr);

    if (addrs.is_empty()) {
      // Invalid dereference
      this->set_to_bottom();
      return;
    }

    if (addrs.is_top()) {
      this->_scalar.scalar_assign_nondet(lhs.var());
      return;
    }

    //
    // Handle memory cells
    //

    // Offset interval
    IntInterval offset_intv = this->_scalar.pointer_offset_to_interval(ptr);
    ikos_assert(offset_intv.sign() == Unsigned);

    if (offset_intv.singleton()) {
      // The offset has one possible value.
      //
      // We can perform the usual reduction and update.
      MachineInt offset = *offset_intv.singleton();
      Signedness sign = this->preferred_cell_sign(lhs);
      bool first = true;

      for (MemoryLocationRef addr : addrs) {
        VariableRef cell =
            this->read_realize_single_cell(addr, offset, size, sign);

        if (first) {
          this->strong_update(lhs, cell);
          first = false;
        } else {
          this->weak_update(lhs, cell);
        }
      }
    } else {
      // The offset is a range.
      //
      // If we try to read a summarized region of the heap we just
      // return top. Note that we do not keep track of writes into
      // summarized cells that's why if we read a summarized cell
      // the only sound result we can return is top.
      //
      // TODO(jnavas): note that we could have a bounded array for which we
      // have the complete set of cells and thus we could be more
      // precise in that case.
      //
      // To deal with this situation in a more systematic way
      // we should combine anyway the value domain with some
      // summarization array-based domain (e.g.,some array domain
      // such as a trivial array smashing or something more
      // expressive like Cousot&Logozzo's POPL'11).
      this->_scalar.scalar_assign_nondet(lhs.var());
    }

    //
    // Handle pointer sets
    //

    if (lhs.is_pointer_var()) {
      PointerSetT pointer_set =
          PointerSetT::bottom(offset_intv.bit_width(), Unsigned);

      for (MemoryLocationRef addr : addrs) {
        pointer_set.join_with(
            this->_pointer_sets.get(addr, offset_intv.bit_width(), Unsigned));
      }

      this->_scalar.pointer_refine(lhs.var(), pointer_set);
    }
  }

  void mem_copy(VariableRef dest,
                VariableRef src,
                const LiteralT& size) override {
    ikos_assert(ScalarVariableTrait::is_pointer(dest));
    ikos_assert(ScalarVariableTrait::is_pointer(src));

    if (this->is_bottom_fast()) {
      return;
    }

    // Null/undefined pointer dereference
    this->_scalar.nullity_assert_non_null(src);
    this->_scalar.nullity_assert_non_null(dest);

    if (size.is_undefined()) {
      this->_scalar.set_to_bottom();
      return;
    } else if (size.is_var()) {
      this->_scalar.uninit_assert_initialized(size.var());
    }

    this->_scalar.normalize();

    if (this->_scalar.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    // Memory locations pointed by dest and src
    PointsToSetT src_addrs = this->_scalar.pointer_to_points_to(src);
    PointsToSetT dest_addrs = this->_scalar.pointer_to_points_to(dest);

    if (src_addrs.is_empty() || dest_addrs.is_empty()) {
      // Invalid dereference
      this->set_to_bottom();
      return;
    }

    if (dest_addrs.is_top()) {
      this->mem_forget_all(); // Very conservative, but sound
      return;
    }

    //
    // Update memory cells
    //

    // Offsets and size intervals
    IntInterval src_intv = this->_scalar.pointer_offset_to_interval(src);
    IntInterval dest_intv = this->_scalar.pointer_offset_to_interval(dest);
    auto size_intv = IntInterval::bottom(1, Unsigned);

    if (size.is_machine_int()) {
      size_intv = IntInterval(size.machine_int());
    } else if (size.is_machine_int_var()) {
      size_intv = this->_scalar.int_to_interval(size.var());
    } else {
      ikos_unreachable("unexpected literal for size");
    }
    assert_compatible(size_intv, src_intv);

    if (size_intv.ub().is_zero()) {
      return; // Does nothing
    }

    // To be sound, remove all reachable cells
    for (MemoryLocationRef addr : dest_addrs) {
      this->mem_forget_cells(addr, dest_intv, size_intv.ub());
    }

    if (dest_addrs.singleton() && dest_intv.singleton() &&
        !src_addrs.is_top() && src_intv.singleton() &&
        !size_intv.lb().is_zero()) {
      // In this case, we can be more precise
      MemoryLocationRef dest_addr = *dest_addrs.singleton();
      MachineInt dest_offset = *dest_intv.singleton();
      MachineInt src_offset = *src_intv.singleton();
      const MachineInt& size_lb = size_intv.lb();
      auto one = MachineInt(1, dest_intv.bit_width(), Unsigned);
      auto src_range = IntInterval(src_offset, src_offset + (size_lb - one));

      boost::optional< ScalarDomain > new_scalar;
      CellSetT dest_cells = this->_cells.get(dest_addr);

      for (MemoryLocationRef src_addr : src_addrs) {
        // Copy from src_addr/src_offset to dest_addr/dest_offset
        ScalarDomain scalar = this->_scalar;
        const CellSetT& src_cells = this->_cells.get(src_addr);

        for (VariableRef cell : src_cells) {
          if (this->cell_range(cell).leq(src_range)) {
            VariableRef new_cell =
                this->make_cell(dest_addr,
                                dest_offset + (CellVariableTrait::offset(cell) -
                                               src_offset),
                                CellVariableTrait::size(cell),
                                MachIntVariableTrait::sign(cell));
            dest_cells.add(new_cell);
            scalar.dynamic_assign(new_cell, cell);
          }
        }

        if (!new_scalar) {
          new_scalar = std::move(scalar);
        } else {
          new_scalar->join_with(std::move(scalar));
        }
      }

      ikos_assert(new_scalar);
      this->_scalar = std::move(*new_scalar);
      this->_cells.set(dest_addr, dest_cells);
    }

    //
    // Update pointer sets
    //

    // Collect source pointer sets
    auto src_pointer_set = PointerSetT::bottom(src_intv.bit_width(), Unsigned);
    if (src_addrs.is_top()) {
      src_pointer_set.set_to_top(); // Be sound
    } else {
      for (MemoryLocationRef addr : src_addrs) {
        src_pointer_set.join_with(
            this->_pointer_sets.get(addr, src_intv.bit_width(), Unsigned));
      }
    }

    // Update destination pointer sets
    for (MemoryLocationRef addr : dest_addrs) {
      PointerSetT dest_pointer_set =
          this->_pointer_sets.get(addr, src_intv.bit_width(), Unsigned);
      dest_pointer_set.join_with(src_pointer_set);
      this->_pointer_sets.set(addr, dest_pointer_set);
    }
  }

  void mem_set(VariableRef dest,
               const LiteralT& value,
               const LiteralT& size) override {
    ikos_assert(ScalarVariableTrait::is_pointer(dest));

    if (this->is_bottom_fast()) {
      return;
    }

    // Null/undefined pointer dereference
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

    this->_scalar.normalize();

    if (this->_scalar.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    // Memory locations pointed by dest
    PointsToSetT addrs = this->_scalar.pointer_to_points_to(dest);

    if (addrs.is_empty()) {
      // Invalid dereference
      this->set_to_bottom();
      return;
    }

    if (addrs.is_top()) {
      this->mem_forget_cells(); // Very conservative, but sound
      return;
    }

    //
    // Update memory cells
    //

    // Offset, size and value intervals
    IntInterval dest_intv = this->_scalar.pointer_offset_to_interval(dest);
    auto size_intv = IntInterval::bottom(1, Unsigned);
    auto value_intv = IntInterval::bottom(1, Unsigned);

    if (size.is_machine_int()) {
      size_intv = IntInterval(size.machine_int());
    } else if (size.is_machine_int_var()) {
      size_intv = this->_scalar.int_to_interval(size.var());
    } else {
      ikos_unreachable("unexpected literal for size");
    }
    assert_compatible(size_intv, dest_intv);

    if (value.is_machine_int()) {
      value_intv = IntInterval(value.machine_int());
    } else if (size.is_machine_int_var()) {
      value_intv = this->_scalar.int_to_interval(value.var());
    } else {
      ikos_unreachable("unexpected literal for value");
    }

    if (size_intv.ub().is_zero()) {
      return; // Does nothing
    }

    if (value_intv.is_zero()) {
      // Memory set to zero
      const MachineInt& size_lb = size_intv.lb();
      auto zero = MachineInt::zero(size_lb.bit_width(), Unsigned);
      auto one = MachineInt(1, size_lb.bit_width(), Unsigned);

      // Offsets that are updated
      auto safe_range_lb =
          IntInterval(dest_intv.lb(), dest_intv.lb() + (size_lb - one));
      auto safe_range_ub =
          IntInterval(dest_intv.ub(), dest_intv.ub() + (size_lb - one));
      IntInterval safe_range = safe_range_lb.meet(safe_range_ub);

      // Possibly updated offsets
      IntInterval unsafe_range =
          add(dest_intv, IntInterval(zero, size_intv.ub() - one));

      for (MemoryLocationRef addr : addrs) {
        const CellSetT& cells = this->_cells.get(addr);

        if (!cells.is_empty()) {
          CellSetT new_cells = cells;

          for (VariableRef cell : cells) {
            IntInterval range = this->cell_range(cell);

            if (range.leq(safe_range)) {
              LiteralT zero_lit = LiteralT::machine_int(
                  MachineInt::zero(MachIntVariableTrait::bit_width(cell),
                                   MachIntVariableTrait::sign(cell)));
              if (addrs.singleton()) {
                this->strong_update(cell, zero_lit);
              } else {
                this->weak_update(cell, zero_lit);
              }
            } else if (!range.meet(unsafe_range).is_bottom()) {
              this->_scalar.dynamic_forget(cell);
              new_cells.remove(cell);
            }
          }

          this->_cells.set(addr, new_cells);
        }
      }
    } else {
      // To be sound, remove all reachable cells
      for (MemoryLocationRef addr : addrs) {
        this->mem_forget_cells(addr, dest_intv, size_intv.ub());
      }
    }

    // Nothing to do for pointer sets
  }

private:
  /// \brief Forget all memory cells
  void mem_forget_cells() {
    if (this->_cells.is_bottom()) {
      return;
    }

    for (auto it = this->_cells.begin(), et = this->_cells.end(); it != et;
         ++it) {
      const CellSetT& cells = it->second;

      if (cells.is_empty()) {
        continue;
      }

      for (VariableRef cell : cells) {
        this->_scalar.dynamic_forget(cell);
      }
    }

    this->_cells.set_to_top();
  }

  /// \brief Forget the memory cells for the given memory location
  void mem_forget_cells(MemoryLocationRef addr) {
    const CellSetT& cells = this->_cells.get(addr);

    if (cells.is_bottom()) {
      return;
    }

    for (VariableRef cell : cells) {
      this->_scalar.dynamic_forget(cell);
    }

    this->_cells.forget(addr);
  }

  /// \brief Forget the memory cells in range
  /// `[addr + offset, addr + offset + size - 1]`
  void mem_forget_cells(MemoryLocationRef addr,
                        const IntInterval& offset,
                        const MachineInt& size) {
    if (size.is_zero()) {
      return;
    }

    auto zero = MachineInt::zero(size.bit_width(), Unsigned);
    auto one = MachineInt(1, size.bit_width(), Unsigned);
    this->mem_forget_cells(addr, add(offset, IntInterval(zero, size - one)));
  }

  /// \brief Forget the memory cells in
  /// `[addr + range.lb(), addr + range.ub()]`
  void mem_forget_cells(MemoryLocationRef addr, const IntInterval& range) {
    const CellSetT& cells = this->_cells.get(addr);

    if (cells.is_bottom() || cells.is_empty()) {
      return;
    }

    CellSetT new_cells = cells;

    for (VariableRef cell : cells) {
      if (this->cell_overlap(cell, range)) {
        this->_scalar.dynamic_forget(cell);
        new_cells.remove(cell);
      }
    }

    this->_cells.set(addr, new_cells);
  }

  /// \brief Forget all pointer sets
  void mem_forget_pointer_sets() { this->_pointer_sets.set_to_top(); }

  /// \brief Forget the pointer set for the given memory location
  void mem_forget_pointer_set(MemoryLocationRef addr) {
    this->_pointer_sets.forget(addr);
  }

public:
  void mem_forget_all() override {
    this->mem_forget_cells();
    this->mem_forget_pointer_sets();
  }

  void mem_forget(MemoryLocationRef addr) override {
    this->mem_forget_cells(addr);
    this->mem_forget_pointer_set(addr);
  }

  void mem_forget(MemoryLocationRef addr,
                  const IntInterval& offset,
                  const MachineInt& size) override {
    if (size.is_zero()) {
      return;
    }

    this->mem_forget_cells(addr, offset, size);
    this->mem_forget_pointer_set(addr);
  }

  void mem_forget(MemoryLocationRef addr, const IntInterval& range) override {
    this->mem_forget_cells(addr, range);
    this->mem_forget_pointer_set(addr);
  }

  void mem_forget_reachable(VariableRef p) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_scalar.uninit_assert_initialized(p);

    this->_scalar.normalize();

    if (this->_scalar.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    if (this->_scalar.nullity_is_null(p)) {
      return; // Does nothing
    }

    // Memory locations pointed by the pointer
    PointsToSetT addrs = this->_scalar.pointer_to_points_to(p);

    if (addrs.is_top()) {
      this->mem_forget_all(); // Very conservative, but sound
      return;
    }

    for (MemoryLocationRef addr : addrs) {
      this->mem_forget(addr);
    }
  }

  void mem_forget_reachable(VariableRef p, const MachineInt& size) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_scalar.uninit_assert_initialized(p);

    this->_scalar.normalize();

    if (this->_scalar.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    if (this->_scalar.nullity_is_null(p)) {
      return; // Does nothing
    }

    if (size.is_zero()) {
      return; // Does nothing
    }

    // Memory locations pointed by the pointer
    PointsToSetT addrs = this->_scalar.pointer_to_points_to(p);

    if (addrs.is_top()) {
      this->mem_forget_all(); // very conservative, but sound
      return;
    }

    // Offset interval
    IntInterval offset_intv = this->_scalar.pointer_offset_to_interval(p);

    for (MemoryLocationRef addr : addrs) {
      this->mem_forget(addr, offset_intv, size);
    }
  }

  void mem_abstract_reachable(VariableRef p) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_scalar.uninit_assert_initialized(p);

    this->_scalar.normalize();

    if (this->_scalar.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    if (this->_scalar.nullity_is_null(p)) {
      return; // Does nothing
    }

    // Memory locations pointed by the pointer
    PointsToSetT addrs = this->_scalar.pointer_to_points_to(p);

    if (addrs.is_top()) {
      this->mem_forget_cells(); // Very conservative, but sound
      return;
    }

    for (MemoryLocationRef addr : addrs) {
      this->mem_forget_cells(addr);
    }
  }

  void mem_abstract_reachable(VariableRef p, const MachineInt& size) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_scalar.uninit_assert_initialized(p);

    this->_scalar.normalize();

    if (this->_scalar.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    if (this->_scalar.nullity_is_null(p)) {
      return; // Does nothing
    }

    if (size.is_zero()) {
      return; // Does nothing
    }

    // Memory locations pointed by the pointer
    PointsToSetT addrs = this->_scalar.pointer_to_points_to(p);

    if (addrs.is_top()) {
      this->mem_forget_cells(); // Very conservative, but sound
      return;
    }

    // Offset interval
    IntInterval offset_intv = this->_scalar.pointer_offset_to_interval(p);

    for (MemoryLocationRef addr : addrs) {
      this->mem_forget_cells(addr, offset_intv, size);
    }
  }

  void mem_zero_reachable(VariableRef p) override {
    this->mem_uninitialize_reachable(p);
  }

  void mem_uninitialize_reachable(VariableRef p) override {
    ikos_assert(ScalarVariableTrait::is_pointer(p));

    if (this->is_bottom_fast()) {
      return;
    }

    this->_scalar.uninit_assert_initialized(p);

    this->_scalar.normalize();

    if (this->_scalar.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    if (this->_scalar.nullity_is_null(p)) {
      return; // Does nothing
    }

    // Memory locations pointed by the pointer
    PointsToSetT addrs = this->_scalar.pointer_to_points_to(p);

    if (addrs.is_top()) {
      this->mem_forget_all(); // Very conservative, but sound
      return;
    }

    for (MemoryLocationRef addr : addrs) {
      this->mem_forget(addr);
    }

    if (auto addr = addrs.singleton()) {
      uint64_t bit_width =
          MachIntVariableTrait::bit_width(ScalarVariableTrait::offset_var(p));
      this->_pointer_sets.set(*addr, PointerSetT::empty(bit_width, Unsigned));
    }
  }

  /// @}
  /// \name Lifetime abstract domain methods
  /// @{

  void lifetime_assign_allocated(MemoryLocationRef m) override {
    this->_lifetime.assign_allocated(m);
  }

  void lifetime_assign_deallocated(MemoryLocationRef m) override {
    this->_lifetime.assign_deallocated(m);
  }

  void lifetime_assert_allocated(MemoryLocationRef m) override {
    this->_lifetime.assert_allocated(m);
  }

  void lifetime_assert_deallocated(MemoryLocationRef m) override {
    this->_lifetime.assert_deallocated(m);
  }

  void lifetime_forget(MemoryLocationRef m) override {
    this->_lifetime.forget(m);
  }

  void lifetime_set(MemoryLocationRef m, Lifetime value) override {
    this->_lifetime.set(m, value);
  }

  Lifetime lifetime_to_lifetime(MemoryLocationRef m) const override {
    return this->_lifetime.get(m);
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

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else if (this->is_top()) {
      o << "T";
    } else {
      o << "(";
      this->_scalar.dump(o);
      o << ", ";
      this->_cells.dump(o);
      o << ", ";
      this->_pointer_sets.dump(o);
      o << ", ";
      this->_lifetime.dump(o);
      o << ")";
    }
  }

  static std::string name() {
    return "value domain using " + ScalarDomain::name() + " and " +
           LifetimeDomain::name();
  }

}; // end class ValueDomain

} // end namespace memory
} // end namespace core
} // end namespace ikos
