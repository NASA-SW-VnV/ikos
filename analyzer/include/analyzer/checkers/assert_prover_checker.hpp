/*******************************************************************************
 *
 * Assertion prover checker.
 *
 * Author: Maxime Arthaud
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

#ifndef ANALYZER_ASSERT_PROVER_CHECKER_HPP
#define ANALYZER_ASSERT_PROVER_CHECKER_HPP

#include <analyzer/analysis/common.hpp>
#include <analyzer/analysis/num_sym_exec.hpp>
#include <analyzer/checkers/checker_api.hpp>
#include <analyzer/utils/demangle.hpp>

namespace analyzer {

using namespace arbos;

template < class AbsDomain >
class assert_prover_checker : public checker< AbsDomain > {
private:
  typedef checker< AbsDomain > checker_t;
  typedef typename analysis_db::db_ptr db_ptr_t;

public:
  assert_prover_checker(context& ctx,
                        db_ptr_t db,
                        display_settings display_invariants,
                        display_settings display_checks)
      : checker_t(ctx, db, display_invariants, display_checks) {}

  virtual const char* name() { return "prover"; }
  virtual const char* description() { return "Assertion prover checker"; }

  virtual void check(Call_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    if (ar::isDirectCall(stmt) &&
        (ar::getFunctionName(stmt) == "__ikos_assert" ||
         demangle(ar::getFunctionName(stmt)) == "__ikos_assert(int)" ||
         demangle(ar::getFunctionName(stmt)) == "__ikos_assert(bool)")) {
      LiteralFactory& lfac = this->_context.lit_factory();
      location loc = ar::getSrcLoc(stmt);
      OpRange arguments = ar::getArguments(stmt);

      if (arguments.size() != 1)
        return;

      Literal cond = lfac[arguments[0]];

      if (exc_domain_traits::is_normal_flow_bottom(inv)) {
        if (this->display_check(UNREACHABLE)) {
          std::cout << location_to_string(loc)
                    << ": [unreachable] __ikos_assert(" << arguments[0] << ")"
                    << std::endl;
        }
        if (this->display_invariant(UNREACHABLE)) {
          std::cout << location_to_string(loc) << ": Invariant:" << std::endl
                    << inv << std::endl;
        }

        this->_db->write("prover",
                         call_context,
                         loc.file,
                         loc.line,
                         loc.column,
                         ar::getUID(stmt),
                         "unreachable");
      } else {
        z_interval flag = z_interval::bottom();
        if (cond.is_var()) {
          flag = num_domain_traits::to_interval(inv, cond.get_var());
        } else if (cond.is_num()) {
          flag = z_interval(cond.get_num< ikos::z_number >());
        } else {
          assert(false && "unreachable");
        }
        boost::optional< ikos::z_number > v = flag.singleton();
        analysis_result result;

        if (v && *v == 0) {
          // the condition is definitely 0
          if (this->display_check(ERR)) {
            std::cout << location_to_string(loc) << ": [error] __ikos_assert("
                      << arguments[0] << "): ∀x ∈ " << arguments[0]
                      << ", x == 0" << std::endl;
          }

          result = ERR;
        } else if (flag[0]) {
          // the condition may be 0
          if (this->display_check(WARNING)) {
            std::cout << location_to_string(loc) << ": [warning] __ikos_assert("
                      << arguments[0] << "): (∃x ∈ " << arguments[0]
                      << ", x == 0) and (∃x ∈ " << arguments[0] << ", x != 0)"
                      << std::endl;
          }

          result = WARNING;
        } else {
          // the condition cannot be 0
          if (this->display_check(OK)) {
            std::cout << location_to_string(loc) << ": [ok] __ikos_assert("
                      << arguments[0] << "): ∀x ∈ " << arguments[0]
                      << ", x != 0" << std::endl;
          }

          result = OK;
        }

        if (this->display_invariant(result)) {
          std::cout << location_to_string(loc) << ": Invariant:" << std::endl
                    << inv << std::endl;
        }

        this->_db->write("prover",
                         call_context,
                         loc.file,
                         loc.line,
                         loc.column,
                         ar::getUID(stmt),
                         tostr(result));
      }
    }
  }

  virtual void check(Invoke_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    check(ar::getFunctionCall(stmt), inv, call_context);
  }
}; // end class assert_prover_checker

} // end namespace analyzer

#endif // ANALYZER_ASSERT_PROVER_CHECKER_HPP
