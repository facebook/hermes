/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "objectstackpromotion"

#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {
namespace {

/// Try to promote the given object to a series of AllocStackInsts if it does
/// not escape its enclosing function. If successful, \p alloc and all of its
/// users are eliminated and replaced with stack operations. For now, this only
/// handles objects with constant key properties, that are accessed with known
/// existing properties.
/// \p alloc the object to promote
/// \p stores are known to always execute without any other intervening users
/// \p destroyer is used to eliminate instructions
bool tryPromoteObject(
    AllocObjectLiteralInst *alloc,
    IRBuilder::InstructionDestroyer &destroyer) {
  IRBuilder builder(alloc->getFunction());
  auto numElems = alloc->getKeyValuePairCount();
  // Used as scratch space to perform the conversion of a number to a string.
  char conversionBuf[NUMBER_TO_STRING_BUF_SIZE];

  // Map from number to its string representation.
  llvh::SmallDenseMap<LiteralNumber *, LiteralString *> conversionMapping;
  /// Convert \p LN to a LiteralString.
  auto convertNumber =
      [&conversionMapping, &conversionBuf, &builder](LiteralNumber *LN) {
        auto &cachedLS = conversionMapping[LN];
        if (cachedLS) {
          return cachedLS;
        }
        auto len = numberToString(
            LN->getValue(), conversionBuf, NUMBER_TO_STRING_BUF_SIZE);
        auto strRef = llvh::StringRef(conversionBuf, len);
        auto *LS = builder.getLiteralString(strRef);
        cachedLS = LS;
        return LS;
      };

  // Map from property key string -> slot index.
  llvh::SmallDenseMap<LiteralString *, size_t> objLayout;
  // Populate the object layout, ensuring all property keys are represented as
  // LiteralStrings.
  for (size_t i = 0; i < numElems; ++i) {
    auto *propKey = alloc->getKey(i);
    if (auto *LS = llvh::dyn_cast<LiteralString>(propKey)) {
      objLayout.insert({LS, i});
    } else {
      auto *LN = llvh::cast<LiteralNumber>(propKey);
      objLayout.insert({convertNumber(LN), i});
    }
  }

  /// \return true if \p V is known to be an existing property key on the
  /// object. This will convert LiteralNumbers to LiteralStrings to do the
  /// check, since the layout is defined in terms of strings.
  auto isInLayout = [&objLayout, &convertNumber](Value *V) -> bool {
    if (auto *LS = llvh::dyn_cast<LiteralString>(V)) {
      return objLayout.count(LS);
    }
    if (auto *LN = llvh::dyn_cast<LiteralNumber>(V)) {
      return objLayout.count(convertNumber(LN));
    }
    return false;
  };

  for (auto *U : alloc->getUsers()) {
    // Loading from the object does not escape.
    if (auto *L = llvh::dyn_cast<PrLoadInst>(U)) {
      assert(L->getObject() == alloc && "Load from a different object");
      continue;
    }
    if (auto *LP = llvh::dyn_cast<TypedLoadParentInst>(U)) {
      assert(LP->getObject() == alloc && "Load from a different object");
      continue;
    }

    // Storing into the object is fine, but storing the object anywhere
    // (including into itself) means we cannot elide the allocation.
    if (auto *S = llvh::dyn_cast<PrStoreInst>(U)) {
      if (S->getStoredValue() != alloc) {
        assert(S->getObject() == alloc && "Unknown usage of object.");
        continue;
      }
    }
    if (auto *SP = llvh::dyn_cast<TypedStoreParentInst>(U)) {
      if (SP->getStoredValue() != alloc) {
        assert(SP->getObject() == alloc && "Unknown usage of object.");
        continue;
      }
    }

    if (auto *SP = llvh::dyn_cast<StorePropertyInst>(U)) {
      if (SP->getStoredValue() == alloc)
        return false;
      if (!isInLayout(SP->getProperty()))
        return false;
      continue;
    }

    if (auto *SOP = llvh::dyn_cast<DefineOwnPropertyInst>(U)) {
      if (SOP->getStoredValue() == alloc)
        return false;
      if (!isInLayout(SOP->getProperty()))
        return false;
      continue;
    }

    if (auto *LP = llvh::dyn_cast<LoadPropertyInst>(U)) {
      if (!isInLayout(LP->getProperty()))
        return false;
      continue;
    }

    // For now, all other instructions are considered to escape.
    return false;
  }

  // The object does not escape, create a stack location for each of its fields.
  builder.setLocation(alloc->getLocation());
  builder.setInsertionPoint(alloc);
  llvh::SmallVector<AllocStackInst *, 8> stackLocs;
  stackLocs.reserve(numElems);

  auto *parentLoc =
      builder.createAllocStackInst("[parent]", Type::createAnyType());
  auto numericPropName = builder.createIdentifier("[numeric prop]");
  for (size_t i = 0; i < numElems; ++i) {
    auto *LS = llvh::dyn_cast<LiteralString>(alloc->getKey(i));
    auto name = LS ? LS->getValue() : numericPropName;
    auto *loc = builder.createAllocStackInst(name, Type::createAnyType());
    stackLocs.push_back(loc);
    builder.createStoreStackInst(alloc->getValue(i), loc);
  }

  /// \return the AllocStackInst replacement of the given property key \p
  /// propKey.
  auto stackLocOfPropKey =
      [&stackLocs, &objLayout, &conversionMapping](Literal *propKey) {
        if (auto *LS = llvh::dyn_cast<LiteralString>(propKey)) {
          return stackLocs[objLayout[LS]];
        }
        auto *LN = llvh::cast<LiteralNumber>(propKey);
        assert(
            conversionMapping.count(LN) &&
            "numeric key should have already been converted");
        return stackLocs[objLayout[conversionMapping[LN]]];
      };

  for (auto *U : alloc->getUsers()) {
    builder.setLocation(U->getLocation());
    builder.setInsertionPoint(U);
    destroyer.add(U);

    // Replace loads with loads from the stack location. Narrow the result type
    // to honor the original type.
    if (auto *L = llvh::dyn_cast<PrLoadInst>(U)) {
      auto *load = builder.createLoadStackInst(stackLocs[L->getPropIndex()]);
      auto *narrowed = builder.createUnionNarrowTrustedInst(load, L->getType());
      L->replaceAllUsesWith(narrowed);
      continue;
    }
    if (auto *LP = llvh::dyn_cast<TypedLoadParentInst>(U)) {
      auto *load = builder.createLoadStackInst(parentLoc);
      auto *narrowed =
          builder.createUnionNarrowTrustedInst(load, LP->getType());
      LP->replaceAllUsesWith(narrowed);
      continue;
    }

    // Replace stores with stores to the stack location.
    if (auto *S = llvh::dyn_cast<PrStoreInst>(U)) {
      builder.createStoreStackInst(
          S->getStoredValue(), stackLocs[S->getPropIndex()]);
      continue;
    }
    if (auto *SP = llvh::dyn_cast<TypedStoreParentInst>(U)) {
      builder.createStoreStackInst(SP->getStoredValue(), parentLoc);
      continue;
    }

    if (auto *LP = llvh::dyn_cast<LoadPropertyInst>(U)) {
      auto *replace = builder.createLoadStackInst(
          stackLocOfPropKey(llvh::cast<Literal>(LP->getProperty())));
      LP->replaceAllUsesWith(replace);
      continue;
    }

    if (auto *SP = llvh::dyn_cast<StorePropertyInst>(U)) {
      auto *replace = builder.createStoreStackInst(
          SP->getStoredValue(),
          stackLocOfPropKey(llvh::cast<Literal>(SP->getProperty())));
      SP->replaceAllUsesWith(replace);
      continue;
    }

    if (auto *SOP = llvh::dyn_cast<DefineOwnPropertyInst>(U)) {
      auto *replace = builder.createStoreStackInst(
          SOP->getStoredValue(),
          stackLocOfPropKey(llvh::cast<Literal>(SOP->getProperty())));
      SOP->replaceAllUsesWith(replace);
      continue;
    }

    llvm_unreachable("Unhandled instruction");
  }
  destroyer.add(alloc);

  return true;
}

bool runObjectStackPromotion(Function *F) {
  bool changed = false;
  // Iterate over all instructions in the function and try to promote any
  // AllocObjectLiteralInsts.
  IRBuilder::InstructionDestroyer destroyer;
  for (auto &BB : *F)
    for (auto &I : BB)
      if (auto *alloc = llvh::dyn_cast<AllocObjectLiteralInst>(&I))
        changed |= tryPromoteObject(alloc, destroyer);
  return changed;
}

} // namespace

Pass *createObjectStackPromotion() {
  class ThisPass : public FunctionPass {
   public:
    explicit ThisPass() : FunctionPass("ObjectStackPromotion") {}
    ~ThisPass() override = default;

    bool runOnFunction(Function *M) override {
      return runObjectStackPromotion(M);
    }
  };
  return new ThisPass();
}

} // namespace hermes
#undef DEBUG_TYPE
