/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SH_MIRROR_H
#define HERMES_SH_MIRROR_H

#include <stdint.h>

#ifdef HERMESVM_COMPRESSED_POINTERS
#define SH_COMPRESSED_POINTER_RAW_TYPE uint32_t
#else
#define SH_COMPRESSED_POINTER_RAW_TYPE uintptr_t
#endif

/// Struct mirroring the layout of PropertyCacheEntry. This allows us to expose
/// the offsets of certain fields without needing to make the actual C++ version
/// available here.
typedef struct SHPropertyCacheEntry {
  SH_COMPRESSED_POINTER_RAW_TYPE clazz;
  uint32_t slot;
} SHPropertyCacheEntry;

#endif
