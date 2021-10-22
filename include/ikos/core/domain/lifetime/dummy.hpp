/******************************************************************************
 *
 * \file
 * \brief A dummy lifetime abstract domain that is either top or bottom
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

namespace ikos {
namespace core {
namespace lifetime {

/// \brief Dummy lifetime abstract domain that is either top or bottom
///
/// This can be used to disable a lifetime analysis in a generic implementation
template < typename MemoryLocationRef >
class DummyDomain final
    : public lifetime::AbstractDomain< MemoryLocationRef,
                                       DummyDomain< MemoryLocationRef > > {
private:
  bool _is_bottom;

private:
  /// \brief Private constructor
  explicit DummyDomain(bool is_bottom) : _is_bottom(is_bottom) {}

public:
  /// \brief Create the top abstract value
  static DummyDomain top() { return DummyDomain(false); }

  /// \brief Create the bottom abstract value
  static DummyDomain bottom() { return DummyDomain(true); }

  /// \brief Copy constructor
  DummyDomain(const DummyDomain&) noexcept = default;

  /// \brief Move constructor
  DummyDomain(DummyDomain&&) noexcept = default;

  /// \brief Copy assignment operator
  DummyDomain& operator=(const DummyDomain&) noexcept = default;

  /// \brief Move assignment operator
  DummyDomain& operator=(DummyDomain&&) noexcept = default;

  /// \brief Destructor
  ~DummyDomain() override = default;

  void normalize() override {}

  bool is_bottom() const override { return this->_is_bottom; }

  bool is_top() const override { return !this->_is_bottom; }

  void set_to_bottom() override { this->_is_bottom = true; }

  void set_to_top() override { this->_is_bottom = false; }

  bool leq(const DummyDomain& other) const override {
    return static_cast< int >(this->_is_bottom) >=
           static_cast< int >(other._is_bottom);
  }

  bool equals(const DummyDomain& other) const override {
    return this->_is_bottom == other._is_bottom;
  }

  void join_with(const DummyDomain& other) override {
    this->_is_bottom = (this->_is_bottom && other._is_bottom);
  }

  void widen_with(const DummyDomain& other) override { this->join_with(other); }

  void meet_with(const DummyDomain& other) override {
    this->_is_bottom = (this->_is_bottom || other._is_bottom);
  }

  void narrow_with(const DummyDomain& other) override {
    this->meet_with(other);
  }

  void assign_allocated(MemoryLocationRef) override {}

  void assign_deallocated(MemoryLocationRef) override {}

  void assert_allocated(MemoryLocationRef) override {}

  void assert_deallocated(MemoryLocationRef) override {}

  bool is_allocated(MemoryLocationRef) const override {
    return this->_is_bottom;
  }

  bool is_deallocated(MemoryLocationRef) const override {
    return this->_is_bottom;
  }

  void set(MemoryLocationRef, const Lifetime& value) override {
    if (value.is_bottom()) {
      this->_is_bottom = true;
    }
  }

  void forget(MemoryLocationRef) override {}

  Lifetime get(MemoryLocationRef) const override {
    if (this->_is_bottom) {
      return Lifetime::bottom();
    } else {
      return Lifetime::top();
    }
  }

  void dump(std::ostream& o) const override {
    if (this->_is_bottom) {
      o << "⊥";
    } else {
      o << "T";
    }
  }

  static std::string name() { return "dummy lifetime domain"; }

}; // end class DummyDomain

} // end namespace lifetime
} // end namespace core
} // end namespace ikos
