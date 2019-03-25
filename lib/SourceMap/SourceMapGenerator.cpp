/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/SourceMap/SourceMapGenerator.h"

#include "hermes/Support/JSONEmitter.h"

namespace hermes {

namespace {

/// A VLQ (VariableLengthQuantity) is a type of encoding used in source maps.
/// Values are represented in a pseudo-base64 using continuation bits.
/// The value is signed because SourceMaps are delta-encoded, so relative
/// offsets (for example, between columns) may be negative. We wrap an int32_t
/// in a struct so we can overload operator<< to output in the VLQ format.
struct VLQ {
  int32_t val;
};

llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, VLQ vlq) {
  return base64vlq::encode(OS, vlq.val);
}
} // namespace

SourceMap::Segment SourceMapGenerator::encodeSourceLocations(
    const SourceMap::Segment &lastSegment,
    llvm::ArrayRef<SourceMap::Segment> segments,
    llvm::raw_ostream &OS) {
  // Currently we only support a single source, so the source ID (and its delta)
  // is always 0.
  SourceMap::Segment prev = lastSegment;
  bool first = true;
  for (const SourceMap::Segment &seg : segments) {
    // Segments are separated by commas.
    OS << (first ? "" : ",") << VLQ{seg.generatedColumn - prev.generatedColumn}
       << VLQ{seg.sourceIndex - prev.sourceIndex}
       << VLQ{seg.representedLine - prev.representedLine}
       << VLQ{seg.representedColumn - prev.representedColumn};
    first = false;
    prev = seg;
  }
  return prev;
}

std::string SourceMapGenerator::getVLQMappingsString() const {
  std::string result;
  llvm::raw_string_ostream OS(result);
  SourceMap::Segment lastSegment;
  // Lines are 0 based: the first line is 0 and not 1, therefore initialize it
  // to 1 so we encode it with a delta of 0.
  lastSegment.representedLine = 1;
  for (const SourceMap::SegmentList &segments : lines_) {
    // The generated column (unlike other fields) resets with each new line.
    lastSegment.generatedColumn = 0;
    lastSegment = encodeSourceLocations(lastSegment, segments, OS);
    OS << ';';
  }
  OS.flush();
  return result;
}

SourceMapGenerator SourceMapGenerator::mergedWithInputSourceMaps() const {
  assert(
      !inputSourceMaps_.empty() &&
      "Cannot merge source maps without input source maps");

  // Create a new SourceMapGenerator with the merged data.
  SourceMapGenerator merged;

  for (uint32_t i = 0, e = lines_.size(); i < e; ++i) {
    SourceMap::SegmentList newLine{};

    for (const auto &seg : lines_[i]) {
      assert(seg.sourceIndex >= 0 && "Negative source index");
      SourceMap::Segment newSeg = seg;

      if (auto loc = getInputLocationForSegment(seg)) {
        // We have an input source map and were able to find a merged source
        // location.
        newSeg.representedLine = loc->line;
        newSeg.representedColumn = loc->column;
        newSeg.sourceIndex = merged.addSource(loc->fileName);
      } else {
        // Failed to find a merge location. Use the existing location,
        // but copy over the source file name.
        newSeg.sourceIndex = merged.addSource(sources_[seg.sourceIndex]);
      }

      newLine.push_back(std::move(newSeg));
    }

    merged.addMappingsLine(std::move(newLine), i);
  }

  return merged;
}

void SourceMapGenerator::outputAsJSON(llvm::raw_ostream &OS) const {
  if (inputSourceMaps_.empty()) {
    this->outputAsJSONImpl(OS);
  } else {
    // If there are input source maps, we must merge them into a new
    // SourceMapGenerator before output.
    mergedWithInputSourceMaps().outputAsJSONImpl(OS);
  }
}

void SourceMapGenerator::outputAsJSONImpl(llvm::raw_ostream &OS) const {
  JSONEmitter json(OS);
  json.openDict();
  json.emitKeyValue("version", 3);

  json.emitKey("sources");
  json.openArray();
  json.emitValues(llvm::makeArrayRef(sources_));
  json.closeArray();

  json.emitKeyValue("mappings", getVLQMappingsString());
  json.closeDict();
  OS.flush();
}

} // namespace hermes
