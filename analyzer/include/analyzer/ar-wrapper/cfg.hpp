/*******************************************************************************
 *
 * Build a IKOS Control Flow Graph from ARBOS IR
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

#ifndef ANALYZER_CFG_HPP
#define ANALYZER_CFG_HPP

#include <memory>
#include <unordered_map>

#include <boost/filesystem.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/optional.hpp>

#include <ikos/common/types.hpp>
#include <ikos/domains/discrete_domains.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/ar-wrapper/literal.hpp>
#include <analyzer/ar-wrapper/wrapper.hpp>

namespace analyzer {

using namespace arbos;

// The values must be such that REG <= PTR <= MEM
enum TrackedPrecision { REG = 0, PTR = 1, MEM = 2 };

boost::filesystem::path relative_to(boost::filesystem::path from,
                                    boost::filesystem::path to) {
  boost::filesystem::path::const_iterator from_it = from.begin();
  boost::filesystem::path::const_iterator to_it = to.begin();

  // loop through both
  while (from_it != from.end() && to_it != to.end() && *to_it == *from_it) {
    ++to_it;
    ++from_it;
  }

  boost::filesystem::path final_path;
  while (from_it != from.end()) {
    final_path /= "..";
    ++from_it;
  }

  while (to_it != to.end()) {
    final_path /= *to_it;
    ++to_it;
  }

  return final_path;
}

inline std::string format_path(const std::string& path) {
  boost::filesystem::path abs_path = boost::filesystem::exists(path)
                                         ? boost::filesystem::canonical(path)
                                         : path;
  boost::filesystem::path rel_path =
      relative_to(boost::filesystem::current_path(), abs_path);
  if (rel_path.string().size() < abs_path.string().size()) {
    return rel_path.string();
  } else {
    return abs_path.string();
  }
}

inline std::string location_to_string(const location& loc) {
  return format_path(loc.file) + ":" + std::to_string(loc.line) + ":" +
         std::to_string(loc.column);
}

inline bool IsMain(arbos::Function_ref func) {
  return ar::getName(func) == "main";
}

//! Collect all the used and defined variables for each statement
class live_info : public arbos_visitor_api {
public:
  typedef std::shared_ptr< live_info > live_info_ptr;
  typedef std::unordered_map< uint64_t, varname_set_t > varset_map_t;

private:
  varset_map_t _uses;
  varset_map_t _defs;
  varname_set_t _all_vars;
  const TrackedPrecision _prec_level;
  VariableFactory& _vfac;

  inline bool isTrackable(const Operand_ref& o) {
    switch (_prec_level) {
      case REG:
        return (!ar::isPointer(o) && ar::isRegVar(o));
      case PTR:
      case MEM:
      default:
        return (ar::isRegVar(o) || ar::isGlobalVar(o) || ar::isAllocaVar(o));
    }
  }

  inline bool isTrackable(const Internal_Variable_ref& v) {
    switch (_prec_level) {
      case REG:
        return !ar::isPointer(v);
      case PTR:
      case MEM:
      default:
        return true;
    }
  }

  inline bool isTrackable(const Var_Addr_Constant_ref& v) {
    return _prec_level > REG; // v is a pointer
  }

  // XXX: REFACTORING NEEDED FOR THESE FUNCTIONS
  //! Add helper function to add the variable in the factory
  //  Overloading for AR_Operand
  varname_t add_to_factory(Operand_ref var) {
    if (ar::ar_internal::is_cst_operand(var)) {
      return add_to_factory(node_cast< AR_Cst_Operand >(var));
    } else if (ar::ar_internal::is_var_operand(var)) {
      return add_to_factory(node_cast< AR_Var_Operand >(var));
    } else {
      throw analyzer_error("CFG analysis: unexpected AR Variable type");
    }
  }

  //! Overloading for AR_Cst_Operand
  varname_t add_to_factory(Cst_Operand_ref var) {
    // Cast the var to add in the varset
    Constant_ref cst_op = var->getConstant();
    if (ar::ar_internal::is_function_addr_constant(cst_op)) {
      return _vfac.get_function_addr(
          node_cast< Function_Addr_Constant >(cst_op));
    } else if (ar::ar_internal::is_var_addr_constant(cst_op)) {
      Var_Addr_Constant_ref cst_op_cast =
          node_cast< AR_Var_Addr_Constant >(cst_op);
      return add_to_factory(
          node_cast< AR_Variable >(cst_op_cast->getVariable()));
    } else {
      throw analyzer_error("CFG analysis: unexpected AR Variable type");
    }
  }

  //! Overloading for AR_Var_Operand
  varname_t add_to_factory(Var_Operand_ref var) {
    return _vfac.get_internal(
        node_cast< AR_Internal_Variable >(var->getInternalVariable()));
  }

  //! Overloading for AR_Internal_Variable
  varname_t add_to_factory(Internal_Variable_ref var) {
    return _vfac.get_internal(var);
  }

  //! Overloading for AR_Variable
  varname_t add_to_factory(Variable_ref var) {
    if (ar::ar_internal::is_local_var(var)) {
      return _vfac.get_local(node_cast< AR_Local_Variable >(var));
    } else if (ar::ar_internal::is_global_var(var)) {
      return _vfac.get_global(node_cast< AR_Global_Variable >(var));
    } else if (ar::ar_internal::is_internal_var(var)) {
      return _vfac.get_internal(node_cast< AR_Internal_Variable >(var));
    } else {
      throw analyzer_error("CFG analysis: unexpected AR Variable type");
    }
  }

  //! Overloading for AR_Var_Addr_Constant
  varname_t add_to_factory(Var_Addr_Constant_ref var) {
    return add_to_factory(node_cast< AR_Variable >(var->getVariable()));
  }

  template < typename Statement_ref, typename Variable >
  void add(varset_map_t& t, const Statement_ref& s, Variable var) {
    if (!isTrackable(var)) {
      return;
    }

    varname_t new_var_in_set = add_to_factory(var);

    typename varset_map_t::iterator it = t.find(ar::getUID(s));
    if (it == t.end()) {
      varname_set_t varset(new_var_in_set);
      t.insert(std::make_pair(ar::getUID(s), varset));
    } else {
      varname_set_t varset = it->second;
      varset += new_var_in_set;
      t[ar::getUID(s)] = varset;
    }

    _all_vars += new_var_in_set;
  }

public:
  live_info(VariableFactory& vfac, TrackedPrecision level)
      : _all_vars(), _prec_level(level), _vfac(vfac) {}

  // Return the set of used variables of s according to the level of
  // precision
  varname_set_t uses(const Statement_ref& s) {
    varset_map_t::iterator it = _uses.find(ar::getUID(s));
    if (it != _uses.end()) {
      return it->second;
    } else {
      return varname_set_t();
    }
  }

  // Return the set of defined variables of s according to the level
  // of precision
  varname_set_t defs(const Statement_ref& s) {
    varset_map_t::iterator it = _defs.find(ar::getUID(s));
    if (it != _defs.end()) {
      return it->second;
    } else {
      return varname_set_t();
    }
  }

  // Return all the variables (used or defined) according to the level
  // of precision.
  varname_set_t all_vars() const { return _all_vars; }

private:
  void visit(Arith_Op_ref s) {
    add(this->_defs, s, ar::getResult(s));
    add(this->_uses, s, ar::getLeftOp(s));
    add(this->_uses, s, ar::getRightOp(s));
  }

  void visit(FP_Op_ref s) {
    add(this->_defs, s, ar::getResult(s));
    add(this->_uses, s, ar::getLeftOp(s));
    add(this->_uses, s, ar::getRightOp(s));
  }

  void visit(Integer_Comparison_ref s) {
    add(this->_uses, s, ar::getLeftOp(s));
    add(this->_uses, s, ar::getRightOp(s));
  }

  void visit(FP_Comparison_ref s) {
    add(this->_uses, s, ar::getLeftOp(s));
    add(this->_uses, s, ar::getRightOp(s));
  }

  void visit(Bitwise_Op_ref s) {
    add(this->_defs, s, ar::getResult(s));
    add(this->_uses, s, ar::getLeftOp(s));
    add(this->_uses, s, ar::getRightOp(s));
  }

  void visit(Abstract_Variable_ref s) { add(this->_defs, s, ar::getVar(s)); }

  void visit(Assignment_ref s) {
    add(this->_defs, s, ar::getLeftOp(s));
    add(this->_uses, s, ar::getRightOp(s));
  }

  void visit(Conv_Op_ref s) {
    add(this->_defs, s, ar::getLeftOp(s));
    add(this->_uses, s, ar::getRightOp(s));
  }

  void visit(Allocate_ref s) {
    add(this->_defs, s, ar::getResult(s));
    add(this->_uses, s, ar::getArraySize(s));
  }

  void visit(Load_ref s) {
    add(this->_defs, s, ar::getResult(s));
    add(this->_uses, s, ar::getPointer(s));
  }

  void visit(Store_ref s) {
    add(this->_uses, s, ar::getPointer(s));
    add(this->_uses, s, ar::getValue(s));
  }

  void visit(Extract_Element_ref s) {
    add(this->_defs, s, ar::getResult(s));
    add(this->_uses, s, ar::getAggregate(s));
    add(this->_uses, s, ar::getOffset(s));
  }

  void visit(Insert_Element_ref s) {
    add(this->_defs, s, ar::getResult(s));
    add(this->_uses, s, ar::getAggregate(s));
    add(this->_uses, s, ar::getOffset(s));
    add(this->_uses, s, ar::getElement(s));
  }

  void visit(Pointer_Shift_ref s) {
    add(this->_defs, s, ar::getResult(s));
    add(this->_uses, s, ar::getBase(s));
    add(this->_uses, s, ar::getOffset(s));
  }

  void visit(Call_ref s) {
    boost::optional< Internal_Variable_ref > r = ar::getReturnValue(s);
    if (r) {
      add(this->_defs, s, *r);
    }
    OpRange args = ar::getArguments(s);
    for (OpRange::iterator it = args.begin(); it != args.end(); ++it) {
      add(this->_uses, s, *it);
    }
  }

  void visit(Invoke_ref s) {
    Call_ref call = ar::getFunctionCall(s);
    boost::optional< Internal_Variable_ref > r = ar::getReturnValue(call);
    if (r) {
      add(this->_defs, s, *r);
    }
    OpRange args = ar::getArguments(call);
    for (OpRange::iterator it = args.begin(); it != args.end(); ++it) {
      add(this->_uses, s, *it);
    }
  }

  void visit(Return_Value_ref s) {
    boost::optional< Operand_ref > r = ar::getReturnValue(s);
    if (r) {
      add(this->_uses, s, *r);
    }
  }

  void visit(Abstract_Memory_ref s) {
    add(this->_uses, s, ar::getPointer(s));
    add(this->_uses, s, ar::getLen(s));
  }

  void visit(MemCpy_ref s) {
    // both source and target are already allocated in memory so they
    // are uses here
    add(this->_uses, s, ar::getSource(s));
    add(this->_uses, s, ar::getTarget(s));
    add(this->_uses, s, ar::getLen(s));
  }

  void visit(MemMove_ref s) {
    // target is already allocated in memory
    add(this->_uses, s, ar::getTarget(s));
    add(this->_uses, s, ar::getSource(s));
    add(this->_uses, s, ar::getLen(s));
  }

  void visit(MemSet_ref s) {
    // base is already allocated in memory
    add(this->_uses, s, ar::getBase(s));
    add(this->_uses, s, ar::getValue(s));
    add(this->_uses, s, ar::getLen(s));
  }

  void visit(Landing_Pad_ref s) { add(this->_defs, s, ar::getVar(s)); }

  void visit(Resume_ref s) { add(this->_uses, s, ar::getVar(s)); }

  void visit(Unreachable_ref) {}

  void visit(VA_Start_ref s) { add(this->_uses, s, ar::getPointer(s)); }

  void visit(VA_End_ref s) { add(this->_uses, s, ar::getPointer(s)); }

  void visit(VA_Arg_ref s) {
    add(this->_defs, s, ar::getResult(s));
    add(this->_uses, s, ar::getPointer(s));
  }

  void visit(VA_Copy_ref s) {
    add(this->_uses, s, ar::getSource(s));
    add(this->_uses, s, ar::getDestination(s));
  }
};

class arbos_cfg;

// Simple wrapper for Basic_Block_ref
template < typename T >
class arbos_node {
  friend class arbos_cfg;
  typedef arbos_node< T > arbos_node_t;

public:
  //! required by Ikos fwd_fixpoint_iterator
  typedef std::vector< T > node_collection_t;

  typedef std::vector< Statement_ref >::iterator iterator;
  typedef std::vector< Statement_ref >::const_iterator const_iterator;

private:
  Basic_Block_ref _block;
  node_collection_t _prev_nodes;
  node_collection_t _next_nodes;
  std::vector< Statement_ref > _stmts;
  VariableFactory& _vfac;
  live_info::live_info_ptr _live;

  // set of non-negative loop counters initialized in this basic block
  varname_set_t _loop_counters;

  arbos_node(Basic_Block_ref block,
             VariableFactory& vfac,
             TrackedPrecision level,
             node_collection_t preds,
             node_collection_t succs,
             std::vector< Statement_ref > stmts,
             live_info::live_info_ptr live)
      : _block(block),
        _prev_nodes(preds),
        _next_nodes(succs),
        _stmts(stmts),
        _vfac(vfac),
        _live(live),
        _loop_counters() {}

public:
  node_collection_t& next_nodes() { return _next_nodes; }
  node_collection_t& prev_nodes() { return _prev_nodes; }

public:
  Basic_Block_ref get() { return _block; }

  iterator begin() { return _stmts.begin(); }
  iterator end() { return _stmts.end(); }
  const_iterator begin() const { return _stmts.begin(); }
  const_iterator end() const { return _stmts.end(); }

  void accept(std::shared_ptr< arbos_visitor_api > vis) {
    vis->visit_start(_block);

    for (iterator I = begin(), E = end(); I != E; ++I) {
      ar::accept(*I, vis);
    }

    vis->visit_end(_block);
  }

  std::string name() const { return ar::getName(_block); }

  void reverse() {
    std::swap(_prev_nodes, _next_nodes);
    std::reverse(_stmts.begin(), _stmts.end());
  }

  void mark_loop_counter(varname_t v) { this->_loop_counters += v; }

  // Set of non-negative loop counters initialized in this node
  varname_set_t loop_counters() const { return this->_loop_counters; }

  //! Return the set of used variables at statement s
  varname_set_t uses(Statement_ref s) const { return _live->uses(s); }

  //! Return the set of defined variables at statement s
  varname_set_t defs(Statement_ref s) const { return _live->defs(s); }

  //! Return all the used/defined variables in the basic block
  varname_set_t all_vars() const { return _live->all_vars(); }

  void write(std::ostream& o) {
    o << name() << ":" << std::endl;
    for (const_iterator I = begin(), E = end(); I != E; ++I) {
      o << "\t" << *I << std::endl;
    }
  }

  friend std::ostream& operator<<(std::ostream& o, arbos_node node) {
    node.write(o);
    return o;
  }
};

class CfgFactory;

// Class to represent a CFG
class arbos_cfg {
  friend class CfgFactory;

public:
  //! cfg node
  typedef arbos_node< Basic_Block_ref > arbos_node_t;

private:
  typedef std::unordered_map< Basic_Block_ref, arbos_node_t > nodes_map_t;

  struct MkSecond
      : public std::unary_function< nodes_map_t::value_type, arbos_node_t > {
    typedef nodes_map_t::value_type Pair;
    typedef arbos_node_t Second;

    MkSecond() {}
    Second operator()(const Pair& p) const { return p.second; }
  };

public:
  //! required by the fixpoint iterator
  typedef arbos_node_t::node_collection_t node_collection_t;

  //! iterator to traverse cfg nodes
  typedef boost::transform_iterator< MkSecond, nodes_map_t::iterator > iterator;

private:
  std::shared_ptr< nodes_map_t > _nodes_map;
  Function_ref _func;
  Basic_Block_ref _entry;
  boost::optional< Basic_Block_ref > _exit;
  std::vector< Local_Variable_ref > _local_vars;
  std::vector< Internal_Variable_ref > _formal_vars;
  std::vector< Internal_Variable_ref > _internal_vars;
  VariableFactory& _vfac;
  LiteralFactory& _lfac;
  std::string _arch;
  const TrackedPrecision _prec_level;

  //! helper
  inline void InsertAdjacent(node_collection_t& c, Basic_Block_ref e) {
    if (find(c.begin(), c.end(), e) == c.end()) {
      c.push_back(e);
    }
  }

  arbos_node_t& makeNode(Basic_Block_ref bb) {
    nodes_map_t::iterator it = _nodes_map->find(bb);
    if (it != _nodes_map->end()) {
      return it->second;
    }

    std::vector< Statement_ref > stmts;
    node_collection_t preds, succs;
    live_info::live_info_ptr live(new live_info(_vfac, _prec_level));

    // Compute use and def sets
    ar::accept(bb, live);

    { // Collect the statements
      StmtRange s = ar::getStatements(bb);
      for (StmtRange::iterator it = s.begin(), et = s.end(); it != et; ++it) {
        stmts.push_back(*it);
      }
    }

    { // Collect the predecessors
      BBRange p = ar::getPreds(bb);
      for (BBRange::iterator it = p.begin(), et = p.end(); it != et; ++it) {
        InsertAdjacent(preds, *it);
      }
    }

    { // Collect the successors
      BBRange s = ar::getSuccs(bb);
      for (BBRange::iterator it = s.begin(), et = s.end(); it != et; ++it) {
        InsertAdjacent(succs, *it);
      }
    }

    arbos_node_t node(bb, _vfac, _prec_level, preds, succs, stmts, live);

    std::pair< nodes_map_t::iterator, bool > res =
        _nodes_map->insert(std::make_pair(bb, node));
    return (res.first)->second;
  }

  arbos_cfg(Function_ref func,
            VariableFactory& vfac,
            LiteralFactory& lfac,
            const std::string& arch,
            TrackedPrecision level)
      : _nodes_map(new nodes_map_t()),
        _func(func),
        _entry(Null_ref),
        _vfac(vfac),
        _lfac(lfac),
        _arch(arch),
        _prec_level(level) {
    Code_ref body = ar::getBody(this->_func);

    { // collect basic blocks
      BBRange b = ar::getBlocks(body);
      for (BBRange::iterator it = b.begin(), et = b.end(); it != et; ++it) {
        makeNode(*it);
      }
    }

    { // collect formal parameters
      IvRange f = ar::getFormalParams(this->_func);
      for (IvRange::iterator it = f.begin(), et = f.end(); it != et; ++it) {
        _formal_vars.push_back(*it);
      }
    }

    { // collect local variables: the ones involved in alloca's.
      LvRange v = ar::getLocalVars(this->_func);
      for (LvRange::iterator it = v.begin(), et = v.end(); it != et; ++it) {
        _local_vars.push_back(*it);
      }
    }

    { // collect internal local variables: registers
      // note: formal parameters are also part of internal local variables.
      IvRange v = ar::getInternalVars(body);
      for (IvRange::iterator it = v.begin(), et = v.end(); it != et; ++it) {
        _internal_vars.push_back(*it);
      }
    }

    // record entry
    _entry = ar::getEntryBlock(body);
    makeNode(_entry);

    // record exit
    _exit = ar::getExitBlock(body);
    if (_exit) {
      makeNode(*_exit);
    }
  }

public:
  // Do not modify api's of these three methods unless Ikos fixpoint
  // iterator agrees.

  //! required by the Ikos fixpoint iterator
  Basic_Block_ref entry() { return _entry; }

  //! required by the Ikos fixpoint iterator
  node_collection_t& next_nodes(Basic_Block_ref bb) {
    arbos_node_t& node = get_node(bb);
    return node.next_nodes();
  }

  //! required by the Ikos fixpoint iterator
  node_collection_t& prev_nodes(Basic_Block_ref bb) {
    arbos_node_t& node = get_node(bb);
    return node.prev_nodes();
  }

public:
  //! make a fresh (deep) copy of the cfg. Note that the default copy
  //! constructor will make a shallow copy. This is intended.
  arbos_cfg clone() const {
    return arbos_cfg(_func, _vfac, _lfac, _arch, _prec_level);
  }

  //! return an begin iterator to the nodes in the cfg
  iterator begin() {
    return boost::make_transform_iterator(_nodes_map->begin(), MkSecond());
  }

  //! return an end iterator to the nodes in the cfg
  iterator end() {
    return boost::make_transform_iterator(_nodes_map->end(), MkSecond());
  }

  //! map a basic block to Cfg node.
  arbos_node_t& get_node(Basic_Block_ref b) {
    nodes_map_t::iterator it = _nodes_map->find(b);
    assert(it != _nodes_map->end());
    return it->second;
  }

  boost::optional< Basic_Block_ref > exit() const { return this->_exit; }

  VariableFactory& var_factory() const { return this->_vfac; }

  LiteralFactory& lit_factory() const { return this->_lfac; }

  std::string func_name() const { return ar::getName(this->_func); }

  Function_ref function() const { return this->_func; }

  const std::string& arch() const { return this->_arch; }

  TrackedPrecision prec_level() const { return this->_prec_level; }

  std::vector< Local_Variable_ref >& local_variables() {
    return this->_local_vars;
  }

  std::vector< Internal_Variable_ref >& internal_variables() {
    return this->_internal_vars;
  }

  std::vector< Internal_Variable_ref >& formal_parameters() {
    return this->_formal_vars;
  }

  std::pair< iterator, iterator > get_nodes() {
    return std::make_pair(begin(), end());
  }

  void accept(std::shared_ptr< arbos_visitor_api > vis) {
    vis->visit_start(_func);

    for (iterator I = begin(), E = end(); I != E; ++I) {
      (*I).accept(vis);
    }

    vis->visit_end(_func);
  }

  //! Return true if the cfg has been reversed.
  //! The reverse only modifies: _entry, _exit, and _nodes_map. The AR
  //! datastructures are untouched.
  bool reverse() {
    if (!_exit)
      return false;
    std::swap(_entry, *_exit);
    for (nodes_map_t::iterator I = _nodes_map->begin(), E = _nodes_map->end();
         I != E;
         ++I) {
      arbos_node_t& n = I->second;
      n.reverse();
    }

    return true;
  }

  void write(std::ostream& o) {
    // Function
    o << "Function: " << func_name() << std::endl;
    o << "entry block: " << _entry << std::endl;
    if (_exit)
      o << "exit block: " << *_exit << std::endl;
    else
      o << "exit block: none\n";
    // Blocks
    {
      for (iterator I = begin(), E = end(); I != E; ++I) {
        arbos_node_t node = *I;
        o << node.name() << ":\n";
        for (arbos_node_t::iterator S = node.begin(); S != node.end(); ++S) {
          o << *S << std::endl;
        }
      }
    }
    // control flow structure
    {
      for (iterator I = begin(), E = end(); I != E; ++I) {
        arbos_node_t node = *I;
        o << node.name() << " -> [";
        node_collection_t succs = node.next_nodes();
        for (node_collection_t::iterator s = succs.begin(); s != succs.end();) {
          o << *s;
          ++s;
          if (s != succs.end())
            o << ";";
        }
        o << "]\n";
      }
    }
  }

  friend std::ostream& operator<<(std::ostream& o, arbos_cfg cfg) {
    cfg.write(o);
    return o;
  }
};

//! A factory for CFGs
class CfgFactory : public boost::noncopyable {
private:
  typedef std::shared_ptr< arbos_cfg > arbos_cfg_ptr;
  typedef std::unordered_map< Function_ref, arbos_cfg_ptr > cfg_map_t;

private:
  Bundle_ref _bundle;
  VariableFactory& _vfac;
  memory_factory& _mfac;
  LiteralFactory& _lfac;
  std::string _arch;
  const TrackedPrecision _level;
  cfg_map_t _map;

public:
  CfgFactory(Bundle_ref bundle,
             VariableFactory& vfac,
             memory_factory& mfac,
             LiteralFactory& lfac,
             TrackedPrecision level)
      : _bundle(bundle),
        _vfac(vfac),
        _mfac(mfac),
        _lfac(lfac),
        _arch(ar::getTargetArch(bundle)),
        _level(level) {}

  arbos_cfg operator[](Function_ref f) {
    cfg_map_t::iterator it = _map.find(f);
    if (it == _map.end()) {
      arbos_cfg_ptr cfg(new arbos_cfg(f, _vfac, _lfac, _arch, _level));
      std::pair< cfg_map_t::iterator, bool > res =
          _map.insert(std::make_pair(f, cfg));
      return *((res.first)->second);
    } else {
      return *(it->second);
    }
  }

  Bundle_ref bundle() const { return _bundle; }

  VariableFactory& var_factory() const { return _vfac; }

  LiteralFactory& lit_factory() const { return _lfac; }

  memory_factory& mem_factory() const { return _mfac; }

  const std::string& arch() const { return _arch; }

  TrackedPrecision prec_level() const { return _level; }
};

} // end namespace analyzer

#endif // ANALYZER_CFG_HPP
