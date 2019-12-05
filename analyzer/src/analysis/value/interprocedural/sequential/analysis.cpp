/*******************************************************************************
 *
 * \file
 * \brief Interprocedural value analysis implementation
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

#include <ikos/analyzer/analysis/value/abstract_domain.hpp>
#include <ikos/analyzer/analysis/value/global_variable.hpp>
#include <ikos/analyzer/analysis/value/interprocedural/init_invariant.hpp>
#include <ikos/analyzer/analysis/value/interprocedural/sequential/analysis.hpp>
#include <ikos/analyzer/analysis/value/interprocedural/sequential/function_fixpoint.hpp>
#include <ikos/analyzer/analysis/value/interprocedural/sequential/global_init_fixpoint.hpp>
#include <ikos/analyzer/analysis/value/interprocedural/sequential/progress.hpp>
#include <ikos/analyzer/checker/checker.hpp>
#include <ikos/analyzer/util/demangle.hpp>
#include <ikos/analyzer/util/log.hpp>
#include <ikos/analyzer/util/progress.hpp>
#include <ikos/analyzer/util/timer.hpp>

namespace ikos {
namespace analyzer {
namespace value {
namespace interprocedural {
namespace sequential {

Analysis::Analysis(Context& ctx) : _ctx(ctx) {}

Analysis::~Analysis() = default;

void Analysis::run() {
  // Bundle
  ar::Bundle* bundle = _ctx.bundle;

  // Create checkers
  std::vector< std::unique_ptr< Checker > > checkers;
  if (_ctx.opts.use_checks) {
    for (CheckerName name : _ctx.opts.analyses) {
      checkers.emplace_back(make_checker(_ctx, name));
    }
  }

  // Initial invariant
  AbstractDomain init_inv = make_initial_abstract_value(_ctx);

  // Initialize global variables
  {
    log::debug("Computing global variable static initialization");

    GlobalsInitPolicy policy = _ctx.opts.globals_init_policy;

    // Setup a progress logger
    std::unique_ptr< analyzer::ProgressLogger > logger =
        make_progress_logger(_ctx.opts.progress,
                             LogLevel::Debug,
                             /* num_tasks = */
                             std::count_if(bundle->global_begin(),
                                           bundle->global_end(),
                                           [=](ar::GlobalVariable* gv) {
                                             return gv->is_definition() &&
                                                    is_initialized(gv, policy);
                                           }));
    ScopeLogger scope(*logger);

    for (auto it = bundle->global_begin(), et = bundle->global_end(); it != et;
         ++it) {
      ar::GlobalVariable* gv = *it;
      if (gv->is_definition() && is_initialized(gv, policy)) {
        logger->start_task("Initializing global variable '" +
                           demangle(gv->name()) + "'");
        GlobalVarInitializerFixpoint fixpoint(_ctx, gv);
        fixpoint.run(init_inv);
        init_inv = fixpoint.exit_invariant();
      }
    }
  }

  if (_ctx.opts.display_invariants == DisplayOption::All) {
    LogMessage msg = log::msg();
    msg << "Invariant after global variable static initialization:\n";
    init_inv.dump(msg.stream());
    msg << "\n";
  }

  // Call global constructors
  ar::GlobalVariable* gv_ctors = bundle->global_or_null("ar.global_ctors");
  if (gv_ctors != nullptr) {
    log::info("Computing global variable dynamic initialization");

    std::vector< std::pair< ar::Function*, MachineInt > > ctors =
        global_ctors(gv_ctors);

    for (const auto& entry : ctors) {
      ar::Function* ctor = entry.first;

      if (ctor->is_declaration()) {
        log::error("global constructor '" + ctor->name() + "' is extern");
        continue;
      }

      // Setup a progress logger
      std::unique_ptr< sequential::ProgressLogger > logger =
          make_progress_logger(_ctx, _ctx.opts.progress, LogLevel::Info);
      ScopeLogger scope(*logger);

      // Create a function fixpoint
      FunctionFixpoint fixpoint(_ctx, checkers, *logger, ctor);

      {
        log::info("Analyzing global constructor '" + demangle(ctor->name()) +
                  "'");
        ScopeTimerDatabase t(_ctx.output_db->times,
                             "ikos-analyzer.value." + ctor->name());
        fixpoint.run(init_inv);
      }

      if (!checkers.empty()) {
        log::info("Checking properties for global constructor '" +
                  demangle(ctor->name()) + "'");
        ScopeTimerDatabase t(_ctx.output_db->times,
                             "ikos-analyzer.check." + ctor->name());
        fixpoint.run_checks();
      }

      init_inv = fixpoint.exit_invariant();
    }

    if (_ctx.opts.display_invariants == DisplayOption::All) {
      LogMessage msg = log::msg();
      msg << "Invariant after global variable dynamic initialization:\n";
      init_inv.dump(msg.stream());
      msg << "\n";
    }
  }

  // Analyze each entry point
  for (ar::Function* entry_point : _ctx.opts.entry_points) {
    if (!entry_point->is_definition()) {
      log::error("missing implementation of function '" + entry_point->name() +
                 "'");
      continue;
    }

    // Entry point initial invariant
    AbstractDomain entry_inv = make_bottom_abstract_value(_ctx);

    if (std::find(_ctx.opts.no_init_globals.begin(),
                  _ctx.opts.no_init_globals.end(),
                  entry_point) == _ctx.opts.no_init_globals.end()) {
      // Use invariant with initialized global variables
      entry_inv = init_inv;
    } else {
      // Default invariant
      entry_inv = make_initial_abstract_value(_ctx);
    }

    if (entry_point->name() == "main" && entry_point->num_parameters() >= 2) {
      entry_inv = init_main_invariant(_ctx, entry_point, entry_inv);
    }

    // Setup a progress logger
    std::unique_ptr< sequential::ProgressLogger > logger =
        make_progress_logger(_ctx, _ctx.opts.progress, LogLevel::Info);
    ScopeLogger scope(*logger);

    // Create a function fixpoint
    FunctionFixpoint fixpoint(_ctx, checkers, *logger, entry_point);

    {
      log::info("Analyzing entry point '" + demangle(entry_point->name()) +
                "'");
      ScopeTimerDatabase t(_ctx.output_db->times,
                           "ikos-analyzer.value." + entry_point->name());
      fixpoint.run(entry_inv);
    }

    if (!checkers.empty()) {
      log::info("Checking properties for entry point '" +
                demangle(entry_point->name()) + "'");
      ScopeTimerDatabase t(_ctx.output_db->times,
                           "ikos-analyzer.check." + entry_point->name());
      fixpoint.run_checks();
    }
  }

  // Call global destructors
  ar::GlobalVariable* gv_dtors = bundle->global_or_null("ar.global_dtors");
  if (gv_dtors != nullptr) {
    log::info("Analyzing global destructors");

    std::vector< std::pair< ar::Function*, MachineInt > > dtors =
        global_dtors(gv_dtors);

    for (const auto& entry : dtors) {
      ar::Function* dtor = entry.first;

      if (dtor->is_declaration()) {
        log::error("global destructor '" + dtor->name() + "' is extern");
        continue;
      }

      // Setup a progress logger
      std::unique_ptr< sequential::ProgressLogger > logger =
          make_progress_logger(_ctx, _ctx.opts.progress, LogLevel::Info);
      ScopeLogger scope(*logger);

      // Create a function fixpoint
      FunctionFixpoint fixpoint(_ctx, checkers, *logger, dtor);

      {
        log::info("Analyzing global destructor '" + demangle(dtor->name()) +
                  "'");
        ScopeTimerDatabase t(_ctx.output_db->times,
                             "ikos-analyzer.value." + dtor->name());
        // Note: We currently analyze destructors with the initial invariant
        fixpoint.run(init_inv);
      }

      if (!checkers.empty()) {
        log::info("Checking properties for global destructor: '" +
                  demangle(dtor->name()) + "'");
        ScopeTimerDatabase t(_ctx.output_db->times,
                             "ikos-analyzer.check." + dtor->name());
        fixpoint.run_checks();
      }

      init_inv = fixpoint.exit_invariant();
    }
  }

  // Insert all functions in the database
  for (auto it = bundle->function_begin(), et = bundle->function_end();
       it != et;
       ++it) {
    _ctx.output_db->functions.insert(*it);
  }
}

} // end namespace sequential
} // end namespace interprocedural
} // end namespace value
} // end namespace analyzer
} // end namespace ikos
