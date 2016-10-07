/*******************************************************************************
 *
 * Generic API for memory domains.
 *
 * Author: Maxime Arthaud
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

#ifndef IKOS_MEMORY_DOMAINS_API_HPP
#define IKOS_MEMORY_DOMAINS_API_HPP

#include <type_traits>

#include <ikos/algorithms/linear_constraints.hpp>
#include <ikos/algorithms/literal.hpp>

namespace ikos {

template < typename Integer, typename Float, typename VariableName >
class memory_domain {
public:
  typedef Integer integer_t;
  typedef Float floating_point_t;
  typedef VariableName variable_name_t;
  typedef linear_expression< Integer, VariableName > int_linear_expression_t;
  typedef literal< Integer, Float, VariableName > literal_t;

public:
  /*
   * Perform the memory write: *p = v
   *
   * Arguments:
   *   p: The pointer variable
   *   v: The stored value
   *   size: The stored size, in bytes (for instance, 4 for a int)
   */
  virtual void mem_write(VariableName p, literal_t v, Integer size) = 0;

  /*
   * Perform the memory read: x = *p
   *
   * Arguments:
   *   x: The result variable
   *   p: The pointer variable
   *   size: The read size, in bytes (for instance, 4 for a int)
   */
  virtual void mem_read(literal_t x, VariableName p, Integer size) = 0;

  /*
   * Perform the memory copy: memcpy(dest, src, size)
   *
   * Arguments:
   *   dest: The destination pointer variable
   *   src: The source pointer variable
   *   size: The number of bytes copied, as a linear expression
   *
   * Notes:
   *   If dst and src overlap, as the behavior in C is undefined, the memory
   *   contents is set to top.
   */
  virtual void mem_copy(VariableName dest,
                        VariableName src,
                        int_linear_expression_t size) = 0;

  /*
   * Perform the memory set: memset(dest, value, size)
   *
   * Arguments:
   *   dest: The destination pointer variable
   *   value: A byte value, as a linear expression
   *   size: The number of written bytes, as a linear expression
   */
  virtual void mem_set(VariableName dest,
                       int_linear_expression_t value,
                       int_linear_expression_t size) = 0;

  /*
   * Forget the memory surface of p
   *
   * Description:
   *   Similar to operator-= but forget only the "surface" part of
   *   the underlying domain. That is, if a variable represents a
   *   pointer it will forget its base address and offset but not its
   *   memory contents.
   */
  virtual void forget_mem_surface(VariableName p) = 0;

  /*
   * Forget the memory contents of p
   *
   * Description:
   *   Similar to operator-= but forget only the memory contents
   *   accessible through pointer p.
   */
  virtual void forget_mem_contents(VariableName p) = 0;

  /*
   * Forget the memory contents in range [p, p + size - 1]
   *
   * Arguments:
   *   p: The pointer variable
   *   size: The size in bytes
   *
   * Description:
   *   Similar to operator-= but forget only all the memory contents
   *   included in the range [p, ..., p + size - 1].
   */
  virtual void forget_mem_contents(VariableName p, Integer size) = 0;

  /*
   * Forget everything related to v
   *
   * If v is a scalar variable, forget its value
   * If v is a pointer, forget its surface (base addresses and offsets) and
   * all the memory contents that may be accessible through it
   */
  virtual void operator-=(VariableName v) = 0;

  virtual ~memory_domain() {}

}; // end class memory_domain

namespace mem_domain_traits {

// Helpers to enable the correct implementation depending on the domain.

struct _is_memory_domain_impl {
  template < typename Domain >
  static std::true_type _test(
      typename std::enable_if<
          std::is_base_of< memory_domain< typename Domain::integer_t,
                                          typename Domain::floating_point_t,
                                          typename Domain::variable_name_t >,
                           Domain >::value,
          int >::type);

  template < typename >
  static std::false_type _test(...);
};

template < typename Domain >
struct _is_memory_domain : public _is_memory_domain_impl {
  typedef decltype(_test< Domain >(0)) type;
};

template < typename Domain >
struct is_memory_domain : public _is_memory_domain< Domain >::type {};

template < typename Domain >
struct enable_if_memory
    : public std::enable_if< is_memory_domain< Domain >::value > {};

template < typename Domain >
struct enable_if_not_memory
    : public std::enable_if< !is_memory_domain< Domain >::value > {};

// mem_write

template < typename Domain, typename Literal, typename Integer >
inline void mem_write(Domain& inv,
                      typename Domain::variable_name_t p,
                      Literal v,
                      Integer size,
                      typename enable_if_memory< Domain >::type* = 0) {
  inv.mem_write(p, v, size);
}

template < typename Domain, typename Literal, typename Integer >
inline void mem_write(Domain& /*inv*/,
                      typename Domain::variable_name_t /*p*/,
                      Literal /*v*/,
                      Integer /*size*/,
                      typename enable_if_not_memory< Domain >::type* = 0) {}

// mem_read

template < typename Domain, typename Literal, typename Integer >
inline void mem_read(Domain& inv,
                     Literal x,
                     typename Domain::variable_name_t p,
                     Integer size,
                     typename enable_if_memory< Domain >::type* = 0) {
  inv.mem_read(x, p, size);
}

template < typename Domain, typename Literal, typename Integer >
inline void mem_read(Domain& /*inv*/,
                     Literal /*x*/,
                     typename Domain::variable_name_t /*p*/,
                     Integer /*size*/,
                     typename enable_if_not_memory< Domain >::type* = 0) {}

// mem_copy

template < typename Domain, typename Integer >
inline void mem_copy(
    Domain& inv,
    typename Domain::variable_name_t dest,
    typename Domain::variable_name_t src,
    linear_expression< Integer, typename Domain::variable_name_t > size,
    typename enable_if_memory< Domain >::type* = 0) {
  inv.mem_copy(dest, src, size);
}

template < typename Domain, typename Integer >
inline void mem_copy(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*dest*/,
    typename Domain::variable_name_t /*src*/,
    linear_expression< Integer, typename Domain::variable_name_t > /*size*/,
    typename enable_if_not_memory< Domain >::type* = 0) {}

// mem_set

template < typename Domain, typename Integer >
inline void mem_set(
    Domain& inv,
    typename Domain::variable_name_t dest,
    linear_expression< Integer, typename Domain::variable_name_t > value,
    linear_expression< Integer, typename Domain::variable_name_t > size,
    typename enable_if_memory< Domain >::type* = 0) {
  inv.mem_set(dest, value, size);
}

template < typename Domain, typename Integer >
inline void mem_set(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*dest*/,
    linear_expression< Integer, typename Domain::variable_name_t > /*value*/,
    linear_expression< Integer, typename Domain::variable_name_t > /*size*/,
    typename enable_if_not_memory< Domain >::type* = 0) {}

// forget_mem_surface

template < typename Domain >
inline void forget_mem_surface(Domain& inv,
                               typename Domain::variable_name_t p,
                               typename enable_if_memory< Domain >::type* = 0) {
  inv.forget_mem_surface(p);
}

template < typename Domain >
inline void forget_mem_surface(
    Domain& inv,
    typename Domain::variable_name_t p,
    typename enable_if_not_memory< Domain >::type* = 0) {
  inv -= p;
}

// forget_mem_contents

template < typename Domain >
inline void forget_mem_contents(
    Domain& inv,
    typename Domain::variable_name_t p,
    typename enable_if_memory< Domain >::type* = 0) {
  inv.forget_mem_contents(p);
}

template < typename Domain >
inline void forget_mem_contents(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*p*/,
    typename enable_if_not_memory< Domain >::type* = 0) {}

template < typename Domain, typename Integer >
inline void forget_mem_contents(
    Domain& inv,
    typename Domain::variable_name_t p,
    Integer size,
    typename enable_if_memory< Domain >::type* = 0) {
  inv.forget_mem_contents(p, size);
}

template < typename Domain, typename Integer >
inline void forget_mem_contents(
    Domain& /*inv*/,
    typename Domain::variable_name_t /*p*/,
    Integer /*size*/,
    typename enable_if_not_memory< Domain >::type* = 0) {}

} // end namespace mem_domain_traits

} // end namespace ikos

#endif // IKOS_MEMORY_DOMAINS_API_HPP
