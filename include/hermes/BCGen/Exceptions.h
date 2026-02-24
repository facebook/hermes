/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_EXCEPTIONS_H
#define HERMES_BCGEN_EXCEPTIONS_H

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/SmallPtrSet.h"
#include "llvh/ADT/SmallVector.h"

namespace hermes {

class BasicBlock;
class Function;
class CatchInst;

/// Maps catch instructions to the list of basic blocks that the catch covers
/// and other metadata that's needed for code generation of the catch
/// instruction.
class CatchCoverageInfo {
 public:
  /// The bytecode location of the catch instruction.
  int catchLocation{0};

  /// The list of basic blocks covered by this catch.
  llvh::SmallVector<BasicBlock *, 8> coveredBlockList{};

  /// The depth of this catch in nesting.
  unsigned depth{0};

  explicit CatchCoverageInfo(int location) : catchLocation{location} {}
  explicit CatchCoverageInfo() = default;
};

//===----------------------------------------------------------------------===//
// Exception Handler Data Structure
//===----------------------------------------------------------------------===//
struct ExceptionHandlerInfo {
  uint32_t start;
  uint32_t end;
  uint32_t target;
  uint32_t depth;

  /// We want to keep deeper ranges to show up earlier, but we also want to
  /// sort all handlers reproducibly.
  bool operator<(const ExceptionHandlerInfo &rhs) const {
    assert(
        (this == &rhs || depth != rhs.depth || start != rhs.start) &&
        "two handlers with equal depth and start");
    return depth > rhs.depth || (depth == rhs.depth && start < rhs.start);
  }
};

/// Maps catch instructions to the basic blocks it covers.
using CatchInfoMap = llvh::DenseMap<CatchInst *, CatchCoverageInfo>;

//// Maps a basic block to it's beginning and end location in the bytecode.
using BasicBlockInfoMap =
    llvh::DenseMap<BasicBlock *, std::pair<uint32_t, uint32_t>>;

/// A list of exception handler table entries.
using ExceptionEntryList = llvh::SmallVector<ExceptionHandlerInfo, 4>;

/// Note: Returns an empty list if generation fails due to excessive recursion.
ExceptionEntryList generateExceptionHandlers(
    CatchInfoMap &catchInfoMap,
    BasicBlockInfoMap &bbMap,
    Function *F);

} // namespace hermes

#endif
