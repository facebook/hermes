/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// DebugInfo methods that are only needed by the full (non-lean) build.
/// These are separated out because they have dependencies on libraries not
/// included in the lean build (e.g., SourceMapGenerator).
//===----------------------------------------------------------------------===//

#include "hermes/BCGen/HBC/DebugInfo.h"

#include "hermes/SourceMap/SourceMapGenerator.h"

namespace hermes::hbc {

using FunctionDebugInfoDeserializer = detail::FunctionDebugInfoDeserializer;

void DebugInfo::populateSourceMap(
    SourceMapGenerator *sourceMap,
    std::vector<uint32_t> &&functionOffsets,
    uint32_t segmentID) const {
  // Since our bytecode is not JavaScript, we interpret the source map in a
  // creative way: each bytecode module is represented as a line, and bytecode
  // addresses in the file are represented as column offsets.
  // Our debug information has a function start and then offsets within the
  // function, but the source map will do its own delta encoding, so we provide
  // absolute addresses to the source map.
  auto segmentFor = [&](const DebugSourceLocation &loc,
                        uint32_t offsetInFile,
                        uint32_t debugOffset) {
    SourceMap::Segment segment;
    segment.generatedColumn = loc.address + offsetInFile;
    assert(loc.line >= 1 && "line numbers in debug info must be 1-based");
    assert(loc.column >= 1 && "column numbers in debug info must be 1-based");
    segment.representedLocation = SourceMap::Segment::SourceLocation(
        sourceMap->getSourceIndex(
            getUTF8FilenameByID(*getFilenameForAddress(debugOffset))),
        loc.line - 1,
        loc.column - 1);
    return segment;
  };

  std::vector<SourceMap::Segment> segments;
  llvh::ArrayRef<uint8_t> locsData = sourceLocationsData();
  uint32_t offset = 0;
  while (offset < locsData.size()) {
    FunctionDebugInfoDeserializer fdid(locsData, offset);
    uint32_t offsetInFile = functionOffsets[fdid.getFunctionIndex()];
    segments.push_back(segmentFor(fdid.getCurrent(), offsetInFile, offset));
    while (!fdid.isDone()) {
      if (auto loc = fdid.next()) {
        segments.push_back(segmentFor(*loc, offsetInFile, offset));
      }
      // Don't add a segment for the debug info that has no location,
      // because we won't be reporting location info on instructions that have
      // no location information for the source map to translate.
    }
    offset = fdid.getOffset();
  }
  sourceMap->addMappingsLine(std::move(segments), segmentID);
  sourceMap->addFunctionOffsets(std::move(functionOffsets), segmentID);
}

} // namespace hermes::hbc
