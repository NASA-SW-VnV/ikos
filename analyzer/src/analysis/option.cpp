/*******************************************************************************
 *
 * \file
 * \brief Implementation for Analyses options
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

#include <boost/iterator/transform_iterator.hpp>

#include <ikos/analyzer/analysis/option.hpp>
#include <ikos/analyzer/analysis/value/machine_int_domain.hpp>
#include <ikos/analyzer/database/output.hpp>

namespace ikos {
namespace analyzer {

void AnalysisOptions::save(SettingsTable& table) {
  auto function_name = [](ar::Function* fun) { return fun->name(); };

  table.insert("analyses",
               to_json(boost::make_transform_iterator(this->analyses.begin(),
                                                      checker_short_name),
                       boost::make_transform_iterator(this->analyses.end(),
                                                      checker_short_name)));
  table
      .insert("entry-points",
              to_json(boost::make_transform_iterator(this->entry_points.begin(),
                                                     function_name),
                      boost::make_transform_iterator(this->entry_points.end(),
                                                     function_name)));

  table.insert("no-init-globals",
               to_json(boost::make_transform_iterator(this->no_init_globals
                                                          .begin(),
                                                      function_name),
                       boost::make_transform_iterator(this->no_init_globals
                                                          .end(),
                                                      function_name)));

  table.insert("machine-int-domain",
               machine_int_domain_option_str(this->machine_int_domain));

  table.insert("procedural", procedural_str(this->procedural));

  table.insert("num-threads", this->num_threads);

  table.insert("widening-strategy",
               widening_strategy_str(this->widening_strategy));

  table.insert("narrowing-strategy",
               narrowing_strategy_str(this->narrowing_strategy));

  table.insert("widening-delay", std::to_string(this->widening_delay));

  JsonDict widening_delay_dict;
  for (const auto& p : this->widening_delay_functions) {
    widening_delay_dict.put(p.first->name(), p.second);
  }
  table.insert("widening-delay-functions", widening_delay_dict);

  table.insert("widening-period", std::to_string(this->widening_period));

  if (this->narrowing_iterations) {
    table.insert("narrowing-iterations",
                 std::to_string(*this->narrowing_iterations));
  }

  table.insert("use-liveness", this->use_liveness);

  table.insert("use-pointer-analysis", this->use_pointer);

  table.insert("use-widening-hints", this->use_widening_hints);

  table.insert("use-partitioning-domain", this->use_partitioning_domain);

  table.insert("use-fixpoint-cache", this->use_fixpoint_cache);

  table.insert("use-checks", this->use_checks);

  table.insert("trace-ar-statements", this->trace_ar_statements);

  table.insert("globals-init-policy",
               globals_init_policy_str(this->globals_init_policy));

  table.insert("hardware-addresses",
               hardware_addresses_str(this->hardware_addresses));

  if (this->argc) {
    table.insert("argc", std::to_string(*this->argc));
  }
}

} // end namespace analyzer
} // end namespace ikos
