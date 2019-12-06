/*******************************************************************************
 *
 * Tests for MachineInt
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

#define BOOST_TEST_MODULE test_machine_integer
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/number/machine_int.hpp>

using Int = ikos::core::MachineInt;
using ikos::core::Signed;
using ikos::core::Unsigned;

BOOST_AUTO_TEST_CASE(test_constructor_signed) {
  BOOST_CHECK(Int(0, 4, Signed) == Int(0, 4, Signed));
  BOOST_CHECK(Int(1, 4, Signed) == Int(1, 4, Signed));
  BOOST_CHECK(Int(7, 4, Signed) == Int(7, 4, Signed));
  BOOST_CHECK(Int(8, 4, Signed) == Int(-8, 4, Signed));
  BOOST_CHECK(Int(9, 4, Signed) == Int(-7, 4, Signed));
  BOOST_CHECK(Int(15, 4, Signed) == Int(-1, 4, Signed));
  BOOST_CHECK(Int(16, 4, Signed) == Int(0, 4, Signed));
  BOOST_CHECK(Int(17, 4, Signed) == Int(1, 4, Signed));
  BOOST_CHECK(Int(-1, 4, Signed) == Int(-1, 4, Signed));
  BOOST_CHECK(Int(-8, 4, Signed) == Int(-8, 4, Signed));
  BOOST_CHECK(Int(-9, 4, Signed) == Int(7, 4, Signed));
  BOOST_CHECK(Int(-15, 4, Signed) == Int(1, 4, Signed));
  BOOST_CHECK(Int(-16, 4, Signed) == Int(0, 4, Signed));
  BOOST_CHECK(Int(-17, 4, Signed) == Int(-1, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_constructor_unsigned) {
  BOOST_CHECK(Int(0, 4, Unsigned) == Int(0, 4, Unsigned));
  BOOST_CHECK(Int(1, 4, Unsigned) == Int(1, 4, Unsigned));
  BOOST_CHECK(Int(7, 4, Unsigned) == Int(7, 4, Unsigned));
  BOOST_CHECK(Int(8, 4, Unsigned) == Int(8, 4, Unsigned));
  BOOST_CHECK(Int(9, 4, Unsigned) == Int(9, 4, Unsigned));
  BOOST_CHECK(Int(15, 4, Unsigned) == Int(15, 4, Unsigned));
  BOOST_CHECK(Int(16, 4, Unsigned) == Int(0, 4, Unsigned));
  BOOST_CHECK(Int(17, 4, Unsigned) == Int(1, 4, Unsigned));
  BOOST_CHECK(Int(-1, 4, Unsigned) == Int(15, 4, Unsigned));
  BOOST_CHECK(Int(-8, 4, Unsigned) == Int(8, 4, Unsigned));
  BOOST_CHECK(Int(-9, 4, Unsigned) == Int(7, 4, Unsigned));
  BOOST_CHECK(Int(-15, 4, Unsigned) == Int(1, 4, Unsigned));
  BOOST_CHECK(Int(-16, 4, Unsigned) == Int(0, 4, Unsigned));
  BOOST_CHECK(Int(-17, 4, Unsigned) == Int(15, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_min) {
  BOOST_CHECK(Int::min(1, Signed) == Int(-1, 1, Signed));
  BOOST_CHECK(Int::min(4, Signed) == Int(-8, 4, Signed));
  BOOST_CHECK(Int::min(8, Signed) == Int(-128, 8, Signed));
  BOOST_CHECK(Int::min(1, Unsigned) == Int(0, 1, Unsigned));
  BOOST_CHECK(Int::min(4, Unsigned) == Int(0, 4, Unsigned));
  BOOST_CHECK(Int::min(8, Unsigned) == Int(0, 8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_max) {
  BOOST_CHECK(Int::max(1, Signed) == Int(0, 1, Signed));
  BOOST_CHECK(Int::max(4, Signed) == Int(7, 4, Signed));
  BOOST_CHECK(Int::max(8, Signed) == Int(127, 8, Signed));
  BOOST_CHECK(Int::max(1, Unsigned) == Int(1, 1, Unsigned));
  BOOST_CHECK(Int::max(4, Unsigned) == Int(15, 4, Unsigned));
  BOOST_CHECK(Int::max(8, Unsigned) == Int(255, 8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_zero) {
  BOOST_CHECK(Int::zero(1, Signed) == Int(0, 1, Signed));
  BOOST_CHECK(Int::zero(4, Signed) == Int(0, 4, Signed));
  BOOST_CHECK(Int::zero(8, Signed) == Int(0, 8, Signed));
  BOOST_CHECK(Int::zero(1, Unsigned) == Int(0, 1, Unsigned));
  BOOST_CHECK(Int::zero(4, Unsigned) == Int(0, 4, Unsigned));
  BOOST_CHECK(Int::zero(8, Unsigned) == Int(0, 8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_all_ones_constructor) {
  BOOST_CHECK(Int::all_ones(1, Signed) == Int(-1, 1, Signed));
  BOOST_CHECK(Int::all_ones(4, Signed) == Int(-1, 4, Signed));
  BOOST_CHECK(Int::all_ones(8, Signed) == Int(-1, 8, Signed));
  BOOST_CHECK(Int::all_ones(1, Unsigned) == Int(1, 1, Unsigned));
  BOOST_CHECK(Int::all_ones(4, Unsigned) == Int(15, 4, Unsigned));
  BOOST_CHECK(Int::all_ones(8, Unsigned) == Int(255, 8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_assign_add_signed) {
  BOOST_CHECK((Int(0, 4, Signed) += Int(1, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) += Int(1, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) += Int(1, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) += Int(1, 4, Signed)) == Int(-7, 4, Signed));
  BOOST_CHECK((Int(0, 4, Signed) += Int(-1, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) += Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) += Int(-1, 4, Signed)) == Int(6, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) += Int(-1, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK((Int(0, 4, Signed) += Int(7, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) += Int(7, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) += Int(7, 4, Signed)) == Int(-2, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) += Int(7, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK((Int(0, 4, Signed) += Int(-8, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) += Int(-8, 4, Signed)) == Int(-7, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) += Int(-8, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) += Int(-8, 4, Signed)) == Int(0, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_assign_add_unsigned) {
  BOOST_CHECK((Int(0, 4, Unsigned) += Int(1, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK((Int(1, 4, Unsigned) += Int(1, 4, Unsigned)) ==
              Int(2, 4, Unsigned));
  BOOST_CHECK((Int(15, 4, Unsigned) += Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK((Int(0, 4, Unsigned) += Int(15, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK((Int(1, 4, Unsigned) += Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK((Int(15, 4, Unsigned) += Int(15, 4, Unsigned)) ==
              Int(14, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_assign_sub_signed) {
  BOOST_CHECK((Int(0, 4, Signed) -= Int(1, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) -= Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) -= Int(1, 4, Signed)) == Int(6, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) -= Int(1, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK((Int(0, 4, Signed) -= Int(-1, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) -= Int(-1, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) -= Int(-1, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) -= Int(-1, 4, Signed)) == Int(-7, 4, Signed));
  BOOST_CHECK((Int(0, 4, Signed) -= Int(7, 4, Signed)) == Int(-7, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) -= Int(7, 4, Signed)) == Int(-6, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) -= Int(7, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) -= Int(7, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK((Int(0, 4, Signed) -= Int(-8, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) -= Int(-8, 4, Signed)) == Int(-7, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) -= Int(-8, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) -= Int(-8, 4, Signed)) == Int(0, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_assign_sub_unsigned) {
  BOOST_CHECK((Int(0, 4, Unsigned) -= Int(1, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK((Int(1, 4, Unsigned) -= Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK((Int(15, 4, Unsigned) -= Int(1, 4, Unsigned)) ==
              Int(14, 4, Unsigned));
  BOOST_CHECK((Int(0, 4, Unsigned) -= Int(15, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK((Int(1, 4, Unsigned) -= Int(15, 4, Unsigned)) ==
              Int(2, 4, Unsigned));
  BOOST_CHECK((Int(15, 4, Unsigned) -= Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_assign_mul_signed) {
  BOOST_CHECK((Int(0, 4, Signed) *= Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) *= Int(1, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) *= Int(1, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) *= Int(1, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK((Int(0, 4, Signed) *= Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) *= Int(-1, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) *= Int(-1, 4, Signed)) == Int(-7, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) *= Int(-1, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK((Int(0, 4, Signed) *= Int(7, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) *= Int(7, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) *= Int(7, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) *= Int(7, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK((Int(0, 4, Signed) *= Int(-8, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) *= Int(-8, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) *= Int(-8, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) *= Int(-8, 4, Signed)) == Int(0, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_assign_mul_unsigned) {
  BOOST_CHECK((Int(0, 4, Unsigned) *= Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK((Int(1, 4, Unsigned) *= Int(1, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK((Int(15, 4, Unsigned) *= Int(1, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK((Int(0, 4, Unsigned) *= Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK((Int(1, 4, Unsigned) *= Int(15, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK((Int(15, 4, Unsigned) *= Int(15, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK((Int(3, 4, Unsigned) *= Int(4, 4, Unsigned)) ==
              Int(12, 4, Unsigned));
  BOOST_CHECK((Int(4, 4, Unsigned) *= Int(4, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_assign_div_signed) {
  BOOST_CHECK((Int(0, 4, Signed) /= Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) /= Int(1, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) /= Int(1, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) /= Int(1, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK((Int(0, 4, Signed) /= Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) /= Int(-1, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) /= Int(-1, 4, Signed)) == Int(-7, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) /= Int(-1, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK((Int(0, 4, Signed) /= Int(7, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) /= Int(7, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) /= Int(7, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) /= Int(7, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK((Int(0, 4, Signed) /= Int(-8, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) /= Int(-8, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) /= Int(-8, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) /= Int(-8, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK((Int(4, 4, Signed) /= Int(2, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK((Int(5, 4, Signed) /= Int(2, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK((Int(-4, 4, Signed) /= Int(2, 4, Signed)) == Int(-2, 4, Signed));
  BOOST_CHECK((Int(-5, 4, Signed) /= Int(2, 4, Signed)) == Int(-2, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_assign_div_unsigned) {
  BOOST_CHECK((Int(0, 4, Unsigned) /= Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK((Int(1, 4, Unsigned) /= Int(1, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK((Int(15, 4, Unsigned) /= Int(1, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK((Int(0, 4, Unsigned) /= Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK((Int(1, 4, Unsigned) /= Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK((Int(15, 4, Unsigned) /= Int(15, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK((Int(3, 4, Unsigned) /= Int(2, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK((Int(4, 4, Unsigned) /= Int(2, 4, Unsigned)) ==
              Int(2, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_assign_rem_signed) {
  BOOST_CHECK((Int(0, 4, Signed) %= Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) %= Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) %= Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) %= Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(0, 4, Signed) %= Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) %= Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) %= Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) %= Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(0, 4, Signed) %= Int(7, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) %= Int(7, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) %= Int(7, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) %= Int(7, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK((Int(0, 4, Signed) %= Int(-8, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(1, 4, Signed) %= Int(-8, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK((Int(7, 4, Signed) %= Int(-8, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK((Int(-8, 4, Signed) %= Int(-8, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(4, 4, Signed) %= Int(2, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(5, 4, Signed) %= Int(2, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK((Int(-4, 4, Signed) %= Int(2, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((Int(-5, 4, Signed) %= Int(2, 4, Signed)) == Int(-1, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_assign_rem_unsigned) {
  BOOST_CHECK((Int(0, 4, Unsigned) %= Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK((Int(1, 4, Unsigned) %= Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK((Int(15, 4, Unsigned) %= Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK((Int(0, 4, Unsigned) %= Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK((Int(1, 4, Unsigned) %= Int(15, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK((Int(15, 4, Unsigned) %= Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK((Int(3, 4, Unsigned) %= Int(2, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK((Int(4, 4, Unsigned) %= Int(2, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_bit_width) {
  BOOST_CHECK(Int(0, 4, Signed).bit_width() == 4);
  BOOST_CHECK(Int(0, 4, Unsigned).bit_width() == 4);
  BOOST_CHECK(Int(0, 8, Signed).bit_width() == 8);
  BOOST_CHECK(Int(0, 8, Unsigned).bit_width() == 8);
}

BOOST_AUTO_TEST_CASE(test_sign) {
  BOOST_CHECK(Int(0, 4, Signed).sign() == Signed);
  BOOST_CHECK(Int(0, 4, Unsigned).sign() == Unsigned);
  BOOST_CHECK(Int(0, 8, Signed).sign() == Signed);
  BOOST_CHECK(Int(0, 8, Unsigned).sign() == Unsigned);
}

BOOST_AUTO_TEST_CASE(test_is_min) {
  BOOST_CHECK(!Int(0, 4, Signed).is_min());
  BOOST_CHECK(!Int(7, 4, Signed).is_min());
  BOOST_CHECK(Int(-8, 4, Signed).is_min());
  BOOST_CHECK(Int(0, 4, Unsigned).is_min());
  BOOST_CHECK(!Int(1, 4, Unsigned).is_min());
  BOOST_CHECK(!Int(15, 4, Unsigned).is_min());
}

BOOST_AUTO_TEST_CASE(test_is_max) {
  BOOST_CHECK(!Int(0, 4, Signed).is_max());
  BOOST_CHECK(Int(7, 4, Signed).is_max());
  BOOST_CHECK(!Int(-8, 4, Signed).is_max());
  BOOST_CHECK(!Int(0, 4, Unsigned).is_max());
  BOOST_CHECK(!Int(1, 4, Unsigned).is_max());
  BOOST_CHECK(Int(15, 4, Unsigned).is_max());
}

BOOST_AUTO_TEST_CASE(test_is_zero) {
  BOOST_CHECK(Int(0, 4, Signed).is_zero());
  BOOST_CHECK(!Int(7, 4, Signed).is_zero());
  BOOST_CHECK(!Int(-8, 4, Signed).is_zero());
  BOOST_CHECK(Int(0, 4, Unsigned).is_zero());
  BOOST_CHECK(!Int(1, 4, Unsigned).is_zero());
  BOOST_CHECK(!Int(15, 4, Unsigned).is_zero());
}

BOOST_AUTO_TEST_CASE(test_high_bit) {
  BOOST_CHECK(!Int(0, 4, Signed).high_bit());
  BOOST_CHECK(!Int(7, 4, Signed).high_bit());
  BOOST_CHECK(Int(-1, 4, Signed).high_bit());
  BOOST_CHECK(Int(-8, 4, Signed).high_bit());
  BOOST_CHECK(!Int(0, 4, Unsigned).high_bit());
  BOOST_CHECK(!Int(1, 4, Unsigned).high_bit());
  BOOST_CHECK(Int(8, 4, Unsigned).high_bit());
  BOOST_CHECK(Int(15, 4, Unsigned).high_bit());
}

BOOST_AUTO_TEST_CASE(test_is_negative) {
  BOOST_CHECK(!Int(0, 4, Signed).is_negative());
  BOOST_CHECK(!Int(7, 4, Signed).is_negative());
  BOOST_CHECK(Int(-1, 4, Signed).is_negative());
  BOOST_CHECK(Int(-8, 4, Signed).is_negative());
  BOOST_CHECK(!Int(0, 4, Unsigned).is_negative());
  BOOST_CHECK(!Int(1, 4, Unsigned).is_negative());
  BOOST_CHECK(!Int(15, 4, Unsigned).is_negative());
}

BOOST_AUTO_TEST_CASE(test_is_non_negative) {
  BOOST_CHECK(Int(0, 4, Signed).is_non_negative());
  BOOST_CHECK(Int(7, 4, Signed).is_non_negative());
  BOOST_CHECK(!Int(-1, 4, Signed).is_non_negative());
  BOOST_CHECK(!Int(-8, 4, Signed).is_non_negative());
  BOOST_CHECK(Int(0, 4, Unsigned).is_non_negative());
  BOOST_CHECK(Int(1, 4, Unsigned).is_non_negative());
  BOOST_CHECK(Int(15, 4, Unsigned).is_non_negative());
}

BOOST_AUTO_TEST_CASE(test_is_strictly_positive) {
  BOOST_CHECK(!Int(0, 4, Signed).is_strictly_positive());
  BOOST_CHECK(Int(7, 4, Signed).is_strictly_positive());
  BOOST_CHECK(!Int(-1, 4, Signed).is_strictly_positive());
  BOOST_CHECK(!Int(-8, 4, Signed).is_strictly_positive());
  BOOST_CHECK(!Int(0, 4, Unsigned).is_strictly_positive());
  BOOST_CHECK(Int(1, 4, Unsigned).is_strictly_positive());
  BOOST_CHECK(Int(15, 4, Unsigned).is_strictly_positive());
}

BOOST_AUTO_TEST_CASE(test_all_ones_method) {
  BOOST_CHECK(!Int(0, 4, Signed).all_ones());
  BOOST_CHECK(!Int(7, 4, Signed).all_ones());
  BOOST_CHECK(Int(-1, 4, Signed).all_ones());
  BOOST_CHECK(!Int(-8, 4, Signed).all_ones());
  BOOST_CHECK(!Int(0, 4, Unsigned).all_ones());
  BOOST_CHECK(!Int(1, 4, Unsigned).all_ones());
  BOOST_CHECK(!Int(8, 4, Unsigned).all_ones());
  BOOST_CHECK(Int(15, 4, Unsigned).all_ones());
}

BOOST_AUTO_TEST_CASE(test_leading_zeros) {
  BOOST_CHECK(Int(0, 4, Signed).leading_zeros() == 4);
  BOOST_CHECK(Int(1, 4, Signed).leading_zeros() == 3);
  BOOST_CHECK(Int(7, 4, Signed).leading_zeros() == 1);
  BOOST_CHECK(Int(-1, 4, Signed).leading_zeros() == 0);
  BOOST_CHECK(Int(-2, 4, Signed).leading_zeros() == 0);
  BOOST_CHECK(Int(-8, 4, Signed).leading_zeros() == 0);
  BOOST_CHECK(Int(0, 4, Unsigned).leading_zeros() == 4);
  BOOST_CHECK(Int(1, 4, Unsigned).leading_zeros() == 3);
  BOOST_CHECK(Int(3, 4, Unsigned).leading_zeros() == 2);
  BOOST_CHECK(Int(8, 4, Unsigned).leading_zeros() == 0);
  BOOST_CHECK(Int(15, 4, Unsigned).leading_zeros() == 0);
}

BOOST_AUTO_TEST_CASE(test_leading_ones) {
  BOOST_CHECK(Int(0, 4, Signed).leading_ones() == 0);
  BOOST_CHECK(Int(1, 4, Signed).leading_ones() == 0);
  BOOST_CHECK(Int(7, 4, Signed).leading_ones() == 0);
  BOOST_CHECK(Int(-1, 4, Signed).leading_ones() == 4);
  BOOST_CHECK(Int(-2, 4, Signed).leading_ones() == 3);
  BOOST_CHECK(Int(-8, 4, Signed).leading_ones() == 1);
  BOOST_CHECK(Int(0, 4, Unsigned).leading_ones() == 0);
  BOOST_CHECK(Int(1, 4, Unsigned).leading_ones() == 0);
  BOOST_CHECK(Int(3, 4, Unsigned).leading_ones() == 0);
  BOOST_CHECK(Int(8, 4, Unsigned).leading_ones() == 1);
  BOOST_CHECK(Int(15, 4, Unsigned).leading_ones() == 4);
}

BOOST_AUTO_TEST_CASE(test_to_z_number) {
  BOOST_CHECK(Int(0, 4, Signed).to_z_number() == 0);
  BOOST_CHECK(Int(7, 4, Signed).to_z_number() == 7);
  BOOST_CHECK(Int(23, 4, Signed).to_z_number() == 7);
  BOOST_CHECK(Int(-8, 4, Signed).to_z_number() == -8);
  BOOST_CHECK(Int(-24, 4, Signed).to_z_number() == -8);
  BOOST_CHECK(Int(8, 4, Signed).to_z_number() == -8);
  BOOST_CHECK(Int(-9, 4, Signed).to_z_number() == 7);

  BOOST_CHECK(Int(0, 4, Unsigned).to_z_number() == 0);
  BOOST_CHECK(Int(15, 4, Unsigned).to_z_number() == 15);
  BOOST_CHECK(Int(31, 4, Unsigned).to_z_number() == 15);
  BOOST_CHECK(Int(16, 4, Unsigned).to_z_number() == 0);
}

BOOST_AUTO_TEST_CASE(test_fits) {
  BOOST_CHECK(Int(0, 32, Signed).fits< int32_t >());
  BOOST_CHECK(Int(1, 32, Signed).fits< int32_t >());
  BOOST_CHECK(Int(-1, 32, Signed).fits< int32_t >());
  BOOST_CHECK(Int::max(32, Signed).fits< int32_t >());
  BOOST_CHECK(Int::min(32, Signed).fits< int32_t >());
  BOOST_CHECK(Int(0, 32, Signed).fits< uint32_t >());
  BOOST_CHECK(Int(1, 32, Signed).fits< uint32_t >());
  BOOST_CHECK(!Int(-1, 32, Signed).fits< uint32_t >());
  BOOST_CHECK(Int::max(32, Signed).fits< uint32_t >());
  BOOST_CHECK(!Int::min(32, Signed).fits< uint32_t >());
  BOOST_CHECK(Int(0, 32, Signed).fits< int64_t >());
  BOOST_CHECK(Int(1, 32, Signed).fits< int64_t >());
  BOOST_CHECK(Int(-1, 32, Signed).fits< int64_t >());
  BOOST_CHECK(Int::max(32, Signed).fits< int64_t >());
  BOOST_CHECK(Int::min(32, Signed).fits< int64_t >());
  BOOST_CHECK(Int(0, 32, Signed).fits< uint64_t >());
  BOOST_CHECK(Int(1, 32, Signed).fits< uint64_t >());
  BOOST_CHECK(!Int(-1, 32, Signed).fits< uint64_t >());
  BOOST_CHECK(Int::max(32, Signed).fits< uint64_t >());
  BOOST_CHECK(!Int::min(32, Signed).fits< uint64_t >());

  BOOST_CHECK(Int(0, 64, Signed).fits< int32_t >());
  BOOST_CHECK(Int(1, 64, Signed).fits< int32_t >());
  BOOST_CHECK(Int(-1, 64, Signed).fits< int32_t >());
  BOOST_CHECK(!Int::max(64, Signed).fits< int32_t >());
  BOOST_CHECK(!Int::min(64, Signed).fits< int32_t >());
  BOOST_CHECK(Int(0, 64, Signed).fits< uint32_t >());
  BOOST_CHECK(Int(1, 64, Signed).fits< uint32_t >());
  BOOST_CHECK(!Int(-1, 64, Signed).fits< uint32_t >());
  BOOST_CHECK(!Int::max(64, Signed).fits< uint32_t >());
  BOOST_CHECK(!Int::min(64, Signed).fits< uint32_t >());
  BOOST_CHECK(Int(0, 64, Signed).fits< int64_t >());
  BOOST_CHECK(Int(1, 64, Signed).fits< int64_t >());
  BOOST_CHECK(Int(-1, 64, Signed).fits< int64_t >());
  BOOST_CHECK(Int::max(64, Signed).fits< int64_t >());
  BOOST_CHECK(Int::min(64, Signed).fits< int64_t >());
  BOOST_CHECK(Int(0, 64, Signed).fits< uint64_t >());
  BOOST_CHECK(Int(1, 64, Signed).fits< uint64_t >());
  BOOST_CHECK(!Int(-1, 64, Signed).fits< uint64_t >());
  BOOST_CHECK(Int::max(64, Signed).fits< uint64_t >());
  BOOST_CHECK(!Int::min(64, Signed).fits< uint64_t >());

  BOOST_CHECK(Int(0, 32, Unsigned).fits< int32_t >());
  BOOST_CHECK(Int(1, 32, Unsigned).fits< int32_t >());
  BOOST_CHECK(!Int::max(32, Unsigned).fits< int32_t >());
  BOOST_CHECK(Int(0, 32, Unsigned).fits< uint32_t >());
  BOOST_CHECK(Int(1, 32, Unsigned).fits< uint32_t >());
  BOOST_CHECK(Int::max(32, Unsigned).fits< uint32_t >());
  BOOST_CHECK(Int(0, 64, Unsigned).fits< int32_t >());
  BOOST_CHECK(Int(1, 64, Unsigned).fits< int32_t >());
  BOOST_CHECK(!Int::max(64, Unsigned).fits< int32_t >());
  BOOST_CHECK(Int(0, 64, Unsigned).fits< uint32_t >());
  BOOST_CHECK(Int(1, 64, Unsigned).fits< uint32_t >());
  BOOST_CHECK(!Int::max(64, Unsigned).fits< uint32_t >());
  BOOST_CHECK(Int(0, 64, Unsigned).fits< int64_t >());
  BOOST_CHECK(Int(1, 64, Unsigned).fits< int64_t >());
  BOOST_CHECK(!Int::max(64, Unsigned).fits< int64_t >());
  BOOST_CHECK(Int(0, 64, Unsigned).fits< uint64_t >());
  BOOST_CHECK(Int(1, 64, Unsigned).fits< uint64_t >());
  BOOST_CHECK(Int::max(64, Unsigned).fits< uint64_t >());
}

BOOST_AUTO_TEST_CASE(test_to) {
  BOOST_CHECK(Int(0, 32, Signed).to< int32_t >() == int32_t(0));
  BOOST_CHECK(Int(1, 32, Signed).to< int32_t >() == int32_t(1));
  BOOST_CHECK(Int(-1, 32, Signed).to< int32_t >() == int32_t(-1));
  BOOST_CHECK(Int::max(32, Signed).to< int32_t >() ==
              std::numeric_limits< int32_t >::max());
  BOOST_CHECK(Int::min(32, Signed).to< int32_t >() ==
              std::numeric_limits< int32_t >::min());
  BOOST_CHECK(Int(0, 32, Signed).to< uint32_t >() == uint32_t(0));
  BOOST_CHECK(Int(1, 32, Signed).to< uint32_t >() == uint32_t(1));
  BOOST_CHECK(Int::max(32, Signed).to< uint32_t >() ==
              uint32_t(std::numeric_limits< int32_t >::max()));
  BOOST_CHECK(Int(0, 32, Signed).to< int64_t >() == int64_t(0));
  BOOST_CHECK(Int(1, 32, Signed).to< int64_t >() == int64_t(1));
  BOOST_CHECK(Int(-1, 32, Signed).to< int64_t >() == int64_t(-1));
  BOOST_CHECK(Int::max(32, Signed).to< int64_t >() ==
              int64_t(std::numeric_limits< int32_t >::max()));
  BOOST_CHECK(Int::min(32, Signed).to< int64_t >() ==
              int64_t(std::numeric_limits< int32_t >::min()));
  BOOST_CHECK(Int(0, 32, Signed).to< uint64_t >() == uint64_t(0));
  BOOST_CHECK(Int(1, 32, Signed).to< uint64_t >() == uint64_t(1));
  BOOST_CHECK(Int::max(32, Signed).to< uint64_t >() ==
              uint64_t(std::numeric_limits< int32_t >::max()));

  BOOST_CHECK(Int(0, 64, Signed).to< int32_t >() == int32_t(0));
  BOOST_CHECK(Int(1, 64, Signed).to< int32_t >() == int32_t(1));
  BOOST_CHECK(Int(-1, 64, Signed).to< int32_t >() == int32_t(-1));
  BOOST_CHECK(Int(0, 64, Signed).to< uint32_t >() == uint32_t(0));
  BOOST_CHECK(Int(1, 64, Signed).to< uint32_t >() == uint32_t(1));
  BOOST_CHECK(Int(0, 64, Signed).to< int64_t >() == int64_t(0));
  BOOST_CHECK(Int(1, 64, Signed).to< int64_t >() == int64_t(1));
  BOOST_CHECK(Int::max(64, Signed).to< int64_t >() ==
              std::numeric_limits< int64_t >::max());
  BOOST_CHECK(Int::min(64, Signed).to< int64_t >() ==
              std::numeric_limits< int64_t >::min());
  BOOST_CHECK(Int(0, 64, Signed).to< uint64_t >() == uint64_t(0));
  BOOST_CHECK(Int(1, 64, Signed).to< uint64_t >() == uint64_t(1));
  BOOST_CHECK(Int::max(64, Signed).to< uint64_t >() ==
              uint64_t(std::numeric_limits< int64_t >::max()));

  BOOST_CHECK(Int(0, 32, Unsigned).to< int32_t >() == int32_t(0));
  BOOST_CHECK(Int(1, 32, Unsigned).to< int32_t >() == int32_t(1));
  BOOST_CHECK(Int(0, 32, Unsigned).to< uint32_t >() == uint32_t(0));
  BOOST_CHECK(Int(1, 32, Unsigned).to< uint32_t >() == uint32_t(1));
  BOOST_CHECK(Int::max(32, Unsigned).to< uint32_t >() ==
              std::numeric_limits< uint32_t >::max());
  BOOST_CHECK(Int(0, 64, Unsigned).to< int32_t >() == int32_t(0));
  BOOST_CHECK(Int(1, 64, Unsigned).to< int32_t >() == int32_t(1));
  BOOST_CHECK(Int(0, 64, Unsigned).to< uint32_t >() == uint32_t(0));
  BOOST_CHECK(Int(1, 64, Unsigned).to< uint32_t >() == uint32_t(1));
  BOOST_CHECK(Int(0, 64, Unsigned).to< int64_t >() == int64_t(0));
  BOOST_CHECK(Int(1, 64, Unsigned).to< int64_t >() == int64_t(1));
  BOOST_CHECK(Int(0, 64, Unsigned).to< uint64_t >() == uint64_t(0));
  BOOST_CHECK(Int(1, 64, Unsigned).to< uint64_t >() == uint64_t(1));
  BOOST_CHECK(Int::max(64, Unsigned).to< uint64_t >() ==
              std::numeric_limits< uint64_t >::max());
}

BOOST_AUTO_TEST_CASE(test_set_min) {
  Int n = Int(1, 4, Signed);
  n.set_min();
  BOOST_CHECK(n.is_min());

  n = Int(1, 4, Unsigned);
  n.set_min();
  BOOST_CHECK(n.is_min());
}

BOOST_AUTO_TEST_CASE(test_set_max) {
  Int n = Int(1, 4, Signed);
  n.set_max();
  BOOST_CHECK(n.is_max());

  n = Int(1, 4, Unsigned);
  n.set_max();
  BOOST_CHECK(n.is_max());
}

BOOST_AUTO_TEST_CASE(test_set_zero) {
  Int n = Int(1, 4, Signed);
  n.set_zero();
  BOOST_CHECK(n.is_zero());

  n = Int(1, 4, Unsigned);
  n.set_zero();
  BOOST_CHECK(n.is_zero());
}

BOOST_AUTO_TEST_CASE(test_incr_signed) {
  BOOST_CHECK((++Int(0, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK((++Int(1, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK((++Int(7, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK((++Int(-8, 4, Signed)) == Int(-7, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_incr_unsigned) {
  BOOST_CHECK((++Int(0, 4, Unsigned)) == Int(1, 4, Unsigned));
  BOOST_CHECK((++Int(1, 4, Unsigned)) == Int(2, 4, Unsigned));
  BOOST_CHECK((++Int(14, 4, Unsigned)) == Int(15, 4, Unsigned));
  BOOST_CHECK((++Int(15, 4, Unsigned)) == Int(0, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_decr_signed) {
  BOOST_CHECK((--Int(0, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK((--Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((--Int(7, 4, Signed)) == Int(6, 4, Signed));
  BOOST_CHECK((--Int(-8, 4, Signed)) == Int(7, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_decr_unsigned) {
  BOOST_CHECK((--Int(0, 4, Unsigned)) == Int(15, 4, Unsigned));
  BOOST_CHECK((--Int(1, 4, Unsigned)) == Int(0, 4, Unsigned));
  BOOST_CHECK((--Int(14, 4, Unsigned)) == Int(13, 4, Unsigned));
  BOOST_CHECK((--Int(15, 4, Unsigned)) == Int(14, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_minus_signed) {
  BOOST_CHECK((-Int(0, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK((-Int(1, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK((-Int(7, 4, Signed)) == Int(-7, 4, Signed));
  BOOST_CHECK((-Int(-7, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK((-Int(-8, 4, Signed)) == Int(-8, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_minus_unsigned) {
  BOOST_CHECK((-Int(0, 4, Unsigned)) == Int(0, 4, Unsigned));
  BOOST_CHECK((-Int(1, 4, Unsigned)) == Int(15, 4, Unsigned));
  BOOST_CHECK((-Int(14, 4, Unsigned)) == Int(2, 4, Unsigned));
  BOOST_CHECK((-Int(15, 4, Unsigned)) == Int(1, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_not_signed) {
  BOOST_CHECK((~Int(0, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK((~Int(1, 4, Signed)) == Int(-2, 4, Signed));
  BOOST_CHECK((~Int(7, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK((~Int(-7, 4, Signed)) == Int(6, 4, Signed));
  BOOST_CHECK((~Int(-8, 4, Signed)) == Int(7, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_not_unsigned) {
  BOOST_CHECK((~Int(0, 4, Unsigned)) == Int(15, 4, Unsigned));
  BOOST_CHECK((~Int(1, 4, Unsigned)) == Int(14, 4, Unsigned));
  BOOST_CHECK((~Int(14, 4, Unsigned)) == Int(1, 4, Unsigned));
  BOOST_CHECK((~Int(15, 4, Unsigned)) == Int(0, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_trunc_signed) {
  BOOST_CHECK(Int(0, 4, Signed).trunc(3) == Int(0, 3, Signed));
  BOOST_CHECK(Int(1, 4, Signed).trunc(3) == Int(1, 3, Signed));
  BOOST_CHECK(Int(2, 4, Signed).trunc(3) == Int(2, 3, Signed));
  BOOST_CHECK(Int(7, 4, Signed).trunc(3) == Int(-1, 3, Signed));
  BOOST_CHECK(Int(-8, 4, Signed).trunc(3) == Int(0, 3, Signed));
  BOOST_CHECK(Int(0, 4, Signed).trunc(2) == Int(0, 2, Signed));
  BOOST_CHECK(Int(1, 4, Signed).trunc(2) == Int(1, 2, Signed));
  BOOST_CHECK(Int(2, 4, Signed).trunc(2) == Int(-2, 2, Signed));
  BOOST_CHECK(Int(7, 4, Signed).trunc(2) == Int(-1, 2, Signed));
  BOOST_CHECK(Int(-8, 4, Signed).trunc(2) == Int(0, 2, Signed));
  BOOST_CHECK(Int(0, 4, Signed).trunc(1) == Int(0, 1, Signed));
  BOOST_CHECK(Int(1, 4, Signed).trunc(1) == Int(-1, 1, Signed));
  BOOST_CHECK(Int(2, 4, Signed).trunc(1) == Int(0, 1, Signed));
  BOOST_CHECK(Int(7, 4, Signed).trunc(1) == Int(-1, 1, Signed));
  BOOST_CHECK(Int(-8, 4, Signed).trunc(1) == Int(0, 1, Signed));
}

BOOST_AUTO_TEST_CASE(test_trunc_unsigned) {
  BOOST_CHECK(Int(0, 4, Unsigned).trunc(3) == Int(0, 3, Unsigned));
  BOOST_CHECK(Int(1, 4, Unsigned).trunc(3) == Int(1, 3, Unsigned));
  BOOST_CHECK(Int(2, 4, Unsigned).trunc(3) == Int(2, 3, Unsigned));
  BOOST_CHECK(Int(14, 4, Unsigned).trunc(3) == Int(6, 3, Unsigned));
  BOOST_CHECK(Int(15, 4, Unsigned).trunc(3) == Int(7, 3, Unsigned));
  BOOST_CHECK(Int(0, 4, Unsigned).trunc(2) == Int(0, 2, Unsigned));
  BOOST_CHECK(Int(1, 4, Unsigned).trunc(2) == Int(1, 2, Unsigned));
  BOOST_CHECK(Int(2, 4, Unsigned).trunc(2) == Int(2, 2, Unsigned));
  BOOST_CHECK(Int(14, 4, Unsigned).trunc(2) == Int(2, 2, Unsigned));
  BOOST_CHECK(Int(15, 4, Unsigned).trunc(2) == Int(3, 2, Unsigned));
  BOOST_CHECK(Int(0, 4, Unsigned).trunc(1) == Int(0, 1, Unsigned));
  BOOST_CHECK(Int(1, 4, Unsigned).trunc(1) == Int(1, 1, Unsigned));
  BOOST_CHECK(Int(2, 4, Unsigned).trunc(1) == Int(0, 1, Unsigned));
  BOOST_CHECK(Int(14, 4, Unsigned).trunc(1) == Int(0, 1, Unsigned));
  BOOST_CHECK(Int(15, 4, Unsigned).trunc(1) == Int(1, 1, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_ext_signed) {
  BOOST_CHECK(Int(0, 4, Signed).ext(5) == Int(0, 5, Signed));
  BOOST_CHECK(Int(1, 4, Signed).ext(5) == Int(1, 5, Signed));
  BOOST_CHECK(Int(2, 4, Signed).ext(5) == Int(2, 5, Signed));
  BOOST_CHECK(Int(7, 4, Signed).ext(5) == Int(7, 5, Signed));
  BOOST_CHECK(Int(-8, 4, Signed).ext(5) == Int(-8, 5, Signed));
  BOOST_CHECK(Int(0, 4, Signed).ext(6) == Int(0, 6, Signed));
  BOOST_CHECK(Int(1, 4, Signed).ext(6) == Int(1, 6, Signed));
  BOOST_CHECK(Int(2, 4, Signed).ext(6) == Int(2, 6, Signed));
  BOOST_CHECK(Int(7, 4, Signed).ext(6) == Int(7, 6, Signed));
  BOOST_CHECK(Int(-8, 4, Signed).ext(6) == Int(-8, 6, Signed));
}

BOOST_AUTO_TEST_CASE(test_ext_unsigned) {
  BOOST_CHECK(Int(0, 4, Unsigned).ext(5) == Int(0, 5, Unsigned));
  BOOST_CHECK(Int(1, 4, Unsigned).ext(5) == Int(1, 5, Unsigned));
  BOOST_CHECK(Int(2, 4, Unsigned).ext(5) == Int(2, 5, Unsigned));
  BOOST_CHECK(Int(14, 4, Unsigned).ext(5) == Int(14, 5, Unsigned));
  BOOST_CHECK(Int(15, 4, Unsigned).ext(5) == Int(15, 5, Unsigned));
  BOOST_CHECK(Int(0, 4, Unsigned).ext(6) == Int(0, 6, Unsigned));
  BOOST_CHECK(Int(1, 4, Unsigned).ext(6) == Int(1, 6, Unsigned));
  BOOST_CHECK(Int(2, 4, Unsigned).ext(6) == Int(2, 6, Unsigned));
  BOOST_CHECK(Int(14, 4, Unsigned).ext(6) == Int(14, 6, Unsigned));
  BOOST_CHECK(Int(15, 4, Unsigned).ext(6) == Int(15, 6, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_sign_cast_signed) {
  BOOST_CHECK(Int(0, 4, Signed).sign_cast(Unsigned) == Int(0, 4, Unsigned));
  BOOST_CHECK(Int(1, 4, Signed).sign_cast(Unsigned) == Int(1, 4, Unsigned));
  BOOST_CHECK(Int(2, 4, Signed).sign_cast(Unsigned) == Int(2, 4, Unsigned));
  BOOST_CHECK(Int(7, 4, Signed).sign_cast(Unsigned) == Int(7, 4, Unsigned));
  BOOST_CHECK(Int(-7, 4, Signed).sign_cast(Unsigned) == Int(9, 4, Unsigned));
  BOOST_CHECK(Int(-8, 4, Signed).sign_cast(Unsigned) == Int(8, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_sign_cast_unsigned) {
  BOOST_CHECK(Int(0, 4, Unsigned).sign_cast(Signed) == Int(0, 4, Signed));
  BOOST_CHECK(Int(1, 4, Unsigned).sign_cast(Signed) == Int(1, 4, Signed));
  BOOST_CHECK(Int(2, 4, Unsigned).sign_cast(Signed) == Int(2, 4, Signed));
  BOOST_CHECK(Int(14, 4, Unsigned).sign_cast(Signed) == Int(-2, 4, Signed));
  BOOST_CHECK(Int(15, 4, Unsigned).sign_cast(Signed) == Int(-1, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_cast) {
  BOOST_CHECK(Int(0, 4, Signed).cast(8, Unsigned) == Int(0, 8, Unsigned));
  BOOST_CHECK(Int(1, 4, Signed).cast(8, Unsigned) == Int(1, 8, Unsigned));
  BOOST_CHECK(Int(2, 4, Signed).cast(8, Unsigned) == Int(2, 8, Unsigned));
  BOOST_CHECK(Int(7, 4, Signed).cast(8, Unsigned) == Int(7, 8, Unsigned));
  BOOST_CHECK(Int(-7, 4, Signed).cast(8, Unsigned) == Int(249, 8, Unsigned));
  BOOST_CHECK(Int(-8, 4, Signed).cast(8, Unsigned) == Int(248, 8, Unsigned));
  BOOST_CHECK(Int(-1, 4, Signed).cast(8, Unsigned) == Int(255, 8, Unsigned));
  BOOST_CHECK(Int(0, 4, Unsigned).cast(8, Signed) == Int(0, 8, Signed));
  BOOST_CHECK(Int(1, 4, Unsigned).cast(8, Signed) == Int(1, 8, Signed));
  BOOST_CHECK(Int(2, 4, Unsigned).cast(8, Signed) == Int(2, 8, Signed));
  BOOST_CHECK(Int(14, 4, Unsigned).cast(8, Signed) == Int(14, 8, Signed));
  BOOST_CHECK(Int(15, 4, Unsigned).cast(8, Signed) == Int(15, 8, Signed));
  BOOST_CHECK(Int(0, 4, Signed).cast(3, Unsigned) == Int(0, 3, Unsigned));
  BOOST_CHECK(Int(1, 4, Signed).cast(3, Unsigned) == Int(1, 3, Unsigned));
  BOOST_CHECK(Int(2, 4, Signed).cast(3, Unsigned) == Int(2, 3, Unsigned));
  BOOST_CHECK(Int(7, 4, Signed).cast(3, Unsigned) == Int(7, 3, Unsigned));
  BOOST_CHECK(Int(-8, 4, Signed).cast(3, Unsigned) == Int(0, 3, Unsigned));
  BOOST_CHECK(Int(0, 4, Signed).cast(2, Unsigned) == Int(0, 2, Unsigned));
  BOOST_CHECK(Int(1, 4, Signed).cast(2, Unsigned) == Int(1, 2, Unsigned));
  BOOST_CHECK(Int(2, 4, Signed).cast(2, Unsigned) == Int(2, 2, Unsigned));
  BOOST_CHECK(Int(7, 4, Signed).cast(2, Unsigned) == Int(3, 2, Unsigned));
  BOOST_CHECK(Int(-8, 4, Signed).cast(2, Unsigned) == Int(0, 2, Unsigned));
  BOOST_CHECK(Int(0, 4, Signed).cast(1, Unsigned) == Int(0, 1, Unsigned));
  BOOST_CHECK(Int(1, 4, Signed).cast(1, Unsigned) == Int(1, 1, Unsigned));
  BOOST_CHECK(Int(2, 4, Signed).cast(1, Unsigned) == Int(0, 1, Unsigned));
  BOOST_CHECK(Int(7, 4, Signed).cast(1, Unsigned) == Int(1, 1, Unsigned));
  BOOST_CHECK(Int(-8, 4, Signed).cast(1, Unsigned) == Int(0, 1, Unsigned));
  BOOST_CHECK(Int(0, 4, Unsigned).cast(3, Signed) == Int(0, 3, Signed));
  BOOST_CHECK(Int(1, 4, Unsigned).cast(3, Signed) == Int(1, 3, Signed));
  BOOST_CHECK(Int(2, 4, Unsigned).cast(3, Signed) == Int(2, 3, Signed));
  BOOST_CHECK(Int(14, 4, Unsigned).cast(3, Signed) == Int(-2, 3, Signed));
  BOOST_CHECK(Int(15, 4, Unsigned).cast(3, Signed) == Int(-1, 3, Signed));
  BOOST_CHECK(Int(0, 4, Unsigned).cast(2, Signed) == Int(0, 2, Signed));
  BOOST_CHECK(Int(1, 4, Unsigned).cast(2, Signed) == Int(1, 2, Signed));
  BOOST_CHECK(Int(2, 4, Unsigned).cast(2, Signed) == Int(2, 2, Signed));
  BOOST_CHECK(Int(14, 4, Unsigned).cast(2, Signed) == Int(-2, 2, Signed));
  BOOST_CHECK(Int(15, 4, Unsigned).cast(2, Signed) == Int(-1, 2, Signed));
  BOOST_CHECK(Int(0, 4, Unsigned).cast(1, Signed) == Int(0, 1, Signed));
  BOOST_CHECK(Int(1, 4, Unsigned).cast(1, Signed) == Int(-1, 1, Signed));
  BOOST_CHECK(Int(2, 4, Unsigned).cast(1, Signed) == Int(0, 1, Signed));
  BOOST_CHECK(Int(14, 4, Unsigned).cast(1, Signed) == Int(0, 1, Signed));
  BOOST_CHECK(Int(15, 4, Unsigned).cast(1, Signed) == Int(-1, 1, Signed));
}

BOOST_AUTO_TEST_CASE(test_add_signed) {
  BOOST_CHECK(add(Int(0, 4, Signed), Int(1, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(add(Int(1, 4, Signed), Int(1, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK(add(Int(7, 4, Signed), Int(1, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK(add(Int(-8, 4, Signed), Int(1, 4, Signed)) == Int(-7, 4, Signed));
  BOOST_CHECK(add(Int(0, 4, Signed), Int(-1, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK(add(Int(1, 4, Signed), Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(add(Int(7, 4, Signed), Int(-1, 4, Signed)) == Int(6, 4, Signed));
  BOOST_CHECK(add(Int(-8, 4, Signed), Int(-1, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(add(Int(0, 4, Signed), Int(7, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(add(Int(1, 4, Signed), Int(7, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK(add(Int(7, 4, Signed), Int(7, 4, Signed)) == Int(-2, 4, Signed));
  BOOST_CHECK(add(Int(-8, 4, Signed), Int(7, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK(add(Int(0, 4, Signed), Int(-8, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK(add(Int(1, 4, Signed), Int(-8, 4, Signed)) == Int(-7, 4, Signed));
  BOOST_CHECK(add(Int(7, 4, Signed), Int(-8, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK(add(Int(-8, 4, Signed), Int(-8, 4, Signed)) == Int(0, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_add_with_overflow_signed) {
  bool overflow = false;
  BOOST_CHECK((add(Int(0, 4, Signed), Int(1, 4, Signed), overflow) ==
                   Int(1, 4, Signed) &&
               !overflow));
  BOOST_CHECK((add(Int(1, 4, Signed), Int(1, 4, Signed), overflow) ==
                   Int(2, 4, Signed) &&
               !overflow));
  BOOST_CHECK((add(Int(7, 4, Signed), Int(1, 4, Signed), overflow) ==
                   Int(-8, 4, Signed) &&
               overflow));
  BOOST_CHECK((add(Int(-8, 4, Signed), Int(1, 4, Signed), overflow) ==
                   Int(-7, 4, Signed) &&
               !overflow));
  BOOST_CHECK((add(Int(0, 4, Signed), Int(-1, 4, Signed), overflow) ==
                   Int(-1, 4, Signed) &&
               !overflow));
  BOOST_CHECK((add(Int(1, 4, Signed), Int(-1, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               !overflow));
  BOOST_CHECK((add(Int(7, 4, Signed), Int(-1, 4, Signed), overflow) ==
                   Int(6, 4, Signed) &&
               !overflow));
  BOOST_CHECK((add(Int(-8, 4, Signed), Int(-1, 4, Signed), overflow) ==
                   Int(7, 4, Signed) &&
               overflow));
  BOOST_CHECK((add(Int(0, 4, Signed), Int(7, 4, Signed), overflow) ==
                   Int(7, 4, Signed) &&
               !overflow));
  BOOST_CHECK((add(Int(1, 4, Signed), Int(7, 4, Signed), overflow) ==
                   Int(-8, 4, Signed) &&
               overflow));
  BOOST_CHECK((add(Int(7, 4, Signed), Int(7, 4, Signed), overflow) ==
                   Int(-2, 4, Signed) &&
               overflow));
  BOOST_CHECK((add(Int(-8, 4, Signed), Int(7, 4, Signed), overflow) ==
                   Int(-1, 4, Signed) &&
               !overflow));
  BOOST_CHECK((add(Int(0, 4, Signed), Int(-8, 4, Signed), overflow) ==
                   Int(-8, 4, Signed) &&
               !overflow));
  BOOST_CHECK((add(Int(1, 4, Signed), Int(-8, 4, Signed), overflow) ==
                   Int(-7, 4, Signed) &&
               !overflow));
  BOOST_CHECK((add(Int(7, 4, Signed), Int(-8, 4, Signed), overflow) ==
                   Int(-1, 4, Signed) &&
               !overflow));
  BOOST_CHECK((add(Int(-8, 4, Signed), Int(-8, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               overflow));
}

BOOST_AUTO_TEST_CASE(test_add_unsigned) {
  BOOST_CHECK(add(Int(0, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(add(Int(1, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(2, 4, Unsigned));
  BOOST_CHECK(add(Int(15, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(add(Int(0, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK(add(Int(1, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(add(Int(15, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(14, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_add_with_overflow_unsigned) {
  bool overflow = false;
  BOOST_CHECK((add(Int(0, 4, Unsigned), Int(1, 4, Unsigned), overflow) ==
                   Int(1, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((add(Int(1, 4, Unsigned), Int(1, 4, Unsigned), overflow) ==
                   Int(2, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((add(Int(15, 4, Unsigned), Int(1, 4, Unsigned), overflow) ==
                   Int(0, 4, Unsigned) &&
               overflow));
  BOOST_CHECK((add(Int(0, 4, Unsigned), Int(15, 4, Unsigned), overflow) ==
                   Int(15, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((add(Int(1, 4, Unsigned), Int(15, 4, Unsigned), overflow) ==
                   Int(0, 4, Unsigned) &&
               overflow));
  BOOST_CHECK((add(Int(15, 4, Unsigned), Int(15, 4, Unsigned), overflow) ==
                   Int(14, 4, Unsigned) &&
               overflow));
  BOOST_CHECK((add(Int(7, 4, Unsigned), Int(8, 4, Unsigned), overflow) ==
                   Int(15, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((add(Int(7, 4, Unsigned), Int(9, 4, Unsigned), overflow) ==
                   Int(0, 4, Unsigned) &&
               overflow));
}

BOOST_AUTO_TEST_CASE(test_sub_signed) {
  BOOST_CHECK(sub(Int(0, 4, Signed), Int(1, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK(sub(Int(1, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(sub(Int(7, 4, Signed), Int(1, 4, Signed)) == Int(6, 4, Signed));
  BOOST_CHECK(sub(Int(-8, 4, Signed), Int(1, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(sub(Int(0, 4, Signed), Int(-1, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(sub(Int(1, 4, Signed), Int(-1, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK(sub(Int(7, 4, Signed), Int(-1, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK(sub(Int(-8, 4, Signed), Int(-1, 4, Signed)) ==
              Int(-7, 4, Signed));
  BOOST_CHECK(sub(Int(0, 4, Signed), Int(7, 4, Signed)) == Int(-7, 4, Signed));
  BOOST_CHECK(sub(Int(1, 4, Signed), Int(7, 4, Signed)) == Int(-6, 4, Signed));
  BOOST_CHECK(sub(Int(7, 4, Signed), Int(7, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(sub(Int(-8, 4, Signed), Int(7, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(sub(Int(0, 4, Signed), Int(-8, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK(sub(Int(1, 4, Signed), Int(-8, 4, Signed)) == Int(-7, 4, Signed));
  BOOST_CHECK(sub(Int(7, 4, Signed), Int(-8, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK(sub(Int(-8, 4, Signed), Int(-8, 4, Signed)) == Int(0, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_sub_with_overflow_signed) {
  bool overflow = false;
  BOOST_CHECK((sub(Int(0, 4, Signed), Int(1, 4, Signed), overflow) ==
                   Int(-1, 4, Signed) &&
               !overflow));
  BOOST_CHECK((sub(Int(1, 4, Signed), Int(1, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               !overflow));
  BOOST_CHECK((sub(Int(7, 4, Signed), Int(1, 4, Signed), overflow) ==
                   Int(6, 4, Signed) &&
               !overflow));
  BOOST_CHECK((sub(Int(-8, 4, Signed), Int(1, 4, Signed), overflow) ==
                   Int(7, 4, Signed) &&
               overflow));
  BOOST_CHECK((sub(Int(0, 4, Signed), Int(-1, 4, Signed), overflow) ==
                   Int(1, 4, Signed) &&
               !overflow));
  BOOST_CHECK((sub(Int(1, 4, Signed), Int(-1, 4, Signed), overflow) ==
                   Int(2, 4, Signed) &&
               !overflow));
  BOOST_CHECK((sub(Int(7, 4, Signed), Int(-1, 4, Signed), overflow) ==
                   Int(-8, 4, Signed) &&
               overflow));
  BOOST_CHECK((sub(Int(-8, 4, Signed), Int(-1, 4, Signed), overflow) ==
                   Int(-7, 4, Signed) &&
               !overflow));
  BOOST_CHECK((sub(Int(0, 4, Signed), Int(7, 4, Signed), overflow) ==
                   Int(-7, 4, Signed) &&
               !overflow));
  BOOST_CHECK((sub(Int(1, 4, Signed), Int(7, 4, Signed), overflow) ==
                   Int(-6, 4, Signed) &&
               !overflow));
  BOOST_CHECK((sub(Int(7, 4, Signed), Int(7, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               !overflow));
  BOOST_CHECK((sub(Int(-8, 4, Signed), Int(7, 4, Signed), overflow) ==
                   Int(1, 4, Signed) &&
               overflow));
  BOOST_CHECK((sub(Int(0, 4, Signed), Int(-8, 4, Signed), overflow) ==
                   Int(-8, 4, Signed) &&
               overflow));
  BOOST_CHECK((sub(Int(1, 4, Signed), Int(-8, 4, Signed), overflow) ==
                   Int(-7, 4, Signed) &&
               overflow));
  BOOST_CHECK((sub(Int(7, 4, Signed), Int(-8, 4, Signed), overflow) ==
                   Int(-1, 4, Signed) &&
               overflow));
  BOOST_CHECK((sub(Int(-8, 4, Signed), Int(-8, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               !overflow));
}

BOOST_AUTO_TEST_CASE(test_sub_unsigned) {
  BOOST_CHECK(sub(Int(0, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK(sub(Int(1, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(sub(Int(15, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(14, 4, Unsigned));
  BOOST_CHECK(sub(Int(0, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(sub(Int(1, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(2, 4, Unsigned));
  BOOST_CHECK(sub(Int(15, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_sub_with_overflow_unsigned) {
  bool overflow = false;
  BOOST_CHECK((sub(Int(0, 4, Unsigned), Int(1, 4, Unsigned), overflow) ==
                   Int(15, 4, Unsigned) &&
               overflow));
  BOOST_CHECK((sub(Int(1, 4, Unsigned), Int(1, 4, Unsigned), overflow) ==
                   Int(0, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((sub(Int(15, 4, Unsigned), Int(1, 4, Unsigned), overflow) ==
                   Int(14, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((sub(Int(0, 4, Unsigned), Int(15, 4, Unsigned), overflow) ==
                   Int(1, 4, Unsigned) &&
               overflow));
  BOOST_CHECK((sub(Int(1, 4, Unsigned), Int(15, 4, Unsigned), overflow) ==
                   Int(2, 4, Unsigned) &&
               overflow));
  BOOST_CHECK((sub(Int(15, 4, Unsigned), Int(15, 4, Unsigned), overflow) ==
                   Int(0, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((sub(Int(3, 4, Unsigned), Int(2, 4, Unsigned), overflow) ==
                   Int(1, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((sub(Int(3, 4, Unsigned), Int(4, 4, Unsigned), overflow) ==
                   Int(15, 4, Unsigned) &&
               overflow));
}

BOOST_AUTO_TEST_CASE(test_mul_signed) {
  BOOST_CHECK(mul(Int(0, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mul(Int(1, 4, Signed), Int(1, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(mul(Int(7, 4, Signed), Int(1, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(mul(Int(-8, 4, Signed), Int(1, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK(mul(Int(0, 4, Signed), Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mul(Int(1, 4, Signed), Int(-1, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK(mul(Int(7, 4, Signed), Int(-1, 4, Signed)) == Int(-7, 4, Signed));
  BOOST_CHECK(mul(Int(-8, 4, Signed), Int(-1, 4, Signed)) ==
              Int(-8, 4, Signed));
  BOOST_CHECK(mul(Int(0, 4, Signed), Int(7, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mul(Int(1, 4, Signed), Int(7, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(mul(Int(7, 4, Signed), Int(7, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(mul(Int(-8, 4, Signed), Int(7, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK(mul(Int(0, 4, Signed), Int(-8, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mul(Int(1, 4, Signed), Int(-8, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK(mul(Int(7, 4, Signed), Int(-8, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK(mul(Int(-8, 4, Signed), Int(-8, 4, Signed)) == Int(0, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_mul_with_overflow_signed) {
  bool overflow = false;
  BOOST_CHECK((mul(Int(0, 4, Signed), Int(1, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               !overflow));
  BOOST_CHECK((mul(Int(1, 4, Signed), Int(1, 4, Signed), overflow) ==
                   Int(1, 4, Signed) &&
               !overflow));
  BOOST_CHECK((mul(Int(7, 4, Signed), Int(1, 4, Signed), overflow) ==
                   Int(7, 4, Signed) &&
               !overflow));
  BOOST_CHECK((mul(Int(-8, 4, Signed), Int(1, 4, Signed), overflow) ==
                   Int(-8, 4, Signed) &&
               !overflow));
  BOOST_CHECK((mul(Int(0, 4, Signed), Int(-1, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               !overflow));
  BOOST_CHECK((mul(Int(1, 4, Signed), Int(-1, 4, Signed), overflow) ==
                   Int(-1, 4, Signed) &&
               !overflow));
  BOOST_CHECK((mul(Int(7, 4, Signed), Int(-1, 4, Signed), overflow) ==
                   Int(-7, 4, Signed) &&
               !overflow));
  BOOST_CHECK((mul(Int(-8, 4, Signed), Int(-1, 4, Signed), overflow) ==
                   Int(-8, 4, Signed) &&
               overflow));
  BOOST_CHECK((mul(Int(0, 4, Signed), Int(7, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               !overflow));
  BOOST_CHECK((mul(Int(1, 4, Signed), Int(7, 4, Signed), overflow) ==
                   Int(7, 4, Signed) &&
               !overflow));
  BOOST_CHECK((mul(Int(7, 4, Signed), Int(7, 4, Signed), overflow) ==
                   Int(1, 4, Signed) &&
               overflow));
  BOOST_CHECK((mul(Int(-8, 4, Signed), Int(7, 4, Signed), overflow) ==
                   Int(-8, 4, Signed) &&
               overflow));
  BOOST_CHECK((mul(Int(0, 4, Signed), Int(-8, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               !overflow));
  BOOST_CHECK((mul(Int(1, 4, Signed), Int(-8, 4, Signed), overflow) ==
                   Int(-8, 4, Signed) &&
               !overflow));
  BOOST_CHECK((mul(Int(7, 4, Signed), Int(-8, 4, Signed), overflow) ==
                   Int(-8, 4, Signed) &&
               overflow));
  BOOST_CHECK((mul(Int(-8, 4, Signed), Int(-8, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               overflow));
  BOOST_CHECK((mul(Int(-1, 1, Signed), Int(-1, 1, Signed), overflow) ==
                   Int(-1, 1, Signed) &&
               overflow));
}

BOOST_AUTO_TEST_CASE(test_mul_unsigned) {
  BOOST_CHECK(mul(Int(0, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(mul(Int(1, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(mul(Int(15, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK(mul(Int(0, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(mul(Int(1, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK(mul(Int(15, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(mul(Int(3, 4, Unsigned), Int(4, 4, Unsigned)) ==
              Int(12, 4, Unsigned));
  BOOST_CHECK(mul(Int(4, 4, Unsigned), Int(4, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_mul_with_overflow_unsigned) {
  bool overflow = false;
  BOOST_CHECK((mul(Int(0, 4, Unsigned), Int(1, 4, Unsigned), overflow) ==
                   Int(0, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((mul(Int(1, 4, Unsigned), Int(1, 4, Unsigned), overflow) ==
                   Int(1, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((mul(Int(15, 4, Unsigned), Int(1, 4, Unsigned), overflow) ==
                   Int(15, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((mul(Int(0, 4, Unsigned), Int(15, 4, Unsigned), overflow) ==
                   Int(0, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((mul(Int(1, 4, Unsigned), Int(15, 4, Unsigned), overflow) ==
                   Int(15, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((mul(Int(15, 4, Unsigned), Int(15, 4, Unsigned), overflow) ==
                   Int(1, 4, Unsigned) &&
               overflow));
  BOOST_CHECK((mul(Int(3, 4, Unsigned), Int(4, 4, Unsigned), overflow) ==
                   Int(12, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((mul(Int(4, 4, Unsigned), Int(4, 4, Unsigned), overflow) ==
                   Int(0, 4, Unsigned) &&
               overflow));
}

BOOST_AUTO_TEST_CASE(test_div_signed) {
  BOOST_CHECK(div(Int(0, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(div(Int(1, 4, Signed), Int(1, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(div(Int(7, 4, Signed), Int(1, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(div(Int(-8, 4, Signed), Int(1, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK(div(Int(0, 4, Signed), Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(div(Int(1, 4, Signed), Int(-1, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK(div(Int(7, 4, Signed), Int(-1, 4, Signed)) == Int(-7, 4, Signed));
  BOOST_CHECK(div(Int(-8, 4, Signed), Int(-1, 4, Signed)) ==
              Int(-8, 4, Signed));
  BOOST_CHECK(div(Int(0, 4, Signed), Int(7, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(div(Int(1, 4, Signed), Int(7, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(div(Int(7, 4, Signed), Int(7, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(div(Int(-8, 4, Signed), Int(7, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK(div(Int(0, 4, Signed), Int(-8, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(div(Int(1, 4, Signed), Int(-8, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(div(Int(7, 4, Signed), Int(-8, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(div(Int(-8, 4, Signed), Int(-8, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(div(Int(4, 4, Signed), Int(2, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK(div(Int(5, 4, Signed), Int(2, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK(div(Int(-4, 4, Signed), Int(2, 4, Signed)) == Int(-2, 4, Signed));
  BOOST_CHECK(div(Int(-5, 4, Signed), Int(2, 4, Signed)) == Int(-2, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_div_with_overflow_and_exact_signed) {
  bool overflow = false;
  bool exact = false;
  BOOST_CHECK((div(Int(0, 4, Signed), Int(1, 4, Signed), overflow, exact) ==
                   Int(0, 4, Signed) &&
               !overflow && exact));
  BOOST_CHECK((div(Int(1, 4, Signed), Int(1, 4, Signed), overflow, exact) ==
                   Int(1, 4, Signed) &&
               !overflow && exact));
  BOOST_CHECK((div(Int(7, 4, Signed), Int(1, 4, Signed), overflow, exact) ==
                   Int(7, 4, Signed) &&
               !overflow && exact));
  BOOST_CHECK((div(Int(-8, 4, Signed), Int(1, 4, Signed), overflow, exact) ==
                   Int(-8, 4, Signed) &&
               !overflow && exact));
  BOOST_CHECK((div(Int(0, 4, Signed), Int(-1, 4, Signed), overflow, exact) ==
                   Int(0, 4, Signed) &&
               !overflow && exact));
  BOOST_CHECK((div(Int(1, 4, Signed), Int(-1, 4, Signed), overflow, exact) ==
                   Int(-1, 4, Signed) &&
               !overflow && exact));
  BOOST_CHECK((div(Int(7, 4, Signed), Int(-1, 4, Signed), overflow, exact) ==
                   Int(-7, 4, Signed) &&
               !overflow && exact));
  BOOST_CHECK((div(Int(-8, 4, Signed), Int(-1, 4, Signed), overflow, exact) ==
                   Int(-8, 4, Signed) &&
               overflow && exact));
  BOOST_CHECK((div(Int(0, 4, Signed), Int(7, 4, Signed), overflow, exact) ==
                   Int(0, 4, Signed) &&
               !overflow && exact));
  BOOST_CHECK((div(Int(1, 4, Signed), Int(7, 4, Signed), overflow, exact) ==
                   Int(0, 4, Signed) &&
               !overflow && !exact));
  BOOST_CHECK((div(Int(7, 4, Signed), Int(7, 4, Signed), overflow, exact) ==
                   Int(1, 4, Signed) &&
               !overflow && exact));
  BOOST_CHECK((div(Int(-8, 4, Signed), Int(7, 4, Signed), overflow, exact) ==
                   Int(-1, 4, Signed) &&
               !overflow && !exact));
  BOOST_CHECK((div(Int(0, 4, Signed), Int(-8, 4, Signed), overflow, exact) ==
                   Int(0, 4, Signed) &&
               !overflow && exact));
  BOOST_CHECK((div(Int(1, 4, Signed), Int(-8, 4, Signed), overflow, exact) ==
                   Int(0, 4, Signed) &&
               !overflow && !exact));
  BOOST_CHECK((div(Int(7, 4, Signed), Int(-8, 4, Signed), overflow, exact) ==
                   Int(0, 4, Signed) &&
               !overflow && !exact));
  BOOST_CHECK((div(Int(-8, 4, Signed), Int(-8, 4, Signed), overflow, exact) ==
                   Int(1, 4, Signed) &&
               !overflow && exact));
  BOOST_CHECK((div(Int(4, 4, Signed), Int(2, 4, Signed), overflow, exact) ==
                   Int(2, 4, Signed) &&
               !overflow && exact));
  BOOST_CHECK((div(Int(5, 4, Signed), Int(2, 4, Signed), overflow, exact) ==
                   Int(2, 4, Signed) &&
               !overflow && !exact));
  BOOST_CHECK((div(Int(-4, 4, Signed), Int(2, 4, Signed), overflow, exact) ==
                   Int(-2, 4, Signed) &&
               !overflow && exact));
  BOOST_CHECK((div(Int(-5, 4, Signed), Int(2, 4, Signed), overflow, exact) ==
                   Int(-2, 4, Signed) &&
               !overflow && !exact));
}

BOOST_AUTO_TEST_CASE(test_div_unsigned) {
  BOOST_CHECK(div(Int(0, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(div(Int(1, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(div(Int(15, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK(div(Int(0, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(div(Int(1, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(div(Int(15, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(div(Int(3, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(div(Int(4, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(2, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_div_with_overflow_and_exact_unsigned) {
  bool overflow = false;
  bool exact = false;
  BOOST_CHECK((div(Int(0, 4, Unsigned), Int(1, 4, Unsigned), overflow, exact) ==
                   Int(0, 4, Unsigned) &&
               !overflow && exact));
  BOOST_CHECK((div(Int(1, 4, Unsigned), Int(1, 4, Unsigned), overflow, exact) ==
                   Int(1, 4, Unsigned) &&
               !overflow && exact));
  BOOST_CHECK(
      (div(Int(15, 4, Unsigned), Int(1, 4, Unsigned), overflow, exact) ==
           Int(15, 4, Unsigned) &&
       !overflow && exact));
  BOOST_CHECK(
      (div(Int(0, 4, Unsigned), Int(15, 4, Unsigned), overflow, exact) ==
           Int(0, 4, Unsigned) &&
       !overflow && exact));
  BOOST_CHECK(
      (div(Int(1, 4, Unsigned), Int(15, 4, Unsigned), overflow, exact) ==
           Int(0, 4, Unsigned) &&
       !overflow && !exact));
  BOOST_CHECK(
      (div(Int(15, 4, Unsigned), Int(15, 4, Unsigned), overflow, exact) ==
           Int(1, 4, Unsigned) &&
       !overflow && exact));
  BOOST_CHECK((div(Int(3, 4, Unsigned), Int(2, 4, Unsigned), overflow, exact) ==
                   Int(1, 4, Unsigned) &&
               !overflow && !exact));
  BOOST_CHECK((div(Int(4, 4, Unsigned), Int(2, 4, Unsigned), overflow, exact) ==
                   Int(2, 4, Unsigned) &&
               !overflow && exact));
}

BOOST_AUTO_TEST_CASE(test_rem_signed) {
  BOOST_CHECK(rem(Int(0, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(rem(Int(1, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(rem(Int(7, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(rem(Int(-8, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(rem(Int(0, 4, Signed), Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(rem(Int(1, 4, Signed), Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(rem(Int(7, 4, Signed), Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(rem(Int(-8, 4, Signed), Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(rem(Int(0, 4, Signed), Int(7, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(rem(Int(1, 4, Signed), Int(7, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(rem(Int(7, 4, Signed), Int(7, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(rem(Int(-8, 4, Signed), Int(7, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK(rem(Int(0, 4, Signed), Int(-8, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(rem(Int(1, 4, Signed), Int(-8, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(rem(Int(7, 4, Signed), Int(-8, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(rem(Int(-8, 4, Signed), Int(-8, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(rem(Int(4, 4, Signed), Int(2, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(rem(Int(5, 4, Signed), Int(2, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(rem(Int(-4, 4, Signed), Int(2, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(rem(Int(-5, 4, Signed), Int(2, 4, Signed)) == Int(-1, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_rem_unsigned) {
  BOOST_CHECK(rem(Int(0, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(rem(Int(1, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(rem(Int(15, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(rem(Int(0, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(rem(Int(1, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(rem(Int(15, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(rem(Int(3, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(rem(Int(4, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_mod_signed) {
  BOOST_CHECK(mod(Int(0, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mod(Int(1, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mod(Int(7, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mod(Int(-8, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mod(Int(0, 4, Signed), Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mod(Int(1, 4, Signed), Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mod(Int(7, 4, Signed), Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mod(Int(-8, 4, Signed), Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mod(Int(0, 4, Signed), Int(7, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mod(Int(1, 4, Signed), Int(7, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(mod(Int(7, 4, Signed), Int(7, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mod(Int(-8, 4, Signed), Int(7, 4, Signed)) == Int(6, 4, Signed));
  BOOST_CHECK(mod(Int(0, 4, Signed), Int(-8, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mod(Int(1, 4, Signed), Int(-8, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(mod(Int(7, 4, Signed), Int(-8, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(mod(Int(-8, 4, Signed), Int(-8, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mod(Int(4, 4, Signed), Int(2, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mod(Int(5, 4, Signed), Int(2, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(mod(Int(-4, 4, Signed), Int(2, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(mod(Int(-5, 4, Signed), Int(2, 4, Signed)) == Int(1, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_mod_unsigned) {
  BOOST_CHECK(mod(Int(0, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(mod(Int(1, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(mod(Int(15, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(mod(Int(0, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(mod(Int(1, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(mod(Int(15, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(mod(Int(3, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(mod(Int(4, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_shl_signed) {
  BOOST_CHECK(shl(Int(0, 4, Signed), Int(0, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(shl(Int(1, 4, Signed), Int(0, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(shl(Int(7, 4, Signed), Int(0, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(shl(Int(-8, 4, Signed), Int(0, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK(shl(Int(0, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(shl(Int(1, 4, Signed), Int(1, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK(shl(Int(7, 4, Signed), Int(1, 4, Signed)) == Int(-2, 4, Signed));
  BOOST_CHECK(shl(Int(-8, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(shl(Int(0, 4, Signed), Int(2, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(shl(Int(1, 4, Signed), Int(2, 4, Signed)) == Int(4, 4, Signed));
  BOOST_CHECK(shl(Int(7, 4, Signed), Int(2, 4, Signed)) == Int(-4, 4, Signed));
  BOOST_CHECK(shl(Int(-8, 4, Signed), Int(2, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(shl(Int(4, 4, Signed), Int(2, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(shl(Int(5, 4, Signed), Int(2, 4, Signed)) == Int(4, 4, Signed));
  BOOST_CHECK(shl(Int(-4, 4, Signed), Int(2, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(shl(Int(-5, 4, Signed), Int(2, 4, Signed)) == Int(-4, 4, Signed));
  BOOST_CHECK(shl(Int(0, 4, Signed), Int(3, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(shl(Int(1, 4, Signed), Int(3, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK(shl(Int(7, 4, Signed), Int(3, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK(shl(Int(-8, 4, Signed), Int(3, 4, Signed)) == Int(0, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_shl_with_overflow_signed) {
  bool overflow = false;
  BOOST_CHECK((shl(Int(0, 4, Signed), Int(0, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               !overflow));
  BOOST_CHECK((shl(Int(1, 4, Signed), Int(0, 4, Signed), overflow) ==
                   Int(1, 4, Signed) &&
               !overflow));
  BOOST_CHECK((shl(Int(7, 4, Signed), Int(0, 4, Signed), overflow) ==
                   Int(7, 4, Signed) &&
               !overflow));
  BOOST_CHECK((shl(Int(-8, 4, Signed), Int(0, 4, Signed), overflow) ==
                   Int(-8, 4, Signed) &&
               !overflow));
  BOOST_CHECK((shl(Int(0, 4, Signed), Int(1, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               !overflow));
  BOOST_CHECK((shl(Int(1, 4, Signed), Int(1, 4, Signed), overflow) ==
                   Int(2, 4, Signed) &&
               !overflow));
  BOOST_CHECK((shl(Int(7, 4, Signed), Int(1, 4, Signed), overflow) ==
                   Int(-2, 4, Signed) &&
               overflow));
  BOOST_CHECK((shl(Int(-8, 4, Signed), Int(1, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               overflow));
  BOOST_CHECK((shl(Int(0, 4, Signed), Int(2, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               !overflow));
  BOOST_CHECK((shl(Int(1, 4, Signed), Int(2, 4, Signed), overflow) ==
                   Int(4, 4, Signed) &&
               !overflow));
  BOOST_CHECK((shl(Int(7, 4, Signed), Int(2, 4, Signed), overflow) ==
                   Int(-4, 4, Signed) &&
               overflow));
  BOOST_CHECK((shl(Int(-8, 4, Signed), Int(2, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               overflow));
  BOOST_CHECK((shl(Int(4, 4, Signed), Int(2, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               overflow));
  BOOST_CHECK((shl(Int(5, 4, Signed), Int(2, 4, Signed), overflow) ==
                   Int(4, 4, Signed) &&
               overflow));
  BOOST_CHECK((shl(Int(-4, 4, Signed), Int(2, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               overflow));
  BOOST_CHECK((shl(Int(-5, 4, Signed), Int(2, 4, Signed), overflow) ==
                   Int(-4, 4, Signed) &&
               overflow));
  BOOST_CHECK((shl(Int(0, 4, Signed), Int(3, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               !overflow));
  BOOST_CHECK((shl(Int(1, 4, Signed), Int(3, 4, Signed), overflow) ==
                   Int(-8, 4, Signed) &&
               overflow));
  BOOST_CHECK((shl(Int(7, 4, Signed), Int(3, 4, Signed), overflow) ==
                   Int(-8, 4, Signed) &&
               overflow));
  BOOST_CHECK((shl(Int(-8, 4, Signed), Int(3, 4, Signed), overflow) ==
                   Int(0, 4, Signed) &&
               overflow));
}

BOOST_AUTO_TEST_CASE(test_shl_unsigned) {
  BOOST_CHECK(shl(Int(0, 4, Unsigned), Int(0, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(shl(Int(1, 4, Unsigned), Int(0, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(shl(Int(14, 4, Unsigned), Int(0, 4, Unsigned)) ==
              Int(14, 4, Unsigned));
  BOOST_CHECK(shl(Int(15, 4, Unsigned), Int(0, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK(shl(Int(0, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(shl(Int(1, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(2, 4, Unsigned));
  BOOST_CHECK(shl(Int(14, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(12, 4, Unsigned));
  BOOST_CHECK(shl(Int(15, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(14, 4, Unsigned));
  BOOST_CHECK(shl(Int(0, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(shl(Int(1, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(4, 4, Unsigned));
  BOOST_CHECK(shl(Int(14, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(8, 4, Unsigned));
  BOOST_CHECK(shl(Int(15, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(12, 4, Unsigned));
  BOOST_CHECK(shl(Int(0, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(shl(Int(1, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(8, 4, Unsigned));
  BOOST_CHECK(shl(Int(14, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(shl(Int(15, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(8, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_shl_with_overflow_unsigned) {
  bool overflow = false;
  BOOST_CHECK((shl(Int(0, 4, Unsigned), Int(0, 4, Unsigned), overflow) ==
                   Int(0, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((shl(Int(1, 4, Unsigned), Int(0, 4, Unsigned), overflow) ==
                   Int(1, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((shl(Int(14, 4, Unsigned), Int(0, 4, Unsigned), overflow) ==
                   Int(14, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((shl(Int(15, 4, Unsigned), Int(0, 4, Unsigned), overflow) ==
                   Int(15, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((shl(Int(0, 4, Unsigned), Int(1, 4, Unsigned), overflow) ==
                   Int(0, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((shl(Int(1, 4, Unsigned), Int(1, 4, Unsigned), overflow) ==
                   Int(2, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((shl(Int(14, 4, Unsigned), Int(1, 4, Unsigned), overflow) ==
                   Int(12, 4, Unsigned) &&
               overflow));
  BOOST_CHECK((shl(Int(15, 4, Unsigned), Int(1, 4, Unsigned), overflow) ==
                   Int(14, 4, Unsigned) &&
               overflow));
  BOOST_CHECK((shl(Int(0, 4, Unsigned), Int(2, 4, Unsigned), overflow) ==
                   Int(0, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((shl(Int(1, 4, Unsigned), Int(2, 4, Unsigned), overflow) ==
                   Int(4, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((shl(Int(14, 4, Unsigned), Int(2, 4, Unsigned), overflow) ==
                   Int(8, 4, Unsigned) &&
               overflow));
  BOOST_CHECK((shl(Int(15, 4, Unsigned), Int(2, 4, Unsigned), overflow) ==
                   Int(12, 4, Unsigned) &&
               overflow));
  BOOST_CHECK((shl(Int(0, 4, Unsigned), Int(3, 4, Unsigned), overflow) ==
                   Int(0, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((shl(Int(1, 4, Unsigned), Int(3, 4, Unsigned), overflow) ==
                   Int(8, 4, Unsigned) &&
               !overflow));
  BOOST_CHECK((shl(Int(14, 4, Unsigned), Int(3, 4, Unsigned), overflow) ==
                   Int(0, 4, Unsigned) &&
               overflow));
  BOOST_CHECK((shl(Int(15, 4, Unsigned), Int(3, 4, Unsigned), overflow) ==
                   Int(8, 4, Unsigned) &&
               overflow));
}

BOOST_AUTO_TEST_CASE(test_lshr_signed) {
  BOOST_CHECK(lshr(Int(0, 4, Signed), Int(0, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(lshr(Int(1, 4, Signed), Int(0, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(lshr(Int(7, 4, Signed), Int(0, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(lshr(Int(-8, 4, Signed), Int(0, 4, Signed)) ==
              Int(-8, 4, Signed));
  BOOST_CHECK(lshr(Int(0, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(lshr(Int(1, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(lshr(Int(7, 4, Signed), Int(1, 4, Signed)) == Int(3, 4, Signed));
  BOOST_CHECK(lshr(Int(-8, 4, Signed), Int(1, 4, Signed)) == Int(4, 4, Signed));
  BOOST_CHECK(lshr(Int(0, 4, Signed), Int(2, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(lshr(Int(1, 4, Signed), Int(2, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(lshr(Int(7, 4, Signed), Int(2, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(lshr(Int(-8, 4, Signed), Int(2, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK(lshr(Int(4, 4, Signed), Int(2, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(lshr(Int(5, 4, Signed), Int(2, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(lshr(Int(-4, 4, Signed), Int(2, 4, Signed)) == Int(3, 4, Signed));
  BOOST_CHECK(lshr(Int(-5, 4, Signed), Int(2, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK(lshr(Int(0, 4, Signed), Int(3, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(lshr(Int(1, 4, Signed), Int(3, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(lshr(Int(7, 4, Signed), Int(3, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(lshr(Int(-8, 4, Signed), Int(3, 4, Signed)) == Int(1, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_lshr_with_exact_signed) {
  bool exact = true;
  BOOST_CHECK(
      (lshr(Int(0, 4, Signed), Int(0, 4, Signed), exact) == Int(0, 4, Signed) &&
       exact));
  BOOST_CHECK(
      (lshr(Int(1, 4, Signed), Int(0, 4, Signed), exact) == Int(1, 4, Signed) &&
       exact));
  BOOST_CHECK(
      (lshr(Int(7, 4, Signed), Int(0, 4, Signed), exact) == Int(7, 4, Signed) &&
       exact));
  BOOST_CHECK((lshr(Int(-8, 4, Signed), Int(0, 4, Signed), exact) ==
                   Int(-8, 4, Signed) &&
               exact));
  BOOST_CHECK(
      (lshr(Int(0, 4, Signed), Int(1, 4, Signed), exact) == Int(0, 4, Signed) &&
       exact));
  BOOST_CHECK(
      (lshr(Int(1, 4, Signed), Int(1, 4, Signed), exact) == Int(0, 4, Signed) &&
       !exact));
  BOOST_CHECK(
      (lshr(Int(7, 4, Signed), Int(1, 4, Signed), exact) == Int(3, 4, Signed) &&
       !exact));
  BOOST_CHECK((lshr(Int(-8, 4, Signed), Int(1, 4, Signed), exact) ==
                   Int(4, 4, Signed) &&
               exact));
  BOOST_CHECK(
      (lshr(Int(0, 4, Signed), Int(2, 4, Signed), exact) == Int(0, 4, Signed) &&
       exact));
  BOOST_CHECK(
      (lshr(Int(1, 4, Signed), Int(2, 4, Signed), exact) == Int(0, 4, Signed) &&
       !exact));
  BOOST_CHECK(
      (lshr(Int(7, 4, Signed), Int(2, 4, Signed), exact) == Int(1, 4, Signed) &&
       !exact));
  BOOST_CHECK((lshr(Int(-8, 4, Signed), Int(2, 4, Signed), exact) ==
                   Int(2, 4, Signed) &&
               exact));
  BOOST_CHECK(
      (lshr(Int(4, 4, Signed), Int(2, 4, Signed), exact) == Int(1, 4, Signed) &&
       exact));
  BOOST_CHECK(
      (lshr(Int(5, 4, Signed), Int(2, 4, Signed), exact) == Int(1, 4, Signed) &&
       !exact));
  BOOST_CHECK((lshr(Int(-4, 4, Signed), Int(2, 4, Signed), exact) ==
                   Int(3, 4, Signed) &&
               exact));
  BOOST_CHECK((lshr(Int(-5, 4, Signed), Int(2, 4, Signed), exact) ==
                   Int(2, 4, Signed) &&
               !exact));
  BOOST_CHECK(
      (lshr(Int(0, 4, Signed), Int(3, 4, Signed), exact) == Int(0, 4, Signed) &&
       exact));
  BOOST_CHECK(
      (lshr(Int(1, 4, Signed), Int(3, 4, Signed), exact) == Int(0, 4, Signed) &&
       !exact));
  BOOST_CHECK(
      (lshr(Int(7, 4, Signed), Int(3, 4, Signed), exact) == Int(0, 4, Signed) &&
       !exact));
  BOOST_CHECK((lshr(Int(-8, 4, Signed), Int(3, 4, Signed), exact) ==
                   Int(1, 4, Signed) &&
               exact));
}

BOOST_AUTO_TEST_CASE(test_lshr_unsigned) {
  BOOST_CHECK(lshr(Int(0, 4, Unsigned), Int(0, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(lshr(Int(1, 4, Unsigned), Int(0, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(lshr(Int(14, 4, Unsigned), Int(0, 4, Unsigned)) ==
              Int(14, 4, Unsigned));
  BOOST_CHECK(lshr(Int(15, 4, Unsigned), Int(0, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK(lshr(Int(0, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(lshr(Int(1, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(lshr(Int(14, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(7, 4, Unsigned));
  BOOST_CHECK(lshr(Int(15, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(7, 4, Unsigned));
  BOOST_CHECK(lshr(Int(0, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(lshr(Int(1, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(lshr(Int(14, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(3, 4, Unsigned));
  BOOST_CHECK(lshr(Int(15, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(3, 4, Unsigned));
  BOOST_CHECK(lshr(Int(0, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(lshr(Int(1, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(lshr(Int(14, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(lshr(Int(15, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_lshr_with_exact_unsigned) {
  bool exact = true;
  BOOST_CHECK((lshr(Int(0, 4, Unsigned), Int(0, 4, Unsigned), exact) ==
                   Int(0, 4, Unsigned) &&
               exact));
  BOOST_CHECK((lshr(Int(1, 4, Unsigned), Int(0, 4, Unsigned), exact) ==
                   Int(1, 4, Unsigned) &&
               exact));
  BOOST_CHECK((lshr(Int(14, 4, Unsigned), Int(0, 4, Unsigned), exact) ==
                   Int(14, 4, Unsigned) &&
               exact));
  BOOST_CHECK((lshr(Int(15, 4, Unsigned), Int(0, 4, Unsigned), exact) ==
                   Int(15, 4, Unsigned) &&
               exact));
  BOOST_CHECK((lshr(Int(0, 4, Unsigned), Int(1, 4, Unsigned), exact) ==
                   Int(0, 4, Unsigned) &&
               exact));
  BOOST_CHECK((lshr(Int(1, 4, Unsigned), Int(1, 4, Unsigned), exact) ==
                   Int(0, 4, Unsigned) &&
               !exact));
  BOOST_CHECK((lshr(Int(14, 4, Unsigned), Int(1, 4, Unsigned), exact) ==
                   Int(7, 4, Unsigned) &&
               exact));
  BOOST_CHECK((lshr(Int(15, 4, Unsigned), Int(1, 4, Unsigned), exact) ==
                   Int(7, 4, Unsigned) &&
               !exact));
  BOOST_CHECK((lshr(Int(0, 4, Unsigned), Int(2, 4, Unsigned), exact) ==
                   Int(0, 4, Unsigned) &&
               exact));
  BOOST_CHECK((lshr(Int(1, 4, Unsigned), Int(2, 4, Unsigned), exact) ==
                   Int(0, 4, Unsigned) &&
               !exact));
  BOOST_CHECK((lshr(Int(14, 4, Unsigned), Int(2, 4, Unsigned), exact) ==
                   Int(3, 4, Unsigned) &&
               !exact));
  BOOST_CHECK((lshr(Int(15, 4, Unsigned), Int(2, 4, Unsigned), exact) ==
                   Int(3, 4, Unsigned) &&
               !exact));
  BOOST_CHECK((lshr(Int(0, 4, Unsigned), Int(3, 4, Unsigned), exact) ==
                   Int(0, 4, Unsigned) &&
               exact));
  BOOST_CHECK((lshr(Int(1, 4, Unsigned), Int(3, 4, Unsigned), exact) ==
                   Int(0, 4, Unsigned) &&
               !exact));
  BOOST_CHECK((lshr(Int(14, 4, Unsigned), Int(3, 4, Unsigned), exact) ==
                   Int(1, 4, Unsigned) &&
               !exact));
  BOOST_CHECK((lshr(Int(15, 4, Unsigned), Int(3, 4, Unsigned), exact) ==
                   Int(1, 4, Unsigned) &&
               !exact));
}

BOOST_AUTO_TEST_CASE(test_ashr_signed) {
  BOOST_CHECK(ashr(Int(0, 4, Signed), Int(0, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(ashr(Int(1, 4, Signed), Int(0, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(ashr(Int(7, 4, Signed), Int(0, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(ashr(Int(-8, 4, Signed), Int(0, 4, Signed)) ==
              Int(-8, 4, Signed));
  BOOST_CHECK(ashr(Int(0, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(ashr(Int(1, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(ashr(Int(7, 4, Signed), Int(1, 4, Signed)) == Int(3, 4, Signed));
  BOOST_CHECK(ashr(Int(-8, 4, Signed), Int(1, 4, Signed)) ==
              Int(-4, 4, Signed));
  BOOST_CHECK(ashr(Int(0, 4, Signed), Int(2, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(ashr(Int(1, 4, Signed), Int(2, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(ashr(Int(7, 4, Signed), Int(2, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(ashr(Int(-8, 4, Signed), Int(2, 4, Signed)) ==
              Int(-2, 4, Signed));
  BOOST_CHECK(ashr(Int(4, 4, Signed), Int(2, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(ashr(Int(5, 4, Signed), Int(2, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(ashr(Int(-4, 4, Signed), Int(2, 4, Signed)) ==
              Int(-1, 4, Signed));
  BOOST_CHECK(ashr(Int(-5, 4, Signed), Int(2, 4, Signed)) ==
              Int(-2, 4, Signed));
  BOOST_CHECK(ashr(Int(0, 4, Signed), Int(3, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(ashr(Int(1, 4, Signed), Int(3, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(ashr(Int(7, 4, Signed), Int(3, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(ashr(Int(-8, 4, Signed), Int(3, 4, Signed)) ==
              Int(-1, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_ashr_with_exact_signed) {
  bool exact = true;
  BOOST_CHECK(
      (ashr(Int(0, 4, Signed), Int(0, 4, Signed), exact) == Int(0, 4, Signed) &&
       exact));
  BOOST_CHECK(
      (ashr(Int(1, 4, Signed), Int(0, 4, Signed), exact) == Int(1, 4, Signed) &&
       exact));
  BOOST_CHECK(
      (ashr(Int(7, 4, Signed), Int(0, 4, Signed), exact) == Int(7, 4, Signed) &&
       exact));
  BOOST_CHECK((ashr(Int(-8, 4, Signed), Int(0, 4, Signed), exact) ==
                   Int(-8, 4, Signed) &&
               exact));
  BOOST_CHECK(
      (ashr(Int(0, 4, Signed), Int(1, 4, Signed), exact) == Int(0, 4, Signed) &&
       exact));
  BOOST_CHECK(
      (ashr(Int(1, 4, Signed), Int(1, 4, Signed), exact) == Int(0, 4, Signed) &&
       !exact));
  BOOST_CHECK(
      (ashr(Int(7, 4, Signed), Int(1, 4, Signed), exact) == Int(3, 4, Signed) &&
       !exact));
  BOOST_CHECK((ashr(Int(-8, 4, Signed), Int(1, 4, Signed), exact) ==
                   Int(-4, 4, Signed) &&
               exact));
  BOOST_CHECK(
      (ashr(Int(0, 4, Signed), Int(2, 4, Signed), exact) == Int(0, 4, Signed) &&
       exact));
  BOOST_CHECK(
      (ashr(Int(1, 4, Signed), Int(2, 4, Signed), exact) == Int(0, 4, Signed) &&
       !exact));
  BOOST_CHECK(
      (ashr(Int(7, 4, Signed), Int(2, 4, Signed), exact) == Int(1, 4, Signed) &&
       !exact));
  BOOST_CHECK((ashr(Int(-8, 4, Signed), Int(2, 4, Signed), exact) ==
                   Int(-2, 4, Signed) &&
               exact));
  BOOST_CHECK(
      (ashr(Int(4, 4, Signed), Int(2, 4, Signed), exact) == Int(1, 4, Signed) &&
       exact));
  BOOST_CHECK(
      (ashr(Int(5, 4, Signed), Int(2, 4, Signed), exact) == Int(1, 4, Signed) &&
       !exact));
  BOOST_CHECK((ashr(Int(-4, 4, Signed), Int(2, 4, Signed), exact) ==
                   Int(-1, 4, Signed) &&
               exact));
  BOOST_CHECK((ashr(Int(-5, 4, Signed), Int(2, 4, Signed), exact) ==
                   Int(-2, 4, Signed) &&
               !exact));
  BOOST_CHECK(
      (ashr(Int(0, 4, Signed), Int(3, 4, Signed), exact) == Int(0, 4, Signed) &&
       exact));
  BOOST_CHECK(
      (ashr(Int(1, 4, Signed), Int(3, 4, Signed), exact) == Int(0, 4, Signed) &&
       !exact));
  BOOST_CHECK(
      (ashr(Int(7, 4, Signed), Int(3, 4, Signed), exact) == Int(0, 4, Signed) &&
       !exact));
  BOOST_CHECK((ashr(Int(-8, 4, Signed), Int(3, 4, Signed), exact) ==
                   Int(-1, 4, Signed) &&
               exact));
}

BOOST_AUTO_TEST_CASE(test_ashr_unsigned) {
  BOOST_CHECK(ashr(Int(0, 4, Unsigned), Int(0, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(ashr(Int(1, 4, Unsigned), Int(0, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(ashr(Int(14, 4, Unsigned), Int(0, 4, Unsigned)) ==
              Int(14, 4, Unsigned));
  BOOST_CHECK(ashr(Int(15, 4, Unsigned), Int(0, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK(ashr(Int(0, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(ashr(Int(1, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(ashr(Int(14, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK(ashr(Int(15, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK(ashr(Int(0, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(ashr(Int(1, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(ashr(Int(14, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK(ashr(Int(15, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK(ashr(Int(0, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(ashr(Int(1, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(ashr(Int(14, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK(ashr(Int(15, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_ashr_with_exact_unsigned) {
  bool exact = true;
  BOOST_CHECK((ashr(Int(0, 4, Unsigned), Int(0, 4, Unsigned), exact) ==
                   Int(0, 4, Unsigned) &&
               exact));
  BOOST_CHECK((ashr(Int(1, 4, Unsigned), Int(0, 4, Unsigned), exact) ==
                   Int(1, 4, Unsigned) &&
               exact));
  BOOST_CHECK((ashr(Int(14, 4, Unsigned), Int(0, 4, Unsigned), exact) ==
                   Int(14, 4, Unsigned) &&
               exact));
  BOOST_CHECK((ashr(Int(15, 4, Unsigned), Int(0, 4, Unsigned), exact) ==
                   Int(15, 4, Unsigned) &&
               exact));
  BOOST_CHECK((ashr(Int(0, 4, Unsigned), Int(1, 4, Unsigned), exact) ==
                   Int(0, 4, Unsigned) &&
               exact));
  BOOST_CHECK((ashr(Int(1, 4, Unsigned), Int(1, 4, Unsigned), exact) ==
                   Int(0, 4, Unsigned) &&
               !exact));
  BOOST_CHECK((ashr(Int(14, 4, Unsigned), Int(1, 4, Unsigned), exact) ==
                   Int(15, 4, Unsigned) &&
               exact));
  BOOST_CHECK((ashr(Int(15, 4, Unsigned), Int(1, 4, Unsigned), exact) ==
                   Int(15, 4, Unsigned) &&
               !exact));
  BOOST_CHECK((ashr(Int(0, 4, Unsigned), Int(2, 4, Unsigned), exact) ==
                   Int(0, 4, Unsigned) &&
               exact));
  BOOST_CHECK((ashr(Int(1, 4, Unsigned), Int(2, 4, Unsigned), exact) ==
                   Int(0, 4, Unsigned) &&
               !exact));
  BOOST_CHECK((ashr(Int(14, 4, Unsigned), Int(2, 4, Unsigned), exact) ==
                   Int(15, 4, Unsigned) &&
               !exact));
  BOOST_CHECK((ashr(Int(15, 4, Unsigned), Int(2, 4, Unsigned), exact) ==
                   Int(15, 4, Unsigned) &&
               !exact));
  BOOST_CHECK((ashr(Int(0, 4, Unsigned), Int(3, 4, Unsigned), exact) ==
                   Int(0, 4, Unsigned) &&
               exact));
  BOOST_CHECK((ashr(Int(1, 4, Unsigned), Int(3, 4, Unsigned), exact) ==
                   Int(0, 4, Unsigned) &&
               !exact));
  BOOST_CHECK((ashr(Int(14, 4, Unsigned), Int(3, 4, Unsigned), exact) ==
                   Int(15, 4, Unsigned) &&
               !exact));
  BOOST_CHECK((ashr(Int(15, 4, Unsigned), Int(3, 4, Unsigned), exact) ==
                   Int(15, 4, Unsigned) &&
               !exact));
}

BOOST_AUTO_TEST_CASE(test_and_signed) {
  BOOST_CHECK(and_(Int(0, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(and_(Int(1, 4, Signed), Int(1, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(and_(Int(7, 4, Signed), Int(1, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(and_(Int(-8, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(and_(Int(0, 4, Signed), Int(-1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(and_(Int(1, 4, Signed), Int(-1, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(and_(Int(7, 4, Signed), Int(-1, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(and_(Int(-8, 4, Signed), Int(-1, 4, Signed)) ==
              Int(-8, 4, Signed));
  BOOST_CHECK(and_(Int(0, 4, Signed), Int(7, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(and_(Int(1, 4, Signed), Int(7, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(and_(Int(7, 4, Signed), Int(7, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(and_(Int(-8, 4, Signed), Int(7, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(and_(Int(0, 4, Signed), Int(-8, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(and_(Int(1, 4, Signed), Int(-8, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(and_(Int(7, 4, Signed), Int(-8, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(and_(Int(-8, 4, Signed), Int(-8, 4, Signed)) ==
              Int(-8, 4, Signed));
  BOOST_CHECK(and_(Int(4, 4, Signed), Int(2, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(and_(Int(5, 4, Signed), Int(2, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(and_(Int(-4, 4, Signed), Int(2, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(and_(Int(-5, 4, Signed), Int(2, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK(and_(Int(-7, 4, Signed), Int(-3, 4, Signed)) ==
              Int(-7, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_and_unsigned) {
  BOOST_CHECK(and_(Int(0, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(and_(Int(1, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(and_(Int(15, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(and_(Int(0, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(and_(Int(1, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(and_(Int(15, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK(and_(Int(3, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(2, 4, Unsigned));
  BOOST_CHECK(and_(Int(4, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(and_(Int(10, 4, Unsigned), Int(5, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(and_(Int(10, 4, Unsigned), Int(8, 4, Unsigned)) ==
              Int(8, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_or_signed) {
  BOOST_CHECK(or_(Int(0, 4, Signed), Int(1, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(or_(Int(1, 4, Signed), Int(1, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(or_(Int(7, 4, Signed), Int(1, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(or_(Int(-8, 4, Signed), Int(1, 4, Signed)) == Int(-7, 4, Signed));
  BOOST_CHECK(or_(Int(0, 4, Signed), Int(-1, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK(or_(Int(1, 4, Signed), Int(-1, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK(or_(Int(7, 4, Signed), Int(-1, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK(or_(Int(-8, 4, Signed), Int(-1, 4, Signed)) ==
              Int(-1, 4, Signed));
  BOOST_CHECK(or_(Int(0, 4, Signed), Int(7, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(or_(Int(1, 4, Signed), Int(7, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(or_(Int(7, 4, Signed), Int(7, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(or_(Int(-8, 4, Signed), Int(7, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK(or_(Int(0, 4, Signed), Int(-8, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK(or_(Int(1, 4, Signed), Int(-8, 4, Signed)) == Int(-7, 4, Signed));
  BOOST_CHECK(or_(Int(7, 4, Signed), Int(-8, 4, Signed)) == Int(-1, 4, Signed));
  BOOST_CHECK(or_(Int(-8, 4, Signed), Int(-8, 4, Signed)) ==
              Int(-8, 4, Signed));
  BOOST_CHECK(or_(Int(4, 4, Signed), Int(2, 4, Signed)) == Int(6, 4, Signed));
  BOOST_CHECK(or_(Int(5, 4, Signed), Int(2, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(or_(Int(-4, 4, Signed), Int(2, 4, Signed)) == Int(-2, 4, Signed));
  BOOST_CHECK(or_(Int(-5, 4, Signed), Int(2, 4, Signed)) == Int(-5, 4, Signed));
  BOOST_CHECK(or_(Int(-7, 4, Signed), Int(-3, 4, Signed)) ==
              Int(-3, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_xor_signed) {
  BOOST_CHECK(xor_(Int(0, 4, Signed), Int(1, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(xor_(Int(1, 4, Signed), Int(1, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(xor_(Int(7, 4, Signed), Int(1, 4, Signed)) == Int(6, 4, Signed));
  BOOST_CHECK(xor_(Int(-8, 4, Signed), Int(1, 4, Signed)) ==
              Int(-7, 4, Signed));
  BOOST_CHECK(xor_(Int(0, 4, Signed), Int(-1, 4, Signed)) ==
              Int(-1, 4, Signed));
  BOOST_CHECK(xor_(Int(1, 4, Signed), Int(-1, 4, Signed)) ==
              Int(-2, 4, Signed));
  BOOST_CHECK(xor_(Int(7, 4, Signed), Int(-1, 4, Signed)) ==
              Int(-8, 4, Signed));
  BOOST_CHECK(xor_(Int(-8, 4, Signed), Int(-1, 4, Signed)) ==
              Int(7, 4, Signed));
  BOOST_CHECK(xor_(Int(0, 4, Signed), Int(7, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(xor_(Int(1, 4, Signed), Int(7, 4, Signed)) == Int(6, 4, Signed));
  BOOST_CHECK(xor_(Int(7, 4, Signed), Int(7, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(xor_(Int(-8, 4, Signed), Int(7, 4, Signed)) ==
              Int(-1, 4, Signed));
  BOOST_CHECK(xor_(Int(0, 4, Signed), Int(-8, 4, Signed)) ==
              Int(-8, 4, Signed));
  BOOST_CHECK(xor_(Int(1, 4, Signed), Int(-8, 4, Signed)) ==
              Int(-7, 4, Signed));
  BOOST_CHECK(xor_(Int(7, 4, Signed), Int(-8, 4, Signed)) ==
              Int(-1, 4, Signed));
  BOOST_CHECK(xor_(Int(-8, 4, Signed), Int(-8, 4, Signed)) ==
              Int(0, 4, Signed));
  BOOST_CHECK(xor_(Int(4, 4, Signed), Int(2, 4, Signed)) == Int(6, 4, Signed));
  BOOST_CHECK(xor_(Int(5, 4, Signed), Int(2, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(xor_(Int(-4, 4, Signed), Int(2, 4, Signed)) ==
              Int(-2, 4, Signed));
  BOOST_CHECK(xor_(Int(-5, 4, Signed), Int(2, 4, Signed)) ==
              Int(-7, 4, Signed));
  BOOST_CHECK(xor_(Int(-7, 4, Signed), Int(-3, 4, Signed)) ==
              Int(4, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_xor_unsigned) {
  BOOST_CHECK(xor_(Int(0, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(xor_(Int(1, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(xor_(Int(15, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(14, 4, Unsigned));
  BOOST_CHECK(xor_(Int(0, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK(xor_(Int(1, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(14, 4, Unsigned));
  BOOST_CHECK(xor_(Int(15, 4, Unsigned), Int(15, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(xor_(Int(3, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(xor_(Int(4, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(6, 4, Unsigned));
  BOOST_CHECK(xor_(Int(10, 4, Unsigned), Int(5, 4, Unsigned)) ==
              Int(15, 4, Unsigned));
  BOOST_CHECK(xor_(Int(10, 4, Unsigned), Int(8, 4, Unsigned)) ==
              Int(2, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_gcd_signed) {
  BOOST_CHECK(gcd(Int(0, 4, Signed), Int(0, 4, Signed)) == Int(0, 4, Signed));
  BOOST_CHECK(gcd(Int(1, 4, Signed), Int(0, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(7, 4, Signed), Int(0, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(gcd(Int(-8, 4, Signed), Int(0, 4, Signed)) == Int(-8, 4, Signed));

  BOOST_CHECK(gcd(Int(0, 4, Signed), Int(1, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(1, 4, Signed), Int(1, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(7, 4, Signed), Int(1, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(-8, 4, Signed), Int(1, 4, Signed)) == Int(1, 4, Signed));

  BOOST_CHECK(gcd(Int(0, 4, Signed), Int(2, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK(gcd(Int(1, 4, Signed), Int(2, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(7, 4, Signed), Int(2, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(-8, 4, Signed), Int(2, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK(gcd(Int(4, 4, Signed), Int(2, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK(gcd(Int(5, 4, Signed), Int(2, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(-4, 4, Signed), Int(2, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK(gcd(Int(-5, 4, Signed), Int(2, 4, Signed)) == Int(1, 4, Signed));

  BOOST_CHECK(gcd(Int(0, 4, Signed), Int(3, 4, Signed)) == Int(3, 4, Signed));
  BOOST_CHECK(gcd(Int(1, 4, Signed), Int(3, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(7, 4, Signed), Int(3, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(-8, 4, Signed), Int(3, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(4, 4, Signed), Int(3, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(5, 4, Signed), Int(3, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(6, 4, Signed), Int(3, 4, Signed)) == Int(3, 4, Signed));
  BOOST_CHECK(gcd(Int(-4, 4, Signed), Int(3, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(-5, 4, Signed), Int(3, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(-6, 4, Signed), Int(3, 4, Signed)) == Int(3, 4, Signed));

  BOOST_CHECK(gcd(Int(0, 4, Signed), Int(7, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(gcd(Int(1, 4, Signed), Int(7, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(7, 4, Signed), Int(7, 4, Signed)) == Int(7, 4, Signed));
  BOOST_CHECK(gcd(Int(-8, 4, Signed), Int(7, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(4, 4, Signed), Int(7, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(5, 4, Signed), Int(7, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(6, 4, Signed), Int(7, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(-4, 4, Signed), Int(7, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(-5, 4, Signed), Int(7, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(-6, 4, Signed), Int(7, 4, Signed)) == Int(1, 4, Signed));

  BOOST_CHECK(gcd(Int(0, 4, Signed), Int(-8, 4, Signed)) == Int(-8, 4, Signed));
  BOOST_CHECK(gcd(Int(1, 4, Signed), Int(-8, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(7, 4, Signed), Int(-8, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(-8, 4, Signed), Int(-8, 4, Signed)) ==
              Int(-8, 4, Signed));
  BOOST_CHECK(gcd(Int(4, 4, Signed), Int(-8, 4, Signed)) == Int(4, 4, Signed));
  BOOST_CHECK(gcd(Int(5, 4, Signed), Int(-8, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(6, 4, Signed), Int(-8, 4, Signed)) == Int(2, 4, Signed));
  BOOST_CHECK(gcd(Int(-4, 4, Signed), Int(-8, 4, Signed)) == Int(4, 4, Signed));
  BOOST_CHECK(gcd(Int(-5, 4, Signed), Int(-8, 4, Signed)) == Int(1, 4, Signed));
  BOOST_CHECK(gcd(Int(-6, 4, Signed), Int(-8, 4, Signed)) == Int(2, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_gcd_unsigned) {
  BOOST_CHECK(gcd(Int(0, 4, Unsigned), Int(0, 4, Unsigned)) ==
              Int(0, 4, Unsigned));
  BOOST_CHECK(gcd(Int(1, 4, Unsigned), Int(0, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(gcd(Int(14, 4, Unsigned), Int(0, 4, Unsigned)) ==
              Int(14, 4, Unsigned));
  BOOST_CHECK(gcd(Int(15, 4, Unsigned), Int(0, 4, Unsigned)) ==
              Int(15, 4, Unsigned));

  BOOST_CHECK(gcd(Int(0, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(gcd(Int(1, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(gcd(Int(14, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(gcd(Int(15, 4, Unsigned), Int(1, 4, Unsigned)) ==
              Int(1, 4, Unsigned));

  BOOST_CHECK(gcd(Int(0, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(2, 4, Unsigned));
  BOOST_CHECK(gcd(Int(1, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(gcd(Int(3, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(gcd(Int(4, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(2, 4, Unsigned));
  BOOST_CHECK(gcd(Int(14, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(2, 4, Unsigned));
  BOOST_CHECK(gcd(Int(15, 4, Unsigned), Int(2, 4, Unsigned)) ==
              Int(1, 4, Unsigned));

  BOOST_CHECK(gcd(Int(0, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(3, 4, Unsigned));
  BOOST_CHECK(gcd(Int(1, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(gcd(Int(3, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(3, 4, Unsigned));
  BOOST_CHECK(gcd(Int(4, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(gcd(Int(14, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(gcd(Int(15, 4, Unsigned), Int(3, 4, Unsigned)) ==
              Int(3, 4, Unsigned));

  BOOST_CHECK(gcd(Int(0, 4, Unsigned), Int(5, 4, Unsigned)) ==
              Int(5, 4, Unsigned));
  BOOST_CHECK(gcd(Int(1, 4, Unsigned), Int(5, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(gcd(Int(3, 4, Unsigned), Int(5, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(gcd(Int(4, 4, Unsigned), Int(5, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(gcd(Int(1, 4, Unsigned), Int(5, 4, Unsigned)) ==
              Int(1, 4, Unsigned));
  BOOST_CHECK(gcd(Int(15, 4, Unsigned), Int(5, 4, Unsigned)) ==
              Int(5, 4, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_lt_signed) {
  BOOST_CHECK(Int(-8, 4, Signed) < Int(0, 4, Signed));
  BOOST_CHECK(Int(-1, 4, Signed) < Int(0, 4, Signed));
  BOOST_CHECK(!(Int(0, 4, Signed) < Int(0, 4, Signed)));
  BOOST_CHECK(!(Int(7, 4, Signed) < Int(0, 4, Signed)));
  BOOST_CHECK(Int(-8, 4, Signed) < Int(-7, 4, Signed));
  BOOST_CHECK(!(Int(-1, 4, Signed) < Int(-7, 4, Signed)));
  BOOST_CHECK(!(Int(0, 4, Signed) < Int(-7, 4, Signed)));
  BOOST_CHECK(!(Int(7, 4, Signed) < Int(-7, 4, Signed)));
  BOOST_CHECK(!(Int(-8, 4, Signed) < Int(-8, 4, Signed)));
  BOOST_CHECK(!(Int(-1, 4, Signed) < Int(-8, 4, Signed)));
  BOOST_CHECK(!(Int(0, 4, Signed) < Int(-8, 4, Signed)));
  BOOST_CHECK(!(Int(7, 4, Signed) < Int(-8, 4, Signed)));
  BOOST_CHECK(Int(-8, 4, Signed) < Int(7, 4, Signed));
  BOOST_CHECK(Int(-1, 4, Signed) < Int(7, 4, Signed));
  BOOST_CHECK(Int(0, 4, Signed) < Int(7, 4, Signed));
  BOOST_CHECK(!(Int(7, 4, Signed) < Int(7, 4, Signed)));
}

BOOST_AUTO_TEST_CASE(test_lt_unsigned) {
  BOOST_CHECK(!(Int(0, 4, Unsigned) < Int(0, 4, Unsigned)));
  BOOST_CHECK(!(Int(13, 4, Unsigned) < Int(0, 4, Unsigned)));
  BOOST_CHECK(!(Int(15, 4, Unsigned) < Int(0, 4, Unsigned)));
  BOOST_CHECK(Int(0, 4, Unsigned) < Int(1, 4, Unsigned));
  BOOST_CHECK(!(Int(13, 4, Unsigned) < Int(1, 4, Unsigned)));
  BOOST_CHECK(!(Int(15, 4, Unsigned) < Int(1, 4, Unsigned)));
  BOOST_CHECK(Int(0, 4, Unsigned) < Int(15, 4, Unsigned));
  BOOST_CHECK(Int(13, 4, Unsigned) < Int(15, 4, Unsigned));
  BOOST_CHECK(!(Int(15, 4, Unsigned) < Int(15, 4, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_le_signed) {
  BOOST_CHECK(Int(-8, 4, Signed) <= Int(0, 4, Signed));
  BOOST_CHECK(Int(-1, 4, Signed) <= Int(0, 4, Signed));
  BOOST_CHECK(Int(0, 4, Signed) <= Int(0, 4, Signed));
  BOOST_CHECK(!(Int(7, 4, Signed) <= Int(0, 4, Signed)));
  BOOST_CHECK(Int(-8, 4, Signed) <= Int(-7, 4, Signed));
  BOOST_CHECK(!(Int(-1, 4, Signed) <= Int(-7, 4, Signed)));
  BOOST_CHECK(!(Int(0, 4, Signed) <= Int(-7, 4, Signed)));
  BOOST_CHECK(!(Int(7, 4, Signed) <= Int(-7, 4, Signed)));
  BOOST_CHECK(Int(-8, 4, Signed) <= Int(-8, 4, Signed));
  BOOST_CHECK(!(Int(-1, 4, Signed) <= Int(-8, 4, Signed)));
  BOOST_CHECK(!(Int(0, 4, Signed) <= Int(-8, 4, Signed)));
  BOOST_CHECK(!(Int(7, 4, Signed) <= Int(-8, 4, Signed)));
  BOOST_CHECK(Int(-8, 4, Signed) <= Int(7, 4, Signed));
  BOOST_CHECK(Int(-1, 4, Signed) <= Int(7, 4, Signed));
  BOOST_CHECK(Int(0, 4, Signed) <= Int(7, 4, Signed));
  BOOST_CHECK(Int(7, 4, Signed) <= Int(7, 4, Signed));
}

BOOST_AUTO_TEST_CASE(test_le_unsigned) {
  BOOST_CHECK(Int(0, 4, Unsigned) <= Int(0, 4, Unsigned));
  BOOST_CHECK(!(Int(13, 4, Unsigned) <= Int(0, 4, Unsigned)));
  BOOST_CHECK(!(Int(15, 4, Unsigned) <= Int(0, 4, Unsigned)));
  BOOST_CHECK(Int(0, 4, Unsigned) <= Int(1, 4, Unsigned));
  BOOST_CHECK(!(Int(13, 4, Unsigned) <= Int(1, 4, Unsigned)));
  BOOST_CHECK(!(Int(15, 4, Unsigned) <= Int(1, 4, Unsigned)));
  BOOST_CHECK(Int(0, 4, Unsigned) <= Int(15, 4, Unsigned));
  BOOST_CHECK(Int(13, 4, Unsigned) <= Int(15, 4, Unsigned));
  BOOST_CHECK(Int(15, 4, Unsigned) <= Int(15, 4, Unsigned));
}
