/*******************************************************************************
 *
 * Tests for muzq
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

#define BOOST_TEST_MODULE test_muzq
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/domain/numeric/dbm.hpp>
#include <ikos/core/domain/numeric/interval.hpp>
#include <ikos/core/example/muzq.hpp>
#include <ikos/core/example/variable_factory.hpp>

using namespace ikos::core;

using VariableFactory = example::VariableFactory;
using Variable = example::VariableFactory::VariableRef;
using ZVarExpr = VariableExpression< ZNumber, Variable >;
using QVarExpr = VariableExpression< QNumber, Variable >;

using Statement = muzq::Statement< Variable >;
using ZLinearExpression = LinearExpression< ZNumber, Variable >;
using ZLinearAssignment = muzq::ZLinearAssignment< Variable >;
using QLinearAssignment = muzq::QLinearAssignment< Variable >;
using ZBinaryOperation = muzq::ZBinaryOperation< Variable >;
using QBinaryOperation = muzq::QBinaryOperation< Variable >;
using ZLinearAssertion = muzq::ZLinearAssertion< Variable >;
using QLinearAssertion = muzq::QLinearAssertion< Variable >;
using BinaryOperator = numeric::BinaryOperator;
using CheckPoint = muzq::CheckPoint< Variable >;
using BasicBlock = muzq::BasicBlock< Variable >;
using ControlFlowGraph = muzq::ControlFlowGraph< Variable >;

using ZInterval = numeric::Interval< ZNumber >;
using ZIntervalDomain = numeric::IntervalDomain< ZNumber, Variable >;
using QIntervalDomain = numeric::IntervalDomain< QNumber, Variable >;
using ZDBM = numeric::DBM< ZNumber, Variable >;

BOOST_AUTO_TEST_CASE(test1) {
  ControlFlowGraph cfg("entry");

  BasicBlock* entry = cfg.get("entry");
  BasicBlock* bb1 = cfg.get("bb1");
  BasicBlock* bb1_t = cfg.get("bb1_t");
  BasicBlock* bb1_f = cfg.get("bb1_f");
  BasicBlock* bb2 = cfg.get("bb2");
  BasicBlock* ret = cfg.get("ret");

  VariableFactory vfac;
  Variable n1(vfac.get("n1"));
  Variable i(vfac.get("i"));

  entry->add_successor(bb1);
  bb1->add_successor(bb1_t);
  bb1->add_successor(bb1_f);
  bb1_t->add_successor(bb2);
  bb2->add_successor(bb1);
  bb1_f->add_successor(ret);

  entry->add(std::make_unique< ZLinearAssignment >(n1, ZLinearExpression(1)));
  entry->add(std::make_unique< ZLinearAssignment >(i, ZLinearExpression(0)));

  bb1_t->add(std::make_unique< ZLinearAssertion >(ZVarExpr(i) <= 9));

  bb1_f->add(std::make_unique< ZLinearAssertion >(ZVarExpr(i) >= 10));

  bb2->add(std::make_unique< CheckPoint >("loop.in"));
  bb2->add(
      std::make_unique< ZLinearAssignment >(i, ZVarExpr(i) + ZVarExpr(n1)));

  ret->add(std::make_unique< CheckPoint >("loop.end"));

  // cfg.dump(std::cout);

  muzq::FixpointIterator< Variable, ZIntervalDomain, QIntervalDomain > fixpoint(
      cfg);
  fixpoint.run({ZIntervalDomain::top(), QIntervalDomain::top()});

  // fixpoint.dump(std::cout);

  ZIntervalDomain loop_in = fixpoint.checkpoint("loop.in").first();
  BOOST_CHECK(loop_in.to_interval(i) == ZInterval(ZBound(0), ZBound(9)));
  BOOST_CHECK(loop_in.to_interval(n1) == ZInterval(1));

  ZIntervalDomain loop_end = fixpoint.checkpoint("loop.end").first();
  BOOST_CHECK(loop_end.to_interval(i) == ZInterval(10));
  BOOST_CHECK(loop_end.to_interval(n1) == ZInterval(1));
}

BOOST_AUTO_TEST_CASE(test2) {
  ControlFlowGraph cfg("entry");

  BasicBlock* entry = cfg.get("entry");
  BasicBlock* bb1 = cfg.get("bb1");
  BasicBlock* bb1_t = cfg.get("bb1_t");
  BasicBlock* bb1_f = cfg.get("bb1_f");
  BasicBlock* bb2 = cfg.get("bb2");
  BasicBlock* ret = cfg.get("ret");

  VariableFactory vfac;
  Variable n1(vfac.get("n1"));
  Variable i(vfac.get("i"));
  Variable tmp1(vfac.get("tmp1"));
  Variable tmp2(vfac.get("tmp2"));
  Variable tmp3(vfac.get("tmp3"));

  entry->add_successor(bb1);
  bb1->add_successor(bb1_t);
  bb1->add_successor(bb1_f);
  bb1_t->add_successor(bb2);
  bb2->add_successor(bb1);
  bb1_f->add_successor(ret);

  entry->add(std::make_unique< ZLinearAssignment >(n1, ZLinearExpression(1)));
  entry->add(std::make_unique< ZLinearAssignment >(i, ZLinearExpression(0)));

  bb1_t->add(std::make_unique< ZLinearAssertion >(ZVarExpr(i) <= 9));

  bb1_f->add(std::make_unique< ZLinearAssertion >(ZVarExpr(i) >= 10));

  bb2->add(std::make_unique< CheckPoint >("loop.in"));
  bb2->add(std::make_unique< ZLinearAssignment >(tmp1, ZLinearExpression(i)));
  bb2->add(std::make_unique< ZBinaryOperation >(tmp2,
                                                BinaryOperator::Add,
                                                tmp1,
                                                n1));
  bb2->add(std::make_unique< ZLinearAssignment >(i, ZLinearExpression(tmp2)));

  ret->add(
      std::make_unique< ZBinaryOperation >(tmp3, BinaryOperator::Sub, i, n1));
  ret->add(std::make_unique< CheckPoint >("loop.end"));

  // cfg.dump(std::cout);

  muzq::FixpointIterator< Variable, ZIntervalDomain, QIntervalDomain > fixpoint(
      cfg);
  fixpoint.run({ZIntervalDomain::top(), QIntervalDomain::top()});

  // fixpoint.dump(std::cout);

  ZIntervalDomain loop_in = fixpoint.checkpoint("loop.in").first();
  BOOST_CHECK(loop_in.to_interval(i) == ZInterval(ZBound(0), ZBound(9)));
  BOOST_CHECK(loop_in.to_interval(n1) == ZInterval(1));

  ZIntervalDomain loop_end = fixpoint.checkpoint("loop.end").first();
  BOOST_CHECK(loop_end.to_interval(i) == ZInterval(10));
  BOOST_CHECK(loop_end.to_interval(n1) == ZInterval(1));
  BOOST_CHECK(loop_end.to_interval(tmp3) == ZInterval(9));
}

BOOST_AUTO_TEST_CASE(test3) {
  ControlFlowGraph cfg("loop1_entry");

  BasicBlock* loop1_entry = cfg.get("loop1_entry");
  BasicBlock* loop1_bb1 = cfg.get("loop1_bb1");
  BasicBlock* loop1_bb1_t = cfg.get("loop1_bb1_t");
  BasicBlock* loop1_bb1_f = cfg.get("loop1_bb1_f");
  BasicBlock* loop1_bb2 = cfg.get("loop1_bb2");

  BasicBlock* loop2_entry = cfg.get("loop2_entry");
  BasicBlock* loop2_bb1 = cfg.get("loop2_bb1");
  BasicBlock* loop2_bb1_t = cfg.get("loop2_bb1_t");
  BasicBlock* loop2_bb1_f = cfg.get("loop2_bb1_f");
  BasicBlock* loop2_bb2 = cfg.get("loop2_bb2");

  BasicBlock* ret = cfg.get("ret");

  loop1_entry->add_successor(loop1_bb1);
  loop1_bb1->add_successor(loop1_bb1_t);
  loop1_bb1->add_successor(loop1_bb1_f);
  loop1_bb1_t->add_successor(loop1_bb2);
  loop1_bb2->add_successor(loop1_bb1);
  loop1_bb1_f->add_successor(loop2_entry);

  loop2_entry->add_successor(loop2_bb1);
  loop2_bb1->add_successor(loop2_bb1_t);
  loop2_bb1->add_successor(loop2_bb1_f);
  loop2_bb1_t->add_successor(loop2_bb2);
  loop2_bb2->add_successor(loop2_bb1);
  loop2_bb1_f->add_successor(ret);

  VariableFactory vfac;
  Variable n1(vfac.get("n1"));
  Variable i(vfac.get("i"));
  Variable j(vfac.get("j"));
  Variable s(vfac.get("s"));

  loop1_entry->add(
      std::make_unique< ZLinearAssignment >(s, ZLinearExpression(0)));
  loop1_entry->add(
      std::make_unique< ZLinearAssignment >(n1, ZLinearExpression(1)));
  loop1_entry->add(
      std::make_unique< ZLinearAssignment >(i, ZLinearExpression(0)));

  loop1_bb1_t->add(std::make_unique< ZLinearAssertion >(ZVarExpr(i) <= 9));

  loop1_bb1_f->add(std::make_unique< ZLinearAssertion >(ZVarExpr(i) >= 10));

  loop1_bb2->add(std::make_unique< CheckPoint >("loop1.in"));
  loop1_bb2->add(
      std::make_unique< ZBinaryOperation >(i, BinaryOperator::Add, i, n1));
  loop1_bb2->add(
      std::make_unique< ZBinaryOperation >(s, BinaryOperator::Add, s, n1));

  loop2_entry->add(
      std::make_unique< ZLinearAssignment >(j, ZLinearExpression(0)));

  loop2_bb1_t->add(std::make_unique< ZLinearAssertion >(ZVarExpr(j) <= 9));
  loop2_bb1_f->add(std::make_unique< ZLinearAssertion >(ZVarExpr(j) >= 10));

  loop2_bb2->add(std::make_unique< CheckPoint >("loop2.in"));
  loop2_bb2->add(
      std::make_unique< ZBinaryOperation >(j, BinaryOperator::Add, j, n1));
  loop2_bb2->add(
      std::make_unique< ZBinaryOperation >(s, BinaryOperator::Add, s, n1));

  ret->add(std::make_unique< CheckPoint >("end"));

  // cfg.dump(std::cout);

  muzq::FixpointIterator< Variable, ZIntervalDomain, QIntervalDomain > fixpoint(
      cfg);
  fixpoint.run({ZIntervalDomain::top(), QIntervalDomain::top()});

  // fixpoint.dump(std::cout);

  ZIntervalDomain loop1_in = fixpoint.checkpoint("loop1.in").first();
  BOOST_CHECK(loop1_in.to_interval(i) == ZInterval(ZBound(0), ZBound(9)));
  BOOST_CHECK(loop1_in.to_interval(j) == ZInterval::top());
  BOOST_CHECK(loop1_in.to_interval(s) ==
              ZInterval(ZBound(0), ZBound::plus_infinity()));

  ZIntervalDomain loop2_in = fixpoint.checkpoint("loop2.in").first();
  BOOST_CHECK(loop2_in.to_interval(i) == ZInterval(10));
  BOOST_CHECK(loop2_in.to_interval(j) == ZInterval(ZBound(0), ZBound(9)));
  BOOST_CHECK(loop2_in.to_interval(s) ==
              ZInterval(ZBound(0), ZBound::plus_infinity()));

  ZIntervalDomain end = fixpoint.checkpoint("end").first();
  BOOST_CHECK(end.to_interval(i) == ZInterval(10));
  BOOST_CHECK(end.to_interval(j) == ZInterval(10));
  BOOST_CHECK(end.to_interval(s) ==
              ZInterval(ZBound(0), ZBound::plus_infinity()));
}

BOOST_AUTO_TEST_CASE(test4) {
  ControlFlowGraph cfg("entry");

  BasicBlock* entry = cfg.get("entry");
  BasicBlock* bb1 = cfg.get("bb1");
  BasicBlock* bb1_t = cfg.get("bb1_t");
  BasicBlock* bb1_f = cfg.get("bb1_f");
  BasicBlock* bb2 = cfg.get("bb2");
  BasicBlock* ret = cfg.get("ret");

  VariableFactory vfac;
  Variable n1(vfac.get("n1"));
  Variable i(vfac.get("i"));
  Variable n(vfac.get("n"));

  entry->add_successor(bb1);
  bb1->add_successor(bb1_t);
  bb1->add_successor(bb1_f);
  bb1_t->add_successor(bb2);
  bb2->add_successor(bb1);
  bb1_f->add_successor(ret);

  entry->add(std::make_unique< ZLinearAssertion >(ZVarExpr(n) >= 1));
  entry->add(std::make_unique< ZLinearAssignment >(n1, ZLinearExpression(1)));
  entry->add(std::make_unique< ZLinearAssignment >(i, ZLinearExpression(0)));

  bb1_t->add(
      std::make_unique< ZLinearAssertion >(ZVarExpr(i) <= ZVarExpr(n) - 1));

  bb1_f->add(std::make_unique< ZLinearAssertion >(ZVarExpr(i) >= ZVarExpr(n)));

  bb2->add(std::make_unique< CheckPoint >("loop.in"));
  bb2->add(std::make_unique< ZBinaryOperation >(i, BinaryOperator::Add, i, n1));

  ret->add(std::make_unique< CheckPoint >("loop.end"));

  // cfg.dump(std::cout);

  muzq::FixpointIterator< Variable, ZDBM, QIntervalDomain > fixpoint(cfg);
  fixpoint.run({ZDBM::top(), QIntervalDomain::top()});

  // fixpoint.dump(std::cout);

  ZDBM loop_in = fixpoint.checkpoint("loop.in").first();
  BOOST_CHECK(loop_in.to_interval(i) ==
              ZInterval(ZBound(0), ZBound::plus_infinity()));
  BOOST_CHECK(loop_in.to_interval(n) ==
              ZInterval(ZBound(1), ZBound::plus_infinity()));
  loop_in.add(ZVarExpr(i) >= ZVarExpr(n));
  BOOST_CHECK(loop_in.is_bottom());

  ZDBM loop_end = fixpoint.checkpoint("loop.end").first();
  BOOST_CHECK(loop_end.to_interval(i) ==
              ZInterval(ZBound(1), ZBound::plus_infinity()));
  BOOST_CHECK(loop_end.to_interval(n) ==
              ZInterval(ZBound(1), ZBound::plus_infinity()));
  loop_end.add(ZVarExpr(i) <= ZVarExpr(n) - 1);
  BOOST_CHECK(loop_end.is_bottom());
}

BOOST_AUTO_TEST_CASE(test5) {
  // This is the program init_rand from Gange et.al paper.
  ControlFlowGraph cfg("entry");

  BasicBlock* entry = cfg.get("entry");
  BasicBlock* bb1 = cfg.get("bb1");
  BasicBlock* bb1_t = cfg.get("bb1_t");
  BasicBlock* bb1_f1 = cfg.get("bb1_f1");
  BasicBlock* bb1_f2 = cfg.get("bb1_f2");
  BasicBlock* bb1_f = cfg.get("bb1_f");
  BasicBlock* bb2_a = cfg.get("bb2_a");
  BasicBlock* bb2_b = cfg.get("bb2_b");
  BasicBlock* ret = cfg.get("ret");

  VariableFactory vfac;
  Variable n1(vfac.get("n1"));
  Variable i1(vfac.get("i1"));
  Variable i2(vfac.get("i2"));
  Variable n(vfac.get("n"));

  entry->add_successor(bb1);
  bb1->add_successor(bb1_t);
  bb1->add_successor(bb1_f1);
  bb1->add_successor(bb1_f2);
  bb1_f1->add_successor(bb1_f);
  bb1_f2->add_successor(bb1_f);
  bb1_t->add_successor(bb2_a);
  bb1_t->add_successor(bb2_b);
  bb2_a->add_successor(bb1);
  bb2_b->add_successor(bb1);
  bb1_f->add_successor(ret);

  entry->add(std::make_unique< ZLinearAssertion >(ZVarExpr(n) >= 1));
  entry->add(std::make_unique< ZLinearAssignment >(n1, ZLinearExpression(1)));
  entry->add(std::make_unique< ZLinearAssignment >(i1, ZLinearExpression(0)));
  entry->add(std::make_unique< ZLinearAssignment >(i2, ZLinearExpression(0)));

  // while (i1 < n && i2 < n){
  bb1_t->add(
      std::make_unique< ZLinearAssertion >(ZVarExpr(i1) <= ZVarExpr(n) - 1));
  bb1_t->add(
      std::make_unique< ZLinearAssertion >(ZVarExpr(i2) <= ZVarExpr(n) - 1));
  bb1_t->add(std::make_unique< CheckPoint >("loop.in"));

  // if (*)
  bb2_a->add(
      std::make_unique< ZBinaryOperation >(i1, BinaryOperator::Add, i1, n1));

  // else
  bb2_b->add(
      std::make_unique< ZBinaryOperation >(i2, BinaryOperator::Add, i2, n1));

  // } end while
  bb1_f1->add(
      std::make_unique< ZLinearAssertion >(ZVarExpr(i1) >= ZVarExpr(n)));
  bb1_f2->add(
      std::make_unique< ZLinearAssertion >(ZVarExpr(i2) >= ZVarExpr(n)));

  ret->add(std::make_unique< CheckPoint >("loop.end"));

  // cfg.dump(std::cout);

  muzq::FixpointIterator< Variable, ZDBM, QIntervalDomain > fixpoint(cfg);
  fixpoint.run({ZDBM::top(), QIntervalDomain::top()});

  // fixpoint.dump(std::cout);

  ZDBM loop_in_i1 = fixpoint.checkpoint("loop.in").first();
  loop_in_i1.add(ZVarExpr(i1) >= ZVarExpr(n));
  BOOST_CHECK(loop_in_i1.is_bottom());

  ZDBM loop_in_i2 = fixpoint.checkpoint("loop.in").first();
  loop_in_i2.add(ZVarExpr(i2) >= ZVarExpr(n));
  BOOST_CHECK(loop_in_i2.is_bottom());
}

BOOST_AUTO_TEST_CASE(test6) {
  ControlFlowGraph cfg("entry");

  BasicBlock* entry = cfg.get("entry");
  BasicBlock* bb7 = cfg.get("bb7");
  BasicBlock* pre_bb7_bb = cfg.get("pre_bb7_bb");
  BasicBlock* pre_bb7_bb8 = cfg.get("pre_bb7_bb8");
  BasicBlock* bb = cfg.get("bb");
  BasicBlock* bb8 = cfg.get("bb8");
  BasicBlock* bb5 = cfg.get("bb5");
  BasicBlock* pre_bb5_bb6 = cfg.get("pre_bb5_bb6");
  BasicBlock* pre_bb5_bb1 = cfg.get("pre_bb5_bb1");
  BasicBlock* bb6 = cfg.get("bb6");
  BasicBlock* bb1 = cfg.get("bb1");
  BasicBlock* bb3 = cfg.get("bb3");
  BasicBlock* pre_bb3_bb4 = cfg.get("pre_bb3_bb4");
  BasicBlock* pre_bb3_bb2 = cfg.get("pre_bb3_bb2");
  BasicBlock* bb2 = cfg.get("bb2");
  BasicBlock* bb4 = cfg.get("bb4");
  BasicBlock* ret = cfg.get("ret");

  entry->add_successor(bb7);
  bb7->add_successor(pre_bb7_bb);
  bb7->add_successor(pre_bb7_bb8);
  pre_bb7_bb->add_successor(bb);
  bb->add_successor(bb5);
  pre_bb7_bb8->add_successor(bb8);
  bb8->add_successor(ret);
  bb5->add_successor(pre_bb5_bb6);
  bb5->add_successor(pre_bb5_bb1);
  pre_bb5_bb6->add_successor(bb6);
  bb6->add_successor(bb7);
  pre_bb5_bb1->add_successor(bb1);
  bb1->add_successor(bb3);
  bb3->add_successor(pre_bb3_bb4);
  bb3->add_successor(pre_bb3_bb2);
  pre_bb3_bb4->add_successor(bb4);
  pre_bb3_bb2->add_successor(bb2);
  bb4->add_successor(bb5);
  bb2->add_successor(bb3);

  VariableFactory vfac;
  Variable cst_1(vfac.get("#1"));
  Variable cst_40000(vfac.get("#40000"));
  Variable cst_4(vfac.get("#4"));
  Variable cst_400(vfac.get("#400"));

  Variable i(vfac.get("i"));
  Variable j(vfac.get("j"));
  Variable k(vfac.get("k"));
  Variable x(vfac.get("x"));
  Variable tmp0(vfac.get("tmp0"));
  Variable tmp1(vfac.get("tmp1"));
  Variable tmp2(vfac.get("tmp2"));
  Variable tmp3(vfac.get("tmp3"));
  Variable temp1(vfac.get("temp1"));
  Variable temp2(vfac.get("temp2"));
  Variable temp3(vfac.get("temp3"));
  Variable temp4(vfac.get("temp4"));
  Variable temp5(vfac.get("temp5"));
  Variable t0(vfac.get("T0"));
  Variable t1(vfac.get("T1"));

  entry->add(std::make_unique< ZLinearAssignment >(cst_40000,
                                                   ZLinearExpression(40000)));
  entry->add(
      std::make_unique< ZLinearAssignment >(cst_400, ZLinearExpression(400)));
  entry->add(
      std::make_unique< ZLinearAssignment >(cst_1, ZLinearExpression(1)));
  entry->add(
      std::make_unique< ZLinearAssignment >(cst_4, ZLinearExpression(4)));
  entry->add(std::make_unique< ZLinearAssignment >(tmp0, ZLinearExpression(5)));
  entry->add(
      std::make_unique< ZLinearAssignment >(temp1, ZLinearExpression(tmp0)));
  entry->add(std::make_unique< ZLinearAssignment >(i, ZLinearExpression(0)));

  pre_bb7_bb->add(std::make_unique< ZLinearAssertion >(ZVarExpr(i) <= 99));
  bb->add(
      std::make_unique< ZLinearAssignment >(tmp1, ZLinearExpression(temp1)));
  bb->add(
      std::make_unique< ZLinearAssignment >(temp2, ZLinearExpression(tmp1)));
  bb->add(std::make_unique< ZLinearAssignment >(temp3, ZLinearExpression(0)));
  bb->add(std::make_unique< ZLinearAssignment >(j, ZLinearExpression(0)));

  pre_bb5_bb6->add(std::make_unique< ZLinearAssertion >(ZVarExpr(j) >= 100));
  bb6->add(std::make_unique< ZBinaryOperation >(temp1,
                                                BinaryOperator::Add,
                                                temp1,
                                                cst_40000));
  bb6->add(
      std::make_unique< ZBinaryOperation >(i, BinaryOperator::Add, i, cst_1));

  pre_bb5_bb1->add(std::make_unique< ZLinearAssertion >(ZVarExpr(j) <= 99));
  bb1->add(
      std::make_unique< ZLinearAssignment >(tmp2, ZLinearExpression(temp2)));
  bb1->add(
      std::make_unique< ZLinearAssignment >(tmp3, ZLinearExpression(temp3)));
  bb1->add(std::make_unique< ZLinearAssignment >(temp4, ZLinearExpression(0)));
  bb1->add(
      std::make_unique< ZLinearAssignment >(temp5, ZLinearExpression(tmp2)));
  bb1->add(std::make_unique< ZLinearAssignment >(k, ZLinearExpression(0)));

  pre_bb3_bb2->add(std::make_unique< ZLinearAssertion >(ZVarExpr(k) <= 9999));
  bb2->add(std::make_unique< ZLinearAssignment >(t0, ZLinearExpression(temp4)));
  bb2->add(std::make_unique< ZLinearAssignment >(t1, ZLinearExpression(temp5)));
  bb2->add(
      std::make_unique< ZBinaryOperation >(x, BinaryOperator::Add, t0, t1));
  bb2->add(std::make_unique< ZBinaryOperation >(temp4,
                                                BinaryOperator::Add,
                                                temp4,
                                                tmp3));
  bb2->add(std::make_unique< ZBinaryOperation >(temp5,
                                                BinaryOperator::Add,
                                                temp5,
                                                cst_4));
  bb2->add(
      std::make_unique< ZBinaryOperation >(k, BinaryOperator::Add, k, cst_1));

  pre_bb3_bb4->add(std::make_unique< ZLinearAssertion >(ZVarExpr(k) >= 1000));
  bb4->add(std::make_unique< ZBinaryOperation >(temp2,
                                                BinaryOperator::Add,
                                                temp2,
                                                cst_400));
  bb4->add(std::make_unique< ZBinaryOperation >(temp3,
                                                BinaryOperator::Add,
                                                temp3,
                                                i));
  bb4->add(
      std::make_unique< ZBinaryOperation >(j, BinaryOperator::Add, j, cst_1));

  pre_bb7_bb8->add(std::make_unique< ZLinearAssertion >(ZVarExpr(i) >= 100));

  ret->add(std::make_unique< CheckPoint >("end"));

  // cfg.dump(std::cout);

  muzq::FixpointIterator< Variable, ZIntervalDomain, QIntervalDomain > fixpoint(
      cfg);
  fixpoint.run({ZIntervalDomain::top(), QIntervalDomain::top()});

  // fixpoint.dump(std::cout);

  ZIntervalDomain end = fixpoint.checkpoint("end").first();
  BOOST_CHECK(end.to_interval(tmp0) == ZInterval(5));
  BOOST_CHECK(end.to_interval(i) == ZInterval(100));
  BOOST_CHECK(end.to_interval(temp1) ==
              ZInterval(ZBound(5), ZBound::plus_infinity()));
}
