/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SMALLHERMESVALUE_H
#define HERMES_VM_SMALLHERMESVALUE_H

#include "hermes/Support/Algorithms.h"
#include "hermes/VM/CompressedPointer.h"
#include "hermes/VM/GCDecl.h"
#include "hermes/VM/HeapAlign.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/SegmentInfo.h"
#include "hermes/VM/SymbolID.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
namespace hermes {
namespace vm {

class BigIntPrimitive;
class StringPrimitive;
class GCCell;
class Runtime;

/// An adaptor class that provides the API of a SmallHermesValue is internally
/// just a HermesValue.
class SmallHermesValueAdaptor : protected HermesValue {
  constexpr explicit SmallHermesValueAdaptor(HermesValue hv)
      : HermesValue(hv) {}

 public:
  SmallHermesValueAdaptor() = default;
  SmallHermesValueAdaptor(const SmallHermesValueAdaptor &) = default;

#ifdef _MSC_VER
  // This is a workaround to ensure is_trivial is true in MSVC.
  SmallHermesValueAdaptor(SmallHermesValueAdaptor &&) = default;
  SmallHermesValueAdaptor &operator=(const SmallHermesValueAdaptor &hv) =
      default;
#else
  SmallHermesValueAdaptor &operator=(const SmallHermesValueAdaptor &hv) =
      delete;
#endif

  using HermesValue::getBool;
  using HermesValue::getSymbol;
  using HermesValue::isBigInt;
  using HermesValue::isBool;
  using HermesValue::isEmpty;
  using HermesValue::isNull;
  using HermesValue::isNumber;
  using HermesValue::isObject;
  using HermesValue::isPointer;
  using HermesValue::isString;
  using HermesValue::isSymbol;
  using HermesValue::isUndefined;

  HermesValue toHV(PointerBase &) const {
    return *this;
  }
  HermesValue unboxToHV(PointerBase &) const {
    return *this;
  }

  GCCell *getPointer(PointerBase &) const {
    return static_cast<GCCell *>(HermesValue::getPointer());
  }
  CompressedPointer getPointer() const {
    assert(
        sizeof(uintptr_t) == sizeof(CompressedPointer::RawType) &&
        "Adaptor should not be used when compressed pointers are enabled.");
    uintptr_t rawPtr = reinterpret_cast<uintptr_t>(HermesValue::getPointer());
    return CompressedPointer::fromRaw(rawPtr);
  }
  GCCell *getObject(PointerBase &) const {
    return static_cast<GCCell *>(HermesValue::getObject());
  }
  StringPrimitive *getString(PointerBase &) const {
    return HermesValue::getString();
  }
  BigIntPrimitive *getBigInt(PointerBase &) const {
    return HermesValue::getBigInt();
  }
  double getNumber(PointerBase &) const {
    return HermesValue::getNumber();
  }
  uint32_t getRelocationID() const {
    return reinterpret_cast<uintptr_t>(HermesValue::getPointer());
  }

  inline void setInGC(SmallHermesValueAdaptor hv, GC &gc);

  SmallHermesValueAdaptor updatePointer(GCCell *ptr, PointerBase &) const {
    return SmallHermesValueAdaptor{HermesValue::updatePointer(ptr)};
  }
  SmallHermesValueAdaptor updatePointer(CompressedPointer ptr) const {
    assert(
        sizeof(uintptr_t) == sizeof(CompressedPointer::RawType) &&
        "Adaptor should not be used when compressed pointers are enabled.");
    GCCell *cellPtr = reinterpret_cast<GCCell *>(ptr.getRaw());
    return SmallHermesValueAdaptor{HermesValue::updatePointer(cellPtr)};
  }
  void unsafeUpdateRelocationID(uint32_t id) {
    HermesValue::unsafeUpdatePointer(
        reinterpret_cast<void *>(static_cast<uintptr_t>(id)));
  }
  void unsafeUpdatePointer(GCCell *ptr, PointerBase &) {
    HermesValue::unsafeUpdatePointer(ptr);
  }

  static constexpr SmallHermesValueAdaptor
  encodeHermesValue(HermesValue hv, GC &, PointerBase &) {
    return SmallHermesValueAdaptor{hv};
  }
  static constexpr SmallHermesValueAdaptor encodeHermesValue(
      HermesValue hv,
      Runtime &) {
    return SmallHermesValueAdaptor{hv};
  }
  static SmallHermesValueAdaptor encodeBigIntValue(
      BigIntPrimitive *ptr,
      PointerBase *) {
    return SmallHermesValueAdaptor{HermesValue::encodeBigIntValue(ptr)};
  }
  static SmallHermesValueAdaptor
  encodeNumberValue(double d, GC &, PointerBase &) {
    return SmallHermesValueAdaptor{HermesValue::encodeNumberValue(d)};
  }
  static SmallHermesValueAdaptor encodeNumberValue(double d, Runtime &) {
    return SmallHermesValueAdaptor{HermesValue::encodeNumberValue(d)};
  }
  static SmallHermesValueAdaptor encodeObjectValue(GCCell *ptr, PointerBase &) {
    return SmallHermesValueAdaptor{HermesValue::encodeObjectValue(ptr)};
  }
  static SmallHermesValueAdaptor encodeObjectValue(CompressedPointer cp) {
    GCCell *cellPtr = reinterpret_cast<GCCell *>(cp.getRaw());
    return SmallHermesValueAdaptor{HermesValue::encodeObjectValue(cellPtr)};
  }
  static SmallHermesValueAdaptor encodeStringValue(
      StringPrimitive *ptr,
      PointerBase &) {
    return SmallHermesValueAdaptor{HermesValue::encodeStringValue(ptr)};
  }
  static SmallHermesValueAdaptor encodeSymbolValue(SymbolID s) {
    return SmallHermesValueAdaptor{HermesValue::encodeSymbolValue(s)};
  }
  static constexpr SmallHermesValueAdaptor encodeBoolValue(bool b) {
    return SmallHermesValueAdaptor{HermesValue::encodeBoolValue(b)};
  }

  static constexpr SmallHermesValueAdaptor encodeNullValue() {
    return SmallHermesValueAdaptor{HermesValue::encodeNullValue()};
  }
  static constexpr SmallHermesValueAdaptor encodeUndefinedValue() {
    return SmallHermesValueAdaptor{HermesValue::encodeUndefinedValue()};
  }
  static constexpr SmallHermesValueAdaptor encodeEmptyValue() {
    return SmallHermesValueAdaptor{HermesValue::encodeEmptyValue()};
  }
};

/// A compressed HermesValue that is always equal to the size of a
/// CompressedPointer. It uses the least significant bits (guaranteed to be zero
/// by the heap alignment) as a tag to determine what the type of the remaining
/// bits is. Some types may use an additional tag bit to form an "extended tag".
/// Doubles have to be boxed on the heap so that they can then be held as
/// pointers. Native values are not supported.
class HermesValue32 {
  using RawType = CompressedPointer::RawType;
  using SmiType = std::make_signed<RawType>::type;

  static constexpr size_t kNumTagBits = LogHeapAlign;
  static constexpr size_t kNumValueBits = 8 * sizeof(RawType) - kNumTagBits;
  /// To match the behaviour of a double, SMI precision cannot exceed 53 bits +
  /// a sign bit. This is only relevant when compressed pointers are allowed but
  /// turned off on a 64-bit platform (e.g. with MallocGC) since we would end up
  /// using HermesValue32, but RawType would be 64 bits.
  static constexpr size_t kNumSmiBits =
      std::min(kNumValueBits, static_cast<size_t>(54));

  /// A 3 bit tag describing the stored value. Tags that represent multiple
  /// types are distinguished using an additional bit found in the "ETag".
  enum class Tag : uint8_t {
    Object,
    BigInt,
    String,
    BoxedDouble,
    SmallInt,
    Symbol,
    BoolAndUndefined,
    EmptyAndNull,
    _Last
  };

  static_assert(
      static_cast<uint8_t>(Tag::_Last) <= (1 << kNumTagBits),
      "Cannot have more enum values than tag bits.");

  static constexpr uint8_t kLastPointerTag =
      static_cast<uint8_t>(Tag::BoxedDouble);
  static constexpr uint8_t kFirstExtendedTag =
      static_cast<uint8_t>(Tag::BoolAndUndefined);

  static constexpr size_t kNumETagBits = kNumTagBits + 1;
  static constexpr size_t kNumETagValueBits = kNumValueBits - 1;

  /// Define an "extended tag", occupying one extra bit. For types that use all
  /// kNumValueBits bits, duplicate the enum value for both possible values of
  /// the extra bit.
  static constexpr uint8_t kETagOffset = 1 << kNumTagBits;
  enum class ETag : uint8_t {
    Object1 = static_cast<uint8_t>(Tag::Object),
    Object2 = static_cast<uint8_t>(Tag::Object) + kETagOffset,
    BigInt1 = static_cast<uint8_t>(Tag::BigInt),
    BigInt2 = static_cast<uint8_t>(Tag::BigInt) + kETagOffset,
    String1 = static_cast<uint8_t>(Tag::String),
    String2 = static_cast<uint8_t>(Tag::String) + kETagOffset,
    BoxedDouble1 = static_cast<uint8_t>(Tag::BoxedDouble),
    BoxedDouble2 = static_cast<uint8_t>(Tag::BoxedDouble) + kETagOffset,
    SmallInt1 = static_cast<uint8_t>(Tag::SmallInt),
    SmallInt2 = static_cast<uint8_t>(Tag::SmallInt) + kETagOffset,
    Symbol1 = static_cast<uint8_t>(Tag::Symbol),
    Symbol2 = static_cast<uint8_t>(Tag::Symbol) + kETagOffset,
    Bool = static_cast<uint8_t>(Tag::BoolAndUndefined),
    Undefined = static_cast<uint8_t>(Tag::BoolAndUndefined) + kETagOffset,
    Empty = static_cast<uint8_t>(Tag::EmptyAndNull),
    Null = static_cast<uint8_t>(Tag::EmptyAndNull) + kETagOffset,
  };

  RawType raw_;

  static constexpr HermesValue32 fromRaw(RawType raw) {
    return HermesValue32(raw);
  }
  /// Helper function to encode a non-pointer value with a given tag.
  static constexpr HermesValue32 fromTagAndValue(Tag tag, RawType value) {
    // Only a small integer can have its top bits set (if it's negative).
    assert(
        (llvh::isUInt<kNumValueBits>(value) || tag == Tag::SmallInt) &&
        "Value out of range.");
    return fromRaw((value << kNumTagBits) | static_cast<uint8_t>(tag));
  }
  static constexpr HermesValue32 fromETagAndValue(ETag etag, RawType value) {
    assert(
        llvh::isUInt<kNumETagValueBits>(value) &&
        "Value must fit in value bits.");
    assert(
        static_cast<uint8_t>(etag) % kETagOffset >= kFirstExtendedTag &&
        "Not an extended type.");
    return fromRaw((value << kNumETagBits) | static_cast<uint8_t>(etag));
  }

  RawType getValue() const {
    assert(getTag() != Tag::SmallInt && "SMIs must use getSmallInt.");
    assert(
        static_cast<uint8_t>(getTag()) < kFirstExtendedTag &&
        "Values for ETags should use getETagValue.");
    return raw_ >> kNumTagBits;
  }
  RawType getETagValue() const {
    assert(
        static_cast<uint8_t>(getTag()) >= kFirstExtendedTag &&
        "Not an extended type.");
    return raw_ >> kNumETagBits;
  }
  SmiType getSmallInt() const {
    assert(getTag() == Tag::SmallInt && "Must be a SMI.");
    return static_cast<SmiType>(raw_) >> kNumTagBits;
  }
  Tag getTag() const {
    return static_cast<Tag>(
        raw_ & llvh::maskTrailingOnes<RawType>(kNumTagBits));
  }
  ETag getETag() const {
    return static_cast<ETag>(
        raw_ & llvh::maskTrailingOnes<RawType>(kNumETagBits));
  }

  /// Assert that the pointer can be encoded.
  static void validatePointer(RawType rawPtr) {
    assert(llvh::countTrailingZeros(rawPtr) >= 3 && "Pointer low bits are set");
  }

  constexpr explicit HermesValue32(RawType raw) : raw_(raw) {}

  static HermesValue32
  encodePointerImpl(GCCell *ptr, Tag tag, PointerBase &pb) {
    return encodePointerImpl(CompressedPointer::encodeNonNull(ptr, pb), tag);
  }

  static HermesValue32 encodePointerImpl(CompressedPointer ptr, Tag tag) {
    RawType p = ptr.getRaw();
    validatePointer(p);
    return fromRaw(p | static_cast<RawType>(tag));
  }

  static SmiType truncateDouble(double d)
      LLVM_NO_SANITIZE("float-cast-overflow") {
    return d;
  }

  /// Truncate \p d to an integer that fits in kNumSmiBits.
  static SmiType doubleToSmi(double d) {
    // Use a generic lambda here so the inactive case of the if constexpr does
    // not need to compile.
    return [](auto d) {
      if constexpr (kNumSmiBits <= 32)
        return llvh::SignExtend32<kNumSmiBits>(truncateDouble(d));
      else
        return llvh::SignExtend64<kNumSmiBits>(truncateDouble(d));
    }(d);
  }

 public:
  HermesValue32() = default;
  HermesValue32(const HermesValue32 &) = default;

#ifdef _MSC_VER
  // This is a workaround to ensure is_trivial is true in MSVC.
  HermesValue32(HermesValue32 &&) = default;
  HermesValue32 &operator=(const HermesValue32 &hv) = default;
#else
  HermesValue32 &operator=(const HermesValue32 &hv) = delete;
#endif

  bool isPointer() const {
    return static_cast<uint8_t>(getTag()) <= kLastPointerTag;
  }
  bool isObject() const {
    return getTag() == Tag::Object;
  }
  bool isBigInt() const {
    return getTag() == Tag::BigInt;
  }
  bool isString() const {
    return getTag() == Tag::String;
  }
  bool isNumber() const {
    Tag tag = getTag();
    return tag == Tag::BoxedDouble || tag == Tag::SmallInt;
  }
  bool isSymbol() const {
    return getTag() == Tag::Symbol;
  }
  bool isUndefined() const {
    return raw_ == encodeUndefinedValue().raw_;
  }
  bool isEmpty() const {
    return raw_ == encodeEmptyValue().raw_;
  }
  bool isNull() const {
    return raw_ == encodeNullValue().raw_;
  }
  bool isBool() const {
    return getETag() == ETag::Bool;
  }

  /// Convert this to a full HermesValue, but do not unbox a BoxedDouble.
  /// This is only intended for diagnostics or for code reuse in the GC.
  inline HermesValue toHV(PointerBase &pb) const;

  /// Convert this to a full HermesValue, and unbox it if it is currently boxed.
  /// This is more commonly useful, and is essentially the reverse process of
  /// encodeHermesValue.
  inline HermesValue unboxToHV(PointerBase &pb) const;

  /// Methods to access pointer values.
  GCCell *getPointer(PointerBase &pb) const {
    assert(isPointer());
    return getPointer().getNonNull(pb);
  }
  GCCell *getObject(PointerBase &pb) const {
    assert(isObject());
    // Since object pointers are the most common type, we have them as the
    // zero-tag and can decode them without needing to remove the tag.
    static_assert(
        static_cast<uint8_t>(Tag::Object) == 0,
        "Object tag must be zero for fast path.");
    return CompressedPointer::fromRaw(raw_).get(pb);
  }

  inline BigIntPrimitive *getBigInt(PointerBase &pb) const;
  inline StringPrimitive *getString(PointerBase &pb) const;
  inline double getNumber(PointerBase &pb) const;

  CompressedPointer getPointer() const {
    assert(isPointer());
    RawType rawPtr = raw_ & llvh::maskLeadingOnes<RawType>(kNumValueBits);
    return CompressedPointer::fromRaw(rawPtr);
  }

  SymbolID getSymbol() const {
    assert(isSymbol());
    return SymbolID::unsafeCreate(getValue());
  }
  bool getBool() const {
    assert(isBool());
    return getETagValue();
  }

  inline void setInGC(HermesValue32 hv, GC &gc);

  HermesValue32 updatePointer(GCCell *ptr, PointerBase &pb) const {
    return encodePointerImpl(ptr, getTag(), pb);
  }
  void unsafeUpdatePointer(GCCell *ptr, PointerBase &pb) {
    setNoBarrier(encodePointerImpl(ptr, getTag(), pb));
  }
  HermesValue32 updatePointer(CompressedPointer ptr) const {
    assert(isPointer());
    return encodePointerImpl(ptr, getTag());
  }

  /// Convert a normal HermesValue to a HermesValue32. If \p hv is a pointer,
  /// this function is guaranteed to not allocate. This is important since an
  /// allocation may invalidate the pointer. For non-pointer values, always
  /// treat this function as though it may allocate.
  inline static HermesValue32 encodeHermesValue(
      HermesValue hv,
      Runtime &runtime);

  /// Encode a double as a HermesValue32. Small integer values will be stored
  /// inline and doubles will be allocated on the heap. Always treat this
  /// function as though it may allocate.
  inline static HermesValue32 encodeNumberValue(double d, Runtime &runtime);

  inline static HermesValue32 encodeObjectValue(GCCell *ptr, PointerBase &pb);
  static HermesValue32 encodeObjectValue(CompressedPointer cp) {
    return encodePointerImpl(cp, Tag::Object);
  }

  static HermesValue32 encodeBigIntValue(
      BigIntPrimitive *ptr,
      PointerBase &pb) {
    return encodePointerImpl(reinterpret_cast<GCCell *>(ptr), Tag::BigInt, pb);
  }

  static HermesValue32 encodeStringValue(
      StringPrimitive *ptr,
      PointerBase &pb) {
    return encodePointerImpl(reinterpret_cast<GCCell *>(ptr), Tag::String, pb);
  }

  static HermesValue32 encodeSymbolValue(SymbolID s) {
    return fromTagAndValue(Tag::Symbol, s.unsafeGetRaw());
  }
  static constexpr HermesValue32 encodeBoolValue(bool b) {
    return fromETagAndValue(ETag::Bool, b);
  }
  static constexpr HermesValue32 encodeNullValue() {
    return fromETagAndValue(ETag::Null, 0);
  }
  static constexpr HermesValue32 encodeUndefinedValue() {
    return fromETagAndValue(ETag::Undefined, 0);
  }
  static constexpr HermesValue32 encodeEmptyValue() {
    return fromETagAndValue(ETag::Empty, 0);
  }

 protected:
  /// Performs an assignment without a barrier, in cases where the RHS
  /// value may contain an object pointer.  WARNING: this is very
  /// dangerous, and should be avoided whenever possible.  Exposed here
  /// for use by subtypes.
  inline void setNoBarrier(HermesValue32 other) {
    raw_ = other.raw_;
  }
};

/// If compressed pointers are allowed, then we should also compress
/// HermesValues. This means that when compressed pointers are allowed,
/// SmallHermesValue will almost always be 32 bits, except with MallocGC, which
/// does not support compressed pointers.
using SmallHermesValue =
#ifdef HERMESVM_ALLOW_COMPRESSED_POINTERS
    HermesValue32
#else
    SmallHermesValueAdaptor
#endif
    ;

static_assert(
    std::is_trivial<SmallHermesValue>::value,
    "SmallHermesValue must be trivial");

using GCSmallHermesValue = GCHermesValueBase<SmallHermesValue>;

} // end namespace vm
} // end namespace hermes

#pragma GCC diagnostic pop
#endif // HERMES_VM_HERMESVALUE_H
