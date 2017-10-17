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

#ifndef ANALYZER_ASSERT_PROVER_CHECKER_HPP
#define ANALYZER_ASSERT_PROVER_CHECKER_HPP

#include <boost/algorithm/string/predicate.hpp>

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
  typedef number_t integer_t;
  typedef ikos::interval< integer_t > int_interval_t;

public:
  assert_prover_checker(context& ctx,
                        results_table_t& results_table,
                        display_settings display_invariants,
                        display_settings display_checks)
      : checker_t(ctx, results_table, display_invariants, display_checks) {}

  virtual const char* name() { return "prover"; }
  virtual const char* description() { return "Assertion prover checker"; }

private:
  template < typename T >
  inline bool display_check(analysis_result result,
                            const T& op,
                            const location& loc) {
    if (checker_t::display_check(result, loc)) {
      std::cerr << " __ikos_assert(" << op << ")";
      return true;
    }
    return false;
  }

public:
  virtual void check(Call_ref stmt,
                     AbsDomain inv,
                     const std::string& call_context) {
    if (ar::isDirectCall(stmt)) {
      if ((ar::getFunctionName(stmt) == "__ikos_assert" ||
           demangle(ar::getFunctionName(stmt)) == "__ikos_assert(int)" ||
           demangle(ar::getFunctionName(stmt)) == "__ikos_assert(bool)")) {
        OpRange arguments = ar::getArguments(stmt);
        location loc = ar::getSrcLoc(stmt);
        boost::optional< Function_ref > function = ar::getParentFunction(stmt);
        analysis_result result = WARNING;
        json_dict info;

        if (arguments.size() != 1) {
          std::ostringstream buf;
          buf << location_to_string(loc)
              << ": __ikos_assert(): unexpected number of parameters";
          throw analyzer_error(buf.str());
        }

        if (ikos::exc_domain_traits::is_normal_flow_bottom(inv)) {
          result = UNREACHABLE;

          if (display_check(result, arguments[0], loc)) {
            std::cerr << std::endl;
          }
        } else {
          LiteralFactory& lfac = this->_context.lit_factory();
          scalar_lit_t cond = lfac[arguments[0]];

          int_interval_t flag = int_interval_t::bottom();
          if (cond.is_integer_var()) {
            flag = ikos::num_domain_traits::to_interval(inv, cond.var());
            info.put("type", "var");
            info.put("name", cond.var()->name());
          } else if (cond.is_integer()) {
            flag = int_interval_t(cond.integer());
            info.put("type", "constant");
          } else {
            throw analyzer_error(
                "assert_prover_checker: unexpected parameter to __ikos_assert");
          }
          boost::optional< integer_t > v = flag.singleton();

          if (v && *v == 0) {
            // the condition is definitely 0
            result = ERROR;

            if (display_check(result, cond, loc)) {
              std::cerr << ": ∀x ∈ " << cond << ", x == 0" << std::endl;
            }
          } else if (flag[0]) {
            // the condition may be 0
            result = WARNING;

            if (display_check(result, cond, loc)) {
              std::cerr << ": (∃x ∈ " << cond << ", x == 0) and (∃x ∈ " << cond
                        << ", x != 0)" << std::endl;
            }
          } else {
            // the condition cannot be 0
            result = OK;

            if (display_check(result, cond, loc)) {
              std::cerr << ": ∀x ∈ " << cond << ", x != 0" << std::endl;
            }
          }
        }

        this->display_invariant(result, inv, loc);
        this->_results.write("prover",
                             result,
                             call_context,
                             function,
                             loc,
                             ar::getUID(stmt),
                             info);
      } else if (ar::getFunctionName(stmt) == "__ikos_debug" ||
                 boost::starts_with(demangle(ar::getFunctionName(stmt)),
                                    "__ikos_debug")) {
        OpRange arguments = ar::getArguments(stmt);
        location loc = ar::getSrcLoc(stmt);

        std::cerr << location_to_string(loc) << ": __ikos_debug(";
        for (auto it = arguments.begin(); it != arguments.end();) {
          std::cerr << *it;
          ++it;
          if (it != arguments.end())
            std::cerr << ", ";
        }
        std::cerr << ")" << std::endl;

        if (arguments.empty() ||
            ikos::exc_domain_traits::is_normal_flow_bottom(inv)) {
          std::cerr << "Invariant: " << inv << std::endl;
        } else {
          LiteralFactory& lfac = this->_context.lit_factory();

          for (auto it = arguments.begin(); it != arguments.end(); ++it) {
            scalar_lit_t v = lfac[*it];

            if (v.is_integer_var()) {
              std::cerr << "\t" << v.var()->name() << " -> "
                        << ikos::num_domain_traits::to_interval(inv, v.var())
                        << std::endl;
            } else if (v.is_floating_point_var()) {
              // ignored for now
            } else if (v.is_pointer_var()) {
              // points-to
              if (ikos::ptr_domain_traits::is_unknown_addr(inv, v.var())) {
                std::cerr << "\t" << v.var()->name() << " -> {}" << std::endl;
              } else {
                ikos::discrete_domain< memloc_t > points_to =
                    ikos::ptr_domain_traits::addrs_set< AbsDomain,
                                                        memloc_t >(inv,
                                                                   v.var());
                std::cerr << "\t" << v.var()->name() << " -> " << points_to
                          << std::endl;
              }

              // offset
              varname_t offset_var =
                  ikos::ptr_domain_traits::offset_var(inv, v.var());
              std::cerr << "\t" << offset_var << " -> "
                        << ikos::num_domain_traits::to_interval(inv, offset_var)
                        << std::endl;

              // nullity
              if (ikos::null_domain_traits::is_null(inv, v.var())) {
                std::cerr << "\t" << v.var()->name() << " is null" << std::endl;
              } else if (ikos::null_domain_traits::is_non_null(inv, v.var())) {
                std::cerr << "\t" << v.var()->name() << " is non-null"
                          << std::endl;
              } else {
                std::cerr << "\t" << v.var()->name() << " may be null"
                          << std::endl;
              }
            } else {
              std::ostringstream buf;
              buf << location_to_string(loc) << ": __ikos_debug(): operand "
                  << v << " is not a variable";
              throw analyzer_error(buf.str());
            }

            // initialized (available for all variables)
            if (ikos::uninit_domain_traits::is_uninitialized(inv, v.var())) {
              std::cerr << "\t" << v.var()->name() << " is uninitialized"
                        << std::endl;
            } else if (ikos::uninit_domain_traits::is_initialized(inv,
                                                                  v.var())) {
              std::cerr << "\t" << v.var()->name() << " is initialized"
                        << std::endl;
            } else {
              std::cerr << "\t" << v.var()->name() << " may be uninitialized"
                        << std::endl;
            }
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
