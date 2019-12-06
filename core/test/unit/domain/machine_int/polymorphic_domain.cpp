/*******************************************************************************
 *
 * Tests for machine_int::PolymorphicDomain
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

#define BOOST_TEST_MODULE test_machine_int_polymorphic_domain
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/domain/machine_int/congruence.hpp>
#include <ikos/core/domain/machine_int/interval.hpp>
#include <ikos/core/domain/machine_int/polymorphic_domain.hpp>
#include <ikos/core/example/machine_int/variable_factory.hpp>

using Int = ikos::core::MachineInt;
using Interval = ikos::core::machine_int::Interval;
using Congruence = ikos::core::machine_int::Congruence;
using ikos::core::Signed;
using ikos::core::Unsigned;
using ikos::core::machine_int::BinaryOperator;
using ikos::core::machine_int::Predicate;
using ikos::core::machine_int::UnaryOperator;
using VariableFactory = ikos::core::example::machine_int::VariableFactory;
using Variable = VariableFactory::VariableRef;
using VariableExpr = ikos::core::VariableExpression< Int, Variable >;
using LinearExpr = ikos::core::LinearExpression< Int, Variable >;
using PolymorphicDomain =
    ikos::core::machine_int::PolymorphicDomain< Variable >;
using IntervalDomain = ikos::core::machine_int::IntervalDomain< Variable >;
using CongruenceDomain = ikos::core::machine_int::CongruenceDomain< Variable >;

BOOST_AUTO_TEST_CASE(is_top_and_bottom) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));

  BOOST_CHECK(PolymorphicDomain(IntervalDomain::top()).is_top());
  BOOST_CHECK(!PolymorphicDomain(IntervalDomain::top()).is_bottom());

  BOOST_CHECK(!PolymorphicDomain(IntervalDomain::bottom()).is_top());
  BOOST_CHECK(PolymorphicDomain(IntervalDomain::bottom()).is_bottom());

  {
    auto inv = PolymorphicDomain(IntervalDomain::top());
    BOOST_CHECK(inv.is_top());
    BOOST_CHECK(!inv.is_bottom());

    inv.set(x, Interval(Int(1, 32, Signed)));
    BOOST_CHECK(!inv.is_top());
    BOOST_CHECK(!inv.is_bottom());

    inv.set(x, Interval::bottom(32, Signed));
    BOOST_CHECK(!inv.is_top());
    BOOST_CHECK(inv.is_bottom());
  }
  {
    auto inv = PolymorphicDomain(CongruenceDomain::top());
    BOOST_CHECK(inv.is_top());
    BOOST_CHECK(!inv.is_bottom());

    inv.set(x, Congruence(Int(1, 32, Signed)));
    BOOST_CHECK(!inv.is_top());
    BOOST_CHECK(!inv.is_bottom());

    inv.set(x, Congruence::bottom(32, Signed));
    BOOST_CHECK(!inv.is_top());
    BOOST_CHECK(inv.is_bottom());
  }
}

BOOST_AUTO_TEST_CASE(set_to_top_and_bottom) {
  VariableFactory vfac;

  {
    auto inv = PolymorphicDomain(IntervalDomain::top());
    BOOST_CHECK(inv.is_top());
    BOOST_CHECK(!inv.is_bottom());

    inv.set_to_bottom();
    BOOST_CHECK(!inv.is_top());
    BOOST_CHECK(inv.is_bottom());

    inv.set_to_top();
    BOOST_CHECK(inv.is_top());
    BOOST_CHECK(!inv.is_bottom());
  }
  {
    auto inv = PolymorphicDomain(CongruenceDomain::top());
    BOOST_CHECK(inv.is_top());
    BOOST_CHECK(!inv.is_bottom());

    inv.set_to_bottom();
    BOOST_CHECK(!inv.is_top());
    BOOST_CHECK(inv.is_bottom());

    inv.set_to_top();
    BOOST_CHECK(inv.is_top());
    BOOST_CHECK(!inv.is_bottom());
  }
}

BOOST_AUTO_TEST_CASE(leq) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));
  Variable z(vfac.get("z", 32, Signed));

  BOOST_CHECK(PolymorphicDomain(IntervalDomain::bottom())
                  .leq(PolymorphicDomain(IntervalDomain::top())));
  BOOST_CHECK(PolymorphicDomain(IntervalDomain::bottom())
                  .leq(PolymorphicDomain(IntervalDomain::bottom())));
  BOOST_CHECK(!PolymorphicDomain(IntervalDomain::top())
                   .leq(PolymorphicDomain(IntervalDomain::bottom())));
  BOOST_CHECK(PolymorphicDomain(IntervalDomain::top())
                  .leq(PolymorphicDomain(IntervalDomain::top())));

  {
    auto inv1 = PolymorphicDomain(IntervalDomain::top());
    inv1.set(x, Interval(Int(0, 32, Signed)));
    BOOST_CHECK(inv1.leq(PolymorphicDomain(IntervalDomain::top())));
    BOOST_CHECK(!inv1.leq(PolymorphicDomain(IntervalDomain::bottom())));

    auto inv2 = PolymorphicDomain(IntervalDomain::top());
    inv2.set(x, Interval(Int(-1, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK(inv2.leq(PolymorphicDomain(IntervalDomain::top())));
    BOOST_CHECK(!inv2.leq(PolymorphicDomain(IntervalDomain::bottom())));
    BOOST_CHECK(inv1.leq(inv2));
    BOOST_CHECK(!inv2.leq(inv1));

    auto inv3 = PolymorphicDomain(IntervalDomain::top());
    inv3.set(x, Interval(Int(0, 32, Signed)));
    inv3.set(y, Interval(Int(-1, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK(inv3.leq(PolymorphicDomain(IntervalDomain::top())));
    BOOST_CHECK(!inv3.leq(PolymorphicDomain(IntervalDomain::bottom())));
    BOOST_CHECK(inv3.leq(inv1));
    BOOST_CHECK(!inv1.leq(inv3));

    auto inv4 = PolymorphicDomain(IntervalDomain::top());
    inv4.set(x, Interval(Int(0, 32, Signed)));
    inv4.set(y, Interval(Int(0, 32, Signed), Int(2, 32, Signed)));
    BOOST_CHECK(inv4.leq(PolymorphicDomain(IntervalDomain::top())));
    BOOST_CHECK(!inv4.leq(PolymorphicDomain(IntervalDomain::bottom())));
    BOOST_CHECK(!inv3.leq(inv4));
    BOOST_CHECK(!inv4.leq(inv3));

    auto inv5 = PolymorphicDomain(IntervalDomain::top());
    inv5.set(x, Interval(Int(0, 32, Signed)));
    inv5.set(y, Interval(Int(0, 32, Signed), Int(2, 32, Signed)));
    inv5.set(z, Interval(Int::min(32, Signed), Int(0, 32, Signed)));
    BOOST_CHECK(inv5.leq(PolymorphicDomain(IntervalDomain::top())));
    BOOST_CHECK(!inv5.leq(PolymorphicDomain(IntervalDomain::bottom())));
    BOOST_CHECK(!inv5.leq(inv3));
    BOOST_CHECK(!inv3.leq(inv5));
    BOOST_CHECK(inv5.leq(inv4));
    BOOST_CHECK(!inv4.leq(inv5));
  }
  {
    auto inv1 = PolymorphicDomain(CongruenceDomain::top());
    inv1.set(x, Congruence(Int(0, 32, Signed)));
    BOOST_CHECK(inv1.leq(PolymorphicDomain(CongruenceDomain::top())));
    BOOST_CHECK(!inv1.leq(PolymorphicDomain(CongruenceDomain::bottom())));

    auto inv2 = PolymorphicDomain(CongruenceDomain::top());
    inv2.set(x, Congruence(Int(2, 32, Signed), Int(0, 32, Signed)));
    BOOST_CHECK(inv2.leq(PolymorphicDomain(CongruenceDomain::top())));
    BOOST_CHECK(!inv2.leq(PolymorphicDomain(CongruenceDomain::bottom())));
    BOOST_CHECK(inv1.leq(inv2));
    BOOST_CHECK(!inv2.leq(inv1));

    auto inv3 = PolymorphicDomain(CongruenceDomain::top());
    inv3.set(x, Congruence(Int(0, 32, Signed)));
    inv3.set(y, Congruence(Int(2, 32, Signed), Int(0, 32, Signed)));
    BOOST_CHECK(inv3.leq(PolymorphicDomain(CongruenceDomain::top())));
    BOOST_CHECK(!inv3.leq(PolymorphicDomain(CongruenceDomain::bottom())));
    BOOST_CHECK(inv3.leq(inv1));
    BOOST_CHECK(!inv1.leq(inv3));

    auto inv4 = PolymorphicDomain(CongruenceDomain::top());
    inv4.set(x, Congruence(Int(0, 32, Signed)));
    inv4.set(y, Congruence(Int(2, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK(inv4.leq(PolymorphicDomain(CongruenceDomain::top())));
    BOOST_CHECK(!inv4.leq(PolymorphicDomain(CongruenceDomain::bottom())));
    BOOST_CHECK(!inv3.leq(inv4));
    BOOST_CHECK(!inv4.leq(inv3));

    auto inv5 = PolymorphicDomain(CongruenceDomain::top());
    inv5.set(x, Congruence(Int(0, 32, Signed)));
    inv5.set(y, Congruence(Int(2, 32, Signed), Int(1, 32, Signed)));
    inv5.set(z, Congruence(Int(3, 32, Signed), Int(0, 32, Signed)));
    BOOST_CHECK(inv5.leq(PolymorphicDomain(CongruenceDomain::top())));
    BOOST_CHECK(!inv5.leq(PolymorphicDomain(CongruenceDomain::bottom())));
    BOOST_CHECK(!inv5.leq(inv3));
    BOOST_CHECK(!inv3.leq(inv5));
    BOOST_CHECK(inv5.leq(inv4));
    BOOST_CHECK(!inv4.leq(inv5));
  }
}

BOOST_AUTO_TEST_CASE(equals) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));

  BOOST_CHECK(!PolymorphicDomain(IntervalDomain::bottom())
                   .equals(PolymorphicDomain(IntervalDomain::top())));
  BOOST_CHECK(PolymorphicDomain(IntervalDomain::bottom())
                  .equals(PolymorphicDomain(IntervalDomain::bottom())));
  BOOST_CHECK(!PolymorphicDomain(IntervalDomain::top())
                   .equals(PolymorphicDomain(IntervalDomain::bottom())));
  BOOST_CHECK(PolymorphicDomain(IntervalDomain::top())
                  .equals(PolymorphicDomain(IntervalDomain::top())));

  {
    auto inv1 = PolymorphicDomain(IntervalDomain::top());
    inv1.set(x, Interval(Int(0, 32, Signed)));
    BOOST_CHECK(!inv1.equals(PolymorphicDomain(IntervalDomain::top())));
    BOOST_CHECK(!inv1.equals(PolymorphicDomain(IntervalDomain::bottom())));
    BOOST_CHECK(inv1.equals(inv1));

    auto inv2 = PolymorphicDomain(IntervalDomain::top());
    inv2.set(x, Interval(Int(-1, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK(!inv2.equals(PolymorphicDomain(IntervalDomain::top())));
    BOOST_CHECK(!inv2.equals(PolymorphicDomain(IntervalDomain::bottom())));
    BOOST_CHECK(!inv1.equals(inv2));
    BOOST_CHECK(!inv2.equals(inv1));

    auto inv3 = PolymorphicDomain(IntervalDomain::top());
    inv3.set(x, Interval(Int(0, 32, Signed)));
    inv3.set(y, Interval(Int(-1, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK(!inv3.equals(PolymorphicDomain(IntervalDomain::top())));
    BOOST_CHECK(!inv3.equals(PolymorphicDomain(IntervalDomain::bottom())));
    BOOST_CHECK(!inv3.equals(inv1));
    BOOST_CHECK(!inv1.equals(inv3));
  }
  {
    auto inv1 = PolymorphicDomain(CongruenceDomain::top());
    inv1.set(x, Congruence(Int(0, 32, Signed)));
    BOOST_CHECK(!inv1.equals(PolymorphicDomain(CongruenceDomain::top())));
    BOOST_CHECK(!inv1.equals(PolymorphicDomain(CongruenceDomain::bottom())));
    BOOST_CHECK(inv1.equals(inv1));

    auto inv2 = PolymorphicDomain(CongruenceDomain::top());
    inv2.set(x, Congruence(Int(2, 32, Signed), Int(0, 32, Signed)));
    BOOST_CHECK(!inv2.equals(PolymorphicDomain(CongruenceDomain::top())));
    BOOST_CHECK(!inv2.equals(PolymorphicDomain(CongruenceDomain::bottom())));
    BOOST_CHECK(!inv1.equals(inv2));
    BOOST_CHECK(!inv2.equals(inv1));

    auto inv3 = PolymorphicDomain(CongruenceDomain::top());
    inv3.set(x, Congruence(Int(0, 32, Signed)));
    inv3.set(y, Congruence(Int(2, 32, Signed), Int(0, 32, Signed)));
    BOOST_CHECK(!inv3.equals(PolymorphicDomain(CongruenceDomain::top())));
    BOOST_CHECK(!inv3.equals(PolymorphicDomain(CongruenceDomain::bottom())));
    BOOST_CHECK(!inv3.equals(inv1));
    BOOST_CHECK(!inv1.equals(inv3));
  }
}

BOOST_AUTO_TEST_CASE(join) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));

  BOOST_CHECK((PolymorphicDomain(IntervalDomain::bottom())
                   .join(PolymorphicDomain(IntervalDomain::top())) ==
               PolymorphicDomain(IntervalDomain::top())));
  BOOST_CHECK((PolymorphicDomain(IntervalDomain::bottom())
                   .join(PolymorphicDomain(IntervalDomain::bottom())) ==
               PolymorphicDomain(IntervalDomain::bottom())));
  BOOST_CHECK((PolymorphicDomain(IntervalDomain::top())
                   .join(PolymorphicDomain(IntervalDomain::top())) ==
               PolymorphicDomain(IntervalDomain::top())));
  BOOST_CHECK((PolymorphicDomain(IntervalDomain::top())
                   .join(PolymorphicDomain(IntervalDomain::bottom())) ==
               PolymorphicDomain(IntervalDomain::top())));

  {
    auto inv1 = PolymorphicDomain(IntervalDomain::top());
    inv1.set(x, Interval(Int(0, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK((inv1.join(PolymorphicDomain(IntervalDomain::top())) ==
                 PolymorphicDomain(IntervalDomain::top())));
    BOOST_CHECK(
        (inv1.join(PolymorphicDomain(IntervalDomain::bottom())) == inv1));
    BOOST_CHECK((PolymorphicDomain(IntervalDomain::top()).join(inv1) ==
                 PolymorphicDomain(IntervalDomain::top())));
    BOOST_CHECK(
        (PolymorphicDomain(IntervalDomain::bottom()).join(inv1) == inv1));
    BOOST_CHECK((inv1.join(inv1) == inv1));

    auto inv2 = PolymorphicDomain(IntervalDomain::top());
    auto inv3 = PolymorphicDomain(IntervalDomain::top());
    inv2.set(x, Interval(Int(-1, 32, Signed), Int(0, 32, Signed)));
    inv3.set(x, Interval(Int(-1, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK((inv1.join(inv2) == inv3));
    BOOST_CHECK((inv2.join(inv1) == inv3));

    auto inv4 = PolymorphicDomain(IntervalDomain::top());
    inv4.set(x, Interval(Int(-1, 32, Signed), Int(0, 32, Signed)));
    inv4.set(y, Interval(Int(0, 32, Signed)));
    BOOST_CHECK((inv4.join(inv2) == inv2));
    BOOST_CHECK((inv2.join(inv4) == inv2));
  }
  {
    auto inv1 = PolymorphicDomain(CongruenceDomain::top());
    inv1.set(x, Congruence(Int(1, 32, Signed)));
    BOOST_CHECK((inv1.join(PolymorphicDomain(CongruenceDomain::top())) ==
                 PolymorphicDomain(CongruenceDomain::top())));
    BOOST_CHECK(
        (inv1.join(PolymorphicDomain(CongruenceDomain::bottom())) == inv1));
    BOOST_CHECK(
        (PolymorphicDomain(PolymorphicDomain(CongruenceDomain::top()))
             .join(inv1) == PolymorphicDomain(CongruenceDomain::top())));
    BOOST_CHECK(
        (PolymorphicDomain(CongruenceDomain::bottom()).join(inv1) == inv1));
    BOOST_CHECK((inv1.join(inv1) == inv1));

    auto inv2 = PolymorphicDomain(CongruenceDomain::top());
    auto inv3 = PolymorphicDomain(CongruenceDomain::top());
    inv2.set(x, Congruence(Int(3, 32, Signed)));
    inv3.set(x, Congruence(Int(2, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK((inv1.join(inv2) == inv3));
    BOOST_CHECK((inv2.join(inv1) == inv3));

    auto inv4 = PolymorphicDomain(CongruenceDomain::top());
    inv4.set(x, Congruence(Int(3, 32, Signed)));
    inv4.set(y, Congruence(Int(4, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK((inv4.join(inv2) == inv2));
    BOOST_CHECK((inv2.join(inv4) == inv2));
  }
}

BOOST_AUTO_TEST_CASE(widening) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));

  BOOST_CHECK((PolymorphicDomain(IntervalDomain::bottom())
                   .widening(PolymorphicDomain(IntervalDomain::top())) ==
               PolymorphicDomain(IntervalDomain::top())));
  BOOST_CHECK((PolymorphicDomain(IntervalDomain::bottom())
                   .widening(PolymorphicDomain(IntervalDomain::bottom())) ==
               PolymorphicDomain(IntervalDomain::bottom())));
  BOOST_CHECK((PolymorphicDomain(IntervalDomain::top())
                   .widening(PolymorphicDomain(IntervalDomain::top())) ==
               PolymorphicDomain(IntervalDomain::top())));
  BOOST_CHECK((PolymorphicDomain(IntervalDomain::top())
                   .widening(PolymorphicDomain(IntervalDomain::bottom())) ==
               PolymorphicDomain(IntervalDomain::top())));

  {
    auto inv1 = PolymorphicDomain(IntervalDomain::top());
    inv1.set(x, Interval(Int(0, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK((inv1.widening(PolymorphicDomain(IntervalDomain::top())) ==
                 PolymorphicDomain(IntervalDomain::top())));
    BOOST_CHECK(
        (inv1.widening(PolymorphicDomain(IntervalDomain::bottom())) == inv1));
    BOOST_CHECK((PolymorphicDomain(IntervalDomain::top()).widening(inv1) ==
                 PolymorphicDomain(IntervalDomain::top())));
    BOOST_CHECK(
        (PolymorphicDomain(IntervalDomain::bottom()).widening(inv1) == inv1));
    BOOST_CHECK((inv1.widening(inv1) == inv1));

    auto inv2 = PolymorphicDomain(IntervalDomain::top());
    auto inv3 = PolymorphicDomain(IntervalDomain::top());
    inv2.set(x, Interval(Int(0, 32, Signed), Int(2, 32, Signed)));
    inv3.set(x, Interval(Int(0, 32, Signed), Int::max(32, Signed)));
    BOOST_CHECK((inv1.widening(inv2) == inv3));
    BOOST_CHECK((inv2.widening(inv1) == inv2));
  }
  {
    auto inv1 = PolymorphicDomain(CongruenceDomain::top());
    inv1.set(x, Congruence(Int(1, 32, Signed)));
    BOOST_CHECK((inv1.widening(PolymorphicDomain(CongruenceDomain::top())) ==
                 PolymorphicDomain(CongruenceDomain::top())));
    BOOST_CHECK(
        (inv1.widening(PolymorphicDomain(CongruenceDomain::bottom())) == inv1));
    BOOST_CHECK((PolymorphicDomain(CongruenceDomain::top()).widening(inv1) ==
                 PolymorphicDomain(CongruenceDomain::top())));
    BOOST_CHECK(
        (PolymorphicDomain(CongruenceDomain::bottom()).widening(inv1) == inv1));
    BOOST_CHECK((inv1.widening(inv1) == inv1));

    auto inv2 = PolymorphicDomain(CongruenceDomain::top());
    auto inv3 = PolymorphicDomain(CongruenceDomain::top());
    inv2.set(x, Congruence(Int(3, 32, Signed)));
    inv3.set(x, Congruence(Int(2, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK((inv1.widening(inv2) == inv3));
    BOOST_CHECK((inv2.widening(inv1) == inv3));

    auto inv4 = PolymorphicDomain(CongruenceDomain::top());
    inv4.set(x, Congruence(Int(3, 32, Signed)));
    inv4.set(y, Congruence(Int(4, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK((inv4.widening(inv2) == inv2));
    BOOST_CHECK((inv2.widening(inv4) == inv2));
  }
}

BOOST_AUTO_TEST_CASE(meet) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));

  BOOST_CHECK((PolymorphicDomain(IntervalDomain::bottom())
                   .meet(PolymorphicDomain(IntervalDomain::top())) ==
               PolymorphicDomain(IntervalDomain::bottom())));
  BOOST_CHECK((PolymorphicDomain(IntervalDomain::bottom())
                   .meet(PolymorphicDomain(IntervalDomain::bottom())) ==
               PolymorphicDomain(IntervalDomain::bottom())));
  BOOST_CHECK((PolymorphicDomain(IntervalDomain::top())
                   .meet(PolymorphicDomain(IntervalDomain::top())) ==
               PolymorphicDomain(IntervalDomain::top())));
  BOOST_CHECK((PolymorphicDomain(IntervalDomain::top())
                   .meet(PolymorphicDomain(IntervalDomain::bottom())) ==
               PolymorphicDomain(IntervalDomain::bottom())));

  {
    auto inv1 = PolymorphicDomain(IntervalDomain::top());
    inv1.set(x, Interval(Int(0, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK((inv1.meet(PolymorphicDomain(IntervalDomain::top())) == inv1));
    BOOST_CHECK((inv1.meet(PolymorphicDomain(IntervalDomain::bottom())) ==
                 PolymorphicDomain(IntervalDomain::bottom())));
    BOOST_CHECK((PolymorphicDomain(IntervalDomain::top()).meet(inv1) == inv1));
    BOOST_CHECK((PolymorphicDomain(IntervalDomain::bottom()).meet(inv1) ==
                 PolymorphicDomain(IntervalDomain::bottom())));
    BOOST_CHECK((inv1.meet(inv1) == inv1));

    auto inv2 = PolymorphicDomain(IntervalDomain::top());
    auto inv3 = PolymorphicDomain(IntervalDomain::top());
    inv2.set(x, Interval(Int(-1, 32, Signed), Int(0, 32, Signed)));
    inv3.set(x, Interval(Int(0, 32, Signed)));
    BOOST_CHECK((inv1.meet(inv2) == inv3));
    BOOST_CHECK((inv2.meet(inv1) == inv3));

    auto inv4 = PolymorphicDomain(IntervalDomain::top());
    auto inv5 = PolymorphicDomain(IntervalDomain::top());
    inv4.set(x, Interval(Int(0, 32, Signed), Int(1, 32, Signed)));
    inv4.set(y, Interval(Int(0, 32, Signed)));
    inv5.set(x, Interval(Int(0, 32, Signed)));
    inv5.set(y, Interval(Int(0, 32, Signed)));
    BOOST_CHECK((inv4.meet(inv2) == inv5));
    BOOST_CHECK((inv2.meet(inv4) == inv5));
  }
  {
    auto inv1 = PolymorphicDomain(CongruenceDomain::top());
    inv1.set(x, Congruence(Int(6, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK(
        (inv1.meet(PolymorphicDomain(CongruenceDomain::top())) == inv1));
    BOOST_CHECK((inv1.meet(PolymorphicDomain(CongruenceDomain::bottom())) ==
                 PolymorphicDomain(CongruenceDomain::bottom())));
    BOOST_CHECK(
        (PolymorphicDomain(CongruenceDomain::top()).meet(inv1) == inv1));
    BOOST_CHECK((PolymorphicDomain(CongruenceDomain::bottom()).meet(inv1) ==
                 PolymorphicDomain(CongruenceDomain::bottom())));
    BOOST_CHECK((inv1.meet(inv1) == inv1));

    auto inv2 = PolymorphicDomain(CongruenceDomain::top());
    auto inv3 = PolymorphicDomain(CongruenceDomain::top());
    inv2.set(x, Congruence(Int(8, 32, Signed), Int(7, 32, Signed)));
    inv3.set(x, Congruence(Int(24, 32, Signed), Int(7, 32, Signed)));
    BOOST_CHECK((inv1.meet(inv2) == inv3));
    BOOST_CHECK((inv2.meet(inv1) == inv3));
  }
}

BOOST_AUTO_TEST_CASE(narrowing) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));

  BOOST_CHECK((PolymorphicDomain(IntervalDomain::bottom())
                   .narrowing(PolymorphicDomain(IntervalDomain::top())) ==
               PolymorphicDomain(IntervalDomain::bottom())));
  BOOST_CHECK((PolymorphicDomain(IntervalDomain::bottom())
                   .narrowing(PolymorphicDomain(IntervalDomain::bottom())) ==
               PolymorphicDomain(IntervalDomain::bottom())));
  BOOST_CHECK((PolymorphicDomain(IntervalDomain::top())
                   .narrowing(PolymorphicDomain(IntervalDomain::top())) ==
               PolymorphicDomain(IntervalDomain::top())));
  BOOST_CHECK((PolymorphicDomain(IntervalDomain::top())
                   .narrowing(PolymorphicDomain(IntervalDomain::bottom())) ==
               PolymorphicDomain(IntervalDomain::bottom())));

  {
    auto inv1 = PolymorphicDomain(IntervalDomain::top());
    inv1.set(x, Interval(Int(0, 32, Signed), Int::max(32, Signed)));
    BOOST_CHECK(
        (inv1.narrowing(PolymorphicDomain(IntervalDomain::top())) == inv1));
    BOOST_CHECK((inv1.narrowing(PolymorphicDomain(IntervalDomain::bottom())) ==
                 PolymorphicDomain(IntervalDomain::bottom())));
    BOOST_CHECK(
        (PolymorphicDomain(IntervalDomain::top()).narrowing(inv1) == inv1));
    BOOST_CHECK((PolymorphicDomain(IntervalDomain::bottom()).narrowing(inv1) ==
                 PolymorphicDomain(IntervalDomain::bottom())));
    BOOST_CHECK((inv1.narrowing(inv1) == inv1));

    auto inv2 = PolymorphicDomain(IntervalDomain::top());
    auto inv3 = PolymorphicDomain(IntervalDomain::top());
    inv2.set(x, Interval(Int(0, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK((inv1.narrowing(inv2) == inv2));
    BOOST_CHECK((inv2.narrowing(inv1) == inv2));
  }
  {
    auto inv1 = PolymorphicDomain(CongruenceDomain::top());
    inv1.set(x, Congruence(Int(6, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK(
        (inv1.narrowing(PolymorphicDomain(CongruenceDomain::top())) == inv1));
    BOOST_CHECK(
        (inv1.narrowing(PolymorphicDomain(CongruenceDomain::bottom())) ==
         PolymorphicDomain(CongruenceDomain::bottom())));
    BOOST_CHECK(
        (PolymorphicDomain(CongruenceDomain::top()).narrowing(inv1) == inv1));
    BOOST_CHECK(
        (PolymorphicDomain(CongruenceDomain::bottom()).narrowing(inv1) ==
         PolymorphicDomain(CongruenceDomain::bottom())));
    BOOST_CHECK((inv1.narrowing(inv1) == inv1));

    auto inv2 = PolymorphicDomain(CongruenceDomain::top());
    auto inv3 = PolymorphicDomain(CongruenceDomain::top());
    inv2.set(x, Congruence(Int(8, 32, Signed), Int(7, 32, Signed)));
    inv3.set(x, Congruence(Int(24, 32, Signed), Int(7, 32, Signed)));
    BOOST_CHECK((inv1.narrowing(inv2) == inv3));
    BOOST_CHECK((inv2.narrowing(inv1) == inv3));
  }
}

BOOST_AUTO_TEST_CASE(assign) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));
  Variable z(vfac.get("z", 32, Signed));

  {
    auto inv1 = PolymorphicDomain(IntervalDomain::top());
    auto inv2 = PolymorphicDomain(IntervalDomain::top());
    inv1.assign(x, Int(0, 32, Signed));
    inv2.set(x, Interval(Int(0, 32, Signed)));
    BOOST_CHECK((inv1 == inv2));

    inv1.set_to_bottom();
    inv1.assign(x, Int(0, 32, Signed));
    BOOST_CHECK(inv1.is_bottom());

    inv1.set_to_top();
    inv1.set(x, Interval(Int(-1, 32, Signed), Int(1, 32, Signed)));
    inv1.assign(y, x);
    BOOST_CHECK(inv1.to_interval(y) ==
                Interval(Int(-1, 32, Signed), Int(1, 32, Signed)));

    inv1.set_to_top();
    inv1.set(x, Interval(Int(-1, 32, Signed), Int(1, 32, Signed)));
    inv1.set(y, Interval(Int(1, 32, Signed), Int(2, 32, Signed)));

    LinearExpr e(Int(1, 32, Signed));
    e.add(Int(2, 32, Signed), x);
    e.add(Int(-3, 32, Signed), y);
    inv1.assign(z, e);

    BOOST_CHECK(inv1.to_interval(z) ==
                Interval(Int(-7, 32, Signed), Int(0, 32, Signed)));
  }
  {
    auto inv1 = PolymorphicDomain(CongruenceDomain::top());
    auto inv2 = PolymorphicDomain(CongruenceDomain::top());
    inv1.assign(x, Int(0, 32, Signed));
    inv2.set(x, Congruence(Int(0, 32, Signed)));
    BOOST_CHECK((inv1 == inv2));

    inv1.set_to_bottom();
    inv1.assign(x, Int(0, 32, Signed));
    BOOST_CHECK(inv1.is_bottom());

    inv1.set_to_top();
    inv1.set(x, Congruence(Int(2, 32, Signed), Int(1, 32, Signed)));
    inv1.assign(y, x);
    BOOST_CHECK(inv1.to_congruence(y) ==
                Congruence(Int(2, 32, Signed), Int(1, 32, Signed)));

    inv1.set_to_top();
    inv1.set(x, Congruence(Int(6, 32, Signed), Int(1, 32, Signed)));
    inv1.set(y, Congruence(Int(8, 32, Signed), Int(7, 32, Signed)));

    LinearExpr e(Int(1, 32, Signed));
    e.add(Int(2, 32, Signed), x);
    e.add(Int(-3, 32, Signed), y);
    inv1.assign(z, e);

    BOOST_CHECK(inv1.to_congruence(z) ==
                Congruence(Int(4, 32, Signed), Int(2, 32, Signed)));
  }
}

BOOST_AUTO_TEST_CASE(unary_apply) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 8, Signed));
  Variable y(vfac.get("y", 6, Signed));
  Variable z(vfac.get("z", 8, Signed));
  Variable w(vfac.get("w", 8, Unsigned));

  {
    auto inv = PolymorphicDomain(IntervalDomain::top());
    inv.assign(x, Int(85, 8, Signed));
    BOOST_CHECK(inv.to_interval(x) == Interval(Int(85, 8, Signed)));
    inv.apply(UnaryOperator::Trunc, y, x);
    BOOST_CHECK(inv.to_interval(y) == Interval(Int(21, 6, Signed)));
    inv.apply(UnaryOperator::Ext, z, y);
    BOOST_CHECK(inv.to_interval(z) == Interval(Int(21, 8, Signed)));
    inv.apply(UnaryOperator::SignCast, w, z);
    BOOST_CHECK(inv.to_interval(w) == Interval(Int(21, 8, Unsigned)));
  }
  {
    auto inv = PolymorphicDomain(CongruenceDomain::top());
    inv.assign(x, Int(85, 8, Signed));
    BOOST_CHECK(inv.to_congruence(x) == Congruence(Int(85, 8, Signed)));
    inv.apply(UnaryOperator::Trunc, y, x);
    BOOST_CHECK(inv.to_congruence(y) == Congruence(Int(21, 6, Signed)));
    inv.apply(UnaryOperator::Ext, z, y);
    BOOST_CHECK(inv.to_congruence(z) == Congruence(Int(21, 8, Signed)));
    inv.apply(UnaryOperator::SignCast, w, z);
    BOOST_CHECK(inv.to_congruence(w) == Congruence(Int(21, 8, Unsigned)));
  }
}

BOOST_AUTO_TEST_CASE(binary_apply) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 8, Signed));
  Variable y(vfac.get("y", 8, Signed));
  Variable z(vfac.get("z", 8, Signed));

  {
    auto inv = PolymorphicDomain(IntervalDomain::top());
    inv.assign(x, Int(85, 8, Signed));
    BOOST_CHECK(inv.to_interval(x) == Interval(Int(85, 8, Signed)));
    inv.apply(BinaryOperator::Add, y, x, Int(43, 8, Signed));
    BOOST_CHECK(inv.to_interval(y) == Interval(Int(-128, 8, Signed)));
    inv.apply(BinaryOperator::SubNoWrap, z, y, Int(1, 8, Signed));
    BOOST_CHECK(inv.is_bottom());
  }
  {
    auto inv = PolymorphicDomain(CongruenceDomain::top());
    inv.assign(x, Int(85, 8, Signed));
    BOOST_CHECK(inv.to_congruence(x) == Congruence(Int(85, 8, Signed)));
    inv.apply(BinaryOperator::Add, y, x, Int(43, 8, Signed));
    BOOST_CHECK(inv.to_congruence(y) == Congruence(Int(-128, 8, Signed)));
    inv.apply(BinaryOperator::SubNoWrap, z, y, Int(1, 8, Signed));
    BOOST_CHECK(inv.is_bottom());
  }
}

BOOST_AUTO_TEST_CASE(add_var) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));

  {
    auto inv = PolymorphicDomain(IntervalDomain::top());
    inv.set(x, Interval(Int(0, 32, Signed), Int(4, 32, Signed)));
    inv.set(y, Interval(Int(-4, 32, Signed), Int(0, 32, Signed)));
    inv.add(Predicate::EQ, x, y);
    BOOST_CHECK(inv.to_interval(x) == Interval(Int(0, 32, Signed)));
    BOOST_CHECK(inv.to_interval(y) == Interval(Int(0, 32, Signed)));

    inv.add(Predicate::NE, x, y);
    BOOST_CHECK(inv.is_bottom());

    inv.set_to_top();
    inv.set(x, Interval(Int(0, 32, Signed), Int(4, 32, Signed)));
    inv.set(y, Interval(Int(1, 32, Signed), Int(5, 32, Signed)));
    inv.add(Predicate::GT, x, y);
    BOOST_CHECK(inv.to_interval(x) ==
                Interval(Int(2, 32, Signed), Int(4, 32, Signed)));
    BOOST_CHECK(inv.to_interval(y) ==
                Interval(Int(1, 32, Signed), Int(3, 32, Signed)));

    inv.add(Predicate::LE, x, y);
    BOOST_CHECK(inv.to_interval(x) ==
                Interval(Int(2, 32, Signed), Int(3, 32, Signed)));
    BOOST_CHECK(inv.to_interval(y) ==
                Interval(Int(2, 32, Signed), Int(3, 32, Signed)));

    inv.add(Predicate::LT, x, y);
    BOOST_CHECK(inv.to_interval(x) == Interval(Int(2, 32, Signed)));
    BOOST_CHECK(inv.to_interval(y) == Interval(Int(3, 32, Signed)));

    inv.add(Predicate::EQ, x, y);
    BOOST_CHECK(inv.is_bottom());

    inv.set_to_top();
    inv.set(x, Interval(Int(0, 32, Signed), Int(4, 32, Signed)));
    inv.set(y, Interval(Int(1, 32, Signed), Int(5, 32, Signed)));
    inv.add(Predicate::GE, x, y);
    BOOST_CHECK(inv.to_interval(x) ==
                Interval(Int(1, 32, Signed), Int(4, 32, Signed)));
    BOOST_CHECK(inv.to_interval(y) ==
                Interval(Int(1, 32, Signed), Int(4, 32, Signed)));

    inv.set_to_top();
    inv.set(y, Interval(Int::min(32, Signed)));
    inv.add(Predicate::LT, x, y);
    BOOST_CHECK(inv.is_bottom());
  }
  {
    auto inv = PolymorphicDomain(CongruenceDomain::top());
    inv.set(x, Congruence(Int(6, 32, Signed), Int(1, 32, Signed)));
    inv.set(y, Congruence(Int(8, 32, Signed), Int(7, 32, Signed)));
    inv.add(Predicate::EQ, x, y);
    BOOST_CHECK(inv.to_congruence(x) ==
                Congruence(Int(24, 32, Signed), Int(7, 32, Signed)));
    BOOST_CHECK(inv.to_congruence(y) ==
                Congruence(Int(24, 32, Signed), Int(7, 32, Signed)));
  }
}

BOOST_AUTO_TEST_CASE(add_int) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));

  {
    auto inv = PolymorphicDomain(IntervalDomain::top());
    inv.set(x, Interval(Int(0, 32, Signed), Int(4, 32, Signed)));
    inv.add(Predicate::EQ, x, Int(1, 32, Signed));
    BOOST_CHECK(inv.to_interval(x) == Interval(Int(1, 32, Signed)));

    inv.add(Predicate::NE, x, Int(1, 32, Signed));
    BOOST_CHECK(inv.is_bottom());

    inv.set_to_top();
    inv.set(x, Interval(Int(0, 32, Signed), Int(4, 32, Signed)));
    inv.add(Predicate::GT, x, Int(2, 32, Signed));
    BOOST_CHECK(inv.to_interval(x) ==
                Interval(Int(3, 32, Signed), Int(4, 32, Signed)));

    inv.add(Predicate::LE, x, Int(3, 32, Signed));
    BOOST_CHECK(inv.to_interval(x) == Interval(Int(3, 32, Signed)));

    inv.add(Predicate::EQ, x, Int(2, 32, Signed));
    BOOST_CHECK(inv.is_bottom());

    inv.set_to_top();
    inv.add(Predicate::GT, y, Int::max(32, Signed));
    BOOST_CHECK(inv.is_bottom());

    inv.set_to_top();
    inv.add(Predicate::LT, y, Int::min(32, Signed));
    BOOST_CHECK(inv.is_bottom());
  }
  {
    auto inv = PolymorphicDomain(CongruenceDomain::top());
    inv.set(x, Congruence(Int(6, 32, Signed), Int(1, 32, Signed)));
    inv.add(Predicate::EQ, x, Int(7, 32, Signed));
    BOOST_CHECK(inv.to_congruence(x) == Congruence(Int(7, 32, Signed)));
  }
}

BOOST_AUTO_TEST_CASE(set) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));

  {
    auto inv = PolymorphicDomain(IntervalDomain::top());
    inv.set(x, Interval(Int(1, 32, Signed), Int(2, 32, Signed)));
    BOOST_CHECK(inv.to_interval(x) ==
                Interval(Int(1, 32, Signed), Int(2, 32, Signed)));

    inv.set(x, Interval::bottom(32, Signed));
    BOOST_CHECK(inv.is_bottom());
  }
  {
    auto inv = PolymorphicDomain(CongruenceDomain::top());
    inv.set(x, Congruence(Int(2, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK(inv.to_congruence(x) ==
                Congruence(Int(2, 32, Signed), Int(1, 32, Signed)));

    inv.set(x, Congruence::bottom(32, Signed));
    BOOST_CHECK(inv.is_bottom());
  }
}

BOOST_AUTO_TEST_CASE(refine) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));

  {
    auto inv = PolymorphicDomain(IntervalDomain::top());
    inv.refine(x, Interval(Int(1, 32, Signed), Int(2, 32, Signed)));
    BOOST_CHECK(inv.to_interval(x) ==
                Interval(Int(1, 32, Signed), Int(2, 32, Signed)));

    inv.refine(x, Interval(Int(3, 32, Signed), Int(4, 32, Signed)));
    BOOST_CHECK(inv.is_bottom());
  }
  {
    auto inv = PolymorphicDomain(CongruenceDomain::top());
    inv.refine(x, Congruence(Int(2, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK(inv.to_congruence(x) ==
                Congruence(Int(2, 32, Signed), Int(1, 32, Signed)));

    inv.refine(x, Congruence(Int(4, 32, Signed), Int(0, 32, Signed)));
    BOOST_CHECK(inv.is_bottom());
  }
}

BOOST_AUTO_TEST_CASE(forget) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));

  {
    auto inv = PolymorphicDomain(IntervalDomain::top());
    inv.set(x, Interval(Int(1, 32, Signed), Int(2, 32, Signed)));
    inv.set(y, Interval(Int(3, 32, Signed), Int(4, 32, Signed)));
    BOOST_CHECK(inv.to_interval(x) ==
                Interval(Int(1, 32, Signed), Int(2, 32, Signed)));
    BOOST_CHECK(inv.to_interval(y) ==
                Interval(Int(3, 32, Signed), Int(4, 32, Signed)));

    inv.forget(x);
    BOOST_CHECK(inv.to_interval(x) == Interval::top(32, Signed));
    BOOST_CHECK(inv.to_interval(y) ==
                Interval(Int(3, 32, Signed), Int(4, 32, Signed)));

    inv.forget(y);
    BOOST_CHECK(inv.is_top());
  }
  {
    auto inv = PolymorphicDomain(CongruenceDomain::top());
    inv.set(x, Congruence(Int(2, 32, Signed), Int(0, 32, Signed)));
    inv.set(y, Congruence(Int(3, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK(inv.to_congruence(x) ==
                Congruence(Int(2, 32, Signed), Int(0, 32, Signed)));
    BOOST_CHECK(inv.to_congruence(y) ==
                Congruence(Int(3, 32, Signed), Int(1, 32, Signed)));

    inv.forget(x);
    BOOST_CHECK(inv.to_congruence(x) == Congruence::top(32, Signed));
    BOOST_CHECK(inv.to_congruence(y) ==
                Congruence(Int(3, 32, Signed), Int(1, 32, Signed)));

    inv.forget(y);
    BOOST_CHECK(inv.is_top());
  }
}

BOOST_AUTO_TEST_CASE(to_interval) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));

  auto inv = PolymorphicDomain(IntervalDomain::top());
  inv.set(x, Interval(Int(1, 32, Signed), Int(2, 32, Signed)));
  inv.set(y, Interval(Int(3, 32, Signed), Int(4, 32, Signed)));

  LinearExpr e1(Int(1, 32, Signed));
  e1.add(Int(2, 32, Signed), x);
  BOOST_CHECK(inv.to_interval(e1) ==
              Interval(Int(3, 32, Signed), Int(5, 32, Signed)));

  LinearExpr e2(Int(1, 32, Signed));
  e2.add(Int(2, 32, Signed), x);
  e2.add(Int(-3, 32, Signed), y);
  BOOST_CHECK(inv.to_interval(e2) ==
              Interval(Int(-9, 32, Signed), Int(-4, 32, Signed)));
}

BOOST_AUTO_TEST_CASE(to_congruence) {
  VariableFactory vfac;
  Variable x(vfac.get("x", 32, Signed));
  Variable y(vfac.get("y", 32, Signed));

  auto inv = PolymorphicDomain(CongruenceDomain::top());
  inv.set(x, Congruence(Int(6, 32, Signed), Int(1, 32, Signed)));
  inv.set(y, Congruence(Int(8, 32, Signed), Int(7, 32, Signed)));

  LinearExpr e1(Int(1, 32, Signed));
  e1.add(Int(2, 32, Signed), x);
  BOOST_CHECK(inv.to_congruence(e1) ==
              Congruence(Int(4, 32, Signed), Int(3, 32, Signed)));

  LinearExpr e2(Int(1, 32, Signed));
  e2.add(Int(2, 32, Signed), x);
  e2.add(Int(-3, 32, Signed), y);
  BOOST_CHECK(inv.to_congruence(e2) ==
              Congruence(Int(4, 32, Signed), Int(2, 32, Signed)));
}
