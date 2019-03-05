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

#include <ikos/analyzer/analysis/execution_engine/context_insensitive.hpp>
#include <ikos/analyzer/analysis/execution_engine/engine.hpp>
#include <ikos/analyzer/analysis/execution_engine/numerical.hpp>
#include <ikos/analyzer/analysis/pointer/pointer.hpp>
#include <ikos/analyzer/analysis/value/intraprocedural/function_fixpoint.hpp>

namespace ikos {
namespace analyzer {
namespace value {
namespace intraprocedural {

FunctionFixpoint::FunctionFixpoint(Context& ctx, ar::Function* function)
    : FwdFixpointIterator(function->body()),
      _ctx(ctx),
      _function(function),
      _empty_call_context(ctx.call_context_factory->get_empty()),
      _profile(ctx.fixpoint_profiler == nullptr
                   ? boost::none
                   : ctx.fixpoint_profiler->profile(function)) {}

void FunctionFixpoint::run(AbstractDomain inv) {
  FwdFixpointIterator::run(std::move(inv));
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
  NumericalExecutionEngine< AbstractDomain >
      exec_engine(std::move(pre),
                  _ctx,
                  this->_empty_call_context,
                  /* precision = */ _ctx.opts.precision,
                  /* liveness = */ _ctx.liveness,
                  /* pointer_info = */ _ctx.pointer == nullptr
                      ? nullptr
                      : &_ctx.pointer->results());
  ContextInsensitiveCallExecutionEngine< AbstractDomain > call_exec_engine(
      exec_engine);
  exec_engine.exec_enter(bb);
  for (ar::Statement* stmt : *bb) {
    transfer_function(exec_engine, call_exec_engine, stmt);
  }
  exec_engine.exec_leave(bb);
  return std::move(exec_engine.inv());
}

AbstractDomain FunctionFixpoint::analyze_edge(ar::BasicBlock* src,
                                              ar::BasicBlock* dest,
                                              AbstractDomain pre) {
  NumericalExecutionEngine< AbstractDomain >
      exec_engine(std::move(pre),
                  _ctx,
                  this->_empty_call_context,
                  /* precision = */ _ctx.opts.precision,
                  /* liveness = */ _ctx.liveness,
                  /* pointer_info = */ _ctx.pointer == nullptr
                      ? nullptr
                      : &_ctx.pointer->results());
  exec_engine.exec_edge(src, dest);
  return std::move(exec_engine.inv());
}

void FunctionFixpoint::process_pre(ar::BasicBlock* /*bb*/,
                                   const AbstractDomain& /*pre*/) {}

void FunctionFixpoint::process_post(ar::BasicBlock* /*bb*/,
                                    const AbstractDomain& /*post*/) {}

void FunctionFixpoint::run_checks(
    const std::vector< std::unique_ptr< Checker > >& checkers) {
  for (ar::BasicBlock* bb : *this->cfg()) {
    NumericalExecutionEngine< AbstractDomain >
        exec_engine(this->pre(bb),
                    _ctx,
                    this->_empty_call_context,
                    /* precision = */ _ctx.opts.precision,
                    /* liveness = */ _ctx.liveness,
                    /* pointer_info = */ _ctx.pointer == nullptr
                        ? nullptr
                        : &_ctx.pointer->results());
    ContextInsensitiveCallExecutionEngine< AbstractDomain > call_exec_engine(
        exec_engine);

    exec_engine.exec_enter(bb);

    for (ar::Statement* stmt : *bb) {
      // Check the statement if it's related to an llvm instruction
      if (stmt->has_frontend()) {
        for (const auto& checker : checkers) {
          checker->check(stmt, exec_engine.inv(), this->_empty_call_context);
        }
      }

      // Propagate
      transfer_function(exec_engine, call_exec_engine, stmt);
    }

    exec_engine.exec_leave(bb);
  }
}

} // end namespace intraprocedural
} // end namespace value
} // end namespace analyzer
} // end namespace ikos
