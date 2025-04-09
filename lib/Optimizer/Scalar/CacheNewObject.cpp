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
/// 1. Property initializations must be ordered so that every user of 'this'
///   dominates subsequent users.
/// 2. The only users of "this" in the chain must be StorePropertyInsts
///   which store to it.
/// 3. Property writes to "this" must use literal keys (not use computed keys).
///
/// As a result, the optimization is fairly conservative but should account
/// for many of the most common construction patterns.
///
/// TODO: There are ways to make this optimization more complex:
/// * Use a more involved heuristic for determining if a function is likely
///   to be invoked as a constructor.
/// * When we have classes, we know which functions are constructors.

#define DEBUG_TYPE "cachenewobject"

#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/IRUtils.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes {
namespace {

/// The minimum number of cachable property names to actually insert the cache
/// instruction.
constexpr size_t kMinPropertiesForCache = 2;

/// \param thisParam the instruction loading the 'this' param in the function.
/// \return vector of unique property names which can be cached, in the order
///   in which they must be cached.
static std::vector<LiteralString *> getPropsForCaching(
    const DominanceInfo &domInfo,
    Function *func,
    Instruction *thisParam) {
  BasicBlock *entryBlock = &func->front();

  // Put the users of 'this' into a set for fast checking of which instructions
  // we care about.
  llvh::DenseSet<Instruction *> thisUsers{};
  llvh::DenseSet<BasicBlock *> thisUsersBlocks{};
  for (Instruction *user : thisParam->getUsers()) {
    thisUsers.insert(user);
    thisUsersBlocks.insert(user->getParent());
  }

  // Order the blocks containing users of 'this' such that each block dominates
  // all other blocks that use 'this'. Note that it is important that we start
  // the traversal at the entry block rather than the block containing the load
  // of 'this'. This handles two important cases where 'this' may not dominate
  // the instruction that leaks it:
  // 1. Phi instructions using 'this' that are not dominated by the block
  //    containing the load of 'this'.
  // 2. Return instructions, which implicitly use 'this' because the caller can
  //    observe the state of the object at the point we return.
  auto orderedBlocks = orderBlocksByDominance(
      domInfo, entryBlock, [&thisUsersBlocks](BasicBlock *block) -> bool {
        // Must dominate all returns as well, because the caller can read the
        // value of 'this' after the constructor returns.
        return thisUsersBlocks.count(block) != 0 ||
            llvh::isa<ReturnInst>(block->getTerminator());
      });

  // Result vector of property keys to cache.
  std::vector<LiteralString *> props{};

  // Keep track of keys that we've seen, so we only add unique keys.
  llvh::DenseSet<LiteralString *> seenProps{};

  // Go through instructions in order to find the StorePropertyInsts we
  // care about.
  for (BasicBlock *block : orderedBlocks) {
    for (Instruction &inst : *block) {
      // From here on we only care about instructions that use 'this'.
      if (!thisUsers.count(&inst))
        continue;

      StorePropertyInst *store = llvh::dyn_cast<StorePropertyInst>(&inst);

      // 'this' used outside of a StorePropertyInst, bail.
      if (!store)
        return props;

      auto *prop = llvh::dyn_cast<LiteralString>(store->getProperty());

      // Property name is not a literal string, bail.
      if (!prop)
        return props;

      // Check if "this" is being used in a non-Object operand position.
      for (uint32_t i = 0, e = store->getNumOperands(); i < e; ++i) {
        if (i != StorePropertyInst::ObjectIdx &&
            store->getOperand(i) == thisParam) {
          return props;
        }
      }

      // Valid store for caching, append to the list if it's new.
      if (!seenProps.count(prop)) {
        props.push_back(prop);
        seenProps.insert(prop);
      }
    }
  }

  return props;
}

/// Insert the CacheNewObject instruction into \p func after \p thisParam.
/// \param keys the literal names of the keys of the this object.
/// \param thisParam the instruction loading the 'this' param in the function.
static void insertCacheInstruction(
    Function *func,
    llvh::ArrayRef<LiteralString *> keys,
    Instruction *thisParam) {
  IRBuilder builder{func};

  builder.setInsertionPointAfter(thisParam);
  // We technically cannot reliably obtain the new target during optimization,
  // as it may have been optimized to undefined if it is unused. For
  // CacheNewObject, this would not cause a correctness problem, but it would
  // prevent the optimization from kicking in at runtime.
  GetNewTargetInst *newTargetInst =
      builder.createGetNewTargetInst(func->getNewTargetParam());
  builder.createCacheNewObjectInst(thisParam, newTargetInst, keys);
}

/// Attempt to cache the new object in \p F.
/// \return true if the function was modified.
static bool cacheNewObjectInFunction(Function *func) {
  LLVM_DEBUG(
      llvh::dbgs() << "Attempting to cache new object in function: '"
                   << func->getInternalNameStr() << "'\n");

  if (func->getDefinitionKind() != Function::DefinitionKind::ES5Function) {
    // Bail if the function is not a normal function.
    // TODO: Apply this optimization to ES6 constructors once they're
    // implemented.
    return false;
  }

  JSDynamicParam *thisDynParam = func->getJSDynamicParam(0);
  if (!thisDynParam->hasOneUser()) {
    // Bail if there's no users or if there's more than one LoadParam.
    return false;
  }

  Instruction *thisParam =
      llvh::dyn_cast<LoadParamInst>(thisDynParam->getUsers().front());

  if (!thisParam || !thisParam->hasUsers()) {
    // No usage of 'this', don't cache anything.
    return false;
  }

  // In loose functions, 'this' can also be coerced into an object,
  // so check for that and update to the 'this' that's actually used.
  // If the function is invoked as a constructor,
  // 'this' is already an object, CoerceThisNS is effectively a Mov,
  // and it actually is the same object.
  for (Instruction *user : thisParam->getUsers()) {
    if (auto *coerce = llvh::dyn_cast<CoerceThisNSInst>(user)) {
      thisParam = coerce;
      break;
    }
  }

  DominanceInfo domInfo{func};
  std::vector<LiteralString *> keys =
      getPropsForCaching(domInfo, func, thisParam);

  // Not enough stores to cache.
  if (keys.size() < kMinPropertiesForCache) {
    LLVM_DEBUG(
        llvh::dbgs() << llvh::format(
            "Not caching new object, needs at least %u keys, found %u\n",
            kMinPropertiesForCache,
            keys.size()));
    return false;
  }

  LLVM_DEBUG(llvh::dbgs() << llvh::format("Caching %u keys\n", keys.size()));

  static_assert(
      kMinPropertiesForCache > 0,
      "CacheNewObjectInst requires at least one key");

  // Actually insert the CacheNewObject instruction.
  insertCacheInstruction(func, keys, thisParam);

  return true;
}

} // namespace

Pass *createCacheNewObject() {
  /// Inserts the CacheNewObjectInst if possible, to reduce the time spent
  /// creating and populating a new hidden class.
  class ThisPass : public FunctionPass {
   public:
    explicit ThisPass() : hermes::FunctionPass("CacheNewObject") {}
    ~ThisPass() override = default;

    bool runOnFunction(Function *F) override {
      return cacheNewObjectInFunction(F);
    }
  };

  return new ThisPass();
}

} // namespace hermes

#undef DEBUG_TYPE
