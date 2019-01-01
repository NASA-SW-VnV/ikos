/*******************************************************************************
 *
 * \file
 * \brief Polymorphic machine integer abstract domain
 *
 * The PolymorphicDomain is a machine integer abstract domain whose behavior
 * depends on the abstract domain it is constructed with. It allows the use of
 * different abstract domains at runtime.
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

#include <memory>

#include <ikos/core/domain/machine_int/abstract_domain.hpp>
#include <ikos/core/support/assert.hpp>
#include <ikos/core/support/mpl.hpp>

namespace ikos {
namespace core {
namespace machine_int {

/// \brief Polymorphic machine integer abstract domain
///
/// The PolymorphicDomain is a machine integer abstract domain whose behavior
/// depends on the abstract domain it is constructed with. It allows the use of
/// different abstract domains at runtime.
template < typename VariableRef >
class PolymorphicDomain final
    : public machine_int::AbstractDomain< VariableRef,
                                          PolymorphicDomain< VariableRef > > {
public:
  using LinearExpressionT = LinearExpression< MachineInt, VariableRef >;
  using VariableTrait = machine_int::VariableTraits< VariableRef >;

private:
  /// Type erasure idiom
  ///
  /// This is commonly known as the 'type erasure idiom', used to implement
  /// std::any.

  class PolymorphicBase {
  public:
    /// \brief Default constructor
    PolymorphicBase() = default;

    /// \brief Copy constructor
    PolymorphicBase(const PolymorphicBase&) = delete;

    /// \brief Move constructor
    PolymorphicBase(PolymorphicBase&&) = delete;

    /// \brief Copy assignment operator
    PolymorphicBase& operator=(const PolymorphicBase&) = delete;

    /// \brief Move assignment operator
    PolymorphicBase& operator=(PolymorphicBase&&) = delete;

    /// \brief Destructor
    virtual ~PolymorphicBase() = default;

    /// \brief Clone the abstract value
    virtual std::unique_ptr< PolymorphicBase > clone() const = 0;

    /// \brief Check if the abstract value is bottom
    virtual bool is_bottom() const = 0;

    /// \brief Check if the abstract value is top
    virtual bool is_top() const = 0;

    /// \brief Set the abstract value to bottom
    virtual void set_to_bottom() = 0;

    /// \brief Set the abstract value to top
    virtual void set_to_top() = 0;

    /// \brief Partial order comparison
    virtual bool leq(const PolymorphicBase& other) const = 0;

    /// \brief Equality comparison
    virtual bool equals(const PolymorphicBase& other) const = 0;

    /// \brief Perform the union of two abstract values
    virtual void join_with(const PolymorphicBase& other) = 0;

    /// \brief Perform a union on a loop head
    virtual void join_loop_with(const PolymorphicBase& other) = 0;

    /// \brief Perform a union on two consecutive iterations of a fix-point
    /// algorithm
    virtual void join_iter_with(const PolymorphicBase& other) = 0;

    /// \brief Perform the widening of two abstract values
    virtual void widen_with(const PolymorphicBase& other) = 0;

    /// \brief Perform the widening of two abstract values with a threshold
    virtual void widen_threshold_with(const PolymorphicBase& other,
                                      const MachineInt& threshold) = 0;

    /// \brief Perform the intersection of two abstract values
    virtual void meet_with(const PolymorphicBase& other) = 0;

    /// \brief Perform the narrowing of two abstract values
    virtual void narrow_with(const PolymorphicBase& other) = 0;

    /// \brief Assign `x = n`
    virtual void assign(VariableRef x, const MachineInt& n) = 0;

    /// \brief Assign `x = y`
    virtual void assign(VariableRef x, VariableRef y) = 0;

    /// \brief Assign `x = e`
    virtual void assign(VariableRef x, const LinearExpressionT& e) = 0;

    /// \brief Apply `x = op y`
    virtual void apply(UnaryOperator op, VariableRef x, VariableRef y) = 0;

    /// \brief Apply `x = y op z`
    virtual void apply(BinaryOperator op,
                       VariableRef x,
                       VariableRef y,
                       VariableRef z) = 0;

    /// \brief Apply `x = y op z`
    virtual void apply(BinaryOperator op,
                       VariableRef x,
                       VariableRef y,
                       const MachineInt& z) = 0;

    /// \brief Apply `x = y op z`
    virtual void apply(BinaryOperator op,
                       VariableRef x,
                       const MachineInt& y,
                       VariableRef z) = 0;

    // \brief Add the constraint `x pred y`
    virtual void add(Predicate pred, VariableRef x, VariableRef y) = 0;

    // \brief Add the constraint `x pred y`
    virtual void add(Predicate pred, VariableRef x, const MachineInt& y) = 0;

    // \brief Add the constraint `x pred y`
    virtual void add(Predicate pred, const MachineInt& x, VariableRef y) = 0;

    /// \brief Set the interval value of a variable
    virtual void set(VariableRef x, const Interval& value) = 0;

    /// \brief Set the congruence value of a variable
    virtual void set(VariableRef x, const Congruence& value) = 0;

    /// \brief Set the interval-congruence value of a variable
    virtual void set(VariableRef x, const IntervalCongruence& value) = 0;

    /// \brief Refine the value of a variable with an interval
    virtual void refine(VariableRef x, const Interval& value) = 0;

    /// \brief Refine the value of a variable with a congruence
    virtual void refine(VariableRef x, const Congruence& value) = 0;

    /// \brief Refine the value of a variable with an interval-congruence
    virtual void refine(VariableRef x, const IntervalCongruence& value) = 0;

    /// \brief Forget a variable
    virtual void forget(VariableRef x) = 0;

    /// \brief Normalize the abstract value
    virtual void normalize() const = 0;

    /// \brief Projection to an interval
    virtual Interval to_interval(VariableRef x) const = 0;

    /// \brief Projection to an interval
    virtual Interval to_interval(const LinearExpressionT& e) const = 0;

    /// \brief Projection to a congruence
    virtual Congruence to_congruence(VariableRef x) const = 0;

    /// \brief Projection to a congruence
    virtual Congruence to_congruence(const LinearExpressionT& e) const = 0;

    /// \brief Projection to an interval-congruence
    virtual IntervalCongruence to_interval_congruence(VariableRef x) const = 0;

    /// \brief Projection to an interval-congruence
    virtual IntervalCongruence to_interval_congruence(
        const LinearExpressionT& e) const = 0;

    /// \brief Mark the variable `x` as a non-negative loop counter
    virtual void mark_counter(VariableRef x) = 0;

    /// \brief Mark the variable `x` as a normal variable, without losing
    /// information
    virtual void unmark_counter(VariableRef x) = 0;

    /// \brief Initialize a non-negative loop counter: `x = c`
    virtual void init_counter(VariableRef x, const MachineInt& c) = 0;

    /// \brief Increment a non-negative loop counter counter: `x += k`
    virtual void incr_counter(VariableRef x, const MachineInt& k) = 0;

    /// \brief Forget a non-negative loop counter
    virtual void forget_counter(VariableRef x) = 0;

    /// \brief Dump the abstract value, for debugging purpose
    virtual void dump(std::ostream&) const = 0;

  }; // end class PolymorphicBase

private:
  template < typename RuntimeDomain >
  class PolymorphicDerived final : public PolymorphicBase {
  public:
    static_assert(
        machine_int::IsAbstractDomain< RuntimeDomain, VariableRef >::value,
        "RuntimeDomain must implement machine_int::AbstractDomain");

  private:
    using PolymorphicDerivedT = PolymorphicDerived< RuntimeDomain >;

  private:
    RuntimeDomain _inv;

  public:
    /// \brief Create an abstract value
    explicit PolymorphicDerived(RuntimeDomain inv) : _inv(std::move(inv)) {}

    /// \brief Destructor
    ~PolymorphicDerived() override = default;

    /// \brief Clone the abstract value
    std::unique_ptr< PolymorphicBase > clone() const override {
      return std::make_unique< PolymorphicDerivedT >(this->_inv);
    }

    /// \brief Check if the abstract value is bottom
    bool is_bottom() const override { return this->_inv.is_bottom(); }

    /// \brief Check if the abstract value is top
    bool is_top() const override { return this->_inv.is_top(); }

    /// \brief Set the abstract value to bottom
    void set_to_bottom() override { this->_inv.set_to_bottom(); }

    /// \brief Set the abstract value to top
    void set_to_top() override { this->_inv.set_to_top(); }

    /// \brief Check if the parameter has the same runtime domain
    bool is_compatible(const PolymorphicBase& other) const {
      return dynamic_cast< const PolymorphicDerivedT* >(&other) != nullptr;
    }

    /// \brief Assert that the parameter has the same runtime domain
    void assert_compatible(const PolymorphicBase& other) const {
      ikos_assert_msg(this->is_compatible(other),
                      "incompatible runtime abstract domains");
      ikos_ignore(other);
    }

    /// \brief Partial order comparison
    bool leq(const PolymorphicBase& other) const override {
      this->assert_compatible(other);
      return this->_inv.leq(
          static_cast< const PolymorphicDerivedT& >(other)._inv);
    }

    /// \brief Equality comparison
    bool equals(const PolymorphicBase& other) const override {
      this->assert_compatible(other);
      return this->_inv.equals(
          static_cast< const PolymorphicDerivedT& >(other)._inv);
    }

    /// \brief Perform the union of two abstract values
    void join_with(const PolymorphicBase& other) override {
      this->assert_compatible(other);
      this->_inv.join_with(
          static_cast< const PolymorphicDerivedT& >(other)._inv);
    }

    /// \brief Perform a union on a loop head
    void join_loop_with(const PolymorphicBase& other) override {
      this->assert_compatible(other);
      this->_inv.join_loop_with(
          static_cast< const PolymorphicDerivedT& >(other)._inv);
    }

    /// \brief Perform a union on two consecutive iterations of a fix-point
    /// algorithm
    void join_iter_with(const PolymorphicBase& other) override {
      this->assert_compatible(other);
      this->_inv.join_iter_with(
          static_cast< const PolymorphicDerivedT& >(other)._inv);
    }

    /// \brief Perform the widening of two abstract values
    void widen_with(const PolymorphicBase& other) override {
      this->assert_compatible(other);
      this->_inv.widen_with(
          static_cast< const PolymorphicDerivedT& >(other)._inv);
    }

    /// \brief Perform the widening of two abstract values with a threshold
    void widen_threshold_with(const PolymorphicBase& other,
                              const MachineInt& threshold) override {
      this->assert_compatible(other);
      this->_inv.widen_threshold_with(static_cast< const PolymorphicDerivedT& >(
                                          other)
                                          ._inv,
                                      threshold);
    }

    /// \brief Perform the intersection of two abstract values
    void meet_with(const PolymorphicBase& other) override {
      this->assert_compatible(other);
      this->_inv.meet_with(
          static_cast< const PolymorphicDerivedT& >(other)._inv);
    }

    /// \brief Perform the narrowing of two abstract values
    void narrow_with(const PolymorphicBase& other) override {
      this->assert_compatible(other);
      this->_inv.narrow_with(
          static_cast< const PolymorphicDerivedT& >(other)._inv);
    }

    /// \brief Assign `x = n`
    void assign(VariableRef x, const MachineInt& n) override {
      this->_inv.assign(x, n);
    }

    /// \brief Assign `x = y`
    void assign(VariableRef x, VariableRef y) override {
      this->_inv.assign(x, y);
    }

    /// \brief Assign `x = e`
    void assign(VariableRef x, const LinearExpressionT& e) override {
      this->_inv.assign(x, e);
    }

    /// \brief Apply `x = op y`
    void apply(UnaryOperator op, VariableRef x, VariableRef y) override {
      this->_inv.apply(op, x, y);
    }

    /// \brief Apply `x = y op z`
    void apply(BinaryOperator op,
               VariableRef x,
               VariableRef y,
               VariableRef z) override {
      this->_inv.apply(op, x, y, z);
    }

    /// \brief Apply `x = y op z`
    void apply(BinaryOperator op,
               VariableRef x,
               VariableRef y,
               const MachineInt& z) override {
      this->_inv.apply(op, x, y, z);
    }

    /// \brief Apply `x = y op z`
    void apply(BinaryOperator op,
               VariableRef x,
               const MachineInt& y,
               VariableRef z) override {
      this->_inv.apply(op, x, y, z);
    }

    // \brief Add the constraint `x pred y`
    void add(Predicate pred, VariableRef x, VariableRef y) override {
      this->_inv.add(pred, x, y);
    }

    // \brief Add the constraint `x pred y`
    void add(Predicate pred, VariableRef x, const MachineInt& y) override {
      this->_inv.add(pred, x, y);
    }

    // \brief Add the constraint `x pred y`
    void add(Predicate pred, const MachineInt& x, VariableRef y) override {
      this->_inv.add(pred, x, y);
    }

    /// \brief Set the interval value of a variable
    void set(VariableRef x, const Interval& value) override {
      this->_inv.set(x, value);
    }

    /// \brief Set the congruence value of a variable
    void set(VariableRef x, const Congruence& value) override {
      this->_inv.set(x, value);
    }

    /// \brief Set the interval-congruence value of a variable
    void set(VariableRef x, const IntervalCongruence& value) override {
      this->_inv.set(x, value);
    }

    /// \brief Refine the value of a variable with an interval
    void refine(VariableRef x, const Interval& value) override {
      this->_inv.refine(x, value);
    }

    /// \brief Refine the value of a variable with a congruence
    void refine(VariableRef x, const Congruence& value) override {
      this->_inv.refine(x, value);
    }

    /// \brief Refine the value of a variable with an interval-congruence
    void refine(VariableRef x, const IntervalCongruence& value) override {
      this->_inv.refine(x, value);
    }

    /// \brief Forget a variable
    void forget(VariableRef x) override { this->_inv.forget(x); }

    /// \brief Normalize the abstract value
    void normalize() const override { this->_inv.normalize(); }

    /// \brief Projection to an interval
    Interval to_interval(VariableRef x) const override {
      return this->_inv.to_interval(x);
    }

    /// \brief Projection to an interval
    Interval to_interval(const LinearExpressionT& e) const override {
      return this->_inv.to_interval(e);
    }

    /// \brief Projection to a congruence
    Congruence to_congruence(VariableRef x) const override {
      return this->_inv.to_congruence(x);
    }

    /// \brief Projection to a congruence
    Congruence to_congruence(const LinearExpressionT& e) const override {
      return this->_inv.to_congruence(e);
    }

    /// \brief Projection to an interval-congruence
    IntervalCongruence to_interval_congruence(VariableRef x) const override {
      return this->_inv.to_interval_congruence(x);
    }

    /// \brief Projection to an interval-congruence
    IntervalCongruence to_interval_congruence(
        const LinearExpressionT& e) const override {
      return this->_inv.to_interval_congruence(e);
    }

    /// \brief Mark the variable `x` as a non-negative loop counter
    void mark_counter(VariableRef x) override { this->_inv.mark_counter(x); }

    /// \brief Mark the variable `x` as a normal variable, without losing
    /// information
    void unmark_counter(VariableRef x) override {
      this->_inv.unmark_counter(x);
    }

    /// \brief Initialize a non-negative loop counter: `x = c`
    void init_counter(VariableRef x, const MachineInt& c) override {
      this->_inv.init_counter(x, c);
    }

    /// \brief Increment a non-negative loop counter counter: `x += k`
    void incr_counter(VariableRef x, const MachineInt& k) override {
      this->_inv.incr_counter(x, k);
    }

    /// \brief Forget a non-negative loop counter
    void forget_counter(VariableRef x) override {
      this->_inv.forget_counter(x);
    }

    /// \brief Dump the abstract value, for debugging purpose
    void dump(std::ostream& o) const override { this->_inv.dump(o); }

  }; // end class PolymorphicDerived

private:
  /// \brief Pointer on the polymorphic base class, or nullptr (for bottom)
  std::unique_ptr< PolymorphicBase > _ptr;

private:
  struct BottomTag {};

  /// \brief Create the bottom abstract value
  explicit PolymorphicDomain(BottomTag) : _ptr(nullptr) {}

public:
  /// \brief Create the top abstract value
  PolymorphicDomain() {
    ikos_unreachable("cannot create a top abstract value of PolymorphicDomain");
  }

  /// \brief Create a polymorphic domain with the given abstract value
  template < typename RuntimeDomain >
  explicit PolymorphicDomain(RuntimeDomain&& inv)
      : _ptr(std::make_unique<
             PolymorphicDerived< remove_cvref_t< RuntimeDomain > > >(
            std::forward< RuntimeDomain >(inv))) {}

  /// \brief Copy constructor
  PolymorphicDomain(const PolymorphicDomain& other)
      : _ptr((other._ptr != nullptr) ? other._ptr->clone() : nullptr) {}

  /// \brief Move constructor
  PolymorphicDomain(PolymorphicDomain&&) noexcept = default;

  /// \brief Copy assignment operator
  PolymorphicDomain& operator=(const PolymorphicDomain& other) {
    if (other._ptr != nullptr) {
      this->_ptr = other._ptr->clone();
    } else {
      this->_ptr = nullptr;
    }
    return *this;
  }

  /// \brief Move assignment operator
  PolymorphicDomain& operator=(PolymorphicDomain&&) noexcept = default;

  /// \brief Destructor
  ~PolymorphicDomain() override = default;

  /// \brief Create the top abstract value
  static PolymorphicDomain top() {
    ikos_unreachable("cannot create a top abstract value of PolymorphicDomain");
  }

  /// \brief Create the bottom abstract value
  static PolymorphicDomain bottom() { return PolymorphicDomain(BottomTag{}); }

  bool is_bottom() const override {
    if (this->_ptr != nullptr) {
      return this->_ptr->is_bottom();
    } else {
      return true;
    }
  }

  bool is_top() const override {
    if (this->_ptr != nullptr) {
      return this->_ptr->is_top();
    } else {
      return false;
    }
  }

  void set_to_bottom() override {
    if (this->_ptr != nullptr) {
      this->_ptr->set_to_bottom();
    } else {
      // no-op
    }
  }

  void set_to_top() override {
    if (this->_ptr != nullptr) {
      this->_ptr->set_to_top();
    } else {
      ikos_unreachable("cannot set bottom to top for PolymorphicDomain");
    }
  }

  bool leq(const PolymorphicDomain& other) const override {
    if (this->_ptr == nullptr) {
      return true;
    } else if (other._ptr == nullptr) {
      return this->_ptr->is_bottom();
    } else {
      return this->_ptr->leq(*other._ptr);
    }
  }

  bool equals(const PolymorphicDomain& other) const override {
    if (this->_ptr == nullptr) {
      return other.is_bottom();
    } else if (other._ptr == nullptr) {
      return this->is_bottom();
    } else {
      return this->_ptr->equals(*other._ptr);
    }
  }

  void join_with(const PolymorphicDomain& other) override {
    if (other._ptr == nullptr) {
      return;
    } else if (this->_ptr == nullptr) {
      this->operator=(other);
    } else {
      this->_ptr->join_with(*other._ptr);
    }
  }

  void join_loop_with(const PolymorphicDomain& other) override {
    if (other._ptr == nullptr) {
      return;
    } else if (this->_ptr == nullptr) {
      this->operator=(other);
    } else {
      this->_ptr->join_loop_with(*other._ptr);
    }
  }

  void join_iter_with(const PolymorphicDomain& other) override {
    if (other._ptr == nullptr) {
      return;
    } else if (this->_ptr == nullptr) {
      this->operator=(other);
    } else {
      this->_ptr->join_iter_with(*other._ptr);
    }
  }

  void widen_with(const PolymorphicDomain& other) override {
    if (other._ptr == nullptr) {
      return;
    } else if (this->_ptr == nullptr) {
      this->operator=(other);
    } else {
      this->_ptr->widen_with(*other._ptr);
    }
  }

  void widen_threshold_with(const PolymorphicDomain& other,
                            const MachineInt& threshold) override {
    if (other._ptr == nullptr) {
      return;
    } else if (this->_ptr == nullptr) {
      this->operator=(other);
    } else {
      this->_ptr->widen_threshold_with(*other._ptr, threshold);
    }
  }

  void meet_with(const PolymorphicDomain& other) override {
    if (this->_ptr == nullptr) {
      return;
    } else if (other._ptr == nullptr) {
      this->_ptr->set_to_bottom();
    } else {
      this->_ptr->meet_with(*other._ptr);
    }
  }

  void narrow_with(const PolymorphicDomain& other) override {
    if (this->_ptr == nullptr) {
      return;
    } else if (other._ptr == nullptr) {
      this->_ptr->set_to_bottom();
    } else {
      this->_ptr->narrow_with(*other._ptr);
    }
  }

  /// \name Machine integer abstract domain methods
  /// @{

  void assign(VariableRef x, const MachineInt& n) override {
    if (this->_ptr != nullptr) {
      this->_ptr->assign(x, n);
    }
  }

  void assign(VariableRef x, VariableRef y) override {
    if (this->_ptr != nullptr) {
      this->_ptr->assign(x, y);
    }
  }

  void assign(VariableRef x, const LinearExpressionT& e) override {
    if (this->_ptr != nullptr) {
      this->_ptr->assign(x, e);
    }
  }

  void apply(UnaryOperator op, VariableRef x, VariableRef y) override {
    if (this->_ptr != nullptr) {
      this->_ptr->apply(op, x, y);
    }
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             VariableRef z) override {
    if (this->_ptr != nullptr) {
      this->_ptr->apply(op, x, y, z);
    }
  }

  void apply(BinaryOperator op,
             VariableRef x,
             VariableRef y,
             const MachineInt& z) override {
    if (this->_ptr != nullptr) {
      this->_ptr->apply(op, x, y, z);
    }
  }

  void apply(BinaryOperator op,
             VariableRef x,
             const MachineInt& y,
             VariableRef z) override {
    if (this->_ptr != nullptr) {
      this->_ptr->apply(op, x, y, z);
    }
  }

  void add(Predicate pred, VariableRef x, VariableRef y) override {
    if (this->_ptr != nullptr) {
      this->_ptr->add(pred, x, y);
    }
  }

  void add(Predicate pred, VariableRef x, const MachineInt& y) override {
    if (this->_ptr != nullptr) {
      this->_ptr->add(pred, x, y);
    }
  }

  void add(Predicate pred, const MachineInt& x, VariableRef y) override {
    if (this->_ptr != nullptr) {
      this->_ptr->add(pred, x, y);
    }
  }

  void set(VariableRef x, const Interval& value) override {
    if (this->_ptr != nullptr) {
      this->_ptr->set(x, value);
    }
  }

  void set(VariableRef x, const Congruence& value) override {
    if (this->_ptr != nullptr) {
      this->_ptr->set(x, value);
    }
  }

  void set(VariableRef x, const IntervalCongruence& value) override {
    if (this->_ptr != nullptr) {
      this->_ptr->set(x, value);
    }
  }

  void refine(VariableRef x, const Interval& value) override {
    if (this->_ptr != nullptr) {
      this->_ptr->refine(x, value);
    }
  }

  void refine(VariableRef x, const Congruence& value) override {
    if (this->_ptr != nullptr) {
      this->_ptr->refine(x, value);
    }
  }

  void refine(VariableRef x, const IntervalCongruence& value) override {
    if (this->_ptr != nullptr) {
      this->_ptr->refine(x, value);
    }
  }

  void forget(VariableRef x) override {
    if (this->_ptr != nullptr) {
      this->_ptr->forget(x);
    }
  }

  void normalize() const override {
    if (this->_ptr != nullptr) {
      this->_ptr->normalize();
    }
  }

  Interval to_interval(VariableRef x) const override {
    if (this->_ptr != nullptr) {
      return this->_ptr->to_interval(x);
    } else {
      return Interval::bottom(VariableTrait::bit_width(x),
                              VariableTrait::sign(x));
    }
  }

  Interval to_interval(const LinearExpressionT& e) const override {
    if (this->_ptr != nullptr) {
      return this->_ptr->to_interval(e);
    } else {
      return Interval::bottom(e.constant().bit_width(), e.constant().sign());
    }
  }

  Congruence to_congruence(VariableRef x) const override {
    if (this->_ptr != nullptr) {
      return this->_ptr->to_congruence(x);
    } else {
      return Congruence::bottom(VariableTrait::bit_width(x),
                                VariableTrait::sign(x));
    }
  }

  Congruence to_congruence(const LinearExpressionT& e) const override {
    if (this->_ptr != nullptr) {
      return this->_ptr->to_congruence(e);
    } else {
      return Congruence::bottom(e.constant().bit_width(), e.constant().sign());
    }
  }

  IntervalCongruence to_interval_congruence(VariableRef x) const override {
    if (this->_ptr != nullptr) {
      return this->_ptr->to_interval_congruence(x);
    } else {
      return IntervalCongruence::bottom(VariableTrait::bit_width(x),
                                        VariableTrait::sign(x));
    }
  }

  IntervalCongruence to_interval_congruence(
      const LinearExpressionT& e) const override {
    if (this->_ptr != nullptr) {
      return this->_ptr->to_interval_congruence(e);
    } else {
      return IntervalCongruence::bottom(e.constant().bit_width(),
                                        e.constant().sign());
    }
  }

  /// @}
  /// \name Non-negative loop counter abstract domain methods
  /// @{

  void mark_counter(VariableRef x) override {
    if (this->_ptr != nullptr) {
      this->_ptr->mark_counter(x);
    }
  }

  void unmark_counter(VariableRef x) override {
    if (this->_ptr != nullptr) {
      this->_ptr->unmark_counter(x);
    }
  }

  void init_counter(VariableRef x, const MachineInt& c) override {
    if (this->_ptr != nullptr) {
      this->_ptr->init_counter(x, c);
    }
  }

  void incr_counter(VariableRef x, const MachineInt& k) override {
    if (this->_ptr != nullptr) {
      this->_ptr->incr_counter(x, k);
    }
  }

  void forget_counter(VariableRef x) override {
    if (this->_ptr != nullptr) {
      this->_ptr->forget_counter(x);
    }
  }

  /// @}

  void dump(std::ostream& o) const override {
    if (this->_ptr != nullptr) {
      this->_ptr->dump(o);
    } else {
      o << "⊥";
    }
  }

  static std::string name() { return "polymorphic domain"; }

}; // end class PolymorphicDomain

} // end namespace machine_int
} // end namespace core
} // end namespace ikos
