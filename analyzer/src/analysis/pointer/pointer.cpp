/******************************************************************************
 *
 * \file
 * \brief Implementation of the pointer analysis
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *               Clement Decoodt
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

#include <ikos/core/domain/exception/exception.hpp>
#include <ikos/core/domain/machine_int/interval.hpp>
#include <ikos/core/domain/memory/dummy.hpp>
#include <ikos/core/domain/scalar/machine_int.hpp>
#include <ikos/core/domain/uninitialized/separate_domain.hpp>
#include <ikos/core/fixpoint/fwd_fixpoint_iterator.hpp>

#include <ikos/analyzer/analysis/execution_engine/context_insensitive.hpp>
#include <ikos/analyzer/analysis/execution_engine/engine.hpp>
#include <ikos/analyzer/analysis/execution_engine/numerical.hpp>
#include <ikos/analyzer/analysis/fixpoint_parameters.hpp>
#include <ikos/analyzer/analysis/pointer/constraint.hpp>
#include <ikos/analyzer/analysis/pointer/function.hpp>
#include <ikos/analyzer/analysis/pointer/pointer.hpp>
#include <ikos/analyzer/analysis/pointer/value.hpp>
#include <ikos/analyzer/util/demangle.hpp>
#include <ikos/analyzer/util/log.hpp>
#include <ikos/analyzer/util/progress.hpp>

namespace ikos {
namespace analyzer {

PointerAnalysis::PointerAnalysis(
    Context& ctx, const FunctionPointerAnalysis& function_pointer)
    : _ctx(ctx),
      _function_pointer(function_pointer),
      _info(ctx.bundle->data_layout()) {}

PointerAnalysis::~PointerAnalysis() = default;

namespace {

/// \brief Numerical abstract domain for the intra-procedural pointer analysis
using MachineIntAbstractDomain = core::machine_int::IntervalDomain< Variable* >;

/// \brief Uninitialized abstract domain for the intra-procedural pointer
/// analysis
using UninitializedAbstractDomain =
    core::uninitialized::SeparateDomain< Variable* >;

/// \brief Scalar abstract domain for the intra-procedural pointer analysis
using ScalarAbstractDomain =
    core::scalar::MachineIntDomain< Variable*,
                                    MemoryLocation*,
                                    UninitializedAbstractDomain,
                                    MachineIntAbstractDomain >;

/// \brief Memory abstract domain for the intra-procedural pointer analysis
using MemoryAbstractDomain = core::memory::
    DummyDomain< Variable*, MemoryLocation*, ScalarAbstractDomain >;

/// \brief Abstract domain for the intra-procedural pointer analysis
using AbstractDomain = core::exception::ExceptionDomain< MemoryAbstractDomain >;

/// \brief Create the bottom abstract value
AbstractDomain make_bottom_abstract_value() {
  auto bottom = MemoryAbstractDomain(
      ScalarAbstractDomain(UninitializedAbstractDomain::bottom(),
                           MachineIntAbstractDomain::bottom()));
  return AbstractDomain(/*normal = */ bottom,
                        /*caught_exceptions = */ bottom,
                        /*propagated_exceptions = */ bottom);
}

/// \brief Create the initial abstract value
AbstractDomain make_initial_abstract_value() {
  auto top = MemoryAbstractDomain(
      ScalarAbstractDomain(UninitializedAbstractDomain::top(),
                           MachineIntAbstractDomain::top()));
  auto bottom = MemoryAbstractDomain(
      ScalarAbstractDomain(UninitializedAbstractDomain::bottom(),
                           MachineIntAbstractDomain::bottom()));
  return AbstractDomain(/*normal = */ top,
                        /*caught_exceptions = */ bottom,
                        /*propagated_exceptions = */ bottom);
}

/// \brief Numerical invariants on an ar::Code
class NumericalCodeInvariants final
    : public core::InterleavedFwdFixpointIterator< ar::Code*, AbstractDomain > {
public:
  using AbstractDomainT = AbstractDomain;

private:
  /// \brief Parent class
  using FwdFixpointIterator =
      core::InterleavedFwdFixpointIterator< ar::Code*, AbstractDomainT >;

private:
  /// \brief Analysis context
  Context& _ctx;

  /// \brief Empty call context
  CallContext* _empty_call_context;

  /// \brief Fixpoint parameters
  const CodeFixpointParameters& _fixpoint_parameters;

  /// \brief Previously computed function pointer analysis
  const FunctionPointerAnalysis& _function_pointer;

public:
  /// \brief Constructor
  NumericalCodeInvariants(Context& ctx,
                          const FunctionPointerAnalysis& function_pointer,
                          ar::Code* code)
      : FwdFixpointIterator(code, make_bottom_abstract_value()),
        _ctx(ctx),
        _empty_call_context(ctx.call_context_factory->get_empty()),
        _fixpoint_parameters(
            code->is_function_body()
                ? ctx.fixpoint_parameters->get(code->function())
                : ctx.fixpoint_parameters->default_params()),
        _function_pointer(function_pointer) {}

  /// \brief Extrapolate the new state after an increasing iteration
  AbstractDomainT extrapolate(ar::BasicBlock* head,
                              unsigned iteration,
                              const AbstractDomainT& before,
                              const AbstractDomainT& after) override {
    if (iteration <= this->_fixpoint_parameters.widening_delay) {
      // Fixed number of iterations using join
      return before.join_iter(after);
    }

    iteration -= this->_fixpoint_parameters.widening_delay;
    iteration--;

    if (iteration % this->_fixpoint_parameters.widening_period != 0) {
      // Not the period, iteration using join
      return before.join_iter(after);
    }

    switch (this->_fixpoint_parameters.widening_strategy) {
      case WideningStrategy::Widen: {
        if (iteration == 0) {
          if (auto threshold =
                  this->_fixpoint_parameters.widening_hints.get(head)) {
            // One iteration using widening with threshold
            return before.widening_threshold(after, *threshold);
          }
        }

        // Iterations using widening until convergence
        return before.widening(after);
      }
      case WideningStrategy::Join: {
        // Iterations using join until convergence
        return before.join_iter(after);
      }
      default: {
        ikos_unreachable("unexpected strategy");
      }
    }
  }

  /// \brief Refine the new state after a decreasing iteration
  AbstractDomainT refine(ar::BasicBlock* head,
                         unsigned iteration,
                         const AbstractDomainT& before,
                         const AbstractDomainT& after) override {
    switch (this->_fixpoint_parameters.narrowing_strategy) {
      case NarrowingStrategy::Narrow: {
        if (iteration == 1) {
          if (auto threshold =
                  this->_fixpoint_parameters.widening_hints.get(head)) {
            // First iteration using narrowing with threshold
            return before.narrowing_threshold(after, *threshold);
          }
        }

        // Iterations using narrowing
        return before.narrowing(after);
      }
      case NarrowingStrategy::Meet: {
        // Iterations using meet
        return before.meet(after);
      }
      default: {
        ikos_unreachable("unexpected strategy");
      }
    }
  }

  bool is_decreasing_iterations_fixpoint(
      ar::BasicBlock* /*head*/,
      unsigned iteration,
      const AbstractDomainT& before,
      const AbstractDomainT& after) override {
    // Check if we reached the number of requested iterations, or convergence
    return (this->_fixpoint_parameters.narrowing_iterations &&
            iteration >= *this->_fixpoint_parameters.narrowing_iterations) ||
           before.leq(after);
  }

  /// \brief Propagate the invariant through the basic block
  AbstractDomainT analyze_node(ar::BasicBlock* bb,
                               AbstractDomainT pre) override {
    NumericalExecutionEngine< AbstractDomainT >
        exec_engine(std::move(pre),
                    _ctx,
                    this->_empty_call_context,
                    ExecutionEngine::NoOption,
                    /* liveness = */ _ctx.liveness,
                    /* pointer_info = */ &_function_pointer.results());
    ContextInsensitiveCallExecutionEngine< AbstractDomainT > call_exec_engine(
        exec_engine);
    exec_engine.exec_enter(bb);
    for (ar::Statement* stmt : *bb) {
      transfer_function(exec_engine, call_exec_engine, stmt);
    }
    exec_engine.exec_leave(bb);
    return std::move(exec_engine.inv());
  }

  /// \brief Propagate the invariant through an edge
  AbstractDomainT analyze_edge(ar::BasicBlock* src,
                               ar::BasicBlock* dest,
                               AbstractDomainT pre) override {
    NumericalExecutionEngine< AbstractDomainT >
        exec_engine(std::move(pre),
                    _ctx,
                    this->_empty_call_context,
                    ExecutionEngine::NoOption,
                    /* liveness = */ _ctx.liveness,
                    /* pointer_info = */ &_function_pointer.results());
    exec_engine.exec_edge(src, dest);
    return std::move(exec_engine.inv());
  }

  /// \brief Process the computed abstract value for a node
  void process_pre(ar::BasicBlock* /*bb*/,
                   const AbstractDomainT& /*pre*/) override {}

  /// \brief Process the computed abstract value for a node
  void process_post(ar::BasicBlock* /*bb*/,
                    const AbstractDomainT& /*post*/) override {}

  /// \brief Get the invariant at the entry of the given basic block
  AbstractDomainT entry(ar::BasicBlock* bb) const {
    NumericalExecutionEngine< AbstractDomainT >
        exec_engine(this->pre(bb),
                    _ctx,
                    this->_empty_call_context,
                    ExecutionEngine::NoOption,
                    /* liveness = */ _ctx.liveness,
                    /* pointer_info = */ &_function_pointer.results());
    exec_engine.exec_enter(bb);
    return std::move(exec_engine.inv());
  }

  /// \brief Execute the given statement with the given invariant
  AbstractDomainT analyze_statement(ar::Statement* stmt,
                                    AbstractDomainT pre) const {
    NumericalExecutionEngine< AbstractDomainT >
        exec_engine(std::move(pre),
                    _ctx,
                    this->_empty_call_context,
                    ExecutionEngine::NoOption,
                    /* liveness = */ _ctx.liveness,
                    /* pointer_info = */ &_function_pointer.results());
    ContextInsensitiveCallExecutionEngine< AbstractDomainT > call_exec_engine(
        exec_engine);
    transfer_function(exec_engine, call_exec_engine, stmt);
    return std::move(exec_engine.inv());
  }
};

} // end anonymous namespace

void PointerAnalysis::run() {
  ar::Bundle* bundle = _ctx.bundle;

  // Setup a progress logger
  std::unique_ptr< ProgressLogger > progress =
      make_progress_logger(_ctx.opts.progress,
                           LogLevel::Info,
                           /* num_tasks = */
                           2 * std::count_if(bundle->global_begin(),
                                             bundle->global_end(),
                                             [](ar::GlobalVariable* gv) {
                                               return gv->is_definition();
                                             }) +
                               2 * std::count_if(bundle->function_begin(),
                                                 bundle->function_end(),
                                                 [](ar::Function* fun) {
                                                   return fun->is_definition();
                                                 }) +
                               1);
  ScopeLogger scope(*progress);

  log::debug("Generating pointer constraints");
  PointerConstraints constraints(bundle->data_layout());

  PointerConstraintsGenerator< NumericalCodeInvariants >
      visitor(_ctx, constraints, &_function_pointer.results());

  for (auto it = bundle->global_begin(), et = bundle->global_end(); it != et;
       ++it) {
    ar::GlobalVariable* gv = *it;
    if (gv->is_definition()) {
      progress->start_task(
          "Generating intra-procedural numerical invariant for initializer of "
          "global variable '" +
          demangle(gv->name()) + "'");
      NumericalCodeInvariants invariants(_ctx,
                                         _function_pointer,
                                         gv->initializer());
      invariants.run(make_initial_abstract_value());

      progress->start_task(
          "Generating pointer constraints for initializer of global variable "
          "'" +
          demangle(gv->name()) + "'");
      visitor.process_global_var_def(gv, invariants);
    } else {
      visitor.process_global_var_decl(gv);
    }
  }

  for (auto it = bundle->function_begin(), et = bundle->function_end();
       it != et;
       ++it) {
    ar::Function* fun = *it;
    if (fun->is_definition()) {
      progress->start_task(
          "Generating intra-procedural numerical invariant for function '" +
          demangle(fun->name()) + "'");
      NumericalCodeInvariants invariants(_ctx, _function_pointer, fun->body());
      invariants.run(make_initial_abstract_value());

      progress->start_task("Generating pointer constraints for function '" +
                           demangle(fun->name()) + "'");
      visitor.process_function_def(fun, invariants);
    } else {
      visitor.process_function_decl(fun);
    }
  }

  log::debug("Solving pointer constraints");
  progress->start_task("Solving pointer constraints");
  constraints.solve();

  // Save information
  constraints.results(this->_info);
}

/// \brief Dump the pointer analysis results, for debugging purpose
void PointerAnalysis::dump(std::ostream& o) const {
  o << "Pointer analysis results:\n";
  this->_info.dump(o);
}

} // end namespace analyzer
} // end namespace ikos
