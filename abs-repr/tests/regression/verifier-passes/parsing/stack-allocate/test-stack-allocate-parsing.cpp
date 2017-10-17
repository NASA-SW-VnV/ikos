/*******************************************************************************
 *
 * ARBOS pass for regression test parsing/vla trac #303
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
  bool _function_visited;

public:
  Verifier() : _function_visited(false) {}
  virtual void nodeStart(AR_Allocate&);

  void print(std::ostream& out) { out << "Test passed!" << std::endl; }
  inline bool testPassed() { return _function_visited; }
};

class VisitorPass : public Pass {
public:
  VisitorPass()
      : Pass("unittest-parsing-vla",
             "Verifier pass for unittest-parsing-vla"){};

  virtual ~VisitorPass() {}

  virtual void execute(AR_Node_Ref< AR_Bundle > bundle) {
    std::cout << "This pass verifies regression test parsing/vla" << std::endl
              << std::endl;
    std::shared_ptr< Verifier > v(new Verifier());
    (*bundle).accept(v);
    if (v->testPassed()) {
      v->print(std::cout);
    }
  }
};

// Visitor method implementation
void Verifier::nodeStart(AR_Allocate& alloca) {
  AR_Node_Ref< AR_Function > f =
      (*(*alloca.getContainingBasicBlock()).getContainingCode())
          .getContainingFunction();
  if ((*f).getFunctionId() != "foo")
    return;
  _function_visited = true;
  std::vector< AR_Node_Ref< AR_Local_Variable > >& localvars =
      (*f).getLocalVariables();
  assert(localvars.size() == 1);
  AR_Node_Ref< AR_Local_Variable > vla = localvars[0];

  assert((*alloca.getLocalVariableAddress()).getVariable().getUID() ==
         vla.getUID());
  AR_Node_Ref< AR_Var_Operand > array_size =
      node_cast< AR_Var_Operand >(alloca.getArraySize());
  assert(array_size.getUID() > 0);
  AR_Node_Ref< AR_Integer_Type > allocated_ty =
      node_cast< AR_Integer_Type >(alloca.getAllocatedType());
  assert(allocated_ty.getUID() > 0);
}
}
extern "C" Pass* init() {
  VisitorPass* pass = new VisitorPass();
  return pass;
}
