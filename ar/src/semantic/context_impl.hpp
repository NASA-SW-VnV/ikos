/*******************************************************************************
 *
 * \file
 * \brief ContextImpl definition
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
#include <tuple>
#include <utility>
#include <vector>

#include <boost/container/flat_map.hpp>

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/type.hpp>
#include <ikos/ar/semantic/value.hpp>

namespace ikos {
namespace ar {

class ContextImpl {
private:
  // List of owned bundles
  std::vector< std::unique_ptr< Bundle > > _bundles;

  // Pre-allocated void type
  VoidType _void_ty;

  // Pre-allocated unsigned integer types
  IntegerType _ui1_ty;
  IntegerType _ui8_ty;
  IntegerType _ui16_ty;
  IntegerType _ui32_ty;
  IntegerType _ui64_ty;

  // Pre-allocated signed integer types
  IntegerType _si1_ty;
  IntegerType _si8_ty;
  IntegerType _si16_ty;
  IntegerType _si32_ty;
  IntegerType _si64_ty;

  // Pre-allocated float types
  FloatType _half_ty;
  FloatType _float_ty;
  FloatType _double_ty;
  FloatType _x86_fp80_ty;
  FloatType _fp128_ty;
  FloatType _ppc_fp128_ty;

  // Libc FILE opaque type (used for intrinsics)
  OpaqueType _libc_file_ty;

  // Using boost::container::flat_map with std::pair causes crashes on FreeBSD.
  // As a workaround, we use std::tuple instead of std::pair here.
  // see https://github.com/NASA-SW-VnV/ikos/issues/22
  // see https://github.com/boostorg/container/issues/97

  // Integer types
  boost::container::flat_map< std::tuple< uint64_t, Signedness >,
                              std::unique_ptr< IntegerType > >
      _integer_types;

  // Pointer types
  boost::container::flat_map< Type*, std::unique_ptr< PointerType > >
      _pointer_types;

  // Array types
  boost::container::flat_map< std::tuple< Type*, ZNumber >,
                              std::unique_ptr< ArrayType > >
      _array_types;

  // Vector types
  boost::container::flat_map< std::tuple< ScalarType*, ZNumber >,
                              std::unique_ptr< VectorType > >
      _vector_types;

  // Function types
  boost::container::flat_map<
      std::tuple< Type*, FunctionType::ParamTypes, bool >,
      std::unique_ptr< FunctionType > >
      _function_types;

  // Other types (struct and opaque)
  std::vector< std::unique_ptr< Type > > _types;

  // Undefined constants
  boost::container::flat_map< Type*, std::unique_ptr< UndefinedConstant > >
      _undefined_constants;

  // Integer constants
  boost::container::flat_map< std::tuple< IntegerType*, MachineInt >,
                              std::unique_ptr< IntegerConstant > >
      _integer_constants;

  // Float constants
  boost::container::flat_map< std::tuple< FloatType*, std::string >,
                              std::unique_ptr< FloatConstant > >
      _float_constants;

  // Null constants
  boost::container::flat_map< PointerType*, std::unique_ptr< NullConstant > >
      _null_constants;

  // Structure constants
  boost::container::flat_map< std::tuple< StructType*, StructConstant::Values >,
                              std::unique_ptr< StructConstant > >
      _struct_constants;

  // Array constants
  boost::container::flat_map< std::tuple< ArrayType*, ArrayConstant::Values >,
                              std::unique_ptr< ArrayConstant > >
      _array_constants;

  // Vector constants
  boost::container::flat_map< std::tuple< VectorType*, VectorConstant::Values >,
                              std::unique_ptr< VectorConstant > >
      _vector_constants;

  // Aggregate zero constants
  boost::container::flat_map< AggregateType*,
                              std::unique_ptr< AggregateZeroConstant > >
      _aggregate_zero_constants;

  // Function pointer constants
  boost::container::flat_map< Function*,
                              std::unique_ptr< FunctionPointerConstant > >
      _function_pointer_constants;

  // Inline assembly constants
  boost::container::flat_map< std::tuple< PointerType*, std::string >,
                              std::unique_ptr< InlineAssemblyConstant > >
      _inline_assembly_constants;

public:
  /// \brief Default constructor
  ContextImpl();

  /// \brief No copy constructor
  ContextImpl(const ContextImpl&) = delete;

  /// \brief No move constructor
  ContextImpl(ContextImpl&&) = delete;

  /// \brief No copy assignment operator
  ContextImpl& operator=(const ContextImpl&) = delete;

  /// \brief No move assignment operator
  ContextImpl& operator=(ContextImpl&&) = delete;

  /// \brief Destructor
  ~ContextImpl();

  /// \brief Add a bundle in the ContextImpl
  void add_bundle(std::unique_ptr< Bundle >);

  // type management

  VoidType* void_type() { return &_void_ty; }

  IntegerType* ui1_type() { return &_ui1_ty; }
  IntegerType* ui8_type() { return &_ui8_ty; }
  IntegerType* ui16_type() { return &_ui16_ty; }
  IntegerType* ui32_type() { return &_ui32_ty; }
  IntegerType* ui64_type() { return &_ui64_ty; }

  IntegerType* si1_type() { return &_si1_ty; }
  IntegerType* si8_type() { return &_si8_ty; }
  IntegerType* si16_type() { return &_si16_ty; }
  IntegerType* si32_type() { return &_si32_ty; }
  IntegerType* si64_type() { return &_si64_ty; }

  FloatType* half_type() { return &_half_ty; }
  FloatType* float_type() { return &_float_ty; }
  FloatType* double_type() { return &_double_ty; }
  FloatType* x86_fp80_type() { return &_x86_fp80_ty; }
  FloatType* fp128_type() { return &_fp128_ty; }
  FloatType* ppc_fp128_type() { return &_ppc_fp128_ty; }

  /// \brief Get the libc FILE opaque type
  OpaqueType* libc_file_type() { return &_libc_file_ty; }

  /// \brief Get or create an integer type
  IntegerType* integer_type(uint64_t bit_width, Signedness sign);

  /// \brief Get or create a pointer type
  PointerType* pointer_type(Type* pointee);

  /// \brief Get or create an array type
  ArrayType* array_type(Type* element_type, const ZNumber& num_element);

  /// \brief Get or create a vector type
  VectorType* vector_type(ScalarType* element_type, const ZNumber& num_element);

  /// \brief Get or create a function type
  FunctionType* function_type(Type* return_type,
                              const FunctionType::ParamTypes& param_types,
                              bool is_var_arg);

  /// \brief Add a type in the ContextImpl
  ///
  /// \returns a pointer on the type
  Type* add_type(std::unique_ptr< Type >);

  // value management

  /// \brief Get or create an undefined constant
  UndefinedConstant* undefined_cst(Type* type);

  /// \brief Get or create an integer constant
  IntegerConstant* integer_cst(IntegerType* type, const MachineInt& value);

  /// \brief Get or create a float constant
  FloatConstant* float_cst(FloatType* type, const std::string& value);

  /// \brief Get or create a null constant
  NullConstant* null_cst(PointerType* type);

  /// \brief Get or create a structure constant
  StructConstant* struct_cst(StructType* type,
                             const StructConstant::Values& values);

  /// \brief Get or create an array constant
  ArrayConstant* array_cst(ArrayType* type,
                           const ArrayConstant::Values& values);

  /// \brief Get or create a vector constant
  VectorConstant* vector_cst(VectorType* type,
                             const VectorConstant::Values& values);

  /// \brief Get or create an aggregate zero constant
  AggregateZeroConstant* aggregate_zero_cst(AggregateType* type);

  /// \brief Get or create a function pointer constant
  FunctionPointerConstant* function_pointer_cst(Function* function);

  /// \brief Get or create an inline assembly constant
  InlineAssemblyConstant* inline_assembly_cst(PointerType* type,
                                              const std::string& code);

}; // end class ContextImpl

} // end namespace ar
} // end namespace ikos
