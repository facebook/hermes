/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_SUPPORT_SOURCEMAP_H
#define HERMES_SUPPORT_SOURCEMAP_H

#include "hermes/Support/OptValue.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
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

/// A class representing a JavaScript source map, version 3 only. It borrows
/// terminology from the SourceMap spec: the "represented" code is the original,
/// while the "generated" code is the output of the minifier/compiler/etc.
/// See https://sourcemaps.info/spec.html for the spec that this class
/// implements.
class SourceMapGenerator {
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

  /// Add a line \p line represented as a list of Segments to the 'mappings'
  /// section.
  void addMappingsLine(SegmentList line) {
    lines_.push_back(std::move(line));
  }

  /// \return the list of mappings lines.
  llvm::ArrayRef<SegmentList> getMappingsLines() const {
    return lines_;
  }

  /// Set the list of sources to \p sources. The sources are the list of
  /// original input filenames. The order should match the indexes used in the
  /// sourceIndex field of Segment.
  void setSources(std::vector<std::string> sources) {
    sources_ = std::move(sources);
  }

  /// Output the given source map as JSON.
  void outputAsJSON(llvm::raw_ostream &OS) const;

  /// Add filename ID to the map to indicate what the source index is.
  /// If the filenameId is already mapped (often the case), do nothing.
  void addFilenameMapping(uint32_t filenameId, uint32_t sourceIndex) {
    filenameTable.try_emplace(filenameId, sourceIndex);
  }

  /// Get the source index given the filename ID.
  uint32_t getSourceIndex(uint32_t filenameId) const {
    auto it = filenameTable.find(filenameId);
    assert(it != filenameTable.end() && "unable to find filenameId");
    return it->second;
  }

 private:
  /// \return the mappings encoded in VLQ format.
  std::string getVLQMappingsString() const;

  /// Encode the list \p segments into \p OS using the SourceMap
  /// Base64-VLQ scheme, delta-encoded starting with \p lastSegment.
  static Segment encodeSourceLocations(
      const Segment &lastSegment,
      llvm::ArrayRef<Segment> segments,
      llvm::raw_ostream &OS);

  /// The list of sources, populating the sources field.
  std::vector<std::string> sources_;

  /// The list of symbol names, populating the names field.
  std::vector<std::string> symbolNames_;

  /// The list of segments in our VLQ scheme.
  std::vector<SegmentList> lines_;

  /// Map from {filenameID => source index}.
  /// Used to translate debug source locations involving string table indices
  /// to indices into sources_.
  llvm::DenseMap<uint32_t, uint32_t> filenameTable{};
};

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

#endif // HERMES_SUPPORT_SOURCEMAP_H
