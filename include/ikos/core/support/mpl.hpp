/*******************************************************************************
 *
 * \file
 * \brief Template metaprogramming utilities
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

#include <iterator>
#include <type_traits>

namespace ikos {
namespace core {

/// Helpers for void_t
namespace detail {

template < typename... Ts >
struct make_void {
  using type = void;
};

} // end namespace detail

/// \brief Utility metafunction that maps a sequence of any types to void
///
/// This is a backport of C++17 std::void_t, see
/// http://en.cppreference.com/w/cpp/types/void_t
template < typename... Ts >
using void_t = typename detail::make_void< Ts... >::type;

/// \brief Remove const, volatile and ref qualifiers
///
/// This is a backport of C++20 std::remove_cvref, see
/// http://en.cppreference.com/w/cpp/types/remove_cvref
template < typename T >
struct remove_cvref {
  using type = std::remove_cv_t< std::remove_reference_t< T > >;
};

/// \brief Helper types
template < typename T >
using remove_cvref_t = typename remove_cvref< T >::type;

/// \brief Form the logical conjunction of the parameters
///
/// Performs a logical AND on the given sequence of type traits
///
/// This is a backport of C++17 std::conjunction, see
/// http://en.cppreference.com/w/cpp/types/conjunction
template < typename... >
struct conjunction : std::true_type {};

template < typename B1 >
struct conjunction< B1 > : B1 {};

template < typename B1, typename... Bn >
struct conjunction< B1, Bn... >
    : std::conditional_t< bool(B1::value), conjunction< Bn... >, B1 > {};

/// Helpers for is_detected
namespace detail {

struct nonesuch {
  nonesuch() = delete;
  ~nonesuch() = delete;
  nonesuch(const nonesuch&) = delete;
  nonesuch(nonesuch&&) = delete;
  void operator=(const nonesuch&) = delete;
  void operator=(nonesuch&&) = delete;
};

template < typename Default,
           typename AlwaysVoid,
           template < typename... >
           class Op,
           typename... Args >
struct detector {
  using value_t = std::false_type;
  using type = Default;
};

template < typename Default,
           template < typename... >
           class Op,
           typename... Args >
struct detector< Default, void_t< Op< Args... > >, Op, Args... > {
  using value_t = std::true_type;
  using type = Op< Args... >;
};

} // end namespace detail

/// \brief Detection idiom
///
/// `detected_or` is an alias for an unspecified class type with two public
/// member typedefs `value_t` and `type`, which are defined as follows:
///   * If the template-id `Op<Args...>` is valid, then `value_t` is an alias
///   for `std::true_type`, and type is an alias for `Op<Args...>`;
///   * Otherwise, `value_t` is an alias for `std::false_type` and type is an
///   alias for `Default`.
///
/// This is a backport from the library fundamentals v2, see
/// http://en.cppreference.com/w/cpp/experimental/is_detected
template < typename Default,
           template < typename... >
           class Op,
           typename... Args >
using detected_or = detail::detector< Default, void, Op, Args... >;

/// \brief Detection idiom
///
/// `is_detected` is equivalent to `typename
/// detected_or<std::experimental::nonesuch, Op, Args...>::value_t`. It is an
/// alias for `std::true_type` if the template-id `Op<Args...>` is valid;
/// otherwise it is an alias for `std::false_type`.
///
/// This is a backport from the library fundamentals v2, see
/// http://en.cppreference.com/w/cpp/experimental/is_detected
template < template < typename... > class Op, typename... Args >
using is_detected =
    typename detail::detector< detail::nonesuch, void, Op, Args... >::value_t;

/// Helpers for supports_equality
namespace detail {

template < typename T, typename U >
using equality_t = decltype(std::declval< T >() == std::declval< U >());

} // end namespace detail

/// \brief Checks if the given types can be compared with ==
template < typename T, typename U >
using supports_equality = is_detected< detail::equality_t, T, U >;

/// Helpers for supports_less_than
namespace detail {

template < typename T, typename U >
using less_than_t = decltype(std::declval< T >() < std::declval< U >());

} // end namespace detail

/// \brief Checks if the given types can be compared with <
template < typename T, typename U >
using supports_less_than = is_detected< detail::less_than_t, T, U >;

} // end namespace core
} // end namespace ikos
