/*******************************************************************************
 *
 * \file
 * \brief Pointer set abstract domain
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
#include <ikos/core/value/pointer/pointer.hpp>
#include <ikos/core/value/pointer/points_to_set.hpp>
#include <ikos/core/value/uninitialized.hpp>

namespace ikos {
namespace core {

/// \brief Pointer set abstract domain
///
/// The pointer set abstraction holds:
///   * A points-to set of memory locations pointed by the pointers
///   * An interval representing the offsets
///
/// It does not handle null and uninitialized pointers.
template < typename MemoryLocationRef >
class PointerSet final
    : public core::AbstractDomain< PointerSet< MemoryLocationRef > > {
public:
  using PointsToSetT = PointsToSet< MemoryLocationRef >;
  using PointerAbsValueT = PointerAbsValue< MemoryLocationRef >;
  using MachineIntInterval = machine_int::Interval;

private:
  /// \brief Set of memory locations (i.e, addresses) pointed by the pointers
  PointsToSetT _points_to;

  /// \brief Offsets interval
  MachineIntInterval _offsets;

private:
  /// \brief Reduce the pointer set
  void reduce() {
    if (this->_points_to.is_bottom() || this->_points_to.is_empty()) {
      this->_offsets.set_to_bottom();
    } else if (this->_offsets.is_bottom()) {
      this->_points_to.set_to_bottom();
    }
  }

public:
  /// \brief Create the top pointer set with the given bit-width and signedness
  /// for the offsets
  static PointerSet top(uint64_t bit_width, Signedness sign) {
    return PointerSet(PointsToSetT::top(),
                      MachineIntInterval::top(bit_width, sign));
  }

  /// \brief Create the bottom pointer set with the given bit-width and
  /// signedness for the offsets
  static PointerSet bottom(uint64_t bit_width, Signedness sign) {
    return PointerSet(PointsToSetT::bottom(),
                      MachineIntInterval::bottom(bit_width, sign));
  }

  /// \brief Create the empty pointer set with the given bit-width and
  /// signedness for the offsets
  static PointerSet empty(uint64_t bit_width, Signedness sign) {
    return PointerSet(PointsToSetT::empty(),
                      MachineIntInterval::bottom(bit_width, sign));
  }

  /// \brief Create the pointer set with the given points-to set and interval
  PointerSet(PointsToSetT points_to, MachineIntInterval offsets)
      : _points_to(std::move(points_to)), _offsets(std::move(offsets)) {
    this->reduce();
  }

  /// \brief Copy constructor
  PointerSet(const PointerSet&) = default;

  /// \brief Move constructor
  PointerSet(PointerSet&&) noexcept = default;

  /// \brief Copy assignment operator
  PointerSet& operator=(const PointerSet&) = default;

  /// \brief Move assignment operator
  PointerSet& operator=(PointerSet&&) noexcept = default;

  /// \brief Destructor
  ~PointerSet() override = default;

  /// \brief Return the uninitialized
  Uninitialized uninitialized() const { return Uninitialized::top(); }

  /// \brief Return the nullity
  Nullity nullity() const { return Nullity::top(); }

  /// \brief Return the points-to set
  const PointsToSetT& points_to() const { return this->_points_to; }

  /// \brief Return the interval offsets
  const MachineIntInterval& offsets() const { return this->_offsets; }

  void normalize() override {
    // Already performed by the reduction
  }

  bool is_bottom() const override {
    return this->_points_to.is_bottom(); // Correct because of reduction
  }

  bool is_top() const override {
    return this->_points_to.is_top() && this->_offsets.is_top();
  }

  void set_to_bottom() override {
    this->_points_to.set_to_bottom();
    this->_offsets.set_to_bottom();
  }

  void set_to_top() override {
    this->_points_to.set_to_top();
    this->_offsets.set_to_top();
  }

  bool leq(const PointerSet& other) const override {
    return this->_points_to.leq(other._points_to) &&
           this->_offsets.leq(other._offsets);
  }

  bool equals(const PointerSet& other) const override {
    return this->_points_to.equals(other._points_to) &&
           this->_offsets.equals(other._offsets);
  }

  void join_with(const PointerSet& other) override {
    this->_points_to.join_with(other._points_to);
    this->_offsets.join_with(other._offsets);
    this->reduce();
  }

  void join_loop_with(const PointerSet& other) override {
    this->_points_to.join_loop_with(other._points_to);
    this->_offsets.join_loop_with(other._offsets);
    this->reduce();
  }

  void join_iter_with(const PointerSet& other) override {
    this->_points_to.join_iter_with(other._points_to);
    this->_offsets.join_iter_with(other._offsets);
    this->reduce();
  }

  void widen_with(const PointerSet& other) override {
    this->_points_to.widen_with(other._points_to);
    this->_offsets.widen_with(other._offsets);
    this->reduce();
  }

  void meet_with(const PointerSet& other) override {
    this->_points_to.meet_with(other._points_to);
    this->_offsets.meet_with(other._offsets);
    this->reduce();
  }

  void narrow_with(const PointerSet& other) override {
    this->_points_to.narrow_with(other._points_to);
    this->_offsets.narrow_with(other._offsets);
    this->reduce();
  }

  /// \brief Add a pointer abstract value in the set
  void add(const PointerAbsValueT& pointer) {
    this->_points_to.join_with(pointer.points_to());
    this->_offsets.join_with(pointer.offset());
    this->reduce();
  }

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else if (this->is_top()) {
      o << "T";
    } else if (this->_points_to.is_empty()) {
      o << "{}";
    } else {
      this->_points_to.dump(o);
      o << " + ";
      this->_offsets.dump(o);
    }
  }

  static std::string name() { return "pointer set abstraction"; }

}; // end class PointerSet

/// \brief Write a pointer set on a stream
template < typename MemoryLocationRef >
inline std::ostream& operator<<(
    std::ostream& o, const PointerSet< MemoryLocationRef >& pointer_set) {
  pointer_set.dump(o);
  return o;
}

} // end namespace core
} // end namespace ikos
