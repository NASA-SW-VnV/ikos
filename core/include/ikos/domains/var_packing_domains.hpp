/**************************************************************************/ /**
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

#ifndef IKOS_VAR_PACKING_DOMAINS_HPP
#define IKOS_VAR_PACKING_DOMAINS_HPP

#include <unordered_map>
#include <unordered_set>

#include <ikos/common/types.hpp>
#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/domains/bitwise_operators_api.hpp>
#include <ikos/domains/division_operators_api.hpp>
#include <ikos/domains/intervals.hpp>
#include <ikos/domains/numerical_domains_api.hpp>
#include <ikos/value/congruence.hpp>
#include <ikos/value/interval.hpp>

namespace ikos {

// forward declaration for var_packing_dbm
template < typename Number,
           typename VariableName,
           std::size_t max_reduction_cycles >
class var_packing_dbm;

// forward declaration for var_packing_dbm_congruence
template < typename Number,
           typename VariableName,
           std::size_t max_reduction_cycles,
           int typeSize >
class var_packing_dbm_congruence;

/// \brief Generic abstract domain with variable packing
///
/// The idea is to keep a set of equivalence classes, using the union-find
/// structure. Each equivalence class holds an abstract domain over all
/// variables within the class.
template < typename Number, typename VariableName, typename Domain >
class var_packing_domain : public abstract_domain,
                           public numerical_domain< Number, VariableName >,
                           public bitwise_operators< Number, VariableName >,
                           public division_operators< Number, VariableName > {
public:
  typedef variable< Number, VariableName > variable_t;
  typedef patricia_tree_set< variable_t > variable_set_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;
  typedef interval< Number > interval_t;
  typedef congruence< Number > congruence_t;
  typedef interval_domain< Number, VariableName > interval_domain_t;
  typedef var_packing_domain< Number, VariableName, Domain >
      var_packing_domain_t;

  template < typename A, typename B, std::size_t C >
  friend class var_packing_dbm;

  template < typename A, typename B, std::size_t C, int D >
  friend class var_packing_dbm_congruence;

private:
  typedef std::shared_ptr< Domain > domain_ptr_t;
  typedef std::unordered_map< VariableName, std::vector< VariableName > >
      classes_vars_t;

private:
  // Implementation of Union-Find

  class equivalence_relation;

  class equivalence_class {
  private:
    std::size_t _rank;
    domain_ptr_t _domain;

    // Note: the reason why sometimes we explicitly call domain->normalize() is
    // because it's better to normalize a (probably) shared domain. If we don't
    // do that, some methods are gonna normalize a copy.

  public:
    explicit equivalence_class() : _rank(0), _domain(new Domain()) {}

    const domain_ptr_t& domain() { return _domain; }

    void set_domain(const domain_ptr_t& domain) { _domain = domain; }

    // Copy before a write
    void copy_domain() { _domain.reset(new Domain(*_domain)); }

    friend class equivalence_relation;
  }; // end class equivalence_class

  class equivalence_relation : public writeable {
  private:
    typedef std::unordered_map< VariableName, VariableName > parents_map_t;
    typedef std::unordered_map< VariableName, equivalence_class > classes_map_t;

  public:
    typedef typename classes_map_t::iterator iterator;
    typedef typename classes_map_t::const_iterator const_iterator;

  private:
    parents_map_t _parents; // variable => parent
    classes_map_t _classes; // root variable => equivalence class

    // Note: we are using _parents.at(key) instead of _parents[key] because
    // VariableName may not have a default constructor

  public:
    bool contains(const VariableName& v) const {
      return _parents.find(v) != _parents.end();
    }

    void add_equiv_class(const VariableName& v) {
      assert(!contains(v));
      _parents.insert(typename parents_map_t::value_type(v, v));
      _classes.insert(
          typename classes_map_t::value_type(v, equivalence_class()));
    }

    void add_var_to_equiv_class(const VariableName& v,
                                const VariableName& parent) {
      assert(!contains(v) && contains(parent));
      const VariableName& parent_root = find_root_var(parent);
      equivalence_class& parent_class = _classes[parent_root];

      if (parent_class._rank == 0) {
        parent_class._rank++;
      }
      _parents.insert(typename parents_map_t::value_type(v, parent_root));
    }

    /// \brief Find
    const VariableName& find_root_var(const VariableName& v) {
      VariableName& parent = _parents.at(v);
      if (parent == v) {
        return v;
      } else {
        return parent = find_root_var(parent);
      }
    }

    equivalence_class& find_equiv_class(const VariableName& v) {
      return _classes[find_root_var(v)];
    }

    const domain_ptr_t& find_domain(const VariableName& v) {
      return find_equiv_class(v).domain();
    }

    /// \brief Union
    bool join_equiv_class(const VariableName& x, const VariableName& y) {
      const VariableName& x_root = find_root_var(x);
      const VariableName& y_root = find_root_var(y);

      if (x_root != y_root) {
        equivalence_class& x_class = _classes[x_root];
        equivalence_class& y_class = _classes[y_root];

        // Merge the domains
        domain_ptr_t merge_domain(new Domain());
        x_class._domain->normalize();
        y_class._domain->normalize();
        *merge_domain = (*x_class._domain) & (*y_class._domain);

        if (x_class._rank > y_class._rank) {
          _parents.at(y_root) = x_root;

          x_class._domain.swap(merge_domain);
          _classes.erase(y_root);
        } else {
          _parents.at(x_root) = y_root;
          if (x_class._rank == y_class._rank) {
            y_class._rank++;
          }

          y_class._domain.swap(merge_domain);
          _classes.erase(x_root);
        }

        return true;
      }

      return false;
    }

    // iterate on equivalence classes
    iterator begin() { return _classes.begin(); }
    const_iterator begin() const { return _classes.begin(); }

    iterator end() { return _classes.end(); }
    const_iterator end() const { return _classes.end(); }

    void clear() {
      _parents.clear();
      _classes.clear();
    }

    variable_set_t variables() const {
      variable_set_t variables;
      for (typename parents_map_t::const_iterator it = _parents.begin();
           it != _parents.end();
           ++it) {
        variables += it->first;
      }
      return variables;
    }

    void forget(const VariableName& v) {
      if (contains(v)) {
        if (_parents.at(v) != v) {
          // v is not the root of the equivalence class
          const VariableName& root = find_root_var(v);

          for (typename parents_map_t::iterator it = _parents.begin();
               it != _parents.end();
               ++it) {
            if (it->second == v) {
              _parents.at(it->first) = root;
            }
          }

          equivalence_class& equiv_class = _classes[root];
          equiv_class.copy_domain();
          *equiv_class.domain() -= v;
        } else {
          // v is the root of the equivalence class
          boost::optional< VariableName > new_root;

          for (typename parents_map_t::iterator it = _parents.begin();
               it != _parents.end();
               ++it) {
            if (it->first != v && it->second == v) {
              if (!new_root) {
                new_root = it->first;
                _classes[*new_root] = _classes[v];
              }
              _parents.at(it->first) = *new_root;
            }
          }

          if (new_root) {
            equivalence_class& equiv_class = _classes[*new_root];
            equiv_class.copy_domain();
            *equiv_class.domain() -= v;
          }
          _classes.erase(v);
        }

        _parents.erase(v);
      }
    }

    void forget_equiv_class(const VariableName& v) {
      if (contains(v)) {
        const VariableName& root = find_root_var(v);
        for (typename parents_map_t::iterator it = _parents.begin();
             it != _parents.end();) {
          if (find_root_var(it->second) == root) {
            it = _parents.erase(it);
          } else {
            ++it;
          }
        }
        _classes.erase(root);
      }
    }

    /// \returns a map root_var => list of variables in that equivalence class
    classes_vars_t equiv_classes_vars() {
      classes_vars_t classes;
      for (typename parents_map_t::iterator it = _parents.begin();
           it != _parents.end();
           ++it) {
        classes[find_root_var(it->second)].push_back(it->first);
      }
      return classes;
    }

    void write(std::ostream& o) {
      o << "({";
      for (typename parents_map_t::iterator it = _parents.begin();
           it != _parents.end();) {
        VariableName key = it->first, value = it->second;
        o << key << " -> " << value;
        ++it;
        if (it != _parents.end()) {
          o << ", ";
        }
      }
      o << "}, {";
      for (typename classes_map_t::iterator it = _classes.begin();
           it != _classes.end();) {
        VariableName key = it->first;
        const domain_ptr_t& domain = it->second.domain();
        o << key << " -> " << *domain;
        ++it;
        if (it != _classes.end()) {
          o << ", ";
        }
      }
      o << "})";
    }

  }; // end class equivalence_relation

private:
  bool _is_bottom;
  bool _is_normalized;
  equivalence_relation _equiv_relation;

private:
  var_packing_domain(bool is_top) : _is_bottom(!is_top), _is_normalized(true) {}

  void set_bottom() {
    _is_bottom = true;
    _is_normalized = true;
    _equiv_relation.clear();
  }

  void normalize() {
    if (_is_normalized)
      return;

    if (_is_bottom) {
      set_bottom();
      _is_normalized = true;
      return;
    }

    for (typename equivalence_relation::iterator it = _equiv_relation.begin();
         it != _equiv_relation.end();
         ++it) {
      const domain_ptr_t& domain = it->second.domain();
      domain->normalize();

      if (domain->is_bottom()) {
        set_bottom();
        _is_normalized = true;
        return;
      }
    }

    _is_normalized = true;
  }

  void merge_existing_equiv_classes(boost::optional< VariableName >& root,
                                    const VariableName& v) {
    if (_equiv_relation.contains(v)) {
      if (root) {
        _equiv_relation.join_equiv_class(v, *root);
      } else {
        root = v;
      }
    }
  }

  void merge_unexisting_equiv_classes(boost::optional< VariableName >& root,
                                      const VariableName& v) {
    if (!_equiv_relation.contains(v)) {
      if (root) {
        _equiv_relation.add_var_to_equiv_class(v, *root);
      } else {
        root = v;
        _equiv_relation.add_equiv_class(v);
      }
    }
  }

  /// \brief Add a relation x = f(y, z)
  const domain_ptr_t& add_relation(const VariableName& x,
                                   const VariableName& y,
                                   const VariableName& z) {
    boost::optional< VariableName > root;
    merge_existing_equiv_classes(root, y);
    merge_existing_equiv_classes(root, z);
    merge_unexisting_equiv_classes(root, y);
    merge_unexisting_equiv_classes(root, z);

    if (x != y && x != z) {
      forget(x);
      _equiv_relation.add_var_to_equiv_class(x, *root);
    }
    // otherwise, x has already been merged

    equivalence_class& equiv_class = _equiv_relation.find_equiv_class(*root);
    equiv_class.copy_domain();
    return equiv_class.domain();
  }

  /// \brief Add a relation x = f(y)
  const domain_ptr_t& add_relation(const VariableName& x,
                                   const VariableName& y) {
    if (!_equiv_relation.contains(y)) {
      _equiv_relation.add_equiv_class(y);
    }

    if (x != y) {
      forget(x);
      _equiv_relation.add_var_to_equiv_class(x, y);
    }

    equivalence_class& equiv_class = _equiv_relation.find_equiv_class(y);
    equiv_class.copy_domain();
    return equiv_class.domain();
  }

public:
  congruence_t to_congruence(VariableName x) {
    if (_is_bottom) {
      return congruence_t::bottom();
    } else {
      if (_equiv_relation.contains(x)) {
        return num_domain_traits::to_congruence(*_equiv_relation.find_domain(x),
                                                x);
      } else {
        return congruence_t::top();
      }
    }
  }

  interval_t to_interval(VariableName x, bool normalize) {
    if (_is_bottom) {
      return interval_t::bottom();
    } else {
      if (_equiv_relation.contains(x)) {
        return num_domain_traits::to_interval(*_equiv_relation.find_domain(x),
                                              x,
                                              normalize);
      } else {
        return interval_t::top();
      }
    }
  }

  interval_t to_interval(linear_expression_t e, bool normalize) {
    if (_is_bottom) {
      return interval_t::bottom();
    } else {
      interval_t result(e.constant());

      for (typename linear_expression_t::iterator it = e.begin(); it != e.end();
           ++it) {
        interval_t c(it->first);
        result += c * to_interval(it->second.name(), normalize);
      }

      return result;
    }
  }

private:
  /// \brief Binary operation using a union semantic (join, widening)
  template < typename op_t >
  var_packing_domain_t union_binary_op(var_packing_domain_t& o) const {
    // Note: o is a copy, thus we can update it to compute the result
    op_t op;
    var_packing_domain_t result_domain(*this);

    // forget variables in result_domain that are not in o
    variable_set_t result_vars = result_domain.variables();
    for (typename variable_set_t::iterator it = result_vars.begin();
         it != result_vars.end();
         ++it) {
      VariableName v = (*it).name();
      if (!o._equiv_relation.contains(v)) {
        result_domain.forget(v);
      }
    }

    // forget variables in o that are not in result_domain
    variable_set_t o_vars = o.variables();
    for (typename variable_set_t::iterator it = o_vars.begin();
         it != o_vars.end();
         ++it) {
      VariableName v = (*it).name();
      if (!result_domain._equiv_relation.contains(v)) {
        o.forget(v);
      }
    }

    // Go through all equivalence classes in result, merge the variables in o
    classes_vars_t result_classes =
        result_domain._equiv_relation.equiv_classes_vars();
    for (typename classes_vars_t::iterator c_it = result_classes.begin();
         c_it != result_classes.end();
         ++c_it) {
      const std::vector< VariableName >& result_vars = c_it->second;

      boost::optional< VariableName > root;
      for (typename std::vector< VariableName >::const_iterator v_it =
               result_vars.begin();
           v_it != result_vars.end();
           ++v_it) {
        o.merge_existing_equiv_classes(root, *v_it);
      }
    }

    // Go through all equivalence classes in o, merge the variables in result
    // and compute the binary operation
    classes_vars_t o_classes = o._equiv_relation.equiv_classes_vars();
    for (typename classes_vars_t::iterator c_it = o_classes.begin();
         c_it != o_classes.end();
         ++c_it) {
      const VariableName& o_root = c_it->first;
      const std::vector< VariableName >& o_vars = c_it->second;
      const domain_ptr_t& o_domain = o._equiv_relation.find_domain(o_root);

      boost::optional< VariableName > root;
      for (typename std::vector< VariableName >::const_iterator v_it =
               o_vars.begin();
           v_it != o_vars.end();
           ++v_it) {
        result_domain.merge_existing_equiv_classes(root, *v_it);
      }

      if (root) {
        equivalence_class& equiv_class =
            result_domain._equiv_relation.find_equiv_class(*root);
        domain_ptr_t merge_domain(new Domain());
        op(*merge_domain, *equiv_class.domain(), *o_domain);
        equiv_class.set_domain(merge_domain);
      }
    }

    result_domain._is_normalized = false;
    return result_domain;
  }

  /// \brief Binary operation using an intersection semantic (meet, narrowing)
  template < typename op_t >
  var_packing_domain_t meet_binary_op(var_packing_domain_t& o) const {
    op_t op;
    var_packing_domain_t result_domain(*this);

    classes_vars_t o_classes = o._equiv_relation.equiv_classes_vars();
    for (typename classes_vars_t::iterator c_it = o_classes.begin();
         c_it != o_classes.end();
         ++c_it) {
      const VariableName& o_root = c_it->first;
      const std::vector< VariableName >& o_vars = c_it->second;
      const domain_ptr_t& o_domain = o._equiv_relation.find_domain(o_root);

      boost::optional< VariableName > root;
      for (typename std::vector< VariableName >::const_iterator v_it =
               o_vars.begin();
           v_it != o_vars.end();
           ++v_it) {
        result_domain.merge_existing_equiv_classes(root, *v_it);
      }

      bool new_domain = !root;
      for (typename std::vector< VariableName >::const_iterator v_it =
               o_vars.begin();
           v_it != o_vars.end();
           ++v_it) {
        result_domain.merge_unexisting_equiv_classes(root, *v_it);
      }

      equivalence_class& equiv_class =
          result_domain._equiv_relation.find_equiv_class(*root);
      if (!new_domain) {
        domain_ptr_t merge_domain(new Domain());
        op(*merge_domain, *equiv_class.domain(), *o_domain);
        equiv_class.set_domain(merge_domain);
      } else {
        equiv_class.set_domain(o_domain);
      }
    }

    result_domain._is_normalized = false;
    return result_domain;
  }

  // Helpers for lattice operations
  struct join_op {
    void operator()(Domain& result, Domain& left, Domain& right) {
      left.normalize();
      right.normalize();
      result = left | right;
    }
  };

  struct meet_op {
    void operator()(Domain& result, Domain& left, Domain& right) {
      left.normalize();
      right.normalize();
      result = left & right;
    }
  };

  struct widening_op {
    void operator()(Domain& result, Domain& left, Domain& right) {
      right.normalize();
      result = left || right;
    }
  };

  struct narrowing_op {
    void operator()(Domain& result, Domain& left, Domain& right) {
      left.normalize();
      right.normalize();
      result = left && right;
    }
  };

public:
  static var_packing_domain_t top() { return var_packing_domain(true); }

  static var_packing_domain_t bottom() { return var_packing_domain(false); }

public:
  var_packing_domain() : _is_bottom(false), _is_normalized(true) {}

  var_packing_domain(const var_packing_domain_t& o)
      : _is_bottom(o._is_bottom),
        _is_normalized(o._is_normalized),
        _equiv_relation(o._equiv_relation) {}

  var_packing_domain_t& operator=(const var_packing_domain_t& o) {
    _is_bottom = o._is_bottom;
    _is_normalized = o._is_normalized;
    _equiv_relation = o._equiv_relation;
    return *this;
  }

  bool is_bottom() {
    normalize();
    return _is_bottom;
  }

  bool is_top() {
    // Does not require normalization
    if (_is_bottom) {
      return false;
    }

    for (typename equivalence_relation::iterator it = _equiv_relation.begin();
         it != _equiv_relation.end();
         ++it) {
      const domain_ptr_t& domain = it->second.domain();
      if (!domain->is_top()) {
        return false;
      }
    }

    return true;
  }

  variable_set_t variables() const { return _equiv_relation.variables(); }

  /// \brief Convert an interval_domain to a var_packing_domain
  explicit var_packing_domain(interval_domain_t o)
      : _is_bottom(o.is_bottom()), _is_normalized(true) {
    if (o.is_bottom())
      return;

    for (typename interval_domain_t::iterator it = o.begin(); it != o.end();
         ++it) {
      _equiv_relation.add_equiv_class(it->first);
      _equiv_relation.find_domain(it->first)->set(it->first, it->second);
    }
  }

  /// \brief Convert a var_packing_domain to an interval_domain
  interval_domain_t get_interval_domain() {
    // Requires normalization
    normalize();

    if (is_bottom()) {
      return interval_domain_t::bottom();
    }

    interval_domain_t result;
    variable_set_t vars = _equiv_relation.variables();
    for (typename variable_set_t::iterator it = vars.begin(); it != vars.end();
         ++it) {
      VariableName v = (*it).name();
      result.set(v, _equiv_relation.find_domain(v)->to_interval(v, false));
    }

    return result;
  }

  bool operator<=(var_packing_domain_t o) {
    // Requires normalization
    normalize();
    o.normalize();

    if (is_bottom()) {
      return true;
    } else if (o.is_bottom()) {
      return false;
    } else {
      classes_vars_t o_classes = o._equiv_relation.equiv_classes_vars();

      // For each equivalence class in o
      for (typename classes_vars_t::iterator c_it = o_classes.begin();
           c_it != o_classes.end();
           ++c_it) {
        const VariableName& o_root = c_it->first;
        const std::vector< VariableName >& o_vars = c_it->second;
        const domain_ptr_t& o_domain = o._equiv_relation.find_domain(o_root);

        // the set of root variables of equivalence classes we have merged
        std::unordered_set< VariableName > this_roots;

        // the domain for `this`, containing all needed variables
        domain_ptr_t this_domain;

        // We need to merge all domains containing a variable in o_vars
        for (typename std::vector< VariableName >::const_iterator v_it =
                 o_vars.begin();
             v_it != o_vars.end();
             ++v_it) {
          const VariableName& o_var = *v_it;

          if (this->_equiv_relation.contains(o_var)) {
            const VariableName& this_root =
                this->_equiv_relation.find_root_var(o_var);

            if (this_roots.find(this_root) == this_roots.end()) {
              this_roots.insert(this_root);
              const domain_ptr_t& domain =
                  this->_equiv_relation.find_domain(this_root);
              if (this_domain == NULL) {
                this_domain = domain;
              } else {
                domain_ptr_t merge_domain(new Domain());
                this_domain->normalize();
                domain->normalize();
                *merge_domain = (*this_domain) & (*domain);
                this_domain.swap(merge_domain);
              }
            }
          }
        }

        if (this_domain != NULL) {
          this_domain->normalize();
          o_domain->normalize();
          if (!(*this_domain <= *o_domain)) {
            return false;
          }
        } else {
          if (o_domain->is_top()) {
            o.forget_equiv_class(o_root);
          } else {
            return false;
          }
        }
      }

      return true;
    }
  }

  /// \brief Join
  var_packing_domain_t operator|(var_packing_domain_t o) {
    // Requires normalization
    normalize();
    o.normalize();

    if (is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      return union_binary_op< join_op >(o);
    }
  }

  /// \brief Widening
  var_packing_domain_t operator||(var_packing_domain_t o) {
    // Requires the normalization of the right operand.
    // The left operand (this) should not be normalized.
    o.normalize();

    if (_is_bottom) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    } else {
      return union_binary_op< widening_op >(o);
    }
  }

  var_packing_domain_t join_loop(var_packing_domain_t o) {
    return this->operator|(o);
  }

  var_packing_domain_t join_iter(var_packing_domain_t o) {
    return this->operator|(o);
  }

  /// \brief Meet
  var_packing_domain_t operator&(var_packing_domain_t o) {
    // Requires normalization
    normalize();
    o.normalize();

    if (this->is_bottom() || o.is_bottom()) {
      return var_packing_domain_t::bottom();
    } else {
      return meet_binary_op< meet_op >(o);
    }
  }

  /// \brief Narrowing
  var_packing_domain_t operator&&(var_packing_domain_t o) {
    // Requires normalization
    normalize();
    o.normalize();

    if (this->is_bottom() || o.is_bottom()) {
      return var_packing_domain_t::bottom();
    } else {
      return meet_binary_op< narrowing_op >(o);
    }
  }

  void operator-=(VariableName v) { forget_num(v); }

  void forget(VariableName v) { forget_num(v); }

  void forget_num(VariableName v) {
    _equiv_relation.forget(v);
    _is_normalized = false;
  }

private:
  void forget_equiv_class(VariableName v) {
    _equiv_relation.forget_equiv_class(v);
    _is_normalized = false;
  }

public:
  template < typename Iterator >
  void forget(Iterator begin, Iterator end) {
    forget_num(begin, end);
  }

  /// \brief Remove a set of variables
  template < typename Iterator >
  void forget_num(Iterator begin, Iterator end) {
    for (auto it = begin; it != end; ++it) {
      forget_num(*it);
    }
  }

  void operator+=(linear_constraint_t cst) {
    if (_is_bottom)
      return;

    if (cst.size() == 0) {
      if (cst.is_contradiction()) {
        set_bottom();
      }
      return;
    }

    boost::optional< VariableName > root;
    for (typename linear_expression_t::iterator it = cst.begin();
         it != cst.end();
         ++it) {
      merge_existing_equiv_classes(root, it->second.name());
    }

    for (typename linear_expression_t::iterator it = cst.begin();
         it != cst.end();
         ++it) {
      merge_unexisting_equiv_classes(root, it->second.name());
    }

    equivalence_class& equiv_class = _equiv_relation.find_equiv_class(*root);
    equiv_class.copy_domain();
    *equiv_class.domain() += cst;
    _is_normalized = false;
  }

  void operator+=(linear_constraint_system_t cst) {
    for (typename linear_constraint_system_t::iterator it = cst.begin();
         it != cst.end();
         ++it) {
      this->operator+=(*it);
    }
  }

  void set(VariableName x, interval_t intv) {
    if (_is_bottom)
      return;

    if (intv.is_bottom()) {
      set_bottom();
      return;
    }

    forget(x);
    if (intv.is_top())
      return;

    _equiv_relation.add_equiv_class(x);
    _equiv_relation.find_domain(x)->set(x, intv);
    _is_normalized = false;
  }

  void refine(VariableName x, interval_t intv) {
    if (_is_bottom)
      return;

    if (intv.is_bottom()) {
      set_bottom();
      return;
    }

    if (intv.is_top())
      return;

    if (_equiv_relation.contains(x)) {
      equivalence_class& equiv_class = _equiv_relation.find_equiv_class(x);
      equiv_class.copy_domain();
      equiv_class.domain()->refine(x, intv);
      _is_normalized = false;
    } else {
      _equiv_relation.add_equiv_class(x);
      _equiv_relation.find_domain(x)->set(x, intv);
    }
  }

  void assign(VariableName x, linear_expression_t e) {
    if (_is_bottom)
      return;

    if (e.is_constant()) {
      forget(x);
      _equiv_relation.add_equiv_class(x);
      _equiv_relation.find_domain(x)->assign(x, e);
      _is_normalized = false;
      return;
    }

    boost::optional< VariableName > root;

    for (typename linear_expression_t::iterator it = e.begin(); it != e.end();
         ++it) {
      merge_existing_equiv_classes(root, it->second.name());
    }

    for (typename linear_expression_t::iterator it = e.begin(); it != e.end();
         ++it) {
      merge_unexisting_equiv_classes(root, it->second.name());
    }

    if (e[x] == 0) {
      forget(x);
      _equiv_relation.add_var_to_equiv_class(x, *root);
    }
    // otherwise, x has already been merged

    equivalence_class& equiv_class = _equiv_relation.find_equiv_class(*root);
    equiv_class.copy_domain();
    equiv_class.domain()->assign(x, e);
    _is_normalized = false;
  }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    if (_is_bottom)
      return;

    add_relation(x, y, z)->apply(op, x, y, z);
    _is_normalized = false;
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    if (_is_bottom)
      return;

    add_relation(x, y)->apply(op, x, y, k);
    _is_normalized = false;
  }

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             uint64_t from,
             uint64_t to) {
    if (_is_bottom)
      return;

    add_relation(x, y)->apply(op, x, y, from, to);
    _is_normalized = false;
  }

  void apply(conv_operation_t op,
             VariableName x,
             Number k,
             uint64_t from,
             uint64_t to) {
    if (_is_bottom)
      return;

    forget(x);
    _equiv_relation.add_equiv_class(x);
    _equiv_relation.find_domain(x)->apply(op, x, k, from, to);
    _is_normalized = false;
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    if (_is_bottom)
      return;

    add_relation(x, y, z)->apply(op, x, y, z);
    _is_normalized = false;
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    if (_is_bottom)
      return;

    add_relation(x, y)->apply(op, x, y, k);
    _is_normalized = false;
  }

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    if (_is_bottom)
      return;

    add_relation(x, y, z)->apply(op, x, y, z);
    _is_normalized = false;
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    if (_is_bottom)
      return;

    add_relation(x, y)->apply(op, x, y, k);
    _is_normalized = false;
  }

  linear_constraint_system_t to_linear_constraint_system() {
    normalize();
    linear_constraint_system_t csts;

    if (is_bottom()) {
      csts += linear_constraint_t::contradiction();
      return csts;
    }

    for (typename equivalence_relation::iterator it = _equiv_relation.begin();
         it != _equiv_relation.end();
         ++it) {
      const domain_ptr_t& domain = it->second.domain();
      csts += domain->to_linear_constraint_system();
    }

    return csts;
  }

  void write(std::ostream& o) {
#if 1
    linear_constraint_system_t csts = to_linear_constraint_system();
    o << csts;
#else
    _equiv_relation.write(o);
#endif
  }

  static std::string domain_name() {
    return Domain::domain_name() + " with Variable Packing";
  }

}; // end class var_packing_domain

namespace num_domain_traits {
namespace detail {

template < typename Number, typename VariableName, typename Domain >
struct normalize_impl< var_packing_domain< Number, VariableName, Domain > > {
  inline void operator()(
      var_packing_domain< Number, VariableName, Domain >& inv) {
    return inv.normalize();
  }
};

template < typename Number, typename VariableName, typename Domain >
struct convert_impl< var_packing_domain< Number, VariableName, Domain >,
                     interval_domain< Number, VariableName > > {
  inline interval_domain< Number, VariableName > operator()(
      var_packing_domain< Number, VariableName, Domain > inv) {
    return inv.get_interval_domain();
  }
};

template < typename Number, typename VariableName, typename Domain >
struct convert_impl< interval_domain< Number, VariableName >,
                     var_packing_domain< Number, VariableName, Domain > > {
  inline var_packing_domain< Number, VariableName, Domain > operator()(
      interval_domain< Number, VariableName > inv) {
    return var_packing_domain< Number, VariableName, Domain >(inv);
  }
};

template < typename Number, typename VariableName, typename Domain >
struct var_to_interval_impl<
    var_packing_domain< Number, VariableName, Domain > > {
  inline interval< Number > operator()(
      var_packing_domain< Number, VariableName, Domain >& inv,
      VariableName v,
      bool normalize) {
    return inv.to_interval(v, normalize);
  }
};

template < typename Number, typename VariableName, typename Domain >
struct lin_expr_to_interval_impl<
    var_packing_domain< Number, VariableName, Domain > > {
  inline interval< Number > operator()(
      var_packing_domain< Number, VariableName, Domain >& inv,
      linear_expression< Number, VariableName > e,
      bool normalize) {
    return inv.to_interval(e, normalize);
  }
};

} // end namespace detail
} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_VAR_PACKING_DOMAINS_HPP
