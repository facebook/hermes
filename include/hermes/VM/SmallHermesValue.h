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
#include "hermes/VM/SymbolID.h"

#include <cassert>
#include <cmath>
#include <cstdint>

namespace hermes {
namespace vm {

class BigIntPrimitive;
class StringPrimitive;
class GCCell;
class Runtime;

/// SmallHermesValue is the HermesValue encoding used on the heap when we want
/// to potentially benefit from a 32-bit representation (depending on native
/// pointer size and HERMESVM_COMPRESSED_POINTERS). A 32-bit representation
/// inherently requires boxing doubles, since they can't fit in 32 bits.
///
/// Additionally, for testing, we can force boxing of doubles even in a 64-bit
/// representation.

#ifndef HERMESVM_BOXED_DOUBLES
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
  using HermesValue::getRaw;
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

  /// \return true if it's a Number since double bits are always inlined in
  /// HermesValue.
  bool isInlinedDouble() const {
    return isNumber();
  }
  /// \return false since HermesValue does not box doubles.
  bool isBoxedDouble() const {
    return false;
  }
  /// This should never be executed since isBoxedDouble() always return false.
  /// Its existence is only to have the same method as HermesValue32 to satisfy
  /// compiler.
  double getBoxedDouble(PointerBase &) const {
    llvm_unreachable("SmallHermesValueAdaptor does not have boxed doubles.");
  }

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
  static SmallHermesValueAdaptor encodeNumberValue(double d, Runtime &) {
    return SmallHermesValueAdaptor{HermesValue::encodeTrustedNumberValue(d)};
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
using SmallHermesValue = SmallHermesValueAdaptor;

#else // #ifndef HERMESVM_BOXED_DOUBLES

/// A compressed HermesValue that is always equal to the size of a
/// CompressedPointer. It uses the least significant bits (guaranteed to be zero
/// by the heap alignment) as a tag to determine what the type of the remaining
/// bits is. Some types may use an additional tag bit to form an "extended tag".
/// Doubles have to be boxed on the heap so that they can then be held as
/// pointers. Native values are not supported.
class HermesValue32 {
  using RawType = CompressedPointer::RawType;
  using SmiType = std::make_signed<RawType>::type;

  RawType raw_;

 public:
  /// Version of the HermesValue32 encoding format.
  /// Changing the format of HermesValue32 requires bumping this version number
  /// and fixing any code that relies on the layout of HermesValue32.
  /// Updated: Mar 11, 2025
  static constexpr size_t kVersion = 1;
  static constexpr size_t kNumRawTypeBits = sizeof(RawType) * 8;
  static constexpr size_t kNumTagBits = LogHeapAlign;
  static constexpr size_t kNumValueBits = kNumRawTypeBits - kNumTagBits;

  /// A 3 bit tag describing the stored value. Tags that represent multiple
  /// types are distinguished using an additional bit found in the "ETag".
  enum class Tag : uint8_t {
    CompressedHV64,
    String,
    BigInt,
    Object,
    BoxedDouble,
    Symbol,
    _Last,

    FirstPointer = String,
    LastPointer = BoxedDouble,
  };

 private:
  static_assert(
      static_cast<uint8_t>(Tag::_Last) <= (1 << kNumTagBits),
      "Cannot have more enum values than tag bits.");

  static constexpr HermesValue32 fromRaw(RawType raw) {
    return HermesValue32(raw);
  }
  /// Helper function to encode a non-pointer value with a given tag.
  static constexpr HermesValue32 fromTagAndValue(Tag tag, RawType value) {
    // The value will have been right-shifted by the tag width, and should
    // have zeroes in the top bits.
    assert(llvh::isUInt<kNumValueBits>(value) && "Value out of range.");
    return fromRaw((value << kNumTagBits) | static_cast<uint8_t>(tag));
  }

  RawType getValue() const {
    return raw_ >> kNumTagBits;
  }

  double getCompressedDouble() const {
    assert(isInlinedDouble() && "Must be a compressed double.");
    return llvh::BitsToDouble(compressedHV64ToBits());
  }

  Tag getTag() const {
    return static_cast<Tag>(
        raw_ & llvh::maskTrailingOnes<RawType>(kNumTagBits));
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

  uint64_t compressedHV64ToBits() const {
    static_assert((uint64_t)Tag::CompressedHV64 == 0, "Must have zero tag");
    assert(getTag() == Tag::CompressedHV64 && "Must be a compressed HV64");
    // The tag is guaranteed to be 0, so we can just shift to decompress.
    return (uint64_t)raw_ << (64 - kNumRawTypeBits);
  }

  static constexpr HermesValue32 bitsToCompressedHV64(uint64_t bits) {
    assert(
        (llvh::isShiftedUInt<kNumValueBits, 64 - kNumValueBits>(bits)) &&
        "Value out of range.");
    static_assert((uint64_t)Tag::CompressedHV64 == 0, "Must have zero tag");
    // The tag is guaranteed to be 0, so we can just shift to compress.
    return fromRaw(bits >> (64 - kNumRawTypeBits));
  }

  /// Encode a HermesValue that is known to either be compressible or a number
  /// as a HermesValue32. "Compressible" values will be stored inline, and
  /// non-compressible doubles will be allocated on the heap. Always treat this
  /// function as though it may allocate.
  inline static HermesValue32 encodeCompressibleOrNumberHV64(
      HermesValue hv,
      Runtime &runtime);

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
    auto tag = getTag();
    return tag >= Tag::FirstPointer && tag <= Tag::LastPointer;
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
    // It's likely to be a CompressedDouble, so check it first.
    return isInlinedDouble() || tag == Tag::BoxedDouble;
  }
  bool isInlinedDouble() const {
    return getTag() == Tag::CompressedHV64 &&
        HermesValue::fromRaw(compressedHV64ToBits()).isNumber();
  }
  bool isBoxedDouble() const {
    return getTag() == Tag::BoxedDouble;
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
    return raw_ == encodeBoolValue(false).raw_ ||
        raw_ == encodeBoolValue(true).raw_;
  }
  RawType getRaw() const {
    return raw_;
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
    return getPointer(pb);
  }

  inline BigIntPrimitive *getBigInt(PointerBase &pb) const;
  inline StringPrimitive *getString(PointerBase &pb) const;
  inline double getNumber(PointerBase &pb) const;
  inline double getBoxedDouble(PointerBase &pb) const;

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
    return HermesValue::fromRaw(compressedHV64ToBits()).getBool();
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
    return bitsToCompressedHV64(HermesValue::encodeBoolValue(b).getRaw());
  }
  static constexpr HermesValue32 encodeNullValue() {
    return bitsToCompressedHV64(HermesValue::encodeNullValue().getRaw());
  }
  static constexpr HermesValue32 encodeUndefinedValue() {
    return bitsToCompressedHV64(HermesValue::encodeUndefinedValue().getRaw());
  }
  static constexpr HermesValue32 encodeEmptyValue() {
    return bitsToCompressedHV64(HermesValue::encodeEmptyValue().getRaw());
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
using SmallHermesValue = HermesValue32;

#endif // #ifndef HERMESVM_BOXED_DOUBLES

static_assert(
    std::is_trivial<SmallHermesValue>::value,
    "SmallHermesValue must be trivial");

/// Base type for GC aware SmallHermesValues. This should only be used when we
/// don't need to handle large allocation specially.
using GCSmallHermesValueBase = GCHermesValueBaseImpl<SmallHermesValue>;

/// GCSmallHermesValue stored in a normal object.
using GCSmallHermesValue = GCSmallHermesValueBase;

/// GCSmallHermesValue stored in an object that supports large allocation.
using GCSmallHermesValueInLargeObj = GCSmallHermesValueBase;

} // end namespace vm
} // end namespace hermes

#endif // HERMES_VM_HERMESVALUE_H
