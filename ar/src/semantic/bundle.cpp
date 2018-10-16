/**************************************************************************/ /**
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
 * Copyright (c) 2017-2018 United States Government as represented by the
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
      _target_triple(std::move(triple)) {
  ikos_assert_msg(!this->_target_triple.empty(), "empty target triple");
}

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

  auto it = this->_functions.find(name);
  if (it != this->_functions.end()) {
    return it->second.get();
  } else {
    ar::FunctionType* type = Intrinsic::type(this, id);
    return Function::create(this, type, name, /*is_definition = */ false, id);
  }
}

bool Bundle::is_name_available(const std::string& name) const {
  return this->_globals.find(name) == this->_globals.end() &&
         this->_functions.find(name) == this->_functions.end();
}

std::string Bundle::find_available_name(StringRef prefix) const {
  std::string name = prefix.to_string();

  if (name.empty()) {
    name += "g";
  }

  if (this->is_name_available(name)) {
    return name;
  }

  // Add a numerical suffix, e.g, ".1"
  name += ".";
  std::size_t idx = 1;

  while (!this->is_name_available(name + std::to_string(idx))) {
    ++idx;
  }

  name += std::to_string(idx);
  return name;
}

void Bundle::add_global_variable(std::unique_ptr< GlobalVariable > gv) {
  ikos_assert_msg(!gv->name().empty(), "name is empty");
  ikos_assert_msg(this->is_name_available(gv->name()), "name already taken");
  this->_globals.emplace(gv->name(), std::move(gv));
}

void Bundle::rename_global_variable(GlobalVariable* /*v*/,
                                    const std::string& prev_name,
                                    const std::string& new_name) {
  ikos_assert_msg(!new_name.empty(), "name is empty");
  ikos_assert_msg(this->is_name_available(new_name), "name already taken");
  std::unique_ptr< GlobalVariable > gv_ptr =
      std::move(this->_globals.at(prev_name));
  this->_globals.erase(prev_name);
  this->_globals.emplace(new_name, std::move(gv_ptr));
}

void Bundle::add_function(std::unique_ptr< Function > f) {
  ikos_assert_msg(!f->name().empty(), "name is empty");
  ikos_assert_msg(this->is_name_available(f->name()), "name already taken");
  this->_functions.emplace(f->name(), std::move(f));
}

void Bundle::rename_function(Function* /*f*/,
                             const std::string& prev_name,
                             const std::string& new_name) {
  ikos_assert_msg(!new_name.empty(), "name is empty");
  ikos_assert_msg(this->is_name_available(new_name), "name already taken");
  std::unique_ptr< Function > fun_ptr =
      std::move(this->_functions.at(prev_name));
  this->_functions.erase(prev_name);
  this->_functions.emplace(new_name, std::move(fun_ptr));
}

} // end namespace ar
} // end namespace ikos
