/*******************************************************************************
 *
 * Tests for machine_int::Constant
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

#define BOOST_TEST_MODULE test_machine_integer_constant
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/number/machine_int.hpp>
#include <ikos/core/value/machine_int/constant.hpp>

using Int = ikos::core::MachineInt;
using Constant = ikos::core::machine_int::Constant;
using ikos::core::Signed;
using ikos::core::Unsigned;

BOOST_AUTO_TEST_CASE(test_bit_width) {
  BOOST_CHECK(Constant(Int(0, 4, Signed)).bit_width() == 4);
  BOOST_CHECK(Constant(Int(0, 4, Unsigned)).bit_width() == 4);
  BOOST_CHECK(Constant(Int(0, 8, Signed)).bit_width() == 8);
  BOOST_CHECK(Constant(Int(0, 8, Unsigned)).bit_width() == 8);
}

BOOST_AUTO_TEST_CASE(test_sign) {
  BOOST_CHECK(Constant(Int(0, 4, Signed)).sign() == Signed);
  BOOST_CHECK(Constant(Int(0, 4, Unsigned)).sign() == Unsigned);
  BOOST_CHECK(Constant(Int(0, 8, Signed)).sign() == Signed);
  BOOST_CHECK(Constant(Int(0, 8, Unsigned)).sign() == Unsigned);
}

BOOST_AUTO_TEST_CASE(test_bottom) {
  BOOST_CHECK(Constant::bottom(8, Signed).is_bottom());
  BOOST_CHECK(!Constant::top(8, Signed).is_bottom());
  BOOST_CHECK(!Constant(Int(0, 8, Signed)).is_bottom());
  BOOST_CHECK(Constant::bottom(8, Unsigned).is_bottom());
  BOOST_CHECK(!Constant::top(8, Unsigned).is_bottom());
  BOOST_CHECK(!Constant(Int(0, 8, Unsigned)).is_bottom());
}

BOOST_AUTO_TEST_CASE(test_top) {
  BOOST_CHECK(!Constant::bottom(8, Signed).is_top());
  BOOST_CHECK(Constant::top(8, Signed).is_top());
  BOOST_CHECK(!Constant(Int(0, 8, Signed)).is_top());
  BOOST_CHECK(!Constant::bottom(8, Unsigned).is_top());
  BOOST_CHECK(Constant::top(8, Unsigned).is_top());
  BOOST_CHECK(!Constant(Int(0, 8, Unsigned)).is_top());
}

BOOST_AUTO_TEST_CASE(test_set_to_bottom) {
  Constant i(Int(0, 8, Signed));
  i.set_to_bottom();
  BOOST_CHECK(i.is_bottom());
  BOOST_CHECK(!i.is_top());
}

BOOST_AUTO_TEST_CASE(test_set_to_top) {
  Constant i(Int(0, 8, Signed));
  i.set_to_top();
  BOOST_CHECK(!i.is_bottom());
  BOOST_CHECK(i.is_top());
}

BOOST_AUTO_TEST_CASE(test_leq) {
  BOOST_CHECK(Constant::bottom(1, Signed).leq(Constant::bottom(1, Signed)));
  BOOST_CHECK(Constant::bottom(1, Signed).leq(Constant::top(1, Signed)));
  BOOST_CHECK(Constant::bottom(1, Signed).leq(Constant(Int(0, 1, Signed))));

  BOOST_CHECK(!Constant::top(1, Signed).leq(Constant::bottom(1, Signed)));
  BOOST_CHECK(Constant::top(1, Signed).leq(Constant::top(1, Signed)));
  BOOST_CHECK(!Constant::top(1, Signed).leq(Constant(Int(0, 1, Signed))));

  BOOST_CHECK(!Constant(Int(0, 8, Signed)).leq(Constant::bottom(8, Signed)));
  BOOST_CHECK(Constant(Int(0, 8, Signed)).leq(Constant::top(8, Signed)));
  BOOST_CHECK(Constant(Int(0, 8, Signed)).leq(Constant(Int(0, 8, Signed))));
  BOOST_CHECK(!Constant(Int(0, 8, Signed)).leq(Constant(Int(1, 8, Signed))));

  BOOST_CHECK(
      !Constant(Int(0, 8, Unsigned)).leq(Constant::bottom(8, Unsigned)));
  BOOST_CHECK(Constant(Int(0, 8, Unsigned)).leq(Constant::top(8, Unsigned)));
  BOOST_CHECK(Constant(Int(0, 8, Unsigned)).leq(Constant(Int(0, 8, Unsigned))));
  BOOST_CHECK(
      !Constant(Int(0, 8, Unsigned)).leq(Constant(Int(1, 8, Unsigned))));
}

BOOST_AUTO_TEST_CASE(test_equals) {
  BOOST_CHECK(Constant::bottom(1, Signed).equals(Constant::bottom(1, Signed)));
  BOOST_CHECK(!Constant::bottom(1, Signed).equals(Constant::top(1, Signed)));
  BOOST_CHECK(!Constant::bottom(1, Signed).equals(Constant(Int(0, 1, Signed))));

  BOOST_CHECK(!Constant::top(1, Signed).equals(Constant::bottom(1, Signed)));
  BOOST_CHECK(Constant::top(1, Signed).equals(Constant::top(1, Signed)));
  BOOST_CHECK(!Constant::top(1, Signed).equals(Constant(Int(0, 1, Signed))));

  BOOST_CHECK(!Constant(Int(0, 8, Signed)).equals(Constant::bottom(8, Signed)));
  BOOST_CHECK(!Constant(Int(0, 8, Signed)).equals(Constant::top(8, Signed)));
  BOOST_CHECK(Constant(Int(0, 8, Signed)).equals(Constant(Int(0, 8, Signed))));
  BOOST_CHECK(!Constant(Int(0, 8, Signed)).equals(Constant(Int(1, 8, Signed))));

  BOOST_CHECK(
      !Constant(Int(0, 8, Unsigned)).equals(Constant::bottom(8, Unsigned)));
  BOOST_CHECK(
      !Constant(Int(0, 8, Unsigned)).equals(Constant::top(8, Unsigned)));
  BOOST_CHECK(
      Constant(Int(0, 8, Unsigned)).equals(Constant(Int(0, 8, Unsigned))));
  BOOST_CHECK(
      !Constant(Int(0, 8, Unsigned)).equals(Constant(Int(1, 8, Unsigned))));
}

BOOST_AUTO_TEST_CASE(test_join) {
  BOOST_CHECK(Constant::top(1, Signed).join(Constant::bottom(1, Signed)) ==
              Constant::top(1, Signed));
  BOOST_CHECK(Constant::top(1, Signed).join(Constant::top(1, Signed)) ==
              Constant::top(1, Signed));
  BOOST_CHECK(Constant::top(1, Signed).join(Constant(Int(0, 1, Signed))) ==
              Constant::top(1, Signed));

  BOOST_CHECK(Constant::bottom(1, Signed).join(Constant::bottom(1, Signed)) ==
              Constant::bottom(1, Signed));
  BOOST_CHECK(Constant::bottom(1, Signed).join(Constant::top(1, Signed)) ==
              Constant::top(1, Signed));
  BOOST_CHECK(Constant::bottom(1, Signed).join(Constant(Int(0, 1, Signed))) ==
              Constant(Int(0, 1, Signed)));

  BOOST_CHECK(Constant(Int(0, 8, Signed)).join(Constant::bottom(8, Signed)) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(Constant(Int(0, 8, Signed)).join(Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(Constant(Int(0, 8, Signed)).join(Constant(Int(0, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(Constant(Int(0, 8, Signed)).join(Constant(Int(1, 8, Signed))) ==
              Constant::top(8, Signed));

  BOOST_CHECK(
      Constant(Int(0, 8, Unsigned)).join(Constant::bottom(8, Unsigned)) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(Constant(Int(0, 8, Unsigned)).join(Constant::top(8, Unsigned)) ==
              Constant::top(8, Unsigned));
  BOOST_CHECK(
      Constant(Int(0, 8, Unsigned)).join(Constant(Int(0, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Constant(Int(0, 8, Unsigned)).join(Constant(Int(1, 8, Unsigned))) ==
      Constant::top(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_widening) {
  BOOST_CHECK(Constant::top(1, Signed).widening(Constant::bottom(1, Signed)) ==
              Constant::top(1, Signed));
  BOOST_CHECK(Constant::top(1, Signed).widening(Constant::top(1, Signed)) ==
              Constant::top(1, Signed));
  BOOST_CHECK(Constant::top(1, Signed).widening(Constant(Int(0, 1, Signed))) ==
              Constant::top(1, Signed));

  BOOST_CHECK(Constant::bottom(1, Signed).widening(
                  Constant::bottom(1, Signed)) == Constant::bottom(1, Signed));
  BOOST_CHECK(Constant::bottom(1, Signed).widening(Constant::top(1, Signed)) ==
              Constant::top(1, Signed));
  BOOST_CHECK(Constant::bottom(1, Signed).widening(
                  Constant(Int(0, 1, Signed))) == Constant(Int(0, 1, Signed)));

  BOOST_CHECK(
      Constant(Int(0, 8, Signed)).widening(Constant::bottom(8, Signed)) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(Constant(Int(0, 8, Signed)).widening(Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(
      Constant(Int(0, 8, Signed)).widening(Constant(Int(0, 8, Signed))) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      Constant(Int(0, 8, Signed)).widening(Constant(Int(1, 8, Signed))) ==
      Constant::top(8, Signed));

  BOOST_CHECK(
      Constant(Int(0, 8, Unsigned)).widening(Constant::bottom(8, Unsigned)) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Constant(Int(0, 8, Unsigned)).widening(Constant::top(8, Unsigned)) ==
      Constant::top(8, Unsigned));
  BOOST_CHECK(
      Constant(Int(0, 8, Unsigned)).widening(Constant(Int(0, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Constant(Int(0, 8, Unsigned)).widening(Constant(Int(1, 8, Unsigned))) ==
      Constant::top(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_meet) {
  BOOST_CHECK(Constant::top(1, Signed).meet(Constant::bottom(1, Signed)) ==
              Constant::bottom(1, Signed));
  BOOST_CHECK(Constant::top(1, Signed).meet(Constant::top(1, Signed)) ==
              Constant::top(1, Signed));
  BOOST_CHECK(Constant::top(1, Signed).meet(Constant(Int(0, 1, Signed))) ==
              Constant(Int(0, 1, Signed)));

  BOOST_CHECK(Constant::bottom(1, Signed).meet(Constant::bottom(1, Signed)) ==
              Constant::bottom(1, Signed));
  BOOST_CHECK(Constant::bottom(1, Signed).meet(Constant::top(1, Signed)) ==
              Constant::bottom(1, Signed));
  BOOST_CHECK(Constant::bottom(1, Signed).meet(Constant(Int(0, 1, Signed))) ==
              Constant::bottom(1, Signed));

  BOOST_CHECK(Constant(Int(0, 8, Signed)).meet(Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(Constant(Int(0, 8, Signed)).meet(Constant::top(8, Signed)) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(Constant(Int(0, 8, Signed)).meet(Constant(Int(0, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(Constant(Int(0, 8, Signed)).meet(Constant(Int(1, 8, Signed))) ==
              Constant::bottom(8, Signed));

  BOOST_CHECK(
      Constant(Int(0, 8, Unsigned)).meet(Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(Constant(Int(0, 8, Unsigned)).meet(Constant::top(8, Unsigned)) ==
              Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Constant(Int(0, 8, Unsigned)).meet(Constant(Int(0, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Constant(Int(0, 8, Unsigned)).meet(Constant(Int(1, 8, Unsigned))) ==
      Constant::bottom(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_narrowing) {
  BOOST_CHECK(Constant::top(1, Signed).narrowing(Constant::bottom(1, Signed)) ==
              Constant::bottom(1, Signed));
  BOOST_CHECK(Constant::top(1, Signed).narrowing(Constant::top(1, Signed)) ==
              Constant::top(1, Signed));
  BOOST_CHECK(Constant::top(1, Signed).narrowing(Constant(Int(0, 1, Signed))) ==
              Constant(Int(0, 1, Signed)));

  BOOST_CHECK(Constant::bottom(1, Signed).narrowing(
                  Constant::bottom(1, Signed)) == Constant::bottom(1, Signed));
  BOOST_CHECK(Constant::bottom(1, Signed).narrowing(Constant::top(1, Signed)) ==
              Constant::bottom(1, Signed));
  BOOST_CHECK(Constant::bottom(1, Signed).narrowing(
                  Constant(Int(0, 1, Signed))) == Constant::bottom(1, Signed));

  BOOST_CHECK(
      Constant(Int(0, 8, Signed)).narrowing(Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(Constant(Int(0, 8, Signed)).narrowing(Constant::top(8, Signed)) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      Constant(Int(0, 8, Signed)).narrowing(Constant(Int(0, 8, Signed))) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      Constant(Int(0, 8, Signed)).narrowing(Constant(Int(1, 8, Signed))) ==
      Constant::bottom(8, Signed));

  BOOST_CHECK(
      Constant(Int(0, 8, Unsigned)).narrowing(Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(
      Constant(Int(0, 8, Unsigned)).narrowing(Constant::top(8, Unsigned)) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Constant(Int(0, 8, Unsigned)).narrowing(Constant(Int(0, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Constant(Int(0, 8, Unsigned)).narrowing(Constant(Int(1, 8, Unsigned))) ==
      Constant::bottom(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_trunc) {
  BOOST_CHECK(Constant::bottom(8, Signed).trunc(6) ==
              Constant::bottom(6, Signed));
  BOOST_CHECK(Constant::top(8, Signed).trunc(6) == Constant::top(6, Signed));
  BOOST_CHECK(Constant(Int(0, 8, Signed)).trunc(6) ==
              Constant(Int(0, 6, Signed)));
  BOOST_CHECK(Constant(Int(1, 8, Signed)).trunc(6) ==
              Constant(Int(1, 6, Signed)));
  BOOST_CHECK(Constant(Int(65, 8, Signed)).trunc(6) ==
              Constant(Int(1, 6, Signed)));
  BOOST_CHECK(Constant(Int(-1, 8, Signed)).trunc(6) ==
              Constant(Int(-1, 6, Signed)));

  BOOST_CHECK(Constant::bottom(8, Unsigned).trunc(6) ==
              Constant::bottom(6, Unsigned));
  BOOST_CHECK(Constant::top(8, Unsigned).trunc(6) ==
              Constant::top(6, Unsigned));
  BOOST_CHECK(Constant(Int(0, 8, Unsigned)).trunc(6) ==
              Constant(Int(0, 6, Unsigned)));
  BOOST_CHECK(Constant(Int(1, 8, Unsigned)).trunc(6) ==
              Constant(Int(1, 6, Unsigned)));
  BOOST_CHECK(Constant(Int(65, 8, Unsigned)).trunc(6) ==
              Constant(Int(1, 6, Unsigned)));
  BOOST_CHECK(Constant(Int(255, 8, Unsigned)).trunc(6) ==
              Constant(Int(63, 6, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_ext) {
  BOOST_CHECK(Constant::bottom(6, Signed).ext(8) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(Constant::top(6, Signed).ext(8) == Constant::top(8, Signed));
  BOOST_CHECK(Constant(Int(0, 6, Signed)).ext(8) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(Constant(Int(1, 6, Signed)).ext(8) ==
              Constant(Int(1, 8, Signed)));
  BOOST_CHECK(Constant(Int(-1, 6, Signed)).ext(8) ==
              Constant(Int(-1, 8, Signed)));

  BOOST_CHECK(Constant::bottom(6, Unsigned).ext(8) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(Constant::top(6, Unsigned).ext(8) == Constant::top(8, Unsigned));
  BOOST_CHECK(Constant(Int(0, 6, Unsigned)).ext(8) ==
              Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(Constant(Int(1, 6, Unsigned)).ext(8) ==
              Constant(Int(1, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_sign_cast) {
  BOOST_CHECK(Constant::bottom(8, Signed).sign_cast(Unsigned) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(Constant::top(8, Signed).sign_cast(Unsigned) ==
              Constant::top(8, Unsigned));
  BOOST_CHECK(Constant(Int(0, 8, Signed)).sign_cast(Unsigned) ==
              Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(Constant(Int(32, 8, Signed)).sign_cast(Unsigned) ==
              Constant(Int(32, 8, Unsigned)));
  BOOST_CHECK(Constant(Int(96, 8, Signed)).sign_cast(Unsigned) ==
              Constant(Int(96, 8, Unsigned)));
  BOOST_CHECK(Constant(Int(-1, 8, Signed)).sign_cast(Unsigned) ==
              Constant(Int(255, 8, Unsigned)));
  BOOST_CHECK(Constant(Int(-2, 8, Signed)).sign_cast(Unsigned) ==
              Constant(Int(254, 8, Unsigned)));

  BOOST_CHECK(Constant::bottom(8, Unsigned).sign_cast(Signed) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(Constant::top(8, Unsigned).sign_cast(Signed) ==
              Constant::top(8, Signed));
  BOOST_CHECK(Constant(Int(0, 8, Unsigned)).sign_cast(Signed) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(Constant(Int(255, 8, Unsigned)).sign_cast(Signed) ==
              Constant(Int(-1, 8, Signed)));
  BOOST_CHECK(Constant(Int(32, 8, Unsigned)).sign_cast(Signed) ==
              Constant(Int(32, 8, Signed)));
  BOOST_CHECK(Constant(Int(150, 8, Unsigned)).sign_cast(Signed) ==
              Constant(Int(-106, 8, Signed)));
  BOOST_CHECK(Constant(Int(254, 8, Unsigned)).sign_cast(Signed) ==
              Constant(Int(-2, 8, Signed)));
}

BOOST_AUTO_TEST_CASE(test_cast) {
  BOOST_CHECK(Constant::bottom(8, Signed).cast(4, Unsigned) ==
              Constant::bottom(4, Unsigned));
  BOOST_CHECK(Constant::top(8, Signed).cast(4, Unsigned) ==
              Constant::top(4, Unsigned));
  BOOST_CHECK(Constant(Int(0, 8, Signed)).cast(4, Unsigned) ==
              Constant(Int(0, 4, Unsigned)));
  BOOST_CHECK(Constant(Int(32, 8, Signed)).cast(4, Unsigned) ==
              Constant(Int(0, 4, Unsigned)));
  BOOST_CHECK(Constant(Int(96, 8, Signed)).cast(4, Unsigned) ==
              Constant(Int(0, 4, Unsigned)));
  BOOST_CHECK(Constant(Int(-1, 8, Signed)).cast(4, Unsigned) ==
              Constant(Int(15, 4, Unsigned)));
  BOOST_CHECK(Constant(Int(-2, 8, Signed)).cast(4, Unsigned) ==
              Constant(Int(14, 4, Unsigned)));

  BOOST_CHECK(Constant::bottom(8, Unsigned).cast(4, Signed) ==
              Constant::bottom(4, Signed));
  BOOST_CHECK(Constant::top(8, Unsigned).cast(4, Signed) ==
              Constant::top(4, Signed));
  BOOST_CHECK(Constant(Int(0, 8, Unsigned)).cast(4, Signed) ==
              Constant(Int(0, 4, Signed)));
  BOOST_CHECK(Constant(Int(255, 8, Unsigned)).cast(4, Signed) ==
              Constant(Int(-1, 4, Signed)));
  BOOST_CHECK(Constant(Int(32, 8, Unsigned)).cast(4, Signed) ==
              Constant(Int(0, 4, Signed)));
  BOOST_CHECK(Constant(Int(150, 8, Unsigned)).cast(4, Signed) ==
              Constant(Int(6, 4, Signed)));
  BOOST_CHECK(Constant(Int(254, 8, Unsigned)).cast(4, Signed) ==
              Constant(Int(-2, 4, Signed)));
}

BOOST_AUTO_TEST_CASE(test_singleton) {
  BOOST_CHECK((Constant::bottom(8, Signed).singleton() == boost::none));
  BOOST_CHECK((Constant::top(8, Signed).singleton() == boost::none));
  BOOST_CHECK((Constant(Int(0, 8, Signed)).singleton() ==
               boost::optional< Int >(Int(0, 8, Signed))));

  BOOST_CHECK((Constant::bottom(8, Unsigned).singleton() == boost::none));
  BOOST_CHECK((Constant::top(8, Unsigned).singleton() == boost::none));
  BOOST_CHECK((Constant(Int(0, 8, Unsigned)).singleton() ==
               boost::optional< Int >(Int(0, 8, Unsigned))));
}

BOOST_AUTO_TEST_CASE(test_contains) {
  BOOST_CHECK(!Constant::bottom(8, Signed).contains(Int(0, 8, Signed)));
  BOOST_CHECK(Constant::top(8, Signed).contains(Int(0, 8, Signed)));
  BOOST_CHECK(Constant(Int(0, 8, Signed)).contains(Int(0, 8, Signed)));
  BOOST_CHECK(!Constant(Int(0, 8, Signed)).contains(Int(2, 8, Signed)));

  BOOST_CHECK(!Constant::bottom(8, Unsigned).contains(Int(0, 8, Unsigned)));
  BOOST_CHECK(Constant::top(8, Unsigned).contains(Int(0, 8, Unsigned)));
  BOOST_CHECK(Constant(Int(1, 8, Unsigned)).contains(Int(1, 8, Unsigned)));
  BOOST_CHECK(!Constant(Int(1, 8, Unsigned)).contains(Int(3, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_add) {
  BOOST_CHECK(add(Constant::top(1, Signed), Constant::bottom(1, Signed)) ==
              Constant::bottom(1, Signed));
  BOOST_CHECK(add(Constant::top(1, Signed), Constant::top(1, Signed)) ==
              Constant::top(1, Signed));
  BOOST_CHECK(add(Constant::bottom(1, Signed), Constant::bottom(1, Signed)) ==
              Constant::bottom(1, Signed));
  BOOST_CHECK(add(Constant::bottom(1, Signed), Constant::top(1, Signed)) ==
              Constant::bottom(1, Signed));

  BOOST_CHECK(add(Constant(Int(0, 8, Signed)), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(add(Constant(Int(0, 8, Signed)), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(add(Constant(Int(0, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(1, 8, Signed)));
  BOOST_CHECK(add(Constant(Int(-1, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(add(Constant(Int(127, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(-128, 8, Signed)));
  BOOST_CHECK(
      add(Constant(Int(-128, 8, Signed)), Constant(Int(-129, 8, Signed))) ==
      Constant(Int(-1, 8, Signed)));

  BOOST_CHECK(
      add(Constant(Int(0, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(add(Constant(Int(0, 8, Unsigned)), Constant::top(8, Unsigned)) ==
              Constant::top(8, Unsigned));
  BOOST_CHECK(
      add(Constant(Int(0, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      add(Constant(Int(1, 8, Unsigned)), Constant(Int(2, 8, Unsigned))) ==
      Constant(Int(3, 8, Unsigned)));
  BOOST_CHECK(
      add(Constant(Int(255, 8, Unsigned)), Constant(Int(256, 8, Unsigned))) ==
      Constant(Int(255, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_add_no_wrap) {
  BOOST_CHECK(
      add_no_wrap(Constant::top(1, Signed), Constant::bottom(1, Signed)) ==
      Constant::bottom(1, Signed));
  BOOST_CHECK(add_no_wrap(Constant::top(1, Signed), Constant::top(1, Signed)) ==
              Constant::top(1, Signed));
  BOOST_CHECK(
      add_no_wrap(Constant::bottom(1, Signed), Constant::bottom(1, Signed)) ==
      Constant::bottom(1, Signed));
  BOOST_CHECK(
      add_no_wrap(Constant::bottom(1, Signed), Constant::top(1, Signed)) ==
      Constant::bottom(1, Signed));

  BOOST_CHECK(
      add_no_wrap(Constant(Int(0, 8, Signed)), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      add_no_wrap(Constant(Int(0, 8, Signed)), Constant::top(8, Signed)) ==
      Constant::top(8, Signed));
  BOOST_CHECK(
      add_no_wrap(Constant(Int(0, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(1, 8, Signed)));
  BOOST_CHECK(
      add_no_wrap(Constant(Int(-1, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      add_no_wrap(Constant(Int(127, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(add_no_wrap(Constant(Int(-128, 8, Signed)),
                          Constant(Int(-129, 8, Signed))) ==
              Constant(Int(-1, 8, Signed)));

  BOOST_CHECK(add_no_wrap(Constant(Int(0, 8, Unsigned)),
                          Constant::bottom(8, Unsigned)) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(
      add_no_wrap(Constant(Int(0, 8, Unsigned)), Constant::top(8, Unsigned)) ==
      Constant::top(8, Unsigned));
  BOOST_CHECK(add_no_wrap(Constant(Int(0, 8, Unsigned)),
                          Constant(Int(1, 8, Unsigned))) ==
              Constant(Int(1, 8, Unsigned)));
  BOOST_CHECK(add_no_wrap(Constant(Int(1, 8, Unsigned)),
                          Constant(Int(2, 8, Unsigned))) ==
              Constant(Int(3, 8, Unsigned)));
  BOOST_CHECK(add_no_wrap(Constant(Int(255, 8, Unsigned)),
                          Constant(Int(256, 8, Unsigned))) ==
              Constant(Int(255, 8, Unsigned)));
  BOOST_CHECK(add_no_wrap(Constant(Int(255, 8, Unsigned)),
                          Constant(Int(1, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_sub) {
  BOOST_CHECK(sub(Constant::top(1, Signed), Constant::bottom(1, Signed)) ==
              Constant::bottom(1, Signed));
  BOOST_CHECK(sub(Constant::top(1, Signed), Constant::top(1, Signed)) ==
              Constant::top(1, Signed));
  BOOST_CHECK(sub(Constant::bottom(1, Signed), Constant::bottom(1, Signed)) ==
              Constant::bottom(1, Signed));
  BOOST_CHECK(sub(Constant::bottom(1, Signed), Constant::top(1, Signed)) ==
              Constant::bottom(1, Signed));

  BOOST_CHECK(sub(Constant(Int(0, 8, Signed)), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(sub(Constant(Int(0, 8, Signed)), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(sub(Constant(Int(0, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(-1, 8, Signed)));
  BOOST_CHECK(sub(Constant(Int(1, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      sub(Constant(Int(-128, 8, Signed)), Constant(Int(-129, 8, Signed))) ==
      Constant(Int(1, 8, Signed)));
  BOOST_CHECK(
      sub(Constant(Int(-128, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(127, 8, Signed)));

  BOOST_CHECK(
      sub(Constant(Int(0, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(sub(Constant(Int(0, 8, Unsigned)), Constant::top(8, Unsigned)) ==
              Constant::top(8, Unsigned));
  BOOST_CHECK(
      sub(Constant(Int(0, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(-1, 8, Unsigned)));
  BOOST_CHECK(
      sub(Constant(Int(1, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      sub(Constant(Int(255, 8, Unsigned)), Constant(Int(256, 8, Unsigned))) ==
      Constant(Int(255, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_sub_no_wrap) {
  BOOST_CHECK(
      sub_no_wrap(Constant::top(1, Signed), Constant::bottom(1, Signed)) ==
      Constant::bottom(1, Signed));
  BOOST_CHECK(sub_no_wrap(Constant::top(1, Signed), Constant::top(1, Signed)) ==
              Constant::top(1, Signed));
  BOOST_CHECK(
      sub_no_wrap(Constant::bottom(1, Signed), Constant::bottom(1, Signed)) ==
      Constant::bottom(1, Signed));
  BOOST_CHECK(
      sub_no_wrap(Constant::bottom(1, Signed), Constant::top(1, Signed)) ==
      Constant::bottom(1, Signed));

  BOOST_CHECK(
      sub_no_wrap(Constant(Int(0, 8, Signed)), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      sub_no_wrap(Constant(Int(0, 8, Signed)), Constant::top(8, Signed)) ==
      Constant::top(8, Signed));
  BOOST_CHECK(
      sub_no_wrap(Constant(Int(0, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(-1, 8, Signed)));
  BOOST_CHECK(
      sub_no_wrap(Constant(Int(1, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Constant(Int(-128, 8, Signed)),
                          Constant(Int(-129, 8, Signed))) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(sub_no_wrap(Constant(Int(-128, 8, Signed)),
                          Constant(Int(1, 8, Signed))) ==
              Constant::bottom(8, Signed));

  BOOST_CHECK(sub_no_wrap(Constant(Int(0, 8, Unsigned)),
                          Constant::bottom(8, Unsigned)) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(
      sub_no_wrap(Constant(Int(0, 8, Unsigned)), Constant::top(8, Unsigned)) ==
      Constant::top(8, Unsigned));
  BOOST_CHECK(sub_no_wrap(Constant(Int(0, 8, Unsigned)),
                          Constant(Int(1, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(sub_no_wrap(Constant(Int(1, 8, Unsigned)),
                          Constant(Int(1, 8, Unsigned))) ==
              Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(sub_no_wrap(Constant(Int(255, 8, Unsigned)),
                          Constant(Int(256, 8, Unsigned))) ==
              Constant(Int(255, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_div) {
  BOOST_CHECK(div(Constant::top(1, Signed), Constant::bottom(1, Signed)) ==
              Constant::bottom(1, Signed));
  BOOST_CHECK(div(Constant::top(1, Signed), Constant::top(1, Signed)) ==
              Constant::top(1, Signed));
  BOOST_CHECK(div(Constant::bottom(1, Signed), Constant::bottom(1, Signed)) ==
              Constant::bottom(1, Signed));
  BOOST_CHECK(div(Constant::bottom(1, Signed), Constant::top(1, Signed)) ==
              Constant::bottom(1, Signed));

  BOOST_CHECK(div(Constant(Int(0, 8, Signed)), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(div(Constant(Int(0, 8, Signed)), Constant::top(8, Signed)) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(div(Constant(Int(0, 8, Signed)), Constant(Int(0, 8, Signed))) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(div(Constant(Int(0, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(div(Constant(Int(1, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(1, 8, Signed)));
  BOOST_CHECK(div(Constant(Int(9, 8, Signed)), Constant(Int(3, 8, Signed))) ==
              Constant(Int(3, 8, Signed)));
  BOOST_CHECK(div(Constant(Int(-9, 8, Signed)), Constant(Int(3, 8, Signed))) ==
              Constant(Int(-3, 8, Signed)));
  BOOST_CHECK(
      div(Constant(Int(-128, 8, Signed)), Constant(Int(127, 8, Signed))) ==
      Constant(Int(-1, 8, Signed)));
  BOOST_CHECK(
      div(Constant(Int(-128, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(-128, 8, Signed)));
  BOOST_CHECK(div(Constant(Int(-128, 8, Signed)),
                  Constant(Int(-1, 8, Signed))) == Constant::bottom(8, Signed));

  BOOST_CHECK(
      div(Constant(Int(0, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(div(Constant(Int(0, 8, Unsigned)), Constant::top(8, Unsigned)) ==
              Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      div(Constant(Int(0, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      div(Constant(Int(1, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      div(Constant(Int(254, 8, Unsigned)), Constant(Int(255, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_div_exact) {
  BOOST_CHECK(
      div_exact(Constant::top(1, Signed), Constant::bottom(1, Signed)) ==
      Constant::bottom(1, Signed));
  BOOST_CHECK(div_exact(Constant::top(1, Signed), Constant::top(1, Signed)) ==
              Constant::top(1, Signed));
  BOOST_CHECK(
      div_exact(Constant::bottom(1, Signed), Constant::bottom(1, Signed)) ==
      Constant::bottom(1, Signed));
  BOOST_CHECK(
      div_exact(Constant::bottom(1, Signed), Constant::top(1, Signed)) ==
      Constant::bottom(1, Signed));

  BOOST_CHECK(
      div_exact(Constant(Int(0, 8, Signed)), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      div_exact(Constant(Int(0, 8, Signed)), Constant::top(8, Signed)) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      div_exact(Constant(Int(0, 8, Signed)), Constant(Int(0, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      div_exact(Constant(Int(0, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      div_exact(Constant(Int(1, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(1, 8, Signed)));
  BOOST_CHECK(
      div_exact(Constant(Int(9, 8, Signed)), Constant(Int(3, 8, Signed))) ==
      Constant(Int(3, 8, Signed)));
  BOOST_CHECK(
      div_exact(Constant(Int(10, 8, Signed)), Constant(Int(3, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      div_exact(Constant(Int(-9, 8, Signed)), Constant(Int(3, 8, Signed))) ==
      Constant(Int(-3, 8, Signed)));
  BOOST_CHECK(div_exact(Constant(Int(-128, 8, Signed)),
                        Constant(Int(127, 8, Signed))) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(
      div_exact(Constant(Int(-128, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(-128, 8, Signed)));
  BOOST_CHECK(
      div_exact(Constant(Int(-128, 8, Signed)), Constant(Int(-1, 8, Signed))) ==
      Constant::bottom(8, Signed));

  BOOST_CHECK(
      div_exact(Constant(Int(0, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(
      div_exact(Constant(Int(0, 8, Unsigned)), Constant::top(8, Unsigned)) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      div_exact(Constant(Int(0, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      div_exact(Constant(Int(1, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(1, 8, Unsigned)));
  BOOST_CHECK(div_exact(Constant(Int(254, 8, Unsigned)),
                        Constant(Int(255, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_rem) {
  BOOST_CHECK(rem(Constant::top(1, Signed), Constant::bottom(1, Signed)) ==
              Constant::bottom(1, Signed));
  BOOST_CHECK(rem(Constant::top(1, Signed), Constant::top(1, Signed)) ==
              Constant::top(1, Signed));
  BOOST_CHECK(rem(Constant::bottom(1, Signed), Constant::bottom(1, Signed)) ==
              Constant::bottom(1, Signed));
  BOOST_CHECK(rem(Constant::bottom(1, Signed), Constant::top(1, Signed)) ==
              Constant::bottom(1, Signed));

  BOOST_CHECK(rem(Constant(Int(0, 8, Signed)), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(rem(Constant(Int(0, 8, Signed)), Constant::top(8, Signed)) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(rem(Constant(Int(0, 8, Signed)), Constant(Int(0, 8, Signed))) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(rem(Constant(Int(0, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(rem(Constant(Int(1, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(rem(Constant(Int(10, 8, Signed)), Constant(Int(3, 8, Signed))) ==
              Constant(Int(1, 8, Signed)));
  BOOST_CHECK(rem(Constant(Int(-10, 8, Signed)), Constant(Int(3, 8, Signed))) ==
              Constant(Int(-1, 8, Signed)));
  BOOST_CHECK(
      rem(Constant(Int(-128, 8, Signed)), Constant(Int(127, 8, Signed))) ==
      Constant(Int(-1, 8, Signed)));
  BOOST_CHECK(rem(Constant(Int(-128, 8, Signed)),
                  Constant(Int(1, 8, Signed))) == Constant(Int(0, 8, Signed)));
  BOOST_CHECK(rem(Constant(Int(-128, 8, Signed)),
                  Constant(Int(-1, 8, Signed))) == Constant(Int(0, 8, Signed)));

  BOOST_CHECK(
      rem(Constant(Int(0, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(rem(Constant(Int(0, 8, Unsigned)), Constant::top(8, Unsigned)) ==
              Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      rem(Constant(Int(0, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      rem(Constant(Int(1, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      rem(Constant(Int(254, 8, Unsigned)), Constant(Int(255, 8, Unsigned))) ==
      Constant(Int(254, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_shl) {
  BOOST_CHECK(shl(Constant::top(8, Signed), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(shl(Constant::top(8, Signed), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(shl(Constant::bottom(8, Signed), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(shl(Constant::bottom(8, Signed), Constant::top(8, Signed)) ==
              Constant::bottom(8, Signed));

  BOOST_CHECK(shl(Constant(Int(0, 8, Signed)), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(shl(Constant(Int(0, 8, Signed)), Constant::top(8, Signed)) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(shl(Constant(Int(0, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(shl(Constant(Int(0, 8, Signed)), Constant(Int(2, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(shl(Constant(Int(0, 8, Signed)), Constant(Int(6, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(shl(Constant(Int(0, 8, Signed)), Constant(Int(7, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(shl(Constant(Int(0, 8, Signed)), Constant(Int(-1, 8, Signed))) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(shl(Constant(Int(0, 8, Signed)), Constant(Int(8, 8, Signed))) ==
              Constant::bottom(8, Signed));

  BOOST_CHECK(shl(Constant(Int(1, 8, Signed)), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(shl(Constant(Int(1, 8, Signed)), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(shl(Constant(Int(1, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(2, 8, Signed)));
  BOOST_CHECK(shl(Constant(Int(1, 8, Signed)), Constant(Int(2, 8, Signed))) ==
              Constant(Int(4, 8, Signed)));
  BOOST_CHECK(shl(Constant(Int(1, 8, Signed)), Constant(Int(6, 8, Signed))) ==
              Constant(Int(64, 8, Signed)));
  BOOST_CHECK(shl(Constant(Int(1, 8, Signed)), Constant(Int(7, 8, Signed))) ==
              Constant(Int(-128, 8, Signed)));
  BOOST_CHECK(shl(Constant(Int(1, 8, Signed)), Constant(Int(8, 8, Signed))) ==
              Constant::bottom(8, Signed));

  BOOST_CHECK(shl(Constant(Int(127, 8, Signed)), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(shl(Constant(Int(127, 8, Signed)), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(shl(Constant(Int(127, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(-2, 8, Signed)));
  BOOST_CHECK(shl(Constant(Int(127, 8, Signed)), Constant(Int(2, 8, Signed))) ==
              Constant(Int(-4, 8, Signed)));
  BOOST_CHECK(shl(Constant(Int(127, 8, Signed)), Constant(Int(6, 8, Signed))) ==
              Constant(Int(-64, 8, Signed)));
  BOOST_CHECK(shl(Constant(Int(127, 8, Signed)), Constant(Int(7, 8, Signed))) ==
              Constant(Int(-128, 8, Signed)));

  BOOST_CHECK(shl(Constant(Int(-128, 8, Signed)),
                  Constant::bottom(8, Signed)) == Constant::bottom(8, Signed));
  BOOST_CHECK(shl(Constant(Int(-128, 8, Signed)), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(shl(Constant(Int(-128, 8, Signed)),
                  Constant(Int(1, 8, Signed))) == Constant(Int(0, 8, Signed)));
  BOOST_CHECK(shl(Constant(Int(-128, 8, Signed)),
                  Constant(Int(2, 8, Signed))) == Constant(Int(0, 8, Signed)));
  BOOST_CHECK(shl(Constant(Int(-128, 8, Signed)),
                  Constant(Int(6, 8, Signed))) == Constant(Int(0, 8, Signed)));
  BOOST_CHECK(shl(Constant(Int(-128, 8, Signed)),
                  Constant(Int(7, 8, Signed))) == Constant(Int(0, 8, Signed)));
  BOOST_CHECK(shl(Constant(Int(-128, 8, Signed)),
                  Constant(Int(8, 8, Signed))) == Constant::bottom(8, Signed));

  BOOST_CHECK(
      shl(Constant(Int(0, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(shl(Constant(Int(0, 8, Unsigned)), Constant::top(8, Unsigned)) ==
              Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl(Constant(Int(0, 8, Unsigned)), Constant(Int(0, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl(Constant(Int(0, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl(Constant(Int(0, 8, Unsigned)), Constant(Int(2, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl(Constant(Int(0, 8, Unsigned)), Constant(Int(8, 8, Unsigned))) ==
      Constant::bottom(8, Unsigned));

  BOOST_CHECK(
      shl(Constant(Int(1, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(
      shl(Constant(Int(1, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(2, 8, Unsigned)));
  BOOST_CHECK(
      shl(Constant(Int(1, 8, Unsigned)), Constant(Int(2, 8, Unsigned))) ==
      Constant(Int(4, 8, Unsigned)));
  BOOST_CHECK(
      shl(Constant(Int(1, 8, Unsigned)), Constant(Int(6, 8, Unsigned))) ==
      Constant(Int(64, 8, Unsigned)));
  BOOST_CHECK(
      shl(Constant(Int(1, 8, Unsigned)), Constant(Int(7, 8, Unsigned))) ==
      Constant(Int(128, 8, Unsigned)));
  BOOST_CHECK(
      shl(Constant(Int(1, 8, Unsigned)), Constant(Int(8, 8, Unsigned))) ==
      Constant::bottom(8, Unsigned));

  BOOST_CHECK(
      shl(Constant(Int(127, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(shl(Constant(Int(127, 8, Unsigned)),
                  Constant::top(8, Unsigned)) == Constant::top(8, Unsigned));
  BOOST_CHECK(
      shl(Constant(Int(127, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(254, 8, Unsigned)));
  BOOST_CHECK(
      shl(Constant(Int(127, 8, Unsigned)), Constant(Int(2, 8, Unsigned))) ==
      Constant(Int(252, 8, Unsigned)));
  BOOST_CHECK(
      shl(Constant(Int(127, 8, Unsigned)), Constant(Int(6, 8, Unsigned))) ==
      Constant(Int(192, 8, Unsigned)));
  BOOST_CHECK(
      shl(Constant(Int(127, 8, Unsigned)), Constant(Int(7, 8, Unsigned))) ==
      Constant(Int(128, 8, Unsigned)));

  BOOST_CHECK(
      shl(Constant(Int(128, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(shl(Constant(Int(128, 8, Unsigned)),
                  Constant::top(8, Unsigned)) == Constant::top(8, Unsigned));
  BOOST_CHECK(
      shl(Constant(Int(128, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl(Constant(Int(128, 8, Unsigned)), Constant(Int(2, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl(Constant(Int(128, 8, Unsigned)), Constant(Int(6, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl(Constant(Int(128, 8, Unsigned)), Constant(Int(7, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_shl_no_wrap) {
  BOOST_CHECK(
      shl_no_wrap(Constant::top(8, Signed), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Constant::top(8, Signed), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Constant::bottom(8, Signed), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Constant::bottom(8, Signed), Constant::top(8, Signed)) ==
      Constant::bottom(8, Signed));

  BOOST_CHECK(
      shl_no_wrap(Constant(Int(0, 8, Signed)), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(0, 8, Signed)), Constant::top(8, Signed)) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(0, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(0, 8, Signed)), Constant(Int(2, 8, Signed))) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(0, 8, Signed)), Constant(Int(6, 8, Signed))) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(0, 8, Signed)), Constant(Int(7, 8, Signed))) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(0, 8, Signed)), Constant(Int(-1, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(0, 8, Signed)), Constant(Int(8, 8, Signed))) ==
      Constant::bottom(8, Signed));

  BOOST_CHECK(
      shl_no_wrap(Constant(Int(1, 8, Signed)), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(1, 8, Signed)), Constant::top(8, Signed)) ==
      Constant::top(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(1, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(2, 8, Signed)));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(1, 8, Signed)), Constant(Int(2, 8, Signed))) ==
      Constant(Int(4, 8, Signed)));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(1, 8, Signed)), Constant(Int(6, 8, Signed))) ==
      Constant(Int(64, 8, Signed)));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(1, 8, Signed)), Constant(Int(7, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(1, 8, Signed)), Constant(Int(8, 8, Signed))) ==
      Constant::bottom(8, Signed));

  BOOST_CHECK(
      shl_no_wrap(Constant(Int(127, 8, Signed)), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(127, 8, Signed)), Constant::top(8, Signed)) ==
      Constant::top(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(127, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(127, 8, Signed)), Constant(Int(2, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(127, 8, Signed)), Constant(Int(6, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(127, 8, Signed)), Constant(Int(7, 8, Signed))) ==
      Constant::bottom(8, Signed));

  BOOST_CHECK(shl_no_wrap(Constant(Int(-128, 8, Signed)),
                          Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(-128, 8, Signed)), Constant::top(8, Signed)) ==
      Constant::top(8, Signed));
  BOOST_CHECK(shl_no_wrap(Constant(Int(-128, 8, Signed)),
                          Constant(Int(1, 8, Signed))) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Constant(Int(-128, 8, Signed)),
                          Constant(Int(2, 8, Signed))) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Constant(Int(-128, 8, Signed)),
                          Constant(Int(6, 8, Signed))) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Constant(Int(-128, 8, Signed)),
                          Constant(Int(7, 8, Signed))) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Constant(Int(-128, 8, Signed)),
                          Constant(Int(8, 8, Signed))) ==
              Constant::bottom(8, Signed));

  BOOST_CHECK(shl_no_wrap(Constant(Int(0, 8, Unsigned)),
                          Constant::bottom(8, Unsigned)) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(
      shl_no_wrap(Constant(Int(0, 8, Unsigned)), Constant::top(8, Unsigned)) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Constant(Int(0, 8, Unsigned)),
                          Constant(Int(0, 8, Unsigned))) ==
              Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Constant(Int(0, 8, Unsigned)),
                          Constant(Int(1, 8, Unsigned))) ==
              Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Constant(Int(0, 8, Unsigned)),
                          Constant(Int(2, 8, Unsigned))) ==
              Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Constant(Int(0, 8, Unsigned)),
                          Constant(Int(8, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));

  BOOST_CHECK(shl_no_wrap(Constant(Int(1, 8, Unsigned)),
                          Constant::bottom(8, Unsigned)) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Constant(Int(1, 8, Unsigned)),
                          Constant(Int(1, 8, Unsigned))) ==
              Constant(Int(2, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Constant(Int(1, 8, Unsigned)),
                          Constant(Int(2, 8, Unsigned))) ==
              Constant(Int(4, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Constant(Int(1, 8, Unsigned)),
                          Constant(Int(6, 8, Unsigned))) ==
              Constant(Int(64, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Constant(Int(1, 8, Unsigned)),
                          Constant(Int(7, 8, Unsigned))) ==
              Constant(Int(128, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Constant(Int(1, 8, Unsigned)),
                          Constant(Int(8, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));

  BOOST_CHECK(shl_no_wrap(Constant(Int(127, 8, Unsigned)),
                          Constant::bottom(8, Unsigned)) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Constant(Int(127, 8, Unsigned)),
                          Constant::top(8, Unsigned)) ==
              Constant::top(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Constant(Int(127, 8, Unsigned)),
                          Constant(Int(1, 8, Unsigned))) ==
              Constant(Int(254, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Constant(Int(127, 8, Unsigned)),
                          Constant(Int(2, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Constant(Int(127, 8, Unsigned)),
                          Constant(Int(6, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Constant(Int(127, 8, Unsigned)),
                          Constant(Int(7, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));

  BOOST_CHECK(shl_no_wrap(Constant(Int(128, 8, Unsigned)),
                          Constant::bottom(8, Unsigned)) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Constant(Int(128, 8, Unsigned)),
                          Constant::top(8, Unsigned)) ==
              Constant::top(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Constant(Int(128, 8, Unsigned)),
                          Constant(Int(1, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Constant(Int(128, 8, Unsigned)),
                          Constant(Int(2, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Constant(Int(128, 8, Unsigned)),
                          Constant(Int(6, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Constant(Int(128, 8, Unsigned)),
                          Constant(Int(7, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_lshr) {
  BOOST_CHECK(lshr(Constant::top(8, Signed), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(lshr(Constant::top(8, Signed), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(lshr(Constant::bottom(8, Signed), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(lshr(Constant::bottom(8, Signed), Constant::top(8, Signed)) ==
              Constant::bottom(8, Signed));

  BOOST_CHECK(lshr(Constant(Int(0, 8, Signed)), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(lshr(Constant(Int(0, 8, Signed)), Constant::top(8, Signed)) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(lshr(Constant(Int(0, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(lshr(Constant(Int(0, 8, Signed)), Constant(Int(7, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(lshr(Constant(Int(0, 8, Signed)), Constant(Int(8, 8, Signed))) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(lshr(Constant(Int(0, 8, Signed)), Constant(Int(-1, 8, Signed))) ==
              Constant::bottom(8, Signed));

  BOOST_CHECK(lshr(Constant(Int(1, 8, Signed)), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(lshr(Constant(Int(1, 8, Signed)), Constant(Int(0, 8, Signed))) ==
              Constant(Int(1, 8, Signed)));
  BOOST_CHECK(lshr(Constant(Int(1, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(lshr(Constant(Int(1, 8, Signed)), Constant(Int(2, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(lshr(Constant(Int(1, 8, Signed)), Constant(Int(6, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(lshr(Constant(Int(1, 8, Signed)), Constant(Int(7, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(lshr(Constant(Int(1, 8, Signed)), Constant(Int(8, 8, Signed))) ==
              Constant::bottom(8, Signed));

  BOOST_CHECK(lshr(Constant(Int(127, 8, Signed)),
                   Constant::bottom(8, Signed)) == Constant::bottom(8, Signed));
  BOOST_CHECK(
      lshr(Constant(Int(127, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(63, 8, Signed)));
  BOOST_CHECK(
      lshr(Constant(Int(127, 8, Signed)), Constant(Int(2, 8, Signed))) ==
      Constant(Int(31, 8, Signed)));
  BOOST_CHECK(lshr(Constant(Int(127, 8, Signed)),
                   Constant(Int(6, 8, Signed))) == Constant(Int(1, 8, Signed)));
  BOOST_CHECK(lshr(Constant(Int(127, 8, Signed)),
                   Constant(Int(7, 8, Signed))) == Constant(Int(0, 8, Signed)));

  BOOST_CHECK(lshr(Constant(Int(-128, 8, Signed)),
                   Constant::bottom(8, Signed)) == Constant::bottom(8, Signed));
  BOOST_CHECK(lshr(Constant(Int(-128, 8, Signed)), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(
      lshr(Constant(Int(-128, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(64, 8, Signed)));
  BOOST_CHECK(
      lshr(Constant(Int(-128, 8, Signed)), Constant(Int(2, 8, Signed))) ==
      Constant(Int(32, 8, Signed)));
  BOOST_CHECK(lshr(Constant(Int(-128, 8, Signed)),
                   Constant(Int(6, 8, Signed))) == Constant(Int(2, 8, Signed)));
  BOOST_CHECK(lshr(Constant(Int(-128, 8, Signed)),
                   Constant(Int(7, 8, Signed))) == Constant(Int(1, 8, Signed)));

  BOOST_CHECK(
      lshr(Constant(Int(0, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(lshr(Constant(Int(0, 8, Unsigned)), Constant::top(8, Unsigned)) ==
              Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Constant(Int(0, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Constant(Int(1, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Constant(Int(255, 8, Unsigned)), Constant(Int(0, 8, Unsigned))) ==
      Constant(Int(255, 8, Unsigned)));

  BOOST_CHECK(
      lshr(Constant(Int(1, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(
      lshr(Constant(Int(1, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Constant(Int(1, 8, Unsigned)), Constant(Int(2, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Constant(Int(1, 8, Unsigned)), Constant(Int(6, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Constant(Int(1, 8, Unsigned)), Constant(Int(7, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Constant(Int(1, 8, Unsigned)), Constant(Int(8, 8, Unsigned))) ==
      Constant::bottom(8, Unsigned));

  BOOST_CHECK(
      lshr(Constant(Int(127, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(
      lshr(Constant(Int(127, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(63, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Constant(Int(127, 8, Unsigned)), Constant(Int(2, 8, Unsigned))) ==
      Constant(Int(31, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Constant(Int(127, 8, Unsigned)), Constant(Int(6, 8, Unsigned))) ==
      Constant(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Constant(Int(127, 8, Unsigned)), Constant(Int(7, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));

  BOOST_CHECK(
      lshr(Constant(Int(128, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(
      lshr(Constant(Int(128, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(64, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Constant(Int(128, 8, Unsigned)), Constant(Int(2, 8, Unsigned))) ==
      Constant(Int(32, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Constant(Int(128, 8, Unsigned)), Constant(Int(6, 8, Unsigned))) ==
      Constant(Int(2, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Constant(Int(128, 8, Unsigned)), Constant(Int(7, 8, Unsigned))) ==
      Constant(Int(1, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_lshr_exact) {
  BOOST_CHECK(
      lshr_exact(Constant::top(8, Signed), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(lshr_exact(Constant::top(8, Signed), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(
      lshr_exact(Constant::bottom(8, Signed), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Constant::bottom(8, Signed), Constant::top(8, Signed)) ==
      Constant::bottom(8, Signed));

  BOOST_CHECK(
      lshr_exact(Constant(Int(0, 8, Signed)), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Constant(Int(0, 8, Signed)), Constant::top(8, Signed)) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Constant(Int(0, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Constant(Int(0, 8, Signed)), Constant(Int(7, 8, Signed))) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Constant(Int(0, 8, Signed)), Constant(Int(8, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Constant(Int(0, 8, Signed)), Constant(Int(-1, 8, Signed))) ==
      Constant::bottom(8, Signed));

  BOOST_CHECK(
      lshr_exact(Constant(Int(1, 8, Signed)), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Constant(Int(1, 8, Signed)), Constant(Int(0, 8, Signed))) ==
      Constant(Int(1, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Constant(Int(1, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Constant(Int(1, 8, Signed)), Constant(Int(2, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Constant(Int(1, 8, Signed)), Constant(Int(6, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Constant(Int(1, 8, Signed)), Constant(Int(7, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Constant(Int(1, 8, Signed)), Constant(Int(8, 8, Signed))) ==
      Constant::bottom(8, Signed));

  BOOST_CHECK(
      lshr_exact(Constant(Int(127, 8, Signed)), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Constant(Int(127, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Constant(Int(127, 8, Signed)), Constant(Int(2, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Constant(Int(127, 8, Signed)), Constant(Int(6, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Constant(Int(127, 8, Signed)), Constant(Int(7, 8, Signed))) ==
      Constant::bottom(8, Signed));

  BOOST_CHECK(
      lshr_exact(Constant(Int(-128, 8, Signed)), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(lshr_exact(Constant(Int(-128, 8, Signed)),
                         Constant::top(8, Signed)) == Constant::top(8, Signed));
  BOOST_CHECK(
      lshr_exact(Constant(Int(-128, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(64, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Constant(Int(-128, 8, Signed)), Constant(Int(2, 8, Signed))) ==
      Constant(Int(32, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Constant(Int(-128, 8, Signed)), Constant(Int(6, 8, Signed))) ==
      Constant(Int(2, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Constant(Int(-128, 8, Signed)), Constant(Int(7, 8, Signed))) ==
      Constant(Int(1, 8, Signed)));

  BOOST_CHECK(lshr_exact(Constant(Int(0, 8, Unsigned)),
                         Constant::bottom(8, Unsigned)) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(
      lshr_exact(Constant(Int(0, 8, Unsigned)), Constant::top(8, Unsigned)) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Constant(Int(0, 8, Unsigned)),
                         Constant(Int(1, 8, Unsigned))) ==
              Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Constant(Int(1, 8, Unsigned)),
                         Constant(Int(1, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Constant(Int(255, 8, Unsigned)),
                         Constant(Int(0, 8, Unsigned))) ==
              Constant(Int(255, 8, Unsigned)));

  BOOST_CHECK(lshr_exact(Constant(Int(1, 8, Unsigned)),
                         Constant::bottom(8, Unsigned)) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Constant(Int(1, 8, Unsigned)),
                         Constant(Int(1, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Constant(Int(1, 8, Unsigned)),
                         Constant(Int(2, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Constant(Int(1, 8, Unsigned)),
                         Constant(Int(6, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Constant(Int(1, 8, Unsigned)),
                         Constant(Int(7, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Constant(Int(1, 8, Unsigned)),
                         Constant(Int(8, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));

  BOOST_CHECK(lshr_exact(Constant(Int(127, 8, Unsigned)),
                         Constant::bottom(8, Unsigned)) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Constant(Int(127, 8, Unsigned)),
                         Constant(Int(1, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Constant(Int(127, 8, Unsigned)),
                         Constant(Int(2, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Constant(Int(127, 8, Unsigned)),
                         Constant(Int(6, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Constant(Int(127, 8, Unsigned)),
                         Constant(Int(7, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));

  BOOST_CHECK(lshr_exact(Constant(Int(128, 8, Unsigned)),
                         Constant::bottom(8, Unsigned)) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Constant(Int(128, 8, Unsigned)),
                         Constant(Int(1, 8, Unsigned))) ==
              Constant(Int(64, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Constant(Int(128, 8, Unsigned)),
                         Constant(Int(2, 8, Unsigned))) ==
              Constant(Int(32, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Constant(Int(128, 8, Unsigned)),
                         Constant(Int(6, 8, Unsigned))) ==
              Constant(Int(2, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Constant(Int(128, 8, Unsigned)),
                         Constant(Int(7, 8, Unsigned))) ==
              Constant(Int(1, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_ashr) {
  BOOST_CHECK(ashr(Constant::top(8, Signed), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(ashr(Constant::top(8, Signed), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(ashr(Constant::bottom(8, Signed), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(ashr(Constant::bottom(8, Signed), Constant::top(8, Signed)) ==
              Constant::bottom(8, Signed));

  BOOST_CHECK(ashr(Constant(Int(0, 8, Signed)), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(ashr(Constant(Int(0, 8, Signed)), Constant::top(8, Signed)) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(ashr(Constant(Int(0, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(ashr(Constant(Int(0, 8, Signed)), Constant(Int(7, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(ashr(Constant(Int(0, 8, Signed)), Constant(Int(8, 8, Signed))) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(ashr(Constant(Int(0, 8, Signed)), Constant(Int(-1, 8, Signed))) ==
              Constant::bottom(8, Signed));

  BOOST_CHECK(ashr(Constant(Int(1, 8, Signed)), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(ashr(Constant(Int(1, 8, Signed)), Constant(Int(0, 8, Signed))) ==
              Constant(Int(1, 8, Signed)));
  BOOST_CHECK(ashr(Constant(Int(1, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(ashr(Constant(Int(1, 8, Signed)), Constant(Int(2, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(ashr(Constant(Int(1, 8, Signed)), Constant(Int(6, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(ashr(Constant(Int(1, 8, Signed)), Constant(Int(7, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(ashr(Constant(Int(1, 8, Signed)), Constant(Int(8, 8, Signed))) ==
              Constant::bottom(8, Signed));

  BOOST_CHECK(ashr(Constant(Int(127, 8, Signed)),
                   Constant::bottom(8, Signed)) == Constant::bottom(8, Signed));
  BOOST_CHECK(
      ashr(Constant(Int(127, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(63, 8, Signed)));
  BOOST_CHECK(
      ashr(Constant(Int(127, 8, Signed)), Constant(Int(2, 8, Signed))) ==
      Constant(Int(31, 8, Signed)));
  BOOST_CHECK(ashr(Constant(Int(127, 8, Signed)),
                   Constant(Int(6, 8, Signed))) == Constant(Int(1, 8, Signed)));
  BOOST_CHECK(ashr(Constant(Int(127, 8, Signed)),
                   Constant(Int(7, 8, Signed))) == Constant(Int(0, 8, Signed)));

  BOOST_CHECK(ashr(Constant(Int(-128, 8, Signed)),
                   Constant::bottom(8, Signed)) == Constant::bottom(8, Signed));
  BOOST_CHECK(ashr(Constant(Int(-128, 8, Signed)), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(
      ashr(Constant(Int(-128, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(-64, 8, Signed)));
  BOOST_CHECK(
      ashr(Constant(Int(-128, 8, Signed)), Constant(Int(2, 8, Signed))) ==
      Constant(Int(-32, 8, Signed)));
  BOOST_CHECK(
      ashr(Constant(Int(-128, 8, Signed)), Constant(Int(6, 8, Signed))) ==
      Constant(Int(-2, 8, Signed)));
  BOOST_CHECK(
      ashr(Constant(Int(-128, 8, Signed)), Constant(Int(7, 8, Signed))) ==
      Constant(Int(-1, 8, Signed)));

  BOOST_CHECK(
      ashr(Constant(Int(0, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(ashr(Constant(Int(0, 8, Unsigned)), Constant::top(8, Unsigned)) ==
              Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Constant(Int(0, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Constant(Int(1, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Constant(Int(255, 8, Unsigned)), Constant(Int(0, 8, Unsigned))) ==
      Constant(Int(255, 8, Unsigned)));

  BOOST_CHECK(
      ashr(Constant(Int(1, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(
      ashr(Constant(Int(1, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Constant(Int(1, 8, Unsigned)), Constant(Int(2, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Constant(Int(1, 8, Unsigned)), Constant(Int(6, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Constant(Int(1, 8, Unsigned)), Constant(Int(7, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Constant(Int(1, 8, Unsigned)), Constant(Int(8, 8, Unsigned))) ==
      Constant::bottom(8, Unsigned));

  BOOST_CHECK(
      ashr(Constant(Int(127, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(
      ashr(Constant(Int(127, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(63, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Constant(Int(127, 8, Unsigned)), Constant(Int(2, 8, Unsigned))) ==
      Constant(Int(31, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Constant(Int(127, 8, Unsigned)), Constant(Int(6, 8, Unsigned))) ==
      Constant(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Constant(Int(127, 8, Unsigned)), Constant(Int(7, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));

  BOOST_CHECK(
      ashr(Constant(Int(128, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(
      ashr(Constant(Int(128, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(192, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Constant(Int(128, 8, Unsigned)), Constant(Int(2, 8, Unsigned))) ==
      Constant(Int(224, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Constant(Int(128, 8, Unsigned)), Constant(Int(6, 8, Unsigned))) ==
      Constant(Int(254, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Constant(Int(128, 8, Unsigned)), Constant(Int(7, 8, Unsigned))) ==
      Constant(Int(255, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_ashr_exact) {
  BOOST_CHECK(
      ashr_exact(Constant::top(8, Signed), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(ashr_exact(Constant::top(8, Signed), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(
      ashr_exact(Constant::bottom(8, Signed), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Constant::bottom(8, Signed), Constant::top(8, Signed)) ==
      Constant::bottom(8, Signed));

  BOOST_CHECK(
      ashr_exact(Constant(Int(0, 8, Signed)), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Constant(Int(0, 8, Signed)), Constant::top(8, Signed)) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Constant(Int(0, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Constant(Int(0, 8, Signed)), Constant(Int(7, 8, Signed))) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Constant(Int(0, 8, Signed)), Constant(Int(8, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Constant(Int(0, 8, Signed)), Constant(Int(-1, 8, Signed))) ==
      Constant::bottom(8, Signed));

  BOOST_CHECK(
      ashr_exact(Constant(Int(1, 8, Signed)), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Constant(Int(1, 8, Signed)), Constant(Int(0, 8, Signed))) ==
      Constant(Int(1, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Constant(Int(1, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Constant(Int(1, 8, Signed)), Constant(Int(2, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Constant(Int(1, 8, Signed)), Constant(Int(6, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Constant(Int(1, 8, Signed)), Constant(Int(7, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Constant(Int(1, 8, Signed)), Constant(Int(8, 8, Signed))) ==
      Constant::bottom(8, Signed));

  BOOST_CHECK(
      ashr_exact(Constant(Int(127, 8, Signed)), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Constant(Int(127, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Constant(Int(127, 8, Signed)), Constant(Int(2, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Constant(Int(127, 8, Signed)), Constant(Int(6, 8, Signed))) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Constant(Int(127, 8, Signed)), Constant(Int(7, 8, Signed))) ==
      Constant::bottom(8, Signed));

  BOOST_CHECK(
      ashr_exact(Constant(Int(-128, 8, Signed)), Constant::bottom(8, Signed)) ==
      Constant::bottom(8, Signed));
  BOOST_CHECK(ashr_exact(Constant(Int(-128, 8, Signed)),
                         Constant::top(8, Signed)) == Constant::top(8, Signed));
  BOOST_CHECK(
      ashr_exact(Constant(Int(-128, 8, Signed)), Constant(Int(1, 8, Signed))) ==
      Constant(Int(-64, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Constant(Int(-128, 8, Signed)), Constant(Int(2, 8, Signed))) ==
      Constant(Int(-32, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Constant(Int(-128, 8, Signed)), Constant(Int(6, 8, Signed))) ==
      Constant(Int(-2, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Constant(Int(-128, 8, Signed)), Constant(Int(7, 8, Signed))) ==
      Constant(Int(-1, 8, Signed)));

  BOOST_CHECK(ashr_exact(Constant(Int(0, 8, Unsigned)),
                         Constant::bottom(8, Unsigned)) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(
      ashr_exact(Constant(Int(0, 8, Unsigned)), Constant::top(8, Unsigned)) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Constant(Int(0, 8, Unsigned)),
                         Constant(Int(1, 8, Unsigned))) ==
              Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Constant(Int(1, 8, Unsigned)),
                         Constant(Int(1, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Constant(Int(255, 8, Unsigned)),
                         Constant(Int(0, 8, Unsigned))) ==
              Constant(Int(255, 8, Unsigned)));

  BOOST_CHECK(ashr_exact(Constant(Int(1, 8, Unsigned)),
                         Constant::bottom(8, Unsigned)) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Constant(Int(1, 8, Unsigned)),
                         Constant(Int(1, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Constant(Int(1, 8, Unsigned)),
                         Constant(Int(2, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Constant(Int(1, 8, Unsigned)),
                         Constant(Int(6, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Constant(Int(1, 8, Unsigned)),
                         Constant(Int(7, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Constant(Int(1, 8, Unsigned)),
                         Constant(Int(8, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));

  BOOST_CHECK(ashr_exact(Constant(Int(127, 8, Unsigned)),
                         Constant::bottom(8, Unsigned)) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Constant(Int(127, 8, Unsigned)),
                         Constant(Int(1, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Constant(Int(127, 8, Unsigned)),
                         Constant(Int(2, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Constant(Int(127, 8, Unsigned)),
                         Constant(Int(6, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Constant(Int(127, 8, Unsigned)),
                         Constant(Int(7, 8, Unsigned))) ==
              Constant::bottom(8, Unsigned));

  BOOST_CHECK(ashr_exact(Constant(Int(128, 8, Unsigned)),
                         Constant::bottom(8, Unsigned)) ==
              Constant::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Constant(Int(128, 8, Unsigned)),
                         Constant(Int(1, 8, Unsigned))) ==
              Constant(Int(192, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Constant(Int(128, 8, Unsigned)),
                         Constant(Int(2, 8, Unsigned))) ==
              Constant(Int(224, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Constant(Int(128, 8, Unsigned)),
                         Constant(Int(6, 8, Unsigned))) ==
              Constant(Int(254, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Constant(Int(128, 8, Unsigned)),
                         Constant(Int(7, 8, Unsigned))) ==
              Constant(Int(255, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_and) {
  BOOST_CHECK(and_(Constant::top(8, Signed), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(and_(Constant::top(8, Signed), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(and_(Constant::bottom(8, Signed), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(and_(Constant::bottom(8, Signed), Constant::top(8, Signed)) ==
              Constant::bottom(8, Signed));

  BOOST_CHECK(and_(Constant(Int(0, 8, Signed)), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(and_(Constant(Int(0, 8, Signed)), Constant::top(8, Signed)) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(and_(Constant(Int(0, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(and_(Constant(Int(1, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(1, 8, Signed)));
  BOOST_CHECK(
      and_(Constant(Int(-128, 8, Signed)), Constant(Int(127, 8, Signed))) ==
      Constant(Int(0, 8, Signed)));
  BOOST_CHECK(and_(Constant(Int(5, 8, Signed)), Constant(Int(10, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(and_(Constant(Int(5, 8, Signed)), Constant(Int(11, 8, Signed))) ==
              Constant(Int(1, 8, Signed)));
  BOOST_CHECK(
      and_(Constant(Int(-1, 8, Signed)), Constant(Int(-2, 8, Signed))) ==
      Constant(Int(-2, 8, Signed)));

  BOOST_CHECK(
      and_(Constant(Int(0, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(and_(Constant(Int(0, 8, Unsigned)), Constant::top(8, Unsigned)) ==
              Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      and_(Constant(Int(0, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      and_(Constant(Int(1, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      and_(Constant(Int(255, 8, Unsigned)), Constant(Int(0, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      and_(Constant(Int(5, 8, Unsigned)), Constant(Int(10, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_or) {
  BOOST_CHECK(or_(Constant::top(8, Signed), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(or_(Constant::top(8, Signed), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(or_(Constant::bottom(8, Signed), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(or_(Constant::bottom(8, Signed), Constant::top(8, Signed)) ==
              Constant::bottom(8, Signed));

  BOOST_CHECK(or_(Constant(Int(0, 8, Signed)), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(or_(Constant(Int(0, 8, Signed)), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(or_(Constant(Int(0, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(1, 8, Signed)));
  BOOST_CHECK(or_(Constant(Int(1, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(1, 8, Signed)));
  BOOST_CHECK(
      or_(Constant(Int(-128, 8, Signed)), Constant(Int(127, 8, Signed))) ==
      Constant(Int(-1, 8, Signed)));
  BOOST_CHECK(or_(Constant(Int(5, 8, Signed)), Constant(Int(10, 8, Signed))) ==
              Constant(Int(15, 8, Signed)));
  BOOST_CHECK(or_(Constant(Int(5, 8, Signed)), Constant(Int(11, 8, Signed))) ==
              Constant(Int(15, 8, Signed)));
  BOOST_CHECK(or_(Constant(Int(-1, 8, Signed)), Constant(Int(-2, 8, Signed))) ==
              Constant(Int(-1, 8, Signed)));

  BOOST_CHECK(
      or_(Constant(Int(0, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(or_(Constant(Int(0, 8, Unsigned)), Constant::top(8, Unsigned)) ==
              Constant::top(8, Unsigned));
  BOOST_CHECK(
      or_(Constant(Int(0, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      or_(Constant(Int(1, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      or_(Constant(Int(255, 8, Unsigned)), Constant(Int(0, 8, Unsigned))) ==
      Constant(Int(255, 8, Unsigned)));
  BOOST_CHECK(
      or_(Constant(Int(5, 8, Unsigned)), Constant(Int(10, 8, Unsigned))) ==
      Constant(Int(15, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_xor) {
  BOOST_CHECK(xor_(Constant::top(8, Signed), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(xor_(Constant::top(8, Signed), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(xor_(Constant::bottom(8, Signed), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(xor_(Constant::bottom(8, Signed), Constant::top(8, Signed)) ==
              Constant::bottom(8, Signed));

  BOOST_CHECK(xor_(Constant(Int(0, 8, Signed)), Constant::bottom(8, Signed)) ==
              Constant::bottom(8, Signed));
  BOOST_CHECK(xor_(Constant(Int(0, 8, Signed)), Constant::top(8, Signed)) ==
              Constant::top(8, Signed));
  BOOST_CHECK(xor_(Constant(Int(0, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(1, 8, Signed)));
  BOOST_CHECK(xor_(Constant(Int(1, 8, Signed)), Constant(Int(1, 8, Signed))) ==
              Constant(Int(0, 8, Signed)));
  BOOST_CHECK(
      xor_(Constant(Int(-128, 8, Signed)), Constant(Int(127, 8, Signed))) ==
      Constant(Int(-1, 8, Signed)));
  BOOST_CHECK(xor_(Constant(Int(5, 8, Signed)), Constant(Int(10, 8, Signed))) ==
              Constant(Int(15, 8, Signed)));
  BOOST_CHECK(xor_(Constant(Int(5, 8, Signed)), Constant(Int(11, 8, Signed))) ==
              Constant(Int(14, 8, Signed)));
  BOOST_CHECK(
      xor_(Constant(Int(-1, 8, Signed)), Constant(Int(-2, 8, Signed))) ==
      Constant(Int(1, 8, Signed)));

  BOOST_CHECK(
      xor_(Constant(Int(0, 8, Unsigned)), Constant::bottom(8, Unsigned)) ==
      Constant::bottom(8, Unsigned));
  BOOST_CHECK(xor_(Constant(Int(0, 8, Unsigned)), Constant::top(8, Unsigned)) ==
              Constant::top(8, Unsigned));
  BOOST_CHECK(
      xor_(Constant(Int(0, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      xor_(Constant(Int(1, 8, Unsigned)), Constant(Int(1, 8, Unsigned))) ==
      Constant(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      xor_(Constant(Int(255, 8, Unsigned)), Constant(Int(0, 8, Unsigned))) ==
      Constant(Int(255, 8, Unsigned)));
  BOOST_CHECK(
      xor_(Constant(Int(5, 8, Unsigned)), Constant(Int(10, 8, Unsigned))) ==
      Constant(Int(15, 8, Unsigned)));
}
