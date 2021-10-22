/*******************************************************************************
 *
 * \file
 * \brief Assertion definitions
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017-2019 United States Government as represented by the
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

#include <ikos/core/support/compiler.hpp>

#undef ikos_assert
#undef ikos_assert_msg
#undef ikos_unreachable

// clang-format off

#ifdef NDEBUG

# define ikos_assert(expr) static_cast< void >(0)

# define ikos_assert_msg(expr, msg) static_cast< void >(0)

# if __has_builtin(__builtin_unreachable) || IKOS_GNUC_PREREQ(4, 5, 0)
#  define ikos_unreachable(msg) __builtin_unreachable()
# elif defined(_MSC_VER)
#  define ikos_unreachable(msg) __assume(false)
# else
#  include <cstdlib>
#  define ikos_unreachable(msg) abort()
# endif

#else

# include <cassert>

/// \macro ikos_assert
///
/// By default, expands to assert(expr)
///
/// If the macro NDEBUG is defined, expands to ((void)0).
# define ikos_assert(expr) assert(expr)

/// \macro ikos_assert_msg
///
/// By default, expands to assert((expr) && (msg))
///
/// If the macro NDEBUG is defined, expands to ((void)0).
# define ikos_assert_msg(expr, msg) assert((expr) && (msg))

/// \macro ikos_unreachable
///
/// Marks that the current location is not supposed to be reachable.
///
/// If the macro NDEBUG is defined, expands to an optimizer hint
/// that the current location is not supposed to be reachable. On compilers that
/// don't support such hints, expands to abort()
///
/// Otherwise, expands to assert(false && (msg))
///
/// Use this instead of assert(0). It conveys intent more clearly and allows
/// compilers to omit some unnecessary code.
# define ikos_unreachable(msg) assert(false && (msg))

#endif

// clang-format on
