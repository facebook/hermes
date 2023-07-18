/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ABI_HERMES_ABI_H
#define HERMES_ABI_HERMES_ABI_H

#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

#ifndef HERMES_EXPORT
#ifdef _MSC_VER
#define HERMES_EXPORT __declspec(dllexport)
#else // _MSC_VER
#define HERMES_EXPORT __attribute__((visibility("default")))
#endif // _MSC_VER
#endif // !defined(HERMES_EXPORT)

#ifdef __cplusplus
extern "C" {
#endif

/// Define the structure for references to pointer types in JS (e.g. string,
/// object, BigInt).
/// TODO: Replace jsi::PointerValue itself with this C implementation to
/// eliminate pointer management overhead in the JSI wrapper.
struct HermesABIManagedPointer;
struct HermesABIManagedPointerVTable {
  /// Pointer to the function that should be invoked when this reference is
  /// released.
  void (*invalidate)(HermesABIManagedPointer *);
};
struct HermesABIManagedPointer {
  const HermesABIManagedPointerVTable *vtable;
};

enum HermesABIErrorCode {
  HermesABIErrorCodeJSINativeException,
  HermesABIErrorCodeJSError,
};

static_assert(
    alignof(HermesABIManagedPointer) % 4 == 0,
    "HermesABIManagedPointer must be at least aligned to pointer size.");

/// Define simple wrappers for the different pointer types.
#define HERMES_ABI_POINTER_TYPES(V) \
  V(Object)                         \
  V(Array)                          \
  V(String)                         \
  V(BigInt)                         \
  V(Symbol)                         \
  V(Function)                       \
  V(ArrayBuffer)                    \
  V(PropNameID)

#define DECLARE_HERMES_ABI_POINTER_TYPE(name) \
  struct HermesABI##name {                    \
    HermesABIManagedPointer *pointer;         \
  };                                          \
  struct HermesABI##name##OrError {           \
    uintptr_t ptrOrError;                     \
  };

HERMES_ABI_POINTER_TYPES(DECLARE_HERMES_ABI_POINTER_TYPE)
#undef DECLARE_HERMES_ABI_POINTER_TYPE

#define HERMES_ABI_TRIVIAL_OR_ERROR_TYPES(V) \
  V(Bool, bool)                              \
  V(Uint64, uint64_t)                        \
  V(Uint8Ptr, uint8_t *)                     \
  V(SizeT, size_t)

#define DECLARE_HERMES_ABI_TRIVIAL_OR_ERROR_TYPES(name, type) \
  struct HermesABI##name##OrError {                           \
    bool is_error;                                            \
    union {                                                   \
      type val;                                               \
      uint16_t error;                                         \
    } data;                                                   \
  };

HERMES_ABI_TRIVIAL_OR_ERROR_TYPES(DECLARE_HERMES_ABI_TRIVIAL_OR_ERROR_TYPES)
#undef DECLARE_HERMES_ABI_TRIVIAL_OR_ERROR_TYPES

struct HermesABIVoidOrError {
  bool is_error;
  uint16_t error;
};

/// Always set the top bit for pointers so they can be easily checked.
#define HERMES_ABI_POINTER_MASK (1 << (sizeof(int) * 8 - 1))

enum HermesABIValueKind {
  HermesABIValueKindUndefined = 0,
  HermesABIValueKindNull = 1,
  HermesABIValueKindBoolean = 2,
  HermesABIValueKindError = 3,
  HermesABIValueKindNumber = 4,
  HermesABIValueKindSymbol = 5 | HERMES_ABI_POINTER_MASK,
  HermesABIValueKindBigInt = 6 | HERMES_ABI_POINTER_MASK,
  HermesABIValueKindString = 7 | HERMES_ABI_POINTER_MASK,
  HermesABIValueKindObject = 9 | HERMES_ABI_POINTER_MASK,
};

struct HermesABIValue {
  HermesABIValueKind kind;
  union {
    bool boolean;
    double number;
    HermesABIManagedPointer *pointer;
    HermesABIErrorCode error;
  } data;
};

/// Define the structure for buffers containing JS source or bytecode. This is
/// designed to fully mirror the functionality of jsi::Buffer.
struct HermesABIBuffer;
struct HermesABIBufferVTable {
  size_t (*size)(const HermesABIBuffer *);
  const uint8_t *(*data)(const HermesABIBuffer *);
  void (*release)(HermesABIBuffer *);
};
struct HermesABIBuffer {
  const HermesABIBufferVTable *vtable;
};

struct HermesABIContext;

struct HermesABIVTable {
  /// Create a new instance of a Hermes Runtime, and return a pointer to its
  /// associated context. The context must be released with
  /// release_hermes_runtime when it is no longer needed.
  HermesABIContext *(*make_hermes_runtime)();
  /// Release the Hermes Runtime associated with the given context.
  void (*release_hermes_runtime)(HermesABIContext *);

  /// Methods for retrieving and clearing exceptions. An exception should be
  /// retrieved if and only if some method returned an error value.
  /// Get and clear the stored JS exception value. The returned value is
  /// guaranteed to not be another exception. This should be called exactly once
  /// after an exception is thrown.
  HermesABIValue (*get_and_clear_js_error_value)(HermesABIContext *);
  const char *(*get_native_exception_message)(HermesABIContext *);
  /// Clear the stored native exception message. This should be called exactly
  /// once after the message is retrieved.
  void (*clear_native_exception_message)(HermesABIContext *);

  /// Check if the given buffer contains Hermes bytecode.
  bool (*is_hermes_bytecode)(const uint8_t *, size_t);

  /// Evaluate the given JavaScript buffer with an associated source URL in the
  /// given context, and return the result.
  HermesABIValue (*evaluate_javascript)(
      HermesABIContext *,
      HermesABIBuffer *,
      const char *);
};

HERMES_EXPORT const HermesABIVTable *get_hermes_abi_vtable();

#ifdef __cplusplus
}
#endif

#endif
