//===---- Outliner.cpp - Outline instructions -------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "llvm_extra/Outliner.h"
#include "llvm_extra/SuffixTree.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <functional>

#define DEBUG_TYPE "outliner"

namespace llvm {
namespace outliner {

/// Find all repeated substrings that satisfy the outlining cost model.
///
/// If a substring appears at least twice, then it must be represented by an
/// internal node which appears in at least two suffixes. Each suffix is
/// represented by a leaf node. To do this, we visit each internal node in the
/// tree that has at least two leaf children. If an internal node represents a
/// beneficial substring, then we use each of its leaf children to find the
/// locations of its substring.
///
/// Technically any internal node with two leaf *descendants* qualifies.
/// However, these would just be substrings of larger repeated substrings. So we
/// only consider internal nodes with at least two leaf *children*.
///
/// \param[out] FunctionList Filled with a list of \p OutlinedFunctions.
/// \param[out] CandidateList Filled with candidate representing each
/// beneficial substring. The \p Candidate objects these point to are owned by
/// the \p OutlinedFunctions in \p FunctionList.
/// \param ST A suffix tree to query.
/// \param Target Object that implements target-specific behaviour.
///
/// \returns The length of the longest candidate found.
static unsigned findCandidates(
    std::vector<OutlinedFunction> &FunctionList,
    std::vector<Candidate *> &CandidateList,
    SuffixTree &ST,
    OutlinerTarget *Target) {
  FunctionList.clear();
  CandidateList.clear();

  const unsigned UnsignedVecSize = ST.Str.size();
  const unsigned MinBeneficialLen = Target->minCandidateLength();

  unsigned MaxLen = 0;
  for (SuffixTreeNode *Parent : ST.MultiLeafParents) {
    assert(Parent && "Nodes in MultiLeafParents cannot be null!");
    unsigned StringLen = Parent->ConcatLen;

    // Too short to be beneficial; skip it.
    // FIXME: This isn't necessarily true for, say, X86. If we factor in
    // instruction lengths we need more information than this.
    if (StringLen < MinBeneficialLen) {
      continue;
    }

    // We have a repeated sequence \p StringLen instructions long. Each leaf
    // child of \p Parent represents one occurrence of it. Below, we build a
    // list of their starting indices.
    SmallVector<unsigned, 4> CandidateStartIndices;
    for (auto &ChildPair : Parent->Children) {
      SuffixTreeNode *M = ChildPair.second;
      assert(M && "Child node is null!");

      if (M->isLeaf()) {
        const unsigned StartIdx = UnsignedVecSize - M->ConcatLen;
        const unsigned EndIdx = StartIdx + StringLen - 1;

        // Trick: Discard some candidates that would be incompatible with the
        // ones we've already found for this sequence. This will save us some
        // work in candidate selection.
        //
        // If two candidates overlap, then we can't outline them both. This
        // happens when we have candidates that look like, say
        //
        // AA (where each "A" is an instruction).
        //
        // We might have some portion of the module that looks like this:
        // AAAAAA (6 A's)
        //
        // In this case, there are 5 different copies of "AA" in this range, but
        // at most 3 can be outlined. If only outlining 3 of these is going to
        // be unbeneficial, then we ought to not bother.
        //
        // Note that two things DON'T overlap when they look like this:
        // start1...end1 .... start2...end2
        // That is, one must either
        // * End before the other starts
        // * Start after the other ends
        if (std::all_of(
                CandidateStartIndices.begin(),
                CandidateStartIndices.end(),
                [StartIdx, EndIdx, StringLen](const unsigned &OtherStartIdx) {
                  const unsigned OtherEndIdx = OtherStartIdx + StringLen - 1;
                  return (EndIdx < OtherStartIdx || StartIdx > OtherEndIdx);
                })) {
          // It doesn't overlap with anything, so we can outline it. Save it.
          CandidateStartIndices.push_back(StartIdx);
        }
      }
    }

    // Not worth outlining if there aren't at least two candidates.
    if (CandidateStartIndices.size() < 2) {
      continue;
    }

    std::vector<OutlinedFunction> NewFunctions;
    Target->createOutlinedFunctions(
        NewFunctions, CandidateStartIndices, StringLen);
    for (OutlinedFunction &OF : NewFunctions) {
      if (OF.getBenefit() < 1) {
        // Not worth outlining.
        continue;
      }

      const unsigned CandidateLen = OF.Candidates[0].getLength();
      if (CandidateLen > MaxLen) {
        MaxLen = CandidateLen;
      }

      const unsigned FuctionIdx = FunctionList.size();
      FunctionList.push_back(std::move(OF));
      // Save candidates to CandidateList for pruning, and set FunctionIdx.
      for (Candidate &C : FunctionList.back().Candidates) {
        C.FunctionIdx = FuctionIdx;
        CandidateList.push_back(&C);
        assert(
            C.getLength() == CandidateLen &&
            "Candidates are not all the same length!");
      }
    }
  }

  return MaxLen;
}

/// Remove overlapping candidates.
///
/// The \p findCandidates function doesn't necessarily remove all overlaps. It
/// only removes overlaps among the children of a single node. For example, if a
/// short candidate is chosen for outlining, then a longer candidate which has
/// that short candidate as a suffix is chosen, that case will not be handled.
/// Therefore we need a separate pruning step to get rid of all overlaps.
///
/// \param[in,out] FunctionList A list of functions to be outlined.
/// \param[in,out] CandidateList A list of outlining candidates sorted in
/// descending order of \p StartIdx.
/// \param MaxCandidateLen The length of the longest candidate.
static void pruneOverlaps(
    std::vector<OutlinedFunction> &FunctionList,
    std::vector<Candidate *> &CandidateList,
    unsigned MaxCandidateLen) {
  // Return true if this candidate became unbeneficial for outlining in a
  // previous step.
  auto ShouldSkipCandidate = [&FunctionList](Candidate &C) {
    // Check if the candidate was removed in a previous step.
    if (C.isDeleted()) {
      return true;
    }

    // C must be alive. Check if we should remove it.
    if (FunctionList[C.FunctionIdx].getBenefit() < 1) {
      C.markDeleted();
      return true;
    }

    // C is in the list, and the outlined function is still beneficial.
    return false;
  };

  // TODO: Experiment with interval trees or other interval-checking structures
  // to lower the time complexity of this function.
  // TODO: Can we do better than the simple greedy choice?

  // Check for overlaps in the range.
  // This is O(MaxCandidateLen * CandidateList.size()).
  for (auto It = CandidateList.begin(), Et = CandidateList.end(); It != Et;
       ++It) {
    Candidate &C1 = **It;

    // If C1 was already pruned, or its function is no longer beneficial for
    // outlining, move to the next candidate.
    if (ShouldSkipCandidate(C1)) {
      continue;
    }

    // Minimum start index of any candidate that could overlap with this one.
    unsigned FarthestPossibleIdx = 0;

    // Either the index is 0, or it's at most MaxCandidateLen indices away.
    if (C1.getStartIdx() > MaxCandidateLen) {
      FarthestPossibleIdx = C1.getStartIdx() - MaxCandidateLen;
    }

    // Compare against the candidates in the list that start at most
    // FarthestPossibleIdx indices away from C1. There are at most
    // MaxCandidateLen of these.
    for (auto Sit = It + 1; Sit != Et; ++Sit) {
      Candidate &C2 = **Sit;

      // Is this candidate too far away to overlap?
      if (C2.getStartIdx() < FarthestPossibleIdx) {
        break;
      }

      // If C2 was already pruned, or its function is no longer beneficial for
      // outlining, move to the next candidate.
      if (ShouldSkipCandidate(C2)) {
        continue;
      }

      // Do C1 and C2 overlap?
      //
      // Not overlapping:
      // High indices... [C1End ... C1Start][C2End ... C2Start] ...Low indices
      //
      // We sorted our candidate list so C2Start <= C1Start. We know that
      // C2End > C2Start since each candidate has length >= 2. Therefore, all we
      // have to check is C2End < C2Start to see if we overlap.
      if (C2.getEndIdx() < C1.getStartIdx()) {
        continue;
      }

      // C1 and C2 overlap.
      // We need to choose the better of the two.
      //
      // Approximate this by picking the one which would have saved us the
      // most instructions before any pruning.
      // TODO: Why not just call getBenefit() and recalculate it?

      // Is C2 a better candidate?
      if (C2.CachedBenefit > C1.CachedBenefit) {
        // Yes, so prune C1. Since C1 is dead, we don't have to compare it
        // against anything anymore, so break.
        C1.markDeleted();
        break;
      }

      // Prune C2 and move on to the next candidate.
      C2.markDeleted();
    }
  }
}

void getFunctionsToOutline(
    std::vector<OutlinedFunction> &FunctionList,
    ArrayRef<unsigned> UnsignedVec,
    OutlinerTarget *Target) {
  // Construct a suffix tree and use it to find candidates.
  SuffixTree ST(UnsignedVec);
  std::vector<Candidate *> CandidateList;
  unsigned MaxCandidateLen =
      findCandidates(FunctionList, CandidateList, ST, Target);

  // Sort the candidates in descending order of \p StartIdx. This is required by
  // \p pruneOverlaps so that it can be more efficient.
  // TODO: Why does the sort need to be stable?
  std::stable_sort(
      CandidateList.begin(),
      CandidateList.end(),
      [](const Candidate *const &LHS, const Candidate *const &RHS) {
        return LHS->getStartIdx() > RHS->getStartIdx();
      });

  // Remove candidates that overlap with other candidates.
  pruneOverlaps(FunctionList, CandidateList, MaxCandidateLen);
};

} // namespace outliner
} // namespace llvm
