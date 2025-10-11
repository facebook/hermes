/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT license.
 *
 * Copyright notices for portions of code adapted from Hermes, Node.js, and V8
 * projects:
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
//   - NodeApiReference finalizers are run in JS thread by
//     processFinalizerQueueFromCode method which is called by
//     NodeApiHandleScope::setResult.
// - Each returned error status is backed up by the extended error message
//   stored in lastError_ that can be retrieved by napi_get_last_error_info.
// - We use macros to handle error statuses. It is done to reduce extensive use
//   of "if-return" statements, and to report failing expressions along with the
//   file name and code line number.

// TODO: Create NodeApiEnvironment with JSI Runtime
// TODO: Arrays with 2^32-1 elements (sparse arrays?)
// TODO: Reduce GCScope and value scopes.
// TODO: Add support for unique strings (aka PropNameID).
// TODO: make sure that all functions that return napi_value have top level
//       handle scope
// TODO: simplify all handle scope functions
// TODO: add post conditions to all handle scope functions
// TODO: Check the latest JSI impl for new instance and function calls
// TODO: Fix Clang compiler warnings
// TODO: Fix the bit comparison warning in napi_get_all_property_names
// TODO: verify Reference classes against Node.js
// TODO: Simplify non-Node-API API

// The Node-API implementation always contains experimental code.
#include "hermes/VM/CallResult.h"
#define NAPI_EXPERIMENTAL
#define NODE_API_EXPERIMENTAL_NO_WARNING

#include "hermes_node_api.h"

#include "hermes/VM/Callable.h"
#include "hermes/VM/HostModel.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSArrayBuffer.h"
#include "hermes/VM/JSDataView.h"
#include "hermes/VM/JSDate.h"
#include "hermes/VM/JSError.h"
#include "hermes/VM/JSTypedArray.h"
#include "hermes/VM/PropertyAccessor.h"
#include "llvh/ADT/SmallVector.h"
#include "llvh/Support/ConvertUTF.h"

#include <algorithm>
#include <atomic>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <unordered_set>

//=============================================================================
// Macros
//=============================================================================

// Check the Node-API status and return it if it is not napi_ok.
#define CHECK_STATUS(...)                                            \
  do {                                                               \
    if (napi_status status__ = (__VA_ARGS__); status__ != napi_ok) { \
      return status__;                                               \
    }                                                                \
  } while (false)

// Adopted from Node.js code
#ifdef _WIN32
#define ABORT() _exit(134)
#else
#define ABORT() abort()
#endif

#define ABORT_IF_FALSE(condition)  \
  do {                             \
    if (!(condition)) {            \
      assert(false && #condition); \
      ABORT();                     \
    }                              \
  } while (false)

// Return error status with message.
#define ERROR_STATUS(status, ...)         \
  ::hermes::node_api::setLastNativeError( \
      env, (status), (__FILE__), (uint32_t)(__LINE__), ##__VA_ARGS__)

// Return napi_generic_failure with message.
#define GENERIC_FAILURE(...) ERROR_STATUS(napi_generic_failure, ##__VA_ARGS__)

// Cast env to NodeApiEnvironment if it is not null.
#define CHECKED_ENV(env)                                          \
  ((env) == nullptr)                                              \
      ? napi_invalid_arg                                          \
      : reinterpret_cast<hermes::node_api::NodeApiEnvironment *>( \
            const_cast<napi_env>(env))

// Check conditions and return error status with message if it is false.
#define RETURN_STATUS_IF_FALSE_WITH_MESSAGE(condition, status, ...)      \
  do {                                                                   \
    if (!(condition)) {                                                  \
      return ::hermes::node_api::setLastNativeError(                     \
          env, (status), (__FILE__), (uint32_t)(__LINE__), __VA_ARGS__); \
    }                                                                    \
  } while (false)

// Check conditions and return error status if it is false.
#define RETURN_STATUS_IF_FALSE(condition, status) \
  RETURN_STATUS_IF_FALSE_WITH_MESSAGE(            \
      (condition), (status), "Condition is false: " #condition)

// Check that the argument is not nullptr.
#define CHECK_ARG(arg)                 \
  RETURN_STATUS_IF_FALSE_WITH_MESSAGE( \
      (arg) != nullptr, napi_invalid_arg, "Argument is null: " #arg)

#define CHECK_ENV(env)         \
  do {                         \
    if (env == nullptr) {      \
      return napi_invalid_arg; \
    }                          \
  } while (false)

// Check that the argument is of Object or Function type.
#define CHECK_ARG_IS_OBJECT(arg)             \
  do {                                       \
    CHECK_ARG(arg);                          \
    RETURN_STATUS_IF_FALSE_WITH_MESSAGE(     \
        phv(arg)->isObject(),                \
        napi_object_expected,                \
        "Argument is not an Object: " #arg); \
  } while (false)

// Check that the argument is of String type.
#define CHECK_ARG_IS_STRING(arg)            \
  do {                                      \
    CHECK_ARG(arg);                         \
    RETURN_STATUS_IF_FALSE_WITH_MESSAGE(    \
        phv(arg)->isString(),               \
        napi_string_expected,               \
        "Argument is not a String: " #arg); \
  } while (false)

#if !defined(NODE_API_CHECK_POSTCONDITIONS) && !defined(NDEBUG)
#define NODE_API_CHECK_POSTCONDITIONS
#endif

#ifdef NODE_API_CHECK_POSTCONDITIONS
// Check Node-API function post conditions.
#define CHECK_POSTCONDITIONS(env, valueStackDelta)                  \
  ::hermes::node_api::NodeApiPostConditionScope postConditionScope{ \
      env, valueStackDelta};
#define NO_RESULT_IF_NULL(expr) postConditionScope.noResultIfNull(expr)
#else
#define CHECK_POSTCONDITIONS(env, valueStackDelta)
#define NO_RESULT_IF_NULL(expr) expr
#endif

#if defined(_WIN32) && !defined(NDEBUG)
extern "C" __declspec(dllimport) void __stdcall DebugBreak();
#endif

bool operator==(const napi_type_tag &lhs, const napi_type_tag &rhs) {
  return lhs.lower == rhs.lower && lhs.upper == rhs.upper;
}

namespace std {
template <>
struct hash<napi_type_tag> {
  std::size_t operator()(const napi_type_tag &tag) const {
    // Combine the hash of individual members
    std::size_t h1 = std::hash<uint64_t>()(tag.lower);
    std::size_t h2 = std::hash<uint64_t>()(tag.upper);
    return h1 ^ (h2 << 1); // Combine hashes with XOR and bit-shifting
  }
};
} // namespace std

namespace hermes::node_api {

// The default version of Node-API to use when user did not request a specific
// version. The version 8 was the last version that was active before Node-API
// is added a mechanism to specify a Node-API version per module.
static const int32_t NodeApiDefaultVersion = 8;

//=============================================================================
// Forward declaration of all classes.
//=============================================================================

class NodeApiCallbackInfo;
class NodeApiDoubleConversion;
class NodeApiEnvironment;
class NodeApiExternalBuffer;
class NodeApiExternalValue;
class NodeApiHostFunctionContext;
template <class T>
class NodeApiOrderedSet;
class NodeApiPendingFinalizers;
template <class T>
class NodeApiRefCountedPtr;
template <class T>
class NodeApiStableAddressStack;
class NodeApiStringBuilder;
class NodeApiValueScope;
class NodeApiEscapableValueScope;
class NodeApiPostConditionScope;

// Forward declaration of NodeApiReference-related classes.
template <class T>
class NodeApiLinkedList;
class NodeApiRefTracker;
class NodeApiReference;
class NodeApiReferenceWithData;
class NodeApiReferenceWithFinalizer;
class NodeApiTrackedFinalizer;
class NodeApiFinalizer;
class NodeApiFinalizerHolder;

// Use existing NodeApiLinkedList pattern for ref tracking
using NodeApiRefList = NodeApiLinkedList<NodeApiRefTracker>;

// Use NodeApiReferenceList for reference management as per design document
using NodeApiReferenceList = NodeApiLinkedList<NodeApiReference>;

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

// Reinterpret cast node_api_basic_finalize to napi_finalize
napi_finalize basicFinalize(node_api_basic_finalize finalize) noexcept;

// Reinterpret cast vm::PinnedHermesValue pointer to napi_value
napi_value napiValue(const vm::PinnedHermesValue *value) noexcept;

// Get underlying vm::PinnedHermesValue and reinterpret cast it napi_value
template <class T>
napi_value napiValue(vm::Handle<T> value) noexcept;

// Reinterpret cast napi_value to vm::PinnedHermesValue pointer
const vm::PinnedHermesValue *phv(napi_value value) noexcept;
// Useful in templates and macros
const vm::PinnedHermesValue *phv(const vm::PinnedHermesValue *value) noexcept;

template <typename T = vm::HermesValue>
vm::Handle<T> asHandle(napi_value value) noexcept;

template <typename T = vm::HermesValue>
vm::Handle<T> asHandle(const vm::PinnedHermesValue *value) noexcept;

// Reinterpret cast napi_ref to NodeApiReference pointer
NodeApiReference *asReference(napi_ref ref) noexcept;
// Reinterpret cast void* to NodeApiReference pointer
NodeApiReference *asReference(void *ref) noexcept;

// Reinterpret cast to NodeApiHostFunctionContext::NodeApiCallbackInfo
NodeApiCallbackInfo *asCallbackInfo(napi_callback_info callbackInfo) noexcept;

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

static napi_status checkJSPreconditions(napi_env env) noexcept;

template <typename TLambda>
class LambdaTask : public Task {
 public:
  LambdaTask(TLambda &&lambda) : lambda_(std::move(lambda)) {}

  void invoke() noexcept override {
    lambda_();
  }

 private:
  TLambda lambda_;
};

template <class T>
std::unique_ptr<Task> makeTask(T &&lambda) noexcept {
  return std::make_unique<LambdaTask<std::remove_reference_t<T>>>(
      std::forward<T>(lambda));
}

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

  // Constructor that creates a new reference by incrementing ref count
  explicit NodeApiRefCountedPtr(T *ptr) noexcept : ptr_(ptr) {
    if (ptr_ != nullptr) {
      ptr_->incRefCount();
    }
  }

  NodeApiRefCountedPtr(const NodeApiRefCountedPtr &other) noexcept
      : ptr_(other.ptr_) {
    if (ptr_ != nullptr) {
      ptr_->incRefCount();
    }
  }

  NodeApiRefCountedPtr(NodeApiRefCountedPtr &&other) noexcept
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

  const T *operator->() const noexcept {
    return ptr_;
  }

  T *get() const noexcept {
    return ptr_;
  }

  explicit operator bool() const noexcept {
    return ptr_ != nullptr;
  }

  bool operator==(std::nullptr_t) const noexcept {
    return ptr_ == nullptr;
  }

  bool operator!=(std::nullptr_t) const noexcept {
    return ptr_ != nullptr;
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
    ABORT_IF_FALSE(size_ > 0 && "Size must be non zero.");
    --size_;
    reduceChunkCount();
  }

  void resize(size_t newSize) noexcept {
    ABORT_IF_FALSE(newSize <= size_ && "Size cannot be increased by resizing.");
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
    ABORT_IF_FALSE(size_ > 0 && "Size must be non zero.");
    size_t lastIndex = size_ - 1;
    return storage_[lastIndex / ChunkSize][lastIndex % ChunkSize];
  }

  T &operator[](size_t index) noexcept {
    ABORT_IF_FALSE(index < size_ && "Index must be less than size.");
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
  static constexpr size_t ChunkSize = 64;

  llvh::SmallVector<std::unique_ptr<T[]>, ChunkSize> storage_;
  size_t size_{0};
};

class NodeApiLinkedListBase {
 public:
  NodeApiLinkedListBase() noexcept {
    // The list is circular:
    // head.next_ points to the first item
    // head.prev_ points to the last item
    head_.next_ = &head_;
    head_.prev_ = &head_;
  }

  class Item {
    friend NodeApiLinkedListBase;
    template <typename T>
    friend class NodeApiLinkedList;

   public:
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

   private:
    void linkNext(Item *item) noexcept {
      if (item->isLinked()) {
        item->unlink();
      }
      item->prev_ = this;
      item->next_ = next_;
      item->next_->prev_ = item;
      next_ = item;
    }

   private:
    Item *next_{};
    Item *prev_{};
  };

  bool isEmpty() noexcept {
    return head_.next_ == head_.prev_;
  }

 protected:
  Item head_;
};

// An intrusive double linked list of items.
// Items in the list must inherit from NodeApiLinkedList<T>::Item.
// We use it instead of std::list to allow item to delete itself in its
// destructor and conveniently move items from list to another. The
// NodeApiLinkedList is used for References - the GC roots that are allocated in
// heap. The GC roots are the vm::PinnedHermesValue instances.
template <class T>
class NodeApiLinkedList final : public NodeApiLinkedListBase {
  // static_assert(
  //     std::is_base_of_v<Item, T>,
  //     "T must inherit from NodeApiLinkedListBase::Item.");
 public:
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

  template <class TLambda>
  void forEach(TLambda lambda) noexcept {
    for (T *item = begin(); item != end();) {
      // lambda can delete the item - get the next one before calling it.
      T *nextItem = static_cast<T *>(item->next_);
      lambda(item);
      item = nextItem;
    }
  }
};

// Specialized container for managing finalizers with O(1) operations
// CRITICAL: JS thread only - never access from background threads
class NodeApiFinalizerHolder {
 public:
  NodeApiFinalizerHolder() noexcept = default;

  // Add a finalizer reference (O(1) operation)
  void addFinalizer(NodeApiReference *ref) noexcept {
    if (ref != nullptr) {
      finalizers_.insert(ref);
    }
  }

  // Remove a finalizer reference (O(1) operation)
  void removeFinalizer(NodeApiReference *ref) noexcept {
    if (ref != nullptr) {
      finalizers_.erase(ref);
    }
  }

  // Invoke all finalizers in the holder
  void invokeAllFinalizers() noexcept; // Implementation after NodeApiReference
                                       // definition

  // Check if holder is empty
  bool isEmpty() const noexcept {
    return finalizers_.empty();
  }

  // Get number of finalizers
  size_t size() const noexcept {
    return finalizers_.size();
  }

 private:
  // O(1) operations via unordered_set
  std::unordered_set<NodeApiReference *> finalizers_;
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

  // Add a NodeApiFinalizerHolder from a NodeApiExternalValue destructor.
  // It can be called from JS or GC background threads.
  void addFinalizerHolder(
      std::unique_ptr<NodeApiFinalizerHolder> &&holder) noexcept {
    if (holder && !holder->isEmpty()) {
      std::scoped_lock lock{mutex_};
      finalizerHolders_.push_back(std::move(holder));
      hasPendingHolders_.store(true, std::memory_order_release);
    }
  }

  // Check if there are pending finalizer holders
  bool hasPendingFinalizers() const noexcept {
    return hasPendingHolders_.load(std::memory_order_acquire);
  }

  // Process all pending finalizer holders (must be called from JS thread)
  void processPendingFinalizers() noexcept {
    std::vector<std::unique_ptr<NodeApiFinalizerHolder>> holders;
    {
      std::scoped_lock lock{mutex_};
      if (finalizerHolders_.empty()) {
        return;
      }
      // Move to local variable to unlock mutex earlier
      holders = std::move(finalizerHolders_);
      hasPendingHolders_.store(false, std::memory_order_release);
    }

    // Process all holders outside the lock
    for (auto &holder : holders) {
      if (holder && !holder->isEmpty()) {
        holder->invokeAllFinalizers();
      }
    }
  }

 private:
  friend class NodeApiRefCountedPtr<NodeApiPendingFinalizers>;

  NodeApiPendingFinalizers() noexcept = default;

  void incRefCount() noexcept {
    int32_t refCount = refCount_.fetch_add(1, std::memory_order_relaxed) + 1;
    ABORT_IF_FALSE(refCount > 1 && "The ref count cannot bounce from zero.");
    ABORT_IF_FALSE(
        refCount < std::numeric_limits<int32_t>::max() &&
        "The ref count is too big.");
  }

  void decRefCount() noexcept {
    int32_t refCount = refCount_.fetch_sub(1, std::memory_order_release) - 1;
    ABORT_IF_FALSE(refCount >= 0 && "The ref count must not be negative.");
    if (refCount == 0) {
      std::atomic_thread_fence(std::memory_order_acquire);
      delete this;
    }
  }

 private:
  std::atomic<int32_t> refCount_{1};
  std::recursive_mutex mutex_;

  std::vector<std::unique_ptr<NodeApiFinalizerHolder>> finalizerHolders_;
  std::atomic<bool> hasPendingHolders_{false};
};

// The main class representing the Node-API environment.
// All Node-API functions are calling methods from this class.
class NodeApiEnvironment {
 public:
  // Initializes a new instance of NodeApiEnvironment.
  explicit NodeApiEnvironment(
      vm::Runtime &runtime,
      hbc::CompileFlags compileFlags,
      std::shared_ptr<TaskRunner> taskRunner,
      std::function<void(napi_env, napi_value)> unhandledErrorCallback,
      int32_t apiVersion,
      const NodeApiRefCountedPtr<NodeApiPendingFinalizers> &pendingFinalizers =
          NodeApiPendingFinalizers::create()) noexcept;

 public:
  // Exported function to increment the ref count by one.
  napi_status incRefCount() noexcept;

  // Exported function to decrement the ref count by one.
  // When the ref count becomes zero, the environment is deleted.
  napi_status decRefCount() noexcept;

  // Internal function to get the Hermes runtime.
  vm::Runtime &runtime() noexcept;

  hbc::CompileFlags compileFlags() noexcept;

  const std::shared_ptr<TaskRunner> &taskRunner() const noexcept;

  const std::function<void(napi_env, napi_value)> &unhandledErrorCallback()
      const noexcept;

  const NodeApiRefCountedPtr<NodeApiPendingFinalizers> &pendingFinalizers()
      const noexcept;

  // Internal function to get the stack of napi_value.
  NodeApiStableAddressStack<vm::PinnedHermesValue> &napiValueStack() noexcept;

  static NodeApiEnvironment *from(napi_env env) noexcept {
    return reinterpret_cast<NodeApiEnvironment *>(env);
  }

  vm::CallResult<vm::HermesValue> callModuleInitializer(
      napi_addon_register_func registerModule) noexcept;

  napi_status initializeModule(
      NodeApiEnvironment &moduleEnv,
      napi_addon_register_func registerModule,
      napi_value *exports) noexcept;

  void setParentEnvironment(NodeApiEnvironment *parentEnvironment) noexcept;

  //---------------------------------------------------------------------------
  // Native error handling methods
  //---------------------------------------------------------------------------
 public:
  // Internal function to set the last native error.
  napi_status setLastNativeError(
      napi_status status,
      const char *fileName,
      uint32_t line,
      const std::string &message) noexcept;

  // Internal function to clear the last error function.
  napi_status clearLastNativeError() noexcept;

  //---------------------------------------------------------------------------
  // Methods to support JS error handling
  //---------------------------------------------------------------------------
 public:
  // Internal function to set code property for the error object.
  // Node.js has a predefined set of codes for common errors.
  napi_status setJSErrorCode(
      vm::Handle<vm::JSError> error,
      napi_value code,
      const char *codeCString) noexcept;

  //---------------------------------------------------------------------------
  // Methods to support catching JS exceptions
  //---------------------------------------------------------------------------
 public:
  napi_status checkExecutionStatus(vm::ExecutionStatus hermesStatus) noexcept;

  napi_status setJSException() noexcept;

  void checkRuntimeThrownValue() noexcept;

  //---------------------------------------------------------------------------
  // Methods to work with Strings
  //---------------------------------------------------------------------------
 public:
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

  // Internal function to get all enumerable string property names as in the
  // for..in statement.
  // The keyConversion specifies if index properties must be converted to
  // strings. The function wraps up the Hermes optimized function that caches
  // results.
  napi_status getForInPropertyNames(
      vm::Handle<vm::JSObject> object,
      napi_key_conversion keyConversion,
      napi_value *result) noexcept;

  // Internal function to convert temporary key storage represented by a
  // array-builder-like ArrayStorage to a JS Array.
  napi_status convertKeyStorageToArray(
      vm::Handle<vm::ArrayStorageSmall> keyStorage,
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

  // Internal function to get SymbolID representing property identifier from the
  // property descriptor.
  napi_status symbolIDFromPropertyDescriptor(
      const napi_property_descriptor *descriptor,
      vm::MutableHandle<vm::SymbolID> *result) noexcept;

  //---------------------------------------------------------------------------
  // Methods to work with Functions
  //---------------------------------------------------------------------------
 public:
  // Internal function to create JS Function for a native callback.
  napi_status createFunction(
      vm::SymbolID name,
      napi_callback callback,
      void *callbackData,
      vm::MutableHandle<vm::Callable> *result) noexcept;

  // Enhanced callIntoModule with custom exception handler support
  // Follows Node.js CallIntoModule pattern from js_native_api_v8.h
  template <class TLambda, class TExceptionHandler>
  vm::ExecutionStatus callIntoModule(
      TLambda &&call,
      TExceptionHandler &&exceptionHandler) noexcept;

  static void rethrowException(
      NodeApiEnvironment *env,
      const vm::PinnedHermesValue *error) noexcept;

  static void triggerUnhandledException(
      NodeApiEnvironment *env,
      const vm::PinnedHermesValue *error) noexcept;

  void triggerFatalException(const vm::PinnedHermesValue *error) noexcept;

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
  napi_status hasPredefinedProperty(
      vm::Handle<vm::JSObject> object,
      NodeApiPredefined key,
      bool *result) noexcept;

  // Internal function to get property value by a predefined key.
  napi_status getPredefinedProperty(
      vm::Handle<vm::JSObject> object,
      NodeApiPredefined key,
      napi_value *result) noexcept;

  // Internal function to set property value by predefined key.
  napi_status setPredefinedProperty(
      vm::Handle<vm::JSObject> object,
      NodeApiPredefined key,
      vm::Handle<> value,
      bool *optResult = nullptr) noexcept;

  // Internal function to check if object has a property with provided name.
  napi_status hasNamedProperty(
      vm::Handle<vm::JSObject> object,
      vm::SymbolID name,
      bool *result) noexcept;

  // Internal function to get property by name.
  napi_status getNamedProperty(
      vm::Handle<vm::JSObject> object,
      vm::SymbolID name,
      napi_value *result) noexcept;

  // Internal function to set property by name.
  napi_status setNamedProperty(
      vm::Handle<vm::JSObject> object,
      vm::SymbolID name,
      vm::Handle<> value,
      bool *optResult = nullptr) noexcept;

  // Internal function to define a property.
  napi_status defineOwnProperty(
      vm::Handle<vm::JSObject> object,
      vm::SymbolID name,
      vm::DefinePropertyFlags dpFlags,
      vm::Handle<> valueOrAccessor,
      bool *result) noexcept;

  //---------------------------------------------------------------------------
  // Methods to work with external data objects
  //---------------------------------------------------------------------------
 public:
  // Internal function to create external value object.
  vm::Handle<vm::DecoratedObject> createExternalObject(
      void *nativeData,
      NodeApiExternalValue **externalValue) noexcept;

  // Internal function to get NodeApiExternalValue associated with the external
  // value type.
  NodeApiExternalValue *getExternalObjectValue(vm::HermesValue value) noexcept;

  // Internal function to get or create NodeApiExternalValue associated with the
  // external value type.
  napi_status getExternalPropertyValue(
      vm::Handle<vm::JSObject> object,
      NodeApiIfNotFound ifNotFound,
      NodeApiExternalValue **result) noexcept;

  // Internal function to associate a finalizer with an object.
  napi_status addObjectFinalizer(
      const vm::PinnedHermesValue *value,
      NodeApiReference *finalizer,
      NodeApiExternalValue **result) noexcept;

  // Internal function to call finalizer callback.
  void callFinalizer(
      napi_finalize finalizeCallback,
      void *nativeData,
      void *finalizeHint) noexcept;

  void enqueueFinalizer(NodeApiRefTracker *finalizer) noexcept;
  void dequeueFinalizer(NodeApiRefTracker *finalizer) noexcept;
  void drainFinalizerQueue() noexcept;

  void invokeFinalizerFromGC(NodeApiRefTracker *finalizer) noexcept;

  void processPendingFinalizers() noexcept;

  // Internal function to process finalizer queue immediately upon exiting
  // functions that may affect GC state.
  napi_status processFinalizerQueueFromCode() noexcept;

  //---------------------------------------------------------------------------
  // Methods to work with references.
  //---------------------------------------------------------------------------
 public:
  // Internal function to add non-finalizing reference.
  void addReference(NodeApiReference *reference) noexcept;

  // Internal function to add finalizing reference.
  void addFinalizingReference(NodeApiReference *reference) noexcept;

  //---------------------------------------------------------------------------
  // Methods to control napi_value stack.
  // napi_value are added on top of the stack.
  // Closing napi_value stack scope deletes all napi_values added after
  // opening the scope.
  //---------------------------------------------------------------------------
 public:
  // Internal function to push new napi_value to the napi_value stack and then
  // return it.
  napi_value pushNewNodeApiValue(const vm::HermesValue &value) noexcept;

  //---------------------------------------------------------------------------
  // Methods to work with weak roots.
  //---------------------------------------------------------------------------
 public:
  // Internal function to create weak root.
  vm::WeakRoot<vm::JSObject> createWeakRoot(vm::JSObject *object) noexcept;

  // Internal function to lock a weak root.
  const vm::PinnedHermesValue &lockWeakRoot(
      vm::WeakRoot<vm::JSObject> &weakRoot) noexcept;

  //---------------------------------------------------------------------------
  // Methods to work with ordered sets.
  // We use them as a temporary storage while retrieving property names.
  // They are treated as GC roots.
  //---------------------------------------------------------------------------
 public:
  // Internal function to add ordered set to be tracked by GC.
  void pushOrderedSet(NodeApiOrderedSet<vm::HermesValue> &set) noexcept;

  // Internal function to remove ordered set from being tracked by GC.
  void popOrderedSet() noexcept;

  //---------------------------------------------------------------------------
  // Methods to work with array buffers and typed arrays
  //---------------------------------------------------------------------------
 public:
  // Internal function to create TypedArray instance.
  template <class TItem, vm::CellKind CellKind>
  napi_status createTypedArray(
      size_t length,
      vm::JSArrayBuffer *buffer,
      size_t byteOffset,
      vm::MutableHandle<vm::JSTypedArrayBase> *result) noexcept;

  // Internal function to get TypedArray name.
  template <vm::CellKind cellKind>
  static constexpr const char *getTypedArrayName() noexcept;

  //---------------------------------------------------------------------------
  // Methods to work with Promises
  //---------------------------------------------------------------------------
 public:
  // Internal function to create Promise object instance.
  napi_status createPromise(
      napi_value *promise,
      vm::MutableHandle<> *resolveFunction,
      vm::MutableHandle<> *rejectFunction) noexcept;

  //---------------------------------------------------------------------------
  // Result setting helpers
  //
  // These functions help to reduce code responsible for returning results.
  //---------------------------------------------------------------------------
 public:
  napi_status makeResultValue(
      const vm::HermesValue &value,
      napi_value *result) noexcept;

  napi_status castResult(
      const vm::PinnedHermesValue *value,
      napi_value *result) noexcept;

 public:
  static bool isOnJSThread() noexcept {
    return tlsCurrentEnvironment_ != nullptr;
  }

 private:
  // tlsCurrentEnvironment_ RAII helper class
  class CurrentEnvironmentScope {
   public:
    explicit CurrentEnvironmentScope(NodeApiEnvironment *env) noexcept
        : env_(napiEnv(env)) {
      ABORT_IF_FALSE(openNodeApiScope(env_, &scope_) == napi_ok);
    }

    ~CurrentEnvironmentScope() noexcept {
      ABORT_IF_FALSE(closeNodeApiScope(env_, scope_) == napi_ok);
    }

   private:
    napi_env env_{};
    void *scope_{};
  };

  // tlsCurrentEnvironment_ RAII helper class
  class InGCFinalizerScope {
   public:
    explicit InGCFinalizerScope(NodeApiEnvironment *env) noexcept
        : env_(env), previousInGCFinalizer_(env->inGCFinalizer_) {
      env->inGCFinalizer_ = true;
    }

    ~InGCFinalizerScope() noexcept {
      env_->inGCFinalizer_ = previousInGCFinalizer_;
    }

   private:
    NodeApiEnvironment *env_;
    bool previousInGCFinalizer_;
  };

 public:
  // Private destructor - it must be triggered by ref counter.
  ~NodeApiEnvironment();

  // This method ensures all finalizers are drained before environment
  // destruction
  void deleteMe() noexcept;

  // Thread-local storage for current environment
  static thread_local NodeApiEnvironment *tlsCurrentEnvironment_;

  static thread_local llvh::SmallVector<NodeApiEnvironment *, 8>
      tlsCurrentEnvStack_;

  // Controls the lifetime of this class instances.
  std::atomic<int32_t> refCount_{1};

  // Used for safe update of finalizer queue.
  NodeApiRefCountedPtr<NodeApiPendingFinalizers> pendingFinalizers_;

  // Reference to the wrapped Hermes runtime.
  vm::Runtime &runtime_;

  // The Node-API version requested by the user of Node-API environment.
  int32_t apiVersion_{NodeApiDefaultVersion};

  // Reference to itself for convenient use in macros.
  napi_env env{napiEnv(this)};

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
  NodeApiReferenceList references_{};
  NodeApiReferenceList finalizingReferences_{};

  // Task runner finalizer queue for asynchronous finalizer execution
  // Contains NodeApiRefTracker instances that require JS thread execution via
  std::unordered_set<NodeApiRefTracker *> taskRunnerFinalizerQueue_{};

  // To ensure that the finalizerQueue_ is being processed only from a single
  // place at a time.
  bool isRunningFinalizers_{false};

  bool isScheduledAsyncFinalizers_{false};

  // True if finalizer is run from another function.
  bool inGCFinalizer_{false};

  // Helps to change the behavior of finalizers when the environment is
  // shutting down.
  bool isShuttingDown_{false};

  // Used by handleFinalizerException to determine if environment is in
  // termination state
  bool isTerminatedOrTerminating_{false};

  // Temporary GC roots for ordered sets used to collect property names.
  llvh::SmallVector<NodeApiOrderedSet<vm::HermesValue> *, 16> orderedSets_;

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
  NodeApiTrackedFinalizer *instanceData_{};

  // To run asynchronous tasks.
  std::shared_ptr<TaskRunner> taskRunner_;

  std::function<void(napi_env, napi_value)> unhandledErrorCallback_;

  NodeApiEnvironment *parentEnvironment_{};

  std::unordered_map<napi_type_tag, void *> taggedData_;

  // HermesValue used for uninitialized values.
  static constexpr vm::HermesValue EmptyHermesValue{
      vm::HermesValue::encodeEmptyValue()};

  static constexpr vm::PinnedHermesValue UndefinedHermesValue{
      vm::HermesValue::encodeUndefinedValue()};

  static constexpr vm::PinnedHermesValue NullHermesValue{
      vm::HermesValue::encodeNullValue()};

  static constexpr vm::PinnedHermesValue TrueHermesValue{
      vm::HermesValue::encodeBoolValue(true)};

  static constexpr vm::PinnedHermesValue FalseHermesValue{
      vm::HermesValue::encodeBoolValue(false)};

  // The sentinel tag in napiValueStack_ used for escapable values.
  // These are the first four ASCII letters of name "Janus" - the god of gates.
  static constexpr uint32_t kEscapableSentinelTag = 0x4a616e75;
  static constexpr uint32_t kUsedEscapableSentinelTag =
      kEscapableSentinelTag + 1;

  // Tag used to indicate external values for DecoratedObject.
  // These are the first four ASCII letters of word "External".
  static constexpr uint32_t kExternalValueTag = 0x45787465;
  static constexpr int32_t kExternalTagSlotIndex = 0;
};

thread_local NodeApiEnvironment *NodeApiEnvironment::tlsCurrentEnvironment_{};
thread_local llvh::SmallVector<NodeApiEnvironment *, 8>
    NodeApiEnvironment::tlsCurrentEnvStack_{};

// RAII class to control scope of napi_value variables.
class NodeApiValueScope {
 public:
  explicit NodeApiValueScope(NodeApiEnvironment &env) noexcept
      : env_(env), savedScope_(env.napiValueStack_.size()) {}

  ~NodeApiValueScope() noexcept {
    env_.napiValueStack_.resize(savedScope_);
  }

 private:
  NodeApiEnvironment &env_;
  size_t savedScope_;
};

// RAII class to control scope of napi_value variables and return values.
class NodeApiEscapableValueScope {
 public:
  explicit NodeApiEscapableValueScope(NodeApiEnvironment &env) noexcept
      : env_(env), savedScope_(env.napiValueStack_.size()) {
    // Reserve space for the escaping value.
    env_.pushNewNodeApiValue(NodeApiEnvironment::UndefinedHermesValue);
  }

  ~NodeApiEscapableValueScope() noexcept {
    env_.napiValueStack_.resize(
        isValueEscaped_ ? savedScope_ + 1 : savedScope_);
  }

  napi_status escapeResult(
      const vm::HermesValue &value,
      napi_value *result) noexcept {
    ABORT_IF_FALSE(!isValueEscaped_ && "The value is already escaped.");
    if (result != nullptr) {
      isValueEscaped_ = true;
      env_.napiValueStack_[savedScope_] = value;
      *result = napiValue(&env_.napiValueStack_[savedScope_]);
    }
    return env_.clearLastNativeError();
  }

  napi_status escapeResult(napi_value *result) noexcept {
    return escapeResult(*phv(*result), result);
  }

 private:
  NodeApiEnvironment &env_;
  size_t savedScope_;
  bool isValueEscaped_{false};
};

#ifdef NODE_API_CHECK_POSTCONDITIONS
// Verifies Node-API function postconditions
class NodeApiPostConditionScope {
 public:
  explicit NodeApiPostConditionScope(
      NodeApiEnvironment *env,
      ptrdiff_t valueStackDelta) noexcept
      : env_(env), valueStackDelta_(valueStackDelta) {
    if (env_ == nullptr) {
      return;
    }

    topGCScope_ = getTopGCScope();
    if (topGCScope_ != nullptr) {
      gcScopeMarker_.emplace(topGCScope_->createMarker());
    }

    oldValueStackSize_ = env_->napiValueStack_.size();
    oldValueStackScopesSize_ = env_->napiValueStackScopes_.size();
  }

  ~NodeApiPostConditionScope() noexcept {
    if (env_ == nullptr) {
      return;
    }

    // Make sure that the GCScope handle stack does not change.
    ABORT_IF_FALSE(topGCScope_ == getTopGCScope());
    if (topGCScope_ != nullptr) {
      vm::GCScope::Marker currentMarker = topGCScope_->createMarker();
      ABORT_IF_FALSE(gcScopeMarker_.value() == currentMarker);
    }

    // Verify that the value stack is updated as expected.
    if (valueStackDelta_ == 0) {
      ABORT_IF_FALSE(oldValueStackSize_ == env_->napiValueStack_.size());
    } else if (valueStackDelta_ > 0) {
      if (env_->lastError_.error_code == napi_ok) {
        // Only change the stack if function succeeded.
        ABORT_IF_FALSE(
            env_->napiValueStack_.size() ==
            oldValueStackSize_ + valueStackDelta_);
      } else {
        ABORT_IF_FALSE(oldValueStackSize_ == env_->napiValueStack_.size());
      }
    }

    // Verify that the scope count is not changed.
    ABORT_IF_FALSE(
        oldValueStackScopesSize_ == env_->napiValueStackScopes_.size());

    // See that vm::Runtime has no unhandled JS exceptions.
    ABORT_IF_FALSE(env_->runtime_.getThrownValue().isEmpty());
  }

  template <typename T>
  T *noResultIfNull(T *value) noexcept {
    if (value == nullptr) {
      valueStackDelta_ = 0;
    }
    return value;
  }

 private:
  // To access the protected getTopGCScope function.
  class HandleRootOwnerAccessor : public vm::HandleRootOwner {
   public:
    using vm::HandleRootOwner::getTopGCScope;
  };

  vm::GCScope *getTopGCScope() noexcept {
    vm::HandleRootOwner &rootOwner = env_->runtime_;
    return static_cast<HandleRootOwnerAccessor &>(rootOwner).getTopGCScope();
  }

 private:
  NodeApiEnvironment *env_;
  ptrdiff_t valueStackDelta_;
  vm::GCScope *topGCScope_{};
  std::optional<vm::GCScope::Marker> gcScopeMarker_{};
  size_t oldValueStackSize_{};
  size_t oldValueStackScopesSize_{};
};
#endif

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
    // Transfer finalizer holder to pending finalizers for thread-safe
    // processing
    if (finalizerHolder_ && !finalizerHolder_->isEmpty()) {
      pendingFinalizers_->addFinalizerHolder(std::move(finalizerHolder_));
      // If we are on JS thread, we can process finalizers immediately.
      // TODO: Ensure that the right env is on the right thread
      if (NodeApiEnvironment::isOnJSThread()) {
        pendingFinalizers_->processPendingFinalizers();
      }
    }
  }

  size_t getMallocSize() const override {
    return sizeof(*this);
  }

  void addFinalizer(NodeApiReference *finalizer) noexcept {
    if (!finalizerHolder_) {
      finalizerHolder_ = std::make_unique<NodeApiFinalizerHolder>();
    }
    finalizerHolder_->addFinalizer(finalizer);
  }

  void removeFinalizer(NodeApiReference *finalizer) noexcept {
    if (finalizerHolder_) {
      finalizerHolder_->removeFinalizer(finalizer);
    }
  }

  void *nativeData() noexcept {
    return nativeData_;
  }

  void setNativeData(void *value) noexcept {
    nativeData_ = value;
  }

  NodeApiFinalizerHolder *getFinalizerHolder() noexcept {
    return finalizerHolder_.get();
  }

 private:
  NodeApiRefCountedPtr<NodeApiPendingFinalizers> pendingFinalizers_;
  void *nativeData_{};
  std::unique_ptr<NodeApiFinalizerHolder> finalizerHolder_;
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

  static vm::CallResult<vm::HermesValue> func(
      void *context,
      vm::Runtime &runtime);

  // Used to delete the context by function finalizer.
  static void finalize(void *context) {
    delete reinterpret_cast<class NodeApiHostFunctionContext *>(context);
  }

  // Used to delete the context by functions used as class constructors.
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
      buffer[i] = napiValue(&context_.env_.UndefinedHermesValue);
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
    vm::Runtime &runtime) {
  vm::NativeArgs hvArgs = runtime.getCurrentFrame().getNativeArgs();
  NodeApiHostFunctionContext *hfc =
      reinterpret_cast<NodeApiHostFunctionContext *>(context);
  NodeApiEnvironment &env = hfc->env_;
  assert(&runtime == &env.runtime());

  NodeApiValueScope scope{env};
  vm::GCScope gcScope{runtime};
  NodeApiCallbackInfo callbackInfo{*hfc, hvArgs};
  napi_value result{};
  vm::ExecutionStatus status = env.callIntoModule(
      [&](NodeApiEnvironment *env) {
        result = hfc->hostCallback_(
            napiEnv(env), reinterpret_cast<napi_callback_info>(&callbackInfo));
      },
      NodeApiEnvironment::rethrowException);

  if (status == vm::ExecutionStatus::EXCEPTION) {
    return vm::ExecutionStatus::EXCEPTION;
  }

  if (result) {
    return *phv(result);
  } else {
    return env.UndefinedHermesValue;
  }
}

//=============================================================================
// HermesNodeApi Classes - Node.js RefTracker Pattern Implementation
//=============================================================================

// Enum for tracking reference ownership patterns.
enum class NodeApiReferenceOwnership : uint8_t {
  kRuntime, // The reference is owned by the runtime. No userland call is needed
            // to destruct the reference.
  kUserland, // The reference is owned by the userland. User code is responsible
             // to delete the reference with appropriate node-api calls.
};

// Composition class for finalizer callbacks (replaces inheritance pattern)
class NodeApiFinalizer {
 public:
  NodeApiFinalizer(
      void *nativeData,
      napi_finalize finalizeCallback,
      void *finalizeHint) noexcept
      : nativeData_(nativeData),
        finalizeCallback_(finalizeCallback),
        finalizeHint_(finalizeHint) {}

  // Execute the finalizer callback
  void callFinalizer(NodeApiEnvironment &env) noexcept {
    if (finalizeCallback_ != nullptr) {
      env.callIntoModule(
          [&](NodeApiEnvironment *envPtr) {
            finalizeCallback_(napiEnv(envPtr), nativeData_, finalizeHint_);
          },
          NodeApiEnvironment::triggerUnhandledException);
    }
  }

  // Accessors
  napi_finalize getFinalizeCallback() const noexcept {
    return finalizeCallback_;
  }

  void *getNativeData() const noexcept {
    return nativeData_;
  }

  void *getFinalizeHint() const noexcept {
    return finalizeHint_;
  }

  // Check if finalizer is set
  bool hasFinalizer() const noexcept {
    return finalizeCallback_ != nullptr;
  }

 private:
  void *nativeData_{nullptr};
  napi_finalize finalizeCallback_{nullptr};
  void *finalizeHint_{nullptr};
};

// Specialized container for managing finalizers with O(1) operations
// Base class for all trackable references and finalizers - uses Hermes linked
// list pattern
class NodeApiRefTracker : public NodeApiLinkedListBase::Item {
 public:
  NodeApiRefTracker() noexcept = default;

  virtual ~NodeApiRefTracker() noexcept {
    unlink(); // Unlink from the list on destruction
  }

  // Virtual finalize method for cleanup operations
  virtual void finalize() noexcept {}

  // Static method to finalize all items in a list
  template <typename T>
  static void finalizeAll(NodeApiLinkedList<T> &list) noexcept {
    list.forEach([](NodeApiRefTracker *item) {
      item->finalize();
      // NOTE: Do NOT delete items automatically - that breaks ownership model!
      // User code must call napi_delete_reference to delete references they
      // own. Only call finalize() to perform cleanup operations.
    });
  }
};

// Matching to Node.js code:
// [X] static New                  ==> static create
// [X] ~Reference()                ==> ~NodeApiReference()
// [X] Ref()                       ==> incRefCount()
// [X] Unref()                     ==> decRefCount
// [X] Get()                       ==> value()
// [X] ResetFinalizer()            ==> resetFinalizer()
// [X] Data()                      ==> nativeData()
// [X] refcount()                  ==> refCount()
// [X] ownership()                 ==> ownership()
// [X] ctor Reference              ==> ctor NodeApiReference
// [X] virtual CallUserFinalizer() ==> callUserFinalizer()
// [X] InvokeFinalizerFromGC()     ==> invokeFinalizerFromGC()
// [ ] static WeakCallback         ==> // Find a way to associate with JSObject
//                                     // the callback
// [ ] SetWeak()                   ==> convertToWeakRootStorage()
// [X] Finalize()                  ==> finalize()

// Base reference wrapper around Hermes values with GC integration
class NodeApiReference : public NodeApiRefTracker {
  friend class NodeApiFinalizerHolder; // Allow access to protected
                                       // invokeFinalizerFromGC()
 public:
  static NodeApiReference *create(
      NodeApiEnvironment &env,
      const vm::PinnedHermesValue *value,
      uint32_t initialRefCount,
      NodeApiReferenceOwnership ownership) noexcept {
    // Make sure GC does not collect the value while we create the reference.
    vm::GCScope scope{env.runtime()};
    // vm::Handle<vm::HermesValue> handleValue =
    // env.runtime().makeHandle(*value);

    NodeApiReference *reference = new (std::nothrow)
        NodeApiReference(env, value, initialRefCount, ownership);
    env.addReference(reference);
    return reference;
  }

  ~NodeApiReference() noexcept {
    resetStorage();
  }

  uint32_t incRefCount(NodeApiEnvironment &env) noexcept {
    // When the storage is cleared in the WeakCallback, and a second pass
    // callback is pending, return 0 unconditionally.
    if (isStorageEmpty()) {
      return 0;
    }
    // Convert from weak to strong storage when transitioning from 0 to 1
    if (++refCount_ == 1 && canBeWeak_) {
      convertToValueStorage(env);
    }
    return refCount_;
  }

  uint32_t decRefCount(NodeApiEnvironment &env) noexcept {
    // When the storage is cleared in the WeakCallback, and a second pass
    // callback is pending, return 0 unconditionally.
    if (isStorageEmpty() || refCount_ == 0) {
      return 0;
    }
    if (--refCount_ == 0) {
      convertToWeakRootStorage(env);
    }
    return refCount_;
  }

  uint32_t refCount() const noexcept {
    return refCount_;
  }

  NodeApiReferenceOwnership ownership() const noexcept {
    return ownership_;
  }

  napi_value value(NodeApiEnvironment &env) const noexcept {
    if (isStorageEmpty()) {
      return nullptr;
    } else {
      return getStorageValue(env);
    }
  }

  virtual void *nativeData() const noexcept {
    // No native data is associated with the base class.
    return nullptr;
  }

  virtual void resetFinalizer() noexcept {
    // No finalizer is set in the base class.
    // Derived classes should override this method to reset the finalizer.
  }

  // Static GC integration methods for reference scanning
  static void getGCRoots(
      NodeApiEnvironment &env,
      NodeApiReferenceList &list,
      vm::RootAcceptor &acceptor) noexcept {
    list.forEach([&](NodeApiReference *ref) {
      if (!ref->isUsingWeakStorage_ && !ref->value_.isEmpty()) {
        acceptor.accept(const_cast<vm::PinnedHermesValue &>(ref->value_));
      }
    });
  }

  static void getGCWeakRoots(
      NodeApiEnvironment &env,
      NodeApiReferenceList &list,
      vm::WeakRootAcceptor &acceptor) noexcept {
    list.forEach([&](NodeApiReference *ref) {
      if (ref->isUsingWeakStorage_ && ref->weakRoot_) {
        acceptor.acceptWeak(ref->weakRoot_);
      }
    });
  }

 protected:
  NodeApiReference(
      NodeApiEnvironment &env,
      const vm::PinnedHermesValue *value,
      uint32_t initialRefCount,
      NodeApiReferenceOwnership ownership) noexcept
      : NodeApiRefTracker(),
        value_(*value),
        refCount_(initialRefCount),
        ownership_(ownership),
        canBeWeak_(сanBeHeldWeakly(value)) {
    if (refCount_ == 0) {
      convertToWeakRootStorage(env);
    }
  }

  virtual void callUserFinalizer() noexcept {
    // Do nothing because the finalizer is not set in this class.
  }

  virtual void invokeFinalizerFromGC() noexcept {
    // Call resetStorage to clean up finalizerHolder_ backlink
    resetStorage();

    // Run the finalizer immediately because there is no finalizerCallback.
    finalize();
  }

  void setFinalizerHolder(NodeApiFinalizerHolder *holder) noexcept {
    finalizerHolder_ = holder;
  }

  void clearFinalizerHolder() noexcept {
    finalizerHolder_ = nullptr;
  }

  void resetStorage() noexcept {
    isUsingWeakStorage_ = false;
    new (&value_) vm::PinnedHermesValue(vm::HermesValue::encodeEmptyValue());

    // Remove from finalizerHolder_ before setting to null
    if (finalizerHolder_ != nullptr) {
      finalizerHolder_->removeFinalizer(this);
      finalizerHolder_ = nullptr;
    }
  }

 private:
  void finalize() noexcept override final {
    // Unconditionally reset the persistent handle so that no weak callback
    // will be invoked again.
    resetStorage();

    // If the Reference is not NodeApiReferenceOwnership::kRuntime, userland
    // code should delete it. Delete it if it is
    // NodeApiReferenceOwnership::kRuntime.
    bool deleteMe = ownership_ == NodeApiReferenceOwnership::kRuntime;

    // Whether the Reference is going to be deleted in the finalizeCallback
    // or not, it should be removed from the tracked list.
    unlink();

    // If the finalizeCallback is present, it should either delete the
    // derived NodeApiReference, or the NodeApiReference ownership was set to
    // NodeApiReferenceOwnership::kRuntime and the deleteMe parameter is true.
    callUserFinalizer();

    if (deleteMe) {
      delete this;
    }
  }

  static bool сanBeHeldWeakly(const vm::PinnedHermesValue *value) {
    // Note that Hermes does not support weak symbols yet.
    return value != nullptr && value->isObject();
  }

  // Functions that simulate the V8 Persistent storage used in Node-API for
  // Node.js. Note that the current implementation uses a union of
  // vm::PinnedHermesValue and vm::WeakRoot<vm::JSObject> to store the value.
  // The vm::PinnedHermesValue is used for strong references, and
  // vm::WeakRoot<vm::JSObject> is used for weak references.
  // The isUsingWeakStorage_ flag indicates which storage is currently active.
 private:
  // To ensure that we do not have to call destructors for the storage.
  static_assert(std::is_trivially_destructible_v<vm::PinnedHermesValue>);
  static_assert(std::is_trivially_destructible_v<vm::WeakRoot<vm::JSObject>>);

  bool isStorageEmpty() const noexcept {
    return !isUsingWeakStorage_ && value_.isEmpty();
  }

  void convertToValueStorage(NodeApiEnvironment &env) noexcept {
    if (!isUsingWeakStorage_) {
      return; // It is already a value storage.
    }
    ABORT_IF_FALSE(canBeWeak_ && "This value cannot use the weak storage.");

    // Remove from finalizerHolder_ when becoming strong (O(1) removal)
    if (finalizerHolder_ != nullptr) {
      finalizerHolder_->removeFinalizer(this);
      // Keep the finalizerHolder_ cached in case if we need it again.
    }

    // Lock the weak root to get the current value before destroying it
    vm::JSObject *lockedObject =
        weakRoot_.get(env.runtime(), env.runtime().getHeap());

    // Create strong storage in the same memory location
    if (lockedObject != nullptr) {
      new (&value_) vm::PinnedHermesValue(
          vm::HermesValue::encodeObjectValue(lockedObject));
    } else {
      // The object was already collected.
      new (&value_) vm::PinnedHermesValue(vm::HermesValue::encodeEmptyValue());
    }
    isUsingWeakStorage_ = false;
  }

  void convertToWeakRootStorage(NodeApiEnvironment &env) noexcept {
    if (isUsingWeakStorage_) {
      return; // It is already a weak storage.
    }

    if (isStorageEmpty()) {
      // If the storage is empty, we cannot convert it to weak storage.
      // This can happen if the value was already collected or reset.
      return;
    }

    if (!canBeWeak_) {
      // Cannot use weak storage for this value type. Reset it instead.
      resetStorage();
      return;
    }

    ABORT_IF_FALSE(value_.isObject() && "Expected an Object");

    // Check if finalizerHolder_ is not null and add directly if possible
    if (finalizerHolder_ != nullptr) {
      // Reuse existing finalizer holder - add this reference directly
      finalizerHolder_->addFinalizer(this);
    } else {
      // Call addObjectFinalizer to get the NodeApiFinalizerHolder and set
      // backlink
      NodeApiExternalValue *externalValue = nullptr;
      if (env.addObjectFinalizer(&value_, this, &externalValue) == napi_ok &&
          externalValue) {
        finalizerHolder_ = externalValue->getFinalizerHolder();
      }
    }

    // The simple case: the value is an object, just create a weak root for it
    new (&weakRoot_) vm::WeakRoot<vm::JSObject>(
        vm::vmcast<vm::JSObject>(value_), env.runtime());
    isUsingWeakStorage_ = true;
  }

  napi_value getStorageValue(NodeApiEnvironment &env) const noexcept {
    vm::PinnedHermesValue rawValue;

    if (isUsingWeakStorage_) {
      vm::JSObject *lockedObject =
          weakRoot_.get(env.runtime(), env.runtime().getHeap());
      if (lockedObject == nullptr) {
        // TODO: Should we return an empty value here?
        // The weakly referenced object has been collected; return undefined.
        return napiValue(&env.UndefinedHermesValue);
      }
      rawValue = vm::HermesValue::encodeObjectValue(lockedObject);
    } else {
      rawValue = value_;
    }

    return env.pushNewNodeApiValue(rawValue);
  }

  // This field order enables more compact class layout and better
  // memory alignment.
 private:
  // Keep either strong or weak value root in the union to optimize storage
  union {
    vm::PinnedHermesValue value_; // Strong reference storage
    vm::WeakRoot<vm::JSObject> weakRoot_; // Weak reference storage
  };

  // Backlink pointer to finalizer holder for efficient removal during
  // strong/weak transitions
  NodeApiFinalizerHolder *finalizerHolder_{nullptr};

  uint32_t refCount_{0};
  // Track which union member is currently active
  bool isUsingWeakStorage_{false};
  // Who is responsible for deleting the reference
  const NodeApiReferenceOwnership ownership_{
      NodeApiReferenceOwnership::kRuntime};
  // Whether this value can be stored as a weak reference
  const bool canBeWeak_{false};
};

// Final class for references that store additional native data
class NodeApiReferenceWithData final : public NodeApiReference {
 public:
  static NodeApiReferenceWithData *create(
      NodeApiEnvironment &env,
      const vm::PinnedHermesValue *value,
      uint32_t initialRefCount,
      NodeApiReferenceOwnership ownership,
      void *nativeData) noexcept {
    NodeApiReferenceWithData *reference =
        new (std::nothrow) NodeApiReferenceWithData(
            env, value, initialRefCount, ownership, nativeData);
    env.addReference(reference);
    return reference;
  }

  // Get the stored native data
  void *nativeData() const noexcept override {
    return nativeData_;
  }

  // Set native data
  void setNativeData(void *data) noexcept {
    nativeData_ = data;
  }

 private:
  NodeApiReferenceWithData(
      NodeApiEnvironment &env,
      const vm::PinnedHermesValue *value,
      uint32_t initialRefCount,
      NodeApiReferenceOwnership ownership,
      void *nativeData) noexcept
      : NodeApiReference(env, value, initialRefCount, ownership),
        nativeData_(nativeData) {}

 private:
  void *nativeData_{nullptr};
};

// Final class for references with finalizer callbacks
class NodeApiReferenceWithFinalizer final : public NodeApiReference {
 public:
  static NodeApiReferenceWithFinalizer *create(
      NodeApiEnvironment &env,
      const vm::PinnedHermesValue *value,
      uint32_t initialRefCount,
      NodeApiReferenceOwnership ownership,
      void *nativeData,
      napi_finalize finalizeCallback,
      void *finalizeHint) noexcept {
    NodeApiReferenceWithFinalizer *reference =
        new (std::nothrow) NodeApiReferenceWithFinalizer(
            env,
            value,
            initialRefCount,
            ownership,
            nativeData,
            finalizeCallback,
            finalizeHint);
    env.addFinalizingReference(reference);
    return reference;
  }

  ~NodeApiReferenceWithFinalizer() override {
    // Try to remove the finalizer from the task runner finalizer queue.
    env_.dequeueFinalizer(this);
  }

  // Override to call user finalizer
  void callUserFinalizer() noexcept override {
    finalizer_.callFinalizer(env_);
  }

  void invokeFinalizerFromGC() noexcept override {
    // Call resetStorage to clean up finalizerHolder_ backlink
    resetStorage();

    // Delegate to environment's method which has proper access to apiVersion_
    env_.invokeFinalizerFromGC(this);
  }

  // Get the composed finalizer object
  const NodeApiFinalizer &getFinalizer() const noexcept {
    return finalizer_;
  }

  // Get the stored native data
  void *nativeData() const noexcept override {
    return finalizer_.getNativeData();
  }

 private:
  NodeApiReferenceWithFinalizer(
      NodeApiEnvironment &env,
      const vm::PinnedHermesValue *value,
      uint32_t initialRefCount,
      NodeApiReferenceOwnership ownership,
      void *nativeData,
      napi_finalize finalizeCallback,
      void *finalizeHint) noexcept
      : NodeApiReference(env, value, initialRefCount, ownership),
        env_(env),
        finalizer_(nativeData, finalizeCallback, finalizeHint) {}

 private:
  NodeApiEnvironment &env_;
  NodeApiFinalizer finalizer_;
};

// Final class for standalone finalizer tracking (not tied to a specific
// reference)
class NodeApiTrackedFinalizer final : public NodeApiRefTracker {
 public:
  static NodeApiTrackedFinalizer *create(
      NodeApiEnvironment &env,
      void *nativeData,
      napi_finalize finalizeCallback,
      void *finalizeHint) noexcept {
    NodeApiTrackedFinalizer *finalizer =
        new (std::nothrow) NodeApiTrackedFinalizer(
            env, nativeData, finalizeCallback, finalizeHint);
    return finalizer;
  }

  void *nativeData() const noexcept {
    return finalizer_.getNativeData();
  }

  // When a TrackedFinalizer is being deleted, it may have been queued to call
  // its finalizer.
  ~NodeApiTrackedFinalizer() override {
    // Remove from the env's tracked list.
    unlink();
    // Try to remove the finalizer from the scheduled second pass callback.
    env_.dequeueFinalizer(this);
  }

  void finalize() noexcept override {
    unlink();
    finalizer_.callFinalizer(env_);
    delete this;
  }

 private:
  NodeApiTrackedFinalizer(
      NodeApiEnvironment &env,
      void *nativeData,
      napi_finalize finalizeCallback,
      void *finalizeHint) noexcept
      : NodeApiRefTracker(),
        env_(env),
        finalizer_(nativeData, finalizeCallback, finalizeHint) {}

 private:
  NodeApiEnvironment &env_;
  NodeApiFinalizer finalizer_;
};

//=============================================================================
// NodeApiFinalizerHolder Implementation (after NodeApiReference is complete)
//=============================================================================

void NodeApiFinalizerHolder::invokeAllFinalizers() noexcept {
  while (!finalizers_.empty()) {
    const auto &ref = finalizers_.begin();
    (*ref)->invokeFinalizerFromGC(); // Delegates to NodeApiEnvironment for API
                                     // version-dependent handling
  }
}

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
    if (it != items_.end() && *it == value) {
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
    CHECK_STATUS(env.checkExecutionStatus(res.getStatus()));
    *result = *res;
    return env.clearLastNativeError();
  }

 private:
  std::string str_;
  llvh::raw_string_ostream stream_;
};

// The external buffer that implements hermes::Buffer
// This class integrates with the Node-API finalizer infrastructure to ensure
// user finalizers are called on the JS thread when the buffer is garbage
// collected.
class NodeApiExternalBuffer final : public hermes::Buffer {
 public:
  NodeApiExternalBuffer(
      NodeApiEnvironment &env,
      void *bufferData,
      size_t bufferSize,
      napi_finalize finalizeCallback,
      void *finalizeHint) noexcept
      : Buffer(reinterpret_cast<uint8_t *>(bufferData), bufferSize),
        pendingFinalizers_(env.pendingFinalizers()),
        finalizer_(nullptr) {
    // Only create finalizer if callback is provided
    if (finalizeCallback != nullptr) {
      vm::PinnedHermesValue empty(vm::HermesValue::encodeEmptyValue());
      finalizer_ = NodeApiReferenceWithFinalizer::create(
          env,
          &empty, // No JS value for external buffers
          0, // Initial ref count (no strong reference needed)
          NodeApiReferenceOwnership::kRuntime, // Runtime-owned
          bufferData, // Native data
          finalizeCallback, // User finalizer
          finalizeHint);
    }
  }

  ~NodeApiExternalBuffer() noexcept override {
    // Called from GC thread via Hermes lambda - must be thread-safe
    // Follow the same pattern as NodeApiExternalValue destructor
    if (finalizer_) {
      // Create a temporary finalizer holder to transfer the single finalizer
      std::unique_ptr<NodeApiFinalizerHolder> finalizerHolder =
          std::make_unique<NodeApiFinalizerHolder>();
      finalizerHolder->addFinalizer(finalizer_);

      // Transfer finalizer holder to pending finalizers for thread-safe
      // processing
      pendingFinalizers_->addFinalizerHolder(std::move(finalizerHolder));

      finalizer_ = nullptr;
    }
  }

  NodeApiExternalBuffer(const NodeApiExternalBuffer &) = delete;
  NodeApiExternalBuffer &operator=(const NodeApiExternalBuffer &) = delete;

 private:
  NodeApiRefCountedPtr<NodeApiPendingFinalizers> pendingFinalizers_;
  NodeApiReferenceWithFinalizer *finalizer_;
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
    int32_t exponent = getExponent(u64);
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

  static int32_t getSign(uint64_t u64) noexcept {
    return (u64 & kSignMask) == 0 ? 1 : -1;
  }

  static int32_t getExponent(uint64_t u64) noexcept {
    int32_t biased_e =
        static_cast<int32_t>((u64 & kExponentMask) >> kPhysicalSignificandSize);
    return biased_e - kExponentBias;
  }

  static uint64_t getSignificand(uint64_t u64) noexcept {
    return (u64 & kSignificandMask) + kHiddenBit;
  }

  static constexpr uint64_t kSignMask = 0x8000'0000'0000'0000;
  static constexpr uint64_t kExponentMask = 0x7FF0'0000'0000'0000;
  static constexpr uint64_t kSignificandMask = 0x000F'FFFF'FFFF'FFFF;
  static constexpr uint64_t kHiddenBit = 0x0010'0000'0000'0000;
  static constexpr int32_t kPhysicalSignificandSize = 52;
  static constexpr int32_t kSignificandSize = 53;
  static constexpr int32_t kExponentBias = 0x3FF + kPhysicalSignificandSize;
};

class NodeApiEnvironmentHolder {
 public:
  NodeApiEnvironment *getOrCreateEnvironment(
      vm::Runtime &runtime,
      hbc::CompileFlags compileFlags,
      std::shared_ptr<TaskRunner> taskRunner,
      const std::function<void(napi_env, napi_value)> &unhandledErrorCallback,
      int32_t apiVersion) noexcept {
    if (rootEnv_ == nullptr) {
      rootEnv_ = NodeApiRefCountedPtr<NodeApiEnvironment>{
          new NodeApiEnvironment(
              runtime,
              compileFlags,
              std::move(taskRunner),
              unhandledErrorCallback,
              apiVersion),
          attachTag};
    }
    return rootEnv_.get();
  }

  NodeApiEnvironment *rootEnv() const noexcept {
    return rootEnv_.get();
  }

  NodeApiEnvironment *createModuleEnvironment(int32_t apiVersion) noexcept {
    ABORT_IF_FALSE(rootEnv_ != nullptr);
    NodeApiRefCountedPtr<NodeApiEnvironment> moduleEnv{
        new NodeApiEnvironment(
            rootEnv_->runtime(),
            rootEnv_->compileFlags(),
            rootEnv_->taskRunner(),
            rootEnv_->unhandledErrorCallback(),
            apiVersion,
            rootEnv_->pendingFinalizers()),
        attachTag};
    NodeApiEnvironment *result = moduleEnv.get();
    result->setParentEnvironment(rootEnv_.get());
    moduleEnvs_.push_back(std::move(moduleEnv));
    return result;
  }

  static vm::CallResult<NodeApiEnvironmentHolder *> fromRuntime(
      vm::Runtime &runtime) {
    vm::GCScope gcScope{runtime};
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
    // Set termination flag before deleting environments
    NodeApiEnvironmentHolder *holder =
        reinterpret_cast<NodeApiEnvironmentHolder *>(ns->context());

    // Notify all environments of runtime termination
    if (holder->rootEnv_) {
      holder->rootEnv_->isTerminatedOrTerminating_ = true;
    }
    for (auto &moduleEnv : holder->moduleEnvs_) {
      if (moduleEnv) {
        moduleEnv->isTerminatedOrTerminating_ = true;
      }
    }

    delete holder;
  }

 private:
  NodeApiRefCountedPtr<NodeApiEnvironment> rootEnv_;
  std::vector<NodeApiRefCountedPtr<NodeApiEnvironment>> moduleEnvs_;
};

} // namespace hermes::node_api

// To simplify use of the internals APIs in Node-API implementation.
using namespace hermes;
using namespace hermes::node_api;

// To reduce requirements to cast between napi_env and NodeApiEnvironment.
struct napi_env__ : NodeApiEnvironment {};

namespace hermes::node_api {

vm::CallResult<napi_env> getOrCreateNodeApiEnvironment(
    vm::Runtime &runtime,
    hbc::CompileFlags compileFlags,
    std::shared_ptr<TaskRunner> taskRunner,
    const std::function<void(napi_env, napi_value)> &unhandledErrorCallback,
    int32_t apiVersion) noexcept {
  vm::CallResult<NodeApiEnvironmentHolder *> holderRes =
      NodeApiEnvironmentHolder::fromRuntime(runtime);
  if (holderRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  return napiEnv((*holderRes)
                     ->getOrCreateEnvironment(
                         runtime,
                         compileFlags,
                         std::move(taskRunner),
                         unhandledErrorCallback,
                         apiVersion));
}

napi_status initializeNodeApiModule(
    vm::Runtime &runtime,
    napi_addon_register_func registerModule,
    int32_t apiVersion,
    napi_value *exports) noexcept {
  vm::CallResult<NodeApiEnvironmentHolder *> holderRes =
      NodeApiEnvironmentHolder::fromRuntime(runtime);
  if (holderRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return napi_pending_exception;
  }
  NodeApiEnvironmentHolder *holder = *holderRes;
  NodeApiEnvironment *moduleEnv = holder->createModuleEnvironment(apiVersion);
  return holder->rootEnv()->initializeModule(
      *moduleEnv, registerModule, exports);
}

napi_status checkNodeApiPreconditions(napi_env env) noexcept {
  return checkJSPreconditions(env);
}

napi_status setNodeApiValue(
    napi_env env,
    vm::CallResult<vm::HermesValue> hvResult,
    napi_value *result) {
  CHECK_STATUS(env->checkExecutionStatus(hvResult.getStatus()));
  return env->makeResultValue(*hvResult, result);
}

napi_status checkJSErrorStatus(
    napi_env env,
    vm::ExecutionStatus hermesStatus) noexcept {
  return CHECKED_ENV(env)->checkExecutionStatus(hermesStatus);
}

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
  return static_cast<napi_env>(env);
}

napi_finalize basicFinalize(node_api_basic_finalize finalize) noexcept {
  return reinterpret_cast<napi_finalize>(reinterpret_cast<void *>(finalize));
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

template <typename T>
vm::Handle<T> asHandle(napi_value value) noexcept {
  return vm::Handle<T>::vmcast(phv(value));
}

template <typename T>
vm::Handle<T> asHandle(const vm::PinnedHermesValue *value) noexcept {
  return vm::Handle<T>::vmcast(value);
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
      *curBuf++ = static_cast<char>(c);
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
    hbc::CompileFlags compileFlags,
    std::shared_ptr<TaskRunner> taskRunner,
    std::function<void(napi_env, napi_value)> unhandledErrorCallback,
    int32_t apiVersion,
    const NodeApiRefCountedPtr<NodeApiPendingFinalizers>
        &pendingFinalizers) noexcept
    : pendingFinalizers_(pendingFinalizers),
      runtime_(runtime),
      apiVersion_(apiVersion),
      compileFlags_(compileFlags),
      taskRunner_(std::move(taskRunner)),
      unhandledErrorCallback_(unhandledErrorCallback) {
  runtime_.addCustomRootsFunction([this](vm::GC *, vm::RootAcceptor &acceptor) {
    napiValueStack_.forEach([&](const vm::PinnedHermesValue &value) {
      acceptor.accept(const_cast<vm::PinnedHermesValue &>(value));
    });
    // Enable GC integration for Node-API references
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
  });
  // Enable weak GC integration for Node-API references
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
              runtime_, vm::createASCIIRef("Promise"))));
  setPredefinedProperty(
      NodeApiPredefined::allRejections,
      vm::HermesValue::encodeSymbolValue(
          runtime_.getIdentifierTable().registerLazyIdentifier(
              runtime_, vm::createASCIIRef("allRejections"))));
  setPredefinedProperty(
      NodeApiPredefined::code,
      vm::HermesValue::encodeSymbolValue(
          runtime_.getIdentifierTable().registerLazyIdentifier(
              runtime_, vm::createASCIIRef("code"))));
  setPredefinedProperty(
      NodeApiPredefined::hostFunction,
      vm::HermesValue::encodeSymbolValue(
          runtime_.getIdentifierTable().registerLazyIdentifier(
              runtime_, vm::createASCIIRef("hostFunction"))));
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
              runtime_, vm::createASCIIRef("onHandled"))));
  setPredefinedProperty(
      NodeApiPredefined::onUnhandled,
      vm::HermesValue::encodeSymbolValue(
          runtime_.getIdentifierTable().registerLazyIdentifier(
              runtime_, vm::createASCIIRef("onUnhandled"))));
  setPredefinedProperty(
      NodeApiPredefined::reject,
      vm::HermesValue::encodeSymbolValue(
          runtime_.getIdentifierTable().registerLazyIdentifier(
              runtime_, vm::createASCIIRef("reject"))));
  setPredefinedProperty(
      NodeApiPredefined::resolve,
      vm::HermesValue::encodeSymbolValue(
          runtime_.getIdentifierTable().registerLazyIdentifier(
              runtime_, vm::createASCIIRef("resolve"))));
}

NodeApiEnvironment::~NodeApiEnvironment() = default;

napi_status NodeApiEnvironment::incRefCount() noexcept {
  refCount_++;
  return napi_status::napi_ok;
}

napi_status NodeApiEnvironment::decRefCount() noexcept {
  if (--refCount_ == 0) {
    deleteMe();
  }
  return napi_status::napi_ok;
}

// Node.js-style controlled shutdown with finalizer drainage
void NodeApiEnvironment::deleteMe() noexcept {
  // Set shutdown flag to prevent task runner scheduling BEFORE processing
  // finalizers
  isShuttingDown_ = true;

  // Drain all pending finalizers immediately - we cannot rely on task runner
  // during shutdown as the environment is being destroyed

  // 1. Process any pending GC-triggered finalizers
  processPendingFinalizers();

  // 2. Drain task runner finalizer queue immediately (no async processing)
  drainFinalizerQueue();

  // 3. Clean up instance data if present
  if (instanceData_ != nullptr) {
    instanceData_->unlink();
  }

  // 4. Restore proper cleanup sequence from destructor (lines 3667-3690)
  // First we must finalize those references that have `napi_finalizer`
  // callbacks. The reason is that addons might store other references which
  // they delete during their `napi_finalizer` callbacks. If we deleted such
  // references here first, they would be doubly deleted when the
  // `napi_finalizer` deleted them subsequently.

  // Finalize all tracked finalizers in the task runner queue
  NodeApiRefTracker::finalizeAll(finalizingReferences_);
  if (instanceData_ != nullptr) {
    instanceData_->finalize();
    instanceData_ = nullptr;
  }
  NodeApiRefTracker::finalizeAll(references_);

  ABORT_IF_FALSE(taskRunnerFinalizerQueue_.empty());
  ABORT_IF_FALSE(finalizingReferences_.isEmpty());
  ABORT_IF_FALSE(references_.isEmpty());

  // 5. Now safe to delete - all finalizers have been executed
  delete this;
}

vm::Runtime &NodeApiEnvironment::runtime() noexcept {
  return runtime_;
}

hbc::CompileFlags NodeApiEnvironment::compileFlags() noexcept {
  return compileFlags_;
}

const std::shared_ptr<TaskRunner> &NodeApiEnvironment::taskRunner()
    const noexcept {
  return taskRunner_;
}

const std::function<void(napi_env, napi_value)> &
NodeApiEnvironment::unhandledErrorCallback() const noexcept {
  return unhandledErrorCallback_;
}

const NodeApiRefCountedPtr<NodeApiPendingFinalizers> &
NodeApiEnvironment::pendingFinalizers() const noexcept {
  return pendingFinalizers_;
}

NodeApiStableAddressStack<vm::PinnedHermesValue> &
NodeApiEnvironment::napiValueStack() noexcept {
  return napiValueStack_;
}

vm::CallResult<vm::HermesValue> NodeApiEnvironment::callModuleInitializer(
    napi_addon_register_func registerModule) noexcept {
  NodeApiValueScope scope{*this};
  vm::GCScope gcScope(runtime_);
  napi_value exports{};
  ABORT_IF_FALSE(napi_create_object(napiEnv(this), &exports) == napi_ok);
  vm::ExecutionStatus status = callIntoModule(
      [&](NodeApiEnvironment *env) {
        napi_value returned_exports = registerModule(napiEnv(env), exports);
        if (returned_exports != nullptr && returned_exports != exports) {
          exports = returned_exports;
        }
      },
      rethrowException);

  if (status == vm::ExecutionStatus::EXCEPTION) {
    return vm::ExecutionStatus::EXCEPTION;
  }

  if (exports != nullptr) {
    return *phv(exports);
  } else {
    return UndefinedHermesValue;
  }
}

napi_status NodeApiEnvironment::initializeModule(
    NodeApiEnvironment &moduleEnv,
    napi_addon_register_func registerModule,
    napi_value *exports) noexcept {
  // Each native module must have its own napi_env.
  // The tricky part is that we must call the registerModule in the
  // module specific Node-API environment, not the current one.
  // Then, we must translate the exports object or the error to the current
  // environment.
  CHECK_STATUS(checkJSPreconditions(napiEnv(this)));
  CHECK_ARG(exports);
  NodeApiEscapableValueScope scope{*this};
  vm::CallResult<vm::HermesValue> res =
      moduleEnv.callModuleInitializer(registerModule);
  CHECK_STATUS(checkExecutionStatus(res.getStatus()));
  return scope.escapeResult(*res, exports);
}

void NodeApiEnvironment::setParentEnvironment(
    NodeApiEnvironment *parentEnvironment) noexcept {
  parentEnvironment_ = parentEnvironment;
}

napi_status NodeApiEnvironment::setLastNativeError(
    napi_status status,
    const char *fileName,
    uint32_t line,
    const std::string &message) noexcept {
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
      "External buffers are not allowed",
      "Cannot run JavaScript",
  };

  // The value of the constant below must be updated to reference the last
  // message in the `napi_status` enum each time a new error message is added.
  // We don't have a napi_status_last as this would result in an ABI
  // change each time a message was added.
  const int32_t lastStatus = napi_cannot_run_js;
  static_assert(
      size(errorMessages) == lastStatus + 1,
      "Count of error messages must match count of error values");

  if (status < napi_ok || status > lastStatus) {
    status = napi_generic_failure;
  }

  lastErrorMessage_.clear();
  NodeApiStringBuilder sb{
      NodeApiStringBuilder::AdoptString, std::move(lastErrorMessage_)};
  sb.append(errorMessages[status]);
  if (!message.empty()) {
    sb.append(": ", std::move(message));
  }
  sb.append("\nFile: ", fileName);
  sb.append("\nLine: ", line);
  lastErrorMessage_ = std::move(sb.str());
  // TODO: Find a better way to provide the extended error message
  lastError_ = {errorMessages[status], 0, 0, status};

  return status;
}

napi_status NodeApiEnvironment::clearLastNativeError() noexcept {
  if (LLVM_UNLIKELY(lastError_.error_code != napi_ok)) {
    lastError_.error_code = napi_ok;
  }
  return napi_ok;
}

//-----------------------------------------------------------------------------
// Methods to support JS error handling
//-----------------------------------------------------------------------------

static napi_status createJSError(
    napi_env env,
    const vm::PinnedValue<vm::JSObject> &errorPrototype,
    napi_value code,
    napi_value message,
    napi_value *result) noexcept {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  NodeApiEscapableValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};
  CHECK_ARG_IS_STRING(message);
  CHECK_ARG(result);
  vm::Handle<vm::JSError> errorHandle =
      env->runtime_.makeHandle<vm::JSError>(vm::JSError::create(
          env->runtime_, vm::Handle<vm::JSObject>::vmcast(&errorPrototype)));
  CHECK_STATUS(env->checkExecutionStatus(
      vm::JSError::setMessage(errorHandle, env->runtime_, asHandle(message))));
  CHECK_STATUS(env->setJSErrorCode(errorHandle, code, nullptr));
  return scope.escapeResult(errorHandle.getHermesValue(), result);
}

static napi_status throwJSError(
    napi_env env,
    const vm::PinnedValue<vm::JSObject> &errorPrototype,
    const char *code,
    const char *message) noexcept {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  NodeApiValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};

  napi_value messageValue;
  CHECK_STATUS(
      napi_create_string_utf8(env, message, NAPI_AUTO_LENGTH, &messageValue));

  vm::Handle<vm::JSError> errorHandle =
      env->runtime_.makeHandle(vm::JSError::create(
          env->runtime_, vm::Handle<vm::JSObject>::vmcast(&errorPrototype)));
  CHECK_STATUS(env->checkExecutionStatus(
      vm::JSError::recordStackTrace(errorHandle, env->runtime_)));
  CHECK_STATUS(env->checkExecutionStatus(vm::JSError::setMessage(
      errorHandle, env->runtime_, asHandle(messageValue))));
  CHECK_STATUS(env->setJSErrorCode(errorHandle, nullptr, code));

  env->thrownJSError_ = errorHandle.getHermesValue();

  // any VM calls after this point and before returning
  // to the JavaScript invoker will fail
  return env->clearLastNativeError();
}

napi_status NodeApiEnvironment::setJSErrorCode(
    vm::Handle<vm::JSError> error,
    napi_value code,
    const char *codeCString) noexcept {
  if (code || codeCString) {
    if (code) {
      CHECK_ARG_IS_STRING(code);
    } else {
      CHECK_STATUS(napi_create_string_utf8(
          napiEnv(this), codeCString, NAPI_AUTO_LENGTH, &code));
    }
    return setPredefinedProperty(
        vm::Handle<vm::JSObject>::vmcast(error),
        NodeApiPredefined::code,
        asHandle(code),
        nullptr);
  }
  return napi_ok;
}

//-----------------------------------------------------------------------------
// Methods to support catching JS exceptions
//-----------------------------------------------------------------------------

enum class NodeApiPendingExceptionCheck {
  kCheck,
  kSkip,
};

// To check Node-API function that do not touch GC state.
template <
    NodeApiPendingExceptionCheck pendingExceptionCheck =
        NodeApiPendingExceptionCheck::kCheck>
static napi_status checkBasicPreconditions(napi_env env) noexcept {
  CHECK_ENV(env);
  if constexpr (pendingExceptionCheck == NodeApiPendingExceptionCheck::kCheck) {
    RETURN_STATUS_IF_FALSE(
        env->thrownJSError_.isEmpty(), napi_pending_exception);
  }
  return napi_ok;
}

// TODO: Can we remove this method?
// Aborts process with a disallowed GC access message.
[[noreturn]] static void abortOnDisallowedGCAccess() noexcept {
  std::cerr << "Finalizer is calling a function that may affect GC state.\n"
               "The finalizers are run directly from GC and must not affect "
               "GC state.\n"
               "Use `node_api_post_finalizer` from inside of the finalizer to "
               "work around this issue.\n"
               "It schedules the call as a new task in the event loop.";
  ABORT();
}

// To check Node-API function preconditions that may touch GC state.
template <
    NodeApiPendingExceptionCheck pendingExceptionCheck =
        NodeApiPendingExceptionCheck::kCheck>
static napi_status checkGCPreconditions(napi_env env) noexcept {
  CHECK_STATUS(checkBasicPreconditions<pendingExceptionCheck>(env));
  if (env->inGCFinalizer_) {
    abortOnDisallowedGCAccess();
  }
  return napi_ok;
}

// To check Node-API function preconditions that may call JavaScript.
static napi_status checkJSPreconditions(napi_env env) noexcept {
  CHECK_STATUS(checkGCPreconditions(env));
  RETURN_STATUS_IF_FALSE(
      !env->isShuttingDown_ && !env->isTerminatedOrTerminating_,
      env->apiVersion_ >= 10 ? napi_cannot_run_js : napi_pending_exception);
  return napi_ok;
}

napi_status NodeApiEnvironment::checkExecutionStatus(
    vm::ExecutionStatus hermesStatus) noexcept {
  if (LLVM_LIKELY(hermesStatus != vm::ExecutionStatus::EXCEPTION)) {
    return napi_ok;
  }

  thrownJSError_ = runtime_.getThrownValue();
  runtime_.clearThrownValue();
  if (!thrownJSError_.isEmpty()) {
    return ERROR_STATUS(napi_pending_exception, "Exception pending");
  }
  return ERROR_STATUS(napi_generic_failure, "Generic error");
}

// TODO: pass file name and line number
napi_status NodeApiEnvironment::setJSException() noexcept {
  thrownJSError_ = runtime_.getThrownValue();
  runtime_.clearThrownValue();
  if (!thrownJSError_.isEmpty()) {
    return ERROR_STATUS(napi_pending_exception);
  }
  return ERROR_STATUS(napi_generic_failure);
}

void NodeApiEnvironment::checkRuntimeThrownValue() noexcept {
  vm::HermesValue thrownValue = runtime_.getThrownValue();
  if (!thrownValue.isEmpty()) {
    thrownJSError_ = thrownValue;
    runtime_.clearThrownValue();
  }
}

napi_status NodeApiEnvironment::getUniqueSymbolID(
    const char *utf8,
    size_t length,
    vm::MutableHandle<vm::SymbolID> *result) noexcept {
  napi_value strValue;
  CHECK_STATUS(napi_create_string_utf8(napiEnv(this), utf8, length, &strValue));
  return getUniqueSymbolID(strValue, result);
}

napi_status NodeApiEnvironment::getUniqueSymbolID(
    napi_value strValue,
    vm::MutableHandle<vm::SymbolID> *result) noexcept {
  CHECK_ARG_IS_STRING(strValue);
  vm::CallResult<vm::Handle<vm::SymbolID>> res = vm::stringToSymbolID(
      runtime_, vm::createPseudoHandle(phv(strValue)->getString()));
  CHECK_STATUS(checkExecutionStatus(res.getStatus()));
  *result = *res;
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::getForInPropertyNames(
    vm::Handle<vm::JSObject> object,
    napi_key_conversion keyConversion,
    napi_value *result) noexcept {
  // Hermes optimizes retrieving property names for the 'for..in' implementation
  // by caching its results. This function takes the advantage from using it.
  uint32_t beginIndex;
  uint32_t endIndex;
  vm::CallResult<vm::Handle<vm::ArrayStorageSmall>> keyStorage =
      vm::getForInPropertyNames(runtime_, object, beginIndex, endIndex);
  CHECK_STATUS(checkExecutionStatus(keyStorage.getStatus()));
  return convertKeyStorageToArray(
      *keyStorage, beginIndex, endIndex - beginIndex, keyConversion, result);
}

napi_status NodeApiEnvironment::convertKeyStorageToArray(
    vm::Handle<vm::ArrayStorageSmall> keyStorage,
    uint32_t startIndex,
    uint32_t length,
    napi_key_conversion keyConversion,
    napi_value *result) noexcept {
  vm::CallResult<vm::PseudoHandle<vm::JSArray>> res =
      vm::JSArray::create(runtime_, length, length);
  CHECK_STATUS(checkExecutionStatus(res.getStatus()));
  vm::Handle<vm::JSArray> array = runtime_.makeHandle(std::move(*res));
  if (keyConversion == napi_key_numbers_to_strings) {
    vm::GCScopeMarkerRAII marker{runtime_};
    vm::MutableHandle<> key{runtime_};
    for (size_t i = 0; i < length; ++i) {
      key = runtime_.makeHandle(keyStorage->at(startIndex + i).toHV(runtime_));
      if (key->isNumber()) {
        CHECK_STATUS(convertIndexToString(key->getNumber(), &key));
      } else if (key->isSymbol() && key->getSymbol().isUniqued()) {
        vm::StringPrimitive *str =
            runtime_.getStringPrimFromSymbolID(key->getSymbol());
        key = vm::HermesValue::encodeStringValue(str);
      }
      vm::ExecutionStatus status =
          vm::JSArray::setElementAt(array, runtime_, i, key);
      CHECK_STATUS(checkExecutionStatus(status));
      marker.flush();
    }
  } else {
    vm::JSArray::setStorageEndIndex(array, runtime_, length);
    vm::NoAllocScope noAlloc{runtime_};
    vm::JSArray *arrPtr = array.get();
    for (uint32_t i = 0; i < length; ++i) {
      vm::SmallHermesValue key = keyStorage->at(startIndex + i);
      if (key.isSymbol() && key.getSymbol().isUniqued()) {
        vm::StringPrimitive *str =
            runtime_.getStringPrimFromSymbolID(key.getSymbol());
        vm::JSArray::unsafeSetExistingElementAt(
            arrPtr,
            runtime_,
            i,
            vm::SmallHermesValue::encodeStringValue(str, runtime_));
      } else {
        vm::JSArray::unsafeSetExistingElementAt(arrPtr, runtime_, i, key);
      }
    }
  }
  return makeResultValue(array.getHermesValue(), result);
}

napi_status NodeApiEnvironment::convertToStringKeys(
    vm::Handle<vm::JSArray> array) noexcept {
  vm::GCScopeMarkerRAII marker{runtime_};
  size_t length = vm::JSArray::getLength(array.get(), runtime_);
  for (size_t i = 0; i < length; ++i) {
    vm::HermesValue key = array->at(runtime_, i).unboxToHV(runtime_);
    if (LLVM_UNLIKELY(key.isNumber())) {
      vm::MutableHandle<> strKey{runtime_};
      CHECK_STATUS(convertIndexToString(key.getNumber(), &strKey));
      vm::ExecutionStatus status =
          vm::JSArray::setElementAt(array, runtime_, i, strKey);
      CHECK_STATUS(checkExecutionStatus(status));
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

napi_status NodeApiEnvironment::createFunction(
    vm::SymbolID name,
    napi_callback callback,
    void *callbackData,
    vm::MutableHandle<vm::Callable> *result) noexcept {
  CHECK_STATUS(checkJSPreconditions(napiEnv(this)));
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
  CHECK_STATUS(checkExecutionStatus(funcRes.getStatus()));
  context.release(); // the context is now owned by the func.
  *result = vm::vmcast<vm::Callable>(*funcRes);
  return clearLastNativeError();
}

// This error handler propagates the exception to the JavaScript
// environment, so it can be handled by the JavaScript code.
// If the environment is already terminating, it does nothing.
void NodeApiEnvironment::rethrowException(
    NodeApiEnvironment *env,
    const vm::PinnedHermesValue *error) noexcept {
  if (env->isTerminatedOrTerminating_) {
    return;
  }
  env->runtime_.setThrownValue(*error);
}

// This error handler is used when the exception cannot be handled from
// JavaScript. It triggers a fatal exception in the environment, which may lead
// to the termination of the process. If the environment is already terminating,
// it does nothing.
void NodeApiEnvironment::triggerUnhandledException(
    NodeApiEnvironment *env,
    const vm::PinnedHermesValue *error) noexcept {
  if (env->isTerminatedOrTerminating_) {
    return;
  }
  env->triggerFatalException(error);
}

template <class TLambda, class TExceptionHandler>
vm::ExecutionStatus NodeApiEnvironment::callIntoModule(
    TLambda &&call,
    TExceptionHandler &&exceptionHandler) noexcept {
  CurrentEnvironmentScope envScope(this);
  size_t openHandleScopesBefore = napiValueStackScopes_.size();
  clearLastNativeError();
  call(this);
  ABORT_IF_FALSE(openHandleScopesBefore == napiValueStackScopes_.size());

  if (!thrownJSError_.isEmpty()) {
    exceptionHandler(this, &thrownJSError_);
    thrownJSError_ = EmptyHermesValue;
  }
  return runtime_.getThrownValue().isEmpty() ? vm::ExecutionStatus::RETURNED
                                             : vm::ExecutionStatus::EXCEPTION;
}

void NodeApiEnvironment::triggerFatalException(
    const vm::PinnedHermesValue *error) noexcept {
  if (parentEnvironment_ != nullptr) {
    // The fatal exception must be triggered in the parent environment.
    parentEnvironment_->triggerFatalException(error);
    return;
  }
  unhandledErrorCallback_(napiEnv(this), napiValue(error));
}

const vm::PinnedHermesValue &NodeApiEnvironment::getPredefinedValue(
    NodeApiPredefined key) noexcept {
  return predefinedValues_[static_cast<size_t>(key)];
}

vm::SymbolID NodeApiEnvironment::getPredefinedSymbol(
    NodeApiPredefined key) noexcept {
  return getPredefinedValue(key).getSymbol();
}

napi_status NodeApiEnvironment::hasPredefinedProperty(
    vm::Handle<vm::JSObject> object,
    NodeApiPredefined key,
    bool *result) noexcept {
  return hasNamedProperty(object, getPredefinedSymbol(key), result);
}

napi_status NodeApiEnvironment::getPredefinedProperty(
    vm::Handle<vm::JSObject> object,
    NodeApiPredefined key,
    napi_value *result) noexcept {
  return getNamedProperty(object, getPredefinedSymbol(key), result);
}

napi_status NodeApiEnvironment::setPredefinedProperty(
    vm::Handle<vm::JSObject> object,
    NodeApiPredefined key,
    vm::Handle<> value,
    bool *optResult) noexcept {
  return setNamedProperty(object, getPredefinedSymbol(key), value, optResult);
}

napi_status NodeApiEnvironment::hasNamedProperty(
    vm::Handle<vm::JSObject> object,
    vm::SymbolID name,
    bool *result) noexcept {
  vm::CallResult<bool> cr = vm::JSObject::hasNamed(object, runtime_, name);
  CHECK_STATUS(checkExecutionStatus(cr.getStatus()));
  *result = *cr;
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::getNamedProperty(
    vm::Handle<vm::JSObject> object,
    vm::SymbolID name,
    napi_value *result) noexcept {
  vm::CallResult<vm::PseudoHandle<>> cr = vm::JSObject::getNamed_RJS(
      object, runtime_, name, vm::PropOpFlags().plusThrowOnError());
  CHECK_STATUS(checkExecutionStatus(cr.getStatus()));
  return makeResultValue(cr->getHermesValue(), result);
}

napi_status NodeApiEnvironment::setNamedProperty(
    vm::Handle<vm::JSObject> object,
    vm::SymbolID name,
    vm::Handle<> value,
    bool *optResult) noexcept {
  vm::CallResult<bool> cr = vm::JSObject::putNamed_RJS(
      object, runtime_, name, value, vm::PropOpFlags().plusThrowOnError());
  CHECK_STATUS(checkExecutionStatus(cr.getStatus()));
  if (optResult != nullptr) {
    *optResult = *cr;
  }
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::defineOwnProperty(
    vm::Handle<vm::JSObject> object,
    vm::SymbolID name,
    vm::DefinePropertyFlags dpFlags,
    vm::Handle<> valueOrAccessor,
    bool *result) noexcept {
  vm::CallResult<bool> cr = vm::JSObject::defineOwnProperty(
      object,
      runtime_,
      name,
      dpFlags,
      valueOrAccessor,
      vm::PropOpFlags().plusThrowOnError());
  CHECK_STATUS(checkExecutionStatus(cr.getStatus()));
  if (result != nullptr) {
    *result = *cr;
  }
  return clearLastNativeError();
}

template <NodeApiUnwrapAction action>
static napi_status
unwrapObject(napi_env env, napi_value object, void **result) noexcept {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  NodeApiValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};

  CHECK_ARG_IS_OBJECT(object);
  if constexpr (action == NodeApiUnwrapAction::KeepWrap) {
    CHECK_ARG(result);
  }

  NodeApiExternalValue *externalValue =
      env->getExternalObjectValue(*phv(object));
  if (!externalValue) {
    CHECK_STATUS(env->getExternalPropertyValue(
        asHandle<vm::JSObject>(object),
        NodeApiIfNotFound::ThenReturnNull,
        &externalValue));
    RETURN_STATUS_IF_FALSE(externalValue, napi_invalid_arg);
  }

  NodeApiReference *reference = asReference(externalValue->nativeData());
  RETURN_STATUS_IF_FALSE(reference, napi_invalid_arg);
  if (result) {
    *result = reference->nativeData();
  }

  if constexpr (action == NodeApiUnwrapAction::RemoveWrap) {
    externalValue->setNativeData(nullptr);

    if (reference->ownership() == NodeApiReferenceOwnership::kUserland) {
      // When the wrap is been removed, the finalizer should be reset.
      reference->resetFinalizer();
    } else {
      delete reference;
    }
  }

  return env->clearLastNativeError();
}

// Create the ExternalObject as a DecoratedObject with a special tag to
// distinguish it from other DecoratedObject instances.
vm::Handle<vm::DecoratedObject> NodeApiEnvironment::createExternalObject(
    void *nativeData,
    NodeApiExternalValue **externalValue) noexcept {
  vm::Handle<vm::DecoratedObject> decoratedObj =
      runtime_.makeHandle(vm::DecoratedObject::create(
          runtime_,
          vm::Handle<vm::JSObject>::vmcast(&runtime_.objectPrototype),
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
napi_status NodeApiEnvironment::getExternalPropertyValue(
    vm::Handle<vm::JSObject> object,
    NodeApiIfNotFound ifNotFound,
    NodeApiExternalValue **result) noexcept {
  NodeApiExternalValue *externalValue{};
  napi_value napiExternalValue;
  napi_status status = getPredefinedProperty(
      object, NodeApiPredefined::napi_externalValue, &napiExternalValue);
  if (status == napi_ok &&
      vm::vmisa<vm::DecoratedObject>(*phv(napiExternalValue))) {
    externalValue = getExternalObjectValue(*phv(napiExternalValue));
    RETURN_STATUS_IF_FALSE(externalValue != nullptr, napi_generic_failure);
  } else if (ifNotFound == NodeApiIfNotFound::ThenCreate) {
    vm::Handle<vm::DecoratedObject> decoratedObj =
        createExternalObject(nullptr, &externalValue);
    CHECK_STATUS(defineOwnProperty(
        object,
        getPredefinedSymbol(NodeApiPredefined::napi_externalValue),
        vm::DefinePropertyFlags::getNewNonEnumerableFlags(),
        decoratedObj,
        nullptr));
  }
  *result = externalValue;
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::addObjectFinalizer(
    const vm::PinnedHermesValue *value,
    NodeApiReference *finalizer,
    NodeApiExternalValue **result) noexcept {
  NodeApiExternalValue *externalValue = getExternalObjectValue(*value);
  if (!externalValue) {
    CHECK_STATUS(getExternalPropertyValue(
        asHandle<vm::JSObject>(value),
        NodeApiIfNotFound::ThenCreate,
        &externalValue));
  }
  externalValue->addFinalizer(finalizer);
  if (result != nullptr) {
    *result = externalValue;
  }
  return clearLastNativeError();
}

void NodeApiEnvironment::callFinalizer(
    napi_finalize finalizeCallback,
    void *nativeData,
    void *finalizeHint) noexcept {
  // Use enhanced callIntoModule with finalizer exception handler
  callIntoModule(
      [&](NodeApiEnvironment *envPtr) {
        finalizeCallback(napiEnv(this), nativeData, finalizeHint);
      },
      NodeApiEnvironment::triggerUnhandledException);
}

// Enqueue the finalizer to the task runner finalizer queue for asynchronous
// execution. Used by node_api_post_finalizer and invokeFinalizerFromGC for
// non-pure finalizers. Automatically schedules task runner processing if not
// already scheduled.
void NodeApiEnvironment::enqueueFinalizer(
    NodeApiRefTracker *finalizer) noexcept {
  // During shutdown, execute finalizers immediately
  // Cannot rely on task runner when environment is being destroyed
  if (isShuttingDown_) {
    finalizer->finalize();
    return;
  }

  taskRunnerFinalizerQueue_.emplace(finalizer);

  // Schedule task runner processing if not already scheduled
  if (!isScheduledAsyncFinalizers_) {
    isScheduledAsyncFinalizers_ = true;

    // Capture smart pointer instead of raw 'this' to avoid use after delete.
    NodeApiRefCountedPtr<NodeApiEnvironment> envPtr{this};

    taskRunner_->post(makeTask([envPtr = std::move(envPtr)]() mutable {
      // Process task runner finalizer queue asynchronously on JS thread
      // envPtr keeps the environment alive until this lambda completes
      envPtr->drainFinalizerQueue();
      envPtr->isScheduledAsyncFinalizers_ = false;
    }));
  }
}

// Remove the finalizer from the scheduled second pass weak callback queue.
// The finalizer can be deleted after this call.
void NodeApiEnvironment::dequeueFinalizer(
    NodeApiRefTracker *finalizer) noexcept {
  taskRunnerFinalizerQueue_.erase(finalizer);
}

void NodeApiEnvironment::drainFinalizerQueue() noexcept {
  // As userland code can delete additional references in a running finalizer,
  // the list of pending finalizers may be mutated as we execute them, so
  // we keep iterating it until it is empty.
  while (!taskRunnerFinalizerQueue_.empty()) {
    NodeApiRefTracker *refTracker = *taskRunnerFinalizerQueue_.begin();
    taskRunnerFinalizerQueue_.erase(refTracker);
    refTracker->finalize();
  }
}

// API version-dependent finalizer dispatch method
// For NodeApiReferenceWithFinalizer instances triggered by GC
void NodeApiEnvironment::invokeFinalizerFromGC(
    NodeApiRefTracker *finalizer) noexcept {
  // During shutdown, always execute immediately regardless of API version.
  // Cannot rely on task runner when environment is being destroyed
  if (isShuttingDown_) {
    finalizer->finalize();
    return;
  }

  if (apiVersion_ != NAPI_VERSION_EXPERIMENTAL) {
    enqueueFinalizer(finalizer);
  } else {
    // The experimental code calls finalizers immediately to release native
    // objects as soon as possible. In that state any code that may affect GC
    // state causes a fatal error. To work around this issue the finalizer code
    // can call node_api_post_finalizer.
    InGCFinalizerScope scope{this};
    finalizer->finalize();
  }
}

void NodeApiEnvironment::processPendingFinalizers() noexcept {
  if (pendingFinalizers_->hasPendingFinalizers()) {
    pendingFinalizers_->processPendingFinalizers();
  }
}

// This function processes pending finalizers from GC-triggered
// NodeApiExternalValue destructors. It handles finalizers that may be created
// in background GC threads and ensures they execute safely on the JS thread.
napi_status NodeApiEnvironment::processFinalizerQueueFromCode() noexcept {
  if (!isRunningFinalizers_) {
    isRunningFinalizers_ = true;

    // Process GC-triggered finalizers from NodeApiExternalValue destructors
    // These are handled via NodeApiPendingFinalizers -> NodeApiFinalizerHolder
    // pattern
    processPendingFinalizers();

    isRunningFinalizers_ = false;
  }
  return napi_ok;
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

napi_value NodeApiEnvironment::pushNewNodeApiValue(
    const vm::HermesValue &value) noexcept {
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
  return UndefinedHermesValue;
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
      return napi_throw_range_error(
          napiEnv(this), "ERR_NAPI_INVALID_TYPEDARRAY_ALIGNMENT", sb.c_str());
    }
  }
  if (length * elementSize + byteOffset > buffer->size()) {
    return napi_throw_range_error(
        napiEnv(this),
        "ERR_NAPI_INVALID_TYPEDARRAY_ALIGNMENT",
        "Invalid typed array length");
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

template <vm::CellKind cellKind>
/*static*/ constexpr const char *
NodeApiEnvironment::getTypedArrayName() noexcept {
#define TYPED_ARRAY(name, type)                              \
  if constexpr (cellKind == vm::CellKind::name##ArrayKind) { \
    return #name "Array";                                    \
  } else
#include "hermes/VM/TypedArrays.def"
  {
    static_assert(false, "Unexpected typed array");
  }
}

napi_status NodeApiEnvironment::createPromise(
    napi_value *promise,
    vm::MutableHandle<> *resolveFunction,
    vm::MutableHandle<> *rejectFunction) noexcept {
  napi_value global, promiseConstructor;
  CHECK_STATUS(napi_get_global(napiEnv(this), &global));
  CHECK_STATUS(getPredefinedProperty(
      asHandle<vm::JSObject>(global),
      NodeApiPredefined::Promise,
      &promiseConstructor));

  // The executor function is to be executed by the constructor during the
  // process of constructing the new Promise object. The executor is custom code
  // that ties an outcome to a promise. We return the resolveFunction and
  // rejectFunction given to the executor. Since the execution is synchronous,
  // we allocate executorData on the callstack.
  struct ExecutorData {
    static vm::CallResult<vm::HermesValue> callback(
        void *context,
        vm::Runtime &runtime) {
      vm::NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
      return (reinterpret_cast<ExecutorData *>(context))->callback(args);
    }

    vm::CallResult<vm::HermesValue> callback(const vm::NativeArgs &args) {
      *resolve = args.getArg(0);
      *reject = args.getArg(1);
      return env_->UndefinedHermesValue;
    }

    NodeApiEnvironment *env_{};
    vm::MutableHandle<> *resolve{};
    vm::MutableHandle<> *reject{};
  } executorData{this, resolveFunction, rejectFunction};

  vm::Handle<vm::NativeFunction> executorFunction = vm::NativeFunction::create(
      runtime_,
      vm::Handle<vm::JSObject>::vmcast(&runtime_.functionPrototype),
      vm::Runtime::makeNullHandle<vm::Environment>(),
      &executorData,
      &ExecutorData::callback,
      getPredefinedSymbol(NodeApiPredefined::Promise),
      /*paramCount:*/ 2,
      runtime_.makeNullHandle<vm::JSObject>());
  napi_value func = pushNewNodeApiValue(executorFunction.getHermesValue());
  return napi_new_instance(
      napiEnv(this), promiseConstructor, 1, &func, promise);
}

static napi_status concludeDeferred(
    napi_env env,
    napi_deferred deferred,
    NodeApiPredefined predefinedProperty,
    napi_value resolution) noexcept {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(deferred);
  CHECK_ARG(resolution);
  NodeApiValueScope scope{*env};

  NodeApiReference *deferredRef = asReference(deferred);

  napi_value jsDeferred = deferredRef->value(*env);
  napi_value resolver{}, callResult{};
  CHECK_STATUS(env->getPredefinedProperty(
      asHandle<vm::JSObject>(jsDeferred), predefinedProperty, &resolver));
  CHECK_STATUS(napi_call_function(
      env,
      napiValue(&env->UndefinedHermesValue),
      resolver,
      1,
      &resolution,
      &callResult));
  delete deferredRef;
  return env->clearLastNativeError();
}

//---------------------------------------------------------------------------
// Result setting helpers
//---------------------------------------------------------------------------

napi_status NodeApiEnvironment::makeResultValue(
    const vm::HermesValue &value,
    napi_value *result) noexcept {
  *result = pushNewNodeApiValue(value);
  return clearLastNativeError();
}

napi_status NodeApiEnvironment::castResult(
    const vm::PinnedHermesValue *value,
    napi_value *result) noexcept {
  *result = napiValue(value);
  return clearLastNativeError();
}

//---------------------------------------------------------------------------
// String helpers
//---------------------------------------------------------------------------

std::u16string latin1ToUtf16(const char *str, size_t length) noexcept {
  // Latin1 has the same codes as Unicode.
  // We just need to expand char to char16_t.
  std::u16string u16str(length, u'\0');
  // Cast to unsigned to avoid signed value expansion to 16 bit.
  const uint8_t *ustr = reinterpret_cast<const uint8_t *>(str);
  std::copy(ustr, ustr + length, &u16str[0]);
  return u16str;
}

//-----------------------------------------------------------------------------
// non Node-API external APIs
//-----------------------------------------------------------------------------

napi_status setNodeApiEnvironmentData(
    napi_env env,
    const napi_type_tag &tag,
    void *data) noexcept {
  CHECK_ENV(env);
  if (env->parentEnvironment_ != nullptr) {
    return setNodeApiEnvironmentData(
        napiEnv(env->parentEnvironment_), tag, data);
  }

  if (data != nullptr) {
    env->taggedData_.try_emplace(tag, data);
  } else {
    env->taggedData_.erase(tag);
  }
  return env->clearLastNativeError();
}

napi_status getNodeApiEnvironmentData(
    napi_env env,
    const napi_type_tag &tag,
    void **data) noexcept {
  CHECK_ENV(env);
  if (env->parentEnvironment_ != nullptr) {
    return getNodeApiEnvironmentData(
        napiEnv(env->parentEnvironment_), tag, data);
  }
  CHECK_ARG(data);

  auto it = env->taggedData_.find(tag);
  *data = (it != env->taggedData_.end()) ? it->second : nullptr;
  return env->clearLastNativeError();
}

napi_status queueMicrotask(napi_env env, napi_value callback) noexcept {
  CHECK_STATUS(checkGCPreconditions(env));
  if (LLVM_UNLIKELY(!env->runtime_.hasMicrotaskQueue())) {
    return GENERIC_FAILURE("Microtasks are not supported in this runtime");
  }

  CHECK_ARG(callback);
  vm::GCScope gcScope{env->runtime_};
  RETURN_STATUS_IF_FALSE(
      vm::vmisa<vm::Callable>(*phv(callback)), napi_invalid_arg);
  vm::Handle<vm::Callable> callbackHandle = asHandle<vm::Callable>(callback);
  env->runtime_.enqueueJob(callbackHandle.get());

  return env->clearLastNativeError();
}

napi_status runInNodeApiContext(
    napi_env env,
    nodeApiCallback callback,
    void *data,
    napi_value *result) noexcept {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ result == nullptr ? 0 : 1);
  NodeApiEscapableValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};
  vm::CallResult<vm::HermesValue> cr = callback(data);
  if (cr.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  return scope.escapeResult(*cr, result);
}

template <>
napi_status setLastNativeError(
    napi_env env,
    napi_status status,
    const char *fileName,
    uint32_t line,
    const std::string &message) noexcept {
  return CHECKED_ENV(env)->setLastNativeError(status, fileName, line, message);
}

napi_status clearLastNativeError(napi_env env) noexcept {
  return CHECKED_ENV(env)->clearLastNativeError();
}

napi_status openNodeApiScope(napi_env env, void **scope) noexcept {
  CHECK_ENV(env);
  CHECK_ARG(scope);
  if (env->tlsCurrentEnvironment_) {
    env->tlsCurrentEnvStack_.push_back(env->tlsCurrentEnvironment_);
  }
  env->tlsCurrentEnvironment_ = env;
  *scope = env;
  return env->clearLastNativeError();
}

napi_status closeNodeApiScope(napi_env env, void *scope) noexcept {
  CHECK_ENV(env);
  CHECK_ARG(scope);
  RETURN_STATUS_IF_FALSE(
      env->tlsCurrentEnvironment_ == scope, napi_invalid_arg);
  env->tlsCurrentEnvironment_ = env->tlsCurrentEnvStack_.empty()
      ? nullptr
      : env->tlsCurrentEnvStack_.pop_back_val();
  return env->clearLastNativeError();
}

} // namespace hermes::node_api

//=============================================================================
// Node-API implementation
//=============================================================================

napi_status NAPI_CDECL napi_get_last_error_info(
    node_api_basic_env basic_env,
    const napi_extended_error_info **result) {
  napi_env env = const_cast<napi_env>(basic_env);
  CHECK_STATUS(
      checkBasicPreconditions<NodeApiPendingExceptionCheck::kSkip>(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(result);
  if (env->lastError_.error_code == napi_ok) {
    env->lastError_ = {nullptr, 0, 0, napi_ok};
  }
  *result = &env->lastError_;
  return napi_ok;
}

napi_status NAPI_CDECL napi_create_function(
    napi_env env,
    const char *utf8Name,
    size_t length,
    napi_callback callback,
    void *callbackData,
    napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);
  CHECK_ARG(callback);
  NodeApiEscapableValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};
  vm::MutableHandle<vm::SymbolID> nameSymbolID{env->runtime_};
  if (utf8Name != nullptr) {
    CHECK_STATUS(env->getUniqueSymbolID(utf8Name, length, &nameSymbolID));
  } else {
    nameSymbolID = env->getPredefinedSymbol(NodeApiPredefined::hostFunction);
  }
  vm::MutableHandle<vm::Callable> func{env->runtime_};
  CHECK_STATUS(
      env->createFunction(nameSymbolID.get(), callback, callbackData, &func));
  return scope.escapeResult(func.getHermesValue(), result);
}

napi_status NAPI_CDECL napi_define_class(
    napi_env env,
    const char *utf8Name,
    size_t length,
    napi_callback constructor,
    void *callbackData,
    size_t propertyCount,
    const napi_property_descriptor *properties,
    napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);
  CHECK_ARG(constructor);
  if (propertyCount > 0) {
    CHECK_ARG(properties);
  }

  NodeApiEscapableValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};

  vm::MutableHandle<vm::SymbolID> nameHandle{env->runtime_};
  CHECK_STATUS(env->getUniqueSymbolID(utf8Name, length, &nameHandle));

  vm::Handle<vm::JSObject> parentHandle =
      vm::Handle<vm::JSObject>::vmcast(&env->runtime_.functionPrototype);

  std::unique_ptr<NodeApiHostFunctionContext> context =
      std::make_unique<NodeApiHostFunctionContext>(
          *env, constructor, callbackData);

  vm::Handle<vm::JSObject> prototypeHandle{
      env->runtime_.makeHandle(vm::JSObject::create(env->runtime_))};

  vm::CallResult<vm::HermesValue> ctorRes =
      vm::FinalizableNativeFunction::create(
          env->runtime(),
          context.get(),
          &NodeApiHostFunctionContext::func,
          &NodeApiHostFunctionContext::finalize,
          nameHandle.get(),
          /*paramCount:*/ 0,
          prototypeHandle);
  CHECK_STATUS(env->checkExecutionStatus(ctorRes.getStatus()));
  context.release(); // the context is now owned by the func.

  vm::Handle<> classHandle = env->runtime_.makeHandle(*ctorRes);

  for (size_t i = 0; i < propertyCount; ++i) {
    const napi_property_descriptor &propDesc = properties[i];
    if ((propDesc.attributes & napi_static) != 0) {
      CHECK_STATUS(
          napi_define_properties(env, napiValue(classHandle), 1, &propDesc));
    } else {
      CHECK_STATUS(napi_define_properties(
          env, napiValue(prototypeHandle), 1, &propDesc));
    }
  }

  return scope.escapeResult(classHandle.getHermesValue(), result);
}

napi_status NAPI_CDECL
napi_get_property_names(napi_env env, napi_value object, napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(object);
  CHECK_ARG(result);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> objRes =
      vm::toObject(env->runtime_, asHandle(object));
  if (objRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  return env->getForInPropertyNames(
      env->runtime_.makeHandle<vm::JSObject>(*objRes),
      napi_key_numbers_to_strings,
      result);
}

napi_status NAPI_CDECL napi_get_all_property_names(
    napi_env env,
    napi_value object,
    napi_key_collection_mode keyMode,
    napi_key_filter keyFilter,
    napi_key_conversion keyConversion,
    napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);
  CHECK_ARG(object);
  NodeApiEscapableValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> cr =
      vm::toObject(env->runtime_, asHandle(object));
  CHECK_STATUS(env->checkExecutionStatus(cr.getStatus()));
  vm::Handle<vm::JSObject> objValue =
      env->runtime_.makeHandle<vm::JSObject>(*cr);

  RETURN_STATUS_IF_FALSE(
      isInEnumRange(keyMode, napi_key_include_prototypes, napi_key_own_only),
      napi_invalid_arg);
  RETURN_STATUS_IF_FALSE(
      isInEnumRange(
          keyConversion, napi_key_keep_numbers, napi_key_numbers_to_strings),
      napi_invalid_arg);

  // We can use optimized code if object has no parent.
  bool hasParent = false;
  if (keyMode == napi_key_include_prototypes) {
    napi_value parent{};
    CHECK_STATUS(napi_get_prototype(env, object, &parent));
    hasParent = phv(parent)->isObject();
  }

  // The fast path used for the 'for..in' implementation.
  if ((keyFilter == (napi_key_enumerable | napi_key_skip_symbols)) &&
      (keyMode == napi_key_include_prototypes || !hasParent)) {
    CHECK_STATUS(env->getForInPropertyNames(objValue, keyConversion, result));
    return scope.escapeResult(result);
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
            objValue,
            env->runtime_,
            ownKeyFlags.setIncludeNonEnumerable(
                (keyFilter & napi_key_enumerable) == 0));
    CHECK_STATUS(env->checkExecutionStatus(ownKeysRes.getStatus()));
    if (keyConversion == napi_key_numbers_to_strings) {
      CHECK_STATUS(env->convertToStringKeys(*ownKeysRes));
    }
    return scope.escapeResult(ownKeysRes->getHermesValue(), result);
  }

  // Collect all properties into the keyStorage.
  // vm::CallResult<vm::PseudoHandle<vm::ArrayStorage>> keyStorageRes =
  //     vm::ArrayStorage::create(env->runtime_, 16);
  // if (keyStorageRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
  //   return env->setJSException();
  // }
  // vm::MutableHandle<vm::ArrayStorage> keyStorageHandle =
  //     env->runtime_.makeMutableHandle(keyStorageRes->get());
  // uint32_t size{0};

  vm::CallResult<vm::HermesValue> keyStorageRes =
      vm::ArrayStorageSmall::create(env->runtime_, 16);
  if (keyStorageRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  vm::MutableHandle<vm::ArrayStorageSmall> keyStorageHandle =
      env->runtime_.makeMutableHandle(
          vm::vmcast<vm::ArrayStorageSmall>(*keyStorageRes));
  uint32_t size{0};

  // Make sure that we do not include into the result properties that were
  // shadowed by the derived objects.
  bool useParentChain = keyMode == napi_key_include_prototypes && hasParent;
  NodeApiOrderedSet<uint32_t> shadowIndexes;
  NodeApiOrderedSet<vm::HermesValue> shadowStrings(
      *env, [](const vm::HermesValue &item1, const vm::HermesValue &item2) {
        return item1.getString()->compare(item2.getString());
      });
  NodeApiOrderedSet<vm::HermesValue> shadowSymbols(
      *env, [](const vm::HermesValue &item1, const vm::HermesValue &item2) {
        vm::SymbolID::RawType rawItem1 = item1.getSymbol().unsafeGetRaw();
        vm::SymbolID::RawType rawItem2 = item2.getSymbol().unsafeGetRaw();
        return rawItem1 < rawItem2 ? -1 : rawItem1 > rawItem2 ? 1 : 0;
      });

  // Should we apply the filter?
  bool useFilter =
      (keyFilter &
       (napi_key_writable | napi_key_enumerable | napi_key_configurable)) != 0;

  // Keep the mutable variables outside of loop for efficiency
  vm::MutableHandle<vm::JSObject> currentObj(env->runtime_, objValue.get());
  vm::MutableHandle<> prop{env->runtime_};
  OptValue<uint32_t> propIndexOpt;
  vm::MutableHandle<vm::StringPrimitive> propString{env->runtime_};

  while (currentObj.get()) {
    vm::GCScope gcScope{env->runtime_};

    vm::CallResult<vm::Handle<vm::JSArray>> props =
        vm::JSObject::getOwnPropertyKeys(
            currentObj, env->runtime_, ownKeyFlags);
    CHECK_STATUS(env->checkExecutionStatus(props.getStatus()));

    vm::GCScope::Marker marker = gcScope.createMarker();
    for (uint32_t i = 0, end = props.getValue()->getEndIndex(); i < end; ++i) {
      gcScope.flushToMarker(marker);
      prop = props.getValue()->at(env->runtime_, i).unboxToHV(env->runtime_);

      // Do not add a property if it is overridden in the derived object.
      if (useParentChain) {
        if (prop->isString()) {
          propString = vm::Handle<vm::StringPrimitive>::vmcast(prop);
          // See if the property name is an index
          propIndexOpt = vm::toArrayIndex(
              vm::StringPrimitive::createStringView(env->runtime_, propString));
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
        vm::ComputedPropertyDescriptor desc;
        vm::CallResult<bool> hasDescriptorRes =
            vm::JSObject::getOwnComputedPrimitiveDescriptor(
                currentObj,
                env->runtime_,
                prop,
                vm::JSObject::IgnoreProxy::No,
                desc);
        CHECK_STATUS(env->checkExecutionStatus(hasDescriptorRes.getStatus()));
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

      CHECK_STATUS(env->checkExecutionStatus(vm::ArrayStorageSmall::push_back(
          keyStorageHandle, env->runtime_, prop)));
      ++size;
    }

    if (!useParentChain) {
      // Do not follow the prototype chain.
      break;
    }

    // Continue to follow the prototype chain.
    vm::CallResult<vm::PseudoHandle<vm::JSObject>> parentRes =
        vm::JSObject::getPrototypeOf(currentObj, env->runtime_);
    CHECK_STATUS(env->checkExecutionStatus(parentRes.getStatus()));
    currentObj = std::move(*parentRes);
  }

  CHECK_STATUS(env->convertKeyStorageToArray(
      keyStorageHandle, 0, size, keyConversion, result));
  return scope.escapeResult(result);
}

napi_status NAPI_CDECL napi_set_property(
    napi_env env,
    napi_value object,
    napi_value key,
    napi_value value) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(object);
  CHECK_ARG(key);
  CHECK_ARG(value);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> cr =
      vm::toObject(env->runtime_, asHandle(object));
  CHECK_STATUS(env->checkExecutionStatus(cr.getStatus()));
  vm::Handle<vm::JSObject> objValue =
      env->runtime_.makeHandle<vm::JSObject>(*cr);

  vm::CallResult<bool> res = vm::JSObject::putComputed_RJS(
      objValue,
      env->runtime_,
      asHandle(key),
      asHandle(value),
      vm::PropOpFlags().plusThrowOnError());
  CHECK_STATUS(env->checkExecutionStatus(res.getStatus()));
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_has_property(
    napi_env env,
    napi_value object,
    napi_value key,
    bool *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(object);
  CHECK_ARG(key);
  CHECK_ARG(result);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> cr =
      vm::toObject(env->runtime_, asHandle(object));
  CHECK_STATUS(env->checkExecutionStatus(cr.getStatus()));
  vm::Handle<vm::JSObject> objValue =
      env->runtime_.makeHandle<vm::JSObject>(*cr);

  vm::CallResult<bool> res =
      vm::JSObject::hasComputed(objValue, env->runtime_, asHandle(key));
  CHECK_STATUS(env->checkExecutionStatus(res.getStatus()));
  *result = *res;
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_get_property(
    napi_env env,
    napi_value object,
    napi_value key,
    napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(object);
  CHECK_ARG(key);
  CHECK_ARG(result);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> objRes =
      vm::toObject(env->runtime_, asHandle(object));
  if (objRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  vm::CallResult<vm::PseudoHandle<>> cr = vm::JSObject::getComputed_RJS(
      env->runtime_.makeHandle<vm::JSObject>(*objRes),
      env->runtime_,
      asHandle(key));
  if (cr.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  return env->makeResultValue(cr->getHermesValue(), result);
}

napi_status NAPI_CDECL napi_delete_property(
    napi_env env,
    napi_value object,
    napi_value key,
    bool *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(object);
  CHECK_ARG(key);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> objRes =
      vm::toObject(env->runtime_, asHandle(object));
  if (objRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  vm::CallResult<bool> cr = vm::JSObject::deleteComputed(
      env->runtime_.makeHandle<vm::JSObject>(*objRes),
      env->runtime_,
      asHandle(key));
  if (cr.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  if (result != nullptr) {
    *result = *cr;
  }
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_has_own_property(
    napi_env env,
    napi_value object,
    napi_value key,
    bool *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(object);
  CHECK_ARG(key);
  CHECK_ARG(result);
  RETURN_STATUS_IF_FALSE(
      phv(key)->isString() || phv(key)->isSymbol(), napi_name_expected);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> objRes =
      vm::toObject(env->runtime_, asHandle(object));
  if (objRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  vm::ComputedPropertyDescriptor desc;
  vm::CallResult<bool> cr = vm::JSObject::getOwnComputedDescriptor(
      env->runtime_.makeHandle<vm::JSObject>(*objRes),
      env->runtime_,
      asHandle(key),
      /*ref*/ desc);
  if (cr.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  *result = *cr;
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_set_named_property(
    napi_env env,
    napi_value object,
    const char *utf8Name,
    napi_value value) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(object);
  CHECK_ARG(utf8Name);
  CHECK_ARG(value);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> objRes =
      vm::toObject(env->runtime_, asHandle(object));
  if (objRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  vm::Handle<vm::JSObject> objHandle =
      env->runtime_.makeHandle<vm::JSObject>(*objRes);

  vm::CallResult<vm::HermesValue> strRes = vm::StringPrimitive::createEfficient(
      env->runtime_,
      llvh::makeArrayRef(
          reinterpret_cast<const uint8_t *>(utf8Name),
          std::char_traits<char>::length(utf8Name)),
      /*IgnoreInputErrors:*/ true);
  if (strRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  vm::CallResult<vm::Handle<vm::SymbolID>> symRes = vm::stringToSymbolID(
      env->runtime_, vm::createPseudoHandle(strRes->getString()));
  if (symRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  vm::CallResult<bool> res = objHandle->putNamedOrIndexed(
      objHandle,
      env->runtime_,
      symRes->get(),
      asHandle(value),
      vm::PropOpFlags().plusThrowOnError());
  if (res.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_has_named_property(
    napi_env env,
    napi_value object,
    const char *utf8Name,
    bool *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(object);
  CHECK_ARG(utf8Name);
  CHECK_ARG(result);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> objRes =
      vm::toObject(env->runtime_, asHandle(object));
  if (objRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  vm::Handle<vm::JSObject> objHandle =
      env->runtime_.makeHandle<vm::JSObject>(*objRes);

  vm::CallResult<vm::HermesValue> strRes = vm::StringPrimitive::createEfficient(
      env->runtime_,
      llvh::makeArrayRef(
          reinterpret_cast<const uint8_t *>(utf8Name),
          std::char_traits<char>::length(utf8Name)),
      /*IgnoreInputErrors:*/ true);
  if (strRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  vm::CallResult<vm::Handle<vm::SymbolID>> symRes = vm::stringToSymbolID(
      env->runtime_, vm::createPseudoHandle(strRes->getString()));
  if (symRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  vm::CallResult<bool> valueRes =
      objHandle->hasNamedOrIndexed(objHandle, env->runtime_, symRes->get());
  if (valueRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  *result = *valueRes;
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_get_named_property(
    napi_env env,
    napi_value object,
    const char *utf8Name,
    napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(object);
  CHECK_ARG(utf8Name);
  CHECK_ARG(result);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> objRes =
      vm::toObject(env->runtime_, asHandle(object));
  if (objRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  vm::Handle<vm::JSObject> objHandle =
      env->runtime_.makeHandle<vm::JSObject>(*objRes);

  vm::CallResult<vm::HermesValue> strRes = vm::StringPrimitive::createEfficient(
      env->runtime_,
      llvh::makeArrayRef(
          reinterpret_cast<const uint8_t *>(utf8Name),
          std::char_traits<char>::length(utf8Name)),
      /*IgnoreInputErrors:*/ true);
  if (strRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  vm::CallResult<vm::Handle<vm::SymbolID>> symRes = vm::stringToSymbolID(
      env->runtime_, vm::createPseudoHandle(strRes->getString()));
  if (symRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  vm::CallResult<vm::PseudoHandle<>> valueRes =
      objHandle->getNamedOrIndexed(objHandle, env->runtime_, symRes->get());
  if (valueRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  return env->makeResultValue(valueRes->getHermesValue(), result);
}

napi_status NAPI_CDECL napi_set_element(
    napi_env env,
    napi_value object,
    uint32_t index,
    napi_value value) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(object);
  CHECK_ARG(value);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> objRes =
      vm::toObject(env->runtime_, asHandle(object));
  if (objRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  vm::Handle<vm::JSObject> objHandle =
      env->runtime_.makeHandle<vm::JSObject>(*objRes);

  vm::CallResult<bool> res = objHandle->putComputed_RJS(
      objHandle,
      env->runtime_,
      env->runtime_.makeHandle(
          vm::HermesValue::encodeTrustedNumberValue(index)),
      asHandle(value),
      vm::PropOpFlags().plusThrowOnError());
  if (res.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_has_element(
    napi_env env,
    napi_value object,
    uint32_t index,
    bool *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(object);
  CHECK_ARG(result);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> objRes =
      vm::toObject(env->runtime_, asHandle(object));
  if (objRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  vm::Handle<vm::JSObject> objHandle =
      env->runtime_.makeHandle<vm::JSObject>(*objRes);

  vm::CallResult<bool> res = objHandle->hasComputed(
      objHandle,
      env->runtime_,
      env->runtime_.makeHandle(
          vm::HermesValue::encodeTrustedNumberValue(index)));
  if (res.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  *result = *res;
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_get_element(
    napi_env env,
    napi_value object,
    uint32_t index,
    napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(object);
  CHECK_ARG(result);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> objRes =
      vm::toObject(env->runtime_, asHandle(object));
  if (objRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  vm::Handle<vm::JSObject> objHandle =
      env->runtime_.makeHandle<vm::JSObject>(*objRes);

  vm::CallResult<vm::PseudoHandle<>> valueRes = objHandle->getComputed_RJS(
      objHandle,
      env->runtime_,
      env->runtime_.makeHandle(
          vm::HermesValue::encodeTrustedNumberValue(index)));
  if (valueRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  return env->makeResultValue(valueRes->getHermesValue(), result);
}

napi_status NAPI_CDECL napi_delete_element(
    napi_env env,
    napi_value object,
    uint32_t index,
    bool *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(object);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> objRes =
      vm::toObject(env->runtime_, asHandle(object));
  if (objRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  vm::Handle<vm::JSObject> objHandle =
      env->runtime_.makeHandle<vm::JSObject>(*objRes);

  vm::CallResult<bool> res = objHandle->deleteComputed(
      objHandle,
      env->runtime_,
      env->runtime_.makeHandle(
          vm::HermesValue::encodeTrustedNumberValue(index)));
  if (res.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  if (result != nullptr) {
    *result = *res;
  }
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_define_properties(
    napi_env env,
    napi_value object,
    size_t propertyCount,
    const napi_property_descriptor *properties) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG_IS_OBJECT(object);
  if (propertyCount > 0) {
    CHECK_ARG(properties);
  }

  NodeApiValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> objRes =
      vm::toObject(env->runtime_, asHandle(object));
  if (objRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  vm::Handle<vm::JSObject> objHandle =
      env->runtime_.makeHandle<vm::JSObject>(*objRes);

  vm::MutableHandle<vm::SymbolID> name{env->runtime_};
  vm::GCScopeMarkerRAII marker{env->runtime_};
  for (size_t i = 0; i < propertyCount; ++i) {
    marker.flush();
    const napi_property_descriptor *p = &properties[i];
    CHECK_STATUS(env->symbolIDFromPropertyDescriptor(p, &name));

    vm::DefinePropertyFlags dpFlags{};
    dpFlags.setEnumerable = 1;
    dpFlags.setConfigurable = 1;
    dpFlags.enumerable = (p->attributes & napi_enumerable) == 0 ? 0 : 1;
    dpFlags.configurable = (p->attributes & napi_configurable) == 0 ? 0 : 1;

    if ((p->getter != nullptr) || (p->setter != nullptr)) {
      vm::MutableHandle<vm::Callable> localGetter{env->runtime_};
      vm::MutableHandle<vm::Callable> localSetter{env->runtime_};

      if (p->getter != nullptr) {
        dpFlags.setGetter = 1;
        CHECK_STATUS(env->createFunction(
            vm::Predefined::getSymbolID(vm::Predefined::get),
            p->getter,
            p->data,
            &localGetter));
      }
      if (p->setter != nullptr) {
        dpFlags.setSetter = 1;
        CHECK_STATUS(env->createFunction(
            vm::Predefined::getSymbolID(vm::Predefined::set),
            p->setter,
            p->data,
            &localSetter));
      }

      vm::PseudoHandle<vm::PropertyAccessor> propRes =
          vm::PropertyAccessor::create(env->runtime_, localGetter, localSetter);

      CHECK_STATUS(env->defineOwnProperty(
          objHandle,
          *name,
          dpFlags,
          env->runtime_.makeHandle(std::move(propRes)),
          nullptr));
    } else {
      dpFlags.setValue = 1;
      dpFlags.setWritable = 1;
      dpFlags.writable = (p->attributes & napi_writable) == 0 ? 0 : 1;
      if (p->method != nullptr) {
        vm::MutableHandle<vm::Callable> method{env->runtime_};
        CHECK_STATUS(
            env->createFunction(name.get(), p->method, p->data, &method));
        CHECK_STATUS(
            env->defineOwnProperty(objHandle, *name, dpFlags, method, nullptr));
      } else {
        CHECK_STATUS(env->defineOwnProperty(
            objHandle, *name, dpFlags, asHandle(p->value), nullptr));
      }
    }
  }

  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_object_freeze(napi_env env, napi_value object) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(object);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> objRes =
      vm::toObject(env->runtime_, asHandle(object));
  if (objRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  vm::ExecutionStatus status = vm::JSObject::freeze(
      env->runtime_.makeHandle<vm::JSObject>(*objRes), env->runtime_);
  if (status == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_object_seal(napi_env env, napi_value object) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(object);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> objRes =
      vm::toObject(env->runtime_, asHandle(object));
  if (objRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  vm::ExecutionStatus status = vm::JSObject::seal(
      env->runtime_.makeHandle<vm::JSObject>(*objRes), env->runtime_);
  if (status == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_is_array(napi_env env, napi_value value, bool *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  *result = vm::vmisa<vm::JSArray>(*phv(value));
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_get_array_length(napi_env env, napi_value value, uint32_t *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(result);
  NodeApiValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};
  CHECK_ARG(value);
  RETURN_STATUS_IF_FALSE(
      vm::vmisa<vm::JSArray>(*phv(value)), napi_array_expected);
  napi_value res;
  CHECK_STATUS(env->getNamedProperty(
      asHandle<vm::JSObject>(value),
      vm::Predefined::getSymbolID(vm::Predefined::length),
      &res));
  RETURN_STATUS_IF_FALSE(phv(res)->isNumber(), napi_number_expected);
  *result = NodeApiDoubleConversion::toUint32(phv(res)->getDouble());
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_strict_equals(napi_env env, napi_value lhs, napi_value rhs, bool *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(lhs);
  CHECK_ARG(rhs);
  CHECK_ARG(result);
  vm::HermesValue::Tag lhsTag = phv(lhs)->getTag();
  if (lhsTag != phv(rhs)->getTag()) {
    *result = false;
  } else if (lhsTag == vm::HermesValue::Tag::Str) {
    *result = phv(lhs)->getString()->equals(phv(rhs)->getString());
  } else if (lhsTag == vm::HermesValue::Tag::BoolSymbol) {
    vm::HermesValue::ETag lhsETag = phv(lhs)->getETag();
    if (lhsETag != phv(rhs)->getETag()) {
      *result = false;
    } else if (lhsETag == vm::HermesValue::ETag::Symbol) {
      *result = phv(lhs)->getSymbol() == phv(rhs)->getSymbol();
    } else {
      *result = phv(lhs)->getBool() == phv(rhs)->getBool();
    }
  } else if (lhsTag == vm::HermesValue::Tag::BigInt) {
    *result = phv(lhs)->getBigInt()->compare(phv(rhs)->getBigInt()) == 0;
  } else {
    *result = phv(lhs)->getRaw() == phv(rhs)->getRaw();
  }
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_get_prototype(napi_env env, napi_value object, napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(object);
  CHECK_ARG(result);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> objRes =
      vm::toObject(env->runtime_, asHandle(object));
  if (objRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  vm::CallResult<vm::PseudoHandle<vm::JSObject>> protoRes =
      vm::JSObject::getPrototypeOf(
          env->runtime_.makeHandle<vm::JSObject>(*objRes), env->runtime_);
  if (protoRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  if (!*protoRes) {
    return env->castResult(&env->NullHermesValue, result);
  }
  return env->makeResultValue(protoRes->getHermesValue(), result);
}

napi_status NAPI_CDECL napi_create_object(napi_env env, napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);
  vm::PseudoHandle<vm::JSObject> res = vm::JSObject::create(env->runtime_);
  return env->makeResultValue(res.getHermesValue(), result);
}

napi_status NAPI_CDECL napi_create_array(napi_env env, napi_value *result) {
  return napi_create_array_with_length(env, 0, result);
}

napi_status NAPI_CDECL
napi_create_array_with_length(napi_env env, size_t length, napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);

  vm::GCScope gcScope{env->runtime_};
  vm::CallResult<vm::PseudoHandle<vm::JSArray>> res = vm::JSArray::create(
      env->runtime_, /*capacity:*/ length, /*length:*/ length);
  CHECK_STATUS(env->checkExecutionStatus(res.getStatus()));
  return env->makeResultValue(res->getHermesValue(), result);
}

napi_status NAPI_CDECL napi_create_string_latin1(
    napi_env env,
    const char *str,
    size_t length,
    napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  if (length > 0) {
    CHECK_ARG(str);
  }
  CHECK_ARG(result);
  if (length == NAPI_AUTO_LENGTH) {
    length = std::char_traits<char>::length(str);
  }
  RETURN_STATUS_IF_FALSE(
      length <= static_cast<size_t>(std::numeric_limits<int32_t>::max()),
      napi_invalid_arg);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> strRes =
      ::hermes::isAllASCII(str, str + length)
      ? vm::StringPrimitive::createEfficient(
            env->runtime_, llvh::makeArrayRef(str, length))
      : vm::StringPrimitive::createEfficient(
            env->runtime_, latin1ToUtf16(str, length));
  if (strRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  return env->makeResultValue(*strRes, result);
}

napi_status NAPI_CDECL napi_create_string_utf8(
    napi_env env,
    const char *str,
    size_t length,
    napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  if (length > 0) {
    CHECK_ARG(str);
  }
  CHECK_ARG(result);
  if (length == NAPI_AUTO_LENGTH) {
    length = std::char_traits<char>::length(str);
  }
  RETURN_STATUS_IF_FALSE(
      length <= static_cast<size_t>(std::numeric_limits<int32_t>::max()),
      napi_invalid_arg);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> strRes = vm::StringPrimitive::createEfficient(
      env->runtime_,
      llvh::makeArrayRef(reinterpret_cast<const uint8_t *>(str), length),
      /*IgnoreInputError:*/ true);
  if (strRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  return env->makeResultValue(*strRes, result);
}

napi_status NAPI_CDECL napi_create_string_utf16(
    napi_env env,
    const char16_t *str,
    size_t length,
    napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  if (length > 0) {
    CHECK_ARG(str);
  }
  CHECK_ARG(result);
  if (length == NAPI_AUTO_LENGTH) {
    length = std::char_traits<char16_t>::length(str);
  }
  RETURN_STATUS_IF_FALSE(
      length <= static_cast<size_t>(std::numeric_limits<int32_t>::max()),
      napi_invalid_arg);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> strRes = vm::StringPrimitive::createEfficient(
      env->runtime_, llvh::makeArrayRef(str, length));
  if (strRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  return env->makeResultValue(*strRes, result);
}

napi_status NAPI_CDECL node_api_create_external_string_latin1(
    napi_env env,
    char *str,
    size_t length,
    node_api_basic_finalize finalizeCallback,
    void *finalizeHint,
    napi_value *result,
    bool *copied) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  // TODO: Add support to Hermes for external strings.
  CHECK_STATUS(napi_create_string_latin1(env, str, length, result));
  if (finalizeCallback != nullptr) {
    finalizeCallback(env, str, finalizeHint);
  }
  if (copied != nullptr) {
    // TODO: we report here false to pass the Node-API tests.
    *copied = false;
  }
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL node_api_create_external_string_utf16(
    napi_env env,
    char16_t *str,
    size_t length,
    node_api_basic_finalize finalizeCallback,
    void *finalizeHint,
    napi_value *result,
    bool *copied) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  // TODO: Add support to Hermes for external strings.
  CHECK_STATUS(napi_create_string_utf16(env, str, length, result));
  if (finalizeCallback != nullptr) {
    finalizeCallback(env, str, finalizeHint);
  }
  if (copied != nullptr) {
    // TODO: we report here false to pass the Node-API tests.
    *copied = false;
  }
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL node_api_create_property_key_latin1(
    napi_env env,
    const char *str,
    size_t length,
    napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  if (length > 0) {
    CHECK_ARG(str);
  }
  CHECK_ARG(result);
  if (length == NAPI_AUTO_LENGTH) {
    length = std::char_traits<char>::length(str);
  }
  RETURN_STATUS_IF_FALSE(
      length <= static_cast<size_t>(std::numeric_limits<int32_t>::max()),
      napi_invalid_arg);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> strRes =
      ::hermes::isAllASCII(str, str + length)
      ? vm::StringPrimitive::createEfficient(
            env->runtime_, llvh::makeArrayRef(str, length))
      : vm::StringPrimitive::createEfficient(
            env->runtime_, latin1ToUtf16(str, length));
  if (strRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  vm::CallResult<vm::Handle<vm::SymbolID>> symRes = vm::stringToSymbolID(
      env->runtime_, vm::createPseudoHandle(strRes->getString()));
  if (symRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  // Get uniqued string from the symbol.
  vm::StringPrimitive *strPrim =
      env->runtime_.getIdentifierTable().getStringPrim(
          env->runtime_, symRes->get());

  return env->makeResultValue(
      vm::HermesValue::encodeStringValue(strPrim), result);
}

napi_status NAPI_CDECL node_api_create_property_key_utf8(
    napi_env env,
    const char *str,
    size_t length,
    napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  if (length > 0) {
    CHECK_ARG(str);
  }
  CHECK_ARG(result);
  if (length == NAPI_AUTO_LENGTH) {
    length = std::char_traits<char>::length(str);
  }
  RETURN_STATUS_IF_FALSE(
      length <= static_cast<size_t>(std::numeric_limits<int32_t>::max()),
      napi_invalid_arg);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> strRes = vm::StringPrimitive::createEfficient(
      env->runtime_,
      llvh::makeArrayRef(reinterpret_cast<const uint8_t *>(str), length),
      /*IgnoreInputErrors:*/ true);
  if (strRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  vm::CallResult<vm::Handle<vm::SymbolID>> symRes = vm::stringToSymbolID(
      env->runtime_, vm::createPseudoHandle(strRes->getString()));
  if (symRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  // Get uniqued string from the symbol.
  vm::StringPrimitive *strPrim =
      env->runtime_.getIdentifierTable().getStringPrim(
          env->runtime_, symRes->get());

  return env->makeResultValue(
      vm::HermesValue::encodeStringValue(strPrim), result);
}

napi_status NAPI_CDECL node_api_create_property_key_utf16(
    napi_env env,
    const char16_t *str,
    size_t length,
    napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  if (length > 0) {
    CHECK_ARG(str);
  }
  CHECK_ARG(result);
  if (length == NAPI_AUTO_LENGTH) {
    length = std::char_traits<char16_t>::length(str);
  }
  RETURN_STATUS_IF_FALSE(
      length <= static_cast<size_t>(std::numeric_limits<int32_t>::max()),
      napi_invalid_arg);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> strRes = vm::StringPrimitive::createEfficient(
      env->runtime_, llvh::makeArrayRef(str, length));
  if (strRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  vm::CallResult<vm::Handle<vm::SymbolID>> symRes = vm::stringToSymbolID(
      env->runtime_, vm::createPseudoHandle(strRes->getString()));
  if (symRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }

  // Get uniqued string from the symbol.
  vm::StringPrimitive *strPrim =
      env->runtime_.getIdentifierTable().getStringPrim(
          env->runtime_, symRes->get());

  return env->makeResultValue(
      vm::HermesValue::encodeStringValue(strPrim), result);
}

napi_status NAPI_CDECL
napi_create_double(napi_env env, double value, napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);
  return env->makeResultValue(
      vm::HermesValue::encodeUntrustedNumberValue(value), result);
}

napi_status NAPI_CDECL
napi_create_int32(napi_env env, int32_t value, napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);
  return env->makeResultValue(
      vm::HermesValue::encodeTrustedNumberValue(static_cast<double>(value)),
      result);
}

napi_status NAPI_CDECL
napi_create_uint32(napi_env env, uint32_t value, napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);
  return env->makeResultValue(
      vm::HermesValue::encodeTrustedNumberValue(static_cast<double>(value)),
      result);
}

napi_status NAPI_CDECL
napi_create_int64(napi_env env, int64_t value, napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);
  return env->makeResultValue(
      vm::HermesValue::encodeTrustedNumberValue(static_cast<double>(value)),
      result);
}

napi_status NAPI_CDECL
napi_create_bigint_int64(napi_env env, int64_t value, napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);
  vm::CallResult<vm::HermesValue> cr =
      vm::BigIntPrimitive::fromSigned(env->runtime_, value);
  CHECK_STATUS(env->checkExecutionStatus(cr.getStatus()));
  return env->makeResultValue(*cr, result);
}

napi_status NAPI_CDECL
napi_create_bigint_uint64(napi_env env, uint64_t value, napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);
  vm::CallResult<vm::HermesValue> cr =
      vm::BigIntPrimitive::fromUnsigned(env->runtime_, value);
  CHECK_STATUS(env->checkExecutionStatus(cr.getStatus()));
  return env->makeResultValue(*cr, result);
}

napi_status NAPI_CDECL napi_create_bigint_words(
    napi_env env,
    int32_t signBit,
    size_t wordCount,
    const uint64_t *words,
    napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(words);
  CHECK_ARG(result);
  RETURN_STATUS_IF_FALSE(wordCount <= INT_MAX, napi_invalid_arg);

  vm::GCScope gcScope{env->runtime_};

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
  vm::CallResult<vm::HermesValue> cr = vm::BigIntPrimitive::fromBytes(
      env->runtime_, llvh::makeArrayRef(ptr, size));
  CHECK_STATUS(env->checkExecutionStatus(cr.getStatus()));
  return env->makeResultValue(*cr, result);
}

napi_status NAPI_CDECL
napi_get_boolean(napi_env env, bool value, napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(result);
  *result = napiValue(value ? &env->TrueHermesValue : &env->FalseHermesValue);
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_create_symbol(napi_env env, napi_value description, napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);

  vm::GCScope gcScope{env->runtime_};
  vm::MutableHandle<vm::StringPrimitive> descString{env->runtime_};
  if (description != nullptr) {
    CHECK_ARG_IS_STRING(description);
    descString = phv(description)->getString();
  } else {
    // If description is undefined, the descString will eventually be "".
    descString = env->runtime_.getPredefinedString(vm::Predefined::emptyString);
  }

  vm::CallResult<vm::SymbolID> cr =
      env->runtime_.getIdentifierTable().createNotUniquedSymbol(
          env->runtime_, descString);
  CHECK_STATUS(env->checkExecutionStatus(cr.getStatus()));
  return env->makeResultValue(vm::HermesValue::encodeSymbolValue(*cr), result);
}

napi_status NAPI_CDECL node_api_symbol_for(
    napi_env env,
    const char *utf8description,
    size_t length,
    napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);
  NodeApiEscapableValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};
  napi_value key{};
  CHECK_STATUS(napi_create_string_utf8(env, utf8description, length, &key));

  vm::CallResult<vm::SymbolID> cr =
      env->runtime_.getSymbolRegistry().getSymbolForKey(
          env->runtime_, asHandle<vm::StringPrimitive>(key));
  CHECK_STATUS(env->checkExecutionStatus(cr.getStatus()));
  return scope.escapeResult(vm::HermesValue::encodeSymbolValue(*cr), result);
}

napi_status NAPI_CDECL napi_create_error(
    napi_env env,
    napi_value code,
    napi_value message,
    napi_value *result) {
  CHECK_ENV(env);
  return createJSError(
      env, env->runtime_.ErrorPrototype, code, message, result);
}

napi_status NAPI_CDECL napi_create_type_error(
    napi_env env,
    napi_value code,
    napi_value message,
    napi_value *result) {
  CHECK_ENV(env);
  return createJSError(
      env, env->runtime_.TypeErrorPrototype, code, message, result);
}

napi_status NAPI_CDECL napi_create_range_error(
    napi_env env,
    napi_value code,
    napi_value message,
    napi_value *result) {
  CHECK_ENV(env);
  return createJSError(
      env, env->runtime_.RangeErrorPrototype, code, message, result);
}

napi_status NAPI_CDECL node_api_create_syntax_error(
    napi_env env,
    napi_value code,
    napi_value message,
    napi_value *result) {
  CHECK_ENV(env);
  return createJSError(
      env, env->runtime_.SyntaxErrorPrototype, code, message, result);
}

napi_status NAPI_CDECL
napi_typeof(napi_env env, napi_value value, napi_valuetype *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
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
    } else if (env->getExternalObjectValue(*hv)) {
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

  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_get_undefined(napi_env env, napi_value *result) {
  CHECK_STATUS(
      checkBasicPreconditions<NodeApiPendingExceptionCheck::kSkip>(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  return env->castResult(&NodeApiEnvironment::UndefinedHermesValue, result);
}

napi_status NAPI_CDECL napi_get_null(napi_env env, napi_value *result) {
  CHECK_STATUS(
      checkBasicPreconditions<NodeApiPendingExceptionCheck::kSkip>(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  return env->castResult(&NodeApiEnvironment::NullHermesValue, result);
}

napi_status NAPI_CDECL napi_get_cb_info(
    napi_env env,
    napi_callback_info callbackInfo,
    size_t *argCount,
    napi_value *args,
    napi_value *thisArg,
    void **data) {
  CHECK_STATUS(checkBasicPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
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

  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_get_new_target(
    napi_env env,
    napi_callback_info callbackInfo,
    napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(callbackInfo);
  CHECK_ARG(result);
  return env->castResult(
      phv(asCallbackInfo(callbackInfo)->getNewTarget()), result);
}

napi_status NAPI_CDECL napi_call_function(
    napi_env env,
    napi_value thisArg,
    napi_value func,
    size_t argCount,
    const napi_value *args,
    napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ result ? 1 : 0);
  CHECK_ARG(thisArg);
  CHECK_ARG(func);
  if (argCount > 0) {
    CHECK_ARG(args);
  }
  RETURN_STATUS_IF_FALSE(vm::vmisa<vm::Callable>(*phv(func)), napi_invalid_arg);

  vm::GCScope gcScope{env->runtime_};

  vm::Handle<vm::Callable> funcHandle = asHandle<vm::Callable>(func);

  if (argCount >= std::numeric_limits<uint32_t>::max() ||
      !env->runtime_.checkAvailableStack(static_cast<uint32_t>(argCount))) {
    return GENERIC_FAILURE("Unable to call function: stack overflow");
  }

  vm::ScopedNativeCallFrame newFrame{
      env->runtime_,
      static_cast<uint32_t>(argCount),
      funcHandle.getHermesValue(),
      /*newTarget:*/ env->UndefinedHermesValue,
      *phv(thisArg)};
  if (LLVM_UNLIKELY(newFrame.overflowed())) {
    CHECK_STATUS(env->checkExecutionStatus(env->runtime_.raiseStackOverflow(
        vm::Runtime::StackOverflowKind::NativeStack)));
  }

  for (uint32_t i = 0; i < argCount; ++i) {
    newFrame->getArgRef(static_cast<int32_t>(i)) = *phv(args[i]);
  }
  vm::CallResult<vm::PseudoHandle<>> callRes =
      vm::Callable::call(funcHandle, env->runtime_);
  CHECK_STATUS(env->checkExecutionStatus(callRes.getStatus()));

  if (result) {
    RETURN_STATUS_IF_FALSE(!callRes->get().isEmpty(), napi_generic_failure);
    return env->makeResultValue(callRes->get(), result);
  }
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_get_global(napi_env env, napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  return env->castResult(
      env->runtime_.getGlobal().unsafeGetPinnedHermesValue(), result);
}

napi_status NAPI_CDECL napi_throw(napi_env env, napi_value error) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(error);
  env->thrownJSError_ = *phv(error);
  // any VM calls after this point and before returning
  // to the JavaScript invoker will fail
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_throw_error(napi_env env, const char *code, const char *message) {
  CHECK_ENV(env);
  return throwJSError(env, env->runtime_.ErrorPrototype, code, message);
}

napi_status NAPI_CDECL
napi_throw_type_error(napi_env env, const char *code, const char *message) {
  CHECK_ENV(env);
  return throwJSError(env, env->runtime_.TypeErrorPrototype, code, message);
}

napi_status NAPI_CDECL
napi_throw_range_error(napi_env env, const char *code, const char *message) {
  CHECK_ENV(env);
  return throwJSError(env, env->runtime_.RangeErrorPrototype, code, message);
}

napi_status NAPI_CDECL node_api_throw_syntax_error(
    napi_env env,
    const char *code,
    const char *message) {
  CHECK_ENV(env);
  return throwJSError(env, env->runtime_.SyntaxErrorPrototype, code, message);
}

napi_status NAPI_CDECL
napi_is_error(napi_env env, napi_value value, bool *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  *result = vm::vmisa<vm::JSError>(*phv(value));
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_get_value_double(napi_env env, napi_value value, double *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  RETURN_STATUS_IF_FALSE(phv(value)->isNumber(), napi_number_expected);
  *result = phv(value)->getDouble();
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_get_value_int32(napi_env env, napi_value value, int32_t *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  RETURN_STATUS_IF_FALSE(phv(value)->isNumber(), napi_number_expected);
  *result = NodeApiDoubleConversion::toInt32(phv(value)->getDouble());
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_get_value_uint32(napi_env env, napi_value value, uint32_t *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  RETURN_STATUS_IF_FALSE(phv(value)->isNumber(), napi_number_expected);
  *result = NodeApiDoubleConversion::toUint32(phv(value)->getDouble());
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_get_value_int64(napi_env env, napi_value value, int64_t *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  RETURN_STATUS_IF_FALSE(phv(value)->isNumber(), napi_number_expected);
  *result = NodeApiDoubleConversion::toInt64(phv(value)->getDouble());
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_get_value_bigint_int64(
    napi_env env,
    napi_value value,
    int64_t *result,
    bool *lossless) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  CHECK_ARG(lossless);
  RETURN_STATUS_IF_FALSE(phv(value)->isBigInt(), napi_bigint_expected);
  vm::BigIntPrimitive *bigInt = phv(value)->getBigInt();
  *lossless =
      bigInt->isTruncationToSingleDigitLossless(/*signedTruncation:*/ true);
  *result = static_cast<int64_t>(bigInt->truncateToSingleDigit());
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_get_value_bigint_uint64(
    napi_env env,
    napi_value value,
    uint64_t *result,
    bool *lossless) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  CHECK_ARG(lossless);
  RETURN_STATUS_IF_FALSE(phv(value)->isBigInt(), napi_bigint_expected);
  vm::BigIntPrimitive *bigInt = phv(value)->getBigInt();
  *lossless =
      bigInt->isTruncationToSingleDigitLossless(/*signedTruncation:*/ false);
  *result = bigInt->truncateToSingleDigit();
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_get_value_bigint_words(
    napi_env env,
    napi_value value,
    int32_t *signBit,
    size_t *wordCount,
    uint64_t *words) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
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

  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_get_value_bool(napi_env env, napi_value value, bool *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  RETURN_STATUS_IF_FALSE(phv(value)->isBool(), napi_boolean_expected);
  *result = phv(value)->getBool();
  return env->clearLastNativeError();
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
    size_t bufSize,
    size_t *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG_IS_STRING(value);

  vm::GCScope gcScope{env->runtime_};

  vm::StringView view = vm::StringPrimitive::createStringView(
      env->runtime_, asHandle<vm::StringPrimitive>(value));

  if (buf == nullptr) {
    CHECK_ARG(result);
    *result = view.length();
  } else if (bufSize != 0) {
    size_t copied = std::min(bufSize - 1, view.length());
    for (auto cur = view.begin(), end = view.begin() + copied; cur < end;
         ++cur) {
      *buf++ = static_cast<char>(*cur);
    }
    *buf = '\0';
    if (result != nullptr) {
      *result = copied;
    }
  } else if (result != nullptr) {
    *result = 0;
  }
  return env->clearLastNativeError();
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
    size_t bufSize,
    size_t *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG_IS_STRING(value);

  vm::GCScope gcScope{env->runtime_};

  vm::StringView view = vm::StringPrimitive::createStringView(
      env->runtime_, asHandle<vm::StringPrimitive>(value));

  if (buf == nullptr) {
    CHECK_ARG(result);
    *result = view.isASCII() || view.length() == 0
        ? view.length()
        : utf8LengthWithReplacements(
              vm::UTF16Ref(view.castToChar16Ptr(), view.length()));
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
    if (result != nullptr) {
      *result = copied;
    }
  } else if (result != nullptr) {
    *result = 0;
  }

  return env->clearLastNativeError();
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
    size_t bufSize,
    size_t *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG_IS_STRING(value);

  vm::GCScope gcScope{env->runtime_};

  vm::StringView view = vm::StringPrimitive::createStringView(
      env->runtime_, asHandle<vm::StringPrimitive>(value));

  if (buf == nullptr) {
    CHECK_ARG(result);
    *result = view.length();
  } else if (bufSize != 0) {
    size_t copied = std::min(bufSize - 1, view.length());
    std::copy(view.begin(), view.begin() + copied, buf);
    buf[copied] = '\0';
    if (result != nullptr) {
      *result = copied;
    }
  } else if (result != nullptr) {
    *result = 0;
  }

  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_coerce_to_bool(napi_env env, napi_value value, napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  bool res = vm::toBoolean(*phv(value));
  *result = napiValue(res ? &env->TrueHermesValue : &env->FalseHermesValue);
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_coerce_to_number(napi_env env, napi_value value, napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(value);
  CHECK_ARG(result);
  vm::GCScope gcScope{env->runtime_};
  vm::CallResult<vm::HermesValue> res =
      vm::toNumber_RJS(env->runtime_, asHandle(value));
  CHECK_STATUS(env->checkExecutionStatus(res.getStatus()));
  return env->makeResultValue(*res, result);
}

napi_status NAPI_CDECL
napi_coerce_to_object(napi_env env, napi_value value, napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(value);
  CHECK_ARG(result);
  vm::GCScope gcScope{env->runtime_};
  vm::CallResult<vm::HermesValue> res =
      vm::toObject(env->runtime_, asHandle(value));
  CHECK_STATUS(env->checkExecutionStatus(res.getStatus()));
  return env->makeResultValue(*res, result);
}

napi_status NAPI_CDECL
napi_coerce_to_string(napi_env env, napi_value value, napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(value);
  CHECK_ARG(result);
  vm::GCScope gcScope{env->runtime_};
  vm::CallResult<vm::PseudoHandle<vm::StringPrimitive>> res =
      vm::toString_RJS(env->runtime_, asHandle(value));
  CHECK_STATUS(env->checkExecutionStatus(res.getStatus()));
  return env->makeResultValue(res->getHermesValue(), result);
}

napi_status NAPI_CDECL napi_wrap(
    napi_env env,
    napi_value object,
    void *nativeData,
    node_api_basic_finalize finalizeCallback,
    void *finalizeHint,
    napi_ref *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG_IS_OBJECT(object);

  NodeApiValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};

  // If we've already wrapped this object, we error out.
  NodeApiExternalValue *externalValue;
  CHECK_STATUS(env->getExternalPropertyValue(
      asHandle<vm::JSObject>(object),
      NodeApiIfNotFound::ThenCreate,
      &externalValue));
  RETURN_STATUS_IF_FALSE(!externalValue->nativeData(), napi_invalid_arg);

  NodeApiReference *reference = nullptr;
  if (result != nullptr) {
    // The returned reference should be deleted via napi_delete_reference()
    // ONLY in response to the finalize callback invocation. (If it is deleted
    // before then, then the finalize callback will never be invoked.)
    // Therefore a finalize callback is required when returning a reference.
    CHECK_ARG(finalizeCallback);
    reference = NodeApiReferenceWithFinalizer::create(
        *env,
        phv(object),
        0,
        NodeApiReferenceOwnership::kUserland,
        nativeData,
        reinterpret_cast<napi_finalize>(finalizeCallback),
        finalizeHint);
    *result = reinterpret_cast<napi_ref>(reference);
  } else if (finalizeCallback != nullptr) {
    // Create a self-deleting reference.
    reference = NodeApiReferenceWithFinalizer::create(
        *env,
        phv(object),
        0,
        NodeApiReferenceOwnership::kRuntime,
        nativeData,
        basicFinalize(finalizeCallback),
        finalizeHint);
  } else {
    // Create a self-deleting reference.
    reference = NodeApiReferenceWithData::create(
        *env, phv(object), 0, NodeApiReferenceOwnership::kRuntime, nativeData);
  }

  externalValue->setNativeData(reference);
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_unwrap(napi_env env, napi_value obj, void **result) {
  return unwrapObject<NodeApiUnwrapAction::KeepWrap>(env, obj, result);
}

napi_status NAPI_CDECL
napi_remove_wrap(napi_env env, napi_value obj, void **result) {
  return unwrapObject<NodeApiUnwrapAction::RemoveWrap>(env, obj, result);
}

napi_status NAPI_CDECL napi_create_external(
    napi_env env,
    void *nativeData,
    node_api_basic_finalize finalizeCallback,
    void *finalizeHint,
    napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);

  vm::GCScope gcScope{env->runtime_};

  vm::Handle<vm::DecoratedObject> decoratedObj =
      env->createExternalObject(nativeData, nullptr);
  if (finalizeCallback != nullptr) {
    NodeApiReferenceWithFinalizer::create(
        *env,
        decoratedObj.unsafeGetPinnedHermesValue(),
        0,
        NodeApiReferenceOwnership::kRuntime,
        nativeData,
        basicFinalize(finalizeCallback),
        finalizeHint);
  }
  return env->makeResultValue(decoratedObj.getHermesValue(), result);
}

// TODO: Update the tag implementation per new code in Node.js
napi_status NAPI_CDECL napi_type_tag_object(
    napi_env env,
    napi_value object,
    const napi_type_tag *typeTag) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(object);
  CHECK_ARG(typeTag);

  NodeApiValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> cr =
      vm::toObject(env->runtime_, asHandle(object));
  CHECK_STATUS(env->checkExecutionStatus(cr.getStatus()));
  vm::Handle<vm::JSObject> objValue =
      env->runtime_.makeHandle<vm::JSObject>(*cr);

  // Fail if the tag already exists
  bool hasTag{};
  CHECK_STATUS(env->hasPredefinedProperty(
      objValue, NodeApiPredefined::napi_typeTag, &hasTag));
  RETURN_STATUS_IF_FALSE(!hasTag, napi_invalid_arg);

  napi_value tagBuffer;
  void *tagBufferData;
  CHECK_STATUS(napi_create_arraybuffer(
      env, sizeof(napi_type_tag), &tagBufferData, &tagBuffer));

  const uint8_t *source = reinterpret_cast<const uint8_t *>(typeTag);
  uint8_t *dest = reinterpret_cast<uint8_t *>(tagBufferData);
  std::copy(source, source + sizeof(napi_type_tag), dest);

  return env->defineOwnProperty(
      objValue,
      env->getPredefinedSymbol(NodeApiPredefined::napi_typeTag),
      vm::DefinePropertyFlags::getNewNonEnumerableFlags(),
      asHandle(tagBuffer),
      nullptr);
}

// TODO: match Node.js code for tags
napi_status NAPI_CDECL napi_check_object_type_tag(
    napi_env env,
    napi_value object,
    const napi_type_tag *typeTag,
    bool *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(object);
  CHECK_ARG(typeTag);
  CHECK_ARG(result);

  NodeApiValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> cr =
      vm::toObject(env->runtime_, asHandle(object));
  CHECK_STATUS(env->checkExecutionStatus(cr.getStatus()));
  vm::Handle<vm::JSObject> objValue =
      env->runtime_.makeHandle<vm::JSObject>(*cr);

  napi_value tagBufferValue;
  CHECK_STATUS(env->getPredefinedProperty(
      objValue, NodeApiPredefined::napi_typeTag, &tagBufferValue));
  vm::JSArrayBuffer *tagBuffer =
      vm::dyn_vmcast_or_null<vm::JSArrayBuffer>(*phv(tagBufferValue));
  if (tagBuffer == nullptr) {
    *result = false;
    return env->clearLastNativeError();
  }

  const uint8_t *source = reinterpret_cast<const uint8_t *>(typeTag);
  const uint8_t *tagBufferData = tagBuffer->getDataBlock(env->runtime_);
  *result = std::equal(
      source,
      source + sizeof(napi_type_tag),
      tagBufferData,
      tagBufferData + sizeof(napi_type_tag));
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_get_value_external(napi_env env, napi_value value, void **result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  NodeApiExternalValue *externalValue =
      env->getExternalObjectValue(*phv(value));
  RETURN_STATUS_IF_FALSE(externalValue != nullptr, napi_invalid_arg);
  *result = externalValue->nativeData();
  return env->clearLastNativeError();
}

// Set initial_refcount to 0 for a weak reference, >0 for a strong reference.
napi_status NAPI_CDECL napi_create_reference(
    napi_env env,
    napi_value value,
    uint32_t initialRefCount,
    napi_ref *result) { // Hermes calls cannot throw JS exceptions here.
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  NodeApiValueScope scope{*env};

  const vm::PinnedHermesValue *hermesValue = phv(value);
  if (env->apiVersion_ < 10) {
    if (!hermesValue->isObject() && !hermesValue->isSymbol()) {
      return ERROR_STATUS(napi_invalid_arg, "Object or Symbol expected.");
    }
  }

  NodeApiReference *reference = NodeApiReference::create(
      *env, hermesValue, initialRefCount, NodeApiReferenceOwnership::kUserland);

  *result = reinterpret_cast<napi_ref>(reference);
  return env->clearLastNativeError();
}

// Deletes a reference. The referenced value is released, and may be GC'd unless
// there are other references to it.
// For a napi_reference returned from `napi_wrap`, this must be called in the
// finalizer.
napi_status NAPI_CDECL napi_delete_reference(napi_env env, napi_ref ref) {
  CHECK_STATUS(
      checkBasicPreconditions<NodeApiPendingExceptionCheck::kSkip>(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(ref);
  delete asReference(ref);
  return env->clearLastNativeError();
}

// Increments the reference count, optionally returning the resulting count.
// After this call the reference will be a strong reference because its
// refcount is >0, and the referenced object is effectively "pinned".
// Calling this when the refcount is 0 and the object is unavailable
// results in an error.
napi_status NAPI_CDECL
napi_reference_ref(napi_env env, napi_ref ref, uint32_t *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(ref);
  uint32_t refCount = asReference(ref)->incRefCount(*env);
  if (result != nullptr) {
    *result = refCount;
  }
  return env->clearLastNativeError();
}

// Decrements the reference count, optionally returning the resulting count. If
// the result is 0 the reference is now weak and the object may be GC'd at any
// time if there are no other references. Calling this when the refcount is
// already 0 results in an error.
napi_status NAPI_CDECL
napi_reference_unref(napi_env env, napi_ref ref, uint32_t *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(ref);
  uint32_t refCount = asReference(ref)->decRefCount(*env);
  if (result != nullptr) {
    *result = refCount;
  }
  return env->clearLastNativeError();
}

// Attempts to get a referenced value. If the reference is weak, the value might
// no longer be available, in that case the call is still successful but the
// result is NULL.
napi_status NAPI_CDECL
napi_get_reference_value(napi_env env, napi_ref ref, napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(ref);
  CHECK_ARG(result);
  *result = NO_RESULT_IF_NULL(asReference(ref)->value(*env));
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_open_handle_scope(napi_env env, napi_handle_scope *result) {
  CHECK_STATUS(
      checkBasicPreconditions<NodeApiPendingExceptionCheck::kSkip>(env));
  CHECK_ARG(result);
  size_t scope = env->napiValueStack_.size();
  env->napiValueStackScopes_.emplace(scope);
  *result =
      reinterpret_cast<napi_handle_scope>(&env->napiValueStackScopes_.top());
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_close_handle_scope(napi_env env, napi_handle_scope scope) {
  CHECK_STATUS(
      checkBasicPreconditions<NodeApiPendingExceptionCheck::kSkip>(env));
  CHECK_ARG(scope);
  RETURN_STATUS_IF_FALSE(
      !env->napiValueStackScopes_.empty(), napi_handle_scope_mismatch);

  size_t *topScope = &env->napiValueStackScopes_.top();
  RETURN_STATUS_IF_FALSE(
      reinterpret_cast<size_t *>(scope) == topScope,
      napi_handle_scope_mismatch);

  env->napiValueStack_.resize(*topScope);
  env->napiValueStackScopes_.pop();
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_open_escapable_handle_scope(
    napi_env env,
    napi_escapable_handle_scope *result) {
  CHECK_STATUS(
      checkBasicPreconditions<NodeApiPendingExceptionCheck::kSkip>(env));
  CHECK_ARG(result);

  env->napiValueStack_.emplace(); // value to escape to parent scope
  env->napiValueStack_.emplace(vm::HermesValue::encodeNativeUInt32(
      NodeApiEnvironment::kEscapableSentinelTag));

  return napi_open_handle_scope(
      env, reinterpret_cast<napi_handle_scope *>(result));
}

napi_status NAPI_CDECL napi_close_escapable_handle_scope(
    napi_env env,
    napi_escapable_handle_scope scope) {
  CHECK_STATUS(checkGCPreconditions<NodeApiPendingExceptionCheck::kSkip>(env));
  CHECK_STATUS(
      napi_close_handle_scope(env, reinterpret_cast<napi_handle_scope>(scope)));

  RETURN_STATUS_IF_FALSE(
      env->napiValueStack_.size() > 1, napi_handle_scope_mismatch);
  vm::PinnedHermesValue &sentinelTag = env->napiValueStack_.top();
  RETURN_STATUS_IF_FALSE(sentinelTag.isDouble(), napi_handle_scope_mismatch);
  uint32_t sentinelTagValue = sentinelTag.getNativeUInt32();
  RETURN_STATUS_IF_FALSE(
      sentinelTagValue == NodeApiEnvironment::kEscapableSentinelTag ||
          sentinelTagValue == NodeApiEnvironment::kUsedEscapableSentinelTag,
      napi_handle_scope_mismatch);

  env->napiValueStack_.pop();
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_escape_handle(
    napi_env env,
    napi_escapable_handle_scope scope,
    napi_value escapee,
    napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_ARG(scope);
  CHECK_ARG(escapee);
  CHECK_ARG(result);

  size_t *stackScope = reinterpret_cast<size_t *>(scope);
  RETURN_STATUS_IF_FALSE(*stackScope > 1, napi_invalid_arg);
  RETURN_STATUS_IF_FALSE(
      *stackScope <= env->napiValueStack_.size(), napi_invalid_arg);

  vm::PinnedHermesValue &sentinelTag = env->napiValueStack_[*stackScope - 1];
  RETURN_STATUS_IF_FALSE(sentinelTag.isDouble(), napi_invalid_arg);
  uint32_t sentinelTagValue = sentinelTag.getNativeUInt32();
  RETURN_STATUS_IF_FALSE(
      sentinelTagValue != NodeApiEnvironment::kUsedEscapableSentinelTag,
      napi_escape_called_twice);
  RETURN_STATUS_IF_FALSE(
      sentinelTagValue == NodeApiEnvironment::kEscapableSentinelTag,
      napi_invalid_arg);

  vm::PinnedHermesValue &escapedValue = env->napiValueStack_[*stackScope - 2];
  escapedValue = *phv(escapee);
  sentinelTag = vm::HermesValue::encodeNativeUInt32(
      NodeApiEnvironment::kUsedEscapableSentinelTag);

  return env->castResult(&escapedValue, result);
}

napi_status NAPI_CDECL napi_new_instance(
    napi_env env,
    napi_value constructor,
    size_t argCount,
    const napi_value *args,
    napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(constructor);
  if (argCount > 0) {
    CHECK_ARG(args);
  }
  CHECK_ARG(result);

  RETURN_STATUS_IF_FALSE(
      vm::vmisa<vm::Callable>(*phv(constructor)), napi_invalid_arg);

  NodeApiEscapableValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};

  vm::Handle<vm::Callable> ctorHandle = asHandle<vm::Callable>(constructor);

  if (argCount >= std::numeric_limits<uint32_t>::max() ||
      !env->runtime_.checkAvailableStack(static_cast<uint32_t>(argCount))) {
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
  vm::CallResult<vm::PseudoHandle<vm::HermesValue>> thisRes =
      vm::Callable::createThisForConstruct_RJS(
          ctorHandle, env->runtime_, ctorHandle);
  if (thisRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  // We need to capture this in case the ctor doesn't return an object,
  // we need to return this object.
  vm::Handle<> thisHandle = env->runtime_.makeHandle(std::move(*thisRes));

  // 13.2.2.8:
  //    Let result be the result of calling the [[Call]] internal property of
  //    F, providing obj as the this value and providing the argument list
  //    passed into [[Construct]] as args.
  //
  // For us result == res.

  vm::ScopedNativeCallFrame newFrame{
      env->runtime_,
      static_cast<uint32_t>(argCount),
      ctorHandle.getHermesValue(),
      ctorHandle.getHermesValue(),
      thisHandle.getHermesValue()};
  if (LLVM_UNLIKELY(newFrame.overflowed())) {
    CHECK_STATUS(env->checkExecutionStatus(env->runtime_.raiseStackOverflow(
        vm::Runtime::StackOverflowKind::NativeStack)));
  }
  for (size_t i = 0; i < argCount; ++i) {
    newFrame->getArgRef(static_cast<int32_t>(i)) = *phv(args[i]);
  }
  // The last parameter indicates that this call should construct an object.
  vm::CallResult<vm::PseudoHandle<>> callRes =
      vm::Callable::call(ctorHandle, env->runtime_);
  CHECK_STATUS(env->checkExecutionStatus(callRes.getStatus()));

  // 13.2.2.9:
  //    If Type(result) is Object then return result
  // 13.2.2.10:
  //    Return obj
  vm::HermesValue resultValue = callRes->get();
  return scope.escapeResult(
      resultValue.isObject() ? resultValue : thisHandle.getHermesValue(),
      result);
}

napi_status NAPI_CDECL napi_instanceof(
    napi_env env,
    napi_value object,
    napi_value constructor,
    bool *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(object);
  CHECK_ARG(constructor);
  CHECK_ARG(result);

  vm::GCScope gcScope{env->runtime_};

  vm::CallResult<vm::HermesValue> ctorRes =
      vm::toObject(env->runtime_, asHandle(constructor));
  if (ctorRes.getStatus() != vm::ExecutionStatus::RETURNED) {
    return env->setJSException();
  }

  if (!vm::vmisa<vm::Callable>(ctorRes.getValue())) {
    return napi_throw_type_error(
        env, "ERR_NAPI_CONS_FUNCTION", "Constructor must be a function");
  }
  vm::CallResult<bool> cr = vm::instanceOfOperator_RJS(
      env->runtime_, asHandle(object), asHandle(constructor));
  CHECK_STATUS(env->checkExecutionStatus(cr.getStatus()));
  *result = *cr;
  return env->clearLastNativeError();
}

// Methods to support catching exceptions
napi_status NAPI_CDECL napi_is_exception_pending(napi_env env, bool *result) {
  CHECK_STATUS(checkGCPreconditions<NodeApiPendingExceptionCheck::kSkip>(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(result);
  *result = !env->thrownJSError_.isEmpty();
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_get_and_clear_last_exception(napi_env env, napi_value *result) {
  CHECK_STATUS(checkGCPreconditions<NodeApiPendingExceptionCheck::kSkip>(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);
  if (env->thrownJSError_.isEmpty()) {
    return napi_get_undefined(env, result);
  }
  return env->makeResultValue(
      std::exchange(env->thrownJSError_, NodeApiEnvironment::EmptyHermesValue),
      result);
}

napi_status NAPI_CDECL
napi_is_arraybuffer(napi_env env, napi_value value, bool *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  *result = vm::vmisa<vm::JSArrayBuffer>(*phv(value));
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_create_arraybuffer(
    napi_env env,
    size_t byteLength,
    void **data,
    napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);

  vm::GCScope gcScope{env->runtime_};
  vm::Handle<vm::JSArrayBuffer> buffer =
      env->runtime_.makeHandle(vm::JSArrayBuffer::create(
          env->runtime_,
          vm::Handle<vm::JSObject>::vmcast(
              &env->runtime_.arrayBufferPrototype)));
  CHECK_STATUS(env->checkExecutionStatus(vm::JSArrayBuffer::createDataBlock(
      env->runtime_, buffer, byteLength, true)));
  if (data != nullptr) {
    *data = buffer->getDataBlock(env->runtime_);
  }
  return env->makeResultValue(buffer.getHermesValue(), result);
}

napi_status NAPI_CDECL napi_create_external_arraybuffer(
    napi_env env,
    void *externalData,
    size_t byteLength,
    node_api_basic_finalize finalizeCallback,
    void *finalizeHint,
    napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);

  vm::GCScope gcScope{env->runtime_};
  vm::Handle<vm::JSArrayBuffer> buffer =
      env->runtime_.makeHandle(vm::JSArrayBuffer::create(
          env->runtime_,
          vm::Handle<vm::JSObject>::vmcast(
              &env->runtime_.arrayBufferPrototype)));
  if (externalData != nullptr) {
    std::unique_ptr<NodeApiExternalBuffer> externalBuffer =
        std::make_unique<NodeApiExternalBuffer>(
            *env,
            externalData,
            byteLength,
            basicFinalize(finalizeCallback),
            finalizeHint);
    CHECK_STATUS(
        env->checkExecutionStatus(vm::JSArrayBuffer::setExternalDataBlock(
            env->runtime_,
            buffer,
            reinterpret_cast<uint8_t *>(externalData),
            byteLength,
            externalBuffer.release(),
            [](vm::GC & /*gc*/, vm::NativeState *ns) {
              std::unique_ptr<NodeApiExternalBuffer> externalBuffer(
                  reinterpret_cast<NodeApiExternalBuffer *>(ns->context()));
            })));
  }
  return env->makeResultValue(buffer.getHermesValue(), result);
}

napi_status NAPI_CDECL napi_get_arraybuffer_info(
    napi_env env,
    napi_value arrayBuffer,
    void **data,
    size_t *byteLength) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(arrayBuffer);
  RETURN_STATUS_IF_FALSE(
      vm::vmisa<vm::JSArrayBuffer>(*phv(arrayBuffer)), napi_invalid_arg);

  vm::JSArrayBuffer *buffer = vm::vmcast<vm::JSArrayBuffer>(*phv(arrayBuffer));
  if (data != nullptr) {
    *data = buffer->attached() ? buffer->getDataBlock(env->runtime_) : nullptr;
  }

  if (byteLength != nullptr) {
    *byteLength = buffer->attached() ? buffer->size() : 0;
  }

  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_is_typedarray(napi_env env, napi_value value, bool *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  *result = vm::vmisa<vm::JSTypedArrayBase>(*phv(value));
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_create_typedarray(
    napi_env env,
    napi_typedarray_type type,
    size_t length,
    napi_value arrayBuffer,
    size_t byteOffset,
    napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(arrayBuffer);
  CHECK_ARG(result);

  NodeApiEscapableValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};

  vm::JSArrayBuffer *buffer =
      vm::dyn_vmcast_or_null<vm::JSArrayBuffer>(*phv(arrayBuffer));
  RETURN_STATUS_IF_FALSE(buffer != nullptr, napi_invalid_arg);

  vm::MutableHandle<vm::JSTypedArrayBase> typedArray{env->runtime_};
  switch (type) {
    case napi_int8_array:
      CHECK_STATUS(env->createTypedArray<int8_t, vm::CellKind::Int8ArrayKind>(
          length, buffer, byteOffset, &typedArray));
      break;
    case napi_uint8_array:
      CHECK_STATUS(env->createTypedArray<uint8_t, vm::CellKind::Uint8ArrayKind>(
          length, buffer, byteOffset, &typedArray));
      break;
    case napi_uint8_clamped_array:
      CHECK_STATUS(
          env->createTypedArray<uint8_t, vm::CellKind::Uint8ClampedArrayKind>(
              length, buffer, byteOffset, &typedArray));
      break;
    case napi_int16_array:
      CHECK_STATUS(env->createTypedArray<int16_t, vm::CellKind::Int16ArrayKind>(
          length, buffer, byteOffset, &typedArray));
      break;
    case napi_uint16_array:
      CHECK_STATUS(
          env->createTypedArray<uint16_t, vm::CellKind::Uint16ArrayKind>(
              length, buffer, byteOffset, &typedArray));
      break;
    case napi_int32_array:
      CHECK_STATUS(env->createTypedArray<int32_t, vm::CellKind::Int32ArrayKind>(
          length, buffer, byteOffset, &typedArray));
      break;
    case napi_uint32_array:
      CHECK_STATUS(
          env->createTypedArray<uint32_t, vm::CellKind::Uint32ArrayKind>(
              length, buffer, byteOffset, &typedArray));
      break;
    case napi_float32_array:
      CHECK_STATUS(env->createTypedArray<float, vm::CellKind::Float32ArrayKind>(
          length, buffer, byteOffset, &typedArray));
      break;
    case napi_float64_array:
      CHECK_STATUS(
          env->createTypedArray<double, vm::CellKind::Float64ArrayKind>(
              length, buffer, byteOffset, &typedArray));
      break;
    case napi_bigint64_array:
      CHECK_STATUS(
          env->createTypedArray<int64_t, vm::CellKind::BigInt64ArrayKind>(
              length, buffer, byteOffset, &typedArray));
      break;
    case napi_biguint64_array:
      CHECK_STATUS(
          env->createTypedArray<uint64_t, vm::CellKind::BigUint64ArrayKind>(
              length, buffer, byteOffset, &typedArray));
      break;
    default:
      return ERROR_STATUS(
          napi_invalid_arg, "Unsupported TypedArray type: ", type);
  }

  return scope.escapeResult(typedArray.getHermesValue(), result);
}

napi_status NAPI_CDECL napi_get_typedarray_info(
    napi_env env,
    napi_value typedArray,
    napi_typedarray_type *type,
    size_t *length,
    void **data,
    napi_value *arrayBuffer,
    size_t *byteOffset) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
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
    *data = array->attached(env->runtime_)
        ? array->getBuffer(env->runtime_)->getDataBlock(env->runtime_) +
            array->getByteOffset()
        : nullptr;
  }

  if (arrayBuffer != nullptr) {
    *arrayBuffer = array->attached(env->runtime_)
        ? env->pushNewNodeApiValue(vm::HermesValue::encodeObjectValue(
              array->getBuffer(env->runtime_)))
        : napiValue(&env->UndefinedHermesValue);
  }

  if (byteOffset != nullptr) {
    *byteOffset = array->getByteOffset();
  }

  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_create_dataview(
    napi_env env,
    size_t byteLength,
    napi_value arrayBuffer,
    size_t byteOffset,
    napi_value *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(arrayBuffer);
  CHECK_ARG(result);

  vm::GCScope gcScope{env->runtime_};

  vm::JSArrayBuffer *buffer =
      vm::dyn_vmcast_or_null<vm::JSArrayBuffer>(*phv(arrayBuffer));
  RETURN_STATUS_IF_FALSE(buffer != nullptr, napi_invalid_arg);

  if (byteLength + byteOffset > buffer->size()) {
    napi_throw_range_error(
        env,
        "ERR_NAPI_INVALID_DATAVIEW_ARGS",
        "byte_offset + byte_length should be less than or "
        "equal to the size in bytes of the array passed in");
    return ERROR_STATUS(napi_pending_exception);
  }
  vm::Handle<vm::JSDataView> viewHandle =
      env->runtime_.makeHandle(vm::JSDataView::create(
          env->runtime_,
          vm::Handle<vm::JSObject>::vmcast(&env->runtime_.dataViewPrototype)));
  viewHandle->setBuffer(env->runtime_, buffer, byteOffset, byteLength);
  return env->makeResultValue(viewHandle.getHermesValue(), result);
}

napi_status NAPI_CDECL
napi_is_dataview(napi_env env, napi_value value, bool *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  *result = vm::vmisa<vm::JSDataView>(*phv(value));
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_get_dataview_info(
    napi_env env,
    napi_value dataView,
    size_t *byteLength,
    void **data,
    napi_value *arrayBuffer,
    size_t *byteOffset) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(dataView);

  vm::GCScope gcScope{env->runtime_}; // for view->getBuffer

  vm::JSDataView *view = vm::dyn_vmcast_or_null<vm::JSDataView>(*phv(dataView));
  RETURN_STATUS_IF_FALSE(view, napi_invalid_arg);

  if (byteLength != nullptr) {
    *byteLength = view->byteLength();
  }

  if (data != nullptr) {
    *data = view->attached(env->runtime_)
        ? view->getBuffer(env->runtime_)->getDataBlock(env->runtime_) +
            view->byteOffset()
        : nullptr;
  }

  if (arrayBuffer != nullptr) {
    *arrayBuffer = view->attached(env->runtime_)
        ? env->pushNewNodeApiValue(
              view->getBuffer(env->runtime_).getHermesValue())
        : napiValue(&env->UndefinedHermesValue);
  }

  if (byteOffset != nullptr) {
    *byteOffset = view->byteOffset();
  }

  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_get_version(node_api_basic_env basic_env, uint32_t *result) {
  napi_env env = const_cast<napi_env>(basic_env);
  CHECK_STATUS(checkBasicPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(result);
  *result = static_cast<uint32_t>(env->apiVersion_);
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_create_promise(
    napi_env env,
    napi_deferred *deferred,
    napi_value *promise) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(deferred);
  CHECK_ARG(promise);

  NodeApiEscapableValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};

  napi_value jsPromise, jsDeferred;
  vm::MutableHandle<> jsResolve{env->runtime_};
  vm::MutableHandle<> jsReject{env->runtime_};
  CHECK_STATUS(env->createPromise(&jsPromise, &jsResolve, &jsReject));

  CHECK_STATUS(napi_create_object(env, &jsDeferred));
  CHECK_STATUS(env->setPredefinedProperty(
      asHandle<vm::JSObject>(jsDeferred),
      NodeApiPredefined::resolve,
      jsResolve));
  CHECK_STATUS(env->setPredefinedProperty(
      asHandle<vm::JSObject>(jsDeferred), NodeApiPredefined::reject, jsReject));

  *deferred = reinterpret_cast<napi_deferred>(NodeApiReference::create(
      *env, phv(jsDeferred), 1, NodeApiReferenceOwnership::kUserland));
  return scope.escapeResult(*phv(jsPromise), promise);
}

napi_status NAPI_CDECL napi_resolve_deferred(
    napi_env env,
    napi_deferred deferred,
    napi_value resolution) {
  return concludeDeferred(
      env, deferred, NodeApiPredefined::resolve, resolution);
}

napi_status NAPI_CDECL napi_reject_deferred(
    napi_env env,
    napi_deferred deferred,
    napi_value resolution) {
  return concludeDeferred(env, deferred, NodeApiPredefined::reject, resolution);
}

napi_status NAPI_CDECL
napi_is_promise(napi_env env, napi_value value, bool *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  NodeApiValueScope scope{*env};

  napi_value global{}, promiseConstructor{};
  CHECK_STATUS(napi_get_global(env, &global));
  CHECK_STATUS(env->getPredefinedProperty(
      asHandle<vm::JSObject>(global),
      NodeApiPredefined::Promise,
      &promiseConstructor));

  return napi_instanceof(env, value, promiseConstructor, result);
}

napi_status NAPI_CDECL
napi_create_date(napi_env env, double dateTime, napi_value *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(result);
  vm::PseudoHandle<vm::JSDate> dateHandle = vm::JSDate::create(
      env->runtime_,
      dateTime,
      vm::Handle<vm::JSObject>::vmcast(&env->runtime_.datePrototype));
  return env->makeResultValue(dateHandle.getHermesValue(), result);
}

napi_status NAPI_CDECL
napi_is_date(napi_env env, napi_value value, bool *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  *result = vm::vmisa<vm::JSDate>(*phv(value));
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_get_date_value(napi_env env, napi_value value, double *result) {
  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(value);
  CHECK_ARG(result);
  vm::JSDate *date = vm::dyn_vmcast_or_null<vm::JSDate>(*phv(value));
  RETURN_STATUS_IF_FALSE(date != nullptr, napi_date_expected);
  *result = date->getPrimitiveValue();
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_run_script(napi_env env, napi_value script, napi_value *result) {
  class StringBuffer : public ::hermes::Buffer {
   public:
    StringBuffer(std::string buffer) : string_(std::move(buffer)) {
      data_ = reinterpret_cast<const uint8_t *>(string_.c_str());
      size_ = string_.size();
    }

   private:
    std::string string_;
  };

  CHECK_STATUS(checkJSPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 1);
  CHECK_ARG(script);
  CHECK_ARG(result);

  NodeApiEscapableValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};

  // Convert the code into UTF8.
  size_t sourceSize{};
  CHECK_STATUS(
      napi_get_value_string_utf8(env, script, nullptr, 0, &sourceSize));
  std::string code(sourceSize, '\0');
  CHECK_STATUS(napi_get_value_string_utf8(
      env, script, &code[0], sourceSize + 1, nullptr));

  // Create a buffer for the code.
  std::unique_ptr<hermes::Buffer> codeBuffer(new StringBuffer(std::move(code)));

  vm::CallResult<vm::HermesValue> cr = env->runtime_.run(
      std::move(codeBuffer), llvh::StringRef(), env->compileFlags_);
  CHECK_STATUS(env->checkExecutionStatus(cr.getStatus()));
  return scope.escapeResult(*cr, result);
}

napi_status NAPI_CDECL napi_add_finalizer(
    napi_env env,
    napi_value object,
    void *nativeData,
    node_api_basic_finalize finalizeCallback,
    void *finalizeHint,
    napi_ref *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG_IS_OBJECT(object);
  CHECK_ARG(finalizeCallback);

  NodeApiValueScope scope{*env};
  vm::GCScope gcScope{env->runtime_};

  // Create a self-deleting reference if the optional out-param result is not
  // set.
  NodeApiReferenceOwnership ownership = result == nullptr
      ? NodeApiReferenceOwnership::kRuntime
      : NodeApiReferenceOwnership::kUserland;
  NodeApiReference *reference = NodeApiReferenceWithFinalizer::create(
      *env,
      phv(object),
      0,
      ownership,
      nativeData,
      basicFinalize(finalizeCallback),
      finalizeHint);
  if (result != nullptr) {
    *result = reinterpret_cast<napi_ref>(reference);
  }
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL node_api_post_finalizer(
    node_api_basic_env basic_env,
    napi_finalize finalizeCallback,
    void *finalizeData,
    void *finalizeHint) {
  napi_env env = const_cast<napi_env>(basic_env);
  CHECK_STATUS(checkBasicPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(finalizeCallback);

  NodeApiValueScope scope{*env};

  NodeApiTrackedFinalizer *finalizer = NodeApiTrackedFinalizer::create(
      *env, finalizeData, finalizeCallback, finalizeHint);
  if (finalizer == nullptr) {
    return GENERIC_FAILURE("Failed to create finalizer");
  }
  env->enqueueFinalizer(finalizer);

  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_adjust_external_memory(
    node_api_basic_env basic_env,
    int64_t change_in_bytes,
    int64_t *adjusted_value) {
  // TODO: Implement napi_adjust_external_memory (check the JSArrayBuffer detach
  // as example)
  napi_env env = const_cast<napi_env>(basic_env);
  CHECK_STATUS(checkBasicPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  return GENERIC_FAILURE("Not implemented");
}

napi_status NAPI_CDECL napi_set_instance_data(
    node_api_basic_env basic_env,
    void *nativeData,
    napi_finalize finalizeCallback,
    void *finalizeHint) {
  napi_env env = const_cast<napi_env>(basic_env);
  CHECK_STATUS(checkBasicPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  NodeApiTrackedFinalizer *oldData = std::exchange(env->instanceData_, nullptr);
  if (oldData != nullptr) {
    // Our contract so far has been to not finalize any old data there may be.
    // So we simply delete it.
    delete oldData;
  }
  env->instanceData_ = NodeApiTrackedFinalizer::create(
      *env, nativeData, finalizeCallback, finalizeHint);
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_get_instance_data(node_api_basic_env basic_env, void **nativeData) {
  napi_env env = const_cast<napi_env>(basic_env);
  CHECK_STATUS(checkBasicPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(nativeData);
  *nativeData = env->instanceData_ != nullptr ? env->instanceData_->nativeData()
                                              : nullptr;
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL
napi_detach_arraybuffer(napi_env env, napi_value arrayBuffer) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(arrayBuffer);
  RETURN_STATUS_IF_FALSE(
      vm::vmisa<vm::JSArrayBuffer>(*phv(arrayBuffer)),
      napi_arraybuffer_expected);
  vm::ExecutionStatus status = vm::JSArrayBuffer::detach(
      env->runtime_, asHandle<vm::JSArrayBuffer>(arrayBuffer));
  if (status == vm::ExecutionStatus::EXCEPTION) {
    return env->setJSException();
  }
  return env->clearLastNativeError();
}

napi_status NAPI_CDECL napi_is_detached_arraybuffer(
    napi_env env,
    napi_value arrayBuffer,
    bool *result) {
  CHECK_STATUS(checkGCPreconditions(env));
  CHECK_POSTCONDITIONS(env, /*valueStackDelta:*/ 0);
  CHECK_ARG(arrayBuffer);
  CHECK_ARG(result);
  vm::JSArrayBuffer *buffer =
      vm::dyn_vmcast_or_null<vm::JSArrayBuffer>(*phv(arrayBuffer));
  *result = buffer != nullptr && !buffer->attached();
  return env->clearLastNativeError();
}
