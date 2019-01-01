/*******************************************************************************
 *
 * \file
 * \brief Implementation of NameValuesPass
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017-2019 United States Government as represented by the
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

#include <ikos/ar/format/namer.hpp>
#include <ikos/ar/pass/name_values.hpp>

namespace ikos {
namespace ar {

const char* NameValuesPass::name() const {
  return "name-values";
}

const char* NameValuesPass::description() const {
  return "Name variables and basic blocks";
}

bool NameValuesPass::run_on_code(Code* code) {
  // Generate all the names
  Namer namer(code);

  // Prefix to add, if requested
  std::string prefix;
  if (code->is_function_body()) {
    prefix = code->function()->name() + ".";
  } else {
    prefix = code->global_var()->name() + ".";
  }

  // Assign the generated names
  if (code->is_function_body()) {
    Function* f = code->function();

    // Name local variables
    for (auto it = f->local_variable_begin(), et = f->local_variable_end();
         it != et;
         ++it) {
      LocalVariable* v = *it;

      if (!v->has_name()) {
        v->set_name(namer.name(v));
      }
      if (this->_prefix) {
        v->set_name(prefix + v->name());
      }
    }
  }

  // Name basic blocks
  for (auto it = code->begin(), et = code->end(); it != et; ++it) {
    BasicBlock* bb = *it;

    if (!bb->has_name()) {
      bb->set_name(namer.name(bb));
    }
  }

  // Name internal variables
  for (auto it = code->internal_variable_begin(),
            et = code->internal_variable_end();
       it != et;
       ++it) {
    InternalVariable* v = *it;

    if (!v->has_name()) {
      v->set_name(namer.name(v));
    }
    if (this->_prefix) {
      v->set_name(prefix + v->name());
    }
  }

  return false; // the semantic hasn't changed
}

} // end namespace ar
} // end namespace ikos
