/*******************************************************************************
 *
 * A value abstract domain 'a la' Mine parameterized by a numerical
 * abstract domain.
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Based on the paper "Field-Sensitive Value Analysis of Embedded C
 * Programs with Union Types and Pointer Arithmetics" by A. Mine
 * (LCTES'06)
 *
 * Notices:
 *
 * Copyright (c) 2011-2016 United States Government as represented by the
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
#include <ikos/domains/division_operators_api.hpp>
#include <ikos/domains/memory_domains_api.hpp>
#include <ikos/domains/nullity.hpp>
#include <ikos/domains/numerical_domains_api.hpp>
#include <ikos/domains/pointer.hpp>
#include <ikos/domains/separate_domains.hpp>
#include <ikos/domains/uninitialized.hpp>

namespace ikos {

namespace var_name_traits {

template < typename VariableName, typename Number >
struct value_domain_cell_var {
  // Return the variable that represents the value at a given memory location
  VariableName operator()(VariableName addr, Number offset, Number size);
};

} // end namespace var_name_traits

// Memory abstraction consisting of cells and a pointer abstraction
// augmented with nullity and uninitialized variable information.
//
// This domain abstracts memory into a set of synthetic cells with
// integer and also pointer type following Mine's paper. If a cell is
// of type integer this domain can model its value and whether it is
// initialized or not. Otherwise, if the cell is of pointer type this
// domain keeps track only whether it is null or not. Therefore, this
// domain does not model the addresses to which a cell of pointer type
// may point to. This is the reason why we allow reduction with an
// external pointer analysis that can provide that information.
template < typename ScalarDomain >
class value_domain
    : public abstract_domain,
      public numerical_domain< typename ScalarDomain::number_t,
                               typename ScalarDomain::variable_name_t >,
      public bitwise_operators< typename ScalarDomain::number_t,
                                typename ScalarDomain::variable_name_t >,
      public division_operators< typename ScalarDomain::number_t,
                                 typename ScalarDomain::variable_name_t >,
      public pointer_domain< typename ScalarDomain::number_t,
                             typename ScalarDomain::variable_name_t >,
      public uninitialized_domain< typename ScalarDomain::variable_name_t >,
      public nullity_domain< typename ScalarDomain::variable_name_t >,
      public memory_domain< typename ScalarDomain::number_t,
                            dummy_number,
                            typename ScalarDomain::variable_name_t > {
private:
  typedef typename ScalarDomain::number_t Number;
  typedef typename ScalarDomain::variable_name_t VariableName;

public:
  typedef Number number_t;
  typedef VariableName variable_name_t;
  typedef bound< Number > bound_t;
  typedef interval< Number > interval_t;
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;
  typedef Number offset_number_t;
  typedef discrete_domain< VariableName > points_to_set_t;
  typedef literal< Number, dummy_number, VariableName > literal_t;
  typedef ScalarDomain scalar_domain_t;
  typedef value_domain< ScalarDomain > value_domain_t;

private:
  typedef pointer_domain_impl< ScalarDomain > pointer_domain_t;
  typedef uninitialized_domain_impl< VariableName > uninitialized_domain_t;
  typedef nullity_domain_impl< VariableName > nullity_domain_t;

private:
  // A cell is a triple <b, o, s> modelling all bytes at address b, starting at
  // offset o up to (o + s - 1).
  //
  // Note that both the offset and the size are numbers.
  class Cell : public writeable {
  private:
    VariableName _base;
    Number _offset;
    Number _size;

    // represents the cell in the pointer+scalar domain
    VariableName _scalar_var;

  public:
    Cell(VariableName base, Number offset, Number size)
        : _base(base), _offset(offset), _size(size), _scalar_var(base) {
      assert(_offset >= 0 && _size >= 1);
      _scalar_var = var_name_traits::value_domain_cell_var< VariableName,
                                                            Number >()(_base,
                                                                       _offset,
                                                                       _size);
    }

    VariableName base() const { return _base; }
    Number offset() const { return _offset; }
    Number size() const { return _size; }
    VariableName scalar_var() const { return _scalar_var; }
    std::size_t index() const { return _scalar_var.index(); }

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

    // Return the number of possible overlaps between the cell and the write at
    // offset `offset` of size `size`
    template < typename PointerDomain >
    bound_t number_overlaps(VariableName offset,
                            Number size,
                            PointerDomain inv) const {
      inv += (variable_t(offset) + size - 1 >= _offset);
      inv += (variable_t(offset) <= _offset + _size - 1);

      interval_t rng = inv.to_interval(offset);
      if (rng.is_bottom()) {
        return 0;
      } else if (rng.ub().is_infinite()) {
        return bound_t::plus_infinity();
      } else {
        return *rng.ub().number() - *rng.lb().number() + 1;
      }
    }

    // Return true iff the cell is a realization of the write at offset
    // `offset` and size `size`
    template < typename PointerDomain >
    bool realizes(VariableName offset, Number size, PointerDomain inv) const {
      if (size != _size)
        return false;

      inv += (variable_t(offset) == _offset);
      return !inv.is_bottom();
    }

    void write(std::ostream& o) {
      o << "C{" << _base.name() << "," << _offset << "," << _size << "}";
    }

  }; // end class Cell

  // This is the discrete_domain but with a different semantic.
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
        throw ikos_error("cell set: trying to call size() on top");
      } else {
        return this->_set.size();
      }
    }

    iterator begin() {
      if (this->_is_top) {
        throw ikos_error("cell set: trying to call begin() on top");
      } else {
        return this->_set.begin();
      }
    }

    iterator end() {
      if (this->_is_top) {
        throw ikos_error("cell set: trying to call end() on top");
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
  typedef separate_domain< VariableName, cell_set_t > var_to_cell_set_t;

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

  // Create a new cell for a write, performing reduction if possible
  Cell write_realize_single_cell(VariableName base,
                                 Number offset,
                                 Number size) {
    Cell c(base, offset, size);

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

  // Perform a write with an approximated offset.
  // returns a list of cells on which we should perform a weak update.
  std::vector< Cell > write_realize_range_cells(VariableName base,
                                                VariableName offset,
                                                Number size) {
    // get the interval
    interval_t offset_intv = _ptr.to_interval(offset);
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

  // Create a new cell for a read
  Cell read_realize_single_cell(VariableName base, Number offset, Number size) {
    Cell c(base, offset, size);

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
      _uninitialized -= _lhs;
      _nullity.make_null(_lhs);
    }

    void integer_var(VariableName rhs) {
      _ptr.assign(_lhs, variable_t(rhs));
      _uninitialized.assign_uninitialized(_lhs, rhs);
      _nullity -= _lhs;
    }

    void floating_point_var(VariableName rhs) {
      _ptr -= _lhs; // ignored
      _uninitialized.assign_uninitialized(_lhs, rhs);
      _nullity -= _lhs;
    }

    void pointer_var(VariableName rhs) {
      _ptr -= _lhs; // for now, ignored
      _uninitialized -= _lhs;
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
      throw ikos_error("value_domain: trying to assign an integer");
    }

    void floating_point(dummy_number) {
      throw ikos_error("value_domain: trying to assign a floating point");
    }

    void undefined() {
      throw ikos_error("value_domain: trying to assign undefined");
    }

    void null() { throw ikos_error("value_domain: trying to assign null"); }

    void integer_var(VariableName lhs) {
      _ptr.assign(lhs, variable_t(_rhs));
      _uninitialized.assign_uninitialized(lhs, _rhs);
      _nullity -= lhs;
    }

    void floating_point_var(VariableName lhs) {
      _ptr -= lhs; // ignored
      _uninitialized.assign_uninitialized(lhs, _rhs);
      _nullity -= lhs;
    }

    void pointer_var(VariableName lhs) {
      _ptr -= lhs; // for now, ignored
      _uninitialized -= lhs;
      _nullity.assign_nullity(lhs, _rhs);
    }

  }; // end class literal_reader

  // Perform a strong update `lhs = rhs`
  void strong_update(VariableName lhs, literal_t rhs) {
    literal_writer v(lhs, _ptr, _uninitialized, _nullity);
    rhs.apply_visitor(v);
  }

  // Perform a strong update `lhs = rhs`
  void strong_update(literal_t lhs, VariableName rhs) {
    literal_reader v(rhs, _ptr, _uninitialized, _nullity);
    lhs.apply_visitor(v);
  }

  // Perform a weak update `lhs = rhs`
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

  // Perform a weak update `lhs = rhs`
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

  // Forget all memory cells that can be accessible through pointer p
  void forget_reachable_cells(VariableName p) {
    if (is_bottom())
      return;

    if (_nullity.is_null(p))
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

  // Forget all memory cells that can be accessible through pointer p and that
  // overlap with [p.offset, ..., p.offset + size - 1]
  void forget_reachable_cells(VariableName p, interval_t size) {
    if (is_bottom())
      return;

    if (_nullity.is_null(p))
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

  // Forget all memory cells of base address `base`
  void forget_cells(VariableName base) {
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

  // Forget all memory cells of base address `base` that overlap
  // with [offset, ..., offset + size - 1]
  void forget_cells(VariableName base, VariableName offset, interval_t size) {
    interval_t offset_intv =
        _ptr.to_interval(offset) & interval_t(0, bound_t::plus_infinity());
    size = size & interval_t(1, bound_t::plus_infinity());
    interval_t range = offset_intv + interval_t(0, size.ub() - 1);
    forget_cells(base, range);
  }

  // Forget all memory cells of base address `base` that overlap with a given
  // range
  void forget_cells(VariableName base, interval_t range) {
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

  // Forget both the surface of v (base addresses and offsets) and
  // all the memory contents that may be accessible through v.
  void operator-=(VariableName v) {
    forget_mem_contents(v); // has to be before forget_mem_surface
    forget_mem_surface(v);
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

  linear_constraint_system_t to_linear_constraint_system() {
    return _ptr.to_linear_constraint_system();
  }

  /*
   * Helpers for num_domain_traits
   */

  void normalize() { _ptr.normalize(); }

  interval_t to_interval(VariableName v) { return _ptr.to_interval(v); }

  interval_t to_interval(linear_expression_t e) { return _ptr.to_interval(e); }

  void from_interval(VariableName v, interval_t i) { _ptr.from_interval(v, i); }

  /*
   * Implement bitwise_operators
   */

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             unsigned width) {
    _ptr.apply(op, x, y, width);
  }

  void apply(conv_operation_t op, VariableName x, Number k, unsigned width) {
    _ptr.apply(op, x, k, width);
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
   * Implement pointer_domain
   */

  void assign_object(VariableName p, VariableName obj) {
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
    if (!is_bottom() && !_nullity.is_null(p)) {
      _ptr.refine_addrs(p, addrs);
    }
  }

  void refine_addrs_offset(VariableName p,
                           points_to_set_t addrs,
                           interval_t offset) {
    if (!is_bottom() && !_nullity.is_null(p)) {
      _ptr.refine_addrs_offset(p, addrs, offset);
    }
  }

  bool is_unknown_addr(VariableName p) { return _ptr.is_unknown_addr(p); }

  points_to_set_t addrs_set(VariableName p) { return _ptr.addrs_set(p); }

  VariableName offset_var(VariableName p) { return _ptr.offset_var(p); }

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

  /*
   * Implement memory_domain
   */

  void mem_write(VariableName pointer, literal_t rhs, Number size) {
    if (is_bottom())
      return;

    if (_nullity.is_null(pointer)) { // null dereference!
      *this = bottom();
      return;
    }

    // get memory locations pointed by pointer
    points_to_set_t addrs = _ptr.addrs_set(pointer);
    if (addrs.is_top()) {
#if 0
      std::cout << "Ignored memory write to " << pointer << " with value " << rhs << std::endl;
#endif
      return;
    }

    // get the offset interval
    interval_t offset_intv = _ptr.to_interval(offset_var(pointer));
    offset_intv = offset_intv & interval_t(0, bound_t::plus_infinity());

    if (offset_intv.is_bottom()) {
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
        Cell c = write_realize_single_cell(*it, *offset_intv.singleton(), size);

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

  void mem_read(literal_t lhs, VariableName pointer, Number size) {
    assert(lhs.is_var());

    if (is_bottom())
      return;

    if (_nullity.is_null(pointer)) { // null dereference!
      *this = bottom();
      return;
    }

    // get memory locations pointed by pointer
    points_to_set_t addrs = _ptr.addrs_set(pointer);
    if (addrs.is_top()) {
#if 0
      std::cout << "Ignored memory read from pointer " << pointer << std::endl;
#endif
      return;
    }

    // get the offset interval
    interval_t offset_intv = _ptr.to_interval(offset_var(pointer));
    offset_intv = offset_intv & interval_t(0, bound_t::plus_infinity());

    if (offset_intv.is_bottom()) {
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
        Cell c = read_realize_single_cell(*it, *offset_intv.singleton(), size);

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

  void mem_copy(VariableName dest, VariableName src, linear_expression_t size) {
    if (is_bottom())
      return;

    if (_nullity.is_null(dest) || _nullity.is_null(src)) { // null dereference!
      *this = bottom();
      return;
    }

    // get memory locations pointed by dest and src
    points_to_set_t src_addrs = _ptr.addrs_set(src);
    points_to_set_t dest_addrs = _ptr.addrs_set(dest);

    if (dest_addrs.is_top()) {
#if 0
      std::cout << "Ignored memory copy from " << src << " to " << dest << std::endl;
#endif
      return;
    }

    // get size and pointer offsets approximation
    interval_t dest_intv = _ptr.to_interval(offset_var(dest)) &
                           interval_t(0, bound_t::plus_infinity());
    interval_t src_intv = _ptr.to_interval(offset_var(src)) &
                          interval_t(0, bound_t::plus_infinity());
    interval_t size_intv =
        _ptr.to_interval(size) & interval_t(1, bound_t::plus_infinity());

    if (dest_intv.is_bottom() || src_intv.is_bottom()) { // buffer overflow
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
      VariableName dest_base = *dest_addrs.begin();
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
              Cell dest_c(dest_base,
                          dest_offset + c.offset() - src_offset,
                          c.size());
              inv.add_cell(dest_c);
              inv.assign(dest_c.scalar_var(),
                         linear_expression_t(c.scalar_var()));
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

  void mem_set(VariableName dest,
               linear_expression_t value,
               linear_expression_t size) {
    if (is_bottom())
      return;

    if (_nullity.is_null(dest)) { // null dereference!
      *this = bottom();
      return;
    }

    // get memory locations pointed by dest
    points_to_set_t addrs = _ptr.addrs_set(dest);

    if (addrs.is_top()) {
#if 0
      std::cout << "Ignored memory set on " << dest << std::endl;
#endif
      return;
    }

    // get size and pointer offsets approximation
    interval_t dest_intv = _ptr.to_interval(offset_var(dest)) &
                           interval_t(0, bound_t::plus_infinity());
    interval_t size_intv =
        _ptr.to_interval(size) & interval_t(1, bound_t::plus_infinity());

    if (dest_intv.is_bottom()) { // buffer overflow
      *this = bottom();
      return;
    }

    if (size_intv.is_bottom())
      return;

    if (_ptr.to_interval(value) == interval_t(Number(0))) { // memory set to 0
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
        VariableName base = *it;
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

    _ptr -= v;
    _nullity -= v;
    _uninitialized -= v;
  }

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

template < typename ScalarDomain >
struct normalize_impl< value_domain< ScalarDomain > > {
  void operator()(value_domain< ScalarDomain >& inv) { inv.normalize(); }
};

template < typename ScalarDomain >
struct var_to_interval_impl< value_domain< ScalarDomain > > {
  interval< typename ScalarDomain::number_t > operator()(
      value_domain< ScalarDomain >& inv,
      typename ScalarDomain::variable_name_t v) {
    return inv.to_interval(v);
  }
};

template < typename ScalarDomain >
struct lin_expr_to_interval_impl< value_domain< ScalarDomain > > {
  interval< typename ScalarDomain::number_t > operator()(
      value_domain< ScalarDomain >& inv,
      typename ScalarDomain::linear_expression_t e) {
    return inv.to_interval(e);
  }
};

template < typename ScalarDomain >
struct from_interval_impl< value_domain< ScalarDomain > > {
  void operator()(value_domain< ScalarDomain >& inv,
                  typename ScalarDomain::variable_name_t v,
                  interval< typename ScalarDomain::number_t > i) {
    inv.from_interval(v, i);
  }
};

} // end namespace detail
} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_VALUE_DOMAIN_HPP
