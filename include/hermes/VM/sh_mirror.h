/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SH_MIRROR_H
#define HERMES_SH_MIRROR_H

#include "hermes/VM/sh_legacy_value.h"
#include "hermes/VM/sh_runtime.h"

#ifdef HERMESVM_COMPRESSED_POINTERS
typedef uint32_t SHCompressedPointerRawType;
#else
typedef uintptr_t SHCompressedPointerRawType;
#endif

typedef struct SHNativeFuncInfo SHNativeFuncInfo;
typedef struct SHUnit SHUnit;

/// Struct mirroring the layout of PropertyCacheEntry. This allows us to expose
/// the offsets of certain fields without needing to make the actual C++ version
/// available here.
typedef struct SHPropertyCacheEntry {
  SHCompressedPointerRawType clazz;
  uint32_t slot;
} SHPropertyCacheEntry;

/// Struct mirroring the layout of GCCell.
typedef struct SHGCCell {
  SHCompressedPointerRawType kindAndSize;
#ifndef NDEBUG
  uint16_t magic;
  uint32_t debugAllocationId;
#endif
} SHGCCell;

/// Struct mirroring the layout of JSObject (without the direct props).
typedef struct SHJSObject {
  SHGCCell base;
  SHObjectFlags flags;
  SHCompressedPointerRawType parent;
  SHCompressedPointerRawType clazz;
  SHCompressedPointerRawType propStorage;
} SHJSObject;

#ifdef HERMESVM_BOXED_DOUBLES
typedef SHCompressedPointerRawType SHGCSmallHermesValue;
#else
typedef SHLegacyValue SHGCSmallHermesValue;
#endif

typedef struct SHJSObjectAndDirectProps {
  SHJSObject base;
  SHGCSmallHermesValue directProps[HERMESVM_DIRECT_PROPERTY_SLOTS];
} SHJSObjectAndDirectProps;

/// Struct mirroring the layout of Callable.
typedef struct SHCallable {
  SHJSObject base;
  SHCompressedPointer environment;
} SHCallable;

/// A pointer to native function.
typedef SHLegacyValue (*NativeJSFunctionPtr)(SHRuntime *shr);

/// Struct mirroring the layout of NativeJSFunction.
typedef struct SHNativeJSFunction {
  SHCallable base;
  NativeJSFunctionPtr functionPtr;
  SHNativeFuncInfo *funcInfo;
  SHUnit *unit;
} SHNativeJSFunction;

/// Struct mirroring the layout of Environment.
typedef struct SHEnvironment {
  SHGCCell base;
  SHCompressedPointer parentEnvironment;
  uint32_t size;

  SHLegacyValue slots[0];
} SHEnvironment;

/// Struct mirroring the layout of FastArray.
typedef struct SHFastArray {
  SHJSObject base;
  SHCompressedPointer indexedStorage;
  SHGCSmallHermesValue length;
} SHFastArray;

/// Struct mirroring the layout of ArrayStorageSmall.
typedef struct SHArrayStorageSmall {
  SHGCCell base;

  uint32_t size;
  SHGCSmallHermesValue storage[0];
} SHArrayStorageSmall;

#endif
