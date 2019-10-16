/*******************************************************************************
 *
 * \file
 * \brief Implementation of AddPartitioningVariablesPass
 *
 * This pass adds annotations to functions that return an error code. It
 * detects integer variables containing the error code and adds calls to the
 * `ikos.partitioning.var.*` intrinsic functions.
 *
 * Authors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2019 United States Government as represented by the
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

#include <unordered_set>

#include <ikos/ar/pass/add_partitioning_variables.hpp>
#include <ikos/ar/semantic/statement.hpp>
#include <ikos/ar/semantic/type.hpp>
#include <ikos/ar/semantic/value.hpp>

namespace ikos {
namespace ar {

const char* AddPartitioningVariablesPass::name() const {
  return "add-partitioning-variables";
}

const char* AddPartitioningVariablesPass::description() const {
  return "Add partitioning variables";
}

bool AddPartitioningVariablesPass::run(Bundle* bundle) {
  bool change = false;

  for (auto it = bundle->function_begin(), et = bundle->function_end();
       it != et;
       ++it) {
    change = this->run_on_function(*it) || change;
  }

  return change;
}

namespace {

/// \brief A pair containing a basic block and an internal variable
struct BasicBlockVarPair {
  BasicBlock* bb;
  InternalVariable* var;
};

} // end anonymous namespace

bool AddPartitioningVariablesPass::run_on_function(Function* fun) {
  if (fun->is_declaration()) {
    return false;
  }

  Bundle* bundle = fun->bundle();
  Context& ctx = bundle->context();

  if (fun->type()->return_type() != IntegerType::si32(ctx)) {
    // Return type is not si32
    return false;
  }

  Code* body = fun->body();
  if (!body->has_exit_block()) {
    // Function has no exit block
    return false;
  }

  BasicBlock* exit_block = body->exit_block();
  if (exit_block->empty()) {
    // Exit block is empty
    return false;
  }

  auto return_stmt = dyn_cast< ReturnValue >(exit_block->back());
  if (return_stmt == nullptr) {
    // Exit block does not end with a return
    return false;
  }

  if (!return_stmt->has_operand()) {
    // Return statement has no operand
    return false;
  }

  auto return_var = dyn_cast< InternalVariable >(return_stmt->operand());
  if (return_var == nullptr) {
    // Function does not return a variable
    return false;
  }

  // Check if the return variable is defined in multiple basic blocks
  BasicBlock* bb = exit_block;
  InternalVariable* var = return_var;

  while (bb != nullptr) {
    if (bb == body->entry_block()) {
      // Return variable is defined in the entry block
      return false;
    }

    for (auto it = bb->rbegin(), et = bb->rend(); it != et; ++it) {
      Statement* stmt = *it;

      if (stmt->result_or_null() == var) {
        if (isa< Assignment >(stmt) &&
            isa< InternalVariable >(stmt->operand(0))) {
          // Track the operand variable
          var = cast< InternalVariable >(stmt->operand(0));
        } else {
          // Return variable is defined in one basic block
          return false;
        }
      }
    }

    if (bb->num_predecessors() == 1) {
      bb = *bb->predecessor_begin();
    } else if (bb->num_predecessors() >= 2) {
      bb = nullptr; // Return variable can be used for partitioning
    } else {
      ikos_unreachable("non-entry basic block with no predecessor");
    }
  }

  // Annotate the function

  // Function to mark a variable as a partitioning variable
  Function* partitioning_var_fun =
      bundle->intrinsic_function(Intrinsic::IkosPartitioningVarSi32);

  // List of predecessors that define the return variable
  std::vector< BasicBlockVarPair > preds = {
      BasicBlockVarPair{exit_block, return_var}};

  // List of basic blocks already seen
  std::unordered_set< BasicBlock* > seen;

  while (!preds.empty()) {
    bb = preds.back().bb;   // Current basic block
    var = preds.back().var; // Tracked variable
    preds.pop_back();

    auto res = seen.insert(bb);
    if (!res.second) {
      continue; // Already seen
    }

    for (auto it = bb->end(); it != bb->begin();) {
      --it;
      Statement* stmt = *it;

      if (stmt->result_or_null() == var) {
        if (isa< Assignment >(stmt) &&
            isa< InternalVariable >(stmt->operand(0))) {
          // Statement assigns the tracked variable to another variable

          // Insert the partitioning annotation
          auto partitioning_annotation =
              Call::create(/* result = */ nullptr,
                           /* function = */ partitioning_var_fun,
                           /* arguments = */ {var});
          it = bb->insert_after(it, std::move(partitioning_annotation));
          --it;

          // Track the operand variable
          var = cast< InternalVariable >(stmt->operand(0));
        } else {
          // Statement defines the tracked variable

          // Insert a partitioning annotation
          auto partitioning_annotation =
              Call::create(/* result = */ nullptr,
                           /* function = */ partitioning_var_fun,
                           /* arguments = */ {var});
          it = bb->insert_after(it, std::move(partitioning_annotation));
          --it;

          // Stop tracking the variable
          var = nullptr;
          break;
        }
      }
    }

    if (var == nullptr) {
      continue;
    }

    // Basic block does not define this variable, look at predecessors
    for (auto it = bb->predecessor_begin(), et = bb->predecessor_end();
         it != et;
         ++it) {
      preds.push_back(BasicBlockVarPair{*it, var});
    }
  }

  return true;
}

} // end namespace ar
} // end namespace ikos
