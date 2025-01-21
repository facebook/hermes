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
  uint32_t totalRegCount = RA_.getMaxHVMRegisterUsage();

  // If there's no registers, nothing to do.
  // If the function has to spill registers, don't reorder to avoid having to
  // deal with the spill registers.
  if (totalRegCount == 0 || !SpillRegisters::isShort(totalRegCount - 1)) {
    return false;
  }

  /// Store info on how to sort each register.
  struct RegSortData {
    Register reg{};
    /// Heuristic score for this register, higher scores will be placed first.
    double score = 0.0;
  };

  // Figure out the type of each register.
  llvh::SmallVector<RegSortData, 32> regTypes[(size_t)RegClass::_last];
  for (auto &rt : regTypes) {
    rt.resize(totalRegCount, RegSortData{});
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

  uint32_t maxNestingDepth = 0;

  // Record the types of each register.
  // Calculate scores for each of the registers based on loop depth, and add to
  // the score every time a register is used as an operand or as the result.
  for (BasicBlock &BB : *F) {
    uint32_t nestingDepth = getLoopNestingDepth(&BB);
    maxNestingDepth = std::max(maxNestingDepth, nestingDepth);
    double loopScore = std::pow(kLoopMultiplier, (double)nestingDepth);

    for (Instruction &I : BB) {
      if (!I.hasOutput() || !RA_.isAllocated(&I))
        continue;
      Register reg = RA_.getRegister(&I);
      RegSortData &regTypeData =
          regTypes[(size_t)reg.getClass()][RA_.getHVMRegisterIndex(reg)];
      // Update the type of the register.
      regTypeData.reg = reg;
      // Add to the score for the result.
      regTypeData.score += loopScore;
      // Add to the score for each operand.
      for (unsigned i = 0, e = I.getNumOperands(); i < e; ++i) {
        Instruction *op = llvh::dyn_cast<Instruction>(I.getOperand(i));
        if (op && RA_.isAllocated(op)) {
          auto opReg = RA_.getRegister(op);
          regTypes[(size_t)opReg.getClass()][RA_.getHVMRegisterIndex(opReg)]
              .score += loopScore;
        }
      }
    }
  }

  // Need to skip the NoOutput registers.
  static_assert((int)RegClass::NoOutput == 0, "NoOutput must be 0");

  for (size_t regClass = 1; regClass < (size_t)RegClass::_last; ++regClass) {
    auto &rt = regTypes[regClass];
    std::sort(
        rt.begin(),
        rt.end(),
        [](const RegSortData &a, const RegSortData &b) -> bool {
          // Sort by score, then by register index.
          // Place higher numbers earlier.
          return std::tuple{a.score, a.reg.getIndexInClass()} >
              std::tuple{b.score, b.reg.getIndexInClass()};
        });
  }

  // Count the registers by type for the remapping.
  // Index is Register, value is index within its register class.
  llvh::SmallVector<uint32_t, 32> remapping[(size_t)RegClass::_last];
  for (size_t regClass = 1; regClass < (size_t)RegClass::_last; ++regClass) {
    remapping[regClass].resize(totalRegCount, UINT32_MAX);
    const auto &rt = regTypes[regClass];
    for (size_t i = 0, e = rt.size(); i < e; ++i) {
      Register reg = rt[i].reg;
      if (reg.isValid()) {
        assert(
            reg.getClass() == (RegClass)regClass && "Invalid register class");
        remapping[regClass][reg.getIndexInClass()] = i;
      }
    }
  }

  // Reassign registers according to the mapping.
  for (BasicBlock &BB : *F) {
    for (Instruction &I : BB) {
      if (!I.hasOutput() || !RA_.isAllocated(&I))
        continue;
      auto reg = RA_.getRegister(&I);
      uint32_t newIdx =
          remapping[(size_t)reg.getClass()][reg.getIndexInClass()];
      assert(newIdx != UINT32_MAX && "Register not remapped");
      RA_.updateRegister(&I, Register(reg.getClass(), newIdx));
    }
  }

  // Set the loop score to the max nesting depth.
  RA_.setMaxLoopDepth(maxNestingDepth);

  return true;
}

} // namespace hermes::hbc
