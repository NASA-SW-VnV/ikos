/*******************************************************************************
 *
 * Tests for Octagon
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

#define BOOST_TEST_MODULE test_octagon_domain
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/domain/numeric/octagon.hpp>
#include <ikos/core/example/variable_factory.hpp>
#include <ikos/core/number/z_number.hpp>

using ZNumber = ikos::core::ZNumber;
using ZBound = ikos::core::Bound< ZNumber >;
using ZInterval = ikos::core::numeric::Interval< ZNumber >;
using VariableFactory = ikos::core::example::VariableFactory;
using Variable = ikos::core::example::VariableFactory::VariableRef;
using VariableExpr = ikos::core::VariableExpression< ZNumber, Variable >;
using BinaryOperator = ikos::core::numeric::BinaryOperator;
using Octagon = ikos::core::numeric::Octagon< ZNumber, Variable >;

BOOST_AUTO_TEST_CASE(test_1) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));
  Variable w(vfac.get("w"));

  auto inv = Octagon::top();
  BOOST_CHECK(inv.is_top());

  inv.assign(x, 0);
  inv.add(VariableExpr(w) == VariableExpr(x));
  inv.apply(BinaryOperator::Add, x, x, ZNumber(1));
  inv.add(VariableExpr(x) == VariableExpr(y));
  inv.add(VariableExpr(x) <= VariableExpr(z));
  inv.normalize();

  BOOST_CHECK(inv.to_interval(x) == ZInterval(1));
  BOOST_CHECK(inv.to_interval(y) == ZInterval(1));
  BOOST_CHECK(inv.to_interval(z) ==
              ZInterval(ZBound(1), ZBound::plus_infinity()));
  BOOST_CHECK(inv.to_interval(w) == ZInterval(0));
}

BOOST_AUTO_TEST_CASE(test_2) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));
  Variable w(vfac.get("w"));

  auto inv = Octagon::top();
  inv.add(VariableExpr(x) >= 0);
  inv.add(VariableExpr(y) >= VariableExpr(x));
  inv.add(VariableExpr(z) == 11);
  inv.add(VariableExpr(z) >= VariableExpr(x) + 1);
  inv.add(VariableExpr(y) <= VariableExpr(z) - 1);
  inv.add(VariableExpr(w) == VariableExpr(y) + 1);
  inv.normalize();

  BOOST_CHECK(inv.to_interval(x) == ZInterval(ZBound(0), ZBound(10)));
  BOOST_CHECK(inv.to_interval(y) == ZInterval(ZBound(0), ZBound(10)));
  BOOST_CHECK(inv.to_interval(z) == ZInterval(11));
  BOOST_CHECK(inv.to_interval(w) == ZInterval(ZBound(1), ZBound(11)));
}

BOOST_AUTO_TEST_CASE(test_3) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable z(vfac.get("z"));
  Variable w(vfac.get("w"));

  auto inv = Octagon::top();
  inv.add(VariableExpr(x) == 0);
  inv.add(VariableExpr(y) == 10);
  inv.add(VariableExpr(z) >= VariableExpr(x));
  inv.add(VariableExpr(z) <= VariableExpr(y) - 1);
  inv.assign(w, z);
  inv.normalize();

  Octagon tmp1 = inv;
  tmp1.add(VariableExpr(z) <= -1);
  BOOST_CHECK(tmp1.is_bottom());

  Octagon tmp2 = inv;
  tmp2.add(VariableExpr(w) <= -1);
  BOOST_CHECK(tmp2.is_bottom());
}

BOOST_AUTO_TEST_CASE(test_4) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto inv1 = Octagon::top();
  inv1.assign(x, 0);
  inv1.add(VariableExpr(x) <= VariableExpr(y) - 1);
  inv1.apply(BinaryOperator::Add, x, x, ZNumber(1));

  Octagon inv2(inv1);
  inv2.add(VariableExpr(x) <= VariableExpr(y) - 1);
  inv2.apply(BinaryOperator::Add, x, x, ZNumber(1));

  Octagon inv3 = inv1.join(inv2);
  BOOST_CHECK(inv3.to_interval(x) == ZInterval(ZBound(1), ZBound(2)));
  BOOST_CHECK(inv3.to_interval(y) ==
              ZInterval(ZBound(1), ZBound::plus_infinity()));
}

BOOST_AUTO_TEST_CASE(test_5) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto inv1 = Octagon::top();
  inv1.add(VariableExpr(y) >= 1);
  inv1.assign(x, 0);
  inv1.add(VariableExpr(x) <= VariableExpr(y) - 1);
  inv1.apply(BinaryOperator::Add, x, x, ZNumber(1));

  Octagon inv2 = inv1;
  inv2.add(VariableExpr(x) <= VariableExpr(y) - 1);
  inv2.apply(BinaryOperator::Add, x, x, ZNumber(1));

  Octagon inv3 = inv1.join(inv2);
  inv3.add(VariableExpr(x) <= VariableExpr(y) - 1);
  BOOST_CHECK(inv3.to_interval(x) == ZInterval(ZBound(1), ZBound(2)));
  BOOST_CHECK(inv3.to_interval(y) ==
              ZInterval(ZBound(1), ZBound::plus_infinity()));
}

BOOST_AUTO_TEST_CASE(test_6) {
  VariableFactory vfac;

  // downsizing
  Variable vi(vfac.get("i"));
  Variable vn(vfac.get("n"));
  Variable tmp1(vfac.get("tmp1"));
  Variable tmp2(vfac.get("tmp2"));
  Variable tmp3(vfac.get("tmp3"));

  auto inv1 = Octagon::top();
  inv1.add(VariableExpr(vn) >= 1);
  inv1.assign(vi, 0);
  inv1.add(VariableExpr(vi) <= VariableExpr(vn) - 1);
  inv1.apply(BinaryOperator::Add, vi, vi, ZNumber(1));
  inv1.apply(BinaryOperator::Add, tmp1, vi, ZNumber(2));
  inv1.apply(BinaryOperator::Add, tmp2, tmp1, ZNumber(2));
  inv1.apply(BinaryOperator::Add, tmp3, tmp2, ZNumber(4));

  inv1.forget(vi);
  BOOST_CHECK(inv1.to_interval(vi).is_top());
  inv1.forget(vn);
  BOOST_CHECK(inv1.to_interval(vn).is_top());
  inv1.forget(tmp3);
  BOOST_CHECK(inv1.to_interval(tmp3).is_top());
  inv1.forget(tmp1);
  BOOST_CHECK(inv1.to_interval(tmp1).is_top());
}

BOOST_AUTO_TEST_CASE(test_7) {
  VariableFactory vfac;
  Variable vj(vfac.get("j"));
  Variable vj1(vfac.get("j1"));

  auto inv = Octagon::top();
  inv.add(VariableExpr(vj) >= 0);
  inv.add(VariableExpr(vj) <= 10);
  inv.add(VariableExpr(vj1) >= 1);
  inv.add(VariableExpr(vj1) <= 11);
  inv.add(VariableExpr(vj1) == VariableExpr(vj) + 1);
  inv.add(VariableExpr(vj) <= 9);
  BOOST_CHECK(inv.to_interval(vj) == ZInterval(ZBound(0), ZBound(9)));
  BOOST_CHECK(inv.to_interval(vj1) == ZInterval(ZBound(1), ZBound(11)));
}

BOOST_AUTO_TEST_CASE(test_8) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));

  auto s1 = Octagon::top();

  s1.assign(x, 0);
  s1.assign(y, 0);

  // std::cout << "Loc 1: " << s1 << std::endl;
  Octagon s2 = s1;

  s2.apply(BinaryOperator::Add, x, x, ZNumber(1));
  // std::cout << "Loc 2a (after x++): " << s2 << std::endl;
  //
  s2.apply(BinaryOperator::Add, y, y, ZNumber(1));
  // std::cout << "Loc 2b (after y++): " << s2 << std::endl;

  Octagon s3 = s1.join(s2);
  // std::cout << "Loc 3, after join: " << s3 << std::endl;

  Octagon s4 = s3;
  s4.apply(BinaryOperator::Add, x, x, ZNumber(1));
  // std::cout << "Loc 4a (after x++): " << s4 << std::endl;

  s4.apply(BinaryOperator::Add, y, y, ZNumber(1));
  // std::cout << "Loc 4b (after y++): " << s4 << std::endl;

  Octagon s5 = s4.widening(s3);
  // std::cout << "Loc 5, after widening : " << s5 << std::endl;

  // TODO(marthaud): Add checks
}

BOOST_AUTO_TEST_CASE(test_9) {
  VariableFactory vfac;
  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable n(vfac.get("n"));
  Variable tmp_x(vfac.get("tmp_x"));
  Variable tmp_y(vfac.get("tmp_y"));

  auto s1 = Octagon::top();
  s1.assign(x, 0);
  s1.assign(y, 0);
  // std::cout << "Loc 1: " << s1 << std::endl;

  Octagon s2 = s1;
  s2.add(VariableExpr(x) <= VariableExpr(n));
  // std::cout << "Loc 2: " << s2 << std::endl;

  Octagon s3 = s2;
  s3.apply(BinaryOperator::Add, tmp_x, x, ZNumber(1));
  s3.apply(BinaryOperator::Add, tmp_y, y, ZNumber(1));
  s3.assign(x, tmp_x);
  s3.assign(y, tmp_y);
  // std::cout << "Loc 3 tmp_x:=x+1, tmp_y:=y+1, x:=tmp_x, y:=tmp_y= " << s3
  //          << std::endl;

  Octagon s4 = s1.join(s3);
  // std::cout << "Loc 4: Loc 1 U Loc 3=" << s4 << std::endl;

  Octagon s5 = s4;
  s5.add(VariableExpr(x) <= VariableExpr(n));
  // std::cout << "Loc 2': x<=n " << s5 << std::endl;

  Octagon s6 = s5;
  s6.apply(BinaryOperator::Add, tmp_x, x, ZNumber(1));
  s6.apply(BinaryOperator::Add, tmp_y, y, ZNumber(1));
  s6.assign(x, tmp_x);
  s6.assign(y, tmp_y);
  // std::cout << "Loc 3': tmp_x:=x+1, tmp_y:=y+1, x:=tmp_x, y:=tmp_y= " << s6
  //          << std::endl;

  Octagon s7 = s1.join(s6);
  // std::cout << "Loc 4': Loc 1 U Loc 3'=" << s7 << std::endl;

  Octagon s8 = s4.widening(s7);

  // std::cout << "Loc 4': Widening(Loc 4, Loc 4')=" << s8 << std::endl;

  Octagon s9 = s8;
  s9.add(VariableExpr(x) <= VariableExpr(n));
  // std::cout << "Loc 2'': x<=n " << s9 << std::endl;

  Octagon s10 = s9;
  s10.apply(BinaryOperator::Add, tmp_x, x, ZNumber(1));
  s10.apply(BinaryOperator::Add, tmp_y, y, ZNumber(1));
  s10.assign(x, tmp_x);
  s10.assign(y, tmp_y);
  // std::cout << "Loc 3'': tmp_x:=x+1, tmp_y:=y+1, x:=tmp_x, y:=tmp_y= " << s10
  //          << std::endl;

  Octagon s11 = s1.join(s10);
  // std::cout << "Loc 4'': Loc 1 U Loc 3''=" << s11 << std::endl;

  Octagon s12 = s8.widening(s11);

  // std::cout << "Loc 4'': Widening(Loc 4', Loc 4'')=" << s12 << std::endl;

  // TODO(marthaud): Add checks
}
