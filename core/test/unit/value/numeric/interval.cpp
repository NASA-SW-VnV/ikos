/*******************************************************************************
 *
 * Tests for Interval
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

#define BOOST_TEST_MODULE test_interval
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/value/numeric/interval.hpp>

using namespace ikos::core;
using namespace ikos::core::numeric;

BOOST_AUTO_TEST_CASE(test_division) {
  BOOST_CHECK(ZInterval(4) / ZInterval::bottom() == ZInterval::bottom());
  BOOST_CHECK(ZInterval::bottom() / ZInterval(2) == ZInterval::bottom());
  BOOST_CHECK(ZInterval::top() / ZInterval(0) == ZInterval::bottom());
  BOOST_CHECK(ZInterval(4) / ZInterval(2) == ZInterval(2));
  BOOST_CHECK(ZInterval(3) / ZInterval(2) == ZInterval(1));
  BOOST_CHECK(ZInterval(-3) / ZInterval(2) == ZInterval(-1));
  BOOST_CHECK(ZInterval(ZBound(1), ZBound(3)) / ZInterval(2) ==
              ZInterval(ZBound(0), ZBound(1)));
  BOOST_CHECK(ZInterval(ZBound(2), ZBound(7)) / ZInterval(2) ==
              ZInterval(ZBound(1), ZBound(3)));
  BOOST_CHECK(ZInterval(ZBound(-3), ZBound(3)) / ZInterval(2) ==
              ZInterval(ZBound(-1), ZBound(1)));
  BOOST_CHECK(ZInterval(ZBound(-3), ZBound::plus_infinity()) / ZInterval(2) ==
              ZInterval(ZBound(-1), ZBound::plus_infinity()));
  BOOST_CHECK(ZInterval(ZBound::minus_infinity(), ZBound(3)) / ZInterval(2) ==
              ZInterval(ZBound::minus_infinity(), ZBound(1)));
  BOOST_CHECK(ZInterval(ZBound(1), ZBound(3)) /
                  ZInterval(ZBound(1), ZBound(2)) ==
              ZInterval(ZBound(0), ZBound(3)));
  BOOST_CHECK(ZInterval(ZBound(-3), ZBound(3)) /
                  ZInterval(ZBound(1), ZBound(2)) ==
              ZInterval(ZBound(-3), ZBound(3)));
  BOOST_CHECK(ZInterval(ZBound(2), ZBound(7)) /
                  ZInterval(ZBound(-2), ZBound(3)) ==
              ZInterval(ZBound(-7), ZBound(7)));
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound(7)) /
                  ZInterval(ZBound(-2), ZBound(3)) ==
              ZInterval(ZBound(-7), ZBound(7)));
  BOOST_CHECK(ZInterval(ZBound::minus_infinity(), ZBound(7)) /
                  ZInterval(ZBound(-2), ZBound(3)) ==
              ZInterval::top());
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound::plus_infinity()) /
                  ZInterval(ZBound(-2), ZBound(3)) ==
              ZInterval::top());
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound(7)) /
                  ZInterval(ZBound::minus_infinity(), ZBound(3)) ==
              ZInterval(ZBound(-7), ZBound(7)));
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound(7)) /
                  ZInterval(ZBound(-2), ZBound::plus_infinity()) ==
              ZInterval(ZBound(-7), ZBound(7)));
  BOOST_CHECK(ZInterval(ZBound(-6), ZBound(-3)) /
                  ZInterval(ZBound(3), ZBound(9)) ==
              ZInterval(ZBound(-2), ZBound(0)));
  BOOST_CHECK(ZInterval(ZBound(-6), ZBound(6)) /
                  ZInterval(ZBound(3), ZBound(9)) ==
              ZInterval(ZBound(-2), ZBound(2)));
}

BOOST_AUTO_TEST_CASE(test_remainder) {
  BOOST_CHECK(ZInterval(4) % ZInterval::bottom() == ZInterval::bottom());
  BOOST_CHECK(ZInterval::bottom() % ZInterval(2) == ZInterval::bottom());
  BOOST_CHECK(ZInterval::top() % ZInterval(0) == ZInterval::bottom());
  BOOST_CHECK(ZInterval(4) % ZInterval(2) == ZInterval(0));
  BOOST_CHECK(ZInterval(3) % ZInterval(2) == ZInterval(1));
  BOOST_CHECK(ZInterval(-3) % ZInterval(2) == ZInterval(-1));
  BOOST_CHECK(ZInterval(ZBound(1), ZBound(3)) % ZInterval(2) ==
              ZInterval(ZBound(0), ZBound(1)));
  BOOST_CHECK(ZInterval(ZBound(2), ZBound(7)) % ZInterval(2) ==
              ZInterval(ZBound(0), ZBound(1)));
  BOOST_CHECK(ZInterval(ZBound(-3), ZBound(3)) % ZInterval(2) ==
              ZInterval(ZBound(-1), ZBound(1)));
  BOOST_CHECK(ZInterval(ZBound(-3), ZBound::plus_infinity()) % ZInterval(2) ==
              ZInterval(ZBound(-1), ZBound(1)));
  BOOST_CHECK(ZInterval(ZBound::minus_infinity(), ZBound(3)) % ZInterval(2) ==
              ZInterval(ZBound(-1), ZBound(1)));
  BOOST_CHECK(ZInterval(ZBound(1), ZBound(3)) %
                  ZInterval(ZBound(1), ZBound(2)) ==
              ZInterval(ZBound(0), ZBound(1)));
  BOOST_CHECK(ZInterval(ZBound(-3), ZBound(3)) %
                  ZInterval(ZBound(1), ZBound(2)) ==
              ZInterval(ZBound(-1), ZBound(1)));
  BOOST_CHECK(ZInterval(ZBound(2), ZBound(7)) %
                  ZInterval(ZBound(-2), ZBound(3)) ==
              ZInterval(ZBound(0), ZBound(2)));
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound(7)) %
                  ZInterval(ZBound(-2), ZBound(3)) ==
              ZInterval(ZBound(-2), ZBound(2)));
  BOOST_CHECK(ZInterval(ZBound::minus_infinity(), ZBound(7)) %
                  ZInterval(ZBound(-2), ZBound(3)) ==
              ZInterval(ZBound(-2), ZBound(2)));
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound::plus_infinity()) %
                  ZInterval(ZBound(-2), ZBound(3)) ==
              ZInterval(ZBound(-2), ZBound(2)));
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound(7)) %
                  ZInterval(ZBound::minus_infinity(), ZBound(3)) ==
              ZInterval(ZBound(-7), ZBound(7)));
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound(7)) %
                  ZInterval(ZBound(-2), ZBound::plus_infinity()) ==
              ZInterval(ZBound(-7), ZBound(7)));
  BOOST_CHECK(ZInterval(ZBound(-6), ZBound(-3)) %
                  ZInterval(ZBound(3), ZBound(9)) ==
              ZInterval(ZBound(-6), ZBound(0)));
  BOOST_CHECK(ZInterval(ZBound(-6), ZBound(6)) %
                  ZInterval(ZBound(3), ZBound(9)) ==
              ZInterval(ZBound(-6), ZBound(6)));
}

BOOST_AUTO_TEST_CASE(test_modulo) {
  BOOST_CHECK(mod(ZInterval(4), ZInterval::bottom()) == ZInterval::bottom());
  BOOST_CHECK(mod(ZInterval::bottom(), ZInterval(2)) == ZInterval::bottom());
  BOOST_CHECK(mod(ZInterval::top(), ZInterval(0)) == ZInterval::bottom());
  BOOST_CHECK(mod(ZInterval(4), ZInterval(2)) == ZInterval(0));
  BOOST_CHECK(mod(ZInterval(3), ZInterval(2)) == ZInterval(1));
  BOOST_CHECK(mod(ZInterval(-3), ZInterval(2)) == ZInterval(1));
  BOOST_CHECK(mod(ZInterval(ZBound(1), ZBound(3)), ZInterval(2)) ==
              ZInterval(ZBound(0), ZBound(1)));
  BOOST_CHECK(mod(ZInterval(ZBound(2), ZBound(7)), ZInterval(2)) ==
              ZInterval(ZBound(0), ZBound(1)));
  BOOST_CHECK(mod(ZInterval(ZBound(-3), ZBound(3)), ZInterval(2)) ==
              ZInterval(ZBound(0), ZBound(1)));
  BOOST_CHECK(mod(ZInterval(ZBound(-3), ZBound::plus_infinity()),
                  ZInterval(2)) == ZInterval(ZBound(0), ZBound(1)));
  BOOST_CHECK(mod(ZInterval(ZBound::minus_infinity(), ZBound(3)),
                  ZInterval(2)) == ZInterval(ZBound(0), ZBound(1)));
  BOOST_CHECK(
      mod(ZInterval(ZBound(1), ZBound(3)), ZInterval(ZBound(1), ZBound(2))) ==
      ZInterval(ZBound(0), ZBound(1)));
  BOOST_CHECK(
      mod(ZInterval(ZBound(-3), ZBound(3)), ZInterval(ZBound(1), ZBound(2))) ==
      ZInterval(ZBound(0), ZBound(1)));
  BOOST_CHECK(
      mod(ZInterval(ZBound(2), ZBound(7)), ZInterval(ZBound(-2), ZBound(3))) ==
      ZInterval(ZBound(0), ZBound(2)));
  BOOST_CHECK(
      mod(ZInterval(ZBound(-2), ZBound(7)), ZInterval(ZBound(-2), ZBound(3))) ==
      ZInterval(ZBound(0), ZBound(2)));
  BOOST_CHECK(mod(ZInterval(ZBound::minus_infinity(), ZBound(7)),
                  ZInterval(ZBound(-2), ZBound(3))) ==
              ZInterval(ZBound(0), ZBound(2)));
  BOOST_CHECK(mod(ZInterval(ZBound(-2), ZBound::plus_infinity()),
                  ZInterval(ZBound(-2), ZBound(3))) ==
              ZInterval(ZBound(0), ZBound(2)));
  BOOST_CHECK(mod(ZInterval(ZBound(-2), ZBound(7)),
                  ZInterval(ZBound::minus_infinity(), ZBound(3))) ==
              ZInterval(ZBound(0), ZBound::plus_infinity()));
  BOOST_CHECK(mod(ZInterval(ZBound(-2), ZBound(7)),
                  ZInterval(ZBound(-2), ZBound::plus_infinity())) ==
              ZInterval(ZBound(0), ZBound::plus_infinity()));
  BOOST_CHECK(
      mod(ZInterval(ZBound(-6), ZBound(-3)), ZInterval(ZBound(3), ZBound(9))) ==
      ZInterval(ZBound(0), ZBound(8)));
  BOOST_CHECK(
      mod(ZInterval(ZBound(-6), ZBound(6)), ZInterval(ZBound(3), ZBound(9))) ==
      ZInterval(ZBound(0), ZBound(8)));
  BOOST_CHECK(mod(ZInterval(ZBound(2), ZBound(7)), ZInterval(20)) ==
              ZInterval(ZBound(2), ZBound(7)));
  BOOST_CHECK(mod(ZInterval(ZBound(2), ZBound(21)), ZInterval(20)) ==
              ZInterval(ZBound(0), ZBound(19)));
  BOOST_CHECK(mod(ZInterval(ZBound(21), ZBound(27)), ZInterval(20)) ==
              ZInterval(ZBound(1), ZBound(7)));
  BOOST_CHECK(mod(ZInterval(ZBound(21), ZBound(27)), ZInterval(-20)) ==
              ZInterval(ZBound(1), ZBound(7)));
  BOOST_CHECK(mod(ZInterval(ZBound(-21), ZBound(27)), ZInterval(-20)) ==
              ZInterval(ZBound(0), ZBound(19)));
  BOOST_CHECK(mod(ZInterval(ZBound(-1), ZBound(7)), ZInterval(20)) ==
              ZInterval(ZBound(0), ZBound(19)));
  BOOST_CHECK(mod(ZInterval(ZBound(-18), ZBound(0)),
                  ZInterval(ZBound(-20), ZBound(-19))) ==
              ZInterval(ZBound(0), ZBound(19)));
}

BOOST_AUTO_TEST_CASE(test_bound_shl) {
  BOOST_CHECK(ZBound::plus_infinity() << ZBound::plus_infinity() ==
              ZBound::plus_infinity());
  BOOST_CHECK(ZBound::plus_infinity() << ZBound(2) == ZBound::plus_infinity());
  BOOST_CHECK(ZBound::minus_infinity() << ZBound::plus_infinity() ==
              ZBound::minus_infinity());
  BOOST_CHECK(ZBound::minus_infinity() << ZBound(2) ==
              ZBound::minus_infinity());
  BOOST_CHECK(ZBound(2) << ZBound::plus_infinity() == ZBound::plus_infinity());
  BOOST_CHECK(ZBound(0) << ZBound::plus_infinity() == ZBound(0));
  BOOST_CHECK(ZBound(-2) << ZBound::plus_infinity() ==
              ZBound::minus_infinity());
  BOOST_CHECK(ZBound(0) << ZBound(2) == ZBound(0));
  BOOST_CHECK(ZBound(2) << ZBound(3) == ZBound(16));
  BOOST_CHECK(ZBound(-2) << ZBound(3) == ZBound(-16));
}

BOOST_AUTO_TEST_CASE(test_interval_shl) {
  BOOST_CHECK(ZInterval(4) << ZInterval::bottom() == ZInterval::bottom());
  BOOST_CHECK(ZInterval::bottom() << ZInterval(2) == ZInterval::bottom());
  BOOST_CHECK(ZInterval::top() << ZInterval(0) == ZInterval::top());
  BOOST_CHECK(ZInterval(4) << ZInterval(2) == ZInterval(16));
  BOOST_CHECK(ZInterval(3) << ZInterval(2) == ZInterval(12));
  BOOST_CHECK(ZInterval(-3) << ZInterval(2) == ZInterval(-12));
  BOOST_CHECK(ZInterval(4) << ZInterval(-2) == ZInterval::bottom());
  BOOST_CHECK(ZInterval(ZBound(1), ZBound(3)) << ZInterval(2) ==
              ZInterval(ZBound(4), ZBound(12)));
  BOOST_CHECK(ZInterval(ZBound(2), ZBound(7)) << ZInterval(2) ==
              ZInterval(ZBound(8), ZBound(28)));
  BOOST_CHECK(ZInterval(ZBound(-3), ZBound(3)) << ZInterval(2) ==
              ZInterval(ZBound(-12), ZBound(12)));
  BOOST_CHECK(ZInterval(ZBound(-3), ZBound::plus_infinity()) << ZInterval(2) ==
              ZInterval(ZBound(-12), ZBound::plus_infinity()));
  BOOST_CHECK(ZInterval(ZBound::minus_infinity(), ZBound(3)) << ZInterval(2) ==
              ZInterval(ZBound::minus_infinity(), ZBound(12)));
  BOOST_CHECK(ZInterval(ZBound(1), ZBound(3))
                  << ZInterval(ZBound(1), ZBound(2)) ==
              ZInterval(ZBound(2), ZBound(12)));
  BOOST_CHECK(ZInterval(ZBound(-3), ZBound(3))
                  << ZInterval(ZBound(1), ZBound(2)) ==
              ZInterval(ZBound(-12), ZBound(12)));
  BOOST_CHECK(ZInterval(ZBound(2), ZBound(7))
                  << ZInterval(ZBound(-2), ZBound(3)) ==
              ZInterval(ZBound(2), ZBound(56)));
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound(7))
                  << ZInterval(ZBound(-2), ZBound(3)) ==
              ZInterval(ZBound(-16), ZBound(56)));
  BOOST_CHECK(ZInterval(ZBound::minus_infinity(), ZBound(7))
                  << ZInterval(ZBound(-2), ZBound(3)) ==
              ZInterval(ZBound::minus_infinity(), ZBound(56)));
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound::plus_infinity())
                  << ZInterval(ZBound(-2), ZBound(3)) ==
              ZInterval(ZBound(-16), ZBound::plus_infinity()));
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound(7))
                  << ZInterval(ZBound::minus_infinity(), ZBound(3)) ==
              ZInterval(ZBound(-16), ZBound(56)));
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound(7))
                  << ZInterval(ZBound(-2), ZBound::plus_infinity()) ==
              ZInterval::top());
  BOOST_CHECK(ZInterval(ZBound(-6), ZBound(-3))
                  << ZInterval(ZBound(3), ZBound(9)) ==
              ZInterval(ZBound(-3072), ZBound(-24)));
  BOOST_CHECK(ZInterval(ZBound(-6), ZBound(6))
                  << ZInterval(ZBound(3), ZBound(9)) ==
              ZInterval(ZBound(-3072), ZBound(3072)));
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound(7))
                  << ZInterval(ZBound::minus_infinity(), ZBound(-1)) ==
              ZInterval::bottom());
  BOOST_CHECK(ZInterval(0) << ZInterval::top() == ZInterval(0));
}

BOOST_AUTO_TEST_CASE(test_bound_shr) {
  BOOST_CHECK(ZBound::plus_infinity() >> ZBound::plus_infinity() ==
              ZBound::plus_infinity());
  BOOST_CHECK(ZBound::plus_infinity() >> ZBound(2) == ZBound::plus_infinity());
  BOOST_CHECK(ZBound::minus_infinity() >> ZBound::plus_infinity() ==
              ZBound::minus_infinity());
  BOOST_CHECK(ZBound::minus_infinity() >> ZBound(2) ==
              ZBound::minus_infinity());
  BOOST_CHECK(ZBound(2) >> ZBound::plus_infinity() == ZBound(0));
  BOOST_CHECK(ZBound(0) >> ZBound::plus_infinity() == ZBound(0));
  BOOST_CHECK(ZBound(-2) >> ZBound::plus_infinity() == ZBound(-1));
  BOOST_CHECK(ZBound(0) >> ZBound(2) == ZBound(0));
  BOOST_CHECK(ZBound(15) >> ZBound(2) == ZBound(3));
  BOOST_CHECK(ZBound(-15) >> ZBound(2) == ZBound(-4));
}

BOOST_AUTO_TEST_CASE(test_interval_shr) {
  BOOST_CHECK(ZInterval(4) >> ZInterval::bottom() == ZInterval::bottom());
  BOOST_CHECK(ZInterval::bottom() >> ZInterval(2) == ZInterval::bottom());
  BOOST_CHECK(ZInterval::top() >> ZInterval(0) == ZInterval::top());
  BOOST_CHECK(ZInterval(15) >> ZInterval(2) == ZInterval(3));
  BOOST_CHECK(ZInterval(1) >> ZInterval(2) == ZInterval(0));
  BOOST_CHECK(ZInterval(-15) >> ZInterval(2) == ZInterval(-4));
  BOOST_CHECK(ZInterval(4) >> ZInterval(-2) == ZInterval::bottom());
  BOOST_CHECK(ZInterval(ZBound(1), ZBound(3)) >> ZInterval(2) == ZInterval(0));
  BOOST_CHECK(ZInterval(ZBound(2), ZBound(7)) >> ZInterval(2) ==
              ZInterval(ZBound(0), ZBound(1)));
  BOOST_CHECK(ZInterval(ZBound(-15), ZBound(15)) >> ZInterval(2) ==
              ZInterval(ZBound(-4), ZBound(3)));
  BOOST_CHECK(ZInterval(ZBound(-15), ZBound::plus_infinity()) >> ZInterval(2) ==
              ZInterval(ZBound(-4), ZBound::plus_infinity()));
  BOOST_CHECK(ZInterval(ZBound::minus_infinity(), ZBound(15)) >> ZInterval(2) ==
              ZInterval(ZBound::minus_infinity(), ZBound(3)));
  BOOST_CHECK(ZInterval(ZBound(0), ZBound(15)) >>
                  ZInterval(ZBound(1), ZBound(2)) ==
              ZInterval(ZBound(0), ZBound(7)));
  BOOST_CHECK(ZInterval(ZBound(-17), ZBound(15)) >>
                  ZInterval(ZBound(1), ZBound(2)) ==
              ZInterval(ZBound(-9), ZBound(7)));
  BOOST_CHECK(ZInterval(ZBound(2), ZBound(7)) >>
                  ZInterval(ZBound(-2), ZBound(3)) ==
              ZInterval(ZBound(0), ZBound(7)));
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound(7)) >>
                  ZInterval(ZBound(-2), ZBound(3)) ==
              ZInterval(ZBound(-2), ZBound(7)));
  BOOST_CHECK(ZInterval(ZBound::minus_infinity(), ZBound(7)) >>
                  ZInterval(ZBound(-2), ZBound(3)) ==
              ZInterval(ZBound::minus_infinity(), ZBound(7)));
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound::plus_infinity()) >>
                  ZInterval(ZBound(-2), ZBound(3)) ==
              ZInterval(ZBound(-2), ZBound::plus_infinity()));
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound(7)) >>
                  ZInterval(ZBound::minus_infinity(), ZBound(3)) ==
              ZInterval(ZBound(-2), ZBound(7)));
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound(7)) >>
                  ZInterval(ZBound(-2), ZBound::plus_infinity()) ==
              ZInterval(ZBound(-2), ZBound(7)));
  BOOST_CHECK(ZInterval(ZBound(-6), ZBound(-3)) >>
                  ZInterval(ZBound(2), ZBound(3)) ==
              ZInterval(ZBound(-2), ZBound(-1)));
  BOOST_CHECK(ZInterval(ZBound(-6), ZBound(6)) >>
                  ZInterval(ZBound(2), ZBound(3)) ==
              ZInterval(ZBound(-2), ZBound(1)));
  BOOST_CHECK(ZInterval(ZBound(-2), ZBound(7)) >>
                  ZInterval(ZBound::minus_infinity(), ZBound(-1)) ==
              ZInterval::bottom());
  BOOST_CHECK(ZInterval(0) >> ZInterval::top() == ZInterval(0));
}

BOOST_AUTO_TEST_CASE(test_and) {
  BOOST_CHECK((ZInterval(4) & ZInterval::bottom()) == ZInterval::bottom());
  BOOST_CHECK((ZInterval::bottom() & ZInterval(2)) == ZInterval::bottom());
  BOOST_CHECK((ZInterval::top() & ZInterval(0)) == ZInterval(0));
  BOOST_CHECK((ZInterval(4) & ZInterval(2)) == ZInterval(0));
  BOOST_CHECK((ZInterval(3) & ZInterval(2)) == ZInterval(2));
  BOOST_CHECK((ZInterval(-3) & ZInterval(2)) == ZInterval(0));
  BOOST_CHECK((ZInterval(ZBound(1), ZBound(3)) & ZInterval(2)) ==
              ZInterval(ZBound(0), ZBound(2)));
  BOOST_CHECK((ZInterval(ZBound(2), ZBound(7)) & ZInterval(2)) ==
              ZInterval(ZBound(0), ZBound(2)));
  BOOST_CHECK((ZInterval(ZBound(-3), ZBound(3)) & ZInterval(2)) ==
              ZInterval(ZBound(0), ZBound(2)));
  BOOST_CHECK((ZInterval(ZBound(-3), ZBound::plus_infinity()) & ZInterval(2)) ==
              ZInterval(ZBound(0), ZBound(2)));
  BOOST_CHECK((ZInterval(ZBound::minus_infinity(), ZBound(3)) & ZInterval(2)) ==
              ZInterval(ZBound(0), ZBound(2)));
  BOOST_CHECK(
      (ZInterval(ZBound(1), ZBound(3)) & ZInterval(ZBound(1), ZBound(2))) ==
      ZInterval(ZBound(0), ZBound(2)));
  BOOST_CHECK(
      (ZInterval(ZBound(-3), ZBound(3)) & ZInterval(ZBound(1), ZBound(2))) ==
      ZInterval(ZBound(0), ZBound(2)));
  BOOST_CHECK(
      (ZInterval(ZBound(2), ZBound(7)) & ZInterval(ZBound(-2), ZBound(3))) ==
      ZInterval(ZBound(0), ZBound(7)));
  BOOST_CHECK((ZInterval(ZBound(-2), ZBound(7)) &
               ZInterval(ZBound(-2), ZBound(3))) == ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound::minus_infinity(), ZBound(7)) &
               ZInterval(ZBound(-2), ZBound(3))) == ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(-2), ZBound::plus_infinity()) &
               ZInterval(ZBound(-2), ZBound(3))) == ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(-2), ZBound(7)) &
               ZInterval(ZBound::minus_infinity(), ZBound(3))) ==
              ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(-2), ZBound(7)) &
               ZInterval(ZBound(-2), ZBound::plus_infinity())) ==
              ZInterval::top());
  BOOST_CHECK(
      (ZInterval(ZBound(-6), ZBound(-3)) & ZInterval(ZBound(3), ZBound(9))) ==
      ZInterval(ZBound(0), ZBound(9)));
  BOOST_CHECK(
      (ZInterval(ZBound(-6), ZBound(6)) & ZInterval(ZBound(3), ZBound(9))) ==
      ZInterval(ZBound(0), ZBound(9)));
}

BOOST_AUTO_TEST_CASE(test_or) {
  BOOST_CHECK((ZInterval(4) | ZInterval::bottom()) == ZInterval::bottom());
  BOOST_CHECK((ZInterval::bottom() | ZInterval(2)) == ZInterval::bottom());
  BOOST_CHECK((ZInterval::top() | ZInterval(-1)) == ZInterval(-1));
  BOOST_CHECK((ZInterval(-1) | ZInterval::top()) == ZInterval(-1));
  BOOST_CHECK((ZInterval(4) | ZInterval(2)) == ZInterval(6));
  BOOST_CHECK((ZInterval(3) | ZInterval(2)) == ZInterval(3));
  BOOST_CHECK((ZInterval(-3) | ZInterval(2)) == ZInterval(-1));
  BOOST_CHECK((ZInterval(ZBound(1), ZBound(3)) | ZInterval(2)) ==
              ZInterval(ZBound(0), ZBound(3)));
  BOOST_CHECK((ZInterval(ZBound(2), ZBound(7)) | ZInterval(2)) ==
              ZInterval(ZBound(0), ZBound(7)));
  BOOST_CHECK((ZInterval(ZBound(-3), ZBound(3)) | ZInterval(2)) ==
              ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(-3), ZBound::plus_infinity()) | ZInterval(2)) ==
              ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound::minus_infinity(), ZBound(3)) | ZInterval(2)) ==
              ZInterval::top());
  BOOST_CHECK(
      (ZInterval(ZBound(1), ZBound(3)) | ZInterval(ZBound(1), ZBound(2))) ==
      ZInterval(ZBound(0), ZBound(3)));
  BOOST_CHECK((ZInterval(ZBound(-3), ZBound(3)) |
               ZInterval(ZBound(1), ZBound(2))) == ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(2), ZBound(7)) |
               ZInterval(ZBound(-2), ZBound(3))) == ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(-2), ZBound(7)) |
               ZInterval(ZBound(-2), ZBound(3))) == ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound::minus_infinity(), ZBound(7)) |
               ZInterval(ZBound(-2), ZBound(3))) == ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(-2), ZBound::plus_infinity()) |
               ZInterval(ZBound(-2), ZBound(3))) == ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(-2), ZBound(7)) |
               ZInterval(ZBound::minus_infinity(), ZBound(3))) ==
              ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(-2), ZBound(7)) |
               ZInterval(ZBound(-2), ZBound::plus_infinity())) ==
              ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(-6), ZBound(-3)) |
               ZInterval(ZBound(3), ZBound(9))) == ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(-6), ZBound(6)) |
               ZInterval(ZBound(3), ZBound(9))) == ZInterval::top());
}

BOOST_AUTO_TEST_CASE(test_xor) {
  BOOST_CHECK((ZInterval(4) ^ ZInterval::bottom()) == ZInterval::bottom());
  BOOST_CHECK((ZInterval::bottom() ^ ZInterval(2)) == ZInterval::bottom());
  BOOST_CHECK((ZInterval::top() ^ ZInterval(-1)) == ZInterval::top());
  BOOST_CHECK((ZInterval(-1) ^ ZInterval::top()) == ZInterval::top());
  BOOST_CHECK((ZInterval(4) ^ ZInterval(2)) == ZInterval(6));
  BOOST_CHECK((ZInterval(3) ^ ZInterval(2)) == ZInterval(1));
  BOOST_CHECK((ZInterval(-3) ^ ZInterval(2)) == ZInterval(-1));
  BOOST_CHECK((ZInterval(ZBound(1), ZBound(3)) ^ ZInterval(2)) ==
              ZInterval(ZBound(0), ZBound(3)));
  BOOST_CHECK((ZInterval(ZBound(2), ZBound(7)) ^ ZInterval(2)) ==
              ZInterval(ZBound(0), ZBound(7)));
  BOOST_CHECK((ZInterval(ZBound(-3), ZBound(3)) ^ ZInterval(2)) ==
              ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(-3), ZBound::plus_infinity()) ^ ZInterval(2)) ==
              ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound::minus_infinity(), ZBound(3)) ^ ZInterval(2)) ==
              ZInterval::top());
  BOOST_CHECK(
      (ZInterval(ZBound(1), ZBound(3)) ^ ZInterval(ZBound(1), ZBound(2))) ==
      ZInterval(ZBound(0), ZBound(3)));
  BOOST_CHECK((ZInterval(ZBound(-3), ZBound(3)) ^
               ZInterval(ZBound(1), ZBound(2))) == ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(2), ZBound(7)) ^
               ZInterval(ZBound(-2), ZBound(3))) == ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(-2), ZBound(7)) ^
               ZInterval(ZBound(-2), ZBound(3))) == ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound::minus_infinity(), ZBound(7)) ^
               ZInterval(ZBound(-2), ZBound(3))) == ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(-2), ZBound::plus_infinity()) ^
               ZInterval(ZBound(-2), ZBound(3))) == ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(-2), ZBound(7)) ^
               ZInterval(ZBound::minus_infinity(), ZBound(3))) ==
              ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(-2), ZBound(7)) ^
               ZInterval(ZBound(-2), ZBound::plus_infinity())) ==
              ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(-6), ZBound(-3)) ^
               ZInterval(ZBound(3), ZBound(9))) == ZInterval::top());
  BOOST_CHECK((ZInterval(ZBound(-6), ZBound(6)) ^
               ZInterval(ZBound(3), ZBound(9))) == ZInterval::top());
}

BOOST_AUTO_TEST_CASE(test_mod_to_sub) {
  BOOST_CHECK(ZInterval(ZBound(1), ZBound(4)).mod_to_sub(ZNumber(8)) ==
              boost::optional< ZNumber >(ZNumber(0)));
  BOOST_CHECK(ZInterval(ZBound(1), ZBound(8)).mod_to_sub(ZNumber(8)) ==
              boost::none);
  BOOST_CHECK(ZInterval(ZBound(1), ZBound(9)).mod_to_sub(ZNumber(8)) ==
              boost::none);
  BOOST_CHECK(ZInterval(ZBound(8), ZBound(9)).mod_to_sub(ZNumber(8)) ==
              boost::optional< ZNumber >(ZNumber(8)));
  BOOST_CHECK(ZInterval(ZBound(18), ZBound(20)).mod_to_sub(ZNumber(8)) ==
              boost::optional< ZNumber >(ZNumber(16)));
  BOOST_CHECK(ZInterval(ZBound(18), ZBound(24)).mod_to_sub(ZNumber(8)) ==
              boost::none);
  BOOST_CHECK(ZInterval(ZBound(-7), ZBound(-1)).mod_to_sub(ZNumber(8)) ==
              boost::optional< ZNumber >(ZNumber(-8)));
  BOOST_CHECK(ZInterval(ZBound(-9), ZBound(-1)).mod_to_sub(ZNumber(8)) ==
              boost::none);
  BOOST_CHECK(ZInterval(ZBound(-10), ZBound(-9)).mod_to_sub(ZNumber(8)) ==
              boost::optional< ZNumber >(ZNumber(-16)));
}

BOOST_AUTO_TEST_CASE(test_widening_threshold) {
  BOOST_CHECK(
      ZInterval(ZBound(0), ZBound(0))
          .widening_threshold(ZInterval(ZBound(-1), ZBound(0)), ZNumber(-47)) ==
      ZInterval(ZBound(-47), ZBound(0)));
  BOOST_CHECK(
      ZInterval(ZBound::minus_infinity(), ZBound(0))
          .widening_threshold(ZInterval(ZBound(0), ZBound(1)), ZNumber(3)) ==
      ZInterval(ZBound::minus_infinity(), ZBound(3)));
  BOOST_CHECK(
      ZInterval(ZBound::minus_infinity(), ZBound(0))
          .widening_threshold(ZInterval(ZBound::minus_infinity(), ZBound(47)),
                              ZNumber(42)) ==
      ZInterval(ZBound::minus_infinity(), ZBound::plus_infinity()));
  BOOST_CHECK(
      ZInterval(ZBound(0), ZBound(0))
          .widening_threshold(ZInterval(ZBound(-1), ZBound(0)), ZNumber(-1)) ==
      ZInterval(ZBound(-1), ZBound(0)));
}

BOOST_AUTO_TEST_CASE(test_narrowing_threshold) {
  BOOST_CHECK(
      ZInterval(ZBound(0), ZBound::plus_infinity())
          .narrowing_threshold(ZInterval(ZBound(0), ZBound(10)), ZNumber(20)) ==
      ZInterval(ZBound(0), ZBound(10)));
  BOOST_CHECK(
      ZInterval(ZBound(0), ZBound(20))
          .narrowing_threshold(ZInterval(ZBound(0), ZBound(10)), ZNumber(20)) ==
      ZInterval(ZBound(0), ZBound(10)));
  BOOST_CHECK(
      ZInterval(ZBound(0), ZBound(30))
          .narrowing_threshold(ZInterval(ZBound(0), ZBound(10)), ZNumber(20)) ==
      ZInterval(ZBound(0), ZBound(30)));
  BOOST_CHECK(ZInterval(ZBound::minus_infinity(), ZBound(0))
                  .narrowing_threshold(ZInterval(ZBound(-10), ZBound(0)),
                                       ZNumber(-20)) ==
              ZInterval(ZBound(-10), ZBound(0)));
  BOOST_CHECK(ZInterval(ZBound(-20), ZBound(0))
                  .narrowing_threshold(ZInterval(ZBound(-10), ZBound(0)),
                                       ZNumber(-20)) ==
              ZInterval(ZBound(-10), ZBound(0)));
  BOOST_CHECK(ZInterval(ZBound(-30), ZBound(0))
                  .narrowing_threshold(ZInterval(ZBound(-10), ZBound(0)),
                                       ZNumber(-20)) ==
              ZInterval(ZBound(-30), ZBound(0)));
  BOOST_CHECK(
      ZInterval(ZBound::minus_infinity(), ZBound::plus_infinity())
          .narrowing_threshold(ZInterval(ZBound(0), ZBound(10)), ZNumber(20)) ==
      ZInterval(ZBound(0), ZBound(10)));
  BOOST_CHECK(
      ZInterval(ZBound(-20), ZBound(20))
          .narrowing_threshold(ZInterval(ZBound(0), ZBound(10)), ZNumber(20)) ==
      ZInterval(ZBound(-20), ZBound(10)));
  BOOST_CHECK(ZInterval(ZBound(-20), ZBound(20))
                  .narrowing_threshold(ZInterval(ZBound(0), ZBound(10)),
                                       ZNumber(-20)) ==
              ZInterval(ZBound(0), ZBound(20)));
}
