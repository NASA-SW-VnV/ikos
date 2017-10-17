/**************************************************************************/ /**
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

#ifndef ANALYZER_CAST_HPP
#define ANALYZER_CAST_HPP

#include <type_traits>

#include <ikos/assert.hpp>

namespace analyzer {

/// \brief Core implementation of isa< T >
template < typename To, typename From, typename Enable = void >
struct isa_impl {
  static bool isa(const From& val) { return To::classof(&val); }
};

/// \brief Always allow upcasts, and perform no dynamic check for them
template < typename To, typename From >
struct isa_impl<
    To,
    From,
    typename std::enable_if_t< std::is_base_of< To, From >::value > > {
  static bool isa(const From&) { return true; }
};

template < typename To, typename From >
struct isa_impl_wrap {
  static bool isa(const From& val) { return isa_impl< To, From >::isa(val); }
};

template < typename To, typename From >
struct isa_impl_wrap< To, const From > {
  static bool isa(const From& val) { return isa_impl< To, From >::isa(val); }
};

template < typename To, typename From >
struct isa_impl_wrap< To, From* > {
  static bool isa(const From* val) {
    ikos_assert_msg(val, "isa<> on a null pointer");
    return isa_impl< To, From >::isa(*val);
  }
};

template < typename To, typename From >
struct isa_impl_wrap< To, From* const > {
  static bool isa(const From* val) {
    ikos_assert_msg(val, "isa<> on a null pointer");
    return isa_impl< To, From >::isa(*val);
  }
};

template < typename To, typename From >
struct isa_impl_wrap< To, const From* > {
  static bool isa(const From* val) {
    ikos_assert_msg(val, "isa<> on a null pointer");
    return isa_impl< To, From >::isa(*val);
  }
};

template < typename To, typename From >
struct isa_impl_wrap< To, const From* const > {
  static bool isa(const From* val) {
    ikos_assert_msg(val, "isa<> on a null pointer");
    return isa_impl< To, From >::isa(*val);
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
  return isa_impl_wrap< X, Y >::isa(val);
}

// Calculate what type the 'cast' function should return, based on a requested
// type of To and a source type of From.
template < typename To, typename From >
struct cast_ret {
  typedef To& type;
};

template < typename To, typename From >
struct cast_ret< To, const From > {
  typedef const To& type;
};

template < typename To, typename From >
struct cast_ret< To, From* > {
  typedef To* type;
};

template < typename To, typename From >
struct cast_ret< To, const From* > {
  typedef const To* type;
};

template < typename To, typename From >
struct cast_ret< To, const From* const > {
  typedef const To* type;
};

/// \brief Return the argument parameter cast to the specified type
///
/// This casting operator asserts that the type is correct. It does not allow
/// a null argument.
///
/// Used like this:
///
/// \code{.cpp}
/// cast< instruction >(val)
/// \endcode
template < typename X, typename Y >
inline typename cast_ret< X, Y >::type cast(Y& val) {
  ikos_assert_msg(isa< X >(val), "cast<>() argument of incompatible type");
  return static_cast< typename cast_ret< X, Y >::type >(val);
}

/// \brief Return the argument parameter cast to the specified type
///
/// This casting operator asserts that the type is correct. It does not allow
/// a null argument.
///
/// Used like this:
///
/// \code{.cpp}
/// cast< instruction >(val)
/// \endcode
template < typename X, typename Y >
inline typename cast_ret< X, Y* >::type cast(Y* val) {
  ikos_assert_msg(isa< X >(val), "cast<>() argument of incompatible type");
  return static_cast< typename cast_ret< X, Y* >::type >(val);
}

/// \brief Return the argument parameter cast to the specified type
///
/// This casting operator returns null if the argument is of the wrong type, so
/// it can used to test for a type as well as cast if successful.
///
/// Used like this:
///
/// \code{.cpp}
/// if (instruction* inst = dyn_cast< instruction >(val)) { ... }
/// \endcode
template < typename X, typename Y >
inline typename cast_ret< X, Y* >::type dyn_cast(Y* val) {
  return isa< X >(val) ? cast< X >(val) : nullptr;
}

} // end namespace analyzer

#endif // ANALYZER_CAST_HPP
