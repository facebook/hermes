/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/Passes/ReorderRegisters.h"
#include "hermes/BCGen/RegAlloc.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"

namespace hermes::hbc {

bool ReorderRegisters::runOnFunction(Function *F) {
  uint32_t totalRegCount = RA_.getMaxRegisterUsage();

  // If there's no registers, nothing to do.
  // If the function has to spill registers, don't reorder to avoid having to
  // deal with the spill registers.
  if (totalRegCount == 0 ||
      !SpillRegisters::isShort(Register(totalRegCount - 1))) {
    return false;
  }

  /// Store info on how to sort each register.
  struct RegSortData {
    Register reg{};
    /// Union of all types stored in this register.
    Type type = Type::createNoType();
    /// Heuristic score for this register, higher scores will be placed first.
    double score = 0.0;

    /// \return the "type class" of this register, used for sorting.
    /// Higher numbers indicate types that are sorted earlier in the final
    /// ordering.
    uint32_t typeClassNumber() const {
      if (type.isNumberType())
        return 3;
      if (type.isNonPtr())
        return 2;
      return 1;
    }
  };

  // Figure out the type of each register.
  llvh::SmallVector<RegSortData, 32> regTypes(totalRegCount, RegSortData{});
  for (size_t i = 0; i < totalRegCount; ++i) {
    regTypes[i].reg = Register(i);
  }

  DominanceInfo dom{F};
  LoopAnalysis loopAnalysis{F, dom};

  llvh::DenseMap<BasicBlock *, uint64_t> loopNestingCache;

  /// Compute how many nested loops surround each BasicBlock,
  /// avoid redundant work by using the loopNestingCache.
  auto getLoopNestingDepth = [&loopAnalysis,
                              &loopNestingCache](BasicBlock *BB) -> uint32_t {
    uint32_t depth = 0;
    BasicBlock *foundCache = nullptr;

    // Walk up the loops, stopping when we find a block we've already seen the
    // loop depth for.
    for (BasicBlock *cur = BB; cur && loopAnalysis.isBlockInLoop(cur);
         cur = loopAnalysis.getLoopPreheader(cur)) {
      auto it = loopNestingCache.find(BB);
      if (it != loopNestingCache.end()) {
        // Cache hit, done looping.
        foundCache = cur;
        depth += it->second;
        break;
      }
      ++depth;
    }

    // Save the result, depth is about to be modified.
    uint32_t result = depth;

    // Populate the cache with any blocks we traversed, stopping at foundCache
    // because it's already populated.
    for (BasicBlock *cur = BB; depth != 0 && cur &&
         loopAnalysis.isBlockInLoop(cur) && cur != foundCache;
         cur = loopAnalysis.getLoopPreheader(cur)) {
      loopNestingCache[cur] = depth;
      --depth;
    }

    return result;
  };

  // Multiplier for loop depth, applied every nested loop.
  static constexpr double kLoopMultiplier = 10.0;

  // Record the types of each register.
  // Calculate scores for each of the registers based on loop depth, and add to
  // the score every time a register is used as an operand or as the result.
  for (BasicBlock &BB : *F) {
    uint32_t nestingDepth = getLoopNestingDepth(&BB);
    double loopScore = std::pow(kLoopMultiplier, (double)nestingDepth);

    for (Instruction &I : BB) {
      if (!RA_.isAllocated(&I))
        continue;
      RegSortData &regTypeData = regTypes[RA_.getRegister(&I).getIndex()];
      // Update the type of the register.
      Type *t = &regTypeData.type;
      *t = Type::unionTy(*t, I.getType());
      // Add to the score for the result.
      regTypeData.score += loopScore;
      // Add to the score for each operand.
      for (unsigned i = 0, e = I.getNumOperands(); i < e; ++i) {
        Instruction *op = llvh::dyn_cast<Instruction>(I.getOperand(i));
        if (op && RA_.isAllocated(op)) {
          regTypes[RA_.getRegister(op).getIndex()].score = loopScore;
        }
      }
    }
  }

  std::sort(
      regTypes.begin(),
      regTypes.end(),
      [](const RegSortData &a, const RegSortData &b) -> bool {
        // Sort by type, then by score, then by register index.
        // Place higher numbers earlier.
        return std::tuple{a.typeClassNumber(), a.score, a.reg.getIndex()} >
            std::tuple{b.typeClassNumber(), b.score, b.reg.getIndex()};
      });

  // Count the registers by type for the remapping.
  // Index is Register, value is index within its register class.
  llvh::SmallVector<uint32_t, 32> remapping(totalRegCount, UINT32_MAX);
  uint32_t numberRegCount = 0;
  uint32_t nonPtrRegCount = 0;
  for (size_t i = 0; i < regTypes.size(); ++i) {
    if (regTypes[i].type.isNumberType())
      ++numberRegCount;
    else if (regTypes[i].type.isNonPtr())
      ++nonPtrRegCount;
    remapping[regTypes[i].reg.getIndex()] = i;
  }

  // Reassign registers according to the mapping.
  for (BasicBlock &BB : *F) {
    for (Instruction &I : BB) {
      if (!RA_.isAllocated(&I))
        continue;
      auto reg = RA_.getRegister(&I);
      uint32_t newIdx = remapping[reg.getIndex()];
      assert(newIdx != UINT32_MAX && "Register not remapped");
      RA_.updateRegister(&I, Register(newIdx));
#ifndef NDEBUG
      if (I.hasOutput()) {
        if (newIdx < numberRegCount) {
          assert(I.getType().isNumberType() && "Expected number type");
        } else if (newIdx < numberRegCount + nonPtrRegCount) {
          assert(I.getType().isNonPtr() && "Expected nonptr type");
        }
      }
#endif
    }
  }

  RA_.setNumberRegCount(numberRegCount);
  RA_.setNonPtrRegCount(nonPtrRegCount);
  return true;
}

} // namespace hermes::hbc
