/**************************************************************************/ /**
 *
 * \file
 * \brief Generic API for numerical domains.
 *
 * Author: Arnaud J. Venet
 *
 * Contributors:
 *   * Maxime Arthaud
 *   * Jorge A. Navas
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

#ifndef IKOS_NUMERICAL_DOMAINS_API_HPP
#define IKOS_NUMERICAL_DOMAINS_API_HPP

#include <ikos/algorithms/linear_constraints.hpp>
#include <ikos/common/types.hpp>
#include <ikos/value/congruence.hpp>
#include <ikos/value/interval.hpp>

namespace ikos {

/// \brief Base class for numerical abstract domains
template < typename Number, typename VariableName >
class numerical_domain {
public:
  typedef Number number_t;
  typedef VariableName variable_name_t;
  typedef linear_expression< Number, VariableName > linear_expression_t;
  typedef linear_constraint< Number, VariableName > linear_constraint_t;
  typedef linear_constraint_system< Number, VariableName >
      linear_constraint_system_t;

public:
  /// \brief Assign x = e
  virtual void assign(VariableName x, linear_expression_t e) = 0;

  /// \brief Apply x = y op z
  virtual void apply(operation_t op,
                     VariableName x,
                     VariableName y,
                     VariableName z) = 0;

  /// \brief Apply x = y op k
  virtual void apply(operation_t op,
                     VariableName x,
                     VariableName y,
                     Number k) = 0;

  /// \brief Add a linear constraint
  virtual void operator+=(linear_constraint_t cst) {
    linear_constraint_system_t csts(cst);
    operator+=(csts);
  }

  /// \brief Add a linear constraint system
  virtual void operator+=(linear_constraint_system_t csts) = 0;

  /// \brief Forget a numerical variable
  virtual void operator-=(VariableName v) = 0;

  /// \brief Forget a numerical variable
  virtual void forget(VariableName v) = 0;

  /// \brief Forget a numerical variable
  virtual void forget_num(VariableName v) = 0;

  // forget numerical variables
  // template < typename Iterator >
  // void forget(Iterator begin, Iterator end);
  // template < typename Iterator >
  // void forget_num(Iterator begin, Iterator end);

  /// \returns a set of linear constraints
  virtual linear_constraint_system_t to_linear_constraint_system() = 0;

  virtual ~numerical_domain() {}

}; // end class numerical_domain

namespace num_domain_traits {

/*
 * Type traits to add functionalities to numerical domains.
 *
 * NOTE: All calls are forwarded to a method call within a template class.
 * This is because we need partial template specialization, and this is only
 * available for classes.
 */

namespace detail {

// default implementation
template < typename NumDomain >
struct normalize_impl {
  inline void operator()(NumDomain& /*inv*/) {}
};

} // end namespace detail

/// \brief Normalize the numerical domain
template < typename NumDomain >
inline void normalize(NumDomain& inv) {
  detail::normalize_impl< NumDomain >()(inv);
}

namespace detail {

// default implementation, using to_linear_constraint_system()
template < typename NumDomainFrom, typename NumDomainTo >
struct convert_impl {
  NumDomainTo operator()(NumDomainFrom inv_from) {
    if (inv_from.is_bottom()) {
      return NumDomainTo::bottom();
    } else {
      NumDomainTo inv_to = NumDomainTo::top();
      inv_to += inv_from.to_linear_constraint_system();
      return inv_to;
    }
  }
};

// implementation for convert< X, X >
template < typename NumDomain >
struct convert_impl< NumDomain, NumDomain > {
  NumDomain operator()(NumDomain inv) { return inv; }
};

} // end namespace detail

/// \brief convert the numerical domain from type NumDomainFrom to NumDomainTo
template < typename NumDomainFrom, typename NumDomainTo >
inline NumDomainTo convert(NumDomainFrom inv) {
  return detail::convert_impl< NumDomainFrom, NumDomainTo >()(inv);
}

namespace detail {

// default implementation, using operator[]
template < typename NumDomain >
struct var_to_interval_impl {
  inline interval< typename NumDomain::number_t > operator()(
      NumDomain& inv, typename NumDomain::variable_name_t v, bool normalize) {
    if (normalize) {
      num_domain_traits::normalize(inv);
    }

    return inv[v];
  }
};

// default implementation, using to_interval() on each variable
template < typename NumDomain >
struct lin_expr_to_interval_impl {
  inline interval< typename NumDomain::number_t > operator()(
      NumDomain& inv,
      typename NumDomain::linear_expression_t e,
      bool normalize) {
    typedef interval< typename NumDomain::number_t > interval_t;

    if (normalize) {
      num_domain_traits::normalize(inv);
    }

    if (inv.is_bottom()) {
      return interval_t::bottom();
    }

    interval_t r(e.constant());
    for (auto it = e.begin(); it != e.end(); ++it) {
      interval_t c(it->first);
      r += c * detail::var_to_interval_impl< NumDomain >()(inv,
                                                           it->second.name(),
                                                           false);
    }

    return r;
  }
};

// VariableName to Congruence default implementation
template < typename NumDomain >
struct var_to_congruence_impl {
  inline congruence< typename NumDomain::number_t > operator()(
      NumDomain& inv, typename NumDomain::variable_name_t v) {
    return inv.to_congruence(v);
  };
};

// default implementation, using set()
template < typename NumDomain >
struct from_interval_impl {
  inline void operator()(NumDomain& inv,
                         typename NumDomain::variable_name_t v,
                         interval< typename NumDomain::number_t > i) {
    inv.set(v, i);
  }
};

// default implementation, using refine()
template < typename NumDomain >
struct refine_interval_impl {
  inline void operator()(NumDomain& inv,
                         typename NumDomain::variable_name_t v,
                         interval< typename NumDomain::number_t > i) {
    inv.refine(v, i);
  }
};

} // end namespace detail

/// \brief Helper to get the congruence approximation of a variable
///
/// \returns an overapproximation of the congruence of the variable v
template < typename NumDomain >
inline congruence< typename NumDomain::number_t > to_congruence(
    NumDomain& inv, typename NumDomain::variable_name_t v) {
  return detail::var_to_congruence_impl< NumDomain >()(inv, v);
}

/// \brief Helper to get the interval approximation of a variable
///
/// If normalize is true, normalizes the abstract domain.
///
/// \returns an overapproximation of the value of the variable v
template < typename NumDomain >
inline interval< typename NumDomain::number_t > to_interval(
    NumDomain& inv,
    typename NumDomain::variable_name_t v,
    bool normalize = true) {
  return detail::var_to_interval_impl< NumDomain >()(inv, v, normalize);
}

/// \brief Helper to get the interval approximation of a linear expression
///
/// If normalize is true, normalizes the abstract domain.
///
/// \returns an overapproximation of the value of the linear expression e
template < typename NumDomain >
inline interval< typename NumDomain::number_t > to_interval(
    NumDomain& inv,
    typename NumDomain::linear_expression_t e,
    bool normalize = true) {
  return detail::lin_expr_to_interval_impl< NumDomain >()(inv, e, normalize);
}

/// \brief Helper to define a new variable
///
/// It should be equivalent to:
/// \code{.cpp}
/// inv -= v;
/// inv += (v <= i.ub());
/// inv += (v >= i.lb());
/// \endcode
template < typename NumDomain >
inline void from_interval(NumDomain& inv,
                          typename NumDomain::variable_name_t v,
                          interval< typename NumDomain::number_t > i) {
  detail::from_interval_impl< NumDomain >()(inv, v, i);
}

/// \brief Helper to refine the value of a variable
///
/// It should be equivalent to:
/// \code{.cpp}
/// inv += (v <= i.ub());
/// inv += (v >= i.lb());
/// \endcode
template < typename NumDomain >
inline void refine_interval(NumDomain& inv,
                            typename NumDomain::variable_name_t v,
                            interval< typename NumDomain::number_t > i) {
  detail::refine_interval_impl< NumDomain >()(inv, v, i);
}

} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_NUMERICAL_DOMAINS_API_HPP
