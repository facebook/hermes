/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/SerializedLiteralGenerator.h"
#include "hermes/BCGen/HBC/BytecodeGenerator.h"
#include "llvh/Support/Endian.h"

namespace hermes {
namespace hbc {

namespace {
void appendTagToBuffer(
    std::vector<unsigned char> &buff,
    SerializedLiteralGenerator::TagType tag,
    int seqLength) {
  if (seqLength > 15) {
    buff.push_back((tag | 0x80) | (seqLength >> 8));
    buff.push_back(seqLength & 0xffff);
  } else {
    buff.push_back(tag + seqLength);
  }
}

/// Write a value to a vector of characters in little-endian format.
template <typename value_type>
void serializeValueToBuffer(
    value_type value,
    std::vector<unsigned char> &buff) {
  // Since endian::write takes a pointer, we can write directly to
  // the buffer. First we resize it to make sure the data fits, then
  // we pass a pointer to the original end of the vector.
  // Since the buffer is a char buffer, we pass in an alignment of 1
  // to endian::write.
  buff.resize(buff.size() + sizeof(value_type));
  llvh::support::endian::write<value_type, 1>(
      buff.data() + buff.size() - sizeof(value_type),
      value,
      llvh::support::endianness::little);
}
} // namespace

void SerializedLiteralGenerator::serializeBuffer(
    llvh::ArrayRef<Literal *> literals,
    std::vector<unsigned char> &buff,
    bool isKeyBuffer) {
  // Stores the last type parsed by the for loop
  TagType lastTag = NumberTag;

  // Stores the type currently being parsed by the for loop
  TagType newTag = NumberTag;

  // Stores the values of each serialized Literal in a sequence so that
  // they can be added to tempBuff after the tag is finalized
  std::vector<unsigned char> tmpSeqBuffer;

  // Stores the length of the current type sequence
  size_t seqLength = 0;

  for (size_t i = 0, buffSize = literals.size(); i < buffSize; ++i) {
    // The first switch simply sets newTag.
    // If newTag is different from lastTag, we append the tag and the
    // tmpSeqBuffer to buff.
    // After this check, we serialize the value to tmpSeqBuffer if possible.
    // Since we can only write the tmpSeqBuffer to buff once we know the
    // sequence ends, the check has to go after the type checking switch.
    // Since we can only write to tmpSeqBuffer after it's been cleared (if
    // the sequence is over), the only way to delay the write to tmpSeqBuffer
    // is to either write to a second temporary buffer, and write that to
    // tmpSeqBuffer afterwards, or add a second switch.
    switch (literals[i]->getKind()) {
      case ValueKind::LiteralNumberKind:
        newTag = llvh::cast<LiteralNumber>(literals[i])
                     ->isIntTypeRepresentible<int32_t>()
            ? IntegerTag
            : NumberTag;
        break;
      case ValueKind::LiteralStringKind: {
        auto str = llvh::cast<LiteralString>(literals[i])->getValue().str();
        int ind =
            isKeyBuffer ? BMGen_.getIdentifierID(str) : BMGen_.getStringID(str);

        if (ind > UINT16_MAX) {
          newTag = LongStringTag;
        } else if (ind > UINT8_MAX) {
          newTag = ShortStringTag;
        } else {
          newTag = ByteStringTag;
        }
        break;
      }
      case ValueKind::LiteralBoolKind:
        newTag = llvh::cast<LiteralBool>(literals[i])->getValue() ? TrueTag
                                                                  : FalseTag;
        break;
      case ValueKind::LiteralNullKind:
        newTag = NullTag;
        break;
      default:
        llvm_unreachable("Invalid Literal Kind");
    }

    if (newTag != lastTag || seqLength == SequenceMax) {
      if (seqLength > 0) {
        appendTagToBuffer(buff, lastTag, seqLength);
        buff.insert(buff.end(), tmpSeqBuffer.begin(), tmpSeqBuffer.end());
        tmpSeqBuffer.resize(0);
      }
      lastTag = newTag;
      seqLength = 0;
    }
    seqLength++;

    switch (literals[i]->getKind()) {
      case ValueKind::LiteralNumberKind: {
        auto litNum = llvh::cast<LiteralNumber>(literals[i]);
        if (llvh::Optional<int> intPointer =
                litNum->isIntTypeRepresentible<int32_t>()) {
          int n = intPointer.getValue();
          serializeValueToBuffer<uint32_t>(n, tmpSeqBuffer);
        } else {
          double num = litNum->getValue();
          serializeValueToBuffer<double>(num, tmpSeqBuffer);
        }
        break;
      }
      case ValueKind::LiteralStringKind: {
        // For strings, we are going to store the index to the string table,
        // which will need to be decoded at runtime.
        auto str = llvh::cast<LiteralString>(literals[i])->getValue().str();
        auto stringID =
            isKeyBuffer ? BMGen_.getIdentifierID(str) : BMGen_.getStringID(str);

        if (stringID > UINT16_MAX) {
          serializeValueToBuffer<uint32_t>(stringID, tmpSeqBuffer);
        } else if (stringID > UINT8_MAX) {
          serializeValueToBuffer<uint16_t>(stringID, tmpSeqBuffer);
        } else {
          serializeValueToBuffer<uint8_t>(stringID, tmpSeqBuffer);
        }
        break;
      }
      case ValueKind::LiteralBoolKind:
      case ValueKind::LiteralNullKind:
        /* no-op */
        break;
      default:
        llvm_unreachable("Invalid Literal Kind");
    }
  }
  // The last value in the buffer can't get serialized in the loop.
  appendTagToBuffer(buff, lastTag, seqLength);
  buff.insert(buff.end(), tmpSeqBuffer.begin(), tmpSeqBuffer.end());
}

} // namespace hbc
} // namespace hermes
