/**************************************************************************/ /**
 *
 * \file
 * \brief Generic API for abstract domains that keep track of (non-)null
 * variables.
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

#ifndef IKOS_NULLITY_DOMAINS_API_HPP
#define IKOS_NULLITY_DOMAINS_API_HPP

#include <type_traits>

#include <ikos/common/types.hpp>

namespace ikos {

/// \brief Base class for abstract domains supporting null variables
template < typename VariableName >
class nullity_domain {
public:
  typedef VariableName variable_name_t;

public:
  /// \brief Mark v as null
  virtual void make_null(VariableName v) = 0;

  /// \brief Mark v as non null
  virtual void make_non_null(VariableName v) = 0;

  /// \brief Assign x = y
  virtual void assign_nullity(VariableName x, VariableName y) = 0;

  /// \brief Add constraint: x == null
  virtual void assert_null(VariableName v) = 0;

  /// \brief Add constraint: x != null
  virtual void assert_non_null(VariableName v) = 0;

  /// \brief Add constraint (x == y) or (x != y)
  virtual void assert_nullity(bool equality,
                              VariableName x,
                              VariableName y) = 0;

  /// \returns true if v is null, otherwise false
  virtual bool is_null(VariableName v) = 0;

  /// \returns true if v is non null, otherwise false
  virtual bool is_non_null(VariableName v) = 0;

  /// \brief Forget a variable
  virtual void operator-=(VariableName v) = 0;

  /// \brief Forget a variable
  virtual void forget(VariableName v) = 0;

  /// \brief Forget a variable
  virtual void forget_nullity(VariableName v) = 0;

  // forget variables
  // template < typename Iterator >
  // void forget(Iterator begin, Iterator end);
  // template < typename Iterator >
  // void forget_nullity(Iterator begin, Iterator end);

  virtual ~nullity_domain() {}

}; // end class nullity_domain

namespace null_domain_traits {

// Helpers to enable the correct implementation depending on the domain.

template < typename Domain >
struct enable_if_nullity
    : public std::enable_if<
          std::is_base_of< nullity_domain< typename Domain::variable_name_t >,
                           Domain >::value > {};

template < typename Domain >
struct enable_if_not_nullity
    : public std::enable_if<
          !std::is_base_of< nullity_domain< typename Domain::variable_name_t >,
                            Domain >::value > {};

// make_null

template < typename Domain >
inline void make_null(Domain& inv,
                      typename Domain::variable_name_t v,
                      typename enable_if_nullity< Domain >::type* = 0) {
  inv.make_null(v);
}

template < typename Domain >
inline void make_null(Domain& /*inv*/,
                      typename Domain::variable_name_t /*v*/,
                      typename enable_if_not_nullity< Domain >::type* = 0) {}

// make_non_null

template < typename Domain >
inline void make_non_null(Domain& inv,
                          typename Domain::variable_name_t v,
                          typename enable_if_nullity< Domain >::type* = 0) {
  inv.make_non_null(v);
}

template < typename Domain >
inline void make_non_null(Domain& /*inv*/,
                          typename Domain::variable_name_t /*v*/,
                          typename enable_if_not_nullity< Domain >::type* = 0) {
}

// assign_nullity

template < typename Domain >
inline void assign_nullity(Domain& inv,
                           typename Domain::variable_name_t x,
                           typename Domain::variable_name_t y,
                           typename enable_if_nullity< Domain >::type* = 0) {
  inv.assign_nullity(x, y);
}

template < typename Domain >
inline void assign_nullity(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*x*/,
    typename Domain::variable_name_t /*y*/,
    typename enable_if_not_nullity< Domain >::type* = 0) {}

// assert_null

template < typename Domain >
inline void assert_null(Domain& inv,
                        typename Domain::variable_name_t v,
                        typename enable_if_nullity< Domain >::type* = 0) {
  inv.assert_null(v);
}

template < typename Domain >
inline void assert_null(Domain& /*inv*/,
                        typename Domain::variable_name_t /*v*/,
                        typename enable_if_not_nullity< Domain >::type* = 0) {}

// assert_non_null

template < typename Domain >
inline void assert_non_null(Domain& inv,
                            typename Domain::variable_name_t v,
                            typename enable_if_nullity< Domain >::type* = 0) {
  inv.assert_non_null(v);
}

template < typename Domain >
inline void assert_non_null(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*v*/,
    typename enable_if_not_nullity< Domain >::type* = 0) {}

// assert_nullity

template < typename Domain >
inline void assert_nullity(Domain& inv,
                           bool equality,
                           typename Domain::variable_name_t x,
                           typename Domain::variable_name_t y,
                           typename enable_if_nullity< Domain >::type* = 0) {
  inv.assert_nullity(equality, x, y);
}

template < typename Domain >
inline void assert_nullity(
    Domain& /*inv*/,
    bool /*equality*/,
    typename Domain::variable_name_t /*x*/,
    typename Domain::variable_name_t /*y*/,
    typename enable_if_not_nullity< Domain >::type* = 0) {}

// is_null

template < typename Domain >
inline bool is_null(Domain& inv,
                    typename Domain::variable_name_t v,
                    typename enable_if_nullity< Domain >::type* = 0) {
  return inv.is_null(v);
}

template < typename Domain >
inline bool is_null(Domain& inv,
                    typename Domain::variable_name_t /*v*/,
                    typename enable_if_not_nullity< Domain >::type* = 0) {
  if (inv.is_bottom()) {
    throw logic_error("null_domain_traits: trying to call is_null() on bottom");
  } else {
    return false;
  }
}

// is_non_null

template < typename Domain >
inline bool is_non_null(Domain& inv,
                        typename Domain::variable_name_t v,
                        typename enable_if_nullity< Domain >::type* = 0) {
  return inv.is_non_null(v);
}

template < typename Domain >
inline bool is_non_null(Domain& inv,
                        typename Domain::variable_name_t /*v*/,
                        typename enable_if_not_nullity< Domain >::type* = 0) {
  if (inv.is_bottom()) {
    throw logic_error(
        "null_domain_traits: trying to call is_non_null() on bottom");
  } else {
    return false;
  }
}

// forget_nullity

template < typename Domain >
inline void forget_nullity(Domain& inv,
                           typename Domain::variable_name_t v,
                           typename enable_if_nullity< Domain >::type* = 0) {
  inv.forget_nullity(v);
}

template < typename Domain >
inline void forget_nullity(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*v*/,
    typename enable_if_not_nullity< Domain >::type* = 0) {}

template < typename Domain, typename Iterator >
inline void forget_nullity(Domain& inv,
                           Iterator begin,
                           Iterator end,
                           typename enable_if_nullity< Domain >::type* = 0) {
  inv.forget_nullity(begin, end);
}

template < typename Domain, typename Iterator >
inline void forget_nullity(
    Domain& /*inv*/,
    Iterator /*begin*/,
    Iterator /*end*/,
    typename enable_if_not_nullity< Domain >::type* = 0) {}

} // end namespace null_domain_traits

} // end namespace ikos

#endif // IKOS_NULLITY_DOMAINS_API_HPP
