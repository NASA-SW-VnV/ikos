/*******************************************************************************
 *
 * ARBOS pass demonstrating the Visitor API
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

#include <arbos/semantics/ar.hpp>

using namespace arbos;

namespace {

class MyVisitor : public Visitor {
private:
  int _bool_ct;
  int _int_ct;
  int _fp_ct;
  int _struct_ct;
  int _array_ct;
  int _void_ct;
  int _ptr_ct;
  int _func_ct;
  std::vector< index64_t > _visited;

private:
  bool isVisited(index64_t uid) {
    return find(_visited.begin(), _visited.end(), uid) != _visited.end();
  }

public:
  MyVisitor()
      : _bool_ct(0),
        _int_ct(0),
        _fp_ct(0),
        _struct_ct(0),
        _array_ct(0),
        _void_ct(0),
        _ptr_ct(0),
        _func_ct(0) {}

  virtual void nodeStart(AR_Function_Type& func_type);
  virtual void nodeStart(AR_Integer_Type& int_type);
  virtual void nodeStart(AR_FP_Type& fp_type);
  virtual void nodeStart(AR_Void_Type& void_type);
  virtual void nodeStart(AR_Pointer_Type& ptr_type);
  virtual void nodeStart(AR_Structure_Type& struct_type);
  virtual void nodeStart(AR_Array_Type& arr_type);

  void print(std::ostream& out) {
    std::cout << std::endl;
    std::cout << "# of boolean type: " << _bool_ct << std::endl;
    std::cout << "# of integer type: " << _int_ct << std::endl;
    std::cout << "# of fp type: " << _fp_ct << std::endl;
    std::cout << "# of struct type: " << _struct_ct << std::endl;
    std::cout << "# of array type: " << _array_ct << std::endl;
    std::cout << "# of void type: " << _void_ct << std::endl;
    std::cout << "# of pointer type: " << _ptr_ct << std::endl;
    std::cout << "# of function type: " << _func_ct << std::endl;
  }
};

class VisitorPass : public Pass {
public:
  VisitorPass() : Pass("visitor", "Demo pass that uses the Visitor API"){};
  virtual ~VisitorPass() {}

  virtual void execute(AR_Node_Ref< AR_Bundle > bundle) {
    std::cout << "This pass lists all the types used this in the AR"
              << std::endl
              << std::endl;
    std::shared_ptr< MyVisitor > v(new MyVisitor());
    (*bundle).accept(v);
    v->print(std::cout);
  }
};

// Visitor method implementation

void MyVisitor::nodeStart(AR_Function_Type& func_type) {
  index64_t uid = func_type.getUID();
  if (!isVisited(uid)) {
    _visited.push_back(uid);
    func_type.print(std::cout);
    std::cout << std::endl;
    _func_ct++;
  }
}

void MyVisitor::nodeStart(AR_Integer_Type& int_type) {
  index64_t uid = int_type.getUID();
  if (!isVisited(uid)) {
    _visited.push_back(uid);
    int_type.print(std::cout);
    std::cout << std::endl;
    _int_ct++;
  }
}

void MyVisitor::nodeStart(AR_FP_Type& fp_type) {
  index64_t uid = fp_type.getUID();
  if (!isVisited(uid)) {
    _visited.push_back(uid);
    fp_type.print(std::cout);
    std::cout << std::endl;
    _fp_ct++;
  }
}

void MyVisitor::nodeStart(AR_Void_Type& void_type) {
  index64_t uid = void_type.getUID();
  if (!isVisited(uid)) {
    _visited.push_back(uid);
    void_type.print(std::cout);
    std::cout << std::endl;
    _void_ct++;
  }
}

void MyVisitor::nodeStart(AR_Pointer_Type& ptr_type) {
  index64_t uid = ptr_type.getUID();
  if (!isVisited(uid)) {
    _visited.push_back(uid);
    ptr_type.print(std::cout);
    std::cout << std::endl;
    _ptr_ct++;
  }
}

void MyVisitor::nodeStart(AR_Structure_Type& struct_type) {
  index64_t uid = struct_type.getUID();
  if (!isVisited(uid)) {
    _visited.push_back(uid);
    struct_type.print(std::cout);
    std::cout << std::endl;
    _struct_ct++;
  }
}

void MyVisitor::nodeStart(AR_Array_Type& arr_type) {
  index64_t uid = arr_type.getUID();
  if (!isVisited(uid)) {
    _visited.push_back(uid);
    arr_type.print(std::cout);
    std::cout << std::endl;
    _array_ct++;
  }
}
}

extern "C" Pass* init() {
  return new VisitorPass();
}
