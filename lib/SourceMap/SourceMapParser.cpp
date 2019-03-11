/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/SourceMap/SourceMapParser.h"

#include "hermes/Parser/JSONParser.h"

#include <algorithm>

using namespace hermes;
using namespace hermes::parser;

namespace hermes {

bool SourceMapParser::parse(llvm::StringRef sourceMap) {
  parser::JSLexer::Allocator alloc;
  parser::JSONFactory factory(alloc);
  SourceErrorManager sm;
  parser::JSONParser jsonParser(factory, sourceMap, sm);

  // Parse for JavaScript version 3 source map https://sourcemaps.info/spec.html
  // Not yet implemented:
  //  1. 'file' field
  //  2. 'names' field
  //  3. 'sourcesContent' field.
  //  4. Index map.
  //  5. Facebook segments extension.
  auto *json =
      llvm::dyn_cast_or_null<JSONObject>(jsonParser.parse().getValue());
  if (json == nullptr) {
    return false;
  }

  auto *version = llvm::dyn_cast_or_null<JSONNumber>(json->get("version"));
  if (version == nullptr) {
    return false;
  }
  if ((uint64_t)version->getValue() != 3) {
    return false;
  }

  // sourceRoot is optional.
  auto *sourceRoot =
      llvm::dyn_cast_or_null<JSONString>(json->get("sourceRoot"));
  if (sourceRoot != nullptr) {
    sourceRoot_ = sourceRoot->str();
  }

  auto *sources = llvm::dyn_cast_or_null<JSONArray>(json->get("sources"));
  if (sources == nullptr) {
    return false;
  }

  sources_.resize(sources->size());
  for (unsigned i = 0, e = sources_.size(); i < e; ++i) {
    auto *file = llvm::dyn_cast_or_null<JSONString>(sources->at(i));
    if (file == nullptr) {
      return false;
    }
    sources_[i] = file->str();
  }

  auto *mappings = llvm::dyn_cast_or_null<JSONString>(json->get("mappings"));
  if (mappings == nullptr) {
    return false;
  }
  return parseMappings(mappings->str());
}

bool SourceMapParser::parseMappings(llvm::StringRef sourceMappings) {
  SourceMapGenerator::SegmentList segments;
  // Represent last line's segment value.
  SourceMapGenerator::Segment lastLineSegment;
  // Lines are encoded zero-based in source map while query
  // via 1-based so converting representedLine to be 1-based.
  lastLineSegment.representedLine = 1;
  SourceMapGenerator::Segment prevSegment = lastLineSegment;

  uint32_t curSegOffset = 0;
  while (curSegOffset < sourceMappings.size()) {
    // Source map mappings may omit ";" for the last line.
    auto endSegOffset = sourceMappings.find_first_of(",;", curSegOffset);
    if (endSegOffset == llvm::StringLiteral::npos) {
      endSegOffset = sourceMappings.size();
    }
    assert(
        endSegOffset <= sourceMappings.size() &&
        "endSegOffset cannot exceed sourceMappings size");
    bool lastSegmentInLine = endSegOffset == sourceMappings.size() ||
        sourceMappings[endSegOffset] == ';';

    const char *pCur = sourceMappings.data() + curSegOffset;
    const char *pSegEnd = sourceMappings.data() + endSegOffset;

    llvm::Optional<SourceMapGenerator::Segment> segmentOpt =
        parseSegment(prevSegment, pCur, pSegEnd);
    if (!segmentOpt.hasValue()) {
      return false;
    }
    segments.emplace_back(segmentOpt.getValue());

    // TODO: assert pCur equals to pSegEnd.

    if (!lastSegmentInLine) {
      prevSegment = segments.back();
    } else {
      // Make a copy so that we can make modification.
      lastLineSegment = segments.back();
      // generated column should be reset for new line.
      lastLineSegment.generatedColumn = 0;
      prevSegment = lastLineSegment;

      lines_.emplace_back(std::move(segments));
    }
    curSegOffset = endSegOffset + 1;
  }
  return true;
}

llvm::Optional<SourceMapGenerator::Segment> SourceMapParser::parseSegment(
    const SourceMapGenerator::Segment &prevSegment,
    const char *&pCur,
    const char *pSegEnd) {
  SourceMapGenerator::Segment segment;

  // Parse 1st field: generatedColumn.
  OptValue<int32_t> val = base64vlq::decode(pCur, pSegEnd);
  if (!val.hasValue()) {
    return llvm::None;
  }
  segment.generatedColumn = prevSegment.generatedColumn + val.getValue();

  // Parse 2nd field: sourceIndex.
  val = base64vlq::decode(pCur, pSegEnd);
  if (!val.hasValue()) {
    return segment;
  }
  segment.sourceIndex = prevSegment.sourceIndex + val.getValue();
  if ((size_t)segment.sourceIndex >= sources_.size()) {
    return llvm::None;
  }

  // Parse 3rd field: representedLine.
  val = base64vlq::decode(pCur, pSegEnd);
  if (!val.hasValue()) {
    // Segment can only be 1, 4 or 5 length.
    return llvm::None;
  }
  segment.representedLine = prevSegment.representedLine + val.getValue();

  // Parse 4th field: representedColumn.
  val = base64vlq::decode(pCur, pSegEnd);
  if (!val.hasValue()) {
    // Segment can only be 1, 4 or 5 length.
    return llvm::None;
  }
  segment.representedColumn = prevSegment.representedColumn + val.getValue();

  // Parse 5th field: nameIndex.
  val = base64vlq::decode(pCur, pSegEnd);
  if (!val.hasValue()) {
    return segment;
  }
  // TODO: store nameIndex in Segment.

  return segment;
}

llvm::Optional<SourceMapTextLocation> SourceMapParser::getLocationForAddress(
    uint32_t line,
    uint32_t column) {
  if (line == 0 || line > lines_.size()) {
    return llvm::None;
  }

  // line is 1-based.
  uint32_t lineIndex = line - 1;
  auto &segments = lines_[lineIndex];
  if (segments.empty()) {
    return llvm::None;
  }
  // Algorithm: we wanted to locate the segment covering
  // the needle(`column`) -- segment.generatedColumn <= column.
  // We achieve it by binary searching the first sentinel
  // segment strictly greater than needle(`column`) and then move backward
  // one slot.
  auto segIter = std::upper_bound(
      segments.begin(),
      segments.end(),
      column,
      [](uint32_t column, const SourceMapGenerator::Segment &seg) {
        return column < (uint32_t)seg.generatedColumn;
      });
  // The found sentinal segment is the first one. No covering segment.
  if (segIter == segments.begin()) {
    return llvm::None;
  }
  // Move back one slot.
  const SourceMapGenerator::Segment &target =
      segIter == segments.end() ? segments.back() : *(--segIter);
  // parseSegment() should have validated this.
  assert(
      (size_t)target.sourceIndex < sources_.size() &&
      "SourceIndex is out-of-range.");
  std::string fileName = getSourceFullPath(target.sourceIndex);
  return SourceMapTextLocation{std::move(fileName),
                               (uint32_t)target.representedLine,
                               (uint32_t)target.representedColumn};
}

} // namespace hermes
