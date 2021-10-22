/*******************************************************************************
 *
 * \file
 * \brief Generic API for abstract domains keeping track of exceptions
 *
 * Author: Maxime Arthaud
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

#pragma once

#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/support/mpl.hpp>

namespace ikos {
namespace core {
namespace exception {

/// \brief Base class for abstract domains supporting exceptions
template < typename UnderlyingDomain, typename Derived >
class AbstractDomain : public core::AbstractDomain< Derived > {
public:
  static_assert(core::IsAbstractDomain< UnderlyingDomain >::value,
                "UnderlyingDomain must implement core::AbstractDomain");

public:
  /// \brief Underlying abstract domain
  using UnderlyingDomainT = UnderlyingDomain;

  /// \brief Create the top abstract value
  AbstractDomain() = default;

  /// \brief Copy constructor
  AbstractDomain(const AbstractDomain&) noexcept = default;

  /// \brief Move constructor
  AbstractDomain(AbstractDomain&&) noexcept = default;

  /// \brief Copy assignment operator
  AbstractDomain& operator=(const AbstractDomain&) noexcept = default;

  /// \brief Move assignment operator
  AbstractDomain& operator=(AbstractDomain&&) noexcept = default;

  /// \brief Provide access to the normal execution flow state
  virtual UnderlyingDomain& normal() = 0;

  /// \brief Provide access to the normal execution flow state
  virtual const UnderlyingDomain& normal() const = 0;

  /// \brief Provide access to the state of all uncaught exceptions
  virtual UnderlyingDomain& caught_exceptions() = 0;

  /// \brief Provide access to the state of all uncaught exceptions
  virtual const UnderlyingDomain& caught_exceptions() const = 0;

  /// \brief Provide access to the state of all propagated exceptions
  virtual UnderlyingDomain propagated_exceptions() = 0;

  /// \brief Provide access to the state of all propagated exceptions
  virtual const UnderlyingDomain& propagated_exceptions() const = 0;

  /// \brief Check if the normal execution flow abstract value is bottom
  virtual bool is_normal_flow_bottom() const = 0;

  /// \brief Check if the normal execution flow abstract value is top
  virtual bool is_normal_flow_top() const = 0;

  /// \brief Set the normal execution flow state to bottom
  virtual void set_normal_flow_to_bottom() = 0;

  /// \brief Set the normal execution flow state to top
  virtual void set_normal_flow_to_top() = 0;

  /// \brief Check if the state of all uncaught exceptions is bottom
  virtual bool is_caught_exceptions_bottom() const = 0;

  /// \brief Check if the state of all uncaught exceptions is top
  virtual bool is_caught_exceptions_top() const = 0;

  /// \brief Set the state of uncaught exceptions to bottom
  virtual void set_caught_exceptions_to_bottom() = 0;

  /// \brief Set the state of uncaught exceptions to top
  virtual void set_caught_exceptions_to_top() = 0;

  /// \brief Check if the state of all propagated exceptions is bottom
  virtual bool is_propagated_exceptions_bottom() const = 0;

  /// \brief Check if the state of all propagated exceptions exceptions is top
  virtual bool is_propagated_exceptions_top() const = 0;

  /// \brief Set the state of all propagated exceptions to bottom
  virtual void set_propagated_exceptions_to_bottom() = 0;

  /// \brief Set the state of all propagated exceptions to top
  virtual void set_propagated_exceptions_to_top() = 0;

  /// \brief Merge propagated exceptions in caught exceptions
  virtual void merge_propagated_in_caught_exceptions() = 0;

  /// \brief Merge caught exceptions in propagated exceptions
  virtual void merge_caught_in_propagated_exceptions() = 0;

  /// \brief Execute the normal branch after an invoke
  virtual void enter_normal() = 0;

  /// \brief Execute the exception branch after an invoke
  virtual void enter_catch() = 0;

  /// \brief Disregard the pending exceptions
  virtual void ignore_exceptions() = 0;

  /// \brief Throw an exception
  virtual void throw_exception() = 0;

  /// \brief Resume/re-throw an exception
  virtual void resume_exception() = 0;

}; // end class AbstractDomain

/// \brief Check if a type is an exception abstract domain
template < typename T, typename = void >
struct IsAbstractDomain : std::false_type {};

template < typename T >
struct IsAbstractDomain< T, void_t< typename T::UnderlyingDomainT > >
    : std::is_base_of<
          exception::AbstractDomain< typename T::UnderlyingDomainT, T >,
          T > {};

} // end namespace exception
} // end namespace core
} // end namespace ikos
