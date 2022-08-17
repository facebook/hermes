/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_PROFILER_BB

#include "hermes/VM/BasicBlockExecutionInfo.h"
#include "hermes/Support/JSONEmitter.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/Runtime.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/Support/MD5.h"

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

static llvh::MD5::MD5Result doMD5Checksum(llvh::ArrayRef<uint8_t> bytecode) {
  llvh::MD5 md5;
  llvh::MD5::MD5Result checksum;
  md5.update(bytecode);
  md5.final(checksum);
  return checksum;
}

void BasicBlockExecutionInfo::dump(llvh::raw_ostream &OS) {
  JSONEmitter json(OS);
  json.openDict();
  json.emitKeyValue("version", BASIC_BLOCK_STAT_VERSION);
  json.emitKeyValue("page_size", (double)hermes::oscompat::page_size());

  json.emitKey("functions");
  json.openArray();

  for (const auto &funcEntry : basicBlockStats_) {
    json.openDict();
    auto md5Result = doMD5Checksum(funcEntry.first->getOpcodeArray());
    json.emitKeyValue("checksum", md5Result.digest().str());

    // hbcdump will be responsible to check overflow scenario(index-zero entry
    // is not empty).
    const auto &funcStat = funcEntry.second;
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
  json.closeArray();
  json.closeDict();
  OS.flush();
}

} // namespace vm
} // namespace hermes

#endif // HERMESVM_PROFILER_BB
