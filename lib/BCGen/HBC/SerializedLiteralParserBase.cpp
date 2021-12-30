/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/SerializedLiteralParserBase.h"
#include "hermes/BCGen/HBC/SerializedLiteralGenerator.h"

#include "llvh/Support/Endian.h"

namespace hermes {
namespace hbc {

void SerializedLiteralParserBase::parseTagAndSeqLength() {
  auto tag = buffer_[currIdx_];
  // If the first bit in the tag is set to 1, it is an indication that the
  // sequence is longer than 15 elements, and that the tag is two bytes long.
  // The format is still the same as described in
  // `SerializedLiteralParserBase.h`.
  if (tag & 0x80) {
    leftInSeq_ = ((tag & 0x0f) << 8) | (buffer_[currIdx_ + 1]);
    currIdx_ += 2;
  } else {
    leftInSeq_ = tag & 0x0f;
    currIdx_ += 1;
  }

  lastTag_ = tag & SerializedLiteralGenerator::TagMask;
}

} // namespace hbc
} // namespace hermes
