/*
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
  /// Parse input \p sourceMapContent and return parsed SourceMap.
  /// Return nullptr on failure if malformed.
  static std::unique_ptr<SourceMap> parse(llvm::StringRef sourceMapContent);

 private:
  /// Delta encoding state.
  struct State {
    int32_t generatedColumn = 0;
    int32_t sourceIndex = 0;
    int32_t representedLine = 0;
    int32_t representedColumn = 0;
    int32_t nameIndex = 0;
  };

  /// Parse "mappings" section from \p sourceMappings. The parsed line mappings
  /// are returned in \p lines.
  static bool parseMappings(
      llvm::StringRef sourceMappings,
      std::vector<SourceMap::SegmentList> &lines);

  /// Parse single segment in mapping.
  static llvm::Optional<SourceMap::Segment>
  parseSegment(const State &state, const char *&pCur, const char *pSegEnd);
};

} // namespace hermes

#endif // HERMES_SUPPORT_SOURCEMAPPARSER_H
