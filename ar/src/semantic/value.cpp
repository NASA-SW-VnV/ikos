/*******************************************************************************
 *
 * \file
 * \brief Value implementation (variables, constants, etc.)
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

#include <ikos/ar/semantic/code.hpp>
#include <ikos/ar/semantic/context.hpp>
#include <ikos/ar/semantic/function.hpp>
#include <ikos/ar/semantic/value.hpp>
#include <ikos/ar/support/assert.hpp>

#include "context_impl.hpp"

namespace ikos {
namespace ar {

// Value

Value::Value(ValueKind kind, Type* type) : _kind(kind), _type(type) {
  ikos_assert_msg(type, "type is null");
}

Value::~Value() = default;

ContextImpl& Value::ctx_impl(Context& ctx) {
  return *(ctx._impl);
}

// Constant

Constant::Constant(ValueKind kind, Type* type) : Value(kind, type) {}

// UndefinedConstant

UndefinedConstant::UndefinedConstant(Type* type)
    : Constant(UndefinedConstantKind, type) {}

UndefinedConstant* UndefinedConstant::get(Context& ctx, Type* type) {
  return ctx_impl(ctx).undefined_cst(type);
}

void UndefinedConstant::dump(std::ostream& o) const {
  o << "undef";
}

// IntegerConstant

IntegerConstant::IntegerConstant(IntegerType* type, MachineInt value)
    : Constant(IntegerConstantKind, type), _value(std::move(value)) {
  ikos_assert(this->_value.bit_width() == type->bit_width());
  ikos_assert(this->_value.sign() == type->sign());
}

IntegerConstant* IntegerConstant::get(Context& ctx,
                                      IntegerType* type,
                                      const MachineInt& value) {
  return ctx_impl(ctx).integer_cst(type, value);
}

IntegerConstant* IntegerConstant::get(Context& ctx,
                                      IntegerType* type,
                                      int value) {
  return ctx_impl(ctx)
      .integer_cst(type, MachineInt(value, type->bit_width(), type->sign()));
}

void IntegerConstant::dump(std::ostream& o) const {
  o << this->_value;
}

// FloatConstant

FloatConstant::FloatConstant(FloatType* type, std::string value)
    : Constant(FloatConstantKind, type), _value(std::move(value)) {}

FloatConstant* FloatConstant::get(Context& ctx,
                                  FloatType* type,
                                  const std::string& value) {
  return ctx_impl(ctx).float_cst(type, value);
}

FloatConstant* FloatConstant::get(Context& ctx,
                                  FloatType* type,
                                  const char* value) {
  return ctx_impl(ctx).float_cst(type, value);
}

void FloatConstant::dump(std::ostream& o) const {
  o << this->_value;
}

// NullConstant

NullConstant::NullConstant(PointerType* type)
    : Constant(NullConstantKind, type) {}

NullConstant* NullConstant::get(Context& ctx, PointerType* type) {
  return ctx_impl(ctx).null_cst(type);
}

void NullConstant::dump(std::ostream& o) const {
  o << "null";
}

// StructConstant

StructConstant::StructConstant(StructType* type, Values values)
    : Constant(StructConstantKind, type), _values(std::move(values)) {
  ikos_assert_msg(std::equal(type->field_begin(),
                             type->field_end(),
                             this->_values.begin(),
                             this->_values.end(),
                             [](const StructType::Field& l, const Field& r) {
                               return l.offset == r.offset &&
                                      l.type == r.value->type();
                             }),
                  "type does not match");
}

StructConstant* StructConstant::get(Context& ctx,
                                    StructType* type,
                                    const Values& values) {
  return ctx_impl(ctx).struct_cst(type, values);
}

void StructConstant::dump(std::ostream& o) const {
  o << "{";
  for (auto it = this->_values.cbegin(), et = this->_values.cend(); it != et;) {
    o << it->offset << ": ";
    it->value->dump(o);
    ++it;
    if (it != et) {
      o << ", ";
    }
  }
  o << "}";
}

// SequentialConstant

SequentialConstant::SequentialConstant(ValueKind kind,
                                       SequentialType* type,
                                       const Values& values)
    : Constant(kind, type), _values(values) {
  ikos_assert_msg(type->num_elements() == values.size(), "incompatible size");
  ikos_assert_msg(std::all_of(values.begin(),
                              values.end(),
                              [=](auto& v) {
                                return type->element_type() == v->type();
                              }),
                  "type does not match");
}

// ArrayConstant

ArrayConstant::ArrayConstant(ArrayType* type, const Values& values)
    : SequentialConstant(ArrayConstantKind, type, values) {}

ArrayConstant* ArrayConstant::get(Context& ctx,
                                  ArrayType* type,
                                  const Values& values) {
  return ctx_impl(ctx).array_cst(type, values);
}

void ArrayConstant::dump(std::ostream& o) const {
  o << "[";
  for (auto it = this->element_begin(), et = this->element_end(); it != et;) {
    (*it)->dump(o);
    ++it;
    if (it != et) {
      o << ", ";
    }
  }
  o << "]";
}

// VectorConstant

VectorConstant::VectorConstant(VectorType* type, const Values& values)
    : SequentialConstant(VectorConstantKind, type, values) {}

VectorConstant* VectorConstant::get(Context& ctx,
                                    VectorType* type,
                                    const Values& values) {
  return ctx_impl(ctx).vector_cst(type, values);
}

void VectorConstant::dump(std::ostream& o) const {
  o << "<";
  for (auto it = this->element_begin(), et = this->element_end(); it != et;) {
    (*it)->dump(o);
    ++it;
    if (it != et) {
      o << ", ";
    }
  }
  o << ">";
}

// AggregateZeroConstant

AggregateZeroConstant::AggregateZeroConstant(AggregateType* type)
    : Constant(AggregateZeroConstantKind, type) {}

AggregateZeroConstant* AggregateZeroConstant::get(Context& ctx,
                                                  AggregateType* type) {
  return ctx_impl(ctx).aggregate_zero_cst(type);
}

void AggregateZeroConstant::dump(std::ostream& o) const {
  o << "aggregate_zero";
}

// FunctionPointerConstant

FunctionPointerConstant::FunctionPointerConstant(PointerType* type,
                                                 Function* function)
    : Constant(FunctionPointerConstantKind, type), _function(function) {
  ikos_assert_msg(function, "function is null");
  ikos_assert_msg(type->pointee() == function->type(), "type does not match");
}

FunctionPointerConstant* FunctionPointerConstant::get(Context& ctx,
                                                      Function* function) {
  return ctx_impl(ctx).function_pointer_cst(function);
}

void FunctionPointerConstant::dump(std::ostream& o) const {
  o << "@" << this->_function->name();
}

// InlineAssemblyConstant

InlineAssemblyConstant::InlineAssemblyConstant(PointerType* type,
                                               std::string code)
    : Constant(InlineAssemblyConstantKind, type), _code(std::move(code)) {}

InlineAssemblyConstant* InlineAssemblyConstant::get(Context& ctx,
                                                    PointerType* type,
                                                    const std::string& code) {
  return ctx_impl(ctx).inline_assembly_cst(type, code);
}

void InlineAssemblyConstant::dump(std::ostream& o) const {
  o << "asm \"" << this->_code << "\"";
}

// Variable

Variable::Variable(ValueKind kind, Type* type) : Value(kind, type) {}

void Variable::set_name(std::string name) {
  this->_name = std::move(name);
}

// GlobalVariable

GlobalVariable::GlobalVariable(Bundle* bundle,
                               PointerType* type,
                               std::string name,
                               bool is_definition,
                               uint64_t alignment)
    : Variable(GlobalVariableKind, type),
      _parent(bundle),
      _initializer(nullptr),
      _alignment(alignment) {
  ikos_assert_msg(bundle, "bundle is null");
  ikos_assert_msg(!name.empty(), "global variable name is empty");

  if (!name.empty() && bundle->is_name_available(name)) {
    Variable::set_name(std::move(name));
  } else {
    Variable::set_name(bundle->find_available_name(name));
  }

  if (is_definition) {
    this->_initializer = std::unique_ptr< Code >(new Code(this));
  }
}

GlobalVariable* GlobalVariable::create(Bundle* bundle,
                                       PointerType* type,
                                       std::string name,
                                       bool is_definition,
                                       uint64_t alignment) {
  GlobalVariable* gv = new GlobalVariable(bundle,
                                          type,
                                          std::move(name),
                                          is_definition,
                                          alignment);
  bundle->add_global_variable(std::unique_ptr< GlobalVariable >(gv));
  return gv;
}

void GlobalVariable::set_name(std::string new_name) {
  // In this case, we need to notify the bundle,
  // because it keeps a map from name to global variables
  ikos_assert_msg(!new_name.empty(), "global variable name is empty");
  std::string prev_name = this->name();

  if (!new_name.empty() && this->_parent->is_name_available(new_name)) {
    Variable::set_name(std::move(new_name));
  } else {
    Variable::set_name(this->_parent->find_available_name(new_name));
  }

  this->_parent->rename_global_variable(this, prev_name, this->name());
}

void GlobalVariable::dump(std::ostream& o) const {
  o << "@" << this->name();
}

// LocalVariable

LocalVariable::LocalVariable(Function* function,
                             PointerType* type,
                             uint64_t alignment)
    : Variable(LocalVariableKind, type),
      _parent(function),
      _alignment(alignment) {
  ikos_assert_msg(function, "function is null");
}

LocalVariable* LocalVariable::create(Function* function,
                                     PointerType* type,
                                     uint64_t alignment) {
  auto lv = std::unique_ptr< LocalVariable >(
      new LocalVariable(function, type, alignment));
  return function->add_local_variable(std::move(lv));
}

Context& LocalVariable::context() const {
  return this->bundle()->context();
}

Bundle* LocalVariable::bundle() const {
  return this->function()->bundle();
}

void LocalVariable::dump(std::ostream& o) const {
  o << "$";
  if (this->has_name()) {
    o << this->name();
  } else {
    o << this;
  }
}

// InternalVariable

InternalVariable::InternalVariable(Code* code, Type* type)
    : Variable(InternalVariableKind, type), _parent(code) {
  ikos_assert_msg(code, "code is null");
}

InternalVariable* InternalVariable::create(Code* code, Type* type) {
  auto iv =
      std::unique_ptr< InternalVariable >(new InternalVariable(code, type));
  return code->add_internal_variable(std::move(iv));
}

Context& InternalVariable::context() const {
  return this->bundle()->context();
}

Bundle* InternalVariable::bundle() const {
  return this->code()->bundle();
}

void InternalVariable::dump(std::ostream& o) const {
  o << "%";
  if (this->has_name()) {
    o << this->name();
  } else {
    o << this;
  }
}

} // end namespace ar
} // end namespace ikos
