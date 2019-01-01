/*******************************************************************************
 *
 * \file
 * \brief Implementation of the Namer
 *
 * Author: Maxime Arthaud
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

#include <deque>

#include <ikos/ar/format/namer.hpp>
#include <ikos/ar/semantic/function.hpp>
#include <ikos/ar/semantic/statement.hpp>

namespace ikos {
namespace ar {

Namer::Namer() = default;

Namer::Namer(Code* code) {
  this->init(code);
}

void Namer::init(Code* code) {
  // Counter
  std::size_t bb_idx = 0;
  std::size_t var_idx = 0;

  // Name parameters
  if (code->is_function_body()) {
    Function* f = code->function();

    for (auto it = f->param_begin(), et = f->param_end(); it != et; ++it) {
      InternalVariable* v = *it;

      if (!v->has_name() &&
          this->_variables.find(v) == this->_variables.end()) {
        this->_variables.emplace(v, std::to_string(++var_idx));
      }
    }
  }

  // Top-down walk through the basic blocks
  std::deque< ar::BasicBlock* > worklist;

  if (code->has_entry_block()) {
    worklist.push_back(code->entry_block());
  }

  while (!worklist.empty()) {
    // Pop the front element
    ar::BasicBlock* bb = worklist.front();
    worklist.pop_front();

    // Already processed
    if (this->_basic_blocks.find(bb) != this->_basic_blocks.end()) {
      continue;
    }

    // Name this basic block
    if (bb->has_name()) {
      this->_basic_blocks.emplace(bb, bb->name()); // mark as processed
    } else {
      this->_basic_blocks.emplace(bb, std::to_string(++bb_idx));
    }

    // Add successors in the worklist
    for (auto bb_it = bb->successor_begin(), bb_et = bb->successor_end();
         bb_it != bb_et;
         ++bb_it) {
      worklist.push_back(*bb_it);
    }

    // Name all variables in statements
    for (auto s_it = bb->begin(), s_et = bb->end(); s_it != s_et; ++s_it) {
      ar::Statement* stmt = *s_it;

      // Name operands
      for (auto op_it = stmt->op_begin(), op_et = stmt->op_end();
           op_it != op_et;
           ++op_it) {
        if (auto op = dyn_cast< Variable >(*op_it)) {
          if (!op->has_name() &&
              this->_variables.find(op) == this->_variables.end()) {
            this->_variables.emplace(op, std::to_string(++var_idx));
          }
        }
      }

      // Name result
      if (stmt->has_result() && !stmt->result()->has_name() &&
          this->_variables.find(stmt->result()) == this->_variables.end()) {
        this->_variables.emplace(stmt->result(), std::to_string(++var_idx));
      }
    }
  }

  // Name unreachable basic blocks
  for (auto it = code->begin(), et = code->end(); it != et; ++it) {
    BasicBlock* bb = *it;

    if (!bb->has_name() &&
        this->_basic_blocks.find(bb) == this->_basic_blocks.end()) {
      this->_basic_blocks.emplace(bb, std::to_string(++bb_idx));
    }
  }

  // Name unused local variables
  if (code->is_function_body()) {
    Function* f = code->function();

    for (auto it = f->local_variable_begin(), et = f->local_variable_end();
         it != et;
         ++it) {
      LocalVariable* v = *it;

      if (!v->has_name() &&
          this->_variables.find(v) == this->_variables.end()) {
        this->_variables.emplace(v, std::to_string(++var_idx));
      }
    }
  }

  // Name unused internal variables
  for (auto it = code->internal_variable_begin(),
            et = code->internal_variable_end();
       it != et;
       ++it) {
    InternalVariable* v = *it;

    if (!v->has_name() && this->_variables.find(v) == this->_variables.end()) {
      this->_variables.emplace(v, std::to_string(++var_idx));
    }
  }
}

const std::string& Namer::name(Variable* v) const {
  if (v->has_name()) {
    return v->name();
  } else {
    ikos_assert(this->_variables.find(v) != this->_variables.end());
    return this->_variables.at(v);
  }
}

const std::string& Namer::name(BasicBlock* bb) const {
  if (bb->has_name()) {
    return bb->name();
  } else {
    ikos_assert(this->_basic_blocks.find(bb) != this->_basic_blocks.end());
    return this->_basic_blocks.at(bb);
  }
}

} // end namespace ar
} // end namespace ikos
