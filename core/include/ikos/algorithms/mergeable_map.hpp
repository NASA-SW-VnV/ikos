/**************************************************************************/ /**
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

#ifndef IKOS_MERGEABLE_MAP_HPP
#define IKOS_MERGEABLE_MAP_HPP

#include <boost/optional.hpp>

#include <ikos/algorithms/patricia_trees.hpp>
#include <ikos/common/types.hpp>

namespace ikos {

/// \brief A mergeable map implementation based on patricia trees
template < typename Key, typename Value >
class mergeable_map : public writeable {
private:
  typedef patricia_tree< Key, Value > patricia_tree_t;
  typedef typename patricia_tree_t::binary_op_t binary_op_t;

public:
  typedef mergeable_map< Key, Value > mergeable_map_t;
  typedef typename patricia_tree_t::iterator iterator;

private:
  patricia_tree_t _tree;

private:
  class union_op : public binary_op_t {
    boost::optional< Value > apply(Value x, Value y) {
      if (x == y)
        return boost::optional< Value >(x);
      else {
        throw logic_error(
            "mergeable map: merging a key with two different values");
        return boost::optional< Value >();
      }
    };

    bool default_is_absorbing() { return false; }

  }; // end class union_op

private:
  static patricia_tree_t do_union(patricia_tree_t t1, patricia_tree_t t2) {
    union_op o;
    t1.merge_with(t2, o);
    return t1;
  }

private:
  mergeable_map(patricia_tree_t t) : _tree(t) {}

public:
  mergeable_map() : _tree(patricia_tree_t()) {}

  mergeable_map(const mergeable_map_t& e) : _tree(e._tree) {}

  mergeable_map_t& operator=(const mergeable_map_t& e) {
    this->_tree = e._tree;
    return *this;
  }

  iterator begin() const { return this->_tree.begin(); }

  iterator end() const { return this->_tree.end(); }

  std::size_t size() const { return this->_tree.size(); }

  mergeable_map_t operator|(mergeable_map_t e) const {
    mergeable_map_t u(do_union(this->_tree, e._tree));
    return u;
  }

  void set(const Key& k, const Value& v) { this->_tree.insert(k, v); }

  mergeable_map_t& operator-=(const Key& k) {
    this->_tree.remove(k);
    return *this;
  }

  boost::optional< Value > operator[](const Key& k) const {
    return this->_tree.lookup(k);
  }

  void clear() { this->_tree = patricia_tree_t(); }

  void write(std::ostream& o) {
    o << "{";
    for (typename patricia_tree_t::iterator it = this->_tree.begin();
         it != this->_tree.end();) {
      Key key = it->first;
      index_traits::write< Key >(o, key);
      o << " -> ";
      o << it->second;
      ++it;
      if (it != this->_tree.end()) {
        o << "; ";
      }
    }
    o << "}";
  }

}; // end class mergeable_map

} // end namespace ikos

#endif // IKOS_MERGEABLE_MAP_HPP
