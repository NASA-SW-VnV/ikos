/*******************************************************************************
 *
 * Tests for Congruence
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

#define BOOST_TEST_MODULE test_congruence
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/value/numeric/congruence.hpp>

using namespace ikos::core;
using namespace ikos::core::numeric;

BOOST_AUTO_TEST_CASE(test_join) {
  BOOST_CHECK(ZCongruence(2).join(ZCongruence(4)) ==
              ZCongruence(ZNumber(2), ZNumber(0)));
  BOOST_CHECK(ZCongruence(1).join(ZCongruence(3)) ==
              ZCongruence(ZNumber(2), ZNumber(1)));
  BOOST_CHECK(ZCongruence(ZNumber(2), ZNumber(0))
                  .join(ZCongruence(ZNumber(2), ZNumber(1))) ==
              ZCongruence::top());
}

BOOST_AUTO_TEST_CASE(test_meet) {
  BOOST_CHECK(ZCongruence(2).meet(ZCongruence(4)) == ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(ZNumber(2), ZNumber(0)).meet(ZCongruence(1)) ==
              ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(ZNumber(2), ZNumber(1)).meet(ZCongruence(1)) ==
              ZCongruence(1));
  BOOST_CHECK(ZCongruence(ZNumber(2), ZNumber(0))
                  .meet(ZCongruence(ZNumber(2), ZNumber(1))) ==
              ZCongruence::bottom());
}

BOOST_AUTO_TEST_CASE(test_addition) {
  BOOST_CHECK(ZCongruence(ZNumber(2), ZNumber(0)) + ZCongruence(4) ==
              ZCongruence(ZNumber(2), ZNumber(0)));
  BOOST_CHECK(ZCongruence::top() + ZCongruence(4) == ZCongruence::top());
}

BOOST_AUTO_TEST_CASE(test_substract) {
  BOOST_CHECK(ZCongruence(ZNumber(2), ZNumber(0)) - ZCongruence(4) ==
              ZCongruence(ZNumber(2), ZNumber(0)));
  BOOST_CHECK(ZCongruence::top() - ZCongruence(4) == ZCongruence::top());
}

BOOST_AUTO_TEST_CASE(test_multiply) {
  BOOST_CHECK(ZCongruence::bottom() * ZCongruence(4) == ZCongruence::bottom());

  BOOST_CHECK(ZCongruence(2) * ZCongruence::bottom() == ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(2) * ZCongruence(4) == ZCongruence(8));
  BOOST_CHECK(ZCongruence(2) * ZCongruence(-4) == ZCongruence(-8));
  BOOST_CHECK(ZCongruence(7) * ZCongruence(3) == ZCongruence(21));
  BOOST_CHECK(ZCongruence(7) * ZCongruence(0) == ZCongruence(0));
  BOOST_CHECK(ZCongruence(4) * ZCongruence(ZNumber(2), ZNumber(0)) ==
              ZCongruence(ZNumber(8), ZNumber(0)));
  BOOST_CHECK(ZCongruence(4) * ZCongruence(ZNumber(2), ZNumber(1)) ==
              ZCongruence(ZNumber(8), ZNumber(4)));
  BOOST_CHECK(ZCongruence(0) * ZCongruence(ZNumber(2), ZNumber(0)) ==
              ZCongruence(0));
  BOOST_CHECK(ZCongruence(4) * ZCongruence::top() ==
              ZCongruence(ZNumber(4), ZNumber(0)));
  BOOST_CHECK(ZCongruence(0) * ZCongruence::top() == ZCongruence(0));

  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) * ZCongruence::bottom() ==
              ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) * ZCongruence(0) ==
              ZCongruence(0));
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) * ZCongruence(4) ==
              ZCongruence(ZNumber(20), ZNumber(4)));
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) * ZCongruence(-4) ==
              ZCongruence(ZNumber(20), ZNumber(16)));
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) *
                  ZCongruence(ZNumber(2), ZNumber(0)) ==
              ZCongruence(ZNumber(2), ZNumber(0)));
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) *
                  ZCongruence(ZNumber(2), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) *
                  ZCongruence(ZNumber(10), ZNumber(0)) ==
              ZCongruence(ZNumber(10), ZNumber(0)));
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) *
                  ZCongruence(ZNumber(10), ZNumber(2)) ==
              ZCongruence(ZNumber(10), ZNumber(2)));
}

BOOST_AUTO_TEST_CASE(test_divide) {
  BOOST_CHECK(ZCongruence::bottom() / ZCongruence(4) == ZCongruence::bottom());
  BOOST_CHECK(ZCongruence::top() / ZCongruence(4) == ZCongruence::top());

  BOOST_CHECK(ZCongruence(2) / ZCongruence::bottom() == ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(3) / ZCongruence(2) == ZCongruence(1));
  BOOST_CHECK(ZCongruence(-3) / ZCongruence(2) == ZCongruence(-1));
  BOOST_CHECK(ZCongruence(7) / ZCongruence(3) == ZCongruence(2));
  BOOST_CHECK(ZCongruence(7) / ZCongruence(0) == ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(4) / ZCongruence(ZNumber(2), ZNumber(0)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(4) / ZCongruence(ZNumber(2), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(0) / ZCongruence(ZNumber(2), ZNumber(0)) ==
              ZCongruence(0));
  BOOST_CHECK(ZCongruence(-4) / ZCongruence(ZNumber(4), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(4) / ZCongruence(ZNumber(4), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(2) / ZCongruence(ZNumber(3), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(7) / ZCongruence(ZNumber(4), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(12) / ZCongruence(ZNumber(3), ZNumber(2)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(6) / ZCongruence(ZNumber(6), ZNumber(3)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(5) / ZCongruence(ZNumber(5), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(4) / ZCongruence::top() == ZCongruence::top());
  BOOST_CHECK(ZCongruence(0) / ZCongruence::top() == ZCongruence(0));

  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) / ZCongruence::bottom() ==
              ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) / ZCongruence(0) ==
              ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) / ZCongruence(4) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(4), ZNumber(1)) / ZCongruence(2) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) / ZCongruence(-4) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(8), ZNumber(1)) / ZCongruence(4) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(8), ZNumber(4)) / ZCongruence(4) ==
              ZCongruence(ZNumber(2), ZNumber(1)));
  BOOST_CHECK(ZCongruence(ZNumber(12), ZNumber(7)) / ZCongruence(3) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) /
                  ZCongruence(ZNumber(2), ZNumber(0)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) /
                  ZCongruence(ZNumber(2), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) /
                  ZCongruence(ZNumber(10), ZNumber(0)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) /
                  ZCongruence(ZNumber(10), ZNumber(2)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(2), ZNumber(0)) / ZCongruence(4) ==
              ZCongruence::top());
}

BOOST_AUTO_TEST_CASE(test_mod) {
  BOOST_CHECK(mod(ZCongruence::bottom(), ZCongruence(4)) ==
              ZCongruence::bottom());
  BOOST_CHECK(mod(ZCongruence::top(), ZCongruence(4)) == ZCongruence::top());

  BOOST_CHECK(mod(ZCongruence(2), ZCongruence::bottom()) ==
              ZCongruence::bottom());
  BOOST_CHECK(mod(ZCongruence(3), ZCongruence(2)) == ZCongruence(1));
  BOOST_CHECK(mod(ZCongruence(-3), ZCongruence(2)) == ZCongruence(1));
  BOOST_CHECK(mod(ZCongruence(7), ZCongruence(3)) == ZCongruence(1));
  BOOST_CHECK(mod(ZCongruence(7), ZCongruence(0)) == ZCongruence::bottom());
  BOOST_CHECK(mod(ZCongruence(4), ZCongruence(ZNumber(2), ZNumber(0))) ==
              ZCongruence(ZNumber(2), ZNumber(0)));
  BOOST_CHECK(mod(ZCongruence(4), ZCongruence(ZNumber(2), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK(mod(ZCongruence(0), ZCongruence(ZNumber(2), ZNumber(0))) ==
              ZCongruence(0));
  BOOST_CHECK(mod(ZCongruence(-4), ZCongruence(ZNumber(4), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK(mod(ZCongruence(4), ZCongruence(ZNumber(4), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK(mod(ZCongruence(2), ZCongruence(ZNumber(3), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK(mod(ZCongruence(7), ZCongruence(ZNumber(4), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK(mod(ZCongruence(12), ZCongruence(ZNumber(3), ZNumber(2))) ==
              ZCongruence::top());
  BOOST_CHECK(mod(ZCongruence(6), ZCongruence(ZNumber(6), ZNumber(3))) ==
              ZCongruence(ZNumber(3), ZNumber(0)));
  BOOST_CHECK(mod(ZCongruence(5), ZCongruence(ZNumber(5), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK(mod(ZCongruence(4), ZCongruence::top()) == ZCongruence::top());
  BOOST_CHECK(mod(ZCongruence(0), ZCongruence::top()) == ZCongruence(0));

  BOOST_CHECK(mod(ZCongruence(ZNumber(5), ZNumber(1)), ZCongruence::bottom()) ==
              ZCongruence::bottom());
  BOOST_CHECK(mod(ZCongruence(ZNumber(5), ZNumber(1)), ZCongruence(0)) ==
              ZCongruence::bottom());
  BOOST_CHECK(mod(ZCongruence(ZNumber(5), ZNumber(1)), ZCongruence(4)) ==
              ZCongruence::top());
  BOOST_CHECK(mod(ZCongruence(ZNumber(4), ZNumber(1)), ZCongruence(2)) ==
              ZCongruence(1));
  BOOST_CHECK(mod(ZCongruence(ZNumber(5), ZNumber(1)), ZCongruence(-4)) ==
              ZCongruence::top());
  BOOST_CHECK(mod(ZCongruence(ZNumber(8), ZNumber(1)), ZCongruence(4)) ==
              ZCongruence(1));
  BOOST_CHECK(mod(ZCongruence(ZNumber(8), ZNumber(4)), ZCongruence(4)) ==
              ZCongruence(0));
  BOOST_CHECK(mod(ZCongruence(ZNumber(12), ZNumber(7)), ZCongruence(3)) ==
              ZCongruence(1));
  BOOST_CHECK(mod(ZCongruence(ZNumber(5), ZNumber(1)),
                  ZCongruence(ZNumber(2), ZNumber(0))) == ZCongruence::top());
  BOOST_CHECK(mod(ZCongruence(ZNumber(5), ZNumber(1)),
                  ZCongruence(ZNumber(2), ZNumber(1))) == ZCongruence::top());
  BOOST_CHECK(mod(ZCongruence(ZNumber(5), ZNumber(1)),
                  ZCongruence(ZNumber(10), ZNumber(0))) ==
              ZCongruence(ZNumber(5), ZNumber(1)));
  BOOST_CHECK(mod(ZCongruence(ZNumber(5), ZNumber(1)),
                  ZCongruence(ZNumber(10), ZNumber(2))) == ZCongruence::top());
  BOOST_CHECK(mod(ZCongruence(ZNumber(2), ZNumber(0)), ZCongruence(4)) ==
              ZCongruence(ZNumber(2), ZNumber(0)));
}

BOOST_AUTO_TEST_CASE(test_rem) {
  BOOST_CHECK(ZCongruence::bottom() % ZCongruence(4) == ZCongruence::bottom());
  BOOST_CHECK(ZCongruence::top() % ZCongruence(4) == ZCongruence::top());

  BOOST_CHECK(ZCongruence(2) % ZCongruence::bottom() == ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(3) % ZCongruence(2) == ZCongruence(1));
  BOOST_CHECK(ZCongruence(-3) % ZCongruence(2) == ZCongruence(-1));
  BOOST_CHECK(ZCongruence(7) % ZCongruence(3) == ZCongruence(1));
  BOOST_CHECK(ZCongruence(7) % ZCongruence(0) == ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(4) % ZCongruence(ZNumber(2), ZNumber(0)) ==
              ZCongruence(ZNumber(2), ZNumber(0)));
  BOOST_CHECK(ZCongruence(4) % ZCongruence(ZNumber(2), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(0) % ZCongruence(ZNumber(2), ZNumber(0)) ==
              ZCongruence(0));
  BOOST_CHECK(ZCongruence(-4) % ZCongruence(ZNumber(4), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(4) % ZCongruence(ZNumber(4), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(2) % ZCongruence(ZNumber(3), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(7) % ZCongruence(ZNumber(4), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(12) % ZCongruence(ZNumber(3), ZNumber(2)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(6) % ZCongruence(ZNumber(6), ZNumber(3)) ==
              ZCongruence(ZNumber(3), ZNumber(0)));
  BOOST_CHECK(ZCongruence(5) % ZCongruence(ZNumber(5), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(4) % ZCongruence::top() == ZCongruence::top());
  BOOST_CHECK(ZCongruence(0) % ZCongruence::top() == ZCongruence(0));

  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) % ZCongruence::bottom() ==
              ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) % ZCongruence(0) ==
              ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) % ZCongruence(4) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(4), ZNumber(1)) % ZCongruence(2) ==
              ZCongruence(ZNumber(2), ZNumber(1)));
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) % ZCongruence(-4) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(8), ZNumber(1)) % ZCongruence(4) ==
              ZCongruence(ZNumber(4), ZNumber(1)));
  BOOST_CHECK(ZCongruence(ZNumber(8), ZNumber(4)) % ZCongruence(4) ==
              ZCongruence(0));
  BOOST_CHECK(ZCongruence(ZNumber(12), ZNumber(7)) % ZCongruence(3) ==
              ZCongruence(ZNumber(3), ZNumber(1)));
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) %
                  ZCongruence(ZNumber(2), ZNumber(0)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) %
                  ZCongruence(ZNumber(2), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) %
                  ZCongruence(ZNumber(10), ZNumber(0)) ==
              ZCongruence(ZNumber(5), ZNumber(1)));
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) %
                  ZCongruence(ZNumber(10), ZNumber(2)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(2), ZNumber(0)) % ZCongruence(4) ==
              ZCongruence(ZNumber(2), ZNumber(0)));
}

BOOST_AUTO_TEST_CASE(test_shl) {
  BOOST_CHECK(ZCongruence::bottom() << ZCongruence(4) == ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(4) << ZCongruence::bottom() == ZCongruence::bottom());

  BOOST_CHECK(ZCongruence(3) << ZCongruence(2) == ZCongruence(12));
  BOOST_CHECK(ZCongruence(-3) << ZCongruence(2) == ZCongruence(-12));
  BOOST_CHECK(ZCongruence(7) << ZCongruence(3) == ZCongruence(56));
  BOOST_CHECK(ZCongruence(7) << ZCongruence(-1) == ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(4) << ZCongruence(ZNumber(2), ZNumber(0)) ==
              ZCongruence(ZNumber(12), ZNumber(4)));
  BOOST_CHECK(ZCongruence(4) << ZCongruence(ZNumber(2), ZNumber(1)) ==
              ZCongruence(ZNumber(24), ZNumber(8)));
  BOOST_CHECK(ZCongruence(0) << ZCongruence(ZNumber(2), ZNumber(0)) ==
              ZCongruence(0));
  BOOST_CHECK(ZCongruence(-4) << ZCongruence(ZNumber(4), ZNumber(1)) ==
              ZCongruence(ZNumber(120), ZNumber(112)));
  BOOST_CHECK(ZCongruence(4) << ZCongruence(ZNumber(4), ZNumber(1)) ==
              ZCongruence(ZNumber(120), ZNumber(8)));
  BOOST_CHECK(ZCongruence(2) << ZCongruence(ZNumber(3), ZNumber(1)) ==
              ZCongruence(ZNumber(28), ZNumber(4)));
  BOOST_CHECK(ZCongruence(7) << ZCongruence(ZNumber(4), ZNumber(1)) ==
              ZCongruence(ZNumber(210), ZNumber(14)));
  BOOST_CHECK(ZCongruence(12) << ZCongruence(ZNumber(3), ZNumber(2)) ==
              ZCongruence(ZNumber(336), ZNumber(48)));
  BOOST_CHECK(ZCongruence(6) << ZCongruence(ZNumber(6), ZNumber(3)) ==
              ZCongruence(ZNumber(3024), ZNumber(48)));
  BOOST_CHECK(ZCongruence(5) << ZCongruence(ZNumber(5), ZNumber(1)) ==
              ZCongruence(ZNumber(310), ZNumber(10)));
  BOOST_CHECK(ZCongruence(4) << ZCongruence::top() ==
              ZCongruence(ZNumber(4), ZNumber(0)));
  BOOST_CHECK(ZCongruence(0) << ZCongruence::top() == ZCongruence(0));
  BOOST_CHECK(ZCongruence::top() << ZCongruence(4) ==
              ZCongruence(ZNumber(16), ZNumber(0)));

  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) << ZCongruence::bottom() ==
              ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) << ZCongruence(0) ==
              ZCongruence(ZNumber(5), ZNumber(1)));
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) << ZCongruence(4) ==
              ZCongruence(ZNumber(80), ZNumber(16)));
  BOOST_CHECK(ZCongruence(ZNumber(4), ZNumber(1)) << ZCongruence(2) ==
              ZCongruence(ZNumber(16), ZNumber(4)));
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) << ZCongruence(-4) ==
              ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(ZNumber(8), ZNumber(1)) << ZCongruence(4) ==
              ZCongruence(ZNumber(128), ZNumber(16)));
  BOOST_CHECK(ZCongruence(ZNumber(8), ZNumber(4)) << ZCongruence(4) ==
              ZCongruence(ZNumber(128), ZNumber(64)));
  BOOST_CHECK(ZCongruence(ZNumber(12), ZNumber(7)) << ZCongruence(3) ==
              ZCongruence(ZNumber(96), ZNumber(56)));
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1))
                  << ZCongruence(ZNumber(2), ZNumber(0)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1))
                  << ZCongruence(ZNumber(2), ZNumber(1)) ==
              ZCongruence(ZNumber(2), ZNumber(0)));
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1))
                  << ZCongruence(ZNumber(10), ZNumber(0)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1))
                  << ZCongruence(ZNumber(10), ZNumber(2)) ==
              ZCongruence(ZNumber(4), ZNumber(0)));
  BOOST_CHECK(ZCongruence(ZNumber(2), ZNumber(0)) << ZCongruence(4) ==
              ZCongruence(ZNumber(32), ZNumber(0)));
}

BOOST_AUTO_TEST_CASE(test_shr) {
  BOOST_CHECK(ZCongruence::bottom() >> ZCongruence(4) == ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(4) >> ZCongruence::bottom() == ZCongruence::bottom());

  BOOST_CHECK(ZCongruence(12) >> ZCongruence(2) == ZCongruence(3));
  BOOST_CHECK(ZCongruence(-12) >> ZCongruence(2) == ZCongruence(-3));
  BOOST_CHECK(ZCongruence(17) >> ZCongruence(3) == ZCongruence(2));
  BOOST_CHECK(ZCongruence(7) >> ZCongruence(-1) == ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(4) >> ZCongruence(ZNumber(2), ZNumber(0)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(4) >> ZCongruence(ZNumber(2), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(0) >> ZCongruence(ZNumber(2), ZNumber(0)) ==
              ZCongruence(0));
  BOOST_CHECK(ZCongruence(-4) >> ZCongruence(ZNumber(4), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(4) >> ZCongruence(ZNumber(4), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(2) >> ZCongruence(ZNumber(3), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(7) >> ZCongruence(ZNumber(4), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(12) >> ZCongruence(ZNumber(3), ZNumber(2)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(6) >> ZCongruence(ZNumber(6), ZNumber(3)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(5) >> ZCongruence(ZNumber(5), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(4) >> ZCongruence::top() == ZCongruence::top());
  BOOST_CHECK(ZCongruence(0) >> ZCongruence::top() == ZCongruence(0));
  BOOST_CHECK(ZCongruence::top() >> ZCongruence(4) == ZCongruence::top());

  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) >> ZCongruence::bottom() ==
              ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) >> ZCongruence(0) ==
              ZCongruence(ZNumber(5), ZNumber(1)));
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) >> ZCongruence(4) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(4), ZNumber(1)) >> ZCongruence(2) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) >> ZCongruence(-4) ==
              ZCongruence::bottom());
  BOOST_CHECK(ZCongruence(ZNumber(8), ZNumber(1)) >> ZCongruence(4) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(8), ZNumber(4)) >> ZCongruence(4) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(12), ZNumber(7)) >> ZCongruence(3) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) >>
                  ZCongruence(ZNumber(2), ZNumber(0)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) >>
                  ZCongruence(ZNumber(2), ZNumber(1)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) >>
                  ZCongruence(ZNumber(10), ZNumber(0)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(5), ZNumber(1)) >>
                  ZCongruence(ZNumber(10), ZNumber(2)) ==
              ZCongruence::top());
  BOOST_CHECK(ZCongruence(ZNumber(2), ZNumber(0)) >> ZCongruence(4) ==
              ZCongruence::top());
}

BOOST_AUTO_TEST_CASE(test_and) {
  BOOST_CHECK((ZCongruence::bottom() & ZCongruence(4)) ==
              ZCongruence::bottom());
  BOOST_CHECK((ZCongruence(4) & ZCongruence::bottom()) ==
              ZCongruence::bottom());

  BOOST_CHECK((ZCongruence(12) & ZCongruence(6)) == ZCongruence(4));
  BOOST_CHECK((ZCongruence(-12) & ZCongruence(6)) == ZCongruence(4));
  BOOST_CHECK((ZCongruence(17) & ZCongruence(3)) == ZCongruence(1));
  BOOST_CHECK((ZCongruence(7) & ZCongruence(-1)) == ZCongruence(7));
  BOOST_CHECK((ZCongruence(4) & ZCongruence(ZNumber(2), ZNumber(0))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(4) & ZCongruence(ZNumber(2), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(0) & ZCongruence(ZNumber(2), ZNumber(0))) ==
              ZCongruence(0));
  BOOST_CHECK((ZCongruence(-1) & ZCongruence(ZNumber(4), ZNumber(1))) ==
              ZCongruence(ZNumber(4), ZNumber(1)));
  BOOST_CHECK((ZCongruence(4) & ZCongruence(ZNumber(4), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(2) & ZCongruence(ZNumber(3), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(7) & ZCongruence(ZNumber(4), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(12) & ZCongruence(ZNumber(3), ZNumber(2))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(6) & ZCongruence(ZNumber(6), ZNumber(3))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(5) & ZCongruence(ZNumber(5), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(4) & ZCongruence::top()) == ZCongruence::top());
  BOOST_CHECK((ZCongruence(0) & ZCongruence::top()) == ZCongruence(0));
  BOOST_CHECK((ZCongruence::top() & ZCongruence(4)) == ZCongruence::top());

  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) & ZCongruence::bottom()) ==
              ZCongruence::bottom());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) & ZCongruence(0)) ==
              ZCongruence(0));
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) & ZCongruence(4)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(4), ZNumber(1)) & ZCongruence(2)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) & ZCongruence(-4)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(8), ZNumber(1)) & ZCongruence(4)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(8), ZNumber(4)) & ZCongruence(4)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(12), ZNumber(7)) & ZCongruence(3)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) &
               ZCongruence(ZNumber(2), ZNumber(0))) == ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) &
               ZCongruence(ZNumber(2), ZNumber(1))) == ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) &
               ZCongruence(ZNumber(10), ZNumber(0))) == ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) &
               ZCongruence(ZNumber(10), ZNumber(2))) == ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(2), ZNumber(0)) & ZCongruence(4)) ==
              ZCongruence::top());
}

BOOST_AUTO_TEST_CASE(test_or) {
  BOOST_CHECK((ZCongruence::bottom() | ZCongruence(4)) ==
              ZCongruence::bottom());
  BOOST_CHECK((ZCongruence(4) | ZCongruence::bottom()) ==
              ZCongruence::bottom());

  BOOST_CHECK((ZCongruence(12) | ZCongruence(6)) == ZCongruence(14));
  BOOST_CHECK((ZCongruence(-12) | ZCongruence(6)) == ZCongruence(-10));
  BOOST_CHECK((ZCongruence(17) | ZCongruence(3)) == ZCongruence(19));
  BOOST_CHECK((ZCongruence(7) | ZCongruence(-1)) == ZCongruence(-1));
  BOOST_CHECK((ZCongruence(4) | ZCongruence(ZNumber(2), ZNumber(0))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(4) | ZCongruence(ZNumber(2), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(0) | ZCongruence(ZNumber(2), ZNumber(0))) ==
              ZCongruence(ZNumber(2), ZNumber(0)));
  BOOST_CHECK((ZCongruence(-1) | ZCongruence(ZNumber(4), ZNumber(1))) ==
              ZCongruence(-1));
  BOOST_CHECK((ZCongruence(4) | ZCongruence(ZNumber(4), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(2) | ZCongruence(ZNumber(3), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(7) | ZCongruence(ZNumber(4), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(12) | ZCongruence(ZNumber(3), ZNumber(2))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(6) | ZCongruence(ZNumber(6), ZNumber(3))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(5) | ZCongruence(ZNumber(5), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(4) | ZCongruence::top()) == ZCongruence::top());
  BOOST_CHECK((ZCongruence(0) | ZCongruence::top()) == ZCongruence::top());
  BOOST_CHECK((ZCongruence::top() | ZCongruence(4)) == ZCongruence::top());

  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) | ZCongruence::bottom()) ==
              ZCongruence::bottom());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) | ZCongruence(0)) ==
              ZCongruence(ZNumber(5), ZNumber(1)));
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) | ZCongruence(4)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(4), ZNumber(1)) | ZCongruence(2)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) | ZCongruence(-4)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(8), ZNumber(1)) | ZCongruence(4)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(8), ZNumber(4)) | ZCongruence(4)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(12), ZNumber(7)) | ZCongruence(3)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) |
               ZCongruence(ZNumber(2), ZNumber(0))) == ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) |
               ZCongruence(ZNumber(2), ZNumber(1))) == ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) |
               ZCongruence(ZNumber(10), ZNumber(0))) == ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) |
               ZCongruence(ZNumber(10), ZNumber(2))) == ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(2), ZNumber(0)) | ZCongruence(4)) ==
              ZCongruence::top());
}

BOOST_AUTO_TEST_CASE(test_xor) {
  BOOST_CHECK((ZCongruence::bottom() ^ ZCongruence(4)) ==
              ZCongruence::bottom());
  BOOST_CHECK((ZCongruence(4) ^ ZCongruence::bottom()) ==
              ZCongruence::bottom());

  BOOST_CHECK((ZCongruence(12) ^ ZCongruence(6)) == ZCongruence(10));
  BOOST_CHECK((ZCongruence(-12) ^ ZCongruence(6)) == ZCongruence(-14));
  BOOST_CHECK((ZCongruence(17) ^ ZCongruence(3)) == ZCongruence(18));
  BOOST_CHECK((ZCongruence(7) ^ ZCongruence(-1)) == ZCongruence(-8));
  BOOST_CHECK((ZCongruence(4) ^ ZCongruence(ZNumber(2), ZNumber(0))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(4) ^ ZCongruence(ZNumber(2), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(0) ^ ZCongruence(ZNumber(2), ZNumber(0))) ==
              ZCongruence(ZNumber(2), ZNumber(0)));
  BOOST_CHECK((ZCongruence(-1) ^ ZCongruence(ZNumber(4), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(4) ^ ZCongruence(ZNumber(4), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(2) ^ ZCongruence(ZNumber(3), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(7) ^ ZCongruence(ZNumber(4), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(12) ^ ZCongruence(ZNumber(3), ZNumber(2))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(6) ^ ZCongruence(ZNumber(6), ZNumber(3))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(5) ^ ZCongruence(ZNumber(5), ZNumber(1))) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(4) ^ ZCongruence::top()) == ZCongruence::top());
  BOOST_CHECK((ZCongruence(0) ^ ZCongruence::top()) == ZCongruence::top());
  BOOST_CHECK((ZCongruence::top() ^ ZCongruence(4)) == ZCongruence::top());

  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) ^ ZCongruence::bottom()) ==
              ZCongruence::bottom());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) ^ ZCongruence(0)) ==
              ZCongruence(ZNumber(5), ZNumber(1)));
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) ^ ZCongruence(4)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(4), ZNumber(1)) ^ ZCongruence(2)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) ^ ZCongruence(-4)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(8), ZNumber(1)) ^ ZCongruence(4)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(8), ZNumber(4)) ^ ZCongruence(4)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(12), ZNumber(7)) ^ ZCongruence(3)) ==
              ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) ^
               ZCongruence(ZNumber(2), ZNumber(0))) == ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) ^
               ZCongruence(ZNumber(2), ZNumber(1))) == ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) ^
               ZCongruence(ZNumber(10), ZNumber(0))) == ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(5), ZNumber(1)) ^
               ZCongruence(ZNumber(10), ZNumber(2))) == ZCongruence::top());
  BOOST_CHECK((ZCongruence(ZNumber(2), ZNumber(0)) ^ ZCongruence(4)) ==
              ZCongruence::top());
}
