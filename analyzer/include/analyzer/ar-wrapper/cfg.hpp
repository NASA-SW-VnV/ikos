/*******************************************************************************
 *
 * Build a IKOS Control Flow Graph from ARBOS IR
 *
 * Author: Jorge A. Navas
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

// convert from arbos to ikos
inline boost::optional< ikos::operation_t > translate_op(arbos::ArithOp op) {
  switch (op) {
    case arbos::add:
      return boost::optional< ikos::operation_t >(ikos::OP_ADDITION);
    case arbos::sub:
      return boost::optional< ikos::operation_t >(ikos::OP_SUBTRACTION);
    case arbos::mul:
      return boost::optional< ikos::operation_t >(ikos::OP_MULTIPLICATION);
    case arbos::sdiv:
      return boost::optional< ikos::operation_t >(ikos::OP_DIVISION);
    default: /* udiv, urem, srem */
      return boost::optional< ikos::operation_t >();
  }
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
  LiteralFactory& _lfac;

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

  inline std::string get_name(Operand_ref o) {
    const Literal& lit = _lfac[o];
    if (lit.is_var())
      return lit.get_var_str();
    else
      return "";
  }

  inline std::string get_name(Internal_Variable_ref v) {
    return ar::getName(v);
  }

  template < typename Statement_ref >
  inline void add_helper(varset_map_t& t,
                         const Statement_ref& s,
                         std::string var_str) {
    typename varset_map_t::iterator it = t.find(ar::getUID(s));
    if (it == t.end()) {
      varname_set_t varset = varname_set_t::bottom();
      varset += _vfac[var_str];
      t.insert(std::make_pair(ar::getUID(s), varset));
    } else {
      varname_set_t varset = it->second;
      varset += _vfac[var_str];
      t[ar::getUID(s)] = varset;
    }
  }

  template < typename Statement_ref, typename Variable >
  inline void add(varset_map_t& t,
                  const Statement_ref& s,
                  Variable var,
                  std::string var_str) {
    if (!isTrackable(var))
      return;
    assert(var_str != "");

    add_helper(t, s, var_str);
    _all_vars += _vfac[var_str];
  }

public:
  live_info(VariableFactory& vfac, LiteralFactory& lfac, TrackedPrecision level)
      : _all_vars(varname_set_t::bottom()),
        _prec_level(level),
        _vfac(vfac),
        _lfac(lfac) {}

  // Return the set of used variables of s according to the level of
  // precision
  varname_set_t uses(const Statement_ref& s) {
    varset_map_t::iterator it = _uses.find(ar::getUID(s));
    if (it != _uses.end()) {
      return it->second;
    } else {
      return varname_set_t::bottom();
    }
  }

  // Return the set of defined variables of s according to the level
  // of precision
  varname_set_t defs(const Statement_ref& s) {
    varset_map_t::iterator it = _defs.find(ar::getUID(s));
    if (it != _defs.end()) {
      return it->second;
    } else {
      return varname_set_t::bottom();
    }
  }

  // Return all the variables (used or defined) according to the level
  // of precision.
  varname_set_t all_vars() const { return _all_vars; }

private:
  void visit(Arith_Op_ref s) {
    add(this->_defs, s, ar::getResult(s), get_name(ar::getResult(s)));
    add(this->_uses, s, ar::getLeftOp(s), get_name(ar::getLeftOp(s)));
    add(this->_uses, s, ar::getRightOp(s), get_name(ar::getRightOp(s)));
  }

  void visit(FP_Op_ref s) {
    add(this->_defs, s, ar::getResult(s), get_name(ar::getResult(s)));
    add(this->_uses, s, ar::getLeftOp(s), get_name(ar::getLeftOp(s)));
    add(this->_uses, s, ar::getRightOp(s), get_name(ar::getRightOp(s)));
  }

  void visit(Integer_Comparison_ref s) {
    add(this->_uses, s, ar::getLeftOp(s), get_name(ar::getLeftOp(s)));
    add(this->_uses, s, ar::getRightOp(s), get_name(ar::getRightOp(s)));
  }

  void visit(FP_Comparison_ref s) {
    add(this->_uses, s, ar::getLeftOp(s), get_name(ar::getLeftOp(s)));
    add(this->_uses, s, ar::getRightOp(s), get_name(ar::getRightOp(s)));
  }

  void visit(Bitwise_Op_ref s) {
    add(this->_defs, s, ar::getResult(s), get_name(ar::getResult(s)));
    add(this->_uses, s, ar::getLeftOp(s), get_name(ar::getLeftOp(s)));
    add(this->_uses, s, ar::getRightOp(s), get_name(ar::getRightOp(s)));
  }

  void visit(Abstract_Variable_ref s) {
    add(this->_defs, s, ar::getVar(s), ar::getName(s));
  }

  void visit(Assignment_ref s) {
    add(this->_defs, s, ar::getLeftOp(s), get_name(ar::getLeftOp(s)));
    add(this->_uses, s, ar::getRightOp(s), get_name(ar::getRightOp(s)));
  }

  void visit(Conv_Op_ref s) {
    add(this->_defs, s, ar::getLeftOp(s), get_name(ar::getLeftOp(s)));
    add(this->_uses, s, ar::getRightOp(s), get_name(ar::getRightOp(s)));
  }

  void visit(Load_ref s) {
    add(this->_defs, s, ar::getResult(s), get_name(ar::getResult(s)));
    add(this->_uses, s, ar::getPointer(s), get_name(ar::getPointer(s)));
  }

  void visit(Pointer_Shift_ref s) {
    add(this->_defs, s, ar::getResult(s), get_name(ar::getResult(s)));
    add(this->_uses, s, ar::getBase(s), get_name(ar::getBase(s)));
    add(this->_uses, s, ar::getOffset(s), get_name(ar::getOffset(s)));
  }

  void visit(Call_ref s) {
    boost::optional< Internal_Variable_ref > r = ar::getReturnValue(s);
    if (r) {
      add(this->_defs, s, *r, ar::getName(*r));
    }
    OpRange args = ar::getArguments(s);
    for (OpRange::iterator it = args.begin(); it != args.end(); ++it) {
      add(this->_uses, s, *it, get_name(*it));
    }
  }

  void visit(Invoke_ref s) {
    Call_ref call = ar::getFunctionCall(s);
    boost::optional< Internal_Variable_ref > r = ar::getReturnValue(call);
    if (r) {
      add(this->_defs, s, *r, ar::getName(*r));
    }
    OpRange args = ar::getArguments(call);
    for (OpRange::iterator it = args.begin(); it != args.end(); ++it) {
      add(this->_uses, s, *it, get_name(*it));
    }
  }

  void visit(Return_Value_ref s) {
    boost::optional< Operand_ref > r = ar::getReturnValue(s);
    if (r) {
      add(this->_uses, s, *r, get_name(*r));
    }
  }

  void visit(Store_ref s) {
    add(this->_uses, s, ar::getPointer(s), get_name(ar::getPointer(s)));
    add(this->_uses, s, ar::getValue(s), get_name(ar::getValue(s)));
  }

  void visit(Abstract_Memory_ref s) {
    add(this->_uses, s, ar::getPointer(s), get_name(ar::getPointer(s)));
    add(this->_uses, s, ar::getLen(s), get_name(ar::getLen(s)));
  }

  void visit(MemCpy_ref s) {
    // both source and target are already allocated in memory so they
    // are uses here
    add(this->_uses, s, ar::getSource(s), get_name(ar::getSource(s)));
    add(this->_uses, s, ar::getTarget(s), get_name(ar::getTarget(s)));
    add(this->_uses, s, ar::getLen(s), get_name(ar::getLen(s)));
  }

  void visit(MemMove_ref s) {
    // target is already allocated in memory
    add(this->_uses, s, ar::getTarget(s), get_name(ar::getTarget(s)));
    add(this->_uses, s, ar::getSource(s), get_name(ar::getSource(s)));
    add(this->_uses, s, ar::getLen(s), get_name(ar::getLen(s)));
  }

  void visit(MemSet_ref s) {
    // base is already allocated in memory
    add(this->_uses, s, ar::getBase(s), get_name(ar::getBase(s)));
    add(this->_uses, s, ar::getValue(s), get_name(ar::getValue(s)));
    add(this->_uses, s, ar::getLen(s), get_name(ar::getLen(s)));
  }

  void visit(Landing_Pad_ref s) {
    add(this->_defs, s, ar::getVar(s), ar::getName(ar::getVar(s)));
  }

  void visit(Resume_ref s) {
    add(this->_uses, s, ar::getVar(s), ar::getName(ar::getVar(s)));
  }

  void visit(Unreachable_ref) {}
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
        _live(live) {}

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

  std::string get_name() const { return ar::getName(_block); }

  void reverse() {
    std::swap(_prev_nodes, _next_nodes);
    std::reverse(_stmts.begin(), _stmts.end());
  }

  //! Return the set of used variables at statement s
  varname_set_t uses(Statement_ref s) const { return _live->uses(s); }

  //! Return the set of defined variables at statement s
  varname_set_t defs(Statement_ref s) const { return _live->defs(s); }

  //! Return all the used/defined variables in the basic block
  varname_set_t all_vars() const { return _live->all_vars(); }

  void write(std::ostream& o) {
    o << get_name() << ":" << std::endl;
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
    live_info::live_info_ptr live(new live_info(_vfac, _lfac, _prec_level));

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
            TrackedPrecision level)
      : _nodes_map(new nodes_map_t()),
        _func(func),
        _vfac(vfac),
        _lfac(lfac),
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
    return arbos_cfg(_func, _vfac, _lfac, _prec_level);
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

  boost::optional< Basic_Block_ref > exit() { return this->_exit; }

  VariableFactory& getVarFactory() { return this->_vfac; }

  LiteralFactory& getLitFactory() { return this->_lfac; }

  std::string get_func_name() const { return ar::getName(this->_func); }

  Function_ref get_func() { return this->_func; }

  TrackedPrecision getPrecisionLevel() const { return this->_prec_level; }

  std::vector< Local_Variable_ref >& get_local_variables() {
    return this->_local_vars;
  }

  std::vector< Internal_Variable_ref >& get_internal_variables() {
    return this->_internal_vars;
  }

  std::vector< Internal_Variable_ref >& get_formal_parameters() {
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
    swap(_entry, *_exit);
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
    o << "Function: " << get_func_name() << std::endl;
    o << "entry block: " << _entry << std::endl;
    if (_exit)
      o << "exit block: " << *_exit << std::endl;
    else
      o << "exit block: none\n";
    // Blocks
    {
      for (iterator I = begin(), E = end(); I != E; ++I) {
        arbos_node_t node = *I;
        o << node.get_name() << ":\n";
        for (arbos_node_t::iterator S = node.begin(); S != node.end(); ++S) {
          o << *S << std::endl;
        }
      }
    }
    // control flow structure
    {
      for (iterator I = begin(), E = end(); I != E; ++I) {
        arbos_node_t node = *I;
        o << node.get_name() << " -> [";
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
  typedef std::shared_ptr< arbos_cfg > arbos_cfg_ptr;
  typedef std::unordered_map< Function_ref, arbos_cfg_ptr > cfg_map_t;

  cfg_map_t _map;
  VariableFactory& _vfac;
  LiteralFactory& _lfac;
  const TrackedPrecision _level;

public:
  CfgFactory(VariableFactory& vfac,
             LiteralFactory& lfac,
             TrackedPrecision level)
      : _vfac(vfac), _lfac(lfac), _level(level) {}

  arbos_cfg operator[](Function_ref f) {
    cfg_map_t::iterator it = _map.find(f);
    if (it == _map.end()) {
      arbos_cfg_ptr cfg(new arbos_cfg(f, _vfac, _lfac, _level));
      std::pair< cfg_map_t::iterator, bool > res =
          _map.insert(std::make_pair(f, cfg));
      return *((res.first)->second);
    } else {
      return *(it->second);
    }
  }

  VariableFactory& getVarFactory() const { return _vfac; }

  LiteralFactory& getLitFactory() const { return _lfac; }

  TrackedPrecision getPrecLevel() const { return _level; }
};

} // end namespace analyzer

#endif // ANALYZER_CFG_HPP
