/*******************************************************************************
 *
 * Class to keep global state across functions.
 *
 * Author: Jorge A. Navas
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2017 United States Government as represented by the
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

#ifndef ANALYZER_CONTEXT_HPP
#define ANALYZER_CONTEXT_HPP

#include <boost/noncopyable.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/analysis/liveness.hpp>
#include <analyzer/ar-wrapper/wrapper.hpp>
#include <analyzer/utils/analysis_db.hpp>

namespace analyzer {

using namespace arbos;

/// Class containing all the global state of the analyses.
class context : public boost::noncopyable {
private:
  //! the program
  Bundle_ref _bundle;

  //! liveness info
  LivenessPass& _liveness;

  //! external pointer information
  PointerInfo _pointer;

  //! CFG factory
  CfgFactory& _cfgfac;

  //! database table containing all analysis results
  results_table_t& _results_table;

public:
  context(Bundle_ref bundle,
          LivenessPass& liveness,
          PointerInfo pointer,
          CfgFactory& cfgfac,
          results_table_t& results_table)
      : _bundle(bundle),
        _liveness(liveness),
        _pointer(pointer),
        _cfgfac(cfgfac),
        _results_table(results_table) {}

  Bundle_ref bundle() const { return _bundle; }
  VariableFactory& var_factory() const { return _cfgfac.var_factory(); }
  LiteralFactory& lit_factory() const { return _cfgfac.lit_factory(); }
  memory_factory& mem_factory() const { return _cfgfac.mem_factory(); }
  LivenessPass& liveness() const { return _liveness; }
  PointerInfo pointer_info() const { return _pointer; }
  void set_pointer_info(PointerInfo pointer) { _pointer = pointer; }
  const std::string& arch() const { return _cfgfac.arch(); }
  TrackedPrecision prec_level() const { return _cfgfac.prec_level(); }
  results_table_t& results_table() { return _results_table; }

  boost::optional< Function_ref > operator[](
      const std::string& fun_name) const {
    return ar::getFunction(_bundle, fun_name);
  }

  arbos_cfg operator[](Function_ref f) const { return _cfgfac[f]; }
};

} // namespace analyzer

#endif // ANALYZER_CONTEXT_HPP
