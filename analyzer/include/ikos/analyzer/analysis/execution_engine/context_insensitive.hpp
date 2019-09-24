/******************************************************************************
 *
 * \file
 * \brief Context insensitive call semantic
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2018-2019 United States Government as represented by the
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

#pragma once

#include <ikos/analyzer/analysis/execution_engine/engine.hpp>
#include <ikos/analyzer/analysis/execution_engine/numerical.hpp>

namespace ikos {
namespace analyzer {

/// \brief Context insensitive call semantic
///
/// It safely ignores function calls, except for direct intrinsic calls.
///
/// Some assumptions are made about the program, see
/// NumericalExecutionEngine::exec_unknown_intern_call() for more info.
template < typename AbstractDomain >
class ContextInsensitiveCallExecutionEngine final : public CallExecutionEngine {
public:
  using NumericalExecutionEngineT = NumericalExecutionEngine< AbstractDomain >;

private:
  /// \brief Numerical execution engine
  NumericalExecutionEngineT& _engine;

public:
  /// \brief Constructor
  explicit ContextInsensitiveCallExecutionEngine(
      NumericalExecutionEngineT& engine)
      : _engine(engine) {}

  /// \brief Exit a function
  ///
  /// This is called whenever we reach the exit node (if there is one).
  ///
  /// Note that this is different from exec(ar::Returnvalue*) if there is
  /// exceptions. This can be used to catch the invariant, including pending
  /// exceptions.
  void exec_exit(ar::Function*) override {}

  /// \brief Execute any call statement
  void exec(ar::CallBase* call) {
    if (auto cst = dyn_cast< ar::FunctionPointerConstant >(call->called())) {
      // Direct call
      ar::Function* fun = cst->function();

      if (fun->is_declaration()) {
        // Extern function
        this->_engine.exec_extern_call(call, fun);
        return;
      }
    }

    // Otherwise
    this->_engine.exec_unknown_intern_call(call);
  }

  /// \brief Execute a Call statement
  void exec(ar::Call* s) override {
    // Execute the call statement
    this->exec(cast< ar::CallBase >(s));

    // Exceptions aren't caught, propagate them
    this->_engine.inv().merge_caught_in_propagated_exceptions();
  }

  /// \brief Execute an Invoke statement
  void exec(ar::Invoke* s) override {
    // Execute the call base statement
    this->exec(cast< ar::CallBase >(s));

    // Exceptions are caught.
    // Nothing to do here.
    // see NumericalExecutionEngine::exec_edge()
  }

  /// \brief Execute a ReturnValue statement
  void exec(ar::ReturnValue*) override {}

}; // end class ContextInsensitiveCallExecutionEngine

} // end namespace analyzer
} // end namespace ikos
