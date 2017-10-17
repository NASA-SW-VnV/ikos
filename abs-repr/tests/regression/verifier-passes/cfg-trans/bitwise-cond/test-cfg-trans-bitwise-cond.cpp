/*******************************************************************************
 *
 * ARBOS pass for regression test cfg-trans-llvm-select
 *
 * Authors: Nija Shi
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2017 United States Government as represented by the
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

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

#include <arbos/common/common.hpp>
#include <arbos/semantics/ar.hpp>

using namespace arbos;

namespace {

class verifier_error : public error {
private:
  verifier_error();

public:
  verifier_error(std::string msg) : error(msg) {}
  virtual ~verifier_error() {}
}; // class verifer_error

class Verifier : public Visitor {
private:
  std::vector< index64_t > _visited;

private:
  bool isVisited(index64_t uid) {
    return find(_visited.begin(), _visited.end(), uid) != _visited.end();
  }

public:
  Verifier() {}
  virtual void nodeStart(AR_Function&);
  void print(std::ostream& out) { out << "Test passed!" << std::endl; }
};

class VisitorPass : public Pass {
public:
  VisitorPass()
      : Pass("unittest-cfg-trans-bitwise-cond",
             "Verifier pass for unittest-cfg-trans-bitwise-cond"){};

  virtual ~VisitorPass() {}

  virtual void execute(AR_Node_Ref< AR_Bundle > bundle) {
    std::cout << "This pass verifies regression test cfg-trans/bitwise-cond"
              << std::endl
              << std::endl;
    std::shared_ptr< Verifier > v(new Verifier());
    (*bundle).accept(v);
    v->print(std::cout);
  }
};

// Visitor method implementation

void Verifier::nodeStart(AR_Function& f) {
  std::string trans =
      "($trans \
     ($edge ($*_entry_split_1_icmp_false) ($*out_entry_merge_1_icmp)) \
     ($edge ($*_entry_split_1_icmp_true) ($*out_entry_merge_1_icmp)) \
     ($edge ($*_entry_split_icmp_false) ($*out_entry_merge_icmp)) \
     ($edge ($*_entry_split_icmp_true) ($*out_entry_merge_icmp)) \
     ($edge ($*in_bb3_to_bb4_phi) ($bb4)) \
     ($edge ($*in_bb_to_bb4_phi) ($bb4)) \
     ($edge ($*out_entry_merge_1_icmp) ($*out_entry_to_bb_and_true)) \
     ($edge ($*out_entry_merge_1_icmp) ($*out_entry_to_bb3_and_false)) \
     ($edge ($*out_entry_merge_icmp) ($*_entry_split_1_icmp_true)) \
     ($edge ($*out_entry_merge_icmp) ($*_entry_split_1_icmp_false)) \
     ($edge ($*out_entry_to_bb3_and_false) ($bb3)) \
     ($edge ($*out_entry_to_bb_and_true) ($bb)) \
     ($edge ($bb) ($*in_bb_to_bb4_phi)) \
     ($edge ($bb3) ($*in_bb3_to_bb4_phi)) \
     ($edge ($entry) ($*_entry_split_icmp_true)) \
     ($edge ($entry) ($*_entry_split_icmp_false)) \
   )";

  std::istringstream iss(trans);
  s_expression_istream seis(iss);
  s_expression_ref e;
  seis >> e;

  AR_Node_Ref< AR_Code > body = f.getFunctionBody();
  std::vector< AR_Node_Ref< AR_Basic_Block > > bblocks = (*body).getBlocks();
  assert(bblocks.size() == 14);

  assert((*(*body).getEntryBlock()).getNameId() == "entry");
  assert((*(*body).getExitBlock()).getNameId() == "bb4");
  std::pair< bblock_iterator, bblock_iterator > next_bblocks;

  if (e) {
    s_expression expr = *e;
    if (s_pattern("trans") ^ expr) {
      for (std::size_t i = 1; i <= expr.n_args(); i++) {
        s_expression edge = expr[i];
        s_expression_ref src, dest;
        if (s_pattern("edge", src, dest) ^ edge) {
          std::string src_bbname = (static_cast< string_atom& >(**src)).data();
          std::string dest_bbname =
              (static_cast< string_atom& >(**dest)).data();
          AR_Node_Ref< AR_Basic_Block > src_bb =
              (*body).getBasicBlockByNameId(src_bbname);
          next_bblocks = (*src_bb).getNextBlocks();
          bool found = false;
          for (bblock_iterator b = next_bblocks.first; b != next_bblocks.second;
               b++) {
            if ((**b).getNameId() == dest_bbname) {
              found = true;
              break;
            }
          }
          assert(found);
        }
      }
    }
  }

  // Verify assertion statements in br_and_op bblock
  AR_Node_Ref< AR_Basic_Block > assert_true =
      (*body).getBasicBlockByNameId("*out_entry_to_bb_and_true");
  std::vector< AR_Node_Ref< AR_Statement > > stmts =
      (*assert_true).getStatements();
  assert(stmts.size() == 1);
  AR_Node_Ref< AR_Integer_Comparison > icmp =
      node_cast< AR_Integer_Comparison >(stmts[0]);
  assert((*icmp).isContinueIfTrue());
  AR_Node_Ref< AR_Var_Operand > var =
      node_cast< AR_Var_Operand >((*icmp).getLeftOperand());
  AR_Node_Ref< AR_Internal_Variable > ivar = (*var).getInternalVariable();
  assert((*ivar).getName() == "tmp3");
  AR_Node_Ref< AR_Integer_Type > int_ty =
      node_cast< AR_Integer_Type >((*ivar).getType());
  assert(int_ty.getUID() > 0);
  assert((*int_ty).getRealSize() == 1);
  assert((*int_ty).getStoreSize() == 1);
  AR_Node_Ref< AR_Cst_Operand > cst_operand =
      node_cast< AR_Cst_Operand >((*icmp).getRightOperand());
  AR_Node_Ref< AR_Integer_Constant > int_cst =
      node_cast< AR_Integer_Constant >((*cst_operand).getConstant());
  assert((*int_cst).getValue() == 1);

  AR_Node_Ref< AR_Basic_Block > assert_false =
      (*body).getBasicBlockByNameId("*out_entry_to_bb3_and_false");
  stmts = (*assert_false).getStatements();
  assert(stmts.size() == 1);
  icmp = node_cast< AR_Integer_Comparison >(stmts[0]);
  assert(!(*icmp).isContinueIfTrue());
  var = node_cast< AR_Var_Operand >((*icmp).getLeftOperand());
  ivar = (*var).getInternalVariable();
  assert((*ivar).getName() == "tmp3");
  int_ty = node_cast< AR_Integer_Type >((*ivar).getType());
  assert(int_ty.getUID() > 0);
  assert((*int_ty).getRealSize() == 1);
  assert((*int_ty).getStoreSize() == 1);
  cst_operand = node_cast< AR_Cst_Operand >((*icmp).getRightOperand());
  int_cst = node_cast< AR_Integer_Constant >((*cst_operand).getConstant());
  assert((*int_cst).getValue() == 1);
}
}

extern "C" Pass* init() {
  VisitorPass* pass = new VisitorPass();
  return pass;
}
