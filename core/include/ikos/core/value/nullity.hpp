/*******************************************************************************
 *
 * \file
 * \brief Nullity abstract value
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

/// \brief Nullity abstract value
///
/// This is either top, bottom, non-null or null.
class Nullity final : public core::AbstractDomain< Nullity > {
private:
  enum Kind : unsigned {
    BottomKind = 0,
    NullKind = 1,
    NonNullKind = 2,
    TopKind = 3
  };

private:
  Kind _kind = TopKind;

private:
  /// \brief Private constructor
  explicit Nullity(Kind kind) : _kind(kind) {}

public:
  /// \brief Return the top nullity value
  static Nullity top() { return Nullity(TopKind); }

  /// \brief Return the bottom nullity value
  static Nullity bottom() { return Nullity(BottomKind); }

  /// \brief Return the non-null nullity value
  static Nullity non_null() { return Nullity(NonNullKind); }

  /// \brief Return the null nullity value
  static Nullity null() { return Nullity(NullKind); }

  /// \brief Copy constructor
  Nullity(const Nullity&) noexcept = default;

  /// \brief Move constructor
  Nullity(Nullity&&) noexcept = default;

  /// \brief Copy assignment operator
  Nullity& operator=(const Nullity&) noexcept = default;

  /// \brief Move assignment operator
  Nullity& operator=(Nullity&&) noexcept = default;

  /// \brief Destructor
  ~Nullity() override = default;

  void normalize() override {}

  bool is_bottom() const override { return this->_kind == BottomKind; }

  bool is_top() const override { return this->_kind == TopKind; }

  /// \brief Return true if this is the non-null nullity value
  bool is_non_null() const { return this->_kind == NonNullKind; }

  /// \brief Return true if this is the null nullity value
  bool is_null() const { return this->_kind == NullKind; }

  void set_to_bottom() override { this->_kind = BottomKind; }

  void set_to_top() override { this->_kind = TopKind; }

  /// \brief Set the nullity value to non-null
  void set_to_non_null() { this->_kind = NonNullKind; }

  /// \brief Set the nullity value to null
  void set_to_null() { this->_kind = NullKind; }

  bool leq(const Nullity& other) const override {
    switch (this->_kind) {
      case BottomKind:
        return true;
      case TopKind:
        return other._kind == TopKind;
      case NullKind:
        return other._kind == NullKind || other._kind == TopKind;
      case NonNullKind:
        return other._kind == NonNullKind || other._kind == TopKind;
      default:
        ikos_unreachable("unreachable");
    }
  }

  bool equals(const Nullity& other) const override {
    return this->_kind == other._kind;
  }

  void join_with(const Nullity& other) override {
    this->_kind = static_cast< Kind >(static_cast< unsigned >(this->_kind) |
                                      static_cast< unsigned >(other._kind));
  }

  void widen_with(const Nullity& other) override { this->join_with(other); }

  void meet_with(const Nullity& other) override {
    this->_kind = static_cast< Kind >(static_cast< unsigned >(this->_kind) &
                                      static_cast< unsigned >(other._kind));
  }

  void narrow_with(const Nullity& other) override { this->meet_with(other); }

  void dump(std::ostream& o) const override {
    switch (this->_kind) {
      case BottomKind: {
        o << "⊥";
        break;
      }
      case NullKind: {
        o << "N";
        break;
      }
      case NonNullKind: {
        o << "NN";
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

  static std::string name() { return "nullity"; }

}; // end class Nullity

} // end namespace core
} // end namespace ikos
