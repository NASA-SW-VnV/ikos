/*******************************************************************************
 *
 * \file
 * \brief Helpers for global variables for the value analysis
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2019 United States Government as represented by the
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

#include <vector>

#include <ikos/core/support/compiler.hpp>

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/statement.hpp>
#include <ikos/ar/semantic/type.hpp>
#include <ikos/ar/semantic/value.hpp>

#include <ikos/analyzer/analysis/execution_engine/engine.hpp>
#include <ikos/analyzer/analysis/option.hpp>

namespace ikos {
namespace analyzer {
namespace value {

/// \brief Return true if the given global variable should be initialized,
/// according to the given policy
bool is_initialized(ar::GlobalVariable* gv, GlobalsInitPolicy policy);

/// \brief Return the global constructors, in call order, given the
/// ar.global_ctors variable
std::vector< std::pair< ar::Function*, MachineInt > > global_ctors(
    ar::GlobalVariable* gv);

/// \brief Return the global destructors, in call order, given the
/// ar.global_dtors variable
std::vector< std::pair< ar::Function*, MachineInt > > global_dtors(
    ar::GlobalVariable* gv);

/// \brief Call execution engine for global variable initializer
class GlobalVarCallExecutionEngine final : public CallExecutionEngine {
public:
  ikos_attribute_unused void exec_exit(ar::Function* /*fun*/) override {}

  ikos_attribute_noreturn void exec(ar::Call* /*call*/) override {
    ikos_unreachable("call statement in global variable initializer");
  }

  ikos_attribute_noreturn void exec(ar::Invoke* /*invoke*/) override {
    ikos_unreachable("invoke statement in global variable initializer");
  }

  ikos_attribute_noreturn void exec(ar::ReturnValue* /*ret*/) override {
    ikos_unreachable("return statement in global variable initializer");
  }

}; // end class GlobalVarCallExecutionEngine

} // end namespace value
} // end namespace analyzer
} // end namespace ikos
