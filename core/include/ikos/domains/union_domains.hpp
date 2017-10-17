/**************************************************************************/ /**
 *
 * \file
 * \brief Generic union abstract domain.
 *
 * Author: Maxime Arthaud
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

#ifndef IKOS_UNION_DOMAINS_HPP
#define IKOS_UNION_DOMAINS_HPP

#include <functional>
#include <memory>

#include <ikos/common/types.hpp>
#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/domains/bitwise_operators_api.hpp>
#include <ikos/domains/division_operators_api.hpp>
#include <ikos/domains/numerical_domains_api.hpp>
#include <ikos/value/congruence.hpp>
#include <ikos/value/interval.hpp>

namespace ikos {

/// \brief Union abstract domain
template < typename Number,
           typename VariableName,
           typename Domain,
           std::size_t MaxHeight >
class union_domain : public abstract_domain,
                     public numerical_domain< Number, VariableName >,
                     public bitwise_operators< Number, VariableName >,
                     public division_operators< Number, VariableName > {
public:
  typedef variable< Number, VariableName > variable_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;
  typedef interval< Number > interval_t;
  typedef congruence< Number > congruence_t;
  typedef union_domain< Number, VariableName, Domain, MaxHeight >
      union_domain_t;

private:
  /*
   * Implementation of a binary tree structure
   */

  class tree;

  typedef std::shared_ptr< tree > tree_ptr;

  class tree {
  public:
    virtual ~tree() {}
    virtual std::size_t size() const = 0;
    virtual std::size_t height() const = 0;
    virtual bool is_leaf() const = 0;
    virtual const tree_ptr& left_branch() const = 0;
    virtual const tree_ptr& right_branch() const = 0;
    virtual Domain value() const = 0;
    virtual void write(std::ostream& o) = 0;

  public:
    inline bool is_node() const { return !is_leaf(); }
  }; // end class tree

  class node : public tree {
  private:
    std::size_t _size;
    std::size_t _height;
    tree_ptr _left_branch;
    tree_ptr _right_branch;

  public:
    node(const tree_ptr& left_branch, const tree_ptr& right_branch)
        : _left_branch(left_branch), _right_branch(right_branch) {
      assert(left_branch != nullptr && right_branch != nullptr);
      _size = _left_branch->size() + _right_branch->size();
      _height = 1 + std::max(_left_branch->height(), _right_branch->height());
    }

    std::size_t size() const { return _size; }
    std::size_t height() const { return _height; }
    bool is_leaf() const { return false; }
    const tree_ptr& left_branch() const { return _left_branch; }
    const tree_ptr& right_branch() const { return _right_branch; }
    Domain value() const {
      throw logic_error("union_domain: trying to call value() on a node");
    }

    void write(std::ostream& o) {
      o << "N(";
      _left_branch->write(o);
      o << ",";
      _right_branch->write(o);
      o << ")";
    }
  }; // end class node

  class leaf : public tree {
  private:
    Domain _value;

  public:
    leaf(const Domain& value) : _value(value) {}

    std::size_t size() const { return 1; }
    std::size_t height() const { return 0; }
    bool is_leaf() const { return true; }
    const tree_ptr& left_branch() const {
      throw logic_error("union_domain: trying to call left_branch() on a leaf");
    }
    const tree_ptr& right_branch() const {
      throw logic_error(
          "union_domain: trying to call right_branch() on a leaf");
    }
    Domain value() const { return _value; }

    void write(std::ostream& o) {
      o << "L(";
      _value.write(o);
      o << ")";
    }
  }; // end class leaf

  static inline tree_ptr make_node(const tree_ptr& left_branch,
                                   const tree_ptr& right_branch) {
    if (left_branch != nullptr) {
      if (right_branch != nullptr) {
        return std::make_shared< node >(left_branch, right_branch);
      } else {
        return left_branch;
      }
    } else {
      if (right_branch != nullptr) {
        return right_branch;
      } else {
        return nullptr;
      }
    }
  }

  static inline tree_ptr make_leaf(Domain value) {
    if (!value.is_bottom()) {
      return std::make_shared< leaf >(value);
    } else {
      return nullptr;
    }
  }

  typedef std::function< Domain(Domain) > unary_op;

  static tree_ptr transform(tree_ptr t, unary_op& op) {
    if (t != nullptr) {
      if (t->is_node()) {
        return make_node(transform(t->left_branch(), op),
                         transform(t->right_branch(), op));
      } else { // t->is_leaf()
        return make_leaf(op(t->value()));
      }
    } else {
      return nullptr;
    }
  }

  static bool is_top(tree_ptr t) {
    if (t != nullptr) {
      if (t->is_node()) {
        if (!is_top(t->left_branch())) {
          return false;
        }
        if (!is_top(t->right_branch())) {
          return false;
        }
        return true;
      } else { // t->is_leaf()
        Domain inv = t->value();
        return inv.is_top();
      }
    } else {
      // bottom
      return false;
    }
  }

  static interval_t to_interval(tree_ptr t, VariableName v) {
    if (t != nullptr) {
      if (t->is_node()) {
        return to_interval(t->left_branch(), v) |
               to_interval(t->right_branch(), v);
      } else { // t->is_leaf()
        Domain inv = t->value();
        return inv[v];
      }
    } else {
      // bottom
      return interval_t::bottom();
    }
  }

  static Domain merge(tree_ptr t) {
    assert(t != nullptr);

    if (t->is_node()) {
      return merge(t->left_branch()) | merge(t->right_branch());
    } else { // t->is_leaf()
      return t->value();
    }
  }

  /// \brief Reduce the height of a tree by merging the nodes at a heigh
  /// greater than max_height
  static tree_ptr reduce_height(tree_ptr t, std::size_t max_height) {
    if (t != nullptr) {
      if (t->height() > max_height) {
        if (max_height == 0) {
          return make_leaf(merge(t));
        } else {
          return make_node(reduce_height(t->left_branch(), max_height - 1),
                           reduce_height(t->right_branch(), max_height - 1));
        }
      } else {
        return t;
      }
    } else {
      return nullptr;
    }
  }

private:
  tree_ptr _tree;

private:
  union_domain(bool is_top)
      : _tree(is_top ? make_leaf(Domain::top()) : nullptr) {}

  union_domain(tree_ptr tree) : _tree(tree) {}

public:
  static union_domain_t top() { return union_domain(true); }

  static union_domain_t bottom() { return union_domain(false); }

public:
  union_domain() : _tree(make_leaf(Domain::top())) {}

  union_domain(const union_domain_t& o) : _tree(o._tree) {}

  union_domain_t& operator=(const union_domain_t& o) {
    _tree = o._tree;
    return *this;
  }

  bool is_bottom() { return _tree == nullptr; }

  bool is_top() { return is_top(_tree); }

  bool operator<=(union_domain_t o) {
    if (is_bottom()) {
      return true;
    } else if (o.is_bottom()) {
      return false;
    } else {
      return merge(_tree) <= merge(o._tree);
    }
  }

  union_domain_t operator|(union_domain_t o) {
    if (is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      tree_ptr left_branch = reduce_height(_tree, MaxHeight - 1);
      tree_ptr right_branch = reduce_height(o._tree, MaxHeight - 1);
      return union_domain(make_node(left_branch, right_branch));
    }
  }

  union_domain_t operator||(union_domain_t o) {
    if (is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      return union_domain(make_leaf(merge(_tree) || merge(o._tree)));
    }
  }

  union_domain_t join_loop(union_domain_t o) { return this->operator|(o); }

  union_domain_t join_iter(union_domain_t o) { return this->operator|(o); }

  union_domain_t operator&(union_domain_t o) {
    if (is_bottom() || o.is_bottom()) {
      return bottom();
    } else {
      Domain o_inv = merge(o._tree);
      unary_op tree_op = [&](Domain inv) { return inv & o_inv; };
      return union_domain(transform(_tree, tree_op));
    }
  }

  union_domain_t operator&&(union_domain_t o) {
    if (is_bottom() || o.is_bottom()) {
      return bottom();
    } else {
      return union_domain(make_leaf(merge(_tree) && merge(o._tree)));
    }
  }

  void operator-=(VariableName v) { forget_num(v); }

  void forget(VariableName v) { forget_num(v); }

  void forget_num(VariableName v) {
    unary_op tree_op = [&](Domain inv) {
      inv.forget_num(v);
      return inv;
    };
    _tree = transform(_tree, tree_op);
  }

  template < typename Iterator >
  void forget(Iterator begin, Iterator end) {
    forget_num(begin, end);
  }

  template < typename Iterator >
  void forget_num(Iterator begin, Iterator end) {
    unary_op tree_op = [&](Domain inv) {
      inv.forget_num(begin, end);
      return inv;
    };
    _tree = transform(_tree, tree_op);
  }

  void operator+=(linear_constraint_t cst) {
    unary_op tree_op = [&](Domain inv) {
      inv += cst;
      return inv;
    };
    _tree = transform(_tree, tree_op);
  }

  void operator+=(linear_constraint_system_t csts) {
    unary_op tree_op = [&](Domain inv) {
      inv += csts;
      return inv;
    };
    _tree = transform(_tree, tree_op);
  }

  interval_t operator[](VariableName x) { return to_interval(_tree, x); }

  congruence_t to_congruence(VariableName v) {
    return congruence_t::top();
  }

  void set(VariableName x, interval_t intv) {
    if (is_bottom())
      return;

    if (intv.is_bottom()) {
      *this = bottom();
      return;
    }

    unary_op tree_op = [&](Domain inv) {
      inv.set(x, intv);
      return inv;
    };
    _tree = transform(_tree, tree_op);
  }

  void assign(VariableName x, linear_expression_t e) {
    unary_op tree_op = [&](Domain inv) {
      inv.assign(x, e);
      return inv;
    };
    _tree = transform(_tree, tree_op);
  }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    unary_op tree_op = [&](Domain inv) {
      inv.apply(op, x, y, z);
      return inv;
    };
    _tree = transform(_tree, tree_op);
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    unary_op tree_op = [&](Domain inv) {
      inv.apply(op, x, y, k);
      return inv;
    };
    _tree = transform(_tree, tree_op);
  }

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             uint64_t from,
             uint64_t to) {
    unary_op tree_op = [&](Domain inv) {
      inv.apply(op, x, y, from, to);
      return inv;
    };
    _tree = transform(_tree, tree_op);
  }

  void apply(conv_operation_t op,
             VariableName x,
             Number k,
             uint64_t from,
             uint64_t to) {
    unary_op tree_op = [&](Domain inv) {
      inv.apply(op, x, k, from, to);
      return inv;
    };
    _tree = transform(_tree, tree_op);
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    unary_op tree_op = [&](Domain inv) {
      inv.apply(op, x, y, z);
      return inv;
    };
    _tree = transform(_tree, tree_op);
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    unary_op tree_op = [&](Domain inv) {
      inv.apply(op, x, y, k);
      return inv;
    };
    _tree = transform(_tree, tree_op);
  }

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    unary_op tree_op = [&](Domain inv) {
      inv.apply(op, x, y, z);
      return inv;
    };
    _tree = transform(_tree, tree_op);
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    unary_op tree_op = [&](Domain inv) {
      inv.apply(op, x, y, k);
      return inv;
    };
    _tree = transform(_tree, tree_op);
  }

  linear_constraint_system_t to_linear_constraint_system() {
    if (is_bottom()) {
      linear_constraint_system_t csts;
      csts += linear_constraint_t::contradiction();
      return csts;
    } else {
      Domain inv = merge(_tree);
      return inv.to_linear_constraint_system();
    }
  }

  void write(std::ostream& o) {
    if (_tree == nullptr) {
      o << "_|_";
    } else {
      _tree->write(o);
    }
  }

  static std::string domain_name() {
    return "Union of " + Domain::domain_name();
  }

}; // end class union_domain

} // end namespace ikos

#endif // IKOS_UNION_DOMAINS_HPP
