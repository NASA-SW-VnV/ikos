/**************************************************************************/ /**
 *
 * \file
 * \brief Reduced product of dbm (using variable packing) and congruences.
 *
 * The implementation is based on Jorge's interval_congruence_domain.
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

#ifndef IKOS_VAR_PACKING_DBM_CONGRUENCES_HPP
#define IKOS_VAR_PACKING_DBM_CONGRUENCES_HPP

#include <ikos/common/types.hpp>
#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/domains/bitwise_operators_api.hpp>
#include <ikos/domains/division_operators_api.hpp>
#include <ikos/domains/domain_products.hpp>
#include <ikos/domains/intervals_congruences.hpp>
#include <ikos/domains/numerical_domains_api.hpp>
#include <ikos/domains/var_packing_dbm.hpp>
#include <ikos/value/congruence.hpp>
#include <ikos/value/interval.hpp>

namespace ikos {

/// \brief Reduced product of DBM (using variable packing) and congruences
template < typename Number,
           typename VariableName,
           std::size_t max_reduction_cycles = 10,
           int TypeSize = -1 >
class var_packing_dbm_congruence
    : public abstract_domain,
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
  typedef congruence< Number, TypeSize > congruence_t;
  typedef var_packing_dbm< Number, VariableName, max_reduction_cycles >
      var_packing_dbm_t;
  typedef congruence_domain< Number, VariableName, TypeSize >
      congruence_domain_t;
  typedef interval_congruence< Number, TypeSize > interval_congruence_t;
  typedef interval_congruence_domain< Number, VariableName, TypeSize >
      interval_congruence_domain_t;
  typedef var_packing_dbm_congruence< Number,
                                      VariableName,
                                      max_reduction_cycles,
                                      TypeSize > var_packing_dbm_congruence_t;

private:
  typedef numerical_domain_product2< Number,
                                     VariableName,
                                     var_packing_dbm_t,
                                     congruence_domain_t > domain_product_t;
  typedef std::shared_ptr< dbm< Number, VariableName > > dbm_ptr_t;
  typedef typename var_packing_dbm_t::var_packing_domain_t::equivalence_relation
      equivalence_relation_t;

private:
  domain_product_t _product;

private:
  var_packing_dbm_congruence(const domain_product_t& product)
      : _product(product) {}

  /// \brief Reduce the equivalence class containing the variable `v`
  ///
  /// Does not normalize the entire domain.
  ///
  /// need_copy is false if we know that a copy has already been made.
  void reduce_equivalence_class(const VariableName& v, bool need_copy = true) {
    if (_product.first()._domain._is_bottom || _product.second().is_bottom())
      return;

    equivalence_relation_t& equiv_relation =
        _product.first()._domain._equiv_relation;
    if (!equiv_relation.contains(v))
      return;

    dbm_ptr_t subdomain = equiv_relation.find_domain(v);
    variable_set_t variables = subdomain->variables();

    bool first_update = true;
    bool change_dbm = true;
    bool change_congruence = true;
    std::size_t num_iters = 0;

    subdomain->normalize();

    while ((change_dbm || change_congruence) &&
           num_iters < max_reduction_cycles) {
      change_dbm = false;
      change_congruence = false;

      for (typename variable_set_t::iterator it = variables.begin();
           it != variables.end();
           ++it) {
        interval_t i = subdomain->to_interval(*it, false);
        congruence_t c = _product.second()[*it];
        interval_congruence_t val(i, c);

        if (val.is_bottom()) {
          *this = bottom();
          return;
        } else {
          if (val.first() != i) {
            if (need_copy && first_update) {
              // need to copy the subdomain before updating it
              equiv_relation.find_equiv_class(v).copy_domain();
              subdomain = equiv_relation.find_domain(v);
              first_update = false;
            }

            *subdomain += within_interval(*it, val.first());
            change_dbm = true;
          }

          if (val.second() != c) {
            _product.second().set((*it).name(), val.second());
            change_congruence = true;
          }
        }
      }

      if (change_dbm) {
        subdomain->normalize();
      }

      num_iters++;
    }
  }

public:
  static var_packing_dbm_congruence_t top() {
    return var_packing_dbm_congruence_t(domain_product_t::top());
  }

  static var_packing_dbm_congruence_t bottom() {
    return var_packing_dbm_congruence_t(domain_product_t::bottom());
  }

public:
  var_packing_dbm_congruence() : _product() {}

  var_packing_dbm_congruence(const var_packing_dbm_congruence_t& o)
      : _product(o._product) {}

  var_packing_dbm_congruence_t& operator=(
      const var_packing_dbm_congruence_t& o) {
    _product = o._product;
    return *this;
  }

  bool is_bottom() { return _product.is_bottom(); }

  bool is_top() { return _product.is_top(); }

  variable_set_t variables() {
    variable_set_t vars = _product.first().variables();

    // congruence_domain::variables() is not defined
    for (typename congruence_domain_t::iterator it = _product.second().begin();
         it != _product.second().end();
         ++it) {
      vars += it->first;
    }

    return vars;
  }

  /// \brief Convert an interval_congruence_domain to a
  /// var_packing_dbm_congruence
  explicit var_packing_dbm_congruence(interval_congruence_domain_t o)
      : _product(var_packing_dbm_t(o.first()), o.second()) {}

  /// \brief Convert a var_packing_dbm_congruence to an
  /// interval_congruence_domain
  interval_congruence_domain_t get_interval_congruence_domain() {
    return interval_congruence_domain_t(_product.first().get_interval_domain(),
                                        _product.second());
  }

  var_packing_dbm_t& first() { return _product.first(); }

  congruence_domain_t& second() { return _product.second(); }

  bool operator<=(var_packing_dbm_congruence_t o) {
    return _product <= o._product;
  }

  var_packing_dbm_congruence_t operator|(var_packing_dbm_congruence_t o) {
    return var_packing_dbm_congruence_t(_product | o._product);
  }

  var_packing_dbm_congruence_t operator||(var_packing_dbm_congruence_t o) {
    return var_packing_dbm_congruence_t(_product || o._product);
  }

  var_packing_dbm_congruence_t join_loop(var_packing_dbm_congruence_t o) {
    return var_packing_dbm_congruence_t(_product.join_loop(o._product));
  }

  var_packing_dbm_congruence_t join_iter(var_packing_dbm_congruence_t o) {
    return var_packing_dbm_congruence_t(_product.join_iter(o._product));
  }

  var_packing_dbm_congruence_t operator&(var_packing_dbm_congruence_t o) {
    return var_packing_dbm_congruence_t(_product & o._product);
  }

  var_packing_dbm_congruence_t operator&&(var_packing_dbm_congruence_t o) {
    return var_packing_dbm_congruence_t(_product && o._product);
  }

  void set(VariableName v, interval_congruence_t x) {
    _product.first().set(v, x.first());
    _product.second().set(v, x.second());
  }

  congruence_t to_congruence(VariableName v) { return _product.second()[v]; }

  interval_t to_interval(VariableName v, bool normalize) {
    interval_congruence_t ic(_product.first().to_interval(v, normalize),
                             _product.second()[v]);
    return ic.first();
  }

  interval_t to_interval(linear_expression_t e, bool normalize) {
    interval_congruence_t ic(_product.first().to_interval(e, normalize),
                             _product.second()[e]);
    return ic.first();
  }

  interval_congruence_t operator[](VariableName v) {
    return interval_congruence_t(_product.first().to_interval(v, true),
                                 _product.second()[v]);
  }

  void operator+=(linear_constraint_t cst) {
    _product += cst;

    if (_product.first()._domain._is_bottom || _product.second().is_bottom())
      return;

    if (cst.size() == 0)
      return;

    typename linear_expression_t::iterator it = cst.begin();
    typename linear_expression_t::iterator it2 = ++cst.begin();

    if ((cst.is_inequality() || cst.is_equality()) &&
        ((cst.size() == 1 && it->first == 1) ||
         (cst.size() == 1 && it->first == -1) ||
         (cst.size() == 2 && it->first == 1 && it2->first == -1) ||
         (cst.size() == 2 && it->first == -1 && it2->first == 1))) {
      // variables are together in the same equivalence class
      reduce_equivalence_class(it->second.name(), false);
    } else {
      for (auto it = cst.begin(); it != cst.end(); ++it) {
        reduce_equivalence_class(it->second.name());
      }
    }
  }

  void operator+=(linear_constraint_system_t csts) {
    _product += csts;

    if (_product.first()._domain._is_bottom || _product.second().is_bottom())
      return;

    for (linear_constraint_t cst : csts) {
      if (cst.size() == 0) {
        continue;
      } else {
        typename linear_expression_t::iterator it = cst.begin();
        typename linear_expression_t::iterator it2 = ++cst.begin();

        if ((cst.is_inequality() || cst.is_equality()) &&
            ((cst.size() == 1 && it->first == 1) ||
             (cst.size() == 1 && it->first == -1) ||
             (cst.size() == 2 && it->first == 1 && it2->first == -1) ||
             (cst.size() == 2 && it->first == -1 && it2->first == 1))) {
          // variables are together in the same equivalence class
          reduce_equivalence_class(it->second.name(), false);
        } else {
          for (auto it = cst.begin(); it != cst.end(); ++it) {
            reduce_equivalence_class(it->second.name());
          }
        }
      }
    }
  }

  void operator-=(VariableName v) { forget_num(v); }

  void forget(VariableName v) { forget_num(v); }

  void forget_num(VariableName v) { _product.forget_num(v); }

  template < typename Iterator >
  void forget(Iterator begin, Iterator end) {
    forget_num(begin, end);
  }

  template < typename Iterator >
  void forget_num(Iterator begin, Iterator end) {
    _product.forget_num(begin, end);
  }

  void assign(VariableName x, linear_expression_t e) {
    _product.assign(x, e);
    reduce_equivalence_class(x, false);
  }

  void apply(operation_t op, VariableName x, VariableName y, VariableName z) {
    _product.apply(op, x, y, z);
    reduce_equivalence_class(x, false);
  }

  void apply(operation_t op, VariableName x, VariableName y, Number k) {
    _product.apply(op, x, y, k);
    reduce_equivalence_class(x, false);
  }

  // bitwise_operators_api

  void apply(conv_operation_t op,
             VariableName x,
             VariableName y,
             uint64_t from,
             uint64_t to) {
    _product.apply(op, x, y, from, to);
    reduce_equivalence_class(x, false);
  }

  void apply(conv_operation_t op,
             VariableName x,
             Number k,
             uint64_t from,
             uint64_t to) {
    _product.apply(op, x, k, from, to);
    reduce_equivalence_class(x, false);
  }

  void apply(bitwise_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    _product.apply(op, x, y, z);
    reduce_equivalence_class(x, false);
  }

  void apply(bitwise_operation_t op, VariableName x, VariableName y, Number k) {
    _product.apply(op, x, y, k);
    reduce_equivalence_class(x, false);
  }

  // division_operators_api

  void apply(div_operation_t op,
             VariableName x,
             VariableName y,
             VariableName z) {
    _product.apply(op, x, y, z);
    reduce_equivalence_class(x, false);
  }

  void apply(div_operation_t op, VariableName x, VariableName y, Number k) {
    _product.apply(op, x, y, k);
    reduce_equivalence_class(x, false);
  }

  void write(std::ostream& o) { _product.write(o); }

  linear_constraint_system_t to_linear_constraint_system() {
    return _product.first().to_linear_constraint_system();
  }

  static std::string domain_name() {
    return "DBM with Variable Packing + Congruences";
  }

}; // end class var_packing_dbm_congruence

namespace num_domain_traits {
namespace detail {

template < typename Number,
           typename VariableName,
           std::size_t max_reduction_cycles,
           int TypeSize >
struct var_to_interval_impl< var_packing_dbm_congruence< Number,
                                                         VariableName,
                                                         max_reduction_cycles,
                                                         TypeSize > > {
  inline interval< Number > operator()(
      var_packing_dbm_congruence< Number,
                                  VariableName,
                                  max_reduction_cycles,
                                  TypeSize >& inv,
      VariableName v,
      bool normalize) {
    return inv.to_interval(v, normalize);
  }
};

template < typename Number,
           typename VariableName,
           std::size_t max_reduction_cycles,
           int TypeSize >
struct lin_expr_to_interval_impl<
    var_packing_dbm_congruence< Number,
                                VariableName,
                                max_reduction_cycles,
                                TypeSize > > {
  inline interval< Number > operator()(
      var_packing_dbm_congruence< Number,
                                  VariableName,
                                  max_reduction_cycles,
                                  TypeSize >& inv,
      linear_expression< Number, VariableName > e,
      bool normalize) {
    return inv.to_interval(e, normalize);
  }
};

template < typename Number,
           typename VariableName,
           std::size_t max_reduction_cycles,
           int TypeSize >
struct from_interval_impl< var_packing_dbm_congruence< Number,
                                                       VariableName,
                                                       max_reduction_cycles,
                                                       TypeSize > > {
  inline void operator()(var_packing_dbm_congruence< Number,
                                                     VariableName,
                                                     max_reduction_cycles,
                                                     TypeSize >& inv,
                         VariableName v,
                         interval< Number > i) {
    interval_congruence< Number > ic(i);
    inv.set(v, ic);
  }
};

template < typename Number,
           typename VariableName,
           std::size_t max_reduction_cycles,
           int TypeSize >
struct convert_impl<
    interval_congruence_domain< Number, VariableName, TypeSize >,
    var_packing_dbm_congruence< Number,
                                VariableName,
                                max_reduction_cycles,
                                TypeSize > > {
  inline var_packing_dbm_congruence< Number,
                                     VariableName,
                                     max_reduction_cycles,
                                     TypeSize >
  operator()(interval_congruence_domain< Number, VariableName, TypeSize > inv) {
    return var_packing_dbm_congruence< Number, VariableName >(inv);
  }
};

template < typename Number,
           typename VariableName,
           std::size_t max_reduction_cycles,
           int TypeSize >
struct convert_impl<
    var_packing_dbm_congruence< Number,
                                VariableName,
                                max_reduction_cycles,
                                TypeSize >,
    interval_congruence_domain< Number, VariableName, TypeSize > > {
  inline interval_congruence_domain< Number, VariableName, TypeSize >
  operator()(var_packing_dbm_congruence< Number,
                                         VariableName,
                                         max_reduction_cycles,
                                         TypeSize > inv) {
    return inv.get_interval_congruence_domain();
  }
};

} // end namespace detail
} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_VAR_PACKING_DBM_CONGRUENCES_HPP
