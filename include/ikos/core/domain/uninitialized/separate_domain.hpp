/*******************************************************************************
 *
 * \file
 * \brief Implementation of an abstract domain keeping track of (un)initialized
 * variables using a separate domain
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

#include <ikos/core/domain/separate_domain.hpp>
#include <ikos/core/domain/uninitialized/abstract_domain.hpp>

namespace ikos {
namespace core {
namespace uninitialized {

/// \brief Uninitialized abstract domain
///
/// Implementation of the uninitialized abstract domain interface using
/// a separate domain.
template < typename VariableRef >
class SeparateDomain final
    : public uninitialized::AbstractDomain< VariableRef,
                                            SeparateDomain< VariableRef > > {
private:
  using SeparateDomainT = core::SeparateDomain< VariableRef, Uninitialized >;

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

  /// \brief Begin iterator over the pairs (variable, uninitialized)
  Iterator begin() const { return this->_inv.begin(); }

  /// \brief End iterator over the pairs (variable, uninitialized)
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

  void assign_initialized(VariableRef x) override {
    this->_inv.set(x, Uninitialized::initialized());
  }

  void assign_uninitialized(VariableRef x) override {
    this->_inv.set(x, Uninitialized::uninitialized());
  }

  void assign(VariableRef x, VariableRef y) override {
    this->_inv.set(x, this->_inv.get(y));
  }

  void assert_initialized(VariableRef x) override {
    this->_inv.refine(x, Uninitialized::initialized());
  }

  bool is_initialized(VariableRef x) const override {
    Uninitialized value = this->_inv.get(x);
    return value.is_bottom() || value.is_initialized();
  }

  bool is_uninitialized(VariableRef x) const override {
    Uninitialized value = this->_inv.get(x);
    return value.is_bottom() || value.is_uninitialized();
  }

  void set(VariableRef x, const Uninitialized& value) override {
    this->_inv.set(x, value);
  }

  void refine(VariableRef x, const Uninitialized& value) override {
    this->_inv.refine(x, value);
  }

  void forget(VariableRef x) override { this->_inv.forget(x); }

  Uninitialized get(VariableRef x) const override { return this->_inv.get(x); }

  void dump(std::ostream& o) const override { return this->_inv.dump(o); }

  static std::string name() { return "uninitialized domain"; }

}; // end class SeparateDomain

} // end namespace uninitialized
} // end namespace core
} // end namespace ikos
