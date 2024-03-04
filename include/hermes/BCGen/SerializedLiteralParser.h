/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_SERIALIZEDLITERALPARSER_H
#define HERMES_BCGEN_SERIALIZEDLITERALPARSER_H

#include "hermes/BCGen/SerializedLiteralGenerator.h"
#include "hermes/Support/Conversions.h"
#include "llvh/ADT/ArrayRef.h"
#include "llvh/Support/Endian.h"

namespace hermes::SerializedLiteralParser {

/// Visit \p totalLen elements serialized in the buffer \p buf with the given
/// visitor \p v.
/// The visitor should define the following methods, which will be invoked
/// when an element of the corresponding type is encountered.
///   void visitStringID(StringID);
///   void visitNumber(double);
///   void visitNull();
///   void visitBool(bool);
template <typename Visitor>
static void
parse(llvh::ArrayRef<uint8_t> buf, unsigned int totalLen, Visitor &v) {
  size_t bufIdx = 0;

  while (totalLen) {
    auto tag = buf[bufIdx++];
    // If the first bit in the tag is set to 1, it is an indication that the
    // sequence is longer than 15 elements, and that the tag is two bytes
    // long.
    size_t leftInSeq;
    if (tag & 0x80)
      leftInSeq = ((tag & 0x0f) << 8) | (buf[bufIdx++]);
    else
      leftInSeq = tag & 0x0f;

    SerializedLiteralGenerator::TagType type =
        tag & SerializedLiteralGenerator::TagMask;
    switch (type) {
      case SerializedLiteralGenerator::ByteStringTag:
        while (leftInSeq-- && totalLen--) {
          uint8_t val = llvh::support::endian::read<uint8_t, 1>(
              &buf[bufIdx], llvh::support::endianness::little);
          v.visitStringID(val);
          bufIdx += 1;
        }
        break;
      case SerializedLiteralGenerator::ShortStringTag:
        while (leftInSeq-- && totalLen--) {
          uint16_t val = llvh::support::endian::read<uint16_t, 1>(
              &buf[bufIdx], llvh::support::endianness::little);
          v.visitStringID(val);
          bufIdx += 2;
        }
        break;
      case SerializedLiteralGenerator::LongStringTag:
        while (leftInSeq-- && totalLen--) {
          uint32_t val = llvh::support::endian::read<uint32_t, 1>(
              &buf[bufIdx], llvh::support::endianness::little);
          v.visitStringID(val);
          bufIdx += 4;
        }
        break;
      case SerializedLiteralGenerator::NumberTag:
        while (leftInSeq-- && totalLen--) {
          double val = llvh::support::endian::read<double, 1>(
              &buf[bufIdx], llvh::support::endianness::little);
          v.visitNumber(val);
          bufIdx += 8;
        }
        break;
      case SerializedLiteralGenerator::IntegerTag:
        while (leftInSeq-- && totalLen--) {
          int32_t val = llvh::support::endian::read<int32_t, 1>(
              &buf[bufIdx], llvh::support::endianness::little);
          v.visitNumber(val);
          bufIdx += 4;
        }
        break;
      case SerializedLiteralGenerator::NullTag:
        while (leftInSeq-- && totalLen--)
          v.visitNull();
        break;
      case SerializedLiteralGenerator::TrueTag:
        while (leftInSeq-- && totalLen--)
          v.visitBool(true);
        break;
      case SerializedLiteralGenerator::FalseTag:
        while (leftInSeq-- && totalLen--)
          v.visitBool(false);
        break;
    }
  }
}

} // namespace hermes::SerializedLiteralParser

#endif // HERMES_BCGEN_SERIALIZEDLITERALPARSER_H
