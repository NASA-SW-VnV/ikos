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
 * Copyright (c) 2011-2016 United States Government as represented by the
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

#include <analyzer/analysis/common.hpp>
#include <analyzer/analysis/liveness.hpp>
#include <analyzer/ar-wrapper/wrapper.hpp>
#include <analyzer/utils/sqlite_api.hpp>

namespace analyzer {

using namespace arbos;

/// Class containing all the global state of the analyses.
class context : public boost::noncopyable {
public:
  typedef sqlite::db_connection db_t;

private:
  //! output analysis results to an external database
  db_t _db;

  //! the program
  Bundle_ref _bundle;

  //! live info
  LivenessPass& _liveness;

  //! external pointer information
  PointerInfo _pointer;

  //! Cfg factory
  CfgFactory& _cfgfac;

public:
  context(db_t db,
          Bundle_ref bundle,
          LivenessPass& liveness,
          PointerInfo pointer,
          CfgFactory& cfgfac)
      : _db(db),
        _bundle(bundle),
        _liveness(liveness),
        _pointer(pointer),
        _cfgfac(cfgfac) {}

  Bundle_ref bundle() const { return _bundle; }
  VariableFactory& var_factory() const { return _cfgfac.getVarFactory(); }
  LiteralFactory& lit_factory() const { return _cfgfac.getLitFactory(); }
  LivenessPass& liveness() const { return _liveness; }
  PointerInfo pointer_info() const { return _pointer; }
  void set_pointer_info(PointerInfo pointer) { _pointer = pointer; }
  TrackedPrecision prec_level() const { return _cfgfac.getPrecLevel(); }
  db_t db() { return _db; }

  boost::optional< Function_ref > operator[](
      const std::string& fun_name) const {
    return ar::getFunction(_bundle, fun_name);
  }

  arbos_cfg operator[](Function_ref f) const { return _cfgfac[f]; }
};

} // namespace analyzer

#endif // ANALYZER_CONTEXT_HPP
