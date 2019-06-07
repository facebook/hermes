/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
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

llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, VLQ vlq) {
  return base64vlq::encode(OS, vlq.val);
}
} // namespace

SourceMapGenerator::State SourceMapGenerator::encodeSourceLocations(
    const SourceMapGenerator::State &lastState,
    llvm::ArrayRef<SourceMap::Segment> segments,
    llvm::raw_ostream &OS) {
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
  llvm::raw_string_ostream OS(result);
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

std::vector<llvm::StringRef> SourceMapGenerator::getSources() const {
  return std::vector<llvm::StringRef>(
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
      SourceMap::Segment newSeg = seg;

      if (auto loc = getInputLocationForSegment(seg)) {
        // We have an input source map and were able to find a merged source
        // location.
        assert(loc->line >= 1 && "line numbers in debug info must be 1-based");
        assert(
            loc->column >= 1 && "column numbers in debug info must be 1-based");
        newSeg.representedLocation = SourceMap::Segment::SourceLocation(
            merged.addSource(loc->fileName), loc->line - 1, loc->column - 1
            // TODO: Handle name index
        );
      } else {
        // Failed to find a merge location. Use the existing location,
        // but copy over the source file name.
        if (seg.representedLocation.hasValue()) {
          newSeg.representedLocation->sourceIndex =
              merged.addSource(sources[seg.representedLocation->sourceIndex]);
        }
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
  json.emitValues(llvm::makeArrayRef(getSources()));
  json.closeArray();

  json.emitKeyValue("mappings", getVLQMappingsString());
  json.closeDict();
  OS.flush();
}

} // namespace hermes
