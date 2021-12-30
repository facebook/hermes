/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_SOURCEMAP_H
#define HERMES_SUPPORT_SOURCEMAP_H

#include "hermes/Parser/JSONParser.h"
#include "hermes/Support/OptValue.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/Support/raw_ostream.h"

#include <vector>

namespace hermes {

/// Represent a source location in original JS source file.
/// It is different from DebugSourceLocation that it may hold string
/// names not existing in string table.
struct SourceMapTextLocation {
  std::string fileName;
  // 1-based
  uint32_t line;
  // 1-based
  uint32_t column;
};

/// Represent a source location in original JS source file.
/// The file name is encoded as an index in the table of paths in the
/// associated source map.
struct SourceMapTextLocationFIndex {
  uint32_t fileIndex;
  // 1-based
  uint32_t line;
  // 1-based
  uint32_t column;
};

/// In-memory representation of JavaScript version 3 source map.
/// See https://sourcemaps.info/spec.html for the spec.
class SourceMap {
 public:
  /// SourceMaps contain a list of lines which are in turn separated into
  /// segments. A segment maps some column of the generated source to some
  /// line/column of the represented source.
  /// Note that the generated line is not stored in the segment, but is instead
  /// implicit in the "mappings" field of the source map: generated lines are
  /// not encoded directly but are simply separated with semicolons in the
  /// encoding.
  struct Segment {
    /// Zero-based starting column of the generated code.
    int32_t generatedColumn = 0;

    struct SourceLocation {
      /// Zero-based index into the "sources" list.
      int32_t sourceIndex = 0;

      /// Zero based starting line in the original source.
      int32_t lineIndex = 0;

      /// Zero based starting column in the original source.
      int32_t columnIndex = 0;

      /// Zero based symbol name index into "names" list.
      llvh::Optional<int32_t> nameIndex = llvh::None;

      SourceLocation(
          int32_t sourceIndex,
          int32_t lineIndex,
          int32_t columnIndex,
          llvh::Optional<int32_t> nameIndex = llvh::None)
          : sourceIndex(sourceIndex),
            lineIndex(lineIndex),
            columnIndex(columnIndex),
            nameIndex(nameIndex) {}

      /// Default constructor
      SourceLocation() = default;
    };

    llvh::Optional<SourceLocation> representedLocation;

    /// Construct a segment that maps a generated column \p genCol to the given
    /// represented line \p repLine and column \p repCol.
    Segment(
        int32_t genCol,
        int32_t sourceIndex,
        int32_t repLine,
        int32_t repCol,
        llvh::Optional<int32_t> repNameIndex = llvh::None)
        : generatedColumn(genCol),
          representedLocation(
              SourceLocation{sourceIndex, repLine, repCol, repNameIndex}) {}

    /// Default constructor.
    Segment() = default;
  };
  using SegmentList = std::vector<Segment>;
  using MetadataEntry = parser::JSONSharedValue;
  using MetadataList = std::vector<llvh::Optional<MetadataEntry>>;

  SourceMap(
      const std::string &sourceRoot,
      std::vector<std::string> &&sources,
      std::vector<SegmentList> &&lines,
      MetadataList &&sourcesMetadata)
      : sourceRoot_(sourceRoot),
        sources_(std::move(sources)),
        lines_(std::move(lines)),
        sourcesMetadata_(std::move(sourcesMetadata)) {}

  /// Query source map text location for \p line and \p column.
  /// In both the input and output of this function, line and column numbers
  /// are 1-based.
  llvh::Optional<SourceMapTextLocation> getLocationForAddress(
      uint32_t line,
      uint32_t column) const;

  /// Query source map text location for \p line and \p column.
  /// In both the input and output of this function, line and column numbers
  /// are 1-based.
  llvh::Optional<SourceMapTextLocationFIndex> getLocationForAddressFIndex(
      uint32_t line,
      uint32_t column) const;

  /// Query source map segment for \p line and \p column.
  /// The line and column arguments are 1-based (but note that the return value
  /// has 0-based line and column indices).
  llvh::Optional<SourceMap::Segment> getSegmentForAddress(
      uint32_t line,
      uint32_t column) const;

  /// \return a list of original sources used by “mappings” entry.
  /// For testing.
  std::vector<std::string> getAllFullPathSources() const {
    std::vector<std::string> sourceFullPath(sources_.size());
    for (uint32_t i = 0, e = sources_.size(); i < e; ++i) {
      sourceFullPath[i] = getSourceFullPath(i);
    }
    return sourceFullPath;
  }

  /// \return the number of source paths.
  uint32_t getNumSourcePaths() const {
    assert(sources_.size() <= UINT32_MAX);
    return (uint32_t)sources_.size();
  }

  /// \return source file path with root combined for source \p index.
  std::string getSourceFullPath(uint32_t index) const {
    assert(index < sources_.size() && "index out-of-range for sources_");
    // TODO: more sophisticated path concat handling.
    return sourceRoot_ + sources_[index];
  }

  /// \return metadata for source \p index, if we have any.
  llvh::Optional<MetadataEntry> getSourceMetadata(uint32_t index) const {
    if (index >= sourcesMetadata_.size()) {
      return llvh::None;
    }
    return sourcesMetadata_[index];
  }

 private:
  /// An optional source root, useful for relocating source files on a server or
  /// removing repeated values in the “sources” entry.  This value is prepended
  /// to the individual entries in the “source” field.
  std::string sourceRoot_;

  /// The list of sources parsed from the "sources" section without sourceRoot_
  /// appended.
  std::vector<std::string> sources_;

  /// The list of segments in VLQ scheme.
  std::vector<SegmentList> lines_;

  /// Metadata for each source keyed by source index. Represents the
  /// x_facebook_sources field in the JSON source map.
  MetadataList sourcesMetadata_;
};

} // namespace hermes

#endif // HERMES_SUPPORT_SOURCEMAP_H
