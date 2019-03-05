/*******************************************************************************
 *
 * \file
 * \brief Fixpoint on a function body
 *
 * Author: Maxime Arthaud
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

#include <ikos/analyzer/analysis/pointer/pointer.hpp>
#include <ikos/analyzer/analysis/value/interprocedural/function_fixpoint.hpp>

namespace ikos {
namespace analyzer {
namespace value {
namespace interprocedural {

FunctionFixpoint::FunctionFixpoint(
    Context& ctx,
    const std::vector< std::unique_ptr< Checker > >& checkers,
    ProgressLogger& logger,
    ar::Function* entry_point)
    : FwdFixpointIterator(entry_point->body()),
      _ctx(ctx),
      _function(entry_point),
      _call_context(ctx.call_context_factory->get_empty()),
      _profile(ctx.fixpoint_profiler == nullptr
                   ? boost::none
                   : ctx.fixpoint_profiler->profile(entry_point)),
      _analyzed_functions{entry_point},
      _checkers(checkers),
      _logger(logger),
      _exec_engine(AbstractDomain::bottom(),
                   ctx,
                   this->_call_context,
                   /* precision = */ ctx.opts.precision,
                   /* liveness = */ ctx.liveness,
                   /* pointer_info = */ ctx.pointer == nullptr
                       ? nullptr
                       : &ctx.pointer->results()),
      _call_exec_engine(ctx,
                        _exec_engine,
                        *this,
                        /* context_stable = */ true,
                        /* convergence_achieved = */ false) {}

FunctionFixpoint::FunctionFixpoint(Context& ctx,
                                   const FunctionFixpoint& caller,
                                   ar::CallBase* call,
                                   ar::Function* callee,
                                   bool context_stable)
    : FwdFixpointIterator(callee->body()),
      _ctx(ctx),
      _function(callee),
      _call_context(
          ctx.call_context_factory->get_context(caller._call_context, call)),
      _profile(ctx.fixpoint_profiler == nullptr
                   ? boost::none
                   : ctx.fixpoint_profiler->profile(callee)),
      _analyzed_functions(caller._analyzed_functions),
      _checkers(caller._checkers),
      _logger(caller._logger),
      _exec_engine(AbstractDomain::bottom(),
                   ctx,
                   this->_call_context,
                   /* precision = */ ctx.opts.precision,
                   /* liveness = */ ctx.liveness,
                   /* pointer_info = */ ctx.pointer == nullptr
                       ? nullptr
                       : &ctx.pointer->results()),
      _call_exec_engine(ctx,
                        _exec_engine,
                        *this,
                        /* context_stable = */ context_stable,
                        /* convergence_achieved = */ false) {
  this->_analyzed_functions.push_back(callee);
}

void FunctionFixpoint::run(AbstractDomain inv) {
  if (!this->_call_context->empty()) {
    this->_logger.start_callee(this->_call_context, this->_function);
  }

  // Compute the fixpoint
  FwdFixpointIterator::run(std::move(inv));

  // Fixpoint reached
  this->_call_exec_engine.mark_convergence_achieved();

  // Clear post invariants, save a lot of memory
  this->clear_post();

  if (!this->_call_context->empty()) {
    this->_logger.end_callee(this->_call_context, this->_function);
  }
}

AbstractDomain FunctionFixpoint::extrapolate(ar::BasicBlock* head,
                                             unsigned iteration,
                                             AbstractDomain before,
                                             AbstractDomain after) {
  if (iteration <= 1) {
    before.join_iter_with(after);
    return before;
  }
  if (iteration == 2 && this->_profile) {
    if (auto threshold = this->_profile->widening_hint(head)) {
      before.widen_threshold_with(after, *threshold);
      return before;
    }
  }
  before.widen_with(after);
  return before;
}

bool FunctionFixpoint::is_decreasing_iterations_fixpoint(
    const AbstractDomain& before, const AbstractDomain& after) {
  if (machine_int_domain_option_has_narrowing(_ctx.opts.machine_int_domain)) {
    return before.leq(after);
  } else {
    return true; // stop after the first decreasing iteration
  }
}

AbstractDomain FunctionFixpoint::analyze_node(ar::BasicBlock* bb,
                                              AbstractDomain pre) {
  this->_exec_engine.set_inv(std::move(pre));
  this->_exec_engine.exec_enter(bb);
  for (ar::Statement* stmt : *bb) {
    transfer_function(this->_exec_engine, this->_call_exec_engine, stmt);
  }
  this->_exec_engine.exec_leave(bb);
  return std::move(this->_exec_engine.inv());
}

AbstractDomain FunctionFixpoint::analyze_edge(ar::BasicBlock* src,
                                              ar::BasicBlock* dest,
                                              AbstractDomain pre) {
  this->_exec_engine.set_inv(std::move(pre));
  this->_exec_engine.exec_edge(src, dest);
  return std::move(this->_exec_engine.inv());
}

void FunctionFixpoint::notify_enter_cycle(ar::BasicBlock* head) {
  this->_logger.start_cycle(head);
}

void FunctionFixpoint::notify_cycle_iteration(
    ar::BasicBlock* head,
    unsigned iteration,
    core::FixpointIterationKind kind) {
  this->_logger.start_cycle_iter(head, iteration, kind);
}

void FunctionFixpoint::notify_leave_cycle(ar::BasicBlock* head) {
  this->_logger.end_cycle(head);
}

void FunctionFixpoint::process_pre(ar::BasicBlock* /*bb*/,
                                   const AbstractDomain& /*pre*/) {}

void FunctionFixpoint::process_post(ar::BasicBlock* bb,
                                    const AbstractDomain& post) {
  if (this->_function->body()->exit_block_or_null() == bb) {
    this->_exec_engine.set_inv(post);
    this->_call_exec_engine.exec_exit(this->_function);
  }
}

void FunctionFixpoint::run_checks() {
  if (!this->_call_context->empty()) {
    this->_logger.start_callee(this->_call_context, this->_function);
  }

  // Check called functions during the transfer function
  this->_call_exec_engine.mark_check_callees();

  for (ar::BasicBlock* bb : *this->cfg()) {
    this->_exec_engine.set_inv(this->pre(bb));
    this->_exec_engine.exec_enter(bb);

    for (ar::Statement* stmt : *bb) {
      // Check the statement if it's related to an llvm instruction
      if (stmt->has_frontend()) {
        for (const auto& checker : this->_checkers) {
          checker->check(stmt, this->_exec_engine.inv(), this->_call_context);
        }
      }

      // Propagate
      transfer_function(this->_exec_engine, this->_call_exec_engine, stmt);
    }

    this->_exec_engine.exec_leave(bb);
  }

  if (!this->_call_context->empty()) {
    this->_logger.end_callee(this->_call_context, this->_function);
  }
}

bool FunctionFixpoint::is_currently_analyzed(ar::Function* fun) const {
  return std::find(this->_analyzed_functions.begin(),
                   this->_analyzed_functions.end(),
                   fun) != this->_analyzed_functions.end();
}

} // end namespace interprocedural
} // end namespace value
} // end namespace analyzer
} // end namespace ikos
