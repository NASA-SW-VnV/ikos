/*******************************************************************************
 *
 * \file
 * \brief Generic union numerical abstract domain
 *
 * Author: Maxime Arthaud
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

#include <memory>

#include <ikos/core/domain/numeric/abstract_domain.hpp>
#include <ikos/core/support/assert.hpp>
#include <ikos/core/value/numeric/congruence.hpp>
#include <ikos/core/value/numeric/interval.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief Union abstract domain
template < typename Number,
           typename VariableRef,
           typename NumericDomain,
           std::size_t MaxHeight >
class UnionDomain final
    : public numeric::AbstractDomain<
          Number,
          VariableRef,
          UnionDomain< Number, VariableRef, NumericDomain, MaxHeight > > {
public:
  using IntervalT = Interval< Number >;
  using CongruenceT = Congruence< Number >;
  using IntervalCongruenceT = IntervalCongruence< Number >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;
  using LinearConstraintSystemT = LinearConstraintSystem< Number, VariableRef >;

private:
  /*
   * Implementation of a binary tree structure
   */

  class Tree;

  using TreePtr = std::shared_ptr< const Tree >;

  class Tree {
  private:
    std::size_t _size;
    std::size_t _height;

  public:
    explicit Tree(std::size_t size, std::size_t height)
        : _size(size), _height(height) {}
    Tree(const Tree&) = delete;
    Tree(Tree&&) = delete;
    Tree& operator=(const Tree&) = delete;
    Tree& operator=(Tree&&) = delete;
    virtual ~Tree() = default;
    std::size_t size() const { return this->_size; }
    std::size_t height() const { return this->_height; }
    bool is_leaf() const { return this->_height == 0; }
    bool is_node() const { return !this->is_leaf(); }
    virtual void dump(std::ostream& o) const = 0;

  }; // end class Tree

  class Node final : public Tree {
  private:
    TreePtr _left_branch;
    TreePtr _right_branch;

  public:
    Node(const TreePtr& left_branch, const TreePtr& right_branch)
        : Tree(/*size = */ left_branch->size() + right_branch->size(),
               /*height = */ 1 +
                   std::max(left_branch->height(), right_branch->height())),
          _left_branch(left_branch),
          _right_branch(right_branch) {}

    const TreePtr& left_branch() const { return this->_left_branch; }

    const TreePtr& right_branch() const { return this->_right_branch; }

    void dump(std::ostream& o) const override {
      o << "N(";
      this->_left_branch->dump(o);
      o << ",";
      this->_right_branch->dump(o);
      o << ")";
    }

  }; // end class Node

  class Leaf final : public Tree {
  private:
    NumericDomain _value;

  public:
    explicit Leaf(NumericDomain value)
        : Tree(/*size = */ 1, /*height = */ 0), _value(std::move(value)) {}

    const NumericDomain& value() const { return this->_value; }

    void dump(std::ostream& o) const override {
      o << "L(";
      this->_value.dump(o);
      o << ")";
    }

  }; // end class Leaf

  static TreePtr make_node(const TreePtr& left_branch,
                           const TreePtr& right_branch) {
    if (left_branch == nullptr) {
      return right_branch;
    }
    if (right_branch == nullptr) {
      return left_branch;
    }
    return std::make_shared< const Node >(left_branch, right_branch);
  }

  static TreePtr make_leaf(NumericDomain value) {
    value.normalize();
    if (value.is_bottom()) {
      return nullptr;
    }
    return std::make_shared< const Leaf >(std::move(value));
  }

  template < typename UnaryOp >
  static TreePtr transform(const TreePtr& t, const UnaryOp& op) {
    if (t == nullptr) {
      return nullptr;
    }
    if (t->is_leaf()) {
      auto leaf = std::static_pointer_cast< const Leaf >(t);
      return make_leaf(op(leaf->value()));
    }
    auto node = std::static_pointer_cast< const Node >(t);
    return make_node(transform(node->left_branch(), op),
                     transform(node->right_branch(), op));
  }

  static bool is_top(const TreePtr& t) {
    if (t == nullptr) {
      return false; // bottom
    }
    if (t->is_leaf()) {
      auto leaf = std::static_pointer_cast< const Leaf >(t);
      return leaf->value().is_top();
    }
    auto node = std::static_pointer_cast< const Node >(t);
    return is_top(node->left_branch()) && is_top(node->right_branch());
  }

  static NumericDomain merge(const TreePtr& t) {
    ikos_assert(t != nullptr);

    if (t->is_leaf()) {
      auto leaf = std::static_pointer_cast< const Leaf >(t);
      return leaf->value();
    }
    auto node = std::static_pointer_cast< const Node >(t);
    return merge(node->left_branch()).join(merge(node->right_branch()));
  }

  /// \brief Reduce the height of a tree by merging the nodes at a heigh
  /// greater than max_height
  static TreePtr reduce_height(const TreePtr& t, std::size_t max_height) {
    if (t == nullptr) {
      return nullptr;
    }
    if (t->height() <= max_height) {
      return t;
    }
    if (max_height == 0) {
      return make_leaf(merge(t));
    } else {
      // t->height() > max_height > 0
      auto node = std::static_pointer_cast< const Node >(t);
      return make_node(reduce_height(node->left_branch(), max_height - 1),
                       reduce_height(node->right_branch(), max_height - 1));
    }
  }

private:
  TreePtr _tree;

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top abstract value
  explicit UnionDomain(TopTag) : _tree(make_leaf(NumericDomain::top())) {}

  /// \brief Create the bottom abstract value
  explicit UnionDomain(BottomTag) : _tree(nullptr) {}

  /// \brief Create an abstract value from a tree
  explicit UnionDomain(TreePtr tree) : _tree(std::move(tree)) {}

public:
  /// \brief Create the top abstract value
  static UnionDomain top() { return UnionDomain(TopTag{}); }

  /// \brief Create the bottom abstract value
  static UnionDomain bottom() { return UnionDomain(BottomTag{}); }

  /// \brief Copy constructor
  UnionDomain(const UnionDomain&) noexcept = default;

  /// \brief Move constructor
  UnionDomain(UnionDomain&&) noexcept = default;

  /// \brief Copy assignment operator
  UnionDomain& operator=(const UnionDomain&) noexcept = default;

  /// \brief Move assignment operator
  UnionDomain& operator=(UnionDomain&&) noexcept = default;

  /// \brief Destructor
  ~UnionDomain() override = default;

  void normalize() override {}

  bool is_bottom() const override {
    // This is correct because make_leaf() calls .is_bottom() on leaf creation
    return this->_tree == nullptr;
  }

  bool is_top() const override { return is_top(_tree); }

  void set_to_bottom() override { this->_tree = nullptr; }

  void set_to_top() override { this->_tree = make_leaf(NumericDomain::top()); }

  bool leq(const UnionDomain& other) const override {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      return merge(this->_tree).leq(merge(other._tree));
    }
  }

  bool equals(const UnionDomain& other) const override {
    if (this->is_bottom()) {
      return other.is_bottom();
    } else if (other.is_bottom()) {
      return false;
    } else {
      return merge(this->_tree).equals(merge(other._tree));
    }
  }

  UnionDomain join(const UnionDomain& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      TreePtr left_branch = reduce_height(this->_tree, MaxHeight - 1);
      TreePtr right_branch = reduce_height(other._tree, MaxHeight - 1);
      return UnionDomain(make_node(left_branch, right_branch));
    }
  }

  void join_with(const UnionDomain& other) override {
    this->operator=(this->join(other));
  }

  void join_loop_with(const UnionDomain& other) override {
    this->join_with(other);
  }

  void join_iter_with(const UnionDomain& other) override {
    this->join_with(other);
  }

  UnionDomain widening(const UnionDomain& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return UnionDomain(
          make_leaf(merge(this->_tree).widening(merge(other._tree))));
    }
  }

  void widen_with(const UnionDomain& other) override {
    this->operator=(this->widening(other));
  }

  UnionDomain widening_threshold(const UnionDomain& other,
                                 const Number& threshold) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return UnionDomain(
          make_leaf(merge(this->_tree)
                        .widening_threshold(merge(other._tree), threshold)));
    }
  }

  void widen_threshold_with(const UnionDomain& other,
                            const Number& threshold) override {
    this->operator=(this->widening_threshold(other, threshold));
  }

  UnionDomain meet(const UnionDomain& other) const override {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else {
      NumericDomain other_inv = merge(other._tree);
      return UnionDomain(transform(_tree, [&](const NumericDomain& inv) {
        return inv.meet(other_inv);
      }));
    }
  }

  void meet_with(const UnionDomain& other) override {
    this->operator=(this->meet(other));
  }

  UnionDomain narrowing(const UnionDomain& other) const override {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else {
      return UnionDomain(
          make_leaf(merge(this->_tree).narrowing(merge(other._tree))));
    }
  }

  void narrow_with(const UnionDomain& other) override {
    this->operator=(this->narrowing(other));
  }

  UnionDomain narrowing_threshold(const UnionDomain& other,
                                  const Number& threshold) const override {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else {
      return UnionDomain(
          make_leaf(merge(this->_tree)
                        .narrowing_threshold(merge(other._tree), threshold)));
    }
  }

  void narrow_threshold_with(const UnionDomain& other,
                             const Number& threshold) override {
    this->operator=(this->narrowing_threshold(other, threshold));
  }

  void assign(VariableRef x, int n) override {
    this->_tree = transform(this->_tree, [&](NumericDomain inv) {
      inv.assign(x, n);
      return inv;
    });
  }

  void assign(VariableRef x, const Number& n) override {
    this->_tree = transform(this->_tree, [&](NumericDomain inv) {
      inv.assign(x, n);
      return inv;
    });
  }

  void assign(VariableRef x, VariableRef y) override {
    this->_tree = transform(this->_tree, [&](NumericDomain inv) {
      inv.assign(x, y);
      return inv;
    });
  }

  void assign(VariableRef x, const LinearExpressionT& e) override {
    this->_tree = transform(this->_tree, [&](NumericDomain inv) {
      inv.assign(x, e);
      return inv;
    });
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             VariableRef z) override {
    this->_tree = transform(this->_tree, [&](NumericDomain inv) {
      inv.apply(op, x, y, z);
      return inv;
    });
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             const Number& z) override {
    this->_tree = transform(this->_tree, [&](NumericDomain inv) {
      inv.apply(op, x, y, z);
      return inv;
    });
  }

  void apply(BinaryOperator op,
             VariableRef x,
             const Number& y,
             VariableRef z) override {
    this->_tree = transform(this->_tree, [&](NumericDomain inv) {
      inv.apply(op, x, y, z);
      return inv;
    });
  }

  void add(const LinearConstraintT& cst) override {
    this->_tree = transform(this->_tree, [&](NumericDomain inv) {
      inv.add(cst);
      return inv;
    });
  }

  void add(const LinearConstraintSystemT& csts) override {
    this->_tree = transform(this->_tree, [&](NumericDomain inv) {
      inv.add(csts);
      return inv;
    });
  }

  void set(VariableRef x, const IntervalT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_tree = transform(this->_tree, [&](NumericDomain inv) {
        inv.set(x, value);
        return inv;
      });
    }
  }

  void set(VariableRef x, const CongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_tree = transform(this->_tree, [&](NumericDomain inv) {
        inv.set(x, value);
        return inv;
      });
    }
  }

  void set(VariableRef x, const IntervalCongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_tree = transform(this->_tree, [&](NumericDomain inv) {
        inv.set(x, value);
        return inv;
      });
    }
  }

  void refine(VariableRef x, const IntervalT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_tree = transform(this->_tree, [&](NumericDomain inv) {
        inv.refine(x, value);
        return inv;
      });
    }
  }

  void refine(VariableRef x, const CongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_tree = transform(this->_tree, [&](NumericDomain inv) {
        inv.refine(x, value);
        return inv;
      });
    }
  }

  void refine(VariableRef x, const IntervalCongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_tree = transform(this->_tree, [&](NumericDomain inv) {
        inv.refine(x, value);
        return inv;
      });
    }
  }

  void forget(VariableRef x) override {
    this->_tree = transform(this->_tree, [&](NumericDomain inv) {
      inv.forget(x);
      return inv;
    });
  }

private:
  static IntervalT to_interval(const TreePtr& t, VariableRef v) {
    if (t == nullptr) {
      return IntervalT::bottom();
    }
    if (t->is_leaf()) {
      auto leaf = std::static_pointer_cast< const Leaf >(t);
      return leaf->value().to_interval(v);
    }
    auto node = std::static_pointer_cast< const Node >(t);
    return to_interval(node->left_branch(), v)
        .join(to_interval(node->right_branch(), v));
  }

public:
  IntervalT to_interval(VariableRef x) const override {
    return to_interval(this->_tree, x);
  }

private:
  static IntervalT to_interval(const TreePtr& t, const LinearExpressionT& e) {
    if (t == nullptr) {
      return IntervalT::bottom();
    }
    if (t->is_leaf()) {
      auto leaf = std::static_pointer_cast< const Leaf >(t);
      return leaf->value().to_interval(e);
    }
    auto node = std::static_pointer_cast< const Node >(t);
    return to_interval(node->left_branch(), e)
        .join(to_interval(node->right_branch(), e));
  }

public:
  IntervalT to_interval(const LinearExpressionT& e) const override {
    return to_interval(this->_tree, e);
  }

private:
  static CongruenceT to_congruence(const TreePtr& t, VariableRef v) {
    if (t == nullptr) {
      return CongruenceT::bottom();
    }
    if (t->is_leaf()) {
      auto leaf = std::static_pointer_cast< const Leaf >(t);
      return leaf->value().to_congruence(v);
    }
    auto node = std::static_pointer_cast< const Node >(t);
    return to_congruence(node->left_branch(), v)
        .join(to_congruence(node->right_branch(), v));
  }

public:
  CongruenceT to_congruence(VariableRef x) const override {
    return to_congruence(this->_tree, x);
  }

private:
  static CongruenceT to_congruence(const TreePtr& t,
                                   const LinearExpressionT& e) {
    if (t == nullptr) {
      return CongruenceT::bottom();
    }
    if (t->is_leaf()) {
      auto leaf = std::static_pointer_cast< const Leaf >(t);
      return leaf->value().to_congruence(e);
    }
    auto node = std::static_pointer_cast< const Node >(t);
    return to_congruence(node->left_branch(), e)
        .join(to_congruence(node->right_branch(), e));
  }

public:
  CongruenceT to_congruence(const LinearExpressionT& e) const override {
    return to_congruence(this->_tree, e);
  }

private:
  static IntervalCongruenceT to_interval_congruence(const TreePtr& t,
                                                    VariableRef v) {
    if (t == nullptr) {
      return IntervalCongruenceT::bottom();
    }
    if (t->is_leaf()) {
      auto leaf = std::static_pointer_cast< const Leaf >(t);
      return leaf->value().to_interval_congruence(v);
    }
    auto node = std::static_pointer_cast< const Node >(t);
    return to_interval_congruence(node->left_branch(), v)
        .join(to_interval_congruence(node->right_branch(), v));
  }

public:
  IntervalCongruenceT to_interval_congruence(VariableRef x) const override {
    return to_interval_congruence(this->_tree, x);
  }

private:
  static IntervalCongruenceT to_interval_congruence(
      const TreePtr& t, const LinearExpressionT& e) {
    if (t == nullptr) {
      return IntervalCongruenceT::bottom();
    }
    if (t->is_leaf()) {
      auto leaf = std::static_pointer_cast< const Leaf >(t);
      return leaf->value().to_interval_congruence(e);
    }
    auto node = std::static_pointer_cast< const Node >(t);
    return to_interval_congruence(node->left_branch(), e)
        .join(to_interval_congruence(node->right_branch(), e));
  }

public:
  IntervalCongruenceT to_interval_congruence(
      const LinearExpressionT& e) const override {
    return to_interval_congruence(this->_tree, e);
  }

  LinearConstraintSystemT to_linear_constraint_system() const override {
    if (this->is_bottom()) {
      return LinearConstraintSystemT(LinearConstraintT::contradiction());
    }

    return merge(this->_tree).to_linear_constraint_system();
  }

  /// \name Non-negative loop counter abstract domain methods
  /// @{

  void counter_mark(VariableRef x) override {
    this->_tree = transform(this->_tree, [&](NumericDomain inv) {
      inv.counter_mark(x);
      return inv;
    });
  }

  void counter_unmark(VariableRef x) override {
    this->_tree = transform(this->_tree, [&](NumericDomain inv) {
      inv.counter_unmark(x);
      return inv;
    });
  }

  void counter_init(VariableRef x, const Number& c) override {
    this->_tree = transform(this->_tree, [&](NumericDomain inv) {
      inv.counter_init(x, c);
      return inv;
    });
  }

  void counter_incr(VariableRef x, const Number& k) override {
    this->_tree = transform(this->_tree, [&](NumericDomain inv) {
      inv.counter_incr(x, k);
      return inv;
    });
  }

  void counter_forget(VariableRef x) override {
    this->_tree = transform(this->_tree, [&](NumericDomain inv) {
      inv.counter_forget(x);
      return inv;
    });
  }

  /// @}

  void dump(std::ostream& o) const override {
    if (this->_tree == nullptr) {
      o << "⊥";
    } else {
      this->_tree->dump(o);
    }
  }

  static std::string name() { return "union of " + NumericDomain::name(); }

}; // end class union_domain

} // end namespace numeric
} // end namespace core
} // end namespace ikos
