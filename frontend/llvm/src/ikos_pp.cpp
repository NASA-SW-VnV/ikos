/*******************************************************************************
 *
 * ikos-pp -- LLVM bitcode Pre-Processor for Static Analysis
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
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

#include <boost/filesystem.hpp>

#include <llvm/ADT/StringSet.h>
#include <llvm/Bitcode/BitcodeWriterPass.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LegacyPassNameParser.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/InitializePasses.h>
#include <llvm/LinkAllPasses.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO.h>

#include <ikos/core/support/assert.hpp>

#include <ikos/frontend/llvm/pass.hpp>

namespace ikos_pp = ikos::frontend::pass;

static llvm::cl::opt< std::string > InputFilename(
    llvm::cl::Positional,
    llvm::cl::desc("<input bitcode file>"),
    llvm::cl::Required,
    llvm::cl::value_desc("filename"));

static llvm::cl::opt< std::string > OutputFilename(
    "o",
    llvm::cl::desc("Override output filename"),
    llvm::cl::value_desc("filename"));

static llvm::cl::opt< bool > OutputAssembly(
    "S", llvm::cl::desc("Write output as LLVM assembly"));

static llvm::cl::list< std::string > EntryPoints(
    "entry-points",
    llvm::cl::desc("List of program entry points"),
    llvm::cl::CommaSeparated,
    llvm::cl::value_desc("function"));

static llvm::cl::opt< bool > InlineAll("inline-all",
                                       llvm::cl::desc("Inline all functions"));

static llvm::cl::opt< bool > NoVerify(
    "no-verify", llvm::cl::desc("Do not run the LLVM bitcode verifier"));

static llvm::cl::opt< bool > DiscardValueNames(
    "discard-value-names",
    llvm::cl::desc("Discard names from Value (other than GlobalValue)."),
    llvm::cl::init(false),
    llvm::cl::Hidden);

static llvm::cl::opt< bool > PreserveBitcodeUseListOrder(
    "preserve-bc-uselistorder",
    llvm::cl::desc("Preserve use-list order when writing LLVM bitcode."),
    llvm::cl::init(true),
    llvm::cl::Hidden);

static llvm::cl::opt< bool > PreserveAssemblyUseListOrder(
    "preserve-ll-uselistorder",
    llvm::cl::desc("Preserve use-list order when writing LLVM assembly."),
    llvm::cl::init(false),
    llvm::cl::Hidden);

enum OptLevelType { None, Basic, Aggressive, Custom };

static llvm::cl::opt< OptLevelType > OptLevel(
    "opt",
    llvm::cl::desc("Optimization level:"),
    llvm::cl::values(
        clEnumValN(None,
                   "none",
                   "Only passes required for the translation to AR"),
        clEnumValN(Basic, "basic", "Basic set of optimizations (recommended)"),
        clEnumValN(Aggressive,
                   "aggressive",
                   "Aggressive optimizations (not recommended)"),
        clEnumValN(Custom, "custom", "Use a custom set of llvm passes")),
    llvm::cl::init(Basic));

/// \brief Set of custom passes
///
/// Automatically populated with the registered Passes by the PassNameParser
static llvm::cl::list< const llvm::PassInfo*, bool, llvm::PassNameParser >
    CustomPassList(llvm::cl::desc("Custom Optimizations available:"));

/// \brief Main for ikos-pp
int main(int argc, char** argv) {
  llvm::InitLLVM x(argc, argv);

  // Program name
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  std::string progname = boost::filesystem::path(argv[0]).filename().string();

  // Enable debug stream buffering
  llvm::EnableDebugBuffering = true;

  // Global context
  llvm::LLVMContext context;

  /*
   * Initialize and register all passes
   */

  llvm::PassRegistry& registry = *llvm::PassRegistry::getPassRegistry();
  llvm::initializeCore(registry);
  llvm::initializeCoroutines(registry);
  llvm::initializeScalarOpts(registry);
  llvm::initializeObjCARCOpts(registry);
  llvm::initializeVectorization(registry);
  llvm::initializeIPO(registry);
  llvm::initializeAnalysis(registry);
  llvm::initializeTransformUtils(registry);
  llvm::initializeInstCombine(registry);
  llvm::initializeAggressiveInstCombine(registry);
  llvm::initializeInstrumentation(registry);
  llvm::initializeTarget(registry);
  llvm::initializeExpandMemCmpPassPass(registry);
  llvm::initializeCodeGenPreparePass(registry);
  llvm::initializeAtomicExpandPass(registry);
  llvm::initializeRewriteSymbolsLegacyPassPass(registry);
  llvm::initializeWinEHPreparePass(registry);
  llvm::initializeSafeStackLegacyPassPass(registry);
  llvm::initializeSjLjEHPreparePass(registry);
  llvm::initializeStackProtectorPass(registry);
  llvm::initializePreISelIntrinsicLoweringLegacyPassPass(registry);
  llvm::initializeGlobalMergePass(registry);
  llvm::initializeIndirectBrExpandPassPass(registry);
  llvm::initializeInterleavedLoadCombinePass(registry);
  llvm::initializeInterleavedAccessPass(registry);
  llvm::initializeEntryExitInstrumenterPass(registry);
  llvm::initializePostInlineEntryExitInstrumenterPass(registry);
  llvm::initializeUnreachableBlockElimLegacyPassPass(registry);
  llvm::initializeExpandReductionsPass(registry);
  llvm::initializeWasmEHPreparePass(registry);
  llvm::initializeWriteBitcodePassPass(registry);
  llvm::initializeHardwareLoopsPass(registry);
  ikos_pp::initialize_ikos_passes(registry);

  /*
   * Parse parameters
   */

  const char* overview =
      "ikos-pp -- LLVM bitcode Pre-Processor for Static Analysis\n";
  llvm::cl::ParseCommandLineOptions(argc, argv, overview);

  // Error diagnostic
  llvm::SMDiagnostic err;

  // If -discard-value-names, discard all the names (except for GlobalValue)
  context.setDiscardValueNames(DiscardValueNames);

  // Load the input module
  std::unique_ptr< llvm::Module > module =
      llvm::parseIRFile(InputFilename, err, context);
  if (!module) {
    err.print(progname.c_str(), llvm::errs());
    return 1;
  }

  // Immediately run the verifier to catch any problems
  if (!NoVerify && verifyModule(*module, &llvm::errs())) {
    llvm::errs() << progname << ": " << InputFilename
                 << ": error: input module is broken!\n";
    return 1;
  }

  // Default to standard output
  if (OutputFilename.empty()) {
    OutputFilename = "-";
  }

  // Output stream
  std::error_code ec;
  std::unique_ptr< llvm::ToolOutputFile > output =
      std::make_unique< llvm::ToolOutputFile >(OutputFilename,
                                               ec,
                                               llvm::sys::fs::OF_None);
  if (ec) {
    llvm::errs() << progname << ": " << ec.message() << '\n';
    return 1;
  }

  /*
   * Build a PassManager
   */

  llvm::legacy::PassManager pass_manager;

  if (OptLevel == None) {
    // Remove switch constructions (opt -lowerswitch)
    pass_manager.add(llvm::createLowerSwitchPass());

    // Lower down atomic instructions (opt -loweratomic)
    pass_manager.add(llvm::createLowerAtomicPass());

    // Lower constant expressions to instructions (ikos-pp -lower-cst-expr)
    pass_manager.add(ikos_pp::create_lower_cst_expr_pass());

    // Lower down select instructions (ikos-pp -lower-select)
    pass_manager.add(ikos_pp::create_lower_select_pass());

    // Ensure one single exit point per function (opt -mergereturn)
    pass_manager.add(llvm::createUnifyFunctionExitNodesPass());
  } else if (OptLevel == Basic) {
    // SSA (opt -mem2reg)
    pass_manager.add(llvm::createPromoteMemoryToRegisterPass());

    // Global dead code elimination (opt -globaldce)
    // note: unfortunately, it removes some debug info about global variables
    pass_manager.add(llvm::createGlobalDCEPass());

    // Dead code elimination (opt -dce)
    pass_manager.add(llvm::createDeadCodeEliminationPass());

    // Remove switch constructions (opt -lowerswitch)
    pass_manager.add(llvm::createLowerSwitchPass());

    // Remove unreachable blocks also dead cycles
    pass_manager.add(ikos_pp::create_remove_unreachable_blocks_pass());

    // Lower down atomic instructions (opt -loweratomic)
    pass_manager.add(llvm::createLowerAtomicPass());

    // Lower constant expressions to instructions (ikos-pp -lower-cst-expr)
    pass_manager.add(ikos_pp::create_lower_cst_expr_pass());

    // Dead code elimination (opt -dce)
    pass_manager.add(llvm::createDeadCodeEliminationPass());

    // Lower down select instructions (ikos-pp -lower-select)
    pass_manager.add(ikos_pp::create_lower_select_pass());

    // Ensure one single exit point per function (opt -mergereturn)
    pass_manager.add(llvm::createUnifyFunctionExitNodesPass());
  } else if (OptLevel == Aggressive) {
    // Turn all functions internal so that we can apply some global
    // optimizations inline them if requested (opt -internalize)
    llvm::StringSet<> exclude_set;
    if (EntryPoints.empty()) {
      exclude_set.insert("main");
    } else {
      for (const auto& entry_point : EntryPoints) {
        exclude_set.insert(entry_point);
      }
    }
    if (exclude_set.count("*") == 0) {
      pass_manager.add(
          llvm::createInternalizePass([=](const llvm::GlobalValue& gv) {
            return exclude_set.find(gv.getName()) != exclude_set.end();
          }));
    }

    // Kill unused internal global (opt -globaldce)
    // note: unfortunately, it removes some debug info about global variables
    pass_manager.add(llvm::createGlobalDCEPass());

    // Remove unreachable blocks
    pass_manager.add(ikos_pp::create_remove_unreachable_blocks_pass());

    // Global optimizations (opt -globalopt)
    pass_manager.add(llvm::createGlobalOptimizerPass());

    // SSA (opt -mem2reg)
    pass_manager.add(llvm::createPromoteMemoryToRegisterPass());

    // Cleanup after SSA (opt -instcombine)
    // disabled, bad for static analysis
    // pass_manager.add(llvm::createInstructionCombiningPass());

    // Simplification (opt -simplifycfg)
    pass_manager.add(llvm::createCFGSimplificationPass());

    // Break aggregates (opt -sroa)
    pass_manager.add(llvm::createSROAPass());

    // Global value numbering and redundant load elimination (opt -gvn)
    // note: unfortunately, it removes some debug information
    pass_manager.add(llvm::createGVNPass());

    // Cleanup after breaking aggregates (opt -instcombine)
    // (bad for static analysis)
    pass_manager.add(llvm::createInstructionCombiningPass());

    // Global dead code elimination (opt -globaldce)
    pass_manager.add(llvm::createGlobalDCEPass());

    // Simplification (opt -simplifycfg)
    pass_manager.add(llvm::createCFGSimplificationPass());

    // Jump threading (opt -jump-threading)
    // (conditional) constant propagation always help analyzers
    pass_manager.add(llvm::createJumpThreadingPass());

    // Sparse conditional constant propagation (opt -sccp)
    pass_manager.add(llvm::createSCCPPass());

    // Dead code elimination (opt -dce)
    pass_manager.add(llvm::createDeadCodeEliminationPass());

    // Lower invoke's (opt -lowerinvoke)
    pass_manager.add(llvm::createLowerInvokePass());

    // Cleanup after lowering invoke's (opt -simplifycfg)
    pass_manager.add(llvm::createCFGSimplificationPass());

    if (InlineAll) {
      // Mark all functions always_inline (ikos-pp -mark-internal-inline)
      pass_manager.add(ikos_pp::create_mark_internal_inline_pass());

      // Inline always_inline functions (opt -always-inline)
      pass_manager.add(llvm::createAlwaysInlinerLegacyPass());

      // Kill unused internal global (opt -globaldce)
      pass_manager.add(llvm::createGlobalDCEPass());
    }

    // Remove unreachable blocks
    pass_manager.add(ikos_pp::create_remove_unreachable_blocks_pass());

    // Dead code elimination (opt -dce)
    pass_manager.add(llvm::createDeadCodeEliminationPass());

    // Canonical form for loops (opt -loop-simplify)
    pass_manager.add(llvm::createLoopSimplifyPass());

    // Cleanup unnecessary blocks (opt -simplifycfg)
    pass_manager.add(llvm::createCFGSimplificationPass());

    // Loop-closed SSA (opt -lcssa)
    pass_manager.add(llvm::createLCSSAPass());

    // Loop invariant code motion (opt -licm)
    pass_manager.add(llvm::createLICMPass());

    // SSA (opt -mem2reg)
    pass_manager.add(llvm::createPromoteMemoryToRegisterPass());

    // Dead loop elimination (opt -loop-deletion)
    pass_manager.add(llvm::createLoopDeletionPass());

    // Cleanup unnecessary blocks (opt -simplifycfg)
    pass_manager.add(llvm::createCFGSimplificationPass());

    // Global dead code elimination (opt -globaldce)
    pass_manager.add(llvm::createGlobalDCEPass());

    // Dead code elimination (opt -dce)
    pass_manager.add(llvm::createDeadCodeEliminationPass());

    // Remove unreachable blocks also dead cycles
    pass_manager.add(ikos_pp::create_remove_unreachable_blocks_pass());

    // Remove switch constructions (opt -lowerswitch)
    pass_manager.add(llvm::createLowerSwitchPass());

    // Lower down atomic instructions (opt -loweratomic)
    pass_manager.add(llvm::createLowerAtomicPass());

    // Lower constant expressions to instructions (ikos-pp -lower-cst-expr)
    pass_manager.add(ikos_pp::create_lower_cst_expr_pass());

    // Dead code elimination (opt -dce)
    pass_manager.add(llvm::createDeadCodeEliminationPass());

    // After lowering constant expressions we remove all
    // side-effect-free printf-like functions. This can trigger the
    // removal of global strings that only feed them.
    pass_manager.add(ikos_pp::create_remove_printf_calls_pass());

    // Dead code elimination (opt -dce)
    pass_manager.add(llvm::createDeadCodeEliminationPass());

    // Global dead code elimination (opt -globaldce)
    pass_manager.add(llvm::createGlobalDCEPass());

    // Lower down select instructions (ikos-pp -lower-select)
    pass_manager.add(ikos_pp::create_lower_select_pass());

    // Ensure one single exit point per function (opt -mergereturn)
    pass_manager.add(llvm::createUnifyFunctionExitNodesPass());
  } else {
    ikos_assert(OptLevel == Custom);

    for (std::size_t i = 0; i < CustomPassList.size(); ++i) {
      const llvm::PassInfo* pass_info = CustomPassList[i];

      if (pass_info->getNormalCtor() != nullptr) {
        llvm::Pass* pass = pass_info->getNormalCtor()();
        pass_manager.add(pass);
      } else {
        llvm::errs() << progname
                     << ": cannot create pass: " << pass_info->getPassName()
                     << "\n";
      }
    }
  }

  // Check that the module is well formed on completion of optimization
  if (!NoVerify) {
    pass_manager.add(llvm::createVerifierPass());
  }

  // Output pass
  if (OutputAssembly) {
    pass_manager.add(llvm::createPrintModulePass(output->os(),
                                                 "",
                                                 PreserveAssemblyUseListOrder));
  } else {
    pass_manager.add(
        createBitcodeWriterPass(output->os(), PreserveBitcodeUseListOrder));
  }

  // Run all the passes
  pass_manager.run(*module);

  output->keep();

  return 0;
}
