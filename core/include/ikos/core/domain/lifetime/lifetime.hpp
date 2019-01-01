/*******************************************************************************
 *
 * \file
 * \brief Implementation of an abstract domain keeping track of memory location
 * lifetimes.
 *
 * Author: Thomas Bailleux
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

#include <ikos/core/domain/lifetime/abstract_domain.hpp>
#include <ikos/core/domain/separate_domain.hpp>

namespace ikos {
namespace core {
namespace lifetime {

/// \brief Lifetime abstract domain
///
/// Implementation of the lifetime abstract domain interface using
/// SeparateDomain
template < typename MemoryLocationRef >
class LifetimeDomain final
    : public lifetime::AbstractDomain< MemoryLocationRef,
                                       LifetimeDomain< MemoryLocationRef > > {
private:
  using SeparateDomainT = SeparateDomain< MemoryLocationRef, Lifetime >;

public:
  using Iterator = typename SeparateDomainT::Iterator;

private:
  SeparateDomainT _inv;

private:
  /// \brief Private constructor
  explicit LifetimeDomain(SeparateDomainT inv) : _inv(std::move(inv)) {}

public:
  /// \brief Create the top abstract value
  LifetimeDomain() : _inv(SeparateDomainT::top()) {}

  /// \brief Copy constructor
  LifetimeDomain(const LifetimeDomain&) = default;

  /// \brief Move constructor
  LifetimeDomain(LifetimeDomain&&) = default;

  /// \brief Copy assignment operator
  LifetimeDomain& operator=(const LifetimeDomain&) = default;

  /// \brief Move assignment operator
  LifetimeDomain& operator=(LifetimeDomain&&) = default;

  /// \brief Destructor
  ~LifetimeDomain() override = default;

  /// \brief Create the top abstract value
  static LifetimeDomain top() { return LifetimeDomain(SeparateDomainT::top()); }

  /// \brief Create the bottom abstract value
  static LifetimeDomain bottom() {
    return LifetimeDomain(SeparateDomainT::bottom());
  }

  /// \brief Begin iterator over the pairs (memory location, lifetime)
  Iterator begin() const { return this->_inv.begin(); }

  /// \brief End iterator over the pairs (memory location, lifetime)
  Iterator end() const { return this->_inv.end(); }

  bool is_bottom() const override { return this->_inv.is_bottom(); }

  bool is_top() const override { return this->_inv.is_top(); }

  void set_to_bottom() override { this->_inv.set_to_bottom(); }

  void set_to_top() override { this->_inv.set_to_top(); }

  bool leq(const LifetimeDomain& other) const override {
    return this->_inv.leq(other._inv);
  }

  bool equals(const LifetimeDomain& other) const override {
    return this->_inv.equals(other._inv);
  }

  void join_with(const LifetimeDomain& other) override {
    this->_inv.join_with(other._inv);
  }

  void widen_with(const LifetimeDomain& other) override {
    this->_inv.widen_with(other._inv);
  }

  void meet_with(const LifetimeDomain& other) override {
    this->_inv.meet_with(other._inv);
  }

  void narrow_with(const LifetimeDomain& other) override {
    this->_inv.narrow_with(other._inv);
  }

  void assign_allocated(MemoryLocationRef x) override {
    this->_inv.set(x, Lifetime::allocated());
  }

  void assign_deallocated(MemoryLocationRef x) override {
    this->_inv.set(x, Lifetime::deallocated());
  }

  void assert_allocated(MemoryLocationRef x) override {
    this->_inv.refine(x, Lifetime::allocated());
  }

  void assert_deallocated(MemoryLocationRef x) override {
    this->_inv.refine(x, Lifetime::deallocated());
  }

  bool is_allocated(MemoryLocationRef x) const override {
    ikos_assert_msg(!this->is_bottom(),
                    "trying to call is_allocated() on bottom");
    return this->_inv.get(x).is_allocated();
  }

  bool is_deallocated(MemoryLocationRef x) const override {
    ikos_assert_msg(!this->is_bottom(),
                    "trying to call is_deallocated() on bottom");
    return this->_inv.get(x).is_deallocated();
  }

  void set(MemoryLocationRef x, const Lifetime& value) override {
    this->_inv.set(x, value);
  }

  void forget(MemoryLocationRef x) override { this->_inv.forget(x); }

  void normalize() const override {}

  Lifetime get(MemoryLocationRef x) const override { return this->_inv.get(x); }

  void dump(std::ostream& o) const override { return this->_inv.dump(o); }

  static std::string name() { return "lifetime domain"; }

}; // end class LifetimeDomain

} // end namespace lifetime
} // end namespace core
} // end namespace ikos
