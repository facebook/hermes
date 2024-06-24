/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SMALLHERMESVALUE_INLINE_H
#define HERMES_VM_SMALLHERMESVALUE_INLINE_H

#include "hermes/VM/SmallHermesValue.h"

#include "hermes/Support/SlowAssert.h"
#include "hermes/VM/BigIntPrimitive.h"
#include "hermes/VM/BoxedDouble.h"
#include "hermes/VM/HermesValue-inline.h"
#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

#ifndef HERMESVM_BOXED_DOUBLES

void SmallHermesValueAdaptor::setInGC(SmallHermesValueAdaptor hv, GC &gc) {
  HermesValue::setInGC(hv, gc);
}

#else // #ifndef HERMESVM_BOXED_DOUBLES

void HermesValue32::setInGC(HermesValue32 hv, GC &gc) {
  setNoBarrier(hv);
  assert(gc.calledByGC());
}

HermesValue HermesValue32::unboxToHV(PointerBase &pb) const {
  switch (getETag()) {
    case ETag::Object1:
    case ETag::Object2:
      return HermesValue::encodeObjectValue(getObject(pb));
    case ETag::BigInt1:
    case ETag::BigInt2:
      return HermesValue::encodeBigIntValue(getBigInt(pb));
    case ETag::String1:
    case ETag::String2:
      return HermesValue::encodeStringValue(getString(pb));
    case ETag::BoxedDouble1:
    case ETag::BoxedDouble2:
      return HermesValue::encodeTrustedNumberValue(
          vmcast<BoxedDouble>(getPointer(pb))->get());
    case ETag::CompressedDouble1:
    case ETag::CompressedDouble2:
      return HermesValue::fromRaw(compressedDoubleToBits());
    case ETag::Symbol1:
    case ETag::Symbol2:
      return HermesValue::encodeSymbolValue(SymbolID::unsafeCreate(getValue()));
    case ETag::Bool:
      return HermesValue::encodeBoolValue(getETagValue());
    case ETag::Undefined:
      return HermesValue::encodeUndefinedValue();
    case ETag::Empty:
      return HermesValue::encodeEmptyValue();
    case ETag::Null:
      return HermesValue::encodeNullValue();
    default:
      llvm_unreachable("No other tag");
  }
}

HermesValue HermesValue32::toHV(PointerBase &pb) const {
  if (getTag() == Tag::BoxedDouble)
    return HermesValue::encodeObjectValue(getPointer(pb));
  return unboxToHV(pb);
}

BigIntPrimitive *HermesValue32::getBigInt(PointerBase &pb) const {
  assert(isBigInt());
  return vmcast<BigIntPrimitive>(getPointer(pb));
}

StringPrimitive *HermesValue32::getString(PointerBase &pb) const {
  assert(isString());
  return vmcast<StringPrimitive>(getPointer(pb));
}

double HermesValue32::getNumber(PointerBase &pb) const {
  assert(isNumber());
  if (LLVM_LIKELY(getTag() == Tag::CompressedDouble))
    return getCompressedDouble();
  return vmcast<BoxedDouble>(getPointer(pb))->get();
}

double HermesValue32::getBoxedDouble(PointerBase &pb) const {
  assert(isBoxedDouble());
  return vmcast<BoxedDouble>(getPointer(pb))->get();
}

/* static */ HermesValue32 HermesValue32::encodeHermesValue(
    HermesValue hv,
    Runtime &runtime) {
#ifdef HERMESVM_SANITIZE_HANDLES
  // When Handle-SAN is on, make a double so that this function always
  // allocates. Note we can't do this for pointer values since they would get
  // invalidated.
  if (!hv.isPointer())
    encodeNumberValue(0.0, runtime);
#endif
  switch (hv.getETag()) {
    case HermesValue::ETag::Empty:
      return encodeEmptyValue();
    case HermesValue::ETag::Undefined:
      return encodeUndefinedValue();
    case HermesValue::ETag::Null:
      return encodeNullValue();
    case HermesValue::ETag::Bool:
      return encodeBoolValue(hv.getBool());
    case HermesValue::ETag::Symbol:
      return encodeSymbolValue(hv.getSymbol());
    case HermesValue::ETag::Str1:
    case HermesValue::ETag::Str2:
      return encodeStringValue(hv.getString(), runtime);
    case HermesValue::ETag::BigInt1:
    case HermesValue::ETag::BigInt2:
      return encodeBigIntValue(hv.getBigInt(), runtime);
    case HermesValue::ETag::Object1:
    case HermesValue::ETag::Object2:
      return encodeObjectValue(static_cast<GCCell *>(hv.getObject()), runtime);
    default:
      assert(
          hv.isNumber() && "Native values are forbidden in SmallHermesValue");
      return encodeNumberValue(hv, runtime);
  }
}

/* static */ HermesValue32 HermesValue32::encodeNumberValue(
    HermesValue hv,
    Runtime &runtime) {
  assert(hv.isNumber() && "May only be called for Number values.");
#ifndef HERMESVM_SANITIZE_HANDLES
  const uint64_t hvRaw = hv.getRaw();
  constexpr uint64_t kShiftAmount = 32 + kNumTagBits;
  // If hvRaw is the part that would go into the HV32 value, followed
  // by zeros (i.e., it's equal to a value of kNumValueBits bits
  // right-shifted by 32 plus the HV32 tag bits), then we can
  // compress.  (Note: the double parens after LLVM_LIKELY below are
  // for macro args.)
  if (LLVM_LIKELY((llvh::isShiftedUInt<kNumValueBits, kShiftAmount>(hvRaw)))) {
    const uint64_t hvHighVal = hvRaw >> kShiftAmount;
    return fromTagAndValue(Tag::CompressedDouble, hvHighVal);
  }
  // Otherwise:
#endif
  return encodePointerImpl(
      BoxedDouble::create(hv.getNumber(), runtime), Tag::BoxedDouble, runtime);
}

/* static */ HermesValue32 HermesValue32::encodeNumberValue(
    double d,
    Runtime &runtime) {
  return encodeNumberValue(HermesValue::encodeTrustedNumberValue(d), runtime);
}

/* static */ HermesValue32 HermesValue32::encodeObjectValue(
    GCCell *ptr,
    PointerBase &pb) {
  assert(
      (!ptr || !vmisa<StringPrimitive>(ptr) || !vmisa<BigIntPrimitive>(ptr)) &&
      "Strings must use encodeStringValue; BigInts, encodeBigIntValue");
  return encodePointerImpl(ptr, Tag::Object, pb);
}

#endif // #ifndef HERMESVM_BOXED_DOUBLES

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SMALLHERMESVALUE_INLINE_H
