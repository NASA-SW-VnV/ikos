/******************************************************************************
 *
 * Pointer analysis.
 *
 * This pass is intended to be used as a pre-step for other analyses.
 * It computes first intra-procedurally numerical invariants for each
 * function. Then, it generates nonuniform points-to constraints in a
 * flow-insensitive manner, solves them, and finally stores the
 * results for subsequent analyses.
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *               Clement Decoodt
 *
 * Contact: ikos@lists.nasa.gov
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

#ifndef ANALYZER_POINTER_HPP
#define ANALYZER_POINTER_HPP

#include <sstream>

#include <ikos/domains/exception.hpp>
#include <ikos/domains/intervals.hpp>
#include <ikos/domains/pta.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/analysis/num_sym_exec.hpp>
#include <analyzer/ar-wrapper/cfg.hpp>
#include <analyzer/cast.hpp>
#include <analyzer/ikos-wrapper/iterators.hpp>

//#define DEBUG

namespace analyzer {

class PointerPass {
  typedef varname_t VariableName;
  typedef memloc_t MemoryLocation;
  typedef ikos::z_number Number;
  typedef ikos::interval< Number > interval_t;

  // Generate numerical invariants for a function
  template < typename AbsNumDomain >
  class NumInvGen : public fwd_fixpoint_iterator< Basic_Block_ref,
                                                  arbos_cfg,
                                                  AbsNumDomain > {
    class Post : public arbos_visitor_api {
      typedef num_sym_exec< AbsNumDomain, Number, ikos::dummy_number >
          sym_exec_t;

      sym_exec_t _post; // transfer function
      VariableFactory& _vfac;

    public:
      Post(AbsNumDomain pre,
           VariableFactory& vfac,
           memory_factory& mfac,
           LiteralFactory& lfac,
           const std::string& arch,
           varname_set_t dead_vars = varname_set_t(),
           varname_set_t loop_counters = varname_set_t())
          : _post(pre,
                  vfac,
                  mfac,
                  lfac,
                  arch,
                  REG /*do not change*/,
                  PointerInfo(),
                  dead_vars,
                  loop_counters),
            _vfac(vfac) {}

      AbsNumDomain post() { return _post.inv(); }

      void visit_start(Basic_Block_ref b) { _post.exec_start(b); }
      void visit_end(Basic_Block_ref b) { _post.exec_end(b); }
      void visit(Arith_Op_ref s) { _post.exec(s); }
      void visit(Integer_Comparison_ref s) { _post.exec(s); }
      void visit(FP_Comparison_ref s) { _post.exec(s); }
      void visit(Bitwise_Op_ref s) { _post.exec(s); }
      void visit(FP_Op_ref s) { _post.exec(s); }
      void visit(Abstract_Variable_ref s) { _post.exec(s); }
      void visit(Unreachable_ref s) { _post.exec(s); }
      void visit(Assignment_ref s) { _post.exec(s); }
      void visit(Conv_Op_ref s) { _post.exec(s); }
      void visit(Pointer_Shift_ref s) { _post.exec(s); }
      void visit(Allocate_ref s) { _post.exec(s); }
      void visit(Store_ref s) { _post.exec(s); }
      void visit(Load_ref s) { _post.exec(s); }
      void visit(Insert_Element_ref s) { _post.exec(s); }
      void visit(Extract_Element_ref s) { _post.exec(s); }
      void visit(MemCpy_ref s) { _post.exec(s); }
      void visit(MemMove_ref s) { _post.exec(s); }
      void visit(MemSet_ref s) { _post.exec(s); }
      void visit(Landing_Pad_ref s) { _post.exec(s); }
      void visit(Resume_ref s) { _post.exec(s); }
      void visit(Abstract_Memory_ref s) { _post.exec(s); }
      void visit(Call_ref s) {
        if (ar::isDirectCall(s) && ar::isExternal(s)) {
          _post.exec_external_call(ar::getReturnValue(s),
                                   s,
                                   ar::getFunctionName(s),
                                   ar::getArguments(s));
        } else {
          _post.exec_unknown_call(ar::getReturnValue(s), ar::getArguments(s));
        }
      }
      void visit(Invoke_ref s) { visit(ar::getFunctionCall(s)); }
      void visit(Return_Value_ref s) { _post.exec(s); }
      void visit(VA_Start_ref s) { _post.exec(s); }
      void visit(VA_End_ref s) { _post.exec(s); }
      void visit(VA_Arg_ref s) { _post.exec(s); }
      void visit(VA_Copy_ref s) { _post.exec(s); }
    };

    typedef fwd_fixpoint_iterator< Basic_Block_ref, arbos_cfg, AbsNumDomain >
        fwd_fixpoint_iterator_t;
    typedef std::unordered_map< Basic_Block_ref, AbsNumDomain > inv_table_t;

    VariableFactory& _vfac;
    memory_factory& _mfac;
    LiteralFactory& _lfac;
    std::string _arch;
    LivenessPass& _live;
    inv_table_t _inv_table;

  public:
    NumInvGen(arbos_cfg cfg,
              VariableFactory& vfac,
              memory_factory& mfac,
              LiteralFactory& lfac,
              LivenessPass& live)
        : fwd_fixpoint_iterator_t(cfg, true),
          _vfac(vfac),
          _mfac(mfac),
          _lfac(lfac),
          _arch(cfg.arch()),
          _live(live) {}

    // To propagate invariants at basic block level
    AbsNumDomain analyze(Basic_Block_ref bb, AbsNumDomain pre) {
      std::string f = this->get_cfg().func_name();
      arbos_cfg::arbos_node_t node = this->get_cfg().get_node(bb);

      std::shared_ptr< Post > vis(new Post(pre,
                                           _vfac,
                                           _mfac,
                                           _lfac,
                                           _arch,
                                           _live.deadSet(f, bb),
                                           node.loop_counters()));
      node.accept(vis);
      return vis->post();
    }

    void check_pre(Basic_Block_ref bb, AbsNumDomain pre) {
      // we need to call explicitly visit_start()
      std::shared_ptr< Post > vis(new Post(pre, _vfac, _mfac, _lfac, _arch));
      vis->visit_start(bb);
      _inv_table.insert(typename inv_table_t::value_type(bb, vis->post()));
    }

    void check_post(Basic_Block_ref bb, AbsNumDomain post) {}

  public:
    // To propagate invariants at the statement level
    template < typename Statement >
    AbsNumDomain analyze_stmt(Statement stmt, AbsNumDomain pre) {
      std::shared_ptr< Post > vis(new Post(pre, _vfac, _mfac, _lfac, _arch));
      vis->visit(stmt);
      return vis->post();
    }

    // Get invariants that hold at the entry of a basic block
    AbsNumDomain operator[](Basic_Block_ref bb) {
      typename inv_table_t::iterator it = _inv_table.find(bb);
      if (it != _inv_table.end()) {
        return it->second;
      } else {
        return AbsNumDomain::top();
      }
    }
  };

public:
  // A visitor to generate points-to constraints from a function
  template < typename AbsNumDomain, typename NumInvGen >
  class PTA : public arbos_visitor_api {
    // The points-to sets computed here are an overapproximation of
    // all memory objects to which a pointer variable can point-to
    // during all program executions. In the way the constraints are
    // generated these points-to sets cannot be used to tell whether
    // a pointer can be null or not.

  public:
    typedef std::unordered_map< VariableName, ikos::pointer_var > pt_var_map_t;
    typedef std::unordered_map< ikos::index64_t, MemoryLocation > address_map_t;
    typedef ikos::discrete_domain< MemoryLocation > ptr_set_t;

  private:
    typedef std::shared_ptr< NumInvGen > num_inv_gen_t;

  private:
    Bundle_ref _bundle;
    Function_ref _func;
    ikos::pta_system& _cs;
    num_inv_gen_t _inv_gen;
    LiteralFactory& _lfac;
    VariableFactory& _vfac;
    memory_factory& _mfac;
    std::string _arch;
    pt_var_map_t& _pt_var_map;
    address_map_t& _address_map;
    PointerInfo _fun_ptr_info;
    AbsNumDomain _block_inv;

  private:
    // Create a pointer variable
    ikos::pointer_var new_pointer_var(VariableName v) {
      pt_var_map_t::iterator it = _pt_var_map.find(v);
      if (it != _pt_var_map.end())
        return it->second;

      ikos::pointer_var pt =
          ikos::mk_pointer_var(ikos::index_traits< VariableName >::index(v));
      _pt_var_map.insert(pt_var_map_t::value_type(v, pt));
      return pt;
    }

    // Create a new object
    // An object is a sequence of bytes in memory that can be
    // allocated on the stack, code space (functions), or in the
    // heap. Global variables of integer or fp type and strings are
    // also objects.
    void new_object_ref(VariableName v,
                        MemoryLocation ml,
                        bool is_function = false) {
      address_map_t::iterator it =
          _address_map.find(ikos::index_traits< MemoryLocation >::index(ml));
      if (it != _address_map.end())
        return;
      _address_map.insert(
          address_map_t::value_type(ikos::index_traits< MemoryLocation >::index(
                                        ml),
                                    ml));

      if (is_function) {
        std::shared_ptr< ikos::function_ref > ref = ikos::mk_function_ref(
            ikos::index_traits< MemoryLocation >::index(ml));
#ifdef DEBUG
        std::cerr << "\t" << *(new_pointer_var(v) == ref) << std::endl;
#endif
        _cs += new_pointer_var(v) == ref;
      } else {
        std::shared_ptr< ikos::object_ref > ref =
            ikos::mk_object_ref(ikos::index_traits< MemoryLocation >::index(ml),
                                zero());
#ifdef DEBUG
        std::cerr << "\t" << *(new_pointer_var(v) == ref) << std::endl;
#endif
        _cs += new_pointer_var(v) == ref;
      }
    }

    // Helper function for creating a memory location from an AR_Operand and a
    // VariableName and create a new object.
    void new_object_ref_helper(Operand_ref op, VariableName ptr) {
      if (ar::isGlobalVar(op)) {
        auto gv = ar::getGlobalVariable(op);
        ikos_assert(gv);
        new_object_ref(ptr, _mfac.get_global(*gv));
      } else if (ar::isAllocaVar(op)) {
        Cst_Operand_ref cst_o = node_cast< Cst_Operand >(op);
        Constant_ref cst = cst_o->getConstant();
        Var_Addr_Constant_ref var_addr = node_cast< Var_Addr_Constant >(cst);
        Variable_ref var = var_addr->getVariable();
        auto lv = node_cast< Local_Variable >(var);

        new_object_ref(ptr, _mfac.get_local(lv));
      } else if (ar::isFunctionPointer(op)) {
        Cst_Operand_ref cst_o = node_cast< Cst_Operand >(op);
        Constant_ref cst = cst_o->getConstant();
        auto fv = node_cast< Function_Addr_Constant >(cst);

        new_object_ref(ptr, _mfac.get_function(fv), true /*function*/);
      }
    }

    // Create a new parameter
    ikos::pointer_var new_param_ref(VariableName fname, unsigned param) {
      std::shared_ptr< ikos::param_ref > par_ref =
          ikos::mk_param_ref(new_pointer_var(fname), param);
      ikos::pointer_var p = new_pointer_var(_vfac.get_param(*par_ref));
      return p;
    }

    // Create a new return
    ikos::pointer_var new_return_ref(VariableName fname) {
      std::shared_ptr< ikos::return_ref > ret_ref =
          ikos::mk_return_ref(new_pointer_var(fname));
      ikos::pointer_var p = new_pointer_var(_vfac.get_ret(*ret_ref));
      return p;
    }

    // Return the interval [0, 0]
    interval_t zero() const { return interval_t(0, 0); }

    // Return the interval [0, sz-1]
    interval_t get_size_interval(Number sz) const {
      return interval_t(0, sz - 1);
    }

    void assign(const scalar_lit_t& lhs, Operand_ref rhs_op) {
      if (!lhs.is_pointer_var())
        return;

      scalar_lit_t rhs = _lfac[rhs_op];

      if (rhs.is_integer()) {
        std::ostringstream buf;
        buf << "pta::assign(): trying to assign an integer to pointer variable "
            << lhs.var();
        throw analyzer_error(buf.str());
      } else if (rhs.is_floating_point()) {
        std::ostringstream buf;
        buf << "pta::assign(): trying to assign a float to pointer variable "
            << lhs.var();
        throw analyzer_error(buf.str());
      } else if (rhs.is_null() || rhs.is_undefined()) {
        return;
      } else if (rhs.is_integer_var()) {
        std::ostringstream buf;
        buf << "pta::assign(): trying to assign integer variable " << rhs.var()
            << " to pointer variable " << lhs.var();
        throw analyzer_error(buf.str());
      } else if (rhs.is_floating_point_var()) {
        std::ostringstream buf;
        buf << "pta::assign(): trying to assign float variable " << rhs.var()
            << " to pointer variable " << lhs.var();
        throw analyzer_error(buf.str());
      } else if (rhs.is_pointer_var()) {
        new_object_ref_helper(rhs_op, rhs.var());

        ikos::pointer_var p1 = new_pointer_var(lhs.var());
        ikos::pointer_var p2 = new_pointer_var(rhs.var());

#ifdef DEBUG
        std::cerr << "\t" << *(p1 == (p2 + zero())) << std::endl;
#endif
        _cs += p1 == (p2 + zero());
      } else {
        throw analyzer_error("unreachable");
      }
    }

  public:
    PTA(Bundle_ref bundle,
        Function_ref func,
        // pointer constraint system
        ikos::pta_system& cs,
        // numerical invariants for the function
        num_inv_gen_t inv_gen,
        LiteralFactory& lfac,  // literal factory
        VariableFactory& vfac, // variable factory
        memory_factory& mfac,  // memory factory
        // mapping variable names to pointers
        pt_var_map_t& pt_var_map,
        // mapping id's to memory locations
        address_map_t& address_map,
        PointerInfo fun_ptr_info = PointerInfo())
        : _bundle(bundle),
          _func(func),
          _cs(cs),
          _inv_gen(inv_gen),
          _lfac(lfac),
          _vfac(vfac),
          _mfac(mfac),
          _arch(ar::getTargetArch(bundle)),
          _pt_var_map(pt_var_map),
          _address_map(address_map),
          _fun_ptr_info(fun_ptr_info),
          _block_inv(AbsNumDomain::top()) {}

    void visit_start(Function_ref f) {
      IvRange fparams = ar::getFormalParams(f);
      unsigned int i = 0;
      VariableName fname = _vfac.get_function_addr(f);

#ifdef DEBUG
      std::cerr << "PTA: formal parameters of ";
      ikos::index_traits< VariableName >::write(std::cerr, fname);
      std::cerr << std::endl;
#endif
      for (IvRange::iterator it = fparams.begin(); it != fparams.end();
           ++it, i++) {
        if (ar::isPointer(*it)) {
          scalar_lit_t p = _lfac[*it];
          if (p.is_pointer_var()) {
            ikos::pointer_var fp = new_pointer_var(p.var());
#ifdef DEBUG
            std::cerr << "\t" << *(fp == (new_param_ref(fname, i) + zero()))
                      << std::endl;
#endif
            _cs += (fp == (new_param_ref(fname, i) + zero()));
          }
        }
      }
    }

    void visit_end(Function_ref f) {}

    void visit_start(Basic_Block_ref b) {
      _block_inv = _inv_gen->operator[](b);
    }

    void visit_end(Basic_Block_ref b) {}

    void visit(Arith_Op_ref s) {
      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Integer_Comparison_ref s) {
      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(FP_Comparison_ref s) {
      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(FP_Op_ref s) {
      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Bitwise_Op_ref s) {
      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Allocate_ref s) {
      scalar_lit_t lhs = _lfac[ar::getResult(s)];
      assert(lhs.is_pointer_var());

      auto lv = ar::getResult(s)->getVariable();
      new_object_ref(lhs.var(),
                     _mfac.get_local(node_cast< Local_Variable >(lv)));

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Abstract_Variable_ref s) {
      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Landing_Pad_ref s) {
      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Resume_ref s) {
      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Unreachable_ref s) {
      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Assignment_ref s) {
      Internal_Variable_ref lhs_op = ar::getLeftOp(s);

      if (!ar::isPointer(lhs_op)) {
        // propagate invariant to the next statement
        _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
        return;
      }

#ifdef DEBUG
      std::cerr << "PTA: " << s << std::endl;
#endif
      assign(_lfac[lhs_op], ar::getRightOp(s));

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Conv_Op_ref s) {
      Var_Operand_ref dest = node_cast< Var_Operand >(ar::getLeftOp(s));
      Operand_ref src = ar::getRightOp(s);

      switch (ar::getConvOp(s)) {
        case arbos::inttoptr:
          // cast from int to ptr (for instance: int x = 5; int *px = x;)
          // TODO
          break;
        case arbos::bitcast: {
          // cast from X* to Y*

          scalar_lit_t dest = _lfac[ar::getLeftOp(s)];
          scalar_lit_t src = _lfac[ar::getRightOp(s)];

          if (dest.is_pointer_var()) {
            if (src.is_null() || src.is_undefined() || src.is_pointer_var()) {
#ifdef DEBUG
              std::cerr << "PTA: " << s << std::endl;
#endif
              assign(dest, ar::getRightOp(s));
            } else {
              throw analyzer_error(
                  "pta::visit(ConvOp, bitcast): unexpected right hand side");
            }
          }
          break;
        }
        default:;
      }

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Pointer_Shift_ref s) {
      scalar_lit_t lhs = _lfac[ar::getResult(s)];
      scalar_lit_t base = _lfac[ar::getBase(s)];
      scalar_lit_t offset = _lfac[ar::getOffset(s)];

      if (lhs.is_pointer_var() && base.is_pointer_var()) {
        new_object_ref_helper(ar::getBase(s), base.var());

#ifdef DEBUG
        std::cerr << "PTA: " << s << std::endl;
#endif
        ikos::pointer_var p1 = new_pointer_var(lhs.var());
        ikos::pointer_var p2 = new_pointer_var(base.var());

        interval_t o = interval_t::top();
        if (offset.is_integer()) {
          o = interval_t(offset.integer(), offset.integer());
        } else if (offset.is_integer_var()) {
          o = ikos::num_domain_traits::to_interval(_block_inv, offset.var());
        } else {
          throw analyzer_error(
              "pta::visit(PointerShift): unexpected offset operand");
        }

#ifdef DEBUG
        std::cerr << "\t" << *(p1 == (p2 + o)) << std::endl;
#endif
        _cs += p1 == (p2 + o);
      }

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Store_ref s) {
      try {
        scalar_lit_t ptr = _lfac[ar::getPointer(s)];
        scalar_lit_t val = _lfac[ar::getValue(s)];

        if (!ar::isPointer(ar::getPointer(s)) || !ptr.is_pointer_var() ||
            !val.is_pointer_var() ||
            (!ar::isPointer(ar::getValue(s)) &&
             !ar::isFunctionPointer(ar::getValue(s)))) {
          // propagate invariant to the next statement
          _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
          return;
        }

#ifdef DEBUG
        std::cerr << "PTA: " << s << std::endl;
#endif

        new_object_ref_helper(ar::getPointer(s), ptr.var());
        new_object_ref_helper(ar::getValue(s), val.var());

        ikos::pointer_var p1 = new_pointer_var(ptr.var());
        ikos::pointer_var p2 = new_pointer_var(val.var());

        interval_t o =
            get_size_interval(ar::getSize(ar::getType(ar::getValue(s))));
#ifdef DEBUG
        std::cerr << "\t" << *(p1 + o << p2) << std::endl;
#endif
        _cs += (p1 + o) << p2;
      } catch (aggregate_literal_error&) {
        // ignored
      }

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Load_ref s) {
      try {
        scalar_lit_t ptr = _lfac[ar::getPointer(s)];
        scalar_lit_t lhs = _lfac[ar::getResult(s)];

        if (!ar::isPointer(ar::getResult(s)) ||
            !ar::isPointer(ar::getPointer(s)) || !ptr.is_pointer_var()) {
          // propagate invariant to the next statement
          _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
          return;
        }

#ifdef DEBUG
        std::cerr << "PTA: " << s << std::endl;
#endif

        new_object_ref_helper(ar::getPointer(s), ptr.var());

        ikos::pointer_var p1 = new_pointer_var(lhs.var());
        ikos::pointer_var p2 = new_pointer_var(ptr.var());

        interval_t o =
            get_size_interval(ar::getSize(ar::getType(ar::getResult(s))));
#ifdef DEBUG
        std::cerr << "\t" << *(p1 *= p2 + o) << std::endl;
#endif
        _cs += p1 *= (p2 + o);
      } catch (aggregate_literal_error&) {
        // ignored
      }

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Extract_Element_ref s) {
      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Insert_Element_ref s) {
      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

  private:
    // memcpy(dest, src, len)
    // memmove(dest, src, len)
    template < typename MemCpyOrMove_ref >
    void visitMemCpyOrMove(MemCpyOrMove_ref s) {
      scalar_lit_t dest = _lfac[ar::getTarget(s)];
      scalar_lit_t src = _lfac[ar::getSource(s)];

      if (!ar::isPointer(ar::getSource(s)) || !src.is_pointer_var() ||
          !ar::isPointer(ar::getTarget(s)) || !dest.is_pointer_var()) {
        // propagate invariant to the next statement
        _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
        return;
      }

#ifdef DEBUG
      std::cerr << "PTA: " << s << std::endl;
#endif

      new_object_ref_helper(ar::getSource(s), src.var());
      new_object_ref_helper(ar::getTarget(s), dest.var());

      ikos::pointer_var src_ptr = new_pointer_var(src.var());
      ikos::pointer_var dest_ptr = new_pointer_var(dest.var());
      ikos::pointer_var tmp_ptr = ikos::mk_pointer_var();

#ifdef DEBUG
      std::cerr << "\t" << *(tmp_ptr *= src_ptr + zero()) << std::endl;
      std::cerr << "\t" << *(dest_ptr + zero() << tmp_ptr) << std::endl;
#endif

      _cs += tmp_ptr *= (src_ptr + zero());
      _cs += (dest_ptr + zero()) << tmp_ptr;

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

  public:
    void visit(MemCpy_ref s) { visitMemCpyOrMove(s); }

    void visit(MemMove_ref s) { visitMemCpyOrMove(s); }

    void visit(MemSet_ref s) {
      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Abstract_Memory_ref s) {
      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Return_Value_ref s) {
      boost::optional< Operand_ref > lhs_op = ar::getReturnValue(s);

      if (lhs_op &&
          (ar::isPointer(*lhs_op) || ar::isFunctionPointer(*lhs_op))) {
        scalar_lit_t lhs = _lfac[*lhs_op];

        if (!lhs.is_pointer_var()) {
          // propagate invariant to the next statement
          _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
          return;
        }

        new_object_ref_helper(*lhs_op, lhs.var());

        ikos::pointer_var p = new_pointer_var(lhs.var());
        VariableName fname = _vfac.get_function_addr(_func);
#ifdef DEBUG
        std::cerr << "\t" << *(new_return_ref(fname) == (p + zero()))
                  << std::endl;
#endif
        _cs += (new_return_ref(fname) == (p + zero()));
      }

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Call_ref call_stmt) {
#ifdef DEBUG
      std::cerr << "PTA call: " << call_stmt << std::endl;
#endif

      boost::optional< Internal_Variable_ref > lhs =
          ar::getReturnValue(call_stmt);
      OpRange actual_params = ar::getArguments(call_stmt);

      /*
       * Collect potential callees
       */

      std::vector< memloc_t > callees;

      if (ar::isIndirectCall(call_stmt)) {
        VariableName ptr =
            _vfac.get_internal(ar::getIndirectCallVar(call_stmt));
        ptr_set_t ptr_set = _fun_ptr_info[ptr].first;

        if (ptr_set.is_top()) {
          // propagate invariant to the next statement
          _block_inv = _inv_gen->analyze_stmt(call_stmt, _block_inv);
          return;
        }

        for (ptr_set_t::iterator it = ptr_set.begin(); it != ptr_set.end();
             ++it) {
          callees.push_back(*it);
        }
      } else { // direct call
        callees.push_back(
            _mfac.get_function(ar::ar_internal::getFunctionAddr(call_stmt)));
      }

      /*
       * Add pointer constraints for each possible callee
       */

      for (std::vector< memloc_t >::iterator it = callees.begin();
           it != callees.end();
           ++it) {
        // Check if the callee is a function
        if (!isa< function_memory_location >(*it)) {
          continue;
        }
        auto mem_it = cast< analyzer::function_memory_location >(*it);
        const std::string& fun_name = mem_it->name();
        boost::optional< Function_ref > callee =
            ar::getFunction(_bundle, fun_name);

        if (!callee || ar::isExternal(*callee)) {
          // ASSUMPTION: if the function code is not available, treat it as an
          // external call
          if (((fun_name == "malloc" || fun_name == "_Znwm" ||
                fun_name == "_Znam" ||
                fun_name == "__cxa_allocate_exception") &&
               actual_params.size() == 1) ||
              (fun_name == "calloc" && actual_params.size() == 2)) {
            // call to malloc, new, new[], __cxa_allocate_exception or calloc
            new_object_ref(_lfac[*lhs].var(), _mfac.get_dyn_alloc(call_stmt));
          }
        } else {
          bool is_va_arg = ar::isVarargs(*callee);
          IvRange formal_params = ar::getFormalParams(*callee);

          if ((!is_va_arg && actual_params.size() != formal_params.size()) ||
              (is_va_arg && actual_params.size() < formal_params.size())) {
            // ASSUMPTION: all function calls have been checked by the compiler
            // and are well-formed. In that case, it means this function cannot
            // be called and that it is just an imprecision of the pointer
            // analysis.
            continue;
          }

          VariableName fname = _vfac.get_function_addr(*callee);
          new_object_ref(fname, _mfac.get_function(*callee), true /*function*/);

          IvRange::iterator FIt = formal_params.begin();
          OpRange::iterator AIt = actual_params.begin();
          unsigned int i = 0;
          for (; FIt != formal_params.end(); ++FIt, ++AIt, i++) {
            if ((ar::isPointer(*AIt) || ar::isFunctionPointer(*AIt)) &&
                ar::isPointer(*FIt)) {
              if (!_lfac[*AIt].is_pointer_var()) {
                // the actual parameter can be null
                continue;
              }

              ikos::pointer_var ap = new_pointer_var(_lfac[*AIt].var());
              ikos::pointer_var fp = new_param_ref(fname, i);
              new_object_ref_helper(*AIt, _lfac[*AIt].var());

#ifdef DEBUG
              std::cerr << "\t" << *(fp == (ap + zero())) << std::endl;
#endif
              _cs += fp == (ap + zero());
            }
          }

          if (lhs && ar::isPointer(*lhs)) {
            ikos::pointer_var p = new_pointer_var(_lfac[*lhs].var());
#ifdef DEBUG
            std::cerr << "\t" << *(p == (new_return_ref(fname) + zero()))
                      << std::endl;
#endif
            _cs += (p == (new_return_ref(fname) + zero()));
          }

          if (is_va_arg) {
            if (_arch == "x86_64") {
              match_down_va_arg_x86_64(formal_params, actual_params);
            } else if (_arch == "i386" || _arch == "i686") {
              match_down_va_arg_x86_32(formal_params, actual_params);
            } else {
              throw analyzer_error(
                  "unsupported architecture, unable to analyze call with "
                  "variable arguments");
            }
          }
        }
      }

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(call_stmt, _block_inv);
    }

  private:
    /*
     * Modeling of va_arg implementation for x86_64
     *
     * See https://www.uclibc.org/docs/psABI-x86_64.pdf, p50
     */
    void match_down_va_arg_x86_64(IvRange formals, OpRange actuals) {
      // create reg_save_area and overflow_arg_area as shadow memory objects
      VariableName reg_save_area =
          _vfac.get_special_shadow("shadow.va_arg.reg_save_area");
      VariableName overflow_arg_area =
          _vfac.get_special_shadow("shadow.va_arg.overflow_arg_area");
      memloc_t reg_save_area_ml =
          _mfac.get_va_arg("shadow.va_arg.reg_save_area");
      memloc_t overflow_arg_area_ml =
          _mfac.get_va_arg("shadow.va_arg.overflow_arg_area");

      new_object_ref(reg_save_area, reg_save_area_ml);
      new_object_ref(overflow_arg_area, overflow_arg_area_ml);

      ikos::pointer_var reg_save_area_ptr = new_pointer_var(reg_save_area);
      ikos::pointer_var overflow_arg_area_ptr =
          new_pointer_var(overflow_arg_area);

      // push everything on the stack
      IvRange::iterator FIt = formals.begin();
      OpRange::iterator AIt = actuals.begin();
      for (; FIt != formals.end(); ++FIt, ++AIt)
        ;
      for (; AIt != actuals.end(); ++AIt) {
        if ((ar::isPointer(*AIt) || ar::isFunctionPointer(*AIt)) &&
            _lfac[*AIt].is_pointer_var()) {
          ikos::pointer_var ap = new_pointer_var(_lfac[*AIt].var());
          new_object_ref_helper(*AIt, _lfac[*AIt].var());

          interval_t o = interval_t::top();
#ifdef DEBUG
          std::cerr << "\t" << *(reg_save_area_ptr + o << ap) << std::endl;
          std::cerr << "\t" << *(overflow_arg_area_ptr + o << ap) << std::endl;
#endif
          _cs += (reg_save_area_ptr + o) << ap;
          _cs += (overflow_arg_area_ptr + o) << ap;
        }
      }
    }

    void match_down_va_arg_x86_32(IvRange formals, OpRange actuals) {
      // create args_area to store parameters
      VariableName args_area =
          _vfac.get_special_shadow("shadow.va_arg.args_area");
      new_object_ref(args_area, _mfac.get_va_arg("shadow.va_arg.args_area"));
      ikos::pointer_var args_area_ptr = new_pointer_var(args_area);

      // push everything on the stack
      IvRange::iterator FIt = formals.begin();
      OpRange::iterator AIt = actuals.begin();
      for (; FIt != formals.end(); ++FIt, ++AIt)
        ;
      for (; AIt != actuals.end(); ++AIt) {
        if ((ar::isPointer(*AIt) || ar::isFunctionPointer(*AIt)) &&
            _lfac[*AIt].is_pointer_var()) {
          ikos::pointer_var ap = new_pointer_var(_lfac[*AIt].var());
          new_object_ref_helper(*AIt, _lfac[*AIt].var());

          interval_t o = interval_t::top();
#ifdef DEBUG
          std::cerr << "\t" << *(args_area_ptr + o << ap) << std::endl;
#endif
          _cs += (args_area_ptr + o) << ap;
        }
      }
    }

  public:
    void visit(Invoke_ref s) { visit(ar::getFunctionCall(s)); }

    void visit(VA_Start_ref s) {
      scalar_lit_t ptr = _lfac[ar::getPointer(s)];

      if (!ar::isPointer(ar::getPointer(s)) || !ptr.is_pointer_var()) {
        // propagate invariant to the next statement
        _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
        return;
      }

#ifdef DEBUG
      std::cerr << "PTA: " << s << std::endl;
#endif

      new_object_ref_helper(ar::getPointer(s), ptr.var());

      if (_arch == "x86_64") {
        visit_va_start_x86_64(ptr);
      } else if (_arch == "i386" || _arch == "i686") {
        visit_va_start_x86_32(ptr);
      } else {
        throw analyzer_error(
            "unsupported architecture, unable to analyze call with variable "
            "arguments");
      }

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

  private:
    void visit_va_start_x86_64(const scalar_lit_t& ptr) {
      ikos::pointer_var p = new_pointer_var(ptr.var());
      ikos::pointer_var reg_save_area_ptr = new_pointer_var(
          _vfac.get_special_shadow("shadow.va_arg.reg_save_area"));
      ikos::pointer_var overflow_arg_area_ptr = new_pointer_var(
          _vfac.get_special_shadow("shadow.va_arg.overflow_arg_area"));

      interval_t o = interval_t::top();
#ifdef DEBUG
      std::cerr << "\t" << *(p + o << reg_save_area_ptr) << std::endl;
      std::cerr << "\t" << *(p + o << overflow_arg_area_ptr) << std::endl;
#endif
      _cs += (p + o) << reg_save_area_ptr;
      _cs += (p + o) << overflow_arg_area_ptr;
    }

    void visit_va_start_x86_32(const scalar_lit_t& ptr) {
      ikos::pointer_var p = new_pointer_var(ptr.var());
      ikos::pointer_var args_area_ptr =
          new_pointer_var(_vfac.get_special_shadow("shadow.va_arg.args_area"));

      interval_t o = interval_t::top();
#ifdef DEBUG
      std::cerr << "\t" << *(p + o << args_area_ptr) << std::endl;
#endif
      _cs += (p + o) << args_area_ptr;
    }

  public:
    void visit(VA_End_ref s) {
      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(VA_Arg_ref s) { throw analyzer_error("unreachable"); }

    void visit(VA_Copy_ref s) {
      scalar_lit_t dest = _lfac[ar::getDestination(s)];
      scalar_lit_t src = _lfac[ar::getSource(s)];

      if (!ar::isPointer(ar::getSource(s)) || !src.is_pointer_var() ||
          !ar::isPointer(ar::getDestination(s)) || !dest.is_pointer_var()) {
        // propagate invariant to the next statement
        _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
        return;
      }

#ifdef DEBUG
      std::cerr << "PTA: " << s << std::endl;
#endif

      new_object_ref_helper(ar::getSource(s), src.var());
      new_object_ref_helper(ar::getDestination(s), dest.var());

      ikos::pointer_var src_ptr = new_pointer_var(src.var());
      ikos::pointer_var dest_ptr = new_pointer_var(dest.var());
      ikos::pointer_var tmp_ptr = ikos::mk_pointer_var();

#ifdef DEBUG
      std::cerr << "\t" << *(tmp_ptr *= src_ptr + zero()) << std::endl;
      std::cerr << "\t" << *(dest_ptr + zero() << tmp_ptr) << std::endl;
#endif

      _cs += tmp_ptr *= (src_ptr + zero());
      _cs += (dest_ptr + zero()) << tmp_ptr;

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

  }; // end class PTA

private:
  typedef ikos::interval_domain< Number, VariableName > num_domain_t;
  typedef ikos::
      exception_domain_impl< num_domain_t, VariableFactory, MemoryLocation >
          abs_domain_t;

  typedef NumInvGen< abs_domain_t > num_inv_gen_t;
  typedef PTA< abs_domain_t, num_inv_gen_t > pta_t;
  typedef std::unordered_map< Function_ref, std::shared_ptr< num_inv_gen_t > >
      inv_gen_map_t;

  // for external queries
  typedef std::unordered_map<
      VariableName,
      std::pair< ikos::discrete_domain< MemoryLocation >, interval_t > >
      ptr_map_t;

  CfgFactory& _cfg_fac;
  VariableFactory& _vfac;
  memory_factory& _mfac;
  LiteralFactory& _lfac;
  LivenessPass& _live;
  PointerInfo _fun_ptr_info;

  ptr_map_t _ptr_map;

  void save(const ikos::pta_system& cs, // solved constraints
                                        // mapping varname to pointer variables
            const pta_t::pt_var_map_t& pt_var_map,
            // mapping id's to varname
            const pta_t::address_map_t& address_map) {
    for (pta_t::pt_var_map_t::const_iterator I = pt_var_map.begin(),
                                             E = pt_var_map.end();
         I != E;
         ++I) {
      ikos::address_set pt_set = cs.get(I->second).first;
      ikos::discrete_domain< MemoryLocation > pt_varset =
          ((pt_set.begin() == pt_set.end())
               ? ikos::discrete_domain< MemoryLocation >::top()
               : ikos::discrete_domain< MemoryLocation >::bottom());

      interval_t offset = cs.get(I->second).second;
      if (offset.is_bottom()) {
        offset = interval_t::top();
      }

      // renaming pt_set to pt_varset (from ids to memory locations)
      for (ikos::address_set::const_iterator PI = pt_set.begin(),
                                             PE = pt_set.end();
           PI != PE;
           ++PI) {
        pta_t::address_map_t::const_iterator AI = address_map.find(*PI);
        assert(AI != address_map.end());
        pt_varset = pt_varset | AI->second;
      }

      _ptr_map.insert(
          ptr_map_t::value_type(I->first, std::make_pair(pt_varset, offset)));

#ifdef DEBUG
      // only points-to sets for quick debugging
      ikos::index_traits< varname_t >::write(std::cerr, I->first);
      std::cerr << " -> " << pt_varset << ", " << offset << std::endl;
#endif
    }
  }

public:
  PointerPass(CfgFactory& cfg_fac,
              VariableFactory& vfac,
              memory_factory& mfac,
              LiteralFactory& lfac,
              LivenessPass& live,
              PointerInfo fun_ptr_info = PointerInfo())
      : _cfg_fac(cfg_fac),
        _vfac(vfac),
        _mfac(mfac),
        _lfac(lfac),
        _live(live),
        _fun_ptr_info(fun_ptr_info) {}

  // Run the pointer analysis
  void execute(Bundle_ref bundle) {
    FuncRange funcs = ar::getFunctions(bundle);
    inv_gen_map_t inv_table;

    std::cout << "** Computing intra-procedural numerical invariants ... "
              << std::endl;

    for (FuncRange::iterator I = funcs.begin(), E = funcs.end(); I != E; ++I) {
      arbos_cfg cfg = _cfg_fac[*I];
      std::shared_ptr< num_inv_gen_t > inv_gen(
          new num_inv_gen_t(cfg, _vfac, _mfac, _lfac, _live));
      inv_gen->run(abs_domain_t::top_no_exception());
      inv_table.insert(inv_gen_map_t::value_type(cfg.function(), inv_gen));
    }

    std::cout << "** Generating pointer constraints ... " << std::endl;

    // Shared by all functions
    ikos::pta_system cs;
    pta_t::pt_var_map_t pt_var_map;
    pta_t::address_map_t address_map;

    for (FuncRange::iterator I = funcs.begin(), E = funcs.end(); I != E; ++I) {
      Function_ref func = *I;
      arbos_cfg cfg = _cfg_fac[func];
      std::shared_ptr< num_inv_gen_t > inv_gen = inv_table[func];
      std::shared_ptr< pta_t > vis(new pta_t(bundle,
                                             *I,
                                             cs,
                                             inv_gen,
                                             _lfac,
                                             _vfac,
                                             _mfac,
                                             pt_var_map,
                                             address_map,
                                             _fun_ptr_info));
      ar::accept(func, vis);
    }

    std::cout << "** Solving pointer constraints ... " << std::endl;
    cs.solve();

    save(cs, pt_var_map, address_map);
  }

  // Return the pointer analysis results
  PointerInfo pointer_info() const {
    PointerInfo pointers;
    for (ptr_map_t::const_iterator I = _ptr_map.begin(), E = _ptr_map.end();
         I != E;
         ++I) {
      pointers.add(I->first, I->second);
    }
    return pointers;
  }

  // Dump the results of the analysis
  void write(std::ostream& o) {
    PointerInfo ptr_info = pointer_info();
    ptr_info.write(o);
  }
};

inline std::ostream& operator<<(std::ostream& o, PointerPass& s) {
  s.write(o);
  return o;
}

} // end namespace analyzer

#endif // ANALYZER_POINTER_HPP
