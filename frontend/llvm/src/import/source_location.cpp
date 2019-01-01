/*******************************************************************************
 *
 * \file
 * \brief Source location for AR statements generated from LLVM
 *
 * Author: Maxime Arthaud
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

#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Instruction.h>

#include <ikos/frontend/llvm/import/source_location.hpp>

namespace ikos {
namespace frontend {
namespace import {

boost::filesystem::path source_path(llvm::DIFile* file) {
  ikos_assert(file != nullptr);

  boost::filesystem::path path;
  boost::filesystem::path filename = file->getFilename().str();
  boost::filesystem::path directory = file->getDirectory().str();

  if (filename.is_absolute()) {
    path = filename;
  } else {
    path = directory / filename;
  }

  if (boost::filesystem::exists(path)) {
    path = boost::filesystem::canonical(path);
  }

  path.make_preferred();
  return path;
}

/// \brief Return the source location of the given instruction
static SourceLocation source_location(llvm::Instruction* inst) {
  ikos_assert(inst != nullptr);

  for (auto it = inst->getIterator(), et = inst->getParent()->end(); it != et;
       ++it) {
    llvm::DILocation* loc = it->getDebugLoc().get();

    if (loc != nullptr) {
      return SourceLocation(loc);
    }
  }

  return {}; // null location
}

SourceLocation source_location(ar::Statement* stmt) {
  ikos_assert(stmt != nullptr);

  if (!stmt->has_frontend()) {
    return {}; // null location
  }

  auto value = stmt->frontend< llvm::Value >();

  if (auto inst = llvm::dyn_cast< llvm::Instruction >(value)) {
    SourceLocation loc = source_location(inst);

    if (loc) {
      return loc;
    }
  }

  // No debug information: could be a phi or bitcast of a global value.
  // Find the next statement with debug info
  llvm::SmallPtrSet< ar::BasicBlock*, 2 > seen;
  ar::BasicBlock* bb = stmt->parent();
  auto it = stmt->iterator() + 1;
  auto et = bb->end();

  while (true) {
    // Iterate over the following statements
    for (; it != et; ++it) {
      ar::Statement* s = *it;

      if (s->has_frontend()) {
        if (auto inst = llvm::dyn_cast< llvm::Instruction >(
                s->frontend< llvm::Value >())) {
          SourceLocation loc = source_location(inst);

          if (loc) {
            return loc;
          }
        }
      }
    }

    if (bb->num_successors() == 1) {
      // Visit the successor
      bb = *bb->successor_begin();

      if (!seen.insert(bb).second) {
        return {}; // Already seen
      }

      it = bb->begin();
      et = bb->end();
      continue;
    }

    if (bb->num_successors() > 1) {
      // Basic block has several successors, check for an assert
      bb = *bb->successor_begin();

      if (!bb->empty()) {
        ar::Statement* s = bb->front();

        if (ar::isa< ar::Comparison >(s) && s->has_frontend()) {
          if (auto inst = llvm::dyn_cast< llvm::Instruction >(
                  s->frontend< llvm::Value >())) {
            SourceLocation loc = source_location(inst);

            if (loc) {
              return loc;
            }
          }
        }
      }
    }

    // Could not find a statement with debug info
    return {};
  }
}

} // end namespace import
} // end namespace frontend
} // end namespace ikos
