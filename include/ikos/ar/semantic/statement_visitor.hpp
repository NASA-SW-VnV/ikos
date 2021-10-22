/*******************************************************************************
 *
 * \file
 * \brief Statement visitor definition
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

#include <ikos/ar/semantic/statement.hpp>
#include <ikos/ar/support/assert.hpp>
#include <ikos/ar/support/cast.hpp>

namespace ikos {
namespace ar {

/// \brief Apply a statement visitor on a statement
///
/// A statement visitor looks like:
///
/// \code{.cpp}
/// struct MyStatementVisitor {
///   using ResultType = int;
///
///   int operator()(Assignment* s) { ... }
///   int operator()(UnaryOperation* s) { ... }
///   int operator()(BinaryOperation* s) { ... }
///   int operator()(Comparison* s) { ... }
///   int operator()(ReturnValue* s) { ... }
///   int operator()(Unreachable* s) { ... }
///   int operator()(Allocate* s) { ... }
///   int operator()(PointerShift* s) { ... }
///   int operator()(Load* s) { ... }
///   int operator()(Store* s) { ... }
///   int operator()(ExtractElement* s) { ... }
///   int operator()(InsertElement* s) { ... }
///   int operator()(ShuffleVector* s) { ... }
///   int operator()(Call* s) { ... }
///   int operator()(Invoke* s) { ... }
///   int operator()(LandingPad* s) { ... }
///   int operator()(Resume* s) { ... }
/// };
/// \endcode
template < typename Visitor >
typename Visitor::ResultType apply_visitor(Visitor& visitor, Statement* s) {
  switch (s->kind()) {
    case Statement::AssignmentKind:
      return visitor(cast< Assignment >(s));
    case Statement::UnaryOperationKind:
      return visitor(cast< UnaryOperation >(s));
    case Statement::BinaryOperationKind:
      return visitor(cast< BinaryOperation >(s));
    case Statement::ComparisonKind:
      return visitor(cast< Comparison >(s));
    case Statement::ReturnValueKind:
      return visitor(cast< ReturnValue >(s));
    case Statement::UnreachableKind:
      return visitor(cast< Unreachable >(s));
    case Statement::AllocateKind:
      return visitor(cast< Allocate >(s));
    case Statement::PointerShiftKind:
      return visitor(cast< PointerShift >(s));
    case Statement::LoadKind:
      return visitor(cast< Load >(s));
    case Statement::StoreKind:
      return visitor(cast< Store >(s));
    case Statement::ExtractElementKind:
      return visitor(cast< ExtractElement >(s));
    case Statement::InsertElementKind:
      return visitor(cast< InsertElement >(s));
    case Statement::ShuffleVectorKind:
      return visitor(cast< ShuffleVector >(s));
    case Statement::CallKind:
      return visitor(cast< Call >(s));
    case Statement::InvokeKind:
      return visitor(cast< Invoke >(s));
    case Statement::LandingPadKind:
      return visitor(cast< LandingPad >(s));
    case Statement::ResumeKind:
      return visitor(cast< Resume >(s));
    default:
      ikos_unreachable("unexpected statement");
  }
}

/// \brief Apply a const statement visitor on a statement
///
/// A const statement visitor looks like:
///
/// \code{.cpp}
/// struct MyStatementVisitor {
///   using ResultType = int;
///
///   int operator()(Assignment* s) const { ... }
///   int operator()(UnaryOperation* s) const { ... }
///   int operator()(BinaryOperation* s) const { ... }
///   int operator()(Comparison* s) const { ... }
///   int operator()(ReturnValue* s) const { ... }
///   int operator()(Unreachable* s) const { ... }
///   int operator()(Allocate* s) const { ... }
///   int operator()(PointerShift* s) const { ... }
///   int operator()(Load* s) const { ... }
///   int operator()(Store* s) const { ... }
///   int operator()(ExtractElement* s) const { ... }
///   int operator()(InsertElement* s) const { ... }
///   int operator()(ShuffleVector* s) const { ... }
///   int operator()(Call* s) const { ... }
///   int operator()(Invoke* s) const { ... }
///   int operator()(LandingPad* s) const { ... }
///   int operator()(Resume* s) const { ... }
/// };
/// \endcode
template < typename Visitor >
typename Visitor::ResultType apply_visitor(const Visitor& visitor,
                                           Statement* s) {
  switch (s->kind()) {
    case Statement::AssignmentKind:
      return visitor(cast< Assignment >(s));
    case Statement::UnaryOperationKind:
      return visitor(cast< UnaryOperation >(s));
    case Statement::BinaryOperationKind:
      return visitor(cast< BinaryOperation >(s));
    case Statement::ComparisonKind:
      return visitor(cast< Comparison >(s));
    case Statement::ReturnValueKind:
      return visitor(cast< ReturnValue >(s));
    case Statement::UnreachableKind:
      return visitor(cast< Unreachable >(s));
    case Statement::AllocateKind:
      return visitor(cast< Allocate >(s));
    case Statement::PointerShiftKind:
      return visitor(cast< PointerShift >(s));
    case Statement::LoadKind:
      return visitor(cast< Load >(s));
    case Statement::StoreKind:
      return visitor(cast< Store >(s));
    case Statement::ExtractElementKind:
      return visitor(cast< ExtractElement >(s));
    case Statement::InsertElementKind:
      return visitor(cast< InsertElement >(s));
    case Statement::ShuffleVectorKind:
      return visitor(cast< ShuffleVector >(s));
    case Statement::CallKind:
      return visitor(cast< Call >(s));
    case Statement::InvokeKind:
      return visitor(cast< Invoke >(s));
    case Statement::ResumeKind:
      return visitor(cast< Resume >(s));
    default:
      ikos_unreachable("unexpected statement");
  }
}

} // end namespace ar
} // end namespace ikos
