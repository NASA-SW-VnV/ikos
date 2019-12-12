/*******************************************************************************
 *
 * \file
 * \brief Implementation of the liveness analysis
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
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

#include <ikos/core/domain/discrete_domain.hpp>
#include <ikos/core/fixpoint/fwd_fixpoint_iterator.hpp>

#include <ikos/ar/semantic/code.hpp>

#include <ikos/analyzer/analysis/liveness.hpp>
#include <ikos/analyzer/analysis/variable.hpp>
#include <ikos/analyzer/util/demangle.hpp>
#include <ikos/analyzer/util/log.hpp>
#include <ikos/analyzer/util/progress.hpp>

namespace ikos {
namespace analyzer {
namespace {

/// \brief Lattice of liveness
///
/// This is a small wrapper around core::DiscreteDomain, with a few changes:
///   * The default constructor returns bottom
///   * Has convenient operator+ and operator-
template < typename VariableRef >
class LivenessDomain final
    : public core::AbstractDomain< LivenessDomain< VariableRef > > {
private:
  using DiscreteDomain = core::DiscreteDomain< VariableRef >;

public:
  using Iterator = typename DiscreteDomain::Iterator;

private:
  DiscreteDomain _inv;

private:
  /// \brief Private constructor
  explicit LivenessDomain(DiscreteDomain inv) : _inv(std::move(inv)) {}

public:
  /// \brief Create the top liveness domain
  static LivenessDomain top() { return LivenessDomain(DiscreteDomain::top()); }

  /// \brief Create the bottom liveness domain
  static LivenessDomain bottom() {
    return LivenessDomain(DiscreteDomain::bottom());
  }

  /// \brief Create the empty liveness domain
  static LivenessDomain empty() {
    return LivenessDomain(DiscreteDomain::bottom());
  }

  /// \brief Create the liveness domain with the given elements
  LivenessDomain(std::initializer_list< VariableRef > elements)
      : _inv(elements) {}

  /// \brief Copy constructor
  LivenessDomain(const LivenessDomain&) noexcept = default;

  /// \brief Move constructor
  LivenessDomain(LivenessDomain&&) noexcept = default;

  /// \brief Copy assignment operator
  LivenessDomain& operator=(const LivenessDomain&) noexcept = default;

  /// \brief Move assignment operator
  LivenessDomain& operator=(LivenessDomain&&) noexcept = default;

  /// \brief Destructor
  ~LivenessDomain() override = default;

  /// \brief Return the number of elements in the liveness domain
  std::size_t size() const { return this->_inv.size(); }

  /// \brief Begin iterator over the elements
  Iterator begin() const { return this->_inv.begin(); }

  /// \brief End iterator over the elements
  Iterator end() const { return this->_inv.end(); }

  void normalize() override {}

  bool is_bottom() const override { return this->_inv.is_bottom(); }

  bool is_top() const override { return this->_inv.is_top(); }

  void set_to_bottom() override { this->_inv.set_to_bottom(); }

  void set_to_top() override { this->_inv.set_to_top(); }

  bool leq(const LivenessDomain& other) const override {
    return this->_inv.leq(other._inv);
  }

  bool equals(const LivenessDomain& other) const override {
    return this->_inv.equals(other._inv);
  }

  void join_with(const LivenessDomain& other) override {
    this->_inv.join_with(other._inv);
  }

  void widen_with(const LivenessDomain& other) override {
    this->_inv.widen_with(other._inv);
  }

  void meet_with(const LivenessDomain& other) override {
    this->_inv.meet_with(other._inv);
  }

  void narrow_with(const LivenessDomain& other) override {
    this->_inv.narrow_with(other._inv);
  }

  void operator-=(const LivenessDomain& other) {
    this->_inv.difference_with(other._inv);
  }

  void operator-=(VariableRef v) { this->_inv.remove(v); }

  void operator+=(const LivenessDomain& other) {
    this->_inv.join_with(other._inv);
  }

  void operator+=(VariableRef v) { this->_inv.add(v); }

  void dump(std::ostream& o) const override { this->_inv.dump(o); }

  static std::string name() { return "liveness domain"; }

}; // end class LivenessDomain

/// \brief Inverse an ar::Code* graph
///
/// The entry node is the exit node, and successors and predecessors are swapped
struct ReverseCodeGraphTrait {
  using NodeRef = ar::BasicBlock*;
  using SuccessorNodeIterator = ar::BasicBlock::BasicBlockIterator;
  using PredecessorNodeIterator = ar::BasicBlock::BasicBlockIterator;

  static ar::BasicBlock* entry(ar::Code* code) {
    ikos_assert_msg(code->has_exit_block(), "code graph cannot be inversed");
    return code->exit_block();
  }

  static SuccessorNodeIterator successor_begin(ar::BasicBlock* bb) {
    return bb->predecessor_begin();
  }

  static SuccessorNodeIterator successor_end(ar::BasicBlock* bb) {
    return bb->predecessor_end();
  }

  static PredecessorNodeIterator predecessor_begin(ar::BasicBlock* bb) {
    return bb->successor_begin();
  }

  static PredecessorNodeIterator predecessor_end(ar::BasicBlock* bb) {
    return bb->successor_end();
  }

}; // end struct ReverseCodeGraphTrait

/// \brief Liveness fixpoint iterator
class LivenessFixpointIterator final
    : public core::InterleavedFwdFixpointIterator< ar::Code*,
                                                   LivenessDomain< Variable* >,
                                                   ReverseCodeGraphTrait > {
private:
  /// \brief Parent class
  using FwdFixpointIterator =
      core::InterleavedFwdFixpointIterator< ar::Code*,
                                            LivenessDomain< Variable* >,
                                            ReverseCodeGraphTrait >;

  /// \brief Liveness abstract domain
  using LivenessDomainT = LivenessDomain< Variable* >;

  /// \brief A pair (kill, gen)
  struct KillGen {
    LivenessDomainT kill;
    LivenessDomainT gen;
  };

  /// \brief Map from basic block to a (kill, gen) pair
  using KillGenMap = llvm::DenseMap< ar::BasicBlock*, KillGen >;

  /// \brief Map from basic block to liveness domain
  using LivenessMap = llvm::DenseMap< ar::BasicBlock*, LivenessDomainT >;

public:
  /// \brief Iterator on a map from basic block to liveness domain
  using LivenessMapIterator = LivenessMap::const_iterator;

private:
  /// \brief Variable factory
  VariableFactory& _vfac;

  /// \brief Store (kill, gen) sets for each basic block
  KillGenMap _kg_map;

  /// \brief Set of all variables (defined and used) for each basic blocks
  LivenessMap _all_vars_map;

  /// \brief Set of live variables at the **entry** of a basic block
  LivenessMap _live_at_entry_map;

  /// \brief Set of dead variables at the **end** of a basic block
  LivenessMap _dead_at_end_map;

public:
  LivenessFixpointIterator(ar::Code* code, VariableFactory& vfac)
      : FwdFixpointIterator(code, LivenessDomainT::bottom()), _vfac(vfac) {
    this->init();
  }

  LivenessMapIterator live_at_entry_begin() const {
    return this->_live_at_entry_map.begin();
  }

  LivenessMapIterator live_at_entry_end() const {
    return this->_live_at_entry_map.end();
  }

  LivenessMapIterator dead_at_end_begin() const {
    return this->_dead_at_end_map.begin();
  }

  LivenessMapIterator dead_at_end_end() const {
    return this->_dead_at_end_map.end();
  }

  /// \brief Compute the set of dead and live variables
  void run(LivenessDomainT inv) override {
    FwdFixpointIterator::run(std::move(inv));
  }

  /// \brief Apply the kill-gen equation
  ///
  /// IN(B) = (OUT(B) \ kill (B)) U gen (B)
  LivenessDomainT analyze_node(ar::BasicBlock* bb,
                               LivenessDomainT post) override {
    auto it = this->_kg_map.find(bb);
    ikos_assert(it != this->_kg_map.end());

    LivenessDomainT pre(post);
    pre -= it->second.kill; // delete kill set
    pre += it->second.gen;  // add gen set
    return pre;
  }

  LivenessDomainT analyze_edge(ar::BasicBlock*,
                               ar::BasicBlock*,
                               LivenessDomainT pre) override {
    return pre;
  }

  /// \brief Store the set of dead variables at the exit of `bb`
  ///
  /// Note: this is the post invariant, because we reversed the graph
  void process_pre(ar::BasicBlock* bb,
                   const LivenessDomainT& post_live) override {
    ikos_assert(!post_live.is_top());

    auto it = this->_all_vars_map.find(bb);
    ikos_assert(it != this->_all_vars_map.end());
    LivenessDomainT all = it->second;

    // dead = all - live
    LivenessDomainT dead(all);
    dead -= post_live;

    this->_dead_at_end_map.try_emplace(bb, dead);
  }

  /// \brief Store the set of live variables at the entry of `bb`
  ///
  /// Note: this is the pre invariant, because we reversed the graph
  void process_post(ar::BasicBlock* bb,
                    const LivenessDomainT& pre_live) override {
    ikos_assert(!pre_live.is_top());

    this->_live_at_entry_map.try_emplace(bb, pre_live);
  }

private:
  /// \brief Compute kill/gen sets for each basic blocks
  void init() {
    ar::Code* code = this->cfg();

    for (auto it = code->begin(), et = code->end(); it != et; ++it) {
      this->init(*it);
    }
  }

  /// \brief Compute kill/gen sets for the given basic block
  void init(ar::BasicBlock* bb) {
    auto kill = LivenessDomainT::empty();
    auto gen = LivenessDomainT::empty();
    auto all = LivenessDomainT::empty();

    for (auto it = bb->rbegin(), et = bb->rend(); it != et; ++it) {
      ar::Statement* stmt = *it;

      // Process defs
      if (stmt->has_result()) {
        Variable* var = this->variable_ref(stmt->result());
        ikos_assert_msg(var != nullptr, "result is not a variable");

        kill += var;
        gen -= var;
        all += var;
      }

      // Process uses
      for (auto op_it = stmt->op_begin(), op_et = stmt->op_end();
           op_it != op_et;
           ++op_it) {
        Variable* var = this->variable_ref(*op_it);
        if (var != nullptr) {
          gen += var;
          all += var;
        }
      }
    }

    this->_kg_map.try_emplace(bb, KillGen{kill, gen});
    this->_all_vars_map.try_emplace(bb, all);
  }

  /// \brief Get the Variable* of an ar::Value
  ///
  /// Returns nullptr if the value is not an internal or local variable
  Variable* variable_ref(ar::Value* value) {
    if (auto ptr = ar::dyn_cast< ar::FunctionPointerConstant >(value)) {
      return this->_vfac.get_function_ptr(ptr);
    } else if (auto gv = ar::dyn_cast< ar::GlobalVariable >(value)) {
      return this->_vfac.get_global(gv);
    } else if (auto lv = ar::dyn_cast< ar::LocalVariable >(value)) {
      return this->_vfac.get_local(lv);
    } else if (auto iv = ar::dyn_cast< ar::InternalVariable >(value)) {
      return this->_vfac.get_internal(iv);
    } else {
      return nullptr;
    }
  }

}; // end class LivenessFixpointIterator

} // end anonymous namespace

LivenessAnalysis::LivenessAnalysis(Context& ctx) : _ctx(ctx) {}

LivenessAnalysis::~LivenessAnalysis() = default;

boost::optional< const LivenessAnalysis::VariableRefList& > LivenessAnalysis::
    live_at_entry(ar::BasicBlock* bb) const {
  auto it = this->_live_at_entry_map.find(bb);
  if (it != this->_live_at_entry_map.end()) {
    return it->second;
  } else {
    return boost::none;
  }
}

boost::optional< const LivenessAnalysis::VariableRefList& > LivenessAnalysis::
    dead_at_end(ar::BasicBlock* bb) const {
  auto it = this->_dead_at_end_map.find(bb);
  if (it != this->_dead_at_end_map.end()) {
    return it->second;
  } else {
    return boost::none;
  }
}

void LivenessAnalysis::run() {
  ar::Bundle* bundle = _ctx.bundle;

  // Setup a progress logger
  std::unique_ptr< ProgressLogger > progress =
      make_progress_logger(_ctx.opts.progress,
                           LogLevel::Info,
                           /* num_tasks = */
                           std::count_if(bundle->global_begin(),
                                         bundle->global_end(),
                                         [](ar::GlobalVariable* gv) {
                                           return gv->is_definition();
                                         }) +
                               std::count_if(bundle->function_begin(),
                                             bundle->function_end(),
                                             [](ar::Function* fun) {
                                               return fun->is_definition();
                                             }));
  ScopeLogger scope(*progress);

  for (auto it = bundle->global_begin(), et = bundle->global_end(); it != et;
       ++it) {
    ar::GlobalVariable* gv = *it;
    if (gv->is_definition()) {
      progress->start_task(
          "Running liveness analysis on initializer of global variable '" +
          demangle(gv->name()) + "'");
      this->run(gv->initializer());
    }
  }

  for (auto it = bundle->function_begin(), et = bundle->function_end();
       it != et;
       ++it) {
    ar::Function* fun = *it;
    if (fun->is_definition()) {
      progress->start_task("Running liveness analysis on function '" +
                           demangle(fun->name()) + "'");
      this->run(fun->body());
    }
  }
}

/// \brief Convert a LivenessDomain into a VariableRefList
static LivenessAnalysis::VariableRefList to_variable_ref_list(
    const LivenessDomain< Variable* >& value) {
  LivenessAnalysis::VariableRefList list;

  if (!value.is_bottom()) {
    list.reserve(value.size());
    std::copy(value.begin(), value.end(), std::back_inserter(list));
  }

  return list;
}

void LivenessAnalysis::run(ar::Code* code) {
  // If the code has no exit block, do nothing
  if (!code->has_exit_block()) {
    return;
  }

  // Run the liveness fixpoint iterator
  LivenessFixpointIterator fixpoint(code, *_ctx.var_factory);
  fixpoint.run(LivenessDomain< Variable* >::bottom());

  // Store the results
  for (auto it = fixpoint.live_at_entry_begin(),
            et = fixpoint.live_at_entry_end();
       it != et;
       ++it) {
    this->_live_at_entry_map.try_emplace(it->first,
                                         to_variable_ref_list(it->second));
  }
  for (auto it = fixpoint.dead_at_end_begin(), et = fixpoint.dead_at_end_end();
       it != et;
       ++it) {
    this->_dead_at_end_map.try_emplace(it->first,
                                       to_variable_ref_list(it->second));
  }
}

void LivenessAnalysis::dump(std::ostream& o) const {
  ar::Bundle* bundle = _ctx.bundle;

  for (auto it = bundle->global_begin(), et = bundle->global_end(); it != et;
       ++it) {
    ar::GlobalVariable* gv = *it;
    if (gv->is_definition()) {
      o << "Liveness analysis results for initializer of global variable @"
        << gv->name() << ":\n";
      this->dump(o, gv->initializer());
    }
  }

  for (auto it = bundle->function_begin(), et = bundle->function_end();
       it != et;
       ++it) {
    ar::Function* fun = *it;
    if (fun->is_definition()) {
      o << "Liveness analysis results for function @" << fun->name() << ":\n";
      this->dump(o, fun->body());
    }
  }
}

void LivenessAnalysis::dump(std::ostream& o, ar::Code* code) const {
  for (ar::BasicBlock* bb : *code) {
    // Live at entry
    o << "live_at_entry(";
    bb->dump(o);
    o << ") = ";
    auto it = this->_live_at_entry_map.find(bb);
    if (it != this->_live_at_entry_map.end()) {
      dump(o, it->second);
    } else {
      o << "none";
    }
    o << "\n";

    // Dead at end
    o << "dead_at_end(";
    bb->dump(o);
    o << ") = ";
    it = this->_dead_at_end_map.find(bb);
    if (it != this->_dead_at_end_map.end()) {
      dump(o, it->second);
    } else {
      o << "none";
    }
    o << "\n";
  }
}

void LivenessAnalysis::dump(std::ostream& o,
                            const LivenessAnalysis::VariableRefList& vars) {
  o << "{";
  for (auto it = vars.begin(), et = vars.end(); it != et;) {
    (*it)->dump(o);
    if (++it != et) {
      o << ", ";
    }
  }
  o << "}";
}

} // end namespace analyzer
} // end namespace ikos
