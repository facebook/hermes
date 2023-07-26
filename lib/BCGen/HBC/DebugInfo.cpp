/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/DebugInfo.h"

#include "hermes/BCGen/HBC/ConsecutiveStringStorage.h"
#include "hermes/SourceMap/SourceMapGenerator.h"
#include "hermes/Support/UTF8.h"

using namespace hermes;
using namespace hbc;

namespace {

/// A type used to iteratively deserialize function debug info.
struct FunctionDebugInfoDeserializer {
  /// Construct a deserializer that begins deserializing at \p offset in \p
  /// data. It will deserialize until the function's debug info is finished
  /// (address delta = -1) at which point next() will return None. The offset of
  /// the next section can be obtained via getOffset().
  FunctionDebugInfoDeserializer(llvh::ArrayRef<uint8_t> data, uint32_t offset)
      : data_(data), offset_(offset) {
    functionIndex_ = decode1Int();
    current_.line = decode1Int();
    current_.column = decode1Int();
  }

  /// \return the next debug location, or None if we reach the end.
  /// Sample usage: while (auto loc = fdid.next()) {...}
  OptValue<DebugSourceLocation> next() {
    auto addressDelta = decode1Int();
    if (addressDelta == -1)
      return llvh::None;
    // Presence of the statement delta is LSB of line delta.
    int64_t lineDelta = decode1Int();
    int64_t columnDelta = decode1Int();
    int64_t scopeAddress = decode1Int();
    int64_t envReg = decode1Int();
    int64_t statementDelta = 0;
    if (lineDelta & 1)
      statementDelta = decode1Int();
    lineDelta >>= 1;

    current_.address += addressDelta;
    current_.line += lineDelta;
    current_.column += columnDelta;
    current_.statement += statementDelta;
    current_.scopeAddress = scopeAddress;
    current_.envReg = envReg;
    return current_;
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
};
} // namespace

DebugScopeDescriptor::Flags::Flags(uint32_t bits) {
  uint32_t mask = 1;
  assert(
      mask == (1 << static_cast<uint32_t>(Bits::InnerScope)) &&
      "mask is not Bits::InnerScope");
  isInnerScope = (bits & mask) != 0;

  mask <<= 1;
  assert(
      mask == (1 << static_cast<uint32_t>(Bits::Dynamic)) &&
      "mask is not Bits::Dynamic");
  isDynamic = (bits & mask) != 0;
}

uint32_t DebugScopeDescriptor::Flags::toUint32() const {
  uint32_t bits = 0;

  uint32_t mask = 1;
  assert(
      mask == (1 << static_cast<uint32_t>(Bits::InnerScope)) &&
      "mask is not Bits::InnerScope");
  bits |= isInnerScope ? mask : 0;

  mask <<= 1;
  assert(
      mask == (1 << static_cast<uint32_t>(Bits::Dynamic)) &&
      "mask is not Bits::Dynamic");
  bits |= isDynamic ? mask : 0;

  return bits;
}

/// Decodes a string at offset \p offset in \p data, updating offset in-place.
/// \return the decoded string.
static llvh::StringRef getString(
    uint32_t *inoutOffset,
    llvh::ArrayRef<uint8_t> data) {
  // The string is represented as its LEB-encoded length, followed by
  // the bytes. This format matches DebugInfoGenerator::appendString().
  uint32_t offset = *inoutOffset;
  int64_t strSize;
  offset += readSignedLEB128(data, offset, &strSize);
  assert(
      strSize >= 0 && // can't be negative
      strSize <= UINT_MAX && // can't overflow uint32
      uint32_t(strSize) + offset >= offset && // sum can't overflow
      uint32_t(strSize) + offset <= data.size() && // validate range
      "Invalid string size");
  const unsigned char *ptr = data.data() + offset;
  offset += strSize;
  *inoutOffset = offset;
  return llvh::StringRef(reinterpret_cast<const char *>(ptr), size_t(strSize));
}

void DebugInfoGenerator::appendString(
    std::vector<uint8_t> &data,
    Identifier str) {
  auto res = stringTableIndex_.try_emplace(
      str.getUnderlyingPointer(), stringTable_.size());

  if (res.second) {
#ifndef NDEBUG
    // Check that the given string is a valid UTF8 string.
    const char *it = str.str().begin();
    while (it < str.str().end()) {
      constexpr bool allowSurrogates = false;
      // keep the current position alive so it can be inspected in the debugger
      // if the assert fails.
      const char *pos = it;
      (void)pos;
      decodeUTF8<allowSurrogates>(it, [](const llvh::Twine &) {
        assert(false && "invalid utf8 char");
      });
    }
    assert(
        it == str.str().end() && "Invalid utf8 string -- read past end of str");
#endif // NDEBUG
    appendSignedLEB128(stringTable_, int64_t(str.str().size()));
    stringTable_.insert(stringTable_.end(), str.str().begin(), str.str().end());
  }

  appendSignedLEB128(data, res.first->second);
}

llvh::StringRef DebugInfo::decodeString(
    uint32_t *inoutOffset,
    llvh::ArrayRef<uint8_t> data) const {
  int64_t strOffset;
  *inoutOffset += readSignedLEB128(data, *inoutOffset, &strOffset);
  uint32_t strOffsetU = strOffset;
  return getString(&strOffsetU, stringTableData());
}

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
  DebugSourceLocation lastLocation = fdid.getCurrent();
  uint32_t lastLocationOffset = debugOffset;
  uint32_t nextLocationOffset = fdid.getOffset();
  while (auto loc = fdid.next()) {
    if (loc->address > offsetInFunction)
      break;
    lastLocation = *loc;
    lastLocationOffset = nextLocationOffset;
    nextLocationOffset = fdid.getOffset();
  }
  if (auto file = getFilenameForAddress(lastLocationOffset)) {
    lastLocation.address = offsetInFunction;
    lastLocation.filenameId = *file;
    return lastLocation;
  }
  return llvh::None;
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
      end = scopeDescDataOffset_;
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
    while (auto loc = fdid.next()) {
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

OptValue<llvh::StringRef> DebugInfo::getTextifiedCalleeUTF8(
    uint32_t debugOffset,
    uint32_t offsetInFunction) const {
  llvh::ArrayRef<uint8_t> data = textifiedCalleeData();
  int64_t entries;
  debugOffset += readSignedLEB128(data, debugOffset, &entries);
  while (entries--) {
    int64_t location;
    debugOffset += readSignedLEB128(data, debugOffset, &location);
    llvh::StringRef name = decodeString(&debugOffset, data);
    if (location == offsetInFunction) {
      return name;
    }
    if (location > offsetInFunction) {
      break;
    }
  }

  return llvh::None;
}

DebugScopeDescriptor DebugInfo::getScopeDescriptor(uint32_t offset) const {
  // Incoming offset is given relative to our lexical region.
  llvh::ArrayRef<uint8_t> data = scopeDescData();
  int64_t parentId;
  int64_t flags;
  int64_t signedCount;
  offset += readSignedLEB128(data, offset, &parentId);
  offset += readSignedLEB128(data, offset, &flags);
  offset += readSignedLEB128(data, offset, &signedCount);
  (void)parentId;
  assert(signedCount >= 0 && "Invalid variable name count");
  const auto numAccessibleNames = static_cast<size_t>(signedCount);

  DebugScopeDescriptor desc;
  if ((int32_t)parentId >= 0) {
    desc.parentOffset = parentId;
  }
  desc.flags = DebugScopeDescriptor::Flags{static_cast<uint32_t>(flags)};

  desc.names.reserve(numAccessibleNames);
  for (size_t i = 0; i < numAccessibleNames; i++) {
    llvh::StringRef name = decodeString(&offset, data);
    desc.names.push_back(name);
  }
  return desc;
}

void DebugInfo::disassembleFilenames(llvh::raw_ostream &os) const {
  os << "Debug filename table:\n";
  for (uint32_t i = 0, e = filenameTable_.size(); i < e; ++i) {
    os << "  " << i << ": " << getFilenameByID(i) << '\n';
  }
  if (filenameTable_.empty()) {
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
    while (auto loc = fdid.next()) {
      OS << "    bc " << loc->address << ": line " << loc->line << " col "
         << loc->column << " scope offset "
         << llvh::format_hex(loc->scopeAddress, 6) << " env ";
      if (loc->envReg != DebugSourceLocation::NO_REG) {
        OS << "r" << loc->envReg;
      } else {
        OS << "none";
      }
      OS << "\n";
      count++;
    }
    if (count == 0) {
      OS << "    (none)\n";
    }
    offset = fdid.getOffset();
  }
  OS << "  " << llvh::format_hex(offset, 6)
     << "  end of debug source table\n\n";
}

void DebugInfo::disassembleScopeDescData(llvh::raw_ostream &OS) const {
  uint32_t offset = 0;
  llvh::ArrayRef<uint8_t> scopeData = scopeDescData();

  OS << "Debug scope descriptor table:\n";
  auto next = [&]() {
    int64_t result;
    offset += readSignedLEB128(scopeData, offset, &result);
    return (int32_t)result;
  };
  while (offset < scopeData.size()) {
    OS << "  " << llvh::format_hex(offset, 6);
    int64_t parentId = next();
    DebugScopeDescriptor::Flags flags{static_cast<uint32_t>(next())};
    int64_t varNamesCount = next();
    OS << "  lexical parent: ";
    if (parentId == -1) {
      OS << "  none";
    } else {
      OS << llvh::format_hex(parentId, 6);
    }
    OS << ", flags: ";
    OS << (flags.isInnerScope ? "Is" : "  ");
    OS << (flags.isDynamic ? "D" : " ");
    OS << ", variable count: " << varNamesCount;
    OS << '\n';
    for (int64_t i = 0; i < varNamesCount; i++) {
      llvh::StringRef name = decodeString(&offset, scopeData);
      OS << "    \"";
      OS.write_escaped(name);
      OS << "\"\n";
    }
  }
  assert(offset == scopeData.size());
  OS << "  " << llvh::format_hex(offset, 6)
     << "  end of debug scope descriptor table\n\n";
}

void DebugInfo::disassembleTextifiedCallee(llvh::raw_ostream &OS) const {
  uint32_t offset = 0;
  llvh::ArrayRef<uint8_t> data = textifiedCalleeData();

  OS << "Textified callees table:\n";
  auto nextInt = [&]() {
    int64_t result;
    offset += readSignedLEB128(data, offset, &result);
    return (int32_t)result;
  };
  while (offset < data.size()) {
    OS << "  " << llvh::format_hex(offset, 6);
    int64_t numEntries = nextInt();
    OS << "  entries: " << numEntries << "\n";
    while (numEntries--) {
      int64_t location = nextInt();
      llvh::StringRef tCallee = decodeString(&offset, data);

      OS << "    bc " << location << " calls ";
      OS.write_escaped(tCallee);
      OS << "\n";
    }
  }
  OS << "  " << llvh::format_hex(offset, 6);
  OS << "  end of textified callees table\n\n";
}

void DebugInfo::disassembleStringTable(llvh::raw_ostream &OS) const {
  uint32_t offset = 0;
  llvh::ArrayRef<uint8_t> stringTable = stringTableData();

  OS << "Debug string table:\n";
  while (offset < stringTable.size()) {
    OS << "  " << llvh::format_hex(offset, 6) << " ";
    OS.write_escaped(getString(&offset, stringTable));
    OS << '\n';
  }
  assert(offset == stringTable.size());
  OS << "  " << llvh::format_hex(offset, 6) << "  end of debug string table\n";
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
            getFilenameByID(*getFilenameForAddress(debugOffset))),
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
    while (auto loc = fdid.next())
      segments.push_back(segmentFor(*loc, offsetInFile, offset));
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

  if (offsets.empty()) {
    return DebugOffsets::NO_OFFSET;
  }
  const uint32_t startOffset = sourcesData_.size();

  if (files_.empty() || files_.back().filenameId != start.filenameId) {
    files_.push_back(DebugFileRegion{
        startOffset, start.filenameId, start.sourceMappingUrlId});
  }

  appendSignedLEB128(sourcesData_, functionIndex);
  appendSignedLEB128(sourcesData_, start.line);
  appendSignedLEB128(sourcesData_, start.column);
  const DebugSourceLocation *previous = &start;

  for (auto &next : offsets) {
    if (next.filenameId != previous->filenameId) {
      files_.push_back(DebugFileRegion{
          (unsigned)sourcesData_.size(),
          next.filenameId,
          start.sourceMappingUrlId});
    }

    int32_t adelta = delta(next.address, previous->address);
    // ldelta needs 64 bits because we will use it to encode an extra bit.
    int64_t ldelta = delta(next.line, previous->line);
    int32_t cdelta = delta(next.column, previous->column);
    int32_t sdelta = delta(next.statement, previous->statement);

    // Encode the presence of statementNo as a bit in the line delta, which is
    // usually very small.
    // ldelta encoding: bits 1..32 contain the line delta. Bit 0 indicates the
    // presence of statementNo.
    ldelta = (ldelta * 2) + (sdelta != 0);

    appendSignedLEB128(sourcesData_, adelta);
    appendSignedLEB128(sourcesData_, ldelta);
    appendSignedLEB128(sourcesData_, cdelta);
    appendSignedLEB128(sourcesData_, next.scopeAddress);
    appendSignedLEB128(sourcesData_, next.envReg);
    if (sdelta)
      appendSignedLEB128(sourcesData_, sdelta);
    previous = &next;
  }
  appendSignedLEB128(sourcesData_, -1);

  return startOffset;
}

DebugInfoGenerator::DebugInfoGenerator(UniquingFilenameTable &&filenameTable)
    : filenameStrings_(
          UniquingFilenameTable::toStorage(std::move(filenameTable))) {
  assert(
      textifiedCallees_.size() == kMostCommonEntryOffset &&
      "Textified callee should initially be kMostCommonEntryOffset");
  assert(
      scopeDescData_.size() == kMostCommonEntryOffset &&
      "Scope desc data should initially be kMostCommonEntryOffset");

  // Initialize the empty data entry in debug lexical table.
  // Initialize the empty data entry in debug lexical table.
  appendSignedLEB128(scopeDescData_, -1); // parent function
  appendSignedLEB128(scopeDescData_, 0); // flags
  appendSignedLEB128(scopeDescData_, 0); // name count

  // Textified callee table.
  appendSignedLEB128(textifiedCallees_, 0);
}

uint32_t DebugInfoGenerator::appendTextifiedCalleeData(
    llvh::ArrayRef<DebugTextifiedCallee> textifiedCallees) {
  if (textifiedCallees.empty()) {
    return kMostCommonEntryOffset;
  }
  const uint32_t startOffset = textifiedCallees_.size();
  appendSignedLEB128(textifiedCallees_, textifiedCallees.size());
  for (const DebugTextifiedCallee &callee : textifiedCallees) {
    appendSignedLEB128(textifiedCallees_, callee.address);
    appendString(textifiedCallees_, callee.textifiedCallee);
  }
  return startOffset;
}

uint32_t DebugInfoGenerator::appendScopeDesc(
    OptValue<uint32_t> parentScopeOffset,
    DebugScopeDescriptor::Flags flags,
    llvh::ArrayRef<Identifier> names) {
  assert(validData && "DebugInfoGenerator not valid");
  if (!parentScopeOffset.hasValue() && names.empty()) {
    return kMostCommonEntryOffset;
  }
  const uint32_t startOffset = scopeDescData_.size();
  appendSignedLEB128(
      scopeDescData_, !parentScopeOffset.hasValue() ? -1 : *parentScopeOffset);
  appendSignedLEB128(scopeDescData_, flags.toUint32());
  appendSignedLEB128(scopeDescData_, names.size());
  for (Identifier name : names) {
    appendString(scopeDescData_, name);
  }
  return startOffset;
}

DebugInfo DebugInfoGenerator::serializeWithMove() {
  assert(validData);
  validData = false;

  auto combinedData = std::move(sourcesData_);

  combinedData.reserve(
      combinedData.size() + scopeDescData_.size() + textifiedCallees_.size() +
      stringTable_.size());

  uint32_t scopeDescStart = combinedData.size();
  combinedData.insert(
      combinedData.end(), scopeDescData_.begin(), scopeDescData_.end());

  uint32_t textifiedCalleeStart = combinedData.size();
  combinedData.insert(
      combinedData.end(), textifiedCallees_.begin(), textifiedCallees_.end());

  // Append the string table data.
  uint32_t stringTableStart = combinedData.size();
  combinedData.insert(
      combinedData.end(), stringTable_.begin(), stringTable_.end());

  return DebugInfo(
      std::move(filenameStrings_),
      std::move(files_),
      scopeDescStart,
      textifiedCalleeStart,
      stringTableStart,
      StreamVector<uint8_t>(std::move(combinedData)));
}
