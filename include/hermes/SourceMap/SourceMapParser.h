/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_SUPPORT_SOURCEMAPPARSER_H
#define HERMES_SUPPORT_SOURCEMAPPARSER_H

#include "hermes/SourceMap/SourceMapGenerator.h"

namespace hermes {

/// JavaScript version 3 source map parser.
/// See https://sourcemaps.info/spec.html for the spec that this class
/// parses.
class SourceMapParser {
 public:
  /// Parse input \p sourceMap and return false on failure if malformed.
  bool parse(llvm::StringRef sourceMap);

  /// Query source map text location for \p line and \p column.
  llvm::Optional<SourceMapTextLocation> getLocationForAddress(
      uint32_t line,
      uint32_t column);

  /// \return a list of original sources used by “mappings” entry.
  /// For testing.
  std::vector<std::string> getAllFullPathSources() const {
    std::vector<std::string> sourceFullPath(sources_.size());
    for (uint32_t i = 0, e = sources_.size(); i < e; ++i) {
      sourceFullPath[i] = getSourceFullPath(i);
    }
    return sourceFullPath;
  }

 private:
  /// Parse "mappings" section.
  bool parseMappings(llvm::StringRef sourceMappings);

  /// Parse single segment in mapping.
  llvm::Optional<SourceMapGenerator::Segment> parseSegment(
      const SourceMapGenerator::Segment &prevSegment,
      const char *&pCur,
      const char *pSegEnd);

  /// \return source file path with root combined for source \p index.
  std::string getSourceFullPath(uint32_t index) const {
    assert(index < sources_.size() && "index out-of-range for sources_");
    // TODO: more sophisticated path concat handling.
    return sourceRoot_ + sources_[index];
  }

  /// An optional source root, useful for relocating source files on a server or
  /// removing repeated values in the “sources” entry.  This value is prepended
  /// to the individual entries in the “source” field.
  std::string sourceRoot_;

  /// The list of sources parsed from the "sources" section without sourceRoot_
  /// appended.
  std::vector<std::string> sources_;

  /// The list of segments in VLQ scheme.
  std::vector<SourceMapGenerator::SegmentList> lines_;
};

} // namespace hermes

#endif // HERMES_SUPPORT_SOURCEMAPPARSER_H
