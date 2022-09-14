/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_STATIC_H_H
#define HERMES_STATIC_H_H

#include "hermes/VM/sh_legacy_value.h"

#include <stdbool.h>

#ifndef __cplusplus
// uchar.h is not universally available, so just define our own.
typedef uint16_t char16_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SHRuntime SHRuntime;
typedef struct SHUnitExt SHUnitExt;
typedef uint32_t SHSymbolID;

/// SHUnit describes a compilation unit.
///
/// <h2>Restrictions</h2>
/// A SHUnit can only be associated with one SHRuntime at a time. Once
/// associated with a runtime, a unit cannot be unloaded and persists until
/// the runtime is destroyed. At that point the unit can be reinitialized and
/// used again by a different runtime.
///
/// <br>One implication is that all units are treated as "persistent" modules,
/// and strings don't need to be copied to RAM until needed.
typedef struct SHUnit {
  /// Whether the data is dirty and must be re-initialized before reuse.
  bool dirty;
  /// Currently registered with an active runtime.
  bool in_use;

  /// Number of symbols (strings).
  uint32_t num_symbols;
  /// Size of the property cache.
  uint32_t num_prop_cache_entries;

  /// Pool of ASCII strings.
  const char *ascii_pool;
  /// Pool of 16-bit strings.
  const char16_t *u16_pool;

  /// Triples of (offset, length, hash).
  /// The high bit of offset indicates whether the string is 7-bit ASCII in
  /// ascii_pool (value 0), or 16-bit (value 1) in u16_pool. The offset is in
  /// the corresponding units (char or char16_t).
  ///
  /// The strings are not 0-terminated (and naturally the length doesn't include
  /// a trailing 0). However, for easier debugging the pools *can* contain zero
  /// terminated strings, so a pointer into the pool can be interpreted as a
  /// regular C string. This decrease the chances for string packing.
  const uint32_t *strings;
  /// Symbols populated by the init code from strings above. This pointer is
  /// initialized by the owner of the struct and typically points into BSS.
  SHSymbolID *symbols;
  /// Property cache. Points to a character array of `num_prop_cache_entries *
  /// SH_PROPERTY_CACHE_ENTRY_SIZE`. Must be zeroed initially.
  char *prop_cache;

  /// Unit main function.
  SHLegacyValue (*unit_main)(SHRuntime *shr);
  /// Unit name.
  const char *unit_name;

  /// Data managed by the runtime. Field populated by the runtime.
  SHUnitExt *runtime_ext;
} SHUnit;

#ifdef HERMESVM_COMPRESSED_POINTERS
#define SH_PROPERTY_CACHE_ENTRY_SIZE 8
#else
#define SH_PROPERTY_CACHE_ENTRY_SIZE 16
#endif

/// Create a runtime instance.
SHRuntime *_sh_init(void);
/// Destroy a runtime instance created by \c _sh_init();
void _sh_done(SHRuntime *shr);

/// Register, initialize and execute a main function of the specified unit.
/// The unit is de-initialized when the runtime is destroyed.
/// Execution of the unit initialization code might throw a JS exception.
SHLegacyValue _sh_unit_init(SHRuntime *shr, SHUnit *unit);

/// Initialize all units passed as arguments in order. If a unit throws a
/// JS exception during initialization, print the exception and stop.
///
/// \param count number of units
/// \return false if a unit threw an exception.
bool _sh_initialize_units(SHRuntime *shr, uint32_t count, ...);

/// Given \p templateObjectID, retrieve the cached template object.
/// if it doesn't exist, return a nullptr.
void *_sh_find_cached_template_object(
    const SHUnit *unit,
    uint32_t templateObjID);

/// Cache a template object in the template map using a template object ID as
/// key.
/// \p templateObjID is the template object ID, and it should not already
/// exist in the map.
/// \p templateObj is the template object that we are caching.
void _sh_cache_template_object(
    SHUnit *unit,
    uint32_t templateObjID,
    SHLegacyValue templateObj);

#ifdef __cplusplus
}
#endif

#endif // HERMES_STATIC_H_H
