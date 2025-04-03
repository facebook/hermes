/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT license.
 *
 * Copyright notices for portions of code adapted from Node.js and V8 projects:
 *
 * Copyright Node.js contributors. All rights reserved.
 * https://github.com/nodejs/node/blob/master/LICENSE
 *
 * Copyright 2011 the V8 project authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 * https://github.com/v8/v8/blob/main/LICENSE
 */

//
// Implementation of Node-API for Hermes engine.
//
// The Node-API C functions are redirecting all calls to the NodeApiEnvironment
// class which implements the API details.
// The most notable parts of the implementation are:
// - The NodeApiEnvironment class is ref-counted.
// - It maintains local stack-based GC roots as napiValueStack_.
//   - The napiValueStackScopes_ is used to control napiValueStack_ handle
//   scopes.
//   - The napiValueStack_ and napiValueStackScopes_ are instances of
//     NodeApiStableAddressStack to maintain stable address of returned
//     napi_value and handle scopes.
//   - napi_value is a pointer to the vm::PinnedHermesValue stored in
//     napiValueStack_.
// - The heap-based GC roots are in the references_ and finalizingReferences_.
//   - references_ vs finalizingReferences_ is chosen based on whether the root
//   needs
//     finalizer call or not.
//   - references_ and finalizingReferences_ are double-linked list.
//   - All heap-based GC roots are stored as references - instances of classes
//     derived from NodeApiReference class. There are many varieties of that
//     class to accommodate different lifetime strategies and to optimize
//     storage size.
//   - napi_ref and napi_ext_ref are pointers to references_ and
//   finalizingReferences_
//     items.
//   - NodeApiReference finalizers are run in JS thread by processFinalizerQueue
//     method which is called by NodeApiHandleScope::setResult.
// - Each returned error status is backed up by the extended error message
//   stored in lastError_ that can be retrieved by napi_get_last_error_info.
// - We use macros to handle error statuses. It is done to reduce extensive use
//   of "if-return" statements, and to report failing expressions along with the
//   file name and code line number.

// TODO: Update with the latest Node.js Node-API implementation that uses the
// version info.

// TODO: Allow DebugBreak in unexpected cases - add functions to indicate
//       expected errors
// TODO: Create NodeApiEnvironment with JSI Runtime
// TODO: Fix Inspector CMake definitions

// TODO: Cannot use functions as a base class
// TODO: NativeFunction vs NativeConstructor
// TODO: Different error messages
// TODO: Arrays with 2^32-1 elements (sparse arrays?)
// TODO: How to provide detailed error messages without breaking tests?
// TODO: Why console.log compiles in V8_JSI?

#include "hermes_node_api.h"

#include "hermes/BCGen/HBC/BytecodeProviderFromSrc.h"
#include "hermes/SourceMap/SourceMapParser.h"
#include "hermes/Support/SimpleDiagHandler.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/HostModel.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSArrayBuffer.h"
#include "hermes/VM/JSDataView.h"
#include "hermes/VM/JSDate.h"
#include "hermes/VM/JSError.h"
#include "hermes/VM/JSProxy.h"
#include "hermes/VM/JSTypedArray.h"
#include "hermes/VM/PropertyAccessor.h"
#include "llvh/ADT/SmallVector.h"
#include "llvh/Support/ConvertUTF.h"

#include <algorithm>
#include <atomic>
#include <unordered_map>

//=============================================================================
// Macros
//=============================================================================

// Check the Node-API status and return it if it is not napi_ok.
#define CHECK_NAPI(...)                         \
  do {                                          \
    if (napi_status status__ = (__VA_ARGS__)) { \
      return status__;                          \
    }                                           \
  } while (false)

// TODO: Provide better cross plat alternative. Can we use an existing Hermes
// function here? Crash if the condition is false.
#if defined(_WIN32)
#define CRASH_IF_FALSE(condition)  \
  do {                             \
    if (!(condition)) {            \
      assert(false && #condition); \
      *((int *)nullptr) = 1;       \
      std::terminate();            \
    }                              \
  } while (false)
#else
#define CRASH_IF_FALSE(condition)  \
  do {                             \
    if (!(condition)) {            \
      assert(false && #condition); \
      __builtin_trap();            \
      std::terminate();            \
    }                              \
  } while (false)
#endif

// Return error status with message.
#define ERROR_STATUS(status, ...) \
  env.setLastNativeError(         \
      (status), (__FILE__), (uint32_t)(__LINE__), __VA_ARGS__)

// Return napi_generic_failure with message.
#define GENERIC_FAILURE(...) ERROR_STATUS(napi_generic_failure, __VA_ARGS__)

// Cast env to NodeApiEnvironment if it is not null.
#define CHECKED_ENV(env) \
  ((env) == nullptr)     \
      ? napi_invalid_arg \
      : reinterpret_cast<hermes::node_api::NodeApiEnvironment *>(env)

// Check env and return error status with message.
#define CHECKED_ENV_ERROR_STATUS(env, status, ...)                    \
  ((env) == nullptr)                                                  \
      ? napi_invalid_arg                                              \
      : reinterpret_cast<hermes::node_api::NodeApiEnvironment *>(env) \
            ->setLastNativeError(                                     \
                (status), (__FILE__), (uint32_t)(__LINE__), __VA_ARGS__)

// Check env and return napi_generic_failure with message.
#define CHECKED_ENV_GENERIC_FAILURE(env, ...) \
  CHECKED_ENV_ERROR_STATUS(env, napi_generic_failure, __VA_ARGS__)

// Check conditions and return error status with message if it is false.
#define RETURN_STATUS_IF_FALSE_WITH_MESSAGE(condition, status, ...) \
  do {                                                              \
    if (!(condition)) {                                             \
      return env.setLastNativeError(                                \
          (status), (__FILE__), (uint32_t)(__LINE__), __VA_ARGS__); \
    }                                                               \
  } while (false)

// Check conditions and return error status if it is false.
#define RETURN_STATUS_IF_FALSE(condition, status) \
  RETURN_STATUS_IF_FALSE_WITH_MESSAGE(            \
      (condition), (status), "Condition is false: " #condition)

// Check conditions and return napi_generic_failure if it is false.
#define RETURN_FAILURE_IF_FALSE(condition) \
  RETURN_STATUS_IF_FALSE_WITH_MESSAGE(     \
      (condition), napi_generic_failure, "Condition is false: " #condition)

// Check that the argument is not nullptr.
#define CHECK_ARG(arg)                 \
  RETURN_STATUS_IF_FALSE_WITH_MESSAGE( \
      (arg) != nullptr, napi_invalid_arg, "Argument is null: " #arg)

// Check that the argument is of Object or Function type.
#define CHECK_OBJECT_ARG(arg)                \
  do {                                       \
    CHECK_ARG(arg);                          \
    RETURN_STATUS_IF_FALSE_WITH_MESSAGE(     \
        phv(arg)->isObject(),                \
        napi_object_expected,                \
        "Argument is not an Object: " #arg); \
  } while (false)

// Check that the argument is of String type.
#define CHECK_STRING_ARG(arg)               \
  do {                                      \
    CHECK_ARG(arg);                         \
    RETURN_STATUS_IF_FALSE_WITH_MESSAGE(    \
        phv(arg)->isString(),               \
        napi_string_expected,               \
        "Argument is not a String: " #arg); \
  } while (false)

#define RAISE_ERROR_IF_FALSE(condition, message)                            \
  do {                                                                      \
    if (!(condition)) {                                                     \
      return runtime.raiseTypeError(message " Condition: " u## #condition); \
    }                                                                       \
  } while (false)

#if defined(_WIN32) && !defined(NDEBUG)
extern "C" __declspec(dllimport) void __stdcall DebugBreak();
#endif

namespace hermes::node_api {

//=============================================================================
// Forward declaration of all classes.
//=============================================================================

class NodeApiCallbackInfo;
class NodeApiDoubleConversion;
class NodeApiEnvironment;
class NodeApiExternalBuffer;
class NodeApiExternalValue;
class NodeApiHandleScope;
class NodeApiHostFunctionContext;
template <class T>
class NodeApiOrderedSet;
class NodeApiPendingFinalizers;
template <class T>
class NodeApiRefCountedPtr;
class NodeApiScriptModel;
template <class T>
class NodeApiStableAddressStack;
class NodeApiStringBuilder;

// Forward declaration of NodeApiReference-related classes.
class NodeApiAtomicRefCountReference;
class NodeApiComplexReference;
template <class TBaseReference>
class NodeApiFinalizeCallbackHolder;
template <class TBaseReference>
class NodeApiFinalizeHintHolder;
class NodeApiFinalizer;
class NodeApiFinalizingAnonymousReference;
class NodeApiFinalizingComplexReference;
template <class TBaseReference>
class NodeApiFinalizingReference;
template <class TReference>
class NodeApiFinalizingReferenceFactory;
class NodeApiFinalizingStrongReference;
class NodeApiInstanceData;
template <class T>
class NodeApiLinkedList;
template <class TBaseReference>
class NodeApiNativeDataHolder;
class NodeApiReference;
class NodeApiStrongReference;
class NodeApiWeakReference;

using NodeApiNativeError = napi_extended_error_info;

//=============================================================================
// Enums
//=============================================================================

// Controls behavior of NodeApiEnvironment::unwrapObject.
enum class NodeApiUnwrapAction { KeepWrap, RemoveWrap };

// Predefined values used by NodeApiEnvironment.
enum class NodeApiPredefined {
  Promise,
  allRejections,
  code,
  hostFunction,
  napi_externalValue,
  napi_typeTag,
  onHandled,
  onUnhandled,
  reject,
  resolve,
  PredefinedCount // a special value that must be last in the enum
};

// The action to take when an external value is not found.
enum class NodeApiIfNotFound {
  ThenCreate,
  ThenReturnNull,
};

//=============================================================================
// Forward declaration of standalone functions.
//=============================================================================

// Size of an array - it must be replaced by std::size after switching to C++17
template <class T, std::size_t N>
constexpr std::size_t size(const T (&array)[N]) noexcept;

// Check if the enum value is in the provided range.
template <class TEnum>
bool isInEnumRange(
    TEnum value,
    TEnum lowerBoundInclusive,
    TEnum upperBoundInclusive) noexcept;

// Reinterpret cast NodeApiEnvironment to napi_env
napi_env napiEnv(NodeApiEnvironment *env) noexcept;

// Reinterpret cast vm::PinnedHermesValue pointer to napi_value
napi_value napiValue(const vm::PinnedHermesValue *value) noexcept;

// Get underlying vm::PinnedHermesValue and reinterpret cast it napi_value
template <class T>
napi_value napiValue(vm::Handle<T> value) noexcept;

// Reinterpret cast napi_value to vm::PinnedHermesValue pointer
const vm::PinnedHermesValue *phv(napi_value value) noexcept;
// Useful in templates and macros
const vm::PinnedHermesValue *phv(const vm::PinnedHermesValue *value) noexcept;

// Reinterpret cast napi_ref to NodeApiReference pointer
NodeApiReference *asReference(napi_ref ref) noexcept;
// Reinterpret cast void* to NodeApiReference pointer
NodeApiReference *asReference(void *ref) noexcept;

// Reinterpret cast to NodeApiHostFunctionContext::NodeApiCallbackInfo
NodeApiCallbackInfo *asCallbackInfo(napi_callback_info callbackInfo) noexcept;

// Get object from HermesValue and cast it to JSObject
vm::JSObject *getObjectUnsafe(const vm::HermesValue &value) noexcept;

// Get object from napi_value and cast it to JSObject
vm::JSObject *getObjectUnsafe(napi_value value) noexcept;

// Copy ASCII input to UTF8 buffer. It is a convenience function to match the
// convertUTF16ToUTF8WithReplacements signature when using std::copy.
size_t copyASCIIToUTF8(
    llvh::ArrayRef<char> input,
    char *buf,
    size_t maxCharacters) noexcept;

// Return length of UTF-8 string after converting a UTF-16 encoded string \p
// input to UTF-8, replacing unpaired surrogates halves with the Unicode
// replacement character. The length does not include the terminating '\0'
// character.
size_t utf8LengthWithReplacements(llvh::ArrayRef<char16_t> input);

// Convert a UTF-16 encoded string \p input to UTF-8 stored in \p buf,
// replacing unpaired surrogates halves with the Unicode replacement character.
// The terminating '\0' is not written.
// \return number of bytes written to \p buf.
size_t convertUTF16ToUTF8WithReplacements(
    llvh::ArrayRef<char16_t> input,
    char *buf,
    size_t bufSize);

//=============================================================================
// Definitions of classes and structs.
//=============================================================================

struct NodeApiAttachTag {
} attachTag;

// A smart pointer for types that implement intrusive ref count using
// methods incRefCount and decRefCount.
template <typename T>
class NodeApiRefCountedPtr final {
 public:
  NodeApiRefCountedPtr() noexcept = default;

  explicit NodeApiRefCountedPtr(T *ptr, NodeApiAttachTag) noexcept
      : ptr_(ptr) {}

  NodeApiRefCountedPtr(const NodeApiRefCountedPtr &other) noexcept
      : ptr_(other.ptr_) {
    if (ptr_ != nullptr) {
      ptr_->incRefCount();
    }
  }

  NodeApiRefCountedPtr(NodeApiRefCountedPtr &&other)
      : ptr_(std::exchange(other.ptr_, nullptr)) {}

  ~NodeApiRefCountedPtr() noexcept {
    if (ptr_ != nullptr) {
      ptr_->decRefCount();
    }
  }

  NodeApiRefCountedPtr &operator=(std::nullptr_t) noexcept {
    if (ptr_ != nullptr) {
      ptr_->decRefCount();
    }
    ptr_ = nullptr;
    return *this;
  }

  NodeApiRefCountedPtr &operator=(const NodeApiRefCountedPtr &other) noexcept {
    if (this != &other) {
      NodeApiRefCountedPtr temp(std::move(*this));
      ptr_ = other.ptr_;
      if (ptr_ != nullptr) {
        ptr_->incRefCount();
      }
    }
    return *this;
  }

  NodeApiRefCountedPtr &operator=(NodeApiRefCountedPtr &&other) noexcept {
    if (this != &other) {
      NodeApiRefCountedPtr temp(std::move(*this));
      ptr_ = std::exchange(other.ptr_, nullptr);
    }
    return *this;
  }

  T *operator->() noexcept {
    return ptr_;
  }

 private:
  T *ptr_{};
};

// Stack of elements where the address of items is not changed as we add new
// values. It is achieved by keeping a SmallVector of the ChunkSize arrays
// called chunks. We use it to keep addresses of GC roots associated with the
// call stack and the related handle scopes. The GC roots are the
// vm::PinnedHermesValue instances. Considering our use case, we do not call the
// destructors for items and require that T has a trivial destructor.
template <class T>
class NodeApiStableAddressStack final {
  static_assert(
      std::is_trivially_destructible_v<T>,
      "T must be trivially destructible.");

 public:
  NodeApiStableAddressStack() noexcept {
    // There is always at least one chunk in the storage
    storage_.emplace_back(new T[ChunkSize]);
  }

  template <class... TArgs>
  void emplace(TArgs &&...args) noexcept {
    size_t newIndex = size_;
    size_t chunkIndex = newIndex / ChunkSize;
    size_t chunkOffset = newIndex % ChunkSize;
    if (chunkOffset == 0 && chunkIndex == storage_.size()) {
      storage_.emplace_back(new T[ChunkSize]);
    }
    new (std::addressof(storage_[chunkIndex][chunkOffset]))
        T(std::forward<TArgs>(args)...);
    ++size_;
  }

  void pop() noexcept {
    CRASH_IF_FALSE(size_ > 0 && "Size must be non zero.");
    --size_;
    reduceChunkCount();
  }

  void resize(size_t newSize) noexcept {
    CRASH_IF_FALSE(newSize <= size_ && "Size cannot be increased by resizing.");
    if (newSize < size_) {
      size_ = newSize;
      reduceChunkCount();
    }
  }

  size_t size() const noexcept {
    return size_;
  }

  bool empty() const noexcept {
    return size_ == 0;
  }

  T &top() noexcept {
    CRASH_IF_FALSE(size_ > 0 && "Size must be non zero.");
    size_t lastIndex = size_ - 1;
    return storage_[lastIndex / ChunkSize][lastIndex % ChunkSize];
  }

  T &operator[](size_t index) noexcept {
    CRASH_IF_FALSE(index < size_ && "Index must be less than size.");
    return storage_[index / ChunkSize][index % ChunkSize];
  }

  template <class F>
  void forEach(const F &f) noexcept {
    size_t remaining = size_;
    for (std::unique_ptr<T[]> &chunk : storage_) {
      size_t chunkSize = std::min(ChunkSize, remaining);
      for (size_t i = 0; i < chunkSize; ++i) {
        f(chunk[i]);
      }
      remaining -= chunkSize;
    }
  }

 private:
  void reduceChunkCount() noexcept {
    // There must be at least one chunk.
    // To reduce number of allocations/deallocations the last chunk must be half
    // full before we delete the next empty chunk.
    size_t requiredChunkCount = std::max<size_t>(
        1, (size_ + ChunkSize / 2 + ChunkSize - 1) / ChunkSize);
    if (requiredChunkCount < storage_.size()) {
      storage_.resize(requiredChunkCount);
    }
  }

 private:
  // The size of 64 entries per chunk is arbitrary at this point.
  // It can be adjusted depending on perf data.
  static const size_t ChunkSize = 64;

  llvh::SmallVector<std::unique_ptr<T[]>, ChunkSize> storage_;
  size_t size_{0};
};

// An intrusive double linked list of items.
// Items in the list must inherit from NodeApiLinkedList<T>::Item.
// We use it instead of std::list to allow item to delete itself in its
// destructor and conveniently move items from list to another. The
// NodeApiLinkedList is used for References - the GC roots that are allocated in
// heap. The GC roots are the vm::PinnedHermesValue instances.
template <class T>
class NodeApiLinkedList final {
 public:
  NodeApiLinkedList() noexcept {
    // The list is circular:
    // head.next_ points to the first item
    // head.prev_ points to the last item
    head_.next_ = &head_;
    head_.prev_ = &head_;
  }

  class Item {
   public:
    void linkNext(T *item) noexcept {
      if (item->isLinked()) {
        item->unlink();
      }
      item->prev_ = this;
      item->next_ = next_;
      item->next_->prev_ = item;
      next_ = item;
    }

    void unlink() noexcept {
      if (isLinked()) {
        prev_->next_ = next_;
        next_->prev_ = prev_;
        prev_ = nullptr;
        next_ = nullptr;
      }
    }

    bool isLinked() const noexcept {
      return prev_ != nullptr;
    }

    friend NodeApiLinkedList;

   private:
    Item *next_{};
    Item *prev_{};
  };

  void pushFront(T *item) noexcept {
    head_.linkNext(item);
  }

  void pushBack(T *item) noexcept {
    head_.prev_->linkNext(item);
  }

  T *begin() noexcept {
    return static_cast<T *>(head_.next_);
  }

  // The end() returns a pointer to an invalid object.
  T *end() noexcept {
    return static_cast<T *>(&head_);
  }

  bool isEmpty() noexcept {
    return head_.next_ == head_.prev_;
  }

  template <class TLambda>
  void forEach(TLambda lambda) noexcept {
    for (T *item = begin(); item != end();) {
      // lambda can delete the item - get the next one before calling it.
      T *nextItem = static_cast<T *>(item->next_);
      lambda(item);
      item = nextItem;
    }
  }

 private:
  Item head_;
};

// The main class representing the Node-API environment.
// All Node-API functions are calling methods from this class.
class NodeApiEnvironment final {
 public:
  // Initializes a new instance of NodeApiEnvironment.
  explicit NodeApiEnvironment(
      vm::Runtime &runtime,
      int32_t apiVersion) noexcept;

  ~NodeApiEnvironment();

 public:
  // TODO: Remove the ref count
  // Exported function to increment the ref count by one.
  napi_status incRefCount() noexcept;

  // Exported function to decrement the ref count by one.
  // When the ref count becomes zero, the environment is deleted.
  napi_status decRefCount() noexcept;

  // Internal function to get the Hermes runtime.
  vm::Runtime &runtime() noexcept;

  // Internal function to get the stack of napi_value.
  NodeApiStableAddressStack<vm::PinnedHermesValue> &napiValueStack() noexcept;

  //---------------------------------------------------------------------------
  // Native error handling methods
  //---------------------------------------------------------------------------
 public:
  // Exported function to get the last native error.
  napi_status getLastNativeError(const NodeApiNativeError **result) noexcept;

  // Internal function to se the last native error.
  template <class... TArgs>
  napi_status setLastNativeError(
      napi_status status,
      const char *fileName,
      uint32_t line,
      TArgs &&...args) noexcept;

  // Internal function to clear the last error function.
  napi_status clearLastNativeError() noexcept;

  //-----------------------------------------------------------------------------
  // Methods to support JS error handling
  //-----------------------------------------------------------------------------
 public:
  // Internal function to create JS error with the specified prototype.
  napi_status createJSError(
      const vm::PinnedHermesValue &errorPrototype,
      napi_value code,
      napi_value message,
      napi_value *result) noexcept;

  // Exported function to create JS Error object.
  napi_status createJSError(
      napi_value code,
      napi_value message,
      napi_value *result) noexcept;

  // Exported function to create JS TypeError object.
  napi_status createJSTypeError(
      napi_value code,
      napi_value message,
      napi_value *result) noexcept;

  // Exported function to create JS RangeError object.
  napi_status createJSRangeError(
      napi_value code,
      napi_value message,
      napi_value *result) noexcept;

  // Exported function to check if the object is an instance of Error object.
  napi_status isJSError(napi_value value, bool *result) noexcept;

  // Exported function to throw provided error value.
  napi_status throwJSError(napi_value error) noexcept;

  // Internal function to create and throw JS error object with the specified
  // prototype.
  napi_status throwJSError(
      const vm::PinnedHermesValue &prototype,
      const char *code,
      const char *message) noexcept;

  // Exported function to create and throw JS Error object.
  napi_status throwJSError(const char *code, const char *message) noexcept;

  // Exported function to create and throw JS TypeError object.
  napi_status throwJSTypeError(const char *code, const char *message) noexcept;

  // Exported function to create and throw JS RangeError object.
  napi_status throwJSRangeError(const char *code, const char *message) noexcept;

  // Internal function to set code property for the error object.
  // Node.js has a predefined set of codes for common errors.
  napi_status setJSErrorCode(
      vm::Handle<vm::JSError> error,
      napi_value code,
      const char *codeCString) noexcept;

  //-----------------------------------------------------------------------------
  // Methods to support catching JS exceptions
  //-----------------------------------------------------------------------------
 public:
  // Exported function to check if there is a pending thrown JS error.
  napi_status isJSErrorPending(bool *result) noexcept;

  // Internal function to check if there is a pending thrown JS error.
  // It returns napi_ok or napi_pending_exception.
  napi_status checkPendingJSError() noexcept;

  // Exported function to get and clear pending thrown JS error.
  napi_status getAndClearPendingJSError(napi_value *result) noexcept;

  // Internal function to check ExecutionStatus and get the thrown JS error.
  napi_status checkJSErrorStatus(
      vm::ExecutionStatus hermesStatus,
      napi_status status = napi_generic_failure) noexcept;

  // Internal function to check ExecutionStatus of callResult and get the thrown
  // JS error.
  template <class T>
  napi_status checkJSErrorStatus(
      const vm::CallResult<T> &callResult,
      napi_status status = napi_generic_failure) noexcept;

  //-----------------------------------------------------------------------------
  // Getters for common singletons
  //-----------------------------------------------------------------------------
 public:
  // Exported function to get the `global` object.
  napi_status getGlobal(napi_value *result) noexcept;

  // Exported function to get the `undefined` value.
  napi_status getUndefined(napi_value *result) noexcept;

  // Internal function to get the `undefined` value.
  const vm::PinnedHermesValue &getUndefined() noexcept;

  // Exported function to get the `null` value.
  napi_status getNull(napi_value *result) noexcept;

  //-----------------------------------------------------------------------------
  // Method to get value type
  //-----------------------------------------------------------------------------
 public:
  // Exported function to get JS type of object.
  napi_status typeOf(napi_value value, napi_valuetype *result) noexcept;

  //-----------------------------------------------------------------------------
  // Methods to work with Booleans
  //-----------------------------------------------------------------------------
 public:
  // Exported function to get napi_value for `true` or `false`.
  napi_status getBoolean(bool value, napi_value *result) noexcept;

  // Exported function to get value of a Boolean value.
  napi_status getBooleanValue(napi_value value, bool *result) noexcept;

  //-----------------------------------------------------------------------------
  // Methods to work with Numbers
  //-----------------------------------------------------------------------------
 public:
  // Exported function to create napi_value for a number.
  template <class T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
  napi_status createNumber(T value, napi_value *result) noexcept;

  // Exported function to get `double` value from a napi_value number.
  napi_status getNumberValue(napi_value value, double *result) noexcept;

  // Exported function to get `int32_t` value from a napi_value number.
  napi_status getNumberValue(napi_value value, int32_t *result) noexcept;

  // Exported function to get `uint32_t` value from a napi_value number.
  napi_status getNumberValue(napi_value value, uint32_t *result) noexcept;

  // Exported function to get `int64_t` value from a napi_value number.
  napi_status getNumberValue(napi_value value, int64_t *result) noexcept;

  //-----------------------------------------------------------------------------
  // Methods to work with Strings
  //-----------------------------------------------------------------------------
 public:
  // Internal function to create napi_value from an ASCII string.
  napi_status createStringASCII(
      const char *str,
      size_t length,
      napi_value *result) noexcept;

  // Exported function to create napi_value from an Latin1 string.
  // The Latin1 is the one-byte encoding that can be converted to UTF-16 by an
  // unsigned expansion of each characted to 16 bit.
  napi_status createStringLatin1(
      const char *str,
      size_t length,
      napi_value *result) noexcept;

  // Exported function to create napi_value from an UTF-8 string.
  napi_status
  createStringUTF8(const char *str, size_t length, napi_value *result) noexcept;

  // Internal function to create napi_value from an UTF-8 string.
  // The str must be zero-terminated.
  napi_status createStringUTF8(const char *str, napi_value *result) noexcept;

  // Exported function to create napi_value from an UTF-16 string.
  napi_status createStringUTF16(
      const char16_t *str,
      size_t length,
      napi_value *result) noexcept;

  // Exported function to get Latin1 string value from a napi_value string.
  napi_status getStringValueLatin1(
      napi_value value,
      char *buf,
      size_t bufSize,
      size_t *result) noexcept;

  // Exported function to get UTF-8 string value from a napi_value string.
  napi_status getStringValueUTF8(
      napi_value value,
      char *buf,
      size_t bufSize,
      size_t *result) noexcept;

  // Exported function to get UTF-16 string value from a napi_value string.
  napi_status getStringValueUTF16(
      napi_value value,
      char16_t *buf,
      size_t bufSize,
      size_t *result) noexcept;

  // Internal function to convert UTF-8 stirng to UTF-16.
  napi_status convertUTF8ToUTF16(
      const char *utf8,
      size_t length,
      std::u16string &out) noexcept;

  // Internal function to get or create unique UTF-8 string SymbolID.
  // Note that unique SymbolID is used by Hermes for string-based identifiers,
  // and non-unique SymbolID is for the JS Symbols.
  napi_status getUniqueSymbolID(
      const char *utf8,
      size_t length,
      vm::MutableHandle<vm::SymbolID> *result) noexcept;

  // Internal function to create unique UTF-8 string SymbolID.
  napi_status getUniqueSymbolID(
      napi_value strValue,
      vm::MutableHandle<vm::SymbolID> *result) noexcept;

  //-----------------------------------------------------------------------------
  // Methods to work with Symbols
  //-----------------------------------------------------------------------------
 public:
  // Exported function to create a JS symbol object.
  napi_status createSymbol(napi_value description, napi_value *result) noexcept;

  //-----------------------------------------------------------------------------
  // Methods to work with BigInt
  //-----------------------------------------------------------------------------
 public:
  napi_status createBigIntFromInt64(int64_t value, napi_value *result);

  napi_status createBigIntFromUint64(uint64_t value, napi_value *result);

  napi_status createBigIntFromWords(
      int signBit,
      size_t wordCount,
      const uint64_t *words,
      napi_value *result);

  napi_status
  getBigIntValueInt64(napi_value value, int64_t *result, bool *lossless);

  napi_status
  getBigIntValueUint64(napi_value value, uint64_t *result, bool *lossless);

  napi_status getBigIntValueWords(
      napi_value value,
      int *signBit,
      size_t *wordCount,
      uint64_t *words);

  //-----------------------------------------------------------------------------
  // Methods to coerce values using JS coercion rules
  //-----------------------------------------------------------------------------
 public:
  // Exported function to coerce napi_value to a Boolean primitive value.
  napi_status coerceToBoolean(napi_value value, napi_value *result) noexcept;

  // Exported function to coerce napi_value to a Number primitive value.
  napi_status coerceToNumber(napi_value value, napi_value *result) noexcept;

  // Exported function to coerce napi_value to a String primitive value.
  napi_status coerceToString(napi_value value, napi_value *result) noexcept;

  // Exported function to coerce napi_value to an Object.
  napi_status coerceToObject(napi_value value, napi_value *result) noexcept;

  //-----------------------------------------------------------------------------
  // Methods to work with Objects
  //-----------------------------------------------------------------------------
 public:
  // Exported function to create a new Object instance.
  napi_status createObject(napi_value *result) noexcept;

  // Exported function to get object's prototype.
  napi_status getPrototype(napi_value object, napi_value *result) noexcept;

  // Exported function to get all enumerable string property names as in the
  // for..in statement. All indexes are converted to strings.
  napi_status getForInPropertyNames(
      napi_value object,
      napi_value *result) noexcept;

  // Internal function to get all enumerable string property names as in the
  // for..in statement.
  // The keyConversion specifies if index properties must be converted to
  // strings. The function wraps up the Hermes optimized function that caches
  // results.
  napi_status getForInPropertyNames(
      napi_value object,
      napi_key_conversion keyConversion,
      napi_value *result) noexcept;

  // Exported function to get all property names depending on the criteria.
  // The keyMode specifies whether to return only own properties or traverse the
  // prototype hierarchy. The keyFilter specifies whether to return enumerable,
  // writable, configurable, or all properties. The keyConversion specifies
  // whether to convert indexes to strings.
  napi_status getAllPropertyNames(
      napi_value object,
      napi_key_collection_mode keyMode,
      napi_key_filter keyFilter,
      napi_key_conversion keyConversion,
      napi_value *result) noexcept;

  // Internal function to convert temporary key storage represented by a
  // array-builder-like BigStorage to a JS Array.
  napi_status convertKeyStorageToArray(
      vm::Handle<vm::BigStorage> keyStorage,
      uint32_t startIndex,
      uint32_t length,
      napi_key_conversion keyConversion,
      napi_value *result) noexcept;

  // Internal function to convert all array elements to strings.
  // We use it to convert property keys represented as uint32 indexes.
  napi_status convertToStringKeys(vm::Handle<vm::JSArray> array) noexcept;

  // Internal function to convert index value to a string.
  napi_status convertIndexToString(
      double value,
      vm::MutableHandle<> *result) noexcept;

  // Exported function to check if object has the property.
  napi_status
  hasProperty(napi_value object, napi_value key, bool *result) noexcept;

  // Exported function to get property value.
  napi_status
  getProperty(napi_value object, napi_value key, napi_value *result) noexcept;

  // Exported function to set property value.
  napi_status
  setProperty(napi_value object, napi_value key, napi_value value) noexcept;

  // Exported function to delete property value.
  napi_status
  deleteProperty(napi_value object, napi_value key, bool *result) noexcept;

  // Exported function to check if object has the own property.
  napi_status
  hasOwnProperty(napi_value object, napi_value key, bool *result) noexcept;

  // Exported function to check if object has a property with property name as a
  // string.
  napi_status hasNamedProperty(
      napi_value object,
      const char *utf8Name,
      bool *result) noexcept;

  // Exported function to get property value with property name as a string.
  napi_status getNamedProperty(
      napi_value object,
      const char *utf8Name,
      napi_value *result) noexcept;

  // Exported function to set property value with property name as a string.
  napi_status setNamedProperty(
      napi_value object,
      const char *utf8Name,
      napi_value value) noexcept;

  // Exported function to define a set of properties.
  napi_status defineProperties(
      napi_value object,
      size_t propertyCount,
      const napi_property_descriptor *properties) noexcept;

  // Internal function to get SymbolID representing property identifier from the
  // property descriptor.
  napi_status symbolIDFromPropertyDescriptor(
      const napi_property_descriptor *descriptor,
      vm::MutableHandle<vm::SymbolID> *result) noexcept;

  // Exported function to freeze the object.
  // The frozen object is an immutable object.
  napi_status objectFreeze(napi_value object) noexcept;

  // Exported function to seal the object.
  // The sealed object cannot change number of its properties, but any writable
  // property value can be changed.
  napi_status objectSeal(napi_value object) noexcept;

  //-----------------------------------------------------------------------------
  // Methods to work with Arrays
  //-----------------------------------------------------------------------------
 public:
  // Exported function to create Array object instance.
  napi_status createArray(size_t length, napi_value *result) noexcept;

  // Exported function to check if value is an Array object instance.
  napi_status isArray(napi_value value, bool *result) noexcept;

  // Exported function to get Array length.
  napi_status getArrayLength(napi_value value, uint32_t *result) noexcept;

  // Exported function to check if Array or Object has an element at specified
  // index.
  napi_status
  hasElement(napi_value object, uint32_t index, bool *result) noexcept;

  // Exported function to get Array or Object element by index.
  napi_status
  getElement(napi_value object, uint32_t index, napi_value *result) noexcept;

  // Exported function to set Array or Object element by index.
  napi_status
  setElement(napi_value object, uint32_t index, napi_value value) noexcept;

  // Exported function to delete Array or Object element by index.
  napi_status
  deleteElement(napi_value object, uint32_t index, bool *result) noexcept;

  //-----------------------------------------------------------------------------
  // Methods to work with Functions
  //-----------------------------------------------------------------------------
 public:
  // Exported function to create JS Function for a native callback.
  napi_status createFunction(
      const char *utf8Name,
      size_t length,
      napi_callback callback,
      void *callbackData,
      napi_value *result) noexcept;

  // Internal function to create JS Function for a native callback.
  napi_status createFunction(
      vm::SymbolID name,
      napi_callback callback,
      void *callbackData,
      vm::MutableHandle<vm::Callable> *result) noexcept;

  // Exported function to call JS Function.
  napi_status callFunction(
      napi_value thisArg,
      napi_value func,
      size_t argCount,
      const napi_value *args,
      napi_value *result) noexcept;

  // Exported function to create a new object instance by calling the JS
  // Function as a constructor.
  napi_status createNewInstance(
      napi_value constructor,
      size_t argCount,
      const napi_value *args,
      napi_value *result) noexcept;

  // Exported function to check if the object was created by the specified
  // constructor.
  napi_status isInstanceOf(
      napi_value object,
      napi_value constructor,
      bool *result) noexcept;

  // Internal function to call into a module.
  template <class TLambda>
  vm::ExecutionStatus callIntoModule(TLambda &&call) noexcept;

  //-----------------------------------------------------------------------------
  // Methods to work with napi_callbacks
  //-----------------------------------------------------------------------------
 public:
  // Exported function to get callback info from inside of native callback.
  napi_status getCallbackInfo(
      napi_callback_info callbackInfo,
      size_t *argCount,
      napi_value *args,
      napi_value *thisArg,
      void **data) noexcept;

  // Exported function to get the new.target from inside of native callback.
  napi_status getNewTarget(
      napi_callback_info callbackInfo,
      napi_value *result) noexcept;

  //---------------------------------------------------------------------------
  // Property access helpers
  //---------------------------------------------------------------------------
 public:
  // Internal function to get predefined value by key.
  const vm::PinnedHermesValue &getPredefinedValue(
      NodeApiPredefined key) noexcept;

  // Internal function to get predefined value as a SymbolID.
  vm::SymbolID getPredefinedSymbol(NodeApiPredefined key) noexcept;

  // Internal function to check if object has property by a predefined key.
  template <class TObject>
  napi_status hasPredefinedProperty(
      TObject object,
      NodeApiPredefined key,
      bool *result) noexcept;

  // Internal function to get property value by a predefined key.
  template <class TObject>
  napi_status getPredefinedProperty(
      TObject object,
      NodeApiPredefined key,
      napi_value *result) noexcept;

  // Internal function to set property value by predefined key.
  template <class TObject, class TValue>
  napi_status setPredefinedProperty(
      TObject object,
      NodeApiPredefined key,
      TValue &&value,
      bool *optResult = nullptr) noexcept;

  // Internal function to check if object has a property with provided name.
  template <class TObject>
  napi_status
  hasNamedProperty(TObject object, vm::SymbolID key, bool *result) noexcept;

  // Internal function to get property by name.
  template <class TObject>
  napi_status getNamedProperty(
      TObject object,
      vm::SymbolID key,
      napi_value *result) noexcept;

  // Internal function to set property by name.
  template <class TObject, class TValue>
  napi_status setNamedProperty(
      TObject object,
      vm::SymbolID key,
      TValue &&value,
      bool *optResult = nullptr) noexcept;

  // Internal function to check if property exists by key of any type.
  template <class TObject, class TKey>
  napi_status
  hasComputedProperty(TObject object, TKey key, bool *result) noexcept;

  // Internal function to get property value by key of any type.
  template <class TObject, class TKey>
  napi_status
  getComputedProperty(TObject object, TKey key, napi_value *result) noexcept;

  // Internal function to set property value by key of any type.
  template <class TObject, class TKey, class TValue>
  napi_status setComputedProperty(
      TObject object,
      TKey key,
      TValue value,
      bool *optResult = nullptr) noexcept;

  // Internal function to delete property by key of any type.
  template <class TObject, class TKey>
  napi_status deleteComputedProperty(
      TObject object,
      TKey key,
      bool *optResult = nullptr) noexcept;

  // Internal function to get own descriptor by key of any type.
  template <class TObject, class TKey>
  napi_status getOwnComputedPropertyDescriptor(
      TObject object,
      TKey key,
      vm::MutableHandle<vm::SymbolID> &tmpSymbolStorage,
      vm::ComputedPropertyDescriptor &desc,
      bool *result) noexcept;

  // Internal function to define a property.
  template <class TObject>
  napi_status defineOwnProperty(
      TObject object,
      vm::SymbolID name,
      vm::DefinePropertyFlags dpFlags,
      vm::Handle<> valueOrAccessor,
      bool *result) noexcept;

  //-----------------------------------------------------------------------------
  // Methods to compare values
  //-----------------------------------------------------------------------------
 public:
  // Exported function to check if two values are equal without coercing them to
  // the same type. It is equivalent to JS `===` operator.
  napi_status
  strictEquals(napi_value lhs, napi_value rhs, bool *result) noexcept;

  //-----------------------------------------------------------------------------
  // Methods to work with external data objects
  //-----------------------------------------------------------------------------
 public:
  // Exported function to define a JS class with instance and static members.
  napi_status defineClass(
      const char *utf8Name,
      size_t length,
      napi_callback constructor,
      void *callbackData,
      size_t propertyCount,
      const napi_property_descriptor *properties,
      napi_value *result) noexcept;

  // Exported function to wrap up native object instance into a JS object.
  napi_status wrapObject(
      napi_value object,
      void *nativeData,
      napi_finalize finalizeCallback,
      void *finalizeHint,
      napi_ref *result) noexcept;

  // Exported function to associated a finalizer along with with a JS object.
  napi_status addFinalizer(
      napi_value object,
      void *nativeData,
      napi_finalize finalizeCallback,
      void *finalizeHint,
      napi_ref *result) noexcept;

  // Exported function to get and/or remove native object from JS object.
  template <NodeApiUnwrapAction action>
  napi_status unwrapObject(napi_value object, void **result) noexcept;

  // Exported function to associate a 16-byte ID such as UUID with an object.
  napi_status typeTagObject(
      napi_value object,
      const napi_type_tag *typeTag) noexcept;

  // Exported function to check if 16-byte ID such as UUID is associated with an
  // object.
  napi_status checkObjectTypeTag(
      napi_value object,
      const napi_type_tag *typeTag,
      bool *result) noexcept;

  // Exported function to create external value object.
  napi_status createExternal(
      void *nativeData,
      napi_finalize finalizeCallback,
      void *finalizeHint,
      napi_value *result) noexcept;

  // Internal function to create external value object.
  vm::Handle<vm::DecoratedObject> createExternalObject(
      void *nativeData,
      NodeApiExternalValue **externalValue) noexcept;

  // Exported function to get native data associated with the external value
  // type.
  napi_status getValueExternal(napi_value value, void **result) noexcept;

  // Internal function to get NodeApiExternalValue associated with the external
  // value type.
  NodeApiExternalValue *getExternalObjectValue(vm::HermesValue value) noexcept;

  // Internal function to get or create NodeApiExternalValue associated with the
  // external value type.
  template <class TObject>
  napi_status getExternalPropertyValue(
      TObject object,
      NodeApiIfNotFound ifNotFound,
      NodeApiExternalValue **result) noexcept;

  // Internal function to associate a finalizer with an object.
  napi_status addObjectFinalizer(
      const vm::PinnedHermesValue *value,
      NodeApiFinalizer *finalizer) noexcept;

  // Internal function to call finalizer callback.
  void callFinalizer(
      napi_finalize finalizeCallback,
      void *nativeData,
      void *finalizeHint) noexcept;

  // Internal function to add finalizer to the finalizer queue.
  void addToFinalizerQueue(NodeApiFinalizer *finalizer) noexcept;

  // Internal function to call all finalizers in the finalizer queue.
  napi_status processFinalizerQueue() noexcept;

  //-----------------------------------------------------------------------------
  // Methods to work with references.
  //-----------------------------------------------------------------------------
 public:
  // Exported function to create a Complex reference.
  napi_status createReference(
      napi_value value,
      uint32_t initialRefCount,
      napi_ref *result) noexcept;

  // Exported function to delete a Complex reference.
  napi_status deleteReference(napi_ref ref) noexcept;

  // Exported function to increment Complex reference ref count.
  // If the ref count was zero, then the weak ref is converted to string ref.
  napi_status incReference(napi_ref ref, uint32_t *result) noexcept;

  // Exported function to decrement Complex reference ref count.
  // If the ref count becomes zero, then the strong reference is converted to
  // weak ref.
  napi_status decReference(napi_ref ref, uint32_t *result) noexcept;

  // Exported function to get JS value from a complex reference.
  napi_status getReferenceValue(napi_ref ref, napi_value *result) noexcept;

  // Internal function to add non-finalizing reference.
  void addReference(NodeApiReference *reference) noexcept;

  // Internal function to add finalizing reference.
  void addFinalizingReference(NodeApiReference *reference) noexcept;

  //-----------------------------------------------------------------------------
  // Methods to control napi_value stack.
  // napi_value are added on top of the stack.
  // Closing napi_value stack scope deletes all napi_values added after
  // opening the scope.
  //-----------------------------------------------------------------------------
 public:
  // Exported function to open napi_value stack scope.
  napi_status openNodeApiValueScope(napi_handle_scope *result) noexcept;

  // Exported function to close napi_value stack scope.
  napi_status closeNodeApiValueScope(napi_handle_scope scope) noexcept;

  // Exported function to open napi_value stack scope that allows one value to
  // escape to the parent scope.
  napi_status openEscapableNodeApiValueScope(
      napi_escapable_handle_scope *result) noexcept;

  // Exported function to close escapable napi_value stack scope.
  napi_status closeEscapableNodeApiValueScope(
      napi_escapable_handle_scope scope) noexcept;

  // Exported function to escape a value from current scope to the parent scope.
  napi_status escapeNodeApiValue(
      napi_escapable_handle_scope scope,
      napi_value escapee,
      napi_value *result) noexcept;

  // Internal function to push new napi_value to the napi_value stack and then
  // return it.
  napi_value pushNewNodeApiValue(vm::HermesValue value) noexcept;

  //-----------------------------------------------------------------------------
  // Methods to work with weak roots.
  //-----------------------------------------------------------------------------
 public:
  // Internal function to create weak root.
  vm::WeakRoot<vm::JSObject> createWeakRoot(vm::JSObject *object) noexcept;

  // Internal function to lock a weak root.
  const vm::PinnedHermesValue &lockWeakRoot(
      vm::WeakRoot<vm::JSObject> &weakRoot) noexcept;

  //-----------------------------------------------------------------------------
  // Methods to work with ordered sets.
  // We use them as a temporary storage while retrieving property names.
  // They are treated as GC roots.
  //-----------------------------------------------------------------------------
 public:
  // Internal function to add ordered set to be tracked by GC.
  void pushOrderedSet(NodeApiOrderedSet<vm::HermesValue> &set) noexcept;

  // Internal function to remove ordered set from being tracked by GC.
  void popOrderedSet() noexcept;

  //-----------------------------------------------------------------------------
  // Methods to work with array buffers and typed arrays
  //-----------------------------------------------------------------------------
 public:
  // Exported function to create JS ArrayBuffer object.
  napi_status createArrayBuffer(
      size_t byteLength,
      void **data,
      napi_value *result) noexcept;

  // Exported function to create JS ArrayBuffer object against external data.
  napi_status createExternalArrayBuffer(
      void *externalData,
      size_t byteLength,
      napi_finalize finalizeCallback,
      void *finalizeHint,
      napi_value *result) noexcept;

  // Exported function to check if the value is an ArrayBuffer instance.
  napi_status isArrayBuffer(napi_value value, bool *result) noexcept;

  // Exported function to get ArrayBuffer info.
  napi_status getArrayBufferInfo(
      napi_value arrayBuffer,
      void **data,
      size_t *byteLength) noexcept;

  // Exported function to detach the native buffer associated with the
  // ArrayBuffer instance.
  napi_status detachArrayBuffer(napi_value arrayBuffer) noexcept;

  // Exported function to check if ArrayBuffer instance has a detached native
  // buffer.
  napi_status isDetachedArrayBuffer(
      napi_value arrayBuffer,
      bool *result) noexcept;

  // Exported function to create JS TypedArray object instance for the
  // arrayBuffer. The TypedArray is an array-like view of an underlying binary
  // data buffer.
  napi_status createTypedArray(
      napi_typedarray_type type,
      size_t length,
      napi_value arrayBuffer,
      size_t byteOffset,
      napi_value *result) noexcept;

  // Internal function to create TypedArray instance.
  template <class TItem, vm::CellKind CellKind>
  napi_status createTypedArray(
      size_t length,
      vm::JSArrayBuffer *buffer,
      size_t byteOffset,
      vm::MutableHandle<vm::JSTypedArrayBase> *result) noexcept;

  // Internal function to get TypedArray name.
  template <vm::CellKind CellKind>
  static constexpr const char *getTypedArrayName() noexcept;

  // Exported function to check if the value is a TypedArray instance.
  napi_status isTypedArray(napi_value value, bool *result) noexcept;

  // Exported function to get TypeArray info.
  napi_status getTypedArrayInfo(
      napi_value typedArray,
      napi_typedarray_type *type,
      size_t *length,
      void **data,
      napi_value *arrayBuffer,
      size_t *byteOffset) noexcept;

  // Exported function to create JS DataView object instance for the
  // arrayBuffer.
  napi_status createDataView(
      size_t byteLength,
      napi_value arrayBuffer,
      size_t byteOffset,
      napi_value *result) noexcept;

  // Exported function to check if the value is a DataView.
  napi_status isDataView(napi_value value, bool *result) noexcept;

  // Exported function to get DataView instance info.
  napi_status getDataViewInfo(
      napi_value dataView,
      size_t *byteLength,
      void **data,
      napi_value *arrayBuffer,
      size_t *byteOffset) noexcept;

  //-----------------------------------------------------------------------------
  // Version management
  //-----------------------------------------------------------------------------
 public:
  // Exported function to get the version of the implemented Node-API.
  napi_status getVersion(uint32_t *result) noexcept;

  //-----------------------------------------------------------------------------
  // Methods to work with Promises
  //-----------------------------------------------------------------------------
 public:
  // Exported function to create Promise object instance.
  napi_status createPromise(
      napi_deferred *deferred,
      napi_value *result) noexcept;

  // Internal function to create Promise object instance.
  napi_status createPromise(
      napi_value *promise,
      vm::MutableHandle<> *resolveFunction,
      vm::MutableHandle<> *rejectFunction) noexcept;

  // Exported function to resolve Promise.
  napi_status resolveDeferred(
      napi_deferred deferred,
      napi_value resolution) noexcept;

  // Exported function to reject Promise.
  napi_status rejectDeferred(
      napi_deferred deferred,
      napi_value resolution) noexcept;

  // Internal function to resolve or reject Promise.
  napi_status concludeDeferred(
      napi_deferred deferred,
      NodeApiPredefined predefinedProperty,
      napi_value resolution) noexcept;

  // Exported function to check if value is a Promise.
  napi_status isPromise(napi_value value, bool *result) noexcept;

  // Internal function to enable Promise rejection tracker.
  napi_status enablePromiseRejectionTracker() noexcept;

  // Internal callback to handle Promise rejection notifications.
  static vm::CallResult<vm::HermesValue> handleRejectionNotification(
      void *context,
      vm::Runtime &runtime,
      vm::NativeArgs args,
      void (*handler)(
          NodeApiEnvironment *env,
          int32_t id,
          vm::HermesValue error)) noexcept;

  // Exported function to check if there is an unhandled Promise rejection.
  napi_status hasUnhandledPromiseRejection(bool *result) noexcept;

  // Exported function to get an clear last unhandled Promise rejection.
  napi_status getAndClearLastUnhandledPromiseRejection(
      napi_value *result) noexcept;

  napi_status drainMicrotasks(int32_t maxCountHint, bool *result) noexcept;

  //-----------------------------------------------------------------------------
  // Memory management
  //-----------------------------------------------------------------------------
 public:
  // Exported function to adjust external memory size. It is not implemented.
  // While it is not clear how to implement it for Hermes.
  napi_status adjustExternalMemory(
      int64_t change_in_bytes,
      int64_t *adjusted_value) noexcept;

  // Exported function to run garbage collection. It must be used only in unit
  // tests.
  napi_status collectGarbage() noexcept;

  //-----------------------------------------------------------------------------
  // Methods to work with Dates
  //-----------------------------------------------------------------------------
 public:
  // Exported function to create JS Date object.
  napi_status createDate(double dateTime, napi_value *result) noexcept;

  // Exported function to check if the value is a Date instance.
  napi_status isDate(napi_value value, bool *result) noexcept;

  // Exported function to get the internal value of the Date object.
  // It is equivalent to the JS Date.prototype.valueOf().
  // the number of milliseconds since midnight 01 January, 1970 UTC.
  napi_status getDateValue(napi_value value, double *result) noexcept;

  //-----------------------------------------------------------------------------
  // Instance data
  //-----------------------------------------------------------------------------
 public:
  // Exported function to associate external data with the environment.
  // Finalizer is not called for the previously associated data.
  napi_status setInstanceData(
      void *nativeData,
      napi_finalize finalizeCallback,
      void *finalizeHint) noexcept;

  // Exported function to get external data associated with the environment.
  napi_status getInstanceData(void **nativeData) noexcept;

  //---------------------------------------------------------------------------
  // Script running
  //---------------------------------------------------------------------------

  // Exported function to run script from a string value.
  // The sourceURL is used only for error reporting.
  napi_status runScript(napi_value source, napi_value *result) noexcept;

  // Internal function to check if buffer contains Hermes VM bytecode.
  static bool isHermesBytecode(const uint8_t *data, size_t length) noexcept;

  //---------------------------------------------------------------------------
  // Methods to create Hermes GC handles for stack-based variables.
  //
  // vm::Handle is a GC root kept on the stack.
  // The vm::Handle<> is a shortcut for vm::Handle<vm::HermesValue>.
  //---------------------------------------------------------------------------
 public:
  vm::Handle<> makeHandle(napi_value value) noexcept;
  vm::Handle<> makeHandle(const vm::PinnedHermesValue *value) noexcept;
  vm::Handle<> makeHandle(vm::HermesValue value) noexcept;
  vm::Handle<> makeHandle(vm::Handle<> value) noexcept;
  vm::Handle<> makeHandle(uint32_t value) noexcept;
  template <class T>
  vm::Handle<T> makeHandle(napi_value value) noexcept;
  template <class T>
  vm::Handle<T> makeHandle(const vm::PinnedHermesValue *value) noexcept;
  template <class T>
  vm::Handle<T> makeHandle(vm::HermesValue value) noexcept;
  template <class T, class TOther>
  vm::Handle<T> makeHandle(vm::Handle<TOther> value) noexcept;
  template <class T>
  vm::Handle<T> makeHandle(vm::PseudoHandle<T> &&value) noexcept;
  template <class T>
  vm::CallResult<vm::Handle<T>> makeHandle(
      vm::CallResult<vm::PseudoHandle<T>> &&callResult) noexcept;
  template <class T>
  vm::CallResult<vm::MutableHandle<T>> makeMutableHandle(
      vm::CallResult<vm::PseudoHandle<T>> &&callResult) noexcept;

  //---------------------------------------------------------------------------
  // Result setting helpers
  //
  // These functions help to reduce code responsible for returning results.
  //---------------------------------------------------------------------------
 public:
  template <class T, class TResult>
  napi_status setResult(T &&value, TResult *result) noexcept;

  template <class T, class TResult>
  napi_status setOptionalResult(T &&value, TResult *result) noexcept;

  template <class T>
  napi_status setOptionalResult(T &&value, std::nullptr_t) noexcept;

  napi_status setPredefinedResult(
      const vm::PinnedHermesValue *value,
      napi_value *result) noexcept;

  template <class T>
  napi_status setResultUnsafe(T &&value, T *result) noexcept;

  napi_status setResultUnsafe(
      vm::HermesValue value,
      napi_value *result) noexcept;

  napi_status setResultUnsafe(vm::SymbolID value, napi_value *result) noexcept;

  napi_status setResultUnsafe(bool value, napi_value *result) noexcept;

  template <class T>
  napi_status setResultUnsafe(
      vm::Handle<T> &&handle,
      napi_value *result) noexcept;

  template <class T>
  napi_status setResultUnsafe(
      vm::PseudoHandle<T> &&handle,
      napi_value *result) noexcept;

  template <class T>
  napi_status setResultUnsafe(
      vm::Handle<T> &&handle,
      vm::MutableHandle<T> *result) noexcept;

  napi_status setResultUnsafe(
      vm::HermesValue value,
      vm::MutableHandle<> *result) noexcept;

  template <class T, class TResult>
  napi_status setResultUnsafe(
      vm::CallResult<T> &&value,
      TResult *result) noexcept;

  template <class T, class TResult>
  napi_status setResultUnsafe(
      vm::CallResult<T> &&,
      napi_status onException,
      TResult *result) noexcept;

  template <class T>
  napi_status checkCallResult(const vm::CallResult<T> &value) noexcept;

  template <class T>
  napi_status checkCallResult(const T & /*value*/) noexcept;

 private:
  // Controls the lifetime of this class instances.
  std::atomic<int> refCount_{1};

  // Used for safe update of finalizer queue.
  NodeApiRefCountedPtr<NodeApiPendingFinalizers> pendingFinalizers_;

  // Reference to the wrapped Hermes runtime.
  vm::Runtime &runtime_;

  // TODO: Use default version 8 if not specified.
  int32_t apiVersion_{0};

  // Reference to itself for convenient use in macros.
  NodeApiEnvironment &env{*this};

  // Flags used by byte code compiler.
  hbc::CompileFlags compileFlags_{};

  // Collection of all predefined values.
  std::array<
      vm::PinnedHermesValue,
      static_cast<size_t>(NodeApiPredefined::PredefinedCount)>
      predefinedValues_{};

  // Stack of napi_value.
  NodeApiStableAddressStack<vm::PinnedHermesValue> napiValueStack_;

  // Stack of napi_value scopes.
  NodeApiStableAddressStack<size_t> napiValueStackScopes_;

  // We store references in two different lists, depending on whether they
  // have `napi_finalizer` callbacks, because we must first finalize the
  // ones that have such a callback. See `~NodeApiEnvironment()` for details.
  NodeApiLinkedList<NodeApiReference> references_{};
  NodeApiLinkedList<NodeApiReference> finalizingReferences_{};

  // Finalizers must be run outside of GC pass because they could access GC
  // objects. Then GC finalizes and object, we put all the associated finalizers
  // to this queue and then run them as soon as have an opportunity to do that
  // safely.
  NodeApiLinkedList<NodeApiFinalizer> finalizerQueue_{};

  // To ensure that the finalizerQueue_ is being processed only from a single
  // place at a time.
  bool isRunningFinalizers_{false};

  // Helps to change the behaviour of finalizers when the environment is
  // shutting down.
  bool isShuttingDown_{false};

  // Temporary GC roots for ordered sets used to collect property names.
  llvh::SmallVector<NodeApiOrderedSet<vm::HermesValue> *, 16> orderedSets_;

  // List of unique string references.
  std::unordered_map<vm::SymbolID::RawType, NodeApiStrongReference *>
      uniqueStrings_;

  // Storage for the last native error message.
  std::string lastErrorMessage_;

  // The last native error.
  NodeApiNativeError lastError_{"", 0, 0, napi_ok};

  // The last JS error.
  vm::PinnedHermesValue thrownJSError_{EmptyHermesValue};

  // ID of last recorded unhandled Promise rejection.
  int32_t lastUnhandledRejectionId_{-1};

  // The last unhandled Promise rejection.
  vm::PinnedHermesValue lastUnhandledRejection_{EmptyHermesValue};

  // External data associated with the environment instance.
  NodeApiInstanceData *instanceData_{};

  // HermesValue used for uninitialized values.
  static constexpr vm::HermesValue EmptyHermesValue{
      vm::HermesValue::encodeEmptyValue()};

  // The sentinel tag in napiValueStack_ used for escapable values.
  // These are the first four ASCII letters of name "Janus" - the god of gates.
  static constexpr uint32_t kEscapeableSentinelTag = 0x4a616e75;
  static constexpr uint32_t kUsedEscapeableSentinelTag =
      kEscapeableSentinelTag + 1;

  // Tag used to indicate external values for DecoratedObject.
  // These are the first four ASCII letters of word "External".
  static constexpr uint32_t kExternalValueTag = 0x45787465;
  static constexpr int32_t kExternalTagSlotIndex = 0;
};

// NodeApiPendingFinalizers is used to update the pending finalizer list in a
// thread safe way when a NodeApiExternalValue is destroyed from a GC background
// thread.
class NodeApiPendingFinalizers {
 public:
  // Create new instance of NodeApiPendingFinalizers.
  static NodeApiRefCountedPtr<NodeApiPendingFinalizers> create() noexcept {
    return NodeApiRefCountedPtr<NodeApiPendingFinalizers>(
        new NodeApiPendingFinalizers(), attachTag);
  }

  // Add pending finalizers from a NodeApiExternalValue destructor.
  // It can be called from JS or GC background threads.
  void addPendingFinalizers(std::unique_ptr<NodeApiLinkedList<NodeApiFinalizer>>
                                &&finalizers) noexcept {
    std::scoped_lock lock{mutex_};
    finalizers_.push_back(std::move(finalizers));
  }

  // Apply pending finalizers to the finalizer queue.
  // It must be called from a JS thread.
  void applyPendingFinalizers(NodeApiEnvironment *env) noexcept {
    std::vector<std::unique_ptr<NodeApiLinkedList<NodeApiFinalizer>>>
        finalizers;
    {
      std::scoped_lock lock{mutex_};
      if (finalizers_.empty()) {
        return;
      }
      // Move to a local variable to unlock the mutex earlier.
      finalizers = std::move(finalizers_);
    }

    for (auto &finalizerList : finalizers) {
      finalizerList->forEach([env](NodeApiFinalizer *finalizer) {
        env->addToFinalizerQueue(finalizer);
      });
    }
  }

 private:
  friend class NodeApiRefCountedPtr<NodeApiPendingFinalizers>;

  NodeApiPendingFinalizers() noexcept = default;

  void incRefCount() noexcept {
    int refCount = refCount_.fetch_add(1, std::memory_order_relaxed) + 1;
    CRASH_IF_FALSE(refCount > 1 && "The ref count cannot bounce from zero.");
    CRASH_IF_FALSE(
        refCount < std::numeric_limits<int>::max() &&
        "The ref count is too big.");
  }

  void decRefCount() noexcept {
    int refCount = refCount_.fetch_sub(1, std::memory_order_release) - 1;
    CRASH_IF_FALSE(refCount >= 0 && "The ref count must not be negative.");
    if (refCount == 0) {
      std::atomic_thread_fence(std::memory_order_acquire);
      delete this;
    }
  }

 private:
  std::atomic<int> refCount_{1};
  std::recursive_mutex mutex_;
  std::vector<std::unique_ptr<NodeApiLinkedList<NodeApiFinalizer>>> finalizers_;
};

// RAII class to control scope of napi_value variables and return values.
class NodeApiHandleScope final {
 public:
  NodeApiHandleScope(
      NodeApiEnvironment &env,
      napi_value *result = nullptr) noexcept
      : env_(env),
        result_(result),
        savedScope_(env.napiValueStack().size()),
        gcScope_(env.runtime()) {}

  ~NodeApiHandleScope() noexcept {
    env_.napiValueStack().resize(savedScope_);
  }

  napi_status setResult(napi_status status) noexcept {
    CHECK_NAPI(status);
    if (result_ != nullptr) {
      if (savedScope_ + 1 < env_.napiValueStack().size()) {
        env_.napiValueStack()[savedScope_] = *phv(*result_);
        *result_ = napiValue(&env_.napiValueStack()[savedScope_]);
      } else {
        CRASH_IF_FALSE(savedScope_ < env_.napiValueStack().size());
        CRASH_IF_FALSE(phv(*result_) == &env_.napiValueStack()[savedScope_]);
      }
      // To make sure that the return value is not removed in the destructor.
      ++savedScope_;
    }
    return env_.processFinalizerQueue();
  }

  template <class T>
  napi_status setResult(T &&value) noexcept {
    return setResult(env_.setResult(std::forward<T>(value), result_));
  }

  template <class T>
  napi_status setOptionalResult(T &&value) noexcept {
    return setResult(env_.setOptionalResult(std::forward<T>(value), result_));
  }

 private:
  NodeApiEnvironment &env_;
  napi_value *result_{};
  size_t savedScope_;
  vm::GCScope gcScope_;
};

// Keep external data with an object.
class NodeApiExternalValue final : public vm::DecoratedObject::Decoration {
 public:
  NodeApiExternalValue(const NodeApiRefCountedPtr<NodeApiPendingFinalizers>
                           &pendingFinalizers) noexcept
      : pendingFinalizers_(pendingFinalizers) {}
  NodeApiExternalValue(
      const NodeApiRefCountedPtr<NodeApiPendingFinalizers> &pendingFinalizers,
      void *nativeData) noexcept
      : pendingFinalizers_(pendingFinalizers), nativeData_(nativeData) {}

  NodeApiExternalValue(const NodeApiExternalValue &other) = delete;
  NodeApiExternalValue &operator=(const NodeApiExternalValue &other) = delete;

  // The destructor is called by GC. It can be called either from JS or GC
  // threads. We move the finalizers to NodeApiPendingFinalizers to be accessed
  // only from JS thread.
  ~NodeApiExternalValue() override {
    pendingFinalizers_->addPendingFinalizers(std::move(finalizers_));
  }

  size_t getMallocSize() const override {
    return sizeof(*this);
  }

  void addFinalizer(NodeApiFinalizer *finalizer) noexcept {
    finalizers_->pushBack(finalizer);
  }

  void *nativeData() noexcept {
    return nativeData_;
  }

  void setNativeData(void *value) noexcept {
    nativeData_ = value;
  }

 private:
  NodeApiRefCountedPtr<NodeApiPendingFinalizers> pendingFinalizers_;
  void *nativeData_{};
  std::unique_ptr<NodeApiLinkedList<NodeApiFinalizer>> finalizers_{
      std::make_unique<NodeApiLinkedList<NodeApiFinalizer>>()};
};

// Keep native data associated with a function.
class NodeApiHostFunctionContext final {
  friend class NodeApiCallbackInfo;

 public:
  NodeApiHostFunctionContext(
      NodeApiEnvironment &env,
      napi_callback hostCallback,
      void *nativeData) noexcept
      : env_{env}, hostCallback_{hostCallback}, nativeData_{nativeData} {}

  static vm::CallResult<vm::HermesValue>
  func(void *context, vm::Runtime &runtime, vm::NativeArgs hvArgs);

  static void finalize(void *context) {
    delete reinterpret_cast<class NodeApiHostFunctionContext *>(context);
  }

  static void finalizeNS(vm::GC & /*gc*/, vm::NativeState *ns) {
    delete reinterpret_cast<class NodeApiHostFunctionContext *>(ns->context());
  }

  void *nativeData() noexcept {
    return nativeData_;
  }

 private:
  NodeApiEnvironment &env_;
  napi_callback hostCallback_;
  void *nativeData_;
};

class NodeApiCallbackInfo final {
 public:
  NodeApiCallbackInfo(
      NodeApiHostFunctionContext &context,
      vm::NativeArgs &nativeArgs) noexcept
      : context_(context), nativeArgs_(nativeArgs) {}

  void args(napi_value *buffer, size_t bufferLength) noexcept {
    size_t min =
        std::min(bufferLength, static_cast<size_t>(nativeArgs_.getArgCount()));
    size_t i{0};
    for (; i < min; ++i) {
      buffer[i] = napiValue(&nativeArgs_.begin()[i]);
    }
    for (; i < bufferLength; ++i) {
      buffer[i] = napiValue(&context_.env_.getUndefined());
    }
  }

  size_t argCount() noexcept {
    return nativeArgs_.getArgCount();
  }

  napi_value thisArg() noexcept {
    return napiValue(&nativeArgs_.getThisArg());
  }

  void *nativeData() noexcept {
    return context_.nativeData();
  }

  napi_value getNewTarget() noexcept {
    const vm::PinnedHermesValue &newTarget = nativeArgs_.getNewTarget();
    return napiValue(newTarget.isUndefined() ? nullptr : &newTarget);
  }

 private:
  NodeApiHostFunctionContext &context_;
  vm::NativeArgs &nativeArgs_;
};

/*static*/ vm::CallResult<vm::HermesValue> NodeApiHostFunctionContext::func(
    void *context,
    vm::Runtime &runtime,
    vm::NativeArgs hvArgs) {
  NodeApiHostFunctionContext *hfc =
      reinterpret_cast<NodeApiHostFunctionContext *>(context);
  NodeApiEnvironment &env = hfc->env_;
  assert(&runtime == &env.runtime());

  NodeApiHandleScope scope{env};
  NodeApiCallbackInfo callbackInfo{*hfc, hvArgs};
  napi_value result{};
  vm::ExecutionStatus status = env.callIntoModule([&](NodeApiEnvironment *env) {
    result = hfc->hostCallback_(
        napiEnv(env), reinterpret_cast<napi_callback_info>(&callbackInfo));
  });

  if (status == vm::ExecutionStatus::EXCEPTION) {
    return vm::ExecutionStatus::EXCEPTION;
  }

  if (result) {
    return *phv(result);
  } else {
    return env.getUndefined();
  }
}

// Different types of references:
// 1. Strong reference - it can wrap up object of any type
//   a. Ref count maintains the reference lifetime. When it reaches zero it is
//   removed.
// 2. Weak reference - it can wrap up only objects
//   a. Ref count maintains the lifetime of the reference. When it reaches zero
//   it is removed.
// 3. Complex reference - it can wrap up only objects
//   a. Ref count only for strong references. Zero converts it to a weak ref.
//   Removal is explicit if external code holds a reference.

// A base class for References that wrap native data and must be finalized.
class NodeApiFinalizer : public NodeApiLinkedList<NodeApiFinalizer>::Item {
 public:
  virtual void finalize(NodeApiEnvironment &env) noexcept = 0;

 protected:
  NodeApiFinalizer() = default;

  ~NodeApiFinalizer() noexcept {
    unlink();
  }
};

// A base class for all references.
class NodeApiReference : public NodeApiLinkedList<NodeApiReference>::Item {
 public:
  enum class ReasonToDelete {
    ZeroRefCount,
    FinalizerCall,
    ExternalCall,
    EnvironmentShutdown,
  };

  static napi_status deleteReference(
      NodeApiEnvironment &env,
      NodeApiReference *reference,
      ReasonToDelete reason) noexcept {
    if (reference && reference->startDeleting(env, reason)) {
      delete reference;
    }
    return env.clearLastNativeError();
  }

  virtual napi_status incRefCount(
      NodeApiEnvironment &env,
      uint32_t & /*result*/) noexcept {
    return GENERIC_FAILURE("This reference does not support ref count.");
  }

  virtual napi_status decRefCount(
      NodeApiEnvironment &env,
      uint32_t & /*result*/) noexcept {
    return GENERIC_FAILURE("This reference does not support ref count.");
  }

  virtual const vm::PinnedHermesValue &value(NodeApiEnvironment &env) noexcept {
    return env.getUndefined();
  }

  virtual void *nativeData() noexcept {
    return nullptr;
  }

  virtual void *finalizeHint() noexcept {
    return nullptr;
  }

  virtual vm::PinnedHermesValue *getGCRoot(
      NodeApiEnvironment & /*env*/) noexcept {
    return nullptr;
  }

  virtual vm::WeakRoot<vm::JSObject> *getGCWeakRoot(
      NodeApiEnvironment & /*env*/) noexcept {
    return nullptr;
  }

  static void getGCRoots(
      NodeApiEnvironment &env,
      NodeApiLinkedList<NodeApiReference> &list,
      vm::RootAcceptor &acceptor) noexcept {
    list.forEach([&](NodeApiReference *ref) {
      if (vm::PinnedHermesValue *value = ref->getGCRoot(env)) {
        acceptor.accept(*value);
      }
    });
  }

  static void getGCWeakRoots(
      NodeApiEnvironment &env,
      NodeApiLinkedList<NodeApiReference> &list,
      vm::WeakRootAcceptor &acceptor) noexcept {
    list.forEach([&](NodeApiReference *ref) {
      if (vm::WeakRoot<vm::JSObject> *weakRoot = ref->getGCWeakRoot(env)) {
        acceptor.acceptWeak(*weakRoot);
      }
    });
  }

  virtual napi_status callFinalizeCallback(NodeApiEnvironment &env) noexcept {
    return napi_ok;
  }

  virtual void finalize(NodeApiEnvironment &env) noexcept {}

  template <class TItem>
  static void finalizeAll(
      NodeApiEnvironment &env,
      NodeApiLinkedList<TItem> &list) noexcept {
    for (TItem *item = list.begin(); item != list.end(); item = list.begin()) {
      item->finalize(env);
    }
  }

  static void deleteAll(
      NodeApiEnvironment &env,
      NodeApiLinkedList<NodeApiReference> &list,
      ReasonToDelete reason) noexcept {
    for (NodeApiReference *ref = list.begin(); ref != list.end();
         ref = list.begin()) {
      deleteReference(env, ref, reason);
    }
  }

 protected:
  // Make protected to avoid using operator delete directly.
  // Use the deleteReference method instead.
  virtual ~NodeApiReference() noexcept {
    unlink();
  }

  virtual bool startDeleting(
      NodeApiEnvironment &env,
      ReasonToDelete /*reason*/) noexcept {
    return true;
  }
};

// A reference with a ref count that can be changed from any thread.
// The reference deletion is done as a part of GC root detection to avoid
// deletion in a random thread.
class NodeApiAtomicRefCountReference : public NodeApiReference {
 public:
  napi_status incRefCount(NodeApiEnvironment &env, uint32_t &result) noexcept
      override {
    result = refCount_.fetch_add(1, std::memory_order_relaxed) + 1;
    CRASH_IF_FALSE(result > 1 && "The ref count cannot bounce from zero.");
    CRASH_IF_FALSE(result < MaxRefCount && "The ref count is too big.");
    return napi_ok;
  }

  napi_status decRefCount(NodeApiEnvironment &env, uint32_t &result) noexcept
      override {
    result = refCount_.fetch_sub(1, std::memory_order_release) - 1;
    if (result == 0) {
      std::atomic_thread_fence(std::memory_order_acquire);
    } else if (result > MaxRefCount) {
      // Decrement of an unsigned value below zero is getting to a very big
      // number.
      CRASH_IF_FALSE(
          result < MaxRefCount && "The ref count must not be negative.");
    }
    return napi_ok;
  }

 protected:
  uint32_t refCount() const noexcept {
    return refCount_;
  }

  bool startDeleting(NodeApiEnvironment &env, ReasonToDelete reason) noexcept
      override {
    return reason != ReasonToDelete::ExternalCall;
  }

 private:
  std::atomic<uint32_t> refCount_{1};

  static constexpr uint32_t MaxRefCount =
      std::numeric_limits<uint32_t>::max() / 2;
};

// Atomic ref counting for vm::PinnedHermesValue.
class NodeApiStrongReference : public NodeApiAtomicRefCountReference {
 public:
  static napi_status create(
      NodeApiEnvironment &env,
      vm::HermesValue value,
      NodeApiStrongReference **result) noexcept {
    CHECK_ARG(result);
    *result = new NodeApiStrongReference(value);
    env.addReference(*result);
    return env.clearLastNativeError();
  }

  const vm::PinnedHermesValue &value(
      NodeApiEnvironment &env) noexcept override {
    return value_;
  }

  vm::PinnedHermesValue *getGCRoot(NodeApiEnvironment &env) noexcept override {
    if (refCount() > 0) {
      return &value_;
    } else {
      deleteReference(env, this, ReasonToDelete::ZeroRefCount);
      return nullptr;
    }
  }

 protected:
  NodeApiStrongReference(vm::HermesValue value) noexcept : value_(value) {}

 private:
  vm::PinnedHermesValue value_;
};

// Atomic ref counting for a vm::WeakRef<vm::HermesValue>.
class NodeApiWeakReference final : public NodeApiAtomicRefCountReference {
 public:
  static napi_status create(
      NodeApiEnvironment &env,
      const vm::PinnedHermesValue *value,
      NodeApiWeakReference **result) noexcept {
    CHECK_OBJECT_ARG(value);
    CHECK_ARG(result);
    *result =
        new NodeApiWeakReference(env.createWeakRoot(getObjectUnsafe(*value)));
    env.addReference(*result);
    return env.clearLastNativeError();
  }

  const vm::PinnedHermesValue &value(
      NodeApiEnvironment &env) noexcept override {
    return env.lockWeakRoot(weakRoot_);
  }

  vm::WeakRoot<vm::JSObject> *getGCWeakRoot(
      NodeApiEnvironment &env) noexcept override {
    if (refCount() > 0) {
      return &weakRoot_;
    } else {
      deleteReference(env, this, ReasonToDelete::ZeroRefCount);
      return nullptr;
    }
  }

 protected:
  NodeApiWeakReference(vm::WeakRoot<vm::JSObject> weakRoot) noexcept
      : weakRoot_(weakRoot) {}

 private:
  vm::WeakRoot<vm::JSObject> weakRoot_;
};

// Keep vm::PinnedHermesValue when ref count > 0 or vm::WeakRoot<vm::JSObject>
// when ref count == 0. The ref count is not atomic and must be changed only
// from the JS thread.
class NodeApiComplexReference : public NodeApiReference {
 public:
  static napi_status create(
      NodeApiEnvironment &env,
      const vm::PinnedHermesValue *value,
      uint32_t initialRefCount,
      NodeApiComplexReference **result) noexcept {
    CHECK_OBJECT_ARG(value);
    CHECK_ARG(result);
    *result = new NodeApiComplexReference(
        initialRefCount,
        *value,
        initialRefCount == 0 ? env.createWeakRoot(getObjectUnsafe(*value))
                             : vm::WeakRoot<vm::JSObject>{});
    env.addReference(*result);
    return env.clearLastNativeError();
  }

  napi_status incRefCount(NodeApiEnvironment &env, uint32_t &result) noexcept
      override {
    if (refCount_ == 0) {
      value_ = env.lockWeakRoot(weakRoot_);
    }
    CRASH_IF_FALSE(++refCount_ < MaxRefCount && "The ref count is too big.");
    result = refCount_;
    return env.clearLastNativeError();
  }

  napi_status decRefCount(NodeApiEnvironment &env, uint32_t &result) noexcept
      override {
    if (refCount_ == 0) {
      // Ignore this error situation to match Node-API for V8 implementation.
      result = 0;
      return napi_ok;
    }
    if (--refCount_ == 0) {
      weakRoot_.~WeakRoot();
      if (value_.isObject()) {
        ::new (std::addressof(weakRoot_)) vm::WeakRoot<vm::JSObject>(env.createWeakRoot(getObjectUnsafe(value_)));
      } else {
        ::new (std::addressof(weakRoot_)) vm::WeakRoot<vm::JSObject>();
      }
    }
    result = refCount_;
    return env.clearLastNativeError();
  }

  const vm::PinnedHermesValue &value(
      NodeApiEnvironment &env) noexcept override {
    if (refCount_ > 0) {
      return value_;
    } else {
      return env.lockWeakRoot(weakRoot_);
    }
  }

  vm::PinnedHermesValue *getGCRoot(
      NodeApiEnvironment & /*env*/) noexcept override {
    return (refCount_ > 0) ? &value_ : nullptr;
  }

  vm::WeakRoot<vm::JSObject> *getGCWeakRoot(
      NodeApiEnvironment & /*env*/) noexcept override {
    return (refCount_ == 0 && weakRoot_) ? &weakRoot_ : nullptr;
  }

 protected:
  NodeApiComplexReference(
      uint32_t initialRefCount,
      const vm::PinnedHermesValue &value,
      vm::WeakRoot<vm::JSObject> weakRoot) noexcept
      : refCount_(initialRefCount), value_(value), weakRoot_(weakRoot) {}

  uint32_t refCount() const noexcept {
    return refCount_;
  }

 private:
  uint32_t refCount_{0};
  vm::PinnedHermesValue value_;
  vm::WeakRoot<vm::JSObject> weakRoot_;

  static constexpr uint32_t MaxRefCount =
      std::numeric_limits<uint32_t>::max() / 2;
};

// Store finalizeHint if it is not null.
template <class TBaseReference>
class NodeApiFinalizeHintHolder : public TBaseReference {
 public:
  template <class... TArgs>
  NodeApiFinalizeHintHolder(void *finalizeHint, TArgs &&...args) noexcept
      : TBaseReference(std::forward<TArgs>(args)...),
        finalizeHint_(finalizeHint) {}

  void *finalizeHint() noexcept override {
    return finalizeHint_;
  }

 private:
  void *finalizeHint_;
};

// Store and call finalizeCallback if it is not null.
template <class TBaseReference>
class NodeApiFinalizeCallbackHolder : public TBaseReference {
  using Super = TBaseReference;

 public:
  template <class... TArgs>
  NodeApiFinalizeCallbackHolder(
      napi_finalize finalizeCallback,
      TArgs &&...args) noexcept
      : TBaseReference(std::forward<TArgs>(args)...),
        finalizeCallback_(finalizeCallback) {}

  napi_status callFinalizeCallback(NodeApiEnvironment &env) noexcept override {
    if (finalizeCallback_) {
      napi_finalize finalizeCallback =
          std::exchange(finalizeCallback_, nullptr);
      env.callFinalizer(
          finalizeCallback, Super::nativeData(), Super::finalizeHint());
    }
    return napi_ok;
  }

 private:
  napi_finalize finalizeCallback_{};
};

// Store nativeData if it is not null.
template <class TBaseReference>
class NodeApiNativeDataHolder : public TBaseReference {
 public:
  template <class... TArgs>
  NodeApiNativeDataHolder(void *nativeData, TArgs &&...args) noexcept
      : TBaseReference(std::forward<TArgs>(args)...), nativeData_(nativeData) {}

  void *nativeData() noexcept override {
    return nativeData_;
  }

 private:
  void *nativeData_;
};

// Common code for references inherited from NodeApiFinalizer.
template <class TBaseReference>
class NodeApiFinalizingReference final : public TBaseReference {
  using Super = TBaseReference;

 public:
  template <class... TArgs>
  NodeApiFinalizingReference(TArgs &&...args) noexcept
      : TBaseReference(std::forward<TArgs>(args)...) {}

  void finalize(NodeApiEnvironment &env) noexcept override {
    Super::callFinalizeCallback(env);
    NodeApiReference::deleteReference(
        env, this, NodeApiReference::ReasonToDelete::FinalizerCall);
  }
};

// Create NodeApiFinalizingReference with the optimized storage.
template <class TReference>
class NodeApiFinalizingReferenceFactory final {
 public:
  template <class... TArgs>
  static TReference *create(
      void *nativeData,
      napi_finalize finalizeCallback,
      void *finalizeHint,
      TArgs &&...args) noexcept {
    int selector = (nativeData ? 0b100 : 0) | (finalizeCallback ? 0b010 : 0) |
        (finalizeHint ? 0b001 : 0);
    switch (selector) {
      default:
      case 0b000:
      case 0b001:
        return new NodeApiFinalizingReference<TReference>(
            std::forward<TArgs>(args)...);
      case 0b010:
        return new NodeApiFinalizingReference<
            NodeApiFinalizeCallbackHolder<TReference>>(
            finalizeCallback, std::forward<TArgs>(args)...);
      case 0b011:
        return new NodeApiFinalizingReference<NodeApiFinalizeCallbackHolder<
            NodeApiFinalizeHintHolder<TReference>>>(
            finalizeCallback, finalizeHint, std::forward<TArgs>(args)...);
      case 0b100:
      case 0b101:
        return new NodeApiFinalizingReference<
            NodeApiNativeDataHolder<TReference>>(
            nativeData, std::forward<TArgs>(args)...);
      case 0b110:
        return new NodeApiFinalizingReference<
            NodeApiNativeDataHolder<NodeApiFinalizeCallbackHolder<TReference>>>(
            nativeData, finalizeCallback, std::forward<TArgs>(args)...);
      case 0b111:
        return new NodeApiFinalizingReference<
            NodeApiNativeDataHolder<NodeApiFinalizeCallbackHolder<
                NodeApiFinalizeHintHolder<TReference>>>>(
            nativeData,
            finalizeCallback,
            finalizeHint,
            std::forward<TArgs>(args)...);
    }
  }
};

// The reference that is never returned to the user code and only used to hold
// the native data and its finalizer callback.
// It is either deleted from the finalizer queue, on environment shutdown, or
// directly when deleting the object wrap.
class NodeApiFinalizingAnonymousReference : public NodeApiReference,
                                            public NodeApiFinalizer {
 public:
  static napi_status create(
      NodeApiEnvironment &env,
      const vm::PinnedHermesValue *value,
      void *nativeData,
      napi_finalize finalizeCallback,
      void *finalizeHint,
      /*optional*/ NodeApiFinalizingAnonymousReference **result) noexcept {
    NodeApiFinalizingAnonymousReference *ref =
        NodeApiFinalizingReferenceFactory<NodeApiFinalizingAnonymousReference>::
            create(nativeData, finalizeCallback, finalizeHint);
    if (value != nullptr) {
      CHECK_OBJECT_ARG(value);
      env.addObjectFinalizer(value, ref);
    }
    env.addFinalizingReference(ref);
    return env.setOptionalResult(std::move(ref), result);
  }
};

// Associates data with NodeApiStrongReference.
class NodeApiFinalizingStrongReference : public NodeApiStrongReference,
                                         public NodeApiFinalizer {
 public:
  static napi_status create(
      NodeApiEnvironment &env,
      const vm::PinnedHermesValue *value,
      void *nativeData,
      napi_finalize finalizeCallback,
      void *finalizeHint,
      NodeApiFinalizingStrongReference **result) noexcept {
    CHECK_ARG(value);
    CHECK_ARG(*result);
    *result =
        NodeApiFinalizingReferenceFactory<NodeApiFinalizingStrongReference>::
            create(nativeData, finalizeCallback, finalizeHint, *value);
    env.addFinalizingReference(*result);
    return env.clearLastNativeError();
  }

 protected:
  NodeApiFinalizingStrongReference(const vm::PinnedHermesValue &value) noexcept
      : NodeApiStrongReference(value) {}

  bool startDeleting(NodeApiEnvironment &env, ReasonToDelete reason) noexcept
      override {
    if (reason == ReasonToDelete::ZeroRefCount) {
      // Let the finalizer to run first.
      env.addToFinalizerQueue(this);
      return false;
    } else if (reason == ReasonToDelete::FinalizerCall) {
      if (refCount() != 0) {
        // On shutdown the finalizer is called when the ref count is not zero
        // yet. Postpone the deletion until all finalizers are finished to run.
        NodeApiFinalizer::unlink();
        env.addReference(this);
        return false;
      }
    }
    return true;
  }
};

// A reference that can be either strong or weak and that holds a finalizer
// callback.
class NodeApiFinalizingComplexReference : public NodeApiComplexReference,
                                          public NodeApiFinalizer {
 public:
  static napi_status create(
      NodeApiEnvironment &env,
      uint32_t initialRefCount,
      bool deleteSelf,
      const vm::PinnedHermesValue *value,
      void *nativeData,
      napi_finalize finalizeCallback,
      void *finalizeHint,
      NodeApiFinalizingComplexReference **result) noexcept {
    CHECK_OBJECT_ARG(value);
    CHECK_ARG(result);
    *result =
        NodeApiFinalizingReferenceFactory<NodeApiFinalizingComplexReference>::
            create(
                nativeData,
                finalizeCallback,
                finalizeHint,
                initialRefCount,
                deleteSelf,
                *value,
                initialRefCount == 0
                    ? env.createWeakRoot(getObjectUnsafe(*value))
                    : vm::WeakRoot<vm::JSObject>{});
    if (initialRefCount == 0) {
      env.addObjectFinalizer(value, *result);
    }
    env.addFinalizingReference(*result);
    return env.clearLastNativeError();
  }

  napi_status incRefCount(NodeApiEnvironment &env, uint32_t &result) noexcept
      override {
    CHECK_NAPI(NodeApiComplexReference::incRefCount(env, result));
    if (result == 1) {
      NodeApiLinkedList<NodeApiFinalizer>::Item::unlink();
    }
    return env.clearLastNativeError();
  }

  napi_status decRefCount(NodeApiEnvironment &env, uint32_t &result) noexcept
      override {
    vm::PinnedHermesValue hv;
    bool shouldConvertToWeakRef = refCount() == 1;
    if (shouldConvertToWeakRef) {
      hv = value(env);
    }
    CHECK_NAPI(NodeApiComplexReference::decRefCount(env, result));
    if (shouldConvertToWeakRef && hv.isObject()) {
      return env.addObjectFinalizer(&hv, this);
    }
    return env.clearLastNativeError();
  }

 protected:
  NodeApiFinalizingComplexReference(
      uint32_t initialRefCount,
      bool deleteSelf,
      const vm::PinnedHermesValue &value,
      vm::WeakRoot<vm::JSObject> weakRoot) noexcept
      : NodeApiComplexReference{initialRefCount, value, weakRoot},
        deleteSelf_{deleteSelf} {}

  bool startDeleting(NodeApiEnvironment &env, ReasonToDelete reason) noexcept
      override {
    if (reason == ReasonToDelete::ExternalCall &&
        NodeApiLinkedList<NodeApiFinalizer>::Item::isLinked()) {
      // Let the finalizer or the environment shutdown to delete the reference.
      deleteSelf_ = true;
      return false;
    }
    if (reason == ReasonToDelete::FinalizerCall && !deleteSelf_) {
      // Let the external call or the environment shutdown to delete the
      // reference.
      NodeApiFinalizer::unlink();
      env.addReference(this);
      return false;
    }
    return true;
  }

 private:
  bool deleteSelf_{false};
};

// Hold custom data associated with the NodeApiEnvironment.
class NodeApiInstanceData : public NodeApiReference {
 public:
  static napi_status create(
      NodeApiEnvironment &env,
      void *nativeData,
      napi_finalize finalizeCallback,
      void *finalizeHint,
      /*optional*/ NodeApiInstanceData **result) noexcept {
    NodeApiInstanceData *ref =
        NodeApiFinalizingReferenceFactory<NodeApiInstanceData>::create(
            nativeData, finalizeCallback, finalizeHint);
    if (result) {
      *result = ref;
    }
    return env.clearLastNativeError();
  }
};

// Sorted list of unique HermesValues.
template <>
class NodeApiOrderedSet<vm::HermesValue> final {
 public:
  using Compare =
      int32_t(const vm::HermesValue &item1, const vm::HermesValue &item2);

  NodeApiOrderedSet(NodeApiEnvironment &env, Compare *compare) noexcept
      : env_(env), compare_(compare) {
    env_.pushOrderedSet(*this);
  }

  ~NodeApiOrderedSet() {
    env_.popOrderedSet();
  }

  bool insert(vm::HermesValue value) noexcept {
    auto it = llvh::lower_bound(
        items_,
        value,
        [this](const vm::HermesValue &item1, const vm::HermesValue &item2) {
          return (*compare_)(item1, item2) < 0;
        });
    if (it != items_.end() && (*compare_)(*it, value) == 0) {
      return false;
    }
    items_.insert(it, value);
    return true;
  }

  static void getGCRoots(
      llvh::iterator_range<NodeApiOrderedSet **> range,
      vm::RootAcceptor &acceptor) noexcept {
    for (NodeApiOrderedSet *set : range) {
      for (vm::PinnedHermesValue &value : set->items_) {
        acceptor.accept(value);
      }
    }
  }

 private:
  NodeApiEnvironment &env_;
  llvh::SmallVector<vm::PinnedHermesValue, 16> items_;
  Compare *compare_{};
};

// Sorted list of unique uint32_t.
template <>
class NodeApiOrderedSet<uint32_t> final {
 public:
  bool insert(uint32_t value) noexcept {
    auto it = llvh::lower_bound(items_, value);
    if (it == items_.end() || *it == value) {
      return false;
    }
    items_.insert(it, value);
    return true;
  }

 private:
  llvh::SmallVector<uint32_t, 16> items_;
};

// Helper class to build a string.
class NodeApiStringBuilder final {
 public:
  // To adopt an existing string instead of creating a new one.
  class AdoptStringTag {};
  constexpr static AdoptStringTag AdoptString{};

  NodeApiStringBuilder(AdoptStringTag, std::string &&str) noexcept
      : str_(std::move(str)), stream_(str_) {}

  template <class... TArgs>
  NodeApiStringBuilder(TArgs &&...args) noexcept : stream_(str_) {
    append(std::forward<TArgs>(args)...);
  }

  NodeApiStringBuilder &append() noexcept {
    return *this;
  }

  template <class TArg0, class... TArgs>
  NodeApiStringBuilder &append(TArg0 &&arg0, TArgs &&...args) noexcept {
    stream_ << arg0;
    return append(std::forward<TArgs>(args)...);
  }

  std::string &str() noexcept {
    stream_.flush();
    return str_;
  }

  const char *c_str() noexcept {
    return str().c_str();
  }

  napi_status makeHVString(
      NodeApiEnvironment &env,
      vm::MutableHandle<> *result) noexcept {
    stream_.flush();
    vm::CallResult<vm::HermesValue> res = vm::StringPrimitive::createEfficient(
        env.runtime(), llvh::makeArrayRef(str_.data(), str_.size()));
    return env.setResult(std::move(res), result);
  }

 private:
  std::string str_;
  llvh::raw_string_ostream stream_;
};

class NodeApiExternalBufferCore {
 public:
  NodeApiExternalBufferCore(
      NodeApiEnvironment &env,
      void *data,
      napi_finalize finalizeCallback,
      void *finalizeHint)
      : env_(&env),
        finalizeCallback_(finalizeCallback),
        data_(data),
        finalizeHint_(finalizeHint) {}

  void setFinalizer(NodeApiFinalizer *finalizer) {
    finalizer_ = finalizer;
  }

  void onBufferDeleted() {
    if (finalizer_ != nullptr) {
      env_->addToFinalizerQueue(finalizer_);
      env_ = nullptr;
    } else {
      delete this;
    }
  }

  static void
  finalize(napi_env env, void * /*finalizeData*/, void *finalizeHint) {
    NodeApiExternalBufferCore *core =
        reinterpret_cast<NodeApiExternalBufferCore *>(finalizeHint);
    if (core->finalizeCallback_ != nullptr) {
      core->finalizeCallback_(env, core->data_, core->finalizeHint_);
    }

    core->finalizer_ = nullptr;
    if (core->env_ == nullptr) {
      delete core;
    }
  }

  NodeApiExternalBufferCore(const NodeApiExternalBufferCore &) = delete;
  NodeApiExternalBufferCore &operator=(const NodeApiExternalBufferCore &) =
      delete;

 private:
  NodeApiFinalizer *finalizer_{};
  NodeApiEnvironment *env_;
  napi_finalize finalizeCallback_;
  void *data_;
  void *finalizeHint_;
};

// The external buffer that implements hermes::Buffer
class NodeApiExternalBuffer final : public hermes::Buffer {
 public:
  static std::unique_ptr<NodeApiExternalBuffer> make(
      napi_env env,
      void *bufferData,
      size_t bufferSize,
      napi_finalize finalizeCallback,
      void *finalizeHint) noexcept {
    return bufferData ? std::make_unique<NodeApiExternalBuffer>(
                            *reinterpret_cast<NodeApiEnvironment *>(env),
                            bufferData,
                            bufferSize,
                            finalizeCallback,
                            finalizeHint)
                      : nullptr;
  }

  NodeApiExternalBuffer(
      NodeApiEnvironment &env,
      void *bufferData,
      size_t bufferSize,
      napi_finalize finalizeCallback,
      void *finalizeHint) noexcept
      : Buffer(reinterpret_cast<uint8_t *>(bufferData), bufferSize),
        core_(new NodeApiExternalBufferCore(
            env,
            bufferData,
            finalizeCallback,
            finalizeHint)) {
    NodeApiFinalizingAnonymousReference *ref =
        NodeApiFinalizingReferenceFactory<NodeApiFinalizingAnonymousReference>::
            create(nullptr, &NodeApiExternalBufferCore::finalize, core_);
    core_->setFinalizer(ref);
    env.addFinalizingReference(ref);
  }

  ~NodeApiExternalBuffer() noexcept override {
    core_->onBufferDeleted();
  }

  NodeApiExternalBuffer(const NodeApiExternalBuffer &) = delete;
  NodeApiExternalBuffer &operator=(const NodeApiExternalBuffer &) = delete;

 private:
  NodeApiExternalBufferCore *core_;
};

// An implementation of PreparedJavaScript that wraps a BytecodeProvider.
class NodeApiScriptModel final {
 public:
  explicit NodeApiScriptModel(
      std::unique_ptr<hbc::BCProvider> bcProvider,
      vm::RuntimeModuleFlags runtimeFlags,
      std::string sourceURL,
      bool isBytecode)
      : bcProvider_(std::move(bcProvider)),
        runtimeFlags_(runtimeFlags),
        sourceURL_(std::move(sourceURL)),
        isBytecode_(isBytecode) {}

  std::shared_ptr<hbc::BCProvider> bytecodeProvider() const {
    return bcProvider_;
  }

  vm::RuntimeModuleFlags runtimeFlags() const {
    return runtimeFlags_;
  }

  const std::string &sourceURL() const {
    return sourceURL_;
  }

  bool isBytecode() const {
    return isBytecode_;
  }

 private:
  std::shared_ptr<hbc::BCProvider> bcProvider_;
  vm::RuntimeModuleFlags runtimeFlags_;
  std::string sourceURL_;
  bool isBytecode_{false};
};

// Conversion routines from double to int32, uin32 and int64.
// The code is adapted from V8 source code to match the Node-API for V8
// behavior. https://github.com/v8/v8/blob/main/src/numbers/conversions-inl.h
// https://github.com/v8/v8/blob/main/src/base/numbers/double.h
class NodeApiDoubleConversion final {
 public:
  // Implements most of https://tc39.github.io/ecma262/#sec-toint32.
  static int32_t toInt32(double value) noexcept {
    if (!std::isnormal(value)) {
      return 0;
    }
    if (value >= std::numeric_limits<int32_t>::min() &&
        value <= std::numeric_limits<int32_t>::max()) {
      // All doubles within these limits are trivially convertable to an int32.
      return static_cast<int32_t>(value);
    }
    uint64_t u64 = toUint64Bits(value);
    int exponent = getExponent(u64);
    uint64_t bits;
    if (exponent < 0) {
      if (exponent <= -kSignificandSize) {
        return 0;
      }
      bits = getSignificand(u64) >> -exponent;
    } else {
      if (exponent > 31) {
        return 0;
      }
      bits = getSignificand(u64) << exponent;
    }
    return static_cast<int32_t>(
        getSign(u64) * static_cast<int64_t>(bits & 0xFFFFFFFFul));
  }

  static uint32_t toUint32(double value) noexcept {
    return static_cast<uint32_t>(toInt32(value));
  }

  static int64_t toInt64(double value) {
    // This code has the Node-API for V8 special behavior.
    // The comment from the napi_get_value_int64 code:
    // https://github.com/nodejs/node/blob/master/src/js_native_api_v8.cc
    //
    // v8::Value::IntegerValue() converts NaN, +Inf, and -Inf to INT64_MIN,
    // inconsistent with v8::Value::Int32Value() which converts those values to
    // 0. Special-case all non-finite values to match that behavior.
    //
    if (!std::isnormal(value)) {
      return 0;
    }
    if (value >= static_cast<double>(std::numeric_limits<int64_t>::max())) {
      return std::numeric_limits<int64_t>::max();
    }
    if (value <= static_cast<double>(std::numeric_limits<int64_t>::min())) {
      return std::numeric_limits<int64_t>::min();
    }
    return static_cast<int64_t>(value);
  }

 private:
  static uint64_t toUint64Bits(double value) noexcept {
    uint64_t result;
    std::memcpy(&result, &value, sizeof(value));
    return result;
  }

  static int getSign(uint64_t u64) noexcept {
    return (u64 & kSignMask) == 0 ? 1 : -1;
  }

  static int getExponent(uint64_t u64) noexcept {
    int biased_e =
        static_cast<int>((u64 & kExponentMask) >> kPhysicalSignificandSize);
    return biased_e - kExponentBias;
  }

  static uint64_t getSignificand(uint64_t u64) noexcept {
    return (u64 & kSignificandMask) + kHiddenBit;
  }

  static constexpr uint64_t kSignMask = 0x8000'0000'0000'0000;
  static constexpr uint64_t kExponentMask = 0x7FF0'0000'0000'0000;
  static constexpr uint64_t kSignificandMask = 0x000F'FFFF'FFFF'FFFF;
  static constexpr uint64_t kHiddenBit = 0x0010'0000'0000'0000;
  static constexpr int kPhysicalSignificandSize = 52;
  static constexpr int kSignificandSize = 53;
  static constexpr int kExponentBias = 0x3FF + kPhysicalSignificandSize;
};

class NodeApiEnvironmentHolder {
 public:
  napi_env getOrCreateEnvironment(
      vm::Runtime &runtime,
      int32_t apiVersion) noexcept {
    if (rootEnv_ == nullptr) {
      rootEnv_ = std::make_unique<NodeApiEnvironment>(runtime, apiVersion);
    }
    return napiEnv(rootEnv_.get());
  }

  napi_env createModuleEnvironment(
      vm::Runtime &runtime,
      int32_t apiVersion) noexcept {
    auto env = std::make_unique<NodeApiEnvironment>(runtime, apiVersion);
    napi_env result = napiEnv(env.get());
    modelEnvs_.push_back(std::move(env));
    return result;
  }

  static vm::CallResult<NodeApiEnvironmentHolder *> fromRuntime(
      vm::Runtime &runtime) {
    vm::GCScope gcScope(runtime);
    vm::HermesValue globalObjectHV = runtime.getGlobal().getHermesValue();
    vm::Handle<vm::JSObject> globalObjectHandle =
        vm::Handle<vm::JSObject>::vmcast(runtime, globalObjectHV);
    vm::SymbolID propSymbol = vm::Predefined::getSymbolID(
        vm::Predefined::InternalPropertyArrayBufferExternalFinalizer);
    vm::NamedPropertyDescriptor desc;
    bool exists = vm::JSObject::getOwnNamedDescriptor(
        globalObjectHandle, runtime, propSymbol, desc);
    if (exists) {
      // Raw pointers below.
      vm::NoAllocScope scope(runtime);
      vm::NativeState *ns =
          vm::vmcast<vm::NativeState>(vm::JSObject::getNamedSlotValueUnsafe(
                                          *globalObjectHandle, runtime, desc)
                                          .getObject(runtime));
      return reinterpret_cast<NodeApiEnvironmentHolder *>(ns->context());
    }

    NodeApiEnvironmentHolder *holder = new NodeApiEnvironmentHolder();
    vm::Handle<vm::NativeState> ns = runtime.makeHandle(
        vm::NativeState::create(runtime, holder, deleteHolder));
    vm::CallResult<bool> res = vm::JSObject::defineOwnProperty(
        globalObjectHandle,
        runtime,
        propSymbol,
        vm::DefinePropertyFlags::getDefaultNewPropertyFlags(),
        ns,
        vm::PropOpFlags().plusThrowOnError());
    if (res.getStatus() == vm::ExecutionStatus::EXCEPTION) {
      return vm::ExecutionStatus::EXCEPTION;
    }
    return holder;
  }

 private:
  static void deleteHolder(vm::GC &, vm::NativeState *ns) {
    delete reinterpret_cast<NodeApiEnvironmentHolder *>(ns->context());
  }

 private:
  std::unique_ptr<NodeApiEnvironment> rootEnv_;
  std::vector<std::unique_ptr<NodeApiEnvironment>> modelEnvs_;
};

vm::CallResult<napi_env> getOrCreateRuntimeNodeApiEnvironment(
    vm::Runtime &runtime,
    int32_t apiVersion) noexcept {
  vm::CallResult<NodeApiEnvironmentHolder *> holderRes =
      NodeApiEnvironmentHolder::fromRuntime(runtime);
  if (holderRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  return (*holderRes)->getOrCreateEnvironment(runtime, apiVersion);
}

vm::CallResult<napi_env> createModuleNodeApiEnvironment(
    vm::Runtime &runtime,
    int32_t apiVersion) noexcept {
  vm::CallResult<NodeApiEnvironmentHolder *> holderRes =
      NodeApiEnvironmentHolder::fromRuntime(runtime);
  if (holderRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  return (*holderRes)->createModuleEnvironment(runtime, apiVersion);
}

// Max size of the runtime's register stack.
// The runtime register stack needs to be small enough to be allocated on the
// native thread stack in Android (1MiB) and on MacOS's thread stack (512 KiB)
// Calculated by: (thread stack size - size of runtime -
// 8 memory pages for other stuff in the thread)
constexpr unsigned kMaxNumRegisters =
    (512 * 1024 - sizeof(vm::Runtime) - 4096 * 8) /
    sizeof(vm::PinnedHermesValue);

template <class T, std::size_t N>
constexpr std::size_t size(const T (&array)[N]) noexcept {
  return N;
}

template <class TEnum>
bool isInEnumRange(
    TEnum value,
    TEnum lowerBoundInclusive,
    TEnum upperBoundInclusive) noexcept {
  return lowerBoundInclusive <= value && value <= upperBoundInclusive;
}

napi_env napiEnv(NodeApiEnvironment *env) noexcept {
  return reinterpret_cast<napi_env>(env);
}

napi_value napiValue(const vm::PinnedHermesValue *value) noexcept {
  return reinterpret_cast<napi_value>(
      const_cast<vm::PinnedHermesValue *>(value));
}

template <class T>
napi_value napiValue(vm::Handle<T> value) noexcept {
  return napiValue(value.unsafeGetPinnedHermesValue());
}

const vm::PinnedHermesValue *phv(napi_value value) noexcept {
  return reinterpret_cast<const vm::PinnedHermesValue *>(value);
}

const vm::PinnedHermesValue *phv(const vm::PinnedHermesValue *value) noexcept {
  return value;
}

NodeApiReference *asReference(napi_ref ref) noexcept {
  return reinterpret_cast<NodeApiReference *>(ref);
}

NodeApiReference *asReference(void *ref) noexcept {
  return reinterpret_cast<NodeApiReference *>(ref);
}

NodeApiCallbackInfo *asCallbackInfo(napi_callback_info callbackInfo) noexcept {
  return reinterpret_cast<NodeApiCallbackInfo *>(callbackInfo);
}

vm::JSObject *getObjectUnsafe(const vm::HermesValue &value) noexcept {
  return reinterpret_cast<vm::JSObject *>(value.getObject());
}

vm::JSObject *getObjectUnsafe(napi_value value) noexcept {
  return getObjectUnsafe(*phv(value));
}

size_t copyASCIIToUTF8(
    llvh::ArrayRef<char> input,
    char *buf,
    size_t maxCharacters) noexcept {
  size_t size = std::min(input.size(), maxCharacters);
  std::char_traits<char>::copy(buf, input.data(), size);
  return size;
}

size_t utf8LengthWithReplacements(llvh::ArrayRef<char16_t> input) {
  size_t length{0};
  for (const char16_t *cur = input.begin(), *end = input.end(); cur < end;) {
    char16_t c = *cur++;
    if (LLVM_LIKELY(c <= 0x7F)) {
      ++length;
    } else if (c <= 0x7FF) {
      length += 2;
    } else if (isLowSurrogate(c)) {
      // Unpaired low surrogate.
      length += 3; // replacement char is 0xFFFD
    } else if (isHighSurrogate(c)) {
      // Leading high surrogate. See if the next character is a low surrogate.
      if (LLVM_UNLIKELY(cur == end || !isLowSurrogate(*cur))) {
        // Trailing or unpaired high surrogate.
        length += 3; // replacement char is 0xFFFD
      } else {
        // The surrogate pair encodes a code point in range 0x10000-0x10FFFF
        // which is encoded as four UTF-8 characters.
        cur++; // to get the low surrogate char
        length += 4;
      }
    } else {
      // Not a surrogate.
      length += 3;
    }
  }

  return length;
}

size_t convertUTF16ToUTF8WithReplacements(
    llvh::ArrayRef<char16_t> input,
    char *buf,
    size_t bufSize) {
  char *curBuf = buf;
  char *endBuf = buf + bufSize;
  for (const char16_t *cur = input.begin(), *end = input.end();
       cur < end && curBuf < endBuf;) {
    char16_t c = *cur++;
    // ASCII fast-path.
    if (LLVM_LIKELY(c <= 0x7F)) {
      *curBuf++ = c;
      continue;
    }

    char32_t c32;
    if (LLVM_LIKELY(c <= 0x7FF)) {
      c32 = c;
    } else if (isLowSurrogate(c)) {
      // Unpaired low surrogate.
      c32 = UNICODE_REPLACEMENT_CHARACTER;
    } else if (isHighSurrogate(c)) {
      // Leading high surrogate. See if the next character is a low surrogate.
      if (LLVM_UNLIKELY(cur == end || !isLowSurrogate(*cur))) {
        // Trailing or unpaired high surrogate.
        c32 = UNICODE_REPLACEMENT_CHARACTER;
      } else {
        // Decode surrogate pair and increment, because we consumed two chars.
        c32 = utf16SurrogatePairToCodePoint(c, *cur++);
      }
    } else {
      // Not a surrogate.
      c32 = c;
    }

    char buff[UTF8CodepointMaxBytes];
    char *ptr = buff;
    encodeUTF8(ptr, c32);
    size_t u8Length = static_cast<size_t>(ptr - buff);
    if (curBuf + u8Length <= endBuf) {
      std::char_traits<char>::copy(curBuf, buff, u8Length);
      curBuf += u8Length;
    } else {
      break;
    }
  }

  return static_cast<size_t>(curBuf - buf);
}

//=============================================================================
// NodeApiEnvironment implementation
//=============================================================================

NodeApiEnvironment::NodeApiEnvironment(
    vm::Runtime &runtime,
    int32_t apiVersion) noexcept
    : runtime_(runtime),
      apiVersion_(apiVersion),
      pendingFinalizers_(NodeApiPendingFinalizers::create()) {
  // TODO: implement
  // switch (runtimeConfig.getCompilationMode()) {
  //   case vm::SmartCompilation:
  //     compileFlags_.lazy = true;
  //     // (Leaves thresholds at default values)
  //     break;
  //   case vm::ForceEagerCompilation:
  //     compileFlags_.lazy = false;
  //     break;
  //   case vm::ForceLazyCompilation:
  //     compileFlags_.lazy = true;
  //     compileFlags_.preemptiveFileCompilationThreshold = 0;
  //     compileFlags_.preemptiveFunctionCompilationThreshold = 0;
  //     break;
  // }

  //  compileFlags_.enableGenerator = runtimeConfig.getEnableGenerator();
  // compileFlags_.emitAsyncBreakCheck =
  // runtimeConfig.getAsyncBreakCheckInEval();

  runtime_.addCustomRootsFunction([this](vm::GC *, vm::RootAcceptor &acceptor) {
    napiValueStack_.forEach([&](const vm::PinnedHermesValue &value) {
      acceptor.accept(const_cast<vm::PinnedHermesValue &>(value));
    });
    NodeApiReference::getGCRoots(*this, references_, acceptor);
    NodeApiReference::getGCRoots(*this, finalizingReferences_, acceptor);
    if (!thrownJSError_.isEmpty()) {
      acceptor.accept(thrownJSError_);
    }
    if (!lastUnhandledRejection_.isEmpty()) {
      acceptor.accept(lastUnhandledRejection_);
    }
    for (vm::PinnedHermesValue &value : predefinedValues_) {
      acceptor.accept(value);
    }
    NodeApiOrderedSet<vm::HermesValue>::getGCRoots(orderedSets_, acceptor);
    for (auto &entry : uniqueStrings_) {
      if (vm::PinnedHermesValue *root = entry.second->getGCRoot(*this)) {
        acceptor.accept(*root);
      }
    }
  });
  runtime_.addCustomWeakRootsFunction(
      [this](vm::GC *, vm::WeakRootAcceptor &acceptor) {
        NodeApiReference::getGCWeakRoots(*this, references_, acceptor);
        NodeApiReference::getGCWeakRoots(
            *this, finalizingReferences_, acceptor);
      });

  vm::GCScope gcScope{runtime_};
  auto setPredefinedProperty =
      [this](NodeApiPredefined key, vm::HermesValue value) noexcept {
        predefinedValues_[static_cast<size_t>(key)] = value;
      };
  setPredefinedProperty(
      NodeApiPredefined::Promise,
      vm::HermesValue::encodeSymbolValue(
          runtime_.getIdentifierTable().registerLazyIdentifier(
              vm::createASCIIRef("Promise"))));
  setPredefinedProperty(
      NodeApiPredefined::allRejections,
      vm::HermesValue::encodeSymbolValue(
          runtime_.getIdentifierTable().registerLazyIdentifier(
              vm::createASCIIRef("allRejections"))));
  setPredefinedProperty(
      NodeApiPredefined::code,
      vm::HermesValue::encodeSymbolValue(
          runtime_.getIdentifierTable().registerLazyIdentifier(
              vm::createASCIIRef("code"))));
  setPredefinedProperty(
      NodeApiPredefined::hostFunction,
      vm::HermesValue::encodeSymbolValue(
          runtime_.getIdentifierTable().registerLazyIdentifier(
              vm::createASCIIRef("hostFunction"))));
  setPredefinedProperty(
      NodeApiPredefined::napi_externalValue,
      vm::HermesValue::encodeSymbolValue(
          runtime_.getIdentifierTable().createNotUniquedLazySymbol(
              vm::createASCIIRef(
                  "node_api.externalValue.735e14c9-354f-489b-9f27-02acbc090975"))));
  setPredefinedProperty(
      NodeApiPredefined::napi_typeTag,
      vm::HermesValue::encodeSymbolValue(
          runtime_.getIdentifierTable().createNotUniquedLazySymbol(
              vm::createASCIIRef(
                  "node_api.typeTag.026ae0ec-b391-49da-a935-0cab733ab615"))));
  setPredefinedProperty(
      NodeApiPredefined::onHandled,
      vm::HermesValue::encodeSymbolValue(
          runtime_.getIdentifierTable().registerLazyIdentifier(
              vm::createASCIIRef("onHandled"))));
  setPredefinedProperty(
      NodeApiPredefined::onUnhandled,
      vm::HermesValue::encodeSymbolValue(
          runtime_.getIdentifierTable().registerLazyIdentifier(
              vm::createASCIIRef("onUnhandled"))));
  setPredefinedProperty(
      NodeApiPredefined::reject,
      vm::HermesValue::encodeSymbolValue(
          runtime_.getIdentifierTable().registerLazyIdentifier(
              vm::createASCIIRef("reject"))));
  setPredefinedProperty(
      NodeApiPredefined::resolve,
      vm::HermesValue::encodeSymbolValue(
          runtime_.getIdentifierTable().registerLazyIdentifier(
              vm::createASCIIRef("resolve"))));

  CRASH_IF_FALSE(enablePromiseRejectionTracker() == napi_ok);
}

NodeApiEnvironment::~NodeApiEnvironment() {
  pendingFinalizers_->applyPendingFinalizers(this);
  pendingFinalizers_ = nullptr;

  isShuttingDown_ = true;
  if (instanceData_) {
    instanceData_->finalize(*this);
    instanceData_ = nullptr;
  }

  // First we must finalize those references that have `napi_finalizer`
  // callbacks. The reason is that addons might store other references which
  // they delete during their `napi_finalizer` callbacks. If we deleted such
  // references here first, they would be doubly deleted when the
  // `napi_finalizer` deleted them subsequently.
  NodeApiReference::finalizeAll(*this, finalizerQueue_);
  NodeApiReference::finalizeAll(*this, finalizingReferences_);
  NodeApiReference::deleteAll(
      *this,
      references_,
      NodeApiReference::ReasonToDelete::EnvironmentShutdown);

  CRASH_IF_FALSE(finalizerQueue_.isEmpty());
  CRASH_IF_FALSE(finalizingReferences_.isEmpty());
  CRASH_IF_FALSE(references_.isEmpty());
}

napi_status NodeApiEnvironment::incRefCount() noexcept {
  refCount_++;
  return napi_status::napi_ok;
}

napi_status NodeApiEnvironment::decRefCount() noexcept {
  if (--refCount_ == 0) {
    delete this;
  }
  return napi_status::napi_ok;
}

vm::Runtime &NodeApiEnvironment::runtime() noexcept {
  return runtime_;
}

NodeApiStableAddressStack<vm::PinnedHermesValue> &
NodeApiEnvironment::napiValueStack() noexcept {
  return napiValueStack_;
}

//---------------------------------------------------------------------------
// Native error handling methods
//---------------------------------------------------------------------------

napi_status NodeApiEnvironment::getLastNativeError(
    const NodeApiNativeError **result) noexcept {
  CHECK_ARG(result);
  if (lastError_.error_code == napi_ok) {
    lastError_ = {nullptr, 0, 0, napi_ok};
  }
  *result = &lastError_;
  return napi_ok;
}

template <class... TArgs>
napi_status NodeApiEnvironment::setLastNativeError(
    napi_status status,
    const char *fileName,
    uint32_t line,
    TArgs &&...args) noexcept {
  // Warning: Keep in-sync with napi_status enum
  static constexpr const char *errorMessages[] = {
      "",
      "Invalid argument",
      "An object was expected",
      "A string was expected",
      "A string or symbol was expected",
      "A function was expected",
      "A number was expected",
      "A boolean was expected",
      "An array was expected",
      "Unknown failure",
      "An exception is pending",
      "The async work item was cancelled",
      "napi_escape_handle already called on scope",
      "Invalid handle scope usage",
      "Invalid callback scope usage",
      "Thread-safe function queue is full",
      "Thread-safe function handle is closing",
      "A bigint was expected",
      "A date was expected",
      "An arraybuffer was expected",
      "A detachable arraybuffer was expected",
      "Main thread would deadlock",
  };

  // The value of the constant below must be updated to reference the last
  // message in the `napi_status` enum each time a new error message is added.
  // We don't have a napi_status_last as this would result in an ABI
  // change each time a message was added.
  const int lastStatus = napi_would_deadlock;
  static_assert(
      size(errorMessages) == lastStatus + 1,
      "Count of error messages must match count of error values");

  if (status < napi_ok || status >= lastStatus) {
    status = napi_generic_failure;
  }

  lastErrorMessage_.clear();
  NodeApiStringBuilder sb{
      NodeApiStringBuilder::AdoptString, std::move(lastErrorMessage_)};
  sb.append(errorMessages[status]);
  if (sizeof...(args) > 0) {
    sb.append(": ", std::forward<TArgs>(args)...);
  }
  sb.append("\nFile: ", fileName);
  sb.append("\nLine: ", line);
  lastErrorMessage_ = std::move(sb.str());
  // TODO: Find a better way to provide the extended error message
  lastError_ = {errorMessages[status], 0, 0, status};

#if defined(_WIN32) && !defined(NDEBUG)
//  DebugBreak();
#endif

  return status;
}

napi_status NodeApiEnvironment::clearLastNativeError() noexcept {
  return lastError_.error_code = napi_ok;
}

//-----------------------------------------------------------------------------
// Methods to support JS error handling
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::createJSError(
    const vm::PinnedHermesValue &errorPrototype,
    napi_value code,
    napi_value message,
    napi_value *result) noexcept {
  NodeApiHandleScope scope{*this, result};
  CHECK_STRING_ARG(message);
  vm::Handle<vm::JSError> errorHandle = makeHandle(
      vm::JSError::create(runtime_, makeHandle<vm::JSObject>(&errorPrototype)));
  CHECK_NAPI(checkJSErrorStatus(
      vm::JSError::setMessage(errorHandle, runtime_, makeHandle(message))));
  CHECK_NAPI(setJSErrorCode(errorHandle, code, nullptr));
  return scope.setResult(std::move(errorHandle));
}

napi_status NodeApiEnvironment::createJSError(
    napi_value code,
    napi_value message,
    napi_value *result) noexcept {
  return createJSError(runtime_.ErrorPrototype, code, message, result);
}

napi_status NodeApiEnvironment::createJSTypeError(
    napi_value code,
    napi_value message,
    napi_value *result) noexcept {
  return createJSError(runtime_.TypeErrorPrototype, code, message, result);
}

napi_status NodeApiEnvironment::createJSRangeError(
    napi_value code,
    napi_value message,
    napi_value *result) noexcept {
  return createJSError(runtime_.RangeErrorPrototype, code, message, result);
}

napi_status NodeApiEnvironment::isJSError(
    napi_value value,
    bool *result) noexcept {
  CHECK_ARG(value);
  return setResult(vm::vmisa<vm::JSError>(*phv(value)), result);
}

napi_status NodeApiEnvironment::throwJSError(napi_value error) noexcept {
  CHECK_ARG(error);
  runtime_.setThrownValue(*phv(error));
  // any VM calls after this point and before returning
  // to the javascript invoker will fail
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::throwJSError(
    const vm::PinnedHermesValue &prototype,
    const char *code,
    const char *message) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};

  napi_value messageValue;
  CHECK_NAPI(createStringUTF8(message, &messageValue));

  vm::Handle<vm::JSError> errorHandle = makeHandle(
      vm::JSError::create(runtime_, makeHandle<vm::JSObject>(&prototype)));
  CHECK_NAPI(
      checkJSErrorStatus(vm::JSError::recordStackTrace(errorHandle, runtime_)));
  CHECK_NAPI(checkJSErrorStatus(vm::JSError::setMessage(
      errorHandle, runtime_, makeHandle(messageValue))));
  CHECK_NAPI(setJSErrorCode(errorHandle, nullptr, code));

  runtime_.setThrownValue(errorHandle.getHermesValue());

  // any VM calls after this point and before returning
  // to the javascript invoker will fail
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::throwJSError(
    const char *code,
    const char *message) noexcept {
  return throwJSError(runtime_.ErrorPrototype, code, message);
}

napi_status NodeApiEnvironment::throwJSTypeError(
    const char *code,
    const char *message) noexcept {
  return throwJSError(runtime_.TypeErrorPrototype, code, message);
}

napi_status NodeApiEnvironment::throwJSRangeError(
    const char *code,
    const char *message) noexcept {
  return throwJSError(runtime_.RangeErrorPrototype, code, message);
}

napi_status NodeApiEnvironment::setJSErrorCode(
    vm::Handle<vm::JSError> error,
    napi_value code,
    const char *codeCString) noexcept {
  if (code || codeCString) {
    if (code) {
      CHECK_STRING_ARG(code);
    } else {
      CHECK_NAPI(createStringUTF8(codeCString, &code));
    }
    return setPredefinedProperty(error, NodeApiPredefined::code, code, nullptr);
  }
  return napi_ok;
}

//-----------------------------------------------------------------------------
// Methods to support catching JS exceptions
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::isJSErrorPending(bool *result) noexcept {
  return setResult(!thrownJSError_.isEmpty(), result);
}

napi_status NodeApiEnvironment::checkPendingJSError() noexcept {
  RETURN_STATUS_IF_FALSE(thrownJSError_.isEmpty(), napi_pending_exception);
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::getAndClearPendingJSError(
    napi_value *result) noexcept {
  if (thrownJSError_.isEmpty()) {
    return getUndefined(result);
  }
  return setResult(std::exchange(thrownJSError_, EmptyHermesValue), result);
}

napi_status NodeApiEnvironment::checkJSErrorStatus(
    vm::ExecutionStatus hermesStatus,
    napi_status status) noexcept {
  if (LLVM_LIKELY(hermesStatus != vm::ExecutionStatus::EXCEPTION)) {
    return napi_ok;
  }

  thrownJSError_ = runtime_.getThrownValue();
  runtime_.clearThrownValue();
  return status;
}

template <class T>
napi_status NodeApiEnvironment::checkJSErrorStatus(
    const vm::CallResult<T> &callResult,
    napi_status status) noexcept {
  return checkJSErrorStatus(callResult.getStatus(), status);
}

//-----------------------------------------------------------------------------
// Getters for common singletons
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::getGlobal(napi_value *result) noexcept {
  return setPredefinedResult(
      runtime_.getGlobal().unsafeGetPinnedHermesValue(), result);
}

napi_status NodeApiEnvironment::getUndefined(napi_value *result) noexcept {
  return setPredefinedResult(
      runtime_.getUndefinedValue().unsafeGetPinnedHermesValue(), result);
}

const vm::PinnedHermesValue &NodeApiEnvironment::getUndefined() noexcept {
  return *runtime_.getUndefinedValue().unsafeGetPinnedHermesValue();
}

napi_status NodeApiEnvironment::getNull(napi_value *result) noexcept {
  return setPredefinedResult(
      runtime_.getNullValue().unsafeGetPinnedHermesValue(), result);
}

//-----------------------------------------------------------------------------
// Method to get value type
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::typeOf(
    napi_value value,
    napi_valuetype *result) noexcept {
  CHECK_ARG(value);
  CHECK_ARG(result);

  const vm::PinnedHermesValue *hv = phv(value);

  if (hv->isNumber()) {
    *result = napi_number;
  } else if (hv->isString()) {
    *result = napi_string;
  } else if (hv->isObject()) {
    if (vm::vmisa<vm::Callable>(*hv)) {
      *result = napi_function;
    } else if (getExternalObjectValue(*hv)) {
      *result = napi_external;
    } else {
      *result = napi_object;
    }
  } else if (hv->isBool()) {
    *result = napi_boolean;
  } else if (hv->isUndefined() || hv->isEmpty()) {
    *result = napi_undefined;
  } else if (hv->isSymbol()) {
    *result = napi_symbol;
  } else if (hv->isNull()) {
    *result = napi_null;
  } else if (hv->isBigInt()) {
    *result = napi_bigint;
  } else {
    // Should not get here unless Hermes has added some new kind of value.
    return ERROR_STATUS(napi_invalid_arg, "Unknown value type");
  }

  return clearLastNativeError();
}

//-----------------------------------------------------------------------------
// Methods to work with Booleans
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::getBoolean(
    bool value,
    napi_value *result) noexcept {
  return setPredefinedResult(
      runtime_.getBoolValue(value).unsafeGetPinnedHermesValue(), result);
}

napi_status NodeApiEnvironment::getBooleanValue(
    napi_value value,
    bool *result) noexcept {
  CHECK_ARG(value);
  CHECK_ARG(result);
  RETURN_STATUS_IF_FALSE(phv(value)->isBool(), napi_boolean_expected);
  return setResult(phv(value)->getBool(), result);
}

//-----------------------------------------------------------------------------
// Methods to work with Numbers
//-----------------------------------------------------------------------------

template <class T, std::enable_if_t<std::is_arithmetic_v<T>, bool>>
napi_status NodeApiEnvironment::createNumber(
    T value,
    napi_value *result) noexcept {
  return setResult(
      vm::HermesValue::encodeUntrustedNumberValue(static_cast<double>(value)),
      result);
}

napi_status NodeApiEnvironment::getNumberValue(
    napi_value value,
    double *result) noexcept {
  CHECK_ARG(value);
  CHECK_ARG(result);
  RETURN_STATUS_IF_FALSE(phv(value)->isNumber(), napi_number_expected);
  return setResult(phv(value)->getDouble(), result);
}

napi_status NodeApiEnvironment::getNumberValue(
    napi_value value,
    int32_t *result) noexcept {
  CHECK_ARG(value);
  CHECK_ARG(result);
  RETURN_STATUS_IF_FALSE(phv(value)->isNumber(), napi_number_expected);
  return setResult(
      NodeApiDoubleConversion::toInt32(phv(value)->getDouble()), result);
}

napi_status NodeApiEnvironment::getNumberValue(
    napi_value value,
    uint32_t *result) noexcept {
  CHECK_ARG(value);
  CHECK_ARG(result);
  RETURN_STATUS_IF_FALSE(phv(value)->isNumber(), napi_number_expected);
  return setResult(
      NodeApiDoubleConversion::toUint32(phv(value)->getDouble()), result);
}

napi_status NodeApiEnvironment::getNumberValue(
    napi_value value,
    int64_t *result) noexcept {
  CHECK_ARG(value);
  CHECK_ARG(result);
  RETURN_STATUS_IF_FALSE(phv(value)->isNumber(), napi_number_expected);
  return setResult(
      NodeApiDoubleConversion::toInt64(phv(value)->getDouble()), result);
}

//-----------------------------------------------------------------------------
// Methods to work with Strings
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::createStringASCII(
    const char *str,
    size_t length,
    napi_value *result) noexcept {
  return setResult(
      vm::StringPrimitive::createEfficient(
          runtime_, llvh::makeArrayRef(str, length)),
      result);
}

napi_status NodeApiEnvironment::createStringLatin1(
    const char *str,
    size_t length,
    napi_value *result) noexcept {
  NodeApiHandleScope scope{*this, result};
  CHECK_ARG(str);
  if (length == NAPI_AUTO_LENGTH) {
    length = std::char_traits<char>::length(str);
  }
  RETURN_STATUS_IF_FALSE(
      length <= static_cast<size_t>(std::numeric_limits<int32_t>::max()),
      napi_invalid_arg);

  if (isAllASCII(str, str + length)) {
    return scope.setResult(createStringASCII(str, length, result));
  }

  // Latin1 has the same codes as Unicode.
  // We just need to expand char to char16_t.
  std::u16string u16str(length, u'\0');
  // Cast to unsigned to avoid signed value expansion to 16 bit.
  const uint8_t *ustr = reinterpret_cast<const uint8_t *>(str);
  std::copy(ustr, ustr + length, &u16str[0]);

  return scope.setResult(
      vm::StringPrimitive::createEfficient(runtime_, std::move(u16str)));
}

napi_status NodeApiEnvironment::createStringUTF8(
    const char *str,
    size_t length,
    napi_value *result) noexcept {
  NodeApiHandleScope scope{*this, result};
  CHECK_ARG(str);
  if (length == NAPI_AUTO_LENGTH) {
    length = std::char_traits<char>::length(str);
  }
  RETURN_STATUS_IF_FALSE(
      length <= static_cast<size_t>(std::numeric_limits<int32_t>::max()),
      napi_invalid_arg);

  if (isAllASCII(str, str + length)) {
    return scope.setResult(createStringASCII(str, length, result));
  }

  std::u16string u16str;
  CHECK_NAPI(convertUTF8ToUTF16(str, length, u16str));
  return scope.setResult(
      vm::StringPrimitive::createEfficient(runtime_, std::move(u16str)));
}

napi_status NodeApiEnvironment::createStringUTF8(
    const char *str,
    napi_value *result) noexcept {
  return createStringUTF8(str, NAPI_AUTO_LENGTH, result);
}

napi_status NodeApiEnvironment::createStringUTF16(
    const char16_t *str,
    size_t length,
    napi_value *result) noexcept {
  NodeApiHandleScope scope{*this, result};
  CHECK_ARG(str);
  if (length == NAPI_AUTO_LENGTH) {
    length = std::char_traits<char16_t>::length(str);
  }
  RETURN_STATUS_IF_FALSE(
      length <= static_cast<size_t>(std::numeric_limits<int32_t>::max()),
      napi_invalid_arg);

  return scope.setResult(vm::StringPrimitive::createEfficient(
      runtime_, llvh::makeArrayRef(str, length)));
}

// Copies a JavaScript string into a LATIN-1 string buffer. The result is the
// number of bytes (excluding the null terminator) copied into buf.
// A sufficient buffer size should be greater than the length of string,
// reserving space for null terminator.
// If bufSize is insufficient, the string will be truncated and null terminated.
// If buf is nullptr, this method returns the length of the string (in bytes)
// via the result parameter.
// The result argument is optional unless buf is nullptr.
napi_status NodeApiEnvironment::getStringValueLatin1(
    napi_value value,
    char *buf,
    size_t bufSize,
    size_t *result) noexcept {
  NodeApiHandleScope scope{*this};
  CHECK_STRING_ARG(value);
  vm::StringView view = vm::StringPrimitive::createStringView(
      runtime_, makeHandle<vm::StringPrimitive>(value));

  if (buf == nullptr) {
    return setResult(view.length(), result);
  } else if (bufSize != 0) {
    size_t copied = std::min(bufSize - 1, view.length());
    for (auto cur = view.begin(), end = view.begin() + copied; cur < end;
         ++cur) {
      *buf++ = static_cast<char>(*cur);
    }
    *buf = '\0';
    return setOptionalResult(std::move(copied), result);
  } else {
    return setOptionalResult(static_cast<size_t>(0), result);
  }
}

// Copies a JavaScript string into a UTF-8 string buffer. The result is the
// number of bytes (excluding the null terminator) copied into buf.
// A sufficient buffer size should be greater than the length of string,
// reserving space for null terminator.
// If bufSize is insufficient, the string will be truncated and null terminated.
// If buf is nullptr, this method returns the length of the string (in bytes)
// via the result parameter.
// The result argument is optional unless buf is nullptr.
napi_status NodeApiEnvironment::getStringValueUTF8(
    napi_value value,
    char *buf,
    size_t bufSize,
    size_t *result) noexcept {
  NodeApiHandleScope scope{*this};
  CHECK_STRING_ARG(value);
  vm::StringView view = vm::StringPrimitive::createStringView(
      runtime_, makeHandle<vm::StringPrimitive>(value));

  if (buf == nullptr) {
    return setResult(
        view.isASCII() || view.length() == 0
            ? view.length()
            : utf8LengthWithReplacements(
                  vm::UTF16Ref(view.castToChar16Ptr(), view.length())),
        result);
  } else if (bufSize != 0) {
    size_t copied = view.length() > 0 ? view.isASCII()
            ? copyASCIIToUTF8(
                  vm::ASCIIRef(view.castToCharPtr(), view.length()),
                  buf,
                  bufSize - 1)
            : convertUTF16ToUTF8WithReplacements(
                  vm::UTF16Ref(view.castToChar16Ptr(), view.length()),
                  buf,
                  bufSize - 1)
                                      : 0;
    buf[copied] = '\0';
    return setOptionalResult(std::move(copied), result);
  } else {
    return setOptionalResult(static_cast<size_t>(0), result);
  }
}

// Copies a JavaScript string into a UTF-16 string buffer. The result is the
// number of 2-byte code units (excluding the null terminator) copied into buf.
// A sufficient buffer size should be greater than the length of string,
// reserving space for null terminator.
// If bufSize is insufficient, the string will be truncated and null terminated.
// If buf is nullptr, this method returns the length of the string (in 2-byte
// code units) via the result parameter.
// The result argument is optional unless buf is nullptr.
napi_status NodeApiEnvironment::getStringValueUTF16(
    napi_value value,
    char16_t *buf,
    size_t bufSize,
    size_t *result) noexcept {
  NodeApiHandleScope scope{*this};
  CHECK_STRING_ARG(value);
  vm::StringView view = vm::StringPrimitive::createStringView(
      runtime_, makeHandle<vm::StringPrimitive>(value));

  if (buf == nullptr) {
    return setResult(view.length(), result);
  } else if (bufSize != 0) {
    size_t copied = std::min(bufSize - 1, view.length());
    std::copy(view.begin(), view.begin() + copied, buf);
    buf[copied] = '\0';
    return setOptionalResult(std::move(copied), result);
  } else {
    return setOptionalResult(static_cast<size_t>(0), result);
  }
}

napi_status NodeApiEnvironment::convertUTF8ToUTF16(
    const char *utf8,
    size_t length,
    std::u16string &out) noexcept {
  // length is the number of input bytes
  out.resize(length);
  const llvh::UTF8 *sourceStart = reinterpret_cast<const llvh::UTF8 *>(utf8);
  const llvh::UTF8 *sourceEnd = sourceStart + length;
  llvh::UTF16 *targetStart = reinterpret_cast<llvh::UTF16 *>(&out[0]);
  llvh::UTF16 *targetEnd = targetStart + out.size();
  llvh::ConversionResult convRes = ConvertUTF8toUTF16(
      &sourceStart,
      sourceEnd,
      &targetStart,
      targetEnd,
      llvh::lenientConversion);
  RETURN_STATUS_IF_FALSE_WITH_MESSAGE(
      convRes != llvh::ConversionResult::targetExhausted,
      napi_generic_failure,
      "not enough space allocated for UTF16 conversion");
  out.resize(reinterpret_cast<char16_t *>(targetStart) - &out[0]);
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::getUniqueSymbolID(
    const char *utf8,
    size_t length,
    vm::MutableHandle<vm::SymbolID> *result) noexcept {
  napi_value strValue;
  CHECK_NAPI(createStringUTF8(utf8, length, &strValue));
  return getUniqueSymbolID(strValue, result);
}

napi_status NodeApiEnvironment::getUniqueSymbolID(
    napi_value strValue,
    vm::MutableHandle<vm::SymbolID> *result) noexcept {
  CHECK_STRING_ARG(strValue);
  return setResult(
      vm::stringToSymbolID(
          runtime_, vm::createPseudoHandle(phv(strValue)->getString())),
      result);
}

//-----------------------------------------------------------------------------
// Methods to work with Symbols
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::createSymbol(
    napi_value description,
    napi_value *result) noexcept {
  NodeApiHandleScope scope{*this, result};
  vm::MutableHandle<vm::StringPrimitive> descString{runtime_};
  if (description != nullptr) {
    CHECK_STRING_ARG(description);
    descString = phv(description)->getString();
  } else {
    // If description is undefined, the descString will eventually be "".
    descString = runtime_.getPredefinedString(vm::Predefined::emptyString);
  }
  return scope.setResult(runtime_.getIdentifierTable().createNotUniquedSymbol(
      runtime_, descString));
}

//-----------------------------------------------------------------------------
// Methods to work with BigInt
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::createBigIntFromInt64(
    int64_t value,
    napi_value *result) {
  NodeApiHandleScope scope{*this, result};
  return scope.setResult(vm::BigIntPrimitive::fromSigned(runtime_, value));
}

napi_status NodeApiEnvironment::createBigIntFromUint64(
    uint64_t value,
    napi_value *result) {
  NodeApiHandleScope scope{*this, result};
  return scope.setResult(vm::BigIntPrimitive::fromUnsigned(runtime_, value));
}

napi_status NodeApiEnvironment::createBigIntFromWords(
    int signBit,
    size_t wordCount,
    const uint64_t *words,
    napi_value *result) {
  NodeApiHandleScope scope{*this, result};
  CHECK_ARG(words);
  RETURN_STATUS_IF_FALSE(wordCount <= INT_MAX, napi_invalid_arg);

  if (signBit) {
    // Use 2's complement algorithm to represent negative numbers.
    llvh::SmallVector<uint64_t, 16> negativeValue{words, words + wordCount};

    // a. flip all bits
    for (uint64_t &entry : negativeValue) {
      entry = ~entry;
    }
    // b. add 1
    for (size_t i = 0; i < negativeValue.size(); ++i) {
      if (++negativeValue[i] >= 1) {
        break; // No need to carry so exit early.
      }
    }
    words = negativeValue.data();
  }

  const uint8_t *ptr = reinterpret_cast<const uint8_t *>(words);
  const uint32_t size = static_cast<uint32_t>(wordCount * sizeof(uint64_t));
  return scope.setResult(
      vm::BigIntPrimitive::fromBytes(runtime_, llvh::makeArrayRef(ptr, size)));
}

napi_status NodeApiEnvironment::getBigIntValueInt64(
    napi_value value,
    int64_t *result,
    bool *lossless) {
  CHECK_ARG(value);
  CHECK_ARG(result);
  CHECK_ARG(lossless);
  RETURN_STATUS_IF_FALSE(phv(value)->isBigInt(), napi_bigint_expected);
  vm::BigIntPrimitive *bigInt = phv(value)->getBigInt();
  *lossless =
      bigInt->isTruncationToSingleDigitLossless(/*signedTruncation:*/ true);
  *result = static_cast<int64_t>(bigInt->truncateToSingleDigit());
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::getBigIntValueUint64(
    napi_value value,
    uint64_t *result,
    bool *lossless) {
  CHECK_ARG(value);
  CHECK_ARG(result);
  CHECK_ARG(lossless);
  RETURN_STATUS_IF_FALSE(phv(value)->isBigInt(), napi_bigint_expected);
  vm::BigIntPrimitive *bigInt = phv(value)->getBigInt();
  *lossless =
      bigInt->isTruncationToSingleDigitLossless(/*signedTruncation:*/ false);
  *result = bigInt->truncateToSingleDigit();
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::getBigIntValueWords(
    napi_value value,
    int *signBit,
    size_t *wordCount,
    uint64_t *words) {
  CHECK_ARG(value);
  CHECK_ARG(wordCount);
  RETURN_STATUS_IF_FALSE(phv(value)->isBigInt(), napi_bigint_expected);
  vm::BigIntPrimitive *bigInt = phv(value)->getBigInt();

  if (signBit == nullptr && words == nullptr) {
    *wordCount = bigInt->getDigits().size();
  } else {
    CHECK_ARG(signBit);
    CHECK_ARG(words);
    llvh::ArrayRef<uint64_t> digits = bigInt->getDigits();
    *wordCount = std::min(*wordCount, digits.size());
    std::memcpy(words, digits.begin(), *wordCount * sizeof(uint64_t));
    *signBit = bigInt->sign() ? 1 : 0;
    if (*signBit) {
      // negate negative numbers, and then add a "-" to the output.
      // a. flip all bits
      for (size_t i = 0; i < *wordCount; ++i) {
        words[i] = ~words[i];
      }
      // b. add 1
      for (size_t i = 0; i < *wordCount; ++i) {
        if (++words[i] >= 1) {
          break; // No need to carry so exit early.
        }
      }
    }
  }

  return clearLastNativeError();
}

//-----------------------------------------------------------------------------
// Methods to coerce values using JS coercion rules
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::coerceToBoolean(
    napi_value value,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};
  CHECK_ARG(value);
  return scope.setResult(vm::toBoolean(*phv(value)));
}

napi_status NodeApiEnvironment::coerceToNumber(
    napi_value value,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};
  CHECK_ARG(value);
  return scope.setResult(vm::toNumber_RJS(runtime_, makeHandle(value)));
}

napi_status NodeApiEnvironment::coerceToObject(
    napi_value value,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};
  CHECK_ARG(value);
  return scope.setResult(vm::toObject(runtime_, makeHandle(value)));
}

napi_status NodeApiEnvironment::coerceToString(
    napi_value value,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};
  CHECK_ARG(value);
  return scope.setResult(vm::toString_RJS(runtime_, makeHandle(value)));
}

//-----------------------------------------------------------------------------
// Methods to work with Objects
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::createObject(napi_value *result) noexcept {
  NodeApiHandleScope scope{*this, result};
  return scope.setResult(vm::JSObject::create(runtime_));
}

napi_status NodeApiEnvironment::getPrototype(
    napi_value object,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};
  napi_value objValue{};
  CHECK_NAPI(coerceToObject(object, &objValue));
  return scope.setResult(vm::JSObject::getPrototypeOf(
      vm::createPseudoHandle(getObjectUnsafe(objValue)), runtime_));
}

napi_status NodeApiEnvironment::getForInPropertyNames(
    napi_value object,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};
  napi_value objValue;
  CHECK_NAPI(coerceToObject(object, &objValue));
  return scope.setResult(
      getForInPropertyNames(objValue, napi_key_numbers_to_strings, result));
}

napi_status NodeApiEnvironment::getForInPropertyNames(
    napi_value object,
    napi_key_conversion keyConversion,
    napi_value *result) noexcept {
  // Hermes optimizes retrieving property names for the 'for..in' implementation
  // by caching its results. This function takes the advantage from using it.
  uint32_t beginIndex;
  uint32_t endIndex;
  vm::CallResult<vm::Handle<vm::BigStorage>> keyStorage =
      vm::getForInPropertyNames(
          runtime_, makeHandle<vm::JSObject>(object), beginIndex, endIndex);
  CHECK_NAPI(checkJSErrorStatus(keyStorage));
  return convertKeyStorageToArray(
      *keyStorage, beginIndex, endIndex - beginIndex, keyConversion, result);
}

napi_status NodeApiEnvironment::getAllPropertyNames(
    napi_value object,
    napi_key_collection_mode keyMode,
    napi_key_filter keyFilter,
    napi_key_conversion keyConversion,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};

  napi_value objValue;
  CHECK_NAPI(coerceToObject(object, &objValue));
  RETURN_STATUS_IF_FALSE(
      isInEnumRange(keyMode, napi_key_include_prototypes, napi_key_own_only),
      napi_invalid_arg);
  RETURN_STATUS_IF_FALSE(
      isInEnumRange(
          keyConversion, napi_key_keep_numbers, napi_key_numbers_to_strings),
      napi_invalid_arg);

  // We can use optimized code if object has no parent.
  bool hasParent;
  {
    napi_value parent;
    CHECK_NAPI(getPrototype(object, &parent));
    hasParent = phv(parent)->isObject();
  }

  // The fast path used for the 'for..in' implementation.
  if (keyFilter == (napi_key_enumerable | napi_key_skip_symbols) &&
      (keyMode == napi_key_include_prototypes || !hasParent)) {
    return scope.setResult(
        getForInPropertyNames(objValue, keyConversion, result));
  }

  // Flags to request own keys
  // Include non-enumerable for proper shadow checks.
  vm::OwnKeysFlags ownKeyFlags =
      vm::OwnKeysFlags()
          .setIncludeNonSymbols((keyFilter & napi_key_skip_strings) == 0)
          .setIncludeSymbols((keyFilter & napi_key_skip_symbols) == 0)
          .plusIncludeNonEnumerable();

  // Use the simple path for own properties without extra filters.
  if ((keyMode == napi_key_own_only || !hasParent) &&
      (keyFilter & (napi_key_writable | napi_key_configurable)) == 0) {
    vm::CallResult<vm::Handle<vm::JSArray>> ownKeysRes =
        vm::JSObject::getOwnPropertyKeys(
            makeHandle<vm::JSObject>(objValue),
            runtime_,
            ownKeyFlags.setIncludeNonEnumerable(
                (keyFilter & napi_key_enumerable) == 0));
    CHECK_NAPI(checkJSErrorStatus(ownKeysRes));
    if (keyConversion == napi_key_numbers_to_strings) {
      CHECK_NAPI(convertToStringKeys(*ownKeysRes));
    }
    return scope.setResult(std::move(*ownKeysRes));
  }

  // Collect all properties into the keyStorage.
  vm::CallResult<vm::MutableHandle<vm::BigStorage>> keyStorageRes =
      makeMutableHandle(vm::BigStorage::create(runtime_, 16));
  CHECK_NAPI(checkJSErrorStatus(keyStorageRes));
  uint32_t size{0};

  // Make sure that we do not include into the result properties that were
  // shadowed by the derived objects.
  bool useShadowTracking = keyMode == napi_key_include_prototypes && hasParent;
  NodeApiOrderedSet<uint32_t> shadowIndexes;
  NodeApiOrderedSet<vm::HermesValue> shadowStrings(
      *this, [](const vm::HermesValue &item1, const vm::HermesValue &item2) {
        return item1.getString()->compare(item2.getString());
      });
  NodeApiOrderedSet<vm::HermesValue> shadowSymbols(
      *this, [](const vm::HermesValue &item1, const vm::HermesValue &item2) {
        vm::SymbolID::RawType rawItem1 = item1.getSymbol().unsafeGetRaw();
        vm::SymbolID::RawType rawItem2 = item2.getSymbol().unsafeGetRaw();
        return rawItem1 < rawItem2 ? -1 : rawItem1 > rawItem2 ? 1 : 0;
      });

  // Should we apply the filter?
  bool useFilter =
      (keyFilter &
       (napi_key_writable | napi_key_enumerable | napi_key_configurable)) != 0;

  // Keep the mutable variables outside of loop for efficiency
  vm::MutableHandle<vm::JSObject> currentObj(
      runtime_, getObjectUnsafe(objValue));
  vm::MutableHandle<> prop{runtime_};
  OptValue<uint32_t> propIndexOpt;
  vm::MutableHandle<vm::StringPrimitive> propString{runtime_};

  while (currentObj.get()) {
    vm::GCScope gcScope{runtime_};

    vm::CallResult<vm::Handle<vm::JSArray>> props =
        vm::JSObject::getOwnPropertyKeys(currentObj, runtime_, ownKeyFlags);
    CHECK_NAPI(checkJSErrorStatus(props));

    vm::GCScope::Marker marker = gcScope.createMarker();
    for (uint32_t i = 0, end = props.getValue()->getEndIndex(); i < end; ++i) {
      gcScope.flushToMarker(marker);
      prop = props.getValue()->at(runtime_, i).unboxToHV(runtime_);

      // Do not add a property if it is overriden in the derived object.
      if (useShadowTracking) {
        if (prop->isString()) {
          propString = vm::Handle<vm::StringPrimitive>::vmcast(prop);
          // See if the property name is an index
          propIndexOpt = vm::toArrayIndex(
              vm::StringPrimitive::createStringView(runtime_, propString));
          if (propIndexOpt) {
            if (!shadowIndexes.insert(propIndexOpt.getValue())) {
              continue;
            }
          } else {
            if (!shadowStrings.insert(prop.getHermesValue())) {
              continue;
            }
          }
        } else if (prop->isNumber()) {
          propIndexOpt = doubleToArrayIndex(prop->getNumber());
          assert(propIndexOpt && "Invalid property index");
          if (!shadowIndexes.insert(propIndexOpt.getValue())) {
            continue;
          }
        } else if (prop->isSymbol()) {
          if (!shadowSymbols.insert(prop.getHermesValue())) {
            continue;
          }
        }
      }

      // Apply filter for the property descriptor flags
      if (useFilter) {
        vm::MutableHandle<vm::SymbolID> tmpSymbolStorage{runtime_};
        vm::ComputedPropertyDescriptor desc;
        vm::CallResult<bool> hasDescriptorRes =
            vm::JSObject::getOwnComputedPrimitiveDescriptor(
                currentObj,
                runtime_,
                prop,
                vm::JSObject::IgnoreProxy::No,
                tmpSymbolStorage,
                desc);
        CHECK_NAPI(checkJSErrorStatus(hasDescriptorRes));
        if (*hasDescriptorRes) {
          if ((keyFilter & napi_key_writable) != 0 && !desc.flags.writable) {
            continue;
          }
          if ((keyFilter & napi_key_enumerable) != 0 &&
              !desc.flags.enumerable) {
            continue;
          }
          if ((keyFilter & napi_key_configurable) != 0 &&
              !desc.flags.configurable) {
            continue;
          }
        }
      }

      CHECK_NAPI(checkJSErrorStatus(
          vm::BigStorage::push_back(*keyStorageRes, runtime_, prop)));
      ++size;
    }

    // Continue to follow the prototype chain.
    vm::CallResult<vm::PseudoHandle<vm::JSObject>> parentRes =
        vm::JSObject::getPrototypeOf(currentObj, runtime_);
    CHECK_NAPI(checkJSErrorStatus(parentRes));
    currentObj = std::move(*parentRes);
  }

  return scope.setResult(
      convertKeyStorageToArray(*keyStorageRes, 0, size, keyConversion, result));
}

napi_status NodeApiEnvironment::convertKeyStorageToArray(
    vm::Handle<vm::BigStorage> keyStorage,
    uint32_t startIndex,
    uint32_t length,
    napi_key_conversion keyConversion,
    napi_value *result) noexcept {
  vm::CallResult<vm::Handle<vm::JSArray>> res =
      vm::JSArray::create(runtime_, length, length);
  CHECK_NAPI(checkJSErrorStatus(res));
  vm::Handle<vm::JSArray> array = *res;
  if (keyConversion == napi_key_numbers_to_strings) {
    vm::GCScopeMarkerRAII marker{runtime_};
    vm::MutableHandle<> key{runtime_};
    for (size_t i = 0; i < length; ++i) {
      key = makeHandle(keyStorage->at(runtime_, startIndex + i));
      if (key->isNumber()) {
        CHECK_NAPI(convertIndexToString(key->getNumber(), &key));
      }
      vm::JSArray::setElementAt(array, runtime_, i, key);
      marker.flush();
    }
  } else {
    vm::JSArray::setStorageEndIndex(array, runtime_, length);
    vm::NoAllocScope noAlloc{runtime_};
    vm::JSArray *arrPtr = array.get();
    for (uint32_t i = 0; i < length; ++i) {
      vm::JSArray::unsafeSetExistingElementAt(
          arrPtr,
          runtime_,
          i,
          vm::SmallHermesValue::encodeHermesValue(
              keyStorage->at(runtime_, startIndex + i), runtime_));
    }
  }
  return setResult(array.getHermesValue(), result);
}

napi_status NodeApiEnvironment::convertToStringKeys(
    vm::Handle<vm::JSArray> array) noexcept {
  vm::GCScopeMarkerRAII marker{runtime_};
  size_t length = vm::JSArray::getLength(array.get(), runtime_);
  for (size_t i = 0; i < length; ++i) {
    vm::HermesValue key = array->at(runtime_, i).unboxToHV(runtime_);
    if (LLVM_UNLIKELY(key.isNumber())) {
      vm::MutableHandle<> strKey{runtime_};
      CHECK_NAPI(convertIndexToString(key.getNumber(), &strKey));
      vm::JSArray::setElementAt(array, runtime_, i, strKey);
      marker.flush();
    }
  }
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::convertIndexToString(
    double value,
    vm::MutableHandle<> *result) noexcept {
  OptValue<uint32_t> index = doubleToArrayIndex(value);
  RETURN_STATUS_IF_FALSE_WITH_MESSAGE(
      index.hasValue(), napi_generic_failure, "Index property is out of range");
  return NodeApiStringBuilder(*index).makeHVString(*this, result);
}

napi_status NodeApiEnvironment::hasProperty(
    napi_value object,
    napi_value key,
    bool *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};
  CHECK_ARG(key);
  napi_value objValue;
  CHECK_NAPI(coerceToObject(object, &objValue));
  return hasComputedProperty(objValue, key, result);
}

napi_status NodeApiEnvironment::getProperty(
    napi_value object,
    napi_value key,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};
  CHECK_ARG(key);
  napi_value objValue;
  CHECK_NAPI(coerceToObject(object, &objValue));
  return scope.setResult(getComputedProperty(objValue, key, result));
}

napi_status NodeApiEnvironment::setProperty(
    napi_value object,
    napi_value key,
    napi_value value) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};
  CHECK_ARG(key);
  CHECK_ARG(value);
  napi_value objValue;
  CHECK_NAPI(coerceToObject(object, &objValue));
  return setComputedProperty(objValue, key, value);
}

napi_status NodeApiEnvironment::deleteProperty(
    napi_value object,
    napi_value key,
    bool *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};
  CHECK_ARG(key);
  napi_value objValue;
  CHECK_NAPI(coerceToObject(object, &objValue));
  return deleteComputedProperty(objValue, key, result);
}

napi_status NodeApiEnvironment::hasOwnProperty(
    napi_value object,
    napi_value key,
    bool *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  CHECK_ARG(key);
  CHECK_ARG(result);
  RETURN_STATUS_IF_FALSE(
      phv(key)->isString() || phv(key)->isSymbol(), napi_name_expected);

  NodeApiHandleScope scope{*this};
  napi_value objValue;
  CHECK_NAPI(coerceToObject(object, &objValue));
  vm::MutableHandle<vm::SymbolID> tmpSymbolStorage{runtime_};
  vm::ComputedPropertyDescriptor desc;
  return getOwnComputedPropertyDescriptor(
      objValue, key, tmpSymbolStorage, desc, result);
}

napi_status NodeApiEnvironment::hasNamedProperty(
    napi_value object,
    const char *utf8Name,
    bool *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};
  CHECK_ARG(utf8Name);
  napi_value objValue, name;
  CHECK_NAPI(coerceToObject(object, &objValue));
  CHECK_NAPI(createStringUTF8(utf8Name, &name));
  return hasComputedProperty(objValue, name, result);
}

napi_status NodeApiEnvironment::getNamedProperty(
    napi_value object,
    const char *utf8Name,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};
  CHECK_ARG(utf8Name);
  napi_value objValue, name;
  CHECK_NAPI(coerceToObject(object, &objValue));
  CHECK_NAPI(createStringUTF8(utf8Name, &name));
  return scope.setResult(getComputedProperty(objValue, name, result));
}

napi_status NodeApiEnvironment::setNamedProperty(
    napi_value object,
    const char *utf8Name,
    napi_value value) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};
  CHECK_ARG(utf8Name);
  CHECK_ARG(value);
  napi_value objValue, name;
  CHECK_NAPI(coerceToObject(object, &objValue));
  CHECK_NAPI(createStringUTF8(utf8Name, &name));
  return setComputedProperty(objValue, name, value);
}

napi_status NodeApiEnvironment::defineProperties(
    napi_value object,
    size_t propertyCount,
    const napi_property_descriptor *properties) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};
  CHECK_OBJECT_ARG(object);
  if (propertyCount > 0) {
    CHECK_ARG(properties);
  }

  napi_value objValue;
  CHECK_NAPI(coerceToObject(object, &objValue));
  vm::Handle<vm::JSObject> objHandle = makeHandle<vm::JSObject>(objValue);
  vm::MutableHandle<vm::SymbolID> name{runtime_};
  vm::GCScopeMarkerRAII marker{runtime_};
  for (size_t i = 0; i < propertyCount; ++i) {
    marker.flush();
    const napi_property_descriptor *p = &properties[i];
    CHECK_NAPI(symbolIDFromPropertyDescriptor(p, &name));

    vm::DefinePropertyFlags dpFlags{};
    dpFlags.setEnumerable = 1;
    dpFlags.setConfigurable = 1;
    dpFlags.enumerable = (p->attributes & napi_enumerable) == 0 ? 0 : 1;
    dpFlags.configurable = (p->attributes & napi_configurable) == 0 ? 0 : 1;

    if ((p->getter != nullptr) || (p->setter != nullptr)) {
      vm::MutableHandle<vm::Callable> localGetter{runtime_};
      vm::MutableHandle<vm::Callable> localSetter{runtime_};

      if (p->getter != nullptr) {
        dpFlags.setGetter = 1;
        CHECK_NAPI(createFunction(
            vm::Predefined::getSymbolID(vm::Predefined::get),
            p->getter,
            p->data,
            &localGetter));
      }
      if (p->setter != nullptr) {
        dpFlags.setSetter = 1;
        CHECK_NAPI(createFunction(
            vm::Predefined::getSymbolID(vm::Predefined::set),
            p->setter,
            p->data,
            &localSetter));
      }

      vm::CallResult<vm::HermesValue> propRes =
          vm::PropertyAccessor::create(runtime_, localGetter, localSetter);
      CHECK_NAPI(checkJSErrorStatus(propRes));
      CHECK_NAPI(defineOwnProperty(
          objHandle, *name, dpFlags, makeHandle(*propRes), nullptr));
    } else {
      dpFlags.setValue = 1;
      dpFlags.setWritable = 1;
      dpFlags.writable = (p->attributes & napi_writable) == 0 ? 0 : 1;
      if (p->method != nullptr) {
        vm::MutableHandle<vm::Callable> method{runtime_};
        CHECK_NAPI(createFunction(name.get(), p->method, p->data, &method));
        CHECK_NAPI(
            defineOwnProperty(objHandle, *name, dpFlags, method, nullptr));
      } else {
        CHECK_NAPI(defineOwnProperty(
            objHandle, *name, dpFlags, makeHandle(p->value), nullptr));
      }
    }
  }

  return processFinalizerQueue();
}

napi_status NodeApiEnvironment::symbolIDFromPropertyDescriptor(
    const napi_property_descriptor *descriptor,
    vm::MutableHandle<vm::SymbolID> *result) noexcept {
  if (descriptor->utf8name != nullptr) {
    return getUniqueSymbolID(descriptor->utf8name, NAPI_AUTO_LENGTH, result);
  } else {
    RETURN_STATUS_IF_FALSE(descriptor->name != nullptr, napi_name_expected);
    const vm::PinnedHermesValue &name = *phv(descriptor->name);
    if (name.isString()) {
      return getUniqueSymbolID(descriptor->name, result);
    } else if (name.isSymbol()) {
      *result = name.getSymbol();
      return clearLastNativeError();
    } else {
      return ERROR_STATUS(
          napi_name_expected, "p->name must be String or Symbol");
    }
  }
}

napi_status NodeApiEnvironment::objectFreeze(napi_value object) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};
  napi_value objValue;
  CHECK_NAPI(coerceToObject(object, &objValue));
  return checkJSErrorStatus(
      vm::JSObject::freeze(makeHandle<vm::JSObject>(objValue), runtime_));
}

napi_status NodeApiEnvironment::objectSeal(napi_value object) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};
  napi_value objValue;
  CHECK_NAPI(coerceToObject(object, &objValue));
  return checkJSErrorStatus(
      vm::JSObject::seal(makeHandle<vm::JSObject>(objValue), runtime_));
}

//-----------------------------------------------------------------------------
// Methods to work with Arrays
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::createArray(
    size_t length,
    napi_value *result) noexcept {
  NodeApiHandleScope scope{*this, result};
  return scope.setResult(
      vm::JSArray::create(runtime_, /*capacity:*/ length, /*length:*/ length));
}

napi_status NodeApiEnvironment::isArray(
    napi_value value,
    bool *result) noexcept {
  CHECK_ARG(value);
  return setResult(vm::vmisa<vm::JSArray>(*phv(value)), result);
}

napi_status NodeApiEnvironment::getArrayLength(
    napi_value value,
    uint32_t *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};
  CHECK_ARG(value);
  RETURN_STATUS_IF_FALSE(
      vm::vmisa<vm::JSArray>(*phv(value)), napi_array_expected);
  napi_value res;
  CHECK_NAPI(getNamedProperty(
      value, vm::Predefined::getSymbolID(vm::Predefined::length), &res));
  RETURN_STATUS_IF_FALSE(phv(res)->isNumber(), napi_number_expected);
  return setResult(
      NodeApiDoubleConversion::toUint32(phv(res)->getDouble()), result);
}

napi_status NodeApiEnvironment::hasElement(
    napi_value object,
    uint32_t index,
    bool *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};
  napi_value objValue;
  CHECK_NAPI(coerceToObject(object, &objValue));
  return hasComputedProperty(objValue, index, result);
}

napi_status NodeApiEnvironment::getElement(
    napi_value object,
    uint32_t index,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};
  napi_value objValue;
  CHECK_NAPI(coerceToObject(object, &objValue));
  return scope.setResult(getComputedProperty(objValue, index, result));
}

napi_status NodeApiEnvironment::setElement(
    napi_value object,
    uint32_t index,
    napi_value value) noexcept {
  CHECK_NAPI(checkPendingJSError());
  CHECK_ARG(value);
  NodeApiHandleScope scope{*this};
  napi_value objValue;
  CHECK_NAPI(coerceToObject(object, &objValue));
  return setComputedProperty(objValue, index, value);
}

napi_status NodeApiEnvironment::deleteElement(
    napi_value object,
    uint32_t index,
    bool *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};
  napi_value objValue;
  CHECK_NAPI(coerceToObject(object, &objValue));
  return deleteComputedProperty(objValue, index, result);
}

//-----------------------------------------------------------------------------
// Methods to work with Functions
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::createFunction(
    const char *utf8Name,
    size_t length,
    napi_callback callback,
    void *callbackData,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};
  CHECK_ARG(callback);
  vm::MutableHandle<vm::SymbolID> nameSymbolID{runtime_};
  if (utf8Name != nullptr) {
    CHECK_NAPI(getUniqueSymbolID(utf8Name, length, &nameSymbolID));
  } else {
    nameSymbolID = getPredefinedSymbol(NodeApiPredefined::hostFunction);
  }
  vm::MutableHandle<vm::Callable> func{runtime_};
  CHECK_NAPI(createFunction(nameSymbolID.get(), callback, callbackData, &func));
  return scope.setResult(func.getHermesValue());
}

napi_status NodeApiEnvironment::createFunction(
    vm::SymbolID name,
    napi_callback callback,
    void *callbackData,
    vm::MutableHandle<vm::Callable> *result) noexcept {
  std::unique_ptr<NodeApiHostFunctionContext> context =
      std::make_unique<NodeApiHostFunctionContext>(
          *this, callback, callbackData);
  vm::CallResult<vm::HermesValue> funcRes =
      vm::FinalizableNativeFunction::createWithoutPrototype(
          runtime_,
          context.get(),
          &NodeApiHostFunctionContext::func,
          &NodeApiHostFunctionContext::finalize,
          name,
          /*paramCount:*/ 0);
  CHECK_NAPI(checkJSErrorStatus(funcRes));
  context.release(); // the context is now owned by the func.
  return setResult(makeHandle<vm::Callable>(*funcRes), result);
}

napi_status NodeApiEnvironment::callFunction(
    napi_value thisArg,
    napi_value func,
    size_t argCount,
    const napi_value *args,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};

  CHECK_ARG(thisArg);
  CHECK_ARG(func);
  if (argCount > 0) {
    CHECK_ARG(args);
  }
  RETURN_STATUS_IF_FALSE(vm::vmisa<vm::Callable>(*phv(func)), napi_invalid_arg);
  vm::Handle<vm::Callable> funcHandle = makeHandle<vm::Callable>(func);

  if (argCount >= std::numeric_limits<uint32_t>::max() ||
      !runtime_.checkAvailableStack(static_cast<uint32_t>(argCount))) {
    return GENERIC_FAILURE("Unable to call function: stack overflow");
  }

  vm::ScopedNativeCallFrame newFrame{
      runtime_,
      static_cast<uint32_t>(argCount),
      funcHandle.getHermesValue(),
      /*newTarget:*/ getUndefined(),
      *phv(thisArg)};
  if (LLVM_UNLIKELY(newFrame.overflowed())) {
    CHECK_NAPI(checkJSErrorStatus(runtime_.raiseStackOverflow(
        vm::Runtime::StackOverflowKind::NativeStack)));
  }

  for (uint32_t i = 0; i < argCount; ++i) {
    newFrame->getArgRef(static_cast<int32_t>(i)) = *phv(args[i]);
  }
  vm::CallResult<vm::PseudoHandle<>> callRes =
      vm::Callable::call(funcHandle, runtime_);
  CHECK_NAPI(checkJSErrorStatus(callRes, napi_pending_exception));

  if (result) {
    RETURN_FAILURE_IF_FALSE(!callRes->get().isEmpty());
    return scope.setResult(callRes->get());
  }
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::createNewInstance(
    napi_value constructor,
    size_t argCount,
    const napi_value *args,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};

  CHECK_ARG(constructor);
  if (argCount > 0) {
    CHECK_ARG(args);
  }

  RETURN_STATUS_IF_FALSE(
      vm::vmisa<vm::Callable>(*phv(constructor)), napi_invalid_arg);
  vm::Handle<vm::Callable> ctorHandle = makeHandle<vm::Callable>(constructor);

  if (argCount >= std::numeric_limits<uint32_t>::max() ||
      !runtime_.checkAvailableStack(static_cast<uint32_t>(argCount))) {
    return GENERIC_FAILURE("Unable to call constructor: stack overflow");
  }

  // We follow es5 13.2.2 [[Construct]] here. Below F == func.
  // 13.2.2.5:
  //    Let proto be the value of calling the [[Get]] internal property of
  //    F with argument "prototype"
  // 13.2.2.6:
  //    If Type(proto) is Object, set the [[Prototype]] internal property
  //    of obj to proto
  // 13.2.2.7:
  //    If Type(proto) is not Object, set the [[Prototype]] internal property
  //    of obj to the standard built-in Object prototype object as described
  //    in 15.2.4
  //
  // Note that 13.2.2.1-4 are also handled by the call to newObject.
  vm::CallResult<vm::PseudoHandle<vm::JSObject>> thisRes =
      vm::Callable::createThisForConstruct_RJS(ctorHandle, runtime_);
  CHECK_NAPI(checkJSErrorStatus(thisRes));
  // We need to capture this in case the ctor doesn't return an object,
  // we need to return this object.
  vm::Handle<vm::JSObject> thisHandle = makeHandle(std::move(*thisRes));

  // 13.2.2.8:
  //    Let result be the result of calling the [[Call]] internal property of
  //    F, providing obj as the this value and providing the argument list
  //    passed into [[Construct]] as args.
  //
  // For us result == res.

  vm::ScopedNativeCallFrame newFrame{
      runtime_,
      static_cast<uint32_t>(argCount),
      ctorHandle.getHermesValue(),
      ctorHandle.getHermesValue(),
      thisHandle.getHermesValue()};
  if (LLVM_UNLIKELY(newFrame.overflowed())) {
    CHECK_NAPI(checkJSErrorStatus(runtime_.raiseStackOverflow(
        vm::Runtime::StackOverflowKind::NativeStack)));
  }
  for (size_t i = 0; i < argCount; ++i) {
    newFrame->getArgRef(static_cast<int32_t>(i)) = *phv(args[i]);
  }
  // The last parameter indicates that this call should construct an object.
  vm::CallResult<vm::PseudoHandle<>> callRes =
      vm::Callable::call(ctorHandle, runtime_);
  CHECK_NAPI(checkJSErrorStatus(callRes, napi_pending_exception));

  // 13.2.2.9:
  //    If Type(result) is Object then return result
  // 13.2.2.10:
  //    Return obj
  vm::HermesValue resultValue = callRes->get();
  return scope.setResult(
      resultValue.isObject() ? std::move(resultValue)
                             : thisHandle.getHermesValue());
}

napi_status NodeApiEnvironment::isInstanceOf(
    napi_value object,
    napi_value constructor,
    bool *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};

  CHECK_ARG(object);
  CHECK_ARG(constructor);
  napi_value ctorValue;
  CHECK_NAPI(coerceToObject(constructor, &ctorValue));
  RETURN_STATUS_IF_FALSE(
      vm::vmisa<vm::Callable>(*phv(ctorValue)), napi_function_expected);
  return setResult(
      vm::instanceOfOperator_RJS(
          runtime_, makeHandle(object), makeHandle(constructor)),
      result);
}

template <class TLambda>
vm::ExecutionStatus NodeApiEnvironment::callIntoModule(
    TLambda &&call) noexcept {
  size_t openHandleScopesBefore = napiValueStackScopes_.size();
  clearLastNativeError();
  call(this);
  CRASH_IF_FALSE(openHandleScopesBefore == napiValueStackScopes_.size());
  if (!thrownJSError_.isEmpty()) {
    runtime_.setThrownValue(thrownJSError_);
    thrownJSError_ = EmptyHermesValue;
  }
  return runtime_.getThrownValue().isEmpty() ? vm::ExecutionStatus::RETURNED
                                             : vm::ExecutionStatus::EXCEPTION;
}

//-----------------------------------------------------------------------------
// Methods to work with napi_callbacks
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::getCallbackInfo(
    napi_callback_info callbackInfo,
    size_t *argCount,
    napi_value *args,
    napi_value *thisArg,
    void **data) noexcept {
  CHECK_ARG(callbackInfo);
  NodeApiCallbackInfo *cbInfo = asCallbackInfo(callbackInfo);
  if (args != nullptr) {
    CHECK_ARG(argCount);
    cbInfo->args(args, *argCount);
  }
  if (argCount != nullptr) {
    *argCount = cbInfo->argCount();
  }
  if (thisArg != nullptr) {
    *thisArg = cbInfo->thisArg();
  }
  if (data != nullptr) {
    *data = cbInfo->nativeData();
  }

  return clearLastNativeError();
}

napi_status NodeApiEnvironment::getNewTarget(
    napi_callback_info callbackInfo,
    napi_value *result) noexcept {
  CHECK_ARG(callbackInfo);
  return setResult(
      reinterpret_cast<NodeApiCallbackInfo *>(callbackInfo)->getNewTarget(),
      result);
}

//-----------------------------------------------------------------------------
// Property access helpers
//-----------------------------------------------------------------------------

const vm::PinnedHermesValue &NodeApiEnvironment::getPredefinedValue(
    NodeApiPredefined key) noexcept {
  return predefinedValues_[static_cast<size_t>(key)];
}

vm::SymbolID NodeApiEnvironment::getPredefinedSymbol(
    NodeApiPredefined key) noexcept {
  return getPredefinedValue(key).getSymbol();
}

template <class TObject>
napi_status NodeApiEnvironment::hasPredefinedProperty(
    TObject object,
    NodeApiPredefined key,
    bool *result) noexcept {
  return hasNamedProperty(object, getPredefinedSymbol(key), result);
}

template <class TObject>
napi_status NodeApiEnvironment::getPredefinedProperty(
    TObject object,
    NodeApiPredefined key,
    napi_value *result) noexcept {
  return getNamedProperty(object, getPredefinedSymbol(key), result);
}

template <class TObject, class TValue>
napi_status NodeApiEnvironment::setPredefinedProperty(
    TObject object,
    NodeApiPredefined key,
    TValue &&value,
    bool *optResult) noexcept {
  return setNamedProperty(
      object, getPredefinedSymbol(key), std::forward<TValue>(value), optResult);
}

template <class TObject>
napi_status NodeApiEnvironment::hasNamedProperty(
    TObject object,
    vm::SymbolID name,
    bool *result) noexcept {
  vm::CallResult<bool> res =
      vm::JSObject::hasNamed(makeHandle<vm::JSObject>(object), runtime_, name);
  return setResult(std::move(res), result);
}

template <class TObject>
napi_status NodeApiEnvironment::getNamedProperty(
    TObject object,
    vm::SymbolID name,
    napi_value *result) noexcept {
  vm::CallResult<vm::PseudoHandle<>> res = vm::JSObject::getNamed_RJS(
      makeHandle<vm::JSObject>(object),
      runtime_,
      name,
      vm::PropOpFlags().plusThrowOnError());
  return setResult(std::move(res), result);
}

template <class TObject, class TValue>
napi_status NodeApiEnvironment::setNamedProperty(
    TObject object,
    vm::SymbolID name,
    TValue &&value,
    bool *optResult) noexcept {
  vm::CallResult<bool> res = vm::JSObject::putNamed_RJS(
      makeHandle<vm::JSObject>(object),
      runtime_,
      name,
      makeHandle(std::forward<TValue>(value)),
      vm::PropOpFlags().plusThrowOnError());
  return setOptionalResult(std::move(res), optResult);
}

template <class TObject, class TKey>
napi_status NodeApiEnvironment::hasComputedProperty(
    TObject object,
    TKey key,
    bool *result) noexcept {
  vm::CallResult<bool> res = vm::JSObject::hasComputed(
      makeHandle<vm::JSObject>(object), runtime_, makeHandle(key));
  return setResult(std::move(res), result);
}

template <class TObject, class TKey>
napi_status NodeApiEnvironment::getComputedProperty(
    TObject object,
    TKey key,
    napi_value *result) noexcept {
  vm::CallResult<vm::PseudoHandle<>> res = vm::JSObject::getComputed_RJS(
      makeHandle<vm::JSObject>(object), runtime_, makeHandle(key));
  return setResult(std::move(res), result);
}

template <class TObject, class TKey, class TValue>
napi_status NodeApiEnvironment::setComputedProperty(
    TObject object,
    TKey key,
    TValue value,
    bool *optResult) noexcept {
  vm::CallResult<bool> res = vm::JSObject::putComputed_RJS(
      makeHandle<vm::JSObject>(object),
      runtime_,
      makeHandle(key),
      makeHandle(value),
      vm::PropOpFlags().plusThrowOnError());
  return setOptionalResult(std::move(res), optResult);
}

template <class TObject, class TKey>
napi_status NodeApiEnvironment::deleteComputedProperty(
    TObject object,
    TKey key,
    bool *optResult) noexcept {
  vm::CallResult<bool> res = vm::JSObject::deleteComputed(
      makeHandle<vm::JSObject>(object),
      runtime_,
      makeHandle(key),
      vm::PropOpFlags());
  return setOptionalResult(std::move(res), optResult);
}

template <class TObject, class TKey>
napi_status NodeApiEnvironment::getOwnComputedPropertyDescriptor(
    TObject object,
    TKey key,
    vm::MutableHandle<vm::SymbolID> &tmpSymbolStorage,
    vm::ComputedPropertyDescriptor &desc,
    bool *result) noexcept {
  vm::CallResult<bool> res = vm::JSObject::getOwnComputedDescriptor(
      makeHandle<vm::JSObject>(object),
      runtime_,
      makeHandle(key),
      tmpSymbolStorage,
      desc);
  return setOptionalResult(std::move(res), result);
}

template <class TObject>
napi_status NodeApiEnvironment::defineOwnProperty(
    TObject object,
    vm::SymbolID name,
    vm::DefinePropertyFlags dpFlags,
    vm::Handle<> valueOrAccessor,
    bool *result) noexcept {
  vm::CallResult<bool> res = vm::JSObject::defineOwnProperty(
      makeHandle<vm::JSObject>(object),
      runtime_,
      name,
      dpFlags,
      valueOrAccessor,
      vm::PropOpFlags().plusThrowOnError());
  return setOptionalResult(std::move(res), result);
}

//-----------------------------------------------------------------------------
// Methods to compare values
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::strictEquals(
    napi_value lhs,
    napi_value rhs,
    bool *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  CHECK_ARG(lhs);
  CHECK_ARG(rhs);
  vm::HermesValue::Tag lhsTag = phv(lhs)->getTag();
  if (lhsTag != phv(rhs)->getTag()) {
    return setResult(false, result);
  } else if (lhsTag == vm::HermesValue::Tag::Str) {
    return setResult(
        phv(lhs)->getString()->equals(phv(rhs)->getString()), result);
  } else if (lhsTag == vm::HermesValue::Tag::BoolSymbol) {
    vm::HermesValue::ETag lhsETag = phv(lhs)->getETag();
    if (lhsETag != phv(rhs)->getETag()) {
      return setResult(false, result);
    }
    if (lhsETag == vm::HermesValue::ETag::Symbol) {
      return setResult(phv(lhs)->getSymbol() == phv(rhs)->getSymbol(), result);
    } else {
      return setResult(phv(lhs)->getBool() == phv(rhs)->getBool(), result);
    }
  } else if (lhsTag == vm::HermesValue::Tag::BigInt) {
    return setResult(
        phv(lhs)->getBigInt()->compare(phv(rhs)->getBigInt()) == 0, result);
  } else {
    return setResult(phv(lhs)->getRaw() == phv(rhs)->getRaw(), result);
  }
}

//-----------------------------------------------------------------------------
// Methods to work with external data objects
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::defineClass(
    const char *utf8Name,
    size_t length,
    napi_callback constructor,
    void *callbackData,
    size_t propertyCount,
    const napi_property_descriptor *properties,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};

  CHECK_ARG(constructor);
  if (propertyCount > 0) {
    CHECK_ARG(properties);
  }

  vm::MutableHandle<vm::SymbolID> nameHandle{runtime_};
  CHECK_NAPI(getUniqueSymbolID(utf8Name, length, &nameHandle));

  vm::Handle<vm::JSObject> parentHandle =
      vm::Handle<vm::JSObject>::vmcast(&runtime_.functionPrototype);

  std::unique_ptr<NodeApiHostFunctionContext> context =
      std::make_unique<NodeApiHostFunctionContext>(
          *this, constructor, callbackData);
  vm::PseudoHandle<vm::NativeConstructor> ctorRes =
      vm::NativeConstructor::create(
          runtime_,
          parentHandle,
          context.get(),
          &NodeApiHostFunctionContext::func,
          /*paramCount:*/ 0,
          vm::NativeConstructor::creatorFunction<vm::JSObject>,
          vm::CellKind::JSObjectKind);
  vm::Handle<vm::JSObject> classHandle =
      makeHandle<vm::JSObject>(std::move(ctorRes));

  vm::NativeState *ns = vm::NativeState::create(
      runtime_, context.release(), &NodeApiHostFunctionContext::finalizeNS);

  vm::CallResult<bool> res = vm::JSObject::defineOwnProperty(
      classHandle,
      runtime_,
      vm::Predefined::getSymbolID(
          vm::Predefined::InternalPropertyArrayBufferExternalFinalizer),
      vm::DefinePropertyFlags::getDefaultNewPropertyFlags(),
      runtime_.makeHandle(ns));
  CHECK_NAPI(checkJSErrorStatus(res));
  RETURN_STATUS_IF_FALSE_WITH_MESSAGE(
      *res, napi_generic_failure, "Cannot set external finalizer for a class");

  vm::Handle<vm::JSObject> prototypeHandle{
      makeHandle(vm::JSObject::create(runtime_))};
  vm::ExecutionStatus st = vm::Callable::defineNameLengthAndPrototype(
      vm::Handle<vm::Callable>::vmcast(classHandle),
      runtime_,
      nameHandle.get(),
      /*paramCount:*/ 0,
      prototypeHandle,
      vm::Callable::WritablePrototype::Yes,
      /*strictMode*/ false);
  CHECK_NAPI(checkJSErrorStatus(st));

  for (size_t i = 0; i < propertyCount; ++i) {
    const napi_property_descriptor *p = properties + i;
    if ((p->attributes & napi_static) != 0) {
      CHECK_NAPI(defineProperties(napiValue(classHandle), 1, p));
    } else {
      CHECK_NAPI(defineProperties(napiValue(prototypeHandle), 1, p));
    }
  }

  return scope.setResult(std::move(classHandle));
}

napi_status NodeApiEnvironment::wrapObject(
    napi_value object,
    void *nativeData,
    napi_finalize finalizeCallback,
    void *finalizeHint,
    napi_ref *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};

  CHECK_OBJECT_ARG(object);
  if (result != nullptr) {
    // The returned reference should be deleted via napi_delete_reference()
    // ONLY in response to the finalize callback invocation. (If it is deleted
    // before that, then the finalize callback will never be invoked.)
    // Therefore a finalize callback is required when returning a reference.
    CHECK_ARG(finalizeCallback);
  }

  // If we've already wrapped this object, we error out.
  NodeApiExternalValue *externalValue;
  CHECK_NAPI(getExternalPropertyValue(
      object, NodeApiIfNotFound::ThenCreate, &externalValue));
  RETURN_STATUS_IF_FALSE(!externalValue->nativeData(), napi_invalid_arg);

  NodeApiReference *reference;
  CHECK_NAPI(NodeApiFinalizingComplexReference::create(
      *this,
      0,
      /*deleteSelf*/ result == nullptr,
      phv(object),
      nativeData,
      finalizeCallback,
      finalizeHint,
      reinterpret_cast<NodeApiFinalizingComplexReference **>(&reference)));
  externalValue->setNativeData(reference);
  return setOptionalResult(reinterpret_cast<napi_ref>(reference), result);
}

napi_status NodeApiEnvironment::addFinalizer(
    napi_value object,
    void *nativeData,
    napi_finalize finalizeCallback,
    void *finalizeHint,
    napi_ref *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};

  CHECK_OBJECT_ARG(object);
  CHECK_ARG(finalizeCallback);
  if (result != nullptr) {
    return NodeApiFinalizingComplexReference::create(
        *this,
        0,
        /*deleteSelf:*/ false,
        phv(object),
        nativeData,
        finalizeCallback,
        finalizeHint,
        reinterpret_cast<NodeApiFinalizingComplexReference **>(result));
  } else {
    return NodeApiFinalizingAnonymousReference::create(
        *this,
        phv(object),
        nativeData,
        finalizeCallback,
        finalizeHint,
        nullptr);
  }
}

template <NodeApiUnwrapAction action>
napi_status NodeApiEnvironment::unwrapObject(
    napi_value object,
    void **result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};

  CHECK_OBJECT_ARG(object);
  if /*constexpr*/ (action == NodeApiUnwrapAction::KeepWrap) {
    CHECK_ARG(result);
  }

  NodeApiExternalValue *externalValue = getExternalObjectValue(*phv(object));
  if (!externalValue) {
    CHECK_NAPI(getExternalPropertyValue(
        object, NodeApiIfNotFound::ThenReturnNull, &externalValue));
    RETURN_STATUS_IF_FALSE(externalValue, napi_invalid_arg);
  }

  NodeApiReference *reference = asReference(externalValue->nativeData());
  RETURN_STATUS_IF_FALSE(reference, napi_invalid_arg);
  if (result) {
    *result = reference->nativeData();
  }

  if /*constexpr*/ (action == NodeApiUnwrapAction::RemoveWrap) {
    externalValue->setNativeData(nullptr);
    NodeApiReference::deleteReference(
        *this, reference, NodeApiReference::ReasonToDelete::ZeroRefCount);
  }

  return clearLastNativeError();
}

napi_status NodeApiEnvironment::typeTagObject(
    napi_value object,
    const napi_type_tag *typeTag) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};

  CHECK_ARG(typeTag);
  napi_value objValue;
  CHECK_NAPI(coerceToObject(object, &objValue));

  // Fail if the tag already exists
  bool hasTag{};
  CHECK_NAPI(hasPredefinedProperty(
      objValue, NodeApiPredefined::napi_typeTag, &hasTag));
  RETURN_STATUS_IF_FALSE(!hasTag, napi_invalid_arg);

  napi_value tagBuffer;
  void *tagBufferData;
  CHECK_NAPI(
      createArrayBuffer(sizeof(napi_type_tag), &tagBufferData, &tagBuffer));

  const uint8_t *source = reinterpret_cast<const uint8_t *>(typeTag);
  uint8_t *dest = reinterpret_cast<uint8_t *>(tagBufferData);
  std::copy(source, source + sizeof(napi_type_tag), dest);

  return defineOwnProperty(
      objValue,
      getPredefinedSymbol(NodeApiPredefined::napi_typeTag),
      vm::DefinePropertyFlags::getNewNonEnumerableFlags(),
      makeHandle(tagBuffer),
      nullptr);
}

napi_status NodeApiEnvironment::checkObjectTypeTag(
    napi_value object,
    const napi_type_tag *typeTag,
    bool *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this};

  CHECK_ARG(typeTag);
  napi_value objValue;
  CHECK_NAPI(coerceToObject(object, &objValue));

  napi_value tagBufferValue;
  CHECK_NAPI(getPredefinedProperty(
      objValue, NodeApiPredefined::napi_typeTag, &tagBufferValue));
  vm::JSArrayBuffer *tagBuffer =
      vm::dyn_vmcast_or_null<vm::JSArrayBuffer>(*phv(tagBufferValue));
  if (tagBuffer == nullptr) {
    return setResult(false, result);
  }

  const uint8_t *source = reinterpret_cast<const uint8_t *>(typeTag);
  const uint8_t *tagBufferData = tagBuffer->getDataBlock(runtime_);
  return setResult(
      std::equal(
          source,
          source + sizeof(napi_type_tag),
          tagBufferData,
          tagBufferData + sizeof(napi_type_tag)),
      result);
}

napi_status NodeApiEnvironment::createExternal(
    void *nativeData,
    napi_finalize finalizeCallback,
    void *finalizeHint,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};

  CHECK_ARG(result);
  vm::Handle<vm::DecoratedObject> decoratedObj =
      createExternalObject(nativeData, nullptr);
  if (finalizeCallback) {
    CHECK_NAPI(NodeApiFinalizingAnonymousReference::create(
        *this,
        decoratedObj.unsafeGetPinnedHermesValue(),
        nativeData,
        finalizeCallback,
        finalizeHint,
        nullptr));
  }
  return scope.setResult(std::move(decoratedObj));
}

// Create the ExternalObject as a DecoratedObject with a special tag to
// distinguish it from other DecoratedObject instances.
vm::Handle<vm::DecoratedObject> NodeApiEnvironment::createExternalObject(
    void *nativeData,
    NodeApiExternalValue **externalValue) noexcept {
  vm::Handle<vm::DecoratedObject> decoratedObj =
      makeHandle(vm::DecoratedObject::create(
          runtime_,
          makeHandle<vm::JSObject>(&runtime_.objectPrototype),
          std::make_unique<NodeApiExternalValue>(
              pendingFinalizers_, nativeData),
          /*additionalSlotCount:*/ 1));

  // Add a special tag to differentiate from other decorated objects.
  vm::DecoratedObject::setAdditionalSlotValue(
      decoratedObj.get(),
      runtime_,
      kExternalTagSlotIndex,
      vm::SmallHermesValue::encodeNumberValue(kExternalValueTag, runtime_));

  if (externalValue) {
    *externalValue =
        static_cast<NodeApiExternalValue *>(decoratedObj->getDecoration());
  }

  return decoratedObj;
}

// Get the external value associated with the ExternalObject.
napi_status NodeApiEnvironment::getValueExternal(
    napi_value value,
    void **result) noexcept {
  NodeApiHandleScope scope{*this};
  CHECK_ARG(value);
  NodeApiExternalValue *externalValue = getExternalObjectValue(*phv(value));
  RETURN_STATUS_IF_FALSE(externalValue != nullptr, napi_invalid_arg);
  return setResult(externalValue->nativeData(), result);
}

// Get the NodeApiExternalValue from value if it is a DecoratedObject created by
// the createExternalObject method. Otherwise, return nullptr.
NodeApiExternalValue *NodeApiEnvironment::getExternalObjectValue(
    vm::HermesValue value) noexcept {
  if (vm::DecoratedObject *decoratedObj =
          vm::dyn_vmcast_or_null<vm::DecoratedObject>(value)) {
    vm::SmallHermesValue tag = vm::DecoratedObject::getAdditionalSlotValue(
        decoratedObj, runtime_, kExternalTagSlotIndex);
    if (tag.isNumber() && tag.getNumber(runtime_) == kExternalValueTag) {
      return static_cast<NodeApiExternalValue *>(decoratedObj->getDecoration());
    }
  }
  return nullptr;
}

// Get the NodeApiExternalValue from the object's property.
// If it is not found and the ifNotFound parameter is
// NodeApiIfNotFound::ThenCreate, then create the property with the new
// NodeApiExternalValue and return it.
template <class TObject>
napi_status NodeApiEnvironment::getExternalPropertyValue(
    TObject object,
    NodeApiIfNotFound ifNotFound,
    NodeApiExternalValue **result) noexcept {
  NodeApiExternalValue *externalValue{};
  napi_value napiExternalValue;
  napi_status status = getPredefinedProperty(
      object, NodeApiPredefined::napi_externalValue, &napiExternalValue);
  if (status == napi_ok &&
      vm::vmisa<vm::DecoratedObject>(*phv(napiExternalValue))) {
    externalValue = getExternalObjectValue(*phv(napiExternalValue));
    RETURN_FAILURE_IF_FALSE(externalValue != nullptr);
  } else if (ifNotFound == NodeApiIfNotFound::ThenCreate) {
    vm::Handle<vm::DecoratedObject> decoratedObj =
        createExternalObject(nullptr, &externalValue);
    CHECK_NAPI(defineOwnProperty(
        object,
        getPredefinedSymbol(NodeApiPredefined::napi_externalValue),
        vm::DefinePropertyFlags::getNewNonEnumerableFlags(),
        makeHandle(decoratedObj),
        nullptr));
  }
  return setResult(std::move(externalValue), result);
}

napi_status NodeApiEnvironment::addObjectFinalizer(
    const vm::PinnedHermesValue *value,
    NodeApiFinalizer *finalizer) noexcept {
  NodeApiExternalValue *externalValue = getExternalObjectValue(*value);
  if (!externalValue) {
    CHECK_NAPI(getExternalPropertyValue(
        value, NodeApiIfNotFound::ThenCreate, &externalValue));
  }
  externalValue->addFinalizer(finalizer);
  return clearLastNativeError();
}

void NodeApiEnvironment::callFinalizer(
    napi_finalize finalizeCallback,
    void *nativeData,
    void *finalizeHint) noexcept {
  callIntoModule([&](NodeApiEnvironment *env) {
    finalizeCallback(napiEnv(env), nativeData, finalizeHint);
  });
}

void NodeApiEnvironment::addToFinalizerQueue(
    NodeApiFinalizer *finalizer) noexcept {
  finalizerQueue_.pushBack(finalizer);
}

napi_status NodeApiEnvironment::processFinalizerQueue() noexcept {
  if (!isRunningFinalizers_) {
    isRunningFinalizers_ = true;
    pendingFinalizers_->applyPendingFinalizers(this);
    NodeApiReference::finalizeAll(*this, finalizerQueue_);
    isRunningFinalizers_ = false;
  }
  return napi_ok;
}

//-----------------------------------------------------------------------------
// Methods to control object lifespan
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::createReference(
    napi_value value,
    uint32_t initialRefCount,
    napi_ref *result) noexcept {
  return NodeApiComplexReference::create(
      *this,
      phv(value),
      initialRefCount,
      reinterpret_cast<NodeApiComplexReference **>(result));
}

napi_status NodeApiEnvironment::deleteReference(napi_ref ref) noexcept {
  CHECK_ARG(ref);
  if (isShuttingDown_) {
    // During shutdown all references are going to be deleted by finalizers.
    return clearLastNativeError();
  }
  return NodeApiReference::deleteReference(
      *this, asReference(ref), NodeApiReference::ReasonToDelete::ExternalCall);
}

napi_status NodeApiEnvironment::incReference(
    napi_ref ref,
    uint32_t *result) noexcept {
  CHECK_ARG(ref);
  uint32_t refCount{};
  CHECK_NAPI(asReference(ref)->incRefCount(*this, refCount));
  return setOptionalResult(std::move(refCount), result);
}

napi_status NodeApiEnvironment::decReference(
    napi_ref ref,
    uint32_t *result) noexcept {
  CHECK_ARG(ref);
  uint32_t refCount{};
  CHECK_NAPI(asReference(ref)->decRefCount(*this, refCount));
  return setOptionalResult(std::move(refCount), result);
}

napi_status NodeApiEnvironment::getReferenceValue(
    napi_ref ref,
    napi_value *result) noexcept {
  CHECK_ARG(ref);
  const vm::PinnedHermesValue &value = asReference(ref)->value(env);
  *result = !value.isUndefined() ? pushNewNodeApiValue(value) : nullptr;
  return clearLastNativeError();
}

void NodeApiEnvironment::addReference(NodeApiReference *reference) noexcept {
  references_.pushBack(reference);
}

void NodeApiEnvironment::addFinalizingReference(
    NodeApiReference *reference) noexcept {
  finalizingReferences_.pushBack(reference);
}

//-----------------------------------------------------------------------------
// Methods to control napi_value stack.
// napi_value are added on top of the stack.
// Closing napi_value stack scope deletes all napi_values added after
// opening the scope.
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::openNodeApiValueScope(
    napi_handle_scope *result) noexcept {
  size_t scope = napiValueStack_.size();
  napiValueStackScopes_.emplace(scope);
  return setResult(
      reinterpret_cast<napi_handle_scope>(&napiValueStackScopes_.top()),
      result);
}

napi_status NodeApiEnvironment::closeNodeApiValueScope(
    napi_handle_scope scope) noexcept {
  CHECK_ARG(scope);
  RETURN_STATUS_IF_FALSE(
      !napiValueStackScopes_.empty(), napi_handle_scope_mismatch);

  size_t *topScope = &napiValueStackScopes_.top();
  RETURN_STATUS_IF_FALSE(
      reinterpret_cast<size_t *>(scope) == topScope,
      napi_handle_scope_mismatch);

  napiValueStack_.resize(*topScope);
  napiValueStackScopes_.pop();
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::openEscapableNodeApiValueScope(
    napi_escapable_handle_scope *result) noexcept {
  CHECK_ARG(result);

  // Escapable handle scope must have a parent scope
  RETURN_STATUS_IF_FALSE(
      !napiValueStackScopes_.empty(), napi_handle_scope_mismatch);

  napiValueStack_.emplace(); // value to escape to parent scope
  napiValueStack_.emplace(
      vm::HermesValue::encodeNativeUInt32(kEscapeableSentinelTag));

  return openNodeApiValueScope(reinterpret_cast<napi_handle_scope *>(result));
}

napi_status NodeApiEnvironment::closeEscapableNodeApiValueScope(
    napi_escapable_handle_scope scope) noexcept {
  CHECK_NAPI(
      closeNodeApiValueScope(reinterpret_cast<napi_handle_scope>(scope)));

  RETURN_STATUS_IF_FALSE(
      napiValueStack_.size() > 1, napi_handle_scope_mismatch);
  vm::PinnedHermesValue &sentinelTag = napiValueStack_.top();
  RETURN_STATUS_IF_FALSE(
      sentinelTag.isNativeValue(), napi_handle_scope_mismatch);
  uint32_t sentinelTagValue = sentinelTag.getNativeUInt32();
  RETURN_STATUS_IF_FALSE(
      sentinelTagValue == kEscapeableSentinelTag ||
          sentinelTagValue == kUsedEscapeableSentinelTag,
      napi_handle_scope_mismatch);

  napiValueStack_.pop();
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::escapeNodeApiValue(
    napi_escapable_handle_scope scope,
    napi_value escapee,
    napi_value *result) noexcept {
  CHECK_ARG(scope);
  CHECK_ARG(escapee);

  size_t *stackScope = reinterpret_cast<size_t *>(scope);
  RETURN_STATUS_IF_FALSE(*stackScope > 1, napi_invalid_arg);
  RETURN_STATUS_IF_FALSE(
      *stackScope <= napiValueStack_.size(), napi_invalid_arg);

  vm::PinnedHermesValue &sentinelTag = napiValueStack_[*stackScope - 1];
  RETURN_STATUS_IF_FALSE(sentinelTag.isNativeValue(), napi_invalid_arg);
  uint32_t sentinelTagValue = sentinelTag.getNativeUInt32();
  RETURN_STATUS_IF_FALSE(
      sentinelTagValue != kUsedEscapeableSentinelTag, napi_escape_called_twice);
  RETURN_STATUS_IF_FALSE(
      sentinelTagValue == kEscapeableSentinelTag, napi_invalid_arg);

  vm::PinnedHermesValue &escapedValue = napiValueStack_[*stackScope - 2];
  escapedValue = *phv(escapee);
  sentinelTag = vm::HermesValue::encodeNativeUInt32(kUsedEscapeableSentinelTag);

  return setResult(napiValue(&escapedValue), result);
}

napi_value NodeApiEnvironment::pushNewNodeApiValue(
    vm::HermesValue value) noexcept {
  napiValueStack_.emplace(value);
  return napiValue(&napiValueStack_.top());
}

//-----------------------------------------------------------------------------
// Methods to work with weak roots.
//-----------------------------------------------------------------------------

vm::WeakRoot<vm::JSObject> NodeApiEnvironment::createWeakRoot(
    vm::JSObject *object) noexcept {
  return vm::WeakRoot<vm::JSObject>(object, runtime_);
}

const vm::PinnedHermesValue &NodeApiEnvironment::lockWeakRoot(
    vm::WeakRoot<vm::JSObject> &weakRoot) noexcept {
  if (vm::JSObject *ptr = weakRoot.get(runtime_, runtime_.getHeap())) {
    return *phv(pushNewNodeApiValue(vm::HermesValue::encodeObjectValue(ptr)));
  }
  return getUndefined();
}

//-----------------------------------------------------------------------------
// Methods to work with ordered sets.
// We use them as a temporary storage while retrieving property names.
// They are treated as GC roots.
//-----------------------------------------------------------------------------

void NodeApiEnvironment::pushOrderedSet(
    NodeApiOrderedSet<vm::HermesValue> &set) noexcept {
  orderedSets_.push_back(&set);
}

void NodeApiEnvironment::popOrderedSet() noexcept {
  orderedSets_.pop_back();
}

//-----------------------------------------------------------------------------
// Methods to work with array buffers and typed arrays
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::createArrayBuffer(
    size_t byteLength,
    void **data,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};
  vm::Handle<vm::JSArrayBuffer> buffer = makeHandle(vm::JSArrayBuffer::create(
      runtime_, makeHandle<vm::JSObject>(runtime_.arrayBufferPrototype)));
  CHECK_NAPI(checkJSErrorStatus(
      vm::JSArrayBuffer::createDataBlock(runtime_, buffer, byteLength, true)));
  if (data != nullptr) {
    *data = buffer->getDataBlock(runtime_);
  }
  return scope.setResult(std::move(buffer));
}

napi_status NodeApiEnvironment::createExternalArrayBuffer(
    void *externalData,
    size_t byteLength,
    napi_finalize finalizeCallback,
    void *finalizeHint,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};
  vm::Handle<vm::JSArrayBuffer> buffer = makeHandle(vm::JSArrayBuffer::create(
      runtime_, makeHandle<vm::JSObject>(&runtime_.arrayBufferPrototype)));
  if (externalData != nullptr) {
    std::unique_ptr<NodeApiExternalBuffer> externalBuffer =
        std::make_unique<NodeApiExternalBuffer>(
            env, externalData, byteLength, finalizeCallback, finalizeHint);
    vm::JSArrayBuffer::setExternalDataBlock(
        runtime_,
        buffer,
        reinterpret_cast<uint8_t *>(externalData),
        byteLength,
        externalBuffer.release(),
        [](vm::GC & /*gc*/, vm::NativeState *ns) {
          std::unique_ptr<NodeApiExternalBuffer> externalBuffer(
              reinterpret_cast<NodeApiExternalBuffer *>(ns->context()));
        });
  }
  return scope.setResult(std::move(buffer));
}

napi_status NodeApiEnvironment::isArrayBuffer(
    napi_value value,
    bool *result) noexcept {
  CHECK_ARG(value);
  return setResult(vm::vmisa<vm::JSArrayBuffer>(*phv(value)), result);
}

napi_status NodeApiEnvironment::getArrayBufferInfo(
    napi_value arrayBuffer,
    void **data,
    size_t *byteLength) noexcept {
  CHECK_ARG(arrayBuffer);
  RETURN_STATUS_IF_FALSE(
      vm::vmisa<vm::JSArrayBuffer>(*phv(arrayBuffer)), napi_invalid_arg);

  vm::JSArrayBuffer *buffer = vm::vmcast<vm::JSArrayBuffer>(*phv(arrayBuffer));
  if (data != nullptr) {
    *data = buffer->attached() ? buffer->getDataBlock(runtime_) : nullptr;
  }

  if (byteLength != nullptr) {
    *byteLength = buffer->attached() ? buffer->size() : 0;
  }

  return clearLastNativeError();
}

napi_status NodeApiEnvironment::detachArrayBuffer(
    napi_value arrayBuffer) noexcept {
  CHECK_ARG(arrayBuffer);
  vm::Handle<vm::JSArrayBuffer> buffer =
      makeHandle<vm::JSArrayBuffer>(arrayBuffer);
  RETURN_STATUS_IF_FALSE(buffer, napi_arraybuffer_expected);
  return checkJSErrorStatus(vm::JSArrayBuffer::detach(runtime_, buffer));
}

napi_status NodeApiEnvironment::isDetachedArrayBuffer(
    napi_value arrayBuffer,
    bool *result) noexcept {
  CHECK_ARG(arrayBuffer);
  vm::JSArrayBuffer *buffer =
      vm::dyn_vmcast_or_null<vm::JSArrayBuffer>(*phv(arrayBuffer));
  RETURN_STATUS_IF_FALSE(buffer, napi_arraybuffer_expected);
  return setResult(!buffer->attached(), result);
}

template <class TElement, vm::CellKind CellKind>
napi_status NodeApiEnvironment::createTypedArray(
    size_t length,
    vm::JSArrayBuffer *buffer,
    size_t byteOffset,
    vm::MutableHandle<vm::JSTypedArrayBase> *result) noexcept {
  constexpr size_t elementSize = sizeof(TElement);
  if (elementSize > 1) {
    if (byteOffset % elementSize != 0) {
      NodeApiStringBuilder sb(
          "start offset of ",
          getTypedArrayName<CellKind>(),
          " should be a multiple of ",
          elementSize);
      return env.throwJSRangeError(
          "ERR_NAPI_INVALID_TYPEDARRAY_ALIGNMENT", sb.c_str());
    }
  }
  if (length * elementSize + byteOffset > buffer->size()) {
    return env.throwJSRangeError(
        "ERR_NAPI_INVALID_TYPEDARRAY_ALIGNMENT", "Invalid typed array length");
  }
  using TypedArray = vm::JSTypedArray<TElement, CellKind>;
  *result = TypedArray::create(runtime_, TypedArray::getPrototype(runtime_));
  vm::JSTypedArrayBase::setBuffer(
      runtime_,
      result->get(),
      buffer,
      byteOffset,
      length * elementSize,
      static_cast<uint8_t>(elementSize));
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::createTypedArray(
    napi_typedarray_type type,
    size_t length,
    napi_value arrayBuffer,
    size_t byteOffset,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};
  CHECK_ARG(arrayBuffer);

  vm::JSArrayBuffer *buffer =
      vm::dyn_vmcast_or_null<vm::JSArrayBuffer>(*phv(arrayBuffer));
  RETURN_STATUS_IF_FALSE(buffer != nullptr, napi_invalid_arg);

  vm::MutableHandle<vm::JSTypedArrayBase> typedArray{runtime_};
  switch (type) {
    case napi_int8_array:
      CHECK_NAPI(createTypedArray<int8_t, vm::CellKind::Int8ArrayKind>(
          length, buffer, byteOffset, &typedArray));
      break;
    case napi_uint8_array:
      CHECK_NAPI(createTypedArray<uint8_t, vm::CellKind::Uint8ArrayKind>(
          length, buffer, byteOffset, &typedArray));
      break;
    case napi_uint8_clamped_array:
      CHECK_NAPI(createTypedArray<uint8_t, vm::CellKind::Uint8ClampedArrayKind>(
          length, buffer, byteOffset, &typedArray));
      break;
    case napi_int16_array:
      CHECK_NAPI(createTypedArray<int16_t, vm::CellKind::Int16ArrayKind>(
          length, buffer, byteOffset, &typedArray));
      break;
    case napi_uint16_array:
      CHECK_NAPI(createTypedArray<uint16_t, vm::CellKind::Uint16ArrayKind>(
          length, buffer, byteOffset, &typedArray));
      break;
    case napi_int32_array:
      CHECK_NAPI(createTypedArray<int32_t, vm::CellKind::Int32ArrayKind>(
          length, buffer, byteOffset, &typedArray));
      break;
    case napi_uint32_array:
      CHECK_NAPI(createTypedArray<uint32_t, vm::CellKind::Uint32ArrayKind>(
          length, buffer, byteOffset, &typedArray));
      break;
    case napi_float32_array:
      CHECK_NAPI(createTypedArray<float, vm::CellKind::Float32ArrayKind>(
          length, buffer, byteOffset, &typedArray));
      break;
    case napi_float64_array:
      CHECK_NAPI(createTypedArray<double, vm::CellKind::Float64ArrayKind>(
          length, buffer, byteOffset, &typedArray));
      break;
    case napi_bigint64_array:
      CHECK_NAPI(createTypedArray<int64_t, vm::CellKind::BigInt64ArrayKind>(
          length, buffer, byteOffset, &typedArray));
      break;
    case napi_biguint64_array:
      CHECK_NAPI(createTypedArray<uint64_t, vm::CellKind::BigUint64ArrayKind>(
          length, buffer, byteOffset, &typedArray));
      break;
    default:
      return ERROR_STATUS(
          napi_invalid_arg, "Unsupported TypedArray type: ", type);
  }

  return scope.setResult(std::move(typedArray));
}

template <vm::CellKind CellKind>
/*static*/ constexpr const char *
NodeApiEnvironment::getTypedArrayName() noexcept {
  static constexpr const char *names[] = {
#define TYPED_ARRAY(name, type) #name "Array",
#include "hermes/VM/TypedArrays.def"
  };
  return names
      [static_cast<int>(CellKind) -
       static_cast<int>(vm::CellKind::TypedArrayBaseKind_first)];
}

napi_status NodeApiEnvironment::isTypedArray(
    napi_value value,
    bool *result) noexcept {
  CHECK_ARG(value);
  return setResult(vm::vmisa<vm::JSTypedArrayBase>(*phv(value)), result);
}

napi_status NodeApiEnvironment::getTypedArrayInfo(
    napi_value typedArray,
    napi_typedarray_type *type,
    size_t *length,
    void **data,
    napi_value *arrayBuffer,
    size_t *byteOffset) noexcept {
  CHECK_ARG(typedArray);

  vm::JSTypedArrayBase *array =
      vm::dyn_vmcast_or_null<vm::JSTypedArrayBase>(*phv(typedArray));
  RETURN_STATUS_IF_FALSE(array != nullptr, napi_invalid_arg);

  if (type != nullptr) {
    if (vm::vmisa<vm::Int8Array>(array)) {
      *type = napi_int8_array;
    } else if (vm::vmisa<vm::Uint8Array>(array)) {
      *type = napi_uint8_array;
    } else if (vm::vmisa<vm::Uint8ClampedArray>(array)) {
      *type = napi_uint8_clamped_array;
    } else if (vm::vmisa<vm::Int16Array>(array)) {
      *type = napi_int16_array;
    } else if (vm::vmisa<vm::Uint16Array>(array)) {
      *type = napi_uint16_array;
    } else if (vm::vmisa<vm::Int32Array>(array)) {
      *type = napi_int32_array;
    } else if (vm::vmisa<vm::Uint32Array>(array)) {
      *type = napi_uint32_array;
    } else if (vm::vmisa<vm::Float32Array>(array)) {
      *type = napi_float32_array;
    } else if (vm::vmisa<vm::Float64Array>(array)) {
      *type = napi_float64_array;
    } else if (vm::vmisa<vm::BigInt64Array>(array)) {
      *type = napi_bigint64_array;
    } else if (vm::vmisa<vm::BigUint64Array>(array)) {
      *type = napi_biguint64_array;
    } else {
      return GENERIC_FAILURE("Unknown TypedArray type");
    }
  }

  if (length != nullptr) {
    *length = array->getLength();
  }

  if (data != nullptr) {
    *data = array->attached(runtime_)
        ? array->getBuffer(runtime_)->getDataBlock(runtime_) +
            array->getByteOffset()
        : nullptr;
  }

  if (arrayBuffer != nullptr) {
    *arrayBuffer = array->attached(runtime_)
        ? pushNewNodeApiValue(
              vm::HermesValue::encodeObjectValue(array->getBuffer(runtime_)))
        : napiValue(&getUndefined());
  }

  if (byteOffset != nullptr) {
    *byteOffset = array->getByteOffset();
  }

  return clearLastNativeError();
}

napi_status NodeApiEnvironment::createDataView(
    size_t byteLength,
    napi_value arrayBuffer,
    size_t byteOffset,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};
  CHECK_ARG(arrayBuffer);

  vm::JSArrayBuffer *buffer =
      vm::dyn_vmcast_or_null<vm::JSArrayBuffer>(*phv(arrayBuffer));
  RETURN_STATUS_IF_FALSE(buffer != nullptr, napi_invalid_arg);

  if (byteLength + byteOffset > buffer->size()) {
    return env.throwJSRangeError(
        "ERR_NAPI_INVALID_DATAVIEW_ARGS",
        "byte_offset + byte_length should be less than or "
        "equal to the size in bytes of the array passed in");
  }
  vm::Handle<vm::JSDataView> viewHandle = makeHandle(vm::JSDataView::create(
      runtime_, makeHandle<vm::JSObject>(runtime_.dataViewPrototype)));
  viewHandle->setBuffer(runtime_, buffer, byteOffset, byteLength);
  return scope.setResult(std::move(viewHandle));
}

napi_status NodeApiEnvironment::isDataView(
    napi_value value,
    bool *result) noexcept {
  CHECK_ARG(value);
  return setResult(vm::vmisa<vm::JSDataView>(*phv(value)), result);
}

napi_status NodeApiEnvironment::getDataViewInfo(
    napi_value dataView,
    size_t *byteLength,
    void **data,
    napi_value *arrayBuffer,
    size_t *byteOffset) noexcept {
  CHECK_ARG(dataView);

  vm::JSDataView *view = vm::dyn_vmcast_or_null<vm::JSDataView>(*phv(dataView));
  RETURN_STATUS_IF_FALSE(view, napi_invalid_arg);

  if (byteLength != nullptr) {
    *byteLength = view->byteLength();
  }

  if (data != nullptr) {
    *data = view->attached(runtime_)
        ? view->getBuffer(runtime_)->getDataBlock(runtime_) + view->byteOffset()
        : nullptr;
  }

  if (arrayBuffer != nullptr) {
    *arrayBuffer = view->attached(runtime_)
        ? pushNewNodeApiValue(view->getBuffer(runtime_).getHermesValue())
        : napiValue(&getUndefined());
  }

  if (byteOffset != nullptr) {
    *byteOffset = view->byteOffset();
  }

  return clearLastNativeError();
}

//-----------------------------------------------------------------------------
// Version management
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::getVersion(uint32_t *result) noexcept {
  return setResult(static_cast<uint32_t>(NAPI_VERSION), result);
}

//-----------------------------------------------------------------------------
// Methods to work with Promises
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::createPromise(
    napi_deferred *deferred,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};
  CHECK_ARG(deferred);

  napi_value jsPromise, jsDeferred;
  vm::MutableHandle<> jsResolve{runtime_};
  vm::MutableHandle<> jsReject{runtime_};
  CHECK_NAPI(createPromise(&jsPromise, &jsResolve, &jsReject));

  CHECK_NAPI(createObject(&jsDeferred));
  CHECK_NAPI(
      setPredefinedProperty(jsDeferred, NodeApiPredefined::resolve, jsResolve));
  CHECK_NAPI(
      setPredefinedProperty(jsDeferred, NodeApiPredefined::reject, jsReject));

  CHECK_NAPI(NodeApiStrongReference::create(
      *this,
      *phv(jsDeferred),
      reinterpret_cast<NodeApiStrongReference **>(deferred)));
  return scope.setResult(std::move(jsPromise));
}

napi_status NodeApiEnvironment::createPromise(
    napi_value *promise,
    vm::MutableHandle<> *resolveFunction,
    vm::MutableHandle<> *rejectFunction) noexcept {
  napi_value global, promiseConstructor;
  CHECK_NAPI(getGlobal(&global));
  CHECK_NAPI(getPredefinedProperty(
      global, NodeApiPredefined::Promise, &promiseConstructor));

  // The executor function is to be executed by the constructor during the
  // process of constructing the new Promise object. The executor is custom code
  // that ties an outcome to a promise. We return the resolveFunction and
  // rejectFunction given to the executor. Since the execution is synchronous,
  // we allocate executorData on the callstack.
  struct ExecutorData {
    static vm::CallResult<vm::HermesValue>
    callback(void *context, vm::Runtime & /*runtime*/, vm::NativeArgs args) {
      return (reinterpret_cast<ExecutorData *>(context))->callback(args);
    }

    vm::CallResult<vm::HermesValue> callback(const vm::NativeArgs &args) {
      *resolve = args.getArg(0);
      *reject = args.getArg(1);
      return env_->getUndefined();
    }

    NodeApiEnvironment *env_{};
    vm::MutableHandle<> *resolve{};
    vm::MutableHandle<> *reject{};
  } executorData{this, resolveFunction, rejectFunction};

  vm::Handle<vm::NativeFunction> executorFunction =
      vm::NativeFunction::createWithoutPrototype(
          runtime_,
          &executorData,
          &ExecutorData::callback,
          getPredefinedSymbol(NodeApiPredefined::Promise),
          2);
  napi_value func = pushNewNodeApiValue(executorFunction.getHermesValue());
  return createNewInstance(promiseConstructor, 1, &func, promise);
}

napi_status NodeApiEnvironment::resolveDeferred(
    napi_deferred deferred,
    napi_value resolution) noexcept {
  return concludeDeferred(deferred, NodeApiPredefined::resolve, resolution);
}

napi_status NodeApiEnvironment::rejectDeferred(
    napi_deferred deferred,
    napi_value resolution) noexcept {
  return concludeDeferred(deferred, NodeApiPredefined::reject, resolution);
}

napi_status NodeApiEnvironment::concludeDeferred(
    napi_deferred deferred,
    NodeApiPredefined predefinedProperty,
    napi_value resolution) noexcept {
  CHECK_ARG(deferred);
  CHECK_ARG(resolution);

  NodeApiReference *ref = asReference(deferred);

  const vm::PinnedHermesValue &jsDeferred = ref->value(*this);
  napi_value resolver, callResult;
  CHECK_NAPI(getPredefinedProperty(&jsDeferred, predefinedProperty, &resolver));
  CHECK_NAPI(callFunction(
      napi_value(&getUndefined()), resolver, 1, &resolution, &callResult));
  return NodeApiReference::deleteReference(
      *this, ref, NodeApiReference::ReasonToDelete::ZeroRefCount);
}

napi_status NodeApiEnvironment::isPromise(
    napi_value value,
    bool *result) noexcept {
  CHECK_ARG(value);

  napi_value global, promiseConstructor;
  CHECK_NAPI(getGlobal(&global));
  CHECK_NAPI(getPredefinedProperty(
      global, NodeApiPredefined::Promise, &promiseConstructor));

  return isInstanceOf(value, promiseConstructor, result);
}

napi_status NodeApiEnvironment::enablePromiseRejectionTracker() noexcept {
  NodeApiHandleScope scope{*this};

  vm::Handle<vm::NativeFunction> onUnhandled =
      vm::NativeFunction::createWithoutPrototype(
          runtime_,
          this,
          [](void *context,
             vm::Runtime &runtime,
             vm::NativeArgs args) -> vm::CallResult<vm::HermesValue> {
            return handleRejectionNotification(
                context,
                runtime,
                args,
                [](NodeApiEnvironment *env, int32_t id, vm::HermesValue error) {
                  env->lastUnhandledRejectionId_ = id;
                  env->lastUnhandledRejection_ = error;
                });
          },
          getPredefinedValue(NodeApiPredefined::onUnhandled).getSymbol(),
          /*paramCount:*/ 2);
  vm::Handle<vm::NativeFunction> onHandled =
      vm::NativeFunction::createWithoutPrototype(
          runtime_,
          this,
          [](void *context,
             vm::Runtime &runtime,
             vm::NativeArgs args) -> vm::CallResult<vm::HermesValue> {
            return handleRejectionNotification(
                context,
                runtime,
                args,
                [](NodeApiEnvironment *env, int32_t id, vm::HermesValue error) {
                  if (env->lastUnhandledRejectionId_ == id) {
                    env->lastUnhandledRejectionId_ = -1;
                    env->lastUnhandledRejection_ = EmptyHermesValue;
                  }
                });
          },
          getPredefinedValue(NodeApiPredefined::onHandled).getSymbol(),
          /*paramCount:*/ 2);

  napi_value options;
  CHECK_NAPI(createObject(&options));
  CHECK_NAPI(setPredefinedProperty(
      options,
      NodeApiPredefined::allRejections,
      vm::Runtime::getBoolValue(true)));
  CHECK_NAPI(setPredefinedProperty(
      options, NodeApiPredefined::onUnhandled, onUnhandled));
  CHECK_NAPI(
      setPredefinedProperty(options, NodeApiPredefined::onHandled, onHandled));

  vm::Handle<vm::Callable> hookFunc = vm::Handle<vm::Callable>::dyn_vmcast(
      makeHandle(&runtime_.promiseRejectionTrackingHook_));
  RETURN_FAILURE_IF_FALSE(hookFunc);
  return checkJSErrorStatus(vm::Callable::executeCall1(
      hookFunc, runtime_, vm::Runtime::getUndefinedValue(), *phv(options)));
}

/*static*/ vm::CallResult<vm::HermesValue>
NodeApiEnvironment::handleRejectionNotification(
    void *context,
    vm::Runtime &runtime,
    vm::NativeArgs args,
    void (*handler)(
        NodeApiEnvironment *env,
        int32_t id,
        vm::HermesValue error)) noexcept {
  // Args: id, error
  RAISE_ERROR_IF_FALSE(args.getArgCount() >= 2, u"Expected two arguments.");
  vm::HermesValue idArg = args.getArg(0);
  RAISE_ERROR_IF_FALSE(idArg.isNumber(), "id arg must be a Number.");
  int32_t id = NodeApiDoubleConversion::toInt32(idArg.getDouble());

  RAISE_ERROR_IF_FALSE(context != nullptr, u"Context must not be null.");
  NodeApiEnvironment *env = reinterpret_cast<NodeApiEnvironment *>(context);

  (*handler)(env, id, args.getArg(1));
  return env->getUndefined();
}

napi_status NodeApiEnvironment::hasUnhandledPromiseRejection(
    bool *result) noexcept {
  return setResult(lastUnhandledRejectionId_ != -1, result);
}

napi_status NodeApiEnvironment::getAndClearLastUnhandledPromiseRejection(
    napi_value *result) noexcept {
  lastUnhandledRejectionId_ = -1;
  return setResult(
      std::exchange(lastUnhandledRejection_, EmptyHermesValue), result);
}

napi_status NodeApiEnvironment::drainMicrotasks(
    int32_t maxCountHint,
    bool *result) noexcept {
  CHECK_ARG(result);
  if (runtime_.hasMicrotaskQueue()) {
    CHECK_NAPI(checkJSErrorStatus(runtime_.drainJobs()));
  }

  runtime_.clearKeptObjects();
  *result = true;
  return napi_ok;
}

//-----------------------------------------------------------------------------
// Memory management
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::adjustExternalMemory(
    int64_t change_in_bytes,
    int64_t *adjusted_value) noexcept {
  return GENERIC_FAILURE("Not implemented");
}

napi_status NodeApiEnvironment::collectGarbage() noexcept {
  runtime_.collect("test");
  CHECK_NAPI(processFinalizerQueue());
  return clearLastNativeError();
}

//-----------------------------------------------------------------------------
// Methods to work with Dates
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::createDate(
    double dateTime,
    napi_value *result) noexcept {
  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};
  vm::PseudoHandle<vm::JSDate> dateHandle = vm::JSDate::create(
      runtime_, dateTime, makeHandle<vm::JSObject>(&runtime_.datePrototype));
  return scope.setResult(std::move(dateHandle));
}

napi_status NodeApiEnvironment::isDate(
    napi_value value,
    bool *result) noexcept {
  CHECK_ARG(value);
  return setResult(vm::vmisa<vm::JSDate>(*phv(value)), result);
}

napi_status NodeApiEnvironment::getDateValue(
    napi_value value,
    double *result) noexcept {
  CHECK_ARG(value);
  vm::JSDate *date = vm::dyn_vmcast_or_null<vm::JSDate>(*phv(value));
  RETURN_STATUS_IF_FALSE(date != nullptr, napi_date_expected);
  return setResult(date->getPrimitiveValue(), result);
}

//-----------------------------------------------------------------------------
// Instance data
//-----------------------------------------------------------------------------

napi_status NodeApiEnvironment::setInstanceData(
    void *nativeData,
    napi_finalize finalizeCallback,
    void *finalizeHint) noexcept {
  if (instanceData_ != nullptr) {
    // Our contract so far has been to not finalize any old data there may be.
    // So we simply delete it.
    delete instanceData_;
    instanceData_ = nullptr;
  }
  return NodeApiInstanceData::create(
      *this, nativeData, finalizeCallback, finalizeHint, &instanceData_);
}

napi_status NodeApiEnvironment::getInstanceData(void **nativeData) noexcept {
  return setResult(
      instanceData_ ? instanceData_->nativeData() : nullptr, nativeData);
}

//---------------------------------------------------------------------------
// Script running
//---------------------------------------------------------------------------

napi_status NodeApiEnvironment::runScript(
    napi_value source,
    napi_value *result) noexcept {
  class StringBuffer : public Buffer {
   public:
    StringBuffer(std::string buffer) : string_(std::move(buffer)) {
      data_ = reinterpret_cast<const uint8_t *>(string_.c_str());
      size_ = string_.size();
    }

   private:
    std::string string_;
  };

  CHECK_NAPI(checkPendingJSError());
  NodeApiHandleScope scope{*this, result};

  // Convert the code into UTF8.
  size_t sourceSize{};
  CHECK_NAPI(getStringValueUTF8(source, nullptr, 0, &sourceSize));
  std::string code(sourceSize, '\0');
  CHECK_NAPI(getStringValueUTF8(source, &code[0], sourceSize + 1, nullptr));

  // Create a buffer for the code.
  std::unique_ptr<hermes::Buffer> codeBuffer(new StringBuffer(std::move(code)));

  hermes::vm::CallResult<hermes::vm::HermesValue> runResult =
      runtime_.run(std::move(codeBuffer), llvh::StringRef(), compileFlags_);
  return scope.setResult(std::move(runResult));
}

/*static*/ bool NodeApiEnvironment::isHermesBytecode(
    const uint8_t *data,
    size_t len) noexcept {
  return hbc::BCProviderFromBuffer::isBytecodeStream(
      llvh::ArrayRef<uint8_t>(data, len));
}

//---------------------------------------------------------------------------
// Methods to create Hermes GC handles for stack-based variables.
//---------------------------------------------------------------------------

vm::Handle<> NodeApiEnvironment::makeHandle(napi_value value) noexcept {
  return makeHandle(phv(value));
}

vm::Handle<> NodeApiEnvironment::makeHandle(
    const vm::PinnedHermesValue *value) noexcept {
  return vm::Handle<>(value);
}

vm::Handle<> NodeApiEnvironment::makeHandle(vm::HermesValue value) noexcept {
  return vm::Handle<>(runtime_, value);
}

vm::Handle<> NodeApiEnvironment::makeHandle(vm::Handle<> value) noexcept {
  return value;
}

// Useful for converting index to a name/index handle.
vm::Handle<> NodeApiEnvironment::makeHandle(uint32_t value) noexcept {
  return makeHandle(vm::HermesValue::encodeTrustedNumberValue(value));
}

template <class T>
vm::Handle<T> NodeApiEnvironment::makeHandle(napi_value value) noexcept {
  return vm::Handle<T>::vmcast(phv(value));
}

template <class T>
vm::Handle<T> NodeApiEnvironment::makeHandle(
    const vm::PinnedHermesValue *value) noexcept {
  return vm::Handle<T>::vmcast(value);
}

template <class T>
vm::Handle<T> NodeApiEnvironment::makeHandle(vm::HermesValue value) noexcept {
  return vm::Handle<T>::vmcast(runtime_, value);
}

template <class T, class TOther>
vm::Handle<T> NodeApiEnvironment::makeHandle(
    vm::Handle<TOther> value) noexcept {
  return vm::Handle<T>::vmcast(value);
}

template <class T>
vm::Handle<T> NodeApiEnvironment::makeHandle(
    vm::PseudoHandle<T> &&value) noexcept {
  return runtime_.makeHandle(std::move(value));
}

template <class T>
vm::CallResult<vm::Handle<T>> NodeApiEnvironment::makeHandle(
    vm::CallResult<vm::PseudoHandle<T>> &&callResult) noexcept {
  if (callResult.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  return runtime_.makeHandle(std::move(*callResult));
}

template <class T>
vm::CallResult<vm::MutableHandle<T>> NodeApiEnvironment::makeMutableHandle(
    vm::CallResult<vm::PseudoHandle<T>> &&callResult) noexcept {
  vm::CallResult<vm::Handle<T>> handleResult =
      makeHandle(std::move(callResult));
  if (handleResult.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  vm::MutableHandle<T> result{runtime_};
  result = *handleResult;
  return result;
}

//---------------------------------------------------------------------------
// Result setting helpers
//---------------------------------------------------------------------------

template <class T, class TResult>
napi_status NodeApiEnvironment::setResult(T &&value, TResult *result) noexcept {
  CHECK_ARG(result);
  return setResultUnsafe(std::forward<T>(value), result);
}

template <class T, class TResult>
napi_status NodeApiEnvironment::setOptionalResult(
    T &&value,
    TResult *result) noexcept {
  if (result) {
    return setResultUnsafe(std::forward<T>(value), result);
  }
  return checkCallResult(std::forward<T>(value));
}

template <class T>
napi_status NodeApiEnvironment::setOptionalResult(
    T &&value,
    std::nullptr_t) noexcept {
  return checkCallResult(std::forward<T>(value));
}

napi_status NodeApiEnvironment::setPredefinedResult(
    const vm::PinnedHermesValue *value,
    napi_value *result) noexcept {
  CHECK_ARG(result);
  *result = napiValue(value);
  return clearLastNativeError();
}

template <class T>
napi_status NodeApiEnvironment::setResultUnsafe(T &&value, T *result) noexcept {
  *result = std::forward<T>(value);
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::setResultUnsafe(
    vm::HermesValue value,
    napi_value *result) noexcept {
  *result = pushNewNodeApiValue(value);
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::setResultUnsafe(
    vm::SymbolID value,
    napi_value *result) noexcept {
  return setResultUnsafe(vm::HermesValue::encodeSymbolValue(value), result);
}

napi_status NodeApiEnvironment::setResultUnsafe(
    bool value,
    napi_value *result) noexcept {
  return setResultUnsafe(vm::HermesValue::encodeBoolValue(value), result);
}

template <class T>
napi_status NodeApiEnvironment::setResultUnsafe(
    vm::Handle<T> &&handle,
    napi_value *result) noexcept {
  return setResultUnsafe(handle.getHermesValue(), result);
}

template <class T>
napi_status NodeApiEnvironment::setResultUnsafe(
    vm::PseudoHandle<T> &&handle,
    napi_value *result) noexcept {
  return setResultUnsafe(handle.getHermesValue(), result);
}

template <class T>
napi_status NodeApiEnvironment::setResultUnsafe(
    vm::Handle<T> &&handle,
    vm::MutableHandle<T> *result) noexcept {
  *result = std::move(handle);
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::setResultUnsafe(
    vm::HermesValue value,
    vm::MutableHandle<> *result) noexcept {
  *result = value;
  return clearLastNativeError();
}

template <class T, class TResult>
napi_status NodeApiEnvironment::setResultUnsafe(
    vm::CallResult<T> &&value,
    TResult *result) noexcept {
  return setResultUnsafe(std::move(value), napi_generic_failure, result);
}

template <class T, class TResult>
napi_status NodeApiEnvironment::setResultUnsafe(
    vm::CallResult<T> &&value,
    napi_status onException,
    TResult *result) noexcept {
  CHECK_NAPI(checkJSErrorStatus(value, onException));
  return setResultUnsafe(std::move(*value), result);
}

template <class T>
napi_status NodeApiEnvironment::checkCallResult(
    const vm::CallResult<T> &value) noexcept {
  CHECK_NAPI(checkJSErrorStatus(value, napi_generic_failure));
  return clearLastNativeError();
}

template <class T>
napi_status NodeApiEnvironment::checkCallResult(const T & /*value*/) noexcept {
  return clearLastNativeError();
}

} // namespace hermes::node_api

//=============================================================================
// Node-API implementation
//=============================================================================

//-----------------------------------------------------------------------------
// Native error handling functions
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL napi_get_last_error_info(
    napi_env env,
    const napi_extended_error_info **result) {
  return CHECKED_ENV(env)->getLastNativeError(result);
}

//-----------------------------------------------------------------------------
// Getters for defined singletons
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL napi_get_undefined(napi_env env, napi_value *result) {
  return CHECKED_ENV(env)->getUndefined(result);
}

napi_status NAPI_CDECL napi_get_null(napi_env env, napi_value *result) {
  return CHECKED_ENV(env)->getNull(result);
}

napi_status NAPI_CDECL napi_get_global(napi_env env, napi_value *result) {
  return CHECKED_ENV(env)->getGlobal(result);
}

napi_status NAPI_CDECL
napi_get_boolean(napi_env env, bool value, napi_value *result) {
  return CHECKED_ENV(env)->getBoolean(value, result);
}

//-----------------------------------------------------------------------------
// Methods to create Primitive types/Objects
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL napi_create_object(napi_env env, napi_value *result) {
  return CHECKED_ENV(env)->createObject(result);
}

napi_status NAPI_CDECL napi_create_array(napi_env env, napi_value *result) {
  return CHECKED_ENV(env)->createArray(/*length:*/ 0, result);
}

napi_status NAPI_CDECL
napi_create_array_with_length(napi_env env, size_t length, napi_value *result) {
  return CHECKED_ENV(env)->createArray(length, result);
}

napi_status NAPI_CDECL
napi_create_double(napi_env env, double value, napi_value *result) {
  return CHECKED_ENV(env)->createNumber(value, result);
}

napi_status NAPI_CDECL
napi_create_int32(napi_env env, int32_t value, napi_value *result) {
  return CHECKED_ENV(env)->createNumber(value, result);
}

napi_status NAPI_CDECL
napi_create_uint32(napi_env env, uint32_t value, napi_value *result) {
  return CHECKED_ENV(env)->createNumber(value, result);
}

napi_status NAPI_CDECL
napi_create_int64(napi_env env, int64_t value, napi_value *result) {
  return CHECKED_ENV(env)->createNumber(value, result);
}

napi_status NAPI_CDECL napi_create_string_latin1(
    napi_env env,
    const char *str,
    size_t length,
    napi_value *result) {
  return CHECKED_ENV(env)->createStringLatin1(str, length, result);
}

napi_status NAPI_CDECL napi_create_string_utf8(
    napi_env env,
    const char *str,
    size_t length,
    napi_value *result) {
  return CHECKED_ENV(env)->createStringUTF8(str, length, result);
}

napi_status NAPI_CDECL napi_create_string_utf16(
    napi_env env,
    const char16_t *str,
    size_t length,
    napi_value *result) {
  return CHECKED_ENV(env)->createStringUTF16(str, length, result);
}

napi_status NAPI_CDECL node_api_create_external_string_latin1(
    napi_env env,
    char *str,
    size_t length,
    node_api_basic_finalize finalize_callback,
    void *finalize_hint,
    napi_value *result,
    bool *copied) {
  // TODO: implement
  // return CHECKED_ENV(env)->createExternalStringLatin1(
  //     str, length, finalize_callback, finalize_hint, result, copied);
  return napi_ok;
}

napi_status NAPI_CDECL node_api_create_external_string_utf16(
    napi_env env,
    char16_t *str,
    size_t length,
    node_api_basic_finalize finalize_callback,
    void *finalize_hint,
    napi_value *result,
    bool *copied) {
  // TODO: implement
  // return CHECKED_ENV(env)->createExternalStringUTF16(
  //    str, length, finalize_callback, finalize_hint, result, copied);
  return napi_ok;
}

napi_status NAPI_CDECL node_api_create_property_key_latin1(
    napi_env env,
    const char *str,
    size_t length,
    napi_value *result) {
  // TODO: implement
  // return CHECKED_ENV(env)->createPropertyKeyLatin1(str, length, result);
  return napi_ok;
}

napi_status NAPI_CDECL node_api_create_property_key_utf8(
    napi_env env,
    const char *str,
    size_t length,
    napi_value *result) {
  // TODO: implement
  // return CHECKED_ENV(env)->createPropertyKeyUTF8(str, length, result);
  return napi_ok;
}

napi_status NAPI_CDECL node_api_create_property_key_utf16(
    napi_env env,
    const char16_t *str,
    size_t length,
    napi_value *result) {
  // TODO: implement
  // return CHECKED_ENV(env)->createPropertyKeyUTF16(str, length, result);
  return napi_ok;
}

napi_status NAPI_CDECL
napi_create_symbol(napi_env env, napi_value description, napi_value *result) {
  return CHECKED_ENV(env)->createSymbol(description, result);
}

napi_status NAPI_CDECL node_api_symbol_for(
    napi_env env,
    const char *utf8description,
    size_t length,
    napi_value *result) {
  // TODO: implement
  // return CHECKED_ENV(env)->symbolFor(utf8description, length, result);
  return napi_ok;
}

napi_status NAPI_CDECL napi_create_function(
    napi_env env,
    const char *utf8name,
    size_t length,
    napi_callback cb,
    void *callback_data,
    napi_value *result) {
  return CHECKED_ENV(env)->createFunction(
      utf8name, length, cb, callback_data, result);
}

napi_status NAPI_CDECL napi_create_error(
    napi_env env,
    napi_value code,
    napi_value msg,
    napi_value *result) {
  return CHECKED_ENV(env)->createJSError(code, msg, result);
}

napi_status NAPI_CDECL napi_create_type_error(
    napi_env env,
    napi_value code,
    napi_value msg,
    napi_value *result) {
  return CHECKED_ENV(env)->createJSTypeError(code, msg, result);
}

napi_status NAPI_CDECL napi_create_range_error(
    napi_env env,
    napi_value code,
    napi_value msg,
    napi_value *result) {
  return CHECKED_ENV(env)->createJSRangeError(code, msg, result);
}

NAPI_EXTERN napi_status NAPI_CDECL node_api_create_syntax_error(
    napi_env env,
    napi_value code,
    napi_value msg,
    napi_value *result) {
  // TODO: implement
  // return CHECKED_ENV(env)->createJSSyntaxError(code, msg, result);
  return napi_ok;
}

//-----------------------------------------------------------------------------
// Methods to get the native napi_value from Primitive type
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL
napi_typeof(napi_env env, napi_value value, napi_valuetype *result) {
  return CHECKED_ENV(env)->typeOf(value, result);
}

napi_status NAPI_CDECL
napi_get_value_double(napi_env env, napi_value value, double *result) {
  return CHECKED_ENV(env)->getNumberValue(value, result);
}

napi_status NAPI_CDECL
napi_get_value_int32(napi_env env, napi_value value, int32_t *result) {
  return CHECKED_ENV(env)->getNumberValue(value, result);
}

napi_status NAPI_CDECL
napi_get_value_uint32(napi_env env, napi_value value, uint32_t *result) {
  return CHECKED_ENV(env)->getNumberValue(value, result);
}

napi_status NAPI_CDECL
napi_get_value_int64(napi_env env, napi_value value, int64_t *result) {
  return CHECKED_ENV(env)->getNumberValue(value, result);
}

napi_status NAPI_CDECL
napi_get_value_bool(napi_env env, napi_value value, bool *result) {
  return CHECKED_ENV(env)->getBooleanValue(value, result);
}

// Copies a JavaScript string into a LATIN-1 string buffer. The result is the
// number of bytes (excluding the null terminator) copied into buf.
// A sufficient buffer size should be greater than the length of string,
// reserving space for null terminator.
// If bufsize is insufficient, the string will be truncated and null terminated.
// If buf is NULL, this method returns the length of the string (in bytes)
// via the result parameter.
// The result argument is optional unless buf is NULL.
napi_status NAPI_CDECL napi_get_value_string_latin1(
    napi_env env,
    napi_value value,
    char *buf,
    size_t bufsize,
    size_t *result) {
  return CHECKED_ENV(env)->getStringValueLatin1(value, buf, bufsize, result);
}

// Copies a JavaScript string into a UTF-8 string buffer. The result is the
// number of bytes (excluding the null terminator) copied into buf.
// A sufficient buffer size should be greater than the length of string,
// reserving space for null terminator.
// If bufsize is insufficient, the string will be truncated and null terminated.
// If buf is NULL, this method returns the length of the string (in bytes)
// via the result parameter.
// The result argument is optional unless buf is NULL.
napi_status NAPI_CDECL napi_get_value_string_utf8(
    napi_env env,
    napi_value value,
    char *buf,
    size_t bufsize,
    size_t *result) {
  return CHECKED_ENV(env)->getStringValueUTF8(value, buf, bufsize, result);
}

// Copies a JavaScript string into a UTF-16 string buffer. The result is the
// number of 2-byte code units (excluding the null terminator) copied into buf.
// A sufficient buffer size should be greater than the length of string,
// reserving space for null terminator.
// If bufsize is insufficient, the string will be truncated and null terminated.
// If buf is NULL, this method returns the length of the string (in 2-byte
// code units) via the result parameter.
// The result argument is optional unless buf is NULL.
napi_status NAPI_CDECL napi_get_value_string_utf16(
    napi_env env,
    napi_value value,
    char16_t *buf,
    size_t bufsize,
    size_t *result) {
  return CHECKED_ENV(env)->getStringValueUTF16(value, buf, bufsize, result);
}

//-----------------------------------------------------------------------------
// Methods to coerce values
// These APIs may execute user scripts
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL
napi_coerce_to_bool(napi_env env, napi_value value, napi_value *result) {
  return CHECKED_ENV(env)->coerceToBoolean(value, result);
}

napi_status NAPI_CDECL
napi_coerce_to_number(napi_env env, napi_value value, napi_value *result) {
  return CHECKED_ENV(env)->coerceToNumber(value, result);
}

napi_status NAPI_CDECL
napi_coerce_to_object(napi_env env, napi_value value, napi_value *result) {
  return CHECKED_ENV(env)->coerceToObject(value, result);
}

napi_status NAPI_CDECL
napi_coerce_to_string(napi_env env, napi_value value, napi_value *result) {
  return CHECKED_ENV(env)->coerceToString(value, result);
}

//-----------------------------------------------------------------------------
// Methods to work with Objects
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL
napi_get_prototype(napi_env env, napi_value object, napi_value *result) {
  return CHECKED_ENV(env)->getPrototype(object, result);
}

napi_status NAPI_CDECL
napi_get_property_names(napi_env env, napi_value object, napi_value *result) {
  return CHECKED_ENV(env)->getForInPropertyNames(object, result);
}

napi_status NAPI_CDECL napi_set_property(
    napi_env env,
    napi_value object,
    napi_value key,
    napi_value value) {
  return CHECKED_ENV(env)->setProperty(object, key, value);
}

napi_status NAPI_CDECL napi_has_property(
    napi_env env,
    napi_value object,
    napi_value key,
    bool *result) {
  return CHECKED_ENV(env)->hasProperty(object, key, result);
}

napi_status NAPI_CDECL napi_get_property(
    napi_env env,
    napi_value object,
    napi_value key,
    napi_value *result) {
  return CHECKED_ENV(env)->getProperty(object, key, result);
}

napi_status NAPI_CDECL napi_delete_property(
    napi_env env,
    napi_value object,
    napi_value key,
    bool *result) {
  return CHECKED_ENV(env)->deleteProperty(object, key, result);
}

napi_status NAPI_CDECL napi_has_own_property(
    napi_env env,
    napi_value object,
    napi_value key,
    bool *result) {
  return CHECKED_ENV(env)->hasOwnProperty(object, key, result);
}

napi_status NAPI_CDECL napi_set_named_property(
    napi_env env,
    napi_value object,
    const char *utf8name,
    napi_value value) {
  return CHECKED_ENV(env)->setNamedProperty(object, utf8name, value);
}

napi_status NAPI_CDECL napi_has_named_property(
    napi_env env,
    napi_value object,
    const char *utf8name,
    bool *result) {
  return CHECKED_ENV(env)->hasNamedProperty(object, utf8name, result);
}

napi_status NAPI_CDECL napi_get_named_property(
    napi_env env,
    napi_value object,
    const char *utf8name,
    napi_value *result) {
  return CHECKED_ENV(env)->getNamedProperty(object, utf8name, result);
}

napi_status NAPI_CDECL napi_set_element(
    napi_env env,
    napi_value object,
    uint32_t index,
    napi_value value) {
  return CHECKED_ENV(env)->setElement(object, index, value);
}

napi_status NAPI_CDECL napi_has_element(
    napi_env env,
    napi_value object,
    uint32_t index,
    bool *result) {
  return CHECKED_ENV(env)->hasElement(object, index, result);
}

napi_status NAPI_CDECL napi_get_element(
    napi_env env,
    napi_value object,
    uint32_t index,
    napi_value *result) {
  return CHECKED_ENV(env)->getElement(object, index, result);
}

napi_status NAPI_CDECL napi_delete_element(
    napi_env env,
    napi_value object,
    uint32_t index,
    bool *result) {
  return CHECKED_ENV(env)->deleteElement(object, index, result);
}

napi_status NAPI_CDECL napi_define_properties(
    napi_env env,
    napi_value object,
    size_t property_count,
    const napi_property_descriptor *properties) {
  return CHECKED_ENV(env)->defineProperties(object, property_count, properties);
}

//-----------------------------------------------------------------------------
// Methods to work with Arrays
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL
napi_is_array(napi_env env, napi_value value, bool *result) {
  return CHECKED_ENV(env)->isArray(value, result);
}

napi_status NAPI_CDECL
napi_get_array_length(napi_env env, napi_value value, uint32_t *result) {
  return CHECKED_ENV(env)->getArrayLength(value, result);
}

//-----------------------------------------------------------------------------
// Methods to compare values
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL
napi_strict_equals(napi_env env, napi_value lhs, napi_value rhs, bool *result) {
  return CHECKED_ENV(env)->strictEquals(lhs, rhs, result);
}

//-----------------------------------------------------------------------------
// Methods to work with Functions
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL napi_call_function(
    napi_env env,
    napi_value recv,
    napi_value func,
    size_t argc,
    const napi_value *argv,
    napi_value *result) {
  return CHECKED_ENV(env)->callFunction(recv, func, argc, argv, result);
}

napi_status NAPI_CDECL napi_new_instance(
    napi_env env,
    napi_value constructor,
    size_t argc,
    const napi_value *argv,
    napi_value *result) {
  return CHECKED_ENV(env)->createNewInstance(constructor, argc, argv, result);
}

napi_status NAPI_CDECL napi_instanceof(
    napi_env env,
    napi_value object,
    napi_value constructor,
    bool *result) {
  return CHECKED_ENV(env)->isInstanceOf(object, constructor, result);
}

//-----------------------------------------------------------------------------
// Methods to work with napi_callbacks
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL napi_get_cb_info(
    napi_env env,
    napi_callback_info cbinfo,
    size_t *argc,
    napi_value *argv,
    napi_value *this_arg,
    void **data) {
  return CHECKED_ENV(env)->getCallbackInfo(cbinfo, argc, argv, this_arg, data);
}

napi_status NAPI_CDECL napi_get_new_target(
    napi_env env,
    napi_callback_info cbinfo,
    napi_value *result) {
  return CHECKED_ENV(env)->getNewTarget(cbinfo, result);
}

//-----------------------------------------------------------------------------
// Methods to work with external data objects
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL napi_define_class(
    napi_env env,
    const char *utf8name,
    size_t length,
    napi_callback constructor,
    void *callback_data,
    size_t property_count,
    const napi_property_descriptor *properties,
    napi_value *result) {
  return CHECKED_ENV(env)->defineClass(
      utf8name,
      length,
      constructor,
      callback_data,
      property_count,
      properties,
      result);
}

napi_status NAPI_CDECL napi_wrap(
    napi_env env,
    napi_value js_object,
    void *native_object,
    napi_finalize finalize_cb,
    void *finalize_hint,
    napi_ref *result) {
  return CHECKED_ENV(env)->wrapObject(
      js_object, native_object, finalize_cb, finalize_hint, result);
}

napi_status NAPI_CDECL
napi_unwrap(napi_env env, napi_value obj, void **result) {
  return CHECKED_ENV(env)
      ->unwrapObject<hermes::node_api::NodeApiUnwrapAction::KeepWrap>(
          obj, result);
}

napi_status NAPI_CDECL
napi_remove_wrap(napi_env env, napi_value obj, void **result) {
  return CHECKED_ENV(env)
      ->unwrapObject<hermes::node_api::NodeApiUnwrapAction::RemoveWrap>(
          obj, result);
}

napi_status NAPI_CDECL napi_create_external(
    napi_env env,
    void *data,
    napi_finalize finalize_cb,
    void *finalize_hint,
    napi_value *result) {
  return CHECKED_ENV(env)->createExternal(
      data, finalize_cb, finalize_hint, result);
}

napi_status NAPI_CDECL
napi_get_value_external(napi_env env, napi_value value, void **result) {
  return CHECKED_ENV(env)->getValueExternal(value, result);
}

//-----------------------------------------------------------------------------
// Methods to control object lifespan
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL napi_create_reference(
    napi_env env,
    napi_value value,
    uint32_t initial_refcount,
    napi_ref *result) {
  return CHECKED_ENV(env)->createReference(value, initial_refcount, result);
}

napi_status NAPI_CDECL napi_delete_reference(napi_env env, napi_ref ref) {
  return CHECKED_ENV(env)->deleteReference(ref);
}

napi_status NAPI_CDECL
napi_reference_ref(napi_env env, napi_ref ref, uint32_t *result) {
  return CHECKED_ENV(env)->incReference(ref, result);
}

napi_status NAPI_CDECL
napi_reference_unref(napi_env env, napi_ref ref, uint32_t *result) {
  return CHECKED_ENV(env)->decReference(ref, result);
}

napi_status NAPI_CDECL
napi_get_reference_value(napi_env env, napi_ref ref, napi_value *result) {
  return CHECKED_ENV(env)->getReferenceValue(ref, result);
}

napi_status NAPI_CDECL
napi_open_handle_scope(napi_env env, napi_handle_scope *result) {
  return CHECKED_ENV(env)->openNodeApiValueScope(result);
}

napi_status NAPI_CDECL
napi_close_handle_scope(napi_env env, napi_handle_scope scope) {
  return CHECKED_ENV(env)->closeNodeApiValueScope(scope);
}

napi_status NAPI_CDECL napi_open_escapable_handle_scope(
    napi_env env,
    napi_escapable_handle_scope *result) {
  return CHECKED_ENV(env)->openEscapableNodeApiValueScope(result);
}

napi_status NAPI_CDECL napi_close_escapable_handle_scope(
    napi_env env,
    napi_escapable_handle_scope scope) {
  return CHECKED_ENV(env)->closeEscapableNodeApiValueScope(scope);
}

napi_status NAPI_CDECL napi_escape_handle(
    napi_env env,
    napi_escapable_handle_scope scope,
    napi_value escapee,
    napi_value *result) {
  return CHECKED_ENV(env)->escapeNodeApiValue(scope, escapee, result);
}

//-----------------------------------------------------------------------------
// Methods to support JS error handling
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL napi_throw(napi_env env, napi_value error) {
  return CHECKED_ENV(env)->throwJSError(error);
}

napi_status NAPI_CDECL
napi_throw_error(napi_env env, const char *code, const char *msg) {
  return CHECKED_ENV(env)->throwJSError(code, msg);
}

napi_status NAPI_CDECL
napi_throw_type_error(napi_env env, const char *code, const char *msg) {
  return CHECKED_ENV(env)->throwJSTypeError(code, msg);
}

napi_status NAPI_CDECL
napi_throw_range_error(napi_env env, const char *code, const char *msg) {
  return CHECKED_ENV(env)->throwJSRangeError(code, msg);
}

napi_status NAPI_CDECL
node_api_throw_syntax_error(napi_env env, const char *code, const char *msg) {
  // TODO: implement
  // return CHECKED_ENV(env)->throwJSSyntaxError(code, msg);
  return napi_ok;
}

napi_status NAPI_CDECL
napi_is_error(napi_env env, napi_value value, bool *result) {
  return CHECKED_ENV(env)->isJSError(value, result);
}

//-----------------------------------------------------------------------------
// Methods to support catching exceptions
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL napi_is_exception_pending(napi_env env, bool *result) {
  return CHECKED_ENV(env)->isJSErrorPending(result);
}

napi_status NAPI_CDECL
napi_get_and_clear_last_exception(napi_env env, napi_value *result) {
  return CHECKED_ENV(env)->getAndClearPendingJSError(result);
}

//-----------------------------------------------------------------------------
// Methods to work with array buffers and typed arrays
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL
napi_is_arraybuffer(napi_env env, napi_value value, bool *result) {
  return CHECKED_ENV(env)->isArrayBuffer(value, result);
}

napi_status NAPI_CDECL napi_create_arraybuffer(
    napi_env env,
    size_t byte_length,
    void **data,
    napi_value *result) {
  return CHECKED_ENV(env)->createArrayBuffer(byte_length, data, result);
}

napi_status NAPI_CDECL napi_create_external_arraybuffer(
    napi_env env,
    void *external_data,
    size_t byte_length,
    napi_finalize finalize_cb,
    void *finalize_hint,
    napi_value *result) {
  return CHECKED_ENV(env)->createExternalArrayBuffer(
      external_data, byte_length, finalize_cb, finalize_hint, result);
}

napi_status NAPI_CDECL napi_get_arraybuffer_info(
    napi_env env,
    napi_value arraybuffer,
    void **data,
    size_t *byte_length) {
  return CHECKED_ENV(env)->getArrayBufferInfo(arraybuffer, data, byte_length);
}

napi_status NAPI_CDECL
napi_is_typedarray(napi_env env, napi_value value, bool *result) {
  return CHECKED_ENV(env)->isTypedArray(value, result);
}

napi_status NAPI_CDECL napi_create_typedarray(
    napi_env env,
    napi_typedarray_type type,
    size_t length,
    napi_value arraybuffer,
    size_t byte_offset,
    napi_value *result) {
  return CHECKED_ENV(env)->createTypedArray(
      type, length, arraybuffer, byte_offset, result);
}

napi_status NAPI_CDECL napi_get_typedarray_info(
    napi_env env,
    napi_value typedarray,
    napi_typedarray_type *type,
    size_t *length,
    void **data,
    napi_value *arraybuffer,
    size_t *byte_offset) {
  return CHECKED_ENV(env)->getTypedArrayInfo(
      typedarray, type, length, data, arraybuffer, byte_offset);
}

napi_status NAPI_CDECL napi_create_dataview(
    napi_env env,
    size_t byte_length,
    napi_value arraybuffer,
    size_t byte_offset,
    napi_value *result) {
  return CHECKED_ENV(env)->createDataView(
      byte_length, arraybuffer, byte_offset, result);
}

napi_status NAPI_CDECL
napi_is_dataview(napi_env env, napi_value value, bool *result) {
  return CHECKED_ENV(env)->isDataView(value, result);
}

napi_status NAPI_CDECL napi_get_dataview_info(
    napi_env env,
    napi_value dataview,
    size_t *byte_length,
    void **data,
    napi_value *arraybuffer,
    size_t *byte_offset) {
  return CHECKED_ENV(env)->getDataViewInfo(
      dataview, byte_length, data, arraybuffer, byte_offset);
}

//-----------------------------------------------------------------------------
// Version management
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL napi_get_version(napi_env env, uint32_t *result) {
  return CHECKED_ENV(env)->getVersion(result);
}

//-----------------------------------------------------------------------------
// Promises
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL napi_create_promise(
    napi_env env,
    napi_deferred *deferred,
    napi_value *promise) {
  return CHECKED_ENV(env)->createPromise(deferred, promise);
}

napi_status NAPI_CDECL napi_resolve_deferred(
    napi_env env,
    napi_deferred deferred,
    napi_value resolution) {
  return CHECKED_ENV(env)->resolveDeferred(deferred, resolution);
}

napi_status NAPI_CDECL napi_reject_deferred(
    napi_env env,
    napi_deferred deferred,
    napi_value resolution) {
  return CHECKED_ENV(env)->rejectDeferred(deferred, resolution);
}

napi_status NAPI_CDECL
napi_is_promise(napi_env env, napi_value value, bool *is_promise) {
  return CHECKED_ENV(env)->isPromise(value, is_promise);
}

//-----------------------------------------------------------------------------
// Running a script
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL
napi_run_script(napi_env env, napi_value script, napi_value *result) {
  return CHECKED_ENV(env)->runScript(script, result);
}

//-----------------------------------------------------------------------------
// Memory management
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL napi_adjust_external_memory(
    napi_env env,
    int64_t change_in_bytes,
    int64_t *adjusted_value) {
  return CHECKED_ENV(env)->adjustExternalMemory(
      change_in_bytes, adjusted_value);
}

//-----------------------------------------------------------------------------
// Dates
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL
napi_create_date(napi_env env, double time, napi_value *result) {
  return CHECKED_ENV(env)->createDate(time, result);
}

napi_status NAPI_CDECL
napi_is_date(napi_env env, napi_value value, bool *is_date) {
  return CHECKED_ENV(env)->isDate(value, is_date);
}

napi_status NAPI_CDECL
napi_get_date_value(napi_env env, napi_value value, double *result) {
  return CHECKED_ENV(env)->getDateValue(value, result);
}

//-----------------------------------------------------------------------------
// Add finalizer for pointer
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL napi_add_finalizer(
    napi_env env,
    napi_value js_object,
    void *native_object,
    napi_finalize finalize_cb,
    void *finalize_hint,
    napi_ref *result) {
  return CHECKED_ENV(env)->addFinalizer(
      js_object, native_object, finalize_cb, finalize_hint, result);
}

//-----------------------------------------------------------------------------
// BigInt
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL
napi_create_bigint_int64(napi_env env, int64_t value, napi_value *result) {
  return CHECKED_ENV(env)->createBigIntFromInt64(value, result);
}

napi_status NAPI_CDECL
napi_create_bigint_uint64(napi_env env, uint64_t value, napi_value *result) {
  return CHECKED_ENV(env)->createBigIntFromUint64(value, result);
}

napi_status NAPI_CDECL napi_create_bigint_words(
    napi_env env,
    int sign_bit,
    size_t word_count,
    const uint64_t *words,
    napi_value *result) {
  return CHECKED_ENV(env)->createBigIntFromWords(
      sign_bit, word_count, words, result);
}

napi_status NAPI_CDECL napi_get_value_bigint_int64(
    napi_env env,
    napi_value value,
    int64_t *result,
    bool *lossless) {
  return CHECKED_ENV(env)->getBigIntValueInt64(value, result, lossless);
}

napi_status NAPI_CDECL napi_get_value_bigint_uint64(
    napi_env env,
    napi_value value,
    uint64_t *result,
    bool *lossless) {
  return CHECKED_ENV(env)->getBigIntValueUint64(value, result, lossless);
}

napi_status NAPI_CDECL napi_get_value_bigint_words(
    napi_env env,
    napi_value value,
    int *sign_bit,
    size_t *word_count,
    uint64_t *words) {
  return CHECKED_ENV(env)->getBigIntValueWords(
      value, sign_bit, word_count, words);
}

//-----------------------------------------------------------------------------
// Object
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL napi_get_all_property_names(
    napi_env env,
    napi_value object,
    napi_key_collection_mode key_mode,
    napi_key_filter key_filter,
    napi_key_conversion key_conversion,
    napi_value *result) {
  return CHECKED_ENV(env)->getAllPropertyNames(
      object, key_mode, key_filter, key_conversion, result);
}

//-----------------------------------------------------------------------------
// Instance data
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL napi_set_instance_data(
    napi_env env,
    void *data,
    napi_finalize finalize_cb,
    void *finalize_hint) {
  return CHECKED_ENV(env)->setInstanceData(data, finalize_cb, finalize_hint);
}

napi_status NAPI_CDECL napi_get_instance_data(napi_env env, void **data) {
  return CHECKED_ENV(env)->getInstanceData(data);
}

//-----------------------------------------------------------------------------
// ArrayBuffer detaching
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL
napi_detach_arraybuffer(napi_env env, napi_value arraybuffer) {
  return CHECKED_ENV(env)->detachArrayBuffer(arraybuffer);
}

napi_status NAPI_CDECL napi_is_detached_arraybuffer(
    napi_env env,
    napi_value arraybuffer,
    bool *result) {
  return CHECKED_ENV(env)->isDetachedArrayBuffer(arraybuffer, result);
}

//-----------------------------------------------------------------------------
// Type tagging
//-----------------------------------------------------------------------------

napi_status NAPI_CDECL napi_type_tag_object(
    napi_env env,
    napi_value object,
    const napi_type_tag *type_tag) {
  return CHECKED_ENV(env)->typeTagObject(object, type_tag);
}

napi_status NAPI_CDECL napi_check_object_type_tag(
    napi_env env,
    napi_value object,
    const napi_type_tag *type_tag,
    bool *result) {
  return CHECKED_ENV(env)->checkObjectTypeTag(object, type_tag, result);
}

napi_status NAPI_CDECL napi_object_freeze(napi_env env, napi_value object) {
  return CHECKED_ENV(env)->objectFreeze(object);
}

napi_status NAPI_CDECL napi_object_seal(napi_env env, napi_value object) {
  return CHECKED_ENV(env)->objectSeal(object);
}

// TODO: remove support for previously existed jsr_* methods
