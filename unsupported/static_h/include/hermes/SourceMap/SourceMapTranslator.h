/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SOURCEMAP_SOURCEMAPTRANSLATOR_H
#define HERMES_SOURCEMAP_SOURCEMAPTRANSLATOR_H

#include "hermes/SourceMap/SourceMap.h"
#include "hermes/Support/SourceErrorManager.h"

#include "llvh/ADT/DenseMap.h"

namespace hermes {

/// SourceErrorManager coordinate translator that translates compiling source
/// code's line:column to its original location based on input source map.
class SourceMapTranslator final : public SourceErrorManager::ICoordTranslator {
  /// Parsed input source maps for each input JS file.
  /// js file's buffer id => SourceMap.
  llvh::DenseMap<unsigned, std::shared_ptr<SourceMap>> sourceMaps_;

  /// For lazily adding new virtual buffer for translated original source file.
  SourceErrorManager &sourceErrorManager_;

 public:
  SourceMapTranslator(SourceErrorManager &sourceErrorManager)
      : sourceErrorManager_(sourceErrorManager) {}

  ~SourceMapTranslator() override = default;

  /// Add new fileBufId => sourceMap mapping.
  /// \p fileBufId is the compiling js file corresponding to \p sourceMap.
  void addSourceMap(unsigned fileBufId, std::unique_ptr<SourceMap> sourceMap);

  /// Translate \p coords to its original location in-place.
  void translate(SourceErrorManager::SourceCoords &coords) override;
};

} // namespace hermes

#endif
