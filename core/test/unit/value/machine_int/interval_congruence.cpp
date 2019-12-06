/*******************************************************************************
 *
 * Tests for machine_int::IntervalCongruence
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

#define BOOST_TEST_MODULE test_machine_integer_interval_congruence
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/number/machine_int.hpp>
#include <ikos/core/value/machine_int/interval_congruence.hpp>

using ikos::core::Signed;
using ikos::core::Unsigned;
using Z = ikos::core::ZNumber;
using Int = ikos::core::MachineInt;
using ZBound = ikos::core::ZBound;
using ZInterval = ikos::core::numeric::ZInterval;
using ZCongruence = ikos::core::numeric::ZCongruence;
using ZIntervalCongruence = ikos::core::numeric::IntervalCongruence< Z >;
using Interval = ikos::core::machine_int::Interval;
using Congruence = ikos::core::machine_int::Congruence;
using IntervalCongruence = ikos::core::machine_int::IntervalCongruence;

BOOST_AUTO_TEST_CASE(test_constructors) {
  // IntervalCongruence(Int)
  BOOST_CHECK(
      IntervalCongruence(Int(0, 8, Signed)) ==
      IntervalCongruence(Interval(Int(0, 8, Signed)),
                         Congruence(Int(0, 8, Signed), Int(0, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence(Int(1, 8, Signed)) ==
      IntervalCongruence(Interval(Int(1, 8, Signed)),
                         Congruence(Int(0, 8, Signed), Int(1, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence(Int(-128, 8, Signed)) ==
      IntervalCongruence(Interval(Int(-128, 8, Signed)),
                         Congruence(Int(0, 8, Signed), Int(-128, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence(Int(127, 8, Signed)) ==
      IntervalCongruence(Interval(Int(127, 8, Signed)),
                         Congruence(Int(0, 8, Signed), Int(127, 8, Signed))));

  BOOST_CHECK(
      IntervalCongruence(Int(0, 8, Unsigned)) ==
      IntervalCongruence(Interval(Int(0, 8, Unsigned)),
                         Congruence(Int(0, 8, Unsigned), Int(0, 8, Unsigned))));
  BOOST_CHECK(
      IntervalCongruence(Int(1, 8, Unsigned)) ==
      IntervalCongruence(Interval(Int(1, 8, Unsigned)),
                         Congruence(Int(0, 8, Unsigned), Int(1, 8, Unsigned))));
  BOOST_CHECK(IntervalCongruence(Int(255, 8, Unsigned)) ==
              IntervalCongruence(Interval(Int(255, 8, Unsigned)),
                                 Congruence(Int(0, 8, Unsigned),
                                            Int(255, 8, Unsigned))));

  // IntervalCongruence(Interval)
  BOOST_CHECK(
      IntervalCongruence(Interval(Int(1, 8, Signed))) ==
      IntervalCongruence(Interval(Int(1, 8, Signed)),
                         Congruence(Int(0, 8, Signed), Int(1, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence(Interval(Int(0, 8, Signed), Int(2, 8, Signed))) ==
      IntervalCongruence(Interval(Int(0, 8, Signed), Int(2, 8, Signed)),
                         Congruence(Int(1, 8, Signed), Int(0, 8, Signed))));

  BOOST_CHECK(
      IntervalCongruence(Interval(Int(1, 8, Unsigned))) ==
      IntervalCongruence(Interval(Int(1, 8, Unsigned)),
                         Congruence(Int(0, 8, Unsigned), Int(1, 8, Unsigned))));
  BOOST_CHECK(
      IntervalCongruence(Interval(Int(0, 8, Unsigned), Int(2, 8, Unsigned))) ==
      IntervalCongruence(Interval(Int(0, 8, Unsigned), Int(2, 8, Unsigned)),
                         Congruence(Int(1, 8, Unsigned), Int(0, 8, Unsigned))));

  // IntervalCongruence(Congruence)
  BOOST_CHECK(
      IntervalCongruence(Congruence(Int(1, 8, Signed))) ==
      IntervalCongruence(Interval(Int(1, 8, Signed)),
                         Congruence(Int(0, 8, Signed), Int(1, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence(Congruence(Int(2, 8, Signed), Int(0, 8, Signed))) ==
      IntervalCongruence(Interval::top(8, Signed),
                         Congruence(Int(2, 8, Signed), Int(0, 8, Signed))));

  BOOST_CHECK(
      IntervalCongruence(Congruence(Int(1, 8, Unsigned))) ==
      IntervalCongruence(Interval(Int(1, 8, Unsigned)),
                         Congruence(Int(0, 8, Unsigned), Int(1, 8, Unsigned))));
  BOOST_CHECK(
      IntervalCongruence(
          Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))) ==
      IntervalCongruence(Interval::top(8, Unsigned),
                         Congruence(Int(2, 8, Unsigned), Int(0, 8, Unsigned))));

  // IntervalCongruence(Interval a, Congruence b)
  BOOST_CHECK(
      IntervalCongruence(Interval::bottom(8, Signed),
                         Congruence(Int(0, 8, Signed), Int(1, 8, Signed)))
          .is_bottom());
  BOOST_CHECK(IntervalCongruence(Interval(Int(0, 8, Signed)),
                                 Congruence::bottom(8, Signed))
                  .is_bottom());
  BOOST_CHECK(
      IntervalCongruence(Interval(Int(2, 8, Signed)),
                         Congruence(Int(1, 8, Signed), Int(0, 8, Signed))) ==
      IntervalCongruence(Interval(Int(2, 8, Signed)),
                         Congruence(Int(0, 8, Signed), Int(2, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence(Interval(Int(0, 8, Signed)),
                         Congruence(Int(0, 8, Signed), Int(1, 8, Signed)))
          .is_bottom());
  BOOST_CHECK(
      IntervalCongruence(Interval(Int(0, 8, Signed), Int(4, 8, Signed)),
                         Congruence(Int(0, 8, Signed), Int(-1, 8, Signed)))
          .is_bottom());
  BOOST_CHECK(
      IntervalCongruence(Interval::top(8, Signed),
                         Congruence(Int(0, 8, Signed), Int(1, 8, Signed))) ==
      IntervalCongruence(Interval(Int(1, 8, Signed)),
                         Congruence(Int(0, 8, Signed), Int(1, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence(Interval(Int(0, 8, Signed), Int(12, 8, Signed)),
                         Congruence(Int(8, 8, Signed), Int(1, 8, Signed))) ==
      IntervalCongruence(Interval(Int(1, 8, Signed), Int(9, 8, Signed)),
                         Congruence(Int(8, 8, Signed), Int(1, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence(Interval(Int(0, 8, Signed), Int(2, 8, Signed)),
                         Congruence(Int(8, 8, Signed), Int(1, 8, Signed))) ==
      IntervalCongruence(Interval(Int(1, 8, Signed)),
                         Congruence(Int(8, 8, Signed), Int(1, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence(Interval(Int(2, 8, Signed), Int(8, 8, Signed)),
                         Congruence(Int(8, 8, Signed), Int(1, 8, Signed)))
          .is_bottom());

  BOOST_CHECK(
      IntervalCongruence::top(1, Signed) ==
      IntervalCongruence(Interval::top(1, Signed), Congruence::top(1, Signed)));
  BOOST_CHECK(
      IntervalCongruence::top(8, Signed) ==
      IntervalCongruence(Interval::top(8, Signed), Congruence::top(8, Signed)));

  BOOST_CHECK(IntervalCongruence::top(1, Unsigned) ==
              IntervalCongruence(Interval::top(1, Unsigned),
                                 Congruence::top(1, Unsigned)));
  BOOST_CHECK(IntervalCongruence::top(8, Unsigned) ==
              IntervalCongruence(Interval::top(8, Unsigned),
                                 Congruence::top(8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_from_z_interval_congruence_wrap) {
  using WrapTag = IntervalCongruence::WrapTag;

  BOOST_CHECK(IntervalCongruence::
                  from_z_interval_congruence(ZIntervalCongruence::bottom(),
                                             8,
                                             Signed,
                                             WrapTag{}) ==
              IntervalCongruence::bottom(8, Signed));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence::top(),
                                                     8,
                                                     Signed,
                                                     WrapTag{}) ==
      IntervalCongruence::top(8, Signed));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZInterval(ZBound(0),
                                                                   ZBound(1))),
                                                     8,
                                                     Signed,
                                                     WrapTag{}) ==
      IntervalCongruence(Interval(Int(0, 8, Signed), Int(1, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence::
          from_z_interval_congruence(ZIntervalCongruence(
                                         ZInterval(ZBound::minus_infinity(),
                                                   ZBound(0))),
                                     8,
                                     Signed,
                                     WrapTag{}) ==
      IntervalCongruence::top(8, Signed));
  BOOST_CHECK(
      IntervalCongruence::
          from_z_interval_congruence(ZIntervalCongruence(
                                         ZInterval(ZBound(0),
                                                   ZBound::plus_infinity())),
                                     8,
                                     Signed,
                                     WrapTag{}) ==
      IntervalCongruence::top(8, Signed));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZInterval(ZBound(0),
                                                                   ZBound(
                                                                       127))),
                                                     8,
                                                     Signed,
                                                     WrapTag{}) ==
      IntervalCongruence(Interval(Int(0, 8, Signed), Int(127, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZInterval(ZBound(0),
                                                                   ZBound(
                                                                       128))),
                                                     8,
                                                     Signed,
                                                     WrapTag{}) ==
      IntervalCongruence::top(8, Signed));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZInterval(ZBound(-128),
                                                                   ZBound(
                                                                       -127))),
                                                     8,
                                                     Signed,
                                                     WrapTag{}) ==
      IntervalCongruence(Interval(Int(-128, 8, Signed), Int(-127, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZInterval(ZBound(-129),
                                                                   ZBound(
                                                                       -127))),
                                                     8,
                                                     Signed,
                                                     WrapTag{}) ==
      IntervalCongruence::top(8, Signed));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZCongruence(128)),
                                                     8,
                                                     Signed,
                                                     WrapTag{}) ==
      IntervalCongruence(Int(-128, 8, Signed)));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZCongruence(Z(128),
                                                                     Z(1))),
                                                     8,
                                                     Signed,
                                                     WrapTag{}) ==
      IntervalCongruence(Congruence(Z(128), Z(1), 8, Signed)));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZCongruence(Z(3),
                                                                     Z(1))),
                                                     8,
                                                     Signed,
                                                     WrapTag{}) ==
      IntervalCongruence::top(8, Signed));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZCongruence(Z(6),
                                                                     Z(1))),
                                                     8,
                                                     Signed,
                                                     WrapTag{}) ==
      IntervalCongruence(Congruence(Z(2), Z(1), 8, Signed)));
  BOOST_CHECK(
      IntervalCongruence::
          from_z_interval_congruence(ZIntervalCongruence(ZInterval(ZBound(1),
                                                                   ZBound(121)),
                                                         ZCongruence(Z(6),
                                                                     Z(1))),
                                     8,
                                     Signed,
                                     WrapTag{}) ==
      IntervalCongruence(Interval(Int(1, 8, Signed), Int(121, 8, Signed)),
                         Congruence(Z(2), Z(1), 8, Signed)));

  BOOST_CHECK(IntervalCongruence::
                  from_z_interval_congruence(ZIntervalCongruence::bottom(),
                                             8,
                                             Unsigned,
                                             WrapTag{}) ==
              IntervalCongruence::bottom(8, Unsigned));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence::top(),
                                                     8,
                                                     Unsigned,
                                                     WrapTag{}) ==
      IntervalCongruence::top(8, Unsigned));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZInterval(ZBound(0),
                                                                   ZBound(1))),
                                                     8,
                                                     Unsigned,
                                                     WrapTag{}) ==
      IntervalCongruence(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned))));
  BOOST_CHECK(
      IntervalCongruence::
          from_z_interval_congruence(ZIntervalCongruence(
                                         ZInterval(ZBound::minus_infinity(),
                                                   ZBound(0))),
                                     8,
                                     Unsigned,
                                     WrapTag{}) ==
      IntervalCongruence::top(8, Unsigned));
  BOOST_CHECK(
      IntervalCongruence::
          from_z_interval_congruence(ZIntervalCongruence(
                                         ZInterval(ZBound(0),
                                                   ZBound::plus_infinity())),
                                     8,
                                     Unsigned,
                                     WrapTag{}) ==
      IntervalCongruence::top(8, Unsigned));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZInterval(ZBound(0),
                                                                   ZBound(
                                                                       254))),
                                                     8,
                                                     Unsigned,
                                                     WrapTag{}) ==
      IntervalCongruence(Interval(Int(0, 8, Unsigned), Int(254, 8, Unsigned))));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZCongruence(257)),
                                                     8,
                                                     Unsigned,
                                                     WrapTag{}) ==
      IntervalCongruence(Int(1, 8, Unsigned)));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZCongruence(Z(128),
                                                                     Z(1))),
                                                     8,
                                                     Unsigned,
                                                     WrapTag{}) ==
      IntervalCongruence(Congruence(Z(128), Z(1), 8, Unsigned)));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZCongruence(Z(3),
                                                                     Z(1))),
                                                     8,
                                                     Unsigned,
                                                     WrapTag{}) ==
      IntervalCongruence::top(8, Unsigned));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZCongruence(Z(6),
                                                                     Z(1))),
                                                     8,
                                                     Unsigned,
                                                     WrapTag{}) ==
      IntervalCongruence(Congruence(Z(2), Z(1), 8, Unsigned)));
  BOOST_CHECK(
      IntervalCongruence::
          from_z_interval_congruence(ZIntervalCongruence(ZInterval(ZBound(1),
                                                                   ZBound(121)),
                                                         ZCongruence(Z(6),
                                                                     Z(1))),
                                     8,
                                     Unsigned,
                                     WrapTag{}) ==
      IntervalCongruence(Interval(Int(1, 8, Unsigned), Int(121, 8, Unsigned)),
                         Congruence(Z(2), Z(1), 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(test_from_z_interval_congruence_trunc) {
  using TruncTag = IntervalCongruence::TruncTag;

  BOOST_CHECK(IntervalCongruence::
                  from_z_interval_congruence(ZIntervalCongruence::bottom(),
                                             8,
                                             Signed,
                                             TruncTag{}) ==
              IntervalCongruence::bottom(8, Signed));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence::top(),
                                                     8,
                                                     Signed,
                                                     TruncTag{}) ==
      IntervalCongruence::top(8, Signed));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZInterval(ZBound(0),
                                                                   ZBound(1))),
                                                     8,
                                                     Signed,
                                                     TruncTag{}) ==
      IntervalCongruence(Interval(Int(0, 8, Signed), Int(1, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence::
          from_z_interval_congruence(ZIntervalCongruence(
                                         ZInterval(ZBound::minus_infinity(),
                                                   ZBound(0))),
                                     8,
                                     Signed,
                                     TruncTag{}) ==
      IntervalCongruence(Interval(Int(-128, 8, Signed), Int(0, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence::
          from_z_interval_congruence(ZIntervalCongruence(
                                         ZInterval(ZBound(0),
                                                   ZBound::plus_infinity())),
                                     8,
                                     Signed,
                                     TruncTag{}) ==
      IntervalCongruence(Interval(Int(0, 8, Signed), Int(127, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZInterval(ZBound(0),
                                                                   ZBound(
                                                                       127))),
                                                     8,
                                                     Signed,
                                                     TruncTag{}) ==
      IntervalCongruence(Interval(Int(0, 8, Signed), Int(127, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZInterval(ZBound(0),
                                                                   ZBound(
                                                                       128))),
                                                     8,
                                                     Signed,
                                                     TruncTag{}) ==
      IntervalCongruence(Interval(Int(0, 8, Signed), Int(127, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZInterval(ZBound(-128),
                                                                   ZBound(
                                                                       -127))),
                                                     8,
                                                     Signed,
                                                     TruncTag{}) ==
      IntervalCongruence(Interval(Int(-128, 8, Signed), Int(-127, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZInterval(ZBound(-129),
                                                                   ZBound(
                                                                       -127))),
                                                     8,
                                                     Signed,
                                                     TruncTag{}) ==
      IntervalCongruence(Interval(Int(-128, 8, Signed), Int(-127, 8, Signed))));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZCongruence(128)),
                                                     8,
                                                     Signed,
                                                     TruncTag{}) ==
      IntervalCongruence::bottom(8, Signed));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZCongruence(Z(128),
                                                                     Z(1))),
                                                     8,
                                                     Signed,
                                                     TruncTag{}) ==
      IntervalCongruence(Congruence(Z(128), Z(1), 8, Signed)));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZCongruence(Z(3),
                                                                     Z(1))),
                                                     8,
                                                     Signed,
                                                     TruncTag{}) ==
      IntervalCongruence(Congruence(Z(3), Z(1), 8, Signed)));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZCongruence(Z(6),
                                                                     Z(1))),
                                                     8,
                                                     Signed,
                                                     TruncTag{}) ==
      IntervalCongruence(Congruence(Z(6), Z(1), 8, Signed)));
  BOOST_CHECK(
      IntervalCongruence::
          from_z_interval_congruence(ZIntervalCongruence(ZInterval(ZBound(1),
                                                                   ZBound(121)),
                                                         ZCongruence(Z(6),
                                                                     Z(1))),
                                     8,
                                     Signed,
                                     TruncTag{}) ==
      IntervalCongruence(Interval(Int(1, 8, Signed), Int(121, 8, Signed)),
                         Congruence(Z(6), Z(1), 8, Signed)));

  BOOST_CHECK(IntervalCongruence::
                  from_z_interval_congruence(ZIntervalCongruence::bottom(),
                                             8,
                                             Unsigned,
                                             TruncTag{}) ==
              IntervalCongruence::bottom(8, Unsigned));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence::top(),
                                                     8,
                                                     Unsigned,
                                                     TruncTag{}) ==
      IntervalCongruence::top(8, Unsigned));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZInterval(ZBound(0),
                                                                   ZBound(1))),
                                                     8,
                                                     Unsigned,
                                                     TruncTag{}) ==
      IntervalCongruence(Interval(Int(0, 8, Unsigned), Int(1, 8, Unsigned))));
  BOOST_CHECK(
      IntervalCongruence::
          from_z_interval_congruence(ZIntervalCongruence(
                                         ZInterval(ZBound::minus_infinity(),
                                                   ZBound(0))),
                                     8,
                                     Unsigned,
                                     TruncTag{}) ==
      IntervalCongruence(Int(0, 8, Unsigned)));
  BOOST_CHECK(
      IntervalCongruence::
          from_z_interval_congruence(ZIntervalCongruence(
                                         ZInterval(ZBound(0),
                                                   ZBound::plus_infinity())),
                                     8,
                                     Unsigned,
                                     TruncTag{}) ==
      IntervalCongruence::top(8, Unsigned));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZInterval(ZBound(0),
                                                                   ZBound(
                                                                       254))),
                                                     8,
                                                     Unsigned,
                                                     TruncTag{}) ==
      IntervalCongruence(Interval(Int(0, 8, Unsigned), Int(254, 8, Unsigned))));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZCongruence(257)),
                                                     8,
                                                     Unsigned,
                                                     TruncTag{}) ==
      IntervalCongruence::bottom(8, Unsigned));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZCongruence(Z(128),
                                                                     Z(1))),
                                                     8,
                                                     Unsigned,
                                                     TruncTag{}) ==
      IntervalCongruence(Congruence(Z(128), Z(1), 8, Unsigned)));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZCongruence(Z(3),
                                                                     Z(1))),
                                                     8,
                                                     Unsigned,
                                                     TruncTag{}) ==
      IntervalCongruence(Congruence(Z(3), Z(1), 8, Unsigned)));
  BOOST_CHECK(
      IntervalCongruence::from_z_interval_congruence(ZIntervalCongruence(
                                                         ZCongruence(Z(6),
                                                                     Z(1))),
                                                     8,
                                                     Unsigned,
                                                     TruncTag{}) ==
      IntervalCongruence(Congruence(Z(6), Z(1), 8, Unsigned)));
  BOOST_CHECK(
      IntervalCongruence::
          from_z_interval_congruence(ZIntervalCongruence(ZInterval(ZBound(1),
                                                                   ZBound(121)),
                                                         ZCongruence(Z(6),
                                                                     Z(1))),
                                     8,
                                     Unsigned,
                                     TruncTag{}) ==
      IntervalCongruence(Interval(Int(1, 8, Unsigned), Int(121, 8, Unsigned)),
                         Congruence(Z(6), Z(1), 8, Unsigned)));
}
