/*******************************************************************************
 *
 * \file
 * \brief Implementation of Bundle
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

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/code.hpp>
#include <ikos/ar/semantic/context.hpp>
#include <ikos/ar/semantic/data_layout.hpp>
#include <ikos/ar/semantic/function.hpp>
#include <ikos/ar/semantic/value.hpp>
#include <ikos/ar/support/assert.hpp>

#include "context_impl.hpp"

namespace ikos {
namespace ar {

Bundle::Bundle(Context& ctx,
               std::unique_ptr< DataLayout > data_layout,
               std::string triple)
    : _context(ctx),
      _data_layout(std::move(data_layout)),
      _target_triple(std::move(triple)) {}

Bundle::~Bundle() = default;

Bundle* Bundle::create(Context& ctx,
                       std::unique_ptr< DataLayout > data_layout,
                       std::string triple) {
  Bundle* bundle = new Bundle(ctx, std::move(data_layout), std::move(triple));
  ctx._impl->add_bundle(std::unique_ptr< Bundle >(bundle));
  return bundle;
}

Function* Bundle::intrinsic_function(Intrinsic::ID id) {
  std::string name = Intrinsic::long_name(id);

  Function* fun = this->_functions.find(name);
  if (fun != nullptr) {
    return fun;
  }

  ar::FunctionType* type = Intrinsic::type(this, id);
  return Function::create(this, type, name, /*is_definition = */ false, id);
}

Function* Bundle::intrinsic_function(Intrinsic::ID id, Type* template_ty) {
  std::string name = Intrinsic::long_name(id, template_ty);

  Function* fun = this->_functions.find(name);
  if (fun != nullptr) {
    return fun;
  }

  ar::FunctionType* type = Intrinsic::type(this, id, template_ty);
  return Function::create(this, type, name, /*is_definition = */ false, id);
}

bool Bundle::is_name_available(const std::string& name) const {
  return !this->_globals.contains(name) && !this->_functions.contains(name);
}

std::string Bundle::find_available_name(StringRef prefix) const {
  ikos_assert_msg(!prefix.empty(), "empty prefix");

  // Add a numerical suffix, e.g, ".1"
  std::string name = prefix.to_string();
  name += ".";
  std::size_t idx = 1;

  while (!this->is_name_available(name + std::to_string(idx))) {
    ++idx;
  }

  name += std::to_string(idx);
  return name;
}

void Bundle::add_global_variable(std::unique_ptr< GlobalVariable > gv) {
  this->_globals.add(std::move(gv));
}

void Bundle::rename_global_variable(GlobalVariable* gv,
                                    const std::string& prev_name,
                                    const std::string& new_name) {
  this->_globals.rename(gv, prev_name, new_name);
}

void Bundle::add_function(std::unique_ptr< Function > fun) {
  this->_functions.add(std::move(fun));
}

void Bundle::rename_function(Function* fun,
                             const std::string& prev_name,
                             const std::string& new_name) {
  this->_functions.rename(fun, prev_name, new_name);
}

} // end namespace ar
} // end namespace ikos
