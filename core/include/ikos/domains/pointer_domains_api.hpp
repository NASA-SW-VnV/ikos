/**************************************************************************/ /**
 *
 * \file
 * \brief Generic API for pointer domains.
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

#ifndef IKOS_POINTER_DOMAINS_API_HPP
#define IKOS_POINTER_DOMAINS_API_HPP

#include <type_traits>

#include <ikos/domains/discrete_domains.hpp>
#include <ikos/value/interval.hpp>

namespace ikos {

/// \brief Base class for pointer abstract domains
template < typename Number, typename VariableName, typename MemoryLocation >
class pointer_domain {
public:
  typedef Number integer_t;
  typedef VariableName variable_name_t;
  typedef MemoryLocation memory_location_t;
  typedef discrete_domain< MemoryLocation > points_to_set_t;

public:
  /// \brief Assign p to a memory object (e.g., p = &obj or p = malloc())
  virtual void assign_object(VariableName p, MemoryLocation obj) = 0;

  /// \brief Assign p = q
  virtual void assign_pointer(VariableName p, VariableName q) = 0;

  /// \brief Assign p = q + o
  virtual void assign_pointer(VariableName p,
                              VariableName q,
                              VariableName o) = 0;

  /// \brief Assign p = q + o
  virtual void assign_pointer(VariableName p, VariableName q, Number o) = 0;

  /// \brief Add constraint (p == q) or (p != q)
  virtual void assert_pointer(bool equality,
                              VariableName p,
                              VariableName q) = 0;

  /// \brief Refine the set of possible addresses of p
  virtual void refine_addrs(VariableName p, points_to_set_t addrs) = 0;

  /// \brief Refine the set of possible addresses and offset of p
  virtual void refine_addrs_offset(VariableName p,
                                   points_to_set_t addrs,
                                   interval< Number > offset) = 0;

  /// \brief Return true if the set of possible addresses of p is unknown
  virtual bool is_unknown_addr(VariableName p) = 0;

  /// \brief Return the set of possible addresses of p
  virtual points_to_set_t addrs_set(VariableName p) = 0;

  /// \brief Return the offset variable associated to p
  virtual VariableName offset_var(VariableName p);

  /// \brief Forget a variable (scalar or pointer)
  virtual void operator-=(VariableName v) = 0;

  /// \brief Forget a variable (scalar or pointer)
  virtual void forget(VariableName v) = 0;

  // forget variables (scalars or pointers)
  // template < typename Iterator >
  // void forget(Iterator begin, Iterator end);

  /// \brief Forget a pointer variable
  virtual void forget_pointer(VariableName p) = 0;

  // forget pointer variables
  // template < typename Iterator >
  // void forget_pointer(Iterator begin, Iterator end);

  virtual ~pointer_domain() {}

}; // end class pointer_domain

// Use var_name_traits to define the default pointer_domain::offset_var()

namespace var_name_traits {

// default implementation: use the same variable for the pointer and the offset
template < typename VariableName >
struct offset_var {
  VariableName operator()(VariableName p) {
    // note that this implementation leads to a lose of precision.
    // you should rather return a different variable name, such as p.offset
    return p;
  }
};

} // end namespace var_name_traits

template < typename Number, typename VariableName, typename MemoryLocation >
VariableName pointer_domain< Number, VariableName, MemoryLocation >::offset_var(
    VariableName p) {
  return var_name_traits::offset_var< VariableName >()(p);
}

namespace ptr_domain_traits {

// Helpers to enable the correct implementation depending on the domain.

struct _is_pointer_domain_impl {
  template < typename Domain >
  static std::true_type _test(
      typename std::enable_if<
          std::is_base_of< pointer_domain< typename Domain::integer_t,
                                           typename Domain::variable_name_t,
                                           typename Domain::memory_location_t >,
                           Domain >::value,
          int >::type);

  template < typename >
  static std::false_type _test(...);
};

template < typename Domain >
struct _is_pointer_domain : public _is_pointer_domain_impl {
  typedef decltype(_test< Domain >(0)) type;
};

template < typename Domain >
struct is_pointer_domain : public _is_pointer_domain< Domain >::type {};

template < typename Domain >
struct enable_if_pointer
    : public std::enable_if< is_pointer_domain< Domain >::value > {};

template < typename Domain >
struct enable_if_not_pointer
    : public std::enable_if< !is_pointer_domain< Domain >::value > {};

// assign_object

template < typename Domain, typename MemoryLocation >
inline void assign_object(Domain& inv,
                          typename Domain::variable_name_t p,
                          MemoryLocation obj,
                          typename enable_if_pointer< Domain >::type* = 0) {
  inv.assign_object(p, obj);
}

template < typename Domain, typename MemoryLocation >
inline void assign_object(Domain& /*inv*/,
                          typename Domain::variable_name_t /*p*/,
                          MemoryLocation /*obj*/,
                          typename enable_if_not_pointer< Domain >::type* = 0) {
}

// assign_pointer

template < typename Domain >
inline void assign_pointer(Domain& inv,
                           typename Domain::variable_name_t p,
                           typename Domain::variable_name_t q,
                           typename enable_if_pointer< Domain >::type* = 0) {
  inv.assign_pointer(p, q);
}

template < typename Domain >
inline void assign_pointer(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*p*/,
    typename Domain::variable_name_t /*q*/,
    typename enable_if_not_pointer< Domain >::type* = 0) {}

// Offset is either Domain::integer_t or Domain::variable_name_t
template < typename Domain, typename Offset >
inline void assign_pointer(Domain& inv,
                           typename Domain::variable_name_t p,
                           typename Domain::variable_name_t q,
                           Offset o,
                           typename enable_if_pointer< Domain >::type* = 0) {
  inv.assign_pointer(p, q, o);
}

template < typename Domain, typename Offset >
inline void assign_pointer(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*p*/,
    typename Domain::variable_name_t /*q*/,
    Offset /*o*/,
    typename enable_if_not_pointer< Domain >::type* = 0) {}

// assert_pointer

template < typename Domain >
inline void assert_pointer(Domain& inv,
                           bool equality,
                           typename Domain::variable_name_t p,
                           typename Domain::variable_name_t q,
                           typename enable_if_pointer< Domain >::type* = 0) {
  inv.assert_pointer(equality, p, q);
}

template < typename Domain >
inline void assert_pointer(
    Domain& /*inv*/,
    bool /*equality*/,
    typename Domain::variable_name_t /*p*/,
    typename Domain::variable_name_t /*q*/,
    typename enable_if_not_pointer< Domain >::type* = 0) {}

// refine_addrs

template < typename Domain, typename MemoryLocation >
inline void refine_addrs(Domain& inv,
                         typename Domain::variable_name_t p,
                         discrete_domain< MemoryLocation > addrs,
                         typename enable_if_pointer< Domain >::type* = 0) {
  inv.refine_addrs(p, addrs);
}

template < typename Domain, typename MemoryLocation >
inline void refine_addrs(Domain& /*inv*/,
                         typename Domain::variable_name_t /*p*/,
                         discrete_domain< MemoryLocation > /*addrs*/,
                         typename enable_if_not_pointer< Domain >::type* = 0) {}

// refine_addrs_offset

template < typename Domain, typename Number, typename MemoryLocation >
inline void refine_addrs_offset(
    Domain& inv,
    typename Domain::variable_name_t p,
    discrete_domain< MemoryLocation > addrs,
    interval< Number > offset,
    typename enable_if_pointer< Domain >::type* = 0) {
  inv.refine_addrs_offset(p, addrs, offset);
}

template < typename Domain, typename Number, typename MemoryLocation >
inline void refine_addrs_offset(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*p*/,
    discrete_domain< MemoryLocation > /*addrs*/,
    interval< Number > /*offset*/,
    typename enable_if_not_pointer< Domain >::type* = 0) {}

// is_unknown_addr

template < typename Domain >
inline bool is_unknown_addr(Domain& inv,
                            typename Domain::variable_name_t p,
                            typename enable_if_pointer< Domain >::type* = 0) {
  return inv.is_unknown_addr(p);
}

template < typename Domain >
inline bool is_unknown_addr(
    Domain& inv,
    typename Domain::variable_name_t /*p*/,
    typename enable_if_not_pointer< Domain >::type* = 0) {
  if (inv.is_bottom()) {
    throw logic_error(
        "ptr_domain_traits: trying to call is_unknown_addr() on bottom");
  } else {
    return true;
  }
}

// addrs_set

template < typename Domain, typename MemoryLocation >
inline discrete_domain< MemoryLocation > addrs_set(
    Domain& inv,
    typename Domain::variable_name_t p,
    typename enable_if_pointer< Domain >::type* = 0) {
  return inv.addrs_set(p);
}

template < typename Domain, typename MemoryLocation >
inline discrete_domain< MemoryLocation > addrs_set(
    Domain& inv,
    typename Domain::variable_name_t /*p*/,
    typename enable_if_not_pointer< Domain >::type* = 0) {
  if (inv.is_bottom()) {
    throw logic_error(
        "ptr_domain_traits: trying to call addrs_set() on bottom");
  } else {
    return discrete_domain< MemoryLocation >::top();
  }
}

// offset_var

template < typename Domain >
inline typename Domain::variable_name_t offset_var(
    Domain& inv,
    typename Domain::variable_name_t p,
    typename enable_if_pointer< Domain >::type* = 0) {
  return inv.offset_var(p);
}

template < typename Domain >
inline typename Domain::variable_name_t offset_var(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*p*/,
    typename enable_if_not_pointer< Domain >::type* = 0) {
  throw logic_error(
      "ptr_domain_traits: trying to call offset_var() on a non-pointer "
      "domain.");
}

// forget_pointer

template < typename Domain >
inline void forget_pointer(Domain& inv,
                           typename Domain::variable_name_t p,
                           typename enable_if_pointer< Domain >::type* = 0) {
  inv.forget_pointer(p);
}

template < typename Domain >
inline void forget_pointer(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*p*/,
    typename enable_if_not_pointer< Domain >::type* = 0) {}

template < typename Domain, typename Iterator >
inline void forget_pointer(Domain& inv,
                           Iterator begin,
                           Iterator end,
                           typename enable_if_pointer< Domain >::type* = 0) {
  inv.forget_pointer(begin, end);
}

template < typename Domain, typename Iterator >
inline void forget_pointer(
    Domain& /*inv*/,
    Iterator /*begin*/,
    Iterator /*end*/,
    typename enable_if_not_pointer< Domain >::type* = 0) {}

} // end namespace ptr_domain_traits

} // end namespace ikos

#endif // IKOS_POINTER_DOMAINS_API_HPP
