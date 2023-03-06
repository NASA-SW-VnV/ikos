/*******************************************************************************
 *
 * \file
 * \brief Solver of pointer constraints systems
 *
 * This analysis operates on a simplified version of the constraint resolution
 * algorithm described in the following paper:
 *
 * Arnaud Venet. A Scalable Nonuniform Pointer Analysis for Embedded Programs.
 * In Proceedings of the International Static Analysis Symposium, SAS 04,
 * Verona, Italy. Lecture Notes in Computer Science, pages 149-164,
 * volume 3148, Springer 2004.
 *
 * Author: Arnaud J. Venet
 *
 * Contributors:
 *   * Jorge A. Navas
 *   * Maxime Arthaud
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

#include <iosfwd>
#include <memory>
#include <unordered_map>
#include <vector>

#include <ikos/core/semantic/dumpable.hpp>
#include <ikos/core/semantic/indexable.hpp>
#include <ikos/core/semantic/memory_location.hpp>
#include <ikos/core/semantic/variable.hpp>
#include <ikos/core/value/machine_int/interval.hpp>
#include <ikos/core/value/pointer/pointer.hpp>

namespace ikos {
namespace core {
namespace pointer {

/// \brief Base class for pointer operands
template < typename VariableRef, typename MemoryLocationRef >
class Operand {
public:
  enum OperandKind { VariableKind, AddressKind };

protected:
  // Kind of operand
  OperandKind _kind;

protected:
  /// \brief Protected constructor
  explicit Operand(OperandKind kind) : _kind(kind) {}

public:
  /// \brief No copy constructor
  Operand(const Operand&) = delete;

  /// \brief No move constructor
  Operand(Operand&&) = delete;

  /// \brief No copy assignment operator
  Operand& operator=(const Operand&) = delete;

  /// \brief No move assignment operator
  Operand& operator=(Operand&&) = delete;

  /// \brief Destructor
  virtual ~Operand() = default;

  /// \brief Get the kind of operand
  OperandKind kind() const { return this->_kind; }

  /// \brief Dump the operand for debugging purpose
  virtual void dump(std::ostream&) const = 0;

}; // end class Operand

/// \brief Represents a pointer variable plus an offset
///
/// For instance, `p + [0, 4]`
template < typename VariableRef, typename MemoryLocationRef >
class VariableOperand final : public Operand< VariableRef, MemoryLocationRef > {
private:
  using OperandT = Operand< VariableRef, MemoryLocationRef >;

private:
  // Pointer variable
  VariableRef _var;

  // Offset interval, in bytes
  machine_int::Interval _offset;

private:
  /// \brief Private constructor
  VariableOperand(VariableRef var, machine_int::Interval offset)
      : OperandT(OperandT::VariableKind),
        _var(var),
        _offset(std::move(offset)) {}

public:
  /// \brief Static constructor
  static std::unique_ptr< VariableOperand > create(
      VariableRef var, machine_int::Interval offset) {
    return std::unique_ptr< VariableOperand >(
        new VariableOperand(var, std::move(offset)));
  }

  /// \brief Return the pointer variable
  VariableRef var() const { return this->_var; }

  /// \brief Return the offset interval, in bytes
  const machine_int::Interval& offset() const { return this->_offset; }

  void dump(std::ostream& o) const override {
    DumpableTraits< VariableRef >::dump(o, this->_var);
    o << " + ";
    o << this->_offset;
  }

}; // end class VariableOperand

/// \brief Represents an address (i.e., memory location) plus an offset
///
/// For instance, `&x + [0, 4]`
template < typename VariableRef, typename MemoryLocationRef >
class AddressOperand final : public Operand< VariableRef, MemoryLocationRef > {
private:
  using OperandT = Operand< VariableRef, MemoryLocationRef >;

private:
  // Address
  MemoryLocationRef _address;

  // Offset interval, in bytes
  machine_int::Interval _offset;

private:
  /// \brief Private constructor
  AddressOperand(MemoryLocationRef address, machine_int::Interval offset)
      : OperandT(OperandT::AddressKind),
        _address(address),
        _offset(std::move(offset)) {}

public:
  /// \brief Static constructor
  static std::unique_ptr< AddressOperand > create(
      MemoryLocationRef address, machine_int::Interval offset) {
    return std::unique_ptr< AddressOperand >(
        new AddressOperand(address, std::move(offset)));
  }

  /// \brief Return the address (i.e., memory location)
  MemoryLocationRef address() const { return this->_address; }

  /// \brief Return the offset interval, in bytes
  const machine_int::Interval& offset() const { return this->_offset; }

  void dump(std::ostream& o) const override {
    DumpableTraits< MemoryLocationRef >::dump(o, this->_address);
    o << " + ";
    o << this->_offset;
  }

}; // end class AddressOperand

/// \brief Base class for pointer constraints
template < typename VariableRef, typename MemoryLocationRef >
class Constraint {
public:
  enum ConstraintKind { AssignKind, StoreKind, LoadKind };

protected:
  // Kind of operand
  ConstraintKind _kind;

protected:
  /// \brief Protected constructor
  explicit Constraint(ConstraintKind kind) : _kind(kind) {}

public:
  /// \brief No copy constructor
  Constraint(const Constraint&) = delete;

  /// \brief No move constructor
  Constraint(Constraint&&) = delete;

  /// \brief No copy assignment operator
  Constraint& operator=(const Constraint&) = delete;

  /// \brief No move assignment operator
  Constraint& operator=(Constraint&&) = delete;

  /// \brief Destructor
  virtual ~Constraint() = default;

  /// \brief Get the kind of constraint
  ConstraintKind kind() const { return this->_kind; }

  /// \brief Dump the constraint for debugging purpose
  virtual void dump(std::ostream&) const = 0;

}; // end class Constraint

/// \brief Pointer assignment constraint
///
/// Represents a constraint `p = operand`
template < typename VariableRef, typename MemoryLocationRef >
class AssignConstraint final
    : public Constraint< VariableRef, MemoryLocationRef > {
private:
  using OperandT = Operand< VariableRef, MemoryLocationRef >;
  using ConstraintT = Constraint< VariableRef, MemoryLocationRef >;

private:
  // Result pointer variable
  VariableRef _result;

  // Operand
  std::unique_ptr< OperandT > _operand;

private:
  /// \brief Private constructor
  AssignConstraint(VariableRef result, std::unique_ptr< OperandT > operand)
      : ConstraintT(ConstraintT::AssignKind),
        _result(result),
        _operand(std::move(operand)) {}

public:
  /// \brief Static constructor
  static std::unique_ptr< AssignConstraint > create(
      VariableRef result, std::unique_ptr< OperandT > operand) {
    return std::unique_ptr< AssignConstraint >(
        new AssignConstraint(result, std::move(operand)));
  }

  /// \brief Return the result variable
  VariableRef result() const { return this->_result; }

  /// \brief Return the operand
  const OperandT* operand() const { return this->_operand.get(); }

  void dump(std::ostream& o) const override {
    DumpableTraits< VariableRef >::dump(o, this->_result);
    o << " = ";
    this->_operand->dump(o);
  }

}; // end class AssignConstraint

/// \brief Store constraint
///
/// Represents a constraint `*p = operand`
template < typename VariableRef, typename MemoryLocationRef >
class StoreConstraint final
    : public Constraint< VariableRef, MemoryLocationRef > {
private:
  using OperandT = Operand< VariableRef, MemoryLocationRef >;
  using ConstraintT = Constraint< VariableRef, MemoryLocationRef >;

private:
  // Pointer variable
  VariableRef _pointer;

  // Operand
  std::unique_ptr< OperandT > _operand;

private:
  /// \brief Private constructor
  StoreConstraint(VariableRef pointer, std::unique_ptr< OperandT > operand)
      : ConstraintT(ConstraintT::StoreKind),
        _pointer(pointer),
        _operand(std::move(operand)) {}

public:
  /// \brief Static constructor
  static std::unique_ptr< StoreConstraint > create(
      VariableRef pointer, std::unique_ptr< OperandT > operand) {
    return std::unique_ptr< StoreConstraint >(
        new StoreConstraint(pointer, std::move(operand)));
  }

  /// \brief Return the pointer variable
  VariableRef pointer() const { return this->_pointer; }

  /// \brief Return the operand
  const OperandT* operand() const { return this->_operand.get(); }

  void dump(std::ostream& o) const override {
    o << "*";
    DumpableTraits< VariableRef >::dump(o, this->_pointer);
    o << " = ";
    this->_operand->dump(o);
  }

}; // end class StoreConstraint

/// \brief Load constraint
///
/// Represents a constraint `p = *operand`
template < typename VariableRef, typename MemoryLocationRef >
class LoadConstraint final
    : public Constraint< VariableRef, MemoryLocationRef > {
private:
  using OperandT = Operand< VariableRef, MemoryLocationRef >;
  using ConstraintT = Constraint< VariableRef, MemoryLocationRef >;

private:
  // Result pointer variable
  VariableRef _result;

  // Operand
  std::unique_ptr< OperandT > _operand;

private:
  /// \brief Private constructor
  LoadConstraint(VariableRef result, std::unique_ptr< OperandT > operand)
      : ConstraintT(ConstraintT::LoadKind),
        _result(result),
        _operand(std::move(operand)) {}

public:
  /// \brief Static constructor
  static std::unique_ptr< LoadConstraint > create(
      VariableRef result, std::unique_ptr< OperandT > operand) {
    return std::unique_ptr< LoadConstraint >(
        new LoadConstraint(result, std::move(operand)));
  }

  /// \brief Return the result pointer variable
  VariableRef result() const { return this->_result; }

  /// \brief Return the operand
  const OperandT* operand() const { return this->_operand.get(); }

  void dump(std::ostream& o) const override {
    DumpableTraits< VariableRef >::dump(o, this->_result);
    o << " = *(";
    this->_operand->dump(o);
    o << ")";
  }

}; // end class LoadConstraint

/// \brief System of pointer constraints
template < typename VariableRef, typename MemoryLocationRef >
class ConstraintSystem {
public:
  static_assert(
      core::IsVariable< VariableRef >::value,
      "VariableRef does not meet the requirements for variable types");
  static_assert(core::IsMemoryLocation< MemoryLocationRef >::value,
                "MemoryLocationRef does not meet the requirements for memory "
                "location types");

private:
  /// \brief Hash for VariableRef
  struct VariableHash {
    std::size_t operator()(VariableRef v) const {
      return std::hash< Index >()(IndexableTraits< VariableRef >::index(v));
    }
  };

  /// \brief Hash for MemoryLocationRef
  struct MemoryLocationHash {
    std::size_t operator()(MemoryLocationRef m) const {
      return std::hash< Index >()(
          IndexableTraits< MemoryLocationRef >::index(m));
    }
  };

  using OperandT = Operand< VariableRef, MemoryLocationRef >;
  using VariableOperandT = VariableOperand< VariableRef, MemoryLocationRef >;
  using AddressOperandT = AddressOperand< VariableRef, MemoryLocationRef >;
  using ConstraintT = Constraint< VariableRef, MemoryLocationRef >;
  using AssignConstraintT = AssignConstraint< VariableRef, MemoryLocationRef >;
  using StoreConstraintT = StoreConstraint< VariableRef, MemoryLocationRef >;
  using LoadConstraintT = LoadConstraint< VariableRef, MemoryLocationRef >;
  using PointerAbsValueT = PointerAbsValue< MemoryLocationRef >;
  using PointsToSetT = PointsToSet< MemoryLocationRef >;
  using ConstraintVector = std::vector< std::unique_ptr< ConstraintT > >;
  using PointerMap =
      std::unordered_map< VariableRef, PointerAbsValueT, VariableHash >;
  using MemoryMap = std::
      unordered_map< MemoryLocationRef, PointerAbsValueT, MemoryLocationHash >;

public:
  using PointerIterator = typename PointerMap::const_iterator;
  using MemoryIterator = typename MemoryMap::const_iterator;

private:
  // List of pointer constraints
  ConstraintVector _csts;

  // Map from pointer variables to pointer abstract values
  PointerMap _pointers;

  // Map from memory locations to pointer abstract values
  //
  // `_memory[m]` contains the abstract union of all the pointers stored at `m`
  MemoryMap _memory;

  // Bit-width of pointer offsets (e.g, 32 or 64)
  uint64_t _offsets_bit_width;

  // Signedness of pointer offsets (usually Unsigned)
  Signedness _offsets_sign;

  // Number of iterations
  std::size_t _iteration = 0;

  // Change seen during iteration
  bool _change_seen = false;

public:
  /// \brief Default constructor
  ConstraintSystem(uint64_t offsets_bit_width, Signedness offsets_sign)
      : _offsets_bit_width(offsets_bit_width), _offsets_sign(offsets_sign) {}

  /// \brief No copy constructor
  ConstraintSystem(const ConstraintSystem&) = delete;

  /// \brief Move constructor
  ConstraintSystem(ConstraintSystem&&) = default;

  /// \brief No copy assignment operator
  ConstraintSystem& operator=(const ConstraintSystem&) = delete;

  /// \brief Move assignment operator
  ConstraintSystem& operator=(ConstraintSystem&&) = default;

  /// \brief Destructor
  ~ConstraintSystem() = default;

  /// \brief Add a pointer constraint
  void add(std::unique_ptr< ConstraintT > cst) {
    this->_csts.emplace_back(std::move(cst));
  }

private:
  class BinaryOp {
  public:
    /// \brief Constructor
    BinaryOp() = default;

    /// \brief No copy constructor
    BinaryOp(const BinaryOp&) = delete;

    /// \brief No move constructor
    BinaryOp(BinaryOp&&) = delete;

    /// \brief No copy assignment operator
    BinaryOp& operator=(const BinaryOp&) = delete;

    /// \brief No move assignment operator
    BinaryOp& operator=(BinaryOp&&) = delete;

    /// \brief Destructor
    virtual ~BinaryOp() = default;

    virtual bool convergence_achieved(const PointerAbsValueT& before,
                                      const PointerAbsValueT& after) const = 0;

    virtual void apply(PointerAbsValueT& before,
                       const PointerAbsValueT& after,
                       std::size_t iteration) const = 0;
  };

  class Extrapolate final : public BinaryOp {
  private:
    std::size_t _threshold;

  public:
    explicit Extrapolate(std::size_t threshold) : _threshold(threshold) {}

    bool convergence_achieved(const PointerAbsValueT& before,
                              const PointerAbsValueT& after) const override {
      return after.leq(before);
    }

    void apply(PointerAbsValueT& before,
               const PointerAbsValueT& after,
               std::size_t iteration) const override {
      if (iteration < this->_threshold) {
        before.join_with(after);
      } else {
        before.widen_with(after);
      }
    }
  };

  class Refine final : public BinaryOp {
  public:
    // This refinement is not a narrowing since it will not ensure
    // convergence. Thus, we need to make sure it is called a finite
    // number of times.

  public:
    Refine() = default;

    bool convergence_achieved(
        const PointerAbsValueT& /*before*/,
        const PointerAbsValueT& /*after*/) const override {
      return false;
    }

    void apply(PointerAbsValueT& before,
               const PointerAbsValueT& after,
               std::size_t /*iteration*/) const override {
      if (after.leq(before)) {
        // descending chain
        before.narrow_with(after);
      } else {
        // no descending chain
        before.join_with(after);
      }
    }
  };

  /// \brief Perform a step of the fixpoint convergence using the given operator
  ///
  /// It processes all the constraints once.
  void step(const BinaryOp& op) {
    for (auto it = this->_csts.begin(), et = this->_csts.end(); it != et;
         ++it) {
      this->process_constraint(it->get(), op);
    }
  }

  /// \brief Process the given constraint
  ///
  /// It updates this->_pointers and this->_memory
  void process_constraint(const ConstraintT* cst, const BinaryOp& op) {
    switch (cst->kind()) {
      case ConstraintT::AssignKind: {
        auto assign = static_cast< const AssignConstraintT* >(cst);
        PointerAbsValueT op_value = this->process_operand(assign->operand());
        this->add_pointer(assign->result(), op_value, op);
      } break;
      case ConstraintT::StoreKind: {
        auto store = static_cast< const StoreConstraintT* >(cst);
        PointerAbsValueT ptr_value = this->get_pointer(store->pointer());
        PointerAbsValueT op_value = this->process_operand(store->operand());
        if (ptr_value.is_bottom()) {
          return;
        }
        for (MemoryLocationRef addr : ptr_value.points_to()) {
          this->add_memory(addr, op_value, op);
        }
      } break;
      case ConstraintT::LoadKind: {
        auto load = static_cast< const LoadConstraintT* >(cst);
        PointerAbsValueT op_value = this->process_operand(load->operand());
        if (op_value.is_bottom()) {
          return;
        }
        for (MemoryLocationRef addr : op_value.points_to()) {
          this->add_pointer(load->result(), this->get_memory(addr), op);
        }
      } break;
      default: {
        ikos_unreachable("unexpected kind");
      }
    }
  }

  /// \brief Return the abstract value for the given operand
  PointerAbsValueT process_operand(const OperandT* op) {
    switch (op->kind()) {
      case OperandT::VariableKind: {
        auto variable_op = static_cast< const VariableOperandT* >(op);
        PointerAbsValueT value = this->get_pointer(variable_op->var());
        value.add_offset(variable_op->offset());
        return value;
      }
      case OperandT::AddressKind: {
        auto address_op = static_cast< const AddressOperandT* >(op);
        return PointerAbsValueT(Uninitialized::top(),
                                Nullity::top(),
                                PointsToSetT{address_op->address()},
                                address_op->offset());
      }
      default: {
        ikos_unreachable("unexpected kind");
      }
    }
  }

public:
  /// \brief Return the abstract value for the given pointer
  PointerAbsValueT get_pointer(VariableRef p) {
    auto it = this->_pointers.find(p);
    if (it == this->_pointers.end()) {
      return PointerAbsValueT::bottom(this->_offsets_bit_width,
                                      this->_offsets_sign);
    } else {
      return it->second;
    }
  }

  /// \brief Begin iterator over the pairs (pointer, abstract value)
  PointerIterator pointer_begin() const { return this->_pointers.cbegin(); }

  /// \brief End iterator over the pairs (pointer, abstract value)
  PointerIterator pointer_end() const { return this->_pointers.cend(); }

  /// \brief Return the abstract value for pointers stored at the given memory
  /// location
  PointerAbsValueT get_memory(MemoryLocationRef m) {
    auto it = this->_memory.find(m);
    if (it == this->_memory.end()) {
      return PointerAbsValueT::bottom(this->_offsets_bit_width,
                                      this->_offsets_sign);
    } else {
      return it->second;
    }
  }

  /// \brief Begin iterator over the pairs (memory location, abstract value)
  MemoryIterator memory_begin() const { return this->_memory.cbegin(); }

  /// \brief End iterator over the pairs (memory location, abstract value)
  MemoryIterator memory_end() const { return this->_memory.cend(); }

private:
  /// \brief Add a pointer abstraction for the given pointer
  void add_pointer(VariableRef p,
                   const PointerAbsValueT& value,
                   const BinaryOp& op) {
    // Get a reference on the current value
    auto it = this->_pointers.find(p);
    if (it == this->_pointers.end()) {
      // add default value
      auto res = this->_pointers
                     .emplace(p,
                              PointerAbsValueT::bottom(this->_offsets_bit_width,
                                                       this->_offsets_sign));
      it = res.first;
    }
    this->add_apply(it->second, value, op);
  }

  /// \brief Add a pointer abstraction for the given memory location
  void add_memory(MemoryLocationRef m,
                  const PointerAbsValueT& value,
                  const BinaryOp& op) {
    // Get a reference on the current value
    auto it = this->_memory.find(m);
    if (it == this->_memory.end()) {
      // add default value
      auto res = this->_memory
                     .emplace(m,
                              PointerAbsValueT::bottom(this->_offsets_bit_width,
                                                       this->_offsets_sign));
      it = res.first;
    }
    this->add_apply(it->second, value, op);
  }

  /// \brief Add `after` in `before`, applying the given binary operator `op`
  void add_apply(PointerAbsValueT& before,
                 const PointerAbsValueT& after,
                 const BinaryOp& op) {
    if (!op.convergence_achieved(before, after)) {
      this->_change_seen = true;
      op.apply(before, after, this->_iteration);
    }
  }

public:
  /// \brief Solve the constraint system
  void solve(std::size_t widening_threshold = 50,
             std::size_t /*narrowing_threshold*/ = 1) {
    this->_iteration = 0;
    Extrapolate widening_op(widening_threshold);
    do {
      this->_iteration++;
      this->_change_seen = false;
      this->step(widening_op);
    } while (this->_change_seen);

    // TODO(marthaud): commented out because this is unsound.
    //
    // The algorithm here does not compute a proper fixpoint, because the
    // widening and narrowing operations should be applied between two
    // consecutive iterations. We can have several constraints updating the
    // same pointer, thus calling the widening/narrowing several times on the
    // same iteration. This is unsound.
    //
    // As a short fix, the narrowing step is disabled.
    //
    // See https://babelfish.arc.nasa.gov/jira/projects/IKOS/issues/IKOS-71

    // Refine narrowing_op;
    // this->_iteration = 0;
    // for (; this->_iteration < narrowing_threshold; ++this->_iteration) {
    //  this->step(narrowing_op);
    // }
  }

  /// \brief Dump the constraint system, for debugging purpose
  void dump(std::ostream& o) const {
    o << "{\n";
    for (auto it = this->_csts.begin(), et = this->_csts.end(); it != et;
         ++it) {
      o << "  ";
      it->get()->dump(o);
      o << "\n";
    }
    o << "}";
  }

}; // end class ConstraintSystem

} // end namespace pointer
} // end namespace core
} // end namespace ikos
