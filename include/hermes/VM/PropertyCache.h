/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef PROJECT_PROPERTYCACHE_H
#define PROJECT_PROPERTYCACHE_H

#include "hermes/VM/GCPointer.h"
#include "hermes/VM/SymbolID.h"
#include "hermes/VM/WeakRoot.h"
#include "hermes/VM/sh_mirror.h"

namespace hermes {
namespace vm {
using SlotIndex = uint32_t;

class HiddenClass;

/// A cache entry for property writes.
struct WritePropertyCacheEntry {
  /// Cached class.
  WeakRoot<HiddenClass> clazz{nullptr};

  /// Cached property index.
  SlotIndex slot{0};
};

/// A cache entry for property reads.
struct ReadPropertyCacheEntry {
  /// Cached class.
  WeakRoot<HiddenClass> clazz{nullptr};

  /// Cached property index.
  SlotIndex slot{0};
};

static_assert(
    sizeof(SHWritePropertyCacheEntry) == sizeof(WritePropertyCacheEntry));
static_assert(
    offsetof(SHWritePropertyCacheEntry, clazz) ==
    offsetof(WritePropertyCacheEntry, clazz));
static_assert(
    offsetof(SHWritePropertyCacheEntry, slot) ==
    offsetof(WritePropertyCacheEntry, slot));
static_assert(
    sizeof(SHReadPropertyCacheEntry) == sizeof(ReadPropertyCacheEntry));
static_assert(
    offsetof(SHReadPropertyCacheEntry, clazz) ==
    offsetof(ReadPropertyCacheEntry, clazz));
static_assert(
    offsetof(SHReadPropertyCacheEntry, slot) ==
    offsetof(ReadPropertyCacheEntry, slot));

} // namespace vm
} // namespace hermes
#endif // PROJECT_PROPERTYCACHE_H
