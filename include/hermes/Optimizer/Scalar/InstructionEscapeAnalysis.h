/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_OPTIMIZER_SCALAR_INSTRUCTION_ESCAPE_ANALYSIS_H
#define HERMES_OPTIMIZER_SCALAR_INSTRUCTION_ESCAPE_ANALYSIS_H

#include "hermes/IR/IR.h"
#include "hermes/Support/OptValue.h"

#include <cassert>

namespace hermes {

/// Analysis that finds sections of blocks where at most one value escapes.
///
/// This class accepts a range of instructions within a basic block and finds
/// prefixes of the range where at most one value escapes. A value "escapes" if
/// the instruction value is used as an operand somewhere outside the range.
///
/// In addition, the analysis supports adding more ranges of instructions and
/// reporting prefixes lengths that are compatible with all ranges analyzed so
/// far. A prefix length is compatible with N ranges of instructions if (1) the
/// length does not exceed the length of the shortest range, (2) the prefix of
/// each range has at most one escaping value, and (3) the prefixes that have
/// exactly one escaping value have that value at the same offset.
///
/// This is useful for identifying ranges of instructions that can be refactored
/// into a function with a single return value.
///
/// Here is an example of a block of pseudocode instructions, with annotations
/// indicating which values escape from each prefix of the range:
///
/// \code
///     (L1) | A = X + 1
///          +-----------> escapes: [A on L3]
///     (L2) | B = 2 * 3
///          +-----------> escapes: [A on L3, B on L3]
///     (L3) | C = A - B
///          +-----------> escapes: [C on L4/L5]
///     (L4) | D = C * C
///          +-----------> escapes: [C on L5, D on L5/L6]
///     (L5) | E = sin(C)
///          +-----------> escapes: [D on L6]
///     (L6) | F = cos(D)
///          +-----------> escapes: []
/// \endcode
///
/// In this case, the prefixes of length 1, 3, 5, and 6 have at most one value
/// escaping. If the input range were just L1-L4, then this class would report
/// L1-L3 as the best (longest) prefix. Notice that a value only counts as one
/// escape even if it's used in multiple locations after the prefix.
class InstructionEscapeAnalysis {
 public:
  explicit InstructionEscapeAnalysis() = default;

  /// An EscapeOffset is either an unsigned integer representing the offset of
  /// the instruction that escapes, or None because no instructions escape.
  using EscapeOffset = OptValue<unsigned>;

  /// A prefix of a range of instructions with at most one escaping value.
  struct Prefix {
    /// Length of the prefix (number of instructions).
    unsigned length;
    /// Offset of the single escaping instruction, or None.
    EscapeOffset offset;

    explicit Prefix(unsigned length, EscapeOffset offset = llvh::None)
        : length(length), offset(offset) {
      assert(
          (!offset.hasValue() || offset.getValue() < length) &&
          "Offset out of bounds!");
    }

    bool operator==(const Prefix &other) const {
      return length == other.length && offset == other.offset;
    }
  };

  /// Add a range to the analysis.
  ///
  /// \param range Range of instructions. Must be nonempty.
  void addRange(BasicBlock::range range);

  /// Remove the last added range. Must call addRange at least once before each
  /// call to this function. It cannot be called twice in a row.
  void removeLastRange();

  /// \return The longest Prefix compatible with all ranges added so far. Must
  /// have added at least one range first.
  const Prefix &longestPrefix() const;

 private:
  /// Combine \p offset into the accumulator \p acc. Return true if successful,
  /// and false if they are incompatible.
  static bool tryMergeOffsets(EscapeOffset &acc, EscapeOffset offset);

  /// Number of ranges added so far.
  unsigned numRanges_{0};
  /// True if the last operation was addRange, false if it was removeLastRange.
  /// This is used to ensure that removeLastRange is not called twice in a row.
  bool canRemove_{false};

  /// Data structure containing the prefixes compatible with a group of ranges,
  /// stored in a way that makes intersection with a new range efficient.
  struct PrefixSet {
    /// Mapping from prefix lengths to EscapeOffsets.
    llvh::DenseMap<unsigned, EscapeOffset> lengthToOffset;
    /// The longest prefix in the map.
    Prefix longest{0};

    friend void swap(PrefixSet &lhs, PrefixSet &rhs) {
      using std::swap;
      swap(lhs.lengthToOffset, rhs.lengthToOffset);
      swap(lhs.longest, rhs.longest);
    }
  };

  /// The primary set, containing the prefixes for all ranges added so far.
  PrefixSet prefixes_;
  /// The auxilliary set, containing the contents of prefixes_ before the last
  /// call to addRange, and to be used as scratch space on the next call.
  PrefixSet auxPrefixes_;
};

} // namespace hermes

#endif // HERMES_OPTIMIZER_SCALAR_INSTRUCTION_ESCAPE_ANALYSIS_H
