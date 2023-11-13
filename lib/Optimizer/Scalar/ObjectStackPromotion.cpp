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
/// handles objects with properties at known offsets that are accessed with
/// PrLoad and PrStore.
/// Instructions to destroy will be added to the given \p destroyer.
bool tryPromoteObject(
    AllocObjectLiteralInst *alloc,
    IRBuilder::InstructionDestroyer &destroyer) {
  for (auto *U : alloc->getUsers()) {
    // Loading from the object does not escape.
    if (auto *L = llvh::dyn_cast<PrLoadInst>(U)) {
      assert(L->getObject() == alloc && "Load from a different object");
      continue;
    }
    if (auto *LP = llvh::dyn_cast<LoadParentInst>(U)) {
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
    if (auto *SP = llvh::dyn_cast<StoreParentInst>(U)) {
      if (SP->getStoredValue() != alloc) {
        assert(SP->getObject() == alloc && "Unknown usage of object.");
        continue;
      }
    }

    // For now, all other instructions are considered to escape.
    return false;
  }

  // The object does not escape, create a stack location for each of its fields.
  IRBuilder builder(alloc->getFunction());
  builder.setLocation(alloc->getLocation());
  builder.setInsertionPoint(alloc);
  auto numElems = alloc->getKeyValuePairCount();
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
    if (auto *LP = llvh::dyn_cast<LoadParentInst>(U)) {
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
    if (auto *SP = llvh::dyn_cast<StoreParentInst>(U)) {
      builder.createStoreStackInst(SP->getStoredValue(), parentLoc);
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
