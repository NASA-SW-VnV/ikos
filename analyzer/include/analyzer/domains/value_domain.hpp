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

#ifndef ANALYZER_VALUE_DOMAIN_HPP
#define ANALYZER_VALUE_DOMAIN_HPP

#include <boost/optional.hpp>

#include <ikos/algorithms/linear_constraints.hpp>
#include <ikos/common/bignums.hpp>
#include <ikos/common/types.hpp>
#include <ikos/domains/discrete_domains.hpp>
#include <ikos/domains/intervals.hpp>
#include <ikos/domains/nullity.hpp>
#include <ikos/domains/separate_domains.hpp>
#include <ikos/domains/uninitialized.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/ikos-wrapper/domains_traits.hpp>

namespace ikos {

template < typename Any1, typename Any2, typename Any3 >
class memory_domain;

template < typename Any1, typename Any2, typename Any3 >
class summary_domain;

namespace num_abstract_domain_impl {

// (Partial) specialized version for value domain
template < typename AbsNumDomain >
inline z_interval to_interval(
    memory_domain< AbsNumDomain, analyzer::varname_t, z_number > inv,
    analyzer::varname_t v) {
  return inv.to_interval(v);
}

// (Partial) specialized version for value domain
template < typename AbsNumDomain >
inline void from_interval(
    memory_domain< AbsNumDomain, analyzer::varname_t, z_number >& inv,
    analyzer::varname_t v,
    z_interval intv) {
  inv.from_interval(v, intv);
}

} // end namespace num_abstract_domain_impl

// The pointer domain can express integer scalar (modelled directly in
// ScalarDomain) and pointer variables. For each pointer p, it keeps
// track of its address as well as its offset. The offset of p is also
// modelled directly by the numerical abstraction ScalarDomain. The
// address of p is modelled by keeping track of all possible memory
// objects (e.g., &'s and malloc's) to which p may point to.
//
// Notes:
//
// - This domain can only model pointers to integer or floating point
//   variables and it does not model pointers with multiple levels of
//   indirection. However, unlike traditional pointer analysis this
//   domain is flow-sensitive.
//
// - It represents the points-to set (all possible addresses of a
//   pointer) using the discrete domain. Thus, we cannot distinguish
//   between bottom and the empty set. The empty set is useful since
//   it can tell us whether a variable is definitely null. One simple
//   solution is to have a special memory object denoting null (e.g.,
//   $null). For simplicity, we do not do it because we combine later
//   this domain with the nullity domain so that can still keep track
//   of that information.
template < typename ScalarDomain, typename VariableName, typename Number >
class pointer_domain : public writeable,
                       public numerical_domain< Number, VariableName >,
                       public bitwise_operators< Number, VariableName >,
                       public division_operators< Number, VariableName > {
  template < typename Any1, typename Any2, typename Any3 >
  friend class memory_domain;
  template < typename Any1, typename Any2, typename Any3 >
  friend class summary_domain;

public:
  typedef pointer_domain< ScalarDomain, VariableName, Number > pointer_domain_t;
  typedef discrete_domain< VariableName > points_to_set_t;

  typedef typename ScalarDomain::variable_t variable_t;
  typedef typename ScalarDomain::linear_constraint_t linear_constraint_t;
  typedef typename ScalarDomain::linear_constraint_system_t
      linear_constraint_system_t;
  typedef typename ScalarDomain::linear_expression_t linear_expression_t;

  typedef interval< Number > interval_t;
  typedef bound< Number > bound_t;

private:
  typedef separate_domain< VariableName, points_to_set_t > points_to_map_t;

  points_to_map_t _ptr_map; // map pointer variables to set of objects
  ScalarDomain _inv;        // scalar and pointer offset variables

private:
  pointer_domain(bool is_bot)
      : writeable(),
        _ptr_map(is_bot ? points_to_map_t::bottom() : points_to_map_t::top()),
        _inv(is_bot ? ScalarDomain::bottom() : ScalarDomain::top()) {}

  pointer_domain(const points_to_map_t& ptr_map, const ScalarDomain& inv)
      : writeable(), _ptr_map(ptr_map), _inv(inv) {}

  // set the set of addresses of x
  void set_addrs_set(VariableName x, points_to_set_t addrs) {
    if (is_bottom())
      return;

    _ptr_map.set(x, addrs);
  }

  // refine the set of addresses of x
  void meet_addrs_set(VariableName x, points_to_set_t addrs) {
    if (is_bottom())
      return;

    _ptr_map.set(x, _ptr_map[x] & addrs);
  }

public:
  static pointer_domain_t top() { return pointer_domain(false); }

  static pointer_domain_t bottom() { return pointer_domain(true); }

public:
  pointer_domain()
      : writeable(),
        _ptr_map(points_to_map_t::top()),
        _inv(ScalarDomain::top()) {}

  pointer_domain(const pointer_domain_t& o)
      : writeable(), _ptr_map(o._ptr_map), _inv(o._inv) {}

  pointer_domain_t& operator=(const pointer_domain_t& o) {
    this->_ptr_map = o._ptr_map;
    this->_inv = o._inv;
    return *this;
  }

  bool is_bottom() {
    if (_ptr_map.is_bottom() || _inv.is_bottom()) {
      *this = bottom();
      return true;
    } else {
      return false;
    }
  }

  bool is_top() { return _ptr_map.is_top() && _inv.is_top(); }

  bool operator<=(pointer_domain_t o) {
    if (is_bottom())
      return true;
    else if (o.is_bottom())
      return false;
    else
      return _ptr_map <= o._ptr_map && _inv <= o._inv;
  }

  pointer_domain_t operator|(pointer_domain_t o) {
    if (is_bottom())
      return o;
    else if (o.is_bottom())
      return *this;
    else
      return pointer_domain_t(_ptr_map | o._ptr_map, _inv | o._inv);
  }

  pointer_domain_t operator||(pointer_domain_t o) {
    return pointer_domain_t(_ptr_map | o._ptr_map, _inv || o._inv);
  }

  pointer_domain_t operator&(pointer_domain_t o) {
    if (is_top())
      return o;
    else if (o.is_top())
      return *this;
    else
      return pointer_domain_t(_ptr_map & o._ptr_map, _inv & o._inv);
  }

  pointer_domain_t operator&&(pointer_domain_t o) {
    return pointer_domain_t(_ptr_map & o._ptr_map, _inv && o._inv);
  }

  void operator-=(VariableName v) {
    _ptr_map -= v;
    _inv -= v;
  }

  void operator-=(std::vector< VariableName > vs) {
    for (typename std::vector< VariableName >::iterator it = vs.begin();
         it != vs.end();
         ++it) {
      _ptr_map -= *it;
    }
    _inv -= vs;
  }

  void assign(VariableName x, linear_expression_t e) { _inv.assign(x, e); }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    _inv.apply(op, x, y, z);
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    _inv.apply(op, x, y, k);
  }

  points_to_set_t get_addrs_set(VariableName x) {
    if (is_bottom())
      return points_to_set_t::bottom();

    return _ptr_map[x];
  }

  void make_object(VariableName x) {
    if (is_bottom())
      return;

    assert(_ptr_map[x].is_top() || _ptr_map[x] == x); // sanity check
    _ptr_map.set(x, x);
  }

  void assign_ptr(VariableName x, VariableName y) {
    if (is_bottom())
      return;

    _ptr_map.set(x, _ptr_map[y]);
  }

  void operator+=(linear_constraint_t cst) { this->_inv += cst; }

  void operator+=(linear_constraint_system_t cst) { this->_inv += cst; }

  interval_t operator[](VariableName x) {
    if (is_top())
      return interval_t::top();
    if (is_bottom())
      return interval_t::bottom();

    return num_abstract_domain_impl::to_interval(_inv, x);
  }

  interval_t operator[](linear_expression_t e) {
    if (is_top())
      return interval_t::top();
    if (is_bottom())
      return interval_t::bottom();

    interval_t r(e.constant());
    for (typename linear_expression_t::iterator it = e.begin(); it != e.end();
         ++it) {
      interval_t c(it->first);
      r += c * num_abstract_domain_impl::to_interval(_inv, it->second.name());
    }
    return r;
  }

  void set(VariableName x, interval_t intv) {
    if (is_bottom())
      return;

    num_abstract_domain_impl::from_interval(_inv, x, intv);
  }

  void write(std::ostream& o) {
    if (is_bottom())
      o << "_|_";
    else if (is_top())
      o << "{}";
    else
      o << "(" << _ptr_map << "," << _inv << ")";
  }

  // bitwise_operators_api and division_operators_api

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             unsigned width) {
    _inv.apply(op, x, y, width);
  }

  void apply(conv_operation_t op, VariableName x, Number k, unsigned width) {
    _inv.apply(op, x, k, width);
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    _inv.apply(op, x, y, z);
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    _inv.apply(op, x, y, k);
  }

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    _inv.apply(op, x, y, z);
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    _inv.apply(op, x, y, k);
  }
};

// Memory abstraction consisting of cells and a pointer abstraction
// augmented with nullity and uninitialized variable information.
//
// This domain abstracts memory into a set of synthetic cells with
// integer and also pointer type following Mine's paper. If a cell is
// of type integer this domain can model its value and whether it is
// initialized or not. Otherwise, if the cell is of pointer type this
// domain keeps track only whether it is null or not. Therefore, this
// domain does not model the addresses to which a cell of pointer type
// may point to. This the reason why we allow reduction with an
// external pointer analysis that can provide that information.
template < typename ScalarDomain, typename VariableName, typename Number >
class memory_domain : public writeable,
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

  typedef pointer_domain< ScalarDomain, VariableName, Number > pointer_domain_t;
  typedef typename pointer_domain_t::points_to_set_t points_to_set_t;
  typedef uninitialized_domain< VariableName > uninitialized_domain_t;
  typedef nullity_domain< VariableName > nullity_domain_t;
  typedef memory_domain< ScalarDomain, VariableName, Number > memory_domain_t;

private:
  // A cell is a triple <V,i,size> modelling all bytes starting at offset i
  // in variable V up to (i + size -1).
  //
  // Note that both the offset and the size are numbers.
  class Cell : public writeable {
  private:
    VariableName _base;
    Number _offset;
    Number _size;

    // represents the cell in the pointer+scalar domain
    VariableName _scalar_var;

  private:
    inline std::string name() const {
      std::ostringstream buf;
      buf << "C{" << _base.name() << "," << _offset << "," << _size << "}";
      return buf.str();
    }

  public:
    Cell(VariableName base, Number offset, Number size)
        : _base(base), _offset(offset), _size(size), _scalar_var(base) {
      assert(_offset >= 0 && _size >= 1);
      _scalar_var = _base.getVarFactory()[name()];
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
      if (size != _size)
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

    CellSet(const CellSet& other)
        : writeable(), _is_top(other._is_top), _set(other._set) {}

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
        throw ikos_error("Size for discrete domain TOP is undefined");
      } else {
        return this->_set.size();
      }
    }

    iterator begin() {
      if (this->_is_top) {
        throw ikos_error("Iterator for discrete domain TOP is undefined");
      } else {
        return this->_set.begin();
      }
    }

    iterator end() {
      if (this->_is_top) {
        throw ikos_error("Iterator for discrete domain TOP is undefined");
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
  }; // class CellSet

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
  //
  // TODO: it should be a map from pointer offsets to set of
  // overlapping cells to make operations, such as killOverlapCells
  // and forget faster.
  var_to_cell_set_t _cells; // memory contents
  pointer_domain_t _ptr;    // pointer+scalar domain (i.e., surface domain)
  uninitialized_domain_t _uninitialized;
  nullity_domain_t _nullity;

  memory_domain(bool is_bot)
      : writeable(),
        _cells(is_bot ? var_to_cell_set_t::bottom() : var_to_cell_set_t::top()),
        _ptr(is_bot ? pointer_domain_t::bottom() : pointer_domain_t::top()),
        _uninitialized(is_bot ? uninitialized_domain_t::bottom()
                              : uninitialized_domain_t::top()),
        _nullity(is_bot ? nullity_domain_t::bottom()
                        : nullity_domain_t::top()) {}

  memory_domain(const var_to_cell_set_t& cells,
                const pointer_domain_t& ptr_inv,
                const uninitialized_domain_t& uinitialized,
                const nullity_domain_t& nullity)
      : writeable(),
        _cells(cells),
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
    // remove the cell from memory abstraction
    cell_set_t cells = _cells[c.base()];
    cells -= c;

    if (cells.is_bottom()) {
      _cells -= c.base();
    } else {
      _cells.set(c.base(), cells);
    }

    // remove the cell from the pointer+scalar abstraction
    _ptr -= c.scalar_var();

    // remove the cell from the nullity abstraction
    _nullity -= c.scalar_var();

    // remove the cell from the uninitialized variable abstraction
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
    interval_t offset_intv = _ptr[offset];
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

  std::vector< VariableName > get_variables(linear_expression_t e) const {
    std::vector< VariableName > vs;
    for (typename linear_expression_t::iterator it = e.begin(); it != e.end();
         ++it) {
      vs.push_back(it->second.name());
    }
    return vs;
  }

  // Weak update of x with e.
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

  // Strong update of x with e
  void strong_update(VariableName x, linear_expression_t e, bool is_pointer) {
    _ptr.assign(x, e);

    std::vector< VariableName > rhs = get_variables(e);
    if (is_pointer) {
      _nullity.assign(x, rhs);
    } else {
      _uninitialized.assign(x, rhs);
    }
  }

  //! Forget all memory cells that can be accessible through pointer o
  void forget_reachable_cells(VariableName o) {
    points_to_set_t addrs_set = _ptr.get_addrs_set(o); // o's base address

    if (addrs_set.is_top())
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

    if (addrs_set.is_top())
      return;

    for (typename points_to_set_t::iterator it = addrs_set.begin();
         it != addrs_set.end();
         ++it) {
      forget_cells(*it, o, size);
    }
  }

  //! Forget all memory cells of base address `base`
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

  //! Forget all memory cells of base address `base` that overlap
  //  with [offset, ..., offset + size - 1]
  void forget_cells(VariableName base, VariableName offset, interval_t size) {
    interval_t offset_intv =
        _ptr[offset] & interval_t(0, bound_t::plus_infinity());
    size = size & interval_t(1, bound_t::plus_infinity());
    interval_t range = offset_intv + interval_t(0, size.ub() - 1);
    forget_cells(base, range);
  }

  //! Forget all memory cells of base address `base` that overlap with a given
  //  range
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
  static memory_domain_t top() { return memory_domain(false); }

  static memory_domain_t bottom() { return memory_domain(true); }

  memory_domain()
      : writeable(),
        _cells(var_to_cell_set_t::top()),
        _ptr(pointer_domain_t::top()),
        _uninitialized(uninitialized_domain_t::top()),
        _nullity(nullity_domain_t::top()) {}

  memory_domain(const memory_domain_t& o)
      : writeable(),
        _cells(o._cells),
        _ptr(o._ptr),
        _uninitialized(o._uninitialized),
        _nullity(o._nullity) {}

  memory_domain_t& operator=(const memory_domain_t& o) {
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

  bool operator<=(memory_domain_t o) {
    if (is_bottom())
      return true;
    else if (o.is_bottom())
      return false;
    else
      return _cells <= o._cells && _ptr <= o._ptr &&
             _uninitialized <= o._uninitialized && _nullity <= o._nullity;
  }

  memory_domain_t operator|(memory_domain_t o) {
    if (is_bottom())
      return o;
    else if (o.is_bottom())
      return *this;
    else
      return memory_domain_t(_cells | o._cells,
                             _ptr | o._ptr,
                             _uninitialized | o._uninitialized,
                             _nullity | o._nullity);
  }

  memory_domain_t operator||(memory_domain_t o) {
    if (is_bottom())
      return o;
    else if (o.is_bottom())
      return *this;
    else
      return memory_domain_t(_cells | o._cells,
                             _ptr || o._ptr,
                             _uninitialized || o._uninitialized,
                             _nullity || o._nullity);
  }

  memory_domain_t operator&(memory_domain_t o) {
    if (is_top())
      return o;
    else if (o.is_top())
      return *this;
    else
      return memory_domain_t(_cells & o._cells,
                             _ptr & o._ptr,
                             _uninitialized & o._uninitialized,
                             _nullity & o._nullity);
  }

  memory_domain_t operator&&(memory_domain_t o) {
    if (is_top())
      return o;
    else if (o.is_top())
      return *this;
    else
      return memory_domain_t(_cells & o._cells,
                             _ptr && o._ptr,
                             _uninitialized && o._uninitialized,
                             _nullity && o._nullity);
  }

  //! Forget both the surface of v (base addresses and offsets) and
  //! all the memory contents that may be accessible through v.
  void operator-=(VariableName v) {
    forget_mem_contents(v); // has to be before forget_mem_surface
    forget_mem_surface(v);
  }

  void assign(VariableName x, linear_expression_t e) {
    boost::optional< variable_t > y = e.get_variable();
    if (y && ((*y).name() == x))
      return; // do nothing

    // Only affect the "surface" semantics so no need to kill cells.
    _ptr.assign(x, e);

    // Do not propagate:
    // - uninitialized variables
    // - nullity
  }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    // Only affect the "surface" semantics so no need to kill cells
    _ptr.apply(op, x, y, z);

    // Do not propagate:
    // - nullity
    // - uninitialized variables
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    // Only affect the "surface" semantics so no need to kill cells
    _ptr.apply(op, x, y, k);

    // Do not propagate:
    // - nullity
    // - uninitialized variables
  }

  // Only pointer+numerical abstraction is propagated
  void operator+=(linear_constraint_t cst) {
    _ptr += cst;

    // Do not propagate:
    // - uninitialized variables
    // - nullity
  }

  // Only pointer+numerical abstraction is propagated
  void operator+=(linear_constraint_system_t cst) { _ptr += cst; }

  // Get/Set methods for accessing to the numerical abstraction
  interval_t to_interval(VariableName x) {
    if (is_top())
      return interval_t::top();
    if (is_bottom())
      return interval_t::bottom();

    return _ptr[x];
  }

  void from_interval(VariableName x, interval_t intv) {
    // Only affect the "surface" semantics so no need to kill cells
    _ptr.set(x, intv);
  }

  /*
   * bitwise_operators_api and division_operators_api
   */

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             unsigned width) {
    // Only affect the "surface" semantics so no need to kill cells
    _ptr.apply(op, x, y, width);

    // Do not propagate:
    // - nullity
    // - uninitialized variables
  }

  void apply(conv_operation_t op, VariableName x, Number k, unsigned width) {
    // Only affect the "surface" semantics so no need to kill cells
    _ptr.apply(op, x, k, width);

    // Do not propagate:
    // - nullity
    // - uninitialized variables
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    // Only affect the "surface" semantics so no need to kill cells
    _ptr.apply(op, x, y, z);

    // Do not propagate:
    // - nullity
    // - uninitialized variables
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    // Only affect the "surface" semantics so no need to kill cells
    _ptr.apply(op, x, y, k);

    // Do not propagate:
    // - nullity
    // - uninitialized variables
  }

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    // Only affect the "surface" semantics so no need to kill cells
    _ptr.apply(op, x, y, z);

    // Do not propagate:
    // - nullity
    // - uninitialized variables
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    // Only affect the "surface" semantics so no need to kill cells
    _ptr.apply(op, x, y, k);

    // Do not propagate:
    // - nullity
    // - uninitialized variables
  }

  void write(std::ostream& o) {
    if (is_bottom())
      o << "_|_";
    else if (is_top())
      o << "{}";
    else {
      o << "(" << _cells << "," << _ptr << "," << _uninitialized << ","
        << _nullity << ")";
    }
  }

  /*
   * Non-standard operations in numerical abstract domains
   */

  //! create a new memory object
  void make_object(VariableName x) { _ptr.make_object(x); }

  //! x points to y
  void assign_ptr(VariableName x, VariableName y) { _ptr.assign_ptr(x, y); }

  //! return true if the set of possible addresses of x is unknown
  bool is_unknown_addr(VariableName x) {
    return _ptr.get_addrs_set(x).is_top();
  }

  //! return the set of possible addresses of x
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

  //! Reduction step: refine the set of possible addresses
  void refine_addrs(VariableName v, points_to_set_t addrs) {
    if (is_bottom())
      return;

    // addrs is just a set of addresses to which v can
    // point-to. However, v can be still null. To be safe, we refine
    // the addresses of v only if we know that v cannot be null.
    if (!is_null(v)) {
      _ptr.meet_addrs_set(v, addrs);
    }
  }

  //! Reduction step: refine the set of possible addresses and offset
  void refine_addrs_and_offset(VariableName v_addr,
                               points_to_set_t addrs,
                               VariableName v_offset,
                               interval_t offset) {
    if (is_bottom())
      return;

    // refine addresses
    refine_addrs(v_addr, addrs);

    // refine offset
    memory_domain_t other(*this);
    num_abstract_domain_impl::from_interval(other, v_offset, offset);
    *this = *this & other;
  }

  //! v is definitely an uninitialized variable.
  void make_uninitialized(VariableName v) {
    _uninitialized.set(v, uninitialized_value::uninitialized());
  }

  //! return if v is uninitialized
  bool is_uninitialized(VariableName v) {
    return _uninitialized[v].is_uninitialized();
  }

  //! v is definitely an initialized variable.
  void make_initialized(VariableName v) {
    _uninitialized.set(v, uninitialized_value::initialized());
  }

  //! return if v is initialized
  bool is_initialized(VariableName v) {
    return _uninitialized[v].is_initialized();
  }

  //! v is definitely null
  void make_null(VariableName v) { _nullity.set(v, nullity_value::null()); }

  //! return if v is null
  bool is_null(VariableName v) { return _nullity[v].is_null(); }

  //! v is definitely non null
  void make_non_null(VariableName v) {
    _nullity.set(v, nullity_value::non_null());
  }

  //! return if v is not null
  bool is_non_null(VariableName v) { return _nullity[v].is_non_null(); }

  //! propagates y's nullity value to x
  void assign_nullity(VariableName x, VariableName y) { _nullity.assign(x, y); }

  //! propagate ys's uninitialized values to x
  void assign_uninitialized(VariableName x, std::vector< VariableName > ys) {
    _uninitialized.assign(x, ys);
  }

  //! memory write: store e at memory object associated with offset in
  //! bytes {offset, ..., offset + size - 1}.
  void mem_write(VariableName offset,
                 Number size,
                 linear_expression_t e,
                 bool is_pointer) {
    if (is_bottom())
      return;

    if (is_null(offset)) { // null dereference!
      *this = bottom();
      return;
    }

    // Get memory locations associated with offset
    points_to_set_t addrs = _ptr.get_addrs_set(offset);
    if (addrs.is_top()) {
#if 0
      std::cout << "Ignored memory write to " << offset 
           << " with value " << e << std::endl;
      std::cout << "Pointer addresses= " << _ptr << std::endl;
#endif
      return;
    }

    // Get the offset interval
    interval_t offset_intv = _ptr[offset];
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
          strong_update(c.scalar_var(), e, is_pointer);
        } else {
          weak_update(c.scalar_var(), e, is_pointer);
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
            write_realize_range_cells(*it, offset, size);
        for (typename std::vector< Cell >::iterator it2 = cells.begin();
             it2 != cells.end();
             ++it2) {
          weak_update(it2->scalar_var(), e, is_pointer);
        }
      }
    }
  }

  //! memory read: read from memory object associated with offset from
  //! bytes {offset, ..., offset + size - 1}
  void mem_read(VariableName lhs,
                VariableName offset,
                Number size,
                bool is_pointer) {
    if (is_bottom())
      return;

    if (is_null(offset)) { // null dereference!
      *this = bottom();
      return;
    }

    // Get memory locations associated with offset
    points_to_set_t addrs = _ptr.get_addrs_set(offset);
    if (addrs.is_top()) {
#if 0
      std::cout << "Ignored memory read from offset " << offset << std::endl;
#endif
      return;
    }

    // Get the offset interval
    interval_t offset_intv = _ptr[offset];
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
          strong_update(lhs, variable_t(c.scalar_var()), is_pointer);
          first = false;
        } else {
          weak_update(lhs, variable_t(c.scalar_var()), is_pointer);
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
      _ptr -= lhs;
      _nullity -= lhs;
      _uninitialized -= lhs;
    }
  }

  //! copy bytes {src, ..., src + size - 1} to {dest, ..., dest + size - 1}
  void mem_copy(VariableName dest, VariableName src, linear_expression_t size) {
    if (is_bottom())
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
    interval_t size_intv = _ptr[size] & interval_t(1, bound_t::plus_infinity());

    if (dest_intv.is_bottom() || src_intv.is_bottom()) { // buffer overflow
      *this = bottom();
      return;
    }

    if (size_intv.is_bottom())
      return;

    // We have to be sound, so we remove all reachable cells
    forget_reachable_cells(dest, size_intv);

    if (dest_addrs.size() == 1 && dest_intv.singleton() &&
        !src_addrs.is_top() && src_intv.singleton()) {
      // in that only case, we can be more precise
      VariableName dest_base = *dest_addrs.begin();
      Number dest_offset = *dest_intv.singleton();
      Number src_offset = *src_intv.singleton();
      Number size_lb = *size_intv.lb().number();

      memory_domain_t prev = *this;
      bool first = true;

      for (typename points_to_set_t::iterator it = src_addrs.begin();
           it != src_addrs.end();
           ++it) {
        memory_domain_t inv = prev;
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
              inv.assign_uninitialized(dest_c.scalar_var(),
                                       std::vector<
                                           VariableName >(1, c.scalar_var()));
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

  //! set bytes {src, ..., src + size - 1} to value
  void mem_set(VariableName dest,
               linear_expression_t value,
               linear_expression_t size) {
    if (is_bottom())
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

    if (dest_intv.is_bottom()) { // buffer overflow
      *this = bottom();
      return;
    }

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
                strong_update(c.scalar_var(), 0, false);
              } else {
                weak_update(c.scalar_var(), 0, false);
              }
            } else if (c.overlaps(unsafe_range)) {
              remove_cell(c);
            }
          }
        }
      }
    } else {
      // To be sound, we remove all reachable cells
      forget_reachable_cells(dest, size_intv);
    }
  }

  //! assert whether x == null or x != null
  void cmp_mem_addr_null(bool is_equality, VariableName x) {
    _nullity.assertion(is_equality, x, nullity_value::null());

    if (_nullity.is_bottom()) {
      *this = bottom();
    }
  }

  //! assert whether x == y or x != y
  void cmp_mem_addr(bool is_equality, VariableName x, VariableName y) {
    _nullity.assertion(is_equality, x, y);
    // TODO: if the points to set of x and y are singleton we could
    // also enforce them to be equal or non-equal.

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
    // remove from the pointer+scalar abstraction
    _ptr -= o;

    // remove from nullity abstraction
    _nullity -= o;

    // remove from the uninitialized variable abstraction
    _uninitialized -= o;
  }

  //! Forget all memory cells that can be accessible through o
  void forget_mem_contents(VariableName o) { forget_reachable_cells(o); }

  //! Forget all memory cells that overlap with [o, ..., o + size - 1]
  void forget_mem_contents(VariableName o, Number size) {
    forget_reachable_cells(o, size);
  }
};

} // end namespace

#endif // ANALYZER_VALUE_DOMAIN_HPP
