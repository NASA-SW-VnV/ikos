/*******************************************************************************
 *
 * \file
 * \brief Sequential intraprocedural value analysis implementation
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
#include <ikos/analyzer/analysis/value/intraprocedural/sequential/analysis.hpp>
#include <ikos/analyzer/analysis/value/intraprocedural/sequential/function_fixpoint.hpp>
#include <ikos/analyzer/checker/checker.hpp>
#include <ikos/analyzer/util/demangle.hpp>
#include <ikos/analyzer/util/log.hpp>
#include <ikos/analyzer/util/progress.hpp>
#include <ikos/analyzer/util/timer.hpp>

namespace ikos {
namespace analyzer {
namespace value {
namespace intraprocedural {
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

    if (!checkers.empty()) {
      progress->start_task("Checking properties for function '" +
                           demangle(function->name()) + "'");
      ScopeTimerDatabase t(_ctx.output_db->times,
                           "ikos-analyzer.check." + function->name());
      fixpoint.run_checks(checkers);
    }
  }
}

} // end namespace sequential
} // end namespace intraprocedural
} // end namespace value
} // end namespace analyzer
} // end namespace ikos
