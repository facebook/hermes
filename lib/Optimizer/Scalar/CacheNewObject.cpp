/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
///
/// The CacheNewObject optimization adds a CacheNewObjectInst at the start
/// of the function if it detects that the function has an "initialization
/// block", i.e. a sequence of writes to the `this` parameter of fixed
/// properties.
/// For example:
///   this.x = x;
///   this.y = y;
///   // etc.
/// constitutes an initialization block and we want to be able to cache the
/// hidden class and just fetch it so we don't have to rebuild it each time
/// this function is called to make a new object.
///
/// The instruction checks new.target to ensure that the function is being
/// called as a constructor, ensuring `this` is a new object.
///
/// This optimization has a few requirements:
/// 1. Property initialization must happen in the entry BasicBlock
///   so that we know the precise sequence of properties placed into "this".
/// 2. The only users of "this" must be StorePropertyInsts which store to it.
/// 3. Property writes to "this" must use literals (not use computed keys).
/// 4. Property initialization must not occur within a "try" block.
///
/// As a result, the optimization is fairly conservative but should account
/// for many of the most common construction patterns.
///
/// TODO: There are ways to make this optimization more complex:
/// * Check outside the entry BasicBlock for cachable stores.
/// * Use a more involved heuristic for determining if a function is likely
///   to be invoked as a constructor.

#define DEBUG_TYPE "cachenewobject"

#include "hermes/Optimizer/Scalar/CacheNewObject.h"

#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"

using namespace hermes;

/// The minimum number of cachable property names to actually insert the cache
/// instruction.
constexpr size_t MIN_PROPERTIES_FOR_CACHE = 2;

/// \return vector of StorePropertyInsts which can be cached.
static std::vector<StorePropertyInst *> getStoresForCaching(Function *func) {
  Parameter *thisParam = func->getThisParameter();
  BasicBlock *entryBlock = &func->front();

  // Put the users of 'this' into a set for fast checking of which instructions
  // we care about.
  llvh::DenseSet<Instruction *> thisUsers{};
  for (Instruction *user : thisParam->getUsers())
    thisUsers.insert(user);

  std::vector<StorePropertyInst *> stores{};

  // Go through instructions in order to find the StorePropertyInsts we
  // care about.
  for (Instruction &inst : *entryBlock) {
    // "try" not allowed because we can't allow exceptional control flow
    // that continues within this function, it could leave the object in an
    // inconsistent state.
    if (llvh::isa<TryStartInst>(inst))
      return stores;

    // From here on we only care about instructions that use 'this'.
    if (!thisUsers.count(&inst))
      continue;

    StorePropertyInst *store = llvh::dyn_cast<StorePropertyInst>(&inst);

    // 'this' used outside of a StorePropertyInst, bail.
    if (!store)
      return stores;

    // Check if "this" is being used in a non-Object operand position.
    for (uint32_t i = 0, e = store->getNumOperands(); i < e; ++i) {
      if (i != StorePropertyInst::ObjectIdx &&
          store->getOperand(i) == thisParam) {
        return stores;
      }
    }

    // Property name is not a literal, bail.
    if (!llvh::isa<Literal>(store->getProperty()))
      return stores;

    // Valid store for caching, append to the list.
    stores.push_back(store);
  }

  return stores;
}

/// Insert the CacheNewObject instruction into \p F and replace all uses
/// of %this with uses of the result of the CacheNewObject instruction.
/// \param keys the literal names of the keys of the this object.
static void insertCacheInstruction(
    Function *func,
    llvh::ArrayRef<Literal *> keys) {
  BasicBlock *entryBlock = &func->front();

  IRBuilder builder{func};
  builder.setInsertionPoint(&entryBlock->front());

  CacheNewObjectInst *cacheInst =
      builder.createCacheNewObjectInst(func->getThisParameter(), keys);
  func->getThisParameter()->replaceAllUsesWith(cacheInst);

  // The replaceAllUsesWith call will replace the first operand of the cache
  // instruction, so put the operand back.
  cacheInst->setOperand(func->getThisParameter(), CacheNewObjectInst::ThisIdx);
}

bool CacheNewObject::runOnFunction(Function *func) {
  // If we can't insert the cache instruction, we're done.
  std::vector<StorePropertyInst *> stores = getStoresForCaching(func);

  // Storage for the property names so we can create the instruction.
  std::vector<Literal *> keys{};
  // Keep track of keys that we've seen, so we only add unique keys.
  llvh::DenseSet<Literal *> seenKeys{};

  // Collect the literal properties to be used in the cache.
  // Only append to `keys` when a new key is encountered.
  for (StorePropertyInst *store : stores) {
    auto *prop = llvh::cast<Literal>(store->getProperty());
    if (!seenKeys.count(prop)) {
      keys.push_back(prop);
      seenKeys.insert(prop);
    }
  }

  LLVM_DEBUG(llvh::dbgs() << llvh::format("Found %u keys\n", keys.size()));

  // Not enough stores to cache.
  if (keys.size() < MIN_PROPERTIES_FOR_CACHE) {
    return false;
  }

  // Actually insert the CacheNewObject instruction.
  insertCacheInstruction(func, keys);

  return true;
}

Pass *hermes::createCacheNewObject() {
  return new CacheNewObject();
}

#undef DEBUG_TYPE
