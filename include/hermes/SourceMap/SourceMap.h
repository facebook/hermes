/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_SUPPORT_SOURCEMAP_H
#define HERMES_SUPPORT_SOURCEMAP_H

#include "hermes/Support/OptValue.h"

#include "llvm/Support/raw_ostream.h"

#include <vector>

namespace hermes {

/// Represent a source location in original JS source file.
/// It is different from DebugSourceLocation that it may hold string
/// names not existing in string table.
struct SourceMapTextLocation {
  std::string fileName;
  uint32_t line;
  uint32_t column;
};

/// The vlq namespace contains support for SourceMap-style Base-64
/// variable-length quantities.
namespace base64vlq {

/// Encode \p value into \p OS as a Base64 variable-length quantity.
/// \return OS.
llvm::raw_ostream &encode(llvm::raw_ostream &OS, int32_t value);

/// Decode a Base64 variable-length quantity from the range starting at \p begin
/// and ending at \p end (whose length is end - begin).
/// \return the decoded value, or None if a value could not be decoded.
/// If a value could be decoded, \p begin is updated to point after the end of
/// the string.
OptValue<int32_t> decode(const char *&begin, const char *end);

} // namespace base64vlq

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

    /// Zero-based index into the "sources" list.
    int32_t sourceIndex = 0;

    /// Zero based starting line in the original source.
    int32_t representedLine = 0;

    /// Zero based starting column in the original source.
    int32_t representedColumn = 0;

    /// Zero based symbol name index into "names" list.
    int32_t representedNameIndex = 0;

    /// Construct a segment maps a generated column \p genCol to the given
    /// represented line \p repLine and column \p repCol.
    Segment(
        int32_t genCol,
        int32_t sourceIndex,
        int32_t repLine,
        int32_t repCol,
        int32_t repNameIndex)
        : generatedColumn(genCol),
          sourceIndex(sourceIndex),
          representedLine(repLine),
          representedColumn(repCol),
          representedNameIndex(repNameIndex) {}

    /// Default constructor.
    Segment() {}
  };
  using SegmentList = std::vector<Segment>;

  SourceMap(
      const std::string &sourceRoot,
      std::vector<std::string> &&sources,
      std::vector<SegmentList> &&lines)
      : sourceRoot_(sourceRoot),
        sources_(std::move(sources)),
        lines_(std::move(lines)) {}

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
  /// \return source file path with root combined for source \p index.
  std::string getSourceFullPath(uint32_t index) const {
    assert(index < sources_.size() && "index out-of-range for sources_");
    // TODO: more sophisticated path concat handling.
    return sourceRoot_ + sources_[index];
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
};

} // namespace hermes

#endif // HERMES_SUPPORT_SOURCEMAP_H
