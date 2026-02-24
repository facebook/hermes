/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/ADT/TransparentConservativeVector.h"
#include "hermes/VM/PropertyCache.h"
#include "hermes/VM/static_h.h"

#include "llvh/ADT/DenseMap.h"

#include <cstdint>

/// Data associated with SHUnit, but fully managed by the runtime.
struct SHUnitExt {
  /// A map from template object ids to template objects.
  llvh::DenseMap<uint32_t, hermes::vm::JSObject *> templateMap{};

  /// Vector of AddPropertyCacheEntry, where each element is lazily allocated
  /// whenever a CodeBlock needs a new entry.
  /// Stored in a central location here for easier marking of roots.
  hermes::TransparentConservativeVector<hermes::vm::AddPropertyCacheEntry>
      addCacheEntries{};

  explicit SHUnitExt() {
    // Reserve entry 0.
    addCacheEntries.emplace_back();
  }
};

namespace hermes::vm {

/// Allocate a new AddPropertyCacheEntry.
/// \return the index of the new entry, but if there's already too many to
/// allocate a new entry, return llvh::None.
/// The entry index will be at most
/// WritePropertyCacheEntry::kMaxAddCacheIndex.
OptValue<uint32_t> sh_unit_allocate_add_cache_entry(SHUnit *unit);

/// \return the number of add cache entries.
inline size_t sh_unit_num_add_cache_entries(SHUnit *unit) {
  return unit->runtime_ext->addCacheEntries.size();
}

/// \return the add cache entry at \p index. Reference is invalidated upon
/// calling allocateAddCacheEntry.
inline AddPropertyCacheEntry &sh_unit_get_add_cache_entry(
    SHUnit *unit,
    uint32_t index) {
  return unit->runtime_ext->addCacheEntries[index];
}

} // namespace hermes::vm
