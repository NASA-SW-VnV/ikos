/*******************************************************************************
 *
 * Tests for VarPackingDBMCongruence
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

#define BOOST_TEST_MODULE test_var_packing_dbm_congruence
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/domain/numeric/var_packing_dbm_congruence.hpp>
#include <ikos/core/example/variable_factory.hpp>

using ZNumber = ikos::core::ZNumber;
using VariableFactory = ikos::core::example::VariableFactory;
using Variable = ikos::core::example::VariableFactory::VariableRef;
using VariableExpr = ikos::core::VariableExpression< ZNumber, Variable >;
using BinaryOperator = ikos::core::numeric::BinaryOperator;
using Bound = ikos::core::ZBound;
using Interval = ikos::core::numeric::ZInterval;
using Congruence = ikos::core::numeric::ZCongruence;
using IntervalCongruence = ikos::core::numeric::IntervalCongruence< ZNumber >;
using VarPackingDBMCongruence =
    ikos::core::numeric::VarPackingDBMCongruence< ZNumber, Variable >;

BOOST_AUTO_TEST_CASE(is_top_and_bottom) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  BOOST_CHECK(VarPackingDBMCongruence::top().is_top());
  BOOST_CHECK(!VarPackingDBMCongruence::top().is_bottom());

  BOOST_CHECK(!VarPackingDBMCongruence::bottom().is_top());
  BOOST_CHECK(VarPackingDBMCongruence::bottom().is_bottom());

  auto inv = VarPackingDBMCongruence::top();
  BOOST_CHECK(inv.is_top());
  BOOST_CHECK(!inv.is_bottom());

  inv.set(x, Interval(1));
  BOOST_CHECK(!inv.is_top());
  BOOST_CHECK(!inv.is_bottom());

  inv.set(x, Interval::bottom());
  BOOST_CHECK(!inv.is_top());
  BOOST_CHECK(inv.is_bottom());

  inv.set_to_top();
  inv.add(VariableExpr(x) - VariableExpr(y) <= 1);
  inv.forget(x);
  BOOST_CHECK(inv.is_top());
}

BOOST_AUTO_TEST_CASE(set_to_top_and_bottom) {
  VariableFactory vfac;

  auto inv = VarPackingDBMCongruence::top();
  BOOST_CHECK(inv.is_top());
  BOOST_CHECK(!inv.is_bottom());

  inv.set_to_bottom();
  BOOST_CHECK(!inv.is_top());
  BOOST_CHECK(inv.is_bottom());

  inv.set_to_top();
  BOOST_CHECK(inv.is_top());
  BOOST_CHECK(!inv.is_bottom());
}

BOOST_AUTO_TEST_CASE(leq) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));
  Variable a(vfac.get("a"));
  Variable b(vfac.get("b"));

  BOOST_CHECK(
      VarPackingDBMCongruence::bottom().leq(VarPackingDBMCongruence::top()));
  BOOST_CHECK(
      VarPackingDBMCongruence::bottom().leq(VarPackingDBMCongruence::bottom()));
  BOOST_CHECK(
      !VarPackingDBMCongruence::top().leq(VarPackingDBMCongruence::bottom()));
  BOOST_CHECK(
      VarPackingDBMCongruence::top().leq(VarPackingDBMCongruence::top()));

  auto inv1 = VarPackingDBMCongruence::top();
  inv1.set(x, Interval(0));
  BOOST_CHECK(inv1.leq(VarPackingDBMCongruence::top()));
  BOOST_CHECK(!inv1.leq(VarPackingDBMCongruence::bottom()));

  auto inv2 = VarPackingDBMCongruence::top();
  inv2.set(x, Interval(Bound(-1), Bound(1)));
  BOOST_CHECK(inv2.leq(VarPackingDBMCongruence::top()));
  BOOST_CHECK(!inv2.leq(VarPackingDBMCongruence::bottom()));
  BOOST_CHECK(inv1.leq(inv2));
  BOOST_CHECK(!inv2.leq(inv1));

  auto inv3 = VarPackingDBMCongruence::top();
  inv3.set(x, Interval(0));
  inv3.set(y, Interval(Bound(-1), Bound(1)));
  BOOST_CHECK(inv3.leq(VarPackingDBMCongruence::top()));
  BOOST_CHECK(!inv3.leq(VarPackingDBMCongruence::bottom()));
  BOOST_CHECK(inv3.leq(inv1));
  BOOST_CHECK(!inv1.leq(inv3));

  auto inv4 = VarPackingDBMCongruence::top();
  inv4.set(x, Interval(0));
  inv4.set(y, Interval(Bound(0), Bound(2)));
  BOOST_CHECK(inv4.leq(VarPackingDBMCongruence::top()));
  BOOST_CHECK(!inv4.leq(VarPackingDBMCongruence::bottom()));
  BOOST_CHECK(!inv3.leq(inv4));
  BOOST_CHECK(!inv4.leq(inv3));

  auto inv5 = VarPackingDBMCongruence::top();
  inv5.set(x, Interval(0));
  inv5.set(y, Interval(Bound(0), Bound(2)));
  inv5.set(z, Interval(Bound::minus_infinity(), Bound(0)));
  BOOST_CHECK(inv5.leq(VarPackingDBMCongruence::top()));
  BOOST_CHECK(!inv5.leq(VarPackingDBMCongruence::bottom()));
  BOOST_CHECK(!inv5.leq(inv3));
  BOOST_CHECK(!inv3.leq(inv5));
  BOOST_CHECK(inv5.leq(inv4));
  BOOST_CHECK(!inv4.leq(inv5));

  inv1.set_to_top();
  inv2.set_to_top();
  inv1.assign(x, 1);
  BOOST_CHECK(inv1.leq(inv2));

  inv2.add(VariableExpr(x) <= 1);
  BOOST_CHECK(inv1.leq(inv2)); // {x = 1} <= {x <= 1}

  inv2.set_to_top();
  inv2.add(VariableExpr(x) <= 0);
  BOOST_CHECK(!inv1.leq(inv2)); // not {x = 1} <= {x <= 0}

  inv1.assign(y, 2);
  inv2.set_to_top();
  inv2.add(VariableExpr(x) <= 1);
  BOOST_CHECK(inv1.leq(inv2)); // {x = 1, y = 2} <= {x <= 1}

  inv2.add(VariableExpr(z) <= 4);
  BOOST_CHECK(!inv1.leq(inv2)); // not {x = 1, y = 2} <= {x <= 1, z <= 4}

  inv1.set_to_top();
  inv2.set_to_top();

  inv1.assign(x, 1);
  inv1.add(VariableExpr(y) <= 2);
  inv1.assign(z, 3);
  inv1.add(VariableExpr(a) >= 4);
  inv1.assign(b, 5);

  inv2.add(VariableExpr(y) <= 3);
  inv2.add(VariableExpr(a) >= 1);
  inv2.assign(z, 3);
  inv2.set(x, Interval(Bound(-1), Bound(1)));

  // {x = 1, y <= 2, z = 3, a >= 4, b = 5} <= {-1 <= x <= 1, y <= 3, z = 3, a >=
  // 1}
  BOOST_CHECK(inv1.leq(inv2));

  inv2.add(VariableExpr(a) >= 5);
  // {x = 1, y <= 2, z = 3, a >= 4, b = 5} <= {-1 <= x <= 1, y <= 3, z = 3, a >=
  // 5}
  BOOST_CHECK(!inv1.leq(inv2));
}

BOOST_AUTO_TEST_CASE(equals) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  BOOST_CHECK(!VarPackingDBMCongruence::bottom().equals(
      VarPackingDBMCongruence::top()));
  BOOST_CHECK(VarPackingDBMCongruence::bottom().equals(
      VarPackingDBMCongruence::bottom()));
  BOOST_CHECK(!VarPackingDBMCongruence::top().equals(
      VarPackingDBMCongruence::bottom()));
  BOOST_CHECK(
      VarPackingDBMCongruence::top().equals(VarPackingDBMCongruence::top()));

  auto inv1 = VarPackingDBMCongruence::top();
  inv1.set(x, Interval(0));
  BOOST_CHECK(!inv1.equals(VarPackingDBMCongruence::top()));
  BOOST_CHECK(!inv1.equals(VarPackingDBMCongruence::bottom()));
  BOOST_CHECK(inv1.equals(inv1));

  auto inv2 = VarPackingDBMCongruence::top();
  inv2.set(x, Interval(Bound(-1), Bound(1)));
  BOOST_CHECK(!inv2.equals(VarPackingDBMCongruence::top()));
  BOOST_CHECK(!inv2.equals(VarPackingDBMCongruence::bottom()));
  BOOST_CHECK(!inv1.equals(inv2));
  BOOST_CHECK(!inv2.equals(inv1));

  auto inv3 = VarPackingDBMCongruence::top();
  inv3.set(x, Interval(0));
  inv3.set(y, Interval(Bound(-1), Bound(1)));
  BOOST_CHECK(!inv3.equals(VarPackingDBMCongruence::top()));
  BOOST_CHECK(!inv3.equals(VarPackingDBMCongruence::bottom()));
  BOOST_CHECK(!inv3.equals(inv1));
  BOOST_CHECK(!inv1.equals(inv3));
}

BOOST_AUTO_TEST_CASE(join) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));
  Variable a(vfac.get("a"));
  Variable b(vfac.get("b"));

  BOOST_CHECK(
      (VarPackingDBMCongruence::bottom().join(VarPackingDBMCongruence::top()) ==
       VarPackingDBMCongruence::top()));
  BOOST_CHECK((VarPackingDBMCongruence::bottom().join(
                   VarPackingDBMCongruence::bottom()) ==
               VarPackingDBMCongruence::bottom()));
  BOOST_CHECK(
      (VarPackingDBMCongruence::top().join(VarPackingDBMCongruence::top()) ==
       VarPackingDBMCongruence::top()));
  BOOST_CHECK(
      (VarPackingDBMCongruence::top().join(VarPackingDBMCongruence::bottom()) ==
       VarPackingDBMCongruence::top()));

  auto inv1 = VarPackingDBMCongruence::top();
  inv1.set(x, Interval(Bound(0), Bound(1)));
  BOOST_CHECK((inv1.join(VarPackingDBMCongruence::top()) ==
               VarPackingDBMCongruence::top()));
  BOOST_CHECK((inv1.join(VarPackingDBMCongruence::bottom()) == inv1));
  BOOST_CHECK((VarPackingDBMCongruence::top().join(inv1) ==
               VarPackingDBMCongruence::top()));
  BOOST_CHECK((VarPackingDBMCongruence::bottom().join(inv1) == inv1));
  BOOST_CHECK((inv1.join(inv1) == inv1));

  auto inv2 = VarPackingDBMCongruence::top();
  auto inv3 = VarPackingDBMCongruence::top();
  inv2.set(x, Interval(Bound(-1), Bound(0)));
  inv3.set(x, Interval(Bound(-1), Bound(1)));
  BOOST_CHECK((inv1.join(inv2) == inv3));
  BOOST_CHECK((inv2.join(inv1) == inv3));

  auto inv4 = VarPackingDBMCongruence::top();
  inv4.set(x, Interval(Bound(-1), Bound(0)));
  inv4.set(y, Interval(0));
  BOOST_CHECK((inv4.join(inv2) == inv2));
  BOOST_CHECK((inv2.join(inv4) == inv2));

  inv1.set_to_top();
  inv1.assign(x, 1);

  inv2.set_to_top();
  inv2.add(VariableExpr(x) <= 1);

  BOOST_CHECK((inv1.join(inv2) == inv2)); // {x = 1} U {x <= 1}

  inv2.set_to_top();
  inv2.add(VariableExpr(x) <= 0);

  inv3.set_to_top();
  inv3.add(VariableExpr(x) <= 1);

  BOOST_CHECK((inv1.join(inv2) == inv3)); // {x = 1} U {x <= 0}

  inv1.assign(y, 2);

  inv2.set_to_top();
  inv2.add(VariableExpr(x) <= 1);
  BOOST_CHECK((inv1.join(inv2) == inv2)); // {x = 1, y = 2} U {x <= 1}

  inv2.add(VariableExpr(z) <= 4);

  inv3.set_to_top();
  inv3.add(VariableExpr(x) <= 1);

  BOOST_CHECK((inv1.join(inv2) == inv3)); // {x = 1, y = 2} U {x <= 1, z <= 4}

  inv1.set_to_top();
  inv1.assign(x, 1);
  inv1.add(VariableExpr(y) <= 2);
  inv1.assign(z, 3);
  inv1.add(VariableExpr(a) >= 4);
  inv1.assign(b, 5);

  inv2.set_to_top();
  inv2.add(VariableExpr(y) <= 3);
  inv2.add(VariableExpr(a) >= 1);
  inv2.assign(z, 3);
  inv2.set(x, Interval(Bound(-1), Bound(1)));

  inv3.set_to_top();
  inv3.set(x, Interval(Bound(-1), Bound(1)));
  inv3.add(VariableExpr(y) <= 3);
  inv3.assign(z, 3);
  inv3.add(VariableExpr(a) >= 1);

  // {x = 1, y <= 2, z = 3, a >= 4, b = 5} U {-1 <= x <= 1, y <= 3, z = 3, a >=
  // 1}
  BOOST_CHECK((inv1.join(inv2) == inv3));

  inv2.add(VariableExpr(a) >= 5);

  // {x = 1, y <= 2, z = 3, a >= 4, b = 5} U {-1 <= x <= 1, y <= 3, z = 3, a >=
  // 5}
  BOOST_CHECK((inv1.join(inv2).to_interval(a) ==
               Interval(Bound(4), Bound::plus_infinity())));

  inv1.set_to_top();
  inv1.assign(x, 1);
  inv1.add(VariableExpr(y) <= 10);
  inv1.add((VariableExpr(z) - VariableExpr(y)) <= 10);

  inv2.set_to_top();
  inv2.assign(y, 0);
  inv2.assign(z, 0);

  inv3.set_to_top();
  inv3.add(VariableExpr(y) <= 10);
  inv3.add((VariableExpr(z) - VariableExpr(y)) <= 10);

  // {x = 1, y <= 10, z - y <= 10} U {y = 0, z = 0}
  BOOST_CHECK((inv1.join(inv2) == inv3));
}

BOOST_AUTO_TEST_CASE(widening) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  BOOST_CHECK(
      (VarPackingDBMCongruence::bottom().widening(
           VarPackingDBMCongruence::top()) == VarPackingDBMCongruence::top()));
  BOOST_CHECK((VarPackingDBMCongruence::bottom().widening(
                   VarPackingDBMCongruence::bottom()) ==
               VarPackingDBMCongruence::bottom()));
  BOOST_CHECK((
      VarPackingDBMCongruence::top().widening(VarPackingDBMCongruence::top()) ==
      VarPackingDBMCongruence::top()));
  BOOST_CHECK((VarPackingDBMCongruence::top().widening(
                   VarPackingDBMCongruence::bottom()) ==
               VarPackingDBMCongruence::top()));

  auto inv1 = VarPackingDBMCongruence::top();
  inv1.set(x, Interval(Bound(0), Bound(1)));
  BOOST_CHECK((inv1.widening(VarPackingDBMCongruence::top()) ==
               VarPackingDBMCongruence::top()));
  BOOST_CHECK((inv1.widening(VarPackingDBMCongruence::bottom()) == inv1));
  BOOST_CHECK((VarPackingDBMCongruence::top().widening(inv1) ==
               VarPackingDBMCongruence::top()));
  BOOST_CHECK((VarPackingDBMCongruence::bottom().widening(inv1) == inv1));
  BOOST_CHECK((inv1.widening(inv1) == inv1));

  auto inv2 = VarPackingDBMCongruence::top();
  auto inv3 = VarPackingDBMCongruence::top();
  inv2.set(x, Interval(Bound(0), Bound(2)));
  inv3.set(x, Interval(Bound(0), Bound::plus_infinity()));
  BOOST_CHECK((inv1.widening(inv2) == inv3));
  BOOST_CHECK((inv2.widening(inv1) == inv2));
}

BOOST_AUTO_TEST_CASE(widening_threshold) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  BOOST_CHECK(
      (VarPackingDBMCongruence::bottom()
           .widening_threshold(VarPackingDBMCongruence::top(), ZNumber(10)) ==
       VarPackingDBMCongruence::top()));
  BOOST_CHECK((
      VarPackingDBMCongruence::bottom()
          .widening_threshold(VarPackingDBMCongruence::bottom(), ZNumber(10)) ==
      VarPackingDBMCongruence::bottom()));
  BOOST_CHECK(
      (VarPackingDBMCongruence::top()
           .widening_threshold(VarPackingDBMCongruence::top(), ZNumber(10)) ==
       VarPackingDBMCongruence::top()));
  BOOST_CHECK(
      (VarPackingDBMCongruence::top()
           .widening_threshold(VarPackingDBMCongruence::bottom(),
                               ZNumber(10)) == VarPackingDBMCongruence::top()));

  auto inv1 = VarPackingDBMCongruence::top();
  inv1.set(x, Interval(Bound(0), Bound(1)));
  BOOST_CHECK(
      (inv1.widening_threshold(VarPackingDBMCongruence::top(), ZNumber(10)) ==
       VarPackingDBMCongruence::top()));
  BOOST_CHECK((inv1.widening_threshold(VarPackingDBMCongruence::bottom(),
                                       ZNumber(10)) == inv1));
  BOOST_CHECK(
      (VarPackingDBMCongruence::top().widening_threshold(inv1, ZNumber(10)) ==
       VarPackingDBMCongruence::top()));
  BOOST_CHECK((
      VarPackingDBMCongruence::bottom().widening_threshold(inv1, ZNumber(10)) ==
      inv1));
  BOOST_CHECK((inv1.widening_threshold(inv1, ZNumber(10)) == inv1));

  auto inv2 = VarPackingDBMCongruence::top();
  auto inv3 = VarPackingDBMCongruence::top();
  inv2.set(x, Interval(Bound(0), Bound(2)));
  inv3.set(x, Interval(Bound(0), Bound(10)));
  BOOST_CHECK((inv1.widening_threshold(inv2, ZNumber(10)) == inv3));
  BOOST_CHECK((inv2.widening_threshold(inv1, ZNumber(10)) == inv2));

  auto inv4 = VarPackingDBMCongruence::top();
  auto inv5 = VarPackingDBMCongruence::top();
  auto inv6 = VarPackingDBMCongruence::top();
  inv4.set(x, Interval(Bound(-1), Bound(0)));
  inv5.set(x, Interval(Bound(-2), Bound(0)));
  inv6.set(x, Interval(Bound(-10), Bound(0)));
  BOOST_CHECK((inv4.widening_threshold(inv5, ZNumber(10)) == inv6));
  BOOST_CHECK((inv5.widening_threshold(inv4, ZNumber(10)) == inv5));
}

BOOST_AUTO_TEST_CASE(narrowing_threshold) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  BOOST_CHECK(
      (VarPackingDBMCongruence::bottom()
           .narrowing_threshold(VarPackingDBMCongruence::top(), ZNumber(10)) ==
       VarPackingDBMCongruence::bottom()));
  BOOST_CHECK((VarPackingDBMCongruence::bottom()
                   .narrowing_threshold(VarPackingDBMCongruence::bottom(),
                                        ZNumber(10)) ==
               VarPackingDBMCongruence::bottom()));
  BOOST_CHECK(
      (VarPackingDBMCongruence::top()
           .narrowing_threshold(VarPackingDBMCongruence::top(), ZNumber(10)) ==
       VarPackingDBMCongruence::top()));
  BOOST_CHECK((VarPackingDBMCongruence::top()
                   .narrowing_threshold(VarPackingDBMCongruence::bottom(),
                                        ZNumber(10)) ==
               VarPackingDBMCongruence::bottom()));

  auto inv1 = VarPackingDBMCongruence::top();
  inv1.set(x, Interval(Bound(0), Bound::plus_infinity()));
  BOOST_CHECK((inv1.narrowing_threshold(VarPackingDBMCongruence::top(),
                                        ZNumber(10)) == inv1));
  BOOST_CHECK((inv1.narrowing_threshold(VarPackingDBMCongruence::bottom(),
                                        ZNumber(10)) ==
               VarPackingDBMCongruence::bottom()));
  BOOST_CHECK(
      (VarPackingDBMCongruence::top().narrowing_threshold(inv1, ZNumber(10)) ==
       inv1));
  BOOST_CHECK(
      (VarPackingDBMCongruence::bottom().narrowing_threshold(inv1,
                                                             ZNumber(10)) ==
       VarPackingDBMCongruence::bottom()));
  BOOST_CHECK((inv1.narrowing_threshold(inv1, ZNumber(10)) == inv1));

  auto inv2 = VarPackingDBMCongruence::top();
  auto inv3 = VarPackingDBMCongruence::top();
  inv2.set(x, Interval(Bound(0), Bound(1)));
  inv3.set(x, Interval(Bound(0), Bound(10)));
  BOOST_CHECK((inv1.narrowing_threshold(inv2, ZNumber(10)) == inv2));
  BOOST_CHECK((inv1.narrowing_threshold(inv3, ZNumber(10)) == inv3));
  BOOST_CHECK((inv3.narrowing_threshold(inv2, ZNumber(10)) == inv2));
  BOOST_CHECK((inv3.narrowing_threshold(inv2, ZNumber(20)) == inv3));
  BOOST_CHECK((inv3.narrowing_threshold(inv2, ZNumber(5)) == inv3));

  auto inv4 = VarPackingDBMCongruence::top();
  auto inv5 = VarPackingDBMCongruence::top();
  inv4.set(x, Interval(Bound(-10), Bound(0)));
  inv5.set(x, Interval(Bound(-1), Bound(0)));
  BOOST_CHECK((inv4.narrowing_threshold(inv5, ZNumber(10)) == inv5));
  BOOST_CHECK((inv4.narrowing_threshold(inv5, ZNumber(20)) == inv4));
  BOOST_CHECK((inv4.narrowing_threshold(inv5, ZNumber(5)) == inv4));
}

BOOST_AUTO_TEST_CASE(meet) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));
  Variable a(vfac.get("a"));
  Variable b(vfac.get("b"));

  BOOST_CHECK(
      (VarPackingDBMCongruence::bottom().meet(VarPackingDBMCongruence::top()) ==
       VarPackingDBMCongruence::bottom()));
  BOOST_CHECK((VarPackingDBMCongruence::bottom().meet(
                   VarPackingDBMCongruence::bottom()) ==
               VarPackingDBMCongruence::bottom()));
  BOOST_CHECK(
      (VarPackingDBMCongruence::top().meet(VarPackingDBMCongruence::top()) ==
       VarPackingDBMCongruence::top()));
  BOOST_CHECK(
      (VarPackingDBMCongruence::top().meet(VarPackingDBMCongruence::bottom()) ==
       VarPackingDBMCongruence::bottom()));

  auto inv1 = VarPackingDBMCongruence::top();
  inv1.set(x, Interval(Bound(0), Bound(1)));
  BOOST_CHECK((inv1.meet(VarPackingDBMCongruence::top()) == inv1));
  BOOST_CHECK((inv1.meet(VarPackingDBMCongruence::bottom()) ==
               VarPackingDBMCongruence::bottom()));
  BOOST_CHECK((VarPackingDBMCongruence::top().meet(inv1) == inv1));
  BOOST_CHECK((VarPackingDBMCongruence::bottom().meet(inv1) ==
               VarPackingDBMCongruence::bottom()));
  BOOST_CHECK((inv1.meet(inv1) == inv1));

  auto inv2 = VarPackingDBMCongruence::top();
  auto inv3 = VarPackingDBMCongruence::top();
  inv2.set(x, Interval(Bound(-1), Bound(0)));
  inv3.set(x, Interval(0));
  BOOST_CHECK((inv1.meet(inv2) == inv3));
  BOOST_CHECK((inv2.meet(inv1) == inv3));

  auto inv4 = VarPackingDBMCongruence::top();
  auto inv5 = VarPackingDBMCongruence::top();
  inv4.set(x, Interval(Bound(0), Bound(1)));
  inv4.set(y, Interval(0));
  inv5.set(x, Interval(0));
  inv5.set(y, Interval(0));
  BOOST_CHECK((inv4.meet(inv2) == inv5));
  BOOST_CHECK((inv2.meet(inv4) == inv5));

  inv1.set_to_top();
  inv1.assign(x, 1);

  inv2.set_to_top();

  BOOST_CHECK((inv1.meet(inv2) == inv1)); // {x = 1} & top()

  inv2.add(VariableExpr(x) <= 1);
  BOOST_CHECK((inv1.meet(inv2) == inv1)); // {x = 1} & {x <= 1}

  inv2.set_to_top();
  inv2.add(VariableExpr(x) <= 0);
  BOOST_CHECK((inv1.meet(inv2) ==
               VarPackingDBMCongruence::bottom())); // {x = 1} & {x <= 0}

  inv1.assign(y, 2);

  inv2.set_to_top();
  inv2.add(VariableExpr(x) <= 1);
  BOOST_CHECK((inv1.meet(inv2) == inv1)); // {x = 1, y = 2} & {x <= 1}

  inv2.add(VariableExpr(z) <= 4);

  inv3.set_to_top();
  inv3.assign(x, 1);
  inv3.assign(y, 2);
  inv3.add(VariableExpr(z) <= 4);
  BOOST_CHECK((inv1.meet(inv2) == inv3)); // {x = 1, y = 2} & {x <= 1, z <= 4}

  inv1.set_to_top();
  inv1.assign(x, 1);
  inv1.add(VariableExpr(y) <= 2);
  inv1.assign(z, 3);
  inv1.add(VariableExpr(a) >= 4);
  inv1.assign(b, 5);

  inv2.set_to_top();
  inv2.add(VariableExpr(y) <= 3);
  inv2.add(VariableExpr(a) >= 1);
  inv2.assign(z, 3);
  inv2.set(x, Interval(Bound(-1), Bound(1)));

  inv3.set_to_top();
  inv3.assign(x, 1);
  inv3.add(VariableExpr(y) <= 2);
  inv3.assign(z, 3);
  inv3.add(VariableExpr(a) >= 4);
  inv3.assign(b, 5);

  // {x = 1, y <= 2, z = 3, a >= 4, b = 5} & {-1 <= x <= 1, y <= 3, z = 3, a >=
  // 1}
  BOOST_CHECK((inv1.meet(inv2) == inv3));

  inv2.add(VariableExpr(a) >= 5);
  inv3.add(VariableExpr(a) >= 5);

  // {x = 1, y <= 2, z = 3, a >= 4, b = 5} & {-1 <= x <= 1, y <= 3, z = 3, a >=
  // 5}
  BOOST_CHECK((inv1.meet(inv2) == inv3));
}

BOOST_AUTO_TEST_CASE(narrowing) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  BOOST_CHECK((VarPackingDBMCongruence::bottom().narrowing(
                   VarPackingDBMCongruence::top()) ==
               VarPackingDBMCongruence::bottom()));
  BOOST_CHECK((VarPackingDBMCongruence::bottom().narrowing(
                   VarPackingDBMCongruence::bottom()) ==
               VarPackingDBMCongruence::bottom()));
  BOOST_CHECK(
      (VarPackingDBMCongruence::top().narrowing(
           VarPackingDBMCongruence::top()) == VarPackingDBMCongruence::top()));
  BOOST_CHECK((VarPackingDBMCongruence::top().narrowing(
                   VarPackingDBMCongruence::bottom()) ==
               VarPackingDBMCongruence::bottom()));

  auto inv1 = VarPackingDBMCongruence::top();
  inv1.set(x, Interval(Bound(0), Bound::plus_infinity()));
  BOOST_CHECK((inv1.narrowing(VarPackingDBMCongruence::top()) == inv1));
  BOOST_CHECK((inv1.narrowing(VarPackingDBMCongruence::bottom()) ==
               VarPackingDBMCongruence::bottom()));
  BOOST_CHECK((VarPackingDBMCongruence::top().narrowing(inv1) == inv1));
  BOOST_CHECK((VarPackingDBMCongruence::bottom().narrowing(inv1) ==
               VarPackingDBMCongruence::bottom()));
  BOOST_CHECK((inv1.narrowing(inv1) == inv1));

  auto inv2 = VarPackingDBMCongruence::top();
  auto inv3 = VarPackingDBMCongruence::top();
  inv2.set(x, Interval(Bound(0), Bound(1)));
  BOOST_CHECK((inv1.narrowing(inv2) == inv2));
  BOOST_CHECK((inv2.narrowing(inv1) == inv2));
}

BOOST_AUTO_TEST_CASE(assign) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));

  auto inv1 = VarPackingDBMCongruence::top();
  auto inv2 = VarPackingDBMCongruence::top();
  inv1.assign(x, 0);
  inv2.set(x, Interval(0));
  BOOST_CHECK((inv1 == inv2));

  inv1.set_to_bottom();
  inv1.assign(x, 0);
  BOOST_CHECK(inv1.is_bottom());

  inv1.set_to_top();
  inv1.set(x, Interval(Bound(-1), Bound(1)));
  inv1.assign(y, x);
  BOOST_CHECK(inv1.to_interval(y) == Interval(Bound(-1), Bound(1)));

  inv1.set_to_top();
  inv1.set(x, Interval(Bound(-1), Bound(1)));
  inv1.set(y, Interval(Bound(1), Bound(2)));
  inv1.assign(z, 2 * VariableExpr(x) - 3 * VariableExpr(y) + 1);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(-7), Bound(0)));

  inv1.set_to_top();
  inv1.assign(x, 7);
  inv1.add(VariableExpr(y) <= 3);
  inv1.add(VariableExpr(y) >= 1);
  inv1.assign(z, VariableExpr(x) + 2 * VariableExpr(y) + 1);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(10), Bound(14)));
}

BOOST_AUTO_TEST_CASE(apply) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));

  auto inv1 = VarPackingDBMCongruence::top();
  auto inv2 = VarPackingDBMCongruence::top();
  inv1.set(x,
           IntervalCongruence(Interval(Bound(-2), Bound(4)),
                              Congruence(ZNumber(2), ZNumber(0))));
  inv1.set(y,
           IntervalCongruence(Interval(Bound(1), Bound(4)),
                              Congruence(ZNumber(3), ZNumber(1))));

  inv1.apply(BinaryOperator::Add, z, x, y);
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(-1), Bound(8)), Congruence::top()));

  inv1.apply(BinaryOperator::Sub, z, x, y);
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(-6), Bound(3)), Congruence::top()));

  inv1.apply(BinaryOperator::Mul, z, x, y);
  BOOST_CHECK(inv1.to_interval_congruence(z) ==
              IntervalCongruence(Interval(Bound(-8), Bound(16)),
                                 Congruence(ZNumber(2), ZNumber(0))));

  inv1.apply(BinaryOperator::Div, z, x, y);
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(-2), Bound(4)), Congruence::top()));

  inv1.apply(BinaryOperator::Rem, z, x, y);
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(-3), Bound(3)), Congruence::top()));

  inv1.apply(BinaryOperator::Mod, z, x, y);
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(0), Bound(3)), Congruence::top()));

  inv1.apply(BinaryOperator::Shl, z, x, y);
  BOOST_CHECK(inv1.to_interval_congruence(z) ==
              IntervalCongruence(Interval(Bound(-32), Bound(64)),
                                 Congruence(ZNumber(4), ZNumber(0))));

  inv1.apply(BinaryOperator::Shr, z, x, y);
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(-1), Bound(2)), Congruence::top()));

  inv1.apply(BinaryOperator::And, z, x, y);
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(0), Bound(4)), Congruence::top()));

  inv1.apply(BinaryOperator::Or, z, x, y);
  BOOST_CHECK(inv1.to_interval_congruence(z) == IntervalCongruence::top());

  inv1.apply(BinaryOperator::Xor, z, x, y);
  BOOST_CHECK(inv1.to_interval_congruence(z) == IntervalCongruence::top());

  inv1.apply(BinaryOperator::Add, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_interval_congruence(z) ==
              IntervalCongruence(Interval(Bound(1), Bound(7)),
                                 Congruence(ZNumber(2), ZNumber(1))));

  inv1.apply(BinaryOperator::Sub, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_interval_congruence(z) ==
              IntervalCongruence(Interval(Bound(-5), Bound(1)),
                                 Congruence(ZNumber(2), ZNumber(1))));

  inv1.apply(BinaryOperator::Mul, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_interval_congruence(z) ==
              IntervalCongruence(Interval(Bound(-6), Bound(12)),
                                 Congruence(ZNumber(6), ZNumber(0))));

  inv1.apply(BinaryOperator::Div, z, x, ZNumber(3));
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(0), Bound(1)), Congruence::top()));

  inv1.apply(BinaryOperator::Rem, z, x, ZNumber(3));
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(-2), Bound(2)), Congruence::top()));

  inv1.apply(BinaryOperator::Mod, z, x, ZNumber(3));
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(0), Bound(2)), Congruence::top()));

  inv1.apply(BinaryOperator::Shl, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_interval_congruence(z) ==
              IntervalCongruence(Interval(Bound(-16), Bound(32)),
                                 Congruence(ZNumber(16), ZNumber(0))));

  inv1.apply(BinaryOperator::Shr, z, x, ZNumber(3));
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(-1), Bound(0)), Congruence::top()));

  inv1.apply(BinaryOperator::And, z, x, ZNumber(3));
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(0), Bound(3)), Congruence::top()));

  inv1.apply(BinaryOperator::Or, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_interval_congruence(z) == IntervalCongruence::top());

  inv1.apply(BinaryOperator::Xor, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_interval_congruence(z) == IntervalCongruence::top());

  inv1.apply(BinaryOperator::Add, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_interval_congruence(z) ==
              IntervalCongruence(Interval(Bound(5), Bound(8)),
                                 Congruence(ZNumber(3), ZNumber(2))));

  inv1.apply(BinaryOperator::Sub, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_interval_congruence(z) ==
              IntervalCongruence(Interval(Bound(0), Bound(3)),
                                 Congruence(ZNumber(3), ZNumber(0))));

  inv1.apply(BinaryOperator::Mul, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_interval_congruence(z) ==
              IntervalCongruence(Interval(Bound(4), Bound(16)),
                                 Congruence(ZNumber(12), ZNumber(4))));

  inv1.apply(BinaryOperator::Div, z, ZNumber(4), y);
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(1), Bound(4)), Congruence::top()));

  inv1.apply(BinaryOperator::Rem, z, ZNumber(4), y);
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(0), Bound(3)), Congruence::top()));

  inv1.apply(BinaryOperator::Mod, z, ZNumber(4), y);
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(0), Bound(3)), Congruence::top()));

  inv1.apply(BinaryOperator::Shl, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_interval_congruence(z) ==
              IntervalCongruence(Interval(Bound(8), Bound(64)),
                                 Congruence(ZNumber(56), ZNumber(8))));

  inv1.apply(BinaryOperator::Shr, z, ZNumber(4), y);
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(0), Bound(2)), Congruence::top()));

  inv1.apply(BinaryOperator::And, z, ZNumber(4), y);
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(0), Bound(4)), Congruence::top()));

  inv1.apply(BinaryOperator::Or, z, ZNumber(4), y);
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(0), Bound(7)), Congruence::top()));

  inv1.apply(BinaryOperator::Xor, z, ZNumber(4), y);
  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Bound(0), Bound(7)), Congruence::top()));
}

BOOST_AUTO_TEST_CASE(add) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));
  Variable w(vfac.get("w"));

  auto inv = VarPackingDBMCongruence::top();
  inv.add(VariableExpr(x) >= 1);
  BOOST_CHECK(inv.to_interval(x) == Interval(Bound(1), Bound::plus_infinity()));

  inv.add(VariableExpr(y) >= VariableExpr(x) + 2);
  BOOST_CHECK(inv.to_interval(x) == Interval(Bound(1), Bound::plus_infinity()));
  BOOST_CHECK(inv.to_interval(y) == Interval(Bound(3), Bound::plus_infinity()));

  inv.add(2 * VariableExpr(x) + 3 * VariableExpr(y) <= VariableExpr(z));
  BOOST_CHECK(inv.to_interval(x) == Interval(Bound(1), Bound::plus_infinity()));
  BOOST_CHECK(inv.to_interval(y) == Interval(Bound(3), Bound::plus_infinity()));
  BOOST_CHECK(inv.to_interval(z) ==
              Interval(Bound(11), Bound::plus_infinity()));

  inv.add(2 * VariableExpr(z) <= 4 * VariableExpr(y));
  BOOST_CHECK(inv.to_interval(x) == Interval(Bound(1), Bound::plus_infinity()));
  BOOST_CHECK(inv.to_interval(y) == Interval(Bound(5), Bound::plus_infinity()));
  BOOST_CHECK(inv.to_interval(z) ==
              Interval(Bound(11), Bound::plus_infinity()));

  inv.add(VariableExpr(z) + VariableExpr(x) <= 20);
  BOOST_CHECK(inv.to_interval(x) == Interval(Bound(1), Bound(9)));
  BOOST_CHECK(inv.to_interval(y) == Interval(Bound(5), Bound::plus_infinity()));
  BOOST_CHECK(inv.to_interval(z) == Interval(Bound(11), Bound(19)));

  inv.add(3 * VariableExpr(y) <= VariableExpr(z));
  BOOST_CHECK(inv.to_interval(x) == Interval(Bound(1), Bound(4)));
  BOOST_CHECK(inv.to_interval(y) == Interval(Bound(5), Bound(6)));
  BOOST_CHECK(inv.to_interval(z) == Interval(Bound(15), Bound(19)));

  inv.add(VariableExpr(x) == VariableExpr(y));
  BOOST_CHECK(inv.is_bottom());

  inv.set_to_top();
  inv.assign(x, 1);
  inv.add(VariableExpr(x) + VariableExpr(y) >= 0);
  inv.add(VariableExpr(x) - VariableExpr(y) >= 3);
  BOOST_CHECK(inv.is_bottom());

  inv.set_to_top();
  inv.assign(y, VariableExpr(x) + 1);
  inv.assign(z, VariableExpr(y) + 1);
  inv.add(VariableExpr(x) <= 7);
  inv.add(VariableExpr(x) >= 1);
  BOOST_CHECK(inv.to_interval(x) == Interval(Bound(1), Bound(7)));
  BOOST_CHECK(inv.to_interval(y) == Interval(Bound(2), Bound(8)));
  BOOST_CHECK(inv.to_interval(z) == Interval(Bound(3), Bound(9)));

  inv.add(VariableExpr(x) == 4 * VariableExpr(w));
  BOOST_CHECK(inv.to_interval(x) == Interval(4));
  BOOST_CHECK(inv.to_interval(y) == Interval(5));
  BOOST_CHECK(inv.to_interval(z) == Interval(6));
}

BOOST_AUTO_TEST_CASE(set) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  auto inv = VarPackingDBMCongruence::top();
  inv.set(x, Interval(Bound(1), Bound(2)));
  BOOST_CHECK(inv.to_interval(x) == Interval(Bound(1), Bound(2)));

  inv.set(x, Interval::bottom());
  BOOST_CHECK(inv.is_bottom());

  inv.set_to_top();
  inv.set(x, Congruence(1));
  BOOST_CHECK(inv.to_interval(x) == Interval(1));

  inv.set_to_top();
  inv.set(x, Congruence(ZNumber(3), ZNumber(1)));
  BOOST_CHECK(inv.to_interval(x) == Interval::top());

  inv.set_to_top();
  inv.set(x,
          IntervalCongruence(Interval(Bound(1), Bound(2)),
                             Congruence(ZNumber(3), ZNumber(1))));
  BOOST_CHECK(inv.to_interval_congruence(x) ==
              IntervalCongruence(Interval(Bound(1), Bound(2)),
                                 Congruence(ZNumber(3), ZNumber(1))));
}

BOOST_AUTO_TEST_CASE(refine) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  auto inv = VarPackingDBMCongruence::top();
  inv.refine(x, Interval(Bound(1), Bound(2)));
  BOOST_CHECK(inv.to_interval(x) == Interval(Bound(1), Bound(2)));

  inv.refine(x, Interval(Bound(3), Bound(4)));
  BOOST_CHECK(inv.is_bottom());

  inv.set_to_top();
  inv.refine(x, Congruence(1));
  BOOST_CHECK(inv.to_interval(x) == Interval(1));

  inv.set_to_top();
  inv.refine(x, Congruence(ZNumber(3), ZNumber(1)));
  BOOST_CHECK(inv.to_interval(x) == Interval::top());

  inv.set_to_top();
  inv.refine(x, Interval(Bound(2), Bound(9)));
  inv.refine(x, Congruence(ZNumber(3), ZNumber(1)));
  BOOST_CHECK(inv.to_interval(x) == Interval(Bound(4), Bound(7)));

  inv.set_to_top();
  inv.refine(x, IntervalCongruence(1));
  BOOST_CHECK(inv.to_interval_congruence(x) == IntervalCongruence(1));

  inv.set_to_top();
  inv.refine(x, Congruence(ZNumber(3), ZNumber(1)));
  BOOST_CHECK(inv.to_interval_congruence(x) ==
              IntervalCongruence(Congruence(ZNumber(3), ZNumber(1))));

  inv.set_to_top();
  inv.refine(x, Interval(Bound(2), Bound(9)));
  inv.refine(x, Congruence(ZNumber(3), ZNumber(1)));
  BOOST_CHECK(inv.to_interval_congruence(x) ==
              IntervalCongruence(Interval(Bound(4), Bound(7)),
                                 Congruence(ZNumber(3), ZNumber(1))));
}

BOOST_AUTO_TEST_CASE(forget) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto inv = VarPackingDBMCongruence::top();
  inv.set(x, Interval(Bound(1), Bound(2)));
  inv.set(y, Interval(Bound(3), Bound(4)));
  BOOST_CHECK(inv.to_interval(x) == Interval(Bound(1), Bound(2)));
  BOOST_CHECK(inv.to_interval(y) == Interval(Bound(3), Bound(4)));

  inv.forget(x);
  BOOST_CHECK(inv.to_interval(x) == Interval::top());
  BOOST_CHECK(inv.to_interval(y) == Interval(Bound(3), Bound(4)));

  inv.forget(y);
  BOOST_CHECK(inv.is_top());

  inv.set_to_top();
  inv.add(VariableExpr(x) - VariableExpr(y) <= 1);
  inv.forget(x);
  BOOST_CHECK(inv.is_top());
}

BOOST_AUTO_TEST_CASE(to_interval) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto inv = VarPackingDBMCongruence::top();
  inv.set(x, Interval(Bound(1), Bound(2)));
  inv.set(y, Interval(Bound(3), Bound(4)));
  BOOST_CHECK(inv.to_interval(2 * VariableExpr(x) + 1) ==
              Interval(Bound(3), Bound(5)));
  BOOST_CHECK(inv.to_interval(2 * VariableExpr(x) - 3 * VariableExpr(y) + 1) ==
              Interval(Bound(-9), Bound(-4)));
}

BOOST_AUTO_TEST_CASE(to_congruence) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto inv = VarPackingDBMCongruence::top();
  inv.set(x, Interval(Bound(1), Bound(2)));
  inv.set(y, Interval(Bound(3), Bound(4)));
  BOOST_CHECK(inv.to_congruence(2 * VariableExpr(x) + 1) ==
              Congruence(ZNumber(2), ZNumber(1)));
  BOOST_CHECK(inv.to_congruence(2 * VariableExpr(x) - 3 * VariableExpr(y) +
                                1) == Congruence::top());
}

BOOST_AUTO_TEST_CASE(to_interval_congruence) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto inv = VarPackingDBMCongruence::top();
  inv.refine(x, Interval(Bound(0), Bound(3)));
  inv.refine(y, Interval(Bound(4), Bound(7)));
  inv.refine(x, Congruence(ZNumber(3), ZNumber(0)));
  inv.refine(y, Congruence(ZNumber(3), ZNumber(1)));
  BOOST_CHECK(inv.to_interval_congruence(2 * VariableExpr(x) + 1) ==
              IntervalCongruence(Interval(Bound(1), Bound(7)),
                                 Congruence(ZNumber(6), ZNumber(1))));
  BOOST_CHECK(inv.to_interval_congruence(2 * VariableExpr(x) -
                                         3 * VariableExpr(y) + 1) ==
              IntervalCongruence(Interval(Bound(-20), Bound(-5)),
                                 Congruence(ZNumber(3), ZNumber(1))));
}
