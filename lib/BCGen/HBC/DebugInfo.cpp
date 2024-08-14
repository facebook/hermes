/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/DebugInfo.h"

#include "hermes/BCGen/HBC/ConsecutiveStringStorage.h"
#include "hermes/SourceMap/SourceMapGenerator.h"

using namespace hermes;
using namespace hbc;

namespace {

/// A type used to iteratively deserialize function debug info.
struct FunctionDebugInfoDeserializer {
  /// Construct a deserializer that begins deserializing at \p offset in \p
  /// data. It will deserialize until the function's debug info is finished
  /// (address delta = -1) at which point isDone() will return true. The offset
  /// of the next section can be obtained via getOffset().
  /// isDone() will not be true until after the first call to next().
  FunctionDebugInfoDeserializer(llvh::ArrayRef<uint8_t> data, uint32_t offset)
      : data_(data), offset_(offset) {
    functionIndex_ = decode1Int();
    current_.line = decode1Int();
    current_.column = decode1Int();
  }

  /// \return the next debug location, or None if we reach the end.
  /// Sample usage: while (auto loc = fdid.next()) {...}
  OptValue<DebugSourceLocation> next() {
    assert(!done_ && "next() called after done");

    auto addressDelta = decode1Int();
    if (addressDelta == -1) {
      done_ = true;
      return llvh::None;
    }
    current_.address += addressDelta;

    // ldelta encoding: bits 2..32 contain the line delta.
    // Bit 0 indicates the presence of location information.
    // Bit 1 indicates the presence of statement information.

    int64_t lineDelta = decode1Int();
    if ((lineDelta & 1) == 0) {
      // No location information here, but not done yet.
      return llvh::None;
    }

    int64_t columnDelta = decode1Int();
    int64_t statementDelta = 0;
    if ((lineDelta & 2) != 0)
      statementDelta = decode1Int();
    lineDelta >>= 2;

    current_.line += lineDelta;
    current_.column += columnDelta;
    current_.statement += statementDelta;
    return current_;
  }

  /// \return whether we're done reading the debug info for this function.
  /// next() must not be called when isDone() is true.
  bool isDone() const {
    return done_;
  }

  /// \return the offset of this deserializer in the data.
  uint32_t getOffset() const {
    return offset_;
  }

  /// \return the index of the function being deserialized.
  uint32_t getFunctionIndex() const {
    return functionIndex_;
  }

  /// \return the current source location.
  const DebugSourceLocation &getCurrent() const {
    return current_;
  }

 private:
  /// LEB-decode the next int.
  int64_t decode1Int() {
    int64_t result;
    offset_ += readSignedLEB128(data_, offset_, &result);
    return result;
  }

  llvh::ArrayRef<uint8_t> data_;
  uint32_t offset_;

  uint32_t functionIndex_;
  DebugSourceLocation current_;

  bool done_ = false;
};
} // namespace

OptValue<uint32_t> DebugInfo::getFilenameForAddress(
    uint32_t debugOffset) const {
  OptValue<uint32_t> value = llvh::None;
  // This is sorted list of (address, filename) pairs so we could use
  // binary search. However, we expect the number of entries to be
  // between zero and one.
  for (int i = 0, e = files_.size(); i < e; i++) {
    if (files_[i].fromAddress <= debugOffset) {
      value = files_[i].filenameId;
    } else
      break;
  }
  return value;
}

OptValue<DebugSourceLocation> DebugInfo::getLocationForAddress(
    uint32_t debugOffset,
    uint32_t offsetInFunction) const {
  assert(debugOffset < data_.size() && "Debug offset out of range");
  FunctionDebugInfoDeserializer fdid(data_.getData(), debugOffset);
  OptValue<DebugSourceLocation> lastLocation = fdid.getCurrent();
  uint32_t lastLocationOffset = debugOffset;
  uint32_t nextLocationOffset = fdid.getOffset();
  for (;;) {
    // isDone() won't be true on the first iteration, because fdid was just
    // constructed.
    auto nextLocation = fdid.next();
    if (fdid.isDone() || fdid.getCurrent().address > offsetInFunction)
      break;
    lastLocation = nextLocation;
    lastLocationOffset = nextLocationOffset;
    nextLocationOffset = fdid.getOffset();
  }
  if (!lastLocation)
    return llvh::None;
  DebugSourceLocation result = *lastLocation;
  if (auto file = getFilenameForAddress(lastLocationOffset)) {
    result.address = offsetInFunction;
    result.filenameId = *file;
    return result;
  }
  return llvh::None;
}

DebugSourceLocation DebugInfo::getLocationForFunction(
    uint32_t debugOffset) const {
  assert(debugOffset < data_.size() && "Debug offset out of range");
  FunctionDebugInfoDeserializer fdid(data_.getData(), debugOffset);
  return fdid.getCurrent();
}

OptValue<DebugSearchResult> DebugInfo::getAddressForLocation(
    uint32_t filenameId,
    uint32_t targetLine,
    OptValue<uint32_t> targetColumn) const {
  // First, get the start/end debug offsets for the given file.
  uint32_t start = 0;
  uint32_t end = 0;
  bool foundFile{false};
  for (const auto &cur : files_) {
    if (foundFile) {
      // We must have found the file on the last iteration,
      // so set the beginning of this file to be the end index,
      // and break out of the loop, ensuring we know where the target
      // file starts and ends.
      end = cur.fromAddress;
      break;
    }
    if (cur.filenameId == filenameId) {
      foundFile = true;
      start = cur.fromAddress;
      end = sourceLocationsData().size();
    }
  }
  if (!foundFile) {
    return llvh::None;
  }

  unsigned offset = start;

  // We consider the best match for a location to be the first of:
  // 1. Exact match
  // 2. Exact line, largest column before the target
  // 3. Exact line, any column
  // If multiple, the lowest bytecode address wins in each category.
  DebugSearchResult best(0, DebugOffsets::NO_OFFSET, 0, 0);

  while (offset < end) {
    FunctionDebugInfoDeserializer fdid(data_.getData(), offset);
    while (!fdid.isDone()) {
      auto loc = fdid.next();
      if (!loc.hasValue()) {
        continue;
      }
      uint32_t line = loc->line;
      uint32_t column = loc->column;
      if (line == targetLine) {
        if (!targetColumn.hasValue() || column == *targetColumn) {
          // Short-circuit on a precise match.
          return DebugSearchResult(
              fdid.getFunctionIndex(), loc->address, line, column);
        }

        if (best.bytecodeOffset == DebugOffsets::NO_OFFSET ||
            (column <= *targetColumn &&
             (best.column > *targetColumn || column > best.column))) {
          best = DebugSearchResult(
              fdid.getFunctionIndex(), loc->address, line, column);
        }
      }
    }
    offset = fdid.getOffset();
  }

  if (best.bytecodeOffset == DebugOffsets::NO_OFFSET) {
    return llvh::None;
  }

  return best;
}

void DebugInfo::disassembleFilenames(llvh::raw_ostream &os) const {
  os << "Debug filename table:\n";
  for (uint32_t i = 0, e = getFilenameTable().size(); i < e; ++i) {
    os << "  " << i << ": " << getUTF8FilenameByID(i) << '\n';
  }
  if (getFilenameTable().empty()) {
    os << "  (none)\n";
  }
  os << '\n';
}

void DebugInfo::disassembleFilesAndOffsets(llvh::raw_ostream &OS) const {
  OS << "Debug file table:\n";
  for (int i = 0, e = files_.size(); i < e; i++) {
    OS << "  source table offset " << llvh::format_hex(files_[i].fromAddress, 6)
       << ": filename id " << files_[i].filenameId << "\n";
  }
  if (files_.empty()) {
    OS << "  (none)\n";
  }
  OS << "\n";

  OS << "Debug source table:\n";

  uint32_t offset = 0;
  llvh::ArrayRef<uint8_t> locsData = sourceLocationsData();
  while (offset < locsData.size()) {
    FunctionDebugInfoDeserializer fdid(locsData, offset);
    OS << "  " << llvh::format_hex(offset, 6);
    OS << "  function idx " << fdid.getFunctionIndex();
    OS << ", starts at line " << fdid.getCurrent().line << " col "
       << fdid.getCurrent().column << "\n";
    uint32_t count = 0;
    while (!fdid.isDone()) {
      if (auto loc = fdid.next()) {
        OS << "    bc " << loc->address << ": line " << loc->line << " col "
           << loc->column << "\n";
        count++;
      }
    }
    if (count == 0) {
      OS << "    (none)\n";
    }
    offset = fdid.getOffset();
  }
  OS << "  " << llvh::format_hex(offset, 6)
     << "  end of debug source table\n\n";
}

#ifndef HERMESVM_LEAN
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
#endif

uint32_t DebugInfoGenerator::appendSourceLocations(
    const DebugSourceLocation &start,
    uint32_t functionIndex,
    llvh::ArrayRef<DebugSourceLocation> offsets) {
  assert(validData && "DebugInfoGenerator not valid");

  // The start of the function isn't part of a statement,
  // so require that statement = 0 for the start debug value.
  assert(start.statement == 0 && "function must start at statement 0");

  auto &files = debugInfo_.getFilesMut();
  auto &sourcesData = debugInfo_.getDataMut();

  if (offsets.empty()) {
    return DebugOffsets::NO_OFFSET;
  }
  const uint32_t startOffset = sourcesData.size();

  if (files.empty() || files.back().filenameId != start.filenameId) {
    files.push_back(DebugFileRegion{
        startOffset, start.filenameId, start.sourceMappingUrlId});
  }

  appendSignedLEB128(sourcesData, functionIndex);
  appendSignedLEB128(sourcesData, start.line);
  appendSignedLEB128(sourcesData, start.column);

  // The previous real source location that has been emitted.
  const DebugSourceLocation *previous = &start;
  // The previous address that has been emitted.
  uint32_t previousAddress = start.address;

  for (auto &next : offsets) {
    if (next.filenameId != previous->filenameId) {
      files.push_back(DebugFileRegion{
          (unsigned)sourcesData.size(),
          next.filenameId,
          start.sourceMappingUrlId});
    }

    int32_t adelta = delta(next.address, previousAddress);
    appendSignedLEB128(sourcesData, adelta);
    previousAddress = next.address;

    if (next.line != 0) {
      // There is a location.
      // ldelta needs 64 bits because we will use it to encode an extra bit.
      int64_t ldelta = delta(next.line, previous->line);
      int32_t cdelta = delta(next.column, previous->column);
      int32_t sdelta = delta(next.statement, previous->statement);

      // Encode the presence of location info as a bit in the line delta,
      // which is usually very small.
      // ldelta encoding: bits 2..32 contain the line delta.
      // Bit 0 indicates the presence of location information.
      // Bit 1 indicates the presence of statement information.
      // This is only ever decoded in FunctionDebugInfoDeserializer.
      ldelta = (ldelta << 2) | ((sdelta != 0) << 1) | 1;
      appendSignedLEB128(sourcesData, ldelta);
      appendSignedLEB128(sourcesData, cdelta);
      if (sdelta)
        appendSignedLEB128(sourcesData, sdelta);
      previous = &next;
    } else {
      // There is no location.
      appendSignedLEB128(sourcesData, 0);
    }
  }
  appendSignedLEB128(sourcesData, -1);

  debugInfo_.resetDataRef();
  return startOffset;
}

void DebugInfoGenerator::generate() && {
  assert(validData);
  validData = false;

  debugInfo_.getFilenameTableMut().appendFilenamesToStorage();
}
