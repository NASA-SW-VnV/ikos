/*******************************************************************************
 *
 * ARBOS pass for regression test cfg-trans-llvm-phi-4
 *
 * Specifically testing a merger bblock connecting to a phi block.
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
public:
  Verifier() {}
  virtual void nodeStart(AR_Function&);
  void print(std::ostream& out) { out << "Test passed!" << std::endl; }
};

class VisitorPass : public Pass {
public:
  VisitorPass()
      : Pass("unittest-cfg-trans-llvm-phi-4",
             "Verifier pass for unittest-cfg-trans-llvm-phi-4"){};

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
  if (f.getFunctionId() != "main")
    return;

  std::string trans =
      "($trans\
     ($edge ($bb_6) ($bb_7))\
     ($edge ($bb_5) ($bb_6))\
     ($edge ($*out_bb_3_to_bb_5_trunc_false) ($bb_5))\
     ($edge ($bb_4) ($bb_6))\
     ($edge ($*out_bb_3_to_bb_4_trunc_true) ($bb_4))\
     ($edge ($*out_bb_2_to_bb_3_icmp_true) ($bb_3))\
     ($edge ($*in_bb_1_to_bb_2_phi) ($bb_2))\
     ($edge ($bb_3) ($*out_bb_3_to_bb_4_trunc_true))\
     ($edge ($bb_3) ($*out_bb_3_to_bb_5_trunc_false))\
     ($edge ($bb_2) ($*out_bb_2_to_bb_3_icmp_true))\
     ($edge ($bb_2) ($*out_bb_2_to_bb_8_icmp_false))\
     ($edge ($bb_7) ($*in_bb_7_to_bb_2_phi))\
     ($edge ($*out_bb_1_merge_icmp) ($*in_bb_1_to_bb_2_phi))\
     ($edge ($*in_bb_7_to_bb_2_phi) ($bb_2))\
     ($edge ($*out_bb_2_to_bb_8_icmp_false) ($bb_8))\
     ($edge ($*_bb_1_split_icmp_false) ($*out_bb_1_merge_icmp))\
     ($edge ($*_bb_1_split_icmp_true) ($*out_bb_1_merge_icmp))\
     ($edge ($bb_1) ($*_bb_1_split_icmp_true))\
     ($edge ($bb_1) ($*_bb_1_split_icmp_false))\
  )";

  std::istringstream iss(trans);
  s_expression_istream seis(iss);
  s_expression_ref e;
  seis >> e;

  AR_Node_Ref< AR_Code > body = f.getFunctionBody();
  std::vector< AR_Node_Ref< AR_Basic_Block > > bblocks = (*body).getBlocks();
  assert(bblocks.size() == 17);

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
}
}

extern "C" Pass* init() {
  VisitorPass* pass = new VisitorPass();
  return pass;
}
