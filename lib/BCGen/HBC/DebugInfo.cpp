/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/DebugInfo.h"

#include "hermes/BCGen/HBC/ConsecutiveStringStorage.h"

namespace hermes::hbc {

using FunctionDebugInfoDeserializer = detail::FunctionDebugInfoDeserializer;

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

uint32_t DebugInfoGenerator::addScopingInfo(
    const DebugScopingInfo &scopingInfo) {
  auto &scopingInfoTable = debugInfo_.getScopingInfoTable();
  // 0 is reserved to mean no scoping info, so start from 1.
  auto nextID = scopingInfoTable.size() + 1;
  auto [iter, success] =
      debugInfo_.getScopingInfoCache().try_emplace(scopingInfo.asKey(), nextID);
  if (success) {
    scopingInfoTable.push_back(scopingInfo);
  }
  return iter->second;
}

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
    files.push_back(
        DebugFileRegion{
            startOffset, start.filenameId, start.sourceMappingUrlId});
  }

  appendSignedLEB128(sourcesData, functionIndex);
  appendSignedLEB128(sourcesData, start.line);
  appendSignedLEB128(sourcesData, start.column);
  appendSignedLEB128(sourcesData, start.envIdx);

  // The previous real source location that has been emitted.
  const DebugSourceLocation *previous = &start;
  // The previous address that has been emitted.
  uint32_t previousAddress = start.address;

  for (auto &next : offsets) {
    if (next.filenameId != previous->filenameId) {
      files.push_back(
          DebugFileRegion{
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
      int32_t envIdxDelta = delta(next.envIdx, previous->envIdx);

      // Encode the presence of location info as a bit in the line delta,
      // which is usually very small.
      // ldelta encoding: bits 3..32 contain the line delta.
      // Bit 0 indicates the presence of location information.
      // Bit 1 indicates the presence of statement information.
      // Bit 2 indicates the presence of envIdx information.
      // This is only ever decoded in FunctionDebugInfoDeserializer.
      ldelta =
          (ldelta << 3) | ((envIdxDelta != 0) << 2) | ((sdelta != 0) << 1) | 1;
      appendSignedLEB128(sourcesData, ldelta);
      appendSignedLEB128(sourcesData, cdelta);

      if (sdelta)
        appendSignedLEB128(sourcesData, sdelta);

      if (envIdxDelta)
        appendSignedLEB128(sourcesData, envIdxDelta);

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

} // namespace hermes::hbc
