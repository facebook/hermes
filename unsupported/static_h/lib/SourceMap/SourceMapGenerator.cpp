/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/SourceMap/SourceMapGenerator.h"

#include "hermes/Support/Base64vlq.h"
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

llvh::raw_ostream &operator<<(llvh::raw_ostream &OS, VLQ vlq) {
  return base64vlq::encode(OS, vlq.val);
}
} // namespace

uint32_t SourceMapGenerator::addSource(
    llvh::StringRef filename,
    llvh::Optional<SourceMap::MetadataEntry> metadata) {
  uint32_t index = filenameTable_.insert(filename);
  if (sourcesMetadata_.size() <= index) {
    sourcesMetadata_.resize(index + 1);
  }
  if (metadata.hasValue() &&
      metadata.getValue()->getKind() != parser::JSONKind::Null) {
    sourcesMetadata_[index] = metadata.getValue();
  }
  return index;
}

std::pair<llvh::Optional<SourceMap::Segment>, const SourceMap *>
SourceMapGenerator::getInputSegmentForSegment(
    const SourceMap::Segment &seg) const {
  if (seg.representedLocation.hasValue()) {
    assert(
        seg.representedLocation->sourceIndex >= 0 && "Negative source index");
  }

  const SourceMap *inputMap = nullptr;
  llvh::Optional<SourceMap::Segment> inputSeg;

  if (seg.representedLocation.hasValue() &&
      (uint32_t)seg.representedLocation->sourceIndex <
          inputSourceMaps_.size()) {
    inputMap = inputSourceMaps_[seg.representedLocation->sourceIndex].get();
    if (inputMap) {
      inputSeg = inputMap->getSegmentForAddress(
          seg.representedLocation->lineIndex + 1,
          seg.representedLocation->columnIndex + 1);
    }
  }

  return std::make_pair(inputSeg, inputMap);
}

bool SourceMapGenerator::hasSourcesMetadata() const {
  for (const auto &entry : sourcesMetadata_) {
    if (entry.hasValue() &&
        entry.getValue()->getKind() != parser::JSONKind::Null) {
      return true;
    }
  }
  return false;
}

SourceMapGenerator::State SourceMapGenerator::encodeSourceLocations(
    const SourceMapGenerator::State &lastState,
    llvh::ArrayRef<SourceMap::Segment> segments,
    llvh::raw_ostream &OS) {
  // Currently we only support a single source, so the source ID (and its delta)
  // is always 0.
  SourceMapGenerator::State state = lastState, prevState = lastState;
  bool first = true;
  for (const SourceMap::Segment &seg : segments) {
    // Segments are separated by commas.
    state.generatedColumn = seg.generatedColumn;
    OS << (first ? "" : ",")
       << VLQ{state.generatedColumn - prevState.generatedColumn};
    if (seg.representedLocation.hasValue()) {
      state.sourceIndex = seg.representedLocation->sourceIndex;
      state.representedLine = seg.representedLocation->lineIndex;
      state.representedColumn = seg.representedLocation->columnIndex;
      OS << VLQ{state.sourceIndex - prevState.sourceIndex}
         << VLQ{state.representedLine - prevState.representedLine}
         << VLQ{state.representedColumn - prevState.representedColumn};

      if (seg.representedLocation->nameIndex.hasValue()) {
        state.nameIndex = seg.representedLocation->nameIndex.getValue();
        OS << VLQ{state.nameIndex - prevState.nameIndex};
      }
    }
    first = false;
    prevState = state;
  }
  return prevState;
}

std::string SourceMapGenerator::getVLQMappingsString() const {
  std::string result;
  llvh::raw_string_ostream OS(result);
  State state;
  for (const SourceMap::SegmentList &segments : lines_) {
    // The generated column (unlike other fields) resets with each new line.
    state.generatedColumn = 0;
    state = encodeSourceLocations(state, segments, OS);
    OS << ';';
  }
  OS.flush();
  return result;
}

std::vector<llvh::StringRef> SourceMapGenerator::getSources() const {
  return std::vector<llvh::StringRef>(
      filenameTable_.begin(), filenameTable_.end());
}

SourceMapGenerator SourceMapGenerator::mergedWithInputSourceMaps() const {
  assert(
      !inputSourceMaps_.empty() &&
      "Cannot merge source maps without input source maps");

  auto sources = getSources();

  // Create a new SourceMapGenerator with the merged data.
  SourceMapGenerator merged;

  for (uint32_t i = 0, e = lines_.size(); i < e; ++i) {
    SourceMap::SegmentList newLine{};

    for (const auto &seg : lines_[i]) {
      auto pair = getInputSegmentForSegment(seg);
      auto inputSeg = pair.first;
      auto inputMap = pair.second;

      SourceMap::Segment newSeg = seg;
      newSeg.representedLocation = llvh::None;

      if (inputSeg.hasValue() && inputSeg->representedLocation.hasValue()) {
        // We have an input source map and were able to find a merged source
        // location.

        auto loc = inputSeg->representedLocation.getValue();
        // Our _output_ sourceRoot is empty, so make sure to canonicalize
        // the path based on the input map's sourceRoot.
        std::string filename = inputMap->getSourceFullPath(loc.sourceIndex);

        newSeg.representedLocation = SourceMap::Segment::SourceLocation(
            merged.addSource(
                filename, inputMap->getSourceMetadata(loc.sourceIndex)),
            loc.lineIndex,
            loc.columnIndex
            // TODO: Handle name index
        );
      }

      if (!inputMap && !newSeg.representedLocation.hasValue() &&
          seg.representedLocation.hasValue()) {
        // Failed to find a merged location because there is no input source
        // map. Use the existing location, but copy over the source file name.
        newSeg.representedLocation = seg.representedLocation;
        newSeg.representedLocation->sourceIndex = merged.addSource(
            sources[seg.representedLocation->sourceIndex],
            getSourceMetadata(seg.representedLocation->sourceIndex));
      }

      // Push the new segment even if it has no represented location. If there
      // is an input source map, all locations we emit will be in terms of it,
      // or be explicitly unmapped.
      newLine.push_back(std::move(newSeg));
    }

    merged.addMappingsLine(std::move(newLine), i);
  }
  merged.functionOffsets_ = functionOffsets_;
  return merged;
}

void SourceMapGenerator::outputAsJSON(llvh::raw_ostream &OS) const {
  if (inputSourceMaps_.empty()) {
    this->outputAsJSONImpl(OS);
  } else {
    // If there are input source maps, we must merge them into a new
    // SourceMapGenerator before output.
    mergedWithInputSourceMaps().outputAsJSONImpl(OS);
  }
}

void SourceMapGenerator::outputAsJSONImpl(llvh::raw_ostream &OS) const {
  JSONEmitter json(OS);
  json.openDict();
  json.emitKeyValue("version", 3);

  json.emitKey("sources");
  json.openArray();
  json.emitValues(llvh::makeArrayRef(getSources()));
  json.closeArray();

  if (hasSourcesMetadata()) {
    json.emitKey("x_facebook_sources");
    json.openArray();
    for (const auto &source : sourcesMetadata_) {
      if (source.hasValue()) {
        source.getValue()->emitInto(json);
      } else {
        json.emitNullValue();
      }
    }
    json.closeArray();
  }

  json.emitKeyValue("mappings", getVLQMappingsString());

  if (!functionOffsets_.empty()) {
    json.emitKey("x_hermes_function_offsets");
    json.openDict();
    for (const auto &entry : functionOffsets_) {
      const auto &segmentFunctionOffsets = entry.second;
      json.emitKey(std::to_string(entry.first));
      json.openArray();
      json.emitValues((llvh::ArrayRef<uint32_t>)segmentFunctionOffsets);
      json.closeArray();
    }
    json.closeDict();
  }
  json.closeDict();
  OS.flush();
}

void SourceMapGenerator::addFunctionOffsets(
    std::vector<uint32_t> &&functionOffsets,
    uint32_t segmentID) {
  assert(functionOffsets.size() > 0 && "functionOffsets can not be empty");
  functionOffsets_[segmentID] = std::move(functionOffsets);
}

} // namespace hermes
