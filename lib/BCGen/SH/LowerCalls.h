/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_SH_LOWERCALLS_H
#define HERMES_BCGEN_SH_LOWERCALLS_H

#include "SHRegAlloc.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes::sh {

/// Lower calls into a series of parameter moves followed by a call with
/// those moved values. Should only run once, right before MovElimination.
class LowerCalls : public FunctionPass {
 public:
  explicit LowerCalls(SHRegisterAllocator &RA)
      : FunctionPass("LowerCalls"), RA_(RA) {}
  ~LowerCalls() override = default;
  bool runOnFunction(Function *F) override {
    IRBuilder builder(F);
    bool changed = false;

    uint32_t maxArgsRegs = RA_.getMaxArgumentRegisters();

    auto stackReg = [maxArgsRegs](int32_t index) {
      return sh::Register(
          RegClass::RegStack, (RegIndex)((int32_t)maxArgsRegs + index));
    };

    for (auto &BB : *F) {
      for (auto &I : BB) {
        auto *call = llvh::dyn_cast<BaseCallInst>(&I);
        // This also matches constructors.
        if (!call)
          continue;
        builder.setInsertionPoint(call);
        changed = true;

        if (llvh::isa<ConstructInst>(call)) {
          // If this is a constructor call, setup new.target.
          RA_.updateRegister(
              builder.createMovInst(call->getCallee()),
              stackReg(hbc::StackFrameLayout::NewTarget));
        } else if (llvh::isa<CallInst>(call)) {
          // If this is a normal call, invalidate new.target.
          RA_.updateRegister(
              builder.createImplicitMovInst(builder.getLiteralUndefined()),
              stackReg(hbc::StackFrameLayout::NewTarget));
        } else if (llvh::isa<CallBuiltinInst>(call)) {
          // If this is a CallBuiltin, invalidate new.target and the callee.
          RA_.updateRegister(
              builder.createImplicitMovInst(builder.getLiteralUndefined()),
              stackReg(hbc::StackFrameLayout::NewTarget));
          RA_.updateRegister(
              builder.createImplicitMovInst(builder.getLiteralEmpty()),
              stackReg(hbc::StackFrameLayout::CalleeClosureOrCB));
        }

        // If this is a normal or constructor call, populate the callee.
        if (llvh::isa<ConstructInst>(call) || llvh::isa<CallInst>(call)) {
          auto *mov = builder.createMovInst(call->getCallee());
          RA_.updateRegister(
              mov, stackReg(hbc::StackFrameLayout::CalleeClosureOrCB));
          call->setCallee(mov);
        }

        RegIndex reg = maxArgsRegs + hbc::StackFrameLayout::ThisArg;

        for (unsigned i = 0, e = call->getNumArguments(); i < e; ++i, --reg) {
          // If this is a Call instruction, emit explicit Movs to the argument
          // registers.
          Value *arg = call->getArgument(i);
          if (i == 0 && llvh::isa<CallBuiltinInst>(call)) {
            // If this is argument 0 of CallBuiltinInst, emit ImplicitMov to
            // encode that the "this" register is implicitly set to undefined.
            auto *imov = builder.createImplicitMovInst(arg);
            RA_.updateRegister(imov, Register(RegClass::RegStack, reg));
          } else {
            auto *mov = builder.createMovInst(arg);
            RA_.updateRegister(mov, Register(RegClass::RegStack, reg));
            call->setArgument(mov, i);
          }
        }
      }
    }
    return changed;
  }

 protected:
  SHRegisterAllocator &RA_;
};

} // namespace hermes::sh

#endif
