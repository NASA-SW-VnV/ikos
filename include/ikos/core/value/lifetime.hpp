/*******************************************************************************
 *
 * \file
 * \brief Lifetime abstract value
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

#include <sstream>

#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/support/assert.hpp>

namespace ikos {
namespace core {

/// \brief Lifetime abstract value
///
/// This is either top, bottom, allocated or deallocated.
class Lifetime final : public core::AbstractDomain< Lifetime > {
private:
  enum Kind : unsigned {
    BottomKind = 0,
    AllocatedKind = 1,
    DeallocatedKind = 2,
    TopKind = 3
  };

private:
  Kind _kind = TopKind;

private:
  /// \brief Private constructor
  explicit Lifetime(Kind kind) : _kind(kind) {}

public:
  /// \brief Return the top lifetime value
  static Lifetime top() { return Lifetime(TopKind); }

  /// \brief Return the bottom lifetime value
  static Lifetime bottom() { return Lifetime(BottomKind); }

  /// \brief Return the deallocated lifetime value
  static Lifetime deallocated() { return Lifetime(DeallocatedKind); }

  /// \brief Return the allocated lifetime value
  static Lifetime allocated() { return Lifetime(AllocatedKind); }

  /// \brief Copy constructor
  Lifetime(const Lifetime&) noexcept = default;

  /// \brief Move constructor
  Lifetime(Lifetime&&) noexcept = default;

  /// \brief Copy assignment operator
  Lifetime& operator=(const Lifetime&) noexcept = default;

  /// \brief Move assignment operator
  Lifetime& operator=(Lifetime&&) noexcept = default;

  /// \brief Destructor
  ~Lifetime() override = default;

  void normalize() override {}

  bool is_bottom() const override { return this->_kind == BottomKind; }

  bool is_top() const override { return this->_kind == TopKind; }

  /// \brief Return true if this is the deallocated lifetime value
  bool is_deallocated() const { return this->_kind == DeallocatedKind; }

  /// \brief Return true if this is the allocated lifetime value
  bool is_allocated() const { return this->_kind == AllocatedKind; }

  void set_to_bottom() override { this->_kind = BottomKind; }

  void set_to_top() override { this->_kind = TopKind; }

  /// \brief Set the lifetime value to deallocated
  void set_to_deallocated() { this->_kind = DeallocatedKind; }

  /// \brief Set the lifetime value to allocated
  void set_to_allocated() { this->_kind = AllocatedKind; }

  bool leq(const Lifetime& other) const override {
    switch (this->_kind) {
      case BottomKind:
        return true;
      case TopKind:
        return other._kind == TopKind;
      case AllocatedKind:
        return other._kind == AllocatedKind || other._kind == TopKind;
      case DeallocatedKind:
        return other._kind == DeallocatedKind || other._kind == TopKind;
      default:
        ikos_unreachable("unreachable");
    }
  }

  bool equals(const Lifetime& other) const override {
    return this->_kind == other._kind;
  }

  void join_with(const Lifetime& other) override {
    this->_kind = static_cast< Kind >(static_cast< unsigned >(this->_kind) |
                                      static_cast< unsigned >(other._kind));
  }

  void widen_with(const Lifetime& other) override { this->join_with(other); }

  void meet_with(const Lifetime& other) override {
    this->_kind = static_cast< Kind >(static_cast< unsigned >(this->_kind) &
                                      static_cast< unsigned >(other._kind));
  }

  void narrow_with(const Lifetime& other) override { this->meet_with(other); }

  void dump(std::ostream& o) const override {
    switch (this->_kind) {
      case BottomKind: {
        o << "⊥";
        break;
      }
      case DeallocatedKind: {
        o << "DA";
        break;
      }
      case AllocatedKind: {
        o << "A";
        break;
      }
      case TopKind: {
        o << "T";
        break;
      }
      default: {
        ikos_unreachable("unreachable");
      }
    }
  }

  static std::string name() { return "lifetime"; }

}; // end class Lifetime

} // end namespace core
} // end namespace ikos
