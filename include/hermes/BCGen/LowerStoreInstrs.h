/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_LOWERSTOREINSTS_H
#define HERMES_BCGEN_LOWERSTOREINSTS_H

#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// Lowers Store instructions down to MOVs after register allocation.
template <class RegisterAllocator>
class LowerStoreInstrs : public FunctionPass {
 public:
  explicit LowerStoreInstrs(RegisterAllocator &RA)
      : FunctionPass("LowerStoreInstrs"), RA_(RA) {}
  ~LowerStoreInstrs() override = default;

  bool runOnFunction(Function *F) override {
    IRBuilder builder(F);
    IRBuilder::InstructionDestroyer destroyer;
    bool changed = false;

    auto PO = postOrderAnalysis(F);
    llvh::SmallVector<BasicBlock *, 16> order(PO.rbegin(), PO.rend());
    for (auto *bbit : order) {
      for (auto &it : bbit->getInstList()) {
        auto *SSI = llvh::dyn_cast<StoreStackInst>(&it);
        if (!SSI)
          continue;

        Value *ptr = SSI->getPtr();
        Value *val = SSI->getValue();

        builder.setInsertionPoint(&it);
        auto dstReg = RA_.getRegister(ptr);
        auto *mov = builder.createMovInst(val);
        RA_.updateRegister(mov, dstReg);
        it.replaceAllUsesWith(mov);
        destroyer.add(&it);
        changed = true;
      }
    }
    return changed;
  }

 private:
  RegisterAllocator &RA_;
};

} // namespace hermes

#endif
