/*******************************************************************************
 *
 * ARBOS pass for regression test cfg-trans-llvm-phi-2
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
      : Pass("unittest-cfg-trans-llvm-phi-2",
             "Verifier pass for unittest-cfg-trans-llvm-phi-2"){};

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
  std::string trans =
      "($trans\
     ($edge ($*out_bb39_to_bb41_icmp_true) ($bb41))\
     ($edge ($bb39) ($*out_bb39_to_bb41_icmp_true))\
     ($edge ($bb39) ($*out_bb39_to_bb54_icmp_false))\
     ($edge ($*in_bb52_to_bb39_phi) ($bb39))\
     ($edge ($*in_bb38_to_bb39_phi) ($bb39))\
     ($edge ($bb38) ($*in_bb38_to_bb39_phi))\
     ($edge ($*out_bb10_to_bb12_icmp_true) ($bb12))\
     ($edge ($bb41) ($bb52))\
     ($edge ($bb35) ($bb36))\
     ($edge ($*in_bb30_to_bb10_phi) ($bb10))\
     ($edge ($*in_bb9_to_bb10_phi) ($bb10))\
     ($edge ($bb9) ($*in_bb9_to_bb10_phi))\
     ($edge ($*out_bb7_to_bb35_icmp_false) ($bb35))\
     ($edge ($bb30) ($*in_bb30_to_bb10_phi))\
     ($edge ($*out_bb7_to_bb9_icmp_true) ($bb9))\
     ($edge ($bb7) ($*out_bb7_to_bb9_icmp_true))\
     ($edge ($bb7) ($*out_bb7_to_bb35_icmp_false))\
     ($edge ($*in_bb33_to_bb7_phi) ($bb7))\
     ($edge ($bb6) ($*in_bb6_to_bb7_phi))\
     ($edge ($bb33) ($*in_bb33_to_bb7_phi))\
     ($edge ($bb32) ($bb33))\
     ($edge ($*out_bb39_to_bb54_icmp_false) ($bb54))\
     ($edge ($bb52) ($*in_bb52_to_bb39_phi))\
     ($edge ($bb36) ($*in_bb36_to_bb4_phi))\
     ($edge ($*out_bb10_to_bb32_icmp_false) ($bb32))\
     ($edge ($*in_bb_to_bb4_phi) ($bb4))\
     ($edge ($bb10) ($*out_bb10_to_bb12_icmp_true))\
     ($edge ($bb10) ($*out_bb10_to_bb32_icmp_false))\
     ($edge ($bb) ($*in_bb_to_bb4_phi))\
     ($edge ($*out_bb4_to_bb38_icmp_false) ($bb38))\
     ($edge ($*in_bb36_to_bb4_phi) ($bb4))\
     ($edge ($bb4) ($*out_bb4_to_bb6_icmp_true))\
     ($edge ($bb4) ($*out_bb4_to_bb38_icmp_false))\
     ($edge ($*in_bb6_to_bb7_phi) ($bb7))\
     ($edge ($bb12) ($bb30))\
     ($edge ($*out_bb4_to_bb6_icmp_true) ($bb6))\
  )";

  std::istringstream iss(trans);
  s_expression_istream seis(iss);
  s_expression_ref e;
  seis >> e;

  AR_Node_Ref< AR_Code > body = f.getFunctionBody();
  std::vector< AR_Node_Ref< AR_Basic_Block > > bblocks = (*body).getBlocks();
  assert(bblocks.size() == 33);

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
