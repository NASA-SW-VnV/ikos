/*******************************************************************************
 *
 * \file
 * \brief Generic API for abstract domains keeping track of null variables
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2018-2019 United States Government as represented by the
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

#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/domain/pointer/operator.hpp>
#include <ikos/core/semantic/variable.hpp>
#include <ikos/core/value/nullity.hpp>

namespace ikos {
namespace core {
namespace nullity {

/// \brief Predicate on pointers
using Predicate = pointer::Predicate;

/// \brief Base class for abstract domains keeping track of null variables
template < typename VariableRef, typename Derived >
class AbstractDomain : public core::AbstractDomain< Derived > {
public:
  static_assert(
      core::IsVariable< VariableRef >::value,
      "VariableRef does not meet the requirements for variable types");

public:
  /// \brief Assign `x = null`
  virtual void assign_null(VariableRef x) = 0;

  /// \brief Assign `x = non-null`
  virtual void assign_non_null(VariableRef x) = 0;

  /// \brief Assign `x = y`
  virtual void assign(VariableRef x, VariableRef y) = 0;

  /// \brief Add the constraint `x == null`
  virtual void assert_null(VariableRef x) = 0;

  /// \brief Add the constraint `x != null`
  virtual void assert_non_null(VariableRef x) = 0;

  // \brief Add the constraint `x pred y`
  virtual void add(Predicate pred, VariableRef x, VariableRef y) = 0;

  /// \brief Return true if `x` is null, otherwise false
  virtual bool is_null(VariableRef x) const = 0;

  /// \brief Return true if `x` is non null, otherwise false
  virtual bool is_non_null(VariableRef x) const = 0;

  /// \brief Set the nullity value of a variable
  virtual void set(VariableRef x, const Nullity& value) = 0;

  /// \brief Refine the nullity value of a variable
  virtual void refine(VariableRef x, const Nullity& value) = 0;

  /// \brief Forget the nullity of a variable
  virtual void forget(VariableRef x) = 0;

  /// \brief Get the nullity value for the given variable
  virtual Nullity get(VariableRef x) const = 0;

}; // end class AbstractDomain

/// \brief Check if a type is a nullity abstract domain
template < typename T, typename VariableRef >
struct IsAbstractDomain
    : std::is_base_of< nullity::AbstractDomain< VariableRef, T >, T > {};

} // end namespace nullity
} // end namespace core
} // end namespace ikos
