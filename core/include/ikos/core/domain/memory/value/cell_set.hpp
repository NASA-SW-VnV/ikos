/*******************************************************************************
 *
 * \file
 * \brief Abstract domain for a set of cells, based on Patricia trees
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

#include <boost/optional.hpp>

#include <ikos/core/adt/patricia_tree/set.hpp>
#include <ikos/core/domain/abstract_domain.hpp>

namespace ikos {
namespace core {
namespace memory {

/// \brief Cell set abstract domain
///
/// It is similar to the DiscreteDomain but has a different semantic.
///
/// The empty cell set represents top.
/// The bottom value is represented as top.
///
/// Note that this is not a lattice.
template < typename VariableRef >
class CellSet final : public core::AbstractDomain< CellSet< VariableRef > > {
private:
  using PatriciaTreeSetT = PatriciaTreeSet< VariableRef >;

public:
  using Iterator = typename PatriciaTreeSetT::Iterator;

private:
  PatriciaTreeSetT _set;

private:
  struct EmptyTag {};

  /// \brief Create the empty cell set
  explicit CellSet(EmptyTag) {}

public:
  /// \brief Create the top cell set
  static CellSet top() { return CellSet(EmptyTag{}); }

  /// \brief Create the bottom cell set
  static CellSet bottom() { return CellSet(EmptyTag{}); }

  /// \brief Create the empty cell set
  static CellSet empty() { return CellSet(EmptyTag{}); }

  /// \brief Create the cell set with the given cells
  CellSet(std::initializer_list< VariableRef > cells) : _set(cells) {}

  /// \brief Copy constructor
  CellSet(const CellSet&) noexcept = default;

  /// \brief Move constructor
  CellSet(CellSet&&) noexcept = default;

  /// \brief Copy assignment operator
  CellSet& operator=(const CellSet&) noexcept = default;

  /// \brief Move assignment operator
  CellSet& operator=(CellSet&&) noexcept = default;

  /// \brief Destructor
  ~CellSet() override = default;

  /// \brief Return the number of cells
  std::size_t size() const { return this->_set.size(); }

  /// \brief Begin iterator over the cells
  Iterator begin() const { return this->_set.begin(); }

  /// \brief End iterator over the cells
  Iterator end() const { return this->_set.end(); }

  void normalize() override {}

  bool is_bottom() const override { return false; }

  bool is_top() const override { return this->_set.empty(); }

  /// \brief Return true if the set is empty
  bool is_empty() const { return this->_set.empty(); }

  void set_to_bottom() override { this->_set.clear(); }

  void set_to_top() override { this->_set.clear(); }

  bool leq(const CellSet& other) const override {
    if (other.is_top()) {
      return true;
    } else if (this->is_top()) {
      return false;
    } else {
      return this->_set.is_subset_of(other._set);
    }
  }

  bool equals(const CellSet& other) const override {
    if (this->is_top()) {
      return other.is_top();
    } else if (other.is_top()) {
      return false;
    } else {
      return this->_set.equals(other._set);
    }
  }

  void join_with(const CellSet& other) override {
    // only keep cells present on both sides
    this->_set.intersect_with(other._set);
  }

  void widen_with(const CellSet& other) override { this->join_with(other); }

  void meet_with(const CellSet& other) override {
    // keep all the cells
    this->_set.join_with(other._set);
  }

  void narrow_with(const CellSet& other) override { this->meet_with(other); }

  /// \brief Perform the set difference
  void difference_with(const CellSet& other) {
    this->_set.difference_with(other._set);
  }

  /// \brief Perform the set difference
  CellSet difference(const CellSet& other) const {
    CellSet tmp(*this);
    tmp.difference_with(other);
    return tmp;
  }

  /// \brief Add a cell in the set
  void add(VariableRef cell) { this->_set.insert(cell); }

  /// \brief Remove a cell from the set
  void remove(VariableRef cell) { this->_set.erase(cell); }

  /// \brief If the cell set is a singleton {c}, return c, otherwise return
  /// boost::none
  boost::optional< VariableRef > singleton() const {
    if (this->_set.size() == 1) {
      return *this->_set.begin();
    } else {
      return boost::none;
    }
  }

  /// \brief Return true if the cell set contains the given cell
  bool contains(VariableRef cell) const {
    return this->is_top() || this->_set.contains(cell);
  }

  void dump(std::ostream& o) const override {
    if (this->is_top()) {
      o << "⊤";
    } else {
      this->_set.dump(o);
    }
  }

  static std::string name() { return "cell set domain"; }

}; // end class CellSet

} // end namespace memory
} // end namespace core
} // end namespace ikos
