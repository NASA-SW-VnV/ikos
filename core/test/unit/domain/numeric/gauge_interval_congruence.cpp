/*******************************************************************************
 *
 * Tests for GaugeIntervalCongruenceDomain
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

#define BOOST_TEST_MODULE test_gauge_interval_congruence_domain
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <ikos/core/domain/numeric/gauge_interval_congruence.hpp>
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
using GaugeIntervalCongruenceDomain =
    ikos::core::numeric::GaugeIntervalCongruenceDomain< ZNumber, Variable >;

#define test_domain(d, is_bottom_v, is_top_v)      \
  do {                                             \
    BOOST_CHECK((d).is_bottom() == (is_bottom_v)); \
    BOOST_CHECK((d).is_top() == (is_top_v));       \
  } while (0)

BOOST_AUTO_TEST_CASE(gauge_interval_congruence_domain_constructors) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable i(vfac.get("i"));

  test_domain(GaugeIntervalCongruenceDomain::bottom(), true, false);
  test_domain(GaugeIntervalCongruenceDomain::top(), false, true);

  auto d = GaugeIntervalCongruenceDomain::top();
  d.set_to_top();
  d.counter_init(i, ZNumber(0));
  test_domain(d, false, false);

  d.set_to_top();
  d.counter_init(i, ZNumber(0));
  d.assign(x, 1);
  test_domain(d, false, false);
}

#define test_domain_get(x, v, y) BOOST_CHECK((x).to_gauge(v) == (y))

#define test_domain_to_interval(x, v, y) BOOST_CHECK((x).to_interval(v) == (y))

BOOST_AUTO_TEST_CASE(gauge_interval_congruence_domain_assign) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));
  Variable i(vfac.get("i"));
  Variable k(vfac.get("k"));

  test_domain_get(GaugeIntervalCongruenceDomain::bottom(), x, Gauge::bottom());
  test_domain_to_interval(GaugeIntervalCongruenceDomain::bottom(),
                          x,
                          ZInterval::bottom());
  test_domain_get(GaugeIntervalCongruenceDomain::top(), x, Gauge::top());
  test_domain_to_interval(GaugeIntervalCongruenceDomain::top(),
                          x,
                          ZInterval::top());

  auto d = GaugeIntervalCongruenceDomain::top();
  d.counter_init(i, ZNumber(0));
  test_domain_get(d, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d, i, ZInterval(ZBound(0), ZBound(0)));
  test_domain_get(d, x, Gauge::top());
  test_domain_to_interval(d, x, ZInterval::top());

  d.assign(x, 1);
  test_domain_get(d, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d, i, ZInterval(ZBound(0), ZBound(0)));
  test_domain_get(d, x, Gauge(GaugeBound(1), GaugeBound(1)));
  test_domain_to_interval(d, x, ZInterval(ZBound(1), ZBound(1)));

  d.assign(y, 2 * VariableExpression(i) + 1);
  test_domain_get(d, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d, i, ZInterval(ZBound(0), ZBound(0)));
  test_domain_get(d, x, Gauge(GaugeBound(1), GaugeBound(1)));
  test_domain_to_interval(d, x, ZInterval(ZBound(1), ZBound(1)));
  test_domain_get(d, y, Gauge(GaugeBound(1) + GaugeBound(2, i)));
  test_domain_to_interval(d, y, ZInterval(ZBound(1), ZBound(1)));

  d.counter_incr(i, ZNumber(1));
  test_domain_get(d, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d, i, ZInterval(ZBound(1), ZBound(1)));
  test_domain_get(d, x, Gauge(GaugeBound(1), GaugeBound(1)));
  test_domain_to_interval(d, x, ZInterval(ZBound(1), ZBound(1)));
  test_domain_get(d, y, Gauge(GaugeBound(-1) + GaugeBound(2, i)));
  test_domain_to_interval(d, y, ZInterval(ZBound(1), ZBound(1)));

  d.counter_init(k, ZNumber(0));
  test_domain_get(d, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d, i, ZInterval(ZBound(1), ZBound(1)));
  test_domain_get(d, k, Gauge(GaugeBound(k)));
  test_domain_to_interval(d, k, ZInterval(ZBound(0), ZBound(0)));
  test_domain_get(d, x, Gauge(GaugeBound(1), GaugeBound(1)));
  test_domain_to_interval(d, x, ZInterval(ZBound(1), ZBound(1)));
  test_domain_get(d, y, Gauge(GaugeBound(-1) + GaugeBound(2, i)));
  test_domain_to_interval(d, y, ZInterval(ZBound(1), ZBound(1)));

  d.assign(z, 3 * VariableExpression(k) + 2 * VariableExpression(y));
  test_domain_get(d, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d, i, ZInterval(ZBound(1), ZBound(1)));
  test_domain_get(d, k, Gauge(GaugeBound(k)));
  test_domain_to_interval(d, k, ZInterval(ZBound(0), ZBound(0)));
  test_domain_get(d, x, Gauge(GaugeBound(1), GaugeBound(1)));
  test_domain_to_interval(d, x, ZInterval(ZBound(1), ZBound(1)));
  test_domain_get(d, y, Gauge(GaugeBound(-1) + GaugeBound(2, i)));
  test_domain_to_interval(d, y, ZInterval(ZBound(1), ZBound(1)));
  test_domain_get(d,
                  z,
                  Gauge(GaugeBound(-2) + GaugeBound(4, i) + GaugeBound(3, k)));
  test_domain_to_interval(d, z, ZInterval(ZBound(2), ZBound(2)));

  d.counter_incr(k, ZNumber(1));
  test_domain_get(d, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d, i, ZInterval(ZBound(1), ZBound(1)));
  test_domain_get(d, k, Gauge(GaugeBound(k)));
  test_domain_to_interval(d, k, ZInterval(ZBound(1), ZBound(1)));
  test_domain_get(d, x, Gauge(GaugeBound(1), GaugeBound(1)));
  test_domain_to_interval(d, x, ZInterval(ZBound(1), ZBound(1)));
  test_domain_get(d, y, Gauge(GaugeBound(-1) + GaugeBound(2, i)));
  test_domain_to_interval(d, y, ZInterval(ZBound(1), ZBound(1)));
  test_domain_get(d,
                  z,
                  Gauge(GaugeBound(-5) + GaugeBound(4, i) + GaugeBound(3, k)));
  test_domain_to_interval(d, z, ZInterval(ZBound(2), ZBound(2)));
}

BOOST_AUTO_TEST_CASE(gauge_interval_congruence_domain_widening) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable i(vfac.get("i"));

  auto d1 = GaugeIntervalCongruenceDomain::top();
  d1.counter_init(i, ZNumber(0));
  d1.assign(x, 0);
  d1.assign(y, 1);
  test_domain_get(d1, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d1, i, ZInterval(0));
  test_domain_get(d1, x, Gauge(0));
  test_domain_to_interval(d1, x, ZInterval(0));
  test_domain_get(d1, y, Gauge(1));
  test_domain_to_interval(d1, y, ZInterval(1));

  GaugeIntervalCongruenceDomain d2 = d1;
  d2.counter_incr(i, ZNumber(1));
  d2.assign(x, VariableExpression(x) + 2);
  d2.assign(y, VariableExpression(y) + 3);
  test_domain_get(d2, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d2, i, ZInterval(1));
  test_domain_get(d2, x, Gauge(2));
  test_domain_to_interval(d2, x, ZInterval(2));
  test_domain_get(d2, y, Gauge(4));
  test_domain_to_interval(d2, y, ZInterval(4));

  GaugeIntervalCongruenceDomain d3 = d1.widening(d2);
  test_domain_get(d3, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d3, i, ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_domain_get(d3, x, Gauge(GaugeBound(2, i)));
  test_domain_to_interval(d3, x, ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_domain_get(d3, y, Gauge(GaugeBound(1) + GaugeBound(3, i)));
  test_domain_to_interval(d3, y, ZInterval(ZBound(1), ZBound::plus_infinity()));

  GaugeIntervalCongruenceDomain d4 = d3;
  d4.assign(y, 3 * VariableExpression(i) + 2);

  GaugeIntervalCongruenceDomain d5 = d3.widening(d4);
  test_domain_get(d5, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d5, i, ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_domain_get(d5, x, Gauge(GaugeBound(2, i)));
  test_domain_to_interval(d5, x, ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_domain_get(d5,
                  y,
                  Gauge(GaugeBound(1) + GaugeBound(3, i),
                        GaugeBound::plus_infinity()));
  test_domain_to_interval(d5, y, ZInterval(ZBound(1), ZBound::plus_infinity()));
}

BOOST_AUTO_TEST_CASE(gauge_interval_congruence_domain_add) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable i(vfac.get("i"));

  auto d1 = GaugeIntervalCongruenceDomain::top();
  d1.counter_init(i, ZNumber(0));
  d1.assign(x, 0);
  d1.assign(y, 1);
  d1.add(VariableExpression(i) <= 10);
  test_domain_get(d1, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d1, i, ZInterval(0));
  test_domain_get(d1, x, Gauge(0));
  test_domain_get(d1, y, Gauge(1));

  GaugeIntervalCongruenceDomain d2 = d1;
  d2.counter_incr(i, ZNumber(1));
  d2.assign(x, VariableExpression(x) + 2);
  d2.assign(y, VariableExpression(y) + 3);
  d2.add(VariableExpression(i) <= 10);
  test_domain_get(d2, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d2, i, ZInterval(1));
  test_domain_get(d2, x, Gauge(2));
  test_domain_get(d2, y, Gauge(4));

  GaugeIntervalCongruenceDomain d3 = d1.widening(d2);
  d3.add(VariableExpression(i) <= 10);
  test_domain_get(d3, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d3, i, ZInterval(ZBound(0), ZBound(10)));
  test_domain_get(d3, x, Gauge(GaugeBound(2, i)));
  test_domain_to_interval(d3, x, ZInterval(ZBound(0), ZBound(20)));
  test_domain_get(d3, y, Gauge(GaugeBound(1) + GaugeBound(3, i)));
  test_domain_to_interval(d3, y, ZInterval(ZBound(1), ZBound(31)));

  GaugeIntervalCongruenceDomain d4 = d3;
  d4.add(VariableExpression(i) >= 11);
  test_domain_get(d4, i, Gauge::bottom());

  GaugeIntervalCongruenceDomain d5 = d3;
  d5.add(VariableExpression(i) == 10);
  test_domain_get(d5, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d5, i, ZInterval(10));
  test_domain_get(d5, x, Gauge(GaugeBound(2, i)));
  test_domain_to_interval(d5, x, ZInterval(20));
  test_domain_get(d5, y, Gauge(GaugeBound(1) + GaugeBound(3, i)));
  test_domain_to_interval(d5, y, ZInterval(31));

  GaugeIntervalCongruenceDomain d6 = d3;
  d6.add(VariableExpression(i) != 10);
  test_domain_get(d6, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d6, i, ZInterval(ZBound(0), ZBound(9)));
  test_domain_get(d6, x, Gauge(GaugeBound(2, i)));
  test_domain_to_interval(d6, x, ZInterval(ZBound(0), ZBound(18)));
  test_domain_get(d6, y, Gauge(GaugeBound(1) + GaugeBound(3, i)));
  test_domain_to_interval(d6, y, ZInterval(ZBound(1), ZBound(28)));
}

BOOST_AUTO_TEST_CASE(gauge_interval_congruence_domain_forget) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable i(vfac.get("i"));

  auto d1 = GaugeIntervalCongruenceDomain::top();
  d1.counter_init(i, ZNumber(0));
  d1.assign(x, 0);
  d1.assign(y, 1);
  d1.add(VariableExpression(i) <= 10);

  GaugeIntervalCongruenceDomain d2 = d1;
  d2.counter_incr(i, ZNumber(1));
  d2.assign(x, VariableExpression(x) + 2);
  d2.assign(y, VariableExpression(y) + 3);
  d2.add(VariableExpression(i) <= 10);

  GaugeIntervalCongruenceDomain d3 = d1.widening(d2);
  d3.add(VariableExpression(i) <= 10);

  d1.forget(i);
  test_domain_get(d1, i, Gauge::top());
  test_domain_to_interval(d1, i, ZInterval::top());
  test_domain_get(d1, x, Gauge(0));
  test_domain_get(d1, y, Gauge(1));

  d3.forget(y);
  test_domain_get(d3, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d3, i, ZInterval(ZBound(0), ZBound(10)));
  test_domain_get(d3, x, Gauge(GaugeBound(2, i)));
  test_domain_to_interval(d3, x, ZInterval(ZBound(0), ZBound(20)));
  test_domain_get(d3, y, Gauge::top());
  test_domain_to_interval(d3, y, ZInterval::top());

  d3.forget(i);
  test_domain_get(d3, i, Gauge::top());
  test_domain_to_interval(d3, i, ZInterval::top());
  test_domain_get(d3, x, Gauge(GaugeBound(0), GaugeBound(20)));
  test_domain_to_interval(d3, x, ZInterval(ZBound(0), ZBound(20)));
}

BOOST_AUTO_TEST_CASE(gauge_interval_congruence_domain_set) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto d1 = GaugeIntervalCongruenceDomain::top();
  d1.assign(x, 0);
  d1.assign(y, 1);

  d1.set(x, ZInterval::top());
  test_domain_get(d1, x, Gauge::top());
  test_domain_get(d1, y, Gauge(1));

  d1.set(x, ZInterval(ZBound(1), ZBound(2)));
  test_domain_get(d1, x, Gauge(GaugeBound(1), GaugeBound(2)));
  test_domain_get(d1, y, Gauge(1));

  d1.set(x, ZInterval(ZBound(1), ZBound::plus_infinity()));
  test_domain_get(d1, x, Gauge(GaugeBound(1), GaugeBound::plus_infinity()));
  test_domain_get(d1, y, Gauge(1));

  d1.set(x, ZInterval(ZBound::minus_infinity(), ZBound(1)));
  test_domain_get(d1, x, Gauge(GaugeBound::minus_infinity(), GaugeBound(1)));
  test_domain_get(d1, y, Gauge(1));
}

BOOST_AUTO_TEST_CASE(gauge_interval_congruence_domain_apply_var_var_var) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));
  Variable w(vfac.get("w"));
  Variable i(vfac.get("i"));
  Variable k(vfac.get("k"));
  Variable zero(vfac.get("zero"));
  Variable one(vfac.get("one"));
  Variable two(vfac.get("two"));

  auto d1 = GaugeIntervalCongruenceDomain::top();
  d1.assign(zero, 0);
  d1.assign(one, 1);
  d1.assign(two, 2);
  d1.counter_init(i, ZNumber(0));
  d1.assign(x, 3);
  d1.assign(y, 2);
  d1.assign(z, 2);
  d1.assign(w, 16);
  test_domain_get(d1, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d1, i, ZInterval(0));
  test_domain_get(d1, x, Gauge(3));
  test_domain_to_interval(d1, x, ZInterval(3));
  test_domain_get(d1, y, Gauge(2));
  test_domain_to_interval(d1, y, ZInterval(2));
  test_domain_get(d1, z, Gauge(2));
  test_domain_to_interval(d1, z, ZInterval(2));
  test_domain_get(d1, w, Gauge(16));
  test_domain_to_interval(d1, w, ZInterval(16));

  GaugeIntervalCongruenceDomain d2 = d1;
  d2.apply(BinaryOperator::Add, x, x, two);
  d2.apply(BinaryOperator::Sub, y, y, one);
  d2.apply(BinaryOperator::Mul, z, z, two);
  d2.apply(BinaryOperator::Div, w, w, two);
  d2.apply(BinaryOperator::Add, k, i, one);
  test_domain_get(d2, x, Gauge(5));
  test_domain_to_interval(d2, x, ZInterval(5));
  test_domain_get(d2, y, Gauge(1));
  test_domain_to_interval(d2, y, ZInterval(1));
  test_domain_get(d2, z, Gauge(4));
  test_domain_to_interval(d2, z, ZInterval(4));
  test_domain_get(d2, w, Gauge(8));
  test_domain_to_interval(d2, w, ZInterval(8));
  test_domain_get(d2, k, Gauge(GaugeBound(1) + GaugeBound(i)));
  test_domain_to_interval(d2, k, ZInterval(1));

  d2.counter_incr(i, ZNumber(1));
  test_domain_get(d2, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d2, i, ZInterval(1));
  test_domain_get(d2, k, Gauge(GaugeBound(i)));
  test_domain_to_interval(d2, k, ZInterval(1));

  GaugeIntervalCongruenceDomain d3 = d1.widening(d2);
  test_domain_get(d3, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d3, i, ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_domain_get(d3, k, Gauge::top());
  test_domain_to_interval(d3, k, ZInterval::top());
  test_domain_get(d3, x, Gauge(GaugeBound(3) + GaugeBound(2, i)));
  test_domain_to_interval(d3, x, ZInterval(ZBound(3), ZBound::plus_infinity()));
  test_domain_get(d3, y, Gauge(GaugeBound(2) + GaugeBound(-1, i)));
  test_domain_to_interval(d3,
                          y,
                          ZInterval(ZBound::minus_infinity(), ZBound(2)));
  test_domain_get(d3, z, Gauge(GaugeBound(2) + GaugeBound(2, i)));
  test_domain_to_interval(d3, z, ZInterval(ZBound(2), ZBound::plus_infinity()));
  test_domain_get(d3, w, Gauge(GaugeBound(16) + GaugeBound(-8, i)));
  test_domain_to_interval(d3,
                          w,
                          ZInterval(ZBound::minus_infinity(), ZBound(16)));

  GaugeIntervalCongruenceDomain d4 = d3;
  d4.apply(BinaryOperator::Add, x, x, two);
  d4.apply(BinaryOperator::Sub, y, y, one);
  d4.apply(BinaryOperator::Mul, z, z, two);
  d4.apply(BinaryOperator::Div, w, w, two);
  d4.apply(BinaryOperator::Add, k, i, one);
  test_domain_get(d4, k, Gauge(GaugeBound(1) + GaugeBound(i)));
  test_domain_to_interval(d4, k, ZInterval(ZBound(1), ZBound::plus_infinity()));
  test_domain_get(d4, x, Gauge(GaugeBound(5) + GaugeBound(2, i)));
  test_domain_to_interval(d4, x, ZInterval(ZBound(5), ZBound::plus_infinity()));
  test_domain_get(d4, y, Gauge(GaugeBound(1) + GaugeBound(-1, i)));
  test_domain_to_interval(d4,
                          y,
                          ZInterval(ZBound::minus_infinity(), ZBound(1)));
  test_domain_get(d4, z, Gauge(GaugeBound(4) + GaugeBound(4, i)));
  test_domain_to_interval(d4, z, ZInterval(ZBound(4), ZBound::plus_infinity()));
  test_domain_get(d4, w, Gauge(GaugeBound::minus_infinity(), GaugeBound(8)));
  test_domain_to_interval(d4,
                          w,
                          ZInterval(ZBound::minus_infinity(), ZBound(8)));

  d4.counter_incr(i, ZNumber(1));
  test_domain_get(d4, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d4, i, ZInterval(ZBound(1), ZBound::plus_infinity()));
  test_domain_get(d4, k, Gauge(GaugeBound(i)));
  test_domain_to_interval(d4, k, ZInterval(ZBound(1), ZBound::plus_infinity()));
  test_domain_get(d4, x, Gauge(GaugeBound(3) + GaugeBound(2, i)));
  test_domain_to_interval(d4, x, ZInterval(ZBound(5), ZBound::plus_infinity()));
  test_domain_get(d4, y, Gauge(GaugeBound(2) + GaugeBound(-1, i)));
  test_domain_to_interval(d4,
                          y,
                          ZInterval(ZBound::minus_infinity(), ZBound(1)));
  test_domain_get(d4, z, Gauge(GaugeBound(0) + GaugeBound(4, i)));
  test_domain_to_interval(d4, z, ZInterval(ZBound(4), ZBound::plus_infinity()));
  test_domain_get(d4, w, Gauge(GaugeBound::minus_infinity(), GaugeBound(8)));
  test_domain_to_interval(d4,
                          w,
                          ZInterval(ZBound::minus_infinity(), ZBound(8)));

  GaugeIntervalCongruenceDomain d5 = d3.join(d4);
  test_domain_get(d5, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d5, i, ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_domain_get(d5, k, Gauge::top());
  test_domain_to_interval(d5, k, ZInterval::top());
  test_domain_get(d5, x, Gauge(GaugeBound(3) + GaugeBound(2, i)));
  test_domain_to_interval(d5, x, ZInterval(ZBound(3), ZBound::plus_infinity()));
  test_domain_get(d5, y, Gauge(GaugeBound(2) + GaugeBound(-1, i)));
  test_domain_to_interval(d5,
                          y,
                          ZInterval(ZBound::minus_infinity(), ZBound(2)));
  test_domain_get(d5,
                  z,
                  Gauge(GaugeBound(2, i), GaugeBound(2) + GaugeBound(4, i)));
  test_domain_to_interval(d5, z, ZInterval(ZBound(2), ZBound::plus_infinity()));
  test_domain_get(d5, w, Gauge(GaugeBound::minus_infinity(), GaugeBound(16)));
  test_domain_to_interval(d5,
                          w,
                          ZInterval(ZBound::minus_infinity(), ZBound(16)));
}

BOOST_AUTO_TEST_CASE(gauge_interval_congruence_domain_apply_var_var_num) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));
  Variable w(vfac.get("w"));
  Variable i(vfac.get("i"));
  Variable k(vfac.get("k"));

  auto d1 = GaugeIntervalCongruenceDomain::top();
  d1.counter_init(i, ZNumber(0));
  d1.assign(x, 3);
  d1.assign(y, 2);
  d1.assign(z, 2);
  d1.assign(w, 16);
  test_domain_get(d1, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d1, i, ZInterval(0));
  test_domain_get(d1, x, Gauge(3));
  test_domain_to_interval(d1, x, ZInterval(3));
  test_domain_get(d1, y, Gauge(2));
  test_domain_to_interval(d1, y, ZInterval(2));
  test_domain_get(d1, z, Gauge(2));
  test_domain_to_interval(d1, z, ZInterval(2));
  test_domain_get(d1, w, Gauge(16));
  test_domain_to_interval(d1, w, ZInterval(16));

  GaugeIntervalCongruenceDomain d2 = d1;
  d2.apply(BinaryOperator::Add, x, x, ZNumber(2));
  d2.apply(BinaryOperator::Sub, y, y, ZNumber(1));
  d2.apply(BinaryOperator::Mul, z, z, ZNumber(2));
  d2.apply(BinaryOperator::Div, w, w, ZNumber(2));
  d2.apply(BinaryOperator::Add, k, i, ZNumber(1));
  test_domain_get(d2, x, Gauge(5));
  test_domain_to_interval(d2, x, ZInterval(5));
  test_domain_get(d2, y, Gauge(1));
  test_domain_to_interval(d2, y, ZInterval(1));
  test_domain_get(d2, z, Gauge(4));
  test_domain_to_interval(d2, z, ZInterval(4));
  test_domain_get(d2, w, Gauge(8));
  test_domain_to_interval(d2, w, ZInterval(8));
  test_domain_get(d2, k, Gauge(GaugeBound(1) + GaugeBound(i)));
  test_domain_to_interval(d2, k, ZInterval(1));

  d2.counter_incr(i, ZNumber(1));
  test_domain_get(d2, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d2, i, ZInterval(1));
  test_domain_get(d2, k, Gauge(GaugeBound(i)));
  test_domain_to_interval(d2, k, ZInterval(1));

  GaugeIntervalCongruenceDomain d3 = d1.widening(d2);
  test_domain_get(d3, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d3, i, ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_domain_get(d3, k, Gauge::top());
  test_domain_to_interval(d3, k, ZInterval::top());
  test_domain_get(d3, x, Gauge(GaugeBound(3) + GaugeBound(2, i)));
  test_domain_to_interval(d3, x, ZInterval(ZBound(3), ZBound::plus_infinity()));
  test_domain_get(d3, y, Gauge(GaugeBound(2) + GaugeBound(-1, i)));
  test_domain_to_interval(d3,
                          y,
                          ZInterval(ZBound::minus_infinity(), ZBound(2)));
  test_domain_get(d3, z, Gauge(GaugeBound(2) + GaugeBound(2, i)));
  test_domain_to_interval(d3, z, ZInterval(ZBound(2), ZBound::plus_infinity()));
  test_domain_get(d3, w, Gauge(GaugeBound(16) + GaugeBound(-8, i)));
  test_domain_to_interval(d3,
                          w,
                          ZInterval(ZBound::minus_infinity(), ZBound(16)));

  GaugeIntervalCongruenceDomain d4 = d3;
  d4.apply(BinaryOperator::Add, x, x, ZNumber(2));
  d4.apply(BinaryOperator::Sub, y, y, ZNumber(1));
  d4.apply(BinaryOperator::Mul, z, z, ZNumber(2));
  d4.apply(BinaryOperator::Div, w, w, ZNumber(2));
  d4.apply(BinaryOperator::Add, k, i, ZNumber(1));
  test_domain_get(d4, k, Gauge(GaugeBound(1) + GaugeBound(i)));
  test_domain_to_interval(d4, k, ZInterval(ZBound(1), ZBound::plus_infinity()));
  test_domain_get(d4, x, Gauge(GaugeBound(5) + GaugeBound(2, i)));
  test_domain_to_interval(d4, x, ZInterval(ZBound(5), ZBound::plus_infinity()));
  test_domain_get(d4, y, Gauge(GaugeBound(1) + GaugeBound(-1, i)));
  test_domain_to_interval(d4,
                          y,
                          ZInterval(ZBound::minus_infinity(), ZBound(1)));
  test_domain_get(d4, z, Gauge(GaugeBound(4) + GaugeBound(4, i)));
  test_domain_to_interval(d4, z, ZInterval(ZBound(4), ZBound::plus_infinity()));
  test_domain_get(d4, w, Gauge(GaugeBound::minus_infinity(), GaugeBound(8)));
  test_domain_to_interval(d4,
                          w,
                          ZInterval(ZBound::minus_infinity(), ZBound(8)));

  d4.counter_incr(i, ZNumber(1));
  test_domain_get(d4, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d4, i, ZInterval(ZBound(1), ZBound::plus_infinity()));
  test_domain_get(d4, k, Gauge(GaugeBound(i)));
  test_domain_to_interval(d4, k, ZInterval(ZBound(1), ZBound::plus_infinity()));
  test_domain_get(d4, x, Gauge(GaugeBound(3) + GaugeBound(2, i)));
  test_domain_to_interval(d4, x, ZInterval(ZBound(5), ZBound::plus_infinity()));
  test_domain_get(d4, y, Gauge(GaugeBound(2) + GaugeBound(-1, i)));
  test_domain_to_interval(d4,
                          y,
                          ZInterval(ZBound::minus_infinity(), ZBound(1)));
  test_domain_get(d4, z, Gauge(GaugeBound(0) + GaugeBound(4, i)));
  test_domain_to_interval(d4, z, ZInterval(ZBound(4), ZBound::plus_infinity()));
  test_domain_get(d4, w, Gauge(GaugeBound::minus_infinity(), GaugeBound(8)));
  test_domain_to_interval(d4,
                          w,
                          ZInterval(ZBound::minus_infinity(), ZBound(8)));

  GaugeIntervalCongruenceDomain d5 = d3.join(d4);
  test_domain_get(d5, i, Gauge(GaugeBound(i)));
  test_domain_to_interval(d5, i, ZInterval(ZBound(0), ZBound::plus_infinity()));
  test_domain_get(d5, k, Gauge::top());
  test_domain_to_interval(d5, k, ZInterval::top());
  test_domain_get(d5, x, Gauge(GaugeBound(3) + GaugeBound(2, i)));
  test_domain_to_interval(d5, x, ZInterval(ZBound(3), ZBound::plus_infinity()));
  test_domain_get(d5, y, Gauge(GaugeBound(2) + GaugeBound(-1, i)));
  test_domain_to_interval(d5,
                          y,
                          ZInterval(ZBound::minus_infinity(), ZBound(2)));
  test_domain_get(d5,
                  z,
                  Gauge(GaugeBound(2, i), GaugeBound(2) + GaugeBound(4, i)));
  test_domain_to_interval(d5, z, ZInterval(ZBound(2), ZBound::plus_infinity()));
  test_domain_get(d5, w, Gauge(GaugeBound::minus_infinity(), GaugeBound(16)));
  test_domain_to_interval(d5,
                          w,
                          ZInterval(ZBound::minus_infinity(), ZBound(16)));
}
