/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_HERMESVALUE_H
#define HERMES_VM_HERMESVALUE_H

#include "hermes/Support/Conversions.h"
#include "hermes/VM/CompressedPointer.h"
#include "hermes/VM/GCDecl.h"
#include "hermes/VM/SymbolID.h"
#include "hermes/VM/sh_legacy_value.h"

#include "llvh/Support/raw_ostream.h"

#include <cassert>
#include <cmath>
#include <cstdint>

namespace llvh {
class raw_ostream;
}

namespace hermes {
namespace vm {

class BigIntPrimitive;
class StringPrimitive;
template <typename T>
class PseudoHandle;

template <typename T>
class PseudoHandle;

// Only used for HV32 API compatibility.
class GCCell;
class Runtime;

// Ensure that HermesValue tags are handled correctly by updating this every
// time the HERMESVALUE_VERSION changes, and going through the JIT and updating
static_assert(
    HERMESVALUE_VERSION == 1,
    "HermesValue version mismatch, HermesValue methods may need to be updated");

/// A NaN-box encoded value.
class HermesValue : public HermesValueBase {
 public:
  using TagType = HVTagType;
  /// Tags are defined as 16-bit values positioned at the high bits of a 64-bit
  /// word.
  enum class Tag : TagType {
    /// If tag < FirstTag, the encoded value is a double.
    First = HVTag_First,
    EmptyInvalid = HVTag_EmptyInvalid,
    UndefinedNull = HVTag_UndefinedNull,
    BoolSymbol = HVTag_BoolSymbol,

    /// Pointer tags start here.
    FirstPointer = HVTag_FirstPointer,
    Str = HVTag_Str,
    BigInt = HVTag_BigInt,
    Object = HVTag_Object,
    Last = HVTag_Last,
  };
  static_assert(Tag::Object <= Tag::Last, "Tags overflow");

  /// An "extended tag", occupying one extra bit.
  enum class ETag : TagType {
    Empty = HVETag_Empty,
#ifdef HERMES_SLOW_DEBUG
    /// An invalid hermes value is one that should never exist in normal
    /// operation, it can be used as a sigil to indicate a programming failure.
    Invalid = HVETag_Invalid,
#endif
    Undefined = HVETag_Undefined,
    Null = HVETag_Null,
    Bool = HVETag_Bool,
    Symbol = HVETag_Symbol,
    Str1 = HVETag_Str1,
    Str2 = HVETag_Str2,
    BigInt1 = HVETag_BigInt1,
    BigInt2 = HVETag_BigInt2,
    Object1 = HVETag_Object1,
    Object2 = HVETag_Object2,

    FirstPointer = HVETag_FirstPointer,
  };

  /// Number of bits used in the high part to encode the sign, exponent and tag.
  static constexpr unsigned kNumTagExpBits = kHV_NumTagExpBits;
  /// Number of bits available for data storage.
  static constexpr unsigned kNumDataBits = kHV_NumDataBits;

  /// Width of a tag in bits. The tag is aligned to the right of the top bits.
  static constexpr unsigned kTagWidth = kHV_TagWidth;
  static constexpr unsigned kTagMask = kHV_TagMask;
  /// Mask to extract the data from the whole 64-bit word.
  static constexpr uint64_t kDataMask = kHV_DataMask;

  static constexpr unsigned kETagWidth = kHV_ETagWidth;
  static constexpr unsigned kETagMask = kHV_ETagMask;

  /// Assert that the pointer can be encoded in \c kNumDataBits.
  static void validatePointer(const void *ptr) {
#if LLVM_PTR_SIZE == 8
    assert(
        (reinterpret_cast<uintptr_t>(ptr) & ~kDataMask) == 0 &&
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

  inline Tag getTag() const {
    return (Tag)((int64_t)this->raw >> kNumDataBits);
  }
  inline ETag getETag() const {
    return (ETag)((int64_t)this->raw >> (kNumDataBits - 1));
  }

  /// Combine two tags into an 8-bit value.
  inline static constexpr unsigned combineETags(ETag aTag, ETag bTag) {
    using UTagType = std::make_unsigned<TagType>::type;
    auto a = static_cast<UTagType>(aTag), b = static_cast<UTagType>(bTag);
    return ((a & kETagMask) << kETagWidth) | (b & kETagMask);
  }

  /// Special functions that allow nullptr to be stored in a HermesValue.
  /// WARNING: These should never be used on the JS stack or heap, and are only
  /// intended for Handles.
  constexpr inline static HermesValue encodeNullptrObjectValueUnsafe() {
    return HermesValue(0, Tag::Object);
  }

  inline static HermesValue encodeObjectValueUnsafe(void *val) {
    validatePointer(val);
    HermesValue RV(reinterpret_cast<uintptr_t>(val), Tag::Object);
    assert(RV.isObject());
    return RV;
  }

  inline static HermesValue encodeStringValueUnsafe(
      const StringPrimitive *val) {
    validatePointer(val);
    HermesValue RV(reinterpret_cast<uintptr_t>(val), Tag::Str);
    assert(RV.isString());
    return RV;
  }

  inline static HermesValue encodeBigIntValueUnsafe(
      const BigIntPrimitive *val) {
    validatePointer(val);
    HermesValue RV(reinterpret_cast<uintptr_t>(val), Tag::BigInt);
    assert(RV.isBigInt());
    return RV;
  }

  inline static HermesValue encodeObjectValue(void *val) {
    assert(val && "Null pointers require special handling.");
    return encodeObjectValueUnsafe(val);
  }

  inline static HermesValue encodeStringValue(const StringPrimitive *val) {
    assert(val && "Null pointers require special handling.");
    return encodeStringValueUnsafe(val);
  }

  inline static HermesValue encodeBigIntValue(const BigIntPrimitive *val) {
    assert(val && "Null pointers require special handling.");
    return encodeBigIntValueUnsafe(val);
  }

  /// Encode a 32-bit unsigned integer bit-for-bit as a HermesValue. We know
  /// that the resulting value will always be a valid non-NaN double.
  inline static HermesValue encodeNativeUInt32(uint32_t val) {
    HermesValue RV(val);
    assert(
        RV.isDouble() && RV.getNativeUInt32() == val &&
        "Native value encoding failed");
    return RV;
  }

  inline static HermesValue encodeNativePointer(const void *p) {
    HermesValue RV(reinterpret_cast<uintptr_t>(p));
    assert(
        RV.isDouble() && RV.getNativePointer<void>() == p &&
        "Native pointer cannot be represented as a double");
    return RV;
  }

  inline static HermesValue encodeSymbolValue(SymbolID val) {
    HermesValue RV(val.unsafeGetRaw(), ETag::Symbol);
    assert(RV.isSymbol());
    return RV;
  }

  constexpr inline static HermesValue encodeBoolValue(bool val) {
    return HermesValue((uint64_t)(val), ETag::Bool);
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

  /// Encodes \p num as a hermes value without checking for NaNs.
  /// Should only be used in for values coming from locations within the VM,
  /// where we know any NaN will be the quiet NaN.
  inline static HermesValue encodeTrustedNumberValue(double num) {
    HermesValue RV(llvh::DoubleToBits(num));
    assert(RV.isDouble() && "value not representable as double");
    return RV;
  }

  /// Encodes \p num as a hermes value without safety checkings.
  /// Should only be used in for values not coming (directly or indirectly) from
  /// user code.
  template <typename T>
  inline static
      typename std::enable_if<std::is_integral<T>::value, HermesValue>::type
      encodeTrustedNumberValue(T num) {
    assert((double)num == num && "value not representable as double");
    return encodeTrustedNumberValue((double)num);
  }

  /// Encode a numeric value into the best possible representation based on the
  /// static type of the parameter. Right now we only have one representation
  /// (double), but that could change in the future.
  inline static HermesValue encodeUntrustedNumberValue(double num) {
    if (LLVM_UNLIKELY(std::isnan(num))) {
      return encodeNaNValue();
    }
    return HermesValue(llvh::DoubleToBits(num));
  }

  /// Encode a numeric value into the best possible representation based on the
  /// static type of the parameter. Right now we only have one representation
  /// (double), but that could change in the future.
  template <typename T>
  inline static
      typename std::enable_if<std::is_integral<T>::value, HermesValue>::type
      encodeUntrustedNumberValue(T num) {
    return encodeTrustedNumberValue((double)num);
  }

  static HermesValue encodeNaNValue() {
    return HermesValue(
        llvh::DoubleToBits(std::numeric_limits<double>::quiet_NaN()));
  }

  /// Keeping tag constant, make a new HermesValue with \p val stored in it.
  inline HermesValue updatePointer(void *val) const {
    assert(isPointer());
    HermesValue V(reinterpret_cast<uintptr_t>(val), getTag());
    assert(V.isPointer());
    return V;
  }

  /// Update a HermesValue in place to encode \p ptr. Used by (de)serializer.
  inline void unsafeUpdatePointer(void *ptr) {
    setNoBarrier(updatePointer(ptr));
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
  inline bool isSymbol() const {
    return getETag() == ETag::Symbol;
  }
  inline bool isBool() const {
    return getETag() == ETag::Bool;
  }
  inline bool isObject() const {
    return getTag() == Tag::Object;
  }
  inline bool isString() const {
    return getTag() == Tag::Str;
  }
  inline bool isBigInt() const {
    return getTag() == Tag::BigInt;
  }
  inline bool isDouble() const {
    return this->raw < ((uint64_t)Tag::First << kNumDataBits);
  }
  inline bool isPointer() const {
    return this->raw >= ((uint64_t)Tag::FirstPointer << kNumDataBits);
  }
  inline bool isNumber() const {
    return isDouble();
  }
  inline bool isNaN() const {
    // Mask out the sign bit since it does not affect whether the value is a
    // NaN. All the other bits must be equal to the NaN bit pattern, since we
    // only use the quiet NaN to represent NaN.
    uint64_t kMask = llvh::maskLeadingZeros<uint64_t>(1);
    return (this->raw & kMask) == (encodeNaNValue().raw & kMask);
  }

  inline RawType getRaw() const {
    return this->raw;
  }

  inline void *getPointer() const {
    assert(isPointer());
    // Mask out the tag.
    return reinterpret_cast<void *>(this->raw & kDataMask);
  }

  inline double getDouble() const {
    assert(isDouble());
    return llvh::BitsToDouble(this->raw);
  }

  /// Get a native uint32 value stored in the HermesValue. This must only be
  /// used in instances where the caller knows the type of this value, since
  /// there is no corresponding tag (it just looks like a double).
  inline uint32_t getNativeUInt32() const {
    assert(isDouble() && "Native uint32 must look like a double.");
    return (uint32_t)this->raw;
  }

  template <class T>
  inline T *getNativePointer() const {
    assert(isDouble() && "Native pointers must look like doubles.");
    return reinterpret_cast<T *>(this->raw);
  }

  inline SymbolID getSymbol() const {
    assert(isSymbol());
    return SymbolID::unsafeCreate((uint32_t)this->raw);
  }

  inline bool getBool() const {
    assert(isBool());
    return (bool)(this->raw & 0x1);
  }

  inline StringPrimitive *getString() const {
    assert(isString());
    return static_cast<StringPrimitive *>(getPointer());
  }

  inline BigIntPrimitive *getBigInt() const {
    assert(isBigInt());
    return static_cast<BigIntPrimitive *>(getPointer());
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
  inline void setInGC(HermesValue hv, GC &gc);

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

  GCCell *getPointer(PointerBase &) const {
    return static_cast<GCCell *>(getPointer());
  }
  GCCell *getObject(PointerBase &) const {
    return static_cast<GCCell *>(getObject());
  }

  static HermesValue encodeHermesValue(HermesValue hv, Runtime &) {
    return hv;
  }
  static HermesValue encodeObjectValue(GCCell *obj, Runtime &) {
    return encodeObjectValue(obj);
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
    this->raw = hv.raw;
  }

 private:
  constexpr explicit HermesValue(uint64_t val) : HermesValueBase{val} {}
  constexpr explicit HermesValue(uint64_t val, Tag tag)
      : HermesValueBase{val | ((uint64_t)tag << kNumDataBits)} {}
  constexpr explicit HermesValue(uint64_t val, ETag etag)
      : HermesValueBase{val | ((uint64_t)etag << (kNumDataBits - 1))} {}

  /// Default move assignment operator used by friends
  /// (PseudoHandle<HermesValue>) in order to allow for move assignment in those
  /// friends. We cannot use SFINAE in PseudoHandle<HermesValue> as that would
  /// require making PseudoHandle<T> not TriviallyCopyable (because we would
  /// have to template or handwrite the move assignment operator).
  HermesValue &operator=(HermesValue &&) = default;

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
  GCHermesValueBase(HVType hv, GC &gc);
  /// Initialize a GCHermesValue from a non-pointer HV. Might perform a write
  /// barrier, depending on the GC.
  /// NOTE: The last parameter is unused, but acts as an overload selector.
  template <typename NeedsBarriers = std::true_type>
  GCHermesValueBase(HVType hv, GC &gc, std::nullptr_t);
  GCHermesValueBase(const HVType &) = delete;

  /// The HermesValue \p hv may be an object pointer.  Assign the
  /// value, and perform any necessary write barriers.
  template <typename NeedsBarriers = std::true_type>
  inline void set(HVType hv, GC &gc);

  /// The HermesValue \p hv must not be an object pointer.  Assign the
  /// value.
  /// Some GCs still need to do a write barrier though, so pass a GC parameter.
  inline void setNonPtr(HVType hv, GC &gc);

  /// Force a write barrier to occur on this value, as if the value was being
  /// set to null. This should be used when a value is becoming unreachable by
  /// the GC, without having anything written to its memory.
  /// NOTE: This barrier is typically used when a variable-sized object's length
  /// decreases.
  inline void unreachableWriteBarrier(GC &gc);

  /// Fills a region of GCHermesValues defined by [\p first, \p last) with the
  /// value \p fill.  If the fill value is an object pointer, must
  /// provide a non-null \p gc argument, to perform write barriers.
  template <typename InputIt>
  static inline void fill(InputIt first, InputIt last, HVType fill, GC &gc);

  /// Same as \p fill except the range expressed by  [\p first, \p last) has not
  /// been previously initialized. Cannot use this on previously initialized
  /// memory, as it will use an incorrect write barrier.
  template <typename InputIt>
  static inline void
  uninitialized_fill(InputIt first, InputIt last, HVType fill, GC &gc);

  /// Copies a range of values and performs a write barrier on each.
  template <typename InputIt, typename OutputIt>
  static inline OutputIt
  copy(InputIt first, InputIt last, OutputIt result, GC &gc);

  /// Same as \p copy, but the range [result, result + (last - first)) has not
  /// been previously initialized. Cannot use this on previously initialized
  /// memory, as it will use an incorrect write barrier.
  template <typename InputIt, typename OutputIt>
  static inline OutputIt
  uninitialized_copy(InputIt first, InputIt last, OutputIt result, GC &gc);

#if !defined(HERMESVM_GC_HADES) && !defined(HERMESVM_GC_RUNTIME)
  /// Same as \p copy, but specialized for raw pointers.
  static inline GCHermesValueBase<HVType> *copy(
      GCHermesValueBase<HVType> *first,
      GCHermesValueBase<HVType> *last,
      GCHermesValueBase<HVType> *result,
      GC &gc);
#endif

  /// Same as \p uninitialized_copy, but specialized for raw pointers. This is
  /// unsafe to use if the memory region being copied into (pointed to by
  /// \p result) is reachable by the GC (for instance, memory within the
  /// size of an ArrayStorage), since it does not update elements atomically.
  static inline GCHermesValueBase<HVType> *uninitialized_copy(
      GCHermesValueBase<HVType> *first,
      GCHermesValueBase<HVType> *last,
      GCHermesValueBase<HVType> *result,
      GC &gc);

  /// Copies a range of values and performs a write barrier on each.
  template <typename InputIt, typename OutputIt>
  static inline OutputIt
  copy_backward(InputIt first, InputIt last, OutputIt result, GC &gc);

  /// Same as \c unreachableWriteBarrier, but for a range of values all becoming
  /// unreachable.
  static inline void rangeUnreachableWriteBarrier(
      GCHermesValueBase<HVType> *first,
      GCHermesValueBase<HVType> *last,
      GC &gc);
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

} // end namespace vm
} // end namespace hermes

#endif // HERMES_VM_HERMESVALUE_H
