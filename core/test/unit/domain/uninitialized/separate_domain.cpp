/*******************************************************************************
 *
 * Tests for UninitializedDomain
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

#define BOOST_TEST_MODULE test_uninitialized_domain
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/domain/uninitialized/separate_domain.hpp>
#include <ikos/core/example/variable_factory.hpp>

using VariableFactory = ikos::core::example::VariableFactory;
using Variable = ikos::core::example::VariableFactory::VariableRef;
using Uninitialized = ikos::core::Uninitialized;
using UninitializedDomain =
    ikos::core::uninitialized::SeparateDomain< Variable >;

BOOST_AUTO_TEST_CASE(is_top_and_bottom) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  BOOST_CHECK(UninitializedDomain::top().is_top());
  BOOST_CHECK(!UninitializedDomain::top().is_bottom());

  BOOST_CHECK(!UninitializedDomain::bottom().is_top());
  BOOST_CHECK(UninitializedDomain::bottom().is_bottom());

  auto inv = UninitializedDomain::top();
  BOOST_CHECK(inv.is_top());
  BOOST_CHECK(!inv.is_bottom());

  inv.assign_initialized(x);
  BOOST_CHECK(!inv.is_top());
  BOOST_CHECK(!inv.is_bottom());

  inv.set(x, Uninitialized::bottom());
  BOOST_CHECK(!inv.is_top());
  BOOST_CHECK(inv.is_bottom());
}

BOOST_AUTO_TEST_CASE(set_to_top_and_bottom) {
  VariableFactory vfac;

  auto inv = UninitializedDomain::top();
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

  BOOST_CHECK(UninitializedDomain::bottom().leq(UninitializedDomain::top()));
  BOOST_CHECK(UninitializedDomain::bottom().leq(UninitializedDomain::bottom()));
  BOOST_CHECK(!UninitializedDomain::top().leq(UninitializedDomain::bottom()));
  BOOST_CHECK(UninitializedDomain::top().leq(UninitializedDomain::top()));

  auto inv1 = UninitializedDomain::top();
  inv1.set(x, Uninitialized::initialized());
  BOOST_CHECK(inv1.leq(UninitializedDomain::top()));
  BOOST_CHECK(!inv1.leq(UninitializedDomain::bottom()));

  auto inv2 = UninitializedDomain::top();
  inv2.set(x, Uninitialized::uninitialized());
  BOOST_CHECK(inv2.leq(UninitializedDomain::top()));
  BOOST_CHECK(!inv2.leq(UninitializedDomain::bottom()));
  BOOST_CHECK(!inv1.leq(inv2));
  BOOST_CHECK(!inv2.leq(inv1));

  auto inv3 = UninitializedDomain::top();
  inv3.set(x, Uninitialized::initialized());
  inv3.set(y, Uninitialized::uninitialized());
  BOOST_CHECK(inv3.leq(UninitializedDomain::top()));
  BOOST_CHECK(!inv3.leq(UninitializedDomain::bottom()));
  BOOST_CHECK(inv3.leq(inv1));
  BOOST_CHECK(!inv1.leq(inv3));
}

BOOST_AUTO_TEST_CASE(join) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  BOOST_CHECK((UninitializedDomain::bottom().join(UninitializedDomain::top()) ==
               UninitializedDomain::top()));
  BOOST_CHECK(
      (UninitializedDomain::bottom().join(UninitializedDomain::bottom()) ==
       UninitializedDomain::bottom()));
  BOOST_CHECK((UninitializedDomain::top().join(UninitializedDomain::top()) ==
               UninitializedDomain::top()));
  BOOST_CHECK((UninitializedDomain::top().join(UninitializedDomain::bottom()) ==
               UninitializedDomain::top()));

  auto inv1 = UninitializedDomain::top();
  inv1.set(x, Uninitialized::initialized());
  BOOST_CHECK(
      (inv1.join(UninitializedDomain::top()) == UninitializedDomain::top()));
  BOOST_CHECK((inv1.join(UninitializedDomain::bottom()) == inv1));
  BOOST_CHECK(
      (UninitializedDomain::top().join(inv1) == UninitializedDomain::top()));
  BOOST_CHECK((UninitializedDomain::bottom().join(inv1) == inv1));
  BOOST_CHECK((inv1.join(inv1) == inv1));

  auto inv2 = UninitializedDomain::top();
  inv2.set(x, Uninitialized::uninitialized());
  BOOST_CHECK((inv1.join(inv2) == UninitializedDomain::top()));
  BOOST_CHECK((inv2.join(inv1) == UninitializedDomain::top()));
}

BOOST_AUTO_TEST_CASE(meet) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));

  BOOST_CHECK((UninitializedDomain::bottom().meet(UninitializedDomain::top()) ==
               UninitializedDomain::bottom()));
  BOOST_CHECK(
      (UninitializedDomain::bottom().meet(UninitializedDomain::bottom()) ==
       UninitializedDomain::bottom()));
  BOOST_CHECK((UninitializedDomain::top().meet(UninitializedDomain::top()) ==
               UninitializedDomain::top()));
  BOOST_CHECK((UninitializedDomain::top().meet(UninitializedDomain::bottom()) ==
               UninitializedDomain::bottom()));

  auto inv1 = UninitializedDomain::top();
  inv1.set(x, Uninitialized::initialized());
  BOOST_CHECK((inv1.meet(UninitializedDomain::top()) == inv1));
  BOOST_CHECK((inv1.meet(UninitializedDomain::bottom()) ==
               UninitializedDomain::bottom()));
  BOOST_CHECK((UninitializedDomain::top().meet(inv1) == inv1));
  BOOST_CHECK((UninitializedDomain::bottom().meet(inv1) ==
               UninitializedDomain::bottom()));
  BOOST_CHECK((inv1.meet(inv1) == inv1));

  auto inv2 = UninitializedDomain::top();
  inv2.set(x, Uninitialized::uninitialized());
  BOOST_CHECK((inv1.meet(inv2) == UninitializedDomain::bottom()));
  BOOST_CHECK((inv2.meet(inv1) == UninitializedDomain::bottom()));
}
