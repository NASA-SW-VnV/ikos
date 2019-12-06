/*******************************************************************************
 *
 * Tests for machine_int::IntervalCongruenceDomain
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

#define BOOST_TEST_MODULE test_machine_int_interval_congruence_domain
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/domain/machine_int/interval_congruence.hpp>
#include <ikos/core/example/machine_int/variable_factory.hpp>

using Int = ikos::core::MachineInt;
using Interval = ikos::core::machine_int::Interval;
using Congruence = ikos::core::machine_int::Congruence;
using IntervalCongruence = ikos::core::machine_int::IntervalCongruence;
using ikos::core::Signed;
using ikos::core::Unsigned;
using ikos::core::machine_int::BinaryOperator;
using ikos::core::machine_int::Predicate;
using ikos::core::machine_int::UnaryOperator;
using VariableFactory = ikos::core::example::machine_int::VariableFactory;
using Variable = VariableFactory::VariableRef;
using VariableExpr = ikos::core::VariableExpression< Int, Variable >;
using LinearExpr = ikos::core::LinearExpression< Int, Variable >;
using IntervalCongruenceDomain =
    ikos::core::machine_int::IntervalCongruenceDomain< Variable >;

BOOST_AUTO_TEST_CASE(is_top_and_bottom) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));

  BOOST_CHECK(IntervalCongruenceDomain::top().is_top());
  BOOST_CHECK(!IntervalCongruenceDomain::top().is_bottom());

  BOOST_CHECK(!IntervalCongruenceDomain::bottom().is_top());
  BOOST_CHECK(IntervalCongruenceDomain::bottom().is_bottom());

  auto inv = IntervalCongruenceDomain::top();
  BOOST_CHECK(inv.is_top());
  BOOST_CHECK(!inv.is_bottom());

  inv.set(x, IntervalCongruence(Int(1, 32, Signed)));
  BOOST_CHECK(!inv.is_top());
  BOOST_CHECK(!inv.is_bottom());

  inv.set(x, IntervalCongruence::bottom(32, Signed));
  BOOST_CHECK(!inv.is_top());
  BOOST_CHECK(inv.is_bottom());
}

BOOST_AUTO_TEST_CASE(set_to_top_and_bottom) {
  VariableFactory vfac;

  auto inv = IntervalCongruenceDomain::top();
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
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));
  Variable z(vfac.get("z", 32, Signed));

  BOOST_CHECK(
      IntervalCongruenceDomain::bottom().leq(IntervalCongruenceDomain::top()));
  BOOST_CHECK(IntervalCongruenceDomain::bottom().leq(
      IntervalCongruenceDomain::bottom()));
  BOOST_CHECK(
      !IntervalCongruenceDomain::top().leq(IntervalCongruenceDomain::bottom()));
  BOOST_CHECK(
      IntervalCongruenceDomain::top().leq(IntervalCongruenceDomain::top()));

  auto inv1 = IntervalCongruenceDomain::top();
  inv1.set(x, IntervalCongruence(Int(0, 32, Signed)));
  BOOST_CHECK(inv1.leq(IntervalCongruenceDomain::top()));
  BOOST_CHECK(!inv1.leq(IntervalCongruenceDomain::bottom()));

  auto inv2 = IntervalCongruenceDomain::top();
  inv2.set(x,
           IntervalCongruence(Interval(Int(0, 32, Signed), Int(10, 32, Signed)),
                              Congruence(Int(2, 32, Signed),
                                         Int(0, 32, Signed))));
  BOOST_CHECK(inv2.leq(IntervalCongruenceDomain::top()));
  BOOST_CHECK(!inv2.leq(IntervalCongruenceDomain::bottom()));
  BOOST_CHECK(inv1.leq(inv2));
  BOOST_CHECK(!inv2.leq(inv1));

  auto inv3 = IntervalCongruenceDomain::top();
  inv3.set(x, IntervalCongruence(Int(0, 32, Signed)));
  inv3.set(y,
           IntervalCongruence(Interval::top(32, Signed),
                              Congruence(Int(2, 32, Signed),
                                         Int(0, 32, Signed))));
  BOOST_CHECK(inv3.leq(IntervalCongruenceDomain::top()));
  BOOST_CHECK(!inv3.leq(IntervalCongruenceDomain::bottom()));
  BOOST_CHECK(inv3.leq(inv1));
  BOOST_CHECK(!inv1.leq(inv3));

  auto inv4 = IntervalCongruenceDomain::top();
  inv4.set(x, IntervalCongruence(Int(0, 32, Signed)));
  inv4.set(y,
           IntervalCongruence(Interval::top(32, Signed),
                              Congruence(Int(2, 32, Signed),
                                         Int(1, 32, Signed))));
  BOOST_CHECK(inv4.leq(IntervalCongruenceDomain::top()));
  BOOST_CHECK(!inv4.leq(IntervalCongruenceDomain::bottom()));
  BOOST_CHECK(!inv3.leq(inv4));
  BOOST_CHECK(!inv4.leq(inv3));

  auto inv5 = IntervalCongruenceDomain::top();
  inv5.set(x, IntervalCongruence(Int(0, 32, Signed)));
  inv5.set(y,
           IntervalCongruence(Interval(Int(0, 32, Signed), Int(4, 32, Signed)),
                              Congruence(Int(2, 32, Signed),
                                         Int(1, 32, Signed))));
  inv5.set(z,
           IntervalCongruence(
               Congruence(Int(3, 32, Signed), Int(0, 32, Signed))));
  BOOST_CHECK(inv5.leq(IntervalCongruenceDomain::top()));
  BOOST_CHECK(!inv5.leq(IntervalCongruenceDomain::bottom()));
  BOOST_CHECK(!inv5.leq(inv3));
  BOOST_CHECK(!inv3.leq(inv5));
  BOOST_CHECK(inv5.leq(inv4));
  BOOST_CHECK(!inv4.leq(inv5));
}

BOOST_AUTO_TEST_CASE(equals) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));

  BOOST_CHECK(!IntervalCongruenceDomain::bottom().equals(
      IntervalCongruenceDomain::top()));
  BOOST_CHECK(IntervalCongruenceDomain::bottom().equals(
      IntervalCongruenceDomain::bottom()));
  BOOST_CHECK(!IntervalCongruenceDomain::top().equals(
      IntervalCongruenceDomain::bottom()));
  BOOST_CHECK(
      IntervalCongruenceDomain::top().equals(IntervalCongruenceDomain::top()));

  auto inv1 = IntervalCongruenceDomain::top();
  inv1.set(x, IntervalCongruence(Int(0, 32, Signed)));
  BOOST_CHECK(!inv1.equals(IntervalCongruenceDomain::top()));
  BOOST_CHECK(!inv1.equals(IntervalCongruenceDomain::bottom()));
  BOOST_CHECK(inv1.equals(inv1));

  auto inv2 = IntervalCongruenceDomain::top();
  inv2.set(x,
           IntervalCongruence(
               Congruence(Int(2, 32, Signed), Int(0, 32, Signed))));
  BOOST_CHECK(!inv2.equals(IntervalCongruenceDomain::top()));
  BOOST_CHECK(!inv2.equals(IntervalCongruenceDomain::bottom()));
  BOOST_CHECK(!inv1.equals(inv2));
  BOOST_CHECK(!inv2.equals(inv1));

  auto inv3 = IntervalCongruenceDomain::top();
  inv3.set(x, IntervalCongruence(Int(0, 32, Signed)));
  inv3.set(y,
           IntervalCongruence(
               Interval(Int(0, 32, Signed), Int(2, 32, Signed))));
  BOOST_CHECK(!inv3.equals(IntervalCongruenceDomain::top()));
  BOOST_CHECK(!inv3.equals(IntervalCongruenceDomain::bottom()));
  BOOST_CHECK(!inv3.equals(inv1));
  BOOST_CHECK(!inv1.equals(inv3));
}

BOOST_AUTO_TEST_CASE(join) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));

  BOOST_CHECK((IntervalCongruenceDomain::bottom().join(
                   IntervalCongruenceDomain::top()) ==
               IntervalCongruenceDomain::top()));
  BOOST_CHECK((IntervalCongruenceDomain::bottom().join(
                   IntervalCongruenceDomain::bottom()) ==
               IntervalCongruenceDomain::bottom()));
  BOOST_CHECK(
      (IntervalCongruenceDomain::top().join(IntervalCongruenceDomain::top()) ==
       IntervalCongruenceDomain::top()));
  BOOST_CHECK((IntervalCongruenceDomain::top().join(
                   IntervalCongruenceDomain::bottom()) ==
               IntervalCongruenceDomain::top()));

  auto inv1 = IntervalCongruenceDomain::top();
  inv1.set(x, IntervalCongruence(Int(1, 32, Signed)));
  BOOST_CHECK((inv1.join(IntervalCongruenceDomain::top()) ==
               IntervalCongruenceDomain::top()));
  BOOST_CHECK((inv1.join(IntervalCongruenceDomain::bottom()) == inv1));
  BOOST_CHECK((IntervalCongruenceDomain::top().join(inv1) ==
               IntervalCongruenceDomain::top()));
  BOOST_CHECK((IntervalCongruenceDomain::bottom().join(inv1) == inv1));
  BOOST_CHECK((inv1.join(inv1) == inv1));

  auto inv2 = IntervalCongruenceDomain::top();
  auto inv3 = IntervalCongruenceDomain::top();
  inv2.set(x, IntervalCongruence(Int(3, 32, Signed)));
  inv3.set(x,
           IntervalCongruence(Interval(Int(0, 32, Signed), Int(3, 32, Signed)),
                              Congruence(Int(2, 32, Signed),
                                         Int(1, 32, Signed))));
  BOOST_CHECK((inv1.join(inv2) == inv3));
  BOOST_CHECK((inv2.join(inv1) == inv3));

  auto inv4 = IntervalCongruenceDomain::top();
  inv4.set(x, IntervalCongruence(Int(3, 32, Signed)));
  inv4.set(y,
           IntervalCongruence(
               Interval(Int(1, 32, Signed), Int(4, 32, Signed))));
  BOOST_CHECK((inv4.join(inv2) == inv2));
  BOOST_CHECK((inv2.join(inv4) == inv2));
}

BOOST_AUTO_TEST_CASE(widening) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));

  BOOST_CHECK((IntervalCongruenceDomain::bottom().widening(
                   IntervalCongruenceDomain::top()) ==
               IntervalCongruenceDomain::top()));
  BOOST_CHECK((IntervalCongruenceDomain::bottom().widening(
                   IntervalCongruenceDomain::bottom()) ==
               IntervalCongruenceDomain::bottom()));
  BOOST_CHECK((IntervalCongruenceDomain::top().widening(
                   IntervalCongruenceDomain::top()) ==
               IntervalCongruenceDomain::top()));
  BOOST_CHECK((IntervalCongruenceDomain::top().widening(
                   IntervalCongruenceDomain::bottom()) ==
               IntervalCongruenceDomain::top()));

  auto inv1 = IntervalCongruenceDomain::top();
  inv1.set(x, IntervalCongruence(Int(1, 32, Signed)));
  BOOST_CHECK((inv1.widening(IntervalCongruenceDomain::top()) ==
               IntervalCongruenceDomain::top()));
  BOOST_CHECK((inv1.widening(IntervalCongruenceDomain::bottom()) == inv1));
  BOOST_CHECK((IntervalCongruenceDomain::top().widening(inv1) ==
               IntervalCongruenceDomain::top()));
  BOOST_CHECK((IntervalCongruenceDomain::bottom().widening(inv1) == inv1));
  BOOST_CHECK((inv1.widening(inv1) == inv1));

  auto inv2 = IntervalCongruenceDomain::top();
  auto inv3 = IntervalCongruenceDomain::top();
  inv2.set(x, IntervalCongruence(Int(3, 32, Signed)));
  inv3.set(x,
           IntervalCongruence(Interval(Int(0, 32, Signed),
                                       Int::max(32, Signed)),
                              Congruence(Int(2, 32, Signed),
                                         Int(1, 32, Signed))));
  BOOST_CHECK((inv1.widening(inv2) == inv3));

  auto inv4 = IntervalCongruenceDomain::top();
  inv4.set(x, IntervalCongruence(Int(3, 32, Signed)));
  inv4.set(y,
           IntervalCongruence(
               Interval(Int(1, 32, Signed), Int(4, 32, Signed))));
  BOOST_CHECK((inv4.widening(inv2) == inv2));
  BOOST_CHECK((inv2.widening(inv4) == inv2));
}

BOOST_AUTO_TEST_CASE(meet) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));

  BOOST_CHECK((IntervalCongruenceDomain::bottom().meet(
                   IntervalCongruenceDomain::top()) ==
               IntervalCongruenceDomain::bottom()));
  BOOST_CHECK((IntervalCongruenceDomain::bottom().meet(
                   IntervalCongruenceDomain::bottom()) ==
               IntervalCongruenceDomain::bottom()));
  BOOST_CHECK(
      (IntervalCongruenceDomain::top().meet(IntervalCongruenceDomain::top()) ==
       IntervalCongruenceDomain::top()));
  BOOST_CHECK((IntervalCongruenceDomain::top().meet(
                   IntervalCongruenceDomain::bottom()) ==
               IntervalCongruenceDomain::bottom()));

  auto inv1 = IntervalCongruenceDomain::top();
  inv1.set(x,
           IntervalCongruence(Interval(Int(1, 32, Signed), Int(13, 32, Signed)),
                              Congruence(Int(6, 32, Signed),
                                         Int(1, 32, Signed))));
  BOOST_CHECK((inv1.meet(IntervalCongruenceDomain::top()) == inv1));
  BOOST_CHECK((inv1.meet(IntervalCongruenceDomain::bottom()) ==
               IntervalCongruenceDomain::bottom()));
  BOOST_CHECK((IntervalCongruenceDomain::top().meet(inv1) == inv1));
  BOOST_CHECK((IntervalCongruenceDomain::bottom().meet(inv1) ==
               IntervalCongruenceDomain::bottom()));
  BOOST_CHECK((inv1.meet(inv1) == inv1));

  auto inv2 = IntervalCongruenceDomain::top();
  auto inv3 = IntervalCongruenceDomain::top();
  inv2.set(x,
           IntervalCongruence(Interval(Int(7, 32, Signed),
                                       Int::max(32, Signed)),
                              Congruence(Int(8, 32, Signed),
                                         Int(7, 32, Signed))));
  inv3.set(x, IntervalCongruence(Int(7, 32, Signed)));
  BOOST_CHECK((inv1.meet(inv2) == inv3));
  BOOST_CHECK((inv2.meet(inv1) == inv3));
}

BOOST_AUTO_TEST_CASE(narrowing) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));

  BOOST_CHECK((IntervalCongruenceDomain::bottom().narrowing(
                   IntervalCongruenceDomain::top()) ==
               IntervalCongruenceDomain::bottom()));
  BOOST_CHECK((IntervalCongruenceDomain::bottom().narrowing(
                   IntervalCongruenceDomain::bottom()) ==
               IntervalCongruenceDomain::bottom()));
  BOOST_CHECK((IntervalCongruenceDomain::top().narrowing(
                   IntervalCongruenceDomain::top()) ==
               IntervalCongruenceDomain::top()));
  BOOST_CHECK((IntervalCongruenceDomain::top().narrowing(
                   IntervalCongruenceDomain::bottom()) ==
               IntervalCongruenceDomain::bottom()));

  auto inv1 = IntervalCongruenceDomain::top();
  inv1.set(x,
           IntervalCongruence(Interval(Int(1, 32, Signed), Int(13, 32, Signed)),
                              Congruence(Int(6, 32, Signed),
                                         Int(1, 32, Signed))));
  BOOST_CHECK((inv1.narrowing(IntervalCongruenceDomain::top()) == inv1));
  BOOST_CHECK((inv1.narrowing(IntervalCongruenceDomain::bottom()) ==
               IntervalCongruenceDomain::bottom()));
  BOOST_CHECK((IntervalCongruenceDomain::top().narrowing(inv1) == inv1));
  BOOST_CHECK((IntervalCongruenceDomain::bottom().narrowing(inv1) ==
               IntervalCongruenceDomain::bottom()));
  BOOST_CHECK((inv1.narrowing(inv1) == inv1));

  auto inv2 = IntervalCongruenceDomain::top();
  auto inv3 = IntervalCongruenceDomain::top();
  inv2.set(x,
           IntervalCongruence(Interval(Int(7, 32, Signed),
                                       Int::max(32, Signed)),
                              Congruence(Int(8, 32, Signed),
                                         Int(7, 32, Signed))));
  inv3.set(x, IntervalCongruence(Int(7, 32, Signed)));
  BOOST_CHECK((inv1.narrowing(inv2) == inv3));
  BOOST_CHECK((inv2.narrowing(inv1) == inv3));
}

BOOST_AUTO_TEST_CASE(assign) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));
  Variable z(vfac.get("z", 32, Signed));

  auto inv1 = IntervalCongruenceDomain::top();
  auto inv2 = IntervalCongruenceDomain::top();
  inv1.assign(x, Int(0, 32, Signed));
  inv2.set(x, IntervalCongruence(Int(0, 32, Signed)));
  BOOST_CHECK((inv1 == inv2));

  inv1.set_to_bottom();
  inv1.assign(x, Int(0, 32, Signed));
  BOOST_CHECK(inv1.is_bottom());

  inv1.set_to_top();
  inv1.set(x,
           IntervalCongruence(Interval(Int(1, 32, Signed), Int(5, 32, Signed)),
                              Congruence(Int(2, 32, Signed),
                                         Int(1, 32, Signed))));
  inv1.assign(y, x);
  BOOST_CHECK(
      inv1.to_interval_congruence(y) ==
      IntervalCongruence(Interval(Int(1, 32, Signed), Int(5, 32, Signed)),
                         Congruence(Int(2, 32, Signed), Int(1, 32, Signed))));

  inv1.set_to_top();
  inv1.set(x,
           IntervalCongruence(Interval(Int(1, 32, Signed), Int(13, 32, Signed)),
                              Congruence(Int(6, 32, Signed),
                                         Int(1, 32, Signed))));
  inv1.set(y,
           IntervalCongruence(Interval(Int(7, 32, Signed), Int(15, 32, Signed)),
                              Congruence(Int(8, 32, Signed),
                                         Int(7, 32, Signed))));

  LinearExpr e(Int(1, 32, Signed));
  e.add(Int(2, 32, Signed), x);
  e.add(Int(-3, 32, Signed), y);
  inv1.assign(z, e);

  BOOST_CHECK(
      inv1.to_interval_congruence(z) ==
      IntervalCongruence(Interval(Int(-42, 32, Signed), Int(6, 32, Signed)),
                         Congruence(Int(4, 32, Signed), Int(2, 32, Signed))));
}

BOOST_AUTO_TEST_CASE(unary_apply) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 8, Signed));
  Variable y(vfac.get("y", 6, Signed));
  Variable z(vfac.get("z", 8, Signed));
  Variable w(vfac.get("w", 8, Unsigned));

  auto inv = IntervalCongruenceDomain::top();
  inv.assign(x, Int(85, 8, Signed));
  BOOST_CHECK(inv.to_interval_congruence(x) ==
              IntervalCongruence(Int(85, 8, Signed)));
  inv.apply(UnaryOperator::Trunc, y, x);
  BOOST_CHECK(inv.to_interval_congruence(y) ==
              IntervalCongruence(Int(21, 6, Signed)));
  inv.apply(UnaryOperator::Ext, z, y);
  BOOST_CHECK(inv.to_interval_congruence(z) ==
              IntervalCongruence(Int(21, 8, Signed)));
  inv.apply(UnaryOperator::SignCast, w, z);
  BOOST_CHECK(inv.to_interval_congruence(w) ==
              IntervalCongruence(Int(21, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(binary_apply) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 8, Signed));
  Variable y(vfac.get("y", 8, Signed));
  Variable z(vfac.get("z", 8, Signed));

  auto inv = IntervalCongruenceDomain::top();
  inv.assign(x, Int(85, 8, Signed));
  BOOST_CHECK(inv.to_interval_congruence(x) ==
              IntervalCongruence(Int(85, 8, Signed)));
  inv.apply(BinaryOperator::Add, y, x, Int(43, 8, Signed));
  BOOST_CHECK(inv.to_interval_congruence(y) ==
              IntervalCongruence(Int(-128, 8, Signed)));
  inv.apply(BinaryOperator::SubNoWrap, z, y, Int(1, 8, Signed));
  BOOST_CHECK(inv.is_bottom());
}

BOOST_AUTO_TEST_CASE(add_var) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));

  auto inv = IntervalCongruenceDomain::top();
  inv.set(x,
          IntervalCongruence(Interval(Int(7, 32, Signed), Int(13, 32, Signed)),
                             Congruence(Int(6, 32, Signed),
                                        Int(1, 32, Signed))));
  inv.set(y,
          IntervalCongruence(Interval::top(32, Signed),
                             Congruence(Int(8, 32, Signed),
                                        Int(7, 32, Signed))));
  inv.add(Predicate::EQ, x, y);
  BOOST_CHECK(inv.to_interval_congruence(x) ==
              IntervalCongruence(Int(7, 32, Signed)));
  BOOST_CHECK(inv.to_interval_congruence(y) ==
              IntervalCongruence(Int(7, 32, Signed)));
}

BOOST_AUTO_TEST_CASE(add_int) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));

  auto inv = IntervalCongruenceDomain::top();
  inv.set(x,
          IntervalCongruence(Interval::top(32, Signed),
                             Congruence(Int(6, 32, Signed),
                                        Int(1, 32, Signed))));
  inv.add(Predicate::EQ, x, Int(7, 32, Signed));
  BOOST_CHECK(inv.to_interval_congruence(x) ==
              IntervalCongruence(Int(7, 32, Signed)));
}

BOOST_AUTO_TEST_CASE(set) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));

  auto inv = IntervalCongruenceDomain::top();
  inv.set(x,
          IntervalCongruence(Interval(Int(3, 32, Signed), Int(11, 32, Signed)),
                             Congruence(Int(2, 32, Signed),
                                        Int(1, 32, Signed))));
  BOOST_CHECK(
      inv.to_interval_congruence(x) ==
      IntervalCongruence(Interval(Int(3, 32, Signed), Int(11, 32, Signed)),
                         Congruence(Int(2, 32, Signed), Int(1, 32, Signed))));

  inv.set(x, IntervalCongruence::bottom(32, Signed));
  BOOST_CHECK(inv.is_bottom());
}

BOOST_AUTO_TEST_CASE(refine) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));

  auto inv = IntervalCongruenceDomain::top();
  inv.set(x,
          IntervalCongruence(Interval(Int(6, 32, Signed), Int(11, 32, Signed)),
                             Congruence(Int(5, 32, Signed),
                                        Int(1, 32, Signed))));
  BOOST_CHECK(
      inv.to_interval_congruence(x) ==
      IntervalCongruence(Interval(Int(6, 32, Signed), Int(11, 32, Signed)),
                         Congruence(Int(5, 32, Signed), Int(1, 32, Signed))));

  inv.refine(x,
             IntervalCongruence(
                 Interval(Int(7, 32, Signed), Int(10, 32, Signed))));
  BOOST_CHECK(inv.is_bottom());
}

BOOST_AUTO_TEST_CASE(forget) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));

  auto inv = IntervalCongruenceDomain::top();
  inv.set(x,
          IntervalCongruence(Interval(Int(0, 32, Signed), Int(10, 32, Signed)),
                             Congruence(Int(2, 32, Signed),
                                        Int(0, 32, Signed))));
  inv.set(y,
          IntervalCongruence(Interval(Int(1, 32, Signed), Int(11, 32, Signed)),
                             Congruence(Int(2, 32, Signed),
                                        Int(1, 32, Signed))));
  BOOST_CHECK(
      inv.to_interval_congruence(x) ==
      IntervalCongruence(Interval(Int(0, 32, Signed), Int(10, 32, Signed)),
                         Congruence(Int(2, 32, Signed), Int(0, 32, Signed))));
  BOOST_CHECK(
      inv.to_interval_congruence(y) ==
      IntervalCongruence(Interval(Int(1, 32, Signed), Int(11, 32, Signed)),
                         Congruence(Int(2, 32, Signed), Int(1, 32, Signed))));

  inv.forget(x);
  BOOST_CHECK(inv.to_interval_congruence(x) ==
              IntervalCongruence::top(32, Signed));
  BOOST_CHECK(
      inv.to_interval_congruence(y) ==
      IntervalCongruence(Interval(Int(1, 32, Signed), Int(11, 32, Signed)),
                         Congruence(Int(2, 32, Signed), Int(1, 32, Signed))));

  inv.forget(y);
  BOOST_CHECK(inv.is_top());
}

BOOST_AUTO_TEST_CASE(to_interval_congruence) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));

  auto inv = IntervalCongruenceDomain::top();
  inv.set(x,
          IntervalCongruence(Interval(Int(7, 32, Signed), Int(13, 32, Signed)),
                             Congruence(Int(6, 32, Signed),
                                        Int(1, 32, Signed))));
  inv.set(y,
          IntervalCongruence(Interval(Int(7, 32, Signed), Int(15, 32, Signed)),
                             Congruence(Int(8, 32, Signed),
                                        Int(7, 32, Signed))));

  LinearExpr e1(Int(1, 32, Signed));
  e1.add(Int(2, 32, Signed), x);
  BOOST_CHECK(
      inv.to_interval_congruence(e1) ==
      IntervalCongruence(Interval(Int(15, 32, Signed), Int(27, 32, Signed)),
                         Congruence(Int(4, 32, Signed), Int(3, 32, Signed))));

  LinearExpr e2(Int(1, 32, Signed));
  e2.add(Int(2, 32, Signed), x);
  e2.add(Int(-3, 32, Signed), y);
  BOOST_CHECK(
      inv.to_interval_congruence(e2) ==
      IntervalCongruence(Interval(Int(-30, 32, Signed), Int(6, 32, Signed)),
                         Congruence(Int(4, 32, Signed), Int(2, 32, Signed))));
}
