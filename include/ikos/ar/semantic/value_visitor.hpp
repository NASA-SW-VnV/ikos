/*******************************************************************************
 *
 * \file
 * \brief Value visitor definition
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

#include <ikos/ar/semantic/value.hpp>
#include <ikos/ar/support/assert.hpp>
#include <ikos/ar/support/cast.hpp>

namespace ikos {
namespace ar {

/// \brief Apply a value visitor on a value
///
/// A value visitor looks like:
///
/// \code{.cpp}
/// struct MyValueVisitor {
///   using ResultType = int;
///
///   int operator()(UndefinedConstant* c) { ... }
///   int operator()(IntegerConstant* c) { ... }
///   int operator()(FloatConstant* c) { ... }
///   int operator()(NullConstant* c) { ... }
///   int operator()(StructConstant* c) { ... }
///   int operator()(ArrayConstant* c) { ... }
///   int operator()(VectorConstant* c) { ... }
///   int operator()(AggregateZeroConstant* c) { ... }
///   int operator()(FunctionPointerConstant* c) { ... }
///   int operator()(InlineAssemblyConstant* c) { ... }
///   int operator()(GlobalVariable* v) { ... }
///   int operator()(LocalVariable* v) { ... }
///   int operator()(InternalVariable* v) { ... }
/// };
/// \endcode
template < typename Visitor >
typename Visitor::ResultType apply_visitor(Visitor& visitor, Value* v) {
  switch (v->kind()) {
    case Value::UndefinedConstantKind:
      return visitor(cast< UndefinedConstant >(v));
    case Value::IntegerConstantKind:
      return visitor(cast< IntegerConstant >(v));
    case Value::FloatConstantKind:
      return visitor(cast< FloatConstant >(v));
    case Value::NullConstantKind:
      return visitor(cast< NullConstant >(v));
    case Value::StructConstantKind:
      return visitor(cast< StructConstant >(v));
    case Value::ArrayConstantKind:
      return visitor(cast< ArrayConstant >(v));
    case Value::VectorConstantKind:
      return visitor(cast< VectorConstant >(v));
    case Value::AggregateZeroConstantKind:
      return visitor(cast< AggregateZeroConstant >(v));
    case Value::FunctionPointerConstantKind:
      return visitor(cast< FunctionPointerConstant >(v));
    case Value::InlineAssemblyConstantKind:
      return visitor(cast< InlineAssemblyConstant >(v));
    case Value::GlobalVariableKind:
      return visitor(cast< GlobalVariable >(v));
    case Value::LocalVariableKind:
      return visitor(cast< LocalVariable >(v));
    case Value::InternalVariableKind:
      return visitor(cast< InternalVariable >(v));
    default:
      ikos_unreachable("unexpected value");
  }
}

/// \brief Apply a const value visitor on a value
///
/// A const value visitor looks like:
///
/// \code{.cpp}
/// struct MyValueVisitor {
///   using ResultType = int;
///
///   int operator()(UndefinedConstant* c) const { ... }
///   int operator()(IntegerConstant* c) const { ... }
///   int operator()(FloatConstant* c) const { ... }
///   int operator()(NullConstant* c) const { ... }
///   int operator()(StructConstant* c) const { ... }
///   int operator()(ArrayConstant* c) const { ... }
///   int operator()(VectorConstant* c) const { ... }
///   int operator()(AggregateZeroConstant* c) const { ... }
///   int operator()(FunctionPointerConstant* c) const { ... }
///   int operator()(InlineAssemblyConstant* c) const { ... }
///   int operator()(GlobalVariable* v) const { ... }
///   int operator()(LocalVariable* v) const { ... }
///   int operator()(InternalVariable* v) const { ... }
/// };
/// \endcode
template < typename Visitor >
typename Visitor::ResultType apply_visitor(const Visitor& visitor, Value* v) {
  switch (v->kind()) {
    case Value::UndefinedConstantKind:
      return visitor(cast< UndefinedConstant >(v));
    case Value::IntegerConstantKind:
      return visitor(cast< IntegerConstant >(v));
    case Value::FloatConstantKind:
      return visitor(cast< FloatConstant >(v));
    case Value::NullConstantKind:
      return visitor(cast< NullConstant >(v));
    case Value::StructConstantKind:
      return visitor(cast< StructConstant >(v));
    case Value::ArrayConstantKind:
      return visitor(cast< ArrayConstant >(v));
    case Value::VectorConstantKind:
      return visitor(cast< VectorConstant >(v));
    case Value::AggregateZeroConstantKind:
      return visitor(cast< AggregateZeroConstant >(v));
    case Value::FunctionPointerConstantKind:
      return visitor(cast< FunctionPointerConstant >(v));
    case Value::InlineAssemblyConstantKind:
      return visitor(cast< InlineAssemblyConstant >(v));
    case Value::GlobalVariableKind:
      return visitor(cast< GlobalVariable >(v));
    case Value::LocalVariableKind:
      return visitor(cast< LocalVariable >(v));
    case Value::InternalVariableKind:
      return visitor(cast< InternalVariable >(v));
    default:
      ikos_unreachable("unexpected value");
  }
}

} // end namespace ar
} // end namespace ikos
