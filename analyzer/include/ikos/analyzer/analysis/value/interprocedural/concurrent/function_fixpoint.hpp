/*******************************************************************************
 *
 * \file
 * \brief Concurrent fixpoint on a function body
 *
 * Author: Sung Kook Kim
 *
 * Contributor: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2019 United States Government as represented by the
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
#include <ikos/ar/semantic/function.hpp>

#include <ikos/core/fixpoint/concurrent_fwd_fixpoint_iterator.hpp>

#include <ikos/analyzer/analysis/call_context.hpp>
#include <ikos/analyzer/analysis/context.hpp>
#include <ikos/analyzer/analysis/execution_engine/fixpoint_cache.hpp>
#include <ikos/analyzer/analysis/fixpoint_parameters.hpp>
#include <ikos/analyzer/analysis/value/abstract_domain.hpp>
#include <ikos/analyzer/checker/checker.hpp>

namespace ikos {
namespace analyzer {
namespace value {
namespace interprocedural {
namespace concurrent {

/// \brief Concurrent interprocedural fixpoint on a function body
class FunctionFixpoint final
    : public core::InterleavedConcurrentFwdFixpointIterator< ar::Code*,
                                                             AbstractDomain > {
private:
  /// \brief Parent class
  using FwdFixpointIterator =
      core::InterleavedConcurrentFwdFixpointIterator< ar::Code*,
                                                      AbstractDomain >;

  /// \brief Function fixpoint cache of callees
  using FixpointCacheT = FixpointCache< FunctionFixpoint, AbstractDomain >;

private:
  /// \brief Analysis context
  Context& _ctx;

  /// \brief Analyzed function
  ar::Function* _function;

  /// \brief Current call context
  CallContext* _call_context;

  /// \brief Fixpoint parameters
  const CodeFixpointParameters& _fixpoint_parameters;

  /// \brief List of property checks to run
  const std::vector< std::unique_ptr< Checker > >& _checkers;

  /// \brief Mutex for _exit_invariant and _return_stmt
  std::mutex _mutex;

  /// \brief Invariant at the end of the function
  AbstractDomain _exit_invariant;

  /// \brief Return statement, or null
  ar::ReturnValue* _return_stmt;

  /// \brief Function fixpoint cache of callees
  FixpointCacheT _callees_cache;

public:
  /// \brief Constructor for an entry point
  ///
  /// \param ctx Analysis context
  /// \param checkers List of checkers to run
  /// \param entry_point Function to analyze
  FunctionFixpoint(Context& ctx,
                   const std::vector< std::unique_ptr< Checker > >& checkers,
                   ar::Function* entry_point);

  /// \brief Constructor for a callee
  ///
  /// \param ctx Analysis context
  /// \param caller Parent function fixpoint
  /// \param call Call statement
  /// \param callee Called function
  FunctionFixpoint(Context& ctx,
                   const FunctionFixpoint& caller,
                   ar::CallBase* call,
                   ar::Function* callee);

  /// \brief Compute the fixpoint
  void run(AbstractDomain inv) override;

  /// \brief Extrapolate the new state after an increasing iteration
  AbstractDomain extrapolate(ar::BasicBlock* head,
                             unsigned iteration,
                             const AbstractDomain& before,
                             const AbstractDomain& after) override;

  /// \brief Refine the new state after a decreasing iteration
  AbstractDomain refine(ar::BasicBlock* head,
                        unsigned iteration,
                        const AbstractDomain& before,
                        const AbstractDomain& after) override;

  /// \brief Check if the decreasing iterations fixpoint is reached
  bool is_decreasing_iterations_fixpoint(ar::BasicBlock* head,
                                         unsigned iteration,
                                         const AbstractDomain& before,
                                         const AbstractDomain& after) override;

  /// \brief Propagate the invariant through the basic block
  AbstractDomain analyze_node(ar::BasicBlock* bb, AbstractDomain pre) override;

  /// \brief Propagate the invariant through an edge
  AbstractDomain analyze_edge(ar::BasicBlock* src,
                              ar::BasicBlock* dest,
                              AbstractDomain pre) override;

  /// \brief Process the computed abstract value for a node
  void process_pre(ar::BasicBlock* bb, const AbstractDomain& pre) override;

  /// \brief Process the computed abstract value for a node
  void process_post(ar::BasicBlock* bb, const AbstractDomain& post) override;

  /// \brief Run the checks with the previously computed fix-point
  void run_checks();

  /// \name Required by InlineCallExecutionEngine
  /// @{

  /// \brief Return the function
  ar::Function* function() const { return this->_function; }

  /// \brief Return the call context
  CallContext* call_context() const { return this->_call_context; }

  /// \brief Return the exit invariant, or bottom
  const AbstractDomain& exit_invariant() const { return this->_exit_invariant; }

  /// \brief Set the exit invariant
  void set_exit_invariant(AbstractDomain invariant) {
    invariant.normalize();
    std::lock_guard< std::mutex > lock(this->_mutex);
    this->_exit_invariant = std::move(invariant);
  }

  /// \brief Return the return statement, or null
  ar::ReturnValue* return_stmt() const { return this->_return_stmt; }

  /// \brief Set the return statement
  void set_return_stmt(ar::ReturnValue* s) {
    std::lock_guard< std::mutex > lock(this->_mutex);
    ikos_assert_msg(this->_return_stmt == nullptr || this->_return_stmt == s,
                    "code has more than one return statement");
    this->_return_stmt = s;
  }

  /// @}

}; // end class FunctionFixpoint

} // end namespace concurrent
} // end namespace interprocedural
} // end namespace value
} // end namespace analyzer
} // end namespace ikos
