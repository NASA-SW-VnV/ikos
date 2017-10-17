/*******************************************************************************
 *
 * Intra-procedural reaching definitions analysis
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

#include <iostream>

#include <ikos/domains/reaching.hpp>

#include <analyzer/ar-wrapper/cfg.hpp>
#include <analyzer/ar-wrapper/literal.hpp>
#include <analyzer/ikos-wrapper/iterators.hpp>

namespace analyzer {

using namespace arbos;
using namespace ikos;

// TODO: pretty-printer
// reaching_domaint_t is a set of "definitions" as defined below. This
// doesn't help much to the user. Instead we should print the
// statement associated to that definition.
class reaching_analyzer
    : public fwd_fixpoint_iterator< Basic_Block_ref,
                                    arbos_cfg,
                                    reaching_domain< varname_t > > {
  typedef reaching_domain< varname_t > reaching_domain_t;
  typedef fwd_fixpoint_iterator< Basic_Block_ref, arbos_cfg, reaching_domain_t >
      fwd_fixpoint_iterator_t;
  typedef std::unordered_map< uint64_t, reaching_domain_t > definitions_t;

  VariableFactory& _vfac;
  //! _DEFS[y] is the set of definitions that assign to variable y
  definitions_t _DEFS;

  //! helper: generate a "definition" from the statement id a
  // "definition" is a unique label attached to a statement
  inline varname_t mk_definition(unsigned long n) {
    std::ostringstream buf;
    buf << n;
    return _vfac["d_" + buf.str()];
  }

public:
  reaching_analyzer(arbos_cfg cfg, VariableFactory& vfac)
      : fwd_fixpoint_iterator_t(cfg, true), _vfac(vfac) {
    typedef arbos_cfg::iterator iterator;

    // Compute _DEFS[y] for each variable y. This is used for the KILL
    // function.
    std::pair< iterator, iterator > nodes = cfg.get_nodes();
    for (iterator bt = nodes.first; bt != nodes.second; ++bt) {
      arbos_cfg::arbos_node_t node = *bt;
      for (arbos_cfg::arbos_node_t::iterator st = node.begin();
           st != node.end();
           ++st) {
        Statement_ref stmt = *st;
        varname_set_t def_vars = bt->defs(stmt);
        unsigned long stmt_uid = ar::getUID(stmt);
        if (!def_vars.empty()) {
          for (varname_set_t::iterator it = def_vars.begin(),
                                       et = def_vars.end();
               it != et;
               ++it) {
            uint64_t index = (*it).index();
            definitions_t::iterator di = this->_DEFS.find(index);
            if (di != this->_DEFS.end()) {
              reaching_domain_t s = di->second;
              s += reaching_domain_t(mk_definition(stmt_uid));
              this->_DEFS[index] = s;
            } else {
              reaching_domain_t s(mk_definition(stmt_uid));
              this->_DEFS.insert(std::make_pair(index, s));
            }
          }
        }
      }
    }
  }

  static reaching_domain_t init(arbos_cfg cfg) {
    return reaching_domain_t::bottom();
  }

  reaching_domain_t analyze(Basic_Block_ref bb, reaching_domain_t pre) {
    reaching_domain_t inv(pre);
    arbos_cfg::arbos_node_t node = this->get_cfg().get_node(bb);
    for (arbos_cfg::arbos_node_t::iterator stmt = node.begin();
         stmt != node.end();
         ++stmt) {
      varname_set_t def_vars = node.defs(*stmt);
      if (!def_vars.empty()) {
        // kill is the set of definitions killed by the statement (i.e.,
        // other definitions of the same variable) in the rest of the CFG
        for (varname_set_t::iterator it = def_vars.begin(), et = def_vars.end();
             it != et;
             ++it)
          inv -= this->_DEFS[(*it).index()];
        // gen is the set of definitions generated by the statement
        inv += mk_definition(ar::getUID(*stmt));
      }
    } // end for
    return inv;
  }

  // This is just an analysis so no check phase
  void check_pre(Basic_Block_ref /*bb*/, reaching_domain_t /*pre*/) {}
  void check_post(Basic_Block_ref /*bb*/, reaching_domain_t /*post*/) {}

}; // end reaching_analyzer class

class ReachingPass : public Pass {
public:
  ReachingPass()
      : Pass("reaching", "intra-procedural reaching definition analysis.") {}

  void execute(Bundle_ref bundle) {
    VariableFactory vfac;
    LiteralFactory lfac(vfac);
    const TrackedPrecision prec_level = REG;

    CfgFactory cfg_fac(vfac, lfac, prec_level);
    FuncRange entries = ar::getFunctions(bundle);
    for (FuncRange::iterator it = entries.begin(), et = entries.end(); it != et;
         ++it) {
      try {
        arbos_cfg cfg = cfg_fac[*it];
        reaching_analyzer a(cfg, vfac);
        a.run(reaching_analyzer::init(cfg));
      } catch (analyzer_error& e) {
        std::cerr << "analyzer error: " << e << std::endl;
        exit(EXIT_FAILURE);
      } catch (ikos::exception& e) {
        std::cerr << "ikos error: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
      } catch (arbos::error& e) {
        std::cerr << "arbos error: " << e << std::endl;
        exit(EXIT_FAILURE);
      } catch (...) {
        std::cerr << "unknown error occurred" << std::endl;
        exit(EXIT_FAILURE);
      }
    }
  }
}; // end class ReachingPass

} // end of ikos namespace

// Here we declare the analysis as an ARBOS pass
extern "C" arbos::Pass* init() {
  return new analyzer::ReachingPass();
}
