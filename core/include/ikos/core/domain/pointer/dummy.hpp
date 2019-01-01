/*******************************************************************************
 *
 * \file
 * \brief A dummy pointer abstract domain
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

#include <ikos/core/domain/pointer/abstract_domain.hpp>

namespace ikos {
namespace core {
namespace pointer {

/// \brief Dummy pointer abstract domain
///
/// This class holds:
///   * A machine integer abstract domain
///   * A nullity abstract domain
template < typename VariableRef,
           typename MemoryLocationRef,
           typename MachineIntDomain,
           typename NullityDomain >
class DummyDomain final
    : public pointer::AbstractDomain< VariableRef,
                                      MemoryLocationRef,
                                      MachineIntDomain,
                                      NullityDomain,
                                      DummyDomain< VariableRef,
                                                   MemoryLocationRef,
                                                   MachineIntDomain,
                                                   NullityDomain > > {
public:
  using LinearExpressionT = LinearExpression< MachineInt, VariableRef >;
  using PointsToSetT = PointsToSet< MemoryLocationRef >;
  using PointerAbsValueT = PointerAbsValue< MemoryLocationRef >;
  using PointerSetT = PointerSet< MemoryLocationRef >;

private:
  MachineIntDomain _machine_int;
  NullityDomain _nullity;

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top abstract value
  explicit DummyDomain(TopTag)
      : _machine_int(MachineIntDomain::top()), _nullity(NullityDomain::top()) {}

  /// \brief Create the bottom abstract value
  explicit DummyDomain(BottomTag)
      : _machine_int(MachineIntDomain::bottom()),
        _nullity(NullityDomain::bottom()) {}

public:
  /// \brief Create the top abstract value
  DummyDomain() : DummyDomain(TopTag{}) {}

  /// \brief Copy constructor
  DummyDomain(const DummyDomain&) = default;

  /// \brief Move constructor
  DummyDomain(DummyDomain&&) = default;

  /// \brief Copy assignment operator
  DummyDomain& operator=(const DummyDomain&) = default;

  /// \brief Move assignment operator
  DummyDomain& operator=(DummyDomain&&) = default;

  /// \brief Destructor
  ~DummyDomain() override = default;

  /// \brief Create the top abstract value
  static DummyDomain top() { return DummyDomain(TopTag{}); }

  /// \brief Create the bottom abstract value
  static DummyDomain bottom() { return DummyDomain(BottomTag{}); }

  bool is_bottom() const override {
    this->normalize();
    return this->_nullity.is_bottom(); // Correct because of normalization
  }

  bool is_top() const override {
    return this->_machine_int.is_top() && this->_nullity.is_top();
  }

  void set_to_bottom() override {
    this->_machine_int.set_to_bottom();
    this->_nullity.set_to_bottom();
  }

  void set_to_top() override {
    this->_machine_int.set_to_top();
    this->_nullity.set_to_top();
  }

  bool leq(const DummyDomain& other) const override {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_machine_int.leq(other._machine_int) &&
             this->_nullity.leq(other._nullity);
    }
  }

  bool equals(const DummyDomain& other) const override {
    if (this->is_bottom()) {
      return other.is_bottom();
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_machine_int.equals(other._machine_int) &&
             this->_nullity.equals(other._nullity);
    }
  }

  void join_with(const DummyDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_machine_int.join_with(other._machine_int);
      this->_nullity.join_with(other._nullity);
    }
  }

  void join_loop_with(const DummyDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_machine_int.join_loop_with(other._machine_int);
      this->_nullity.join_loop_with(other._nullity);
    }
  }

  void join_iter_with(const DummyDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_machine_int.join_iter_with(other._machine_int);
      this->_nullity.join_iter_with(other._nullity);
    }
  }

  void widen_with(const DummyDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_machine_int.widen_with(other._machine_int);
      this->_nullity.widen_with(other._nullity);
    }
  }

  void widen_threshold_with(const DummyDomain& other,
                            const MachineInt& threshold) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_machine_int.widen_threshold_with(other._machine_int, threshold);
      this->_nullity.widen_with(other._nullity);
    }
  }

  void meet_with(const DummyDomain& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_machine_int.meet_with(other._machine_int);
      this->_nullity.meet_with(other._nullity);
    }
  }

  void narrow_with(const DummyDomain& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_machine_int.narrow_with(other._machine_int);
      this->_nullity.narrow_with(other._nullity);
    }
  }

  MachineIntDomain& integers() override { return this->_machine_int; }

  const MachineIntDomain& integers() const override {
    return this->_machine_int;
  }

  NullityDomain& nullity() override { return this->_nullity; }

  const NullityDomain& nullity() const override { return this->_nullity; }

  void assign_address(VariableRef p,
                      MemoryLocationRef,
                      Nullity nullity) override {
    this->_nullity.set(p, nullity);
  }

  void assign_null(VariableRef p) override {
    this->_nullity.set(p, Nullity::null());
  }

  void assign_undef(VariableRef p) override { this->_nullity.forget(p); }

  void assign(VariableRef p, VariableRef q) override {
    this->_nullity.assign(p, q);
  }

  void assign(VariableRef p, VariableRef q, VariableRef) override {
    this->_nullity.assign(p, q);
  }

  void assign(VariableRef p, VariableRef q, const MachineInt&) override {
    this->_nullity.assign(p, q);
  }

  void assign(VariableRef p, VariableRef q, const LinearExpressionT&) override {
    this->_nullity.assign(p, q);
  }

  void assert_null(VariableRef p) override { this->_nullity.assert_null(p); }

  void assert_non_null(VariableRef p) override {
    this->_nullity.assert_non_null(p);
  }

  void add(Predicate pred, VariableRef p, VariableRef q) override {
    this->_nullity.add(pred, p, q);
  }

  void refine(VariableRef, const PointsToSetT&) override {}

  void refine(VariableRef,
              const PointsToSetT&,
              const machine_int::Interval&) override {}

  void refine(VariableRef, const PointerAbsValueT&) override {}

  void refine(VariableRef, const PointerSetT&) override {}

  PointsToSetT points_to(VariableRef) const override {
    if (this->is_bottom()) {
      return PointsToSetT::bottom();
    } else {
      return PointsToSetT::top();
    }
  }

  PointerAbsValueT get(VariableRef p) const override {
    VariableRef var = this->offset_var(p);
    auto bit_width = machine_int::VariableTraits< VariableRef >::bit_width(var);
    auto sign = machine_int::VariableTraits< VariableRef >::sign(var);

    if (this->is_bottom()) {
      return PointerAbsValueT::bottom(bit_width, sign);
    } else {
      return PointerAbsValueT::top(bit_width, sign);
    }
  }

  void forget(VariableRef p) override { this->_nullity.forget(p); }

  void normalize() const override {
    // is_bottom() will normalize
    if (this->_machine_int.is_bottom() || this->_nullity.is_bottom()) {
      const_cast< DummyDomain* >(this)->set_to_bottom();
    }
  }

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else if (this->is_top()) {
      o << "T";
    } else {
      o << "(";
      this->_machine_int.dump(o);
      o << ", ";
      this->_nullity.dump(o);
      o << ")";
    }
  }

  static std::string name() {
    return "dummy pointer domain with " + MachineIntDomain::name() + ", " +
           NullityDomain::name();
  }

}; // end class DummyDomain

} // end namespace pointer
} // end namespace core
} // end namespace ikos
