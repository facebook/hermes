/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/Passes/PeepholeLowering.h"
#include "hermes/BCGen/CommonPeepholeLowering.h"
#include "hermes/FrontEndDefs/Builtins.h"
#include "hermes/FrontEndDefs/Typeof.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"

namespace hermes {

namespace hbc {

/// Convert an IR Type to a TypeOfIsTypes bitmask for runtime type checking.
static TypeOfIsTypes irTypeToTypeOfIsTypes(TypeContext &tc, Type type) {
  TypeOfIsTypes result;
  // Uninit maps to undefined at runtime.
  if (tc.canBeUndefined(type) || tc.canBeUninit(type))
    result = result.withUndefined(true);
  if (tc.canBeNull(type))
    result = result.withNull(true);
  if (tc.canBeBoolean(type))
    result = result.withBoolean(true);
  if (tc.canBeString(type))
    result = result.withString(true);
  if (tc.canBeNumber(type))
    result = result.withNumber(true);
  if (tc.canBeBigInt(type))
    result = result.withBigint(true);
  if (tc.canBeSymbol(type))
    result = result.withSymbol(true);
  // IR Object encompasses both objects and callables.
  if (tc.canBeObject(type)) {
    result = result.withObject(true);
    result = result.withFunction(true);
  }
  return result;
}

class DoLower {
  Function *const F_;
  IRBuilder builder_{F_};
  IRBuilder::InstructionDestroyer destroyer_{};
  bool optimize_;

 public:
  explicit DoLower(Function *F, bool optimize) : F_(F), optimize_(optimize) {}

  bool run() {
    bool changed = false;
    for (auto &BB : *F_) {
      for (auto &I : BB) {
        if (Value *replaceVal = peep(&I)) {
          if (replaceVal != &I)
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
  /// non-null value must be returned; if it is different from \p I, it will be
  /// used to replace all uses of \p I.
  Value *peep(Instruction *I) {
    switch (I->getKind()) {
      case ValueKind::CoerceThisNSInstKind:
        // This transformation is purely an optimization to collapse a sequence
        // of LoadParam + CoerceThisNS into a LoadThisNS, so skip it if
        // optimizations are disabled.
        if (!optimize_)
          return nullptr;
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
      case ValueKind::CheckedTypeCastInstKind:
        return lowerCheckedTypeCast(llvh::cast<CheckedTypeCastInst>(I));
      case ValueKind::CallInstKind:
        return stripEnvFromCall(llvh::cast<CallInst>(I), builder_);
      case ValueKind::CreateFunctionInstKind:
      case ValueKind::CreateGeneratorInstKind:
      case ValueKind::CreateClassInstKind:
        return lowerTopLevelFunction(
            llvh::cast<BaseCreateLexicalChildInst>(I), builder_);
      default:
        return nullptr;
    }
  }

  Value *lowerCheckedTypeCast(CheckedTypeCastInst *inst) {
    destroyer_.add(inst);
    builder_.setInsertionPoint(inst);
    TypeContext &tc = builder_.getTypeContext();
    if (tc.isSubsetOf(inst->getCheckedValue()->getType(), inst->getType())) {
      // No need to do any work.
      return inst->getCheckedValue();
    }
    TypeOfIsTypes types = irTypeToTypeOfIsTypes(tc, inst->getType());
    auto *replace = builder_.createCallBuiltinInst(
        BuiltinMethod::HermesBuiltin_checkedTypeCast,
        {inst->getCheckedValue(), builder_.getLiteralNumber(types.getRaw())});
    replace->setType(inst->getType());
    return replace;
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
    if (SCI->getNumOperands() == 1) {
      // One string is a noop.
      return SCI->getOperand(0);
    }
    if (SCI->getNumOperands() == 2) {
      // Two strings can be concatenated with HBCStringConcatInst.
      return builder_.createHBCStringConcatInst(
          SCI->getOperand(0), SCI->getOperand(1));
    }
    auto *firstString = SCI->getOperand(0);
    CallInst::ArgumentList restOfStrings;
    for (size_t i = 1, e = SCI->getNumOperands(); i < e; ++i) {
      restOfStrings.push_back(SCI->getOperand(i));
    }
    auto *concatRes = builder_.createCallInst(
        builder_.createLoadPropertyInst(
            builder_.createTryLoadGlobalPropertyInst("HermesInternal"),
            "concat"),
        builder_.getLiteralUndefined(),
        firstString,
        restOfStrings);
    concatRes->setType(Type::createString());
    return concatRes;
  }

  Value *lowerTopLevelFunction(
      BaseCreateLexicalChildInst *BCLI,
      IRBuilder &builder) {
    if (!llvh::isa<EmptySentinel>(BCLI->getScope()))
      return nullptr;
    BCLI->setScope(builder.getLiteralUndefined());
    return BCLI;
  }
};

Pass *createPeepholeLowering(bool optimize) {
  class ThisPass : public FunctionPass {
    bool optimize_;

   public:
    explicit ThisPass(bool optimize)
        : FunctionPass("PeepholeLowering"), optimize_(optimize) {}
    bool runOnFunction(Function *F) override {
      return DoLower(F, optimize_).run();
    }
  };
  return new ThisPass(optimize);
}

} // namespace hbc

} // namespace hermes

#undef DEBUG_TYPE
