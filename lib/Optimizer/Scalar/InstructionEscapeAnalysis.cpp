/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "instructionescapeanalysis"

#include "hermes/Optimizer/Scalar/InstructionEscapeAnalysis.h"

#include "llvh/Support/Casting.h"

#include <utility>

namespace hermes {

// Iterate through the range, keeping track of instruction uses in a hash map.
// For an instruction I with N users, insert a mapping from I to N. Then, for
// each of its operands X, decrement the user count of X in the map and remove
// the entry if it reaches zero. The points where the map size is <= 1 represent
// the prefixes that work (have at most one escaping value).
//
// Example:
//
//     Instructions    Map                 Map Size    Prefix Lengths
//     X = Y + Z       {X -> 2}            1           [1]
//     W = X * X       {X -> 1, W -> 1}    2           [1]
//     A = B + X       {A -> 1, W -> 1}    2           [1]
//     Q = W - A       {}                  0           [1, 4]
//
// When the function is called on another range, it does the same thing, but
// only includes prefixes that are also compatible with the first range. It
// achieves this by reading from prefixes_ (the old results) and writing to
// auxPrefixes_ (the new results), and then swapping them at the end.
void InstructionEscapeAnalysis::addRange(BasicBlock::range range) {
  assert(range.begin() != range.end() && "Range must be nonempty!");
  auxPrefixes_.lengthToOffset.clear();

  // A value at a given offset that escapes to numUsers later locations.
  struct Escape {
    unsigned offset;
    unsigned numUsers;
  };
  llvh::DenseMap<const Instruction *, Escape> escapes;

  unsigned instructionIndex = 0;
  for (auto iter = range.begin(), end = range.end(); iter != end;
       ++iter, ++instructionIndex) {
    Instruction *inst = &*iter;
    // Decrement user counts for this instruction's operands, since they are no
    // longer considered escapes when the prefix is extended to include inst.
    for (unsigned i = 0, e = inst->getNumOperands(); i < e; ++i) {
      Value *op = inst->getOperand(i);
      if (auto *opInst = llvh::dyn_cast<Instruction>(op)) {
        const auto it = escapes.find(opInst);
        if (it != escapes.end()) {
          assert(it->second.numUsers >= 1 && "Invalid entry in escapes!");
          if (--it->second.numUsers == 0) {
            escapes.erase(it);
          }
        }
      }
    }

    // Insert this instruction's escape information.
    const auto numUsers = inst->getNumUsers();
    if (numUsers > 0) {
      auto insert =
          escapes.try_emplace(inst, Escape{instructionIndex, numUsers});
      (void)insert;
      assert(insert.second && "Failed to insert instruction in escapes map!");
    }

    // See if we have a new candidate prefix.
    const auto numEscapes = escapes.size();
    if (numEscapes <= 1) {
      const auto prefixLength = instructionIndex + 1;
      auto escapeOffset = numEscapes == 0
          ? llvh::None
          : EscapeOffset(escapes.begin()->second.offset);
      // For the first range (when numRanges_ is 0), all prefixes are valid. For
      // subsequent ranges, make sure the prefix is compatible with past ranges.
      const auto it = prefixes_.lengthToOffset.find(prefixLength);
      if (numRanges_ == 0 ||
          (it != prefixes_.lengthToOffset.end() &&
           tryMergeOffsets(escapeOffset, it->second))) {
        auto insert =
            auxPrefixes_.lengthToOffset.try_emplace(prefixLength, escapeOffset);
        (void)insert;
        assert(insert.second && "Failed to insert prefix!");
        auxPrefixes_.longest = Prefix(prefixLength, escapeOffset);
      }
    }
  }
  assert(
      auxPrefixes_.longest.length > 0 &&
      "Nonempty range must have a prefix of length 1");

  // We've finished writing new information from prefixes_ into auxPrefixes_, so
  // swap them to make prefixes_ hold to the new data.
  swap(prefixes_, auxPrefixes_);
  ++numRanges_;
  canRemove_ = true;
}

void InstructionEscapeAnalysis::removeLastRange() {
  assert(numRanges_ > 0 && "No ranges added yet!");
  assert(canRemove_ && "removeLastRange() called more than once!");
  // After running addRange, prefixes_ contains the new prefixes and
  // auxPrefixes_ contains the old ones. Revert to the old state by swapping.
  swap(prefixes_, auxPrefixes_);
  // Clear the auxiliary map, since there's no need for it anymore.
  auxPrefixes_.lengthToOffset.clear();
  --numRanges_;
  canRemove_ = false;
}

const InstructionEscapeAnalysis::Prefix &
InstructionEscapeAnalysis::longestPrefix() const {
  assert(numRanges_ > 0 && "No ranges added yet!");
  return prefixes_.longest;
}

// Two EscapeOffsets can be merged except when they both have a value and those
// values are different. If one has a value and the other doesn't, we merge them
// by storing the one with a value in \p acc.
bool InstructionEscapeAnalysis::tryMergeOffsets(
    EscapeOffset &acc,
    EscapeOffset offset) {
  if (!acc.hasValue()) {
    acc = offset;
    return true;
  }
  if (!offset.hasValue()) {
    return true;
  }
  return acc == offset;
}

} // namespace hermes

#undef DEBUG_TYPE
