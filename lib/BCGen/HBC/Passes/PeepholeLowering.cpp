/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/Passes/PeepholeLowering.h"
#include "hermes/BCGen/CommonPeepholeLowering.h"
#include "hermes/IR/Instrs.h"

namespace hermes {

namespace hbc {

class DoLower {
  Function *const F_;
  IRBuilder builder_{F_};
  IRBuilder::InstructionDestroyer destroyer_{};

 public:
  explicit DoLower(Function *F) : F_(F) {}

  bool run() {
    bool changed = false;
    for (auto &BB : *F_) {
      for (auto &I : BB) {
        if (Value *replaceVal = peep(&I)) {
          I.replaceAllUsesWith(replaceVal);
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
      case ValueKind::CoerceThisNSInstKind:
        return lowerCoerceThisNSInst(
            llvh::cast<CoerceThisNSInst>(I), builder_, destroyer_);
      case ValueKind::BinaryExponentiationInstKind:
        return lowerBinaryExponentiationInst(
            llvh::cast<BinaryOperatorInst>(I), builder_, destroyer_);
      case ValueKind::ThrowTypeErrorInstKind:
        return lowerThrowTypeError(llvh::cast<ThrowTypeErrorInst>(I));
      case ValueKind::GetTemplateObjectInstKind:
        return lowerGetTemplateObject(llvh::cast<GetTemplateObjectInst>(I));
      case ValueKind::StringConcatInstKind:
        return lowerStringConcat(llvh::cast<StringConcatInst>(I));
      default:
        return nullptr;
    }
  }

  Value *lowerThrowTypeError(ThrowTypeErrorInst *TTE) {
    destroyer_.add(TTE);
    builder_.setInsertionPoint(TTE);
    auto *replace = builder_.createCallBuiltinInst(
        BuiltinMethod::HermesBuiltin_throwTypeError, {TTE->getMessage()});
    builder_.createUnreachableInst();
    return replace;
  }

  Value *lowerGetTemplateObject(GetTemplateObjectInst *GTO) {
    destroyer_.add(GTO);
    CallInst::ArgumentList argList;
    // Copy over all operands because they are deliberately set up using
    // the same layout as HermesBuiltin_getTemplateObject.
    for (uint32_t i = 0, e = GTO->getNumOperands(); i < e; ++i) {
      argList.push_back(GTO->getOperand(i));
    }
    builder_.setInsertionPoint(GTO);
    auto *call = builder_.createCallBuiltinInst(
        BuiltinMethod::HermesBuiltin_getTemplateObject, argList);
    GTO->replaceAllUsesWith(call);
    return call;
  }

  Value *lowerStringConcat(StringConcatInst *SCI) {
    destroyer_.add(SCI);
    builder_.setInsertionPoint(SCI);
    auto *firstString = SCI->getOperand(0);
    CallInst::ArgumentList restOfStrings;
    for (size_t i = 1, e = SCI->getNumOperands(); i < e; ++i) {
      restOfStrings.push_back(SCI->getOperand(i));
    }
    return builder_.createCallInst(
        builder_.createLoadPropertyInst(
            builder_.createTryLoadGlobalPropertyInst("HermesInternal"),
            "concat"),
        builder_.getLiteralUndefined(),
        firstString,
        restOfStrings);
  }
};

bool PeepholeLowering::runOnFunction(Function *F) {
  return DoLower(F).run();
}

} // namespace hbc

} // namespace hermes

#undef DEBUG_TYPE
