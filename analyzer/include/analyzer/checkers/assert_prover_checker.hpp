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
#include <analyzer/checkers/checker_api.hpp>
#include <analyzer/analysis/num_sym_exec.hpp>

namespace analyzer {

using namespace arbos;

template < class AbsDomain >
class assert_prover_checker : public checker< AbsDomain > {
private:
  typedef checker< AbsDomain > checker_t;
  typedef typename analysis_db::db_ptr db_ptr_t;

public:
  assert_prover_checker(context& ctx, db_ptr_t db) : checker_t(ctx, db) {}

  virtual const char* name() { return "prover"; }
  virtual const char* description() { return "Assertion prover checker"; }

  virtual void check(Call_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    LiteralFactory& lfac = this->_context.lit_factory();

    if (ar::isDirectCall(stmt) &&
        boost::starts_with(ar::getFunctionName(stmt), "__ikos_assert")) {
      location loc = ar::getSrcLoc(stmt);

#ifdef DISPLAY_INVARIANTS
      std::cout << "Invariant at " << location_to_string(loc) << std::endl
                << inv << std::endl;
#endif

      if (inv.is_bottom()) {
        this->_db->write("prover",
                         call_context,
                         loc.first,
                         loc.second,
                         "unreachable");
#ifdef DISPLAY_CHECKS
        std::cout << "unreachable"
                  << "|" << loc.first << "|" << loc.second << std::endl;
#endif
      } else {
        OpRange args = ar::getArguments(stmt);
        if (args.size() == 1) {
          Literal AssrtCond = lfac[*(args.begin())];
          z_interval flag = z_interval::bottom();
          if (AssrtCond.is_var()) {
            flag =
                num_abstract_domain_impl::to_interval(inv, AssrtCond.get_var());
          } else if (AssrtCond.is_num()) {
            flag = z_interval(AssrtCond.get_num< ikos::z_number >());
          } else {
            throw "unreachable";
          }
          boost::optional< ikos::z_number > v = flag.singleton();
          if (v) {
            if (*v == 0) {
              this->_db->write("prover",
                               call_context,
                               loc.first,
                               loc.second,
                               "error");
#ifdef DISPLAY_CHECKS
              std::cout << "unsafe"
                        << "|" << loc.first << "|" << loc.second << std::endl;
#endif
            } else {
              this->_db->write("prover",
                               call_context,
                               loc.first,
                               loc.second,
                               "ok");
#ifdef DISPLAY_CHECKS
              std::cout << "safe"
                        << "|" << loc.first << "|" << loc.second << std::endl;
#endif
            }
          } else {
            this->_db->write("prover",
                             call_context,
                             loc.first,
                             loc.second,
                             "warning");
#ifdef DISPLAY_CHECKS
            std::cout << "warning"
                      << "|" << loc.first << "|" << loc.second << std::endl;
#endif
          }
        }
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
