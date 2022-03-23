/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_PROFILER_BB

#ifndef HERMES_VM_BASICBLOCKEXECUTIONINFO_H
#define HERMES_VM_BASICBLOCKEXECUTIONINFO_H

#include "llvh/ADT/DenseMap.h"
#include "llvh/Support/raw_ostream.h"

namespace hermes {
namespace vm {

class Runtime;
class CodeBlock;

/// Stores basic blocks execution statistics of runtime.
/// There is one BasicBlockExecutionInfo per runtime and one
/// FunctionStatisticMap per function; inside FunctionStatisticMap each basic
/// block's profile point index is used to key into the vector array.
/// The first entrypoint basic block will have the biggest profile index.
class BasicBlockExecutionInfo {
  /// Each vector entry is a <basic_block_execution_count,
  /// global_sequence_number> pair.
  /// basic_block_execution_count: the number of times this basic block
  /// executes.
  /// global_sequence_number: global sequence number when this basic block
  /// executes the first time(1-based).
  using FunctionStatisticMap = std::vector<std::pair<uint64_t, uint64_t>>;
  /// Maps from function's CodeBlock to its profiling statistics.
  using RuntimeStatisticMap = llvh::DenseMap<CodeBlock *, FunctionStatisticMap>;

 private:
  /// A global sequence number to assign the next basic block first time
  /// execution order.
  uint64_t nextBasicBlockSequenceNumber_{1};

  /// Storage table maps from function CodeBlock to its basic blocks execution
  /// statistics.
  RuntimeStatisticMap basicBlockStats_;

  /// Caches last entered code block.
  CodeBlock *lastCodeBlock_{nullptr};

  /// Caches last entered code block's statistics map.
  FunctionStatisticMap *lastFuncStatMap_{nullptr};

  /// Get the function statistics map for \p curCodeBlock.
  FunctionStatisticMap &getFuncStat(CodeBlock *curCodeBlock) {
    // Update the cache if codeblock changed.
    if (LLVM_UNLIKELY(curCodeBlock != lastCodeBlock_)) {
      lastCodeBlock_ = curCodeBlock;
      lastFuncStatMap_ = &basicBlockStats_[curCodeBlock];
    }
    assert(
        lastFuncStatMap_ != nullptr && "lastFuncStatMap_ should not be null.");
    return *lastFuncStatMap_;
  }

  /// Resize empty \p funcStat using \entrypointIndex(entry point basic block's
  /// profile index).
  void resizeFuncStatMap(
      FunctionStatisticMap &funcStat,
      uint16_t entrypointIndex);

 public:
  /// Called when a basic block begins execution.
  void executeBlock(CodeBlock *curCodeBlock, uint16_t pointIndex) {
    FunctionStatisticMap &funcStat = getFuncStat(curCodeBlock);
    // Initialize the stats table when executing the first basic
    // block(entrypoint) of a function.
    if (LLVM_UNLIKELY(funcStat.empty())) {
      resizeFuncStatMap(funcStat, pointIndex);
    }
    assert(pointIndex < funcStat.size() && "Why is the table not big enough?");
    auto &blockStat = funcStat[pointIndex];
    if (blockStat.first == 0) {
      // Record block's ordering when it executes the first time.
      assert(blockStat.second == 0);
      assert(
          nextBasicBlockSequenceNumber_ != 0 &&
          "nextBasicBlockSequenceNumber overflows!");
      blockStat.second = nextBasicBlockSequenceNumber_++;
    }
    ++blockStat.first;
  }

  /// Dump the statistics to \p OS in json format.
  void dump(llvh::raw_ostream &OS);
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_BASICBLOCKEXECUTIONINFO_H
#endif // HERMESVM_PROFILER_BB
