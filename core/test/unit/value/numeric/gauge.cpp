/*******************************************************************************
 *
 * Tests for Gauge
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

#define BOOST_TEST_MODULE test_gauge
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/example/variable_factory.hpp>
#include <ikos/core/number/z_number.hpp>
#include <ikos/core/value/numeric/gauge.hpp>

using ZNumber = ikos::core::ZNumber;
using ZBound = ikos::core::ZBound;
using VariableFactory = ikos::core::example::VariableFactory;
using Variable = ikos::core::example::VariableFactory::VariableRef;
using ZConstant = ikos::core::numeric::ZConstant;
using ZInterval = ikos::core::numeric::ZInterval;
using GaugeBound = ikos::core::numeric::GaugeBound< ZNumber, Variable >;
using Gauge = ikos::core::numeric::Gauge< ZNumber, Variable >;

#define test_gauge_bound(g,                                        \
                         is_infinite_v,                            \
                         is_plus_infinity_v,                       \
                         is_minus_infinity_v,                      \
                         num_coeffs_v,                             \
                         is_constant_v,                            \
                         constant_v)                               \
  do {                                                             \
    BOOST_CHECK((g).is_infinite() == (is_infinite_v));             \
    BOOST_CHECK((g).is_plus_infinity() == (is_plus_infinity_v));   \
    BOOST_CHECK((g).is_minus_infinity() == (is_minus_infinity_v)); \
    BOOST_CHECK((g).is_constant() == (is_constant_v));             \
    if ((g).is_finite()) {                                         \
      BOOST_CHECK((g).num_coeffs() == (num_coeffs_v));             \
    }                                                              \
    if ((g).is_constant()) {                                       \
      BOOST_CHECK((g).constant() == (constant_v));                 \
    }                                                              \
  } while (0)

BOOST_AUTO_TEST_CASE(gauge_bound_constructors) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  test_gauge_bound(GaugeBound::plus_infinity(), true, true, false, 0, false, 0);
  test_gauge_bound(GaugeBound::minus_infinity(),
                   true,
                   false,
                   true,
                   0,
                   false,
                   0);
  test_gauge_bound(GaugeBound(0), false, false, false, 0, true, 0);
  test_gauge_bound(GaugeBound(1), false, false, false, 0, true, 1);
  test_gauge_bound(GaugeBound(x), false, false, false, 1, false, 0);
  test_gauge_bound(GaugeBound(2, x), false, false, false, 1, false, 0);

  GaugeBound g(1);
  g = g + GaugeBound(2, x);
  g = g + GaugeBound(3, y);
  test_gauge_bound(g, false, false, false, 2, false, 0);
}

#define test_gauge_bound_coeff(g, v, c) BOOST_CHECK((g).coeff(v) == (c))

BOOST_AUTO_TEST_CASE(gauge_bound_coeff) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  test_gauge_bound_coeff(GaugeBound(0), x, 0);
  test_gauge_bound_coeff(GaugeBound(1), x, 0);
  test_gauge_bound_coeff(GaugeBound(0) + GaugeBound(x), x, 1);
  test_gauge_bound_coeff(GaugeBound(0) + GaugeBound(-1, x), x, -1);
  test_gauge_bound_coeff(GaugeBound(0) + GaugeBound(y), x, 0);
  test_gauge_bound_coeff(GaugeBound(0) + GaugeBound(-1, y), x, 0);
  test_gauge_bound_coeff(GaugeBound(0) + GaugeBound(x) + GaugeBound(y), x, 1);
  test_gauge_bound_coeff(GaugeBound(0) + GaugeBound(-1, x) + GaugeBound(y),
                         x,
                         -1);
}

#define test_gauge_bound_to_zbound(g, b) BOOST_CHECK((g).to_bound() == (b))

BOOST_AUTO_TEST_CASE(gauge_bound_to_bound) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  test_gauge_bound_to_zbound(GaugeBound::minus_infinity(),
                             boost::optional< ZBound >(
                                 ZBound::minus_infinity()));
  test_gauge_bound_to_zbound(GaugeBound::plus_infinity(),
                             boost::optional< ZBound >(
                                 ZBound::plus_infinity()));
  test_gauge_bound_to_zbound(GaugeBound(1),
                             boost::optional< ZBound >(ZBound(1)));
  test_gauge_bound_to_zbound(GaugeBound(42),
                             boost::optional< ZBound >(ZBound(42)));
  test_gauge_bound_to_zbound(GaugeBound(x), boost::none);
  test_gauge_bound_to_zbound(GaugeBound(3) + GaugeBound(2, x), boost::none);
}

#define test_gauge_bound_add(x, y, z) BOOST_CHECK((x) + (y) == (z))

BOOST_AUTO_TEST_CASE(gauge_bound_add) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));

  test_gauge_bound_add(GaugeBound::plus_infinity(),
                       GaugeBound::plus_infinity(),
                       GaugeBound::plus_infinity());
  test_gauge_bound_add(GaugeBound::plus_infinity(),
                       GaugeBound(1),
                       GaugeBound::plus_infinity());
  test_gauge_bound_add(GaugeBound::plus_infinity(),
                       GaugeBound(2, x),
                       GaugeBound::plus_infinity());
  test_gauge_bound_add(GaugeBound::minus_infinity(),
                       GaugeBound::minus_infinity(),
                       GaugeBound::minus_infinity());
  test_gauge_bound_add(GaugeBound::minus_infinity(),
                       GaugeBound(1),
                       GaugeBound::minus_infinity());
  test_gauge_bound_add(GaugeBound::minus_infinity(),
                       GaugeBound(2, x),
                       GaugeBound::minus_infinity());
  test_gauge_bound_add(GaugeBound(1),
                       GaugeBound::plus_infinity(),
                       GaugeBound::plus_infinity());
  test_gauge_bound_add(GaugeBound(1), GaugeBound(2), GaugeBound(3));
  test_gauge_bound_add(GaugeBound(1),
                       GaugeBound(2, x),
                       GaugeBound(1) + GaugeBound(2, x));
  test_gauge_bound_add(GaugeBound(x), GaugeBound(2, x), GaugeBound(3, x));
  test_gauge_bound_add(GaugeBound(x),
                       GaugeBound(2, y),
                       GaugeBound(x) + GaugeBound(2, y));
  test_gauge_bound_add(GaugeBound(x) + GaugeBound(2, y),
                       GaugeBound(3, y) + GaugeBound(4, z),
                       GaugeBound(x) + GaugeBound(5, y) + GaugeBound(4, z));
}

#define test_gauge_bound_sub(x, y, z) BOOST_CHECK((x) - (y) == (z))

BOOST_AUTO_TEST_CASE(gauge_bound_sub) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));

  test_gauge_bound_sub(GaugeBound::plus_infinity(),
                       GaugeBound::minus_infinity(),
                       GaugeBound::plus_infinity());
  test_gauge_bound_sub(GaugeBound::plus_infinity(),
                       GaugeBound(1),
                       GaugeBound::plus_infinity());
  test_gauge_bound_sub(GaugeBound::plus_infinity(),
                       GaugeBound(2, x),
                       GaugeBound::plus_infinity());
  test_gauge_bound_sub(GaugeBound::minus_infinity(),
                       GaugeBound::plus_infinity(),
                       GaugeBound::minus_infinity());
  test_gauge_bound_sub(GaugeBound::minus_infinity(),
                       GaugeBound(1),
                       GaugeBound::minus_infinity());
  test_gauge_bound_sub(GaugeBound::minus_infinity(),
                       GaugeBound(2, x),
                       GaugeBound::minus_infinity());
  test_gauge_bound_sub(GaugeBound(1),
                       GaugeBound::plus_infinity(),
                       GaugeBound::minus_infinity());
  test_gauge_bound_sub(GaugeBound(1), GaugeBound(2), GaugeBound(-1));
  test_gauge_bound_sub(GaugeBound(1),
                       GaugeBound(2, x),
                       GaugeBound(1) + GaugeBound(-2, x));
  test_gauge_bound_sub(GaugeBound(x), GaugeBound(2, x), GaugeBound(-1, x));
  test_gauge_bound_sub(GaugeBound(x),
                       GaugeBound(2, y),
                       GaugeBound(x) + GaugeBound(-2, y));
  test_gauge_bound_sub(GaugeBound(x) + GaugeBound(2, y),
                       GaugeBound(3, y) + GaugeBound(4, z),
                       GaugeBound(x) + GaugeBound(-1, y) + GaugeBound(-4, z));
}

#define test_gauge_bound_neg(x, y) BOOST_CHECK(-(x) == (y))

BOOST_AUTO_TEST_CASE(gauge_bound_neg) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  test_gauge_bound_neg(GaugeBound::plus_infinity(),
                       GaugeBound::minus_infinity());
  test_gauge_bound_neg(GaugeBound::minus_infinity(),
                       GaugeBound::plus_infinity());
  test_gauge_bound_neg(GaugeBound(1), GaugeBound(-1));
  test_gauge_bound_neg(GaugeBound(-2), GaugeBound(2));
  test_gauge_bound_neg(GaugeBound(2, x), GaugeBound(-2, x));
  test_gauge_bound_neg(GaugeBound(2) + GaugeBound(x) + GaugeBound(3, y),
                       GaugeBound(-2) + GaugeBound(-1, x) + GaugeBound(-3, y));
}

#define test_gauge_bound_mul(x, y, z) BOOST_CHECK((x)*ZNumber(y) == (z))

BOOST_AUTO_TEST_CASE(gauge_bound_mul) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  test_gauge_bound_mul(GaugeBound::plus_infinity(),
                       2,
                       GaugeBound::plus_infinity());
  test_gauge_bound_mul(GaugeBound::plus_infinity(),
                       -2,
                       GaugeBound::minus_infinity());
  test_gauge_bound_mul(GaugeBound::plus_infinity(), 0, GaugeBound(0));
  test_gauge_bound_mul(GaugeBound::minus_infinity(),
                       2,
                       GaugeBound::minus_infinity());
  test_gauge_bound_mul(GaugeBound::minus_infinity(),
                       -2,
                       GaugeBound::plus_infinity());
  test_gauge_bound_mul(GaugeBound::minus_infinity(), 0, GaugeBound(0));
  test_gauge_bound_mul(GaugeBound(1), 2, GaugeBound(2));
  test_gauge_bound_mul(GaugeBound(1), -2, GaugeBound(-2));
  test_gauge_bound_mul(GaugeBound(1), 0, GaugeBound(0));
  test_gauge_bound_mul(GaugeBound(2, x), 2, GaugeBound(4, x));
  test_gauge_bound_mul(GaugeBound(2, x), -2, GaugeBound(-4, x));
  test_gauge_bound_mul(GaugeBound(2, x), 0, GaugeBound(0));
  test_gauge_bound_mul(GaugeBound(2) + GaugeBound(x) + GaugeBound(-3, y),
                       2,
                       GaugeBound(4) + GaugeBound(2, x) + GaugeBound(-6, y));
  test_gauge_bound_mul(GaugeBound(2) + GaugeBound(x) + GaugeBound(-3, y),
                       -2,
                       GaugeBound(-4) + GaugeBound(-2, x) + GaugeBound(6, y));
  test_gauge_bound_mul(GaugeBound(2) + GaugeBound(x) + GaugeBound(-3, y),
                       0,
                       GaugeBound(0));
}

#define test_gauge_bound_le(x, y, r) BOOST_CHECK(((x) <= (y)) == (r))

BOOST_AUTO_TEST_CASE(gauge_bound_le) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));

  test_gauge_bound_le(GaugeBound::plus_infinity(),
                      GaugeBound::plus_infinity(),
                      true);
  test_gauge_bound_le(GaugeBound::plus_infinity(),
                      GaugeBound::minus_infinity(),
                      false);
  test_gauge_bound_le(GaugeBound::plus_infinity(), GaugeBound(1), false);
  test_gauge_bound_le(GaugeBound::plus_infinity(), GaugeBound(2, x), false);
  test_gauge_bound_le(GaugeBound::minus_infinity(),
                      GaugeBound::plus_infinity(),
                      true);
  test_gauge_bound_le(GaugeBound::minus_infinity(),
                      GaugeBound::minus_infinity(),
                      true);
  test_gauge_bound_le(GaugeBound::minus_infinity(), GaugeBound(1), true);
  test_gauge_bound_le(GaugeBound::minus_infinity(), GaugeBound(2, x), true);
  test_gauge_bound_le(GaugeBound(1), GaugeBound::plus_infinity(), true);
  test_gauge_bound_le(GaugeBound(1), GaugeBound(2), true);
  test_gauge_bound_le(GaugeBound(1), GaugeBound(0), false);
  test_gauge_bound_le(GaugeBound(1), GaugeBound(2, x), false);
  test_gauge_bound_le(GaugeBound(1), GaugeBound(2) + GaugeBound(2, x), true);
  test_gauge_bound_le(GaugeBound(x), GaugeBound(2, x), true);
  test_gauge_bound_le(GaugeBound(x), GaugeBound(2, y), false);
  test_gauge_bound_le(GaugeBound(x) + GaugeBound(2, y),
                      GaugeBound(3, y) + GaugeBound(4, z),
                      false);
}

#define test_gauge_bound_ge(x, y, r) BOOST_CHECK(((x) >= (y)) == (r))

BOOST_AUTO_TEST_CASE(gauge_bound_ge) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));

  test_gauge_bound_ge(GaugeBound::plus_infinity(),
                      GaugeBound::plus_infinity(),
                      true);
  test_gauge_bound_ge(GaugeBound::plus_infinity(),
                      GaugeBound::minus_infinity(),
                      true);
  test_gauge_bound_ge(GaugeBound::plus_infinity(), GaugeBound(1), true);
  test_gauge_bound_ge(GaugeBound::plus_infinity(), GaugeBound(2, x), true);
  test_gauge_bound_ge(GaugeBound::minus_infinity(),
                      GaugeBound::plus_infinity(),
                      false);
  test_gauge_bound_ge(GaugeBound::minus_infinity(),
                      GaugeBound::minus_infinity(),
                      true);
  test_gauge_bound_ge(GaugeBound::minus_infinity(), GaugeBound(1), false);
  test_gauge_bound_ge(GaugeBound::minus_infinity(), GaugeBound(2, x), false);
  test_gauge_bound_ge(GaugeBound(1), GaugeBound::plus_infinity(), false);
  test_gauge_bound_ge(GaugeBound(1), GaugeBound(2), false);
  test_gauge_bound_ge(GaugeBound(1), GaugeBound(0), true);
  test_gauge_bound_ge(GaugeBound(1), GaugeBound(2, x), false);
  test_gauge_bound_ge(GaugeBound(2) + GaugeBound(2, x), GaugeBound(1), true);
  test_gauge_bound_ge(GaugeBound(3, x), GaugeBound(2, x), true);
  test_gauge_bound_ge(GaugeBound(x), GaugeBound(2, y), false);
  test_gauge_bound_ge(GaugeBound(x) + GaugeBound(2, y),
                      GaugeBound(3, y) + GaugeBound(4, z),
                      false);
}

#define test_gauge_bound_min(x, y, z) BOOST_CHECK(min((x), (y)) == (z))

BOOST_AUTO_TEST_CASE(gauge_bound_min) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));

  test_gauge_bound_min(GaugeBound::plus_infinity(),
                       GaugeBound::minus_infinity(),
                       GaugeBound::minus_infinity());
  test_gauge_bound_min(GaugeBound::plus_infinity(),
                       GaugeBound(1),
                       GaugeBound(1));
  test_gauge_bound_min(GaugeBound::plus_infinity(),
                       GaugeBound(2, x),
                       GaugeBound(2, x));
  test_gauge_bound_min(GaugeBound::minus_infinity(),
                       GaugeBound::plus_infinity(),
                       GaugeBound::minus_infinity());
  test_gauge_bound_min(GaugeBound::minus_infinity(),
                       GaugeBound(1),
                       GaugeBound::minus_infinity());
  test_gauge_bound_min(GaugeBound::minus_infinity(),
                       GaugeBound(2, x),
                       GaugeBound::minus_infinity());
  test_gauge_bound_min(GaugeBound(1),
                       GaugeBound::plus_infinity(),
                       GaugeBound(1));
  test_gauge_bound_min(GaugeBound(1), GaugeBound(2), GaugeBound(1));
  test_gauge_bound_min(GaugeBound(1), GaugeBound(2, x), GaugeBound(0));
  test_gauge_bound_min(GaugeBound(x), GaugeBound(2, x), GaugeBound(x));
  test_gauge_bound_min(GaugeBound(x), GaugeBound(2, y), GaugeBound(0));
  test_gauge_bound_min(GaugeBound(x) + GaugeBound(2, y),
                       GaugeBound(3, y) + GaugeBound(4, z),
                       GaugeBound(2, y));
  test_gauge_bound_min(GaugeBound(4) + GaugeBound(x) + GaugeBound(4, y),
                       GaugeBound(2) + GaugeBound(3, y) + GaugeBound(-4, z),
                       GaugeBound(2) + GaugeBound(3, y) + GaugeBound(-4, z));
}

#define test_gauge_bound_max(x, y, z) BOOST_CHECK(max((x), (y)) == (z))

BOOST_AUTO_TEST_CASE(gauge_bound_max) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));

  test_gauge_bound_max(GaugeBound::plus_infinity(),
                       GaugeBound::minus_infinity(),
                       GaugeBound::plus_infinity());
  test_gauge_bound_max(GaugeBound::plus_infinity(),
                       GaugeBound(1),
                       GaugeBound::plus_infinity());
  test_gauge_bound_max(GaugeBound::plus_infinity(),
                       GaugeBound(2, x),
                       GaugeBound::plus_infinity());
  test_gauge_bound_max(GaugeBound::minus_infinity(),
                       GaugeBound::plus_infinity(),
                       GaugeBound::plus_infinity());
  test_gauge_bound_max(GaugeBound::minus_infinity(),
                       GaugeBound(1),
                       GaugeBound(1));
  test_gauge_bound_max(GaugeBound::minus_infinity(),
                       GaugeBound(2, x),
                       GaugeBound(2, x));
  test_gauge_bound_max(GaugeBound(1),
                       GaugeBound::plus_infinity(),
                       GaugeBound::plus_infinity());
  test_gauge_bound_max(GaugeBound(1), GaugeBound(2), GaugeBound(2));
  test_gauge_bound_max(GaugeBound(1),
                       GaugeBound(2, x),
                       GaugeBound(1) + GaugeBound(2, x));
  test_gauge_bound_max(GaugeBound(x), GaugeBound(2, x), GaugeBound(2, x));
  test_gauge_bound_max(GaugeBound(x),
                       GaugeBound(2, y),
                       GaugeBound(x) + GaugeBound(2, y));
  test_gauge_bound_max(GaugeBound(x) + GaugeBound(2, y),
                       GaugeBound(3, y) + GaugeBound(4, z),
                       GaugeBound(x) + GaugeBound(3, y) + GaugeBound(4, z));
  test_gauge_bound_max(GaugeBound(2) + GaugeBound(x) + GaugeBound(4, y),
                       GaugeBound(4) + GaugeBound(3, y) + GaugeBound(-4, z),
                       GaugeBound(4) + GaugeBound(x) + GaugeBound(4, y));
}

#define test_gauge(g, is_bottom_v, is_top_v, is_singleton_v, singleton_v) \
  do {                                                                    \
    BOOST_CHECK((g).is_bottom() == (is_bottom_v));                        \
    BOOST_CHECK((g).is_top() == (is_top_v));                              \
    BOOST_CHECK(!!((g).singleton()) == (is_singleton_v));                 \
    if ((g).singleton()) {                                                \
      BOOST_CHECK(*(g).singleton() == GaugeBound(singleton_v));           \
    }                                                                     \
  } while (0)

BOOST_AUTO_TEST_CASE(gauge_constructors) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  test_gauge(Gauge::top(), false, true, false, 0);
  test_gauge(Gauge::bottom(), true, false, false, 0);
  test_gauge(Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
             false,
             false,
             false,
             0);
  test_gauge(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
             false,
             false,
             false,
             0);
  test_gauge(Gauge(GaugeBound::minus_infinity(), GaugeBound::plus_infinity()),
             false,
             true,
             false,
             0);
  test_gauge(Gauge(0), false, false, true, 0);
  test_gauge(Gauge(GaugeBound(x)), false, false, true, x);
  test_gauge(Gauge(GaugeBound(0), GaugeBound(x)), false, false, false, 0);
}

#define test_gauge_interval(g, i) BOOST_CHECK((g).interval() == (i))

BOOST_AUTO_TEST_CASE(gauge_interval) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  test_gauge_interval(Gauge::top(),
                      boost::optional< ZInterval >(ZInterval::top()));
  test_gauge_interval(Gauge::bottom(),
                      boost::optional< ZInterval >(ZInterval::bottom()));
  test_gauge_interval(Gauge(GaugeBound(1), GaugeBound(2)),
                      boost::optional< ZInterval >(
                          ZInterval(ZBound(1), ZBound(2))));
  test_gauge_interval(Gauge(GaugeBound::minus_infinity(), GaugeBound(2)),
                      boost::optional< ZInterval >(
                          ZInterval(ZBound::minus_infinity(), ZBound(2))));
  test_gauge_interval(Gauge(GaugeBound(1), GaugeBound::plus_infinity()),
                      boost::optional< ZInterval >(
                          ZInterval(ZBound(1), ZBound::plus_infinity())));
  test_gauge_interval(Gauge(GaugeBound(0), GaugeBound(2, x)), boost::none);
  test_gauge_interval(Gauge(GaugeBound(-2, x), GaugeBound(2, x)), boost::none);
}

#define test_gauge_le(x, y, r) BOOST_CHECK((x).leq(y) == (r))

BOOST_AUTO_TEST_CASE(gauge_leq) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  test_gauge_le(Gauge::top(), Gauge::bottom(), false);
  test_gauge_le(Gauge::top(), Gauge::top(), true);
  test_gauge_le(Gauge::top(), Gauge(GaugeBound(0), GaugeBound(1)), false);
  test_gauge_le(Gauge::top(), Gauge(GaugeBound(0), GaugeBound(2, x)), false);
  test_gauge_le(Gauge::top(),
                Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                false);
  test_gauge_le(Gauge::top(),
                Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                false);
  test_gauge_le(Gauge::bottom(), Gauge::bottom(), true);
  test_gauge_le(Gauge::bottom(), Gauge::top(), true);
  test_gauge_le(Gauge::bottom(), Gauge(GaugeBound(0), GaugeBound(1)), true);
  test_gauge_le(Gauge::bottom(), Gauge(GaugeBound(0), GaugeBound(2, x)), true);
  test_gauge_le(Gauge::bottom(),
                Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                true);
  test_gauge_le(Gauge::bottom(),
                Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                true);
  test_gauge_le(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                Gauge::bottom(),
                false);
  test_gauge_le(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                Gauge::top(),
                true);
  test_gauge_le(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                Gauge(GaugeBound(0), GaugeBound(1)),
                false);
  test_gauge_le(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                true);
  test_gauge_le(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                false);
  test_gauge_le(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                Gauge(GaugeBound::minus_infinity(), GaugeBound(2, x)),
                false);
  test_gauge_le(Gauge(GaugeBound(0), GaugeBound(1)),
                Gauge(GaugeBound(-1), GaugeBound(2)),
                true);
  test_gauge_le(Gauge(GaugeBound(0), GaugeBound(1)),
                Gauge(GaugeBound(-1), GaugeBound(2) + GaugeBound(2, x)),
                true);
  test_gauge_le(Gauge(GaugeBound(-2), GaugeBound(1)),
                Gauge(GaugeBound(-1), GaugeBound(2)),
                false);
  test_gauge_le(Gauge(GaugeBound(0), GaugeBound(3)),
                Gauge(GaugeBound(-1), GaugeBound(2)),
                false);
  test_gauge_le(Gauge(GaugeBound(-1) + GaugeBound(2, x),
                      GaugeBound(3) + GaugeBound(3, x)),
                Gauge(GaugeBound(-1),
                      GaugeBound(4) + GaugeBound(4, x) + GaugeBound(y)),
                true);
}

#define test_gauge_eq(x, y, r) BOOST_CHECK((x).equals(y) == (r))

BOOST_AUTO_TEST_CASE(gauge_equals) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  test_gauge_eq(Gauge::top(), Gauge::bottom(), false);
  test_gauge_eq(Gauge::top(), Gauge::top(), true);
  test_gauge_eq(Gauge::top(), Gauge(GaugeBound(0), GaugeBound(1)), false);
  test_gauge_eq(Gauge::top(), Gauge(GaugeBound(0), GaugeBound(2, x)), false);
  test_gauge_eq(Gauge::top(),
                Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                false);
  test_gauge_eq(Gauge::top(),
                Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                false);
  test_gauge_eq(Gauge::bottom(), Gauge::bottom(), true);
  test_gauge_eq(Gauge::bottom(), Gauge::top(), false);
  test_gauge_eq(Gauge::bottom(), Gauge(GaugeBound(0), GaugeBound(1)), false);
  test_gauge_eq(Gauge::bottom(), Gauge(GaugeBound(0), GaugeBound(2, x)), false);
  test_gauge_eq(Gauge::bottom(),
                Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                false);
  test_gauge_eq(Gauge::bottom(),
                Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                false);
  test_gauge_eq(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                Gauge::bottom(),
                false);
  test_gauge_eq(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                Gauge::top(),
                false);
  test_gauge_eq(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                Gauge(GaugeBound(0), GaugeBound(1)),
                false);
  test_gauge_eq(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                true);
  test_gauge_eq(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                false);
  test_gauge_eq(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                Gauge(GaugeBound::minus_infinity(), GaugeBound(2, x)),
                false);
  test_gauge_eq(Gauge(GaugeBound(0), GaugeBound(1)),
                Gauge(GaugeBound(-1), GaugeBound(2)),
                false);
  test_gauge_eq(Gauge(GaugeBound(0), GaugeBound(1)),
                Gauge(GaugeBound(-1), GaugeBound(2) + GaugeBound(2, x)),
                false);
  test_gauge_eq(Gauge(GaugeBound(-2), GaugeBound(1)),
                Gauge(GaugeBound(-1), GaugeBound(2)),
                false);
  test_gauge_eq(Gauge(GaugeBound(0), GaugeBound(3)),
                Gauge(GaugeBound(-1), GaugeBound(2)),
                false);
  test_gauge_eq(Gauge(GaugeBound(-1) + GaugeBound(2, x),
                      GaugeBound(3) + GaugeBound(3, x)),
                Gauge(GaugeBound(-1),
                      GaugeBound(4) + GaugeBound(4, x) + GaugeBound(y)),
                false);
}

#define test_gauge_join(x, y, z) BOOST_CHECK((x).join(y) == (z))

BOOST_AUTO_TEST_CASE(gauge_join) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  test_gauge_join(Gauge::top(), Gauge::bottom(), Gauge::top());
  test_gauge_join(Gauge::top(), Gauge::top(), Gauge::top());
  test_gauge_join(Gauge::top(),
                  Gauge(GaugeBound(0), GaugeBound(1)),
                  Gauge::top());
  test_gauge_join(Gauge::top(),
                  Gauge(GaugeBound(0), GaugeBound(2, x)),
                  Gauge::top());
  test_gauge_join(Gauge::top(),
                  Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge::top());
  test_gauge_join(Gauge::top(),
                  Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                  Gauge::top());
  test_gauge_join(Gauge::bottom(), Gauge::bottom(), Gauge::bottom());
  test_gauge_join(Gauge::bottom(), Gauge::top(), Gauge::top());
  test_gauge_join(Gauge::bottom(),
                  Gauge(GaugeBound(0), GaugeBound(1)),
                  Gauge(GaugeBound(0), GaugeBound(1)));
  test_gauge_join(Gauge::bottom(),
                  Gauge(GaugeBound(0), GaugeBound(2, x)),
                  Gauge(GaugeBound(0), GaugeBound(2, x)));
  test_gauge_join(Gauge::bottom(),
                  Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  test_gauge_join(Gauge::bottom(),
                  Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                  Gauge(GaugeBound::minus_infinity(), GaugeBound(0)));
  test_gauge_join(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge::bottom(),
                  Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  test_gauge_join(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge::top(),
                  Gauge::top());
  test_gauge_join(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge(GaugeBound(0), GaugeBound(1)),
                  Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  test_gauge_join(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  test_gauge_join(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                  Gauge::top());
  test_gauge_join(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge(GaugeBound::minus_infinity(), GaugeBound(2, x)),
                  Gauge::top());
  test_gauge_join(Gauge(GaugeBound(0), GaugeBound(1)),
                  Gauge(GaugeBound(-1), GaugeBound(2)),
                  Gauge(GaugeBound(-1), GaugeBound(2)));
  test_gauge_join(Gauge(GaugeBound(0), GaugeBound(1)),
                  Gauge(GaugeBound(-1), GaugeBound(2) + GaugeBound(2, x)),
                  Gauge(GaugeBound(-1), GaugeBound(2) + GaugeBound(2, x)));
  test_gauge_join(Gauge(GaugeBound(-2), GaugeBound(1)),
                  Gauge(GaugeBound(-1), GaugeBound(2)),
                  Gauge(GaugeBound(-2), GaugeBound(2)));
  test_gauge_join(Gauge(GaugeBound(0), GaugeBound(3)),
                  Gauge(GaugeBound(-1), GaugeBound(2)),
                  Gauge(GaugeBound(-1), GaugeBound(3)));
  test_gauge_join(Gauge(GaugeBound(-1) + GaugeBound(2, x),
                        GaugeBound(3) + GaugeBound(3, x)),
                  Gauge(GaugeBound(-1),
                        GaugeBound(4) + GaugeBound(4, x) + GaugeBound(y)),
                  Gauge(GaugeBound(-1),
                        GaugeBound(4) + GaugeBound(4, x) + GaugeBound(y)));
}

#define test_gauge_meet(x, y, z) BOOST_CHECK((x).meet(y) == (z))

BOOST_AUTO_TEST_CASE(gauge_meet) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  test_gauge_meet(Gauge::top(), Gauge::bottom(), Gauge::bottom());
  test_gauge_meet(Gauge::top(), Gauge::top(), Gauge::top());
  test_gauge_meet(Gauge::top(),
                  Gauge(GaugeBound(0), GaugeBound(1)),
                  Gauge(GaugeBound(0), GaugeBound(1)));
  test_gauge_meet(Gauge::top(),
                  Gauge(GaugeBound(0), GaugeBound(2, x)),
                  Gauge(GaugeBound(0), GaugeBound(2, x)));
  test_gauge_meet(Gauge::top(),
                  Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  test_gauge_meet(Gauge::top(),
                  Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                  Gauge(GaugeBound::minus_infinity(), GaugeBound(0)));
  test_gauge_meet(Gauge::bottom(), Gauge::bottom(), Gauge::bottom());
  test_gauge_meet(Gauge::bottom(), Gauge::top(), Gauge::bottom());
  test_gauge_meet(Gauge::bottom(),
                  Gauge(GaugeBound(0), GaugeBound(1)),
                  Gauge::bottom());
  test_gauge_meet(Gauge::bottom(),
                  Gauge(GaugeBound(0), GaugeBound(2, x)),
                  Gauge::bottom());
  test_gauge_meet(Gauge::bottom(),
                  Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge::bottom());
  test_gauge_meet(Gauge::bottom(),
                  Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                  Gauge::bottom());
  test_gauge_meet(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge::bottom(),
                  Gauge::bottom());
  test_gauge_meet(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge::top(),
                  Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  test_gauge_meet(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge(GaugeBound(0), GaugeBound(1)),
                  Gauge(GaugeBound(0), GaugeBound(1)));
  test_gauge_meet(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  test_gauge_meet(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                  Gauge(GaugeBound(0), GaugeBound(0)));
  test_gauge_meet(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                  Gauge(GaugeBound::minus_infinity(), GaugeBound(2, x)),
                  Gauge(GaugeBound(0), GaugeBound(2, x)));
  test_gauge_meet(Gauge(GaugeBound(0), GaugeBound(1)),
                  Gauge(GaugeBound(-1), GaugeBound(2)),
                  Gauge(GaugeBound(0), GaugeBound(1)));
  test_gauge_meet(Gauge(GaugeBound(0), GaugeBound(1)),
                  Gauge(GaugeBound(-1), GaugeBound(2) + GaugeBound(2, x)),
                  Gauge(GaugeBound(0), GaugeBound(1)));
  test_gauge_meet(Gauge(GaugeBound(-2), GaugeBound(1)),
                  Gauge(GaugeBound(-1), GaugeBound(2)),
                  Gauge(GaugeBound(-1), GaugeBound(1)));
  test_gauge_meet(Gauge(GaugeBound(0), GaugeBound(3)),
                  Gauge(GaugeBound(-1), GaugeBound(2)),
                  Gauge(GaugeBound(0), GaugeBound(2)));
  test_gauge_meet(Gauge(GaugeBound(0), GaugeBound(3)),
                  Gauge(GaugeBound(4), GaugeBound(5)),
                  Gauge::bottom());
  test_gauge_meet(Gauge(GaugeBound(-1) + GaugeBound(2, x),
                        GaugeBound(3) + GaugeBound(3, x)),
                  Gauge(GaugeBound(-1),
                        GaugeBound(4) + GaugeBound(4, x) + GaugeBound(y)),
                  Gauge(GaugeBound(-1) + GaugeBound(2, x),
                        GaugeBound(3) + GaugeBound(3, x)));
  test_gauge_meet(Gauge(GaugeBound(-1) + GaugeBound(2, x),
                        GaugeBound(3) + GaugeBound(3, x)),
                  Gauge(GaugeBound(-1) + GaugeBound(3, y),
                        GaugeBound(3) + GaugeBound(4, y)),
                  Gauge(GaugeBound(-1) + GaugeBound(3, y),
                        GaugeBound(3) + GaugeBound(3, x) + GaugeBound(3, y)));
  test_gauge_meet(Gauge(GaugeBound(0), GaugeBound(y)),
                  Gauge(GaugeBound(x), GaugeBound::plus_infinity()),
                  Gauge(GaugeBound(x), GaugeBound(x) + GaugeBound(y)));
  test_gauge_meet(Gauge(GaugeBound(x), GaugeBound(1) + GaugeBound(x)),
                  Gauge(GaugeBound(x) + GaugeBound(y) + GaugeBound(1),
                        GaugeBound::plus_infinity()),
                  Gauge(GaugeBound(x) + GaugeBound(y) + GaugeBound(1)));
}

#define test_gauge_neg(x, y) BOOST_CHECK(-(x) == (y))

BOOST_AUTO_TEST_CASE(gauge_neg) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  test_gauge_neg(Gauge::top(), Gauge::top());
  test_gauge_neg(Gauge::bottom(), Gauge::bottom());
  test_gauge_neg(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge(GaugeBound::minus_infinity(), GaugeBound(0)));
  test_gauge_neg(Gauge(GaugeBound(0), GaugeBound(1)),
                 Gauge(GaugeBound(-1), GaugeBound(0)));
  test_gauge_neg(Gauge(GaugeBound(-2), GaugeBound(1)),
                 Gauge(GaugeBound(-1), GaugeBound(2)));
  test_gauge_neg(Gauge(GaugeBound(-1) + GaugeBound(2, x),
                       GaugeBound(3) + GaugeBound(3, x)),
                 Gauge(GaugeBound(-3) + GaugeBound(-3, x),
                       GaugeBound(1) + GaugeBound(-2, x)));
}

#define test_gauge_add(x, y, z) BOOST_CHECK((x) + (y) == (z))

BOOST_AUTO_TEST_CASE(gauge_add) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  test_gauge_add(Gauge::top(), Gauge::bottom(), Gauge::bottom());
  test_gauge_add(Gauge::top(), Gauge::top(), Gauge::top());
  test_gauge_add(Gauge::top(),
                 Gauge(GaugeBound(0), GaugeBound(1)),
                 Gauge::top());
  test_gauge_add(Gauge::top(),
                 Gauge(GaugeBound(0), GaugeBound(2, x)),
                 Gauge::top());
  test_gauge_add(Gauge::top(),
                 Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge::top());
  test_gauge_add(Gauge::top(),
                 Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                 Gauge::top());
  test_gauge_add(Gauge::bottom(), Gauge::bottom(), Gauge::bottom());
  test_gauge_add(Gauge::bottom(), Gauge::top(), Gauge::bottom());
  test_gauge_add(Gauge::bottom(),
                 Gauge(GaugeBound(0), GaugeBound(1)),
                 Gauge::bottom());
  test_gauge_add(Gauge::bottom(),
                 Gauge(GaugeBound(0), GaugeBound(2, x)),
                 Gauge::bottom());
  test_gauge_add(Gauge::bottom(),
                 Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge::bottom());
  test_gauge_add(Gauge::bottom(),
                 Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                 Gauge::bottom());
  test_gauge_add(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge::bottom(),
                 Gauge::bottom());
  test_gauge_add(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge::top(),
                 Gauge::top());
  test_gauge_add(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge(GaugeBound(0), GaugeBound(1)),
                 Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  test_gauge_add(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  test_gauge_add(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                 Gauge::top());
  test_gauge_add(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge(GaugeBound::minus_infinity(), GaugeBound(2, x)),
                 Gauge::top());
  test_gauge_add(Gauge(GaugeBound(0), GaugeBound(1)),
                 Gauge(GaugeBound(-1), GaugeBound(2)),
                 Gauge(GaugeBound(-1), GaugeBound(3)));
  test_gauge_add(Gauge(GaugeBound(0), GaugeBound(1)),
                 Gauge(GaugeBound(-1), GaugeBound(2) + GaugeBound(2, x)),
                 Gauge(GaugeBound(-1), GaugeBound(3) + GaugeBound(2, x)));
  test_gauge_add(Gauge(GaugeBound(-2), GaugeBound(1)),
                 Gauge(GaugeBound(-1), GaugeBound(2)),
                 Gauge(GaugeBound(-3), GaugeBound(3)));
  test_gauge_add(Gauge(GaugeBound(0), GaugeBound(3)),
                 Gauge(GaugeBound(-1), GaugeBound(2)),
                 Gauge(GaugeBound(-1), GaugeBound(5)));
  test_gauge_add(Gauge(GaugeBound(-1) + GaugeBound(2, x),
                       GaugeBound(3) + GaugeBound(3, x)),
                 Gauge(GaugeBound(-1),
                       GaugeBound(4) + GaugeBound(4, x) + GaugeBound(y)),
                 Gauge(GaugeBound(-2) + GaugeBound(2, x),
                       GaugeBound(7) + GaugeBound(7, x) + GaugeBound(y)));
}

#define test_gauge_sub(x, y, z) BOOST_CHECK((x) - (y) == (z))

BOOST_AUTO_TEST_CASE(gauge_sub) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  test_gauge_sub(Gauge::top(), Gauge::bottom(), Gauge::bottom());
  test_gauge_sub(Gauge::top(), Gauge::top(), Gauge::top());
  test_gauge_sub(Gauge::top(),
                 Gauge(GaugeBound(0), GaugeBound(1)),
                 Gauge::top());
  test_gauge_sub(Gauge::top(),
                 Gauge(GaugeBound(0), GaugeBound(2, x)),
                 Gauge::top());
  test_gauge_sub(Gauge::top(),
                 Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge::top());
  test_gauge_sub(Gauge::top(),
                 Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                 Gauge::top());
  test_gauge_sub(Gauge::bottom(), Gauge::bottom(), Gauge::bottom());
  test_gauge_sub(Gauge::bottom(), Gauge::top(), Gauge::bottom());
  test_gauge_sub(Gauge::bottom(),
                 Gauge(GaugeBound(0), GaugeBound(1)),
                 Gauge::bottom());
  test_gauge_sub(Gauge::bottom(),
                 Gauge(GaugeBound(0), GaugeBound(2, x)),
                 Gauge::bottom());
  test_gauge_sub(Gauge::bottom(),
                 Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge::bottom());
  test_gauge_sub(Gauge::bottom(),
                 Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                 Gauge::bottom());
  test_gauge_sub(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge::bottom(),
                 Gauge::bottom());
  test_gauge_sub(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge::top(),
                 Gauge::top());
  test_gauge_sub(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge(GaugeBound(0), GaugeBound(1)),
                 Gauge(GaugeBound(-1), GaugeBound::plus_infinity()));
  test_gauge_sub(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge::top());
  test_gauge_sub(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge(GaugeBound::minus_infinity(), GaugeBound(0)),
                 Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  test_gauge_sub(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                 Gauge(GaugeBound::minus_infinity(), GaugeBound(2, x)),
                 Gauge(GaugeBound(-2, x), GaugeBound::plus_infinity()));
  test_gauge_sub(Gauge(GaugeBound(0), GaugeBound(1)),
                 Gauge(GaugeBound(-1), GaugeBound(2)),
                 Gauge(GaugeBound(-2), GaugeBound(2)));
  test_gauge_sub(Gauge(GaugeBound(0), GaugeBound(1)),
                 Gauge(GaugeBound(-1), GaugeBound(2) + GaugeBound(2, x)),
                 Gauge(GaugeBound(-2) + GaugeBound(-2, x), GaugeBound(2)));
  test_gauge_sub(Gauge(GaugeBound(-2), GaugeBound(1)),
                 Gauge(GaugeBound(-1), GaugeBound(2)),
                 Gauge(GaugeBound(-4), GaugeBound(2)));
  test_gauge_sub(Gauge(GaugeBound(0), GaugeBound(3)),
                 Gauge(GaugeBound(-1), GaugeBound(2)),
                 Gauge(GaugeBound(-2), GaugeBound(4)));
  test_gauge_sub(Gauge(GaugeBound(-1) + GaugeBound(2, x),
                       GaugeBound(3) + GaugeBound(3, x)),
                 Gauge(GaugeBound(-1),
                       GaugeBound(4) + GaugeBound(4, x) + GaugeBound(y)),
                 Gauge(GaugeBound(-5) + GaugeBound(-2, x) + GaugeBound(-1, y),
                       GaugeBound(4) + GaugeBound(3, x)));
}

#define test_gauge_mul_cst(x, y, z) BOOST_CHECK((x)*ZNumber(y) == (z))

BOOST_AUTO_TEST_CASE(gauge_mul_cst) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  test_gauge_mul_cst(Gauge::top(), 0, Gauge(0));
  test_gauge_mul_cst(Gauge::top(), 1, Gauge::top());
  test_gauge_mul_cst(Gauge::top(), -1, Gauge::top());
  test_gauge_mul_cst(Gauge::bottom(), 0, Gauge::bottom());
  test_gauge_mul_cst(Gauge::bottom(), 1, Gauge::bottom());
  test_gauge_mul_cst(Gauge::bottom(), -1, Gauge::bottom());
  test_gauge_mul_cst(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                     0,
                     Gauge(0));
  test_gauge_mul_cst(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                     2,
                     Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  test_gauge_mul_cst(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                     -2,
                     Gauge(GaugeBound::minus_infinity(), GaugeBound(0)));
  test_gauge_mul_cst(Gauge(GaugeBound(0), GaugeBound(1)), 0, Gauge(0));
  test_gauge_mul_cst(Gauge(GaugeBound(0), GaugeBound(1)),
                     2,
                     Gauge(GaugeBound(0), GaugeBound(2)));
  test_gauge_mul_cst(Gauge(GaugeBound(0), GaugeBound(1)),
                     -2,
                     Gauge(GaugeBound(-2), GaugeBound(0)));
  test_gauge_mul_cst(Gauge(GaugeBound(-2), GaugeBound(1)), 0, Gauge(0));
  test_gauge_mul_cst(Gauge(GaugeBound(-2), GaugeBound(1)),
                     2,
                     Gauge(GaugeBound(-4), GaugeBound(2)));
  test_gauge_mul_cst(Gauge(GaugeBound(-2), GaugeBound(1)),
                     -2,
                     Gauge(GaugeBound(-2), GaugeBound(4)));
  test_gauge_mul_cst(Gauge(GaugeBound(-1) + GaugeBound(2, x),
                           GaugeBound(3) + GaugeBound(3, x)),
                     0,
                     Gauge(0));
  test_gauge_mul_cst(Gauge(GaugeBound(-1) + GaugeBound(2, x),
                           GaugeBound(3) + GaugeBound(3, x)),
                     2,
                     Gauge(GaugeBound(-2) + GaugeBound(4, x),
                           GaugeBound(6) + GaugeBound(6, x)));
  test_gauge_mul_cst(Gauge(GaugeBound(-1) + GaugeBound(2, x),
                           GaugeBound(3) + GaugeBound(3, x)),
                     -2,
                     Gauge(GaugeBound(-6) + GaugeBound(-6, x),
                           GaugeBound(2) + GaugeBound(-4, x)));
}

#define test_gauge_mul_intv(x, y, z) BOOST_CHECK((x) * (y) == (z))

BOOST_AUTO_TEST_CASE(gauge_mul_intv) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  test_gauge_mul_intv(Gauge::top(), ZInterval::top(), Gauge::top());
  test_gauge_mul_intv(Gauge::top(), ZInterval::bottom(), Gauge::bottom());
  test_gauge_mul_intv(Gauge::top(),
                      ZInterval(ZBound(0), ZBound(1)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge::top(),
                      ZInterval(ZBound::minus_infinity(), ZBound(1)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge::top(),
                      ZInterval(ZBound(0), ZBound::plus_infinity()),
                      Gauge::top());
  test_gauge_mul_intv(Gauge::bottom(), ZInterval::top(), Gauge::bottom());
  test_gauge_mul_intv(Gauge::bottom(), ZInterval::bottom(), Gauge::bottom());
  test_gauge_mul_intv(Gauge::bottom(),
                      ZInterval(ZBound(0), ZBound(1)),
                      Gauge::bottom());
  test_gauge_mul_intv(Gauge::bottom(),
                      ZInterval(ZBound::minus_infinity(), ZBound(1)),
                      Gauge::bottom());
  test_gauge_mul_intv(Gauge::bottom(),
                      ZInterval(ZBound(0), ZBound::plus_infinity()),
                      Gauge::bottom());
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(2)),
                      ZInterval::top(),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(2)),
                      ZInterval::bottom(),
                      Gauge::bottom());
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(2)),
                      ZInterval(ZBound::minus_infinity(), ZBound(3)),
                      Gauge(GaugeBound::minus_infinity(), GaugeBound(6)));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(2)),
                      ZInterval(ZBound::minus_infinity(), ZBound(-3)),
                      Gauge(GaugeBound::minus_infinity(), GaugeBound(-3)));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(2)),
                      ZInterval(ZBound(1), ZBound::plus_infinity()),
                      Gauge(GaugeBound(1), GaugeBound::plus_infinity()));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(2)),
                      ZInterval(ZBound(-2), ZBound::plus_infinity()),
                      Gauge(GaugeBound(-4), GaugeBound::plus_infinity()));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(2)),
                      ZInterval(ZBound(2), ZBound(3)),
                      Gauge(GaugeBound(2), GaugeBound(6)));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(2)),
                      ZInterval(ZBound(-3), ZBound(-2)),
                      Gauge(GaugeBound(-6), GaugeBound(-2)));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(2)),
                      ZInterval(ZBound(-2), ZBound(3)),
                      Gauge(GaugeBound(-4), GaugeBound(6)));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(1) + GaugeBound(x)),
                      ZInterval::top(),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(1) + GaugeBound(x)),
                      ZInterval::bottom(),
                      Gauge::bottom());
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(1) + GaugeBound(x)),
                      ZInterval(ZBound::minus_infinity(), ZBound(3)),
                      Gauge(GaugeBound::minus_infinity(),
                            GaugeBound(3) + GaugeBound(3, x)));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(1) + GaugeBound(x)),
                      ZInterval(ZBound::minus_infinity(), ZBound(-3)),
                      Gauge(GaugeBound::minus_infinity(), GaugeBound(-3)));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(1) + GaugeBound(x)),
                      ZInterval(ZBound(1), ZBound::plus_infinity()),
                      Gauge(GaugeBound(1), GaugeBound::plus_infinity()));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(1) + GaugeBound(x)),
                      ZInterval(ZBound(-2), ZBound::plus_infinity()),
                      Gauge(GaugeBound(-2) + GaugeBound(-2, x),
                            GaugeBound::plus_infinity()));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(1) + GaugeBound(x)),
                      ZInterval(ZBound(2), ZBound(3)),
                      Gauge(GaugeBound(2), GaugeBound(3) + GaugeBound(3, x)));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(1) + GaugeBound(x)),
                      ZInterval(ZBound(-3), ZBound(-2)),
                      Gauge(GaugeBound(-3) + GaugeBound(-3, x),
                            GaugeBound(-2)));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound(1) + GaugeBound(x)),
                      ZInterval(ZBound(-2), ZBound(3)),
                      Gauge(GaugeBound(-2) + GaugeBound(-2, x),
                            GaugeBound(3) + GaugeBound(3, x)));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound::plus_infinity()),
                      ZInterval::top(),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound::plus_infinity()),
                      ZInterval::bottom(),
                      Gauge::bottom());
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound::plus_infinity()),
                      ZInterval(ZBound::minus_infinity(), ZBound(3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound::plus_infinity()),
                      ZInterval(ZBound::minus_infinity(), ZBound(-3)),
                      Gauge(GaugeBound::minus_infinity(), GaugeBound(-3)));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound::plus_infinity()),
                      ZInterval(ZBound(1), ZBound::plus_infinity()),
                      Gauge(GaugeBound(1), GaugeBound::plus_infinity()));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound::plus_infinity()),
                      ZInterval(ZBound(-2), ZBound::plus_infinity()),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound::plus_infinity()),
                      ZInterval(ZBound(2), ZBound(3)),
                      Gauge(GaugeBound(2), GaugeBound::plus_infinity()));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound::plus_infinity()),
                      ZInterval(ZBound(-3), ZBound(-2)),
                      Gauge(GaugeBound::minus_infinity(), GaugeBound(-2)));
  test_gauge_mul_intv(Gauge(GaugeBound(1), GaugeBound::plus_infinity()),
                      ZInterval(ZBound(-2), ZBound(3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(-1), GaugeBound::plus_infinity()),
                      ZInterval::top(),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(-1), GaugeBound::plus_infinity()),
                      ZInterval::bottom(),
                      Gauge::bottom());
  test_gauge_mul_intv(Gauge(GaugeBound(-1), GaugeBound::plus_infinity()),
                      ZInterval(ZBound::minus_infinity(), ZBound(3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(-1), GaugeBound::plus_infinity()),
                      ZInterval(ZBound::minus_infinity(), ZBound(-3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(-1), GaugeBound::plus_infinity()),
                      ZInterval(ZBound(1), ZBound::plus_infinity()),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(-1), GaugeBound::plus_infinity()),
                      ZInterval(ZBound(-2), ZBound::plus_infinity()),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(-1), GaugeBound::plus_infinity()),
                      ZInterval(ZBound(2), ZBound(3)),
                      Gauge(GaugeBound(-3), GaugeBound::plus_infinity()));
  test_gauge_mul_intv(Gauge(GaugeBound(-1), GaugeBound::plus_infinity()),
                      ZInterval(ZBound(-3), ZBound(-2)),
                      Gauge(GaugeBound::minus_infinity(), GaugeBound(3)));
  test_gauge_mul_intv(Gauge(GaugeBound(-1), GaugeBound::plus_infinity()),
                      ZInterval(ZBound(-2), ZBound(3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(2, x),
                            GaugeBound::plus_infinity()),
                      ZInterval::top(),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(2, x),
                            GaugeBound::plus_infinity()),
                      ZInterval::bottom(),
                      Gauge::bottom());
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(2, x),
                            GaugeBound::plus_infinity()),
                      ZInterval(ZBound::minus_infinity(), ZBound(3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(2, x),
                            GaugeBound::plus_infinity()),
                      ZInterval(ZBound::minus_infinity(), ZBound(-3)),
                      Gauge(GaugeBound::minus_infinity(),
                            GaugeBound(-3) + GaugeBound(-6, x)));
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(2, x),
                            GaugeBound::plus_infinity()),
                      ZInterval(ZBound(1), ZBound::plus_infinity()),
                      Gauge(GaugeBound(1) + GaugeBound(2, x),
                            GaugeBound::plus_infinity()));
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(2, x),
                            GaugeBound::plus_infinity()),
                      ZInterval(ZBound(-2), ZBound::plus_infinity()),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(2, x),
                            GaugeBound::plus_infinity()),
                      ZInterval(ZBound(2), ZBound(3)),
                      Gauge(GaugeBound(2) + GaugeBound(4, x),
                            GaugeBound::plus_infinity()));
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(2, x),
                            GaugeBound::plus_infinity()),
                      ZInterval(ZBound(-3), ZBound(-2)),
                      Gauge(GaugeBound::minus_infinity(),
                            GaugeBound(-2) + GaugeBound(-4, x)));
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(2, x),
                            GaugeBound::plus_infinity()),
                      ZInterval(ZBound(-2), ZBound(3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(-1)),
                      ZInterval::top(),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(-1)),
                      ZInterval::bottom(),
                      Gauge::bottom());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(-1)),
                      ZInterval(ZBound::minus_infinity(), ZBound(3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(-1)),
                      ZInterval(ZBound::minus_infinity(), ZBound(-3)),
                      Gauge(GaugeBound(3), GaugeBound::plus_infinity()));
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(-1)),
                      ZInterval(ZBound(1), ZBound::plus_infinity()),
                      Gauge(GaugeBound::minus_infinity(), GaugeBound(-1)));
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(-1)),
                      ZInterval(ZBound(-2), ZBound::plus_infinity()),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(-1)),
                      ZInterval(ZBound(2), ZBound(3)),
                      Gauge(GaugeBound::minus_infinity(), GaugeBound(-2)));
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(-1)),
                      ZInterval(ZBound(-3), ZBound(-2)),
                      Gauge(GaugeBound(2), GaugeBound::plus_infinity()));
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(-1)),
                      ZInterval(ZBound(-2), ZBound(3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(1)),
                      ZInterval::top(),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(1)),
                      ZInterval::bottom(),
                      Gauge::bottom());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(1)),
                      ZInterval(ZBound::minus_infinity(), ZBound(3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(1)),
                      ZInterval(ZBound::minus_infinity(), ZBound(-3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(1)),
                      ZInterval(ZBound(1), ZBound::plus_infinity()),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(1)),
                      ZInterval(ZBound(-2), ZBound::plus_infinity()),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(1)),
                      ZInterval(ZBound(2), ZBound(3)),
                      Gauge(GaugeBound::minus_infinity(), GaugeBound(3)));
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(1)),
                      ZInterval(ZBound(-3), ZBound(-2)),
                      Gauge(GaugeBound(-3), GaugeBound::plus_infinity()));
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(), GaugeBound(1)),
                      ZInterval(ZBound(-2), ZBound(3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(),
                            GaugeBound(1) + GaugeBound(3, x)),
                      ZInterval::top(),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(),
                            GaugeBound(1) + GaugeBound(3, x)),
                      ZInterval::bottom(),
                      Gauge::bottom());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(),
                            GaugeBound(1) + GaugeBound(3, x)),
                      ZInterval(ZBound::minus_infinity(), ZBound(3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(),
                            GaugeBound(1) + GaugeBound(3, x)),
                      ZInterval(ZBound::minus_infinity(), ZBound(-3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(),
                            GaugeBound(1) + GaugeBound(3, x)),
                      ZInterval(ZBound(1), ZBound::plus_infinity()),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(),
                            GaugeBound(1) + GaugeBound(3, x)),
                      ZInterval(ZBound(-2), ZBound::plus_infinity()),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(),
                            GaugeBound(1) + GaugeBound(3, x)),
                      ZInterval(ZBound(2), ZBound(3)),
                      Gauge(GaugeBound::minus_infinity(),
                            GaugeBound(3) + GaugeBound(9, x)));
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(),
                            GaugeBound(1) + GaugeBound(3, x)),
                      ZInterval(ZBound(-3), ZBound(-2)),
                      Gauge(GaugeBound(-3) + GaugeBound(-9, x),
                            GaugeBound::plus_infinity()));
  test_gauge_mul_intv(Gauge(GaugeBound::minus_infinity(),
                            GaugeBound(1) + GaugeBound(3, x)),
                      ZInterval(ZBound(-2), ZBound(3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(-1) + GaugeBound(-2, x), GaugeBound(-1)),
                      ZInterval::top(),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(-1) + GaugeBound(-2, x), GaugeBound(-1)),
                      ZInterval::bottom(),
                      Gauge::bottom());
  test_gauge_mul_intv(Gauge(GaugeBound(-1) + GaugeBound(-2, x), GaugeBound(-1)),
                      ZInterval(ZBound::minus_infinity(), ZBound(3)),
                      Gauge(GaugeBound(-3) + GaugeBound(-6, x),
                            GaugeBound::plus_infinity()));
  test_gauge_mul_intv(Gauge(GaugeBound(-1) + GaugeBound(-2, x), GaugeBound(-1)),
                      ZInterval(ZBound::minus_infinity(), ZBound(-3)),
                      Gauge(GaugeBound(3), GaugeBound::plus_infinity()));
  test_gauge_mul_intv(Gauge(GaugeBound(-1) + GaugeBound(-2, x), GaugeBound(-1)),
                      ZInterval(ZBound(1), ZBound::plus_infinity()),
                      Gauge(GaugeBound::minus_infinity(), GaugeBound(-1)));
  test_gauge_mul_intv(Gauge(GaugeBound(-1) + GaugeBound(-2, x), GaugeBound(-1)),
                      ZInterval(ZBound(-2), ZBound::plus_infinity()),
                      Gauge(GaugeBound::minus_infinity(),
                            GaugeBound(2) + GaugeBound(4, x)));
  test_gauge_mul_intv(Gauge(GaugeBound(-1) + GaugeBound(-2, x), GaugeBound(-1)),
                      ZInterval(ZBound(2), ZBound(3)),
                      Gauge(GaugeBound(-3) + GaugeBound(-6, x),
                            GaugeBound(-2)));
  test_gauge_mul_intv(Gauge(GaugeBound(-1) + GaugeBound(-2, x), GaugeBound(-1)),
                      ZInterval(ZBound(-3), ZBound(-2)),
                      Gauge(GaugeBound(2), GaugeBound(3) + GaugeBound(6, x)));
  test_gauge_mul_intv(Gauge(GaugeBound(-1) + GaugeBound(-2, x), GaugeBound(-1)),
                      ZInterval(ZBound(-2), ZBound(3)),
                      Gauge(GaugeBound(-3) + GaugeBound(-6, x),
                            GaugeBound(2) + GaugeBound(4, x)));
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x)),
                      ZInterval::top(),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x)),
                      ZInterval::bottom(),
                      Gauge::bottom());
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x)),
                      ZInterval(ZBound::minus_infinity(), ZBound(3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x)),
                      ZInterval(ZBound::minus_infinity(), ZBound(-3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x)),
                      ZInterval(ZBound(1), ZBound::plus_infinity()),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x)),
                      ZInterval(ZBound(-2), ZBound::plus_infinity()),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x)),
                      ZInterval(ZBound(2), ZBound(3)),
                      Gauge(GaugeBound(2) + GaugeBound(-3, x),
                            GaugeBound(3) + GaugeBound(3, x)));
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x)),
                      ZInterval(ZBound(-3), ZBound(-2)),
                      Gauge(GaugeBound(-3) + GaugeBound(-3, x),
                            GaugeBound(-2) + GaugeBound(3, x)));
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x)),
                      ZInterval(ZBound(-2), ZBound(3)),
                      Gauge(GaugeBound(-2) + GaugeBound(-3, x),
                            GaugeBound(3) + GaugeBound(3, x)));
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x) + GaugeBound(2, y)),
                      ZInterval::top(),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x) + GaugeBound(2, y)),
                      ZInterval::bottom(),
                      Gauge::bottom());
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x) + GaugeBound(2, y)),
                      ZInterval(ZBound::minus_infinity(), ZBound(3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x) + GaugeBound(2, y)),
                      ZInterval(ZBound::minus_infinity(), ZBound(-3)),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x) + GaugeBound(2, y)),
                      ZInterval(ZBound(1), ZBound::plus_infinity()),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x) + GaugeBound(2, y)),
                      ZInterval(ZBound(-2), ZBound::plus_infinity()),
                      Gauge::top());
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x) + GaugeBound(2, y)),
                      ZInterval(ZBound(2), ZBound(3)),
                      Gauge(GaugeBound(2) + GaugeBound(-3, x),
                            GaugeBound(3) + GaugeBound(3, x) +
                                GaugeBound(6, y)));
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x) + GaugeBound(2, y)),
                      ZInterval(ZBound(-3), ZBound(-2)),
                      Gauge(GaugeBound(-3) + GaugeBound(-3, x) +
                                GaugeBound(-6, y),
                            GaugeBound(-2) + GaugeBound(3, x)));
  test_gauge_mul_intv(Gauge(GaugeBound(1) + GaugeBound(-1, x),
                            GaugeBound(1) + GaugeBound(x) + GaugeBound(2, y)),
                      ZInterval(ZBound(-2), ZBound(3)),
                      Gauge(GaugeBound(-2) + GaugeBound(-3, x) +
                                GaugeBound(-4, y),
                            GaugeBound(3) + GaugeBound(3, x) +
                                GaugeBound(6, y)));
}

#define test_gauge_forget(x, v, y) BOOST_CHECK((x).forget(v) == (y))

BOOST_AUTO_TEST_CASE(gauge_forget) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  test_gauge_forget(Gauge::top(), x, Gauge::top());
  test_gauge_forget(Gauge::bottom(), x, Gauge::bottom());
  test_gauge_forget(Gauge(GaugeBound(0), GaugeBound(1)),
                    x,
                    Gauge(GaugeBound(0), GaugeBound(1)));
  test_gauge_forget(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                    x,
                    Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  test_gauge_forget(Gauge(GaugeBound::minus_infinity(), GaugeBound(1)),
                    x,
                    Gauge(GaugeBound::minus_infinity(), GaugeBound(1)));
  test_gauge_forget(Gauge(GaugeBound(0), GaugeBound(2, x)), x, Gauge::top());
  test_gauge_forget(Gauge(GaugeBound(-1, x), GaugeBound(2)), x, Gauge::top());
  test_gauge_forget(Gauge(GaugeBound(0), GaugeBound(2, y)),
                    x,
                    Gauge(GaugeBound(0), GaugeBound(2, y)));
}

#define test_gauge_coalesce(x, v, l, u, y) \
  BOOST_CHECK((x).coalesce(v, ZNumber(l), u) == (y))

BOOST_AUTO_TEST_CASE(gauge_coalesce) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  test_gauge_coalesce(Gauge::top(),
                      x,
                      0,
                      ZBound::plus_infinity(),
                      Gauge::top());
  test_gauge_coalesce(Gauge::bottom(),
                      x,
                      0,
                      ZBound::plus_infinity(),
                      Gauge::bottom());
  test_gauge_coalesce(Gauge(GaugeBound(0), GaugeBound(1)),
                      x,
                      0,
                      ZBound::plus_infinity(),
                      Gauge(GaugeBound(0), GaugeBound(1)));
  test_gauge_coalesce(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                      x,
                      0,
                      ZBound::plus_infinity(),
                      Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  test_gauge_coalesce(Gauge(GaugeBound::minus_infinity(), GaugeBound(1)),
                      x,
                      0,
                      ZBound::plus_infinity(),
                      Gauge(GaugeBound::minus_infinity(), GaugeBound(1)));
  test_gauge_coalesce(Gauge(GaugeBound(0), GaugeBound(2, x)),
                      x,
                      0,
                      ZBound::plus_infinity(),
                      Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  test_gauge_coalesce(Gauge(GaugeBound(-1, x), GaugeBound(2)),
                      x,
                      0,
                      ZBound::plus_infinity(),
                      Gauge(GaugeBound::minus_infinity(), GaugeBound(2)));
  test_gauge_coalesce(Gauge(GaugeBound(0), GaugeBound(2, y)),
                      x,
                      0,
                      ZBound::plus_infinity(),
                      Gauge(GaugeBound(0), GaugeBound(2, y)));
  test_gauge_coalesce(Gauge(GaugeBound(x), GaugeBound(1) + GaugeBound(x)),
                      x,
                      1,
                      ZBound(2),
                      Gauge(GaugeBound(1), GaugeBound(3)));
  test_gauge_coalesce(Gauge(GaugeBound(2) + GaugeBound(-2, x),
                            GaugeBound(3) + GaugeBound(-2, x)),
                      x,
                      1,
                      ZBound(3),
                      Gauge(GaugeBound(-4), GaugeBound(1)));
  test_gauge_coalesce(Gauge(GaugeBound(2) + GaugeBound(-2, x) +
                                GaugeBound(3, y),
                            GaugeBound(3) + GaugeBound(-2, x) +
                                GaugeBound(3, y)),
                      x,
                      1,
                      ZBound(3),
                      Gauge(GaugeBound(-4) + GaugeBound(3, y),
                            GaugeBound(1) + GaugeBound(3, y)));
  test_gauge_coalesce(Gauge(GaugeBound(2) + GaugeBound(-2, x),
                            GaugeBound(3) + GaugeBound(2, x)),
                      x,
                      1,
                      ZBound::plus_infinity(),
                      Gauge::top());
  test_gauge_coalesce(Gauge(GaugeBound(2) + GaugeBound(-2, x) +
                                GaugeBound(3, y),
                            GaugeBound(3) + GaugeBound(2, x) +
                                GaugeBound(3, y)),
                      x,
                      1,
                      ZBound::plus_infinity(),
                      Gauge::top());
}

#define test_gauge_counter_incr(x, v, k, y) \
  BOOST_CHECK((x).counter_incr(v, ZNumber(k)) == (y))

BOOST_AUTO_TEST_CASE(gauge_counter_incr) {
  VariableFactory vfac;
  Variable i(vfac.get("i"));
  Variable k(vfac.get("k"));

  test_gauge_counter_incr(Gauge::top(), i, 1, Gauge::top());
  test_gauge_counter_incr(Gauge::bottom(), i, 1, Gauge::bottom());
  test_gauge_counter_incr(Gauge(GaugeBound(0), GaugeBound(1)),
                          i,
                          1,
                          Gauge(GaugeBound(0), GaugeBound(1)));
  test_gauge_counter_incr(Gauge(GaugeBound(0), GaugeBound::plus_infinity()),
                          i,
                          1,
                          Gauge(GaugeBound(0), GaugeBound::plus_infinity()));
  test_gauge_counter_incr(Gauge(GaugeBound(1) + GaugeBound(2, i),
                                GaugeBound::plus_infinity()),
                          i,
                          1,
                          Gauge(GaugeBound(-1) + GaugeBound(2, i),
                                GaugeBound::plus_infinity()));
  test_gauge_counter_incr(Gauge(GaugeBound::minus_infinity(), GaugeBound(1)),
                          i,
                          1,
                          Gauge(GaugeBound::minus_infinity(), GaugeBound(1)));
  test_gauge_counter_incr(Gauge(GaugeBound::minus_infinity(),
                                GaugeBound(1) + GaugeBound(2, i)),
                          i,
                          1,
                          Gauge(GaugeBound::minus_infinity(),
                                GaugeBound(-1) + GaugeBound(2, i)));
  test_gauge_counter_incr(Gauge(GaugeBound(i)),
                          i,
                          1,
                          Gauge(GaugeBound(-1) + GaugeBound(i),
                                GaugeBound(-1) + GaugeBound(i)));
  test_gauge_counter_incr(Gauge(GaugeBound(0), GaugeBound(2, i)),
                          i,
                          1,
                          Gauge(GaugeBound(-2), GaugeBound(2, i)));
  test_gauge_counter_incr(Gauge(GaugeBound(1),
                                GaugeBound(2) + GaugeBound(3, i)),
                          i,
                          1,
                          Gauge(GaugeBound(-1),
                                GaugeBound(1) + GaugeBound(3, i)));
  test_gauge_counter_incr(Gauge(GaugeBound(-2) + GaugeBound(2, i) +
                                    GaugeBound(2, k),
                                GaugeBound(2) + GaugeBound(3, i) +
                                    GaugeBound(3, k)),
                          i,
                          1,
                          Gauge(GaugeBound(-4) + GaugeBound(2, i) +
                                    GaugeBound(2, k),
                                GaugeBound(-1) + GaugeBound(3, i) +
                                    GaugeBound(3, k)));
  test_gauge_counter_incr(Gauge(GaugeBound(-2) + GaugeBound(2, i) +
                                    GaugeBound(2, k),
                                GaugeBound(2) + GaugeBound(3, i) +
                                    GaugeBound(3, k)),
                          i,
                          2,
                          Gauge(GaugeBound(-6) + GaugeBound(2, i) +
                                    GaugeBound(2, k),
                                GaugeBound(-4) + GaugeBound(3, i) +
                                    GaugeBound(3, k)));
}

#define test_gauge_widening_interpol(x, y, k, u, v, z) \
  BOOST_CHECK((x).widening_interpol((y), k, ZNumber(u), v) == (z))

BOOST_AUTO_TEST_CASE(gauge_widening_interpol) {
  VariableFactory vfac;
  Variable i(vfac.get("i"));
  Variable k(vfac.get("k"));

  test_gauge_widening_interpol(Gauge::top(),
                               Gauge::bottom(),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge::top());
  test_gauge_widening_interpol(Gauge::top(),
                               Gauge::top(),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge::top());
  test_gauge_widening_interpol(Gauge::top(),
                               Gauge(GaugeBound(0), GaugeBound(1)),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge::top());
  test_gauge_widening_interpol(Gauge::top(),
                               Gauge(GaugeBound(0), GaugeBound(2, i)),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge::top());
  test_gauge_widening_interpol(Gauge::bottom(),
                               Gauge::bottom(),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge::bottom());
  test_gauge_widening_interpol(Gauge::bottom(),
                               Gauge::top(),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge::top());
  test_gauge_widening_interpol(Gauge::bottom(),
                               Gauge(GaugeBound(0), GaugeBound(1)),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge(GaugeBound(0), GaugeBound(1)));
  test_gauge_widening_interpol(Gauge::bottom(),
                               Gauge(GaugeBound(0), GaugeBound(2, i)),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge(GaugeBound(0), GaugeBound(2, i)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(1),
                               i,
                               0,
                               ZConstant(1),
                               Gauge(GaugeBound(i)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(GaugeBound(0), GaugeBound(1)),
                               i,
                               0,
                               ZConstant(1),
                               Gauge(GaugeBound(0), GaugeBound(i)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(GaugeBound(0), GaugeBound(2)),
                               i,
                               0,
                               ZConstant(1),
                               Gauge(GaugeBound(0), GaugeBound(2, i)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(GaugeBound(0), GaugeBound(3)),
                               i,
                               0,
                               ZConstant(1),
                               Gauge(GaugeBound(0), GaugeBound(3, i)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(GaugeBound(-2), GaugeBound(2)),
                               i,
                               0,
                               ZConstant(1),
                               Gauge(GaugeBound(-2, i), GaugeBound(2, i)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(4),
                               i,
                               0,
                               ZConstant(3),
                               Gauge(GaugeBound(i), GaugeBound(2, i)));
  test_gauge_widening_interpol(Gauge(1),
                               Gauge(2),
                               i,
                               0,
                               ZConstant(1),
                               Gauge(GaugeBound(1) + GaugeBound(i)));
  test_gauge_widening_interpol(Gauge(1),
                               Gauge(-2),
                               i,
                               0,
                               ZConstant(1),
                               Gauge(GaugeBound(1) + GaugeBound(-3, i)));
  test_gauge_widening_interpol(Gauge(GaugeBound(4, i)),
                               Gauge(GaugeBound(1) + GaugeBound(4, i)),
                               k,
                               0,
                               ZConstant(1),
                               Gauge(GaugeBound(4, i) + GaugeBound(k)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(1),
                               i,
                               2,
                               ZConstant(3),
                               Gauge(GaugeBound(-2) + GaugeBound(i)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(3),
                               i,
                               2,
                               ZConstant(3),
                               Gauge(GaugeBound(-6) + GaugeBound(3, i)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(-3),
                               i,
                               2,
                               ZConstant(3),
                               Gauge(GaugeBound(6) + GaugeBound(-3, i)));
  test_gauge_widening_interpol(Gauge(GaugeBound(1) + GaugeBound(2, i)),
                               Gauge(GaugeBound(1) + GaugeBound(2, i)),
                               i,
                               2,
                               ZConstant(3),
                               Gauge(GaugeBound(1) + GaugeBound(2, i)));
  test_gauge_widening_interpol(Gauge(GaugeBound(1) + GaugeBound(2, i)),
                               Gauge(GaugeBound(2) + GaugeBound(2, i)),
                               i,
                               2,
                               ZConstant(3),
                               Gauge(GaugeBound(-1) + GaugeBound(3, i)));
  test_gauge_widening_interpol(Gauge(GaugeBound(1),
                                     GaugeBound(1) + GaugeBound(2, i)),
                               Gauge(GaugeBound(2),
                                     GaugeBound(2) + GaugeBound(2, i)),
                               i,
                               2,
                               ZConstant(3),
                               Gauge(GaugeBound(-1) + GaugeBound(i),
                                     GaugeBound(-1) + GaugeBound(3, i)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(GaugeBound(2),
                                     GaugeBound::plus_infinity()),
                               i,
                               0,
                               ZConstant(1),
                               Gauge(GaugeBound(2, i),
                                     GaugeBound::plus_infinity()));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(GaugeBound::minus_infinity(),
                                     GaugeBound(2)),
                               i,
                               0,
                               ZConstant(1),
                               Gauge(GaugeBound::minus_infinity(),
                                     GaugeBound(2, i)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge::top(),
                               i,
                               0,
                               ZConstant(1),
                               Gauge::top());
  test_gauge_widening_interpol(Gauge(GaugeBound::minus_infinity(),
                                     GaugeBound(0)),
                               Gauge(1),
                               i,
                               0,
                               ZConstant(1),
                               Gauge(GaugeBound::minus_infinity(),
                                     GaugeBound(i)));
  test_gauge_widening_interpol(Gauge(GaugeBound(0),
                                     GaugeBound::plus_infinity()),
                               Gauge(1),
                               i,
                               0,
                               ZConstant(1),
                               Gauge(GaugeBound(i),
                                     GaugeBound::plus_infinity()));
  test_gauge_widening_interpol(Gauge::top(),
                               Gauge(1),
                               i,
                               0,
                               ZConstant(1),
                               Gauge::top());
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(1),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge(GaugeBound(0), GaugeBound(1)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(GaugeBound(0), GaugeBound(1)),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge(GaugeBound(0), GaugeBound(1)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(GaugeBound(0), GaugeBound(2)),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge(GaugeBound(0), GaugeBound(2)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(GaugeBound(-2), GaugeBound(2)),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge(GaugeBound(-2), GaugeBound(2)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(GaugeBound(0), GaugeBound(2)),
                               i,
                               1,
                               ZConstant::top(),
                               Gauge(GaugeBound(0), GaugeBound(2)));
  test_gauge_widening_interpol(Gauge(GaugeBound(4, k)),
                               Gauge(GaugeBound(2) + GaugeBound(4, k)),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge(GaugeBound(4, k),
                                     GaugeBound(2) + GaugeBound(4, k)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(GaugeBound(i)),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge(GaugeBound(i)));
  test_gauge_widening_interpol(Gauge(1),
                               Gauge(GaugeBound(i)),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge(GaugeBound(i),
                                     GaugeBound(1) + GaugeBound(i)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(GaugeBound(i), GaugeBound(2, i)),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge(GaugeBound(i), GaugeBound(2, i)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(GaugeBound(i), GaugeBound(2, i)),
                               i,
                               2,
                               ZConstant::top(),
                               Gauge(GaugeBound(-2) + GaugeBound(i),
                                     GaugeBound(2, i)));
  test_gauge_widening_interpol(Gauge(GaugeBound(0), GaugeBound(1)),
                               Gauge(GaugeBound(2) + GaugeBound(i),
                                     GaugeBound(3) + GaugeBound(3, i)),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge(GaugeBound(i),
                                     GaugeBound(3) + GaugeBound(3, i)));
  test_gauge_widening_interpol(Gauge(GaugeBound(0), GaugeBound(1)),
                               Gauge(GaugeBound(2) + GaugeBound(i),
                                     GaugeBound(3) + GaugeBound(3, i)),
                               i,
                               2,
                               ZConstant::top(),
                               Gauge(GaugeBound(-2) + GaugeBound(i),
                                     GaugeBound(3) + GaugeBound(3, i)));
  test_gauge_widening_interpol(Gauge(GaugeBound(-2) + GaugeBound(i),
                                     GaugeBound(4) + GaugeBound(3, i)),
                               Gauge(GaugeBound(2) + GaugeBound(i),
                                     GaugeBound(3) + GaugeBound(3, i)),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge(GaugeBound(-2) + GaugeBound(i),
                                     GaugeBound(4) + GaugeBound(3, i)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(GaugeBound(2),
                                     GaugeBound::plus_infinity()),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge(GaugeBound(0),
                                     GaugeBound::plus_infinity()));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge(GaugeBound::minus_infinity(),
                                     GaugeBound(2)),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge(GaugeBound::minus_infinity(),
                                     GaugeBound(2)));
  test_gauge_widening_interpol(Gauge(0),
                               Gauge::top(),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge::top());
  test_gauge_widening_interpol(Gauge(GaugeBound::minus_infinity(),
                                     GaugeBound(0)),
                               Gauge(1),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge(GaugeBound::minus_infinity(),
                                     GaugeBound(1)));
  test_gauge_widening_interpol(Gauge(GaugeBound(0),
                                     GaugeBound::plus_infinity()),
                               Gauge(1),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge(GaugeBound(0),
                                     GaugeBound::plus_infinity()));
  test_gauge_widening_interpol(Gauge::top(),
                               Gauge(1),
                               i,
                               0,
                               ZConstant::top(),
                               Gauge::top());
}

#define test_gauge_widening_interval(x, y, z) \
  BOOST_CHECK((x).widening_interval(y) == (z))

BOOST_AUTO_TEST_CASE(gauge_widening_interval) {
  VariableFactory vfac;
  Variable i(vfac.get("i"));

  test_gauge_widening_interval(Gauge::top(), Gauge::bottom(), Gauge::top());
  test_gauge_widening_interval(Gauge::top(), Gauge::top(), Gauge::top());
  test_gauge_widening_interval(Gauge::top(),
                               Gauge(GaugeBound(0), GaugeBound(1)),
                               Gauge::top());
  test_gauge_widening_interval(Gauge::top(),
                               Gauge(GaugeBound(0), GaugeBound(2, i)),
                               Gauge::top());
  test_gauge_widening_interval(Gauge::bottom(),
                               Gauge::bottom(),
                               Gauge::bottom());
  test_gauge_widening_interval(Gauge::bottom(), Gauge::top(), Gauge::top());
  test_gauge_widening_interval(Gauge::bottom(),
                               Gauge(GaugeBound(0), GaugeBound(1)),
                               Gauge(GaugeBound(0), GaugeBound(1)));
  test_gauge_widening_interval(Gauge::bottom(),
                               Gauge(GaugeBound(0), GaugeBound(2, i)),
                               Gauge(GaugeBound(0), GaugeBound(2, i)));
  test_gauge_widening_interval(Gauge(0),
                               Gauge(1),
                               Gauge(GaugeBound(0),
                                     GaugeBound::plus_infinity()));
  test_gauge_widening_interval(Gauge(0),
                               Gauge(GaugeBound(0), GaugeBound(1)),
                               Gauge(GaugeBound(0),
                                     GaugeBound::plus_infinity()));
  test_gauge_widening_interval(Gauge(0),
                               Gauge(GaugeBound(0), GaugeBound(2)),
                               Gauge(GaugeBound(0),
                                     GaugeBound::plus_infinity()));
  test_gauge_widening_interval(Gauge(0),
                               Gauge(GaugeBound(0), GaugeBound(3)),
                               Gauge(GaugeBound(0),
                                     GaugeBound::plus_infinity()));
  test_gauge_widening_interval(Gauge(0),
                               Gauge(GaugeBound(-2), GaugeBound(2)),
                               Gauge(GaugeBound::minus_infinity(),
                                     GaugeBound::plus_infinity()));
  test_gauge_widening_interval(Gauge(GaugeBound(0), GaugeBound(4)),
                               Gauge(4),
                               Gauge(GaugeBound(0), GaugeBound(4)));
  test_gauge_widening_interval(Gauge(1),
                               Gauge(2),
                               Gauge(GaugeBound(1),
                                     GaugeBound::plus_infinity()));
  test_gauge_widening_interval(Gauge(1),
                               Gauge(-2),
                               Gauge(GaugeBound::minus_infinity(),
                                     GaugeBound(1)));
  test_gauge_widening_interval(Gauge(GaugeBound(4, i)),
                               Gauge(GaugeBound(1) + GaugeBound(4, i)),
                               Gauge(GaugeBound(4, i),
                                     GaugeBound::plus_infinity()));
  test_gauge_widening_interval(Gauge(GaugeBound(1),
                                     GaugeBound(1) + GaugeBound(2, i)),
                               Gauge(GaugeBound(2),
                                     GaugeBound(2) + GaugeBound(2, i)),
                               Gauge(GaugeBound(1),
                                     GaugeBound::plus_infinity()));
  test_gauge_widening_interval(Gauge(0),
                               Gauge(GaugeBound(i)),
                               Gauge(GaugeBound(0),
                                     GaugeBound::plus_infinity()));
  test_gauge_widening_interval(Gauge(1), Gauge(GaugeBound(i)), Gauge::top());
  test_gauge_widening_interval(Gauge(0),
                               Gauge(GaugeBound(i), GaugeBound(2, i)),
                               Gauge(GaugeBound(0),
                                     GaugeBound::plus_infinity()));
  test_gauge_widening_interval(Gauge(GaugeBound(0), GaugeBound(1)),
                               Gauge(GaugeBound(2) + GaugeBound(i),
                                     GaugeBound(3) + GaugeBound(3, i)),
                               Gauge(GaugeBound(0),
                                     GaugeBound::plus_infinity()));
  test_gauge_widening_interval(Gauge(GaugeBound(-2) + GaugeBound(i),
                                     GaugeBound(4) + GaugeBound(3, i)),
                               Gauge(GaugeBound(2) + GaugeBound(i),
                                     GaugeBound(3) + GaugeBound(3, i)),
                               Gauge(GaugeBound(-2) + GaugeBound(i),
                                     GaugeBound(4) + GaugeBound(3, i)));
  test_gauge_widening_interval(Gauge(0),
                               Gauge(GaugeBound(2),
                                     GaugeBound::plus_infinity()),
                               Gauge(GaugeBound(0),
                                     GaugeBound::plus_infinity()));
  test_gauge_widening_interval(Gauge(0),
                               Gauge(GaugeBound::minus_infinity(),
                                     GaugeBound(2)),
                               Gauge::top());
  test_gauge_widening_interval(Gauge(0), Gauge::top(), Gauge::top());
  test_gauge_widening_interval(Gauge(GaugeBound::minus_infinity(),
                                     GaugeBound(0)),
                               Gauge(1),
                               Gauge::top());
  test_gauge_widening_interval(Gauge(GaugeBound(0),
                                     GaugeBound::plus_infinity()),
                               Gauge(1),
                               Gauge(GaugeBound(0),
                                     GaugeBound::plus_infinity()));
  test_gauge_widening_interval(Gauge::top(), Gauge(1), Gauge::top());
}

#define test_gauge_widening_interval_threshold(x, y, t, z) \
  BOOST_CHECK((x).widening_interval_threshold(y, t) == (z))

BOOST_AUTO_TEST_CASE(gauge_widening_interval_threshold) {
  VariableFactory vfac;
  Variable i(vfac.get("i"));

  test_gauge_widening_interval_threshold(Gauge::top(),
                                         Gauge::bottom(),
                                         ZNumber(10),
                                         Gauge::top());
  test_gauge_widening_interval_threshold(Gauge::top(),
                                         Gauge::top(),
                                         ZNumber(10),
                                         Gauge::top());
  test_gauge_widening_interval_threshold(Gauge::top(),
                                         Gauge(GaugeBound(0), GaugeBound(1)),
                                         ZNumber(10),
                                         Gauge::top());
  test_gauge_widening_interval_threshold(Gauge::top(),
                                         Gauge(GaugeBound(0), GaugeBound(2, i)),
                                         ZNumber(10),
                                         Gauge::top());
  test_gauge_widening_interval_threshold(Gauge::bottom(),
                                         Gauge::bottom(),
                                         ZNumber(10),
                                         Gauge::bottom());
  test_gauge_widening_interval_threshold(Gauge::bottom(),
                                         Gauge::top(),
                                         ZNumber(10),
                                         Gauge::top());
  test_gauge_widening_interval_threshold(Gauge::bottom(),
                                         Gauge(GaugeBound(0), GaugeBound(1)),
                                         ZNumber(10),
                                         Gauge(GaugeBound(0), GaugeBound(1)));
  test_gauge_widening_interval_threshold(Gauge::bottom(),
                                         Gauge(GaugeBound(0), GaugeBound(2, i)),
                                         ZNumber(10),
                                         Gauge(GaugeBound(0),
                                               GaugeBound(2, i)));
  test_gauge_widening_interval_threshold(Gauge(0),
                                         Gauge(1),
                                         ZNumber(10),
                                         Gauge(GaugeBound(0), GaugeBound(10)));
  test_gauge_widening_interval_threshold(Gauge(0),
                                         Gauge(GaugeBound(0), GaugeBound(1)),
                                         ZNumber(10),
                                         Gauge(GaugeBound(0), GaugeBound(10)));
  test_gauge_widening_interval_threshold(Gauge(0),
                                         Gauge(GaugeBound(0), GaugeBound(2)),
                                         ZNumber(10),
                                         Gauge(GaugeBound(0), GaugeBound(10)));
  test_gauge_widening_interval_threshold(Gauge(0),
                                         Gauge(GaugeBound(0), GaugeBound(3)),
                                         ZNumber(10),
                                         Gauge(GaugeBound(0), GaugeBound(10)));
  test_gauge_widening_interval_threshold(Gauge(0),
                                         Gauge(GaugeBound(-2), GaugeBound(2)),
                                         ZNumber(10),
                                         Gauge(GaugeBound::minus_infinity(),
                                               GaugeBound(10)));
  test_gauge_widening_interval_threshold(Gauge(GaugeBound(0), GaugeBound(4)),
                                         Gauge(4),
                                         ZNumber(10),
                                         Gauge(GaugeBound(0), GaugeBound(4)));
  test_gauge_widening_interval_threshold(Gauge(1),
                                         Gauge(2),
                                         ZNumber(10),
                                         Gauge(GaugeBound(1), GaugeBound(10)));
  test_gauge_widening_interval_threshold(Gauge(1),
                                         Gauge(-2),
                                         ZNumber(-10),
                                         Gauge(GaugeBound(-10), GaugeBound(1)));
  test_gauge_widening_interval_threshold(Gauge(GaugeBound(4, i)),
                                         Gauge(GaugeBound(1) +
                                               GaugeBound(4, i)),
                                         ZNumber(10),
                                         Gauge(GaugeBound(4, i),
                                               GaugeBound::plus_infinity()));
  test_gauge_widening_interval_threshold(Gauge(GaugeBound(1),
                                               GaugeBound(1) +
                                                   GaugeBound(2, i)),
                                         Gauge(GaugeBound(2),
                                               GaugeBound(2) +
                                                   GaugeBound(2, i)),
                                         ZNumber(10),
                                         Gauge(GaugeBound(1),
                                               GaugeBound::plus_infinity()));
  test_gauge_widening_interval_threshold(Gauge(0),
                                         Gauge(GaugeBound(i)),
                                         ZNumber(10),
                                         Gauge(GaugeBound(0),
                                               GaugeBound::plus_infinity()));
  test_gauge_widening_interval_threshold(Gauge(1),
                                         Gauge(GaugeBound(i)),
                                         ZNumber(10),
                                         Gauge::top());
  test_gauge_widening_interval_threshold(Gauge(0),
                                         Gauge(GaugeBound(i), GaugeBound(2, i)),
                                         ZNumber(10),
                                         Gauge(GaugeBound(0),
                                               GaugeBound::plus_infinity()));
  test_gauge_widening_interval_threshold(Gauge(GaugeBound(0), GaugeBound(1)),
                                         Gauge(GaugeBound(2) + GaugeBound(i),
                                               GaugeBound(3) +
                                                   GaugeBound(3, i)),
                                         ZNumber(10),
                                         Gauge(GaugeBound(0),
                                               GaugeBound::plus_infinity()));
  test_gauge_widening_interval_threshold(Gauge(GaugeBound(-2) + GaugeBound(i),
                                               GaugeBound(4) +
                                                   GaugeBound(3, i)),
                                         Gauge(GaugeBound(2) + GaugeBound(i),
                                               GaugeBound(3) +
                                                   GaugeBound(3, i)),
                                         ZNumber(10),
                                         Gauge(GaugeBound(-2) + GaugeBound(i),
                                               GaugeBound(4) +
                                                   GaugeBound(3, i)));
  test_gauge_widening_interval_threshold(Gauge(0),
                                         Gauge(GaugeBound(2),
                                               GaugeBound::plus_infinity()),
                                         ZNumber(10),
                                         Gauge(GaugeBound(0),
                                               GaugeBound::plus_infinity()));
  test_gauge_widening_interval_threshold(Gauge(0),
                                         Gauge(GaugeBound::minus_infinity(),
                                               GaugeBound(2)),
                                         ZNumber(10),
                                         Gauge(GaugeBound::minus_infinity(),
                                               GaugeBound(10)));
  test_gauge_widening_interval_threshold(Gauge(0),
                                         Gauge::top(),
                                         ZNumber(10),
                                         Gauge::top());
  test_gauge_widening_interval_threshold(Gauge(GaugeBound::minus_infinity(),
                                               GaugeBound(0)),
                                         Gauge(1),
                                         ZNumber(10),
                                         Gauge(GaugeBound::minus_infinity(),
                                               GaugeBound(10)));
  test_gauge_widening_interval_threshold(Gauge(GaugeBound(0),
                                               GaugeBound::plus_infinity()),
                                         Gauge(1),
                                         ZNumber(10),
                                         Gauge(GaugeBound(0),
                                               GaugeBound::plus_infinity()));
  test_gauge_widening_interval_threshold(Gauge::top(),
                                         Gauge(1),
                                         ZNumber(10),
                                         Gauge::top());
}

#define test_gauge_narrowing_interval_threshold(x, y, t, z) \
  BOOST_CHECK((x).narrowing_interval_threshold(y, t) == (z))

BOOST_AUTO_TEST_CASE(gauge_narrowing_interval_threshold) {
  VariableFactory vfac;
  Variable i(vfac.get("i"));

  test_gauge_narrowing_interval_threshold(Gauge::top(),
                                          Gauge::bottom(),
                                          ZNumber(10),
                                          Gauge::bottom());
  test_gauge_narrowing_interval_threshold(Gauge::top(),
                                          Gauge::top(),
                                          ZNumber(10),
                                          Gauge::top());
  test_gauge_narrowing_interval_threshold(Gauge::top(),
                                          Gauge(GaugeBound(0), GaugeBound(1)),
                                          ZNumber(10),
                                          Gauge(GaugeBound(0), GaugeBound(1)));
  test_gauge_narrowing_interval_threshold(Gauge::top(),
                                          Gauge(GaugeBound(0),
                                                GaugeBound(2, i)),
                                          ZNumber(10),
                                          Gauge(GaugeBound(0),
                                                GaugeBound(2, i)));
  test_gauge_narrowing_interval_threshold(Gauge::bottom(),
                                          Gauge::bottom(),
                                          ZNumber(10),
                                          Gauge::bottom());
  test_gauge_narrowing_interval_threshold(Gauge::bottom(),
                                          Gauge::top(),
                                          ZNumber(10),
                                          Gauge::bottom());
  test_gauge_narrowing_interval_threshold(Gauge::bottom(),
                                          Gauge(GaugeBound(0), GaugeBound(1)),
                                          ZNumber(10),
                                          Gauge::bottom());
  test_gauge_narrowing_interval_threshold(Gauge::bottom(),
                                          Gauge(GaugeBound(0),
                                                GaugeBound(2, i)),
                                          ZNumber(10),
                                          Gauge::bottom());
  test_gauge_narrowing_interval_threshold(Gauge(0),
                                          Gauge(1),
                                          ZNumber(10),
                                          Gauge(0));
  test_gauge_narrowing_interval_threshold(Gauge(GaugeBound(0),
                                                GaugeBound::plus_infinity()),
                                          Gauge(GaugeBound(0), GaugeBound(90)),
                                          ZNumber(100),
                                          Gauge(GaugeBound(0), GaugeBound(90)));
  test_gauge_narrowing_interval_threshold(Gauge(GaugeBound(0), GaugeBound(100)),
                                          Gauge(GaugeBound(0), GaugeBound(90)),
                                          ZNumber(100),
                                          Gauge(GaugeBound(0), GaugeBound(90)));
  test_gauge_narrowing_interval_threshold(Gauge(GaugeBound(0), GaugeBound(110)),
                                          Gauge(GaugeBound(0), GaugeBound(90)),
                                          ZNumber(100),
                                          Gauge(GaugeBound(0),
                                                GaugeBound(110)));
  test_gauge_narrowing_interval_threshold(Gauge(GaugeBound::minus_infinity(),
                                                GaugeBound(0)),
                                          Gauge(GaugeBound(-90), GaugeBound(0)),
                                          ZNumber(-100),
                                          Gauge(GaugeBound(-90),
                                                GaugeBound(0)));
  test_gauge_narrowing_interval_threshold(Gauge(GaugeBound(-100),
                                                GaugeBound(0)),
                                          Gauge(GaugeBound(-90), GaugeBound(0)),
                                          ZNumber(-100),
                                          Gauge(GaugeBound(-90),
                                                GaugeBound(0)));
  test_gauge_narrowing_interval_threshold(Gauge(GaugeBound(-110),
                                                GaugeBound(0)),
                                          Gauge(GaugeBound(-90), GaugeBound(0)),
                                          ZNumber(-100),
                                          Gauge(GaugeBound(-110),
                                                GaugeBound(0)));
  test_gauge_narrowing_interval_threshold(Gauge(GaugeBound(4, i),
                                                GaugeBound::plus_infinity()),
                                          Gauge(GaugeBound(4, i),
                                                GaugeBound(90)),
                                          ZNumber(100),
                                          Gauge(GaugeBound(4, i),
                                                GaugeBound(90)));
  test_gauge_narrowing_interval_threshold(Gauge(GaugeBound(0),
                                                GaugeBound(4, i)),
                                          Gauge(GaugeBound(0),
                                                GaugeBound(3, i)),
                                          ZNumber(100),
                                          Gauge(GaugeBound(0),
                                                GaugeBound(4, i)));
  test_gauge_narrowing_interval_threshold(Gauge(GaugeBound(4, i)),
                                          Gauge(GaugeBound(3, i),
                                                GaugeBound(4, i)),
                                          ZNumber(100),
                                          Gauge(GaugeBound(4, i)));
}
