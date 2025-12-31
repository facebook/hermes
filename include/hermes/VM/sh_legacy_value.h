/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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
/// Heap Pointer Encoding
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
///
/// Native Pointer/Value Encoding
/// ================
/// We also have limited support for storing a native pointer or 32 bit "native"
/// uint32 in a HermesValue. When doing so, we do not associate a tag with the
/// native value and instead require that the value is a valid non-NaN double
/// bit-for-bit. It is the caller's responsibility to keep track of where these
/// native values are stored.
///
/// Native pointers cannot be NaN-boxed because on platforms where the ARM
/// memory tagging extension is enabled, the top byte may also have bits set
/// in it. On Android, these tags are added in the 56th to 59th bits of pointers
/// allocated in the native heap. However, as long as it is only the top byte
/// and the bottom 48 bits that have non-zero values, we are guaranteed that the
/// value will not be a NaN.
///
/// Fortunately, since the native pointers will appear as doubles to anything
/// other than the code that created them, anything that scans HermesValues
/// (e.g. the GC or heap snapshots), will simply ignore them.
///
/// Unlike native pointers, native uint32 may be NaN-boxed since they can fit in
/// the low bits of a NaN, but since we always know where they are, it is more
/// efficient to avoid tagging them.

#ifndef HERMES_SH_LEGACY_VALUE_H
#define HERMES_SH_LEGACY_VALUE_H

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "hermes/Support/sh_tryfast_fp_cvt.h"
#include "hermes/VM/sh_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Version of the HermesValue encoding format.
/// Changing the format of HermesValue requires bumping this version number
/// and fixing any code that relies on the layout of HermesValue.
/// Updated: Feb 21, 2025
#define HERMESVALUE_VERSION 2

struct HermesValueBase {
  union {
    // 64 raw bits stored and reinterpreted as necessary.
    uint64_t raw;
    double f64;
  };
};

typedef struct HermesValueBase SHLegacyValue;

typedef int32_t HVTagType;

/// Tags are defined as 16-bit values positioned at the high bits of a 64-bit
/// word.
/// We define them in the enum as negative numbers, with the last tag being -1.
/// They range from -7 to -1 (signed).
/// If in an unsigned comparison: tag < FirstTag, the encoded value is a double.
enum HVTag {
  HVTag_First = (HVTagType)(int8_t)0xf9,
  HVTag_EmptyInvalid = HVTag_First,
  HVTag_UndefinedNull,
  HVTag_BoolSymbol,
  HVTag_Unused,
  /// Pointer tags start here.
  HVTag_FirstPointer,
  HVTag_Str = HVTag_FirstPointer,
  HVTag_BigInt,
  HVTag_Object,
  HVTag_Last = (int8_t)0xff,
};

static_assert(HVTag_Object <= HVTag_Last, "Tags overflow");

/// An "extended tag", occupying one extra bit.
enum HVETag {
  HVETag_Empty = HVTag_EmptyInvalid * 2,
#ifdef HERMES_SLOW_DEBUG
  /// An invalid hermes value is one that should never exist in normal
  /// operation, it can be used as a sigil to indicate a programming failure.
  HVETag_Invalid = HVTag_EmptyInvalid * 2 + 1,
#endif
  HVETag_Undefined = HVTag_UndefinedNull * 2,
  HVETag_Null = HVTag_UndefinedNull * 2 + 1,
  HVETag_Bool = HVTag_BoolSymbol * 2,
  HVETag_Symbol = HVTag_BoolSymbol * 2 + 1,
  HVETag_Str1 = HVTag_Str * 2,
  HVETag_Str2 = HVTag_Str * 2 + 1,
  HVETag_BigInt1 = HVTag_BigInt * 2,
  HVETag_BigInt2 = HVTag_BigInt * 2 + 1,
  HVETag_Object1 = HVTag_Object * 2,
  HVETag_Object2 = HVTag_Object * 2 + 1,

  HVETag_FirstPointer = HVETag_Str1,

  /// This represents the last tag that corresponds either to a number, or a
  /// value that can be encoded entirely in its most significant 29 bits, with
  /// the rest being 0. This is used by HermesValue32 to determine whether the
  /// value should be considered for storage in "compressed HV64" form.
  HVETag_LastNumberOrCompressible = HVETag_Bool,
};

/// Number of bits used in the high part to encode the sign, exponent and tag.
static const unsigned kHV_NumTagExpBits = 16;
/// Number of bits available for data storage.
static const unsigned kHV_NumDataBits = (64 - kHV_NumTagExpBits);

/// Width of a tag in bits. The tag is aligned to the right of the top bits.
static const unsigned kHV_TagWidth = 3;
static const unsigned kHV_TagMask = (1 << kHV_TagWidth) - 1;
/// Mask to extract the data from the whole 64-bit word.
static const uint64_t kHV_DataMask = (1ull << kHV_NumDataBits) - 1;

static const unsigned kHV_ETagWidth = 4;
static const unsigned kHV_ETagMask = (1 << kHV_ETagWidth) - 1;

/// The value of a bool is encoded in the most significant bit after the ETag.
static const unsigned kHV_BoolBitIdx = kHV_NumDataBits - 2;

static inline SHLegacyValue _sh_ljs_encode_raw_tag(
    uint64_t val,
    enum HVTag tag) {
  return (SHLegacyValue){val | ((uint64_t)tag << kHV_NumDataBits)};
}
static inline SHLegacyValue _sh_ljs_encode_raw_etag(
    uint64_t val,
    enum HVETag etag) {
  return (SHLegacyValue){val | ((uint64_t)etag << (kHV_NumDataBits - 1))};
}
static inline enum HVTag _sh_ljs_get_tag(SHLegacyValue v) {
  return (enum HVTag)((int64_t)v.raw >> kHV_NumDataBits);
}
static inline enum HVETag _sh_ljs_get_etag(SHLegacyValue v) {
  return (enum HVETag)((int64_t)v.raw >> (kHV_NumDataBits - 1));
}

static inline SHLegacyValue _sh_ljs_double(double v) {
  union {
    double d;
    uint64_t i;
  } u;
  u.d = v;
  return (SHLegacyValue){u.i};
}
static inline SHLegacyValue _sh_ljs_untrusted_double(double v) {
  if (SH_UNLIKELY(v != v))
    return _sh_ljs_double(NAN);
  return _sh_ljs_double(v);
}

static inline SHLegacyValue _sh_ljs_native_pointer(void *p) {
  return (SHLegacyValue){(uintptr_t)p};
}

/// Encode a 32-bit unsigned integer bit-for-bit as a HermesValue. We know that
/// the resulting value will always be a valid non-NaN double.
static inline SHLegacyValue _sh_ljs_native_uint32(uint32_t val) {
  return (SHLegacyValue){val};
}

static inline SHLegacyValue _sh_ljs_bool(bool b) {
  // Bool occupies the most significant bit after the ETag.
  return _sh_ljs_encode_raw_etag((uint64_t)b << kHV_BoolBitIdx, HVETag_Bool);
}

static inline SHLegacyValue _sh_ljs_object(void *p) {
  return _sh_ljs_encode_raw_tag((uint64_t)(uintptr_t)p, HVTag_Object);
}

static inline SHLegacyValue _sh_ljs_undefined() {
  return _sh_ljs_encode_raw_etag(0, HVETag_Undefined);
}
static inline SHLegacyValue _sh_ljs_null() {
  return _sh_ljs_encode_raw_etag(0, HVETag_Null);
}
static inline SHLegacyValue _sh_ljs_empty() {
  return _sh_ljs_encode_raw_etag(0, HVETag_Empty);
}

/// Create a SHLegacyValue that has the raw representation 0. This value
/// must never become visible to user code, and is guaranteed to be ignored by
/// the GC.
static inline SHLegacyValue _sh_ljs_raw_zero_value_unsafe() {
  return (SHLegacyValue){0};
}

static inline bool _sh_ljs_is_undefined(SHLegacyValue v) {
  return _sh_ljs_get_etag(v) == HVETag_Undefined;
}
static inline bool _sh_ljs_is_null(SHLegacyValue v) {
  return _sh_ljs_get_etag(v) == HVETag_Null;
}
static inline bool _sh_ljs_is_empty(SHLegacyValue v) {
  return _sh_ljs_get_etag(v) == HVETag_Empty;
}
static inline bool _sh_ljs_is_bool(SHLegacyValue v) {
  return _sh_ljs_get_etag(v) == HVETag_Bool;
}
static inline bool _sh_ljs_is_symbol(SHLegacyValue v) {
  return _sh_ljs_get_etag(v) == HVETag_Symbol;
}
static inline bool _sh_ljs_is_object(SHLegacyValue v) {
  return _sh_ljs_get_tag(v) == HVTag_Object;
}
static inline bool _sh_ljs_is_double(SHLegacyValue v) {
  return v.raw < ((uint64_t)HVTag_First << kHV_NumDataBits);
}
static inline bool _sh_ljs_is_pointer(SHLegacyValue v) {
  return v.raw >= ((uint64_t)HVTag_FirstPointer << kHV_NumDataBits);
}
static inline bool _sh_ljs_is_bigint(SHLegacyValue v) {
  return _sh_ljs_get_tag(v) == HVTag_BigInt;
}
static inline bool _sh_ljs_is_string(SHLegacyValue v) {
  return _sh_ljs_get_tag(v) == HVTag_Str;
}

static inline bool _sh_ljs_get_bool(SHLegacyValue v) {
  // Bool occupies the most significant bit after the ETag.
  return (bool)(v.raw & (1ull << kHV_BoolBitIdx));
}
static inline double _sh_ljs_get_double(SHLegacyValue v) {
  return v.f64;
}
static inline void *_sh_ljs_get_pointer(SHLegacyValue v) {
  // Mask out the tag.
  return (void *)(uintptr_t)(v.raw & kHV_DataMask);
}
static inline void *_sh_ljs_get_native_pointer(SHLegacyValue v) {
  return (void *)(uintptr_t)v.raw;
}

/// Get a native uint32 value stored in the SHLegacyValue. This must only be
/// used in instances where the caller knows the type of this value, since
/// there is no corresponding tag (it just looks like a double).
static inline uint32_t _sh_ljs_get_native_uint32(SHLegacyValue v) {
  return v.raw;
}

/// Test whether the value is a non-NaN number. Since we use
/// NaN-boxing, this just checks if the parameter is NaN, which can have
/// some performance advantages over _sh_ljs_is_double():
///  1. Checking for NaN is typically a single instruction.
///  2. The operation is done in a floating point register, which may avoid
///     some moves if other users of the value are floating point operations.
static inline bool _sh_ljs_is_non_nan_number(SHLegacyValue v) {
  double d = v.f64;
  // NaN is the only double value that does not compare equal to itself.
  return d == d;
}

/// If the value is a number that can be efficiently truncated to a 32 bit
/// number, return true and store the result in \p res. Otherwise, return
/// false and leave \p res in an unspecified state.
static inline bool _sh_ljs_tryfast_truncate_to_int32(
    SHLegacyValue v,
    int32_t *res) {
// If we are compiling with ARM v8.3 or above, there is a special instruction
// to do the conversion.
#ifdef __ARM_FEATURE_JCVT
  if (!_sh_ljs_is_non_nan_number(v))
    return false;
  *res = __builtin_arm_jcvt(v.f64);
  return true;
#endif

  // Since we use NaN-boxing for non-number values, we know that any
  // non-number values will fail the attempted conversion to int32. So we can
  // simply attempt the conversion without checking for numbers.
  if (HERMES_TRYFAST_F64_TO_64_IS_FAST) {
    int64_t fast = _sh_tryfast_f64_to_i64_cvt(v.f64);
    *res = (int32_t)fast;
    return (double)fast == v.f64;
  } else {
    *res = _sh_tryfast_f64_to_i32_cvt(v.f64);
    return (double)*res == v.f64;
  }
}

/// If the value is a number that can be efficiently truncated to a 32 bit
/// unsigned number, return true and store the result in \p res.
/// Otherwise, return false and leave \p res in an unspecified state.
static inline bool _sh_ljs_tryfast_truncate_to_uint32(
    SHLegacyValue v,
    uint32_t *res) {
// If we are compiling with ARM v8.3 or above, there is a special instruction
// to do the conversion.
#ifdef __ARM_FEATURE_JCVT
  if (!_sh_ljs_is_non_nan_number(v))
    return false;
  *res = (uint32_t)__builtin_arm_jcvt(v.f64);
  return true;
#endif

  // Since we use NaN-boxing for non-number values, we know that any
  // non-number values will fail the attempted conversion to uint32. So we can
  // simply attempt the conversion without checking for numbers.
  if (HERMES_TRYFAST_F64_TO_64_IS_FAST) {
    uint64_t fast = _sh_tryfast_f64_to_u64_cvt(v.f64);
    *res = (uint32_t)fast;
    return (double)fast == v.f64;
  } else {
    *res = _sh_tryfast_f64_to_u32_cvt(v.f64);
    return (double)*res == v.f64;
  }
}

/// Test whether the given HermesValues are both non-NaN number values.
/// Since we use NaN-boxing, this just checks if either parameter is NaN, which
/// can have some performance advantages over _sh_ljs_is_double():
///  1. This can typically be done with a single comparison if the
///     architecture provides a condition code that is set if either
///     operand to a comparison is NaN (e.g. VS on ARM).
///  2. The operation is done in a floating point register, which may avoid
///     some moves if other users of the value are floating point operations.
static inline bool _sh_ljs_are_both_non_nan_numbers(
    SHLegacyValue a,
    SHLegacyValue b) {
  // We do not use isunordered() here because it may produce a call on some
  // compilers (e.g. MSVC). Instead, we use a builtin when it is available, or
  // fall back to checking each operand for NaN if it is not.
#ifdef __has_builtin
#if __has_builtin(__builtin_isunordered)
  return !__builtin_isunordered(a.f64, b.f64);
#endif
#endif
  return _sh_ljs_is_non_nan_number(a) && _sh_ljs_is_non_nan_number(b);
}

/// Flags associated with an object.
typedef union {
  struct {
    /// New properties cannot be added.
    uint32_t noExtend : 1;

    /// \c Object.seal() has been invoked on this object, marking all properties
    /// as non-configurable. When \c Sealed is set, \c NoExtend is always set
    /// too.
    uint32_t sealed : 1;

    /// \c Object.freeze() has been invoked on this object, marking all
    /// properties as non-configurable and non-writable. When \c Frozen is set,
    /// \c Sealed and must \c NoExtend are always set too.
    uint32_t frozen : 1;

    /// This object has indexed storage. This flag will not change at runtime,
    /// it is set at construction and its value never changes. It is not a
    /// state.
    uint32_t indexedStorage : 1;

    /// This flag is set to true when \c IndexedStorage is true and
    /// \c class->hasIndexLikeProperties are false. It allows our fast paths to
    /// do a simple bit check.
    uint32_t fastIndexProperties : 1;

    /// This flag indicates this is a special object whose properties are
    /// managed by C++ code, and not via the standard property storage
    /// mechanisms.
    uint32_t hostObject : 1;

    /// this is lazily created object that must be initialized before it can be
    /// used. Note that lazy objects must have no properties defined on them,
    uint32_t lazyObject : 1;

    /// This flag indicates this is a proxy exotic Object
    uint32_t proxyObject : 1;

    /// This flag is set when any other objects which contain this object in
    /// their parent chain are cached in the AddPropertyCache, or if this object
    /// was found in the prototype chain of an array.
    ///
    /// If the flag is set, then changing the parent or HiddenClass of this
    /// object will increment the parentCacheEpoch in Runtime.
    /// Note that property adds are not cached if any object in the parent chain
    /// is in dictionary mode.
    uint32_t isCachedUsingEpoch : 1;

    /// A non-zero object id value, assigned lazily. It is 0 before it is
    /// assigned. If an object started out as lazy, the objectID is the lazy
    /// object index used to identify when it gets initialized.
    uint32_t objectID : 23;
  };
  uint32_t bits;
} SHObjectFlags;

/// Used for debugging.
void _sh_ljs_dump_to_stderr(SHLegacyValue v);

#ifdef __cplusplus
}
#endif

#endif // HERMES_SH_LEGACY_VALUE_H
