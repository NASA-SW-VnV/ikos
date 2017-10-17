/*******************************************************************************
 *
 * Liveness variable analysis
 *
 * Author: Jorge A. Navas
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
#ifndef ANALYZER_LIVENESS_HPP
#define ANALYZER_LIVENESS_HPP

#include <algorithm>

#include <ikos/domains/dataflow_domain.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/ar-wrapper/cfg.hpp>
#include <analyzer/ikos-wrapper/iterators.hpp>

namespace analyzer {
namespace liveness_set_impl {

//! Lattice of liveness using std::set
template < typename VariableName >
class liveness_domain : public ikos::abstract_domain {
private:
  typedef liveness_domain< VariableName > liveness_domain_t;
  typedef std::set< VariableName > ElemVector;

public:
  typedef typename ElemVector::iterator iterator;
  typedef typename ElemVector::const_iterator const_iterator;

private:
  bool _is_top;
  ElemVector _inv;

  liveness_domain(ElemVector inv, bool is_top) : _is_top(is_top), _inv(inv) {
    if (this->_is_top)
      _inv.clear();
  }

public:
  static liveness_domain_t top() { return liveness_domain(ElemVector(), true); }

  static liveness_domain_t bottom() {
    return liveness_domain(ElemVector(), false);
  }

  liveness_domain() : _is_top(false) {}

  liveness_domain(VariableName e) : _is_top(false) { this->_inv.insert(e); }

  ~liveness_domain() { this->_inv.clear(); }

  liveness_domain(const liveness_domain_t& other) : _is_top(other._is_top) {
    if (!is_top()) {
      std::copy(other._inv.begin(),
                other._inv.end(),
                std::inserter(this->_inv, this->_inv.end()));
    }
  }

  liveness_domain_t& operator=(liveness_domain_t other) {
    this->_is_top = other._is_top;
    if (this->_is_top)
      this->_inv.clear();
    else
      this->_inv.swap(other._inv);
    return *this;
  }

  iterator begin() { return this->_inv.begin(); }
  iterator end() { return this->_inv.end(); }
  const_iterator begin() const { return this->_inv.begin(); }
  const_iterator end() const { return this->_inv.end(); }

  unsigned size() const { return this->_inv.size(); }

  bool is_top() { return this->_is_top; }

  bool is_bottom() { return (!is_top() && (size() == 0)); }

  bool operator<=(liveness_domain_t other) {
    if (this->is_bottom() || other.is_top())
      return true;
    if (this->is_top() || other.is_bottom())
      return false;
    return std::includes(this->_inv.begin(),
                         this->_inv.end(),
                         other._inv.begin(),
                         other._inv.end());
  }

  void operator-=(liveness_domain_t other) {
    if (this->size() == 0 || other.size() == 0)
      return;
    if (this->is_top() || other.is_top())
      return;
    for (iterator it = other.begin(), et = other.end(); it != et; ++it)
      this->_inv.erase(*it);
    // this->_inv.erase(other._inv.begin (), other._inv.end ());
  }

  void operator+=(liveness_domain_t other) {
    if (is_top())
      return;
    if (other.is_top()) {
      this->_is_top = true;
      this->_inv.clear();
    } else
      this->_inv.insert(other._inv.begin(), other._inv.end());
  }

  liveness_domain_t operator|(liveness_domain_t other) {
    if (this->is_top() || other.is_top())
      return liveness_domain_t::top();

    if (this->is_bottom())
      return other;
    else if (other.is_bottom())
      return *this;
    else {
      ElemVector s(this->_inv);
      s.insert(other._inv.begin(), other._inv.end());
      return liveness_domain_t(s, false);
    }
  }

  liveness_domain_t operator||(liveness_domain_t other) {
    return this->operator|(other);
  }

  liveness_domain_t join_loop(liveness_domain_t other) {
    return this->operator|(other);
  }

  liveness_domain_t join_iter(liveness_domain_t other) {
    return this->operator|(other);
  }

  liveness_domain_t operator&(liveness_domain_t other) {
    if (this->is_bottom() || other.is_bottom())
      return liveness_domain_t::bottom();

    if (this->is_top())
      return other;
    else if (other.is_top())
      return *this;
    else {
      ElemVector s;
      std::set_intersection(this->_inv.begin(),
                            this->_inv.end(),
                            other._inv.begin(),
                            other._inv.end(),
                            std::inserter(s, s.end()));
      return liveness_domain_t(s, false);
    }
  }

  liveness_domain_t operator&&(liveness_domain_t other) {
    return this->operator&(other);
  }

  void write(std::ostream& o) {
    if (is_top())
      o << "{...}";
    else if (is_bottom())
      o << "_|_";
    else {
      o << "{";
      for (iterator it = begin(); it != end();) {
        o << (*it)->str();
        ++it;
        if (it != end())
          o << "; ";
      }
      o << "}";
    }
  }

  static std::string domain_name() {
    return "Liveness (std::set implementation)";
  }

}; // end class liveness_domain

} // end namespace liveness_set_impl

namespace liveness_discrete_impl {

//! Lattice of liveness using dataflow_domain
template < typename VariableName >
class liveness_domain : public ikos::abstract_domain {
private:
  typedef ikos::dataflow_domain< VariableName > dataflow_domain_t;

public:
  typedef typename dataflow_domain_t::iterator iterator;

private:
  typedef liveness_domain< VariableName > liveness_domain_t;

  dataflow_domain_t _inv;

  liveness_domain(dataflow_domain_t inv) : _inv(inv) {}

public:
  static liveness_domain_t top() {
    return liveness_domain(dataflow_domain_t::top());
  }

  static liveness_domain_t bottom() {
    return liveness_domain(dataflow_domain_t::bottom());
  }

  liveness_domain() : _inv(dataflow_domain_t::bottom()) {}

  liveness_domain(VariableName v) : _inv(v) {}

  liveness_domain(const liveness_domain_t& other) : _inv(other._inv) {}

  liveness_domain_t& operator=(liveness_domain_t other) {
    this->_inv = other._inv;
    return *this;
  }

  iterator begin() { return this->_inv.begin(); }
  iterator end() { return this->_inv.end(); }

  unsigned size() { return this->_inv.size(); }

  bool is_bottom() { return this->_inv.is_bottom(); }

  bool is_top() { return this->_inv.is_top(); }

  bool operator<=(liveness_domain_t other) {
    return (this->_inv <= other._inv);
  }

  void operator-=(liveness_domain_t x) { this->_inv -= x._inv; }

  void operator+=(liveness_domain_t x) { this->_inv |= x._inv; }

  liveness_domain_t operator|(liveness_domain_t other) {
    return liveness_domain_t(this->_inv | other._inv);
  }

  liveness_domain_t operator||(liveness_domain_t other) {
    return this->operator|(other);
  }

  liveness_domain_t join_loop(liveness_domain_t other) {
    return this->operator|(other);
  }

  liveness_domain_t join_iter(liveness_domain_t other) {
    return this->operator|(other);
  }

  liveness_domain_t operator&(liveness_domain_t other) {
    return liveness_domain_t(this->_inv & other._inv);
  }

  liveness_domain_t operator&&(liveness_domain_t other) {
    return this->operator&(other);
  }

  void write(std::ostream& o) { this->_inv.write(o); }

  static std::string domain_name() {
    return "Liveness (ikos::patricia_tree implementation)";
  }

}; // end class liveness_domain

} // end namespace liveness_discrete_impl

//! Compute liveness for an arbos cfg
class Liveness : public backward_fixpoint_iterator<
                     Basic_Block_ref,
                     arbos_cfg,
                     liveness_discrete_impl::liveness_domain< varname_t > > {
public:
  typedef liveness_discrete_impl::liveness_domain< varname_t >
      liveness_domain_t;

private:
  friend class LivenessPass;

  typedef std::unordered_map< Basic_Block_ref, varname_set_t > varset_map_t;
  typedef backward_fixpoint_iterator< Basic_Block_ref,
                                      arbos_cfg,
                                      liveness_domain_t >
      backward_fixpoint_iterator_t;
  typedef std::pair< liveness_domain_t, liveness_domain_t > kill_gen_t;
  typedef std::unordered_map< Basic_Block_ref, kill_gen_t > kill_gen_map_t;

public:
  typedef varset_map_t::const_iterator live_const_iterator;
  typedef varset_map_t::const_iterator dead_const_iterator;

private:
  varset_map_t _live_map; //! live set
  varset_map_t _dead_map; //! dead set
  kill_gen_map_t _kg_map; //! to solve efficiently kill-set equations

public:
  Liveness(arbos_cfg cfg) : backward_fixpoint_iterator_t(cfg, true) {
    init(cfg);
  }

  live_const_iterator live_begin() const { return _live_map.begin(); }
  live_const_iterator live_end() const { return _live_map.end(); }

  dead_const_iterator dead_begin() const { return _dead_map.begin(); }
  dead_const_iterator dead_end() const { return _dead_map.end(); }

private:
  //! Compute kill/gen sets for each basic block
  void init(arbos_cfg cfg) {
    typedef arbos_cfg::iterator cfg_iterator;
    typedef arbos_cfg::arbos_node_t::iterator node_iterator;

    std::pair< cfg_iterator, cfg_iterator > nodes = cfg.get_nodes();
    for (cfg_iterator it = nodes.first, et = nodes.second; it != et; ++it) {
      liveness_domain_t kill, gen;
      arbos_cfg::arbos_node_t node = *it;
      for (node_iterator s = node.begin(); s != node.end(); ++s) {
        varname_set_t defs = node.defs(*s);
        varname_set_t uses = node.uses(*s);
        for (varname_set_t::iterator d = defs.begin(); d != defs.end(); ++d) {
          kill += *d;
          gen -= *d;
        }
        for (varname_set_t::iterator u = uses.begin(); u != uses.end(); ++u) {
          gen += *u;
        }
      }
      _kg_map.insert(
          kill_gen_map_t::value_type(node.get(), kill_gen_t(kill, gen)));
    }
  }

  void run(liveness_domain_t inv) {
    try {
      backward_fixpoint_iterator_t::run(inv);
    } catch (cfg_not_reversible_error&) {
      // no exit node
    }

    // some basic blocks might not have a live/dead set:
    // * if the control flow graph has no exit node
    // * if the control flow graph has dead cycles
    varname_set_t all_vars;
    for (auto it = this->get_cfg().begin(); it != this->get_cfg().end(); ++it) {
      Basic_Block_ref bb = it->get();
      if (_live_map.find(bb) == _live_map.end()) {
        if (all_vars.empty()) { // fill all_vars
          all_vars = this->vars();
        }
        _live_map.insert(varset_map_t::value_type(bb, all_vars));
        _dead_map.insert(varset_map_t::value_type(bb, varname_set_t()));
      }
    }
  }

  //! collect all variables in the control flow graph
  varname_set_t vars() {
    varname_set_t all_vars;
    for (auto it = this->get_cfg().begin(); it != this->get_cfg().end(); ++it) {
      all_vars |= it->all_vars();
    }
    return all_vars;
  }

  //! apply the kill-gen equation: IN(B) = (OUT(B) \ kill (B)) U gen (B)
  liveness_domain_t analyze(Basic_Block_ref bb, liveness_domain_t post) {
    kill_gen_map_t::iterator it = _kg_map.find(bb);
    assert(it != _kg_map.end() && "No found kill/gen sets for block");

    liveness_domain_t pre(post);
    pre -= it->second.first;  // delete kill set
    pre += it->second.second; // add gen set
    return pre;
  }

  void storeLiveSet(Basic_Block_ref bb, liveness_domain_t inv) {
    varname_set_t live;
    assert(!inv.is_top());
    if (!inv.is_bottom()) {
      for (auto it = inv.begin(); it != inv.end(); ++it) {
        live += *it;
      }
    }
    _live_map.insert(varset_map_t::value_type(bb, live));
  }

  void storeDeadSet(Basic_Block_ref bb, liveness_domain_t inv) {
    varname_set_t dead;
    assert(!inv.is_top());
    if (!inv.is_bottom()) {
      arbos_cfg::arbos_node_t node = this->get_cfg().get_node(bb);
      varname_set_t allVars = node.all_vars();
      for (auto it = allVars.begin(); it != allVars.end(); ++it) {
        if (!(liveness_domain_t(*it) <= inv)) {
          dead += *it;
        }
      }
    }
    _dead_map.insert(varset_map_t::value_type(bb, dead));
  }

  void markAlive(varname_t v) {
    for (auto it = _live_map.begin(); it != _live_map.end(); ++it) {
      it->second += v;
    }
    for (auto it = _dead_map.begin(); it != _dead_map.end(); ++it) {
      it->second -= v;
    }
  }

  //! store set of live variables at the entry of bb
  void check_pre(Basic_Block_ref bb, liveness_domain_t pre) {
    storeLiveSet(bb, pre);
  }

  //! store set of dead variables at the exit of bb
  void check_post(Basic_Block_ref bb, liveness_domain_t post) {
    storeDeadSet(bb, post);
  }
};

//! Compute liveness for a whole bundle (module)
class LivenessPass {
  typedef std::shared_ptr< Liveness > liveness_ptr;
  typedef std::unordered_map< std::string, liveness_ptr > map_t;
  typedef std::shared_ptr< map_t > map_ptr;

public:
  typedef map_t::iterator iterator;
  typedef map_t::const_iterator const_iterator;

private:
  CfgFactory& _cfg_fac;
  map_ptr _map;

public:
  LivenessPass(CfgFactory& cfg_fac) : _cfg_fac(cfg_fac), _map(new map_t()) {}

  // Note: the default copy constructor will make a shallow copy. This
  // is intended.

  iterator begin() { return _map->begin(); }
  iterator end() { return _map->end(); }
  const_iterator begin() const { return _map->begin(); }
  const_iterator end() const { return _map->end(); }

  void execute(Bundle_ref bundle) {
    FuncRange entries = ar::getFunctions(bundle);
    for (FuncRange::iterator I = entries.begin(), E = entries.end(); I != E;
         ++I) {
      arbos_cfg cfg = _cfg_fac[*I];
      // note that we clone cfg because Liveness will reverse it.
      liveness_ptr live(new Liveness(cfg.clone()));
      live->run(Liveness::liveness_domain_t::bottom());
      _map->insert(std::make_pair(cfg.func_name(), live));
    }
  }

  //! return for a given function f and basic block bb the set of live
  //! variables at the beginning of bb.
  varname_set_t liveSet(const std::string& f, Basic_Block_ref bb) const {
    const_iterator it = _map->find(f);
    if (it != _map->end()) {
      liveness_ptr live = it->second;
      Liveness::live_const_iterator bb_it = live->_live_map.find(bb);
      if (bb_it != live->_live_map.end()) {
        return bb_it->second;
      }
    }
    throw analyzer_error(
        "liveness_pass::liveSet(): could not get liveness information");
  }

  //! return for a given function f and basic block bb the set of dead
  //! variables at the exit of bb.
  varname_set_t deadSet(const std::string& f, Basic_Block_ref bb) const {
    const_iterator it = _map->find(f);
    if (it != _map->end()) {
      liveness_ptr live = it->second;
      Liveness::dead_const_iterator bb_it = live->_dead_map.find(bb);
      if (bb_it != live->_dead_map.end()) {
        return bb_it->second;
      }
    }
    throw analyzer_error(
        "liveness_pass::deadSet(): could not get liveness information");
  }

  void markAlive(const std::string& f, varname_t v) {
    iterator it = _map->find(f);
    if (it != _map->end()) {
      liveness_ptr live = it->second;
      live->markAlive(v);
    }
  }

  std::ostream& dump(std::ostream& o) {
    for (auto F = begin(); F != end(); ++F) {
      o << "function " << F->first << std::endl;
      std::shared_ptr< Liveness > L = F->second;
      for (auto B = L->live_begin(); B != L->live_end(); ++B) {
        o << ar::getName(B->first) << " : live={";
        for (auto V = B->second.begin(); V != B->second.end(); ++V) {
          o << (*V)->str() << ";";
        }
        o << "}\n";
      }
      for (auto B = L->dead_begin(); B != L->dead_end(); ++B) {
        o << ar::getName(B->first) << " : dead={";
        for (auto V = B->second.begin(); V != B->second.end(); ++V) {
          o << (*V)->str() << ";";
        }
        o << "}\n";
      }
    }
    return o;
  }

}; // end class LivenessPass

inline std::ostream& operator<<(std::ostream& o, LivenessPass LP) {
  return LP.dump(o);
}

} // end namespace analyzer

#endif // ANALYZER_LIVENESS_HPP
