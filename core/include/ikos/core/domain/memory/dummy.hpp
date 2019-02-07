/*******************************************************************************
 *
 * \file
 * \brief A dummy memory abstract domain
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

#include <ikos/core/domain/memory/abstract_domain.hpp>

namespace ikos {
namespace core {
namespace memory {

/// \brief Dummy memory abstract domain
///
/// This class holds:
///   * A machine integer abstract domain (embedded in the pointer domain)
///   * A nullity abstract domain (embedded in the pointer domain)
///   * A pointer abstract domain
///   * An uninitialized abstract domain
///   * A lifetime abstract domain
///
/// It safely ignores all memory operations.
template < typename VariableRef,
           typename MemoryLocationRef,
           typename VariableFactory,
           typename MachineIntDomain,
           typename NullityDomain,
           typename PointerDomain,
           typename UninitializedDomain,
           typename LifetimeDomain >
class DummyDomain final
    : public memory::AbstractDomain< VariableRef,
                                     MemoryLocationRef,
                                     VariableFactory,
                                     MachineIntDomain,
                                     NullityDomain,
                                     PointerDomain,
                                     UninitializedDomain,
                                     LifetimeDomain,
                                     DummyDomain< VariableRef,
                                                  MemoryLocationRef,
                                                  VariableFactory,
                                                  MachineIntDomain,
                                                  NullityDomain,
                                                  PointerDomain,
                                                  UninitializedDomain,
                                                  LifetimeDomain > > {
private:
  /// \brief Trait for typed variables,
  /// see ikos/core/semantic/memory/variable.hpp
  using MemVariableTrait = memory::VariableTraits< VariableRef >;

  using LiteralT = Literal< VariableRef, MemoryLocationRef >;

  using Interval = machine_int::Interval;

private:
  PointerDomain _pointer;
  UninitializedDomain _uninitialized;
  LifetimeDomain _lifetime;

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top abstract value
  explicit DummyDomain(TopTag)
      : _pointer(PointerDomain::top()),
        _uninitialized(UninitializedDomain::top()),
        _lifetime(LifetimeDomain::top()) {}

  /// \brief Create the bottom abstract value
  explicit DummyDomain(BottomTag)
      : _pointer(PointerDomain::bottom()),
        _uninitialized(UninitializedDomain::bottom()),
        _lifetime(LifetimeDomain::bottom()) {}

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

  /*
   * Implement core::AbstractDomain
   */

  bool is_bottom() const override {
    this->normalize();
    return this->_lifetime.is_bottom(); // Correct because of normalization
  }

  bool is_top() const override {
    return this->_pointer.is_top() && this->_uninitialized.is_top() &&
           this->_lifetime.is_top();
  }

  void set_to_bottom() override {
    this->_pointer.set_to_bottom();
    this->_uninitialized.set_to_bottom();
    this->_lifetime.set_to_bottom();
  }

  void set_to_top() override {
    this->_pointer.set_to_top();
    this->_uninitialized.set_to_top();
    this->_lifetime.set_to_top();
  }

  bool leq(const DummyDomain& other) const override {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_pointer.leq(other._pointer) &&
             this->_uninitialized.leq(other._uninitialized) &&
             this->_lifetime.leq(other._lifetime);
    }
  }

  bool equals(const DummyDomain& other) const override {
    if (this->is_bottom()) {
      return other.is_bottom();
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_pointer.equals(other._pointer) &&
             this->_uninitialized.equals(other._uninitialized) &&
             this->_lifetime.equals(other._lifetime);
    }
  }

  void join_with(const DummyDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_pointer.join_with(other._pointer);
      this->_uninitialized.join_with(other._uninitialized);
      this->_lifetime.join_with(other._lifetime);
    }
  }

  void join_loop_with(const DummyDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_pointer.join_loop_with(other._pointer);
      this->_uninitialized.join_loop_with(other._uninitialized);
      this->_lifetime.join_loop_with(other._lifetime);
    }
  }

  void join_iter_with(const DummyDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_pointer.join_iter_with(other._pointer);
      this->_uninitialized.join_iter_with(other._uninitialized);
      this->_lifetime.join_iter_with(other._lifetime);
    }
  }

  void widen_with(const DummyDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_pointer.widen_with(other._pointer);
      this->_uninitialized.widen_with(other._uninitialized);
      this->_lifetime.widen_with(other._lifetime);
    }
  }

  void widen_threshold_with(const DummyDomain& other,
                            const MachineInt& threshold) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_pointer.widen_threshold_with(other._pointer, threshold);
      this->_uninitialized.widen_with(other._uninitialized);
      this->_lifetime.widen_with(other._lifetime);
    }
  }

  void meet_with(const DummyDomain& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_pointer.meet_with(other._pointer);
      this->_uninitialized.meet_with(other._uninitialized);
      this->_lifetime.meet_with(other._lifetime);
    }
  }

  void narrow_with(const DummyDomain& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_pointer.narrow_with(other._pointer);
      this->_uninitialized.narrow_with(other._uninitialized);
      this->_lifetime.narrow_with(other._lifetime);
    }
  }

  /*
   * Implement memory::AbstractDomain
   */

  MachineIntDomain& integers() override { return this->_pointer.integers(); }

  const MachineIntDomain& integers() const override {
    return this->_pointer.integers();
  }

  NullityDomain& nullity() override { return this->_pointer.nullity(); }

  const NullityDomain& nullity() const override {
    return this->_pointer.nullity();
  }

  PointerDomain& pointers() override { return this->_pointer; }

  const PointerDomain& pointers() const override { return this->_pointer; }

  UninitializedDomain& uninitialized() override { return this->_uninitialized; }

  const UninitializedDomain& uninitialized() const override {
    return this->_uninitialized;
  }

  LifetimeDomain& lifetime() override { return this->_lifetime; }

  const LifetimeDomain& lifetime() const override { return this->_lifetime; }

  void mem_write(VariableFactory&,
                 VariableRef,
                 const LiteralT&,
                 const MachineInt&) override {}

  void mem_read(VariableFactory&,
                const LiteralT&,
                VariableRef,
                const MachineInt&) override {}

  void mem_copy(VariableFactory&,
                VariableRef,
                VariableRef,
                const LiteralT&) override {}

  void mem_set(VariableFactory&,
               VariableRef,
               const LiteralT&,
               const LiteralT&) override {}

  void forget_surface(VariableRef x) override {
    if (MemVariableTrait::is_int(x)) {
      this->integers().forget(x);
      this->uninitialized().forget(x);
    } else if (MemVariableTrait::is_float(x)) {
      this->uninitialized().forget(x);
    } else if (MemVariableTrait::is_pointer(x)) {
      this->pointers().forget(x);
      this->uninitialized().forget(x);
    } else {
      ikos_unreachable("unexpected type");
    }
  }

  void forget_mem() override {}

  void forget_mem(MemoryLocationRef) override {}

  void forget_mem(MemoryLocationRef, VariableRef, const MachineInt&) override {}

  void forget_mem(MemoryLocationRef, const Interval&) override {}

  void forget_reachable_mem(VariableRef) override {}

  void forget_reachable_mem(VariableRef, const MachineInt&) override {}

  void abstract_reachable_mem(VariableRef) override {}

  void abstract_reachable_mem(VariableRef, const MachineInt&) override {}

  void zero_reachable_mem(VariableRef) override {}

  void uninitialize_reachable_mem(VariableRef) override {}

  void normalize() const override {
    // is_bottom() will normalize
    if (this->_pointer.is_bottom()) {
      const_cast< DummyDomain* >(this)->_uninitialized.set_to_bottom();
      const_cast< DummyDomain* >(this)->_lifetime.set_to_bottom();
    } else if (this->_uninitialized.is_bottom()) {
      const_cast< DummyDomain* >(this)->_pointer.set_to_bottom();
      const_cast< DummyDomain* >(this)->_lifetime.set_to_bottom();
    } else if (this->_lifetime.is_bottom()) {
      const_cast< DummyDomain* >(this)->_pointer.set_to_bottom();
      const_cast< DummyDomain* >(this)->_uninitialized.set_to_bottom();
    }
  }

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else if (this->is_top()) {
      o << "T";
    } else {
      o << "(";
      this->_pointer.dump(o);
      o << ", ";
      this->_uninitialized.dump(o);
      o << ", ";
      this->_lifetime.dump(o);
      o << ")";
    }
  }

  static std::string name() {
    return "dummy memory domain with " + PointerDomain::name() + ", " +
           UninitializedDomain::name() + " and " + LifetimeDomain::name();
  }

}; // end class DummyDomain

} // end namespace memory
} // end namespace core
} // end namespace ikos
