/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_TOOLS_HBCDUMP_HBCPARSER_H
#define HERMES_TOOLS_HBCDUMP_HBCPARSER_H

#include "hermes/BCGen/HBC/BytecodeDisassembler.h"
#include "llvh/ADT/StringRef.h"

#include <unordered_map>
#include <unordered_set>

namespace hermes {

// Maps bytecode instruction ip => profile_index.
using ProfileIndexMap = std::unordered_map<const uint8_t *, uint16_t>;
// Maps profile_index => static_instruction_count.
using BasicBlockStaticInstCountMap = std::unordered_map<uint16_t, uint64_t>;

/// Parser for hermes bytecode file.
class HBCParser {
 private:
  std::shared_ptr<hbc::BCProvider> bcProvider_;
  // func_id => checksum.
  std::unordered_map<uint32_t, std::string> funcChecksumMap_;
  // func_id => ProfileIndexMap.
  // This map is lazily built for each function on demand.
  std::unordered_map<uint32_t, ProfileIndexMap> funcProfileIndexMap_;
  // func_id => BasicBlockStaticInstCountMap.
  // This map is lazily built for each function on demand.
  std::unordered_map<uint32_t, BasicBlockStaticInstCountMap>
      funcBasicBlockStaticInstCountMap_;

 private:
  /// Build <ip => profile_index> map for \p funcId.
  /// Each basic block should have a ProfilePoint index.
  /// Since ProfilePoint instruction is inserted by hermes compiler which has
  /// different basic block view from hbc parser(which uses disassembly), to
  /// build this map, we have to combine both hbc basic block view and
  /// ProfilePoint instruction together: we first build basic block ranges from
  /// bytecode disassembly(hbc parser), then we use any ProfilePoint
  /// instruction in the middle of a basic block to divide basic blocks even
  /// further.
  ProfileIndexMap buildProfileIndexMap(uint32_t funcId);

  /// Generate <func_id => checksum> map.
  std::unordered_map<uint32_t, std::string> generateFunctionChecksumMap();

 public:
  HBCParser(std::shared_ptr<hbc::BCProvider> bcProvider)
      : bcProvider_(std::move(bcProvider)),
        funcChecksumMap_(generateFunctionChecksumMap()) {}

  std::shared_ptr<hbc::BCProvider> getBCProvider() {
    return bcProvider_;
  }

  /// Get the checksum of function with \p funcId.
  llvh::StringRef getFunctionChecksum(uint32_t funcId) {
    return funcChecksumMap_[funcId];
  }

  /// For \p profileIndex block in function \p funcId, return its bytecode
  /// offset from start of the function.
  uint32_t getBasicBlockOffset(uint32_t funcId, uint16_t profileIndex);

  /// Get debug source location information at \p opcodeOffset in \p funcId.
  llvh::Optional<SourceMapTextLocation> getSourceLocation(
      uint32_t funcId,
      uint32_t offsetInFunction) {
    return bcProvider_->getLocationForAddress(funcId, offsetInFunction);
  }

  /// Get function id from its \p checksum.
  llvh::Optional<uint32_t> functionIdFromChecksum(llvh::StringRef checksum) {
    for (const auto &entry : funcChecksumMap_) {
      if (entry.second == checksum) {
        return entry.first;
      }
    }
    return llvh::None;
  }

  /// Get BasicBlockStaticInstCountMap for \p funcId.
  BasicBlockStaticInstCountMap &getBasicBlockStaticInstCountMap(
      uint32_t funcId);

  /// Get the <ip => profile_index> map for function with \p funcId.
  ProfileIndexMap &getProfileIndexMap(uint32_t funcId) {
    if (funcProfileIndexMap_.find(funcId) == funcProfileIndexMap_.end()) {
      funcProfileIndexMap_[funcId] = buildProfileIndexMap(funcId);
    }
    return funcProfileIndexMap_[funcId];
  }
};

} // namespace hermes

#endif // HERMES_TOOLS_HBCDUMP_HBCPARSER_H
