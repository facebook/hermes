/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ABI_HERMES_ABI_H
#define HERMES_ABI_HERMES_ABI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct HermesABIRuntimeConfig;
struct HermesABIRuntime;
struct HermesABIManagedPointer;
struct HermesABIGrowableBuffer;
struct HermesABIBuffer;
struct HermesABIMutableBuffer;
struct HermesABIHostFunction;
struct HermesABIPropNameIDList;
struct HermesABIHostObject;
struct HermesABINativeState;

/// Define the structure for references to pointer types in JS (e.g. string,
/// object, BigInt).
/// TODO: Replace jsi::PointerValue itself with this C implementation to
/// eliminate pointer management overhead in the JSI wrapper.
struct HermesABIManagedPointerVTable {
  /// Pointer to the function that should be invoked when this reference is
  /// released.
  void (*invalidate)(struct HermesABIManagedPointer *self);
};
struct HermesABIManagedPointer {
  const struct HermesABIManagedPointerVTable *vtable;
};

/// Enum for the types of errors that may be returned. These also indicate how
/// the error information should be retrieved.
enum HermesABIErrorCode {
  HermesABIErrorCodeNativeException,
  HermesABIErrorCodeJSError,
};

#define HERMES_ABI_POINTER_TYPES(V) \
  V(Object)                         \
  V(Array)                          \
  V(String)                         \
  V(BigInt)                         \
  V(Symbol)                         \
  V(Function)                       \
  V(ArrayBuffer)                    \
  V(PropNameID)                     \
  V(WeakObject)

/// For each type of pointer reference that can be held across the ABI, define
/// two structs. The first just wraps a HermesABIManagedPointer * to indicate
/// the type it references. The second allows us to represent a value that is
/// either a pointer or an error, and packs the error code such that the struct
/// is still pointer sized. This works by using the low bit of the pointer to
/// indicate that there is an error, since we know that the pointer is aligned
/// to the word size.
/// The second lowest bit is reserved for future use. If the low bit is set, the
/// error code can be obtained by right shifting ptr_or_error by 2.

#define DECLARE_HERMES_ABI_POINTER_TYPE(name) \
  struct HermesABI##name {                    \
    struct HermesABIManagedPointer *pointer;  \
  };                                          \
  struct HermesABI##name##OrError {           \
    uintptr_t ptr_or_error;                   \
  };

HERMES_ABI_POINTER_TYPES(DECLARE_HERMES_ABI_POINTER_TYPE)
#undef DECLARE_HERMES_ABI_POINTER_TYPE

/// Define the return type for functions that may return void or an error code.
/// This uses the same scheme as pointers, where the low bit indicates whether
/// there was an error, and the remaining bits hold the error code.
struct HermesABIVoidOrError {
  uintptr_t void_or_error;
};

/// Define a struct for holding a boolean value. Similar to the above, the low
/// bit is used to indicate whether there was an error, and the remaining bits
/// hold either the boolean value or the error code.
struct HermesABIBoolOrError {
  uintptr_t bool_or_error;
};

/// Define a struct for holding either a uint8_t* or an error code. Note that
/// this requires a separate field to disambiguate errors, since there are no
/// alignment bits available in the pointer.
struct HermesABIUint8PtrOrError {
  bool is_error;
  union {
    uint8_t *val;
    uint16_t error;
  } data;
};

/// Define a struct for holding either a size_t or an error code.
struct HermesABISizeTOrError {
  bool is_error;
  union {
    size_t val;
    uint16_t error;
  } data;
};

/// Similar to the pointer types, PropNameIDListPtr is known to always point to
/// a word aligned type, so we can pack the error message using the same
/// scheme.
struct HermesABIPropNameIDListPtrOrError {
  uintptr_t ptr_or_error;
};

/// Always set the top bit for pointers so they can be easily checked.
#define HERMES_ABI_POINTER_MASK (1 << (sizeof(int) * 8 - 1))

/// Enum for the types of JavaScript values that can be represented in the ABI.
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

/// Struct representing a JavaScript value. This owns the reference to any
/// HermesABIManagedPointer, and must be explicitly released when no longer
/// needed. For efficiency, the error tag and code are part of the
/// representation, but this type should never be used when an error is
/// possible, use HermesABIValueOrError instead.
struct HermesABIValue {
  enum HermesABIValueKind kind;
  union {
    bool boolean;
    double number;
    struct HermesABIManagedPointer *pointer;
    enum HermesABIErrorCode error;
  } data;
};

/// Struct for representing either a HermesABIValue or an error. The underlying
/// representation is exactly the same as HermesABIValue, so this is purely to
/// provide type safety.
struct HermesABIValueOrError {
  struct HermesABIValue value;
};

/// Define a growable byte buffer that can be used to pass binary data and
/// strings. This allows the user of the C-API to wrap their own resizable
/// buffer and provide it to the API implementation so that data of variable
/// length can be passed without requiring an additional copy.
/// For example, writing to the buffer is typically done as follows:
///   if (buf->size < numBytes) {
///     buf->vtable->grow_to(buf, numBytes);
///     if (buf->size < numBytes)
///       fatal("Failed to allocate memory");
///   }
///   memcpy(buf->data, data, numBytes);
///   buf->used = numBytes;
struct HermesABIGrowableBufferVTable {
  /// Grow the buffer to the specified size. It may not acquire the full
  /// amount, so a caller should check the new size. This can only be used to
  /// grow the buffer, values smaller than the current size will have no effect.
  void (*grow_to)(struct HermesABIGrowableBuffer *buf, size_t sz);
};
struct HermesABIGrowableBuffer {
  const struct HermesABIGrowableBufferVTable *vtable;
  /// The current pointer to the buffer data. This may be updated by a call to
  /// grow_to.
  uint8_t *data;
  /// The total size of the buffer in bytes.
  size_t size;
  /// The number of bytes currently used.
  size_t used;
};

/// Define the structure for buffers containing JS source or bytecode. This is
/// designed to mirror the functionality of jsi::Buffer.
struct HermesABIBufferVTable {
  void (*release)(struct HermesABIBuffer *self);
};
struct HermesABIBuffer {
  const struct HermesABIBufferVTable *vtable;
  const uint8_t *data;
  size_t size;
};

/// Define the structure for buffers mutable buffers used to share data with
/// JavaScript. The data and size fields must not be modified after allocation.
/// The contents of the buffer may be modified by the user or the runtime and
/// the user must ensure that access is properly synchronized.
struct HermesABIMutableBufferVTable {
  void (*release)(struct HermesABIMutableBuffer *self);
};
struct HermesABIMutableBuffer {
  const struct HermesABIMutableBufferVTable *vtable;
  uint8_t *data;
  size_t size;
};

/// Define the structure for host functions. This is designed to recreate the
/// functionality of jsi::HostFunction.
struct HermesABIHostFunctionVTable {
  void (*release)(struct HermesABIHostFunction *);
  struct HermesABIValueOrError (*call)(
      struct HermesABIHostFunction *self,
      struct HermesABIRuntime *rt,
      const struct HermesABIValue *this_arg,
      const struct HermesABIValue *args,
      size_t arg_count);
};
struct HermesABIHostFunction {
  const struct HermesABIHostFunctionVTable *vtable;
};

/// Define the structure for lists of PropNameIDs, so that they can be returned
/// by get_property_names on a HostObject.
struct HermesABIPropNameIDListVTable {
  void (*release)(struct HermesABIPropNameIDList *);
};
struct HermesABIPropNameIDList {
  const struct HermesABIPropNameIDListVTable *vtable;
  const struct HermesABIPropNameID *props;
  size_t size;
};

/// Define the structure for host objects. This is designed to recreate the
/// functionality of jsi::HostObject.
struct HermesABIHostObjectVTable {
  void (*release)(struct HermesABIHostObject *);
  struct HermesABIValueOrError (*get)(
      struct HermesABIHostObject *self,
      struct HermesABIRuntime *rt,
      struct HermesABIPropNameID name);
  struct HermesABIVoidOrError (*set)(
      struct HermesABIHostObject *self,
      struct HermesABIRuntime *rt,
      struct HermesABIPropNameID name,
      const struct HermesABIValue *value);
  struct HermesABIPropNameIDListPtrOrError (*get_property_names)(
      struct HermesABIHostObject *self,
      struct HermesABIRuntime *rt);
};
struct HermesABIHostObject {
  const struct HermesABIHostObjectVTable *vtable;
};

struct HermesABINativeStateVTable {
  void (*release)(struct HermesABINativeState *self);
};
struct HermesABINativeState {
  const struct HermesABINativeStateVTable *vtable;
};

struct HermesABIRuntimeVTable {
  /// Release the given runtime.
  void (*release)(struct HermesABIRuntime *);

  /// Methods for retrieving and clearing exceptions. An exception should be
  /// retrieved if and only if some method returned an error value.
  /// Get and clear the stored JS exception value. This should be called exactly
  /// once after an exception is thrown.
  struct HermesABIValue (*get_and_clear_js_error_value)(
      struct HermesABIRuntime *rt);
  /// Get and clear the stored native exception message. The message is UTF-8
  /// encoded.
  void (*get_and_clear_native_exception_message)(
      struct HermesABIRuntime *rt,
      struct HermesABIGrowableBuffer *msg_buf);

  /// Set the current error before returning control to the ABI. These are
  /// intended to be used to throw exceptions from HostFunctions and
  /// HostObjects.
  /// Report a JavaScript exception with the given value.
  void (*set_js_error_value)(
      struct HermesABIRuntime *rt,
      const struct HermesABIValue *error_value);
  /// Report a native exception with the given UTF-8 message.
  void (*set_native_exception_message)(
      struct HermesABIRuntime *rt,
      const uint8_t *utf8,
      size_t length);

  struct HermesABIPropNameID (*clone_propnameid)(
      struct HermesABIRuntime *rt,
      struct HermesABIPropNameID name);
  struct HermesABIString (
      *clone_string)(struct HermesABIRuntime *rt, struct HermesABIString str);
  struct HermesABISymbol (
      *clone_symbol)(struct HermesABIRuntime *rt, struct HermesABISymbol sym);
  struct HermesABIObject (
      *clone_object)(struct HermesABIRuntime *rt, struct HermesABIObject obj);
  struct HermesABIBigInt (*clone_bigint)(
      struct HermesABIRuntime *rt,
      struct HermesABIBigInt bigint);

  /// Evaluate the given JavaScript source with an associated source URL in the
  /// given runtime, and return the result. The buffer must have a past-the-end
  /// null terminator.
  struct HermesABIValueOrError (*evaluate_javascript_source)(
      struct HermesABIRuntime *rt,
      struct HermesABIBuffer *buf,
      const char *source_url,
      size_t source_url_len);

  /// Evaluate the given Hermes bytecode with an associated source URL in the
  /// given runtime, and return the result. No validation is performed on the
  /// bytecode, so the caller must ensure it is valid.
  struct HermesABIValueOrError (*evaluate_hermes_bytecode)(
      struct HermesABIRuntime *rt,
      struct HermesABIBuffer *buf,
      const char *source_url,
      size_t source_url_len);

  /// Obtain a reference to the global object.
  struct HermesABIObject (*get_global_object)(struct HermesABIRuntime *rt);

  /// Create a JavaScript string from the given UTF-8 encoded string.
  struct HermesABIStringOrError (*create_string_from_utf8)(
      struct HermesABIRuntime *rt,
      const uint8_t *utf8,
      size_t len);

  /// Create a new empty JavaScript object and return a reference to it.
  struct HermesABIObjectOrError (*create_object)(struct HermesABIRuntime *rt);

  /// Check if an object has the given property.
  struct HermesABIBoolOrError (*has_object_property_from_string)(
      struct HermesABIRuntime *rt,
      struct HermesABIObject obj,
      struct HermesABIString str);
  struct HermesABIBoolOrError (*has_object_property_from_propnameid)(
      struct HermesABIRuntime *rt,
      struct HermesABIObject obj,
      struct HermesABIPropNameID name);

  /// Get a property with the given name from an object.
  struct HermesABIValueOrError (*get_object_property_from_string)(
      struct HermesABIRuntime *rt,
      struct HermesABIObject obj,
      struct HermesABIString str);
  struct HermesABIValueOrError (*get_object_property_from_propnameid)(
      struct HermesABIRuntime *rt,
      struct HermesABIObject obj,
      struct HermesABIPropNameID name);

  /// Set a property with the given name on an object to the given value.
  struct HermesABIVoidOrError (*set_object_property_from_string)(
      struct HermesABIRuntime *rt,
      struct HermesABIObject obj,
      struct HermesABIString str,
      const struct HermesABIValue *value);
  struct HermesABIVoidOrError (*set_object_property_from_propnameid)(
      struct HermesABIRuntime *rt,
      struct HermesABIObject obj,
      struct HermesABIPropNameID name,
      const struct HermesABIValue *value);

  /// Get the names of all enumerable string properties on the given object.
  struct HermesABIArrayOrError (*get_object_property_names)(
      struct HermesABIRuntime *rt,
      struct HermesABIObject obj);

  /// Inform the runtime that there is additional memory associated with a given
  /// JavaScript object that is not visible to the GC. This can be used if an
  /// object is known to retain some native memory, and may be used to guide
  /// decisions about when to run garbage collection.
  /// This method may be invoked multiple times on an object, and subsequent
  /// calls will overwrite any previously set value. Once the object is garbage
  /// collected, the associated external memory will be considered freed and may
  /// no longer factor into GC decisions.
  struct HermesABIVoidOrError (*set_object_external_memory_pressure)(
      struct HermesABIRuntime *rt,
      struct HermesABIObject obj,
      size_t amount);

  /// Create a new JS array with the given length and return a reference to it.
  struct HermesABIArrayOrError (
      *create_array)(struct HermesABIRuntime *rt, size_t length);

  /// Get the length of the given array by reading its .length property.
  size_t (*get_array_length)(
      struct HermesABIRuntime *rt,
      struct HermesABIArray arr);

  /// Get the value at the specified index in the given array.
  struct HermesABIValueOrError (*get_array_value_at_index)(
      struct HermesABIRuntime *rt,
      struct HermesABIArray arr,
      size_t index);

  /// Set the value at a specified index in the given array to the given value.
  struct HermesABIVoidOrError (*set_array_value_at_index)(
      struct HermesABIRuntime *rt,
      struct HermesABIArray arr,
      size_t index,
      const struct HermesABIValue *value);

  /// Create an ArrayBuffer that is backed by the given buffer. This allows
  /// native and JS code to efficiently share data, since both can read and
  /// write it.
  struct HermesABIArrayBufferOrError (*create_arraybuffer_from_external_data)(
      struct HermesABIRuntime *rt,
      struct HermesABIMutableBuffer *buf);

  /// Get a pointer to the underlying data for the given ArrayBuffer.
  struct HermesABIUint8PtrOrError (*get_arraybuffer_data)(
      struct HermesABIRuntime *rt,
      struct HermesABIArrayBuffer ab);

  /// Get the size of the ArrayBuffer storage. This is not affected by
  /// overriding the byteLength property.
  struct HermesABISizeTOrError (*get_arraybuffer_size)(
      struct HermesABIRuntime *rt,
      struct HermesABIArrayBuffer ab);

  /// Create a new PropNameID from the given string or symbol.
  struct HermesABIPropNameIDOrError (*create_propnameid_from_string)(
      struct HermesABIRuntime *rt,
      struct HermesABIString str);
  struct HermesABIPropNameIDOrError (*create_propnameid_from_symbol)(
      struct HermesABIRuntime *rt,
      struct HermesABISymbol sym);

  /// Return true if the two PropNameIDs are equal, false otherwise.
  bool (*prop_name_id_equals)(
      struct HermesABIRuntime *rt,
      struct HermesABIPropNameID a,
      struct HermesABIPropNameID b);

  /// Call the function \p fn with \p arg_count \p args, and with the the this
  /// parameter set to \p js_this.
  struct HermesABIValueOrError (*call)(
      struct HermesABIRuntime *rt,
      struct HermesABIFunction fn,
      const struct HermesABIValue *js_this,
      const struct HermesABIValue *args,
      size_t arg_count);

  /// Call the function \p fn as a constructor with \p arg_count \p args.
  /// Equivalent to invoking the function with `new`.
  struct HermesABIValueOrError (*call_as_constructor)(
      struct HermesABIRuntime *rt,
      struct HermesABIFunction fn,
      const struct HermesABIValue *args,
      size_t arg_count);

  struct HermesABIFunctionOrError (*create_function_from_host_function)(
      struct HermesABIRuntime *rt,
      struct HermesABIPropNameID name,
      unsigned int length,
      struct HermesABIHostFunction *hf);
  struct HermesABIHostFunction *(*get_host_function)(
      struct HermesABIRuntime *rt,
      struct HermesABIFunction fn);

  struct HermesABIObjectOrError (*create_object_from_host_object)(
      struct HermesABIRuntime *rt,
      struct HermesABIHostObject *ho);
  struct HermesABIHostObject *(*get_host_object)(
      struct HermesABIRuntime *rt,
      struct HermesABIObject obj);

  struct HermesABINativeState *(*get_native_state)(
      struct HermesABIRuntime *rt,
      struct HermesABIObject obj);
  struct HermesABIVoidOrError (*set_native_state)(
      struct HermesABIRuntime *rt,
      struct HermesABIObject obj,
      struct HermesABINativeState *ns);

  bool (*object_is_array)(
      struct HermesABIRuntime *rt,
      struct HermesABIObject obj);
  bool (*object_is_arraybuffer)(
      struct HermesABIRuntime *rt,
      struct HermesABIObject obj);
  bool (*object_is_function)(
      struct HermesABIRuntime *rt,
      struct HermesABIObject obj);

  struct HermesABIWeakObjectOrError (*create_weak_object)(
      struct HermesABIRuntime *rt,
      struct HermesABIObject obj);
  struct HermesABIValue (*lock_weak_object)(
      struct HermesABIRuntime *rt,
      struct HermesABIWeakObject wo);

  void (*get_utf8_from_string)(
      struct HermesABIRuntime *rt,
      struct HermesABIString str,
      struct HermesABIGrowableBuffer *buf);
  void (*get_utf8_from_propnameid)(
      struct HermesABIRuntime *rt,
      struct HermesABIPropNameID name,
      struct HermesABIGrowableBuffer *buf);
  void (*get_utf8_from_symbol)(
      struct HermesABIRuntime *rt,
      struct HermesABISymbol sym,
      struct HermesABIGrowableBuffer *buf);

  struct HermesABIBoolOrError (*instance_of)(
      struct HermesABIRuntime *rt,
      struct HermesABIObject obj,
      struct HermesABIFunction ctor);

  bool (*strict_equals_symbol)(
      struct HermesABIRuntime *rt,
      struct HermesABISymbol a,
      struct HermesABISymbol b);
  bool (*strict_equals_bigint)(
      struct HermesABIRuntime *rt,
      struct HermesABIBigInt a,
      struct HermesABIBigInt b);
  bool (*strict_equals_string)(
      struct HermesABIRuntime *rt,
      struct HermesABIString a,
      struct HermesABIString b);
  bool (*strict_equals_object)(
      struct HermesABIRuntime *rt,
      struct HermesABIObject a,
      struct HermesABIObject b);

  struct HermesABIBoolOrError (
      *drain_microtasks)(struct HermesABIRuntime *rt, int max_hint);
};

/// An instance of a Hermes Runtime.
struct HermesABIRuntime {
  const struct HermesABIRuntimeVTable *vt;
};

struct HermesABIVTable {
  /// Create a new instance of a Hermes Runtime, and return a pointer to it. The
  /// runtime must be explicitly released when it is no longer needed.
  struct HermesABIRuntime *(*make_hermes_runtime)(
      const struct HermesABIRuntimeConfig *config);

  /// Check if the given buffer contains Hermes bytecode.
  bool (*is_hermes_bytecode)(const uint8_t *buf, size_t len);
};

#endif
