/*******************************************************************************
 *
 * \file
 * \brief ikos intrinsics definitions
 *
 * Include this header in an analyzed code to define ikos intrinsics, such as
 * __ikos_assert(), __ikos_assume(), etc.
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2018-2019 United States Government as represented by the
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

#if defined(__IKOS__)

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
#define IKOS_NOEXCEPT noexcept
#else
#define IKOS_NOEXCEPT
#endif

/// \brief Test the given condition
///
/// The assertion prover (ikos -a prover) will try to prove that the condition
/// always holds.
extern void __ikos_assert(int condition) IKOS_NOEXCEPT;

// clang-format off

/// \macro __ikos_assume
///
/// Assume that the given condition always holds.
#define __ikos_assume(condition) if (!(condition)) { __builtin_unreachable(); }

// clang-format on

/// \brief Return a non-deterministic signed integer
extern int __ikos_nondet_int(void) IKOS_NOEXCEPT;

/// \brief Return a non-deterministic unsigned integer
extern unsigned __ikos_nondet_uint(void) IKOS_NOEXCEPT;

/// \brief Check if a memory access is valid
///
/// The buffer overflow analysis (ikos -a boa) will check if the memory at
/// `[ptr, ptr + size - 1]` is accessible.
///
/// The null pointer dereference analysis (ikos -a nullity) will check if the
/// pointer is null.
extern void __ikos_check_mem_access(const void* ptr, size_t size) IKOS_NOEXCEPT;

/// \brief Check if a null-terminated string access is valid
///
/// The buffer overflow analysis (ikos -a boa) will check if the given string
/// access leads to a buffer overflow.
///
/// The null pointer dereference analysis (ikos -a nullity) will check if the
/// pointer is null.
extern void __ikos_check_string_access(const char* str) IKOS_NOEXCEPT;

/// \brief Assume that the memory pointed by `ptr` has the given size, in bytes
extern void __ikos_assume_mem_size(const void* ptr, size_t size) IKOS_NOEXCEPT;

/// \brief Forget the memory contents at `[ptr, ptr + size - 1]`
///
/// Assume the memory now contains random bytes.
///
/// This is undefined behavior if the pointer is null.
extern void __ikos_forget_mem(void* ptr, size_t size) IKOS_NOEXCEPT;

/// \brief Abstract the memory contents at `[ptr, ptr + size - 1]`
///
/// Assume the memory now contains random bytes, but no valid pointers.
///
/// This is undefined behavior if the pointer is null.
extern void __ikos_abstract_mem(void* ptr, size_t size) IKOS_NOEXCEPT;

/// \brief Watch the memory writes at `[ptr, ptr + size - 1]`
///
/// The memory watcher (ikos -a watch) will print the location of statements
/// that write at the given memory location.
extern void __ikos_watch_mem(const void* ptr, size_t size) IKOS_NOEXCEPT;

/// \brief Print the invariant at the function call
///
/// The debugger (ikos -a dbg) will print the current invariant.
extern void __ikos_print_invariant(void) IKOS_NOEXCEPT;

/// \brief Partition the invariant according to the given integer variable
extern void __ikos_partitioning_var_int(int) IKOS_NOEXCEPT;

/// \brief Join the current partitions
extern void __ikos_partitioning_join(void) IKOS_NOEXCEPT;

/// \brief Disable the current partitioning
extern void __ikos_partitioning_disable(void) IKOS_NOEXCEPT;

/// \brief Print the information on the given values at the function call
///
/// The debugger (ikos -a dbg) will print the information on the
/// given values at the function call location.
///
/// \param desc Description, for debugging purpose
///
/// \code{.c}
/// __ikos_print_values("x", x);
/// \endcode
extern void __ikos_print_values(const char* desc, ...) IKOS_NOEXCEPT;

#undef IKOS_NOEXCEPT

#ifdef __cplusplus
}
#endif

#else // __IKOS__

#define __ikos_assert(condition)

#define __ikos_assume(condition)

#define __ikos_nondet_int() ((int)0)

#define __ikos_nondet_uint() ((unsigned)0)

#define __ikos_check_mem_access(ptr, size)

#define __ikos_check_string_access(str)

#define __ikos_assume_mem_size(ptr, size)

#define __ikos_forget_mem(ptr, size)

#define __ikos_abstract_mem(ptr, size)

#define __ikos_watch_mem(ptr, size)

#define __ikos_partitioning_var_int(x)

#define __ikos_partitioning_join()

#define __ikos_partitioning_disable()

#define __ikos_print_invariant()

#define __ikos_print_values(...)

#endif
