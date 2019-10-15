/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// This header defines the "NaN-boxing" encoding format used to represent
/// values in Hermes.
///
/// NaN-boxing relies on the representation of doubles in IEEE-754:
///
/// \pre
/// s   eeeeeee,eeee mmmm,mmmmmmmm,mmmmmmmm,mmmmmmmm,mmmmmmmm,mmmmmmmm,mmmmmmmm
/// |   |            |
/// 63  62           51                                                       0
///
/// s: 1-bit sign
/// e: 11-bit exponent
/// m: 52-bit mantissa
/// \endpre
///
/// An exponent of all 1-s (0x7ff) and a non-zero mantissa is used to encode
/// NaN. So, as long as the top 12 bits are 0x7ff or 0xfff and the bottom 52
/// bits are not 0, we can store any bit patterm in the bottom bits and it will
/// be interpreted as NaN.
///
/// This is what the "canonical quiet NaN" looks like:
/// \pre
/// 0   1111111,1111 1000,00000000,00000000,00000000,00000000,00000000,00000000
/// |   |            |
/// 63  62           51                                                       0
/// \endpre
///
/// Note that the sign bit can have any value.
/// We have chosen to set the sign bit as 1 and encode out 4-bit type tag in
/// bits 50-47. The type tag cannot be zero because it's reserved for the
/// "canonical quiet NaN". Thus our type tags range between:
/// \pre
/// 1   1111111,1111 1111,1xxxxxxx,xxxxxxxx,xxxxxxxx,xxxxxxxx,xxxxxxxx,xxxxxxxx
///   and
/// 1   1111111,1111 1000,1xxxxxxx,xxxxxxxx,xxxxxxxx,xxxxxxxx,xxxxxxxx,xxxxxxxx
/// \endpre
///
/// Masking only the top 17 bits, our tag range is [0xffff8 .. 0xfff88].
/// Anything lower than 0xfff88 represents a real number,
/// specifically 0xfff80 covers the "canonical quiet NaN".
///
/// On 32-bit platforms clearly we have enough bits to encode any pointer in
/// the low word.
///
/// On 64-bit platforms, we could always arrange our own heap to fit within the
/// available 47-bits. In practice however that is not necessary (yet) because
/// current 64-bit platforms use at most than 48 bits of virtual address
/// space (Linux x86-64 uses 48, Linux ARM64 uses 40). Additionally, the highest
/// representative bit (bit 47 or 39) is "sign-extended" to the higher bits.
/// Negative values are used for kernel addresses (top bit 1).
///
/// Since in our case we never need to represent a kernel address - all
/// addresses are within our own heap - we know that the top bit is 0 and we
/// don't need to store it leaving us with exactly 47-bits.
///
/// Should the OS requirements change in the distant future, we can "squeeze" 3
/// more bits by relying on the 8-byte alignment of all our allocations and
/// shifting the values to the right. That is still not needed however.

#ifndef HERMES_VM_HERMESVALUE_H
#define HERMES_VM_HERMESVALUE_H

#include "hermes/Support/Conversions.h"
#include "hermes/VM/GCDecl.h"
#include "hermes/VM/SymbolID.h"

#include "llvm/Support/raw_ostream.h"

#include <cassert>
#include <cmath>
#include <cstdint>

namespace llvm {
class raw_ostream;
}

namespace hermes {
namespace vm {

class StringPrimitive;

// Tags are defined as 17-bit values positioned at the high bits of a 64-bit
// word.

using TagKind = uint32_t;

/// If tag < FirstTag, the encoded value is a double.
static constexpr TagKind FirstTag = 0xfff88000 >> 15;
static constexpr TagKind LastTag = 0xffff8000 >> 15;

static constexpr TagKind EmptyTag = FirstTag + 0;
static constexpr TagKind UndefinedTag = FirstTag + 1;
static constexpr TagKind NullTag = FirstTag + 2;
static constexpr TagKind BoolTag = FirstTag + 3;
static constexpr TagKind NativeValueTag = FirstTag + 4;
static constexpr TagKind SymbolTag = FirstTag + 5;

#ifdef HERMES_SLOW_DEBUG
/// An invalid hermes value is one that should never exist in normal operation,
/// it can be used as a sigil to indicate a programming failure.
static constexpr TagKind InvalidTag = FirstTag + 6;
#endif

// Pointer tags start here.
static constexpr TagKind StrTag = FirstTag + 13;
static constexpr TagKind ObjectTag = FirstTag + 14;
static_assert(ObjectTag == LastTag, "Mis-configured tags");

/// Only values in the range [FirstPointerTag..LastTag] are pointers.
static constexpr TagKind FirstPointerTag = StrTag;

/// A NaN-box encoded value.
class HermesValue {
 public:
  /// Number of bits used in the high part to encode the sign, exponent and tag.
  static constexpr unsigned kNumTagExpBits = 17;
  /// Number of bits available for data storage.
  static constexpr unsigned kNumDataBits = (64 - kNumTagExpBits);

  /// Width of a tag in bits. The tag is aligned to the right of the top bits.
  static constexpr unsigned kTagWidth = 5;
  static constexpr unsigned kTagMask = (1 << kTagWidth) - 1;
  /// Mask to extract the data from the whole 64-bit word.
  static constexpr uint64_t kDataMask = (1ull << kNumDataBits) - 1;

  /// Assert that the pointer can be encoded in \c kNumDataBits.
  static void validatePointer(const void *ptr) {
#if LLVM_PTR_SIZE == 8
    assert(
        (safeTypeCast<const void *, uint64_t>(ptr) & ~kDataMask) == 0 &&
        "Pointer top bits are set");
#endif
  }

  using RawType = uint64_t;

  HermesValue() = default;

  constexpr inline static HermesValue fromRaw(RawType raw) {
    return HermesValue(raw);
  }

  /// Dump the contents to stderr.
  void dump(llvm::raw_ostream &stream = llvm::errs()) const;

  inline TagKind getTag() const {
    return (TagKind)(raw_ >> kNumDataBits);
  }

  /// Combine two tags into a 10-bit value.
  inline static constexpr unsigned combineTags(TagKind a, TagKind b) {
    return ((a & kTagMask) << kTagWidth) | (b & kTagMask);
  }

  constexpr inline static HermesValue encodeNullptrObjectValue() {
    return HermesValue(0, ObjectTag);
  }
  inline static HermesValue encodeObjectValue(void *val) {
    validatePointer(val);
    HermesValue RV(safeTypeCast<void *, uintptr_t>(val), ObjectTag);
    assert(RV.isObject());
    return RV;
  }

  inline static HermesValue encodeStringValue(const StringPrimitive *val) {
    validatePointer(val);
    HermesValue RV(safeTypeCast<const void *, uintptr_t>(val), StrTag);
    assert(RV.isString());
    return RV;
  }

  inline static HermesValue encodeNativeValue(int64_t val) {
    HermesValue RV(((uint64_t)val) & kDataMask, NativeValueTag);
    assert(
        RV.isNativeValue() && RV.getNativeValue() == val &&
        "native value doesn't fit");
    return RV;
  }

  inline static HermesValue encodeNativeUInt32(uint32_t val) {
    HermesValue RV(val, NativeValueTag);
    assert(
        RV.isNativeValue() && RV.getNativeUInt32() == val &&
        "native value doesn't fit");
    return RV;
  }

  inline static HermesValue encodeNativePointer(const void *p) {
    assert(
        (reinterpret_cast<uintptr_t>(p) & ~kDataMask) == 0 &&
        "Native pointer must contain zeroes in the high bits");
    HermesValue RV(reinterpret_cast<uintptr_t>(p), NativeValueTag);
    assert(
        RV.isNativeValue() && RV.getNativePointer<void>() == p &&
        "Native pointer doesn't fit");
    return RV;
  }

  inline static HermesValue encodeSymbolValue(SymbolID val) {
    HermesValue RV(val.unsafeGetRaw(), SymbolTag);
    assert(RV.isSymbol());
    return RV;
  }

  constexpr inline static HermesValue encodeBoolValue(bool val) {
    return HermesValue((uint64_t)(val), BoolTag);
  }

  inline static constexpr HermesValue encodeNullValue() {
    return HermesValue(0, NullTag);
  }

  inline static constexpr HermesValue encodeUndefinedValue() {
    return HermesValue(0, UndefinedTag);
  }

  inline static constexpr HermesValue encodeEmptyValue() {
    return HermesValue(0, EmptyTag);
  }

#ifdef HERMES_SLOW_DEBUG
  inline static constexpr HermesValue encodeInvalidValue() {
    return HermesValue(0, InvalidTag);
  }
#endif

  inline static HermesValue encodeDoubleValue(double num) {
    HermesValue RV(safeTypeCast<double, uint64_t>(num));
    assert(RV.isDouble());
    return RV;
  }

  /// Encode a double value, which it cannot be guaranteed that the NaN bits are
  /// all zeroes.
  inline static HermesValue encodeUntrustedDoubleValue(double num) {
    return std::isnan(num) ? encodeNaNValue() : encodeDoubleValue(num);
  }

  /// Encode a numeric value into the best possible representation based on the
  /// static type of the parameter. Right now we only have one representation
  /// (double), but that could change in the future.
  inline static HermesValue encodeNumberValue(double num) {
    return encodeDoubleValue(num);
  }

  /// Encode a numeric value into the best possible representation based on the
  /// static type of the parameter. Right now we only have one representation
  /// (double), but that could change in the future.
  template <typename T>
  inline static
      typename std::enable_if<std::is_integral<T>::value, HermesValue>::type
      encodeNumberValue(T num) {
    assert((double)num == num && "value not representable as double");
    return encodeDoubleValue((double)num);
  }

  static constexpr HermesValue encodeNaNValue() {
    return HermesValue(safeTypeCast<double, uint64_t>(
        std::numeric_limits<double>::quiet_NaN()));
  }

  /// Keeping tag constant, make a new HermesValue with \p val stored in it.
  inline HermesValue updatePointer(void *val) const {
    assert(isPointer());
    HermesValue V(safeTypeCast<void *, uintptr_t>(val), getTag());
    assert(V.isPointer());
    return V;
  }

  /// Update a HermesValue to encode \p ptr. Used by (de)seserializer.
  /// We need to have this function instead of using \p updatePointer because
  /// we also want to be able to update \p NativePointer HermesValue, which
  /// will fail the \p isPointer check in \p updatePointer.
  /// \param ptr pointer value to encode.
  inline void unsafeUpdatePointer(void *ptr) {
    raw_ = (uint64_t)(safeTypeCast<void *, uintptr_t>(ptr)) |
        (uint64_t)getTag() << kNumDataBits;
  }

  inline bool isNull() const {
    return getTag() == NullTag;
  }
  inline bool isUndefined() const {
    return getTag() == UndefinedTag;
  }
  inline bool isEmpty() const {
    return getTag() == EmptyTag;
  }
#ifdef HERMES_SLOW_DEBUG
  inline bool isInvalid() const {
    return getTag() == InvalidTag;
  }
#endif
  inline bool isNativeValue() const {
    return getTag() == NativeValueTag;
  }
  inline bool isSymbol() const {
    return getTag() == SymbolTag;
  }
  inline bool isBool() const {
    return getTag() == BoolTag;
  }
  inline bool isObject() const {
    return getTag() == ObjectTag;
  }
  inline bool isString() const {
    return getTag() == StrTag;
  }
  inline bool isDouble() const {
    return raw_ < ((uint64_t)FirstTag << kNumDataBits);
  }
  inline bool isPointer() const {
    return raw_ >= ((uint64_t)FirstPointerTag << kNumDataBits);
  }
  inline bool isNumber() const {
    return isDouble();
  }

  inline RawType getRaw() const {
    return raw_;
  }

  inline void *getPointer() const {
    assert(isPointer());
    // Mask out the tag.
    return safeSizeTrunc<uint64_t, void *>(raw_ & kDataMask);
  }

  inline double getDouble() const {
    assert(isDouble());
    return safeTypeCast<uint64_t, double>(raw_);
  }

  inline int64_t getNativeValue() const {
    assert(isNativeValue());
    // Sign-extend the value.
    return (static_cast<int64_t>(raw_ & kDataMask) << (64 - kNumDataBits)) >>
        (64 - kNumDataBits);
  }

  inline uint32_t getNativeUInt32() const {
    assert(isNativeValue());
    return (uint32_t)raw_;
  }

  template <class T>
  inline T *getNativePointer() const {
    assert(isNativeValue());
    // Zero-extend the value because we know that bit 47 must be 0 (or otherwise
    // it would be a kernel address).
    return reinterpret_cast<T *>(raw_ & kDataMask);
  }

  inline SymbolID getSymbol() const {
    assert(isSymbol());
    return SymbolID::unsafeCreate((uint32_t)raw_);
  }

  inline bool getBool() const {
    assert(isBool());
    return (bool)(raw_ & 0x1);
  }

  inline StringPrimitive *getString() const {
    assert(isString());
    return static_cast<StringPrimitive *>(getPointer());
  }

  inline void *getObject() const {
    assert(isObject());
    return getPointer();
  }

  inline double getNumber() const {
    return getDouble();
  }

  template <class T>
  inline typename std::enable_if<std::is_integral<T>::value, T>::type
  getNumberAs() const {
    double num = getDouble();
    assert(
        num >= std::numeric_limits<T>::lowest() &&
        num <= std::numeric_limits<T>::max() && (T)num == num &&
        "value not representable as type");
    return num;
  }

  template <class T>
  inline typename std::enable_if<!std::is_integral<T>::value, T>::type
  getNumberAs() const {
    return getDouble();
  }

  /// This form performs assignments of pointer values without
  /// write barriers, to be used only within GC. The GC argument is
  /// used to assert that this is used only within GC.
  inline void setInGC(HermesValue hv, GC *gc);

  /// We delete the assignment operator: HermesValues should either
  /// PinnedHermesValues, to indicate that they can be assigned, or
  /// GCHermesValues, to enforce that assignments of pointer values
  /// require a write barrier.
  /// (except in MSVC: this is to work around lack of CWG 1734.
  /// HermesValue will not be considered trivial otherwise.)
  /// TODO(T40821815) Consider removing this workaround when updating MSVC
#ifndef _MSC_VER
  void operator=(const HermesValue &hv) = delete;
#endif

 protected:
  /// Performs an assignment without a barrier, in cases where the RHS
  /// value may contain an object pointer.  WARNING: this is very
  /// dangerous, and should be avoided whenever possible.  Exposed here
  /// for use by subtypes, such as PinnedHermesValue, where we're
  /// sure that \p this is not an address in the heap, is treated as a root
  /// location.
  void setNoBarrier(HermesValue hv) {
    raw_ = hv.raw_;
  }

 private:
  constexpr explicit HermesValue(uint64_t val) : raw_(val) {}
  constexpr explicit HermesValue(uint64_t val, TagKind tag)
      : raw_(val | ((uint64_t)tag << kNumDataBits)) {}

  // 64 raw bits stored and reinterpreted as necessary.
  uint64_t raw_;

}; // class HermesValue

static_assert(
    std::is_trivial<HermesValue>::value,
    "HermesValue must be trivial");

/// A HermesValue which is stored in non-moveable memory and is known to the
/// garbage collector.
class PinnedHermesValue : public HermesValue {
 public:
  constexpr PinnedHermesValue()
      : HermesValue(HermesValue::encodeUndefinedValue()) {}
  constexpr PinnedHermesValue(HermesValue v) : HermesValue(v) {}
  constexpr PinnedHermesValue(const PinnedHermesValue &) = default;
  PinnedHermesValue &operator=(const PinnedHermesValue &phv) {
    setNoBarrier(phv);
    return *this;
  }
  PinnedHermesValue &operator=(const HermesValue &hv) {
    setNoBarrier(hv);
    return *this;
  }
} HERMES_ATTRIBUTE_WARN_UNUSED_VARIABLES;

// All HermesValues stored in heap object should be of this
// type. Hides assignment operator, but provides set operations that
// do a write barrier for pointer values, or else assert that the new
// value is not a pointer.
class GCHermesValue : public HermesValue {
 public:
  GCHermesValue() : HermesValue(HermesValue::encodeUndefinedValue()) {}
  GCHermesValue(const GCHermesValue &) = delete;

  /// The HermesValue \p hv may be an object pointer.  Assign the
  /// value, and perform any necessary write barriers.
  template <typename NeedsBarriers = std::true_type>
  inline void set(HermesValue hv, GC *gc);

  /// The HermesValue \p hv must not be an object pointer.  Assign the
  /// value.
  inline void setNonPtr(HermesValue hv);

  /// Fills a region of GCHermesValues defined by [\p from, \p to) with the
  /// value \p fill.  If the fill value is an object pointer, must
  /// provide a non-null \p gc argument, to perform write barriers.
  template <typename InputIt>
  static inline void
  fill(InputIt first, InputIt last, HermesValue fill, GC *gc = nullptr);

  /// Copies a range of values and performs a write barrier on each.
  template <typename InputIt, typename OutputIt>
  static inline OutputIt
  copy(InputIt first, InputIt last, OutputIt result, GC *gc);

  /// Copies a range of values and performs a write barrier on each.
  template <typename InputIt, typename OutputIt>
  static inline OutputIt
  copy_backward(InputIt first, InputIt last, OutputIt result, GC *gc);
};

llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, HermesValue hv);

/// SafeNumericEncoder can encode any numeric type into a numeric HermesValue,
/// as well as safely encoding any possible untrusted floating-point values.
template <typename T, bool b = std::is_floating_point<T>::value>
struct SafeNumericEncoder {
  static HermesValue encode(T val) {
    return HermesValue::encodeNumberValue(val);
  }
};

template <typename T>
struct SafeNumericEncoder<T, true> {
  static HermesValue encode(T val) {
    return HermesValue::encodeUntrustedDoubleValue(val);
  }
};

} // end namespace vm
} // end namespace hermes

#endif // HERMES_VM_HERMESVALUE_H
