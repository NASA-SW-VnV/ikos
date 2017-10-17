/*******************************************************************************
 *
 * Analyzer ARBOS pass. This is the entry point for all analyses.
 *
 * Authors: Maxime Arthaud
 *          Jorge A. Navas
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

#include <iostream>
#include <memory>
#include <vector>

#include <ikos/domains/value_domain.hpp>
#include <ikos/domains/summary_domain.hpp>
#include <ikos/domains/exception.hpp>
#include <ikos/domains/counter_domains_api.hpp>

#include <analyzer/ar-wrapper/call_graph.hpp>
#include <analyzer/ar-wrapper/wrapper.hpp>
#include <analyzer/config.hpp>
#include <analyzer/utils/demangle.hpp>
#include <analyzer/utils/stats.hpp>

#include <analyzer/checkers/assert_prover_checker.hpp>
#include <analyzer/checkers/buffer_overflow_checker.hpp>
#include <analyzer/checkers/checker_api.hpp>
#include <analyzer/checkers/division_by_zero_checker.hpp>
#include <analyzer/checkers/null_dereference_checker.hpp>
#include <analyzer/checkers/pointer_alignment_checker.hpp>
#include <analyzer/checkers/uninitialized_variable_checker.hpp>

#include <analyzer/analysis/call_graph_topology.hpp>
#include <analyzer/analysis/function_pointer.hpp>
#include <analyzer/analysis/inliner.hpp>
#include <analyzer/analysis/liveness.hpp>
#include <analyzer/analysis/loop_counters.hpp>
#include <analyzer/analysis/memory_location.hpp>
#include <analyzer/analysis/num_sym_exec.hpp>
#include <analyzer/analysis/pointer.hpp>
#include <analyzer/analysis/summary.hpp>
#include <analyzer/analysis/sym_exec_warning_callbacks.hpp>
#include <analyzer/analysis/variable_name.hpp>

namespace analyzer {

using namespace arbos;

static Option< std::vector< std::string > > ANALYSES(
    "analysis,a", "Type of analysis (boa, dbz, prover, nullity, upa, uva)");
static Option< std::vector< std::string > > ENTRY_POINTS(
    "entry-points,e", "The entry point(s) of the program (default: main)");
static Option< bool > INTRAPROCEDURAL(
    "intra",
    "Run an intraprocedural analysis instead of an interprocedural analysis");
static Option< bool > NO_LIVENESS("no-liveness",
                                  "Do not run the liveness analysis");
static Option< bool > NO_POINTER("no-pointer",
                                 "Do not run the pointer analysis");
static Option< bool > USE_SUMMARIES(
    "summaries", "Use function summarization instead of inlining");
static Option< bool > USE_POINTER_SUMMARIES(
    "pointer-summaries", "Use function summarization for the pointer analysis");
static Option< std::string > PRECISION_LEVEL(
    "precision-level,p",
    "The precision level (reg, ptr, mem). Default to mem",
    "mem");
static Option< std::string > DISPLAY_INVARIANTS("display-invariants",
                                                "Display invariants",
                                                "off");
static Option< std::string > DISPLAY_CHECKS("display-checks",
                                            "Display checks",
                                            "off");
static Option< std::string > OUTPUT_DB("output-db",
                                       "The output database file",
                                       "output.db");

//! Propagate invariants through a function
template < typename AbsDomain,
           typename VariableName,
           typename MemoryLocation,
           typename Number >
class function_analyzer
    : public fwd_fixpoint_iterator< Basic_Block_ref, arbos_cfg, AbsDomain > {
private:
  typedef function_analyzer< AbsDomain, VariableName, MemoryLocation, Number >
      function_analyzer_t;
  typedef ikos::patricia_tree_set< VariableName > varname_set_t;
  typedef fwd_fixpoint_iterator< Basic_Block_ref, arbos_cfg, AbsDomain >
      fwd_fixpoint_iterator_t;
  typedef checker< AbsDomain > checker_t;

  typedef sym_exec_call< function_analyzer_t, AbsDomain > sym_exec_call_t;
  typedef typename sym_exec_call_t::sym_exec_call_ptr_t sym_exec_call_ptr_t;
  typedef typename sym_exec_call_t::function_names_t function_names_t;

private:
  //! Propagate invariants through a basic block
  class transfer_fun_visitor : public arbos_visitor_api {
  private:
    typedef num_sym_exec< AbsDomain, Number, ikos::dummy_number > sym_exec_t;

  private:
    //! global analysis state
    context& _context;

    //! symbolic execution object
    sym_exec_t _sym_exec;

    //! define the semantics for the callsite (e.g., inlining, etc)
    sym_exec_call_ptr_t _call_semantic;

    //! is the context stable?
    bool _is_context_stable;

    //! keep track of the convergence. We compute a global fixpoint by computing
    //! a fixpoint for each function.
    bool _convergence_achieved;

    //! current function analyzer
    function_analyzer_t& _function_analyzer;

    //! current call context
    std::string _call_context;

    //! already analyzed functions
    function_names_t _analyzed_functions;

    //! callback for warnings
    std::shared_ptr< sym_exec_warning_callback > _warning_callback;

  public:
    transfer_fun_visitor(
        context& ctx,
        AbsDomain inv,
        bool is_context_stable,
        bool convergence_achieved,
        function_analyzer_t& function_analyzer,
        std::string call_context,
        function_names_t analyzed_functions,
        sym_exec_call_ptr_t call_semantic,
        varname_set_t dead_vars = varname_set_t(),
        varname_set_t loop_counters = varname_set_t(),
        std::shared_ptr< sym_exec_warning_callback > warning_callback = nullptr)
        : _context(ctx),
          _sym_exec(inv,
                    _context.var_factory(),
                    _context.mem_factory(),
                    _context.lit_factory(),
                    _context.arch(),
                    _context.prec_level(),
                    _context.pointer_info(),
                    dead_vars,
                    loop_counters,
                    warning_callback),
          _call_semantic(call_semantic),
          _is_context_stable(is_context_stable),
          _convergence_achieved(convergence_achieved),
          _function_analyzer(function_analyzer),
          _call_context(call_context),
          _analyzed_functions(analyzed_functions),
          _warning_callback(warning_callback) {}

    AbsDomain inv() { return _sym_exec.inv(); }

    void visit_start(Basic_Block_ref b) { _sym_exec.exec_start(b); }
    void visit_end(Basic_Block_ref b) { _sym_exec.exec_end(b); }
    void visit(Arith_Op_ref s) { _sym_exec.exec(s); }
    void visit(Integer_Comparison_ref s) { _sym_exec.exec(s); }
    void visit(FP_Comparison_ref s) { _sym_exec.exec(s); }
    void visit(Assignment_ref s) { _sym_exec.exec(s); }
    void visit(Conv_Op_ref s) { _sym_exec.exec(s); }
    void visit(Bitwise_Op_ref s) { _sym_exec.exec(s); }
    void visit(FP_Op_ref s) { _sym_exec.exec(s); }
    void visit(Allocate_ref s) { _sym_exec.exec(s); }
    void visit(Store_ref s) { _sym_exec.exec(s); }
    void visit(Load_ref s) { _sym_exec.exec(s); }
    void visit(Insert_Element_ref s) { _sym_exec.exec(s); }
    void visit(Extract_Element_ref s) { _sym_exec.exec(s); }
    void visit(Pointer_Shift_ref s) { _sym_exec.exec(s); }
    void visit(Abstract_Variable_ref s) { _sym_exec.exec(s); }
    void visit(Abstract_Memory_ref s) { _sym_exec.exec(s); }
    void visit(MemCpy_ref s) { _sym_exec.exec(s); }
    void visit(MemMove_ref s) { _sym_exec.exec(s); }
    void visit(MemSet_ref s) { _sym_exec.exec(s); }
    void visit(Call_ref call_stmt) {
      _sym_exec.set_inv(_call_semantic->call(_context,
                                             _function_analyzer,
                                             call_stmt,
                                             _sym_exec.inv(),
                                             _convergence_achieved,
                                             _is_context_stable,
                                             _call_context,
                                             _analyzed_functions,
                                             _warning_callback));
    }
    void visit(Invoke_ref s) { visit(ar::getFunctionCall(s)); }
    void visit(Return_Value_ref s) {
      _sym_exec.exec(s);
      _call_semantic->ret(_context, _function_analyzer, s, _sym_exec.inv());
    }
    void visit(Landing_Pad_ref s) { _sym_exec.exec(s); }
    void visit(Resume_ref s) { _sym_exec.exec(s); }
    void visit(Unreachable_ref s) { _sym_exec.exec(s); }
    void visit(VA_Start_ref s) { _sym_exec.exec(s); }
    void visit(VA_End_ref s) { _sym_exec.exec(s); }
    void visit(VA_Arg_ref s) { _sym_exec.exec(s); }
    void visit(VA_Copy_ref s) { _sym_exec.exec(s); }

  }; // end class transfer_fun_visitor

private:
  typedef std::shared_ptr< transfer_fun_visitor > transfer_fun_visitor_ptr_t;

private:
  // Check properties and propagate invariants through a basic block
  class checker_visitor : public arbos_visitor_api {
  private:
    //! The transfer function to propagate the invariant through a basic block
    transfer_fun_visitor_ptr_t _transfer_fun;

    //! call string: context in which the current function is called
    std::string _call_context;

    //! list of property checks to run
    std::vector< std::unique_ptr< checker_t > >& _checkers;

  public:
    checker_visitor(transfer_fun_visitor_ptr_t transfer_fun,
                    const std::string& call_context,
                    std::vector< std::unique_ptr< checker_t > >& checkers)
        : _transfer_fun(transfer_fun),
          _call_context(call_context),
          _checkers(checkers) {}

  private:
    template < typename Statement >
    void check_and_propagate_inv(Statement stmt) {
      // check
      for (auto it = _checkers.begin(); it != _checkers.end(); ++it) {
        (*it)->check(stmt, _transfer_fun->inv(), _call_context);
      }

      // propagate invariant
      _transfer_fun->visit(stmt);
    }

  public:
    void visit_start(Basic_Block_ref b) {
      for (auto it = _checkers.begin(); it != _checkers.end(); ++it) {
        (*it)->check_start(b, _transfer_fun->inv(), _call_context);
      }
      _transfer_fun->visit_start(b);
    }

    void visit_end(Basic_Block_ref b) {
      for (auto it = _checkers.begin(); it != _checkers.end(); ++it) {
        (*it)->check_end(b, _transfer_fun->inv(), _call_context);
      }
      _transfer_fun->visit_end(b);
    }

    void visit(Arith_Op_ref s) { check_and_propagate_inv(s); }
    void visit(Integer_Comparison_ref s) { check_and_propagate_inv(s); }
    void visit(FP_Op_ref s) { check_and_propagate_inv(s); }
    void visit(FP_Comparison_ref s) { check_and_propagate_inv(s); }
    void visit(Assignment_ref s) { check_and_propagate_inv(s); }
    void visit(Conv_Op_ref s) { check_and_propagate_inv(s); }
    void visit(Bitwise_Op_ref s) { check_and_propagate_inv(s); }
    void visit(Allocate_ref s) { check_and_propagate_inv(s); }
    void visit(Store_ref s) { check_and_propagate_inv(s); }
    void visit(Load_ref s) { check_and_propagate_inv(s); }
    void visit(Insert_Element_ref s) { check_and_propagate_inv(s); }
    void visit(Extract_Element_ref s) { check_and_propagate_inv(s); }
    void visit(Pointer_Shift_ref s) { check_and_propagate_inv(s); }
    void visit(Abstract_Variable_ref s) { check_and_propagate_inv(s); }
    void visit(Abstract_Memory_ref s) { check_and_propagate_inv(s); }
    void visit(MemCpy_ref s) { check_and_propagate_inv(s); }
    void visit(MemMove_ref s) { check_and_propagate_inv(s); }
    void visit(MemSet_ref s) { check_and_propagate_inv(s); }
    void visit(Call_ref s) { check_and_propagate_inv(s); }
    void visit(Invoke_ref s) { check_and_propagate_inv(s); }
    void visit(Return_Value_ref s) { check_and_propagate_inv(s); }
    void visit(Landing_Pad_ref s) { check_and_propagate_inv(s); }
    void visit(Resume_ref s) { check_and_propagate_inv(s); }
    void visit(Unreachable_ref s) { check_and_propagate_inv(s); }
    void visit(VA_Start_ref s) { check_and_propagate_inv(s); }
    void visit(VA_End_ref s) { check_and_propagate_inv(s); }
    void visit(VA_Arg_ref s) { check_and_propagate_inv(s); }
    void visit(VA_Copy_ref s) { check_and_propagate_inv(s); }

  }; // end class checker_visitor

private:
  typedef std::shared_ptr< checker_visitor > checker_visitor_ptr_t;

private:
  //! global context
  context& _context;

  //! analyze calls and returns
  sym_exec_call_ptr_t _call_semantic;

  //! check whether the interprocedural analysis does not get into cycles
  function_names_t _analyzed_functions;

  //! call string: context in which the current function is called
  std::string _call_context;

  //! list of property checks to run
  std::vector< std::unique_ptr< checker_t > >& _checkers;

public:
  // entry point constructor
  function_analyzer(arbos_cfg cfg,
                    context& ctx,
                    sym_exec_call_ptr_t call_semantic,
                    std::string call_context,
                    std::vector< std::unique_ptr< checker_t > >& checkers)
      : fwd_fixpoint_iterator_t(cfg, true),
        _context(ctx),
        _call_semantic(call_semantic),
        _analyzed_functions(),
        _call_context(call_context),
        _checkers(checkers) {}

  // constructor called from the inliner
  function_analyzer(arbos_cfg cfg,
                    context& ctx,
                    sym_exec_call_ptr_t call_semantic,
                    bool is_context_stable,
                    function_analyzer_t& caller,
                    std::string call_context,
                    function_names_t analyzed_functions)
      : fwd_fixpoint_iterator_t(cfg, is_context_stable),
        _context(ctx),
        _call_semantic(call_semantic),
        _analyzed_functions(analyzed_functions),
        _call_context(call_context),
        _checkers(caller._checkers) {}

  std::string func_name() { return this->get_cfg().func_name(); }

  Function_ref function() { return this->get_cfg().function(); }

  sym_exec_call_ptr_t call_semantic() { return this->_call_semantic; }

  AbsDomain analyze(Basic_Block_ref bb, AbsDomain pre) {
    arbos_cfg::arbos_node_t node = this->get_cfg().get_node(bb);

    transfer_fun_visitor_ptr_t vis = std::make_shared<
        transfer_fun_visitor >(_context,
                               pre,
                               this->_is_context_stable,
                               false, /* convergence achieved */
                               *this, /* caller */
                               _call_context,
                               _analyzed_functions,
                               _call_semantic,
                               _context.liveness().deadSet(func_name(), bb),
                               node.loop_counters());
    node.accept(vis);
    return vis->inv();
  }

  void process_post(Basic_Block_ref bb, AbsDomain post) {
    boost::optional< Basic_Block_ref > exit = this->get_cfg().exit();
    if (exit && *exit == bb) { // exit node
      _call_semantic->exit(_context, *this, post);
    }

    fwd_fixpoint_iterator_t::process_post(bb, post);
  }

  void check_pre(Basic_Block_ref /*bb*/, AbsDomain /*pre*/) {}

  void check_post(Basic_Block_ref /*bb*/, AbsDomain /*post*/) {}

  /// Run checks with the previously computed fix-point
  void check() {
    for (auto it = this->get_cfg().begin(); it != this->get_cfg().end(); ++it) {
      arbos_cfg::arbos_node_t node = *it;
      Basic_Block_ref bb = node.get();
      AbsDomain pre = this->get_pre(bb);

      std::shared_ptr< sym_exec_warning_callback > warning_callback =
          std::make_shared< warning_database_writer >(_context.results_table(),
                                                      _call_context);

      transfer_fun_visitor_ptr_t transfer_fun_vis = std::make_shared<
          transfer_fun_visitor >(_context,
                                 pre,
                                 true,  /* context is stable */
                                 true,  /* convergence achieved */
                                 *this, /* caller */
                                 _call_context,
                                 _analyzed_functions,
                                 _call_semantic,
                                 _context.liveness().deadSet(func_name(), bb),
                                 node.loop_counters(),
                                 warning_callback);

      checker_visitor_ptr_t checker_vis =
          std::make_shared< checker_visitor >(transfer_fun_vis,
                                              _call_context,
                                              _checkers);

      node.accept(checker_vis);
    }
  }

}; // end class function_analyzer

class AnalyzerPass : public Pass {
public:
  AnalyzerPass() : Pass("analyzer", "Analyzer pass") {}

  virtual ~AnalyzerPass() {}

  virtual void execute(Bundle_ref bundle) {
    /*
     * Initialize database and tables
     */
    sqlite::db_connection db(OUTPUT_DB);
    settings_table_t settings_table(db);
    times_table_t times_table(db);
    results_table_t results_table(db);

    /*
     * Analysis options
     */

    if (!ANALYSES) {
      std::cerr << "error: missing argument --analysis" << std::endl;
      exit(EXIT_FAILURE);
    }
    const std::vector< std::string >& analyses = ANALYSES.getValue();

    for (auto it = analyses.begin(); it != analyses.end(); ++it) {
      if (*it != "boa" && *it != "dbz" && *it != "upa" && *it != "prover" &&
          *it != "nullity" && *it != "uva") {
        std::cerr << "error: unknown analysis \"" << *it << "\"" << std::endl;
        exit(EXIT_FAILURE);
      }
    }
    settings_table.write("analyses", to_json(analyses.begin(), analyses.end()));

    std::vector< Function_ref > entry_points;
    if (ENTRY_POINTS) {
      const std::vector< std::string >& entry_pts_names =
          ENTRY_POINTS.getValue();

      // find associated Function_ref
      for (auto it = entry_pts_names.begin(); it != entry_pts_names.end();
           ++it) {
        boost::optional< Function_ref > fun = ar::getFunction(bundle, *it);

        if (!fun) {
          std::cerr << "error: could not find function \"" << *it << "\""
                    << std::endl;
          exit(EXIT_FAILURE);
        }

        entry_points.push_back(*fun);
      }
      settings_table.write("entry-points",
                           to_json(entry_pts_names.begin(),
                                   entry_pts_names.end()));
    } else {
      if (INTRAPROCEDURAL) {
        // for intraprocedural analyses, all functions are entry points
        entry_points = ar::getFunctions(bundle);
        settings_table.write("entry-points", "all");
      } else {
        // try to find main()
        boost::optional< Function_ref > main_fun =
            ar::getFunction(bundle, "main");

        if (!main_fun) {
          std::cerr << "error: could not find main()" << std::endl;
          exit(EXIT_FAILURE);
        }

        entry_points.push_back(*main_fun);
        settings_table.write("entry-points", "[\"main\"]");
      }
    }

    const bool interprocedural = !INTRAPROCEDURAL;
    settings_table.write("procedural", interprocedural ? "inter" : "intra");

    const bool use_liveness = !NO_LIVENESS;
    settings_table.write("use-liveness", use_liveness);

    const bool use_pointer = !NO_POINTER;
    settings_table.write("use-pointer-analysis", use_pointer);

    const bool use_summaries = USE_SUMMARIES;
    settings_table.write("use-summaries", use_summaries);

    const bool use_pointer_summaries = USE_POINTER_SUMMARIES;
    settings_table.write("use-pointer-summaries", use_pointer_summaries);

    const display_settings display_invariants =
        ((DISPLAY_INVARIANTS.getValue() == "all")
             ? display_settings::ALL
             : (DISPLAY_INVARIANTS.getValue() == "fail")
                   ? display_settings::FAIL
                   : display_settings::OFF);

    const display_settings display_checks =
        ((DISPLAY_CHECKS.getValue() == "all")
             ? display_settings::ALL
             : (DISPLAY_CHECKS.getValue() == "fail") ? display_settings::FAIL
                                                     : display_settings::OFF);
    const TrackedPrecision precision_level =
        ((PRECISION_LEVEL.getValue() == "reg")
             ? REG
             : ((PRECISION_LEVEL.getValue() == "ptr") ? PTR : MEM));
    settings_table.write("precision-level",
                         ((precision_level == REG)
                              ? "reg"
                              : ((precision_level == PTR) ? "ptr" : "mem")));

    settings_table.write("num-abstract-domain",
                         abs_num_domain_t::domain_name());
    settings_table.write("summary-num-abstract-domain",
                         sum_abs_num_domain_t::domain_name());

    // Initialize factories
    VariableFactory var_factory;
    memory_factory mem_factory;
    LiteralFactory lit_factory(var_factory);
    CfgFactory cfg_factory(bundle, var_factory, mem_factory, lit_factory, precision_level);

    // Initialize liveness info
    LivenessPass liveness(cfg_factory);

    // Initialize context
    context ctx(bundle, liveness, PointerInfo(), cfg_factory, results_table);

    // Initialize stats
    analysis_stats stats;

#ifndef NDEBUG
    std::cerr << "* WARNING: ikos was compiled in debug mode, "
              << "the analysis might be slow." << std::endl;
#endif

    /*
     * Start the analysis
     */

    try {
      /*
       * First, run a liveness analysis
       *
       * The goal is to detect unused variables to speed up the following
       * analyses
       */
      if (use_liveness) {
        stats.start("liveness");
        std::cout << "Running liveness variable analysis ... " << std::endl;
        liveness.execute(bundle);
        // std::cerr << liveness << std::endl;
        stats.stop("liveness");
        times_table.write("arbos.liveness-analysis",
                          stats.elapsed("liveness").count());
      }

      /*
       * Find non-negative loop counters
       */
      if (ikos::counter_domain_traits::is_counter_domain<
              abs_num_domain_t >::value) {
        stats.start("loop-counters");
        std::cout << "Running detection of loop counters ... " << std::endl;
        LoopCountersPass loop_counters_pass(cfg_factory, lit_factory);
        loop_counters_pass.execute(bundle);
        // std::cerr << loop_counters_pass << std::endl;
        stats.stop("loop-counters");
        times_table.write("arbos.loop-counters",
                          stats.elapsed("loop-counters").count());
      }

      /*
       * Run a fast intraprocedural pointer analysis
       *
       * The goal here is to get all function pointers so that we can analyse
       * precisely indirect calls in the following analyses
       */
      FunctionPointerPass fun_pointer_pass(cfg_factory,
                                           var_factory,
                                           mem_factory,
                                           lit_factory);

      if (use_pointer) {
        stats.start("fun-pointer");
        std::cout << "Running function pointer analysis ... " << std::endl;
        fun_pointer_pass.execute(bundle);
        // std::cerr << fun_pointer_pass << std::endl;
        ctx.set_pointer_info(fun_pointer_pass.pointer_info());
        stats.stop("fun-pointer");
        times_table.write("arbos.function-pointer-analysis",
                          stats.elapsed("fun-pointer").count());
      }

      /*
       * Build the call graph and compute the topological order
       *
       * That step is only needed if you run an analysis using function
       * summarization.
       */
      StrongComponentsGraph_ref strong_comp_graph;
      std::vector< Function_ref > topo_order;

      if (use_summaries || use_pointer_summaries) {
        std::cout << "Building call graph ... " << std::endl;
        ar::buildCallGraph(bundle, ctx.pointer_info(), var_factory);
        // boost::write_graph(bundle);

        stats.start("connected-components");
        std::cout << "Strongly connected components analysis ... " << std::endl;
        strong_comp_graph =
            StrongComponentsGraph_ref(new StrongComponentsGraph(bundle));
        strong_comp_graph->build();
        // boost::write_graph(strong_comp_graph);
        stats.stop("connected-components");
        times_table.write("arbos.strongly-connected-components-analysis",
                          stats.elapsed("connected-components").count());

        stats.start("topological-order");
        std::cout << "Topological sort of the call graph ... " << std::endl;
        topo_order = topological_sort(strong_comp_graph);
        stats.stop("topological-order");
        times_table.write("arbos.topological-sort-call-graph",
                          stats.elapsed("topological-order").count());
      }

      /*
       * Run a deep (still intraprocedural) pointer analysis
       *
       * That step uses the result of the previous function pointer analysis.
       */
      if (use_pointer) {
        stats.start("pointer");
        if (use_pointer_summaries) {
          NumericalSummaryPass< sum_abs_num_domain_t,
                                varname_t,
                                memloc_t,
                                number_t >
              num_summary_pass(cfg_factory,
                               var_factory,
                               mem_factory,
                               lit_factory,
                               liveness,
                               ctx.pointer_info());
          std::cout << "Building numerical summaries (bottom-up) ... "
                    << std::endl;
          num_summary_pass.execute(bundle, strong_comp_graph, topo_order);
          // std::cerr << num_summary_pass << std::endl;

          PointerSummaryPass< abs_num_domain_t,
                              sum_abs_num_domain_t,
                              varname_t,
                              memloc_t,
                              number_t >
              pointer_pass(cfg_factory,
                           var_factory,
                           mem_factory,
                           lit_factory,
                           liveness,
                           ctx.pointer_info(),
                           num_summary_pass);
          std::cout << "Running pointer analysis ... " << std::endl;
          pointer_pass.execute(bundle, strong_comp_graph, topo_order);
          // std::cerr << pointer_pass << std::endl;
          ctx.set_pointer_info(pointer_pass.pointer_info());
        } else {
          PointerPass pointer_pass(cfg_factory,
                                   var_factory,
                                   mem_factory,
                                   lit_factory,
                                   liveness,
                                   ctx.pointer_info());
          std::cout << "Running pointer analysis ... " << std::endl;
          pointer_pass.execute(bundle);
          // std::cerr << pointer_pass << std::endl;
          ctx.set_pointer_info(pointer_pass.pointer_info());
        }
        stats.stop("pointer");
        times_table.write("arbos.pointer-analysis",
                          stats.elapsed("pointer").count());
      }

      /*
       * Final step, run a value analysis, and check properties on the result
       */
      stats.start("value");
      if (use_summaries) {
        typedef ikos::
            summary_domain< abs_num_domain_t, VariableFactory, memloc_t >
                abs_value_domain_t;
        typedef ikos::
            summary_domain< sum_abs_num_domain_t, VariableFactory, memloc_t >
                sum_abs_value_domain_t;
        typedef function_analyzer< abs_value_domain_t,
                                   varname_t,
                                   memloc_t,
                                   number_t >
            function_analyzer_t;
        typedef checker< abs_value_domain_t > checker_t;
        typedef std::unique_ptr< checker_t > checker_ptr_t;

        std::vector< checker_ptr_t > checkers =
            generate_checkers< abs_value_domain_t >(analyses,
                                                    ctx,
                                                    results_table,
                                                    display_invariants,
                                                    display_checks);

        ValueSummaryPass< sum_abs_value_domain_t,
                          varname_t,
                          memloc_t,
                          number_t >
            value_summary_pass(cfg_factory,
                               var_factory,
                               mem_factory,
                               lit_factory,
                               liveness,
                               ctx.pointer_info());
        std::cout << "Building value analysis summaries (bottom-up) ... "
                  << std::endl;
        value_summary_pass.execute(bundle, strong_comp_graph, topo_order);
        // std::cerr << value_summary_pass << std::endl;

        CheckSummaryPass< function_analyzer_t,
                          abs_value_domain_t,
                          sum_abs_value_domain_t,
                          varname_t,
                          memloc_t,
                          number_t >
            check_pass(cfg_factory,
                       var_factory,
                       mem_factory,
                       lit_factory,
                       liveness,
                       ctx.pointer_info(),
                       checkers,
                       value_summary_pass);
        std::cout << "Running value analysis (top-down) ... " << std::endl;
        check_pass.execute(bundle,
                           entry_points,
                           results_table,
                           strong_comp_graph,
                           topo_order);
      } else {
        typedef ikos::exception_domain_impl<
            ikos::value_domain< abs_num_domain_t, VariableFactory, memloc_t >,
            VariableFactory,
            memloc_t >
            abs_value_domain_t;
        typedef function_analyzer< abs_value_domain_t,
                                   varname_t,
                                   memloc_t,
                                   number_t >
            function_analyzer_t;
        typedef sym_exec_call< function_analyzer_t, abs_value_domain_t >
            sym_exec_call_t;
        typedef
            typename sym_exec_call_t::sym_exec_call_ptr_t sym_exec_call_ptr_t;
        typedef inline_sym_exec_call< function_analyzer_t, abs_value_domain_t >
            inline_sym_exec_call_t;
        typedef context_insensitive_sym_exec_call< function_analyzer_t,
                                                   abs_value_domain_t >
            context_insensitive_sym_exec_call_t;
        typedef checker< abs_value_domain_t > checker_t;
        typedef std::unique_ptr< checker_t > checker_ptr_t;

        std::cout << "Running value analysis ... " << std::endl;
        std::vector< checker_ptr_t > checkers =
            generate_checkers< abs_value_domain_t >(analyses,
                                                    ctx,
                                                    results_table,
                                                    display_invariants,
                                                    display_checks);

        for (auto it = entry_points.begin(); it != entry_points.end(); ++it) {
          Function_ref function = *it;
          std::string fun_name = ar::getName(function);
          arbos_cfg cfg = cfg_factory[function];

          sym_exec_call_ptr_t call_semantic;

          if (interprocedural) {
            call_semantic = sym_exec_call_ptr_t(
                new inline_sym_exec_call_t(precision_level));
          } else {
            call_semantic = sym_exec_call_ptr_t(
                new context_insensitive_sym_exec_call_t(precision_level));
          }

          function_analyzer_t analyzer(cfg, ctx, call_semantic, ".", checkers);

          stats.start("value." + fun_name);
          std::cout << "*** Analyzing entry point: " << demangle(fun_name)
                    << std::endl;
          analyzer.run(abs_value_domain_t::top_no_exception());
          stats.stop("value." + fun_name);

          stats.start("check." + fun_name);
          std::cout << "*** Writing results for entry point: "
                    << demangle(fun_name) << std::endl;
          analyzer.check();
          stats.stop("check." + fun_name);

          times_table.write("arbos.value-analysis." + fun_name,
                            stats.elapsed("value." + fun_name).count());
          times_table.write("arbos.check." + fun_name,
                            stats.elapsed("check." + fun_name).count());
        }
      }
      stats.stop("value");
      times_table.write("arbos.value-analysis", stats.elapsed("value").count());
    } catch (analyzer_error& e) {
      std::cerr << "analyzer error: " << e << std::endl;
      exit(EXIT_FAILURE);
    } catch (ikos::exception& e) {
      std::cerr << "ikos error: " << e.what() << std::endl;
      exit(EXIT_FAILURE);
    } catch (arbos::error& e) {
      std::cerr << "arbos error: " << e << std::endl;
      exit(EXIT_FAILURE);
    } catch (sqlite::db_error& e) {
      std::cerr << "db error: " << e << std::endl;
      exit(EXIT_FAILURE);
    } catch (std::exception& e) {
      std::cerr << "system error: " << e.what() << std::endl;
      exit(EXIT_FAILURE);
    } catch (...) {
      std::cerr << "unknown error occurred" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  template < typename AbsDomain >
  static std::vector< std::unique_ptr< checker< AbsDomain > > >
  generate_checkers(const std::vector< std::string >& analyses,
                    context& ctx,
                    results_table_t& results_table,
                    const display_settings display_invariants,
                    const display_settings display_checks) {
    typedef checker< AbsDomain > checker_t;
    typedef std::unique_ptr< checker_t > checker_ptr_t;

    std::vector< checker_ptr_t > checkers;

    for (auto it = analyses.begin(); it != analyses.end(); ++it) {
      if (*it == "boa") {
        checker_ptr_t checker = std::make_unique<
            buffer_overflow_checker< AbsDomain > >(ctx,
                                                   results_table,
                                                   display_invariants,
                                                   display_checks);
        checkers.push_back(std::move(checker));
      } else if (*it == "dbz") {
        checker_ptr_t checker = std::make_unique<
            division_by_zero_checker< AbsDomain > >(ctx,
                                                    results_table,
                                                    display_invariants,
                                                    display_checks);
        checkers.push_back(std::move(checker));
      } else if (*it == "prover") {
        checker_ptr_t checker = std::make_unique<
            assert_prover_checker< AbsDomain > >(ctx,
                                                 results_table,
                                                 display_invariants,
                                                 display_checks);
        checkers.push_back(std::move(checker));
      } else if (*it == "nullity") {
        checker_ptr_t checker = std::make_unique<
            null_dereference_checker< AbsDomain > >(ctx,
                                                    results_table,
                                                    display_invariants,
                                                    display_checks);
        checkers.push_back(std::move(checker));
      } else if (*it == "upa") {
        checker_ptr_t checker = std::make_unique<
            pointer_alignment_checker< AbsDomain > >(ctx,
                                                     results_table,
                                                     display_invariants,
                                                     display_checks);
        checkers.push_back(std::move(checker));
      } else if (*it == "uva") {
        checker_ptr_t checker = std::make_unique<
            uninitialized_variable_checker< AbsDomain > >(ctx,
                                                          results_table,
                                                          display_invariants,
                                                          display_checks);
        checkers.push_back(std::move(checker));
      } else {
        std::cerr << "error: unknown analysis \"" << *it << "\"" << std::endl;
        exit(EXIT_FAILURE);
      }
    }

    return std::move(checkers);
  }

}; // end class AnalyzerPass

} // end of analyzer namespace

extern "C" arbos::Pass* init() {
  return new analyzer::AnalyzerPass();
}
