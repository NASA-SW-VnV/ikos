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
  using DomainPtr = std::shared_ptr< const Domain >;

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

  /*
   * Implementation of Union-Find
   */

  /// \brief Equivalence class
  ///
  /// Represents a 'pack' of variables
  class EquivalenceClass {
  private:
    std::size_t _rank;
    DomainPtr _domain_ptr;

    // Invariant: _domain_ptr is normalized

    // TODO(marthaud): We could store the list of variables in the class

  public:
    /// \brief Create an empty equivalence class
    EquivalenceClass()
        : _rank(0),
          _domain_ptr(std::make_shared< const Domain >(Domain::top())) {}

    /// \brief Create an equivalence class with the given domain
    EquivalenceClass(Domain domain)
        : _rank(0),
          _domain_ptr(std::make_shared< const Domain >(std::move(domain))) {}

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

    /// \brief Return the rank
    std::size_t rank() const { return this->_rank; }

    /// \brief Increment the rank
    void increment_rank() { this->_rank++; }

    /// \brief Return the abstract domain
    const Domain& domain() const { return *this->_domain_ptr; }

    /// \brief Update the abstract domain
    void set_domain(Domain domain) {
      domain.normalize();
      ikos_assert(!domain.is_bottom());
      this->_domain_ptr = std::make_shared< const Domain >(std::move(domain));
    }

    /// \brief Return the abstract domain pointer
    const DomainPtr& domain_ptr() const { return this->_domain_ptr; }

    /// \brief Update the abstract domain pointer
    void set_domain_ptr(const DomainPtr& domain_ptr) {
      ikos_assert(!domain_ptr->is_bottom());
      this->_domain_ptr = domain_ptr;
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

    /// \brief Create an equivalence class containing the given variable
    ///
    /// Precondition: `v` is not already present in the relation
    void add_equiv_class(VariableRef v, Domain domain) {
      ikos_assert_msg(!this->contains(v), "variable already present");
      this->_parents.emplace(v, v);
      this->_classes.emplace(v, EquivalenceClass(std::move(domain)));
    }

    /// \brief Add a variable in an equivalence class
    ///
    /// Precondition: `v` is not already present in the relation
    /// Precondition: `parent` is present in the relation
    void add_var_to_equiv_class(VariableRef v, VariableRef parent) {
      ikos_assert_msg(!this->contains(v), "variable already present");
      ikos_assert_msg(this->contains(parent), "variable missing");

      VariableRef parent_root = this->find_root_var(parent);
      EquivalenceClass& parent_class = this->_classes.find(parent_root)->second;

      if (parent_class.rank() == 0) {
        parent_class.increment_rank();
      }
      this->_parents.emplace(v, parent_root);
    }

    /// \brief Find the root of the equivalence class containing `v`
    VariableRef find_root_var(VariableRef v) {
      VariableRef& parent = this->_parents.find(v)->second;

      if (parent == v) {
        return v;
      } else {
        return parent = this->find_root_var(parent);
      }
    }

    /// \brief Find the root of the equivalence class containing `v`
    VariableRef cfind_root_var(VariableRef v) const {
      VariableRef parent = this->_parents.find(v)->second;

      if (parent == v) {
        return v;
      } else {
        return this->cfind_root_var(parent);
      }
    }

    /// \brief Find the equivalence class containing `v`
    EquivalenceClass& find_equiv_class(VariableRef v) {
      return this->_classes.find(this->find_root_var(v))->second;
    }

    /// \brief Find the equivalence class containing `v`
    const EquivalenceClass& cfind_equiv_class(VariableRef v) const {
      return this->_classes.find(this->cfind_root_var(v))->second;
    }

    /// \brief Find the abstract domain containing `v`
    const Domain& find_domain(VariableRef v) {
      return this->find_equiv_class(v).domain();
    }

    /// \brief Find the abstract domain containing `v`
    const Domain& cfind_domain(VariableRef v) const {
      return this->cfind_equiv_class(v).domain();
    }

    /// \brief Find the abstract domain pointer containing `v`
    DomainPtr find_domain_ptr(VariableRef v) {
      return this->find_equiv_class(v).domain_ptr();
    }

    /// \brief Find the abstract domain pointer containing `v`
    DomainPtr cfind_domain_ptr(VariableRef v) const {
      return this->cfind_equiv_class(v).domain_ptr();
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

      EquivalenceClass& x_class = this->_classes.find(x_root)->second;
      EquivalenceClass& y_class = this->_classes.find(y_root)->second;

      // Merge the abstract domains
      Domain domain = x_class.domain().meet(y_class.domain());

      if (x_class.rank() > y_class.rank()) {
        this->_parents.find(y_root)->second = x_root;

        x_class.set_domain(std::move(domain));
        this->_classes.erase(y_root);
      } else {
        this->_parents.find(x_root)->second = y_root;

        if (x_class.rank() == y_class.rank()) {
          y_class.increment_rank();
        }

        y_class.set_domain(std::move(domain));
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
    void forget(VariableRef v) {
      auto it = this->_parents.find(v);

      if (it == this->_parents.end()) {
        return;
      }

      if (it->second != v) {
        // v is not the root of the equivalence class
        VariableRef root = this->find_root_var(it->second);

        // Update parents
        for (auto& p : this->_parents) {
          if (p.second == v) {
            p.second = root;
          }
        }

        EquivalenceClass& equiv_class = this->_classes.find(root)->second;
        Domain domain = equiv_class.domain();
        domain.forget(v);
        equiv_class.set_domain(std::move(domain));
      } else {
        // v is the root of the equivalence class
        boost::optional< VariableRef > new_root;

        // Find a new root
        for (auto& p : this->_parents) {
          if (p.second == v && p.first != v) {
            if (!new_root) {
              new_root = p.first;
            }
            p.second = *new_root;
          }
        }

        if (new_root) {
          EquivalenceClass equiv_class =
              std::move(this->_classes.find(v)->second);
          Domain domain = equiv_class.domain();
          domain.forget(v);
          equiv_class.set_domain(std::move(domain));
          this->_classes.emplace(*new_root, std::move(equiv_class));
        }

        this->_classes.erase(v);
      }

      this->_parents.erase(v);
    }

    /// \brief Forget the equivalence class containing the given variable
    void forget_equiv_class(VariableRef v) {
      if (!this->contains(v)) {
        return;
      }

      VariableRef root = this->find_root_var(v);

      for (auto it = this->_parents.begin(); it != this->_parents.end();) {
        if (this->find_root_var(it->second) == root) {
          it = this->_parents.erase(it);
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
        it->second.domain().dump(o);
        ++it;
        if (it != et) {
          o << ", ";
        }
      }
      o << "})";
    }

  }; // end class EquivalenceRelation

private:
  EquivalenceRelation _equiv_relation;
  bool _is_bottom;

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top abstract value
  explicit VarPackingDomain(TopTag) : _is_bottom(false) {}

  /// \brief Create the bottom abstract value
  explicit VarPackingDomain(BottomTag) : _is_bottom(true) {}

public:
  /// \brief Create the top abstract value
  static VarPackingDomain top() { return VarPackingDomain(TopTag{}); }

  /// \brief Create the bottom abstract value
  static VarPackingDomain bottom() { return VarPackingDomain(BottomTag{}); }

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

  void normalize() override {}

  bool is_bottom() const override { return this->_is_bottom; }

  bool is_top() const override {
    if (this->_is_bottom) {
      return false;
    }

    for (const auto& equiv_class : this->_equiv_relation) {
      if (!equiv_class.second.domain().is_top()) {
        return false;
      }
    }

    return true;
  }

  void set_to_bottom() override {
    this->_equiv_relation.clear();
    this->_is_bottom = true;
  }

  void set_to_top() override {
    this->_equiv_relation.clear();
    this->_is_bottom = false;
  }

  bool leq(const VarPackingDomain& other) const override {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      RootVariablesMap other_roots = other._equiv_relation.root_to_vars();

      // For each equivalence class in `other`
      for (const auto& other_class : other_roots) {
        VariableRef other_root = other_class.first;
        DomainPtr other_domain_ptr =
            other._equiv_relation.cfind_domain_ptr(other_root);

        // Set of root variables of equivalence classes we have merged
        boost::container::flat_set< VariableRef > this_roots;

        // Domain for `this`, containing all needed variables
        DomainPtr this_domain_ptr = nullptr;

        // Merge domains in `this`
        for (VariableRef v : other_class.second) {
          if (!this->_equiv_relation.contains(v)) {
            continue; // v not in `this`
          }

          VariableRef this_root = this->_equiv_relation.cfind_root_var(v);

          if (!this_roots.insert(this_root).second) {
            continue; // Equivalence class for v already merged
          }

          DomainPtr domain_ptr =
              this->_equiv_relation.cfind_domain_ptr(this_root);

          // Merge `domain` into `this_domain`
          if (this_domain_ptr == nullptr) {
            this_domain_ptr = domain_ptr;
          } else if (this_domain_ptr == domain_ptr) {
            // Nothing to do
          } else {
            Domain domain = (*this_domain_ptr).meet(*domain_ptr);
            domain.normalize();
            this_domain_ptr =
                std::make_shared< const Domain >(std::move(domain));
          }
        }

        // Compare `this_domain_ptr` and `other_domain_ptr`
        if (this_domain_ptr == nullptr) {
          if (!other_domain_ptr->is_top()) {
            return false;
          }
        } else if (this_domain_ptr == other_domain_ptr) {
          // (*this_domain_ptr).leq(*other_domain_ptr) is true
        } else {
          if (!((*this_domain_ptr).leq(*other_domain_ptr))) {
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

  /// \brief Binary operation with a union semantic (join, widening)
  ///
  /// This uses a copy of `other` for ease of implementation.
  ///
  /// TODO(marthaud): Try to implement this without copying `other`
  template < typename BinaryOperator >
  void union_binary_op_with(VarPackingDomain other, const BinaryOperator& op) {
    // Forget variables in `this` that are not in `other`
    // TODO(marthaud): variables() needs to creates a temporary vector because
    // forget() might invalidate iterators
    for (VariableRef v : this->_equiv_relation.variables()) {
      if (!other._equiv_relation.contains(v)) {
        this->_equiv_relation.forget(v);
      }
    }

    // Forget variables in `other` that are not in `this`
    // TODO(marthaud): variables() needs to creates a temporary vector because
    // forget() might invalidate iterators
    for (VariableRef v : other._equiv_relation.variables()) {
      if (!this->_equiv_relation.contains(v)) {
        other._equiv_relation.forget(v);
      }
    }

    {
      // Iterate on equivalence classes in `this`, merge the variables in
      // `other`
      RootVariablesMap this_roots = this->_equiv_relation.root_to_vars();
      for (const auto& this_class : this_roots) {
        boost::optional< VariableRef > root;

        for (VariableRef v : this_class.second) {
          other.merge_existing_equiv_classes(root, v);
        }
      }
    }

    {
      // Iterator on equivalence classes in `other`, merge the variables in
      // `this` and compute the binary operation
      RootVariablesMap other_roots = other._equiv_relation.root_to_vars();
      for (const auto& other_class : other_roots) {
        const VariableRef& other_root = other_class.first;
        DomainPtr other_domain_ptr =
            other._equiv_relation.find_domain_ptr(other_root);

        boost::optional< VariableRef > root;
        for (VariableRef v : other_class.second) {
          this->merge_existing_equiv_classes(root, v);
        }

        if (root) {
          EquivalenceClass& equiv_class =
              this->_equiv_relation.find_equiv_class(*root);
          if (equiv_class.domain_ptr() == other_domain_ptr) {
            // Nothing to do, left and right packs are the same
          } else {
            equiv_class.set_domain(op(equiv_class.domain(), *other_domain_ptr));
          }
        }
      }
    }
  }

  /// \brief Binary operation with an intersection semantic (meet, narrowing)
  template < typename BinaryOperator >
  void meet_binary_op_with(const VarPackingDomain& other, BinaryOperator op) {
    RootVariablesMap other_roots = other._equiv_relation.root_to_vars();
    for (const auto& other_class : other_roots) {
      const VariableRef& other_root = other_class.first;
      DomainPtr other_domain_ptr =
          other._equiv_relation.cfind_domain_ptr(other_root);

      boost::optional< VariableRef > root;
      for (VariableRef v : other_class.second) {
        this->merge_existing_equiv_classes(root, v);
      }

      bool new_domain = !root;
      for (VariableRef v : other_class.second) {
        this->merge_unexisting_equiv_classes(root, v);
      }

      EquivalenceClass& equiv_class =
          this->_equiv_relation.find_equiv_class(*root);
      if (!new_domain) {
        if (equiv_class.domain_ptr() == other_domain_ptr) {
          // Nothing to do, left and right packs are the same
        } else {
          Domain domain = op(equiv_class.domain(), *other_domain_ptr);
          domain.normalize();
          if (domain.is_bottom()) {
            this->set_to_bottom();
            return;
          }
          equiv_class.set_domain(std::move(domain));
        }
      } else {
        equiv_class.set_domain_ptr(other_domain_ptr);
      }
    }
  }

  struct JoinOperator {
    Domain operator()(const Domain& left, const Domain& right) const {
      return left.join(right);
    }
  };

  struct MeetOperator {
    Domain operator()(const Domain& left, const Domain& right) const {
      return left.meet(right);
    }
  };

  struct WideningOperator {
    Domain operator()(const Domain& left, const Domain& right) const {
      return left.widening(right);
    }
  };

  struct WideningThresholdOperator {
    const Number& threshold;

    Domain operator()(const Domain& left, const Domain& right) const {
      return left.widening_threshold(right, threshold);
    }
  };

  struct NarrowingOperator {
    Domain operator()(const Domain& left, const Domain& right) const {
      return left.narrowing(right);
    }
  };

  struct NarrowingThresholdOperator {
    const Number& threshold;

    Domain operator()(const Domain& left, const Domain& right) const {
      return left.narrowing_threshold(right, threshold);
    }
  };

public:
  void join_with(VarPackingDomain&& other) override {
    if (this->is_bottom()) {
      this->operator=(std::move(other));
    } else if (other.is_bottom()) {
      return;
    } else {
      this->union_binary_op_with(std::move(other), JoinOperator{});
    }
  }

  void join_with(const VarPackingDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->union_binary_op_with(other, JoinOperator{});
    }
  }

  VarPackingDomain join(const VarPackingDomain& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      VarPackingDomain result = *this;
      result.union_binary_op_with(other, JoinOperator{});
      return result;
    }
  }

  void widen_with(const VarPackingDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->union_binary_op_with(other, WideningOperator{});
    }
  }

  VarPackingDomain widening(const VarPackingDomain& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      VarPackingDomain result = *this;
      result.union_binary_op_with(other, WideningOperator{});
      return result;
    }
  }

  void widen_threshold_with(const VarPackingDomain& other,
                            const Number& threshold) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->union_binary_op_with(other, WideningThresholdOperator{threshold});
    }
  }

  VarPackingDomain widening_threshold(const VarPackingDomain& other,
                                      const Number& threshold) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      VarPackingDomain result = *this;
      result.union_binary_op_with(other, WideningThresholdOperator{threshold});
      return result;
    }
  }

  void meet_with(const VarPackingDomain& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->meet_binary_op_with(other, MeetOperator{});
    }
  }

  VarPackingDomain meet(const VarPackingDomain& other) const override {
    if (this->is_bottom() || other.is_bottom()) {
      return VarPackingDomain::bottom();
    } else {
      VarPackingDomain result = *this;
      result.meet_binary_op_with(other, MeetOperator{});
      return result;
    }
  }

  void narrow_with(const VarPackingDomain& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->meet_binary_op_with(other, NarrowingOperator{});
    }
  }

  VarPackingDomain narrowing(const VarPackingDomain& other) const override {
    if (this->is_bottom() || other.is_bottom()) {
      return VarPackingDomain::bottom();
    } else {
      VarPackingDomain result = *this;
      result.meet_binary_op_with(other, NarrowingOperator{});
      return result;
    }
  }

  void narrow_threshold_with(const VarPackingDomain& other,
                             const Number& threshold) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->meet_binary_op_with(other, NarrowingThresholdOperator{threshold});
    }
  }

  VarPackingDomain narrowing_threshold(const VarPackingDomain& other,
                                       const Number& threshold) const override {
    if (this->is_bottom() || other.is_bottom()) {
      return VarPackingDomain::bottom();
    } else {
      VarPackingDomain result = *this;
      result.meet_binary_op_with(other, NarrowingThresholdOperator{threshold});
      return result;
    }
  }

  void assign(VariableRef x, int n) override { this->assign(x, Number(n)); }

  void assign(VariableRef x, const Number& n) override {
    if (this->is_bottom()) {
      return;
    }

    this->_equiv_relation.forget(x);
    Domain domain = Domain::top();
    domain.assign(x, n);
    this->_equiv_relation.add_equiv_class(x, std::move(domain));
  }

  void assign(VariableRef x, VariableRef y) override {
    if (this->is_bottom()) {
      return;
    }

    if (x == y) {
      return;
    }

    if (!this->_equiv_relation.contains(y)) {
      this->_equiv_relation.add_equiv_class(y);
    }

    this->_equiv_relation.forget(x);
    this->_equiv_relation.add_var_to_equiv_class(x, y);
    EquivalenceClass& equiv_class = this->_equiv_relation.find_equiv_class(y);
    Domain domain = equiv_class.domain();
    domain.assign(x, y);
    equiv_class.set_domain(std::move(domain));
  }

  void assign(VariableRef x, const LinearExpressionT& e) override {
    if (this->is_bottom()) {
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
      this->_equiv_relation.forget(x);
      this->_equiv_relation.add_var_to_equiv_class(x, *root);
    }

    // Otherwise, x has already been merged

    EquivalenceClass& equiv_class =
        this->_equiv_relation.find_equiv_class(*root);
    Domain domain = equiv_class.domain();
    domain.assign(x, e);
    equiv_class.set_domain(std::move(domain));
  }

private:
  /// \brief Add a relation x = f(y, z)
  EquivalenceClass& add_relation(VariableRef x, VariableRef y, VariableRef z) {
    boost::optional< VariableRef > root;
    this->merge_existing_equiv_classes(root, y);
    this->merge_existing_equiv_classes(root, z);
    this->merge_unexisting_equiv_classes(root, y);
    this->merge_unexisting_equiv_classes(root, z);

    if (x != y && x != z) {
      this->_equiv_relation.forget(x);
      this->_equiv_relation.add_var_to_equiv_class(x, *root);
    }
    // Otherwise, x has already been merged

    return this->_equiv_relation.find_equiv_class(*root);
  }

  /// \brief Add a relation x = f(y)
  EquivalenceClass& add_relation(VariableRef x, VariableRef y) {
    if (!this->_equiv_relation.contains(y)) {
      this->_equiv_relation.add_equiv_class(y);
    }

    if (x != y) {
      this->_equiv_relation.forget(x);
      this->_equiv_relation.add_var_to_equiv_class(x, y);
    }

    return this->_equiv_relation.find_equiv_class(y);
  }

public:
  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             VariableRef z) override {
    if (this->is_bottom()) {
      return;
    }

    EquivalenceClass& equiv_class = this->add_relation(x, y, z);
    Domain domain = equiv_class.domain();
    domain.apply(op, x, y, z);
    domain.normalize();
    if (domain.is_bottom()) {
      this->set_to_bottom();
    } else {
      equiv_class.set_domain(std::move(domain));
    }
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             const Number& z) override {
    if (this->is_bottom()) {
      return;
    }

    EquivalenceClass& equiv_class = this->add_relation(x, y);
    Domain domain = equiv_class.domain();
    domain.apply(op, x, y, z);
    domain.normalize();
    if (domain.is_bottom()) {
      this->set_to_bottom();
    } else {
      equiv_class.set_domain(std::move(domain));
    }
  }

  void apply(BinaryOperator op,
             VariableRef x,
             const Number& y,
             VariableRef z) override {
    if (this->is_bottom()) {
      return;
    }

    EquivalenceClass& equiv_class = add_relation(x, z);
    Domain domain = equiv_class.domain();
    domain.apply(op, x, y, z);
    domain.normalize();
    if (domain.is_bottom()) {
      this->set_to_bottom();
    } else {
      equiv_class.set_domain(std::move(domain));
    }
  }

  void add(const LinearConstraintT& cst) override {
    if (this->is_bottom()) {
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
    Domain domain = equiv_class.domain();
    domain.add(cst);
    domain.normalize();
    if (domain.is_bottom()) {
      this->set_to_bottom();
    } else {
      equiv_class.set_domain(std::move(domain));
    }
  }

  void add(const LinearConstraintSystemT& csts) override {
    for (const LinearConstraintT& cst : csts) {
      this->add(cst);
    }
  }

  void set(VariableRef x, const IntervalT& value) override {
    if (this->is_bottom()) {
      return;
    }

    if (value.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_equiv_relation.forget(x);

    if (value.is_top()) {
      return;
    }

    Domain domain = Domain::top();
    domain.set(x, value);
    this->_equiv_relation.add_equiv_class(x, std::move(domain));
  }

  void set(VariableRef x, const CongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    }

    if (value.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_equiv_relation.forget(x);

    if (value.is_top()) {
      return;
    }

    Domain domain = Domain::top();
    domain.set(x, value);
    this->_equiv_relation.add_equiv_class(x, std::move(domain));
  }

  void set(VariableRef x, const IntervalCongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    }

    if (value.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    this->_equiv_relation.forget(x);

    if (value.is_top()) {
      return;
    }

    Domain domain = Domain::top();
    domain.set(x, value);
    this->_equiv_relation.add_equiv_class(x, std::move(domain));
  }

  void refine(VariableRef x, const IntervalT& value) override {
    if (this->is_bottom()) {
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
      Domain domain = equiv_class.domain();
      domain.refine(x, value);
      domain.normalize();
      if (domain.is_bottom()) {
        this->set_to_bottom();
      } else {
        equiv_class.set_domain(std::move(domain));
      }
    } else {
      Domain domain = Domain::top();
      domain.set(x, value);
      this->_equiv_relation.add_equiv_class(x, std::move(domain));
    }
  }

  void refine(VariableRef x, const CongruenceT& value) override {
    if (this->is_bottom()) {
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
      Domain domain = equiv_class.domain();
      domain.refine(x, value);
      domain.normalize();
      if (domain.is_bottom()) {
        this->set_to_bottom();
      } else {
        equiv_class.set_domain(std::move(domain));
      }
    } else {
      Domain domain = Domain::top();
      domain.set(x, value);
      this->_equiv_relation.add_equiv_class(x, std::move(domain));
    }
  }

  void refine(VariableRef x, const IntervalCongruenceT& value) override {
    if (this->is_bottom()) {
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
      Domain domain = equiv_class.domain();
      domain.refine(x, value);
      domain.normalize();
      if (domain.is_bottom()) {
        this->set_to_bottom();
      } else {
        equiv_class.set_domain(std::move(domain));
      }
    } else {
      Domain domain = Domain::top();
      domain.set(x, value);
      this->_equiv_relation.add_equiv_class(x, std::move(domain));
    }
  }

  void forget(VariableRef x) override {
    if (this->is_bottom()) {
      return;
    }

    this->_equiv_relation.forget(x);
  }

private:
  /// \brief Forget the equivalence class containing `x`
  void forget_equiv_class(VariableRef x) {
    this->_equiv_relation.forget_equiv_class(x);
  }

public:
  IntervalT to_interval(VariableRef x) const override {
    if (this->is_bottom()) {
      return IntervalT::bottom();
    } else {
      if (this->_equiv_relation.contains(x)) {
        return this->_equiv_relation.cfind_domain(x).to_interval(x);
      } else {
        return IntervalT::top();
      }
    }
  }

  IntervalT to_interval(const LinearExpressionT& e) const override {
    return Parent::to_interval(e);
  }

  CongruenceT to_congruence(VariableRef x) const override {
    if (this->is_bottom()) {
      return CongruenceT::bottom();
    } else {
      if (this->_equiv_relation.contains(x)) {
        return this->_equiv_relation.cfind_domain(x).to_congruence(x);
      } else {
        return CongruenceT::top();
      }
    }
  }

  CongruenceT to_congruence(const LinearExpressionT& e) const override {
    return Parent::to_congruence(e);
  }

  IntervalCongruenceT to_interval_congruence(VariableRef x) const override {
    if (this->is_bottom()) {
      return IntervalCongruenceT::bottom();
    } else {
      if (this->_equiv_relation.contains(x)) {
        return this->_equiv_relation.cfind_domain(x).to_interval_congruence(x);
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
    if (this->is_bottom()) {
      return LinearConstraintSystemT(LinearConstraintT::contradiction());
    }

    LinearConstraintSystemT csts;
    for (const auto& equiv_class : this->_equiv_relation) {
      csts.add(equiv_class.second.domain().to_linear_constraint_system());
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
