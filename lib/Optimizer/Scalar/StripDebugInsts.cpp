/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
///
/// This optimization deletes debug-only instructions, for example when they are
/// added but not needed/supported by the backend.
//===----------------------------------------------------------------------===//

#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {
namespace {

/// Run the pass on \p F.
bool stripDebug(Function *F) {
  IRBuilder::InstructionDestroyer destroyer;

  bool changed = false;
  for (auto &BB : *F) {
    for (auto &I : BB) {
      // Destroy EvalCompilationDataInst and Debugger.
      switch (I.getKind()) {
        case ValueKind::EvalCompilationDataInstKind:
        case ValueKind::DebuggerInstKind:
          destroyer.add(&I);
          changed = true;
          break;
        default:
          break;
      }
    }
  }

  return changed;
}

} // namespace

Pass *createStripDebugInsts() {
  class ThisPass : public FunctionPass {
   public:
    explicit ThisPass() : hermes::FunctionPass("StripDebugInsts") {}
    ~ThisPass() override = default;

    bool runOnFunction(Function *F) override {
      return stripDebug(F);
    }
  };
  return new ThisPass();
}

} // namespace hermes

#undef DEBUG_TYPE
