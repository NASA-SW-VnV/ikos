/*******************************************************************************
 *
 * Tests for memory::PartitioningDomain
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

#define BOOST_TEST_MODULE test_memory_partitioning_domain
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/domain/machine_int/interval.hpp>
#include <ikos/core/domain/memory/dummy.hpp>
#include <ikos/core/domain/memory/partitioning.hpp>
#include <ikos/core/domain/scalar/machine_int.hpp>
#include <ikos/core/domain/uninitialized/separate_domain.hpp>
#include <ikos/core/example/memory_factory.hpp>
#include <ikos/core/example/scalar/variable_factory.hpp>

using Int = ikos::core::MachineInt;
using Interval = ikos::core::machine_int::Interval;
using ikos::core::Signed;
using ikos::core::Unsigned;
using ikos::core::machine_int::BinaryOperator;
using ikos::core::machine_int::Predicate;
using ikos::core::machine_int::UnaryOperator;
using VariableFactory = ikos::core::example::scalar::VariableFactory;
using Variable = VariableFactory::VariableRef;
using MemoryFactory = ikos::core::example::MemoryFactory;
using MemoryLocation = MemoryFactory::MemoryLocationRef;
using VariableExpr = ikos::core::VariableExpression< Int, Variable >;
using LinearExpr = ikos::core::LinearExpression< Int, Variable >;
using IntervalDomain = ikos::core::machine_int::IntervalDomain< Variable >;
using UninitializedDomain =
    ikos::core::uninitialized::SeparateDomain< Variable >;
using ScalarDomain = ikos::core::scalar::MachineIntDomain< Variable,
                                                           MemoryLocation,
                                                           UninitializedDomain,
                                                           IntervalDomain >;
using MemoryDomain =
    ikos::core::memory::DummyDomain< Variable, MemoryLocation, ScalarDomain >;
using PartitioningDomain = ikos::core::memory::
    PartitioningDomain< Variable, MemoryLocation, MemoryDomain >;

static PartitioningDomain make_top() {
  return PartitioningDomain(MemoryDomain(
      ScalarDomain(UninitializedDomain::top(), IntervalDomain::top())));
}

static PartitioningDomain make_bottom() {
  return PartitioningDomain(MemoryDomain(
      ScalarDomain(UninitializedDomain::bottom(), IntervalDomain::bottom())));
}

BOOST_AUTO_TEST_CASE(is_top_and_bottom) {
  VariableFactory vfac;
  Variable x(vfac.get_int("x", 32, Signed));

  BOOST_CHECK(make_top().is_top());
  BOOST_CHECK(!make_top().is_bottom());

  BOOST_CHECK(!make_bottom().is_top());
  BOOST_CHECK(make_bottom().is_bottom());

  auto inv = make_top();
  BOOST_CHECK(inv.is_top());
  BOOST_CHECK(!inv.is_bottom());

  inv.int_set(x, Interval(Int(1, 32, Signed)));
  BOOST_CHECK(!inv.is_top());
  BOOST_CHECK(!inv.is_bottom());

  inv.partitioning_set_variable(x);
  BOOST_CHECK(!inv.is_top());
  BOOST_CHECK(!inv.is_bottom());

  inv.int_set(x, Interval::bottom(32, Signed));
  BOOST_CHECK(!inv.is_top());
  BOOST_CHECK(inv.is_bottom());

  inv.partitioning_set_variable(x);
  BOOST_CHECK(!inv.is_top());
  BOOST_CHECK(inv.is_bottom());

  auto inv1 = make_top();
  inv1.int_assign(x, Int(1, 32, Signed));
  inv1.partitioning_set_variable(x);

  auto inv2 = make_top();
  inv2.int_assign(x, Int(3, 32, Signed));
  inv2.partitioning_set_variable(x);

  auto inv3 = inv1.join(inv2);
  BOOST_CHECK(!inv3.is_top());
  BOOST_CHECK(!inv3.is_bottom());

  inv3.int_add(Predicate::EQ, x, Int(2, 32, Signed));
  BOOST_CHECK(!inv3.is_top());
  BOOST_CHECK(inv3.is_bottom());
}

BOOST_AUTO_TEST_CASE(set_to_top_and_bottom) {
  auto inv = make_top();
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
  Variable x(vfac.get_int("x", 32, Signed));
  Variable y(vfac.get_int("y", 32, Signed));
  Variable z(vfac.get_int("z", 32, Signed));

  BOOST_CHECK(make_bottom().leq(make_top()));
  BOOST_CHECK(make_bottom().leq(make_bottom()));
  BOOST_CHECK(!make_top().leq(make_bottom()));
  BOOST_CHECK(make_top().leq(make_top()));

  auto inv1 = make_top();
  inv1.int_set(x, Interval(Int(0, 32, Signed)));
  BOOST_CHECK(inv1.leq(make_top()));
  BOOST_CHECK(!inv1.leq(make_bottom()));

  auto inv2 = make_top();
  inv2.int_set(x, Interval(Int(-1, 32, Signed), Int(1, 32, Signed)));
  BOOST_CHECK(inv2.leq(make_top()));
  BOOST_CHECK(!inv2.leq(make_bottom()));
  BOOST_CHECK(inv1.leq(inv2));
  BOOST_CHECK(!inv2.leq(inv1));

  auto inv3 = make_top();
  inv3.int_set(x, Interval(Int(0, 32, Signed)));
  inv3.int_set(y, Interval(Int(-1, 32, Signed), Int(1, 32, Signed)));
  BOOST_CHECK(inv3.leq(make_top()));
  BOOST_CHECK(!inv3.leq(make_bottom()));
  BOOST_CHECK(inv3.leq(inv1));
  BOOST_CHECK(!inv1.leq(inv3));

  auto inv4 = make_top();
  inv4.int_set(x, Interval(Int(0, 32, Signed)));
  inv4.int_set(y, Interval(Int(0, 32, Signed), Int(2, 32, Signed)));
  BOOST_CHECK(inv4.leq(make_top()));
  BOOST_CHECK(!inv4.leq(make_bottom()));
  BOOST_CHECK(!inv3.leq(inv4));
  BOOST_CHECK(!inv4.leq(inv3));

  auto inv5 = make_top();
  inv5.int_set(x, Interval(Int(0, 32, Signed)));
  inv5.int_set(y, Interval(Int(0, 32, Signed), Int(2, 32, Signed)));
  inv5.int_set(z, Interval(Int::min(32, Signed), Int(0, 32, Signed)));
  BOOST_CHECK(inv5.leq(make_top()));
  BOOST_CHECK(!inv5.leq(make_bottom()));
  BOOST_CHECK(!inv5.leq(inv3));
  BOOST_CHECK(!inv3.leq(inv5));
  BOOST_CHECK(inv5.leq(inv4));
  BOOST_CHECK(!inv4.leq(inv5));

  auto inv6 = make_top();
  inv6.int_set(x, Interval(Int(1, 32, Signed), Int(9, 32, Signed)));
  inv6.partitioning_set_variable(x);

  auto inv7 = make_top();
  inv7.int_set(x, Interval(Int(-9, 32, Signed), Int(-1, 32, Signed)));
  inv7.partitioning_set_variable(x);

  auto inv8 = inv6.join(inv7);

  auto inv9 = inv8;
  inv9.int_refine(x, Interval(Int(-3, 32, Signed), Int(3, 32, Signed)));
  BOOST_CHECK(inv8.leq(make_top()));
  BOOST_CHECK(!inv8.leq(make_bottom()));
  BOOST_CHECK(!inv8.leq(inv9));
  BOOST_CHECK(inv9.leq(inv8));
}

BOOST_AUTO_TEST_CASE(equals) {
  VariableFactory vfac;
  Variable x(vfac.get_int("x", 32, Signed));
  Variable y(vfac.get_int("y", 32, Signed));

  BOOST_CHECK(!make_bottom().equals(make_top()));
  BOOST_CHECK(make_bottom().equals(make_bottom()));
  BOOST_CHECK(!make_top().equals(make_bottom()));
  BOOST_CHECK(make_top().equals(make_top()));

  auto inv1 = make_top();
  inv1.int_set(x, Interval(Int(0, 32, Signed)));
  BOOST_CHECK(!inv1.equals(make_top()));
  BOOST_CHECK(!inv1.equals(make_bottom()));
  BOOST_CHECK(inv1.equals(inv1));

  auto inv2 = make_top();
  inv2.int_set(x, Interval(Int(-1, 32, Signed), Int(1, 32, Signed)));
  BOOST_CHECK(!inv2.equals(make_top()));
  BOOST_CHECK(!inv2.equals(make_bottom()));
  BOOST_CHECK(!inv1.equals(inv2));
  BOOST_CHECK(!inv2.equals(inv1));

  auto inv3 = make_top();
  inv3.int_set(x, Interval(Int(0, 32, Signed)));
  inv3.int_set(y, Interval(Int(-1, 32, Signed), Int(1, 32, Signed)));
  BOOST_CHECK(!inv3.equals(make_top()));
  BOOST_CHECK(!inv3.equals(make_bottom()));
  BOOST_CHECK(!inv3.equals(inv1));
  BOOST_CHECK(!inv1.equals(inv3));
}

BOOST_AUTO_TEST_CASE(join) {
  VariableFactory vfac;
  Variable x(vfac.get_int("x", 32, Signed));
  Variable y(vfac.get_int("y", 32, Signed));

  BOOST_CHECK((make_bottom().join(make_top()) == make_top()));
  BOOST_CHECK((make_bottom().join(make_bottom()) == make_bottom()));
  BOOST_CHECK((make_top().join(make_top()) == make_top()));
  BOOST_CHECK((make_top().join(make_bottom()) == make_top()));

  auto inv1 = make_top();
  inv1.int_set(x, Interval(Int(0, 32, Signed), Int(1, 32, Signed)));
  BOOST_CHECK((inv1.join(make_top()) == make_top()));
  BOOST_CHECK((inv1.join(make_bottom()) == inv1));
  BOOST_CHECK((make_top().join(inv1) == make_top()));
  BOOST_CHECK((make_bottom().join(inv1) == inv1));
  BOOST_CHECK((inv1.join(inv1) == inv1));

  auto inv2 = make_top();
  auto inv3 = make_top();
  inv2.int_set(x, Interval(Int(-1, 32, Signed), Int(0, 32, Signed)));
  inv3.int_set(x, Interval(Int(-1, 32, Signed), Int(1, 32, Signed)));
  BOOST_CHECK((inv1.join(inv2) == inv3));
  BOOST_CHECK((inv2.join(inv1) == inv3));

  auto inv4 = make_top();
  inv4.int_set(x, Interval(Int(-1, 32, Signed), Int(0, 32, Signed)));
  inv4.int_set(y, Interval(Int(0, 32, Signed)));
  BOOST_CHECK((inv4.join(inv2) == inv2));
  BOOST_CHECK((inv2.join(inv4) == inv2));

  auto inv5 = make_top();
  inv5.int_set(x, Interval(Int(1, 32, Signed), Int(9, 32, Signed)));
  inv5.int_set(y, Interval(Int(1, 32, Signed), Int(2, 32, Signed)));
  inv5.partitioning_set_variable(x);

  auto inv6 = make_top();
  inv6.int_set(x, Interval(Int(-9, 32, Signed), Int(-1, 32, Signed)));
  inv6.int_set(y, Interval(Int(4, 32, Signed), Int(5, 32, Signed)));
  inv6.partitioning_set_variable(x);

  BOOST_CHECK((inv5.join(inv6).int_to_interval(x) ==
               Interval(Int(-9, 32, Signed), Int(9, 32, Signed))));
  BOOST_CHECK((inv6.join(inv5).int_to_interval(x) ==
               Interval(Int(-9, 32, Signed), Int(9, 32, Signed))));
  BOOST_CHECK((inv5.join(inv6).int_to_interval(y) ==
               Interval(Int(1, 32, Signed), Int(5, 32, Signed))));
  BOOST_CHECK((inv6.join(inv5).int_to_interval(y) ==
               Interval(Int(1, 32, Signed), Int(5, 32, Signed))));

  auto inv7 = inv5.join(inv6);
  inv7.int_add(Predicate::LT, y, Int(4, 32, Signed));
  BOOST_CHECK((inv7 == inv5));

  auto inv8 = make_top();
  inv8.int_set(x, Interval(Int(-1, 32, Signed), Int(1, 32, Signed)));
  inv8.int_set(y, Interval(Int(7, 32, Signed), Int(8, 32, Signed)));
  inv8.partitioning_set_variable(x);

  BOOST_CHECK((inv5.join(inv6).join(inv8) == inv8.join(inv5.join(inv6))));

  auto inv9 = inv5.join(inv6).join(inv8);
  inv9.int_add(Predicate::NE, x, Int(0, 32, Signed));
  BOOST_CHECK((inv9.int_to_interval(x) ==
               Interval(Int(-9, 32, Signed), Int(9, 32, Signed))));
  BOOST_CHECK((inv9.int_to_interval(y) ==
               Interval(Int(1, 32, Signed), Int(8, 32, Signed))));
}

BOOST_AUTO_TEST_CASE(widening) {
  VariableFactory vfac;
  Variable x(vfac.get_int("x", 32, Signed));

  BOOST_CHECK((make_bottom().widening(make_top()) == make_top()));
  BOOST_CHECK((make_bottom().widening(make_bottom()) == make_bottom()));
  BOOST_CHECK((make_top().widening(make_top()) == make_top()));
  BOOST_CHECK((make_top().widening(make_bottom()) == make_top()));

  {
    auto inv1 = make_top();
    inv1.int_set(x, Interval(Int(0, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK((inv1.widening(make_top()) == make_top()));
    BOOST_CHECK((inv1.widening(make_bottom()) == inv1));
    BOOST_CHECK((make_top().widening(inv1) == make_top()));
    BOOST_CHECK((make_bottom().widening(inv1) == inv1));
    BOOST_CHECK((inv1.widening(inv1) == inv1));

    auto inv2 = make_top();
    auto inv3 = make_top();
    inv2.int_set(x, Interval(Int(0, 32, Signed), Int(2, 32, Signed)));
    inv3.int_set(x, Interval(Int(0, 32, Signed), Int::max(32, Signed)));
    BOOST_CHECK((inv1.widening(inv2) == inv3));
    BOOST_CHECK((inv2.widening(inv1) == inv2));
  }

  {
    auto inv1 = make_top();
    inv1.int_set(x, Interval(Int(0, 32, Signed), Int(1, 32, Signed)));
    inv1.partitioning_set_variable(x);
    BOOST_CHECK((inv1.widening(make_top()) == make_top()));
    BOOST_CHECK((inv1.widening(make_bottom()) == inv1));
    BOOST_CHECK((make_top().widening(inv1) == make_top()));
    BOOST_CHECK((make_bottom().widening(inv1) == inv1));
    BOOST_CHECK((inv1.widening(inv1) == inv1));

    auto inv2 = make_top();
    auto inv3 = make_top();
    inv2.int_set(x, Interval(Int(0, 32, Signed), Int(2, 32, Signed)));
    inv2.partitioning_set_variable(x);
    inv3.int_set(x, Interval(Int(0, 32, Signed), Int::max(32, Signed)));
    inv3.partitioning_set_variable(x);
    BOOST_CHECK((inv1.widening(inv2) == inv3));
    BOOST_CHECK((inv2.widening(inv1) == inv2));
  }
}

BOOST_AUTO_TEST_CASE(meet) {
  VariableFactory vfac;
  Variable x(vfac.get_int("x", 32, Signed));
  Variable y(vfac.get_int("y", 32, Signed));

  BOOST_CHECK((make_bottom().meet(make_top()) == make_bottom()));
  BOOST_CHECK((make_bottom().meet(make_bottom()) == make_bottom()));
  BOOST_CHECK((make_top().meet(make_top()) == make_top()));
  BOOST_CHECK((make_top().meet(make_bottom()) == make_bottom()));

  {
    auto inv1 = make_top();
    inv1.int_set(x, Interval(Int(0, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK((inv1.meet(make_top()) == inv1));
    BOOST_CHECK((inv1.meet(make_bottom()) == make_bottom()));
    BOOST_CHECK((make_top().meet(inv1) == inv1));
    BOOST_CHECK((make_bottom().meet(inv1) == make_bottom()));
    BOOST_CHECK((inv1.meet(inv1) == inv1));

    auto inv2 = make_top();
    auto inv3 = make_top();
    inv2.int_set(x, Interval(Int(-1, 32, Signed), Int(0, 32, Signed)));
    inv3.int_set(x, Interval(Int(0, 32, Signed)));
    BOOST_CHECK((inv1.meet(inv2) == inv3));
    BOOST_CHECK((inv2.meet(inv1) == inv3));

    auto inv4 = make_top();
    auto inv5 = make_top();
    inv4.int_set(x, Interval(Int(0, 32, Signed), Int(1, 32, Signed)));
    inv4.int_set(y, Interval(Int(0, 32, Signed)));
    inv5.int_set(x, Interval(Int(0, 32, Signed)));
    inv5.int_set(y, Interval(Int(0, 32, Signed)));
    BOOST_CHECK((inv4.meet(inv2) == inv5));
    BOOST_CHECK((inv2.meet(inv4) == inv5));
  }

  {
    auto inv1 = make_top();
    inv1.int_set(x, Interval(Int(0, 32, Signed), Int(1, 32, Signed)));
    inv1.partitioning_set_variable(x);
    BOOST_CHECK((inv1.meet(make_top()) == inv1));
    BOOST_CHECK((inv1.meet(make_bottom()) == make_bottom()));
    BOOST_CHECK((make_top().meet(inv1) == inv1));
    BOOST_CHECK((make_bottom().meet(inv1) == make_bottom()));
    BOOST_CHECK((inv1.meet(inv1) == inv1));

    auto inv2 = make_top();
    auto inv3 = make_top();
    inv2.int_set(x, Interval(Int(-1, 32, Signed), Int(0, 32, Signed)));
    inv2.partitioning_set_variable(x);
    inv3.int_set(x, Interval(Int(0, 32, Signed)));
    inv3.partitioning_set_variable(x);
    BOOST_CHECK((inv1.meet(inv2) == inv3));
    BOOST_CHECK((inv2.meet(inv1) == inv3));

    auto inv4 = make_top();
    auto inv5 = make_top();
    inv4.int_set(x, Interval(Int(0, 32, Signed), Int(1, 32, Signed)));
    inv4.int_set(y, Interval(Int(0, 32, Signed)));
    inv4.partitioning_set_variable(x);
    inv5.int_set(x, Interval(Int(0, 32, Signed)));
    inv5.int_set(y, Interval(Int(0, 32, Signed)));
    inv5.partitioning_set_variable(x);
    BOOST_CHECK((inv4.meet(inv2) == inv5));
    BOOST_CHECK((inv2.meet(inv4) == inv5));
  }
}

BOOST_AUTO_TEST_CASE(narrowing) {
  VariableFactory vfac;
  Variable x(vfac.get_int("x", 32, Signed));

  BOOST_CHECK((make_bottom().narrowing(make_top()) == make_bottom()));
  BOOST_CHECK((make_bottom().narrowing(make_bottom()) == make_bottom()));
  BOOST_CHECK((make_top().narrowing(make_top()) == make_top()));
  BOOST_CHECK((make_top().narrowing(make_bottom()) == make_bottom()));

  {
    auto inv1 = make_top();
    inv1.int_set(x, Interval(Int(0, 32, Signed), Int::max(32, Signed)));
    BOOST_CHECK((inv1.narrowing(make_top()) == inv1));
    BOOST_CHECK((inv1.narrowing(make_bottom()) == make_bottom()));
    BOOST_CHECK((make_top().narrowing(inv1) == inv1));
    BOOST_CHECK((make_bottom().narrowing(inv1) == make_bottom()));
    BOOST_CHECK((inv1.narrowing(inv1) == inv1));

    auto inv2 = make_top();
    inv2.int_set(x, Interval(Int(0, 32, Signed), Int(1, 32, Signed)));
    BOOST_CHECK((inv1.narrowing(inv2) == inv2));
    BOOST_CHECK((inv2.narrowing(inv1) == inv2));
  }

  {
    auto inv1 = make_top();
    inv1.int_set(x, Interval(Int(0, 32, Signed), Int::max(32, Signed)));
    inv1.partitioning_set_variable(x);
    BOOST_CHECK((inv1.narrowing(make_top()) == inv1));
    BOOST_CHECK((inv1.narrowing(make_bottom()) == make_bottom()));
    BOOST_CHECK((make_top().narrowing(inv1) == inv1));
    BOOST_CHECK((make_bottom().narrowing(inv1) == make_bottom()));
    BOOST_CHECK((inv1.narrowing(inv1) == inv1));

    auto inv2 = make_top();
    inv2.int_set(x, Interval(Int(0, 32, Signed), Int(1, 32, Signed)));
    inv2.partitioning_set_variable(x);
    BOOST_CHECK((inv1.narrowing(inv2) == inv2));
    BOOST_CHECK((inv2.narrowing(inv1) == inv2));
  }
}

BOOST_AUTO_TEST_CASE(assign) {
  VariableFactory vfac;
  Variable x(vfac.get_int("x", 32, Signed));
  Variable y(vfac.get_int("y", 32, Signed));
  Variable z(vfac.get_int("z", 32, Signed));

  auto inv1 = make_top();
  auto inv2 = make_top();
  inv1.int_assign(x, Int(0, 32, Signed));
  inv2.int_set(x, Interval(Int(0, 32, Signed)));
  BOOST_CHECK((inv1 == inv2));

  inv1.set_to_bottom();
  inv1.int_assign(x, Int(0, 32, Signed));
  BOOST_CHECK(inv1.is_bottom());

  inv1.set_to_top();
  inv1.int_set(x, Interval(Int(-1, 32, Signed), Int(1, 32, Signed)));
  inv1.int_assign(y, x);
  BOOST_CHECK(inv1.int_to_interval(y) ==
              Interval(Int(-1, 32, Signed), Int(1, 32, Signed)));

  inv1.set_to_top();
  inv1.int_set(x, Interval(Int(-1, 32, Signed), Int(1, 32, Signed)));
  inv1.int_set(y, Interval(Int(1, 32, Signed), Int(2, 32, Signed)));

  LinearExpr e(Int(1, 32, Signed));
  e.add(Int(2, 32, Signed), x);
  e.add(Int(-3, 32, Signed), y);
  inv1.int_assign(z, e);

  BOOST_CHECK(inv1.int_to_interval(z) ==
              Interval(Int(-7, 32, Signed), Int(0, 32, Signed)));
}

BOOST_AUTO_TEST_CASE(unary_apply) {
  VariableFactory vfac;
  Variable x(vfac.get_int("x", 8, Signed));
  Variable y(vfac.get_int("y", 6, Signed));
  Variable z(vfac.get_int("z", 8, Signed));
  Variable w(vfac.get_int("w", 8, Unsigned));

  auto inv = make_top();
  inv.int_assign(x, Int(85, 8, Signed));
  BOOST_CHECK(inv.int_to_interval(x) == Interval(Int(85, 8, Signed)));
  inv.int_apply(UnaryOperator::Trunc, y, x);
  BOOST_CHECK(inv.int_to_interval(y) == Interval(Int(21, 6, Signed)));
  inv.int_apply(UnaryOperator::Ext, z, y);
  BOOST_CHECK(inv.int_to_interval(z) == Interval(Int(21, 8, Signed)));
  inv.int_apply(UnaryOperator::SignCast, w, z);
  BOOST_CHECK(inv.int_to_interval(w) == Interval(Int(21, 8, Unsigned)));
}

BOOST_AUTO_TEST_CASE(binary_apply) {
  VariableFactory vfac;
  Variable x(vfac.get_int("x", 8, Signed));
  Variable y(vfac.get_int("y", 8, Signed));
  Variable z(vfac.get_int("z", 8, Signed));

  auto inv = make_top();
  inv.int_assign(x, Int(85, 8, Signed));
  BOOST_CHECK(inv.int_to_interval(x) == Interval(Int(85, 8, Signed)));
  inv.int_apply(BinaryOperator::Add, y, x, Int(43, 8, Signed));
  BOOST_CHECK(inv.int_to_interval(y) == Interval(Int(-128, 8, Signed)));
  inv.int_apply(BinaryOperator::SubNoWrap, z, y, Int(1, 8, Signed));
  BOOST_CHECK(inv.is_bottom());
}

BOOST_AUTO_TEST_CASE(add_var) {
  VariableFactory vfac;
  Variable x(vfac.get_int("x", 32, Signed));
  Variable y(vfac.get_int("y", 32, Signed));

  auto inv = make_top();
  inv.int_set(x, Interval(Int(0, 32, Signed), Int(4, 32, Signed)));
  inv.int_set(y, Interval(Int(-4, 32, Signed), Int(0, 32, Signed)));
  inv.int_add(Predicate::EQ, x, y);
  BOOST_CHECK(inv.int_to_interval(x) == Interval(Int(0, 32, Signed)));
  BOOST_CHECK(inv.int_to_interval(y) == Interval(Int(0, 32, Signed)));

  inv.int_add(Predicate::NE, x, y);
  BOOST_CHECK(inv.is_bottom());

  inv.set_to_top();
  inv.int_set(x, Interval(Int(0, 32, Signed), Int(4, 32, Signed)));
  inv.int_set(y, Interval(Int(1, 32, Signed), Int(5, 32, Signed)));
  inv.int_add(Predicate::GT, x, y);
  BOOST_CHECK(inv.int_to_interval(x) ==
              Interval(Int(2, 32, Signed), Int(4, 32, Signed)));
  BOOST_CHECK(inv.int_to_interval(y) ==
              Interval(Int(1, 32, Signed), Int(3, 32, Signed)));

  inv.int_add(Predicate::LE, x, y);
  BOOST_CHECK(inv.int_to_interval(x) ==
              Interval(Int(2, 32, Signed), Int(3, 32, Signed)));
  BOOST_CHECK(inv.int_to_interval(y) ==
              Interval(Int(2, 32, Signed), Int(3, 32, Signed)));

  inv.int_add(Predicate::LT, x, y);
  BOOST_CHECK(inv.int_to_interval(x) == Interval(Int(2, 32, Signed)));
  BOOST_CHECK(inv.int_to_interval(y) == Interval(Int(3, 32, Signed)));

  inv.int_add(Predicate::EQ, x, y);
  BOOST_CHECK(inv.is_bottom());

  inv.set_to_top();
  inv.int_set(x, Interval(Int(0, 32, Signed), Int(4, 32, Signed)));
  inv.int_set(y, Interval(Int(1, 32, Signed), Int(5, 32, Signed)));
  inv.int_add(Predicate::GE, x, y);
  BOOST_CHECK(inv.int_to_interval(x) ==
              Interval(Int(1, 32, Signed), Int(4, 32, Signed)));
  BOOST_CHECK(inv.int_to_interval(y) ==
              Interval(Int(1, 32, Signed), Int(4, 32, Signed)));

  inv.set_to_top();
  inv.int_set(y, Interval(Int::min(32, Signed)));
  inv.int_add(Predicate::LT, x, y);
  BOOST_CHECK(inv.is_bottom());
}

BOOST_AUTO_TEST_CASE(add_int) {
  VariableFactory vfac;
  Variable x(vfac.get_int("x", 32, Signed));
  Variable y(vfac.get_int("y", 32, Signed));

  auto inv = make_top();
  inv.int_set(x, Interval(Int(0, 32, Signed), Int(4, 32, Signed)));
  inv.int_add(Predicate::EQ, x, Int(1, 32, Signed));
  BOOST_CHECK(inv.int_to_interval(x) == Interval(Int(1, 32, Signed)));

  inv.int_add(Predicate::NE, x, Int(1, 32, Signed));
  BOOST_CHECK(inv.is_bottom());

  inv.set_to_top();
  inv.int_set(x, Interval(Int(0, 32, Signed), Int(4, 32, Signed)));
  inv.int_add(Predicate::GT, x, Int(2, 32, Signed));
  BOOST_CHECK(inv.int_to_interval(x) ==
              Interval(Int(3, 32, Signed), Int(4, 32, Signed)));

  inv.int_add(Predicate::LE, x, Int(3, 32, Signed));
  BOOST_CHECK(inv.int_to_interval(x) == Interval(Int(3, 32, Signed)));

  inv.int_add(Predicate::EQ, x, Int(2, 32, Signed));
  BOOST_CHECK(inv.is_bottom());

  inv.set_to_top();
  inv.int_add(Predicate::GT, y, Int::max(32, Signed));
  BOOST_CHECK(inv.is_bottom());

  inv.set_to_top();
  inv.int_add(Predicate::LT, y, Int::min(32, Signed));
  BOOST_CHECK(inv.is_bottom());
}

BOOST_AUTO_TEST_CASE(set) {
  VariableFactory vfac;
  Variable x(vfac.get_int("x", 32, Signed));

  auto inv = make_top();
  inv.int_set(x, Interval(Int(1, 32, Signed), Int(2, 32, Signed)));
  BOOST_CHECK(inv.int_to_interval(x) ==
              Interval(Int(1, 32, Signed), Int(2, 32, Signed)));

  inv.int_set(x, Interval::bottom(32, Signed));
  BOOST_CHECK(inv.is_bottom());
}

BOOST_AUTO_TEST_CASE(refine) {
  VariableFactory vfac;
  Variable x(vfac.get_int("x", 32, Signed));

  auto inv = make_top();
  inv.int_refine(x, Interval(Int(1, 32, Signed), Int(2, 32, Signed)));
  BOOST_CHECK(inv.int_to_interval(x) ==
              Interval(Int(1, 32, Signed), Int(2, 32, Signed)));

  inv.int_refine(x, Interval(Int(3, 32, Signed), Int(4, 32, Signed)));
  BOOST_CHECK(inv.is_bottom());
}

BOOST_AUTO_TEST_CASE(forget) {
  VariableFactory vfac;
  Variable x(vfac.get_int("x", 32, Signed));
  Variable y(vfac.get_int("y", 32, Signed));

  auto inv = make_top();
  inv.int_set(x, Interval(Int(1, 32, Signed), Int(2, 32, Signed)));
  inv.int_set(y, Interval(Int(3, 32, Signed), Int(4, 32, Signed)));
  BOOST_CHECK(inv.int_to_interval(x) ==
              Interval(Int(1, 32, Signed), Int(2, 32, Signed)));
  BOOST_CHECK(inv.int_to_interval(y) ==
              Interval(Int(3, 32, Signed), Int(4, 32, Signed)));

  inv.int_forget(x);
  BOOST_CHECK(inv.int_to_interval(x) == Interval::top(32, Signed));
  BOOST_CHECK(inv.int_to_interval(y) ==
              Interval(Int(3, 32, Signed), Int(4, 32, Signed)));

  inv.int_forget(y);
  BOOST_CHECK(inv.is_top());
}

BOOST_AUTO_TEST_CASE(to_interval) {
  VariableFactory vfac;
  Variable x(vfac.get_int("x", 32, Signed));
  Variable y(vfac.get_int("y", 32, Signed));

  auto inv = make_top();
  inv.int_set(x, Interval(Int(1, 32, Signed), Int(2, 32, Signed)));
  inv.int_set(y, Interval(Int(3, 32, Signed), Int(4, 32, Signed)));

  LinearExpr e1(Int(1, 32, Signed));
  e1.add(Int(2, 32, Signed), x);
  BOOST_CHECK(inv.int_to_interval(e1) ==
              Interval(Int(3, 32, Signed), Int(5, 32, Signed)));

  LinearExpr e2(Int(1, 32, Signed));
  e2.add(Int(2, 32, Signed), x);
  e2.add(Int(-3, 32, Signed), y);
  BOOST_CHECK(inv.int_to_interval(e2) ==
              Interval(Int(-9, 32, Signed), Int(-4, 32, Signed)));
}
