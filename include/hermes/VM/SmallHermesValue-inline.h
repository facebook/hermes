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
  auto tag = getTag();
  if (tag == Tag::CompressedHV64)
    return HermesValue::fromRaw(compressedHV64ToBits());
  switch (tag) {
    case Tag::Object:
      return HermesValue::encodeObjectValue(getObject(pb));
    case Tag::BigInt:
      return HermesValue::encodeBigIntValue(getBigInt(pb));
    case Tag::String:
      return HermesValue::encodeStringValue(getString(pb));
    case Tag::BoxedDouble:
      return HermesValue::encodeTrustedNumberValue(
          vmcast<BoxedDouble>(getPointer(pb))->get());
    case Tag::Symbol:
      return HermesValue::encodeSymbolValue(SymbolID::unsafeCreate(getValue()));
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
  if (LLVM_LIKELY(getTag() == Tag::CompressedHV64))
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

  if (hv.isNumberOrCompressible())
    return encodeCompressibleOrNumberHV64(hv, runtime);

  switch (hv.getETag()) {
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
      llvm_unreachable("No other ETag");
  }
}

/* static */ HermesValue32 HermesValue32::encodeCompressibleOrNumberHV64(
    HermesValue hv,
    Runtime &runtime) {
  const uint64_t hvRaw = hv.getRaw();

#ifdef HERMESVM_SANITIZE_HANDLES
  // If Handle-San is enabled, always box doubles on the heap. This ensures that
  // callers have to treat a HermesValue32 containing a number as a pointer.
  if (!hv.isNumber())
    return bitsToCompressedHV64(hvRaw);
#else
  constexpr uint64_t kShiftAmount = 64 - kNumValueBits;
  // If hvRaw is the part that would go into the HV32 value, followed
  // by zeros (i.e., it's equal to a value of kNumValueBits bits
  // right-shifted to the top of the 64 bit value), then we can compress.
  // (Note: the double parens after LLVM_LIKELY below are for macro args.)
  if (LLVM_LIKELY((llvh::isShiftedUInt<kNumValueBits, kShiftAmount>(hvRaw))))
    return bitsToCompressedHV64(hvRaw);

  assert(hv.isNumber() && "Must be compressible or number HV64");
#endif

  return encodePointerImpl(
      BoxedDouble::create(hv.getNumber(), runtime), Tag::BoxedDouble, runtime);
}

/* static */ HermesValue32 HermesValue32::encodeNumberValue(
    double d,
    Runtime &runtime) {
  return encodeCompressibleOrNumberHV64(
      HermesValue::encodeTrustedNumberValue(d), runtime);
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
