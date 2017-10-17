/*******************************************************************************
 *
 * ARBOS pass for regression test parsing/src-lines-1
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
  virtual void nodeStart(AR_Assignment&);

  void print(std::ostream& out) { out << "Test passed!" << std::endl; }
};

class VisitorPass : public Pass {
public:
  VisitorPass()
      : Pass("unittest-parsing-src-lines-1",
             "Verifier pass for unittest-parsing-src-lines-1"){};

  virtual ~VisitorPass() {}

  virtual void execute(AR_Node_Ref< AR_Bundle > bundle) {
    std::cout << "This pass verifies regression test parsing/gv-int"
              << std::endl
              << std::endl;
    std::shared_ptr< Verifier > v(new Verifier());
    (*bundle).accept(v);
    v->print(std::cout);
  }
};

// Visitor method implementation

void Verifier::nodeStart(AR_Assignment& assign) {
  AR_Node_Ref< AR_Internal_Variable > var = assign.getLeftOperand();
  AR_Node_Ref< AR_Source_Location > srcloc = assign.getSourceLocation();
  AR_Node_Ref< AR_Basic_Block > bb = assign.getContainingBasicBlock();
  if ((*bb).getNameId() == "*in_for.inc_to_for.cond_phi") {
    assert((*srcloc).getLineNumber() == 5);
    assert((*srcloc).getColumnNumber() == 3);
  } else if ((*bb).getNameId() == "*in_entry_to_for.cond_phi") {
    assert((*srcloc).getLineNumber() == 5);
    assert((*srcloc).getColumnNumber() == 3);
  } else if ((*bb).getNameId() == "*out_for.cond_to_for.end_icmp_false") {
    assert((*srcloc).getLineNumber() == 5);
    assert((*srcloc).getColumnNumber() == 12);
  } else if ((*bb).getNameId() == "*out_for.cond_to_for.body_icmp_true") {
    assert((*srcloc).getLineNumber() == 5);
    assert((*srcloc).getColumnNumber() == 12);
  } else if ((*bb).getNameId() == "for.body") {
    assert((*srcloc).getLineNumber() == 6);
    if ((*var).getName() == "main.arrayidx")
      assert((*srcloc).getColumnNumber() == 12);
    else if ((*var).getName() == "main.arrayidx2")
      assert((*srcloc).getColumnNumber() == 5);
    else
      throw verifier_error(
          "[Unit test failed]: cannot determine srcloc of assignment "
          "statement");
  } else if ((*bb).getNameId() == "for.end") {
    assert((*srcloc).getLineNumber() == 8);
    assert((*srcloc).getColumnNumber() == 3);
  } else {
    throw verifier_error(
        "[Unit test failed]: cannot determine srcloc of assignment statement");
  }
}
}

extern "C" Pass* init() {
  VisitorPass* pass = new VisitorPass();
  return pass;
}
