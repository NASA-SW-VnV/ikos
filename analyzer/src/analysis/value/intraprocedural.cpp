/*******************************************************************************
 *
 * \file
 * \brief Intraprocedural value analysis implementation
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

#include <memory>
#include <vector>

#include <ikos/core/fixpoint/fwd_fixpoint_iterator.hpp>

#include <ikos/analyzer/analysis/execution_engine/context_insensitive.hpp>
#include <ikos/analyzer/analysis/execution_engine/engine.hpp>
#include <ikos/analyzer/analysis/execution_engine/numerical.hpp>
#include <ikos/analyzer/analysis/fixpoint_profile.hpp>
#include <ikos/analyzer/analysis/pointer/pointer.hpp>
#include <ikos/analyzer/analysis/value/abstract_domain.hpp>
#include <ikos/analyzer/analysis/value/intraprocedural.hpp>
#include <ikos/analyzer/analysis/value/machine_int_domain.hpp>
#include <ikos/analyzer/analysis/variable.hpp>
#include <ikos/analyzer/checker/checker.hpp>
#include <ikos/analyzer/util/demangle.hpp>
#include <ikos/analyzer/util/log.hpp>
#include <ikos/analyzer/util/progress.hpp>
#include <ikos/analyzer/util/timer.hpp>

namespace ikos {
namespace analyzer {

IntraproceduralValueAnalysis::IntraproceduralValueAnalysis(Context& ctx)
    : _ctx(ctx) {}

IntraproceduralValueAnalysis::~IntraproceduralValueAnalysis() = default;

namespace {

using namespace value;

/// \brief Fixpoint on a function body
class FunctionFixpoint final
    : public core::InterleavedFwdFixpointIterator< ar::Code*, AbstractDomain > {
private:
  /// \brief Parent class
  using FwdFixpointIterator =
      core::InterleavedFwdFixpointIterator< ar::Code*, AbstractDomain >;

private:
  /// \brief Analysis context
  Context& _ctx;

  /// \brief Analyzed function
  ar::Function* _function;

  /// \brief Empty call context
  CallContext* _empty_call_context;

  /// \brief Fixpoint profile
  boost::optional< const FixpointProfile& > _profile;

public:
  /// \brief Create a function fixpoint iterator
  FunctionFixpoint(Context& ctx, ar::Function* function)
      : FwdFixpointIterator(function->body()),
        _ctx(ctx),
        _function(function),
        _empty_call_context(ctx.call_context_factory->get_empty()),
        _profile(ctx.fixpoint_profiler == nullptr
                     ? boost::none
                     : ctx.fixpoint_profiler->profile(function)) {}

  /// \brief Extrapolate the new state after an increasing iteration
  AbstractDomain extrapolate(ar::BasicBlock* head,
                             unsigned iteration,
                             AbstractDomain before,
                             AbstractDomain after) override {
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

  /// \brief Check if the decreasing iterations fixpoint is reached
  bool is_decreasing_iterations_fixpoint(const AbstractDomain& before,
                                         const AbstractDomain& after) override {
    if (machine_int_domain_option_has_narrowing(_ctx.opts.machine_int_domain)) {
      return before.leq(after);
    } else {
      return true; // stop after the first decreasing iteration
    }
  }

  /// \brief Propagate the invariant through the basic block
  AbstractDomain analyze_node(ar::BasicBlock* bb, AbstractDomain pre) override {
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

  /// \brief Propagate the invariant through an edge
  AbstractDomain analyze_edge(ar::BasicBlock* src,
                              ar::BasicBlock* dest,
                              AbstractDomain pre) override {
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

  /// \brief Process the computed abstract value for a node
  void process_pre(ar::BasicBlock* /*bb*/,
                   const AbstractDomain& /*pre*/) override {}

  /// \brief Process the computed abstract value for a node
  void process_post(ar::BasicBlock* /*bb*/,
                    const AbstractDomain& /*post*/) override {}

  /// \brief Run the checks with the previously computed fix-point
  void run_checks(const std::vector< std::unique_ptr< Checker > >& checkers) {
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

}; // end class FunctionFixpoint

} // end anonymous namespace

void IntraproceduralValueAnalysis::run() {
  // Bundle
  ar::Bundle* bundle = _ctx.bundle;

  // Create checkers
  std::vector< std::unique_ptr< Checker > > checkers;
  for (CheckerName name : _ctx.opts.analyses) {
    checkers.emplace_back(make_checker(_ctx, name));
  }

  // Initial invariant
  value::AbstractDomain init_inv(
      /*normal=*/value::MemoryAbstractDomain(
          value::PointerAbstractDomain(value::make_top_machine_int_domain(
                                           _ctx.opts.machine_int_domain),
                                       value::NullityAbstractDomain::top()),
          value::UninitializedAbstractDomain::top(),
          value::LifetimeAbstractDomain::top()),
      /*caught_exceptions=*/value::MemoryAbstractDomain::bottom(),
      /*propagated_exceptions=*/value::MemoryAbstractDomain::bottom());

  // Setup a progress logger
  std::unique_ptr< ProgressLogger > progress =
      make_progress_logger(_ctx.opts.progress,
                           LogLevel::Info,
                           /* num_tasks = */
                           2 * std::count_if(bundle->function_begin(),
                                             bundle->function_end(),
                                             [](ar::Function* fun) {
                                               return fun->is_definition();
                                             }));
  ScopeLogger scope(*progress);

  // Analyze every function in the bundle
  for (auto it = bundle->function_begin(), et = bundle->function_end();
       it != et;
       ++it) {
    ar::Function* function = *it;

    // Insert the function in the database
    _ctx.output_db->functions.insert(function);

    if (!function->is_definition()) {
      continue;
    }

    FunctionFixpoint fixpoint(_ctx, function);

    {
      progress->start_task("Analyzing function '" + demangle(function->name()) +
                           "'");
      ScopeTimerDatabase t(_ctx.output_db->times,
                           "ikos-analyzer.value." + function->name());
      fixpoint.run(init_inv);
    }

    {
      progress->start_task("Checking properties for function '" +
                           demangle(function->name()) + "'");
      ScopeTimerDatabase t(_ctx.output_db->times,
                           "ikos-analyzer.check." + function->name());
      fixpoint.run_checks(checkers);
    }
  }
}

} // end namespace analyzer
} // end namespace ikos
