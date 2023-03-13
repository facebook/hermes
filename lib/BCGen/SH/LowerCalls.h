/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_SH_LOWERCALLS_H
#define HERMES_BCGEN_SH_LOWERCALLS_H

#include "hermes/BCGen/HBC/HVMRegisterAllocator.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes::sh {

/// Lower calls into a series of parameter moves followed by a call with
/// those moved values. Should only run once, right before MovElimination.
class LowerCalls : public FunctionPass {
 public:
  explicit LowerCalls(hbc::HVMRegisterAllocator &RA)
      : FunctionPass("LowerCalls"), RA_(RA) {}
  ~LowerCalls() override = default;
  bool runOnFunction(Function *F) override {
    IRBuilder builder(F);
    bool changed = false;

    for (auto &BB : *F) {
      for (auto &I : BB) {
        auto *call = llvh::dyn_cast<BaseCallInst>(&I);
        // This also matches constructors.
        if (!call)
          continue;
        builder.setInsertionPoint(call);
        changed = true;

        auto reg = RA_.getLastRegister().getIndex() -
            hbc::HVMRegisterAllocator::CALL_EXTRA_REGISTERS;

        for (int i = 0, e = call->getNumArguments(); i < e; i++, --reg) {
          // If this is a Call instruction, emit explicit Movs to the argument
          // registers. If this is a CallN instruction, emit ImplicitMovs
          // instead, to express that these registers get written to by the
          // CallN, even though they are not the destination. Lastly, if this is
          // argument 0 of CallBuiltinInst emit ImplicitMov to encode that the
          // "this" register is implicitly set to undefined.
          Value *arg = call->getArgument(i);
          if (llvh::isa<HBCCallNInst>(call) ||
              (i == 0 && llvh::isa<CallBuiltinInst>(call))) {
            auto *imov = builder.createImplicitMovInst(arg);
            RA_.updateRegister(imov, Register(reg));
          } else {
            auto *mov = builder.createMovInst(arg);
            RA_.updateRegister(mov, Register(reg));
            call->setArgument(mov, i);
          }
        }
      }
    }
    return changed;
  }

 protected:
  hbc::HVMRegisterAllocator &RA_;
};

} // namespace hermes::sh

#endif
