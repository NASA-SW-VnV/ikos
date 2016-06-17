/*******************************************************************************
 *
 * ARBOS pass for regression test api/factory trac #35
 *
 * Authors: Nija Shi
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2016 United States Government as represented by the
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
#include <arbos/common/common.hpp>
#include <arbos/semantics/ar.hpp>
#include <iostream>
#include <vector>

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
  Verifier() : Visitor() {}
  virtual void nodeStart(AR_Integer_Type&);
  virtual void nodeStart(AR_Internal_Variable&);

  void print(std::ostream& out) { out << "Test passed!" << std::endl; }
};

class VisitorPass : public Pass {
public:
  VisitorPass()
      : Pass("unittest-api-factory", "Verifier pass for unittest-api-factory") {
  }

  virtual ~VisitorPass() {}

  virtual void execute(AR_Node_Ref< AR_Bundle > bundle) {
    std::cout << "This pass verifies regression test api/factory" << std::endl
              << std::endl;
    std::shared_ptr< Verifier > v(new Verifier());
    (*bundle).accept(v);
    v->print(std::cout);
  }
};

// Visitor method implementation
void Verifier::nodeStart(AR_Integer_Type& ty) {
  AR_Node_Ref< AR_Integer_Type > intty(ty.getUID());
  AR_Node_Ref< AR_Operand > op = ARFactory::createConstantInteger(5, intty);
  assert(op.getUID() > 0);
  AR_Node_Ref< AR_Cst_Operand > cstop = node_cast< AR_Cst_Operand >(op);
  AR_Node_Ref< AR_Integer_Constant > intcst =
      node_cast< AR_Integer_Constant >((*cstop).getConstant());
  assert((*intcst).getValue() == 5);
  assert((*intcst).getType().getUID() == ty.getUID());
}

void Verifier::nodeStart(AR_Internal_Variable& var) {
  AR_Node_Ref< AR_Internal_Variable > iv(var.getUID());
  AR_Node_Ref< AR_Operand > op = ARFactory::createVarOperand(iv);
  assert(op.getUID() > 0);
  AR_Node_Ref< AR_Var_Operand > varop = node_cast< AR_Var_Operand >(op);
  assert((*varop).getInternalVariable() == iv);
}
}

extern "C" Pass* init() {
  VisitorPass* pass = new VisitorPass();
  return pass;
}
