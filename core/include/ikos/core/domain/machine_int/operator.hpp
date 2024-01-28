/*******************************************************************************
 *
 * \file
 * \brief Operators for machine integer abstract domains
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

#include <cstdint>

#include <ikos/core/number/signedness.hpp>
#include <ikos/core/support/assert.hpp>

namespace ikos {
namespace core {
namespace machine_int {

/// \brief Unary machine integer operations
enum class UnaryOperator {
  Trunc,
  Ext,
  SignCast,
  Cast,
};

/// \brief Binary machine integer operations
enum class BinaryOperator {
  Add,
  AddNoWrap,
  Sub,
  SubNoWrap,
  Mul,
  MulNoWrap,
  Div,
  DivExact,
  Rem,
  Shl,
  ShlNoWrap,
  LShr,
  LShrExact,
  AShr,
  AShrExact,
  And,
  Or,
  Xor,
};

/// \brief Predicate on machine integers
enum class Predicate {
  EQ,
  NE,
  GT,
  GE,
  LT,
  LE,
};

/// \brief Apply a machine integer unary operator on the given operands
template < typename T >
T apply_unary_operator(UnaryOperator op,
                       const T& operand,
                       uint64_t result_bit_width,
                       Signedness result_sign) {
  switch (op) {
    case UnaryOperator::Trunc:
      return operand.trunc(result_bit_width);
    case UnaryOperator::Ext:
      return operand.ext(result_bit_width);
    case UnaryOperator::SignCast:
      return operand.sign_cast(result_sign);
    case UnaryOperator::Cast:
      return operand.cast(result_bit_width, result_sign);
    default:
      ikos_unreachable("unreachable");
  }
}

/// \brief Apply a machine integer binary operator on the given operands
template < typename T >
T apply_bin_operator(BinaryOperator op, const T& lhs, const T& rhs) {
  switch (op) {
    case BinaryOperator::Add:
      return add(lhs, rhs);
    case BinaryOperator::AddNoWrap:
      return add_no_wrap(lhs, rhs);
    case BinaryOperator::Sub:
      return sub(lhs, rhs);
    case BinaryOperator::SubNoWrap:
      return sub_no_wrap(lhs, rhs);
    case BinaryOperator::Mul:
      return mul(lhs, rhs);
    case BinaryOperator::MulNoWrap:
      return mul_no_wrap(lhs, rhs);
    case BinaryOperator::Div:
      return div(lhs, rhs);
    case BinaryOperator::DivExact:
      return div_exact(lhs, rhs);
    case BinaryOperator::Rem:
      return rem(lhs, rhs);
    case BinaryOperator::Shl:
      return shl(lhs, rhs);
    case BinaryOperator::ShlNoWrap:
      return shl_no_wrap(lhs, rhs);
    case BinaryOperator::LShr:
      return lshr(lhs, rhs);
    case BinaryOperator::LShrExact:
      return lshr_exact(lhs, rhs);
    case BinaryOperator::AShr:
      return ashr(lhs, rhs);
    case BinaryOperator::AShrExact:
      return ashr_exact(lhs, rhs);
    case BinaryOperator::And:
      return and_(lhs, rhs);
    case BinaryOperator::Or:
      return or_(lhs, rhs);
    case BinaryOperator::Xor:
      return xor_(lhs, rhs);
    default:
      ikos_unreachable("unreachable");
  }
}

/// \brief Compare the given operands with the given predicate
template < typename T >
bool compare(Predicate pred, const T& lhs, const T& rhs) {
  switch (pred) {
    case Predicate::EQ:
      return lhs == rhs;
    case Predicate::NE:
      return lhs != rhs;
    case Predicate::GT:
      return lhs > rhs;
    case Predicate::GE:
      return lhs >= rhs;
    case Predicate::LT:
      return lhs < rhs;
    case Predicate::LE:
      return lhs <= rhs;
    default:
      ikos_unreachable("unreachable");
  }
}

} // end namespace machine_int
} // end namespace core
} // end namespace ikos
