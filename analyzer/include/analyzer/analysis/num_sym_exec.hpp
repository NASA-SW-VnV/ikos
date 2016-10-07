/******************************************************************************
 *
 * This class executes abstractly ARBOS statements with different
 * levels of precision.
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * It can reason about registers (REG), pointers (PTR) and memory
 * contents (MEM). In the case of registers and memory contents only
 * those storing integers are modelled. Thus, floating-point
 * computations or memory contents accessible through multiple pointer
 * dereferences are safely ignored.
 *
 * Levels of precision:
 *
 * 1) If level of precision is REG then only integer scalar variables
 * are modelled using a numerical abstraction.
 *
 * 2) If the level of precision is PTR then both integer and pointer
 * scalar variables are modelled. If a variable is a pointer we model
 * its address, offset and size. The offset and size are modelled by a
 * numerical abstraction while the address is modelled by a symbolic
 * abstraction. This symbolic abstraction consists of a set of
 * points-to relationships that keeps track of all possible memory
 * objects (i.e., &'s and mallocs) to which the pointer may point to.
 * Thus, a pointer is abstracted by a triple <A,O,S> where A is the
 * set of addresses to which p may point to, O is the offset from the
 * beginning of the block expressed in bytes, and S is the size of the
 * block. The value domain keeps tracks of these triples.
 *
 * 3) If the level of precision is MEM then same level than PTR plus
 * memory contents storing integers are modelled as well. That is, we
 * can keep track of which values are stored in a triple <A,O,S>.
 *
 * Notices:
 *
 * Copyright (c) 2011-2016 United States Government as represented by the
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

#ifndef ANALYZER_NUM_SYM_EXEC_HPP
#define ANALYZER_NUM_SYM_EXEC_HPP

#include <unordered_map>

#include <boost/noncopyable.hpp>

#include <ikos/domains/memory_domains_api.hpp>
#include <ikos/domains/nullity_domains_api.hpp>
#include <ikos/domains/numerical_domains_api.hpp>
#include <ikos/domains/pointer_domains_api.hpp>
#include <ikos/domains/uninitialized_domains_api.hpp>
#include <ikos/domains/exception_domains_api.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/analysis/context.hpp>
#include <analyzer/analysis/sym_exec_api.hpp>
#include <analyzer/ar-wrapper/cfg.hpp>
#include <analyzer/ar-wrapper/literal.hpp>

namespace analyzer {

using namespace arbos;
using namespace ikos;

namespace ar_size {

//! Return the size in bytes.
//  The returned size is extracted from the AR type.
inline uint64_t getAllocSize(const Type_ref& type) {
  if (ar::ar_internal::is_pointer_type(type)) {
    Pointer_Type_ref ptr_type = node_cast< AR_Pointer_Type >(type);
    return ar::getSize(ar::getPointeeType(ptr_type));
  } else {
    return ar::getSize(type);
  }
}

//! Return the allocated size (in bytes) by the global or alloca
//  The returned size is extracted from the AR type.
inline uint64_t getAllocSize(const Operand_ref& o) {
  if (ar::ar_internal::is_cst_operand(o)) {
    Cst_Operand_ref cst_o = node_cast< AR_Cst_Operand >(o);
    Constant_ref cst = ar::getConstant(cst_o);
    if (ar::ar_internal::is_var_addr_constant(cst)) {
      Var_Addr_Constant_ref var_cst = node_cast< Var_Addr_Constant >(cst);
      return getAllocSize(ar::getType(var_cst));
    } else if (ar::ar_internal::is_function_addr_constant(cst)) {
      return (uint64_t)0;
    }
  }
  assert(false && "Cannot determine allocated size");
  exit(EXIT_FAILURE);
}

} // end namespace ar_size

namespace num_sym_exec_impl {

//! Shadow variable to propagate the _whole_ memory allocation size
//  of an alloca, global variable or a malloc-like allocation site.
template < typename VariableName >
inline VariableName get_shadow_size(VariableName v) {
  // All shadow variables must start with prefix "shadow"
  return v.getVarFactory()["shadow." + v.name() + ".size"];
}

} // end namespace num_sym_exec_impl

using namespace num_sym_exec_impl;

template < typename AbsValueDomain, typename VariableName, typename Number >
class num_sym_exec : public sym_exec< AbsValueDomain >,
                     public boost::noncopyable {
  /*

    This class performs the transfer function on each statement.  It
    relies on a value domain (template parameter AbsValueDomain) which
    keeps track of the following information:

    - For an integer scalar x:

      - A range that overapproximates the value of x. The
        representation of the range depends on the underlying
        numerical domain.

      - Whether x might be uninitialized or not.

    - For a pointer scalar p (only if _prec_level >= PTR):

      - The offset from the base address of the object that contains
        p. For this, we add "p" in the underlying numerical domain
        where "p" actually represents the p's offset.

      - The actual size of the allocated memory for p (including
        padding). For this, we add a shadow variable that keeps track
        of the allocated size by the memory object (&'s and
        mallocs) associated with p in the underlying numerical domain.

      - The address of p via a set of memory objects (&'s and mallocs)
        to which p may point to (ie., points-to sets)

      - Whether p might be null or not.

      - In addition to this, if _prec_level == MEM, it also keeps
        track of the content of p (i.e., *p). This handled internally
        by the value analysis (Load and Store).

   */

  typedef typename AbsValueDomain::variable_t variable_t;
  typedef typename AbsValueDomain::linear_expression_t linExpr;
  typedef typename AbsValueDomain::linear_constraint_t linCst;
  typedef interval< Number > interval_t;

  AbsValueDomain _inv;
  VariableFactory& _vfac;
  LiteralFactory& _lfac;
  const TrackedPrecision _prec_level;
  PointerInfo _pointer;
  std::vector< VariableName > _dead_vars;

  typedef std::unordered_map< VariableName, Operand_ref > mem_objects_t;

  //! keep track of memory objects, and the source Operand_ref
  mem_objects_t _mem_objects;

  enum nullity_value_t { MUSTNULL, MUSTNONNULL, MAYNULL };

  void keep_alive(VariableName x) {
    _dead_vars.erase(std::remove(_dead_vars.begin(), _dead_vars.end(), x),
                     _dead_vars.end());
  }

  //! Allocate a new memory object ptr with unknown size. Return true
  //  if the memory object has not been seen before.
  //
  //  We consider as a memory object an alloca (i.e., stack
  //  variables), global variables, malloc-like allocation sites,
  //  function pointers, and destination of inttoptr
  //  instructions. Also, variables whose address might have been
  //  taken are translated to global variables by the front-end.
  bool _make_mem_object(AbsValueDomain& inv,
                        Operand_ref ptr,
                        LiteralFactory& lfac,
                        nullity_value_t NullVal) {
    Literal lPtr = lfac[ptr];
    assert(lPtr.is_var());

    if (_mem_objects.find(lPtr.get_var()) != _mem_objects.end()) {
      return false;
    }

    _mem_objects.insert(
        typename mem_objects_t::value_type(lPtr.get_var(), ptr));

    // update nullity
    if (NullVal == MUSTNULL) {
      null_domain_traits::make_null(inv, lPtr.get_var());
    } else if (NullVal == MUSTNONNULL) {
      null_domain_traits::make_non_null(inv, lPtr.get_var());
    }

    // update pointer info and offset
    // note: use lPtr as a base address
    ptr_domain_traits::assign_object(inv, lPtr.get_var(), lPtr.get_var());

    // make sure that the scope of the memory object survives the rest
    // of the program.
    keep_alive(lPtr.get_var());

    return true;
  }

  //! Allocate a new memory object ptr of size AllocSize (in bytes).
  //  If AllocSize is empty then the size is extracted statically from
  //  the AR type.
  void make_mem_object(AbsValueDomain& inv,
                       Operand_ref ptr,
                       LiteralFactory& lfac,
                       nullity_value_t NullVal,
                       boost::optional< ikos::z_number > AllocSize =
                           boost::optional< ikos::z_number >()) {
    if (_make_mem_object(inv, ptr, lfac, NullVal)) {
      Literal lPtr = lfac[ptr];
      if (AllocSize) {
        inv.assign(get_shadow_size(lPtr.get_var()), *AllocSize);
      } else {
        inv.assign(get_shadow_size(lPtr.get_var()), ar_size::getAllocSize(ptr));
      }
    }
  }

  //! Allocate a new memory object ptr of size AllocSize
  void make_mem_object(AbsValueDomain& inv,
                       Operand_ref ptr,
                       LiteralFactory& lfac,
                       nullity_value_t NullVal,
                       VariableName AllocSize) {
    if (_make_mem_object(inv, ptr, lfac, NullVal)) {
      Literal lPtr = lfac[ptr];
      _inv.assign(get_shadow_size(lPtr.get_var()), linExpr(AllocSize));
    }
  }

  //! Abstract all the memory contents between {base,...,(base+len)-1}
  //  pre: len is in bytes
  void abstract_memory(Operand_ref base, Operand_ref len) {
    Literal Base = _lfac[base];
    assert(Base.is_var());

    Literal Len = _lfac[len];

    // If len is not constant we do not bother and remove anything
    // touched by base
    ikos::z_number n = -1;
    if (Len.is_var()) {
      boost::optional< ikos::z_number > len =
          num_domain_traits::to_interval(_inv, Len.get_var()).singleton();
      if (len) {
        n = *len;
      }
    } else if (Len.is_num()) {
      n = Len.get_num< ikos::z_number >();
    }

    if (n < 0) {
      mem_domain_traits::forget_mem_contents(_inv, Base.get_var());
    } else {
      mem_domain_traits::forget_mem_contents(_inv, Base.get_var(), n);
    }
  }

  //! Model an assignment lhs := rhs
  void Assign(const Literal& lhs, const Operand_ref& rhs, bool IsPointer) {
    Literal rhs_lit = _lfac[rhs];
    VariableName lhs_var = lhs.get_var();

    if (rhs_lit.is_undefined_cst()) {
      _inv -= lhs_var;
      uninit_domain_traits::make_uninitialized(_inv, lhs_var);
      return;
    }

    if (_prec_level >= PTR && IsPointer) {
      if (rhs_lit.is_null_cst()) {
        mem_domain_traits::forget_mem_surface(_inv, lhs_var);
        null_domain_traits::make_null(_inv, lhs_var);
        return;
      }

      assert(rhs_lit.is_var());

      // Create lazily a new memory object only if global, alloca or
      // function pointer.
      if (ar::isGlobalVar(rhs) || ar::isAllocaVar(rhs)) {
        make_mem_object(_inv, rhs, _lfac, MUSTNONNULL);
      } else if (ar::isFunctionPointer(rhs)) {
        make_mem_object(_inv,
                        rhs,
                        _lfac,
                        MUSTNONNULL,
                        /*no size if function pointer*/
                        Literal::make_num< Number >(0));
      }

      // update pointer info and offset
      ptr_domain_traits::assign_pointer(_inv, lhs_var, rhs_lit.get_var());

      // update nullity
      null_domain_traits::assign_nullity(_inv, lhs_var, rhs_lit.get_var());

      return;
    }

    if (!IsPointer) {
      if (rhs_lit.is_var()) {
        // update numerical abstraction
        _inv.assign(lhs_var, linExpr(rhs_lit.get_var()));

        // update uninitialized variables
        uninit_domain_traits::assign_uninitialized(_inv,
                                                   lhs_var,
                                                   rhs_lit.get_var());
      } else if (rhs_lit.is_num()) {
        // update numerical abstraction
        _inv.assign(lhs_var, rhs_lit.get_num< Number >());

        // update uninitialized variables
        uninit_domain_traits::make_initialized(_inv, lhs_var);
      } else {
        _inv -= lhs_var;
      }
    }
  }

  template < typename Op, typename Stmt_ref >
  void binary_operation(Op op, Stmt_ref stmt) {
    /// Execute a binary operation in which all operands are integer
    /// scalars but not pointers.

    VariableName res = _vfac[ar::getName(ar::getResult(stmt))];
    Literal left = _lfac[ar::getLeftOp(stmt)];
    Literal right = _lfac[ar::getRightOp(stmt)];

    if (left.is_undefined_cst() || right.is_undefined_cst()) {
      _inv -= res;
      uninit_domain_traits::make_uninitialized(_inv, res);
      return;
    }

    assert((left.is_var() || left.is_num()) &&
           (right.is_var() || right.is_num()));

    if (left.is_var() && right.is_var()) {
      // update numerical abstraction
      _inv.apply(op, res, left.get_var(), right.get_var());

      // update uninitialized variables
      uninit_domain_traits::assign_uninitialized(_inv,
                                                 res,
                                                 left.get_var(),
                                                 right.get_var());
    } else if (left.is_var() && right.is_num()) {
      // update numerical abstraction
      _inv.apply(op, res, left.get_var(), right.get_num< Number >());

      // update uninitialized variables
      uninit_domain_traits::assign_uninitialized(_inv, res, left.get_var());
    } else if (left.is_num() && right.is_var()) {
      // update numerical abstraction
      _inv.assign(res, left.get_num< Number >());
      _inv.apply(op, res, res, right.get_var());

      // update uninitialized variables
      uninit_domain_traits::assign_uninitialized(_inv, res, right.get_var());
    } else if (left.is_num() && right.is_num()) {
      // update numerical abstraction
      _inv.assign(res, left.get_num< Number >());
      _inv.apply(op, res, res, right.get_num< Number >());

      // update uninitialized variables
      uninit_domain_traits::make_initialized(_inv, res);
    }
  }

  std::vector< linCst > make_linear_cst(CompOp pred,
                                        bool isPositive,
                                        linExpr X,
                                        linExpr Y) {
    linCst cst;
    switch (pred) {
      case eq:
        cst = (isPositive ? X == Y : X != Y);
        break;
      case ne:
        cst = (isPositive ? X != Y : X == Y);
        break;
      case ult:
      case slt:
        cst = (isPositive ? X <= Y - 1 : X >= Y);
        break;
      case ule:
      case sle:
        cst = (isPositive ? X <= Y : X >= Y + 1);
        break;
      default: {
        assert(false && "unreachable");
        exit(EXIT_FAILURE);
      }
    }

    std::vector< linCst > csts;
    csts.push_back(cst);
    if (isPositive && (pred == ult || pred == ule))
      csts.push_back(linCst(X >= 0));

    return csts;
  }

  std::vector< linCst > make_linear_cst(CompOp pred,
                                        bool isPositive,
                                        Operand_ref X,
                                        Operand_ref Y) {
    Literal lX = _lfac[X];
    Literal lY = _lfac[Y];

    assert((lX.is_var() || lX.is_num()) && (lY.is_var() || lY.is_num()));

    if (lX.is_var() && lY.is_var()) {
      return make_linear_cst(pred,
                             isPositive,
                             linExpr(lX.get_var()),
                             linExpr(lY.get_var()));
    }

    if (lX.is_var() && lY.is_num()) {
      return make_linear_cst(pred,
                             isPositive,
                             linExpr(lX.get_var()),
                             linExpr(lY.get_num< Number >()));
    }

    if (lX.is_num() && lY.is_var()) {
      return make_linear_cst(pred,
                             isPositive,
                             linExpr(lX.get_num< Number >()),
                             linExpr(lY.get_var()));
    }

    if (lX.is_num() && lY.is_num()) {
      return make_linear_cst(pred,
                             isPositive,
                             linExpr(lX.get_num< Number >()),
                             linExpr(lY.get_num< Number >()));
    }

    assert(false && "unreachable");
    exit(EXIT_FAILURE);
  }

  //! Refine the addresses of ptr using information from an external
  //! pointer analysis. This is vital to improve the value analysis
  //! because it can only reason about pointers to integers but not
  //! about pointers to pointers.
  bool refineAddr(VariableName ptr) {
    std::pair< PointerInfo::ptr_set_t, z_interval > ptr_info = _pointer[ptr];
    if (ptr_info.first.is_top())
      return false;
    ptr_domain_traits::refine_addrs(_inv, ptr, ptr_info.first);
    return true;
  }

  //! Refine the addresses and offset of ptr using information from an
  //! external pointer analysis.
  bool refineAddrAndOffset(VariableName ptr) {
    std::pair< PointerInfo::ptr_set_t, z_interval > ptr_info = _pointer[ptr];
    if (ptr_info.first.is_top())
      return false;
    ptr_domain_traits::refine_addrs_offset(_inv,
                                           ptr,
                                           ptr_info.first,
                                           ptr_info.second);
    return true;
  }

public:
  void match_down(IvRange formals, OpRange actuals) {
    assert(formals.size() == actuals.size());

    IvRange::iterator F = formals.begin();
    OpRange::iterator A = actuals.begin();
    for (; F != formals.end(); ++F, ++A) {
      Literal Flit = _lfac[*F];
      Assign(Flit, *A, ar::isPointer(*A));
    }
  }

  void match_up(OpRange /*actuals*/,
                boost::optional< Internal_Variable_ref > lhs_cs_ret,
                IvRange /*formals*/,
                boost::optional< Operand_ref > callee_ret) {
    if (lhs_cs_ret && callee_ret) {
      Assign(_lfac[*lhs_cs_ret], *callee_ret, ar::isPointer(*lhs_cs_ret));
    }
  }

  void clear_local_vars() {
    if (_prec_level < PTR)
      return;

    // Cleanup local variables
    for (auto it = _mem_objects.begin(); it != _mem_objects.end(); ++it) {
      if (ar::isAllocaVar(it->second)) {
        // set the size to 0
        _inv.assign(get_shadow_size(it->first), 0);

        // forget the memory content
        mem_domain_traits::forget_mem_contents(_inv, it->first);

        // no need to remove ptr from _mem_objects, because _mem_objects is not
        // propagated to the caller
      }
    }
  }

  num_sym_exec(
      AbsValueDomain inv,
      VariableFactory& vfac,
      LiteralFactory& lfac,
      TrackedPrecision prec_level,
      PointerInfo pointer = PointerInfo(),
      std::vector< VariableName > dead_vars = std::vector< VariableName >())
      : sym_exec< AbsValueDomain >(),
        _inv(inv),
        _vfac(vfac),
        _lfac(lfac),
        _prec_level(prec_level),
        _pointer(pointer),
        _dead_vars(dead_vars) {}

  AbsValueDomain inv() { return _inv; }

  void set_inv(AbsValueDomain inv) { _inv = inv; }

  void exec_start(Basic_Block_ref bb) {
    // check if it is a catch block
    BBRange preds = ar::getPreds(bb);
    if (!preds.empty()) {
      // check if all predecessors end with an invoke statement and if
      // this basic block is the normal/non-exception path.
      bool invoke_normal = true;
      for (auto it = preds.begin(); it != preds.end() && invoke_normal; ++it) {
        StmtRange stmts = ar::getStatements(*it);
        if (!stmts.empty() && ar::ar_internal::is_invoke(stmts.back())) {
          Invoke_ref invoke = node_cast< Invoke >(stmts.back());
          invoke_normal &= (invoke->getNormal() == bb);
        } else {
          invoke_normal = false;
        }
      }

      // mark the beginning of a block that should be executed only if there was
      // no exception
      if (invoke_normal) {
        exc_domain_traits::ignore_exceptions(_inv);
      }
      // exc_domain_traits::enter_catch() is called when we reach the
      // landing pad statement
    }
  }

  void exec_end(Basic_Block_ref bb) {
    // Note that _dead_vars do not contain shadow variables (internal
    // variables added by the analyses) since shadow variables do not
    // appear in the ARBOS CFG.
    for (typename std::vector< VariableName >::iterator it = _dead_vars.begin();
         it != _dead_vars.end();
         ++it) {
      mem_domain_traits::forget_mem_surface(_inv, *it);
    }
  }

  void exec(Arith_Op_ref stmt) {
    if (boost::optional< ikos::operation_t > o =
            translate_op(ar::getArithOp(stmt))) {
      // addition, subtraction, multiplication and (signed) division
      binary_operation< ikos::operation_t, Arith_Op_ref >(*o, stmt);
    } else {
      // sdiv, udiv, srem, urem
      div_operation_t op;
      switch (ar::getArithOp(stmt)) {
        case udiv:
          op = OP_UDIV;
          break;
        case sdiv:
          op = OP_SDIV;
          break;
        case urem:
          op = OP_UREM;
          break;
        case srem:
          op = OP_SREM;
          break;
        default: {
          assert(false && "unreachable");
          exit(EXIT_FAILURE);
        }
      }
      binary_operation< div_operation_t, Arith_Op_ref >(op, stmt);
    }
  }

  void exec(Integer_Comparison_ref stmt) {
    Operand_ref X = ar::getLeftOp(stmt);
    Operand_ref Y = ar::getRightOp(stmt);
    CompOp pred = ar::getPredicate(stmt);
    bool isPos = ar::isPositive(stmt);

    if (ar::ar_internal::is_undefined(X) || ar::ar_internal::is_undefined(Y))
      return;

    if (ar::isPointer(X) || ar::isPointer(Y)) {
      if (_prec_level == REG)
        return;

      // reason only about equalities and disequalities between
      // pointers
      if (pred != eq && pred != ne)
        return;

      CompOp n_pred;

      if (pred == eq)
        n_pred = (isPos) ? eq : ne;
      if (pred == ne)
        n_pred = (isPos) ? ne : eq;

      Literal lX = _lfac[X];
      Literal lY = _lfac[Y];

      // cases like lX.is_null_cst && lY.is_null_cst should be handle
      // by the front-end so we do not bother here.

      if (lX.is_var() && lY.is_null_cst()) { // x == null or x != null
        // reduction with the external pointer analysis
        refineAddr(lX.get_var());
        if (n_pred == eq) {
          null_domain_traits::assert_null(_inv, lX.get_var());
        } else {
          null_domain_traits::assert_non_null(_inv, lX.get_var());
        }
      } else if (lX.is_null_cst() && lY.is_var()) { // y == null or y != null
        // reduction with the external pointer analysis
        refineAddr(lY.get_var());
        if (n_pred == eq) {
          null_domain_traits::assert_null(_inv, lY.get_var());
        } else {
          null_domain_traits::assert_non_null(_inv, lY.get_var());
        }
      } else if (lX.is_var() && lY.is_var()) {
        // reduction with the external pointer analysis
        refineAddr(lX.get_var());
        refineAddr(lY.get_var());
        null_domain_traits::assert_nullity(_inv,
                                           n_pred == eq,
                                           lX.get_var(),
                                           lY.get_var());
        ptr_domain_traits::assert_pointer(_inv,
                                          n_pred == eq,
                                          lX.get_var(),
                                          lY.get_var());
      }
    } else {
      // X and Y are registers.
      std::vector< linCst > csts = make_linear_cst(pred, isPos, X, Y);
      for (typename std::vector< linCst >::iterator I = csts.begin(),
                                                    E = csts.end();
           I != E;
           ++I) {
        _inv += *I;
      }
    }
  }

  void exec(Bitwise_Op_ref stmt) {
    bitwise_operation_t op;
    switch (ar::getBitwiseOp(stmt)) {
      case _and:
        op = OP_AND;
        break;
      case _or:
        op = OP_OR;
        break;
      case _xor:
        op = OP_XOR;
        break;
      case _shl:
        op = OP_SHL;
        break;
      case _lshr:
        op = OP_LSHR;
        break;
      case _ashr:
        op = OP_ASHR;
        break;
      default: {
        assert(false && "unreachable");
        exit(EXIT_FAILURE);
      }
    }
    binary_operation< bitwise_operation_t, Bitwise_Op_ref >(op, stmt);
  }

  void exec(Abstract_Variable_ref stmt) {
    VariableName n = _vfac[ar::getName(stmt)];
    _inv -= n;
  }

  // Mark the beginning of a catch() {...} block
  void exec(Landing_Pad_ref stmt) {
    Internal_Variable_ref exc = ar::getVar(stmt);
    VariableName exc_var = _vfac[ar::getName(exc)];
    exc_domain_traits::enter_catch(_inv, exc_var);
  }

  void exec(Resume_ref stmt) {
    Internal_Variable_ref exc = ar::getVar(stmt);
    VariableName exc_var = _vfac[ar::getName(exc)];
    exc_domain_traits::resume_exception(_inv, exc_var);
  }

  void exec(Unreachable_ref /*stmt*/) {
    // unreachable should propagate exceptions
    exc_domain_traits::set_normal_flow_bottom(_inv);
  }

  void exec(Assignment_ref stmt) {
    Internal_Variable_ref lhs = ar::getLeftOp(stmt);
    Operand_ref rhs = ar::getRightOp(stmt);

    Assign(this->_lfac[lhs], rhs, ar::isPointer(lhs));
  }

  void exec(Conv_Op_ref stmt) {
    Var_Operand_ref dest = node_cast< Var_Operand >(ar::getLeftOp(stmt));
    VariableName dest_var = _vfac[ar::getName(dest)];
    Operand_ref src = ar::getRightOp(stmt);
    Literal lSrc = _lfac[src];

    if (!(lSrc.is_var() || lSrc.is_num())) {
      _inv -= dest_var;
      return;
    }

    conv_operation_t op;
    switch (ar::getConvOp(stmt)) {
      // Pointer casts
      case inttoptr: {
        /*
           from code like int x = 5; int *px =x;
        */
        if (_prec_level < PTR)
          return;
        make_mem_object(_inv,
                        ar::getLeftOp(stmt),
                        _lfac,
                        MUSTNONNULL,
                        /*should be probably better pointer size*/
                        Literal::make_num< Number >(8));
        return;
      }
      case bitcast: {
        if (!ar::isPointer(dest)) {
          _inv -= dest_var;
          return;
        }

        if (_prec_level >= PTR) {
          Assign(_lfac[ar::getLeftOp(stmt)], src, true);
        }
        return;
      }
      case ::arbos::trunc:
        op = OP_TRUNC;
        break;
      case zext:
        op = OP_ZEXT;
        break;
      case sext:
        op = OP_SEXT;
        break;
      // case ptrtoint
      default:
        _inv -= dest_var;
        return;
    }

    /////
    /// Trunc, ZExt, or SExt for scalars.
    /////

    uint64_t Width = ar::getSize(ar::getType(dest));

    if (lSrc.is_var()) {
      // update numerical abstraction
      _inv.apply(op, dest_var, lSrc.get_var(), Width);

      // update uninitialized variables
      uninit_domain_traits::assign_uninitialized(_inv,
                                                 dest_var,
                                                 lSrc.get_var());
    } else if (lSrc.is_num()) {
      // update numerical abstraction
      _inv.apply(op, dest_var, lSrc.get_num< Number >(), Width);

      // update uninitialized variables
      uninit_domain_traits::make_initialized(_inv, dest_var);
    }
  }

  void exec(Pointer_Shift_ref stmt) {
    if (_prec_level < PTR)
      return;

    // This statement performs pointer arithmetic lhs = base + offset
    //
    // 1) no uninitialized variable information is propagated here
    //    since lhs is always a pointer.
    //
    // 2) lhs is null iff base is null regardless offset.
    //
    //    ASSUMPTION: this is only true if we can prove separately
    //    that base+offset cannot be out-of-bounds.
    //
    // 3) points-to[lhs] = points-to[base]
    //
    // 4) In the numerical abstraction, lhs is base plus offset.
    //

    Operand_ref base = ar::getBase(stmt);
    Operand_ref offset = ar::getOffset(stmt);

    Literal lhs = _lfac[ar::getResult(stmt)];
    Literal lBase = _lfac[base];
    Literal lOffset = _lfac[offset];

    if (lBase.is_undefined_cst()) {
      mem_domain_traits::forget_mem_surface(_inv, lhs.get_var());
      uninit_domain_traits::make_uninitialized(_inv, lhs.get_var());
      return;
    }

    if (lBase.is_null_cst()) {
      mem_domain_traits::forget_mem_surface(_inv, lhs.get_var());
      null_domain_traits::make_null(_inv, lhs.get_var());
      return;
    }

    assert(lOffset.is_var() || lOffset.is_num());
    assert(lBase.is_var());

    // Create lazily a memory object
    if (ar::isGlobalVar(base) || ar::isAllocaVar(base)) {
      make_mem_object(_inv, base, _lfac, MUSTNONNULL);
    }

    null_domain_traits::assign_nullity(_inv, lhs.get_var(), lBase.get_var());

    // update the pointer info and offset
    if (lOffset.is_var()) {
      ptr_domain_traits::assign_pointer(_inv,
                                        lhs.get_var(),
                                        lBase.get_var(),
                                        lOffset.get_var());
    } else {
      assert(lOffset.is_num());
      ptr_domain_traits::assign_pointer(_inv,
                                        lhs.get_var(),
                                        lBase.get_var(),
                                        lOffset.get_num< Number >());
    }
  }

private:
  // memcpy  (dest, src, len)
  // memmove (dest, src, len)
  template < typename MemCpyOrMove_ref >
  void execMemCpyOrMove(MemCpyOrMove_ref stmt) {
    if (_prec_level < PTR)
      return;

    // Both src and dest must be already allocated in memory so
    // offsets and sizes for both src and dest are part already of the
    // invariants

    Literal dest = _lfac[ar::getTarget(stmt)];
    Literal src = _lfac[ar::getSource(stmt)];
    Literal size = _lfac[ar::getLen(stmt)];
    assert(dest.is_var() && src.is_var());
    assert(size.is_var() || size.is_num());

    // Create lazily memory objects
    if (ar::isGlobalVar(ar::getTarget(stmt)) ||
        ar::isAllocaVar(ar::getTarget(stmt))) {
      make_mem_object(_inv, ar::getTarget(stmt), _lfac, MUSTNONNULL);
    }
    if (ar::isAllocaVar(ar::getSource(stmt)) ||
        ar::isGlobalVar(ar::getSource(stmt))) {
      make_mem_object(_inv, ar::getSource(stmt), _lfac, MUSTNONNULL);
    }

    // Reduction between value and pointer analysis
    refineAddrAndOffset(dest.get_var());
    refineAddrAndOffset(src.get_var());

    if (_prec_level >= MEM) {
      linExpr size_expr = size.is_var() ? linExpr(size.get_var())
                                        : linExpr(size.get_num< Number >());
      mem_domain_traits::mem_copy(_inv,
                                  dest.get_var(),
                                  src.get_var(),
                                  size_expr);
    }
  }

public:
  void exec(MemCpy_ref stmt) { return execMemCpyOrMove(stmt); }

  void exec(MemMove_ref stmt) { return execMemCpyOrMove(stmt); }

  void exec(MemSet_ref stmt) {
    if (_prec_level < PTR)
      return;

    Literal dest = _lfac[ar::getBase(stmt)];
    Literal value = _lfac[ar::getValue(stmt)];
    Literal size = _lfac[ar::getLen(stmt)];
    assert(dest.is_var());
    assert(value.is_var() || value.is_num());
    assert(size.is_var() || size.is_num());

    // Create lazily memory objects
    if (ar::isGlobalVar(ar::getBase(stmt)) ||
        ar::isAllocaVar(ar::getBase(stmt))) {
      make_mem_object(_inv, ar::getBase(stmt), _lfac, MUSTNONNULL);
    }

    // Reduction between value and pointer analysis
    refineAddrAndOffset(dest.get_var());

    if (_prec_level >= MEM) {
      linExpr value_expr = value.is_var() ? linExpr(value.get_var())
                                          : linExpr(value.get_num< Number >());
      linExpr size_expr = size.is_var() ? linExpr(size.get_var())
                                        : linExpr(size.get_num< Number >());
      mem_domain_traits::mem_set(_inv, dest.get_var(), value_expr, size_expr);
    }
  }

  void exec(Call_ref cs) {
    assert(ar::isDirectCall(cs) && ar::isExternal(cs) &&
           "This method should only execute external calls");

    exec_external_call(ar::getReturnValue(cs),
                       ar::getFunctionName(cs),
                       ar::getArguments(cs));
  }

  //! Analysis of library calls (calls for which code is not
  //! available). TODO: add support for more library calls.
  void exec_external_call(boost::optional< Internal_Variable_ref > lhs,
                          std::string fun_name,
                          OpRange arguments) {
    if (exc_domain_traits::is_normal_flow_bottom(_inv))
      return;

    /*
     * Here library functions that we want to consider specially.
     */
    if ((fun_name == "malloc" || fun_name == "_Znwm" || fun_name == "_Znam" ||
         fun_name == "__cxa_allocate_exception") &&
        arguments.size() == 1) {
      // call to malloc, new, new[] or __cxa_allocate_exception
      analyze_malloc(lhs, arguments[0]);
    } else if (fun_name == "calloc" && arguments.size() == 2) {
      analyze_calloc(lhs, arguments[0], arguments[1]);
    } else if ((fun_name == "free" || fun_name == "_ZdlPv" ||
                fun_name == "_ZdaPv" || fun_name == "__cxa_free_exception") &&
               arguments.size() == 1) {
      // call to free, delete, delete[] or __cxa_free_exception
      analyze_free(arguments[0]);
    } else if (fun_name == "read" && arguments.size() == 3) {
      analyze_read(lhs, arguments[0], arguments[1], arguments[2]);
    } else if (fun_name == "__cxa_throw" && arguments.size() == 3) {
      analyze_cxa_throw(arguments[0], arguments[1], arguments[2]);
    } else if (fun_name == "strlen" && arguments.size() == 1) {
      analyze_strlen(lhs, arguments[0]);
    } else if (fun_name == "strnlen" && arguments.size() == 2) {
      analyze_strnlen(lhs, arguments[0], arguments[1]);
    } else if (fun_name == "strcpy" && arguments.size() == 2) {
      analyze_strcpy(lhs, arguments[0], arguments[1]);
    } else if (fun_name == "strncpy" && arguments.size() == 3) {
      analyze_strncpy(lhs, arguments[0], arguments[1], arguments[2]);
    } else if (fun_name == "strcat" && arguments.size() == 2) {
      analyze_strcat(lhs, arguments[0], arguments[1]);
    } else if (fun_name == "strncat" && arguments.size() == 3) {
      analyze_strncat(lhs, arguments[0], arguments[1], arguments[2]);
    } else {
      // default case: forget all actual parameters of pointer type
      // (very conservative!)
      if (_prec_level >= PTR) {
        for (OpRange::iterator it = arguments.begin(), et = arguments.end();
             it != et;
             ++it) {
          if (ar::isPointer(*it)) {
            Literal p = _lfac[*it];
            if (p.is_var()) {
              mem_domain_traits::forget_mem_contents(_inv, p.get_var());
            }
          }
        }
      }

      // forget the lhs
      if (lhs) {
        Literal ret = _lfac[*lhs];
        assert(ret.is_var());
        _inv -= ret.get_var();

        // ASSUMPTION:
        // The claim about the correctness of the program under analysis
        // can be made only if all calls to unavailable code are assumed
        // to be correct and without side-effects.
        // We will assume that the lhs of an external call site is
        // always initialized (if scalar). However, in case of a
        // pointer, we do not assume that a non-null pointer is
        // returned.
        if (!ar::isPointer(*lhs)) {
          uninit_domain_traits::make_initialized(_inv, ret.get_var());
        }
      }

      // ASSUMPTION:
      // The external call will not raise an exception.
    }
  }

  void exec(Invoke_ref stmt) { exec(ar::getFunctionCall(stmt)); }

  void exec(Return_Value_ref stmt) {
    // mark the result variable as alive
    boost::optional< Operand_ref > ret = ar::getReturnValue(stmt);
    if (ret) {
      Literal lRet = _lfac[*ret];
      if (lRet.is_var()) {
        keep_alive(lRet.get_var());
      }
    }

    // deallocate the memory for local variables
    clear_local_vars();
  }

  void exec(Store_ref stmt) {
    Literal lPtr = _lfac[ar::getPointer(stmt)];
    if (lPtr.is_undefined_cst() || lPtr.is_null_cst()) {
      exc_domain_traits::set_normal_flow_bottom(_inv);
      return;
    }

    if (_prec_level < PTR)
      return;

    // Create lazily memory objects
    if (ar::isAllocaVar(ar::getPointer(stmt)) ||
        ar::isGlobalVar(ar::getPointer(stmt))) {
      make_mem_object(_inv, ar::getPointer(stmt), _lfac, MUSTNONNULL);
    }
    if (ar::isGlobalVar(ar::getValue(stmt)) ||
        ar::isAllocaVar(ar::getValue(stmt))) {
      make_mem_object(_inv, ar::getValue(stmt), _lfac, MUSTNONNULL);
    } else if (ar::isFunctionPointer(ar::getValue(stmt))) {
      make_mem_object(_inv,
                      ar::getValue(stmt),
                      _lfac,
                      MUSTNONNULL,
                      /*no size if function pointer*/
                      Literal::make_num< Number >(0));
    }

    // Reduction between value and pointer analysis
    if (lPtr.is_var())
      refineAddrAndOffset(lPtr.get_var());

    if (_prec_level < MEM)
      return;

    // Perform memory write in the value domain
    assert(lPtr.is_var());
    Literal lVal = _lfac[ar::getValue(stmt)];
    ikos::z_number size =
        ar::getSize(ar::getPointeeType(ar::getType(ar::getPointer(stmt))));

    // TODO: replace analyzer::Literal by ikos::literal
    typedef ikos::literal< ikos::z_number, ikos::dummy_number, varname_t >
        literal_t;

    if (lVal.is_var()) {
      if (ar::isInteger(ar::getType(ar::getValue(stmt)))) {
        mem_domain_traits::mem_write(_inv,
                                     lPtr.get_var(),
                                     literal_t::integer_var(lVal.get_var()),
                                     size);
      } else if (ar::isFloat(ar::getType(ar::getValue(stmt)))) {
        mem_domain_traits::mem_write(_inv,
                                     lPtr.get_var(),
                                     literal_t::floating_point_var(
                                         lVal.get_var()),
                                     size);
      } else if (ar::isPointer(ar::getType(ar::getValue(stmt)))) {
        mem_domain_traits::mem_write(_inv,
                                     lPtr.get_var(),
                                     literal_t::pointer_var(lVal.get_var()),
                                     size);
      } else if (ar::isFunctionPointer(ar::getValue(stmt))) {
        // special case for constant function pointer
        mem_domain_traits::mem_write(_inv,
                                     lPtr.get_var(),
                                     literal_t::pointer_var(lVal.get_var()),
                                     size);
      } else {
        assert(false && "unreachable");
      }
    } else if (lVal.is_num()) {
      mem_domain_traits::mem_write(_inv,
                                   lPtr.get_var(),
                                   literal_t::integer(lVal.get_num< Number >()),
                                   size);
    } else if (lVal.is_float_cst()) {
      mem_domain_traits::mem_write(_inv,
                                   lPtr.get_var(),
                                   literal_t::floating_point(dummy_number()),
                                   size);
    } else if (lVal.is_undefined_cst()) {
      mem_domain_traits::mem_write(_inv,
                                   lPtr.get_var(),
                                   literal_t::undefined(),
                                   size);
    } else if (lVal.is_null_cst()) {
      mem_domain_traits::mem_write(_inv,
                                   lPtr.get_var(),
                                   literal_t::null(),
                                   size);
    } else {
      assert(false && "unreachable");
    }
  }

  void exec(Load_ref stmt) {
    Literal lPtr = _lfac[ar::getPointer(stmt)];
    if (lPtr.is_undefined_cst() || lPtr.is_null_cst()) {
      exc_domain_traits::set_normal_flow_bottom(_inv);
      return;
    }

    if (_prec_level < PTR)
      return;

    Internal_Variable_ref lhs = ar::getResult(stmt);
    VariableName lhs_var = _vfac[ar::getName(lhs)];

    // Create lazily a memory object
    if (ar::isGlobalVar(ar::getPointer(stmt)) ||
        ar::isAllocaVar(ar::getPointer(stmt))) {
      make_mem_object(_inv, ar::getPointer(stmt), _lfac, MUSTNONNULL);

      // Take the address of a global variable x:
      // int x = 0; int * p = &x;
      null_domain_traits::make_non_null(_inv, lhs_var);
    }

    // Reduction between value and pointer analysis
    if (lPtr.is_var())
      refineAddrAndOffset(lPtr.get_var());

    if (_prec_level < MEM)
      return;

    // Perform memory read in the value domain
    assert(lPtr.is_var());
    ikos::z_number size =
        ar::getSize(ar::getPointeeType(ar::getType(ar::getPointer(stmt))));

    // TODO: replace analyzer::Literal by ikos::literal
    typedef ikos::literal< ikos::z_number, ikos::dummy_number, varname_t >
        literal_t;

    if (ar::isInteger(ar::getType(lhs))) {
      mem_domain_traits::mem_read(_inv,
                                  literal_t::integer_var(lhs_var),
                                  lPtr.get_var(),
                                  size);
    } else if (ar::isFloat(ar::getType(lhs))) {
      mem_domain_traits::mem_read(_inv,
                                  literal_t::floating_point_var(lhs_var),
                                  lPtr.get_var(),
                                  size);
    } else if (ar::isPointer(ar::getType(lhs))) {
      mem_domain_traits::mem_read(_inv,
                                  literal_t::pointer_var(lhs_var),
                                  lPtr.get_var(),
                                  size);
    } else {
      assert(false && "unreachable");
    }

    // Reduction between value and pointer analysis
    if (ar::isPointer(lhs)) {
      refineAddrAndOffset(lhs_var);
    }
  }

  //! Abstract the memory contents but not the surface
  void exec(Abstract_Memory_ref stmt) {
    if (_prec_level < MEM)
      return;

    // This AR statement can only be generated to indicate that a
    // global variable does not have an initializer.
    abstract_memory(ar::getPointer(stmt), ar::getLen(stmt));
  }

  // Not implemented
  void exec(FP_Comparison_ref /*stmt*/) {} // no floating point reasoning
  void exec(FP_Op_ref /*stmt*/) {}         // no floating point reasoning

private:
  /////
  // Analysis of external (library) calls
  // Note: This code should go to a different file if it keeps growing.
  //////

  /*
    #include <stdlib.h>
    void* malloc(size_t size)

    This function returns a pointer to a newly allocated block size
    bytes long, or a null pointer if the block could not be allocated.
   */
  void analyze_malloc(boost::optional< Internal_Variable_ref > lhs,
                      Operand_ref size) {
    if (_prec_level < PTR)
      return;

    if (!lhs)
      return;

    if (ar::isPointer(*lhs) &&
        ar::isInteger(ar::getPointeeType(ar::getType(*lhs)))) {
      Literal Size = _lfac[size];

      Operand_ref ptr;
      arbos::convert(*lhs, ptr);

      // Create a new memory object
      if (Size.is_var()) {
        make_mem_object(_inv,
                        ptr,
                        _lfac,
                        /* malloc can return NULL if no more dynamic
                         * memory is available */
                        MAYNULL,
                        Size.get_var());
      } else if (Size.is_num()) {
        make_mem_object(_inv,
                        ptr,
                        _lfac,
                        /* malloc can return NULL if no more dynamic
                         * memory is available */
                        MAYNULL,
                        Size.get_num< ikos::z_number >());
      }
    }
  }

  /*
    #include <stdlib.h>
    void* calloc(size_t count, size_t size)

    The calloc() function contiguously allocates enough space for count objects
    that are size bytes of memory each and returns a pointer to the allocated
    memory. The allocated memory is filled with bytes of value zero.
   */
  void analyze_calloc(boost::optional< Internal_Variable_ref > lhs,
                      Operand_ref count,
                      Operand_ref size) {
    if (_prec_level < PTR)
      return;

    if (!lhs)
      return;

    if (ar::isPointer(*lhs) &&
        ar::isInteger(ar::getPointeeType(ar::getType(*lhs)))) {
      Literal Count = _lfac[count];
      Literal Size = _lfac[size];

      Operand_ref ptr;
      arbos::convert(*lhs, ptr);

      // Create a new memory object
      if (!_make_mem_object(_inv, ptr, _lfac, MAYNULL))
        return;

      Literal Ptr = _lfac[ptr];

      // TODO: Check for integer overflow
      if (Count.is_var() && Size.is_var()) {
        _inv.apply(ikos::OP_MULTIPLICATION,
                   get_shadow_size(Ptr.get_var()),
                   Count.get_var(),
                   Size.get_var());
      } else if (Count.is_var() && Size.is_num()) {
        _inv.apply(ikos::OP_MULTIPLICATION,
                   get_shadow_size(Ptr.get_var()),
                   Count.get_var(),
                   Size.get_num< ikos::z_number >());
      } else if (Count.is_num() && Size.is_var()) {
        _inv.apply(ikos::OP_MULTIPLICATION,
                   get_shadow_size(Ptr.get_var()),
                   Size.get_var(),
                   Count.get_num< ikos::z_number >());
      } else if (Count.is_num() && Size.is_num()) {
        _inv.assign(get_shadow_size(Ptr.get_var()),
                    Count.get_num< ikos::z_number >() *
                        Size.get_num< ikos::z_number >());
      }
    }
  }

  /*
    #include <stdlib.h>
    void free(void* ptr)

    This function deallocates the memory allocated via a previous call to
    malloc().
   */
  void analyze_free(Operand_ref op) {
    Literal ptr = _lfac[op];

    if (!ptr.is_var())
      return;

    if (_prec_level < PTR)
      return;

    if (null_domain_traits::is_null(_inv, ptr.get_var()))
      return;

    // set the size to 0
    ikos::discrete_domain< VariableName > points_to =
        ptr_domain_traits::addrs_set(_inv, ptr.get_var());

    if (points_to.is_top())
      return;

    for (auto it = points_to.begin(); it != points_to.end(); ++it) {
      if (points_to.size() == 1) {
        _inv.assign(get_shadow_size(*it), 0);
      } else {
        _inv -= get_shadow_size(*it);
      }
    }

    if (_prec_level < MEM)
      return;

    // forget memory contents
    mem_domain_traits::forget_mem_contents(_inv, ptr.get_var());
  }

  /*
    #include <fcntl.h>
    int read(int handle, void* buffer, int nbyte);

    The read() function attempts to read nbytes from the file
    associated with handle, and places the characters read into
    buffer. If the file is opened using O_TEXT, it removes carriage
    returns and detects the end of the file.

    The function returns the number of bytes read. On end-of-file, 0
    is returned, on error it returns -1, setting errno to indicate the
    type of error that occurred.
  */
  void analyze_read(boost::optional< Internal_Variable_ref > lhs,
                    Operand_ref /*handle*/,
                    Operand_ref buf,
                    Operand_ref len) {
    if (lhs) {
      Literal ret = _lfac[*lhs];
      assert(ret.is_var());
      _inv -= ret.get_var();
      uninit_domain_traits::make_initialized(_inv, ret.get_var());
    }

    if (_prec_level < MEM)
      return;

    abstract_memory(buf, len);
  }

  /*
    __cxa_throw(void* exception, std::type_info* tinfo, void (*dest)(void*))

    After constructing the exception object with the throw argument value,
    the generated code calls the __cxa_throw runtime library routine. This
    routine never returns.
  */
  void analyze_cxa_throw(Operand_ref exception,
                         Operand_ref tinfo,
                         Operand_ref dest) {
    Literal lException = _lfac[exception];
    Literal lTinfo = _lfac[tinfo];
    Literal lDest = _lfac[dest];
    assert(lException.is_var());
    assert(lTinfo.is_var());
    assert(lDest.is_var() || lDest.is_null_cst());

    clear_local_vars();

    boost::optional< VariableName > d;
    if (lDest.is_var()) {
      d = lDest.get_var();
    }
    exc_domain_traits::throw_exception(_inv,
                                       lException.get_var(),
                                       lTinfo.get_var(),
                                       d);
  }

  /*
    #include <string.h>
    size_t strlen(const char* s);

    The strlen() function computes the length of the string s.

    The strlen() function returns the number of characters that precede
    the terminating NUL character.
  */
  void analyze_strlen(boost::optional< Internal_Variable_ref > lhs,
                      Operand_ref str) {
    Literal lStr = _lfac[str];

    if (lStr.is_undefined_cst() || lStr.is_null_cst()) {
      exc_domain_traits::set_normal_flow_bottom(_inv);
      return;
    }

    assert(lStr.is_var());

    if (_prec_level < PTR)
      return;

    if (null_domain_traits::is_null(_inv, lStr.get_var())) {
      exc_domain_traits::set_normal_flow_bottom(_inv);
      return;
    }

    // Create lazily memory objects
    if (ar::isGlobalVar(str) || ar::isAllocaVar(str)) {
      make_mem_object(_inv, str, _lfac, MUSTNONNULL);
    }

    // Reduction between value and pointer analysis
    refineAddrAndOffset(lStr.get_var());

    if (!lhs)
      return;

    Literal ret = _lfac[*lhs];

    // ret is in [0, size - 1]
    _inv -= ret.get_var();
    _inv += (variable_t(ret.get_var()) >= 0);
    uninit_domain_traits::make_initialized(_inv, ret.get_var());

    ikos::discrete_domain< VariableName > points_to =
        ptr_domain_traits::addrs_set(_inv, lStr.get_var());

    if (points_to.is_top())
      return;

    AbsValueDomain inv = AbsValueDomain::bottom();
    for (auto it = points_to.begin(); it != points_to.end(); ++it) {
      AbsValueDomain tmp = _inv;
      tmp +=
          (variable_t(ret.get_var()) <= variable_t(get_shadow_size(*it)) - 1);
      inv = inv | tmp;
    }
    _inv = inv;
  }

  /*
    #include <string.h>
    size_t strnlen(const char* s);

    The strnlen() function attempts to compute the length of s, but
    never scans beyond the first maxlen bytes of s.

    The strnlen() function returns either the same result as strlen()
    or maxlen, whichever is smaller.
  */
  void analyze_strnlen(boost::optional< Internal_Variable_ref > lhs,
                       Operand_ref str,
                       Operand_ref maxlen) {
    analyze_strlen(lhs, str);

    if (exc_domain_traits::is_normal_flow_bottom(_inv))
      return;

    if (!lhs)
      return;

    // ret <= maxlen
    Literal ret = _lfac[*lhs];
    Literal lMaxLen = _lfac[maxlen];
    if (lMaxLen.is_var()) {
      _inv += variable_t(ret.get_var()) <= variable_t(lMaxLen.get_var());
    } else if (lMaxLen.is_num()) {
      _inv += variable_t(ret.get_var()) <= lMaxLen.get_num< ikos::z_number >();
    }
  }

  /*
    #include <string.h>
    char* strcpy(char* dst, const char* src);

    The strcpy() function copies the string src to dst (including
    the terminating `\0' character).

    The strcpy() function returns dst.
  */
  void analyze_strcpy(boost::optional< Internal_Variable_ref > lhs,
                      Operand_ref dest,
                      Operand_ref src) {
    Literal lDest = _lfac[dest];
    Literal lSrc = _lfac[src];

    if (lDest.is_undefined_cst() || lDest.is_null_cst() ||
        lSrc.is_undefined_cst() || lSrc.is_null_cst()) {
      exc_domain_traits::set_normal_flow_bottom(_inv);
      return;
    }

    assert(lDest.is_var() && lSrc.is_var());

    if (_prec_level < PTR)
      return;

    if (null_domain_traits::is_null(_inv, lDest.get_var()) ||
        null_domain_traits::is_null(_inv, lSrc.get_var())) {
      exc_domain_traits::set_normal_flow_bottom(_inv);
      return;
    }

    // Create lazily memory objects
    if (ar::isGlobalVar(dest) || ar::isAllocaVar(dest)) {
      make_mem_object(_inv, dest, _lfac, MUSTNONNULL);
    }
    if (ar::isAllocaVar(src) || ar::isGlobalVar(src)) {
      make_mem_object(_inv, src, _lfac, MUSTNONNULL);
    }

    // Reduction between value and pointer analysis
    refineAddrAndOffset(lDest.get_var());
    refineAddrAndOffset(lSrc.get_var());

    if (_prec_level >= MEM) {
      // Do not keep track of the content
      mem_domain_traits::forget_mem_contents(_inv, lDest.get_var());
    }

    if (lhs) {
      Assign(_lfac[*lhs], dest, true);
    }
  }

  /*
    #include <string.h>
    char* strncpy(char* dst, const char* src, size_t n);

    The strncpy() function copies at most n characters from src into dst.
    If src is less than n characters long, the remainder of dst is filled
    with `\0' characters.  Otherwise, dst is not terminated.

    The strncpy() function returns dst.
  */
  void analyze_strncpy(boost::optional< Internal_Variable_ref > lhs,
                       Operand_ref dest,
                       Operand_ref src,
                       Operand_ref /*size*/) {
    analyze_strcpy(lhs, dest, src);
  }

  /*
    #include <string.h>
    char* strcat(char* s1, const char* s2);

    The strcat() function appends a copy of the null-terminated string s2
    to the end of the null-terminated string s1, then add a terminating `\0'.
    The string s1 must have sufficient space to hold the result.

    The strcat() function returns the pointer s1.
  */
  void analyze_strcat(boost::optional< Internal_Variable_ref > lhs,
                      Operand_ref s1,
                      Operand_ref s2) {
    Literal lS1 = _lfac[s1];
    Literal lS2 = _lfac[s2];

    if (lS1.is_undefined_cst() || lS1.is_null_cst() || lS2.is_undefined_cst() ||
        lS2.is_null_cst()) {
      exc_domain_traits::set_normal_flow_bottom(_inv);
      return;
    }

    assert(lS1.is_var() && lS2.is_var());

    if (_prec_level < PTR)
      return;

    if (null_domain_traits::is_null(_inv, lS1.get_var()) ||
        null_domain_traits::is_null(_inv, lS2.get_var())) {
      exc_domain_traits::set_normal_flow_bottom(_inv);
      return;
    }

    // Create lazily memory objects
    if (ar::isGlobalVar(s1) || ar::isAllocaVar(s1)) {
      make_mem_object(_inv, s1, _lfac, MUSTNONNULL);
    }
    if (ar::isAllocaVar(s2) || ar::isGlobalVar(s2)) {
      make_mem_object(_inv, s2, _lfac, MUSTNONNULL);
    }

    // Reduction between value and pointer analysis
    refineAddrAndOffset(lS1.get_var());
    refineAddrAndOffset(lS2.get_var());

    if (_prec_level >= MEM) {
      // Do not keep track of the content
      mem_domain_traits::forget_mem_contents(_inv, lS1.get_var());
    }

    if (lhs) {
      Assign(_lfac[*lhs], s1, true);
    }
  }

  /*
    #include <string.h>
    char* strncat(char* s1, const char* s2, size_t n);

    The strncat() function appends a copy of the null-terminated string s2
    to the end of the null-terminated string s1, then add a terminating `\0'.
    The string s1 must have sufficient space to hold the result.

    The strncat() function appends not more than n characters from s2, and
    then adds a terminating `\0'.

    The strncat() function returns the pointer s1.
  */
  void analyze_strncat(boost::optional< Internal_Variable_ref > lhs,
                       Operand_ref s1,
                       Operand_ref s2,
                       Operand_ref n) {
    analyze_strcat(lhs, s1, s2);
  }

}; // end class num_sym_exec

} // end namespace analyzer

#endif // ANALYZER_NUM_SYM_EXEC_HPP
