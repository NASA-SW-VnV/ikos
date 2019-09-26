/*******************************************************************************
 *
 * \file
 * \brief Generic API for executing statements
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2019 United States Government as represented by the
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

#include <ikos/ar/semantic/code.hpp>
#include <ikos/ar/semantic/statement.hpp>
#include <ikos/ar/semantic/statement_visitor.hpp>

#include <ikos/analyzer/json/json.hpp>
#include <ikos/analyzer/support/flags.hpp>

namespace ikos {
namespace analyzer {

/// \brief Execution engine for AR (Abstract Representation) statements
///
/// This is the base class for execution engines.
///
/// This has to be used together with a `CallExecutionEngine` to execute
/// function calls and return statements. This allows to have different
/// inter-procedural strategies (e.g., inlining, summaries-based,
/// intra-procedural, etc).
class ExecutionEngine {
public:
  /// \brief Execution engine options
  enum ExecutionEngineOption : unsigned {
    NoOption = 0x0,

    /// \brief Update the allocation size variable
    UpdateAllocSizeVar = 0x1,
  };

  /// \brief Execution engine options
  using ExecutionEngineOptions = Flags< ExecutionEngineOption >;

public:
  /// \brief Constructor
  ExecutionEngine() noexcept = default;

  /// \brief Copy constructor
  ExecutionEngine(const ExecutionEngine&) noexcept = default;

  /// \brief Move constructor
  ExecutionEngine(ExecutionEngine&&) noexcept = default;

  /// \brief Copy assignment operator
  ExecutionEngine& operator=(const ExecutionEngine&) noexcept = default;

  /// \brief Move assignment operator
  ExecutionEngine& operator=(ExecutionEngine&&) noexcept = default;

  /// \brief Destructor
  virtual ~ExecutionEngine() = default;

  /// \brief Enter a basic block
  virtual void exec_enter(ar::BasicBlock* bb) = 0;

  /// \brief Leave a basic block
  virtual void exec_leave(ar::BasicBlock* bb) = 0;

  /// \brief Execute an edge from `src` to `dest`
  virtual void exec_edge(ar::BasicBlock* src, ar::BasicBlock* dest) = 0;

  /// \brief Execute an Assignment statement
  virtual void exec(ar::Assignment* s) = 0;

  /// \brief Execute an UnaryOperation statement
  virtual void exec(ar::UnaryOperation* s) = 0;

  /// \brief Execute a BinaryOperation statement
  virtual void exec(ar::BinaryOperation* s) = 0;

  /// \brief Execute a Comparison statement
  virtual void exec(ar::Comparison* s) = 0;

  /// \brief Execute an Unreachable statement
  virtual void exec(ar::Unreachable* s) = 0;

  /// \brief Execute an Allocate statement
  virtual void exec(ar::Allocate* s) = 0;

  /// \brief Execute a PointerShift statement
  virtual void exec(ar::PointerShift* s) = 0;

  /// \brief Execute a Load statement
  virtual void exec(ar::Load* s) = 0;

  /// \brief Execute a Store statement
  virtual void exec(ar::Store* s) = 0;

  /// \brief Execute an ExtractElement statement
  virtual void exec(ar::ExtractElement* s) = 0;

  /// \brief Execute an InsertElement statement
  virtual void exec(ar::InsertElement* s) = 0;

  /// \brief Execute a ShuffleVector statement
  virtual void exec(ar::ShuffleVector* s) = 0;

  /// \brief Execute a LandingPad statement
  virtual void exec(ar::LandingPad* s) = 0;

  /// \brief Execute a Resume statement
  virtual void exec(ar::Resume* s) = 0;

  /// \brief Execute a call to the given extern function
  virtual void exec_extern_call(ar::CallBase* call, ar::Function* fun) = 0;

  /// \brief Execute a call to the given intrinsic function
  virtual void exec_intrinsic_call(ar::CallBase* call, ar::Intrinsic::ID) = 0;

  /// \brief Execute a call to an unknown extern function
  virtual void exec_unknown_extern_call(ar::CallBase* call) = 0;

  /// \brief Execute a call to an unknown internal function
  virtual void exec_unknown_intern_call(ar::CallBase* call) = 0;

  /// \brief Execute a call to an unknown function
  ///
  /// \param call
  ///   The call statement
  /// \param may_write_params
  ///   True if the function call might write on a pointer parameter
  /// \param ignore_unknown_write
  ///   True to ignore writes on unknown pointer parameters (unsound)
  /// \param may_write_globals
  ///   True if the function call might update a global variable
  /// \param may_throw_exc
  ///   True if the function call might throw an exception
  virtual void exec_unknown_call(ar::CallBase* call,
                                 bool may_write_params,
                                 bool ignore_unknown_write,
                                 bool may_write_globals,
                                 bool may_throw_exc) = 0;

  /// \brief Assign formal parameters to the actual arguments of a function call
  virtual void match_down(ar::CallBase* call, ar::Function* called) = 0;

  /// \brief Assign the return value of a function call
  virtual void match_up(ar::CallBase* call, ar::ReturnValue* ret) = 0;

}; // end class ExecutionEngine

/// \brief Execution engine for Call statements
///
/// This is used to execute function calls and return statements, based on
/// different inter-procedural strategies (e.g., inlining, summaries-based,
/// intra-procedural, etc).
class CallExecutionEngine {
public:
  /// \brief Constructor
  CallExecutionEngine() noexcept = default;

  /// \brief Copy constructor
  CallExecutionEngine(const CallExecutionEngine&) noexcept = default;

  /// \brief Move constructor
  CallExecutionEngine(CallExecutionEngine&&) noexcept = default;

  /// \brief Copy assignment operator
  CallExecutionEngine& operator=(const CallExecutionEngine&) noexcept = default;

  /// \brief Move assignment operator
  CallExecutionEngine& operator=(CallExecutionEngine&&) noexcept = default;

  /// \brief Destructor
  virtual ~CallExecutionEngine() = default;

  /// \brief Exit a function
  ///
  /// This is called whenever we reach the exit node (if there is one).
  ///
  /// Note that this is different from exec(ar::Returnvalue*) if there is
  /// exceptions. This can be used to catch the invariant, including pending
  /// exceptions.
  virtual void exec_exit(ar::Function*) = 0;

  /// \brief Execute a Call statement
  virtual void exec(ar::Call* s) = 0;

  /// \brief Execute an Invoke statement
  virtual void exec(ar::Invoke* s) = 0;

  /// \brief Execute a ReturnValue statement
  virtual void exec(ar::ReturnValue* s) = 0;

}; // end class CallExecutionEngine

/// \brief Execute the transfert function for a Statement
template < typename ExecEngine, typename CallExecEngine >
inline void transfer_function(ExecEngine& exec_engine,
                              CallExecEngine& call_exec_engine,
                              ar::Statement* stmt) {
  struct StatementVisitor {
    using ResultType = void;

    ExecEngine& exec_engine;
    CallExecEngine& call_exec_engine;

    void operator()(ar::Assignment* s) { exec_engine.exec(s); }
    void operator()(ar::UnaryOperation* s) { exec_engine.exec(s); }
    void operator()(ar::BinaryOperation* s) { exec_engine.exec(s); }
    void operator()(ar::Comparison* s) { exec_engine.exec(s); }
    void operator()(ar::ReturnValue* s) { call_exec_engine.exec(s); }
    void operator()(ar::Unreachable* s) { exec_engine.exec(s); }
    void operator()(ar::Allocate* s) { exec_engine.exec(s); }
    void operator()(ar::PointerShift* s) { exec_engine.exec(s); }
    void operator()(ar::Load* s) { exec_engine.exec(s); }
    void operator()(ar::Store* s) { exec_engine.exec(s); }
    void operator()(ar::ExtractElement* s) { exec_engine.exec(s); }
    void operator()(ar::InsertElement* s) { exec_engine.exec(s); }
    void operator()(ar::ShuffleVector* s) { exec_engine.exec(s); }
    void operator()(ar::Call* s) { call_exec_engine.exec(s); }
    void operator()(ar::Invoke* s) { call_exec_engine.exec(s); }
    void operator()(ar::LandingPad* s) { exec_engine.exec(s); }
    void operator()(ar::Resume* s) { exec_engine.exec(s); }
  };
  StatementVisitor visitor{exec_engine, call_exec_engine};
  ar::apply_visitor(visitor, stmt);
}

} // end namespace analyzer
} // end namespace ikos
