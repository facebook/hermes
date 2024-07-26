/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "HermesSandboxRuntime.h"

#include "external/hermes_sandbox_impl_compiled.h"
#include "hermes/ADT/ManagedChunkedList.h"
#include "jsi/jsilib.h"

#include <atomic>
#include <chrono>
#include <deque>
#include <random>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

#if __has_builtin(__builtin_unreachable)
#define BUILTIN_UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
#define BUILTIN_UNREACHABLE __assume(false)
#else
#define BUILTIN_UNREACHABLE assert(false);
#endif

#if __has_feature(thread_sanitizer) || defined(__SANITIZE_THREAD__)
#define THREAD_SANITIZER_BUILD 1
#else
#define THREAD_SANITIZER_BUILD 0
#endif

using namespace facebook::jsi;

namespace {

struct SandboxManagedPointer;
struct SandboxBuffer;
struct SandboxGrowableBuffer;
struct SandboxHostFunction;
struct SandboxPropNameIDList;
struct SandboxHostObject;
struct SandboxNativeState;

/// Mirror the types defined in hermes_abi.h but in the form they would have in
/// the sandbox. For instance, pointers and size_t are replaced with uint32.

struct SandboxManagedPointerVTable {
  /// Pointer to the function that should be invoked when this reference is
  /// released.
  /* void (*)(HermesABIManagedPointer *) */ u32 invalidate;
};
struct SandboxManagedPointer {
  /* const HermesABIManagedPointerVTable * */ u32 vtable;
};

enum SandboxErrorCode {
  SandboxErrorCodeNativeException,
  SandboxErrorCodeJSError,
};

static_assert(
    alignof(SandboxManagedPointer) % 4 == 0,
    "SandboxManagedPointer must be at least aligned to pointer size.");

/// Define simple wrappers for the different pointer types.
#define SANDBOX_POINTER_TYPES(V) \
  V(Object)                      \
  V(Array)                       \
  V(String)                      \
  V(BigInt)                      \
  V(Symbol)                      \
  V(Function)                    \
  V(ArrayBuffer)                 \
  V(PropNameID)                  \
  V(WeakObject)

#define DECLARE_SANDBOX_POINTER_TYPE(name)      \
  struct Sandbox##name {                        \
    /* HermesABIManagedPointer* */ u32 pointer; \
  };                                            \
  struct Sandbox##name##OrError {               \
    u32 ptr_or_error;                           \
  };

SANDBOX_POINTER_TYPES(DECLARE_SANDBOX_POINTER_TYPE)
#undef DECLARE_SANDBOX_POINTER_TYPE

struct SandboxVoidOrError {
  u32 void_or_error;
};

struct SandboxBoolOrError {
  u32 bool_or_error;
};

struct SandboxUint8PtrOrError {
  bool is_error;
  union {
    u32 val;
    uint16_t error;
  } data;
};

struct SandboxSizeTOrError {
  bool is_error;
  union {
    u32 val;
    uint16_t error;
  } data;
};

struct SandboxPropNameIDListPtrOrError {
  u32 ptr_or_error;
};

/// Always set the top bit for pointers so they can be easily checked.
#define SANDBOX_POINTER_MASK (1u << 31)

enum SandboxValueKind : uint32_t {
  SandboxValueKindUndefined = 0,
  SandboxValueKindNull = 1,
  SandboxValueKindBoolean = 2,
  SandboxValueKindError = 3,
  SandboxValueKindNumber = 4,
  SandboxValueKindSymbol = 5u | SANDBOX_POINTER_MASK,
  SandboxValueKindBigInt = 6u | SANDBOX_POINTER_MASK,
  SandboxValueKindString = 7u | SANDBOX_POINTER_MASK,
  SandboxValueKindObject = 9u | SANDBOX_POINTER_MASK,
};

struct SandboxValue {
  SandboxValueKind kind;
  union {
    bool boolean;
    alignas(8) double number;
    /* HermesABIManagedPointer* */ u32 pointer;
    SandboxErrorCode error;
  } data;
};
struct SandboxValueOrError {
  SandboxValue value;
};

struct SandboxBufferVTable {
  /* void (*)(HermesABIBuffer *) */ u32 release;
};
struct SandboxBuffer {
  /* const HermesABIBufferVTable* */ u32 vtable;
  /* const uint8_t * */ u32 data;
  /* size_t */ u32 size;
};

struct SandboxGrowableBufferVTable {
  /* void (*)(struct HermesABIGrowableBuffer *, size_t) */ u32 try_grow_to;
};
struct SandboxGrowableBuffer {
  /* const struct HermesABIGrowableBufferVTable * */ u32 vtable;
  /* uint8_t * */ u32 data;
  /* size_t */ u32 size;
  /* size_t */ u32 used;
};

struct SandboxHostFunctionVTable {
  /* void (*)(struct HermesABIHostFunction *) */ u32 release;
  /* struct HermesABIValueOrError (*)(
         struct HermesABIHostFunction *,
         struct HermesABIRuntime *rt,
         const struct HermesABIValue *,
         const struct HermesABIValue *,
         size_t) */
  u32 call;
};
struct SandboxHostFunction {
  /* const struct HermesABIHostFunctionVTable * */ u32 vtable;
};

struct SandboxPropNameIDListVTable {
  /* void (*)(struct HermesABIPropNameIDList *) */ u32 release;
};
struct SandboxPropNameIDList {
  /* const struct HermesABIPropNameIDListVTable * */ u32 vtable;
  /* const struct HermesABIPropNameID * */ u32 props;
  /* size_t */ u32 size;
};

struct SandboxHostObjectVTable {
  /* void (*)(struct HermesABIHostObject *) */ u32 release;
  /* struct HermesABIValueOrError (*)(
         struct HermesABIHostObject *,
         struct HermesABIRuntime *rt,
         struct HermesABIPropNameID) */
  u32 get;
  /* struct HermesABIVoidOrError (*)(
         struct HermesABIHostObject *,
         struct HermesABIRuntime *rt,
         struct HermesABIPropNameID,
         const struct HermesABIValue *) */
  u32 set;
  /* struct HermesABIPropNameIDListPtrOrError (*)(
         struct HermesABIHostObject *,
         struct HermesABIRuntime *) */
  u32 get_own_keys;
};
struct SandboxHostObject {
  /* const struct HermesABIHostObjectVTable * */ u32 vtable;
};

struct SandboxNativeStateVTable {
  /* void (*)(struct HermesABINativeState *) */ u32 release;
};
struct SandboxNativeState {
  /* const struct HermesABINativeStateVTable *vtable */ u32 vtable;
};

/// List all of the vtable functions with their signatures.
#define SANDBOX_CONTEXT_VTABLE_FUNCTIONS(F)                                   \
  F(release, void, (w2c_hermes *, u32 srt))                                   \
  F(get_and_clear_js_error_value, void, (w2c_hermes *, u32 resVal, u32 srt))  \
  F(get_and_clear_native_exception_message,                                   \
    void,                                                                     \
    (w2c_hermes *, u32 srt, u32 buf))                                         \
  F(set_js_error_value, void, (w2c_hermes *, u32 srt, u32 val))               \
  F(set_native_exception_message,                                             \
    void,                                                                     \
    (w2c_hermes *, u32 srt, u32 buf, u32 len))                                \
  F(clone_propnameid, u32, (w2c_hermes *, u32 srt, u32 propnameid))           \
  F(clone_string, u32, (w2c_hermes *, u32 srt, u32 str))                      \
  F(clone_symbol, u32, (w2c_hermes *, u32 srt, u32 sym))                      \
  F(clone_object, u32, (w2c_hermes *, u32 srt, u32 obj))                      \
  F(clone_bigint, u32, (w2c_hermes *, u32 srt, u32 bi))                       \
  F(evaluate_javascript_source,                                               \
    void,                                                                     \
    (w2c_hermes *,                                                            \
     u32 resValOrError,                                                       \
     u32 srt,                                                                 \
     u32 buf,                                                                 \
     u32 sourceUrl,                                                           \
     u32 sourceUrlLength))                                                    \
  F(evaluate_hermes_bytecode,                                                 \
    void,                                                                     \
    (w2c_hermes *,                                                            \
     u32 resValueOrError,                                                     \
     u32 srt,                                                                 \
     u32 buf,                                                                 \
     u32 sourceUrl,                                                           \
     u32 sourceUrlLength))                                                    \
  F(get_global_object, u32, (w2c_hermes *, u32 srt))                          \
  F(create_string_from_utf8, u32, (w2c_hermes *, u32 srt, u32 buf, u32 len))  \
  F(create_object, u32, (w2c_hermes *, u32 srt))                              \
  F(has_object_property_from_value, u32, (w2c_hermes *, u32, u32, u32))       \
  F(has_object_property_from_propnameid, u32, (w2c_hermes *, u32, u32, u32))  \
  F(get_object_property_from_value, void, (w2c_hermes *, u32, u32, u32, u32)) \
  F(get_object_property_from_propnameid,                                      \
    void,                                                                     \
    (w2c_hermes *, u32, u32, u32, u32))                                       \
  F(set_object_property_from_value, u32, (w2c_hermes *, u32, u32, u32, u32))  \
  F(set_object_property_from_propnameid,                                      \
    u32,                                                                      \
    (w2c_hermes *, u32, u32, u32, u32))                                       \
  F(get_object_property_names, u32, (w2c_hermes *, u32, u32))                 \
  F(set_object_external_memory_pressure, u32, (w2c_hermes *, u32, u32, u32))  \
  F(create_array, u32, (w2c_hermes *, u32, u32))                              \
  F(get_array_length, u32, (w2c_hermes *, u32, u32))                          \
  F(create_arraybuffer_from_external_data, u32, (w2c_hermes *, u32, u32))     \
  F(get_arraybuffer_data, void, (w2c_hermes *, u32, u32, u32))                \
  F(get_arraybuffer_size, void, (w2c_hermes *, u32, u32, u32))                \
  F(create_propnameid_from_string, u32, (w2c_hermes *, u32, u32))             \
  F(create_propnameid_from_symbol, u32, (w2c_hermes *, u32, u32))             \
  F(prop_name_id_equals, u32, (w2c_hermes *, u32, u32, u32))                  \
  F(call, void, (w2c_hermes *, u32, u32, u32, u32, u32, u32))                 \
  F(call_as_constructor, void, (w2c_hermes *, u32, u32, u32, u32, u32))       \
  F(create_function_from_host_function,                                       \
    u32,                                                                      \
    (w2c_hermes *, u32, u32, u32, u32))                                       \
  F(get_host_function, u32, (w2c_hermes *, u32, u32))                         \
  F(create_object_from_host_object, u32, (w2c_hermes *, u32, u32))            \
  F(get_host_object, u32, (w2c_hermes *, u32, u32))                           \
  F(get_native_state, u32, (w2c_hermes *, u32, u32))                          \
  F(set_native_state, u32, (w2c_hermes *, u32, u32, u32))                     \
  F(object_is_array, u32, (w2c_hermes *, u32, u32))                           \
  F(object_is_arraybuffer, u32, (w2c_hermes *, u32, u32))                     \
  F(object_is_function, u32, (w2c_hermes *, u32, u32))                        \
  F(create_weak_object, u32, (w2c_hermes *, u32, u32))                        \
  F(lock_weak_object, void, (w2c_hermes *, u32, u32, u32))                    \
  F(get_utf8_from_string, void, (w2c_hermes *, u32, u32, u32))                \
  F(get_utf8_from_propnameid, void, (w2c_hermes *, u32, u32, u32))            \
  F(get_utf8_from_symbol, void, (w2c_hermes *, u32, u32, u32))                \
  F(instance_of, u32, (w2c_hermes *, u32, u32, u32))                          \
  F(strict_equals_symbol, u32, (w2c_hermes *, u32, u32, u32))                 \
  F(strict_equals_bigint, u32, (w2c_hermes *, u32, u32, u32))                 \
  F(strict_equals_string, u32, (w2c_hermes *, u32, u32, u32))                 \
  F(strict_equals_object, u32, (w2c_hermes *, u32, u32, u32))                 \
  F(drain_microtasks, u32, (w2c_hermes *, u32, u32))                          \
  F(create_bigint_from_int64, u32, (w2c_hermes *, u32, u64))                  \
  F(create_bigint_from_uint64, u32, (w2c_hermes *, u32, u64))                 \
  F(bigint_is_int64, u32, (w2c_hermes *, u32, u32))                           \
  F(bigint_is_uint64, u32, (w2c_hermes *, u32, u32))                          \
  F(bigint_truncate_to_uint64, u64, (w2c_hermes *, u32, u32))                 \
  F(bigint_to_string, u32, (w2c_hermes *, u32, u32, u32))

/// Declare the vtable structure as it appears in the sandbox, with u32 as the
/// function pointer type.
struct SandboxRuntimeVTable {
#define DECLARE_SANDBOX_VTABLE_FUNCTION(name, ret, args) u32 name;
  SANDBOX_CONTEXT_VTABLE_FUNCTIONS(DECLARE_SANDBOX_VTABLE_FUNCTION)
#undef DECLARE_SANDBOX_VTABLE_FUNCTION
};

/// Declare the vtable structure with actual C function pointers. The vtable
/// pointers from the SandboxRuntimeVTable are converted and copied into this
/// structure, to make the functions easier and more efficient to call.
struct SandboxRuntimeVTableMirror {
#define DECLARE_SANDBOX_VTABLE_FUNCTION(name, ret, args) ret(*name) args;
  SANDBOX_CONTEXT_VTABLE_FUNCTIONS(DECLARE_SANDBOX_VTABLE_FUNCTION)
#undef DECLARE_SANDBOX_VTABLE_FUNCTION
};

struct SandboxRuntime {
  u32 vtable;
};

struct SandboxVTable {
  /* struct HermesABIRuntime *(*make_hermes_runtime)
     (const struct HermesABIRuntimeConfig *config); */
  u32 make_hermes_runtime;
  /* bool (*is_hermes_bytecode)(const uint8_t *buf, size_t len) */
  u32 is_hermes_bytecode;
};

namespace sb {

/// Mirror the helper functions from HermesABIHelpers.h.
#define DECLARE_SANDBOX_POINTER_HELPERS(name)           \
  Sandbox##name create##name(u32 ptr) {                 \
    return {ptr};                                       \
  }                                                     \
  bool isError(Sandbox##name##OrError p) {              \
    return p.ptr_or_error & 1;                          \
  }                                                     \
  SandboxErrorCode getError(Sandbox##name##OrError p) { \
    assert(isError(p));                                 \
    return (SandboxErrorCode)(p.ptr_or_error >> 2);     \
  }                                                     \
  Sandbox##name get##name(Sandbox##name##OrError p) {   \
    assert(!isError(p));                                \
    return create##name(p.ptr_or_error);                \
  }
SANDBOX_POINTER_TYPES(DECLARE_SANDBOX_POINTER_HELPERS)
#undef DECLARE_SANDBOX_POINTER_HELPERS

SandboxVoidOrError createVoidOrError(void) {
  return {0};
}
SandboxVoidOrError createVoidOrError(SandboxErrorCode err) {
  return {(u32)((err << 2) | 1)};
}
bool isError(SandboxVoidOrError v) {
  return v.void_or_error & 1;
}
SandboxErrorCode getError(SandboxVoidOrError v) {
  assert(isError(v));
  return (SandboxErrorCode)(v.void_or_error >> 2);
}

bool isError(const SandboxBoolOrError &p) {
  return p.bool_or_error & 1;
}
SandboxErrorCode getError(const SandboxBoolOrError &p) {
  return (SandboxErrorCode)(p.bool_or_error >> 2);
}
bool getBool(const SandboxBoolOrError &p) {
  return p.bool_or_error >> 2;
}

bool isError(const SandboxUint8PtrOrError &p) {
  return p.is_error;
}
SandboxErrorCode getError(const SandboxUint8PtrOrError &p) {
  return (SandboxErrorCode)p.data.error;
}
u32 getUint8Ptr(SandboxUint8PtrOrError p) {
  return p.data.val;
}

bool isError(const SandboxSizeTOrError &p) {
  return p.is_error;
}
SandboxErrorCode getError(const SandboxSizeTOrError &p) {
  return (SandboxErrorCode)p.data.error;
}
size_t getSizeT(const SandboxSizeTOrError &p) {
  return p.data.val;
}

SandboxPropNameIDListPtrOrError createPropNameIDListPtrOrError(u32 ptr) {
  return {ptr};
}
SandboxPropNameIDListPtrOrError createPropNameIDListPtrOrError(
    SandboxErrorCode err) {
  return {static_cast<u32>((err << 2) | 1)};
}

SandboxValue createUndefinedValue() {
  SandboxValue val;
  val.kind = SandboxValueKindUndefined;
  return val;
}
SandboxValue createNullValue() {
  SandboxValue val;
  val.kind = SandboxValueKindNull;
  return val;
}
SandboxValue createBoolValue(bool b) {
  SandboxValue val;
  val.kind = SandboxValueKindBoolean;
  val.data.boolean = b;
  return val;
}
SandboxValue createNumberValue(double d) {
  SandboxValue val;
  val.kind = SandboxValueKindNumber;
  val.data.number = d;
  return val;
}

/// Helpers to create a SandboxValue from a pointer to a ManagedPointer.
SandboxValue createObjectValue(u32 ptr) {
  SandboxValue val;
  val.kind = SandboxValueKindObject;
  val.data.pointer = ptr;
  return val;
}
SandboxValue createStringValue(u32 ptr) {
  SandboxValue val;
  val.kind = SandboxValueKindString;
  val.data.pointer = ptr;
  return val;
}
SandboxValue createBigIntValue(u32 ptr) {
  SandboxValue val;
  val.kind = SandboxValueKindBigInt;
  val.data.pointer = ptr;
  return val;
}
SandboxValue createSymbolValue(u32 ptr) {
  SandboxValue val;
  val.kind = SandboxValueKindSymbol;
  val.data.pointer = ptr;
  return val;
}

SandboxValueKind getValueKind(const SandboxValue &val) {
  return val.kind;
}

bool isBoolValue(const SandboxValue &val) {
  return getValueKind(val) == SandboxValueKindBoolean;
}
bool isNumberValue(const SandboxValue &val) {
  return getValueKind(val) == SandboxValueKindNumber;
}
bool isObjectValue(const SandboxValue &val) {
  return getValueKind(val) == SandboxValueKindObject;
}
bool isStringValue(const SandboxValue &val) {
  return getValueKind(val) == SandboxValueKindString;
}
bool isBigIntValue(const SandboxValue &val) {
  return getValueKind(val) == SandboxValueKindBigInt;
}
bool isSymbolValue(const SandboxValue &val) {
  return getValueKind(val) == SandboxValueKindSymbol;
}

bool getBoolValue(const SandboxValue &val) {
  (void)isBoolValue;
  assert(isBoolValue(val));
  return val.data.boolean;
}
double getNumberValue(const SandboxValue &val) {
  (void)isNumberValue;
  assert(isNumberValue(val));
  return val.data.number;
}
SandboxObject getObjectValue(const SandboxValue &val) {
  (void)isObjectValue;
  assert(isObjectValue(val));
  return createObject(val.data.pointer);
}
SandboxString getStringValue(const SandboxValue &val) {
  (void)isStringValue;
  assert(isStringValue(val));
  return createString(val.data.pointer);
}
SandboxBigInt getBigIntValue(const SandboxValue &val) {
  (void)isBigIntValue;
  assert(isBigIntValue(val));
  return createBigInt(val.data.pointer);
}
SandboxSymbol getSymbolValue(const SandboxValue &val) {
  (void)isSymbolValue;
  assert(isSymbolValue(val));
  return createSymbol(val.data.pointer);
}
u32 getPointerValue(const SandboxValue &val) {
  assert(getValueKind(val) | SANDBOX_POINTER_MASK);
  return val.data.pointer;
}

SandboxValueOrError createValueOrError(SandboxValue val) {
  SandboxValueOrError res;
  res.value = val;
  return res;
}
SandboxValueOrError createValueOrError(SandboxErrorCode err) {
  SandboxValueOrError res;
  res.value.kind = SandboxValueKindError;
  res.value.data.error = err;
  return res;
}
bool isError(const SandboxValueOrError &val) {
  return getValueKind(val.value) == SandboxValueKindError;
}
SandboxValue getValue(const SandboxValueOrError &val) {
  assert(!isError(val));
  return val.value;
}
SandboxErrorCode getError(const SandboxValueOrError &val) {
  assert(isError(val));
  return val.value.data.error;
}

/// Mapping from a type to the wasm type enum used to define function
/// signatures. This is used by FunctionTypeImpl below to create the function
/// type for a given function signature.
template <typename T>
struct WasmType;
template <>
struct WasmType<u32> : std::integral_constant<wasm_rt_type_t, WASM_RT_I32> {};
template <>
struct WasmType<u64> : std::integral_constant<wasm_rt_type_t, WASM_RT_I64> {};
template <>
struct WasmType<f32> : std::integral_constant<wasm_rt_type_t, WASM_RT_F32> {};
template <>
struct WasmType<f64> : std::integral_constant<wasm_rt_type_t, WASM_RT_F64> {};

/// Unpack the given function signature using template specialization, and
/// combine the types of the parameters and return value into a
/// wasm_rt_func_type_t.
/// https://devblogs.microsoft.com/oldnewthing/20200713-00/?p=103978
template <typename F>
struct FunctionTypeImpl;

template <typename Ret, typename... Args>
struct FunctionTypeImpl<Ret(w2c_hermes *, Args...)> {
  static wasm_rt_func_type_t get() {
    // Turn the given signature into a wasm_rt_func_type_t by converting each
    // type into the corresponding wasm enum type, and calling
    // wasm2c_hermes_get_func_type.
    // For instance, a signature of void(w2c_hermes*, u32, u32) would result in
    // the call wasm2c_hermes_get_func_type(2, 0, WASM_RT_I32, WASM_RT_I32).
    // When there is a return value, the return type is added to the end of the
    // list, and the second argument is 1. For example u32(w2c_hermes*, u32)
    // would result in
    // wasm2c_hermes_get_func_type(1, 1, WASM_RT_I32,WASM_RT_I32).

    if constexpr (std::is_same_v<Ret, void>) {
      return wasm2c_hermes_get_func_type(
          sizeof...(Args), 0, WasmType<Args>::value...);
    } else {
      return wasm2c_hermes_get_func_type(
          sizeof...(Args), 1, WasmType<Args>::value..., WasmType<Ret>::value);
    }
  }
};

/// Convert the function type represented by the template parameter \p F to a
/// wasm_rt_func_type_t that can be used for populating and validating the types
/// of functions.
template <typename F>
wasm_rt_func_type_t getFunctionType() {
  return FunctionTypeImpl<F>::get();
}

/// Get the function pointer corresponding to the given index \p idx and cast it
/// to the signature \p F. This retrieves the function from the indirect
/// function table, and checks that the function has the expected type.
template <typename F>
F *getFunctionPtr(w2c_hermes *mod, u32 idx) {
  auto &table = mod->w2c_0x5F_indirect_function_table;
  if (idx >= table.size)
    abort();
  auto &funcref = table.data[idx];
  if (getFunctionType<F>() != funcref.func_type)
    abort();
  return (F *)funcref.func;
}

/// Helper class to manage a pointer to a value or array of type \p T in the
/// sandbox heap. This maintains both the size and value of the referenced
/// memory, and performs bounds checks to ensure that it falls entirely within
/// the sandbox heap. It recomputes the actual pointer to the data on each
/// access, making it safe to hold across sandbox operations (since they may
/// otherwise move the sandbox heap if it grows).
/// While it does provide some safety, it should be used with care. Operations
/// that return pointers into the sandbox heap such as the * and -> operators
/// should not be mixed with operations that may move the sandbox heap, such as
/// calling a sandbox function. For example the following would be unsafe:
///   myPtr->field = w2c_hermes_malloc(mod, 10);
/// since the call to w2c_hermes_malloc may move the sandbox heap, invalidating
/// the reference into sandbox memory on the LHS. It can instead be safely
/// rewritten as:
///   auto tmp = w2c_hermes_malloc(mod, 10);
///   myPtr->field = tmp;
template <typename T>
class Ptr {
 protected:
  w2c_hermes *mod_;
  u32 ptr_;
#ifndef NDEBUG
  u32 n_;
#endif

  /// Set the pointer to refer to the address \p ptr, with \p elements of T.
  void set(u32 ptr, u32 n) {
    ptr_ = ptr;
#ifndef NDEBUG
    n_ = n;
#endif

    // Check that the memory range accessed through this Ptr is entirely within
    // the module's memory.
    if (((u64)ptr + sizeof(T) * (u64)n) > mod_->w2c_memory.size)
      abort();

    // Check for null-dereferences: ensure that the pointer must be non-null if
    // the size is non-zero. This is to prevent writes at the zero address
    // which, given that zero is valid address in WASM linear memory, could be
    // abused to tamper with the Hermes VM internal state.
    if (ptr == 0 && n != 0)
      abort();
  }

  /// Constructor to create a Ptr and defer initializing it. This is used by
  /// subclasses that need to perform some work before initializing the pointer.
  Ptr(w2c_hermes *mod) : Ptr(mod, 0, 0){};

 public:
  Ptr(w2c_hermes *mod, u32 ptr, u32 n = 1) : mod_(mod) {
    set(ptr, n);
  }

  /// Define operators to conveniently access the referenced value. We can
  /// compute the address without a bounds check here because the constructor
  /// has checked that the pointer is valid.
  /// Note that these references are never safe to hold across operations that
  /// may result in moving the sandbox heap.
  T *operator->() const {
    return &**this;
  }
  T &operator*() const {
    return *reinterpret_cast<T *>(&mod_->w2c_memory.data[ptr_]);
  }
  T &operator[](size_t idx) const {
    assert(idx < n_);
    return (&**this)[idx];
  }

  /// Allow implicit conversion to u32 for convenience, particularly when
  /// passing a Ptr to a sandbox function that accepts u32.
  operator u32() const {
    return ptr_;
  }
};

/// Invalidate the ManagedPointer referenced by \p ptr.
void releasePointer(w2c_hermes *mod, u32 ptr) {
  auto vtable = sb::Ptr<SandboxManagedPointer>(mod, ptr)->vtable;
  auto invId = sb::Ptr<SandboxManagedPointerVTable>(mod, vtable)->invalidate;
  auto inv = getFunctionPtr<void(w2c_hermes *, u32)>(mod, invId);
  inv(mod, ptr);
}

} // namespace sb

/// Helper class to save and restore a value on exiting a scope.
template <typename T>
class SaveAndRestore {
  T &target_;
  T oldVal_;

 public:
  SaveAndRestore(T &target) : target_(target), oldVal_(target) {}
  ~SaveAndRestore() {
    target_ = oldVal_;
  }
};

#if WASM_RT_USE_MMAP

/// Get the page size of the system.
static uintptr_t os_native_pagesize() {
#if defined(_WIN32)
  SYSTEM_INFO systemInfo;
  GetSystemInfo(&systemInfo);
  uintptr_t pageSize = systemInfo.dwPageSize;
#else
  uintptr_t pageSize = sysconf(_SC_PAGESIZE);
#endif
  return pageSize;
}

/// Remove read/write/execute access to the page containing the given address.
/// Any accesses to the page will result in a bad memory access.
static int os_guardpage(void *addr) {
#if defined(_WIN32)
  return VirtualFree(addr, os_native_pagesize(), MEM_DECOMMIT) ? 0 : -1;
#else
  return mprotect(addr, os_native_pagesize(), PROT_NONE);
#endif
}

#endif // WASM_RT_USE_MMAP

/// Define a simple wrapper class that manages the lifetime of the w2c_hermes
/// instance. This lets us maintain the order of the destructor relative to
/// other fields in HermesSandboxRuntimeImpl, which may need to be destroyed
/// first.
class W2CHermesRAII : public w2c_hermes {
 public:
  W2CHermesRAII() {
    wasm2c_hermes_instantiate(
        this,
        (w2c_env *)this,
        (w2c_hermes__import *)this,
        (w2c_wasi__snapshot__preview1 *)this);
    w2c_hermes_0x5Finitialize(this);

#if WASM_RT_USE_MMAP

    // When the sandbox memory is allocated with mmap, and the
    // lowest used address is larger than the page size, mprotect
    // the first page so we trap on null accesses.
    const uintptr_t pageSize = os_native_pagesize();
    const uintptr_t global_base = w2c_hermes_get_global_base(this);
    if (pageSize <= global_base) {
      // Add a guard page based at NULL (address 0)
      os_guardpage(this->w2c_memory.data);
    }

    // Move stack end to the next page boundary and make room for the guard
    // page.
    uintptr_t stackEndPageAligned =
        (this->w2c_0x5F_stack_end + pageSize - 1) & ~(pageSize - 1);

    // Add a guard page to the bottom of the stack.
    // Ensure that there are at least 2 pages in stack
    // before the guard page.
    if (stackEndPageAligned + 3 * pageSize <= this->w2c_0x5F_stack_pointer &&
        !os_guardpage((void *)(this->w2c_memory.data + stackEndPageAligned))) {
      // Adjust the stack end to after the guard page.
      this->w2c_0x5F_stack_end = stackEndPageAligned + pageSize;
    }

#endif // WASM_RT_USE_MMAP
  }
  ~W2CHermesRAII() {
    wasm2c_hermes_free(this);
  }
};

/// Helper class to manage allocations made in the stack in sandbox memory.
/// Constructing this allocates memory for a T on the stack, and the destructor
/// will restore the stack. Note that these must be destroyed in the reverse
/// order of construction.
template <typename T>
class StackAlloc : public sb::Ptr<T> {
  /// Save the old level since alignment may mean the stack needs to be restored
  /// by further than sizeof(T).
  u32 oldLevel_;

 public:
  StackAlloc(w2c_hermes *mod) : sb::Ptr<T>(mod) {
    oldLevel_ = w2c_hermes_stackSave(mod);

    // Sanity check that the stack allocation is a reasonable size.
    static_assert(sizeof(T) < 512);
    sb::Ptr<T>::set(w2c_hermes_stackAlloc(mod, sizeof(T)), 1);
  }
  ~StackAlloc() {
    w2c_hermes_stackRestore(this->mod_, oldLevel_);
  }

  StackAlloc(const StackAlloc &) = delete;
  StackAlloc &operator=(const StackAlloc &) = delete;

  StackAlloc(StackAlloc &&other) = delete;
  StackAlloc &operator=(StackAlloc &&other) = delete;
};

/// Helper class to allocate and manage data allocated in a LIFO allocation
/// region. Unlike StackAlloc, this can be used for allocations with dynamic
/// size, since the allocations will be made on the heap.
template <typename T>
class LIFOAlloc : public sb::Ptr<T> {
 public:
  LIFOAlloc(w2c_hermes *mod, u32 n)
      : sb::Ptr<T>(mod, w2c_hermes_malloc(mod, sizeof(T) * n), n) {
    // TODO: Implement a more efficient allocator.
  }
  ~LIFOAlloc() {
    w2c_hermes_free(this->mod_, this->ptr_);
  }

  LIFOAlloc(const LIFOAlloc &) = delete;
  LIFOAlloc &operator=(const LIFOAlloc &) = delete;

  LIFOAlloc(LIFOAlloc &&other) = delete;
  LIFOAlloc &operator=(LIFOAlloc &&other) = delete;
};

/// Manage ownership of user-provided native JSI state, such as HostFunction,
/// HostObject, and NativeState. These cannot be stored directly in the sandbox
/// heap since they may be corrupted, so this provides a simple indexed data
/// structure to store them. Indices are reused after allocations are freed.
/// TODO(T174477603): Allow the table to shrink if it has too many free entries.
template <typename T>
class NativeTable {
  /// Store the elements for the table. Use a deque rather than a vector so we
  /// can hold stable references to elements (needed by getHostFunction for
  /// instance).
  std::deque<T> table_;

  /// Store the indices of free elements in the table. This list is consulted
  /// first before adding more elements to the table.
  std::vector<u32> freeList_;

 public:
  ~NativeTable() {
    assert(table_.size() == freeList_.size() && "Dangling native reference.");
  }

  /// Get the element at the given index.
  T &at(size_t idx) {
    if (idx >= table_.size())
      abort();
    return table_[idx];
  }

  /// Get the total size of this table.
  size_t size() const {
    return table_.size();
  }

  /// Emplace a new element in the table, reusing a free index if possible.
  template <typename... Args>
  u32 emplace(Args &&...args) {
    if (freeList_.empty()) {
      table_.emplace_back(std::forward<Args>(args)...);
      return table_.size() - 1;
    }
    u32 idx = freeList_.back();
    freeList_.pop_back();
    table_[idx] = T(std::forward<Args>(args)...);
    return idx;
  }

  /// Release the entry at the given index, adding it to the free list.
  void release(u32 idx) {
    if (idx >= table_.size())
      abort();
    table_[idx] = {};
    freeList_.push_back(idx);
  }
};

/// Define a helper macro to throw an exception for unimplemented methods. The
/// actual throw is kept in a separate function because throwing generates a lot
/// of code.
[[noreturn]] void throwUnimplementedImpl(const char *name) {
  throw JSINativeException(std::string("Unimplemented function ") + name);
}

#define THROW_UNIMPLEMENTED() throwUnimplementedImpl(__func__)

class HermesSandboxRuntimeImpl : public facebook::hermes::HermesSandboxRuntime,
                                 public W2CHermesRAII {
  class ManagedPointerHolder;

  /// Pre-allocated vtables for each of the sandbox types used by this wrapper.
  StackAlloc<SandboxBufferVTable> bufferVTable_;
  StackAlloc<SandboxGrowableBufferVTable> growableBufferVTable_;
  StackAlloc<SandboxHostFunctionVTable> hostFunctionVTable_;
  StackAlloc<SandboxPropNameIDListVTable> propNameIDListVTable_;
  StackAlloc<SandboxHostObjectVTable> hostObjectVTable_;
  StackAlloc<SandboxNativeStateVTable> nativeStateVTable_;

  /// The list of pointers currently retained through JSI. This manages the
  /// lifetime of the underlying pointers through the ABI so that they are
  /// released when the corresponding JSI pointer is released.
  hermes::ManagedChunkedList<ManagedPointerHolder> managedPointers_;

  /// Tables for user-provided native JSI types.
  NativeTable<HostFunctionType> hostFunctions_;
  NativeTable<std::shared_ptr<HostObject>> hostObjects_;
  NativeTable<std::shared_ptr<NativeState>> nativeStates_;

  /// A copy of the vtable for srt_ with all of the function pointers retrieved
  /// once at creation. This allows for efficient access, and makes the
  /// functions more convenient to call.
  SandboxRuntimeVTableMirror vt_;

  /// A pointer to the ABI runtime object allocated inside sandbox memory.
  u32 srt_;

  /// Whether we are currently processing a JSError. This is used to detect
  /// recursive invocations of the JSError constructor and prevent them from
  /// causing a stack overflow.
  bool activeJSError_ = false;

  /// Whether the integrator has requested we terminate execution with a timeout
  /// exception.
  std::atomic<bool> asyncTimeout_{false};

#ifndef NDEBUG
  /// Counter to track heap allocations that cannot be managed by a LIFOAlloc.
  /// This helps ensure that these allocations are correctly cleaned up.
  size_t numAllocs_ = 0;
#endif

  /// Allocate an array of \p n elements of type T in the sandbox heap.
  template <typename T>
  sb::Ptr<T> sbAlloc(size_t n = 1) {
    // Check for integer overflows
    static constexpr size_t maxN = UINT32_MAX / sizeof(T);
    if (n > maxN)
      abort();
    auto ptr = w2c_hermes_malloc(this, sizeof(T) * n);
    return sb::Ptr<T>(this, ptr, n);
  }

  /// Manage the allocation counter.
  void incAllocDbg() {
#ifndef NDEBUG
    numAllocs_++;
#endif
  }
  void decAllocDbg() {
#ifndef NDEBUG
    numAllocs_--;
#endif
  }

  /// Add the given function pointer \p func to the function table, populating
  /// its associated fields. The function table must already have space for this
  /// additional entry.
  template <typename F>
  void registerFunction(F *func, u32 funcIdx) {
    wasm_rt_funcref_table_t &table = w2c_0x5F_indirect_function_table;
    assert(funcIdx < table.size && "Function index out of range");
    table.data[funcIdx].func_type = sb::getFunctionType<F>();
    table.data[funcIdx].func = (wasm_rt_function_ptr_t)func;
    table.data[funcIdx].module_instance = static_cast<w2c_hermes *>(this);
  }

  /// Cast from the given module pointer to the JSI runtime pointer.
  static HermesSandboxRuntimeImpl &getRuntime(w2c_hermes *mod) {
    return *static_cast<HermesSandboxRuntimeImpl *>(mod);
  }

  /// Helper to provide access to a non-const module pointer from const methods.
  w2c_hermes *getMutMod() const {
    return const_cast<HermesSandboxRuntimeImpl *>(this);
  }

  /// A ManagedChunkedList element that indicates whether it's occupied based on
  /// a refcount.
  class ManagedPointerHolder : public PointerValue {
    /// The reference count indicating the number of existing JSI references
    /// that point to this ManagedPointerHolder.
    std::atomic<uint32_t> refCount_;

    /// The managed pointer value in the sandbox. A non-zero value indicates
    /// that the underlying SandboxManagedPointer is still live (even if the
    /// reference count is zero).
    u32 managedPointer_;

    /// Store the runtime pointer and nextFree in a union to save space since
    /// they are never needed at the same time.
    union {
      /// The runtime that owns this ManagedPointerHolder. We need to retain
      /// this to invalidate the reference inside the sandbox.
      HermesSandboxRuntimeImpl *runtime_;

      /// The next free entry. This value is managed by the list itself.
      ManagedPointerHolder *nextFree_;
    };

   public:
    ManagedPointerHolder() : refCount_(0), managedPointer_(0) {}

    /// Determine whether the element is occupied by inspecting the refcount.
    bool isFree() const {
      // See comments in hermes_vtable.cpp for why we use relaxed operations
      // here and why we need different ordering under TSAN.

#if THREAD_SANITIZER_BUILD
      return refCount_.load(std::memory_order_acquire) == 0;
#else
      return refCount_.load(std::memory_order_relaxed) == 0;
#endif
    }

    /// Store a value and start the refcount at 1. After invocation, this
    /// instance is occupied with a value, and the "nextFree" methods should
    /// not be used until the value is released.
    void emplace(HermesSandboxRuntimeImpl &runtime, u32 managedPointer) {
      assert(isFree() && "Emplacing already occupied value");
      refCount_.store(1, std::memory_order_relaxed);
      managedPointer_ = managedPointer;
      runtime_ = &runtime;
    }

    /// Get the next free element. Must not be called when this instance is
    /// occupied with a value.
    ManagedPointerHolder *getNextFree() {
      assert(isFree() && "Free pointer unusable while occupied");
      return nextFree_;
    }

    /// Set the next free element. Must not be called when this instance is
    /// occupied with a value.
    void setNextFree(ManagedPointerHolder *nextFree) {
      assert(isFree() && "Free pointer unusable while occupied");
      nextFree_ = nextFree;
    }

    u32 getManagedPointer() const {
      assert(!isFree() && "Value not present");
      return managedPointer_;
    }

    void invalidate() override {
      dec();
    }

    void inc() {
      // See comments in hermes_vtable.cpp for why we use relaxed operations
      // here.
      auto oldCount = refCount_.fetch_add(1, std::memory_order_relaxed);
      assert(oldCount && "Cannot resurrect a pointer");
      assert(oldCount + 1 != 0 && "Ref count overflow");
      (void)oldCount;
    }

    void dec() {
      // See comments in hermes_vtable.cpp for why we use relaxed operations
      // here, and why TSAN requires different ordering.

#if THREAD_SANITIZER_BUILD
      auto oldCount = refCount_.fetch_sub(1, std::memory_order_release);
#else
      auto oldCount = refCount_.fetch_sub(1, std::memory_order_relaxed);
#endif
      assert(oldCount > 0 && "Ref count underflow");
      // TODO(T174477630): releasePointer can only be invoked from the JS thread
      // since it involves calling back into the sandbox. This means that
      // destroying a JSI Pointer created by the sandbox can only currently be
      // done from the JS thread.
      if (oldCount == 1)
        sb::releasePointer(runtime_, managedPointer_);
    }
  };

  /// Implement a simple Buffer for exposing to the sandbox.
  class BufferWrapper : public SandboxBuffer {
   public:
    /// Create a new BufferWrapper by copying the contents of \p buf into
    /// sandbox memory.
    static sb::Ptr<BufferWrapper> create(
        HermesSandboxRuntimeImpl &runtime,
        const std::shared_ptr<const Buffer> &buf) {
      runtime.incAllocDbg();

      auto sz = buf->size();
      // Hermes requires the buffer to be null-terminated. Allocate an extra
      // byte and make sure it is set to null.
      auto data = runtime.sbAlloc<char>(sz + 1);
      memcpy(&*data, buf->data(), sz);
      data[sz] = 0;

      auto self = runtime.sbAlloc<BufferWrapper>();
      self->vtable = runtime.bufferVTable_;
      self->data = data;
      self->size = sz;
      return self;
    }

    static void release(w2c_hermes *mod, u32 buf) {
      static_cast<HermesSandboxRuntimeImpl *>(mod)->decAllocDbg();

      // Free the data first, then the buffer itself.
      w2c_hermes_free(mod, sb::Ptr<BufferWrapper>(mod, buf)->data);
      w2c_hermes_free(mod, buf);
    }
  };

  /// Implement GrowableBuffer for exposing to the sandbox.
  class GrowableBufferImpl : public SandboxGrowableBuffer {
   public:
    /// Create a new GrowableBufferImpl instance.
    static sb::Ptr<GrowableBufferImpl> create(
        HermesSandboxRuntimeImpl &runtime) {
      runtime.incAllocDbg();

      auto self = runtime.sbAlloc<GrowableBufferImpl>();
      self->vtable = runtime.growableBufferVTable_;
      self->data = 0;
      self->size = 0;
      self->used = 0;
      return self;
    }

    static void release(w2c_hermes *mod, sb::Ptr<GrowableBufferImpl> self) {
      static_cast<HermesSandboxRuntimeImpl *>(mod)->decAllocDbg();

      w2c_hermes_free(mod, self->data);
      w2c_hermes_free(mod, self);
    }

    static void try_grow_to(w2c_hermes *mod, u32 buf, u32 sz) {
      sb::Ptr<GrowableBufferImpl> self{mod, buf};
      if (sz < self->size)
        return;
      if (u32 newData = w2c_hermes_realloc(mod, self->data, sz)) {
        self->data = newData;
        self->size = sz;
      }
    }

    /// Copy the contents of this GrowableBuffer into an std::string and return
    /// it.
    std::string getString(w2c_hermes *mod) {
      return {&*sb::Ptr<char>(mod, data, used), used};
    }
  };

  /// Invoke the given \p fn and return the result. If an exception occurs,
  /// catch it and convert it to an error struct with type \p T by invoking \p
  /// wrapErr.
  template <typename T, size_t N, typename Fn>
  auto
  sbRethrow(T (*wrapErr)(SandboxErrorCode), const char (&where)[N], Fn fn) {
    try {
      return fn();
    } catch (const JSError &e) {
      // Caught a JSError, retrieve its value and set the reported error.
      StackAlloc<SandboxValue> errVal(this);
      *errVal = toSandboxValue(e.value());
      vt_.set_js_error_value(this, srt_, errVal);
      return wrapErr(SandboxErrorCodeJSError);
    } catch (const std::exception &e) {
      // For all other native exceptions, register a native exception message
      // with the location where this error occurred
      std::string what{"Exception in "};
      what.append(where, N - 1).append(": ").append(e.what());
      LIFOAlloc<char> whatBuf(this, what.size());
      memcpy(&*whatBuf, what.c_str(), what.size());
      vt_.set_native_exception_message(this, srt_, whatBuf, what.size());
      return wrapErr(SandboxErrorCodeNativeException);
    } catch (...) {
      // Unknown exception, register a generic message.
      std::string what{"An unknown exception occurred in "};
      what.append(where, N - 1);
      LIFOAlloc<char> whatBuf(this, what.size());
      memcpy(&*whatBuf, what.c_str(), what.size());
      vt_.set_native_exception_message(this, srt_, whatBuf, what.size());
      return wrapErr(SandboxErrorCodeNativeException);
    }
  }

  /// Implement the HostFunction interface, for calling back from the sandbox
  /// into a native user provided JSI function.
  class HostFunctionWrapper : public SandboxHostFunction {
    /// The index of the jsi::HostFunction to be invoked.
    u32 hfIdx_;

   public:
    /// Create a new HostFunctionWrapper that will invoke the given \p
    /// hostFunction.
    static sb::Ptr<HostFunctionWrapper> create(
        HermesSandboxRuntimeImpl &runtime,
        HostFunctionType hostFunction) {
      runtime.incAllocDbg();

      auto self = runtime.sbAlloc<HostFunctionWrapper>();
      self->vtable = runtime.hostFunctionVTable_;
      self->hfIdx_ = runtime.hostFunctions_.emplace(std::move(hostFunction));
      return self;
    }

    static void release(w2c_hermes *mod, u32 hfw) {
      static_cast<HermesSandboxRuntimeImpl *>(mod)->decAllocDbg();

      // Free the HostFunction from the table.
      sb::Ptr<HostFunctionWrapper> self(mod, hfw);
      getRuntime(mod).hostFunctions_.release(self->hfIdx_);
      // Free the wrapper object in the sandbox heap.
      w2c_hermes_free(mod, hfw);
    }

    static void call(
        w2c_hermes *mod,
        u32 res,
        u32 func,
        u32 srt,
        u32 thisArg,
        u32 args,
        u32 count) {
      sb::Ptr<HostFunctionWrapper> self(mod, func);
      auto &rt = getRuntime(mod);

      auto ret = rt.sbRethrow(sb::createValueOrError, "HostFunction", [&] {
        // Convert the arguments from SandboxValue to jsi::Value.
        sb::Ptr<SandboxValue> argsPtr(mod, args, count);
        std::vector<Value> jsiArgs;
        jsiArgs.reserve(count);
        for (size_t i = 0; i < count; ++i)
          jsiArgs.emplace_back(rt.cloneToJSIValue(argsPtr[i]));

        auto jsiThisArg =
            rt.cloneToJSIValue(*sb::Ptr<SandboxValue>(&rt, thisArg));
        auto &hf = rt.hostFunctions_.at(self->hfIdx_);

        // Call the user provided function and convert the result back to a
        // SandboxValue. Note that the resulting value must be cloned because
        // the returned jsi::Value will go out of scope after this function
        // returns.
        return sb::createValueOrError(
            rt.cloneToSandboxValue(hf(rt, jsiThisArg, jsiArgs.data(), count)));
      });

      // Write the result to the return value location.
      *sb::Ptr<SandboxValueOrError>(&rt, res) = ret;
    }

    HostFunctionType &getHostFunction(HermesSandboxRuntimeImpl &rt) {
      return rt.hostFunctions_.at(hfIdx_);
    }
  };

  /// Implement PropNameIDList for exposing to the sandbox.
  class PropNameIDListWrapper : public SandboxPropNameIDList {
   public:
    /// Create a new PropNameIDListWrapper with the given size. The caller must
    /// then write to the allocated props array before it is released.
    static sb::Ptr<PropNameIDListWrapper> create(
        HermesSandboxRuntimeImpl &runtime,
        size_t sz) {
      runtime.incAllocDbg();

      static_assert(
          alignof(PropNameIDListWrapper) == alignof(SandboxPropNameID),
          "Alignment must match to create a single allocation.");

      // Allocate the wrapper and the array of props in a single allocation.
      static constexpr size_t maxSize =
          (UINT32_MAX - sizeof(PropNameIDListWrapper)) /
          sizeof(SandboxPropNameID);

      // Check for integer overflows
      if (sz > maxSize) {
        abort();
      }
      auto alloc = w2c_hermes_malloc(
          &runtime,
          sizeof(PropNameIDListWrapper) + sizeof(SandboxPropNameID) * sz);
      if (!alloc) {
        abort();
      }

      sb::Ptr<PropNameIDListWrapper> self(&runtime, alloc);
      self->vtable = runtime.propNameIDListVTable_;
      self->size = sz;
      self->props = alloc + sizeof(PropNameIDListWrapper);
      return self;
    }

    static void release(w2c_hermes *mod, u32 self) {
      static_cast<HermesSandboxRuntimeImpl *>(mod)->decAllocDbg();

      sb::Ptr<PropNameIDListWrapper> selfPtr(mod, self);
      auto sz = selfPtr->size;

      // Obtain a pointer to the props array and free each entry.
      sb::Ptr<SandboxPropNameID> propsPtr(
          mod, self + sizeof(PropNameIDListWrapper), sz);
      for (size_t i = 0; i < sz; ++i)
        sb::releasePointer(mod, propsPtr[i].pointer);

      // Free the PropNameIDListWrapper itself.
      w2c_hermes_free(mod, self);
    }
  };

  /// Implement HostObject for exposing to the sandbox. This allows the sandbox
  /// to interact with jsi::HostObjects.
  class HostObjectWrapper : public SandboxHostObject {
    /// The index of the user-provided jsi::HostObject.
    u32 hoIdx_;

   public:
    /// Create a new HostObjectWrapper that when accessed by JS executing in the
    /// sandbox will pass accesses through to the given \p hostObject.
    static sb::Ptr<HostObjectWrapper> create(
        HermesSandboxRuntimeImpl &runtime,
        std::shared_ptr<HostObject> hostObject) {
      runtime.incAllocDbg();

      auto self = runtime.sbAlloc<HostObjectWrapper>();
      self->vtable = runtime.hostObjectVTable_;
      self->hoIdx_ = runtime.hostObjects_.emplace(std::move(hostObject));
      return self;
    }

    static void get(w2c_hermes *mod, u32 res, u32 obj, u32 srt, u32 prop) {
      sb::Ptr<HostObjectWrapper> self(mod, obj);
      auto &rt = getRuntime(mod);

      auto ret = rt.sbRethrow(sb::createValueOrError, "HostObject", [&] {
        // Convert the prop to a jsi::PropNameID and invoke the getter on the
        // jsi::HostObject.
        auto jsiProp = rt.cloneToJSIPropNameID(SandboxPropNameID{prop});
        auto res = rt.hostObjects_.at(self->hoIdx_)->get(rt, jsiProp);
        return sb::createValueOrError(rt.cloneToSandboxValue(res));
      });
      *sb::Ptr<SandboxValueOrError>(mod, res) = ret;
    }

    static u32 set(w2c_hermes *mod, u32 obj, u32 srt, u32 prop, u32 val) {
      sb::Ptr<HostObjectWrapper> self(mod, obj);
      auto &rt = getRuntime(mod);

      auto ret = rt.sbRethrow(sb::createVoidOrError, "HostObject", [&] {
        // Convert the prop and value to JSI types and invoke the setter on the
        // jsi::HostObject.
        auto jsiProp = rt.cloneToJSIPropNameID(SandboxPropNameID{prop});
        auto jsiVal = rt.cloneToJSIValue(*sb::Ptr<SandboxValue>(mod, val));
        rt.hostObjects_.at(self->hoIdx_)->set(rt, jsiProp, jsiVal);
        return sb::createVoidOrError();
      });
      return ret.void_or_error;
    }

    static u32 get_own_keys(w2c_hermes *mod, u32 obj, u32 srt) {
      sb::Ptr<HostObjectWrapper> self(mod, obj);
      auto &rt = getRuntime(mod);

      auto ret = rt.sbRethrow(
          sb::createPropNameIDListPtrOrError,
          "HostObject::getPropertyNames",
          [&]() -> SandboxPropNameIDListPtrOrError {
            // Invoke the getOwnPropertyNames method on the jsi::HostObject.
            auto props = rt.hostObjects_.at(self->hoIdx_)->getPropertyNames(rt);
            size_t sz = props.size();
            auto res = PropNameIDListWrapper::create(rt, sz);
            sb::Ptr<SandboxPropNameID> sbProps(mod, res->props, sz);

            // Convert each of the returned props to SandboxPropNameIDs and
            // populate the array in the PropNameIDListWrapper.
            for (size_t i = 0; i < sz; ++i)
              sbProps[i] = rt.cloneToSandboxPropNameID(props[i]);
            return sb::createPropNameIDListPtrOrError(res);
          });
      return ret.ptr_or_error;
    }

    static void release(w2c_hermes *mod, u32 how) {
      static_cast<HermesSandboxRuntimeImpl *>(mod)->decAllocDbg();

      // Release the table entry for this HostObject.
      sb::Ptr<HostObjectWrapper> self(mod, how);
      getRuntime(mod).hostObjects_.release(self->hoIdx_);
      // Free the wrapper object in the sandbox heap.
      w2c_hermes_free(mod, how);
    }

    std::shared_ptr<HostObject> getHostObject(
        HermesSandboxRuntimeImpl &rt) const {
      return rt.hostObjects_.at(hoIdx_);
    }
  };

  /// Implement NativeState for exposing to the sandbox. Since NativeState is
  /// only accessible from native code, this is only used to ensure the
  /// NativeState is freed when the corresponding object is garbage collected.
  class NativeStateWrapper : public SandboxNativeState {
    /// The index of the jsi::NativeState that is stored.
    u32 nsIdx_;

   public:
    /// Create a new NativeStateWrapper that manages the lifetime of the given
    /// jsi::NativeState and allows it to be retrieved by native code.
    static sb::Ptr<NativeStateWrapper> create(
        HermesSandboxRuntimeImpl &runtime,
        std::shared_ptr<NativeState> nativeState) {
      runtime.incAllocDbg();

      auto self = runtime.sbAlloc<NativeStateWrapper>();
      self->vtable = runtime.nativeStateVTable_;
      self->nsIdx_ = runtime.nativeStates_.emplace(std::move(nativeState));
      return self;
    }

    static void release(w2c_hermes *mod, u32 nsw) {
      static_cast<HermesSandboxRuntimeImpl *>(mod)->decAllocDbg();

      // Free the HostFunction which is allocated on the regular heap.
      sb::Ptr<NativeStateWrapper> self(mod, nsw);
      getRuntime(mod).nativeStates_.release(self->nsIdx_);
      // Free the wrapper object in the sandbox heap.
      w2c_hermes_free(mod, nsw);
    }

    std::shared_ptr<NativeState> getNativeState(
        HermesSandboxRuntimeImpl &rt) const {
      return rt.nativeStates_.at(nsIdx_);
    }
  };

  PointerValue *clone(const PointerValue *pv) {
    // TODO: Evaluate whether to keep this null check. It is currently here for
    //       compatibility with hermes' API, but it is odd that it is the only
    //       API that allows null.
    if (!pv)
      return nullptr;

    auto *nonConst = const_cast<PointerValue *>(pv);
    static_cast<ManagedPointerHolder *>(nonConst)->inc();
    return nonConst;
  }

  /// Convert the error code returned by the sandbox into a C++ exception.
  [[noreturn]] void throwError(SandboxErrorCode err) {
    if (err == SandboxErrorCodeJSError) {
      StackAlloc<SandboxValue> resValue(this);
      vt_.get_and_clear_js_error_value(this, resValue, srt_);
      // We have to get and clear the error regardless of whether it is used.
      auto errVal = intoJSIValue(*resValue);

      // If we are already in the process of creating a JSError, it means that
      // something in JSError's constructor is throwing. We cannot handle this
      // gracefully, so bail.
      if (activeJSError_)
        throw JSINativeException("Error thrown while handling error.");

      // Record the fact that we are in the process of creating a JSError.
      SaveAndRestore s(activeJSError_);
      activeJSError_ = true;
      throw JSError(*this, std::move(errVal));
    } else if (err == SandboxErrorCodeNativeException) {
      auto buf = GrowableBufferImpl::create(
          *const_cast<HermesSandboxRuntimeImpl *>(this));
      vt_.get_and_clear_native_exception_message(this, srt_, buf);
      auto msg = buf->getString(this);
      GrowableBufferImpl::release(this, buf);
      throw JSINativeException(std::move(msg));
    }

    // Sandbox threw an unrecognized error code, it may be compromised.
    abort();
  }

  /// Define a series of helper functions to convert to/from the sandbox
  /// representation to JSI. Note that as a rule, we deliberately accept the
  /// parameter by value to force a copy onto the stack. Taking a copy makes it
  /// safe to pass in values that are in the sandbox's memory, since that memory
  /// may move.

#define DECLARE_POINTER_CONVERSIONS(name)                             \
  name intoJSI##name(Sandbox##name p) {                               \
    return make<name>(&managedPointers_.add(*this, p.pointer));       \
  }                                                                   \
  name intoJSI##name(Sandbox##name##OrError p) {                      \
    return intoJSI##name(unwrap(p));                                  \
  }                                                                   \
  Sandbox##name toSandbox##name(const name &p) const {                \
    return sb::create##name(                                          \
        static_cast<const ManagedPointerHolder *>(getPointerValue(p)) \
            ->getManagedPointer());                                   \
  }                                                                   \
  Sandbox##name unwrap(Sandbox##name##OrError p) {                    \
    if (sb::isError(p))                                               \
      throwError(sb::getError(p));                                    \
    return sb::get##name(p);                                          \
  }
  SANDBOX_POINTER_TYPES(DECLARE_POINTER_CONVERSIONS)
#undef DECLARE_POINTER_CONVERSIONS

  PropNameID cloneToJSIPropNameID(SandboxPropNameID name) {
    return intoJSIPropNameID(
        sb::createPropNameID(vt_.clone_propnameid(this, srt_, name.pointer)));
  }
  SandboxPropNameID cloneToSandboxPropNameID(const PropNameID &name) const {
    auto mp = static_cast<const ManagedPointerHolder *>(getPointerValue(name))
                  ->getManagedPointer();
    return sb::createPropNameID(vt_.clone_propnameid(getMutMod(), srt_, mp));
  }

  void unwrap(SandboxVoidOrError v) {
    if (sb::isError(v))
      throwError(sb::getError(v));
  }
  bool unwrap(SandboxBoolOrError p) {
    if (sb::isError(p))
      throwError(sb::getError(p));
    return sb::getBool(p);
  }
  u32 unwrap(SandboxUint8PtrOrError p) {
    if (sb::isError(p))
      throwError(sb::getError(p));
    return sb::getUint8Ptr(p);
  }
  size_t unwrap(SandboxSizeTOrError p) {
    if (sb::isError(p))
      throwError(sb::getError(p));
    return sb::getSizeT(p);
  }

  /// Take ownership of the given value \p v and wrap it in a jsi::Value that
  /// will now manage its lifetime.
  Value intoJSIValue(SandboxValue sv) {
    switch (sv.kind) {
      case SandboxValueKindUndefined:
        return Value::undefined();
      case SandboxValueKindNull:
        return Value::null();
      case SandboxValueKindBoolean:
        return Value(sv.data.boolean);
      case SandboxValueKindNumber:
        return Value(sv.data.number);
      case SandboxValueKindString:
        return make<String>(
            &managedPointers_.add(*this, sb::getPointerValue(sv)));
      case SandboxValueKindObject:
        return make<Object>(
            &managedPointers_.add(*this, sb::getPointerValue(sv)));
      case SandboxValueKindSymbol:
        return make<Symbol>(
            &managedPointers_.add(*this, sb::getPointerValue(sv)));
      case SandboxValueKindBigInt:
        return make<BigInt>(
            &managedPointers_.add(*this, sb::getPointerValue(sv)));
      default:
        // Sandbox returned a value with an unknown kind, it may be compromised.
        abort();
    }
  }

  /// If \p sv is an error, re-throw it as a C++ exception, otherwise take
  /// ownership of the value and wrap it in a jsi::Value.
  Value intoJSIValue(SandboxValueOrError sv) {
    if (sb::isError(sv))
      throwError(sb::getError(sv));
    return intoJSIValue(sb::getValue(sv));
  }

  /// Create a jsi::Value from the given SandboxValue without taking ownership
  /// of it. This will clone any underlying pointers if needed.
  Value cloneToJSIValue(SandboxValue v) {
    switch (sb::getValueKind(v)) {
      case SandboxValueKindUndefined:
        return Value::undefined();
      case SandboxValueKindNull:
        return Value::null();
      case SandboxValueKindBoolean:
        return Value(sb::getBoolValue(v));
      case SandboxValueKindNumber:
        return Value(sb::getNumberValue(v));
      case SandboxValueKindString:
        return intoJSIString(SandboxString{
            vt_.clone_string(this, srt_, sb::getStringValue(v).pointer)});
      case SandboxValueKindObject:
        return intoJSIObject(SandboxObject{
            vt_.clone_object(this, srt_, sb::getObjectValue(v).pointer)});
      case SandboxValueKindSymbol:
        return intoJSISymbol(SandboxSymbol{
            vt_.clone_symbol(this, srt_, sb::getSymbolValue(v).pointer)});
      case SandboxValueKindBigInt:
        return intoJSIBigInt(SandboxBigInt{
            vt_.clone_bigint(this, srt_, sb::getBigIntValue(v).pointer)});
      default:
        // Sandbox returned a value with an unknown kind, it may be compromised.
        abort();
    }
  }

  /// Convert the given jsi::Value \p v to a SandboxValue. The SandboxValue will
  /// be invalidated once the jsi::Value is released. This is useful when a
  /// SandboxValue is only needed for a temporary duration that falls within the
  /// lifetime of \p v.
  static SandboxValue toSandboxValue(const Value &v) {
    if (v.isUndefined())
      return sb::createUndefinedValue();
    if (v.isNull())
      return sb::createNullValue();
    if (v.isBool())
      return sb::createBoolValue(v.getBool());
    if (v.isNumber())
      return sb::createNumberValue(v.getNumber());

    u32 mp = static_cast<const ManagedPointerHolder *>(getPointerValue(v))
                 ->getManagedPointer();
    if (v.isString())
      return sb::createStringValue(mp);
    if (v.isObject())
      return sb::createObjectValue(mp);
    if (v.isSymbol())
      return sb::createSymbolValue(mp);
    if (v.isBigInt())
      return sb::createBigIntValue(mp);

    BUILTIN_UNREACHABLE;
  }

  /// Convert the given jsi::Value \p v to a SandboxValue. The resulting
  /// SandboxValue will need to be explicitly released.
  SandboxValue cloneToSandboxValue(const Value &v) {
    if (v.isUndefined())
      return sb::createUndefinedValue();
    if (v.isNull())
      return sb::createNullValue();
    if (v.isBool())
      return sb::createBoolValue(v.getBool());
    if (v.isNumber())
      return sb::createNumberValue(v.getNumber());

    u32 mp = static_cast<const ManagedPointerHolder *>(getPointerValue(v))
                 ->getManagedPointer();
    if (v.isString())
      return sb::createStringValue(vt_.clone_string(this, srt_, mp));
    if (v.isObject())
      return sb::createObjectValue(vt_.clone_object(this, srt_, mp));
    if (v.isSymbol())
      return sb::createSymbolValue(vt_.clone_symbol(this, srt_, mp));
    if (v.isBigInt())
      return sb::createBigIntValue(vt_.clone_bigint(this, srt_, mp));

    BUILTIN_UNREACHABLE;
  }

 public:
  HermesSandboxRuntimeImpl()
      : bufferVTable_(this),
        growableBufferVTable_(this),
        hostFunctionVTable_(this),
        propNameIDListVTable_(this),
        hostObjectVTable_(this),
        nativeStateVTable_(this),
        managedPointers_(0.5, 0.5) {
    // Grow the function table for the vtable functions we need to register.
    wasm_rt_funcref_table_t &table = w2c_0x5F_indirect_function_table;
    static constexpr size_t kNumVTableFunctions = 10;
    auto oldEndIdx = wasm_rt_grow_funcref_table(
        &table, kNumVTableFunctions, wasm_rt_funcref_null_value);
    auto funcIdx = oldEndIdx;

    // For each vtable function that needs to be exposed to the sandbox,
    // register it and populate its index in the vtable.
    registerFunction(&BufferWrapper::release, funcIdx);
    bufferVTable_->release = funcIdx++;

    registerFunction(&GrowableBufferImpl::try_grow_to, funcIdx);
    growableBufferVTable_->try_grow_to = funcIdx++;

    registerFunction(&HostFunctionWrapper::release, funcIdx);
    hostFunctionVTable_->release = funcIdx++;
    registerFunction(&HostFunctionWrapper::call, funcIdx);
    hostFunctionVTable_->call = funcIdx++;

    registerFunction(&PropNameIDListWrapper::release, funcIdx);
    propNameIDListVTable_->release = funcIdx++;

    registerFunction(&HostObjectWrapper::release, funcIdx);
    hostObjectVTable_->release = funcIdx++;
    registerFunction(&HostObjectWrapper::get, funcIdx);
    hostObjectVTable_->get = funcIdx++;
    registerFunction(&HostObjectWrapper::set, funcIdx);
    hostObjectVTable_->set = funcIdx++;
    registerFunction(&HostObjectWrapper::get_own_keys, funcIdx);
    hostObjectVTable_->get_own_keys = funcIdx++;

    registerFunction(&NativeStateWrapper::release, funcIdx);
    nativeStateVTable_->release = funcIdx++;

    assert(
        funcIdx == oldEndIdx + kNumVTableFunctions &&
        "Wrong number of functions registered");

    auto sbVt =
        sb::Ptr<SandboxVTable>(this, w2c_hermes_get_hermes_abi_vtable(this));

    // Initialize the ABI runtime in the sandbox.
    auto *makeFn = sb::getFunctionPtr<u32(w2c_hermes *, u32)>(
        this, sbVt->make_hermes_runtime);
    srt_ = makeFn(this, 0);

    // Copy the vtable into vt_ for efficient access.
    sb::Ptr<SandboxRuntimeVTable> srtVt(
        this, sb::Ptr<SandboxRuntime>(this, srt_)->vtable);

#define RETRIEVE_VTABLE_FUNCTIONS(name, ret, args) \
  vt_.name = sb::getFunctionPtr<ret args>(this, srtVt->name);
    SANDBOX_CONTEXT_VTABLE_FUNCTIONS(RETRIEVE_VTABLE_FUNCTIONS)
#undef RETRIEVE_VTABLE_FUNCTIONS
  }
  ~HermesSandboxRuntimeImpl() override {
    // Destroy the context first, so any finalizers that may reference vtables
    // or ManagedPointers will run.
    vt_.release(this, srt_);
    assert(managedPointers_.sizeForTests() == 0 && "Dangling references");
    assert(numAllocs_ == 0 && "Dangling heap allocations.");
  }

  void asyncTriggerTimeout() override {
    asyncTimeout_.store(true, std::memory_order_relaxed);
  }

  /// Return true if an asynchronous timeout has been triggered.
  bool testAsyncTimeout() {
    return asyncTimeout_.load(std::memory_order_relaxed);
  }

  /// Clear the asynchronous timeout flag if it's set, and returns its value.
  bool testAndClearAsyncTimeout() {
    // Optimistically try loading first, before doing the more expensive atomic
    // CAS.
    if (!testAsyncTimeout())
      return false;
    return asyncTimeout_.exchange(false, std::memory_order_relaxed);
  }

  Value evaluateJavaScript(
      const std::shared_ptr<const Buffer> &buffer,
      const std::string &sourceURL) override {
    // Allocate and fill a buffer in the sandbox with the contents of the
    // provided jsi::Buffer.
    auto sbuf = BufferWrapper::create(*this, buffer);

    // Allocate and fill a character array in the sandbox for the source URL.
    LIFOAlloc<char> surl(this, sourceURL.size());
    memcpy(&*surl, sourceURL.c_str(), sourceURL.size());

    StackAlloc<SandboxValueOrError> resValueOrError(this);

    // Call the appropriate function based on whether the buffer contains
    // bytecode. Note that the buffer is released since it will now be managed
    // by the runtime.
    if (isHermesBytecode(buffer->data(), buffer->size()))
      vt_.evaluate_hermes_bytecode(
          this, resValueOrError, srt_, sbuf, surl, sourceURL.size());
    else
      vt_.evaluate_javascript_source(
          this, resValueOrError, srt_, sbuf, surl, sourceURL.size());

    return intoJSIValue(*resValueOrError);
  }

  Value evaluateHermesBytecode(
      const std::shared_ptr<const Buffer> &buffer,
      const std::string &sourceURL) override {
    // Allocate and fill a buffer in the sandbox with the contents of the
    // provided jsi::Buffer.
    auto sbuf = BufferWrapper::create(*this, buffer);

    // Allocate and fill a character array in the sandbox for the source URL.
    LIFOAlloc<char> surl(this, sourceURL.size());
    memcpy(&*surl, sourceURL.c_str(), sourceURL.size());

    StackAlloc<SandboxValueOrError> resValueOrError(this);
    vt_.evaluate_hermes_bytecode(
        this, resValueOrError, srt_, sbuf, surl, sourceURL.size());
    return intoJSIValue(*resValueOrError);
  }

  std::shared_ptr<const PreparedJavaScript> prepareJavaScript(
      const std::shared_ptr<const Buffer> &buffer,
      std::string sourceURL) override {
    return std::make_shared<const SourceJavaScriptPreparation>(
        buffer, std::move(sourceURL));
  }

  Value evaluatePreparedJavaScript(
      const std::shared_ptr<const PreparedJavaScript> &js) override {
    assert(dynamic_cast<const SourceJavaScriptPreparation *>(js.get()));
    auto sjp = std::static_pointer_cast<const SourceJavaScriptPreparation>(js);
    return evaluateJavaScript(sjp, sjp->sourceURL());
  }

  void queueMicrotask(const Function & /*callback*/) override {
    THROW_UNIMPLEMENTED();
  }

  bool drainMicrotasks(int maxMicrotasksHint = -1) override {
    SandboxBoolOrError resBoolOrError{
        vt_.drain_microtasks(this, srt_, maxMicrotasksHint)};
    return unwrap(resBoolOrError);
  }

  Object global() override {
    return intoJSIObject(SandboxObject{vt_.get_global_object(this, srt_)});
  }

  std::string description() override {
    return "HermesSandboxRuntime";
  }

  bool isInspectable() override {
    THROW_UNIMPLEMENTED();
  }

  Instrumentation &instrumentation() override {
    THROW_UNIMPLEMENTED();
  }

 protected:
  PointerValue *cloneSymbol(const Runtime::PointerValue *pv) override {
    return clone(pv);
  }
  PointerValue *cloneBigInt(const Runtime::PointerValue *pv) override {
    return clone(pv);
  }
  PointerValue *cloneString(const Runtime::PointerValue *pv) override {
    return clone(pv);
  }
  PointerValue *cloneObject(const Runtime::PointerValue *pv) override {
    return clone(pv);
  }
  PointerValue *clonePropNameID(const Runtime::PointerValue *pv) override {
    return clone(pv);
  }

  PropNameID createPropNameIDFromAscii(const char *str, size_t length)
      override {
    return createPropNameIDFromString(createStringFromAscii(str, length));
  }
  PropNameID createPropNameIDFromUtf8(const uint8_t *utf8, size_t length)
      override {
    return createPropNameIDFromString(createStringFromUtf8(utf8, length));
  }
  PropNameID createPropNameIDFromString(const String &str) override {
    return intoJSIPropNameID(
        SandboxPropNameIDOrError{vt_.create_propnameid_from_string(
            this, srt_, toSandboxString(str).pointer)});
  }
  PropNameID createPropNameIDFromSymbol(const Symbol &sym) override {
    return intoJSIPropNameID(
        SandboxPropNameIDOrError{vt_.create_propnameid_from_symbol(
            this, srt_, toSandboxSymbol(sym).pointer)});
  }
  std::string utf8(const PropNameID &name) override {
    auto outBuf = GrowableBufferImpl::create(*this);
    vt_.get_utf8_from_propnameid(
        this, srt_, toSandboxPropNameID(name).pointer, outBuf);
    auto ret = outBuf->getString(this);
    GrowableBufferImpl::release(this, outBuf);
    return ret;
  }
  bool compare(const PropNameID &lhs, const PropNameID &rhs) override {
    return vt_.prop_name_id_equals(
        this,
        srt_,
        toSandboxPropNameID(lhs).pointer,
        toSandboxPropNameID(rhs).pointer);
  }

  std::string symbolToString(const Symbol &sym) override {
    auto outBuf = GrowableBufferImpl::create(*this);
    vt_.get_utf8_from_symbol(this, srt_, toSandboxSymbol(sym).pointer, outBuf);
    auto ret = outBuf->getString(this);
    GrowableBufferImpl::release(this, std::move(outBuf));
    return ret;
  }

  BigInt createBigIntFromInt64(int64_t i) override {
    return intoJSIBigInt(
        SandboxBigIntOrError{vt_.create_bigint_from_int64(this, srt_, i)});
  }
  BigInt createBigIntFromUint64(uint64_t u) override {
    return intoJSIBigInt(
        SandboxBigIntOrError{vt_.create_bigint_from_uint64(this, srt_, u)});
  }
  bool bigintIsInt64(const BigInt &bi) override {
    return vt_.bigint_is_int64(this, srt_, toSandboxBigInt(bi).pointer);
  }
  bool bigintIsUint64(const BigInt &bi) override {
    return vt_.bigint_is_uint64(this, srt_, toSandboxBigInt(bi).pointer);
  }
  uint64_t truncate(const BigInt &bi) override {
    return vt_.bigint_truncate_to_uint64(
        this, srt_, toSandboxBigInt(bi).pointer);
  }
  String bigintToString(const BigInt &bi, int radix) override {
    return intoJSIString(SandboxStringOrError{vt_.bigint_to_string(
        this, srt_, toSandboxBigInt(bi).pointer, (u32)radix)});
  }

  String createStringFromAscii(const char *str, size_t length) override {
    return createStringFromUtf8((const uint8_t *)str, length);
  }
  String createStringFromUtf8(const uint8_t *utf8, size_t length) override {
    LIFOAlloc<char> s(this, length);
    memcpy(&*s, utf8, length);
    SandboxStringOrError res{
        vt_.create_string_from_utf8(this, srt_, s, length)};
    return intoJSIString(res);
  }
  std::string utf8(const String &str) override {
    auto outBuf = GrowableBufferImpl::create(*this);
    vt_.get_utf8_from_string(this, srt_, toSandboxString(str).pointer, outBuf);
    auto ret = outBuf->getString(this);
    GrowableBufferImpl::release(this, std::move(outBuf));
    return ret;
  }

  Object createObject() override {
    return intoJSIObject(SandboxObjectOrError{vt_.create_object(this, srt_)});
  }
  Object createObject(std::shared_ptr<HostObject> ho) override {
    auto how = HostObjectWrapper::create(*this, std::move(ho));
    return intoJSIObject(SandboxObjectOrError{
        vt_.create_object_from_host_object(this, srt_, how)});
  }
  std::shared_ptr<HostObject> getHostObject(const Object &obj) override {
    u32 how = vt_.get_host_object(this, srt_, toSandboxObject(obj).pointer);
    return sb::Ptr<HostObjectWrapper>(this, how)->getHostObject(*this);
  }
  HostFunctionType &getHostFunction(const Function &fn) override {
    u32 hfw = vt_.get_host_function(this, srt_, toSandboxFunction(fn).pointer);
    return sb::Ptr<HostFunctionWrapper>(this, hfw)->getHostFunction(*this);
  }

  bool hasNativeState(const Object &obj) override {
    return vt_.get_native_state(this, srt_, toSandboxObject(obj).pointer);
  }
  std::shared_ptr<NativeState> getNativeState(const Object &obj) override {
    assert(hasNativeState(obj));
    u32 nsw = vt_.get_native_state(this, srt_, toSandboxObject(obj).pointer);
    return sb::Ptr<NativeStateWrapper>(this, nsw)->getNativeState(*this);
  }
  void setNativeState(const Object &obj, std::shared_ptr<NativeState> state)
      override {
    auto nsw = NativeStateWrapper::create(*this, std::move(state));
    SandboxVoidOrError resVoidOrError{
        vt_.set_native_state(this, srt_, toSandboxObject(obj).pointer, nsw)};
    unwrap(resVoidOrError);
  }

  Value getProperty(const Object &obj, const PropNameID &name) override {
    StackAlloc<SandboxValueOrError> resValueOrError(this);
    vt_.get_object_property_from_propnameid(
        this,
        resValueOrError,
        srt_,
        toSandboxObject(obj).pointer,
        toSandboxPropNameID(name).pointer);
    return intoJSIValue(*resValueOrError);
  }
  Value getProperty(const Object &obj, const String &name) override {
    return getProperty(obj, createPropNameIDFromString(name));
  }
  bool hasProperty(const Object &obj, const PropNameID &name) override {
    SandboxBoolOrError resBoolOrError{vt_.has_object_property_from_propnameid(
        this,
        srt_,
        toSandboxObject(obj).pointer,
        toSandboxPropNameID(name).pointer)};
    return unwrap(resBoolOrError);
  }
  bool hasProperty(const Object &obj, const String &name) override {
    return hasProperty(obj, createPropNameIDFromString(name));
  }
  void setPropertyValue(
      const Object &obj,
      const PropNameID &name,
      const Value &value) override {
    StackAlloc<SandboxValue> propVal(this);
    *propVal = toSandboxValue(value);

    SandboxVoidOrError resVoidOrError{vt_.set_object_property_from_propnameid(
        this,
        srt_,
        toSandboxObject(obj).pointer,
        toSandboxPropNameID(name).pointer,
        /* value */ propVal)};
    unwrap(resVoidOrError);
  }
  void setPropertyValue(
      const Object &obj,
      const String &name,
      const Value &value) override {
    setPropertyValue(obj, createPropNameIDFromString(name), value);
  }

  bool isArray(const Object &obj) const override {
    return vt_.object_is_array(getMutMod(), srt_, toSandboxObject(obj).pointer);
  }
  bool isArrayBuffer(const Object &obj) const override {
    return vt_.object_is_arraybuffer(
        getMutMod(), srt_, toSandboxObject(obj).pointer);
  }
  bool isFunction(const Object &obj) const override {
    return vt_.object_is_function(
        getMutMod(), srt_, toSandboxObject(obj).pointer);
  }
  bool isHostObject(const Object &obj) const override {
    return vt_.get_host_object(getMutMod(), srt_, toSandboxObject(obj).pointer);
  }
  bool isHostFunction(const Function &fn) const override {
    return vt_.get_host_function(
        getMutMod(), srt_, toSandboxFunction(fn).pointer);
  }
  Array getPropertyNames(const Object &obj) override {
    return intoJSIArray(SandboxArrayOrError{vt_.get_object_property_names(
        this, srt_, toSandboxObject(obj).pointer)});
  }

  WeakObject createWeakObject(const Object &obj) override {
    return intoJSIWeakObject(SandboxWeakObjectOrError{
        vt_.create_weak_object(this, srt_, toSandboxObject(obj).pointer)});
  }
  Value lockWeakObject(const WeakObject &wo) override {
    StackAlloc<SandboxValue> resValue(this);
    vt_.lock_weak_object(this, resValue, srt_, toSandboxWeakObject(wo).pointer);
    return intoJSIValue(*resValue);
  }

  Array createArray(size_t length) override {
    return intoJSIArray(
        SandboxArrayOrError{vt_.create_array(this, srt_, length)});
  }
  ArrayBuffer createArrayBuffer(
      std::shared_ptr<MutableBuffer> buffer) override {
    THROW_UNIMPLEMENTED();
  }
  size_t size(const Array &arr) override {
    return vt_.get_array_length(this, srt_, toSandboxArray(arr).pointer);
  }
  size_t size(const ArrayBuffer &ab) override {
    StackAlloc<SandboxSizeTOrError> resSizeTOrError(this);
    vt_.get_arraybuffer_size(
        this, resSizeTOrError, srt_, toSandboxArrayBuffer(ab).pointer);
    return unwrap(*resSizeTOrError);
  }
  uint8_t *data(const ArrayBuffer &) override {
    THROW_UNIMPLEMENTED();
  }
  Value getValueAtIndex(const Array &arr, size_t i) override {
    if (i >= arr.length(*this))
      throw JSINativeException("Array index out of bounds.");

    StackAlloc<SandboxValueOrError> resValueOrError(this);
    StackAlloc<SandboxValue> sbKey(this);
    *sbKey = sb::createNumberValue(i);
    vt_.get_object_property_from_value(
        this, resValueOrError, srt_, toSandboxObject(arr).pointer, sbKey);
    return intoJSIValue(*resValueOrError);
  }
  void setValueAtIndexImpl(const Array &arr, size_t i, const Value &value)
      override {
    if (i >= arr.length(*this))
      throw JSINativeException("Array index out of bounds.");

    StackAlloc<SandboxValue> sbVal(this);
    StackAlloc<SandboxValue> sbKey(this);
    *sbKey = sb::createNumberValue(i);
    *sbVal = toSandboxValue(value);
    SandboxVoidOrError resVoidOrError{vt_.set_object_property_from_value(
        this, srt_, toSandboxObject(arr).pointer, sbKey, sbVal)};
    unwrap(resVoidOrError);
  }

  Function createFunctionFromHostFunction(
      const PropNameID &name,
      unsigned int paramCount,
      HostFunctionType func) override {
    auto hfw = HostFunctionWrapper::create(*this, std::move(func));
    return intoJSIFunction(
        SandboxFunctionOrError{vt_.create_function_from_host_function(
            this, srt_, toSandboxPropNameID(name).pointer, paramCount, hfw)});
  }
  Value call(
      const Function &fn,
      const Value &jsThis,
      const Value *args,
      size_t count) override {
    // Convert the arguments from JSI values to SandboxValues and write them
    // into the sandbox heap. We don't need to clone the values since they only
    // remain live during this function.
    LIFOAlloc<SandboxValue> sbArgs(this, count);
    for (size_t i = 0; i < count; ++i)
      sbArgs[i] = toSandboxValue(args[i]);

    StackAlloc<SandboxValue> jsThisValue(this);
    StackAlloc<SandboxValueOrError> resValueOrError(this);
    *jsThisValue = toSandboxValue(jsThis);

    vt_.call(
        this,
        resValueOrError,
        srt_,
        toSandboxFunction(fn).pointer,
        /* this */ jsThisValue,
        sbArgs,
        count);
    return intoJSIValue(*resValueOrError);
  }
  Value callAsConstructor(const Function &fn, const Value *args, size_t count)
      override {
    // Convert the arguments from JSI values to SandboxValues and write them
    // into the sandbox heap. We don't need to clone the values since they only
    // remain live during this function.
    LIFOAlloc<SandboxValue> sbArgs(this, count);
    for (size_t i = 0; i < count; ++i)
      sbArgs[i] = toSandboxValue(args[i]);

    StackAlloc<SandboxValueOrError> resValueOrError(this);
    vt_.call_as_constructor(
        this,
        resValueOrError,
        srt_,
        toSandboxFunction(fn).pointer,
        sbArgs,
        count);
    return intoJSIValue(*resValueOrError);
  }

  bool strictEquals(const Symbol &a, const Symbol &b) const override {
    return vt_.strict_equals_symbol(
        getMutMod(),
        srt_,
        toSandboxSymbol(a).pointer,
        toSandboxSymbol(b).pointer);
  }
  bool strictEquals(const BigInt &a, const BigInt &b) const override {
    return vt_.strict_equals_bigint(
        getMutMod(),
        srt_,
        toSandboxBigInt(a).pointer,
        toSandboxBigInt(b).pointer);
  }
  bool strictEquals(const String &a, const String &b) const override {
    return vt_.strict_equals_string(
        getMutMod(),
        srt_,
        toSandboxString(a).pointer,
        toSandboxString(b).pointer);
  }
  bool strictEquals(const Object &a, const Object &b) const override {
    return vt_.strict_equals_object(
        getMutMod(),
        srt_,
        toSandboxObject(a).pointer,
        toSandboxObject(b).pointer);
  }

  bool instanceOf(const Object &o, const Function &f) override {
    SandboxBoolOrError resBoolOrError{vt_.instance_of(
        this, srt_, toSandboxObject(o).pointer, toSandboxFunction(f).pointer)};
    return unwrap(resBoolOrError);
  }

  void setExternalMemoryPressure(const Object &obj, size_t amount) override {
    SandboxVoidOrError resVoidOrError{vt_.set_object_external_memory_pressure(
        this, srt_, toSandboxObject(obj).pointer, amount)};
    unwrap(resVoidOrError);
  }
};

} // namespace

/// Provide implementations for the functions imported by the sandbox.

extern "C" {

/// These definitions comes from wasi/api.h in Emscripten.
#define WASI_EINVAL 28
#define WASI_ENOSYS 52
#define WASI_CLOCKID_REALTIME 0
#define WASI_CLOCKID_MONOTONIC 1

/* import: 'wasi_snapshot_preview1' 'environ_get' */
u32 w2c_wasi__snapshot__preview1_environ_get(
    struct w2c_wasi__snapshot__preview1 *,
    u32,
    u32) {
  return WASI_ENOSYS;
}

/* import: 'wasi_snapshot_preview1' 'environ_sizes_get' */
u32 w2c_wasi__snapshot__preview1_environ_sizes_get(
    struct w2c_wasi__snapshot__preview1 *,
    u32,
    u32) {
  return WASI_ENOSYS;
}

/* import: 'wasi_snapshot_preview1' 'fd_close' */
u32 w2c_wasi__snapshot__preview1_fd_close(
    struct w2c_wasi__snapshot__preview1 *,
    u32) {
  return WASI_ENOSYS;
}

/* import: 'wasi_snapshot_preview1' 'fd_fdstat_get' */
u32 w2c_wasi__snapshot__preview1_fd_fdstat_get(
    struct w2c_wasi__snapshot__preview1 *,
    u32,
    u32) {
  return WASI_ENOSYS;
}

/* import: 'wasi_snapshot_preview1' 'fd_seek' */
u32 w2c_wasi__snapshot__preview1_fd_seek(
    struct w2c_wasi__snapshot__preview1 *,
    u32,
    u64,
    u32,
    u32) {
  return WASI_ENOSYS;
}

/* import: 'wasi_snapshot_preview1' 'fd_write' */
u32 w2c_wasi__snapshot__preview1_fd_write(
    struct w2c_wasi__snapshot__preview1 *,
    u32,
    u32,
    u32,
    u32) {
  return WASI_ENOSYS;
}

/* import: 'wasi_snapshot_preview1' 'clock_time_get' */
u32 w2c_wasi__snapshot__preview1_clock_time_get(
    struct w2c_wasi__snapshot__preview1 *modPtr,
    u32 clock_id,
    u64 max_lag,
    u32 out) {
  auto *mod = reinterpret_cast<w2c_hermes *>(modPtr);
  sb::Ptr<u64> outPtr(mod, out);

  // Only allow access to the time at a millisecond granularity.
  std::chrono::milliseconds timeMs;
  if (clock_id == WASI_CLOCKID_REALTIME) {
    timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
  } else if (clock_id == WASI_CLOCKID_MONOTONIC) {
    timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch());
  } else {
    return WASI_EINVAL;
  }
  static constexpr u64 nsPerMs = 1000 * 1000;
  *outPtr = timeMs.count() * nsPerMs;
  return 0;
}

/* import: 'env' 'emscripten_notify_memory_growth' */
void w2c_env_emscripten_notify_memory_growth(struct w2c_env *, u32) {}

/* import: 'hermes_import' 'getentropy' */
u32 w2c_hermes__import_getentropy(
    struct w2c_hermes__import *modPtr,
    u32 buffer,
    u32 length) {
  auto *mod = reinterpret_cast<w2c_hermes *>(modPtr);
  sb::Ptr<char> bufPtr(mod, buffer, length);
  auto r = std::random_device()();
  memcpy(&*bufPtr, &r, std::min<size_t>(sizeof(r), length));
  return 0;
}

/* import: 'hermes_import' 'test_timeout' */
u32 w2c_hermes__import_test_timeout(struct w2c_hermes__import *modPtr) {
  auto *mod = reinterpret_cast<w2c_hermes *>(modPtr);
  return static_cast<HermesSandboxRuntimeImpl *>(mod)->testAsyncTimeout();
}

/* import: 'hermes_import' 'test_and_clear_timeout' */
u32 w2c_hermes__import_test_and_clear_timeout(
    struct w2c_hermes__import *modPtr) {
  auto *mod = reinterpret_cast<w2c_hermes *>(modPtr);
  return static_cast<HermesSandboxRuntimeImpl *>(mod)
      ->testAndClearAsyncTimeout();
}

/* import: 'wasi_snapshot_preview1' 'proc_exit' */
void w2c_wasi__snapshot__preview1_proc_exit(
    struct w2c_wasi__snapshot__preview1 *,
    u32) {
  abort();
}
}

namespace facebook {
namespace hermes {

/*static*/ bool HermesSandboxRuntime::isHermesBytecode(
    const uint8_t *data,
    size_t len) {
  // "Hermes" in ancient Greek encoded in UTF-16BE and truncated to 8 bytes.
  constexpr uint64_t MAGIC = 0x1F1903C103BC1FC6;
  return (len >= sizeof(MAGIC) && memcmp(data, &MAGIC, sizeof(MAGIC)) == 0);
}

std::unique_ptr<HermesSandboxRuntime> makeHermesSandboxRuntime() {
  return std::make_unique<HermesSandboxRuntimeImpl>();
}

} // namespace hermes
} // namespace facebook
