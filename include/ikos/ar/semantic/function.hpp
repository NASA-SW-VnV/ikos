/*******************************************************************************
 *
 * \file
 * \brief Function definition
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

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <boost/iterator/transform_iterator.hpp>

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/context.hpp>
#include <ikos/ar/semantic/intrinsic.hpp>
#include <ikos/ar/semantic/type.hpp>
#include <ikos/ar/semantic/value.hpp>
#include <ikos/ar/support/iterator.hpp>
#include <ikos/ar/support/traceable.hpp>

namespace ikos {
namespace ar {

// forward declaration
class Code;

/// \brief Function
class Function : public Traceable {
private:
  // Parent bundle
  Bundle* _parent;

  // Name
  std::string _name;

  // Type
  FunctionType* _type;

  // Intrinsic ID
  Intrinsic::ID _intrinsic_id;

  // Body (if definition)
  std::unique_ptr< Code > _body;

  // List of local variables (if definition)
  std::vector< std::unique_ptr< LocalVariable > > _local_vars;

  // List of parameters (if definition)
  std::vector< InternalVariable* > _parameters;

public:
  /// \brief Iterator over a list of local variables
  using LocalVariableIterator = boost::transform_iterator<
      SeqExposeRawPtr< LocalVariable >,
      std::vector< std::unique_ptr< LocalVariable > >::const_iterator >;

  /// \brief Iterator over a list of internal variables
  using InternalVariableIterator =
      std::vector< InternalVariable* >::const_iterator;

private:
  /// \brief Private constructor
  Function(Bundle* bundle,
           FunctionType* type,
           std::string name,
           bool is_definition,
           Intrinsic::ID intrinsic_id);

public:
  /// \brief No copy constructor
  Function(const Function&) = delete;

  /// \brief No move constructor
  Function(Function&&) = delete;

  /// \brief No copy assignment operator
  Function& operator=(const Function&) = delete;

  /// \brief No move assignment operator
  Function& operator=(Function&&) = delete;

  /// \brief Destructor
  ~Function();

  /// \brief Static constructor
  static Function* create(Bundle* bundle,
                          FunctionType* type,
                          std::string name,
                          bool is_definition,
                          Intrinsic::ID intrinsic_id = Intrinsic::NotIntrinsic);

  /// \brief Get parent context
  Context& context() const { return this->bundle()->context(); }

  /// \brief Get parent bundle
  Bundle* bundle() const { return this->_parent; }

  /// \brief Get the name
  const std::string& name() const { return this->_name; }

  /// \brief Set the name
  void set_name(std::string new_name);

  /// \brief Get the function type
  FunctionType* type() const { return this->_type; }

  /// \brief Is this a declaration?
  bool is_declaration() const { return this->_body == nullptr; }

  /// \brief Is this a definition?
  bool is_definition() const { return this->_body != nullptr; }

  /// \brief Is it a variable argument (var-arg) function?
  bool is_var_arg() const { return this->_type->is_var_arg(); }

  /// \brief Is it an intrinsic function?
  bool is_intrinsic() const {
    return this->_intrinsic_id != Intrinsic::NotIntrinsic;
  }

  /// \brief Is it an ikos intrinsic function?
  bool is_ikos_intrinsic() const {
    return this->_intrinsic_id >= Intrinsic::_BeginIkosIntrinsic &&
           this->_intrinsic_id <= Intrinsic::_EndIkosIntrinsic;
  }

  /// \brief Is it a libc intrinsic function?
  bool is_libc_intrinsic() const {
    return this->_intrinsic_id >= Intrinsic::_BeginLibcIntrinsic &&
           this->_intrinsic_id <= Intrinsic::_EndLibcIntrinsic;
  }

  /// \brief Is it a libc++ intrinsic function
  bool is_libcpp_intrinsic() const {
    return this->_intrinsic_id >= Intrinsic::_BeginLibcppIntrinsic &&
           this->_intrinsic_id <= Intrinsic::_EndLibcppIntrinsic;
  }

  /// \brief Get the intrinsic ID
  Intrinsic::ID intrinsic_id() const { return this->_intrinsic_id; }

  /// \brief Get the function body code
  Code* body() const {
    ikos_assert_msg(this->_body, "function is a declaration");
    return this->_body.get();
  }

  /// \brief Get the function body code, or null if it's a declaration
  Code* body_or_null() const { return this->_body.get(); }

  /// \brief Begin iterator over the list of local variables
  LocalVariableIterator local_variable_begin() const {
    ikos_assert_msg(this->_body, "function is a declaration");
    return boost::make_transform_iterator(this->_local_vars.cbegin(),
                                          SeqExposeRawPtr< LocalVariable >());
  }

  /// \brief End iterator over the list of local variables
  LocalVariableIterator local_variable_end() const {
    ikos_assert_msg(this->_body, "function is a declaration");
    return boost::make_transform_iterator(this->_local_vars.cend(),
                                          SeqExposeRawPtr< LocalVariable >());
  }

  /// \brief Get the i-th parameter variable
  InternalVariable* param(std::size_t i) const {
    ikos_assert_msg(i < this->_parameters.size(), "invalid index");
    return this->_parameters[i];
  }

  /// \brief Begin iterator over the list of parameter variables
  InternalVariableIterator param_begin() const {
    ikos_assert_msg(this->_body, "function is a declaration");
    return this->_parameters.cbegin();
  }

  /// \brief End iterator over the list of parameter variables
  InternalVariableIterator param_end() const {
    ikos_assert_msg(this->_body, "function is a declaration");
    return this->_parameters.cend();
  }

  /// \brief Get the number of parameters
  std::size_t num_parameters() const { return this->_type->num_parameters(); }

  /// \brief Get a function pointer on this function
  FunctionPointerConstant* pointer() {
    return FunctionPointerConstant::get(this->context(), this);
  }

private:
  /// \brief Add a local variable in the function
  ///
  /// \returns a pointer on the local variable
  LocalVariable* add_local_variable(std::unique_ptr< LocalVariable >);

  // friends
  friend class LocalVariable;

}; // end class Function

} // end namespace ar
} // end namespace ikos
