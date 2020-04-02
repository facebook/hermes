/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/SerializedLiteralParser.h"

#include "hermes/BCGen/HBC/SerializedLiteralGenerator.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/RuntimeModule.h"

#include "llvm/Support/Endian.h"

namespace hermes {
namespace vm {

using SLG = hermes::hbc::SerializedLiteralGenerator;

HermesValue SerializedLiteralParser::get(Runtime *) {
  assert(hasNext() && "Object buffer doesn't have any more values");

  if (leftInSeq_ == 0) {
    parseTagAndSeqLength();
    // We set lastValues for types which don't require reading from the buffer
    // in the if clause, to avoid unnecessarily creating new HermesValues later
    switch (lastTag_) {
      case SLG::ByteStringTag:
      case SLG::ShortStringTag:
      case SLG::LongStringTag:
      case SLG::NumberTag:
      case SLG::IntegerTag:
        break;
      case SLG::NullTag:
        lastValue_ = HermesValue::encodeNullValue();
        break;
      case SLG::TrueTag:
        lastValue_ = HermesValue::encodeBoolValue(true);
        break;
      case SLG::FalseTag:
        lastValue_ = HermesValue::encodeBoolValue(false);
        break;
    }
  }
  leftInSeq_--;
  elemsLeft_--;

  // Passing the nullptr instead of a RuntimeModule signifies that
  // the generator is generating object key values, and that it should
  // just return the StringID rather than find a String Primitive.
  switch (lastTag_) {
    case SLG::ByteStringTag: {
      uint8_t val = llvm::support::endian::read<uint8_t, 1>(
          buffer_.data() + currIdx_, llvm::support::endianness::little);
      lastValue_ = runtimeModule_ == nullptr
          ? HermesValue::encodeSymbolValue(SymbolID::unsafeCreate(val))
          : HermesValue::encodeStringValue(
                runtimeModule_->getStringPrimFromStringIDMayAllocate(val));
      currIdx_ += 1;
      break;
    }
    case SLG::ShortStringTag: {
      uint16_t val = llvm::support::endian::read<uint16_t, 1>(
          buffer_.data() + currIdx_, llvm::support::endianness::little);
      lastValue_ = runtimeModule_ == nullptr
          ? HermesValue::encodeSymbolValue(SymbolID::unsafeCreate(val))
          : HermesValue::encodeStringValue(
                runtimeModule_->getStringPrimFromStringIDMayAllocate(val));
      currIdx_ += 2;
      break;
    }
    case SLG::LongStringTag: {
      uint32_t val = llvm::support::endian::read<uint32_t, 1>(
          buffer_.data() + currIdx_, llvm::support::endianness::little);
      lastValue_ = runtimeModule_ == nullptr
          ? HermesValue::encodeSymbolValue(SymbolID::unsafeCreate(val))
          : HermesValue::encodeStringValue(
                runtimeModule_->getStringPrimFromStringIDMayAllocate(val));
      currIdx_ += 4;
      break;
    }
    case SLG::NumberTag: {
      double val = llvm::support::endian::read<double, 1>(
          buffer_.data() + currIdx_, llvm::support::endianness::little);
      lastValue_ = HermesValue::encodeNumberValue(val);
      currIdx_ += 8;
      break;
    }
    case SLG::IntegerTag: {
      int32_t val = llvm::support::endian::read<int32_t, 1>(
          buffer_.data() + currIdx_, llvm::support::endianness::little);
      lastValue_ = HermesValue::encodeNumberValue((double)val);
      currIdx_ += 4;
      break;
    }
    default:
      break;
  }
  return lastValue_;
}

} // namespace vm
} // namespace hermes
