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

#include <ikos/analyzer/analysis/value/global_variable.hpp>
#include <ikos/analyzer/support/cast.hpp>

namespace ikos {
namespace analyzer {
namespace value {

bool is_initialized(ar::GlobalVariable* gv, GlobalsInitPolicy policy) {
  switch (policy) {
    case GlobalsInitPolicy::All: {
      // Initialize all global variables
      return true;
    }
    case GlobalsInitPolicy::SkipBigArrays: {
      // Initialize all global variables except arrays with more than 100
      // elements
      ar::Type* type = gv->type()->pointee();
      return !isa< ar::ArrayType >(type) ||
             cast< ar::ArrayType >(type)->num_elements() <= 100;
    }
    case GlobalsInitPolicy::SkipStrings: {
      // Initialize all global variables except strings ([n x si8]*)
      ar::Type* type = gv->type()->pointee();
      return !isa< ar::ArrayType >(type) ||
             cast< ar::ArrayType >(type)->element_type() !=
                 ar::IntegerType::si8(gv->context());
    }
    case GlobalsInitPolicy::None: {
      // Do not initialize any global variable
      return false;
    }
    default: {
      ikos_unreachable("unreachable");
    }
  }
}

/// \brief Return the list of pair (function, priority) for arrays
/// ar.global_ctors or ar.global_dtors, given the global variable
static std::vector< std::pair< ar::Function*, MachineInt > > global_cdtors(
    ar::GlobalVariable* gv) {
  if (gv == nullptr || gv->is_declaration()) {
    return {};
  }

  ar::BasicBlock* bb = gv->initializer()->entry_block();

  if (bb->empty() || !isa< ar::Store >(bb->back())) {
    return {};
  }

  auto store = cast< ar::Store >(bb->back());

  if (store->pointer() != gv || !isa< ar::ArrayConstant >(store->value())) {
    return {};
  }

  auto cst = cast< ar::ArrayConstant >(store->value());
  std::vector< std::pair< ar::Function*, MachineInt > > entries;

  for (ar::Value* element : cst->values()) {
    if (!isa< ar::StructConstant >(element)) {
      continue;
    }

    auto e = cast< ar::StructConstant >(element);

    if (e->num_fields() != 3) {
      continue;
    }

    auto it = e->field_begin();
    ar::Value* fst = it->value;
    ++it;
    ar::Value* snd = it->value;

    if (!isa< ar::IntegerConstant >(fst) ||
        !isa< ar::FunctionPointerConstant >(snd)) {
      continue;
    }

    const MachineInt& priority = cast< ar::IntegerConstant >(fst)->value();
    ar::Function* fun = cast< ar::FunctionPointerConstant >(snd)->function();
    entries.emplace_back(fun, priority);
  }

  return entries;
}

std::vector< std::pair< ar::Function*, MachineInt > > global_ctors(
    ar::GlobalVariable* gv) {
  auto entries = global_cdtors(gv);
  std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
    return a.second < b.second;
  });
  return entries;
}

std::vector< std::pair< ar::Function*, MachineInt > > global_dtors(
    ar::GlobalVariable* gv) {
  auto entries = global_cdtors(gv);
  std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
    return a.second > b.second;
  });
  return entries;
}

} // end namespace value
} // end namespace analyzer
} // end namespace ikos
