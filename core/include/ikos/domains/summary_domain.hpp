/**************************************************************************/ /**
 *
 * \file
 * \brief Implementation of a summary-based abstract domain
 *
 * A value abstract domain able to build function summaries using an underlying
 * numerical relational abstract domain. The domain is based on Antoine Mine's
 * paper and Jorge Navas's implementation in value_domain.hpp
 *
 * Author: Maxime Arthaud
 *
 * Contributors: Clement Decoodt
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

#ifndef IKOS_SUMMARY_DOMAIN_HPP
#define IKOS_SUMMARY_DOMAIN_HPP

#include <set>

#include <boost/iterator/filter_iterator.hpp>

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
struct summary_domain_cell_var {
  /// \returns the variable that represents the value at a given
  /// memory location
  typename VariableFactory::variable_name_t operator()(VariableFactory& vfac,
                                                       MemoryLocation addr,
                                                       Number offset,
                                                       bound< Number > size,
                                                       bool output);
};

template < typename VariableName >
struct summary_domain_tmp_var {
  /// \returns a new temporary variable
  ///
  /// The parameter v can be any variable name. It is given as a way to get
  /// the internal variable factory in the implementation.
  VariableName operator()(VariableName v);
};

} // end namespace var_name_traits

namespace summary_domain_impl {

/// \brief Memory cell
///
/// A cell is a quadruple <b, o, s, type> modelling all bytes at address b,
/// starting at offset o up to (o + size - 1). The type indicates whether it
/// refers to the old value (Input) or the new value (Output).
///
/// Note that the offset is an integer and the size is a bound (can be +oo).
template < typename VariableFactory, typename MemoryLocation, typename Number >
class Cell {
private:
  typedef typename VariableFactory::variable_name_t VariableName;
  typedef bound< Number > bound_t;
  typedef interval< Number > interval_t;
  typedef variable< Number, VariableName > variable_t;

public:
  typedef enum { Input, Output } cell_type_t;

private:
  MemoryLocation _base;
  Number _offset;
  bound_t _size;
  cell_type_t _type;

  /// represents the cell in the pointer+scalar domain
  VariableName _scalar_var;

public:
  Cell(VariableFactory& vfac,
       MemoryLocation base,
       Number offset,
       bound_t size,
       cell_type_t type)
      : _base(base), _offset(offset), _size(size), _type(type) {
    assert(_offset >= 0 && _size >= 1);
    _scalar_var =
        var_name_traits::summary_domain_cell_var< VariableFactory,
                                                  MemoryLocation,
                                                  Number >()(vfac,
                                                             _base,
                                                             _offset,
                                                             _size,
                                                             _type == Output);
  }

  Cell(VariableFactory& vfac,
       MemoryLocation base,
       interval_t range,
       cell_type_t type)
      : _base(base),
        _offset(*range.lb().number()),
        _size(range.ub() - range.lb() + 1),
        _type(type) {
    assert(!range.is_bottom() && range.lb().number());
    _scalar_var =
        var_name_traits::summary_domain_cell_var< VariableFactory,
                                                  MemoryLocation,
                                                  Number >()(vfac,
                                                             _base,
                                                             _offset,
                                                             _size,
                                                             _type == Output);
  }

  MemoryLocation base() const { return _base; }
  Number offset() const { return _offset; }
  bound_t size() const { return _size; }
  cell_type_t type() const { return _type; }
  bool is_input() const { return _type == Input; }
  bool is_output() const { return _type == Output; }
  VariableName scalar_var() const { return _scalar_var; }

  bool operator<(const Cell& o) const {
    // lexicographical order ensures that !(c1 < c2) && !(c2 < c1) => c1 == c2
    // it is required to use std::set< Cell >
    return _offset < o._offset || (_offset == o._offset && _size < o._size) ||
           (_offset == o._offset && _size == o._size && _type < o._type) ||
           (_offset == o._offset && _size == o._size && _type == o._type &&
            _base < o._base);
  }

  bool operator==(const Cell& o) const {
    return _base == o._base && _offset == o._offset && _size == o._size &&
           _type == o._type;
  }

  interval_t range() const {
    return interval_t(bound_t(_offset), bound_t(_offset) + _size - 1);
  }

  bool overlaps(interval_t range) const {
    interval_t meet = this->range() & range;
    return !meet.is_bottom();
  }

  bool overlaps(const Cell& c) const { return this->overlaps(c.range()); }

  /// \returns the number of possible overlaps between the cell and the write at
  /// offset `offset` of size `size`
  template < typename PointerDomain >
  bound_t number_overlaps(VariableName offset,
                          Number size,
                          PointerDomain inv) const {
    inv += (variable_t(offset) + size - 1 >= _offset);

    if (_size.is_finite()) {
      inv += (variable_t(offset) <= _offset + *_size.number() - 1);
    }

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
    if (!(_size.is_finite() && *_size.number() == size))
      return false;

    inv += (variable_t(offset) == _offset);
    return !inv.is_bottom();
  }

  void write(std::ostream& o) {
    o << "C{";
    index_traits< MemoryLocation >::write(o, _base);
    o << "," << _offset << "," << _size << "," << ((_type == Input) ? "I" : "O")
      << "}";
  }

  friend std::ostream& operator<<(std::ostream& o, Cell c) {
    c.write(o);
    return o;
  }

}; // end class Cell

} // end namespace summary_domain_impl

/// \brief Summary abstract domain
template < typename ScalarDomain,
           typename VariableFactory,
           typename MemoryLocation >
class summary_domain
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
  typedef dummy_number floating_point_t;
  typedef VariableFactory variable_factory_t;
  typedef VariableName variable_name_t;
  typedef MemoryLocation memory_location_t;
  typedef bound< Number > bound_t;
  typedef interval< Number > interval_t;
  typedef congruence< Number > congruence_t;
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;
  typedef discrete_domain< MemoryLocation > points_to_set_t;
  typedef literal< Number, dummy_number, VariableName > literal_t;
  typedef ScalarDomain scalar_domain_t;
  typedef summary_domain< ScalarDomain, VariableFactory, MemoryLocation >
      summary_domain_t;

private:
  typedef summary_domain_impl::Cell< VariableFactory, MemoryLocation, Number >
      Cell;
  typedef std::set< Cell > cell_set_t;
  typedef patricia_tree< MemoryLocation, cell_set_t > var_to_cell_set_t;
  typedef pointer_domain_impl< ScalarDomain, MemoryLocation > pointer_domain_t;
  typedef uninitialized_domain_impl< VariableName > uninitialized_domain_t;
  typedef nullity_domain_impl< VariableName > nullity_domain_t;

private:
  // _cells is a map from a memory object to a set of cells (ordered by offset).
  // There are 2 kind of cells: Input cell and Output cell.
  //
  // An output cell means that the value of the bytes [cell.offset, cell.offset
  // + cell.size - 1] has changed, and the new value is approximated in the
  // underlying abstract domain. No cell means the object is unchanged.
  //
  // An input cell refers to the value of the bytes [cell.offset, cell.offset +
  // cell.size - 1] before the call to the current function.
  //
  // If _cells is empty, it means nothing has changed, thus output = input.
  // This is different from top. That's why we need a special member _is_top to
  // indicate that everything has possibly changed.

  bool _is_top;
  var_to_cell_set_t _cells;
  pointer_domain_t _ptr; // surface domain
  uninitialized_domain_t _uninitialized;
  nullity_domain_t _nullity;

private:
  summary_domain(bool is_top)
      : _is_top(is_top),
        _cells(),
        _ptr(is_top ? pointer_domain_t::top() : pointer_domain_t::bottom()),
        _uninitialized(is_top ? uninitialized_domain_t::top()
                              : uninitialized_domain_t::bottom()),
        _nullity(is_top ? nullity_domain_t::top()
                        : nullity_domain_t::bottom()) {}

  summary_domain(bool is_top,
                 const var_to_cell_set_t& cells,
                 const pointer_domain_t& ptr,
                 const uninitialized_domain_t& uninitialized,
                 const nullity_domain_t& nullity)
      : _is_top(is_top),
        _cells(cells),
        _ptr(ptr),
        _uninitialized(uninitialized),
        _nullity(nullity) {}

private:
  void add_cell(const Cell& c) {
    boost::optional< cell_set_t > found = this->_cells.lookup(c.base());

    if (found) {
      cell_set_t& cells = *found;
      cells.insert(c);
      this->_cells.insert(c.base(), cells);
    } else {
      cell_set_t cells;
      cells.insert(c);
      this->_cells.insert(c.base(), cells);
    }
  }

  void remove_cell(const Cell& c) {
    // remove the cell from the memory abstraction
    boost::optional< cell_set_t > found = this->_cells.lookup(c.base());

    if (found) {
      cell_set_t& cells = *found;
      cells.erase(c);

      if (cells.empty()) {
        this->_cells.remove(c.base());
      } else {
        this->_cells.insert(c.base(), cells);
      }
    }

    // remove the cell from the pointer+scalar abstraction, nullity
    // abstraction and uninitialized variable abstraction
    _ptr -= c.scalar_var();
    _nullity -= c.scalar_var();
    _uninitialized -= c.scalar_var();
  }

  /// \brief Shrink the cell `shrunk` to remove all the bytes of cell `overlap`
  void shrink_cell(VariableFactory& vfac,
                   const Cell& shrunk,
                   const Cell& overlap) {
    interval_t range = shrunk.range();
    interval_t meet = range & overlap.range();
    assert(!meet.is_bottom());

    interval_t left(range.lb(), meet.lb() - 1);
    if (!left.is_bottom()) {
      add_cell(Cell(vfac, shrunk.base(), left, shrunk.type()));
    }

    interval_t right(meet.ub() + 1, range.ub());
    if (!right.is_bottom()) {
      add_cell(Cell(vfac, shrunk.base(), right, shrunk.type()));
    }

    remove_cell(shrunk);
  }

  /// \brief Create a new cell for a write, performing reduction if possible
  Cell realize_single_out_cell(VariableFactory& vfac,
                               MemoryLocation base,
                               Number offset,
                               Number size) {
    Cell c(vfac, base, offset, size, Cell::Output);

    boost::optional< cell_set_t > found = this->_cells.lookup(base);
    if (!found) { // no cell found for base
      add_cell(c);
      return c;
    } else {
      cell_set_t& cells = *found;

      if (cells.find(c) != cells.end()) { // same cell found
        return c;
      } else {
        for (output_cell_set_iterator it = output_begin(cells);
             it != output_end(cells);
             ++it) {
          if (c.overlaps(*it)) {
            shrink_cell(vfac, *it, c);
          }
        }

        // TODO: perform a reduction to improve precision
        add_cell(c);
        return c;
      }
    }
  }

  /// \brief Create a new cell for a read
  boost::optional< Cell > realize_single_in_cell(VariableFactory& vfac,
                                                 MemoryLocation base,
                                                 Number offset,
                                                 Number size) {
    Cell c_in(vfac, base, offset, size, Cell::Input);
    Cell c_out(vfac, base, offset, size, Cell::Output);

    boost::optional< cell_set_t > found = this->_cells.lookup(base);
    if (!found) { // no cell found for base, return an input cell
      add_cell(c_in);
      return boost::optional< Cell >(c_in);
    } else {
      cell_set_t& cells = *found;

      if (cells.find(c_out) != cells.end()) { // output cell found
        return boost::optional< Cell >(c_out);
      } else {
        for (output_cell_set_iterator it = output_begin(cells);
             it != output_end(cells);
             ++it) {
          if (c_out.overlaps(*it)) { // overlapping output cell, no realization
            return boost::optional< Cell >();
          }
        }

        // no overlapping output cell, return an input cell
        add_cell(c_in);
        return boost::optional< Cell >(c_in);
      }
    }
  }

  /// \brief Perform a write with an approximated offset
  ///
  /// \returns a list of cells on which we should perform a weak update
  std::vector< Cell > realize_range_out_cells(VariableFactory& vfac,
                                              MemoryLocation base,
                                              VariableName offset,
                                              Number size) {
    // get the interval
    interval_t offset_intv = _ptr.to_interval(offset, true);
    offset_intv = offset_intv & interval_t(0, bound_t::plus_infinity());
    assert(!offset_intv.is_bottom());
    interval_t range = offset_intv + interval_t(0, size - 1);

    boost::optional< cell_set_t > found = this->_cells.lookup(base);
    if (!found) { // no cell, create a cell covering all bytes
      add_cell(Cell(vfac, base, range, Cell::Output));
      return std::vector< Cell >();
    } else {
      cell_set_t& cells = *found;
      std::vector< Cell > updated_cells;

      for (output_cell_set_iterator it = output_begin(cells);
           it != output_end(cells);
           ++it) {
        const Cell& c = *it;

        if (c.overlaps(range)) {
          if (c.number_overlaps(offset, size, _ptr) == 1 &&
              c.realizes(offset, size, _ptr)) {
            // that cell has only one way to be affected by the write statement

            // perform a weak update
            updated_cells.push_back(c);

            // create a cell from range.lb to c.offset - 1 if needed
            if (range.lb() < c.offset()) {
              add_cell(Cell(vfac,
                            base,
                            interval_t(range.lb(), c.offset() - 1),
                            Cell::Output));
            }

            if (range.ub() <= bound_t(c.offset()) + c.size() - 1) {
              // end reached
              range = interval_t::bottom();
              break;
            } else {
              range = interval_t(bound_t(c.offset()) + c.size(), range.ub());
            }
          } else {
            range = range | c.range();
            remove_cell(c);
          }
        }
      }

      if (!range.is_bottom() && range.lb().number()) {
        add_cell(Cell(vfac, base, range, Cell::Output));
      }

      return updated_cells;
    }
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
      _ptr.assign(lhs, variable_t(_rhs));
      _ptr.forget_num(lhs);
      _uninitialized.assign_uninitialized(lhs, _rhs);
      _nullity -= lhs;
    }

    void floating_point_var(VariableName lhs) {
      _ptr -= lhs; // ignored
      _uninitialized.assign_uninitialized(lhs, _rhs);
      _nullity -= lhs;
    }

    void pointer_var(VariableName lhs) {
      _ptr.assign_pointer(lhs, _rhs);
      _ptr.forget_num(lhs);
      _uninitialized.assign_uninitialized(lhs, _rhs);
      _nullity.assign_nullity(lhs, _rhs);
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
    if (is_top() || is_bottom())
      return;

    if (_nullity.is_null(p) || _uninitialized.is_uninitialized(p))
      return;

    points_to_set_t addrs_set = _ptr.addrs_set(p); // p's base address

    if (addrs_set.is_top())
      return;

    for (typename points_to_set_t::iterator it = addrs_set.begin();
         it != addrs_set.end();
         ++it) {
      forget_cells(p->var_factory(), *it);
    }
  }

  /// \brief Forget all memory cells that can be accessible through pointer p
  // and that overlap with [p.offset, ..., p.offset + size - 1]
  void forget_reachable_cells(VariableName p, interval_t size) {
    if (is_top() || is_bottom())
      return;

    if (_nullity.is_null(p) || _uninitialized.is_uninitialized(p))
      return;

    points_to_set_t addrs_set = _ptr.addrs_set(p);

    if (addrs_set.is_top())
      return;

    for (typename points_to_set_t::iterator it = addrs_set.begin();
         it != addrs_set.end();
         ++it) {
      forget_cells(p->var_factory(), *it, offset_var(p), size);
    }
  }

  /// \brief Forget all memory cells of base address `base`
  void forget_cells(VariableFactory& vfac, MemoryLocation base) {
    // remove all output cells
    boost::optional< cell_set_t > found = this->_cells.lookup(base);
    if (found) {
      cell_set_t& cells = *found;

      for (output_cell_set_iterator it = output_begin(cells);
           it != output_end(cells);
           ++it) {
        remove_cell(*it);
      }
    }

    // add an output cell covering all bytes
    add_cell(
        Cell(vfac, base, Number(0), bound_t::plus_infinity(), Cell::Output));
  }

  /// \brief Forget all memory cells of base address `base` that overlap
  ///  with [offset, ..., offset + size - 1]
  void forget_cells(VariableFactory& vfac,
                    MemoryLocation base,
                    VariableName offset,
                    interval_t size) {
    interval_t offset_intv = _ptr.to_interval(offset, true) &
                             interval_t(0, bound_t::plus_infinity());
    size = size & interval_t(1, bound_t::plus_infinity());
    interval_t range = offset_intv + interval_t(0, size.ub() - 1);
    forget_cells(vfac, base, range);
  }

  /// \brief Forget all memory cells of base address `base` that overlap with a
  /// given range
  void forget_cells(VariableFactory& vfac,
                    MemoryLocation base,
                    interval_t range) {
    boost::optional< cell_set_t > found = this->_cells.lookup(base);
    if (found) {
      cell_set_t& cells = *found;

      for (output_cell_set_iterator it = output_begin(cells);
           it != output_end(cells);
           ++it) {
        const Cell& c = *it;

        if (c.overlaps(range)) {
          range = range | c.range();
          remove_cell(c);
        }
      }
    }

    // add an output cell
    add_cell(Cell(vfac, base, range, Cell::Output));
  }

  // Iterators to iterate over input/output cells only

  struct is_input_cell {
    bool operator()(const Cell& c) { return c.is_input(); }
  };

  struct is_output_cell {
    bool operator()(const Cell& c) { return c.is_output(); }
  };

  typedef typename cell_set_t::iterator cell_set_iterator;
  typedef boost::filter_iterator< is_input_cell, cell_set_iterator >
      input_cell_set_iterator;
  typedef boost::filter_iterator< is_output_cell, cell_set_iterator >
      output_cell_set_iterator;

  static input_cell_set_iterator make_input_iterator(
      cell_set_iterator begin, cell_set_iterator end = cell_set_iterator()) {
    return boost::make_filter_iterator< is_input_cell >(begin, end);
  }

  static output_cell_set_iterator make_output_iterator(
      cell_set_iterator begin, cell_set_iterator end = cell_set_iterator()) {
    return boost::make_filter_iterator< is_output_cell >(begin, end);
  }

  static input_cell_set_iterator input_begin(cell_set_t& s) {
    return make_input_iterator(s.begin(), s.end());
  }

  static input_cell_set_iterator input_end(cell_set_t& s) {
    return make_input_iterator(s.end(), s.end());
  }

  static output_cell_set_iterator output_begin(cell_set_t& s) {
    return make_output_iterator(s.begin(), s.end());
  }

  static output_cell_set_iterator output_end(cell_set_t& s) {
    return make_output_iterator(s.end(), s.end());
  }

  // Merge summary_domains

  /// \brief Add the relation C{a,o,s,O} = C{a,o,s,I}
  static void add_unchanged_relation(const Cell& c, pointer_domain_t& ptr) {
    assert(c.is_output());
    Cell in_cell(c.scalar_var()->var_factory(),
                 c.base(),
                 c.offset(),
                 c.size(),
                 Cell::Input);
    ptr.assign(c.scalar_var(), linear_expression_t(in_cell.scalar_var()));
  }

  /// \brief Add the relation C{a,o,s,O} = C{a,o,s,I}
  static void add_unchanged_relation(VariableFactory& vfac,
                                     const Cell& c,
                                     cell_set_t& cells,
                                     pointer_domain_t& ptr) {
    assert(c.is_output());
    Cell in_cell(vfac, c.base(), c.offset(), c.size(), Cell::Input);
    cells.insert(in_cell);
    ptr.assign(c.scalar_var(), linear_expression_t(in_cell.scalar_var()));
  }

  struct merge_cells_op : public var_to_cell_set_t::generic_binary_op_t {
    // surface domains
    pointer_domain_t& _left_ptr;
    pointer_domain_t& _right_ptr;

    merge_cells_op(pointer_domain_t& left_ptr, pointer_domain_t& right_ptr)
        : _left_ptr(left_ptr), _right_ptr(right_ptr) {}

    /// \brief Merge 2 set of cells
    boost::optional< cell_set_t > apply(cell_set_t left, cell_set_t right) {
      assert(!left.empty() && !right.empty());
      MemoryLocation base = (*left.begin()).base();
      cell_set_t result;

      // Hack to get a VariableFactory to create the cells
      VariableFactory& vfac = (*left.begin()).scalar_var()->var_factory();

      // Input cells are kept
      result.insert(input_begin(left), input_end(left));
      result.insert(input_begin(right), input_end(right));

      // Output cells: we keep identical cells and we merge overlapping cells
      output_cell_set_iterator left_it = output_begin(left);
      output_cell_set_iterator right_it = output_begin(right);

      // note: cells are ordered by _offset
      while (left_it != output_end(left) && right_it != output_end(right)) {
        const Cell& left_c = *left_it;
        const Cell& right_c = *right_it;

        if (left_c == right_c) { // identical cells
          result.insert(left_c);
          ++left_it;
          ++right_it;
        } else if (left_c.overlaps(right_c)) { // overlapping cells
          interval_t range = left_c.range() | right_c.range();
          ++left_it;
          ++right_it;

          // merge all cells overlapping with range
          while ((left_it != output_end(left) && left_it->overlaps(range)) ||
                 (right_it != output_end(right) && right_it->overlaps(range))) {
            if (left_it != output_end(left)) {
              range = range | left_it->range();
              ++left_it;
            } else {
              range = range | right_it->range();
              ++right_it;
            }
          }

          result.insert(Cell(vfac, base, range, Cell::Output));
        } else if (left_c.offset() < right_c.offset()) {
          result.insert(left_c);
          add_unchanged_relation(vfac, left_c, result, _right_ptr);
          ++left_it;
        } else {
          result.insert(right_c);
          add_unchanged_relation(vfac, right_c, result, _left_ptr);
          ++right_it;
        }
      }

      for (; left_it != output_end(left); ++left_it) {
        result.insert(*left_it);
        add_unchanged_relation(vfac, *left_it, result, _right_ptr);
      }

      for (; right_it != output_end(right); ++right_it) {
        result.insert(*right_it);
        add_unchanged_relation(vfac, *right_it, result, _left_ptr);
      }

      return boost::optional< cell_set_t >(result);
    }

    boost::optional< cell_set_t > apply_left(cell_set_t left) {
      cell_set_t result = left;
      VariableFactory& vfac = (*left.begin()).scalar_var()->var_factory();

      for (output_cell_set_iterator it = output_begin(left);
           it != output_end(left);
           ++it) {
        add_unchanged_relation(vfac, *it, result, _right_ptr);
      }

      return boost::optional< cell_set_t >(result);
    }

    boost::optional< cell_set_t > apply_right(cell_set_t right) {
      cell_set_t result = right;
      VariableFactory& vfac = (*right.begin()).scalar_var()->var_factory();

      for (output_cell_set_iterator it = output_begin(right);
           it != output_end(right);
           ++it) {
        add_unchanged_relation(vfac, *it, result, _left_ptr);
      }

      return boost::optional< cell_set_t >(result);
    }

  }; // end struct merge_cells_op

  /// \brief Merge 2 map of cells
  static var_to_cell_set_t merge_cells(var_to_cell_set_t left_cells,
                                       pointer_domain_t& left_ptr,
                                       var_to_cell_set_t right_cells,
                                       pointer_domain_t& right_ptr) {
    merge_cells_op op(left_ptr, right_ptr);
    var_to_cell_set_t result_cells(left_cells);
    result_cells.generic_merge_with(right_cells, op);
    return result_cells;
  }

  /// \brief Compare 2 map of cells
  static bool leq_cells(var_to_cell_set_t left_cells,
                        pointer_domain_t& left_ptr,
                        var_to_cell_set_t right_cells,
                        pointer_domain_t& right_ptr) {
    typedef patricia_tree_set< MemoryLocation > variable_set_t;

    // Get all variables
    variable_set_t variables;
    for (typename var_to_cell_set_t::iterator it = left_cells.begin();
         it != left_cells.end();
         ++it) {
      variables += it->first;
    }
    for (typename var_to_cell_set_t::iterator it = right_cells.begin();
         it != right_cells.end();
         ++it) {
      variables += it->first;
    }

    // Compare the set of cells
    for (typename variable_set_t::iterator it = variables.begin();
         it != variables.end();
         ++it) {
      MemoryLocation var = *it;
      boost::optional< cell_set_t > left_set = left_cells.lookup(var);
      boost::optional< cell_set_t > right_set = right_cells.lookup(var);

      if (left_set && right_set) {
        output_cell_set_iterator left_it = output_begin(*left_set);
        output_cell_set_iterator right_it = output_begin(*right_set);

        while (left_it != output_end(*left_set) &&
               right_it != output_end(*right_set)) {
          const Cell& left_c = *left_it;
          const Cell& right_c = *right_it;

          if (left_c == right_c) { // identical cells
            ++left_it;
            ++right_it;
          } else if (left_c.overlaps(right_c)) { // overlapping cells
            return false;
          } else if (left_c.offset() < right_c.offset()) {
            add_unchanged_relation(left_c, right_ptr);
            ++left_it;
          } else {
            add_unchanged_relation(right_c, left_ptr);
            ++right_it;
          }
        }

        for (; left_it != output_end(*left_set); ++left_it) {
          add_unchanged_relation(*left_it, right_ptr);
        }

        for (; right_it != output_end(*right_set); ++right_it) {
          add_unchanged_relation(*right_it, left_ptr);
        }
      } else if (left_set) {
        for (output_cell_set_iterator it2 = output_begin(*left_set);
             it2 != output_end(*left_set);
             ++it2) {
          add_unchanged_relation(*it2, right_ptr);
        }
      } else {
        for (output_cell_set_iterator it2 = output_begin(*right_set);
             it2 != output_end(*right_set);
             ++it2) {
          add_unchanged_relation(*it2, left_ptr);
        }
      }
    }

    return true;
  }

  // Compose summary_domains

  struct compose_cells_op : public var_to_cell_set_t::generic_binary_op_t {
    pointer_domain_t& _left_ptr;
    uninitialized_domain_t& _left_uninitialized;
    nullity_domain_t& _left_nullity;

    pointer_domain_t& _right_ptr;
    uninitialized_domain_t& _right_uninitialized;
    nullity_domain_t& _right_nullity;

    // temporary variables
    std::vector< VariableName > _tmp_vars;

    compose_cells_op(pointer_domain_t& left_ptr,
                     uninitialized_domain_t& left_uninitialized,
                     nullity_domain_t& left_nullity,
                     pointer_domain_t& right_ptr,
                     uninitialized_domain_t& right_uninitialized,
                     nullity_domain_t& right_nullity)
        : _left_ptr(left_ptr),
          _left_uninitialized(left_uninitialized),
          _left_nullity(left_nullity),
          _right_ptr(right_ptr),
          _right_uninitialized(right_uninitialized),
          _right_nullity(right_nullity) {}

    // Get a new temporary variable
    VariableName new_tmp_var(VariableName base) {
      VariableName tmp =
          var_name_traits::summary_domain_tmp_var< VariableName >()(base);
      _tmp_vars.push_back(tmp);
      return tmp;
    }

    void remove_temporary_vars(pointer_domain_t& ptr) const {
      ptr.forget(_tmp_vars.begin(), _tmp_vars.end());
    }

    boost::optional< cell_set_t > apply(cell_set_t left, cell_set_t right) {
      assert(!left.empty() && !right.empty());
      MemoryLocation base = (*left.begin()).base();
      cell_set_t result;

      // left input cells are kept
      result.insert(input_begin(left), input_end(left));

      // match right input cells with left output cells
      output_cell_set_iterator left_out_it = output_begin(left);
      input_cell_set_iterator right_in_it = input_begin(right);

      for (; right_in_it != input_end(right); ++right_in_it) {
        const Cell& right_in_c = *right_in_it;

        for (; left_out_it != output_end(left) &&
               left_out_it->range().ub() < right_in_c.offset();
             ++left_out_it)
          ;

        if (left_out_it == output_end(left)) {
          result.insert(right_in_c);
        } else if (right_in_c.range() == left_out_it->range()) { // matches
          VariableName tmp = new_tmp_var(right_in_c.scalar_var());
          _left_ptr.assign(tmp, linear_expression_t(left_out_it->scalar_var()));
          _right_ptr.assign(tmp, linear_expression_t(right_in_c.scalar_var()));
          _right_ptr -= right_in_c.scalar_var();
          _right_uninitialized -= right_in_c.scalar_var();
          _right_nullity -= right_in_c.scalar_var();
        } else if (right_in_c.overlaps(*left_out_it)) { // overlaps
          _right_ptr -= right_in_c.scalar_var();
          _right_uninitialized -= right_in_c.scalar_var();
          _right_nullity -= right_in_c.scalar_var();
        } else {
          result.insert(right_in_c);
        }
      }

      // right output cells are kept
      result.insert(output_begin(right), output_end(right));

      // if needed, shrink left output cells
      left_out_it = output_begin(left);
      output_cell_set_iterator right_out_it = output_begin(right);

      for (; left_out_it != output_end(left); ++left_out_it) {
        const Cell& left_out_c = *left_out_it;

        for (; right_out_it != output_end(right) &&
               right_out_it->range().ub() < left_out_c.offset();
             ++right_out_it)
          ;

        if (right_out_it == output_end(right)) {
          result.insert(left_out_c);
        } else if (left_out_c.overlaps(*right_out_it)) {
          _left_ptr -= left_out_c.scalar_var();
          _left_uninitialized -= left_out_c.scalar_var();
          _left_nullity -= left_out_c.scalar_var();
          interval_t range = left_out_c.range();

          while (!range.is_bottom() && right_out_it != output_end(right) &&
                 right_out_it->overlaps(range)) {
            interval_t meet = range & right_out_it->range();
            interval_t left(range.lb(), meet.lb() - 1);
            if (!left.is_bottom()) {
              result.insert(Cell(left_out_c.scalar_var()->var_factory(),
                                 base,
                                 left,
                                 Cell::Output));
            }

            if (meet.ub().is_finite()) {
              range = interval_t(meet.ub() + 1, range.ub());
            } else {
              range = interval_t::bottom();
            }

            if (!range.is_bottom()) {
              ++right_out_it;
            }
          }

          if (!range.is_bottom()) {
            result.insert(Cell(left_out_c.scalar_var()->var_factory(),
                               base,
                               range,
                               Cell::Output));
          }
        } else {
          result.insert(left_out_c);
        }
      }

      return boost::optional< cell_set_t >(result);
    }

    boost::optional< cell_set_t > apply_left(cell_set_t left) {
      return boost::optional< cell_set_t >(left);
    }

    boost::optional< cell_set_t > apply_right(cell_set_t right) {
      return boost::optional< cell_set_t >(right);
    }

  }; // end struct compose_cells_op

public:
  static summary_domain_t top() { return summary_domain_t(true); }

  static summary_domain_t bottom() { return summary_domain_t(false); }

  static summary_domain_t unchanged() {
    return summary_domain(false,
                          var_to_cell_set_t(),
                          pointer_domain_t::top(),
                          uninitialized_domain_t::top(),
                          nullity_domain_t::top());
  }

public:
  summary_domain()
      : _is_top(true),
        _cells(),
        _ptr(pointer_domain_t::top()),
        _uninitialized(uninitialized_domain_t::top()),
        _nullity(nullity_domain_t::top()) {}

  bool is_top() { return _is_top; }

  bool is_bottom() {
    if (!_is_top && (_ptr.is_bottom() || _uninitialized.is_bottom() ||
                     _nullity.is_bottom())) {
      *this = bottom();
      return true;
    } else {
      return false;
    }
  };

  bool operator<=(summary_domain_t o) {
    if (is_top()) {
      return o.is_top();
    } else if (o.is_top()) {
      return true;
    } else if (is_bottom()) {
      return true;
    } else if (o.is_bottom()) {
      return false;
    } else {
      pointer_domain_t left(_ptr);
      pointer_domain_t& right(o._ptr);
      return leq_cells(_cells, left, o._cells, right) && left <= right &&
             _uninitialized <= o._uninitialized && _nullity <= o._nullity;
    }
  }

  summary_domain_t operator|(summary_domain_t o) {
    if (is_top() || o.is_top()) {
      return summary_domain_t::top();
    } else if (is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      pointer_domain_t left(_ptr);
      pointer_domain_t& right(o._ptr);

      var_to_cell_set_t cells = merge_cells(_cells, left, o._cells, right);
      return summary_domain_t(false,
                              cells,
                              left | right,
                              _uninitialized | o._uninitialized,
                              _nullity | o._nullity);
    }
  }

  summary_domain_t operator||(summary_domain_t o) {
    if (is_top() || o.is_top()) {
      return summary_domain_t::top();
    } else if (is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      pointer_domain_t left(_ptr);
      pointer_domain_t& right(o._ptr);

      var_to_cell_set_t cells = merge_cells(_cells, left, o._cells, right);
      return summary_domain_t(false,
                              cells,
                              left || right,
                              _uninitialized || o._uninitialized,
                              _nullity || o._nullity);
    }
  }

  summary_domain_t join_loop(summary_domain_t o) { return this->operator|(o); }

  summary_domain_t join_iter(summary_domain_t o) { return this->operator|(o); }

  summary_domain_t operator&(summary_domain_t o) {
    if (is_top()) {
      return o;
    } else if (o.is_top()) {
      return *this;
    } else if (is_bottom() || o.is_bottom()) {
      return summary_domain_t::bottom();
    } else {
      pointer_domain_t left(_ptr);
      pointer_domain_t& right(o._ptr);

      var_to_cell_set_t cells = merge_cells(_cells, left, o._cells, right);
      return summary_domain_t(false,
                              cells,
                              left & right,
                              _uninitialized & o._uninitialized,
                              _nullity & o._nullity);
    }
  }

  summary_domain_t operator&&(summary_domain_t o) {
    if (is_top()) {
      return o;
    } else if (o.is_top()) {
      return *this;
    } else if (is_bottom() || o.is_bottom()) {
      return summary_domain_t::bottom();
    } else {
      pointer_domain_t left(_ptr);
      pointer_domain_t& right(o._ptr);

      var_to_cell_set_t cells = merge_cells(_cells, left, o._cells, right);
      return summary_domain_t(false,
                              cells,
                              left && right,
                              _uninitialized && o._uninitialized,
                              _nullity && o._nullity);
    }
  }

  /// \brief Simulate a function call, applying the summary o
  summary_domain_t compose(summary_domain_t o) {
    if (is_top() || o.is_top()) {
      return summary_domain_t::top();
    } else if (is_bottom() || o.is_bottom()) {
      return summary_domain_t::bottom();
    } else {
      pointer_domain_t left_ptr(_ptr);
      uninitialized_domain_t left_uninitialized(_uninitialized);
      nullity_domain_t left_nullity(_nullity);

      pointer_domain_t& right_ptr(o._ptr);
      uninitialized_domain_t& right_uninitialized(o._uninitialized);
      nullity_domain_t& right_nullity(o._nullity);

      compose_cells_op op(left_ptr,
                          left_uninitialized,
                          left_nullity,
                          right_ptr,
                          right_uninitialized,
                          right_nullity);
      var_to_cell_set_t cells(_cells);
      cells.generic_merge_with(o._cells, op);

      pointer_domain_t merge_ptr = left_ptr & right_ptr;
      op.remove_temporary_vars(merge_ptr);
      return summary_domain_t(false,
                              cells,
                              merge_ptr,
                              left_uninitialized & right_uninitialized,
                              left_nullity & right_nullity);
    }
  }

  void operator-=(VariableName v) { forget_mem_surface(v); }

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

  void assign(VariableName x, linear_expression_t e) {
    if (is_top())
      return;

    _ptr.assign(x, e);
  }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    if (is_top())
      return;

    _ptr.apply(op, x, y, z);
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    if (is_top())
      return;

    _ptr.apply(op, x, y, k);
  }

  void operator+=(linear_constraint_t cst) {
    if (is_top())
      return;

    _ptr += cst;
  }

  void operator+=(linear_constraint_system_t csts) {
    if (is_top())
      return;

    _ptr += csts;
  }

  void forget_num(VariableName v) {
    if (is_top())
      return;

    _ptr.forget_num(v);
  }

  template < typename Iterator >
  void forget_num(Iterator begin, Iterator end) {
    if (is_top())
      return;

    _ptr.forget_num(begin, end);
  }

  linear_constraint_system_t to_linear_constraint_system() {
    if (is_top())
      return linear_constraint_system_t();

    return _ptr.to_linear_constraint_system();
  }

  /*
   * Helpers for num_domain_traits
   */

  void normalize() {
    if (is_top())
      return;

    _ptr.normalize();
  }

  /// \brief Convert to another abstract domain
  ///
  /// uses num_domain_traits::convert() internally
  template < typename ScalarDomainTo >
  summary_domain< ScalarDomainTo, VariableFactory, MemoryLocation > convert() {
    return summary_domain<
        ScalarDomainTo,
        VariableFactory,
        MemoryLocation >(_is_top,
                         _cells,
                         num_domain_traits::convert<
                             pointer_domain_impl< ScalarDomain,
                                                  MemoryLocation >,
                             pointer_domain_impl< ScalarDomainTo,
                                                  MemoryLocation > >(_ptr),
                         _uninitialized,
                         _nullity);
  }

  interval_t to_interval(VariableName v, bool normalize) {
    if (is_top())
      return interval_t::top();

    return _ptr.to_interval(v, normalize);
  }

  interval_t to_interval(linear_expression_t e, bool normalize) {
    if (is_top())
      return interval_t::top();

    return _ptr.to_interval(e, normalize);
  }

  congruence_t to_congruence(VariableName v) {
    if (is_top())
      return congruence_t::top();

    return _ptr.to_congruence(v);
  }

  void from_interval(VariableName v, interval_t i) {
    if (is_top())
      return;

    _ptr.from_interval(v, i);
  }

  /*
   * Implement bitwise_operators
   */

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             uint64_t from,
             uint64_t to) {
    if (is_top())
      return;

    _ptr.apply(op, x, y, from, to);
  }

  void apply(conv_operation_t op,
             VariableName x,
             Number k,
             uint64_t from,
             uint64_t to) {
    if (is_top())
      return;

    _ptr.apply(op, x, k, from, to);
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    if (is_top())
      return;

    _ptr.apply(op, x, y, z);
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    if (is_top())
      return;

    _ptr.apply(op, x, y, k);
  }

  /*
   * Implement division_operators
   */

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    if (is_top())
      return;

    _ptr.apply(op, x, y, z);
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    if (is_top())
      return;

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
    if (is_top())
      return;

    _ptr.assign_object(p, obj);
  }

  void assign_pointer(VariableName p, VariableName q) {
    if (is_top())
      return;

    _ptr.assign_pointer(p, q);
  }

  void assign_pointer(VariableName p, VariableName q, VariableName o) {
    if (is_top())
      return;

    _ptr.assign_pointer(p, q, o);
  }

  void assign_pointer(VariableName p, VariableName q, Number o) {
    if (is_top())
      return;

    _ptr.assign_pointer(p, q, o);
  }

  void assert_pointer(bool equality, VariableName p, VariableName q) {
    if (is_top())
      return;

    _ptr.assert_pointer(equality, p, q);
  }

  void refine_addrs(VariableName p, points_to_set_t addrs) {
    if (is_top() || is_bottom())
      return;

    if (!_nullity.is_null(p) && !_uninitialized.is_uninitialized(p)) {
      _ptr.refine_addrs(p, addrs);
    }
  }

  void refine_addrs_offset(VariableName p,
                           points_to_set_t addrs,
                           interval_t offset) {
    if (is_top() || is_bottom())
      return;

    if (!_nullity.is_null(p) && !_uninitialized.is_uninitialized(p)) {
      _ptr.refine_addrs_offset(p, addrs, offset);
    }
  }

  bool is_unknown_addr(VariableName p) {
    if (is_top())
      return true;

    return _ptr.is_unknown_addr(p);
  }

  points_to_set_t addrs_set(VariableName p) {
    if (is_top())
      return points_to_set_t::top();

    return _ptr.addrs_set(p);
  }

  VariableName offset_var(VariableName p) { return _ptr.offset_var(p); }

  void forget_pointer(VariableName p) {
    if (is_top())
      return;

    _ptr.forget_pointer(p);
  }

  template < typename Iterator >
  void forget_pointer(Iterator begin, Iterator end) {
    if (is_top())
      return;

    _ptr.forget_pointer(begin, end);
  }

  /*
   * Implement uninitialized_domain
   */

  void make_initialized(VariableName v) {
    if (is_top())
      return;

    _uninitialized.make_initialized(v);
  }

  void make_uninitialized(VariableName v) {
    if (is_top())
      return;

    _uninitialized.make_uninitialized(v);
  }

  void assign_uninitialized(VariableName x, VariableName y) {
    if (is_top())
      return;

    _uninitialized.assign_uninitialized(x, y);
  }

  void assign_uninitialized(VariableName x, VariableName y, VariableName z) {
    if (is_top())
      return;

    _uninitialized.assign_uninitialized(x, y, z);
  }

  void assign_uninitialized(VariableName x,
                            const std::vector< VariableName >& y) {
    if (is_top())
      return;

    _uninitialized.assign_uninitialized(x, y);
  }

  bool is_initialized(VariableName v) {
    if (is_top())
      return false;

    return _uninitialized.is_initialized(v);
  }

  bool is_uninitialized(VariableName v) {
    if (is_top())
      return false;

    return _uninitialized.is_uninitialized(v);
  }

  void forget_uninitialized(VariableName v) {
    if (is_top())
      return;

    _uninitialized.forget_uninitialized(v);
  }

  template < typename Iterator >
  void forget_uninitialized(Iterator begin, Iterator end) {
    if (is_top())
      return;

    _uninitialized.forget_uninitialized(begin, end);
  }

  /*
   * Implement nullity_domain
   */

  void make_null(VariableName v) {
    if (is_top())
      return;

    _nullity.make_null(v);
  }

  void make_non_null(VariableName v) {
    if (is_top())
      return;

    _nullity.make_non_null(v);
  }

  void assign_nullity(VariableName x, VariableName y) {
    if (is_top())
      return;

    _nullity.assign_nullity(x, y);
  }

  void assert_null(VariableName v) {
    if (is_top())
      return;

    _nullity.assert_null(v);
    if (_nullity.is_bottom()) {
      *this = bottom();
    }
  }

  void assert_non_null(VariableName v) {
    if (is_top())
      return;

    _nullity.assert_non_null(v);
    if (_nullity.is_bottom()) {
      *this = bottom();
    }
  }

  void assert_nullity(bool equality, VariableName x, VariableName y) {
    if (is_top())
      return;

    _nullity.assert_nullity(equality, x, y);
    if (_nullity.is_bottom()) {
      *this = bottom();
    }
  }

  bool is_null(VariableName v) {
    if (is_top())
      return false;

    return _nullity.is_null(v);
  }

  bool is_non_null(VariableName v) {
    if (is_top())
      return false;

    return _nullity.is_non_null(v);
  }

  void forget_nullity(VariableName v) {
    if (is_top())
      return;

    return _nullity.forget_nullity(v);
  }

  template < typename Iterator >
  void forget_nullity(Iterator begin, Iterator end) {
    if (is_top())
      return;

    return _nullity.forget_nullity(begin, end);
  }

  /*
   * Implement memory_domain
   */

  void mem_write(VariableFactory& vfac,
                 VariableName pointer,
                 literal_t rhs,
                 Number size) {
    if (is_top() || is_bottom())
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

    interval_t offset_intv = _ptr.to_interval(offset_var(pointer), true);
    offset_intv = offset_intv & interval_t(0, bound_t::plus_infinity());

    if (offset_intv.is_bottom()) { // buffer underflow
      *this = bottom();
      return;
    }

    if (offset_intv.singleton()) {
      // The offset is an integer.
      for (typename points_to_set_t::iterator it = addrs.begin();
           it != addrs.end();
           ++it) {
        Cell c =
            realize_single_out_cell(vfac, *it, *offset_intv.singleton(), size);

        if (addrs.size() == 1) {
          strong_update(c.scalar_var(), rhs);
        } else {
          weak_update(c.scalar_var(), rhs);
        }
      }
    } else {
      // The offset is a range.
      for (typename points_to_set_t::iterator it = addrs.begin();
           it != addrs.end();
           ++it) {
        std::vector< Cell > cells =
            realize_range_out_cells(vfac, *it, offset_var(pointer), size);
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

    if (is_top() || is_bottom())
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
      bool first = true;
      for (typename points_to_set_t::iterator it = addrs.begin();
           it != addrs.end();
           ++it) {
        boost::optional< Cell > c =
            realize_single_in_cell(vfac, *it, *offset_intv.singleton(), size);

        if (!c) { // cannot realize the cell : just forget it
          _ptr -= lhs.var();
          _nullity -= lhs.var();
          _uninitialized -= lhs.var();
          break;
        }

        if (first) {
          strong_update(lhs, c->scalar_var());
          first = false;
        } else {
          weak_update(lhs, c->scalar_var());
        }
      }
    } else {
      // The offset is a range.
      _ptr -= lhs.var();
      _nullity -= lhs.var();
      _uninitialized -= lhs.var();
    }
  }

  void mem_copy(VariableFactory& vfac,
                VariableName dest,
                VariableName src,
                linear_expression_t size) {
    if (is_top() || is_bottom())
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
        _ptr.to_interval(size, true) & interval_t(0, bound_t::plus_infinity());

    if (dest_intv.is_bottom() || src_intv.is_bottom()) { // buffer underflow
      *this = bottom();
      return;
    }

    if (size_intv.is_bottom())
      return;

    if (dest_addrs.size() == 1 && dest_intv.singleton() &&
        !src_addrs.is_top() && src_intv.singleton()) {
      // in that only case, we can be more precise
      MemoryLocation dest_base = *dest_addrs.begin();
      Number dest_offset = *dest_intv.singleton();
      Number src_offset = *src_intv.singleton();
      Number size_lb = *size_intv.lb().number();
      interval_t updated_range(dest_offset,
                               bound_t(dest_offset) + size_intv.ub() - 1);

      // Remove updated output cells in dest
      boost::optional< cell_set_t > found = this->_cells.lookup(dest_base);
      if (found) {
        cell_set_t& cells = *found;

        for (output_cell_set_iterator it = output_begin(cells);
             it != output_end(cells);
             ++it) {
          const Cell& c = *it;

          if (c.overlaps(updated_range)) {
            updated_range = updated_range | c.range();
            remove_cell(c);
          }
        }
      }

      // Memory copy the source cells
      summary_domain_t prev = *this;
      bool first = true;

      for (typename points_to_set_t::iterator it = src_addrs.begin();
           it != src_addrs.end();
           ++it) {
        summary_domain_t inv = prev;
        found = prev._cells.lookup(*it);
        interval_t range = updated_range;

        if (found) {
          cell_set_t& cells = *found;

          for (output_cell_set_iterator c_it = output_begin(cells);
               c_it != output_end(cells);
               ++c_it) {
            const Cell& c = *c_it;

            if (c.range() <= interval_t(src_offset, src_offset + size_lb - 1)) {
              Cell out_cell(vfac,
                            dest_base,
                            dest_offset + c.offset() - src_offset,
                            c.size(),
                            Cell::Output);
              inv.add_cell(out_cell);
              inv.assign(out_cell.scalar_var(),
                         linear_expression_t(c.scalar_var()));

              // create a cell from range.lb
              // to dest_offset + c.offset - src_offset - 1 if needed
              if (range.lb() < dest_offset + c.offset() - src_offset) {
                inv.add_cell(
                    Cell(vfac,
                         dest_base,
                         interval_t(range.lb(),
                                    dest_offset + c.offset() - src_offset - 1),
                         Cell::Output));
              }

              if (range.ub() <= bound_t(dest_offset) + c.offset() - src_offset +
                                    c.size() - 1) {
                // end reached
                range = interval_t::bottom();
                break;
              } else {
                range = interval_t(bound_t(dest_offset) + c.offset() -
                                       src_offset + c.size(),
                                   range.ub());
              }
            }
          }
        }

        if (!range.is_bottom()) {
          inv.add_cell(Cell(vfac, dest_base, range, Cell::Output));
        }

        if (first) {
          *this = inv;
          first = false;
        } else {
          *this = *this | inv;
        }
      }
    } else {
      // to be sound, we remove all reachable cells
      forget_reachable_cells(dest, size_intv);
    }
  }

  void mem_set(VariableFactory& vfac,
               VariableName dest,
               linear_expression_t value,
               linear_expression_t size) {
    if (is_top() || is_bottom())
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

      for (typename points_to_set_t::iterator it = addrs.begin();
           it != addrs.end();
           ++it) {
        MemoryLocation base = *it;

        // possibly updated offsets
        interval_t range = dest_intv + interval_t(0, size_intv.ub() - 1);

        boost::optional< cell_set_t > found = this->_cells.lookup(base);
        if (found) {
          cell_set_t& cells = *found;

          for (output_cell_set_iterator it = output_begin(cells);
               it != output_end(cells);
               ++it) {
            const Cell& c = *it;

            if (c.range() <= safe_range) {
              if (addrs.size() == 1) {
                strong_update(c.scalar_var(), literal_t::integer(0));
              } else {
                weak_update(c.scalar_var(), literal_t::integer(0));
              }

              // create a cell from range.lb to c.offset - 1 if needed
              if (range.lb() < c.offset()) {
                add_cell(Cell(vfac,
                              base,
                              interval_t(range.lb(), c.offset() - 1),
                              Cell::Output));
              }

              if (range.ub() <= bound_t(c.offset()) + c.size() - 1) {
                // end reached
                range = interval_t::bottom();
                break;
              } else {
                range = interval_t(bound_t(c.offset()) + c.size(), range.ub());
              }
            } else if (c.overlaps(range)) {
              range = range | c.range();
              remove_cell(c);
            }
          }
        }

        if (!range.is_bottom()) {
          add_cell(Cell(vfac, base, range, Cell::Output));
        }
      }
    } else {
      // to be sound, we remove all reachable cells
      forget_reachable_cells(dest, size_intv);
    }
  }

  void forget_mem_surface(VariableName v) {
    if (is_top() || is_bottom())
      return;

    _ptr.forget(v);
    _nullity.forget(v);
    _uninitialized.forget(v);
  }

  template < typename Iterator >
  void forget_mem_surface(Iterator begin, Iterator end) {
    if (is_top() || is_bottom())
      return;

    _ptr.forget(begin, end);
    _nullity.forget(begin, end);
    _uninitialized.forget(begin, end);
  }

  void forget_mem_contents() { *this = top(); }

  void forget_mem_contents(VariableName p) { forget_reachable_cells(p); }

  void forget_mem_contents(VariableName p, Number size) {
    forget_reachable_cells(p, size);
  }

  void write(std::ostream& o) {
    if (is_top()) {
      o << "T";
    } else if (is_bottom()) {
      o << "_|_";
    } else {
      o << "({";
      for (typename var_to_cell_set_t::iterator it = _cells.begin();
           it != _cells.end();) {
        o << it->first << " -> {";
        cell_set_t cells = it->second;
        for (typename cell_set_t::iterator it2 = cells.begin();
             it2 != cells.end();) {
          o << *it2;
          ++it2;
          if (it2 != cells.end())
            o << ", ";
        }
        o << "}";
        ++it;
        if (it != _cells.end())
          o << "; ";
      }
      o << "}, " << _ptr << ", " << _uninitialized << ", " << _nullity << ")";
    }
  }

  static std::string domain_name() {
    return "Summary of " + ScalarDomain::domain_name();
  }

  // required for convert()
  template < typename Any, typename VarFactory, typename MemLoc >
  friend class summary_domain;

}; // end class summary_domain

namespace num_domain_traits {
namespace detail {

template < typename ScalarDomain,
           typename VariableFactory,
           typename MemoryLocation >
struct normalize_impl<
    summary_domain< ScalarDomain, VariableFactory, MemoryLocation > > {
  inline void operator()(
      summary_domain< ScalarDomain, VariableFactory, MemoryLocation >& inv) {
    inv.normalize();
  }
};

template < typename ScalarDomainFrom,
           typename ScalarDomainTo,
           typename VariableFactory,
           typename MemoryLocation >
struct convert_impl<
    summary_domain< ScalarDomainFrom, VariableFactory, MemoryLocation >,
    summary_domain< ScalarDomainTo, VariableFactory, MemoryLocation > > {
  inline summary_domain< ScalarDomainTo, VariableFactory, MemoryLocation >
  operator()(
      summary_domain< ScalarDomainFrom, VariableFactory, MemoryLocation > inv) {
    return inv.template convert< ScalarDomainTo >();
  }
};

/// \brief Fix for ambiguousity between convert_impl< X, X >
///  and convert_impl< summary_domain< X >, summary_domain< Y > >
template < typename ScalarDomain,
           typename VariableFactory,
           typename MemoryLocation >
struct convert_impl<
    summary_domain< ScalarDomain, VariableFactory, MemoryLocation >,
    summary_domain< ScalarDomain, VariableFactory, MemoryLocation > > {
  inline summary_domain< ScalarDomain, VariableFactory, MemoryLocation >
  operator()(
      summary_domain< ScalarDomain, VariableFactory, MemoryLocation > inv) {
    return inv;
  }
};

template < typename ScalarDomain,
           typename VariableFactory,
           typename MemoryLocation >
struct var_to_interval_impl<
    summary_domain< ScalarDomain, VariableFactory, MemoryLocation > > {
  inline interval< typename ScalarDomain::number_t > operator()(
      summary_domain< ScalarDomain, VariableFactory, MemoryLocation >& inv,
      typename ScalarDomain::variable_name_t v,
      bool normalize) {
    return inv.to_interval(v, normalize);
  }
};

template < typename ScalarDomain,
           typename VariableFactory,
           typename MemoryLocation >
struct lin_expr_to_interval_impl<
    summary_domain< ScalarDomain, VariableFactory, MemoryLocation > > {
  inline interval< typename ScalarDomain::number_t > operator()(
      summary_domain< ScalarDomain, VariableFactory, MemoryLocation >& inv,
      typename ScalarDomain::linear_expression_t e,
      bool normalize) {
    return inv.to_interval(e, normalize);
  }
};

template < typename ScalarDomain,
           typename VariableFactory,
           typename MemoryLocation >
struct from_interval_impl<
    summary_domain< ScalarDomain, VariableFactory, MemoryLocation > > {
  inline void operator()(
      summary_domain< ScalarDomain, VariableFactory, MemoryLocation >& inv,
      typename ScalarDomain::variable_name_t v,
      interval< typename ScalarDomain::number_t > i) {
    inv.from_interval(v, i);
  }
};

} // end namespace detail
} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_SUMMARY_DOMAIN_HPP
