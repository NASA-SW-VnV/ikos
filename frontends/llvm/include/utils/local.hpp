#ifndef IKOSPP_LOCAL_HPP
#define IKOSPP_LOCAL_HPP

#include "llvm/IR/Function.h"

/*
 * Port some local transformations unavailable with llvm2.9 from newer
 * versions
 */

namespace ikos_pp {
using namespace llvm;

void changeToUnreachable(Instruction* I, bool UseLLVMTrap);

/// \brief Remove all blocks that can not be reached from the function's entry.
///
/// Returns true if any basic block was removed.
bool removeUnreachableBlocks(llvm::Function& F);

} // end namespace

#endif // IKOSPP_LOCAL_HPP
