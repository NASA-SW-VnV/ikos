/*******************************************************************************
 *
 * Tests for ConstantDomain
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

#include <ikos/core/domain/numeric/constant.hpp>
#include <ikos/core/example/variable_factory.hpp>

using ZNumber = ikos::core::ZNumber;
using VariableFactory = ikos::core::example::VariableFactory;
using Variable = ikos::core::example::VariableFactory::VariableRef;
using VariableExpr = ikos::core::VariableExpression< ZNumber, Variable >;
using BinaryOperator = ikos::core::numeric::BinaryOperator;
using Constant = ikos::core::numeric::ZConstant;
using Bound = ikos::core::ZBound;
using Interval = ikos::core::numeric::ZInterval;
using Congruence = ikos::core::numeric::ZCongruence;
using IntervalCongruence = ikos::core::numeric::IntervalCongruence< ZNumber >;
using ConstantDomain = ikos::core::numeric::ConstantDomain< ZNumber, Variable >;

BOOST_AUTO_TEST_CASE(is_top_and_bottom) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  BOOST_CHECK(ConstantDomain::top().is_top());
  BOOST_CHECK(!ConstantDomain::top().is_bottom());

  BOOST_CHECK(!ConstantDomain::bottom().is_top());
  BOOST_CHECK(ConstantDomain::bottom().is_bottom());

  auto inv = ConstantDomain::top();
  BOOST_CHECK(inv.is_top());
  BOOST_CHECK(!inv.is_bottom());

  inv.set(x, Constant(1));
  BOOST_CHECK(!inv.is_top());
  BOOST_CHECK(!inv.is_bottom());

  inv.set(x, Constant::bottom());
  BOOST_CHECK(!inv.is_top());
  BOOST_CHECK(inv.is_bottom());
}

BOOST_AUTO_TEST_CASE(iterators) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto inv = ConstantDomain::top();
  BOOST_CHECK((inv.begin() == inv.end()));

  inv.set(x, Constant(1));
  std::array< std::pair< Variable, Constant >, 1 > tab = {{{x, Constant(1)}}};
  BOOST_CHECK(
      std::equal(inv.begin(), inv.end(), std::begin(tab), std::end(tab)));

  inv.set(y, Constant(2));
  std::array< std::pair< Variable, Constant >, 2 > tab2 = {{
      {y, Constant(2)},
      {x, Constant(1)},
  }};
  BOOST_CHECK(
      std::equal(inv.begin(), inv.end(), std::begin(tab2), std::end(tab2)));
}

BOOST_AUTO_TEST_CASE(set_to_top_and_bottom) {
  VariableFactory vfac;

  auto inv = ConstantDomain::top();
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

  BOOST_CHECK(ConstantDomain::bottom().leq(ConstantDomain::top()));
  BOOST_CHECK(ConstantDomain::bottom().leq(ConstantDomain::bottom()));
  BOOST_CHECK(!ConstantDomain::top().leq(ConstantDomain::bottom()));
  BOOST_CHECK(ConstantDomain::top().leq(ConstantDomain::top()));

  auto inv1 = ConstantDomain::top();
  inv1.set(x, Constant(0));
  BOOST_CHECK(inv1.leq(ConstantDomain::top()));
  BOOST_CHECK(!inv1.leq(ConstantDomain::bottom()));

  auto inv2 = ConstantDomain::top();
  inv2.set(x, Constant(1));
  BOOST_CHECK(inv2.leq(ConstantDomain::top()));
  BOOST_CHECK(!inv2.leq(ConstantDomain::bottom()));
  BOOST_CHECK(!inv1.leq(inv2));
  BOOST_CHECK(!inv2.leq(inv1));

  auto inv3 = ConstantDomain::top();
  inv3.set(x, Constant(0));
  inv3.set(y, Constant(1));
  BOOST_CHECK(inv3.leq(ConstantDomain::top()));
  BOOST_CHECK(!inv3.leq(ConstantDomain::bottom()));
  BOOST_CHECK(inv3.leq(inv1));
  BOOST_CHECK(!inv1.leq(inv3));

  auto inv4 = ConstantDomain::top();
  inv4.set(x, Constant(0));
  inv4.set(y, Constant(2));
  BOOST_CHECK(inv4.leq(ConstantDomain::top()));
  BOOST_CHECK(!inv4.leq(ConstantDomain::bottom()));
  BOOST_CHECK(!inv3.leq(inv4));
  BOOST_CHECK(!inv4.leq(inv3));
}

BOOST_AUTO_TEST_CASE(equals) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  BOOST_CHECK(!ConstantDomain::bottom().equals(ConstantDomain::top()));
  BOOST_CHECK(ConstantDomain::bottom().equals(ConstantDomain::bottom()));
  BOOST_CHECK(!ConstantDomain::top().equals(ConstantDomain::bottom()));
  BOOST_CHECK(ConstantDomain::top().equals(ConstantDomain::top()));

  auto inv1 = ConstantDomain::top();
  inv1.set(x, Constant(0));
  BOOST_CHECK(!inv1.equals(ConstantDomain::top()));
  BOOST_CHECK(!inv1.equals(ConstantDomain::bottom()));
  BOOST_CHECK(inv1.equals(inv1));

  auto inv2 = ConstantDomain::top();
  inv2.set(x, Constant(1));
  BOOST_CHECK(!inv2.equals(ConstantDomain::top()));
  BOOST_CHECK(!inv2.equals(ConstantDomain::bottom()));
  BOOST_CHECK(!inv1.equals(inv2));
  BOOST_CHECK(!inv2.equals(inv1));

  auto inv3 = ConstantDomain::top();
  inv3.set(x, Constant(0));
  inv3.set(y, Constant(1));
  BOOST_CHECK(!inv3.equals(ConstantDomain::top()));
  BOOST_CHECK(!inv3.equals(ConstantDomain::bottom()));
  BOOST_CHECK(!inv3.equals(inv1));
  BOOST_CHECK(!inv1.equals(inv3));
}

BOOST_AUTO_TEST_CASE(join) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  BOOST_CHECK((ConstantDomain::bottom().join(ConstantDomain::top()) ==
               ConstantDomain::top()));
  BOOST_CHECK((ConstantDomain::bottom().join(ConstantDomain::bottom()) ==
               ConstantDomain::bottom()));
  BOOST_CHECK((ConstantDomain::top().join(ConstantDomain::top()) ==
               ConstantDomain::top()));
  BOOST_CHECK((ConstantDomain::top().join(ConstantDomain::bottom()) ==
               ConstantDomain::top()));

  auto inv1 = ConstantDomain::top();
  inv1.set(x, Constant(0));
  BOOST_CHECK((inv1.join(ConstantDomain::top()) == ConstantDomain::top()));
  BOOST_CHECK((inv1.join(ConstantDomain::bottom()) == inv1));
  BOOST_CHECK((ConstantDomain::top().join(inv1) == ConstantDomain::top()));
  BOOST_CHECK((ConstantDomain::bottom().join(inv1) == inv1));
  BOOST_CHECK((inv1.join(inv1) == inv1));

  auto inv2 = ConstantDomain::top();
  inv2.set(x, Constant(-1));
  BOOST_CHECK((inv1.join(inv2) == ConstantDomain::top()));
  BOOST_CHECK((inv2.join(inv1) == ConstantDomain::top()));

  auto inv4 = ConstantDomain::top();
  inv4.set(x, Constant(-1));
  inv4.set(y, Constant(0));
  BOOST_CHECK((inv4.join(inv2) == inv2));
  BOOST_CHECK((inv2.join(inv4) == inv2));
}

BOOST_AUTO_TEST_CASE(widening) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  BOOST_CHECK((ConstantDomain::bottom().widening(ConstantDomain::top()) ==
               ConstantDomain::top()));
  BOOST_CHECK((ConstantDomain::bottom().widening(ConstantDomain::bottom()) ==
               ConstantDomain::bottom()));
  BOOST_CHECK((ConstantDomain::top().widening(ConstantDomain::top()) ==
               ConstantDomain::top()));
  BOOST_CHECK((ConstantDomain::top().widening(ConstantDomain::bottom()) ==
               ConstantDomain::top()));

  auto inv1 = ConstantDomain::top();
  inv1.set(x, Constant(0));
  BOOST_CHECK((inv1.widening(ConstantDomain::top()) == ConstantDomain::top()));
  BOOST_CHECK((inv1.widening(ConstantDomain::bottom()) == inv1));
  BOOST_CHECK((ConstantDomain::top().widening(inv1) == ConstantDomain::top()));
  BOOST_CHECK((ConstantDomain::bottom().widening(inv1) == inv1));
  BOOST_CHECK((inv1.widening(inv1) == inv1));

  auto inv2 = ConstantDomain::top();
  inv2.set(x, Constant(-1));
  BOOST_CHECK((inv1.widening(inv2) == ConstantDomain::top()));
  BOOST_CHECK((inv2.widening(inv1) == ConstantDomain::top()));

  auto inv4 = ConstantDomain::top();
  inv4.set(x, Constant(-1));
  inv4.set(y, Constant(0));
  BOOST_CHECK((inv4.widening(inv2) == inv2));
  BOOST_CHECK((inv2.widening(inv4) == inv2));
}

BOOST_AUTO_TEST_CASE(meet) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  BOOST_CHECK((ConstantDomain::bottom().meet(ConstantDomain::top()) ==
               ConstantDomain::bottom()));
  BOOST_CHECK((ConstantDomain::bottom().meet(ConstantDomain::bottom()) ==
               ConstantDomain::bottom()));
  BOOST_CHECK((ConstantDomain::top().meet(ConstantDomain::top()) ==
               ConstantDomain::top()));
  BOOST_CHECK((ConstantDomain::top().meet(ConstantDomain::bottom()) ==
               ConstantDomain::bottom()));

  auto inv1 = ConstantDomain::top();
  inv1.set(x, Constant(0));
  BOOST_CHECK((inv1.meet(ConstantDomain::top()) == inv1));
  BOOST_CHECK(
      (inv1.meet(ConstantDomain::bottom()) == ConstantDomain::bottom()));
  BOOST_CHECK((ConstantDomain::top().meet(inv1) == inv1));
  BOOST_CHECK(
      (ConstantDomain::bottom().meet(inv1) == ConstantDomain::bottom()));
  BOOST_CHECK((inv1.meet(inv1) == inv1));

  auto inv2 = ConstantDomain::top();
  inv2.set(x, Constant(-1));
  BOOST_CHECK((inv1.meet(inv2) == ConstantDomain::bottom()));
  BOOST_CHECK((inv2.meet(inv1) == ConstantDomain::bottom()));

  auto inv4 = ConstantDomain::top();
  inv4.set(x, Constant(-1));
  inv4.set(y, Constant(0));
  BOOST_CHECK((inv4.meet(inv2) == inv4));
  BOOST_CHECK((inv2.meet(inv4) == inv4));
}

BOOST_AUTO_TEST_CASE(narrowing) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  BOOST_CHECK((ConstantDomain::bottom().narrowing(ConstantDomain::top()) ==
               ConstantDomain::bottom()));
  BOOST_CHECK((ConstantDomain::bottom().narrowing(ConstantDomain::bottom()) ==
               ConstantDomain::bottom()));
  BOOST_CHECK((ConstantDomain::top().narrowing(ConstantDomain::top()) ==
               ConstantDomain::top()));
  BOOST_CHECK((ConstantDomain::top().narrowing(ConstantDomain::bottom()) ==
               ConstantDomain::bottom()));

  auto inv1 = ConstantDomain::top();
  inv1.set(x, Constant(0));
  BOOST_CHECK((inv1.narrowing(ConstantDomain::top()) == inv1));
  BOOST_CHECK(
      (inv1.narrowing(ConstantDomain::bottom()) == ConstantDomain::bottom()));
  BOOST_CHECK((ConstantDomain::top().narrowing(inv1) == inv1));
  BOOST_CHECK(
      (ConstantDomain::bottom().narrowing(inv1) == ConstantDomain::bottom()));
  BOOST_CHECK((inv1.narrowing(inv1) == inv1));

  auto inv2 = ConstantDomain::top();
  inv2.set(x, Constant(-1));
  BOOST_CHECK((inv1.narrowing(inv2) == ConstantDomain::bottom()));
  BOOST_CHECK((inv2.narrowing(inv1) == ConstantDomain::bottom()));

  auto inv4 = ConstantDomain::top();
  inv4.set(x, Constant(-1));
  inv4.set(y, Constant(0));
  BOOST_CHECK((inv4.narrowing(inv2) == inv4));
  BOOST_CHECK((inv2.narrowing(inv4) == inv4));
}

BOOST_AUTO_TEST_CASE(assign) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));

  auto inv1 = ConstantDomain::top();
  auto inv2 = ConstantDomain::top();
  inv1.assign(x, 0);
  inv2.set(x, Constant(0));
  BOOST_CHECK((inv1 == inv2));

  inv1.set_to_bottom();
  inv1.assign(x, 0);
  BOOST_CHECK(inv1.is_bottom());

  inv1.set_to_top();
  inv1.set(x, Constant(1));
  inv1.assign(y, x);
  BOOST_CHECK(inv1.to_constant(y) == Constant(1));

  inv1.set_to_top();
  inv1.set(x, Constant(1));
  inv1.set(y, Constant(2));
  inv1.assign(z, 2 * VariableExpr(x) - 3 * VariableExpr(y) + 1);
  BOOST_CHECK(inv1.to_constant(z) == Constant(-3));
}

BOOST_AUTO_TEST_CASE(apply) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));

  auto inv1 = ConstantDomain::top();
  auto inv2 = ConstantDomain::top();
  inv1.set(x, Constant(2));
  inv1.set(y, Constant(3));

  inv1.apply(BinaryOperator::Add, z, x, y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(5));

  inv1.apply(BinaryOperator::Sub, z, x, y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(-1));

  inv1.apply(BinaryOperator::Mul, z, x, y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(6));

  inv1.apply(BinaryOperator::Div, z, x, y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(0));

  inv1.apply(BinaryOperator::Rem, z, x, y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(2));

  inv1.apply(BinaryOperator::Mod, z, x, y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(2));

  inv1.apply(BinaryOperator::Shl, z, x, y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(16));

  inv1.apply(BinaryOperator::Shr, z, x, y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(0));

  inv1.apply(BinaryOperator::And, z, x, y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(2));

  inv1.apply(BinaryOperator::Or, z, x, y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(3));

  inv1.apply(BinaryOperator::Xor, z, x, y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(1));

  inv1.apply(BinaryOperator::Add, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_constant(z) == Constant(5));

  inv1.apply(BinaryOperator::Sub, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_constant(z) == Constant(-1));

  inv1.apply(BinaryOperator::Mul, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_constant(z) == Constant(6));

  inv1.apply(BinaryOperator::Div, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_constant(z) == Constant(0));

  inv1.apply(BinaryOperator::Rem, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_constant(z) == Constant(2));

  inv1.apply(BinaryOperator::Mod, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_constant(z) == Constant(2));

  inv1.apply(BinaryOperator::Shl, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_constant(z) == Constant(16));

  inv1.apply(BinaryOperator::Shr, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_constant(z) == Constant(0));

  inv1.apply(BinaryOperator::And, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_constant(z) == Constant(2));

  inv1.apply(BinaryOperator::Or, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_constant(z) == Constant(3));

  inv1.apply(BinaryOperator::Xor, z, x, ZNumber(3));
  BOOST_CHECK(inv1.to_constant(z) == Constant(1));

  inv1.apply(BinaryOperator::Add, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(7));

  inv1.apply(BinaryOperator::Sub, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(1));

  inv1.apply(BinaryOperator::Mul, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(12));

  inv1.apply(BinaryOperator::Div, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(1));

  inv1.apply(BinaryOperator::Rem, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(1));

  inv1.apply(BinaryOperator::Mod, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(1));

  inv1.apply(BinaryOperator::Shl, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(32));

  inv1.apply(BinaryOperator::Shr, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(0));

  inv1.apply(BinaryOperator::And, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(0));

  inv1.apply(BinaryOperator::Or, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(7));

  inv1.apply(BinaryOperator::Xor, z, ZNumber(4), y);
  BOOST_CHECK(inv1.to_constant(z) == Constant(7));
}

BOOST_AUTO_TEST_CASE(add) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  auto inv = ConstantDomain::top();
  inv.add(VariableExpr(x) == 1);
  BOOST_CHECK(inv.to_constant(x) == Constant(1));
}

BOOST_AUTO_TEST_CASE(set) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  auto inv = ConstantDomain::top();
  inv.set(x, Constant(1));
  BOOST_CHECK(inv.to_constant(x) == Constant(1));

  inv.set(x, Interval(Bound(1), Bound(2)));
  BOOST_CHECK(inv.to_constant(x) == Constant::top());

  inv.set(x, Interval::bottom());
  BOOST_CHECK(inv.is_bottom());

  inv.set_to_top();
  inv.set(x, Congruence(1));
  BOOST_CHECK(inv.to_constant(x) == Constant(1));

  inv.set_to_top();
  inv.set(x, Congruence(ZNumber(3), ZNumber(1)));
  BOOST_CHECK(inv.to_constant(x) == Constant::top());

  inv.set_to_top();
  inv.set(x, IntervalCongruence(1));
  BOOST_CHECK(inv.to_constant(x) == Constant(1));

  inv.set_to_top();
  inv.set(x,
          IntervalCongruence(Interval(Bound(1), Bound(4)),
                             Congruence(ZNumber(3), ZNumber(1))));
  BOOST_CHECK(inv.to_constant(x) == Constant::top());
}

BOOST_AUTO_TEST_CASE(refine) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  auto inv = ConstantDomain::top();
  inv.refine(x, Interval(Bound(1), Bound(2)));
  BOOST_CHECK(inv.is_top());

  inv.refine(x, Interval(Bound(3), Bound(4)));
  BOOST_CHECK(inv.is_top());

  inv.set_to_top();
  inv.refine(x, Congruence(1));
  BOOST_CHECK(inv.to_constant(x) == Constant(1));

  inv.set_to_top();
  inv.refine(x, Congruence(ZNumber(3), ZNumber(1)));
  BOOST_CHECK(inv.to_constant(x) == Constant::top());

  inv.set_to_top();
  inv.refine(x, IntervalCongruence(1));
  BOOST_CHECK(inv.to_constant(x) == Constant(1));

  inv.set_to_top();
  inv.refine(x,
             IntervalCongruence(Interval(Bound(1), Bound(4)),
                                Congruence(ZNumber(3), ZNumber(1))));
  BOOST_CHECK(inv.to_constant(x) == Constant::top());
}

BOOST_AUTO_TEST_CASE(forget) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto inv = ConstantDomain::top();
  inv.set(x, Constant(1));
  inv.set(y, Constant(3));
  BOOST_CHECK(inv.to_constant(x) == Constant(1));
  BOOST_CHECK(inv.to_constant(y) == Constant(3));

  inv.forget(x);
  BOOST_CHECK(inv.to_constant(x) == Constant::top());
  BOOST_CHECK(inv.to_constant(y) == Constant(3));

  inv.forget(y);
  BOOST_CHECK(inv.is_top());
}

BOOST_AUTO_TEST_CASE(to_interval) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto inv = ConstantDomain::top();
  inv.set(x, Constant(1));
  inv.set(y, Constant(3));
  BOOST_CHECK(inv.to_interval(2 * VariableExpr(x) + 1) == Interval(3));
  BOOST_CHECK(inv.to_interval(2 * VariableExpr(x) - 3 * VariableExpr(y) + 1) ==
              Interval(-6));
}

BOOST_AUTO_TEST_CASE(to_congruence) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto inv = ConstantDomain::top();
  inv.set(x, Constant(1));
  inv.set(y, Constant(3));
  BOOST_CHECK(inv.to_congruence(2 * VariableExpr(x) + 1) == Congruence(3));
  BOOST_CHECK(inv.to_congruence(2 * VariableExpr(x) - 3 * VariableExpr(y) +
                                1) == Congruence(-6));
}

BOOST_AUTO_TEST_CASE(to_interval_congruence) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto inv = ConstantDomain::top();
  inv.set(x, Constant(1));
  inv.set(y, Constant(3));
  BOOST_CHECK(inv.to_interval_congruence(2 * VariableExpr(x) + 1) ==
              IntervalCongruence(3));
  BOOST_CHECK(inv.to_interval_congruence(2 * VariableExpr(x) -
                                         3 * VariableExpr(y) + 1) ==
              IntervalCongruence(-6));
}
