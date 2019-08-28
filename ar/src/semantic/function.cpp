/*******************************************************************************
 *
 * \file
 * \brief Function implementation
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
#include <ikos/ar/semantic/function.hpp>
#include <ikos/ar/semantic/value.hpp>
#include <ikos/ar/support/assert.hpp>

namespace ikos {
namespace ar {

Function::Function(Bundle* bundle,
                   FunctionType* type,
                   std::string name,
                   bool is_definition,
                   Intrinsic::ID intrinsic_id)
    : _parent(bundle),
      _type(type),
      _intrinsic_id(intrinsic_id),
      _body(nullptr) {
  ikos_assert_msg(bundle, "bundle is null");
  ikos_assert_msg(type, "type is null");
  ikos_assert_msg(!name.empty(), "function name is empty");
  ikos_assert_msg(intrinsic_id == Intrinsic::NotIntrinsic || !is_definition,
                  "intrinsic function cannot be a definition");

  if (!name.empty() && bundle->is_name_available(name)) {
    this->_name = std::move(name);
  } else {
    this->_name = bundle->find_available_name(name);
  }

  if (is_definition) {
    // Create function body
    this->_body = std::unique_ptr< Code >(new Code(this));

    // Create unnamed parameters
    _parameters.reserve(type->num_parameters());
    for (auto it = type->param_begin(), et = type->param_end(); it != et;
         ++it) {
      InternalVariable* param =
          InternalVariable::create(this->_body.get(), *it);
      _parameters.push_back(param);
    }
  }
}

Function::~Function() = default;

Function* Function::create(Bundle* bundle,
                           FunctionType* type,
                           std::string name,
                           bool is_definition,
                           Intrinsic::ID intrinsic_id) {
  auto fun =
      new Function(bundle, type, std::move(name), is_definition, intrinsic_id);
  bundle->add_function(std::unique_ptr< Function >(fun));
  return fun;
}

void Function::set_name(std::string new_name) {
  // In this case, we need to notify the bundle,
  // because it keeps a map from name to functions
  ikos_assert_msg(!new_name.empty(), "name is empty");
  std::string prev_name = this->name();

  if (!new_name.empty() && this->_parent->is_name_available(new_name)) {
    this->_name = std::move(new_name);
  } else {
    this->_name = this->_parent->find_available_name(new_name);
  }

  this->_parent->rename_function(this, prev_name, this->_name);
}

LocalVariable* Function::add_local_variable(
    std::unique_ptr< LocalVariable > lv) {
  this->_local_vars.emplace_back(std::move(lv));
  return this->_local_vars.back().get();
}

} // end namespace ar
} // end namespace ikos
