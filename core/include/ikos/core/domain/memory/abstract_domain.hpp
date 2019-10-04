/*******************************************************************************
 *
 * \file
 * \brief Generic API for memory abstract domains
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2019 United States Government as represented by the
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

#pragma once

#include <boost/optional.hpp>

#include <ikos/core/domain/scalar/abstract_domain.hpp>
#include <ikos/core/literal.hpp>
#include <ikos/core/value/lifetime.hpp>

namespace ikos {
namespace core {
namespace memory {

/// \brief Base class for memory abstract domains
///
/// A memory abstract domain is a scalar abstract domain with memory operations
template < typename VariableRef, typename MemoryLocationRef, typename Derived >
class AbstractDomain
    : public scalar::AbstractDomain< VariableRef, MemoryLocationRef, Derived > {
public:
  using IntInterval = machine_int::Interval;
  using LiteralT = Literal< VariableRef, MemoryLocationRef >;

public:
  /// \name Memory abstract domain methods
  /// @{

  /// \brief Perform the memory write `*p = v`
  ///
  /// \param p The pointer variable
  /// \param v The stored value
  /// \param size The stored size, in bytes (for instance, 4 for a int)
  virtual void mem_write(VariableRef p,
                         const LiteralT& v,
                         const MachineInt& size) = 0;

  /// \brief Perform the memory read `x = *p`
  ///
  /// \param x The result variable
  /// \param p The pointer variable
  /// \param size The read size, in bytes (for instance, 4 for a int)
  virtual void mem_read(const LiteralT& x,
                        VariableRef p,
                        const MachineInt& size) = 0;

  /// \brief Perform the memory copy `memcpy(dest, src, size)`
  ///
  /// \param dest The destination pointer variable
  /// \param src The source pointer variable
  /// \param size The number of bytes copied, as a literal
  ///
  /// Notes:
  ///   If `dst` and `src` overlap, as the behavior in C is undefined, the
  ///   memory contents is set to top.
  virtual void mem_copy(VariableRef dest,
                        VariableRef src,
                        const LiteralT& size) = 0;

  /// \brief Perform the memory set `memset(dest, value, size)`
  ///
  /// \param dest The destination pointer variable
  /// \param value The byte value, as a literal
  /// \param size The number of written bytes, as a literal
  virtual void mem_set(VariableRef dest,
                       const LiteralT& value,
                       const LiteralT& size) = 0;

  /// \brief Forget all memory contents
  virtual void mem_forget_all() = 0;

  /// \brief Forget the memory contents at the given memory location
  virtual void mem_forget(MemoryLocationRef addr) = 0;

  /// \brief Forget the memory contents in range
  /// `[addr + offset, addr + offset + size - 1]`
  ///
  /// \param addr The memory location
  /// \param offset The offset as a machine integer interval
  /// \param size The size in bytes
  virtual void mem_forget(MemoryLocationRef addr,
                          const IntInterval& offset,
                          const MachineInt& size) = 0;

  /// \brief Forget the memory contents in range
  /// `[addr + range.lb(), addr + range.ub()]`
  ///
  /// \param addr The memory location
  /// \param range The byte range as a machine integer interval
  virtual void mem_forget(MemoryLocationRef addr, const IntInterval& range) = 0;

  /// \brief Forget the memory contents accessible through pointer `p`
  virtual void mem_forget_reachable(VariableRef p) = 0;

  /// \brief Forget the memory contents in range `[p, p + size - 1]`
  ///
  /// Forget all memory contents that can be accessible through pointer `p`
  /// and that overlap with `[p.offset, ..., p.offset + size - 1]`
  //
  /// \param p The pointer variable
  /// \param size The size in bytes
  virtual void mem_forget_reachable(VariableRef p, const MachineInt& size) = 0;

  /// \brief Abstract the memory contents reachable through pointer `p`
  ///
  /// Abstract all memory contents that can be accessible through pointer `p`.
  /// Suppose it contains random bytes, and no valid pointers (unlike
  /// forget_reachable_mem).
  virtual void mem_abstract_reachable(VariableRef p) = 0;

  /// \brief Abstract the memory contents in range `[p, p + size - 1]`
  ///
  /// Abstract all memory contents that can be accessible through pointer `p`
  /// and that overlap with `[p.offset, ..., p.offset + size - 1]`. Suppose it
  /// contains random bytes, and no valid pointers (unlike
  /// forget_reachable_mem).
  ///
  /// \param p The pointer variable
  /// \param size The size in bytes
  virtual void mem_abstract_reachable(VariableRef p,
                                      const MachineInt& size) = 0;

  /// \brief Set the memory contents accessible through pointer `p` to zero
  virtual void mem_zero_reachable(VariableRef p) = 0;

  /// \brief Set the memory contents accessible through pointer `p` to
  /// uninitialized
  virtual void mem_uninitialize_reachable(VariableRef p) = 0;

  /// @}
  /// \name Lifetime abstract domain methods
  /// @{

  /// \brief Assign `m = allocated`
  virtual void lifetime_assign_allocated(MemoryLocationRef m) = 0;

  /// \brief Assign `m = deallocated`
  virtual void lifetime_assign_deallocated(MemoryLocationRef m) = 0;

  /// \brief Add the constraint `m == allocated`
  virtual void lifetime_assert_allocated(MemoryLocationRef m) = 0;

  /// \brief Add the constraint `m == deallocated`
  virtual void lifetime_assert_deallocated(MemoryLocationRef m) = 0;

  /// \brief Forget the lifetime of a memory location
  virtual void lifetime_forget(MemoryLocationRef m) = 0;

  /// \brief Set the lifetime of a memory location
  virtual void lifetime_set(MemoryLocationRef m, Lifetime value) = 0;

  /// \brief Get the lifetime value for the given memory location
  virtual Lifetime lifetime_to_lifetime(MemoryLocationRef m) const = 0;

  /// @}
  /// \name Partitioning abstract domain methods
  /// @{

  /// \brief Partition the abstract value according to the given variable
  virtual void partitioning_set_variable(VariableRef x) = 0;

  /// \brief Return the current partitioning variable, or boost::none
  virtual boost::optional< VariableRef > partitioning_variable() const = 0;

  /// \brief Join the current partitions
  virtual void partitioning_join() = 0;

  /// \brief Disable the current partitioning
  virtual void partitioning_disable() = 0;

  /// @}

}; // end class AbstractDomain

/// \brief Check if a type is a memory abstract domain
template < typename T, typename VariableRef, typename MemoryLocationRef >
struct IsAbstractDomain
    : std::is_base_of<
          memory::AbstractDomain< VariableRef, MemoryLocationRef, T >,
          T > {};

} // end namespace memory
} // end namespace core
} // end namespace ikos
