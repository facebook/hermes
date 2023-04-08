/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/SHSerializedLiteralParser.h"

#include "hermes/BCGen/HBC/SerializedLiteralGenerator.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/RuntimeModule.h"
#include "hermes/VM/static_h.h"

#include "llvh/Support/Endian.h"

namespace hermes {
namespace vm {

using SLG = hermes::hbc::SerializedLiteralGenerator;

HermesValue SHSerializedLiteralParser::get(Runtime &runtime) {
  assert(hasNext() && "Object buffer doesn't have any more values");

  if (leftInSeq_ == 0)
    parseTagAndSeqLength();
  leftInSeq_--;
  elemsLeft_--;

  // Passing the nullptr instead of a SHRuntime signifies that
  // the generator is generating object key values, and that it should
  // just return the StringID rather than find a String Primitive.
  switch (lastTag_) {
    case SLG::ByteStringTag: {
      uint8_t val = llvh::support::endian::read<uint8_t, 1>(
          buffer_.data() + currIdx_, llvh::support::endianness::little);
      currIdx_ += 1;
      return unit_ == nullptr
          ? HermesValue::encodeSymbolValue(SymbolID::unsafeCreate(val))
          : HermesValue::encodeStringValue(runtime.getStringPrimFromSymbolID(
                SymbolID::unsafeCreate(unit_->symbols[val])));
    }
    case SLG::ShortStringTag: {
      uint16_t val = llvh::support::endian::read<uint16_t, 1>(
          buffer_.data() + currIdx_, llvh::support::endianness::little);
      currIdx_ += 2;
      return unit_ == nullptr
          ? HermesValue::encodeSymbolValue(SymbolID::unsafeCreate(val))
          : HermesValue::encodeStringValue(runtime.getStringPrimFromSymbolID(
                SymbolID::unsafeCreate(unit_->symbols[val])));
    }
    case SLG::LongStringTag: {
      uint32_t val = llvh::support::endian::read<uint32_t, 1>(
          buffer_.data() + currIdx_, llvh::support::endianness::little);
      currIdx_ += 4;
      return unit_ == nullptr
          ? HermesValue::encodeSymbolValue(SymbolID::unsafeCreate(val))
          : HermesValue::encodeStringValue(runtime.getStringPrimFromSymbolID(
                SymbolID::unsafeCreate(unit_->symbols[val])));
    }
    case SLG::NumberTag: {
      double val = llvh::support::endian::read<double, 1>(
          buffer_.data() + currIdx_, llvh::support::endianness::little);
      currIdx_ += 8;
      return HermesValue::encodeTrustedNumberValue(val);
    }
    case SLG::IntegerTag: {
      int32_t val = llvh::support::endian::read<int32_t, 1>(
          buffer_.data() + currIdx_, llvh::support::endianness::little);
      currIdx_ += 4;
      return HermesValue::encodeTrustedNumberValue(val);
    }
    case SLG::NullTag:
      return HermesValue::encodeNullValue();
    case SLG::TrueTag:
      return HermesValue::encodeBoolValue(true);
    case SLG::FalseTag:
      return HermesValue::encodeBoolValue(false);
  }
  llvm_unreachable("No other valid tag");
}

} // namespace vm
} // namespace hermes
