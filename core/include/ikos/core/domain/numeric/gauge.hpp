/*******************************************************************************
 *
 * \file
 * \brief Implementation of the Gauge Domain
 *
 * Based on Arnaud Venet's paper: The Gauge Domain: Scalable Analysis of
 * Linear Inequality Invariants, in CAV, 129-154, 2012.
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017-2019 United States Government as represented by the
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

#include <ikos/core/adt/patricia_tree/map.hpp>
#include <ikos/core/adt/patricia_tree/set.hpp>
#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/domain/numeric/abstract_domain.hpp>
#include <ikos/core/domain/numeric/constant.hpp>
#include <ikos/core/domain/numeric/interval.hpp>
#include <ikos/core/linear_constraint.hpp>
#include <ikos/core/linear_expression.hpp>
#include <ikos/core/value/numeric/gauge.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief Gauge semilattice
template < typename Number, typename VariableRef >
class GaugeSemiLattice final
    : public core::AbstractDomain< GaugeSemiLattice< Number, VariableRef > > {
public:
  using GaugeBoundT = GaugeBound< Number, VariableRef >;
  using GaugeT = Gauge< Number, VariableRef >;
  using BoundT = Bound< Number >;
  using ConstantT = Constant< Number >;
  using IntervalT = Interval< Number >;
  using VariableExpressionT = VariableExpression< Number, VariableRef >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;
  using LinearConstraintSystemT = LinearConstraintSystem< Number, VariableRef >;

private:
  using PatriciaTreeMapT = PatriciaTreeMap< VariableRef, GaugeT >;

private:
  PatriciaTreeMapT _tree;
  bool _is_bottom;

  /* Invariants:
   * _is_bottom => _tree.empty()
   * for v in _tree: _tree.at(v) != GaugeT::top()
   * for v in _tree: _tree.at(v) != GaugeT::bottom()
   */

private:
  struct TopTag {};
  struct BottomTag {};
  struct BottomFound {};

  /// \brief Create the top abstract value
  explicit GaugeSemiLattice(TopTag) : _is_bottom(false) {}

  /// \brief Create the bottom abstract value
  explicit GaugeSemiLattice(BottomTag) : _is_bottom(true) {}

public:
  /// \brief Create the top abstract value
  static GaugeSemiLattice top() { return GaugeSemiLattice(TopTag{}); }

  /// \brief Create the bottom abstract value
  static GaugeSemiLattice bottom() { return GaugeSemiLattice(BottomTag{}); }

  /// \brief Copy constructor
  GaugeSemiLattice(const GaugeSemiLattice&) noexcept = default;

  /// \brief Move constructor
  GaugeSemiLattice(GaugeSemiLattice&&) noexcept = default;

  /// \brief Copy assignment operator
  GaugeSemiLattice& operator=(const GaugeSemiLattice&) noexcept = default;

  /// \brief Move assignment operator
  GaugeSemiLattice& operator=(GaugeSemiLattice&&) noexcept = default;

  /// \brief Destructor
  ~GaugeSemiLattice() override = default;

  void normalize() override {}

  bool is_bottom() const override { return this->_is_bottom; }

  bool is_top() const override {
    return !this->_is_bottom && this->_tree.empty();
  }

  void set_to_bottom() override {
    this->_is_bottom = true;
    this->_tree.clear();
  }

  void set_to_top() override {
    this->_is_bottom = false;
    this->_tree.clear();
  }

  bool leq(const GaugeSemiLattice& other) const override {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_tree.leq(other._tree, [](const GaugeT& x, const GaugeT& y) {
        return x.leq(y);
      });
    }
  }

  bool equals(const GaugeSemiLattice& other) const override {
    if (this->is_bottom()) {
      return other.is_bottom();
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_tree.equals(other._tree,
                                [](const GaugeT& x, const GaugeT& y) {
                                  return x.equals(y);
                                });
    }
  }

  void join_with(const GaugeSemiLattice& other) override {
    if (other.is_bottom()) {
      return;
    } else if (this->is_bottom()) {
      this->operator=(other);
    } else {
      this->_tree.intersect_with(other._tree,
                                 [](const GaugeT& x, const GaugeT& y) {
                                   GaugeT z = x.join(y);
                                   if (z.is_top()) {
                                     return boost::optional< GaugeT >(
                                         boost::none);
                                   }
                                   return boost::optional< GaugeT >(z);
                                 });
    }
  }

  void meet_with(const GaugeSemiLattice& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      try {
        this->_tree.join_with(other._tree,
                              [](const GaugeT& x, const GaugeT& y) {
                                GaugeT z = x.meet(y);
                                if (z.is_bottom()) {
                                  throw BottomFound();
                                }
                                return boost::optional< GaugeT >(z);
                              });
      } catch (BottomFound&) {
        this->set_to_bottom();
      }
    }
  }

  // \brief Widening interval-like
  void widen_interval_with(const GaugeSemiLattice& other) {
    if (other.is_bottom()) {
      return;
    } else if (this->is_bottom()) {
      this->operator=(other);
    } else {
      this->_tree.intersect_with(other._tree,
                                 [](const GaugeT& x, const GaugeT& y) {
                                   GaugeT z = x.widening_interval(y);
                                   if (z.is_top()) {
                                     return boost::optional< GaugeT >(
                                         boost::none);
                                   }
                                   return boost::optional< GaugeT >(z);
                                 });
    }
  }

  // \brief Widening interval-like with a threshold
  void widen_interval_threshold_with(const GaugeSemiLattice& other,
                                     const Number& threshold) {
    if (other.is_bottom()) {
      return;
    } else if (this->is_bottom()) {
      this->operator=(other);
    } else {
      this->_tree.intersect_with(other._tree,
                                 [threshold](const GaugeT& x, const GaugeT& y) {
                                   GaugeT z =
                                       x.widening_interval_threshold(y,
                                                                     threshold);
                                   if (z.is_top()) {
                                     return boost::optional< GaugeT >(
                                         boost::none);
                                   }
                                   return boost::optional< GaugeT >(z);
                                 });
    }
  }

  /// \brief Widening with interpolation
  void widen_interpol_with(const GaugeSemiLattice& other,
                           VariableRef k,
                           const Number& u,
                           const ConstantT& v) {
    if (other.is_bottom()) {
      return;
    } else if (this->is_bottom()) {
      this->operator=(other);
    } else {
      this->_tree.intersect_with(other._tree,
                                 [k, u, v](const GaugeT& x, const GaugeT& y) {
                                   GaugeT z = x.widening_interpol(y, k, u, v);
                                   if (z.is_top()) {
                                     return boost::optional< GaugeT >(
                                         boost::none);
                                   }
                                   return boost::optional< GaugeT >(z);
                                 });
    }
  }

  void widen_with(const GaugeSemiLattice& other) override {
    this->widen_interval_with(other);
  }

  void narrow_with(const GaugeSemiLattice& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      try {
        this->_tree.join_with(other._tree,
                              [](const GaugeT& x, const GaugeT& y) {
                                GaugeT z = x.narrowing(y);
                                if (z.is_bottom()) {
                                  throw BottomFound();
                                }
                                return boost::optional< GaugeT >(z);
                              });
      } catch (BottomFound&) {
        this->set_to_bottom();
      }
    }
  }

  // \brief Narrowing interval-like with a threshold
  void narrow_interval_threshold_with(const GaugeSemiLattice& other,
                                      const Number& threshold) {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      try {
        this->_tree.join_with(other._tree,
                              [threshold](const GaugeT& x, const GaugeT& y) {
                                GaugeT z =
                                    x.narrowing_interval_threshold(y,
                                                                   threshold);
                                if (z.is_bottom()) {
                                  throw BottomFound();
                                }
                                return boost::optional< GaugeT >(z);
                              });
      } catch (BottomFound&) {
        this->set_to_bottom();
      }
    }
  }

  /// \brief Increment counter `v` by `k`
  void counter_incr(VariableRef v, const Number& k) {
    if (this->is_bottom()) {
      return;
    }

    this->_tree.transform([v, k](VariableRef, const GaugeT& x) {
      GaugeT y = x.counter_incr(v, k);
      if (y.is_top()) {
        return boost::optional< GaugeT >(boost::none);
      } else {
        return boost::optional< GaugeT >(y);
      }
    });
  }

  /// \brief Forget counter `v`
  ///
  /// \param value Interval for `v`
  void counter_forget(VariableRef v, IntervalT value = IntervalT::top()) {
    if (this->is_bottom()) {
      return;
    }

    value.meet_with(IntervalT(BoundT(0), BoundT::plus_infinity()));
    ikos_assert(!value.is_bottom());
    ikos_assert(value.lb().is_finite());

    Number l = *value.lb().number();
    const BoundT& u = value.ub();
    this->_tree.transform([v, l, u](VariableRef, const GaugeT& x) {
      GaugeT y = x.coalesce(v, l, u);
      if (y.is_top()) {
        return boost::optional< GaugeT >(boost::none);
      } else {
        return boost::optional< GaugeT >(y);
      }
    });
  }

  /// \brief Forget variable `v`
  void forget(VariableRef v) {
    if (this->is_bottom()) {
      return;
    }
    this->_tree.erase(v);
  }

  /// \brief Set the gauge for the given variable
  void set(VariableRef v, const GaugeT& g) {
    if (this->is_bottom()) {
      return;
    } else if (g.is_bottom()) {
      this->set_to_bottom();
    } else if (g.is_top()) {
      this->_tree.erase(v);
    } else {
      this->_tree.insert_or_assign(v, g);
    }
  }

  /// \brief Refine the gauge for the given variable
  void refine(VariableRef v, const GaugeT& g) {
    if (this->is_bottom()) {
      return;
    } else if (g.is_bottom()) {
      this->set_to_bottom();
    } else if (g.is_top()) {
      return;
    } else {
      try {
        this->_tree.update_or_insert(
            [](const GaugeT& x, const GaugeT& y) {
              GaugeT z = x.meet(y);
              if (z.is_bottom()) {
                throw BottomFound();
              }
              return boost::optional< GaugeT >(z);
            },
            v,
            g);
      } catch (BottomFound&) {
        this->set_to_bottom();
      }
    }
  }

  /// \brief Get the gauge for the given variable
  GaugeT get(VariableRef v) const {
    if (this->is_bottom()) {
      return GaugeT::bottom();
    } else {
      boost::optional< const GaugeT& > g = this->_tree.at(v);
      if (g) {
        return *g;
      } else {
        return GaugeT::top();
      }
    }
  }

  LinearConstraintSystemT to_linear_constraint_system() const {
    if (this->is_bottom()) {
      return LinearConstraintSystemT(LinearConstraintT::contradiction());
    }

    LinearConstraintSystemT csts;
    for (auto it = this->_tree.begin(); it != this->_tree.end(); ++it) {
      const GaugeBoundT& lb = it->second.lb();
      const GaugeBoundT& ub = it->second.ub();
      if (lb.is_finite()) {
        csts.add(lb.lin_expr() <= VariableExpressionT(it->first));
      }
      if (ub.is_finite()) {
        csts.add(VariableExpressionT(it->first) <= ub.lin_expr());
      }
    }

    return csts;
  }

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else {
      this->_tree.dump(o);
    }
  }

  static std::string name() { return "gauge semilattice"; }

}; // end class GaugeSemiLattice

/// \brief Gauge abstract domain
template < typename Number, typename VariableRef >
class GaugeDomain final
    : public numeric::AbstractDomain< Number,
                                      VariableRef,
                                      GaugeDomain< Number, VariableRef > > {
public:
  using GaugeBoundT = GaugeBound< Number, VariableRef >;
  using GaugeT = Gauge< Number, VariableRef >;
  using BoundT = Bound< Number >;
  using ConstantT = Constant< Number >;
  using IntervalT = Interval< Number >;
  using CongruenceT = Congruence< Number >;
  using IntervalCongruenceT = IntervalCongruence< Number >;
  using VariableExpressionT = VariableExpression< Number, VariableRef >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;
  using LinearConstraintSystemT = LinearConstraintSystem< Number, VariableRef >;

public:
  using SectionDomainT = ConstantDomain< Number, VariableRef >;
  using GaugeSemiLatticeT = GaugeSemiLattice< Number, VariableRef >;
  using CounterSetT = PatriciaTreeSet< VariableRef >;
  using IntervalDomainT = IntervalDomain< Number, VariableRef >;

private:
  SectionDomainT _sections;
  GaugeSemiLatticeT _gauges;
  CounterSetT _counters;
  IntervalDomainT _intervals;

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top abstract value
  explicit GaugeDomain(TopTag)
      : _sections(SectionDomainT::top()),
        _gauges(GaugeSemiLatticeT::top()),
        _counters(),
        _intervals(IntervalDomainT::top()) {}

  /// \brief Create the bottom abstract value
  explicit GaugeDomain(BottomTag)
      : _sections(SectionDomainT::bottom()),
        _gauges(GaugeSemiLatticeT::bottom()),
        _counters(),
        _intervals(IntervalDomainT::bottom()) {}

public:
  /// \brief Create the top abstract value
  static GaugeDomain top() { return GaugeDomain(TopTag{}); }

  /// \brief Create the bottom abstract value
  static GaugeDomain bottom() { return GaugeDomain(BottomTag{}); }

  /// \brief Copy constructor
  GaugeDomain(const GaugeDomain&) noexcept = default;

  /// \brief Move constructor
  GaugeDomain(GaugeDomain&&) noexcept = default;

  /// \brief Copy assignment operator
  GaugeDomain& operator=(const GaugeDomain&) noexcept = default;

  /// \brief Move assignment operator
  GaugeDomain& operator=(GaugeDomain&&) noexcept = default;

  /// \brief Destructor
  ~GaugeDomain() override = default;

  void normalize() override {
    if (this->_sections.is_bottom() || this->_gauges.is_bottom() ||
        this->_intervals.is_bottom()) {
      this->set_to_bottom();
    }
  }

  bool is_bottom() const override {
    return this->_sections.is_bottom() || this->_gauges.is_bottom() ||
           this->_intervals.is_bottom();
  }

  bool is_top() const override {
    return this->_sections.is_top() && this->_gauges.is_top() &&
           this->_counters.empty() && this->_intervals.is_top();
  }

  void set_to_bottom() override {
    this->_sections.set_to_bottom();
    this->_gauges.set_to_bottom();
    this->_counters.clear();
    this->_intervals.set_to_bottom();
  }

  void set_to_top() override {
    this->_sections.set_to_top();
    this->_gauges.set_to_top();
    this->_counters.clear();
    this->_intervals.set_to_top();
  }

  bool leq(const GaugeDomain& other) const override {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_sections.leq(other._sections) &&
             this->_gauges.leq(other._gauges) &&
             other._counters.is_subset_of(this->_counters) &&
             this->_intervals.leq(other._intervals);
    }
  }

  bool equals(const GaugeDomain& other) const override {
    if (this->is_bottom()) {
      return other.is_bottom();
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_sections.equals(other._sections) &&
             this->_gauges.equals(other._gauges) &&
             this->_counters.equals(other._counters) &&
             this->_intervals.equals(other._intervals);
    }
  }

private:
  /// \brief Make sure `l` and `r` have the same set of counters
  static void uniformize_counters(GaugeDomain& l, GaugeDomain& r) {
    for (VariableRef v : l._counters.difference(r._counters)) {
      l.counter_unmark(v);
    }
    for (VariableRef v : r._counters.difference(l._counters)) {
      r.counter_unmark(v);
    }
    ikos_assert(l._counters == r._counters);
  }

public:
  void join_with(const GaugeDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else if (this->_counters.equals(other._counters)) {
      this->_sections.join_with(other._sections);
      this->_gauges.join_with(other._gauges);
      this->_counters.intersect_with(other._counters);
      this->_intervals.join_with(other._intervals);
    } else {
      GaugeDomain other_copy = other;
      uniformize_counters(*this, other_copy);
      this->join_with(other_copy);
    }
  }

  void join_loop_with(const GaugeDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else if (this->_counters.equals(other._counters)) {
      SectionDomainT sections = this->_sections.join(other._sections);

      if (this->_sections.equals(sections)) {
        this->_gauges.join_with(other._gauges);
        this->_counters.intersect_with(other._counters);
        this->_intervals.join_with(other._intervals);
      } else {
        GaugeSemiLatticeT gauges = this->_gauges;

        for (auto it = this->_sections.begin(), et = this->_sections.end();
             it != et;
             ++it) {
          VariableRef k = it->first;
          const ConstantT& u = it->second;
          ConstantT v = other._sections.to_constant(k);
          if (u != v) {
            ikos_assert(u.is_number());
            gauges.widen_interpol_with(other._gauges, k, *u.number(), v);
          }
        }

        this->_sections = sections;
        this->_gauges = gauges;
        this->_counters.intersect_with(other._counters);
        this->_intervals.join_with(other._intervals);
      }
    } else {
      GaugeDomain other_copy = other;
      uniformize_counters(*this, other_copy);
      this->join_loop_with(other_copy);
    }
  }

  void join_iter_with(const GaugeDomain& other) override {
    this->join_loop_with(other);
  }

  void widen_with(const GaugeDomain& other) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else if (this->_counters.equals(other._counters)) {
      SectionDomainT sections = this->_sections.widening(other._sections);

      if (this->_sections.equals(sections)) {
        this->_gauges.widen_interval_with(other._gauges);
        this->_counters.intersect_with(other._counters);
        this->_intervals.widen_with(other._intervals);
      } else {
        GaugeSemiLatticeT gauges = this->_gauges;

        for (auto it = this->_sections.begin(), et = this->_sections.end();
             it != et;
             ++it) {
          VariableRef k = it->first;
          const ConstantT& u = it->second;
          ConstantT v = other._sections.to_constant(k);
          if (u != v) {
            ikos_assert(u.is_number());
            gauges.widen_interpol_with(other._gauges, k, *u.number(), v);
          }
        }

        this->_sections = sections;
        this->_gauges = gauges;
        this->_counters.intersect_with(other._counters);
        this->_intervals.widen_with(other._intervals);
      }
    } else {
      GaugeDomain other_copy = other;
      uniformize_counters(*this, other_copy);
      this->widen_with(other_copy);
    }
  }

  void widen_threshold_with(const GaugeDomain& other,
                            const Number& threshold) override {
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else if (this->_counters.equals(other._counters)) {
      SectionDomainT sections = this->_sections.widening(other._sections);

      if (this->_sections.equals(sections)) {
        this->_gauges.widen_interval_threshold_with(other._gauges, threshold);
        this->_counters.intersect_with(other._counters);
        this->_intervals.widen_threshold_with(other._intervals, threshold);
      } else {
        GaugeSemiLatticeT gauges = this->_gauges;

        for (auto it = this->_sections.begin(), et = this->_sections.end();
             it != et;
             ++it) {
          VariableRef k = it->first;
          const ConstantT& u = it->second;
          ConstantT v = other._sections.to_constant(k);
          if (u != v) {
            ikos_assert(u.is_number());
            gauges.widen_interpol_with(other._gauges, k, *u.number(), v);
          }
        }

        this->_sections = sections;
        this->_gauges = gauges;
        this->_counters.intersect_with(other._counters);
        this->_intervals.widen_threshold_with(other._intervals, threshold);
      }
    } else {
      GaugeDomain other_copy = other;
      uniformize_counters(*this, other_copy);
      this->widen_threshold_with(other_copy, threshold);
    }
  }

  void meet_with(const GaugeDomain& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else if (this->_counters.equals(other._counters)) {
      this->_sections.meet_with(other._sections);
      this->_gauges.meet_with(other._gauges);
      this->_counters.join_with(other._counters);
      this->_intervals.meet_with(other._intervals);
    } else {
      GaugeDomain other_copy = other;
      uniformize_counters(*this, other_copy);
      this->meet_with(other_copy);
    }
  }

  void narrow_with(const GaugeDomain& other) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else if (this->_counters.equals(other._counters)) {
      this->_sections.narrow_with(other._sections);
      this->_gauges.narrow_with(other._gauges);
      this->_counters.join_with(other._counters);
      this->_intervals.narrow_with(other._intervals);
    } else {
      GaugeDomain other_copy = other;
      uniformize_counters(*this, other_copy);
      this->narrow_with(other_copy);
    }
  }

  void narrow_threshold_with(const GaugeDomain& other,
                             const Number& threshold) override {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else if (this->_counters.equals(other._counters)) {
      this->_sections.narrow_threshold_with(other._sections, threshold);
      this->_gauges.narrow_interval_threshold_with(other._gauges, threshold);
      this->_counters.join_with(other._counters);
      this->_intervals.narrow_threshold_with(other._intervals, threshold);
    } else {
      GaugeDomain other_copy = other;
      uniformize_counters(*this, other_copy);
      this->narrow_threshold_with(other_copy, threshold);
    }
  }

private:
  /// \brief Return true if the given variable is a counter
  bool is_counter(VariableRef x) const { return this->_counters.contains(x); }

public:
  void assign(VariableRef x, int n) override {
    ikos_assert(!this->is_counter(x));
    this->set(x, GaugeT(n));
  }

  void assign(VariableRef x, const Number& n) override {
    ikos_assert(!this->is_counter(x));
    this->set(x, GaugeT(n));
  }

  void assign(VariableRef x, VariableRef y) override {
    ikos_assert(!this->is_counter(x));
    this->set(x, this->to_gauge(y));
  }

  void assign(VariableRef x, const LinearExpressionT& e) override {
    ikos_assert(!this->is_counter(x));
    this->set(x, this->to_gauge(e));
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             VariableRef z) override {
    if (this->is_bottom()) {
      return;
    }

    ikos_assert(!this->is_counter(x));

    switch (op) {
      case BinaryOperator::Add: {
        this->set(x, this->to_gauge(y) + this->to_gauge(z));
      } break;
      case BinaryOperator::Sub: {
        this->set(x, this->to_gauge(y) - this->to_gauge(z));
      } break;
      case BinaryOperator::Mul: {
        GaugeT yg = this->to_gauge(y);
        GaugeT zg = this->to_gauge(z);
        boost::optional< IntervalT > yi = yg.interval();
        boost::optional< IntervalT > zi = zg.interval();
        if (yi) {
          this->set(x, zg * (*yi));
        } else if (zi) {
          this->set(x, yg * (*zi));
        } else {
          // use interval representation
          this->set(x, this->to_interval(y) * this->to_interval(z));
        }
      } break;
      case BinaryOperator::Div:
      case BinaryOperator::Rem:
      case BinaryOperator::Mod:
      case BinaryOperator::Shl:
      case BinaryOperator::Shr:
      case BinaryOperator::And:
      case BinaryOperator::Or:
      case BinaryOperator::Xor: {
        IntervalT yi = this->to_interval(y);
        IntervalT zi = this->to_interval(z);

        if (zi.singleton()) {
          this->apply(op, x, y, *zi.singleton());
        } else if (yi.singleton()) {
          this->apply(op, x, *yi.singleton(), z);
        } else {
          // use interval representation
          this->set(x, apply_bin_operator(op, yi, zi));
        }
      } break;
      default: {
        ikos_unreachable("unreachable");
      }
    }
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             const Number& z) override {
    if (this->is_bottom()) {
      return;
    }

    ikos_assert(!this->is_counter(x));

    switch (op) {
      case BinaryOperator::Add: {
        this->set(x, this->to_gauge(y) + GaugeT(z));
      } break;
      case BinaryOperator::Sub: {
        this->set(x, this->to_gauge(y) - GaugeT(z));
      } break;
      case BinaryOperator::Mul: {
        this->set(x, this->to_gauge(y) * z);
      } break;
      case BinaryOperator::Mod: {
        if (z == 0) {
          this->set_to_bottom();
          return;
        }

        IntervalT yi = this->to_interval(y);
        boost::optional< Number > n = yi.mod_to_sub(z);

        if (n) {
          // Equivalent to x = y - n
          this->set(x, this->to_gauge(y) - GaugeT(*n));
        } else {
          this->set(x, GaugeT(GaugeBoundT(0), GaugeBoundT(abs(z) - 1)));
        }
      } break;
      case BinaryOperator::Div:
      case BinaryOperator::Rem:
      case BinaryOperator::Shl:
      case BinaryOperator::Shr:
      case BinaryOperator::And:
      case BinaryOperator::Or:
      case BinaryOperator::Xor: {
        // use interval representation
        this->set(x,
                  apply_bin_operator(op, this->to_interval(y), IntervalT(z)));
      } break;
      default: {
        ikos_unreachable("unreachable");
      }
    }
  }

  void apply(BinaryOperator op,
             VariableRef x,
             const Number& y,
             VariableRef z) override {
    if (this->is_bottom()) {
      return;
    }

    ikos_assert(!this->is_counter(x));

    switch (op) {
      case BinaryOperator::Add: {
        this->set(x, GaugeT(y) + this->to_gauge(z));
      } break;
      case BinaryOperator::Sub: {
        this->set(x, GaugeT(y) - this->to_gauge(z));
      } break;
      case BinaryOperator::Mul: {
        this->set(x, this->to_gauge(z) * y);
      } break;
      case BinaryOperator::Div:
      case BinaryOperator::Rem:
      case BinaryOperator::Mod:
      case BinaryOperator::Shl:
      case BinaryOperator::Shr:
      case BinaryOperator::And:
      case BinaryOperator::Or:
      case BinaryOperator::Xor: {
        // use interval representation
        this->set(x,
                  apply_bin_operator(op, IntervalT(y), this->to_interval(z)));
      } break;
      default: {
        ikos_unreachable("unreachable");
      }
    }
  }

  void add(const LinearConstraintT& cst) override {
    this->add(LinearConstraintSystemT{cst});
  }

  void add(const LinearConstraintSystemT& csts) override {
    if (this->is_bottom()) {
      return;
    }

    LinearConstraintSystemT counters_csts;
    for (const LinearConstraintT& cst : csts) {
      if (cst.is_contradiction()) {
        this->set_to_bottom();
        return;
      }

      GaugeT g = this->to_gauge(cst.expression());

      if (cst.is_inequality()) { // e <= 0
        if (g.lb().is_finite()) {
          counters_csts.add(g.lb().lin_expr() <= 0);
        }
      } else if (cst.is_equality()) { // e == 0
        if (g.lb().is_finite()) {
          counters_csts.add(g.lb().lin_expr() <= 0);
        }
        if (g.ub().is_finite()) {
          counters_csts.add(g.ub().lin_expr() >= 0);
        }
      } else if (cst.is_disequation()) { // e != 0
        if (g.singleton()) {
          counters_csts.add(g.lb().lin_expr() != 0);
        }
      } else {
        ikos_unreachable("unreachable");
      }
    }

    this->_intervals.add(counters_csts);
  }

  /// \brief Set the gauge for the variable `x`
  void set(VariableRef x, const GaugeT& value) {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      ikos_assert(!this->is_counter(x));
      this->_gauges.set(x, value);
    }
  }

  void set(VariableRef x, const IntervalT& value) override {
    this->set(x, GaugeT(value));
  }

  void set(VariableRef x, const CongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      ikos_assert(!this->is_counter(x));
      boost::optional< Number > n = value.singleton();
      if (n) {
        this->_gauges.set(x, GaugeT(*n));
      } else {
        this->_gauges.forget(x);
      }
    }
  }

  void set(VariableRef x, const IntervalCongruenceT& value) override {
    this->set(x, GaugeT(value.interval()));
  }

  /// \brief Refine the gauge for the variable `x`
  void refine(VariableRef x, const GaugeT& value) {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      ikos_assert(!this->is_counter(x));
      this->_gauges.refine(x, value);
    }
  }

  void refine(VariableRef x, const IntervalT& value) override {
    this->refine(x, GaugeT(value));
  }

  void refine(VariableRef x, const CongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      ikos_assert(!this->is_counter(x));
      IntervalCongruenceT iv(this->to_interval(x), value);
      this->refine(x, GaugeT(iv.interval()));
    }
  }

  void refine(VariableRef x, const IntervalCongruenceT& value) override {
    if (this->is_bottom()) {
      return;
    } else if (value.is_bottom()) {
      this->set_to_bottom();
    } else {
      ikos_assert(!this->is_counter(x));
      IntervalCongruenceT iv(this->to_interval(x));
      iv.meet_with(value);
      this->refine(x, GaugeT(iv.interval()));
    }
  }

  void forget(VariableRef x) override {
    if (this->is_bottom()) {
      return;
    } else if (this->is_counter(x)) {
      this->counter_forget(x);
    } else {
      this->_gauges.forget(x);
    }
  }

  /// \brief Projection to a gauge
  ///
  /// Return an overapproximation of the value of `x` as an interval
  GaugeT to_gauge(VariableRef x) const {
    if (this->is_bottom()) {
      return GaugeT::bottom();
    } else if (this->is_counter(x)) {
      return GaugeT(GaugeBoundT(x));
    } else {
      return this->_gauges.get(x);
    }
  }

  /// \brief Projection to a gauge
  ///
  /// Return an overapproximation of the linear expression `e` as a gauge
  GaugeT to_gauge(const LinearExpressionT& e) const {
    if (this->is_bottom()) {
      return GaugeT::bottom();
    }

    GaugeT r(e.constant());
    for (const auto& term : e) {
      if (this->is_counter(term.first)) {
        r += GaugeT(GaugeBoundT(term.second, term.first));
      } else {
        r += this->_gauges.get(term.first) * term.second;
      }
    }
    return r;
  }

  /// \brief Projection to an interval
  ///
  /// Return an overapproximation of the gauge `g` as an interval
  IntervalT to_interval(const GaugeT& g) const {
    if (this->is_bottom() || g.is_bottom()) {
      return IntervalT::bottom();
    } else {
      BoundT lb = g.lb().is_finite()
                      ? this->_intervals.to_interval(g.lb().lin_expr()).lb()
                      : BoundT::minus_infinity();
      BoundT ub = g.ub().is_finite()
                      ? this->_intervals.to_interval(g.ub().lin_expr()).ub()
                      : BoundT::plus_infinity();
      return IntervalT(lb, ub);
    }
  }

  IntervalT to_interval(VariableRef x) const override {
    if (this->is_bottom()) {
      return IntervalT::bottom();
    } else if (this->is_counter(x)) {
      return this->_intervals.to_interval(x);
    } else {
      return this->to_interval(this->_gauges.get(x));
    }
  }

  IntervalT to_interval(const LinearExpressionT& e) const override {
    if (this->is_bottom()) {
      return IntervalT::bottom();
    } else {
      return this->to_interval(this->to_gauge(e));
    }
  }

  CongruenceT to_congruence(VariableRef x) const override {
    if (this->is_bottom()) {
      return CongruenceT::bottom();
    }

    boost::optional< Number > n = this->to_interval(x).singleton();
    if (n) {
      return CongruenceT(*n);
    } else {
      return CongruenceT::top();
    }
  }

  CongruenceT to_congruence(const LinearExpressionT& e) const override {
    if (this->is_bottom()) {
      return CongruenceT::bottom();
    }

    boost::optional< Number > n = this->to_interval(e).singleton();
    if (n) {
      return CongruenceT(*n);
    } else {
      return CongruenceT::top();
    }
  }

  IntervalCongruenceT to_interval_congruence(VariableRef x) const override {
    return IntervalCongruenceT(this->to_interval(x));
  }

  IntervalCongruenceT to_interval_congruence(
      const LinearExpressionT& e) const override {
    return IntervalCongruenceT(this->to_interval(e));
  }

  LinearConstraintSystemT to_linear_constraint_system() const override {
    if (this->is_bottom()) {
      return LinearConstraintSystemT(LinearConstraintT::contradiction());
    }

    LinearConstraintSystemT csts;
    csts = this->_gauges.to_linear_constraint_system();
    csts.add(this->_intervals.to_linear_constraint_system());
    return csts;
  }

  /// \name Non-negative loop counter abstract domain methods
  /// @{

  /// \brief Mark the variable `x` as a non-negative loop counter
  void counter_mark(VariableRef x) override {
    if (this->is_bottom()) {
      return;
    }

    if (this->_counters.contains(x)) {
      return;
    }

    IntervalT itv = this->to_interval(x);
    itv.meet_with(IntervalT(BoundT(0), BoundT::plus_infinity()));

    if (itv.is_bottom()) {
      this->set_to_bottom();
      return;
    }

    if (itv.singleton()) {
      this->_sections.set(x, ConstantT(*itv.singleton()));
    }
    this->_gauges.forget(x);
    this->_counters.insert(x);
    this->_intervals.set(x, itv);
  }

  /// \brief Mark the variable `x` as a normal variable, without losing
  /// information
  void counter_unmark(VariableRef x) override {
    if (this->is_bottom()) {
      return;
    }

    if (!this->_counters.contains(x)) {
      return;
    }

    IntervalT itv = this->_intervals.to_interval(x);

    this->_sections.forget(x);
    this->_gauges.counter_forget(x, itv);
    this->_gauges.set(x, GaugeT(itv));
    this->_counters.erase(x);
    this->_intervals.forget(x);
  }

  /// \brief Initialize a non-negative loop counter: `x = c`
  ///
  /// Precondition: `c >= 0`
  void counter_init(VariableRef x, const Number& c) override {
    ikos_assert(c >= 0);

    if (this->is_bottom()) {
      return;
    }

    if (this->_counters.contains(x)) {
      this->_gauges.counter_forget(x, this->_intervals.to_interval(x));
    } else {
      this->_gauges.forget(x);
    }

    this->_sections.assign(x, c);
    this->_counters.insert(x);
    this->_intervals.assign(x, c);
  }

  /// \brief Increment a non-negative loop counter counter: `x += k`
  ///
  /// Precondition: `k >= 0`
  void counter_incr(VariableRef x, const Number& k) override {
    ikos_assert(k >= 0);

    if (this->is_bottom()) {
      return;
    }

    this->_sections.apply(BinaryOperator::Add, x, x, k);
    this->_gauges.counter_incr(x, k);
    this->_intervals.apply(BinaryOperator::Add, x, x, k);
  }

  /// \brief Forget a non-negative loop counter
  void counter_forget(VariableRef x) override {
    if (this->is_bottom()) {
      return;
    }

    IntervalT itv = this->_intervals.to_interval(x);

    this->_sections.forget(x);
    this->_gauges.counter_forget(x, itv);
    this->_counters.erase(x);
    this->_intervals.forget(x);
  }

  /// @}

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else {
      o << "(sections=";
      this->_sections.dump(o);
      o << ", gauges=";
      this->_gauges.dump(o);
      o << ", counters=";
      this->_counters.dump(o);
      o << ", intervals=";
      this->_intervals.dump(o);
      o << ")";
    }
  }

  static std::string name() { return "gauge domain"; }

}; // end class GaugeDomain

} // end namespace numeric
} // end namespace core
} // end namespace ikos
