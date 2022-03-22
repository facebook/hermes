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

namespace hermes {
namespace vm {
using SlotIndex = uint32_t;

class HiddenClass;

/// A cache entry for a property lookup.
/// If the class operation that we are performing
/// matches the values in the cache entry, \c slot is the index of a
/// non-accessor property.
struct PropertyCacheEntry {
  /// Cached class.
  WeakRoot<HiddenClass> clazz{nullptr};

  /// Cached property index.
  SlotIndex slot{0};
};

} // namespace vm
} // namespace hermes
#endif // PROJECT_PROPERTYCACHE_H
