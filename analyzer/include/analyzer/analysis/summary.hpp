/******************************************************************************
 *
 * Analyses using function summarization.
 *
 * Author: Maxime Arthaud
 *
 * Contributors: Clement Decoodt
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

#ifndef ANALYZER_SUMMARY_HPP
#define ANALYZER_SUMMARY_HPP

#include <ikos/domains/pta.hpp>

#include <analyzer/analysis/call_graph_topology.hpp>
#include <analyzer/analysis/context.hpp>
#include <analyzer/analysis/pointer.hpp>
#include <analyzer/checkers/checker_api.hpp>
#include <analyzer/utils/demangle.hpp>

//#define DEBUG

namespace analyzer {

/*
 * Manage a function call
 */
template < typename AbsNumDomain >
class SymExecCall {
public:
  virtual ~SymExecCall() {}
  virtual AbsNumDomain exec_call(Call_ref stmt, AbsNumDomain inv) = 0;
  virtual void exec_ret(Return_Value_ref stmt, AbsNumDomain inv) = 0;
};

/*
 * TransferFunctionVisitor visits all statements and updates the abstract
 * domain
 */
template < typename AbsDomain,
           typename VariableName,
           typename MemoryLocation,
           typename Number >
class TransferFunctionVisitor : public arbos_visitor_api {
private:
  typedef num_sym_exec< AbsDomain, Number, ikos::dummy_number > sym_exec_t;
  typedef ikos::patricia_tree_set< VariableName > varname_set_t;
  typedef SymExecCall< AbsDomain > sym_exec_call_t;
  typedef std::shared_ptr< sym_exec_call_t > sym_exec_call_ptr_t;

private:
  sym_exec_t _sym_exec;
  sym_exec_call_ptr_t _sym_exec_call;

public:
  TransferFunctionVisitor(
      AbsDomain pre,
      sym_exec_call_ptr_t sym_exec_call,
      VariableFactory& vfac,
      memory_factory& mfac,
      LiteralFactory& lfac,
      const std::string& arch,
      TrackedPrecision prec_level,
      PointerInfo pointer_info = PointerInfo(), // refine a value analysis
      varname_set_t dead_vars = varname_set_t())
      : _sym_exec(pre,
                  vfac,
                  mfac,
                  lfac,
                  arch,
                  prec_level,
                  pointer_info,
                  dead_vars),
        _sym_exec_call(sym_exec_call) {}

  AbsDomain inv() { return _sym_exec.inv(); }

  void visit_start(Basic_Block_ref b) { _sym_exec.exec_start(b); }
  void visit_end(Basic_Block_ref b) { _sym_exec.exec_end(b); }
  void visit(Arith_Op_ref s) { _sym_exec.exec(s); }
  void visit(Integer_Comparison_ref s) { _sym_exec.exec(s); }
  void visit(FP_Comparison_ref s) { _sym_exec.exec(s); }
  void visit(Bitwise_Op_ref s) { _sym_exec.exec(s); }
  void visit(FP_Op_ref s) { _sym_exec.exec(s); }
  void visit(Abstract_Variable_ref s) { _sym_exec.exec(s); }
  void visit(Unreachable_ref s) { _sym_exec.exec(s); }
  void visit(Assignment_ref s) { _sym_exec.exec(s); }
  void visit(Conv_Op_ref s) { _sym_exec.exec(s); }
  void visit(Pointer_Shift_ref s) { _sym_exec.exec(s); }
  void visit(Allocate_ref s) { _sym_exec.exec(s); }
  void visit(Store_ref s) { _sym_exec.exec(s); }
  void visit(Load_ref s) { _sym_exec.exec(s); }
  void visit(Insert_Element_ref s) { _sym_exec.exec(s); }
  void visit(Extract_Element_ref s) { _sym_exec.exec(s); }
  void visit(MemCpy_ref s) { _sym_exec.exec(s); }
  void visit(MemMove_ref s) { _sym_exec.exec(s); }
  void visit(MemSet_ref s) { _sym_exec.exec(s); }
  void visit(Landing_Pad_ref s) { _sym_exec.exec(s); }
  void visit(Resume_ref s) { _sym_exec.exec(s); }
  void visit(Abstract_Memory_ref s) { _sym_exec.exec(s); }
  void visit(Call_ref stmt) {
    if (ar::isDirectCall(stmt) && ar::isExternal(stmt)) {
      _sym_exec.exec_external_call(ar::getReturnValue(stmt),
                                   stmt,
                                   ar::getFunctionName(stmt),
                                   ar::getArguments(stmt));
    } else {
      _sym_exec.set_inv(_sym_exec_call->exec_call(stmt, _sym_exec.inv()));
    }
  }
  void visit(Invoke_ref stmt) { visit(ar::getFunctionCall(stmt)); }
  void visit(Return_Value_ref stmt) {
    _sym_exec.exec(stmt);
    _sym_exec_call->exec_ret(stmt, _sym_exec.inv());
  }
  void visit(VA_Start_ref s) { _sym_exec.exec(s); }
  void visit(VA_End_ref s) { _sym_exec.exec(s); }
  void visit(VA_Arg_ref s) { _sym_exec.exec(s); }
  void visit(VA_Copy_ref s) { _sym_exec.exec(s); }

}; // end class TransferFunctionVisitor

// Convert a std::vector< Internal_Variable_ref > to a
// patricia_tree_set< varname_t >
ikos::patricia_tree_set< varname_t > from_internal_vars(
    const std::vector< Internal_Variable_ref >& internal_vars,
    LiteralFactory& lfac) {
  ikos::patricia_tree_set< varname_t > vars;

  for (typename std::vector< Internal_Variable_ref >::const_iterator it =
           internal_vars.begin();
       it != internal_vars.end();
       ++it) {
    vars += lfac[*it].var();
  }

  return vars;
}

// Convert a std::vector< Local_Variable_ref > to a
// patricia_tree_set< varname_t >
ikos::patricia_tree_set< varname_t > from_local_vars(
    const std::vector< Local_Variable_ref >& local_vars, LiteralFactory& lfac) {
  ikos::patricia_tree_set< varname_t > vars;

  for (typename std::vector< Local_Variable_ref >::const_iterator it =
           local_vars.begin();
       it != local_vars.end();
       ++it) {
    vars += lfac[*it].var();
  }

  return vars;
}

/*
 * Numerical analysis using function summarization
 *
 * This pass is intended to be used as a pre-step for other analyses.
 * It computes inter-procedurally a numerical invariant for each function,
 * called a summary.
 */
template < typename AbsNumDomain,
           typename VariableName,
           typename MemoryLocation,
           typename Number >
class NumericalSummaryPass : public ikos::writeable, public boost::noncopyable {
public:
  /*
   * NumericalAnalysis is computing an analysis of a function with a relational
   * abstract domain.
   */
  class NumericalAnalysis : public ikos::writeable,
                            public fwd_fixpoint_iterator< Basic_Block_ref,
                                                          arbos_cfg,
                                                          AbsNumDomain > {
  private:
    typedef fwd_fixpoint_iterator< Basic_Block_ref, arbos_cfg, AbsNumDomain >
        fwd_fixpoint_iterator_t;
    typedef std::shared_ptr< NumericalAnalysis > numerical_analysis_ptr_t;
    typedef TransferFunctionVisitor< AbsNumDomain,
                                     VariableName,
                                     MemoryLocation,
                                     Number >
        transfer_function_visitor_t;
    typedef num_sym_exec< AbsNumDomain, Number, ikos::dummy_number > sym_exec_t;
    typedef SymExecCall< AbsNumDomain > sym_exec_call_t;
    typedef std::unordered_map< Basic_Block_ref, AbsNumDomain > inv_table_t;

  private:
    /*
     * Manage a function call
     *
     * In case there is a summary for the callee, just use it. Otherwise, we are
     * analysing a strongly connected component: we start analyzing the callee
     * if we are not already doing it. We don't assume anything on the result.
     */
    class NumericalSummaryExecCall : public sym_exec_call_t {
    private:
      NumericalAnalysis& _numerical_analysis;
      NumericalSummaryPass& _summary_pass;

    public:
      NumericalSummaryExecCall(NumericalAnalysis& numerical_analysis)
          : _numerical_analysis(numerical_analysis),
            _summary_pass(numerical_analysis._summary_pass) {}

      AbsNumDomain exec_call(Call_ref stmt, AbsNumDomain inv) {
        CfgFactory& cfg_fac = _summary_pass.cfg_factory();
        VariableFactory& vfac = _summary_pass.var_factory();
        summaries_t& summaries = _summary_pass._summaries;
        std::vector< Function_ref > callees;

        if (ar::isDirectCall(stmt)) {
          callees.push_back(ar::getFunction(stmt));
        } else if (ar::isIndirectCall(stmt)) {
          PointerInfo pointer_info = _summary_pass.pointer_info();
          Bundle_ref bundle = _numerical_analysis._bundle;
          VariableName ptr = vfac.get_internal(ar::getIndirectCallVar(stmt));
          ptr_set_t ptr_set = pointer_info[ptr].first;

          if (ptr_set.is_top()) {
            // No points-to information
            location loc = ar::getSrcLoc(stmt);
            std::cerr << location_to_string(loc)
                      << ": warning: indirect call cannot be resolved"
                      << std::endl;
            return exec_call_forget(stmt, inv);
          }

          for (ptr_set_t::iterator it = ptr_set.begin(); it != ptr_set.end();
               ++it) {
            // Check if the callee is a function
            if (!isa< function_memory_location >(*it)) {
              continue;
            }
            auto mem_loc_it = cast< analyzer::function_memory_location >(*it);
            boost::optional< Function_ref > fun =
                ar::getFunction(bundle, mem_loc_it->name());

            if (!fun) { // external call
              return exec_call_forget(stmt, inv);
            }

            callees.push_back(*fun);
          }
        } else {
          throw analyzer_error("unreachable");
        }

        // start the analysis of callee functions we haven't analyzed yet
        for (typename std::vector< Function_ref >::iterator it =
                 callees.begin();
             it != callees.end();
             ++it) {
          Function_ref callee = *it;

          if (summaries.find(callee) == summaries.end()) {
            std::cout << "** Analyzing function "
                      << demangle(ar::getName(callee))
                      << " (within a cycle of recursion)" << std::endl;
            arbos_cfg cfg = cfg_fac[callee];
            numerical_analysis_ptr_t callee_summary(
                new NumericalAnalysis(_summary_pass,
                                      _numerical_analysis._bundle,
                                      cfg));
            summaries.insert(
                typename summaries_t::value_type(callee, callee_summary));
            callee_summary->run(_summary_pass.default_call_context(callee));
          }
        }

        if (ikos::exc_domain_traits::is_normal_flow_bottom(inv) ||
            !ar::getReturnValue(stmt)) {
          return inv;
        } else {
          AbsNumDomain result = AbsNumDomain::bottom();

          for (typename std::vector< Function_ref >::iterator it =
                   callees.begin();
               it != callees.end();
               ++it) {
            Function_ref callee = *it;
            numerical_analysis_ptr_t callee_summary = summaries[callee];

            if (callee_summary->is_done()) {
#ifdef DEBUG
              std::cerr << "--- using " << demangle(ar::getName(callee))
                        << " summary" << std::endl;
#endif
              result =
                  result |
                  exec_call_with_summary(stmt, inv, callee, callee_summary);
            } else {
#ifdef DEBUG
              std::cerr << "--- analyzing a cycle, forgetting the result"
                        << std::endl;
#endif
              return exec_call_forget(stmt, inv);
            }
          }

          return result;
        }
      }

      void exec_ret(Return_Value_ref stmt, AbsNumDomain pre) {
        _numerical_analysis._return_inv = pre;
        _numerical_analysis._return_operand = ar::getReturnValue(stmt);
      }

    private:
      AbsNumDomain exec_call_with_summary(Call_ref stmt,
                                          AbsNumDomain inv,
                                          Function_ref callee,
                                          numerical_analysis_ptr_t summary) {
        if (ikos::exc_domain_traits::is_normal_flow_bottom(inv) ||
            !ar::getReturnValue(stmt)) {
          return inv;
        }

        CfgFactory& cfg_fac = _summary_pass.cfg_factory();
        VariableFactory& vfac = _summary_pass.var_factory();
        memory_factory& mfac = _summary_pass.mem_factory();
        LiteralFactory& lfac = _summary_pass.lit_factory();

        sym_exec_t sym_exec(inv, vfac, mfac, lfac, cfg_fac.arch(), PTR);
        AbsNumDomain callee_inv = summary->return_inv();

        // assign parameters
        OpRange actual_params = ar::getArguments(stmt);
        IvRange formal_params = ar::getFormalParams(callee);
        sym_exec.match_down(formal_params, actual_params, false);

        // merge
        sym_exec.set_inv(sym_exec.inv() & callee_inv);

        // assign return value
        sym_exec.match_up(actual_params,
                          ar::getReturnValue(stmt),
                          formal_params,
                          summary->return_operand(),
                          false);

        inv = sym_exec.inv();

        // forget local vars
        ikos::patricia_tree_set< VariableName > vars =
            from_local_vars(cfg_fac[callee].local_variables(), lfac);
        inv.forget(vars.begin(), vars.end());

        // forget internal vars
        vars = from_internal_vars(cfg_fac[callee].internal_variables(), lfac);
        inv.forget(vars.begin(), vars.end());

        // forget formal parameters
        vars = from_internal_vars(formal_params, lfac);
        inv.forget(vars.begin(), vars.end());

        return inv;
      }

      AbsNumDomain exec_call_forget(Call_ref stmt, AbsNumDomain inv) {
        if (ar::getReturnValue(stmt)) {
          VariableFactory& vfac = _summary_pass.var_factory();
          VariableName lhs = vfac.get_internal(*(ar::getReturnValue(stmt)));
          inv -= lhs;
        }

        return inv;
      }

    }; // end class SummaryExecCall

  private:
    NumericalSummaryPass& _summary_pass;
    Bundle_ref _bundle;

    // invariants
    inv_table_t _inv_table;
    AbsNumDomain _return_inv;
    boost::optional< Operand_ref > _return_operand;

    bool _done; // to avoid cycles

  public:
    NumericalAnalysis(NumericalSummaryPass& summary_pass,
                      Bundle_ref bundle,
                      arbos_cfg cfg)
        : fwd_fixpoint_iterator_t(cfg, true),
          _summary_pass(summary_pass),
          _bundle(bundle),
          _done(false) {}

    bool is_done() const { return _done; }

    const inv_table_t& invariant_table() const { return _inv_table; }

    const AbsNumDomain& operator[](Basic_Block_ref bb) const {
      assert(_inv_table.find(bb) != _inv_table.end());
      return _inv_table.at(bb);
    }

    const AbsNumDomain& return_inv() const { return _return_inv; }

    boost::optional< Operand_ref > return_operand() const {
      return _return_operand;
    }

    AbsNumDomain analyze(Basic_Block_ref bb, AbsNumDomain pre) {
      std::string fun_name = this->get_cfg().func_name();
      arbos_cfg::arbos_node_t node = this->get_cfg().get_node(bb);

      std::shared_ptr< sym_exec_call_t > sym_exec_call(
          new NumericalSummaryExecCall(*this));
      std::shared_ptr< transfer_function_visitor_t > visitor(
          new transfer_function_visitor_t(pre,
                                          sym_exec_call,
                                          _summary_pass.var_factory(),
                                          _summary_pass.mem_factory(),
                                          _summary_pass.lit_factory(),
                                          this->get_cfg().arch(),
                                          PTR));
      node.accept(visitor);
      return visitor->inv();
    }

    template < typename Statement >
    AbsNumDomain analyze_stmt(Statement stmt, AbsNumDomain pre) {
      std::shared_ptr< sym_exec_call_t > sym_exec_call(
          new NumericalSummaryExecCall(*this));
      std::shared_ptr< transfer_function_visitor_t > visitor(
          new transfer_function_visitor_t(pre,
                                          sym_exec_call,
                                          _summary_pass.var_factory(),
                                          _summary_pass.mem_factory(),
                                          _summary_pass.lit_factory(),
                                          this->get_cfg().arch(),
                                          PTR));
      visitor->visit(stmt);
      return visitor->inv();
    }

  private:
    void check_pre(Basic_Block_ref bb, AbsNumDomain pre) {
#ifdef DEBUG
      std::cerr << "\t" << ar::getName(bb) << ": " << pre << std::endl;
#endif
      _inv_table.insert(typename inv_table_t::value_type(bb, pre));
    }

    void check_post(Basic_Block_ref bb, AbsNumDomain post) {}

  public:
    void run(AbsNumDomain inv) {
      fwd_fixpoint_iterator_t::run(inv);
      _done = true;
    }

    void write(std::ostream& o) {
      for (typename inv_table_t::iterator it = _inv_table.begin();
           it != _inv_table.end();
           ++it) {
        o << "\t" << ar::getName(it->first) << ": " << it->second << std::endl;
      }
    }
  }; // end class NumericalAnalysis

private:
  typedef std::shared_ptr< NumericalAnalysis > numerical_analysis_ptr_t;
  typedef std::unordered_map< Function_ref, numerical_analysis_ptr_t >
      summaries_t;
  typedef PointerInfo::ptr_set_t ptr_set_t;

private:
  CfgFactory& _cfg_fac;
  VariableFactory& _vfac;
  memory_factory& _mfac;
  LiteralFactory& _lfac;
  LivenessPass& _live;
  PointerInfo _pointer_info; // to solve indirect calls
  summaries_t _summaries;

public:
  NumericalSummaryPass(CfgFactory& cfg_fac,
                       VariableFactory& vfac,
                       memory_factory& mfac,
                       LiteralFactory& lfac,
                       LivenessPass& live,
                       PointerInfo pointer_info)
      : _cfg_fac(cfg_fac),
        _vfac(vfac),
        _mfac(mfac),
        _lfac(lfac),
        _live(live),
        _pointer_info(pointer_info) {}

  inline CfgFactory& cfg_factory() { return _cfg_fac; }
  inline VariableFactory& var_factory() { return _vfac; }
  inline memory_factory& mem_factory() { return _mfac; }
  inline LiteralFactory& lit_factory() { return _lfac; }
  inline LivenessPass& live() { return _live; }
  inline PointerInfo pointer_info() { return _pointer_info; }

  inline AbsNumDomain default_call_context(Function_ref /*f*/) const {
    return AbsNumDomain::top();
  }

  void execute(Bundle_ref bundle,
               StrongComponentsGraph_ref /*strong_comp_graph*/,
               const std::vector< Function_ref >& topo_order) {
    // bottom-up analysis
    for (std::vector< Function_ref >::const_iterator it = topo_order.begin();
         it != topo_order.end();
         ++it) {
      Function_ref root_fun = *it;
      std::cout << "** Analyzing function " << demangle(ar::getName(root_fun))
                << std::endl;
      arbos_cfg cfg = _cfg_fac[root_fun];
      numerical_analysis_ptr_t summary(
          new NumericalAnalysis(*this, bundle, cfg));
      _summaries.insert(typename summaries_t::value_type(root_fun, summary));
      summary->run(default_call_context(root_fun));
    }
  }

  NumericalAnalysis& operator[](Function_ref fun) const {
    assert(_summaries.find(fun) != _summaries.end());
    return *_summaries.at(fun);
  }

  void write(std::ostream& o) {
    for (typename summaries_t::iterator it = _summaries.begin();
         it != _summaries.end();
         ++it) {
      boost::optional< Operand_ref > return_op = it->second->return_operand();
      AbsNumDomain return_inv = it->second->return_inv();

      o << it->first << ": (";
      if (return_op) {
        o << *return_op;
      } else {
        o << "void";
      }
      o << ", ";
      return_inv.write(o);
      o << ")" << std::endl;
    }
  }

}; // end class NumericalSummaryPass

/*
 * Pointer analysis using function summarization
 *
 * It propagates invariants with a top-down analysis using pre-computed
 * summaries. Then, it generates nonuniform points-to constraints in a
 * flow-insensitive manner, solves them, and finally stores the results for
 * subsequent analyses.
 */
template < typename AbsNumDomain,
           typename SumAbsNumDomain,
           typename VariableName,
           typename MemoryLocation,
           typename Number >
class PointerSummaryPass : public ikos::writeable, public boost::noncopyable {
private:
  typedef NumericalSummaryPass< SumAbsNumDomain,
                                VariableName,
                                MemoryLocation,
                                Number >
      numerical_summary_pass_t;
  typedef typename numerical_summary_pass_t::NumericalAnalysis summary_t;

private:
  class NumericalAnalysis : public ikos::writeable,
                            public fwd_fixpoint_iterator< Basic_Block_ref,
                                                          arbos_cfg,
                                                          AbsNumDomain > {
  private:
    typedef fwd_fixpoint_iterator< Basic_Block_ref, arbos_cfg, AbsNumDomain >
        fwd_fixpoint_iterator_t;
    typedef std::shared_ptr< NumericalAnalysis > numerical_analysis_ptr_t;
    typedef TransferFunctionVisitor< AbsNumDomain,
                                     VariableName,
                                     MemoryLocation,
                                     Number >
        transfer_function_visitor_t;
    typedef SymExecCall< AbsNumDomain > sym_exec_call_t;
    typedef std::unordered_map< Basic_Block_ref, AbsNumDomain > inv_table_t;

  private:
    class PropagateExecCall : public sym_exec_call_t {
    private:
      NumericalAnalysis& _numerical_analysis;
      PointerSummaryPass& _pointer_pass;

    public:
      PropagateExecCall(NumericalAnalysis& numerical_analysis)
          : _numerical_analysis(numerical_analysis),
            _pointer_pass(numerical_analysis._pointer_pass) {}

      AbsNumDomain exec_call(Call_ref stmt, AbsNumDomain inv) {
        VariableFactory& vfac = _pointer_pass.var_factory();
        numerical_summary_pass_t& summary_pass = _pointer_pass.summary_pass();
        std::vector< Function_ref > callees;

        if (ar::isDirectCall(stmt)) {
          callees.push_back(ar::getFunction(stmt));
        } else if (ar::isIndirectCall(stmt)) {
          PointerInfo fun_ptr_info = _pointer_pass.fun_ptr_info();
          Bundle_ref bundle = _numerical_analysis._bundle;
          VariableName ptr = vfac.get_internal(ar::getIndirectCallVar(stmt));
          ptr_set_t ptr_set = fun_ptr_info[ptr].first;

          if (ptr_set.is_top()) {
            // No points-to information
            location loc = ar::getSrcLoc(stmt);
            std::cerr << location_to_string(loc)
                      << ": warning: indirect call cannot be resolved"
                      << std::endl;
            return exec_call_forget(stmt, inv);
          }

          for (ptr_set_t::iterator it = ptr_set.begin(); it != ptr_set.end();
               ++it) {
            // Check if the callee is a function
            if (!isa< function_memory_location >(*it)) {
              continue;
            }
            auto mem_loc_it = cast< analyzer::function_memory_location >(*it);
            boost::optional< Function_ref > fun =
                ar::getFunction(bundle, mem_loc_it->name());

            if (!fun) { // external call
              return exec_call_forget(stmt, inv);
            }

            callees.push_back(*fun);
          }
        } else {
          throw analyzer_error("unreachable");
        }

        // add calling contexts
        for (typename std::vector< Function_ref >::iterator it =
                 callees.begin();
             it != callees.end();
             ++it) {
          Function_ref callee = *it;
          numerical_analysis_ptr_t callee_num_analysis =
              _pointer_pass._invariants[callee];
          callee_num_analysis->add_call_context(
              exec_match_parameters(stmt, inv, callee));
        }

        if (ikos::exc_domain_traits::is_normal_flow_bottom(inv) ||
            !ar::getReturnValue(stmt)) {
          return inv;
        } else {
          AbsNumDomain result = AbsNumDomain::bottom();

          for (typename std::vector< Function_ref >::iterator it =
                   callees.begin();
               it != callees.end();
               ++it) {
            Function_ref callee = *it;
            const summary_t& callee_summary = summary_pass[callee];
            result = result |
                     exec_call_with_summary(stmt, inv, callee, callee_summary);
          }

          return result;
        }
      }

      void exec_ret(Return_Value_ref stmt, AbsNumDomain pre) {}

    private:
      AbsNumDomain exec_match_parameters(Call_ref stmt,
                                         AbsNumDomain inv,
                                         Function_ref callee) {
        if (ikos::exc_domain_traits::is_normal_flow_bottom(inv)) {
          return inv;
        }

        CfgFactory& cfg_fac = _pointer_pass.cfg_factory();
        LiteralFactory& lfac = _pointer_pass.lit_factory();
        Function_ref caller = _numerical_analysis.get_cfg().function();

        num_sym_exec< AbsNumDomain, Number, ikos::dummy_number >
            sym_exec(inv,
                     _pointer_pass.var_factory(),
                     _pointer_pass.mem_factory(),
                     _pointer_pass.lit_factory(),
                     cfg_fac.arch(),
                     PTR);

        // assign parameters
        sym_exec.match_down(ar::getFormalParams(callee),
                            ar::getArguments(stmt),
                            false);
        inv = sym_exec.inv();

        // forget local vars
        ikos::patricia_tree_set< VariableName > vars =
            from_local_vars(cfg_fac[caller].local_variables(), lfac);
        inv.forget(vars.begin(), vars.end());

        // forget internal vars
        vars = from_internal_vars(cfg_fac[caller].internal_variables(), lfac);
        inv.forget(vars.begin(), vars.end());

        return inv;
      }

      AbsNumDomain exec_call_with_summary(Call_ref stmt,
                                          AbsNumDomain caller_inv,
                                          Function_ref callee,
                                          const summary_t& summary) {
        if (ikos::exc_domain_traits::is_normal_flow_bottom(caller_inv) ||
            !ar::getReturnValue(stmt)) {
          return caller_inv;
        }

        // convert the AbsNumDomain to a SumAbsNumDomain
        SumAbsNumDomain inv =
            ikos::num_domain_traits::convert< AbsNumDomain, SumAbsNumDomain >(
                caller_inv);

        CfgFactory& cfg_fac = _pointer_pass.cfg_factory();
        VariableFactory& vfac = _pointer_pass.var_factory();
        memory_factory& mfac = _pointer_pass.mem_factory();
        LiteralFactory& lfac = _pointer_pass.lit_factory();

        num_sym_exec< SumAbsNumDomain, Number, ikos::dummy_number >
            sym_exec(inv, vfac, mfac, lfac, cfg_fac.arch(), PTR);
        SumAbsNumDomain callee_inv = summary.return_inv();

        // assign parameters
        OpRange actual_params = ar::getArguments(stmt);
        IvRange formal_params = ar::getFormalParams(callee);
        sym_exec.match_down(formal_params, actual_params, false);

        // merge
        sym_exec.set_inv(sym_exec.inv() & callee_inv);

        // assign return value
        sym_exec.match_up(actual_params,
                          ar::getReturnValue(stmt),
                          formal_params,
                          summary.return_operand(),
                          false);
        inv = sym_exec.inv();

        // forget local vars
        ikos::patricia_tree_set< VariableName > vars =
            from_internal_vars(cfg_fac[callee].internal_variables(), lfac);
        inv.forget(vars.begin(), vars.end());

        // forget internal vars
        vars = from_local_vars(cfg_fac[callee].local_variables(), lfac);
        inv.forget(vars.begin(), vars.end());

        // forget formal parameters
        vars = from_internal_vars(formal_params, lfac);
        inv.forget(vars.begin(), vars.end());

        return ikos::num_domain_traits::convert< SumAbsNumDomain,
                                                 AbsNumDomain >(inv);
      }

      AbsNumDomain exec_call_forget(Call_ref stmt, AbsNumDomain inv) {
        if (ar::getReturnValue(stmt)) {
          VariableFactory& vfac = _pointer_pass.var_factory();
          VariableName lhs = vfac.get_internal(*(ar::getReturnValue(stmt)));
          inv -= lhs;
        }

        return inv;
      }

    }; // end class PropagateExecCall

  private:
    PointerSummaryPass& _pointer_pass;
    Bundle_ref _bundle;

    // invariants
    AbsNumDomain _call_context;
    inv_table_t _inv_table;

  public:
    NumericalAnalysis(PointerSummaryPass& pointer_pass,
                      Bundle_ref bundle,
                      arbos_cfg cfg)
        : fwd_fixpoint_iterator_t(cfg, true),
          _pointer_pass(pointer_pass),
          _bundle(bundle),
          _call_context(AbsNumDomain::top()) {}

    void reset_call_context() { _call_context = AbsNumDomain::top(); }

    void add_call_context(const AbsNumDomain& inv) {
      if (_call_context.is_top()) {
        _call_context = inv;
      } else {
        _call_context = _call_context | inv;
      }

#ifdef DEBUG
      std::cerr << "call context for " << demangle(this->get_cfg().func_name())
                << ": " << _call_context << std::endl;
#endif
    }

    AbsNumDomain analyze(Basic_Block_ref bb, AbsNumDomain pre) {
      std::string fun_name = this->get_cfg().func_name();
      arbos_cfg::arbos_node_t node = this->get_cfg().get_node(bb);

      std::shared_ptr< sym_exec_call_t > sym_exec_call(
          new PropagateExecCall(*this));
      std::shared_ptr< transfer_function_visitor_t > visitor(
          new transfer_function_visitor_t(pre,
                                          sym_exec_call,
                                          _pointer_pass.var_factory(),
                                          _pointer_pass.mem_factory(),
                                          _pointer_pass.lit_factory(),
                                          this->get_cfg().arch(),
                                          PTR));
      node.accept(visitor);
      return visitor->inv();
    }

    template < typename Statement >
    AbsNumDomain analyze_stmt(Statement stmt, AbsNumDomain pre) {
      std::shared_ptr< sym_exec_call_t > sym_exec_call(
          new PropagateExecCall(*this));
      std::shared_ptr< transfer_function_visitor_t > visitor(
          new transfer_function_visitor_t(pre,
                                          sym_exec_call,
                                          _pointer_pass.var_factory(),
                                          _pointer_pass.mem_factory(),
                                          _pointer_pass.lit_factory(),
                                          this->get_cfg().arch(),
                                          PTR));
      visitor->visit(stmt);
      return visitor->inv();
    }

    AbsNumDomain operator[](Basic_Block_ref bb) const {
      typename inv_table_t::const_iterator it = _inv_table.find(bb);
      if (it != _inv_table.end()) {
        return it->second;
      } else {
        return AbsNumDomain::top();
      }
    }

  private:
    void check_pre(Basic_Block_ref bb, AbsNumDomain pre) {
#ifdef DEBUG
      std::cerr << "\t" << ar::getName(bb) << ": " << pre << std::endl;
#endif
      _inv_table.insert(typename inv_table_t::value_type(bb, pre));
    }

    void check_post(Basic_Block_ref bb, AbsNumDomain post) {}

  public:
    void run() { fwd_fixpoint_iterator_t::run(_call_context); }

    void write(std::ostream& o) {
      for (typename inv_table_t::iterator it = _inv_table.begin();
           it != _inv_table.end();
           ++it) {
        o << "\t" << ar::getName(it->first) << ": " << it->second << std::endl;
      }
    }
  }; // end class NumericalAnalysis

private:
  typedef std::shared_ptr< NumericalAnalysis > numerical_analysis_ptr_t;
  typedef std::unordered_map< Function_ref, numerical_analysis_ptr_t >
      invariants_t;
  typedef PointerPass::PTA< AbsNumDomain, NumericalAnalysis > pta_t;
  typedef typename pta_t::pt_var_map_t pt_var_map_t;
  typedef typename pta_t::address_map_t address_map_t;
  typedef PointerInfo::ptr_set_t ptr_set_t;
  typedef ikos::interval< Number > interval_t;

private:
  CfgFactory& _cfg_fac;
  VariableFactory& _vfac;
  memory_factory& _mfac;
  LiteralFactory& _lfac;
  LivenessPass& _live;
  PointerInfo _fun_ptr_info; // to solve indirect calls
  numerical_summary_pass_t& _summary_pass;

  invariants_t _invariants;
  PointerInfo _pointer_info;

public:
  PointerSummaryPass(CfgFactory& cfg_fac,
                     VariableFactory& vfac,
                     memory_factory& mfac,
                     LiteralFactory& lfac,
                     LivenessPass& live,
                     PointerInfo fun_ptr_info,
                     numerical_summary_pass_t& summary_pass)
      : _cfg_fac(cfg_fac),
        _vfac(vfac),
        _mfac(mfac),
        _lfac(lfac),
        _live(live),
        _fun_ptr_info(fun_ptr_info),
        _summary_pass(summary_pass) {}

  inline CfgFactory& cfg_factory() { return _cfg_fac; }
  inline VariableFactory& var_factory() { return _vfac; }
  inline memory_factory& mem_factory() { return _mfac; }
  inline LiteralFactory& lit_factory() { return _lfac; }
  inline LivenessPass& live() { return _live; }
  inline PointerInfo fun_ptr_info() { return _fun_ptr_info; }
  inline numerical_summary_pass_t& summary_pass() { return _summary_pass; }
  inline PointerInfo pointer_info() { return _pointer_info; }

public:
  void execute(Bundle_ref bundle,
               StrongComponentsGraph_ref strong_comp_graph,
               const std::vector< Function_ref >& topo_order) {
    // Propagate invariants (top-down)
    std::cout << "** Propagating invariants (top-down) ... " << std::endl;

    // initialize all numerical analysis
    FuncRange all_functions = ar::getFunctions(bundle);
    for (FuncRange::iterator it = all_functions.begin();
         it != all_functions.end();
         ++it) {
      Function_ref fun = *it;
      arbos_cfg cfg = _cfg_fac[fun];
      numerical_analysis_ptr_t num_analysis(
          new NumericalAnalysis(*this, bundle, cfg));
      _invariants.insert(typename invariants_t::value_type(fun, num_analysis));
    }

    // top-down analysis
    for (std::vector< Function_ref >::const_reverse_iterator root_it =
             topo_order.rbegin();
         root_it != topo_order.rend();
         ++root_it) {
      std::vector< Function_ref > functions =
          strong_comp_graph->component(*root_it);

      for (std::vector< Function_ref >::const_iterator it = functions.begin();
           it != functions.end();
           ++it) {
        Function_ref fun = *it;
        std::cout << "*** Analyzing function " << demangle(ar::getName(fun))
                  << std::endl;
        numerical_analysis_ptr_t num_analysis = _invariants[fun];

        if (functions.size() > 1) {
          // to be sound on a cycle, we just assume the input domain is TOP
          num_analysis->reset_call_context();
        }

        num_analysis->run();
      }
    }

    // Generating pointer constraints
    std::cout << "** Generating pointer constraints ... " << std::endl;
    ikos::pta_system csts_system;
    pt_var_map_t pt_var_map;
    address_map_t address_map;

    for (FuncRange::iterator it = all_functions.begin();
         it != all_functions.end();
         ++it) {
      Function_ref fun = *it;
      std::shared_ptr< pta_t > visitor(new pta_t(bundle,
                                                 fun,
                                                 csts_system,
                                                 _invariants[fun],
                                                 _lfac,
                                                 _vfac,
                                                 _mfac,
                                                 pt_var_map,
                                                 address_map,
                                                 _fun_ptr_info));
      ar::accept(fun, visitor);
    }

    std::cout << "** Solving pointer constraints ... " << std::endl;
    csts_system.solve();
    save_pointer_info(csts_system, pt_var_map, address_map);
  }

private:
  void save_pointer_info(
      const ikos::pta_system& csts_system, // solved constraints
      // mapping varname to pointer variables
      const pt_var_map_t& pt_var_map,
      // mapping id's to varname
      const address_map_t& address_map) {
    for (typename pt_var_map_t::const_iterator it = pt_var_map.begin();
         it != pt_var_map.end();
         ++it) {
      const VariableName& var_name = it->first;
      const ikos::pointer_var& pt_var = it->second;

      ikos::address_set points_to_addrs = csts_system.get(pt_var).first;
      interval_t offset = csts_system.get(pt_var).second;
      if (offset.is_bottom()) {
        offset = interval_t::top();
      }

      if (points_to_addrs.empty()) {
        _pointer_info
            .add(var_name,
                 std::make_pair(ikos::discrete_domain< MemoryLocation >::top(),
                                offset));
      } else {
        ikos::discrete_domain< MemoryLocation > points_to_vars =
            ikos::discrete_domain< MemoryLocation >::bottom();

        for (ikos::address_set::const_iterator a_it = points_to_addrs.begin();
             a_it != points_to_addrs.end();
             ++a_it) {
          typename address_map_t::const_iterator v_it = address_map.find(*a_it);
          assert(v_it != address_map.end());
          points_to_vars += v_it->second;
        }

        _pointer_info.add(var_name, std::make_pair(points_to_vars, offset));
      }
    }
  }

public:
  void write(std::ostream& o) { _pointer_info.write(o); }

}; // end class PointerSummaryPass

/*
 * Value analysis using function summarization
 *
 * This pass is intended to be used as a pre-step for other analyses.
 * It computes inter-procedurally a value analysis for each function, called a
 * summary.
 */
template < typename AbsValueDomain,
           typename VariableName,
           typename MemoryLocation,
           typename Number >
class ValueSummaryPass : public ikos::writeable, public boost::noncopyable {
public:
  class NumericalAnalysis : public ikos::writeable,
                            public fwd_fixpoint_iterator< Basic_Block_ref,
                                                          arbos_cfg,
                                                          AbsValueDomain > {
  private:
    typedef fwd_fixpoint_iterator< Basic_Block_ref, arbos_cfg, AbsValueDomain >
        fwd_fixpoint_iterator_t;
    typedef std::shared_ptr< NumericalAnalysis > numerical_analysis_ptr_t;
    typedef TransferFunctionVisitor< AbsValueDomain,
                                     VariableName,
                                     MemoryLocation,
                                     Number >
        transfer_function_visitor_t;
    typedef num_sym_exec< AbsValueDomain, Number, ikos::dummy_number >
        sym_exec_t;
    typedef SymExecCall< AbsValueDomain > sym_exec_call_t;
    typedef std::unordered_map< Basic_Block_ref, AbsValueDomain > inv_table_t;

  private:
    /*
     * Manage a function call
     */
    class ValueSummaryExecCall : public sym_exec_call_t {
    private:
      NumericalAnalysis& _numerical_analysis;
      ValueSummaryPass& _summary_pass;

    public:
      ValueSummaryExecCall(NumericalAnalysis& numerical_analysis)
          : _numerical_analysis(numerical_analysis),
            _summary_pass(numerical_analysis._summary_pass) {}

      AbsValueDomain exec_call(Call_ref stmt, AbsValueDomain inv) {
        CfgFactory& cfg_fac = _summary_pass.cfg_factory();
        VariableFactory& vfac = _summary_pass.var_factory();
        summaries_t& summaries = _summary_pass._summaries;
        std::vector< Function_ref > callees;

        if (ar::isDirectCall(stmt)) {
          callees.push_back(ar::getFunction(stmt));
        } else if (ar::isIndirectCall(stmt)) {
          PointerInfo pointer_info = _summary_pass.pointer_info();
          Bundle_ref bundle = _numerical_analysis._bundle;
          VariableName ptr = vfac.get_internal(ar::getIndirectCallVar(stmt));
          ptr_set_t ptr_set = pointer_info[ptr].first;

          if (ptr_set.is_top()) {
            // No points-to information
            location loc = ar::getSrcLoc(stmt);
            std::cerr << location_to_string(loc)
                      << ": warning: indirect call cannot be resolved "
                         "(analysis unsound if function has side effects)"
                      << std::endl;
            return exec_call_forget(stmt, inv);
          }

          for (ptr_set_t::iterator it = ptr_set.begin(); it != ptr_set.end();
               ++it) {
            // Check if the callee is a function
            if (!isa< function_memory_location >(*it)) {
              continue;
            }
            auto mem_loc_it = cast< analyzer::function_memory_location >(*it);
            boost::optional< Function_ref > fun =
                ar::getFunction(bundle, mem_loc_it->name());

            if (!fun) { // external call
              return exec_call_forget(stmt, inv);
            }

            callees.push_back(*fun);
          }
        } else {
          throw analyzer_error("unreachable");
        }

        // start the analysis of callee functions we haven't analyzed yet
        for (typename std::vector< Function_ref >::iterator it =
                 callees.begin();
             it != callees.end();
             ++it) {
          Function_ref callee = *it;

          if (summaries.find(callee) == summaries.end()) {
            std::cout << "** Analyzing function "
                      << demangle(ar::getName(callee))
                      << " (within a cycle of recursion)" << std::endl;
            arbos_cfg cfg = cfg_fac[callee];
            numerical_analysis_ptr_t callee_summary(
                new NumericalAnalysis(_summary_pass,
                                      _numerical_analysis._bundle,
                                      cfg));
            summaries.insert(
                typename summaries_t::value_type(callee, callee_summary));
            callee_summary->run(_summary_pass.default_call_context(callee));
          }
        }

        if (ikos::exc_domain_traits::is_normal_flow_bottom(inv)) {
          return inv;
        } else {
          AbsValueDomain result = AbsValueDomain::bottom();

          for (typename std::vector< Function_ref >::iterator it =
                   callees.begin();
               it != callees.end();
               ++it) {
            Function_ref callee = *it;
            numerical_analysis_ptr_t callee_summary = summaries[callee];

            if (callee_summary->is_done()) {
#ifdef DEBUG
              std::cerr << "--- using " << demangle(ar::getName(callee))
                        << " summary" << std::endl;
#endif
              result =
                  result |
                  exec_call_with_summary(stmt, inv, callee, callee_summary);
            } else {
              location loc = ar::getSrcLoc(stmt);
              std::cerr << location_to_string(loc)
                        << ": warning: found cycle of recursion (analysis "
                           "might be unsound)"
                        << std::endl;
#ifdef DEBUG
              std::cerr << "--- analyzing a cycle, forgetting the result"
                        << std::endl;
#endif
              return exec_call_forget(stmt, inv);
            }
          }

          return result;
        }
      }

      void exec_ret(Return_Value_ref stmt, AbsValueDomain pre) {
        _numerical_analysis._return_inv = pre;
        _numerical_analysis._return_operand = ar::getReturnValue(stmt);
      }

    private:
      AbsValueDomain exec_call_with_summary(Call_ref stmt,
                                            AbsValueDomain inv,
                                            Function_ref callee,
                                            numerical_analysis_ptr_t summary) {
        if (ikos::exc_domain_traits::is_normal_flow_bottom(inv)) {
          return inv;
        }

        CfgFactory& cfg_fac = _summary_pass.cfg_factory();
        VariableFactory& vfac = _summary_pass.var_factory();
        memory_factory& mfac = _summary_pass.mem_factory();
        LiteralFactory& lfac = _summary_pass.lit_factory();

        sym_exec_t sym_exec(inv,
                            vfac,
                            mfac,
                            lfac,
                            cfg_fac.arch(),
                            MEM,
                            _summary_pass.pointer_info());
        AbsValueDomain callee_inv = summary->return_inv();

        // assign parameters
        OpRange actual_params = ar::getArguments(stmt);
        IvRange formal_params = ar::getFormalParams(callee);
        sym_exec.match_down(formal_params, actual_params, false);

        // merge
        inv = sym_exec.inv();
        sym_exec.set_inv(inv.compose(callee_inv));

        // assign return value
        sym_exec.match_up(actual_params,
                          ar::getReturnValue(stmt),
                          formal_params,
                          summary->return_operand(),
                          false);

        inv = sym_exec.inv();

        // forget local vars
        ikos::patricia_tree_set< VariableName > vars =
            from_local_vars(cfg_fac[callee].local_variables(), lfac);
        inv.forget_mem_surface(vars.begin(), vars.end());

        // forget internal vars
        vars = from_internal_vars(cfg_fac[callee].internal_variables(), lfac);
        inv.forget_mem_surface(vars.begin(), vars.end());

        // forget formal parameters
        vars = from_internal_vars(formal_params, lfac);
        inv.forget_mem_surface(vars.begin(), vars.end());

        return inv;
      }

      // might be unsound because the memory could be updated
      AbsValueDomain exec_call_forget(Call_ref stmt, AbsValueDomain inv) {
        if (ar::getReturnValue(stmt)) {
          VariableFactory& vfac = _summary_pass.var_factory();
          VariableName lhs = vfac.get_internal(*(ar::getReturnValue(stmt)));
          inv -= lhs;
        }

        OpRange parameters = ar::getArguments(stmt);
        for (OpRange::iterator it = parameters.begin(); it != parameters.end();
             ++it) {
          if (ar::isPointer(*it)) {
            scalar_lit_t param = _summary_pass.lit_factory()[*it];
            if (param.is_pointer_var()) {
              inv -= param.var();
            }
          }
        }

        return inv;
      }

    }; // end class ValueSummaryExecCall

  private:
    ValueSummaryPass& _summary_pass;
    Bundle_ref _bundle;

    // invariants
    inv_table_t _inv_table;
    AbsValueDomain _return_inv;
    boost::optional< Operand_ref > _return_operand;

    bool _done; // to avoid cycles

  public:
    NumericalAnalysis(ValueSummaryPass& summary_pass,
                      Bundle_ref bundle,
                      arbos_cfg cfg)
        : fwd_fixpoint_iterator_t(cfg, true),
          _summary_pass(summary_pass),
          _bundle(bundle),
          _done(false) {}

    bool is_done() const { return _done; }

    const inv_table_t& invariant_table() const { return _inv_table; }

    const AbsValueDomain& operator[](Basic_Block_ref bb) const {
      assert(_inv_table.find(bb) != _inv_table.end());
      return _inv_table.at(bb);
    }

    const AbsValueDomain& return_inv() const { return _return_inv; }

    boost::optional< Operand_ref > return_operand() const {
      return _return_operand;
    }

    AbsValueDomain analyze(Basic_Block_ref bb, AbsValueDomain pre) {
      std::string fun_name = this->get_cfg().func_name();
      arbos_cfg::arbos_node_t node = this->get_cfg().get_node(bb);

      std::shared_ptr< sym_exec_call_t > sym_exec_call(
          new ValueSummaryExecCall(*this));
      std::shared_ptr< transfer_function_visitor_t > visitor(
          new transfer_function_visitor_t(pre,
                                          sym_exec_call,
                                          _summary_pass.var_factory(),
                                          _summary_pass.mem_factory(),
                                          _summary_pass.lit_factory(),
                                          this->get_cfg().arch(),
                                          MEM,
                                          _summary_pass.pointer_info()));
      node.accept(visitor);
      return visitor->inv();
    }

    template < typename Statement >
    AbsValueDomain analyze_stmt(Statement stmt, AbsValueDomain pre) {
      std::shared_ptr< sym_exec_call_t > sym_exec_call(
          new ValueSummaryExecCall(*this));
      std::shared_ptr< transfer_function_visitor_t > visitor(
          new transfer_function_visitor_t(pre,
                                          sym_exec_call,
                                          _summary_pass.var_factory(),
                                          _summary_pass.mem_factory(),
                                          _summary_pass.lit_factory(),
                                          this->get_cfg().arch(),
                                          MEM,
                                          _summary_pass.pointer_info()));
      visitor->visit(stmt);
      return visitor->inv();
    }

  private:
    void check_pre(Basic_Block_ref bb, AbsValueDomain pre) {
#ifdef DEBUG
      std::cerr << "\t" << ar::getName(bb) << ": " << pre << std::endl;
#endif
      _inv_table.insert(typename inv_table_t::value_type(bb, pre));
    }

    void check_post(Basic_Block_ref bb, AbsValueDomain post) {}

  public:
    void run(AbsValueDomain inv) {
      fwd_fixpoint_iterator_t::run(inv);
      _done = true;
    }

    void write(std::ostream& o) {
      for (typename inv_table_t::iterator it = _inv_table.begin();
           it != _inv_table.end();
           ++it) {
        o << "\t" << ar::getName(it->first) << ": " << it->second << std::endl;
      }
    }
  }; // end class NumericalAnalysis

private:
  typedef std::shared_ptr< NumericalAnalysis > numerical_analysis_ptr_t;
  typedef std::unordered_map< Function_ref, numerical_analysis_ptr_t >
      summaries_t;
  typedef PointerInfo::ptr_set_t ptr_set_t;

private:
  CfgFactory& _cfg_fac;
  VariableFactory& _vfac;
  memory_factory& _mfac;
  LiteralFactory& _lfac;
  LivenessPass& _live;
  PointerInfo _pointer_info;
  summaries_t _summaries;

public:
  ValueSummaryPass(CfgFactory& cfg_fac,
                   VariableFactory& vfac,
                   memory_factory& mfac,
                   LiteralFactory& lfac,
                   LivenessPass& live,
                   PointerInfo pointer_info)
      : _cfg_fac(cfg_fac),
        _vfac(vfac),
        _mfac(mfac),
        _lfac(lfac),
        _live(live),
        _pointer_info(pointer_info) {}

  inline CfgFactory& cfg_factory() { return _cfg_fac; }
  inline VariableFactory& var_factory() { return _vfac; }
  inline memory_factory& mem_factory() { return _mfac; }
  inline LiteralFactory& lit_factory() { return _lfac; }
  inline LivenessPass& live() { return _live; }
  inline PointerInfo pointer_info() { return _pointer_info; }

  inline AbsValueDomain default_call_context(Function_ref /*f*/) const {
    return AbsValueDomain::unchanged();
  }

  void execute(Bundle_ref bundle,
               StrongComponentsGraph_ref /*strong_comp_graph*/,
               const std::vector< Function_ref >& topo_order) {
    // bottom-up analysis
    for (std::vector< Function_ref >::const_iterator it = topo_order.begin();
         it != topo_order.end();
         ++it) {
      Function_ref root_fun = *it;
      std::cout << "** Analyzing function " << demangle(ar::getName(root_fun))
                << std::endl;
      arbos_cfg cfg = _cfg_fac[root_fun];
      numerical_analysis_ptr_t summary(
          new NumericalAnalysis(*this, bundle, cfg));
      _summaries.insert(typename summaries_t::value_type(root_fun, summary));
      summary->run(default_call_context(root_fun));
    }
  }

  NumericalAnalysis& operator[](Function_ref fun) const {
    assert(_summaries.find(fun) != _summaries.end());
    return *_summaries.at(fun);
  }

  void write(std::ostream& o) {
    for (typename summaries_t::iterator it = _summaries.begin();
         it != _summaries.end();
         ++it) {
      boost::optional< Operand_ref > return_op = it->second->return_operand();
      AbsValueDomain return_inv = it->second->return_inv();

      o << it->first << ": (";
      if (return_op) {
        o << *return_op;
      } else {
        o << "void";
      }
      o << ", ";
      return_inv.write(o);
      o << ")" << std::endl;
    }
  }

}; // end class ValueSummaryPass

/*
 * Checker pass using function summarization
 *
 * It propagates invariants with a top-down analysis using pre-computed
 * summaries.
 */
template < typename FunctionAnalyzer,
           typename AbsValueDomain,
           typename SumAbsValueDomain,
           typename VariableName,
           typename MemoryLocation,
           typename Number >
class CheckSummaryPass : public boost::noncopyable {
private:
  typedef CheckSummaryPass< FunctionAnalyzer,
                            AbsValueDomain,
                            SumAbsValueDomain,
                            VariableName,
                            MemoryLocation,
                            Number >
      check_summary_pass_t;
  typedef sym_exec_call< FunctionAnalyzer, AbsValueDomain > sym_exec_call_t;
  typedef typename sym_exec_call_t::sym_exec_call_ptr_t sym_exec_call_ptr_t;
  typedef typename sym_exec_call_t::function_names_t function_names_t;
  typedef ValueSummaryPass< SumAbsValueDomain,
                            VariableName,
                            MemoryLocation,
                            Number >
      value_summary_pass_t;
  typedef typename value_summary_pass_t::NumericalAnalysis summary_t;
  typedef PointerInfo::ptr_set_t ptr_set_t;
  typedef checker< AbsValueDomain > checker_t;

private:
  class PropagateExecCall : public sym_exec_call_t {
  private:
    Function_ref _function;
    check_summary_pass_t& _check_pass;

  public:
    PropagateExecCall(TrackedPrecision prec_level,
                      Function_ref function,
                      check_summary_pass_t& check_pass)
        : sym_exec_call_t(prec_level),
          _function(function),
          _check_pass(check_pass) {}

    AbsValueDomain call(
        context& ctx,
        FunctionAnalyzer& /*caller*/,
        Call_ref stmt,
        AbsValueDomain inv,
        bool /*convergence_achieved*/,
        bool /*is_ctx_stable*/,
        std::string current_path,
        function_names_t /*analyzed_funcs*/,
        std::shared_ptr< sym_exec_warning_callback > /*warning_callback*/ =
            nullptr) {
      VariableFactory& vfac = _check_pass.var_factory();
      value_summary_pass_t& summary_pass = _check_pass.summary_pass();
      std::vector< Function_ref > callees;

      if (ar::isDirectCall(stmt)) {
        callees.push_back(ar::getFunction(stmt));
      } else if (ar::isIndirectCall(stmt)) {
        PointerInfo pointer_info = _check_pass.pointer_info();
        Bundle_ref bundle = ctx.bundle();
        VariableName ptr = vfac.get_internal(ar::getIndirectCallVar(stmt));
        ptr_set_t ptr_set = pointer_info[ptr].first;

        if (ptr_set.is_top()) {
          // No points-to information
          location loc = ar::getSrcLoc(stmt);
          std::cerr << location_to_string(loc)
                    << ": warning: indirect call cannot be resolved (analysis "
                       "unsound if function has side effects)"
                    << std::endl;
          return call_forget(stmt, inv);
        }

        for (ptr_set_t::iterator it = ptr_set.begin(); it != ptr_set.end();
             ++it) {
          // Check if the callee is a function
          if (!isa< function_memory_location >(*it)) {
            continue;
          }
          auto mem_loc_it = cast< analyzer::function_memory_location >(*it);
          boost::optional< Function_ref > fun =
              ar::getFunction(bundle, mem_loc_it->name());

          if (!fun) { // external call
            return call_forget(stmt, inv);
          }

          callees.push_back(*fun);
        }
      } else {
        throw analyzer_error("unreachable");
      }

      // add calling contexts
      location loc = ar::getSrcLoc(stmt);
      std::string path = current_path + "/" + ar::getName(_function) + "@" +
                         std::to_string(loc.line) + "@" +
                         std::to_string(loc.column) + "@" +
                         std::to_string(ar::getUID(stmt));
      for (typename std::vector< Function_ref >::iterator it = callees.begin();
           it != callees.end();
           ++it) {
        Function_ref callee = *it;
        _check_pass.add_call_context(callee,
                                     path,
                                     match_parameters(stmt, inv, callee));
      }

      if (ikos::exc_domain_traits::is_normal_flow_bottom(inv)) {
        return inv;
      } else {
        AbsValueDomain result = AbsValueDomain::bottom();

        for (typename std::vector< Function_ref >::iterator it =
                 callees.begin();
             it != callees.end();
             ++it) {
          Function_ref callee = *it;
          const summary_t& callee_summary = summary_pass[callee];
          result =
              result | call_with_summary(stmt, inv, callee, callee_summary);
        }

        return result;
      }
    }

    void ret(context& ctx,
             FunctionAnalyzer& analyzer,
             Return_Value_ref stmt,
             AbsValueDomain pre) {}

    void exit(context& ctx, FunctionAnalyzer& analyzer, AbsValueDomain inv) {}

  private:
    AbsValueDomain match_parameters(Call_ref stmt,
                                    AbsValueDomain inv,
                                    Function_ref callee) {
      if (ikos::exc_domain_traits::is_normal_flow_bottom(inv)) {
        return inv;
      }

      CfgFactory& cfg_fac = _check_pass.cfg_factory();
      VariableFactory& vfac = _check_pass.var_factory();
      memory_factory& mfac = _check_pass.mem_factory();
      LiteralFactory& lfac = _check_pass.lit_factory();

      num_sym_exec< AbsValueDomain, Number, ikos::dummy_number >
          sym_exec(inv,
                   vfac,
                   mfac,
                   lfac,
                   cfg_fac.arch(),
                   this->_prec_level,
                   _check_pass.pointer_info());

      // assign parameters
      sym_exec.match_down(ar::getFormalParams(callee),
                          ar::getArguments(stmt),
                          false);

      inv = sym_exec.inv();

      // forget local vars
      ikos::patricia_tree_set< VariableName > vars =
          from_local_vars(cfg_fac[_function].local_variables(), lfac);
      inv.forget_mem_surface(vars.begin(), vars.end());

      // forget internal vars
      vars = from_internal_vars(cfg_fac[_function].internal_variables(), lfac);
      inv.forget_mem_surface(vars.begin(), vars.end());

      return inv;
    }

    AbsValueDomain call_with_summary(Call_ref stmt,
                                     AbsValueDomain caller_inv,
                                     Function_ref callee,
                                     const summary_t& summary) {
      if (ikos::exc_domain_traits::is_normal_flow_bottom(caller_inv)) {
        return caller_inv;
      }

      // convert the AbsValueDomain to a SumAbsValueDomain
      SumAbsValueDomain inv =
          ikos::num_domain_traits::convert< AbsValueDomain, SumAbsValueDomain >(
              caller_inv);

      CfgFactory& cfg_fac = _check_pass.cfg_factory();
      VariableFactory& vfac = _check_pass.var_factory();
      memory_factory& mfac = _check_pass.mem_factory();
      LiteralFactory& lfac = _check_pass.lit_factory();

      num_sym_exec< SumAbsValueDomain, Number, ikos::dummy_number >
          sym_exec(inv,
                   vfac,
                   mfac,
                   lfac,
                   cfg_fac.arch(),
                   this->_prec_level,
                   _check_pass.pointer_info());
      SumAbsValueDomain callee_inv = summary.return_inv();

      // assign parameters
      OpRange actual_params = ar::getArguments(stmt);
      IvRange formal_params = ar::getFormalParams(callee);
      sym_exec.match_down(formal_params, actual_params, false);

      // merge
      inv = sym_exec.inv();
      sym_exec.set_inv(inv.compose(callee_inv));

      // assign return value
      sym_exec.match_up(actual_params,
                        ar::getReturnValue(stmt),
                        formal_params,
                        summary.return_operand(),
                        false);

      inv = sym_exec.inv();

      // forget local variables
      ikos::patricia_tree_set< VariableName > vars =
          from_local_vars(cfg_fac[callee].local_variables(), lfac);
      inv.forget_mem_surface(vars.begin(), vars.end());

      // forget internal vars
      vars = from_internal_vars(cfg_fac[callee].internal_variables(), lfac);
      inv.forget_mem_surface(vars.begin(), vars.end());

      // forget formal parameters
      vars = from_internal_vars(formal_params, lfac);
      inv.forget_mem_surface(vars.begin(), vars.end());

      // convert the SumAbsValueDomain to a AbsValueDomain
      return ikos::num_domain_traits::convert< SumAbsValueDomain,
                                               AbsValueDomain >(inv);
    }

    // might be unsound because the memory could be updated
    AbsValueDomain call_forget(Call_ref stmt, AbsValueDomain inv) {
      if (ar::getReturnValue(stmt)) {
        VariableFactory& vfac = _check_pass.var_factory();
        VariableName lhs = vfac.get_internal(*(ar::getReturnValue(stmt)));
        inv -= lhs;
      }

      OpRange parameters = ar::getArguments(stmt);
      for (OpRange::iterator it = parameters.begin(); it != parameters.end();
           ++it) {
        if (ar::isPointer(*it)) {
          scalar_lit_t param = _check_pass.lit_factory()[*it];
          if (param.is_pointer_var()) {
            inv -= param.var();
          }
        }
      }

      return inv;
    }

  }; // end class PropagateExecCall

private:
  typedef std::unordered_map< std::string, AbsValueDomain > call_contexts_t;
  typedef std::unordered_map< Function_ref, call_contexts_t >
      map_call_contexts_t;

private:
  CfgFactory& _cfg_fac;
  VariableFactory& _vfac;
  memory_factory& _mfac;
  LiteralFactory& _lfac;
  LivenessPass& _live;
  PointerInfo _pointer_info;
  std::vector< std::unique_ptr< checker_t > >& _checkers;
  value_summary_pass_t& _summary_pass;

  bool _merge_call_contexts;
  map_call_contexts_t _call_contexts;

public:
  CheckSummaryPass(CfgFactory& cfg_fac,
                   VariableFactory& vfac,
                   memory_factory& mfac,
                   LiteralFactory& lfac,
                   LivenessPass& live,
                   PointerInfo pointer_info,
                   std::vector< std::unique_ptr< checker_t > >& checkers,
                   value_summary_pass_t& summary_pass,
                   bool merge_call_contexts = false)
      : _cfg_fac(cfg_fac),
        _vfac(vfac),
        _mfac(mfac),
        _lfac(lfac),
        _live(live),
        _pointer_info(pointer_info),
        _checkers(checkers),
        _summary_pass(summary_pass),
        _merge_call_contexts(merge_call_contexts) {}

  inline CfgFactory& cfg_factory() { return _cfg_fac; }
  inline VariableFactory& var_factory() { return _vfac; }
  inline memory_factory& mem_factory() { return _mfac; }
  inline LiteralFactory& lit_factory() { return _lfac; }
  inline LivenessPass& live() { return _live; }
  inline PointerInfo pointer_info() { return _pointer_info; }
  inline value_summary_pass_t& summary_pass() { return _summary_pass; }

  inline AbsValueDomain default_call_context(Function_ref /*f*/) const {
    return AbsValueDomain::unchanged();
  }

  void add_call_context(Function_ref fun,
                        const std::string& path,
                        AbsValueDomain inv) {
    if (ikos::exc_domain_traits::is_normal_flow_bottom(inv))
      return;

    if (_call_contexts.find(fun) == _call_contexts.end()) {
      _call_contexts.insert(
          typename map_call_contexts_t::value_type(fun, call_contexts_t()));
    }

#ifdef DEBUG
    std::cerr << "call context from " << path << ": " << inv << std::endl;
#endif

    if (_merge_call_contexts) {
      if (_call_contexts[fun].empty()) {
        _call_contexts[fun]["."] = inv;
      } else {
        _call_contexts[fun]["."] = _call_contexts[fun]["."] | inv;
      }
    } else {
      _call_contexts[fun][path] = inv;
    }
  }

  void execute(Bundle_ref bundle,
               const std::vector< Function_ref >& entry_points,
               results_table_t& results_table,
               StrongComponentsGraph_ref strong_comp_graph,
               const std::vector< Function_ref >& topo_order) {
    // create a context with no liveness information
    // TODO: fix the liveness analysis
    LivenessPass empty_live(_cfg_fac);
    context ctx(bundle, empty_live, _pointer_info, _cfg_fac, results_table);

    // top-down analysis
    for (std::vector< Function_ref >::const_reverse_iterator root_it =
             topo_order.rbegin();
         root_it != topo_order.rend();
         ++root_it) {
      std::vector< Function_ref > functions =
          strong_comp_graph->component(*root_it);

      for (std::vector< Function_ref >::const_iterator it = functions.begin();
           it != functions.end();
           ++it) {
        Function_ref fun = *it;
        arbos_cfg cfg = _cfg_fac[fun];

        std::shared_ptr< sym_exec_call_t > sym_exec_call(
            new PropagateExecCall(_cfg_fac.prec_level(), fun, *this));

        if (std::find(entry_points.begin(), entry_points.end(), fun) !=
            entry_points.end()) {
          std::cout << "*** Analyzing entry point "
                    << demangle(ar::getName(fun)) << std::endl;
          FunctionAnalyzer analysis(cfg, ctx, sym_exec_call, ".", _checkers);
          analysis.run(default_call_context(fun));
        }

        if (_call_contexts.find(fun) == _call_contexts.end()) {
          continue; // function never called
        }

        if (functions.size() > 1) {
          // to be sound on a cycle, we just assume the input domain is TOP
          std::cout << "*** Analyzing function " << demangle(ar::getName(fun))
                    << " (within a cycle of recursion)" << std::endl;
          FunctionAnalyzer analysis(cfg, ctx, sym_exec_call, "*", _checkers);
          analysis.run(default_call_context(fun));
        } else {
          // for each call context
          for (typename call_contexts_t::iterator call_it =
                   _call_contexts[fun].begin();
               call_it != _call_contexts[fun].end();
               ++call_it) {
            std::cout << "*** Analyzing function " << demangle(ar::getName(fun))
                      << " (called from " << call_it->first << ")" << std::endl;
            FunctionAnalyzer analysis(cfg,
                                      ctx,
                                      sym_exec_call,
                                      call_it->first,
                                      _checkers);
            analysis.run(call_it->second);
          }
        }
      }
    }
  }

}; // end class CheckSummaryPass

} // end namespace analyzer

#endif // ANALYZER_SUMMARY_HPP
