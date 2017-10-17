/*******************************************************************************
 *
 * Implementations of sym_exec_warning_callback
 *
 * Authors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2016-2017 United States Government as represented by the
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

#ifndef ANALYZER_SYM_EXEC_WARNING_CALLBACKS_HPP
#define ANALYZER_SYM_EXEC_WARNING_CALLBACKS_HPP

#include <analyzer/analysis/sym_exec_api.hpp>
#include <analyzer/ar-wrapper/cfg.hpp>
#include <analyzer/utils/analysis_db.hpp>

namespace analyzer {

// Warning callback that prints warnings on the standard error
class warning_console_writer : public sym_exec_warning_callback {
public:
  warning_console_writer() {}

  void warning(const std::string& kind,
               const std::string& message,
               Statement_ref stmt,
               const json_dict& info = json_dict()) {
    std::cerr << location_to_string(ar::getSrcLoc(stmt))
              << ": warning: " << message << std::endl;
  }

}; // end class warning_console

// Warning callback that stores warnings in the database
class warning_database_writer : public sym_exec_warning_callback {
private:
  results_table_t& _results;
  std::string _call_context;

public:
  warning_database_writer(results_table_t& results,
                          const std::string& call_context)
      : _results(results), _call_context(call_context) {}

  void warning(const std::string& kind,
               const std::string& /*message*/,
               Statement_ref stmt,
               const json_dict& info = json_dict()) {
    _results.write(kind,
                   boost::optional< analysis_result >(),
                   _call_context,
                   ar::getParentFunction(stmt),
                   ar::getSrcLoc(stmt),
                   ar::getUID(stmt),
                   info);
  }

}; // end class warning_database_writer

} // end namespace analyzer

#endif // ANALYZER_SYM_EXEC_WARNING_CALLBACKS_HPP
