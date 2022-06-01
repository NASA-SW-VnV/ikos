/*******************************************************************************
 *
 * ikos-analyzer -- IKOS static analyzer
 *
 * This is the entry point for all analyses.
 *
 * Authors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2019 United States Government as represented by the
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

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/Process.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/WithColor.h>
#include <llvm/Support/raw_ostream.h>

#include <ikos/ar/format/dot.hpp>
#include <ikos/ar/format/formatter.hpp>
#include <ikos/ar/format/text.hpp>
#include <ikos/ar/pass/add_loop_counters.hpp>
#include <ikos/ar/pass/add_partitioning_variables.hpp>
#include <ikos/ar/pass/name_values.hpp>
#include <ikos/ar/pass/simplify_cfg.hpp>
#include <ikos/ar/pass/simplify_upcast_comparison.hpp>
#include <ikos/ar/verify/frontend.hpp>
#include <ikos/ar/verify/type.hpp>

#include <ikos/frontend/llvm/import.hpp>

#include <ikos/analyzer/analysis/call_context.hpp>
#include <ikos/analyzer/analysis/context.hpp>
#include <ikos/analyzer/analysis/fixpoint_parameters.hpp>
#include <ikos/analyzer/analysis/hardware_addresses.hpp>
#include <ikos/analyzer/analysis/literal.hpp>
#include <ikos/analyzer/analysis/liveness.hpp>
#include <ikos/analyzer/analysis/memory_location.hpp>
#include <ikos/analyzer/analysis/option.hpp>
#include <ikos/analyzer/analysis/pointer/function.hpp>
#include <ikos/analyzer/analysis/pointer/pointer.hpp>
#include <ikos/analyzer/analysis/result.hpp>
#include <ikos/analyzer/analysis/value/interprocedural/concurrent/analysis.hpp>
#include <ikos/analyzer/analysis/value/interprocedural/sequential/analysis.hpp>
#include <ikos/analyzer/analysis/value/intraprocedural/concurrent/analysis.hpp>
#include <ikos/analyzer/analysis/value/intraprocedural/sequential/analysis.hpp>
#include <ikos/analyzer/analysis/variable.hpp>
#include <ikos/analyzer/analysis/widening_hint.hpp>
#include <ikos/analyzer/checker/name.hpp>
#include <ikos/analyzer/database/output.hpp>
#include <ikos/analyzer/util/color.hpp>
#include <ikos/analyzer/util/log.hpp>
#include <ikos/analyzer/util/timer.hpp>

namespace ar = ikos::ar;
namespace llvm_to_ar = ikos::frontend::import;
namespace analyzer = ikos::analyzer;

/// \name Main options
/// @{

static llvm::cl::OptionCategory MainCategory("Main Options");

static llvm::cl::opt< std::string > InputFilename(
    llvm::cl::Positional,
    llvm::cl::desc("<input bitcode file>"),
    llvm::cl::Required,
    llvm::cl::value_desc("file"));

static llvm::cl::opt< std::string > OutputFilename(
    "o",
    llvm::cl::desc("Output database filename (default: output.db)"),
    llvm::cl::value_desc("file"),
    llvm::cl::init("output.db"),
    llvm::cl::cat(MainCategory));

static llvm::cl::opt< analyzer::LogLevel > LogLevel(
    "log",
    llvm::cl::desc("Log level:"),
    llvm::cl::values(
        clEnumValN(analyzer::LogLevel::None, "none", "Disable logging"),
        clEnumValN(analyzer::LogLevel::Critical, "critical", "Critical level"),
        clEnumValN(analyzer::LogLevel::Error, "error", "Error level"),
        clEnumValN(analyzer::LogLevel::Warning,
                   "warning",
                   "Warning level (default)"),
        clEnumValN(analyzer::LogLevel::Info, "info", "Informative level"),
        clEnumValN(analyzer::LogLevel::Debug, "debug", "Debug level"),
        clEnumValN(analyzer::LogLevel::All, "all", "Show all messages")),
    llvm::cl::init(analyzer::LogLevel::Warning),
    llvm::cl::cat(MainCategory));

static llvm::cl::opt< analyzer::ProgressOption > Progress(
    "progress",
    llvm::cl::desc("Progress report:"),
    llvm::cl::values(
        clEnumValN(analyzer::ProgressOption::Auto,
                   "auto",
                   "Interactive if the output is a terminal (default)"),
        clEnumValN(analyzer::ProgressOption::Interactive,
                   "interactive",
                   "Interactive"),
        clEnumValN(analyzer::ProgressOption::Linear, "linear", "Linear"),
        clEnumValN(analyzer::ProgressOption::None,
                   "no",
                   "Disable progress report")),
    llvm::cl::init(analyzer::ProgressOption::Auto),
    llvm::cl::cat(MainCategory));

/// @}
/// \name Analysis options
/// @{

static llvm::cl::OptionCategory AnalysisCategory("Analysis Options");

static llvm::cl::list< analyzer::CheckerName > Analyses(
    "a",
    llvm::cl::desc("Available analyses:"),
    llvm::cl::CommaSeparated,
    llvm::cl::OneOrMore,
    llvm::cl::values(
        clEnumValN(analyzer::CheckerName::BufferOverflow,
                   checker_short_name(analyzer::CheckerName::BufferOverflow),
                   checker_long_name(analyzer::CheckerName::BufferOverflow)),
        clEnumValN(analyzer::CheckerName::DivisionByZero,
                   checker_short_name(analyzer::CheckerName::DivisionByZero),
                   checker_long_name(analyzer::CheckerName::DivisionByZero)),
        clEnumValN(
            analyzer::CheckerName::NullPointerDereference,
            checker_short_name(analyzer::CheckerName::NullPointerDereference),
            checker_long_name(analyzer::CheckerName::NullPointerDereference)),
        clEnumValN(analyzer::CheckerName::AssertProver,
                   checker_short_name(analyzer::CheckerName::AssertProver),
                   checker_long_name(analyzer::CheckerName::AssertProver)),
        clEnumValN(analyzer::CheckerName::UnalignedPointer,
                   checker_short_name(analyzer::CheckerName::UnalignedPointer),
                   checker_long_name(analyzer::CheckerName::UnalignedPointer)),
        clEnumValN(
            analyzer::CheckerName::UninitializedVariable,
            checker_short_name(analyzer::CheckerName::UninitializedVariable),
            checker_long_name(analyzer::CheckerName::UninitializedVariable)),
        clEnumValN(analyzer::CheckerName::SignedIntOverflow,
                   checker_short_name(analyzer::CheckerName::SignedIntOverflow),
                   checker_long_name(analyzer::CheckerName::SignedIntOverflow)),
        clEnumValN(
            analyzer::CheckerName::UnsignedIntOverflow,
            checker_short_name(analyzer::CheckerName::UnsignedIntOverflow),
            checker_long_name(analyzer::CheckerName::UnsignedIntOverflow)),
        clEnumValN(analyzer::CheckerName::ShiftCount,
                   checker_short_name(analyzer::CheckerName::ShiftCount),
                   checker_long_name(analyzer::CheckerName::ShiftCount)),
        clEnumValN(analyzer::CheckerName::PointerOverflow,
                   checker_short_name(analyzer::CheckerName::PointerOverflow),
                   checker_long_name(analyzer::CheckerName::PointerOverflow)),
        clEnumValN(analyzer::CheckerName::PointerCompare,
                   checker_short_name(analyzer::CheckerName::PointerCompare),
                   checker_long_name(analyzer::CheckerName::PointerCompare)),
        clEnumValN(analyzer::CheckerName::Soundness,
                   checker_short_name(analyzer::CheckerName::Soundness),
                   checker_long_name(analyzer::CheckerName::Soundness)),
        clEnumValN(analyzer::CheckerName::FunctionCall,
                   checker_short_name(analyzer::CheckerName::FunctionCall),
                   checker_long_name(analyzer::CheckerName::FunctionCall)),
        clEnumValN(analyzer::CheckerName::DeadCode,
                   checker_short_name(analyzer::CheckerName::DeadCode),
                   checker_long_name(analyzer::CheckerName::DeadCode)),
        clEnumValN(analyzer::CheckerName::DoubleFree,
                   checker_short_name(analyzer::CheckerName::DoubleFree),
                   checker_long_name(analyzer::CheckerName::DoubleFree)),
        clEnumValN(analyzer::CheckerName::Debug,
                   checker_short_name(analyzer::CheckerName::Debug),
                   checker_long_name(analyzer::CheckerName::Debug)),
        clEnumValN(analyzer::CheckerName::MemoryWatch,
                   checker_short_name(analyzer::CheckerName::MemoryWatch),
                   checker_long_name(analyzer::CheckerName::MemoryWatch))),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< analyzer::MachineIntDomainOption > Domain(
    "d",
    llvm::cl::desc("Available abstract domains:"),
    llvm::cl::values(
        clEnumValN(analyzer::MachineIntDomainOption::Interval,
                   machine_int_domain_option_str(
                       analyzer::MachineIntDomainOption::Interval),
                   "Interval domain"),
        clEnumValN(analyzer::MachineIntDomainOption::Congruence,
                   machine_int_domain_option_str(
                       analyzer::MachineIntDomainOption::Congruence),
                   "Congruence domain"),
        clEnumValN(analyzer::MachineIntDomainOption::IntervalCongruence,
                   machine_int_domain_option_str(
                       analyzer::MachineIntDomainOption::IntervalCongruence),
                   "Reduced product of Interval and Congruence"),
        clEnumValN(analyzer::MachineIntDomainOption::DBM,
                   machine_int_domain_option_str(
                       analyzer::MachineIntDomainOption::DBM),
                   "Difference-Bound Matrices domain"),
        clEnumValN(analyzer::MachineIntDomainOption::VarPackDBM,
                   machine_int_domain_option_str(
                       analyzer::MachineIntDomainOption::VarPackDBM),
                   "Difference-Bound Matrices domain with variable packing"),
        clEnumValN(
            analyzer::MachineIntDomainOption::VarPackDBMCongruence,
            machine_int_domain_option_str(
                analyzer::MachineIntDomainOption::VarPackDBMCongruence),
            "Reduced product of DBM with variable packing and Congruence"),
        clEnumValN(analyzer::MachineIntDomainOption::Gauge,
                   machine_int_domain_option_str(
                       analyzer::MachineIntDomainOption::Gauge),
                   "Gauge domain"),
        clEnumValN(
            analyzer::MachineIntDomainOption::GaugeIntervalCongruence,
            machine_int_domain_option_str(
                analyzer::MachineIntDomainOption::GaugeIntervalCongruence),
            "Reduced product of Gauge, Interval and Congruence"),
        clEnumValN(analyzer::MachineIntDomainOption::ApronInterval,
                   machine_int_domain_option_str(
                       analyzer::MachineIntDomainOption::ApronInterval),
                   "APRON Interval domain"),
        clEnumValN(analyzer::MachineIntDomainOption::ApronOctagon,
                   machine_int_domain_option_str(
                       analyzer::MachineIntDomainOption::ApronOctagon),
                   "APRON Octagon domain"),
        clEnumValN(analyzer::MachineIntDomainOption::ApronPolkaPolyhedra,
                   machine_int_domain_option_str(
                       analyzer::MachineIntDomainOption::ApronPolkaPolyhedra),
                   "APRON Polka Polyhedra domain"),
        clEnumValN(
            analyzer::MachineIntDomainOption::ApronPolkaLinearEqualities,
            machine_int_domain_option_str(
                analyzer::MachineIntDomainOption::ApronPolkaLinearEqualities),
            "APRON Polka Linear Equalities domain"),
        clEnumValN(analyzer::MachineIntDomainOption::ApronPplPolyhedra,
                   machine_int_domain_option_str(
                       analyzer::MachineIntDomainOption::ApronPplPolyhedra),
                   "APRON PPL Polyhedra domain"),
        clEnumValN(
            analyzer::MachineIntDomainOption::ApronPplLinearCongruences,
            machine_int_domain_option_str(
                analyzer::MachineIntDomainOption::ApronPplLinearCongruences),
            "APRON PPL Linear Congruences domain"),
        clEnumValN(analyzer::MachineIntDomainOption::
                       ApronPkgridPolyhedraLinearCongruences,
                   machine_int_domain_option_str(
                       analyzer::MachineIntDomainOption::
                           ApronPkgridPolyhedraLinearCongruences),
                   "APRON Pkgrid Polyhedra and Linear Congruences domain"),
        clEnumValN(analyzer::MachineIntDomainOption::VarPackApronOctagon,
                   machine_int_domain_option_str(
                       analyzer::MachineIntDomainOption::VarPackApronOctagon),
                   "APRON Octagon domain with variable packing"),
        clEnumValN(
            analyzer::MachineIntDomainOption::VarPackApronPolkaPolyhedra,
            machine_int_domain_option_str(
                analyzer::MachineIntDomainOption::VarPackApronPolkaPolyhedra),
            "APRON Polka Polyhedra domain with variable packing"),
        clEnumValN(
            analyzer::MachineIntDomainOption::VarPackApronPolkaLinearEqualities,
            machine_int_domain_option_str(
                analyzer::MachineIntDomainOption::
                    VarPackApronPolkaLinearEqualities),
            "APRON Polka Linear Equalities domain with variable packing"),
        clEnumValN(
            analyzer::MachineIntDomainOption::VarPackApronPplPolyhedra,
            machine_int_domain_option_str(
                analyzer::MachineIntDomainOption::VarPackApronPplPolyhedra),
            "APRON PPL Polyhedra domain with variable packing"),
        clEnumValN(
            analyzer::MachineIntDomainOption::VarPackApronPplLinearCongruences,
            machine_int_domain_option_str(analyzer::MachineIntDomainOption::
                                              VarPackApronPplLinearCongruences),
            "APRON PPL Linear Congruences domain with variable packing"),
        clEnumValN(analyzer::MachineIntDomainOption::
                       VarPackApronPkgridPolyhedraLinearCongruences,
                   machine_int_domain_option_str(
                       analyzer::MachineIntDomainOption::
                           VarPackApronPkgridPolyhedraLinearCongruences),
                   "APRON Pkgrid Polyhedra and Linear Congruences domain with "
                   "variable packing")),
    llvm::cl::init(analyzer::MachineIntDomainOption::Interval),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::list< std::string > EntryPoints(
    "entry-points",
    llvm::cl::desc("List of program entry points (ex: main)"),
    llvm::cl::CommaSeparated,
    llvm::cl::OneOrMore,
    llvm::cl::value_desc("function"),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::list< std::string > NoInitGlobals(
    "no-init-globals",
    llvm::cl::desc(
        "Global variables should not be initialized\nfor these entry points"),
    llvm::cl::CommaSeparated,
    llvm::cl::value_desc("function"),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< analyzer::Procedural > Procedural(
    "proc",
    llvm::cl::desc("Procedurality:"),
    llvm::cl::values(clEnumValN(analyzer::Procedural::Interprocedural,
                                "inter",
                                "Interprocedural analysis (default)"),
                     clEnumValN(analyzer::Procedural::Intraprocedural,
                                "intra",
                                "Intraprocedural analysis")),
    llvm::cl::init(analyzer::Procedural::Interprocedural),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< int > Jobs("j",
                                 llvm::cl::desc("Number of threads"),
                                 llvm::cl::init(1),
                                 llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< analyzer::WideningStrategy > WideningStrategy(
    "widening-strategy",
    llvm::cl::desc("Strategy for increasing iterations"),
    llvm::cl::values(
        clEnumValN(analyzer::WideningStrategy::Widen,
                   widening_strategy_str(analyzer::WideningStrategy::Widen),
                   "Widening operator (default)"),
        clEnumValN(analyzer::WideningStrategy::Join,
                   widening_strategy_str(analyzer::WideningStrategy::Join),
                   "Join operator")),
    llvm::cl::init(analyzer::WideningStrategy::Widen),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< analyzer::NarrowingStrategy > NarrowingStrategy(
    "narrowing-strategy",
    llvm::cl::desc("Strategy for decreasing iterations"),
    llvm::cl::values(
        clEnumValN(analyzer::NarrowingStrategy::Narrow,
                   narrowing_strategy_str(analyzer::NarrowingStrategy::Narrow),
                   "Narrowing operator (default)"),
        clEnumValN(analyzer::NarrowingStrategy::Meet,
                   narrowing_strategy_str(analyzer::NarrowingStrategy::Meet),
                   "Meet operator")),
    llvm::cl::init(analyzer::NarrowingStrategy::Narrow),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< unsigned > WideningDelay(
    "widening-delay",
    llvm::cl::desc(
        "Number of loop iterations before using the widening strategy"),
    llvm::cl::init(1),
    llvm::cl::value_desc("int"),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::list< std::string > WideningDelayFunctions(
    "widening-delay-functions",
    llvm::cl::desc("Widening delay for specific functions"),
    llvm::cl::CommaSeparated,
    llvm::cl::value_desc("function:int"),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< unsigned > WideningPeriod(
    "widening-period",
    llvm::cl::desc("Number of loop iterations between each widening"),
    llvm::cl::init(1),
    llvm::cl::value_desc("int"),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< int > NarrowingIterations(
    "narrowing-iterations",
    llvm::cl::desc("Perform a fixed number of narrowing iterations"),
    llvm::cl::init(-1),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< bool > NoLiveness(
    "no-liveness",
    llvm::cl::desc("Disable the liveness analysis"),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< bool > NoPointer(
    "no-pointer",
    llvm::cl::desc("Disable the pointer analysis"),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< bool > NoWideningHints(
    "no-widening-hints",
    llvm::cl::desc("Disable the widening hint analysis"),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< bool > EnablePartitioningDomain(
    "enable-partitioning-domain",
    llvm::cl::desc("Enable the partitioning abstract domain"),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< bool > NoFixpointCache(
    "no-fixpoint-cache",
    llvm::cl::desc("Disable the cache of fixpoints"),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< bool > NoChecks("no-checks",
                                      llvm::cl::desc("Disable all the checks"),
                                      llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< analyzer::GlobalsInitPolicy > GlobalsInitPolicy(
    "globals-init",
    llvm::cl::desc("Policy of initialization for global variables"),
    llvm::cl::values(
        clEnumValN(analyzer::GlobalsInitPolicy::All,
                   "all",
                   "Initialize all global variables"),
        clEnumValN(
            analyzer::GlobalsInitPolicy::SkipBigArrays,
            "skip-big-arrays",
            "Initialize all global variables except big arrays (default)"),
        clEnumValN(analyzer::GlobalsInitPolicy::SkipStrings,
                   "skip-strings",
                   "Initialize all global variables except strings"),
        clEnumValN(analyzer::GlobalsInitPolicy::None,
                   "none",
                   "Do not initialize any global variable")),
    llvm::cl::init(analyzer::GlobalsInitPolicy::SkipBigArrays),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::list< std::string > HardwareAddresses(
    "hardware-addresses",
    llvm::cl::desc(
        "Specify ranges (x-y) of hardware addresses, separated by a comma"),
    llvm::cl::CommaSeparated,
    llvm::cl::value_desc("range"),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< std::string > HardwareAddressesFile(
    "hardware-addresses-file",
    llvm::cl::desc("Specify ranges (x-y) of hardware addresses from a file "
                   "(one range per line)"),
    llvm::cl::value_desc("file"),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< int > Argc("argc",
                                 llvm::cl::desc("Specify a value for argc"),
                                 llvm::cl::init(-1),
                                 llvm::cl::cat(AnalysisCategory));

/// @}
/// \name Import options
/// @{

static llvm::cl::OptionCategory ImportCategory(
    "Import Options", "Options for the translation from LLVM to AR");

static llvm::cl::opt< bool > NoVerify(
    "no-verify", llvm::cl::desc("Do not run the LLVM bitcode verifier"));

static llvm::cl::opt< bool > NoLibIkos(
    "no-libikos",
    llvm::cl::desc("Do not use ikos intrinsics (__ikos_assert, etc.)"),
    llvm::cl::cat(ImportCategory));

static llvm::cl::opt< bool > NoLibc(
    "no-libc",
    llvm::cl::desc("Do not use libc intrinsics (malloc, free, etc.)"),
    llvm::cl::cat(ImportCategory));

static llvm::cl::opt< bool > NoLibcpp(
    "no-libcpp",
    llvm::cl::desc("Do not use libcpp intrinsics (__cxa_throw, etc.)"),
    llvm::cl::cat(ImportCategory));

static llvm::cl::opt< bool > AllowDebugInfoMismatch(
    "allow-dbg-mismatch",
    llvm::cl::desc("Allow incorrect debug information in the module"),
    llvm::cl::cat(ImportCategory));

/// @}
/// \name Passes options
/// @{

static llvm::cl::OptionCategory PassCategory("AR Passes Options");

static llvm::cl::opt< bool > NoTypeCheck(
    "no-type-check",
    llvm::cl::desc("Do not run the type checker"),
    llvm::cl::cat(PassCategory));

static llvm::cl::opt< bool > NoSimplifyCFG(
    "no-simplify-cfg",
    llvm::cl::desc("Do not run the simplify-cfg pass"),
    llvm::cl::cat(PassCategory));

static llvm::cl::opt< bool > AddLoopCounters(
    "add-loop-counters",
    llvm::cl::desc("Add a loop counter in each cycle"),
    llvm::cl::cat(PassCategory));

static llvm::cl::opt< bool > AddPartitioningVariables(
    "add-partitioning-variables",
    llvm::cl::desc("Add partitioning variable annotations on return variables"),
    llvm::cl::cat(PassCategory));

static llvm::cl::opt< bool > NameValues(
    "name-values",
    llvm::cl::desc("Give names to variables and basic blocks"),
    llvm::cl::cat(PassCategory));

static llvm::cl::opt< bool > NoNamePrefix(
    "no-name-prefix",
    llvm::cl::desc("Do not prefix variable names (if -name-values)"),
    llvm::cl::cat(PassCategory));

static llvm::cl::opt< bool > NoSimplifyUpcastComparison(
    "no-simplify-upcast-comparison",
    llvm::cl::desc("Do not simplify the implicit upcast before a comparison"),
    llvm::cl::cat(PassCategory));

/// @}
/// \name Debug options
/// @{

static llvm::cl::OptionCategory DebugCategory(
    "Debug Options", "Options to print analysis informations");

static llvm::cl::opt< bool > DisplayLiveness(
    "display-liveness",
    llvm::cl::desc("Display liveness analysis results"),
    llvm::cl::cat(DebugCategory));

static llvm::cl::opt< bool > DisplayFunctionPointer(
    "display-function-pointer",
    llvm::cl::desc("Display function pointer analysis results"),
    llvm::cl::cat(DebugCategory));

static llvm::cl::opt< bool > DisplayPointer(
    "display-pointer",
    llvm::cl::desc("Display pointer analysis results"),
    llvm::cl::cat(DebugCategory));

static llvm::cl::opt< bool > DisplayFixpointParameters(
    "display-fixpoint-parameters",
    llvm::cl::desc("Display fixpoint parameters"),
    llvm::cl::cat(DebugCategory));

static llvm::cl::opt< bool > DisplayAR(
    "display-ar",
    llvm::cl::desc("Display the Abstract Representation as text"),
    llvm::cl::cat(DebugCategory));

static llvm::cl::opt< bool > TraceARStmts(
    "trace-ar-stmts",
    llvm::cl::desc("Trace Abstract Representation statements during analysis"),
    llvm::cl::cat(DebugCategory));

static llvm::cl::opt< bool > GenerateDot(
    "generate-dot",
    llvm::cl::desc("Generate a .dot file for each function"),
    llvm::cl::cat(DebugCategory));

static llvm::cl::opt< std::string > GenerateDotDirectory(
    "generate-dot-dir",
    llvm::cl::desc("Output directory for .dot files"),
    llvm::cl::value_desc("directory"),
    llvm::cl::init("."),
    llvm::cl::cat(DebugCategory));

static llvm::cl::opt< analyzer::DisplayOption > DisplayInvariants(
    "display-inv",
    llvm::cl::desc("Display computed invariants"),
    llvm::cl::values(clEnumValN(analyzer::DisplayOption::All,
                                "all",
                                "Display all invariants"),
                     clEnumValN(analyzer::DisplayOption::Fail,
                                "fail",
                                "Display invariants for failed checks"),
                     clEnumValN(analyzer::DisplayOption::None,
                                "no",
                                "Do not display invariants (default)")),
    llvm::cl::init(analyzer::DisplayOption::None),
    llvm::cl::cat(DebugCategory));

static llvm::cl::opt< analyzer::DisplayOption > DisplayChecks(
    "display-checks",
    llvm::cl::desc("Display checks"),
    llvm::cl::values(
        clEnumValN(analyzer::DisplayOption::All, "all", "Display all checks"),
        clEnumValN(analyzer::DisplayOption::Fail,
                   "fail",
                   "Display only failed checks"),
        clEnumValN(analyzer::DisplayOption::None,
                   "no",
                   "Do not display checks (default)")),
    llvm::cl::init(analyzer::DisplayOption::None),
    llvm::cl::cat(DebugCategory));

/// @}
/// \name Formatting options
/// @{

static llvm::cl::OptionCategory FormatCategory("Formatting Options");

static llvm::cl::opt< bool > NoShowResultType(
    "no-show-result-type",
    llvm::cl::desc("Do not show the result type of statements"),
    llvm::cl::cat(FormatCategory));

static llvm::cl::opt< bool > ShowOperandTypes(
    "show-operand-types",
    llvm::cl::desc("Show the operand types of statements"),
    llvm::cl::cat(FormatCategory));

static llvm::cl::opt< bool > OrderGlobals(
    "order-globals",
    llvm::cl::desc("Order global variables and functions by name"),
    llvm::cl::cat(FormatCategory));

/// @}

/// \brief Return true if colors are enabled
static bool colors_enabled() {
  // Use a function from llvm/lib/Support/WithColor.cpp so that the global
  // constructors are called, and the 'color' option is registered.
  llvm::WithColor x(llvm::outs(), llvm::HighlightColor::String);

  const llvm::StringMap< llvm::cl::Option* >& opts =
      llvm::cl::getRegisteredOptions();
  auto it = opts.find("color");
  ikos_assert_msg(it != opts.end(), "Option 'color' is not registered");
  auto opt =
      static_cast< llvm::cl::opt< llvm::cl::boolOrDefault >* >(it->second);
  if (opt->getValue() == llvm::cl::BOU_UNSET) {
    return llvm::sys::Process::StandardOutIsDisplayed();
  } else {
    return opt->getValue() == llvm::cl::BOU_TRUE;
  }
}

/// \brief Build LLVM to AR import options from command line arguments
static llvm_to_ar::Importer::ImportOptions make_import_options() {
  llvm_to_ar::Importer::ImportOptions opts;
  opts.set(llvm_to_ar::Importer::EnableLibIkos, !NoLibIkos);
  opts.set(llvm_to_ar::Importer::EnableLibc, !NoLibc);
  opts.set(llvm_to_ar::Importer::EnableLibcpp, !NoLibcpp);
  opts.set(llvm_to_ar::Importer::AllowMismatchDebugInfo,
           AllowDebugInfoMismatch);
  return opts;
}

/// \brief Build format options from command line arguments
static ar::Formatter::FormatOptions make_format_options() {
  ar::Formatter::FormatOptions opts;
  opts.set(ar::Formatter::ShowResultType, !NoShowResultType);
  opts.set(ar::Formatter::ShowOperandTypes, ShowOperandTypes);
  opts.set(ar::Formatter::OrderGlobals, OrderGlobals);
  return opts;
}

/// \brief Parse a list of function names and return a list of functions
static std::vector< ar::Function* > parse_function_names(
    const llvm::cl::list< std::string >& opt, ar::Bundle* bundle) {
  std::vector< ar::Function* > functions;

  if (std::find(opt.begin(), opt.end(), "*") != opt.end()) {
    // Wildcard: all functions
    std::copy_if(bundle->function_begin(),
                 bundle->function_end(),
                 std::back_inserter(functions),
                 [](ar::Function* fun) { return fun->is_definition(); });
    return functions;
  }

  for (std::string name : opt) {
    boost::trim(name);
    ar::Function* fun = bundle->function_or_null(name);

    if (fun == nullptr) {
      std::ostringstream buf;
      buf << "could not find function '" << name << "'";
      throw analyzer::ArgumentError(buf.str());
    }

    if (!fun->is_definition()) {
      std::ostringstream buf;
      buf << "missing implementation for function '" << name << "'";
      throw analyzer::ArgumentError(buf.str());
    }

    functions.push_back(fun);
  }

  return functions;
}

/// \brief Interpret an unsigned integer value in the string `str`
static unsigned stou(const std::string& str,
                     size_t* pos = nullptr,
                     int base = 10) {
  // NOLINTNEXTLINE(google-runtime-int)
  long long n = std::stoll(str, pos, base);

  if (n < 0 || n > std::numeric_limits< unsigned >::max()) {
    throw std::out_of_range("stou: out of range");
  }

  return static_cast< unsigned >(n);
}

/// \brief Parse a list of "function:unsigned" and return a map
static boost::container::flat_map< ar::Function*, unsigned >
parse_function_names_to_unsigned(const llvm::cl::list< std::string >& opt,
                                 ar::Bundle* bundle) {
  boost::container::flat_map< ar::Function*, unsigned > map;

  for (const auto& str : opt) {
    size_t colon = str.find(':');

    if (colon == std::string::npos) {
      std::ostringstream buf;
      buf << "could not find separator ':' in '" << str << "'";
      throw analyzer::ArgumentError(buf.str());
    }

    std::string name = str.substr(0, colon);
    boost::trim(name);

    ar::Function* fun = bundle->function_or_null(name);

    if (fun == nullptr) {
      std::ostringstream buf;
      buf << "could not find function '" << name << "'";
      throw analyzer::ArgumentError(buf.str());
    }

    std::string number_str = str.substr(colon + 1);
    boost::trim(number_str);
    unsigned number;

    try {
      number = stou(number_str, nullptr, 0);
    } catch (const std::invalid_argument&) {
      std::ostringstream buf;
      buf << "could not parse integer '" << number_str << "'";
      throw analyzer::ArgumentError(buf.str());
    } catch (const std::out_of_range&) {
      std::ostringstream buf;
      buf << "integer out of range '" << number_str << "'";
      throw analyzer::ArgumentError(buf.str());
    }

    map[fun] = number;
  }

  return map;
}

/// \brief Build analysis options from command line arguments
static analyzer::AnalysisOptions make_analysis_options(ar::Bundle* bundle) {
  return analyzer::AnalysisOptions{
      .analyses = {Analyses.begin(), Analyses.end()},
      .entry_points = parse_function_names(EntryPoints, bundle),
      .no_init_globals = parse_function_names(NoInitGlobals, bundle),
      .machine_int_domain = Domain,
      .procedural = Procedural,
      .num_threads = Jobs,
      .widening_strategy = WideningStrategy,
      .narrowing_strategy = NarrowingStrategy,
      .widening_delay = WideningDelay,
      .widening_delay_functions =
          parse_function_names_to_unsigned(WideningDelayFunctions, bundle),
      .widening_period = WideningPeriod,
      .narrowing_iterations =
          ((NarrowingIterations >= 0)
               ? boost::optional< unsigned >(NarrowingIterations)
               : boost::none),
      .use_liveness = !NoLiveness,
      .use_pointer = !NoPointer,
      .use_widening_hints = !NoWideningHints,
      .use_partitioning_domain = EnablePartitioningDomain,
      .use_fixpoint_cache = !NoFixpointCache,
      .use_checks = !NoChecks,
      .trace_ar_statements = TraceARStmts,
      .globals_init_policy = GlobalsInitPolicy,
      .progress = Progress,
      .display_invariants = DisplayInvariants,
      .display_checks = DisplayChecks,
      .hardware_addresses = {bundle, HardwareAddresses, HardwareAddressesFile},
      .argc = ((Argc >= 0) ? boost::optional< int >(Argc) : boost::none),
  };
}

/// \brief Generate a .dot file for each function in the given Bundle
static void generate_dot(ar::Bundle* bundle,
                         const boost::filesystem::path& directory) {
  boost::system::error_code err;
  ar::DotFormatter formatter(make_format_options());

  if (!boost::filesystem::exists(directory)) {
    if (!boost::filesystem::create_directories(directory, err)) {
      analyzer::log::error(directory.string() + ": " + err.message());
      return;
    }
  }
  if (!boost::filesystem::is_directory(directory, err)) {
    analyzer::log::error(directory.string() + ": not a directory");
    return;
  }

  for (auto it = bundle->function_begin(), et = bundle->function_end();
       it != et;
       ++it) {
    ar::Function* fun = *it;

    if (!fun->is_definition()) {
      continue;
    }

    std::string filename = fun->name() + ".dot";
    boost::filesystem::path filepath = directory / filename;
    analyzer::log::debug("Creating " +
                         ((directory == ".") ? filename : filepath.string()));
    boost::filesystem::ofstream output(filepath);

    if (!output.is_open()) {
      analyzer::log::error(filepath.string() + ": " + strerror(errno));
      return;
    }

    formatter.format(output, fun);
  }
}

/// \brief Main for ikos-analyzer
int main(int argc, char** argv) {
  llvm::InitLLVM x(argc, argv);

  // Program name
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  std::string progname = boost::filesystem::path(argv[0]).filename().string();

  // Enable debug stream buffering
  llvm::EnableDebugBuffering = true;

  // LLVM context
  llvm::LLVMContext llvm_context;

  /*
   * Parse parameters
   */

  const char* overview = "ikos-analyzer -- IKOS static analyzer";
  llvm::cl::ParseCommandLineOptions(argc, argv, overview);

  // Set log level
  analyzer::log::Level = LogLevel;

  // Enable colors, if asked
  analyzer::color::Enable = colors_enabled();

  try {
    // Initialize output database
    // This might throw DbError, see catch()
    analyzer::log::debug("Creating output database '" + OutputFilename + "'");
    analyzer::sqlite::DbConnection db(OutputFilename);
    db.set_journal_mode(analyzer::sqlite::JournalMode::Off);
    db.set_synchronous_flag(analyzer::sqlite::SynchronousFlag::Off);
    analyzer::OutputDatabase output_db(db);

    // Load the input module
    std::unique_ptr< llvm::Module > module = nullptr;
    {
      analyzer::log::debug("Loading LLVM bitcode");
      analyzer::ScopeTimerDatabase t(output_db.times, "ikos-analyzer.load-bc");
      llvm::SMDiagnostic err; // Error diagnostic
      module = llvm::parseIRFile(InputFilename, err, llvm_context);
      if (!module) {
        err.print(progname.c_str(), llvm::errs());
        return 2;
      }
    }

    // Immediately run the verifier to catch any problems
    if (!NoVerify) {
      analyzer::log::debug("Verifying integrity of LLVM bitcode");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.verify-bc");
      if (verifyModule(*module, &llvm::errs())) {
        llvm::errs() << progname << ": " << InputFilename
                     << ": error: input module is broken!\n";
        return 3;
      }
    }

    // Check for debug information in LLVM
    {
      analyzer::log::debug("Checking for debug information");
      if (!llvm_to_ar::has_debug_info(*module)) {
        // We warn but allow analysis to proceed.
        llvm::errs() << progname << ": " << InputFilename
                     << ": warning: llvm bitcode has no debug information\n";
      }
    }

    // AR context
    ar::Context ar_context;

    // Translate LLVM bitcode into AR
    // This might throw ImportError, see catch()
    ar::Bundle* bundle = nullptr;
    {
      analyzer::log::info("Translating LLVM bitcode to AR");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.llvm-to-ar");
      llvm_to_ar::Importer importer(ar_context);
      bundle = importer.import(*module, make_import_options());
    }

    // Run type checker
    if (!NoTypeCheck) {
      analyzer::log::debug("Running type verifier on AR");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.type-checker");
      if (!ar::TypeVerifier(/*all = */ true).verify(bundle, std::cerr)) {
        llvm::errs() << progname << ": " << InputFilename
                     << ": error: type checker\n";
        return 7;
      }
    }

    // Check for debug information in AR
    if (!ar::FrontendVerifier(/*all = */ true).verify(bundle, std::cerr)) {
      return 8;
    }

    // Simplify the control flow graph
    if (!NoSimplifyCFG) {
      analyzer::log::debug("Running simplify-cfg pass on AR");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.simplify-cfg");
      ar::SimplifyCFGPass().run(bundle);
    }

    // Add a loop counter in each cycle, for the Gauge domain
    if (AddLoopCounters) {
      analyzer::log::debug("Running add-loop-counters pass on AR");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.add-loop-counters");
      ar::AddLoopCountersPass().run(bundle);
    }

    // Add partitioning variable annotations, for the Partitioning domain
    if (AddPartitioningVariables) {
      analyzer::log::debug("Running add-partitioning-variables pass on AR");
      analyzer::ScopeTimerDatabase
          t(output_db.times, "ikos-analyzer.add-partitioning-variables");
      ar::AddPartitioningVariablesPass().run(bundle);
    }

    // Simplify upcast comparison loop
    if (!NoSimplifyUpcastComparison) {
      analyzer::log::debug("Running simplify-upcast-comparison pass on AR");
      analyzer::ScopeTimerDatabase
          t(output_db.times, "ikos-analyzer.simplify-upcast-comparison");
      ar::SimplifyUpcastComparisonPass().run(bundle);
    }

    // Name variables and basic block, for debugging purpose only
    if (NameValues) {
      analyzer::log::debug("Running name-values pass on AR");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.name-values");
      ar::NameValuesPass(!NoNamePrefix).run(bundle);
    }

    // Display the abstract representation
    if (DisplayAR) {
      analyzer::log::info("Printing Abstract Representation");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.display-ar");
      ar::TextFormatter formatter(make_format_options());
      formatter.format(analyzer::log::msg().stream(), bundle);
    }

    // Generate .dot files
    if (GenerateDot) {
      analyzer::log::info("Generating .dot files");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.generate-dot");
      generate_dot(bundle, GenerateDotDirectory.getValue());
    }

    // Save analysis options in the database
    analyzer::AnalysisOptions opts = make_analysis_options(bundle);
    opts.save(output_db.settings);

    // Initialize factories
    analyzer::MemoryFactory mem_factory;
    analyzer::VariableFactory var_factory(bundle);
    analyzer::LiteralFactory lit_factory(var_factory, bundle->data_layout());
    analyzer::CallContextFactory call_context_factory;

    // Fixpoint parameters
    analyzer::FixpointParameters fixpoint_parameters(opts);

    // Analysis context
    analyzer::Context ctx(bundle,
                          opts,
                          boost::filesystem::current_path(),
                          output_db,
                          mem_factory,
                          var_factory,
                          lit_factory,
                          call_context_factory,
                          fixpoint_parameters);

    // Run a liveness analysis
    //
    // The goal is to detect unused variables to speed up the following
    // analyses
    analyzer::LivenessAnalysis liveness(ctx);
    if (!NoLiveness) {
      analyzer::log::info("Running liveness analysis");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.liveness-analysis");
      liveness.run();
      ctx.liveness = &liveness;
    }
    if (DisplayLiveness) {
      liveness.dump(analyzer::log::msg().stream());
    }

    // Run a widening hint analysis
    //
    // This is used to detect widening hints, useful for other analyses
    if (!NoWideningHints) {
      analyzer::WideningHintAnalysis widening_hint(ctx);
      analyzer::log::info("Running widening hint analysis");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.widening-hint-analysis");
      widening_hint.run();
    }
    if (DisplayFixpointParameters) {
      fixpoint_parameters.dump(analyzer::log::msg().stream());
    }

    // Run a fast intraprocedural function pointer analysis
    //
    // The goal here is to get all function pointers so that we can analyse
    // precisely indirect calls in the following analyses
    analyzer::FunctionPointerAnalysis function_pointer(ctx);
    if (Procedural == analyzer::Procedural::Intraprocedural && !NoPointer) {
      analyzer::log::info("Running function pointer analysis");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.function-pointer-analysis");
      function_pointer.run();
      ctx.function_pointer = &function_pointer;
    }
    if (DisplayFunctionPointer) {
      function_pointer.dump(analyzer::log::msg().stream());
    }

    // Run a deep (still intraprocedural) pointer analysis
    //
    // That step uses the result of the previous function pointer analysis.
    analyzer::PointerAnalysis pointer(ctx, function_pointer);
    if (Procedural == analyzer::Procedural::Intraprocedural && !NoPointer) {
      analyzer::log::info("Running pointer analysis");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.pointer-analysis");
      pointer.run();
      ctx.pointer = &pointer;
    }
    if (DisplayPointer) {
      pointer.dump(analyzer::log::msg().stream());
    }

    // Final step, run a value analysis, and check properties on the results
    if (Procedural == analyzer::Procedural::Interprocedural) {
      analyzer::log::info("Running interprocedural value analysis");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.value-analysis");
      if (Jobs == 1) {
        analyzer::value::interprocedural::sequential::Analysis(ctx).run();
      } else {
        analyzer::value::interprocedural::concurrent::Analysis(ctx).run();
      }
    } else if (Procedural == analyzer::Procedural::Intraprocedural) {
      analyzer::log::info("Running intraprocedural value analysis");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.value-analysis");
      if (Jobs == 1) {
        analyzer::value::intraprocedural::sequential::Analysis(ctx).run();
      } else {
        analyzer::value::intraprocedural::concurrent::Analysis(ctx).run();
      }
    } else {
      ikos_unreachable("unreachable");
    }
    return 0;
  } catch (analyzer::sqlite::DbError& err) {
    llvm::errs() << progname << ": " << OutputFilename
                 << ": error: " << err.what() << "\n";
    return 1;
  } catch (llvm_to_ar::ImportError& err) {
    llvm::errs() << progname << ": " << InputFilename
                 << ": error: " << err.what() << "\n";
    return 5;
  } catch (std::exception& err) {
    // catch any std::exception, core::Exception or analyzer::Exception
    llvm::errs() << progname << ": " << InputFilename
                 << ": error: " << err.what() << "\n";
    return 9;
  }
}
