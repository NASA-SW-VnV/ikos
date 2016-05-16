/******************************************************************************
 *
 * This class executes abstractly ARBOS statements with different
 * levels of precision.
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *
 * It can reason about registers (REG), pointers (PTR) and memory
 * contents (MEM). In the case of registers and memory contents only
 * those storing integers are modelled. Thus, floating-point
 * computations or memory contents accesible through multiple pointer
 * dereferences are safely ignored.
 *
 * Levels of precision:
 *
 * 1) If level of precision is REG then only integer scalar variables
 * are modelled using a numerical abstraction.
 *
 * 2) If the level of precision is PTR then both integer and pointer
 * scalar variables are modelled. If a variable is a pointer we model
 * its address, offset and size. The offset and size are modelled by a
 * numerical abstraction while the address is modelled by a symbolic
 * abstraction. This symbolic abstraction consists of a set of
 * points-to relationships that keeps track of all possible memory
 * objects (i.e., &'s and mallocs) to which the pointer may point to.
 * Thus, a pointer is abstracted by a triple <A,O,S> where A is the
 * set of addresses to which p may point to, O is the offset from the
 * beginning of the block expressed in bytes, and S is the size of the
 * block. The value domain keeps tracks of these triples.
 *
 * 3) If the level of precision is MEM then same level than PTR plus
 * memory contents storing integers are modelled as well. That is, we
 * can keep track of which values are stored in a triple <A,O,S>.
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

#ifndef ANALYZER_NUM_SYM_EXEC_HPP
#define ANALYZER_NUM_SYM_EXEC_HPP

#include <boost/algorithm/string/predicate.hpp>
#include <boost/noncopyable.hpp>

#include <analyzer/ar-wrapper/cfg.hpp>
#include <analyzer/ar-wrapper/literal.hpp>
#include <analyzer/ikos-wrapper/domains_traits.hpp>
#include <analyzer/analysis/common.hpp>
#include <analyzer/analysis/context.hpp>
#include <analyzer/analysis/sym_exec_api.hpp>
#include <analyzer/domains/value_domain.hpp>
#include <analyzer/domains/summary_domain.hpp>

namespace analyzer {

using namespace arbos;
using namespace ikos;

namespace value_domain_impl {
// Here operations which are not standard in numerical abstract
// domains but needed for a value analysis that models memory
// contents and/or keep track of nullity and uninitialized variable
// information.

// x is a new memory object (e.g., &'s and mallocs)
template < typename AbsDomain, typename VariableName >
void make_object(AbsDomain& inv, VariableName x) {}

template < typename AbsDomain, typename VariableName, typename Number >
void make_object(memory_domain< AbsDomain, VariableName, Number >& inv,
                 VariableName x) {
  inv.make_object(x);
}

template < typename AbsDomain, typename VariableName, typename Number >
void make_object(summary_domain< AbsDomain, VariableName, Number >& inv,
                 VariableName x) {
  inv.make_object(x);
}

// x and y are may point to the same memory location
template < typename AbsDomain, typename VariableName >
void assign_ptr(AbsDomain& inv, VariableName x, VariableName y) {}

template < typename AbsDomain, typename VariableName, typename Number >
void assign_ptr(memory_domain< AbsDomain, VariableName, Number >& inv,
                VariableName x,
                VariableName y) {
  inv.assign_ptr(x, y);
}

template < typename AbsDomain, typename VariableName, typename Number >
void assign_ptr(summary_domain< AbsDomain, VariableName, Number >& inv,
                VariableName x,
                VariableName y) {
  inv.assign_ptr(x, y);
}

// Return if nothing can be said about the address of x
template < typename AbsDomain, typename VariableName >
bool is_unknown_addr(AbsDomain& inv, VariableName x) {
  return true;
}

template < typename AbsDomain, typename VariableName, typename Number >
bool is_unknown_addr(memory_domain< AbsDomain, VariableName, Number >& inv,
                     VariableName x) {
  return inv.is_unknown_addr(x);
}

template < typename AbsDomain, typename VariableName, typename Number >
bool is_unknown_addr(summary_domain< AbsDomain, VariableName, Number >& inv,
                     VariableName x) {
  return inv.is_unknown_addr(x);
}

// Return the addresses of x
template < typename AbsDomain, typename VariableName >
std::vector< VariableName > get_addrs_set(AbsDomain& inv, VariableName x) {
  return std::vector< VariableName >();
}

template < typename AbsDomain, typename VariableName, typename Number >
std::vector< VariableName > get_addrs_set(
    memory_domain< AbsDomain, VariableName, Number >& inv, VariableName x) {
  assert(!is_unknown_addr(inv, x));
  return inv.get_addrs_set(x);
}

template < typename AbsDomain, typename VariableName, typename Number >
std::vector< VariableName > get_addrs_set(
    summary_domain< AbsDomain, VariableName, Number >& inv, VariableName x) {
  assert(!is_unknown_addr(inv, x));
  return inv.get_addrs_set(x);
}

// Refine the addresses of x
template < typename AbsDomain, typename VariableName, typename PtrSet >
void refine_addrs(AbsDomain& inv, VariableName v, PtrSet addrs) {}

template < typename AbsDomain, typename VariableName, typename Number >
void refine_addrs(
    memory_domain< AbsDomain, VariableName, Number >& inv,
    VariableName v,
    typename memory_domain< AbsDomain, VariableName, Number >::points_to_set_t
        addrs) {
  inv.refine_addrs(v, addrs);
}

template < typename AbsDomain, typename VariableName, typename Number >
void refine_addrs(
    summary_domain< AbsDomain, VariableName, Number >& inv,
    VariableName v,
    typename summary_domain< AbsDomain, VariableName, Number >::points_to_set_t
        addrs) {
  inv.refine_addrs(v, addrs);
}

// Refine the addresses and offset of x
template < typename AbsDomain, typename VariableName, typename PtrSet >
void refine_addrs_and_offset(AbsDomain& inv,
                             VariableName v_addr,
                             PtrSet addrs,
                             VariableName v_offset,
                             z_interval offset) {}

template < typename AbsDomain, typename VariableName, typename Number >
void refine_addrs_and_offset(
    memory_domain< AbsDomain, VariableName, Number >& inv,
    VariableName v_addr,
    typename memory_domain< AbsDomain, VariableName, Number >::points_to_set_t
        addrs,
    VariableName v_offset,
    z_interval offset) {
  inv.refine_addrs_and_offset(v_addr, addrs, v_offset, offset);
}

template < typename AbsDomain, typename VariableName, typename Number >
void refine_addrs_and_offset(
    summary_domain< AbsDomain, VariableName, Number >& inv,
    VariableName v_addr,
    typename summary_domain< AbsDomain, VariableName, Number >::points_to_set_t
        addrs,
    VariableName v_offset,
    z_interval offset) {
  inv.refine_addrs_and_offset(v_addr, addrs, v_offset, offset);
}

// memory write (store)
template < typename AbsDomain, typename VariableName, typename Number >
void mem_write(AbsDomain& inv,
               VariableName offset,
               Number size,
               typename AbsDomain::linear_expression_t e,
               bool is_pointer) {}

template < typename AbsDomain, typename VariableName, typename Number >
void mem_write(memory_domain< AbsDomain, VariableName, Number >& inv,
               VariableName offset,
               Number size,
               typename memory_domain< AbsDomain,
                                       VariableName,
                                       Number >::linear_expression_t e,
               bool is_pointer) {
  inv.mem_write(offset, size, e, is_pointer);
}

template < typename AbsDomain, typename VariableName, typename Number >
void mem_write(summary_domain< AbsDomain, VariableName, Number >& inv,
               VariableName offset,
               Number size,
               typename summary_domain< AbsDomain,
                                        VariableName,
                                        Number >::linear_expression_t e,
               bool is_pointer) {
  inv.mem_write(offset, size, e, is_pointer);
}

// memory read (load)
template < typename AbsDomain, typename VariableName, typename Number >
void mem_read(AbsDomain& inv,
              VariableName lhs,
              VariableName offset,
              Number size,
              bool is_pointer) {}

template < typename AbsDomain, typename VariableName, typename Number >
void mem_read(memory_domain< AbsDomain, VariableName, Number >& inv,
              VariableName lhs,
              VariableName offset,
              Number size,
              bool is_pointer) {
  inv.mem_read(lhs, offset, size, is_pointer);
}

template < typename AbsDomain, typename VariableName, typename Number >
void mem_read(summary_domain< AbsDomain, VariableName, Number >& inv,
              VariableName lhs,
              VariableName offset,
              Number size,
              bool is_pointer) {
  inv.mem_read(lhs, offset, size, is_pointer);
}

// memory copy
template < typename AbsDomain, typename VariableName >
void mem_copy(AbsDomain& inv,
              VariableName dest,
              VariableName src,
              typename AbsDomain::linear_expression_t size) {}

template < typename AbsDomain, typename VariableName, typename Number >
void mem_copy(memory_domain< AbsDomain, VariableName, Number >& inv,
              VariableName dest,
              VariableName src,
              typename memory_domain< AbsDomain,
                                      VariableName,
                                      Number >::linear_expression_t size) {
  inv.mem_copy(dest, src, size);
}

template < typename AbsDomain, typename VariableName, typename Number >
void mem_copy(summary_domain< AbsDomain, VariableName, Number >& inv,
              VariableName dest,
              VariableName src,
              typename summary_domain< AbsDomain,
                                       VariableName,
                                       Number >::linear_expression_t size) {
  inv.mem_copy(dest, src, size);
}

// memory set
template < typename AbsDomain, typename VariableName >
void mem_set(AbsDomain& inv,
             VariableName dest,
             typename AbsDomain::linear_expression_t value,
             typename AbsDomain::linear_expression_t size) {}

template < typename AbsDomain, typename VariableName, typename Number >
void mem_set(memory_domain< AbsDomain, VariableName, Number >& inv,
             VariableName dest,
             typename memory_domain< AbsDomain,
                                     VariableName,
                                     Number >::linear_expression_t value,
             typename memory_domain< AbsDomain,
                                     VariableName,
                                     Number >::linear_expression_t size) {
  inv.mem_set(dest, value, size);
}

template < typename AbsDomain, typename VariableName, typename Number >
void mem_set(summary_domain< AbsDomain, VariableName, Number >& inv,
             VariableName dest,
             typename summary_domain< AbsDomain,
                                      VariableName,
                                      Number >::linear_expression_t value,
             typename summary_domain< AbsDomain,
                                      VariableName,
                                      Number >::linear_expression_t size) {
  inv.mem_set(dest, value, size);
}

// compare memory addresses
template < typename AbsDomain, typename VariableName >
void cmp_mem_addr(AbsDomain& inv, CompOp pred, VariableName x, VariableName y) {
}

template < typename AbsDomain, typename VariableName, typename Number >
void cmp_mem_addr(memory_domain< AbsDomain, VariableName, Number >& inv,
                  CompOp pred,
                  VariableName x,
                  VariableName y) {
  switch (pred) {
    case eq:
      inv.cmp_mem_addr(true, x, y);
      break;
    case ne:
      inv.cmp_mem_addr(false, x, y);
      break;
    default:;
  }
}

template < typename AbsDomain, typename VariableName, typename Number >
void cmp_mem_addr(summary_domain< AbsDomain, VariableName, Number >& inv,
                  CompOp pred,
                  VariableName x,
                  VariableName y) {
  switch (pred) {
    case eq:
      inv.cmp_mem_addr(true, x, y);
      break;
    case ne:
      inv.cmp_mem_addr(false, x, y);
      break;
    default:;
  }
}

// compare memory address with null
template < typename AbsDomain, typename VariableName >
void cmp_mem_addr_null(AbsDomain& inv, CompOp pred, VariableName x) {}

template < typename AbsDomain, typename VariableName, typename Number >
void cmp_mem_addr_null(memory_domain< AbsDomain, VariableName, Number >& inv,
                       CompOp pred,
                       VariableName x) {
  switch (pred) {
    case eq:
      inv.cmp_mem_addr_null(true, x);
      break;
    case ne:
      inv.cmp_mem_addr_null(false, x);
      break;
    default:;
  }
}

template < typename AbsDomain, typename VariableName, typename Number >
void cmp_mem_addr_null(summary_domain< AbsDomain, VariableName, Number >& inv,
                       CompOp pred,
                       VariableName x) {
  switch (pred) {
    case eq:
      inv.cmp_mem_addr_null(true, x);
      break;
    case ne:
      inv.cmp_mem_addr_null(false, x);
      break;
    default:;
  }
}

// Similar to operator-= but forgetting only the "surface" part of
// the underlying domain. That is, if a variable represents a
// pointer it will forget its base address and offset but not
// memory contents.
template < typename AbsDomain, typename VariableName >
void forget_mem_surface(AbsDomain& inv, VariableName p) {}

template < typename AbsDomain, typename VariableName, typename Number >
void forget_mem_surface(memory_domain< AbsDomain, VariableName, Number >& inv,
                        VariableName p) {
  inv.forget_mem_surface(p);
}

template < typename AbsDomain, typename VariableName, typename Number >
void forget_mem_surface(summary_domain< AbsDomain, VariableName, Number >& inv,
                        VariableName p) {
  inv.forget_mem_surface(p);
}

// Similar to operator-= but forgetting only the memory contents
template < typename AbsDomain, typename VariableName >
void forget_mem_contents(AbsDomain& inv, VariableName p) {}

template < typename AbsDomain, typename VariableName, typename Number >
void forget_mem_contents(memory_domain< AbsDomain, VariableName, Number >& inv,
                         VariableName p) {
  inv.forget_mem_contents(p);
}

template < typename AbsDomain, typename VariableName, typename Number >
void forget_mem_contents(summary_domain< AbsDomain, VariableName, Number >& inv,
                         varname_t p) {
  inv.forget_mem_contents(p);
}

// Similar to operator-= but forgetting only all the memory contents
// included in the range [p, ..., p + len - 1]. The value of len is given
// in bytes.
template < typename AbsDomain, typename VariableName >
void forget_mem_contents(AbsDomain& inv, VariableName p, ikos::z_number len) {}

template < typename AbsDomain, typename VariableName, typename Number >
void forget_mem_contents(memory_domain< AbsDomain, VariableName, Number >& inv,
                         VariableName p,
                         ikos::z_number len) {
  inv.forget_mem_contents(p, len);
}

template < typename AbsDomain, typename VariableName, typename Number >
void forget_mem_contents(summary_domain< AbsDomain, VariableName, Number >& inv,
                         varname_t p,
                         ikos::z_number len) {
  inv.forget_mem_contents(p, len);
}

////
// Uninitialized variable analysis
////

template < typename AbsDomain, typename VariableName >
void make_uninitialized(AbsDomain& inv, VariableName x) {}

template < typename AbsDomain, typename VariableName, typename Number >
void make_uninitialized(memory_domain< AbsDomain, VariableName, Number >& inv,
                        VariableName x) {
  inv.make_uninitialized(x);
}

template < typename AbsDomain, typename VariableName, typename Number >
void make_uninitialized(summary_domain< AbsDomain, VariableName, Number >& inv,
                        VariableName x) {
  inv.make_uninitialized(x);
}

template < typename AbsDomain, typename VariableName >
void make_initialized(AbsDomain& inv, VariableName x) {}

template < typename AbsDomain, typename VariableName, typename Number >
void make_initialized(memory_domain< AbsDomain, VariableName, Number >& inv,
                      VariableName x) {
  inv.make_initialized(x);
}

template < typename AbsDomain, typename VariableName, typename Number >
void make_initialized(summary_domain< AbsDomain, VariableName, Number >& inv,
                      VariableName x) {
  inv.make_initialized(x);
}

template < typename AbsDomain, typename VariableName >
void assign_uninitialized(AbsDomain& inv,
                          VariableName x,
                          std::vector< VariableName > ys) {}

template < typename AbsDomain, typename VariableName, typename Number >
void assign_uninitialized(memory_domain< AbsDomain, VariableName, Number >& inv,
                          VariableName x,
                          std::vector< VariableName > ys) {
  inv.assign_uninitialized(x, ys);
}

template < typename AbsDomain, typename VariableName, typename Number >
void assign_uninitialized(
    summary_domain< AbsDomain, VariableName, Number >& inv,
    VariableName x,
    std::vector< VariableName > ys) {
  inv.assign_uninitialized(x, ys);
}

// true iff x is definitely uninitialized.
template < typename AbsDomain, typename VariableName >
bool is_uninitialized(AbsDomain inv, VariableName x) {
  return false;
}

template < typename AbsDomain, typename VariableName, typename Number >
bool is_uninitialized(memory_domain< AbsDomain, VariableName, Number > inv,
                      VariableName x) {
  return inv.is_uninitialized(x);
}

template < typename AbsDomain, typename VariableName, typename Number >
bool is_uninitialized(summary_domain< AbsDomain, VariableName, Number > inv,
                      VariableName x) {
  return inv.is_uninitialized(x);
}

// true iff x is definitely initialized.
template < typename AbsDomain, typename VariableName >
bool is_initialized(AbsDomain inv, VariableName x) {
  return false;
}

template < typename AbsDomain, typename VariableName, typename Number >
bool is_initialized(memory_domain< AbsDomain, VariableName, Number > inv,
                    VariableName x) {
  return inv.is_initialized(x);
}

template < typename AbsDomain, typename VariableName, typename Number >
bool is_initialized(summary_domain< AbsDomain, VariableName, Number > inv,
                    VariableName x) {
  return inv.is_initialized(x);
}

////
// Nullity analysis
////

template < typename AbsDomain, typename VariableName >
void make_null(AbsDomain& inv, VariableName x) {}

template < typename AbsDomain, typename VariableName, typename Number >
void make_null(memory_domain< AbsDomain, VariableName, Number >& inv,
               VariableName x) {
  inv.make_null(x);
}

template < typename AbsDomain, typename VariableName, typename Number >
void make_null(summary_domain< AbsDomain, VariableName, Number >& inv,
               VariableName x) {
  inv.make_null(x);
}

template < typename AbsDomain, typename VariableName >
void make_non_null(AbsDomain& inv, VariableName x) {}

template < typename AbsDomain, typename VariableName, typename Number >
void make_non_null(memory_domain< AbsDomain, VariableName, Number >& inv,
                   VariableName x) {
  inv.make_non_null(x);
}

template < typename AbsDomain, typename VariableName, typename Number >
void make_non_null(summary_domain< AbsDomain, VariableName, Number >& inv,
                   VariableName x) {
  inv.make_non_null(x);
}

template < typename AbsDomain, typename VariableName >
void assign_nullity(AbsDomain& inv, VariableName x, VariableName y) {}

template < typename AbsDomain, typename VariableName, typename Number >
void assign_nullity(memory_domain< AbsDomain, VariableName, Number >& inv,
                    VariableName x,
                    VariableName y) {
  inv.assign_nullity(x, y);
}

template < typename AbsDomain, typename VariableName, typename Number >
void assign_nullity(summary_domain< AbsDomain, VariableName, Number >& inv,
                    VariableName x,
                    VariableName y) {
  inv.assign_nullity(x, y);
}

// true iff x is definitely null.
template < typename AbsDomain, typename VariableName >
bool is_null(AbsDomain inv, VariableName x) {
  return false;
}

template < typename AbsDomain, typename VariableName, typename Number >
bool is_null(memory_domain< AbsDomain, VariableName, Number > inv,
             VariableName x) {
  return inv.is_null(x);
}

template < typename AbsDomain, typename VariableName, typename Number >
bool is_null(summary_domain< AbsDomain, VariableName, Number > inv,
             VariableName x) {
  return inv.is_null(x);
}

// true iff x is definitely non null.
template < typename AbsDomain, typename VariableName >
bool is_non_null(AbsDomain inv, VariableName x) {
  return false;
}

template < typename AbsDomain, typename VariableName, typename Number >
bool is_non_null(memory_domain< AbsDomain, VariableName, Number > inv,
                 VariableName x) {
  return inv.is_non_null(x);
}

template < typename AbsDomain, typename VariableName, typename Number >
bool is_non_null(summary_domain< AbsDomain, VariableName, Number > inv,
                 VariableName x) {
  return inv.is_non_null(x);
}

} // end namespace value_domain_impl

namespace ar_size {

//! Return the size in bytes.
//  The returned size is extracted from the AR type.
inline uint64_t getAllocSize(const Type_ref& type) {
  if (ar::ar_internal::is_pointer_type(type)) {
    Pointer_Type_ref ptr_type = node_cast< AR_Pointer_Type >(type);
    return ar::getSize(ar::getPointeeType(ptr_type));
  } else {
    return ar::getSize(type);
  }
}

//! Return the allocated size (in bytes) by the global or alloca
//  The returned size is extracted from the AR type.
inline uint64_t getAllocSize(const Operand_ref& o) {
  if (ar::ar_internal::is_cst_operand(o)) {
    Cst_Operand_ref cst_o = node_cast< AR_Cst_Operand >(o);
    Constant_ref cst = ar::getConstant(cst_o);
    if (ar::ar_internal::is_var_addr_constant(cst)) {
      Var_Addr_Constant_ref var_cst = node_cast< Var_Addr_Constant >(cst);
      return getAllocSize(ar::getType(var_cst));
    } else if (ar::ar_internal::is_function_addr_constant(cst)) {
      return (uint64_t)0;
    }
  }
  assert(false && "Cannot determine allocated size");
  exit(EXIT_FAILURE);
}

} // end namespace ar_size

namespace num_sym_exec_impl {
//! Shadow variable to propagate the _whole_ memory allocation size
//  of an alloca, global variable or a malloc-like allocation site.
template < typename VariableName >
inline VariableName get_shadow_size(VariableName v) {
  // All shadow variables must start with prefix "shadow"
  return v.getVarFactory()["shadow." + v.name() + ".size"];
}

//! Return the size of the object to which v points to. Note that v
//  may point to more than one object so we join the sizes of all.
template < typename AbsValueDomain, typename VariableName >
z_interval getPtrAllocSize(AbsValueDomain inv, VariableName ptr) {
  if (value_domain_impl::is_unknown_addr(inv, ptr))
    return z_interval::top();

  std::vector< VariableName > ptrSet =
      value_domain_impl::get_addrs_set(inv, ptr);
  assert(!ptrSet.empty());

  z_interval size = z_interval::bottom();
  for (typename std::vector< VariableName >::iterator I = ptrSet.begin(),
                                                      E = ptrSet.end();
       I != E;
       ++I) {
    VariableName v = num_sym_exec_impl::get_shadow_size(*I);
    size = size | num_abstract_domain_impl::to_interval(inv, v);
  }
  return size;
}

} // end namespace

using namespace num_sym_exec_impl;

template < typename AbsValueDomain, typename VariableName, typename Number >
class num_sym_exec : public sym_exec< AbsValueDomain >,
                     public boost::noncopyable {
  /*

    This class performs the transfer function on each statement.  It
    relies on a value domain (template parameter AbsValueDomain) which
    keeps track of the following information:

    - For an integer scalar x:

      - A range that overapproximates the value of x. The
        representation of the range depends on the underlying
        numerical domain.

      - Whether x might be uninitialized or not.

    - For a pointer scalar p (only if _prec_level >= PTR):

      - The offset from the base address of the object that contains
        p. For this, we add "p" in the underlying numerical domain
        where "p" actually represents the p's offset.

      - The actual size of the allocated memory for p (including
        padding). For this, we add a shadow variable that keeps track
        of the allocated size by the memory object (&'s and
        mallocs) associated with p in the underlying numerical domain.

      - The address of p via a set of memory objects (&'s and mallocs)
        to which p may point to (ie., points-to sets)

      - Whether p might be null or not.

      - In addition to this, if _prec_level == MEM, it also keeps
        track of the content of p (i.e., *p). This handled internally
        by the value analysis (Load and Store).

   */

  typedef typename AbsValueDomain::variable_t variable_t;
  typedef typename AbsValueDomain::linear_expression_t linExpr;
  typedef typename AbsValueDomain::linear_constraint_t linCst;
  typedef interval< Number > interval_t;

  AbsValueDomain _inv;
  VariableFactory& _vfac;
  LiteralFactory& _lfac;
  const TrackedPrecision _prec_level;
  PointerInfo _pointer;
  std::vector< VariableName > _dead_vars;

  typedef boost::unordered_set< Operand_ref,
                                Operand_Hasher::hash,
                                Operand_Hasher::eq > operand_set_t;

  operand_set_t _mem_objects; //! keep track of memory objects

  enum nullity_value_t { MUSTNULL, MUSTNONNULL, MAYNULL };

  void keep_alive(VariableName x) {
    _dead_vars.erase(std::remove(_dead_vars.begin(), _dead_vars.end(), x),
                     _dead_vars.end());
  }

  //! Allocate a new memory object ptr with unknown size. Return true
  //  if the memory object has not been seen before.
  //
  //  We consider as a memory object an alloca (i.e., stack
  //  variables), global variables, malloc-like allocation sites,
  //  function pointers, and destination of inttoptr
  //  instructions. Also, variables whose address might have been
  //  taken are translated to global variables by the front-end.
  bool _make_mem_object(AbsValueDomain& inv,
                        Operand_ref ptr,
                        LiteralFactory& lfac,
                        nullity_value_t NullVal) {
    if (_mem_objects.find(ptr) != _mem_objects.end())
      return false;
    _mem_objects.insert(ptr);

    Literal lPtr = lfac[ptr];

    // update nullity
    if (NullVal == MUSTNULL)
      value_domain_impl::make_null(inv, lPtr.get_var());
    else if (NullVal == MUSTNONNULL)
      value_domain_impl::make_non_null(inv, lPtr.get_var());

    // zero offset in the numerical abstraction
    inv.assign(lPtr.get_var(), Literal::make_num< Number >((uint64_t)0));

    // pointer info: create a new memory object
    value_domain_impl::make_object(inv, lPtr.get_var());

    // make sure that the scope of the memory object survives the rest
    // of the program.
    keep_alive(lPtr.get_var());

    return true;
  }

  //! Allocate a new memory object ptr of size AllocSize (in bytes).
  //  If AllocSize is empty then the size is extracted statically from
  //  the AR type.
  void make_mem_object(AbsValueDomain& inv,
                       Operand_ref ptr,
                       LiteralFactory& lfac,
                       nullity_value_t NullVal,
                       boost::optional< ikos::z_number > AllocSize =
                           boost::optional< ikos::z_number >()) {
    if (_make_mem_object(inv, ptr, lfac, NullVal)) {
      Literal lPtr = lfac[ptr];
      if (AllocSize) {
        inv.assign(get_shadow_size(lPtr.get_var()), *AllocSize);
      } else {
        inv.assign(get_shadow_size(lPtr.get_var()), ar_size::getAllocSize(ptr));
      }
    }
  }

  //! Allocate a new memory object ptr of size AllocSize
  void make_mem_object(AbsValueDomain& inv,
                       Operand_ref ptr,
                       LiteralFactory& lfac,
                       nullity_value_t NullVal,
                       VariableName AllocSize) {
    if (_make_mem_object(inv, ptr, lfac, NullVal)) {
      Literal lPtr = lfac[ptr];
      _inv.assign(get_shadow_size(lPtr.get_var()), linExpr(AllocSize));
    }
  }

  //! Abstract all the memory contents between {base,...,(base+len)-1}
  //  pre: len is in bytes
  void abstract_memory(Operand_ref base, Operand_ref len) {
    Literal Base = _lfac[base];
    assert(Base.is_var());
    Literal Len = _lfac[len];

    // If len is not constant we do not bother and remove anything
    // touched by base
    ikos::z_number n = -1;
    if (Len.is_var()) {
      boost::optional< ikos::z_number > len =
          num_abstract_domain_impl::to_interval(_inv, Len.get_var())
              .singleton();
      if (len) {
        n = *len;
      }
    } else if (Len.is_num()) {
      n = Len.get_num< ikos::z_number >();
    }

    if (n < 0)
      value_domain_impl::forget_mem_contents(_inv, Base.get_var());
    else
      value_domain_impl::forget_mem_contents(_inv, Base.get_var(), n);
  }

  //! Model an assignment lhs := rhs
  void Assign(const Literal& lhs, const Operand_ref& rhs, bool IsPointer) {
    Literal rhs_lit = _lfac[rhs];
    VariableName lhs_var = lhs.get_var();

    if (rhs_lit.is_undefined_cst()) {
      _inv -= lhs_var;
      value_domain_impl::make_uninitialized(_inv, lhs_var);
      return;
    }

    if (_prec_level >= PTR && IsPointer) {
      if (rhs_lit.is_null_cst()) {
        value_domain_impl::forget_mem_surface(_inv, lhs_var);
        value_domain_impl::make_null(_inv, lhs_var);
        return;
      }

      assert(rhs_lit.is_var());

      // Create lazily a new memory object only if global, alloca or
      // function pointer.
      if (ar::isGlobalVar(rhs) || ar::isAllocaVar(rhs)) {
        make_mem_object(_inv, rhs, _lfac, MUSTNONNULL);
      } else if (ar::isFunctionPointer(rhs)) {
        make_mem_object(_inv,
                        rhs,
                        _lfac,
                        MUSTNONNULL,
                        /*no size if function pointer*/
                        Literal::make_num< Number >(0));
      }

      // TODO: wrap these three operations into
      //       value_domain_impl::assign_ptr

      // update offset in the numerical abstraction
      _inv.assign(lhs_var, linExpr(rhs_lit.get_var()));

      // update pointer info
      value_domain_impl::assign_ptr(_inv, lhs_var, rhs_lit.get_var());

      // update nullity
      value_domain_impl::assign_nullity(_inv, lhs_var, rhs_lit.get_var());
      return;
    }

    if (!IsPointer) {
      if (rhs_lit.is_var()) {
        // update numerical abstraction
        _inv.assign(lhs_var, linExpr(rhs_lit.get_var()));

        // update uninitialized variables
        std::vector< VariableName > vs;
        vs.push_back(rhs_lit.get_var());
        value_domain_impl::assign_uninitialized(_inv, lhs_var, vs);
      } else if (rhs_lit.is_num()) {
        // update numerical abstraction
        _inv.assign(lhs_var, rhs_lit.get_num< Number >());

        // update uninitialized variables
        value_domain_impl::make_initialized(_inv, lhs_var);
      } else {
        _inv -= lhs_var;
      }
    }
  }

  template < typename Op, typename Stmt_ref >
  void binary_operation(Op op, Stmt_ref stmt) {
    /// Execute a binary operation in which all operands are integer
    /// scalars but not pointers.

    VariableName res = _vfac[ar::getName(ar::getResult(stmt))];
    Literal left = _lfac[ar::getLeftOp(stmt)];
    Literal right = _lfac[ar::getRightOp(stmt)];

    if (left.is_undefined_cst() || right.is_undefined_cst()) {
      _inv -= res;
      value_domain_impl::make_uninitialized(_inv, res);
      return;
    }

    assert((left.is_var() || left.is_num()) &&
           (right.is_var() || right.is_num()));

    if (left.is_var() && right.is_var()) {
      // TODO: do together apply/assign and propagation of
      //       uninitialized info

      // update numerical abstraction
      _inv.apply(op, res, left.get_var(), right.get_var());

      // update uninitialized variables
      std::vector< VariableName > vs;
      vs.push_back(left.get_var());
      vs.push_back(right.get_var());
      value_domain_impl::assign_uninitialized(_inv, res, vs);
    } else if (left.is_var() && right.is_num()) {
      // update numerical abstraction
      _inv.apply(op, res, left.get_var(), right.get_num< Number >());

      // update uninitialized variables
      std::vector< VariableName > vs;
      vs.push_back(left.get_var());
      value_domain_impl::assign_uninitialized(_inv, res, vs);
    } else if (left.is_num() && right.is_var()) {
      // update numerical abstraction
      _inv.assign(res, left.get_num< Number >());
      _inv.apply(op, res, res, right.get_var());

      // update uninitialized variables
      std::vector< VariableName > vs;
      vs.push_back(right.get_var());
      value_domain_impl::assign_uninitialized(_inv, res, vs);
    } else if (left.is_num() && right.is_num()) {
      // update numerical abstraction
      _inv.assign(res, left.get_num< Number >());
      _inv.apply(op, res, res, right.get_num< Number >());

      // update uninitialized variables
      value_domain_impl::make_initialized(_inv, res);
    }
  }

  std::vector< linCst > make_linear_cst(CompOp pred,
                                        bool isPositive,
                                        linExpr X,
                                        linExpr Y) {
    linCst cst;
    switch (pred) {
      case eq:
        cst = (isPositive ? X == Y : X != Y);
        break;
      case ne:
        cst = (isPositive ? X != Y : X == Y);
        break;
      case ult:
      case slt:
        cst = (isPositive ? X <= Y - 1 : X >= Y);
        break;
      case ule:
      case sle:
        cst = (isPositive ? X <= Y : X >= Y + 1);
        break;
      default: {
        assert(false && "unreachable");
        exit(EXIT_FAILURE);
      }
    }

    std::vector< linCst > csts;
    csts.push_back(cst);
    if (isPositive && (pred == ult || pred == ule))
      csts.push_back(linCst(X >= 0));

    return csts;
  }

  std::vector< linCst > make_linear_cst(CompOp pred,
                                        bool isPositive,
                                        Operand_ref X,
                                        Operand_ref Y) {
    Literal lX = _lfac[X];
    Literal lY = _lfac[Y];

    assert((lX.is_var() || lX.is_num()) && (lY.is_var() || lY.is_num()));

    if (lX.is_var() && lY.is_var()) {
      return make_linear_cst(pred,
                             isPositive,
                             linExpr(lX.get_var()),
                             linExpr(lY.get_var()));
    }

    if (lX.is_var() && lY.is_num()) {
      return make_linear_cst(pred,
                             isPositive,
                             linExpr(lX.get_var()),
                             linExpr(lY.get_num< Number >()));
    }

    if (lX.is_num() && lY.is_var()) {
      return make_linear_cst(pred,
                             isPositive,
                             linExpr(lX.get_num< Number >()),
                             linExpr(lY.get_var()));
    }

    if (lX.is_num() && lY.is_num()) {
      return make_linear_cst(pred,
                             isPositive,
                             linExpr(lX.get_num< Number >()),
                             linExpr(lY.get_num< Number >()));
    }

    assert(false && "unreachable");
    exit(EXIT_FAILURE);
  }

  //! Refine the addresses of ptr using information from an external
  //! pointer analysis. This is vital to improve the value analysis
  //! because it can only reason about pointers to integers but not
  //! about pointers to pointers.
  bool refineAddr(VariableName ptr) {
    std::pair< PointerInfo::ptr_set_t, z_interval > ptr_info = _pointer[ptr];
    if (ptr_info.first.is_top())
      return false;
    value_domain_impl::refine_addrs(_inv, ptr, ptr_info.first);
    return true;
  }

  //! Refine the addresses and offset of ptr using information from an
  //! external pointer analysis.
  bool refineAddrAndOffset(VariableName ptr) {
    std::pair< PointerInfo::ptr_set_t, z_interval > ptr_info = _pointer[ptr];
    if (ptr_info.first.is_top())
      return false;
    value_domain_impl::refine_addrs_and_offset(_inv,
                                               ptr,
                                               ptr_info.first,
                                               ptr,
                                               ptr_info.second);
#if 0                                      
    std::cout << "REFINE addresses of " << ptr
         << " with " << "(" << ptr_info.first << "," 
         << ptr_info.second << ")" << "\n";
#endif
    return true;
  }

public:
  void match_down(IvRange formals, OpRange actuals) {
    assert(formals.size() == actuals.size());

    IvRange::iterator F = formals.begin();
    OpRange::iterator A = actuals.begin();
    for (; F != formals.end(); ++F, ++A) {
      Literal Flit = _lfac[*F];
      Assign(Flit, *A, ar::isPointer(*A));
    }
  }

  void match_up(OpRange /*actuals*/,
                boost::optional< Internal_Variable_ref > lhs_cs_ret,
                IvRange /*formals*/,
                boost::optional< Operand_ref > callee_ret) {
    if (lhs_cs_ret && callee_ret) {
      Assign(_lfac[*lhs_cs_ret], *callee_ret, ar::isPointer(*lhs_cs_ret));
    }
  }

  num_sym_exec(
      AbsValueDomain inv,
      VariableFactory& vfac,
      LiteralFactory& lfac,
      TrackedPrecision prec_level,
      PointerInfo pointer = PointerInfo(),
      std::vector< VariableName > dead_vars = std::vector< VariableName >())
      : sym_exec< AbsValueDomain >(),
        _inv(inv),
        _vfac(vfac),
        _lfac(lfac),
        _prec_level(prec_level),
        _pointer(pointer),
        _dead_vars(dead_vars) {}

  AbsValueDomain inv() { return _inv; }

  void set_inv(AbsValueDomain inv) { _inv = inv; }

  void exec_start(Basic_Block_ref b) {}

  void exec_end(Basic_Block_ref b) {
// Note that _dead_vars do not contain shadow variables (internal
// variables added by the analyses) since shadow variables do not
// appear in the ARBOS Cfg.
#if 0
    std::cout << "Removing dead variables at exit of block " << ar::getName (b) << "{";
    for(typename std::vector<VariableName>::iterator I=_dead_vars.begin();
        I!=_dead_vars.end();++I)
    { std::cout << *I << ";"; }
    std::cout << "}\n";
#endif
    for (typename std::vector< VariableName >::iterator it = _dead_vars.begin();
         it != _dead_vars.end();
         ++it) {
      value_domain_impl::forget_mem_surface(_inv, *it);
    }
  }

  void exec(Arith_Op_ref stmt) {
    if (boost::optional< ikos::operation_t > o =
            translate_op(ar::getArithOp(stmt))) {
      // addition, subtraction, multiplication and (signed) division
      binary_operation< ikos::operation_t, Arith_Op_ref >(*o, stmt);
    } else {
      // sdiv, udiv, srem, urem
      div_operation_t op;
      switch (ar::getArithOp(stmt)) {
        case udiv:
          op = OP_UDIV;
          break;
        case sdiv:
          op = OP_SDIV;
          break;
        case urem:
          op = OP_UREM;
          break;
        case srem:
          op = OP_SREM;
          break;
        default: {
          assert(false && "unreachable");
          exit(EXIT_FAILURE);
        }
      }
      binary_operation< div_operation_t, Arith_Op_ref >(op, stmt);
    }
  }

  void exec(Integer_Comparison_ref stmt) {
    Operand_ref X = ar::getLeftOp(stmt);
    Operand_ref Y = ar::getRightOp(stmt);
    CompOp pred = ar::getPredicate(stmt);
    bool isPos = ar::isPositive(stmt);

    if (ar::ar_internal::is_undefined(X) || ar::ar_internal::is_undefined(Y))
      return;

    if (ar::isPointer(X) || ar::isPointer(Y)) {
      if (_prec_level == REG)
        return;

      // reason only about equalities and disequalities between
      // pointers
      if (pred != eq && pred != ne)
        return;

      CompOp n_pred;

      if (pred == eq)
        n_pred = (isPos) ? eq : ne;
      if (pred == ne)
        n_pred = (isPos) ? ne : eq;

      Literal lX = _lfac[X];
      Literal lY = _lfac[Y];

      // cases like lX.is_null_cst && lY.is_null_cst should be handle
      // by the front-end so we do not bother here.

      if (lX.is_var() && lY.is_null_cst()) { // x == null or x != null
        // reduction with the external pointer analysis
        refineAddr(lX.get_var());
        value_domain_impl::cmp_mem_addr_null(_inv, n_pred, lX.get_var());
      } else if (lX.is_null_cst() && lY.is_var()) { // y == null or y != null
        // reduction with the external pointer analysis
        refineAddr(lY.get_var());
        value_domain_impl::cmp_mem_addr_null(_inv, n_pred, lY.get_var());
      } else if (lX.is_var() && lY.is_var()) {
        // reduction with the external pointer analysis
        refineAddr(lX.get_var());
        refineAddr(lY.get_var());
        value_domain_impl::cmp_mem_addr(_inv,
                                        n_pred,
                                        lX.get_var(),
                                        lY.get_var());
      }
    } else {
      // X and Y are registers.
      std::vector< linCst > csts = make_linear_cst(pred, isPos, X, Y);
      for (typename std::vector< linCst >::iterator I = csts.begin(),
                                                    E = csts.end();
           I != E;
           ++I) {
        _inv += *I;
      }
    }
  }

  void exec(Bitwise_Op_ref stmt) {
    bitwise_operation_t op;
    switch (ar::getBitwiseOp(stmt)) {
      case _and:
        op = OP_AND;
        break;
      case _or:
        op = OP_OR;
        break;
      case _xor:
        op = OP_XOR;
        break;
      case _shl:
        op = OP_SHL;
        break;
      case _lshr:
        op = OP_LSHR;
        break;
      case _ashr:
        op = OP_ASHR;
        break;
      default: {
        assert(false && "unreachable");
        exit(EXIT_FAILURE);
      }
    }
    binary_operation< bitwise_operation_t, Bitwise_Op_ref >(op, stmt);
  }

  void exec(Abstract_Variable_ref stmt) {
    VariableName n = _vfac[ar::getName(stmt)];
    _inv -= n;
  }

  void exec(Landing_Pad_ref stmt) {
    Internal_Variable_ref exc = ar::getVar(stmt);
    VariableName exc_var = _vfac[ar::getName(exc)];
    _inv -= exc_var;
  }

  void exec(Resume_ref /*stmt*/) { _inv = AbsValueDomain::bottom(); }

  void exec(Unreachable_ref /*stmt*/) { _inv = AbsValueDomain::bottom(); }

  void exec(Assignment_ref stmt) {
    Internal_Variable_ref lhs = ar::getLeftOp(stmt);
    Operand_ref rhs = ar::getRightOp(stmt);

    Assign(this->_lfac[lhs], rhs, ar::isPointer(lhs));
  }

  void exec(Conv_Op_ref stmt) {
    Var_Operand_ref dest = node_cast< Var_Operand >(ar::getLeftOp(stmt));
    VariableName dest_var = _vfac[ar::getName(dest)];
    Operand_ref src = ar::getRightOp(stmt);
    Literal lSrc = _lfac[src];

    if (!(lSrc.is_var() || lSrc.is_num())) {
      _inv -= dest_var;
      return;
    }

    conv_operation_t op;
    switch (ar::getConvOp(stmt)) {
      // Pointer casts
      case inttoptr: {
        /*
           from code like int x = 5; int *px =x;
        */
        if (_prec_level < PTR)
          return;
        make_mem_object(_inv,
                        ar::getLeftOp(stmt),
                        _lfac,
                        MUSTNONNULL,
                        /*should be probably better pointer size*/
                        Literal::make_num< Number >(8));
        return;
      }
      case bitcast: {
        if (!ar::isPointer(dest)) {
          _inv -= dest_var;
          return;
        }

        if (_prec_level >= PTR) {
          Assign(_lfac[ar::getLeftOp(stmt)], src, true);
        }
        return;
      }
      case ::arbos::trunc:
        op = OP_TRUNC;
        break;
      case zext:
        op = OP_ZEXT;
        break;
      case sext:
        op = OP_SEXT;
        break;
      // case ptrtoint
      default:
        _inv -= dest_var;
        return;
    }

    /////
    /// Trunc, ZExt, or SExt for scalars.
    /////

    uint64_t Width = ar::getSize(ar::getType(dest));

    if (lSrc.is_var()) {
      // update numerical abstraction
      _inv.apply(op, dest_var, lSrc.get_var(), Width);

      // update uninitialized variables
      std::vector< VariableName > vs;
      vs.push_back(lSrc.get_var());
      value_domain_impl::assign_uninitialized(_inv, dest_var, vs);
    } else if (lSrc.is_num()) {
      // update numerical abstraction
      _inv.apply(op, dest_var, lSrc.get_num< Number >(), Width);

      // update uninitialized variables
      value_domain_impl::make_initialized(_inv, dest_var);
    }
  }

  void exec(Pointer_Shift_ref stmt) {
    if (_prec_level < PTR)
      return;

    // This statement performs pointer arithmetic lhs = base + offset
    //
    // 1) no uninitialized variable information is propagated here
    //    since lhs is always a pointer.
    //
    // 2) lhs is null iff base is null regardless offset.
    //
    //    ASSUMPTION: this is only true if we can prove separately
    //    that base+offset cannot be out-of-bounds.
    //
    // 3) points-to[lhs] = points-to[base]
    //
    // 4) In the numerical abstraction, lhs is base plus offset.
    //

    Operand_ref base = ar::getBase(stmt);
    Operand_ref offset = ar::getOffset(stmt);

    Literal lhs = _lfac[ar::getResult(stmt)];
    Literal lBase = _lfac[base];
    Literal lOffset = _lfac[offset];

    if (lBase.is_undefined_cst()) {
      value_domain_impl::forget_mem_surface(_inv, lhs.get_var());
      value_domain_impl::make_uninitialized(_inv, lhs.get_var());
      return;
    }

    if (lBase.is_null_cst()) {
      value_domain_impl::forget_mem_surface(_inv, lhs.get_var());
      value_domain_impl::make_null(_inv, lhs.get_var());
      return;
    }

    assert(lOffset.is_var() || lOffset.is_num());
    assert(lBase.is_var());

    // Create lazily a memory object
    if (ar::isGlobalVar(base) || ar::isAllocaVar(base)) {
      make_mem_object(_inv, base, _lfac, MUSTNONNULL);
    }

    // TODO: wrap these three operations into
    //       value_domain_impl::apply_ptr

    value_domain_impl::assign_nullity(_inv, lhs.get_var(), lBase.get_var());

    // update the offset: lhs = base + offset
    if (lOffset.is_var()) {
      _inv.apply(ikos::OP_ADDITION,
                 lhs.get_var(),
                 lBase.get_var(),
                 lOffset.get_var());
    } else {
      assert(lOffset.is_num());
      _inv.apply(ikos::OP_ADDITION,
                 lhs.get_var(),
                 lBase.get_var(),
                 lOffset.get_num< Number >());
    }

    // update pointer info
    value_domain_impl::assign_ptr(_inv, lhs.get_var(), lBase.get_var());
  }

private:
  // memcpy  (dest, src, len)
  // memmove (dest, src, len)
  template < typename MemCpyOrMove_ref >
  void execMemCpyOrMove(MemCpyOrMove_ref stmt) {
    if (_prec_level < PTR)
      return;

    // Both src and dest must be already allocated in memory so
    // offsets and sizes for both src and dest are part already of the
    // invariants

    Literal dest = _lfac[ar::getTarget(stmt)];
    Literal src = _lfac[ar::getSource(stmt)];
    Literal size = _lfac[ar::getLen(stmt)];
    assert(dest.is_var() && src.is_var());
    assert(size.is_var() || size.is_num());

    // Create lazily memory objects
    if (ar::isGlobalVar(ar::getTarget(stmt)) ||
        ar::isAllocaVar(ar::getTarget(stmt))) {
      make_mem_object(_inv, ar::getTarget(stmt), _lfac, MUSTNONNULL);
    }
    if (ar::isAllocaVar(ar::getSource(stmt)) ||
        ar::isGlobalVar(ar::getSource(stmt))) {
      make_mem_object(_inv, ar::getSource(stmt), _lfac, MUSTNONNULL);
    }

    // Reduction between value and pointer analysis
    refineAddrAndOffset(dest.get_var());
    refineAddrAndOffset(src.get_var());

    if (_prec_level >= MEM) {
      linExpr size_expr = size.is_var() ? linExpr(size.get_var())
                                        : linExpr(size.get_num< Number >());
      value_domain_impl::mem_copy(_inv,
                                  dest.get_var(),
                                  src.get_var(),
                                  size_expr);
    }
  }

public:
  void exec(MemCpy_ref stmt) { return execMemCpyOrMove(stmt); }

  void exec(MemMove_ref stmt) { return execMemCpyOrMove(stmt); }

  void exec(MemSet_ref stmt) {
    if (_prec_level < PTR)
      return;

    Literal dest = _lfac[ar::getBase(stmt)];
    Literal value = _lfac[ar::getValue(stmt)];
    Literal size = _lfac[ar::getLen(stmt)];
    assert(dest.is_var());
    assert(value.is_var() || value.is_num());
    assert(size.is_var() || size.is_num());

    // Create lazily memory objects
    if (ar::isGlobalVar(ar::getBase(stmt)) ||
        ar::isAllocaVar(ar::getBase(stmt))) {
      make_mem_object(_inv, ar::getBase(stmt), _lfac, MUSTNONNULL);
    }

    // Reduction between value and pointer analysis
    refineAddrAndOffset(dest.get_var());

    if (_prec_level >= MEM) {
      linExpr value_expr = value.is_var() ? linExpr(value.get_var())
                                          : linExpr(value.get_num< Number >());
      linExpr size_expr = size.is_var() ? linExpr(size.get_var())
                                        : linExpr(size.get_num< Number >());
      value_domain_impl::mem_set(_inv, dest.get_var(), value_expr, size_expr);
    }
  }

  //! Analysis of library calls (calls for which code is not
  //! available). TODO: add support for more library calls.
  void exec(Call_ref cs) {
    assert(ar::isExternal(cs) &&
           "This method should only execute external calls");

    boost::optional< Internal_Variable_ref > lhs = ar::getReturnValue(cs);

    ///
    // Here library functions that we want to consider specially.
    ///
    if ((ar::getFunctionName(cs) == "malloc" ||
         ar::getFunctionName(cs) == "_Znwm" ||
         ar::getFunctionName(cs) == "_Znam" ||
         ar::getFunctionName(cs) == "__cxa_allocate_exception") &&
        ar::getNumOfArgs(cs) == 1) {
      // call to malloc, new, new[] or __cxa_allocate_exception
      Operand_ref arg;
      boost::tie(arg) = ar::getUnaryArgs(cs);
      analyze_malloc(lhs, arg);
    } else if ((ar::getFunctionName(cs) == "free" ||
                ar::getFunctionName(cs) == "_ZdlPv" ||
                ar::getFunctionName(cs) == "_ZdaPv" ||
                ar::getFunctionName(cs) == "__cxa_free_exception") &&
               ar::getNumOfArgs(cs) == 1) {
      // call to free, delete, delete[] or __cxa_free_exception
      Operand_ref arg;
      boost::tie(arg) = ar::getUnaryArgs(cs);
      analyze_free(arg);
    } else if (ar::getFunctionName(cs) == "read" && ar::getNumOfArgs(cs) == 3) {
      Operand_ref arg1, arg2, arg3;
      boost::tie(arg1, arg2, arg3) = ar::getTernaryArgs(cs);
      analyze_read(lhs, arg1, arg2, arg3);
    } else { // default case: forget all actual parameters of pointer type
      // (very conservative!)
      if (_prec_level >= PTR) {
        OpRange apars = ar::getArguments(cs);
        for (OpRange::iterator it = apars.begin(), et = apars.end(); it != et;
             ++it) {
          if (ar::isPointer(*it)) {
            Literal p = _lfac[*it];
            if (p.is_var()) {
              value_domain_impl::forget_mem_contents(_inv, p.get_var());
            }
          }
        }
      }

      // forget the lhs
      if (lhs) {
        Literal ret = _lfac[*lhs];
        assert(ret.is_var());
        _inv -= ret.get_var();
        // ASSUMPTION:
        // The claim about the correctness of the program under analysis
        // can be made only if all calls to unavailable code are assumed
        // to be correct and without side-effects.
        // We will assume that the lhs of an external call site is
        // always initialized (if scalar). However, in case of a
        // pointer, we do not assume that a non-null pointer is
        // returned.
        if (!ar::isPointer(*lhs)) {
          value_domain_impl::make_initialized(_inv, ret.get_var());
        }
      }
    }
  }

  void exec(Invoke_ref stmt) { exec(ar::getFunctionCall(stmt)); }

  void exec(Store_ref stmt) {
    Literal lPtr = _lfac[ar::getPointer(stmt)];
    if (lPtr.is_undefined_cst() || lPtr.is_null_cst()) {
      _inv = AbsValueDomain::bottom();
      return;
    }

    if (_prec_level < PTR)
      return;

    // Create lazily memory objects
    if (ar::isAllocaVar(ar::getPointer(stmt)) ||
        ar::isGlobalVar(ar::getPointer(stmt))) {
      make_mem_object(_inv, ar::getPointer(stmt), _lfac, MUSTNONNULL);
    }
    if (ar::isGlobalVar(ar::getValue(stmt)) ||
        ar::isAllocaVar(ar::getValue(stmt))) {
      make_mem_object(_inv, ar::getValue(stmt), _lfac, MUSTNONNULL);
    }

    // Reduction between value and pointer analysis
    if (lPtr.is_var())
      refineAddrAndOffset(lPtr.get_var());

    if (_prec_level < MEM)
      return;

    // Perform memory write in the value domain
    if (ar::isInteger(ar::getType(ar::getValue(stmt))) ||
        ar::isPointer(ar::getType(ar::getValue(stmt)))) {
      Literal lVal = _lfac[ar::getValue(stmt)];
      if (!lVal.is_var() && !lVal.is_num())
        return;

      linExpr e = (lVal.is_var() ? linExpr(lVal.get_var())
                                 : linExpr(lVal.get_num< Number >()));
      ikos::z_number size(ar::getSize(ar::getType(ar::getValue(stmt))));
      assert(lPtr.is_var());
      value_domain_impl::mem_write(_inv,
                                   lPtr.get_var(),
                                   size,
                                   e,
                                   ar::isPointer(
                                       ar::getType(ar::getValue(stmt))));
    }
  }

  void exec(Load_ref stmt) {
    Literal lPtr = _lfac[ar::getPointer(stmt)];
    if (lPtr.is_undefined_cst() || lPtr.is_null_cst()) {
      _inv = AbsValueDomain::bottom();
      return;
    }

    if (_prec_level < PTR)
      return;

    Internal_Variable_ref lhs = ar::getResult(stmt);
    VariableName lhs_var = _vfac[ar::getName(lhs)];

    // Create lazily a memory object
    if (ar::isGlobalVar(ar::getPointer(stmt)) ||
        ar::isAllocaVar(ar::getPointer(stmt))) {
      make_mem_object(_inv, ar::getPointer(stmt), _lfac, MUSTNONNULL);

      // Take the address of a global variable x:
      // int x = 0; int * p = &x;
      value_domain_impl::make_non_null(_inv, lhs_var);
    }

    // Reduction between value and pointer analysis
    if (lPtr.is_var())
      refineAddrAndOffset(lPtr.get_var());

    // Reduction between value and pointer analysis
    if (ar::isPointer(lhs))
      refineAddrAndOffset(lhs_var);

    if (_prec_level < MEM)
      return;

    /// Perform memory read in the value domain
    assert(lPtr.is_var());
    if (ar::isInteger(ar::getType(lhs)) || ar::isPointer(ar::getType(lhs))) {
      ikos::z_number size(ar::getSize(ar::getType(lhs)));
      value_domain_impl::mem_read(_inv,
                                  lhs_var,
                                  lPtr.get_var(),
                                  size,
                                  ar::isPointer(ar::getType(lhs)));
    }
  }

  //! Abstract the memory contents but not the surface
  void exec(Abstract_Memory_ref stmt) {
    if (_prec_level < MEM)
      return;

    // This AR statement can only be generated to indicate that a
    // global variable does not have an initializer.
    abstract_memory(ar::getPointer(stmt), ar::getLen(stmt));
  }

  // Not implemented
  void exec(FP_Comparison_ref /*stmt*/) {} // no floating point reasoning
  void exec(FP_Op_ref /*stmt*/) {}         // no floating point reasoning

private:
  /////
  // Analysis of external (library) calls
  // Note: This code should go to a different file if it keeps
  // growing.
  //////

  /*
    #include <stdlib.h>
    void* malloc(size_t size)

    This function returns a pointer to a newly allocated block size
    bytes long, or a null pointer if the block could not be allocated.
   */
  void analyze_malloc(boost::optional< Internal_Variable_ref > lhs,
                      Operand_ref size) {
    if (_prec_level < PTR)
      return;

    if (!lhs)
      return;

    if (ar::isPointer(*lhs)) {
      if (ar::isInteger(ar::getPointeeType(ar::getType(*lhs)))) {
        Literal Size = _lfac[size];

        Operand_ref o;
        arbos::convert(*lhs, o);

        // Create a new memory object
        if (Size.is_var()) {
          make_mem_object(_inv,
                          o,
                          _lfac,
                          /* malloc can return NULL if no more dynamic
                           * memory is available */
                          MAYNULL,
                          Size.get_var());
        } else if (Size.is_num()) {
          make_mem_object(_inv,
                          o,
                          _lfac,
                          /* malloc can return NULL if no more dynamic
                           * memory is available */
                          MAYNULL,
                          Size.get_num< ikos::z_number >());
        }
      }
    }
  }

  /*
    #include <stdlib.h>
    void free(void* ptr)

    This function deallocates the memory allocated via a previous call to
    malloc().
   */
  void analyze_free(Operand_ref op) {
    Literal ptr = _lfac[op];
    assert(ptr.is_var());

    if (_prec_level < PTR)
      return;

    if (value_domain_impl::is_unknown_addr(_inv, ptr.get_var()))
      return;

    // set the size to 0
    std::vector< VariableName > points_to =
        value_domain_impl::get_addrs_set(_inv, ptr.get_var());

    for (typename std::vector< VariableName >::iterator it = points_to.begin();
         it != points_to.end();
         it++) {
      if (points_to.size() == 1) {
        _inv.assign(get_shadow_size(*it), 0);
      } else {
        _inv -= get_shadow_size(*it);
      }
    }

    if (_prec_level < MEM)
      return;

    // forget memory contents
    value_domain_impl::forget_mem_contents(_inv, ptr.get_var());
  }

  /*
    #include <fcntl.h>
    int read(int handle, void* buffer, int nbyte);

    The read() function attempts to read nbytes from the file
    associated with handle, and places the characters read into
    buffer. If the file is opened using O_TEXT, it removes carriage
    returns and detects the end of the file.

    The function returns the number of bytes read. On end-of-file, 0
    is returned, on error it returns -1, setting errno to indicate the
    type of error that occurred.
  */
  void analyze_read(boost::optional< Internal_Variable_ref > lhs,
                    Operand_ref /*handle*/,
                    Operand_ref buf,
                    Operand_ref len) {
    if (lhs) {
      Literal ret = _lfac[*lhs];
      assert(ret.is_var());
      _inv -= ret.get_var();
      value_domain_impl::make_initialized(_inv, ret.get_var());
    }

    if (_prec_level < MEM)
      return;

    abstract_memory(buf, len);
  }
};

} // end namespace analyzer

#endif // ANALYZER_NUM_SYM_EXEC_HPP
