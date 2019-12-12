/*******************************************************************************
 *
 * \file
 * \brief IntervalCongruence class
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
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

#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/number/bound.hpp>
#include <ikos/core/value/numeric/congruence.hpp>
#include <ikos/core/value/numeric/interval.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief Interval-Congruence abstract value
template < typename Number >
class IntervalCongruence {};

/// \brief Interval-Congruence abstract value
///
/// This is implemented as a pair of interval and congruence abstract value.
template <>
class IntervalCongruence< ZNumber > final
    : public core::AbstractDomain< IntervalCongruence< ZNumber > > {
public:
  using NumberT = ZNumber;
  using BoundT = Bound< ZNumber >;
  using IntervalT = Interval< ZNumber >;
  using CongruenceT = Congruence< ZNumber >;

private:
  IntervalT _i;
  CongruenceT _c;

private:
  /// \brief r(c, a) is the smallest element of c greater or equal than a
  static ZNumber r(const CongruenceT& c, const ZNumber& a) {
    ikos_assert(!c.is_bottom() && c.modulus() != 0);
    return a + mod(c.residue() - a, c.modulus());
  }

  /// \brief l(c, a) is the greatest element of c smaller or equal than a
  static ZNumber l(const CongruenceT& c, const ZNumber& a) {
    ikos_assert(!c.is_bottom() && c.modulus() != 0);
    return a - mod(a - c.residue(), c.modulus());
  }

  /// \brief Reduction
  ///
  /// Let (i, c) be a pair of interval and congruence
  /// if (c.is_bottom() || i.is_bottom()) (bottom(), bottom());
  /// if (c = 0Z+a and a notin i)         (bottom(), bottom());
  /// if (c = 0Z+a)                       ([a,a]   , c);
  /// if (i=[a,b] and r(c,a) > l(c,b))    (bottom(), bottom());
  /// if (i=[a,b])                        ([r(c,a), l(c,b)], c);
  /// if (i=[a,+oo])                      ([r(c,a), +oo], c);
  /// if (i=[-oo,b])                      ([-oo, l(c,b)], c);
  /// otherwise                           (i,c)
  void reduce() {
    if (this->_c.is_bottom()) {
      this->_i.set_to_bottom();
      return;
    }

    if (this->_i.is_bottom()) {
      this->_c.set_to_bottom();
      return;
    }

    if (this->_c.is_top()) {
      // congruence is top and interval is a singleton
      boost::optional< ZNumber > n = this->_i.singleton();
      if (n) {
        this->_c = CongruenceT(*n);
      }
      return;
    }

    if (this->_c.modulus() == 0) {
      // congruence is a singleton, so we refine the interval
      if (!this->_i.contains(this->_c.residue())) {
        this->_i.set_to_bottom();
        this->_c.set_to_bottom();
      } else {
        this->_i = IntervalT(this->_c.residue());
      }
      return;
    }

    // refine lower and upper bounds of the interval using congruences
    const BoundT& lb = this->_i.lb();
    const BoundT& ub = this->_i.ub();

    if (lb.is_finite() && ub.is_finite()) {
      ZNumber x = r(this->_c, *lb.number());
      ZNumber y = l(this->_c, *ub.number());
      if (x > y) {
        this->_i.set_to_bottom();
        this->_c.set_to_bottom();
      } else if (x == y) {
        this->_i = IntervalT(x);
        this->_c = CongruenceT(x);
      } else {
        this->_i = IntervalT(BoundT(x), BoundT(y));
      }
    } else if (lb.is_finite()) {
      ZNumber x = r(this->_c, *lb.number());
      this->_i = IntervalT(BoundT(x), ub);
    } else if (ub.is_finite()) {
      ZNumber y = l(this->_c, *ub.number());
      this->_i = IntervalT(lb, BoundT(y));
    }
  }

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top interval-congruence
  explicit IntervalCongruence(TopTag)
      : _i(IntervalT::top()), _c(CongruenceT::top()) {}

  /// \brief Create the bottom interval-congruence
  explicit IntervalCongruence(BottomTag)
      : _i(IntervalT::bottom()), _c(CongruenceT::bottom()) {}

public:
  /// \brief Create the top interval-congruence
  static IntervalCongruence top() { return IntervalCongruence(TopTag{}); }

  /// \brief Create the bottom interval-congruence
  static IntervalCongruence bottom() { return IntervalCongruence(BottomTag{}); }

  /// \brief Create the interval-congruence ([n, n], 0Z+n)
  explicit IntervalCongruence(int n) : _i(n), _c(n) {}

  /// \brief Create the interval-congruence ([n, n], 0Z+n)
  explicit IntervalCongruence(const ZNumber& n) : _i(n), _c(n) {}

  /// \brief Create the interval-congruence (i, c)
  IntervalCongruence(IntervalT i, CongruenceT c)
      : _i(std::move(i)), _c(std::move(c)) {
    this->reduce();
  }

  /// \brief Create the interval-congruence (i, T)
  explicit IntervalCongruence(IntervalT i)
      : _i(std::move(i)), _c(CongruenceT::top()) {
    this->reduce();
  }

  /// \brief Create the interval-congruence (T, c)
  explicit IntervalCongruence(CongruenceT c)
      : _i(IntervalT::top()), _c(std::move(c)) {
    this->reduce();
  }

  /// \brief Copy constructor
  IntervalCongruence(const IntervalCongruence&) = default;

  /// \brief Move constructor
  IntervalCongruence(IntervalCongruence&&) = default;

  /// \brief Copy assignment operator
  IntervalCongruence& operator=(const IntervalCongruence&) = default;

  /// \brief Move assignment operator
  IntervalCongruence& operator=(IntervalCongruence&&) noexcept = default;

  /// \brief Destructor
  ~IntervalCongruence() override = default;

  /// \brief Return the interval
  const IntervalT& interval() const { return this->_i; }

  /// \brief Return the congruence
  const CongruenceT& congruence() const { return this->_c; }

  void normalize() override {
    // Already performed by the reduction
  }

  bool is_bottom() const override {
    return this->_c.is_bottom(); // Correct because of reduction
  }

  bool is_top() const override {
    return this->_i.is_top() && this->_c.is_top();
  }

  void set_to_bottom() override {
    this->_i.set_to_bottom();
    this->_c.set_to_bottom();
  }

  void set_to_top() override {
    this->_i.set_to_top();
    this->_c.set_to_top();
  }

  bool leq(const IntervalCongruence& other) const override {
    return this->_i.leq(other._i) && this->_c.leq(other._c);
  }

  bool equals(const IntervalCongruence& other) const override {
    return this->_i.equals(other._i) && this->_c.equals(other._c);
  }

  IntervalCongruence join(const IntervalCongruence& other) const override {
    return IntervalCongruence(this->_i.join(other._i), this->_c.join(other._c));
  }

  void join_with(const IntervalCongruence& other) override {
    this->_i.join_with(other._i);
    this->_c.join_with(other._c);
    this->reduce();
  }

  IntervalCongruence widening(const IntervalCongruence& other) const override {
    return IntervalCongruence(this->_i.widening(other._i),
                              this->_c.widening(other._c));
  }

  void widen_with(const IntervalCongruence& other) override {
    this->_i.widen_with(other._i);
    this->_c.widen_with(other._c);
    this->reduce();
  }

  IntervalCongruence widening_threshold(const IntervalCongruence& other,
                                        const ZNumber& threshold) const {
    return IntervalCongruence(this->_i.widening_threshold(other._i, threshold),
                              this->_c.widening_threshold(other._c, threshold));
  }

  void widen_threshold_with(const IntervalCongruence& other,
                            const ZNumber& threshold) {
    this->_i.widen_threshold_with(other._i, threshold);
    this->_c.widen_threshold_with(other._c, threshold);
    this->reduce();
  }

  IntervalCongruence meet(const IntervalCongruence& other) const override {
    return IntervalCongruence(this->_i.meet(other._i), this->_c.meet(other._c));
  }

  void meet_with(const IntervalCongruence& other) override {
    this->_i.meet_with(other._i);
    this->_c.meet_with(other._c);
    this->reduce();
  }

  IntervalCongruence narrowing(const IntervalCongruence& other) const override {
    return IntervalCongruence(this->_i.narrowing(other._i),
                              this->_c.narrowing(other._c));
  }

  void narrow_with(const IntervalCongruence& other) override {
    this->_i.narrow_with(other._i);
    this->_c.narrow_with(other._c);
    this->reduce();
  }

  IntervalCongruence narrowing_threshold(const IntervalCongruence& other,
                                         const ZNumber& threshold) const {
    return IntervalCongruence(this->_i.narrowing_threshold(other._i, threshold),
                              this->_c.narrowing_threshold(other._c,
                                                           threshold));
  }

  void narrow_threshold_with(const IntervalCongruence& other,
                             const ZNumber& threshold) {
    this->_i.narrow_threshold_with(other._i, threshold);
    this->_c.narrow_threshold_with(other._c, threshold);
    this->reduce();
  }

  /// \brief Unary minus
  IntervalCongruence operator-() const {
    return IntervalCongruence(-this->_i, -this->_c);
  }

  /// \brief Add interval-congruences
  void operator+=(const IntervalCongruence& other) {
    this->_i += other._i;
    this->_c += other._c;
    this->reduce();
  }

  /// \brief If the interval-congruence is a singleton, return its value,
  /// otherwise return boost::none
  boost::optional< ZNumber > singleton() const {
    return this->_c.singleton(); // correct because of reduction
  }

  /// \brief Return true if the interval-congruence contains n
  bool contains(int n) const {
    return this->_i.contains(n) && this->_c.contains(n);
  }

  /// \brief Return true if the interval-congruence contains n
  bool contains(const ZNumber& n) const {
    return this->_i.contains(n) && this->_c.contains(n);
  }

  void dump(std::ostream& o) const override {
    o << "(" << this->_i << ", " << this->_c << ")";
  }

  static std::string name() { return "interval-congruence"; }

}; // end class IntervalCongruence< ZNumber >

/// \brief Add interval-congruences
inline IntervalCongruence< ZNumber > operator+(
    const IntervalCongruence< ZNumber >& lhs,
    const IntervalCongruence< ZNumber >& rhs) {
  return IntervalCongruence< ZNumber >(lhs.interval() + rhs.interval(),
                                       lhs.congruence() + rhs.congruence());
}

/// \brief Substract interval-congruences
inline IntervalCongruence< ZNumber > operator-(
    const IntervalCongruence< ZNumber >& lhs,
    const IntervalCongruence< ZNumber >& rhs) {
  return IntervalCongruence< ZNumber >(lhs.interval() - rhs.interval(),
                                       lhs.congruence() - rhs.congruence());
}

/// \brief Multiply interval-congruences
inline IntervalCongruence< ZNumber > operator*(
    const IntervalCongruence< ZNumber >& lhs,
    const IntervalCongruence< ZNumber >& rhs) {
  return IntervalCongruence< ZNumber >(lhs.interval() * rhs.interval(),
                                       lhs.congruence() * rhs.congruence());
}

/// \brief Divide interval-congruences
inline IntervalCongruence< ZNumber > operator/(
    const IntervalCongruence< ZNumber >& lhs,
    const IntervalCongruence< ZNumber >& rhs) {
  return IntervalCongruence< ZNumber >(lhs.interval() / rhs.interval(),
                                       lhs.congruence() / rhs.congruence());
}

/// \brief Remainder of interval-congruences
inline IntervalCongruence< ZNumber > operator%(
    const IntervalCongruence< ZNumber >& lhs,
    const IntervalCongruence< ZNumber >& rhs) {
  return IntervalCongruence< ZNumber >(lhs.interval() % rhs.interval(),
                                       lhs.congruence() % rhs.congruence());
}

/// \brief Modulo of interval-congruences
inline IntervalCongruence< ZNumber > mod(
    const IntervalCongruence< ZNumber >& lhs,
    const IntervalCongruence< ZNumber >& rhs) {
  return IntervalCongruence< ZNumber >(mod(lhs.interval(), rhs.interval()),
                                       mod(lhs.congruence(), rhs.congruence()));
}

/// \brief Left binary shift of interval-congruences
inline IntervalCongruence< ZNumber > operator<<(
    const IntervalCongruence< ZNumber >& lhs,
    const IntervalCongruence< ZNumber >& rhs) {
  return IntervalCongruence< ZNumber >(lhs.interval() << rhs.interval(),
                                       lhs.congruence() << rhs.congruence());
}

/// \brief Right binary shift of interval-congruences
inline IntervalCongruence< ZNumber > operator>>(
    const IntervalCongruence< ZNumber >& lhs,
    const IntervalCongruence< ZNumber >& rhs) {
  return IntervalCongruence< ZNumber >(lhs.interval() >> rhs.interval(),
                                       lhs.congruence() >> rhs.congruence());
}

/// \brief Bitwise AND of interval-congruences
inline IntervalCongruence< ZNumber > operator&(
    const IntervalCongruence< ZNumber >& lhs,
    const IntervalCongruence< ZNumber >& rhs) {
  return IntervalCongruence< ZNumber >(lhs.interval() & rhs.interval(),
                                       lhs.congruence() & rhs.congruence());
}

/// \brief Bitwise OR of interval-congruences
inline IntervalCongruence< ZNumber > operator|(
    const IntervalCongruence< ZNumber >& lhs,
    const IntervalCongruence< ZNumber >& rhs) {
  return IntervalCongruence< ZNumber >(lhs.interval() | rhs.interval(),
                                       lhs.congruence() | rhs.congruence());
}

/// \brief Bitwise XOR of interval-congruences
inline IntervalCongruence< ZNumber > operator^(
    const IntervalCongruence< ZNumber >& lhs,
    const IntervalCongruence< ZNumber >& rhs) {
  return IntervalCongruence< ZNumber >(lhs.interval() ^ rhs.interval(),
                                       lhs.congruence() ^ rhs.congruence());
}

/// \brief Interval-Congruence abstract value on rationals
///
/// The congruence domain on rationals is just a wrapper of the constant domain.
/// We instead implement this as the interval domain alone.
template <>
class IntervalCongruence< QNumber > final
    : public core::AbstractDomain< IntervalCongruence< QNumber > > {
public:
  using NumberT = QNumber;

private:
  QInterval _i;

public:
  static IntervalCongruence top() {
    return IntervalCongruence(QInterval::top());
  }

  static IntervalCongruence bottom() {
    return IntervalCongruence(QInterval::bottom());
  }

  explicit IntervalCongruence(int n) : _i(n) {}

  explicit IntervalCongruence(const QNumber& n) : _i(n) {}

  IntervalCongruence(QInterval i, const QCongruence&) : _i(std::move(i)) {}

  explicit IntervalCongruence(QInterval i) : _i(std::move(i)) {}

  explicit IntervalCongruence(const QCongruence&) : _i(QInterval::top()) {}

  IntervalCongruence(const IntervalCongruence&) = default;

  IntervalCongruence(IntervalCongruence&&) = default;

  IntervalCongruence& operator=(const IntervalCongruence&) = default;

  IntervalCongruence& operator=(IntervalCongruence&&) = default;

  ~IntervalCongruence() override = default;

  const QInterval& interval() const { return this->_i; }

  QCongruence congruence() const { return QCongruence::top(); }

  void normalize() override {}

  bool is_bottom() const override { return this->_i.is_bottom(); }

  bool is_top() const override { return this->_i.is_top(); }

  void set_to_bottom() override { this->_i.set_to_bottom(); }

  void set_to_top() override { this->_i.set_to_top(); }

  bool leq(const IntervalCongruence& other) const override {
    return this->_i.leq(other._i);
  }

  bool equals(const IntervalCongruence& other) const override {
    return this->_i.equals(other._i);
  }

  void join_with(const IntervalCongruence& other) override {
    this->_i.join_with(other._i);
  }

  void widen_with(const IntervalCongruence& other) override {
    this->_i.widen_with(other._i);
  }

  void widen_threshold_with(const IntervalCongruence& other,
                            const QNumber& threshold) {
    this->_i.widen_threshold_with(other._i, threshold);
  }

  void meet_with(const IntervalCongruence& other) override {
    this->_i.meet_with(other._i);
  }

  void narrow_with(const IntervalCongruence& other) override {
    this->_i.narrow_with(other._i);
  }

  IntervalCongruence operator-() const { return IntervalCongruence(-this->_i); }

  void operator+=(const IntervalCongruence& other) { this->_i += other._i; }

  boost::optional< QNumber > singleton() const { return this->_i.singleton(); }

  bool contains(int n) const { return this->_i.contains(n); }

  bool contains(const QNumber& n) const { return this->_i.contains(n); }

  void dump(std::ostream& o) const override { this->_i.dump(o); }

  static std::string name() { return "interval-congruence"; }

}; // end class IntervalCongruence< QNumber >

inline IntervalCongruence< QNumber > operator+(
    const IntervalCongruence< QNumber >& lhs,
    const IntervalCongruence< QNumber >& rhs) {
  return IntervalCongruence< QNumber >(lhs.interval() + rhs.interval());
}

inline IntervalCongruence< QNumber > operator-(
    const IntervalCongruence< QNumber >& lhs,
    const IntervalCongruence< QNumber >& rhs) {
  return IntervalCongruence< QNumber >(lhs.interval() - rhs.interval());
}

inline IntervalCongruence< QNumber > operator*(
    const IntervalCongruence< QNumber >& lhs,
    const IntervalCongruence< QNumber >& rhs) {
  return IntervalCongruence< QNumber >(lhs.interval() * rhs.interval());
}

inline IntervalCongruence< QNumber > operator/(
    const IntervalCongruence< QNumber >& lhs,
    const IntervalCongruence< QNumber >& rhs) {
  return IntervalCongruence< QNumber >(lhs.interval() / rhs.interval());
}

/// \brief Write an interval-congruence on a stream
template < typename Number >
inline std::ostream& operator<<(std::ostream& o,
                                const IntervalCongruence< Number >& ic) {
  ic.dump(o);
  return o;
}

} // end namespace numeric
} // end namespace core
} // end namespace ikos
