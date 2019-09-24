/******************************************************************************
 *
 * \file
 * \brief Implementation of the function pointer analysis
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
#include <ikos/core/domain/memory/dummy.hpp>
#include <ikos/core/domain/scalar/dummy.hpp>

#include <ikos/analyzer/analysis/pointer/constraint.hpp>
#include <ikos/analyzer/analysis/pointer/function.hpp>
#include <ikos/analyzer/analysis/pointer/value.hpp>
#include <ikos/analyzer/util/demangle.hpp>
#include <ikos/analyzer/util/log.hpp>
#include <ikos/analyzer/util/progress.hpp>

namespace ikos {
namespace analyzer {

FunctionPointerAnalysis::FunctionPointerAnalysis(Context& ctx)
    : _ctx(ctx), _info(ctx.bundle->data_layout()) {}

FunctionPointerAnalysis::~FunctionPointerAnalysis() = default;

namespace {

/// \brief Dummy scalar abstract domain
using ScalarAbstractDomain =
    core::scalar::DummyDomain< Variable*, MemoryLocation* >;

/// \brief Dummy memory abstract domain
using MemoryAbstractDomain = core::memory::
    DummyDomain< Variable*, MemoryLocation*, ScalarAbstractDomain >;

/// \brief Dummy abstract domain for the function pointer analysis
///
/// This is either top or bottom.
using AbstractDomain = core::exception::ExceptionDomain< MemoryAbstractDomain >;

/// \brief Create the top abstract value
AbstractDomain make_top_abstract_value() {
  auto top = MemoryAbstractDomain(ScalarAbstractDomain::top());
  return AbstractDomain(/*normal = */ top,
                        /*caught_exceptions = */ top,
                        /*propagated_exceptions = */ top);
}

/// \brief An empty code invariants
class EmptyCodeInvariants {
public:
  using AbstractDomainT = AbstractDomain;

public:
  /// \brief Get the invariant at the entry of the given basic block
  AbstractDomainT entry(ar::BasicBlock* /*bb*/) const {
    return make_top_abstract_value();
  }

  /// \brief Analyze the given statement and update the invariant
  AbstractDomainT analyze_statement(ar::Statement* /*stmt*/,
                                    const AbstractDomainT& /*inv*/) const {
    return make_top_abstract_value();
  }
};

} // end anonymous namespace

void FunctionPointerAnalysis::run() {
  ar::Bundle* bundle = _ctx.bundle;

  // Setup a progress logger
  std::unique_ptr< ProgressLogger > progress =
      make_progress_logger(_ctx.opts.progress,
                           LogLevel::Info,
                           /* num_tasks = */
                           std::count_if(bundle->global_begin(),
                                         bundle->global_end(),
                                         [](ar::GlobalVariable* gv) {
                                           return gv->is_definition();
                                         }) +
                               std::count_if(bundle->function_begin(),
                                             bundle->function_end(),
                                             [](ar::Function* fun) {
                                               return fun->is_definition();
                                             }) +
                               1);
  ScopeLogger scope(*progress);

  log::debug("Generating pointer constraints");
  PointerConstraints constraints(bundle->data_layout());
  PointerConstraintsGenerator< EmptyCodeInvariants > visitor(_ctx,
                                                             constraints,
                                                             nullptr);

  for (auto it = bundle->global_begin(), et = bundle->global_end(); it != et;
       ++it) {
    ar::GlobalVariable* gv = *it;
    if (gv->is_definition()) {
      progress->start_task(
          "Generating pointer constraints for initializer of global variable "
          "'" +
          demangle(gv->name()) + "'");
      visitor.process_global_var_def(gv, EmptyCodeInvariants());
    } else {
      visitor.process_global_var_decl(gv);
    }
  }

  for (auto it = bundle->function_begin(), et = bundle->function_end();
       it != et;
       ++it) {
    ar::Function* fun = *it;
    if (fun->is_definition()) {
      progress->start_task("Generating pointer constraints for function '" +
                           demangle(fun->name()) + "'");
      visitor.process_function_def(fun, EmptyCodeInvariants());
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

void FunctionPointerAnalysis::dump(std::ostream& o) const {
  o << "Function pointer analysis results:\n";
  this->_info.dump(o);
}

} // end namespace analyzer
} // end namespace ikos
