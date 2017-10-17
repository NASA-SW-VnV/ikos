/*******************************************************************************
 *
 * ARBOS pass for regression test cfg-trans-llvm-phi-1
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
      : Pass("unittest-cfg-trans-llvm-phi-1",
             "Verifier pass for unittest-cfg-trans-llvm-phi-1"){};

  virtual ~VisitorPass() {}

  virtual void execute(AR_Node_Ref< AR_Bundle > bundle) {
    std::cout << "This pass verifies regression test parsing/call-args"
              << std::endl
              << std::endl;
    std::shared_ptr< Verifier > v(new Verifier());
    (*bundle).accept(v);
    v->print(std::cout);
  }
};

// Visitor method implementation

void Verifier::nodeStart(AR_Function& f) {
  AR_Node_Ref< AR_Code > body = f.getFunctionBody();
  std::vector< AR_Node_Ref< AR_Basic_Block > > bblocks = (*body).getBlocks();
  std::vector< AR_Node_Ref< AR_Basic_Block > >::iterator b = bblocks.begin();

  std::pair< bblock_iterator, bblock_iterator > next_bblocks;
  for (; b != bblocks.end(); b++) {
    if ((**b).getNameId() == "bb") {
      next_bblocks = (**b).getNextBlocks();
      assert(std::distance(next_bblocks.first, next_bblocks.second) == 1);
      assert((**next_bblocks.first).getNameId() == "*in_bb_to_bb2_phi");
    } else if ((**b).getNameId() == "*in_bb_to_bb2_phi") {
      next_bblocks = (**b).getNextBlocks();
      assert(std::distance(next_bblocks.first, next_bblocks.second) == 1);
      assert((**next_bblocks.first).getNameId() == "bb2");
    } else if ((**b).getNameId() == "*in_bb8_to_bb2_phi") {
      next_bblocks = (**b).getNextBlocks();
      assert(std::distance(next_bblocks.first, next_bblocks.second) == 1);
      assert((**next_bblocks.first).getNameId() == "bb2");
    } else if ((**b).getNameId() == "bb2") {
      next_bblocks = (**b).getNextBlocks();
      assert(std::distance(next_bblocks.first, next_bblocks.second) == 2);
      std::string bb0_name = (**next_bblocks.first).getNameId();
      next_bblocks.first++;
      std::string bb1_name = (**next_bblocks.first).getNameId();
      assert(bb0_name != bb1_name);
      if (bb0_name == "*out_bb2_to_bb10_icmp_false")
        assert(bb1_name == "*out_bb2_to_bb3_icmp_true");
      else if (bb0_name == "*out_bb2_to_bb3_icmp_true")
        assert(bb1_name == "*out_bb2_to_bb10_icmp_false");
      else
        assert(false);
    } else if ((**b).getNameId() == "*out_bb2_to_bb10_icmp_false") {
      next_bblocks = (**b).getNextBlocks();
      assert(std::distance(next_bblocks.first, next_bblocks.second) == 1);
      assert((**next_bblocks.first).getNameId() == "bb10");
    } else if ((**b).getNameId() == "*out_bb2_to_bb3_icmp_true") {
      next_bblocks = (**b).getNextBlocks();
      assert(std::distance(next_bblocks.first, next_bblocks.second) == 1);
      assert((**next_bblocks.first).getNameId() == "bb3");
    } else if ((**b).getNameId() == "bb8") {
      next_bblocks = (**b).getNextBlocks();
      assert(std::distance(next_bblocks.first, next_bblocks.second) == 1);
      assert((**next_bblocks.first).getNameId() == "*in_bb8_to_bb2_phi");
    } else if ((**b).getNameId() == "bb3") {
      next_bblocks = (**b).getNextBlocks();
      assert(std::distance(next_bblocks.first, next_bblocks.second) == 1);
      assert((**next_bblocks.first).getNameId() == "bb8");
    }

    assert((*(*body).getEntryBlock()).getNameId() == "bb");
    assert((*(*body).getExitBlock()).getNameId() == "bb10");
  }
}
}

extern "C" Pass* init() {
  VisitorPass* pass = new VisitorPass();
  return pass;
}
