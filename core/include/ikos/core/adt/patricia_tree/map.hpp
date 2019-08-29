/*******************************************************************************
 *
 * \file
 * \brief Patricia tree map
 *
 * Implementation of Patricia trees based on the algorithms described in
 * C. Okasaki and A. Gill's paper: "Fast Mergeable Integer Maps",
 * Workshop on ML, September 1998, pages 77-86.
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

#include <iostream>
#include <iterator>
#include <memory>
#include <stack>

#include <boost/optional.hpp>

#include <ikos/core/adt/patricia_tree/utils.hpp>
#include <ikos/core/semantic/dumpable.hpp>
#include <ikos/core/semantic/indexable.hpp>
#include <ikos/core/support/assert.hpp>

namespace ikos {
namespace core {

// forward declarations
namespace patricia_tree_map_impl {

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace patricia_tree_utils;

template < typename Key, typename Value >
class PatriciaTree;

template < typename Key, typename Value >
class PatriciaTreeIterator;

template < typename Key, typename Value >
inline bool empty(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& tree);

template < typename Key, typename Value >
inline std::size_t size(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& tree);

template < typename Key, typename Value >
inline boost::optional< const Value& > find_value(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& tree,
    const Key& key);

template < typename Key, typename Value, typename Compare >
inline bool leq(const std::shared_ptr< const PatriciaTree< Key, Value > >& s,
                const std::shared_ptr< const PatriciaTree< Key, Value > >& t,
                const Compare& cmp);

template < typename Key, typename Value, typename Compare >
inline bool equals(const std::shared_ptr< const PatriciaTree< Key, Value > >& s,
                   const std::shared_ptr< const PatriciaTree< Key, Value > >& t,
                   const Compare& cmp);

template < typename Key, typename Value >
inline std::shared_ptr< const PatriciaTree< Key, Value > > insert_or_assign(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& tree,
    const Key& key,
    const Value& value);

template < typename Key, typename Value, typename CombiningFunction >
inline std::shared_ptr< const PatriciaTree< Key, Value > > update_or_insert(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& tree,
    const CombiningFunction& combine,
    const Key& key,
    const Value& value);

template < typename Key, typename Value, typename CombiningFunction >
inline std::shared_ptr< const PatriciaTree< Key, Value > > update_or_ignore(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& tree,
    const CombiningFunction& combine,
    const Key& key,
    const Value& value);

template < typename Key, typename Value >
inline std::shared_ptr< const PatriciaTree< Key, Value > > erase(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& tree,
    const Key& key);

template < typename Key, typename Value, typename UnaryOp >
inline std::shared_ptr< const PatriciaTree< Key, Value > > transform(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& tree,
    const UnaryOp& op);

template < typename Key, typename Value, typename CombiningFunction >
inline std::shared_ptr< const PatriciaTree< Key, Value > > join(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& s,
    const std::shared_ptr< const PatriciaTree< Key, Value > >& t,
    const CombiningFunction& combine);

template < typename Key, typename Value, typename CombiningFunction >
inline std::shared_ptr< const PatriciaTree< Key, Value > > intersect(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& s,
    const std::shared_ptr< const PatriciaTree< Key, Value > >& t,
    const CombiningFunction& combine);

template < typename Key, typename Value, typename BinaryOp >
inline typename BinaryOp::ResultType binary_operation(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& s,
    const std::shared_ptr< const PatriciaTree< Key, Value > >& t,
    const BinaryOp& op);

} // end namespace patricia_tree_map_impl

/// \brief An implementation of the patricia tree map data structure
///
/// Requirements:
///
/// Key must implement IndexableTraits
/// Key must implement bool Key::operator==(const Key&) const
/// Value must implement bool Value::operator==(const Value&) const
template < typename Key, typename Value >
class PatriciaTreeMap final {
public:
  static_assert(IsIndexable< Key >::value,
                "Key must implement IndexableTraits");

private:
  using PatriciaTree = patricia_tree_map_impl::PatriciaTree< Key, Value >;

public:
  using Iterator = patricia_tree_map_impl::PatriciaTreeIterator< Key, Value >;

private:
  std::shared_ptr< const PatriciaTree > _tree;

private:
  /// \brief Private constructor
  explicit PatriciaTreeMap(std::shared_ptr< const PatriciaTree > tree)
      : _tree(std::move(tree)) {}

public:
  /// \brief Create an empty patricia tree map
  PatriciaTreeMap() = default;

  /// \brief Copy constructor
  PatriciaTreeMap(const PatriciaTreeMap&) noexcept = default;

  /// \brief Move constructor
  PatriciaTreeMap(PatriciaTreeMap&&) noexcept = default;

  /// \brief Copy assignment operator
  PatriciaTreeMap& operator=(const PatriciaTreeMap&) noexcept = default;

  /// \brief Move assignment operator
  PatriciaTreeMap& operator=(PatriciaTreeMap&&) noexcept = default;

  /// \brief Destructor
  ~PatriciaTreeMap() = default;

  /// \brief Return true if the map is empty
  bool empty() const { return patricia_tree_map_impl::empty(this->_tree); }

  /// \brief Return the number of elements in the map
  std::size_t size() const { return patricia_tree_map_impl::size(this->_tree); }

  /// \brief Clear the content of the map
  void clear() { this->_tree.reset(); }

  /// \brief Find the value associated with the given key
  boost::optional< const Value& > at(const Key& key) const {
    return patricia_tree_map_impl::find_value(this->_tree, key);
  }

  /// \brief Return the begin iterator over the elements of the map
  Iterator begin() const { return Iterator(this->_tree); }

  /// \brief Return the end iterator over the elements of the map
  Iterator end() const { return Iterator(); }

  /// \brief Lower or equal comparison
  ///
  /// The comparison function needs to be a callable of type:
  ///   bool(const Value& left, const Value& right)
  template < typename Compare >
  bool leq(const PatriciaTreeMap& other, const Compare& cmp) const {
    return patricia_tree_map_impl::leq(this->_tree, other._tree, cmp);
  }

  /// \brief Equality comparison
  ///
  /// The comparison function should be a callable of type:
  ///   bool(const Value& left, const Value& right)
  template < typename Compare >
  bool equals(const PatriciaTreeMap& other, const Compare& cmp) const {
    return patricia_tree_map_impl::equals(this->_tree, other._tree, cmp);
  }

  /// \brief Insert an element or assign a new value for the given `key`
  void insert_or_assign(const Key& key, const Value& value) {
    this->_tree =
        patricia_tree_map_impl::insert_or_assign(this->_tree, key, value);
  }

  /// \brief Find the value corresponding to `key` and replace its bound value
  /// with `combine(old_value, value)`.
  ///
  /// If the key is not found, insert (`key`, `value`).
  ///
  /// The combining function should be a callable of type:
  ///   boost::optional< Value >(const Value& old, const Value& new)
  template < typename CombiningFunction >
  void update_or_insert(const CombiningFunction& combine,
                        const Key& key,
                        const Value& value) {
    this->_tree = patricia_tree_map_impl::update_or_insert(this->_tree,
                                                           combine,
                                                           key,
                                                           value);
  }

  /// \brief Find the value corresponding to `key` and replace its bound value
  /// with `combine(old_value, value)`.
  ///
  /// If the key is not found, leave the map unchanged.
  ///
  /// The combining function should be a callable of type:
  ///   boost::optional< Value >(const Value& old, const Value& new)
  template < typename CombiningFunction >
  void update_or_ignore(const CombiningFunction& combine,
                        const Key& key,
                        const Value& value) {
    this->_tree = patricia_tree_map_impl::update_or_ignore(this->_tree,
                                                           combine,
                                                           key,
                                                           value);
  }

  /// \brief Remove an element from the map, if present
  void erase(const Key& key) {
    this->_tree = patricia_tree_map_impl::erase(this->_tree, key);
  }

  /// \brief Apply an unary operator on all the elements
  ///
  /// The operator should be a callable of type:
  ///   boost::optional< Value >(const Key& key, const Value& value)
  template < typename UnaryOp >
  void transform(const UnaryOp& op) {
    this->_tree = patricia_tree_map_impl::transform(this->_tree, op);
  }

  /// \brief Perform the union of two patricia tree maps
  ///
  /// The combining function should be a callable of type:
  ///   boost::optional< Value >(const Value& left, const Value& right)
  template < typename CombiningFunction >
  void join_with(const PatriciaTreeMap& other,
                 const CombiningFunction& combine) {
    this->_tree =
        patricia_tree_map_impl::join(this->_tree, other._tree, combine);
  }

  /// \brief Perform the union of two patricia tree maps
  ///
  /// The combining function should be a callable of type:
  ///   boost::optional< Value >(const Value& left, const Value& right)
  template < typename CombiningFunction >
  PatriciaTreeMap join(const PatriciaTreeMap& other,
                       const CombiningFunction& combine) const {
    return PatriciaTreeMap(
        patricia_tree_map_impl::join(this->_tree, other._tree, combine));
  }

  /// \brief Perform the intersection of two patricia tree maps
  ///
  /// The combining function should be a callable of type:
  ///   boost::optional< Value >(const Value& left, const Value& right)
  template < typename CombiningFunction >
  void intersect_with(const PatriciaTreeMap& other,
                      const CombiningFunction& combine) {
    this->_tree =
        patricia_tree_map_impl::intersect(this->_tree, other._tree, combine);
  }

  /// \brief Perform the intersection of two patricia tree maps
  ///
  /// The combining function should be a callable of type:
  ///   boost::optional< Value >(const Value& left, const Value& right)
  template < typename CombiningFunction >
  PatriciaTreeMap intersect(const PatriciaTreeMap& other,
                            const CombiningFunction& combine) const {
    return PatriciaTreeMap(
        patricia_tree_map_impl::intersect(this->_tree, other._tree, combine));
  }

  /// \brief Perform a generic binary operation
  ///
  /// Example of binary operator:
  ///
  /// \code{.cpp}
  /// struct MyBinaryOp {
  ///   // Result type
  ///   using ResultType = ...;
  ///
  ///   // Return true if the binary operator has a special behavior on
  ///   // equality of trees
  ///   bool has_equals() const { ... }
  ///
  ///   // If has_equals() is true, return the result if two trees are equal,
  ///   // given the tree `t`
  ///   ResultType equals(const PatriciaTreeMap< Key, Value >& t) const { ... }
  ///
  ///   // Return the result of the binary operation with an empty right hand
  ///   // side tree, given the left hand side tree `l`
  ///   ResultType left(const PatriciaTreeMap< Key, Value >& l) const { ... }
  ///
  ///   // Return the result of the binary operation with an empty left hand
  ///   // side tree, given the right hand side tree `r`
  ///   ResultType right(const PatriciaTreeMap< Key, Value >& r) const { ... }
  ///
  ///   // Return the result of the binary operation with a singleton
  ///   // (`k`, `v`) as a right hand side tree, and the left hand side tree `l`
  ///   ResultType left_with_right_leaf(const PatriciaTreeMap< Key, Value >& l,
  ///                                   const Key& k,
  ///                                   const Value& v) const { ... }
  ///
  ///   // Return the result of the binary operation with a singleton
  ///   // (`k`, `v`) as a left hand side tree, and the right hand side tree `r`
  ///   ResultType right_with_left_leaf(const PatriciaTreeMap< Key, Value >& r,
  ///                                   const Key& k,
  ///                                   const Value& v) const { ... }
  ///
  ///   // Return the result of the binary operation, given the results for
  ///   // subtrees with separated keys
  ///   ResultType merge(ResultType l, ResultType r) const { ... }
  /// };
  /// \endcode
  template < typename BinaryOp >
  typename BinaryOp::ResultType binary_operation(const PatriciaTreeMap& other,
                                                 const BinaryOp& op) const {
    return patricia_tree_map_impl::binary_operation(this->_tree,
                                                    other._tree,
                                                    op);
  }

  /// \brief Dump the map, for debugging purpose
  void dump(std::ostream& o) const {
    static_assert(IsDumpable< Key >::value,
                  "Key must implement DumpableTraits");
    static_assert(IsDumpable< Value >::value,
                  "Value must implement DumpableTraits");
    o << "{";
    for (auto it = this->begin(), et = this->end(); it != et;) {
      DumpableTraits< Key >::dump(o, it->first);
      o << " -> ";
      DumpableTraits< Value >::dump(o, it->second);
      ++it;
      if (it != et) {
        o << "; ";
      }
    }
    o << "}";
  }

  // Allow binary_operation to call the private constructor
  template < typename K, typename V, typename BinaryOp >
  friend typename BinaryOp::ResultType patricia_tree_map_impl::binary_operation(
      const std::shared_ptr<
          const patricia_tree_map_impl::PatriciaTree< K, V > >& s,
      const std::shared_ptr<
          const patricia_tree_map_impl::PatriciaTree< K, V > >& t,
      const BinaryOp& op);

}; // end class PatriciaTreeMap

namespace patricia_tree_map_impl {

template < typename Key, typename Value >
class PatriciaTree {
private:
  std::size_t _size;

public:
  explicit PatriciaTree(std::size_t size) : _size(size) {}

  // PatriciaTree is immutable
  PatriciaTree(const PatriciaTree&) = delete;
  PatriciaTree(PatriciaTree&&) = delete;
  PatriciaTree& operator=(const PatriciaTree&) = delete;
  PatriciaTree& operator=(PatriciaTree&&) = delete;

  virtual ~PatriciaTree() = default;

  std::size_t size() const { return this->_size; }

  bool is_leaf() const { return this->_size == 1; }

  bool is_node() const { return !this->is_leaf(); }

}; // end class PatriciaTree

template < typename Key, typename Value >
class PatriciaTreeNode final : public PatriciaTree< Key, Value > {
private:
  Index _prefix;
  Index _branching_bit;
  std::shared_ptr< const PatriciaTree< Key, Value > > _left_tree;
  std::shared_ptr< const PatriciaTree< Key, Value > > _right_tree;

public:
  PatriciaTreeNode(
      Index prefix,
      Index branching_bit,
      std::shared_ptr< const PatriciaTree< Key, Value > > left_tree,
      std::shared_ptr< const PatriciaTree< Key, Value > > right_tree)
      : PatriciaTree< Key, Value >(left_tree->size() + right_tree->size()),
        _prefix(prefix),
        _branching_bit(branching_bit),
        _left_tree(std::move(left_tree)),
        _right_tree(std::move(right_tree)) {}

  Index prefix() const { return this->_prefix; }

  Index branching_bit() const { return this->_branching_bit; }

  const std::shared_ptr< const PatriciaTree< Key, Value > >& left_tree() const {
    return this->_left_tree;
  }

  const std::shared_ptr< const PatriciaTree< Key, Value > >& right_tree()
      const {
    return this->_right_tree;
  }

}; // end class PatriciaTreeNode

template < typename Key, typename Value >
class PatriciaTreeLeaf final : public PatriciaTree< Key, Value > {
private:
  std::pair< Key, Value > _pair;

public:
  PatriciaTreeLeaf(const Key& key, const Value& value)
      : PatriciaTree< Key, Value >(1), _pair(key, value) {}

  const Key& key() const { return this->_pair.first; }

  const Value& value() const { return this->_pair.second; }

  const std::pair< Key, Value >& pair() const { return this->_pair; }

}; // end class PatriciaTreeLeaf

template < typename Key, typename Value >
inline bool empty(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& tree) {
  return tree == nullptr;
}

template < typename Key, typename Value >
inline std::size_t size(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& tree) {
  if (tree != nullptr) {
    return tree->size();
  } else {
    return 0;
  }
}

/// \brief Return the leaf associated with the given key, or nullptr
template < typename Key, typename Value >
inline std::shared_ptr< const PatriciaTreeLeaf< Key, Value > > find_leaf(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& tree,
    const Key& key) {
  if (tree == nullptr) {
    return nullptr;
  }
  if (tree->is_leaf()) {
    auto leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(tree);
    if (leaf->key() != key) {
      return nullptr;
    }
    return leaf;
  }
  auto node =
      std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(tree);
  if (is_zero_bit(IndexableTraits< Key >::index(key), node->branching_bit())) {
    return find_leaf(node->left_tree(), key);
  } else {
    return find_leaf(node->right_tree(), key);
  }
}

template < typename Key, typename Value >
inline boost::optional< const Value& > find_value(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& tree,
    const Key& key) {
  auto leaf = find_leaf(tree, key);
  if (leaf == nullptr) {
    return boost::none;
  } else {
    return leaf->value();
  }
}

template < typename Key, typename Value, typename Compare >
inline bool leq(const std::shared_ptr< const PatriciaTree< Key, Value > >& s,
                const std::shared_ptr< const PatriciaTree< Key, Value > >& t,
                const Compare& cmp) {
  if (s == t) {
    return true;
  }
  if (s == nullptr) {
    return false;
  }
  if (t == nullptr) {
    return true;
  }
  if (s->is_leaf()) {
    if (t->is_node()) {
      return false;
    }
    auto s_leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(s);
    auto t_leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(t);
    return s_leaf->key() == t_leaf->key() &&
           cmp(s_leaf->value(), t_leaf->value());
  }
  if (t->is_leaf()) {
    auto t_leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(t);
    auto s_value = find_value(s, t_leaf->key());
    if (s_value) {
      return cmp(*s_value, t_leaf->value());
    } else {
      return false;
    }
  }
  auto s_node =
      std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(s);
  auto t_node =
      std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(t);
  if (s_node->size() < t_node->size()) {
    return false;
  }
  Index m = s_node->branching_bit();
  Index n = t_node->branching_bit();
  Index p = s_node->prefix();
  Index q = t_node->prefix();
  if (m == n && p == q) {
    return leq(s_node->left_tree(), t_node->left_tree(), cmp) &&
           leq(s_node->right_tree(), t_node->right_tree(), cmp);
  }
  if (m < n && match_prefix(q, p, m)) {
    if (is_zero_bit(q, m)) {
      return leq(s_node->left_tree(), t, cmp);
    } else {
      return leq(s_node->right_tree(), t, cmp);
    }
  }
  return false; // t contains bindings that are not in s
}

template < typename Key, typename Value, typename Compare >
inline bool equals(const std::shared_ptr< const PatriciaTree< Key, Value > >& s,
                   const std::shared_ptr< const PatriciaTree< Key, Value > >& t,
                   const Compare& cmp) {
  if (s == t) {
    return true;
  }
  if (s == nullptr || t == nullptr) {
    return false;
  }
  if (s->is_leaf()) {
    if (t->is_node()) {
      return false;
    }
    auto s_leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(s);
    auto t_leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(t);
    return s_leaf->key() == t_leaf->key() &&
           cmp(s_leaf->value(), t_leaf->value());
  }
  if (t->is_leaf()) {
    return false;
  }
  auto s_node =
      std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(s);
  auto t_node =
      std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(t);
  return s_node->size() == t_node->size() &&
         s_node->prefix() == t_node->prefix() &&
         s_node->branching_bit() == t_node->branching_bit() &&
         equals(s_node->left_tree(), t_node->left_tree(), cmp) &&
         equals(s_node->right_tree(), t_node->right_tree(), cmp);
}

/// \brief Create a node
///
/// Prevent the creation of a node with only one child.
template < typename Key, typename Value >
inline std::shared_ptr< const PatriciaTree< Key, Value > > make_node(
    Index prefix,
    Index branching_bit,
    const std::shared_ptr< const PatriciaTree< Key, Value > >& left_tree,
    const std::shared_ptr< const PatriciaTree< Key, Value > >& right_tree) {
  if (left_tree == nullptr) {
    return right_tree;
  }
  if (right_tree == nullptr) {
    return left_tree;
  }
  return std::make_shared< const PatriciaTreeNode< Key, Value > >(prefix,
                                                                  branching_bit,
                                                                  left_tree,
                                                                  right_tree);
}

/// \brief Join non-null patricia trees
template < typename Key, typename Value >
inline std::shared_ptr< const PatriciaTreeNode< Key, Value > > join_trees(
    Index prefix_s,
    const std::shared_ptr< const PatriciaTree< Key, Value > >& s,
    Index prefix_t,
    const std::shared_ptr< const PatriciaTree< Key, Value > >& t) {
  ikos_assert(s != nullptr && t != nullptr);

  Index m = branching_bit(prefix_s, prefix_t);

  if (is_zero_bit(prefix_s, m)) {
    return std::make_shared<
        const PatriciaTreeNode< Key, Value > >(mask(prefix_s, m), m, s, t);
  } else {
    return std::make_shared<
        const PatriciaTreeNode< Key, Value > >(mask(prefix_s, m), m, t, s);
  }
}

template < typename Key, typename Value >
inline std::shared_ptr< const PatriciaTree< Key, Value > > insert_or_assign(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& tree,
    const Key& key,
    const Value& value) {
  if (tree == nullptr) {
    return std::make_shared< const PatriciaTreeLeaf< Key, Value > >(key, value);
  }
  if (tree->is_leaf()) {
    auto leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(tree);
    if (leaf->key() == key) {
      if (leaf->value() == value) {
        return tree;
      } else {
        return std::make_shared< const PatriciaTreeLeaf< Key, Value > >(key,
                                                                        value);
      }
    }
    auto new_leaf =
        std::make_shared< const PatriciaTreeLeaf< Key, Value > >(key, value);
    return join_trees< Key, Value >(IndexableTraits< Key >::index(key),
                                    new_leaf,
                                    IndexableTraits< Key >::index(leaf->key()),
                                    leaf);
  }
  auto node =
      std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(tree);
  if (match_prefix(IndexableTraits< Key >::index(key),
                   node->prefix(),
                   node->branching_bit())) {
    if (is_zero_bit(IndexableTraits< Key >::index(key),
                    node->branching_bit())) {
      auto new_left_tree = insert_or_assign(node->left_tree(), key, value);
      if (new_left_tree == node->left_tree()) {
        return std::move(node);
      }
      return make_node(node->prefix(),
                       node->branching_bit(),
                       new_left_tree,
                       node->right_tree());
    } else {
      auto new_right_tree = insert_or_assign(node->right_tree(), key, value);
      if (new_right_tree == node->right_tree()) {
        return std::move(node);
      }
      return make_node(node->prefix(),
                       node->branching_bit(),
                       node->left_tree(),
                       new_right_tree);
    }
  }
  auto new_leaf =
      std::make_shared< const PatriciaTreeLeaf< Key, Value > >(key, value);
  return join_trees< Key, Value >(IndexableTraits< Key >::index(key),
                                  new_leaf,
                                  node->prefix(),
                                  node);
}

template < typename Key, typename Value, typename CombiningFunction >
inline std::shared_ptr< const PatriciaTree< Key, Value > > update_or_insert(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& tree,
    const CombiningFunction& combine,
    const Key& key,
    const Value& value) {
  if (tree == nullptr) {
    return std::make_shared< const PatriciaTreeLeaf< Key, Value > >(key, value);
  }
  if (tree->is_leaf()) {
    auto leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(tree);
    if (leaf->key() == key) {
      boost::optional< Value > new_value = combine(leaf->value(), value);
      if (new_value) {
        if (leaf->value() == *new_value) {
          return tree;
        } else {
          return std::make_shared<
              const PatriciaTreeLeaf< Key, Value > >(key, *new_value);
        }
      }
      return nullptr;
    }
    auto new_leaf =
        std::make_shared< const PatriciaTreeLeaf< Key, Value > >(key, value);
    return join_trees< Key, Value >(IndexableTraits< Key >::index(key),
                                    new_leaf,
                                    IndexableTraits< Key >::index(leaf->key()),
                                    leaf);
  }
  auto node =
      std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(tree);
  if (match_prefix(IndexableTraits< Key >::index(key),
                   node->prefix(),
                   node->branching_bit())) {
    if (is_zero_bit(IndexableTraits< Key >::index(key),
                    node->branching_bit())) {
      auto new_left_tree =
          update_or_insert(node->left_tree(), combine, key, value);
      if (new_left_tree == node->left_tree()) {
        return std::move(node);
      }
      return make_node(node->prefix(),
                       node->branching_bit(),
                       new_left_tree,
                       node->right_tree());
    } else {
      auto new_right_tree =
          update_or_insert(node->right_tree(), combine, key, value);
      if (new_right_tree == node->right_tree()) {
        return std::move(node);
      }
      return make_node(node->prefix(),
                       node->branching_bit(),
                       node->left_tree(),
                       new_right_tree);
    }
  }
  auto new_leaf =
      std::make_shared< const PatriciaTreeLeaf< Key, Value > >(key, value);
  return join_trees< Key, Value >(IndexableTraits< Key >::index(key),
                                  new_leaf,
                                  node->prefix(),
                                  node);
}

template < typename Key, typename Value, typename CombiningFunction >
inline std::shared_ptr< const PatriciaTree< Key, Value > > update_or_ignore(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& tree,
    const CombiningFunction& combine,
    const Key& key,
    const Value& value) {
  if (tree == nullptr) {
    return nullptr;
  }
  if (tree->is_leaf()) {
    auto leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(tree);
    if (leaf->key() == key) {
      boost::optional< Value > new_value = combine(leaf->value(), value);
      if (new_value) {
        if (leaf->value() == *new_value) {
          return tree;
        } else {
          return std::make_shared<
              const PatriciaTreeLeaf< Key, Value > >(key, *new_value);
        }
      }
      return nullptr;
    }
    return tree;
  }
  auto node =
      std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(tree);
  if (match_prefix(IndexableTraits< Key >::index(key),
                   node->prefix(),
                   node->branching_bit())) {
    if (is_zero_bit(IndexableTraits< Key >::index(key),
                    node->branching_bit())) {
      auto new_left_tree =
          update_or_ignore(node->left_tree(), combine, key, value);
      if (new_left_tree == node->left_tree()) {
        return node;
      }
      return make_node(node->prefix(),
                       node->branching_bit(),
                       new_left_tree,
                       node->right_tree());
    } else {
      auto new_right_tree =
          update_or_ignore(node->right_tree(), combine, key, value);
      if (new_right_tree == node->right_tree()) {
        return node;
      }
      return make_node(node->prefix(),
                       node->branching_bit(),
                       node->left_tree(),
                       new_right_tree);
    }
  }
  return tree;
}

/// \brief Update or insert an existing leaf `t_leaf` in a tree `s`
template < typename Key, typename Value, typename CombiningFunction >
inline std::shared_ptr< const PatriciaTree< Key, Value > >
update_or_insert_leaf(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& s,
    const std::shared_ptr< const PatriciaTreeLeaf< Key, Value > >& t_leaf,
    const CombiningFunction& combine) {
  if (s == t_leaf) {
    return s;
  }
  if (s == nullptr) {
    return t_leaf;
  }
  if (s->is_leaf()) {
    auto s_leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(s);
    if (s_leaf->key() == t_leaf->key()) {
      boost::optional< Value > new_value =
          combine(s_leaf->value(), t_leaf->value());
      if (new_value) {
        if (s_leaf->value() == *new_value) {
          return std::move(s_leaf);
        } else if (t_leaf->value() == *new_value) {
          return t_leaf;
        } else {
          return std::make_shared<
              const PatriciaTreeLeaf< Key, Value > >(s_leaf->key(), *new_value);
        }
      }
      return nullptr;
    }
    return join_trees< Key, Value >(IndexableTraits< Key >::index(
                                        s_leaf->key()),
                                    s_leaf,
                                    IndexableTraits< Key >::index(
                                        t_leaf->key()),
                                    t_leaf);
  }
  auto s_node =
      std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(s);
  if (match_prefix(IndexableTraits< Key >::index(t_leaf->key()),
                   s_node->prefix(),
                   s_node->branching_bit())) {
    if (is_zero_bit(IndexableTraits< Key >::index(t_leaf->key()),
                    s_node->branching_bit())) {
      auto new_left_tree =
          update_or_insert_leaf(s_node->left_tree(), t_leaf, combine);
      if (new_left_tree == s_node->left_tree()) {
        return std::move(s_node);
      }
      return make_node(s_node->prefix(),
                       s_node->branching_bit(),
                       new_left_tree,
                       s_node->right_tree());
    } else {
      auto new_right_tree =
          update_or_insert_leaf(s_node->right_tree(), t_leaf, combine);
      if (new_right_tree == s_node->right_tree()) {
        return std::move(s_node);
      }
      return make_node(s_node->prefix(),
                       s_node->branching_bit(),
                       s_node->left_tree(),
                       new_right_tree);
    }
  }
  return join_trees< Key, Value >(s_node->prefix(),
                                  s_node,
                                  IndexableTraits< Key >::index(t_leaf->key()),
                                  t_leaf);
}

template < typename Key, typename Value >
inline std::shared_ptr< const PatriciaTree< Key, Value > > erase(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& tree,
    const Key& key) {
  if (tree == nullptr) {
    return nullptr;
  }
  if (tree->is_leaf()) {
    auto leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(tree);
    if (leaf->key() == key) {
      return nullptr;
    } else {
      return tree;
    }
  }
  auto node =
      std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(tree);
  if (match_prefix(IndexableTraits< Key >::index(key),
                   node->prefix(),
                   node->branching_bit())) {
    if (is_zero_bit(IndexableTraits< Key >::index(key),
                    node->branching_bit())) {
      auto new_left_tree = erase(node->left_tree(), key);
      if (new_left_tree == node->left_tree()) {
        return std::move(node);
      }
      return make_node(node->prefix(),
                       node->branching_bit(),
                       new_left_tree,
                       node->right_tree());
    } else {
      auto new_right_tree = erase(node->right_tree(), key);
      if (new_right_tree == node->right_tree()) {
        return std::move(node);
      }
      return make_node(node->prefix(),
                       node->branching_bit(),
                       node->left_tree(),
                       new_right_tree);
    }
  }
  return tree;
}

template < typename Key, typename Value, typename UnaryOp >
inline std::shared_ptr< const PatriciaTree< Key, Value > > transform(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& tree,
    const UnaryOp& op) {
  if (tree == nullptr) {
    return nullptr;
  }
  if (tree->is_leaf()) {
    auto leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(tree);
    boost::optional< Value > new_value = op(leaf->key(), leaf->value());
    if (new_value) {
      if (leaf->value() == *new_value) {
        return tree;
      } else {
        return std::make_shared<
            const PatriciaTreeLeaf< Key, Value > >(leaf->key(), *new_value);
      }
    }
    return nullptr;
  }
  auto node =
      std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(tree);
  auto new_left_tree = transform(node->left_tree(), op);
  auto new_right_tree = transform(node->right_tree(), op);
  if (node->left_tree() == new_left_tree &&
      node->right_tree() == new_right_tree) {
    return tree;
  } else {
    return make_node(node->prefix(),
                     node->branching_bit(),
                     new_left_tree,
                     new_right_tree);
  }
}

template < typename Key, typename Value, typename CombiningFunction >
inline std::shared_ptr< const PatriciaTree< Key, Value > > join(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& s,
    const std::shared_ptr< const PatriciaTree< Key, Value > >& t,
    const CombiningFunction& combine) {
  if (s == t) {
    return s;
  }
  if (s == nullptr) {
    return t;
  }
  if (t == nullptr) {
    return s;
  }
  if (s->is_leaf()) {
    auto s_leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(s);
    return update_or_insert_leaf(t,
                                 s_leaf,
                                 [=](const Value& t_value,
                                     const Value& s_value) {
                                   // reverse the parameters
                                   return combine(s_value, t_value);
                                 });
  }
  if (t->is_leaf()) {
    auto t_leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(t);
    return update_or_insert_leaf(s, t_leaf, combine);
  }
  auto s_node =
      std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(s);
  auto t_node =
      std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(t);
  Index m = s_node->branching_bit();
  Index n = t_node->branching_bit();
  Index p = s_node->prefix();
  Index q = t_node->prefix();
  if (m == n && p == q) {
    // The two trees have the same prefix
    auto new_left = join(s_node->left_tree(), t_node->left_tree(), combine);
    auto new_right = join(s_node->right_tree(), t_node->right_tree(), combine);
    if (new_left == s_node->left_tree() && new_right == s_node->right_tree()) {
      return s;
    }
    if (new_left == t_node->left_tree() && new_right == t_node->right_tree()) {
      return t;
    }
    return make_node(p, m, new_left, new_right);
  }
  if (m < n && match_prefix(q, p, m)) {
    // q contains p, join t with a subtree of s
    if (is_zero_bit(q, m)) {
      auto new_left = join(s_node->left_tree(), t, combine);
      if (s_node->left_tree() == new_left) {
        return s;
      }
      return make_node(p, m, new_left, s_node->right_tree());
    } else {
      auto new_right = join(s_node->right_tree(), t, combine);
      if (s_node->right_tree() == new_right) {
        return s;
      }
      return make_node(p, m, s_node->left_tree(), new_right);
    }
  }
  if (m > n && match_prefix(p, q, n)) {
    // p contains q. Merge s with a subtree of t.
    if (is_zero_bit(p, n)) {
      auto new_left = join(s, t_node->left_tree(), combine);
      if (t_node->left_tree() == new_left) {
        return t;
      }
      return make_node(q, n, new_left, t_node->right_tree());
    } else {
      auto new_right = join(s, t_node->right_tree(), combine);
      if (t_node->right_tree() == new_right) {
        return t;
      }
      return make_node(q, n, t_node->left_tree(), new_right);
    }
  }
  // The prefixes disagree
  return join_trees(p, s, q, t);
}

template < typename Key, typename Value, typename CombiningFunction >
inline std::shared_ptr< const PatriciaTree< Key, Value > > intersect(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& s,
    const std::shared_ptr< const PatriciaTree< Key, Value > >& t,
    const CombiningFunction& combine) {
  if (s == t) {
    return s;
  }
  if (s == nullptr || t == nullptr) {
    return nullptr;
  }
  if (s->is_leaf()) {
    auto s_leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(s);
    auto t_leaf = find_leaf(t, s_leaf->key());
    if (t_leaf) {
      boost::optional< Value > new_value =
          combine(s_leaf->value(), t_leaf->value());
      if (new_value) {
        if (s_leaf->value() == *new_value) {
          return std::move(s_leaf);
        } else if (t_leaf->value() == *new_value) {
          return std::move(t_leaf);
        } else {
          return std::make_shared<
              const PatriciaTreeLeaf< Key, Value > >(s_leaf->key(), *new_value);
        }
      }
    }
    return nullptr;
  }
  if (t->is_leaf()) {
    auto t_leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(t);
    auto s_leaf = find_leaf(s, t_leaf->key());
    if (s_leaf) {
      boost::optional< Value > new_value =
          combine(s_leaf->value(), t_leaf->value());
      if (new_value) {
        if (s_leaf->value() == *new_value) {
          return std::move(s_leaf);
        } else if (t_leaf->value() == *new_value) {
          return std::move(t_leaf);
        } else {
          return std::make_shared<
              const PatriciaTreeLeaf< Key, Value > >(t_leaf->key(), *new_value);
        }
      }
    }
    return nullptr;
  }
  auto s_node =
      std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(s);
  auto t_node =
      std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(t);
  Index m = s_node->branching_bit();
  Index n = t_node->branching_bit();
  Index p = s_node->prefix();
  Index q = t_node->prefix();
  if (m == n && p == q) {
    // The two trees have the same prefix
    auto new_left =
        intersect(s_node->left_tree(), t_node->left_tree(), combine);
    auto new_right =
        intersect(s_node->right_tree(), t_node->right_tree(), combine);
    if (new_left == s_node->left_tree() && new_right == s_node->right_tree()) {
      return s;
    }
    if (new_left == t_node->left_tree() && new_right == t_node->right_tree()) {
      return t;
    }
    return make_node(p, m, new_left, new_right);
  }
  if (m < n && match_prefix(q, p, m)) {
    // q contains p, intersect t with a subtree of s
    if (is_zero_bit(q, m)) {
      return intersect(s_node->left_tree(), t, combine);
    } else {
      return intersect(s_node->right_tree(), t, combine);
    }
  }
  if (m > n && match_prefix(p, q, n)) {
    // p contains q, intersect s with a subtree of t
    if (is_zero_bit(p, n)) {
      return intersect(s, t_node->left_tree(), combine);
    } else {
      return intersect(s, t_node->right_tree(), combine);
    }
  }
  // The prefixes disagree
  return nullptr;
}

template < typename Key, typename Value, typename BinaryOp >
inline typename BinaryOp::ResultType binary_operation(
    const std::shared_ptr< const PatriciaTree< Key, Value > >& s,
    const std::shared_ptr< const PatriciaTree< Key, Value > >& t,
    const BinaryOp& op) {
  if (op.has_equals() && s == t) {
    return op.equals(PatriciaTreeMap< Key, Value >(s));
  }
  if (s == nullptr) {
    return op.right(PatriciaTreeMap< Key, Value >(t));
  }
  if (t == nullptr) {
    return op.left(PatriciaTreeMap< Key, Value >(s));
  }
  if (s->is_leaf()) {
    auto s_leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(s);
    return op.right_with_left_leaf(PatriciaTreeMap< Key, Value >(t),
                                   s_leaf->key(),
                                   s_leaf->value());
  }
  if (t->is_leaf()) {
    auto t_leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(t);
    return op.left_with_right_leaf(PatriciaTreeMap< Key, Value >(s),
                                   t_leaf->key(),
                                   t_leaf->value());
  }
  auto s_node =
      std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(s);
  auto t_node =
      std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(t);
  Index m = s_node->branching_bit();
  Index n = t_node->branching_bit();
  Index p = s_node->prefix();
  Index q = t_node->prefix();
  if (m == n && p == q) {
    // The two trees have the same prefix
    return op.merge(binary_operation(s_node->left_tree(),
                                     t_node->left_tree(),
                                     op),
                    binary_operation(s_node->right_tree(),
                                     t_node->right_tree(),
                                     op));
  }
  if (m < n && match_prefix(q, p, m)) {
    // q contains p, join t with a subtree of s
    if (is_zero_bit(q, m)) {
      return op.merge(op.left(
                          PatriciaTreeMap< Key, Value >(s_node->right_tree())),
                      binary_operation(s_node->left_tree(), t, op));
    } else {
      return op.merge(op.left(
                          PatriciaTreeMap< Key, Value >(s_node->left_tree())),
                      binary_operation(s_node->right_tree(), t, op));
    }
  }
  if (m > n && match_prefix(p, q, n)) {
    // p contains q. Merge s with a subtree of t.
    if (is_zero_bit(p, n)) {
      return op.merge(op.right(
                          PatriciaTreeMap< Key, Value >(t_node->right_tree())),
                      binary_operation(s, t_node->left_tree(), op));
    } else {
      return op.merge(op.right(
                          PatriciaTreeMap< Key, Value >(t_node->left_tree())),
                      binary_operation(s, t_node->right_tree(), op));
    }
  }
  return op.merge(op.left(PatriciaTreeMap< Key, Value >(s)),
                  op.right(PatriciaTreeMap< Key, Value >(t)));
}

template < typename Key, typename Value >
class PatriciaTreeIterator final {
public:
  // Required types for iterators
  using iterator_category = std::forward_iterator_tag;
  using value_type = const std::pair< Key, Value >;
  using difference_type = std::ptrdiff_t;
  using pointer = const std::pair< Key, Value >*;
  using reference = const std::pair< Key, Value >&;

private:
  std::shared_ptr< const PatriciaTreeLeaf< Key, Value > > _leaf;
  std::stack< std::shared_ptr< const PatriciaTreeNode< Key, Value > > > _stack;

public:
  /// \brief Create an end iterator
  PatriciaTreeIterator() = default;

  /// \brief Create an iterator on the given patricia tree
  explicit PatriciaTreeIterator(
      const std::shared_ptr< const PatriciaTree< Key, Value > >& tree) {
    if (tree != nullptr) {
      this->look_for_next_leaf(tree);
    }
  }

  /// \brief Pre-increment the iterator
  PatriciaTreeIterator& operator++() {
    ikos_assert(this->_leaf != nullptr);

    if (this->_stack.empty()) {
      // We reached the end
      this->_leaf = nullptr;
      return *this;
    }

    // Otherwise, we pop out a branch from the stack and move to the leftmost
    // leaf in its right-hand subtree.
    auto node = this->_stack.top();
    this->_stack.pop();
    this->look_for_next_leaf(node->right_tree());
    return *this;
  }

  /// \brief Post-increment the iterator
  const PatriciaTreeIterator operator++(int) {
    PatriciaTreeIterator r = *this;
    ++(*this);
    return r;
  }

  /// \brief Compare two iterators
  bool operator==(const PatriciaTreeIterator& other) const {
    // No need to check the stack
    return this->_leaf == other._leaf;
  }

  /// \brief Compare two iterators
  bool operator!=(const PatriciaTreeIterator& other) const {
    // No need to check the stack
    return this->_leaf != other._leaf;
  }

  /// \brief Dereference the iterator
  reference operator*() const { return this->_leaf->pair(); }

  /// \brief Dereference the iterator
  pointer operator->() const { return &this->_leaf->pair(); }

private:
  /// \brief Find the leftmost leaf, store all intermediate nodes
  void look_for_next_leaf(
      const std::shared_ptr< const PatriciaTree< Key, Value > >& tree) {
    auto t = tree;
    ikos_assert(t != nullptr);
    while (t->is_node()) {
      auto node =
          std::static_pointer_cast< const PatriciaTreeNode< Key, Value > >(t);
      this->_stack.push(node);
      t = node->left_tree();
      ikos_assert(t != nullptr); // a node always has two children
    }
    this->_leaf =
        std::static_pointer_cast< const PatriciaTreeLeaf< Key, Value > >(t);
  }

}; // end class PatriciaTreeIterator

} // end namespace patricia_tree_map_impl
} // end namespace core
} // end namespace ikos
