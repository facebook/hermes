/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/SourceMap/SourceMapTranslator.h"

namespace hermes {

void SourceMapTranslator::addSourceMap(
    unsigned fileBufId,
    std::unique_ptr<SourceMap> sourceMap) {
  assert(sourceMap != nullptr);
  assert(
      sourceMaps_.find(fileBufId) == sourceMaps_.end() &&
      "fileBufId has been added");
  sourceMaps_[fileBufId] = std::move(sourceMap);
}

void SourceMapTranslator::translate(SourceErrorManager::SourceCoords &coords) {
  auto result = sourceMaps_.find(coords.bufId);
  if (result == sourceMaps_.end()) {
    // No input source map for coords.bufId.
    return;
  }
  const std::shared_ptr<SourceMap> &sourceMap = result->second;

  llvh::Optional<SourceMapTextLocation> originalLocOpt =
      sourceMap->getLocationForAddress(coords.line, coords.col);
  if (originalLocOpt.hasValue()) {
    coords.bufId = sourceErrorManager_.addNewVirtualSourceBuffer(
        originalLocOpt.getValue().fileName);
    coords.line = originalLocOpt.getValue().line;
    coords.col = originalLocOpt.getValue().column;
  }
}

} // namespace hermes
