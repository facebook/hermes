/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_REMOVEMOVS_H
#define HERMES_BCGEN_REMOVEMOVS_H

#include "hermes/BCGen/SH/SH.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {

/// After register allocation, remove useless MovInsts
template <class RegisterAllocator>
class RemoveMovs : public FunctionPass {
 public:
  explicit RemoveMovs(RegisterAllocator &RA)
      : FunctionPass("RemoveMovs"), ra_(RA) {}
  ~RemoveMovs() override = default;
  bool runOnFunction(Function *F) override {
    IRBuilder builder(F);
    IRBuilder::InstructionDestroyer destroyer;
    bool changed = false;
    for (auto &BB : *F) {
      for (auto &I : BB) {
        if (auto *Mov = llvh::dyn_cast<MovInst>(&I)) {
          // Delete Mov when the src/dst registers are the same.
          if (auto optSrc = ra_.getOptionalRegister(Mov->getSingleOperand());
              optSrc && *optSrc == ra_.getRegister(&I)) {
            Mov->replaceAllUsesWith(Mov->getSingleOperand());
            destroyer.add(&I);
            changed = true;
          }
        }
      }
    }
    return changed;
  }

 private:
  RegisterAllocator &ra_;
};

} // namespace hermes

#endif
