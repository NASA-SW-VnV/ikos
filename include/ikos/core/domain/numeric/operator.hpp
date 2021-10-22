/*******************************************************************************
 *
 * \file
 * \brief Operators for numerical abstract domains
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

#include <type_traits>

#include <ikos/core/number/q_number.hpp>
#include <ikos/core/number/z_number.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief Binary arithmetic operations
enum class BinaryOperator {
  /// \brief Addition
  Add,

  /// \brief Substraction
  Sub,

  /// \brief Multiplication
  Mul,

  /// \brief Division
  ///
  /// On integers (Z), this is the integer division with rounding towards zero.
  /// On rationals (Q), this is the exact division.
  Div,

  /// \brief Remainder
  ///
  /// On integers (Z), this is the integer remainder, see operator% on ZNumber.
  /// This is undefined on rationals.
  Rem,

  /// \brief Modulo
  ///
  /// On integers (Z), this is the integer modulo, see mod() on ZNumber.
  /// This is undefined on rationals.
  Mod,

  /// \brief Left binary shift
  ///
  /// On integers (Z), this is the left binary shift, see operator<< on ZNumber.
  /// This is undefined on rationals.
  Shl,

  /// \brief Right binary shift
  ///
  /// On integers (Z), this is the right binary shift, see operator>> on
  /// ZNumber.
  /// This is undefined on rationals.
  Shr,

  /// \brief Bitwise AND
  ///
  /// On integers (Z), this is the bitwise AND, see operator& on ZNumber.
  /// This is undefined on rationals.
  And,

  /// \brief Bitwise OR
  ///
  /// On integers (Z), this is the bitwise OR, see operator| on ZNumber.
  /// This is undefined on rationals.
  Or,

  /// \brief Bitwise XOR
  ///
  /// On integers (Z), this is the bitwise XOR, see operator^ on ZNumber.
  /// This is undefined on rationals.
  Xor,
};

/// \brief Get a textual representation of the given binary operator
inline const char* bin_operator_text(BinaryOperator op) {
  switch (op) {
    case BinaryOperator::Add:
      return "+";
    case BinaryOperator::Sub:
      return "-";
    case BinaryOperator::Mul:
      return "*";
    case BinaryOperator::Div:
      return "/";
    case BinaryOperator::Rem:
      return "%";
    case BinaryOperator::Mod:
      return "mod";
    case BinaryOperator::Shl:
      return "<<";
    case BinaryOperator::Shr:
      return ">>";
    case BinaryOperator::And:
      return "&";
    case BinaryOperator::Or:
      return "|";
    case BinaryOperator::Xor:
      return "^";
    default:
      ikos_unreachable("unsupported operator");
  }
}

namespace detail {

template < typename T, typename = void >
struct ApplyBinOperator {};

} // end namespace detail

/// \brief Apply an arithmetic binary operator on the given operands
template < typename T >
inline T apply_bin_operator(BinaryOperator op, const T& lhs, const T& rhs) {
  return detail::ApplyBinOperator< T >()(op, lhs, rhs);
}

namespace detail {

/// \brief Implementation for abstract values on ZNumber
template < typename T >
struct ApplyBinOperator<
    T,
    std::enable_if_t< std::is_same< typename T::NumberT, ZNumber >::value > > {
  inline T operator()(BinaryOperator op, const T& lhs, const T& rhs) const {
    switch (op) {
      case BinaryOperator::Add:
        return lhs + rhs;
      case BinaryOperator::Sub:
        return lhs - rhs;
      case BinaryOperator::Mul:
        return lhs * rhs;
      case BinaryOperator::Div:
        return lhs / rhs;
      case BinaryOperator::Rem:
        return lhs % rhs;
      case BinaryOperator::Mod:
        return mod(lhs, rhs);
      case BinaryOperator::Shl:
        return lhs << rhs;
      case BinaryOperator::Shr:
        return lhs >> rhs;
      case BinaryOperator::And:
        return lhs & rhs;
      case BinaryOperator::Or:
        return lhs | rhs;
      case BinaryOperator::Xor:
        return lhs ^ rhs;
      default:
        ikos_unreachable("unsupported operator");
    }
  }
};

/// \brief Implementation for abstract values on QNumber
template < typename T >
struct ApplyBinOperator<
    T,
    std::enable_if_t< std::is_same< typename T::NumberT, QNumber >::value > > {
  inline T operator()(BinaryOperator op, const T& lhs, const T& rhs) const {
    switch (op) {
      case BinaryOperator::Add:
        return lhs + rhs;
      case BinaryOperator::Sub:
        return lhs - rhs;
      case BinaryOperator::Mul:
        return lhs * rhs;
      case BinaryOperator::Div:
        return lhs / rhs;
      default:
        ikos_unreachable("unsupported operator");
    }
  }
};

} // end namespace detail

} // end namespace numeric
} // end namespace core
} // end namespace ikos
