/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LoweringPasses.h"

#include "hermes/IR/IRBuilder.h"

namespace hermes::sh {

/// Lower calls into a series of parameter moves followed by a call with
/// those moved values. Should only run once, right before MovElimination.
static bool lowerCalls(Function *F, SHRegisterAllocator &RA) {
  IRBuilder builder(F);
  bool changed = false;

  uint32_t maxArgsRegs = RA.getMaxArgumentRegisters();

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

      if (llvh::isa<CallBuiltinInst>(call)) {
        // If this is a CallBuiltin, invalidate new.target and the callee.
        RA.updateRegister(
            builder.createImplicitMovInst(builder.getLiteralUndefined()),
            stackReg(hbc::StackFrameLayout::NewTarget));
        RA.updateRegister(
            builder.createImplicitMovInst(builder.getLiteralEmpty()),
            stackReg(hbc::StackFrameLayout::CalleeClosureOrCB));
      }

      // If this is a normal or constructor call, populate the callee and
      // new.target.
      if (llvh::isa<ConstructInst>(call) || llvh::isa<CallInst>(call)) {
        RA.updateRegister(
            builder.createMovInst(call->getNewTarget()),
            stackReg(hbc::StackFrameLayout::NewTarget));
        auto *mov = builder.createMovInst(call->getCallee());
        RA.updateRegister(
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
          RA.updateRegister(imov, Register(RegClass::RegStack, reg));
        } else {
          auto *mov = builder.createMovInst(arg);
          RA.updateRegister(mov, Register(RegClass::RegStack, reg));
          call->setArgument(mov, i);
        }
      }
    }
  }
  return changed;
}

Pass *createLowerCalls(SHRegisterAllocator &RA) {
  class LowerCalls : public FunctionPass {
    SHRegisterAllocator &RA_;

   public:
    explicit LowerCalls(SHRegisterAllocator &RA)
        : FunctionPass("LowerCalls"), RA_(RA) {}
    bool runOnFunction(Function *F) override {
      return lowerCalls(F, RA_);
    }
  };
  return new LowerCalls(RA);
}

} // namespace hermes::sh
