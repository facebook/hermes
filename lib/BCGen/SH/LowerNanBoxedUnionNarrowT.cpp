/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LoweringPasses.h"

#include "hermes/IR/IRBuilder.h"

namespace hermes::sh {

/// When operating on Nan-boxed values, UnionNarrowTrusted doesn't change the
/// data representation, in simply acts as an assertion of the input type.
/// This pass replaces UnionNarrowTrusted with a simple copy of the input and
/// narrows input's type to the result type.
static bool lowerNanBoxedUnionNarrowTrusted(Function *F) {
  bool changed = false;
  IRBuilder::InstructionDestroyer destroyer{};
  IRBuilder builder(F);

  for (BasicBlock &BB : *F) {
    for (Instruction &I : BB) {
      auto *UNT = llvh::dyn_cast<UnionNarrowTrustedInst>(&I);
      if (!UNT)
        continue;

      changed = true;
      destroyer.add(UNT);

      auto *input = UNT->getSingleOperand();

      // Note that ordinarily the UNT result type is narrower than the input
      // type, but we might already have narrowed the input type from a
      // different UNT.
      Type t = Type::intersectTy(UNT->getType(), input->getType());
      if (!t.isNoType()) {
        // This is the normal path were we simply narrow the input type.
        input->setType(t);
        UNT->replaceAllUsesWith(input);
      } else {
        // This is the degenerate case where the input type has been narrowed to
        // an impossibility, meaning that this code will never execute. However,
        // we still want to maintain the type invariants.
        builder.setInsertionPoint(UNT);
        builder.setLocation(UNT->getLocation());
        auto *deadValue = builder.createLIRDeadValueInst(UNT->getType());
        UNT->replaceAllUsesWith(deadValue);
      }
    }
  }

  return changed;
}

Pass *createLowerNanBoxedUnionNarrowTrusted() {
  class LowerNBUNT : public FunctionPass {
   public:
    explicit LowerNBUNT() : FunctionPass("LowerNanBoxedUnionNarrowTrusted") {}
    bool runOnFunction(Function *F) override {
      return lowerNanBoxedUnionNarrowTrusted(F);
    }
  };
  return new LowerNBUNT();
}

} // namespace hermes::sh
