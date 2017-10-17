/*******************************************************************************
 *
 * ikos-pp -- LLVM bitcode Pre-Processor for static analysis
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
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

#include "llvm/ADT/StringSet.h"
#include "llvm/Bitcode/BitcodeWriterPass.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/LinkAllPasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO.h"

#include "passes/lower_cst_expr.hpp"
#include "passes/lower_select.hpp"
#include "passes/mark_internal_inline.hpp"
#include "passes/mark_no_return_functions.hpp"
#include "passes/name_values.hpp"
#include "passes/remove_printf_calls.hpp"
#include "passes/remove_unreachable_blocks.hpp"

static llvm::cl::opt< std::string > InputFilename(
    llvm::cl::Positional,
    llvm::cl::desc("<input LLVM bitcode file>"),
    llvm::cl::Required,
    llvm::cl::value_desc("filename"));

static llvm::cl::opt< std::string > OutputFilename(
    "o",
    llvm::cl::desc("Override output filename"),
    llvm::cl::init(""),
    llvm::cl::value_desc("filename"));

static llvm::cl::opt< std::string > AsmOutputFilename(
    "oll",
    llvm::cl::desc("Output analyzed bitcode"),
    llvm::cl::init(""),
    llvm::cl::value_desc("filename"));

static llvm::cl::opt< bool > OutputAssembly(
    "S", llvm::cl::desc("Write output as LLVM assembly"));

static llvm::cl::opt< std::string > DefaultDataLayout(
    "default-data-layout",
    llvm::cl::desc("Data layout string to use if not specified by module"),
    llvm::cl::init(""),
    llvm::cl::value_desc("layout-string"));

static llvm::cl::list< std::string > EntryPoints(
    "entry-points",
    llvm::cl::value_desc(""),
    llvm::cl::desc("List of entry points"),
    llvm::cl::CommaSeparated);

static llvm::cl::opt< bool > InlineAll("ikospp-inline-all",
                                       llvm::cl::desc("Inline all functions"),
                                       llvm::cl::init(false));

static llvm::cl::opt< bool > Verify(
    "ikospp-verify",
    llvm::cl::desc("Verify the module is well formed upon all transformations"),
    llvm::cl::init(false));

enum PPLevel { simple, full };

static llvm::cl::opt< PPLevel > PPLevel(
    "ikospp-level",
    llvm::cl::desc("Choose preprocessing level:"),
    llvm::cl::values(clEnumVal(simple, "Only passes required for correctness"),
                     clEnumVal(full, "Enable all optimizations")),
    llvm::cl::init(full));

// removes extension from filename if there is one
std::string getFileName(const std::string& str) {
  std::string filename = str;
  size_t lastdot = str.find_last_of(".");
  if (lastdot != std::string::npos)
    filename = str.substr(0, lastdot);
  return filename;
}

int main(int argc, char** argv) {
  llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);
  llvm::PrettyStackTraceProgram PSTP(argc, argv);

  // Enable debug stream buffering.
  llvm::EnableDebugBuffering = true;

  // Call llvm_shutdown() on exit
  llvm::llvm_shutdown_obj shutdown;

  // Global context
  llvm::LLVMContext context;

  /*
   * initialize
   */

  llvm::PassRegistry& Registry = *llvm::PassRegistry::getPassRegistry();

  llvm::initializeCore(Registry);
  llvm::initializeScalarOpts(Registry);
  llvm::initializeObjCARCOpts(Registry);
  llvm::initializeVectorization(Registry);
  llvm::initializeIPO(Registry);
  llvm::initializeAnalysis(Registry);
  llvm::initializeTransformUtils(Registry);
  llvm::initializeInstCombine(Registry);
  llvm::initializeInstrumentation(Registry);
  llvm::initializeTarget(Registry);
  llvm::initializeCodeGenPreparePass(Registry);
  llvm::initializeAtomicExpandPass(Registry);
  llvm::initializeUnreachableBlockElimLegacyPassPass(Registry);

  /*
   * parse parameters
   */

  llvm::cl::ParseCommandLineOptions(argc,
                                    argv,
                                    "IkosPP -- LLVM bitcode Pre-Processor for "
                                    "Static Analysis\n");

  llvm::SMDiagnostic err;
  std::unique_ptr< llvm::Module > module;
  std::unique_ptr< llvm::tool_output_file > output;
  std::unique_ptr< llvm::tool_output_file > asmOutput;

  module = llvm::parseIRFile(InputFilename, err, context);
  if (!module) {
    llvm::errs() << "error: bitcode was not properly read\n"
                 << err.getMessage() << "\n";
    return 3;
  }

  if (!OutputFilename.empty()) {
    std::error_code ec(errno, std::generic_category());
    output.reset(new llvm::tool_output_file(OutputFilename.c_str(),
                                            ec,
                                            (llvm::sys::fs::OpenFlags)8));
  }

  if (!AsmOutputFilename.empty()) {
    std::error_code ec(errno, std::generic_category());
    asmOutput.reset(new llvm::tool_output_file(AsmOutputFilename.c_str(),
                                               ec,
                                               (llvm::sys::fs::OpenFlags)8));
  }

  /*
   * run passes
   */

  llvm::legacy::PassManager pass_manager;

  if (PPLevel == simple) {
    // SSA (opt -mem2reg)
    pass_manager.add(llvm::createPromoteMemoryToRegisterPass());
  } else {
    // turn all functions internal so that we can apply some global
    // optimizations inline them if requested (opt -internalize)
    llvm::StringSet<> exclude_set;
    if (EntryPoints.empty()) {
      exclude_set.insert("main");
    } else {
      for (auto it = EntryPoints.begin(); it != EntryPoints.end(); ++it) {
        exclude_set.insert(it->c_str());
      }
    }
    pass_manager.add(
        llvm::createInternalizePass([=](const llvm::GlobalValue& gv) {
          return exclude_set.find(gv.getName()) != exclude_set.end();
        }));

    // kill unused internal global (opt -globaldce)
    // note: unfortunately, it removes some debug info about global variables
    pass_manager.add(llvm::createGlobalDCEPass());

    // remove unreachable blocks
    pass_manager.add(new ikos_pp::RemoveUnreachableBlocks());

    // global optimizations (opt -globalopt)
    pass_manager.add(llvm::createGlobalOptimizerPass());

    // SSA (opt -mem2reg)
    pass_manager.add(llvm::createPromoteMemoryToRegisterPass());

    // cleanup after SSA (opt -instcombine)
    // disabled, bad for static analysis
    // pass_manager.add(llvm::createInstructionCombiningPass());

    // (opt -simplifycfg)
    pass_manager.add(llvm::createCFGSimplificationPass());

    // break aggregates (opt -sroa)
    pass_manager.add(llvm::createSROAPass());

    // global value numbering and redundant load elimination (opt -gvn)
    // note: unfortunately, it removes some debug information
    pass_manager.add(llvm::createGVNPass());

    // cleanup after breaking aggregates (opt -instcombine)
    // (bad for static analysis)
    pass_manager.add(llvm::createInstructionCombiningPass());

    // global dead code elimination (opt -globaldce)
    pass_manager.add(llvm::createGlobalDCEPass());

    // (opt -simplifycfg)
    pass_manager.add(llvm::createCFGSimplificationPass());

    // (conditional) constant propagation always help analyzers
    // jump threading (opt -jump-threading)
    pass_manager.add(llvm::createJumpThreadingPass());

    // sparse conditional constant propagation (opt -sccp)
    pass_manager.add(llvm::createSCCPPass());

    // dead code elimination (opt -dce)
    pass_manager.add(llvm::createDeadCodeEliminationPass());

    // lower invoke's (opt -lowerinvoke)
    pass_manager.add(llvm::createLowerInvokePass());

    // cleanup after lowering invoke's (opt -simplifycfg)
    pass_manager.add(llvm::createCFGSimplificationPass());

    if (InlineAll) {
      pass_manager.add(new ikos_pp::MarkInternalInline());

      // inline always_inline functions (opt -always-inline)
      pass_manager.add(llvm::createAlwaysInlinerLegacyPass());

      // kill unused internal global (opt -globaldce)
      pass_manager.add(llvm::createGlobalDCEPass());
    }

    // remove unreachable blocks
    pass_manager.add(new ikos_pp::RemoveUnreachableBlocks());

    // dead instruction elimination (opt -die)
    pass_manager.add(llvm::createDeadInstEliminationPass());

    // canonical form for loops (opt -loop-simplify)
    pass_manager.add(llvm::createLoopSimplifyPass());

    // cleanup unnecessary blocks (opt -simplifycfg)
    pass_manager.add(llvm::createCFGSimplificationPass());

    // loop-closed SSA (opt -lcssa)
    pass_manager.add(llvm::createLCSSAPass());

    // loop invariant code motion (opt -licm)
    // pass disabled because of a segmentation fault in the LICM pass
    // see https://bugs.llvm.org/show_bug.cgi?id=27146
    // pass_manager.add(llvm::createLICMPass());

    // SSA (opt -mem2reg)
    pass_manager.add(llvm::createPromoteMemoryToRegisterPass());

    // dead loop elimination (opt -loop-deletion)
    pass_manager.add(llvm::createLoopDeletionPass());

    // cleanup unnecessary blocks (opt -simplifycfg)
    pass_manager.add(llvm::createCFGSimplificationPass());
  }

  /*
   * Here passes needed for correctness and/or avoid crashing the static
   * analyzer
   */

  // ensure one single exit point per function (opt -mergereturn)
  pass_manager.add(llvm::createUnifyFunctionExitNodesPass());

  // MarkNoReturnFunctions only insert unreachable instructions if
  // the function does not have an exit block.
  pass_manager.add(new ikos_pp::MarkNoReturnFunctions());

  // global dead code elimination (opt -globaldce)
  pass_manager.add(llvm::createGlobalDCEPass());

  // dead code elimination (opt -dce)
  pass_manager.add(llvm::createDeadCodeEliminationPass());

  // remove unreachable blocks also dead cycles
  pass_manager.add(new ikos_pp::RemoveUnreachableBlocks());

  // remove switch constructions (opt -lowerswitch)
  pass_manager.add(llvm::createLowerSwitchPass());

  // lower down atomic instructions (opt -loweratomic)
  pass_manager.add(llvm::createLowerAtomicPass());

  // lower constant expressions to instructions
  pass_manager.add(new ikos_pp::LowerCstExprPass());

  // dead code elimination (opt -dce)
  pass_manager.add(llvm::createDeadCodeEliminationPass());

  if (PPLevel == full) {
    // after lowering constant expressions we remove all
    // side-effect-free printf-like functions. This can trigger the
    // removal of global strings that only feed them.
    pass_manager.add(new ikos_pp::RemovePrintfCalls());

    // dead code elimination (opt -dce)
    pass_manager.add(llvm::createDeadCodeEliminationPass());

    // global dead code elimination (opt -globaldce)
    pass_manager.add(llvm::createGlobalDCEPass());
  }

  // must be the last ones
  pass_manager.add(new ikos_pp::LowerSelect());
  pass_manager.add(new ikos_pp::NameValues());

  if (Verify) {
    pass_manager.add(llvm::createVerifierPass());
  }

  if (!AsmOutputFilename.empty()) {
    pass_manager.add(llvm::createPrintModulePass(
        llvm::cast< llvm::raw_ostream >(asmOutput->os())));
  }

  if (!OutputFilename.empty()) {
    if (OutputAssembly) {
      pass_manager.add(
          createPrintModulePass(llvm::cast< llvm::raw_ostream >(output->os())));
    } else {
      pass_manager.add(createBitcodeWriterPass(
          llvm::cast< llvm::raw_ostream >(output->os())));
    }
  }

  pass_manager.run(*module);

  if (!AsmOutputFilename.empty()) {
    asmOutput->keep();
  }
  if (!OutputFilename.empty()) {
    output->keep();
  }
  return 0;
}
