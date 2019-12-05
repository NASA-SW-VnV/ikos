/*******************************************************************************
 *
 * \file
 * \brief Fixpoint on a global variable initializer
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
#include <ikos/analyzer/analysis/value/global_variable.hpp>
#include <ikos/analyzer/analysis/value/interprocedural/sequential/global_init_fixpoint.hpp>

namespace ikos {
namespace analyzer {
namespace value {
namespace interprocedural {
namespace sequential {

GlobalVarInitializerFixpoint::GlobalVarInitializerFixpoint(
    Context& ctx, ar::GlobalVariable* gv)
    : FwdFixpointIterator(gv->initializer(), make_bottom_abstract_value(ctx)),
      _gv(gv),
      _ctx(ctx),
      _empty_call_context(ctx.call_context_factory->get_empty()) {}

void GlobalVarInitializerFixpoint::run(AbstractDomain inv) {
  // Allocate memory for the global variable
  NumericalExecutionEngineT exec_engine(std::move(inv),
                                        _ctx,
                                        this->_empty_call_context,
                                        ExecutionEngine::UpdateAllocSizeVar,
                                        /* liveness = */ _ctx.liveness,
                                        /* pointer_info = */ _ctx.pointer ==
                                                nullptr
                                            ? nullptr
                                            : &_ctx.pointer->results());
  exec_engine
      .allocate_memory(_ctx.var_factory->get_global(_gv),
                       _ctx.mem_factory->get_global(_gv),
                       core::Nullity::non_null(),
                       core::Lifetime::top(),
                       NumericalExecutionEngineT::MemoryInitialValue::Zero);

  // Compute the fixpoint
  FwdFixpointIterator::run(std::move(exec_engine.inv()));
}

AbstractDomain GlobalVarInitializerFixpoint::analyze_node(ar::BasicBlock* bb,
                                                          AbstractDomain pre) {
  NumericalExecutionEngineT exec_engine(std::move(pre),
                                        _ctx,
                                        this->_empty_call_context,
                                        ExecutionEngine::UpdateAllocSizeVar,
                                        /* liveness = */ _ctx.liveness,
                                        /* pointer_info = */ _ctx.pointer ==
                                                nullptr
                                            ? nullptr
                                            : &_ctx.pointer->results());
  GlobalVarCallExecutionEngine call_exec_engine;
  exec_engine.exec_enter(bb);
  for (ar::Statement* stmt : *bb) {
    transfer_function(exec_engine, call_exec_engine, stmt);
  }
  exec_engine.exec_leave(bb);
  return std::move(exec_engine.inv());
}

AbstractDomain GlobalVarInitializerFixpoint::analyze_edge(ar::BasicBlock* src,
                                                          ar::BasicBlock* dest,
                                                          AbstractDomain pre) {
  NumericalExecutionEngineT exec_engine(std::move(pre),
                                        _ctx,
                                        this->_empty_call_context,
                                        ExecutionEngine::UpdateAllocSizeVar,
                                        /* liveness = */ _ctx.liveness,
                                        /* pointer_info = */ _ctx.pointer ==
                                                nullptr
                                            ? nullptr
                                            : &_ctx.pointer->results());
  exec_engine.exec_edge(src, dest);
  return std::move(exec_engine.inv());
}

void GlobalVarInitializerFixpoint::process_pre(ar::BasicBlock* /*bb*/,
                                               const AbstractDomain& /*pre*/) {}

void GlobalVarInitializerFixpoint::process_post(
    ar::BasicBlock* /*bb*/, const AbstractDomain& /*post*/) {}

const AbstractDomain& GlobalVarInitializerFixpoint::exit_invariant() const {
  ar::Code* code = this->cfg();
  ikos_assert_msg(code->has_exit_block(), "initializer without exit block");
  return this->post(code->exit_block());
}

} // end namespace sequential
} // end namespace interprocedural
} // end namespace value
} // end namespace analyzer
} // end namespace ikos
