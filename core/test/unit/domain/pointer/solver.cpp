/*******************************************************************************
 *
 * Tests for pointer::ConstraintSystem
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

#define BOOST_TEST_MODULE test_pointer_solver
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/domain/pointer/solver.hpp>
#include <ikos/core/example/variable_factory.hpp>

using ikos::core::Unsigned;
using Int = ikos::core::MachineInt;
using Interval = ikos::core::machine_int::Interval;
using Nullity = ikos::core::Nullity;
using Uninitialized = ikos::core::Uninitialized;
using VariableFactory = ikos::core::example::VariableFactory;
using Variable = VariableFactory::VariableRef;
using MemoryFactory = ikos::core::example::VariableFactory;
using MemLocation = MemoryFactory::VariableRef;
using VarOperand =
    ikos::core::pointer::VariableOperand< Variable, MemLocation >;
using AddrOperand =
    ikos::core::pointer::AddressOperand< Variable, MemLocation >;
using Assign = ikos::core::pointer::AssignConstraint< Variable, MemLocation >;
using Store = ikos::core::pointer::StoreConstraint< Variable, MemLocation >;
using Load = ikos::core::pointer::LoadConstraint< Variable, MemLocation >;
using PointerAbsValue = ikos::core::PointerAbsValue< MemLocation >;
using PointsToSet = ikos::core::PointsToSet< MemLocation >;
using ConstraintSystem =
    ikos::core::pointer::ConstraintSystem< Variable, MemLocation >;

BOOST_AUTO_TEST_CASE(test_1) {
  // Sample function:
  //
  // int **f(int **p, int *q) {
  //   int *tmp = q + 1;
  //   *p = tmp;
  //   return p;
  // }

  VariableFactory vfac;
  MemoryFactory memfac;

  Variable p(vfac.get("p"));
  Variable q(vfac.get("q"));
  Variable tmp(vfac.get("tmp"));

  MemLocation x(memfac.get("x"));
  MemLocation y(memfac.get("y"));

  ConstraintSystem s(64, Unsigned);

  // tmp = q + 1;
  s.add(Assign::create(tmp,
                       VarOperand::create(q, Interval(Int(4, 64, Unsigned)))));

  // *p = tmp
  s.add(Store::create(p,
                      VarOperand::create(tmp, Interval(Int(0, 64, Unsigned)))));

  // f(&x + [0, 4], &y + [8, 16])
  s.add(Assign::create(p,
                       AddrOperand::create(x,
                                           Interval(Int(0, 64, Unsigned),
                                                    Int(4, 64, Unsigned)))));
  s.add(Assign::create(q,
                       AddrOperand::create(y,
                                           Interval(Int(8, 64, Unsigned),
                                                    Int(16, 64, Unsigned)))));

  s.solve();

  BOOST_CHECK(
      s.get_pointer(p) ==
      PointerAbsValue(Uninitialized::top(),
                      Nullity::top(),
                      PointsToSet{x},
                      Interval(Int(0, 64, Unsigned), Int(4, 64, Unsigned))));
  BOOST_CHECK(
      s.get_pointer(q) ==
      PointerAbsValue(Uninitialized::top(),
                      Nullity::top(),
                      PointsToSet{y},
                      Interval(Int(8, 64, Unsigned), Int(16, 64, Unsigned))));
  BOOST_CHECK(
      s.get_pointer(tmp) ==
      PointerAbsValue(Uninitialized::top(),
                      Nullity::top(),
                      PointsToSet{y},
                      Interval(Int(12, 64, Unsigned), Int(20, 64, Unsigned))));
  BOOST_CHECK(
      s.get_memory(x) ==
      PointerAbsValue(Uninitialized::top(),
                      Nullity::top(),
                      PointsToSet{y},
                      Interval(Int(12, 64, Unsigned), Int(20, 64, Unsigned))));
  BOOST_CHECK(s.get_memory(y) == PointerAbsValue::bottom(64, Unsigned));
}

BOOST_AUTO_TEST_CASE(test_2) {
  // int x, y;
  // int **p, *q;
  // q = &x;
  // p = &q;
  // *p = &y;

  // translated by llvm:
  // i32* x = alloca i32
  // i32* y = alloca i32
  // i32*** p = alloca i32**
  // i32** q = alloca i32*
  // store x, q
  // store q, p
  // tmp = load p
  // store y, tmp

  VariableFactory vfac;
  MemoryFactory memfac;

  Variable x(vfac.get("x"));
  Variable y(vfac.get("y"));
  Variable p(vfac.get("p"));
  Variable q(vfac.get("q"));
  Variable tmp(vfac.get("tmp"));

  MemLocation mx(memfac.get("&x"));
  MemLocation my(memfac.get("&y"));
  MemLocation mp(memfac.get("&p"));
  MemLocation mq(memfac.get("&q"));

  ConstraintSystem s(64, Unsigned);
  Interval zero(Int(0, 64, Unsigned));

  // i32* x = alloca i32
  s.add(Assign::create(x, AddrOperand::create(mx, zero)));
  // i32* y = alloca i32
  s.add(Assign::create(y, AddrOperand::create(my, zero)));
  // i32*** p = alloca i32**
  s.add(Assign::create(p, AddrOperand::create(mp, zero)));
  // i32** q = alloca i32*
  s.add(Assign::create(q, AddrOperand::create(mq, zero)));
  // store x, q
  s.add(Store::create(q, VarOperand::create(x, zero)));
  // store q, p
  s.add(Store::create(p, VarOperand::create(q, zero)));
  // tmp = load p
  s.add(Load::create(tmp, VarOperand::create(p, zero)));
  // store y, tmp
  s.add(Store::create(tmp, VarOperand::create(y, zero)));

  s.solve();

  BOOST_CHECK(s.get_memory(mp) == PointerAbsValue(Uninitialized::top(),
                                                  Nullity::top(),
                                                  PointsToSet{mq},
                                                  zero));
  BOOST_CHECK(s.get_memory(mq) == PointerAbsValue(Uninitialized::top(),
                                                  Nullity::top(),
                                                  PointsToSet{mx, my},
                                                  zero));
}

BOOST_AUTO_TEST_CASE(test_3) {
  // Sample function:
  // global struct foo * x = null;

  //-- %main._0 = call noalias i8* @malloc(i64 8) nounwind, !dbg !18
  // p0 = malloc(8);
  //-- %main._1 = bitcast i8* %main._0 to %struct.foo*, !dbg !18
  // p1 = p0;
  //-- store %struct.foo* %main._1, %struct.foo** @x, align 8, !dbg !18
  // *x = p1
  //-- %main._4 = load %struct.foo** @x, align 8, !dbg !21
  // p4 = *x;
  //-- %main._5 = getelementptr inbounds %struct.foo* %main._4, i32 0, i32 0,
  //! dbg !21
  // p5 = p4 + 0;
  //-- store i32 5, i32* %main._5, align 4, !dbg !21
  /*C*/
  // *p5 = 5; // x->a = 5;
  //-- %main._6 = load %struct.foo** @x, align 8, !dbg !22
  // p6 = *x;
  //-- %main._7 = load %struct.foo** @x, align 8, !dbg !22
  // p7 = *x;
  //-- %main._8 = getelementptr inbounds %struct.foo* %main._7, i32 0, i32 0,
  //! dbg !22
  // p8 = p7 + 0;
  //-- %main._9 = load i32* %main._8, align 4, !dbg !22
  /*C*/
  // p9 = *p8;  // *(x).a
  //-- %main._10 = add nsw i32 %main._9, 7, !dbg !22
  /*C*/
  // p10 = p9 + 7;
  //-- %main._11 = getelementptr inbounds %struct.foo* %main._6, i32 0, i32 1,
  //! dbg !22
  // p11 = p6 + 4;
  //--  store i32 %main._10, i32* %main._11, align 4, !dbg !22
  /*C*/
  // *p11 = p10; // x->b = x->a + 7;
  //
  // At this point x->a = 5 and x->b = 12

  VariableFactory vfac;
  MemoryFactory memfac;

  Variable x(vfac.get("x"));
  Variable p0(vfac.get("p0"));
  Variable p1(vfac.get("p1"));
  Variable p4(vfac.get("p4"));
  Variable p5(vfac.get("p5"));
  Variable p6(vfac.get("p6"));
  Variable p7(vfac.get("p7"));
  Variable p8(vfac.get("p8"));
  Variable p11(vfac.get("p11"));

  MemLocation mx(memfac.get("&x"));
  MemLocation malloc(memfac.get("malloc"));

  ConstraintSystem s(64, Unsigned);
  Interval zero(Int(0, 64, Unsigned));

  // declaration of global variable x
  s.add(Assign::create(x, AddrOperand::create(mx, zero)));
  // p0 = malloc(8);
  s.add(Assign::create(p0, AddrOperand::create(malloc, zero)));
  // p1 = p0;
  s.add(Assign::create(p1, VarOperand::create(p0, zero)));
  // *x = p1;
  s.add(Store::create(x, VarOperand::create(p1, zero)));
  // p4 = *x;
  s.add(Load::create(p4, VarOperand::create(x, zero)));
  // p5 = p4;
  s.add(Assign::create(p5, VarOperand::create(p4, zero)));
  // *p5 = 5;
  // no pointer constraint
  // p6 = *x;
  s.add(Load::create(p6, VarOperand::create(x, zero)));
  // p7 = *x;
  s.add(Load::create(p7, VarOperand::create(x, zero)));
  // p8 = p7;
  s.add(Assign::create(p8, VarOperand::create(p7, zero)));
  // p9 = *p8;
  // no pointer constraint
  // p10 = p9 + 7;
  // no pointer constraint
  // p11  = p6 + 4;
  s.add(Assign::create(p11,
                       VarOperand::create(p6, Interval(Int(4, 64, Unsigned)))));
  // *p11 = p10;
  // no pointer constraint

  s.solve();

  BOOST_CHECK(s.get_pointer(p5) == PointerAbsValue(Uninitialized::top(),
                                                   Nullity::top(),
                                                   PointsToSet{malloc},
                                                   zero));
  BOOST_CHECK(s.get_pointer(p8) == PointerAbsValue(Uninitialized::top(),
                                                   Nullity::top(),
                                                   PointsToSet{malloc},
                                                   zero));
  BOOST_CHECK(s.get_pointer(p11) ==
              PointerAbsValue(Uninitialized::top(),
                              Nullity::top(),
                              PointsToSet{malloc},
                              Interval(Int(4, 64, Unsigned))));
}

BOOST_AUTO_TEST_CASE(test_4) {
  /*
    l = malloc (sizeof(int) * ncols);
    for (i=0;i<10;i++)
    {
      l[i] = malloc (sizeof(int) * nrows)
    }
   */

  VariableFactory vfac;
  MemoryFactory memfac;

  Variable l(vfac.get("l"));
  Variable tmp(vfac.get("tmp"));

  MemLocation ncols(memfac.get("ncols"));
  MemLocation nrows(memfac.get("nrows"));

  ConstraintSystem s(64, Unsigned);
  Interval zero(Int(0, 64, Unsigned));

  s.add(Assign::create(l, AddrOperand::create(ncols, zero)));
  s.add(Assign::create(tmp,
                       VarOperand::create(l,
                                          Interval(Int(0, 64, Unsigned),
                                                   Int(9, 64, Unsigned)))));
  s.add(Store::create(tmp, AddrOperand::create(nrows, zero)));

  s.solve();

  BOOST_CHECK(s.get_pointer(l) == PointerAbsValue(Uninitialized::top(),
                                                  Nullity::top(),
                                                  PointsToSet{ncols},
                                                  zero));
  BOOST_CHECK(
      s.get_pointer(tmp) ==
      PointerAbsValue(Uninitialized::top(),
                      Nullity::top(),
                      PointsToSet{ncols},
                      Interval(Int(0, 64, Unsigned), Int(9, 64, Unsigned))));
  BOOST_CHECK(s.get_memory(ncols) == PointerAbsValue(Uninitialized::top(),
                                                     Nullity::top(),
                                                     PointsToSet{nrows},
                                                     zero));
  BOOST_CHECK(s.get_memory(nrows) == PointerAbsValue::bottom(64, Unsigned));
}
