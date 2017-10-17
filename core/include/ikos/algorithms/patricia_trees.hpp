/**************************************************************************/ /**
 *
 * \file
 * \brief Implementation of patricia trees
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

#ifndef IKOS_PATRICIA_TREES_HPP
#define IKOS_PATRICIA_TREES_HPP

#include <algorithm>
#include <deque>
#include <iostream>
#include <memory>
#include <utility>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/optional.hpp>

#include <ikos/common/types.hpp>

namespace ikos {

template < typename Value >
class partial_order {
public:
  virtual bool leq(Value, Value) = 0;

  virtual bool default_is_top() = 0; // True if the default value is the top
                                     // element for the partial order (false if
                                     // it is bottom)

  virtual ~partial_order() {}

}; // end class partial_order

template < typename Value >
class generic_partial_order {
public:
  virtual bool leq(Value, Value) = 0;

  virtual bool leq_left(Value) = 0; // True if Value <= default value

  virtual bool leq_right(Value) = 0; // True if default value <= Value

  virtual ~generic_partial_order() {}

}; // end class generic_partial_order

template < typename Value >
class unary_op {
public:
  virtual boost::optional< Value > apply(Value) = 0;

  virtual ~unary_op() {}

}; // end class unary_op

template < typename Value >
class binary_op {
public:
  virtual boost::optional< Value > apply(
      Value, Value) = 0; // The operation is idempotent: apply(x, x) = x

  virtual bool default_is_absorbing() = 0; // True if the default value is
                                           // absorbing (false if it is neutral)

  virtual ~binary_op() {}

}; // end class binary_op

template < typename Value >
class generic_binary_op {
public:
  virtual boost::optional< Value > apply(Value, Value) = 0;

  virtual boost::optional< Value > apply_left(Value) = 0;

  virtual boost::optional< Value > apply_right(Value) = 0;

  virtual ~generic_binary_op() {}

}; // end class generic_binary_op

namespace patricia_trees_impl {

inline index64_t highest_bit(index64_t x, index64_t m) {
  index64_t x_ = x & ~(m - 1);
  index64_t m_ = m;

  while (x_ != m_) {
    x_ = x_ & ~m_;
    m_ = 2 * m_;
  }
  return m_;
}

inline index64_t compute_branching_bit(index64_t p0,
                                       index64_t m0,
                                       index64_t p1,
                                       index64_t m1) {
  return highest_bit(p0 ^ p1, std::max((index64_t)1, 2 * std::max(m0, m1)));
}

inline index64_t mask(index64_t k, index64_t m) {
  return (k | (m - 1)) & ~m;
}

inline bool zero_bit(index64_t k, index64_t m) {
  return (k & m) == 0;
}

inline bool match_prefix(index64_t k, index64_t p, index64_t m) {
  return mask(k, m) == p;
}

class failed {};

template < typename Key, typename Value >
class tree {
public:
  typedef tree< Key, Value > tree_t;
  typedef std::shared_ptr< tree_t > tree_ptr;
  typedef tree_ptr ptr;
  typedef unary_op< Value > unary_op_t;
  typedef binary_op< Value > binary_op_t;
  typedef generic_binary_op< Value > generic_binary_op_t;
  typedef partial_order< Value > partial_order_t;
  typedef generic_partial_order< Value > generic_partial_order_t;
  struct binding_t {
    const Key& first;
    const Value& second;
    binding_t(const Key& first_, const Value& second_)
        : first(first_), second(second_) {}
  };

public:
  static tree_ptr make_node(index64_t, index64_t, tree_ptr, tree_ptr);
  static tree_ptr make_leaf(const Key&, const Value&);
  static tree_ptr merge(tree_ptr, tree_ptr, binary_op_t&, bool);
  static tree_ptr generic_merge(tree_ptr, tree_ptr, generic_binary_op_t&, bool);
  static tree_ptr join(tree_ptr t0, tree_ptr t1);
  static tree_ptr insert(
      tree_ptr, const Key&, const Value&, binary_op_t&, bool);
  static tree_ptr transform(tree_ptr, unary_op_t&);
  static tree_ptr remove(tree_ptr, const Key&);
  static void compare(tree_ptr, tree_ptr, partial_order_t&, bool);
  static void generic_compare(tree_ptr,
                              tree_ptr,
                              generic_partial_order_t&,
                              bool);

public:
  virtual std::size_t size() const = 0;
  virtual bool is_leaf() const = 0;
  virtual binding_t binding() const = 0;
  virtual tree_ptr left_branch() const = 0;
  virtual tree_ptr right_branch() const = 0;
  virtual index64_t prefix() const = 0;
  virtual index64_t branching_bit() const = 0;
  virtual boost::optional< Value > lookup(const Key&) const = 0;

public:
  bool is_node() const { return !is_leaf(); }

  virtual ~tree() {}

public:
  class iterator : public boost::iterator_facade< iterator,
                                                  binding_t,
                                                  boost::forward_traversal_tag,
                                                  binding_t > {
    friend class boost::iterator_core_access;

  private:
    typedef std::pair< tree_ptr, int > branching_t;
    typedef std::deque< branching_t > branching_stack_t;

  private:
    tree_ptr _current;
    branching_stack_t _stack;

  public:
    iterator() {}

    iterator(tree_ptr t) { this->look_for_next_leaf(t); }

  private:
    void look_for_next_leaf(tree_ptr t) {
      if (t) {
        if (t->is_leaf()) {
          this->_current = t;
        } else {
          branching_t b(t, 0);
          this->_stack.push_back(b);
          this->look_for_next_leaf(t->left_branch());
        }
      } else {
        if (!this->_stack.empty()) {
          branching_t up;
          do {
            up = this->_stack.back();
            this->_stack.pop_back();
          } while (!this->_stack.empty() && up.second == 1);
          if (!(this->_stack.empty() && up.second == 1)) {
            branching_t b(up.first, 1);
            this->_stack.push_back(b);
            this->look_for_next_leaf(up.first->right_branch());
          }
        }
      }
    }

    void increment() {
      if (this->_current) {
        this->_current.reset();
        if (!this->_stack.empty()) {
          branching_t up;
          do {
            up = this->_stack.back();
            this->_stack.pop_back();
          } while (!this->_stack.empty() && up.second == 1);
          if (!(this->_stack.empty() && up.second == 1)) {
            this->look_for_next_leaf(up.first->right_branch());
          }
        }
      } else {
        throw logic_error(
            "patricia tree: trying to increment an empty iterator");
      }
    }

    bool equal(const iterator& it) const {
      if (this->_current != it._current) {
        return false;
      }
      if (this->_stack.size() != it._stack.size()) {
        return false;
      }
      typename branching_stack_t::const_iterator s_it1, s_it2;
      s_it1 = this->_stack.begin();
      s_it2 = it._stack.begin();
      while (s_it1 != this->_stack.end()) {
        if (*s_it1 != *s_it2) {
          return false;
        }
        ++s_it1;
        ++s_it2;
      }
      return true;
    }

    binding_t dereference() const {
      if (this->_current) {
        return this->_current->binding();
      } else {
        throw logic_error(
            "patricia tree: trying to dereference an empty iterator");
      }
    }

  }; // end class iterator

}; // end class tree

template < typename Key, typename Value >
class node : public tree< Key, Value > {
private:
  typedef typename tree< Key, Value >::ptr tree_ptr;
  typedef typename tree< Key, Value >::binding_t binding_t;
  typedef node< Key, Value > node_t;

private:
  std::size_t _size;
  index64_t _prefix;
  index64_t _branching_bit;
  tree_ptr _left_branch;
  tree_ptr _right_branch;

private:
  node();
  node(const node_t&);
  node_t& operator=(const node_t&);

public:
  node(index64_t prefix_,
       index64_t branching_bit_,
       tree_ptr left_branch_,
       tree_ptr right_branch_)
      : _prefix(prefix_),
        _branching_bit(branching_bit_),
        _left_branch(left_branch_),
        _right_branch(right_branch_) {
    this->_size = 0;
    if (left_branch_) {
      this->_size += left_branch_->size();
    }
    if (right_branch_) {
      this->_size += right_branch_->size();
    }
  }

  std::size_t size() const { return this->_size; }

  index64_t prefix() const { return this->_prefix; }

  index64_t branching_bit() const { return this->_branching_bit; }

  bool is_leaf() const { return false; }

  binding_t binding() const {
    throw logic_error("patricia tree: trying to call binding() on a node");
  }

  tree_ptr left_branch() const { return this->_left_branch; }

  tree_ptr right_branch() const { return this->_right_branch; }

  boost::optional< Value > lookup(const Key& key) const {
    if (index_traits< Key >::index(key) <= this->_prefix) {
      if (this->_left_branch) {
        return this->_left_branch->lookup(key);
      } else {
        return boost::optional< Value >();
      }
    } else {
      if (this->_right_branch) {
        return this->_right_branch->lookup(key);
      } else {
        return boost::optional< Value >();
      }
    }
  }

}; // end class node

template < typename Key, typename Value >
class leaf : public tree< Key, Value > {
private:
  typedef typename tree< Key, Value >::ptr tree_ptr;
  typedef typename tree< Key, Value >::binding_t binding_t;
  typedef leaf< Key, Value > leaf_t;

private:
  Key _key;
  Value _value;

private:
  leaf();
  leaf(const leaf_t&);
  leaf_t& operator=(const leaf_t&);

public:
  leaf(const Key& key_, const Value& value_) : _key(key_), _value(value_) {}

  std::size_t size() const { return 1; }

  index64_t prefix() const { return index_traits< Key >::index(this->_key); }

  index64_t branching_bit() const { return 0; }

  bool is_leaf() const { return true; }

  binding_t binding() const { return binding_t(this->_key, this->_value); }

  tree_ptr left_branch() const {
    throw logic_error("patricia tree: trying to call left_branch() on a leaf");
  }

  tree_ptr right_branch() const {
    throw logic_error("patricia tree: trying to call right_branch() on a leaf");
  }

  boost::optional< Value > lookup(const Key& key_) const {
    if (index_traits< Key >::index(this->_key) ==
        index_traits< Key >::index(key_)) {
      return boost::optional< Value >(this->_value);
    } else {
      return boost::optional< Value >();
    }
  }

}; // end class leaf

template < typename Key, typename Value >
typename tree< Key, Value >::ptr tree< Key, Value >::make_node(
    index64_t prefix,
    index64_t branching_bit,
    typename tree< Key, Value >::ptr left_branch,
    typename tree< Key, Value >::ptr right_branch) {
  typedef typename tree< Key, Value >::ptr tree_ptr;
  tree_ptr n;
  if (left_branch) {
    if (right_branch) {
      n = tree_ptr(new node< Key, Value >(prefix,
                                          branching_bit,
                                          left_branch,
                                          right_branch));
    } else {
      n = left_branch;
    }
  } else {
    if (right_branch) {
      n = right_branch;
    } else {
      // both branches are empty
    }
  }
  return n;
}

template < typename Key, typename Value >
typename tree< Key, Value >::ptr tree< Key, Value >::make_leaf(
    const Key& key, const Value& value) {
  typedef typename tree< Key, Value >::ptr tree_ptr;
  return tree_ptr(new leaf< Key, Value >(key, value));
}

template < typename Key, typename Value >
typename tree< Key, Value >::ptr tree< Key, Value >::join(
    typename tree< Key, Value >::ptr t0, typename tree< Key, Value >::ptr t1) {
  typedef typename tree< Key, Value >::ptr tree_ptr;
  index64_t p0 = t0->prefix();
  index64_t p1 = t1->prefix();
  index64_t m =
      compute_branching_bit(p0, t0->branching_bit(), p1, t1->branching_bit());
  tree_ptr t;

  if (zero_bit(p0, m)) {
    t = make_node(mask(p0, m), m, t0, t1);
  } else {
    t = make_node(mask(p0, m), m, t1, t0);
  }
  return t;
}

template < typename Key, typename Value >
typename tree< Key, Value >::ptr tree< Key, Value >::insert(
    typename tree< Key, Value >::ptr t,
    const Key& key_,
    const Value& value_,
    binary_op_t& op,
    bool combine_left_to_right) {
  typedef typename tree< Key, Value >::ptr tree_ptr;
  tree_ptr nil;
  if (t) {
    if (t->is_node()) {
      index64_t branching_bit = t->branching_bit();
      index64_t prefix = t->prefix();
      if (match_prefix(index_traits< Key >::index(key_),
                       prefix,
                       branching_bit)) {
        if (zero_bit(index_traits< Key >::index(key_), branching_bit)) {
          tree_ptr lb = t->left_branch();
          tree_ptr new_lb;
          if (lb) {
            new_lb = insert(lb, key_, value_, op, combine_left_to_right);
          } else {
            if (!op.default_is_absorbing()) {
              new_lb = make_leaf(key_, value_);
            }
          }
          if (new_lb == lb) {
            return t;
          } else {
            return make_node(prefix, branching_bit, new_lb, t->right_branch());
          }
        } else {
          tree_ptr rb = t->right_branch();
          tree_ptr new_rb;
          if (rb) {
            new_rb = insert(rb, key_, value_, op, combine_left_to_right);
          } else {
            if (!op.default_is_absorbing()) {
              new_rb = make_leaf(key_, value_);
            }
          }
          if (new_rb == rb) {
            return t;
          } else {
            return make_node(prefix, branching_bit, t->left_branch(), new_rb);
          }
        }
      } else {
        if (op.default_is_absorbing()) {
          return t;
        } else {
          return join(make_leaf(key_, value_), t);
        }
      }
    } else {
      binding_t b = t->binding();
      const Key& key = b.first;
      const Value& value = b.second;
      if (index_traits< Key >::index(key) == index_traits< Key >::index(key_)) {
        boost::optional< Value > new_value = combine_left_to_right
                                                 ? op.apply(value, value_)
                                                 : op.apply(value_, value);
        if (new_value) {
          if (*new_value == value) {
            return t;
          } else {
            return make_leaf(key_, *new_value);
          }
        } else {
          return nil;
        }
      } else {
        if (op.default_is_absorbing()) {
          return t;
        } else {
          return join(make_leaf(key_, value_), t);
        }
      }
    }
  } else {
    if (op.default_is_absorbing()) {
      return nil;
    } else {
      return make_leaf(key_, value_);
    }
  }
}

template < typename Key, typename Value >
typename tree< Key, Value >::ptr tree< Key, Value >::transform(
    typename tree< Key, Value >::ptr t, unary_op_t& op) {
  typedef typename tree< Key, Value >::ptr tree_ptr;
  tree_ptr nil;
  if (t) {
    if (t->is_node()) {
      index64_t branching_bit = t->branching_bit();
      index64_t prefix = t->prefix();
      tree_ptr lb = t->left_branch();
      tree_ptr rb = t->right_branch();
      tree_ptr new_lb, new_rb;
      if (lb) {
        new_lb = transform(lb, op);
      } else {
        new_lb = lb;
      }
      if (rb) {
        new_rb = transform(rb, op);
      } else {
        new_rb = rb;
      }
      if (lb == new_lb && rb == new_rb) {
        return t;
      } else {
        return make_node(prefix, branching_bit, new_lb, new_rb);
      }
    } else {
      binding_t b = t->binding();
      const Value& value = b.second;
      boost::optional< Value > new_value = op.apply(value);
      if (new_value) {
        if (*new_value == value) {
          return t;
        } else {
          return make_leaf(b.first, *new_value);
        }
      } else {
        return nil;
      }
    }
  } else {
    return t;
  }
}

template < typename Key, typename Value >
typename tree< Key, Value >::ptr tree< Key, Value >::remove(
    typename tree< Key, Value >::ptr t, const Key& key_) {
  typedef typename tree< Key, Value >::ptr tree_ptr;
  tree_ptr nil;
  index64_t id = index_traits< Key >::index(key_);
  if (t) {
    if (t->is_node()) {
      index64_t branching_bit = t->branching_bit();
      index64_t prefix = t->prefix();
      if (match_prefix(id, prefix, branching_bit)) {
        if (zero_bit(id, branching_bit)) {
          tree_ptr lb = t->left_branch();
          tree_ptr new_lb;
          if (lb) {
            new_lb = remove(lb, key_);
          }
          if (new_lb == lb) {
            return t;
          } else {
            return make_node(prefix, branching_bit, new_lb, t->right_branch());
          }
        } else {
          tree_ptr rb = t->right_branch();
          tree_ptr new_rb;
          if (rb) {
            new_rb = remove(rb, key_);
          }
          if (new_rb == rb) {
            return t;
          } else {
            return make_node(prefix, branching_bit, t->left_branch(), new_rb);
          }
        }
      } else {
        return t;
      }
    } else {
      binding_t b = t->binding();
      const Key& key = b.first;
      if (index_traits< Key >::index(key) == id) {
        return nil;
      } else {
        return t;
      }
    }
  } else {
    return nil;
  }
}

template < typename Key, typename Value >
typename tree< Key, Value >::ptr tree< Key, Value >::merge(
    typename tree< Key, Value >::ptr s,
    typename tree< Key, Value >::ptr t,
    binary_op_t& op,
    bool combine_left_to_right) {
  typedef typename tree< Key, Value >::ptr tree_ptr;
  tree_ptr nil;
  if (s) {
    if (t) {
      if (s == t) {
        return s;
      } else if (s->is_leaf()) {
        binding_t b = s->binding();
        if (op.default_is_absorbing()) {
          boost::optional< Value > value = t->lookup(b.first);
          if (value) {
            boost::optional< Value > new_value =
                combine_left_to_right ? op.apply(b.second, *value)
                                      : op.apply(*value, b.second);
            if (new_value) {
              if (*new_value == b.second) {
                return s;
              } else {
                return make_leaf(b.first, *new_value);
              }
            } else {
              return nil;
            }
          } else {
            return nil;
          }
        } else {
          return insert(t, b.first, b.second, op, !combine_left_to_right);
        }
      } else if (t->is_leaf()) {
        binding_t b = t->binding();
        if (op.default_is_absorbing()) {
          boost::optional< Value > value = s->lookup(b.first);
          if (value) {
            boost::optional< Value > new_value =
                combine_left_to_right ? op.apply(*value, b.second)
                                      : op.apply(b.second, *value);
            if (new_value) {
              if (*new_value == b.second) {
                return t;
              } else {
                return make_leaf(b.first, *new_value);
              }
            } else {
              return nil;
            }
          } else {
            return nil;
          }
        } else {
          return insert(s, b.first, b.second, op, combine_left_to_right);
        }
      } else {
        if (s->branching_bit() == t->branching_bit() &&
            s->prefix() == t->prefix()) {
          tree_ptr new_lb = merge(s->left_branch(),
                                  t->left_branch(),
                                  op,
                                  combine_left_to_right);
          tree_ptr new_rb = merge(s->right_branch(),
                                  t->right_branch(),
                                  op,
                                  combine_left_to_right);
          if (new_lb == s->left_branch() && new_rb == s->right_branch()) {
            return s;
          } else if (new_lb == t->left_branch() &&
                     new_rb == t->right_branch()) {
            return t;
          } else {
            return make_node(s->prefix(), s->branching_bit(), new_lb, new_rb);
          }
        } else if (s->branching_bit() > t->branching_bit() &&
                   match_prefix(t->prefix(), s->prefix(), s->branching_bit())) {
          if (zero_bit(t->prefix(), s->branching_bit())) {
            tree_ptr new_lb =
                merge(s->left_branch(), t, op, combine_left_to_right);
            tree_ptr new_rb =
                op.default_is_absorbing() ? nil : s->right_branch();
            if (new_lb == s->left_branch() && new_rb == s->right_branch()) {
              return s;
            } else {
              return make_node(s->prefix(), s->branching_bit(), new_lb, new_rb);
            }
          } else {
            tree_ptr new_lb =
                op.default_is_absorbing() ? nil : s->left_branch();
            tree_ptr new_rb =
                merge(s->right_branch(), t, op, combine_left_to_right);
            if (new_lb == s->left_branch() && new_rb == s->right_branch()) {
              return s;
            } else {
              return make_node(s->prefix(), s->branching_bit(), new_lb, new_rb);
            }
          }
        } else if (s->branching_bit() < t->branching_bit() &&
                   match_prefix(s->prefix(), t->prefix(), t->branching_bit())) {
          if (zero_bit(s->prefix(), t->branching_bit())) {
            tree_ptr new_lb =
                merge(s, t->left_branch(), op, combine_left_to_right);
            tree_ptr new_rb =
                op.default_is_absorbing() ? nil : t->right_branch();
            if (new_lb == t->left_branch() && new_rb == t->right_branch()) {
              return t;
            } else {
              return make_node(t->prefix(), t->branching_bit(), new_lb, new_rb);
            }
          } else {
            tree_ptr new_lb =
                op.default_is_absorbing() ? nil : t->left_branch();
            tree_ptr new_rb =
                merge(s, t->right_branch(), op, combine_left_to_right);
            if (new_lb == t->left_branch() && new_rb == t->right_branch()) {
              return t;
            } else {
              return make_node(t->prefix(), t->branching_bit(), new_lb, new_rb);
            }
          }
        } else {
          if (op.default_is_absorbing()) {
            return nil;
          } else {
            return join(s, t);
          }
        }
      }
    } else {
      if (op.default_is_absorbing()) {
        return nil;
      } else {
        return s;
      }
    }
  } else {
    if (op.default_is_absorbing()) {
      return nil;
    } else {
      return t;
    }
  }
}

template < typename Key, typename Value >
typename tree< Key, Value >::ptr tree< Key, Value >::generic_merge(
    typename tree< Key, Value >::ptr s,
    typename tree< Key, Value >::ptr t,
    generic_binary_op_t& op,
    bool combine_left_to_right) {
  typedef typename tree< Key, Value >::ptr tree_ptr;
  tree_ptr nil;
  if (s) {
    if (t) {
      if (s->is_leaf()) {
        if (t->is_leaf()) {
          if (index_traits< Key >::index(s->binding().first) ==
              index_traits< Key >::index(t->binding().first)) {
            boost::optional< Value > new_value =
                combine_left_to_right
                    ? op.apply(s->binding().second, t->binding().second)
                    : op.apply(t->binding().second, s->binding().second);
            if (new_value) {
              return make_leaf(s->binding().first, *new_value);
            } else {
              return nil;
            }
          } else {
            boost::optional< Value > s_new_value =
                combine_left_to_right ? op.apply_left(s->binding().second)
                                      : op.apply_right(s->binding().second);
            boost::optional< Value > t_new_value =
                combine_left_to_right ? op.apply_right(t->binding().second)
                                      : op.apply_left(t->binding().second);
            if (s_new_value && t_new_value) {
              return join(make_leaf(s->binding().first, *s_new_value),
                          make_leaf(t->binding().first, *t_new_value));
            } else if (s_new_value) {
              return make_leaf(s->binding().first, *s_new_value);
            } else if (t_new_value) {
              return make_leaf(t->binding().first, *t_new_value);
            } else {
              return nil;
            }
          }
        } else { // t->is_node()
          index64_t branching_bit = t->branching_bit();
          index64_t prefix = t->prefix();
          if (match_prefix(index_traits< Key >::index(s->binding().first),
                           prefix,
                           branching_bit)) {
            if (zero_bit(index_traits< Key >::index(s->binding().first),
                         branching_bit)) {
              tree_ptr new_lb =
                  generic_merge(s, t->left_branch(), op, combine_left_to_right);
              tree_ptr new_rb = generic_merge(nil,
                                              t->right_branch(),
                                              op,
                                              combine_left_to_right);
              return make_node(prefix, branching_bit, new_lb, new_rb);
            } else {
              tree_ptr new_lb = generic_merge(nil,
                                              t->left_branch(),
                                              op,
                                              combine_left_to_right);
              tree_ptr new_rb = generic_merge(s,
                                              t->right_branch(),
                                              op,
                                              combine_left_to_right);
              return make_node(prefix, branching_bit, new_lb, new_rb);
            }
          } else {
            boost::optional< Value > new_value =
                combine_left_to_right ? op.apply_left(s->binding().second)
                                      : op.apply_right(s->binding().second);
            tree_ptr new_t = generic_merge(nil, t, op, combine_left_to_right);
            if (new_value) {
              return join(make_leaf(s->binding().first, *new_value), new_t);
            } else {
              return new_t;
            }
          }
        }
      } else if (t->is_leaf()) {
        return generic_merge(t, s, op, !combine_left_to_right);
      } else { // s->is_node() && t->is_node()
        if (s->branching_bit() == t->branching_bit() &&
            s->prefix() == t->prefix()) {
          tree_ptr new_lb = generic_merge(s->left_branch(),
                                          t->left_branch(),
                                          op,
                                          combine_left_to_right);
          tree_ptr new_rb = generic_merge(s->right_branch(),
                                          t->right_branch(),
                                          op,
                                          combine_left_to_right);
          return make_node(s->prefix(), s->branching_bit(), new_lb, new_rb);
        } else if (s->branching_bit() > t->branching_bit() &&
                   match_prefix(t->prefix(), s->prefix(), s->branching_bit())) {
          if (zero_bit(t->prefix(), s->branching_bit())) {
            tree_ptr new_lb =
                generic_merge(s->left_branch(), t, op, combine_left_to_right);
            tree_ptr new_rb = generic_merge(s->right_branch(),
                                            nil,
                                            op,
                                            combine_left_to_right);
            return make_node(s->prefix(), s->branching_bit(), new_lb, new_rb);
          } else {
            tree_ptr new_lb =
                generic_merge(s->left_branch(), nil, op, combine_left_to_right);
            tree_ptr new_rb =
                generic_merge(s->right_branch(), t, op, combine_left_to_right);
            return make_node(s->prefix(), s->branching_bit(), new_lb, new_rb);
          }
        } else if (s->branching_bit() < t->branching_bit() &&
                   match_prefix(s->prefix(), t->prefix(), t->branching_bit())) {
          if (zero_bit(s->prefix(), t->branching_bit())) {
            tree_ptr new_lb =
                generic_merge(s, t->left_branch(), op, combine_left_to_right);
            tree_ptr new_rb = generic_merge(nil,
                                            t->right_branch(),
                                            op,
                                            combine_left_to_right);
            return make_node(t->prefix(), t->branching_bit(), new_lb, new_rb);
          } else {
            tree_ptr new_lb =
                generic_merge(nil, t->left_branch(), op, combine_left_to_right);
            tree_ptr new_rb =
                generic_merge(s, t->right_branch(), op, combine_left_to_right);
            return make_node(t->prefix(), t->branching_bit(), new_lb, new_rb);
          }
        } else {
          tree_ptr new_s = generic_merge(s, nil, op, combine_left_to_right);
          tree_ptr new_t = generic_merge(nil, t, op, combine_left_to_right);
          return join(new_s, new_t);
        }
      }
    } else { // t is null
      if (s->is_leaf()) {
        boost::optional< Value > new_value =
            combine_left_to_right ? op.apply_left(s->binding().second)
                                  : op.apply_right(s->binding().second);
        if (new_value) {
          return make_leaf(s->binding().first, *new_value);
        } else {
          return nil;
        }
      } else { // s->is_node()
        tree_ptr new_lb =
            generic_merge(s->left_branch(), nil, op, combine_left_to_right);
        tree_ptr new_rb =
            generic_merge(s->right_branch(), nil, op, combine_left_to_right);
        return make_node(s->prefix(), s->branching_bit(), new_lb, new_rb);
      }
    }
  } else {
    if (t) {
      return generic_merge(t, nil, op, !combine_left_to_right);
    } else {
      return nil;
    }
  }
}

template < typename Key, typename Value >
void tree< Key, Value >::compare(typename tree< Key, Value >::ptr s,
                                 typename tree< Key, Value >::ptr t,
                                 partial_order_t& po,
                                 bool compare_left_to_right) {
  if (s) {
    if (t) {
      if (s != t) {
        if (s->is_leaf()) {
          binding_t b = s->binding();
          const Key& key = b.first;
          const Value& value = b.second;
          boost::optional< Value > value_ = t->lookup(key);
          if (value_) {
            Value left = compare_left_to_right ? value : *value_;
            Value right = compare_left_to_right ? *value_ : value;
            if (!po.leq(left, right)) {
              throw failed();
            }
          } else {
            if ((compare_left_to_right && !po.default_is_top()) ||
                (!compare_left_to_right && po.default_is_top())) {
              throw failed();
            }
          }
          if (compare_left_to_right && po.default_is_top() && !t->is_leaf()) {
            throw failed();
          }
          if (!compare_left_to_right && !po.default_is_top() && !t->is_leaf()) {
            throw failed();
          }
        } else if (t->is_leaf()) {
          compare(t, s, po, !compare_left_to_right);
        } else {
          if (s->branching_bit() == t->branching_bit() &&
              s->prefix() == t->prefix()) {
            compare(s->left_branch(),
                    t->left_branch(),
                    po,
                    compare_left_to_right);
            compare(s->right_branch(),
                    t->right_branch(),
                    po,
                    compare_left_to_right);
          } else if (s->branching_bit() > t->branching_bit() &&
                     match_prefix(t->prefix(),
                                  s->prefix(),
                                  s->branching_bit())) {
            if ((compare_left_to_right && !po.default_is_top()) ||
                (!compare_left_to_right && po.default_is_top())) {
              throw failed();
            }
            if (zero_bit(t->prefix(), s->branching_bit())) {
              compare(s->left_branch(), t, po, compare_left_to_right);
            } else {
              compare(s->right_branch(), t, po, compare_left_to_right);
            }
          } else if (s->branching_bit() < t->branching_bit() &&
                     match_prefix(s->prefix(),
                                  t->prefix(),
                                  t->branching_bit())) {
            if ((compare_left_to_right && po.default_is_top()) ||
                (!compare_left_to_right && !po.default_is_top())) {
              throw failed();
            }
            if (zero_bit(s->prefix(), t->branching_bit())) {
              compare(s, t->left_branch(), po, compare_left_to_right);
            } else {
              compare(s, t->right_branch(), po, compare_left_to_right);
            }
          } else {
            throw failed();
          }
        }
      }
    } else {
      if ((compare_left_to_right && !po.default_is_top()) ||
          (!compare_left_to_right && po.default_is_top())) {
        throw failed();
      }
    }
  } else {
    if (t) {
      if ((compare_left_to_right && po.default_is_top()) ||
          (!compare_left_to_right && !po.default_is_top())) {
        throw failed();
      }
    } else {
      // s and t are empty
    }
  }
}

template < typename Key, typename Value >
void tree< Key, Value >::generic_compare(typename tree< Key, Value >::ptr s,
                                         typename tree< Key, Value >::ptr t,
                                         generic_partial_order_t& po,
                                         bool compare_left_to_right) {
  typedef typename tree< Key, Value >::ptr tree_ptr;
  tree_ptr nil;
  if (s) {
    if (t) {
      if (s != t) {
        if (s->is_leaf()) {
          if (t->is_leaf()) {
            Value left = compare_left_to_right ? s->binding().second
                                               : t->binding().second;
            Value right = compare_left_to_right ? t->binding().second
                                                : s->binding().second;
            if (index_traits< Key >::index(s->binding().first) ==
                index_traits< Key >::index(t->binding().first)) {
              if (!po.leq(left, right)) {
                throw failed();
              }
            } else {
              if (!po.leq_left(left)) {
                throw failed();
              }
              if (!po.leq_right(right)) {
                throw failed();
              }
            }
          } else { // t->is_node()
            index64_t branching_bit = t->branching_bit();
            index64_t prefix = t->prefix();
            if (match_prefix(index_traits< Key >::index(s->binding().first),
                             prefix,
                             branching_bit)) {
              if (zero_bit(index_traits< Key >::index(s->binding().first),
                           branching_bit)) {
                generic_compare(s, t->left_branch(), po, compare_left_to_right);
                generic_compare(nil,
                                t->right_branch(),
                                po,
                                compare_left_to_right);
              } else {
                generic_compare(nil,
                                t->left_branch(),
                                po,
                                compare_left_to_right);
                generic_compare(s,
                                t->right_branch(),
                                po,
                                compare_left_to_right);
              }
            } else {
              generic_compare(s, nil, po, compare_left_to_right);
              generic_compare(nil, t, po, compare_left_to_right);
            }
          }
        } else if (t->is_leaf()) {
          generic_compare(t, s, po, !compare_left_to_right);
        } else { // s->is_node() && t->is_node()
          if (s->branching_bit() == t->branching_bit() &&
              s->prefix() == t->prefix()) {
            generic_compare(s->left_branch(),
                            t->left_branch(),
                            po,
                            compare_left_to_right);
            generic_compare(s->right_branch(),
                            t->right_branch(),
                            po,
                            compare_left_to_right);
          } else if (s->branching_bit() > t->branching_bit() &&
                     match_prefix(t->prefix(),
                                  s->prefix(),
                                  s->branching_bit())) {
            if (zero_bit(t->prefix(), s->branching_bit())) {
              generic_compare(s->left_branch(), t, po, compare_left_to_right);
              generic_compare(s->right_branch(),
                              nil,
                              po,
                              compare_left_to_right);
            } else {
              generic_compare(s->left_branch(), nil, po, compare_left_to_right);
              generic_compare(s->right_branch(), t, po, compare_left_to_right);
            }
          } else if (s->branching_bit() < t->branching_bit() &&
                     match_prefix(s->prefix(),
                                  t->prefix(),
                                  t->branching_bit())) {
            if (zero_bit(s->prefix(), t->branching_bit())) {
              generic_compare(s, t->left_branch(), po, compare_left_to_right);
              generic_compare(nil,
                              t->right_branch(),
                              po,
                              compare_left_to_right);
            } else {
              generic_compare(nil, t->left_branch(), po, compare_left_to_right);
              generic_compare(s, t->right_branch(), po, compare_left_to_right);
            }
          } else {
            generic_compare(s, nil, po, compare_left_to_right);
            generic_compare(nil, t, po, compare_left_to_right);
          }
        }
      }
    } else { // t is null
      if (s->is_leaf()) {
        Value value = s->binding().second;
        if ((compare_left_to_right && !po.leq_left(value)) ||
            (!compare_left_to_right && !po.leq_right(value))) {
          throw failed();
        }
      } else { // s->is_node()
        generic_compare(s->left_branch(), nil, po, compare_left_to_right);
        generic_compare(s->right_branch(), nil, po, compare_left_to_right);
      }
    }
  } else {
    if (t) {
      generic_compare(t, nil, po, !compare_left_to_right);
    } else {
      // s and t are empty
    }
  }
}

} // end namespace patricia_trees_impl

/// \brief Fast mergeable map based on patricia trees
template < typename Key, typename Value >
class patricia_tree {
private:
  typedef patricia_trees_impl::tree< Key, Value > tree_t;
  typedef typename tree_t::ptr tree_ptr;
  typedef typename tree_t::binding_t binding_t;

public:
  typedef patricia_tree< Key, Value > patricia_tree_t;
  typedef typename tree_t::unary_op_t unary_op_t;
  typedef typename tree_t::binary_op_t binary_op_t;
  typedef typename tree_t::generic_binary_op_t generic_binary_op_t;
  typedef typename tree_t::partial_order_t partial_order_t;
  typedef typename tree_t::generic_partial_order_t generic_partial_order_t;

private:
  tree_ptr _tree;

public:
  class iterator : public boost::iterator_facade< iterator,
                                                  binding_t,
                                                  boost::forward_traversal_tag,
                                                  binding_t > {
    friend class boost::iterator_core_access;
    friend class patricia_tree< Key, Value >;

  private:
    typename tree_t::iterator _it;

  public:
    iterator() {}

    iterator(const patricia_tree_t& pt) : _it(pt._tree) {}

  private:
    iterator(tree_ptr t) : _it(t) {}

    void increment() { ++this->_it; }

    bool equal(const iterator& other) const { return this->_it == other._it; }

    binding_t dereference() const { return *this->_it; }

  }; // end class iterator

  class insert_op : public binary_op_t {
    boost::optional< Value > apply(Value /* old_value */, Value new_value) {
      return boost::optional< Value >(new_value);
    }

    bool default_is_absorbing() { return false; }

  }; // end class insert_op

public:
  patricia_tree() {}

  patricia_tree(const patricia_tree_t& t) : _tree(t._tree) {}

  patricia_tree_t& operator=(const patricia_tree_t& t) {
    this->_tree = t._tree;
    return *this;
  }

  std::size_t size() const {
    if (this->_tree) {
      return this->_tree->size();
    } else {
      return 0;
    }
  }

  iterator begin() const { return iterator(this->_tree); }

  iterator end() const { return iterator(); }

  boost::optional< Value > lookup(const Key& key) const {
    if (this->_tree) {
      return this->_tree->lookup(key);
    } else {
      return boost::optional< Value >();
    }
  }

  void merge_with(const patricia_tree_t& t, binary_op_t& op) {
    this->_tree = tree_t::merge(this->_tree, t._tree, op, true);
  }

  void generic_merge_with(const patricia_tree_t& t, generic_binary_op_t& op) {
    this->_tree = tree_t::generic_merge(this->_tree, t._tree, op, true);
  }

  void insert(const Key& key, const Value& value) {
    insert_op op;
    this->_tree = tree_t::insert(this->_tree, key, value, op, true);
  }

  void transform(unary_op_t& op) {
    this->_tree = tree_t::transform(this->_tree, op);
  }

  void remove(const Key& key) {
    this->_tree = tree_t::remove(this->_tree, key);
  }

  void clear() { this->_tree.reset(); }

  bool empty() const { return !this->_tree; }

  bool leq(const patricia_tree_t& t, partial_order_t& po) const {
    try {
      tree_t::compare(this->_tree, t._tree, po, true);
      return true;
    } catch (patricia_trees_impl::failed& e) {
      return false;
    }
  }

  bool generic_leq(const patricia_tree_t& t,
                   generic_partial_order_t& po) const {
    try {
      tree_t::generic_compare(this->_tree, t._tree, po, true);
      return true;
    } catch (patricia_trees_impl::failed& e) {
      return false;
    }
  }

}; // end class patricia_tree

/// \brief Fast mergeable set based on patricia trees
template < typename Element >
class patricia_tree_set : public writeable {
private:
  typedef patricia_tree< Element, bool > patricia_tree_t;

public:
  typedef patricia_tree_set< Element > patricia_tree_set_t;
  typedef typename patricia_tree_t::unary_op_t unary_op_t;
  typedef typename patricia_tree_t::binary_op_t binary_op_t;
  typedef typename patricia_tree_t::partial_order_t partial_order_t;

private:
  patricia_tree_t _tree;

public:
  class iterator : public boost::iterator_facade< iterator,
                                                  Element,
                                                  boost::forward_traversal_tag,
                                                  Element > {
    friend class boost::iterator_core_access;
    friend class patricia_tree_set< Element >;

  private:
    typename patricia_tree_t::iterator _it;

  public:
    iterator() {}

    iterator(const patricia_tree_set_t& ptset) : _it(ptset._tree) {}

  private:
    iterator(patricia_tree_t t) : _it(t) {}

    void increment() { ++this->_it; }

    bool equal(const iterator& other) const { return this->_it == other._it; }

    Element dereference() const { return this->_it->first; }

  }; // end class iterator

private:
  class union_op : public binary_op_t {
    boost::optional< bool > apply(bool /* x */, bool /* y */) {
      return boost::optional< bool >(true);
    };

    bool default_is_absorbing() { return false; }

  }; // end class union_op

  class intersection_op : public binary_op_t {
    boost::optional< bool > apply(bool /* x */, bool /* y */) {
      return boost::optional< bool >(true);
    };

    bool default_is_absorbing() { return true; }

  }; // end class intersection_op

  class subset_po : public partial_order_t {
    bool leq(bool /* x */, bool /* y */) { return true; };

    bool default_is_top() { return false; }

  }; // end class subset_po

private:
  static patricia_tree_t do_union(patricia_tree_t t1, patricia_tree_t t2) {
    union_op o;
    t1.merge_with(t2, o);
    return t1;
  }

  static patricia_tree_t do_intersection(patricia_tree_t t1,
                                         patricia_tree_t t2) {
    intersection_op o;
    t1.merge_with(t2, o);
    return t1;
  }

private:
  patricia_tree_set(patricia_tree_t t) : _tree(t) {}

public:
  patricia_tree_set() {}

  patricia_tree_set(const patricia_tree_set_t& s) : _tree(s._tree) {}

  patricia_tree_set_t& operator=(const patricia_tree_set_t& t) {
    this->_tree = t._tree;
    return *this;
  }

  patricia_tree_set(const Element& e) { this->_tree.insert(e, true); }

  std::size_t size() const { return this->_tree.size(); }

  iterator begin() const { return iterator(this->_tree); }

  iterator end() const { return iterator(); }

  bool operator[](const Element& x) const {
    return (bool)this->_tree.lookup(x);
  }

  patricia_tree_set_t operator+(const Element& e) const {
    patricia_tree_t t = this->_tree;
    t.insert(e, true);
    return patricia_tree_set_t(t);
  }

  patricia_tree_set_t& operator+=(const Element& e) {
    this->_tree.insert(e, true);
    return *this;
  }

  patricia_tree_set_t operator-(const Element& e) const {
    patricia_tree_t t = this->_tree;
    t.remove(e);
    return patricia_tree_set_t(t);
  }

  patricia_tree_set_t& operator-=(const Element& e) {
    this->_tree.remove(e);
    return *this;
  }

  patricia_tree_set& clear() {
    this->_tree.clear();
    return *this;
  }

  bool empty() const { return this->_tree.empty(); }

  patricia_tree_set_t operator|(const patricia_tree_set_t& s) const {
    patricia_tree_set_t u(do_union(this->_tree, s._tree));
    return u;
  }

  patricia_tree_set_t& operator|=(const patricia_tree_set_t& s) {
    this->_tree = do_union(this->_tree, s._tree);
    return *this;
  }

  patricia_tree_set_t operator&(const patricia_tree_set_t& s) const {
    patricia_tree_set_t i(do_intersection(this->_tree, s._tree));
    return i;
  }

  patricia_tree_set_t& operator&=(const patricia_tree_set_t& s) {
    this->_tree = do_intersection(this->_tree, s._tree);
    return *this;
  }

  bool operator<=(const patricia_tree_set_t& s) const {
    subset_po po;
    return this->_tree.leq(s._tree, po);
  }

  bool operator>=(const patricia_tree_set_t& s) const {
    return s.operator<=(*this);
  }

  bool operator==(const patricia_tree_set_t& s) const {
    return (this->operator<=(s) && s.operator<=(*this));
  }

  friend std::ostream& operator<<(std::ostream& o, patricia_tree_set_t& s) {
    s.write(o);
    return o;
  }

  void write(std::ostream& o) {
    o << "{";
    for (iterator it = begin(); it != end();) {
      // Temp element is defined because the write is not const, making changes
      // to temp_elem possible via write trait call.
      Element temp_elem = *it;
      index_traits< Element >::write(o, temp_elem);
      ++it;
      if (it != end()) {
        o << "; ";
      }
    }
    o << "}";
  }

}; // end class patricia_tree_set

} // end namespace ikos

#endif // IKOS_PATRICIA_TREES_HPP
