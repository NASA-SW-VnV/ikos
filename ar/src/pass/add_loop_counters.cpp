/*******************************************************************************
 *
 * \file
 * \brief Implementation of AddLoopCountersPass
 *
 * This pass adds a hidden loop counter variable within each loop, using the
 * `ikos.counter.init` and `ikos.counter.incr` intrinsic functions.
 *
 * This can be used by the gauge abstract domain to infer loop invariants.
 *
 * Authors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017-2019 United States Government as represented by the
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

#include <ikos/ar/pass/add_loop_counters.hpp>
#include <ikos/ar/semantic/code.hpp>
#include <ikos/ar/semantic/statement.hpp>
#include <ikos/ar/semantic/value.hpp>

namespace ikos {
namespace ar {

const char* AddLoopCountersPass::name() const {
  return "add-loop-counters";
}

const char* AddLoopCountersPass::description() const {
  return "Add loop counters";
}

bool AddLoopCountersPass::run(Bundle* bundle) {
  // Create the counter intrinsic functions before iterating on codes
  bundle->intrinsic_function(Intrinsic::IkosCounterInit);
  bundle->intrinsic_function(Intrinsic::IkosCounterIncr);

  return CodePass::run(bundle);
}

namespace {

/// \brief Iterate over the components in the control flow graph
class LoopIterator : public core::WtoComponentVisitor< Code* > {
private:
  using WtoVertexT = core::WtoVertex< Code* >;
  using WtoCycleT = core::WtoCycle< Code* >;

private:
  // Code
  Code* _code;

  // List of basic blocks in the current cycle
  std::vector< BasicBlock* > _blocks;

public:
  explicit LoopIterator(Code* code) : _code(code) {}

  void visit(const WtoVertexT& vertex) override {
    this->_blocks.push_back(vertex.node());
  }

  void visit(const WtoCycleT& cycle) override {
    std::vector< BasicBlock* > current_blocks = std::move(this->_blocks);

    // Collect all basic blocks within the cycle
    this->_blocks.clear();
    this->_blocks.push_back(cycle.head());

    for (auto it = cycle.begin(), et = cycle.end(); it != et; ++it) {
      it->accept(*this);
    }

    this->add_loop_counter(cycle, this->_blocks);

    // Update _blocks
    this->_blocks.insert(this->_blocks.end(),
                         current_blocks.begin(),
                         current_blocks.end());
  }

  /// \brief Add a loop counter in the given cycle
  void add_loop_counter(const WtoCycleT& cycle,
                        const std::vector< BasicBlock* >& blocks) {
    Bundle* bundle = this->_code->bundle();
    Context& ctx = bundle->context();

    // Get the intrinsics for loop counters
    Function* counter_init =
        bundle->intrinsic_function(Intrinsic::IkosCounterInit);
    Function* counter_incr =
        bundle->intrinsic_function(Intrinsic::IkosCounterIncr);

    // Create the loop counter variable
    IntegerType* size_ty = IntegerType::size_type(this->_code->bundle());
    InternalVariable* var = InternalVariable::create(this->_code, size_ty);

    // Constants
    IntegerConstant* zero = IntegerConstant::get(ctx, size_ty, 0);
    IntegerConstant* one = IntegerConstant::get(ctx, size_ty, 1);

    // Add initialization statement in parent blocks that aren't in the cycle
    std::size_t blocks_incoming_edge = 0;

    for (BasicBlock* bb : blocks) {
      bool has_incoming_edge = false;

      for (auto it = bb->predecessor_begin(), et = bb->predecessor_end();
           it != et;
           ++it) {
        BasicBlock* pred = *it;

        if (std::find(blocks.begin(), blocks.end(), pred) == blocks.end()) {
          // predecessor is not in the cycle
          has_incoming_edge = true;
          pred->push_back(Call::create(/* result = */ var,
                                       /* function = */ counter_init,
                                       /* arguments = */ {zero}));
        }
      }

      if (has_incoming_edge) {
        blocks_incoming_edge++;
      }
    }

    if (blocks_incoming_edge > 1) {
      // Cycle with multiple blocks having incoming edges from outside of the
      // cycle. This pattern is unusual. We cannot know where to put the
      // increment statement, so just abort silently.
      return;
    }

    // Add an increment statement in incoming blocks of the head that are in the
    // cycle
    BasicBlock* head = cycle.head();
    for (auto it = head->predecessor_begin(), et = head->predecessor_end();
         it != et;
         ++it) {
      BasicBlock* pred = *it;
      if (std::find(blocks.begin(), blocks.end(), pred) != blocks.end()) {
        pred->push_back(Call::create(/* result = */ var,
                                     /* function = */ counter_incr,
                                     /* arguments = */ {var, one}));
      }
    }
  }

}; // end class LoopIterator

} // end anonymous namespace

bool AddLoopCountersPass::run_on_code(Code* code) {
  // Compute the weak topological order
  core::Wto< Code* > wto(code);

  // Add a loop counter in each cycle
  LoopIterator it(code);
  wto.accept(it);

  return true;
}

} // end namespace ar
} // end namespace ikos
