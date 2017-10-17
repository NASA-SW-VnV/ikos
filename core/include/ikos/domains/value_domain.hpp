/**************************************************************************/ /**
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
 * Copyright (c) 2011-2017 United States Government as represented by the
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

#ifndef IKOS_VALUE_DOMAIN_HPP
#define IKOS_VALUE_DOMAIN_HPP

#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/domains/bitwise_operators_api.hpp>
#include <ikos/domains/counter_domains_api.hpp>
#include <ikos/domains/division_operators_api.hpp>
#include <ikos/domains/memory_domains_api.hpp>
#include <ikos/domains/nullity.hpp>
#include <ikos/domains/numerical_domains_api.hpp>
#include <ikos/domains/pointer.hpp>
#include <ikos/domains/separate_domains.hpp>
#include <ikos/domains/uninitialized.hpp>
#include <ikos/value/congruence.hpp>
#include <ikos/value/interval.hpp>

namespace ikos {

namespace var_name_traits {

template < typename VariableFactory, typename MemoryLocation, typename Number >
struct value_domain_cell_var {
  /// \returns the variable that represents the value at a given memory location
  typename VariableFactory::variable_name_t operator()(VariableFactory& vfac,
                                                       MemoryLocation addr,
                                                       Number offset,
                                                       Number size);
};

} // end namespace var_name_traits

/// \brief Memory abstract domain
///
/// Memory abstraction consisting of cells and a pointer abstraction
/// augmented with nullity and uninitialized variable information.
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
template < typename ScalarDomain,
           typename VariableFactory,
           typename MemoryLocation >
class value_domain
    : public abstract_domain,
      public numerical_domain< typename ScalarDomain::number_t,
                               typename ScalarDomain::variable_name_t >,
      public bitwise_operators< typename ScalarDomain::number_t,
                                typename ScalarDomain::variable_name_t >,
      public division_operators< typename ScalarDomain::number_t,
                                 typename ScalarDomain::variable_name_t >,
      public counter_domain< typename ScalarDomain::number_t,
                             typename ScalarDomain::variable_name_t >,
      public pointer_domain< typename ScalarDomain::number_t,
                             typename ScalarDomain::variable_name_t,
                             MemoryLocation >,
      public uninitialized_domain< typename ScalarDomain::variable_name_t >,
      public nullity_domain< typename ScalarDomain::variable_name_t >,
      public memory_domain< typename ScalarDomain::number_t,
                            dummy_number,
                            VariableFactory > {
private:
  typedef typename ScalarDomain::number_t Number;
  typedef typename ScalarDomain::variable_name_t VariableName;

public:
  typedef Number number_t;
  typedef Number integer_t;
  typedef VariableFactory variable_factory_t;
  typedef dummy_number floating_point_t;
  typedef VariableName variable_name_t;
  typedef MemoryLocation memory_location_t;
  typedef bound< Number > bound_t;
  typedef congruence< Number > congruence_t;
  typedef interval< Number > interval_t;
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;
  typedef discrete_domain< MemoryLocation > points_to_set_t;
  typedef literal< Number, dummy_number, VariableName > literal_t;
  typedef ScalarDomain scalar_domain_t;
  typedef value_domain< ScalarDomain, VariableFactory, MemoryLocation >
      value_domain_t;

private:
  typedef pointer_domain_impl< ScalarDomain, MemoryLocation > pointer_domain_t;
  typedef uninitialized_domain_impl< VariableName > uninitialized_domain_t;
  typedef nullity_domain_impl< VariableName > nullity_domain_t;

private:
  /// \brief Memory cell
  ///
  /// A cell is a triple <b, o, s> modelling all bytes at address b, starting at
  /// offset o up to (o + s - 1).
  ///
  /// Note that both the offset and the size are numbers.
  class Cell : public writeable {
  private:
    MemoryLocation _base;
    Number _offset;
    Number _size;

    // represents the cell in the pointer+scalar domain
    VariableName _scalar_var;

  public:
    Cell(VariableFactory& vfac, MemoryLocation base, Number offset, Number size)
        : _base(base), _offset(offset), _size(size) {
      assert(_offset >= 0 && _size >= 1);
      _scalar_var = var_name_traits::value_domain_cell_var< VariableFactory,
                                                            MemoryLocation,
                                                            Number >()(vfac,
                                                                       _base,
                                                                       _offset,
                                                                       _size);
    }

    MemoryLocation base() const { return _base; }
    Number offset() const { return _offset; }
    Number size() const { return _size; }
    VariableName scalar_var() const { return _scalar_var; }

    std::size_t index() const {
      return index_traits< VariableName >::index(_scalar_var);
    }

    bool operator==(const Cell& o) const {
      return _base == o._base && _offset == o._offset && _size == o._size;
    }

    interval_t range() const {
      return interval_t(bound_t(_offset),
                        bound_t(_offset) + bound_t(_size) - 1);
    }

    bool overlaps(interval_t range) const {
      interval_t meet = this->range() & range;
      return !meet.is_bottom();
    }

    bool overlaps(const Cell& c) const { return overlaps(c.range()); }

    /// \returns the number of possible overlaps between the cell and the write
    /// at offset `offset` of size `size`
    template < typename PointerDomain >
    bound_t number_overlaps(VariableName offset,
                            Number size,
                            PointerDomain inv) const {
      linear_constraint_system_t csts;
      csts += (variable_t(offset) + size - 1 >= _offset);
      csts += (variable_t(offset) <= _offset + _size - 1);
      inv += csts;

      interval_t rng = inv.to_interval(offset, true);
      if (rng.is_bottom()) {
        return 0;
      } else if (rng.ub().is_infinite()) {
        return bound_t::plus_infinity();
      } else {
        return *rng.ub().number() - *rng.lb().number() + 1;
      }
    }

    /// \returns true if the cell is a realization of the write at offset
    /// `offset` and size `size`
    template < typename PointerDomain >
    bool realizes(VariableName offset, Number size, PointerDomain inv) const {
      if (size != _size)
        return false;

      inv += (variable_t(offset) == _offset);
      return !inv.is_bottom();
    }

    void write(std::ostream& o) {
      o << "C{";
      index_traits< MemoryLocation >::write(o, _base);
      o << "," << _offset << "," << _size << "}";
    }

  }; // end class Cell

  /// \brief Cell set
  ///
  /// This is the discrete_domain but with a different semantic.
  class CellSet : public writeable {
  private:
    typedef patricia_tree_set< Cell > ptset_t;

  public:
    typedef typename ptset_t::iterator iterator;

  private:
    bool _is_top;
    ptset_t _set;

  private:
    CellSet(bool is_top) : _is_top(is_top) {}

    CellSet(ptset_t set) : _is_top(false), _set(set) {}

  public:
    static CellSet bottom() { return CellSet(false); }

    static CellSet top() { return CellSet(true); }

  public:
    CellSet() : _is_top(true) {}

    CellSet(const CellSet& other) : _is_top(other._is_top), _set(other._set) {}

    CellSet(const Cell& s) : _is_top(false), _set(s) {}

    bool is_top() { return this->_is_top; }

    bool is_bottom() { return !this->_is_top && this->_set.empty(); }

    bool operator<=(CellSet other) {
      return other._is_top || (!this->_is_top && this->_set <= other._set);
    }

    bool operator==(CellSet other) {
      return (this->_is_top && other._is_top) || (this->_set == other._set);
    }

    CellSet operator|(CellSet other) {
      if (this->_is_top || other._is_top) {
        return CellSet(true);
      } else {
        // only keep cells present on both sides
        return CellSet(this->_set & other._set);
      }
    }

    CellSet operator&(CellSet other) {
      if (this->is_bottom() || other.is_bottom()) {
        return CellSet(false);
      } else if (this->_is_top) {
        return other;
      } else if (other._is_top) {
        return *this;
      } else {
        // keep all cells
        return CellSet(this->_set | other._set);
      }
    }

    CellSet operator||(CellSet other) { return this->operator|(other); }

    CellSet operator&&(CellSet other) { return this->operator&(other); }

    CellSet& operator+=(const Cell& s) {
      if (!this->_is_top) {
        this->_set += s;
      }
      return *this;
    }

    CellSet operator+(const Cell& s) {
      CellSet r(*this);
      r.operator+=(s);
      return r;
    }

    CellSet& operator-=(const Cell& s) {
      if (!this->_is_top) {
        this->_set -= s;
      }
      return *this;
    }

    CellSet operator-(const Cell& s) {
      CellSet r(*this);
      r.operator-=(s);
      return r;
    }

    std::size_t size() {
      if (this->_is_top) {
        throw logic_error("cell set: trying to call size() on top");
      } else {
        return this->_set.size();
      }
    }

    iterator begin() {
      if (this->_is_top) {
        throw logic_error("cell set: trying to call begin() on top");
      } else {
        return this->_set.begin();
      }
    }

    iterator end() {
      if (this->_is_top) {
        throw logic_error("cell set: trying to call end() on top");
      } else {
        return this->_set.end();
      }
    }

    void write(std::ostream& o) {
      if (this->_is_top) {
        o << "{...}";
      } else if (this->_set.empty()) {
        o << "_|_";
      } else {
        o << this->_set;
      }
    }

  }; // end class CellSet

  // TODO: cleanup of cells from this->_cells for which we lost its
  // content in the pointer+numerical abstraction. E.g, joins can keep
  // cells for which we don't have any content.

private:
  typedef CellSet cell_set_t;
  typedef typename cell_set_t::iterator cell_set_iterator;
  typedef separate_domain< MemoryLocation, cell_set_t > var_to_cell_set_t;

private:
  // _cells is a map from a base pointer (memory objects) p to
  // a set of synthetic cells whose base is p.

  // TODO: it should be a map from pointer offsets to set of
  // overlapping cells to make operations faster.

  var_to_cell_set_t _cells; // memory contents
  pointer_domain_t _ptr;    // pointer+scalar domain (i.e., surface domain)
  uninitialized_domain_t _uninitialized;
  nullity_domain_t _nullity;

  value_domain(bool is_bottom)
      : _cells(is_bottom ? var_to_cell_set_t::bottom()
                         : var_to_cell_set_t::top()),
        _ptr(is_bottom ? pointer_domain_t::bottom() : pointer_domain_t::top()),
        _uninitialized(is_bottom ? uninitialized_domain_t::bottom()
                                 : uninitialized_domain_t::top()),
        _nullity(is_bottom ? nullity_domain_t::bottom()
                           : nullity_domain_t::top()) {}

  value_domain(const var_to_cell_set_t& cells,
               const pointer_domain_t& ptr_inv,
               const uninitialized_domain_t& uinitialized,
               const nullity_domain_t& nullity)
      : _cells(cells),
        _ptr(ptr_inv),
        _uninitialized(uinitialized),
        _nullity(nullity) {}

  void add_cell(const Cell& c) {
    cell_set_t cells = _cells[c.base()];

    if (cells.is_top()) {
      _cells.set(c.base(), cell_set_t(c));
    } else {
      cells += c;
      _cells.set(c.base(), cells);
    }
  }

  void remove_cell(const Cell& c) {
    // remove the cell from the memory abstraction
    cell_set_t cells = _cells[c.base()];
    cells -= c;

    if (cells.is_bottom()) {
      _cells -= c.base();
    } else {
      _cells.set(c.base(), cells);
    }

    // remove the cell from the pointer+scalar abstraction, nullity
    // abstraction and uninitialized variable abstraction
    _ptr -= c.scalar_var();
    _nullity -= c.scalar_var();
    _uninitialized -= c.scalar_var();
  }

  /// \brief Create a new cell for a write, performing reduction if possible
  Cell write_realize_single_cell(VariableFactory& vfac,
                                 MemoryLocation base,
                                 Number offset,
                                 Number size) {
    Cell c(vfac, base, offset, size);

    cell_set_t cells = _cells[base];
    if (cells.is_top()) { // no cell found for base
      add_cell(c);
    } else {
      bool found = false;

      for (cell_set_iterator it = cells.begin(); it != cells.end(); ++it) {
        if (*it == c) {
          found = true;
        } else if (c.overlaps(*it)) {
          remove_cell(*it);
        }
      }

      if (!found) {
        add_cell(c);
      }
    }

    return c;
  }

  /// \brief Perform a write with an approximated offset.
  ///
  /// \returns a list of cells on which we should perform a weak update.
  std::vector< Cell > write_realize_range_cells(MemoryLocation base,
                                                VariableName offset,
                                                Number size) {
    // get the interval
    interval_t offset_intv = _ptr.to_interval(offset, true);
    offset_intv = offset_intv & interval_t(0, bound_t::plus_infinity());
    interval_t range = interval_t(0, size - 1) + offset_intv;

    cell_set_t cells = _cells[base];
    if (cells.is_top()) { // no cell found for base
      return std::vector< Cell >();
    }

    std::vector< Cell > updated_cells;
    for (cell_set_iterator it = cells.begin(); it != cells.end(); ++it) {
      const Cell& c = *it;

      if (c.overlaps(range)) {
        if (c.number_overlaps(offset, size, _ptr) == 1 &&
            c.realizes(offset, size, _ptr)) {
          // that cell has only one way to be affected by the write statement
          updated_cells.push_back(c);
        } else {
          remove_cell(c);
        }
      }
    }

    return updated_cells;
  }

  /// \brief Create a new cell for a read
  Cell read_realize_single_cell(VariableFactory& vfac,
                                MemoryLocation base,
                                Number offset,
                                Number size) {
    Cell c(vfac, base, offset, size);

    cell_set_t cells = _cells[base];
    if (cells.is_top() || !(cell_set_t(c) <= cells)) {
      add_cell(c);
    }
    // TODO: perform further reduction in case of partial overlaps

    return c;
  }

  class literal_writer : public literal_t::template visitor<> {
  private:
    VariableName _lhs;
    pointer_domain_t& _ptr;
    uninitialized_domain_t& _uninitialized;
    nullity_domain_t& _nullity;

  public:
    literal_writer(VariableName lhs,
                   pointer_domain_t& ptr,
                   uninitialized_domain_t& uninitialized,
                   nullity_domain_t& nullity)
        : _lhs(lhs),
          _ptr(ptr),
          _uninitialized(uninitialized),
          _nullity(nullity) {}

    void integer(Number rhs) {
      _ptr.assign(_lhs, rhs);
      _ptr.forget_pointer(_lhs);
      _uninitialized.make_initialized(_lhs);
      _nullity -= _lhs;
    }

    void floating_point(dummy_number) {
      _ptr -= _lhs; // ignored
      _uninitialized.make_initialized(_lhs);
      _nullity -= _lhs;
    }

    void undefined() {
      _ptr -= _lhs;
      _uninitialized.make_uninitialized(_lhs);
      _nullity -= _lhs;
    }

    void null() {
      _ptr -= _lhs;
      _uninitialized.make_initialized(_lhs);
      _nullity.make_null(_lhs);
    }

    void integer_var(VariableName rhs) {
      _ptr.assign(_lhs, variable_t(rhs));
      _ptr.forget_pointer(_lhs);
      _uninitialized.assign_uninitialized(_lhs, rhs);
      _nullity -= _lhs;
    }

    void floating_point_var(VariableName rhs) {
      _ptr -= _lhs; // ignored
      _uninitialized.assign_uninitialized(_lhs, rhs);
      _nullity -= _lhs;
    }

    void pointer_var(VariableName rhs) {
      _ptr.assign_pointer(_lhs, rhs);
      _ptr.forget_num(_lhs);
      _uninitialized.assign_uninitialized(_lhs, rhs);
      _nullity.assign_nullity(_lhs, rhs);
    }

  }; // end class literal_writer

  class literal_reader : public literal_t::template visitor<> {
  private:
    VariableName _rhs;
    pointer_domain_t& _ptr;
    uninitialized_domain_t& _uninitialized;
    nullity_domain_t& _nullity;

  public:
    literal_reader(VariableName rhs,
                   pointer_domain_t& ptr,
                   uninitialized_domain_t& uninitialized,
                   nullity_domain_t& nullity)
        : _rhs(rhs),
          _ptr(ptr),
          _uninitialized(uninitialized),
          _nullity(nullity) {}

    void integer(Number v) {
      throw logic_error("value_domain: trying to assign an integer");
    }

    void floating_point(dummy_number) {
      throw logic_error("value_domain: trying to assign a floating point");
    }

    void undefined() {
      throw logic_error("value_domain: trying to assign undefined");
    }

    void null() { throw logic_error("value_domain: trying to assign null"); }

    void integer_var(VariableName lhs) {
      // in case the rhs is a null pointer, assign the integer to zero
      // (implicit cast from pointer to int)
      if (_nullity.is_null(_rhs)) {
        _ptr.assign(lhs, Number(0));
      } else {
        _ptr.assign(lhs, variable_t(_rhs));
      }
      _ptr.forget_pointer(lhs);
      _uninitialized.assign_uninitialized(lhs, _rhs);
      _nullity -= lhs;
    }

    void floating_point_var(VariableName lhs) {
      _ptr -= lhs; // ignored
      _uninitialized.assign_uninitialized(lhs, _rhs);
      _nullity -= lhs;
    }

    void pointer_var(VariableName lhs) {
      // in case the rhs is the integer zero, assign the pointer to null
      // (implicit cast from int to pointer)
      if (_ptr.to_interval(_rhs, false) == interval_t(Number(0))) {
        _ptr.forget_pointer(lhs);
        _nullity.make_null(lhs);
      } else {
        _ptr.assign_pointer(lhs, _rhs);
        _nullity.assign_nullity(lhs, _rhs);
      }
      _ptr.forget_num(lhs);
      _uninitialized.assign_uninitialized(lhs, _rhs);
    }

  }; // end class literal_reader

  /// \brief Perform a strong update `lhs = rhs`
  void strong_update(VariableName lhs, literal_t rhs) {
    literal_writer v(lhs, _ptr, _uninitialized, _nullity);
    rhs.apply_visitor(v);
  }

  /// \brief Perform a strong update `lhs = rhs`
  void strong_update(literal_t lhs, VariableName rhs) {
    literal_reader v(rhs, _ptr, _uninitialized, _nullity);
    lhs.apply_visitor(v);
  }

  /// \brief Perform a weak update `lhs = rhs`
  void weak_update(VariableName lhs, literal_t rhs) {
    pointer_domain_t ptr_inv(_ptr);
    uninitialized_domain_t uninitialized_inv(_uninitialized);
    nullity_domain_t nullity_inv(_nullity);

    literal_writer v(lhs, ptr_inv, uninitialized_inv, nullity_inv);
    rhs.apply_visitor(v);

    _ptr = _ptr | ptr_inv;
    _uninitialized = _uninitialized | uninitialized_inv;
    _nullity = _nullity | nullity_inv;
  }

  /// \brief Perform a weak update `lhs = rhs`
  void weak_update(literal_t lhs, VariableName rhs) {
    pointer_domain_t ptr_inv(_ptr);
    uninitialized_domain_t uninitialized_inv(_uninitialized);
    nullity_domain_t nullity_inv(_nullity);

    literal_reader v(rhs, ptr_inv, uninitialized_inv, nullity_inv);
    lhs.apply_visitor(v);

    _ptr = _ptr | ptr_inv;
    _uninitialized = _uninitialized | uninitialized_inv;
    _nullity = _nullity | nullity_inv;
  }

  /// \brief Forget all memory cells that can be accessible through pointer p
  void forget_reachable_cells(VariableName p) {
    if (is_bottom())
      return;

    if (_nullity.is_null(p) || _uninitialized.is_uninitialized(p))
      return;

    points_to_set_t addrs_set = _ptr.addrs_set(p); // p's base address

    if (addrs_set.is_top())
      return;

    for (typename points_to_set_t::iterator it = addrs_set.begin();
         it != addrs_set.end();
         ++it) {
      forget_cells(*it);
    }
  }

  /// \brief Forget all memory cells that can be accessible through pointer p
  /// and that overlap with [p.offset, ..., p.offset + size - 1]
  void forget_reachable_cells(VariableName p, interval_t size) {
    if (is_bottom())
      return;

    if (_nullity.is_null(p) || _uninitialized.is_uninitialized(p))
      return;

    points_to_set_t addrs_set = _ptr.addrs_set(p);

    if (addrs_set.is_top())
      return;

    for (typename points_to_set_t::iterator it = addrs_set.begin();
         it != addrs_set.end();
         ++it) {
      forget_cells(*it, offset_var(p), size);
    }
  }

  /// \brief Forget all memory cells
  void forget_cells() {
    if (is_bottom())
      return;

    for (auto b_it = _cells.begin(); b_it != _cells.end(); ++b_it) {
      cell_set_t cells = b_it->second;

      if (cells.is_top())
        continue;

      for (auto c_it = cells.begin(); c_it != cells.end(); ++c_it) {
        _ptr -= c_it->scalar_var();
        _nullity -= c_it->scalar_var();
        _uninitialized -= c_it->scalar_var();
      }
    }

    _cells = var_to_cell_set_t::top();
  }

  /// \brief Forget all memory cells of base address `base`
  void forget_cells(MemoryLocation base) {
    cell_set_t cells = _cells[base];

    if (cells.is_top())
      return;

    for (cell_set_iterator it = cells.begin(); it != cells.end(); ++it) {
      _ptr -= it->scalar_var();
      _nullity -= it->scalar_var();
      _uninitialized -= it->scalar_var();
    }

    _cells -= base;
  }

  /// \brief Forget all memory cells of base address `base` that overlap
  /// with [offset, ..., offset + size - 1]
  void forget_cells(MemoryLocation base, VariableName offset, interval_t size) {
    interval_t offset_intv = _ptr.to_interval(offset, true) &
                             interval_t(0, bound_t::plus_infinity());
    size = size & interval_t(1, bound_t::plus_infinity());
    interval_t range = offset_intv + interval_t(0, size.ub() - 1);
    forget_cells(base, range);
  }

  /// \brief Forget all memory cells of base address `base` that overlap with a
  /// given range
  void forget_cells(MemoryLocation base, interval_t range) {
    cell_set_t cells = _cells[base];

    if (cells.is_top())
      return;

    for (cell_set_iterator it = cells.begin(); it != cells.end(); ++it) {
      const Cell& c = *it;

      if (c.overlaps(range)) {
        remove_cell(c);
      }
    }
  }

public:
  static value_domain_t top() { return value_domain(false); }

  static value_domain_t bottom() { return value_domain(true); }

  value_domain()
      : _cells(var_to_cell_set_t::top()),
        _ptr(pointer_domain_t::top()),
        _uninitialized(uninitialized_domain_t::top()),
        _nullity(nullity_domain_t::top()) {}

  value_domain(const value_domain_t& o)
      : _cells(o._cells),
        _ptr(o._ptr),
        _uninitialized(o._uninitialized),
        _nullity(o._nullity) {}

  value_domain_t& operator=(const value_domain_t& o) {
    _cells = o._cells;
    _ptr = o._ptr;
    _uninitialized = o._uninitialized;
    _nullity = o._nullity;
    return *this;
  }

  bool is_bottom() {
    if (_cells.is_bottom() || _ptr.is_bottom() || _uninitialized.is_bottom() ||
        _nullity.is_bottom()) {
      *this = bottom();
      return true;
    } else {
      return false;
    }
  }

  bool is_top() {
    return _cells.is_top() && _ptr.is_top() && _uninitialized.is_top() &&
           _nullity.is_top();
  }

  bool operator<=(value_domain_t o) {
    if (is_bottom()) {
      return true;
    } else if (o.is_bottom()) {
      return false;
    } else {
      return _cells <= o._cells && _ptr <= o._ptr &&
             _uninitialized <= o._uninitialized && _nullity <= o._nullity;
    }
  }

  value_domain_t operator|(value_domain_t o) {
    if (is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      return value_domain_t(_cells | o._cells,
                            _ptr | o._ptr,
                            _uninitialized | o._uninitialized,
                            _nullity | o._nullity);
    }
  }

  value_domain_t operator||(value_domain_t o) {
    if (is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      return value_domain_t(_cells | o._cells,
                            _ptr || o._ptr,
                            _uninitialized || o._uninitialized,
                            _nullity || o._nullity);
    }
  }

  value_domain_t join_loop(value_domain_t o) {
    if (is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      return value_domain_t(_cells | o._cells,
                            _ptr.join_loop(o._ptr),
                            _uninitialized.join_loop(o._uninitialized),
                            _nullity.join_loop(o._nullity));
    }
  }

  value_domain_t join_iter(value_domain_t o) {
    if (is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      return value_domain_t(_cells | o._cells,
                            _ptr.join_iter(o._ptr),
                            _uninitialized.join_iter(o._uninitialized),
                            _nullity.join_iter(o._nullity));
    }
  }

  value_domain_t operator&(value_domain_t o) {
    if (is_bottom() || o.is_bottom()) {
      return bottom();
    } else {
      return value_domain_t(_cells & o._cells,
                            _ptr & o._ptr,
                            _uninitialized & o._uninitialized,
                            _nullity & o._nullity);
    }
  }

  value_domain_t operator&&(value_domain_t o) {
    if (is_bottom() || o.is_bottom()) {
      return bottom();
    } else {
      return value_domain_t(_cells & o._cells,
                            _ptr && o._ptr,
                            _uninitialized && o._uninitialized,
                            _nullity && o._nullity);
    }
  }

  void operator-=(VariableName v) { forget_mem_surface(v); }

  /// \brief Forget the surface of v
  void forget(VariableName v) { forget_mem_surface(v); }

  template < typename Iterator >
  void forget(Iterator begin, Iterator end) {
    forget_mem_surface(begin, end);
  }

  /*
   * Implement numerical_domain
   *
   * These methods only affect the surface semantic.
   *
   * The user is responsible for calling the appropriate methods of
   * pointer_domain, uninitialized_domain and nullity_domain, so we don't
   * call them here.
   */

  void assign(VariableName x, linear_expression_t e) { _ptr.assign(x, e); }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    _ptr.apply(op, x, y, z);
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    _ptr.apply(op, x, y, k);
  }

  void operator+=(linear_constraint_t cst) { _ptr += cst; }

  void operator+=(linear_constraint_system_t csts) { _ptr += csts; }

  void forget_num(VariableName v) { _ptr.forget_num(v); }

  template < typename Iterator >
  void forget_num(Iterator begin, Iterator end) {
    _ptr.forget_num(begin, end);
  }

  linear_constraint_system_t to_linear_constraint_system() {
    return _ptr.to_linear_constraint_system();
  }

  /*
   * Helpers for num_domain_traits
   */

  void normalize() { _ptr.normalize(); }

  congruence_t to_congruence(VariableName v) {
    return num_domain_traits::to_congruence(_ptr, v);
  }

  interval_t to_interval(VariableName v, bool normalize) {
    return _ptr.to_interval(v, normalize);
  }

  interval_t to_interval(linear_expression_t e, bool normalize) {
    return _ptr.to_interval(e, normalize);
  }

  void from_interval(VariableName v, interval_t i) { _ptr.from_interval(v, i); }

  /*
   * Implement bitwise_operators
   */

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             uint64_t from,
             uint64_t to) {
    _ptr.apply(op, x, y, from, to);
  }

  void apply(conv_operation_t op,
             VariableName x,
             Number k,
             uint64_t from,
             uint64_t to) {
    _ptr.apply(op, x, k, from, to);
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    _ptr.apply(op, x, y, z);
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    _ptr.apply(op, x, y, k);
  }

  /*
   * Implement division_operators
   */

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    _ptr.apply(op, x, y, z);
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    _ptr.apply(op, x, y, k);
  }

  /*
   * Implement counter_domain
   */

  void mark_counter(VariableName v) { _ptr.mark_counter(v); }

  void unmark_counter(VariableName v) { _ptr.unmark_counter(v); }

  void init_counter(VariableName v, Number c) { _ptr.init_counter(v, c); }

  void incr_counter(VariableName v, Number k) { _ptr.incr_counter(v, k); }

  void forget_counter(VariableName v) { _ptr.forget_counter(v); }

  /*
   * Implement pointer_domain
   */

  void assign_object(VariableName p, MemoryLocation obj) {
    _ptr.assign_object(p, obj);
  }

  void assign_pointer(VariableName p, VariableName q) {
    _ptr.assign_pointer(p, q);
  }

  void assign_pointer(VariableName p, VariableName q, VariableName o) {
    _ptr.assign_pointer(p, q, o);
  }

  void assign_pointer(VariableName p, VariableName q, Number o) {
    _ptr.assign_pointer(p, q, o);
  }

  void assert_pointer(bool equality, VariableName p, VariableName q) {
    _ptr.assert_pointer(equality, p, q);
  }

  void refine_addrs(VariableName p, points_to_set_t addrs) {
    if (!is_bottom() && !_nullity.is_null(p) &&
        !_uninitialized.is_uninitialized(p)) {
      _ptr.refine_addrs(p, addrs);
    }
  }

  void refine_addrs_offset(VariableName p,
                           points_to_set_t addrs,
                           interval_t offset) {
    if (!is_bottom() && !_nullity.is_null(p) &&
        !_uninitialized.is_uninitialized(p)) {
      _ptr.refine_addrs_offset(p, addrs, offset);
    }
  }

  bool is_unknown_addr(VariableName p) { return _ptr.is_unknown_addr(p); }

  points_to_set_t addrs_set(VariableName p) { return _ptr.addrs_set(p); }

  VariableName offset_var(VariableName p) { return _ptr.offset_var(p); }

  void forget_pointer(VariableName p) { _ptr.forget_pointer(p); }

  template < typename Iterator >
  void forget_pointer(Iterator begin, Iterator end) {
    _ptr.forget_pointer(begin, end);
  }

  /*
   * Implement uninitialized_domain
   */

  void make_initialized(VariableName v) {
    return _uninitialized.make_initialized(v);
  }

  void make_uninitialized(VariableName v) {
    return _uninitialized.make_uninitialized(v);
  }

  void assign_uninitialized(VariableName x, VariableName y) {
    return _uninitialized.assign_uninitialized(x, y);
  }

  void assign_uninitialized(VariableName x, VariableName y, VariableName z) {
    return _uninitialized.assign_uninitialized(x, y, z);
  }

  void assign_uninitialized(VariableName x,
                            const std::vector< VariableName >& y) {
    return _uninitialized.assign_uninitialized(x, y);
  }

  bool is_initialized(VariableName v) {
    return _uninitialized.is_initialized(v);
  }

  bool is_uninitialized(VariableName v) {
    return _uninitialized.is_uninitialized(v);
  }

  void forget_uninitialized(VariableName v) {
    _uninitialized.forget_uninitialized(v);
  }

  template < typename Iterator >
  void forget_uninitialized(Iterator begin, Iterator end) {
    _uninitialized.forget_uninitialized(begin, end);
  }

  /*
   * Implement nullity_domain
   */

  void make_null(VariableName v) { _nullity.make_null(v); }

  void make_non_null(VariableName v) { _nullity.make_non_null(v); }

  void assign_nullity(VariableName x, VariableName y) {
    _nullity.assign_nullity(x, y);
  }

  void assert_null(VariableName v) {
    _nullity.assert_null(v);
    if (_nullity.is_bottom()) {
      *this = bottom();
    }
  }

  void assert_non_null(VariableName v) {
    _nullity.assert_non_null(v);
    if (_nullity.is_bottom()) {
      *this = bottom();
    }
  }

  void assert_nullity(bool equality, VariableName x, VariableName y) {
    _nullity.assert_nullity(equality, x, y);
    if (_nullity.is_bottom()) {
      *this = bottom();
    }
  }

  bool is_null(VariableName v) { return _nullity.is_null(v); }

  bool is_non_null(VariableName v) { return _nullity.is_non_null(v); }

  void forget_nullity(VariableName v) { return _nullity.forget_nullity(v); }

  template < typename Iterator >
  void forget_nullity(Iterator begin, Iterator end) {
    return _nullity.forget_nullity(begin, end);
  }

  /*
   * Implement memory_domain
   */

  void mem_write(VariableFactory& vfac,
                 VariableName pointer,
                 literal_t rhs,
                 Number size) {
    if (is_bottom())
      return;

    if (_nullity.is_null(pointer) || _uninitialized.is_uninitialized(pointer)) {
      // null/undefined dereference
      *this = bottom();
      return;
    }

    // get memory locations pointed by pointer
    points_to_set_t addrs = _ptr.addrs_set(pointer);
    if (addrs.is_top()) {
      forget_mem_contents(); // very conservative, but sound
      return;
    }

    // get the offset interval
    interval_t offset_intv = _ptr.to_interval(offset_var(pointer), true);
    offset_intv = offset_intv & interval_t(0, bound_t::plus_infinity());

    if (offset_intv.is_bottom()) { // buffer underflow
      *this = bottom();
      return;
    }

    if (offset_intv.singleton()) {
      // The offset is an integer.
      //
      // We can perform the usual reduction and update.

      for (typename points_to_set_t::iterator it = addrs.begin();
           it != addrs.end();
           ++it) {
        Cell c = write_realize_single_cell(vfac,
                                           *it,
                                           *offset_intv.singleton(),
                                           size);

        if (addrs.size() == 1) {
          strong_update(c.scalar_var(), rhs);
        } else {
          weak_update(c.scalar_var(), rhs);
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

      for (typename points_to_set_t::iterator it = addrs.begin();
           it != addrs.end();
           ++it) {
        std::vector< Cell > cells =
            write_realize_range_cells(*it, offset_var(pointer), size);
        for (typename std::vector< Cell >::iterator it2 = cells.begin();
             it2 != cells.end();
             ++it2) {
          weak_update(it2->scalar_var(), rhs);
        }
      }
    }
  }

  void mem_read(VariableFactory& vfac,
                literal_t lhs,
                VariableName pointer,
                Number size) {
    assert(lhs.is_var());

    if (is_bottom())
      return;

    if (_nullity.is_null(pointer) || _uninitialized.is_uninitialized(pointer)) {
      // null/undefined dereference
      *this = bottom();
      return;
    }

    // get memory locations pointed by pointer
    points_to_set_t addrs = _ptr.addrs_set(pointer);
    if (addrs.is_top()) {
      _ptr -= lhs.var();
      _nullity -= lhs.var();
      _uninitialized -= lhs.var();
      return;
    }

    // get the offset interval
    interval_t offset_intv = _ptr.to_interval(offset_var(pointer), true);
    offset_intv = offset_intv & interval_t(0, bound_t::plus_infinity());

    if (offset_intv.is_bottom()) { // buffer underflow
      *this = bottom();
      return;
    }

    if (offset_intv.singleton()) {
      // The offset is an integer.
      //
      // We can perform the usual reduction and update.

      bool first = true;
      for (typename points_to_set_t::iterator it = addrs.begin();
           it != addrs.end();
           ++it) {
        Cell c =
            read_realize_single_cell(vfac, *it, *offset_intv.singleton(), size);

        if (first) {
          strong_update(lhs, c.scalar_var());
          first = false;
        } else {
          weak_update(lhs, c.scalar_var());
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
      // TODO: note that we could have a bounded array for which we
      // have the complete set of cells and thus we could be more
      // precise in that case.
      //
      // To deal with this situation in a more systematic way
      // we should combine anyway the value domain with some
      // summarization array-based domain (e.g.,some array domain
      // such as a trivial array smashing or something more
      // expressive like Cousot&Logozzo's POPL'11).
      _ptr -= lhs.var();
      _nullity -= lhs.var();
      _uninitialized -= lhs.var();
    }
  }

  void mem_copy(VariableFactory& vfac,
                VariableName dest,
                VariableName src,
                linear_expression_t size) {
    if (is_bottom())
      return;

    if (_nullity.is_null(dest) || _uninitialized.is_uninitialized(dest) ||
        _nullity.is_null(src) || _uninitialized.is_uninitialized(src)) {
      // null/undefined dereference
      *this = bottom();
      return;
    }

    // get memory locations pointed by dest and src
    points_to_set_t src_addrs = _ptr.addrs_set(src);
    points_to_set_t dest_addrs = _ptr.addrs_set(dest);

    if (dest_addrs.is_top()) {
      forget_mem_contents(); // very conservative, but sound
      return;
    }

    // get size and pointer offsets approximation
    interval_t dest_intv = _ptr.to_interval(offset_var(dest), true) &
                           interval_t(0, bound_t::plus_infinity());
    interval_t src_intv = _ptr.to_interval(offset_var(src), true) &
                          interval_t(0, bound_t::plus_infinity());
    interval_t size_intv =
        _ptr.to_interval(size, true) & interval_t(1, bound_t::plus_infinity());

    if (dest_intv.is_bottom() || src_intv.is_bottom()) { // buffer underflow
      *this = bottom();
      return;
    }

    if (size_intv.is_bottom())
      return;

    // we have to be sound, so we remove all reachable cells
    forget_reachable_cells(dest, size_intv);

    if (dest_addrs.size() == 1 && dest_intv.singleton() &&
        !src_addrs.is_top() && src_intv.singleton()) {
      // in that only case, we can be more precise
      MemoryLocation dest_base = *dest_addrs.begin();
      Number dest_offset = *dest_intv.singleton();
      Number src_offset = *src_intv.singleton();
      Number size_lb = *size_intv.lb().number();

      value_domain_t prev = *this;
      bool first = true;

      for (typename points_to_set_t::iterator it = src_addrs.begin();
           it != src_addrs.end();
           ++it) {
        value_domain_t inv = prev;
        cell_set_t src_cells = prev._cells[*it];

        if (!src_cells.is_top()) {
          for (cell_set_iterator c_it = src_cells.begin();
               c_it != src_cells.end();
               ++c_it) {
            const Cell& c = *c_it;

            if (c.range() <= interval_t(src_offset, src_offset + size_lb - 1)) {
              Cell dest_c(vfac,
                          dest_base,
                          dest_offset + c.offset() - src_offset,
                          c.size());
              inv.add_cell(dest_c);
              inv.assign(dest_c.scalar_var(),
                         linear_expression_t(c.scalar_var()));
              inv.assign_pointer(dest_c.scalar_var(), c.scalar_var());
              inv.assign_nullity(dest_c.scalar_var(), c.scalar_var());
              inv.assign_uninitialized(dest_c.scalar_var(), c.scalar_var());
            }
          }
        }

        if (first) {
          *this = inv;
          first = false;
        } else {
          *this = *this | inv;
        }
      }
    }
  }

  void mem_set(VariableFactory& vfac,
               VariableName dest,
               linear_expression_t value,
               linear_expression_t size) {
    if (is_bottom())
      return;

    if (_nullity.is_null(dest) || _uninitialized.is_uninitialized(dest)) {
      // null/undefined dereference
      *this = bottom();
      return;
    }

    // get memory locations pointed by dest
    points_to_set_t addrs = _ptr.addrs_set(dest);

    if (addrs.is_top()) {
      forget_mem_contents(); // very conservative, but sound
      return;
    }

    // get size and pointer offsets approximation
    interval_t dest_intv = _ptr.to_interval(offset_var(dest), true) &
                           interval_t(0, bound_t::plus_infinity());
    interval_t size_intv =
        _ptr.to_interval(size, true) & interval_t(1, bound_t::plus_infinity());

    if (dest_intv.is_bottom()) { // buffer underflow
      *this = bottom();
      return;
    }

    if (size_intv.is_bottom())
      return;

    if (_ptr.to_interval(value, true) ==
        interval_t(Number(0))) { // memory set to 0
      Number size_lb = *size_intv.lb().number();

      // offsets we know they are updated
      interval_t safe_range = interval_t::bottom();
      if (dest_intv.ub().is_finite()) {
        safe_range = interval_t(dest_intv.lb(), dest_intv.lb() + size_lb - 1) &
                     interval_t(dest_intv.ub(), dest_intv.ub() + size_lb - 1);
      }

      // possibly updated offsets
      interval_t unsafe_range = dest_intv + interval_t(0, size_intv.ub() - 1);

      for (typename points_to_set_t::iterator it = addrs.begin();
           it != addrs.end();
           ++it) {
        MemoryLocation base = *it;
        cell_set_t cells = _cells[base];

        if (!cells.is_top()) {
          for (cell_set_iterator c_it = cells.begin(); c_it != cells.end();
               ++c_it) {
            const Cell& c = *c_it;

            if (c.range() <= safe_range) {
              if (addrs.size() == 1) {
                strong_update(c.scalar_var(), literal_t::integer(0));
              } else {
                weak_update(c.scalar_var(), literal_t::integer(0));
              }
            } else if (c.overlaps(unsafe_range)) {
              remove_cell(c);
            }
          }
        }
      }
    } else {
      // to be sound, we remove all reachable cells
      forget_reachable_cells(dest, size_intv);
    }
  }

  void forget_mem_surface(VariableName v) {
    if (is_bottom())
      return;

    _ptr.forget(v);
    _nullity.forget(v);
    _uninitialized.forget(v);
  }

  template < typename Iterator >
  void forget_mem_surface(Iterator begin, Iterator end) {
    if (is_bottom())
      return;

    _ptr.forget(begin, end);
    _nullity.forget(begin, end);
    _uninitialized.forget(begin, end);
  }

  void forget_mem_contents() { forget_cells(); }

  void forget_mem_contents(VariableName p) { forget_reachable_cells(p); }

  void forget_mem_contents(VariableName p, Number size) {
    forget_reachable_cells(p, size);
  }

  void write(std::ostream& o) {
    if (is_bottom()) {
      o << "_|_";
    } else if (is_top()) {
      o << "{}";
    } else {
      o << "(" << _cells << "," << _ptr << "," << _uninitialized << ","
        << _nullity << ")";
    }
  }

  static std::string domain_name() {
    return "Memory (" + pointer_domain_t::domain_name() + ", " +
           uninitialized_domain_t::domain_name() + ", " +
           nullity_domain_t::domain_name() + ")";
  }

}; // end class value_domain

namespace num_domain_traits {
namespace detail {

template < typename ScalarDomain,
           typename VariableFactory,
           typename MemoryLocation >
struct normalize_impl<
    value_domain< ScalarDomain, VariableFactory, MemoryLocation > > {
  inline void operator()(
      value_domain< ScalarDomain, VariableFactory, MemoryLocation >& inv) {
    inv.normalize();
  }
};

template < typename ScalarDomain,
           typename VariableFactory,
           typename MemoryLocation >
struct var_to_interval_impl<
    value_domain< ScalarDomain, VariableFactory, MemoryLocation > > {
  inline interval< typename ScalarDomain::number_t > operator()(
      value_domain< ScalarDomain, VariableFactory, MemoryLocation >& inv,
      typename ScalarDomain::variable_name_t v,
      bool normalize) {
    return inv.to_interval(v, normalize);
  }
};

template < typename ScalarDomain,
           typename VariableFactory,
           typename MemoryLocation >
struct lin_expr_to_interval_impl<
    value_domain< ScalarDomain, VariableFactory, MemoryLocation > > {
  inline interval< typename ScalarDomain::number_t > operator()(
      value_domain< ScalarDomain, VariableFactory, MemoryLocation >& inv,
      typename ScalarDomain::linear_expression_t e,
      bool normalize) {
    return inv.to_interval(e, normalize);
  }
};

template < typename ScalarDomain,
           typename VariableFactory,
           typename MemoryLocation >
struct from_interval_impl<
    value_domain< ScalarDomain, VariableFactory, MemoryLocation > > {
  inline void operator()(
      value_domain< ScalarDomain, VariableFactory, MemoryLocation >& inv,
      typename ScalarDomain::variable_name_t v,
      interval< typename ScalarDomain::number_t > i) {
    inv.from_interval(v, i);
  }
};

} // end namespace detail
} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_VALUE_DOMAIN_HPP
