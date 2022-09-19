/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_STATIC_H_H
#define HERMES_STATIC_H_H

#include "hermes/VM/sh_legacy_value.h"

#include <setjmp.h>
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

typedef struct SHLocals {
  struct SHLocals *prev;
  unsigned count;
  SHLegacyValue locals[0];
} SHLocals;

typedef struct SHJmpBuf {
  struct SHJmpBuf *prev;
  jmp_buf buf;
} SHJmpBuf;

/// Create a runtime instance.
SHRuntime *_sh_init(void);
/// Destroy a runtime instance created by \c _sh_init();
void _sh_done(SHRuntime *shr);

/// Register, initialize and execute a main function of the specified unit.
/// The unit is de-initialized when the runtime is destroyed.
/// Execution of the unit initialization code might throw a JS exception.
SHLegacyValue _sh_unit_init(SHRuntime *shr, SHUnit *unit);

/// Execute \c _sh_unit_init and catch JS exceptions. \p resultOrExc is
/// initialized to either the returned value or the thrown exception.
///
/// \return false if an exception was thrown.
bool _sh_unit_init_guarded(
    SHRuntime *shr,
    SHUnit *unit,
    SHLegacyValue *resultOrExc);

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

/// Add the locals to the gc list, allocate register stack, return a pointer to
/// the frame.
SHLegacyValue *_sh_enter(SHRuntime *shr, SHLocals *locals, uint32_t stackSize);
void _sh_leave(SHRuntime *shr, SHLocals *locals, SHLegacyValue *frame);

/// Add a locals struct to the root stack, allocate the requested number of
/// registers and return the previous register stack pointer.
SHLegacyValue *
_sh_push_locals(SHRuntime *shr, SHLocals *locals, uint32_t stackSize);
/// Pop the locals (which must be current), restore the register stack.
void _sh_pop_locals(SHRuntime *shr, SHLocals *locals, SHLegacyValue *savedSP);

/// Index 0 loads "this", 1 the first param, etc.
SHLegacyValue _sh_ljs_param(SHLegacyValue *frame, uint32_t index);

/// Obtain the raw "this" value and coerce it to an object.
SHLegacyValue _sh_ljs_load_this_ns(SHRuntime *shr, SHLegacyValue *frame);

/// Allocate an empty, uninitialized object (immediately before a constructor).
SHLegacyValue _sh_ljs_create_this(
    SHRuntime *shr,
    SHLegacyValue *prototype,
    SHLegacyValue *callable);

#define _sh_try(shr, jbuf) (_sh_push_try(shr, jbuf), _setjmp((jbuf)->buf))
void _sh_push_try(SHRuntime *shr, SHJmpBuf *buf);
void _sh_end_try(SHRuntime *shr, SHJmpBuf *prev);

/// \param frame the value that should be set to the current frame
///     (Runtime::currentFrame_).
/// \param stackSize <tt>frame + stackSize</tt> will be the new value of the
///     register stack pointer.
SHLegacyValue _sh_catch(
    SHRuntime *shr,
    SHLocals *locals,
    SHLegacyValue *frame,
    uint32_t stackSize);
void _sh_throw_current(SHRuntime *shr) __attribute__((noreturn));
void _sh_throw(SHRuntime *shr, SHLegacyValue value) __attribute__((noreturn));

/// Performs a function call. The new frame is at the top of the stack.
/// Arguments, this, and callee must be populated.
SHLegacyValue
_sh_ljs_call(SHRuntime *shr, SHLegacyValue *frame, uint32_t argCount);

/// Performs a function call. The new frame is at the top of the stack.
/// Arguments, this, callee, and "new.target" must be populated.
SHLegacyValue
_sh_ljs_construct(SHRuntime *shr, SHLegacyValue *frame, uint32_t argCount);

/// Create a new environment with the specified size and the current function's
/// environment as parent.
/// \p result will contain the result on exit, but is also used as a temporary
///     (thus is not `const`).
void _sh_ljs_create_environment(
    SHRuntime *shr,
    SHLegacyValue *frame,
    SHLegacyValue *result,
    uint32_t size);

/// Get the enclosing LJS environment, \p level levels up, where 0 represents
/// the environment of the current function.
/// TODO: implement a "raw" version of this function returning a raw pointer,
///       as well as raw version of the other env-related functions, as soon
///       as we have the ability to mark unencoded pointers.
SHLegacyValue
_sh_ljs_get_env(SHRuntime *shr, SHLegacyValue *frame, uint32_t level);

SHLegacyValue _sh_ljs_load_from_env(SHLegacyValue env, uint32_t index);

void _sh_ljs_store_to_env(
    SHRuntime *shr,
    SHLegacyValue env,
    SHLegacyValue val,
    uint32_t index);

/// Same as _sh_ljs_store_to_env(), but \p value is known to not be a pointer
/// (though the old value being overwritten still might be).
void _sh_ljs_store_np_to_env(
    SHRuntime *shr,
    SHLegacyValue env,
    SHLegacyValue val,
    uint32_t index);

/// \param env  Should be JSNull if there is no environment.
SHLegacyValue _sh_ljs_create_closure(
    SHRuntime *shr,
    const SHLegacyValue *env,
    SHLegacyValue (*func)(SHRuntime *),
    SHSymbolID name,
    uint32_t paramCount);

SHLegacyValue _sh_ljs_get_global_object(SHRuntime *shr);
void _sh_ljs_declare_global_var(SHRuntime *shr, SHSymbolID name);

void _sh_ljs_put_by_id_loose_rjs(
    SHRuntime *shr,
    const SHLegacyValue *target,
    SHSymbolID symID,
    const SHLegacyValue *value,
    char *propCacheEntry);
void _sh_ljs_put_by_id_strict_rjs(
    SHRuntime *shr,
    const SHLegacyValue *target,
    SHSymbolID symID,
    const SHLegacyValue *value,
    char *propCacheEntry);

SHLegacyValue _sh_ljs_try_get_by_id_rjs(
    SHRuntime *shr,
    const SHLegacyValue *source,
    SHSymbolID symID,
    char *propCacheEntry);
SHLegacyValue _sh_ljs_get_by_id_rjs(
    SHRuntime *shr,
    const SHLegacyValue *source,
    SHSymbolID symID,
    char *propCacheEntry);

double _sh_ljs_to_double_rjs(SHRuntime *shr, const SHLegacyValue *n);

bool _sh_ljs_less_rjs(
    SHRuntime *shr,
    const SHLegacyValue *a,
    const SHLegacyValue *b);
bool _sh_ljs_greater_rjs(
    SHRuntime *shr,
    const SHLegacyValue *a,
    const SHLegacyValue *b);
bool _sh_ljs_less_equal_rjs(
    SHRuntime *shr,
    const SHLegacyValue *a,
    const SHLegacyValue *b);
bool _sh_ljs_greater_equal_rjs(
    SHRuntime *shr,
    const SHLegacyValue *a,
    const SHLegacyValue *b);

SHLegacyValue
_sh_ljs_add_rjs(SHRuntime *shr, const SHLegacyValue *a, const SHLegacyValue *b);

#ifdef __cplusplus
}
#endif

#endif // HERMES_STATIC_H_H
