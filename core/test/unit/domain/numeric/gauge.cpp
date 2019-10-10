/*******************************************************************************
 *
 * Tests for GaugeDomain
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

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <ikos/core/domain/numeric/gauge.hpp>
#include <ikos/core/example/variable_factory.hpp>
#include <ikos/core/number/z_number.hpp>
#include <ikos/core/value/numeric/gauge.hpp>

using ZNumber = ikos::core::ZNumber;
using ZBound = ikos::core::ZBound;
using VariableFactory = ikos::core::example::VariableFactory;
using Variable = ikos::core::example::VariableFactory::VariableRef;
using VariableExpression = ikos::core::VariableExpression< ZNumber, Variable >;
using BinaryOperator = ikos::core::numeric::BinaryOperator;
using ZConstant = ikos::core::numeric::ZConstant;
using ZInterval = ikos::core::numeric::ZInterval;
using GaugeBound = ikos::core::numeric::GaugeBound< ZNumber, Variable >;
using Gauge = ikos::core::numeric::Gauge< ZNumber, Variable >;
using GaugeSemiLattice =
    ikos::core::numeric::GaugeSemiLattice< ZNumber, Variable >;
using GaugeDomain = ikos::core::numeric::GaugeDomain< ZNumber, Variable >;

// NOLINTNEXTLINE(readability-identifier-naming)
static VariableFactory vfac;

// NOLINTNEXTLINE(readability-identifier-naming)
static GaugeSemiLattice l1 = GaugeSemiLattice::top();
// NOLINTNEXTLINE(readability-identifier-naming)
static GaugeSemiLattice l2 = GaugeSemiLattice::top();
// NOLINTNEXTLINE(readability-identifier-naming)
static GaugeSemiLattice l3 = GaugeSemiLattice::top();
// NOLINTNEXTLINE(readability-identifier-naming)
static GaugeSemiLattice l4 = GaugeSemiLattice::top();
// NOLINTNEXTLINE(readability-identifier-naming)
static GaugeSemiLattice l5 = GaugeSemiLattice::top();
// NOLINTNEXTLINE(readability-identifier-naming)
static GaugeSemiLattice l6 = GaugeSemiLattice::top();
// NOLINTNEXTLINE(readability-identifier-naming)
static GaugeSemiLattice l7 = GaugeSemiLattice::top();
// NOLINTNEXTLINE(readability-identifier-naming)
static GaugeSemiLattice l8 = GaugeSemiLattice::top();
// NOLINTNEXTLINE(readability-identifier-naming)
static GaugeSemiLattice l9 = GaugeSemiLattice::top();
// NOLINTNEXTLINE(readability-identifier-naming)
static GaugeSemiLattice l10 = GaugeSemiLattice::top();
// NOLINTNEXTLINE(readability-identifier-naming)
static GaugeSemiLattice l11 = GaugeSemiLattice::top();
// NOLINTNEXTLINE(readability-identifier-naming)
static GaugeSemiLattice l12 = GaugeSemiLattice::top();
// NOLINTNEXTLINE(readability-identifier-naming)
static GaugeSemiLattice l13 = GaugeSemiLattice::top();

// initialization
static bool init_unit_test() {
  boost::unit_test::framework::master_test_suite().p_name.value =
      "test_gauge_domain";

  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable i(vfac.get("i"));
  Variable k(vfac.get("k"));

  l1.set(x, Gauge(GaugeBound(0), GaugeBound(1)));

  l2.set(x, Gauge(GaugeBound(0), GaugeBound(1)));
  l2.set(y, Gauge(GaugeBound(i), GaugeBound(2, i)));

  l3.set(x, Gauge(GaugeBound(0), GaugeBound::plus_infinity()));

  l4.set(x, Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  l4.set(y, Gauge(GaugeBound(-2) + GaugeBound(i), GaugeBound(3, i)));

  l5.set(x, Gauge(GaugeBound(0), GaugeBound(1)));
  l5.set(y, Gauge(GaugeBound(2, k), GaugeBound(2, k)));

  l6.set(x, Gauge(GaugeBound(0), GaugeBound(1)));
  l6.set(y, Gauge(GaugeBound(0), GaugeBound(2, i) + GaugeBound(2, k)));

  l7.set(x, Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  l7.set(y, Gauge(GaugeBound(-2), GaugeBound(3, i) + GaugeBound(2, k)));

  l8.set(x, Gauge(GaugeBound(0), GaugeBound(1)));
  l8.set(y, Gauge(GaugeBound(-2) + GaugeBound(i), GaugeBound(3, i)));

  l9.set(x, Gauge(GaugeBound(0), GaugeBound(1)));
  l9.set(y, Gauge(GaugeBound(2, k), GaugeBound(2, i) + GaugeBound(2, k)));

  l10.set(x, Gauge(GaugeBound(0), GaugeBound(1)));
  l10.set(y, Gauge(GaugeBound(0), GaugeBound::plus_infinity()));

  l11.set(x, Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  l11.set(y, Gauge(GaugeBound(-2), GaugeBound::plus_infinity()));

  l12.set(x, Gauge(GaugeBound(0), GaugeBound(1)));
  l12.set(y, Gauge(GaugeBound(1), GaugeBound(6)));

  l13.set(x, Gauge(GaugeBound(0), GaugeBound(1)));
  l13.set(y, Gauge(GaugeBound(0), GaugeBound(6) + GaugeBound(2, k)));

  return true;
}

int main(int argc, char* argv[]) {
  return boost::unit_test::unit_test_main(&init_unit_test, argc, argv);
}

#define test_gauge_semilattice(inv, is_bottom_v, is_top_v) \
  do {                                                     \
    BOOST_CHECK((inv).is_bottom() == (is_bottom_v));       \
    BOOST_CHECK((inv).is_top() == (is_top_v));             \
  } while (0)

BOOST_AUTO_TEST_CASE(gauge_semilattice_constructors) {
  test_gauge_semilattice(GaugeSemiLattice::bottom(), true, false);
  test_gauge_semilattice(GaugeSemiLattice::top(), false, true);
  test_gauge_semilattice(l1, false, false);
  test_gauge_semilattice(l2, false, false);
  test_gauge_semilattice(l3, false, false);
  test_gauge_semilattice(l4, false, false);
  test_gauge_semilattice(l5, false, false);
  test_gauge_semilattice(l6, false, false);
  test_gauge_semilattice(l7, false, false);
  test_gauge_semilattice(l8, false, false);
  test_gauge_semilattice(l9, false, false);
  test_gauge_semilattice(l10, false, false);
  test_gauge_semilattice(l11, false, false);
  test_gauge_semilattice(l12, false, false);
  test_gauge_semilattice(l13, false, false);
}

#define test_gauge_semilattice_leq(x, y, r) BOOST_CHECK((x).leq(y) == (r))

BOOST_AUTO_TEST_CASE(gauge_semilattice_leq) {
  test_gauge_semilattice_leq(GaugeSemiLattice::top(),
                             GaugeSemiLattice::top(),
                             true);
  test_gauge_semilattice_leq(GaugeSemiLattice::top(),
                             GaugeSemiLattice::bottom(),
                             false);
  test_gauge_semilattice_leq(GaugeSemiLattice::top(), l1, false);
  test_gauge_semilattice_leq(GaugeSemiLattice::top(), l2, false);
  test_gauge_semilattice_leq(GaugeSemiLattice::top(), l3, false);
  test_gauge_semilattice_leq(GaugeSemiLattice::top(), l4, false);
  test_gauge_semilattice_leq(GaugeSemiLattice::top(), l5, false);
  test_gauge_semilattice_leq(GaugeSemiLattice::bottom(),
                             GaugeSemiLattice::top(),
                             true);
  test_gauge_semilattice_leq(GaugeSemiLattice::bottom(),
                             GaugeSemiLattice::bottom(),
                             true);
  test_gauge_semilattice_leq(GaugeSemiLattice::bottom(), l1, true);
  test_gauge_semilattice_leq(GaugeSemiLattice::bottom(), l2, true);
  test_gauge_semilattice_leq(GaugeSemiLattice::bottom(), l3, true);
  test_gauge_semilattice_leq(GaugeSemiLattice::bottom(), l4, true);
  test_gauge_semilattice_leq(GaugeSemiLattice::bottom(), l5, true);
  test_gauge_semilattice_leq(l1, GaugeSemiLattice::top(), true);
  test_gauge_semilattice_leq(l1, GaugeSemiLattice::bottom(), false);
  test_gauge_semilattice_leq(l1, l1, true);
  test_gauge_semilattice_leq(l1, l2, false);
  test_gauge_semilattice_leq(l1, l3, true);
  test_gauge_semilattice_leq(l1, l4, false);
  test_gauge_semilattice_leq(l1, l5, false);
  test_gauge_semilattice_leq(l2, GaugeSemiLattice::top(), true);
  test_gauge_semilattice_leq(l2, GaugeSemiLattice::bottom(), false);
  test_gauge_semilattice_leq(l2, l1, true);
  test_gauge_semilattice_leq(l2, l2, true);
  test_gauge_semilattice_leq(l2, l3, true);
  test_gauge_semilattice_leq(l2, l4, true);
  test_gauge_semilattice_leq(l2, l5, false);
  test_gauge_semilattice_leq(l3, GaugeSemiLattice::top(), true);
  test_gauge_semilattice_leq(l3, GaugeSemiLattice::bottom(), false);
  test_gauge_semilattice_leq(l3, l1, false);
  test_gauge_semilattice_leq(l3, l2, false);
  test_gauge_semilattice_leq(l3, l3, true);
  test_gauge_semilattice_leq(l3, l4, false);
  test_gauge_semilattice_leq(l3, l5, false);
  test_gauge_semilattice_leq(l4, GaugeSemiLattice::top(), true);
  test_gauge_semilattice_leq(l4, GaugeSemiLattice::bottom(), false);
  test_gauge_semilattice_leq(l4, l1, false);
  test_gauge_semilattice_leq(l4, l2, false);
  test_gauge_semilattice_leq(l4, l3, true);
  test_gauge_semilattice_leq(l4, l4, true);
  test_gauge_semilattice_leq(l4, l5, false);
  test_gauge_semilattice_leq(l5, GaugeSemiLattice::top(), true);
  test_gauge_semilattice_leq(l5, GaugeSemiLattice::bottom(), false);
  test_gauge_semilattice_leq(l5, l1, true);
  test_gauge_semilattice_leq(l5, l2, false);
  test_gauge_semilattice_leq(l5, l3, true);
  test_gauge_semilattice_leq(l5, l4, false);
  test_gauge_semilattice_leq(l5, l5, true);
}

#define test_gauge_semilattice_join(x, y, z) BOOST_CHECK(((x).join(y) == (z)))

BOOST_AUTO_TEST_CASE(gauge_semilattice_join) {
  test_gauge_semilattice_join(GaugeSemiLattice::top(),
                              GaugeSemiLattice::top(),
                              GaugeSemiLattice::top());
  test_gauge_semilattice_join(GaugeSemiLattice::top(),
                              GaugeSemiLattice::bottom(),
                              GaugeSemiLattice::top());
  test_gauge_semilattice_join(GaugeSemiLattice::top(),
                              l1,
                              GaugeSemiLattice::top());
  test_gauge_semilattice_join(GaugeSemiLattice::top(),
                              l2,
                              GaugeSemiLattice::top());
  test_gauge_semilattice_join(GaugeSemiLattice::top(),
                              l3,
                              GaugeSemiLattice::top());
  test_gauge_semilattice_join(GaugeSemiLattice::top(),
                              l4,
                              GaugeSemiLattice::top());
  test_gauge_semilattice_join(GaugeSemiLattice::top(),
                              l5,
                              GaugeSemiLattice::top());
  test_gauge_semilattice_join(GaugeSemiLattice::bottom(),
                              GaugeSemiLattice::top(),
                              GaugeSemiLattice::top());
  test_gauge_semilattice_join(GaugeSemiLattice::bottom(),
                              GaugeSemiLattice::bottom(),
                              GaugeSemiLattice::bottom());
  test_gauge_semilattice_join(GaugeSemiLattice::bottom(), l1, l1);
  test_gauge_semilattice_join(GaugeSemiLattice::bottom(), l2, l2);
  test_gauge_semilattice_join(GaugeSemiLattice::bottom(), l3, l3);
  test_gauge_semilattice_join(GaugeSemiLattice::bottom(), l4, l4);
  test_gauge_semilattice_join(GaugeSemiLattice::bottom(), l5, l5);
  test_gauge_semilattice_join(l1,
                              GaugeSemiLattice::top(),
                              GaugeSemiLattice::top());
  test_gauge_semilattice_join(l1, GaugeSemiLattice::bottom(), l1);
  test_gauge_semilattice_join(l1, l1, l1);
  test_gauge_semilattice_join(l1, l2, l1);
  test_gauge_semilattice_join(l1, l3, l3);
  test_gauge_semilattice_join(l1, l4, l3);
  test_gauge_semilattice_join(l1, l5, l1);
  test_gauge_semilattice_join(l2,
                              GaugeSemiLattice::top(),
                              GaugeSemiLattice::top());
  test_gauge_semilattice_join(l2, GaugeSemiLattice::bottom(), l2);
  test_gauge_semilattice_join(l2, l1, l1);
  test_gauge_semilattice_join(l2, l2, l2);
  test_gauge_semilattice_join(l2, l3, l3);
  test_gauge_semilattice_join(l2, l4, l4);
  test_gauge_semilattice_join(l2, l5, l6);
  test_gauge_semilattice_join(l3,
                              GaugeSemiLattice::top(),
                              GaugeSemiLattice::top());
  test_gauge_semilattice_join(l3, GaugeSemiLattice::bottom(), l3);
  test_gauge_semilattice_join(l3, l1, l3);
  test_gauge_semilattice_join(l3, l2, l3);
  test_gauge_semilattice_join(l3, l3, l3);
  test_gauge_semilattice_join(l3, l4, l3);
  test_gauge_semilattice_join(l3, l5, l3);
  test_gauge_semilattice_join(l4,
                              GaugeSemiLattice::top(),
                              GaugeSemiLattice::top());
  test_gauge_semilattice_join(l4, GaugeSemiLattice::bottom(), l4);
  test_gauge_semilattice_join(l4, l1, l3);
  test_gauge_semilattice_join(l4, l2, l4);
  test_gauge_semilattice_join(l4, l3, l3);
  test_gauge_semilattice_join(l4, l4, l4);
  test_gauge_semilattice_join(l4, l5, l7);
  test_gauge_semilattice_join(l5,
                              GaugeSemiLattice::top(),
                              GaugeSemiLattice::top());
  test_gauge_semilattice_join(l5, GaugeSemiLattice::bottom(), l5);
  test_gauge_semilattice_join(l5, l1, l1);
  test_gauge_semilattice_join(l5, l2, l6);
  test_gauge_semilattice_join(l5, l3, l3);
  test_gauge_semilattice_join(l5, l4, l7);
  test_gauge_semilattice_join(l5, l5, l5);
}

#define test_gauge_semilattice_meet(x, y, z) BOOST_CHECK(((x).meet(y) == (z)))

BOOST_AUTO_TEST_CASE(gauge_semilattice_meet) {
  test_gauge_semilattice_meet(GaugeSemiLattice::top(),
                              GaugeSemiLattice::top(),
                              GaugeSemiLattice::top());
  test_gauge_semilattice_meet(GaugeSemiLattice::top(),
                              GaugeSemiLattice::bottom(),
                              GaugeSemiLattice::bottom());
  test_gauge_semilattice_meet(GaugeSemiLattice::top(), l1, l1);
  test_gauge_semilattice_meet(GaugeSemiLattice::top(), l2, l2);
  test_gauge_semilattice_meet(GaugeSemiLattice::top(), l3, l3);
  test_gauge_semilattice_meet(GaugeSemiLattice::top(), l4, l4);
  test_gauge_semilattice_meet(GaugeSemiLattice::top(), l5, l5);
  test_gauge_semilattice_meet(GaugeSemiLattice::bottom(),
                              GaugeSemiLattice::top(),
                              GaugeSemiLattice::bottom());
  test_gauge_semilattice_meet(GaugeSemiLattice::bottom(),
                              GaugeSemiLattice::bottom(),
                              GaugeSemiLattice::bottom());
  test_gauge_semilattice_meet(GaugeSemiLattice::bottom(),
                              l1,
                              GaugeSemiLattice::bottom());
  test_gauge_semilattice_meet(GaugeSemiLattice::bottom(),
                              l2,
                              GaugeSemiLattice::bottom());
  test_gauge_semilattice_meet(GaugeSemiLattice::bottom(),
                              l3,
                              GaugeSemiLattice::bottom());
  test_gauge_semilattice_meet(GaugeSemiLattice::bottom(),
                              l4,
                              GaugeSemiLattice::bottom());
  test_gauge_semilattice_meet(GaugeSemiLattice::bottom(),
                              l5,
                              GaugeSemiLattice::bottom());
  test_gauge_semilattice_meet(l1, GaugeSemiLattice::top(), l1);
  test_gauge_semilattice_meet(l1,
                              GaugeSemiLattice::bottom(),
                              GaugeSemiLattice::bottom());
  test_gauge_semilattice_meet(l1, l1, l1);
  test_gauge_semilattice_meet(l1, l2, l2);
  test_gauge_semilattice_meet(l1, l3, l1);
  test_gauge_semilattice_meet(l1, l4, l8);
  test_gauge_semilattice_meet(l1, l5, l5);
  test_gauge_semilattice_meet(l2, GaugeSemiLattice::top(), l2);
  test_gauge_semilattice_meet(l2,
                              GaugeSemiLattice::bottom(),
                              GaugeSemiLattice::bottom());
  test_gauge_semilattice_meet(l2, l1, l2);
  test_gauge_semilattice_meet(l2, l2, l2);
  test_gauge_semilattice_meet(l2, l3, l2);
  test_gauge_semilattice_meet(l2, l4, l2);
  test_gauge_semilattice_meet(l2, l5, l9);
  test_gauge_semilattice_meet(l3, GaugeSemiLattice::top(), l3);
  test_gauge_semilattice_meet(l3,
                              GaugeSemiLattice::bottom(),
                              GaugeSemiLattice::bottom());
  test_gauge_semilattice_meet(l3, l1, l1);
  test_gauge_semilattice_meet(l3, l2, l2);
  test_gauge_semilattice_meet(l3, l3, l3);
  test_gauge_semilattice_meet(l3, l4, l4);
  test_gauge_semilattice_meet(l3, l5, l5);
  test_gauge_semilattice_meet(l4, GaugeSemiLattice::top(), l4);
  test_gauge_semilattice_meet(l4,
                              GaugeSemiLattice::bottom(),
                              GaugeSemiLattice::bottom());
  test_gauge_semilattice_meet(l4, l1, l8);
  test_gauge_semilattice_meet(l4, l2, l2);
  test_gauge_semilattice_meet(l4, l3, l4);
  test_gauge_semilattice_meet(l4, l4, l4);
  test_gauge_semilattice_meet(l4, l5, l5);
  test_gauge_semilattice_meet(l5, GaugeSemiLattice::top(), l5);
  test_gauge_semilattice_meet(l5,
                              GaugeSemiLattice::bottom(),
                              GaugeSemiLattice::bottom());
  test_gauge_semilattice_meet(l5, l1, l5);
  test_gauge_semilattice_meet(l5, l2, l5);
  test_gauge_semilattice_meet(l5, l3, l5);
  test_gauge_semilattice_meet(l5, l4, l5);
  test_gauge_semilattice_meet(l5, l5, l5);
}

static void test_gauge_semilattice_forget(GaugeSemiLattice x,
                                          const Variable& v,
                                          const GaugeSemiLattice& y) {
  x.forget(v);
  BOOST_CHECK((x == y));
}

BOOST_AUTO_TEST_CASE(gauge_semilattice_forget) {
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  test_gauge_semilattice_forget(GaugeSemiLattice::top(),
                                x,
                                GaugeSemiLattice::top());
  test_gauge_semilattice_forget(GaugeSemiLattice::bottom(),
                                x,
                                GaugeSemiLattice::bottom());
  test_gauge_semilattice_forget(l1, x, GaugeSemiLattice::top());
  test_gauge_semilattice_forget(l1, y, l1);
  test_gauge_semilattice_forget(l2, y, l1);
  test_gauge_semilattice_forget(l3, x, GaugeSemiLattice::top());
  test_gauge_semilattice_forget(l4, y, l3);
  test_gauge_semilattice_forget(l5, y, l1);
}

static void test_gauge_semilattice_counter_incr(GaugeSemiLattice x,
                                                const Variable& v,
                                                int k,
                                                const GaugeSemiLattice& y) {
  x.counter_incr(v, ZNumber(k));
  BOOST_CHECK((x == y));
}

BOOST_AUTO_TEST_CASE(gauge_semilattice_counter_incr) {
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable i(vfac.get("i"));
  Variable k(vfac.get("k"));

  test_gauge_semilattice_counter_incr(GaugeSemiLattice::top(),
                                      i,
                                      1,
                                      GaugeSemiLattice::top());
  test_gauge_semilattice_counter_incr(GaugeSemiLattice::bottom(),
                                      i,
                                      1,
                                      GaugeSemiLattice::bottom());

  // l1.counter_incr(i, 1)
  test_gauge_semilattice_counter_incr(l1, i, 1, l1);

  // l2.counter_incr(i, 1)
  GaugeSemiLattice l2_prime = l1;
  l2_prime.set(x, Gauge(GaugeBound(0), GaugeBound(1)));
  l2_prime.set(y,
               Gauge(GaugeBound(-2) + GaugeBound(i),
                     GaugeBound(-1) + GaugeBound(2, i)));
  test_gauge_semilattice_counter_incr(l2, i, 1, l2_prime);

  // l3.counter_incr(i, 1)
  test_gauge_semilattice_counter_incr(l3, i, 1, l3);

  // l4.counter_incr(i, 1)
  auto l4_prime = GaugeSemiLattice::top();
  l4_prime.set(x, Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  l4_prime.set(y,
               Gauge(GaugeBound(-3) + GaugeBound(i),
                     GaugeBound(-3) + GaugeBound(3, i)));
  test_gauge_semilattice_counter_incr(l4, i, 1, l4_prime);

  // l5.counter_incr(i, 1)
  test_gauge_semilattice_counter_incr(l5, i, 1, l5);

  // l5.counter_incr(k, 2)
  auto l5_prime = GaugeSemiLattice::top();
  l5_prime.set(x, Gauge(GaugeBound(0), GaugeBound(1)));
  l5_prime.set(y,
               Gauge(GaugeBound(-4) + GaugeBound(2, k),
                     GaugeBound(-4) + GaugeBound(2, k)));
  test_gauge_semilattice_counter_incr(l5, k, 2, l5_prime);

  // l6.counter_incr(i, 1)
  auto l6_prime = GaugeSemiLattice::top();
  l6_prime.set(x, Gauge(GaugeBound(0), GaugeBound(1)));
  l6_prime.set(y, Gauge(GaugeBound(-2), GaugeBound(2, i) + GaugeBound(2, k)));
  test_gauge_semilattice_counter_incr(l6, i, 1, l6_prime);

  // l7.counter_incr(i, 1)
  auto l7_prime = GaugeSemiLattice::top();
  l7_prime.set(x, Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  l7_prime.set(y,
               Gauge(GaugeBound(-3),
                     GaugeBound(-2) + GaugeBound(3, i) + GaugeBound(2, k)));
  test_gauge_semilattice_counter_incr(l7, i, 1, l7_prime);
}

static void test_gauge_semilattice_counter_forget(GaugeSemiLattice x,
                                                  const Variable& v,
                                                  const GaugeSemiLattice& y) {
  x.counter_forget(v);
  BOOST_CHECK((x == y));
}

static void test_gauge_semilattice_counter_forget(GaugeSemiLattice x,
                                                  const Variable& v,
                                                  const ZInterval& intv,
                                                  const GaugeSemiLattice& y) {
  x.counter_forget(v, intv);
  BOOST_CHECK((x == y));
}

BOOST_AUTO_TEST_CASE(gauge_semilattice_counter_forget) {
  Variable x(vfac.get("x"));
  Variable i(vfac.get("i"));
  Variable k(vfac.get("k"));

  test_gauge_semilattice_counter_forget(GaugeSemiLattice::top(),
                                        x,
                                        GaugeSemiLattice::top());
  test_gauge_semilattice_counter_forget(GaugeSemiLattice::bottom(),
                                        x,
                                        GaugeSemiLattice::bottom());
  test_gauge_semilattice_counter_forget(l1, i, l1);
  test_gauge_semilattice_counter_forget(l2, i, l10);
  test_gauge_semilattice_counter_forget(l2,
                                        i,
                                        ZInterval(ZBound(1), ZBound(3)),
                                        l12);
  test_gauge_semilattice_counter_forget(l3, i, l3);
  test_gauge_semilattice_counter_forget(l4, i, l11);
  test_gauge_semilattice_counter_forget(l5, i, l5);
  test_gauge_semilattice_counter_forget(l5, k, l10);
  test_gauge_semilattice_counter_forget(l6, i, l10);
  test_gauge_semilattice_counter_forget(l6,
                                        i,
                                        ZInterval(ZBound(1), ZBound(3)),
                                        l13);
  test_gauge_semilattice_counter_forget(l6,
                                        i,
                                        ZInterval(ZBound(1),
                                                  ZBound::plus_infinity()),
                                        l10);
  test_gauge_semilattice_counter_forget(l6, k, l10);
  test_gauge_semilattice_counter_forget(l7, i, l11);
  test_gauge_semilattice_counter_forget(l7, k, l11);
}

#define test_gauge_semilattice_get(x, v, y) BOOST_CHECK((x).get(v) == (y))

BOOST_AUTO_TEST_CASE(gauge_semilattice_get) {
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  test_gauge_semilattice_get(GaugeSemiLattice::top(), x, Gauge::top());
  test_gauge_semilattice_get(GaugeSemiLattice::bottom(), x, Gauge::bottom());
  test_gauge_semilattice_get(l1, x, Gauge(GaugeBound(0), GaugeBound(1)));
  test_gauge_semilattice_get(l1, y, Gauge::top());
  test_gauge_semilattice_get(l2, x, Gauge(GaugeBound(0), GaugeBound(1)));
  test_gauge_semilattice_get(l3,
                             x,
                             Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  test_gauge_semilattice_get(l4,
                             x,
                             Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  test_gauge_semilattice_get(l5, x, Gauge(GaugeBound(0), GaugeBound(1)));
}

#define test_gauge_domain(d, is_bottom_v, is_top_v) \
  do {                                              \
    BOOST_CHECK((d).is_bottom() == (is_bottom_v));  \
    BOOST_CHECK((d).is_top() == (is_top_v));        \
  } while (0)

BOOST_AUTO_TEST_CASE(gauge_domain_constructors) {
  Variable x(vfac.get("x"));
  Variable i(vfac.get("i"));

  test_gauge_domain(GaugeDomain::bottom(), true, false);
  test_gauge_domain(GaugeDomain::top(), false, true);

  auto d = GaugeDomain::top();
  d.set_to_top();
  d.counter_init(i, ZNumber(0));
  test_gauge_domain(d, false, false);

  d.set_to_top();
  d.counter_init(i, ZNumber(0));
  d.assign(x, 1);
  test_gauge_domain(d, false, false);
}

#define test_gauge_domain_get(x, v, y) BOOST_CHECK((x).to_gauge(v) == (y))

#define test_gauge_domain_to_interval(x, v, y) \
  BOOST_CHECK((x).to_interval(v) == (y))

BOOST_AUTO_TEST_CASE(gauge_domain_assign) {
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));
  Variable i(vfac.get("i"));
  Variable k(vfac.get("k"));

  test_gauge_domain_get(GaugeDomain::bottom(), x, Gauge::bottom());
  test_gauge_domain_to_interval(GaugeDomain::bottom(), x, ZInterval::bottom());
  test_gauge_domain_get(GaugeDomain::top(), x, Gauge::top());
  test_gauge_domain_to_interval(GaugeDomain::top(), x, ZInterval::top());

  auto d = GaugeDomain::top();
  d.counter_init(i, ZNumber(0));
  test_gauge_domain_get(d, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d, i, ZInterval(ZBound(0), ZBound(0)));
  test_gauge_domain_get(d, x, Gauge::top());
  test_gauge_domain_to_interval(d, x, ZInterval::top());

  d.assign(x, 1);
  test_gauge_domain_get(d, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d, i, ZInterval(ZBound(0), ZBound(0)));
  test_gauge_domain_get(d, x, Gauge(GaugeBound(1), GaugeBound(1)));
  test_gauge_domain_to_interval(d, x, ZInterval(ZBound(1), ZBound(1)));

  d.assign(y, 2 * VariableExpression(i) + 1);
  test_gauge_domain_get(d, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d, i, ZInterval(ZBound(0), ZBound(0)));
  test_gauge_domain_get(d, x, Gauge(GaugeBound(1), GaugeBound(1)));
  test_gauge_domain_to_interval(d, x, ZInterval(ZBound(1), ZBound(1)));
  test_gauge_domain_get(d, y, Gauge(GaugeBound(1) + GaugeBound(2, i)));
  test_gauge_domain_to_interval(d, y, ZInterval(ZBound(1), ZBound(1)));

  d.counter_incr(i, ZNumber(1));
  test_gauge_domain_get(d, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d, i, ZInterval(ZBound(1), ZBound(1)));
  test_gauge_domain_get(d, x, Gauge(GaugeBound(1), GaugeBound(1)));
  test_gauge_domain_to_interval(d, x, ZInterval(ZBound(1), ZBound(1)));
  test_gauge_domain_get(d, y, Gauge(GaugeBound(-1) + GaugeBound(2, i)));
  test_gauge_domain_to_interval(d, y, ZInterval(ZBound(1), ZBound(1)));

  d.counter_init(k, ZNumber(0));
  test_gauge_domain_get(d, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d, i, ZInterval(ZBound(1), ZBound(1)));
  test_gauge_domain_get(d, k, Gauge(GaugeBound(k)));
  test_gauge_domain_to_interval(d, k, ZInterval(ZBound(0), ZBound(0)));
  test_gauge_domain_get(d, x, Gauge(GaugeBound(1), GaugeBound(1)));
  test_gauge_domain_to_interval(d, x, ZInterval(ZBound(1), ZBound(1)));
  test_gauge_domain_get(d, y, Gauge(GaugeBound(-1) + GaugeBound(2, i)));
  test_gauge_domain_to_interval(d, y, ZInterval(ZBound(1), ZBound(1)));

  d.assign(z, 3 * VariableExpression(k) + 2 * VariableExpression(y));
  test_gauge_domain_get(d, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d, i, ZInterval(ZBound(1), ZBound(1)));
  test_gauge_domain_get(d, k, Gauge(GaugeBound(k)));
  test_gauge_domain_to_interval(d, k, ZInterval(ZBound(0), ZBound(0)));
  test_gauge_domain_get(d, x, Gauge(GaugeBound(1), GaugeBound(1)));
  test_gauge_domain_to_interval(d, x, ZInterval(ZBound(1), ZBound(1)));
  test_gauge_domain_get(d, y, Gauge(GaugeBound(-1) + GaugeBound(2, i)));
  test_gauge_domain_to_interval(d, y, ZInterval(ZBound(1), ZBound(1)));
  test_gauge_domain_get(d,
                        z,
                        Gauge(GaugeBound(-2) + GaugeBound(4, i) +
                              GaugeBound(3, k)));
  test_gauge_domain_to_interval(d, z, ZInterval(ZBound(2), ZBound(2)));

  d.counter_incr(k, ZNumber(1));
  test_gauge_domain_get(d, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d, i, ZInterval(ZBound(1), ZBound(1)));
  test_gauge_domain_get(d, k, Gauge(GaugeBound(k)));
  test_gauge_domain_to_interval(d, k, ZInterval(ZBound(1), ZBound(1)));
  test_gauge_domain_get(d, x, Gauge(GaugeBound(1), GaugeBound(1)));
  test_gauge_domain_to_interval(d, x, ZInterval(ZBound(1), ZBound(1)));
  test_gauge_domain_get(d, y, Gauge(GaugeBound(-1) + GaugeBound(2, i)));
  test_gauge_domain_to_interval(d, y, ZInterval(ZBound(1), ZBound(1)));
  test_gauge_domain_get(d,
                        z,
                        Gauge(GaugeBound(-5) + GaugeBound(4, i) +
                              GaugeBound(3, k)));
  test_gauge_domain_to_interval(d, z, ZInterval(ZBound(2), ZBound(2)));
}

BOOST_AUTO_TEST_CASE(gauge_domain_widening) {
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable i(vfac.get("i"));

  auto d1 = GaugeDomain::top();
  d1.counter_init(i, ZNumber(0));
  d1.assign(x, 0);
  d1.assign(y, 1);
  test_gauge_domain_get(d1, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d1, i, ZInterval(0));
  test_gauge_domain_get(d1, x, Gauge(0));
  test_gauge_domain_to_interval(d1, x, ZInterval(0));
  test_gauge_domain_get(d1, y, Gauge(1));
  test_gauge_domain_to_interval(d1, y, ZInterval(1));

  GaugeDomain d2 = d1;
  d2.counter_incr(i, ZNumber(1));
  d2.assign(x, VariableExpression(x) + 2);
  d2.assign(y, VariableExpression(y) + 3);
  test_gauge_domain_get(d2, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d2, i, ZInterval(1));
  test_gauge_domain_get(d2, x, Gauge(2));
  test_gauge_domain_to_interval(d2, x, ZInterval(2));
  test_gauge_domain_get(d2, y, Gauge(4));
  test_gauge_domain_to_interval(d2, y, ZInterval(4));

  GaugeDomain d3 = d1.widening(d2);
  test_gauge_domain_get(d3, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d3,
                                i,
                                ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_gauge_domain_get(d3, x, Gauge(GaugeBound(2, i)));
  test_gauge_domain_to_interval(d3,
                                x,
                                ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_gauge_domain_get(d3, y, Gauge(GaugeBound(1) + GaugeBound(3, i)));
  test_gauge_domain_to_interval(d3,
                                y,
                                ZInterval(ZBound(1), ZBound::plus_infinity()));

  GaugeDomain d4 = d3;
  d4.assign(y, 3 * VariableExpression(i) + 2);

  GaugeDomain d5 = d3.widening(d4);
  test_gauge_domain_get(d5, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d5,
                                i,
                                ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_gauge_domain_get(d5, x, Gauge(GaugeBound(2, i)));
  test_gauge_domain_to_interval(d5,
                                x,
                                ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_gauge_domain_get(d5,
                        y,
                        Gauge(GaugeBound(1) + GaugeBound(3, i),
                              GaugeBound::plus_infinity()));
  test_gauge_domain_to_interval(d5,
                                y,
                                ZInterval(ZBound(1), ZBound::plus_infinity()));
}

BOOST_AUTO_TEST_CASE(gauge_domain_widening_narrowing_threshold) {
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable i(vfac.get("i"));

  auto d1 = GaugeDomain::top();
  d1.counter_init(i, ZNumber(0));
  d1.assign(x, 0);
  d1.assign(y, 1);

  GaugeDomain d2 = d1;
  d2.counter_incr(i, ZNumber(1));
  d2.assign(x, VariableExpression(x) + 2);
  d2.assign(y, VariableExpression(y) + 3);

  GaugeDomain d3 = d1.widening_threshold(d2, ZNumber(10));
  test_gauge_domain_get(d3, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d3, i, ZInterval(ZBound(0), ZBound(10)));
  test_gauge_domain_get(d3, x, Gauge(GaugeBound(2, i)));
  test_gauge_domain_to_interval(d3, x, ZInterval(ZBound(0), ZBound(20)));
  test_gauge_domain_get(d3, y, Gauge(GaugeBound(1) + GaugeBound(3, i)));
  test_gauge_domain_to_interval(d3, y, ZInterval(ZBound(1), ZBound(31)));

  GaugeDomain d4 = d3;
  d4.add(VariableExpression(i) <= 5);
  d4 = d4.narrowing_threshold(d3, ZNumber(10));
  test_gauge_domain_get(d4, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d4, i, ZInterval(ZBound(0), ZBound(5)));
  test_gauge_domain_get(d4, x, Gauge(GaugeBound(2, i)));
  test_gauge_domain_to_interval(d4, x, ZInterval(ZBound(0), ZBound(10)));
  test_gauge_domain_get(d4, y, Gauge(GaugeBound(1) + GaugeBound(3, i)));
  test_gauge_domain_to_interval(d4, y, ZInterval(ZBound(1), ZBound(16)));
}

BOOST_AUTO_TEST_CASE(gauge_domain_add) {
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable i(vfac.get("i"));

  auto d1 = GaugeDomain::top();
  d1.counter_init(i, ZNumber(0));
  d1.assign(x, 0);
  d1.assign(y, 1);
  d1.add(VariableExpression(i) <= 10);
  test_gauge_domain_get(d1, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d1, i, ZInterval(0));
  test_gauge_domain_get(d1, x, Gauge(0));
  test_gauge_domain_get(d1, y, Gauge(1));

  GaugeDomain d2 = d1;
  d2.counter_incr(i, ZNumber(1));
  d2.assign(x, VariableExpression(x) + 2);
  d2.assign(y, VariableExpression(y) + 3);
  d2.add(VariableExpression(i) <= 10);
  test_gauge_domain_get(d2, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d2, i, ZInterval(1));
  test_gauge_domain_get(d2, x, Gauge(2));
  test_gauge_domain_get(d2, y, Gauge(4));

  GaugeDomain d3 = d1.widening(d2);
  d3.add(VariableExpression(i) <= 10);
  test_gauge_domain_get(d3, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d3, i, ZInterval(ZBound(0), ZBound(10)));
  test_gauge_domain_get(d3, x, Gauge(GaugeBound(2, i)));
  test_gauge_domain_to_interval(d3, x, ZInterval(ZBound(0), ZBound(20)));
  test_gauge_domain_get(d3, y, Gauge(GaugeBound(1) + GaugeBound(3, i)));
  test_gauge_domain_to_interval(d3, y, ZInterval(ZBound(1), ZBound(31)));

  GaugeDomain d4 = d3;
  d4.add(VariableExpression(i) >= 11);
  test_gauge_domain_get(d4, i, Gauge::bottom());

  GaugeDomain d5 = d3;
  d5.add(VariableExpression(i) == 10);
  test_gauge_domain_get(d5, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d5, i, ZInterval(10));
  test_gauge_domain_get(d5, x, Gauge(GaugeBound(2, i)));
  test_gauge_domain_to_interval(d5, x, ZInterval(20));
  test_gauge_domain_get(d5, y, Gauge(GaugeBound(1) + GaugeBound(3, i)));
  test_gauge_domain_to_interval(d5, y, ZInterval(31));

  GaugeDomain d6 = d3;
  d6.add(VariableExpression(i) != 10);
  test_gauge_domain_get(d6, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d6, i, ZInterval(ZBound(0), ZBound(9)));
  test_gauge_domain_get(d6, x, Gauge(GaugeBound(2, i)));
  test_gauge_domain_to_interval(d6, x, ZInterval(ZBound(0), ZBound(18)));
  test_gauge_domain_get(d6, y, Gauge(GaugeBound(1) + GaugeBound(3, i)));
  test_gauge_domain_to_interval(d6, y, ZInterval(ZBound(1), ZBound(28)));
}

BOOST_AUTO_TEST_CASE(gauge_domain_forget) {
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable i(vfac.get("i"));

  auto d1 = GaugeDomain::top();
  d1.counter_init(i, ZNumber(0));
  d1.assign(x, 0);
  d1.assign(y, 1);
  d1.add(VariableExpression(i) <= 10);

  GaugeDomain d2 = d1;
  d2.counter_incr(i, ZNumber(1));
  d2.assign(x, VariableExpression(x) + 2);
  d2.assign(y, VariableExpression(y) + 3);
  d2.add(VariableExpression(i) <= 10);

  GaugeDomain d3 = d1.widening(d2);
  d3.add(VariableExpression(i) <= 10);

  d1.forget(i);
  test_gauge_domain_get(d1, i, Gauge::top());
  test_gauge_domain_to_interval(d1, i, ZInterval::top());
  test_gauge_domain_get(d1, x, Gauge(0));
  test_gauge_domain_get(d1, y, Gauge(1));

  d3.forget(y);
  test_gauge_domain_get(d3, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d3, i, ZInterval(ZBound(0), ZBound(10)));
  test_gauge_domain_get(d3, x, Gauge(GaugeBound(2, i)));
  test_gauge_domain_to_interval(d3, x, ZInterval(ZBound(0), ZBound(20)));
  test_gauge_domain_get(d3, y, Gauge::top());
  test_gauge_domain_to_interval(d3, y, ZInterval::top());

  d3.forget(i);
  test_gauge_domain_get(d3, i, Gauge::top());
  test_gauge_domain_to_interval(d3, i, ZInterval::top());
  test_gauge_domain_get(d3, x, Gauge(GaugeBound(0), GaugeBound(20)));
  test_gauge_domain_to_interval(d3, x, ZInterval(ZBound(0), ZBound(20)));
}

BOOST_AUTO_TEST_CASE(gauge_domain_set) {
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto d1 = GaugeDomain::top();
  d1.assign(x, 0);
  d1.assign(y, 1);

  d1.set(x, ZInterval::top());
  test_gauge_domain_get(d1, x, Gauge::top());
  test_gauge_domain_get(d1, y, Gauge(1));

  d1.set(x, ZInterval(ZBound(1), ZBound(2)));
  test_gauge_domain_get(d1, x, Gauge(GaugeBound(1), GaugeBound(2)));
  test_gauge_domain_get(d1, y, Gauge(1));

  d1.set(x, ZInterval(ZBound(1), ZBound::plus_infinity()));
  test_gauge_domain_get(d1,
                        x,
                        Gauge(GaugeBound(1), GaugeBound::plus_infinity()));
  test_gauge_domain_get(d1, y, Gauge(1));

  d1.set(x, ZInterval(ZBound::minus_infinity(), ZBound(1)));
  test_gauge_domain_get(d1,
                        x,
                        Gauge(GaugeBound::minus_infinity(), GaugeBound(1)));
  test_gauge_domain_get(d1, y, Gauge(1));
}

BOOST_AUTO_TEST_CASE(gauge_domain_apply_var_var_var) {
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));
  Variable w(vfac.get("w"));
  Variable i(vfac.get("i"));
  Variable k(vfac.get("k"));
  Variable zero(vfac.get("zero"));
  Variable one(vfac.get("one"));
  Variable two(vfac.get("two"));

  auto d1 = GaugeDomain::top();
  d1.assign(zero, 0);
  d1.assign(one, 1);
  d1.assign(two, 2);
  d1.counter_init(i, ZNumber(0));
  d1.assign(x, 3);
  d1.assign(y, 2);
  d1.assign(z, 2);
  d1.assign(w, 16);
  test_gauge_domain_get(d1, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d1, i, ZInterval(0));
  test_gauge_domain_get(d1, x, Gauge(3));
  test_gauge_domain_to_interval(d1, x, ZInterval(3));
  test_gauge_domain_get(d1, y, Gauge(2));
  test_gauge_domain_to_interval(d1, y, ZInterval(2));
  test_gauge_domain_get(d1, z, Gauge(2));
  test_gauge_domain_to_interval(d1, z, ZInterval(2));
  test_gauge_domain_get(d1, w, Gauge(16));
  test_gauge_domain_to_interval(d1, w, ZInterval(16));

  GaugeDomain d2 = d1;
  d2.apply(BinaryOperator::Add, x, x, two);
  d2.apply(BinaryOperator::Sub, y, y, one);
  d2.apply(BinaryOperator::Mul, z, z, two);
  d2.apply(BinaryOperator::Div, w, w, two);
  d2.apply(BinaryOperator::Add, k, i, one);
  test_gauge_domain_get(d2, x, Gauge(5));
  test_gauge_domain_to_interval(d2, x, ZInterval(5));
  test_gauge_domain_get(d2, y, Gauge(1));
  test_gauge_domain_to_interval(d2, y, ZInterval(1));
  test_gauge_domain_get(d2, z, Gauge(4));
  test_gauge_domain_to_interval(d2, z, ZInterval(4));
  test_gauge_domain_get(d2, w, Gauge(8));
  test_gauge_domain_to_interval(d2, w, ZInterval(8));
  test_gauge_domain_get(d2, k, Gauge(GaugeBound(1) + GaugeBound(i)));
  test_gauge_domain_to_interval(d2, k, ZInterval(1));

  d2.counter_incr(i, ZNumber(1));
  test_gauge_domain_get(d2, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d2, i, ZInterval(1));
  test_gauge_domain_get(d2, k, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d2, k, ZInterval(1));

  GaugeDomain d3 = d1.widening(d2);
  test_gauge_domain_get(d3, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d3,
                                i,
                                ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_gauge_domain_get(d3, k, Gauge::top());
  test_gauge_domain_to_interval(d3, k, ZInterval::top());
  test_gauge_domain_get(d3, x, Gauge(GaugeBound(3) + GaugeBound(2, i)));
  test_gauge_domain_to_interval(d3,
                                x,
                                ZInterval(ZBound(3), ZBound::plus_infinity()));
  test_gauge_domain_get(d3, y, Gauge(GaugeBound(2) + GaugeBound(-1, i)));
  test_gauge_domain_to_interval(d3,
                                y,
                                ZInterval(ZBound::minus_infinity(), ZBound(2)));
  test_gauge_domain_get(d3, z, Gauge(GaugeBound(2) + GaugeBound(2, i)));
  test_gauge_domain_to_interval(d3,
                                z,
                                ZInterval(ZBound(2), ZBound::plus_infinity()));
  test_gauge_domain_get(d3, w, Gauge(GaugeBound(16) + GaugeBound(-8, i)));
  test_gauge_domain_to_interval(d3,
                                w,
                                ZInterval(ZBound::minus_infinity(),
                                          ZBound(16)));

  GaugeDomain d4 = d3;
  d4.apply(BinaryOperator::Add, x, x, two);
  d4.apply(BinaryOperator::Sub, y, y, one);
  d4.apply(BinaryOperator::Mul, z, z, two);
  d4.apply(BinaryOperator::Div, w, w, two);
  d4.apply(BinaryOperator::Add, k, i, one);
  test_gauge_domain_get(d4, k, Gauge(GaugeBound(1) + GaugeBound(i)));
  test_gauge_domain_to_interval(d4,
                                k,
                                ZInterval(ZBound(1), ZBound::plus_infinity()));
  test_gauge_domain_get(d4, x, Gauge(GaugeBound(5) + GaugeBound(2, i)));
  test_gauge_domain_to_interval(d4,
                                x,
                                ZInterval(ZBound(5), ZBound::plus_infinity()));
  test_gauge_domain_get(d4, y, Gauge(GaugeBound(1) + GaugeBound(-1, i)));
  test_gauge_domain_to_interval(d4,
                                y,
                                ZInterval(ZBound::minus_infinity(), ZBound(1)));
  test_gauge_domain_get(d4, z, Gauge(GaugeBound(4) + GaugeBound(4, i)));
  test_gauge_domain_to_interval(d4,
                                z,
                                ZInterval(ZBound(4), ZBound::plus_infinity()));
  test_gauge_domain_get(d4,
                        w,
                        Gauge(GaugeBound::minus_infinity(), GaugeBound(8)));
  test_gauge_domain_to_interval(d4,
                                w,
                                ZInterval(ZBound::minus_infinity(), ZBound(8)));

  d4.counter_incr(i, ZNumber(1));
  test_gauge_domain_get(d4, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d4,
                                i,
                                ZInterval(ZBound(1), ZBound::plus_infinity()));
  test_gauge_domain_get(d4, k, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d4,
                                k,
                                ZInterval(ZBound(1), ZBound::plus_infinity()));
  test_gauge_domain_get(d4, x, Gauge(GaugeBound(3) + GaugeBound(2, i)));
  test_gauge_domain_to_interval(d4,
                                x,
                                ZInterval(ZBound(5), ZBound::plus_infinity()));
  test_gauge_domain_get(d4, y, Gauge(GaugeBound(2) + GaugeBound(-1, i)));
  test_gauge_domain_to_interval(d4,
                                y,
                                ZInterval(ZBound::minus_infinity(), ZBound(1)));
  test_gauge_domain_get(d4, z, Gauge(GaugeBound(0) + GaugeBound(4, i)));
  test_gauge_domain_to_interval(d4,
                                z,
                                ZInterval(ZBound(4), ZBound::plus_infinity()));
  test_gauge_domain_get(d4,
                        w,
                        Gauge(GaugeBound::minus_infinity(), GaugeBound(8)));
  test_gauge_domain_to_interval(d4,
                                w,
                                ZInterval(ZBound::minus_infinity(), ZBound(8)));

  GaugeDomain d5 = d3.join(d4);
  test_gauge_domain_get(d5, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d5,
                                i,
                                ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_gauge_domain_get(d5, k, Gauge::top());
  test_gauge_domain_to_interval(d5, k, ZInterval::top());
  test_gauge_domain_get(d5, x, Gauge(GaugeBound(3) + GaugeBound(2, i)));
  test_gauge_domain_to_interval(d5,
                                x,
                                ZInterval(ZBound(3), ZBound::plus_infinity()));
  test_gauge_domain_get(d5, y, Gauge(GaugeBound(2) + GaugeBound(-1, i)));
  test_gauge_domain_to_interval(d5,
                                y,
                                ZInterval(ZBound::minus_infinity(), ZBound(2)));
  test_gauge_domain_get(d5,
                        z,
                        Gauge(GaugeBound(2, i),
                              GaugeBound(2) + GaugeBound(4, i)));
  test_gauge_domain_to_interval(d5,
                                z,
                                ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_gauge_domain_get(d5,
                        w,
                        Gauge(GaugeBound::minus_infinity(), GaugeBound(16)));
  test_gauge_domain_to_interval(d5,
                                w,
                                ZInterval(ZBound::minus_infinity(),
                                          ZBound(16)));
}

BOOST_AUTO_TEST_CASE(gauge_domain_apply_var_var_num) {
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));
  Variable w(vfac.get("w"));
  Variable i(vfac.get("i"));
  Variable k(vfac.get("k"));

  auto d1 = GaugeDomain::top();
  d1.counter_init(i, ZNumber(0));
  d1.assign(x, 3);
  d1.assign(y, 2);
  d1.assign(z, 2);
  d1.assign(w, 16);
  test_gauge_domain_get(d1, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d1, i, ZInterval(0));
  test_gauge_domain_get(d1, x, Gauge(3));
  test_gauge_domain_to_interval(d1, x, ZInterval(3));
  test_gauge_domain_get(d1, y, Gauge(2));
  test_gauge_domain_to_interval(d1, y, ZInterval(2));
  test_gauge_domain_get(d1, z, Gauge(2));
  test_gauge_domain_to_interval(d1, z, ZInterval(2));
  test_gauge_domain_get(d1, w, Gauge(16));
  test_gauge_domain_to_interval(d1, w, ZInterval(16));

  GaugeDomain d2 = d1;
  d2.apply(BinaryOperator::Add, x, x, ZNumber(2));
  d2.apply(BinaryOperator::Sub, y, y, ZNumber(1));
  d2.apply(BinaryOperator::Mul, z, z, ZNumber(2));
  d2.apply(BinaryOperator::Div, w, w, ZNumber(2));
  d2.apply(BinaryOperator::Add, k, i, ZNumber(1));
  test_gauge_domain_get(d2, x, Gauge(5));
  test_gauge_domain_to_interval(d2, x, ZInterval(5));
  test_gauge_domain_get(d2, y, Gauge(1));
  test_gauge_domain_to_interval(d2, y, ZInterval(1));
  test_gauge_domain_get(d2, z, Gauge(4));
  test_gauge_domain_to_interval(d2, z, ZInterval(4));
  test_gauge_domain_get(d2, w, Gauge(8));
  test_gauge_domain_to_interval(d2, w, ZInterval(8));
  test_gauge_domain_get(d2, k, Gauge(GaugeBound(1) + GaugeBound(i)));
  test_gauge_domain_to_interval(d2, k, ZInterval(1));

  d2.counter_incr(i, ZNumber(1));
  test_gauge_domain_get(d2, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d2, i, ZInterval(1));
  test_gauge_domain_get(d2, k, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d2, k, ZInterval(1));

  GaugeDomain d3 = d1.widening(d2);
  test_gauge_domain_get(d3, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d3,
                                i,
                                ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_gauge_domain_get(d3, k, Gauge::top());
  test_gauge_domain_to_interval(d3, k, ZInterval::top());
  test_gauge_domain_get(d3, x, Gauge(GaugeBound(3) + GaugeBound(2, i)));
  test_gauge_domain_to_interval(d3,
                                x,
                                ZInterval(ZBound(3), ZBound::plus_infinity()));
  test_gauge_domain_get(d3, y, Gauge(GaugeBound(2) + GaugeBound(-1, i)));
  test_gauge_domain_to_interval(d3,
                                y,
                                ZInterval(ZBound::minus_infinity(), ZBound(2)));
  test_gauge_domain_get(d3, z, Gauge(GaugeBound(2) + GaugeBound(2, i)));
  test_gauge_domain_to_interval(d3,
                                z,
                                ZInterval(ZBound(2), ZBound::plus_infinity()));
  test_gauge_domain_get(d3, w, Gauge(GaugeBound(16) + GaugeBound(-8, i)));
  test_gauge_domain_to_interval(d3,
                                w,
                                ZInterval(ZBound::minus_infinity(),
                                          ZBound(16)));

  GaugeDomain d4 = d3;
  d4.apply(BinaryOperator::Add, x, x, ZNumber(2));
  d4.apply(BinaryOperator::Sub, y, y, ZNumber(1));
  d4.apply(BinaryOperator::Mul, z, z, ZNumber(2));
  d4.apply(BinaryOperator::Div, w, w, ZNumber(2));
  d4.apply(BinaryOperator::Add, k, i, ZNumber(1));
  test_gauge_domain_get(d4, k, Gauge(GaugeBound(1) + GaugeBound(i)));
  test_gauge_domain_to_interval(d4,
                                k,
                                ZInterval(ZBound(1), ZBound::plus_infinity()));
  test_gauge_domain_get(d4, x, Gauge(GaugeBound(5) + GaugeBound(2, i)));
  test_gauge_domain_to_interval(d4,
                                x,
                                ZInterval(ZBound(5), ZBound::plus_infinity()));
  test_gauge_domain_get(d4, y, Gauge(GaugeBound(1) + GaugeBound(-1, i)));
  test_gauge_domain_to_interval(d4,
                                y,
                                ZInterval(ZBound::minus_infinity(), ZBound(1)));
  test_gauge_domain_get(d4, z, Gauge(GaugeBound(4) + GaugeBound(4, i)));
  test_gauge_domain_to_interval(d4,
                                z,
                                ZInterval(ZBound(4), ZBound::plus_infinity()));
  test_gauge_domain_get(d4,
                        w,
                        Gauge(GaugeBound::minus_infinity(), GaugeBound(8)));
  test_gauge_domain_to_interval(d4,
                                w,
                                ZInterval(ZBound::minus_infinity(), ZBound(8)));

  d4.counter_incr(i, ZNumber(1));
  test_gauge_domain_get(d4, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d4,
                                i,
                                ZInterval(ZBound(1), ZBound::plus_infinity()));
  test_gauge_domain_get(d4, k, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d4,
                                k,
                                ZInterval(ZBound(1), ZBound::plus_infinity()));
  test_gauge_domain_get(d4, x, Gauge(GaugeBound(3) + GaugeBound(2, i)));
  test_gauge_domain_to_interval(d4,
                                x,
                                ZInterval(ZBound(5), ZBound::plus_infinity()));
  test_gauge_domain_get(d4, y, Gauge(GaugeBound(2) + GaugeBound(-1, i)));
  test_gauge_domain_to_interval(d4,
                                y,
                                ZInterval(ZBound::minus_infinity(), ZBound(1)));
  test_gauge_domain_get(d4, z, Gauge(GaugeBound(0) + GaugeBound(4, i)));
  test_gauge_domain_to_interval(d4,
                                z,
                                ZInterval(ZBound(4), ZBound::plus_infinity()));
  test_gauge_domain_get(d4,
                        w,
                        Gauge(GaugeBound::minus_infinity(), GaugeBound(8)));
  test_gauge_domain_to_interval(d4,
                                w,
                                ZInterval(ZBound::minus_infinity(), ZBound(8)));

  GaugeDomain d5 = d3.join(d4);
  test_gauge_domain_get(d5, i, Gauge(GaugeBound(i)));
  test_gauge_domain_to_interval(d5,
                                i,
                                ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_gauge_domain_get(d5, k, Gauge::top());
  test_gauge_domain_to_interval(d5, k, ZInterval::top());
  test_gauge_domain_get(d5, x, Gauge(GaugeBound(3) + GaugeBound(2, i)));
  test_gauge_domain_to_interval(d5,
                                x,
                                ZInterval(ZBound(3), ZBound::plus_infinity()));
  test_gauge_domain_get(d5, y, Gauge(GaugeBound(2) + GaugeBound(-1, i)));
  test_gauge_domain_to_interval(d5,
                                y,
                                ZInterval(ZBound::minus_infinity(), ZBound(2)));
  test_gauge_domain_get(d5,
                        z,
                        Gauge(GaugeBound(2, i),
                              GaugeBound(2) + GaugeBound(4, i)));
  test_gauge_domain_to_interval(d5,
                                z,
                                ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_gauge_domain_get(d5,
                        w,
                        Gauge(GaugeBound::minus_infinity(), GaugeBound(16)));
  test_gauge_domain_to_interval(d5,
                                w,
                                ZInterval(ZBound::minus_infinity(),
                                          ZBound(16)));
}
