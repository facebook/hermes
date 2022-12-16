/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/Lowering.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/Support/Statistic.h"

#define DEBUG_TYPE "lirpeephole"

using namespace hermes;

STATISTIC(NumLIRPeephole, "Number of LIR peephole replacements");

namespace {

class LIRPeephole {
  Function *const F_;
  IRBuilder builder_{F_};
  IRBuilder::InstructionDestroyer destroyer_{};

 public:
  explicit LIRPeephole(Function *F) : F_(F) {}

  bool run() {
    bool changed = false;
    LLVM_DEBUG(
        llvh::dbgs() << "-- LIRPeephole " << F_->getInternalNameStr() << "\n");
    for (auto &BB : *F_) {
      for (auto &I : BB) {
        if (Value *replaceVal = peep(&I)) {
          I.replaceAllUsesWith(replaceVal);
          ++NumLIRPeephole;
          changed = true;
        }
      }
    }
    return changed;
  }

 private:
  /// Perform peephole optimization on the specified instruction. Replacement
  /// instructions need to be created and inserted in the correct position by
  /// using the builder. Instructions for deletion should be inserted in the
  /// destroyer in the correct order (users first). If a change is made, a
  /// non-null value must be returned; it will be used to replace all uses of
  /// \p I.
  Value *peep(Instruction *I) {
    switch (I->getKind()) {
      case ValueKind::CoerceThisNSInstKind: {
        auto *CTI = llvh::cast<CoerceThisNSInst>(I);
        // If the operand is LoadParamInst with one user, loading "this",
        // we can replace them with Load
        if (auto *LPI =
                llvh::dyn_cast<LoadParamInst>(CTI->getSingleOperand())) {
          if (LPI->hasOneUser() &&
              LPI->getParam() == F_->getJSDynamicParam(0)) {
            LLVM_DEBUG(
                llvh::dbgs()
                << "Replacing CoerceThisNS/LoadParam -> LoadThisNS\n");
            destroyer_.add(CTI);
            destroyer_.add(LPI);
            builder_.setInsertionPointAfter(CTI);
            return builder_.createLIRGetThisNSInst();
          }
        }
      } break;

      default:
        break;
    }
    return nullptr;
  }
};

} // namespace

namespace hermes {
Pass *createLIRPeephole() {
  class ThisPass : public FunctionPass {
   public:
    explicit ThisPass() : FunctionPass("LIRPeephole") {}
    ~ThisPass() override = default;
    bool runOnFunction(Function *F) override {
      return LIRPeephole(F).run();
    }
  };
  return new ThisPass();
}
} // namespace hermes

#undef DEBUG_TYPE
