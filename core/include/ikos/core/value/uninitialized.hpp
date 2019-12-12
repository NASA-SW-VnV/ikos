/*******************************************************************************
 *
 * \file
 * \brief Uninitialized abstract value
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

#include <sstream>

#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/support/assert.hpp>

namespace ikos {
namespace core {

/// \brief Uninitialized abstract value
///
/// This is either top, bottom, initialized or uninitialized.
class Uninitialized final : public core::AbstractDomain< Uninitialized > {
private:
  enum Kind : unsigned {
    BottomKind = 0,
    InitializedKind = 1,
    UninitializedKind = 2,
    TopKind = 3
  };

private:
  Kind _kind = TopKind;

private:
  /// \brief Private constructor
  explicit Uninitialized(Kind kind) : _kind(kind) {}

public:
  /// \brief Return the top uninitialized value
  static Uninitialized top() { return Uninitialized(TopKind); }

  /// \brief Return the bottom uninitialized value
  static Uninitialized bottom() { return Uninitialized(BottomKind); }

  /// \brief Return the initialized abstract value
  static Uninitialized initialized() { return Uninitialized(InitializedKind); }

  /// \brief Return the uninitialized abstract value
  static Uninitialized uninitialized() {
    return Uninitialized(UninitializedKind);
  }

  /// \brief Copy constructor
  Uninitialized(const Uninitialized&) noexcept = default;

  /// \brief Move constructor
  Uninitialized(Uninitialized&&) noexcept = default;

  /// \brief Copy assignment operator
  Uninitialized& operator=(const Uninitialized&) noexcept = default;

  /// \brief Move assignment operator
  Uninitialized& operator=(Uninitialized&&) noexcept = default;

  /// \brief Destructor
  ~Uninitialized() override = default;

  void normalize() override {}

  bool is_bottom() const override { return this->_kind == BottomKind; }

  bool is_top() const override { return this->_kind == TopKind; }

  /// \brief Return true if this is the initialized abstract value
  bool is_initialized() const { return this->_kind == InitializedKind; }

  /// \brief Return true if this is the uninitialized abstract value
  bool is_uninitialized() const { return this->_kind == UninitializedKind; }

  void set_to_bottom() override { this->_kind = BottomKind; }

  void set_to_top() override { this->_kind = TopKind; }

  /// \brief Set the uninitialized value to initialized
  void set_to_initialized() { this->_kind = InitializedKind; }

  /// \brief Set the uninitialized value to uninitialized
  void set_to_uninitialized() { this->_kind = UninitializedKind; }

  bool leq(const Uninitialized& other) const override {
    switch (this->_kind) {
      case BottomKind:
        return true;
      case TopKind:
        return other._kind == TopKind;
      case InitializedKind:
        return other._kind == InitializedKind || other._kind == TopKind;
      case UninitializedKind:
        return other._kind == UninitializedKind || other._kind == TopKind;
      default:
        ikos_unreachable("unreachable");
    }
  }

  bool equals(const Uninitialized& other) const override {
    return this->_kind == other._kind;
  }

  void join_with(const Uninitialized& other) override {
    this->_kind = static_cast< Kind >(static_cast< unsigned >(this->_kind) |
                                      static_cast< unsigned >(other._kind));
  }

  void widen_with(const Uninitialized& other) override {
    this->join_with(other);
  }

  void meet_with(const Uninitialized& other) override {
    this->_kind = static_cast< Kind >(static_cast< unsigned >(this->_kind) &
                                      static_cast< unsigned >(other._kind));
  }

  void narrow_with(const Uninitialized& other) override {
    this->meet_with(other);
  }

  void dump(std::ostream& o) const override {
    switch (this->_kind) {
      case BottomKind: {
        o << "⊥";
        break;
      }
      case InitializedKind: {
        o << "I";
        break;
      }
      case UninitializedKind: {
        o << "U";
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

  static std::string name() { return "uninitialized"; }

}; // end class Uninitialized

} // end namespace core
} // end namespace ikos
