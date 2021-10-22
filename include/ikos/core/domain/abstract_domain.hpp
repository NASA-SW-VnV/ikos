/*******************************************************************************
 *
 * \file
 * \brief Generic API for abstract domains
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

#include <iosfwd>
#include <string>
#include <type_traits>

namespace ikos {
namespace core {

/// \brief Base class for abstract domains
///
/// Abstract domains should be thread-safe:
/// * It should be safe to call a `const` method on an abstract value shared
///   between multiple threads;
/// * It should be safe to copy an abstract value between multiple threads;
/// * It is NOT safe to call a non-`const` method on an abstract value
///   shared between multiple threads;
template < typename Derived >
class AbstractDomain {
public:
  virtual ~AbstractDomain() {
    // Checks only when the Derived class is instantiated
    static_assert(std::is_base_of< AbstractDomain< Derived >, Derived >::value,
                  "Derived must inherit from AbstractDomain");
    static_assert(std::is_final< Derived >::value, "Derived must be final");
    static_assert(!std::is_default_constructible< Derived >::value,
                  "Derived must NOT be default constructible");
    static_assert(std::is_copy_constructible< Derived >::value,
                  "Derived must be copy constructible");
    static_assert(std::is_move_constructible< Derived >::value,
                  "Derived must be move constructible");
    static_assert(std::is_copy_assignable< Derived >::value,
                  "Derived must be copy assignable");
    static_assert(std::is_move_assignable< Derived >::value,
                  "Derived must be move assignable");
    // Derived must provide Derived::name()
    static_assert(std::is_same< decltype(Derived::name()), std::string >::value,
                  "Derived::name() does not exist");
  }

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

  /// \brief Normalize the abstract value
  virtual void normalize() = 0;

  /// \brief Check if the abstract value is bottom
  ///
  /// Consider calling `normalize()` before calling `is_bottom()`, otherwise
  /// it might normalize a temporary copy.
  virtual bool is_bottom() const = 0;

  /// \brief Check if the abstract value is top
  virtual bool is_top() const = 0;

  /// \brief Set the abstract value to bottom
  virtual void set_to_bottom() = 0;

  /// \brief Set the abstract value to top
  virtual void set_to_top() = 0;

  /// \brief Partial order comparison
  ///
  /// Check for the semantic inclusion.
  virtual bool leq(const Derived& other) const = 0;

  /// \brief Equality comparison
  ///
  /// This is semantically equivalent to `a.leq(b) && b.leq(a)`
  virtual bool equals(const Derived& other) const = 0;

  /// \brief Equality comparison
  bool operator==(const Derived& other) const { return this->equals(other); }

  /// \brief Inequality comparison
  bool operator!=(const Derived& other) const { return !this->equals(other); }

  /// \brief Perform the union of two abstract values
  ///
  /// If the abstract domain is a lattice, this is the least upper bound
  /// operation.
  virtual void join_with(Derived&& other) { this->join_with(other); }

  /// \brief Perform the union of two abstract values
  ///
  /// If the abstract domain is a lattice, this is the least upper bound
  /// operation.
  virtual void join_with(const Derived& other) = 0;

  /// \brief Perform a union on a loop head
  ///
  /// Example: `pre_in.join_loop_with(pre_back)`
  ///
  /// For most abstract domains, this is equivalent to join_with.
  virtual void join_loop_with(Derived&& other) {
    this->join_with(std::move(other));
  }

  /// \brief Perform a union on a loop head
  ///
  /// Example: `pre_in.join_loop_with(pre_back)`
  ///
  /// For most abstract domains, this is equivalent to join_with.
  virtual void join_loop_with(const Derived& other) { this->join_with(other); }

  /// \brief Perform a union on two consecutive iterations of a fix-point
  /// algorithm
  ///
  /// Example: `x(n).join_iter_with(x(n+1))`
  ///
  /// For most abstract domains, this is equivalent to join_with.
  virtual void join_iter_with(Derived&& other) {
    this->join_with(std::move(other));
  }

  /// \brief Perform a union on two consecutive iterations of a fix-point
  /// algorithm
  ///
  /// Example: `x(n).join_iter_with(x(n+1))`
  ///
  /// For most abstract domains, this is equivalent to join_with.
  virtual void join_iter_with(const Derived& other) { this->join_with(other); }

  /// \brief Perform the widening of two abstract values
  ///
  /// If the abstract domain has finite ascending chains, one doesn't need to
  /// define a widening operator and can simply use the join instead.
  virtual void widen_with(const Derived& other) = 0;

  /// \brief Perform the intersection of two abstract values
  ///
  /// If the abstract domain is a lattice, this is the greatest lower bound
  /// operation.
  virtual void meet_with(const Derived& other) = 0;

  /// \brief Perform the narrowing of two abstract values
  ///
  /// If the abstract domain has finite descending chains, one doesn't need to
  /// define a narrowing operator and can simply use the meet instead.
  virtual void narrow_with(const Derived& other) = 0;

  /// \brief Perform the union of two abstract values
  virtual Derived join(const Derived& other) const {
    Derived tmp(static_cast< const Derived& >(*this));
    tmp.join_with(other);
    return tmp;
  }

  /// \brief Perform a union on a loop head
  virtual Derived join_loop(const Derived& other) const {
    Derived tmp(static_cast< const Derived& >(*this));
    tmp.join_loop_with(other);
    return tmp;
  }

  /// \brief Perform a union on two consecutive iterations of a fix-point
  /// algorithm
  virtual Derived join_iter(const Derived& other) const {
    Derived tmp(static_cast< const Derived& >(*this));
    tmp.join_iter_with(other);
    return tmp;
  }

  /// \brief Perform the widening of two abstract values
  virtual Derived widening(const Derived& other) const {
    Derived tmp(static_cast< const Derived& >(*this));
    tmp.widen_with(other);
    return tmp;
  }

  /// \brief Perform the intersection of two abstract values
  virtual Derived meet(const Derived& other) const {
    Derived tmp(static_cast< const Derived& >(*this));
    tmp.meet_with(other);
    return tmp;
  }

  /// \brief Perform the narrowing of two abstract values
  virtual Derived narrowing(const Derived& other) const {
    Derived tmp(static_cast< const Derived& >(*this));
    tmp.narrow_with(other);
    return tmp;
  }

  /// \brief Dump the abstract value, for debugging purpose
  virtual void dump(std::ostream&) const = 0;

}; // end class AbstractDomain

/// \brief Check if a type is an abstract domain
template < typename T >
struct IsAbstractDomain : std::is_base_of< AbstractDomain< T >, T > {};

} // end namespace core
} // end namespace ikos
