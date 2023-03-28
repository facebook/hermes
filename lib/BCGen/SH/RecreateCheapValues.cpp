/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LoweringPasses.h"

#include "hermes/IR/IRBuilder.h"

namespace hermes::sh {

/// This pass replaces Movs with LoadConstant when the constant is not a
/// pointer.
/// Must run after RA since that's when Movs are introduced.
static bool recreateCheapValues(Function *F, SHRegisterAllocator &RA) {
  IRBuilder builder(F);
  llvh::SmallPtrSet<Instruction *, 4> potentiallyUnused;
  bool changed = false;

  for (auto &BB : *F) {
    IRBuilder::InstructionDestroyer destroyer;
    for (auto &I : BB) {
      auto *mov = llvh::dyn_cast<MovInst>(&I);
      if (!mov)
        continue;
      auto *load = llvh::dyn_cast<HBCLoadConstInst>(mov->getSingleOperand());
      if (!load)
        continue;
      Literal *literal = load->getConst();

      if (!literal->getType().isNonPtr())
        continue;

      builder.setInsertionPoint(mov);
      auto *recreation = builder.createHBCLoadConstInst(literal);
      RA.updateRegister(recreation, RA.getRegister(mov));
      mov->replaceAllUsesWith(recreation);
      destroyer.add(mov);
      // Defer checking whether the load is unused until after any Mov(s)
      // using it have been destroyed.
      potentiallyUnused.insert(load);
      changed = true;
    }
  }

  {
    IRBuilder::InstructionDestroyer destroyer;
    for (auto *inst : potentiallyUnused) {
      if (!inst->hasUsers()) {
        destroyer.add(inst);
      }
    }
  }
  return changed;
}

Pass *createRecreateCheapValues(SHRegisterAllocator &RA) {
  class RecreateCheapValues : public FunctionPass {
    SHRegisterAllocator &RA_;

   public:
    explicit RecreateCheapValues(SHRegisterAllocator &RA)
        : FunctionPass("RecreateCheapValues"), RA_(RA) {}
    bool runOnFunction(Function *F) override {
      return recreateCheapValues(F, RA_);
    }
  };
  return new RecreateCheapValues(RA);
}

} // namespace hermes::sh
