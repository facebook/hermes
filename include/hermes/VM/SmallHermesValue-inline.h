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
  switch (getTag()) {
    case Tag::Object:
      return HermesValue::encodeObjectValue(getObject(pb));
    case Tag::String:
      return HermesValue::encodeStringValue(getString(pb));
    case Tag::BoxedDouble:
      return HermesValue::encodeNumberValue(
          vmcast<BoxedDouble>(getPointer(pb))->get());
    case Tag::SmallInt:
      return HermesValue::encodeNumberValue(getSmallInt());
    case Tag::Symbol:
      return HermesValue::encodeSymbolValue(SymbolID::unsafeCreate(getValue()));
    case Tag::Bool:
      return HermesValue::encodeBoolValue(getValue());
    case Tag::Extended:
      switch (getValueTag()) {
        case ValueTag::Undefined:
          return HermesValue::encodeUndefinedValue();
        case ValueTag::Empty:
          return HermesValue::encodeEmptyValue();
        case ValueTag::Null:
          return HermesValue::encodeNullValue();
      }
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
  return getPointer(pb);
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

/* static */ HermesValue32
HermesValue32::encodeHermesValue(HermesValue hv, GC *gc, PointerBase *pb) {
#ifdef HERMESVM_SANITIZE_HANDLES
  // When Handle-SAN is on, make a double so that this function always
  // allocates. Note we can't do this for pointer values since they would get
  // invalidated.
  if (!hv.isPointer())
    encodeNumberValue(0.0, gc, pb);
#endif
  switch (hv.getETag()) {
    case ETag::Empty:
      return encodeEmptyValue();
    case ETag::Undefined:
      return encodeUndefinedValue();
    case ETag::Null:
      return encodeNullValue();
    case ETag::Bool:
      return encodeBoolValue(hv.getBool());
    case ETag::Symbol:
      return encodeSymbolValue(hv.getSymbol());
    case ETag::Str1:
    case ETag::Str2:
      return encodeStringValue(hv.getString(), pb);
    case ETag::Object1:
    case ETag::Object2:
      return encodeObjectValue(static_cast<GCCell *>(hv.getObject()), pb);
    default:
      assert(
          hv.isNumber() && "Native values are forbidden in SmallHermesValue");
      return encodeNumberValue(hv.getNumber(), gc, pb);
  }
}

/* static */ HermesValue32 HermesValue32::encodeHermesValue(
    HermesValue hv,
    Runtime *runtime) {
  return encodeHermesValue(hv, &runtime->getHeap(), runtime);
}

/* static */ HermesValue32
HermesValue32::encodeNumberValue(double d, GC *gc, PointerBase *pb) {
  // Always box values when Handle-SAN is on so we can catch any mistakes.
#ifndef HERMESVM_SANITIZE_HANDLES
  const SmiType i = doubleToSmi(d);
  if (LLVM_LIKELY(
          i == d && llvh::isInt<kNumSmiBits>(i) &&
          llvh::DoubleToBits(d) != llvh::DoubleToBits(-0.0)))
    return fromTagAndValue(Tag::SmallInt, i);
#endif
  return encodePointerImpl(BoxedDouble::create(d, gc), Tag::BoxedDouble, pb);
}

/* static */ HermesValue32 HermesValue32::encodeNumberValue(
    double d,
    Runtime *runtime) {
  return encodeNumberValue(d, &runtime->getHeap(), runtime);
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
