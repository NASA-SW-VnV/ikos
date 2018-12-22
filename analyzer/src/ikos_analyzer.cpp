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
 * Copyright (c) 2011-2018 United States Government as represented by the
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

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>

#include <ikos/ar/format/dot.hpp>
#include <ikos/ar/format/formatter.hpp>
#include <ikos/ar/format/text.hpp>
#include <ikos/ar/pass/add_loop_counters.hpp>
#include <ikos/ar/pass/name_values.hpp>
#include <ikos/ar/pass/simplify_cfg.hpp>
#include <ikos/ar/pass/simplify_upcast_comparison.hpp>
#include <ikos/ar/pass/unify_exit_nodes.hpp>
#include <ikos/ar/verify/frontend.hpp>
#include <ikos/ar/verify/type.hpp>

#include <ikos/frontend/llvm/import.hpp>

#include <ikos/analyzer/analysis/call_context.hpp>
#include <ikos/analyzer/analysis/context.hpp>
#include <ikos/analyzer/analysis/fixpoint_profile.hpp>
#include <ikos/analyzer/analysis/hardware_addresses.hpp>
#include <ikos/analyzer/analysis/literal.hpp>
#include <ikos/analyzer/analysis/liveness.hpp>
#include <ikos/analyzer/analysis/memory_location.hpp>
#include <ikos/analyzer/analysis/option.hpp>
#include <ikos/analyzer/analysis/pointer/function.hpp>
#include <ikos/analyzer/analysis/pointer/pointer.hpp>
#include <ikos/analyzer/analysis/result.hpp>
#include <ikos/analyzer/analysis/value/interprocedural.hpp>
#include <ikos/analyzer/analysis/value/intraprocedural.hpp>
#include <ikos/analyzer/analysis/variable.hpp>
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
    llvm::cl::value_desc("filename"));

static llvm::cl::opt< std::string > OutputFilename(
    "o",
    llvm::cl::desc("Output database filename (default: output.db)"),
    llvm::cl::value_desc("filename"),
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

enum class ColorOpt { Yes, No, Auto };

static llvm::cl::opt< ColorOpt > Color(
    "color",
    llvm::cl::desc("Enable terminal colors:"),
    llvm::cl::values(
        clEnumValN(ColorOpt::Yes, "yes", "Enable colors"),
        clEnumValN(ColorOpt::Auto,
                   "auto",
                   "Enable colors if the output is a terminal (default)"),
        clEnumValN(ColorOpt::No, "no", "Disable colors")),
    llvm::cl::init(ColorOpt::Auto),
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
                   checker_long_name(analyzer::CheckerName::DoubleFree))),
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

static llvm::cl::opt< bool > NoLiveness(
    "no-liveness",
    llvm::cl::desc("Disable the liveness analysis"),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< bool > NoPointer(
    "no-pointer",
    llvm::cl::desc("Disable the pointer analysis"),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< bool > NoFixpointProfiles(
    "no-fixpoint-profiles",
    llvm::cl::desc("Disable the fixpoint profiles analysis"),
    llvm::cl::cat(AnalysisCategory));

static llvm::cl::opt< analyzer::Precision > Precision(
    "prec",
    llvm::cl::desc("Precision level:"),
    llvm::cl::values(
        clEnumValN(analyzer::Precision::Register,
                   precision_str(analyzer::Precision::Register),
                   "Only track immediate values"),
        clEnumValN(analyzer::Precision::Pointer,
                   precision_str(analyzer::Precision::Pointer),
                   "Track immediate values and pointers"),
        clEnumValN(analyzer::Precision::Memory,
                   precision_str(analyzer::Precision::Memory),
                   "Track immediate values, pointers and memory (default)")),
    llvm::cl::init(analyzer::Precision::Memory),
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
    "disable-type-check",
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

static llvm::cl::opt< bool > DisplayFixpointProfiles(
    "display-fixpoint-profiles",
    llvm::cl::desc("Display fixpoint profiles analysis results"),
    llvm::cl::cat(DebugCategory));

static llvm::cl::opt< bool > DisplayAR(
    "display-ar",
    llvm::cl::desc("Display the Abstract Representation as text"),
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

/// \brief Given a range of function names, return an iterator on the first
/// undefined function, or the end iterator otherwise
template < typename Iterator >
inline Iterator find_undefined_functions(Iterator begin,
                                         Iterator end,
                                         ar::Bundle* bundle) {
  return std::find_if(begin, end, [=](const auto& name) {
    return bundle->function_or_null(name) == nullptr;
  });
}

/// \brief Build analysis options from command line arguments
static analyzer::AnalysisOptions make_analysis_options(ar::Bundle* bundle) {
  auto resolve_function = [=](const auto& name) {
    return bundle->function_or_null(name);
  };

  return analyzer::AnalysisOptions{
      .analyses = {Analyses.begin(), Analyses.end()},
      .entry_points = {boost::make_transform_iterator(EntryPoints.begin(),
                                                      resolve_function),
                       boost::make_transform_iterator(EntryPoints.end(),
                                                      resolve_function)},
      .no_init_globals = {boost::make_transform_iterator(NoInitGlobals.begin(),
                                                         resolve_function),
                          boost::make_transform_iterator(NoInitGlobals.end(),
                                                         resolve_function)},
      .machine_int_domain = Domain,
      .procedural = Procedural,
      .use_liveness = !NoLiveness,
      .use_pointer = !NoPointer,
      .precision = Precision,
      .globals_init_policy = GlobalsInitPolicy,
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
    analyzer::log::error(directory.string() + ": Not a directory");
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
  llvm::InitLLVM X(argc, argv);

  // Program name
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
  analyzer::color::Enable =
      (Color == ColorOpt::Yes ||
       (Color == ColorOpt::Auto && analyzer::log::out_isatty()));

  try {
#ifndef NDEBUG
    analyzer::log::warning(
        "ikos was compiled in debug mode, the analysis might be slow");
#endif

    // Initialize output database
    // This might throw DbError, see catch()
    analyzer::log::debug("Creating output database " + OutputFilename);
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
    {
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
        llvm::errs() << progname << ": " << InputFilename
                     << ": error: llvm bitcode has no debug information\n";
        return 4;
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

    // Check that EntryPoints and NoInitGlobals have valid function names
    {
      auto it = find_undefined_functions(EntryPoints.begin(),
                                         EntryPoints.end(),
                                         bundle);
      if (it != EntryPoints.end()) {
        llvm::errs() << progname << ": " << InputFilename
                     << ": error: could not find function '" << *it << "'\n";
        return 6;
      }

      it = find_undefined_functions(NoInitGlobals.begin(),
                                    NoInitGlobals.end(),
                                    bundle);
      if (it != NoInitGlobals.end()) {
        llvm::errs() << progname << ": " << InputFilename
                     << ": error: could not find function '" << *it << "'\n";
        return 6;
      }
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

    // Simplify upcast comparison loop
    if (!NoSimplifyUpcastComparison) {
      analyzer::log::debug("Running simplify-upcast-comparison pass on AR");
      analyzer::ScopeTimerDatabase
          t(output_db.times, "ikos-analyzer.simplify-upcast-comparison");
      ar::SimplifyUpcastComparisonPass().run(bundle);
    }

    // Unify all exit nodes
    {
      analyzer::log::debug("Running unify-exit-nodes pass on AR");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.unify-exit-nodes");
      ar::UnifyExitNodesPass().run(bundle);
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
      formatter.format(analyzer::log::out(), bundle);
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

    // Analysis context
    analyzer::Context ctx(bundle,
                          opts,
                          boost::filesystem::current_path(),
                          output_db,
                          mem_factory,
                          var_factory,
                          lit_factory,
                          call_context_factory);

    // First, run a liveness analysis
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
      liveness.dump(analyzer::log::out());
    }

    // Run the fixpoint profile analysis
    //
    // This is used to detect widening hints, useful for other analyses
    analyzer::FixpointProfileAnalysis profiler(ctx);
    if (!NoFixpointProfiles) {
      analyzer::log::info("Running fixpoint profile analysis");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.fixpoint-profile-analysis");
      profiler.run();
      ctx.fixpoint_profiler = &profiler;
    }
    if (DisplayFixpointProfiles) {
      profiler.dump(analyzer::log::out());
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
      function_pointer.dump(analyzer::log::out());
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
      pointer.dump(analyzer::log::out());
    }

    // Final step, run a value analysis, and check properties on the results
    if (Procedural == analyzer::Procedural::Interprocedural) {
      analyzer::InterproceduralValueAnalysis analysis(ctx);
      analyzer::log::info("Running interprocedural value analysis");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.value-analysis");
      analysis.run();
    } else if (Procedural == analyzer::Procedural::Intraprocedural) {
      analyzer::IntraproceduralValueAnalysis analysis(ctx);
      analyzer::log::info("Running intraprocedural value analysis");
      analyzer::ScopeTimerDatabase t(output_db.times,
                                     "ikos-analyzer.value-analysis");
      analysis.run();
    } else {
      ikos_unreachable("unreachable");
    }
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
