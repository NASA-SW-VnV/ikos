/*******************************************************************************
 *
 * Tests for DiscreteDomain
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

#define BOOST_TEST_MODULE test_discrete_domain
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/domain/discrete_domain.hpp>
#include <ikos/core/example/variable_factory.hpp>

using VariableFactory = ikos::core::example::VariableFactory;
using Variable = ikos::core::example::VariableFactory::VariableRef;
using DiscreteDomain = ikos::core::DiscreteDomain< Variable >;

BOOST_AUTO_TEST_CASE(is_top_and_bottom) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  BOOST_CHECK(DiscreteDomain::top().is_top());
  BOOST_CHECK(!DiscreteDomain::top().is_bottom());

  BOOST_CHECK(!DiscreteDomain::bottom().is_top());
  BOOST_CHECK(DiscreteDomain::bottom().is_bottom());

  auto inv = DiscreteDomain::top();
  BOOST_CHECK(inv.is_top());
  BOOST_CHECK(!inv.is_bottom());

  inv.set_to_bottom();
  BOOST_CHECK(!inv.is_top());
  BOOST_CHECK(inv.is_bottom());

  inv.add(x);
  BOOST_CHECK(!inv.is_top());
  BOOST_CHECK(!inv.is_bottom());
}

BOOST_AUTO_TEST_CASE(iterators) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  DiscreteDomain inv = DiscreteDomain::bottom();
  BOOST_CHECK((inv.begin() == inv.end()));

  inv.add(x);
  std::array< Variable, 1 > tab = {{x}};
  BOOST_CHECK(
      std::equal(inv.begin(),
                 inv.end(),
                 std::begin(tab),
                 std::end(tab),
                 [](const Variable& a, const Variable& b) { return a == b; }));

  inv.add(y);
  std::array< Variable, 2 > tab2 = {{y, x}};
  BOOST_CHECK(
      std::equal(inv.begin(),
                 inv.end(),
                 std::begin(tab2),
                 std::end(tab2),
                 [](const Variable& a, const Variable& b) { return a == b; }));
}

BOOST_AUTO_TEST_CASE(set_to_top_and_bottom) {
  VariableFactory vfac;

  auto inv = DiscreteDomain::top();
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

  BOOST_CHECK(DiscreteDomain::bottom().leq(DiscreteDomain::top()));
  BOOST_CHECK(DiscreteDomain::bottom().leq(DiscreteDomain::bottom()));
  BOOST_CHECK(!DiscreteDomain::top().leq(DiscreteDomain::bottom()));
  BOOST_CHECK(DiscreteDomain::top().leq(DiscreteDomain::top()));

  DiscreteDomain inv1{x};
  BOOST_CHECK(inv1.leq(DiscreteDomain::top()));
  BOOST_CHECK(!inv1.leq(DiscreteDomain::bottom()));

  DiscreteDomain inv2{x, y};
  BOOST_CHECK(inv2.leq(DiscreteDomain::top()));
  BOOST_CHECK(!inv2.leq(DiscreteDomain::bottom()));
  BOOST_CHECK(inv1.leq(inv2));
  BOOST_CHECK(!inv2.leq(inv1));

  DiscreteDomain inv3{y};
  BOOST_CHECK(inv3.leq(DiscreteDomain::top()));
  BOOST_CHECK(!inv3.leq(DiscreteDomain::bottom()));
  BOOST_CHECK(!inv3.leq(inv1));
  BOOST_CHECK(!inv1.leq(inv3));
  BOOST_CHECK(inv3.leq(inv2));
  BOOST_CHECK(!inv2.leq(inv3));
}

BOOST_AUTO_TEST_CASE(equals) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  BOOST_CHECK(!DiscreteDomain::bottom().equals(DiscreteDomain::top()));
  BOOST_CHECK(DiscreteDomain::bottom().equals(DiscreteDomain::bottom()));
  BOOST_CHECK(!DiscreteDomain::top().equals(DiscreteDomain::bottom()));
  BOOST_CHECK(DiscreteDomain::top().equals(DiscreteDomain::top()));

  DiscreteDomain inv1{x};
  BOOST_CHECK(!inv1.equals(DiscreteDomain::top()));
  BOOST_CHECK(!inv1.equals(DiscreteDomain::bottom()));
  BOOST_CHECK(inv1.equals(inv1));

  DiscreteDomain inv2{y};
  BOOST_CHECK(!inv2.equals(DiscreteDomain::top()));
  BOOST_CHECK(!inv2.equals(DiscreteDomain::bottom()));
  BOOST_CHECK(!inv1.equals(inv2));
  BOOST_CHECK(!inv2.equals(inv1));

  DiscreteDomain inv3{x, y};
  BOOST_CHECK(!inv3.equals(DiscreteDomain::top()));
  BOOST_CHECK(!inv3.equals(DiscreteDomain::bottom()));
  BOOST_CHECK(!inv3.equals(inv1));
  BOOST_CHECK(!inv1.equals(inv3));
}

BOOST_AUTO_TEST_CASE(join) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));

  BOOST_CHECK((DiscreteDomain::bottom().join(DiscreteDomain::top()) ==
               DiscreteDomain::top()));
  BOOST_CHECK((DiscreteDomain::bottom().join(DiscreteDomain::bottom()) ==
               DiscreteDomain::bottom()));
  BOOST_CHECK((DiscreteDomain::top().join(DiscreteDomain::top()) ==
               DiscreteDomain::top()));
  BOOST_CHECK((DiscreteDomain::top().join(DiscreteDomain::bottom()) ==
               DiscreteDomain::top()));

  DiscreteDomain inv1{x};
  BOOST_CHECK((inv1.join(DiscreteDomain::top()) == DiscreteDomain::top()));
  BOOST_CHECK((inv1.join(DiscreteDomain::bottom()) == inv1));
  BOOST_CHECK((DiscreteDomain::top().join(inv1) == DiscreteDomain::top()));
  BOOST_CHECK((DiscreteDomain::bottom().join(inv1) == inv1));
  BOOST_CHECK((inv1.join(inv1) == inv1));

  DiscreteDomain inv2{y};
  DiscreteDomain inv3{x, y};
  BOOST_CHECK((inv1.join(inv2) == inv3));
  BOOST_CHECK((inv2.join(inv1) == inv3));

  DiscreteDomain inv4{y, z};
  BOOST_CHECK((inv4.join(inv3) == DiscreteDomain{x, y, z}));
  BOOST_CHECK((inv3.join(inv4) == DiscreteDomain{x, y, z}));
}

BOOST_AUTO_TEST_CASE(meet) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));

  BOOST_CHECK((DiscreteDomain::bottom().meet(DiscreteDomain::top()) ==
               DiscreteDomain::bottom()));
  BOOST_CHECK((DiscreteDomain::bottom().meet(DiscreteDomain::bottom()) ==
               DiscreteDomain::bottom()));
  BOOST_CHECK((DiscreteDomain::top().meet(DiscreteDomain::top()) ==
               DiscreteDomain::top()));
  BOOST_CHECK((DiscreteDomain::top().meet(DiscreteDomain::bottom()) ==
               DiscreteDomain::bottom()));

  DiscreteDomain inv1{x};
  BOOST_CHECK((inv1.meet(DiscreteDomain::top()) == inv1));
  BOOST_CHECK(
      (inv1.meet(DiscreteDomain::bottom()) == DiscreteDomain::bottom()));
  BOOST_CHECK((DiscreteDomain::top().meet(inv1) == inv1));
  BOOST_CHECK(
      (DiscreteDomain::bottom().meet(inv1) == DiscreteDomain::bottom()));
  BOOST_CHECK((inv1.meet(inv1) == inv1));

  DiscreteDomain inv2{y};
  BOOST_CHECK((inv1.meet(inv2) == DiscreteDomain::bottom()));
  BOOST_CHECK((inv2.meet(inv1) == DiscreteDomain::bottom()));

  DiscreteDomain inv3{x, y};
  BOOST_CHECK((inv1.meet(inv3) == inv1));
  BOOST_CHECK((inv3.meet(inv1) == inv1));
  BOOST_CHECK((inv2.meet(inv3) == inv2));
  BOOST_CHECK((inv3.meet(inv2) == inv2));

  DiscreteDomain inv4{y, z};
  BOOST_CHECK((inv3.meet(inv4) == inv2));
  BOOST_CHECK((inv4.meet(inv3) == inv2));
}

BOOST_AUTO_TEST_CASE(difference) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));

  BOOST_CHECK((DiscreteDomain::bottom().difference(DiscreteDomain::top()) ==
               DiscreteDomain::bottom()));
  BOOST_CHECK((DiscreteDomain::bottom().difference(DiscreteDomain::bottom()) ==
               DiscreteDomain::bottom()));
  BOOST_CHECK((DiscreteDomain::top().difference(DiscreteDomain::top()) ==
               DiscreteDomain::bottom()));
  BOOST_CHECK((DiscreteDomain::top().difference(DiscreteDomain::bottom()) ==
               DiscreteDomain::top()));

  DiscreteDomain inv1{x};
  BOOST_CHECK(
      (inv1.difference(DiscreteDomain::top()) == DiscreteDomain::bottom()));
  BOOST_CHECK((inv1.difference(DiscreteDomain::bottom()) == inv1));
  BOOST_CHECK(
      (DiscreteDomain::top().difference(inv1) == DiscreteDomain::top()));
  BOOST_CHECK(
      (DiscreteDomain::bottom().difference(inv1) == DiscreteDomain::bottom()));
  BOOST_CHECK((inv1.difference(inv1) == DiscreteDomain::bottom()));

  DiscreteDomain inv2{y};
  BOOST_CHECK((inv1.difference(inv2) == inv1));
  BOOST_CHECK((inv2.difference(inv1) == inv2));

  DiscreteDomain inv3{x, y};
  BOOST_CHECK((inv1.difference(inv3) == DiscreteDomain::bottom()));
  BOOST_CHECK((inv3.difference(inv1) == inv2));
  BOOST_CHECK((inv2.difference(inv3) == DiscreteDomain::bottom()));
  BOOST_CHECK((inv3.difference(inv2) == inv1));

  DiscreteDomain inv4{y, z};
  BOOST_CHECK((inv3.difference(inv4) == inv1));
  BOOST_CHECK((inv4.difference(inv3) == DiscreteDomain{z}));
}

BOOST_AUTO_TEST_CASE(add) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto inv = DiscreteDomain::top();
  BOOST_CHECK(inv.is_top());
  inv.add(x);
  BOOST_CHECK(inv.is_top());

  inv = DiscreteDomain::bottom();
  inv.add(x);
  BOOST_CHECK((inv == DiscreteDomain{x}));
  inv.add(y);
  BOOST_CHECK((inv == DiscreteDomain{x, y}));
}

BOOST_AUTO_TEST_CASE(remove_) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto inv = DiscreteDomain::top();
  BOOST_CHECK(inv.is_top());
  inv.remove(x);
  BOOST_CHECK(inv.is_top());

  inv = DiscreteDomain{x, y};
  inv.remove(x);
  BOOST_CHECK((inv == DiscreteDomain{y}));
  inv.remove(y);
  BOOST_CHECK((inv == DiscreteDomain::bottom()));
}
