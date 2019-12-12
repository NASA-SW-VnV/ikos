/*******************************************************************************
 *
 * \file
 * \brief Products of abstract domains
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

#include <sstream>
#include <type_traits>

#include <ikos/core/domain/abstract_domain.hpp>

namespace ikos {
namespace core {

/// \brief Domain product of 2 abstract domains
template < typename Domain1, typename Domain2 >
class DomainProduct2 final
    : public AbstractDomain< DomainProduct2< Domain1, Domain2 > > {
public:
  static_assert(IsAbstractDomain< Domain1 >::value,
                "Domain1 must implement AbstractDomain");
  static_assert(IsAbstractDomain< Domain2 >::value,
                "Domain2 must implement AbstractDomain");

private:
  Domain1 _first;
  Domain2 _second;

public:
  /// \brief Create the abstract value with the given abstract values
  DomainProduct2(Domain1 first, Domain2 second)
      : _first(std::move(first)), _second(std::move(second)) {
    this->normalize();
  }

  /// \brief Copy constructor
  DomainProduct2(const DomainProduct2&) noexcept(
      (std::is_nothrow_copy_constructible< Domain1 >::value) &&
      (std::is_nothrow_copy_constructible< Domain2 >::value)) = default;

  /// \brief Move constructor
  DomainProduct2(DomainProduct2&&) noexcept(
      (std::is_nothrow_move_constructible< Domain1 >::value) &&
      (std::is_nothrow_move_constructible< Domain2 >::value)) = default;

  /// \brief Copy assignment operator
  DomainProduct2& operator=(const DomainProduct2&) noexcept(
      (std::is_nothrow_copy_assignable< Domain1 >::value) &&
      (std::is_nothrow_copy_assignable< Domain2 >::value)) = default;

  /// \brief Move assignment operator
  DomainProduct2& operator=(DomainProduct2&&) noexcept(
      (std::is_nothrow_move_assignable< Domain1 >::value) &&
      (std::is_nothrow_move_assignable< Domain2 >::value)) = default;

  /// \brief Destructor
  ~DomainProduct2() override = default;

  /// \brief Return the first abstract value
  ///
  /// Note: does not normalize.
  const Domain1& first() const { return this->_first; }

  /// \brief Return the first abstract value
  ///
  /// Note: does not normalize.
  Domain1& first() { return this->_first; }

  /// \brief Return the second abstract value
  ///
  /// Note: does not normalize.
  const Domain2& second() const { return this->_second; }

  /// \brief Return the second abstract value
  ///
  /// Note: does not normalize.
  Domain2& second() { return this->_second; }

  void normalize() override {
    this->_first.normalize();
    if (this->_first.is_bottom()) {
      this->_second.set_to_bottom();
      return;
    }

    this->_second.normalize();
    if (this->_second.is_bottom()) {
      this->_first.set_to_bottom();
      return;
    }
  }

  bool is_bottom() const override {
    return this->_first.is_bottom() || this->_second.is_bottom();
  }

  bool is_top() const override {
    return this->_first.is_top() && this->_second.is_top();
  }

  void set_to_bottom() override {
    this->_first.set_to_bottom();
    this->_second.set_to_bottom();
  }

  void set_to_top() override {
    this->_first.set_to_top();
    this->_second.set_to_top();
  }

  bool leq(const DomainProduct2& other) const override {
    if (this->is_bottom()) {
      return true;
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_first.leq(other._first) && this->_second.leq(other._second);
    }
  }

  bool equals(const DomainProduct2& other) const override {
    if (this->is_bottom()) {
      return other.is_bottom();
    } else if (other.is_bottom()) {
      return false;
    } else {
      return this->_first.equals(other._first) &&
             this->_second.equals(other._second);
    }
  }

  void join_with(DomainProduct2&& other) override {
    this->normalize();
    other.normalize();
    if (this->is_bottom()) {
      this->operator=(std::move(other));
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_first.join_with(std::move(other._first));
      this->_second.join_with(std::move(other._second));
    }
  }

  void join_with(const DomainProduct2& other) override {
    this->normalize();
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_first.join_with(other._first);
      this->_second.join_with(other._second);
    }
  }

  void join_loop_with(DomainProduct2&& other) override {
    this->normalize();
    other.normalize();
    if (this->is_bottom()) {
      this->operator=(std::move(other));
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_first.join_loop_with(std::move(other._first));
      this->_second.join_loop_with(std::move(other._second));
    }
  }

  void join_loop_with(const DomainProduct2& other) override {
    this->normalize();
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_first.join_loop_with(other._first);
      this->_second.join_loop_with(other._second);
    }
  }

  void join_iter_with(DomainProduct2&& other) override {
    this->normalize();
    other.normalize();
    if (this->is_bottom()) {
      this->operator=(std::move(other));
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_first.join_iter_with(std::move(other._first));
      this->_second.join_iter_with(std::move(other._second));
    }
  }

  void join_iter_with(const DomainProduct2& other) override {
    this->normalize();
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_first.join_iter_with(other._first);
      this->_second.join_iter_with(other._second);
    }
  }

  void widen_with(const DomainProduct2& other) override {
    this->normalize();
    if (this->is_bottom()) {
      this->operator=(other);
    } else if (other.is_bottom()) {
      return;
    } else {
      this->_first.widen_with(other._first);
      this->_second.widen_with(other._second);
    }
  }

  void meet_with(const DomainProduct2& other) override {
    this->normalize();
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_first.meet_with(other._first);
      this->_second.meet_with(other._second);
    }
  }

  void narrow_with(const DomainProduct2& other) override {
    this->normalize();
    if (this->is_bottom()) {
      return;
    } else if (other.is_bottom()) {
      this->set_to_bottom();
    } else {
      this->_first.narrow_with(other._first);
      this->_second.narrow_with(other._second);
    }
  }

  DomainProduct2 join(const DomainProduct2& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return DomainProduct2(this->_first.join(other._first),
                            this->_second.join(other._second));
    }
  }

  DomainProduct2 join_loop(const DomainProduct2& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return DomainProduct2(this->_first.join_loop(other._first),
                            this->_second.join_loop(other._second));
    }
  }

  DomainProduct2 join_iter(const DomainProduct2& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return DomainProduct2(this->_first.join_iter(other._first),
                            this->_second.join_iter(other._second));
    }
  }

  DomainProduct2 widening(const DomainProduct2& other) const override {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else {
      return DomainProduct2(this->_first.widening(other._first),
                            this->_second.widening(other._second));
    }
  }

  DomainProduct2 meet(const DomainProduct2& other) const override {
    if (this->is_bottom()) {
      return *this;
    } else if (other.is_bottom()) {
      return other;
    } else {
      return DomainProduct2(this->_first.meet(other._first),
                            this->_second.meet(other._second));
    }
  }

  DomainProduct2 narrowing(const DomainProduct2& other) const override {
    if (this->is_bottom()) {
      return *this;
    } else if (other.is_bottom()) {
      return other;
    } else {
      return DomainProduct2(this->_first.narrowing(other._first),
                            this->_second.narrowing(other._second));
    }
  }

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else {
      o << "(";
      this->_first.dump(o);
      o << ", ";
      this->_second.dump(o);
      o << ")";
    }
  }

  static std::string name() {
    return "product " + Domain1::name() + " x " + Domain2::name();
  }

}; // end class DomainProduct2

/// \brief Domain product of 3 abstract domains
template < typename Domain1, typename Domain2, typename Domain3 >
class DomainProduct3 final
    : public AbstractDomain< DomainProduct3< Domain1, Domain2, Domain3 > > {
public:
  static_assert(IsAbstractDomain< Domain1 >::value,
                "Domain1 must implement AbstractDomain");
  static_assert(IsAbstractDomain< Domain2 >::value,
                "Domain2 must implement AbstractDomain");
  static_assert(IsAbstractDomain< Domain3 >::value,
                "Domain3 must implement AbstractDomain");

private:
  using Product12 = DomainProduct2< Domain1, Domain2 >;
  using Product123 = DomainProduct2< Product12, Domain3 >;

private:
  Product123 _product;

private:
  /// \brief Private constructor
  explicit DomainProduct3(Product123 product) : _product(std::move(product)) {}

public:
  /// \brief Create the abstract value with the given abstract values
  DomainProduct3(Domain1 first, Domain2 second, Domain3 third)
      : _product(Product12(std::move(first), std::move(second)),
                 std::move(third)) {}

  /// \brief Copy constructor
  DomainProduct3(const DomainProduct3&) noexcept(
      (std::is_nothrow_copy_constructible< Domain1 >::value) &&
      (std::is_nothrow_copy_constructible< Domain2 >::value) &&
      (std::is_nothrow_copy_constructible< Domain3 >::value)) = default;

  /// \brief Move constructor
  DomainProduct3(DomainProduct3&&) noexcept(
      (std::is_nothrow_move_constructible< Domain1 >::value) &&
      (std::is_nothrow_move_constructible< Domain2 >::value) &&
      (std::is_nothrow_move_constructible< Domain3 >::value)) = default;

  /// \brief Copy assignment operator
  DomainProduct3& operator=(const DomainProduct3&) noexcept(
      (std::is_nothrow_copy_assignable< Domain1 >::value) &&
      (std::is_nothrow_copy_assignable< Domain2 >::value) &&
      (std::is_nothrow_copy_assignable< Domain3 >::value)) = default;

  /// \brief Move assignment operator
  DomainProduct3& operator=(DomainProduct3&&) noexcept(
      (std::is_nothrow_move_assignable< Domain1 >::value) &&
      (std::is_nothrow_move_assignable< Domain2 >::value) &&
      (std::is_nothrow_move_assignable< Domain3 >::value)) = default;

  /// \brief Destructor
  ~DomainProduct3() override = default;

  /// \brief Return the first abstract value
  ///
  /// Note: does not normalize.
  const Domain1& first() const { return this->_product.first().first(); }

  /// \brief Return the first abstract value
  ///
  /// Note: does not normalize.
  Domain1& first() { return this->_product.first().first(); }

  /// \brief Return the second abstract value
  ///
  /// Note: does not normalize.
  const Domain2& second() const { return this->_product.first().second(); }

  /// \brief Return the second abstract value
  ///
  /// Note: does not normalize.
  Domain2& second() { return this->_product.first().second(); }

  /// \brief Return the third abstract value
  ///
  /// Note: does not normalize.
  const Domain3& third() const { return this->_product.second(); }

  /// \brief Return the third abstract value
  ///
  /// Note: does not normalize.
  Domain3& third() { return this->_product.second(); }

  void normalize() override { this->_product.normalize(); }

  bool is_bottom() const override { return this->_product.is_bottom(); }

  bool is_top() const override { return this->_product.is_top(); }

  void set_to_bottom() override { this->_product.set_to_bottom(); }

  void set_to_top() override { this->_product.set_to_top(); }

  bool leq(const DomainProduct3& other) const override {
    return this->_product.leq(other._product);
  }

  bool equals(const DomainProduct3& other) const override {
    return this->_product.equals(other._product);
  }

  void join_with(DomainProduct3&& other) override {
    this->_product.join_with(std::move(other._product));
  }

  void join_with(const DomainProduct3& other) override {
    this->_product.join_with(other._product);
  }

  void join_loop_with(DomainProduct3&& other) override {
    this->_product.join_loop_with(std::move(other._product));
  }

  void join_loop_with(const DomainProduct3& other) override {
    this->_product.join_loop_with(other._product);
  }

  void join_iter_with(DomainProduct3&& other) override {
    this->_product.join_iter_with(std::move(other._product));
  }

  void join_iter_with(const DomainProduct3& other) override {
    this->_product.join_iter_with(other._product);
  }

  void widen_with(const DomainProduct3& other) override {
    this->_product.widen_with(other._product);
  }

  void meet_with(const DomainProduct3& other) override {
    this->_product.meet_with(other._product);
  }

  void narrow_with(const DomainProduct3& other) override {
    this->_product.narrow_with(other._product);
  }

  DomainProduct3 join(const DomainProduct3& other) const override {
    return DomainProduct3(this->_product.join(other._product));
  }

  DomainProduct3 join_loop(const DomainProduct3& other) const override {
    return DomainProduct3(this->_product.join_loop(other._product));
  }

  DomainProduct3 join_iter(const DomainProduct3& other) const override {
    return DomainProduct3(this->_product.join_iter(other._product));
  }

  DomainProduct3 widening(const DomainProduct3& other) const override {
    return DomainProduct3(this->_product.widening(other._product));
  }

  DomainProduct3 meet(const DomainProduct3& other) const override {
    return DomainProduct3(this->_product.meet(other._product));
  }

  DomainProduct3 narrowing(const DomainProduct3& other) const override {
    return DomainProduct3(this->_product.narrowing(other._product));
  }

  void dump(std::ostream& o) const override {
    if (this->is_bottom()) {
      o << "⊥";
    } else {
      o << "(";
      this->first().dump(o);
      o << ", ";
      this->second().dump(o);
      o << ", ";
      this->third().dump(o);
      o << ")";
    }
  }

  static std::string name() {
    return "product " + Domain1::name() + " x " + Domain2::name() + " x " +
           Domain3::name();
  }

}; // end class DomainProduct3

} // end namespace core
} // end namespace ikos
