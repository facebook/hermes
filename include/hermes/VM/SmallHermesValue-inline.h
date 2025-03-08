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
  // We check for CompressedHV64 and pointers first, because we know that they
  // comprise the overwhelming majority of values.
  if (tag == Tag::CompressedHV64)
    return HermesValue::fromRaw(compressedHV64ToBits());
  if (tag <= Tag::Object) {
    // This must be a pointer tag, since the only tag before the first pointer
    // tag is CompressedHV64, which we have already checked for.
    assert(tag == Tag::Object || tag == Tag::BigInt || tag == Tag::String);
    auto toHV64Tag = [](Tag tag) {
      // Compute the HV64 tag by applying an offset to the HV32 tag.
      auto offs = (uint32_t)HermesValue::Tag::Str - (uint32_t)Tag::String;
      return (HermesValue::Tag)(offs + (uint32_t)tag);
    };
    // Check that the calculation is correct.
    static_assert(toHV64Tag(Tag::Object) == HermesValue::Tag::Object);
    static_assert(toHV64Tag(Tag::BigInt) == HermesValue::Tag::BigInt);
    static_assert(toHV64Tag(Tag::String) == HermesValue::Tag::Str);
    return HermesValue::fromTagAndValue(
        toHV64Tag(tag), (uint64_t)getPointer(pb));
  }
  if (tag == Tag::Symbol)
    return HermesValue::encodeSymbolValue(getSymbol());
  assert(tag == Tag::BoxedDouble);
  return HermesValue::encodeTrustedNumberValue(
      vmcast<BoxedDouble>(getPointer(pb))->get());
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

  // We check for CompressedHV64 and pointers first, because we know that they
  // comprise the overwhelming majority of values.
  if (hv.isNumberOrCompressible())
    return encodeCompressibleOrNumberHV64(hv, runtime);

  if (hv.isPointer()) {
    auto tag = hv.getTag();
    // Make sure we have accounted for all the HV tags.
    assert(
        tag == HermesValue::Tag::Str || tag == HermesValue::Tag::BigInt ||
        tag == HermesValue::Tag::Object);
    auto toHV32Tag = [](HermesValue::Tag tag) {
      // Compute the HV32 tag by applying an offset to the HV64 tag.
      auto offs = (uint32_t)HermesValue::Tag::Str - (uint32_t)Tag::String;
      return (Tag)((uint32_t)tag - offs);
    };
    // Check that the calculation is correct.
    static_assert(toHV32Tag(HermesValue::Tag::Object) == Tag::Object);
    static_assert(toHV32Tag(HermesValue::Tag::BigInt) == Tag::BigInt);
    static_assert(toHV32Tag(HermesValue::Tag::Str) == Tag::String);
    return encodePointerImpl(
        (GCCell *)hv.getPointer(), toHV32Tag(tag), runtime);
  }
  assert(hv.isSymbol() && "Must be a symbol");
  return encodeSymbolValue(hv.getSymbol());
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
