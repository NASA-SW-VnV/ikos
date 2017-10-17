/**************************************************************************/ /**
 *
 * \file
 * \brief Generic API for abstract domains that keep track of exceptions.
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

#ifndef IKOS_EXCEPTION_DOMAINS_API_HPP
#define IKOS_EXCEPTION_DOMAINS_API_HPP

#include <type_traits>

#include <boost/optional.hpp>

namespace ikos {

/// \brief Base class for abstract domains supporting exceptions
template < typename VariableName >
class exception_domain {
public:
  typedef VariableName variable_name_t;

public:
  /*
   * // return the top abstract domain with no pending exception
   * // note that this is different from top() because top() represents the
   * // state with any pending exception.
   * static T top_no_exception();
   *
   * note: T is the concrete class type.
   */

  virtual bool is_bottom() = 0;

  virtual bool is_normal_flow_bottom() = 0;

  virtual bool is_pending_exceptions_bottom() = 0;

  virtual bool is_top() = 0;

  virtual bool is_normal_flow_top() = 0;

  virtual bool is_pending_exceptions_top() = 0;

  virtual void set_normal_flow_bottom() = 0;

  virtual void set_pending_exceptions_top() = 0;

  /// \brief Enter a catch block
  ///
  /// \param exc The result variable that contains the exception structure
  ///            (exception pointer and exception type id)
  virtual void enter_catch(variable_name_t exc) = 0;

  /// \brief Disregard the pending exception
  virtual void ignore_exceptions() = 0;

  /// \brief Throw an exception
  ///
  /// \param exc The exception pointer
  /// \param tinfo The pointer to the exception type_info
  /// \param dest The pointer to the destructor of the exception (or null)
  virtual void throw_exception(variable_name_t exc,
                               variable_name_t tinfo,
                               boost::optional< variable_name_t > dest) = 0;

  /// \brief Resume/rethrow an exception
  ///
  /// \param exc The exception structure (exception pointer and exception
  ///            type id)
  virtual void resume_exception(variable_name_t exc) = 0;

  virtual ~exception_domain() {}

}; // end class exception_domain

namespace exc_domain_traits {

// Helpers to enable the correct implementation depending on the domain.

template < typename Domain >
struct enable_if_exception
    : public std::enable_if<
          std::is_base_of< exception_domain< typename Domain::variable_name_t >,
                           Domain >::value > {};

template < typename Domain >
struct enable_if_not_exception
    : public std::enable_if< !std::is_base_of<
          exception_domain< typename Domain::variable_name_t >,
          Domain >::value > {};

// top_no_exception

template < typename Domain >
inline Domain top_no_exception(
    typename enable_if_exception< Domain >::type* = 0) {
  return Domain::top_no_exception();
}

template < typename Domain >
inline Domain top_no_exception(
    typename enable_if_not_exception< Domain >::type* = 0) {
  return Domain::top();
}

// is_normal_flow_bottom

template < typename Domain >
inline bool is_normal_flow_bottom(
    Domain& inv, typename enable_if_exception< Domain >::type* = 0) {
  return inv.is_normal_flow_bottom();
}

template < typename Domain >
inline bool is_normal_flow_bottom(
    Domain& inv, typename enable_if_not_exception< Domain >::type* = 0) {
  return inv.is_bottom();
}

// is_pending_exceptions_bottom

template < typename Domain >
inline bool is_pending_exceptions_bottom(
    Domain& inv, typename enable_if_exception< Domain >::type* = 0) {
  return inv.is_pending_exceptions_bottom();
}

template < typename Domain >
inline bool is_pending_exceptions_bottom(
    Domain& /*inv*/, typename enable_if_not_exception< Domain >::type* = 0) {
  return false;
}

// is_normal_flow_top

template < typename Domain >
inline bool is_normal_flow_top(
    Domain& inv, typename enable_if_exception< Domain >::type* = 0) {
  return inv.is_normal_flow_top();
}

template < typename Domain >
inline bool is_normal_flow_top(
    Domain& inv, typename enable_if_not_exception< Domain >::type* = 0) {
  return inv.is_top();
}

// is_pending_exceptions_top

template < typename Domain >
inline bool is_pending_exceptions_top(
    Domain& inv, typename enable_if_exception< Domain >::type* = 0) {
  return inv.is_pending_exceptions_top();
}

template < typename Domain >
inline bool is_pending_exceptions_top(
    Domain& /*inv*/, typename enable_if_not_exception< Domain >::type* = 0) {
  return true;
}

// set_normal_flow_bottom

template < typename Domain >
inline void set_normal_flow_bottom(
    Domain& inv, typename enable_if_exception< Domain >::type* = 0) {
  inv.set_normal_flow_bottom();
}

template < typename Domain >
inline void set_normal_flow_bottom(
    Domain& inv, typename enable_if_not_exception< Domain >::type* = 0) {
  inv = Domain::bottom();
}

// set_pending_exceptions_top

template < typename Domain >
inline void set_pending_exceptions_top(
    Domain& inv, typename enable_if_exception< Domain >::type* = 0) {
  inv.set_pending_exceptions_top();
}

template < typename Domain >
inline void set_pending_exceptions_top(
    Domain& /*inv*/, typename enable_if_not_exception< Domain >::type* = 0) {}

// enter_catch

template < typename Domain >
inline void enter_catch(Domain& inv,
                        typename Domain::variable_name_t exc,
                        typename enable_if_exception< Domain >::type* = 0) {
  inv.enter_catch(exc);
}

template < typename Domain >
inline void enter_catch(Domain& inv,
                        typename Domain::variable_name_t /*exc*/,
                        typename enable_if_not_exception< Domain >::type* = 0) {
  inv = Domain::top();
}

// ignore_exceptions

template < typename Domain >
inline void ignore_exceptions(
    Domain& inv, typename enable_if_exception< Domain >::type* = 0) {
  inv.ignore_exceptions();
}

template < typename Domain >
inline void ignore_exceptions(
    Domain& /*inv*/, typename enable_if_not_exception< Domain >::type* = 0) {}

// throw_exception

template < typename Domain >
inline void throw_exception(
    Domain& inv,
    typename Domain::variable_name_t exc,
    typename Domain::variable_name_t tinfo,
    boost::optional< typename Domain::variable_name_t > dest,
    typename enable_if_exception< Domain >::type* = 0) {
  inv.throw_exception(exc, tinfo, dest);
}

template < typename Domain >
inline void throw_exception(
    Domain& inv,
    typename Domain::variable_name_t /*exc*/,
    typename Domain::variable_name_t /*tinfo*/,
    boost::optional< typename Domain::variable_name_t > /*dest*/,
    typename enable_if_not_exception< Domain >::type* = 0) {
  inv = Domain::bottom();
}

// resume_exception

template < typename Domain >
inline void resume_exception(
    Domain& inv,
    typename Domain::variable_name_t exc,
    typename enable_if_exception< Domain >::type* = 0) {
  inv.resume_exception(exc);
}

template < typename Domain >
inline void resume_exception(
    Domain& inv,
    typename Domain::variable_name_t /*exc*/,
    typename enable_if_not_exception< Domain >::type* = 0) {
  inv = Domain::bottom();
}

} // end namespace exc_domain_traits

} // end namespace ikos

#endif // IKOS_EXCEPTION_DOMAINS_API_HPP
