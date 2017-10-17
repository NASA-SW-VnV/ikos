/**************************************************************************/ /**
 *
 * \file
 * \brief Generic API for abstract domains that keep track of (un)initialized
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

#ifndef IKOS_UNINITIALIZED_DOMAINS_API_HPP
#define IKOS_UNINITIALIZED_DOMAINS_API_HPP

#include <type_traits>

#include <ikos/common/types.hpp>

namespace ikos {

/// \brief Base class for abstract domains supporting uninitialized variables
template < typename VariableName >
class uninitialized_domain {
public:
  typedef VariableName variable_name_t;

public:
  /// \brief Mark v as initialized
  virtual void make_initialized(VariableName v) = 0;

  /// \brief Mark v as uninitialized
  virtual void make_uninitialized(VariableName v) = 0;

  /// \brief Assign x = y
  virtual void assign_uninitialized(VariableName x, VariableName y) = 0;

  /// \brief Assign x = f(y, z)
  virtual void assign_uninitialized(VariableName x,
                                    VariableName y,
                                    VariableName z) = 0;

  /// \returns true if v is initialized, otherwise false
  virtual bool is_initialized(VariableName v) = 0;

  /// \returns true if v is uninitialized, otherwise false
  virtual bool is_uninitialized(VariableName v) = 0;

  /// \brief Forget a variable
  virtual void operator-=(VariableName v) = 0;

  /// \brief Forget a variable
  virtual void forget(VariableName v) = 0;

  /// \brief Forget a variable
  virtual void forget_uninitialized(VariableName v) = 0;

  // forget variables
  // template < typename Iterator >
  // void forget(Iterator begin, Iterator end);
  // template < typename Iterator >
  // void forget_uninitialized(Iterator begin, Iterator end);

  virtual ~uninitialized_domain() {}

}; // end class uninitialized_domain

namespace uninit_domain_traits {

// Helpers to enable the correct implementation depending on the domain.

template < typename Domain >
struct enable_if_uninitialized
    : public std::enable_if< std::is_base_of<
          uninitialized_domain< typename Domain::variable_name_t >,
          Domain >::value > {};

template < typename Domain >
struct enable_if_not_uninitialized
    : public std::enable_if< !std::is_base_of<
          uninitialized_domain< typename Domain::variable_name_t >,
          Domain >::value > {};

// make_initialized

template < typename Domain >
inline void make_initialized(
    Domain& inv,
    typename Domain::variable_name_t v,
    typename enable_if_uninitialized< Domain >::type* = 0) {
  inv.make_initialized(v);
}

template < typename Domain >
inline void make_initialized(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*v*/,
    typename enable_if_not_uninitialized< Domain >::type* = 0) {}

// make_uninitialized

template < typename Domain >
inline void make_uninitialized(
    Domain& inv,
    typename Domain::variable_name_t v,
    typename enable_if_uninitialized< Domain >::type* = 0) {
  inv.make_uninitialized(v);
}

template < typename Domain >
inline void make_uninitialized(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*v*/,
    typename enable_if_not_uninitialized< Domain >::type* = 0) {}

// assign_uninitialized

template < typename Domain >
inline void assign_uninitialized(
    Domain& inv,
    typename Domain::variable_name_t x,
    typename Domain::variable_name_t y,
    typename enable_if_uninitialized< Domain >::type* = 0) {
  inv.assign_uninitialized(x, y);
}

template < typename Domain >
inline void assign_uninitialized(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*x*/,
    typename Domain::variable_name_t /*y*/,
    typename enable_if_not_uninitialized< Domain >::type* = 0) {}

template < typename Domain >
inline void assign_uninitialized(
    Domain& inv,
    typename Domain::variable_name_t x,
    typename Domain::variable_name_t y,
    typename Domain::variable_name_t z,
    typename enable_if_uninitialized< Domain >::type* = 0) {
  inv.assign_uninitialized(x, y, z);
}

template < typename Domain >
inline void assign_uninitialized(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*x*/,
    typename Domain::variable_name_t /*y*/,
    typename Domain::variable_name_t /*z*/,
    typename enable_if_not_uninitialized< Domain >::type* = 0) {}

// is_initialized

template < typename Domain >
inline bool is_initialized(
    Domain& inv,
    typename Domain::variable_name_t v,
    typename enable_if_uninitialized< Domain >::type* = 0) {
  return inv.is_initialized(v);
}

template < typename Domain >
inline bool is_initialized(
    Domain& inv,
    typename Domain::variable_name_t /*v*/,
    typename enable_if_not_uninitialized< Domain >::type* = 0) {
  if (inv.is_bottom()) {
    throw logic_error(
        "uninit_domain_traits: trying to call is_initialized() on bottom");
  } else {
    return false;
  }
}

// is_uninitialized

template < typename Domain >
inline bool is_uninitialized(
    Domain& inv,
    typename Domain::variable_name_t v,
    typename enable_if_uninitialized< Domain >::type* = 0) {
  return inv.is_uninitialized(v);
}

template < typename Domain >
inline bool is_uninitialized(
    Domain& inv,
    typename Domain::variable_name_t /*v*/,
    typename enable_if_not_uninitialized< Domain >::type* = 0) {
  if (inv.is_bottom()) {
    throw logic_error(
        "uninit_domain_traits: trying to call is_uninitialized() on bottom");
  } else {
    return false;
  }
}

// forget_uninitialized

template < typename Domain >
inline void forget_uninitialized(
    Domain& inv,
    typename Domain::variable_name_t v,
    typename enable_if_uninitialized< Domain >::type* = 0) {
  inv.forget_uninitialized(v);
}

template < typename Domain >
inline void forget_uninitialized(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*v*/,
    typename enable_if_not_uninitialized< Domain >::type* = 0) {}

template < typename Domain, typename Iterator >
inline void forget_uninitialized(
    Domain& inv,
    Iterator begin,
    Iterator end,
    typename enable_if_uninitialized< Domain >::type* = 0) {
  inv.forget_uninitialized(begin, end);
}

template < typename Domain, typename Iterator >
inline void forget_uninitialized(
    Domain& /*inv*/,
    Iterator /*begin*/,
    Iterator /*end*/,
    typename enable_if_not_uninitialized< Domain >::type* = 0) {}

} // end namespace uninit_domain_traits

} // end namespace ikos

#endif // IKOS_UNINITIALIZED_DOMAINS_API_HPP
