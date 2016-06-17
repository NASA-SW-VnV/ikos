/*******************************************************************************
 *
 * ikos-pp -- LLVM bitcode Pre-Processor for static analysis
 *
 * Author: Jorge A. Navas
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
#include "passes/lower_gv_initializers.hpp"
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
    llvm::cl::desc("data layout string to use if not specified by module"),
    llvm::cl::init(""),
    llvm::cl::value_desc("layout-string"));

static llvm::cl::opt< bool > InlineAll("ikospp-inline-all",
                                       llvm::cl::desc("Inline all functions"),
                                       llvm::cl::init(false));

static llvm::cl::opt< int > SROA_Threshold(
    "sroa-threshold",
    llvm::cl::desc("Threshold for ScalarReplAggregates pass"),
    llvm::cl::init(INT_MAX));

static llvm::cl::opt< bool > Verify(
    "ikospp-verify",
    llvm::cl::desc("Verify the module is well formed upon all transformations"),
    llvm::cl::init(false));

enum PPLevel { simple, full };

static llvm::cl::opt< PPLevel > PPLevel(
    "ikospp-level",
    cl::desc("Choose preprocessing level:"),
    cl::values(clEnumVal(simple, "Only passes required for correctness"),
               clEnumVal(full, "Enable all optimizations"),
               clEnumValEnd),
    cl::init(full));

// removes extension from filename if there is one
std::string getFileName(const std::string& str) {
  std::string filename = str;
  size_t lastdot = str.find_last_of(".");
  if (lastdot != std::string::npos)
    filename = str.substr(0, lastdot);
  return filename;
}

int main(int argc, char** argv) {
  llvm::llvm_shutdown_obj shutdown; // calls llvm_shutdown() on exit
  llvm::cl::ParseCommandLineOptions(argc,
                                    argv,
                                    "IkosPP -- LLVM bitcode Pre-Processor for "
                                    "Static Analysis\n");

  llvm::sys::PrintStackTraceOnErrorSignal();
  llvm::PrettyStackTraceProgram PSTP(argc, argv);
  llvm::EnableDebugBuffering = true;

  std::string error_msg;
  llvm::SMDiagnostic err;
  llvm::LLVMContext& context = llvm::getGlobalContext();
  std::unique_ptr< llvm::Module > module;
  std::unique_ptr< llvm::tool_output_file > output;
  std::unique_ptr< llvm::tool_output_file > asmOutput;

  module = llvm::parseIRFile(InputFilename, err, context);
  if (module.get() == nullptr) {
    llvm::errs() << "error: "
                 << "Bitcode was not properly read; " << err.getMessage()
                 << "\n";
    return 3;
  }

  if (!OutputFilename.empty()) {
    std::error_code ec(errno, std::generic_category());
    output.reset(new llvm::tool_output_file(OutputFilename.c_str(),
                                            ec,
                                            (llvm::sys::fs::OpenFlags)8));
  }

  if (!error_msg.empty()) {
    llvm::errs() << "error: " << error_msg << "\n";
    return 3;
  }

  if (!AsmOutputFilename.empty()) {
    std::error_code ec(errno, std::generic_category());
    asmOutput.reset(new llvm::tool_output_file(AsmOutputFilename.c_str(),
                                               ec,
                                               (llvm::sys::fs::OpenFlags)8));
  }

  if (!error_msg.empty()) {
    llvm::errs() << "error: " << error_msg << "\n";
    return 3;
  }

  ///////////////////////////////
  // initialise and run passes //
  ///////////////////////////////

  llvm::legacy::PassManager pass_manager;
  llvm::PassRegistry& Registry = *llvm::PassRegistry::getPassRegistry();

  llvm::initializeAnalysis(Registry);

#if (LLVM_VERSION_MAJOR == 3) && (LLVM_VERSION_MINOR == 7)
  /// call graph and other IPA passes
  llvm::initializeIPA(Registry);
#endif

  if (PPLevel == simple) {
    // SSA
    pass_manager.add(llvm::createPromoteMemoryToRegisterPass());
  } else {
    // turn all functions internal so that we can apply some global
    // optimizations inline them if requested
    const char* excluded_list[] = {"main"};
    pass_manager.add(llvm::createInternalizePass(
        excluded_list)); // internalize all excluding main
    pass_manager.add(
        llvm::createGlobalDCEPass()); // kill unused internal global
    pass_manager.add(new ikos_pp::RemoveUnreachableBlocks());

    // global optimizations
    pass_manager.add(llvm::createGlobalOptimizerPass());

    // SSA
    pass_manager.add(llvm::createPromoteMemoryToRegisterPass());

    // cleanup after SSA
    // pass_manager.add (llvm::createInstructionCombiningPass ()); // bad for
    // static analysis
    pass_manager.add(llvm::createCFGSimplificationPass());

    // break aggregates
    pass_manager.add(
        llvm::createScalarReplAggregatesPass(SROA_Threshold, true));

    // global value numbering and redundant load elimination
    pass_manager.add(llvm::createGVNPass());

    // cleanup after break aggregates
    pass_manager.add(
        llvm::createInstructionCombiningPass()); // bad for static analysis

    pass_manager.add(llvm::createGlobalDCEPass());

    pass_manager.add(llvm::createCFGSimplificationPass());

    // (conditional) constant propagation always help analyzers
    pass_manager.add(llvm::createJumpThreadingPass());
    pass_manager.add(llvm::createSCCPPass());
    pass_manager.add(llvm::createDeadCodeEliminationPass());
    // pass_manager.add (llvm::createGlobalDCEPass ());
    // pass_manager.add (llvm::createGlobalOptimizerPass());

    // lower invoke's
    pass_manager.add(llvm::createLowerInvokePass());

    // cleanup after lowering invoke's
    pass_manager.add(llvm::createCFGSimplificationPass());

    if (InlineAll) {
      pass_manager.add(new ikos_pp::MarkInternalInline());
      pass_manager.add(llvm::createAlwaysInlinerPass());
      pass_manager.add(
          llvm::createGlobalDCEPass()); // kill unused internal global
    }

    pass_manager.add(new ikos_pp::RemoveUnreachableBlocks());
    pass_manager.add(llvm::createDeadInstEliminationPass());

    // canonical form for loops
    pass_manager.add(llvm::createLoopSimplifyPass());
    pass_manager.add(
        llvm::createCFGSimplificationPass()); // cleanup unnecessary blocks

    // loop-closed SSA
    pass_manager.add(llvm::createLCSSAPass());

    // trivial invariants outside loops
#if (LLVM_VERSION_MAJOR == 3) && (LLVM_VERSION_MINOR == 7)
    pass_manager.add(llvm::createBasicAliasAnalysisPass());
#else
    pass_manager.add(llvm::createBasicAAWrapperPass());
#endif
    pass_manager.add(llvm::createLICMPass()); // LICM needs alias analysis
    pass_manager.add(llvm::createPromoteMemoryToRegisterPass());

    // dead loop elimination
    pass_manager.add(llvm::createLoopDeletionPass());
    pass_manager.add(
        llvm::createCFGSimplificationPass()); // cleanup unnecessary blocks

    // lower initializers of global variables
    pass_manager.add(new ikos_pp::LowerGvInitializers());
  }

  ////
  // Here passes needed for correctness and/or avoid crashing the static analyzer
  ////

  // ensure one single exit point per function
  pass_manager.add(llvm::createUnifyFunctionExitNodesPass());

  // MarkNoReturnFunctions only insert unreachable instructions if
  // the function does not have an exit block.
  pass_manager.add(new ikos_pp::MarkNoReturnFunctions());

  // We then perform DCE
  pass_manager.add(llvm::createGlobalDCEPass());
  pass_manager.add(llvm::createDeadCodeEliminationPass());

  // remove unreachable blocks also dead cycles
  pass_manager.add(new ikos_pp::RemoveUnreachableBlocks());

  // remove switch constructions
  pass_manager.add(llvm::createLowerSwitchPass());

  // lower down atomic instructions
  pass_manager.add(llvm::createLowerAtomicPass());

  // lower constant expressions to instructions
  pass_manager.add(new ikos_pp::LowerCstExprPass());
  pass_manager.add(llvm::createDeadCodeEliminationPass());

  if (PPLevel == full) {
    // after lowering constant expressions we remove all
    // side-effect-free printf-like functions. This can trigger the
    // removal of global strings that only feed them.
    pass_manager.add(new ikos_pp::RemovePrintfCalls());
    pass_manager.add(llvm::createDeadCodeEliminationPass());
    pass_manager.add(llvm::createGlobalDCEPass());
  }

  // must be the last ones:
  pass_manager.add(new ikos_pp::LowerSelect());
  pass_manager.add(new ikos_pp::NameValues());

  if (Verify)
    pass_manager.add(llvm::createVerifierPass());

  if (!AsmOutputFilename.empty())
    pass_manager.add(llvm::createPrintModulePass(
        cast< llvm::raw_ostream >(asmOutput->os())));

  if (!OutputFilename.empty()) {
    if (OutputAssembly)
      pass_manager.add(
          createPrintModulePass(cast< llvm::raw_ostream >(output->os())));
    else
      pass_manager.add(
          createBitcodeWriterPass(cast< llvm::raw_ostream >(output->os())));
  }

  pass_manager.run(*module.get());

  if (!AsmOutputFilename.empty())
    asmOutput->keep();
  if (!OutputFilename.empty())
    output->keep();
  return 0;
}
