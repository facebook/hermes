/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "funccallnopts"

#include "hermes/BCGen/HBC/Passes/FuncCallNOpts.h"

#include "hermes/IR/IRBuilder.h"

using namespace hermes;

namespace {

/// \return the arguments for a given CallInst \p call, excluding the 'this'.
std::vector<Value *> getArgumentsWithoutThis(CallInst *call) {
  std::vector<Value *> result;
  unsigned argCount = call->getNumArguments();
  result.reserve(argCount - 1);
  for (unsigned i = 1; i < argCount; i++) {
    result.push_back(call->getArgument(i));
  }
  return result;
}

} // namespace

/// Replace Call instructions with HBCCallNInst when the argument count is
/// in range.
bool FuncCallNOpts::runOnFunction(Function *F) {
  bool changed = false;
  IRBuilder::InstructionDestroyer destroyer;
  IRBuilder builder{F};

  for (BasicBlock &BB : *F) {
    for (Instruction &insn : BB) {
      // We can only operate on CallInst, not its subclasses.
      if (insn.getKind() == ValueKind::CallInstKind) {
        auto *call = llvh::cast<CallInst>(&insn);
        unsigned argCount = call->getNumArguments();
        if (HBCCallNInst::kMinArgs <= argCount &&
            argCount <= HBCCallNInst::kMaxArgs) {
          builder.setLocation(call->getLocation());
          builder.setCurrentSourceLevelScope(call->getSourceLevelScope());
          builder.setInsertionPoint(call);
          HBCCallNInst *newCall = builder.createHBCCallNInst(
              call->getCallee(),
              call->getThis(),
              getArgumentsWithoutThis(call));
          call->replaceAllUsesWith(newCall);
          destroyer.add(call);
          changed = true;
        }
      }
    }
  }
  return changed;
}

#undef DEBUG_TYPE
