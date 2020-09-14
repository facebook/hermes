/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PARSER_PREPARSER_H
#define HERMES_PARSER_PREPARSER_H

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/SmallVector.h"
#include "llvh/Support/SMLoc.h"

namespace hermes {
namespace parser {
using llvh::SMLoc;

/// Allow using \p SMLoc in \p llvh::DenseMaps.
struct SMLocInfo : llvh::DenseMapInfo<SMLoc> {
  static inline SMLoc getEmptyKey() {
    return SMLoc::getFromPointer(0);
  }
  static inline SMLoc getTombstoneKey() {
    return SMLoc::getFromPointer((const char *)1);
  }
  static inline bool isEqual(const SMLoc &a, const SMLoc &b) {
    return a == b;
  }
  static unsigned getHashValue(const SMLoc &Val) {
    return (unsigned)(uintptr_t)Val.getPointer();
  }
};

/// Per buffer information from preparsing.
struct PreParsedBufferInfo {
  /// Map from function body start to end.
  llvh::DenseMap<SMLoc, SMLoc, SMLocInfo> bodyStartToEnd{};
};

/// Per \p Context information from preparsing.
struct PreParsedData {
  /// Vector from source buffer id to that buffer's \p PreParsedBufferInfo.
  llvh::SmallVector<std::unique_ptr<PreParsedBufferInfo>, 4> bufferInfo{};

  /// Get the \p PreParsedBufferInfo for a source buffer, allocating it if
  /// necessary.
  PreParsedBufferInfo *getBufferInfo(uint32_t bufferId) {
    if (bufferInfo.size() < bufferId + 1) {
      bufferInfo.resize(bufferId + 1);
    }
    if (!bufferInfo[bufferId]) {
      bufferInfo[bufferId] = llvh::make_unique<PreParsedBufferInfo>();
    }
    return bufferInfo[bufferId].get();
  }
};

} // namespace parser
} // namespace hermes
#endif
