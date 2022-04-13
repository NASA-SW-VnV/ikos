/*******************************************************************************
 *
 * ikos-import -- Translate LLVM bitcode into AR
 *
 * Author: Maxime Arthaud
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

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>

#include <ikos/ar/format/dot.hpp>
#include <ikos/ar/format/text.hpp>
#include <ikos/ar/pass/simplify_cfg.hpp>
#include <ikos/ar/verify/type.hpp>

#include <ikos/frontend/llvm/import.hpp>

namespace ar = ikos::ar;
namespace llvm_to_ar = ikos::frontend::import;

static llvm::cl::opt< std::string > InputFilename(
    llvm::cl::Positional,
    llvm::cl::desc("<input bitcode file>"),
    llvm::cl::Required,
    llvm::cl::value_desc("filename"));

static llvm::cl::opt< std::string > OutputFilename(
    "o",
    llvm::cl::desc("Override output filename"),
    llvm::cl::value_desc("filename"));

static llvm::cl::opt< bool > NoVerify(
    "no-verify", llvm::cl::desc("Do not run the LLVM bitcode verifier"));

static llvm::cl::opt< bool > NoLibIkos(
    "no-libikos",
    llvm::cl::desc("Do not use ikos intrinsics (__ikos_assert, etc.)"));

static llvm::cl::opt< bool > NoLibc(
    "no-libc",
    llvm::cl::desc("Do not use libc intrinsics (malloc, free, etc.)"));

static llvm::cl::opt< bool > NoLibcpp(
    "no-libcpp",
    llvm::cl::desc("Do not use libcpp intrinsics (__cxa_throw, etc.)"));

static llvm::cl::opt< bool > AllowDebugInfoMismatch(
    "allow-dbg-mismatch",
    llvm::cl::desc("Allow incorrect debug information in the module"));

static llvm::cl::opt< bool > NoTypeCheck(
    "no-type-check", llvm::cl::desc("Do not run the AR type checker"));

static llvm::cl::opt< bool > NoSimplifyCFG(
    "no-simplify-cfg", llvm::cl::desc("Do not run the simplify-cfg pass"));

enum OutputFormatType { None, Text, Dot };

static llvm::cl::opt< OutputFormatType > OutputFormat(
    "format",
    llvm::cl::desc("Output format:"),
    llvm::cl::values(clEnumValN(None, "no", "Disable output"),
                     clEnumValN(Text, "text", "Text format"),
                     clEnumValN(Dot, "dot", "Dot format")),
    llvm::cl::init(Text));

static llvm::cl::opt< bool > NoShowResultType(
    "no-show-result-type",
    llvm::cl::desc("Do not show the result type of statements"));

static llvm::cl::opt< bool > ShowOperandTypes(
    "show-operand-types",
    llvm::cl::desc("Show the operand types of statements"));

static llvm::cl::opt< bool > OrderGlobals(
    "order-globals",
    llvm::cl::desc("Order global variables and functions by name"));

/// \brief Build import options from command line arguments
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

/// \brief Main for ikos-import
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

  const char* overview = "ikos-import -- Translate LLVM bitcode into AR";
  llvm::cl::ParseCommandLineOptions(argc, argv, overview);

  try {
    // Error diagnostic
    llvm::SMDiagnostic err;

    // Load the input module
    std::unique_ptr< llvm::Module > module =
        llvm::parseIRFile(InputFilename, err, llvm_context);
    if (!module) {
      err.print(progname.c_str(), llvm::errs());
      return 1;
    }

    // Immediately run the verifier to catch any problems
    if (!NoVerify && verifyModule(*module, &llvm::errs())) {
      llvm::errs() << progname << ": " << InputFilename
                   << ": error: input module is broken!\n";
      return 2;
    }

    // Check for debug information
    if (!llvm_to_ar::has_debug_info(*module)) {
      // Warn but allow to proceed.
      llvm::errs() << progname << ": " << InputFilename
                   << ": warning: input module has no debug information\n";
      llvm::errs() << "... Output may be less user friendly.\n";
    }

    // AR context
    ar::Context ar_context;

    // Translate LLVM bitcode into AR
    ar::Bundle* bundle = nullptr;
    try {
      llvm_to_ar::Importer importer(ar_context);
      bundle = importer.import(*module, make_import_options());
    } catch (llvm_to_ar::ImportError& err) {
      llvm::errs() << progname << ": " << InputFilename
                   << ": error: " << err.what() << "\n";
      return 3;
    }

    // Run type checker
    ar::TypeVerifier verifier(/*all = */ true);
    if (!NoTypeCheck && !verifier.verify(bundle, std::cerr)) {
      llvm::errs() << progname << ": " << InputFilename
                   << ": error: type checker\n";
      return 4;
    }

    // Simplify the control flow graph
    if (!NoSimplifyCFG) {
      ar::SimplifyCFGPass().run(bundle);
    }

    // Generate output
    if (OutputFormat == Text) {
      ar::TextFormatter formatter(make_format_options());

      if (OutputFilename.empty() || OutputFilename == "-") {
        // Default to standard output
        formatter.format(std::cout, bundle);
      } else {
        boost::filesystem::ofstream output(OutputFilename.getValue());

        if (!output.is_open()) {
          llvm::errs() << progname << ": " << OutputFilename << ": "
                       << strerror(errno) << "\n";
          return 5;
        }

        formatter.format(output, bundle);
      }
    } else if (OutputFormat == Dot) {
      ar::DotFormatter formatter(make_format_options());

      if (OutputFilename.empty()) {
        // Default to current directory
        OutputFilename = ".";
      }

      boost::system::error_code ec;
      boost::filesystem::path output_dir(OutputFilename.getValue());

      if (!boost::filesystem::exists(output_dir)) {
        if (!boost::filesystem::create_directories(output_dir, ec)) {
          llvm::errs() << progname << ": " << OutputFilename << ": "
                       << ec.message() << "\n";
          return 5;
        }
      }
      if (!boost::filesystem::is_directory(output_dir, ec)) {
        llvm::errs() << progname << ": " << OutputFilename
                     << ": Not a directory\n";
        return 5;
      }

      for (auto it = bundle->function_begin(), et = bundle->function_end();
           it != et;
           ++it) {
        ar::Function* fun = *it;

        if (!fun->is_definition()) {
          continue;
        }

        std::string filename = fun->name() + ".dot";

        std::cout << "creating " << filename;
        if (OutputFilename != ".") {
          std::cout << " in directory " << OutputFilename;
        }
        std::cout << "\n";

        boost::filesystem::ofstream output(output_dir / filename);

        if (!output.is_open()) {
          llvm::errs() << progname << ": " << OutputFilename << "/" << filename
                       << ": " << strerror(errno) << "\n";
          return 5;
        }

        formatter.format(output, fun);
      }
    }

    return 0;
  } catch (std::exception& err) {
    llvm::errs() << progname << ": " << InputFilename
                 << ": error: " << err.what() << "\n";
    return 6;
  }
}
