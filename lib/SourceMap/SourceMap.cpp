/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/SourceMap/SourceMap.h"

using namespace hermes;

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

llvm::Optional<SourceMapTextLocation> SourceMap::getLocationForAddress(
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
      [](uint32_t column, const Segment &seg) {
        return column < (uint32_t)seg.generatedColumn;
      });
  // The found sentinal segment is the first one. No covering segment.
  if (segIter == segments.begin()) {
    return llvm::None;
  }
  // Move back one slot.
  const Segment &target =
      segIter == segments.end() ? segments.back() : *(--segIter);
  // Unmapped location
  if (!target.representedLocation.hasValue()) {
    return llvm::None;
  }
  // parseSegment() should have validated this.
  assert(
      (size_t)target.representedLocation->sourceIndex < sources_.size() &&
      "SourceIndex is out-of-range.");
  std::string fileName =
      getSourceFullPath(target.representedLocation->sourceIndex);
  return SourceMapTextLocation{
      std::move(fileName),
      (uint32_t)target.representedLocation->lineIndex,
      (uint32_t)target.representedLocation->columnIndex};
}

} // namespace hermes
