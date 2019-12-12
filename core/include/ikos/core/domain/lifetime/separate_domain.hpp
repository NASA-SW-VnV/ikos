/*******************************************************************************
 *
 * \file
 * \brief Implementation of an abstract domain keeping track of memory location
 * lifetimes using a separate domain.
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

/// \brief Lifetime separate abstract domain
///
/// Implementation of the lifetime abstract domain interface using
/// a separate domain.
template < typename MemoryLocationRef >
class SeparateDomain final
    : public lifetime::AbstractDomain< MemoryLocationRef,
                                       SeparateDomain< MemoryLocationRef > > {
private:
  using SeparateDomainT = core::SeparateDomain< MemoryLocationRef, Lifetime >;

public:
  using Iterator = typename SeparateDomainT::Iterator;

private:
  SeparateDomainT _inv;

private:
  /// \brief Private constructor
  explicit SeparateDomain(SeparateDomainT inv) : _inv(std::move(inv)) {}

public:
  /// \brief Create the top abstract value
  static SeparateDomain top() { return SeparateDomain(SeparateDomainT::top()); }

  /// \brief Create the bottom abstract value
  static SeparateDomain bottom() {
    return SeparateDomain(SeparateDomainT::bottom());
  }

  /// \brief Copy constructor
  SeparateDomain(const SeparateDomain&) noexcept = default;

  /// \brief Move constructor
  SeparateDomain(SeparateDomain&&) noexcept = default;

  /// \brief Copy assignment operator
  SeparateDomain& operator=(const SeparateDomain&) noexcept = default;

  /// \brief Move assignment operator
  SeparateDomain& operator=(SeparateDomain&&) noexcept = default;

  /// \brief Destructor
  ~SeparateDomain() override = default;

  /// \brief Begin iterator over the pairs (memory location, lifetime)
  Iterator begin() const { return this->_inv.begin(); }

  /// \brief End iterator over the pairs (memory location, lifetime)
  Iterator end() const { return this->_inv.end(); }

  void normalize() override {}

  bool is_bottom() const override { return this->_inv.is_bottom(); }

  bool is_top() const override { return this->_inv.is_top(); }

  void set_to_bottom() override { this->_inv.set_to_bottom(); }

  void set_to_top() override { this->_inv.set_to_top(); }

  bool leq(const SeparateDomain& other) const override {
    return this->_inv.leq(other._inv);
  }

  bool equals(const SeparateDomain& other) const override {
    return this->_inv.equals(other._inv);
  }

  void join_with(const SeparateDomain& other) override {
    this->_inv.join_with(other._inv);
  }

  void widen_with(const SeparateDomain& other) override {
    this->_inv.widen_with(other._inv);
  }

  void meet_with(const SeparateDomain& other) override {
    this->_inv.meet_with(other._inv);
  }

  void narrow_with(const SeparateDomain& other) override {
    this->_inv.narrow_with(other._inv);
  }

  void assign_allocated(MemoryLocationRef m) override {
    this->_inv.set(m, Lifetime::allocated());
  }

  void assign_deallocated(MemoryLocationRef m) override {
    this->_inv.set(m, Lifetime::deallocated());
  }

  void assert_allocated(MemoryLocationRef m) override {
    this->_inv.refine(m, Lifetime::allocated());
  }

  void assert_deallocated(MemoryLocationRef m) override {
    this->_inv.refine(m, Lifetime::deallocated());
  }

  bool is_allocated(MemoryLocationRef m) const override {
    Lifetime value = this->_inv.get(m);
    return value.is_bottom() || value.is_allocated();
  }

  bool is_deallocated(MemoryLocationRef m) const override {
    Lifetime value = this->_inv.get(m);
    return value.is_bottom() || value.is_deallocated();
  }

  void set(MemoryLocationRef m, const Lifetime& value) override {
    this->_inv.set(m, value);
  }

  void forget(MemoryLocationRef m) override { this->_inv.forget(m); }

  Lifetime get(MemoryLocationRef m) const override { return this->_inv.get(m); }

  void dump(std::ostream& o) const override { return this->_inv.dump(o); }

  static std::string name() { return "lifetime domain"; }

}; // end class SeparateDomain

} // end namespace lifetime
} // end namespace core
} // end namespace ikos
