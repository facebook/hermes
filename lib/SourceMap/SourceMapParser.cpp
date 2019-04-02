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

std::unique_ptr<SourceMap> SourceMapParser::parse(
    llvm::StringRef sourceMapContent) {
  parser::JSLexer::Allocator alloc;
  parser::JSONFactory factory(alloc);
  SourceErrorManager sm;
  parser::JSONParser jsonParser(factory, sourceMapContent, sm);

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
    return nullptr;
  }

  auto *version = llvm::dyn_cast_or_null<JSONNumber>(json->get("version"));
  if (version == nullptr) {
    return nullptr;
  }
  if ((uint64_t)version->getValue() != 3) {
    return nullptr;
  }

  // sourceRoot is optional.
  std::string sourceRoot;
  auto *sourceRootJson =
      llvm::dyn_cast_or_null<JSONString>(json->get("sourceRoot"));
  if (sourceRootJson != nullptr) {
    sourceRoot = sourceRootJson->str();
  }

  auto *sourcesJson = llvm::dyn_cast_or_null<JSONArray>(json->get("sources"));
  if (sourcesJson == nullptr) {
    return nullptr;
  }

  std::vector<std::string> sources(sourcesJson->size());
  for (unsigned i = 0, e = sources.size(); i < e; ++i) {
    auto *file = llvm::dyn_cast_or_null<JSONString>(sourcesJson->at(i));
    if (file == nullptr) {
      return nullptr;
    }
    sources[i] = file->str();
  }

  auto *mappings = llvm::dyn_cast_or_null<JSONString>(json->get("mappings"));
  if (mappings == nullptr) {
    return nullptr;
  }

  std::vector<SourceMap::SegmentList> lines;
  bool succeed = parseMappings(mappings->str(), lines);
  if (!succeed) {
    return nullptr;
  }
  return llvm::make_unique<SourceMap>(
      sourceRoot, std::move(sources), std::move(lines));
}

bool SourceMapParser::parseMappings(
    llvm::StringRef sourceMappings,
    std::vector<SourceMap::SegmentList> &lines) {
  assert(lines.empty() && "lines is an out parameter so should be empty");
  SourceMap::SegmentList segments;
  // Represent last line's segment value.
  SourceMap::Segment lastLineSegment;
  // Lines are encoded zero-based in source map while query
  // via 1-based so converting representedLine to be 1-based.
  lastLineSegment.representedLine = 1;
  SourceMap::Segment prevSegment = lastLineSegment;

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

    if (pCur == pSegEnd && lastSegmentInLine && segments.empty()) {
      // The line is empty, so avoid doing any extra work.
      lines.emplace_back(std::move(segments));
    } else {
      llvm::Optional<SourceMap::Segment> segmentOpt =
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

        lines.emplace_back(std::move(segments));
      }
    }
    curSegOffset = endSegOffset + 1;
  }
  return true;
}

llvm::Optional<SourceMap::Segment> SourceMapParser::parseSegment(
    const SourceMap::Segment &prevSegment,
    const char *&pCur,
    const char *pSegEnd) {
  SourceMap::Segment segment;

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

} // namespace hermes
