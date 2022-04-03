/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSMAPIMPL_H
#define HERMES_VM_JSMAPIMPL_H

#include "hermes/VM/Callable.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/IterationKind.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/OrderedHashMap.h"

namespace hermes {
namespace vm {

/// The implementation of a JSMap object. Internally, a Map works the same way
/// as a Set, and hence we share the same implementation for them, with
/// different CellKind.
template <CellKind C>
class JSMapImpl final : public JSObject {
  using Super = JSObject;

 public:
  static const ObjectVTable vt;

  static constexpr CellKind getCellKind() {
    return C;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == C;
  }

  static PseudoHandle<JSMapImpl<C>> create(
      Runtime &runtime,
      Handle<JSObject> parentHandle);

  /// Allocate the internal element storage.
  static ExecutionStatus initializeStorage(
      Handle<JSMapImpl> self,
      Runtime &runtime) {
    auto crtRes = OrderedHashMap::create(runtime);
    if (LLVM_UNLIKELY(crtRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    auto storageHandle = runtime.makeHandle<OrderedHashMap>(std::move(*crtRes));
    self->storage_.set(runtime, storageHandle.get(), runtime.getHeap());
    return ExecutionStatus::RETURNED;
  }

  /// Advance iterator and return the next.
  HashMapEntry *iteratorNext(Runtime &runtime, HashMapEntry *entry) {
    return storage_.getNonNull(runtime)->iteratorNext(runtime, entry);
  }

  /// Add a value.
  static void addValue(
      Handle<JSMapImpl> self,
      Runtime &runtime,
      Handle<> key,
      Handle<> value) {
    self->assertInitialized();
    OrderedHashMap::insert(
        runtime.makeHandle<OrderedHashMap>(self->storage_),
        runtime,
        key,
        value);
  }

  /// \return true if a key exists.
  static bool hasKey(Handle<JSMapImpl> self, Runtime &runtime, Handle<> key) {
    self->assertInitialized();
    return OrderedHashMap::has(
        runtime.makeHandle<OrderedHashMap>(self->storage_), runtime, key);
  }

  static HermesValue
  getValue(Handle<JSMapImpl> self, Runtime &runtime, Handle<> key) {
    self->assertInitialized();
    return OrderedHashMap::get(
        runtime.makeHandle<OrderedHashMap>(self->storage_), runtime, key);
  }

  /// Delelet a key. \return true if succeeds.
  static bool
  deleteKey(Handle<JSMapImpl> self, Runtime &runtime, Handle<> key) {
    self->assertInitialized();
    return OrderedHashMap::erase(
        runtime.makeHandle<OrderedHashMap>(self->storage_), runtime, key);
  }

  /// \returns the size.
  static uint32_t getSize(JSMapImpl *self, Runtime &runtime) {
    self->assertInitialized();
    return self->storage_.getNonNull(runtime)->size();
  }

  /// Clear all elements from the storage.
  static void clear(Handle<JSMapImpl> self, Runtime &runtime) {
    self->assertInitialized();
    self->storage_.getNonNull(runtime)->clear(runtime);
  }

  /// Call \p callbackfn for each entry, with \p thisArg as this.
  static ExecutionStatus forEach(
      Handle<JSMapImpl> self,
      Runtime &runtime,
      Handle<Callable> callbackfn,
      Handle<> thisArg) {
    self->assertInitialized();
    MutableHandle<HashMapEntry> entry{runtime};
    GCScopeMarkerRAII marker{runtime};
    for (entry = self->storage_.getNonNull(runtime)->iteratorNext(runtime);
         entry;
         entry = self->storage_.getNonNull(runtime)->iteratorNext(
             runtime, entry.get())) {
      marker.flush();
      HermesValue key = entry->key;
      HermesValue value = entry->value;
      assert(!key.isEmpty() && "Invalid key encountered");
      assert(!value.isEmpty() && "Invalid value encountered");
      if (LLVM_UNLIKELY(
              Callable::executeCall3(
                  callbackfn,
                  runtime,
                  thisArg,
                  value,
                  key,
                  self.getHermesValue()) == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
    }
    return ExecutionStatus::RETURNED;
  }

  /// Build the metadata for this map implementation, and store it into \p mb.
  static void MapOrSetBuildMeta(const GCCell *cell, Metadata::Builder &mb);

  JSMapImpl(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz)
      : JSObject(runtime, *parent, *clazz) {}

 private:
  /// The underlying storage.
  GCPointer<OrderedHashMap> storage_{nullptr};

  void assertInitialized() {
    assert(storage_ && "Element storage uninitialized.");
  }
};

/// JSMapTypeTraits binds iterator type and its corresponding container type.
template <CellKind T>
struct JSMapTypeTraits;
template <>
struct JSMapTypeTraits<CellKind::JSSetIteratorKind> {
  static constexpr CellKind ContainerKind = CellKind::JSSetKind;
};
template <>
struct JSMapTypeTraits<CellKind::JSMapIteratorKind> {
  static constexpr CellKind ContainerKind = CellKind::JSMapKind;
};

template <CellKind C>
class JSMapIteratorImpl final : public JSObject {
  using Super = JSObject;

 public:
  static const ObjectVTable vt;

  static constexpr CellKind getCellKind() {
    return C;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == C;
  }

  /// Create a handle of JSMapIterator.
  static PseudoHandle<JSMapIteratorImpl<C>> create(
      Runtime &runtime,
      Handle<JSObject> prototype);

  /// Initialize the iterator, set its targeting Set data and iteration kind.
  /// The \p gc parameter is necessary for write barriers.
  void initializeIterator(
      Runtime &runtime,
      Handle<JSMapImpl<JSMapTypeTraits<C>::ContainerKind>> data,
      IterationKind kind) {
    data_.set(runtime, data.get(), runtime.getHeap());
    iterationKind_ = kind;

    assert(data_ && "Invalid storage data");
  }

  /// Check if the iterator has been properly initialized.
  /// data_ is nullptr either when the iterator is not initialized,
  /// or iteration has finished.
  bool isInitialized() const {
    return data_ || iterationFinished_;
  }

  /// Iterate to the next element and return.
  static CallResult<HermesValue> nextElement(
      Handle<JSMapIteratorImpl> self,
      Runtime &runtime) {
    MutableHandle<> value{runtime};
    if (!self->iterationFinished_) {
      // Iteration has not yet reached the end previously.
      assert(self->data_ && "Storage uninitialized");
      // Advance the iterator.
      self->itr_.set(
          runtime,
          self->data_.getNonNull(runtime)->iteratorNext(
              runtime, self->itr_.get(runtime)),
          runtime.getHeap());
      if (self->itr_) {
        switch (self->iterationKind_) {
          case IterationKind::Key:
            value = self->itr_.getNonNull(runtime)->key;
            break;
          case IterationKind::Value:
            value = self->itr_.getNonNull(runtime)->value;
            break;
          case IterationKind::Entry: {
            // If we are iterating both key and value, we need to create an
            // array.
            auto arrRes = JSArray::create(runtime, 2, 2);
            if (arrRes == ExecutionStatus::EXCEPTION) {
              return ExecutionStatus::EXCEPTION;
            }
            auto arrHandle = *arrRes;
            value = self->itr_.getNonNull(runtime)->key;
            JSArray::setElementAt(arrHandle, runtime, 0, value);
            value = self->itr_.getNonNull(runtime)->value;
            JSArray::setElementAt(arrHandle, runtime, 1, value);
            value = arrHandle.getHermesValue();
            break;
          };
          case IterationKind::NumKinds:
            llvm_unreachable("Invalid iteration kind");
            return HermesValue::encodeEmptyValue();
        }
      } else {
        // If the next element in the iterator is invalid, we have
        // reached the end.
        self->iterationFinished_ = true;
        self->data_.setNull(runtime.getHeap());
      }
    }
    return createIterResultObject(runtime, value, self->iterationFinished_)
        .getHermesValue();
  }

  /// Build the metadata for this map implementation, and store it into \p mb.
  static void MapOrSetIteratorBuildMeta(
      const GCCell *cell,
      Metadata::Builder &mb);

  JSMapIteratorImpl(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz)
      : JSObject(runtime, *parent, *clazz) {}

 private:
  /// The internal pointer to the Map data. nullptr if the iterator has not been
  /// initialized or the iteration has ended.
  GCPointer<JSMapImpl<JSMapTypeTraits<C>::ContainerKind>> data_{nullptr};

  /// Iteration pointer to the element storage in the Map.
  GCPointer<HashMapEntry> itr_{nullptr};

  IterationKind iterationKind_;

  /// Indicating whether iteration has reached the end.
  bool iterationFinished_{false};
};

using JSMap = JSMapImpl<CellKind::JSMapKind>;
using JSSet = JSMapImpl<CellKind::JSSetKind>;
using JSMapIterator = JSMapIteratorImpl<CellKind::JSMapIteratorKind>;
using JSSetIterator = JSMapIteratorImpl<CellKind::JSSetIteratorKind>;

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JSMAPIMPL_H
