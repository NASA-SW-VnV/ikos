/*******************************************************************************
 *
 * \file
 * \brief Widening hint analysis implementation
 *
 * Author: Thomas Bailleux
 *
 * Contributor: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2018-2019 United States Government as represented by the
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

#include <ikos/core/fixpoint/wto.hpp>

#include <ikos/ar/semantic/statement.hpp>

#include <ikos/analyzer/analysis/fixpoint_parameters.hpp>
#include <ikos/analyzer/analysis/widening_hint.hpp>
#include <ikos/analyzer/support/cast.hpp>
#include <ikos/analyzer/util/demangle.hpp>
#include <ikos/analyzer/util/progress.hpp>

namespace ikos {
namespace analyzer {

namespace {

/// \brief Weak topological visitor to find widening hints
class WideningHintWtoVisitor : public core::WtoComponentVisitor< ar::Code* > {
private:
  using WtoVertexT = core::WtoVertex< ar::Code* >;
  using WtoCycleT = core::WtoCycle< ar::Code* >;

private:
  WideningHints& _hints;

public:
  /// \brief Constructor
  explicit WideningHintWtoVisitor(WideningHints& hints) : _hints(hints) {}

  /// \brief No copy constructor
  WideningHintWtoVisitor(const WideningHintWtoVisitor&) = delete;

  /// \brief No move constructor
  WideningHintWtoVisitor(WideningHintWtoVisitor&&) = delete;

  /// \brief No copy assignment operator
  WideningHintWtoVisitor& operator=(const WideningHintWtoVisitor&) = delete;

  /// \brief No move assignment operator
  WideningHintWtoVisitor& operator=(WideningHintWtoVisitor&&) = delete;

  /// \brief Destructor
  ~WideningHintWtoVisitor() override = default;

  void visit(const WtoVertexT&) override {}

  void visit(const WtoCycleT& cycle) override {
    auto head = cycle.head();

    if (head->num_successors() > 1) {
      auto successor = *(head->successor_begin());
      if (auto constant = this->extract_constant(successor)) {
        this->_hints.add(head, *constant);
      }
    }

    for (auto it = cycle.begin(), et = cycle.end(); it != et; ++it) {
      it->accept(*this);
    }
  }

  boost::optional< ar::MachineInt > extract_constant(ar::BasicBlock* bb) const {
    if (bb->empty()) {
      return boost::none;
    }

    // we have a comparison, check if there is a constant
    if (auto cmp = dyn_cast< ar::Comparison >(bb->front())) {
      ar::IntegerConstant* constant = nullptr;
      bool cst_left;

      if (cmp->left()->is_integer_constant()) {
        constant = cast< ar::IntegerConstant >(cmp->left());
        cst_left = true;
      } else if (cmp->right()->is_integer_constant()) {
        constant = cast< ar::IntegerConstant >(cmp->right());
        cst_left = false;
      } else {
        return boost::none;
      }

      ar::MachineInt value = constant->value();
      ar::MachineInt one(1, value.bit_width(), value.sign());
      bool overflow = false;

      // check if the comparison is <= or >=
      if (cmp->predicate() == ar::Comparison::UIGE ||
          cmp->predicate() == ar::Comparison::SIGE) {
        if (cst_left) {
          // case `cst >= var` <=> `cst + 1 > var`
          value = add(value, one, overflow);
        } else {
          // case `var >= cst` <=> `var > cst - 1`
          value = sub(value, one, overflow);
        }
      } else if (cmp->predicate() == ar::Comparison::UILE ||
                 cmp->predicate() == ar::Comparison::SILE) {
        if (cst_left) {
          // case `cst <= var` <=> `cst - 1 < var`
          value = sub(value, one, overflow);
        } else {
          // case `var <= cst` <=> `var < cst + 1`
          value = add(value, one, overflow);
        }
      }
      if (overflow) {
        return boost::none;
      }
      return std::move(value);
    } else {
      return boost::none;
    }
  }

}; // end class WideningHintWtoVisitor

} // end anonymous namespace

WideningHintAnalysis::WideningHintAnalysis(Context& ctx) : _ctx(ctx) {}

WideningHintAnalysis::~WideningHintAnalysis() = default;

void WideningHintAnalysis::run() {
  ar::Bundle* bundle = this->_ctx.bundle;

  // Setup a progress logger
  std::unique_ptr< ProgressLogger > progress =
      make_progress_logger(_ctx.opts.progress,
                           LogLevel::Info,
                           /* num_tasks = */
                           std::count_if(bundle->function_begin(),
                                         bundle->function_end(),
                                         [](ar::Function* fun) {
                                           return fun->is_definition();
                                         }));
  ScopeLogger scope(*progress);

  for (auto it = bundle->function_begin(), et = bundle->function_end();
       it != et;
       ++it) {
    ar::Function* fun = *it;
    if (fun->is_definition()) {
      progress->start_task("Running widening hint analysis on function '" +
                           demangle(fun->name()) + "'");
      this->run(fun);
    }
  }
}

void WideningHintAnalysis::run(ar::Function* fun) {
  if (!fun->is_definition()) {
    return;
  }

  CodeFixpointParameters& parameters = this->_ctx.fixpoint_parameters->get(fun);
  WideningHintWtoVisitor visitor(parameters.widening_hints);
  core::Wto< ar::Code* > wto(fun->body());
  wto.accept(visitor);
}

} // end namespace analyzer
} // end namespace ikos
