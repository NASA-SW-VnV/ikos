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

#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/domain/lifetime/abstract_domain.hpp>
#include <ikos/core/domain/machine_int/abstract_domain.hpp>
#include <ikos/core/domain/nullity/abstract_domain.hpp>
#include <ikos/core/domain/pointer/abstract_domain.hpp>
#include <ikos/core/domain/uninitialized/abstract_domain.hpp>
#include <ikos/core/literal.hpp>
#include <ikos/core/semantic/memory/variable.hpp>
#include <ikos/core/semantic/memory_location.hpp>
#include <ikos/core/semantic/variable.hpp>

namespace ikos {
namespace core {
namespace memory {

/// \brief Base class for memory abstract domains
///
/// A memory abstract domain should hold:
///   * A machine integer abstract domain (embedded in the pointer domain)
///   * A nullity abstract domain (embedded in the pointer domain)
///   * A pointer abstract domain
///   * An uninitialized abstract domain
///   * A lifetime abstract domain
template < typename VariableRef,
           typename MemoryLocationRef,
           typename VariableFactory,
           typename MachineIntDomain,
           typename NullityDomain,
           typename PointerDomain,
           typename UninitializedDomain,
           typename LifetimeDomain,
           typename Derived >
class AbstractDomain : public core::AbstractDomain< Derived > {
public:
  static_assert(
      core::IsVariable< VariableRef >::value,
      "VariableRef does not meet the requirements for variable types");
  static_assert(memory::IsVariable< VariableRef >::value,
                "VariableRef must implement memory::VariableTraits");
  static_assert(core::IsMemoryLocation< MemoryLocationRef >::value,
                "MemoryLocationRef does not meet the requirements for memory "
                "location types");
  static_assert(
      machine_int::IsAbstractDomain< MachineIntDomain, VariableRef >::value,
      "MachineIntDomain must implement machine_int::AbstractDomain");
  static_assert(nullity::IsAbstractDomain< NullityDomain, VariableRef >::value,
                "NullityDomain must implement nullity::AbstractDomain");
  static_assert(pointer::IsAbstractDomain< PointerDomain,
                                           VariableRef,
                                           MemoryLocationRef,
                                           MachineIntDomain,
                                           NullityDomain >::value,
                "PointerDomain must implement pointer::AbstractDomain");
  static_assert(
      uninitialized::IsAbstractDomain< UninitializedDomain,
                                       VariableRef >::value,
      "UninitializedDomain must implement uninitialized::AbstractDomain");
  static_assert(
      lifetime::IsAbstractDomain< LifetimeDomain, MemoryLocationRef >::value,
      "LifetimeDomain must implement lifetime::AbstractDomain");

public:
  using LiteralT = Literal< VariableRef, MemoryLocationRef >;

public:
  /// \brief Provide access to the underlying machine integer abstract domain
  virtual MachineIntDomain& integers() = 0;

  /// \brief Provide access to the underlying machine integer abstract domain
  virtual const MachineIntDomain& integers() const = 0;

  /// \brief Provide access to the underlying nullity abstract domain
  virtual NullityDomain& nullity() = 0;

  /// \brief Provide access to the underlying nullity abstract domain
  virtual const NullityDomain& nullity() const = 0;

  /// \brief Provide access to the underlying pointer abstract domain
  virtual PointerDomain& pointers() = 0;

  /// \brief Provide access to the underlying pointer abstract domain
  virtual const PointerDomain& pointers() const = 0;

  /// \brief Provide access to the underlying uninitialized abstract domain
  virtual UninitializedDomain& uninitialized() = 0;

  /// \brief Provide access to the underlying uninitialized abstract domain
  virtual const UninitializedDomain& uninitialized() const = 0;

  /// \brief Provide access to the underlying lifetime abstract domain
  virtual LifetimeDomain& lifetime() = 0;

  /// \brief Provide access to the underlying lifetime abstract domain
  virtual const LifetimeDomain& lifetime() const = 0;

  /// \brief Perform the widening of two abstract values with a threshold
  virtual void widen_threshold_with(const Derived& other,
                                    const MachineInt& threshold) = 0;

  /// \brief Perform the widening of two abstract values with a threshold
  virtual Derived widening_threshold(const Derived& other,
                                     const MachineInt& threshold) const {
    Derived tmp(static_cast< const Derived& >(*this));
    tmp.widen_threshold_with(other, threshold);
    return tmp;
  }

  /// \brief Perform the memory write `*p = v`
  ///
  /// \param vfac The variable factory
  /// \param p The pointer variable
  /// \param v The stored value
  /// \param size The stored size, in bytes (for instance, 4 for a int)
  virtual void mem_write(VariableFactory& vfac,
                         VariableRef p,
                         const LiteralT& v,
                         const MachineInt& size) = 0;

  /// \brief Perform the memory read `x = *p`
  ///
  /// \param vfac The variable factory
  /// \param x The result variable
  /// \param p The pointer variable
  /// \param size The read size, in bytes (for instance, 4 for a int)
  virtual void mem_read(VariableFactory& vfac,
                        const LiteralT& x,
                        VariableRef p,
                        const MachineInt& size) = 0;

  /// \brief Perform the memory copy `memcpy(dest, src, size)`
  ///
  /// \param vfac The variable factory
  /// \param dest The destination pointer variable
  /// \param src The source pointer variable
  /// \param size The number of bytes copied, as a literal
  ///
  /// Notes:
  ///   If dst and src overlap, as the behavior in C is undefined, the memory
  ///   contents is set to top.
  virtual void mem_copy(VariableFactory& vfac,
                        VariableRef dest,
                        VariableRef src,
                        const LiteralT& size) = 0;

  /// \brief Perform the memory set `memset(dest, value, size)`
  ///
  /// \param vfac The variable factory
  /// \param dest The destination pointer variable
  /// \param value The byte value, as a literal
  /// \param size The number of written bytes, as a literal
  virtual void mem_set(VariableFactory& vfac,
                       VariableRef dest,
                       const LiteralT& value,
                       const LiteralT& size) = 0;

  /// \brief Forget the memory surface of variable `x`
  ///
  /// Forget only the "surface" part of the underlying domain.
  /// It does not update the memory.
  ///
  /// If `x` is a scalar variable, forget its value.
  /// If `x` is a pointer variable, forget its base address and offset.
  virtual void forget_surface(VariableRef x) = 0;

  /// \brief Forget all memory contents
  virtual void forget_mem() = 0;

  /// \brief Forget the memory contents at the given memory location
  virtual void forget_mem(MemoryLocationRef addr) = 0;

  /// \brief Forget the memory contents in range
  /// `[addr + offset, addr + offset + size - 1]`
  ///
  /// \param addr The memory location
  /// \param offset The machine integer variable
  /// \param size The size in bytes
  virtual void forget_mem(MemoryLocationRef addr,
                          VariableRef offset,
                          const MachineInt& size) = 0;

  /// \brief Forget the memory contents in range
  /// `[addr + range.lb(), addr + range.ub()]`
  ///
  /// \param addr The memory location
  /// \param range The byte range as a machine integer interval
  virtual void forget_mem(MemoryLocationRef addr,
                          const machine_int::Interval& range) = 0;

  /// \brief Forget the memory contents accessible through pointer `p`
  virtual void forget_reachable_mem(VariableRef p) = 0;

  /// \brief Forget the memory contents in range `[p, p + size - 1]`
  ///
  /// Forget all memory contents that can be accessible through pointer `p`
  /// and that overlap with `[p.offset, ..., p.offset + size - 1]`
  //
  /// \param p The pointer variable
  /// \param size The size in bytes
  virtual void forget_reachable_mem(VariableRef p, const MachineInt& size) = 0;

  /// \brief Abstract the memory contents reachable through pointer `p`
  ///
  /// Abstract all memory contents that can be accessible through pointer `p`.
  /// Suppose it contains random bytes, and no valid pointers (unlike
  /// forget_reachable_mem).
  virtual void abstract_reachable_mem(VariableRef p) = 0;

  /// \brief Abstract the memory contents in range `[p, p + size - 1]`
  ///
  /// Abstract all memory contents that can be accessible through pointer `p`
  /// and that overlap with `[p.offset, ..., p.offset + size - 1]`. Suppose it
  /// contains random bytes, and no valid pointers (unlike
  /// forget_reachable_mem).
  ///
  /// \param p The pointer variable
  /// \param size The size in bytes
  virtual void abstract_reachable_mem(VariableRef p,
                                      const MachineInt& size) = 0;

  /// \brief Set the memory contents accessible through pointer `p` to zero
  virtual void zero_reachable_mem(VariableRef p) = 0;

  /// \brief Set the memory contents accessible through pointer `p` to
  /// uninitialized
  virtual void uninitialize_reachable_mem(VariableRef p) = 0;

  /// \brief Normalize the abstract value
  virtual void normalize() const = 0;

}; // end class AbstractDomain

} // end namespace memory
} // end namespace core
} // end namespace ikos
