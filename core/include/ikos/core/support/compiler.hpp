/*******************************************************************************
 *
 * \file
 * \brief Define several macros, based on the current compiler
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

#pragma once

// clang-format off

/// \macro __has_builtin
/// \brief Evaluates to 1 if the builtin is supported, otherwise 0.
#ifndef __has_builtin
# define __has_builtin(x) 0
#endif

/// \macro __has_attribute
/// \brief Evaluates to 1 if the attribute is supported, otherwise 0.
#ifndef __has_attribute
# define __has_attribute(x) 0
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

/// \macro ikos_likely
/// \brief Tell the compiler that the given condition is likely to yield true
/// \macro ikos_unlikely
/// \brief Tell the compiler that the given condition is likely to yield false
#if __has_builtin(__builtin_expect) || IKOS_GNUC_PREREQ(4, 0, 0)
# define ikos_likely(EXPR) __builtin_expect(static_cast< bool >(EXPR), true)
# define ikos_unlikely(EXPR) __builtin_expect(static_cast< bool >(EXPR), false)
#else
# define ikos_likely(EXPR) (EXPR)
# define ikos_unlikely(EXPR) (EXPR)
#endif

/// \macro ikos_attribute_noreturn
/// \brief Tell the compiler that the function does not return.
#ifdef __GNUC__
# define ikos_attribute_noreturn __attribute__((noreturn))
#elif defined(_MSC_VER)
# define ikos_attribute_noreturn __declspec(noreturn)
#else
# define ikos_attribute_noreturn
#endif

/// \macro ikos_attribute_unused
/// \brief Remove unused function warnings for the given function
#if __has_attribute(unused) || IKOS_GNUC_PREREQ(3, 1, 0)
# define ikos_attribute_unused __attribute__((__unused__))
#else
# define ikos_attribute_unused
#endif

/// \macro ikos_ignore
/// \brief Remove unused variable warnings for the given variable
#define ikos_ignore(VAR) static_cast< void >(VAR)

// clang-format on
