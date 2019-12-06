/*******************************************************************************
 *
 * Tests for Constant
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

#define BOOST_TEST_MODULE test_constant
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/value/numeric/constant.hpp>

using namespace ikos::core;
using namespace ikos::core::numeric;

BOOST_AUTO_TEST_CASE(join) {
  BOOST_CHECK(ZConstant(2).join(ZConstant(4)) == ZConstant::top());
  BOOST_CHECK(ZConstant(3).join(ZConstant(3)) == ZConstant(3));
}

BOOST_AUTO_TEST_CASE(meet) {
  BOOST_CHECK(ZConstant(2).meet(ZConstant(4)) == ZConstant::bottom());
  BOOST_CHECK(ZConstant(3).meet(ZConstant(3)) == ZConstant(3));
}

BOOST_AUTO_TEST_CASE(addition) {
  BOOST_CHECK(ZConstant(10) + ZConstant(5) == ZConstant(15));
}

BOOST_AUTO_TEST_CASE(substract) {
  BOOST_CHECK(ZConstant(10) - ZConstant(5) == ZConstant(5));
}

BOOST_AUTO_TEST_CASE(multiply) {
  BOOST_CHECK(ZConstant(10) * ZConstant(5) == ZConstant(50));
  BOOST_CHECK(ZConstant(10) * ZConstant(0) == ZConstant(0));
  BOOST_CHECK(ZConstant(0) * ZConstant(10) == ZConstant(0));
}

BOOST_AUTO_TEST_CASE(divide) {
  BOOST_CHECK(ZConstant(10) / ZConstant(5) == ZConstant(2));
  BOOST_CHECK(ZConstant(10) / ZConstant(0) == ZConstant::bottom());
  BOOST_CHECK(ZConstant(0) / ZConstant::top() == ZConstant(0));
}

BOOST_AUTO_TEST_CASE(test_remainder) {
  BOOST_CHECK(ZConstant::bottom() % ZConstant(5) == ZConstant::bottom());
  BOOST_CHECK(ZConstant(10) % ZConstant::bottom() == ZConstant::bottom());
  BOOST_CHECK(ZConstant(10) % ZConstant(0) == ZConstant::bottom());
  BOOST_CHECK(ZConstant(0) % ZConstant::top() == ZConstant(0));
  BOOST_CHECK(ZConstant(10) % ZConstant::top() == ZConstant::top());
  BOOST_CHECK(ZConstant::top() % ZConstant(5) == ZConstant::top());
  BOOST_CHECK(ZConstant(11) % ZConstant(5) == ZConstant(1));
  BOOST_CHECK(ZConstant(-11) % ZConstant(5) == ZConstant(-1));
}

BOOST_AUTO_TEST_CASE(test_mod) {
  BOOST_CHECK(mod(ZConstant::bottom(), ZConstant(5)) == ZConstant::bottom());
  BOOST_CHECK(mod(ZConstant(10), ZConstant::bottom()) == ZConstant::bottom());
  BOOST_CHECK(mod(ZConstant(10), ZConstant(0)) == ZConstant::bottom());
  BOOST_CHECK(mod(ZConstant(0), ZConstant::top()) == ZConstant(0));
  BOOST_CHECK(mod(ZConstant(10), ZConstant::top()) == ZConstant::top());
  BOOST_CHECK(mod(ZConstant::top(), ZConstant(5)) == ZConstant::top());
  BOOST_CHECK(mod(ZConstant(11), ZConstant(5)) == ZConstant(1));
  BOOST_CHECK(mod(ZConstant(-11), ZConstant(5)) == ZConstant(4));
}

BOOST_AUTO_TEST_CASE(test_shl) {
  BOOST_CHECK(ZConstant::bottom() << ZConstant(5) == ZConstant::bottom());
  BOOST_CHECK(ZConstant(10) << ZConstant::bottom() == ZConstant::bottom());
  BOOST_CHECK(ZConstant(10) << ZConstant(-1) == ZConstant::bottom());
  BOOST_CHECK(ZConstant(0) << ZConstant::top() == ZConstant(0));
  BOOST_CHECK(ZConstant(10) << ZConstant::top() == ZConstant::top());
  BOOST_CHECK(ZConstant::top() << ZConstant(2) == ZConstant::top());
  BOOST_CHECK(ZConstant(11) << ZConstant(2) == ZConstant(44));
  BOOST_CHECK(ZConstant(-11) << ZConstant(2) == ZConstant(-44));
}

BOOST_AUTO_TEST_CASE(test_shr) {
  BOOST_CHECK(ZConstant::bottom() >> ZConstant(5) == ZConstant::bottom());
  BOOST_CHECK(ZConstant(10) >> ZConstant::bottom() == ZConstant::bottom());
  BOOST_CHECK(ZConstant(10) >> ZConstant(-1) == ZConstant::bottom());
  BOOST_CHECK(ZConstant(0) >> ZConstant::top() == ZConstant(0));
  BOOST_CHECK(ZConstant(10) >> ZConstant::top() == ZConstant::top());
  BOOST_CHECK(ZConstant::top() >> ZConstant(2) == ZConstant::top());
  BOOST_CHECK(ZConstant(11) >> ZConstant(2) == ZConstant(2));
  BOOST_CHECK(ZConstant(-11) >> ZConstant(2) == ZConstant(-3));
}

BOOST_AUTO_TEST_CASE(test_and) {
  BOOST_CHECK((ZConstant::bottom() & ZConstant(5)) == ZConstant::bottom());
  BOOST_CHECK((ZConstant(10) & ZConstant::bottom()) == ZConstant::bottom());
  BOOST_CHECK((ZConstant(0) & ZConstant::top()) == ZConstant(0));
  BOOST_CHECK((ZConstant::top() & ZConstant(0)) == ZConstant(0));
  BOOST_CHECK((ZConstant(10) & ZConstant::top()) == ZConstant::top());
  BOOST_CHECK((ZConstant::top() & ZConstant(2)) == ZConstant::top());
  BOOST_CHECK((ZConstant(1) & ZConstant(2)) == ZConstant(0));
  BOOST_CHECK((ZConstant(3) & ZConstant(2)) == ZConstant(2));
  BOOST_CHECK((ZConstant(-1) & ZConstant(2)) == ZConstant(2));
  BOOST_CHECK((ZConstant(-1) & ZConstant(-2)) == ZConstant(-2));
}

BOOST_AUTO_TEST_CASE(test_or) {
  BOOST_CHECK((ZConstant::bottom() | ZConstant(5)) == ZConstant::bottom());
  BOOST_CHECK((ZConstant(10) | ZConstant::bottom()) == ZConstant::bottom());
  BOOST_CHECK((ZConstant(-1) | ZConstant::top()) == ZConstant(-1));
  BOOST_CHECK((ZConstant::top() | ZConstant(-1)) == ZConstant(-1));
  BOOST_CHECK((ZConstant(10) | ZConstant::top()) == ZConstant::top());
  BOOST_CHECK((ZConstant::top() | ZConstant(2)) == ZConstant::top());
  BOOST_CHECK((ZConstant(1) | ZConstant(2)) == ZConstant(3));
  BOOST_CHECK((ZConstant(3) | ZConstant(2)) == ZConstant(3));
  BOOST_CHECK((ZConstant(-1) | ZConstant(2)) == ZConstant(-1));
  BOOST_CHECK((ZConstant(-1) | ZConstant(-2)) == ZConstant(-1));
}

BOOST_AUTO_TEST_CASE(test_xor) {
  BOOST_CHECK((ZConstant::bottom() ^ ZConstant(5)) == ZConstant::bottom());
  BOOST_CHECK((ZConstant(10) ^ ZConstant::bottom()) == ZConstant::bottom());
  BOOST_CHECK((ZConstant(10) ^ ZConstant::top()) == ZConstant::top());
  BOOST_CHECK((ZConstant::top() ^ ZConstant(2)) == ZConstant::top());
  BOOST_CHECK((ZConstant(1) ^ ZConstant(2)) == ZConstant(3));
  BOOST_CHECK((ZConstant(3) ^ ZConstant(2)) == ZConstant(1));
  BOOST_CHECK((ZConstant(-1) ^ ZConstant(2)) == ZConstant(-3));
  BOOST_CHECK((ZConstant(-1) ^ ZConstant(-2)) == ZConstant(1));
}
