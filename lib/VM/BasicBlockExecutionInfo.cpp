/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_PROFILER_BB

#include "hermes/VM/BasicBlockExecutionInfo.h"

#include "hermes/Support/JSONEmitter.h"
#include "hermes/Support/MD5.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/Runtime.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/Support/MD5.h"

#include <deque>
#include <string>

const static int32_t BASIC_BLOCK_STAT_VERSION = 2;

namespace hermes {
namespace vm {

void BasicBlockExecutionInfo::resizeFuncStatMap(
    FunctionStatisticMap &funcStat,
    uint16_t pointIndex) {
  assert(funcStat.empty());
  // If entrypoint block's pointIndex is zero(index overflow) we know
  // there are more than 2^16 basic blocks and overflowed blocks share
  // index zero so resize to maximum block number.
  // Note: in this case the stats table is inaccurate - all overflowed blocks
  // share the zero-index entry.
  const uint32_t kMaxBlockNumbers = 1 << 16;
  funcStat.resize(pointIndex == 0 ? kMaxBlockNumbers : pointIndex + 1);
}

void BasicBlockExecutionInfo::dump(llvh::raw_ostream &OS) {
  JSONEmitter json(OS);
  json.openDict();
  json.emitKeyValue("version", BASIC_BLOCK_STAT_VERSION);
  json.emitKeyValue("page_size", (double)hermes::oscompat::page_size());

  json.emitKey("functions");
  json.openArray();

  using CodeChecksumPair = std::pair<CodeBlock *, llvh::SmallString<32>>;
  std::deque<CodeChecksumPair> codeBlocksAndMD5;
  // We want to detect collisions.
  llvh::DenseMap<llvh::StringRef, unsigned> checksumToIndex;
  for (const auto &funcEntry : basicBlockStats_) {
    const llvh::SmallString checksum =
        doMD5Checksum(
            funcEntry.first->getFunctionID(), funcEntry.first->getOpcodeArray())
            .digest();
    llvh::StringRef checksumRef =
        codeBlocksAndMD5.emplace_back(funcEntry.first, std::move(checksum))
            .second;

    auto iter = checksumToIndex.find(checksumRef);
    if (iter != checksumToIndex.end()) {
      hermes_fatal(
          (std::string(
               "There was a collision on the MD5 hashes for two functions.\n") +
           "Checksum: " + checksum +
           ", indexes: " + std::to_string(iter->second) + ", " +
           std::to_string(funcEntry.first->getFunctionID()) + ".\n" +
           "This should be extremely unlikely; it's probably a bug.\n"
           "Results from the profile written are suspect.\n")
              .str());
    } else {
      checksumToIndex[checksumRef] = funcEntry.first->getFunctionID();
    }
  }

  // It's not necessary for correctness to sort these, but doing so
  // makes it easier to verify that identical runs produce identical
  // profiles.
  std::sort(
      codeBlocksAndMD5.begin(),
      codeBlocksAndMD5.end(),
      [](const CodeChecksumPair &a, const CodeChecksumPair &b) {
        return a.second < b.second;
      });

  for (const auto &pair : codeBlocksAndMD5) {
    CodeBlock *codeBlock = pair.first;
    json.openDict();

    json.emitKeyValue("checksum", pair.second);

    // hbcdump will be responsible to check overflow scenario(index-zero entry
    // is not empty).
    const auto &funcStat = basicBlockStats_.at(codeBlock);

    json.emitKey("basic_blocks");
    uint16_t profileIndex = 0;
    json.openArray();
    for (const auto &blockStat : funcStat) {
      json.openDict();
      // profile_index is used as the identifier for basic block.
      json.emitKeyValue("profile_index", profileIndex);
      json.emitKeyValue("execution_count", (double)blockStat.first);
      json.emitKeyValue("order", (double)blockStat.second);
      json.closeDict();
      ++profileIndex;
    }
    json.closeArray();

    json.closeDict();
  }
  json.closeArray(); // Functions.

  json.closeDict();
  OS.flush();
}

} // namespace vm
} // namespace hermes

#endif // HERMESVM_PROFILER_BB
