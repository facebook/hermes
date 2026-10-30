/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/ADT/TransparentConservativeVector.h"
#include "hermes/VM/PropertyCache.h"
#include "hermes/VM/RegExpBytecode.h"
#include "hermes/VM/static_h.h"

#include "llvh/ADT/DenseMap.h"

#include <cstdint>
#include <vector>

/// Data associated with SHUnit, but fully managed by the runtime.
struct SHUnitExt {
  /// A map from template object ids to template objects.
  llvh::DenseMap<uint32_t, hermes::vm::JSObject *> templateMap{};

  /// Vector of AddPropertyCacheEntry, where each element is lazily allocated
  /// whenever a CodeBlock needs a new entry.
  /// Stored in a central location here for easier marking of roots.
  hermes::TransparentConservativeVector<hermes::vm::AddPropertyCacheEntry>
      addCacheEntries{};

  /// The shared bytecode of each regexp literal, indexed like
  /// SHUnit::regexp_table and created on first use. Mutator thread only.
  std::vector<llvh::IntrusiveRefCntPtr<hermes::vm::RegExpBytecode>> regExpCache;

  /// Create the data for a unit with \p numRegExps regexp literals.
  explicit SHUnitExt(uint32_t numRegExps) : regExpCache(numRegExps) {
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

/// \return the shared bytecode of regexp literal \p regExpID of \p unit,
/// creating it from the unit's precompiled bytecode on first use. Mutator
/// thread only.
llvh::IntrusiveRefCntPtr<RegExpBytecode> sh_unit_get_regexp_bytecode(
    SHUnit *unit,
    uint32_t regExpID);

} // namespace hermes::vm
