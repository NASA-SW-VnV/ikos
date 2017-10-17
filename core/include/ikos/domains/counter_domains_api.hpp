/**************************************************************************/ /**
 *
 * \file
 * \brief Generic API for (non-negative) counter-aware abstract domains.
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017 United States Government as represented by the
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

#ifndef IKOS_COUNTER_DOMAINS_API_HPP
#define IKOS_COUNTER_DOMAINS_API_HPP

#include <type_traits>

#include <ikos/common/types.hpp>

namespace ikos {

/// \brief Base class for non-negative counter-aware abstract domains
template < typename Number, typename VariableName >
class counter_domain {
public:
  /// \brief Mark the variable `v` as a non-negative loop counter
  virtual void mark_counter(VariableName v) = 0;

  /// \brief Mark the variable `v` as a normal variable,
  /// without losing information
  virtual void unmark_counter(VariableName v) = 0;

  /// \brief Initialize a counter: v = c
  ///
  /// Precondition: c >= 0
  virtual void init_counter(VariableName v, Number c) = 0;

  /// \brief Increment a counter: v += k
  ///
  /// Precondition: k >= 1
  virtual void incr_counter(VariableName v, Number k) = 0;

  /// \brief forget a counter
  virtual void forget_counter(VariableName v) = 0;

  virtual ~counter_domain() {}

}; // end class counter_domain

namespace counter_domain_traits {

// Helpers to enable the correct implementation depending on the domain.

struct _is_counter_domain_impl {
  template < typename Domain >
  static std::true_type _test(
      typename std::enable_if<
          std::is_base_of< counter_domain< typename Domain::number_t,
                                           typename Domain::variable_name_t >,
                           Domain >::value,
          int >::type);

  template < typename >
  static std::false_type _test(...);
};

template < typename Domain >
struct _is_counter_domain : public _is_counter_domain_impl {
  typedef decltype(_test< Domain >(0)) type;
};

template < typename Domain >
struct is_counter_domain : public _is_counter_domain< Domain >::type {};

template < typename Domain >
struct enable_if_counter
    : public std::enable_if< is_counter_domain< Domain >::value > {};

template < typename Domain >
struct enable_if_not_counter
    : public std::enable_if< !is_counter_domain< Domain >::value > {};

template < typename Domain >
inline void mark_counter(Domain& inv,
                         typename Domain::variable_name_t v,
                         typename enable_if_counter< Domain >::type* = 0) {
  inv.mark_counter(v);
}

template < typename Domain >
inline void mark_counter(Domain& /*inv*/,
                         typename Domain::variable_name_t /*v*/,
                         typename enable_if_not_counter< Domain >::type* = 0) {}

template < typename Domain >
inline void unmark_counter(Domain& inv,
                           typename Domain::variable_name_t v,
                           typename enable_if_counter< Domain >::type* = 0) {
  inv.unmark_counter(v);
}

template < typename Domain >
inline void unmark_counter(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*v*/,
    typename enable_if_not_counter< Domain >::type* = 0) {}

template < typename Domain >
inline void init_counter(Domain& inv,
                         typename Domain::variable_name_t v,
                         typename Domain::number_t c,
                         typename enable_if_counter< Domain >::type* = 0) {
  inv.init_counter(v, c);
}

template < typename Domain >
inline void init_counter(Domain& inv,
                         typename Domain::variable_name_t v,
                         typename Domain::number_t c,
                         typename enable_if_not_counter< Domain >::type* = 0) {
  inv.assign(v, c);
}

template < typename Domain >
inline void incr_counter(Domain& inv,
                         typename Domain::variable_name_t v,
                         typename Domain::number_t k,
                         typename enable_if_counter< Domain >::type* = 0) {
  inv.incr_counter(v, k);
}

template < typename Domain >
inline void incr_counter(Domain& inv,
                         typename Domain::variable_name_t v,
                         typename Domain::number_t k,
                         typename enable_if_not_counter< Domain >::type* = 0) {
  typedef variable< typename Domain::number_t,
                    typename Domain::variable_name_t > variable_t;
  inv.assign(v, variable_t(v) + k);
}

template < typename Domain >
inline void forget_counter(Domain& inv,
                           typename Domain::variable_name_t v,
                           typename enable_if_counter< Domain >::type* = 0) {
  inv.forget_counter(v);
}

template < typename Domain >
inline void forget_counter(
    Domain& inv,
    typename Domain::variable_name_t v,
    typename enable_if_not_counter< Domain >::type* = 0) {
  inv.forget_num(v);
}

} // end namespace counter_domain_traits
} // end namespace ikos

#endif // IKOS_COUNTER_DOMAINS_API_HPP
