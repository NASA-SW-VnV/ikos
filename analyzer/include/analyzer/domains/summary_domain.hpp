/*******************************************************************************
 *
 * A value abstract domain able to build function summaries using an underlying
 * numerical relational abstract domain. The domain is based on Antoine Mine's
 * paper and Jorge Navas's implementation in value_domain.hpp
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
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

#ifndef ANALYZER_SUMMARY_DOMAIN_HPP
#define ANALYZER_SUMMARY_DOMAIN_HPP

#include <boost/iterator/filter_iterator.hpp>

#include <analyzer/ikos-wrapper/domains_traits.hpp>
#include <analyzer/domains/value_domain.hpp>

namespace ikos {

namespace summary_domain_impl {

// A cell is a quadruple <V, o, size, type> modelling all bytes starting at
// offset o in variable V up to (o + size - 1). The type indicates whether it
// refers to the old value (Input) or the new value (Output).
//
// Note that the offset is an integer and the size is a bound (can be +oo).
template < typename VariableName, typename Number >
class Cell {
private:
  typedef bound< Number > bound_t;
  typedef interval< Number > interval_t;
  typedef variable< Number, VariableName > variable_t;

public:
  typedef enum { Input, Output } cell_type_t;

private:
  VariableName _base;
  Number _offset;
  bound_t _size;
  cell_type_t _type;
  VariableName _scalar_var;

private:
  inline std::string name() const {
    std::ostringstream buf;
    buf << "C{" << _base.name() << "," << _offset << "," << _size << ","
        << ((_type == Input) ? "I" : "O") << "}";
    return buf.str();
  }

public:
  Cell(VariableName base, Number offset, bound_t size, cell_type_t type)
      : _base(base),
        _offset(offset),
        _size(size),
        _type(type),
        _scalar_var(base) {
    assert(_offset >= 0 && _size >= 1);
    _scalar_var = _base.getVarFactory()[name()];
  }

  Cell(VariableName base, interval_t range, cell_type_t type)
      : _base(base),
        _offset(*range.lb().number()),
        _size(range.ub() - range.lb() + 1),
        _type(type),
        _scalar_var(base) {
    assert(!range.is_bottom() && range.lb().number());
    _scalar_var = _base.getVarFactory()[name()];
  }

  VariableName base() const { return _base; }
  Number offset() const { return _offset; }
  bound_t size() const { return _size; }
  cell_type_t type() const { return _type; }
  bool is_input() const { return _type == Input; }
  bool is_output() const { return _type == Output; }
  VariableName scalar_var() const { return _scalar_var; }

  bool operator<(const Cell& o) const {
    // lexicographical order ensures that !(c1 < c2) && !(c2 < c1) => c1 == c2
    // it is needed to use std::set< Cell >
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

  // Return the number of possible overlaps between the cell and the write at
  // offset `offset` of size `size`
  template < typename PointerDomain >
  bound_t number_overlaps(VariableName offset,
                          Number size,
                          PointerDomain inv) const {
    inv += (variable_t(offset) + size - 1 >= _offset);

    if (_size.is_finite()) {
      inv += (variable_t(offset) <= _offset + *_size.number() - 1);
    }

    interval_t rng = inv[offset];
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
    if (!(_size.is_finite() && *_size.number() == size))
      return false;

    inv += (variable_t(offset) == _offset);
    return !inv.is_bottom();
  }

  void write(std::ostream& o) { o << name(); }

  friend std::ostream& operator<<(std::ostream& o, Cell c) {
    c.write(o);
    return o;
  }
};

} // end namespace summary_domain_impl

template < typename ScalarDomain, typename VariableName, typename Number >
class summary_domain : public writeable,
                       public numerical_domain< Number, VariableName >,
                       public bitwise_operators< Number, VariableName >,
                       public division_operators< Number, VariableName > {
public:
  typedef bound< Number > bound_t;
  typedef interval< Number > interval_t;
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;
  typedef patricia_tree_set< variable_t > variable_set_t;
  typedef discrete_domain< VariableName > points_to_set_t;
  typedef ScalarDomain scalar_domain_t;
  typedef summary_domain< ScalarDomain, VariableName, Number > summary_domain_t;

private:
  typedef summary_domain_impl::Cell< VariableName, Number > Cell;
  typedef std::set< Cell > cell_set_t;
  typedef patricia_tree< VariableName, cell_set_t > var_to_cell_set_t;
  typedef pointer_domain< ScalarDomain, VariableName, Number > pointer_domain_t;
  typedef uninitialized_domain< VariableName > uninitialized_domain_t;
  typedef nullity_domain< VariableName > nullity_domain_t;

private:
  /* _cells is a map from a memory object to a set of cells (ordered by offset).
   * There are 2 kind of cells: Input cell and Output cell.
   *
   * An output cell means that the value of the bytes [cell.offset, cell.offset
   * + cell.size - 1] has changed, and the new value is approximated in the
   * underlying abstract domain. No cell means the object is unchanged.
   *
   * An input cell refers to the value of the bytes [cell.offset, cell.offset +
   * cell.size - 1] before the call to the current function.
   *
   * If _cells is empty, it means nothing has changed, thus output = input.
   * This is different from top. That's why we need a special member _is_top to
   * indicate that everything has possibly changed.
   */

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

    // remove the cell from the pointer+scalar abstraction
    _ptr -= c.scalar_var();

    // remove the cell from the nullity abstraction
    _nullity -= c.scalar_var();

    // remove the cell from the uninitialized variable abstraction
    _uninitialized -= c.scalar_var();
  }

  // Shrink the cell `shrunk` to remove all the bytes of cell `overlap`
  void shrink_cell(const Cell& shrunk, const Cell& overlap) {
    interval_t range = shrunk.range();
    interval_t meet = range & overlap.range();
    assert(!meet.is_bottom());

    interval_t left(range.lb(), meet.lb() - 1);
    if (!left.is_bottom()) {
      add_cell(Cell(shrunk.base(), left, shrunk.type()));
    }

    interval_t right(meet.ub() + 1, range.ub());
    if (!right.is_bottom()) {
      add_cell(Cell(shrunk.base(), right, shrunk.type()));
    }

    remove_cell(shrunk);
  }

  // Create a new cell for a write, performing reduction if possible
  Cell realize_single_out_cell(VariableName base, Number offset, Number size) {
    Cell c(base, offset, size, Cell::Output);

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
            shrink_cell(*it, c);
          }
        }

        // TODO: perform a reduction to improve precision
        add_cell(c);
        return c;
      }
    }
  }

  // Create a new cell for a read
  boost::optional< Cell > realize_single_in_cell(VariableName base,
                                                 Number offset,
                                                 Number size) {
    Cell c_in(base, offset, size, Cell::Input);
    Cell c_out(base, offset, size, Cell::Output);

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

  // Perform a write with an approximated offset.
  // returns a list of cells on which we should perform a weak update.
  std::vector< Cell > realize_range_out_cells(VariableName base,
                                              VariableName offset,
                                              Number size) {
    // get the interval
    interval_t offset_intv = _ptr[offset];
    offset_intv = offset_intv & interval_t(0, bound_t::plus_infinity());
    assert(!offset_intv.is_bottom());
    interval_t range = offset_intv + interval_t(0, size - 1);

    boost::optional< cell_set_t > found = this->_cells.lookup(base);
    if (!found) { // no cell, create a cell covering all bytes
      add_cell(Cell(base, range, Cell::Output));
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
              add_cell(Cell(base,
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
        add_cell(Cell(base, range, Cell::Output));
      }

      return updated_cells;
    }
  }

  // Perform a strong update `x = e`
  void strong_update(VariableName x, linear_expression_t e, bool is_pointer) {
    _ptr.assign(x, e);

    std::vector< VariableName > rhs = get_variables(e);
    if (is_pointer) {
      _nullity.assign(x, rhs);
    } else {
      _uninitialized.assign(x, rhs);
    }
  }

  // Perform a weak update `x = e`
  void weak_update(VariableName x, linear_expression_t e, bool is_pointer) {
    pointer_domain_t ptr_inv(_ptr);
    ptr_inv.assign(x, e);
    _ptr = _ptr | ptr_inv;

    std::vector< VariableName > rhs = get_variables(e);
    if (is_pointer) {
      nullity_domain_t inv(_nullity);
      inv.assign(x, rhs);
      _nullity = _nullity | inv;
    } else {
      uninitialized_domain_t inv(_uninitialized);
      inv.assign(x, rhs);
      _uninitialized = _uninitialized | inv;
    }
  }

  // Get all variables in a linear expression
  std::vector< VariableName > get_variables(linear_expression_t e) const {
    std::vector< VariableName > vs;
    for (typename linear_expression_t::iterator it = e.begin(); it != e.end();
         ++it) {
      vs.push_back(it->second.name());
    }
    return vs;
  }

  //! Forget all memory cells that can be accessible through pointer o
  void forget_reachable_cells(VariableName o) {
    points_to_set_t addrs_set = _ptr.get_addrs_set(o);

    if (addrs_set.is_top() || addrs_set.is_bottom())
      return;

    for (typename points_to_set_t::iterator it = addrs_set.begin();
         it != addrs_set.end();
         ++it) {
      forget_cells(*it);
    }
  }

  //! Forget all memory cells that can be accessible through pointer o and that
  //  overlap with [o, ..., o + size - 1]
  void forget_reachable_cells(VariableName o, interval_t size) {
    points_to_set_t addrs_set = _ptr.get_addrs_set(o);

    if (addrs_set.is_top() || addrs_set.is_bottom())
      return;

    for (typename points_to_set_t::iterator it = addrs_set.begin();
         it != addrs_set.end();
         ++it) {
      forget_cells(*it, o, size);
    }
  }

  //! Forget all memory cells of base address `base`
  void forget_cells(VariableName base) {
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
    add_cell(Cell(base, 0, bound_t::plus_infinity(), Cell::Output));
  }

  //! Forget all memory cells of base address `base` that overlap
  //  with [offset, ..., offset + size - 1]
  void forget_cells(VariableName base, VariableName offset, interval_t size) {
    interval_t offset_intv =
        _ptr[offset] & interval_t(0, bound_t::plus_infinity());
    size = size & interval_t(1, bound_t::plus_infinity());
    interval_t range = offset_intv + interval_t(0, size.ub() - 1);
    forget_cells(base, range);
  }

  //! Forget all memory cells of base address `base` that overlap
  //  with [offset, ..., offset + size - 1]
  void forget_cells(VariableName base, interval_t range) {
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
    add_cell(Cell(base, range, Cell::Output));
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

  // Add the relation C{a,o,s,O} = C{a,o,s,I}
  static void add_unchanged_relation(const Cell& c, pointer_domain_t& ptr) {
    assert(c.is_output());
    Cell in_cell(c.base(), c.offset(), c.size(), Cell::Input);
    ptr.assign(c.scalar_var(), linear_expression_t(in_cell.scalar_var()));
  }

  // Add the relation C{a,o,s,O} = C{a,o,s,I}
  static void add_unchanged_relation(const Cell& c,
                                     cell_set_t& cells,
                                     pointer_domain_t& ptr) {
    assert(c.is_output());
    Cell in_cell(c.base(), c.offset(), c.size(), Cell::Input);
    cells.insert(in_cell);
    ptr.assign(c.scalar_var(), linear_expression_t(in_cell.scalar_var()));
  }

  struct merge_cells_op : public var_to_cell_set_t::generic_binary_op_t {
    // surface domains
    pointer_domain_t& _left_ptr;
    pointer_domain_t& _right_ptr;

    merge_cells_op(pointer_domain_t& left_ptr, pointer_domain_t& right_ptr)
        : _left_ptr(left_ptr), _right_ptr(right_ptr) {}

    // merge 2 set of cells
    boost::optional< cell_set_t > apply(cell_set_t left, cell_set_t right) {
      assert(!left.empty() && !right.empty());
      VariableName base = (*left.begin()).base();
      cell_set_t result;

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

          result.insert(Cell(base, range, Cell::Output));
        } else if (left_c.offset() < right_c.offset()) {
          result.insert(left_c);
          add_unchanged_relation(left_c, result, _right_ptr);
          ++left_it;
        } else {
          result.insert(right_c);
          add_unchanged_relation(right_c, result, _left_ptr);
          ++right_it;
        }
      }

      for (; left_it != output_end(left); ++left_it) {
        result.insert(*left_it);
        add_unchanged_relation(*left_it, result, _right_ptr);
      }

      for (; right_it != output_end(right); ++right_it) {
        result.insert(*right_it);
        add_unchanged_relation(*right_it, result, _left_ptr);
      }

      return boost::optional< cell_set_t >(result);
    }

    boost::optional< cell_set_t > apply_left(cell_set_t left) {
      cell_set_t result = left;

      for (output_cell_set_iterator it = output_begin(left);
           it != output_end(left);
           ++it) {
        add_unchanged_relation(*it, result, _right_ptr);
      }

      return boost::optional< cell_set_t >(result);
    }

    boost::optional< cell_set_t > apply_right(cell_set_t right) {
      cell_set_t result = right;

      for (output_cell_set_iterator it = output_begin(right);
           it != output_end(right);
           ++it) {
        add_unchanged_relation(*it, result, _left_ptr);
      }

      return boost::optional< cell_set_t >(result);
    }

  }; // end struct merge_cells_op

  // Merge 2 map of cells
  static var_to_cell_set_t merge_cells(var_to_cell_set_t left_cells,
                                       pointer_domain_t& left_ptr,
                                       var_to_cell_set_t right_cells,
                                       pointer_domain_t& right_ptr) {
    merge_cells_op op(left_ptr, right_ptr);
    var_to_cell_set_t result_cells(left_cells);
    result_cells.generic_merge_with(right_cells, op);
    return result_cells;
  }

  // Compare 2 map of cells
  static bool leq_cells(var_to_cell_set_t left_cells,
                        pointer_domain_t& left_ptr,
                        var_to_cell_set_t right_cells,
                        pointer_domain_t& right_ptr) {
    typedef patricia_tree_set< VariableName > variable_set_t;

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
      VariableName var = *it;
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
    VariableName new_tmp_var(analyzer::VariableFactory& vfac) {
      VariableName tmp = vfac["shadow.tmp." + std::to_string(_tmp_vars.size())];
      _tmp_vars.push_back(tmp);
      return tmp;
    }

    void remove_temporary_vars(pointer_domain_t& ptr) const {
      for (typename std::vector< VariableName >::const_iterator it =
               _tmp_vars.begin();
           it != _tmp_vars.end();
           ++it) {
        ptr -= (*it);
      }
    }

    boost::optional< cell_set_t > apply(cell_set_t left, cell_set_t right) {
      assert(!left.empty() && !right.empty());
      VariableName base = (*left.begin()).base();
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
          VariableName tmp = new_tmp_var(base.getVarFactory());
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
              result.insert(Cell(base, left, Cell::Output));
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
            result.insert(Cell(base, range, Cell::Output));
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
    if (is_top())
      return o.is_top();
    else if (o.is_top())
      return true;
    else if (is_bottom())
      return true;
    else if (o.is_bottom())
      return false;
    else {
      pointer_domain_t left(_ptr);
      pointer_domain_t& right(o._ptr);
      return leq_cells(_cells, left, o._cells, right) && left <= right &&
             _uninitialized <= o._uninitialized && _nullity <= o._nullity;
    }
  }

  summary_domain_t operator|(summary_domain_t o) {
    if (is_top() || o.is_top())
      return summary_domain_t::top();
    else if (is_bottom())
      return o;
    else if (o.is_bottom())
      return *this;
    else {
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
    if (is_top() || o.is_top())
      return summary_domain_t::top();
    else if (is_bottom())
      return o;
    else if (o.is_bottom())
      return *this;
    else {
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

  summary_domain_t operator&(summary_domain_t o) {
    if (is_top())
      return o;
    else if (o.is_top())
      return *this;
    else if (is_bottom() || o.is_bottom())
      return summary_domain_t::bottom();
    else {
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
    if (is_top())
      return o;
    else if (o.is_top())
      return *this;
    else if (is_bottom() || o.is_bottom())
      return summary_domain_t::bottom();
    else {
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

  // Simulate a function call, applying the summary o
  summary_domain_t compose(summary_domain_t o) {
    if (is_top() || o.is_top())
      return summary_domain_t::top();
    else if (is_bottom() || o.is_bottom())
      return summary_domain_t::bottom();
    else {
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

  // Forget both the surface of v (base addresses and offsets) and
  // all the memory contents that may be accessible through v.
  void operator-=(VariableName v) {
    if (is_top())
      return;

    forget_mem_contents(v); // has to be before forget_mem_surface
    forget_mem_surface(v);
  }

  void assign(VariableName x, linear_expression_t e) {
    if (is_top())
      return;

    boost::optional< variable_t > y = e.get_variable();
    if (y && ((*y).name() == x))
      return;

    // Only affect the "surface" semantics so no need to kill cells.
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

  void operator+=(linear_constraint_system_t cst) {
    if (is_top())
      return;

    _ptr += cst;
  }

  interval_t operator[](VariableName x) {
    if (is_top())
      return interval_t::top();
    else if (is_bottom())
      return interval_t::bottom();

    return _ptr[x];
  }

  void set(VariableName x, interval_t intv) {
    if (is_top())
      return;

    _ptr.set(x, intv);
  }

  /*
   * bitwise_operators_api and division_operators_api
   */

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             unsigned width) {
    if (is_top())
      return;

    _ptr.apply(op, x, y, width);
  }

  void apply(conv_operation_t op, VariableName x, Number k, unsigned width) {
    if (is_top())
      return;

    _ptr.apply(op, x, k, width);
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

  void write(std::ostream& o) {
    if (is_top())
      o << "T";
    else if (is_bottom())
      o << "_|_";
    else {
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

  /*
   * Non-standard operations in numerical abstract domains
   */

  // create a new memory object
  void make_object(VariableName x) {
    if (is_top())
      return;

    _ptr.make_object(x);
  }

  // x points to y
  void assign_ptr(VariableName x, VariableName y) {
    if (is_top())
      return;

    _ptr.assign_ptr(x, y);
  }

  // return true if the set of possible addresses of x is unknown
  bool is_unknown_addr(VariableName x) {
    if (is_top())
      return true;

    return _ptr.get_addrs_set(x).is_top();
  }

  // return the set of possible addresses of x
  std::vector< VariableName > get_addrs_set(VariableName x) {
    assert(!is_unknown_addr(x));
    // points_to_set does not satisfy requirements of boost::copy
    points_to_set_t addrs = _ptr.get_addrs_set(x);

    std::vector< VariableName > res;
    if (!addrs.is_bottom()) {
      for (typename points_to_set_t::iterator it = addrs.begin();
           it != addrs.end();
           ++it) {
        res.push_back(*it);
      }
    }
    return res;
  }

  // reduction step: refine the set of possible addresses
  void refine_addrs(VariableName v, points_to_set_t addrs) {
    if (is_top() || is_bottom())
      return;

    // addrs is just a set of addresses to which v can
    // point-to. However, v can be still null. To be safe, we refine
    // the addresses of v only if we know that v cannot be null.
    if (!is_null(v)) {
      _ptr.meet_addrs_set(v, addrs);
    }
  }

  // reduction step: refine the set of possible addresses and offset
  void refine_addrs_and_offset(VariableName v_addr,
                               points_to_set_t addrs,
                               VariableName v_offset,
                               z_interval offset) {
    if (is_top() || is_bottom())
      return;

    // refine addresses
    refine_addrs(v_addr, addrs);

    // refine offset
    summary_domain_t other(*this);
    num_abstract_domain_impl::from_interval(other, v_offset, offset);
    *this = *this & other;
  }

  // v is definitely an uninitialized variable.
  void make_uninitialized(VariableName v) {
    if (is_top())
      return;

    _uninitialized.set(v, uninitialized_value::uninitialized());
  }

  // return if v is uninitialized
  bool is_uninitialized(VariableName v) {
    if (is_top())
      return false;

    return _uninitialized[v].is_uninitialized();
  }

  // v is definitely an initialized variable.
  void make_initialized(VariableName v) {
    if (is_top())
      return;

    _uninitialized.set(v, uninitialized_value::initialized());
  }

  // return if v is initialized
  bool is_initialized(VariableName v) {
    if (is_top())
      return false;

    return _uninitialized[v].is_initialized();
  }

  // v is definitely null
  void make_null(VariableName v) {
    if (is_top())
      return;

    _nullity.set(v, nullity_value::null());
  }

  // return if v is null
  bool is_null(VariableName v) {
    if (is_top())
      return false;

    return _nullity[v].is_null();
  }

  // v is definitely non null
  void make_non_null(VariableName v) {
    if (is_top())
      return;

    _nullity.set(v, nullity_value::non_null());
  }

  // return if v is not null
  bool is_non_null(VariableName v) {
    if (is_top())
      return false;

    return _nullity[v].is_non_null();
  }

  // propagates y's nullity value to x
  void assign_nullity(VariableName x, VariableName y) {
    if (is_top())
      return;

    _nullity.assign(x, y);
  }

  // propagate ys's uninitialized values to x
  void assign_uninitialized(VariableName x,
                            const std::vector< VariableName >& ys) {
    if (is_top())
      return;

    _uninitialized.assign(x, ys);
  }

  // memory write: store e at memory object associated with offset in
  // bytes [offset, offset + size - 1].
  void mem_write(VariableName offset,
                 Number size,
                 linear_expression_t e,
                 bool is_pointer) {
    if (is_top() || is_bottom())
      return;

    if (is_null(offset)) { // null dereference!
      *this = bottom();
      return;
    }

    // get memory locations associated with offset
    points_to_set_t addrs = _ptr.get_addrs_set(offset);
    if (addrs.is_top()) {
#if 1
      std::cerr << "Ignored memory write to " << offset << " with value " << e
                << std::endl;
#endif
      return;
    }

    interval_t offset_intv = _ptr[offset];
    offset_intv = offset_intv & interval_t(0, bound_t::plus_infinity());

    if (offset_intv.is_bottom()) {
      *this = bottom();
      return;
    }

    if (offset_intv.singleton()) {
      // The offset is an integer.
      for (typename points_to_set_t::iterator it = addrs.begin();
           it != addrs.end();
           ++it) {
        Cell c = realize_single_out_cell(*it, *offset_intv.singleton(), size);

        if (addrs.size() == 1) {
          strong_update(c.scalar_var(), e, is_pointer);
        } else {
          weak_update(c.scalar_var(), e, is_pointer);
        }
      }
    } else {
      // The offset is a range.
      for (typename points_to_set_t::iterator it = addrs.begin();
           it != addrs.end();
           ++it) {
        std::vector< Cell > cells = realize_range_out_cells(*it, offset, size);
        for (typename std::vector< Cell >::iterator it2 = cells.begin();
             it2 != cells.end();
             ++it2) {
          weak_update(it2->scalar_var(), e, is_pointer);
        }
      }
    }
  }

  // memory read: read from memory object associated with offset from
  // bytes [offset, offset + size - 1]
  void mem_read(VariableName lhs,
                VariableName offset,
                Number size,
                bool is_pointer) {
    if (is_top() || is_bottom())
      return;

    if (is_null(offset)) { // null dereference!
      *this = bottom();
      return;
    }

    // get memory locations associated with offset
    points_to_set_t addrs = _ptr.get_addrs_set(offset);
    if (addrs.is_top()) {
#if 1
      std::cerr << "Ignored memory read from offset " << offset << std::endl;
#endif
      return;
    }

    interval_t offset_intv = _ptr[offset];
    offset_intv = offset_intv & interval_t(0, bound_t::plus_infinity());

    if (offset_intv.is_bottom()) {
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
            realize_single_in_cell(*it, *offset_intv.singleton(), size);

        if (!c) { // cannot realize the cell : just forget it
          _ptr -= lhs;
          _nullity -= lhs;
          _uninitialized -= lhs;
          break;
        }

        if (first) {
          strong_update(lhs, variable_t(c->scalar_var()), is_pointer);
          first = false;
        } else {
          weak_update(lhs, variable_t(c->scalar_var()), is_pointer);
        }
      }
    } else {
      // The offset is a range.
      _ptr -= lhs;
      _nullity -= lhs;
      _uninitialized -= lhs;
    }
  }

  //! copy bytes {src, ..., src + size - 1} to {dest, ..., dest + size - 1}
  void mem_copy(VariableName dest, VariableName src, linear_expression_t size) {
    if (is_top() || is_bottom())
      return;

    if (is_null(dest) || is_null(src)) { // null dereference!
      *this = bottom();
      return;
    }

    // Get memory locations associated with dest and src
    points_to_set_t src_addrs = _ptr.get_addrs_set(src);
    points_to_set_t dest_addrs = _ptr.get_addrs_set(dest);

    if (dest_addrs.is_top()) {
#if 0
      std::cout << "Ignored memory copy from " << src << " to " << dest << std::endl;
#endif
      return;
    }

    // Get size and pointer offsets approximation
    interval_t dest_intv = _ptr[dest] & interval_t(0, bound_t::plus_infinity());
    interval_t src_intv = _ptr[src] & interval_t(0, bound_t::plus_infinity());
    interval_t size_intv = _ptr[size] & interval_t(0, bound_t::plus_infinity());
    assert(!dest_intv.is_bottom() && !src_intv.is_bottom());

    if (size_intv.is_bottom())
      return;

    if (dest_addrs.size() == 1 && dest_intv.singleton() &&
        !src_addrs.is_top() && src_intv.singleton()) {
      // in that only case, we can be more precise
      VariableName dest_base = *dest_addrs.begin();
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
              Cell out_cell(dest_base,
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
                    Cell(dest_base,
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
          inv.add_cell(Cell(dest_base, range, Cell::Output));
        }

        if (first) {
          *this = inv;
          first = false;
        } else {
          *this = *this | inv;
        }
      }
    } else {
      // To be sound, we remove all reachable cells
      forget_reachable_cells(dest, size_intv);
    }
  }

  //! set bytes {src, ..., src + size - 1} to value
  void mem_set(VariableName dest,
               linear_expression_t value,
               linear_expression_t size) {
    if (is_top() || is_bottom())
      return;

    if (is_null(dest)) { // null dereference!
      *this = bottom();
      return;
    }

    // Get memory locations
    points_to_set_t addrs = _ptr.get_addrs_set(dest);

    if (addrs.is_top()) {
#if 0
      std::cout << "Ignored memory set on " << dest << std::endl;
#endif
      return;
    }

    // Get size and pointer offsets approximation
    interval_t dest_intv = _ptr[dest] & interval_t(0, bound_t::plus_infinity());
    interval_t size_intv = _ptr[size] & interval_t(1, bound_t::plus_infinity());
    assert(!dest_intv.is_bottom());

    if (size_intv.is_bottom())
      return;

    if (_ptr[value] == interval_t(Number(0))) { // memory set to 0
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
        VariableName base = *it;

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
                strong_update(c.scalar_var(), 0, false);
              } else {
                weak_update(c.scalar_var(), 0, false);
              }

              // create a cell from range.lb to c.offset - 1 if needed
              if (range.lb() < c.offset()) {
                add_cell(Cell(base,
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
          add_cell(Cell(base, range, Cell::Output));
        }
      }
    } else {
      // To be sound, we remove all reachable cells
      forget_reachable_cells(dest, size_intv);
    }
  }

  // assert whether x == null or x != null
  void cmp_mem_addr_null(bool is_equality, VariableName x) {
    if (is_top() || is_bottom())
      return;

    _nullity.assertion(is_equality, x, nullity_value::null());

    if (_nullity.is_bottom()) {
      *this = bottom();
    }
  }

  // assert whether x == y or x != y
  void cmp_mem_addr(bool is_equality, VariableName x, VariableName y) {
    if (is_top() || is_bottom())
      return;

    _nullity.assertion(is_equality, x, y);

    if (_nullity.is_bottom()) {
      *this = bottom();
      return;
    }

    points_to_set_t addrs_x = _ptr.get_addrs_set(x);
    points_to_set_t addrs_y = _ptr.get_addrs_set(y);

    if (is_equality) {
      points_to_set_t addrs_xy = addrs_x & addrs_y;

      if (addrs_xy.is_bottom()) {
        *this = bottom();
        return;
      }

      // x and y's points-to sets
      _ptr.set_addrs_set(x, addrs_xy);
      _ptr.set_addrs_set(y, addrs_xy);

      // x and y's offsets
      _ptr += (linear_expression_t(x) == linear_expression_t(y));

      if (_ptr.is_bottom()) {
        *this = bottom();
        return;
      }
    } else {
      if (!addrs_x.is_top() && !addrs_y.is_top() && addrs_x.size() == 1 &&
          addrs_x == addrs_y) {
        // x and y's offsets
        _ptr += (linear_expression_t(x) != linear_expression_t(y));

        if (_ptr.is_bottom()) {
          *this = bottom();
          return;
        }
      }
    }
  }

  //! Forget the surface information (pointer base address and offset) of the
  //! variable v but without modifying the memory content.
  void forget_mem_surface(VariableName o) {
    if (is_top())
      return;

    // remove from the pointer+scalar abstraction
    _ptr -= o;

    // remove from nullity abstraction
    _nullity -= o;

    // remove from the uninitialized variable abstraction
    _uninitialized -= o;
  }

  void forget_mem_surface(variable_set_t vs) {
    for (typename variable_set_t::iterator it = vs.begin(); it != vs.end();
         ++it) {
      forget_mem_surface(it->name());
    }
  }

  //! Forget all memory cells that can be accessible through o
  void forget_mem_contents(VariableName o) {
    if (is_top())
      return;

    forget_reachable_cells(o);
  }

  //! Forget all memory cells that overlap with [o, ..., o + size - 1]
  void forget_mem_contents(VariableName o, Number size) {
    if (is_top())
      return;

    forget_reachable_cells(o, size);
  }

  // convert to another abstract domain
  // uses num_abstract_domain_impl::convert() internally
  template < typename T >
  T convert() {
    return T(_is_top,
             _cells,
             pointer_domain< typename T::scalar_domain_t,
                             VariableName,
                             Number >(_ptr._ptr_map,
                                      num_abstract_domain_impl::convert<
                                          ScalarDomain,
                                          typename T::scalar_domain_t >(
                                          _ptr._inv)),
             _uninitialized,
             _nullity);
  }

  template < typename Any1, typename Any2, typename Any3 >
  friend class summary_domain;
}; // end class summary_domain

} // end namespace ikos

#endif // ANALYZER_SUMMARY_DOMAIN_HPP
