/*******************************************************************************
 *
 * \file
 * \brief Buffer overflow checker implementation
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2018 United States Government as represented by the
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

#include <ikos/analyzer/checker/buffer_overflow.hpp>
#include <ikos/analyzer/json/helper.hpp>
#include <ikos/analyzer/support/cast.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {

BufferOverflowChecker::BufferOverflowChecker(Context& ctx)
    : Checker(ctx),
      _ar_context(ctx.bundle->context()),
      _data_layout(ctx.bundle->data_layout()),
      _offset_type(ar::IntegerType::size_type(ctx.bundle)),
      _size_one(
          ar::IntegerConstant::get(this->_ar_context, this->_offset_type, 1)) {}

CheckerName BufferOverflowChecker::name() const {
  return CheckerName::BufferOverflow;
}

const char* BufferOverflowChecker::description() const {
  return "Buffer overflow checker";
}

/// \brief Return true if `lit >= n`
static bool is_greater_equal(const ScalarLit& lit,
                             const MachineInt& n,
                             const value::AbstractDomain& inv) {
  if (lit.is_undefined()) {
    return false;
  } else if (lit.is_machine_int()) {
    return lit.machine_int() >= n;
  } else if (lit.is_machine_int_var()) {
    if (inv.is_normal_flow_bottom()) {
      return false;
    }

    value::AbstractDomain tmp(inv);
    tmp.normal().integers().add(core::machine_int::Predicate::LT, lit.var(), n);
    return tmp.is_normal_flow_bottom();
  } else {
    ikos_unreachable("unreachable");
  }
}

void BufferOverflowChecker::check(ar::Statement* stmt,
                                  const value::AbstractDomain& inv,
                                  CallContext* call_context) {
  if (auto load = dyn_cast< ar::Load >(stmt)) {
    this->check_mem_access(load,
                           load->operand(),
                           this->store_size(load->result()->type()),
                           inv,
                           call_context);
  } else if (auto store = dyn_cast< ar::Store >(stmt)) {
    this->check_mem_access(store,
                           store->pointer(),
                           this->store_size(store->value()->type()),
                           inv,
                           call_context);
  } else if (auto memcpy = dyn_cast< ar::MemoryCopy >(stmt)) {
    this->check_mem_access(memcpy,
                           memcpy->source(),
                           memcpy->length(),
                           inv,
                           call_context);
    this->check_mem_access(memcpy,
                           memcpy->destination(),
                           memcpy->length(),
                           inv,
                           call_context);
  } else if (auto memmove = dyn_cast< ar::MemoryMove >(stmt)) {
    this->check_mem_access(memmove,
                           memmove->source(),
                           memmove->length(),
                           inv,
                           call_context);
    this->check_mem_access(memmove,
                           memmove->destination(),
                           memmove->length(),
                           inv,
                           call_context);
  } else if (auto memset = dyn_cast< ar::MemorySet >(stmt)) {
    this->check_mem_access(memset,
                           memset->pointer(),
                           memset->length(),
                           inv,
                           call_context);
  } else if (auto call = dyn_cast< ar::IntrinsicCall >(stmt)) {
    ar::Function* fun = call->called_function();

    switch (fun->intrinsic_id()) {
      /// IKOS does not keep track of the string length (which is different from
      /// the allocated size), thus it is hard to check if these function calls
      /// are safe or not.
      ///
      /// In most cases here, we just check if the first byte is accessible.
      case ar::Intrinsic::LibcStrlen: {
        this->check_mem_access(call,
                               call->argument(0),
                               this->_size_one,
                               inv,
                               call_context);
      } break;
      case ar::Intrinsic::LibcStrnlen: {
        const ScalarLit& n = this->_lit_factory.get_scalar(call->argument(1));
        if (is_greater_equal(n, this->_size_one->value(), inv)) {
          this->check_mem_access(call,
                                 call->argument(0),
                                 this->_size_one,
                                 inv,
                                 call_context);
        }
      } break;
      case ar::Intrinsic::LibcStrcpy: {
        this->check_mem_access(call,
                               call->argument(0),
                               this->_size_one,
                               inv,
                               call_context);
        this->check_mem_access(call,
                               call->argument(1),
                               this->_size_one,
                               inv,
                               call_context);
        this->check_strcpy(call,
                           call->argument(0),
                           call->argument(1),
                           inv,
                           call_context);
      } break;
      case ar::Intrinsic::LibcStrncpy: {
        const ScalarLit& n = this->_lit_factory.get_scalar(call->argument(2));
        if (is_greater_equal(n, this->_size_one->value(), inv)) {
          this->check_mem_access(call,
                                 call->argument(0),
                                 this->_size_one,
                                 inv,
                                 call_context);
          this->check_mem_access(call,
                                 call->argument(1),
                                 this->_size_one,
                                 inv,
                                 call_context);
          // TODO(marthaud): check_strncpy
        }
      } break;
      case ar::Intrinsic::LibcStrcat: {
        this->check_mem_access(call,
                               call->argument(0),
                               this->_size_one,
                               inv,
                               call_context);
        this->check_mem_access(call,
                               call->argument(1),
                               this->_size_one,
                               inv,
                               call_context);
      } break;
      case ar::Intrinsic::LibcStrncat: {
        const ScalarLit& n = this->_lit_factory.get_scalar(call->argument(2));
        if (is_greater_equal(n, this->_size_one->value(), inv)) {
          this->check_mem_access(call,
                                 call->argument(0),
                                 this->_size_one,
                                 inv,
                                 call_context);
          this->check_mem_access(call,
                                 call->argument(1),
                                 this->_size_one,
                                 inv,
                                 call_context);
        }
      } break;
      default: {
        break;
      }
    }
  }
}

void BufferOverflowChecker::check_mem_access(ar::Statement* stmt,
                                             ar::Value* pointer,
                                             ar::Value* access_size,
                                             const value::AbstractDomain& inv,
                                             CallContext* call_context) {
  CheckResult check = this->check_mem_access(stmt, pointer, access_size, inv);
  this->display_invariant(check.result, stmt, inv);
  this->_checks.insert(check.kind,
                       CheckerName::BufferOverflow,
                       check.result,
                       stmt,
                       call_context,
                       check.operands,
                       check.info);
}

BufferOverflowChecker::CheckResult BufferOverflowChecker::check_mem_access(
    ar::Statement* stmt,
    ar::Value* pointer,
    ar::Value* access_size,
    value::AbstractDomain inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (this->display_mem_access_check(Result::Unreachable,
                                       stmt,
                                       pointer,
                                       access_size)) {
      out() << std::endl;
    }
    return {CheckKind::Unreachable, Result::Unreachable, {}, {}};
  }

  const ScalarLit& ptr = this->_lit_factory.get_scalar(pointer);
  const ScalarLit& size = this->_lit_factory.get_scalar(access_size);

  // Check uninitialized

  if (ptr.is_undefined() ||
      (ptr.is_pointer_var() &&
       inv.normal().uninitialized().is_uninitialized(ptr.var()))) {
    // Undefined pointer operand
    if (this->display_mem_access_check(Result::Error,
                                       stmt,
                                       pointer,
                                       access_size)) {
      out() << ": undefined pointer operand" << std::endl;
    }
    return {CheckKind::UninitializedVariable, Result::Error, {pointer}, {}};
  }

  if (size.is_undefined() ||
      (size.is_machine_int_var() &&
       inv.normal().uninitialized().is_uninitialized(size.var()))) {
    // Undefined pointer operand
    if (this->display_mem_access_check(Result::Error,
                                       stmt,
                                       pointer,
                                       access_size)) {
      out() << ": undefined size operand" << std::endl;
    }
    return {CheckKind::UninitializedVariable, Result::Error, {access_size}, {}};
  }

  // Check null pointer dereference

  if (ptr.is_null() ||
      (ptr.is_pointer_var() && inv.normal().nullity().is_null(ptr.var()))) {
    // Null pointer operand
    if (this->display_mem_access_check(Result::Error,
                                       stmt,
                                       pointer,
                                       access_size)) {
      out() << ": null pointer dereference" << std::endl;
    }
    return {CheckKind::NullPointerDereference, Result::Error, {pointer}, {}};
  }

  // Check unexpected operand
  if (!ptr.is_pointer_var()) {
    log::error("unexpected pointer operand");
    return {CheckKind::UnexpectedOperand, Result::Error, {pointer}, {}};
  }
  if (!size.is_machine_int() && !size.is_machine_int_var()) {
    log::error("unexpected size operand");
    return {CheckKind::UnexpectedOperand, Result::Error, {access_size}, {}};
  }

  // Initialize global variable pointer and function pointer
  this->init_global_ptr(pointer, inv);

  // Variable representing the pointer offset
  Variable* offset_var = inv.normal().pointers().offset_var(ptr.var());

  // Points-to set of the pointer
  PointsToSet addrs = inv.normal().pointers().points_to(ptr.var());

  if (addrs.is_empty()) {
    // Pointer is invalid
    if (this->display_mem_access_check(Result::Error,
                                       stmt,
                                       pointer,
                                       access_size)) {
      out() << ": empty points-to set for pointer" << std::endl;
    }
    return {CheckKind::InvalidPointerDereference, Result::Error, {pointer}, {}};
  } else if (addrs.is_top()) {
    // Unknown points-to set
    if (this->display_mem_access_check(Result::Warning,
                                       stmt,
                                       pointer,
                                       access_size)) {
      out() << ": no points-to information for pointer" << std::endl;
    }
    return {CheckKind::UnknownMemoryAccess, Result::Warning, {pointer}, {}};
  }

  JsonDict info;
  JsonList points_to_info;

  IntInterval offset_intv = inv.normal().integers().to_interval(offset_var);
  info.put("offset", to_json(offset_intv));

  // Add a shadow variable `offset_plus_size = offset + access_size`
  Variable* offset_plus_size =
      _ctx.var_factory->get_named_shadow(this->_offset_type,
                                         "shadow.offset_plus_size");

  IntInterval size_intv;
  if (size.is_machine_int_var()) {
    size_intv = inv.normal().integers().to_interval(size.var());
    inv.normal().integers().apply(IntBinaryOperator::Add,
                                  offset_plus_size,
                                  offset_var,
                                  size.var());
  } else if (size.is_machine_int()) {
    size_intv = IntInterval(size.machine_int());
    inv.normal().integers().apply(IntBinaryOperator::Add,
                                  offset_plus_size,
                                  offset_var,
                                  size.machine_int());
  } else {
    ikos_unreachable("unexpected access size");
  }
  info.put("access_size", to_json(size_intv));

  if (auto element_size =
          this->is_array_access(stmt, inv, offset_intv, addrs)) {
    info.put("array_element_size", *element_size);
  }

  // Are all the points-to in/valid
  bool all_valid = true;
  bool all_invalid = true;

  for (auto addr : addrs) {
    AllocSizeVariable* size_var = _ctx.var_factory->get_alloc_size(addr);
    this->init_global_alloc_size(addr, size_var, inv);

    // add block info
    JsonDict block_info = {
        {"id", _ctx.output_db->memory_locations.insert(addr)}};

    // perform analysis
    auto result_pair = this->check_memory_location_access(stmt,
                                                          pointer,
                                                          access_size,
                                                          inv,
                                                          addr,
                                                          size_var,
                                                          offset_var,
                                                          offset_plus_size,
                                                          offset_intv,
                                                          block_info);

    block_info.put("status", static_cast< int >(result_pair.first));
    block_info.put("kind", static_cast< int >(result_pair.second));

    if (result_pair.first == Result::Error) {
      all_valid = false;
    } else if (result_pair.first == Result::Warning) {
      all_valid = false;
      all_invalid = false;
    } else {
      all_invalid = false;
    }

    points_to_info.add(block_info);
  }

  info.put("points_to", points_to_info);

  if (all_invalid) {
    return {CheckKind::BufferOverflow,
            Result::Error,
            {pointer, access_size},
            info};
  } else if (!all_valid) {
    return {CheckKind::BufferOverflow,
            Result::Warning,
            {pointer, access_size},
            info};
  } else {
    return {CheckKind::BufferOverflow, Result::Ok, {pointer, access_size}, {}};
  }
}

std::pair< Result, BufferOverflowChecker::BufferOverflowCheckKind >
BufferOverflowChecker::check_memory_location_access(
    ar::Statement* stmt,
    ar::Value* pointer,
    ar::Value* access_size,
    const value::AbstractDomain& inv,
    MemoryLocation* addr,
    AllocSizeVariable* size_var,
    Variable* offset_var,
    Variable* offset_plus_size,
    const IntInterval& offset_intv,
    JsonDict& block_info) {
  if (isa< FunctionMemoryLocation >(addr)) {
    // Try to dereference a function pointer, this is an error
    if (this->display_mem_access_check(Result::Error,
                                       stmt,
                                       pointer,
                                       access_size,
                                       addr)) {
      out() << ": dereferencing a function pointer" << std::endl;
    }
    return {Result::Error, BufferOverflowCheckKind::Function};
  }

  if (isa< DynAllocMemoryLocation >(addr)) {
    // Dynamic allocated memory location
    // Check for use after free

    auto lifetime = inv.normal().lifetime().get(addr);

    if (lifetime.is_deallocated()) {
      // Use after free
      if (this->display_mem_access_check(Result::Error,
                                         stmt,
                                         pointer,
                                         access_size,
                                         addr)) {
        out() << ": use after free" << std::endl;
      }
      return {Result::Error, BufferOverflowCheckKind::UseAfterFree};
    } else if (lifetime.is_top()) {
      // Possible use after free
      if (this->display_mem_access_check(Result::Warning,
                                         stmt,
                                         pointer,
                                         access_size,
                                         addr)) {
        out() << ": possible use after free" << std::endl;
      }
      return {Result::Warning, BufferOverflowCheckKind::UseAfterFree};
    } else {
      ikos_assert(lifetime.is_allocated());
    }
  }

  if (isa< LocalMemoryLocation >(addr)) {
    // Stack memory location
    // Check for dangling stack pointer

    auto lifetime = inv.normal().lifetime().get(addr);

    if (lifetime.is_deallocated()) {
      // Access to a dangling stack pointer
      if (this->display_mem_access_check(Result::Error,
                                         stmt,
                                         pointer,
                                         access_size,
                                         addr)) {
        out() << ": access to a dangling stack pointer" << std::endl;
      }
      return {Result::Error, BufferOverflowCheckKind::UseAfterReturn};
    } else if (lifetime.is_top()) {
      // Possible access to a dangling stack pointer
      if (this->display_mem_access_check(Result::Warning,
                                         stmt,
                                         pointer,
                                         access_size,
                                         addr)) {
        out() << ": possible access to a dangling stack pointer" << std::endl;
      }
      return {Result::Warning, BufferOverflowCheckKind::UseAfterReturn};
    } else {
      ikos_assert(lifetime.is_allocated());
    }
  }

  if (isa< AbsoluteZeroMemoryLocation >(addr)) {
    // Checks: hardware addresses

    // Compute the writable interval for offset o ([o, o + access_size])
    auto offset_plus_size_intv =
        inv.normal().integers().to_interval(offset_plus_size);
    auto one = IntInterval(MachineInt(1, offset_intv.bit_width(), Unsigned));
    auto last_byte_offset_intv = sub_no_wrap(offset_plus_size_intv, one);
    auto writable_interval = last_byte_offset_intv.join(offset_intv);

    if (_ctx.opts.hardware_addresses.geq(writable_interval)) {
      // The offset_var is completely included in an hardware address range
      // specified by the user, so we're Ok
      if (this->display_mem_access_check(Result::Ok,
                                         stmt,
                                         pointer,
                                         access_size,
                                         addr)) {
        out() << "[hardware addresses]: ∀o ∈ offset, o <= ";
        access_size->dump(out());
        out() << " && o + access_size <= ";
        access_size->dump(out());
        out() << std::endl;
      }
      return {Result::Ok, BufferOverflowCheckKind::HardwareAddresses};
    } else if (_ctx.opts.hardware_addresses.is_meet_bottom(offset_intv) ||
               _ctx.opts.hardware_addresses.is_meet_bottom(
                   last_byte_offset_intv)) {
      // The offset_var isn't included in an hardware address range at all.
      // This is an error
      if (this->display_mem_access_check(Result::Error,
                                         stmt,
                                         pointer,
                                         access_size,
                                         addr)) {
        out() << "[hardware addresses]: ∀o ∈ offset, o > ";
        access_size->dump(out());
        out() << " || o + access_size > ";
        access_size->dump(out());
        out() << std::endl;
      }
      return {Result::Error, BufferOverflowCheckKind::HardwareAddresses};
    } else {
      // The offset_var isn't completely included in an hardware address range
      // specified by the user, so it could overflow somewhere
      // This is a warning
      if (this->display_mem_access_check(Result::Warning,
                                         stmt,
                                         pointer,
                                         access_size,
                                         addr)) {
        out() << "[hardware addresses]: ∃o ∈ offset, o > ";
        access_size->dump(out());
        out() << " || o + access_size > ";
        access_size->dump(out());
        out() << std::endl;
      }
      return {Result::Warning, BufferOverflowCheckKind::HardwareAddresses};
    }
  }

  // add `size` (min, max) to block_info
  IntInterval size_intv = inv.normal().integers().to_interval(size_var);
  block_info.put("size", to_json(size_intv));

  // add `offset + access_size - size` (min, max) to block_info
  MachineInt zero(0, this->_data_layout.pointers.bit_width, Unsigned);
  MachineInt one(1, this->_data_layout.pointers.bit_width, Unsigned);
  IntLinearExpression expr(zero);
  expr.add(one, offset_plus_size);
  expr.add(-one, size_var);
  IntInterval diff_intv = inv.normal().integers().to_interval(expr);
  block_info.put("diff", to_json(diff_intv));

  // Checks: `offset > mem_size || offset + access_size > mem_size`
  value::AbstractDomain tmp1(inv);
  tmp1.normal().integers().add(IntPredicate::GT, offset_var, size_var);

  value::AbstractDomain tmp2(inv);
  tmp2.normal().integers().add(IntPredicate::GT, offset_plus_size, size_var);

  bool is_bottom = tmp1.is_normal_flow_bottom() && tmp2.is_normal_flow_bottom();

  if (is_bottom) {
    // offset_var <= size_var and offset_plus_size <= size_var, so we're
    // safe here
    if (this->display_mem_access_check(Result::Ok,
                                       stmt,
                                       pointer,
                                       access_size,
                                       addr)) {
      out() << ": ∀o ∈ offset, o <= ";
      access_size->dump(out());
      out() << " && o + access_size <= ";
      access_size->dump(out());
      out() << std::endl;
    }
    return {Result::Ok, BufferOverflowCheckKind::OutOfBound};
  }

  // Check: `offset <= mem_size && offset + access_size <= mem_size`
  value::AbstractDomain tmp3(inv);
  tmp3.normal().integers().add(IntPredicate::LE, offset_var, size_var);
  tmp3.normal().integers().add(IntPredicate::LE, offset_plus_size, size_var);
  is_bottom = tmp3.is_normal_flow_bottom();

  if (is_bottom) {
    if (this->display_mem_access_check(Result::Error,
                                       stmt,
                                       pointer,
                                       access_size,
                                       addr)) {
      out() << ": ∀o ∈ offset, o > ";
      access_size->dump(out());
      out() << " || o + access_size > ";
      access_size->dump(out());
      out() << std::endl;
    }
    return {Result::Error, BufferOverflowCheckKind::OutOfBound};
  } else {
    if (this->display_mem_access_check(Result::Warning,
                                       stmt,
                                       pointer,
                                       access_size,
                                       addr)) {
      out() << ": ∃o ∈ offset, o <= ";
      access_size->dump(out());
      out() << " && o + access_size <= ";
      access_size->dump(out());
      out() << std::endl;
    }
    return {Result::Warning, BufferOverflowCheckKind::OutOfBound};
  }
}

void BufferOverflowChecker::check_strcpy(ar::Statement* stmt,
                                         ar::Value* dest_op,
                                         ar::Value* src_op,
                                         const value::AbstractDomain& inv,
                                         CallContext* call_context) {
  CheckResult check = this->check_strcpy(stmt, dest_op, src_op, inv);
  this->display_invariant(check.result, stmt, inv);
  this->_checks.insert(check.kind,
                       CheckerName::BufferOverflow,
                       check.result,
                       stmt,
                       call_context,
                       check.operands,
                       check.info);
}

BufferOverflowChecker::CheckResult BufferOverflowChecker::check_strcpy(
    ar::Statement* stmt,
    ar::Value* dest_op,
    ar::Value* src_op,
    value::AbstractDomain inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (this->display_strcpy_check(Result::Unreachable,
                                   stmt,
                                   dest_op,
                                   src_op)) {
      out() << std::endl;
    }
    return {CheckKind::Unreachable, Result::Unreachable, {}, {}};
  }

  const ScalarLit& dest = this->_lit_factory.get_scalar(dest_op);
  const ScalarLit& src = this->_lit_factory.get_scalar(src_op);

  // Check uninitialized

  if (src.is_undefined() ||
      (src.is_pointer_var() &&
       inv.normal().uninitialized().is_uninitialized(src.var()))) {
    // Undefined source pointer operand
    if (this->display_strcpy_check(Result::Error, stmt, dest_op, src_op)) {
      out() << ": undefined source pointer" << std::endl;
    }
    return {CheckKind::UninitializedVariable, Result::Error, {src_op}, {}};
  }

  if (dest.is_undefined() ||
      (dest.is_pointer_var() &&
       inv.normal().uninitialized().is_uninitialized(dest.var()))) {
    // Undefined destination pointer operand
    if (this->display_strcpy_check(Result::Error, stmt, dest_op, src_op)) {
      out() << ": undefined destination pointer" << std::endl;
    }
    return {CheckKind::UninitializedVariable, Result::Error, {dest_op}, {}};
  }

  // Check null pointer dereference

  if (src.is_null() ||
      (src.is_pointer_var() && inv.normal().nullity().is_null(src.var()))) {
    // Null source pointer operand
    if (this->display_mem_access_check(Result::Error, stmt, dest_op, src_op)) {
      out() << ": null source pointer" << std::endl;
    }
    return {CheckKind::NullPointerDereference, Result::Error, {src_op}, {}};
  }

  if (dest.is_null() ||
      (dest.is_pointer_var() && inv.normal().nullity().is_null(dest.var()))) {
    // Null destination pointer operand
    if (this->display_mem_access_check(Result::Error, stmt, dest_op, src_op)) {
      out() << ": null destination pointer" << std::endl;
    }
    return {CheckKind::NullPointerDereference, Result::Error, {dest_op}, {}};
  }

  // Check unexpected operand
  if (!src.is_pointer_var()) {
    log::error("unexpected source pointer operand");
    return {CheckKind::UnexpectedOperand, Result::Error, {src_op}, {}};
  }
  if (!dest.is_pointer_var()) {
    log::error("unexpected destination pointer operand");
    return {CheckKind::UnexpectedOperand, Result::Error, {dest_op}, {}};
  }

  // Initialize global variable pointers and function pointers
  this->init_global_ptr(dest_op, inv);
  this->init_global_ptr(src_op, inv);

  PointsToSet dest_addrs = inv.normal().pointers().points_to(dest.var());
  PointsToSet src_addrs = inv.normal().pointers().points_to(src.var());

  if (src_addrs.is_empty()) {
    // Source pointer is invalid
    if (this->display_strcpy_check(Result::Error, stmt, dest_op, src_op)) {
      out() << ": empty points-to set for source pointer" << std::endl;
    }
    return {CheckKind::InvalidPointerDereference, Result::Error, {src_op}, {}};
  }
  if (dest_addrs.is_empty()) {
    // Destination pointer is invalid
    if (this->display_strcpy_check(Result::Error, stmt, dest_op, src_op)) {
      out() << ": empty points-to set for destination pointer" << std::endl;
    }
    return {CheckKind::InvalidPointerDereference, Result::Error, {dest_op}, {}};
  }
  if (src_addrs.is_top()) {
    // Unknown source points-to set
    if (this->display_strcpy_check(Result::Error, stmt, dest_op, src_op)) {
      out() << ": no points-to information for source pointer" << std::endl;
    }
    return {CheckKind::UnknownMemoryAccess, Result::Warning, {src_op}, {}};
  }
  if (dest_addrs.is_top()) {
    // Unknown source points-to set
    if (this->display_strcpy_check(Result::Error, stmt, dest_op, src_op)) {
      out() << ": no points-to information for destination pointer"
            << std::endl;
    }
    return {CheckKind::UnknownMemoryAccess, Result::Warning, {dest_op}, {}};
  }

  bool all_valid = true;

  for (auto dest_addr : dest_addrs) {
    AllocSizeVariable* dest_size = _ctx.var_factory->get_alloc_size(dest_addr);
    this->init_global_alloc_size(dest_addr, dest_size, inv);
    Variable* dest_offset = inv.normal().pointers().offset_var(dest.var());
    Variable* max_space_available =
        _ctx.var_factory->get_named_shadow(this->_offset_type,
                                           "shadow.max_space_available");

    for (auto src_addr : src_addrs) {
      AllocSizeVariable* src_size = _ctx.var_factory->get_alloc_size(src_addr);
      this->init_global_alloc_size(src_addr, src_size, inv);
      Variable* src_offset = inv.normal().pointers().offset_var(src.var());
      Variable* max_space_needed =
          _ctx.var_factory->get_named_shadow(this->_offset_type,
                                             "shadow.max_space_needed");

      value::AbstractDomain tmp(inv);
      tmp.normal().integers().apply(IntBinaryOperator::Sub,
                                    max_space_available,
                                    dest_size,
                                    dest_offset);
      tmp.normal().integers().apply(IntBinaryOperator::Sub,
                                    max_space_needed,
                                    src_size,
                                    src_offset);
      tmp.normal().integers().add(IntPredicate::GT,
                                  max_space_needed,
                                  max_space_available);
      bool is_bottom = tmp.is_normal_flow_bottom();

      if (is_bottom &&
          this->display_strcpy_check(Result::Ok, stmt, dest_op, src_op)) {
        out() << ": ∀(s, d) ∈ src.offset x dest.offset, ";
        src_size->dump(out());
        out() << " - s <= ";
        dest_size->dump(out());
        out() << " - d" << std::endl;
      } else if (!is_bottom && this->display_strcpy_check(Result::Warning,
                                                          stmt,
                                                          dest_op,
                                                          src_op)) {
        out() << ": ∃(s, d) ∈ src.offset x dest.offset, ";
        src_size->dump(out());
        out() << " - s > ";
        dest_size->dump(out());
        out() << " - d" << std::endl;
      }

      all_valid = all_valid && is_bottom;
    }
  }

  return {CheckKind::StrcpyBufferOverflow,
          all_valid ? Result::Ok : Result::Warning,
          {dest_op, src_op},
          {}};
}

ar::IntegerConstant* BufferOverflowChecker::store_size(ar::Type* type) {
  return ar::IntegerConstant::get(this->_ar_context,
                                  this->_offset_type,
                                  MachineInt(this->_data_layout
                                                 .store_size_in_bytes(type),
                                             this->_offset_type->bit_width(),
                                             this->_offset_type->sign()));
}

void BufferOverflowChecker::init_global_ptr(ar::Value* value,
                                            value::AbstractDomain& inv) {
  if (auto gv = dyn_cast< ar::GlobalVariable >(value)) {
    Variable* ptr = _ctx.var_factory->get_global(gv);
    MemoryLocation* addr = _ctx.mem_factory->get_global(gv);
    inv.normal().pointers().assign_address(ptr,
                                           addr,
                                           core::Nullity::non_null());
    inv.normal().uninitialized().set(ptr, core::Uninitialized::initialized());
  } else if (auto cst = dyn_cast< ar::FunctionPointerConstant >(value)) {
    auto fun = cst->function();
    Variable* ptr = _ctx.var_factory->get_function_ptr(fun);
    MemoryLocation* addr = _ctx.mem_factory->get_function(fun);
    inv.normal().pointers().assign_address(ptr,
                                           addr,
                                           core::Nullity::non_null());
    inv.normal().uninitialized().set(ptr, core::Uninitialized::initialized());
  }
}

void BufferOverflowChecker::init_global_alloc_size(MemoryLocation* addr,
                                                   AllocSizeVariable* size_var,
                                                   value::AbstractDomain& inv) {
  if (auto gv = dyn_cast< GlobalMemoryLocation >(addr)) {
    MachineInt size(this->_data_layout.store_size_in_bytes(
                        gv->global_var()->type()->pointee()),
                    this->_data_layout.pointers.bit_width,
                    Unsigned);
    inv.normal().integers().assign(size_var, size);
  } else if (isa< FunctionMemoryLocation >(addr)) {
    MachineInt size(0, this->_data_layout.pointers.bit_width, Unsigned);
    inv.normal().integers().assign(size_var, size);
  }
}

/// \brief Check whether an interval is a multiple of a number
static bool is_multiple(const core::machine_int::Interval& interval,
                        const MachineInt& n) {
  return (mod(interval.lb(), n).is_zero() || interval.lb().is_min()) &&
         (mod(interval.ub(), n).is_zero() || interval.ub().is_max());
}

boost::optional< MachineInt > BufferOverflowChecker::is_array_access(
    ar::Statement* stmt,
    const value::AbstractDomain& inv,
    const IntInterval& offset_intv,
    const PointsToSet& addrs) const {
  // Use heuristics to determine if it is an array access
  ar::Type* access_type = nullptr;

  // Load or Store
  if (auto load = dyn_cast< ar::Load >(stmt)) {
    access_type = load->result()->type();
  } else if (auto store = dyn_cast< ar::Store >(stmt)) {
    access_type = store->value()->type();
  } else {
    return boost::none;
  }

  MachineInt element_size(this->_data_layout.store_size_in_bytes(access_type),
                          this->_offset_type->bit_width(),
                          this->_offset_type->sign());

  // Offset is a multiple of the element size
  if (!is_multiple(offset_intv, element_size)) {
    return boost::none;
  }

  if (!std::all_of(addrs.begin(), addrs.end(), [&](MemoryLocation* addr) {
        if (auto local = dyn_cast< LocalMemoryLocation >(addr)) {
          // Local variable with an array type
          auto type = local->local_var()->type()->pointee();
          return type->is_array() &&
                 cast< ar::ArrayType >(type)->element_type() == access_type;
        } else if (auto global = dyn_cast< GlobalMemoryLocation >(addr)) {
          // Global variable with an array type
          auto type = global->global_var()->type()->pointee();
          return type->is_array() &&
                 cast< ar::ArrayType >(type)->element_type() == access_type;
        } else if (auto dyn_alloc = dyn_cast< DynAllocMemoryLocation >(addr)) {
          AllocSizeVariable* size_var = _ctx.var_factory->get_alloc_size(addr);
          IntInterval size_intv = inv.normal().integers().to_interval(size_var);

          // At least >= 2 elements
          if (size_intv.ub() <= element_size) {
            return false;
          }

          // Size is a multiple of the element size
          if (!is_multiple(size_intv, element_size)) {
            return false;
          }

          // Check if the next instruction is a bitcast to `access_type*`
          auto call = dyn_alloc->call();

          auto next_stmt = call->next_statement();
          if (next_stmt == nullptr) {
            return false;
          }

          auto unary = dyn_cast< ar::UnaryOperation >(next_stmt);
          return unary != nullptr && unary->operand() == call->result() &&
                 unary->op() == ar::UnaryOperation::Bitcast &&
                 unary->result()->type()->is_pointer() &&
                 cast< ar::PointerType >(unary->result()->type())->pointee() ==
                     access_type;
        }

        return false;
      })) {
    return boost::none;
  }

  return element_size;
}

bool BufferOverflowChecker::display_mem_access_check(
    Result result,
    ar::Statement* stmt,
    ar::Value* pointer,
    ar::Value* access_size) const {
  if (this->display_check(result, stmt)) {
    out() << "check_mem_access(pointer=";
    pointer->dump(out());
    out() << ", access_size=";
    access_size->dump(out());
    out() << ")";
    return true;
  }
  return false;
}

bool BufferOverflowChecker::display_mem_access_check(
    Result result,
    ar::Statement* stmt,
    ar::Value* pointer,
    ar::Value* access_size,
    MemoryLocation* addr) const {
  if (this->display_check(result, stmt)) {
    out() << "check_mem_access(pointer=";
    pointer->dump(out());
    out() << ", access_size=";
    access_size->dump(out());
    out() << ", addr=";
    addr->dump(out());
    out() << ")";
    return true;
  }
  return false;
}

bool BufferOverflowChecker::display_strcpy_check(Result result,
                                                 ar::Statement* stmt,
                                                 ar::Value* dest_op,
                                                 ar::Value* src_op) const {
  if (this->display_check(result, stmt)) {
    out() << "check_strcpy(dest=";
    dest_op->dump(out());
    out() << ", src=";
    src_op->dump(out());
    out() << ")";
    return true;
  }
  return false;
}

} // end namespace analyzer
} // end namespace ikos
