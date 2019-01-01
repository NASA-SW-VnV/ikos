/*******************************************************************************
 *
 * \file
 * \brief Implementation of a pointer abstract domain
 *
 * Authors:
 *   * Maxime Arthaud
 *   * Jorge A. Navas
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2019 United States Government as represented by the
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

#include <ikos/core/domain/machine_int/abstract_domain.hpp>
#include <ikos/core/domain/pointer/abstract_domain.hpp>
#include <ikos/core/domain/separate_domain.hpp>

namespace ikos {
namespace core {
namespace pointer {

/// \brief Pointer abstract domain
///
/// The pointer domain can express invariants on machine integer variables
/// (modelled directly in the underlying `MachineIntDomain`) and pointer
/// variables. For each pointer `p`, it keeps track of its address, its offset
/// and its nullity.
///
/// The offset of `p` is modelled directly by the numerical abstraction
/// `MachineIntDomain` with the special variable `offset_var(p)`.
///
/// The address of `p` is modelled by keeping track of all possible memory
/// locations (e.g., &'s and malloc's) to which `p` may point to, also called
/// the points-to set.
///
/// The nullity (null/non-null) of `p` is modelled by the underlying nullity
/// abstract domain `NullityDomain`.
///
/// Notes:
///
/// - Unlike traditional pointer analyses, this domain is flow-sensitive.
///
/// - The points-to set can be empty. This is useful since it can tell us
///   whether a variable is definitely null.
template < typename VariableRef,
           typename MemoryLocationRef,
           typename MachineIntDomain,
           typename NullityDomain >
class PointerDomain final
    : public pointer::AbstractDomain< VariableRef,
                                      MemoryLocationRef,
                                      MachineIntDomain,
                                      NullityDomain,
                                      PointerDomain< VariableRef,
                                                     MemoryLocationRef,
                                                     MachineIntDomain,
                                                     NullityDomain > > {
public:
  using LinearExpressionT = LinearExpression< MachineInt, VariableRef >;
  using PointsToSetT = PointsToSet< MemoryLocationRef >;
  using PointerAbsValueT = PointerAbsValue< MemoryLocationRef >;
  using PointerSetT = PointerSet< MemoryLocationRef >;

private:
  using PointsToMap = SeparateDomain< VariableRef, PointsToSetT >;
  using IntVariableTrait = machine_int::VariableTraits< VariableRef >;

private:
  /// \brief Map pointer variables to set of addresses
  PointsToMap _points_to_map;

  /// \brief Underlying nullity abstract domains for pointers
  NullityDomain _nullity;

  /// \brief Underlying machine integer abstract domains for scalars and offsets
  MachineIntDomain _inv;

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top abstract value
  explicit PointerDomain(TopTag)
      : _points_to_map(PointsToMap::top()),
        _nullity(NullityDomain::top()),
        _inv(MachineIntDomain::top()) {}

  /// \brief Create the bottom abstract value
  explicit PointerDomain(BottomTag)
      : _points_to_map(PointsToMap::bottom()),
        _nullity(NullityDomain::bottom()),
        _inv(MachineIntDomain::bottom()) {}

public:
  /// \brief Create the top abstract value
  PointerDomain() : PointerDomain(TopTag{}) {}

  /// \brief Create an abstract value with the given underlying domains
  ///
  /// \param inv The machine integer abstract value
  /// \param nullity The nullity abstract value
  explicit PointerDomain(MachineIntDomain inv, NullityDomain nullity)
      : _points_to_map(PointsToMap::top()),
        _nullity(std::move(nullity)),
        _inv(std::move(inv)) {
    this->normalize();
  }

  /// \brief Copy constructor
  PointerDomain(const PointerDomain&) = default;

  /// \brief Move constructor
  PointerDomain(PointerDomain&&) = default;

  /// \brief Copy assignment operator
  PointerDomain& operator=(const PointerDomain&) = default;

  /// \brief Move assignment operator
  PointerDomain& operator=(PointerDomain&&) = default;

  /// \brief Destructor
  ~PointerDomain() override = default;

  /// \brief Create the top abstract value
  static PointerDomain top() { return PointerDomain(TopTag{}); }

  /// \brief Create the bottom abstract value
  static PointerDomain bottom() { return PointerDomain(BottomTag{}); }

  /*
   * Implement core::AbstractDomain
   */

private:
  /// \brief Return true if the abstract value is bottom
  ///
  /// Does not normalize.
  bool is_bottom_fast() const { return this->_points_to_map.is_bottom(); }

public:
  bool is_bottom() const override {
    this->normalize();
    return this->_points_to_map.is_bottom(); // Correct because of normalization
  }

  bool is_top() const override {
    return this->_points_to_map.is_top() && this->_nullity.is_top() &&
           this->_inv.is_top();
  }

  void set_to_bottom() override {
    this->_points_to_map.set_to_bottom();
    this->_nullity.set_to_bottom();
    this->_inv.set_to_bottom();
  }

  void set_to_top() override {
    this->_points_to_map.set_to_top();
    this->_nullity.set_to_top();
    this->_inv.set_to_top();
  }

  bool leq(const PointerDomain& other) const override {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_points_to_map.leq(other._points_to_map) &&
             this->_nullity.leq(other._nullity) && this->_inv.leq(other._inv);
    }
  }

  bool equals(const PointerDomain& other) const override {
    if (this->is_bottom()) {
      return other.is_bottom();
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_points_to_map.equals(other._points_to_map) &&
             this->_nullity.equals(other._nullity) &&
             this->_inv.equals(other._inv);
    }
  }

  void join_with(const PointerDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_points_to_map.join_with(other._points_to_map);
      this->_nullity.join_with(other._nullity);
      this->_inv.join_with(other._inv);
    }
  }

  void join_loop_with(const PointerDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_points_to_map.join_loop_with(other._points_to_map);
      this->_nullity.join_loop_with(other._nullity);
      this->_inv.join_loop_with(other._inv);
    }
  }

  void join_iter_with(const PointerDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_points_to_map.join_iter_with(other._points_to_map);
      this->_nullity.join_iter_with(other._nullity);
      this->_inv.join_iter_with(other._inv);
    }
  }

  void widen_with(const PointerDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_points_to_map.widen_with(other._points_to_map);
      this->_nullity.widen_with(other._nullity);
      this->_inv.widen_with(other._inv);
    }
  }

  void widen_threshold_with(const PointerDomain& other,
                            const MachineInt& threshold) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_points_to_map.widen_with(other._points_to_map);
      this->_nullity.widen_with(other._nullity);
      this->_inv.widen_threshold_with(other._inv, threshold);
    }
  }

  void meet_with(const PointerDomain& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_points_to_map.meet_with(other._points_to_map);
      this->_nullity.meet_with(other._nullity);
      this->_inv.meet_with(other._inv);
    }
  }

  void narrow_with(const PointerDomain& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_points_to_map.narrow_with(other._points_to_map);
      this->_nullity.narrow_with(other._nullity);
      this->_inv.narrow_with(other._inv);
    }
  }

  /*
   * Implement pointer::AbstractDomain
   */

  MachineIntDomain& integers() override { return this->_inv; }

  const MachineIntDomain& integers() const override { return this->_inv; }

  NullityDomain& nullity() override { return this->_nullity; }

  const NullityDomain& nullity() const override { return this->_nullity; }

  void assign_address(VariableRef p,
                      MemoryLocationRef addr,
                      Nullity nullity) override {
    if (this->is_bottom_fast()) {
      return;
    }

    // Update points-to map
    this->_points_to_map.set(p, PointsToSetT{addr});

    // Update nullity
    this->_nullity.set(p, nullity);

    // Update offset
    VariableRef offset = this->offset_var(p);
    this->_inv.assign(offset,
                      MachineInt::zero(IntVariableTrait::bit_width(offset),
                                       IntVariableTrait::sign(offset)));
  }

  void assign_null(VariableRef p) override {
    if (this->is_bottom_fast()) {
      return;
    }

    // Update points-to map
    this->_points_to_map.set(p, PointsToSetT::empty());

    // Update nullity
    this->_nullity.assign_null(p);

    // Update offset
    this->_inv.forget(this->offset_var(p));
  }

  void assign_undef(VariableRef p) override {
    if (this->is_bottom_fast()) {
      return;
    }

    // Update points-to map
    this->_points_to_map.set(p, PointsToSetT::empty());

    // Update nullity
    this->_nullity.forget(p);

    // Update offset
    this->_inv.forget(this->offset_var(p));
  }

  void assign(VariableRef p, VariableRef q) override {
    if (this->is_bottom_fast()) {
      return;
    }

    // Update points-to map
    this->_points_to_map.set(p, this->_points_to_map.get(q));

    // Update nullity
    this->_nullity.assign(p, q);

    // Update offset
    this->_inv.assign(this->offset_var(p), this->offset_var(q));
  }

  void assign(VariableRef p, VariableRef q, VariableRef o) override {
    if (this->is_bottom_fast()) {
      return;
    }

    // Update points-to map
    this->_points_to_map.set(p, this->_points_to_map.get(q));

    // Update nullity
    this->_nullity.assign(p, q);

    // Update offset
    this->_inv.apply(machine_int::BinaryOperator::Add,
                     this->offset_var(p),
                     this->offset_var(q),
                     o);
  }

  void assign(VariableRef p, VariableRef q, const MachineInt& o) override {
    if (this->is_bottom_fast()) {
      return;
    }

    // Update points-to map
    this->_points_to_map.set(p, this->_points_to_map.get(q));

    // Update nullity
    this->_nullity.assign(p, q);

    // Update offset
    this->_inv.apply(machine_int::BinaryOperator::Add,
                     this->offset_var(p),
                     this->offset_var(q),
                     o);
  }

  void assign(VariableRef p,
              VariableRef q,
              const LinearExpressionT& o) override {
    if (this->is_bottom_fast()) {
      return;
    }

    // Update points-to map
    this->_points_to_map.set(p, this->_points_to_map.get(q));

    // Update nullity
    this->_nullity.assign(p, q);

    // Update offset
    VariableRef offset_p = this->offset_var(p);
    VariableRef offset_q = this->offset_var(q);
    MachineInt one(1,
                   IntVariableTrait::bit_width(offset_p),
                   IntVariableTrait::sign(offset_p));
    LinearExpressionT offset(o);
    offset.add(one, offset_q);
    this->_inv.assign(offset_p, offset);
  }

  void assert_null(VariableRef p) override {
    if (this->is_bottom_fast()) {
      return;
    }

    // Update nullity
    this->_nullity.assert_null(p);

    // Update points-to map
    this->_points_to_map.refine(p, PointsToSetT::empty());
  }

  void assert_non_null(VariableRef p) override {
    if (this->is_bottom_fast()) {
      return;
    }

    // Update nullity
    this->_nullity.assert_non_null(p);

    // Update points-to map
    if (this->_points_to_map.get(p).is_empty()) {
      this->set_to_bottom();
    }
  }

  void add(Predicate pred, VariableRef p, VariableRef q) override {
    if (this->is_bottom_fast()) {
      return;
    }

    // Update nullity
    this->_nullity.add(pred, p, q);

    if (this->_nullity.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    // Update points-to map and offsets
    PointsToSetT addrs_p = this->_points_to_map.get(p);
    PointsToSetT addrs_q = this->_points_to_map.get(q);
    Nullity nullity_p = this->_nullity.get(p);
    Nullity nullity_q = this->_nullity.get(q);

    switch (pred) {
      case Predicate::EQ: {
        // p == q
        PointsToSetT addrs_pq = addrs_p.meet(addrs_q);

        if (addrs_pq.is_bottom() ||
            (addrs_pq.is_empty() && nullity_p.is_non_null())) {
          this->set_to_bottom();
          return;
        }

        // p and q's points-to sets
        this->_points_to_map.set(p, addrs_pq);
        this->_points_to_map.set(q, addrs_pq);

        // p and q's offsets
        this->_inv.add(machine_int::Predicate::EQ,
                       this->offset_var(p),
                       this->offset_var(q));
      } break;
      case Predicate::NE: {
        // p != q
        if (nullity_p.is_non_null() && nullity_q.is_non_null() &&
            addrs_p.singleton() && addrs_p == addrs_q) {
          // p and q's offsets
          this->_inv.add(machine_int::Predicate::NE,
                         this->offset_var(p),
                         this->offset_var(q));
        }
      } break;
      case Predicate::GT: {
        // p > q
        if (nullity_p.is_non_null() && nullity_q.is_non_null() &&
            addrs_p.singleton() && addrs_p == addrs_q) {
          // p and q's offsets
          this->_inv.add(machine_int::Predicate::GT,
                         this->offset_var(p),
                         this->offset_var(q));
        }
      } break;
      case Predicate::GE: {
        // p >= q
        if (nullity_p.is_non_null() && nullity_q.is_non_null() &&
            addrs_p.singleton() && addrs_p == addrs_q) {
          // p and q's offsets
          this->_inv.add(machine_int::Predicate::GE,
                         this->offset_var(p),
                         this->offset_var(q));
        }
      } break;
      case Predicate::LT: {
        // p < q
        if (nullity_p.is_non_null() && nullity_q.is_non_null() &&
            addrs_p.singleton() && addrs_p == addrs_q) {
          // p and q's offsets
          this->_inv.add(machine_int::Predicate::LT,
                         this->offset_var(p),
                         this->offset_var(q));
        }
      } break;
      case Predicate::LE: {
        // p <= q
        if (nullity_p.is_non_null() && nullity_q.is_non_null() &&
            addrs_p.singleton() && addrs_p == addrs_q) {
          // p and q's offsets
          this->_inv.add(machine_int::Predicate::LE,
                         this->offset_var(p),
                         this->offset_var(q));
        }
      } break;
    }
  }

  void refine(VariableRef p, const PointsToSetT& addrs) override {
    // Update points-to map
    this->_points_to_map.refine(p, addrs);
  }

  void refine(VariableRef p,
              const PointsToSetT& addrs,
              const machine_int::Interval& offset) override {
    // Update points-to map
    this->_points_to_map.refine(p, addrs);

    // Update offset
    this->_inv.refine(this->offset_var(p), offset);
  }

  void refine(VariableRef p, const PointerAbsValueT& value) override {
    if (value.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    // Update points-to map
    this->_points_to_map.refine(p, value.points_to());

    // Update offset
    if (value.points_to().is_empty()) {
      // value.offset() is bottom because the pointer is null or uninitialized
      this->_inv.forget(this->offset_var(p));
    } else {
      this->_inv.refine(this->offset_var(p), value.offset());
    }

    // Update nullity
    this->_nullity.refine(p, value.nullity());
  }

  void refine(VariableRef p, const PointerSetT& value) override {
    if (value.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    // Update points-to map
    this->_points_to_map.refine(p, value.points_to());

    // Update offset
    if (value.points_to().is_empty()) {
      // value.offsets() is bottom because pointers are null or uninitialized
      this->_inv.forget(this->offset_var(p));
    } else {
      this->_inv.refine(this->offset_var(p), value.offsets());
    }
  }

  PointsToSetT points_to(VariableRef p) const override {
    return this->_points_to_map.get(p);
  }

  PointerAbsValueT get(VariableRef p) const override {
    return PointerAbsValueT(this->_points_to_map.get(p),
                            this->_inv.to_interval(this->offset_var(p)),
                            this->_nullity.get(p),
                            Uninitialized::top());
  }

  void forget(VariableRef p) override {
    this->_points_to_map.forget(p);
    this->_nullity.forget(p);
    this->_inv.forget(this->offset_var(p));
  }

  void normalize() const override {
    // is_bottom() will normalize
    if (this->_points_to_map.is_bottom() || this->_nullity.is_bottom() ||
        this->_inv.is_bottom()) {
      const_cast< PointerDomain* >(this)->set_to_bottom();
    }
  }

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else {
      o << "(";
      this->_points_to_map.dump(o);
      o << ", ";
      this->_nullity.dump(o);
      o << ", ";
      this->_inv.dump(o);
      o << ")";
    }
  }

  static std::string name() {
    return "pointer domain using " + MachineIntDomain::name() + " and " +
           NullityDomain::name();
  }

}; // end class PointerDomain

} // end namespace pointer
} // end namespace core
} // end namespace ikos
