/*******************************************************************************
 *
 * Division-by-zero checker.
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

#ifndef ANALYZER_DIVISION_BY_ZERO_CHECKER_HPP
#define ANALYZER_DIVISION_BY_ZERO_CHECKER_HPP

#include <analyzer/analysis/common.hpp>
#include <analyzer/analysis/num_sym_exec.hpp>
#include <analyzer/checkers/checker_api.hpp>

namespace analyzer {

using namespace arbos;

template < class AbsDomain >
class division_by_zero_checker : public checker< AbsDomain > {
private:
  typedef checker< AbsDomain > checker_t;
  typedef number_t integer_t;
  typedef ikos::interval< integer_t > int_interval_t;

public:
  division_by_zero_checker(context& ctx,
                           results_table_t& results_table,
                           display_settings display_invariants,
                           display_settings display_checks)
      : checker_t(ctx, results_table, display_invariants, display_checks) {}

  virtual const char* name() { return "dbz"; }
  virtual const char* description() { return "Division by zero checker"; }

private:
  inline bool display_check(analysis_result result,
                            Arith_Op_ref stmt,
                            const location& loc) {
    if (checker_t::display_check(result, loc)) {
      std::cerr << " check_dbz(" << stmt << ")";
      return true;
    }
    return false;
  }

public:
  virtual void check(Arith_Op_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    if ((ar::getArithOp(stmt) == udiv) || (ar::getArithOp(stmt) == sdiv) ||
        (ar::getArithOp(stmt) == urem) || (ar::getArithOp(stmt) == srem)) {
      location loc = ar::getSrcLoc(stmt);
      boost::optional< Function_ref > function = ar::getParentFunction(stmt);
      analysis_result result = WARNING;
      json_dict info;

      if (ikos::exc_domain_traits::is_normal_flow_bottom(inv)) {
        result = UNREACHABLE;

        if (display_check(result, stmt, loc)) {
          std::cerr << std::endl;
        }
      } else {
        LiteralFactory& lfac = this->_context.lit_factory();
        scalar_lit_t lit = lfac[ar::getRightOp(stmt)];

        int_interval_t divisor = int_interval_t::bottom();
        if (lit.is_integer()) {
          divisor = int_interval_t(lit.integer());
          info.put("type", "constant");
        } else if (lit.is_integer_var()) {
          divisor = ikos::num_domain_traits::to_interval(inv, lit.var());
          info.put("type", "var");
          info.put("name", lit.var()->name());
        } else {
          throw analyzer_error(
              "division_by_zero_checker: unexpected operand to Arith_Op");
        }

        json_dict_put(info, "min", divisor.lb());
        json_dict_put(info, "max", divisor.ub());
        boost::optional< integer_t > d = divisor.singleton();

        if (d && (*d == 0)) {
          // the second operand is definitely 0
          result = ERROR;

          if (display_check(result, stmt, loc)) {
            std::cerr << ": ∀d ∈ divisor, d == 0" << std::endl;
          }
        } else if (divisor[0]) {
          // the second operand may be 0
          result = WARNING;

          if (display_check(result, stmt, loc)) {
            std::cerr << ": ∃d ∈ divisor, d == 0" << std::endl;
          }
        } else {
          // the second operand cannot be definitely 0
          result = OK;

          if (display_check(result, stmt, loc)) {
            std::cerr << ": ∀d ∈ divisor, d != 0" << std::endl;
          }
        }
      }

      this->display_invariant(result, inv, loc);
      this->_results.write("dbz",
                           result,
                           call_context,
                           function,
                           loc,
                           ar::getUID(stmt),
                           info);
    }
  }

}; // end class division_by_zero_checker

} // end namespace analyzer

#endif // ANALYZER_DIVISION_BY_ZERO_CHECKER_HPP
