/*******************************************************************************
 *
 * Tests for WPO
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2019 United States Government as represented by the
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

#define BOOST_TEST_MODULE test_wpo
#define BOOST_TEST_DYN_LINK
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <ikos/core/example/muzq.hpp>
#include <ikos/core/example/variable_factory.hpp>
#include <ikos/core/fixpoint/wpo.hpp>

using Variable = ikos::core::example::VariableFactory::VariableRef;
using BasicBlock = ikos::core::muzq::BasicBlock< Variable >;
using ControlFlowGraph = ikos::core::muzq::ControlFlowGraph< Variable >;
using Wpo = ikos::core::Wpo< ControlFlowGraph* >;

BOOST_AUTO_TEST_CASE(test1) {
  // Control flow graph from Figure 2 of "Deterministic Parallel Fixpoint
  // Computation", in POPL 2020.
  ControlFlowGraph cfg("bb1");

  BasicBlock* bb1 = cfg.get("bb1");
  BasicBlock* bb2 = cfg.get("bb2");
  BasicBlock* bb3 = cfg.get("bb3");
  BasicBlock* bb4 = cfg.get("bb4");
  BasicBlock* bb5 = cfg.get("bb5");
  BasicBlock* bb6 = cfg.get("bb6");
  BasicBlock* bb7 = cfg.get("bb7");
  BasicBlock* bb8 = cfg.get("bb8");
  BasicBlock* bb9 = cfg.get("bb9");
  BasicBlock* bb10 = cfg.get("bb10");

  bb1->add_successor(bb2);

  bb2->add_successor(bb3);
  bb2->add_successor(bb6);
  bb2->add_successor(bb10);

  bb3->add_successor(bb4);
  bb3->add_successor(bb5);

  bb4->add_successor(bb3);

  bb5->add_successor(bb2);

  bb6->add_successor(bb5);
  bb6->add_successor(bb7);
  bb6->add_successor(bb9);

  bb7->add_successor(bb8);

  bb8->add_successor(bb6);

  bb9->add_successor(bb8);

  Wpo wpo(&cfg);

  // wpo.dump(std::cout);

  BOOST_CHECK(wpo.size() == 13);
  BOOST_CHECK(wpo.entry() == 12);

  BOOST_CHECK(wpo.node(12) == bb1);
  BOOST_CHECK(wpo.is_plain(12));
  BOOST_CHECK(wpo.successors(12) == std::vector< std::size_t >({11}));
  BOOST_CHECK(wpo.predecessors(12) == std::vector< std::size_t >());

  BOOST_CHECK(wpo.node(11) == bb2);
  BOOST_CHECK(wpo.is_head(11));
  BOOST_CHECK(wpo.successors(11) == std::vector< std::size_t >({5, 9}));
  BOOST_CHECK(wpo.predecessors(11) == std::vector< std::size_t >({12}));

  BOOST_CHECK(wpo.node(10) == bb2);
  BOOST_CHECK(wpo.is_exit(10));
  BOOST_CHECK(wpo.successors(10) == std::vector< std::size_t >({0}));
  BOOST_CHECK(wpo.predecessors(10) == std::vector< std::size_t >({6}));

  BOOST_CHECK(wpo.node(9) == bb3);
  BOOST_CHECK(wpo.is_head(9));
  BOOST_CHECK(wpo.successors(9) == std::vector< std::size_t >({7}));
  BOOST_CHECK(wpo.predecessors(9) == std::vector< std::size_t >({11}));

  BOOST_CHECK(wpo.node(8) == bb3);
  BOOST_CHECK(wpo.is_exit(8));
  BOOST_CHECK(wpo.successors(8) == std::vector< std::size_t >({6}));
  BOOST_CHECK(wpo.predecessors(8) == std::vector< std::size_t >({7}));

  BOOST_CHECK(wpo.node(7) == bb4);
  BOOST_CHECK(wpo.is_plain(7));
  BOOST_CHECK(wpo.successors(7) == std::vector< std::size_t >({8}));
  BOOST_CHECK(wpo.predecessors(7) == std::vector< std::size_t >({9}));

  BOOST_CHECK(wpo.node(6) == bb5);
  BOOST_CHECK(wpo.is_plain(6));
  BOOST_CHECK(wpo.successors(6) == std::vector< std::size_t >({10}));
  BOOST_CHECK(wpo.predecessors(6) == std::vector< std::size_t >({8, 4}));

  BOOST_CHECK(wpo.node(5) == bb6);
  BOOST_CHECK(wpo.is_head(5));
  BOOST_CHECK(wpo.successors(5) == std::vector< std::size_t >({1, 3}));
  BOOST_CHECK(wpo.predecessors(5) == std::vector< std::size_t >({11}));

  BOOST_CHECK(wpo.node(4) == bb6);
  BOOST_CHECK(wpo.is_exit(4));
  BOOST_CHECK(wpo.successors(4) == std::vector< std::size_t >({6}));
  BOOST_CHECK(wpo.predecessors(4) == std::vector< std::size_t >({2}));

  BOOST_CHECK(wpo.node(3) == bb7);
  BOOST_CHECK(wpo.is_plain(3));
  BOOST_CHECK(wpo.successors(3) == std::vector< std::size_t >({2}));
  BOOST_CHECK(wpo.predecessors(3) == std::vector< std::size_t >({5}));

  BOOST_CHECK(wpo.node(2) == bb8);
  BOOST_CHECK(wpo.is_plain(2));
  BOOST_CHECK(wpo.successors(2) == std::vector< std::size_t >({4}));
  BOOST_CHECK(wpo.predecessors(2) == std::vector< std::size_t >({3, 1}));

  BOOST_CHECK(wpo.node(1) == bb9);
  BOOST_CHECK(wpo.is_plain(1));
  BOOST_CHECK(wpo.successors(1) == std::vector< std::size_t >({2}));
  BOOST_CHECK(wpo.predecessors(1) == std::vector< std::size_t >({5}));

  BOOST_CHECK(wpo.node(0) == bb10);
  BOOST_CHECK(wpo.is_plain(0));
  BOOST_CHECK(wpo.successors(0) == std::vector< std::size_t >());
  BOOST_CHECK(wpo.predecessors(0) == std::vector< std::size_t >({10}));
}
