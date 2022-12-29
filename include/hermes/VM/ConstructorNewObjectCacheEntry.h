/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_CONSTRUCTORNEWOBJECTCACHEENTRY_H
#define HERMES_VM_CONSTRUCTORNEWOBJECTCACHEENTRY_H

#include "hermes/Support/CheckedMalloc.h"
#include "hermes/VM/HiddenClass.h"
#include "hermes/VM/WeakRoot-inline.h"
#include "hermes/VM/WeakRoot.h"

#include "llvh/ADT/ArrayRef.h"
#include "llvh/Support/TrailingObjects.h"

namespace hermes {
namespace vm {

/// Stores the state for an entry in the constructor new object cache. It is
/// intended to be constructed with a fixed set of hidden classes for the
/// parent. After instantiation, the user of this class can separately cache the
/// actual object hidden class to be used.
/// It is safe to just cache weakroots for the parent classes, since they will
/// remain alive as long as the prototype objects remain alive and unmodified.
/// If they become unreachable, this cache is invalidated.
class ConstructorNewObjectCacheEntry final : private llvh::TrailingObjects<
                                                 ConstructorNewObjectCacheEntry,
                                                 WeakRoot<HiddenClass>> {
  friend TrailingObjects;

  /// This enum describes how to interpret the cache state from a
  /// ConstructorNewObjectCacheEntry*. Failed is deliberately equal to 0, to
  /// make it as fast and cheap to inline a failure check as possible.
  enum { Failed, Uninitialized, FirstPtr };

 public:
  /// Create a new ConstructorNewObjectCacheEntry for the given \p protoClazzes
  /// and return a pointer to it. Note that this does not populate the hidden
  /// class for the object itself, the caller must set that separately. The
  /// entry is allocated using malloc, and the caller is responsible for calling
  /// free() on it once it is no longer in use. This is safe to do since the
  /// entry is trivially destructible.
  static ConstructorNewObjectCacheEntry *create(
      Runtime &runtime,
      llvh::ArrayRef<HiddenClass *> protoClazzes) {
    auto sz = totalSizeToAlloc<WeakRoot<HiddenClass>>(protoClazzes.size());
    void *mem = checkedMalloc(sz);
    return new (mem) ConstructorNewObjectCacheEntry(runtime, protoClazzes);
  }

  /// Get the cached clazz_. Executes a read barrier, so the returned value is
  /// safe to be stored.
  HiddenClass *getClazz(Runtime &runtime) const {
    return clazz_.get(runtime, runtime.getHeap());
  }
  /// Set the cached clazz_ to \p clazz.
  void setClazz(PointerBase &base, HiddenClass *clazz) {
    clazz_.set(base, clazz);
  }

  /// Get the HiddenClasses corresponding to the prototype chain.
  llvh::ArrayRef<WeakRoot<HiddenClass>> getProtoClazzes() {
    return {getProtoClazzesPtr(), numProtoClazzes_};
  }

  /// Mark the WeakRoots owned by this entry. The owner of this entry is
  /// responsible for calling this method to ensure the WeakRoots are updated
  /// (for instance if the classes become unreachable or are moved).
  void markCachedHiddenClasses(WeakRootAcceptor &acceptor);

  /// Create a pointer representing a cache entry that is uninitialized.
  static ConstructorNewObjectCacheEntry *uninitialized() {
    return (ConstructorNewObjectCacheEntry *)Uninitialized;
  }
  /// Create a pointer representing that caching has failed for this entry.
  static ConstructorNewObjectCacheEntry *failed() {
    return (ConstructorNewObjectCacheEntry *)Failed;
  }
  /// \return true if the entry points to a valid cache entry.
  static bool isInitialized(ConstructorNewObjectCacheEntry *entry) {
    return (uintptr_t)entry >= FirstPtr;
  }

 private:
  ConstructorNewObjectCacheEntry(
      Runtime &runtime,
      llvh::ArrayRef<HiddenClass *> protoClazzes);

  WeakRoot<HiddenClass> *getProtoClazzesPtr() {
    return getTrailingObjects<WeakRoot<HiddenClass>>();
  }

  /// The number of cached hidden classes for the prototype chain.
  size_t numProtoClazzes_;
  /// The cached class to be used for a newly created object.
  WeakRoot<HiddenClass> clazz_;
};

static_assert(
    std::is_trivially_destructible_v<ConstructorNewObjectCacheEntry>,
    "Entries must be trivially destructible.");

} // namespace vm
} // namespace hermes
#endif // HERMES_VM_CONSTRUCTORNEWOBJECTCACHEENTRY_H
