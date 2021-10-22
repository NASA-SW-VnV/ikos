/*******************************************************************************
 *
 * \file
 * \brief Casting definitions (isa, cast, dyn_cast)
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

#include <type_traits>

#include <ikos/core/support/assert.hpp>

namespace ikos {
namespace core {

/// \brief Core implementation of isa< T >
template < typename To, typename From, typename Enable = void >
struct IsaImpl {
  static bool isa(const From& val) { return To::classof(&val); }
};

/// \brief Always allow upcasts, and perform no dynamic check for them
template < typename To, typename From >
struct IsaImpl<
    To,
    From,
    typename std::enable_if_t< std::is_base_of< To, From >::value > > {
  static bool isa(const From&) { return true; }
};

template < typename To, typename From >
struct IsaImplWrap {
  static bool isa(const From& val) { return IsaImpl< To, From >::isa(val); }
};

template < typename To, typename From >
struct IsaImplWrap< To, const From > {
  static bool isa(const From& val) { return IsaImpl< To, From >::isa(val); }
};

template < typename To, typename From >
struct IsaImplWrap< To, From* > {
  static bool isa(const From* val) {
    ikos_assert_msg(val, "isa<> on a null pointer");
    return IsaImpl< To, From >::isa(*val);
  }
};

template < typename To, typename From >
struct IsaImplWrap< To, From* const > {
  static bool isa(const From* val) {
    ikos_assert_msg(val, "isa<> on a null pointer");
    return IsaImpl< To, From >::isa(*val);
  }
};

template < typename To, typename From >
struct IsaImplWrap< To, const From* > {
  static bool isa(const From* val) {
    ikos_assert_msg(val, "isa<> on a null pointer");
    return IsaImpl< To, From >::isa(*val);
  }
};

template < typename To, typename From >
struct IsaImplWrap< To, const From* const > {
  static bool isa(const From* val) {
    ikos_assert_msg(val, "isa<> on a null pointer");
    return IsaImpl< To, From >::isa(*val);
  }
};

/// \brief Check if the parameter is an instance of the template type argument
///
/// Used like this:
///
/// \code{.cpp}
/// if (isa< GlobalVariable >(v)) { ... }
/// \endcode
template < typename X, typename Y >
inline bool isa(const Y& val) {
  return IsaImplWrap< X, Y >::isa(val);
}

// Calculate what type the 'cast' function should return, based on a requested
// type of To and a source type of From.
template < typename To, typename From >
struct CastRet {
  using Type = To&;
};

template < typename To, typename From >
struct CastRet< To, const From > {
  using Type = const To&;
};

template < typename To, typename From >
struct CastRet< To, From* > {
  using Type = To*;
};

template < typename To, typename From >
struct CastRet< To, const From* > {
  using Type = const To*;
};

template < typename To, typename From >
struct CastRet< To, const From* const > {
  using Type = const To*;
};

/// \brief Return the argument parameter cast to the specified type
///
/// This casting operator asserts that the type is correct. It does not allow
/// a null argument.
///
/// Used like this:
///
/// \code{.cpp}
/// Instruction* inst = cast< Instruction >(val)
/// \endcode
template < typename X, typename Y >
inline typename CastRet< X, Y >::Type cast(Y& val) {
  ikos_assert_msg(isa< X >(val), "cast<>() argument of incompatible type");
  return static_cast< typename CastRet< X, Y >::Type >(val);
}

/// \brief Return the argument parameter cast to the specified type
///
/// This casting operator asserts that the type is correct. It does not allow
/// a null argument.
///
/// Used like this:
///
/// \code{.cpp}
/// Instruction* inst = cast< Instruction >(val)
/// \endcode
template < typename X, typename Y >
inline typename CastRet< X, Y* >::Type cast(Y* val) {
  ikos_assert_msg(isa< X >(val), "cast<>() argument of incompatible type");
  return static_cast< typename CastRet< X, Y* >::Type >(val);
}

/// \brief Return the argument parameter cast to the specified type
///
/// This is equivalent to cast< X >, except that a null value is allowed.
template < typename X, typename Y >
inline typename CastRet< X, Y >::Type cast_or_null(Y& val) {
  if (val == nullptr) {
    return nullptr;
  }
  ikos_assert_msg(isa< X >(val),
                  "cast_or_null<>() argument of incompatible type");
  return static_cast< typename CastRet< X, Y >::Type >(val);
}

/// \brief Return the argument parameter cast to the specified type
///
/// This is equivalent to cast< X >, except that a null value is allowed.
template < typename X, typename Y >
inline typename CastRet< X, Y* >::Type cast_or_null(Y* val) {
  if (val == nullptr) {
    return nullptr;
  }
  ikos_assert_msg(isa< X >(val),
                  "cast_or_null<>() argument of incompatible type");
  return static_cast< typename CastRet< X, Y* >::Type >(val);
}

/// \brief Return the argument parameter cast to the specified type
///
/// This casting operator returns null if the argument is of the wrong type, so
/// it can used to test for a type as well as cast if successful.
///
/// Used like this:
///
/// \code{.cpp}
/// if (Instruction* inst = dyn_cast< Instruction >(val)) { ... }
/// \endcode
template < typename X, typename Y >
inline typename CastRet< X, Y* >::Type dyn_cast(Y* val) {
  return isa< X >(val) ? cast< X >(val) : nullptr;
}

/// \brief Return the argument parameter cast to the specified type
///
/// This is equivalent to dyn_cast< X >, except that a null value is allowed.
template < typename X, typename Y >
inline typename CastRet< X, Y* >::Type dyn_cast_or_null(Y* val) {
  return (val != nullptr && isa< X >(val)) ? cast< X >(val) : nullptr;
}

} // end namespace core
} // end namespace ikos
