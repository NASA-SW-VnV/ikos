/*******************************************************************************
 *
 * Generic API for numerical domains.
 *
 * Author: Arnaud J. Venet
 *
 * Contributors: Maxime Arthaud
 *               Jorge A. Navas
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

#ifndef IKOS_NUMERICAL_DOMAINS_API_HPP
#define IKOS_NUMERICAL_DOMAINS_API_HPP

#include <ikos/common/types.hpp>
#include <ikos/algorithms/linear_constraints.hpp>

namespace ikos {

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
  // assign x = e
  virtual void assign(VariableName x, linear_expression_t e) = 0;

  // apply x = y op z
  virtual void apply(operation_t op,
                     VariableName x,
                     VariableName y,
                     VariableName z) = 0;

  // apply x = y op k
  virtual void apply(operation_t op,
                     VariableName x,
                     VariableName y,
                     Number k) = 0;

  // add a linear constraint
  virtual void operator+=(linear_constraint_t cst) {
    linear_constraint_system_t csts(cst);
    operator+=(csts);
  }

  // add a linear constraint system
  virtual void operator+=(linear_constraint_system_t csts) = 0;

  // forget a variable
  virtual void operator-=(VariableName v) = 0;

  // return a set of linear constraints
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
  void operator()(NumDomain& /*inv*/) {}
};

} // end namespace detail

// normalize the numerical domain
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

// convert the numerical domain from type NumDomainFrom to NumDomainTo
template < typename NumDomainFrom, typename NumDomainTo >
inline NumDomainTo convert(NumDomainFrom inv) {
  return detail::convert_impl< NumDomainFrom, NumDomainTo >()(inv);
}

// see intervals.hpp for declaration of to_interval() and from_interval()

} // end namespace num_domain_traits

} // end namespace ikos

#endif // IKOS_NUMERICAL_DOMAINS_API_HPP
