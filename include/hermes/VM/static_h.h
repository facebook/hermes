/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_STATIC_H_H
#define HERMES_STATIC_H_H

#include "hermes/VM/sh_legacy_value.h"

#include <math.h>
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

  /// Object key buffer.
  const unsigned char *obj_key_buffer;
  /// Size of object key buffer.
  uint32_t obj_key_buffer_size;
  /// Object value buffer.
  const unsigned char *obj_val_buffer;
  /// Size of object value buffer.
  uint32_t obj_val_buffer_size;
  /// Array value buffer.
  const unsigned char *array_buffer;
  /// Size of array value buffer.
  uint32_t array_buffer_size;

  /// Unit main function.
  SHLegacyValue (*unit_main)(SHRuntime *shr);
  /// Unit name.
  const char *unit_name;

  /// Data managed by the runtime. Field populated by the runtime.
  SHUnitExt *runtime_ext;
} SHUnit;

#if defined(HERMESVM_COMPRESSED_POINTERS) || HERMESVM_SIZEOF_VOID_P == 4
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
SHRuntime *_sh_init(int argc, char **argv);
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

void _sh_cache_object_literal_hidden_class(
    SHRuntime *shr,
    const SHUnit *unit,
    uint32_t keyBufferIndex,
    SHLegacyValue clazz);

void *_sh_find_object_literal_hidden_class(
    SHRuntime *shr,
    const SHUnit *unit,
    uint32_t numLiterals,
    uint32_t keyBufferIndex);

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

/// Coerce a value assumed to contain 'this' to an object using non-strict
/// mode rules. Primitives are boxed, \c null or \c undefed produce the global
/// object.
SHLegacyValue _sh_ljs_coerce_this_ns(SHRuntime *shr, SHLegacyValue value);

/// `arguments` operations all work with a lazy "register" that contains either
/// undefined or a reified array. The first call to _sh_reify_arguments_* will
/// populate the reified array. This is an optimization to allow arguments[i] to
/// just load an argument instead of doing a full array allocation and property
/// lookup.

/// Get a property of the 'arguments' array by value.
SHLegacyValue _sh_ljs_get_arguments_prop_by_val_loose(
    SHRuntime *shr,
    SHLegacyValue *frame,
    SHLegacyValue *idx,
    SHLegacyValue *lazyReg);
SHLegacyValue _sh_ljs_get_arguments_prop_by_val_strict(
    SHRuntime *shr,
    SHLegacyValue *frame,
    SHLegacyValue *idx,
    SHLegacyValue *lazyReg);

/// Get the length of the 'arguments' array.
SHLegacyValue _sh_ljs_get_arguments_length(
    SHRuntime *shr,
    SHLegacyValue *frame,
    SHLegacyValue *lazyReg);

/// Ensure that the reified array has been created, updating \p lazyReg in place
/// if it has not.
void _sh_ljs_reify_arguments_loose(
    SHRuntime *shr,
    SHLegacyValue *frame,
    SHLegacyValue *lazyReg);
void _sh_ljs_reify_arguments_strict(
    SHRuntime *shr,
    SHLegacyValue *frame,
    SHLegacyValue *lazyReg);

/// Allocate an empty, uninitialized object (immediately before a constructor).
SHLegacyValue _sh_ljs_create_this(
    SHRuntime *shr,
    SHLegacyValue *prototype,
    SHLegacyValue *callable);

#define _sh_try(shr, jbuf) (_sh_push_try(shr, jbuf), _setjmp((jbuf)->buf))
void _sh_push_try(SHRuntime *shr, SHJmpBuf *buf);
void _sh_end_try(SHRuntime *shr);

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

/// Performs a function call. The new frame is at the top of the stack.
/// The arguments (excluding 'this') must be populated.
SHLegacyValue _sh_ljs_call_builtin(
    SHRuntime *shr,
    SHLegacyValue *frame,
    uint32_t argCount,
    uint32_t builtinMethodID);

SHLegacyValue _sh_ljs_get_builtin_closure(
    SHRuntime *shr,
    uint32_t builtinMethodID);

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
void _sh_ljs_try_put_by_id_loose_rjs(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHSymbolID symID,
    SHLegacyValue *value,
    char *propCacheEntry);
void _sh_ljs_try_put_by_id_strict_rjs(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHSymbolID symID,
    SHLegacyValue *value,
    char *propCacheEntry);
void _sh_ljs_put_by_val_loose_rjs(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHLegacyValue *key,
    SHLegacyValue *value);
void _sh_ljs_put_by_val_strict_rjs(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHLegacyValue *key,
    SHLegacyValue *value);

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
SHLegacyValue _sh_ljs_get_by_val_rjs(
    SHRuntime *shr,
    SHLegacyValue *source,
    SHLegacyValue *key);

/// Put an enumerable property.
void _sh_ljs_put_own_by_val(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHLegacyValue *key,
    SHLegacyValue *value);
/// Put a non-enumerable property.
void _sh_ljs_put_own_ne_by_val(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHLegacyValue *key,
    SHLegacyValue *value);

void _sh_ljs_put_own_by_index(
    SHRuntime *shr,
    SHLegacyValue *target,
    uint32_t key,
    SHLegacyValue *value);

/// Put an enumerable property.
void _sh_ljs_put_new_own_by_id(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHSymbolID key,
    SHLegacyValue *value);
/// Put a non-enumerable property.
void _sh_ljs_put_new_own_ne_by_id(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHSymbolID key,
    SHLegacyValue *value);

void _sh_ljs_put_own_getter_setter_by_val(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHLegacyValue *key,
    SHLegacyValue *getter,
    SHLegacyValue *setter,
    bool isEnumerable);

SHLegacyValue
_sh_ljs_del_by_id_strict(SHRuntime *shr, SHLegacyValue *target, SHSymbolID key);
SHLegacyValue
_sh_ljs_del_by_id_loose(SHRuntime *shr, SHLegacyValue *target, SHSymbolID key);
SHLegacyValue _sh_ljs_del_by_val_strict(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHLegacyValue *key);
SHLegacyValue _sh_ljs_del_by_val_loose(
    SHRuntime *shr,
    SHLegacyValue *target,
    SHLegacyValue *key);

/// Get the string associated with the given SHSymbolID. The caller is
/// responsible for ensuring that \p symID is not garbage collected.
SHLegacyValue _sh_ljs_get_string(SHRuntime *shr, SHSymbolID symID);

SHLegacyValue
_sh_ljs_create_regexp(SHRuntime *shr, SHSymbolID pattern, SHSymbolID flags);

/// \param value the string of the BigInt.
/// \param size  the size of the string \c value.
SHLegacyValue
_sh_ljs_create_bigint(SHRuntime *shr, const uint8_t *value, uint32_t size);

double _sh_ljs_to_double_rjs(SHRuntime *shr, const SHLegacyValue *n);
bool _sh_ljs_to_boolean(SHLegacyValue b);
SHLegacyValue _sh_ljs_to_numeric_rjs(SHRuntime *shr, const SHLegacyValue *n);
SHLegacyValue _sh_ljs_to_int32_rjs(SHRuntime *shr, const SHLegacyValue *n);

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
bool _sh_ljs_equal_rjs(
    SHRuntime *shr,
    const SHLegacyValue *a,
    const SHLegacyValue *b);
bool _sh_ljs_strict_equal(SHLegacyValue a, SHLegacyValue b);

SHLegacyValue
_sh_ljs_add_rjs(SHRuntime *shr, const SHLegacyValue *a, const SHLegacyValue *b);
SHLegacyValue
_sh_ljs_sub_rjs(SHRuntime *shr, const SHLegacyValue *a, const SHLegacyValue *b);
SHLegacyValue
_sh_ljs_mul_rjs(SHRuntime *shr, const SHLegacyValue *a, const SHLegacyValue *b);
SHLegacyValue
_sh_ljs_div_rjs(SHRuntime *shr, const SHLegacyValue *a, const SHLegacyValue *b);
SHLegacyValue
_sh_ljs_mod_rjs(SHRuntime *shr, const SHLegacyValue *a, const SHLegacyValue *b);
SHLegacyValue
_sh_ljs_is_in(SHRuntime *shr, SHLegacyValue *name, SHLegacyValue *obj);
SHLegacyValue _sh_ljs_instance_of(
    SHRuntime *shr,
    SHLegacyValue *object,
    SHLegacyValue *constructor);

SHLegacyValue _sh_ljs_inc_rjs(SHRuntime *shr, const SHLegacyValue *n);
SHLegacyValue _sh_ljs_dec_rjs(SHRuntime *shr, const SHLegacyValue *n);

SHLegacyValue _sh_ljs_bit_or_rjs(
    SHRuntime *shr,
    const SHLegacyValue *a,
    const SHLegacyValue *b);
SHLegacyValue _sh_ljs_bit_and_rjs(
    SHRuntime *shr,
    const SHLegacyValue *a,
    const SHLegacyValue *b);
SHLegacyValue _sh_ljs_bit_xor_rjs(
    SHRuntime *shr,
    const SHLegacyValue *a,
    const SHLegacyValue *b);
SHLegacyValue _sh_ljs_right_shift_rjs(
    SHRuntime *shr,
    const SHLegacyValue *a,
    const SHLegacyValue *b);
SHLegacyValue _sh_ljs_unsigned_right_shift_rjs(
    SHRuntime *shr,
    const SHLegacyValue *a,
    const SHLegacyValue *b);
SHLegacyValue _sh_ljs_left_shift_rjs(
    SHRuntime *shr,
    const SHLegacyValue *a,
    const SHLegacyValue *b);
SHLegacyValue _sh_ljs_bit_not_rjs(SHRuntime *shr, const SHLegacyValue *a);
SHLegacyValue _sh_ljs_minus_rjs(SHRuntime *shr, const SHLegacyValue *n);

SHLegacyValue _sh_ljs_add_empty_string_rjs(
    SHRuntime *shr,
    const SHLegacyValue *a);

SHLegacyValue _sh_ljs_typeof(SHRuntime *shr, SHLegacyValue *v);

SHLegacyValue _sh_ljs_new_object(SHRuntime *shr);
SHLegacyValue _sh_ljs_new_object_with_parent(
    SHRuntime *shr,
    const SHLegacyValue *parent);

/// \p sizeHint the eventual size of the resultant object.
/// \p numLiterals the number of literals to read off the buffer
///   to populate the start of the object.
SHLegacyValue _sh_ljs_new_object_with_buffer(
    SHRuntime *shr,
    SHUnit *unit,
    uint32_t sizeHint,
    uint32_t numLiterals,
    uint32_t keyBufferIndex,
    uint32_t valBufferIndex);

/// \p sizeHint the size of the resultant array.
SHLegacyValue _sh_ljs_new_array(SHRuntime *shr, uint32_t sizeHint);

/// \p numElements the size of the resultant array.
/// \p numLiterals the number of literals to read off the buffer
///   to populate the start of the array.
SHLegacyValue _sh_ljs_new_array_with_buffer(
    SHRuntime *shr,
    SHUnit *unit,
    uint32_t numElements,
    uint32_t numLiterals,
    uint32_t bufferIndex);

/// \p base[in/out] is the object to be iterated.
/// \p index[out] is the next index in the list.
/// \p size[out] is the size of the list.
/// \return the array of properties, or undefined if there is nothing to
///   iterate.
SHLegacyValue _sh_ljs_get_pname_list_rjs(
    SHRuntime *shr,
    SHLegacyValue *base,
    SHLegacyValue *index,
    SHLegacyValue *size);

/// \p props is the array of properties
/// \p base is the object to be iterated.
/// \p index is the iterating index.
/// \p size is the size of the property list.
/// \return the next property, undefined if unavailable
SHLegacyValue _sh_ljs_get_next_pname_rjs(
    SHRuntime *shr,
    SHLegacyValue *props,
    SHLegacyValue *base,
    SHLegacyValue *index,
    SHLegacyValue *size);

/// If \p src is an array with an unmodified [Symbol.iterator], return 0. Else
/// return the Symbol.iterator property of \p src and update \p src in place to
/// the .next() method on the iterator.
SHLegacyValue _sh_ljs_iterator_begin_rjs(SHRuntime *shr, SHLegacyValue *src);

/// Move \p iteratorOrIdx forward, and return the value at the new iterator. \p
/// srcOrNext is the source if we are iterating over an array, or the .next()
/// method otherwise.
SHLegacyValue _sh_ljs_iterator_next_rjs(
    SHRuntime *shr,
    SHLegacyValue *iteratorOrIdx,
    const SHLegacyValue *srcOrNext);

/// If the given iterator is an object, call iterator.return(), ignoring
/// catchable exceptions if ignoreExceptions is true.
void _sh_ljs_iterator_close_rjs(
    SHRuntime *shr,
    const SHLegacyValue *iteratorOrIdx,
    bool ignoreExceptions);

SHLegacyValue _sh_ljs_direct_eval(SHRuntime *shr, SHLegacyValue *input);

/// Run a % b if b != 0, otherwise use `fmod` so the operation can't fail.
/// \return the double representing the result of the JS mod operation on the
///   two integers.
static inline double _sh_mod_int32(int32_t a, int32_t b) {
  if (b == 0) {
    // Avoid the divide-by-zero and return NaN directly.
    return nan("");
  }
  return (double)(a % b);
}

__attribute__((const)) static inline double _sh_mod_double(double a, double b) {
  // This is technically "undefined behavior", but we do this casting to quickly
  // check for integers in Conversions.h as well.
  int32_t aInt = (int32_t)a;
  int32_t bInt = (int32_t)b;

  // If both numbers are integers, use the fast path.
  // `-0` must be handled specially, so just check `a != 0` for simplicity.
  if (a != 0 && (double)aInt == a && (double)bInt == b) {
    return _sh_mod_int32(aInt, bInt);
  }

  // Actually have to do the double operation.
  return fmod(a, b);
}

/// Call the \c hermes::truncateToInt32SlowPath function.
/// Annotated ((const)) to let the compiler know that the function will not
/// read/write global memory at all (it's just doing math), allowing for
/// optimizations around the locals struct.
__attribute__((const)) int32_t _sh_to_int32_double_slow_path(double d);

/// C version of the hermes::truncateToInt32 function.
/// Inlines the fast path for SH to use, calls out to the slow path.
static inline int32_t _sh_to_int32_double(double d) {
  // Check of the value can be converted to integer without loss. We want to
  // use the widest available integer because this conversion will be much
  // faster than the bit-twiddling slow path.
  intmax_t fast = (intmax_t)d;
  if (fast == d)
    return (int32_t)fast;
  return _sh_to_int32_double_slow_path(d);
}

#ifdef __cplusplus
}
#endif

#endif // HERMES_STATIC_H_H
