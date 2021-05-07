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
/// bits are not 0, we can store any bit pattern in the bottom bits and it will
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
/// We have chosen to set the sign bit as 1 and encode out 3-bit type tag in
/// bits 50-48. The type tag cannot be zero because it's reserved for the
/// "canonical quiet NaN". Thus our type tags range between:
/// \pre
/// 1   1111111,1111 1111,xxxxxxxx,xxxxxxxx,xxxxxxxx,xxxxxxxx,xxxxxxxx,xxxxxxxx
///   and
/// 1   1111111,1111 1001,xxxxxxxx,xxxxxxxx,xxxxxxxx,xxxxxxxx,xxxxxxxx,xxxxxxxx
/// \endpre
///
/// Masking only the top 16 bits, our tag range is [0xffff .. 0xfff9].
/// Anything lower than 0xfff8 represents a real number, specifically 0xfff8
/// covers the "canonical quiet NaN".
///
/// Extended Tags
/// =============
/// This leaves us with 48 data bits and only 7 tags. In some cases we don't
/// need the full 48 bits, allowing us to extend the width of the tag. We have
/// chosen to reserve the last 3 tags (0xfffd, 0xfffe, 0xffff) for full width
/// 48-bit data, and to extend the first 4 tags with one bit to the right.
/// We call the resulting 4-bit tag an "extended tag".
///
/// Pointer Encoding
/// ================
/// On 32-bit platforms clearly we have enough bits to encode any pointer in
/// the low word.
///
/// On 64-bit platforms, we could theoretically arrange our own heap to fit
/// within the available 48-bits. In practice however that is not necessary
/// (yet) because current 64-bit platforms use at most 48 bits of virtual
/// address space. x86-64 requires bit 47 to be sign extended into the top 16
/// bits (leaving effectively 47 address bits), while Linux ARM64 simply
/// requires the top N bits (depending on configuration, but at least 16) be all
/// 0 or all 1. In both cases negative values are used for kernel addresses
/// (top bit 1).
///
/// Since in our case we never need to represent a kernel address - all
/// addresses are within our own heap - we know that the top bit is 0 and we
/// don't need to store it leaving us with exactly 48-bits.
///
/// Should the OS requirements change in the distant future, we can "squeeze" 3
/// more bits by relying on the 8-byte alignment of all our allocations and
/// shifting the values to the right. That is still not needed however.

#ifndef HERMES_VM_HERMESVALUE_H
#define HERMES_VM_HERMESVALUE_H

#include "hermes/Support/Conversions.h"
#include "hermes/VM/GCDecl.h"
#include "hermes/VM/SymbolID.h"

#include "llvh/Support/raw_ostream.h"

#include <cassert>
#include <cmath>
#include <cstdint>

namespace llvh {
class raw_ostream;
}

namespace hermes {
namespace vm {

class StringPrimitive;
template <typename T>
class PseudoHandle;

template <typename T>
class PseudoHandle;

// Only used for HV32 API compatibility.
class PointerBase;
class GCCell;
class Runtime;

// Tags are defined as 16-bit values positioned at the high bits of a 64-bit
// word.

using TagKind = uint32_t;

/// If tag < FirstTag, the encoded value is a double.
static constexpr TagKind FirstTag = 0xfff9;
static constexpr TagKind LastTag = 0xffff;

static constexpr TagKind EmptyInvalidTag = FirstTag;
static constexpr TagKind UndefinedNullTag = FirstTag + 1;
static constexpr TagKind BoolTag = FirstTag + 2;
static constexpr TagKind SymbolTag = FirstTag + 3;

// Tags with 48-bit data start here.
static constexpr TagKind NativeValueTag = FirstTag + 4;

// Pointer tags start here.
static constexpr TagKind StrTag = FirstTag + 5;
static constexpr TagKind ObjectTag = FirstTag + 6;

static_assert(ObjectTag == LastTag, "Tags mismatch");

/// Only values in the range [FirstPointerTag..LastTag] are pointers.
static constexpr TagKind FirstPointerTag = StrTag;

/// A NaN-box encoded value.
class HermesValue {
 public:
  /// An "extended tag", occupying one extra bit.
  enum class ETag : uint32_t {
    Empty = EmptyInvalidTag * 2,
#ifdef HERMES_SLOW_DEBUG
    /// An invalid hermes value is one that should never exist in normal
    /// operation, it can be used as a sigil to indicate a programming failure.
    Invalid = EmptyInvalidTag * 2 + 1,
#endif
    Undefined = UndefinedNullTag * 2,
    Null = UndefinedNullTag * 2 + 1,
    Bool = BoolTag * 2,
    Symbol = SymbolTag * 2,
    Native1 = NativeValueTag * 2,
    Native2 = NativeValueTag * 2 + 1,
    Str1 = StrTag * 2,
    Str2 = StrTag * 2 + 1,
    Object1 = ObjectTag * 2,
    Object2 = ObjectTag * 2 + 1,

    FirstPointer = Str1,
  };

  /// Number of bits used in the high part to encode the sign, exponent and tag.
  static constexpr unsigned kNumTagExpBits = 16;
  /// Number of bits available for data storage.
  static constexpr unsigned kNumDataBits = (64 - kNumTagExpBits);

  /// Width of a tag in bits. The tag is aligned to the right of the top bits.
  static constexpr unsigned kTagWidth = 4;
  static constexpr unsigned kTagMask = (1 << kTagWidth) - 1;
  /// Mask to extract the data from the whole 64-bit word.
  static constexpr uint64_t kDataMask = (1ull << kNumDataBits) - 1;

  static constexpr unsigned kETagWidth = 5;
  static constexpr unsigned kETagMask = (1 << kTagWidth) - 1;

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
  HermesValue(const HermesValue &) = default;
#ifdef _MSC_VER
  // As below, this is a workaround to ensure is_trivial is true in MSVC.
  HermesValue(HermesValue &&) = default;
#endif

  constexpr inline static HermesValue fromRaw(RawType raw) {
    return HermesValue(raw);
  }

  /// Dump the contents to stderr.
  void dump(llvh::raw_ostream &stream = llvh::errs()) const;

  inline TagKind getTag() const {
    return (TagKind)(raw_ >> kNumDataBits);
  }
  inline ETag getETag() const {
    return (ETag)(raw_ >> (kNumDataBits - 1));
  }

  /// Combine two tags into an 8-bit value.
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
    return HermesValue(0, ETag::Null);
  }

  inline static constexpr HermesValue encodeUndefinedValue() {
    return HermesValue(0, ETag::Undefined);
  }

  inline static constexpr HermesValue encodeEmptyValue() {
    return HermesValue(0, ETag::Empty);
  }

#ifdef HERMES_SLOW_DEBUG
  inline static constexpr HermesValue encodeInvalidValue() {
    return HermesValue(0, ETag::Invalid);
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
    return getETag() == ETag::Null;
  }
  inline bool isUndefined() const {
    return getETag() == ETag::Undefined;
  }
  inline bool isEmpty() const {
    return getETag() == ETag::Empty;
  }
#ifdef HERMES_SLOW_DEBUG
  inline bool isInvalid() const {
    return getETag() == ETag::Invalid;
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
        // The cast is to ignore the following warning:
        // implicit conversion from 'int64_t' to 'double' changes value.
        num <= (double)std::numeric_limits<T>::max() && (T)num == num &&
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
  HermesValue &operator=(const HermesValue &hv) = delete;
#else
  HermesValue &operator=(const HermesValue &hv) = default;
#endif

  /// @name HV32 Compatibility APIs - DO NOT CALL DIRECTLY
  /// @{

  GCCell *getPointer(PointerBase *) const {
    return static_cast<GCCell *>(getPointer());
  }

  static HermesValue encodeHermesValue(HermesValue hv, Runtime *) {
    return hv;
  }

  /// }

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
  constexpr explicit HermesValue(uint64_t val, ETag etag)
      : raw_(val | ((uint64_t)etag << (kNumDataBits - 1))) {}

  /// Default move assignment operator used by friends
  /// (PseudoHandle<HermesValue>) in order to allow for move assignment in those
  /// friends. We cannot use SFINAE in PseudoHandle<HermesValue> as that would
  /// require making PseudoHandle<T> not TriviallyCopyable (because we would
  /// have to template or handwrite the move assignment operator).
  HermesValue &operator=(HermesValue &&) = default;

  // 64 raw bits stored and reinterpreted as necessary.
  uint64_t raw_;

  friend class PseudoHandle<HermesValue>;
  friend struct HVConstants;
}; // class HermesValue

static_assert(
    std::is_trivial<HermesValue>::value,
    "HermesValue must be trivial");

/// Encode common double constants to HermesValue.
/// The tricks is that that we know the bit patterns of known constants.
struct HVConstants {
  static constexpr HermesValue kZero = HermesValue(0);
  static constexpr HermesValue kOne = HermesValue(0x3ff0ull << 48);
  static constexpr HermesValue kNegOne = HermesValue(0xbff0ull << 48);
};

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
  template <typename T>
  inline PinnedHermesValue &operator=(PseudoHandle<T> &&hv);
} HERMES_ATTRIBUTE_WARN_UNUSED_VARIABLES;

// All HermesValues stored in heap object should be of this
// type. Hides assignment operator, but provides set operations that
// do a write barrier for pointer values, or else assert that the new
// value is not a pointer.
template <typename HVType>
class GCHermesValueBase final : public HVType {
 public:
  GCHermesValueBase() : HVType(HVType::encodeUndefinedValue()) {}
  /// Initialize a GCHermesValue from another HV. Performs a write barrier.
  template <typename NeedsBarriers = std::true_type>
  GCHermesValueBase(HVType hv, GC *gc);
  /// Initialize a GCHermesValue from a non-pointer HV. Might perform a write
  /// barrier, depending on the GC.
  /// NOTE: The last parameter is unused, but acts as an overload selector.
  template <typename NeedsBarriers = std::true_type>
  GCHermesValueBase(HVType hv, GC *gc, std::nullptr_t);
  GCHermesValueBase(const HVType &) = delete;

  /// The HermesValue \p hv may be an object pointer.  Assign the
  /// value, and perform any necessary write barriers.
  template <typename NeedsBarriers = std::true_type>
  inline void set(HVType hv, GC *gc);

  /// The HermesValue \p hv must not be an object pointer.  Assign the
  /// value.
  /// Some GCs still need to do a write barrier though, so pass a GC parameter.
  inline void setNonPtr(HVType hv, GC *gc);

  /// Force a write barrier to occur on this value, as if the value was being
  /// set to null. This should be used when a value is becoming unreachable by
  /// the GC, without having anything written to its memory.
  /// NOTE: This barrier is typically used when a variable-sized object's length
  /// decreases.
  inline void unreachableWriteBarrier(GC *gc);

  /// Fills a region of GCHermesValues defined by [\p first, \p last) with the
  /// value \p fill.  If the fill value is an object pointer, must
  /// provide a non-null \p gc argument, to perform write barriers.
  template <typename InputIt>
  static inline void fill(InputIt first, InputIt last, HVType fill, GC *gc);

  /// Same as \p fill except the range expressed by  [\p first, \p last) has not
  /// been previously initialized. Cannot use this on previously initialized
  /// memory, as it will use an incorrect write barrier.
  template <typename InputIt>
  static inline void
  uninitialized_fill(InputIt first, InputIt last, HVType fill, GC *gc);

  /// Copies a range of values and performs a write barrier on each.
  template <typename InputIt, typename OutputIt>
  static inline OutputIt
  copy(InputIt first, InputIt last, OutputIt result, GC *gc);

  /// Same as \p copy, but the range [result, result + (last - first)) has not
  /// been previously initialized. Cannot use this on previously initialized
  /// memory, as it will use an incorrect write barrier.
  template <typename InputIt, typename OutputIt>
  static inline OutputIt
  uninitialized_copy(InputIt first, InputIt last, OutputIt result, GC *gc);

#if !defined(HERMESVM_GC_HADES) && !defined(HERMESVM_GC_RUNTIME)
  /// Same as \p copy, but specialised for raw pointers.
  static inline GCHermesValueBase<HVType> *copy(
      GCHermesValueBase<HVType> *first,
      GCHermesValueBase<HVType> *last,
      GCHermesValueBase<HVType> *result,
      GC *gc);
#endif

  /// Same as \p uninitialized_copy, but specialised for raw pointers. This is
  /// unsafe to use if the memory region being copied into (pointed to by
  /// \p result) is reachable by the GC (for instance, memory within the
  /// size of an ArrayStorage), since it does not update elements atomically.
  static inline GCHermesValueBase<HVType> *uninitialized_copy(
      GCHermesValueBase<HVType> *first,
      GCHermesValueBase<HVType> *last,
      GCHermesValueBase<HVType> *result,
      GC *gc);

  /// Copies a range of values and performs a write barrier on each.
  template <typename InputIt, typename OutputIt>
  static inline OutputIt
  copy_backward(InputIt first, InputIt last, OutputIt result, GC *gc);

  /// Same as \c unreachableWriteBarrier, but for a range of values all becoming
  /// unreachable.
  static inline void rangeUnreachableWriteBarrier(
      GCHermesValueBase<HVType> *first,
      GCHermesValueBase<HVType> *last,
      GC *gc);
};

using GCHermesValue = GCHermesValueBase<HermesValue>;

/// copyToPinned is harder to generalise since it also depends on
/// PinnedHermesValue, so we keep it in a separate struct for now.
struct GCHermesValueUtil {
  /// Copies a range of values to a non-heap location, e.g., the JS stack.
  static inline void copyToPinned(
      const GCHermesValue *first,
      const GCHermesValue *last,
      PinnedHermesValue *result);
};

llvh::raw_ostream &operator<<(llvh::raw_ostream &OS, HermesValue hv);

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
