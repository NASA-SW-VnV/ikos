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
 *
 * Contact: ikos@lists.nasa.gov
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

#ifndef ANALYZER_POINTER_HPP
#define ANALYZER_POINTER_HPP

#include <ikos/domains/exception.hpp>
#include <ikos/domains/intervals.hpp>
#include <ikos/domains/pta.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/analysis/num_sym_exec.hpp>
#include <analyzer/ar-wrapper/cfg.hpp>
#include <analyzer/ikos-wrapper/iterators.hpp>

//#define DEBUG

namespace analyzer {

using namespace ikos;

class PointerPass {
  typedef varname_t VariableName;
  typedef ikos::z_number Number;

  // Generate numerical invariants for a function
  template < typename AbsNumDomain >
  class NumInvGen : public fwd_fixpoint_iterator< Basic_Block_ref,
                                                  arbos_cfg,
                                                  AbsNumDomain > {
    class Post : public arbos_visitor_api {
      typedef num_sym_exec< AbsNumDomain, VariableName, Number > sym_exec_t;

      sym_exec_t _post; //! transfer function
      VariableFactory& _vfac;

    public:
      Post(
          AbsNumDomain pre,
          VariableFactory& vfac,
          LiteralFactory& lfac,
          std::vector< VariableName > dead_vars = std::vector< VariableName >())
          : _post(pre,
                  vfac,
                  lfac,
                  REG /*do not change*/,
                  PointerInfo(),
                  dead_vars),
            _vfac(vfac) {}

      AbsNumDomain post() { return _post.inv(); }

      void visit_start(Basic_Block_ref b) { _post.exec_start(b); }
      void visit_end(Basic_Block_ref b) { _post.exec_end(b); }
      void visit(Arith_Op_ref s) { _post.exec(s); }
      void visit(Integer_Comparison_ref s) { _post.exec(s); }
      void visit(Bitwise_Op_ref s) { _post.exec(s); }
      void visit(Abstract_Variable_ref s) { _post.exec(s); }
      void visit(Unreachable_ref s) { _post.exec(s); }
      void visit(Assignment_ref s) { _post.exec(s); }
      void visit(Conv_Op_ref s) { _post.exec(s); }
      void visit(Pointer_Shift_ref s) { _post.exec(s); }
      void visit(Store_ref s) { _post.exec(s); }
      void visit(Load_ref s) { _post.exec(s); }
      void visit(MemCpy_ref s) { _post.exec(s); }
      void visit(MemMove_ref s) { _post.exec(s); }
      void visit(MemSet_ref s) { _post.exec(s); }
      void visit(Landing_Pad_ref s) { _post.exec(s); }
      void visit(Resume_ref s) { _post.exec(s); }
      void visit(Abstract_Memory_ref s) { _post.exec(s); }
      void visit(Call_ref s) {
        if (ar::isDirectCall(s) && ar::isExternal(s)) {
          _post.exec(s);
        } else {
          AbsNumDomain inv = _post.inv();

          if (exc_domain_traits::is_normal_flow_bottom(inv))
            return;

          exc_domain_traits::set_pending_exceptions_top(inv);

          if (ar::getReturnValue(s)) {
            VariableName lhs = _vfac[ar::getName(*(ar::getReturnValue(s)))];
            inv -= lhs;
            _post.set_inv(inv);
          }
        }
      }
      void visit(Invoke_ref s) { visit(ar::getFunctionCall(s)); }
      void visit(Return_Value_ref s) { _post.exec(s); }

      // NOT IMPLEMENTED
      void visit(FP_Op_ref) {}
      void visit(FP_Comparison_ref) {}
    };

    typedef fwd_fixpoint_iterator< Basic_Block_ref, arbos_cfg, AbsNumDomain >
        fwd_fixpoint_iterator_t;
    typedef std::unordered_map< Basic_Block_ref, AbsNumDomain > inv_table_t;

    VariableFactory& _vfac;
    LiteralFactory& _lfac;
    LivenessPass& _live;
    inv_table_t _inv_table;

  public:
    NumInvGen(arbos_cfg cfg,
              VariableFactory& vfac,
              LiteralFactory& lfac,
              LivenessPass& live)
        : fwd_fixpoint_iterator_t(cfg, true),
          _vfac(vfac),
          _lfac(lfac),
          _live(live) {}

    //! To propagate invariants at basic block level
    AbsNumDomain analyze(Basic_Block_ref bb, AbsNumDomain pre) {
      std::string f = this->get_cfg().get_func_name();
      arbos_cfg::arbos_node_t node = this->get_cfg().get_node(bb);

      std::shared_ptr< Post > vis(
          new Post(pre, _vfac, _lfac, _live.deadSet(f, bb)));
      node.accept(vis);
      return vis->post();
    }

    void check_pre(Basic_Block_ref bb, AbsNumDomain pre) {
      // we need to call explicitly visit_start()
      std::shared_ptr< Post > vis(new Post(pre, _vfac, _lfac));
      vis->visit_start(bb);
      _inv_table.insert(typename inv_table_t::value_type(bb, vis->post()));
    }

    void check_post(Basic_Block_ref bb, AbsNumDomain post) {}

  public:
    //! To propagate invariants at the statement level
    template < typename Statement >
    AbsNumDomain analyze_stmt(Statement stmt, AbsNumDomain pre) {
      std::shared_ptr< Post > vis(new Post(pre, _vfac, _lfac));
      vis->visit(stmt);
      return vis->post();
    }

    //! Get invariants that hold at the entry of a basic block
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
    typedef std::unordered_map< VariableName, pointer_var > pt_var_map_t;
    typedef std::unordered_map< ikos::index64_t, VariableName > address_map_t;
    typedef ikos::discrete_domain< VariableName > ptr_set_t;

  private:
    typedef std::shared_ptr< NumInvGen > num_inv_gen_t;

    Bundle_ref _bundle;
    Function_ref _func;
    pta_system& _cs;
    num_inv_gen_t _inv_gen;
    LiteralFactory& _lfac;
    VariableFactory& _vfac;
    pt_var_map_t& _pt_var_map;
    address_map_t& _address_map;
    PointerInfo _fun_ptr_info;
    AbsNumDomain _block_inv;

    //! Create a pointer variable
    pointer_var new_pointer_var(VariableName v) {
      pt_var_map_t::iterator it = _pt_var_map.find(v);
      if (it != _pt_var_map.end())
        return it->second;

      pointer_var pt = ikos::mk_pointer_var(v.index());
      _pt_var_map.insert(pt_var_map_t::value_type(v, pt));
      return pt;
    }

    //! Create a new object
    //  An object is a sequence of bytes in memory that can be
    //  allocated on the stack, code space (functions), or in the
    //  heap. Global variables of integer or fp type and strings are
    //  also objects.
    void new_object_ref(VariableName v, bool IsFunction = false) {
      address_map_t::iterator it = _address_map.find(v.index());
      if (it != _address_map.end())
        return;
      _address_map.insert(address_map_t::value_type(v.index(), v));

      if (IsFunction) {
        std::shared_ptr< function_ref > ref = ikos::mk_function_ref(v.index());
#ifdef DEBUG
        std::cout << "\t" << *(new_pointer_var(v) == ref) << std::endl;
#endif
        _cs += new_pointer_var(v) == ref;
      } else {
        std::shared_ptr< object_ref > ref =
            ikos::mk_object_ref(v.index(), zero());
#ifdef DEBUG
        std::cout << "\t" << *(new_pointer_var(v) == ref) << std::endl;
#endif
        _cs += new_pointer_var(v) == ref;
      }
    }

    //! Create a new parameter
    pointer_var new_param_ref(VariableName fname, unsigned param) {
      std::shared_ptr< param_ref > par_ref =
          ikos::mk_param_ref(new_pointer_var(fname), param);
      std::ostringstream buf;
      par_ref->print(buf);
      pointer_var p = new_pointer_var(_vfac[buf.str()]);
      return p;
    }

    //! Create a new return
    pointer_var new_return_ref(VariableName fname) {
      std::shared_ptr< return_ref > ret_ref =
          ikos::mk_return_ref(new_pointer_var(fname));
      std::ostringstream buf;
      ret_ref->print(buf);
      pointer_var p = new_pointer_var(_vfac[buf.str()]);
      return p;
    }

    //! Return the interval [0,0]
    z_interval zero() const {
      return z_interval(Literal::make_num< Number >(0),
                        Literal::make_num< Number >(0));
    }

    //! Return the interval [0,sz-1]
    z_interval get_size_interval(uint64_t sz) const {
      return z_interval(Literal::make_num< Number >(0),
                        Literal::make_num< Number >(sz - 1));
    }

    void assign(Literal Lhs, Operand_ref rhs) {
      Literal Rhs = _lfac[rhs];
      if (Rhs.is_null_cst() || Rhs.is_undefined_cst()) {
        return;
      }

      assert(Rhs.is_var());

      if (ar::isGlobalVar(rhs) || ar::isAllocaVar(rhs)) {
        new_object_ref(Rhs.get_var());
      } else if (ar::isFunctionPointer(rhs)) {
        new_object_ref(Rhs.get_var(), true /*function*/);
      }

      pointer_var p1 = new_pointer_var(Lhs.get_var());
      pointer_var p2 = new_pointer_var(Rhs.get_var());

#ifdef DEBUG
      std::cout << "\t" << *(p1 == (p2 + zero())) << std::endl;
#endif
      _cs += p1 == (p2 + zero());
    }

  public:
    PTA(Bundle_ref bundle,
        Function_ref func,
        // pointer constraint system
        pta_system& cs,
        // numerical invariants for the function
        num_inv_gen_t inv_gen,
        LiteralFactory& lfac,  // literal factory
        VariableFactory& vfac, // variable factory
        // mapping variable names to pointers
        pt_var_map_t& pt_var_map,
        // mapping id's to variable names
        address_map_t& address_map,
        PointerInfo fun_ptr_info = PointerInfo())
        : _bundle(bundle),
          _func(func),
          _cs(cs),
          _inv_gen(inv_gen),
          _lfac(lfac),
          _vfac(vfac),
          _pt_var_map(pt_var_map),
          _address_map(address_map),
          _fun_ptr_info(fun_ptr_info),
          _block_inv(AbsNumDomain::top()) {}

    void visit_start(Function_ref f) {
      IvRange fparams = ar::getFormalParams(f);
      unsigned int i = 0;
      VariableName fname = _vfac[ar::getName(f)];
#ifdef DEBUG
      std::cout << "PTA: formal parameters of " << fname << std::endl;
#endif
      for (IvRange::iterator It = fparams.begin(), E = fparams.end(); It != E;
           ++It, i++) {
        if (ar::isPointer(*It)) {
          pointer_var fp = new_pointer_var(_lfac[*It].get_var());
#ifdef DEBUG
          std::cout << "\t" << *(fp == (new_param_ref(fname, i) + zero()))
                    << std::endl;
#endif
          _cs += (fp == (new_param_ref(fname, i) + zero()));
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

    void visit(Bitwise_Op_ref s) {
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
      Internal_Variable_ref lhs = ar::getLeftOp(s);
      if (!ar::isPointer(lhs)) {
        // propagate invariant to the next statement
        _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
        return;
      }

#ifdef DEBUG
      std::cout << "PTA: " << s << std::endl;
#endif
      assign(_lfac[lhs], ar::getRightOp(s));

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Conv_Op_ref s) {
      Var_Operand_ref dest = node_cast< Var_Operand >(ar::getLeftOp(s));
      Operand_ref src = ar::getRightOp(s);

      switch (ar::getConvOp(s)) {
        case inttoptr:
          // TODO
          break;
        case bitcast: {
          if (ar::isPointer(src) || ar::isFunctionPointer(src)) {
#ifdef DEBUG
            std::cout << "PTA: " << s << std::endl;
#endif
            assign(_lfac[ar::getLeftOp(s)], src);
          }
        }
        default:;
      }

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Pointer_Shift_ref s) {
      Operand_ref base = ar::getBase(s);
      Operand_ref offset = ar::getOffset(s);
      Literal Lhs = _lfac[ar::getResult(s)];
      Literal Base = _lfac[base];
      Literal Offset = _lfac[offset];
      // pre: Lhs and Base are pointers

      if (Lhs.is_var() && Base.is_var()) {
        if (ar::isGlobalVar(base) || ar::isAllocaVar(base)) {
          new_object_ref(Base.get_var());
        }

#ifdef DEBUG
        std::cout << "PTA: " << s << std::endl;
#endif
        pointer_var p1 = new_pointer_var(Lhs.get_var());
        pointer_var p2 = new_pointer_var(Base.get_var());

        z_interval o = z_interval::top();
        if (Offset.is_num()) {
          o = z_interval(Offset.get_num< Number >(),
                         Offset.get_num< Number >());
        } else if (Offset.is_var()) {
          o = num_domain_traits::to_interval(_block_inv, Offset.get_var());
        } else {
          assert(false && "unreachable");
        }
#ifdef DEBUG
        std::cout << "\t" << *(p1 == (p2 + o)) << std::endl;
#endif
        _cs += p1 == (p2 + o);
      }

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Store_ref s) {
      Literal Lhs = _lfac[ar::getPointer(s)];
      Literal Rhs = _lfac[ar::getValue(s)];

      if (!ar::isPointer(ar::getPointer(s)) || !Lhs.is_var() || !Rhs.is_var() ||
          (!ar::isPointer(ar::getValue(s)) &&
           !ar::isFunctionPointer(ar::getValue(s)))) {
        // propagate invariant to the next statement
        _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
        return;
      }

#ifdef DEBUG
      std::cout << "PTA: " << s << std::endl;
#endif

      if (ar::isGlobalVar(ar::getPointer(s)) ||
          ar::isAllocaVar(ar::getPointer(s))) {
        new_object_ref(Lhs.get_var());
      }

      if (ar::isGlobalVar(ar::getValue(s)) ||
          ar::isAllocaVar(ar::getValue(s))) {
        new_object_ref(Rhs.get_var());
      } else if (ar::isFunctionPointer(ar::getValue(s))) {
        new_object_ref(Rhs.get_var(), true /*function*/);
      }

      pointer_var p1 = new_pointer_var(Lhs.get_var());
      pointer_var p2 = new_pointer_var(Rhs.get_var());

      z_interval o =
          get_size_interval(ar::getSize(ar::getType(ar::getValue(s))));
#ifdef DEBUG
      std::cout << "\t" << *(p1 + o << p2) << std::endl;
#endif
      _cs += (p1 + o) << p2;

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Load_ref s) {
      Literal Lhs = _lfac[ar::getResult(s)];
      Literal Rhs = _lfac[ar::getPointer(s)];

      if (!ar::isPointer(ar::getResult(s)) ||
          !ar::isPointer(ar::getPointer(s)) || !Rhs.is_var()) {
        // propagate invariant to the next statement
        _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
        return;
      }

#ifdef DEBUG
      std::cout << "PTA: " << s << std::endl;
#endif

      if (ar::isGlobalVar(ar::getPointer(s)) ||
          ar::isAllocaVar(ar::getPointer(s))) {
        new_object_ref(Rhs.get_var());
      }

      pointer_var p1 = new_pointer_var(Lhs.get_var());
      pointer_var p2 = new_pointer_var(Rhs.get_var());

      z_interval o =
          get_size_interval(ar::getSize(ar::getType(ar::getResult(s))));
#ifdef DEBUG
      std::cout << "\t" << *(p1 *= p2 + o) << std::endl;
#endif
      _cs += p1 *= (p2 + o);

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

  private:
    // memcpy(dest, src, len)
    // memmove(dest, src, len)
    template < typename MemCpyOrMove_ref >
    void visitMemCpyOrMove(MemCpyOrMove_ref s) {
      Literal src = _lfac[ar::getSource(s)];
      Literal dest = _lfac[ar::getTarget(s)];

      if (!ar::isPointer(ar::getSource(s)) || !src.is_var() ||
          !ar::isPointer(ar::getTarget(s)) || !dest.is_var()) {
        // propagate invariant to the next statement
        _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
        return;
      }

#ifdef DEBUG
      std::cout << "PTA: " << s << std::endl;
#endif

      if (ar::isGlobalVar(ar::getSource(s)) ||
          ar::isAllocaVar(ar::getSource(s))) {
        new_object_ref(src.get_var());
      }
      if (ar::isGlobalVar(ar::getTarget(s)) ||
          ar::isAllocaVar(ar::getTarget(s))) {
        new_object_ref(dest.get_var());
      }

      pointer_var src_ptr = new_pointer_var(src.get_var());
      pointer_var dest_ptr = new_pointer_var(dest.get_var());
      pointer_var tmp_ptr = ikos::mk_pointer_var();

#ifdef DEBUG
      std::cout << "\t" << *(tmp_ptr *= src_ptr + zero()) << std::endl;
      std::cout << "\t" << *(dest_ptr + zero() << tmp_ptr) << std::endl;
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
      boost::optional< Operand_ref > lhs = ar::getReturnValue(s);

      if (lhs && (ar::isPointer(*lhs) || ar::isFunctionPointer(*lhs))) {
        Literal Ret = _lfac[*lhs];

        if (!Ret.is_var()) {
          // propagate invariant to the next statement
          _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
          return;
        }

        if (ar::isGlobalVar(*lhs) || ar::isAllocaVar(*lhs)) {
          new_object_ref(Ret.get_var());
        } else if (ar::isFunctionPointer(*lhs)) {
          new_object_ref(Ret.get_var(), true /*function*/);
        }

        pointer_var p = new_pointer_var(Ret.get_var());
        VariableName fname = _vfac[ar::getName(_func)];
#ifdef DEBUG
        std::cout << "\t" << *(new_return_ref(fname) == (p + zero()))
                  << std::endl;
#endif
        _cs += (new_return_ref(fname) == (p + zero()));
      }

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(s, _block_inv);
    }

    void visit(Call_ref call_stmt) {
#ifdef DEBUG
      std::cout << "PTA call: " << call_stmt << std::endl;
#endif

      boost::optional< Internal_Variable_ref > lhs =
          ar::getReturnValue(call_stmt);
      OpRange actual_params = ar::getArguments(call_stmt);

      /*
       * Collect potential callees
       */

      std::vector< std::string > callees;

      if (ar::isIndirectCall(call_stmt)) {
        VariableName ptr =
            _vfac[ar::getName(ar::getIndirectCallVar(call_stmt))];
        ptr_set_t ptr_set = _fun_ptr_info[ptr].first;

        if (ptr_set.is_top()) {
          // propagate invariant to the next statement
          _block_inv = _inv_gen->analyze_stmt(call_stmt, _block_inv);
          return;
        }

        for (ptr_set_t::iterator it = ptr_set.begin(); it != ptr_set.end();
             ++it) {
          callees.push_back(it->name());
        }
      } else { // direct call
        callees.push_back(ar::getFunctionName(call_stmt));
      }

      /*
       * Add pointer constraints for each possible callee
       */

      for (std::vector< std::string >::iterator it = callees.begin();
           it != callees.end();
           ++it) {
        const std::string& fun_name = *it;
        boost::optional< Function_ref > callee = ar::getFunction(_bundle, *it);

        if (!callee || ar::isExternal(*callee)) {
          // ASSUMPTION: if the function code is not available, treat it as an
          // external call
          if (((fun_name == "malloc" || fun_name == "_Znwm" ||
                fun_name == "_Znam" ||
                fun_name == "__cxa_allocate_exception") &&
               actual_params.size() == 1) ||
              (fun_name == "calloc" && actual_params.size() == 2)) {
            // call to malloc, new, new[], __cxa_allocate_exception or calloc
            new_object_ref(_lfac[*lhs].get_var());
          }
        } else {
          if (ar::isVarargs(*callee))
            continue;

          IvRange formal_params = ar::getFormalParams(*callee);

          if (actual_params.size() != formal_params.size()) {
            // ASSUMPTION: all function calls have been checked by the compiler
            // and are well-formed. In that case, it means this function cannot
            // be called and that it is just an imprecision of the pointer
            // analysis.
            continue;
          }

          VariableName fname = _vfac[fun_name];
          new_object_ref(fname, true /*function*/);

          IvRange::iterator FIt = formal_params.begin();
          OpRange::iterator AIt = actual_params.begin();
          unsigned int i = 0;
          for (; AIt != actual_params.end(); ++AIt, ++FIt, i++) {
            if ((ar::isPointer(*AIt) || ar::isFunctionPointer(*AIt)) &&
                ar::isPointer(*FIt)) {
              if (!_lfac[*AIt].is_var()) {
                // the actual parameter can be null
                continue;
              }

              pointer_var ap = new_pointer_var(_lfac[*AIt].get_var());
              pointer_var fp = new_param_ref(fname, i);
              if (ar::isGlobalVar(*AIt) || ar::isAllocaVar(*AIt)) {
                new_object_ref(_lfac[*AIt].get_var());
              } else if (ar::isFunctionPointer(*AIt)) {
                new_object_ref(_lfac[*AIt].get_var(), true /*function*/);
              }

#ifdef DEBUG
              std::cout << "\t" << *(fp == (ap + zero())) << std::endl;
#endif
              _cs += fp == (ap + zero());
            }
          }

          if (lhs && ar::isPointer(*lhs)) {
            pointer_var p = new_pointer_var(_lfac[*lhs].get_var());
#ifdef DEBUG
            std::cout << "\t" << *(p == (new_return_ref(fname) + zero()))
                      << std::endl;
#endif
            _cs += (p == (new_return_ref(fname) + zero()));
          }
        }
      }

      // propagate invariant to the next statement
      _block_inv = _inv_gen->analyze_stmt(call_stmt, _block_inv);
    }

    void visit(Invoke_ref s) { visit(ar::getFunctionCall(s)); }

    // NOT IMPLEMENTED
    void visit(FP_Op_ref) {}
    void visit(FP_Comparison_ref) {}

  }; // end class PTA

private:
  typedef interval_domain< Number, VariableName > num_domain_t;
  typedef exception_domain_impl< num_domain_t > abs_domain_t;

  typedef NumInvGen< abs_domain_t > num_inv_gen_t;
  typedef PTA< abs_domain_t, num_inv_gen_t > pta_t;
  typedef std::unordered_map< Function_ref, std::shared_ptr< num_inv_gen_t > >
      inv_gen_map_t;

  //! for external queries
  typedef std::unordered_map<
      VariableName,
      std::pair< discrete_domain< VariableName >, z_interval > > ptr_map_t;

  CfgFactory& _cfg_fac;
  VariableFactory& _vfac;
  LiteralFactory& _lfac;
  LivenessPass& _live;
  PointerInfo _fun_ptr_info;

  ptr_map_t _ptr_map;

  void save(const pta_system& cs, // solved constraints
            // mapping varname to pointer variables
            const pta_t::pt_var_map_t& pt_var_map,
            // mapping id's to varname
            const pta_t::address_map_t& address_map) {
    for (pta_t::pt_var_map_t::const_iterator I = pt_var_map.begin(),
                                             E = pt_var_map.end();
         I != E;
         ++I) {
      address_set pt_set = cs.get(I->second).first;
      discrete_domain< VariableName > pt_varset =
          ((pt_set.begin() == pt_set.end())
               ? discrete_domain< VariableName >::top()
               : discrete_domain< VariableName >::bottom());

      z_interval offset = cs.get(I->second).second;
      if (offset.is_bottom()) {
        offset = z_interval::top();
      }

      // renaming pt_set to pt_varset (from ids to variable names)
      for (address_set::const_iterator PI = pt_set.begin(), PE = pt_set.end();
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
      std::cout << I->first << "-> " << pt_varset << std::endl;
#endif
    }
  }

public:
  PointerPass(CfgFactory& cfg_fac,
              VariableFactory& vfac,
              LiteralFactory& lfac,
              LivenessPass& live,
              PointerInfo fun_ptr_info = PointerInfo())
      : _cfg_fac(cfg_fac),
        _vfac(vfac),
        _lfac(lfac),
        _live(live),
        _fun_ptr_info(fun_ptr_info) {}

  //! Run the pointer analysis
  void execute(Bundle_ref bundle) {
    FuncRange funcs = ar::getFunctions(bundle);
    inv_gen_map_t inv_table;

    std::cout << "** Computing intra-procedural numerical invariants ... "
              << std::endl;

    for (FuncRange::iterator I = funcs.begin(), E = funcs.end(); I != E; ++I) {
      arbos_cfg cfg = _cfg_fac[*I];
      std::shared_ptr< num_inv_gen_t > inv_gen(
          new num_inv_gen_t(cfg, _vfac, _lfac, _live));
      inv_gen->run(abs_domain_t::top_no_exception());
      inv_table.insert(inv_gen_map_t::value_type(cfg.get_func(), inv_gen));
    }

    std::cout << "** Generating pointer constraints ... " << std::endl;

    // Shared by all functions
    pta_system cs;
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
                                             pt_var_map,
                                             address_map,
                                             _fun_ptr_info));
      ar::accept(func, vis);
    }

    std::cout << "** Solving pointer constraints ... " << std::endl;
    cs.solve();

    save(cs, pt_var_map, address_map);
  }

  //! Return the pointer analysis results
  PointerInfo pointer_info() const {
    PointerInfo pointers;
    for (ptr_map_t::const_iterator I = _ptr_map.begin(), E = _ptr_map.end();
         I != E;
         ++I) {
      pointers.add(I->first, I->second);
    }
    return pointers;
  }

  //! Dump the results of the analysis
  void write(std::ostream& o) {
    PointerInfo ptr_info = pointer_info();
    ptr_info.write(o);
  }
};

inline std::ostream& operator<<(std::ostream& o, PointerPass& s) {
  s.write(o);
  return o;
}

} // end namespace

#endif // ANALYZER_POINTER_HPP
