/*******************************************************************************
 *
 * \file
 * \brief Type visitor definition
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

#include <ikos/ar/semantic/type.hpp>
#include <ikos/ar/support/assert.hpp>
#include <ikos/ar/support/cast.hpp>

namespace ikos {
namespace ar {

/// \brief Apply a type visitor on a type
///
/// A type visitor looks like:
///
/// \code{.cpp}
/// struct MyTypeVisitor {
///   using ResultType = int;
///
///   int operator()(VoidType* t) { ... }
///   int operator()(IntegerType* t) { ... }
///   int operator()(FloatType* t) { ... }
///   int operator()(PointerType* t) { ... }
///   int operator()(StructType* t) { ... }
///   int operator()(ArrayType* t) { ... }
///   int operator()(VectorType* t) { ... }
///   int operator()(OpaqueType* t) { ... }
///   int operator()(FunctionType* t) { ... }
/// };
/// \endcode
template < typename Visitor >
typename Visitor::ResultType apply_visitor(Visitor& visitor, Type* t) {
  switch (t->kind()) {
    case Type::VoidKind:
      return visitor(cast< VoidType >(t));
    case Type::IntegerKind:
      return visitor(cast< IntegerType >(t));
    case Type::FloatKind:
      return visitor(cast< FloatType >(t));
    case Type::PointerKind:
      return visitor(cast< PointerType >(t));
    case Type::StructKind:
      return visitor(cast< StructType >(t));
    case Type::ArrayKind:
      return visitor(cast< ArrayType >(t));
    case Type::VectorKind:
      return visitor(cast< VectorType >(t));
    case Type::OpaqueKind:
      return visitor(cast< OpaqueType >(t));
    case Type::FunctionKind:
      return visitor(cast< FunctionType >(t));
    default:
      ikos_unreachable("unexpected type");
  }
}

/// \brief Apply a const type visitor on a type
///
/// A const type visitor looks like:
///
/// \code{.cpp}
/// struct MyTypeVisitor {
///   using ResultType = int;
///
///   int operator()(VoidType* t) const { ... }
///   int operator()(IntegerType* t) const { ... }
///   int operator()(FloatType* t) const { ... }
///   int operator()(PointerType* t) const { ... }
///   int operator()(StructType* t) const { ... }
///   int operator()(ArrayType* t) const { ... }
///   int operator()(VectorType* t) const { ... }
///   int operator()(OpaqueType* t) const { ... }
///   int operator()(FunctionType* t) const { ... }
/// };
/// \endcode
template < typename Visitor >
typename Visitor::ResultType apply_visitor(const Visitor& visitor, Type* t) {
  switch (t->kind()) {
    case Type::VoidKind:
      return visitor(cast< VoidType >(t));
    case Type::IntegerKind:
      return visitor(cast< IntegerType >(t));
    case Type::FloatKind:
      return visitor(cast< FloatType >(t));
    case Type::PointerKind:
      return visitor(cast< PointerType >(t));
    case Type::StructKind:
      return visitor(cast< StructType >(t));
    case Type::ArrayKind:
      return visitor(cast< ArrayType >(t));
    case Type::VectorKind:
      return visitor(cast< VectorType >(t));
    case Type::OpaqueKind:
      return visitor(cast< OpaqueType >(t));
    case Type::FunctionKind:
      return visitor(cast< FunctionType >(t));
    default:
      ikos_unreachable("unexpected type");
  }
}

} // end namespace ar
} // end namespace ikos
