/*******************************************************************************
 *
 * \file
 * \brief Global analysis context
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

#pragma once

#include <boost/filesystem.hpp>

#include <ikos/ar/semantic/bundle.hpp>

#include <ikos/analyzer/analysis/option.hpp>

namespace ikos {
namespace analyzer {

// forward declarations
class OutputDatabase;
class MemoryFactory;
class VariableFactory;
class LiteralFactory;
class CallContextFactory;
class LivenessAnalysis;
class FunctionPointerAnalysis;
class PointerAnalysis;
class FixpointParameters;

/// \brief Global analysis context
///
/// Class holding all the global state of the analyses
class Context {
public:
  /// \brief Input program
  ar::Bundle* bundle;

  /// \brief Analysis options
  const AnalysisOptions opts;

  /// \brief Working directory
  boost::filesystem::path wd;

  /// \brief Output database
  OutputDatabase* output_db;

  /// \brief Memory location factory
  MemoryFactory* mem_factory;

  /// \brief Variable factory
  VariableFactory* var_factory;

  /// \brief Literal factory
  LiteralFactory* lit_factory;

  /// \brief Call context factory
  CallContextFactory* call_context_factory;

  /// \brief Fixpoint parameters
  FixpointParameters* fixpoint_parameters;

  /// \brief Liveness analysis, or null
  LivenessAnalysis* liveness;

  /// \brief Function pointer analysis, or null
  FunctionPointerAnalysis* function_pointer;

  /// \brief Pointer analysis, or null
  PointerAnalysis* pointer;

public:
  /// \brief Constructor
  Context(ar::Bundle* bundle_,
          AnalysisOptions opts_,
          boost::filesystem::path wd_,
          OutputDatabase& output_db_,
          MemoryFactory& mem_factory_,
          VariableFactory& var_factory_,
          LiteralFactory& lit_factory_,
          CallContextFactory& call_context_factory_,
          FixpointParameters& fixpoint_parameters_)
      : bundle(bundle_),
        opts(std::move(opts_)),
        wd(std::move(wd_)),
        output_db(&output_db_),
        mem_factory(&mem_factory_),
        var_factory(&var_factory_),
        lit_factory(&lit_factory_),
        call_context_factory(&call_context_factory_),
        fixpoint_parameters(&fixpoint_parameters_),
        liveness(nullptr),
        function_pointer(nullptr),
        pointer(nullptr) {}

  /// \brief No copy constructor
  Context(const Context&) = delete;

  /// \brief No move constructor
  Context(Context&&) = delete;

  /// \brief No copy assignment operator
  Context& operator=(const Context&) = delete;

  /// \brief No move assignment operator
  Context& operator=(Context&&) = delete;

  /// \brief Destructor
  ~Context() = default;

}; // end class Context

} // end namespace analyzer
} // end namespace ikos
