/*******************************************************************************
 *
 * \file
 * \brief Implementation of a mergeable map based on patricia trees
 *
 * Author: Jorge A. Navas
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

#include <ikos/core/adt/patricia_tree/map.hpp>
#include <ikos/core/common/type.hpp>
#include <ikos/core/semantic/dumpable.hpp>

namespace ikos {
namespace core {

/// \brief A mergeable map implementation based on patricia trees
template < typename Key, typename Value >
class mergeable_map : public writeable {
private:
  using PatriciaTreeMap = PatriciaTreeMap< Key, Value >;
  using mergeable_map_t = mergeable_map< Key, Value >;

public:
  using iterator = typename PatriciaTreeMap::Iterator;

private:
  PatriciaTreeMap _tree;

private:
private:
  mergeable_map(PatriciaTreeMap t) : _tree(std::move(t)) {}

public:
  mergeable_map() = default;

  mergeable_map(const mergeable_map_t&) = default;

  mergeable_map_t& operator=(const mergeable_map_t&) = default;

  iterator begin() const { return this->_tree.begin(); }

  iterator end() const { return this->_tree.end(); }

  std::size_t size() const { return this->_tree.size(); }

  mergeable_map_t operator|(mergeable_map_t e) const {
    return mergeable_map_t(this->_tree.join(e._tree, [](Value x, Value y) {
      if (x == y)
        return boost::optional< Value >(x);
      else {
        throw LogicError(
            "mergeable map: merging a key with two different values");
      }
    }));
  }

  void set(const Key& k, const Value& v) { this->_tree.insert_or_assign(k, v); }

  mergeable_map_t& operator-=(const Key& k) {
    this->_tree.erase(k);
    return *this;
  }

  boost::optional< const Value& > operator[](const Key& k) const {
    return this->_tree.at(k);
  }

  void clear() { this->_tree.clear(); }

  void write(std::ostream& o) { this->_tree.dump(o); }

  void dump(std::ostream& o) const { this->_tree.dump(o); }

}; // end class mergeable_map

} // end namespace core
} // end namespace ikos
