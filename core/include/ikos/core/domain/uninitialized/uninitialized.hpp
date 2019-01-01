/*******************************************************************************
 *
 * \file
 * \brief Implementation of an abstract domain keeping track of (un)initialized
 * variables
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
/// SeparateDomain
template < typename VariableRef >
class UninitializedDomain final
    : public uninitialized::
          AbstractDomain< VariableRef, UninitializedDomain< VariableRef > > {
private:
  using SeparateDomainT = SeparateDomain< VariableRef, Uninitialized >;

public:
  using Iterator = typename SeparateDomainT::Iterator;

private:
  SeparateDomainT _inv;

private:
  /// \brief Private constructor
  explicit UninitializedDomain(SeparateDomainT inv) : _inv(std::move(inv)) {}

public:
  /// \brief Create the top abstract value
  UninitializedDomain() : _inv(SeparateDomainT::top()) {}

  /// \brief Copy constructor
  UninitializedDomain(const UninitializedDomain&) = default;

  /// \brief Move constructor
  UninitializedDomain(UninitializedDomain&&) = default;

  /// \brief Copy assignment operator
  UninitializedDomain& operator=(const UninitializedDomain&) = default;

  /// \brief Move assignment operator
  UninitializedDomain& operator=(UninitializedDomain&&) = default;

  /// \brief Destructor
  ~UninitializedDomain() override = default;

  /// \brief Create the top abstract value
  static UninitializedDomain top() {
    return UninitializedDomain(SeparateDomainT::top());
  }

  /// \brief Create the bottom abstract value
  static UninitializedDomain bottom() {
    return UninitializedDomain(SeparateDomainT::bottom());
  }

  /// \brief Begin iterator over the pairs (variable, uninitialized)
  Iterator begin() const { return this->_inv.begin(); }

  /// \brief End iterator over the pairs (variable, uninitialized)
  Iterator end() const { return this->_inv.end(); }

  bool is_bottom() const override { return this->_inv.is_bottom(); }

  bool is_top() const override { return this->_inv.is_top(); }

  void set_to_bottom() override { this->_inv.set_to_bottom(); }

  void set_to_top() override { this->_inv.set_to_top(); }

  bool leq(const UninitializedDomain& other) const override {
    return this->_inv.leq(other._inv);
  }

  bool equals(const UninitializedDomain& other) const override {
    return this->_inv.equals(other._inv);
  }

  void join_with(const UninitializedDomain& other) override {
    this->_inv.join_with(other._inv);
  }

  void widen_with(const UninitializedDomain& other) override {
    this->_inv.widen_with(other._inv);
  }

  void meet_with(const UninitializedDomain& other) override {
    this->_inv.meet_with(other._inv);
  }

  void narrow_with(const UninitializedDomain& other) override {
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

  void assign(VariableRef x, std::initializer_list< VariableRef > l) override {
    if (this->is_bottom()) {
      return;
    }

    bool has_top = false;
    bool has_uninitialized = false;

    for (VariableRef y : l) {
      Uninitialized yv = this->_inv.get(y);

      if (yv.is_top()) {
        has_top = true;
      } else if (yv.is_uninitialized()) {
        has_uninitialized = true;
      } else {
        ikos_assert(yv.is_initialized());
      }
    }

    if (has_top) {
      this->_inv.set(x, Uninitialized::top());
    } else if (has_uninitialized) {
      this->_inv.set(x, Uninitialized::uninitialized());
    } else {
      this->_inv.set(x, Uninitialized::initialized());
    }
  }

  bool is_initialized(VariableRef x) const override {
    ikos_assert_msg(!this->is_bottom(),
                    "trying to call is_initialized() on bottom");
    return this->_inv.get(x).is_initialized();
  }

  bool is_uninitialized(VariableRef x) const override {
    ikos_assert_msg(!this->is_bottom(),
                    "trying to call is_uninitialized() on bottom");
    return this->_inv.get(x).is_uninitialized();
  }

  void set(VariableRef x, const Uninitialized& value) override {
    this->_inv.set(x, value);
  }

  void refine(VariableRef x, const Uninitialized& value) override {
    this->_inv.refine(x, value);
  }

  void forget(VariableRef x) override { this->_inv.forget(x); }

  void normalize() const override {}

  Uninitialized get(VariableRef x) const override { return this->_inv.get(x); }

  void dump(std::ostream& o) const override { return this->_inv.dump(o); }

  static std::string name() { return "uninitialized domain"; }

}; // end class UninitializedDomain

} // end namespace uninitialized
} // end namespace core
} // end namespace ikos
