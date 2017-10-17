/*******************************************************************************
 *
 * ARBOS pass for regression test parsing-uid-internal_var
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
private:
  std::vector< AR_Node_Ref< AR_Internal_Variable > > _internal_vars;
  std::unordered_map< index64_t, int > _usages_ct;

  inline void add_ref(index64_t uid) {
    if (_usages_ct.find(uid) == _usages_ct.end()) {
      _usages_ct[uid] = 0;
    }
    _usages_ct[uid]++;
  }

  inline bool contains(AR_Node_Ref< AR_Internal_Variable > v) {
    std::vector< AR_Node_Ref< AR_Internal_Variable > >::iterator i =
        _internal_vars.begin();
    for (; i != _internal_vars.end(); i++) {
      if ((*i).getUID() == v.getUID()) {
        assert((**i).getName() == (*v).getName());
        add_ref(v.getUID());
        return true;
      }
    }
    return false;
  }

public:
  Verifier() {}
  virtual void nodeStart(AR_Code&);
  virtual void nodeStart(AR_Var_Operand&);

  void print(std::ostream& out) {
    std::unordered_map< index64_t, int >::iterator p = _usages_ct.begin();
    for (; p != _usages_ct.end(); p++) {
      assert(_usages_ct[p->first] >= 0);
    }
    out << "Test passed!" << std::endl;
  }
};

class VisitorPass : public Pass {
public:
  VisitorPass()
      : Pass("unittest-parsing-uid-internal_var",
             "Verifier pass for unittest-parsing-uid-internal_var"){};

  virtual ~VisitorPass() {}

  virtual void execute(AR_Node_Ref< AR_Bundle > bundle) {
    std::cout << "This pass verifies regression test parsing/uid-internal_var"
              << std::endl
              << std::endl;
    std::shared_ptr< Verifier > v(new Verifier());
    (*bundle).accept(v);
    v->print(std::cout);
  }
};

// Visitor method implementation

void Verifier::nodeStart(AR_Code& code) {
  // Updates current scope, which is AR_Code
  _internal_vars = code.getInternalVariables();
  for (auto& p : _internal_vars) {
    std::cout << (*p).getName() << std::endl;
  }
  assert(_internal_vars.size() == 8);
}

void Verifier::nodeStart(AR_Var_Operand& op) {
  AR_Node_Ref< AR_Internal_Variable > v = op.getInternalVariable();
  assert(contains(v));
}
}

extern "C" Pass* init() {
  VisitorPass* pass = new VisitorPass();
  return pass;
}
