/*******************************************************************************
 *
 * \file
 * \brief Abstract domain using variable packing techniques
 *
 * Implementation of an abstract domain using variable packing techniques to
 * improve the performance of a relational abstract domain.
 *
 * Based on Arnaud Venet & Guillaume Brat's paper: Precise and efficient static
 * array bound checking for large embedded C programs, in PLDI, 231-242, 2004.
 *
 * The idea is to keep a set of equivalence classes, using the union-find
 * structure. Each equivalence class holds an abstract domain over all
 * variables within the class.
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
#include <unordered_map>

#include <boost/container/flat_set.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/optional.hpp>

#include <ikos/core/domain/numeric/abstract_domain.hpp>
#include <ikos/core/semantic/indexable.hpp>
#include <ikos/core/support/assert.hpp>
#include <ikos/core/value/numeric/congruence.hpp>
#include <ikos/core/value/numeric/interval.hpp>

namespace ikos {
namespace core {
namespace numeric {

// Forward declaration
template < typename Number,
           typename VariableRef,
           std::size_t MaxReductionCycles >
class VarPackingDBM;

// Forward declaration
template < typename Number,
           typename VariableRef,
           std::size_t MaxReductionCycles >
class VarPackingDBMCongruence;

/// \brief Generic abstract domain with variable packing
///
/// The idea is to keep a set of equivalence classes, using the union-find
/// structure. Each equivalence class holds an abstract domain over all
/// variables within the class.
template < typename Number, typename VariableRef, typename Domain >
class VarPackingDomain final
    : public numeric::AbstractDomain<
          Number,
          VariableRef,
          VarPackingDomain< Number, VariableRef, Domain > > {
public:
  static_assert(numeric::IsAbstractDomain< Domain, Number, VariableRef >::value,
                "Domain must be a numerical abstract domain");

public:
  using IntervalT = Interval< Number >;
  using CongruenceT = Congruence< Number >;
  using IntervalCongruenceT = IntervalCongruence< Number >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;
  using LinearConstraintSystemT = LinearConstraintSystem< Number, VariableRef >;

public:
  template < typename, typename, std::size_t >
  friend class VarPackingDBM;

  template < typename, typename, std::size_t >
  friend class VarPackingDBMCongruence;

private:
  /// \brief Shared pointer on the underlying abstract domain
  using DomainPtr = std::shared_ptr< Domain >;

  /// \brief Hash function for VariableRef
  struct VariableRefHash {
    std::size_t operator()(VariableRef v) const {
      return IndexableTraits< VariableRef >::index(v);
    }
  };

  /// \brief Map from root variable to list of variables
  using RootVariablesMap = std::
      unordered_map< VariableRef, std::vector< VariableRef >, VariableRefHash >;

  /// \brief Parent class
  using Parent =
      numeric::AbstractDomain< Number, VariableRef, VarPackingDomain >;

  /// \brief Result of a call to EquivalenceRelation::forget(v)
  enum class ForgetResult {
    /// \brief Success
    Success,

    /// \brief Bottom was found
    Bottom,
  };

  /*
   * Implementation of Union-Find
   */

  // forward declaration
  class EquivalenceRelation;

  /// \brief Equivalence class
  ///
  /// Represents a 'pack' of variables
  class EquivalenceClass {
  public:
    std::size_t rank;
    DomainPtr domain;

    // TODO(marthaud): We could store the list of variables in the class

    // Note: The reason why sometimes we explicitly call domain->normalize() is
    // because it is better to normalize a (probably) shared domain. If we don't
    // do that, some methods will normalize a copy.

  public:
    /// \brief Create an empty equivalence class
    explicit EquivalenceClass()
        : rank(0), domain(std::make_shared< Domain >()) {}

    /// \brief Copy constructor
    EquivalenceClass(const EquivalenceClass&) noexcept = default;

    /// \brief Move constructor
    EquivalenceClass(EquivalenceClass&&) noexcept = default;

    /// \brief Copy assignment operator
    EquivalenceClass& operator=(const EquivalenceClass&) noexcept = default;

    /// \brief Move assignment operator
    EquivalenceClass& operator=(EquivalenceClass&&) noexcept = default;

    /// \brief Destructor
    ~EquivalenceClass() = default;

    /// \brief Copy before a write
    void copy_domain() {
      // XXX(marthaud): This is not thread safe.
      if (this->domain.use_count() == 1) {
        return; // copy is unnecessary
      }
      this->domain = std::make_shared< Domain >(*this->domain);
    }

  }; // end class EquivalenceClass

  /// \brief Equivalence relation
  ///
  /// Hold the equivalence classes
  class EquivalenceRelation {
  private:
    using ParentMap =
        std::unordered_map< VariableRef, VariableRef, VariableRefHash >;
    using ClassMap =
        std::unordered_map< VariableRef, EquivalenceClass, VariableRefHash >;

  private:
    // Map from variable to parent
    ParentMap _parents;

    // Map from root variable to equivalence class
    ClassMap _classes;

    // Note: We are using at(key) instead of operator[](key) because VariableRef
    // may not have a default constructor

  public:
    /// \brief Create an empty equivalence relation
    explicit EquivalenceRelation() = default;

    /// \brief Copy constructor
    EquivalenceRelation(const EquivalenceRelation&) = default;

    /// \brief Move constructor
    EquivalenceRelation(EquivalenceRelation&&) = default;

    /// \brief Copy assignment operator
    EquivalenceRelation& operator=(const EquivalenceRelation&) = default;

    /// \brief Move assignment operator
    EquivalenceRelation& operator=(EquivalenceRelation&&) = default;

    /// \brief Destructor
    ~EquivalenceRelation() = default;

    /// \brief Return true if the equivalence relation contains `v`
    bool contains(VariableRef v) const {
      return this->_parents.find(v) != this->_parents.end();
    }

    /// \brief Create an equivalence class containing the given variable
    ///
    /// Precondition: `v` is not already present in the relation
    void add_equiv_class(VariableRef v) {
      ikos_assert_msg(!this->contains(v), "variable already present");
      this->_parents.emplace(v, v);
      this->_classes.emplace(v, EquivalenceClass());
    }

    /// \brief Add a variable in an equivalence class
    ///
    /// Precondition: `v` is not already present in the relation
    /// Precondition: `parent` is present in the relation
    void add_var_to_equiv_class(VariableRef v, VariableRef parent) {
      ikos_assert_msg(!this->contains(v), "variable already present");
      ikos_assert_msg(this->contains(parent), "variable missing");

      VariableRef parent_root = this->find_root_var(parent);
      EquivalenceClass& parent_class = this->_classes[parent_root];

      if (parent_class.rank == 0) {
        parent_class.rank++;
      }
      this->_parents.emplace(v, parent_root);
    }

    /// \brief Find the root of the equivalence class containing `v`
    VariableRef find_root_var(VariableRef v) {
      VariableRef& parent = this->_parents.at(v);

      if (parent == v) {
        return v;
      } else {
        return parent = this->find_root_var(parent);
      }
    }

    /// \brief Find the root of the equivalence class containing `v`
    VariableRef cfind_root_var(VariableRef v) const {
      VariableRef parent = this->_parents.at(v);

      if (parent == v) {
        return v;
      } else {
        return this->cfind_root_var(parent);
      }
    }

    /// \brief Find the equivalence class containing `v`
    EquivalenceClass& find_equiv_class(VariableRef v) {
      return this->_classes[this->find_root_var(v)];
    }

    /// \brief Find the equivalence class containing `v`
    const EquivalenceClass& cfind_equiv_class(VariableRef v) const {
      return this->_classes.at(this->cfind_root_var(v));
    }

    /// \brief Find the abstract domain containing `v`
    const DomainPtr& find_domain(VariableRef v) {
      return this->find_equiv_class(v).domain;
    }

    /// \brief Find the abstract domain containing `v`
    const DomainPtr& cfind_domain(VariableRef v) const {
      return this->cfind_equiv_class(v).domain;
    }

    /// \brief Merge two equivalence classes
    ///
    /// Return whether `x` and `y` where on the same equivalence class
    bool join_equiv_class(VariableRef x, VariableRef y) {
      VariableRef x_root = this->find_root_var(x);
      VariableRef y_root = this->find_root_var(y);

      if (x_root == y_root) {
        return false;
      }

      EquivalenceClass& x_class = this->_classes[x_root];
      EquivalenceClass& y_class = this->_classes[y_root];

      // Merge the domains
      DomainPtr merge_domain = std::make_shared< Domain >();
      x_class.domain->normalize();
      y_class.domain->normalize();
      *merge_domain = (*x_class.domain).meet(*y_class.domain);

      if (x_class.rank > y_class.rank) {
        this->_parents.at(y_root) = x_root;

        x_class.domain.swap(merge_domain);
        this->_classes.erase(y_root);
      } else {
        this->_parents.at(x_root) = y_root;

        if (x_class.rank == y_class.rank) {
          y_class.rank++;
        }

        y_class.domain.swap(merge_domain);
        this->_classes.erase(x_root);
      }

      return true;
    }

  private:
    struct GetVar {
      const VariableRef& operator()(
          const std::pair< const VariableRef, VariableRef >& p) const {
        return p.first;
      }
    };

  public:
    /// \brief Begin iterator on the variables
    auto var_begin() const {
      return boost::make_transform_iterator(this->_parents.cbegin(), GetVar());
    }

    /// \brief End iterator on the variables
    auto var_end() const {
      return boost::make_transform_iterator(this->_parents.cend(), GetVar());
    }

    /// \brief Begin iterator on the equivalence classes
    auto begin() const { return this->_classes.cbegin(); }

    /// \brief End iterator on the equivalence classes
    auto end() const { return this->_classes.cend(); }

    /// \brief Return the list of variables
    std::vector< VariableRef > variables() const {
      return {this->var_begin(), this->var_end()};
    }

    /// \brief Clear the equivalence relation
    void clear() {
      this->_parents.clear();
      this->_classes.clear();
    }

    /// \brief Forget the given variable
    ///
    /// Note: calling forget() on an equivalence class might reduce it to bottom
    ForgetResult forget(VariableRef v) {
      auto it = this->_parents.find(v);

      if (it == this->_parents.end()) {
        return ForgetResult::Success;
      }

      if (it->second != v) {
        // v is not the root of the equivalence class
        VariableRef root = this->find_root_var(it->second);

        // update parents
        for (auto& p : this->_parents) {
          if (p.second == v) {
            p.second = root;
          }
        }

        EquivalenceClass& equiv_class = this->_classes[root];
        equiv_class.copy_domain();
        equiv_class.domain->forget(v);
      } else {
        // v is the root of the equivalence class
        boost::optional< VariableRef > new_root;

        for (auto& p : this->_parents) {
          if (p.second == v && p.first != v) {
            if (!new_root) {
              new_root = p.first;
            }
            p.second = *new_root;
          }
        }

        if (new_root) {
          EquivalenceClass equiv_class = std::move(this->_classes[v]);
          equiv_class.copy_domain();
          equiv_class.domain->forget(v);
          this->_classes.emplace(*new_root, std::move(equiv_class));
        } else {
          EquivalenceClass& equiv_class = this->_classes[v];
          if (equiv_class.domain->is_bottom()) {
            // In that case, do nothing
            return ForgetResult::Bottom;
          }
        }

        this->_classes.erase(v);
      }

      this->_parents.erase(v);
      return ForgetResult::Success;
    }

    /// \brief Forget the equivalence class containing the given variable
    void forget_equiv_class(VariableRef v) {
      if (!this->contains(v)) {
        return;
      }

      VariableRef root = this->find_root_var(v);

      for (auto it = this->_parents.begin(); it != this->_parents.end();) {
        if (this->find_root_var(it->second) == root) {
          it = _parents.erase(it);
        } else {
          ++it;
        }
      }

      this->_classes.erase(root);
    }

    /// \brief Return a map from root variables to the list of variables in the
    /// equivalence class
    RootVariablesMap root_to_vars() const {
      RootVariablesMap roots;
      for (const auto& p : this->_parents) {
        roots[this->cfind_root_var(p.second)].push_back(p.first);
      }
      return roots;
    }

    void dump(std::ostream& o) const {
      o << "({";
      for (auto it = this->_parents.begin(), et = this->_parents.end();
           it != et;) {
        DumpableTraits< VariableRef >::dump(o, it->first);
        o << " -> ";
        DumpableTraits< VariableRef >::dump(o, it->second);
        ++it;
        if (it != et) {
          o << ", ";
        }
      }
      o << "}, {";
      for (auto it = this->_classes.begin(), et = this->_classes.end();
           it != et;) {
        DumpableTraits< VariableRef >::dump(o, it->first);
        o << " -> ";
        it->second.domain->dump(o);
        ++it;
        if (it != et) {
          o << ", ";
        }
      }
      o << "})";
    }

  }; // end class EquivalenceRelation

private:
  bool _is_bottom;
  bool _is_normalized;
  EquivalenceRelation _equiv_relation;

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top abstract value
  explicit VarPackingDomain(TopTag) : _is_bottom(false), _is_normalized(true) {}

  /// \brief Create the bottom abstract value
  explicit VarPackingDomain(BottomTag)
      : _is_bottom(true), _is_normalized(true) {}

public:
  /// \brief Create the top abstract value
  VarPackingDomain() : VarPackingDomain(TopTag{}) {}

  /// \brief Copy constructor
  VarPackingDomain(const VarPackingDomain&) = default;

  /// \brief Move constructor
  VarPackingDomain(VarPackingDomain&&) = default;

  /// \brief Copy assignment operator
  VarPackingDomain& operator=(const VarPackingDomain&) = default;

  /// \brief Move assignment operator
  VarPackingDomain& operator=(VarPackingDomain&&) = default;

  /// \brief Destructor
  ~VarPackingDomain() override = default;

  /// \brief Create the top abstract value
  static VarPackingDomain top() { return VarPackingDomain(TopTag{}); }

  /// \brief Create the bottom abstract value
  static VarPackingDomain bottom() { return VarPackingDomain(BottomTag{}); }

  /// \brief Normalize the abstract value
  void normalize() const override {
    if (this->_is_normalized) {
      return;
    }

    auto self = const_cast< VarPackingDomain* >(this);

    if (this->_is_bottom) {
      self->set_to_bottom();
      return;
    }

    for (const auto& equiv_class : this->_equiv_relation) {
      equiv_class.second.domain->normalize();

      if (equiv_class.second.domain->is_bottom()) {
        self->set_to_bottom();
        return;
      }
    }

    self->_is_normalized = true;
  }

  bool is_bottom() const override {
    this->normalize();
    return this->_is_bottom;
  }

  bool is_top() const override {
    // Does not require normalization

    if (this->_is_bottom) {
      return false;
    }

    for (const auto& equiv_class : this->_equiv_relation) {
      if (!equiv_class.second.domain->is_top()) {
        return false;
      }
    }

    return true;
  }

  void set_to_bottom() override {
    this->_is_bottom = true;
    this->_is_normalized = true;
    this->_equiv_relation.clear();
  }

  void set_to_top() override {
    this->_is_bottom = false;
    this->_is_normalized = true;
    this->_equiv_relation.clear();
  }

  bool leq(const VarPackingDomain& other) const override {
    // Requires normalization
    this->normalize();
    other.normalize();

    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      RootVariablesMap other_roots = other._equiv_relation.root_to_vars();

      // For each equivalence class in `other`
      for (const auto& other_class : other_roots) {
        const VariableRef& other_root = other_class.first;
        const DomainPtr& other_domain =
            other._equiv_relation.cfind_domain(other_root);

        // Set of root variables of equivalence classes we have merged
        boost::container::flat_set< VariableRef > this_roots;

        // Domain for `this`, containing all needed variables
        DomainPtr this_domain = nullptr;

        // Merge domains in `this`
        for (VariableRef v : other_class.second) {
          if (!this->_equiv_relation.contains(v)) {
            continue; // v not in `this`
          }

          VariableRef this_root = this->_equiv_relation.cfind_root_var(v);

          if (this_roots.find(this_root) != this_roots.end()) {
            continue; // equivalence class for v already merged
          }

          this_roots.insert(this_root);

          const DomainPtr& domain =
              this->_equiv_relation.cfind_domain(this_root);

          // Merge `domain` into `this_domain`
          if (this_domain == nullptr) {
            this_domain = domain;
          } else if (this_domain == domain) {
            // nothing to do
          } else {
            DomainPtr merge_domain = std::make_shared< Domain >();
            this_domain->normalize();
            domain->normalize();
            *merge_domain = (*this_domain).meet(*domain);
            this_domain.swap(merge_domain);
          }
        }

        // Compare `this_domain` and `other_domain`
        if (this_domain == nullptr) {
          if (!other_domain->is_top()) {
            return false;
          }
        } else if (this_domain == other_domain) {
          // this_domain.leq(other_domain) is true
        } else {
          this_domain->normalize();
          other_domain->normalize();
          if (!((*this_domain).leq(*other_domain))) {
            return false;
          }
        }
      }

      return true;
    }
  }

  bool equals(const VarPackingDomain& other) const override {
    return this->leq(other) && other.leq(*this);
  }

private:
  void merge_existing_equiv_classes(boost::optional< VariableRef >& root,
                                    VariableRef v) {
    if (!this->_equiv_relation.contains(v)) {
      return;
    }

    if (root) {
      this->_equiv_relation.join_equiv_class(v, *root);
    } else {
      root = v;
    }
  }

  void merge_unexisting_equiv_classes(boost::optional< VariableRef >& root,
                                      VariableRef v) {
    if (this->_equiv_relation.contains(v)) {
      return;
    }

    if (root) {
      this->_equiv_relation.add_var_to_equiv_class(v, *root);
    } else {
      root = v;
      this->_equiv_relation.add_equiv_class(v);
    }
  }

  /// \brief Apply a binary operation using a union semantic (join, widening)
  template < typename BinaryOperator >
  VarPackingDomain union_binary_op(VarPackingDomain other,
                                   const BinaryOperator& op) const {
    // `other` is a copy, thus we can update it
    /// TODO(marthaud): try to implement this without copying `other`
    VarPackingDomain result(*this);

    // Forget variables in `result` that are not in `other`
    // TODO(marthaud): variables() needs to creates a temporary vector because
    // forget() might invalidate iterators
    for (VariableRef v : result._equiv_relation.variables()) {
      if (!other._equiv_relation.contains(v)) {
        result._equiv_relation.forget(v);
      }
    }

    // Forget variables in `other` that are not in `result`
    // TODO(marthaud): variables() needs to creates a temporary vector because
    // forget() might invalidate iterators
    for (VariableRef v : other._equiv_relation.variables()) {
      if (!result._equiv_relation.contains(v)) {
        other._equiv_relation.forget(v);
      }
    }

    {
      // Iterate on equivalence classes in `results`, merge the variables in
      // `other`
      RootVariablesMap result_roots = result._equiv_relation.root_to_vars();
      for (const auto& result_class : result_roots) {
        boost::optional< VariableRef > root;

        for (VariableRef v : result_class.second) {
          other.merge_existing_equiv_classes(root, v);
        }
      }
    }

    {
      // Iterator on equivalence classes in `other`, merge the variables in
      // `results` and compute the binary operation
      RootVariablesMap other_roots = other._equiv_relation.root_to_vars();
      for (const auto& other_class : other_roots) {
        const VariableRef& other_root = other_class.first;
        const DomainPtr& other_domain =
            other._equiv_relation.find_domain(other_root);

        boost::optional< VariableRef > root;
        for (VariableRef v : other_class.second) {
          result.merge_existing_equiv_classes(root, v);
        }

        if (root) {
          EquivalenceClass& equiv_class =
              result._equiv_relation.find_equiv_class(*root);
          if (equiv_class.domain == other_domain) {
            // nothing to do, left and right packs are the same
          } else {
            DomainPtr merge_domain = std::make_shared< Domain >();
            op(*merge_domain, *equiv_class.domain, *other_domain);
            equiv_class.domain = merge_domain;
          }
        }
      }
    }

    result._is_normalized = false;
    return result;
  }

  /// \brief Binary operation using an intersection semantic (meet, narrowing)
  template < typename BinaryOperator >
  VarPackingDomain meet_binary_op(const VarPackingDomain& other,
                                  BinaryOperator op) const {
    VarPackingDomain result(*this);

    RootVariablesMap other_roots = other._equiv_relation.root_to_vars();
    for (const auto& other_class : other_roots) {
      const VariableRef& other_root = other_class.first;
      const DomainPtr& other_domain =
          other._equiv_relation.cfind_domain(other_root);

      boost::optional< VariableRef > root;
      for (VariableRef v : other_class.second) {
        result.merge_existing_equiv_classes(root, v);
      }

      bool new_domain = !root;
      for (VariableRef v : other_class.second) {
        result.merge_unexisting_equiv_classes(root, v);
      }

      EquivalenceClass& equiv_class =
          result._equiv_relation.find_equiv_class(*root);
      if (!new_domain) {
        if (equiv_class.domain == other_domain) {
          // nothing to do, left and right packs are the same
        } else {
          DomainPtr merge_domain = std::make_shared< Domain >();
          op(*merge_domain, *equiv_class.domain, *other_domain);
          equiv_class.domain = merge_domain;
        }
      } else {
        equiv_class.domain = other_domain;
      }
    }

    result._is_normalized = false;
    return result;
  }

  struct JoinOperator {
    void operator()(Domain& result, Domain& left, Domain& right) const {
      left.normalize();
      right.normalize();
      result = left.join(right);
    }
  };

  struct MeetOperator {
    void operator()(Domain& result, Domain& left, Domain& right) const {
      left.normalize();
      right.normalize();
      result = left.meet(right);
    }
  };

  struct WideningOperator {
    void operator()(Domain& result, Domain& left, Domain& right) const {
      right.normalize();
      result = left.widening(right);
    }
  };

  struct WideningThresholdOperator {
    const Number& threshold;

    void operator()(Domain& result, Domain& left, Domain& right) const {
      right.normalize();
      result = left.widening_threshold(right, threshold);
    }
  };

  struct NarrowingOperator {
    void operator()(Domain& result, Domain& left, Domain& right) const {
      left.normalize();
      right.normalize();
      result = left.narrowing(right);
    }
  };

public:
  VarPackingDomain join(const VarPackingDomain& other) const override {
    // Requires normalization
    this->normalize();
    other.normalize();

    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return this->union_binary_op(other, JoinOperator{});
    }
  }

  void join_with(const VarPackingDomain& other) override {
    this->operator=(this->join(other));
  }

  VarPackingDomain widening(const VarPackingDomain& other) const override {
    // Requires the normalization of the right operand.
    // The left operand (this) should not be normalized.
    other.normalize();

    if (this->_is_bottom) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return this->union_binary_op(other, WideningOperator{});
    }
  }

  void widen_with(const VarPackingDomain& other) override {
    this->operator=(this->widening(other));
  }

  VarPackingDomain widening_threshold(const VarPackingDomain& other,
                                      const Number& threshold) const override {
    // Requires the normalization of the right operand.
    // The left operand (this) should not be normalized.
    other.normalize();

    if (this->_is_bottom) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return this->union_binary_op(other, WideningThresholdOperator{threshold});
    }
  }

  void widen_threshold_with(const VarPackingDomain& other,
                            const Number& threshold) override {
    this->operator=(this->widening_threshold(other, threshold));
  }

  VarPackingDomain meet(const VarPackingDomain& other) const override {
    // Requires normalization
    this->normalize();
    other.normalize();

    if (this->is_bottom() || other.is_bottom()) {
      return VarPackingDomain::bottom();
    } else {
      return this->meet_binary_op(other, MeetOperator{});
    }
  }

  void meet_with(const VarPackingDomain& other) override {
    this->operator=(this->meet(other));
  }

  VarPackingDomain narrowing(const VarPackingDomain& other) const override {
    // Requires normalization
    this->normalize();
    other.normalize();

    if (this->is_bottom() || other.is_bottom()) {
      return VarPackingDomain::bottom();
    } else {
      return this->meet_binary_op(other, NarrowingOperator{});
    }
  }

  void narrow_with(const VarPackingDomain& other) override {
    this->operator=(this->narrowing(other));
  }

  void assign(VariableRef x, int n) override { this->assign(x, Number(n)); }

  void assign(VariableRef x, const Number& n) override {
    if (this->_is_bottom) {
      return;
    }

    auto result = this->_equiv_relation.forget(x);
    if (result == ForgetResult::Bottom) {
      this->set_to_bottom();
      return;
    }

    this->_equiv_relation.add_equiv_class(x);
    this->_equiv_relation.find_domain(x)->assign(x, n);
    this->_is_normalized = false;
  }

  void assign(VariableRef x, VariableRef y) override {
    if (this->_is_bottom) {
      return;
    }

    if (x == y) {
      return;
    }

    if (!this->_equiv_relation.contains(y)) {
      this->_equiv_relation.add_equiv_class(y);
    }

    auto result = this->_equiv_relation.forget(x);
    if (result == ForgetResult::Bottom) {
      this->set_to_bottom();
      return;
    }

    this->_equiv_relation.add_var_to_equiv_class(x, y);
    EquivalenceClass& equiv_class = this->_equiv_relation.find_equiv_class(y);
    equiv_class.copy_domain();
    equiv_class.domain->assign(x, y);
    this->_is_normalized = false;
  }

  void assign(VariableRef x, const LinearExpressionT& e) override {
    if (this->_is_bottom) {
      return;
    }

    if (e.is_constant()) {
      this->assign(x, e.constant());
      return;
    }

    boost::optional< VariableRef > root;

    for (const auto& term : e) {
      this->merge_existing_equiv_classes(root, term.first);
    }

    for (const auto& term : e) {
      this->merge_unexisting_equiv_classes(root, term.first);
    }

    if (e.factor(x) == 0) {
      auto result = this->_equiv_relation.forget(x);
      if (result == ForgetResult::Bottom) {
        this->set_to_bottom();
        return;
      }

      this->_equiv_relation.add_var_to_equiv_class(x, *root);
    }

    // otherwise, x has already been merged

    EquivalenceClass& equiv_class =
        this->_equiv_relation.find_equiv_class(*root);
    equiv_class.copy_domain();
    equiv_class.domain->assign(x, e);
    this->_is_normalized = false;
  }

private:
  /// \brief Add a relation x = f(y, z)
  const DomainPtr& add_relation(VariableRef x, VariableRef y, VariableRef z) {
    boost::optional< VariableRef > root;
    this->merge_existing_equiv_classes(root, y);
    this->merge_existing_equiv_classes(root, z);
    this->merge_unexisting_equiv_classes(root, y);
    this->merge_unexisting_equiv_classes(root, z);

    if (x != y && x != z) {
      auto result = this->_equiv_relation.forget(x);
      if (result == ForgetResult::Bottom) {
        this->_is_bottom = true;
        return this->_equiv_relation.find_domain(x);
      }

      this->_equiv_relation.add_var_to_equiv_class(x, *root);
    }
    // otherwise, x has already been merged

    EquivalenceClass& equiv_class =
        this->_equiv_relation.find_equiv_class(*root);
    equiv_class.copy_domain();
    return equiv_class.domain;
  }

  /// \brief Add a relation x = f(y)
  const DomainPtr& add_relation(VariableRef x, VariableRef y) {
    if (!this->_equiv_relation.contains(y)) {
      this->_equiv_relation.add_equiv_class(y);
    }

    if (x != y) {
      auto result = this->_equiv_relation.forget(x);
      if (result == ForgetResult::Bottom) {
        this->_is_bottom = true;
        return this->_equiv_relation.find_domain(x);
      }
      this->_equiv_relation.add_var_to_equiv_class(x, y);
    }

    EquivalenceClass& equiv_class = this->_equiv_relation.find_equiv_class(y);
    equiv_class.copy_domain();
    return equiv_class.domain;
  }

public:
  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             VariableRef z) override {
    if (this->_is_bottom) {
      return;
    }

    this->add_relation(x, y, z)->apply(op, x, y, z);
    this->_is_normalized = false;
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             const Number& z) override {
    if (this->_is_bottom) {
      return;
    }

    add_relation(x, y)->apply(op, x, y, z);
    this->_is_normalized = false;
  }

  void apply(BinaryOperator op,
             VariableRef x,
             const Number& y,
             VariableRef z) override {
    if (this->_is_bottom) {
      return;
    }

    add_relation(x, z)->apply(op, x, y, z);
    this->_is_normalized = false;
  }

  void add(const LinearConstraintT& cst) override {
    if (this->_is_bottom) {
      return;
    }

    if (cst.num_terms() == 0) {
      if (cst.is_contradiction()) {
        this->set_to_bottom();
      }
      return;
    }

    boost::optional< VariableRef > root;
    for (const auto& term : cst) {
      this->merge_existing_equiv_classes(root, term.first);
    }

    for (const auto& term : cst) {
      this->merge_unexisting_equiv_classes(root, term.first);
    }

    EquivalenceClass& equiv_class =
        this->_equiv_relation.find_equiv_class(*root);
    equiv_class.copy_domain();
    equiv_class.domain->add(cst);
    this->_is_normalized = false;
  }

  void add(const LinearConstraintSystemT& csts) override {
    for (const LinearConstraintT& cst : csts) {
      this->add(cst);
    }
  }

  void set(VariableRef x, const IntervalT& value) override {
    if (this->_is_bottom) {
      return;
    }

    if (value.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    auto result = this->_equiv_relation.forget(x);
    if (result == ForgetResult::Bottom) {
      this->set_to_bottom();
      return;
    }

    if (value.is_top()) {
      return;
    }

    this->_equiv_relation.add_equiv_class(x);
    this->_equiv_relation.find_domain(x)->set(x, value);
    this->_is_normalized = false;
  }

  void set(VariableRef x, const CongruenceT& value) override {
    if (this->_is_bottom) {
      return;
    }

    if (value.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    auto result = this->_equiv_relation.forget(x);
    if (result == ForgetResult::Bottom) {
      this->set_to_bottom();
      return;
    }

    if (value.is_top()) {
      return;
    }

    this->_equiv_relation.add_equiv_class(x);
    this->_equiv_relation.find_domain(x)->set(x, value);
    this->_is_normalized = false;
  }

  void set(VariableRef x, const IntervalCongruenceT& value) override {
    if (this->_is_bottom) {
      return;
    }

    if (value.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    auto result = this->_equiv_relation.forget(x);
    if (result == ForgetResult::Bottom) {
      this->set_to_bottom();
      return;
    }

    if (value.is_top()) {
      return;
    }

    this->_equiv_relation.add_equiv_class(x);
    this->_equiv_relation.find_domain(x)->set(x, value);
    this->_is_normalized = false;
  }

  void refine(VariableRef x, const IntervalT& value) override {
    if (this->_is_bottom) {
      return;
    }

    if (value.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    if (value.is_top()) {
      return;
    }

    if (this->_equiv_relation.contains(x)) {
      EquivalenceClass& equiv_class = this->_equiv_relation.find_equiv_class(x);
      equiv_class.copy_domain();
      equiv_class.domain->refine(x, value);
      this->_is_normalized = false;
    } else {
      this->_equiv_relation.add_equiv_class(x);
      this->_equiv_relation.find_domain(x)->set(x, value);
    }
  }

  void refine(VariableRef x, const CongruenceT& value) override {
    if (this->_is_bottom) {
      return;
    }

    if (value.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    if (value.is_top()) {
      return;
    }

    if (this->_equiv_relation.contains(x)) {
      EquivalenceClass& equiv_class = this->_equiv_relation.find_equiv_class(x);
      equiv_class.copy_domain();
      equiv_class.domain->refine(x, value);
      this->_is_normalized = false;
    } else {
      this->_equiv_relation.add_equiv_class(x);
      this->_equiv_relation.find_domain(x)->set(x, value);
    }
  }

  void refine(VariableRef x, const IntervalCongruenceT& value) override {
    if (this->_is_bottom) {
      return;
    }

    if (value.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    if (value.is_top()) {
      return;
    }

    if (this->_equiv_relation.contains(x)) {
      EquivalenceClass& equiv_class = this->_equiv_relation.find_equiv_class(x);
      equiv_class.copy_domain();
      equiv_class.domain->refine(x, value);
      this->_is_normalized = false;
    } else {
      this->_equiv_relation.add_equiv_class(x);
      this->_equiv_relation.find_domain(x)->set(x, value);
    }
  }

  void forget(VariableRef x) override {
    if (this->_is_bottom) {
      return;
    }

    auto result = this->_equiv_relation.forget(x);
    this->_is_bottom = (result == ForgetResult::Bottom);
    this->_is_normalized = false;
  }

private:
  /// \brief Forget the equivalence class containing `x`
  void forget_equiv_class(VariableRef x) {
    this->_equiv_relation.forget_equiv_class(x);
  }

public:
  IntervalT to_interval(VariableRef x) const override {
    if (this->_is_bottom) {
      return IntervalT::bottom();
    } else {
      if (this->_equiv_relation.contains(x)) {
        return this->_equiv_relation.cfind_domain(x)->to_interval(x);
      } else {
        return IntervalT::top();
      }
    }
  }

  IntervalT to_interval(const LinearExpressionT& e) const override {
    return Parent::to_interval(e);
  }

  CongruenceT to_congruence(VariableRef x) const override {
    if (this->_is_bottom) {
      return CongruenceT::bottom();
    } else {
      if (this->_equiv_relation.contains(x)) {
        return this->_equiv_relation.cfind_domain(x)->to_congruence(x);
      } else {
        return CongruenceT::top();
      }
    }
  }

  CongruenceT to_congruence(const LinearExpressionT& e) const override {
    return Parent::to_congruence(e);
  }

  IntervalCongruenceT to_interval_congruence(VariableRef x) const override {
    if (this->_is_bottom) {
      return IntervalCongruenceT::bottom();
    } else {
      if (this->_equiv_relation.contains(x)) {
        return this->_equiv_relation.cfind_domain(x)->to_interval_congruence(x);
      } else {
        return IntervalCongruenceT::top();
      }
    }
  }

  IntervalCongruenceT to_interval_congruence(
      const LinearExpressionT& e) const override {
    return Parent::to_interval_congruence(e);
  }

  LinearConstraintSystemT to_linear_constraint_system() const override {
    this->normalize();

    if (this->is_bottom()) {
      return LinearConstraintSystemT(LinearConstraintT::contradiction());
    }

    LinearConstraintSystemT csts;
    for (const auto& equiv_class : this->_equiv_relation) {
      csts.add(equiv_class.second.domain->to_linear_constraint_system());
    }

    return csts;
  }

  void dump(std::ostream& o) const override {
#if 1
    this->to_linear_constraint_system().dump(o);
#else
    this->_equiv_relation.dump(o);
#endif
  }

  static std::string name() {
    return Domain::name() + " with variable packing";
  }

}; // end class VarPackingDomain

} // end namespace numeric
} // end namespace core
} // end namespace ikos
