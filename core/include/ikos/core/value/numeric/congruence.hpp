/*******************************************************************************
 *
 * \file
 * \brief Congruence value for the congruence domain
 *
 * Author: Alexandre C. D. Wimmers
 *
 * Contributors: Jorge A. Navas
 *               Maxime Arthaud
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

#include <boost/optional.hpp>

#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/number.hpp>
#include <ikos/core/value/numeric/constant.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief Congruence abstract value
template < typename Number >
class Congruence {};

/// \brief Congruence abstract value
///
/// This is either bottom, or aZ + b.
///
/// Note that top is represented as 1Z + 0.
template <>
class Congruence< ZNumber > final
    : public core::AbstractDomain< Congruence< ZNumber > > {
public:
  using NumberT = ZNumber;

private:
  bool _is_bottom;
  ZNumber _a;
  ZNumber _b;

  // Invariant: !_is_bottom => _a >= 0
  // Invariant: !_is_bottom && _a != 0 => 0 <= _b < _a
  // Invariant: _is_bottom => _a == _b == 0

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top congruence 1Z + 0
  explicit Congruence(TopTag) : _is_bottom(false), _a(1), _b(0) {}

  /// \brief Create the bottom congruence
  explicit Congruence(BottomTag) : _is_bottom(true), _a(0), _b(0) {}

  /// \brief Reduce the congruence
  void reduce() {
    ikos_assert(this->_a >= 0);

    // if a != 0: 0 <= b < a
    if (this->_a != 0) {
      this->_b = mod(this->_b, this->_a);
    }
  }

public:
  /// \brief Create the congruence 1Z + 0
  static Congruence top() { return Congruence(TopTag{}); }

  /// \brief Create the bottom congruence
  static Congruence bottom() { return Congruence(BottomTag{}); }

  /// \brief Create the congruence 0Z + n
  explicit Congruence(int n) : _is_bottom(false), _a(0), _b(n) {}

  /// \brief Create the congruence 0Z + n
  explicit Congruence(ZNumber n) : _is_bottom(false), _a(0), _b(std::move(n)) {}

  /// \brief Create the congruence aZ + b
  Congruence(ZNumber a, ZNumber b)
      : _is_bottom(false), _a(std::move(a)), _b(std::move(b)) {
    this->reduce();
  }

  /// \brief Copy constructor
  Congruence(const Congruence&) = default;

  /// \brief Move constructor
  Congruence(Congruence&&) = default;

  /// \brief Copy assignment operator
  Congruence& operator=(const Congruence&) = default;

  /// \brief Move assignment operator
  Congruence& operator=(Congruence&&) noexcept = default;

  /// \brief Destructor
  ~Congruence() override = default;

  /// \brief Return the modulus
  const ZNumber& modulus() const {
    ikos_assert(!this->is_bottom());
    return this->_a;
  }

  /// \brief Return the residue
  const ZNumber& residue() const {
    ikos_assert(!this->is_bottom());
    return this->_b;
  }

  void normalize() override {
    // Already performed by reduction
  }

  bool is_bottom() const override { return this->_is_bottom; }

  bool is_top() const override { return this->_a == 1; }

  /// \brief Return true if the congruence is 0Z + 0
  bool is_zero() const {
    return !this->_is_bottom && this->_a == 0 && this->_b == 0;
  }

  void set_to_bottom() override {
    this->_is_bottom = true;
    this->_a = 0;
    this->_b = 0;
  }

  void set_to_top() override {
    this->_is_bottom = false;
    this->_a = 1;
    this->_b = 0;
  }

  bool leq(const Congruence& other) const override {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else if (this->_a == 0 && other._a == 0) {
      return this->_b == other._b;
    } else if (this->_a == 0) {
      return mod(this->_b, other._a) == other._b;
    } else if (other._a == 0) {
      return false;
    } else {
      return mod(this->_a, other._a) == 0 &&
             mod(this->_b, other._a) == other._b;
    }
  }

  bool equals(const Congruence& other) const override {
    if (this->is_bottom()) {
      return other.is_bottom();
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_a == other._a && this->_b == other._b;
    }
  }

  Congruence join(const Congruence& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return Congruence(gcd(this->_a, other._a, abs(this->_b - other._b)),
                        min(this->_b, other._b));
    }
  }

  void join_with(const Congruence& other) override {
    this->operator=(this->join(other));
  }

  Congruence widening(const Congruence& other) const override {
    // equivalent to join, domain is flat
    return this->join(other);
  }

  void widen_with(const Congruence& other) override {
    // equivalent to join, domain is flat
    this->join_with(other);
  }

  Congruence widening_threshold(const Congruence& other,
                                const ZNumber& /*threshold*/) const {
    // equivalent to join, domain is flat
    return this->join(other);
  }

  void widen_threshold_with(const Congruence& other,
                            const ZNumber& /*threshold*/) {
    // equivalent to join, domain is flat
    this->join_with(other);
  }

  Congruence meet(const Congruence& other) const override {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    }

    // Note: lcm has meaning only if both a and o.a are not 0
    if (this->_a == 0 && other._a == 0) {
      if (this->_b == other._b) {
        return *this;
      } else {
        return bottom();
      }
    } else if (this->_a == 0) {
      if (mod(this->_b, other._a) == other._b) {
        return *this;
      } else {
        return bottom();
      }
    } else if (other._a == 0) {
      if (mod(other._b, this->_a) == this->_b) {
        return other;
      } else {
        return bottom();
      }
    } else {
      // pre: a and o.a != 0
      ZNumber x; // gcd(a, o.a)
      ZNumber u;
      ZNumber v;
      gcd_extended(this->_a, other._a, x, u, v);
      if (mod(this->_b, x) == mod(other._b, x)) {
        // See http://www.dsi.unive.it/~avp/domains.pdf
        // See https://math.stackexchange.com/a/1644698
        ZNumber lambda = (this->_b - other._b) / x;
        ZNumber sigma = this->_b - this->_a * u * lambda;
        return Congruence(lcm(this->_a, other._a), sigma);
      } else {
        return bottom();
      }
    }
  }

  void meet_with(const Congruence& other) override {
    this->operator=(this->meet(other));
  }

  Congruence narrowing(const Congruence& other) const override {
    // equivalent to meet, domain is flat
    return this->meet(other);
  }

  void narrow_with(const Congruence& other) override {
    // equivalent to meet, domain is flat
    this->meet_with(other);
  }

  Congruence narrowing_threshold(const Congruence& other,
                                 const ZNumber& /*threshold*/) const {
    // equivalent to meet, domain is flat
    return this->meet(other);
  }

  void narrow_threshold_with(const Congruence& other,
                             const ZNumber& /*threshold*/) {
    // equivalent to meet, domain is flat
    this->meet_with(other);
  }

  /// \brief Unary minus
  Congruence operator-() const {
    if (this->is_bottom() || this->is_top()) {
      return *this;
    } else {
      return Congruence(this->_a, -this->_b);
    }
  }

  /// \brief Add a congruence
  void operator+=(const Congruence& other) {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_a = gcd(this->_a, other._a);
      this->_b += other._b;
      this->reduce();
    }
  }

  /// \brief Substract a congruence
  void operator-=(const Congruence& other) {
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_a = gcd(this->_a, other._a);
      this->_b -= other._b;
      this->reduce();
    }
  }

  /// \brief If the congruence is a singleton 0Z + n, return n, otherwise return
  /// boost::none
  boost::optional< ZNumber > singleton() const {
    if (!this->is_bottom() && this->_a == 0) {
      return this->_b;
    } else {
      return boost::none;
    }
  }

  /// \brief Return true if the congruence contains n
  bool contains(int n) const { return this->contains(ZNumber(n)); }

  /// \brief Return true if the congruence contains n
  bool contains(const ZNumber& n) const {
    if (this->is_bottom()) {
      return false;
    } else if (this->_a == 0) {
      return n == this->_b;
    } else {
      return mod(n, this->_a) == this->_b;
    }
  }

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
      return;
    }

    if (this->_a == 0) {
      o << this->_b;
      return;
    }

    if (this->_b == 0) {
      o << this->_a << "Z";
    } else {
      o << this->_a << "Z+" << this->_b;
    }
  }

  static std::string name() { return "congruence"; }

}; // end class Congruence< ZNumber >

/// \brief Add congruences
inline Congruence< ZNumber > operator+(const Congruence< ZNumber >& lhs,
                                       const Congruence< ZNumber >& rhs) {
  using CongruenceT = Congruence< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return CongruenceT::bottom();
  } else {
    return CongruenceT(gcd(lhs.modulus(), rhs.modulus()),
                       lhs.residue() + rhs.residue());
  }
}

/// \brief Add a congruence and a number
inline Congruence< ZNumber > operator+(const Congruence< ZNumber >& lhs,
                                       const ZNumber& rhs) {
  using CongruenceT = Congruence< ZNumber >;

  if (lhs.is_bottom()) {
    return CongruenceT::bottom();
  } else {
    return CongruenceT(lhs.modulus(), lhs.residue() + rhs);
  }
}

/// \brief Substract congruences
inline Congruence< ZNumber > operator-(const Congruence< ZNumber >& lhs,
                                       const Congruence< ZNumber >& rhs) {
  using CongruenceT = Congruence< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return CongruenceT::bottom();
  } else {
    return CongruenceT(gcd(lhs.modulus(), rhs.modulus()),
                       lhs.residue() - rhs.residue());
  }
}

/// \brief Substract a number
inline Congruence< ZNumber > operator-(const Congruence< ZNumber >& lhs,
                                       const ZNumber& rhs) {
  using CongruenceT = Congruence< ZNumber >;

  if (lhs.is_bottom()) {
    return CongruenceT::bottom();
  } else {
    return CongruenceT(lhs.modulus(), lhs.residue() - rhs);
  }
}

/// \brief Multiply congruences
inline Congruence< ZNumber > operator*(const Congruence< ZNumber >& lhs,
                                       const Congruence< ZNumber >& rhs) {
  using CongruenceT = Congruence< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return CongruenceT::bottom();
  } else {
    return CongruenceT(gcd(lhs.modulus() * rhs.modulus(),
                           lhs.modulus() * rhs.residue(),
                           rhs.modulus() * lhs.residue()),
                       lhs.residue() * rhs.residue());
  }
}

/// \brief Divide congruences
inline Congruence< ZNumber > operator/(const Congruence< ZNumber >& lhs,
                                       const Congruence< ZNumber >& rhs) {
  using CongruenceT = Congruence< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return CongruenceT::bottom();
  } else if (rhs.modulus() == 0 && rhs.residue() == 0) {
    return CongruenceT::bottom();
  } else {
    // Stefan Bygde's paper "Static WCET analysis based on abstract
    // interpretation and counting of elements" provides a precise operator,
    // but it only works with positive numbers.

    // Special case: 0Z+0 / a'Z+b'
    if (lhs.modulus() == 0 && lhs.residue() == 0) {
      return CongruenceT(0);
    }

    // Special case: 0Z+b / 0Z+b'
    if (lhs.modulus() == 0 && rhs.modulus() == 0) {
      ikos_assert(rhs.residue() != 0);

      return CongruenceT(lhs.residue() / rhs.residue());
    }

    // Special case: aZ+b / 0Z+b'
    // if b'|a and b'|b then abs(a/b')Z + b/b'
    if (rhs.modulus() == 0) {
      ikos_assert(rhs.residue() != 0);

      if (mod(lhs.modulus(), rhs.residue()) == 0 &&
          mod(lhs.residue(), rhs.residue()) == 0) {
        return CongruenceT(abs(lhs.modulus() / rhs.residue()),
                           lhs.residue() / rhs.residue());
      }
    }

    // General case
    return CongruenceT::top();
  }
}

/// \brief Signed remainder
inline Congruence< ZNumber > operator%(const Congruence< ZNumber >& lhs,
                                       const Congruence< ZNumber >& rhs) {
  using CongruenceT = Congruence< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return CongruenceT::bottom();
  } else if (rhs.modulus() == 0 && rhs.residue() == 0) {
    return CongruenceT::bottom();
  } else {
    // Stefan Bygde's paper "Static WCET analysis based on abstract
    // interpretation and counting of elements" provides a precise operator,
    // but it only works with positive numbers.

    // Special case: 0Z+0 % a'Z+b'
    if (lhs.modulus() == 0 && lhs.residue() == 0) {
      return CongruenceT(0);
    }

    // Special case: 0Z+b % a'Z+b'
    if (lhs.modulus() == 0 && rhs.modulus() == 0) {
      ikos_assert(rhs.residue() != 0);

      return CongruenceT(lhs.residue() % rhs.residue());
    }

    // Special case: aZ+b % 0Z+b'
    if (rhs.modulus() == 0) {
      ikos_assert(rhs.residue() != 0);

      if (mod(lhs.modulus(), rhs.residue()) == 0) {
        ZNumber n = lhs.residue() % rhs.residue();
        ZNumber m = (lhs.residue() - lhs.modulus()) % rhs.residue();
        return CongruenceT(n).join(CongruenceT(m));
      }
    }

    // Special case: 0Z+b % a'Z+b'
    if (lhs.modulus() == 0) {
      ZNumber n = rhs.modulus() *
                      ((abs(lhs.residue()) - rhs.residue()) / rhs.modulus()) +
                  rhs.residue();
      if (n <= 0) {
        return CongruenceT(lhs.residue());
      }
    }

    // General case
    return CongruenceT(gcd(lhs.modulus(), rhs.modulus(), rhs.residue()),
                       lhs.residue());
  }
}

/// \brief Modulo
inline Congruence< ZNumber > mod(const Congruence< ZNumber >& lhs,
                                 const Congruence< ZNumber >& rhs) {
  using CongruenceT = Congruence< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return CongruenceT::bottom();
  } else if (rhs.modulus() == 0 && rhs.residue() == 0) {
    return CongruenceT::bottom();
  } else {
    // Stefan Bygde's paper "Static WCET analysis based on abstract
    // interpretation and counting of elements" provides a precise operator,
    // but it only works with positive numbers.

    // Special case: mod(0Z+0, a'Z+b')
    if (lhs.modulus() == 0 && lhs.residue() == 0) {
      return CongruenceT(0);
    }

    // Special case: mod(aZ+b, 0Z+b'):
    // if b'|a then 0Z + mod(b, b')
    if (rhs.modulus() == 0) {
      ikos_assert(rhs.residue() != 0);

      if (mod(lhs.modulus(), rhs.residue()) == 0) {
        return CongruenceT(mod(lhs.residue(), rhs.residue()));
      }
    }

    // Special case: 0Z+b % a'Z+b'
    if (lhs.modulus() == 0 && lhs.residue() >= 0) {
      ZNumber n = rhs.modulus() *
                      ((abs(lhs.residue()) - rhs.residue()) / rhs.modulus()) +
                  rhs.residue();
      if (n <= 0) {
        return CongruenceT(lhs.residue());
      }
    }

    // General case
    return CongruenceT(gcd(lhs.modulus(), rhs.modulus(), rhs.residue()),
                       lhs.residue());
  }
}

/// \brief Left binary shift of congruences
inline Congruence< ZNumber > operator<<(const Congruence< ZNumber >& lhs,
                                        const Congruence< ZNumber >& rhs) {
  using CongruenceT = Congruence< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return CongruenceT::bottom();
  } else {
    if (rhs.modulus() == 0) {
      // aZ + b << 0Z + b'
      const ZNumber& b = rhs.residue();

      if (b < 0) {
        // Invalid operand
        return CongruenceT::bottom();
      }

      // aZ + b << 0Z + b' = (a*2^b')Z + b*2^b'
      ZNumber x = 1 << b;
      return CongruenceT(lhs.modulus() * x, lhs.residue() * x);
    }

    // aZ + b << a'Z + b' = (gcd(a, b * (2^a' - 1)))*(2^b')Z + b*(2^b')
    ZNumber x = 1 << rhs.residue();
    ZNumber y = 1 << rhs.modulus();
    ZNumber a = gcd(lhs.modulus(), lhs.residue() * (y - 1)) * x;
    ZNumber b = lhs.residue() * x;
    return CongruenceT(a, b);
  }
}

/// \brief Right binary shift of congruences
inline Congruence< ZNumber > operator>>(const Congruence< ZNumber >& lhs,
                                        const Congruence< ZNumber >& rhs) {
  using CongruenceT = Congruence< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return CongruenceT::bottom();
  } else {
    if (rhs.modulus() == 0) {
      // aZ + b >> 0Z + b'
      const ZNumber& b = rhs.residue();

      if (b < 0) {
        // Invalid operand
        return CongruenceT::bottom();
      }
    }

    if (lhs.is_zero() || rhs.is_zero()) {
      // 0 >> aZ + b
      // aZ + b >> 0
      return lhs;
    }

    if (lhs.modulus() == 0 && rhs.modulus() == 0) {
      // 0Z + b >> 0Z + b'
      return CongruenceT(lhs.residue() >> rhs.residue());
    }

    return CongruenceT::top();
  }
}

/// \brief Bitwise AND of congruences
inline Congruence< ZNumber > operator&(const Congruence< ZNumber >& lhs,
                                       const Congruence< ZNumber >& rhs) {
  using CongruenceT = Congruence< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return CongruenceT::bottom();
  } else if (lhs.is_zero() || rhs.is_zero()) {
    return CongruenceT(0);
  } else if (lhs == CongruenceT(-1)) {
    return rhs;
  } else if (rhs == CongruenceT(-1)) {
    return lhs;
  } else if (lhs.modulus() == 0 && rhs.modulus() == 0) {
    return CongruenceT(lhs.residue() & rhs.residue());
  } else {
    return CongruenceT::top();
  }
}

/// \brief Bitwise OR of congruences
inline Congruence< ZNumber > operator|(const Congruence< ZNumber >& lhs,
                                       const Congruence< ZNumber >& rhs) {
  using CongruenceT = Congruence< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return CongruenceT::bottom();
  } else if (lhs == CongruenceT(-1) || rhs == CongruenceT(-1)) {
    return CongruenceT(-1);
  } else if (lhs.is_zero()) {
    return rhs;
  } else if (rhs.is_zero()) {
    return lhs;
  } else if (lhs.modulus() == 0 && rhs.modulus() == 0) {
    return CongruenceT(lhs.residue() | rhs.residue());
  } else {
    return CongruenceT::top();
  }
}

/// \brief Bitwise XOR of congruences
inline Congruence< ZNumber > operator^(const Congruence< ZNumber >& lhs,
                                       const Congruence< ZNumber >& rhs) {
  using CongruenceT = Congruence< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return CongruenceT::bottom();
  } else if (lhs.is_zero()) {
    return rhs;
  } else if (rhs.is_zero()) {
    return lhs;
  } else if (lhs.modulus() == 0 && rhs.modulus() == 0) {
    return CongruenceT(lhs.residue() ^ rhs.residue());
  } else {
    return CongruenceT::top();
  }
}

/// \brief Congruence on rationals
///
/// We do not try to define the congruence domain on rationals.
/// We instead implement this as the constant domain, for simplicity.
template <>
class Congruence< QNumber > final
    : public core::AbstractDomain< Congruence< QNumber > > {
public:
  using NumberT = QNumber;

private:
  QConstant _cst;

public:
  static Congruence top() { return Congruence(QConstant::top()); }

  static Congruence bottom() { return Congruence(QConstant::bottom()); }

  explicit Congruence(int n) : _cst(n) {}

  explicit Congruence(QNumber n) : _cst(std::move(n)) {}

  explicit Congruence(QConstant cst) : _cst(std::move(cst)) {}

  Congruence(const QNumber&, const QNumber&) : _cst(QConstant::top()) {}

  Congruence(const Congruence&) = default;

  Congruence(Congruence&&) = default;

  Congruence& operator=(const Congruence&) = default;

  Congruence& operator=(Congruence&&) noexcept = default;

  ~Congruence() override = default;

  void normalize() override { this->_cst.normalize(); }

  bool is_bottom() const override { return this->_cst.is_bottom(); }

  bool is_top() const override { return this->_cst.is_top(); }

  bool is_zero() const { return this->_cst.is_zero(); }

  void set_to_bottom() override { this->_cst.set_to_bottom(); }

  void set_to_top() override { this->_cst.set_to_top(); }

  bool leq(const Congruence& other) const override {
    return this->_cst.leq(other._cst);
  }

  bool equals(const Congruence& other) const override {
    return this->_cst.equals(other._cst);
  }

  void join_with(const Congruence& other) override {
    this->_cst.join_with(other._cst);
  }

  void widen_with(const Congruence& other) override {
    this->_cst.widen_with(other._cst);
  }

  void widen_threshold_with(const Congruence& other, const QNumber& threshold) {
    this->_cst.widen_threshold_with(other._cst, threshold);
  }

  void meet_with(const Congruence& other) override {
    this->_cst.meet_with(other._cst);
  }

  void narrow_with(const Congruence& other) override {
    this->_cst.narrow_with(other._cst);
  }

  void narrow_threshold_with(const Congruence& other,
                             const QNumber& threshold) {
    this->_cst.narrow_threshold_with(other._cst, threshold);
  }

  Congruence operator-() const { return Congruence(-this->_cst); }

  void operator+=(const Congruence& other) { this->_cst += other._cst; }

  void operator-=(const Congruence& other) { this->_cst -= other._cst; }

  const QConstant& to_constant() const { return this->_cst; }

  boost::optional< QNumber > singleton() const {
    return this->_cst.singleton();
  }

  bool contains(int n) const { return this->_cst.contains(n); }

  bool contains(const QNumber& n) const { return this->_cst.contains(n); }

  void dump(std::ostream& o) const override { this->_cst.dump(o); }

  static std::string name() { return "congruence"; }

}; // end class Congruence< QNumber >

inline Congruence< QNumber > operator+(const Congruence< QNumber >& lhs,
                                       const Congruence< QNumber >& rhs) {
  return Congruence< QNumber >(lhs.to_constant() + rhs.to_constant());
}

inline Congruence< QNumber > operator-(const Congruence< QNumber >& lhs,
                                       const Congruence< QNumber >& rhs) {
  return Congruence< QNumber >(lhs.to_constant() - rhs.to_constant());
}

inline Congruence< QNumber > operator*(const Congruence< QNumber >& lhs,
                                       const Congruence< QNumber >& rhs) {
  return Congruence< QNumber >(lhs.to_constant() * rhs.to_constant());
}

inline Congruence< QNumber > operator/(const Congruence< QNumber >& lhs,
                                       const Congruence< QNumber >& rhs) {
  return Congruence< QNumber >(lhs.to_constant() / rhs.to_constant());
}

/// \brief Write a congruence on a stream
template < typename Number >
inline std::ostream& operator<<(std::ostream& o,
                                const Congruence< Number >& congruence) {
  congruence.dump(o);
  return o;
}

/// \brief Congruence on unlimited precision integers
using ZCongruence = Congruence< ZNumber >;

/// \brief Congruence on unlimited precision rationals
using QCongruence = Congruence< QNumber >;

} // end namespace numeric
} // end namespace core
} // end namespace ikos
