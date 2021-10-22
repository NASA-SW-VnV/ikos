/*******************************************************************************
 *
 * \file
 * \brief PointsToSet class
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
#include <ikos/core/semantic/memory_location.hpp>

namespace ikos {
namespace core {

/// \brief Represents a set of memory locations (i.e, addresses)
template < typename MemoryLocationRef >
class PointsToSet final
    : public core::AbstractDomain< PointsToSet< MemoryLocationRef > > {
public:
  static_assert(IsMemoryLocation< MemoryLocationRef >::value,
                "MemoryLocationRef does not meet the requirements for memory "
                "location types");

private:
  enum Kind { BottomKind, TopKind, SetKind };

private:
  using PatriciaTreeSetT = PatriciaTreeSet< MemoryLocationRef >;

public:
  using Iterator = typename PatriciaTreeSetT::Iterator;

private:
  Kind _kind;
  PatriciaTreeSetT _set;

private:
  struct TopTag {};
  struct BottomTag {};
  struct EmptyTag {};

  /// \brief Create the top points-to set
  explicit PointsToSet(TopTag) : _kind(TopKind) {}

  /// \brief Create the bottom points-to set
  explicit PointsToSet(BottomTag) : _kind(BottomKind) {}

  /// \brief Create the empty points-to set
  explicit PointsToSet(EmptyTag) : _kind(SetKind) {}

public:
  /// \brief Create the top points-to set
  static PointsToSet top() { return PointsToSet(TopTag{}); }

  /// \brief Create the bottom points-to set
  static PointsToSet bottom() { return PointsToSet(BottomTag{}); }

  /// \brief Create the empty points-to set
  static PointsToSet empty() { return PointsToSet(EmptyTag{}); }

  /// \brief Create the points-to set with the given memory locations
  PointsToSet(std::initializer_list< MemoryLocationRef > elements)
      : _kind(SetKind), _set(elements) {}

  /// \brief Copy constructor
  PointsToSet(const PointsToSet&) noexcept = default;

  /// \brief Move constructor
  PointsToSet(PointsToSet&&) noexcept = default;

  /// \brief Copy assignment operator
  PointsToSet& operator=(const PointsToSet&) noexcept = default;

  /// \brief Move assignment operator
  PointsToSet& operator=(PointsToSet&&) noexcept = default;

  /// \brief Destructor
  ~PointsToSet() override = default;

  /// \brief Return the number of memory locations in the points-to set
  std::size_t size() const {
    ikos_assert(this->_kind == SetKind);
    return this->_set.size();
  }

  /// \brief Begin iterator over the memory locations
  Iterator begin() const {
    ikos_assert(this->_kind == SetKind);
    return this->_set.begin();
  }

  /// \brief End iterator over the memory locations
  Iterator end() const {
    ikos_assert(this->_kind == SetKind);
    return this->_set.end();
  }

  void normalize() override {}

  bool is_bottom() const override { return this->_kind == BottomKind; }

  bool is_top() const override { return this->_kind == TopKind; }

  /// \brief Return true if the points-to value is a set (not top or bottom)
  bool is_set() const { return this->_kind == SetKind; }

  /// \brief Return true if the points-to set is the empty set
  bool is_empty() const { return this->_kind == SetKind && this->_set.empty(); }

  void set_to_bottom() override {
    this->_kind = BottomKind;
    this->_set.clear();
  }

  void set_to_top() override {
    this->_kind = TopKind;
    this->_set.clear();
  }

  /// \brief Set the points-to set to the empty set
  void set_to_empty() {
    this->_kind = SetKind;
    this->_set.clear();
  }

  bool leq(const PointsToSet& other) const override {
    switch (this->_kind) {
      case BottomKind:
        return true;
      case TopKind:
        return other._kind == TopKind;
      case SetKind:
        return other._kind == TopKind ||
               (other._kind == SetKind && this->_set.is_subset_of(other._set));
      default:
        ikos_unreachable("unreachable");
    }
  }

  bool equals(const PointsToSet& other) const override {
    if (this->_kind == SetKind) {
      return other._kind == SetKind && this->_set.equals(other._set);
    } else {
      return this->_kind == other._kind;
    }
  }

  void join_with(const PointsToSet& other) override {
    if (this->is_top() || other.is_bottom()) {
      return;
    } else if (other.is_top()) {
      this->set_to_top();
    } else if (this->is_bottom()) {
      this->operator=(other);
    } else {
      this->_set.join_with(other._set);
    }
  }

  void widen_with(const PointsToSet& other) override { this->join_with(other); }

  void meet_with(const PointsToSet& other) override {
    if (this->is_bottom() || other.is_top()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else if (this->is_top()) {
      this->operator=(other);
    } else {
      this->_set.intersect_with(other._set);
    }
  }

  void narrow_with(const PointsToSet& other) override {
    this->meet_with(other);
  }

  /// \brief Perform the set difference
  void difference_with(const PointsToSet& other) {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else if (other.is_top()) {
      this->set_to_empty();
    } else if (this->is_top()) {
      return;
    } else {
      this->_set.difference_with(other._set);
    }
  }

  /// \brief Perform the set difference
  PointsToSet difference(const PointsToSet& other) const {
    PointsToSet tmp(*this);
    tmp.difference_with(other);
    return tmp;
  }

  /// \brief Add a memory location in the points-to set
  void add(MemoryLocationRef m) {
    if (!this->is_set()) {
      return;
    }

    this->_set.insert(m);
  }

  /// \brief Remove a memory location from the points-to set
  void remove(MemoryLocationRef m) {
    if (!this->is_set()) {
      return;
    }

    this->_set.erase(m);
  }

  /// \brief If the points-to set is a singleton {m}, return m, otherwise return
  /// boost::none
  boost::optional< MemoryLocationRef > singleton() const {
    if (this->is_set() && this->_set.size() == 1) {
      return *this->_set.begin();
    } else {
      return boost::none;
    }
  }

  /// \brief Return true if the points-to set contains m
  bool contains(MemoryLocationRef m) const {
    return this->is_top() || (this->is_set() && this->_set.contains(m));
  }

  void dump(std::ostream& o) const override {
    switch (this->_kind) {
      case BottomKind: {
        o << "⊥";
        break;
      }
      case TopKind: {
        o << "T";
        break;
      }
      case SetKind: {
        this->_set.dump(o);
        break;
      }
      default: {
        ikos_unreachable("unreachable");
      }
    }
  }

  static std::string name() { return "points-to set"; }

}; // end class PointsToSet

/// \brief Write a points-to set on a stream
template < typename MemoryLocationRef >
inline std::ostream& operator<<(
    std::ostream& o, const PointsToSet< MemoryLocationRef >& points_to) {
  points_to.dump(o);
  return o;
}

} // end namespace core
} // end namespace ikos
