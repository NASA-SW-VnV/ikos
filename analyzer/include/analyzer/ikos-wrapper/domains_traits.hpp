/*******************************************************************************
 *
 * Extend IKOS abstract domains with some extra functionality.
 *
 * Author: Jorge A. Navas
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2016 United States Government as represented by the
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

#ifndef ANALYZER_DOMAINS_TRAITS_HPP
#define ANALYZER_DOMAINS_TRAITS_HPP

#include <analyzer/analysis/common.hpp>

#include <ikos/domains/intervals.hpp>
#include <ikos/domains/congruences.hpp>
#include <ikos/domains/intervals_congruences.hpp>
#include <ikos/domains/octagons.hpp>
#include <ikos/domains/var_packing_dbm_congruences.hpp>

namespace ikos {

namespace num_abstract_domain_impl {

template < typename AbsDomain >
struct is_octagon {
  static const bool value = false;
};

template <>
struct is_octagon< octagon< z_number, analyzer::varname_t > > {
  static const bool value = true;
};

template < typename AbsDomain >
struct is_interval {
  static const bool value = false;
};

template <>
struct is_interval< interval_domain< z_number, analyzer::varname_t > > {
  static const bool value = true;
};

template < typename AbsDomain >
struct is_congruence {
  static const bool value = false;
};

template <>
struct is_congruence< congruence_domain< z_number, analyzer::varname_t > > {
  static const bool value = true;
};

template < typename AbsDomain >
struct is_interval_with_congruence {
  static const bool value = false;
};

template <>
struct is_interval_with_congruence<
    interval_congruence_domain< z_number, analyzer::varname_t > > {
  static const bool value = true;
};

} // end namespace num_abstract_domain_impl

namespace num_abstract_domain_impl {

/// The checking phase done by the analyses must convert the generic
/// type abs_num_domain_t to integer intervals.

// Default implementation
template < typename AbsNumDomain, typename VariableName >
inline z_interval to_interval(AbsNumDomain inv, VariableName v) {
  return inv[v];
}

// Default implementation
template < typename AbsNumDomain, typename VariableName >
inline void from_interval(AbsNumDomain& inv, VariableName v, z_interval i) {
  inv.set(v, i);
}

// Specialized version
template <>
inline z_interval to_interval(
    congruence_domain< z_number, analyzer::varname_t > inv,
    analyzer::varname_t v) {
  boost::optional< z_number > n = inv[v].singleton();
  if (n)
    return z_interval(*n);
  else
    return z_interval::top();
}

// Specialized version
template <>
inline z_interval to_interval(
    interval_congruence_domain< z_number, analyzer::varname_t > inv,
    analyzer::varname_t v) {
  return inv[v].first();
}

// Specialized version
template <>
inline z_interval to_interval(
    var_packing_dbm_congruence< z_number, analyzer::varname_t > inv,
    analyzer::varname_t v) {
  return inv[v].first();
}

// Specialized version
template <>
inline void from_interval(
    congruence_domain< z_number, analyzer::varname_t >& inv,
    analyzer::varname_t v,
    z_interval val) {
  boost::optional< z_number > n = val.singleton();
  if (n)
    inv.set(v, *n);
}

// Specialized version
template <>
inline void from_interval(
    interval_congruence_domain< z_number, analyzer::varname_t >& inv,
    analyzer::varname_t v,
    z_interval val) {
  interval_congruence< z_number > c(val);
  inv.set(v, c);
}

// Specialized version
template <>
inline void from_interval(
    var_packing_dbm_congruence< z_number, analyzer::varname_t >& inv,
    analyzer::varname_t v,
    z_interval val) {
  interval_congruence< z_number > c(val);
  inv.set(v, c);
}

/// Some numerical domains are not in closed form by default. Some
/// analyses must enforce that for avoiding losing precision.

// Default implementation
template < typename AbsNumDomain >
void normalize(AbsNumDomain& inv) {}

// Specialized version
template <>
void normalize(octagon< z_number, analyzer::varname_t >& inv) {
  inv.normalize();
}

/// Convert an abstract domain to another one

// Default implementation, using method to_linear_constraint_system()
template < typename AbsNumDomainA, typename AbsNumDomainB >
AbsNumDomainB convert(AbsNumDomainA inv_a) {
  if (inv_a.is_bottom()) {
    return AbsNumDomainB::bottom();
  } else {
    typedef typename AbsNumDomainA::linear_constraint_system_t
        linear_constraint_system_t;
    typedef typename linear_constraint_system_t::iterator
        linear_constraint_system_iterator_t;

    AbsNumDomainB inv_b;
    linear_constraint_system_t csts = inv_a.to_linear_constraint_system();
    for (linear_constraint_system_iterator_t it = csts.begin();
         it != csts.end();
         ++it) {
      inv_b += *it;
    }
    return inv_b;
  }
}

template < typename AbsNumDomain >
AbsNumDomain convert(AbsNumDomain inv) {
  return inv;
}

template <>
var_packing_dbm< z_number, analyzer::varname_t > convert(
    interval_domain< z_number, analyzer::varname_t > inv) {
  return var_packing_dbm< z_number, analyzer::varname_t >(inv);
}

template <>
interval_domain< z_number, analyzer::varname_t > convert(
    var_packing_dbm< z_number, analyzer::varname_t > inv) {
  return inv.get_interval_domain();
}

template <>
var_packing_dbm_congruence< z_number, analyzer::varname_t > convert(
    interval_congruence_domain< z_number, analyzer::varname_t > inv) {
  return var_packing_dbm_congruence< z_number, analyzer::varname_t >(inv);
}

template <>
interval_congruence_domain< z_number, analyzer::varname_t > convert(
    var_packing_dbm_congruence< z_number, analyzer::varname_t > inv) {
  return inv.get_interval_congruence_domain();
}

} // end namespace num_abstract_domain_impl

} // end namespace ikos

#endif // ANALYZER_DOMAINS_TRAITS_HPP
