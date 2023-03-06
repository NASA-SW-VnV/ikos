/*******************************************************************************
 *
 * \file
 * \brief Pointer alignment checker implementation
 *
 * Author: Clement Decoodt
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

#include <ikos/analyzer/analysis/literal.hpp>
#include <ikos/analyzer/checker/pointer_alignment.hpp>
#include <ikos/analyzer/json/helper.hpp>
#include <ikos/analyzer/support/cast.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {

PointerAlignmentChecker::PointerAlignmentChecker(Context& ctx)
    : Checker(ctx), _data_layout(ctx.bundle->data_layout()) {}

CheckerName PointerAlignmentChecker::name() const {
  return CheckerName::UnalignedPointer;
}

const char* PointerAlignmentChecker::description() const {
  return "pointer alignment checker";
}

void PointerAlignmentChecker::check(ar::Statement* stmt,
                                    const value::AbstractDomain& inv,
                                    CallContext* call_context) {
  if (auto store = dyn_cast< ar::Store >(stmt)) {
    this->check_alignment(store,
                          store->pointer(),
                          store->alignment(),
                          inv,
                          call_context);
  } else if (auto load = dyn_cast< ar::Load >(stmt)) {
    this->check_alignment(load,
                          load->operand(),
                          load->alignment(),
                          inv,
                          call_context);
  } else if (auto memcpy = dyn_cast< ar::MemoryCopy >(stmt)) {
    this->check_alignment(memcpy,
                          memcpy->source(),
                          memcpy->source_alignment(),
                          inv,
                          call_context);
    this->check_alignment(memcpy,
                          memcpy->destination(),
                          memcpy->destination_alignment(),
                          inv,
                          call_context);
  } else if (auto memmove = dyn_cast< ar::MemoryMove >(stmt)) {
    this->check_alignment(memmove,
                          memmove->source(),
                          memmove->source_alignment(),
                          inv,
                          call_context);
    this->check_alignment(memmove,
                          memmove->destination(),
                          memmove->destination_alignment(),
                          inv,
                          call_context);
  } else if (auto memset = dyn_cast< ar::MemorySet >(stmt)) {
    this->check_alignment(memset,
                          memset->pointer(),
                          memset->alignment(),
                          inv,
                          call_context);
  }
}

void PointerAlignmentChecker::check_alignment(ar::Statement* stmt,
                                              ar::Value* operand,
                                              uint64_t alignment_req,
                                              const value::AbstractDomain& inv,
                                              CallContext* call_context) {
  CheckResult check = this->check_alignment(stmt, operand, alignment_req, inv);
  this->display_invariant(check.result, stmt, inv);
  this->_checks.insert(check.kind,
                       CheckerName::UnalignedPointer,
                       check.result,
                       stmt,
                       call_context,
                       std::array< ar::Value*, 1 >{{operand}},
                       check.info);
}

PointerAlignmentChecker::CheckResult PointerAlignmentChecker::check_alignment(
    ar::Statement* stmt,
    ar::Value* operand,
    uint64_t alignment_req,
    const value::AbstractDomain& inv) {
  if (inv.is_normal_flow_bottom()) {
    // Statement unreachable
    if (auto msg =
            this->display_alignment_check(Result::Unreachable, stmt, operand)) {
      *msg << "\n";
    }
    return {CheckKind::Unreachable, Result::Unreachable, {}};
  }

  const ScalarLit& ptr = this->_lit_factory.get_scalar(operand);

  if (ptr.is_undefined() || (ptr.is_pointer_var() &&
                             inv.normal().uninit_is_uninitialized(ptr.var()))) {
    // Undefined operand
    if (auto msg =
            this->display_alignment_check(Result::Error, stmt, operand)) {
      *msg << ": undefined operand\n";
    }
    return {CheckKind::UninitializedVariable, Result::Error, {}};
  }

  if (ptr.is_null() ||
      (ptr.is_pointer_var() && inv.normal().nullity_is_null(ptr.var()))) {
    // Null operand
    if (auto msg =
            this->display_alignment_check(Result::Error, stmt, operand)) {
      *msg << ": null operand\n";
    }
    return {CheckKind::NullPointerDereference, Result::Error, {}};
  }

  if (!ptr.is_pointer_var()) {
    log::error("unexpected pointer operand");
    return {CheckKind::UnexpectedOperand, Result::Error, {}};
  }

  if (alignment_req == 0 || alignment_req == 1) {
    // No alignment requirement found, or always-safe alignment requirement
    if (auto msg = this->display_alignment_check(Result::Ok, stmt, operand)) {
      *msg << ": pointer alignment always safe\n";
    }
    return {CheckKind::UnalignedPointer, Result::Ok, {}};
  }

  // Pointer variable
  Variable* ptr_var = ptr.var();

  // Points-to set of the pointer
  PointsToSet addrs = inv.normal().pointer_to_points_to(ptr_var);

  if (auto gv = dyn_cast< ar::GlobalVariable >(operand)) {
    addrs = PointsToSet{_ctx.mem_factory->get_global(gv)};
  }

  if (addrs.is_empty()) {
    // Pointer is invalid
    if (auto msg =
            this->display_alignment_check(Result::Error, stmt, operand)) {
      *msg << ": empty points-to set for pointer\n";
    }
    return {CheckKind::InvalidPointerDereference, Result::Error, {}};
  }

  if (addrs.is_top()) {
    // Unknown points-to set
    if (auto msg =
            this->display_alignment_check(Result::Warning, stmt, operand)) {
      *msg << ": no points-to information for pointer\n";
    }
    return {CheckKind::UnknownMemoryAccess, Result::Warning, {}};
  }

  Congruence alignment_req_c = to_congruence(alignment_req, 0);
  Congruence offset_c = inv.normal().pointer_offset_to_congruence(ptr_var);

  if (isa< ar::GlobalVariable >(operand)) {
    offset_c = to_congruence(0, 0);
  }

  JsonDict info;
  JsonList points_to_info;

  info.put("requirement", to_json(alignment_req_c));
  info.put("offset", to_json(offset_c));

  // Are all the points-to in/valid
  bool all_valid = true;
  bool all_invalid = true;

  // The goal is to check the following property:
  //   points_to(ptr) != TOP &&
  //   ∀a ∈ points_to_set(ptr), ∀o ∈ offset, a + o ≡ 0 [alignment_req]
  //   (eq. to a + o % alignment_req == 0)
  //
  // Iterate through the points-to set of the pointer
  // - If every memory location has an alignment greater or equal to the
  // requirement, then it's OK.
  // - If every memory location is unaligned based on the requirement,
  // set to ERROR state.
  // - Otherwise, it's a WARNING

  for (MemoryLocation* addr : addrs) {
    // Add info to json
    JsonDict block_info = {
        {"id", _ctx.output_db->memory_locations.insert(addr)}};

    // Is the points_to correctly aligned?
    Result is_correctly_aligned =
        this->check_memory_location_alignment(addr,
                                              offset_c,
                                              alignment_req_c,
                                              block_info);

    if (is_correctly_aligned == Result::Ok) {
      all_invalid = false;

      if (auto msg = this->display_alignment_check(is_correctly_aligned,
                                                   stmt,
                                                   operand)) {
        *msg << ": memory location (";
        addr->dump(msg->stream());
        *msg << ") with offset (" << offset_c << ") is correctly aligned\n";
      }
    } else if (is_correctly_aligned == Result::Error) {
      all_valid = false;

      if (auto msg = this->display_alignment_check(is_correctly_aligned,
                                                   stmt,
                                                   operand)) {
        *msg << ": memory location (";
        addr->dump(msg->stream());
        *msg << ") with offset (" << offset_c << ") is unaligned\n";
      }
    } else {
      all_valid = false;
      all_invalid = false;

      if (auto msg = this->display_alignment_check(is_correctly_aligned,
                                                   stmt,
                                                   operand)) {
        *msg << ": memory location (";
        addr->dump(msg->stream());
        *msg << ") with offset (" << offset_c << ") may be unaligned\n";
      }
    }

    points_to_info.add(block_info);
  }

  info.put("points_to", points_to_info);

  if (all_valid) {
    if (auto msg = this->display_alignment_check(Result::Ok, stmt, operand)) {
      *msg << ": pointer is aligned\n";
    }
    return {CheckKind::UnalignedPointer, Result::Ok, {}};
  } else if (all_invalid) {
    if (auto msg =
            this->display_alignment_check(Result::Error, stmt, operand)) {
      *msg << ": pointer is unaligned\n";
    }
    return {CheckKind::UnalignedPointer, Result::Error, info};
  } else {
    if (auto msg =
            this->display_alignment_check(Result::Warning, stmt, operand)) {
      *msg << ": pointer may be unaligned\n";
    }
    return {CheckKind::UnalignedPointer, Result::Warning, info};
  }
}

Result PointerAlignmentChecker::check_memory_location_alignment(
    MemoryLocation* memloc,
    const Congruence& offset_c,
    const Congruence& alignment_req_c,
    JsonDict& block_info) {
  // Get the alignment of the memory_location
  bool pto_in_alignment_req = false;
  bool alignment_req_in_pto = true;

  if (auto local_memloc = dyn_cast< LocalMemoryLocation >(memloc)) {
    ar::LocalVariable* lv = local_memloc->local_var();
    Congruence local_alignment_c =
        to_congruence(lv->has_alignment() ? lv->alignment() : 1, 0);

    pto_in_alignment_req =
        add(local_alignment_c, offset_c).leq(alignment_req_c);
    alignment_req_in_pto =
        alignment_req_c.leq(add(local_alignment_c, offset_c));

    block_info.put("congruence", to_json(local_alignment_c));
  } else if (auto global_memloc = dyn_cast< GlobalMemoryLocation >(memloc)) {
    ar::GlobalVariable* gv = global_memloc->global_var();
    Congruence global_alignment_c =
        to_congruence(gv->has_alignment() ? gv->alignment() : 1, 0);

    pto_in_alignment_req =
        add(global_alignment_c, offset_c).leq(alignment_req_c);
    alignment_req_in_pto =
        alignment_req_c.leq(add(global_alignment_c, offset_c));
    block_info.put("congruence", to_json(global_alignment_c));
  } else if (isa< FunctionMemoryLocation >(memloc)) {
    return Result::Error;
  } else if (isa< AggregateMemoryLocation >(memloc)) {
    return Result::Error;
  } else if (isa< AbsoluteZeroMemoryLocation >(memloc)) {
    pto_in_alignment_req = offset_c.leq(alignment_req_c);
    alignment_req_in_pto = alignment_req_c.leq(offset_c);
  } else if (isa< ArgvMemoryLocation >(memloc)) {
    // We suppose that argv has the best possible alignment
    pto_in_alignment_req = offset_c.leq(alignment_req_c);
    alignment_req_in_pto = alignment_req_c.leq(offset_c);
  } else if (isa< LibcErrnoMemoryLocation >(memloc)) {
    // We suppose that errno has the best possible alignment
    pto_in_alignment_req = offset_c.leq(alignment_req_c);
    alignment_req_in_pto = alignment_req_c.leq(offset_c);
  } else if (isa< DynAllocMemoryLocation >(memloc)) {
    // We suppose a dynamic allocation like malloc always returns the best
    // possible alignment for every type, even for vectors
    pto_in_alignment_req = offset_c.leq(alignment_req_c);
    alignment_req_in_pto = alignment_req_c.leq(offset_c);
  } else {
    ikos_unreachable("unexpected memory location");
  }

  // - If the alignment is in the requirement, we are sure the requirement
  // is met
  // - If the alignment contains the requirement (meaning the requirement is
  // in the alignment), this may be a precision issue
  // - Otherwise, there is no common part between the found alignment and
  // the requirement, thus we are sure the requirement is not met
  if (pto_in_alignment_req) {
    return Result::Ok;
  } else if (alignment_req_in_pto) {
    return Result::Warning;
  } else {
    return Result::Error;
  }
}

core::machine_int::Congruence PointerAlignmentChecker::to_congruence(
    uint64_t a, uint64_t b) const {
  return Congruence(ZNumber(a),
                    ZNumber(b),
                    this->_data_layout.pointers.bit_width,
                    Unsigned);
}

llvm::Optional< LogMessage > PointerAlignmentChecker::display_alignment_check(
    Result result, ar::Statement* stmt, ar::Value* operand) const {
  auto msg = this->display_check(result, stmt);
  if (msg) {
    *msg << "check_pointer_alignment(";
    operand->dump(msg->stream());
    *msg << ")";
  }
  return msg;
}

} // end namespace analyzer
} // end namespace ikos
