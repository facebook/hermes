/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LoweringPasses.h"

#include "hermes/BCGen/SerializedLiteralGenerator.h"

#include "hermes/IR/IRBuilder.h"

namespace hermes::sh {
namespace {

/// Lower the given \p allocInst into an \c AllocObjectInst, and a series of \c
/// StoreNewOwnPropertyInst that populate its fields.
bool lowerAlloc(AllocObjectLiteralInst *allocInst) {
  Function *F = allocInst->getParent()->getParent();
  IRBuilder builder(F);

  auto size = allocInst->getKeyValuePairCount();

  // Replace AllocObjectLiteral with a regular AllocObject
  builder.setLocation(allocInst->getLocation());
  builder.setInsertionPoint(allocInst);
  auto *Obj = builder.createAllocObjectInst(size, nullptr);

  for (unsigned i = 0; i < allocInst->getKeyValuePairCount(); i++) {
    Literal *key = allocInst->getKey(i);
    Value *value = allocInst->getValue(i);
    builder.createStoreNewOwnPropertyInst(
        value, allocInst, key, IRBuilder::PropEnumerable::Yes);
  }
  allocInst->replaceAllUsesWith(Obj);
  allocInst->eraseFromParent();

  return true;
}

/// Lower the given \p allocInst to either an \c AllocObjectInst or an
/// \c HBCAllocObjectFromBufferInst, handling non-serializable values by
/// emitting explicit stores for them.
bool lowerAllocObjectBuffer(AllocObjectLiteralInst *allocInst) {
  Function *F = allocInst->getParent()->getParent();
  IRBuilder builder(F);
  // We currently serialize all keys, even if the additional placeholders may
  // increase size, to improve performance and because the serialized values are
  // cheap relative to the corresponding native code.
  // TODO: Revisit this tradeoff once we have a better model for native code
  // size.
  auto size = std::min<size_t>(UINT16_MAX, allocInst->getKeyValuePairCount());

  // Should not create HBCAllocObjectFromBufferInst, since we do not want to
  // serialize any keys. Note that as mentioned above, this will currently only
  // happen if there are no properties to serialize, so this will just produce a
  // single AllocObject.
  if (size == 0) {
    return lowerAlloc(allocInst);
  }

  // Replace AllocObjectLiteral with HBCAllocObjectFromBufferInst
  builder.setLocation(allocInst->getLocation());
  builder.setInsertionPointAfter(allocInst);
  HBCAllocObjectFromBufferInst::ObjectPropertyMap propMap;

  unsigned i = 0;
  for (; i < size; i++) {
    Literal *key = allocInst->getKey(i);
    Value *value = allocInst->getValue(i);
    Literal *propLiteral = nullptr;
    // Property name can be either a LiteralNumber or a LiteralString.
    if (auto *LN = llvh::dyn_cast<LiteralNumber>(key)) {
      assert(
          LN->convertToArrayIndex() &&
          "LiteralNumber can be a property name only if it can be converted to array index.");
      propLiteral = LN;
    } else {
      propLiteral = cast<LiteralString>(key);
    }

    if (SerializedLiteralGenerator::isSerializableLiteral(value)) {
      propMap.push_back(std::pair<Literal *, Literal *>(
          propLiteral, llvh::cast<Literal>(value)));
    } else {
      // The key has a non-serializable value, insert a placeholder so we
      // generate the final hidden class upfront, and then store the value
      // later.
      // TODO: Evaluate whether the placeholder is worthwhile for properties
      // that are numbers, since their order is not important.
      propMap.push_back(std::pair<Literal *, Literal *>(
          propLiteral, builder.getLiteralNull()));
      builder.createStorePropertyInst(value, allocInst, key);
    }
  }

  // Handle properties beyond best num of properties or that cannot fit in
  // maxSize.
  for (; i < allocInst->getKeyValuePairCount(); i++) {
    Literal *key = allocInst->getKey(i);
    Value *value = allocInst->getValue(i);
    builder.createStoreNewOwnPropertyInst(
        value, allocInst, key, IRBuilder::PropEnumerable::Yes);
  }

  // Emit HBCAllocObjectFromBufferInst.
  // First, we reset insertion location.
  builder.setLocation(allocInst->getLocation());
  builder.setInsertionPoint(allocInst);
  auto *alloc = builder.createHBCAllocObjectFromBufferInst(
      propMap, allocInst->getKeyValuePairCount());
  allocInst->replaceAllUsesWith(alloc);
  allocInst->eraseFromParent();

  return true;
}

bool lowerAllocObjectLiteral(Function *F) {
  bool changed = false;

  for (BasicBlock &BB : *F) {
    // We need to increase the iterator before calling lowerAllocObjectBuffer.
    // Otherwise deleting the instruction will invalidate the iterator.
    for (auto it = BB.begin(), e = BB.end(); it != e;) {
      if (auto *A = llvh::dyn_cast<AllocObjectLiteralInst>(&*it++)) {
        changed |= lowerAllocObjectBuffer(A);
      }
    }
  }

  return changed;
}

} // namespace

Pass *createLowerAllocObjectLiteral() {
  class ThisPass : public FunctionPass {
   public:
    explicit ThisPass() : FunctionPass("LowerAllocObjectLiteral") {}
    bool runOnFunction(Function *F) override {
      return lowerAllocObjectLiteral(F);
    }
  };
  return new ThisPass();
}

} // namespace hermes::sh
