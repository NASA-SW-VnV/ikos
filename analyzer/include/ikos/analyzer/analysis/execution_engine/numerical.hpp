/******************************************************************************
 *
 * \file
 * \brief Numerical execution engine
 *
 * Author: Maxime Arthaud
 *
 * Contributors: Jorge A. Navas
 *               Clement Decoodt
 *               Thomas Bailleux
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

#include <ikos/ar/semantic/intrinsic.hpp>
#include <ikos/ar/verify/type.hpp>

#include <ikos/core/domain/exception/abstract_domain.hpp>
#include <ikos/core/domain/memory/abstract_domain.hpp>

#include <ikos/analyzer/analysis/context.hpp>
#include <ikos/analyzer/analysis/execution_engine/engine.hpp>
#include <ikos/analyzer/analysis/literal.hpp>
#include <ikos/analyzer/analysis/liveness.hpp>
#include <ikos/analyzer/analysis/option.hpp>
#include <ikos/analyzer/analysis/pointer/value.hpp>
#include <ikos/analyzer/support/assert.hpp>
#include <ikos/analyzer/support/cast.hpp>

namespace ikos {
namespace analyzer {

/// \brief Numerical execution engine
///
/// This class performs the transfer function on each AR (Abstract
/// Representation) statement with different levels of precision.
///
/// It relies on an abstract domain.
///
/// The abstract domain must implement the exception abstract domain interface
/// to handle exception propagation correctly.
///
/// The exception abstract domain must provide an underlying abstract domain
/// that must implement the memory abstract domain interface to handle scalar
/// variables and memory operations.
template < typename AbstractDomain >
class NumericalExecutionEngine final : public ExecutionEngine {
public:
  static_assert(core::exception::IsAbstractDomain< AbstractDomain >::value,
                "AbstractDomain must implement exception::AbstractDomain");
  static_assert(core::memory::IsAbstractDomain<
                    typename AbstractDomain::UnderlyingDomainT,
                    Variable*,
                    MemoryLocation* >::value,
                "AbstractDomain::UnderlyingDomainT must implement "
                "memory::AbstractDomain");

private:
  using IntInterval = core::machine_int::Interval;
  using IntIntervalCongruence = core::machine_int::IntervalCongruence;
  using IntVariable = core::VariableExpression< MachineInt, Variable* >;
  using IntLinearExpression = core::LinearExpression< MachineInt, Variable* >;
  using IntLinearConstraint = core::LinearConstraint< MachineInt, Variable* >;
  using IntLinearConstraintSystem =
      core::LinearConstraintSystem< MachineInt, Variable* >;
  using Nullity = core::Nullity;
  using Uninitialized = core::Uninitialized;
  using Lifetime = core::Lifetime;
  using IntUnaryOperator = core::machine_int::UnaryOperator;
  using IntBinaryOperator = core::machine_int::BinaryOperator;
  using IntPredicate = core::machine_int::Predicate;
  using PointerPredicate = core::pointer::Predicate;

private:
  /// \brief Current invariant
  AbstractDomain _inv;

  /// \brief Analysis context
  Context& _ctx;

  /// \brief Memory location factory
  MemoryFactory& _mem_factory;

  /// \brief Variable factory
  VariableFactory& _var_factory;

  /// \brief Literal factory
  LiteralFactory& _lit_factory;

  /// \brief Data layout
  const ar::DataLayout& _data_layout;

  /// \brief Call context
  CallContext* _call_context;

  /// \brief Execution engine options
  ExecutionEngineOptions _opts;

  /// \brief Optional liveness information
  const LivenessAnalysis* _liveness;

  /// \brief Optional pointer information
  const PointerInfo* _pointer_info;

public:
  /// \brief Constructor
  ///
  /// \param inv Initial invariant
  /// \param ctx Analysis context
  /// \param call_context Calling context
  /// \param opts Execution engine options
  /// \param liveness Liveness analysis, or null
  /// \param pointer_info Pointer information, or null
  NumericalExecutionEngine(AbstractDomain inv,
                           Context& ctx,
                           CallContext* call_context,
                           ExecutionEngineOptions opts,
                           const LivenessAnalysis* liveness = nullptr,
                           const PointerInfo* pointer_info = nullptr)
      : _inv(std::move(inv)),
        _ctx(ctx),
        _mem_factory(*ctx.mem_factory),
        _var_factory(*ctx.var_factory),
        _lit_factory(*ctx.lit_factory),
        _data_layout(ctx.bundle->data_layout()),
        _call_context(call_context),
        _opts(opts),
        _liveness(liveness),
        _pointer_info(pointer_info) {}

private:
  /// \brief Private copy constructor
  NumericalExecutionEngine(const NumericalExecutionEngine&) = default;

public:
  /// \brief Public move constructor
  NumericalExecutionEngine(NumericalExecutionEngine&&) noexcept(
      std::is_nothrow_move_constructible< AbstractDomain >::value) = default;

  /// \brief No copy assignment operator
  NumericalExecutionEngine& operator=(const NumericalExecutionEngine&) = delete;

  /// \brief No move assignment operator
  NumericalExecutionEngine& operator=(NumericalExecutionEngine&&) = delete;

  /// \brief Destructor
  ~NumericalExecutionEngine() override = default;

  /// \brief Create a fresh numerical execution engine, with its own abstract
  /// domain
  NumericalExecutionEngine fork() const { return *this; }

  /// \brief Return the current invariant
  AbstractDomain& inv() { return this->_inv; }

  /// \brief Return the current invariant
  const AbstractDomain& inv() const { return this->_inv; }

  /// \brief Update the current invariant
  void set_inv(const AbstractDomain& inv) { this->_inv = inv; }

  /// \brief Update the current invariant
  void set_inv(AbstractDomain&& inv) { this->_inv = std::move(inv); }

  /// \brief Return the liveness analysis used, or null
  const LivenessAnalysis* liveness() const { return this->_liveness; }

  /// \brief Return the pointer information, or null
  const PointerInfo* pointer_info() const { return this->_pointer_info; }

public:
  /// \name Helpers to allocate memory
  /// @{

  /// \brief Initial value for a memory block
  enum class MemoryInitialValue {
    // Memory is initialized with zeros
    Zero,

    // Memory is uninitialized, reading it is an error
    Uninitialized,

    // Memory is unknown, reading it returns a non-deterministic value
    Unknown,
  };

  /// \brief Allocate a new memory object `addr` with unknown size
  ///
  /// We consider as a memory object an alloca (i.e., stack variables), global
  /// variables, malloc-like allocation sites, function pointers, and
  /// destination of inttoptr instructions. Also, variables whose address might
  /// have been taken are translated to global variables by the front-end.
  void allocate_memory(Variable* ptr,
                       MemoryLocation* addr,
                       Nullity nullity,
                       Lifetime lifetime,
                       MemoryInitialValue init_val) {
    // Update pointer
    this->_inv.normal().pointer_assign(ptr, addr, nullity);

    // Update memory location lifetime
    this->_inv.normal().lifetime_set(addr, lifetime);

    // Update memory value
    if (init_val == MemoryInitialValue::Zero) {
      this->_inv.normal().mem_zero_reachable(ptr);
    } else if (init_val == MemoryInitialValue::Uninitialized) {
      this->_inv.normal().mem_uninitialize_reachable(ptr);
    } else if (init_val == MemoryInitialValue::Unknown) {
      this->_inv.normal().mem_forget_reachable(ptr);
    } else {
      ikos_unreachable("unreachable");
    }
  }

  /// \brief Allocate a new memory object `addr` of size `alloc_size` (in bytes)
  void allocate_memory(Variable* ptr,
                       MemoryLocation* addr,
                       Nullity nullity,
                       Lifetime lifetime,
                       MemoryInitialValue init_val,
                       const MachineInt& alloc_size) {
    // Update pointer, lifetime and initial value for the memory location
    this->allocate_memory(ptr, addr, nullity, lifetime, init_val);
    if (init_val == MemoryInitialValue::Uninitialized) {
      // When the size of the allocation is known (like it is here)
      // we mark the storage as uninitialized by assigning it
      // the undefined value.
      this->_inv.normal().mem_write(ptr, ScalarLit::undefined(), alloc_size);
    }

    if (this->_opts.test(ExecutionEngine::UpdateAllocSizeVar)) {
      // Update allocation size var
      Variable* alloc_size_var = this->_var_factory.get_alloc_size(addr);
      this->_inv.normal().int_assign(alloc_size_var, alloc_size);
    }
  }

  /// \brief Allocate a new memory object `addr` of size `alloc_size` (in bytes)
  void allocate_memory(Variable* ptr,
                       MemoryLocation* addr,
                       Nullity nullity,
                       Lifetime lifetime,
                       MemoryInitialValue init_val,
                       Variable* alloc_size) {
    // Update pointer, lifetime and initial value for the memory location
    this->allocate_memory(ptr, addr, nullity, lifetime, init_val);

    if (this->_opts.test(ExecutionEngine::UpdateAllocSizeVar)) {
      // Update allocation size var
      Variable* alloc_size_var = this->_var_factory.get_alloc_size(addr);
      this->_inv.normal().uninit_assert_initialized(alloc_size);
      this->_inv.normal().int_assign(alloc_size_var, alloc_size);
    }
  }

private:
  /// @}
  /// \name Internal helpers
  /// @{

  /// \brief Initialize a global variable or function pointer operand
  ///
  /// Global variables and constant function pointers are not stored in the
  /// initial invariant, so it is necessary to initialize them on the fly when
  /// we need them.
  void init_global_operand(ar::Value* value) {
    if (auto gv = dyn_cast< ar::GlobalVariable >(value)) {
      this->_inv.normal().pointer_assign(this->_var_factory.get_global(gv),
                                         this->_mem_factory.get_global(gv),
                                         Nullity::non_null());
    } else if (auto fun_ptr = dyn_cast< ar::FunctionPointerConstant >(value)) {
      auto fun = fun_ptr->function();
      this->_inv.normal().pointer_assign(this->_var_factory.get_function_ptr(
                                             fun),
                                         this->_mem_factory.get_function(fun),
                                         Nullity::non_null());
    } else if (auto struct_cst = dyn_cast< ar::StructConstant >(value)) {
      for (auto it = struct_cst->field_begin(), et = struct_cst->field_end();
           it != et;
           ++it) {
        this->init_global_operand(it->value);
      }
    } else if (auto seq_cst = dyn_cast< ar::SequentialConstant >(value)) {
      for (auto it = seq_cst->element_begin(), et = seq_cst->element_end();
           it != et;
           ++it) {
        this->init_global_operand(*it);
      }
    }
  }

  /// \brief Initialize global variables and function pointer operands
  void init_global_operands(ar::Statement* s) {
    for (auto it = s->op_begin(), et = s->op_end(); it != et; ++it) {
      this->init_global_operand(*it);
    }
  }

  /// \brief Prepare a memory access (read/write) on the given pointer
  ///
  /// Return true if the memory access can be performed, i.e the pointer is
  /// non-null and well defined
  bool prepare_mem_access(const ScalarLit& ptr) {
    if (ptr.is_undefined()) {
      // Undefined pointer dereference
      this->_inv.set_normal_flow_to_bottom();
      return false;
    } else if (ptr.is_null()) {
      // Null pointer dereference
      this->_inv.set_normal_flow_to_bottom();
      return false;
    }

    ikos_assert_msg(ptr.is_pointer_var(), "unexpected parameter");

    // Reduction between value and pointer analysis
    this->refine_addresses_offset(ptr.var());

    // Assert `ptr != null`
    this->_inv.normal().nullity_assert_non_null(ptr.var());

    this->_inv.normal().normalize();

    // Ready for read/write
    return !this->_inv.is_normal_flow_bottom();
  }

  /// \brief Normalize the nullity domain
  ///
  /// Check if the given pointer variable points to AbsoluteZeroMemoryLocation.
  /// If so, check if the offset interval contains zero, and update the nullity
  /// domain accordingly.
  void normalize_absolute_zero_nullity(Variable* p) {
    auto nullity = this->_inv.normal().nullity_to_nullity(p);
    if (nullity.is_bottom() || nullity.is_top()) {
      return;
    }

    PointsToSet addrs = this->_inv.normal().pointer_to_points_to(p);

    if (addrs.contains(this->_mem_factory.get_absolute_zero())) {
      IntIntervalCongruence offset =
          this->_inv.normal().pointer_offset_to_interval_congruence(p);
      auto zero = MachineInt::zero(offset.bit_width(), offset.sign());

      if (offset.is_bottom()) {
        return;
      } else if (addrs.singleton()) {
        if (offset.singleton() == boost::optional< MachineInt >(zero)) {
          // Pointer is definitely null (base is zero, offset = 0)
          this->_inv.normal().nullity_set(p, Nullity::null());
        } else if (!offset.contains(zero)) {
          // Pointer is definitely non-null (base is zero, offset != 0)
          this->_inv.normal().nullity_set(p, Nullity::non_null());
        } else {
          // Pointer might be null (base is zero, offset contains zero)
          this->_inv.normal().nullity_set(p, Nullity::top());
        }
      } else if (offset.contains(zero)) {
        // Pointer might be null (base might be zero, offset contains zero)
        this->_inv.normal().nullity_set(p, Nullity::top());
      }
    }
  }

  /// \brief Refine the addresses of `ptr` using information from an external
  /// pointer analysis
  void refine_addresses(Variable* ptr) {
    if (!this->_pointer_info) {
      return;
    }

    PointerAbsValue value = this->_pointer_info->get(ptr);
    this->_inv.normal().pointer_refine(ptr, value.points_to());
  }

  /// \brief Refine the addresses and offset of `ptr` using information from an
  /// external pointer analysis
  void refine_addresses_offset(Variable* ptr) {
    if (!this->_pointer_info) {
      return;
    }

    PointerAbsValue value = this->_pointer_info->get(ptr);
    this->_inv.normal().pointer_refine(ptr, value);
  }

private:
  /// @}
  /// \name Helpers for assignments
  /// @{

  /// \brief Integer variable assignment
  class IntegerAssign : public ScalarLit::template Visitor<> {
  private:
    Variable* _lhs;
    AbstractDomain& _inv;

  public:
    IntegerAssign(Variable* lhs, AbstractDomain& inv) : _lhs(lhs), _inv(inv) {}

    void machine_int(const MachineInt& rhs) {
      this->_inv.normal().int_assign(this->_lhs, rhs);
    }

    void floating_point(const DummyNumber&) { ikos_unreachable("unreachable"); }

    void memory_location(MemoryLocation*) { ikos_unreachable("unreachable"); }

    void null() { ikos_unreachable("unreachable"); }

    void undefined() { this->_inv.normal().int_assign_undef(this->_lhs); }

    void machine_int_var(Variable* rhs) {
      this->_inv.normal().int_assign(this->_lhs, rhs);
    }

    void floating_point_var(Variable*) { ikos_unreachable("unreachable"); }

    void pointer_var(Variable*) { ikos_unreachable("unreachable"); }

  }; // end class IntegerAssign

  /// \brief Floating point variable assignment
  class FloatingPointAssign : public ScalarLit::template Visitor<> {
  private:
    Variable* _lhs;
    AbstractDomain& _inv;

  public:
    FloatingPointAssign(Variable* lhs, AbstractDomain& inv)
        : _lhs(lhs), _inv(inv) {}

    void machine_int(const MachineInt&) { ikos_unreachable("unreachable"); }

    void floating_point(const DummyNumber&) {
      this->_inv.normal().float_assign_nondet(this->_lhs);
    }

    void memory_location(MemoryLocation*) { ikos_unreachable("unreachable"); }

    void null() { ikos_unreachable("unreachable"); }

    void undefined() { this->_inv.normal().float_assign_undef(this->_lhs); }

    void machine_int_var(Variable*) { ikos_unreachable("unreachable"); }

    void floating_point_var(Variable* rhs) {
      this->_inv.normal().float_assign(this->_lhs, rhs);
    }

    void pointer_var(Variable*) { ikos_unreachable("unreachable"); }

  }; // end class FloatingPointAssign

  /// \brief Pointer variable assignment
  class PointerAssign : public ScalarLit::template Visitor<> {
  private:
    Variable* _lhs;
    AbstractDomain& _inv;

  public:
    PointerAssign(Variable* lhs, AbstractDomain& inv) : _lhs(lhs), _inv(inv) {}

    void machine_int(const MachineInt&) { ikos_unreachable("unreachable"); }

    void floating_point(const DummyNumber&) { ikos_unreachable("unreachable"); }

    void memory_location(MemoryLocation* addr) {
      this->_inv.normal().pointer_assign(this->_lhs, addr, Nullity::non_null());
    }

    void null() { this->_inv.normal().pointer_assign_null(this->_lhs); }

    void undefined() { this->_inv.normal().pointer_assign_undef(this->_lhs); }

    void machine_int_var(Variable*) { ikos_unreachable("unreachable"); }

    void floating_point_var(Variable*) { ikos_unreachable("unreachable"); }

    void pointer_var(Variable* rhs) {
      this->_inv.normal().pointer_assign(this->_lhs, rhs);
    }

  }; // end class PointerAssign

  /// \brief Scalar assignment `lhs = rhs`
  ///
  /// Requires that `lhs` and `rhs` have the same type.
  /// Propagates uninitialized variables.
  void assign(const ScalarLit& lhs, const ScalarLit& rhs) {
    if (lhs.is_machine_int_var()) {
      IntegerAssign v(lhs.var(), this->_inv);
      rhs.apply_visitor(v);
    } else if (lhs.is_floating_point_var()) {
      FloatingPointAssign v(lhs.var(), this->_inv);
      rhs.apply_visitor(v);
    } else if (lhs.is_pointer_var()) {
      PointerAssign v(lhs.var(), this->_inv);
      rhs.apply_visitor(v);
    } else {
      ikos_unreachable("left hand side is not a variable");
    }
  }

private:
  /// @}
  /// \name Helpers for implicit bitcasts
  /// @{

  /// \brief Integer variable implicit bitcast
  class IntegerImplicitBitcast : public ScalarLit::template Visitor<> {
  private:
    Variable* _lhs;
    ar::IntegerType* _type;
    AbstractDomain& _inv;

  public:
    IntegerImplicitBitcast(Variable* lhs, AbstractDomain& inv)
        : _lhs(lhs),
          _type(ar::cast< ar::IntegerType >(lhs->type())),
          _inv(inv) {}

    void machine_int(const MachineInt& rhs) {
      ikos_assert(this->_type->bit_width() == rhs.bit_width());
      if (this->_type->sign() == rhs.sign()) {
        this->_inv.normal().int_assign(this->_lhs, rhs);
      } else {
        this->_inv.normal().int_assign(this->_lhs,
                                       rhs.sign_cast(this->_type->sign()));
      }
    }

    void floating_point(const DummyNumber&) { ikos_unreachable("unreachable"); }

    void memory_location(MemoryLocation*) { ikos_unreachable("unreachable"); }

    void null() { ikos_unreachable("unreachable"); }

    void undefined() { this->_inv.set_normal_flow_to_bottom(); }

    void machine_int_var(Variable* rhs) {
      auto rhs_type = ar::cast< ar::IntegerType >(rhs->type());

      ikos_assert(this->_type->bit_width() == rhs_type->bit_width());
      if (this->_type->sign() == rhs_type->sign()) {
        this->_inv.normal().uninit_assert_initialized(rhs);
        this->_inv.normal().int_assign(this->_lhs, rhs);
      } else {
        this->_inv.normal().int_apply(IntUnaryOperator::SignCast,
                                      this->_lhs,
                                      rhs);
      }
    }

    void floating_point_var(Variable*) { ikos_unreachable("unreachable"); }

    void pointer_var(Variable*) { ikos_unreachable("unreachable"); }

  }; // end class IntegerImplicitBitcast

  /// \brief Floating point variable implement bitcast
  class FloatingPointImplicitBitcast : public ScalarLit::template Visitor<> {
  private:
    Variable* _lhs;
    AbstractDomain& _inv;

  public:
    FloatingPointImplicitBitcast(Variable* lhs, AbstractDomain& inv)
        : _lhs(lhs), _inv(inv) {}

    void machine_int(const MachineInt&) { ikos_unreachable("unreachable"); }

    void floating_point(const DummyNumber&) {
      this->_inv.normal().float_assign_nondet(this->_lhs);
    }

    void memory_location(MemoryLocation*) { ikos_unreachable("unreachable"); }

    void null() { ikos_unreachable("unreachable"); }

    void undefined() { this->_inv.set_normal_flow_to_bottom(); }

    void machine_int_var(Variable*) { ikos_unreachable("unreachable"); }

    void floating_point_var(Variable* rhs) {
      this->_inv.normal().uninit_assert_initialized(rhs);
      this->_inv.normal().float_assign(this->_lhs, rhs);
    }

    void pointer_var(Variable*) { ikos_unreachable("unreachable"); }

  }; // end class FloatingPointImplicitBitcast

  /// \brief Pointer variable implicit bitcast
  class PointerImplicitBitcast : public ScalarLit::template Visitor<> {
  private:
    Variable* _lhs;
    AbstractDomain& _inv;

  public:
    PointerImplicitBitcast(Variable* lhs, AbstractDomain& inv)
        : _lhs(lhs), _inv(inv) {}

    void machine_int(const MachineInt&) { ikos_unreachable("unreachable"); }

    void floating_point(const DummyNumber&) { ikos_unreachable("unreachable"); }

    void memory_location(MemoryLocation* addr) {
      this->_inv.normal().pointer_assign(this->_lhs, addr, Nullity::non_null());
    }

    void null() { this->_inv.normal().pointer_assign_null(this->_lhs); }

    void undefined() { this->_inv.set_normal_flow_to_bottom(); }

    void machine_int_var(Variable*) { ikos_unreachable("unreachable"); }

    void floating_point_var(Variable*) { ikos_unreachable("unreachable"); }

    void pointer_var(Variable* rhs) {
      this->_inv.normal().uninit_assert_initialized(rhs);
      this->_inv.normal().pointer_assign(this->_lhs, rhs);
    }

  }; // end class PointerImplicitBitcast

  /// \brief Implicit bitcast `lhs = rhs`
  ///
  /// Requires either one of:
  ///  - `lhs` and `rhs` have the same type
  ///  - `lhs` and `rhs` are integers of same bit-width (signed <-> unsigned)
  ///  - `lhs` and `rhs` are pointer types (ie., A* <-> B*)
  ///
  /// Implicit bitcast on an uninitialized variable is an error.
  void implicit_bitcast(const ScalarLit& lhs, const ScalarLit& rhs) {
    if (lhs.is_machine_int_var()) {
      IntegerImplicitBitcast v(lhs.var(), this->_inv);
      rhs.apply_visitor(v);
    } else if (lhs.is_floating_point_var()) {
      FloatingPointImplicitBitcast v(lhs.var(), this->_inv);
      rhs.apply_visitor(v);
    } else if (lhs.is_pointer_var()) {
      PointerImplicitBitcast v(lhs.var(), this->_inv);
      rhs.apply_visitor(v);
    } else {
      ikos_unreachable("left hand side is not a variable");
    }
  }

private:
  /// @}
  /// \name Helpers for aggregate (struct, array) statements
  /// @{

  /// \brief Return the type void*
  ar::Type* void_ptr_type() const {
    ar::Context& ctx = _ctx.bundle->context();
    return ar::PointerType::get(ctx, ar::VoidType::get(ctx));
  }

  /// \brief Initialize an aggregate memory location
  ///
  /// Internal variables of aggregate types are modeled as if they were in
  /// memory, at a symbolic location.
  ///
  /// Returns a pointer to the symbolic location of the aggregate in memory.
  Variable* init_aggregate_memory(const AggregateLit& aggregate) {
    ikos_assert_msg(aggregate.is_var(), "aggregate is not a variable");

    auto var = cast< InternalVariable >(aggregate.var());
    this->allocate_memory(var,
                          this->_mem_factory.get_aggregate(var->internal_var()),
                          Nullity::non_null(),
                          Lifetime::top(),
                          MemoryInitialValue::Zero);
    return var;
  }

  /// \brief Return a pointer to the symbolic location of the aggregate in
  /// memory
  Variable* aggregate_pointer(const AggregateLit& aggregate) {
    ikos_assert_msg(aggregate.is_var(), "aggregate is not a variable");

    auto var = cast< InternalVariable >(aggregate.var());
    this->_inv.normal().pointer_assign(var,
                                       this->_mem_factory.get_aggregate(
                                           var->internal_var()),
                                       Nullity::non_null());
    return var;
  }

  /// \brief Write an aggregate in the memory
  void mem_write_aggregate(Variable* ptr, const AggregateLit& aggregate) {
    if (aggregate.size().is_zero()) {
      return; // Nothing to do
    } else if (aggregate.is_cst()) {
      // Pointer to write the aggregate in the memory
      Variable* write_ptr =
          this->_var_factory.get_named_shadow(this->void_ptr_type(),
                                              "shadow.mem_write_aggregate.ptr");

      for (const auto& field : aggregate.fields()) {
        this->_inv.normal().pointer_assign(write_ptr, ptr, field.offset);
        this->_inv.normal().mem_write(write_ptr, field.value, field.size);
      }

      // Clean-up
      this->_inv.normal().pointer_forget(write_ptr);
    } else if (aggregate.is_zero() || aggregate.is_undefined()) {
      // aggregate.size() is in bytes, compute bit-width, and check
      // if the bit-width fits in an unsigned int
      // XXX: We should use mem_zero_reachable and mem_uninitialize_reachable
      bool overflow;
      MachineInt eight(8, aggregate.size().bit_width(), Unsigned);
      MachineInt bit_width = mul(aggregate.size(), eight, overflow);
      if (overflow || !bit_width.fits< uint64_t >()) {
        // Too big for a cell
        this->_inv.normal().mem_forget_reachable(ptr);
      } else if (aggregate.is_zero()) {
        MachineInt zero(0, bit_width.to< uint64_t >(), Signed);
        this->_inv.normal().mem_write(ptr,
                                      ScalarLit::machine_int(zero),
                                      aggregate.size());
      } else if (aggregate.is_undefined()) {
        this->_inv.normal().mem_write(ptr,
                                      ScalarLit::undefined(),
                                      aggregate.size());
      } else {
        ikos_unreachable("unreachable");
      }
    } else if (aggregate.is_var()) {
      Variable* aggregate_ptr = this->aggregate_pointer(aggregate);
      this->_inv.normal().mem_copy(ptr,
                                   aggregate_ptr,
                                   ScalarLit::machine_int(aggregate.size()));
    } else {
      ikos_unreachable("unreachable");
    }
  }

  /// \brief Aggregate assignment `lhs = rhs`
  ///
  /// Propagates uninitialized variables.
  void assign(const AggregateLit& lhs, const AggregateLit& rhs) {
    ikos_assert_msg(lhs.is_var(), "left hand side is not a variable");

    Variable* ptr = this->init_aggregate_memory(lhs);
    this->mem_write_aggregate(ptr, rhs);
  }

  /// \brief Assignment `lhs = rhs`
  ///
  /// Requires that `lhs` and `rhs` have the same type.
  /// Propagates uninitialized variables.
  void assign(const Literal& lhs, const Literal& rhs) {
    if (lhs.is_scalar()) {
      ikos_assert_msg(rhs.is_scalar(), "unexpected right hand side");
      this->assign(lhs.scalar(), rhs.scalar());
    } else if (lhs.is_aggregate()) {
      ikos_assert_msg(rhs.is_aggregate(), "unexpected right hand side");
      this->assign(lhs.aggregate(), rhs.aggregate());
    } else {
      ikos_unreachable("unreachable");
    }
  }

  /// \brief Implicit bitcast `lhs = rhs`
  ///
  /// Requires either one of:
  ///  - `lhs` and `rhs` have the same type
  ///  - `lhs` and `rhs` are integers of same bit-width (signed <-> unsigned)
  ///  - `lhs` and `rhs` are pointer types (ie., A* <-> B*)
  ///
  /// Implicit bitcast on an uninitialized variable is an error.
  void implicit_bitcast(const Literal& lhs, const Literal& rhs) {
    if (lhs.is_scalar()) {
      ikos_assert_msg(rhs.is_scalar(), "unexpected right hand side");
      this->implicit_bitcast(lhs.scalar(), rhs.scalar());
    } else if (lhs.is_aggregate()) {
      ikos_assert_msg(rhs.is_aggregate(), "unexpected right hand side");
      this->assign(lhs.aggregate(), rhs.aggregate());
    } else {
      ikos_unreachable("unreachable");
    }
  }

  /// \brief Randomly throw unknown exceptions with the current invariant
  ///
  /// Equivalent to if (rand()) { throw rand(); }
  void throw_unknown_exceptions() {
    this->_inv.caught_exceptions().join_with(this->_inv.normal());
  }

public:
  /// \brief Deallocate the memory for the given local variables
  void deallocate_local_variables(ar::Function::LocalVariableIterator begin,
                                  ar::Function::LocalVariableIterator end) {
    for (auto it = begin; it != end; ++it) {
      LocalVariable* var = this->_var_factory.get_local(*it);
      MemoryLocation* addr = this->_mem_factory.get_local(*it);

      // Forget local variable pointer
      this->_inv.normal().pointer_forget(var);
      this->_inv.caught_exceptions().pointer_forget(var);
      this->_inv.propagated_exceptions().pointer_forget(var);

      // Forget the memory content
      this->_inv.normal().mem_forget(addr);
      this->_inv.caught_exceptions().mem_forget(addr);
      this->_inv.propagated_exceptions().mem_forget(addr);

      // Set the memory location lifetime to deallocated
      this->_inv.normal().lifetime_assign_deallocated(addr);
      this->_inv.caught_exceptions().lifetime_assign_deallocated(addr);
      this->_inv.propagated_exceptions().lifetime_assign_deallocated(addr);

      if (this->_opts.test(ExecutionEngine::UpdateAllocSizeVar)) {
        // Forget the allocation size variable
        AllocSizeVariable* alloc_size_var =
            this->_var_factory.get_alloc_size(addr);
        this->_inv.normal().int_forget(alloc_size_var);
        this->_inv.caught_exceptions().int_forget(alloc_size_var);
        this->_inv.propagated_exceptions().int_forget(alloc_size_var);
      }
    }
  }

public:
  /// @}
  /// \name Implement ExecutionEngine
  /// @{

  /// \brief Enter a basic block
  void exec_enter(ar::BasicBlock*) override {}

  /// \brief Leave a basic block
  ///
  /// Use the liveness analysis to remove dead variables
  void exec_leave(ar::BasicBlock* bb) override {
    if (this->_liveness == nullptr) {
      return;
    }

    boost::optional< const LivenessAnalysis::VariableRefList& > dead =
        this->_liveness->dead_at_end(bb);

    if (!dead) {
      return;
    }

    // Do not remove the returned variable
    Variable* returned_var = nullptr;
    if (!bb->empty() && isa< ar::ReturnValue >(bb->back())) {
      auto ret = cast< ar::ReturnValue >(bb->back());

      if (ret->has_operand()) {
        const Literal& v = this->_lit_factory.get(ret->operand());

        if (v.is_var()) {
          returned_var = v.var();
        }
      }
    }

    for (Variable* var : *dead) {
      if (var == returned_var) { // Ignore
        continue;
      }

      // Special case for aggregate internal variables: Clean-up the memory
      if (auto iv = dyn_cast< InternalVariable >(var)) {
        ar::InternalVariable* ar_iv = iv->internal_var();
        if (ar_iv->type()->is_aggregate()) {
          MemoryLocation* addr = this->_mem_factory.get_aggregate(ar_iv);
          this->_inv.normal().mem_forget(addr);
          this->_inv.caught_exceptions().mem_forget(addr);
          this->_inv.propagated_exceptions().mem_forget(addr);
        }
      }

      // Clean-up scalars
      this->_inv.normal().scalar_forget(var);
      this->_inv.caught_exceptions().scalar_forget(var);
      this->_inv.propagated_exceptions().scalar_forget(var);
    }
  }

  /// \brief Execute an edge from `src` to `dest`
  void exec_edge(ar::BasicBlock* src, ar::BasicBlock* dest) override {
    // Check if the source block ends with an invoke

    if (src->empty()) {
      return;
    }

    ar::Statement* stmt = src->back();
    if (!isa< ar::Invoke >(stmt)) {
      return;
    }

    auto invoke = cast< ar::Invoke >(stmt);
    if (invoke->normal_dest() == dest) {
      this->_inv.enter_normal();
    } else if (invoke->exception_dest() == dest) {
      this->_inv.enter_catch();
    } else {
      ikos_unreachable("unreachable");
    }
  }

  /// \brief Execute an Assignment statement
  ///
  /// Unlike most statements, this propagates uninitialized variables.
  void exec(ar::Assignment* s) override {
    this->init_global_operands(s);

    this->assign(this->_lit_factory.get(s->result()),
                 this->_lit_factory.get(s->operand()));
  }

  /// \brief Execute an UnaryOperation statement
  void exec(ar::UnaryOperation* s) override {
    if (s->has_undefined_constant_operand()) {
      this->_inv.set_normal_flow_to_bottom();
      return;
    }

    this->init_global_operands(s);

    const Literal& lhs = this->_lit_factory.get(s->result());
    const Literal& rhs = this->_lit_factory.get(s->operand());

    switch (s->op()) {
      case ar::UnaryOperation::UTrunc:
      case ar::UnaryOperation::STrunc: {
        this->exec_int_conv(IntUnaryOperator::Trunc,
                            lhs.scalar(),
                            rhs.scalar());
      } break;
      case ar::UnaryOperation::ZExt:
      case ar::UnaryOperation::SExt: {
        this->exec_int_conv(IntUnaryOperator::Ext, lhs.scalar(), rhs.scalar());
      } break;
      case ar::UnaryOperation::FPTrunc:
      case ar::UnaryOperation::FPExt: {
        this->exec_float_conv(lhs.scalar(), rhs.scalar());
      } break;
      case ar::UnaryOperation::FPToUI:
      case ar::UnaryOperation::FPToSI: {
        this->exec_float_to_int_conv(lhs.scalar(), rhs.scalar());
      } break;
      case ar::UnaryOperation::UIToFP:
      case ar::UnaryOperation::SIToFP: {
        this->exec_int_to_float_conv(lhs.scalar(), rhs.scalar());
      } break;
      case ar::UnaryOperation::PtrToUI:
      case ar::UnaryOperation::PtrToSI: {
        this->exec_ptr_to_int_conv(lhs.scalar(), rhs.scalar());
      } break;
      case ar::UnaryOperation::UIToPtr:
      case ar::UnaryOperation::SIToPtr: {
        this->exec_int_to_ptr_conv(lhs.scalar(), rhs.scalar());
      } break;
      case ar::UnaryOperation::Bitcast: {
        this->exec_bitcast(s, lhs, rhs);
      } break;
    }
  }

private:
  /// \brief Execute an integer conversion
  void exec_int_conv(IntUnaryOperator op,
                     const ScalarLit& lhs,
                     const ScalarLit& rhs) {
    ikos_assert_msg(lhs.is_machine_int_var(),
                    "left hand side is not an integer variable");

    if (rhs.is_machine_int()) {
      auto type = cast< ar::IntegerType >(lhs.var()->type());
      this->_inv.normal()
          .int_assign(lhs.var(),
                      core::machine_int::apply_unary_operator(op,
                                                              rhs.machine_int(),
                                                              type->bit_width(),
                                                              type->sign()));
    } else if (rhs.is_machine_int_var()) {
      this->_inv.normal().int_apply(op, lhs.var(), rhs.var());
    } else {
      ikos_unreachable("unexpected arguments");
    }
  }

  /// \brief Execute a floating point conversion
  void exec_float_conv(const ScalarLit& lhs, const ScalarLit& rhs) {
    ikos_assert_msg(lhs.is_floating_point_var(),
                    "left hand side is not a floating point variable");

    if (rhs.is_floating_point_var()) {
      this->_inv.normal().uninit_assert_initialized(rhs.var());
    }
    this->_inv.normal().float_assign_nondet(lhs.var());
  }

  /// \brief Execute a conversion from floating point to integer
  void exec_float_to_int_conv(const ScalarLit& lhs, const ScalarLit& rhs) {
    ikos_assert_msg(lhs.is_machine_int_var(),
                    "left hand side is not an integer variable");

    if (rhs.is_floating_point_var()) {
      this->_inv.normal().uninit_assert_initialized(rhs.var());
    }
    this->_inv.normal().int_assign_nondet(lhs.var());
  }

  /// \brief Execute a conversion from integer to floating point
  void exec_int_to_float_conv(const ScalarLit& lhs, const ScalarLit& rhs) {
    ikos_assert_msg(lhs.is_floating_point_var(),
                    "left hand side is not a floating point variable");

    if (rhs.is_machine_int_var()) {
      this->_inv.normal().uninit_assert_initialized(rhs.var());
    }
    this->_inv.normal().float_assign_nondet(lhs.var());
  }

  /// \brief Execute a conversion from pointer to integer
  void exec_ptr_to_int_conv(const ScalarLit& lhs, const ScalarLit& rhs) {
    ikos_assert_msg(lhs.is_machine_int_var(),
                    "left hand side is not an integer variable");

    if (rhs.is_null()) {
      auto type = cast< ar::IntegerType >(lhs.var()->type());
      auto zero = MachineInt::zero(type->bit_width(), type->sign());
      this->_inv.normal().int_assign(lhs.var(), zero);
    } else if (rhs.is_pointer_var()) {
      this->_inv.normal()
          .scalar_pointer_to_int(lhs.var(),
                                 rhs.var(),
                                 this->_mem_factory.get_absolute_zero());
    } else {
      ikos_unreachable("unreachable");
    }
  }

  /// \brief Execute a conversion from integer to pointer
  void exec_int_to_ptr_conv(const ScalarLit& lhs, const ScalarLit& rhs) {
    if (rhs.is_machine_int()) {
      MachineInt addr = rhs.machine_int();

      if (addr.is_zero()) {
        this->_inv.normal().pointer_assign_null(lhs.var());
      } else {
        addr = addr.cast(this->_data_layout.pointers.bit_width, Unsigned);
        this->_inv.normal()
            .pointer_assign(lhs.var(),
                            this->_mem_factory.get_absolute_zero(),
                            Nullity::non_null());
        this->_inv.normal().pointer_assign(lhs.var(), lhs.var(), addr);
      }
    } else if (rhs.is_machine_int_var()) {
      this->_inv.normal()
          .scalar_int_to_pointer(lhs.var(),
                                 rhs.var(),
                                 this->_mem_factory.get_absolute_zero());
    } else {
      ikos_unreachable("unexpected operand");
    }
  }

  /// \brief Execute a bitcast
  ///
  /// Valid bitcasts are:
  ///   * pointer casts: A* to B*
  ///   * primitive type casts with the same bit-width
  ///
  /// A primitive type is either an integer, a floating point or a vector
  /// of integers or floating points.
  void exec_bitcast(ar::UnaryOperation* s,
                    const Literal& lhs,
                    const Literal& rhs) {
    if (rhs.is_var()) {
      this->_inv.normal().uninit_assert_initialized(rhs.var());
    }

    if (lhs.is_scalar()) {
      this->exec_bitcast(s, lhs.scalar(), rhs);
    } else if (lhs.is_aggregate()) {
      this->exec_bitcast(s, lhs.aggregate(), rhs);
    } else {
      ikos_unreachable("unreachable");
    }
  }

  /// \brief Execute a bitcast with a scalar left hand side
  void exec_bitcast(ar::UnaryOperation* s,
                    const ScalarLit& lhs,
                    const Literal& rhs) {
    if (lhs.is_pointer_var()) {
      // Pointer cast: A* to B*
      this->assign(lhs, rhs.scalar());
    } else if (lhs.is_machine_int_var()) {
      if (rhs.is_scalar() && rhs.scalar().is_machine_int()) {
        // Sign cast: (u|s)iN to (u|s)iN
        auto type = ar::cast< ar::IntegerType >(s->result()->type());
        this->_inv.normal()
            .int_assign(lhs.var(),
                        rhs.scalar().machine_int().cast(type->bit_width(),
                                                        type->sign()));
      } else if (rhs.is_scalar() && rhs.scalar().is_machine_int_var()) {
        // Sign cast: (u|s)iN to (u|s)iN
        this->_inv.normal().int_apply(IntUnaryOperator::SignCast,
                                      lhs.var(),
                                      rhs.scalar().var());
      } else {
        this->_inv.normal().int_assign_nondet(lhs.var());
      }
    } else {
      ikos_unreachable("unexpected left hand side");
    }
  }

  /// \brief Execute a bitcast with an aggregate left hand side
  void exec_bitcast(ar::UnaryOperation* /*s*/,
                    const AggregateLit& lhs,
                    const Literal& rhs) {
    ikos_assert_msg(lhs.is_var(), "left hand side is not a variable");

    if (rhs.is_scalar()) {
      Variable* ptr = this->init_aggregate_memory(lhs);
      this->_inv.normal().mem_forget_reachable(ptr);
    } else if (rhs.is_aggregate()) {
      this->assign(lhs, rhs.aggregate());
    } else {
      ikos_unreachable("unreachable");
    }
  }

public:
  /// \brief Execute a BinaryOperation statement
  void exec(ar::BinaryOperation* s) override {
    if (s->has_undefined_constant_operand()) {
      this->_inv.set_normal_flow_to_bottom();
      return;
    }

    if (s->result()->type()->is_vector()) {
      this->exec_vector_bin_operation(s);
      return;
    }

    const ScalarLit& lhs = this->_lit_factory.get_scalar(s->result());
    const ScalarLit& left = this->_lit_factory.get_scalar(s->left());
    const ScalarLit& right = this->_lit_factory.get_scalar(s->right());

    switch (s->op()) {
      case ar::BinaryOperation::UAdd:
      case ar::BinaryOperation::SAdd: {
        this->exec_int_bin_operation(lhs,
                                     s->has_no_wrap()
                                         ? IntBinaryOperator::AddNoWrap
                                         : IntBinaryOperator::Add,
                                     left,
                                     right);
      } break;
      case ar::BinaryOperation::USub:
      case ar::BinaryOperation::SSub: {
        this->exec_int_bin_operation(lhs,
                                     s->has_no_wrap()
                                         ? IntBinaryOperator::SubNoWrap
                                         : IntBinaryOperator::Sub,
                                     left,
                                     right);
      } break;
      case ar::BinaryOperation::UMul:
      case ar::BinaryOperation::SMul: {
        this->exec_int_bin_operation(lhs,
                                     s->has_no_wrap()
                                         ? IntBinaryOperator::MulNoWrap
                                         : IntBinaryOperator::Mul,
                                     left,
                                     right);
      } break;
      case ar::BinaryOperation::UDiv:
      case ar::BinaryOperation::SDiv: {
        this->exec_int_bin_operation(lhs,
                                     s->is_exact() ? IntBinaryOperator::DivExact
                                                   : IntBinaryOperator::Div,
                                     left,
                                     right);
      } break;
      case ar::BinaryOperation::URem:
      case ar::BinaryOperation::SRem: {
        this->exec_int_bin_operation(lhs, IntBinaryOperator::Rem, left, right);
      } break;
      case ar::BinaryOperation::UShl:
      case ar::BinaryOperation::SShl: {
        this->exec_int_bin_operation(lhs,
                                     s->has_no_wrap()
                                         ? IntBinaryOperator::ShlNoWrap
                                         : IntBinaryOperator::Shl,
                                     left,
                                     right);
      } break;
      case ar::BinaryOperation::ULShr:
      case ar::BinaryOperation::SLShr: {
        this->exec_int_bin_operation(lhs,
                                     s->is_exact()
                                         ? IntBinaryOperator::LShrExact
                                         : IntBinaryOperator::LShr,
                                     left,
                                     right);
      } break;
      case ar::BinaryOperation::UAShr:
      case ar::BinaryOperation::SAShr: {
        this->exec_int_bin_operation(lhs,
                                     s->is_exact()
                                         ? IntBinaryOperator::AShrExact
                                         : IntBinaryOperator::AShr,
                                     left,
                                     right);
      } break;
      case ar::BinaryOperation::UAnd:
      case ar::BinaryOperation::SAnd: {
        this->exec_int_bin_operation(lhs, IntBinaryOperator::And, left, right);
      } break;
      case ar::BinaryOperation::UOr:
      case ar::BinaryOperation::SOr: {
        this->exec_int_bin_operation(lhs, IntBinaryOperator::Or, left, right);
      } break;
      case ar::BinaryOperation::UXor:
      case ar::BinaryOperation::SXor: {
        this->exec_int_bin_operation(lhs, IntBinaryOperator::Xor, left, right);
      } break;
      case ar::BinaryOperation::FAdd:
      case ar::BinaryOperation::FSub:
      case ar::BinaryOperation::FMul:
      case ar::BinaryOperation::FDiv:
      case ar::BinaryOperation::FRem: {
        this->exec_float_bin_operation(lhs, left, right);
      } break;
      default: {
        ikos_unreachable("unreachable");
      }
    }
  }

private:
  /// \brief Execute an integer binary operation
  void exec_int_bin_operation(const ScalarLit& lhs,
                              IntBinaryOperator op,
                              const ScalarLit& left,
                              const ScalarLit& right) {
    ikos_assert_msg(lhs.is_machine_int_var(),
                    "left hand side is not an integer variable");

    if (left.is_machine_int()) {
      if (right.is_machine_int()) {
        this->_inv.normal().int_assign(lhs.var(), left.machine_int());
        this->_inv.normal().int_apply(op,
                                      lhs.var(),
                                      lhs.var(),
                                      right.machine_int());
      } else if (right.is_machine_int_var()) {
        this->_inv.normal().int_apply(op,
                                      lhs.var(),
                                      left.machine_int(),
                                      right.var());
      } else {
        ikos_unreachable("unexpected right operand");
      }
    } else if (left.is_machine_int_var()) {
      if (right.is_machine_int()) {
        this->_inv.normal().int_apply(op,
                                      lhs.var(),
                                      left.var(),
                                      right.machine_int());
      } else if (right.is_machine_int_var()) {
        this->_inv.normal().int_apply(op, lhs.var(), left.var(), right.var());
      } else {
        ikos_unreachable("unexpected right operand");
      }
    } else {
      ikos_unreachable("unexpected left operand");
    }
  }

  /// \brief Execute a floating point binary operation
  void exec_float_bin_operation(const ScalarLit& lhs,
                                const ScalarLit& left,
                                const ScalarLit& right) {
    ikos_assert_msg(lhs.is_floating_point_var(),
                    "left hand side is not a floating point variable");

    // TODO(marthaud): add floating point reasoning

    if (left.is_floating_point_var()) {
      this->_inv.normal().uninit_assert_initialized(left.var());
    }
    if (right.is_floating_point_var()) {
      this->_inv.normal().uninit_assert_initialized(right.var());
    }
    this->_inv.normal().float_assign_nondet(lhs.var());
  }

  /// \brief Execute a vector binary operation
  void exec_vector_bin_operation(ar::BinaryOperation* s) {
    const AggregateLit& lhs = this->_lit_factory.get_aggregate(s->result());
    ikos_assert_msg(lhs.is_var(), "left hand side is not a variable");

    // Ignore the semantic while being sound
    Variable* ptr = this->init_aggregate_memory(lhs);
    this->_inv.normal().mem_forget_reachable(ptr);
  }

public:
  /// \brief Execute a Comparison statement
  void exec(ar::Comparison* s) override {
    if (s->has_undefined_constant_operand()) {
      this->_inv.set_normal_flow_to_bottom();
      return;
    }

    this->init_global_operands(s);

    const ScalarLit& left = this->_lit_factory.get_scalar(s->left());
    const ScalarLit& right = this->_lit_factory.get_scalar(s->right());

    switch (s->predicate()) {
      case ar::Comparison::UIEQ:
      case ar::Comparison::SIEQ: {
        this->exec_int_comparison(IntPredicate::EQ, left, right);
      } break;
      case ar::Comparison::UINE:
      case ar::Comparison::SINE: {
        this->exec_int_comparison(IntPredicate::NE, left, right);
      } break;
      case ar::Comparison::UIGT:
      case ar::Comparison::SIGT: {
        this->exec_int_comparison(IntPredicate::GT, left, right);
      } break;
      case ar::Comparison::UIGE:
      case ar::Comparison::SIGE: {
        this->exec_int_comparison(IntPredicate::GE, left, right);
      } break;
      case ar::Comparison::UILT:
      case ar::Comparison::SILT: {
        this->exec_int_comparison(IntPredicate::LT, left, right);
      } break;
      case ar::Comparison::UILE:
      case ar::Comparison::SILE: {
        this->exec_int_comparison(IntPredicate::LE, left, right);
      } break;
      case ar::Comparison::FOEQ:
      case ar::Comparison::FOGT:
      case ar::Comparison::FOGE:
      case ar::Comparison::FOLT:
      case ar::Comparison::FOLE:
      case ar::Comparison::FONE:
      case ar::Comparison::FORD:
      case ar::Comparison::FUNO:
      case ar::Comparison::FUEQ:
      case ar::Comparison::FUGT:
      case ar::Comparison::FUGE:
      case ar::Comparison::FULT:
      case ar::Comparison::FULE:
      case ar::Comparison::FUNE: {
        this->exec_float_comparison(left, right);
      } break;
      case ar::Comparison::PEQ: {
        this->exec_ptr_comparison(PointerPredicate::EQ, left, right);
      } break;
      case ar::Comparison::PNE: {
        this->exec_ptr_comparison(PointerPredicate::NE, left, right);
      } break;
      case ar::Comparison::PGT: {
        this->exec_ptr_comparison(PointerPredicate::GT, left, right);
      } break;
      case ar::Comparison::PGE: {
        this->exec_ptr_comparison(PointerPredicate::GE, left, right);
      } break;
      case ar::Comparison::PLT: {
        this->exec_ptr_comparison(PointerPredicate::LT, left, right);
      } break;
      case ar::Comparison::PLE: {
        this->exec_ptr_comparison(PointerPredicate::LE, left, right);
      } break;
      default: {
        ikos_unreachable("unreachable");
      }
    }
  }

private:
  /// \brief Execute an integer comparison
  void exec_int_comparison(IntPredicate pred,
                           const ScalarLit& left,
                           const ScalarLit& right) {
    if (left.is_machine_int()) {
      if (right.is_machine_int()) {
        if (!compare(pred, left.machine_int(), right.machine_int())) {
          this->_inv.set_normal_flow_to_bottom();
        }
      } else if (right.is_machine_int_var()) {
        this->_inv.normal().int_add(pred, left.machine_int(), right.var());
      } else {
        ikos_unreachable("unexpected right operand");
      }
    } else if (left.is_machine_int_var()) {
      if (right.is_machine_int()) {
        this->_inv.normal().int_add(pred, left.var(), right.machine_int());
      } else if (right.is_machine_int_var()) {
        this->_inv.normal().int_add(pred, left.var(), right.var());
      } else {
        ikos_unreachable("unexpected right operand");
      }
    } else {
      ikos_unreachable("unexpected left operand");
    }
  }

  /// \brief Execute a floating point comparison
  void exec_float_comparison(const ScalarLit& left, const ScalarLit& right) {
    // TODO(marthaud): add floating point reasoning

    if (left.is_floating_point_var()) {
      this->_inv.normal().uninit_assert_initialized(left.var());
    }
    if (right.is_floating_point_var()) {
      this->_inv.normal().uninit_assert_initialized(right.var());
    }
  }

  /// \brief Execute a pointer comparison
  void exec_ptr_comparison(PointerPredicate pred,
                           const ScalarLit& left,
                           const ScalarLit& right) {
    if (left.is_null()) {
      if (right.is_null()) {
        // Compare `null pred null`
        if (pred == PointerPredicate::NE || pred == PointerPredicate::GT ||
            pred == PointerPredicate::LT) {
          this->_inv.set_normal_flow_to_bottom();
        }
      } else if (right.is_pointer_var()) {
        // Compare `null pred p`
        this->refine_addresses(right.var());

        if (pred == PointerPredicate::EQ) {
          this->_inv.normal().nullity_assert_null(right.var());
        } else if (pred == PointerPredicate::NE ||
                   pred == PointerPredicate::GT ||
                   pred == PointerPredicate::LT) {
          this->_inv.normal().nullity_assert_non_null(right.var());
        } else {
          this->_inv.normal().uninit_assert_initialized(right.var());
        }
      } else {
        ikos_unreachable("unexpected right operand");
      }
    } else if (left.is_pointer_var()) {
      if (right.is_null()) {
        // Compare `p pred null`
        this->refine_addresses(left.var());

        if (pred == PointerPredicate::EQ) {
          this->_inv.normal().nullity_assert_null(left.var());
        } else if (pred == PointerPredicate::NE ||
                   pred == PointerPredicate::GT ||
                   pred == PointerPredicate::LT) {
          this->_inv.normal().nullity_assert_non_null(left.var());
        } else {
          this->_inv.normal().uninit_assert_initialized(left.var());
        }
      } else if (right.is_pointer_var()) {
        // Compare `p pred q`

        // Reduction with the external pointer analysis
        this->refine_addresses_offset(left.var());
        this->refine_addresses_offset(right.var());

        this->_inv.normal().pointer_add(pred, left.var(), right.var());
      } else {
        ikos_unreachable("unexpected right operand");
      }
    } else {
      ikos_unreachable("unexpected left operand");
    }
  }

public:
  /// \brief Execute an Unreachable statement
  void exec(ar::Unreachable*) override {
    // Unreachable propagates exceptions
    this->_inv.set_normal_flow_to_bottom();
  }

  /// \brief Execute an Allocate statement
  void exec(ar::Allocate* s) override {
    if (s->has_undefined_constant_operand()) {
      this->_inv.set_normal_flow_to_bottom();
      return;
    }

    const ScalarLit& lhs = this->_lit_factory.get_scalar(s->result());
    const ScalarLit& array_size =
        this->_lit_factory.get_scalar(s->array_size());
    ikos_assert_msg(lhs.is_pointer_var(),
                    "left hand side is not a pointer variable");

    // Allocate the memory
    MemoryLocation* addr = this->_mem_factory.get_local(s->result());
    this->allocate_memory(lhs.var(),
                          addr,
                          Nullity::non_null(),
                          Lifetime::allocated(),
                          MemoryInitialValue::Uninitialized);

    if (this->_opts.test(ExecutionEngine::UpdateAllocSizeVar)) {
      // Set the allocation size variable
      Variable* alloc_size_var = this->_var_factory.get_alloc_size(addr);
      auto element_size = MachineInt(this->_data_layout.alloc_size_in_bytes(
                                         s->allocated_type()),
                                     this->_data_layout.pointers.bit_width,
                                     Unsigned);
      if (array_size.is_machine_int()) {
        bool overflow;
        MachineInt alloc_size_int =
            mul(array_size.machine_int(), element_size, overflow);
        if (overflow) {
          this->_inv.set_normal_flow_to_bottom(); // undefined behavior
        } else {
          this->_inv.normal().int_assign(alloc_size_var, alloc_size_int);

          // When the size of the allocation is known (like it is here)
          // we mark the storage as uninitialized by assigning it
          // the undefined value.
          this->_inv.normal().mem_write(lhs.var(),
                                        ScalarLit::undefined(),
                                        alloc_size_int);
        }
      } else if (array_size.is_machine_int_var()) {
        this->_inv.normal().int_apply(IntBinaryOperator::MulNoWrap,
                                      alloc_size_var,
                                      array_size.var(),
                                      element_size);
      } else {
        ikos_unreachable("unexpected array size parameter");
      }
    }
  }

  /// \brief Execute a PointerShift statement
  void exec(ar::PointerShift* s) override {
    if (s->has_undefined_constant_operand()) {
      this->_inv.set_normal_flow_to_bottom();
      return;
    }

    this->init_global_operands(s);

    const ScalarLit& lhs = this->_lit_factory.get_scalar(s->result());
    const ScalarLit& base = this->_lit_factory.get_scalar(s->pointer());
    ikos_assert_msg(lhs.is_pointer_var(),
                    "left hand side is not a pointer variable");
    ikos_assert_msg(base.is_null() || base.is_pointer_var(),
                    "unexpected base operand");

    // Build a linear expression of the operands
    uint64_t bit_width = this->_data_layout.pointers.bit_width;
    auto zero = MachineInt::zero(bit_width, Unsigned);
    auto offset_expr = IntLinearExpression(zero);

    for (auto it = s->term_begin(), et = s->term_end(); it != et; ++it) {
      auto term = *it;
      const ScalarLit& offset = this->_lit_factory.get_scalar(term.second);

      if (offset.is_machine_int()) {
        offset_expr.add(
            mul(term.first, offset.machine_int().cast(bit_width, Unsigned)));
      } else if (offset.is_machine_int_var()) {
        offset_expr.add(term.first, offset.var());
      } else {
        ikos_unreachable("unexpected offset operand");
      }
    }

    if (base.is_null()) {
      this->_inv.normal().pointer_assign(lhs.var(),
                                         this->_mem_factory.get_absolute_zero(),
                                         Nullity::null());
      this->_inv.normal().pointer_assign(lhs.var(), lhs.var(), offset_expr);
    } else {
      this->_inv.normal().pointer_assign(lhs.var(), base.var(), offset_expr);
    }

    this->normalize_absolute_zero_nullity(lhs.var());
  }

  /// \brief Execute a Load statement
  ///
  /// Reading uninitialized memory is an error.
  void exec(ar::Load* s) override {
    if (s->has_undefined_constant_operand()) {
      this->_inv.set_normal_flow_to_bottom();
      return;
    }

    this->init_global_operands(s);

    const ScalarLit& ptr = this->_lit_factory.get_scalar(s->operand());

    if (!this->prepare_mem_access(ptr)) {
      return;
    }

    const Literal& result = this->_lit_factory.get(s->result());

    auto size =
        MachineInt(this->_data_layout.store_size_in_bytes(s->result()->type()),
                   this->_data_layout.pointers.bit_width,
                   Unsigned);

    if (result.is_scalar()) {
      const ScalarLit& lhs = result.scalar();
      ikos_assert_msg(lhs.is_var(), "left hand side is not a variable");

      if (!s->is_volatile()) {
        // Perform memory read in the value domain
        this->_inv.normal().mem_read(lhs, ptr.var(), size);
      } else {
        this->_inv.normal().scalar_assign_nondet(lhs.var());
      }

      // Reduction between value and pointer analysis
      if (lhs.is_pointer_var()) {
        this->refine_addresses_offset(lhs.var());
      }
    } else if (result.is_aggregate()) {
      const AggregateLit& lhs = result.aggregate();
      ikos_assert_msg(lhs.is_var(), "left hand side is not a variable");

      Variable* lhs_ptr = this->init_aggregate_memory(lhs);

      if (!s->is_volatile()) {
        // Perform memory read in the value domain
        this->_inv.normal().mem_copy(lhs_ptr,
                                     ptr.var(),
                                     ScalarLit::machine_int(size));
      } else {
        this->_inv.normal().mem_forget_reachable(lhs_ptr);
      }
    } else {
      ikos_unreachable("unexpected left hand side");
    }
  }

  /// \brief Execute a Store statement
  ///
  /// Writing an uninitialized variable is an error.
  void exec(ar::Store* s) override {
    if (s->has_undefined_constant_operand()) {
      this->_inv.set_normal_flow_to_bottom();
      return;
    }

    this->init_global_operands(s);

    const ScalarLit& ptr = this->_lit_factory.get_scalar(s->pointer());

    if (!this->prepare_mem_access(ptr)) {
      return;
    }

    if (this->_inv.normal().pointer_to_points_to(ptr.var()).is_top()) {
      // Ignore memory write, analysis could be unsound.
      // See CheckKind::IgnoredStore
      return;
    }

    const Literal& val = this->_lit_factory.get(s->value());

    auto size =
        MachineInt(this->_data_layout.store_size_in_bytes(s->value()->type()),
                   this->_data_layout.pointers.bit_width,
                   Unsigned);

    if (val.is_scalar()) {
      const ScalarLit& rhs = val.scalar();

      if (rhs.is_pointer_var()) {
        this->refine_addresses_offset(rhs.var());
      }

      // Perform memory write in the value domain
      this->_inv.normal().mem_write(ptr.var(), rhs, size);
    } else if (val.is_aggregate()) {
      this->mem_write_aggregate(ptr.var(), val.aggregate());
    } else {
      ikos_unreachable("unexpected right hand side");
    }
  }

  /// \brief Execute an ExtractElement statement
  void exec(ar::ExtractElement* s) override {
    if (s->has_undefined_constant_operand()) {
      this->_inv.set_normal_flow_to_bottom();
      return;
    }

    this->init_global_operands(s);

    const Literal& lhs = this->_lit_factory.get(s->result());
    const AggregateLit& rhs = this->_lit_factory.get_aggregate(s->aggregate());
    const ScalarLit& offset = this->_lit_factory.get_scalar(s->offset());
    ikos_assert_msg(rhs.is_var(), "right hand side is not a variable");

    Variable* rhs_ptr = this->aggregate_pointer(rhs);
    Variable* read_ptr =
        this->_var_factory.get_named_shadow(this->void_ptr_type(),
                                            "shadow.extract_element.ptr");
    if (offset.is_machine_int_var()) {
      this->_inv.normal().pointer_assign(read_ptr, rhs_ptr, offset.var());
    } else if (offset.is_machine_int()) {
      this->_inv.normal().pointer_assign(read_ptr,
                                         rhs_ptr,
                                         offset.machine_int());
    } else {
      ikos_unreachable("unexpected offset operand");
    }

    auto size =
        MachineInt(this->_data_layout.store_size_in_bytes(s->result()->type()),
                   this->_data_layout.pointers.bit_width,
                   Unsigned);

    if (lhs.is_scalar()) {
      ikos_assert_msg(lhs.scalar().is_var(),
                      "left hand side is not a variable");

      this->_inv.normal().mem_read(lhs.scalar(), read_ptr, size);
    } else if (lhs.is_aggregate()) {
      ikos_assert_msg(lhs.aggregate().is_var(),
                      "left hand side is not a variable");

      Variable* lhs_ptr = this->init_aggregate_memory(lhs.aggregate());
      this->_inv.normal().mem_copy(lhs_ptr,
                                   read_ptr,
                                   ScalarLit::machine_int(size));
    } else {
      ikos_unreachable("unexpected left hand side");
    }

    // Clean-up
    this->_inv.normal().pointer_forget(read_ptr);
  }

  /// \brief Execute an InsertElement statement
  ///
  /// Unlike most statements, this accepts undefined aggregate operands
  void exec(ar::InsertElement* s) override {
    if (s->offset()->is_undefined_constant() ||
        s->element()->is_undefined_constant()) {
      this->_inv.set_normal_flow_to_bottom();
      return;
    }

    this->init_global_operands(s);

    const AggregateLit& lhs = this->_lit_factory.get_aggregate(s->result());
    const AggregateLit& rhs = this->_lit_factory.get_aggregate(s->aggregate());
    const ScalarLit& offset = this->_lit_factory.get_scalar(s->offset());
    const Literal& element = this->_lit_factory.get(s->element());
    ikos_assert_msg(lhs.is_var(), "left hand side is not a variable");

    Variable* lhs_ptr = this->init_aggregate_memory(lhs);

    // First, copy the aggregate value
    this->mem_write_aggregate(lhs_ptr, rhs);

    // Then, insert the element
    Variable* write_ptr =
        this->_var_factory.get_named_shadow(this->void_ptr_type(),
                                            "shadow.insert_element.ptr");
    if (offset.is_machine_int_var()) {
      this->_inv.normal().pointer_assign(write_ptr, lhs_ptr, offset.var());
    } else if (offset.is_machine_int()) {
      this->_inv.normal().pointer_assign(write_ptr,
                                         lhs_ptr,
                                         offset.machine_int());
    } else {
      ikos_unreachable("unexpected offset operand");
    }

    auto size =
        MachineInt(this->_data_layout.store_size_in_bytes(s->element()->type()),
                   this->_data_layout.pointers.bit_width,
                   Unsigned);

    if (element.is_scalar()) {
      this->_inv.normal().mem_write(write_ptr, element.scalar(), size);
    } else if (element.is_aggregate()) {
      this->mem_write_aggregate(write_ptr, element.aggregate());
    } else {
      ikos_unreachable("unexpected element operand");
    }

    // Clean-up
    this->_inv.normal().pointer_forget(write_ptr);
  }

  /// \brief Execute a ShuffleVector statement
  void exec(ar::ShuffleVector* s) override {
    if (s->has_undefined_constant_operand()) {
      this->_inv.set_normal_flow_to_bottom();
      return;
    }

    this->init_global_operands(s);

    const AggregateLit& lhs = this->_lit_factory.get_aggregate(s->result());
    ikos_assert_msg(lhs.is_var(), "left hand side is not a variable");

    // Ignore the semantic while being sound
    Variable* ptr = this->init_aggregate_memory(lhs);
    this->_inv.normal().mem_forget_reachable(ptr);
  }

  /// \brief Execute a LandingPad statement
  void exec(ar::LandingPad*) override {}

  /// \brief Execute a Resume statement
  void exec(ar::Resume* s) override {
    if (s->has_undefined_constant_operand()) {
      this->_inv.set_normal_flow_to_bottom();
      return;
    }

    this->_inv.resume_exception();
  }

  /// @}
  /// \name Execute call statements
  /// @{

  /// \brief Execute a call to the given extern function
  void exec_extern_call(ar::CallBase* call, ar::Function* fun) override {
    ikos_assert(fun->is_declaration());
    ikos_assert(ar::TypeVerifier::is_valid_call(call, fun->type()));

    if (fun->is_intrinsic()) {
      this->exec_intrinsic_call(call, fun->intrinsic_id());
    } else {
      this->exec_unknown_extern_call(call);
    }
  }

  /// \brief Execute a call to the given intrinsic function
  void exec_intrinsic_call(ar::CallBase* call, ar::Intrinsic::ID id) override {
    this->_inv.normal().normalize();

    if (this->_inv.is_normal_flow_bottom()) {
      return;
    }

    if (id == ar::Intrinsic::IkosPartitioningVar) {
      // Unlike most functions, it propagates uninitialized variables
      this->exec_ikos_partitioning_var(call);
      return;
    }

    // Check for uninitialized variables
    for (auto it = call->op_begin(), et = call->op_end(); it != et; ++it) {
      ar::Value* op = *it;

      if (isa< ar::UndefinedConstant >(op)) {
        this->_inv.set_normal_flow_to_bottom();
        return;
      } else if (auto iv = dyn_cast< ar::InternalVariable >(op)) {
        Variable* var = this->_var_factory.get_internal(iv);
        this->_inv.normal().uninit_assert_initialized(var);
      }
    }

    this->_inv.normal().normalize();

    if (this->_inv.is_normal_flow_bottom()) {
      return;
    }

    switch (id) {
      case ar::Intrinsic::MemoryCopy:
      case ar::Intrinsic::MemoryMove: {
        this->exec_memcpy_or_memmove(call);
      } break;
      case ar::Intrinsic::MemorySet: {
        this->exec_memset(call);
      } break;
      case ar::Intrinsic::VarArgStart:
      case ar::Intrinsic::VarArgEnd:
      case ar::Intrinsic::VarArgGet:
      case ar::Intrinsic::VarArgCopy: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ true,
                                /* ignore_unknown_write = */ true,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::StackSave:
      case ar::Intrinsic::StackRestore: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LifetimeStart:
      case ar::Intrinsic::LifetimeEnd: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::EhTypeidFor: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::Trap: {
        this->exec_abort(call);
      } break;
      // <ikos/analyzer/intrinsic.h>
      case ar::Intrinsic::IkosAssert:
      case ar::Intrinsic::IkosAssume:
      case ar::Intrinsic::IkosNonDet: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::IkosCounterInit: {
        this->exec_ikos_counter_init(call);
      } break;
      case ar::Intrinsic::IkosCounterIncr: {
        this->exec_ikos_counter_incr(call);
      } break;
      case ar::Intrinsic::IkosCheckMemAccess:
      case ar::Intrinsic::IkosCheckStringAccess: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::IkosAssumeMemSize: {
        this->exec_ikos_assume_mem_size(call);
      } break;
      case ar::Intrinsic::IkosForgetMemory: {
        this->exec_ikos_forget_memory(call);
      } break;
      case ar::Intrinsic::IkosAbstractMemory: {
        this->exec_ikos_abstract_memory(call);
      } break;
      case ar::Intrinsic::IkosWatchMemory: {
        this->exec_ikos_watch_memory(call);
      } break;
      case ar::Intrinsic::IkosPartitioningVar: {
        this->exec_ikos_partitioning_var(call);
      } break;
      case ar::Intrinsic::IkosPartitioningJoin: {
        this->exec_ikos_partitioning_join(call);
      } break;
      case ar::Intrinsic::IkosPartitioningDisable: {
        this->exec_ikos_partitioning_disable(call);
      } break;
      case ar::Intrinsic::IkosPrintInvariant:
      case ar::Intrinsic::IkosPrintValues: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      // <stdlib.h>
      case ar::Intrinsic::LibcMalloc: {
        this->exec_malloc(call);
      } break;
      case ar::Intrinsic::LibcCalloc: {
        this->exec_calloc(call);
      } break;
      case ar::Intrinsic::LibcValloc: {
        this->exec_valloc(call);
      } break;
      case ar::Intrinsic::LibcAlignedAlloc: {
        this->exec_aligned_alloc(call);
      } break;
      case ar::Intrinsic::LibcRealloc: {
        this->exec_realloc(call);
      } break;
      case ar::Intrinsic::LibcFree: {
        this->exec_free(call);
      } break;
      case ar::Intrinsic::LibcAbs: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LibcRand:
      case ar::Intrinsic::LibcSrand: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LibcExit: {
        this->exec_exit(call);
      } break;
      case ar::Intrinsic::LibcAbort: {
        this->exec_abort(call);
      } break;
      // <errno.h>
      case ar::Intrinsic::LibcErrnoLocation: {
        this->exec_errno_location(call);
      } break;
      // <fcntl.h>
      case ar::Intrinsic::LibcOpen: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      // <unistd.h>
      case ar::Intrinsic::LibcClose: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LibcRead: {
        this->exec_read(call);
      } break;
      case ar::Intrinsic::LibcWrite: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      // <stdio.h>
      case ar::Intrinsic::LibcGets: {
        this->exec_gets(call);
      } break;
      case ar::Intrinsic::LibcFgets: {
        this->exec_fgets(call);
      } break;
      case ar::Intrinsic::LibcGetc: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LibcFgetc: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LibcGetchar: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LibcPuts: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LibcFputs: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LibcPutc: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LibcFputc: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LibcPrintf: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LibcFprintf: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LibcSprintf: {
        this->exec_sprintf(call);
      } break;
      case ar::Intrinsic::LibcSnprintf: {
        this->exec_snprintf(call);
      } break;
      case ar::Intrinsic::LibcScanf: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ true,
                                /* ignore_unknown_write = */ true,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LibcFscanf: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ true,
                                /* ignore_unknown_write = */ true,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LibcSscanf: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ true,
                                /* ignore_unknown_write = */ true,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LibcFopen: {
        this->exec_fopen(call);
      } break;
      case ar::Intrinsic::LibcFclose: {
        this->exec_fclose(call);
      } break;
      case ar::Intrinsic::LibcFflush: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      // <string.h>
      case ar::Intrinsic::LibcStrlen: {
        this->exec_strlen(call);
      } break;
      case ar::Intrinsic::LibcStrnlen: {
        this->exec_strnlen(call);
      } break;
      case ar::Intrinsic::LibcStrcpy: {
        this->exec_strcpy(call);
      } break;
      case ar::Intrinsic::LibcStrncpy: {
        this->exec_strncpy(call);
      } break;
      case ar::Intrinsic::LibcStrcat: {
        this->exec_strcat(call);
      } break;
      case ar::Intrinsic::LibcStrncat: {
        this->exec_strncat(call);
      } break;
      case ar::Intrinsic::LibcStrcmp: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LibcStrncmp: {
        this->exec_unknown_call(call,
                                /* may_write_params = */ false,
                                /* ignore_unknown_write = */ false,
                                /* may_write_globals = */ false,
                                /* may_throw_exc = */ false);
      } break;
      case ar::Intrinsic::LibcStrstr: {
        this->exec_strstr(call);
      } break;
      case ar::Intrinsic::LibcStrchr: {
        this->exec_strchr(call);
      } break;
      case ar::Intrinsic::LibcStrdup: {
        this->exec_strdup(call);
      } break;
      case ar::Intrinsic::LibcStrndup: {
        this->exec_strndup(call);
      } break;
      case ar::Intrinsic::LibcStrcpyCheck: {
        this->exec_strcpy(call);
      } break;
      case ar::Intrinsic::LibcMemoryCopyCheck: {
        this->exec_memcpy_or_memmove(call);
      } break;
      case ar::Intrinsic::LibcMemoryMoveCheck: {
        this->exec_memcpy_or_memmove(call);
      } break;
      case ar::Intrinsic::LibcMemorySetCheck: {
        this->exec_memset(call);
      } break;
      case ar::Intrinsic::LibcStrcatCheck: {
        this->exec_strcat(call);
      } break;
      case ar::Intrinsic::LibcppNew:
      case ar::Intrinsic::LibcppNewArray: {
        this->exec_new(call);
      } break;
      case ar::Intrinsic::LibcppDelete: {
        this->exec_free(call);
      } break;
      case ar::Intrinsic::LibcppDeleteArray: {
        // TODO(marthaud): delete[] also calls the destructor on each element
        this->exec_free(call);
      } break;
      case ar::Intrinsic::LibcppAllocateException: {
        this->exec_allocate_exception(call);
      } break;
      case ar::Intrinsic::LibcppFreeException: {
        this->exec_free(call);
      } break;
      case ar::Intrinsic::LibcppThrow: {
        this->exec_throw(call);
      } break;
      case ar::Intrinsic::LibcppBeginCatch: {
        this->exec_begin_catch(call);
      } break;
      case ar::Intrinsic::LibcppEndCatch: {
        this->exec_end_catch(call);
      } break;
      default: {
        ikos_unreachable("unreachable");
      } break;
    }
  }

  /// \brief Execute a call to an unknown extern function
  void exec_unknown_extern_call(ar::CallBase* call) override {
    this->exec_unknown_call(call,
                            /* may_write_params = */ true,
                            /* ignore_unknown_write = */ true,
                            /* may_write_globals = */ false,
                            /* may_throw_exc = */ true);
  }

  /// \brief Execute a call to an unknown internal function
  void exec_unknown_intern_call(ar::CallBase* call) override {
    this->exec_unknown_call(call,
                            /* may_write_params = */ true,
                            /* ignore_unknown_write = */ false,
                            /* may_write_globals = */ true,
                            /* may_throw_exc = */ true);
  }

  /// \brief Execute a call to an unknown function
  ///
  /// \param call
  ///   The call statement
  /// \param may_write_params
  ///   True if the function call might write on a pointer parameter
  /// \param ignore_unknown_write
  ///   True to ignore writes on unknown pointer parameters (unsound)
  /// \param may_write_globals
  ///   True if the function call might update a global variable
  /// \param may_throw_exc
  ///   True if the function call might throw an exception
  void exec_unknown_call(ar::CallBase* call,
                         bool may_write_params,
                         bool ignore_unknown_write,
                         bool may_write_globals,
                         bool may_throw_exc) override {
    this->_inv.normal().normalize();

    if (this->_inv.is_normal_flow_bottom()) {
      return;
    }

    // Check for uninitialized variables
    for (auto it = call->op_begin(), et = call->op_end(); it != et; ++it) {
      ar::Value* op = *it;

      if (isa< ar::UndefinedConstant >(op)) {
        this->_inv.set_normal_flow_to_bottom();
        return;
      } else if (auto iv = dyn_cast< ar::InternalVariable >(op)) {
        Variable* var = this->_var_factory.get_internal(iv);
        this->_inv.normal().uninit_assert_initialized(var);
      }
    }

    this->_inv.normal().normalize();

    if (this->_inv.is_normal_flow_bottom()) {
      return;
    }

    if (may_write_globals) {
      // Forget all memory contents
      this->_inv.normal().mem_forget_all();
    } else if (may_write_params) {
      // Forget all memory contents pointed by pointer parameters
      for (auto it = call->arg_begin(), et = call->arg_end(); it != et; ++it) {
        ar::Value* arg = *it;

        if (!isa< ar::InternalVariable >(arg) ||
            !isa< ar::PointerType >(arg->type())) {
          continue;
        }

        auto iv = cast< ar::InternalVariable >(arg);
        Variable* ptr = this->_var_factory.get_internal(iv);

        this->init_global_operand(arg);
        this->refine_addresses(ptr);

        if (this->_inv.normal().nullity_is_null(ptr)) {
          continue; // Safe
        } else if (ignore_unknown_write &&
                   this->_inv.normal().pointer_to_points_to(ptr).is_top()) {
          // Ignore side effect on the memory
          // See CheckKind::IgnoredCallSideEffectOnPointerParameter
          continue;
        } else {
          this->_inv.normal().mem_forget_reachable(ptr);
        }
      }
    }

    if (may_throw_exc) {
      // The call can throw exceptions
      this->throw_unknown_exceptions();
    }

    // ASSUMPTION:
    // The claim about the correctness of the program under analysis can be
    // made only if all calls to unavailable code are assumed to be correct
    // and without side-effects. We will assume that the lhs of an external
    // call site is always initialized. However, in case of a pointer, we do
    // not assume that a non-null pointer is returned.
    if (call->has_result()) {
      // Forget the result
      const Literal& ret = this->_lit_factory.get(call->result());

      if (ret.is_scalar()) {
        ikos_assert_msg(ret.scalar().is_var(),
                        "left hand side is not a variable");
        this->_inv.normal().scalar_assign_nondet(ret.scalar().var());
      } else if (ret.is_aggregate()) {
        ikos_assert_msg(ret.aggregate().is_var(),
                        "left hand side is not a variable");
        Variable* ret_ptr = this->aggregate_pointer(ret.aggregate());
        this->_inv.normal().mem_forget_reachable(ret_ptr);
      } else {
        ikos_unreachable("unexpected left hand side");
      }
    }
  }

private:
  /// @}
  /// \name Execution of intrinsic functions
  /// @{

  /// \brief Execute a call to memcpy(dest, src, len) or memmove(dest, src, len)
  void exec_memcpy_or_memmove(ar::CallBase* call) {
    this->init_global_operands(call);

    // Both src and dest must be already allocated in memory so offsets and
    // sizes for both src and dest are already part of the invariants
    const ScalarLit& dest = this->_lit_factory.get_scalar(call->argument(0));
    const ScalarLit& src = this->_lit_factory.get_scalar(call->argument(1));
    const ScalarLit& size = this->_lit_factory.get_scalar(call->argument(2));

    if (!this->prepare_mem_access(src)) {
      return;
    }
    if (!this->prepare_mem_access(dest)) {
      return;
    }

    if (this->_inv.normal().pointer_to_points_to(dest.var()).is_top()) {
      // Ignore memory copy/move, analysis could be unsound.
      // See CheckKind::IgnoredMemoryCopy, CheckKind::IgnoredMemoryMove
    } else if (cast< ar::IntegerConstant >(call->argument(5))->value() == 0) {
      // Non-volatile
      this->_inv.normal().mem_copy(dest.var(), src.var(), size);
    } else {
      // Volatile
      if (size.is_machine_int()) {
        this->_inv.normal().mem_forget_reachable(dest.var(),
                                                 size.machine_int());
      } else if (size.is_machine_int_var()) {
        IntInterval size_intv = this->_inv.normal().int_to_interval(size.var());
        this->_inv.normal().mem_forget_reachable(dest.var(), size_intv.ub());
      } else {
        ikos_unreachable("unreachable");
      }
    }

    if (call->has_result()) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
      ikos_assert_msg(lhs.is_pointer_var(),
                      "left hand side is not a pointer variable");
      this->assign(lhs, dest);
    }
  }

  /// \brief Execute a call to memset(dest, byte, len)
  void exec_memset(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& dest = this->_lit_factory.get_scalar(call->argument(0));
    const ScalarLit& value = this->_lit_factory.get_scalar(call->argument(1));
    const ScalarLit& size = this->_lit_factory.get_scalar(call->argument(2));

    ikos_assert_msg(value.is_machine_int_var() || value.is_machine_int(),
                    "unexpected value operand");
    ikos_assert_msg(size.is_machine_int_var() || size.is_machine_int(),
                    "unexpected size operand");

    if (!this->prepare_mem_access(dest)) {
      return;
    }

    if (this->_inv.normal().pointer_to_points_to(dest.var()).is_top()) {
      // Ignore memory set, analysis could be unsound.
      // See CheckKind::IgnoredMemorySet
    } else {
      this->_inv.normal().mem_set(dest.var(), value, size);
    }

    if (call->has_result()) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
      ikos_assert_msg(lhs.is_pointer_var(),
                      "left hand side is not a pointer variable");
      this->assign(lhs, dest);
    }
  }

  /// \brief Execute a call to ikos.counter.init
  void exec_ikos_counter_init(ar::CallBase* call) {
    ikos_assert(call->has_result());
    ikos_assert(call->num_arguments() == 1);

    const ScalarLit& ret = this->_lit_factory.get_scalar(call->result());
    const ScalarLit& init = this->_lit_factory.get_scalar(call->argument(0));

    ikos_assert_msg(ret.is_machine_int_var(),
                    "left hand side is not an integer variable");
    ikos_assert_msg(init.is_machine_int(), "operand is not a machine integer");

    this->_inv.normal().counter_init(ret.var(), init.machine_int());
  }

  /// \brief Execute a call to ikos.counter.incr
  void exec_ikos_counter_incr(ar::CallBase* call) {
    ikos_assert(call->has_result());
    ikos_assert(call->num_arguments() == 2);
    ikos_assert(call->result() == call->argument(0));

    const ScalarLit& ret = this->_lit_factory.get_scalar(call->result());
    const ScalarLit& incr = this->_lit_factory.get_scalar(call->argument(1));

    ikos_assert_msg(ret.is_machine_int_var(),
                    "left hand side is not an integer variable");
    ikos_assert_msg(incr.is_machine_int(), "operand is not a machine integer");

    this->_inv.normal().counter_incr(ret.var(), incr.machine_int());
  }

  /// \brief Execute a call to ikos.assume_mem_size
  void exec_ikos_assume_mem_size(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& ptr = this->_lit_factory.get_scalar(call->argument(0));
    const ScalarLit& size = this->_lit_factory.get_scalar(call->argument(1));

    if (!this->prepare_mem_access(ptr)) {
      return;
    }

    if (!this->_opts.test(ExecutionEngine::UpdateAllocSizeVar)) {
      return;
    }

    PointsToSet addrs = this->_inv.normal().pointer_to_points_to(ptr.var());

    if (addrs.is_bottom()) {
      return;
    } else if (addrs.is_top()) {
      // Ignore (this is sound)
      return;
    }

    for (auto addr : addrs) {
      Variable* alloc_size_var = this->_var_factory.get_alloc_size(addr);

      if (size.is_machine_int()) {
        this->_inv.normal().int_assign(alloc_size_var, size.machine_int());
      } else if (size.is_machine_int_var()) {
        this->_inv.normal().int_assign(alloc_size_var, size.var());
      } else {
        ikos_unreachable("unreachable");
      }
    }
  }

  /// \brief Execute a call to ikos.forget_memory
  void exec_ikos_forget_memory(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& ptr = this->_lit_factory.get_scalar(call->argument(0));
    const ScalarLit& size = this->_lit_factory.get_scalar(call->argument(1));

    if (!this->prepare_mem_access(ptr)) {
      return;
    }

    if (this->_inv.normal().pointer_to_points_to(ptr.var()).is_top()) {
      // Ignore ikos.forget_memory, analysis could be unsound.
      // See CheckKind::UnknownMemoryAccess
    } else if (size.is_machine_int()) {
      this->_inv.normal().mem_forget_reachable(ptr.var(), size.machine_int());
    } else if (size.is_machine_int_var()) {
      IntInterval size_intv = this->_inv.normal().int_to_interval(size.var());
      this->_inv.normal().mem_forget_reachable(ptr.var(), size_intv.ub());
    } else {
      ikos_unreachable("unreachable");
    }
  }

  /// \brief Execute a call to ikos.abstract_memory
  void exec_ikos_abstract_memory(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& ptr = this->_lit_factory.get_scalar(call->argument(0));
    const ScalarLit& size = this->_lit_factory.get_scalar(call->argument(1));

    if (!this->prepare_mem_access(ptr)) {
      return;
    }

    if (this->_inv.normal().pointer_to_points_to(ptr.var()).is_top()) {
      // Ignore ikos.abstract_memory, analysis could be unsound.
      // See CheckKind::UnknownMemoryAccess
    } else if (size.is_machine_int()) {
      this->_inv.normal().mem_abstract_reachable(ptr.var(), size.machine_int());
    } else if (size.is_machine_int_var()) {
      IntInterval size_intv = this->_inv.normal().int_to_interval(size.var());
      this->_inv.normal().mem_abstract_reachable(ptr.var(), size_intv.ub());
    } else {
      ikos_unreachable("unreachable");
    }
  }

  /// \brief Execute a call to ikos.watch_memory
  void exec_ikos_watch_memory(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& ptr = this->_lit_factory.get_scalar(call->argument(0));
    const ScalarLit& size = this->_lit_factory.get_scalar(call->argument(1));

    if (!this->prepare_mem_access(ptr)) {
      return;
    }

    // Save the watched pointer
    Variable* watch_mem_ptr =
        this->_var_factory.get_named_shadow(this->void_ptr_type(),
                                            "shadow.watch_mem.ptr");
    this->_inv.normal().pointer_assign(watch_mem_ptr, ptr.var());

    // Save the watched size
    Variable* watch_mem_size =
        this->_var_factory.get_named_shadow(ar::IntegerType::size_type(
                                                this->_ctx.bundle),
                                            "shadow.watch_mem.size");
    if (size.is_machine_int()) {
      this->_inv.normal().int_assign(watch_mem_size, size.machine_int());
    } else if (size.is_machine_int_var()) {
      this->_inv.normal().int_assign(watch_mem_size, size.var());
    } else {
      ikos_unreachable("unexpected size parameter");
    }
  }

  /// \brief Execute a call to ikos.partitioning.var.si32
  void exec_ikos_partitioning_var(ar::CallBase* call) {
    const ScalarLit& arg = this->_lit_factory.get_scalar(call->argument(0));

    if (arg.is_var()) {
      this->_inv.normal().partitioning_set_variable(arg.var());
    }
  }

  /// \brief Execute a call to ikos.partitioning.join
  void exec_ikos_partitioning_join(ar::CallBase*) {
    this->_inv.normal().partitioning_join();
  }

  /// \brief Execute a call to ikos.partitioning.disable
  void exec_ikos_partitioning_disable(ar::CallBase*) {
    this->_inv.normal().partitioning_disable();
  }

  /// \brief Execute a dynamic allocation
  void exec_dynamic_alloc(ar::CallBase* call,
                          ar::Value* size,
                          bool may_return_null,
                          bool may_throw_exc,
                          MemoryInitialValue init_val) {
    if (may_throw_exc) {
      this->throw_unknown_exceptions();
    }

    if (!call->has_result()) {
      return;
    }

    const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
    const ScalarLit& size_l = this->_lit_factory.get_scalar(size);
    ikos_assert_msg(lhs.is_pointer_var(),
                    "left hand side is not a pointer variable");

    auto nullity = may_return_null ? Nullity::top() : Nullity::non_null();

    MemoryLocation* addr =
        this->_mem_factory.get_dyn_alloc(call, this->_call_context);

    if (size_l.is_machine_int_var()) {
      this->allocate_memory(lhs.var(),
                            addr,
                            nullity,
                            Lifetime::allocated(),
                            init_val,
                            size_l.var());
    } else if (size_l.is_machine_int()) {
      this->allocate_memory(lhs.var(),
                            addr,
                            nullity,
                            Lifetime::allocated(),
                            init_val,
                            size_l.machine_int());
    } else {
      ikos_unreachable("unexpected size operand");
    }
  }

  /// \brief Execute a call to libc malloc
  ///
  /// #include <stdlib.h>
  /// void* malloc(size_t size);
  ///
  /// The malloc() function returns a pointer to a newly allocated block size
  /// bytes long, or a null pointer if the block could not be allocated.
  void exec_malloc(ar::CallBase* call) {
    this->exec_dynamic_alloc(call,
                             call->argument(0),
                             /* may_return_null = */ true,
                             /* may_throw_exc = */ false,
                             MemoryInitialValue::Uninitialized);
  }

  /// \brief Execute a call to libc calloc
  ///
  /// #include <stdlib.h>
  /// void* calloc(size_t count, size_t size);
  ///
  /// The calloc() function contiguously allocates enough space for count
  /// objects that are size bytes of memory each and returns a pointer to the
  /// allocated memory. The allocated memory is filled with bytes of value zero.
  void exec_calloc(ar::CallBase* call) {
    if (!call->has_result()) {
      return;
    }

    const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
    const ScalarLit& count = this->_lit_factory.get_scalar(call->argument(0));
    const ScalarLit& size = this->_lit_factory.get_scalar(call->argument(1));
    ikos_assert_msg(lhs.is_pointer_var(),
                    "left hand side is not a pointer variable");

    // Allocate the memory
    MemoryLocation* addr =
        this->_mem_factory.get_dyn_alloc(call, this->_call_context);
    this->allocate_memory(lhs.var(),
                          addr,
                          Nullity::top(),
                          Lifetime::allocated(),
                          MemoryInitialValue::Zero);

    if (!this->_opts.test(ExecutionEngine::UpdateAllocSizeVar)) {
      return;
    }

    // Set the allocation size variable
    Variable* alloc_size_var = this->_var_factory.get_alloc_size(addr);
    if (count.is_machine_int()) {
      if (size.is_machine_int()) {
        bool overflow;
        MachineInt alloc_size_int =
            mul(count.machine_int(), size.machine_int(), overflow);
        if (overflow) {
          this->_inv.set_normal_flow_to_bottom(); // Undefined behavior
        } else {
          this->_inv.normal().int_assign(alloc_size_var, alloc_size_int);
        }
      } else if (size.is_machine_int_var()) {
        this->_inv.normal().int_apply(IntBinaryOperator::MulNoWrap,
                                      alloc_size_var,
                                      count.machine_int(),
                                      size.var());
      } else {
        ikos_unreachable("unexpected size parameter");
      }
    } else if (count.is_machine_int_var()) {
      if (size.is_machine_int()) {
        this->_inv.normal().int_apply(IntBinaryOperator::MulNoWrap,
                                      alloc_size_var,
                                      count.var(),
                                      size.machine_int());
      } else if (size.is_machine_int_var()) {
        this->_inv.normal().int_apply(IntBinaryOperator::MulNoWrap,
                                      alloc_size_var,
                                      count.var(),
                                      size.var());
      } else {
        ikos_unreachable("unexpected size parameter");
      }
    } else {
      ikos_unreachable("unexpected count parameter");
    }
  }

  /// \brief Execute a call to libc valloc
  ///
  /// #include <stdlib.h>
  /// void* valloc(size_t size);
  ///
  /// The valloc() function allocates size bytes of memory and returns a pointer
  /// to the allocated memory.  The allocated memory is aligned on a page
  /// boundary.
  void exec_valloc(ar::CallBase* call) {
    this->exec_dynamic_alloc(call,
                             call->argument(0),
                             /* may_return_null = */ true,
                             /* may_throw_exc = */ false,
                             MemoryInitialValue::Uninitialized);
  }

  /// \brief Execute a call to libc aligned_alloc
  ///
  /// #include <stdlib.h>
  /// void* aligned_alloc(size_t alignment, size_t size);
  ///
  /// The function posix_memalign() allocates size bytes and places the address
  /// of the allocated memory in *memptr. The address of the allocated memory
  /// will be a multiple of alignment, which must be a power of two and a
  /// multiple of sizeof(void *). If size is 0, then posix_memalign() returns
  /// either NULL, or a unique pointer value that can later be successfully
  /// passed to free(3).
  ///
  /// The function aligned_alloc() is the same as memalign(), except for the
  /// added restriction that size should be a multiple of alignment.
  void exec_aligned_alloc(ar::CallBase* call) {
    this->exec_dynamic_alloc(call,
                             call->argument(1),
                             /* may_return_null = */ true,
                             /* may_throw_exc = */ false,
                             MemoryInitialValue::Uninitialized);
  }

  /// \brief Execute a call to libc realloc
  ///
  /// #include <stdlib.h>
  /// void* realloc(void* ptr, size_t size);
  ///
  /// The realloc() function tries to change the size of the allocation pointed
  /// to by ptr to size, and returns ptr.  If there is not enough room to
  /// enlarge the memory allocation pointed to by ptr, realloc() creates a new
  /// allocation, copies as much of the old data pointed to by ptr as will fit
  /// to the new allocation, frees the old allocation, and returns a pointer to
  /// the allocated memory.  If ptr is NULL, realloc() is identical to a call to
  /// malloc() for size bytes.  If size is zero and ptr is not NULL, a new,
  /// minimum sized object is allocated and the original object is freed.  When
  /// extending a region allocated with calloc(3), realloc(3) does not guarantee
  /// that the additional memory is also zero-filled.
  void exec_realloc(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& ptr = this->_lit_factory.get_scalar(call->argument(0));
    const ScalarLit& size = this->_lit_factory.get_scalar(call->argument(1));

    // Allocate the memory
    if (call->has_result()) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
      ikos_assert_msg(lhs.is_pointer_var(),
                      "left hand side is not a pointer variable");

      MemoryLocation* addr =
          this->_mem_factory.get_dyn_alloc(call, this->_call_context);
      if (size.is_machine_int_var()) {
        this->allocate_memory(lhs.var(),
                              addr,
                              Nullity::top(),
                              Lifetime::allocated(),
                              MemoryInitialValue::Uninitialized,
                              size.var());
      } else if (size.is_machine_int()) {
        this->allocate_memory(lhs.var(),
                              addr,
                              Nullity::top(),
                              Lifetime::allocated(),
                              MemoryInitialValue::Uninitialized,
                              size.machine_int());
      } else {
        ikos_unreachable("unexpected size operand");
      }
    }

    // Copy data
    if (call->has_result()) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
      ikos_assert_msg(lhs.is_pointer_var(),
                      "left hand side is not a pointer variable");

      if (ptr.is_pointer_var() &&
          !this->_inv.normal().nullity_is_null(ptr.var())) {
        // This should be the size of `ptr` instead of `size`
        this->_inv.normal().mem_copy(lhs.var(), ptr.var(), size);
      }
    }

    this->_inv.normal().normalize();

    if (this->_inv.is_normal_flow_bottom()) {
      // If the mem_copy generated an error
      return;
    }

    // Free the pointer
    this->exec_free(call);
  }

  /// \brief Execute a call to libc free, libc++ delete or delete[], etc.
  ///
  /// #include <stdlib.h>
  /// void free(void* ptr);
  ///
  /// The free() function deallocates the memory allocation pointed to by ptr.
  /// If ptr is a NULL pointer, no operation is performed.
  void exec_free(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& ptr = this->_lit_factory.get_scalar(call->argument(0));

    if (ptr.is_null()) {
      // This is safe, according to C standards
      return;
    }

    ikos_assert_msg(ptr.is_pointer_var(), "unexpected parameter");

    if (this->_inv.normal().nullity_is_null(ptr.var())) {
      // This is safe, according to C standards
      return;
    }

    // Reduction between value and pointer analysis
    this->refine_addresses(ptr.var());

    PointsToSet addrs = this->_inv.normal().pointer_to_points_to(ptr.var());

    if (addrs.is_bottom()) {
      return;
    } else if (addrs.is_top()) {
      // Ignored memory deallocation, analysis could be unsound.
      // See CheckKind::IgnoredFree
      return;
    }

    // Forget memory contents
    this->_inv.normal().mem_forget_reachable(ptr.var());

    // Forget the allocation size and set the new lifetime
    for (auto addr : addrs) {
      if (!isa< DynAllocMemoryLocation >(addr)) {
        if (addrs.size() == 1) {
          // This is an error
          this->_inv.set_normal_flow_to_bottom();
          return;
        } else {
          continue;
        }
      }

      if (addrs.size() == 1) {
        this->_inv.normal().lifetime_assign_deallocated(addr);
      } else {
        this->_inv.normal().lifetime_forget(addr);
      }

      if (this->_opts.test(ExecutionEngine::UpdateAllocSizeVar)) {
        Variable* alloc_size_var = this->_var_factory.get_alloc_size(addr);
        this->_inv.normal().int_forget(alloc_size_var);
      }
    }
  }

  /// \brief Execute a call to libc exit
  ///
  /// #include <stdlib.h>
  /// void exit(int status);
  ///
  /// The exit() functions terminate a process.
  ///
  /// It also performs the following functions in the order listed:
  ///   1. Call the functions registered with the atexit(3) function, in the
  /// reverse order of their registration.
  ///   2. Flush all open output streams.
  ///   3. Close all open streams.
  ///   4. Unlink all files created with the tmpfile(3) function.
  void exec_exit(ar::CallBase* /*call*/) {
    // TODO(marthaud): analyze functions registered by atexit()
    this->_inv.set_normal_flow_to_bottom();
  }

  /// \brief Execute a call to libc abort
  ///
  /// #include <stdlib.h>
  /// void abort(void);
  ///
  /// The abort() function causes abnormal program termination to occur, unless
  /// the signal SIGABRT is being caught and the signal handler does not return.
  void exec_abort(ar::CallBase* /*call*/) {
    this->_inv.set_normal_flow_to_bottom();
  }

  /// \brief Execute a call to libc errno_location
  ///
  /// #include <errno.h>
  /// int* __errno_location(void);
  ///
  /// The __errno_location() function returns a pointer to the errno variable.
  void exec_errno_location(ar::CallBase* call) {
    // Forget the current value of errno
    MemoryLocation* addr = this->_mem_factory.get_libc_errno();
    this->_inv.normal().mem_forget(addr);

    // Assign the result
    if (call->has_result()) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
      ikos_assert_msg(lhs.is_pointer_var(),
                      "left hand side is not a pointer variable");
      this->_inv.normal().pointer_assign(lhs.var(), addr, Nullity::non_null());
    }
  }

  /// \brief Execute a call to libc read
  ///
  /// #include <unistd.h>
  /// ssize_t read(int handle, void* buffer, size_t nbyte);
  ///
  /// The read() function attempts to read nbytes from the file associated with
  /// handle, and places the characters read into buffer. If the file is opened
  /// using O_TEXT, it removes carriage returns and detects the end of the file.
  ///
  /// The function returns the number of bytes read. On end-of-file, 0 is
  /// returned, on error it returns -1, setting errno to indicate the type of
  /// error that occurred.
  void exec_read(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& ptr = this->_lit_factory.get_scalar(call->argument(1));
    const ScalarLit& size = this->_lit_factory.get_scalar(call->argument(2));

    if (!this->prepare_mem_access(ptr)) {
      return;
    }

    if (this->_inv.normal().pointer_to_points_to(ptr.var()).is_top()) {
      // Ignore read, analysis could be unsound.
      // See CheckKind::IgnoredCallSideEffectOnPointerParameter
    } else if (size.is_machine_int()) {
      this->_inv.normal().mem_abstract_reachable(ptr.var(), size.machine_int());
    } else if (size.is_machine_int_var()) {
      IntInterval size_intv = this->_inv.normal().int_to_interval(size.var());
      this->_inv.normal().mem_abstract_reachable(ptr.var(), size_intv.ub());
    } else {
      ikos_unreachable("unreachable");
    }

    if (call->has_result()) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
      ikos_assert_msg(lhs.is_machine_int_var(),
                      "left hand side is not an integer variable");
      this->_inv.normal().int_assign_nondet(lhs.var());
    }
  }

  /// \brief Execute a call to libc gets
  ///
  /// #include <stdio.h>
  /// char* gets(char* str);
  ///
  /// The gets() function is equivalent to fgets() with an infinite size and a
  /// stream of stdin, except that the newline character (if any) is not stored
  /// in the string.  It is the caller's responsibility to ensure that the input
  /// line, if any, is sufficiently short to fit in the string.
  void exec_gets(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& ptr = this->_lit_factory.get_scalar(call->argument(0));

    if (!this->prepare_mem_access(ptr)) {
      return;
    }

    if (this->_inv.normal().pointer_to_points_to(ptr.var()).is_top()) {
      // Ignore gets, analysis could be unsound.
      // See CheckKind::IgnoredCallSideEffectOnPointerParameter
    } else {
      this->_inv.normal().mem_abstract_reachable(ptr.var());
    }

    if (call->has_result()) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
      ikos_assert_msg(lhs.is_pointer_var(),
                      "left hand side is not a pointer variable");
      this->_inv.normal().pointer_assign(lhs.var(), ptr.var());
      this->_inv.normal().nullity_set(lhs.var(),
                                      Nullity::top()); // Returns null on errors
    }
  }

  /// \brief Execute a call to libc fgets
  ///
  /// #include <stdio.h>
  /// char* fgets(char* str, int size, FILE* stream);
  ///
  /// The fgets() function reads at most one less than the number of characters
  /// specified by size from the given stream and stores them in the string str.
  /// Reading stops when a newline character is found, at end-of-file or error.
  /// The newline, if any, is retained.  If any characters are read and there is
  /// no error, a `\0' character is appended to end the string.
  void exec_fgets(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& ptr = this->_lit_factory.get_scalar(call->argument(0));
    const ScalarLit& size = this->_lit_factory.get_scalar(call->argument(1));

    if (!this->prepare_mem_access(ptr)) {
      return;
    }

    // Size is a ui32, convert it to a size_t
    auto size_type = ar::IntegerType::size_type(this->_ctx.bundle);

    if (this->_inv.normal().pointer_to_points_to(ptr.var()).is_top()) {
      // Ignore fgets, analysis could be unsound.
      // See CheckKind::IgnoredCallSideEffectOnPointerParameter
    } else if (size.is_machine_int()) {
      this->_inv.normal()
          .mem_abstract_reachable(ptr.var(),
                                  size.machine_int()
                                      .cast(size_type->bit_width(),
                                            ar::Unsigned));
    } else if (size.is_machine_int_var()) {
      IntInterval size_intv = this->_inv.normal()
                                  .int_to_interval(size.var())
                                  .cast(size_type->bit_width(), ar::Unsigned);
      this->_inv.normal().mem_abstract_reachable(ptr.var(), size_intv.ub());
    } else {
      ikos_unreachable("unreachable");
    }

    if (call->has_result()) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
      ikos_assert_msg(lhs.is_pointer_var(),
                      "left hand side is not a pointer variable");
      this->_inv.normal().pointer_assign(lhs.var(), ptr.var());
      this->_inv.normal().nullity_set(lhs.var(),
                                      Nullity::top()); // Returns null on errors
    }
  }

  /// \brief Execute a call to libc sprintf
  ///
  /// #include <stdio.h>
  /// int sprintf(char* str, const char* format, ...);
  ///
  /// The snprintf() and vsnprintf() functions will write at most size-1 of the
  /// characters printed into the output string (the size'th character then gets
  /// the terminating `\0'); if the return value is greater than or equal to the
  /// size argument, the string was too short and some of the printed characters
  /// were discarded.  The output is always null-terminated, unless size is 0.
  ///
  /// The sprintf() and vsprintf() functions effectively assume a size of
  /// INT_MAX + 1.
  void exec_sprintf(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& ptr = this->_lit_factory.get_scalar(call->argument(0));

    if (!this->prepare_mem_access(ptr)) {
      return;
    }

    if (this->_inv.normal().pointer_to_points_to(ptr.var()).is_top()) {
      // Ignore sprintf, analysis could be unsound.
      // See CheckKind::IgnoredCallSideEffectOnPointerParameter
    } else {
      this->_inv.normal().mem_abstract_reachable(ptr.var());
    }

    if (call->has_result()) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
      ikos_assert_msg(lhs.is_machine_int_var(),
                      "left hand side is not an integer variable");
      this->_inv.normal().int_assign_nondet(lhs.var());
    }
  }

  /// \brief Execute a call to libc snprintf
  ///
  /// #include <stdio.h>
  /// int snprintf(char* str, size_t size, const char* format, ...);
  ///
  /// The snprintf() and vsnprintf() functions will write at most size-1 of the
  /// characters printed into the output string (the size'th character then gets
  /// the terminating `\0'); if the return value is greater than or equal to the
  /// size argument, the string was too short and some of the printed characters
  /// were discarded.  The output is always null-terminated, unless size is 0.
  void exec_snprintf(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& ptr = this->_lit_factory.get_scalar(call->argument(0));
    const ScalarLit& size = this->_lit_factory.get_scalar(call->argument(1));

    // Calling snprintf with zero bufsz and null pointer buffer can be used to
    // determine the buffer size needed to contain the output. That case is
    // allowed. Otherwise, check first argument.
    bool checkPointer =
        !(size.is_machine_int() && size.machine_int().is_zero());

    if (checkPointer) {
      if (!this->prepare_mem_access(ptr)) {
        return;
      }

      if (this->_inv.normal().pointer_to_points_to(ptr.var()).is_top()) {
        // Ignore snprintf, analysis could be unsound.
        // See CheckKind::IgnoredCallSideEffectOnPointerParameter
      } else if (size.is_machine_int()) {
        this->_inv.normal().mem_abstract_reachable(ptr.var(),
                                                   size.machine_int());
      } else if (size.is_machine_int_var()) {
        IntInterval size_intv = this->_inv.normal().int_to_interval(size.var());
        this->_inv.normal().mem_abstract_reachable(ptr.var(), size_intv.ub());
      } else {
        ikos_unreachable("unreachable");
      }
    }

    if (call->has_result()) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
      ikos_assert_msg(lhs.is_machine_int_var(),
                      "left hand side is not an integer variable");
      this->_inv.normal().int_assign_nondet(lhs.var());
    }
  }

  /// \brief Execute a call to libc fopen
  ///
  /// #include <stdio.h>
  /// FILE* fopen(const char* path, const char* mode);
  ///
  /// The fopen() function opens the file whose name is the string pointed to by
  /// path and associates a stream with it.
  void exec_fopen(ar::CallBase* call) {
    if (!call->has_result()) {
      return;
    }

    const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
    ikos_assert_msg(lhs.is_pointer_var(),
                    "left hand side is not a pointer variable");

    MemoryLocation* addr =
        this->_mem_factory.get_dyn_alloc(call, this->_call_context);

    this->allocate_memory(lhs.var(),
                          addr,
                          Nullity::top(),
                          Lifetime::allocated(),
                          MemoryInitialValue::Unknown);
  }

  /// \brief Execute a call to libc fclose
  ///
  /// #include <stdio.h>
  /// int fclose(FILE* stream);
  ///
  /// The fclose() function dissociates the named stream from its underlying
  /// file or set of functions.  If the stream was being used for output, any
  /// buffered data is written first, using fflush(3).
  void exec_fclose(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& ptr = this->_lit_factory.get_scalar(call->argument(0));

    // fclose(NULL) is undefined behavior
    if (!this->prepare_mem_access(ptr)) {
      return;
    }

    this->exec_free(call);

    if (call->has_result()) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
      ikos_assert_msg(lhs.is_machine_int_var(),
                      "left hand side is not an integer variable");
      this->_inv.normal().int_assign_nondet(lhs.var());
    }
  }

  /// \brief Execute a call to libc strlen
  ///
  /// #include <string.h>
  /// size_t strlen(const char* s);
  ///
  /// The strlen() function computes the length of the string s.
  ///
  /// The strlen() function returns the number of characters that precede the
  /// terminating NULL character.
  void exec_strlen(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& str = this->_lit_factory.get_scalar(call->argument(0));

    if (!this->prepare_mem_access(str)) {
      return;
    }

    if (!call->has_result()) {
      return;
    }

    const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
    ikos_assert_msg(lhs.is_machine_int_var(),
                    "left hand side is not an integer variable");

    // lhs is in [0, size - 1]
    this->_inv.normal().int_assign_nondet(lhs.var());

    PointsToSet addrs = this->_inv.normal().pointer_to_points_to(str.var());

    if (addrs.is_top()) {
      return;
    }

    boost::optional< AbstractDomain > inv = boost::none;

    for (MemoryLocation* addr : addrs) {
      AbstractDomain tmp = this->_inv;

      if (auto gv = dyn_cast< GlobalMemoryLocation >(addr)) {
        auto alloc_size = MachineInt(this->_data_layout.store_size_in_bytes(
                                         gv->global_var()->type()->pointee()),
                                     this->_data_layout.pointers.bit_width,
                                     Unsigned);
        tmp.normal().int_add(IntPredicate::LT, lhs.var(), alloc_size);
      } else {
        Variable* size_var = this->_var_factory.get_alloc_size(addr);
        tmp.normal().int_add(IntPredicate::LT, lhs.var(), size_var);
      }

      if (!inv) {
        inv = std::move(tmp);
      } else {
        inv->join_with(tmp);
      }
    }

    if (!inv) {
      this->_inv.set_to_bottom();
    } else {
      this->_inv = std::move(*inv);
    }
  }

  /// \brief Execute a call to libc strlen
  ///
  /// #include <string.h>
  /// size_t strnlen(const char* s, size_t maxlen);
  ///
  /// The strnlen() function attempts to compute the length of s, but never
  /// scans beyond the first maxlen bytes of s.
  ///
  /// The strnlen() function returns either the same result as strlen() or
  /// maxlen, whichever is smaller.
  void exec_strnlen(ar::CallBase* call) {
    this->exec_strlen(call);

    if (!call->has_result()) {
      return;
    }

    // lhs <= maxlen
    const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
    const ScalarLit& maxlen = this->_lit_factory.get_scalar(call->argument(1));
    ikos_assert_msg(lhs.is_machine_int_var(),
                    "left hand side is not an integer variable");

    if (maxlen.is_machine_int()) {
      this->_inv.normal().int_add(IntPredicate::LE,
                                  lhs.var(),
                                  maxlen.machine_int());
    } else if (maxlen.is_machine_int_var()) {
      this->_inv.normal().int_add(IntPredicate::LE, lhs.var(), maxlen.var());
    } else {
      ikos_unreachable("unexpected maxlen parameter");
    }
  }

  /// \brief Execute a call to libc strcpy
  ///
  /// #include <string.h>
  /// char* strcpy(char* dst, const char* src);
  ///
  /// The strcpy() function copies the string src to dst (including the
  /// terminating `\0' character).
  ///
  /// The strcpy() function returns dst.
  void exec_strcpy(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& dest = this->_lit_factory.get_scalar(call->argument(0));
    const ScalarLit& src = this->_lit_factory.get_scalar(call->argument(1));

    if (!this->prepare_mem_access(dest)) {
      return;
    }
    if (!this->prepare_mem_access(src)) {
      return;
    }

    if (this->_inv.normal().pointer_to_points_to(dest.var()).is_top()) {
      // Ignore strcpy, analysis could be unsound.
      // See CheckKind::IgnoredCallSideEffectOnPointerParameter
    } else {
      // Do not keep track of the content
      this->_inv.normal().mem_abstract_reachable(dest.var());
    }

    if (call->has_result()) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
      ikos_assert_msg(lhs.is_pointer_var(),
                      "left hand side is not a pointer variable");
      this->assign(lhs, dest);
    }
  }

  /// \brief Execute a call to libc strncpy
  ///
  /// #include <string.h>
  /// char* strncpy(char* dst, const char* src, size_t n);
  ///
  /// The strncpy() function copies at most n characters from src into dst.
  /// If src is less than n characters long, the remainder of dst is filled with
  /// `\0' characters. Otherwise, dst is not terminated.
  ///
  /// The strncpy() function returns dst.
  void exec_strncpy(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& dest = this->_lit_factory.get_scalar(call->argument(0));
    const ScalarLit& src = this->_lit_factory.get_scalar(call->argument(1));
    const ScalarLit& size = this->_lit_factory.get_scalar(call->argument(2));

    if (!this->prepare_mem_access(dest)) {
      return;
    }
    if (!this->prepare_mem_access(src)) {
      return;
    }

    if (this->_inv.normal().pointer_to_points_to(dest.var()).is_top()) {
      // Ignore strncpy, analysis could be unsound.
      // See CheckKind::IgnoredCallSideEffectOnPointerParameter
    } else if (size.is_machine_int()) {
      this->_inv.normal().mem_abstract_reachable(dest.var(),
                                                 size.machine_int());
    } else if (size.is_machine_int_var()) {
      IntInterval size_intv = this->_inv.normal().int_to_interval(size.var());
      this->_inv.normal().mem_abstract_reachable(dest.var(), size_intv.ub());
    } else {
      ikos_unreachable("unreachable");
    }

    if (call->has_result()) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
      ikos_assert_msg(lhs.is_pointer_var(),
                      "left hand side is not a pointer variable");
      this->assign(lhs, dest);
    }
  }

  /// \brief Execute a call to libc strcat
  ///
  /// #include <string.h>
  /// char* strcat(char* s1, const char* s2);
  ///
  /// The strcat() function appends a copy of the null-terminated string s2 to
  /// the end of the null-terminated string s1, then add a terminating \0. The
  /// string s1 must have sufficient space to hold the result.
  ///
  /// The strcat() function returns the pointer s1.
  void exec_strcat(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& s1 = this->_lit_factory.get_scalar(call->argument(0));
    const ScalarLit& s2 = this->_lit_factory.get_scalar(call->argument(1));

    if (!this->prepare_mem_access(s1)) {
      return;
    }
    if (!this->prepare_mem_access(s2)) {
      return;
    }

    if (this->_inv.normal().pointer_to_points_to(s1.var()).is_top()) {
      // Ignore strcat, analysis could be unsound.
      // See CheckKind::IgnoredCallSideEffectOnPointerParameter
    } else {
      // Do not keep track of the content
      this->_inv.normal().mem_abstract_reachable(s1.var());
    }

    if (call->has_result()) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
      ikos_assert_msg(lhs.is_pointer_var(),
                      "left hand side is not a pointer variable");
      this->assign(lhs, s1);
    }
  }

  /// \brief Execute a call to libc strcat
  ///
  /// #include <string.h>
  /// char* strncat(char* s1, const char* s2, size_t n);
  ///
  /// The strncat() function appends a copy of the null-terminated string s2 to
  /// the end of the null-terminated string s1, then add a terminating `\0'. The
  /// string s1 must have sufficient space to hold the result.
  ///
  /// The strncat() function appends not more than n characters from s2, and
  /// then adds a terminating `\0'.
  ///
  /// The strncat() function returns the pointer s1.
  void exec_strncat(ar::CallBase* call) { this->exec_strcat(call); }

  /// \brief Execute a call to libc strstr
  ///
  /// #include <string.h>
  /// char* strstr(const char* haystack, const char* needle);
  ///
  /// The strstr() function locates the first occurrence of the null-terminated
  /// string needle in the null-terminated string haystack.
  void exec_strstr(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& haystack =
        this->_lit_factory.get_scalar(call->argument(0));
    const ScalarLit& needle = this->_lit_factory.get_scalar(call->argument(1));

    if (!this->prepare_mem_access(haystack)) {
      return;
    }
    if (!this->prepare_mem_access(needle)) {
      return;
    }

    if (call->has_result()) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
      ikos_assert_msg(lhs.is_pointer_var(),
                      "left hand side is not a pointer variable");
      this->_inv.normal().pointer_assign(lhs.var(), haystack.var());
      this->_inv.normal().nullity_set(lhs.var(), Nullity::top());
      this->_inv.normal().pointer_forget_offset(lhs.var());
    }
  }

  /// \brief Execute a call to libc strchr
  ///
  /// #include <string.h>
  /// char* strchr(const char* s, int c);
  ///
  /// The strchr() function locates the first occurrence of c (converted to a
  /// char) in the string pointed to by s.  The terminating null character is
  /// considered to be part of the string; therefore if c is `\0', the functions
  /// locate the terminating `\0'.
  void exec_strchr(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& s = this->_lit_factory.get_scalar(call->argument(0));

    if (!this->prepare_mem_access(s)) {
      return;
    }

    if (call->has_result()) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
      ikos_assert_msg(lhs.is_pointer_var(),
                      "left hand side is not a pointer variable");
      this->_inv.normal().pointer_assign(lhs.var(), s.var());
      this->_inv.normal().nullity_set(lhs.var(), Nullity::top());
      this->_inv.normal().pointer_forget_offset(lhs.var());
    }
  }

  /// \brief Execute a call to libc strdup
  ///
  /// #include <string.h>
  /// char* strdup(const char* s1);
  ///
  /// The strdup() function allocates sufficient memory for a copy of the string
  /// s1, does the copy, and returns a pointer to it.  The pointer may
  /// subsequently be used as an argument to the function free(3).
  void exec_strdup(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& s = this->_lit_factory.get_scalar(call->argument(0));

    if (!this->prepare_mem_access(s)) {
      return;
    }

    if (call->has_result()) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
      ikos_assert_msg(lhs.is_pointer_var(),
                      "left hand side is not a pointer variable");

      MemoryLocation* addr =
          this->_mem_factory.get_dyn_alloc(call, this->_call_context);
      this->allocate_memory(lhs.var(),
                            addr,
                            Nullity::top(),
                            Lifetime::allocated(),
                            MemoryInitialValue::Unknown);
    }
  }

  /// \brief Execute a call to libc strndup
  ///
  /// #include <string.h>
  /// char* strndup(const char* s1, size_t n);
  ///
  /// The strndup() function copies at most n characters from the string s1
  /// always NUL terminating the copied string.
  void exec_strndup(ar::CallBase* call) {
    this->init_global_operands(call);

    const ScalarLit& s = this->_lit_factory.get_scalar(call->argument(0));
    const ScalarLit& n = this->_lit_factory.get_scalar(call->argument(1));

    if (!this->prepare_mem_access(s)) {
      return;
    }

    if (!call->has_result()) {
      return;
    }

    const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
    ikos_assert_msg(lhs.is_pointer_var(),
                    "left hand side is not a pointer variable");

    MemoryLocation* addr =
        this->_mem_factory.get_dyn_alloc(call, this->_call_context);
    this->allocate_memory(lhs.var(),
                          addr,
                          Nullity::top(),
                          Lifetime::allocated(),
                          MemoryInitialValue::Unknown);

    if (this->_opts.test(ExecutionEngine::UpdateAllocSizeVar)) {
      // sizeof(addr) <= n
      Variable* alloc_size_var = this->_var_factory.get_alloc_size(addr);

      if (n.is_machine_int()) {
        this->_inv.normal().int_add(IntPredicate::LE,
                                    alloc_size_var,
                                    n.machine_int());
      } else if (n.is_machine_int_var()) {
        this->_inv.normal().int_add(IntPredicate::LE, alloc_size_var, n.var());
      } else {
        ikos_unreachable("unexpected size operand");
      }
    }
  }

  /// \brief Execute a call to libc++ new or new[]
  ///
  /// operator new(size_t)
  /// operator new[](size_t)
  ///
  /// Allocates requested number of bytes. These allocation functions are called
  /// by new-expressions to allocate memory in which new object would then be
  /// initialized. They may also be called using regular function call syntax.
  void exec_new(ar::CallBase* call) {
    this->exec_dynamic_alloc(call,
                             call->argument(0),
                             /* may_return_null = */ false,
                             /* may_throw_exc = */ true,
                             MemoryInitialValue::Uninitialized);
  }

  /// \brief Execute a call to libc++ allocate exception
  ///
  /// void* __cxa_allocate_exception(size_t thrown_size) noexcept;
  ///
  /// Allocates memory to hold the exception to be thrown. thrown_size is the
  /// size of the exception object. Can allocate additional memory to hold
  /// private data. If memory can not be allocated, call std::terminate().
  void exec_allocate_exception(ar::CallBase* call) {
    this->exec_dynamic_alloc(call,
                             call->argument(0),
                             /* may_return_null = */ false,
                             /* may_throw_exc = */ false,
                             MemoryInitialValue::Uninitialized);
  }

  /// \brief Execute a call to libc++ throw
  ///
  /// __cxa_throw(void* exception, std::type_info* tinfo, void (*dest)(void*))
  ///
  /// After constructing the exception object with the throw argument value, the
  /// generated code calls the __cxa_throw runtime library routine. This routine
  /// never returns.
  void exec_throw(ar::CallBase* /*call*/) { this->_inv.throw_exception(); }

  /// \brief Execute a call to libc++ begin catch
  ///
  /// void* __cxa_begin_catch(void* exc_obj) noexcept;
  ///
  /// When entering a catch scope, __cxa_begin_catch is called with the
  /// exception object `exc_obj`. This routine returns the adjusted pointer to
  /// the exception object.
  ///
  /// Assume that it returns `exc_obj` unchanged.
  void exec_begin_catch(ar::CallBase* call) {
    if (!call->has_result()) {
      return;
    }
    const ScalarLit& lhs = this->_lit_factory.get_scalar(call->result());
    const ScalarLit& exc_obj = this->_lit_factory.get_scalar(call->argument(0));
    this->assign(lhs, exc_obj);
  }

  /// \brief Execute a call to libc++ end catch
  ///
  /// void __cxa_end_catch();
  ///
  /// Locates the most recently caught exception and decrements its handler
  /// count. Removes the exception from the caughtÃ“exception stack, if the
  /// handler count goes to zero. Destroys the exception if the handler count
  /// goes to zero, and the exception was not re-thrown by throw. Collaboration
  /// between __cxa_rethrow() and __cxa_end_catch() is necessary to handle the
  /// last point. Though implementation-defined, one possibility is for
  /// __cxa_rethrow() to set a flag in the handlerCount member of the exception
  /// header to mark an exception being rethrown.
  void exec_end_catch(ar::CallBase* /*call*/) {}

  /// @}

public:
  void match_down(ar::CallBase* call, ar::Function* called) override {
    ikos_assert(called->is_definition());
    ikos_assert(ar::TypeVerifier::is_valid_call(call, called->type()));

    auto param_it = called->param_begin();
    auto param_et = called->param_end();
    auto arg_it = call->arg_begin();
    auto arg_et = call->arg_end();
    for (; param_it != param_et && arg_it != arg_et; ++param_it, ++arg_it) {
      this->init_global_operand(*arg_it);
      this->implicit_bitcast(this->_lit_factory.get(*param_it),
                             this->_lit_factory.get(*arg_it));
    }
  }

  void match_up(ar::CallBase* call, ar::ReturnValue* ret) override {
    if (ret == nullptr || !ret->has_operand()) {
      // No return value
      return;
    }

    const Literal& return_value = this->_lit_factory.get(ret->operand());

    if (call->has_result()) {
      // Assign the result variable
      const Literal& result = this->_lit_factory.get(call->result());
      this->init_global_operand(ret->operand());
      this->implicit_bitcast(result, return_value);

      if (return_value.is_var()) {
        // If the current partitioning is based on the return variable,
        // we automatically update it to the result variable.
        auto partitioning_var = this->_inv.normal().partitioning_variable();
        if (partitioning_var && *partitioning_var == return_value.var()) {
          this->_inv.normal().partitioning_set_variable(result.var());
        }
      }
    }

    // Clean-up the invariant
    if (!return_value.is_var()) {
      return;
    } else if (return_value.is_scalar()) {
      this->_inv.normal().scalar_forget(return_value.var());
    } else if (return_value.is_aggregate()) {
      this->_inv.normal().mem_forget_reachable(return_value.var());
      this->_inv.normal().scalar_forget(return_value.var());
    } else {
      ikos_unreachable("unreachable");
    }
  }

}; // end class NumericalExecutionEngine

} // end namespace analyzer
} // end namespace ikos
