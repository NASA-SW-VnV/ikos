/******************************************************************************
 *
 * \file
 * \brief Pointer constraints generation
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
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

#include <ikos/core/domain/pointer/solver.hpp>

#include <ikos/ar/semantic/code.hpp>
#include <ikos/ar/semantic/data_layout.hpp>
#include <ikos/ar/semantic/function.hpp>
#include <ikos/ar/semantic/statement.hpp>
#include <ikos/ar/semantic/statement_visitor.hpp>
#include <ikos/ar/verify/type.hpp>

#include <ikos/analyzer/analysis/context.hpp>
#include <ikos/analyzer/analysis/literal.hpp>
#include <ikos/analyzer/analysis/memory_location.hpp>
#include <ikos/analyzer/analysis/pointer/value.hpp>
#include <ikos/analyzer/analysis/variable.hpp>
#include <ikos/analyzer/exception.hpp>
#include <ikos/analyzer/support/cast.hpp>

namespace ikos {
namespace analyzer {

/// \brief Pointer constraint
using PointerConstraint =
    core::pointer::Constraint< Variable*, MemoryLocation* >;

/// \brief System of pointer constraints
class PointerConstraints {
private:
  /// \brief System of pointer constraints
  core::pointer::ConstraintSystem< Variable*, MemoryLocation* > _system;

public:
  /// \brief Constructor
  explicit PointerConstraints(const ar::DataLayout& dl);

  /// \brief No copy constructor
  PointerConstraints(const PointerConstraints&) = delete;

  /// \brief No move constructor
  PointerConstraints(PointerConstraints&&) = delete;

  /// \brief No copy assignment operator
  PointerConstraints& operator=(const PointerConstraints&) = delete;

  /// \brief No move assignment operator
  PointerConstraints& operator=(PointerConstraints&&) = delete;

  /// \brief Destructor
  ~PointerConstraints();

  /// \brief Add a pointer constraint
  void add(std::unique_ptr< PointerConstraint > cst);

  /// \brief Solve pointer constraints
  void solve();

  /// \brief Export results
  void results(PointerInfo&) const;

  /// \brief Dump the pointer constraints, for debugging purpose
  void dump(std::ostream&) const;

}; // end class PointerConstraints

/// \brief Generate points-to constraints for a given ar::Code*
template < typename CodeInvariants >
class PointerConstraintsGenerator {
private:
  using AbstractDomainT = typename CodeInvariants::AbstractDomainT;

private:
  using MachineIntLinearExpression =
      core::LinearExpression< MachineInt, Variable* >;
  using VarOp = core::pointer::VariableOperand< Variable*, MemoryLocation* >;
  using AddrOp = core::pointer::AddressOperand< Variable*, MemoryLocation* >;
  using AssignCst =
      core::pointer::AssignConstraint< Variable*, MemoryLocation* >;
  using StoreCst = core::pointer::StoreConstraint< Variable*, MemoryLocation* >;
  using LoadCst = core::pointer::LoadConstraint< Variable*, MemoryLocation* >;

private:
  /// \brief Analysis context
  Context& _ctx;

  /// \brief Data layout
  const ar::DataLayout& _data_layout;

  /// \brief System of pointer constraints
  PointerConstraints& _csts;

  /// \brief Information about function pointers, or null
  const PointerInfo* _function_pointer;

public:
  /// \brief Constructor
  PointerConstraintsGenerator(Context& ctx,
                              PointerConstraints& csts,
                              const PointerInfo* function_pointer = nullptr)
      : _ctx(ctx),
        _data_layout(ctx.bundle->data_layout()),
        _csts(csts),
        _function_pointer(function_pointer) {}

  /// \brief Add pointer constraints for a function definition
  ///
  /// \param function The function
  /// \param invariants Numerical invariants for each basic block
  void process_function_def(ar::Function* function,
                            const CodeInvariants& invariants) {
    this->process_function_decl(function);

    for (ar::BasicBlock* bb : *function->body()) {
      BasicBlockVisitor vis(this->_ctx,
                            this->_csts,
                            this->_function_pointer,
                            invariants,
                            invariants.entry(bb));
      vis.process(bb);
    }
  }

  /// \brief Add pointer constraints for a function declaration
  void process_function_decl(ar::Function* function) {
    Variable* fun_ptr = _ctx.var_factory->get_function_ptr(function);
    MemoryLocation* fun_addr = _ctx.mem_factory->get_function(function);
    this->_csts.add(
        AssignCst::create(fun_ptr, AddrOp::create(fun_addr, zero())));
  }

  /// \brief Add pointer constraints for a global variable definition
  ///
  /// \param gv The global variable
  /// \param invariants Numerical invariants for each basic block
  void process_global_var_def(ar::GlobalVariable* gv,
                              const CodeInvariants& invariants) {
    this->process_global_var_decl(gv);

    for (ar::BasicBlock* bb : *gv->initializer()) {
      BasicBlockVisitor vis(this->_ctx,
                            this->_csts,
                            this->_function_pointer,
                            invariants,
                            invariants.entry(bb));
      vis.process(bb);
    }
  }

  /// \brief Add pointer constraints for a global variable declaration
  void process_global_var_decl(ar::GlobalVariable* gv) {
    Variable* gv_ptr = _ctx.var_factory->get_global(gv);
    MemoryLocation* gv_addr = _ctx.mem_factory->get_global(gv);
    this->_csts.add(AssignCst::create(gv_ptr, AddrOp::create(gv_addr, zero())));
  }

private:
  /// \name Implementation
  /// @{

  /// \brief Return the interval [0, 0]
  MachineIntInterval zero() const {
    return MachineIntInterval(
        MachineInt::zero(this->_data_layout.pointers.bit_width, Unsigned));
  }

  /// \brief Visit all statements of an ar::BasicBlock*
  class BasicBlockVisitor {
  private:
    /// \brief Analysis context
    Context& _ctx;

    /// \brief Data layout
    const ar::DataLayout& _data_layout;

    /// \brief Literal factory
    LiteralFactory& _lit_factory;

    /// \brief System of pointer constraints
    PointerConstraints& _csts;

    /// \brief Information about function pointers
    const PointerInfo* _function_pointer;

    /// \brief Numerical invariants before for each basic block
    const CodeInvariants& _invariants;

    /// \brief Current invariant
    AbstractDomainT _inv;

  public:
    /// \brief Constructor
    BasicBlockVisitor(Context& ctx,
                      PointerConstraints& csts,
                      const PointerInfo* function_pointer,
                      const CodeInvariants& invariants,
                      AbstractDomainT inv)
        : _ctx(ctx),
          _data_layout(ctx.bundle->data_layout()),
          _lit_factory(*ctx.lit_factory),
          _csts(csts),
          _function_pointer(function_pointer),
          _invariants(invariants),
          _inv(std::move(inv)) {}

    /// \brief Return the interval [0, 0]
    MachineIntInterval zero() const {
      return MachineIntInterval(
          MachineInt::zero(this->_data_layout.pointers.bit_width, Unsigned));
    }

    /// \brief Generate points-to constraint for the given basic block
    void process(ar::BasicBlock* bb) {
      for (ar::Statement* stmt : *bb) {
        // Generate pointer constraints for the statement
        ar::apply_visitor(*this, stmt);

        // Propagate the invariant
        this->_inv =
            this->_invariants.analyze_statement(stmt, std::move(this->_inv));
      }
    }

  private:
    /// \brief Perform a memory copy from `src` to `dest`
    void mem_copy(Variable* dest, Variable* src) {
      // Create a temporary variable
      ar::PointerType* void_ptr_ty =
          ar::PointerType::get(_ctx.bundle->context(),
                               ar::VoidType::get(_ctx.bundle->context()));
      Variable* tmp = _ctx.var_factory->create_unnamed_shadow(void_ptr_ty);

      // tmp = *src
      this->_csts.add(LoadCst::create(tmp, VarOp::create(src, zero())));

      // *dest = tmp
      this->_csts.add(StoreCst::create(dest, VarOp::create(tmp, zero())));
    }

    /// \brief Assign `lhs = rhs`
    void assign(const ScalarLit& lhs, const ScalarLit& rhs) {
      if (!lhs.is_pointer_var()) {
        return;
      }

      if (rhs.is_machine_int()) {
        ikos_unreachable("assignment from integer to pointer");
      } else if (rhs.is_floating_point()) {
        ikos_unreachable("assignment from float to pointer");
      } else if (rhs.is_null() || rhs.is_undefined()) {
        return;
      } else if (rhs.is_machine_int_var()) {
        ikos_unreachable("assignment from integer to pointer");
      } else if (rhs.is_floating_point_var()) {
        ikos_unreachable("assignment from float to pointer");
      } else if (rhs.is_pointer_var()) {
        this->_csts.add(
            AssignCst::create(lhs.var(), VarOp::create(rhs.var(), zero())));
      } else {
        ikos_unreachable("unreachable");
      }
    }

  private:
    /// @}
    /// \name Helpers for aggregate (struct,array) statements
    /// @{

    /// \brief Return a pointer to the symbolic location of the aggregate in
    /// memory
    InternalVariable* aggregate_pointer(const AggregateLit& aggregate) {
      ikos_assert_msg(aggregate.is_var(), "aggregate is not a variable");

      auto var = cast< InternalVariable >(aggregate.var());
      auto addr = _ctx.mem_factory->get_aggregate(var->internal_var());
      this->_csts.add(AssignCst::create(var, AddrOp::create(addr, zero())));
      return var;
    }

    /// \brief Write an aggregate in the memory
    void mem_write_aggregate(Variable* ptr, const AggregateLit& aggregate) {
      if (aggregate.is_cst()) {
        // In theory, we should add stores at `ptr + field.offset` instead of
        // `ptr`, but the pointer analysis ignores offsets.
        for (const auto& field : aggregate.fields()) {
          if (field.value.is_var()) {
            this->_csts.add(
                StoreCst::create(ptr,
                                 VarOp::create(field.value.var(), zero())));
          }
        }
      } else if (aggregate.is_zero() || aggregate.is_undefined()) {
        return; // nothing to do
      } else if (aggregate.is_var()) {
        this->mem_copy(ptr, this->aggregate_pointer(aggregate));
      } else {
        ikos_unreachable("unreachable");
      }
    }

  private:
    /// \brief Aggregate assignment `lhs = rhs`
    void assign(const AggregateLit& lhs, const AggregateLit& rhs) {
      ikos_assert_msg(lhs.is_var(), "left hand side is not a variable");

      this->mem_write_aggregate(this->aggregate_pointer(lhs), rhs);
    }

    /// \brief Assignment `lhs = rhs`
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

  public:
    // Statement visitor
    using ResultType = void;

    void operator()(ar::Assignment* s) {
      this->assign(this->_lit_factory.get(s->result()),
                   this->_lit_factory.get(s->operand()));
    }

    void operator()(ar::UnaryOperation* s) {
      switch (s->op()) {
        case ar::UnaryOperation::UIToPtr:
        case ar::UnaryOperation::SIToPtr: {
          // Cast from int to ptr (for instance: int x = 5; int *px = x;)
          uint64_t bit_width = this->_data_layout.pointers.bit_width;
          const ScalarLit& lhs = this->_lit_factory.get_scalar(s->result());
          const ScalarLit& rhs = this->_lit_factory.get_scalar(s->operand());

          if (rhs.is_machine_int()) {
            auto offset = rhs.machine_int().cast(bit_width, Unsigned);
            AssignCst::create(lhs.var(),
                              AddrOp::create(_ctx.mem_factory
                                                 ->get_absolute_zero(),
                                             MachineIntInterval(offset)));
          } else if (rhs.is_machine_int_var()) {
            auto offset_intv =
                this->_inv.normal().int_to_interval(rhs.var()).cast(bit_width,
                                                                    Unsigned);
            AssignCst::create(lhs.var(),
                              AddrOp::create(_ctx.mem_factory
                                                 ->get_absolute_zero(),
                                             offset_intv));
          } else {
            ikos_unreachable("unexpected operand");
          }
        } break;
        case ar::UnaryOperation::Bitcast: {
          if (s->result()->type()->is_pointer()) {
            // Pointer cast
            ikos_assert(s->operand()->type()->is_pointer());
            this->assign(this->_lit_factory.get_scalar(s->result()),
                         this->_lit_factory.get_scalar(s->operand()));
          }
        } break;
        default:
          break;
      }
    }

    void operator()(ar::BinaryOperation*) {}

    void operator()(ar::Comparison*) {}

    void operator()(ar::ReturnValue* s) {
      if (!s->has_operand()) {
        return;
      }

      Variable* ret_var = _ctx.var_factory->get_return(s->code()->function());
      const Literal& rhs = this->_lit_factory.get(s->operand());

      if (rhs.is_scalar()) {
        if (rhs.scalar().is_pointer_var()) {
          this->_csts.add(
              AssignCst::create(ret_var,
                                VarOp::create(rhs.scalar().var(), zero())));
        }
      } else if (rhs.is_aggregate()) {
        this->mem_write_aggregate(ret_var, rhs.aggregate());
      } else {
        ikos_unreachable("unreachable");
      }
    }

    void operator()(ar::Unreachable*) {}

    void operator()(ar::Allocate* s) {
      Variable* local_var = _ctx.var_factory->get_local(s->result());
      MemoryLocation* local_addr = _ctx.mem_factory->get_local(s->result());
      this->_csts.add(
          AssignCst::create(local_var, AddrOp::create(local_addr, zero())));
    }

    void operator()(ar::PointerShift* s) {
      const ScalarLit& lhs = this->_lit_factory.get_scalar(s->result());
      const ScalarLit& base = this->_lit_factory.get_scalar(s->pointer());
      ikos_assert_msg(lhs.is_pointer_var(),
                      "left hand side is not a pointer variable");

      if (base.is_undefined()) {
        return;
      }

      uint64_t bit_width = this->_data_layout.pointers.bit_width;
      MachineIntLinearExpression offset_expr(
          MachineInt::zero(bit_width, Unsigned));

      for (auto it = s->term_begin(), et = s->term_end(); it != et; ++it) {
        auto term = *it;
        const ScalarLit& offset = this->_lit_factory.get_scalar(term.second);

        if (offset.is_undefined()) {
          return;
        } else if (offset.is_machine_int()) {
          offset_expr.add(
              mul(term.first, offset.machine_int().cast(bit_width, Unsigned)));
        } else if (offset.is_machine_int_var()) {
          offset_expr.add(term.first, offset.var());
        } else {
          ikos_unreachable("unreachable");
        }
      }

      MachineIntInterval offset_intv =
          this->_inv.normal().int_to_interval(offset_expr);
      if (base.is_null()) {
        this->_csts.add(
            AssignCst::create(lhs.var(),
                              AddrOp::create(_ctx.mem_factory
                                                 ->get_absolute_zero(),
                                             offset_intv)));
      } else {
        this->_csts.add(
            AssignCst::create(lhs.var(),
                              VarOp::create(base.var(), offset_intv)));
      }
    }

    void operator()(ar::Load* s) {
      const ScalarLit& ptr = this->_lit_factory.get_scalar(s->operand());
      const Literal& result = this->_lit_factory.get(s->result());

      if (!ptr.is_pointer_var()) {
        return;
      }

      if (result.is_scalar()) {
        const ScalarLit& lhs = result.scalar();
        ikos_assert_msg(lhs.is_var(), "left hand side is not a variable");

        if (lhs.is_pointer_var()) {
          this->_csts.add(
              LoadCst::create(lhs.var(), VarOp::create(ptr.var(), zero())));
        }
      } else if (result.is_aggregate()) {
        const AggregateLit& lhs = result.aggregate();
        ikos_assert_msg(lhs.is_var(), "left hand side is not a variable");

        this->mem_copy(this->aggregate_pointer(lhs), ptr.var());
      } else {
        ikos_unreachable("unexpected left hand side");
      }
    }

    void operator()(ar::Store* s) {
      const ScalarLit& ptr = this->_lit_factory.get_scalar(s->pointer());
      const Literal& val = this->_lit_factory.get(s->value());

      if (!ptr.is_pointer_var()) {
        return;
      }

      if (val.is_scalar()) {
        const ScalarLit& rhs = val.scalar();

        if (rhs.is_pointer_var()) {
          this->_csts.add(
              StoreCst::create(ptr.var(), VarOp::create(rhs.var(), zero())));
        }
      } else if (val.is_aggregate()) {
        this->mem_write_aggregate(ptr.var(), val.aggregate());
      } else {
        ikos_unreachable("unexpected right hand side");
      }
    }

    void operator()(ar::ExtractElement* s) {
      const Literal& lhs = this->_lit_factory.get(s->result());
      const AggregateLit& rhs =
          this->_lit_factory.get_aggregate(s->aggregate());
      ikos_assert_msg(rhs.is_var(), "right hand side is not a variable");

      Variable* rhs_ptr = this->aggregate_pointer(rhs);

      if (lhs.is_scalar()) {
        ikos_assert_msg(lhs.scalar().is_var(),
                        "left hand side is not a variable");

        if (lhs.scalar().is_pointer_var()) {
          this->_csts.add(LoadCst::create(lhs.scalar().var(),
                                          VarOp::create(rhs_ptr, zero())));
        }
      } else if (lhs.is_aggregate()) {
        ikos_assert_msg(lhs.aggregate().is_var(),
                        "left hand side is not a variable");

        Variable* lhs_ptr = this->aggregate_pointer(lhs.aggregate());
        this->mem_copy(lhs_ptr, rhs_ptr);
      } else {
        ikos_unreachable("unexpected left hand side");
      }
    }

    void operator()(ar::InsertElement* s) {
      const AggregateLit& lhs = this->_lit_factory.get_aggregate(s->result());
      const AggregateLit& rhs =
          this->_lit_factory.get_aggregate(s->aggregate());
      const Literal& element = this->_lit_factory.get(s->element());
      ikos_assert_msg(lhs.is_var(), "left hand side is not a variable");

      Variable* lhs_ptr = this->aggregate_pointer(lhs);

      // first, copy the aggregate value
      this->mem_write_aggregate(lhs_ptr, rhs);

      // then insert the element
      if (element.is_scalar()) {
        if (element.scalar().is_pointer_var()) {
          this->_csts.add(
              StoreCst::create(lhs_ptr,
                               VarOp::create(element.scalar().var(), zero())));
        }
      } else if (element.is_aggregate()) {
        this->mem_write_aggregate(lhs_ptr, element.aggregate());
      } else {
        ikos_unreachable("unexpected element operand");
      }
    }

    void operator()(ar::ShuffleVector* s) {
      const AggregateLit& lhs = this->_lit_factory.get_aggregate(s->result());
      const AggregateLit& left = this->_lit_factory.get_aggregate(s->left());
      const AggregateLit& right = this->_lit_factory.get_aggregate(s->right());
      ikos_assert_msg(lhs.is_var(), "left hand side is not a variable");

      Variable* lhs_ptr = this->aggregate_pointer(lhs);
      this->mem_write_aggregate(lhs_ptr, left);
      this->mem_write_aggregate(lhs_ptr, right);
    }

    void operator()(ar::LandingPad*) {}

    void operator()(ar::Resume*) {}

    void operator()(ar::CallBase* s) {
      // Collect potential callees
      std::vector< MemoryLocation* > callees;

      ar::Value* called = s->called();
      if (auto ptr = dyn_cast< ar::InternalVariable >(called)) {
        Variable* ptr_var = _ctx.var_factory->get_internal(ptr);

        if (!this->_function_pointer) {
          return;
        }

        PointsToSet points_to =
            this->_function_pointer->get(ptr_var).points_to();

        if (points_to.is_top() || points_to.is_bottom()) {
          return;
        }

        std::copy(points_to.begin(),
                  points_to.end(),
                  std::back_inserter(callees));
      } else if (auto cst = dyn_cast< ar::FunctionPointerConstant >(called)) {
        callees.push_back(_ctx.mem_factory->get_function(cst->function()));
      } else {
        // undefined, null, inline asm, global or local variable
        return;
      }

      // Add pointer constraints for each possible callee
      for (MemoryLocation* mem : callees) {
        // Check if the callee is a function
        if (!isa< FunctionMemoryLocation >(mem)) {
          continue;
        }

        ar::Function* fun = cast< FunctionMemoryLocation >(mem)->function();

        if (!ar::TypeVerifier::is_valid_call(s, fun->type())) {
          // Ill-formed function call, ignore callee.
          // This could be because of an imprecision of the pointer analysis.
          continue;
        }

        if (fun->is_declaration()) {
          // External call
          this->process_extern_call(s, fun);
        } else if (fun->is_definition()) {
          this->process_intern_call(s, fun);
        } else {
          ikos_unreachable("unreachable");
        }
      }
    }

    void process_extern_call(ar::CallBase* s, ar::Function* fun) {
      if (fun->is_intrinsic()) {
        this->process_intrinsic_call(s, fun->intrinsic_id());
      } else {
        // nothing to do
      }
    }

    void process_intrinsic_call(ar::CallBase* s, ar::Intrinsic::ID id) {
      switch (id) {
        case ar::Intrinsic::MemoryCopy:
        case ar::Intrinsic::MemoryMove: {
          this->process_mem_copy(s);
        } break;
        case ar::Intrinsic::MemorySet:
        case ar::Intrinsic::VarArgStart:
        case ar::Intrinsic::VarArgEnd:
        case ar::Intrinsic::VarArgGet:
        case ar::Intrinsic::VarArgCopy:
        case ar::Intrinsic::StackSave:
        case ar::Intrinsic::StackRestore:
        case ar::Intrinsic::LifetimeStart:
        case ar::Intrinsic::LifetimeEnd:
        case ar::Intrinsic::EhTypeidFor:
        case ar::Intrinsic::Trap:
          break; // do nothing
        // <ikos/analyzer/intrinsic.h>
        case ar::Intrinsic::IkosAssert:
        case ar::Intrinsic::IkosAssume:
        case ar::Intrinsic::IkosNonDet:
        case ar::Intrinsic::IkosCounterInit:
        case ar::Intrinsic::IkosCounterIncr:
        case ar::Intrinsic::IkosCheckMemAccess:
        case ar::Intrinsic::IkosCheckStringAccess:
        case ar::Intrinsic::IkosAssumeMemSize:
        case ar::Intrinsic::IkosForgetMemory:
        case ar::Intrinsic::IkosAbstractMemory:
        case ar::Intrinsic::IkosWatchMemory:
        case ar::Intrinsic::IkosPartitioningVar:
        case ar::Intrinsic::IkosPartitioningJoin:
        case ar::Intrinsic::IkosPartitioningDisable:
        case ar::Intrinsic::IkosPrintInvariant:
        case ar::Intrinsic::IkosPrintValues:
          break; // do nothing
        // <stdlib.h>
        case ar::Intrinsic::LibcMalloc:
        case ar::Intrinsic::LibcCalloc:
        case ar::Intrinsic::LibcValloc:
        case ar::Intrinsic::LibcAlignedAlloc:
        case ar::Intrinsic::LibcRealloc: {
          this->process_dyn_alloc(s);
        } break;
        case ar::Intrinsic::LibcFree:
        case ar::Intrinsic::LibcAbs:
        case ar::Intrinsic::LibcRand:
        case ar::Intrinsic::LibcSrand:
        case ar::Intrinsic::LibcExit:
        case ar::Intrinsic::LibcAbort:
          break; // do nothing
        // <errno.h>
        case ar::Intrinsic::LibcErrnoLocation: {
          this->process_errno_location(s);
        } break;
        // <fcntl.h>
        case ar::Intrinsic::LibcOpen:
          break; // do nothing
        // <unistd.h>
        case ar::Intrinsic::LibcClose:
        case ar::Intrinsic::LibcRead:
        case ar::Intrinsic::LibcWrite:
          break; // do nothing
        // <stdio.h>
        case ar::Intrinsic::LibcGets:
        case ar::Intrinsic::LibcFgets: {
          this->assign_call_result(s, s->argument(0));
        } break;
        case ar::Intrinsic::LibcGetc:
        case ar::Intrinsic::LibcFgetc:
        case ar::Intrinsic::LibcGetchar:
        case ar::Intrinsic::LibcPuts:
        case ar::Intrinsic::LibcFputs:
        case ar::Intrinsic::LibcPutc:
        case ar::Intrinsic::LibcFputc:
        case ar::Intrinsic::LibcPrintf:
        case ar::Intrinsic::LibcFprintf:
        case ar::Intrinsic::LibcSprintf:
        case ar::Intrinsic::LibcSnprintf:
        case ar::Intrinsic::LibcScanf:
        case ar::Intrinsic::LibcFscanf:
        case ar::Intrinsic::LibcSscanf:
          break; // do nothing
        case ar::Intrinsic::LibcFopen: {
          this->process_dyn_alloc(s);
        } break;
        case ar::Intrinsic::LibcFclose:
        case ar::Intrinsic::LibcFflush:
          break; // do nothing
        // <string.h>
        case ar::Intrinsic::LibcStrlen:
        case ar::Intrinsic::LibcStrnlen:
          break; // do nothing
        case ar::Intrinsic::LibcStrcpy:
        case ar::Intrinsic::LibcStrncpy:
        case ar::Intrinsic::LibcStrcat:
        case ar::Intrinsic::LibcStrncat: {
          this->assign_call_result(s, s->argument(0));
        } break;
        case ar::Intrinsic::LibcStrcmp:
        case ar::Intrinsic::LibcStrncmp:
          break; // do nothing
        case ar::Intrinsic::LibcStrstr:
        case ar::Intrinsic::LibcStrchr: {
          this->process_string_search(s);
        } break;
        case ar::Intrinsic::LibcStrdup:
        case ar::Intrinsic::LibcStrndup: {
          this->process_dyn_alloc(s);
        } break;
        case ar::Intrinsic::LibcStrcpyCheck: {
          this->assign_call_result(s, s->argument(0));
        } break;
        case ar::Intrinsic::LibcMemoryCopyCheck:
        case ar::Intrinsic::LibcMemoryMoveCheck: {
          this->process_mem_copy(s);
          this->assign_call_result(s, s->argument(0));
        } break;
        case ar::Intrinsic::LibcMemorySetCheck:
        case ar::Intrinsic::LibcStrcatCheck: {
          this->assign_call_result(s, s->argument(0));
        } break;
        case ar::Intrinsic::LibcppNew:
        case ar::Intrinsic::LibcppNewArray: {
          this->process_dyn_alloc(s);
        } break;
        case ar::Intrinsic::LibcppDelete:
        case ar::Intrinsic::LibcppDeleteArray:
          break; // do nothing
        case ar::Intrinsic::LibcppAllocateException: {
          this->process_dyn_alloc(s);
        } break;
        case ar::Intrinsic::LibcppFreeException:
        case ar::Intrinsic::LibcppThrow:
          break; // do nothing
        case ar::Intrinsic::LibcppBeginCatch: {
          this->assign_call_result(s, s->argument(0));
        } break;
        case ar::Intrinsic::LibcppEndCatch:
          break; // do nothing
        default: {
          ikos_unreachable("unreachable");
        } break;
      }
    }

    void process_mem_copy(ar::CallBase* s) {
      const ScalarLit& dest = this->_lit_factory.get_scalar(s->argument(0));
      const ScalarLit& src = this->_lit_factory.get_scalar(s->argument(1));

      if (!src.is_pointer_var() || !dest.is_pointer_var()) {
        return;
      }

      this->mem_copy(dest.var(), src.var());
    }

    void process_dyn_alloc(ar::CallBase* s) {
      if (!s->has_result()) {
        return;
      }

      // This analysis is context insensitive
      CallContext* context = _ctx.call_context_factory->get_empty();
      MemoryLocation* dyn_addr = _ctx.mem_factory->get_dyn_alloc(s, context);
      Variable* var = _ctx.var_factory->get_internal(s->result());
      this->_csts.add(AssignCst::create(var, AddrOp::create(dyn_addr, zero())));
    }

    void assign_call_result(ar::CallBase* s, ar::Value* operand) {
      if (!s->has_result()) {
        return;
      }

      this->assign(this->_lit_factory.get(s->result()),
                   this->_lit_factory.get(operand));
    }

    void process_string_search(ar::CallBase* s) {
      if (!s->has_result()) {
        return;
      }

      const ScalarLit& lhs = this->_lit_factory.get_scalar(s->result());
      const ScalarLit& ptr = this->_lit_factory.get_scalar(s->argument(0));

      if (!lhs.is_pointer_var() || !ptr.is_pointer_var()) {
        return;
      }

      auto top = MachineIntInterval::top(this->_data_layout.pointers.bit_width,
                                         Unsigned);
      this->_csts.add(
          AssignCst::create(lhs.var(), VarOp::create(ptr.var(), top)));
    }

    void process_errno_location(ar::CallBase* s) {
      if (!s->has_result()) {
        return;
      }

      MemoryLocation* addr = _ctx.mem_factory->get_libc_errno();
      Variable* var = _ctx.var_factory->get_internal(s->result());
      this->_csts.add(AssignCst::create(var, AddrOp::create(addr, zero())));
    }

    void process_intern_call(ar::CallBase* s, ar::Function* fun) {
      // Handle parameters
      auto param_it = fun->param_begin();
      auto param_et = fun->param_end();
      auto arg_it = s->arg_begin();
      auto arg_et = s->arg_end();
      for (; param_it != param_et && arg_it != arg_et; ++param_it, ++arg_it) {
        this->assign(this->_lit_factory.get(*param_it),
                     this->_lit_factory.get(*arg_it));
      }

      // Handle result
      if (!s->has_result()) {
        return;
      }

      Variable* ret_var = _ctx.var_factory->get_return(fun);
      const Literal& lhs = this->_lit_factory.get(s->result());

      if (lhs.is_scalar()) {
        if (lhs.scalar().is_pointer_var()) {
          this->_csts.add(AssignCst::create(lhs.scalar().var(),
                                            VarOp::create(ret_var, zero())));
        }
      } else if (lhs.is_aggregate()) {
        Variable* lhs_ptr = this->aggregate_pointer(lhs.aggregate());
        this->_csts.add(
            AssignCst::create(ret_var, VarOp::create(lhs_ptr, zero())));
      } else {
        ikos_unreachable("unreachable");
      }
    }

  }; // end class BasicBlockVisitor

  /// @}

}; // end class PointerConstraintsGenerator

} // end namespace analyzer
} // end namespace ikos
