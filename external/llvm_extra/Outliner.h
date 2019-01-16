//===---- Outliner.h - Outliner data structures -----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Contains all data structures shared between the outliner implemented in
/// Outliner.cpp and target implementations of the outliner.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_EXTRA_OUTLINER_H
#define LLVM_EXTRA_OUTLINER_H

#include "llvm/ADT/ArrayRef.h"

#include <cassert>
#include <utility>
#include <vector>

namespace llvm {
namespace outliner {

/// An individual sequence of instructions to be replaced with a call to an
/// outlined function.
struct Candidate {
 private:
  /// The start index of this candidate in the instruction list.
  const unsigned StartIdx;

  /// The number of instructions in this candidate.
  const unsigned Len;

  /// Set to true to mark the candidate as deleted without having to remove it
  /// from the \p OutlinedFunction's \p Candidates vector.
  bool Deleted = false;

 public:
  /// Target-defined cost of calling an outlined function from this point.
  const unsigned CallOverhead;

  /// The index of this candidate's \p OutlinedFunction in the list of
  /// \p OutlinedFunctions. We store the index rather than a pointer to avoid
  /// invalidation when the vector grows.
  unsigned FunctionIdx;

  /// Cached value of \p OF.getBenefit() where \p OF is the \p OutlinedFunction
  /// this candidate belongs to.
  ///
  /// This is a fixed value which is not updated during the candidate pruning
  /// process. It is only used for deciding which candidate to keep if two
  /// candidates overlap.
  unsigned CachedBenefit = 0;

  /// Return the number of instructions in this Candidate.
  unsigned getLength() const {
    return Len;
  }

  /// Return the start index of this candidate.
  unsigned getStartIdx() const {
    return StartIdx;
  }

  /// Return the end index of this candidate.
  unsigned getEndIdx() const {
    return StartIdx + Len - 1;
  }

  /// Returns true if this candidate is marked as deleted.
  bool isDeleted() const {
    return Deleted;
  }

  /// Mark this candidate as deleted.
  void markDeleted() {
    Deleted = true;
  }

  Candidate(unsigned StartIdx, unsigned Len, unsigned CallOverhead)
      : StartIdx(StartIdx), Len(Len), CallOverhead(CallOverhead) {}
};

/// The information necessary to create an outlined function for some class of
/// candidate.
struct OutlinedFunction {
 public:
  /// The candidates that would be replaced by a call to this function.
  ///
  /// NOTE: Candidates for which \p isDeleted() returns true should be ignored.
  std::vector<Candidate> Candidates;

  /// Target-defined size of the sequence being outlined.
  const unsigned SequenceSize;

  /// Target-defined overhead of constructing a frame for this function.
  const unsigned FrameOverhead;

  /// Return the target-defined benefit of outlining this function compared to
  /// not outlining it.
  unsigned getBenefit() const {
    unsigned NotOutlinedCost = 0;
    unsigned OutlinedCost = FrameOverhead + SequenceSize;
    for (const Candidate &C : Candidates) {
      if (!C.isDeleted()) {
        NotOutlinedCost += SequenceSize;
        OutlinedCost += C.CallOverhead;
      }
    }
    // The benefit of outlining is the amount by which it reduces cost.
    return NotOutlinedCost < OutlinedCost ? 0 : NotOutlinedCost - OutlinedCost;
  }

  OutlinedFunction(
      std::vector<Candidate> Cands,
      unsigned SequenceSize,
      unsigned FrameOverhead)
      : Candidates(std::move(Cands)),
        SequenceSize(SequenceSize),
        FrameOverhead(FrameOverhead) {
    assert(Candidates.size() >= 2 && "Should be at least two candidates!");
    assert(SequenceSize > 0 && "SequenceSize should be nonzero!");
    const unsigned B = getBenefit();
    for (Candidate &C : Candidates) {
      assert(
          C.getLength() == Candidates.front().getLength() &&
          "Candidates are not all the same length!");
      C.CachedBenefit = B;
    }
  }

  // Prevent copies, so that we don't accidentally copy an OutlinedFunction and
  // its Candidates vector, invalidating references to candidates.
  OutlinedFunction(const OutlinedFunction &) = delete;
  OutlinedFunction(OutlinedFunction &&) = default;
  OutlinedFunction &operator=(OutlinedFunction &&) = default;
};

/// An interface for targets to hook into the outlining process.
class OutlinerTarget {
 public:
  virtual ~OutlinerTarget() = default;

  /// Return the minimum candidate length worth outlining, in terms of number
  /// of instructions.
  virtual unsigned minCandidateLength() = 0;

  /// Create zero or more \p OutlinedFunctions given potential candidates.
  ///
  /// The potential candidates are sections of the unsigned vector starting at
  /// the indices given in \p StartIndices. They are all of the same length,
  /// \p Len, and they do not overlap.
  ///
  /// This interface allows the target to be flexible. If suffix tree matches
  /// can always be outlined, then this can just create one \p OutlinedFunction
  /// with all the candidates. If there are additional constraints on outlining,
  /// then the implementation might create a \p OutlinedFunction that only
  /// includes some of the candidates, or a substring of each candidate.
  ///
  /// \param[out] Functions Output parameter for the \p OutlinedFunctions.
  /// \param StartIndices Indices in the unsigned vector where candidates start.
  /// \param Len The length of each candidate. At least minCandidateLength().
  virtual void createOutlinedFunctions(
      std::vector<OutlinedFunction> &Functions,
      ArrayRef<unsigned> StartIndices,
      unsigned Len) = 0;
};

/// Find opportunities for outlining and store the results in \p FunctionList.
///
/// Specifically, it searches \p UnsignedVec for non-overlapping repeated
/// sequences, and then creates \p OutlinedFunction structs with information to
/// build functions that factor out the repetitions.
///
/// The caller should create \p UnsignedVec by mapping instructions to unsigned
/// integers using some definition of instruction equivalence.
///
/// \param[out] FunctionList A list of functions to create.
/// \param UnsignedVec The code to search, as a string of integers.
/// \param Target Object that implements target-specific behaviour.
void getFunctionsToOutline(
    std::vector<OutlinedFunction> &FunctionList,
    ArrayRef<unsigned> UnsignedVec,
    OutlinerTarget *Target);

} // namespace outliner
} // namespace llvm

#endif // LLVM_EXTRA_OUTLINER_H
