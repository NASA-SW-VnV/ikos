/******************************************************************************
 *
 * This class executes abstractly ARBOS statements with different
 * levels of precision.
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *               Clement Decoodt
 *
 * Contact: ikos@lists.nasa.gov
 *
 * It can reason about registers (REG), pointers (PTR) and memory
 * contents (MEM).
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
 * memory contents. That is, we can keep track of which values are stored
 * in a triple <A,O,S>.
 *
 * Notices:
 *
 * Copyright (c) 2011-2017 United States Government as represented by the
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

#include <sstream>
#include <unordered_map>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/noncopyable.hpp>

#include <ikos/domains/counter_domains_api.hpp>
#include <ikos/domains/exception_domains_api.hpp>
#include <ikos/domains/memory_domains_api.hpp>
#include <ikos/domains/nullity_domains_api.hpp>
#include <ikos/domains/numerical_domains_api.hpp>
#include <ikos/domains/pointer_domains_api.hpp>
#include <ikos/domains/uninitialized_domains_api.hpp>

#include <analyzer/cast.hpp>
#include <analyzer/analysis/common.hpp>
#include <analyzer/analysis/memory_location.hpp>
#include <analyzer/analysis/variable_name.hpp>
#include <analyzer/analysis/context.hpp>
#include <analyzer/analysis/sym_exec_api.hpp>
#include <analyzer/ar-wrapper/aggregate_literal.hpp>
#include <analyzer/ar-wrapper/cfg.hpp>
#include <analyzer/ar-wrapper/literal.hpp>

namespace analyzer {

namespace sym_exec_traits {

namespace detail {

template < typename VariableFactory, typename memory_location_t >
struct alloc_size_var_impl {
  typename VariableFactory::variable_name_t operator()(VariableFactory& vfac,
                                                       memory_location_t addr);
};

// implementation for analyzer::varname_t, analyzer::memloc_t
template <>
struct alloc_size_var_impl< VariableFactory, memloc_t > {
  VariableFactory::variable_name_t operator()(VariableFactory& vfac,
                                              memloc_t addr) {
    return vfac.get_alloc_size(addr);
  }
};

}; // end namespace detail

// return a shadow variable that represents the whole memory allocated size
// of a alloca, global variable or a malloc-like allocation site.
template < typename VariableFactory, typename memory_location_t >
inline typename VariableFactory::variable_name_t alloc_size_var(
    VariableFactory& vfac, memory_location_t addr) {
  return detail::alloc_size_var_impl< VariableFactory,
                                      memory_location_t >()(vfac, addr);
}

} // end namespace sym_exec_traits

using namespace sym_exec_traits;

template < typename AbsValueDomain, typename Integer, typename Float >
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
        p. For this, we rely on the pointer_domain_impl that uses the variable
        offset_var(p) = "p.offset" in the underlying numerical domain to
        represent p's offset.

      - The actual size of the allocated memory for p (including
        padding). For this, we add a shadow variable alloc_size_var(obj) =
        "shadow.obj.size" that keeps track of the allocated size by the memory
        object (&'s and mallocs) associated with p in the underlying numerical
        domain.

      - The address of p via a set of memory objects (&'s and mallocs)
        to which p may point to (ie., points-to sets).

      - Whether p might be null or not.

      - Whether p might be uninitialized or not.

      - In addition to this, if _prec_level == MEM, it also keeps
        track of the content of p (i.e., *p). This is handled internally
        by the value analysis (Load and Store).

   */

public:
  typedef Integer integer_t;
  typedef Float floating_point_t;
  typedef analyzer::varname_t variable_name_t;
  typedef analyzer::memloc_t memory_location_t;
  typedef ikos::literal< Integer, Float, variable_name_t > scalar_lit_t;
  typedef aggregate_literal< Integer, Float, variable_name_t > aggregate_lit_t;
  typedef ikos::patricia_tree_set< variable_name_t > varname_set_t;
  typedef ikos::variable< Integer, variable_name_t > int_variable_t;
  typedef ikos::interval< Integer > int_interval_t;
  typedef ikos::linear_expression< Integer, variable_name_t >
      int_linear_expression_t;
  typedef ikos::linear_constraint< Integer, variable_name_t >
      int_linear_constraint_t;
  typedef ikos::linear_constraint_system< Integer, variable_name_t >
      int_linear_constraint_system_t;

private:
  typedef std::unordered_map< variable_name_t, Operand_ref > mem_objects_t;
  enum class nullity_value { null, non_null, may_null };
  enum class uninit_value { initialized, uninitialized, may_uninitialized };

private:
  AbsValueDomain _inv;
  VariableFactory& _vfac;
  LiteralFactory& _lfac;
  memory_factory& _mfac;
  std::string _arch;
  const TrackedPrecision _prec_level;
  PointerInfo _pointer;
  varname_set_t _dead_vars;
  varname_set_t _loop_counters;
  std::shared_ptr< sym_exec_warning_callback > _warning_callback;

public:
  num_sym_exec(
      AbsValueDomain inv,
      VariableFactory& vfac,
      memory_factory& mfac,
      LiteralFactory& lfac,
      const std::string& arch,
      TrackedPrecision prec_level,
      PointerInfo pointer = PointerInfo(),
      varname_set_t dead_vars = varname_set_t(),
      varname_set_t loop_counters = varname_set_t(),
      std::shared_ptr< sym_exec_warning_callback > warning_callback = nullptr)
      : sym_exec< AbsValueDomain >(),
        _inv(inv),
        _vfac(vfac),
        _lfac(lfac),
        _mfac(mfac),
        _arch(arch),
        _prec_level(prec_level),
        _pointer(pointer),
        _dead_vars(dead_vars),
        _loop_counters(loop_counters),
        _warning_callback(warning_callback) {}

  AbsValueDomain inv() const { return _inv; }

  void set_inv(AbsValueDomain inv) { _inv = inv; }

private:
  void keep_alive(variable_name_t x) { _dead_vars -= x; }

public:
  /*
   * Helpers for memory instructions
   */

  /*
   * Allocate a new memory object ptr with unknown size.
   *
   * We consider as a memory object an alloca (i.e., stack
   * variables), global variables, malloc-like allocation sites,
   * function pointers, and destination of inttoptr
   * instructions. Also, variables whose address might have been
   * taken are translated to global variables by the front-end.
   */
  void allocate_memory(variable_name_t ptr,
                       memory_location_t memloc,
                       nullity_value null_val,
                       uninit_value uninit_val) {
    if (_prec_level < PTR)
      return;

    // update pointer info and offset
    // note: use ptr.var() as a base address name
    ikos::ptr_domain_traits::assign_object(_inv, ptr, memloc);

    // update nullity
    if (null_val == nullity_value::null) {
      ikos::null_domain_traits::make_null(_inv, ptr);
    } else if (null_val == nullity_value::non_null) {
      ikos::null_domain_traits::make_non_null(_inv, ptr);
    } else {
      ikos::null_domain_traits::forget_nullity(_inv, ptr);
    }

    // update uninitialized variables
    if (uninit_val == uninit_value::initialized) {
      ikos::uninit_domain_traits::make_initialized(_inv, ptr);
    } else if (uninit_val == uninit_value::uninitialized) {
      ikos::uninit_domain_traits::make_uninitialized(_inv, ptr);
    } else {
      ikos::uninit_domain_traits::forget_uninitialized(_inv, ptr);
    }
  }

  // Allocate a new memory object ptr of size alloc_size (in bytes).
  void allocate_memory(variable_name_t ptr,
                       memory_location_t memloc,
                       nullity_value null_val,
                       uninit_value uninit_val,
                       integer_t alloc_size) {
    if (_prec_level < PTR)
      return;

    // update pointer info, offset, nullity, uninitialized variables
    allocate_memory(ptr, memloc, null_val, uninit_val);

    // update allocated size var
    variable_name_t v = alloc_size_var(_vfac, memloc);
    _inv.assign(v, alloc_size);
  }

  // Allocate a new memory object ptr of size alloc_size (in bytes).
  void allocate_memory(variable_name_t ptr,
                       memory_location_t memloc,
                       nullity_value null_val,
                       uninit_value uninit_val,
                       variable_name_t alloc_size) {
    if (_prec_level < PTR)
      return;

    // update pointer info, offset, nullity, uninitialized variables
    allocate_memory(ptr, memloc, null_val, uninit_val);

    // update allocated size var
    _inv.assign(alloc_size_var(_vfac, memloc),
                int_linear_expression_t(alloc_size));
  }

private:
  // Create memory objects for global variables and constant function pointers
  void init_global_var_ptr(Operand_ref op) {
    if (_prec_level < PTR)
      return;

    if (ar::isGlobalVar(op)) {
      auto gv = ar::getGlobalVariable(op);
      ikos_assert(gv);
      allocate_memory(_vfac.get_global(*gv),
                      _mfac.get_global(*gv),
                      nullity_value::non_null,
                      uninit_value::initialized,
                      ar::getAllocatedSize(op));
    } else if (ar::isFunctionPointer(op)) {
      // size of 1 byte, because function pointers can be compared, thus each
      // function should have a different address, so functions should be at
      // least one byte.
      Cst_Operand_ref cst_o = node_cast< Cst_Operand >(op);
      Constant_ref cst = cst_o->getConstant();
      auto fv = node_cast< Function_Addr_Constant >(cst);

      allocate_memory(_vfac.get_function_addr(fv),
                      _mfac.get_function(fv),
                      nullity_value::non_null,
                      uninit_value::initialized,
                      integer_t(1));
    }
  }

private:
  /*
   * Prepare a memory access (read/write) on the given pointer
   *
   * Return True if the memory access can be performed,
   * i.e the pointer is non-null, well defined, and _prec_level == MEM.
   */
  bool prepare_mem_access(const scalar_lit_t& ptr) {
    if (ptr.is_null() || ptr.is_undefined()) {
      ikos::exc_domain_traits::set_normal_flow_bottom(_inv);
      return false;
    } else if (!ptr.is_pointer_var()) {
      throw analyzer_error(
          "num_sym_exec::prepare_mem_access(): unexpected parameter");
    }

    if (_prec_level < PTR)
      return false;

    // reduction between value and pointer analysis
    refine_addrs_offset(ptr.var());

    if (ikos::exc_domain_traits::is_normal_flow_bottom(_inv))
      return false;

    if (ikos::null_domain_traits::is_null(_inv, ptr.var()) ||
        ikos::uninit_domain_traits::is_uninitialized(_inv, ptr.var())) {
      // null/undefined dereference
      ikos::exc_domain_traits::set_normal_flow_bottom(_inv);
      return false;
    }

    return _prec_level == MEM; // ready for read/write
  }

public:
  // Model a pointer arithmetic lhs := base + offset
  void pointer_shift(const scalar_lit_t& lhs,
                     const scalar_lit_t& base,
                     const scalar_lit_t& offset) {
    if (_prec_level < PTR)
      return;

    if (!lhs.is_pointer_var()) {
      throw analyzer_error(
          "num_sym_exec::pointer_shift(): left hand side is not a pointer "
          "variable");
    }

    /*
     * 1) lhs is null iff base is null regardless offset.
     *
     *    ASSUMPTION: this is only true if we can prove separately
     *    that base+offset cannot be out-of-bounds.
     *
     * 2) points-to[lhs] = points-to[base]
     *
     * 3) In the numerical abstraction, lhs is base plus offset.
     */

    if (base.is_undefined() || offset.is_undefined()) {
      ikos::mem_domain_traits::forget_mem_surface(_inv, lhs.var());
      ikos::uninit_domain_traits::make_uninitialized(_inv, lhs.var());
    } else if (base.is_null()) {
      ikos::mem_domain_traits::forget_mem_surface(_inv, lhs.var());
      ikos::null_domain_traits::make_null(_inv, lhs.var());
      ikos::uninit_domain_traits::make_initialized(_inv, lhs.var());
    } else if (base.is_pointer_var()) {
      // update nullity
      ikos::null_domain_traits::assign_nullity(_inv, lhs.var(), base.var());

      // update the pointer info and offset, and uninitialized variables
      if (offset.is_integer_var()) {
        ikos::ptr_domain_traits::assign_pointer(_inv,
                                                lhs.var(),
                                                base.var(),
                                                offset.var());
        ikos::uninit_domain_traits::assign_uninitialized(_inv,
                                                         lhs.var(),
                                                         base.var(),
                                                         offset.var());
      } else if (offset.is_integer()) {
        ikos::ptr_domain_traits::assign_pointer(_inv,
                                                lhs.var(),
                                                base.var(),
                                                offset.integer());
        ikos::uninit_domain_traits::assign_uninitialized(_inv,
                                                         lhs.var(),
                                                         base.var());
      } else {
        throw analyzer_error(
            "num_sym_exec::pointer_shift(): unexpected offset operand");
      }
    } else {
      throw analyzer_error(
          "num_sym_exec::pointer_shift(): unexpected base operand");
    }
  }

  // Abstract all the memory contents between *ptr and *(ptr + len - 1). len is
  // in bytes.
  void abstract_memory(const scalar_lit_t& ptr, const scalar_lit_t& len) {
    if (_prec_level < MEM)
      return;

    if (!ptr.is_pointer_var()) {
      throw analyzer_error(
          "num_sym_exec::abstract_memory(): unexpected pointer operand");
    }

    // if len is not constant, we remove anything accessible through ptr
    integer_t n(-1);
    if (len.is_integer_var()) {
      int_interval_t len_itv =
          ikos::num_domain_traits::to_interval(_inv, len.var());
      if (len_itv.ub().is_finite()) {
        n = *len_itv.ub().number();
      }
    } else if (len.is_integer()) {
      n = len.integer();
    } else {
      throw analyzer_error(
          "num_sym_exec::abstract_memory(): unexpected length operand");
    }

    // reduction between value and pointer analysis
    refine_addrs_offset(ptr.var());

    if (n < 0) {
      ikos::mem_domain_traits::forget_mem_contents(_inv, ptr.var());
    } else {
      ikos::mem_domain_traits::forget_mem_contents(_inv, ptr.var(), n);
    }
  }

private:
  // Refine the addresses of ptr using information from an external
  // pointer analysis.
  void refine_addrs(variable_name_t ptr) {
    if (_prec_level < PTR)
      return;

    std::pair< PointerInfo::ptr_set_t, int_interval_t > ptr_info =
        _pointer[ptr];

    if (ptr_info.first.is_top())
      return;

    ikos::ptr_domain_traits::refine_addrs(_inv, ptr, ptr_info.first);
  }

  // Refine the addresses and offset of ptr using information from an
  // external pointer analysis.
  void refine_addrs_offset(variable_name_t ptr) {
    if (_prec_level < PTR)
      return;

    std::pair< PointerInfo::ptr_set_t, int_interval_t > ptr_info =
        _pointer[ptr];

    if (ptr_info.first.is_top())
      return;

    ikos::ptr_domain_traits::refine_addrs_offset(_inv,
                                                 ptr,
                                                 ptr_info.first,
                                                 ptr_info.second);
  }

private:
  /*
   * Helpers for assignments
   */

  class integer_assign : public scalar_lit_t::template visitor<> {
  private:
    variable_name_t _lhs;
    AbsValueDomain& _inv;

  public:
    integer_assign(variable_name_t lhs, AbsValueDomain& inv)
        : _lhs(lhs), _inv(inv) {}

    void integer(integer_t rhs) {
      // update numerical abstraction
      _inv.assign(_lhs, int_linear_expression_t(rhs));

      // update uninitialized variables
      ikos::uninit_domain_traits::make_initialized(_inv, _lhs);
    }

    void floating_point(floating_point_t /*rhs*/) {
      std::ostringstream buf;
      buf << "num_sym_exec::assign(): trying to assign a float to integer "
             "variable "
          << _lhs;
      throw type_error(buf.str());
    }

    void undefined() {
      _inv -= _lhs;
      ikos::uninit_domain_traits::make_uninitialized(_inv, _lhs);
    }

    void null() {
      std::ostringstream buf;
      buf << "num_sym_exec::assign(): trying to assign null to integer "
             "variable "
          << _lhs;
      throw type_error(buf.str());
    }

    void integer_var(variable_name_t rhs) {
      // update numerical abstraction
      _inv.assign(_lhs, int_linear_expression_t(rhs));

      // update uninitialized variables
      ikos::uninit_domain_traits::assign_uninitialized(_inv, _lhs, rhs);
    }

    void floating_point_var(variable_name_t rhs) {
      std::ostringstream buf;
      buf << "num_sym_exec::assign(): trying to assign float variable " << rhs
          << " to integer variable " << _lhs;
      throw type_error(buf.str());
    }

    void pointer_var(variable_name_t rhs) {
      std::ostringstream buf;
      buf << "num_sym_exec::assign(): trying to assign pointer variable " << rhs
          << " to integer variable " << _lhs;
      throw type_error(buf.str());
    }

  }; // end class integer_assign

  class floating_point_assign : public scalar_lit_t::template visitor<> {
  private:
    variable_name_t _lhs;
    AbsValueDomain& _inv;

  public:
    floating_point_assign(variable_name_t lhs, AbsValueDomain& inv)
        : _lhs(lhs), _inv(inv) {}

    void integer(integer_t /*rhs*/) {
      std::ostringstream buf;
      buf << "num_sym_exec::assign(): trying to assign an integer to float "
             "variable "
          << _lhs;
      throw type_error(buf.str());
    }

    void floating_point(floating_point_t rhs) {
      // TODO: update numerical abstraction
      _inv -= _lhs;

      // update uninitialized variables
      ikos::uninit_domain_traits::make_initialized(_inv, _lhs);
    }

    void undefined() {
      _inv -= _lhs;
      ikos::uninit_domain_traits::make_uninitialized(_inv, _lhs);
    }

    void null() {
      std::ostringstream buf;
      buf << "num_sym_exec::assign(): trying to assign null to float variable "
          << _lhs;
      throw type_error(buf.str());
    }

    void integer_var(variable_name_t rhs) {
      std::ostringstream buf;
      buf << "num_sym_exec::assign(): trying to assign integer variable " << rhs
          << " to float variable " << _lhs;
      throw type_error(buf.str());
    }

    void floating_point_var(variable_name_t rhs) {
      // TODO: update numerical abstraction
      _inv -= _lhs;

      // update uninitialized variables
      ikos::uninit_domain_traits::assign_uninitialized(_inv, _lhs, rhs);
    }

    void pointer_var(variable_name_t rhs) {
      std::ostringstream buf;
      buf << "num_sym_exec::assign(): trying to assign pointer variable " << rhs
          << " to float variable " << _lhs;
      throw type_error(buf.str());
    }

  }; // end class floating_point_assign

  class pointer_assign : public scalar_lit_t::template visitor<> {
  private:
    variable_name_t _lhs;
    AbsValueDomain& _inv;

  public:
    pointer_assign(variable_name_t lhs, AbsValueDomain& inv)
        : _lhs(lhs), _inv(inv) {}

    void integer(integer_t /*rhs*/) {
      std::ostringstream buf;
      buf << "num_sym_exec::assign(): trying to assign an integer to pointer "
             "variable "
          << _lhs;
      throw type_error(buf.str());
    }

    void floating_point(floating_point_t /*rhs*/) {
      std::ostringstream buf;
      buf << "num_sym_exec::assign(): trying to assign a float to pointer "
             "variable "
          << _lhs;
      throw type_error(buf.str());
    }

    void undefined() {
      ikos::mem_domain_traits::forget_mem_surface(_inv, _lhs);
      ikos::uninit_domain_traits::make_uninitialized(_inv, _lhs);
    }

    void null() {
      ikos::mem_domain_traits::forget_mem_surface(_inv, _lhs);
      ikos::uninit_domain_traits::make_initialized(_inv, _lhs);
      ikos::null_domain_traits::make_null(_inv, _lhs);
    }

    void integer_var(variable_name_t rhs) {
      std::ostringstream buf;
      buf << "num_sym_exec::assign(): trying to assign integer variable " << rhs
          << " to pointer variable " << _lhs;
      throw type_error(buf.str());
    }

    void floating_point_var(variable_name_t rhs) {
      std::ostringstream buf;
      buf << "num_sym_exec::assign(): trying to assign float variable " << rhs
          << " to pointer variable " << _lhs;
      throw type_error(buf.str());
    }

    void pointer_var(variable_name_t rhs) {
      // update pointer info and offset
      ikos::ptr_domain_traits::assign_pointer(_inv, _lhs, rhs);

      // update uninitialized variables
      ikos::uninit_domain_traits::assign_uninitialized(_inv, _lhs, rhs);

      // update nullity
      ikos::null_domain_traits::assign_nullity(_inv, _lhs, rhs);
    }

  }; // end class pointer_assign

public:
  // Model a scalar assignment lhs := rhs
  void assign(const scalar_lit_t& lhs, const scalar_lit_t& rhs) {
    if (lhs.is_integer_var()) {
      integer_assign v(lhs.var(), _inv);
      rhs.apply_visitor(v);
    } else if (lhs.is_floating_point_var()) {
      floating_point_assign v(lhs.var(), _inv);
      rhs.apply_visitor(v);
    } else if (lhs.is_pointer_var()) {
      if (_prec_level < PTR) // ignore pointers
        return;

      pointer_assign v(lhs.var(), _inv);
      rhs.apply_visitor(v);
    } else {
      throw analyzer_error(
          "num_sym_exec::assign(): left hand side is not a variable");
    }
  }

private:
  /*
   * Helpers for aggregate (struct,array) instructions
   *
   * Aggregates in internal variables are modeled as if they were in the
   * memory, at a symbolic location. Insert_Element and Extract_Element are
   * thus translated into a memory write/read.
   */

  // Return a pointer to the symbolic location of the aggregate in memory
  scalar_lit_t ptr_var_aggregate(const aggregate_lit_t& aggregate) {
    assert(aggregate.is_var());
    internal_variable_name* agg_var =
        cast< internal_variable_name >(aggregate.var());

    scalar_lit_t ptr = scalar_lit_t::pointer_var(aggregate.var());
    allocate_memory(aggregate.var(),
                    _mfac.get_aggregate(agg_var->internal_var()),
                    nullity_value::non_null,
                    uninit_value::initialized);
    return ptr;
  }

  // Write an aggregate in the memory
  void mem_write_aggregate(const scalar_lit_t& ptr,
                           const aggregate_lit_t& aggregate) {
    if (!ptr.is_pointer_var()) {
      throw analyzer_error(
          "num_sym_exec::mem_write_aggregate(): unexpected pointer");
    }

    if (_prec_level < MEM)
      return;

    if (aggregate.is_cst()) {
      // pointer to write the aggregate in the memory
      scalar_lit_t write_ptr = scalar_lit_t::pointer_var(
          _vfac.get_special_shadow("shadow.mem_write_aggregate.ptr"));

      for (auto it = aggregate.fields().begin(); it != aggregate.fields().end();
           ++it) {
        pointer_shift(write_ptr, ptr, scalar_lit_t::integer(it->offset));
        ikos::mem_domain_traits::mem_write(_inv,
                                           _vfac,
                                           write_ptr.var(),
                                           it->value,
                                           it->size);
      }

      // clean-up
      ikos::mem_domain_traits::forget_mem_surface(_inv, write_ptr.var());
    } else if (aggregate.is_undefined()) {
      ikos::mem_domain_traits::mem_write(_inv,
                                         _vfac,
                                         ptr.var(),
                                         scalar_lit_t::undefined(),
                                         aggregate.size());
    } else if (aggregate.is_var()) {
      scalar_lit_t aggregate_ptr = ptr_var_aggregate(aggregate);
      ikos::mem_domain_traits::mem_copy(_inv,
                                        _vfac,
                                        ptr.var(),
                                        aggregate_ptr.var(),
                                        int_linear_expression_t(
                                            aggregate.size()));
    } else {
      throw analyzer_error("unreachable");
    }
  }

  // Model an aggregate assignment lhs := rhs
  void assign(const aggregate_lit_t& lhs, const aggregate_lit_t& rhs) {
    if (!lhs.is_var()) {
      throw analyzer_error(
          "num_sym_exec::assign(): left hand side is not a variable");
    }

    if (_prec_level < MEM)
      return;

    scalar_lit_t lhs_ptr = ptr_var_aggregate(lhs);
    mem_write_aggregate(lhs_ptr, rhs);
  }

  /*
   * Helpers for integer instructions
   */

  // Execute a binary operation in which all operands are integer scalars.
  template < typename Op >
  void binary_operation(const scalar_lit_t& lhs,
                        Op op,
                        const scalar_lit_t& left,
                        const scalar_lit_t& right) {
    if (!lhs.is_integer_var()) {
      throw analyzer_error(
          "num_sym_exec::binary_operation(): left hand side is not an integer "
          "variable");
    }

    if (left.is_undefined() || right.is_undefined()) {
      _inv -= lhs.var();
      ikos::uninit_domain_traits::make_uninitialized(_inv, lhs.var());
      return;
    }

    if (left.is_integer_var() && right.is_integer_var()) {
      // update numerical abstraction
      _inv.apply(op, lhs.var(), left.var(), right.var());

      // update uninitialized variables
      ikos::uninit_domain_traits::assign_uninitialized(_inv,
                                                       lhs.var(),
                                                       left.var(),
                                                       right.var());
    } else if (left.is_integer_var() && right.is_integer()) {
      // update numerical abstraction
      _inv.apply(op, lhs.var(), left.var(), right.integer());

      // update uninitialized variables
      ikos::uninit_domain_traits::assign_uninitialized(_inv,
                                                       lhs.var(),
                                                       left.var());
    } else if (left.is_integer() && right.is_integer_var()) {
      // update numerical abstraction
      _inv.assign(lhs.var(), left.integer());
      _inv.apply(op, lhs.var(), lhs.var(), right.var());

      // update uninitialized variables
      ikos::uninit_domain_traits::assign_uninitialized(_inv,
                                                       lhs.var(),
                                                       right.var());
    } else if (left.is_integer() && right.is_integer()) {
      // update numerical abstraction
      _inv.assign(lhs.var(), left.integer());
      _inv.apply(op, lhs.var(), lhs.var(), right.integer());

      // update uninitialized variables
      ikos::uninit_domain_traits::make_initialized(_inv, lhs.var());
    } else {
      throw analyzer_error(
          "num_sym_exec::binary_operation(): unexpected arguments");
    }
  }

  // Execute a conversion operation in which all operands are integer scalars.
  void conv_operation(const scalar_lit_t& lhs,
                      ikos::conv_operation_t op,
                      const scalar_lit_t& rhs,
                      uint64_t from_width,
                      uint64_t to_width) {
    if (!lhs.is_integer_var()) {
      throw analyzer_error(
          "num_sym_exec::conv_operation(): left hand side is not an integer "
          "variable");
    }

    if (rhs.is_undefined()) {
      _inv -= lhs.var();
      ikos::uninit_domain_traits::make_uninitialized(_inv, lhs.var());
      return;
    }

    if (rhs.is_integer_var()) {
      // update numerical abstraction
      _inv.apply(op, lhs.var(), rhs.var(), from_width, to_width);

      // update uninitialized variables
      ikos::uninit_domain_traits::assign_uninitialized(_inv,
                                                       lhs.var(),
                                                       rhs.var());
    } else if (rhs.is_integer()) {
      // update numerical abstraction
      _inv.apply(op, lhs.var(), rhs.integer(), from_width, to_width);

      // update uninitialized variables
      ikos::uninit_domain_traits::make_initialized(_inv, lhs.var());
    } else {
      throw analyzer_error(
          "num_sym_exec::conv_operation(): unexpected arguments");
    }
  }

private:
  int_linear_constraint_system_t make_linear_csts(CompOp pred,
                                                  bool positive,
                                                  int_linear_expression_t x,
                                                  int_linear_expression_t y) {
    int_linear_constraint_system_t csts;

    switch (pred) {
      case eq:
        csts += (positive ? x == y : x != y);
        break;
      case ne:
        csts += (positive ? x != y : x == y);
        break;
      case ult:
      case slt:
        csts += (positive ? x <= y - 1 : x >= y);
        break;
      case ule:
      case sle:
        csts += (positive ? x <= y : x >= y + 1);
        break;
      default: { throw analyzer_error("unreachable"); }
    }

    if (pred == ult || pred == ule) {
      if (positive) {
        csts += (x >= 0);
      } else {
        csts += (y >= 0);
      }
    }

    return csts;
  }

  int_linear_constraint_system_t make_linear_csts(CompOp pred,
                                                  bool positive,
                                                  const scalar_lit_t& x,
                                                  const scalar_lit_t& y) {
    int_linear_expression_t x_expr;
    int_linear_expression_t y_expr;

    if (x.is_integer_var()) {
      x_expr = int_linear_expression_t(x.var());
    } else if (x.is_integer()) {
      x_expr = int_linear_expression_t(x.integer());
    } else {
      throw analyzer_error(
          "num_sym_exec:make_linear_csts(): unexpected operand");
    }

    if (y.is_integer_var()) {
      y_expr = int_linear_expression_t(y.var());
    } else if (y.is_integer()) {
      y_expr = int_linear_expression_t(y.integer());
    } else {
      throw analyzer_error(
          "num_sym_exec:make_linear_csts(): unexpected operand");
    }

    return make_linear_csts(pred, positive, x_expr, y_expr);
  }

public:
  /*
   * Helpers for exceptions
   */

  // Randomly throw unknown exceptions with the current invariant.
  // Equivalent to if (rand()) { throw rand(); }
  void throw_unknown_exceptions() {
    AbsValueDomain exc_inv = _inv;
    variable_name_t exc_var =
        _vfac.get_special_shadow("shadow.throw_unknown_exceptions.exc");
    ikos::mem_domain_traits::forget_mem_surface(exc_inv, exc_var);
    ikos::exc_domain_traits::resume_exception(exc_inv, exc_var);
    _inv = _inv | exc_inv;
  }

public:
  /*
   * Function calls (match up/down)
   */

  void match_down(IvRange formals, OpRange actuals, bool is_va_arg) {
    if ((!is_va_arg && actuals.size() != formals.size()) ||
        (is_va_arg && actuals.size() < formals.size())) {
      throw analyzer_error(
          "num_sym_exec::match_down(): number of arguments does not match");
    }

    IvRange::iterator f = formals.begin();
    OpRange::iterator a = actuals.begin();
    for (; f != formals.end(); ++f, ++a) {
      init_global_var_ptr(*a);

      try {
        assign(_lfac[*f], _lfac[*a]);
      } catch (aggregate_literal_error&) {
        assign(_lfac.lookup_aggregate(*f), _lfac.lookup_aggregate(*a));
      }
    }

    if (is_va_arg) {
      if (_arch == "x86_64") {
        match_down_va_arg_x86_64(formals, actuals);
      } else if (_arch == "i386" || _arch == "i686") {
        match_down_va_arg_x86_32(formals, actuals);
      } else {
        throw analyzer_error(
            "unsupported architecture, unable to analyze call with variable "
            "arguments");
      }
    }
  }

private:
  /*
   * Modeling of va_arg implementation for x86_64
   *
   * See https://www.uclibc.org/docs/psABI-x86_64.pdf, p50
   */
  void match_down_va_arg_x86_64(IvRange formals, OpRange actuals) {
    if (_prec_level < PTR)
      return;

    // create reg_save_area and overflow_arg_area as shadow variables
    // FIXME: unsound if there is a call of a va_arg function inside another
    // va_arg function, because of the naming convention.
    scalar_lit_t reg_save_area_ptr = scalar_lit_t::pointer_var(
        _vfac.get_special_shadow("shadow.va_arg.reg_save_area"));
    scalar_lit_t overflow_arg_area_ptr = scalar_lit_t::pointer_var(
        _vfac.get_special_shadow("shadow.va_arg.overflow_arg_area"));
    memloc_t reg_save_area_ml = _mfac.get_va_arg("shadow.va_arg.reg_save_area");
    memloc_t overflow_arg_area_ml =
        _mfac.get_va_arg("shadow.va_arg.overflow_arg_area");

    allocate_memory(reg_save_area_ptr.var(),
                    reg_save_area_ml,
                    nullity_value::non_null,
                    uninit_value::initialized);
    allocate_memory(overflow_arg_area_ptr.var(),
                    overflow_arg_area_ml,
                    nullity_value::non_null,
                    uninit_value::initialized);

    integer_t gp_offset(0);
    integer_t fp_offset(6 * 8);
    integer_t overflow_arg_area_size(0);

    // pointer to write on reg_save_area/overflow_arg_area
    scalar_lit_t write_ptr = scalar_lit_t::pointer_var(
        _vfac.get_special_shadow("shadow.va_arg.ptr"));

    // push everything on the stack
    integer_t gp_curr_offset(0);
    integer_t fp_curr_offset(6 * 8);
    IvRange::iterator f = formals.begin();
    OpRange::iterator a = actuals.begin();
    for (; a != actuals.end(); ++a) {
      try {
        init_global_var_ptr(*a);
        scalar_lit_t v = _lfac[*a];

        if (v.is_undefined()) {
          ikos::exc_domain_traits::set_normal_flow_bottom(_inv);
          return;
        } else if (v.is_integer() || v.is_integer_var() || v.is_null() ||
                   v.is_pointer_var()) {
          if (gp_curr_offset < 6 * 8) {
            // push on reg_save_area
            if (_prec_level >= MEM) {
              pointer_shift(write_ptr,
                            reg_save_area_ptr,
                            scalar_lit_t::integer(gp_curr_offset));
              ikos::mem_domain_traits::mem_write(_inv,
                                                 _vfac,
                                                 write_ptr.var(),
                                                 v,
                                                 integer_t(8));
            }

            // update gp_offset
            gp_curr_offset += 8;
            if (f != formals.end()) {
              gp_offset += 8;
            }
          } else if (f != formals.end()) {
            // ignore
          } else {
            // push on overflow_arg_area
            if (_prec_level >= MEM) {
              pointer_shift(write_ptr,
                            overflow_arg_area_ptr,
                            scalar_lit_t::integer(overflow_arg_area_size));
              ikos::mem_domain_traits::mem_write(_inv,
                                                 _vfac,
                                                 write_ptr.var(),
                                                 v,
                                                 integer_t(8));
            }

            // update overflow_arg_area_size
            overflow_arg_area_size += 8;
          }
        } else if (v.is_floating_point() || v.is_floating_point_var()) {
          if (fp_curr_offset < 6 * 8 + 8 * 16) {
            // push on reg_save_area
            if (_prec_level >= MEM) {
              pointer_shift(write_ptr,
                            reg_save_area_ptr,
                            scalar_lit_t::integer(fp_curr_offset));
              ikos::mem_domain_traits::mem_write(_inv,
                                                 _vfac,
                                                 write_ptr.var(),
                                                 v,
                                                 integer_t(16));
            }

            // update fp_offset
            fp_curr_offset += 16;
            if (f != formals.end()) {
              fp_offset += 16;
            }
          } else if (f != formals.end()) {
            // ignore
          } else {
            // push on overflow_arg_area
            if (_prec_level >= MEM) {
              pointer_shift(write_ptr,
                            overflow_arg_area_ptr,
                            scalar_lit_t::integer(overflow_arg_area_size));
              ikos::mem_domain_traits::mem_write(_inv,
                                                 _vfac,
                                                 write_ptr.var(),
                                                 v,
                                                 integer_t(8));
            }

            // update overflow_arg_area_size
            overflow_arg_area_size += 8;
          }
        } else {
          throw analyzer_error("unreachable");
        }

        if (f != formals.end()) {
          ++f;
        }
      } catch (aggregate_literal_error&) {
        throw analyzer_error(
            "aggregate argument in function call with variable arguments are "
            "not currently supported");
      }
    }

    // update sizes
    _inv.assign(alloc_size_var(_vfac, reg_save_area_ml),
                fp_curr_offset > 6 * 8 ? fp_curr_offset : gp_curr_offset);
    _inv.assign(alloc_size_var(_vfac, overflow_arg_area_ml),
                overflow_arg_area_size);

    // assign gp_offset, fp_offset
    assign(scalar_lit_t::integer_var(
               _vfac.get_special_shadow("shadow.va_arg.gp_offset")),
           scalar_lit_t::integer(gp_offset));
    assign(scalar_lit_t::integer_var(
               _vfac.get_special_shadow("shadow.va_arg.fp_offset")),
           scalar_lit_t::integer(fp_offset));

    // clean-up
    ikos::mem_domain_traits::forget_mem_surface(_inv, write_ptr.var());
  }

  /*
   * Modeling of va_arg implementation of i386, i686 (x86_32)
   *
   * On that architecture, va_list contains a pointer on the stack
   */
  void match_down_va_arg_x86_32(IvRange formals, OpRange actuals) {
    if (_prec_level < PTR)
      return;

    // create args_area to store parameters
    scalar_lit_t args_area_ptr = scalar_lit_t::pointer_var(
        _vfac.get_special_shadow("shadow.va_arg.args_area"));
    memloc_t args_area_ml = _mfac.get_va_arg("shadow.va_arg.args_area");

    allocate_memory(args_area_ptr.var(),
                    args_area_ml,
                    nullity_value::non_null,
                    uninit_value::initialized);

    integer_t args_area_size(0);

    // pointer to write on args_area
    scalar_lit_t write_ptr = scalar_lit_t::pointer_var(
        _vfac.get_special_shadow("shadow.va_arg.ptr"));

    IvRange::iterator f = formals.begin();
    OpRange::iterator a = actuals.begin();
    for (; a != actuals.end(); ++a) {
      try {
        init_global_var_ptr(*a);
        scalar_lit_t v = _lfac[*a];

        if (v.is_undefined()) {
          ikos::exc_domain_traits::set_normal_flow_bottom(_inv);
          return;
        } else if (f != formals.end()) {
          // ignore
        } else if (v.is_integer() || v.is_integer_var() || v.is_null() ||
                   v.is_pointer_var()) {
          // push on args_area
          if (_prec_level >= MEM) {
            pointer_shift(write_ptr,
                          args_area_ptr,
                          scalar_lit_t::integer(args_area_size));
            ikos::mem_domain_traits::mem_write(_inv,
                                               _vfac,
                                               write_ptr.var(),
                                               v,
                                               integer_t(4));
          }

          // update args_area_size
          args_area_size += 4;
        } else if (v.is_floating_point() || v.is_floating_point_var()) {
          // push on args_area
          if (_prec_level >= MEM) {
            pointer_shift(write_ptr,
                          args_area_ptr,
                          scalar_lit_t::integer(args_area_size));
            ikos::mem_domain_traits::mem_write(_inv,
                                               _vfac,
                                               write_ptr.var(),
                                               v,
                                               integer_t(8));
          }

          // update args_area_size
          args_area_size += 8;
        } else {
          throw analyzer_error("unreachable");
        }

        if (f != formals.end()) {
          ++f;
        }
      } catch (aggregate_literal_error&) {
        throw analyzer_error(
            "aggregate argument in function call with variable arguments are "
            "not currently supported");
      }
    }

    // update size
    _inv.assign(alloc_size_var(_vfac, args_area_ml), args_area_size);

    // clean-up
    ikos::mem_domain_traits::forget_mem_surface(_inv, write_ptr.var());
  }

public:
  void match_up(OpRange /*actuals*/,
                boost::optional< Internal_Variable_ref > lhs_cs_ret,
                IvRange /*formals*/,
                boost::optional< Operand_ref > callee_ret,
                bool is_va_arg) {
    if (lhs_cs_ret && callee_ret) {
      init_global_var_ptr(*callee_ret);

      try {
        assign(_lfac[*lhs_cs_ret], _lfac[*callee_ret]);
      } catch (aggregate_literal_error&) {
        assign(_lfac.lookup_aggregate(*lhs_cs_ret),
               _lfac.lookup_aggregate(*callee_ret));
      }
    }

    if (is_va_arg) {
      if (_arch == "x86_64") {
        match_up_va_arg_x86_64();
      } else if (_arch == "i386" || _arch == "i686") {
        match_up_va_arg_x86_32();
      } else {
        throw analyzer_error(
            "unsupported architecture, unable to analyze call with variable "
            "arguments");
      }
    }
  }

private:
  void match_up_va_arg_x86_64() {
    if (_prec_level < PTR)
      return;

    scalar_lit_t reg_save_area_ptr = scalar_lit_t::pointer_var(
        _vfac.get_special_shadow("shadow.va_arg.reg_save_area"));
    scalar_lit_t overflow_arg_area_ptr = scalar_lit_t::pointer_var(
        _vfac.get_special_shadow("shadow.va_arg.overflow_arg_area"));
    scalar_lit_t gp_offset_var = scalar_lit_t::integer_var(
        _vfac.get_special_shadow("shadow.va_arg.gp_offset"));
    scalar_lit_t fp_offset_var = scalar_lit_t::integer_var(
        _vfac.get_special_shadow("shadow.va_arg.fp_offset"));

    memloc_t reg_save_area_ml = _mfac.get_va_arg("shadow.va_arg.reg_save_area");
    memloc_t overflow_arg_area_ml =
        _mfac.get_va_arg("shadow.va_arg.overflow_arg_area");

    // FIXME: clean-up the memory even when an exception has been thrown

    _inv.assign(alloc_size_var(_vfac, reg_save_area_ml), 0);
    _inv.assign(alloc_size_var(_vfac, overflow_arg_area_ml), 0);

    if (_prec_level >= MEM) {
      ikos::mem_domain_traits::forget_mem_contents(_inv,
                                                   reg_save_area_ptr.var());
      ikos::mem_domain_traits::forget_mem_contents(_inv,
                                                   overflow_arg_area_ptr.var());
    }

    ikos::mem_domain_traits::forget_mem_surface(_inv, reg_save_area_ptr.var());
    ikos::mem_domain_traits::forget_mem_surface(_inv,
                                                overflow_arg_area_ptr.var());
    ikos::mem_domain_traits::forget_mem_surface(_inv, gp_offset_var.var());
    ikos::mem_domain_traits::forget_mem_surface(_inv, fp_offset_var.var());
  }

  void match_up_va_arg_x86_32() {
    if (_prec_level < PTR)
      return;

    scalar_lit_t args_area_ptr = scalar_lit_t::pointer_var(
        _vfac.get_special_shadow("shadow.va_arg.args_area"));

    memloc_t args_area_ml = _mfac.get_va_arg("shadow.va_arg.args_area");

    // FIXME: clean-up the memory even when an exception has been thrown

    _inv.assign(alloc_size_var(_vfac, args_area_ml), 0);

    if (_prec_level >= MEM) {
      ikos::mem_domain_traits::forget_mem_contents(_inv, args_area_ptr.var());
    }

    ikos::mem_domain_traits::forget_mem_surface(_inv, args_area_ptr.var());
  }

public:
  void deallocate_local_vars(LvRange local_vars) {
    if (_prec_level < PTR)
      return;

    if (local_vars.empty())
      return;

    variable_name_t exc_var = _vfac.get_special_shadow("shadow.deallocate.exc");

    /*
     * Deallocate the local variables, both in the current abstract domain,
     * but also in the abstract domains representing the pending exceptions
     *
     * For that, we simulate a:
     * catch(exc) { deallocate(local_vars); throw exc; }
     */
    AbsValueDomain normal = _inv;
    ikos::exc_domain_traits::ignore_exceptions(normal);
    AbsValueDomain pending_exceptions = _inv;
    ikos::exc_domain_traits::enter_catch(pending_exceptions, exc_var);

    for (auto it = local_vars.begin(); it != local_vars.end(); ++it) {
      scalar_lit_t v = _lfac[*it];
      memory_location_t memloc_v = _mfac.get_local(*it);

      // FIX: the liveness analysis removes local variables before
      // deallocate_local_vars() is called, so we need to initialize them
      // correctly here before calling forget_mem_contents()
      // TODO: the liveness analysis should only deal with AR_Internal_Variable
      ikos::ptr_domain_traits::assign_object(normal, v.var(), memloc_v);
      ikos::ptr_domain_traits::assign_object(pending_exceptions,
                                             v.var(),
                                             memloc_v);

      // set the allocated size to 0
      normal.assign(alloc_size_var(_vfac, memloc_v), 0);
      pending_exceptions.assign(alloc_size_var(_vfac, memloc_v), 0);

      if (_prec_level >= MEM) {
        // forget the memory content
        ikos::mem_domain_traits::forget_mem_contents(normal, v.var());
        ikos::mem_domain_traits::forget_mem_contents(pending_exceptions,
                                                     v.var());
      }

      // forget local variable pointer
      ikos::mem_domain_traits::forget_mem_surface(normal, v.var());
      ikos::mem_domain_traits::forget_mem_surface(pending_exceptions, v.var());
    }

    ikos::exc_domain_traits::resume_exception(pending_exceptions, exc_var);
    _inv = normal | pending_exceptions;
  }

  void exec_start(Basic_Block_ref bb) {
    // check if it is a catch block
    BBRange preds = ar::getPreds(bb);
    if (!preds.empty()) {
      // check if all predecessors end with an invoke statement and if
      // this basic block is the normal/non-exception path.
      bool invoke_normal = true;
      for (auto it = preds.begin(); it != preds.end() && invoke_normal; ++it) {
        StmtRange stmts = ar::getStatements(*it);
        if (!stmts.empty() && ar::is_invoke_stmt(stmts.back())) {
          Invoke_ref invoke = node_cast< Invoke >(stmts.back());
          invoke_normal &= (invoke->getNormal() == bb);
        } else {
          invoke_normal = false;
        }
      }

      // mark the beginning of a block that should be executed only if there
      // was no exception
      if (invoke_normal) {
        ikos::exc_domain_traits::ignore_exceptions(_inv);
      }
      // ikos::exc_domain_traits::enter_catch() is called when we reach the
      // landing pad statement
    }
  }

  void exec_end(Basic_Block_ref bb) {
    // Note that _dead_vars do not contain shadow variables (internal
    // variables added by the analyses) since shadow variables do not
    // appear in the ARBOS CFG.
    for (auto it = _dead_vars.begin(); it != _dead_vars.end(); ++it) {
      ikos::mem_domain_traits::forget_mem_surface(_inv, *it);
    }

    // Call init_counter() for each non-negative loop counter
    for (auto it = _loop_counters.begin(); it != _loop_counters.end(); ++it) {
      ikos::counter_domain_traits::mark_counter(_inv, *it);
    }
  }

  void exec(Assignment_ref stmt) {
    init_global_var_ptr(ar::getRightOp(stmt));

    try {
      assign(_lfac[ar::getLeftOp(stmt)], _lfac[ar::getRightOp(stmt)]);
    } catch (aggregate_literal_error&) {
      assign(_lfac.lookup_aggregate(ar::getLeftOp(stmt)),
             _lfac.lookup_aggregate(ar::getRightOp(stmt)));
    }
  }

  /*
   * Arithmetic instructions
   */

  void exec(Arith_Op_ref stmt) {
    scalar_lit_t lhs = _lfac[ar::getResult(stmt)];
    scalar_lit_t left = _lfac[ar::getLeftOp(stmt)];
    scalar_lit_t right = _lfac[ar::getRightOp(stmt)];

    switch (ar::getArithOp(stmt)) {
      case arbos::add:
        binary_operation(lhs, ikos::OP_ADDITION, left, right);
        break;
      case arbos::sub:
        binary_operation(lhs, ikos::OP_SUBTRACTION, left, right);
        break;
      case arbos::mul:
        binary_operation(lhs, ikos::OP_MULTIPLICATION, left, right);
        break;
      case arbos::udiv:
        binary_operation(lhs, ikos::OP_UDIV, left, right);
        break;
      case arbos::sdiv:
        binary_operation(lhs, ikos::OP_SDIV, left, right);
        break;
      case arbos::urem:
        binary_operation(lhs, ikos::OP_UREM, left, right);
        break;
      case arbos::srem:
        binary_operation(lhs, ikos::OP_SREM, left, right);
        break;
      default:
        throw analyzer_error("unreachable");
    }
  }

  void exec(Integer_Comparison_ref stmt) {
    scalar_lit_t x = _lfac[ar::getLeftOp(stmt)];
    scalar_lit_t y = _lfac[ar::getRightOp(stmt)];
    CompOp pred = ar::getPredicate(stmt);
    bool positive = ar::isPositive(stmt);

    if (x.is_undefined() || y.is_undefined()) // unknown behavior
      return;

    if (x.is_pointer_var() || x.is_null() || y.is_pointer_var() ||
        y.is_null()) { // pointer comparison
      if (_prec_level < PTR)
        return;

      CompOp n_pred;

      if (pred == eq) {
        n_pred = positive ? eq : ne;
      } else if (pred == ne) {
        n_pred = positive ? ne : eq;
      } else {
        // reason only about equalities and disequalities between pointers
        return;
      }

      // create lazily memory objects
      init_global_var_ptr(ar::getLeftOp(stmt));
      init_global_var_ptr(ar::getRightOp(stmt));

      if (x.is_null() && y.is_null()) { // null == null or null != null
        if (n_pred == ne) {
          ikos::exc_domain_traits::set_normal_flow_bottom(_inv);
        }
      } else if (x.is_pointer_var() && y.is_null()) { // x == null or x != null
        // reduction with the external pointer analysis
        refine_addrs(x.var());
        if (n_pred == eq) {
          ikos::null_domain_traits::assert_null(_inv, x.var());
        } else {
          ikos::null_domain_traits::assert_non_null(_inv, x.var());
        }
      } else if (x.is_null() && y.is_pointer_var()) { // y == null or y != null
        // reduction with the external pointer analysis
        refine_addrs(y.var());
        if (n_pred == eq) {
          ikos::null_domain_traits::assert_null(_inv, y.var());
        } else {
          ikos::null_domain_traits::assert_non_null(_inv, y.var());
        }
      } else if (x.is_pointer_var() && y.is_pointer_var()) {
        // reduction with the external pointer analysis
        refine_addrs_offset(x.var());
        refine_addrs_offset(y.var());
        ikos::null_domain_traits::assert_nullity(_inv,
                                                 n_pred == eq,
                                                 x.var(),
                                                 y.var());
        ikos::ptr_domain_traits::assert_pointer(_inv,
                                                n_pred == eq,
                                                x.var(),
                                                y.var());
      } else {
        throw analyzer_error(
            "num_sym_exec::exec(Integer_Comparison): unexpected operands");
      }
    } else { // integer comparison
      _inv += make_linear_csts(pred, positive, x, y);
    }
  }

  void exec(FP_Comparison_ref) {
    // no floating point reasoning
  }

  void exec(Bitwise_Op_ref stmt) {
    scalar_lit_t lhs = _lfac[ar::getResult(stmt)];
    scalar_lit_t left = _lfac[ar::getLeftOp(stmt)];
    scalar_lit_t right = _lfac[ar::getRightOp(stmt)];

    switch (ar::getBitwiseOp(stmt)) {
      case arbos::_and:
        binary_operation(lhs, ikos::OP_AND, left, right);
        break;
      case arbos::_or:
        binary_operation(lhs, ikos::OP_OR, left, right);
        break;
      case arbos::_xor:
        binary_operation(lhs, ikos::OP_XOR, left, right);
        break;
      case arbos::_shl:
        binary_operation(lhs, ikos::OP_SHL, left, right);
        break;
      case arbos::_lshr:
        binary_operation(lhs, ikos::OP_LSHR, left, right);
        break;
      case arbos::_ashr:
        binary_operation(lhs, ikos::OP_ASHR, left, right);
        break;
      default:
        throw analyzer_error("unreachable");
    }
  }

  void exec(FP_Op_ref stmt) {
    scalar_lit_t lhs = _lfac[ar::getResult(stmt)];
    scalar_lit_t left = _lfac[ar::getLeftOp(stmt)];
    scalar_lit_t right = _lfac[ar::getRightOp(stmt)];

    if (!lhs.is_floating_point_var()) {
      throw analyzer_error(
          "num_sym_exec::exec(FP_Op): left hand side is not a float variable");
    }

    // no floating point reasoning
    _inv -= lhs.var();

    // update uninitialized variables

    if (left.is_undefined() || right.is_undefined()) {
      ikos::uninit_domain_traits::make_uninitialized(_inv, lhs.var());
      return;
    }

    if (left.is_floating_point_var() && right.is_floating_point_var()) {
      ikos::uninit_domain_traits::assign_uninitialized(_inv,
                                                       lhs.var(),
                                                       left.var(),
                                                       right.var());
    } else if (left.is_floating_point_var() && right.is_floating_point()) {
      ikos::uninit_domain_traits::assign_uninitialized(_inv,
                                                       lhs.var(),
                                                       left.var());
    } else if (left.is_floating_point() && right.is_floating_point_var()) {
      ikos::uninit_domain_traits::assign_uninitialized(_inv,
                                                       lhs.var(),
                                                       right.var());
    } else if (left.is_floating_point() && right.is_floating_point()) {
      ikos::uninit_domain_traits::make_initialized(_inv, lhs.var());
    } else {
      throw analyzer_error("num_sym_exec::exec(FP_Op): unexpected arguments");
    }
  }

  void exec(Conv_Op_ref stmt) {
    scalar_lit_t dest = _lfac[ar::getLeftOp(stmt)];
    scalar_lit_t src = _lfac[ar::getRightOp(stmt)];
    uint64_t src_width = ar::getRealSize(ar::getType(ar::getRightOp(stmt)));
    uint64_t dest_width = ar::getRealSize(ar::getType(ar::getLeftOp(stmt)));

    if (!dest.is_var()) {
      throw analyzer_error(
          "num_sym_exec::exec(Conv_Op): left hand side is not a variable");
    }

    switch (ar::getConvOp(stmt)) {
      case arbos::trunc:
        conv_operation(dest, ikos::OP_TRUNC, src, src_width, dest_width);
        break;
      case arbos::zext:
        conv_operation(dest, ikos::OP_ZEXT, src, src_width, dest_width);
        break;
      case arbos::sext:
        conv_operation(dest, ikos::OP_SEXT, src, src_width, dest_width);
        break;
      case arbos::fptrunc:
      case arbos::fpext:
      case arbos::fptoui:
      case arbos::fptosi:
      case arbos::uitofp:
      case arbos::sitofp:
        _inv -= dest.var();

        if (src.is_undefined()) {
          ikos::uninit_domain_traits::make_uninitialized(_inv, dest.var());
        } else if (src.is_integer() || src.is_floating_point()) {
          ikos::uninit_domain_traits::make_initialized(_inv, dest.var());
        } else if (src.is_integer_var() || src.is_floating_point_var()) {
          ikos::uninit_domain_traits::assign_uninitialized(_inv,
                                                           dest.var(),
                                                           src.var());
        } else {
          throw analyzer_error("unreachable");
        }
        break;
      case arbos::ptrtoint:
        _inv -= dest.var();

        if (src.is_null() ||
            (src.is_pointer_var() &&
             !ikos::exc_domain_traits::is_normal_flow_bottom(_inv) &&
             ikos::null_domain_traits::is_null(_inv, src.var()))) {
          _inv.assign(dest.var(), int_linear_expression_t(0));
          ikos::uninit_domain_traits::make_initialized(_inv, dest.var());
        } else if (src.is_pointer_var()) {
          ikos::uninit_domain_traits::assign_uninitialized(_inv,
                                                           dest.var(),
                                                           src.var());
        } else {
          throw analyzer_error("unreachable");
        }
        break;
      case arbos::inttoptr: {
        // cast from int to ptr (for instance: int x = 5; int *px = x;)
        if (_prec_level < PTR)
          return;

        if (!dest.is_pointer_var()) {
          throw analyzer_error(
              "num_sym_exec::exec(ConvOp, inttoptr): left hand side is not a "
              "pointer");
        }
        if (!src.is_integer() && !src.is_integer_var()) {
          throw analyzer_error(
              "num_sym_exec::exec(ConvOp, inttoptr): right hand side is not an "
              "integer");
        }

        if (_warning_callback) {
          json_dict info = {{"dest", dest.var()->name()}};
          if (src.is_integer()) {
            info.put("src", src.integer());
          } else {
            info.put("src", src.var()->name());
          }
          _warning_callback->warning("cast-int-ptr",
                                     "cast from integer to pointer (analysis "
                                     "might be unsound)",
                                     stmt,
                                     info);
        }

        // ASSUMPTION: the address points to a unique block of memory
        // If the address is not infered from the numerical domain (we do not
        // have the exact address), do not create a memory location
        if (src.is_integer()) {
          allocate_memory(dest.var(),
                          _mfac.get_fixed_addr(src.integer()),
                          src.integer() == 0 ? nullity_value::null
                                             : nullity_value::non_null,
                          uninit_value::initialized);
        } else if (src.is_integer_var()) {
          int_interval_t addr_itv =
              ikos::num_domain_traits::to_interval(_inv, src.var());
          if (addr_itv.singleton()) {
            allocate_memory(dest.var(),
                            _mfac.get_fixed_addr(*addr_itv.singleton()),
                            *addr_itv.singleton() == 0
                                ? nullity_value::null
                                : nullity_value::non_null,
                            uninit_value::may_uninitialized);
          }
        }

        break;
      }
      case arbos::bitcast:
        if (_prec_level < PTR)
          return;

        if (dest.is_pointer_var()) {
          if (src.is_null() || src.is_undefined() || src.is_pointer_var()) {
            init_global_var_ptr(ar::getRightOp(stmt));
            assign(dest, src);
          } else {
            throw analyzer_error(
                "num_sym_exec::exec(ConvOp, bitcast): unexpected right hand "
                "side");
          }
        } else if (dest.is_integer_var() || dest.is_floating_point_var()) {
          _inv -= dest.var();

          if (src.is_undefined()) {
            ikos::uninit_domain_traits::make_uninitialized(_inv, dest.var());
          } else if (src.is_integer() || src.is_floating_point()) {
            ikos::uninit_domain_traits::make_initialized(_inv, dest.var());
          } else if (src.is_integer_var() || src.is_floating_point_var()) {
            ikos::uninit_domain_traits::assign_uninitialized(_inv,
                                                             dest.var(),
                                                             src.var());
          } else {
            throw analyzer_error("unreachable");
          }
        } else {
          throw analyzer_error("unreachable");
        }
        break;
      default:
        throw analyzer_error("unreachable");
    }
  }

  void exec(Abstract_Variable_ref stmt) {
    // statement never generated by the frontend
    throw analyzer_error("num_sym_exec::exec(AbstractVariable): unreachable");
  }

  /*
   * Exception related instructions
   */

  // Mark the beginning of a catch() {...} block
  void exec(Landing_Pad_ref stmt) {
    Internal_Variable_ref exc = ar::getVar(stmt);
    variable_name_t exc_var = _vfac.get_internal(exc);
    ikos::exc_domain_traits::enter_catch(_inv, exc_var);
  }

  void exec(Resume_ref stmt) {
    Internal_Variable_ref exc = ar::getVar(stmt);
    variable_name_t exc_var = _vfac.get_internal(exc);
    ikos::exc_domain_traits::resume_exception(_inv, exc_var);
  }

  void exec(Unreachable_ref /*stmt*/) {
    // unreachable should propagate exceptions
    ikos::exc_domain_traits::set_normal_flow_bottom(_inv);
  }

  /*
   * Memory instructions
   */

  // Create a memory object for a stack variable
  void exec(Allocate_ref stmt) {
    if (_prec_level < PTR)
      return;

    scalar_lit_t lhs = _lfac[ar::getResult(stmt)];
    scalar_lit_t array_size = _lfac[ar::getArraySize(stmt)];
    memory_location_t mem_lhs =
        _mfac.get_local(node_cast< arbos::AR_Local_Variable >(
            ar::getResult(stmt)->getVariable()));

    if (!lhs.is_pointer_var()) {
      throw analyzer_error(
          "num_sym_exec::exec(Allocate): left hand side is not a pointer "
          "variable");
    }

    allocate_memory(lhs.var(),
                    mem_lhs,
                    nullity_value::non_null,
                    uninit_value::initialized);

    // TODO: Check for integer overflow
    if (array_size.is_integer_var()) {
      _inv.apply(ikos::OP_MULTIPLICATION,
                 alloc_size_var(_vfac, mem_lhs),
                 array_size.var(),
                 ar::getSize(ar::getAllocatedType(stmt)));
    } else if (array_size.is_integer()) {
      _inv.assign(alloc_size_var(_vfac, mem_lhs),
                  array_size.integer() *
                      ar::getSize(ar::getAllocatedType(stmt)));
    } else {
      throw analyzer_error(
          "num_sym_exec::exec(Allocate): unexpected array size parameter");
    }
  }

  void exec(Pointer_Shift_ref stmt) {
    // create lazily a memory object
    init_global_var_ptr(ar::getBase(stmt));

    pointer_shift(_lfac[ar::getResult(stmt)],
                  _lfac[ar::getBase(stmt)],
                  _lfac[ar::getOffset(stmt)]);
  }

  void exec(Store_ref stmt) {
    // create lazily memory objects
    init_global_var_ptr(ar::getPointer(stmt));

    scalar_lit_t ptr = _lfac[ar::getPointer(stmt)];

    if (!prepare_mem_access(ptr))
      return;

    if (ikos::ptr_domain_traits::is_unknown_addr(_inv, ptr.var())) {
      if (_warning_callback) {
        json_dict info = {{"pointer", ptr.var()->name()}};
        std::ostringstream buf;
        buf << "ignored memory write to " << ptr.var()
            << " (analysis might be unsound)";
        _warning_callback->warning("ignored-mem-write", buf.str(), stmt, info);
      }
      return;
    }

    integer_t w_size =
        ar::getSize(ar::getPointeeType(ar::getType(ar::getPointer(stmt))));

    // create lazily memory objects
    init_global_var_ptr(ar::getValue(stmt));

    try {
      scalar_lit_t val = _lfac[ar::getValue(stmt)];

      if (val.is_pointer_var()) {
        refine_addrs_offset(val.var());
      }

      // perform memory write in the value domain
      ikos::mem_domain_traits::mem_write(_inv, _vfac, ptr.var(), val, w_size);
    } catch (aggregate_literal_error&) {
      const aggregate_lit_t& val = _lfac.lookup_aggregate(ar::getValue(stmt));
      mem_write_aggregate(ptr, val);
    }
  }

  void exec(Load_ref stmt) {
    // create lazily a memory object
    init_global_var_ptr(ar::getPointer(stmt));

    scalar_lit_t ptr = _lfac[ar::getPointer(stmt)];

    if (!prepare_mem_access(ptr))
      return;

    integer_t r_size =
        ar::getSize(ar::getPointeeType(ar::getType(ar::getPointer(stmt))));

    try {
      scalar_lit_t lhs = _lfac[ar::getResult(stmt)];

      if (!lhs.is_var()) {
        throw analyzer_error(
            "num_sym_exec::exec(Load): left hand side is not a variable");
      }

      // perform memory read in the value domain
      ikos::mem_domain_traits::mem_read(_inv, _vfac, lhs, ptr.var(), r_size);

      // reduction between value and pointer analysis
      if (lhs.is_pointer_var()) {
        refine_addrs_offset(lhs.var());
      }
    } catch (aggregate_literal_error&) {
      const aggregate_lit_t& lhs = _lfac.lookup_aggregate(ar::getResult(stmt));

      if (!lhs.is_var()) {
        throw analyzer_error(
            "num_sym_exec::exec(Load): left hand side is not a variable");
      }

      scalar_lit_t lhs_ptr = ptr_var_aggregate(lhs);
      ikos::mem_domain_traits::mem_copy(_inv,
                                        _vfac,
                                        lhs_ptr.var(),
                                        ptr.var(),
                                        int_linear_expression_t(r_size));
    }
  }

private:
  // memcpy(dest, src, len)
  // memmove(dest, src, len)
  template < typename MemCpyOrMove_ref >
  void execMemCpyOrMove(MemCpyOrMove_ref stmt) {
    // create lazily memory objects
    init_global_var_ptr(ar::getTarget(stmt));
    init_global_var_ptr(ar::getSource(stmt));

    // Both src and dest must be already allocated in memory so
    // offsets and sizes for both src and dest are already part of the
    // invariants
    scalar_lit_t dest = _lfac[ar::getTarget(stmt)];
    scalar_lit_t src = _lfac[ar::getSource(stmt)];
    scalar_lit_t size = _lfac[ar::getLen(stmt)];

    if (!prepare_mem_access(dest) || !prepare_mem_access(src))
      return;

    if (ikos::ptr_domain_traits::is_unknown_addr(_inv, dest.var())) {
      if (_warning_callback) {
        json_dict info = {{"dest", dest.var()->name()},
                          {"src", src.var()->name()}};
        std::ostringstream buf;
        buf << "ignored memory copy from " << src.var() << " to " << dest.var()
            << " (analysis might be unsound)";
        _warning_callback->warning("ignored-mem-copy", buf.str(), stmt, info);
      }
      return;
    }

    if (size.is_integer_var()) {
      ikos::mem_domain_traits::mem_copy(_inv,
                                        _vfac,
                                        dest.var(),
                                        src.var(),
                                        int_linear_expression_t(size.var()));
    } else if (size.is_integer()) {
      ikos::mem_domain_traits::mem_copy(_inv,
                                        _vfac,
                                        dest.var(),
                                        src.var(),
                                        int_linear_expression_t(
                                            size.integer()));
    } else {
      throw analyzer_error(
          "num_sym_exec::exec(MemCpy|MemMove): unexpected size operand");
    }
  }

public:
  void exec(MemCpy_ref stmt) { return execMemCpyOrMove(stmt); }

  void exec(MemMove_ref stmt) { return execMemCpyOrMove(stmt); }

  void exec(MemSet_ref stmt) {
    // create lazily a memory object
    init_global_var_ptr(ar::getBase(stmt));

    scalar_lit_t dest = _lfac[ar::getBase(stmt)];
    scalar_lit_t value = _lfac[ar::getValue(stmt)];
    scalar_lit_t size = _lfac[ar::getLen(stmt)];

    if (!value.is_integer_var() && !value.is_integer()) {
      throw analyzer_error(
          "num_sym_exec::exec(MemSet): unexpected value operand");
    } else if (!size.is_integer_var() && !size.is_integer()) {
      throw analyzer_error(
          "num_sym_exec::exec(MemSet): unexpected size operand");
    }

    if (!prepare_mem_access(dest))
      return;

    if (ikos::ptr_domain_traits::is_unknown_addr(_inv, dest.var())) {
      if (_warning_callback) {
        json_dict info = {{"pointer", dest.var()->name()}};
        std::ostringstream buf;
        buf << "warning: ignored memory set to " << dest.var()
            << " (analysis might be unsound)";
        _warning_callback->warning("ignored-mem-set", buf.str(), stmt, info);
      }
      return;
    }

    int_linear_expression_t value_expr =
        value.is_integer_var() ? int_linear_expression_t(value.var())
                               : int_linear_expression_t(value.integer());
    int_linear_expression_t size_expr =
        size.is_integer_var() ? int_linear_expression_t(size.var())
                              : int_linear_expression_t(size.integer());
    ikos::mem_domain_traits::mem_set(_inv,
                                     _vfac,
                                     dest.var(),
                                     value_expr,
                                     size_expr);
  }

  void exec(Insert_Element_ref stmt) {
    const aggregate_lit_t& lhs = _lfac.lookup_aggregate(ar::getResult(stmt));
    const aggregate_lit_t& rhs = _lfac.lookup_aggregate(ar::getAggregate(stmt));
    const scalar_lit_t& offset = _lfac[ar::getOffset(stmt)];

    if (!lhs.is_var()) {
      throw analyzer_error(
          "num_sym_exec::exec(Insert_Element): left hand side is not a "
          "variable");
    }

    if (_prec_level < MEM)
      return;

    scalar_lit_t lhs_ptr = ptr_var_aggregate(lhs);

    // first, copy the aggregate value
    mem_write_aggregate(lhs_ptr, rhs);

    // then insert the element
    scalar_lit_t write_ptr = scalar_lit_t::pointer_var(
        _vfac.get_special_shadow("shadow.insert_element.ptr"));
    pointer_shift(write_ptr, lhs_ptr, offset);

    integer_t w_size = ar::getSize(ar::getType(ar::getElement(stmt)));

    try {
      const scalar_lit_t& element = _lfac[ar::getElement(stmt)];
      ikos::mem_domain_traits::mem_write(_inv,
                                         _vfac,
                                         write_ptr.var(),
                                         element,
                                         w_size);
    } catch (aggregate_literal_error&) {
      const aggregate_lit_t& element =
          _lfac.lookup_aggregate(ar::getElement(stmt));
      mem_write_aggregate(write_ptr, element);
    }

    // clean-up
    ikos::mem_domain_traits::forget_mem_surface(_inv, write_ptr.var());
  }

  void exec(Extract_Element_ref stmt) {
    const aggregate_lit_t& rhs = _lfac.lookup_aggregate(ar::getAggregate(stmt));
    const scalar_lit_t& offset = _lfac[ar::getOffset(stmt)];

    if (!rhs.is_var()) {
      throw analyzer_error(
          "num_sym_exec::exec(Extract_Element): right hand side is not a "
          "variable");
    }

    if (_prec_level < MEM)
      return;

    scalar_lit_t rhs_ptr = ptr_var_aggregate(rhs);
    scalar_lit_t read_ptr = scalar_lit_t::pointer_var(
        _vfac.get_special_shadow("shadow.extract_element.ptr"));
    pointer_shift(read_ptr, rhs_ptr, offset);

    integer_t r_size = ar::getSize(ar::getType(ar::getResult(stmt)));

    try {
      const scalar_lit_t& lhs = _lfac[ar::getResult(stmt)];

      if (!lhs.is_var()) {
        throw analyzer_error(
            "num_sym_exec::exec(Extract_Element): left hand side is not a "
            "variable");
      }

      ikos::mem_domain_traits::mem_read(_inv,
                                        _vfac,
                                        lhs,
                                        read_ptr.var(),
                                        r_size);
    } catch (aggregate_literal_error&) {
      const aggregate_lit_t& lhs = _lfac.lookup_aggregate(ar::getResult(stmt));

      if (!lhs.is_var()) {
        throw analyzer_error(
            "num_sym_exec::exec(Extract_Element): left hand side is not a "
            "variable");
      }

      scalar_lit_t lhs_ptr = ptr_var_aggregate(lhs);
      ikos::mem_domain_traits::mem_copy(_inv,
                                        _vfac,
                                        lhs_ptr.var(),
                                        read_ptr.var(),
                                        int_linear_expression_t(r_size));
    }

    // clean-up
    ikos::mem_domain_traits::forget_mem_surface(_inv, read_ptr.var());
  }

  // Abstract the memory contents but not the surface
  void exec(Abstract_Memory_ref stmt) {
    // This AR statement can only be generated to indicate that a
    // global variable does not have an initializer.

    // create lazily a memory object
    init_global_var_ptr(ar::getPointer(stmt));

    abstract_memory(_lfac[ar::getPointer(stmt)], _lfac[ar::getLen(stmt)]);
  }

  /*
   * Variable arguments instructions
   */

  void exec(VA_Start_ref stmt) {
    // create lazily memory objects
    init_global_var_ptr(ar::getPointer(stmt));

    scalar_lit_t ptr = _lfac[ar::getPointer(stmt)];

    if (!prepare_mem_access(ptr))
      return;

    if (ikos::ptr_domain_traits::is_unknown_addr(_inv, ptr.var())) {
      if (_warning_callback) {
        json_dict info = {{"pointer", ptr.var()->name()}};
        std::ostringstream buf;
        buf << "ignored va_start on " << ptr.var()
            << " (analysis might be unsound)";
        _warning_callback->warning("ignored-va-start", buf.str(), stmt, info);
      }
      return;
    }

    if (_arch == "x86_64") {
      exec_va_start_x86_64(ptr);
    } else if (_arch == "i386" || _arch == "i686") {
      exec_va_start_x86_32(ptr);
    } else {
      throw analyzer_error(
          "unsupported architecture, unable to analyze call with variable "
          "arguments");
    }
  }

private:
  void mem_write_va_list_x86_64(const scalar_lit_t& ptr,
                                const scalar_lit_t& gp_offset,
                                const scalar_lit_t& fp_offset,
                                const scalar_lit_t& overflow_arg_area,
                                const scalar_lit_t& reg_save_area) {
    // pointer to write in va_list
    scalar_lit_t write_ptr = scalar_lit_t::pointer_var(
        _vfac.get_special_shadow("shadow.va_arg.ptr"));

    // write gp_offset
    assign(write_ptr, ptr);
    ikos::mem_domain_traits::mem_write(_inv,
                                       _vfac,
                                       write_ptr.var(),
                                       gp_offset,
                                       integer_t(4));

    // write fp_offset
    pointer_shift(write_ptr, ptr, scalar_lit_t::integer(4));
    ikos::mem_domain_traits::mem_write(_inv,
                                       _vfac,
                                       write_ptr.var(),
                                       fp_offset,
                                       integer_t(4));

    // write overflow_arg_area
    pointer_shift(write_ptr, ptr, scalar_lit_t::integer(8));
    ikos::mem_domain_traits::mem_write(_inv,
                                       _vfac,
                                       write_ptr.var(),
                                       overflow_arg_area,
                                       integer_t(8));

    // write reg_save_area
    pointer_shift(write_ptr, ptr, scalar_lit_t::integer(16));
    ikos::mem_domain_traits::mem_write(_inv,
                                       _vfac,
                                       write_ptr.var(),
                                       reg_save_area,
                                       integer_t(8));

    // clean-up
    ikos::mem_domain_traits::forget_mem_surface(_inv, write_ptr.var());
  }

  void exec_va_start_x86_64(const scalar_lit_t& ptr) {
    mem_write_va_list_x86_64(ptr,
                             scalar_lit_t::integer_var(_vfac.get_special_shadow(
                                 "shadow.va_arg.gp_offset")),
                             scalar_lit_t::integer_var(_vfac.get_special_shadow(
                                 "shadow.va_arg.fp_offset")),
                             scalar_lit_t::pointer_var(_vfac.get_special_shadow(
                                 "shadow.va_arg.overflow_arg_area")),
                             scalar_lit_t::pointer_var(_vfac.get_special_shadow(
                                 "shadow.va_arg.reg_save_area")));
  }

  void exec_va_start_x86_32(const scalar_lit_t& ptr) {
    ikos::mem_domain_traits::mem_write(_inv,
                                       _vfac,
                                       ptr.var(),
                                       scalar_lit_t::pointer_var(
                                           _vfac.get_special_shadow(
                                               "shadow.va_arg.args_area")),
                                       integer_t(4));
  }

public:
  void exec(VA_End_ref stmt) {
    // create lazily memory objects
    init_global_var_ptr(ar::getPointer(stmt));

    scalar_lit_t ptr = _lfac[ar::getPointer(stmt)];

    if (!prepare_mem_access(ptr))
      return;

    if (ikos::ptr_domain_traits::is_unknown_addr(_inv, ptr.var())) {
      if (_warning_callback) {
        json_dict info = {{"pointer", ptr.var()->name()}};
        std::ostringstream buf;
        buf << "ignored va_end on " << ptr.var()
            << " (analysis might be unsound)";
        _warning_callback->warning("ignored-va-end", buf.str(), stmt, info);
      }
      return;
    }

    if (_arch == "x86_64") {
      exec_va_end_x86_64(ptr);
    } else if (_arch == "i386" || _arch == "i686") {
      exec_va_end_x86_32(ptr);
    } else {
      throw analyzer_error(
          "unsupported architecture, unable to analyze call with variable "
          "arguments");
    }
  }

private:
  void exec_va_end_x86_64(const scalar_lit_t& ptr) {
    mem_write_va_list_x86_64(ptr,
                             scalar_lit_t::undefined(),
                             scalar_lit_t::undefined(),
                             scalar_lit_t::undefined(),
                             scalar_lit_t::undefined());
  }

  void exec_va_end_x86_32(const scalar_lit_t& ptr) {
    ikos::mem_domain_traits::mem_write(_inv,
                                       _vfac,
                                       ptr.var(),
                                       scalar_lit_t::undefined(),
                                       integer_t(4));
  }

public:
  void exec(VA_Arg_ref /*stmt*/) {
    // statement never generated by the frontend
    throw analyzer_error("num_sym_exec::exec(VA_Arg): unreachable");
  }

  void exec(VA_Copy_ref stmt) {
    // create lazily memory objects
    init_global_var_ptr(ar::getDestination(stmt));
    init_global_var_ptr(ar::getSource(stmt));

    // Both src and dest must be already allocated in memory so
    // offsets and sizes for both src and dest are already part of the
    // invariants
    scalar_lit_t dest = _lfac[ar::getDestination(stmt)];
    scalar_lit_t src = _lfac[ar::getSource(stmt)];

    if (!prepare_mem_access(dest) || !prepare_mem_access(src))
      return;

    if (ikos::ptr_domain_traits::is_unknown_addr(_inv, dest.var())) {
      if (_warning_callback) {
        json_dict info = {{"dest", dest.var()->name()},
                          {"src", src.var()->name()}};
        std::ostringstream buf;
        buf << "ignored va_copy from " << src.var() << " to " << dest.var()
            << " (analysis might be unsound)";
        _warning_callback->warning("ignored-va-copy", buf.str(), stmt, info);
      }
      return;
    }

    if (_arch == "x86_64") {
      ikos::mem_domain_traits::mem_copy(_inv,
                                        _vfac,
                                        dest.var(),
                                        src.var(),
                                        int_linear_expression_t(24));
    } else if (_arch == "i386" || _arch == "i686") {
      ikos::mem_domain_traits::mem_copy(_inv,
                                        _vfac,
                                        dest.var(),
                                        src.var(),
                                        int_linear_expression_t(4));
    } else {
      throw analyzer_error(
          "unsupported architecture, unable to analyze call with variable "
          "arguments");
    }
  }

  /*
   * Function calls
   */

  // Analysis of library calls (calls for which code is not
  // available). TODO: add support for more library calls.
  void exec_external_call(boost::optional< Internal_Variable_ref > lhs,
                          Call_ref call_stmt,
                          const std::string& fun_name,
                          OpRange arguments) {
    if (ikos::exc_domain_traits::is_normal_flow_bottom(_inv))
      return;

    /*
     * Here library functions that we want to consider specially.
     */
    if (fun_name == "malloc" && arguments.size() == 1) {
      analyze_malloc(lhs, call_stmt, arguments[0]);
    } else if ((fun_name == "_Znwm" || fun_name == "_Znam") &&
               arguments.size() == 1) {
      // call to new or new[]
      analyze_new(lhs, call_stmt, arguments[0]);
    } else if (fun_name == "__cxa_allocate_exception" &&
               arguments.size() == 1) {
      analyze_cxa_allocate_exception(lhs, call_stmt, arguments[0]);
    } else if (fun_name == "calloc" && arguments.size() == 2) {
      analyze_calloc(lhs, call_stmt, arguments[0], arguments[1]);
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
    } else if (fun_name == "__ikos_assert" ||
               demangle(fun_name) == "__ikos_assert(int)" ||
               demangle(fun_name) == "__ikos_assert(bool)" ||
               fun_name == "__ikos_debug" ||
               boost::starts_with(demangle(fun_name), "__ikos_debug")) {
      assert(!lhs || ar::isVoid(*lhs));
      // no side effects
    } else if (fun_name == "__ikos_unknown" ||
               demangle(fun_name) == "__ikos_unknown()") {
      // no side effects
      if (lhs) {
        scalar_lit_t ret = _lfac[*lhs];

        if (!ret.is_var()) {
          throw analyzer_error(
              "num_sym_exec::exec_external_call: left hand side is not a "
              "variable");
        }

        ikos::mem_domain_traits::forget_mem_surface(_inv, ret.var());
        ikos::uninit_domain_traits::make_initialized(_inv, ret.var());
      }
    } else {
      // default case: forget all actual parameters of pointer type
      // (very conservative)
      if (_prec_level >= MEM) {
        for (auto it = arguments.begin(); it != arguments.end(); ++it) {
          if (ar::isPointer(*it)) {
            scalar_lit_t p = _lfac[*it];
            if (p.is_pointer_var()) {
              init_global_var_ptr(*it);
              refine_addrs(p.var());
              ikos::mem_domain_traits::forget_mem_contents(_inv, p.var());
            }
          }
        }
      }

      // forget the lhs
      if (lhs) {
        try {
          scalar_lit_t ret = _lfac[*lhs];

          if (!ret.is_var()) {
            throw analyzer_error(
                "num_sym_exec::exec_external_call(): left hand side is not a "
                "variable");
          }

          ikos::mem_domain_traits::forget_mem_surface(_inv, ret.var());
        } catch (aggregate_literal_error&) {
          const aggregate_lit_t& ret = _lfac.lookup_aggregate(*lhs);

          if (!ret.is_var()) {
            throw analyzer_error(
                "num_sym_exec::exec_external_call(): left hand side is not a "
                "variable");
          }

          if (_prec_level >= MEM) {
            scalar_lit_t ret_ptr = ptr_var_aggregate(ret);
            ikos::mem_domain_traits::forget_mem_contents(_inv, ret_ptr.var());
          }
        } catch (void_var_literal_error& e) {
          ikos::mem_domain_traits::forget_mem_surface(_inv, e.var());
        }
      }

      if (fun_name.empty() || is_mangled(fun_name)) {
        // ASSUMPTION:
        // The external call to a C++ function can throw exceptions.
        throw_unknown_exceptions();
      } else {
        // ASSUMPTION:
        // The external call won't raise an exception.
      }

      // set lhs initialized
      if (lhs) {
        try {
          scalar_lit_t ret = _lfac[*lhs];

          // ASSUMPTION:
          // The claim about the correctness of the program under analysis
          // can be made only if all calls to unavailable code are assumed
          // to be correct and without side-effects.
          // We will assume that the lhs of an external call site is always
          // initialized. However, in case of a pointer, we do not assume
          // that a non-null pointer is returned.
          ikos::uninit_domain_traits::make_initialized(_inv, ret.var());
        } catch (aggregate_literal_error&) {
        } catch (void_var_literal_error&) {
        }
      }
    }
  }

  void exec_unknown_call(boost::optional< Internal_Variable_ref > lhs,
                         OpRange arguments) {
    if (ikos::exc_domain_traits::is_normal_flow_bottom(_inv))
      return;

    if (_prec_level >= MEM) {
      // forget all memory contents
      ikos::mem_domain_traits::forget_mem_contents(_inv);
    }

    // forget the lhs
    if (lhs) {
      try {
        scalar_lit_t ret = _lfac[*lhs];

        if (!ret.is_var()) {
          throw analyzer_error(
              "num_sym_exec::exec_unknown_call: left hand side is not a "
              "variable");
        }

        ikos::mem_domain_traits::forget_mem_surface(_inv, ret.var());
      } catch (aggregate_literal_error&) {
        const aggregate_lit_t& ret = _lfac.lookup_aggregate(*lhs);

        if (!ret.is_var()) {
          throw analyzer_error(
              "num_sym_exec::exec_unknown_call: left hand side is not a "
              "variable");
        }

        // nothing to do, because we already forgot all memory contents
      } catch (void_var_literal_error& e) {
        ikos::mem_domain_traits::forget_mem_surface(_inv, e.var());
      }
    }

    // might throw exceptions
    throw_unknown_exceptions();
  }

  void exec(Return_Value_ref stmt) {
    // mark the result variable as alive
    boost::optional< Operand_ref > ret_op = ar::getReturnValue(stmt);
    if (ret_op) {
      try {
        const scalar_lit_t& ret = _lfac[*ret_op];

        if (ret.is_var()) {
          keep_alive(ret.var());
        }
      } catch (aggregate_literal_error&) {
        const aggregate_lit_t& ret = _lfac.lookup_aggregate(*ret_op);

        if (ret.is_var()) {
          keep_alive(ret.var());
        }
      } catch (void_var_literal_error&) {
      }
    }
  }

private:
  /*
   * Analysis of external (library) calls
   *
   * Note: This code should go to a different file if it keeps growing.
   */

  void analyze_dynamic_alloc(boost::optional< Internal_Variable_ref > lhs_op,
                             Call_ref call_stmt,
                             Operand_ref size_op,
                             bool may_return_null,
                             bool may_throw_exc) {
    if (_prec_level < PTR)
      return;

    if (!lhs_op)
      return;

    scalar_lit_t lhs = _lfac[*lhs_op];
    scalar_lit_t size = _lfac[size_op];

    if (!lhs.is_pointer_var()) {
      throw analyzer_error(
          "num_sym_exec::analyze_dynamic_alloc(): left hand side is not a "
          "pointer variable");
    }

    ikos::mem_domain_traits::forget_mem_surface(_inv, lhs.var());

    if (may_throw_exc) {
      throw_unknown_exceptions();
    }

    nullity_value null_val =
        may_return_null ? nullity_value::may_null : nullity_value::non_null;
    uninit_value uninit_val = uninit_value::initialized;

    memloc_t dyn_memloc = _mfac.get_dyn_alloc(call_stmt);

    if (!lhs.is_pointer_var()) {
      throw analyzer_error(
          "num_sym_exec::analyze_dynamic_alloc(): Left hand side is not a "
          "pointer");
    }

    if (size.is_integer_var()) {
      allocate_memory(lhs.var(), dyn_memloc, null_val, uninit_val, size.var());
    } else if (size.is_integer()) {
      allocate_memory(lhs.var(),
                      dyn_memloc,
                      null_val,
                      uninit_val,
                      size.integer());
    } else {
      throw analyzer_error(
          "num_sym_exec::analyze_dynamic_alloc(): unexpected size parameter");
    }
  }

  /*
    #include <stdlib.h>
    void* malloc(size_t size)

    This function returns a pointer to a newly allocated block size
    bytes long, or a null pointer if the block could not be allocated.
   */
  void analyze_malloc(boost::optional< Internal_Variable_ref > lhs_op,
                      Call_ref call_stmt,
                      Operand_ref size_op) {
    analyze_dynamic_alloc(lhs_op, call_stmt, size_op, true, false);
  }

  /*
    operator new(unsigned long)
    operator new[](unsigned long)

    Allocates requested number of bytes. These allocation functions are called
    by new-expressions to allocate memory in which new object would then be
    initialized. They may also be called using regular function call syntax.
   */
  void analyze_new(boost::optional< Internal_Variable_ref > lhs_op,
                   Call_ref call_stmt,
                   Operand_ref size_op) {
    analyze_dynamic_alloc(lhs_op, call_stmt, size_op, false, true);
  }

  /*
    void* __cxa_allocate_exception(size_t thrown_size) throw();

    Allocates memory to hold the exception to be thrown. thrown_size is the size
    of the exception object. Can allocate additional memory to hold private
    data. If memory can not be allocated, call std::terminate().
   */
  void analyze_cxa_allocate_exception(
      boost::optional< Internal_Variable_ref > lhs_op,
      Call_ref call_stmt,
      Operand_ref size_op) {
    analyze_dynamic_alloc(lhs_op, call_stmt, size_op, false, false);
  }

  /*
    #include <stdlib.h>
    void* calloc(size_t count, size_t size)

    The calloc() function contiguously allocates enough space for count objects
    that are size bytes of memory each and returns a pointer to the allocated
    memory. The allocated memory is filled with bytes of value zero.
   */
  void analyze_calloc(boost::optional< Internal_Variable_ref > lhs_op,
                      Call_ref call_stmt,
                      Operand_ref count_op,
                      Operand_ref size_op) {
    if (_prec_level < PTR)
      return;

    if (!lhs_op)
      return;

    scalar_lit_t lhs = _lfac[*lhs_op];
    scalar_lit_t count = _lfac[count_op];
    scalar_lit_t size = _lfac[size_op];

    if (!lhs.is_pointer_var()) {
      throw analyzer_error(
          "num_sym_exec::analyze_calloc(): left hand side is not a pointer "
          "variable");
    }

    memloc_t dyn_memloc = _mfac.get_dyn_alloc(call_stmt);

    if (!lhs.is_pointer_var()) {
      throw analyzer_error(
          "num_sym_exec::analyze_calloc(): Left hand side is not a pointer");
    }

    allocate_memory(lhs.var(),
                    dyn_memloc,
                    nullity_value::may_null,
                    uninit_value::initialized);

    // TODO: Check for integer overflow
    if (count.is_integer_var()) {
      if (size.is_integer_var()) {
        _inv.apply(ikos::OP_MULTIPLICATION,
                   alloc_size_var(_vfac, dyn_memloc),
                   count.var(),
                   size.var());
      } else if (size.is_integer()) {
        _inv.apply(ikos::OP_MULTIPLICATION,
                   alloc_size_var(_vfac, dyn_memloc),
                   count.var(),
                   size.integer());
      } else {
        throw analyzer_error(
            "num_sym_exec::analyze_calloc(): unexpected size parameter");
      }
    } else if (count.is_integer()) {
      if (size.is_integer_var()) {
        _inv.apply(ikos::OP_MULTIPLICATION,
                   alloc_size_var(_vfac, dyn_memloc),
                   size.var(),
                   count.integer());
      } else if (size.is_integer()) {
        _inv.assign(alloc_size_var(_vfac, dyn_memloc),
                    count.integer() * size.integer());
      } else {
        throw analyzer_error(
            "num_sym_exec::analyze_calloc(): unexpected size parameter");
      }
    } else {
      throw analyzer_error(
          "num_sym_exec::analyze_calloc(): unexpected count parameter");
    }
  }

  /*
    #include <stdlib.h>
    void free(void* ptr)

    This function deallocates the memory allocated via a previous call to
    malloc().
   */
  void analyze_free(Operand_ref op) {
    scalar_lit_t ptr = _lfac[op];

    if (ptr.is_null()) // this is safe, according to C/C++ standards
      return;

    if (!ptr.is_pointer_var()) {
      throw analyzer_error(
          "num_sym_exec::analyze_free(): unexpected parameter");
    }

    if (_prec_level < PTR)
      return;

    if (ikos::null_domain_traits::is_null(_inv, ptr.var())) // this is safe
      return;

    // reduction between value and pointer analysis
    refine_addrs(ptr.var());

    // set the size to 0
    ikos::discrete_domain< memory_location_t > points_to =
        ikos::ptr_domain_traits::addrs_set< AbsValueDomain,
                                            memory_location_t >(_inv,
                                                                ptr.var());

    if (points_to.is_top())
      return;

    for (auto it = points_to.begin(); it != points_to.end(); ++it) {
      if (points_to.size() == 1) {
        _inv.assign(alloc_size_var(_vfac, *it), 0);
      } else {
        _inv -= alloc_size_var(_vfac, *it);
      }
    }

    if (_prec_level < MEM)
      return;

    // forget memory contents
    ikos::mem_domain_traits::forget_mem_contents(_inv, ptr.var());
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
  void analyze_read(boost::optional< Internal_Variable_ref > lhs_op,
                    Operand_ref /*handle_op*/,
                    Operand_ref buf_op,
                    Operand_ref len_op) {
    if (lhs_op) {
      scalar_lit_t lhs = _lfac[*lhs_op];

      if (!lhs.is_integer_var()) {
        throw analyzer_error(
            "num_sym_exec::analyze_read(): left hand side is not an integer "
            "variable");
      }

      _inv -= lhs.var();
      ikos::uninit_domain_traits::make_initialized(_inv, lhs.var());
    }

    if (_prec_level < MEM)
      return;

    // create lazily a memory object
    init_global_var_ptr(buf_op);

    abstract_memory(_lfac[buf_op], _lfac[len_op]);
  }

  /*
    __cxa_throw(void* exception, std::type_info* tinfo, void (*dest)(void*))

    After constructing the exception object with the throw argument value,
    the generated code calls the __cxa_throw runtime library routine. This
    routine never returns.
  */
  void analyze_cxa_throw(Operand_ref exception_op,
                         Operand_ref tinfo_op,
                         Operand_ref dest_op) {
    scalar_lit_t exception = _lfac[exception_op];
    scalar_lit_t tinfo = _lfac[tinfo_op];
    scalar_lit_t dest = _lfac[dest_op];

    if (!exception.is_pointer_var()) {
      throw analyzer_error(
          "num_sym_exec::analyze_cxa_throw(): unexpected exception parameter");
    } else if (!tinfo.is_pointer_var()) {
      throw analyzer_error(
          "num_sym_exec::analyze_cxa_throw(): unexpected tinfo parameter");
    }

    if (dest.is_null()) {
      ikos::exc_domain_traits::throw_exception(_inv,
                                               exception.var(),
                                               tinfo.var(),
                                               boost::optional<
                                                   variable_name_t >());
    } else if (dest.is_pointer_var()) {
      ikos::exc_domain_traits::throw_exception(_inv,
                                               exception.var(),
                                               tinfo.var(),
                                               boost::optional<
                                                   variable_name_t >(
                                                   dest.var()));
    } else {
      throw analyzer_error(
          "num_sym_exec::analyze_cxa_throw(): unexpected destructor parameter");
    }
  }

  /*
    #include <string.h>
    size_t strlen(const char* s);

    The strlen() function computes the length of the string s.

    The strlen() function returns the number of characters that precede
    the terminating NUL character.
  */
  void analyze_strlen(boost::optional< Internal_Variable_ref > lhs_op,
                      Operand_ref str_op) {
    // create lazily a memory object
    init_global_var_ptr(str_op);

    scalar_lit_t str = _lfac[str_op];

    if (!prepare_mem_access(str))
      return;

    if (!lhs_op)
      return;

    scalar_lit_t lhs = _lfac[*lhs_op];

    if (!lhs.is_integer_var()) {
      throw analyzer_error(
          "num_sym_exec::analyze_strlen(): left hand side is not an integer "
          "variable");
    }

    // lhs is in [0, size - 1]
    _inv -= lhs.var();
    _inv += (int_variable_t(lhs.var()) >= 0);
    ikos::uninit_domain_traits::make_initialized(_inv, lhs.var());

    if (ikos::exc_domain_traits::is_normal_flow_bottom(_inv))
      return;

    ikos::discrete_domain< memory_location_t > points_to =
        ikos::ptr_domain_traits::addrs_set< AbsValueDomain,
                                            memory_location_t >(_inv,
                                                                str.var());

    if (points_to.is_top())
      return;

    AbsValueDomain inv = AbsValueDomain::bottom();
    for (auto it = points_to.begin(); it != points_to.end(); ++it) {
      AbsValueDomain tmp = _inv;
      tmp += (int_variable_t(lhs.var()) <=
              int_variable_t(alloc_size_var(_vfac, *it)) - 1);
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
  void analyze_strnlen(boost::optional< Internal_Variable_ref > lhs_op,
                       Operand_ref str_op,
                       Operand_ref maxlen_op) {
    analyze_strlen(lhs_op, str_op);

    if (ikos::exc_domain_traits::is_normal_flow_bottom(_inv))
      return;

    if (!lhs_op)
      return;

    // lhs <= maxlen
    scalar_lit_t lhs = _lfac[*lhs_op];
    scalar_lit_t maxlen = _lfac[maxlen_op];
    if (maxlen.is_integer_var()) {
      _inv += int_variable_t(lhs.var()) <= int_variable_t(maxlen.var());
    } else if (maxlen.is_integer()) {
      _inv += int_variable_t(lhs.var()) <= maxlen.integer();
    } else {
      throw analyzer_error(
          "num_sym_exec::analyze_strnlen(): unexpected maxlen parameter");
    }
  }

  /*
    #include <string.h>
    char* strcpy(char* dst, const char* src);

    The strcpy() function copies the string src to dst (including
    the terminating `\0' character).

    The strcpy() function returns dst.
  */
  void analyze_strcpy(boost::optional< Internal_Variable_ref > lhs_op,
                      Operand_ref dest_op,
                      Operand_ref src_op) {
    // create lazily memory objects
    init_global_var_ptr(dest_op);
    init_global_var_ptr(src_op);

    scalar_lit_t dest = _lfac[dest_op];
    scalar_lit_t src = _lfac[src_op];

    if (!prepare_mem_access(dest) || !prepare_mem_access(src))
      return;

    // Do not keep track of the content
    ikos::mem_domain_traits::forget_mem_contents(_inv, dest.var());

    if (lhs_op) {
      assign(_lfac[*lhs_op], dest);
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
  void analyze_strncpy(boost::optional< Internal_Variable_ref > lhs_op,
                       Operand_ref dest_op,
                       Operand_ref src_op,
                       Operand_ref /*size_op*/) {
    analyze_strcpy(lhs_op, dest_op, src_op);
  }

  /*
    #include <string.h>
    char* strcat(char* s1, const char* s2);

    The strcat() function appends a copy of the null-terminated string s2
    to the end of the null-terminated string s1, then add a terminating \0.
    The string s1 must have sufficient space to hold the result.

    The strcat() function returns the pointer s1.
  */
  void analyze_strcat(boost::optional< Internal_Variable_ref > lhs_op,
                      Operand_ref s1_op,
                      Operand_ref s2_op) {
    // create lazily memory objects
    init_global_var_ptr(s1_op);
    init_global_var_ptr(s2_op);

    scalar_lit_t s1 = _lfac[s1_op];
    scalar_lit_t s2 = _lfac[s2_op];

    if (!prepare_mem_access(s1) || !prepare_mem_access(s2))
      return;

    // Do not keep track of the content
    ikos::mem_domain_traits::forget_mem_contents(_inv, s1.var());

    if (lhs_op) {
      assign(_lfac[*lhs_op], s1);
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
  void analyze_strncat(boost::optional< Internal_Variable_ref > lhs_op,
                       Operand_ref s1_op,
                       Operand_ref s2_op,
                       Operand_ref n_op) {
    analyze_strcat(lhs_op, s1_op, s2_op);
  }

}; // end class num_sym_exec

} // end namespace analyzer

#endif // ANALYZER_NUM_SYM_EXEC_HPP
