/*******************************************************************************
 *
 * \file
 * \brief Implementation of ContextImpl
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
#include <ikos/ar/semantic/context.hpp>
#include <ikos/ar/semantic/function.hpp>
#include <ikos/ar/semantic/type.hpp>

#include "context_impl.hpp"

namespace ikos {
namespace ar {

ContextImpl::ContextImpl()
    : _ui1_ty(1, Unsigned),
      _ui8_ty(8, Unsigned),
      _ui16_ty(16, Unsigned),
      _ui32_ty(32, Unsigned),
      _ui64_ty(64, Unsigned),
      _si1_ty(1, Signed),
      _si8_ty(8, Signed),
      _si16_ty(16, Signed),
      _si32_ty(32, Signed),
      _si64_ty(64, Signed),
      _half_ty(16, Half),
      _float_ty(32, Float),
      _double_ty(64, Double),
      _x86_fp80_ty(80, X86_FP80),
      _fp128_ty(128, FP128),
      _ppc_fp128_ty(128, PPC_FP128) {}

ContextImpl::~ContextImpl() = default;

void ContextImpl::add_bundle(std::unique_ptr< Bundle > bundle) {
  this->_bundles.emplace_back(std::move(bundle));
}

IntegerType* ContextImpl::integer_type(uint64_t bit_width, Signedness sign) {
  auto it = this->_integer_types.find(std::make_tuple(bit_width, sign));
  if (it == this->_integer_types.end()) {
    auto type =
        std::unique_ptr< IntegerType >(new IntegerType(bit_width, sign));
    auto res = this->_integer_types.emplace(std::make_tuple(bit_width, sign),
                                            std::move(type));
    ikos_assert(res.second);
    return res.first->second.get();
  } else {
    return it->second.get();
  }
}

PointerType* ContextImpl::pointer_type(Type* pointee) {
  auto it = this->_pointer_types.find(pointee);
  if (it == this->_pointer_types.end()) {
    auto type = std::unique_ptr< PointerType >(new PointerType(pointee));
    auto res = this->_pointer_types.emplace(pointee, std::move(type));
    ikos_assert(res.second);
    return res.first->second.get();
  } else {
    return it->second.get();
  }
}

ArrayType* ContextImpl::array_type(Type* element_type,
                                   const ZNumber& num_element) {
  auto it = this->_array_types.find(std::make_tuple(element_type, num_element));
  if (it == this->_array_types.end()) {
    auto type =
        std::unique_ptr< ArrayType >(new ArrayType(element_type, num_element));
    auto res =
        this->_array_types.emplace(std::make_tuple(element_type, num_element),
                                   std::move(type));
    ikos_assert(res.second);
    return res.first->second.get();
  } else {
    return it->second.get();
  }
}

VectorType* ContextImpl::vector_type(ScalarType* element_type,
                                     const ZNumber& num_element) {
  auto it =
      this->_vector_types.find(std::make_tuple(element_type, num_element));
  if (it == this->_vector_types.end()) {
    auto type = std::unique_ptr< VectorType >(
        new VectorType(element_type, num_element));
    auto res =
        this->_vector_types.emplace(std::make_tuple(element_type, num_element),
                                    std::move(type));
    ikos_assert(res.second);
    return res.first->second.get();
  } else {
    return it->second.get();
  }
}

FunctionType* ContextImpl::function_type(
    Type* return_type,
    const FunctionType::ParamTypes& param_types,
    bool is_var_arg) {
  auto it = this->_function_types.find(
      std::make_tuple(return_type, param_types, is_var_arg));
  if (it == this->_function_types.end()) {
    auto type = std::unique_ptr< FunctionType >(
        new FunctionType(return_type, param_types, is_var_arg));
    auto res = this->_function_types.emplace(std::make_tuple(return_type,
                                                             param_types,
                                                             is_var_arg),
                                             std::move(type));
    ikos_assert(res.second);
    return res.first->second.get();
  } else {
    return it->second.get();
  }
}

Type* ContextImpl::add_type(std::unique_ptr< Type > type) {
  this->_types.emplace_back(std::move(type));
  return this->_types.back().get();
}

UndefinedConstant* ContextImpl::undefined_cst(Type* type) {
  auto it = this->_undefined_constants.find(type);
  if (it == this->_undefined_constants.end()) {
    auto cst =
        std::unique_ptr< UndefinedConstant >(new UndefinedConstant(type));
    auto res = this->_undefined_constants.emplace(type, std::move(cst));
    ikos_assert(res.second);
    return res.first->second.get();
  } else {
    return it->second.get();
  }
}

IntegerConstant* ContextImpl::integer_cst(IntegerType* type,
                                          const MachineInt& value) {
  auto it = this->_integer_constants.find(std::make_tuple(type, value));
  if (it == this->_integer_constants.end()) {
    auto cst =
        std::unique_ptr< IntegerConstant >(new IntegerConstant(type, value));
    auto res = this->_integer_constants.emplace(std::make_tuple(type, value),
                                                std::move(cst));
    ikos_assert(res.second);
    return res.first->second.get();
  } else {
    return it->second.get();
  }
}

FloatConstant* ContextImpl::float_cst(FloatType* type,
                                      const std::string& value) {
  auto it = this->_float_constants.find(std::make_tuple(type, value));
  if (it == this->_float_constants.end()) {
    auto cst = std::unique_ptr< FloatConstant >(new FloatConstant(type, value));
    auto res = this->_float_constants.emplace(std::make_tuple(type, value),
                                              std::move(cst));
    ikos_assert(res.second);
    return res.first->second.get();
  } else {
    return it->second.get();
  }
}

NullConstant* ContextImpl::null_cst(PointerType* type) {
  auto it = this->_null_constants.find(type);
  if (it == this->_null_constants.end()) {
    auto cst = std::unique_ptr< NullConstant >(new NullConstant(type));
    auto res = this->_null_constants.emplace(type, std::move(cst));
    ikos_assert(res.second);
    return res.first->second.get();
  } else {
    return it->second.get();
  }
}

StructConstant* ContextImpl::struct_cst(StructType* type,
                                        const StructConstant::Values& values) {
  auto it = this->_struct_constants.find(std::make_tuple(type, values));
  if (it == this->_struct_constants.end()) {
    auto cst =
        std::unique_ptr< StructConstant >(new StructConstant(type, values));
    auto res = this->_struct_constants.emplace(std::make_tuple(type, values),
                                               std::move(cst));
    ikos_assert(res.second);
    return res.first->second.get();
  } else {
    return it->second.get();
  }
}

ArrayConstant* ContextImpl::array_cst(ArrayType* type,
                                      const ArrayConstant::Values& values) {
  auto it = this->_array_constants.find(std::make_tuple(type, values));
  if (it == this->_array_constants.end()) {
    auto cst =
        std::unique_ptr< ArrayConstant >(new ArrayConstant(type, values));
    auto res = this->_array_constants.emplace(std::make_tuple(type, values),
                                              std::move(cst));
    ikos_assert(res.second);
    return res.first->second.get();
  } else {
    return it->second.get();
  }
}

VectorConstant* ContextImpl::vector_cst(VectorType* type,
                                        const VectorConstant::Values& values) {
  auto it = this->_vector_constants.find(std::make_tuple(type, values));
  if (it == this->_vector_constants.end()) {
    auto cst =
        std::unique_ptr< VectorConstant >(new VectorConstant(type, values));
    auto res = this->_vector_constants.emplace(std::make_tuple(type, values),
                                               std::move(cst));
    ikos_assert(res.second);
    return res.first->second.get();
  } else {
    return it->second.get();
  }
}

AggregateZeroConstant* ContextImpl::aggregate_zero_cst(AggregateType* type) {
  auto it = this->_aggregate_zero_constants.find(type);
  if (it == this->_aggregate_zero_constants.end()) {
    auto cst = std::unique_ptr< AggregateZeroConstant >(
        new AggregateZeroConstant(type));
    auto res = this->_aggregate_zero_constants.emplace(type, std::move(cst));
    ikos_assert(res.second);
    return res.first->second.get();
  } else {
    return it->second.get();
  }
}

FunctionPointerConstant* ContextImpl::function_pointer_cst(Function* function) {
  auto it = this->_function_pointer_constants.find(function);
  if (it == this->_function_pointer_constants.end()) {
    ikos_assert_msg(function, "function is null");
    PointerType* fun_ptr_type = this->pointer_type(function->type());
    auto cst = std::unique_ptr< FunctionPointerConstant >(
        new FunctionPointerConstant(fun_ptr_type, function));
    auto res =
        this->_function_pointer_constants.emplace(function, std::move(cst));
    ikos_assert(res.second);
    return res.first->second.get();
  } else {
    return it->second.get();
  }
}

InlineAssemblyConstant* ContextImpl::inline_assembly_cst(
    PointerType* type, const std::string& code) {
  auto it = this->_inline_assembly_constants.find(std::make_tuple(type, code));
  if (it == this->_inline_assembly_constants.end()) {
    auto cst = std::unique_ptr< InlineAssemblyConstant >(
        new InlineAssemblyConstant(type, code));
    auto res =
        this->_inline_assembly_constants.emplace(std::make_tuple(type, code),
                                                 std::move(cst));
    ikos_assert(res.second);
    return res.first->second.get();
  } else {
    return it->second.get();
  }
}

} // end namespace ar
} // end namespace ikos
