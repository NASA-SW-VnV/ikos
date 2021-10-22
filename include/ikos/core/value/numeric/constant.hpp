/*******************************************************************************
 *
 * \file
 * \brief Constant abstract value
 *
 * Author: Arnaud J. Venet
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

#include <type_traits>

#include <boost/optional.hpp>

#include <ikos/core/domain/abstract_domain.hpp>
#include <ikos/core/number.hpp>

namespace ikos {
namespace core {
namespace numeric {

/// \brief Constant abstract value
///
/// This is either top, bottom or a constant.
template < typename Number >
class Constant final : public core::AbstractDomain< Constant< Number > > {
public:
  using NumberT = Number;

private:
  enum Kind { BottomKind, TopKind, NumberKind };

private:
  Kind _kind;
  Number _n;

private:
  struct TopTag {};
  struct BottomTag {};

  /// \brief Create the top constant
  explicit Constant(TopTag) : _kind(TopKind) {}

  /// \brief Create the bottom constant
  explicit Constant(BottomTag) : _kind(BottomKind) {}

public:
  /// \brief Create the top constant
  static Constant top() { return Constant(TopTag{}); }

  /// \brief Create the bottom constant
  static Constant bottom() { return Constant(BottomTag{}); }

  /// \brief Create the constant n
  explicit Constant(int n) : _kind(NumberKind), _n(n) {}

  /// \brief Create the constant n
  explicit Constant(Number n) : _kind(NumberKind), _n(std::move(n)) {}

  /// \brief Copy constructor
  Constant(const Constant&) noexcept(
      std::is_nothrow_copy_constructible< Number >::value) = default;

  /// \brief Move constructor
  Constant(Constant&&) noexcept(
      std::is_nothrow_move_constructible< Number >::value) = default;

  /// \brief Copy assignment operator
  Constant& operator=(const Constant&) noexcept(
      std::is_nothrow_copy_assignable< Number >::value) = default;

  /// \brief Move assignment operator
  Constant& operator=(Constant&&) noexcept(
      std::is_nothrow_move_assignable< Number >::value) = default;

  /// \brief Destructor
  ~Constant() override = default;

  void normalize() override {}

  bool is_bottom() const override { return this->_kind == BottomKind; }

  bool is_top() const override { return this->_kind == TopKind; }

  /// \brief Return true if the constant is a number
  bool is_number() const { return this->_kind == NumberKind; }

  /// \brief Return true if the congruence is 0Z + 0
  bool is_zero() const { return this->_kind == NumberKind && this->_n == 0; }

  void set_to_bottom() override { this->_kind = BottomKind; }

  void set_to_top() override { this->_kind = TopKind; }

  bool leq(const Constant& other) const override {
    switch (this->_kind) {
      case BottomKind:
        return true;
      case TopKind:
        return other._kind == TopKind;
      case NumberKind:
        return other._kind == TopKind ||
               (other._kind == NumberKind && this->_n == other._n);
      default:
        ikos_unreachable("unreachable");
    }
  }

  bool equals(const Constant& other) const override {
    if (this->_kind == NumberKind) {
      return other._kind == NumberKind && this->_n == other._n;
    } else {
      return this->_kind == other._kind;
    }
  }

  Constant join(const Constant& other) const override {
    if (this->is_bottom() || other.is_top()) {
      return other;
    } else if (this->is_top() || other.is_bottom()) {
      return *this;
    } else if (this->_n == other._n) {
      return *this;
    } else {
      return top();
    }
  }

  void join_with(const Constant& other) override {
    this->operator=(this->join(other));
  }

  Constant widening(const Constant& other) const override {
    // equivalent to join, domain is flat
    return this->join(other);
  }

  void widen_with(const Constant& other) override {
    // equivalent to join, domain is flat
    this->join_with(other);
  }

  Constant widening_threshold(const Constant& other,
                              const Number& /*threshold*/) const {
    // equivalent to join, domain is flat
    return this->join(other);
  }

  void widen_threshold_with(const Constant& other,
                            const Number& /*threshold*/) {
    // equivalent to join, domain is flat
    this->join_with(other);
  }

  Constant meet(const Constant& other) const override {
    if (this->is_bottom() || other.is_top()) {
      return *this;
    } else if (this->is_top() || other.is_bottom()) {
      return other;
    } else if (this->_n == other._n) {
      return *this;
    } else {
      return bottom();
    }
  }

  void meet_with(const Constant& other) override {
    this->operator=(this->meet(other));
  }

  Constant narrowing(const Constant& other) const override {
    // equivalent to meet, domain is flat
    return this->meet(other);
  }

  void narrow_with(const Constant& other) override {
    // equivalent to meet, domain is flat
    this->meet_with(other);
  }

  Constant narrowing_threshold(const Constant& other,
                               const Number& /*threshold*/) const {
    // equivalent to meet, domain is flat
    return this->meet(other);
  }

  void narrow_threshold_with(const Constant& other,
                             const Number& /*threshold*/) {
    // equivalent to meet, domain is flat
    this->meet_with(other);
  }

  /// \brief Unary minus
  Constant operator-() const {
    if (this->is_number()) {
      return Constant(-this->_n);
    } else {
      return *this;
    }
  }

  /// \brief Add constants
  void operator+=(const Constant& other) {
    if (this->is_bottom() || this->is_top()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else if (other.is_top()) {
      this->set_to_top();
    } else {
      this->_n += other._n;
    }
  }

  /// \brief Substract constants
  void operator-=(const Constant& other) {
    if (this->is_bottom() || this->is_top()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else if (other.is_top()) {
      this->set_to_top();
    } else {
      this->_n -= other._n;
    }
  }

  /// \brief If the constant in a number n, return n, otherwise return
  /// boost::none
  boost::optional< Number > number() const {
    if (this->is_number()) {
      return this->_n;
    } else {
      return boost::none;
    }
  }

  /// \brief If the constant in a singleton n, return n, otherwise return
  /// boost::none
  boost::optional< Number > singleton() const { return this->number(); }

  /// \brief Return true if the constant contains n
  bool contains(int n) const {
    return this->is_top() || (this->is_number() && this->_n == n);
  }

  /// \brief Return true if the constant contains n
  bool contains(const Number& n) const {
    return this->is_top() || (this->is_number() && this->_n == n);
  }

  void dump(std::ostream& o) const override {
    switch (this->_kind) {
      case BottomKind: {
        o << "⊥";
        break;
      }
      case TopKind: {
        o << "T";
        break;
      }
      case NumberKind: {
        o << this->_n;
        break;
      }
    }
  }

  static std::string name() { return "constant"; }

  // Friends

  template < typename T >
  friend Constant< T > operator+(const Constant< T >& lhs,
                                 const Constant< T >& rhs);

  template < typename T >
  friend Constant< T > operator-(const Constant< T >& lhs,
                                 const Constant< T >& rhs);

  template < typename T >
  friend Constant< T > operator*(const Constant< T >& lhs,
                                 const Constant< T >& rhs);

  template < typename T >
  friend Constant< T > operator/(const Constant< T >& lhs,
                                 const Constant< T >& rhs);

  friend Constant< ZNumber > operator%(const Constant< ZNumber >& lhs,
                                       const Constant< ZNumber >& rhs);

  friend Constant< ZNumber > mod(const Constant< ZNumber >& lhs,
                                 const Constant< ZNumber >& rhs);

  friend Constant< ZNumber > operator<<(const Constant< ZNumber >& lhs,
                                        const Constant< ZNumber >& rhs);

  friend Constant< ZNumber > operator>>(const Constant< ZNumber >& lhs,
                                        const Constant< ZNumber >& rhs);

  friend Constant< ZNumber > operator&(const Constant< ZNumber >& lhs,
                                       const Constant< ZNumber >& rhs);

  friend Constant< ZNumber > operator|(const Constant< ZNumber >& lhs,
                                       const Constant< ZNumber >& rhs);

  friend Constant< ZNumber > operator^(const Constant< ZNumber >& lhs,
                                       const Constant< ZNumber >& rhs);

}; // end class Constant

/// \brief Add constants
template < typename Number >
inline Constant< Number > operator+(const Constant< Number >& lhs,
                                    const Constant< Number >& rhs) {
  using ConstantT = Constant< Number >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return ConstantT::bottom();
  } else if (lhs.is_top() || rhs.is_top()) {
    return ConstantT::top();
  } else {
    return ConstantT(lhs._n + rhs._n);
  }
}

/// \brief Substract constants
template < typename Number >
inline Constant< Number > operator-(const Constant< Number >& lhs,
                                    const Constant< Number >& rhs) {
  using ConstantT = Constant< Number >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return ConstantT::bottom();
  } else if (lhs.is_top() || rhs.is_top()) {
    return ConstantT::top();
  } else {
    return ConstantT(lhs._n - rhs._n);
  }
}

/// \brief Multiply constants
template < typename Number >
inline Constant< Number > operator*(const Constant< Number >& lhs,
                                    const Constant< Number >& rhs) {
  using ConstantT = Constant< Number >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return ConstantT::bottom();
  } else if (lhs.is_zero() || rhs.is_zero()) {
    return ConstantT(0);
  } else if (lhs.is_top() || rhs.is_top()) {
    return ConstantT::top();
  } else {
    return ConstantT(lhs._n * rhs._n);
  }
}

/// \brief Divide constants
template < typename Number >
inline Constant< Number > operator/(const Constant< Number >& lhs,
                                    const Constant< Number >& rhs) {
  using ConstantT = Constant< Number >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return ConstantT::bottom();
  } else if (rhs.is_zero()) {
    return ConstantT::bottom();
  } else if (lhs.is_zero()) {
    return ConstantT(0);
  } else if (lhs.is_top() || rhs.is_top()) {
    return ConstantT::top();
  } else {
    return ConstantT(lhs._n / rhs._n);
  }
}

/// \brief Remainder of constants
inline Constant< ZNumber > operator%(const Constant< ZNumber >& lhs,
                                     const Constant< ZNumber >& rhs) {
  using ConstantT = Constant< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return ConstantT::bottom();
  } else if (rhs.is_zero()) {
    return ConstantT::bottom();
  } else if (lhs.is_zero()) {
    return ConstantT(0);
  } else if (lhs.is_top() || rhs.is_top()) {
    return ConstantT::top();
  } else {
    return ConstantT(lhs._n % rhs._n);
  }
}

/// \brief Modulo of constants
inline Constant< ZNumber > mod(const Constant< ZNumber >& lhs,
                               const Constant< ZNumber >& rhs) {
  using ConstantT = Constant< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return ConstantT::bottom();
  } else if (rhs.is_zero()) {
    return ConstantT::bottom();
  } else if (lhs.is_zero()) {
    return ConstantT(0);
  } else if (lhs.is_top() || rhs.is_top()) {
    return ConstantT::top();
  } else {
    return ConstantT(mod(lhs._n, rhs._n));
  }
}

/// \brief Left binary shift of constants
inline Constant< ZNumber > operator<<(const Constant< ZNumber >& lhs,
                                      const Constant< ZNumber >& rhs) {
  using ConstantT = Constant< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return ConstantT::bottom();
  } else if (rhs.is_number() && rhs._n < 0) {
    return ConstantT::bottom();
  } else if (lhs.is_zero()) {
    return ConstantT(0);
  } else if (lhs.is_top() || rhs.is_top()) {
    return ConstantT::top();
  } else {
    return ConstantT(lhs._n << rhs._n);
  }
}

/// \brief Right binary shift of constants
inline Constant< ZNumber > operator>>(const Constant< ZNumber >& lhs,
                                      const Constant< ZNumber >& rhs) {
  using ConstantT = Constant< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return ConstantT::bottom();
  } else if (rhs.is_number() && rhs._n < 0) {
    return ConstantT::bottom();
  } else if (lhs.is_zero()) {
    return ConstantT(0);
  } else if (lhs.is_top() || rhs.is_top()) {
    return ConstantT::top();
  } else {
    return ConstantT(lhs._n >> rhs._n);
  }
}

/// \brief Bitwise AND of constants
inline Constant< ZNumber > operator&(const Constant< ZNumber >& lhs,
                                     const Constant< ZNumber >& rhs) {
  using ConstantT = Constant< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return ConstantT::bottom();
  } else if (lhs.is_zero() || rhs.is_zero()) {
    return ConstantT(0);
  } else if (lhs.is_top() || rhs.is_top()) {
    return ConstantT::top();
  } else {
    return ConstantT(lhs._n & rhs._n);
  }
}

/// \brief Bitwise OR of constants
inline Constant< ZNumber > operator|(const Constant< ZNumber >& lhs,
                                     const Constant< ZNumber >& rhs) {
  using ConstantT = Constant< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return ConstantT::bottom();
  } else if (lhs == ConstantT(-1) || rhs == ConstantT(-1)) {
    return ConstantT(-1);
  } else if (lhs.is_top() || rhs.is_top()) {
    return ConstantT::top();
  } else {
    return ConstantT(lhs._n | rhs._n);
  }
}

/// \brief Bitwise XOR of constants
inline Constant< ZNumber > operator^(const Constant< ZNumber >& lhs,
                                     const Constant< ZNumber >& rhs) {
  using ConstantT = Constant< ZNumber >;

  if (lhs.is_bottom() || rhs.is_bottom()) {
    return ConstantT::bottom();
  } else if (lhs.is_top() || rhs.is_top()) {
    return ConstantT::top();
  } else {
    return ConstantT(lhs._n ^ rhs._n);
  }
}

/// \brief Write a constant on a stream
template < typename Number >
inline std::ostream& operator<<(std::ostream& o,
                                const Constant< Number >& cst) {
  cst.dump(o);
  return o;
}

/// \brief Constant on unlimited precision integers
using ZConstant = Constant< ZNumber >;

/// \brief Constant on unlimited precision rationals
using QConstant = Constant< QNumber >;

} // end namespace numeric
} // end namespace core
} // end namespace ikos
