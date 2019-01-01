/*******************************************************************************
 *
 * \file
 * \brief Pointer abstract value
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

#include <ikos/core/value/machine_int/interval.hpp>
#include <ikos/core/value/nullity.hpp>
#include <ikos/core/value/pointer/points_to_set.hpp>
#include <ikos/core/value/uninitialized.hpp>

namespace ikos {
namespace core {

/// \brief Pointer abstract value
///
/// The pointer abstraction holds:
///   * A points-to set of memory locations pointed by the pointer
///   * An interval representing the offset
///   * A nullity abstract value
///   * An uninitialized abstract value
template < typename MemoryLocationRef >
class PointerAbsValue final
    : public core::AbstractDomain< PointerAbsValue< MemoryLocationRef > > {
public:
  using PointsToSetT = PointsToSet< MemoryLocationRef >;
  using MachineIntInterval = machine_int::Interval;

private:
  /// \brief Set of memory locations (i.e, addresses) pointed by the pointer
  PointsToSetT _points_to;

  /// \brief Offset interval
  MachineIntInterval _offset;

  /// \brief Nullity
  Nullity _nullity;

  /// \brief Uninitialized
  Uninitialized _uninitialized;

private:
  /// \brief Normalize the pointer abstract value
  void normalize() {
    if (this->_points_to.is_bottom() || this->_nullity.is_bottom() ||
        this->_uninitialized.is_bottom()) {
      this->set_to_bottom();
    } else if (this->_nullity.is_null()) {
      this->set_to_null();
    } else if (this->_uninitialized.is_uninitialized()) {
      this->set_to_uninitialized();
    } else if (this->_points_to.is_empty()) {
      this->_offset.set_to_bottom();
    }
  }

public:
  /// \brief Create the top abstract value
  PointerAbsValue()
      : _points_to(PointsToSetT::top()),
        _offset(MachineIntInterval::top()),
        _nullity(Nullity::top()),
        _uninitialized(Uninitialized::top()) {}

  /// \brief Create the abstract value with the given points-to set, interval,
  /// nullity and uninitialized
  PointerAbsValue(PointsToSetT points_to,
                  MachineIntInterval offset,
                  Nullity nullity,
                  Uninitialized uninitialized)
      : _points_to(std::move(points_to)),
        _offset(std::move(offset)),
        _nullity(std::move(nullity)),
        _uninitialized(std::move(uninitialized)) {
    this->normalize();
  }

  /// \brief Copy constructor
  PointerAbsValue(const PointerAbsValue&) = default;

  /// \brief Move constructor
  PointerAbsValue(PointerAbsValue&&) = default;

  /// \brief Copy assignment operator
  PointerAbsValue& operator=(const PointerAbsValue&) = default;

  /// \brief Move assignment operator
  PointerAbsValue& operator=(PointerAbsValue&&) = default;

  /// \brief Destructor
  ~PointerAbsValue() override = default;

  /// \brief Create the top abstract value
  static PointerAbsValue top() {
    return PointerAbsValue(PointsToSetT::top(),
                           MachineIntInterval::top(),
                           Nullity::top(),
                           Uninitialized::top());
  }

  /// \brief Create the bottom abstract value
  static PointerAbsValue bottom() {
    return PointerAbsValue(PointsToSetT::bottom(),
                           MachineIntInterval::bottom(),
                           Nullity::bottom(),
                           Uninitialized::bottom());
  }

  /// \brief Create the top abstract value with the given bit-width and
  /// signedness for the offset
  static PointerAbsValue top(unsigned bit_width, Signedness sign) {
    return PointerAbsValue(PointsToSetT::top(),
                           MachineIntInterval::top(bit_width, sign),
                           Nullity::top(),
                           Uninitialized::top());
  }

  /// \brief Create the bottom abstract value with the given bit-width and
  /// signedness for the offset
  static PointerAbsValue bottom(unsigned bit_width, Signedness sign) {
    return PointerAbsValue(PointsToSetT::bottom(),
                           MachineIntInterval::bottom(bit_width, sign),
                           Nullity::bottom(),
                           Uninitialized::bottom());
  }

  /// \brief Create the null pointer with the given bit-width and signedness for
  /// the offset
  static PointerAbsValue null(unsigned bit_width, Signedness sign) {
    return PointerAbsValue(PointsToSetT::empty(),
                           MachineIntInterval::bottom(bit_width, sign),
                           Nullity::null(),
                           Uninitialized::initialized());
  }

  /// \brief Create the uninitialized pointer with the given bit-width and
  /// signedness for the offset
  static PointerAbsValue uninitialized(unsigned bit_width, Signedness sign) {
    return PointerAbsValue(PointsToSetT::empty(),
                           MachineIntInterval::bottom(bit_width, sign),
                           Nullity::top(),
                           Uninitialized::uninitialized());
  }

  /// \brief Return the points-to set
  const PointsToSetT& points_to() const { return this->_points_to; }

  /// \brief Return the interval offset
  const MachineIntInterval& offset() const { return this->_offset; }

  /// \brief Return the nullity
  const Nullity& nullity() const { return this->_nullity; }

  /// \brief Return the uninitialized
  const Uninitialized& uninitialized() const { return this->_uninitialized; }

  bool is_bottom() const override {
    return this->_points_to.is_bottom(); // Correct because of normalization
  }

  bool is_top() const override {
    return this->_points_to.is_top() && this->_offset.is_top() &&
           this->_nullity.is_top() && this->_uninitialized.is_top();
  }

  /// \brief Return true if the pointer is uninitialized
  bool is_uninitialized() const {
    return this->_uninitialized.is_uninitialized();
  }

  /// \brief Return true if the pointer is null
  bool is_null() const { return this->_nullity.is_null(); }

  void set_to_bottom() override {
    this->_points_to.set_to_bottom();
    this->_offset.set_to_bottom();
    this->_nullity.set_to_bottom();
    this->_uninitialized.set_to_bottom();
  }

  void set_to_top() override {
    this->_points_to.set_to_top();
    this->_offset.set_to_top();
    this->_nullity.set_to_top();
    this->_uninitialized.set_to_top();
  }

  /// \brief Set the pointer to null
  void set_to_null() {
    this->_points_to.set_to_empty();
    this->_offset.set_to_bottom();
    this->_nullity.set_to_null();
    this->_uninitialized.set_to_initialized();
  }

  /// \brief Set the pointer to uninitialized
  void set_to_uninitialized() {
    this->_points_to.set_to_empty();
    this->_offset.set_to_bottom();
    this->_nullity.set_to_top();
    this->_uninitialized.set_to_uninitialized();
  }

  bool leq(const PointerAbsValue& other) const override {
    return this->_points_to.leq(other._points_to) &&
           this->_offset.leq(other._offset) &&
           this->_nullity.leq(other._nullity) &&
           this->_uninitialized.leq(other._uninitialized);
  }

  bool equals(const PointerAbsValue& other) const override {
    return this->_points_to.equals(other._points_to) &&
           this->_offset.equals(other._offset) &&
           this->_nullity.equals(other._nullity) &&
           this->_uninitialized.equals(other._uninitialized);
  }

  void join_with(const PointerAbsValue& other) override {
    this->_points_to.join_with(other._points_to);
    this->_offset.join_with(other._offset);
    this->_nullity.join_with(other._nullity);
    this->_uninitialized.join_with(other._uninitialized);
    this->normalize();
  }

  void join_loop_with(const PointerAbsValue& other) override {
    this->_points_to.join_loop_with(other._points_to);
    this->_offset.join_loop_with(other._offset);
    this->_nullity.join_loop_with(other._nullity);
    this->_uninitialized.join_loop_with(other._uninitialized);
    this->normalize();
  }

  void join_iter_with(const PointerAbsValue& other) override {
    this->_points_to.join_iter_with(other._points_to);
    this->_offset.join_iter_with(other._offset);
    this->_nullity.join_iter_with(other._nullity);
    this->_uninitialized.join_iter_with(other._uninitialized);
    this->normalize();
  }

  void widen_with(const PointerAbsValue& other) override {
    this->_points_to.widen_with(other._points_to);
    this->_offset.widen_with(other._offset);
    this->_nullity.widen_with(other._nullity);
    this->_uninitialized.widen_with(other._uninitialized);
    this->normalize();
  }

  void meet_with(const PointerAbsValue& other) override {
    this->_points_to.meet_with(other._points_to);
    this->_offset.meet_with(other._offset);
    this->_nullity.meet_with(other._nullity);
    this->_uninitialized.meet_with(other._uninitialized);
    this->normalize();
  }

  void narrow_with(const PointerAbsValue& other) override {
    this->_points_to.narrow_with(other._points_to);
    this->_offset.narrow_with(other._offset);
    this->_nullity.narrow_with(other._nullity);
    this->_uninitialized.narrow_with(other._uninitialized);
    this->normalize();
  }

  /// \brief Add an offset to the pointer
  void add_offset(const MachineIntInterval& offset) {
    this->_offset = add(this->_offset, offset);
  }

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else if (this->is_uninitialized()) {
      o << "U";
    } else if (this->is_null()) {
      o << "N";
    } else {
      this->_points_to.dump(o);
      o << " + ";
      this->_offset.dump(o);
      o << ", ";
      this->_nullity.dump(o);
      o << ", ";
      this->_uninitialized.dump(o);
    }
  }

  static std::string name() { return "pointer abstraction"; }

}; // end class PointerAbsValue

/// \brief Write a pointer abstract value on a stream
template < typename MemoryLocationRef >
inline std::ostream& operator<<(
    std::ostream& o, const PointerAbsValue< MemoryLocationRef >& pointer) {
  pointer.dump(o);
  return o;
}

} // end namespace core
} // end namespace ikos
