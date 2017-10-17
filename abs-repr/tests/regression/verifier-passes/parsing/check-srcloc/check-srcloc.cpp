/*******************************************************************************
 *
 * ARBOS pass for checking availablity of srcloc information
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

  virtual void nodeStart(AR_Integer_Comparison& icmp);
  virtual void nodeStart(AR_FP_Comparison& fcmp);
  virtual void nodeStart(AR_Assignment& assign);
  virtual void nodeStart(AR_Arith_Op& arith_op);
  virtual void nodeStart(AR_Conv_Op& conv_op);
  virtual void nodeStart(AR_Bitwise_Op& bitwise_op);
  virtual void nodeStart(AR_FP_Op& fp_op);
  virtual void nodeStart(AR_Store& store);
  virtual void nodeStart(AR_Load& load);
  virtual void nodeStart(AR_Insert_Element& insert_element);
  virtual void nodeStart(AR_Extract_Element& extract_element);
  virtual void nodeStart(AR_NOP& nop);
  virtual void nodeStart(AR_Unreachable& unreachable);
  virtual void nodeStart(AR_Return_Value& ret_val);
  virtual void nodeStart(AR_MemCpy& memcpy);
  virtual void nodeStart(AR_MemMove& memmv);
  virtual void nodeStart(AR_MemSet& memset);
  virtual void nodeStart(AR_Pointer_Shift& ptr_shift);
  virtual void nodeStart(AR_Call& call);
  virtual void nodeStart(AR_Invoke& meth_call);
  virtual void nodeStart(AR_Abstract_Variable& ab_var);
  virtual void nodeStart(AR_Abstract_Memory& ab_mem);
  virtual void nodeStart(AR_VA_Start& va_start);
  virtual void nodeStart(AR_VA_End& va_end);
  virtual void nodeStart(AR_VA_Arg& va_arg);
  virtual void nodeStart(AR_VA_Copy& va_copy);

  void print(std::ostream& out) { out << "Test passed!" << std::endl; }

private:
  void verify_srcline(AR_Node_Ref< AR_Statement > stmt) {
    AR_Node_Ref< AR_Source_Location > srcloc = (*stmt).getSourceLocation();
    assert(!(*srcloc).getFilename().empty());
    assert((*srcloc).getLineNumber() > 0);
  }
};

class VisitorPass : public Pass {
public:
  VisitorPass()
      : Pass("check-srcloc", "Verifier pass for parsing-check-srcloc") {}

  virtual ~VisitorPass() {}

  virtual void execute(AR_Node_Ref< AR_Bundle > bundle) {
    std::cout << "This pass verifies regression test parsing/check-srcloc"
              << std::endl
              << std::endl;
    std::shared_ptr< Verifier > v(new Verifier());
    (*bundle).accept(v);
    v->print(std::cout);
  }
};

// Visitor method implementation

void Verifier::nodeStart(AR_Integer_Comparison& icmp) {
  verify_srcline(AR_Node_Ref< AR_Statement >(icmp.getUID()));
}

void Verifier::nodeStart(AR_FP_Comparison& fcmp) {
  verify_srcline(AR_Node_Ref< AR_Statement >(fcmp.getUID()));
}

void Verifier::nodeStart(AR_Assignment& assign) {
  verify_srcline(AR_Node_Ref< AR_Statement >(assign.getUID()));
}

void Verifier::nodeStart(AR_Arith_Op& arith_op) {
  verify_srcline(AR_Node_Ref< AR_Statement >(arith_op.getUID()));
}

void Verifier::nodeStart(AR_Conv_Op& conv_op) {
  verify_srcline(AR_Node_Ref< AR_Statement >(conv_op.getUID()));
}

void Verifier::nodeStart(AR_Bitwise_Op& bitwise_op) {
  verify_srcline(AR_Node_Ref< AR_Statement >(bitwise_op.getUID()));
}

void Verifier::nodeStart(AR_FP_Op& fp_op) {
  verify_srcline(AR_Node_Ref< AR_Statement >(fp_op.getUID()));
}

void Verifier::nodeStart(AR_Store& store) {
  verify_srcline(AR_Node_Ref< AR_Statement >(store.getUID()));
}

void Verifier::nodeStart(AR_Load& load) {
  verify_srcline(AR_Node_Ref< AR_Statement >(load.getUID()));
}

void Verifier::nodeStart(AR_Insert_Element& insert_element) {
  verify_srcline(AR_Node_Ref< AR_Statement >(insert_element.getUID()));
}

void Verifier::nodeStart(AR_Extract_Element& extract_element) {
  verify_srcline(AR_Node_Ref< AR_Statement >(extract_element.getUID()));
}

void Verifier::nodeStart(AR_NOP& nop) {
  verify_srcline(AR_Node_Ref< AR_Statement >(nop.getUID()));
}

void Verifier::nodeStart(AR_Unreachable& unreachable) {
  verify_srcline(AR_Node_Ref< AR_Statement >(unreachable.getUID()));
}

void Verifier::nodeStart(AR_Return_Value& ret_val) {
  verify_srcline(AR_Node_Ref< AR_Statement >(ret_val.getUID()));
}

void Verifier::nodeStart(AR_MemCpy& memcpy) {
  verify_srcline(AR_Node_Ref< AR_Statement >(memcpy.getUID()));
}

void Verifier::nodeStart(AR_MemMove& memmv) {
  verify_srcline(AR_Node_Ref< AR_Statement >(memmv.getUID()));
}

void Verifier::nodeStart(AR_MemSet& memset) {
  verify_srcline(AR_Node_Ref< AR_Statement >(memset.getUID()));
}

void Verifier::nodeStart(AR_Pointer_Shift& ptr_shift) {
  verify_srcline(AR_Node_Ref< AR_Statement >(ptr_shift.getUID()));
}

void Verifier::nodeStart(AR_Call& call) {
  verify_srcline(AR_Node_Ref< AR_Statement >(call.getUID()));
}

void Verifier::nodeStart(AR_Invoke& meth_call) {
  verify_srcline(AR_Node_Ref< AR_Statement >(meth_call.getUID()));
}

void Verifier::nodeStart(AR_Abstract_Variable& ab_var) {
  verify_srcline(AR_Node_Ref< AR_Statement >(ab_var.getUID()));
}

void Verifier::nodeStart(AR_Abstract_Memory& ab_mem) {
  verify_srcline(AR_Node_Ref< AR_Statement >(ab_mem.getUID()));
}

void Verifier::nodeStart(AR_VA_Start& va_start) {
  verify_srcline(AR_Node_Ref< AR_Statement >(va_start.getUID()));
}

void Verifier::nodeStart(AR_VA_End& va_end) {
  verify_srcline(AR_Node_Ref< AR_Statement >(va_end.getUID()));
}

void Verifier::nodeStart(AR_VA_Arg& va_arg) {
  verify_srcline(AR_Node_Ref< AR_Statement >(va_arg.getUID()));
}

void Verifier::nodeStart(AR_VA_Copy& va_copy) {
  verify_srcline(AR_Node_Ref< AR_Statement >(va_copy.getUID()));
}
}

extern "C" Pass* init() {
  VisitorPass* pass = new VisitorPass();
  return pass;
}
