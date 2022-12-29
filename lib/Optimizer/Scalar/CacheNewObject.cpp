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

/// \return vector of unique property names which can be cached, in the order
///   in which they must be cached.
static std::vector<Literal *> getPropsForCaching(
    Function *func,
    Instruction *thisParam) {
  BasicBlock *entryBlock = &func->front();

  // Put the users of 'this' into a set for fast checking of which instructions
  // we care about.
  llvh::DenseSet<Instruction *> thisUsers{};
  for (Instruction *user : thisParam->getUsers())
    thisUsers.insert(user);

  // Result vector of property keys to cache.
  std::vector<Literal *> props{};

  // Keep track of keys that we've seen, so we only add unique keys.
  llvh::DenseSet<Literal *> seenProps{};

  // Go through instructions in order to find the StorePropertyInsts we
  // care about.
  for (Instruction &inst : *entryBlock) {
    // "try" not allowed because we can't allow exceptional control flow
    // that continues within this function, it could leave the object in an
    // inconsistent state.
    if (llvh::isa<TryStartInst>(inst)) {
      return props;
    }

    // From here on we only care about instructions that use 'this'.
    if (!thisUsers.count(&inst))
      continue;

    StorePropertyInst *store = llvh::dyn_cast<StorePropertyInst>(&inst);

    // 'this' used outside of a StorePropertyInst, bail.
    if (!store)
      return props;

    // Check if "this" is being used in a non-Object operand position.
    for (uint32_t i = 0, e = store->getNumOperands(); i < e; ++i) {
      if (i != StorePropertyInst::ObjectIdx &&
          store->getOperand(i) == thisParam) {
        return props;
      }
    }

    auto *prop = llvh::dyn_cast<Literal>(store->getProperty());

    // Property name is not a literal, bail.
    if (!prop)
      return props;

    // Valid store for caching, append to the list if it's new.
    if (!seenProps.count(prop)) {
      props.push_back(prop);
      seenProps.insert(prop);
    }
  }

  return props;
}

/// Insert the CacheNewObject instruction into the beginning of \p F.
/// \param keys the literal names of the keys of the this object.
static void insertCacheInstruction(
    Function *func,
    llvh::ArrayRef<Literal *> keys,
    Instruction *thisParam) {
  IRBuilder builder{func};
  builder.setInsertionPointAfter(thisParam);

  GetNewTargetInst *newTargetInst = builder.createGetNewTargetInst();
  builder.createCacheNewObjectInst(thisParam, newTargetInst, keys);
}

bool CacheNewObject::runOnFunction(Function *func) {
  LLVM_DEBUG(
      llvh::dbgs() << "Attempting to cache new object in function: '"
                   << func->getInternalNameStr() << "'\n");

  Value *dynParam = func->getJSDynamicParam(0);
  if (!dynParam->hasUsers())
    return false;

  Instruction *thisParam = llvh::cast<LoadParamInst>(dynParam->getUsers()[0]);
  if (!thisParam->hasUsers())
    return false;
  if (auto CTI = llvh::dyn_cast<CoerceThisNSInst>(thisParam->getUsers()[0]))
    thisParam = CTI;

  std::vector<Literal *> keys = getPropsForCaching(func, thisParam);

  // Not enough stores to cache.
  if (keys.size() < MIN_PROPERTIES_FOR_CACHE) {
    LLVM_DEBUG(
        llvh::dbgs() << llvh::format(
            "Not caching new object, needs at least %u keys, found %u\n",
            MIN_PROPERTIES_FOR_CACHE,
            keys.size()));
    return false;
  }

  LLVM_DEBUG(llvh::dbgs() << llvh::format("Caching %u keys\n", keys.size()));

  // Actually insert the CacheNewObject instruction.
  insertCacheInstruction(func, keys, thisParam);

  return true;
}

Pass *hermes::createCacheNewObject() {
  return new CacheNewObject();
}

#undef DEBUG_TYPE
