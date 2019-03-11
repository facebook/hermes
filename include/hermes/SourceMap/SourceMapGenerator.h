/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_SUPPORT_SOURCEMAPGENERATOR_H
#define HERMES_SUPPORT_SOURCEMAPGENERATOR_H

#include "hermes/SourceMap/SourceMap.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"

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
  void addMappingsLine(SourceMap::SegmentList line) {
    lines_.push_back(std::move(line));
  }

  /// \return the list of mappings lines.
  llvm::ArrayRef<SourceMap::SegmentList> getMappingsLines() const {
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
  static SourceMap::Segment encodeSourceLocations(
      const SourceMap::Segment &lastSegment,
      llvm::ArrayRef<SourceMap::Segment> segments,
      llvm::raw_ostream &OS);

  /// The list of sources, populating the sources field.
  std::vector<std::string> sources_;

  /// The list of symbol names, populating the names field.
  std::vector<std::string> symbolNames_;

  /// The list of segments in our VLQ scheme.
  std::vector<SourceMap::SegmentList> lines_;

  /// Map from {filenameID => source index}.
  /// Used to translate debug source locations involving string table indices
  /// to indices into sources_.
  llvm::DenseMap<uint32_t, uint32_t> filenameTable{};
};

} // namespace hermes

#endif // HERMES_SUPPORT_SOURCEMAPGENERATOR_H
