/*******************************************************************************
 *
 * \file
 * \brief Patricia tree set
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

#include <ikos/core/adt/patricia_tree/utils.hpp>
#include <ikos/core/semantic/dumpable.hpp>
#include <ikos/core/semantic/indexable.hpp>
#include <ikos/core/support/assert.hpp>

namespace ikos {
namespace core {

// forward declarations
namespace patricia_tree_set_impl {

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace patricia_tree_utils;

template < typename Key >
class PatriciaTree;

template < typename Key >
class PatriciaTreeIterator;

template < typename Key >
inline bool empty(const std::shared_ptr< const PatriciaTree< Key > >& tree);

template < typename Key >
inline std::size_t size(
    const std::shared_ptr< const PatriciaTree< Key > >& tree);

template < typename Key >
inline bool contains(const std::shared_ptr< const PatriciaTree< Key > >& tree,
                     const Key& key);

template < typename Key >
inline bool is_subset_of(const std::shared_ptr< const PatriciaTree< Key > >& s,
                         const std::shared_ptr< const PatriciaTree< Key > >& t);

template < typename Key >
inline bool equals(const std::shared_ptr< const PatriciaTree< Key > >& s,
                   const std::shared_ptr< const PatriciaTree< Key > >& t);

template < typename Key >
inline std::shared_ptr< const PatriciaTree< Key > > insert(
    const std::shared_ptr< const PatriciaTree< Key > >& tree, const Key& key);

template < typename Key >
inline std::shared_ptr< const PatriciaTree< Key > > erase(
    const std::shared_ptr< const PatriciaTree< Key > >& tree, const Key& key);

template < typename Key, typename Predicate >
inline std::shared_ptr< const PatriciaTree< Key > > filter(
    const std::shared_ptr< const PatriciaTree< Key > >& tree,
    const Predicate& pred);

template < typename Key >
inline std::shared_ptr< const PatriciaTree< Key > > join(
    const std::shared_ptr< const PatriciaTree< Key > >& s,
    const std::shared_ptr< const PatriciaTree< Key > >& t);

template < typename Key >
inline std::shared_ptr< const PatriciaTree< Key > > intersect(
    const std::shared_ptr< const PatriciaTree< Key > >& s,
    const std::shared_ptr< const PatriciaTree< Key > >& t);

template < typename Key >
inline std::shared_ptr< const PatriciaTree< Key > > difference(
    const std::shared_ptr< const PatriciaTree< Key > >& s,
    const std::shared_ptr< const PatriciaTree< Key > >& t);

} // end namespace patricia_tree_set_impl

/// \brief An implementation of the patricia tree set data structure
///
/// Requirements:
///
/// Key must implement IndexableTraits
/// Key must implement bool Key::operator==(const Key&) const
template < typename Key >
class PatriciaTreeSet final {
public:
  static_assert(IsIndexable< Key >::value,
                "Key must implement IndexableTraits");

private:
  using PatriciaTree = patricia_tree_set_impl::PatriciaTree< Key >;

public:
  using Iterator = patricia_tree_set_impl::PatriciaTreeIterator< Key >;

private:
  std::shared_ptr< const PatriciaTree > _tree;

private:
  /// \brief Private constructor
  explicit PatriciaTreeSet(std::shared_ptr< const PatriciaTree > tree)
      : _tree(std::move(tree)) {}

public:
  /// \brief Create an empty patricia tree set
  PatriciaTreeSet() = default;

  /// \brief Create a patricia tree set with the given elements
  PatriciaTreeSet(std::initializer_list< Key > elements) {
    for (const Key& e : elements) {
      this->insert(e);
    }
  }

  /// \brief Create a patricia tree set with the content of the range [firt,
  /// last)
  template < typename InputIterator >
  PatriciaTreeSet(InputIterator first, InputIterator last) {
    for (auto it = first; it != last; ++it) {
      this->insert(*it);
    }
  }

  /// \brief Copy constructor
  PatriciaTreeSet(const PatriciaTreeSet&) noexcept = default;

  /// \brief Move constructor
  PatriciaTreeSet(PatriciaTreeSet&&) noexcept = default;

  /// \brief Copy assignment operator
  PatriciaTreeSet& operator=(const PatriciaTreeSet&) noexcept = default;

  /// \brief Move assignment operator
  PatriciaTreeSet& operator=(PatriciaTreeSet&&) noexcept = default;

  /// \brief Destructor
  ~PatriciaTreeSet() = default;

  /// \brief Return true if the set is empty
  bool empty() const { return patricia_tree_set_impl::empty(this->_tree); }

  /// \brief Return the number of elements in the set
  std::size_t size() const { return patricia_tree_set_impl::size(this->_tree); }

  /// \brief Clear the content of the set
  void clear() { this->_tree.reset(); }

  /// \brief Return true if the set contains the given key
  bool contains(const Key& key) const {
    return patricia_tree_set_impl::contains(this->_tree, key);
  }

  /// \brief Return true if the set is a subset of `other`
  bool is_subset_of(const PatriciaTreeSet& other) const {
    return patricia_tree_set_impl::is_subset_of(this->_tree, other._tree);
  }

  /// \brief Return true if the sets are equal
  bool equals(const PatriciaTreeSet& other) const {
    return patricia_tree_set_impl::equals(this->_tree, other._tree);
  }

  /// \brief Return true if the sets are equal
  bool operator==(const PatriciaTreeSet& other) const {
    return patricia_tree_set_impl::equals(this->_tree, other._tree);
  }

  /// \brief Return the begin iterator over the elements of the set
  Iterator begin() const { return Iterator(this->_tree); }

  /// \brief Return the end iterator over the elements of the set
  Iterator end() const { return Iterator(); }

  /// \brief Insert an element in the set
  void insert(const Key& key) {
    this->_tree = patricia_tree_set_impl::insert(this->_tree, key);
  }

  /// \brief Remove an element from the set
  void erase(const Key& key) {
    this->_tree = patricia_tree_set_impl::erase(this->_tree, key);
  }

  /// \brief Remove the elements for which predicate(e) returns false
  template < typename Predicate >
  void filter(const Predicate& pred) {
    this->_tree = patricia_tree_set_impl::filter(this->_tree, pred);
  }

  /// \brief Perform the union of two patricia tree sets
  void join_with(const PatriciaTreeSet& other) {
    this->_tree = patricia_tree_set_impl::join(this->_tree, other._tree);
  }

  /// \brief Perform the union of two patricia tree sets
  PatriciaTreeSet join(const PatriciaTreeSet& other) const {
    return PatriciaTreeSet(
        patricia_tree_set_impl::join(this->_tree, other._tree));
  }

  /// \brief Perform the intersection of two patricia tree sets
  void intersect_with(const PatriciaTreeSet& other) {
    this->_tree = patricia_tree_set_impl::intersect(this->_tree, other._tree);
  }

  /// \brief Perform the intersection of two patricia tree sets
  PatriciaTreeSet intersect(const PatriciaTreeSet& other) const {
    return PatriciaTreeSet(
        patricia_tree_set_impl::intersect(this->_tree, other._tree));
  }

  /// \brief Perform the difference of two patricia tree sets
  void difference_with(const PatriciaTreeSet& other) {
    this->_tree = patricia_tree_set_impl::difference(this->_tree, other._tree);
  }

  /// \brief Perform the difference of two patricia tree sets
  PatriciaTreeSet difference(const PatriciaTreeSet& other) const {
    return PatriciaTreeSet(
        patricia_tree_set_impl::difference(this->_tree, other._tree));
  }

  /// \brief Dump the set, for debugging purpose
  void dump(std::ostream& o) const {
    static_assert(IsDumpable< Key >::value,
                  "Key must implement DumpableTraits");
    o << "{";
    for (auto it = this->begin(), et = this->end(); it != et;) {
      DumpableTraits< Key >::dump(o, *it);
      ++it;
      if (it != et) {
        o << "; ";
      }
    }
    o << "}";
  }

}; // end class PatriciaTreeSet

/// \brief Write a patricia tree set on a stream
template < typename Key >
inline std::ostream& operator<<(std::ostream& o,
                                const PatriciaTreeSet< Key >& tree) {
  tree.dump(o);
  return o;
}

namespace patricia_tree_set_impl {

template < typename Key >
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

template < typename Key >
class PatriciaTreeNode final : public PatriciaTree< Key > {
private:
  Index _prefix;
  Index _branching_bit;
  std::shared_ptr< const PatriciaTree< Key > > _left_tree;
  std::shared_ptr< const PatriciaTree< Key > > _right_tree;

public:
  PatriciaTreeNode(Index prefix,
                   Index branching_bit,
                   std::shared_ptr< const PatriciaTree< Key > > left_tree,
                   std::shared_ptr< const PatriciaTree< Key > > right_tree)
      : PatriciaTree< Key >(left_tree->size() + right_tree->size()),
        _prefix(prefix),
        _branching_bit(branching_bit),
        _left_tree(std::move(left_tree)),
        _right_tree(std::move(right_tree)) {}

  Index prefix() const { return this->_prefix; }

  Index branching_bit() const { return this->_branching_bit; }

  const std::shared_ptr< const PatriciaTree< Key > >& left_tree() const {
    return this->_left_tree;
  }

  const std::shared_ptr< const PatriciaTree< Key > >& right_tree() const {
    return this->_right_tree;
  }

}; // end class PatriciaTreeNode

template < typename Key >
class PatriciaTreeLeaf final : public PatriciaTree< Key > {
private:
  Key _key;

public:
  explicit PatriciaTreeLeaf(Key key)
      : PatriciaTree< Key >(1), _key(std::move(key)) {}

  const Key& key() const { return this->_key; }

}; // end class PatriciaTreeLeaf

template < typename Key >
inline bool empty(const std::shared_ptr< const PatriciaTree< Key > >& tree) {
  return tree == nullptr;
}

template < typename Key >
inline std::size_t size(
    const std::shared_ptr< const PatriciaTree< Key > >& tree) {
  if (tree != nullptr) {
    return tree->size();
  } else {
    return 0;
  }
}

template < typename Key >
inline bool contains(const std::shared_ptr< const PatriciaTree< Key > >& tree,
                     const Key& key) {
  if (tree == nullptr) {
    return false;
  }
  if (tree->is_leaf()) {
    auto leaf = std::static_pointer_cast< const PatriciaTreeLeaf< Key > >(tree);
    return leaf->key() == key;
  }
  auto node = std::static_pointer_cast< const PatriciaTreeNode< Key > >(tree);
  if (is_zero_bit(IndexableTraits< Key >::index(key), node->branching_bit())) {
    return contains(node->left_tree(), key);
  } else {
    return contains(node->right_tree(), key);
  }
}

template < typename Key >
inline bool is_subset_of(
    const std::shared_ptr< const PatriciaTree< Key > >& s,
    const std::shared_ptr< const PatriciaTree< Key > >& t) {
  if (s == t) {
    return true;
  }
  if (s == nullptr) {
    return true;
  }
  if (t == nullptr) {
    return false;
  }
  if (s->is_leaf()) {
    auto s_leaf = std::static_pointer_cast< const PatriciaTreeLeaf< Key > >(s);
    return contains(t, s_leaf->key());
  }
  if (t->is_leaf()) {
    return false;
  }
  auto s_node = std::static_pointer_cast< const PatriciaTreeNode< Key > >(s);
  auto t_node = std::static_pointer_cast< const PatriciaTreeNode< Key > >(t);
  if (s_node->size() > t_node->size()) {
    return false;
  }
  Index m = s_node->branching_bit();
  Index n = t_node->branching_bit();
  Index p = s_node->prefix();
  Index q = t_node->prefix();
  if (m == n && p == q) {
    return is_subset_of(s_node->left_tree(), t_node->left_tree()) &&
           is_subset_of(s_node->right_tree(), t_node->right_tree());
  }
  if (m > n && match_prefix(p, q, n)) {
    if (is_zero_bit(p, n)) {
      return is_subset_of(s_node->left_tree(), t_node->left_tree()) &&
             is_subset_of(s_node->right_tree(), t_node->left_tree());
    } else {
      return is_subset_of(s_node->left_tree(), t_node->right_tree()) &&
             is_subset_of(s_node->right_tree(), t_node->right_tree());
    }
  }
  return false; // s contains bindings that are not in t
}

template < typename Key >
inline bool equals(const std::shared_ptr< const PatriciaTree< Key > >& s,
                   const std::shared_ptr< const PatriciaTree< Key > >& t) {
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
    auto s_leaf = std::static_pointer_cast< const PatriciaTreeLeaf< Key > >(s);
    auto t_leaf = std::static_pointer_cast< const PatriciaTreeLeaf< Key > >(t);
    return s_leaf->key() == t_leaf->key();
  }
  if (t->is_leaf()) {
    return false;
  }
  auto s_node = std::static_pointer_cast< const PatriciaTreeNode< Key > >(s);
  auto t_node = std::static_pointer_cast< const PatriciaTreeNode< Key > >(t);
  return s_node->size() == t_node->size() &&
         s_node->prefix() == t_node->prefix() &&
         s_node->branching_bit() == t_node->branching_bit() &&
         equals(s_node->left_tree(), t_node->left_tree()) &&
         equals(s_node->right_tree(), t_node->right_tree());
}

/// \brief Create a node
///
/// Prevent the creation of a node with only one child.
template < typename Key >
inline std::shared_ptr< const PatriciaTree< Key > > make_node(
    Index prefix,
    Index branching_bit,
    const std::shared_ptr< const PatriciaTree< Key > >& left_tree,
    const std::shared_ptr< const PatriciaTree< Key > >& right_tree) {
  if (left_tree == nullptr) {
    return right_tree;
  }
  if (right_tree == nullptr) {
    return left_tree;
  }
  return std::make_shared< const PatriciaTreeNode< Key > >(prefix,
                                                           branching_bit,
                                                           left_tree,
                                                           right_tree);
}

/// \brief Join non-null patricia trees
template < typename Key >
inline std::shared_ptr< const PatriciaTreeNode< Key > > join_trees(
    Index prefix_s,
    const std::shared_ptr< const PatriciaTree< Key > >& s,
    Index prefix_t,
    const std::shared_ptr< const PatriciaTree< Key > >& t) {
  ikos_assert(s != nullptr && t != nullptr);

  Index m = branching_bit(prefix_s, prefix_t);

  if (is_zero_bit(prefix_s, m)) {
    return std::make_shared< const PatriciaTreeNode< Key > >(mask(prefix_s, m),
                                                             m,
                                                             s,
                                                             t);
  } else {
    return std::make_shared< const PatriciaTreeNode< Key > >(mask(prefix_s, m),
                                                             m,
                                                             t,
                                                             s);
  }
}

template < typename Key >
inline std::shared_ptr< const PatriciaTree< Key > > insert(
    const std::shared_ptr< const PatriciaTree< Key > >& tree, const Key& key) {
  if (tree == nullptr) {
    return std::make_shared< const PatriciaTreeLeaf< Key > >(key);
  }
  if (tree->is_leaf()) {
    auto leaf = std::static_pointer_cast< const PatriciaTreeLeaf< Key > >(tree);
    if (leaf->key() == key) {
      return tree;
    }
    auto new_leaf = std::make_shared< const PatriciaTreeLeaf< Key > >(key);
    return join_trees< Key >(IndexableTraits< Key >::index(key),
                             new_leaf,
                             IndexableTraits< Key >::index(leaf->key()),
                             leaf);
  }
  auto node = std::static_pointer_cast< const PatriciaTreeNode< Key > >(tree);
  if (match_prefix(IndexableTraits< Key >::index(key),
                   node->prefix(),
                   node->branching_bit())) {
    if (is_zero_bit(IndexableTraits< Key >::index(key),
                    node->branching_bit())) {
      auto new_left_tree = insert(node->left_tree(), key);
      if (new_left_tree == node->left_tree()) {
        return std::move(node);
      }
      return make_node(node->prefix(),
                       node->branching_bit(),
                       new_left_tree,
                       node->right_tree());
    } else {
      auto new_right_tree = insert(node->right_tree(), key);
      if (new_right_tree == node->right_tree()) {
        return std::move(node);
      }
      return make_node(node->prefix(),
                       node->branching_bit(),
                       node->left_tree(),
                       new_right_tree);
    }
  }
  auto new_leaf = std::make_shared< const PatriciaTreeLeaf< Key > >(key);
  return join_trees< Key >(IndexableTraits< Key >::index(key),
                           new_leaf,
                           node->prefix(),
                           node);
}

/// \brief Insert the leaf `t_leaf` into the patricia tree `s`
template < typename Key >
inline std::shared_ptr< const PatriciaTree< Key > > insert_leaf(
    const std::shared_ptr< const PatriciaTree< Key > >& s,
    const std::shared_ptr< const PatriciaTreeLeaf< Key > >& t_leaf) {
  if (s == t_leaf) {
    return s;
  }
  if (s == nullptr) {
    return t_leaf;
  }
  if (s->is_leaf()) {
    auto s_leaf = std::static_pointer_cast< const PatriciaTreeLeaf< Key > >(s);
    if (s_leaf->key() == t_leaf->key()) {
      return std::move(s_leaf);
    }
    return join_trees< Key >(IndexableTraits< Key >::index(s_leaf->key()),
                             s_leaf,
                             IndexableTraits< Key >::index(t_leaf->key()),
                             t_leaf);
  }
  auto s_node = std::static_pointer_cast< const PatriciaTreeNode< Key > >(s);
  if (match_prefix(IndexableTraits< Key >::index(t_leaf->key()),
                   s_node->prefix(),
                   s_node->branching_bit())) {
    if (is_zero_bit(IndexableTraits< Key >::index(t_leaf->key()),
                    s_node->branching_bit())) {
      auto new_left_tree = insert_leaf(s_node->left_tree(), t_leaf);
      if (new_left_tree == s_node->left_tree()) {
        return std::move(s_node);
      }
      return make_node(s_node->prefix(),
                       s_node->branching_bit(),
                       new_left_tree,
                       s_node->right_tree());
    } else {
      auto new_right_tree = insert_leaf(s_node->right_tree(), t_leaf);
      if (new_right_tree == s_node->right_tree()) {
        return std::move(s_node);
      }
      return make_node(s_node->prefix(),
                       s_node->branching_bit(),
                       s_node->left_tree(),
                       new_right_tree);
    }
  }
  return join_trees< Key >(s_node->prefix(),
                           s_node,
                           IndexableTraits< Key >::index(t_leaf->key()),
                           t_leaf);
}

template < typename Key >
inline std::shared_ptr< const PatriciaTree< Key > > erase(
    const std::shared_ptr< const PatriciaTree< Key > >& tree, const Key& key) {
  if (tree == nullptr) {
    return nullptr;
  }
  if (tree->is_leaf()) {
    auto leaf = std::static_pointer_cast< const PatriciaTreeLeaf< Key > >(tree);
    if (leaf->key() == key) {
      return nullptr;
    } else {
      return tree;
    }
  }
  auto node = std::static_pointer_cast< const PatriciaTreeNode< Key > >(tree);
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

template < typename Key, typename Predicate >
inline std::shared_ptr< const PatriciaTree< Key > > filter(
    const std::shared_ptr< const PatriciaTree< Key > >& tree,
    const Predicate& pred) {
  if (tree == nullptr) {
    return nullptr;
  }
  if (tree->is_leaf()) {
    auto leaf = std::static_pointer_cast< const PatriciaTreeLeaf< Key > >(tree);
    if (pred(leaf->key())) {
      return tree;
    } else {
      return nullptr;
    }
  }
  auto node = std::static_pointer_cast< const PatriciaTreeNode< Key > >(tree);
  auto new_left_tree = filter(node->left_tree(), pred);
  auto new_right_tree = filter(node->right_tree(), pred);
  if (new_left_tree == node->left_tree() &&
      new_right_tree == node->right_tree()) {
    return tree;
  } else {
    return make_node(node->prefix(),
                     node->branching_bit(),
                     new_left_tree,
                     new_right_tree);
  }
}

template < typename Key >
inline std::shared_ptr< const PatriciaTree< Key > > join(
    const std::shared_ptr< const PatriciaTree< Key > >& s,
    const std::shared_ptr< const PatriciaTree< Key > >& t) {
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
    auto s_leaf = std::static_pointer_cast< const PatriciaTreeLeaf< Key > >(s);
    return insert_leaf(t, s_leaf);
  }
  if (t->is_leaf()) {
    auto t_leaf = std::static_pointer_cast< const PatriciaTreeLeaf< Key > >(t);
    return insert_leaf(s, t_leaf);
  }
  auto s_node = std::static_pointer_cast< const PatriciaTreeNode< Key > >(s);
  auto t_node = std::static_pointer_cast< const PatriciaTreeNode< Key > >(t);
  Index m = s_node->branching_bit();
  Index n = t_node->branching_bit();
  Index p = s_node->prefix();
  Index q = t_node->prefix();
  if (m == n && p == q) {
    // The two trees have the same prefix
    auto new_left = join(s_node->left_tree(), t_node->left_tree());
    auto new_right = join(s_node->right_tree(), t_node->right_tree());
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
      auto new_left = join(s_node->left_tree(), t);
      if (s_node->left_tree() == new_left) {
        return s;
      }
      return make_node(p, m, new_left, s_node->right_tree());
    } else {
      auto new_right = join(s_node->right_tree(), t);
      if (s_node->right_tree() == new_right) {
        return s;
      }
      return make_node(p, m, s_node->left_tree(), new_right);
    }
  }
  if (m > n && match_prefix(p, q, n)) {
    // p contains q. Merge s with a subtree of t.
    if (is_zero_bit(p, n)) {
      auto new_left = join(s, t_node->left_tree());
      if (t_node->left_tree() == new_left) {
        return t;
      }
      return make_node(q, n, new_left, t_node->right_tree());
    } else {
      auto new_right = join(s, t_node->right_tree());
      if (t_node->right_tree() == new_right) {
        return t;
      }
      return make_node(q, n, t_node->left_tree(), new_right);
    }
  }
  // The prefixes disagree
  return join_trees(p, s, q, t);
}

template < typename Key >
inline std::shared_ptr< const PatriciaTree< Key > > intersect(
    const std::shared_ptr< const PatriciaTree< Key > >& s,
    const std::shared_ptr< const PatriciaTree< Key > >& t) {
  if (s == t) {
    return s;
  }
  if (s == nullptr || t == nullptr) {
    return nullptr;
  }
  if (s->is_leaf()) {
    auto s_leaf = std::static_pointer_cast< const PatriciaTreeLeaf< Key > >(s);
    if (contains(t, s_leaf->key())) {
      return s;
    } else {
      return nullptr;
    }
  }
  if (t->is_leaf()) {
    auto t_leaf = std::static_pointer_cast< const PatriciaTreeLeaf< Key > >(t);
    if (contains(s, t_leaf->key())) {
      return t;
    } else {
      return nullptr;
    }
  }
  auto s_node = std::static_pointer_cast< const PatriciaTreeNode< Key > >(s);
  auto t_node = std::static_pointer_cast< const PatriciaTreeNode< Key > >(t);
  Index m = s_node->branching_bit();
  Index n = t_node->branching_bit();
  Index p = s_node->prefix();
  Index q = t_node->prefix();
  if (m == n && p == q) {
    // The two trees have the same prefix
    auto new_left = intersect(s_node->left_tree(), t_node->left_tree());
    auto new_right = intersect(s_node->right_tree(), t_node->right_tree());
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
      return intersect(s_node->left_tree(), t);
    } else {
      return intersect(s_node->right_tree(), t);
    }
  }
  if (m > n && match_prefix(p, q, n)) {
    // p contains q, intersect s with a subtree of t
    if (is_zero_bit(p, n)) {
      return intersect(s, t_node->left_tree());
    } else {
      return intersect(s, t_node->right_tree());
    }
  }
  // The prefixes disagree
  return nullptr;
}

template < typename Key >
inline std::shared_ptr< const PatriciaTree< Key > > difference(
    const std::shared_ptr< const PatriciaTree< Key > >& s,
    const std::shared_ptr< const PatriciaTree< Key > >& t) {
  if (s == t) {
    return nullptr;
  }
  if (s == nullptr) {
    return nullptr;
  }
  if (t == nullptr) {
    return s;
  }
  if (s->is_leaf()) {
    auto s_leaf = std::static_pointer_cast< const PatriciaTreeLeaf< Key > >(s);
    if (contains(t, s_leaf->key())) {
      return nullptr;
    } else {
      return s;
    }
  }
  if (t->is_leaf()) {
    auto t_leaf = std::static_pointer_cast< const PatriciaTreeLeaf< Key > >(t);
    return erase(s, t_leaf->key());
  }
  auto s_node = std::static_pointer_cast< const PatriciaTreeNode< Key > >(s);
  auto t_node = std::static_pointer_cast< const PatriciaTreeNode< Key > >(t);
  Index m = s_node->branching_bit();
  Index n = t_node->branching_bit();
  Index p = s_node->prefix();
  Index q = t_node->prefix();
  if (m == n && p == q) {
    // The two trees have the same prefix
    auto new_left = difference(s_node->left_tree(), t_node->left_tree());
    auto new_right = difference(s_node->right_tree(), t_node->right_tree());
    if (new_left == s_node->left_tree() && new_right == s_node->right_tree()) {
      return s;
    }
    if (new_left == t_node->left_tree() && new_right == t_node->right_tree()) {
      return t;
    }
    return make_node(p, m, new_left, new_right);
  }
  if (m < n && match_prefix(q, p, m)) {
    // q contains p, diff t with a subtree of s
    if (is_zero_bit(q, m)) {
      auto new_left = difference(s_node->left_tree(), t);
      if (s_node->left_tree() == new_left) {
        return s;
      }
      return make_node(p, m, new_left, s_node->right_tree());
    } else {
      auto new_right = difference(s_node->right_tree(), t);
      if (s_node->right_tree() == new_right) {
        return s;
      }
      return make_node(p, m, s_node->left_tree(), new_right);
    }
  }
  if (m > n && match_prefix(p, q, n)) {
    // p contains q, diff s with a subtree of t
    if (is_zero_bit(p, n)) {
      return difference(s, t_node->left_tree());
    } else {
      return difference(s, t_node->right_tree());
    }
  }
  // The prefixes disagree
  return s;
}

template < typename Key >
class PatriciaTreeIterator final {
public:
  // Required types for iterators
  using iterator_category = std::forward_iterator_tag;
  using value_type = const Key&;
  using difference_type = std::ptrdiff_t;
  using pointer = const Key*;
  using reference = const Key&;

private:
  std::shared_ptr< const PatriciaTreeLeaf< Key > > _leaf;
  std::stack< std::shared_ptr< const PatriciaTreeNode< Key > > > _stack;

public:
  /// \brief Create an end iterator
  PatriciaTreeIterator() = default;

  /// \brief Create an iterator on the given patricia tree
  explicit PatriciaTreeIterator(
      const std::shared_ptr< const PatriciaTree< Key > >& tree) {
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
  reference operator*() const { return this->_leaf->key(); }

  /// \brief Dereference the iterator
  pointer operator->() const { return &this->_leaf->key(); }

private:
  /// \brief Find the leftmost leaf, store all intermediate nodes
  void look_for_next_leaf(
      const std::shared_ptr< const PatriciaTree< Key > >& tree) {
    auto t = tree;
    ikos_assert(t != nullptr);
    while (t->is_node()) {
      auto node = std::static_pointer_cast< const PatriciaTreeNode< Key > >(t);
      this->_stack.push(node);
      t = node->left_tree();
      ikos_assert(t != nullptr); // a node always has two children
    }
    this->_leaf = std::static_pointer_cast< const PatriciaTreeLeaf< Key > >(t);
  }

}; // end class PatriciaTreeIterator

} // end namespace patricia_tree_set_impl
} // end namespace core
} // end namespace ikos
