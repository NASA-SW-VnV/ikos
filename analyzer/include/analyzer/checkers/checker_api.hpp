/*******************************************************************************
 *
 * Generic API for property checking on ARBOS statements.
 *
 * Author: Maxime Arthaud
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

#ifndef ANALYZER_CHECKER_API_HPP
#define ANALYZER_CHECKER_API_HPP

#include <string>

#include <analyzer/analysis/context.hpp>
#include <analyzer/ar-wrapper/wrapper.hpp>
#include <analyzer/utils/analysis_db.hpp>

namespace analyzer {

using namespace arbos;

template < class AbsDomain >
class checker {
protected:
  //! global analysis state
  context& _context;

  //! output analysis results to an external database
  results_table_t& _results;

  //! Display invariants (ALL, FAIL, OFF)
  display_settings _display_invariants;

  //! Display checks (ALL, FAIL, OFF)
  display_settings _display_checks;

protected:
  checker(context& ctx,
          results_table_t& results_table,
          display_settings display_invariants,
          display_settings display_checks)
      : _context(ctx),
        _results(results_table),
        _display_invariants(display_invariants),
        _display_checks(display_checks) {}

  inline bool display_invariant(analysis_result result) {
    return _display_invariants == display_settings::ALL ||
           (_display_invariants == display_settings::FAIL &&
            (result == ERROR || result == WARNING || result == UNREACHABLE));
  }

  inline bool display_invariant(analysis_result result,
                                AbsDomain inv,
                                const location& loc) {
    if (display_invariant(result)) {
      std::cerr << location_to_string(loc) << ": Invariant:" << std::endl
                << inv << std::endl;
      return true;
    }
    return false;
  }

  inline bool display_check(analysis_result result) {
    return _display_checks == display_settings::ALL ||
           (_display_checks == display_settings::FAIL &&
            (result == ERROR || result == WARNING || result == UNREACHABLE));
  }

  inline bool display_check(analysis_result result, const location& loc) {
    if (display_check(result)) {
      std::cerr << location_to_string(loc) << ": [" << result_to_str(result)
                << "]";
      return true;
    }
    return false;
  }

public:
  virtual ~checker() {}

  virtual const char* name() = 0;
  virtual const char* description() = 0;

  virtual void check_start(Basic_Block_ref b,
                           AbsDomain inv,
                           const std::string& call_context) {}
  virtual void check_end(Basic_Block_ref b,
                         AbsDomain inv,
                         const std::string& call_context) {}
  virtual void check(Arith_Op_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Integer_Comparison_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(FP_Op_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(FP_Comparison_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Assignment_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Conv_Op_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Bitwise_Op_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Allocate_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Store_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Load_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Insert_Element_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Extract_Element_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Pointer_Shift_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Abstract_Variable_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Abstract_Memory_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(MemCpy_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(MemMove_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(MemSet_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Call_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Return_Value_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Invoke_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Landing_Pad_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Resume_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(Unreachable_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(VA_Start_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(VA_End_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(VA_Arg_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}
  virtual void check(VA_Copy_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {}

}; // end class checker

} // end namespace analyzer

#endif // ANALYZER_CHECKER_API_HPP
