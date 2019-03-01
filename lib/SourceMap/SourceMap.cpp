/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/SourceMap/SourceMap.h"

#include "hermes/Parser/JSONParser.h"
#include "hermes/Support/JSONEmitter.h"

#include <algorithm>

using namespace hermes;
using namespace hermes::parser;

namespace hermes {
namespace base64vlq {

static constexpr uint32_t Base64Count = 64;
static constexpr const char Base64Chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
// Expect to have 64 characters + terminating null (unfortunately)
static_assert(
    sizeof(Base64Chars) == Base64Count + 1,
    "Base64Chars has unexpected length");

/// Decode a Base64 character.
/// \return the integer value, or None if not a Base64 character.
static OptValue<uint32_t> base64Decode(char c) {
  // This is not very optimal. A 127-byte lookup table would be faster.
  for (const char &bc : Base64Chars) {
    if (c == bc)
      return &bc - &Base64Chars[0];
  }
  return llvm::None;
}

// Each digit is stored in the low 5 bits, with bit 6 as a continuation flag.
enum {
  // Width in bits of each VLQ digit.
  DigitWidth = 5,

  // Mask to get at just the digit bits of a Base64 value.
  DigitMask = (1 << DigitWidth) - 1,

  // Flag indicating more digits follow.
  ContinuationFlag = 1 << DigitWidth,

  // The first digit reserves the LSB for the sign of the final value.
  SignBit = 1,
};

llvm::raw_ostream &encode(llvm::raw_ostream &OS, int32_t value) {
  // The first sextet reserves the LSB for the sign bit. Make space for it.
  // Widen to 64 bits to ensure we can multiply the value by 2.
  int64_t wideVal = value;
  wideVal *= 2;
  if (wideVal < 0)
    wideVal = -wideVal | SignBit;
  assert(wideVal >= 0 && "wideVal should not be negative any more");
  do {
    auto digit = wideVal & DigitMask;
    wideVal >>= DigitWidth;
    if (wideVal > 0)
      digit |= ContinuationFlag;
    assert(digit < Base64Count && "digit cannot exceed Base64 character count");
    OS << Base64Chars[digit];
  } while (wideVal > 0);
  return OS;
}

OptValue<int32_t> decode(const char *&begin, const char *end) {
  int64_t result = 0;
  for (const char *cursor = begin; cursor < end; cursor++) {
    OptValue<uint32_t> word = base64Decode(*cursor);
    int32_t shift = DigitWidth * (cursor - begin);

    // Fail if our shift has grown too large, or if we couldn't decode a Base64
    // character. This shift check is what ensures 'result' cannot overflow.
    if (!word || shift > 32)
      return llvm::None;

    // Digits are encoded little-endian (least-significant first).
    int64_t digit = *word & DigitMask;
    result |= (digit << shift);

    // Continue if we have a continuation flag.
    if (*word & ContinuationFlag)
      continue;

    // We're done. The sign bit is the LSB; fix up the sign.
    // Ensure we use a /2 (not shift) because we need round-towards-zero.
    if (result & SignBit) {
      result = -result;
    }
    result /= 2;

    // Check for overflow.
    if (result > INT32_MAX || result < INT32_MIN)
      return llvm::None;

    // Success. Update the begin pointer to say where we stopped.
    begin = cursor + 1;
    return int32_t(result);
  }
  // Exited the loop: we never found a character without a continuation bit.
  return llvm::None;
}
} // namespace base64vlq

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

SourceMapGenerator::Segment SourceMapGenerator::encodeSourceLocations(
    const Segment &lastSegment,
    llvm::ArrayRef<Segment> segments,
    llvm::raw_ostream &OS) {
  // Currently we only support a single source, so the source ID (and its delta)
  // is always 0.
  Segment prev = lastSegment;
  bool first = true;
  for (const Segment &seg : segments) {
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
  Segment lastSegment;
  // Lines are 0 based: the first line is 0 and not 1, therefore initialize it
  // to 1 so we encode it with a delta of 0.
  lastSegment.representedLine = 1;
  for (const SegmentList &segments : lines_) {
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
