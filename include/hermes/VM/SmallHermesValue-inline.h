/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SMALLHERMESVALUE_INLINE_H
#define HERMES_VM_SMALLHERMESVALUE_INLINE_H

#include "hermes/VM/SmallHermesValue.h"

#include "hermes/Support/SlowAssert.h"
#include "hermes/VM/BoxedDouble.h"
#include "hermes/VM/HermesValue-inline.h"
#include "hermes/VM/PointerBase.h"
#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

void SmallHermesValueAdaptor::setInGC(SmallHermesValueAdaptor hv, GC *gc) {
  HermesValue::setInGC(hv, gc);
}

void HermesValue32::setInGC(HermesValue32 hv, GC *gc) {
  setNoBarrier(hv);
  assert(gc->calledByGC());
}

HermesValue HermesValue32::unboxToHV(PointerBase *pb) const {
  switch (getETag()) {
    case ETag::Object1:
    case ETag::Object2:
      return HermesValue::encodeObjectValue(getObject(pb));
    case ETag::String1:
    case ETag::String2:
      return HermesValue::encodeStringValue(getString(pb));
    case ETag::BoxedDouble1:
    case ETag::BoxedDouble2:
      return HermesValue::encodeNumberValue(
          vmcast<BoxedDouble>(getPointer(pb))->get());
    case ETag::SmallInt1:
    case ETag::SmallInt2:
      return HermesValue::encodeNumberValue(getSmallInt());
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

HermesValue HermesValue32::toHV(PointerBase *pb) const {
  if (getTag() == Tag::BoxedDouble)
    return HermesValue::encodeObjectValue(getPointer(pb));
  return unboxToHV(pb);
}

GCCell *HermesValue32::getPointer(PointerBase *pb) const {
  assert(isPointer());
  RawType rawPtr = raw_ & llvh::maskLeadingOnes<RawType>(kNumValueBits);
  return GCPointerBase::storageTypeToPointer(
      GCPointerBase::rawToStorageType(rawPtr), pb);
}

GCCell *HermesValue32::getObject(PointerBase *pb) const {
  assert(isObject());
  // Since object pointers are the most common type, we have them as the
  // zero-tag and can decode them without needing to remove the tag.
  static_assert(
      static_cast<uint8_t>(Tag::Object) == 0,
      "Object tag must be zero for fast path.");
  return GCPointerBase::storageTypeToPointer(
      GCPointerBase::rawToStorageType(raw_), pb);
}

StringPrimitive *HermesValue32::getString(PointerBase *pb) const {
  assert(isString());
  return vmcast<StringPrimitive>(getPointer(pb));
}

double HermesValue32::getNumber(PointerBase *pb) const {
  assert(isNumber());
  if (LLVM_LIKELY(getTag() == Tag::SmallInt))
    return getSmallInt();
  return vmcast<BoxedDouble>(getPointer(pb))->get();
}

/* static */ HermesValue32 HermesValue32::encodeHermesValue(
    HermesValue hv,
    Runtime *runtime) {
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
    case HermesValue::ETag::Object1:
    case HermesValue::ETag::Object2:
      return encodeObjectValue(static_cast<GCCell *>(hv.getObject()), runtime);
    default:
      assert(
          hv.isNumber() && "Native values are forbidden in SmallHermesValue");
      return encodeNumberValue(hv.getNumber(), runtime);
  }
}

/* static */ HermesValue32 HermesValue32::encodeNumberValue(
    double d,
    Runtime *runtime) {
  // Always box values when Handle-SAN is on so we can catch any mistakes.
#ifndef HERMESVM_SANITIZE_HANDLES
  const SmiType i = doubleToSmi(d);
  if (LLVM_LIKELY(
          llvh::DoubleToBits(d) == llvh::DoubleToBits(i) &&
          llvh::isInt<kNumSmiBits>(i)))
    return fromTagAndValue(Tag::SmallInt, i);
#endif
  return encodePointerImpl(
      BoxedDouble::create(d, runtime), Tag::BoxedDouble, runtime);
}

/* static */ HermesValue32
HermesValue32::encodePointerImpl(GCCell *ptr, Tag tag, PointerBase *pb) {
  static_assert(
      sizeof(RawType) == sizeof(GCPointerBase::StorageType),
      "Raw storage must fit a GCPointer");
  RawType p = GCPointerBase::storageTypeToRaw(
      GCPointerBase::pointerToStorageType(ptr, pb));
  validatePointer(p);
  return fromRaw(p | static_cast<RawType>(tag));
}

HermesValue32 HermesValue32::updatePointer(GCCell *ptr, PointerBase *pb) const {
  return encodePointerImpl(ptr, getTag(), pb);
}

void HermesValue32::unsafeUpdatePointer(GCCell *ptr, PointerBase *pb) {
  setNoBarrier(encodePointerImpl(ptr, getTag(), pb));
}

/* static */ HermesValue32 HermesValue32::encodeObjectValue(
    GCCell *ptr,
    PointerBase *pb) {
  assert(
      (!ptr || !vmisa<StringPrimitive>(ptr)) &&
      "Strings must use encodeStringValue");
  return encodePointerImpl(ptr, Tag::Object, pb);
}
/* static */ HermesValue32 HermesValue32::encodeStringValue(
    StringPrimitive *ptr,
    PointerBase *pb) {
  return encodePointerImpl(static_cast<GCCell *>(ptr), Tag::String, pb);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SMALLHERMESVALUE_INLINE_H
