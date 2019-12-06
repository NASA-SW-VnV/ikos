/*******************************************************************************
 *
 * Tests for machine_int::Interval
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

#define BOOST_TEST_MODULE test_machine_integer_interval
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/number/machine_int.hpp>
#include <ikos/core/value/machine_int/interval.hpp>

using Int = ikos::core::MachineInt;
using Interval = ikos::core::machine_int::Interval;
using ikos::core::Signed;
using ikos::core::Unsigned;
using ZBound = ikos::core::Bound< ikos::core::ZNumber >;
using ZInterval = ikos::core::numeric::ZInterval;

BOOST_AUTO_TEST_CASE(test_constructors) {
  BOOST_CHECK(Interval(Int(0, 8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Interval(Int(1, 8, Unsigned)) ==
              Interval(Int(1, 8, Unsigned), Int(1, 8, Unsigned)));

  BOOST_CHECK(Interval(Int(1, 8, Signed), Int(2, 8, Signed)) ==
              Interval(Int(1, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned)) ==
              Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned)));

  BOOST_CHECK(Interval::top(1, Signed) ==
              Interval(Int(-1, 1, Signed), Int(0, 1, Signed)));
  BOOST_CHECK(Interval::top(8, Signed) ==
              Interval(Int(-128, 8, Signed), Int(127, 8, Signed)));

  BOOST_CHECK(Interval::top(1, Unsigned) ==
              Interval(Int(0, 1, Unsigned), Int(1, 1, Unsigned)));
  BOOST_CHECK(Interval::top(8, Unsigned) ==
              Interval(Int(0, 8, Unsigned), Int(255, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_bit_width) {
  BOOST_CHECK(Interval(Int(0, 4, Signed)).bit_width() == 4);
  BOOST_CHECK(Interval(Int(0, 4, Unsigned)).bit_width() == 4);
  BOOST_CHECK(Interval(Int(0, 8, Signed)).bit_width() == 8);
  BOOST_CHECK(Interval(Int(0, 8, Unsigned)).bit_width() == 8);
}

BOOST_AUTO_TEST_CASE(test_sign) {
  BOOST_CHECK(Interval(Int(0, 4, Signed)).sign() == Signed);
  BOOST_CHECK(Interval(Int(0, 4, Unsigned)).sign() == Unsigned);
  BOOST_CHECK(Interval(Int(0, 8, Signed)).sign() == Signed);
  BOOST_CHECK(Interval(Int(0, 8, Unsigned)).sign() == Unsigned);
}

BOOST_AUTO_TEST_CASE(test_lb_and_ub) {
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed)).lb() ==
              Int(0, 8, Signed));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed)).ub() ==
              Int(1, 8, Signed));
  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)).lb() ==
              Int(0, 8, Unsigned));
  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)).ub() ==
              Int(1, 8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_bottom) {
  BOOST_CHECK(Interval::bottom(8, Signed).is_bottom());
  BOOST_CHECK(!Interval::top(8, Signed).is_bottom());
  BOOST_CHECK(!Interval(Int(0, 8, Signed), Int(1, 8, Signed)).is_bottom());
  BOOST_CHECK(!Interval(Int(-128, 8, Signed), Int(127, 8, Signed)).is_bottom());
  BOOST_CHECK(Interval(Int(1, 8, Signed), Int(0, 8, Signed)).is_bottom());
  BOOST_CHECK(Interval::bottom(8, Unsigned).is_bottom());
  BOOST_CHECK(!Interval::top(8, Unsigned).is_bottom());
  BOOST_CHECK(!Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)).is_bottom());
  BOOST_CHECK(Interval(Int(1, 8, Unsigned), Int(0, 8, Unsigned)).is_bottom());
  BOOST_CHECK(
      !Interval(Int(0, 8, Unsigned), Int(255, 8, Unsigned)).is_bottom());
}

BOOST_AUTO_TEST_CASE(test_top) {
  BOOST_CHECK(!Interval::bottom(8, Signed).is_top());
  BOOST_CHECK(Interval::top(8, Signed).is_top());
  BOOST_CHECK(!Interval(Int(0, 8, Signed), Int(1, 8, Signed)).is_top());
  BOOST_CHECK(!Interval(Int(1, 8, Signed), Int(0, 8, Signed)).is_top());
  BOOST_CHECK(Interval(Int(-128, 8, Signed), Int(127, 8, Signed)).is_top());
  BOOST_CHECK(!Interval::bottom(8, Unsigned).is_top());
  BOOST_CHECK(Interval::top(8, Unsigned).is_top());
  BOOST_CHECK(!Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)).is_top());
  BOOST_CHECK(!Interval(Int(1, 8, Unsigned), Int(0, 8, Unsigned)).is_top());
  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(255, 8, Unsigned)).is_top());
}

BOOST_AUTO_TEST_CASE(test_set_to_bottom) {
  Interval i(Int(0, 8, Signed));
  i.set_to_bottom();
  BOOST_CHECK(i.is_bottom());
  BOOST_CHECK(!i.is_top());
}

BOOST_AUTO_TEST_CASE(test_set_to_top) {
  Interval i(Int(0, 8, Signed));
  i.set_to_top();
  BOOST_CHECK(!i.is_bottom());
  BOOST_CHECK(i.is_top());
}

BOOST_AUTO_TEST_CASE(test_leq) {
  BOOST_CHECK(Interval::bottom(8, Signed).leq(Interval::bottom(8, Signed)));
  BOOST_CHECK(Interval::bottom(8, Signed).leq(Interval::top(8, Signed)));

  BOOST_CHECK(!Interval::top(8, Signed).leq(Interval::bottom(8, Signed)));
  BOOST_CHECK(Interval::top(8, Signed).leq(Interval::top(8, Signed)));

  BOOST_CHECK(!Interval(Int(0, 8, Signed)).leq(Interval::bottom(8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed)).leq(Interval::top(8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed))
                  .leq(Interval(Int(-1, 8, Signed), Int(1, 8, Signed))));
  BOOST_CHECK(!Interval(Int(0, 8, Signed))
                   .leq(Interval(Int(1, 8, Signed), Int(2, 8, Signed))));

  BOOST_CHECK(
      !Interval(Int(0, 8, Unsigned)).leq(Interval::bottom(8, Unsigned)));
  BOOST_CHECK(Interval(Int(0, 8, Unsigned)).leq(Interval::top(8, Unsigned)));
  BOOST_CHECK(Interval(Int(1, 8, Unsigned))
                  .leq(Interval(Int(0, 8, Unsigned), Int(2, 8, Unsigned))));
  BOOST_CHECK(!Interval(Int(1, 8, Unsigned))
                   .leq(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned))));

  BOOST_CHECK(!Interval(Int(-4, 8, Signed), Int(-2, 8, Signed))
                   .leq(Interval::bottom(8, Signed)));
  BOOST_CHECK(Interval(Int(-4, 8, Signed), Int(-2, 8, Signed))
                  .leq(Interval::top(8, Signed)));
  BOOST_CHECK(Interval(Int(-4, 8, Signed), Int(-2, 8, Signed))
                  .leq(Interval(Int(-5, 8, Signed), Int(0, 8, Signed))));
  BOOST_CHECK(!Interval(Int(-4, 8, Signed), Int(-2, 8, Signed))
                   .leq(Interval(Int(-3, 8, Signed), Int(0, 8, Signed))));
  BOOST_CHECK(!Interval(Int(-4, 8, Signed), Int(-2, 8, Signed))
                   .leq(Interval(Int(-5, 8, Signed), Int(-3, 8, Signed))));

  BOOST_CHECK(!Interval(Int(100, 8, Unsigned), Int(200, 8, Unsigned))
                   .leq(Interval::bottom(8, Unsigned)));
  BOOST_CHECK(Interval(Int(100, 8, Unsigned), Int(200, 8, Unsigned))
                  .leq(Interval::top(8, Unsigned)));
  BOOST_CHECK(Interval(Int(100, 8, Unsigned), Int(200, 8, Unsigned))
                  .leq(Interval(Int(99, 8, Unsigned), Int(201, 8, Unsigned))));
  BOOST_CHECK(
      !Interval(Int(100, 8, Unsigned), Int(200, 8, Unsigned))
           .leq(Interval(Int(151, 8, Unsigned), Int(255, 8, Unsigned))));
  BOOST_CHECK(!Interval(Int(100, 8, Unsigned), Int(200, 8, Unsigned))
                   .leq(Interval(Int(42, 8, Unsigned), Int(101, 8, Unsigned))));
}

BOOST_AUTO_TEST_CASE(test_equals) {
  BOOST_CHECK(Interval::bottom(8, Signed).equals(Interval::bottom(8, Signed)));
  BOOST_CHECK(!Interval::bottom(8, Signed).equals(Interval::top(8, Signed)));

  BOOST_CHECK(!Interval::top(8, Signed).equals(Interval::bottom(8, Signed)));
  BOOST_CHECK(Interval::top(8, Signed).equals(Interval::top(8, Signed)));

  BOOST_CHECK(!Interval(Int(0, 8, Signed)).equals(Interval::bottom(8, Signed)));
  BOOST_CHECK(!Interval(Int(0, 8, Signed)).equals(Interval::top(8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed)).equals(Interval(Int(0, 8, Signed))));
  BOOST_CHECK(!Interval(Int(0, 8, Signed))
                   .equals(Interval(Int(-1, 8, Signed), Int(1, 8, Signed))));
  BOOST_CHECK(!Interval(Int(0, 8, Signed))
                   .equals(Interval(Int(1, 8, Signed), Int(2, 8, Signed))));

  BOOST_CHECK(
      !Interval(Int(0, 8, Unsigned)).equals(Interval::bottom(8, Unsigned)));
  BOOST_CHECK(
      !Interval(Int(0, 8, Unsigned)).equals(Interval::top(8, Unsigned)));
  BOOST_CHECK(!Interval(Int(1, 8, Unsigned))
                   .equals(Interval(Int(0, 8, Unsigned), Int(2, 8, Unsigned))));
  BOOST_CHECK(!Interval(Int(1, 8, Unsigned))
                   .equals(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned))));

  BOOST_CHECK(!Interval(Int(-4, 8, Signed), Int(-2, 8, Signed))
                   .equals(Interval::bottom(8, Signed)));
  BOOST_CHECK(!Interval(Int(-4, 8, Signed), Int(-2, 8, Signed))
                   .equals(Interval::top(8, Signed)));
  BOOST_CHECK(!Interval(Int(-4, 8, Signed), Int(-2, 8, Signed))
                   .equals(Interval(Int(-5, 8, Signed), Int(0, 8, Signed))));
  BOOST_CHECK(!Interval(Int(-4, 8, Signed), Int(-2, 8, Signed))
                   .equals(Interval(Int(-3, 8, Signed), Int(0, 8, Signed))));
  BOOST_CHECK(!Interval(Int(-4, 8, Signed), Int(-2, 8, Signed))
                   .equals(Interval(Int(-5, 8, Signed), Int(-3, 8, Signed))));
  BOOST_CHECK(Interval(Int(-4, 8, Signed), Int(-2, 8, Signed))
                  .equals(Interval(Int(-4, 8, Signed), Int(-2, 8, Signed))));

  BOOST_CHECK(!Interval(Int(100, 8, Unsigned), Int(200, 8, Unsigned))
                   .equals(Interval::bottom(8, Unsigned)));
  BOOST_CHECK(!Interval(Int(100, 8, Unsigned), Int(200, 8, Unsigned))
                   .equals(Interval::top(8, Unsigned)));
  BOOST_CHECK(
      !Interval(Int(100, 8, Unsigned), Int(200, 8, Unsigned))
           .equals(Interval(Int(99, 8, Unsigned), Int(201, 8, Unsigned))));
  BOOST_CHECK(
      !Interval(Int(100, 8, Unsigned), Int(200, 8, Unsigned))
           .equals(Interval(Int(151, 8, Unsigned), Int(255, 8, Unsigned))));
  BOOST_CHECK(
      !Interval(Int(100, 8, Unsigned), Int(200, 8, Unsigned))
           .equals(Interval(Int(42, 8, Unsigned), Int(101, 8, Unsigned))));
  BOOST_CHECK(
      Interval(Int(100, 8, Unsigned), Int(200, 8, Unsigned))
          .equals(Interval(Int(100, 8, Unsigned), Int(200, 8, Unsigned))));
}

BOOST_AUTO_TEST_CASE(test_join) {
  BOOST_CHECK(Interval::top(8, Signed).join(Interval::bottom(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(Interval::top(8, Signed).join(Interval::top(8, Signed)) ==
              Interval::top(8, Signed));

  BOOST_CHECK(Interval::bottom(8, Signed).join(Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval::bottom(8, Signed).join(Interval::top(8, Signed)) ==
              Interval::top(8, Signed));

  BOOST_CHECK(Interval(Int(0, 8, Signed)).join(Interval::bottom(8, Signed)) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed)).join(Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(Interval(Int(0, 8, Signed)).join(Interval(Int(1, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(Interval(Int(-1, 8, Signed)).join(Interval(Int(1, 8, Signed))) ==
              Interval(Int(-1, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(
      Interval(Int(-128, 8, Signed)).join(Interval(Int(-129, 8, Signed))) ==
      Interval::top(8, Signed));

  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                  .join(Interval::bottom(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                  .join(Interval::top(8, Signed)) == Interval::top(8, Signed));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                  .join(Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(-1, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                  .join(Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(2, 8, Signed)));

  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(127, 8, Signed))
                  .join(Interval::bottom(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(127, 8, Signed))
                  .join(Interval::top(8, Signed)) == Interval::top(8, Signed));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(127, 8, Signed))
                  .join(Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(-1, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(127, 8, Signed))
                  .join(Interval(Int(-128, 8, Signed))) ==
              Interval::top(8, Signed));

  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned)).join(Interval::bottom(8, Unsigned)) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(Interval(Int(0, 8, Unsigned)).join(Interval::top(8, Unsigned)) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned)).join(Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(
      Interval(Int(1, 8, Unsigned)).join(Interval(Int(2, 8, Unsigned))) ==
      Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(
      Interval(Int(255, 8, Unsigned)).join(Interval(Int(256, 8, Unsigned))) ==
      Interval::top(8, Unsigned));

  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned))
                  .join(Interval::bottom(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned))
                  .join(Interval::top(8, Unsigned)) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned))
                  .join(Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(1, 8, Unsigned), Int(3, 8, Unsigned)));
  BOOST_CHECK(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned))
                  .join(Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(2, 8, Unsigned), Int(4, 8, Unsigned)));

  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned))
                  .join(Interval::bottom(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned))
                  .join(Interval::top(8, Unsigned)) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned))
          .join(Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned))) ==
      Interval::top(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_widening) {
  BOOST_CHECK(Interval::top(8, Signed).widening(Interval::bottom(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(Interval::top(8, Signed).widening(Interval::top(8, Signed)) ==
              Interval::top(8, Signed));

  BOOST_CHECK(Interval::bottom(8, Signed).widening(
                  Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(Interval::bottom(8, Signed).widening(Interval::top(8, Signed)) ==
              Interval::top(8, Signed));

  BOOST_CHECK(
      Interval(Int(0, 8, Signed)).widening(Interval::bottom(8, Signed)) ==
      Interval(Int(0, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed)).widening(Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(
      Interval(Int(0, 8, Signed)).widening(Interval(Int(1, 8, Signed))) ==
      Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(
      Interval(Int(-1, 8, Signed)).widening(Interval(Int(-2, 8, Signed))) ==
      Interval(Int(-128, 8, Signed), Int(-1, 8, Signed)));

  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                  .widening(Interval::bottom(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                  .widening(Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                  .widening(Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(-128, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                  .widening(Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));

  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(127, 8, Signed))
                  .widening(Interval::bottom(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(127, 8, Signed))
                  .widening(Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(127, 8, Signed))
                  .widening(Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(127, 8, Signed))
                  .widening(Interval(Int(-128, 8, Signed))) ==
              Interval::top(8, Signed));

  BOOST_CHECK(
      Interval(Int(1, 8, Unsigned)).widening(Interval::bottom(8, Unsigned)) ==
      Interval(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned)).widening(Interval::top(8, Unsigned)) ==
      Interval::top(8, Unsigned));
  BOOST_CHECK(
      Interval(Int(1, 8, Unsigned)).widening(Interval(Int(2, 8, Unsigned))) ==
      Interval(Int(1, 8, Unsigned), Int(255, 8, Unsigned)));
  BOOST_CHECK(Interval(Int(255, 8, Unsigned))
                  .widening(Interval(Int(256, 8, Unsigned))) ==
              Interval::top(8, Unsigned));

  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned))
                  .widening(Interval::bottom(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned))
                  .widening(Interval::top(8, Unsigned)) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(
      Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned))
          .widening(Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned), Int(3, 8, Unsigned)));
  BOOST_CHECK(
      Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned))
          .widening(Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
      Interval(Int(2, 8, Unsigned), Int(255, 8, Unsigned)));

  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned))
                  .widening(Interval::bottom(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned))
                  .widening(Interval::top(8, Unsigned)) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned))
          .widening(Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned))) ==
      Interval::top(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_meet) {
  BOOST_CHECK(Interval::top(8, Signed).meet(Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval::top(8, Signed).meet(Interval::top(8, Signed)) ==
              Interval::top(8, Signed));

  BOOST_CHECK(Interval::bottom(8, Signed).meet(Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval::bottom(8, Signed).meet(Interval::top(8, Signed)) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(Interval(Int(0, 8, Signed)).meet(Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval(Int(0, 8, Signed)).meet(Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed)).meet(Interval(Int(1, 8, Signed))) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval(Int(-1, 8, Signed)).meet(Interval(Int(1, 8, Signed))) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                  .meet(Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                  .meet(Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                  .meet(Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                  .meet(Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(1, 8, Signed)));

  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(127, 8, Signed))
                  .meet(Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(127, 8, Signed))
                  .meet(Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(127, 8, Signed))
                  .meet(Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(127, 8, Signed))
                  .meet(Interval(Int(-128, 8, Signed))) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned)).meet(Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(Interval(Int(0, 8, Unsigned)).meet(Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned)).meet(Interval(Int(1, 8, Unsigned))) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      Interval(Int(1, 8, Unsigned)).meet(Interval(Int(2, 8, Unsigned))) ==
      Interval::bottom(8, Unsigned));

  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned))
                  .meet(Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned))
                  .meet(Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned))
                  .meet(Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(2, 8, Unsigned)));
  BOOST_CHECK(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned))
                  .meet(Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(3, 8, Unsigned)));

  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned))
                  .meet(Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned))
                  .meet(Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned))
          .meet(Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned))) ==
      Interval::bottom(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_narrowing) {
  BOOST_CHECK(Interval::top(8, Signed).narrowing(Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval::top(8, Signed).narrowing(Interval::top(8, Signed)) ==
              Interval::top(8, Signed));

  BOOST_CHECK(Interval::bottom(8, Signed).narrowing(
                  Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(Interval::bottom(8, Signed).narrowing(Interval::top(8, Signed)) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(
      Interval(Int(0, 8, Signed)).narrowing(Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(Interval(Int(0, 8, Signed)).narrowing(Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(
      Interval(Int(0, 8, Signed)).narrowing(Interval(Int(1, 8, Signed))) ==
      Interval(Int(0, 8, Signed)));
  BOOST_CHECK(
      Interval(Int(-1, 8, Signed)).narrowing(Interval(Int(1, 8, Signed))) ==
      Interval(Int(-1, 8, Signed)));

  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                  .narrowing(Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                  .narrowing(Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                  .narrowing(Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                  .narrowing(Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));

  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(127, 8, Signed))
                  .narrowing(Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(127, 8, Signed))
                  .narrowing(Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(127, 8, Signed))
                  .narrowing(Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(127, 8, Signed))
                  .narrowing(Interval(Int(-128, 8, Signed))) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(Interval(Int(-128, 8, Signed), Int(0, 8, Signed))
                  .narrowing(Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval(Int(-128, 8, Signed), Int(0, 8, Signed))
                  .narrowing(Interval::top(8, Signed)) ==
              Interval(Int(-128, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Interval(Int(-128, 8, Signed), Int(0, 8, Signed))
                  .narrowing(Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(-1, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned)).narrowing(Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned)).narrowing(Interval::top(8, Unsigned)) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned)).narrowing(Interval(Int(1, 8, Unsigned))) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      Interval(Int(1, 8, Unsigned)).narrowing(Interval(Int(2, 8, Unsigned))) ==
      Interval(Int(1, 8, Unsigned)));

  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned))
                  .narrowing(Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned))
                  .narrowing(Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(
      Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned))
          .narrowing(Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
      Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)));
  BOOST_CHECK(
      Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned))
          .narrowing(Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
      Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)));

  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned))
                  .narrowing(Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned))
                  .narrowing(Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned))
          .narrowing(Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned))) ==
      Interval::bottom(8, Unsigned));

  BOOST_CHECK(Interval(Int(10, 8, Unsigned), Int(255, 8, Unsigned))
                  .narrowing(Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(Interval(Int(10, 8, Unsigned), Int(255, 8, Unsigned))
                  .narrowing(Interval::top(8, Unsigned)) ==
              Interval(Int(10, 8, Unsigned), Int(255, 8, Unsigned)));
  BOOST_CHECK(
      Interval(Int(10, 8, Unsigned), Int(255, 8, Unsigned))
          .narrowing(Interval(Int(10, 8, Unsigned), Int(11, 8, Unsigned))) ==
      Interval(Int(10, 8, Unsigned), Int(11, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_trunc) {
  BOOST_CHECK(Interval::bottom(8, Signed).trunc(6) ==
              Interval::bottom(6, Signed));
  BOOST_CHECK(Interval::top(8, Signed).trunc(6) == Interval::top(6, Signed));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed)).trunc(6) ==
              Interval(Int(0, 6, Signed), Int(1, 6, Signed)));
  BOOST_CHECK(Interval(Int(-128, 8, Signed), Int(127, 8, Signed)).trunc(6) ==
              Interval::top(6, Signed));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(32, 8, Signed)).trunc(6) ==
              Interval::top(6, Signed));
  BOOST_CHECK(Interval(Int(96, 8, Signed), Int(97, 8, Signed)).trunc(6) ==
              Interval(Int(32, 6, Signed), Int(33, 6, Signed)));
  BOOST_CHECK(Interval(Int(-1, 8, Signed), Int(0, 8, Signed)).trunc(6) ==
              Interval::top(6, Signed));
  BOOST_CHECK(Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)).trunc(6) ==
              Interval(Int(-2, 6, Signed), Int(-1, 6, Signed)));
  BOOST_CHECK(Interval(Int(0, 32, Signed), Int(255, 32, Signed)).trunc(8) ==
              Interval::top(8, Signed));

  BOOST_CHECK(Interval::bottom(8, Unsigned).trunc(6) ==
              Interval::bottom(6, Unsigned));
  BOOST_CHECK(Interval::top(8, Unsigned).trunc(6) ==
              Interval::top(6, Unsigned));
  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)).trunc(6) ==
              Interval(Int(0, 6, Unsigned), Int(1, 6, Unsigned)));
  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(255, 8, Unsigned)).trunc(6) ==
              Interval::top(6, Unsigned));
  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(32, 8, Unsigned)).trunc(6) ==
              Interval(Int(0, 6, Unsigned), Int(32, 6, Unsigned)));
  BOOST_CHECK(Interval(Int(96, 8, Unsigned), Int(97, 8, Unsigned)).trunc(6) ==
              Interval(Int(32, 6, Unsigned), Int(33, 6, Unsigned)));
  BOOST_CHECK(Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned)).trunc(6) ==
              Interval(Int(62, 6, Unsigned), Int(63, 6, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_ext) {
  BOOST_CHECK(Interval::bottom(6, Signed).ext(8) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval::top(6, Signed).ext(8) ==
              Interval(Int(-32, 8, Signed), Int(31, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 6, Signed), Int(1, 6, Signed)).ext(8) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(Interval(Int(-32, 6, Signed), Int(31, 6, Signed)).ext(8) ==
              Interval(Int(-32, 8, Signed), Int(31, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 6, Signed), Int(31, 6, Signed)).ext(8) ==
              Interval(Int(0, 8, Signed), Int(31, 8, Signed)));
  BOOST_CHECK(Interval(Int(-1, 6, Signed), Int(0, 6, Signed)).ext(8) ==
              Interval(Int(-1, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Interval(Int(-2, 6, Signed), Int(-1, 6, Signed)).ext(8) ==
              Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)));

  BOOST_CHECK(Interval::bottom(6, Unsigned).ext(8) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(Interval::top(6, Unsigned).ext(8) ==
              Interval(Int(0, 8, Unsigned), Int(63, 8, Unsigned)));
  BOOST_CHECK(Interval(Int(0, 6, Unsigned), Int(1, 6, Unsigned)).ext(8) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(Interval(Int(0, 6, Unsigned), Int(32, 6, Unsigned)).ext(8) ==
              Interval(Int(0, 8, Unsigned), Int(32, 8, Unsigned)));
  BOOST_CHECK(Interval(Int(32, 6, Unsigned), Int(33, 6, Unsigned)).ext(8) ==
              Interval(Int(32, 8, Unsigned), Int(33, 8, Unsigned)));
  BOOST_CHECK(Interval(Int(62, 6, Unsigned), Int(63, 6, Unsigned)).ext(8) ==
              Interval(Int(62, 8, Unsigned), Int(63, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_sign_cast) {
  BOOST_CHECK(Interval::bottom(8, Signed).sign_cast(Unsigned) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(Interval::top(8, Signed).sign_cast(Unsigned) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(
      Interval(Int(0, 8, Signed), Int(1, 8, Signed)).sign_cast(Unsigned) ==
      Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(
      Interval(Int(0, 8, Signed), Int(32, 8, Signed)).sign_cast(Unsigned) ==
      Interval(Int(0, 8, Unsigned), Int(32, 8, Unsigned)));
  BOOST_CHECK(
      Interval(Int(96, 8, Signed), Int(97, 8, Signed)).sign_cast(Unsigned) ==
      Interval(Int(96, 8, Unsigned), Int(97, 8, Unsigned)));
  BOOST_CHECK(
      Interval(Int(-1, 8, Signed), Int(0, 8, Signed)).sign_cast(Unsigned) ==
      Interval::top(8, Unsigned));
  BOOST_CHECK(
      Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)).sign_cast(Unsigned) ==
      Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned)));
  BOOST_CHECK(Interval(Int(-128, 8, Signed), Int(-127, 8, Signed))
                  .sign_cast(Unsigned) ==
              Interval(Int(128, 8, Unsigned), Int(129, 8, Unsigned)));

  BOOST_CHECK(Interval::bottom(8, Unsigned).sign_cast(Signed) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval::top(8, Unsigned).sign_cast(Signed) ==
              Interval::top(8, Signed));
  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)).sign_cast(Signed) ==
      Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned), Int(255, 8, Unsigned)).sign_cast(Signed) ==
      Interval::top(8, Signed));
  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned), Int(32, 8, Unsigned)).sign_cast(Signed) ==
      Interval(Int(0, 8, Signed), Int(32, 8, Signed)));
  BOOST_CHECK(Interval(Int(150, 8, Unsigned), Int(151, 8, Unsigned))
                  .sign_cast(Signed) ==
              Interval(Int(-106, 8, Signed), Int(-105, 8, Signed)));
  BOOST_CHECK(Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned))
                  .sign_cast(Signed) ==
              Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)));
  BOOST_CHECK(
      Interval(Int(1, 8, Unsigned), Int(150, 8, Unsigned)).sign_cast(Signed) ==
      Interval::top(8, Signed));
}

BOOST_AUTO_TEST_CASE(test_cast) {
  BOOST_CHECK(Interval::bottom(8, Signed).cast(4, Unsigned) ==
              Interval::bottom(4, Unsigned));
  BOOST_CHECK(Interval::top(8, Signed).cast(4, Unsigned) ==
              Interval::top(4, Unsigned));
  BOOST_CHECK(
      Interval(Int(0, 8, Signed), Int(1, 8, Signed)).cast(4, Unsigned) ==
      Interval(Int(0, 4, Unsigned), Int(1, 4, Unsigned)));
  BOOST_CHECK(
      Interval(Int(0, 8, Signed), Int(32, 8, Signed)).cast(4, Unsigned) ==
      Interval(Int(0, 4, Unsigned), Int(15, 4, Unsigned)));
  BOOST_CHECK(
      Interval(Int(96, 8, Signed), Int(97, 8, Signed)).cast(4, Unsigned) ==
      Interval(Int(0, 4, Unsigned), Int(1, 4, Unsigned)));
  BOOST_CHECK(
      Interval(Int(-1, 8, Signed), Int(0, 8, Signed)).cast(4, Unsigned) ==
      Interval::top(4, Unsigned));
  BOOST_CHECK(
      Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)).cast(4, Unsigned) ==
      Interval(Int(14, 4, Unsigned), Int(15, 4, Unsigned)));
  BOOST_CHECK(
      Interval(Int(-128, 8, Signed), Int(-127, 8, Signed)).cast(4, Unsigned) ==
      Interval(Int(0, 4, Unsigned), Int(1, 4, Unsigned)));

  BOOST_CHECK(Interval::bottom(8, Unsigned).cast(4, Signed) ==
              Interval::bottom(4, Signed));
  BOOST_CHECK(Interval::top(8, Unsigned).cast(4, Signed) ==
              Interval::top(4, Signed));
  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)).cast(4, Signed) ==
      Interval(Int(0, 4, Signed), Int(1, 4, Signed)));
  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned), Int(255, 8, Unsigned)).cast(4, Signed) ==
      Interval::top(4, Signed));
  BOOST_CHECK(
      Interval(Int(0, 8, Unsigned), Int(32, 8, Unsigned)).cast(4, Signed) ==
      Interval::top(4, Signed));
  BOOST_CHECK(
      Interval(Int(150, 8, Unsigned), Int(151, 8, Unsigned)).cast(4, Signed) ==
      Interval(Int(6, 4, Signed), Int(7, 4, Signed)));
  BOOST_CHECK(
      Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned)).cast(4, Signed) ==
      Interval(Int(-2, 4, Signed), Int(-1, 4, Signed)));
  BOOST_CHECK(
      Interval(Int(1, 8, Unsigned), Int(150, 8, Unsigned)).cast(4, Signed) ==
      Interval::top(4, Signed));
}

BOOST_AUTO_TEST_CASE(test_singleton) {
  BOOST_CHECK((Interval::bottom(8, Signed).singleton() == boost::none));
  BOOST_CHECK((Interval::top(8, Signed).singleton() == boost::none));
  BOOST_CHECK((Interval(Int(0, 8, Signed), Int(1, 8, Signed)).singleton() ==
               boost::none));
  BOOST_CHECK((Interval(Int(0, 8, Signed)).singleton() ==
               boost::optional< Int >(Int(0, 8, Signed))));

  BOOST_CHECK((Interval::bottom(8, Unsigned).singleton() == boost::none));
  BOOST_CHECK((Interval::top(8, Unsigned).singleton() == boost::none));
  BOOST_CHECK((Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)).singleton() ==
               boost::none));
  BOOST_CHECK((Interval(Int(0, 8, Unsigned)).singleton() ==
               boost::optional< Int >(Int(0, 8, Unsigned))));
}

BOOST_AUTO_TEST_CASE(test_contains) {
  BOOST_CHECK(!Interval::bottom(8, Signed).contains(Int(0, 8, Signed)));
  BOOST_CHECK(Interval::top(8, Signed).contains(Int(0, 8, Signed)));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                  .contains(Int(0, 8, Signed)));
  BOOST_CHECK(!Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                   .contains(Int(2, 8, Signed)));
  BOOST_CHECK(!Interval(Int(0, 8, Signed), Int(1, 8, Signed))
                   .contains(Int(-1, 8, Signed)));

  BOOST_CHECK(!Interval::bottom(8, Unsigned).contains(Int(0, 8, Unsigned)));
  BOOST_CHECK(Interval::top(8, Unsigned).contains(Int(0, 8, Unsigned)));
  BOOST_CHECK(Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))
                  .contains(Int(1, 8, Unsigned)));
  BOOST_CHECK(!Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))
                   .contains(Int(3, 8, Unsigned)));
  BOOST_CHECK(!Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))
                   .contains(Int(0, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_add) {
  BOOST_CHECK(add(Interval::top(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(add(Interval::top(8, Signed), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(add(Interval::bottom(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(add(Interval::bottom(8, Signed), Interval::top(8, Signed)) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(add(Interval(Int(0, 8, Signed)), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(add(Interval(Int(0, 8, Signed)), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(add(Interval(Int(0, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(1, 8, Signed)));
  BOOST_CHECK(add(Interval(Int(-1, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(
      add(Interval(Int(-128, 8, Signed)), Interval(Int(-129, 8, Signed))) ==
      Interval(Int(-1, 8, Signed)));

  BOOST_CHECK(add(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(add(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval::top(8, Signed)) == Interval::top(8, Signed));
  BOOST_CHECK(add(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(-1, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(add(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(1, 8, Signed), Int(3, 8, Signed)));

  BOOST_CHECK(add(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(add(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval::top(8, Signed)) == Interval::top(8, Signed));
  BOOST_CHECK(add(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(-1, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(add(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(-128, 8, Signed))) ==
              Interval(Int(-128, 8, Signed), Int(-1, 8, Signed)));

  BOOST_CHECK(add(Interval(Int(1, 8, Signed), Int(2, 8, Signed)),
                  Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(-128, 8, Signed), Int(-127, 8, Signed)));
  BOOST_CHECK(add(Interval(Int(1, 8, Signed), Int(126, 8, Signed)),
                  Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(-128, 8, Signed), Int(-3, 8, Signed)));
  BOOST_CHECK(add(Interval(Int(0, 8, Signed), Int(126, 8, Signed)),
                  Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(add(Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(-128, 8, Signed))) ==
              Interval(Int(126, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(add(Interval(Int(-2, 8, Signed), Int(0, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(-128, 8, Signed))) ==
              Interval::top(8, Signed));

  BOOST_CHECK(
      add(Interval(Int(0, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(add(Interval(Int(0, 8, Unsigned)), Interval::top(8, Unsigned)) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(
      add(Interval(Int(0, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      add(Interval(Int(1, 8, Unsigned)), Interval(Int(2, 8, Unsigned))) ==
      Interval(Int(3, 8, Unsigned)));
  BOOST_CHECK(
      add(Interval(Int(255, 8, Unsigned)), Interval(Int(256, 8, Unsigned))) ==
      Interval(Int(255, 8, Unsigned)));

  BOOST_CHECK(add(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                  Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(add(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                  Interval::top(8, Unsigned)) == Interval::top(8, Unsigned));
  BOOST_CHECK(add(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                  Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(3, 8, Unsigned), Int(5, 8, Unsigned)));
  BOOST_CHECK(add(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                  Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(5, 8, Unsigned), Int(7, 8, Unsigned)));

  BOOST_CHECK(add(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                  Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(add(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                  Interval::top(8, Unsigned)) == Interval::top(8, Unsigned));
  BOOST_CHECK(add(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                  Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval::top(8, Unsigned));

  BOOST_CHECK(add(Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned)),
                  Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(add(Interval(Int(1, 8, Unsigned), Int(255, 8, Unsigned)),
                  Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(254, 8, Unsigned)));
  BOOST_CHECK(add(Interval(Int(0, 8, Unsigned), Int(255, 8, Unsigned)),
                  Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(add(Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned)),
                  Interval(Int(128, 8, Unsigned), Int(128, 8, Unsigned))) ==
              Interval(Int(126, 8, Unsigned), Int(127, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_add_no_wrap) {
  BOOST_CHECK(
      add_no_wrap(Interval::top(8, Signed), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(add_no_wrap(Interval::top(8, Signed), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(
      add_no_wrap(Interval::bottom(8, Signed), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      add_no_wrap(Interval::bottom(8, Signed), Interval::top(8, Signed)) ==
      Interval::bottom(8, Signed));

  BOOST_CHECK(
      add_no_wrap(Interval(Int(0, 8, Signed)), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      add_no_wrap(Interval(Int(0, 8, Signed)), Interval::top(8, Signed)) ==
      Interval::top(8, Signed));
  BOOST_CHECK(
      add_no_wrap(Interval(Int(0, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(1, 8, Signed)));
  BOOST_CHECK(
      add_no_wrap(Interval(Int(-1, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(0, 8, Signed)));
  BOOST_CHECK(add_no_wrap(Interval(Int(-128, 8, Signed)),
                          Interval(Int(127, 8, Signed))) ==
              Interval(Int(-1, 8, Signed)));

  BOOST_CHECK(add_no_wrap(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                          Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(add_no_wrap(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                          Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(add_no_wrap(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                          Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(-1, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(add_no_wrap(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                          Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(1, 8, Signed), Int(3, 8, Signed)));

  BOOST_CHECK(add_no_wrap(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                          Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(add_no_wrap(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                          Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(add_no_wrap(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                          Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(-1, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(add_no_wrap(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                          Interval(Int(-128, 8, Signed))) ==
              Interval(Int(-128, 8, Signed), Int(-1, 8, Signed)));

  BOOST_CHECK(add_no_wrap(Interval(Int(1, 8, Signed), Int(2, 8, Signed)),
                          Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(add_no_wrap(Interval(Int(1, 8, Signed), Int(126, 8, Signed)),
                          Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(add_no_wrap(Interval(Int(0, 8, Signed), Int(126, 8, Signed)),
                          Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(127, 8, Signed)));
  BOOST_CHECK(
      add_no_wrap(Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(-128, 8, Signed))) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      add_no_wrap(Interval(Int(-2, 8, Signed), Int(0, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(-128, 8, Signed))) ==
      Interval(Int(-128, 8, Signed)));

  BOOST_CHECK(add_no_wrap(Interval(Int(0, 8, Unsigned)),
                          Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      add_no_wrap(Interval(Int(0, 8, Unsigned)), Interval::top(8, Unsigned)) ==
      Interval::top(8, Unsigned));
  BOOST_CHECK(add_no_wrap(Interval(Int(0, 8, Unsigned)),
                          Interval(Int(1, 8, Unsigned))) ==
              Interval(Int(1, 8, Unsigned)));
  BOOST_CHECK(add_no_wrap(Interval(Int(1, 8, Unsigned)),
                          Interval(Int(2, 8, Unsigned))) ==
              Interval(Int(3, 8, Unsigned)));
  BOOST_CHECK(add_no_wrap(Interval(Int(255, 8, Unsigned)),
                          Interval(Int(256, 8, Unsigned))) ==
              Interval(Int(255, 8, Unsigned)));

  BOOST_CHECK(add_no_wrap(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                          Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(add_no_wrap(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                          Interval::top(8, Unsigned)) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(add_no_wrap(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                          Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(3, 8, Unsigned), Int(5, 8, Unsigned)));
  BOOST_CHECK(add_no_wrap(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                          Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(5, 8, Unsigned), Int(7, 8, Unsigned)));

  BOOST_CHECK(add_no_wrap(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                          Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(add_no_wrap(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                          Interval::top(8, Unsigned)) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(
      add_no_wrap(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                  Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned))) ==
      Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned)));

  BOOST_CHECK(
      add_no_wrap(Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned)),
                  Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      add_no_wrap(Interval(Int(1, 8, Unsigned), Int(255, 8, Unsigned)),
                  Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      add_no_wrap(Interval(Int(0, 8, Unsigned), Int(255, 8, Unsigned)),
                  Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
      Interval(Int(255, 8, Unsigned)));
  BOOST_CHECK(
      add_no_wrap(Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned)),
                  Interval(Int(128, 8, Unsigned), Int(128, 8, Unsigned))) ==
      Interval::bottom(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_sub) {
  BOOST_CHECK(sub(Interval::top(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(sub(Interval::top(8, Signed), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(sub(Interval::bottom(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(sub(Interval::bottom(8, Signed), Interval::top(8, Signed)) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(sub(Interval(Int(0, 8, Signed)), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(sub(Interval(Int(0, 8, Signed)), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(sub(Interval(Int(0, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(-1, 8, Signed)));
  BOOST_CHECK(sub(Interval(Int(1, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(
      sub(Interval(Int(-128, 8, Signed)), Interval(Int(-129, 8, Signed))) ==
      Interval(Int(1, 8, Signed)));

  BOOST_CHECK(sub(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(sub(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval::top(8, Signed)) == Interval::top(8, Signed));
  BOOST_CHECK(sub(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(sub(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(-2, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(sub(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(sub(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval::top(8, Signed)) == Interval::top(8, Signed));
  BOOST_CHECK(sub(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(-128, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(sub(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(-128, 8, Signed))) ==
              Interval(Int(-128, 8, Signed), Int(-1, 8, Signed)));

  BOOST_CHECK(sub(Interval(Int(1, 8, Signed), Int(2, 8, Signed)),
                  Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(-126, 8, Signed), Int(-125, 8, Signed)));
  BOOST_CHECK(sub(Interval(Int(1, 8, Signed), Int(126, 8, Signed)),
                  Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(-126, 8, Signed), Int(-1, 8, Signed)));
  BOOST_CHECK(sub(Interval(Int(0, 8, Signed), Int(126, 8, Signed)),
                  Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(-127, 8, Signed), Int(-1, 8, Signed)));
  BOOST_CHECK(sub(Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(-128, 8, Signed))) ==
              Interval(Int(126, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(sub(Interval(Int(-2, 8, Signed), Int(0, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(-128, 8, Signed))) ==
              Interval::top(8, Signed));

  BOOST_CHECK(
      sub(Interval(Int(0, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(sub(Interval(Int(0, 8, Unsigned)), Interval::top(8, Unsigned)) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(
      sub(Interval(Int(0, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(-1, 8, Unsigned)));
  BOOST_CHECK(
      sub(Interval(Int(1, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      sub(Interval(Int(255, 8, Unsigned)), Interval(Int(256, 8, Unsigned))) ==
      Interval(Int(255, 8, Unsigned)));

  BOOST_CHECK(sub(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                  Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(sub(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                  Interval::top(8, Unsigned)) == Interval::top(8, Unsigned));
  BOOST_CHECK(sub(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                  Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(sub(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                  Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval::top(8, Unsigned));

  BOOST_CHECK(sub(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                  Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(sub(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                  Interval::top(8, Unsigned)) == Interval::top(8, Unsigned));
  BOOST_CHECK(sub(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                  Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval(Int(1, 8, Unsigned), Int(129, 8, Unsigned)));

  BOOST_CHECK(sub(Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned)),
                  Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)));
  BOOST_CHECK(sub(Interval(Int(1, 8, Unsigned), Int(255, 8, Unsigned)),
                  Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(sub(Interval(Int(0, 8, Unsigned), Int(255, 8, Unsigned)),
                  Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(sub(Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned)),
                  Interval(Int(128, 8, Unsigned), Int(128, 8, Unsigned))) ==
              Interval(Int(126, 8, Unsigned), Int(127, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_sub_no_wrap) {
  BOOST_CHECK(
      sub_no_wrap(Interval::top(8, Signed), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(sub_no_wrap(Interval::top(8, Signed), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(
      sub_no_wrap(Interval::bottom(8, Signed), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      sub_no_wrap(Interval::bottom(8, Signed), Interval::top(8, Signed)) ==
      Interval::bottom(8, Signed));

  BOOST_CHECK(
      sub_no_wrap(Interval(Int(0, 8, Signed)), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      sub_no_wrap(Interval(Int(0, 8, Signed)), Interval::top(8, Signed)) ==
      Interval(Int(-127, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(
      sub_no_wrap(Interval(Int(0, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(-1, 8, Signed)));
  BOOST_CHECK(
      sub_no_wrap(Interval(Int(1, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(0, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Interval(Int(-128, 8, Signed)),
                          Interval(Int(-129, 8, Signed))) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(sub_no_wrap(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                          Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(sub_no_wrap(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                          Interval::top(8, Signed)) ==
              Interval(Int(-127, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                          Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                          Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(-2, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(sub_no_wrap(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                          Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(sub_no_wrap(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                          Interval::top(8, Signed)) ==
              Interval(Int(-127, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                          Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                          Interval(Int(-128, 8, Signed))) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(sub_no_wrap(Interval(Int(1, 8, Signed), Int(2, 8, Signed)),
                          Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(-126, 8, Signed), Int(-125, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Interval(Int(1, 8, Signed), Int(126, 8, Signed)),
                          Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(-126, 8, Signed), Int(-1, 8, Signed)));
  BOOST_CHECK(sub_no_wrap(Interval(Int(0, 8, Signed), Int(126, 8, Signed)),
                          Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(-127, 8, Signed), Int(-1, 8, Signed)));
  BOOST_CHECK(
      sub_no_wrap(Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(-128, 8, Signed))) ==
      Interval(Int(126, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(
      sub_no_wrap(Interval(Int(-2, 8, Signed), Int(0, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(-128, 8, Signed))) ==
      Interval(Int(126, 8, Signed), Int(127, 8, Signed)));

  BOOST_CHECK(sub_no_wrap(Interval(Int(0, 8, Unsigned)),
                          Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      sub_no_wrap(Interval(Int(0, 8, Unsigned)), Interval::top(8, Unsigned)) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(sub_no_wrap(Interval(Int(0, 8, Unsigned)),
                          Interval(Int(1, 8, Unsigned))) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(sub_no_wrap(Interval(Int(1, 8, Unsigned)),
                          Interval(Int(1, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(sub_no_wrap(Interval(Int(255, 8, Unsigned)),
                          Interval(Int(256, 8, Unsigned))) ==
              Interval(Int(255, 8, Unsigned)));

  BOOST_CHECK(sub_no_wrap(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                          Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(sub_no_wrap(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                          Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(sub_no_wrap(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                          Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(sub_no_wrap(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                          Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned)));

  BOOST_CHECK(sub_no_wrap(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                          Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(sub_no_wrap(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                          Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(
      sub_no_wrap(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                  Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned))) ==
      Interval::bottom(8, Unsigned));

  BOOST_CHECK(
      sub_no_wrap(Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned)),
                  Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      sub_no_wrap(Interval(Int(1, 8, Unsigned), Int(255, 8, Unsigned)),
                  Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      sub_no_wrap(Interval(Int(0, 8, Unsigned), Int(255, 8, Unsigned)),
                  Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      sub_no_wrap(Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned)),
                  Interval(Int(128, 8, Unsigned), Int(128, 8, Unsigned))) ==
      Interval(Int(126, 8, Unsigned), Int(127, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_rem) {
  BOOST_CHECK(rem(Interval::top(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(rem(Interval::top(8, Signed), Interval::top(8, Signed)) ==
              Interval(Int(-127, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(rem(Interval::bottom(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(rem(Interval::bottom(8, Signed), Interval::top(8, Signed)) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(rem(Interval(Int(0, 8, Signed)), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(rem(Interval(Int(0, 8, Signed)), Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(rem(Interval(Int(0, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(rem(Interval(Int(1, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(
      rem(Interval(Int(-128, 8, Signed)), Interval(Int(127, 8, Signed))) ==
      Interval(Int(-1, 8, Signed)));

  BOOST_CHECK(rem(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(rem(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(rem(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(rem(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));

  BOOST_CHECK(rem(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(rem(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(rem(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(rem(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(-128, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));

  BOOST_CHECK(rem(Interval(Int(1, 8, Signed), Int(2, 8, Signed)),
                  Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(rem(Interval(Int(1, 8, Signed), Int(126, 8, Signed)),
                  Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(126, 8, Signed)));
  BOOST_CHECK(rem(Interval(Int(0, 8, Signed), Int(126, 8, Signed)),
                  Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(126, 8, Signed)));
  BOOST_CHECK(rem(Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(-128, 8, Signed))) ==
              Interval(Int(-2, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(rem(Interval(Int(-2, 8, Signed), Int(0, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(-128, 8, Signed))) ==
              Interval(Int(-2, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(rem(Interval::top(8, Signed), Interval(Int(0, 8, Signed))) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(rem(Interval::top(8, Signed), Interval(Int(42, 8, Signed))) ==
              Interval(Int(-41, 8, Signed), Int(41, 8, Signed)));
  BOOST_CHECK(rem(Interval(Int(1, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(42, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(41, 8, Signed)));
  BOOST_CHECK(rem(Interval(Int(1, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(-52, 8, Signed), Int(42, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(51, 8, Signed)));
  BOOST_CHECK(rem(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                  Interval(Int(-52, 8, Signed), Int(42, 8, Signed))) ==
              Interval(Int(-51, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(rem(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(42, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(rem(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(rem(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(42, 8, Signed))) ==
              Interval(Int(-127, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(rem(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(-127, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(
      rem(Interval(Int(0, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(rem(Interval(Int(0, 8, Unsigned)), Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      rem(Interval(Int(0, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      rem(Interval(Int(1, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      rem(Interval(Int(255, 8, Unsigned)), Interval(Int(0, 8, Unsigned))) ==
      Interval::bottom(8, Unsigned));

  BOOST_CHECK(rem(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                  Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(rem(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                  Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(rem(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                  Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(rem(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                  Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(3, 8, Unsigned)));

  BOOST_CHECK(rem(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                  Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(rem(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                  Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(rem(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                  Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));

  BOOST_CHECK(rem(Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned)),
                  Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(rem(Interval(Int(1, 8, Unsigned), Int(255, 8, Unsigned)),
                  Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(254, 8, Unsigned)));
  BOOST_CHECK(rem(Interval(Int(0, 8, Unsigned), Int(255, 8, Unsigned)),
                  Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(254, 8, Unsigned)));
  BOOST_CHECK(rem(Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned)),
                  Interval(Int(128, 8, Unsigned), Int(128, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_shl) {
  BOOST_CHECK(shl(Interval::top(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(shl(Interval::top(8, Signed), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval::bottom(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(shl(Interval::bottom(8, Signed), Interval::top(8, Signed)) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(shl(Interval(Int(0, 8, Signed)), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(shl(Interval(Int(0, 8, Signed)), Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(shl(Interval(Int(0, 8, Signed)),
                  Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(shl(Interval(Int(0, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));

  BOOST_CHECK(shl(Interval(Int(1, 8, Signed)), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(shl(Interval(Int(1, 8, Signed)), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(1, 8, Signed)),
                  Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(1, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(2, 8, Signed)));
  BOOST_CHECK(shl(Interval(Int(1, 8, Signed)), Interval(Int(2, 8, Signed))) ==
              Interval(Int(4, 8, Signed)));
  BOOST_CHECK(shl(Interval(Int(1, 8, Signed)), Interval(Int(6, 8, Signed))) ==
              Interval(Int(64, 8, Signed)));
  BOOST_CHECK(shl(Interval(Int(1, 8, Signed)), Interval(Int(7, 8, Signed))) ==
              Interval(Int(-128, 8, Signed)));

  BOOST_CHECK(shl(Interval(Int(127, 8, Signed)), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(shl(Interval(Int(127, 8, Signed)), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(127, 8, Signed)),
                  Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(127, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(-2, 8, Signed)));
  BOOST_CHECK(shl(Interval(Int(127, 8, Signed)), Interval(Int(2, 8, Signed))) ==
              Interval(Int(-4, 8, Signed)));
  BOOST_CHECK(shl(Interval(Int(127, 8, Signed)), Interval(Int(6, 8, Signed))) ==
              Interval(Int(-64, 8, Signed)));
  BOOST_CHECK(shl(Interval(Int(127, 8, Signed)), Interval(Int(7, 8, Signed))) ==
              Interval(Int(-128, 8, Signed)));

  BOOST_CHECK(shl(Interval(Int(-128, 8, Signed)),
                  Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(shl(Interval(Int(-128, 8, Signed)), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(-128, 8, Signed)),
                  Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(-128, 8, Signed)),
                  Interval(Int(1, 8, Signed))) == Interval(Int(0, 8, Signed)));
  BOOST_CHECK(shl(Interval(Int(-128, 8, Signed)),
                  Interval(Int(2, 8, Signed))) == Interval(Int(0, 8, Signed)));
  BOOST_CHECK(shl(Interval(Int(-128, 8, Signed)),
                  Interval(Int(6, 8, Signed))) == Interval(Int(0, 8, Signed)));
  BOOST_CHECK(shl(Interval(Int(-128, 8, Signed)),
                  Interval(Int(7, 8, Signed))) == Interval(Int(0, 8, Signed)));

  BOOST_CHECK(shl(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(shl(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval::top(8, Signed)) == Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(shl(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(4, 8, Signed)));

  BOOST_CHECK(shl(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(shl(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval::top(8, Signed)) == Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(shl(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(7, 8, Signed))) == Interval::top(8, Signed));

  BOOST_CHECK(shl(Interval(Int(1, 8, Signed), Int(2, 8, Signed)),
                  Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(-128, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl(Interval(Int(1, 8, Signed), Int(126, 8, Signed)),
                  Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(0, 8, Signed), Int(126, 8, Signed)),
                  Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)),
                  Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(-2, 8, Signed), Int(0, 8, Signed)),
                  Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval::top(8, Signed));

  BOOST_CHECK(shl(Interval::top(8, Signed), Interval(Int(0, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval::top(8, Signed), Interval(Int(4, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(1, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(4, 8, Signed))) == Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(1, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                  Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                  Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(shl(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                  Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval::top(8, Signed));

  BOOST_CHECK(
      shl(Interval(Int(0, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(shl(Interval(Int(0, 8, Unsigned)), Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl(Interval(Int(0, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl(Interval(Int(1, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(2, 8, Unsigned)));
  BOOST_CHECK(
      shl(Interval(Int(255, 8, Unsigned)), Interval(Int(0, 8, Unsigned))) ==
      Interval(Int(255, 8, Unsigned)));

  BOOST_CHECK(shl(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                  Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(shl(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                  Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(128, 8, Unsigned)));
  BOOST_CHECK(shl(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                  Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(4, 8, Unsigned), Int(12, 8, Unsigned)));
  BOOST_CHECK(shl(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                  Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(16, 8, Unsigned), Int(48, 8, Unsigned)));

  BOOST_CHECK(shl(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                  Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(shl(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                  Interval::top(8, Unsigned)) == Interval::top(8, Unsigned));
  BOOST_CHECK(shl(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                  Interval(Int(4, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval::top(8, Unsigned));

  BOOST_CHECK(
      shl(Interval(Int(1, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(shl(Interval(Int(1, 8, Unsigned)), Interval::top(8, Unsigned)) ==
              Interval(Int(1, 8, Unsigned), Int(128, 8, Unsigned)));
  BOOST_CHECK(shl(Interval(Int(1, 8, Unsigned)),
                  Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval(Int(1, 8, Unsigned), Int(128, 8, Unsigned)));
  BOOST_CHECK(
      shl(Interval(Int(1, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(2, 8, Unsigned)));
  BOOST_CHECK(
      shl(Interval(Int(1, 8, Unsigned)), Interval(Int(2, 8, Unsigned))) ==
      Interval(Int(4, 8, Unsigned)));
  BOOST_CHECK(
      shl(Interval(Int(1, 8, Unsigned)), Interval(Int(6, 8, Unsigned))) ==
      Interval(Int(64, 8, Unsigned)));
  BOOST_CHECK(
      shl(Interval(Int(1, 8, Unsigned)), Interval(Int(7, 8, Unsigned))) ==
      Interval(Int(128, 8, Unsigned)));

  BOOST_CHECK(
      shl(Interval(Int(127, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(shl(Interval(Int(127, 8, Unsigned)),
                  Interval::top(8, Unsigned)) == Interval::top(8, Unsigned));
  BOOST_CHECK(shl(Interval(Int(127, 8, Unsigned)),
                  Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(
      shl(Interval(Int(127, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(254, 8, Unsigned)));
  BOOST_CHECK(
      shl(Interval(Int(127, 8, Unsigned)), Interval(Int(2, 8, Unsigned))) ==
      Interval(Int(252, 8, Unsigned)));
  BOOST_CHECK(
      shl(Interval(Int(127, 8, Unsigned)), Interval(Int(6, 8, Unsigned))) ==
      Interval(Int(192, 8, Unsigned)));
  BOOST_CHECK(
      shl(Interval(Int(127, 8, Unsigned)), Interval(Int(7, 8, Unsigned))) ==
      Interval(Int(128, 8, Unsigned)));

  BOOST_CHECK(
      shl(Interval(Int(128, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(shl(Interval(Int(128, 8, Unsigned)),
                  Interval::top(8, Unsigned)) == Interval::top(8, Unsigned));
  BOOST_CHECK(shl(Interval(Int(128, 8, Unsigned)),
                  Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(
      shl(Interval(Int(128, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl(Interval(Int(128, 8, Unsigned)), Interval(Int(2, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl(Interval(Int(128, 8, Unsigned)), Interval(Int(6, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      shl(Interval(Int(128, 8, Unsigned)), Interval(Int(7, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));

  BOOST_CHECK(shl(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                  Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(shl(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                  Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(128, 8, Unsigned)));
  BOOST_CHECK(shl(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                  Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(4, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_shl_no_wrap) {
  BOOST_CHECK(
      shl_no_wrap(Interval::top(8, Signed), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Interval::top(8, Signed), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Interval::bottom(8, Signed), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Interval::bottom(8, Signed), Interval::top(8, Signed)) ==
      Interval::bottom(8, Signed));

  BOOST_CHECK(
      shl_no_wrap(Interval(Int(0, 8, Signed)), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Interval(Int(0, 8, Signed)), Interval::top(8, Signed)) ==
      Interval(Int(0, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Signed)),
                          Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(
      shl_no_wrap(Interval(Int(0, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(0, 8, Signed)));

  BOOST_CHECK(
      shl_no_wrap(Interval(Int(1, 8, Signed)), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Interval(Int(1, 8, Signed)), Interval::top(8, Signed)) ==
      Interval(Int(1, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(1, 8, Signed)),
                          Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(1, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(
      shl_no_wrap(Interval(Int(1, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(2, 8, Signed)));
  BOOST_CHECK(
      shl_no_wrap(Interval(Int(1, 8, Signed)), Interval(Int(2, 8, Signed))) ==
      Interval(Int(4, 8, Signed)));
  BOOST_CHECK(
      shl_no_wrap(Interval(Int(1, 8, Signed)), Interval(Int(6, 8, Signed))) ==
      Interval(Int(64, 8, Signed)));
  BOOST_CHECK(
      shl_no_wrap(Interval(Int(1, 8, Signed)), Interval(Int(7, 8, Signed))) ==
      Interval::bottom(8, Signed));

  BOOST_CHECK(
      shl_no_wrap(Interval(Int(127, 8, Signed)), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Interval(Int(127, 8, Signed)), Interval::top(8, Signed)) ==
      Interval(Int(127, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(127, 8, Signed)),
                          Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(127, 8, Signed)));
  BOOST_CHECK(
      shl_no_wrap(Interval(Int(127, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Interval(Int(127, 8, Signed)), Interval(Int(2, 8, Signed))) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Interval(Int(127, 8, Signed)), Interval(Int(6, 8, Signed))) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Interval(Int(127, 8, Signed)), Interval(Int(7, 8, Signed))) ==
      Interval::bottom(8, Signed));

  BOOST_CHECK(shl_no_wrap(Interval(Int(-128, 8, Signed)),
                          Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Interval(Int(-128, 8, Signed)), Interval::top(8, Signed)) ==
      Interval(Int(-128, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(-128, 8, Signed)),
                          Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(-128, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(-128, 8, Signed)),
                          Interval(Int(1, 8, Signed))) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Interval(Int(-128, 8, Signed)),
                          Interval(Int(2, 8, Signed))) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Interval(Int(-128, 8, Signed)),
                          Interval(Int(6, 8, Signed))) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Interval(Int(-128, 8, Signed)),
                          Interval(Int(7, 8, Signed))) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                          Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                          Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                          Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                          Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(4, 8, Signed)));

  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                          Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                          Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                          Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                          Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                          Interval(Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));

  BOOST_CHECK(shl_no_wrap(Interval(Int(1, 8, Signed), Int(2, 8, Signed)),
                          Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Interval(Int(1, 8, Signed), Int(126, 8, Signed)),
                          Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Signed), Int(126, 8, Signed)),
                          Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)),
                          Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(-128, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(-2, 8, Signed), Int(0, 8, Signed)),
                          Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(-128, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(
      shl_no_wrap(Interval::top(8, Signed), Interval(Int(0, 8, Signed))) ==
      Interval::top(8, Signed));
  BOOST_CHECK(
      shl_no_wrap(Interval::top(8, Signed), Interval(Int(4, 8, Signed))) ==
      Interval::top(8, Signed));
  BOOST_CHECK(shl_no_wrap(Interval(Int(1, 8, Signed), Int(127, 8, Signed)),
                          Interval(Int(4, 8, Signed))) ==
              Interval(Int(16, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                          Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(-128, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                          Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                          Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                          Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(-128, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                          Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(-128, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Unsigned)),
                          Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      shl_no_wrap(Interval(Int(0, 8, Unsigned)), Interval::top(8, Unsigned)) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Unsigned)),
                          Interval(Int(1, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(1, 8, Unsigned)),
                          Interval(Int(1, 8, Unsigned))) ==
              Interval(Int(2, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(255, 8, Unsigned)),
                          Interval(Int(0, 8, Unsigned))) ==
              Interval(Int(255, 8, Unsigned)));

  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                          Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                          Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(128, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                          Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(4, 8, Unsigned), Int(12, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                          Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(16, 8, Unsigned), Int(48, 8, Unsigned)));

  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                          Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                          Interval::top(8, Unsigned)) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                          Interval(Int(4, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval::top(8, Unsigned));

  BOOST_CHECK(shl_no_wrap(Interval(Int(1, 8, Unsigned)),
                          Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      shl_no_wrap(Interval(Int(1, 8, Unsigned)), Interval::top(8, Unsigned)) ==
      Interval(Int(1, 8, Unsigned), Int(128, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(1, 8, Unsigned)),
                          Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval(Int(1, 8, Unsigned), Int(128, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(1, 8, Unsigned)),
                          Interval(Int(1, 8, Unsigned))) ==
              Interval(Int(2, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(1, 8, Unsigned)),
                          Interval(Int(2, 8, Unsigned))) ==
              Interval(Int(4, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(1, 8, Unsigned)),
                          Interval(Int(6, 8, Unsigned))) ==
              Interval(Int(64, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(1, 8, Unsigned)),
                          Interval(Int(7, 8, Unsigned))) ==
              Interval(Int(128, 8, Unsigned)));

  BOOST_CHECK(shl_no_wrap(Interval(Int(127, 8, Unsigned)),
                          Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Interval(Int(127, 8, Unsigned)),
                          Interval::top(8, Unsigned)) ==
              Interval(Int(127, 8, Unsigned), Int(255, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(127, 8, Unsigned)),
                          Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval(Int(127, 8, Unsigned), Int(255, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(127, 8, Unsigned)),
                          Interval(Int(1, 8, Unsigned))) ==
              Interval(Int(254, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(127, 8, Unsigned)),
                          Interval(Int(2, 8, Unsigned))) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Interval(Int(127, 8, Unsigned)),
                          Interval(Int(6, 8, Unsigned))) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Interval(Int(127, 8, Unsigned)),
                          Interval(Int(7, 8, Unsigned))) ==
              Interval::bottom(8, Unsigned));

  BOOST_CHECK(shl_no_wrap(Interval(Int(128, 8, Unsigned)),
                          Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Interval(Int(128, 8, Unsigned)),
                          Interval::top(8, Unsigned)) ==
              Interval(Int(128, 8, Unsigned), Int(255, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(128, 8, Unsigned)),
                          Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval(Int(128, 8, Unsigned), Int(255, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(128, 8, Unsigned)),
                          Interval(Int(1, 8, Unsigned))) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Interval(Int(128, 8, Unsigned)),
                          Interval(Int(2, 8, Unsigned))) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Interval(Int(128, 8, Unsigned)),
                          Interval(Int(6, 8, Unsigned))) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Interval(Int(128, 8, Unsigned)),
                          Interval(Int(7, 8, Unsigned))) ==
              Interval::bottom(8, Unsigned));

  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                          Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                          Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(128, 8, Unsigned)));
  BOOST_CHECK(shl_no_wrap(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                          Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(4, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_lshr) {
  BOOST_CHECK(lshr(Interval::top(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(lshr(Interval::top(8, Signed), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(lshr(Interval::bottom(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(lshr(Interval::bottom(8, Signed), Interval::top(8, Signed)) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(lshr(Interval(Int(0, 8, Signed)), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Signed)), Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Signed)),
                   Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));

  BOOST_CHECK(lshr(Interval(Int(1, 8, Signed)), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(lshr(Interval(Int(1, 8, Signed)), Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(1, 8, Signed)),
                   Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(1, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(1, 8, Signed)), Interval(Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(1, 8, Signed)), Interval(Int(6, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(1, 8, Signed)), Interval(Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));

  BOOST_CHECK(lshr(Interval(Int(127, 8, Signed)),
                   Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(lshr(Interval(Int(127, 8, Signed)), Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(127, 8, Signed)),
                   Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(
      lshr(Interval(Int(127, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(63, 8, Signed)));
  BOOST_CHECK(
      lshr(Interval(Int(127, 8, Signed)), Interval(Int(2, 8, Signed))) ==
      Interval(Int(31, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(127, 8, Signed)),
                   Interval(Int(6, 8, Signed))) == Interval(Int(1, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(127, 8, Signed)),
                   Interval(Int(7, 8, Signed))) == Interval(Int(0, 8, Signed)));

  BOOST_CHECK(lshr(Interval(Int(-128, 8, Signed)),
                   Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(lshr(Interval(Int(-128, 8, Signed)), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(lshr(Interval(Int(-128, 8, Signed)),
                   Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(
      lshr(Interval(Int(-128, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(64, 8, Signed)));
  BOOST_CHECK(
      lshr(Interval(Int(-128, 8, Signed)), Interval(Int(2, 8, Signed))) ==
      Interval(Int(32, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(-128, 8, Signed)),
                   Interval(Int(6, 8, Signed))) == Interval(Int(2, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(-128, 8, Signed)),
                   Interval(Int(7, 8, Signed))) == Interval(Int(1, 8, Signed)));

  BOOST_CHECK(lshr(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                   Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                   Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                   Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                   Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));

  BOOST_CHECK(lshr(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(63, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(7, 8, Signed))) == Interval(Int(0, 8, Signed)));

  BOOST_CHECK(lshr(Interval(Int(1, 8, Signed), Int(2, 8, Signed)),
                   Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(1, 8, Signed), Int(126, 8, Signed)),
                   Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Signed), Int(126, 8, Signed)),
                   Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)),
                   Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(1, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(-2, 8, Signed), Int(0, 8, Signed)),
                   Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));

  BOOST_CHECK(lshr(Interval::top(8, Signed), Interval(Int(0, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(lshr(Interval::top(8, Signed), Interval(Int(4, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(15, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(1, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(4, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(7, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                   Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(15, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(7, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                   Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(15, 8, Signed)));
  BOOST_CHECK(lshr(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                   Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(15, 8, Signed)));

  BOOST_CHECK(
      lshr(Interval(Int(0, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Unsigned)), Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Interval(Int(0, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Interval(Int(1, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Interval(Int(255, 8, Unsigned)), Interval(Int(0, 8, Unsigned))) ==
      Interval(Int(255, 8, Unsigned)));

  BOOST_CHECK(lshr(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                   Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                   Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(lshr(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                   Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(lshr(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                   Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(0, 8, Unsigned)));

  BOOST_CHECK(lshr(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                   Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                   Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                   Interval(Int(4, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned)));

  BOOST_CHECK(
      lshr(Interval(Int(1, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(lshr(Interval(Int(1, 8, Unsigned)), Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(lshr(Interval(Int(1, 8, Unsigned)),
                   Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Interval(Int(1, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Interval(Int(1, 8, Unsigned)), Interval(Int(2, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Interval(Int(1, 8, Unsigned)), Interval(Int(6, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Interval(Int(1, 8, Unsigned)), Interval(Int(7, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));

  BOOST_CHECK(
      lshr(Interval(Int(127, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      lshr(Interval(Int(127, 8, Unsigned)), Interval::top(8, Unsigned)) ==
      Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(lshr(Interval(Int(127, 8, Unsigned)),
                   Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Interval(Int(127, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(63, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Interval(Int(127, 8, Unsigned)), Interval(Int(2, 8, Unsigned))) ==
      Interval(Int(31, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Interval(Int(127, 8, Unsigned)), Interval(Int(6, 8, Unsigned))) ==
      Interval(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Interval(Int(127, 8, Unsigned)), Interval(Int(7, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));

  BOOST_CHECK(
      lshr(Interval(Int(128, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      lshr(Interval(Int(128, 8, Unsigned)), Interval::top(8, Unsigned)) ==
      Interval(Int(1, 8, Unsigned), Int(128, 8, Unsigned)));
  BOOST_CHECK(lshr(Interval(Int(128, 8, Unsigned)),
                   Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval(Int(1, 8, Unsigned), Int(128, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Interval(Int(128, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(64, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Interval(Int(128, 8, Unsigned)), Interval(Int(2, 8, Unsigned))) ==
      Interval(Int(32, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Interval(Int(128, 8, Unsigned)), Interval(Int(6, 8, Unsigned))) ==
      Interval(Int(2, 8, Unsigned)));
  BOOST_CHECK(
      lshr(Interval(Int(128, 8, Unsigned)), Interval(Int(7, 8, Unsigned))) ==
      Interval(Int(1, 8, Unsigned)));

  BOOST_CHECK(lshr(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                   Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                   Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(lshr(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                   Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(0, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_lshr_exact) {
  BOOST_CHECK(
      lshr_exact(Interval::top(8, Signed), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(lshr_exact(Interval::top(8, Signed), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(
      lshr_exact(Interval::bottom(8, Signed), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Interval::bottom(8, Signed), Interval::top(8, Signed)) ==
      Interval::bottom(8, Signed));

  BOOST_CHECK(
      lshr_exact(Interval(Int(0, 8, Signed)), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Interval(Int(0, 8, Signed)), Interval::top(8, Signed)) ==
      Interval(Int(0, 8, Signed)));
  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Signed)),
                         Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Interval(Int(0, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(0, 8, Signed)));

  BOOST_CHECK(
      lshr_exact(Interval(Int(1, 8, Signed)), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Interval(Int(1, 8, Signed)), Interval::top(8, Signed)) ==
      Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(lshr_exact(Interval(Int(1, 8, Signed)),
                         Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Interval(Int(1, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(0, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Interval(Int(1, 8, Signed)), Interval(Int(2, 8, Signed))) ==
      Interval(Int(0, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Interval(Int(1, 8, Signed)), Interval(Int(6, 8, Signed))) ==
      Interval(Int(0, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Interval(Int(1, 8, Signed)), Interval(Int(7, 8, Signed))) ==
      Interval(Int(0, 8, Signed)));

  BOOST_CHECK(
      lshr_exact(Interval(Int(127, 8, Signed)), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      lshr_exact(Interval(Int(127, 8, Signed)), Interval::top(8, Signed)) ==
      Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(lshr_exact(Interval(Int(127, 8, Signed)),
                         Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Interval(Int(127, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(63, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Interval(Int(127, 8, Signed)), Interval(Int(2, 8, Signed))) ==
      Interval(Int(31, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Interval(Int(127, 8, Signed)), Interval(Int(6, 8, Signed))) ==
      Interval(Int(1, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Interval(Int(127, 8, Signed)), Interval(Int(7, 8, Signed))) ==
      Interval(Int(0, 8, Signed)));

  BOOST_CHECK(
      lshr_exact(Interval(Int(-128, 8, Signed)), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(lshr_exact(Interval(Int(-128, 8, Signed)),
                         Interval::top(8, Signed)) == Interval::top(8, Signed));
  BOOST_CHECK(lshr_exact(Interval(Int(-128, 8, Signed)),
                         Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(
      lshr_exact(Interval(Int(-128, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(64, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Interval(Int(-128, 8, Signed)), Interval(Int(2, 8, Signed))) ==
      Interval(Int(32, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Interval(Int(-128, 8, Signed)), Interval(Int(6, 8, Signed))) ==
      Interval(Int(2, 8, Signed)));
  BOOST_CHECK(
      lshr_exact(Interval(Int(-128, 8, Signed)), Interval(Int(7, 8, Signed))) ==
      Interval(Int(1, 8, Signed)));

  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                         Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                         Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                         Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                         Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));

  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                         Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                         Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                         Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                         Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(63, 8, Signed)));
  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                         Interval(Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));

  BOOST_CHECK(lshr_exact(Interval(Int(1, 8, Signed), Int(2, 8, Signed)),
                         Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(lshr_exact(Interval(Int(1, 8, Signed), Int(126, 8, Signed)),
                         Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Signed), Int(126, 8, Signed)),
                         Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(lshr_exact(Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)),
                         Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(1, 8, Signed)));
  BOOST_CHECK(lshr_exact(Interval(Int(-2, 8, Signed), Int(0, 8, Signed)),
                         Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));

  BOOST_CHECK(
      lshr_exact(Interval::top(8, Signed), Interval(Int(0, 8, Signed))) ==
      Interval::top(8, Signed));
  BOOST_CHECK(
      lshr_exact(Interval::top(8, Signed), Interval(Int(4, 8, Signed))) ==
      Interval(Int(0, 8, Signed), Int(15, 8, Signed)));
  BOOST_CHECK(lshr_exact(Interval(Int(1, 8, Signed), Int(127, 8, Signed)),
                         Interval(Int(4, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(7, 8, Signed)));
  BOOST_CHECK(lshr_exact(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                         Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(15, 8, Signed)));
  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                         Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(7, 8, Signed)));
  BOOST_CHECK(lshr_exact(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                         Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(15, 8, Signed)));
  BOOST_CHECK(lshr_exact(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                         Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(15, 8, Signed)));

  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Unsigned)),
                         Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      lshr_exact(Interval(Int(0, 8, Unsigned)), Interval::top(8, Unsigned)) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Unsigned)),
                         Interval(Int(1, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(1, 8, Unsigned)),
                         Interval(Int(1, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(255, 8, Unsigned)),
                         Interval(Int(0, 8, Unsigned))) ==
              Interval(Int(255, 8, Unsigned)));

  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                         Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                         Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                         Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                         Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(0, 8, Unsigned)));

  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                         Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                         Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                         Interval(Int(4, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned)));

  BOOST_CHECK(lshr_exact(Interval(Int(1, 8, Unsigned)),
                         Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      lshr_exact(Interval(Int(1, 8, Unsigned)), Interval::top(8, Unsigned)) ==
      Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(1, 8, Unsigned)),
                         Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(1, 8, Unsigned)),
                         Interval(Int(1, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(1, 8, Unsigned)),
                         Interval(Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(1, 8, Unsigned)),
                         Interval(Int(6, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(1, 8, Unsigned)),
                         Interval(Int(7, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned)));

  BOOST_CHECK(lshr_exact(Interval(Int(127, 8, Unsigned)),
                         Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      lshr_exact(Interval(Int(127, 8, Unsigned)), Interval::top(8, Unsigned)) ==
      Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(127, 8, Unsigned)),
                         Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(127, 8, Unsigned)),
                         Interval(Int(1, 8, Unsigned))) ==
              Interval(Int(63, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(127, 8, Unsigned)),
                         Interval(Int(2, 8, Unsigned))) ==
              Interval(Int(31, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(127, 8, Unsigned)),
                         Interval(Int(6, 8, Unsigned))) ==
              Interval(Int(1, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(127, 8, Unsigned)),
                         Interval(Int(7, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned)));

  BOOST_CHECK(lshr_exact(Interval(Int(128, 8, Unsigned)),
                         Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      lshr_exact(Interval(Int(128, 8, Unsigned)), Interval::top(8, Unsigned)) ==
      Interval(Int(1, 8, Unsigned), Int(128, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(128, 8, Unsigned)),
                         Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval(Int(1, 8, Unsigned), Int(128, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(128, 8, Unsigned)),
                         Interval(Int(1, 8, Unsigned))) ==
              Interval(Int(64, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(128, 8, Unsigned)),
                         Interval(Int(2, 8, Unsigned))) ==
              Interval(Int(32, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(128, 8, Unsigned)),
                         Interval(Int(6, 8, Unsigned))) ==
              Interval(Int(2, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(128, 8, Unsigned)),
                         Interval(Int(7, 8, Unsigned))) ==
              Interval(Int(1, 8, Unsigned)));

  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                         Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                         Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(lshr_exact(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                         Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(0, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_ashr) {
  BOOST_CHECK(ashr(Interval::top(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(ashr(Interval::top(8, Signed), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(ashr(Interval::bottom(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(ashr(Interval::bottom(8, Signed), Interval::top(8, Signed)) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(ashr(Interval(Int(0, 8, Signed)), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Signed)), Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Signed)),
                   Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));

  BOOST_CHECK(ashr(Interval(Int(1, 8, Signed)), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(ashr(Interval(Int(1, 8, Signed)), Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(1, 8, Signed)),
                   Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(1, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(1, 8, Signed)), Interval(Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(1, 8, Signed)), Interval(Int(6, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(1, 8, Signed)), Interval(Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));

  BOOST_CHECK(ashr(Interval(Int(127, 8, Signed)),
                   Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(ashr(Interval(Int(127, 8, Signed)), Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(127, 8, Signed)),
                   Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(
      ashr(Interval(Int(127, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(63, 8, Signed)));
  BOOST_CHECK(
      ashr(Interval(Int(127, 8, Signed)), Interval(Int(2, 8, Signed))) ==
      Interval(Int(31, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(127, 8, Signed)),
                   Interval(Int(6, 8, Signed))) == Interval(Int(1, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(127, 8, Signed)),
                   Interval(Int(7, 8, Signed))) == Interval(Int(0, 8, Signed)));

  BOOST_CHECK(ashr(Interval(Int(-128, 8, Signed)),
                   Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(ashr(Interval(Int(-128, 8, Signed)), Interval::top(8, Signed)) ==
              Interval(Int(-128, 8, Signed), Int(-1, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(-128, 8, Signed)),
                   Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(-128, 8, Signed), Int(-1, 8, Signed)));
  BOOST_CHECK(
      ashr(Interval(Int(-128, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(-64, 8, Signed)));
  BOOST_CHECK(
      ashr(Interval(Int(-128, 8, Signed)), Interval(Int(2, 8, Signed))) ==
      Interval(Int(-32, 8, Signed)));
  BOOST_CHECK(
      ashr(Interval(Int(-128, 8, Signed)), Interval(Int(6, 8, Signed))) ==
      Interval(Int(-2, 8, Signed)));
  BOOST_CHECK(
      ashr(Interval(Int(-128, 8, Signed)), Interval(Int(7, 8, Signed))) ==
      Interval(Int(-1, 8, Signed)));

  BOOST_CHECK(ashr(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                   Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                   Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                   Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                   Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));

  BOOST_CHECK(ashr(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(63, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(7, 8, Signed))) == Interval(Int(0, 8, Signed)));

  BOOST_CHECK(ashr(Interval(Int(1, 8, Signed), Int(2, 8, Signed)),
                   Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(1, 8, Signed), Int(126, 8, Signed)),
                   Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Signed), Int(126, 8, Signed)),
                   Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)),
                   Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(-1, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(-2, 8, Signed), Int(0, 8, Signed)),
                   Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(-1, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(ashr(Interval::top(8, Signed), Interval(Int(0, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(ashr(Interval::top(8, Signed), Interval(Int(4, 8, Signed))) ==
              Interval(Int(-8, 8, Signed), Int(7, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(1, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(4, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(7, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                   Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(-8, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(7, 8, Signed)));
  BOOST_CHECK(ashr(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                   Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(-8, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(
      ashr(Interval(Int(0, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Unsigned)), Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Interval(Int(0, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Interval(Int(1, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Interval(Int(255, 8, Unsigned)), Interval(Int(0, 8, Unsigned))) ==
      Interval(Int(255, 8, Unsigned)));

  BOOST_CHECK(ashr(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                   Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                   Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(ashr(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                   Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(ashr(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                   Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(0, 8, Unsigned)));

  BOOST_CHECK(ashr(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                   Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                   Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                   Interval(Int(4, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned)));

  BOOST_CHECK(
      ashr(Interval(Int(1, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(ashr(Interval(Int(1, 8, Unsigned)), Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(ashr(Interval(Int(1, 8, Unsigned)),
                   Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Interval(Int(1, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Interval(Int(1, 8, Unsigned)), Interval(Int(2, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Interval(Int(1, 8, Unsigned)), Interval(Int(6, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Interval(Int(1, 8, Unsigned)), Interval(Int(7, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));

  BOOST_CHECK(
      ashr(Interval(Int(127, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      ashr(Interval(Int(127, 8, Unsigned)), Interval::top(8, Unsigned)) ==
      Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(ashr(Interval(Int(127, 8, Unsigned)),
                   Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Interval(Int(127, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(63, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Interval(Int(127, 8, Unsigned)), Interval(Int(2, 8, Unsigned))) ==
      Interval(Int(31, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Interval(Int(127, 8, Unsigned)), Interval(Int(6, 8, Unsigned))) ==
      Interval(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Interval(Int(127, 8, Unsigned)), Interval(Int(7, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));

  BOOST_CHECK(
      ashr(Interval(Int(128, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      ashr(Interval(Int(128, 8, Unsigned)), Interval::top(8, Unsigned)) ==
      Interval(Int(128, 8, Unsigned), Int(255, 8, Unsigned)));
  BOOST_CHECK(ashr(Interval(Int(128, 8, Unsigned)),
                   Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval(Int(128, 8, Unsigned), Int(255, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Interval(Int(128, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(192, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Interval(Int(128, 8, Unsigned)), Interval(Int(2, 8, Unsigned))) ==
      Interval(Int(224, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Interval(Int(128, 8, Unsigned)), Interval(Int(6, 8, Unsigned))) ==
      Interval(Int(254, 8, Unsigned)));
  BOOST_CHECK(
      ashr(Interval(Int(128, 8, Unsigned)), Interval(Int(7, 8, Unsigned))) ==
      Interval(Int(255, 8, Unsigned)));

  BOOST_CHECK(ashr(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                   Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                   Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(ashr(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                   Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(0, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_ashr_exact) {
  BOOST_CHECK(
      ashr_exact(Interval::top(8, Signed), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(ashr_exact(Interval::top(8, Signed), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(
      ashr_exact(Interval::bottom(8, Signed), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Interval::bottom(8, Signed), Interval::top(8, Signed)) ==
      Interval::bottom(8, Signed));

  BOOST_CHECK(
      ashr_exact(Interval(Int(0, 8, Signed)), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Interval(Int(0, 8, Signed)), Interval::top(8, Signed)) ==
      Interval(Int(0, 8, Signed)));
  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Signed)),
                         Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Interval(Int(0, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(0, 8, Signed)));

  BOOST_CHECK(
      ashr_exact(Interval(Int(1, 8, Signed)), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Interval(Int(1, 8, Signed)), Interval::top(8, Signed)) ==
      Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(ashr_exact(Interval(Int(1, 8, Signed)),
                         Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Interval(Int(1, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(0, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Interval(Int(1, 8, Signed)), Interval(Int(2, 8, Signed))) ==
      Interval(Int(0, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Interval(Int(1, 8, Signed)), Interval(Int(6, 8, Signed))) ==
      Interval(Int(0, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Interval(Int(1, 8, Signed)), Interval(Int(7, 8, Signed))) ==
      Interval(Int(0, 8, Signed)));

  BOOST_CHECK(
      ashr_exact(Interval(Int(127, 8, Signed)), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Interval(Int(127, 8, Signed)), Interval::top(8, Signed)) ==
      Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(ashr_exact(Interval(Int(127, 8, Signed)),
                         Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Interval(Int(127, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(63, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Interval(Int(127, 8, Signed)), Interval(Int(2, 8, Signed))) ==
      Interval(Int(31, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Interval(Int(127, 8, Signed)), Interval(Int(6, 8, Signed))) ==
      Interval(Int(1, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Interval(Int(127, 8, Signed)), Interval(Int(7, 8, Signed))) ==
      Interval(Int(0, 8, Signed)));

  BOOST_CHECK(
      ashr_exact(Interval(Int(-128, 8, Signed)), Interval::bottom(8, Signed)) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      ashr_exact(Interval(Int(-128, 8, Signed)), Interval::top(8, Signed)) ==
      Interval(Int(-128, 8, Signed), Int(-1, 8, Signed)));
  BOOST_CHECK(ashr_exact(Interval(Int(-128, 8, Signed)),
                         Interval(Int(0, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(-128, 8, Signed), Int(-1, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Interval(Int(-128, 8, Signed)), Interval(Int(1, 8, Signed))) ==
      Interval(Int(-64, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Interval(Int(-128, 8, Signed)), Interval(Int(2, 8, Signed))) ==
      Interval(Int(-32, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Interval(Int(-128, 8, Signed)), Interval(Int(6, 8, Signed))) ==
      Interval(Int(-2, 8, Signed)));
  BOOST_CHECK(
      ashr_exact(Interval(Int(-128, 8, Signed)), Interval(Int(7, 8, Signed))) ==
      Interval(Int(-1, 8, Signed)));

  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                         Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                         Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                         Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                         Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));

  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                         Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                         Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                         Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                         Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(63, 8, Signed)));
  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                         Interval(Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));

  BOOST_CHECK(ashr_exact(Interval(Int(1, 8, Signed), Int(2, 8, Signed)),
                         Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(ashr_exact(Interval(Int(1, 8, Signed), Int(126, 8, Signed)),
                         Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Signed), Int(126, 8, Signed)),
                         Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(ashr_exact(Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)),
                         Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(-1, 8, Signed)));
  BOOST_CHECK(ashr_exact(Interval(Int(-2, 8, Signed), Int(0, 8, Signed)),
                         Interval(Int(7, 8, Signed), Int(7, 8, Signed))) ==
              Interval(Int(-1, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(
      ashr_exact(Interval::top(8, Signed), Interval(Int(0, 8, Signed))) ==
      Interval::top(8, Signed));
  BOOST_CHECK(
      ashr_exact(Interval::top(8, Signed), Interval(Int(4, 8, Signed))) ==
      Interval(Int(-8, 8, Signed), Int(7, 8, Signed)));
  BOOST_CHECK(ashr_exact(Interval(Int(1, 8, Signed), Int(127, 8, Signed)),
                         Interval(Int(4, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(7, 8, Signed)));
  BOOST_CHECK(ashr_exact(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                         Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(-8, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                         Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(7, 8, Signed)));
  BOOST_CHECK(ashr_exact(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                         Interval(Int(4, 8, Signed), Int(4, 8, Signed))) ==
              Interval(Int(-8, 8, Signed), Int(0, 8, Signed)));

  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Unsigned)),
                         Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      ashr_exact(Interval(Int(0, 8, Unsigned)), Interval::top(8, Unsigned)) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Unsigned)),
                         Interval(Int(1, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(1, 8, Unsigned)),
                         Interval(Int(1, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(255, 8, Unsigned)),
                         Interval(Int(0, 8, Unsigned))) ==
              Interval(Int(255, 8, Unsigned)));

  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                         Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                         Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                         Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                         Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(0, 8, Unsigned)));

  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                         Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                         Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                         Interval(Int(4, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned)));

  BOOST_CHECK(ashr_exact(Interval(Int(1, 8, Unsigned)),
                         Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      ashr_exact(Interval(Int(1, 8, Unsigned)), Interval::top(8, Unsigned)) ==
      Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(1, 8, Unsigned)),
                         Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(1, 8, Unsigned)),
                         Interval(Int(1, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(1, 8, Unsigned)),
                         Interval(Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(1, 8, Unsigned)),
                         Interval(Int(6, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(1, 8, Unsigned)),
                         Interval(Int(7, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned)));

  BOOST_CHECK(ashr_exact(Interval(Int(127, 8, Unsigned)),
                         Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      ashr_exact(Interval(Int(127, 8, Unsigned)), Interval::top(8, Unsigned)) ==
      Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(127, 8, Unsigned)),
                         Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(127, 8, Unsigned)),
                         Interval(Int(1, 8, Unsigned))) ==
              Interval(Int(63, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(127, 8, Unsigned)),
                         Interval(Int(2, 8, Unsigned))) ==
              Interval(Int(31, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(127, 8, Unsigned)),
                         Interval(Int(6, 8, Unsigned))) ==
              Interval(Int(1, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(127, 8, Unsigned)),
                         Interval(Int(7, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned)));

  BOOST_CHECK(ashr_exact(Interval(Int(128, 8, Unsigned)),
                         Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      ashr_exact(Interval(Int(128, 8, Unsigned)), Interval::top(8, Unsigned)) ==
      Interval(Int(128, 8, Unsigned), Int(255, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(128, 8, Unsigned)),
                         Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned))) ==
              Interval(Int(128, 8, Unsigned), Int(255, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(128, 8, Unsigned)),
                         Interval(Int(1, 8, Unsigned))) ==
              Interval(Int(192, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(128, 8, Unsigned)),
                         Interval(Int(2, 8, Unsigned))) ==
              Interval(Int(224, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(128, 8, Unsigned)),
                         Interval(Int(6, 8, Unsigned))) ==
              Interval(Int(254, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(128, 8, Unsigned)),
                         Interval(Int(7, 8, Unsigned))) ==
              Interval(Int(255, 8, Unsigned)));

  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                         Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                         Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(ashr_exact(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                         Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(0, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_and) {
  BOOST_CHECK(and_(Interval::top(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(and_(Interval::top(8, Signed), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(and_(Interval::bottom(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(and_(Interval::bottom(8, Signed), Interval::top(8, Signed)) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(and_(Interval(Int(0, 8, Signed)), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(and_(Interval(Int(0, 8, Signed)), Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(and_(Interval(Int(0, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(and_(Interval(Int(1, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(1, 8, Signed)));
  BOOST_CHECK(
      and_(Interval(Int(-128, 8, Signed)), Interval(Int(127, 8, Signed))) ==
      Interval(Int(0, 8, Signed)));

  BOOST_CHECK(and_(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                   Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(and_(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                   Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(and_(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                   Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(and_(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                   Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));

  BOOST_CHECK(and_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(and_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval::top(8, Signed)) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(and_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(and_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(-128, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));

  BOOST_CHECK(and_(Interval(Int(1, 8, Signed), Int(2, 8, Signed)),
                   Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(2, 8, Signed)));
  BOOST_CHECK(and_(Interval(Int(1, 8, Signed), Int(126, 8, Signed)),
                   Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(126, 8, Signed)));
  BOOST_CHECK(and_(Interval(Int(0, 8, Signed), Int(126, 8, Signed)),
                   Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(126, 8, Signed)));
  BOOST_CHECK(and_(Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)),
                   Interval(Int(-128, 8, Signed), Int(-128, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(and_(Interval(Int(-2, 8, Signed), Int(0, 8, Signed)),
                   Interval(Int(-128, 8, Signed), Int(-128, 8, Signed))) ==
              Interval::top(8, Signed));

  BOOST_CHECK(and_(Interval::top(8, Signed), Interval(Int(0, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(and_(Interval::top(8, Signed), Interval(Int(42, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(42, 8, Signed)));
  BOOST_CHECK(and_(Interval(Int(1, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(42, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(42, 8, Signed)));
  BOOST_CHECK(and_(Interval(Int(1, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(-52, 8, Signed), Int(42, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(and_(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                   Interval(Int(-52, 8, Signed), Int(42, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(and_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(-128, 8, Signed), Int(42, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(and_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(-128, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(and_(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                   Interval(Int(-128, 8, Signed), Int(42, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(and_(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                   Interval(Int(-128, 8, Signed), Int(127, 8, Signed))) ==
              Interval::top(8, Signed));

  BOOST_CHECK(
      and_(Interval(Int(0, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(and_(Interval(Int(0, 8, Unsigned)), Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      and_(Interval(Int(0, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      and_(Interval(Int(1, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      and_(Interval(Int(255, 8, Unsigned)), Interval(Int(0, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));

  BOOST_CHECK(and_(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                   Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(and_(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                   Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(and_(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                   Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(and_(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                   Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(3, 8, Unsigned)));

  BOOST_CHECK(and_(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                   Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(and_(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                   Interval::top(8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));
  BOOST_CHECK(and_(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                   Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)));

  BOOST_CHECK(and_(Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned)),
                   Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned)));
  BOOST_CHECK(and_(Interval(Int(1, 8, Unsigned), Int(255, 8, Unsigned)),
                   Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval(Int(1, 8, Unsigned), Int(255, 8, Unsigned)));
  BOOST_CHECK(and_(Interval(Int(0, 8, Unsigned), Int(255, 8, Unsigned)),
                   Interval(Int(255, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(255, 8, Unsigned)));
  BOOST_CHECK(and_(Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned)),
                   Interval(Int(128, 8, Unsigned), Int(128, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(128, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_or) {
  BOOST_CHECK(or_(Interval::top(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(or_(Interval::top(8, Signed), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(or_(Interval::bottom(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(or_(Interval::bottom(8, Signed), Interval::top(8, Signed)) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(or_(Interval(Int(0, 8, Signed)), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(or_(Interval(Int(0, 8, Signed)), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(or_(Interval(Int(0, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(1, 8, Signed)));
  BOOST_CHECK(or_(Interval(Int(1, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(1, 8, Signed)));
  BOOST_CHECK(
      or_(Interval(Int(-128, 8, Signed)), Interval(Int(127, 8, Signed))) ==
      Interval(Int(-1, 8, Signed)));

  BOOST_CHECK(or_(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(or_(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval::top(8, Signed)) == Interval::top(8, Signed));
  BOOST_CHECK(or_(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(or_(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                  Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(3, 8, Signed)));

  BOOST_CHECK(or_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(or_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval::top(8, Signed)) == Interval::top(8, Signed));
  BOOST_CHECK(or_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(or_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(-128, 8, Signed))) == Interval::top(8, Signed));

  BOOST_CHECK(or_(Interval(Int(1, 8, Signed), Int(2, 8, Signed)),
                  Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(or_(Interval(Int(1, 8, Signed), Int(126, 8, Signed)),
                  Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(or_(Interval(Int(0, 8, Signed), Int(126, 8, Signed)),
                  Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(or_(Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(-128, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(or_(Interval(Int(-2, 8, Signed), Int(0, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(-128, 8, Signed))) ==
              Interval::top(8, Signed));

  BOOST_CHECK(or_(Interval::top(8, Signed), Interval(Int(0, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(or_(Interval::top(8, Signed), Interval(Int(42, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(or_(Interval(Int(1, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(42, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(or_(Interval(Int(1, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(-52, 8, Signed), Int(42, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(or_(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                  Interval(Int(-52, 8, Signed), Int(42, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(or_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(42, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(or_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(127, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(or_(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(42, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(or_(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                  Interval(Int(-128, 8, Signed), Int(127, 8, Signed))) ==
              Interval::top(8, Signed));

  BOOST_CHECK(
      or_(Interval(Int(0, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(or_(Interval(Int(0, 8, Unsigned)), Interval::top(8, Unsigned)) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(
      or_(Interval(Int(0, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      or_(Interval(Int(1, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      or_(Interval(Int(255, 8, Unsigned)), Interval(Int(0, 8, Unsigned))) ==
      Interval(Int(255, 8, Unsigned)));

  BOOST_CHECK(or_(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                  Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(or_(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                  Interval::top(8, Unsigned)) == Interval::top(8, Unsigned));
  BOOST_CHECK(or_(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                  Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(3, 8, Unsigned)));
  BOOST_CHECK(or_(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                  Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned)));

  BOOST_CHECK(or_(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                  Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(or_(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                  Interval::top(8, Unsigned)) == Interval::top(8, Unsigned));
  BOOST_CHECK(or_(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                  Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval::top(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_xor) {
  BOOST_CHECK(xor_(Interval::top(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(xor_(Interval::top(8, Signed), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(xor_(Interval::bottom(8, Signed), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(xor_(Interval::bottom(8, Signed), Interval::top(8, Signed)) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(xor_(Interval(Int(0, 8, Signed)), Interval::bottom(8, Signed)) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(xor_(Interval(Int(0, 8, Signed)), Interval::top(8, Signed)) ==
              Interval::top(8, Signed));
  BOOST_CHECK(xor_(Interval(Int(0, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(1, 8, Signed)));
  BOOST_CHECK(xor_(Interval(Int(1, 8, Signed)), Interval(Int(1, 8, Signed))) ==
              Interval(Int(0, 8, Signed)));
  BOOST_CHECK(
      xor_(Interval(Int(-128, 8, Signed)), Interval(Int(127, 8, Signed))) ==
      Interval(Int(-1, 8, Signed)));

  BOOST_CHECK(xor_(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                   Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(xor_(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                   Interval::top(8, Signed)) == Interval::top(8, Signed));
  BOOST_CHECK(xor_(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                   Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(xor_(Interval(Int(0, 8, Signed), Int(1, 8, Signed)),
                   Interval(Int(1, 8, Signed), Int(2, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(3, 8, Signed)));

  BOOST_CHECK(xor_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval::bottom(8, Signed)) == Interval::bottom(8, Signed));
  BOOST_CHECK(xor_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval::top(8, Signed)) == Interval::top(8, Signed));
  BOOST_CHECK(xor_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(-1, 8, Signed), Int(0, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(xor_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(-128, 8, Signed))) == Interval::top(8, Signed));

  BOOST_CHECK(xor_(Interval(Int(1, 8, Signed), Int(2, 8, Signed)),
                   Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(xor_(Interval(Int(1, 8, Signed), Int(126, 8, Signed)),
                   Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(xor_(Interval(Int(0, 8, Signed), Int(126, 8, Signed)),
                   Interval(Int(127, 8, Signed), Int(127, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(xor_(Interval(Int(-2, 8, Signed), Int(-1, 8, Signed)),
                   Interval(Int(-128, 8, Signed), Int(-128, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(xor_(Interval(Int(-2, 8, Signed), Int(0, 8, Signed)),
                   Interval(Int(-128, 8, Signed), Int(-128, 8, Signed))) ==
              Interval::top(8, Signed));

  BOOST_CHECK(xor_(Interval::top(8, Signed), Interval(Int(0, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(xor_(Interval::top(8, Signed), Interval(Int(42, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(xor_(Interval(Int(1, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(42, 8, Signed))) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(xor_(Interval(Int(1, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(-52, 8, Signed), Int(42, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(xor_(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                   Interval(Int(-52, 8, Signed), Int(42, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(xor_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(-128, 8, Signed), Int(42, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(xor_(Interval(Int(0, 8, Signed), Int(127, 8, Signed)),
                   Interval(Int(-128, 8, Signed), Int(127, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(xor_(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                   Interval(Int(-128, 8, Signed), Int(42, 8, Signed))) ==
              Interval::top(8, Signed));
  BOOST_CHECK(xor_(Interval(Int(-128, 8, Signed), Int(0, 8, Signed)),
                   Interval(Int(-128, 8, Signed), Int(127, 8, Signed))) ==
              Interval::top(8, Signed));

  BOOST_CHECK(
      xor_(Interval(Int(0, 8, Unsigned)), Interval::bottom(8, Unsigned)) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(xor_(Interval(Int(0, 8, Unsigned)), Interval::top(8, Unsigned)) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(
      xor_(Interval(Int(0, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      xor_(Interval(Int(1, 8, Unsigned)), Interval(Int(1, 8, Unsigned))) ==
      Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      xor_(Interval(Int(255, 8, Unsigned)), Interval(Int(0, 8, Unsigned))) ==
      Interval(Int(255, 8, Unsigned)));

  BOOST_CHECK(xor_(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                   Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(xor_(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)),
                   Interval::top(8, Unsigned)) == Interval::top(8, Unsigned));
  BOOST_CHECK(xor_(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                   Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(3, 8, Unsigned)));
  BOOST_CHECK(xor_(Interval(Int(2, 8, Unsigned), Int(3, 8, Unsigned)),
                   Interval(Int(3, 8, Unsigned), Int(4, 8, Unsigned))) ==
              Interval(Int(0, 8, Unsigned), Int(7, 8, Unsigned)));

  BOOST_CHECK(xor_(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                   Interval::bottom(8, Unsigned)) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(xor_(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                   Interval::top(8, Unsigned)) == Interval::top(8, Unsigned));
  BOOST_CHECK(xor_(Interval(Int(0, 8, Unsigned), Int(127, 8, Unsigned)),
                   Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned))) ==
              Interval::top(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_to_z_interval) {
  BOOST_CHECK(Interval::bottom(8, Signed).to_z_interval() ==
              ZInterval::bottom());
  BOOST_CHECK(Interval::top(8, Signed).to_z_interval() ==
              ZInterval(ZBound(-128), ZBound(127)));
  BOOST_CHECK(Interval(Int(0, 8, Signed), Int(1, 8, Signed)).to_z_interval() ==
              ZInterval(ZBound(0), ZBound(1)));
  BOOST_CHECK(Interval(Int(-1, 8, Signed), Int(1, 8, Signed)).to_z_interval() ==
              ZInterval(ZBound(-1), ZBound(1)));

  BOOST_CHECK(Interval::bottom(8, Unsigned).to_z_interval() ==
              ZInterval::bottom());
  BOOST_CHECK(Interval::top(8, Unsigned).to_z_interval() ==
              ZInterval(ZBound(0), ZBound(255)));
  BOOST_CHECK(
      Interval(Int(1, 8, Unsigned), Int(2, 8, Unsigned)).to_z_interval() ==
      ZInterval(ZBound(1), ZBound(2)));
  BOOST_CHECK(
      Interval(Int(1, 8, Unsigned), Int(254, 8, Unsigned)).to_z_interval() ==
      ZInterval(ZBound(1), ZBound(254)));
}

BOOST_AUTO_TEST_CASE(test_from_z_interval_wrap) {
  using WrapTag = Interval::WrapTag;

  BOOST_CHECK(
      Interval::from_z_interval(ZInterval::bottom(), 8, Signed, WrapTag{}) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      Interval::from_z_interval(ZInterval::top(), 8, Signed, WrapTag{}) ==
      Interval::top(8, Signed));
  BOOST_CHECK(
      Interval::from_z_interval(ZInterval(ZBound::minus_infinity(), ZBound(0)),
                                8,
                                Signed,
                                WrapTag{}) == Interval::top(8, Signed));
  BOOST_CHECK(
      Interval::from_z_interval(ZInterval(ZBound(0), ZBound::plus_infinity()),
                                8,
                                Signed,
                                WrapTag{}) == Interval::top(8, Signed));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(0), ZBound(1)),
                                        8,
                                        Signed,
                                        WrapTag{}) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(0), ZBound(127)),
                                        8,
                                        Signed,
                                        WrapTag{}) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(0), ZBound(128)),
                                        8,
                                        Signed,
                                        WrapTag{}) == Interval::top(8, Signed));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(128), ZBound(129)),
                                        8,
                                        Signed,
                                        WrapTag{}) ==
              Interval(Int(-128, 8, Signed), Int(-127, 8, Signed)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(128), ZBound(256)),
                                        8,
                                        Signed,
                                        WrapTag{}) ==
              Interval(Int(-128, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(128), ZBound(383)),
                                        8,
                                        Signed,
                                        WrapTag{}) ==
              Interval(Int(-128, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-128), ZBound(-127)),
                                        8,
                                        Signed,
                                        WrapTag{}) ==
              Interval(Int(-128, 8, Signed), Int(-127, 8, Signed)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-129), ZBound(-127)),
                                        8,
                                        Signed,
                                        WrapTag{}) == Interval::top(8, Signed));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-130), ZBound(-129)),
                                        8,
                                        Signed,
                                        WrapTag{}) ==
              Interval(Int(126, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-383), ZBound(-129)),
                                        8,
                                        Signed,
                                        WrapTag{}) ==
              Interval(Int(-127, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-385), ZBound(-129)),
                                        8,
                                        Signed,
                                        WrapTag{}) == Interval::top(8, Signed));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-385), ZBound(-383)),
                                        8,
                                        Signed,
                                        WrapTag{}) == Interval::top(8, Signed));

  BOOST_CHECK(
      Interval::from_z_interval(ZInterval::bottom(), 8, Unsigned, WrapTag{}) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      Interval::from_z_interval(ZInterval::top(), 8, Unsigned, WrapTag{}) ==
      Interval::top(8, Unsigned));
  BOOST_CHECK(
      Interval::from_z_interval(ZInterval(ZBound::minus_infinity(), ZBound(0)),
                                8,
                                Unsigned,
                                WrapTag{}) == Interval::top(8, Unsigned));
  BOOST_CHECK(
      Interval::from_z_interval(ZInterval(ZBound(0), ZBound::plus_infinity()),
                                8,
                                Unsigned,
                                WrapTag{}) == Interval::top(8, Unsigned));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(0), ZBound(1)),
                                        8,
                                        Unsigned,
                                        WrapTag{}) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(0), ZBound(254)),
                                        8,
                                        Unsigned,
                                        WrapTag{}) ==
              Interval(Int(0, 8, Unsigned), Int(254, 8, Unsigned)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(0), ZBound(255)),
                                        8,
                                        Unsigned,
                                        WrapTag{}) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(256), ZBound(257)),
                                        8,
                                        Unsigned,
                                        WrapTag{}) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(256), ZBound(510)),
                                        8,
                                        Unsigned,
                                        WrapTag{}) ==
              Interval(Int(0, 8, Unsigned), Int(254, 8, Unsigned)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-2), ZBound(-1)),
                                        8,
                                        Unsigned,
                                        WrapTag{}) ==
              Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-256), ZBound(-1)),
                                        8,
                                        Unsigned,
                                        WrapTag{}) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-257), ZBound(-256)),
                                        8,
                                        Unsigned,
                                        WrapTag{}) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-258), ZBound(-257)),
                                        8,
                                        Unsigned,
                                        WrapTag{}) ==
              Interval(Int(254, 8, Unsigned), Int(255, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_from_z_interval_trunc) {
  using TruncTag = Interval::TruncTag;

  BOOST_CHECK(
      Interval::from_z_interval(ZInterval::bottom(), 8, Signed, TruncTag{}) ==
      Interval::bottom(8, Signed));
  BOOST_CHECK(
      Interval::from_z_interval(ZInterval::top(), 8, Signed, TruncTag{}) ==
      Interval::top(8, Signed));
  BOOST_CHECK(
      Interval::from_z_interval(ZInterval(ZBound::minus_infinity(), ZBound(0)),
                                8,
                                Signed,
                                TruncTag{}) ==
      Interval(Int(-128, 8, Signed), Int(0, 8, Signed)));
  BOOST_CHECK(
      Interval::from_z_interval(ZInterval(ZBound(0), ZBound::plus_infinity()),
                                8,
                                Signed,
                                TruncTag{}) ==
      Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(0), ZBound(1)),
                                        8,
                                        Signed,
                                        TruncTag{}) ==
              Interval(Int(0, 8, Signed), Int(1, 8, Signed)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(0), ZBound(127)),
                                        8,
                                        Signed,
                                        TruncTag{}) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(0), ZBound(128)),
                                        8,
                                        Signed,
                                        TruncTag{}) ==
              Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(128), ZBound(129)),
                                        8,
                                        Signed,
                                        TruncTag{}) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(128), ZBound(256)),
                                        8,
                                        Signed,
                                        TruncTag{}) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(128), ZBound(383)),
                                        8,
                                        Signed,
                                        TruncTag{}) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-128), ZBound(-127)),
                                        8,
                                        Signed,
                                        TruncTag{}) ==
              Interval(Int(-128, 8, Signed), Int(-127, 8, Signed)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-129), ZBound(-127)),
                                        8,
                                        Signed,
                                        TruncTag{}) ==
              Interval(Int(-128, 8, Signed), Int(-127, 8, Signed)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-130), ZBound(-129)),
                                        8,
                                        Signed,
                                        TruncTag{}) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-383), ZBound(-129)),
                                        8,
                                        Signed,
                                        TruncTag{}) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-385), ZBound(-129)),
                                        8,
                                        Signed,
                                        TruncTag{}) ==
              Interval::bottom(8, Signed));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-385), ZBound(-383)),
                                        8,
                                        Signed,
                                        TruncTag{}) ==
              Interval::bottom(8, Signed));

  BOOST_CHECK(
      Interval::from_z_interval(ZInterval::bottom(), 8, Unsigned, TruncTag{}) ==
      Interval::bottom(8, Unsigned));
  BOOST_CHECK(
      Interval::from_z_interval(ZInterval::top(), 8, Unsigned, TruncTag{}) ==
      Interval::top(8, Unsigned));
  BOOST_CHECK(
      Interval::from_z_interval(ZInterval(ZBound::minus_infinity(), ZBound(0)),
                                8,
                                Unsigned,
                                TruncTag{}) == Interval(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      Interval::from_z_interval(ZInterval(ZBound(0), ZBound::plus_infinity()),
                                8,
                                Unsigned,
                                TruncTag{}) == Interval::top(8, Unsigned));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(0), ZBound(1)),
                                        8,
                                        Unsigned,
                                        TruncTag{}) ==
              Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(0), ZBound(254)),
                                        8,
                                        Unsigned,
                                        TruncTag{}) ==
              Interval(Int(0, 8, Unsigned), Int(254, 8, Unsigned)));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(0), ZBound(255)),
                                        8,
                                        Unsigned,
                                        TruncTag{}) ==
              Interval::top(8, Unsigned));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(256), ZBound(257)),
                                        8,
                                        Unsigned,
                                        TruncTag{}) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(256), ZBound(510)),
                                        8,
                                        Unsigned,
                                        TruncTag{}) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-2), ZBound(-1)),
                                        8,
                                        Unsigned,
                                        TruncTag{}) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-256), ZBound(-1)),
                                        8,
                                        Unsigned,
                                        TruncTag{}) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-257), ZBound(-256)),
                                        8,
                                        Unsigned,
                                        TruncTag{}) ==
              Interval::bottom(8, Unsigned));
  BOOST_CHECK(Interval::from_z_interval(ZInterval(ZBound(-258), ZBound(-257)),
                                        8,
                                        Unsigned,
                                        TruncTag{}) ==
              Interval::bottom(8, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_widening_threshold) {
  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(128, 8, Unsigned))
                  .widening_threshold(Interval(Int(0, 8, Unsigned),
                                               Int(140, 8, Unsigned)),
                                      Int(142, 8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(142, 8, Unsigned)));

  BOOST_CHECK(
      Interval(Int(0, 8, Signed), Int(10, 8, Signed))
          .widening_threshold(Interval(Int(0, 8, Signed), Int(20, 8, Signed)),
                              Int(18, 8, Signed)) ==
      Interval(Int(0, 8, Signed), Int(127, 8, Signed)));

  BOOST_CHECK(Interval(Int(3, 8, Unsigned), Int(254, 8, Unsigned))
                  .widening_threshold(Interval(Int(1, 8, Unsigned),
                                               Int(254, 8, Unsigned)),
                                      Int(0, 8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(254, 8, Unsigned)));

  BOOST_CHECK(
      Interval(Int(42, 8, Signed), Int(43, 8, Signed))
          .widening_threshold(Interval(Int(41, 8, Signed), Int(42, 8, Signed)),
                              Int(42, 8, Signed)) ==
      Interval(Int(-128, 8, Signed), Int(43, 8, Signed)));

  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(42, 8, Unsigned))
                  .widening_threshold(Interval(Int(0, 8, Unsigned),
                                               Int(73, 8, Unsigned)),
                                      Int(73, 8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(73, 8, Unsigned)));

  BOOST_CHECK(
      Interval(Int(0, 8, Signed), Int(47, 8, Signed))
          .widening_threshold(Interval(Int(0, 8, Signed), Int(48, 8, Signed)),
                              Int(49, 32, Unsigned)) ==
      Interval(Int(0, 8, Signed), Int(49, 8, Signed)));

  BOOST_CHECK(
      Interval(Int(0, 8, Signed), Int(47, 8, Signed))
          .widening_threshold(Interval(Int(0, 8, Signed), Int(48, 8, Signed)),
                              Int(257, 32, Unsigned)) ==
      Interval(Int(0, 8, Signed), Int(127, 8, Signed)));
}

BOOST_AUTO_TEST_CASE(test_narrowing_threshold) {
  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(255, 8, Unsigned))
                  .narrowing_threshold(Interval(Int(0, 8, Unsigned),
                                                Int(200, 8, Unsigned)),
                                       Int(100, 8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(200, 8, Unsigned)));

  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(100, 8, Unsigned))
                  .narrowing_threshold(Interval(Int(0, 8, Unsigned),
                                                Int(80, 8, Unsigned)),
                                       Int(100, 8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(80, 8, Unsigned)));

  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(150, 8, Unsigned))
                  .narrowing_threshold(Interval(Int(0, 8, Unsigned),
                                                Int(80, 8, Unsigned)),
                                       Int(100, 8, Unsigned)) ==
              Interval(Int(0, 8, Unsigned), Int(150, 8, Unsigned)));

  BOOST_CHECK(Interval(Int(0, 8, Unsigned), Int(100, 8, Unsigned))
                  .narrowing_threshold(Interval(Int(5, 8, Unsigned),
                                                Int(100, 8, Unsigned)),
                                       Int(10, 8, Unsigned)) ==
              Interval(Int(5, 8, Unsigned), Int(100, 8, Unsigned)));

  BOOST_CHECK(Interval(Int(10, 8, Unsigned), Int(100, 8, Unsigned))
                  .narrowing_threshold(Interval(Int(20, 8, Unsigned),
                                                Int(100, 8, Unsigned)),
                                       Int(10, 8, Unsigned)) ==
              Interval(Int(20, 8, Unsigned), Int(100, 8, Unsigned)));

  BOOST_CHECK(Interval(Int(20, 8, Unsigned), Int(100, 8, Unsigned))
                  .narrowing_threshold(Interval(Int(30, 8, Unsigned),
                                                Int(100, 8, Unsigned)),
                                       Int(10, 8, Unsigned)) ==
              Interval(Int(20, 8, Unsigned), Int(100, 8, Unsigned)));

  BOOST_CHECK(
      Interval(Int(0, 8, Signed), Int(127, 8, Signed))
          .narrowing_threshold(Interval(Int(0, 8, Signed), Int(120, 8, Signed)),
                               Int(100, 8, Signed)) ==
      Interval(Int(0, 8, Signed), Int(120, 8, Signed)));

  BOOST_CHECK(
      Interval(Int(0, 8, Signed), Int(100, 8, Signed))
          .narrowing_threshold(Interval(Int(0, 8, Signed), Int(80, 8, Signed)),
                               Int(100, 8, Signed)) ==
      Interval(Int(0, 8, Signed), Int(80, 8, Signed)));

  BOOST_CHECK(
      Interval(Int(0, 8, Signed), Int(100, 8, Signed))
          .narrowing_threshold(Interval(Int(0, 8, Signed), Int(80, 8, Signed)),
                               Int(90, 8, Signed)) ==
      Interval(Int(0, 8, Signed), Int(100, 8, Signed)));

  BOOST_CHECK(Interval(Int(-128, 8, Signed), Int(100, 8, Signed))
                  .narrowing_threshold(Interval(Int(-120, 8, Signed),
                                                Int(100, 8, Signed)),
                                       Int(-100, 8, Signed)) ==
              Interval(Int(-120, 8, Signed), Int(100, 8, Signed)));

  BOOST_CHECK(Interval(Int(-100, 8, Signed), Int(100, 8, Signed))
                  .narrowing_threshold(Interval(Int(-80, 8, Signed),
                                                Int(100, 8, Signed)),
                                       Int(-100, 8, Signed)) ==
              Interval(Int(-80, 8, Signed), Int(100, 8, Signed)));

  BOOST_CHECK(Interval(Int(-90, 8, Signed), Int(100, 8, Signed))
                  .narrowing_threshold(Interval(Int(-80, 8, Signed),
                                                Int(100, 8, Signed)),
                                       Int(-100, 8, Signed)) ==
              Interval(Int(-90, 8, Signed), Int(100, 8, Signed)));
}
