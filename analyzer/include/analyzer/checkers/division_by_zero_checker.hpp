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
  typedef typename analysis_db::db_ptr db_ptr_t;

public:
  division_by_zero_checker(context& ctx,
                           db_ptr_t db,
                           display_settings display_invariants,
                           display_settings display_checks)
      : checker_t(ctx, db, display_invariants, display_checks) {}

  virtual const char* name() { return "dbz"; }
  virtual const char* description() { return "Division by zero checker"; }

  virtual void check(Arith_Op_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    LiteralFactory& lfac = this->_context.lit_factory();

    if ((ar::getArithOp(stmt) == udiv) || (ar::getArithOp(stmt) == sdiv) ||
        (ar::getArithOp(stmt) == urem) || (ar::getArithOp(stmt) == srem)) {
      location loc = ar::getSrcLoc(stmt);

      if (exc_domain_traits::is_normal_flow_bottom(inv)) {
        if (this->display_check(UNREACHABLE)) {
          std::cout << location_to_string(loc) << ": [unreachable] " << stmt
                    << std::endl;
        }
        if (this->display_invariant(UNREACHABLE)) {
          std::cout << location_to_string(loc) << ": Invariant:" << std::endl
                    << inv << std::endl;
        }

        this->_db->write("dbz",
                         call_context,
                         loc.file,
                         loc.line,
                         loc.column,
                         ar::getUID(stmt),
                         "unreachable");
      } else {
        Literal lit = lfac[ar::getRightOp(stmt)];
        assert(lit.is_var() || lit.is_num());

        z_interval divisor = z_interval::bottom();
        if (lit.is_num()) {
          divisor = z_interval(lit.get_num< ikos::z_number >());
        } else {
          divisor = num_domain_traits::to_interval(inv, lit.get_var());
        }

        boost::optional< ikos::z_number > d = divisor.singleton();
        analysis_result result;

        if (d && (*d == 0)) {
          // the second operand is definitely 0
          if (this->display_check(ERR)) {
            std::cout << location_to_string(loc) << ": [error] " << stmt
                      << ": ∀d ∈ divisor, d == 0" << std::endl;
          }

          result = ERR;
        } else if (divisor[0]) {
          // the second operand may be 0
          if (this->display_check(WARNING)) {
            std::cout << location_to_string(loc) << ": [warning] " << stmt
                      << ": ∃d ∈ divisor, d == 0" << std::endl;
          }

          result = WARNING;
        } else {
          // the second operand cannot be definitely 0
          if (this->display_check(OK)) {
            std::cout << location_to_string(loc) << ": [ok] " << stmt
                      << ": ∀d ∈ divisor, d != 0" << std::endl;
          }

          result = OK;
        }

        if (this->display_invariant(result)) {
          std::cout << location_to_string(loc) << ": Invariant:" << std::endl
                    << inv << std::endl;
        }

        this->_db->write("dbz",
                         call_context,
                         loc.file,
                         loc.line,
                         loc.column,
                         ar::getUID(stmt),
                         tostr(result));
      }
    }
  }
}; // end class division_by_zero_checker

} // end namespace analyzer

#endif // ANALYZER_DIVISION_BY_ZERO_CHECKER_HPP
