/*******************************************************************************
 *
 * Tests for UnionDomain
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

#define BOOST_TEST_MODULE test_interval_domain
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/domain/numeric/interval.hpp>
#include <ikos/core/domain/numeric/union.hpp>
#include <ikos/core/example/variable_factory.hpp>
#include <ikos/core/number/z_number.hpp>

using ZNumber = ikos::core::ZNumber;
using VariableFactory = ikos::core::example::VariableFactory;
using Variable = ikos::core::example::VariableFactory::VariableRef;
using VariableExpr = ikos::core::VariableExpression< ZNumber, Variable >;
using BinaryOperator = ikos::core::numeric::BinaryOperator;
using Bound = ikos::core::ZBound;
using Interval = ikos::core::numeric::ZInterval;
using Congruence = ikos::core::numeric::ZCongruence;
using IntervalCongruence = ikos::core::numeric::IntervalCongruence< ZNumber >;
using IntervalDomain = ikos::core::numeric::IntervalDomain< ZNumber, Variable >;
using UnionDomain =
    ikos::core::numeric::UnionDomain< ZNumber, Variable, IntervalDomain, 2 >;

BOOST_AUTO_TEST_CASE(is_top_and_bottom) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  BOOST_CHECK(UnionDomain::top().is_top());
  BOOST_CHECK(!UnionDomain::top().is_bottom());

  BOOST_CHECK(!UnionDomain::bottom().is_top());
  BOOST_CHECK(UnionDomain::bottom().is_bottom());

  auto inv = UnionDomain::top();
  BOOST_CHECK(inv.is_top());
  BOOST_CHECK(!inv.is_bottom());

  inv.assign(x, 1);
  BOOST_CHECK(!inv.is_top());
  BOOST_CHECK(!inv.is_bottom());

  inv.set(x, Interval::bottom());
  BOOST_CHECK(!inv.is_top());
  BOOST_CHECK(inv.is_bottom());
}

BOOST_AUTO_TEST_CASE(set_to_top_and_bottom) {
  VariableFactory vfac;

  auto inv = UnionDomain::top();
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

  BOOST_CHECK(UnionDomain::bottom().leq(UnionDomain::top()));
  BOOST_CHECK(UnionDomain::bottom().leq(UnionDomain::bottom()));
  BOOST_CHECK(!UnionDomain::top().leq(UnionDomain::bottom()));
  BOOST_CHECK(UnionDomain::top().leq(UnionDomain::top()));

  auto inv_a = UnionDomain::top();
  auto inv_b = UnionDomain::top();
  inv_a.assign(x, 1);
  BOOST_CHECK(inv_a.leq(inv_b));

  inv_b.add(VariableExpr(x) <= 1);
  BOOST_CHECK(inv_a.leq(inv_b));

  inv_b.set_to_top();
  inv_b.add(VariableExpr(x) <= 0);
  BOOST_CHECK(!inv_a.leq(inv_b));

  inv_a.assign(y, 2);
  inv_b.set_to_top();
  inv_b.add(VariableExpr(x) <= 1);
  BOOST_CHECK(inv_a.leq(inv_b));

  inv_b.add(VariableExpr(z) <= 4);
  BOOST_CHECK(!inv_a.leq(inv_b));

  inv_a.set_to_top();
  inv_b.set_to_top();

  inv_a.assign(x, 1);
  inv_a.add(VariableExpr(y) <= 2);
  inv_a.assign(z, 3);
  inv_a.add(VariableExpr(a) >= 4);
  inv_a.assign(b, 5);

  inv_b.add(VariableExpr(y) <= 3);
  inv_b.add(VariableExpr(a) >= 1);
  inv_b.assign(z, 3);
  inv_b.set(x, Interval(Bound(-1), Bound(1)));
  BOOST_CHECK(inv_a.leq(inv_b));

  inv_b.add(VariableExpr(a) >= 5);
  BOOST_CHECK(!inv_a.leq(inv_b));
}

BOOST_AUTO_TEST_CASE(equals) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  BOOST_CHECK(!UnionDomain::bottom().equals(UnionDomain::top()));
  BOOST_CHECK(UnionDomain::bottom().equals(UnionDomain::bottom()));
  BOOST_CHECK(!UnionDomain::top().equals(UnionDomain::bottom()));
  BOOST_CHECK(UnionDomain::top().equals(UnionDomain::top()));

  auto inv1 = UnionDomain::top();
  inv1.set(x, Interval(0));
  BOOST_CHECK(!inv1.equals(UnionDomain::top()));
  BOOST_CHECK(!inv1.equals(UnionDomain::bottom()));
  BOOST_CHECK(inv1.equals(inv1));

  auto inv2 = UnionDomain::top();
  inv2.set(x, Interval(Bound(-1), Bound(1)));
  BOOST_CHECK(!inv2.equals(UnionDomain::top()));
  BOOST_CHECK(!inv2.equals(UnionDomain::bottom()));
  BOOST_CHECK(!inv1.equals(inv2));
  BOOST_CHECK(!inv2.equals(inv1));

  auto inv3 = UnionDomain::top();
  inv3.set(x, Interval(0));
  inv3.set(y, Interval(Bound(-1), Bound(1)));
  BOOST_CHECK(!inv3.equals(UnionDomain::top()));
  BOOST_CHECK(!inv3.equals(UnionDomain::bottom()));
  BOOST_CHECK(!inv3.equals(inv1));
  BOOST_CHECK(!inv1.equals(inv3));
}

BOOST_AUTO_TEST_CASE(join) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  BOOST_CHECK(
      (UnionDomain::bottom().join(UnionDomain::top()) == UnionDomain::top()));
  BOOST_CHECK((UnionDomain::bottom().join(UnionDomain::bottom()) ==
               UnionDomain::bottom()));
  BOOST_CHECK(
      (UnionDomain::top().join(UnionDomain::top()) == UnionDomain::top()));
  BOOST_CHECK(
      (UnionDomain::top().join(UnionDomain::bottom()) == UnionDomain::top()));

  auto inv_a = UnionDomain::top();
  auto inv_b = UnionDomain::top();
  auto inv_c = UnionDomain::top();

  inv_a.set_to_top();
  inv_a.assign(x, 0);
  inv_b.set_to_top();
  inv_b.assign(x, 2);
  inv_c = inv_a.join(inv_b);
  // {x = 0} U {x = 2}

  {
    UnionDomain inv = inv_c;
    inv.add(VariableExpr(x) == 1);
    BOOST_CHECK(inv.is_bottom());
  }

  inv_a.set_to_top();
  inv_a.assign(x, 4);
  inv_c = inv_c.join(inv_a);
  // ({x = 0} U {x = 2}) U {x = 4}

  {
    UnionDomain inv = inv_c;
    inv.add(VariableExpr(x) == 1);
    BOOST_CHECK(inv.is_bottom());
  }
  {
    UnionDomain inv = inv_c;
    inv.add(VariableExpr(x) == 3);
    BOOST_CHECK(inv.is_bottom());
  }

  inv_a.set_to_top();
  inv_a.assign(x, 6);
  inv_b.set_to_top();
  inv_b.assign(x, 8);
  inv_a = inv_a.join(inv_b);

  inv_b.set_to_top();
  inv_b.assign(x, 10);

  inv_a = inv_b.join(inv_a);

  inv_c = inv_c.join(inv_a);
  // "(({x = 0} U {x = 2}) U {x = 4}) U ({x = 10} U ({x = 6} U {x = 8}))"
  BOOST_CHECK(inv_c.to_interval(x) == Interval(Bound(0), Bound(10)));
  {
    UnionDomain inv = inv_c;
    inv.add(VariableExpr(x) == 1);
    BOOST_CHECK(!inv.is_bottom());
  }
  {
    UnionDomain inv = inv_c;
    inv.add(VariableExpr(x) == 3);
    BOOST_CHECK(inv.is_bottom());
  }
  {
    UnionDomain inv = inv_c;
    inv.add(VariableExpr(x) == 5);
    BOOST_CHECK(inv.is_bottom());
  }
  {
    UnionDomain inv = inv_c;
    inv.add(VariableExpr(x) == 7);
    BOOST_CHECK(!inv.is_bottom());
  }

  inv_c.add(VariableExpr(x) >= 5);
  BOOST_CHECK(inv_c.to_interval(x) == Interval(Bound(6), Bound(10)));
  {
    UnionDomain inv = inv_c;
    inv.add(VariableExpr(x) == 7);
    BOOST_CHECK(!inv.is_bottom());
  }
  {
    UnionDomain inv = inv_c;
    inv.add(VariableExpr(x) == 9);
    BOOST_CHECK(inv.is_bottom());
  }

  inv_c.add(VariableExpr(x) <= 7);
  BOOST_CHECK(inv_c.to_interval(x) == Interval(Bound(6), Bound(7)));

  inv_c.add(VariableExpr(x) >= 8);
  BOOST_CHECK(inv_c.is_bottom());
}

BOOST_AUTO_TEST_CASE(widening) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  BOOST_CHECK((UnionDomain::bottom().widening(UnionDomain::top()) ==
               UnionDomain::top()));
  BOOST_CHECK((UnionDomain::bottom().widening(UnionDomain::bottom()) ==
               UnionDomain::bottom()));
  BOOST_CHECK(
      (UnionDomain::top().widening(UnionDomain::top()) == UnionDomain::top()));
  BOOST_CHECK((UnionDomain::top().widening(UnionDomain::bottom()) ==
               UnionDomain::top()));

  auto inv1 = UnionDomain::top();
  inv1.set(x, Interval(Bound(0), Bound(1)));
  BOOST_CHECK((inv1.widening(UnionDomain::top()) == UnionDomain::top()));
  BOOST_CHECK((inv1.widening(UnionDomain::bottom()) == inv1));
  BOOST_CHECK((UnionDomain::top().widening(inv1) == UnionDomain::top()));
  BOOST_CHECK((UnionDomain::bottom().widening(inv1) == inv1));
  BOOST_CHECK((inv1.widening(inv1) == inv1));

  auto inv2 = UnionDomain::top();
  auto inv3 = UnionDomain::top();
  inv2.set(x, Interval(Bound(0), Bound(2)));
  inv3.set(x, Interval(Bound(0), Bound::plus_infinity()));
  BOOST_CHECK((inv1.widening(inv2) == inv3));
  BOOST_CHECK((inv2.widening(inv1) == inv2));
}

BOOST_AUTO_TEST_CASE(meet) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  BOOST_CHECK((UnionDomain::bottom().meet(UnionDomain::top()) ==
               UnionDomain::bottom()));
  BOOST_CHECK((UnionDomain::bottom().meet(UnionDomain::bottom()) ==
               UnionDomain::bottom()));
  BOOST_CHECK(
      (UnionDomain::top().meet(UnionDomain::top()) == UnionDomain::top()));
  BOOST_CHECK((UnionDomain::top().meet(UnionDomain::bottom()) ==
               UnionDomain::bottom()));

  auto inv_a = UnionDomain::top();
  auto inv_b = UnionDomain::top();
  inv_a.assign(x, 0);
  inv_b.assign(x, 10);
  inv_a.join_with(inv_b);
  inv_b.set_to_top();
  inv_b.add(VariableExpr(x) <= 5);
  UnionDomain inv = inv_a.meet(inv_b);
  BOOST_CHECK(inv.to_interval(x) == Interval(0));

  auto inv1 = UnionDomain::top();
  inv1.set(x, Interval(Bound(0), Bound(1)));
  BOOST_CHECK((inv1.meet(UnionDomain::top()) == inv1));
  BOOST_CHECK((inv1.meet(UnionDomain::bottom()) == UnionDomain::bottom()));
  BOOST_CHECK((UnionDomain::top().meet(inv1) == inv1));
  BOOST_CHECK((UnionDomain::bottom().meet(inv1) == UnionDomain::bottom()));
  BOOST_CHECK((inv1.meet(inv1) == inv1));

  auto inv2 = UnionDomain::top();
  auto inv3 = UnionDomain::top();
  inv2.set(x, Interval(Bound(-1), Bound(0)));
  inv3.set(x, Interval(0));
  BOOST_CHECK((inv1.meet(inv2) == inv3));
  BOOST_CHECK((inv2.meet(inv1) == inv3));

  auto inv4 = UnionDomain::top();
  auto inv5 = UnionDomain::top();
  inv4.set(x, Interval(Bound(0), Bound(1)));
  inv4.set(y, Interval(0));
  inv5.set(x, Interval(0));
  inv5.set(y, Interval(0));
  BOOST_CHECK((inv4.meet(inv2) == inv5));
  BOOST_CHECK((inv2.meet(inv4) == inv5));
}

BOOST_AUTO_TEST_CASE(narrowing) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  BOOST_CHECK((UnionDomain::bottom().narrowing(UnionDomain::top()) ==
               UnionDomain::bottom()));
  BOOST_CHECK((UnionDomain::bottom().narrowing(UnionDomain::bottom()) ==
               UnionDomain::bottom()));
  BOOST_CHECK(
      (UnionDomain::top().narrowing(UnionDomain::top()) == UnionDomain::top()));
  BOOST_CHECK((UnionDomain::top().narrowing(UnionDomain::bottom()) ==
               UnionDomain::bottom()));

  auto inv1 = UnionDomain::top();
  inv1.set(x, Interval(Bound(0), Bound::plus_infinity()));
  BOOST_CHECK((inv1.narrowing(UnionDomain::top()) == inv1));
  BOOST_CHECK((inv1.narrowing(UnionDomain::bottom()) == UnionDomain::bottom()));
  BOOST_CHECK((UnionDomain::top().narrowing(inv1) == inv1));
  BOOST_CHECK((UnionDomain::bottom().narrowing(inv1) == UnionDomain::bottom()));
  BOOST_CHECK((inv1.narrowing(inv1) == inv1));

  auto inv2 = UnionDomain::top();
  auto inv3 = UnionDomain::top();
  inv2.set(x, Interval(Bound(0), Bound(1)));
  BOOST_CHECK((inv1.narrowing(inv2) == inv2));
  BOOST_CHECK((inv2.narrowing(inv1) == inv2));
}

BOOST_AUTO_TEST_CASE(assign) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));

  auto inv1 = UnionDomain::top();
  auto inv2 = UnionDomain::top();
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
}

BOOST_AUTO_TEST_CASE(apply) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));

  auto inv1 = UnionDomain::top();
  auto inv2 = UnionDomain::top();
  inv1.set(x, Interval(Bound(-1), Bound(1)));
  inv1.set(y, Interval(Bound(1), Bound(2)));

  inv1.apply(BinaryOperator::Add, z, x, y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(0), Bound(3)));

  inv1.apply(BinaryOperator::Sub, z, x, y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(-3), Bound(0)));

  inv1.apply(BinaryOperator::Mul, z, x, y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(-2), Bound(2)));

  inv1.apply(BinaryOperator::Div, z, x, y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(-1), Bound(1)));

  inv1.apply(BinaryOperator::Rem, z, x, y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(-1), Bound(1)));

  inv1.apply(BinaryOperator::Mod, z, x, y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(0), Bound(1)));

  inv1.apply(BinaryOperator::Shl, z, x, y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(-4), Bound(4)));

  inv1.apply(BinaryOperator::Shr, z, x, y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(-1), Bound(0)));

  inv1.apply(BinaryOperator::And, z, x, y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(0), Bound(2)));

  inv1.apply(BinaryOperator::Or, z, x, y);
  BOOST_CHECK(inv1.to_interval(z) == Interval::top());

  inv1.apply(BinaryOperator::Xor, z, x, y);
  BOOST_CHECK(inv1.to_interval(z) == Interval::top());

  inv1.apply(BinaryOperator::Add, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(2), Bound(4)));

  inv1.apply(BinaryOperator::Sub, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(-4), Bound(-2)));

  inv1.apply(BinaryOperator::Mul, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(-3), Bound(3)));

  inv1.apply(BinaryOperator::Div, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(0), Bound(0)));

  inv1.apply(BinaryOperator::Rem, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(-1), Bound(1)));

  inv1.apply(BinaryOperator::Mod, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(0), Bound(2)));

  inv1.apply(BinaryOperator::Shl, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(-8), Bound(8)));

  inv1.apply(BinaryOperator::Shr, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(-1), Bound(0)));

  inv1.apply(BinaryOperator::And, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(0), Bound(3)));

  inv1.apply(BinaryOperator::Or, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_interval(z) == Interval::top());

  inv1.apply(BinaryOperator::Xor, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_interval(z) == Interval::top());

  inv1.apply(BinaryOperator::Add, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(5), Bound(6)));

  inv1.apply(BinaryOperator::Sub, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(2), Bound(3)));

  inv1.apply(BinaryOperator::Mul, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(4), Bound(8)));

  inv1.apply(BinaryOperator::Div, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(2), Bound(4)));

  inv1.apply(BinaryOperator::Rem, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(0), Bound(1)));

  inv1.apply(BinaryOperator::Mod, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(0), Bound(1)));

  inv1.apply(BinaryOperator::Shl, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(8), Bound(16)));

  inv1.apply(BinaryOperator::Shr, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(1), Bound(2)));

  inv1.apply(BinaryOperator::And, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(0), Bound(2)));

  inv1.apply(BinaryOperator::Or, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(0), Bound(7)));

  inv1.apply(BinaryOperator::Xor, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_interval(z) == Interval(Bound(0), Bound(7)));
}

BOOST_AUTO_TEST_CASE(add) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));

  auto inv = UnionDomain::top();
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
  BOOST_CHECK(inv.to_interval(x) == Interval(Bound(1), Bound(9)));
  BOOST_CHECK(inv.to_interval(y) == Interval(Bound(5), Bound(6)));
  BOOST_CHECK(inv.to_interval(z) == Interval(Bound(15), Bound(19)));

  inv.add(VariableExpr(x) == VariableExpr(y));
  BOOST_CHECK(inv.to_interval(x) == Interval(Bound(5), Bound(6)));
  BOOST_CHECK(inv.to_interval(y) == Interval(Bound(5), Bound(6)));
  BOOST_CHECK(inv.to_interval(z) == Interval(Bound(15), Bound(19)));

  inv.add(VariableExpr(x) >= VariableExpr(z));
  BOOST_CHECK(inv.is_bottom());
}

BOOST_AUTO_TEST_CASE(set) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));
  Variable w(vfac.get("w"));

  auto inv_a = UnionDomain::top();
  auto inv_b = UnionDomain::top();
  inv_a.assign(x, 0);
  inv_b.assign(x, 2);
  UnionDomain inv = inv_a.join(inv_b);
  inv.set(y, Interval(Bound(0), Bound(4)));
  inv.set(z, Interval(Bound::minus_infinity(), Bound(42)));
  inv.set(w, Interval(Bound(42), Bound::plus_infinity()));
  BOOST_CHECK(inv.to_interval(x) == Interval(Bound(0), Bound(2)));
  BOOST_CHECK(inv.to_interval(y) == Interval(Bound(0), Bound(4)));
  BOOST_CHECK(inv.to_interval(z) ==
              Interval(Bound::minus_infinity(), Bound(42)));
  BOOST_CHECK(inv.to_interval(w) ==
              Interval(Bound(42), Bound::plus_infinity()));
}

BOOST_AUTO_TEST_CASE(refine) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  auto inv = UnionDomain::top();
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
}

BOOST_AUTO_TEST_CASE(forget) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto inv = UnionDomain::top();
  inv.set(x, Interval(Bound(1), Bound(2)));
  inv.set(y, Interval(Bound(3), Bound(4)));
  BOOST_CHECK(inv.to_interval(x) == Interval(Bound(1), Bound(2)));
  BOOST_CHECK(inv.to_interval(y) == Interval(Bound(3), Bound(4)));

  inv.forget(x);
  BOOST_CHECK(inv.to_interval(x) == Interval::top());
  BOOST_CHECK(inv.to_interval(y) == Interval(Bound(3), Bound(4)));

  inv.forget(y);
  BOOST_CHECK(inv.is_top());
}

BOOST_AUTO_TEST_CASE(to_interval) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto inv = UnionDomain::top();
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

  auto inv = UnionDomain::top();
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

  auto inv = UnionDomain::top();
  inv.set(x, Interval(Bound(1), Bound(2)));
  inv.set(y, Interval(Bound(3), Bound(4)));
  BOOST_CHECK(inv.to_interval_congruence(2 * VariableExpr(x) + 1) ==
              IntervalCongruence(Interval(Bound(3), Bound(5)),
                                 Congruence(ZNumber(2), ZNumber(1))));
  BOOST_CHECK(inv.to_interval_congruence(2 * VariableExpr(x) -
                                         3 * VariableExpr(y) + 1) ==
              IntervalCongruence(Interval(Bound(-9), Bound(-4))));
}
