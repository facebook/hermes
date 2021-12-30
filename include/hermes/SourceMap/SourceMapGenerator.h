/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_SOURCEMAPGENERATOR_H
#define HERMES_SUPPORT_SOURCEMAPGENERATOR_H

#include "hermes/SourceMap/SourceMap.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/Support/StringSetVector.h"
#include "llvh/ADT/ArrayRef.h"

#include <llvh/ADT/DenseMap.h>
#include <vector>

namespace hermes {

/// A class representing a JavaScript source map, version 3 only. It borrows
/// terminology from the SourceMap spec: the "represented" code is the original,
/// while the "generated" code is the output of the minifier/compiler/etc.
/// See https://sourcemaps.info/spec.html for the spec that this class
/// implements.
class SourceMapGenerator {
 public:
  /// Add a line \p line represented as a list of Segments to the 'mappings'
  /// section.
  /// \param segmentID the ID of the segment ( = the BytecodeModule /
  /// RuntimeModule), used as the "line" when reporting stack traces from the
  /// VM.
  void addMappingsLine(SourceMap::SegmentList line, uint32_t segmentID) {
    if (lines_.size() <= segmentID) {
      lines_.resize(segmentID + 1);
    }
    lines_[segmentID] = std::move(line);
  }

  /// \return the list of mappings lines.
  llvh::ArrayRef<SourceMap::SegmentList> getMappingsLines() const {
    return lines_;
  }

  /// Set the list of input source maps to \p maps.
  /// The order should match the indexes used in the sourceIndex field of
  /// Segment.
  void setInputSourceMaps(std::vector<std::unique_ptr<SourceMap>> maps) {
    inputSourceMaps_ = std::move(maps);
  }

  /// Adds the source filename to filenameTable_ if it doesn't already exist.
  /// If \p metadata is provided and is non-null, it becomes the metadata entry
  /// associated with this source (even if the source existed previously).
  /// \return the index of \p filename in filenameTable_.
  uint32_t addSource(
      llvh::StringRef filename,
      llvh::Optional<SourceMap::MetadataEntry> metadata = llvh::None);

  /// Output the given source map as JSON.
  void outputAsJSON(llvh::raw_ostream &OS) const;

  /// Adds a list of function offsets indexed by function ID for a given
  /// bytecode segment. This list will be printed under
  /// x_hermes_function_offsets in the output of
  /// SourceMapGenerator::outputAsJSON.
  void addFunctionOffsets(
      std::vector<uint32_t> &&functionOffsets,
      uint32_t segmentID);

  /// Get the source index given the filename.
  uint32_t getSourceIndex(llvh::StringRef filename) const {
    auto it = filenameTable_.find(filename);
    assert(it != filenameTable_.end() && "unable to find filenameId");
    return std::distance(filenameTable_.begin(), it);
  }

 private:
  /// Delta encoding state.
  struct State {
    int32_t generatedColumn = 0;
    int32_t sourceIndex = 0;
    int32_t representedLine = 0;
    int32_t representedColumn = 0;
    int32_t nameIndex = 0;
  };

  /// Implementation of outputAsJSON for a merged source map generator.
  void outputAsJSONImpl(llvh::raw_ostream &OS) const;

  /// \return the mappings encoded in VLQ format.
  std::string getVLQMappingsString() const;

  /// \return a list of sources, in order.
  /// This list refers to internals of the StringMap and is invalidated by
  /// addSource().
  std::vector<llvh::StringRef> getSources() const;

  /// Encode the list \p segments into \p OS using the SourceMap
  /// Base64-VLQ scheme, delta-encoded with \p lastState as the starting state.
  static SourceMapGenerator::State encodeSourceLocations(
      const SourceMapGenerator::State &lastState,
      llvh::ArrayRef<SourceMap::Segment> segments,
      llvh::raw_ostream &OS);

  /// Merge the input source maps with the state in this generator,
  /// and return a new generator which contains a merged representation.
  SourceMapGenerator mergedWithInputSourceMaps() const;

  /// \return the input source map segment for \p seg if the input source map
  /// exists and has a valid location for \p seg. The input segment may be
  /// llvh::None, in which case the input source map may be nullptr.
  std::pair<llvh::Optional<SourceMap::Segment>, const SourceMap *>
  getInputSegmentForSegment(const SourceMap::Segment &seg) const;

  bool hasSourcesMetadata() const;

  /// \return metadata for source \index, if we have any.
  llvh::Optional<SourceMap::MetadataEntry> getSourceMetadata(
      uint32_t index) const {
    if (index >= sourcesMetadata_.size()) {
      return llvh::None;
    }
    return sourcesMetadata_[index];
  }

  /// The list of symbol names, populating the names field.
  std::vector<std::string> symbolNames_;

  /// The list of segments in our VLQ scheme.
  std::vector<SourceMap::SegmentList> lines_;

  /// The list of input source maps, such that the input file i has the
  /// SourceMap at index i. If no map was provided for a file, this list
  /// contains nullptr.
  std::vector<std::unique_ptr<SourceMap>> inputSourceMaps_;

  /// Map from {filename => source index}.
  StringSetVector filenameTable_{};

  /// Metadata for each source keyed by source index. Represents the
  /// x_facebook_sources field in the JSON source map.
  SourceMap::MetadataList sourcesMetadata_;

  ///  Maps segmentID to a vector of function offsets indexed by their
  /// function id.
  llvh::DenseMap<uint32_t, std::vector<uint32_t>> functionOffsets_{};
};

} // namespace hermes

#endif // HERMES_SUPPORT_SOURCEMAPGENERATOR_H
