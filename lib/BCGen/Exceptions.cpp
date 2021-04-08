/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "exceptions"

#include "hermes/BCGen/Exceptions.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"

#include "llvh/Support/Debug.h"
#include "llvh/Support/raw_ostream.h"

using namespace hermes;

using llvh::isa;

/// Construct the list of basic blocks covered by each catch instruction.
/// Use recursion to handle nested catches.
bool hermes::constructCatchMap(
    Function *F,
    CatchInfoMap &catchInfoMap,
    llvh::SmallVectorImpl<CatchInst *> &aliveCatches,
    llvh::SmallPtrSetImpl<BasicBlock *> &visited,
    BasicBlock *currentBlock,
    uint32_t maxRecursionDepth) {
  if (maxRecursionDepth == 0) {
    F->getContext().getSourceErrorManager().error(
        F->getSourceRange(), "Too deeply nested try/catch");
    return false;
  }

  if (!visited.insert(currentBlock).second)
    return true;
  // TryEndInst can only show up at the beginning of a block;
  // TryStartInst can only show up at the end of a block.
  // Hence we process the block with the order:
  // TryEndInst => block body => TryStartInst => successors.
  bool isTryStartBlock = llvh::isa<TryStartInst>(currentBlock->getTerminator());
  bool isTryEndBlock = llvh::isa<TryEndInst>(&currentBlock->front());
  CatchInst *currentCatch = nullptr;

  if (isTryEndBlock) {
    // Hit a TryEndInst, marking the end of a try region, save the current
    // catch.
    currentCatch = aliveCatches.pop_back_val();
  }

  // For every nested try that's alive, we add the basic block into it.
  for (auto &aliveCatche : aliveCatches) {
    catchInfoMap[aliveCatche].coveredBlockList.push_back(currentBlock);
  }

  if (isTryStartBlock) {
    // Hit a TryStartInst, marking the start of a new try region.
    // The first instruction of the catch target block must be CatchInst.
    auto tryStartInst = cast<TryStartInst>(currentBlock->getTerminator());
    auto catchInst = cast<CatchInst>(&tryStartInst->getCatchTarget()->front());
    catchInfoMap[catchInst].depth = aliveCatches.size();

    // Pushing the CatchInst to the try stack, and continue scan the try body.
    aliveCatches.push_back(catchInst);
    if (!constructCatchMap(
            F,
            catchInfoMap,
            aliveCatches,
            visited,
            tryStartInst->getTryBody(),
            maxRecursionDepth - 1))
      return false;
    aliveCatches.pop_back();

    // We also want to continue scan into the catch blocks.
    if (!constructCatchMap(
            F,
            catchInfoMap,
            aliveCatches,
            visited,
            tryStartInst->getCatchTarget(),
            maxRecursionDepth - 1))
      return false;
  } else {
    // No TryStartInst, we iterate successors normally.
    for (auto itr = succ_begin(currentBlock), e = succ_end(currentBlock);
         itr != e;
         ++itr) {
      if (!constructCatchMap(
              F,
              catchInfoMap,
              aliveCatches,
              visited,
              *itr,
              maxRecursionDepth - 1))
        return false;
    }
  }

  if (isTryEndBlock) {
    // After coming back from the recursion, we recover the stack.
    assert(currentCatch && "currentCatch is null when there is TryEndInst");
    aliveCatches.push_back(currentCatch);
  }
  return true;
}

ExceptionEntryList hermes::generateExceptionHandlers(
    CatchInfoMap &catchInfoMap,
    BasicBlockInfoMap &bbMap,
    Function *F) {
  // Construct the list of blocks and depth covered by each CatchInst.
  llvh::SmallVector<CatchInst *, 4> aliveCatches{};
  llvh::SmallPtrSet<BasicBlock *, 32> visited{};
  static constexpr uint32_t MAX_RECURSION_DEPTH = 1024;
  if (!constructCatchMap(
          F,
          catchInfoMap,
          aliveCatches,
          visited,
          &F->front(),
          MAX_RECURSION_DEPTH))
    return {};

  ExceptionEntryList exception_entries;
  for (auto I : catchInfoMap) {
    auto &catchInfo = I.second;
    // The basic blocks covered by a catch instruction may not be continuous.
    // For each basic block, we walk through the current list of ranges,
    // and try to merge them into a minimum number of ranges.
    llvh::SmallVector<std::pair<uint32_t, uint32_t>, 4> catch_ranges;
    for (auto BB : catchInfo.coveredBlockList) {
      auto it = bbMap.find(BB);
      assert(it != bbMap.end() && "Basic Block missing.");

      auto resolved_loc = it->second;
      if (resolved_loc.first == resolved_loc.second) {
        // Empty basic block, skip.
        continue;
      }
      catch_ranges.push_back(resolved_loc);
    }
    std::sort(catch_ranges.begin(), catch_ranges.end());
    // After ranges are sorted, a range could only be merged with it's
    // previous range, if they are adjacent.
    // Note: no range can overlap, as basic blocks do not overlap.
    int nextIndex = 0;
    for (auto resolved_loc : catch_ranges) {
      // If we are looking at the first range, or the range cannot
      // be merged with the previous range, we store this range.
      if (nextIndex == 0 ||
          catch_ranges[nextIndex - 1].second != resolved_loc.first) {
        catch_ranges[nextIndex++] = resolved_loc;
        continue;
      }
      // Otherwise we merge with the previous range.
      catch_ranges[nextIndex - 1].second = resolved_loc.second;
    }
    // The merging happened in-place. Hence we need to throw away the rest.
    catch_ranges.resize(nextIndex);
    // For each range, we register it as an exception handler entry.
    for (auto range : catch_ranges) {
      exception_entries.push_back(
          {(uint32_t)range.first,
           (uint32_t)range.second,
           (uint32_t)catchInfo.catchLocation,
           catchInfo.depth});
    }
  }
  // Sort ranges by depth. In hermes, depth increase when you nest try inside
  // try/catch/finally.
  std::sort(exception_entries.begin(), exception_entries.end());
  return exception_entries;
}

#undef DEBUG_TYPE
