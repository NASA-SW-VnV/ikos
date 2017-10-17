/*******************************************************************************
 *
 * Intra-procedural copy propagation for copy assignments involving
 * only scalar variables.
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

#include <ikos/common/types.hpp>
#include <ikos/domains/cpa.hpp>
#include <ikos/number.hpp>

#include <analyzer/ar-wrapper/cfg.hpp>
#include <analyzer/ikos-wrapper/iterators.hpp>

namespace analyzer {

using namespace arbos;
using namespace ikos;

template < typename Number, typename VariableName >
class cpa_visitor : public arbos_visitor_api {
  typedef cpa_domain< VariableName, Number > cpa_domain_t;
  typedef typename cpa_domain_t::substitution_t substitution_t;
  typedef typename cpa_domain_t::expression_t expression_t;

  cpa_domain_t _inv;
  VariableFactory& _vfac;
  LiteralFactory& _lfac;

  boost::optional< expression_t > operator[](const Operand_ref& o) {
    scalar_lit_t lit = _lfac[o];

    if (lit.is_integer()) {
      return boost::optional< expression_t >(
          expression_t::number(lit.integer()));
    } else if (lit.is_var()) {
      return boost::optional< expression_t >(expression_t::variable(lit.var()));
    } else if (lit.is_null()) {
      return boost::optional< expression_t >(expression_t::number(Number(0)));
    } else if (lit.is_floating_point()) {
      // Literal class do not keep track of floating points so we
      // need to dig it up here using the ar namespace
      assert(ar::ar_internal::is_cst_operand(o));
      Cst_Operand_ref cst_o = node_cast< Cst_Operand >(o);
      Constant_ref cst = ar::getConstant(cst_o);
      FP_Constant_ref n = node_cast< FP_Constant >(cst);
      return boost::optional< expression_t >(
          expression_t::number(ar::getFPValueStr(n)));
    } else {
      return boost::optional< expression_t >();
    }
  }

public:
  cpa_visitor(cpa_domain_t inv, VariableFactory& vfac, LiteralFactory& lfac)
      : _inv(inv), _vfac(vfac), _lfac(lfac) {}

  cpa_domain_t inv() { return this->_inv; }

  void visit(Arith_Op_ref stmt) {
    VariableName lhs = _vfac[ar::getName(ar::getResult(stmt))];
    cpa_domain_t kill = this->_inv[lhs];
    cpa_domain_t gen = cpa_domain_t::bottom();
    boost::optional< expression_t > l1 = this->operator[](ar::getLeftOp(stmt));
    boost::optional< expression_t > l2 = this->operator[](ar::getRightOp(stmt));
    if (l1 && l2) {
      switch (ar::getArithOp(stmt)) {
        case arbos::add:
          gen += substitution_t(lhs, (*l1) + (*l2));
          break;
        case arbos::sub:
          gen += substitution_t(lhs, (*l1) - (*l2));
          break;
        case arbos::mul:
          gen += substitution_t(lhs, (*l1) * (*l2));
          break;
        case arbos::sdiv:
        case arbos::udiv:
          gen += substitution_t(lhs, (*l1) / (*l2));
          break;
        default:;
          ;
      }
    }
    this->_inv.transfer_function(kill, gen);
  }

  void visit(FP_Op_ref stmt) {
    VariableName lhs = _vfac[ar::getName(ar::getResult(stmt))];
    cpa_domain_t kill = this->_inv[lhs];
    // TODO: GEN set
    cpa_domain_t gen = cpa_domain_t::bottom();

    this->_inv.transfer_function(kill, gen);
  }

  void visit(Bitwise_Op_ref stmt) {
    VariableName lhs = _vfac[ar::getName(ar::getResult(stmt))];
    cpa_domain_t kill = this->_inv[lhs];
    // TODO: GEN set
    cpa_domain_t gen = cpa_domain_t::bottom();

    this->_inv.transfer_function(kill, gen);
  }

  void visit(Load_ref stmt) {
    cpa_domain_t kill = this->_inv[_vfac[ar::getName(ar::getResult(stmt))]];
    cpa_domain_t gen = cpa_domain_t::bottom();
    this->_inv.transfer_function(kill, gen);
  }

  void visit(Abstract_Variable_ref stmt) {
    cpa_domain_t kill = this->_inv[_vfac[ar::getName(ar::getVar(stmt))]];
    cpa_domain_t gen = cpa_domain_t::bottom();
    this->_inv.transfer_function(kill, gen);
  }

  void visit(Unreachable_ref stmt) { this->_inv = cpa_domain_t::bottom(); }

  void visit(Assignment_ref stmt) {
    VariableName lhs = _vfac[ar::getName(ar::getLeftOp(stmt))];
    cpa_domain_t kill = this->_inv[lhs];
    cpa_domain_t gen = cpa_domain_t::bottom();
    if (boost::optional< expression_t > exp =
            this->operator[](ar::getRightOp(stmt)))
      gen += substitution_t(lhs, *exp);

    this->_inv.transfer_function(kill, gen);
  }

  void visit(Conv_Op_ref stmt) {
    boost::optional< expression_t > lhs = this->operator[](ar::getLeftOp(stmt));
    if (lhs) {
      typename expression_t::variable_set_t lhs_vars = (*lhs).variables();
      assert(lhs_vars.size() == 1);
      VariableName x = *(lhs_vars.begin());
      cpa_domain_t kill = this->_inv[x];
      cpa_domain_t gen = cpa_domain_t::bottom();
      if (boost::optional< expression_t > exp =
              this->operator[](ar::getRightOp(stmt)))
        gen += substitution_t(x, *exp);
      this->_inv.transfer_function(kill, gen);
    }
  }

  void visit(Call_ref stmt) {
    if (ar::getReturnValue(stmt)) {
      cpa_domain_t kill =
          this->_inv[_vfac[ar::getName(*(ar::getReturnValue(stmt)))]];
      cpa_domain_t gen = cpa_domain_t::bottom();
      this->_inv.transfer_function(kill, gen);
    }
  }

  void visit(Invoke_ref stmt) { visit(ar::getFunctionCall(stmt)); }

  // NOT IMPLEMENTED
  void visit(Basic_Block_ref) {}
  void visit(Integer_Comparison_ref) {}
  void visit(FP_Comparison_ref) {}
  void visit(Allocate_ref) {}
  void visit(Store_ref) {}
  void visit(Pointer_Shift_ref) {}
  void visit(Abstract_Memory_ref) {}
  void visit(MemCpy_ref) {}
  void visit(MemMove_ref) {}
  void visit(MemSet_ref) {}
  void visit(Landing_Pad_ref) {}
  void visit(Resume_ref) {}
  void visit(Return_Value_ref) {}
  void visit(Insert_Element_ref) {}
  void visit(Extract_Element_ref) {}
  void visit(VA_Start_ref) {}
  void visit(VA_End_ref) {}
  void visit(VA_Arg_ref) {}
  void visit(VA_Copy_ref) {}

}; // end class cpa_visitor

typedef ikos::q_number number_t;

class cpa_analyzer
    : public fwd_fixpoint_iterator< Basic_Block_ref,
                                    arbos_cfg,
                                    cpa_domain< varname_t, number_t > > {
  typedef cpa_domain< varname_t, number_t > cpa_domain_t;
  typedef fwd_fixpoint_iterator< Basic_Block_ref, arbos_cfg, cpa_domain_t >
      fwd_fixpoint_iterator_t;

  typedef cpa_visitor< number_t, varname_t > cpa_visitor_t;
  typedef std::shared_ptr< cpa_visitor_t > cpa_visitor_ptr;

  VariableFactory& _vfac;
  LiteralFactory& _lfac;

public:
  cpa_analyzer(arbos_cfg cfg, VariableFactory& vfac, LiteralFactory& lfac)
      : fwd_fixpoint_iterator_t(cfg, true), _vfac(vfac), _lfac(lfac) {}

  static cpa_domain_t init(arbos_cfg cfg) { return cpa_domain_t::bottom(); }

  cpa_domain_t analyze(Basic_Block_ref bb, cpa_domain_t pre) {
    arbos_cfg::arbos_node_t node = this->get_cfg().get_node(bb);
    cpa_visitor_ptr vis(new cpa_visitor_t(pre, _vfac, _lfac));
    node.accept(vis);
    return vis->inv();
  }

  // This is just an analysis so no check property
  void check_pre(Basic_Block_ref /*bb*/, cpa_domain_t /*inv*/) {}
  void check_post(Basic_Block_ref /*bb*/, cpa_domain_t /*inv*/) {}

}; // end cpa_analyzer class

class CPAPass : public Pass {
public:
  CPAPass() : Pass("cpa", "intra-procedural copy propagation analysis.") {}

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
        cpa_analyzer a(cfg, vfac, lfac);
        a.run(cpa_analyzer::init(cfg));
      } catch (analyzer_error& e) {
        std::cerr << "analyzer error: " << e << std::endl;
        exit(EXIT_FAILURE);
      } catch (ikos::exception& e) {
        std::cerr << "ikos error: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
      } catch (arbos::error& e) {
        std::cerr << "arbos error: " << e << std::endl;
        exit(EXIT_FAILURE);
      } catch (std::exception& e) {
        std::cerr << "system error: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
      } catch (...) {
        std::cerr << "unknown error occurred" << std::endl;
        exit(EXIT_FAILURE);
      }
    } // end for
  }

}; // end class CPAPass

} // end of analyzer namespace

// Here we declare the analysis as an ARBOS pass
extern "C" arbos::Pass* init() {
  return new analyzer::CPAPass();
}
