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
  /// Cached class: either for the object being read from,
  /// or it's prototype.
  WeakRoot<HiddenClass> clazz{nullptr};

  /// If \p clazz is a HiddenClass for the prototype, this is
  /// non-null and is the HiddenClass for the child object.  We call this
  /// a "negative match": the property was not found on an object
  /// with this HiddenClass, so if the object used in a subsequent
  /// read has the same HiddenClass, it also won't have the property.
  WeakRoot<HiddenClass> negMatchClazz{nullptr};

  /// Cached property index: in the object if \p clazz is the object's
  /// HiddenClass, or in the object's prototype if \p clazz is the
  /// prototype's HiddenClass.
  SlotIndex slot{0};
};

/// A cache entry used for all private name operations, e.g. reads, stores and
/// `in` checks using a private name.
struct PrivateNameCacheEntry {
  /// Cached class.
  WeakRoot<HiddenClass> clazz{nullptr};

  /// Cached symbol value.
  WeakRootSymbolID nameVal{};

  /// Cached property index. In the case of reads and stores, this is the offset
  /// where the property can be found. For existence checks, like the `in`
  /// operator, this is just a boolean.
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
static_assert(sizeof(SHPrivateNameCacheEntry) == sizeof(PrivateNameCacheEntry));
static_assert(
    offsetof(SHPrivateNameCacheEntry, clazz) ==
    offsetof(PrivateNameCacheEntry, clazz));
static_assert(
    offsetof(SHPrivateNameCacheEntry, nameVal) ==
    offsetof(PrivateNameCacheEntry, nameVal));
static_assert(
    offsetof(SHPrivateNameCacheEntry, slot) ==
    offsetof(PrivateNameCacheEntry, slot));

} // namespace vm
} // namespace hermes
#endif // PROJECT_PROPERTYCACHE_H
