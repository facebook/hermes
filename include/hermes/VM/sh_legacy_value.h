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
/// Native Pointer Encoding
/// ================
/// We also have limited support for storing a native pointer in a HermesValue.
/// When doing so, we do not associate a tag with the native pointer and instead
/// require that the pointer is a valid non-NaN double bit-for-bit. It is the
/// caller's responsibility to keep track of where these native pointers are
/// stored.
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

#ifndef HERMES_SH_LEGACY_VALUE_H
#define HERMES_SH_LEGACY_VALUE_H

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "hermes/VM/sh_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Version of the HermesValue encoding format.
/// Changing the format of HermesValue requires bumping this version number
/// and fixing any code that relies on the layout of HermesValue.
/// Updated: Aug 27, 2024
#define HERMESVALUE_VERSION 1

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
/// If tag < FirstTag, the encoded value is a double.
enum HVTag {
  HVTag_First = (HVTagType)(int8_t)0xf9,
  HVTag_EmptyInvalid = HVTag_First,
  HVTag_UndefinedNull,
  HVTag_BoolSymbol,
  HVTag_NativeValue,
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
  HVETag_Native1 = HVTag_NativeValue * 2,
  HVETag_Native2 = HVTag_NativeValue * 2 + 1,
  HVETag_Str1 = HVTag_Str * 2,
  HVETag_Str2 = HVTag_Str * 2 + 1,
  HVETag_BigInt1 = HVTag_BigInt * 2,
  HVETag_BigInt2 = HVTag_BigInt * 2 + 1,
  HVETag_Object1 = HVTag_Object * 2,
  HVETag_Object2 = HVTag_Object * 2 + 1,

  HVETag_FirstPointer = HVETag_Str1,
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
  if (__builtin_expect(v != v, false))
    return _sh_ljs_double(NAN);
  return _sh_ljs_double(v);
}

static inline SHLegacyValue _sh_ljs_native_pointer(void *p) {
  return (SHLegacyValue){(uintptr_t)p};
}

static inline SHLegacyValue _sh_ljs_native_uint32(uint32_t val) {
  return _sh_ljs_encode_raw_tag((uint64_t)val, HVTag_NativeValue);
}

static inline SHLegacyValue _sh_ljs_bool(bool b) {
  return _sh_ljs_encode_raw_etag(b, HVETag_Bool);
}

static inline SHLegacyValue _sh_ljs_object(void *p) {
  return _sh_ljs_encode_raw_tag((uint64_t)p, HVTag_Object);
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
  // Clear the ETag and return the raw values that are left.
  return (bool)(v.raw & 1);
}
static inline double _sh_ljs_get_double(SHLegacyValue v) {
  return v.f64;
}
static inline void *_sh_ljs_get_pointer(SHLegacyValue v) {
  // Mask out the tag.
  return (void *)(v.raw & kHV_DataMask);
}
static inline void *_sh_ljs_get_native_pointer(SHLegacyValue v) {
  return (void *)(uintptr_t)v.raw;
}

/// Flags associated with an object.
typedef struct SHObjectFlags {
  /// New properties cannot be added.
  uint32_t noExtend : 1;

  /// \c Object.seal() has been invoked on this object, marking all properties
  /// as non-configurable. When \c Sealed is set, \c NoExtend is always set too.
  uint32_t sealed : 1;

  /// \c Object.freeze() has been invoked on this object, marking all properties
  /// as non-configurable and non-writable. When \c Frozen is set, \c Sealed and
  /// must \c NoExtend are always set too.
  uint32_t frozen : 1;

  /// This object has indexed storage. This flag will not change at runtime, it
  /// is set at construction and its value never changes. It is not a state.
  uint32_t indexedStorage : 1;

  /// This flag is set to true when \c IndexedStorage is true and
  /// \c class->hasIndexLikeProperties are false. It allows our fast paths to do
  /// a simple bit check.
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

  /// A non-zero object id value, assigned lazily. It is 0 before it is
  /// assigned. If an object started out as lazy, the objectID is the lazy
  /// object index used to identify when it gets initialized.
  uint32_t objectID : 24;
} SHObjectFlags;

#ifdef __cplusplus
}
#endif

#endif // HERMES_SH_LEGACY_VALUE_H
