/*******************************************************************************
 *
 * \file
 * \brief Variable implementation
 *
 * Author: Clement Decoodt
 *
 * Contributors: Maxime Arthaud
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

#include <boost/thread/locks.hpp>

#include <ikos/analyzer/analysis/variable.hpp>
#include <ikos/analyzer/exception.hpp>

namespace ikos {
namespace analyzer {

// Variable

Variable::Variable(VariableKind kind, ar::Type* type)
    : _kind(kind), _type(type), _offset_var(nullptr) {
  ikos_assert(this->_type != nullptr);
}

Variable::~Variable() = default;

// LocalVariable

LocalVariable::LocalVariable(ar::LocalVariable* var)
    : Variable(LocalVariableKind, var->type()), _var(var) {
  ikos_assert(this->_var != nullptr);
}

void LocalVariable::dump(std::ostream& o) const {
  this->_var->dump(o);
}

// GlobalVariable

GlobalVariable::GlobalVariable(ar::GlobalVariable* var)
    : Variable(GlobalVariableKind, var->type()), _var(var) {
  ikos_assert(this->_var != nullptr);
}

void GlobalVariable::dump(std::ostream& o) const {
  this->_var->dump(o);
}

// InternalVariable

InternalVariable::InternalVariable(ar::InternalVariable* var)
    : Variable(InternalVariableKind, var->type()), _var(var) {
  ikos_assert(this->_var != nullptr);
}

void InternalVariable::dump(std::ostream& o) const {
  return this->_var->dump(o);
}

// InlineAssemblyPointerVariable

InlineAssemblyPointerVariable::InlineAssemblyPointerVariable(
    ar::InlineAssemblyConstant* inline_asm)
    : Variable(InlineAssemblyPointerVariableKind, inline_asm->type()),
      _inline_asm(inline_asm) {
  ikos_assert(this->_inline_asm != nullptr);
}

void InlineAssemblyPointerVariable::dump(std::ostream& o) const {
  o << "%asm:" << this->_inline_asm;
}

// FunctionPointerVariable

FunctionPointerVariable::FunctionPointerVariable(ar::Function* fun)
    : Variable(FunctionPointerVariableKind,
               ar::PointerType::get(fun->context(), fun->type())),
      _fun(fun) {
  ikos_assert(this->_fun != nullptr);
}

void FunctionPointerVariable::dump(std::ostream& o) const {
  o << "@" << this->_fun->name();
}

// CellVariable

CellVariable::CellVariable(ar::Type* type,
                           MemoryLocation* address,
                           MachineInt offset,
                           MachineInt size)
    : Variable(CellVariableKind, type),
      _address(address),
      _offset(std::move(offset)),
      _size(std::move(size)) {
  ikos_assert(this->_address != nullptr);
}

void CellVariable::dump(std::ostream& o) const {
  o << "C{";
  this->_address->dump(o);
  o << "," << this->_offset << "," << this->_size << "}";
}

// OffsetVariable

OffsetVariable::OffsetVariable(ar::Type* type, Variable* pointer)
    : Variable(OffsetVariableKind, type), _pointer(pointer) {
  ikos_assert(this->_pointer != nullptr);
}

void OffsetVariable::dump(std::ostream& o) const {
  this->_pointer->dump(o);
  o << ".offset";
}

// AllocSizeVariable

AllocSizeVariable::AllocSizeVariable(ar::Type* type, MemoryLocation* address)
    : Variable(AllocSizeVariableKind, type), _address(address) {
  ikos_assert(this->_address != nullptr);
}

void AllocSizeVariable::dump(std::ostream& o) const {
  this->_address->dump(o);
  o << ".size";
}

// ReturnVariable

ReturnVariable::ReturnVariable(ar::Function* fun)
    : Variable(ReturnVariableKind, fun->type()->return_type()), _fun(fun) {
  ikos_assert(this->_fun != nullptr);
}

void ReturnVariable::dump(std::ostream& o) const {
  o << this->_fun->name() << ".return";
}

// NamedShadowVariable

NamedShadowVariable::NamedShadowVariable(ar::Type* type, std::string name)
    : Variable(NamedShadowVariableKind, type), _name(std::move(name)) {
  ikos_assert(!this->_name.empty());
}

void NamedShadowVariable::dump(std::ostream& o) const {
  o << this->_name;
}

// UnnamedShadowVariable

UnnamedShadowVariable::UnnamedShadowVariable(ar::Type* type, std::size_t id)
    : Variable(UnnamedShadowVariableKind, type), _id(id) {}

void UnnamedShadowVariable::dump(std::ostream& o) const {
  o << "unnamed." << this->_id;
}

// VariableFactory

VariableFactory::VariableFactory(ar::Bundle* bundle)
    : _ar_context(bundle->context()),
      _size_type(ar::IntegerType::size_type(bundle)) {}

VariableFactory::~VariableFactory() = default;

LocalVariable* VariableFactory::get_local(ar::LocalVariable* var) {
  {
    boost::shared_lock< boost::shared_mutex > lock(this->_local_variable_mutex);
    auto it = this->_local_variable_map.find(var);
    if (it != this->_local_variable_map.end()) {
      return it->second.get();
    }
  }

  auto vn = std::make_unique< LocalVariable >(var);
  vn->set_offset_var(
      std::make_unique< OffsetVariable >(this->_size_type, vn.get()));

  {
    boost::unique_lock< boost::shared_mutex > lock(this->_local_variable_mutex);
    auto res = this->_local_variable_map.try_emplace(var, std::move(vn));
    return res.first->second.get();
  }
}

GlobalVariable* VariableFactory::get_global(ar::GlobalVariable* var) {
  {
    boost::shared_lock< boost::shared_mutex > lock(
        this->_global_variable_mutex);
    auto it = this->_global_variable_map.find(var);
    if (it != this->_global_variable_map.end()) {
      return it->second.get();
    }
  }

  auto vn = std::make_unique< GlobalVariable >(var);
  vn->set_offset_var(
      std::make_unique< OffsetVariable >(this->_size_type, vn.get()));

  {
    boost::unique_lock< boost::shared_mutex > lock(
        this->_global_variable_mutex);
    auto res = this->_global_variable_map.try_emplace(var, std::move(vn));
    return res.first->second.get();
  }
}

InternalVariable* VariableFactory::get_internal(ar::InternalVariable* var) {
  {
    boost::shared_lock< boost::shared_mutex > lock(
        this->_internal_variable_mutex);
    auto it = this->_internal_variable_map.find(var);
    if (it != this->_internal_variable_map.end()) {
      return it->second.get();
    }
  }

  auto vn = std::make_unique< InternalVariable >(var);
  if (vn->type()->is_pointer() || vn->type()->is_aggregate()) {
    vn->set_offset_var(
        std::make_unique< OffsetVariable >(this->_size_type, vn.get()));
  }

  {
    boost::unique_lock< boost::shared_mutex > lock(
        this->_internal_variable_mutex);
    auto res = this->_internal_variable_map.try_emplace(var, std::move(vn));
    return res.first->second.get();
  }
}

InlineAssemblyPointerVariable* VariableFactory::get_asm_ptr(
    ar::InlineAssemblyConstant* cst) {
  {
    boost::shared_lock< boost::shared_mutex > lock(
        this->_inline_asm_pointer_mutex);
    auto it = this->_inline_asm_pointer_map.find(cst);
    if (it != this->_inline_asm_pointer_map.end()) {
      return it->second.get();
    }
  }

  auto vn = std::make_unique< InlineAssemblyPointerVariable >(cst);
  vn->set_offset_var(
      std::make_unique< OffsetVariable >(this->_size_type, vn.get()));

  {
    boost::unique_lock< boost::shared_mutex > lock(
        this->_inline_asm_pointer_mutex);
    auto res = this->_inline_asm_pointer_map.try_emplace(cst, std::move(vn));
    return res.first->second.get();
  }
}

FunctionPointerVariable* VariableFactory::get_function_ptr(ar::Function* fun) {
  {
    boost::shared_lock< boost::shared_mutex > lock(
        this->_function_pointer_mutex);
    auto it = this->_function_pointer_map.find(fun);
    if (it != this->_function_pointer_map.end()) {
      return it->second.get();
    }
  }

  auto vn = std::make_unique< FunctionPointerVariable >(fun);
  vn->set_offset_var(
      std::make_unique< OffsetVariable >(this->_size_type, vn.get()));

  {
    boost::unique_lock< boost::shared_mutex > lock(
        this->_function_pointer_mutex);
    auto res = this->_function_pointer_map.try_emplace(fun, std::move(vn));
    return res.first->second.get();
  }
}

FunctionPointerVariable* VariableFactory::get_function_ptr(
    ar::FunctionPointerConstant* cst) {
  return this->get_function_ptr(cst->function());
}

CellVariable* VariableFactory::get_cell(MemoryLocation* address,
                                        const MachineInt& offset,
                                        const MachineInt& size,
                                        Signedness sign) {
  auto key = std::make_tuple(address, offset, size);

  {
    boost::shared_lock< boost::shared_mutex > lock(this->_cell_mutex);
    auto it = this->_cell_map.find(key);
    if (it != this->_cell_map.end()) {
      return it->second.get();
    }
  }

  {
    // Create a memory cell variable
    // A cell can be either an integer, a float or a pointer
    // The integer type should have the right bit-width and the given signedness
    // The parameter `size` is in bytes, compute bit-width = size * 8
    //
    // Note: IntegerType::get() is not thread safe, so lock the mutex here
    boost::unique_lock< boost::shared_mutex > lock(this->_cell_mutex);
    bool overflow;
    MachineInt eight(8, size.bit_width(), Unsigned);
    MachineInt bit_width = mul(size, eight, overflow);
    if (overflow || !bit_width.fits< uint64_t >()) {
      throw LogicError("variable factory: cell size too big");
    }
    ar::Type* type = ar::IntegerType::get(this->_ar_context,
                                          bit_width.to< uint64_t >(),
                                          sign);
    auto vn = std::make_unique< CellVariable >(type, address, offset, size);
    vn->set_offset_var(
        std::make_unique< OffsetVariable >(this->_size_type, vn.get()));
    auto res = this->_cell_map.emplace(key, std::move(vn));
    return res.first->second.get();
  }
}

AllocSizeVariable* VariableFactory::get_alloc_size(MemoryLocation* address) {
  {
    boost::shared_lock< boost::shared_mutex > lock(this->_alloc_size_mutex);
    auto it = this->_alloc_size_map.find(address);
    if (it != this->_alloc_size_map.end()) {
      return it->second.get();
    }
  }

  auto vn = std::make_unique< AllocSizeVariable >(this->_size_type, address);

  {
    boost::unique_lock< boost::shared_mutex > lock(this->_alloc_size_mutex);
    auto res = this->_alloc_size_map.try_emplace(address, std::move(vn));
    return res.first->second.get();
  }
}

ReturnVariable* VariableFactory::get_return(ar::Function* fun) {
  {
    boost::shared_lock< boost::shared_mutex > lock(
        this->_return_variable_mutex);
    auto it = this->_return_variable_map.find(fun);
    if (it != this->_return_variable_map.end()) {
      return it->second.get();
    }
  }

  auto vn = std::make_unique< ReturnVariable >(fun);
  if (vn->type()->is_pointer() || vn->type()->is_aggregate()) {
    vn->set_offset_var(
        std::make_unique< OffsetVariable >(this->_size_type, vn.get()));
  }

  {
    boost::unique_lock< boost::shared_mutex > lock(
        this->_return_variable_mutex);
    auto res = this->_return_variable_map.try_emplace(fun, std::move(vn));
    return res.first->second.get();
  }
}

NamedShadowVariable* VariableFactory::get_named_shadow(ar::Type* type,
                                                       llvm::StringRef name) {
  {
    boost::shared_lock< boost::shared_mutex > lock(
        this->_named_shadow_variable_mutex);
    auto it = this->_named_shadow_variable_map.find(name);
    if (it != this->_named_shadow_variable_map.end()) {
      return it->second.get();
    }
  }

  auto vn = std::make_unique< NamedShadowVariable >(type, name.str());
  if (vn->type()->is_pointer() || vn->type()->is_aggregate()) {
    vn->set_offset_var(
        std::make_unique< OffsetVariable >(this->_size_type, vn.get()));
  }

  {
    boost::unique_lock< boost::shared_mutex > lock(
        this->_named_shadow_variable_mutex);
    auto res =
        this->_named_shadow_variable_map.try_emplace(name, std::move(vn));
    return res.first->second.get();
  }
}

UnnamedShadowVariable* VariableFactory::create_unnamed_shadow(ar::Type* type) {
  boost::lock_guard< boost::mutex > lock(this->_unnamed_shadow_variable_mutex);
  std::size_t id = this->_unnamed_shadow_variable_vec.size();
  auto vn = std::make_unique< UnnamedShadowVariable >(type, id);
  if (vn->type()->is_pointer() || vn->type()->is_aggregate()) {
    vn->set_offset_var(
        std::make_unique< OffsetVariable >(this->_size_type, vn.get()));
  }
  this->_unnamed_shadow_variable_vec.emplace_back(std::move(vn));
  return this->_unnamed_shadow_variable_vec.back().get();
}

} // end namespace analyzer
} // end namespace ikos
