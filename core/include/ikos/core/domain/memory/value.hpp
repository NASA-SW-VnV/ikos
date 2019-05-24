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

#include <ikos/core/domain/memory/abstract_domain.hpp>
#include <ikos/core/domain/memory/value/cell_set.hpp>
#include <ikos/core/domain/memory/value/mem_loc_to_cell_set.hpp>
#include <ikos/core/domain/memory/value/mem_loc_to_pointer_set.hpp>
#include <ikos/core/semantic/memory/cell.hpp>

namespace ikos {
namespace core {
namespace memory {

/// \brief Traits for creating cell variables
///
/// Elements to provide:
///
/// static VariableRef cell(VariableFactory& vfac,
///                         MemoryLocationRef base,
///                         const MachineInt& offset,
///                         const MachineInt& size,
///                         Signedness sign)
///   Get or create the cell with the given base address, offset and size
///   If a new cell is created, it will have the given signedness
template < typename VariableRef,
           typename MemoryLocationRef,
           typename VariableFactory >
struct CellFactoryTraits {};

/// \brief Value abstract domain
///
/// Memory abstraction consisting of cells and a pointer abstraction
/// augmented with uninitialized variable information.
///
/// This domain abstracts memory into a set of synthetic cells with
/// integer, float or pointer type following Mine's paper. If a cell is
/// of type integer this domain can model its value and whether it is
/// initialized or not. If the cell is of type float, the domain keeps
/// track of whether it is initialized or not. If the cell is of pointer
/// type this domain keeps track of its address, its offset, whether it is
/// null or not and whether it is initialized or not.
/// We also allow reduction with an external pointer analysis that can
/// provide points-to set and offset information.
///
/// A memory cell is a triple `(base, offset, size)` modelling all bytes at
/// address `base`, starting at offset `offset` up to `offset + size - 1`. A
/// memory cell is represented by a variable implementing
/// `memory::CellVariableTraits`. The variable doesn't have a fixed type. It is
/// either an integer of 8*size bits, a floating point of 8*size bits or a
/// pointer.
template < typename VariableRef,
           typename MemoryLocationRef,
           typename VariableFactory,
           typename MachineIntDomain,
           typename NullityDomain,
           typename PointerDomain,
           typename UninitializedDomain,
           typename LifetimeDomain >
class ValueDomain final
    : public memory::AbstractDomain< VariableRef,
                                     MemoryLocationRef,
                                     VariableFactory,
                                     MachineIntDomain,
                                     NullityDomain,
                                     PointerDomain,
                                     UninitializedDomain,
                                     LifetimeDomain,
                                     ValueDomain< VariableRef,
                                                  MemoryLocationRef,
                                                  VariableFactory,
                                                  MachineIntDomain,
                                                  NullityDomain,
                                                  PointerDomain,
                                                  UninitializedDomain,
                                                  LifetimeDomain > > {
public:
  static_assert(memory::IsCellVariable< VariableRef, MemoryLocationRef >::value,
                "VariableRef must implement memory::CellVariableTraits");

private:
  /// \brief Trait for typed variables,
  /// see ikos/core/semantic/memory/variable.hpp
  using MemVariableTrait = memory::VariableTraits< VariableRef >;

  /// \brief Trait for integer variables,
  /// see ikos/core/semantic/machine_int/variable.hpp
  using MachIntVariableTrait = machine_int::VariableTraits< VariableRef >;

  /// \brief Trait for cell variables,
  /// see ikos/core/semantic/memory/cell.hpp
  using CellVariableTrait =
      CellVariableTraits< VariableRef, MemoryLocationRef >;

  /// \brief Trait for creating cell variables, see above
  using CellFactoryTrait =
      CellFactoryTraits< VariableRef, MemoryLocationRef, VariableFactory >;

  /// \brief Set of cells
  using CellSetT = CellSet< VariableRef >;

  /// \brief Map from base addresses to set of synthetic cells
  ///
  /// TODO(jnavas): it should be a map from pointer offsets to set of
  /// overlapping cells to make operations faster.
  using MemLocToCellSetT = MemLocToCellSet< MemoryLocationRef, VariableRef >;

  /// \brief Points-to set
  using PointsToSetT = PointsToSet< MemoryLocationRef >;

  /// \brief Pointer abstract value
  using PointerAbsValueT = PointerAbsValue< MemoryLocationRef >;

  /// \brief Pointer set
  using PointerSetT = PointerSet< MemoryLocationRef >;

  /// \brief Map from base addresses to set of pointers
  using MemLocToPointerSetT = MemLocToPointerSet< MemoryLocationRef >;

  /// \brief Literal
  using LiteralT = Literal< VariableRef, MemoryLocationRef >;

  /// \brief Machine integer interval
  using Interval = machine_int::Interval;

  /// \brief Machine integer interval-congruence
  using IntervalCongruence = machine_int::IntervalCongruence;

private:
  MemLocToCellSetT _cells;
  MemLocToPointerSetT _pointer_sets;
  PointerDomain _pointer;
  UninitializedDomain _uninitialized;
  LifetimeDomain _lifetime;

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top abstract value
  explicit ValueDomain(TopTag)
      : _cells(MemLocToCellSetT::top()),
        _pointer_sets(MemLocToPointerSetT::top()),
        _pointer(PointerDomain::top()),
        _uninitialized(UninitializedDomain::top()),
        _lifetime(LifetimeDomain::top()) {}

  /// \brief Create the bottom abstract value
  explicit ValueDomain(BottomTag)
      : _cells(MemLocToCellSetT::bottom()),
        _pointer_sets(MemLocToPointerSetT::bottom()),
        _pointer(PointerDomain::bottom()),
        _uninitialized(UninitializedDomain::bottom()),
        _lifetime(LifetimeDomain::bottom()) {}

public:
  /// \brief Create the top abstract value
  ValueDomain() : ValueDomain(TopTag{}) {}

  /// \brief Create an abstract value with the given underlying domains
  ///
  /// \param pointer The pointer abstract value
  /// \param uninitialized The uninitialized abstract value
  explicit ValueDomain(PointerDomain pointer,
                       UninitializedDomain uninitialized,
                       LifetimeDomain lifetime)
      : _cells(MemLocToCellSetT::top()),
        _pointer_sets(MemLocToPointerSetT::top()),
        _pointer(std::move(pointer)),
        _uninitialized(std::move(uninitialized)),
        _lifetime(std::move(lifetime)) {
    this->normalize();
  }

  /// \brief Copy constructor
  ValueDomain(const ValueDomain&) = default;

  /// \brief Move constructor
  ValueDomain(ValueDomain&&) = default;

  /// \brief Copy assignment operator
  ValueDomain& operator=(const ValueDomain&) = default;

  /// \brief Move assignment operator
  ValueDomain& operator=(ValueDomain&&) = default;

  /// \brief Destructor
  ~ValueDomain() override = default;

  /// \brief Create the top abstract value
  static ValueDomain top() { return ValueDomain(TopTag{}); }

  /// \brief Create the bottom abstract value
  static ValueDomain bottom() { return ValueDomain(BottomTag{}); }

  /*
   * Implement core::AbstractDomain
   */

  bool is_bottom() const override {
    this->normalize();
    return this->_cells.is_bottom(); // Correct because of normalization
  }

  bool is_top() const override {
    return this->_cells.is_top() && this->_pointer_sets.is_top() &&
           this->_pointer.is_top() && this->_uninitialized.is_top() &&
           this->_lifetime.is_top();
  }

  void set_to_bottom() override {
    this->_cells.set_to_bottom();
    this->_pointer_sets.set_to_bottom();
    this->_pointer.set_to_bottom();
    this->_uninitialized.set_to_bottom();
    this->_lifetime.set_to_bottom();
  }

  void set_to_top() override {
    this->_cells.set_to_top();
    this->_pointer_sets.set_to_top();
    this->_pointer.set_to_top();
    this->_uninitialized.set_to_top();
    this->_lifetime.set_to_top();
  }

  bool leq(const ValueDomain& other) const override {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_cells.leq(other._cells) &&
             this->_pointer_sets.leq(other._pointer_sets) &&
             this->_pointer.leq(other._pointer) &&
             this->_uninitialized.leq(other._uninitialized) &&
             this->_lifetime.leq(other._lifetime);
    }
  }

  bool equals(const ValueDomain& other) const override {
    if (this->is_bottom()) {
      return other.is_bottom();
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_cells.equals(other._cells) &&
             this->_pointer_sets.equals(other._pointer_sets) &&
             this->_pointer.equals(other._pointer) &&
             this->_uninitialized.equals(other._uninitialized) &&
             this->_lifetime.equals(other._lifetime);
    }
  }

  void join_with(const ValueDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_cells.join_with(other._cells);
      this->_pointer_sets.join_with(other._pointer_sets);
      this->_pointer.join_with(other._pointer);
      this->_uninitialized.join_with(other._uninitialized);
      this->_lifetime.join_with(other._lifetime);
    }
  }

  void join_loop_with(const ValueDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_cells.join_loop_with(other._cells);
      this->_pointer_sets.join_loop_with(other._pointer_sets);
      this->_pointer.join_loop_with(other._pointer);
      this->_uninitialized.join_loop_with(other._uninitialized);
      this->_lifetime.join_loop_with(other._lifetime);
    }
  }

  void join_iter_with(const ValueDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_cells.join_iter_with(other._cells);
      this->_pointer_sets.join_iter_with(other._pointer_sets);
      this->_pointer.join_iter_with(other._pointer);
      this->_uninitialized.join_iter_with(other._uninitialized);
      this->_lifetime.join_iter_with(other._lifetime);
    }
  }

  void widen_with(const ValueDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_cells.widen_with(other._cells);
      this->_pointer_sets.widen_with(other._pointer_sets);
      this->_pointer.widen_with(other._pointer);
      this->_uninitialized.widen_with(other._uninitialized);
      this->_lifetime.widen_with(other._lifetime);
    }
  }

  void widen_threshold_with(const ValueDomain& other,
                            const MachineInt& threshold) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_cells.widen_with(other._cells);
      this->_pointer_sets.join_with(other._pointer_sets);
      this->_pointer.widen_threshold_with(other._pointer, threshold);
      this->_uninitialized.widen_with(other._uninitialized);
      this->_lifetime.widen_with(other._lifetime);
    }
  }

  void meet_with(const ValueDomain& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_cells.meet_with(other._cells);
      this->_pointer_sets.meet_with(other._pointer_sets);
      this->_pointer.meet_with(other._pointer);
      this->_uninitialized.meet_with(other._uninitialized);
      this->_lifetime.meet_with(other._lifetime);
    }
  }

  void narrow_with(const ValueDomain& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_cells.narrow_with(other._cells);
      this->_pointer_sets.narrow_with(other._pointer_sets);
      this->_pointer.narrow_with(other._pointer);
      this->_uninitialized.narrow_with(other._uninitialized);
      this->_lifetime.narrow_with(other._lifetime);
    }
  }

  /*
   * Implement memory::AbstractDomain
   */

  MachineIntDomain& integers() override { return this->_pointer.integers(); }

  const MachineIntDomain& integers() const override {
    return this->_pointer.integers();
  }

  NullityDomain& nullity() override { return this->_pointer.nullity(); }

  const NullityDomain& nullity() const override {
    return this->_pointer.nullity();
  }

  PointerDomain& pointers() override { return this->_pointer; }

  const PointerDomain& pointers() const override { return this->_pointer; }

  UninitializedDomain& uninitialized() override { return this->_uninitialized; }

  const UninitializedDomain& uninitialized() const override {
    return this->_uninitialized;
  }

  LifetimeDomain& lifetime() override { return this->_lifetime; }

  const LifetimeDomain& lifetime() const override { return this->_lifetime; }

private:
  /// \brief Return the offset variable associated to `p`
  VariableRef offset_var(VariableRef p) const {
    return this->_pointer.offset_var(p);
  }

  /// \brief Return the byte range for a given cell
  Interval cell_range(VariableRef cell) const {
    const MachineInt& offset = CellVariableTrait::offset(cell);
    const MachineInt& size = CellVariableTrait::size(cell);
    MachineInt one(1, offset.bit_width(), Unsigned);
    return Interval(offset, offset + (size - one));
  }

  /// \brief Return true if the given cell overlaps with the given byte range
  bool cell_overlap(VariableRef cell, const Interval& range) const {
    Interval meet = this->cell_range(cell).meet(range);
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
                          VariableRef offset,
                          const MachineInt& size) const {
    const MachineInt& cell_offset = CellVariableTrait::offset(cell);
    const MachineInt& cell_size = CellVariableTrait::size(cell);
    MachineInt one(1, size.bit_width(), Unsigned);

    if (size != cell_size) {
      return false;
    }

    IntervalCongruence offset_ic =
        this->integers().to_interval_congruence(offset);

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

    offset_ic.meet_with(IntervalCongruence(Interval(lb, ub)));

    return offset_ic == IntervalCongruence(cell_offset);
  }

  /// \brief Get or create the cell with the given base address, offset and size
  ///
  /// If a new cell is created, it will have the given signedness
  VariableRef cell(VariableFactory& vfac,
                   MemoryLocationRef base,
                   const MachineInt& offset,
                   const MachineInt& size,
                   Signedness sign) {
    ikos_assert(offset.sign() == Unsigned);
    ikos_assert(size.sign() == Unsigned);
    ikos_assert(size.is_strictly_positive());
    VariableRef c = CellFactoryTrait::cell(vfac, base, offset, size, sign);
    ikos_assert(MemVariableTrait::is_cell(c));
    ikos_assert(MachIntVariableTrait::bit_width(c) ==
                size.to< unsigned >() * 8);
    return c;
  }

  /// \brief Create a new cell for a write, performing reduction if possible
  VariableRef write_realize_single_cell(VariableFactory& vfac,
                                        MemoryLocationRef base,
                                        const MachineInt& offset,
                                        const MachineInt& size,
                                        Signedness sign) {
    VariableRef new_cell = this->cell(vfac, base, offset, size, sign);
    const CellSetT& cells = this->_cells.get(base);

    if (cells.is_empty()) {
      // no cell found for the base address
      this->_cells.set(base, CellSetT{new_cell});
      return new_cell;
    }

    CellSetT new_cells = cells;
    bool found = false;

    // remove overlapping cells
    for (VariableRef cell : cells) {
      if (cell == new_cell) {
        found = true;
      } else if (this->cell_overlap(cell, new_cell)) {
        this->forget_surface_cell(cell);
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
  std::vector< VariableRef > write_realize_range_cells(MemoryLocationRef base,
                                                       VariableRef offset,
                                                       const MachineInt& size) {
    // offset interval
    Interval offset_intv = this->integers().to_interval(offset);
    Interval size_intv(MachineInt::zero(size.bit_width(), Unsigned),
                       size - MachineInt(1, size.bit_width(), Unsigned));
    Interval range = add(offset_intv, size_intv);

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
          // that cell has only one way to be affected by the write statement
          updated_cells.push_back(cell);
        } else {
          this->forget_surface_cell(cell);
          new_cells.remove(cell);
        }
      }
    }

    this->_cells.set(base, new_cells);
    return updated_cells;
  }

  /// \brief Create a new cell for a read
  VariableRef read_realize_single_cell(VariableFactory& vfac,
                                       MemoryLocationRef base,
                                       const MachineInt& offset,
                                       const MachineInt& size,
                                       Signedness sign) {
    VariableRef new_cell = this->cell(vfac, base, offset, size, sign);
    CellSetT cells = this->_cells.get(base);
    cells.add(new_cell);
    this->_cells.set(base, cells);

    // TODO(marthaud): perform further reduction in case of partial overlaps
    return new_cell;
  }

  /// \brief Assignment `var = literal`
  class LiteralWriter : public LiteralT::template Visitor<> {
  private:
    VariableRef _lhs; // cell variable
    MachineIntDomain& _integer;
    PointerDomain& _pointer;
    UninitializedDomain& _uninitialized;

  public:
    LiteralWriter(VariableRef lhs,
                  PointerDomain& pointer,
                  UninitializedDomain& uninitialized)
        : _lhs(lhs),
          _integer(pointer.integers()),
          _pointer(pointer),
          _uninitialized(uninitialized) {}

    void machine_int(const MachineInt& rhs) {
      if (MachIntVariableTrait::bit_width(_lhs) == rhs.bit_width()) {
        Signedness lhs_sign = MachIntVariableTrait::sign(_lhs);
        if (lhs_sign == rhs.sign()) {
          _integer.assign(_lhs, rhs);
        } else {
          _integer.assign(_lhs, rhs.sign_cast(lhs_sign));
        }
      } else {
        _integer.forget(_lhs);
      }
      _pointer.forget(_lhs);
      _uninitialized.assign_initialized(_lhs);
    }

    void floating_point(const DummyNumber&) {
      _integer.forget(_lhs);
      _pointer.forget(_lhs);
      _uninitialized.assign_initialized(_lhs);
    }

    void memory_location(MemoryLocationRef addr) {
      _integer.forget(_lhs);
      _pointer.assign_address(_lhs, addr, Nullity::non_null());
      _uninitialized.assign_initialized(_lhs);
    }

    void null() {
      _integer.forget(_lhs);
      _pointer.assign_null(_lhs);
      _uninitialized.assign_initialized(_lhs);
    }

    void undefined() {
      _integer.forget(_lhs);
      _pointer.assign_undef(_lhs);
      _uninitialized.assign_uninitialized(_lhs);
    }

    void machine_int_var(VariableRef rhs) {
      if (MachIntVariableTrait::bit_width(_lhs) ==
          MachIntVariableTrait::bit_width(rhs)) {
        if (MachIntVariableTrait::sign(_lhs) ==
            MachIntVariableTrait::sign(rhs)) {
          _integer.assign(_lhs, rhs);
        } else {
          _integer.apply(machine_int::UnaryOperator::SignCast, _lhs, rhs);
        }
      } else {
        _integer.forget(_lhs);
      }
      _pointer.forget(_lhs);
      _uninitialized.assign(_lhs, rhs);
    }

    void floating_point_var(VariableRef rhs) {
      _integer.forget(_lhs);
      _pointer.forget(_lhs);
      _uninitialized.assign(_lhs, rhs);
    }

    void pointer_var(VariableRef rhs) {
      _integer.forget(_lhs);
      _pointer.assign(_lhs, rhs);
      _uninitialized.assign(_lhs, rhs);
    }

  }; // end class LiteralWriter

  /// \brief Assignment `literal = var`
  class LiteralReader : public LiteralT::template Visitor<> {
  private:
    VariableRef _rhs; // cell variable
    MachineIntDomain& _integer;
    NullityDomain& _nullity;
    PointerDomain& _pointer;
    UninitializedDomain& _uninitialized;

  public:
    LiteralReader(VariableRef rhs,
                  PointerDomain& pointer,
                  UninitializedDomain& uninitialized)
        : _rhs(rhs),
          _integer(pointer.integers()),
          _nullity(pointer.nullity()),
          _pointer(pointer),
          _uninitialized(uninitialized) {}

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
      // in case the rhs is a null pointer, assign the integer to zero
      // (implicit cast from pointer to int)
      if (_nullity.is_null(_rhs)) {
        _integer.assign(lhs,
                        MachineInt::zero(MachIntVariableTrait::bit_width(lhs),
                                         MachIntVariableTrait::sign(lhs)));
      } else if (MachIntVariableTrait::bit_width(lhs) ==
                 MachIntVariableTrait::bit_width(_rhs)) {
        if (MachIntVariableTrait::sign(lhs) ==
            MachIntVariableTrait::sign(_rhs)) {
          _integer.assign(lhs, _rhs);
        } else {
          _integer.apply(machine_int::UnaryOperator::SignCast, lhs, _rhs);
        }
      } else {
        _integer.forget(lhs);
      }
      _uninitialized.assign(lhs, _rhs);
    }

    void floating_point_var(VariableRef lhs) {
      _uninitialized.assign(lhs, _rhs);
    }

    void pointer_var(VariableRef lhs) {
      // in case the rhs is the integer zero, assign the pointer to null
      // (implicit cast from int to pointer)
      if (_integer.to_interval(_rhs).is_zero()) {
        _pointer.assign_null(lhs);
      } else {
        _pointer.assign(lhs, _rhs);
      }
      _uninitialized.assign(lhs, _rhs);
    }

  }; // end class LiteralReader

  /// \brief Perform a strong update `lhs = rhs`
  void strong_update(VariableRef lhs, const LiteralT& rhs) {
    LiteralWriter v(lhs, this->_pointer, this->_uninitialized);
    rhs.apply_visitor(v);
  }

  /// \brief Perform a weak update `lhs = rhs`
  void weak_update(VariableRef lhs, const LiteralT& rhs) {
    PointerDomain pointer_inv(this->_pointer);
    UninitializedDomain uninitialized_inv(this->_uninitialized);

    LiteralWriter v(lhs, pointer_inv, uninitialized_inv);
    rhs.apply_visitor(v);

    this->_pointer.join_with(pointer_inv);
    this->_uninitialized.join_with(uninitialized_inv);
  }

  /// \brief Perform a strong update `lhs = rhs`
  void strong_update(const LiteralT& lhs, VariableRef rhs) {
    LiteralReader v(rhs, this->_pointer, this->_uninitialized);
    lhs.apply_visitor(v);
  }

  /// \brief Perform a weak update `lhs = rhs`
  void weak_update(const LiteralT& lhs, VariableRef rhs) {
    PointerDomain pointer_inv(this->_pointer);
    UninitializedDomain uninitialized_inv(this->_uninitialized);

    LiteralReader v(rhs, pointer_inv, uninitialized_inv);
    lhs.apply_visitor(v);

    this->_pointer.join_with(pointer_inv);
    this->_uninitialized.join_with(uninitialized_inv);
  }

  /// \brief Preferred signedness for a cell created from a literal
  ///
  /// For machine integers, return the integer sign
  /// For pointers, return Unsigned
  /// By default, return Signed
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

public:
  void mem_write(VariableFactory& vfac,
                 VariableRef ptr,
                 const LiteralT& rhs,
                 const MachineInt& size) override {
    if (this->is_bottom()) {
      return;
    }

    if (this->nullity().is_null(ptr) ||
        this->uninitialized().is_uninitialized(ptr)) {
      // null/undefined dereference
      this->set_to_bottom();
      return;
    }

    // memory locations pointed by the pointer
    PointsToSetT addrs = this->_pointer.points_to(ptr);

    if (addrs.is_empty()) {
      // invalid dereference
      this->set_to_bottom();
      return;
    }

    if (size.is_zero()) {
      // does nothing
      return;
    }

    if (addrs.is_top()) {
      this->forget_mem(); // very conservative, but sound
      return;
    }

    // Update synthetic cells

    // offset interval
    Interval offset_intv = this->integers().to_interval(this->offset_var(ptr));
    ikos_assert(offset_intv.sign() == Unsigned);

    if (offset_intv.singleton()) {
      // The offset has one possible value.
      //
      // We can perform the usual reduction and update.
      const MachineInt& offset = *offset_intv.singleton();
      Signedness sign = this->preferred_cell_sign(rhs);

      for (MemoryLocationRef addr : addrs) {
        VariableRef cell =
            this->write_realize_single_cell(vfac, addr, offset, size, sign);

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
            this->write_realize_range_cells(addr, this->offset_var(ptr), size);
        for (VariableRef cell : cells) {
          this->weak_update(cell, rhs);
        }
      }
    }

    // Update pointer sets

    PointerAbsValueT rhs_ptr;
    if (rhs.is_memory_location()) {
      rhs_ptr =
          PointerAbsValueT(PointsToSetT{rhs.memory_location()},
                           Interval(MachineInt::zero(offset_intv.bit_width(),
                                                     Unsigned)),
                           Nullity::non_null(),
                           Uninitialized::initialized());
    } else if (rhs.is_pointer_var()) {
      rhs_ptr = this->_pointer.get(rhs.var());
    } else {
      // right hand side is not a pointer, nothing else to do
      return;
    }

    for (MemoryLocationRef addr : addrs) {
      PointerSetT pointer_set =
          this->_pointer_sets.get(addr, offset_intv.bit_width(), Unsigned);
      pointer_set.add(rhs_ptr);
      this->_pointer_sets.set(addr, pointer_set);
    }
  }

  void mem_read(VariableFactory& vfac,
                const LiteralT& lhs,
                VariableRef ptr,
                const MachineInt& size) override {
    ikos_assert(lhs.is_var());
    ikos_assert(size.is_strictly_positive());

    if (this->is_bottom()) {
      return;
    }

    if (this->nullity().is_null(ptr) ||
        this->uninitialized().is_uninitialized(ptr)) {
      // null/undefined dereference
      this->set_to_bottom();
      return;
    }

    // memory locations pointed by the pointer
    PointsToSetT addrs = this->_pointer.points_to(ptr);

    if (addrs.is_empty()) {
      // invalid dereference
      this->set_to_bottom();
      return;
    }

    if (addrs.is_top()) {
      this->forget_surface(lhs.var());
      return;
    }

    // Handle synthetic cells

    // offset interval
    Interval offset_intv = this->integers().to_interval(this->offset_var(ptr));
    ikos_assert(offset_intv.sign() == Unsigned);

    if (offset_intv.singleton()) {
      // The offset has one possible value.
      //
      // We can perform the usual reduction and update.
      const MachineInt& offset = *offset_intv.singleton();
      Signedness sign = this->preferred_cell_sign(lhs);
      bool first = true;

      for (MemoryLocationRef addr : addrs) {
        VariableRef cell =
            this->read_realize_single_cell(vfac, addr, offset, size, sign);

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
      this->forget_surface(lhs.var());
    }

    // Handle pointer sets

    if (lhs.is_pointer_var()) {
      PointerSetT pointer_set =
          PointerSetT::bottom(offset_intv.bit_width(), Unsigned);

      for (MemoryLocationRef addr : addrs) {
        pointer_set.join_with(
            this->_pointer_sets.get(addr, offset_intv.bit_width(), Unsigned));
      }

      this->_pointer.refine(lhs.var(), pointer_set);
    }
  }

  void mem_copy(VariableFactory& vfac,
                VariableRef dest,
                VariableRef src,
                const LiteralT& size) override {
    if (this->is_bottom()) {
      return;
    }

    if (this->nullity().is_null(dest) ||
        this->uninitialized().is_uninitialized(dest) ||
        this->nullity().is_null(src) ||
        this->uninitialized().is_uninitialized(src)) {
      // null/undefined dereference
      this->set_to_bottom();
      return;
    }

    // memory locations pointed by dest and src
    PointsToSetT src_addrs = this->_pointer.points_to(src);
    PointsToSetT dest_addrs = this->_pointer.points_to(dest);

    if (src_addrs.is_empty() || dest_addrs.is_empty()) {
      // invalid dereference
      this->set_to_bottom();
      return;
    }

    if (dest_addrs.is_top()) {
      this->forget_mem(); // very conservative, but sound
      return;
    }

    // Update synthetic cells

    // offsets and size intervals
    Interval src_intv = this->integers().to_interval(this->offset_var(src));
    Interval dest_intv = this->integers().to_interval(this->offset_var(dest));
    Interval size_intv;

    if (size.is_machine_int()) {
      size_intv = Interval(size.machine_int());
    } else if (size.is_machine_int_var()) {
      size_intv = this->integers().to_interval(size.var());
    } else {
      ikos_unreachable("unexpected literal for size");
    }
    assert_compatible(size_intv, src_intv);

    if (size_intv.ub().is_zero()) {
      return; // does nothing
    }

    // to be sound, remove all reachable cells
    for (MemoryLocationRef addr : dest_addrs) {
      this->forget_cells(addr, this->offset_var(dest), size_intv.ub());
    }

    if (dest_addrs.singleton() && dest_intv.singleton() &&
        !src_addrs.is_top() && src_intv.singleton() &&
        !size_intv.lb().is_zero()) {
      // in that case, we can be more precise
      MemoryLocationRef dest_addr = *dest_addrs.singleton();
      const MachineInt& dest_offset = *dest_intv.singleton();
      const MachineInt& src_offset = *src_intv.singleton();
      const MachineInt& size_lb = size_intv.lb();
      MachineInt one(1, dest_intv.bit_width(), Unsigned);
      Interval src_range(src_offset, src_offset + (size_lb - one));

      ValueDomain prev(*this);
      bool first = true;

      for (MemoryLocationRef src_addr : src_addrs) {
        // copy from src_addr/src_offset to dest_addr/dest_offset
        ValueDomain inv(prev);
        const CellSetT& src_cells = inv._cells.get(src_addr);

        if (!src_cells.is_empty()) {
          CellSetT dest_cells = inv._cells.get(dest_addr);

          for (VariableRef cell : src_cells) {
            if (this->cell_range(cell).leq(src_range)) {
              VariableRef new_cell =
                  this->cell(vfac,
                             dest_addr,
                             dest_offset +
                                 (CellVariableTrait::offset(cell) - src_offset),
                             CellVariableTrait::size(cell),
                             MachIntVariableTrait::sign(cell));
              dest_cells.add(new_cell);
              inv.integers().assign(new_cell, cell);
              inv.pointers().assign(new_cell, cell);
              inv.uninitialized().assign(new_cell, cell);
            }
          }

          inv._cells.set(dest_addr, dest_cells);
        }

        if (first) {
          this->operator=(std::move(inv));
          first = false;
        } else {
          this->join_with(inv);
        }
      }
    }

    // Update pointer sets

    // Collect source pointer sets
    PointerSetT src_pointer_set =
        PointerSetT::bottom(src_intv.bit_width(), Unsigned);

    if (src_addrs.is_top()) {
      src_pointer_set.set_to_top(); // sound
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

  void mem_set(VariableFactory& /*vfac*/,
               VariableRef dest,
               const LiteralT& value,
               const LiteralT& size) override {
    if (this->is_bottom()) {
      return;
    }

    if (this->nullity().is_null(dest) ||
        this->uninitialized().is_uninitialized(dest)) {
      // null/undefined dereference
      this->set_to_bottom();
      return;
    }

    // memory locations pointed by dest
    PointsToSetT addrs = this->_pointer.points_to(dest);

    if (addrs.is_empty()) {
      // invalid dereference
      this->set_to_bottom();
      return;
    }

    if (addrs.is_top()) {
      this->forget_cells(); // very conservative, but sound
      return;
    }

    // Update synthetic cells

    // offset, size and value intervals
    Interval dest_intv = this->integers().to_interval(this->offset_var(dest));
    Interval size_intv, value_intv;

    if (size.is_machine_int()) {
      size_intv = Interval(size.machine_int());
    } else if (size.is_machine_int_var()) {
      size_intv = this->integers().to_interval(size.var());
    } else {
      ikos_unreachable("unexpected literal for size");
    }
    assert_compatible(size_intv, dest_intv);

    if (value.is_machine_int()) {
      value_intv = Interval(value.machine_int());
    } else if (size.is_machine_int_var()) {
      value_intv = this->integers().to_interval(value.var());
    } else {
      ikos_unreachable("unexpected literal for value");
    }

    if (size_intv.ub().is_zero()) {
      return; // does nothing
    }

    if (value_intv.is_zero()) {
      // memory set to zero
      const MachineInt& size_lb = size_intv.lb();
      MachineInt zero(0, size_lb.bit_width(), Unsigned);
      MachineInt one(1, size_lb.bit_width(), Unsigned);

      // offsets that are updated
      Interval safe_range_lb(dest_intv.lb(), dest_intv.lb() + (size_lb - one));
      Interval safe_range_ub(dest_intv.ub(), dest_intv.ub() + (size_lb - one));
      Interval safe_range = safe_range_lb.meet(safe_range_ub);

      // possibly updated offsets
      Interval unsafe_range =
          add(dest_intv, Interval(zero, size_intv.ub() - one));

      for (MemoryLocationRef addr : addrs) {
        const CellSetT& cells = this->_cells.get(addr);

        if (!cells.is_empty()) {
          CellSetT new_cells = cells;

          for (VariableRef cell : cells) {
            Interval range = this->cell_range(cell);

            if (range.leq(safe_range)) {
              LiteralT zero_lit = LiteralT::machine_int(
                  MachineInt(0,
                             MachIntVariableTrait::bit_width(cell),
                             MachIntVariableTrait::sign(cell)));
              if (addrs.singleton()) {
                this->strong_update(cell, zero_lit);
              } else {
                this->weak_update(cell, zero_lit);
              }
            } else if (range.leq(unsafe_range)) {
              this->forget_surface_cell(cell);
              new_cells.remove(cell);
            }
          }

          this->_cells.set(addr, new_cells);
        }
      }
    } else {
      // to be sound, remove all reachable cells
      for (MemoryLocationRef addr : addrs) {
        this->forget_cells(addr, this->offset_var(dest), size_intv.ub());
      }
    }

    // Nothing to do for pointer sets
  }

  void forget_surface(VariableRef x) override {
    if (MemVariableTrait::is_cell(x)) {
      this->forget_surface_cell(x);
    } else if (MemVariableTrait::is_int(x)) {
      this->integers().forget(x);
      this->uninitialized().forget(x);
    } else if (MemVariableTrait::is_float(x)) {
      this->uninitialized().forget(x);
    } else if (MemVariableTrait::is_pointer(x)) {
      this->pointers().forget(x);
      this->uninitialized().forget(x);
    } else {
      ikos_unreachable("unexpected type");
    }
  }

private:
  /// \brief Forget a cell variable
  void forget_surface_cell(VariableRef x) {
    ikos_assert(MemVariableTrait::is_cell(x));
    this->integers().forget(x);
    this->pointers().forget(x);
    this->uninitialized().forget(x);
  }

  /// \brief Forget all synthetic cells
  void forget_cells() {
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
        this->forget_surface_cell(cell);
      }
    }

    this->_cells.set_to_top();
  }

  /// \brief Forget the synthetic cells for the given memory location
  void forget_cells(MemoryLocationRef addr) {
    const CellSetT& cells = this->_cells.get(addr);

    if (cells.is_bottom()) {
      return;
    }

    for (VariableRef cell : cells) {
      this->forget_surface_cell(cell);
    }

    this->_cells.forget(addr);
  }

  /// \brief Forget the synthetic cells in range
  /// `[addr + offset, addr + offset + size - 1]`
  void forget_cells(MemoryLocationRef addr,
                    VariableRef offset,
                    const MachineInt& size) {
    if (this->is_bottom()) {
      return;
    }

    if (size.is_zero()) {
      return;
    }
    if (size.is_max()) {
      this->forget_mem(addr);
      return;
    }

    Interval offset_intv = this->integers().to_interval(offset);
    Interval size_intv(MachineInt::zero(size.bit_width(), Unsigned),
                       size - MachineInt(1, size.bit_width(), Unsigned));
    this->forget_cells(addr, add(offset_intv, size_intv));
  }

  /// \brief Forget the synthetic cells in
  /// `[addr + range.lb(), addr + range.ub()]`
  void forget_cells(MemoryLocationRef addr, const Interval& range) {
    ikos_assert(!range.is_bottom());

    const CellSetT& cells = this->_cells.get(addr);
    CellSetT new_cells = cells;

    if (cells.is_empty()) {
      return;
    }

    for (VariableRef cell : cells) {
      if (cell_overlap(cell, range)) {
        this->forget_surface_cell(cell);
        new_cells.remove(cell);
      }
    }

    this->_cells.set(addr, new_cells);
  }

  /// \brief Forget all pointer sets
  void forget_pointer_sets() { this->_pointer_sets.set_to_top(); }

  /// \brief Forget the pointer set for the given memory location
  void forget_pointer_set(MemoryLocationRef addr) {
    this->_pointer_sets.forget(addr);
  }

public:
  void forget_mem() override {
    this->forget_cells();
    this->forget_pointer_sets();
  }

  void forget_mem(MemoryLocationRef addr) override {
    this->forget_cells(addr);
    this->forget_pointer_set(addr);
  }

  void forget_mem(MemoryLocationRef addr,
                  VariableRef offset,
                  const MachineInt& size) override {
    if (this->is_bottom()) {
      return;
    }

    if (size.is_zero()) {
      return;
    }

    this->forget_cells(addr, offset, size);
    this->forget_pointer_set(addr);
  }

  void forget_mem(MemoryLocationRef addr, const Interval& range) override {
    this->forget_cells(addr, range);
    this->forget_pointer_set(addr);
  }

  void forget_reachable_mem(VariableRef p) override {
    if (this->is_bottom()) {
      return;
    }

    if (this->nullity().is_null(p)) {
      return;
    }

    // memory locations pointed by the pointer
    PointsToSetT addrs = this->_pointer.points_to(p);

    if (addrs.is_top()) {
      this->forget_mem(); // very conservative, but sound
      return;
    }

    for (MemoryLocationRef addr : addrs) {
      this->forget_mem(addr);
    }
  }

  void forget_reachable_mem(VariableRef p, const MachineInt& size) override {
    if (this->is_bottom()) {
      return;
    }

    if (size.is_zero()) {
      return;
    }

    if (this->nullity().is_null(p)) {
      return;
    }

    // memory locations pointed by the pointer
    PointsToSetT addrs = this->_pointer.points_to(p);

    if (addrs.is_top()) {
      this->forget_mem(); // very conservative, but sound
      return;
    }

    for (MemoryLocationRef addr : addrs) {
      this->forget_mem(addr, this->offset_var(p), size);
    }
  }

  void abstract_reachable_mem(VariableRef p) override {
    if (this->is_bottom()) {
      return;
    }

    if (this->nullity().is_null(p)) {
      return;
    }

    // memory locations pointed by the pointer
    PointsToSetT addrs = this->_pointer.points_to(p);

    if (addrs.is_top()) {
      this->forget_cells(); // very conservative, but sound
      return;
    }

    for (MemoryLocationRef addr : addrs) {
      this->forget_cells(addr);
    }
  }

  void abstract_reachable_mem(VariableRef p, const MachineInt& size) override {
    if (this->is_bottom()) {
      return;
    }

    if (size.is_zero()) {
      return;
    }

    if (this->nullity().is_null(p)) {
      return;
    }

    // memory locations pointed by the pointer
    PointsToSetT addrs = this->_pointer.points_to(p);

    if (addrs.is_top()) {
      this->forget_cells(); // very conservative, but sound
      return;
    }

    for (MemoryLocationRef addr : addrs) {
      this->forget_cells(addr, this->offset_var(p), size);
    }
  }

  void zero_reachable_mem(VariableRef p) override {
    this->uninitialize_reachable_mem(p);
  }

  void uninitialize_reachable_mem(VariableRef p) override {
    if (this->is_bottom()) {
      return;
    }

    if (this->nullity().is_null(p)) {
      return;
    }

    // memory locations pointed by the pointer
    PointsToSetT addrs = this->_pointer.points_to(p);

    if (addrs.is_top()) {
      this->forget_mem(); // very conservative, but sound
      return;
    }

    for (MemoryLocationRef addr : addrs) {
      this->forget_mem(addr);
    }

    if (auto addr = addrs.singleton()) {
      unsigned bit_width = MachIntVariableTrait::bit_width(this->offset_var(p));
      this->_pointer_sets.set(*addr, PointerSetT::empty(bit_width, Unsigned));
    }
  }

  void normalize() const override {
    // is_bottom() will normalize
    if (this->_cells.is_bottom() || this->_pointer_sets.is_bottom() ||
        this->_pointer.is_bottom() || this->_uninitialized.is_bottom() ||
        this->_lifetime.is_bottom()) {
      const_cast< ValueDomain* >(this)->set_to_bottom();
    }
  }

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else if (this->is_top()) {
      o << "T";
    } else {
      o << "(";
      this->_cells.dump(o);
      o << ", ";
      this->_pointer_sets.dump(o);
      o << ", ";
      this->_pointer.dump(o);
      o << ", ";
      this->_uninitialized.dump(o);
      o << ", ";
      this->_lifetime.dump(o);
      o << ")";
    }
  }

  static std::string name() {
    return "value domain with " + PointerDomain::name() + ", " +
           UninitializedDomain::name() + " and " + LifetimeDomain::name();
  }

}; // end class ValueDomain

} // end namespace memory
} // end namespace core
} // end namespace ikos
