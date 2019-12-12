/*******************************************************************************
 *
 * \file
 * \brief Discrete domain based on Patricia trees
 *
 * Author: Arnaud J. Venet
 *
 * Contributors: Maxime Arthaud
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

#include <boost/optional.hpp>

#include <ikos/core/adt/patricia_tree/set.hpp>
#include <ikos/core/domain/abstract_domain.hpp>

namespace ikos {
namespace core {

/// \brief Discrete abstract domain
///
/// The implementation is based on patricia trees.
template < typename Element >
class DiscreteDomain final
    : public AbstractDomain< DiscreteDomain< Element > > {
private:
  using PatriciaTreeSetT = PatriciaTreeSet< Element >;

public:
  using Iterator = typename PatriciaTreeSetT::Iterator;

private:
  PatriciaTreeSetT _set;
  bool _top;

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top discrete domain
  explicit DiscreteDomain(TopTag) : _top(true) {}

  /// \brief Create the bottom discrete domain
  explicit DiscreteDomain(BottomTag) : _top(false) {}

public:
  /// \brief Create the top discrete domain
  static DiscreteDomain top() { return DiscreteDomain(TopTag{}); }

  /// \brief Create the bottom discrete domain
  static DiscreteDomain bottom() { return DiscreteDomain(BottomTag{}); }

  /// \brief Create the discrete domain with the given elements
  DiscreteDomain(std::initializer_list< Element > elements)
      : _set(elements), _top(false) {}

  /// \brief Copy constructor
  DiscreteDomain(const DiscreteDomain&) noexcept = default;

  /// \brief Move constructor
  DiscreteDomain(DiscreteDomain&&) noexcept = default;

  /// \brief Copy assignment operator
  DiscreteDomain& operator=(const DiscreteDomain&) noexcept = default;

  /// \brief Move assignment operator
  DiscreteDomain& operator=(DiscreteDomain&&) noexcept = default;

  /// \brief Destructor
  ~DiscreteDomain() override = default;

  /// \brief Return the number of elements in the discrete domain
  std::size_t size() const {
    ikos_assert(!this->is_top());
    return this->_set.size();
  }

  /// \brief Begin iterator over the elements
  Iterator begin() const {
    ikos_assert(!this->is_top());
    return this->_set.begin();
  }

  /// \brief End iterator over the elements
  Iterator end() const {
    ikos_assert(!this->is_top());
    return this->_set.end();
  }

  void normalize() override {}

  bool is_bottom() const override { return !this->_top && this->_set.empty(); }

  bool is_top() const override { return this->_top; }

  void set_to_bottom() override {
    this->_top = false;
    this->_set.clear();
  }

  void set_to_top() override {
    this->_top = true;
    this->_set.clear();
  }

  bool leq(const DiscreteDomain& other) const override {
    if (other.is_top()) {
      return true;
    } else if (this->is_top()) {
      return false;
    } else {
      return this->_set.is_subset_of(other._set);
    }
  }

  bool equals(const DiscreteDomain& other) const override {
    if (this->is_top()) {
      return other.is_top();
    } else if (other.is_top()) {
      return false;
    } else {
      return this->_set.equals(other._set);
    }
  }

  void join_with(const DiscreteDomain& other) override {
    if (this->is_top()) {
      return;
    } else if (other.is_top()) {
      this->set_to_top();
    } else {
      this->_set.join_with(other._set);
    }
  }

  void widen_with(const DiscreteDomain& other) override {
    this->join_with(other);
  }

  void meet_with(const DiscreteDomain& other) override {
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

  void narrow_with(const DiscreteDomain& other) override {
    this->meet_with(other);
  }

  /// \brief Perform the set difference
  void difference_with(const DiscreteDomain& other) {
    if (other.is_top()) {
      this->set_to_bottom();
    } else if (this->is_top()) {
      return;
    } else {
      this->_set.difference_with(other._set);
    }
  }

  /// \brief Perform the set difference
  DiscreteDomain difference(const DiscreteDomain& other) const {
    DiscreteDomain tmp(*this);
    tmp.difference_with(other);
    return tmp;
  }

  /// \brief Add an element in the discrete domain
  void add(const Element& e) {
    if (this->is_top()) {
      return;
    }

    this->_set.insert(e);
  }

  /// \brief Remove an element from the discrete domain
  void remove(const Element& e) {
    if (this->is_top()) {
      return;
    }

    this->_set.erase(e);
  }

  /// \brief If the discrete domain is a singleton {e}, return e, otherwise
  /// return boost::none
  boost::optional< Element > singleton() const {
    if (!this->is_top() && this->_set.size() == 1) {
      return *this->_set.begin();
    } else {
      return boost::none;
    }
  }

  /// \brief Return true if the discrete domain contains e
  bool contains(const Element& e) const {
    return this->is_top() || this->_set.contains(e);
  }

  void dump(std::ostream& o) const override {
    if (this->is_top()) {
      o << "⊤";
    } else if (this->is_bottom()) {
      o << "⊥";
    } else {
      this->_set.dump(o);
    }
  }

  static std::string name() { return "discrete domain"; }

}; // end class DiscreteDomain

} // end namespace core
} // end namespace ikos
