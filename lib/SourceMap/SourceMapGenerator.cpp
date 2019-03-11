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

void SourceMapGenerator::outputAsJSON(llvm::raw_ostream &OS) const {
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
