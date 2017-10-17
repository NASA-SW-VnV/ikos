/**************************************************************************/ /**
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

/// \macro __has_builtin
/// \brief Evaluates to 1 if the builtin is supported, otherwise 0.
#ifndef __has_builtin
# define __has_builtin(x) 0
#endif

/// \macro IKOS_GNUC_PREREQ
/// \brief Evaluates to 1 if the compiler is GCC >= maj.min.patch, otherwise 0.
#ifndef IKOS_GNUC_PREREQ
# if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
#  define IKOS_GNUC_PREREQ(maj, min, patch) \
    ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) + __GNUC_PATCHLEVEL__ >= \
     ((maj) << 20) + ((min) << 10) + (patch))
# elif defined(__GNUC__) && defined(__GNUC_MINOR__)
#  define IKOS_GNUC_PREREQ(maj, min, patch) \
    ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) >= ((maj) << 20) + ((min) << 10))
# else
#  define IKOS_GNUC_PREREQ(maj, min, patch) 0
# endif
#endif

#undef ikos_assert
#undef ikos_assert_msg
#undef ikos_unreachable

#if defined(IKOS_DISABLE_ASSERTS) || defined(NDEBUG)

# define ikos_assert(expr) ((void)0)

# define ikos_assert_msg(expr, msg) ((void)0)

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
/// If the macro IKOS_DISABLE_ASSERTS is defined, expands to ((void)0),
/// regardless of whether the macro NDEBUG is defined.
# define ikos_assert(expr) assert(expr)

/// \macro ikos_assert_msg
///
/// By default, expands to assert((expr) && (msg))
///
/// If the macro IKOS_DISABLE_ASSERTS is defined, expands to ((void)0),
/// regardless of whether the macro NDEBUG is defined.
# define ikos_assert_msg(expr, msg) assert((expr) && (msg))

/// \macro ikos_unreachable
///
/// Marks that the current location is not supposed to be reachable.
///
/// If IKOS_DISABLE_ASSERTS or NDEBUG are defined, expands to an optimizer hint
/// that the current location is not supposed to be reachable. On compilers that
/// don't support such hints, expands to abort()
///
/// Otherwise, expands to assert(false && (msg))
///
/// Use this instead of assert(0). It conveys intent more clearly and allows
/// compilers to omit some unnecessary code.
# define ikos_unreachable(msg) assert(false && (msg))

#endif
