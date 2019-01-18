/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/JSWeakMapImpl.h"

#include "hermes/VM/WeakRefHolder.h"

namespace hermes {
namespace vm {

void JSWeakMapImplBase::WeakMapImplBaseBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSWeakMapImplBase *>(cell);
  mb.addField("@valueStorage", &self->valueStorage_);
}

/// Set a key/value, overwriting the previous value at that key,
/// or add a new key/value if the key doesn't exist.
ExecutionStatus JSWeakMapImplBase::setValue(
    Handle<JSWeakMapImplBase> self,
    Runtime *runtime,
    Handle<JSObject> key,
    Handle<> value) {
  WeakRefHolder<JSObject> weakRef(runtime, key);
  WeakRefKey mapKey(*weakRef, runtime->gcStableHashHermesValue(key));
  DenseMapT::iterator it = self->map_.find(mapKey);

  if (it != self->map_.end()) {
    // Key already exists, update existing value.
    assert(it->second < self->valueStorage_->size() && "invalid index");
    self->valueStorage_->at(it->second).set(*value, &runtime->getHeap());
    return ExecutionStatus::RETURNED;
  }

  // Index in valueStorage_ in which to place the new element.
  auto cr = getFreeValueStorageIndex(self, runtime);
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint32_t i = *cr;

  auto result = self->map_.try_emplace(mapKey, i);
  (void)result;
  assert(result.second && "unable to add a new value to map");

  self->valueStorage_->at(i).set(*value, &runtime->getHeap());
  return ExecutionStatus::RETURNED;
}

/// Delete a key/value in the map.
/// \return true if the key/value existed and was removed.
bool JSWeakMapImplBase::deleteValue(
    Handle<JSWeakMapImplBase> self,
    Runtime *runtime,
    Handle<JSObject> key) {
  WeakRefHolder<JSObject> weakRef(runtime, key);
  WeakRefKey mapKey(*weakRef, runtime->gcStableHashHermesValue(key));
  DenseMapT::iterator it = self->map_.find(mapKey);
  if (it == self->map_.end()) {
    return false;
  }
  self->deleteInternal(it);
  return true;
}

/// \return true if the \p key exists in the map.
bool JSWeakMapImplBase::hasValue(
    Handle<JSWeakMapImplBase> self,
    Runtime *runtime,
    Handle<JSObject> key) {
  WeakRefHolder<JSObject> weakRef(runtime, key);
  WeakRefKey mapKey(*weakRef, runtime->gcStableHashHermesValue(key));
  DenseMapT::iterator it = self->map_.find_as(mapKey);
  return it != self->map_.end();
}

HermesValue JSWeakMapImplBase::getValue(
    Handle<JSWeakMapImplBase> self,
    Runtime *runtime,
    Handle<JSObject> key) {
  WeakRefHolder<JSObject> weakRef(runtime, key);
  WeakRefKey mapKey(*weakRef, runtime->gcStableHashHermesValue(key));
  DenseMapT::iterator it = self->map_.find(mapKey);
  if (it == self->map_.end()) {
    return HermesValue::encodeUndefinedValue();
  }
  return self->valueStorage_->at(it->second);
}

/// Mark weak references and remove any invalid weak refs.
void JSWeakMapImplBase::markWeakRefs(GC *gc) {
  for (auto it = map_.begin(); it != map_.end(); ++it) {
    if (it->first.ref.isValid()) {
      // If the reference is valid, then mark the weak ref.
      gc->markWeakRef(it->first.ref);
    } else {
      // Otherwise, clear the value and remove the key from the map.
      deleteInternal(it);
    }
  }
}

void JSWeakMapImplBase::deleteInternal(
    JSWeakMapImplBase::DenseMapT::iterator it) {
  assert(it != map_.end() && "Invalid iterator to deleteInternal");
  valueStorage_->at(it->second)
      .setNonPtr(HermesValue::encodeNativeUInt32(freeListHead_));
  freeListHead_ = it->second;
  map_.erase(it);
}

CallResult<uint32_t> JSWeakMapImplBase::getFreeValueStorageIndex(
    Handle<JSWeakMapImplBase> self,
    Runtime *runtime) {
  // Index in valueStorage_ in which to place the new element.
  uint32_t i;
  // True if using the nextIndex field to get i.
  bool useNextIndex;
  if (self->freeListHead_ == kFreeListInvalid) {
    // No elements in the free list.
    i = self->nextIndex_;
    if (LLVM_UNLIKELY(self->nextIndex_ == UINT32_MAX)) {
      return runtime->raiseRangeError("Out of space for elements in map");
    }
    useNextIndex = true;
  } else {
    // Free list has an element, use it.
    i = self->freeListHead_;
    useNextIndex = false;
  }

  auto storageHandle = runtime->makeMutableHandle(self->valueStorage_);
  if (i >= storageHandle->size()) {
    if (LLVM_UNLIKELY(
            BigStorage::resize(
                storageHandle,
                runtime,
                std::max(i + 1, storageHandle->size() * 2)) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  // Update internal state here to ensure we don't corrupt it on exception.
  if (useNextIndex) {
    ++self->nextIndex_;
  } else {
    // Set the start of the free list to the next element.
    // If the next element is kFreeListInvalid, the free list is now empty.
    self->freeListHead_ = self->valueStorage_->at(i).getNativeUInt32();
  }

  assert(i < storageHandle->size() && "invalid index");
  self->valueStorage_.set(*storageHandle, &runtime->getHeap());

  return i;
}

template <CellKind C>
void JSWeakMapImpl<C>::WeakMapOrSetBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  WeakMapImplBaseBuildMeta(cell, mb);
}

void WeakMapBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSWeakMapImpl<CellKind::WeakMapKind>::WeakMapOrSetBuildMeta(cell, mb);
}

void WeakSetBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSWeakMapImpl<CellKind::WeakSetKind>::WeakMapOrSetBuildMeta(cell, mb);
}

template <CellKind C>
const ObjectVTable JSWeakMapImpl<C>::vt{
    VTable(
        C,
        sizeof(JSWeakMapImpl),
        JSWeakMapImpl::_finalizeImpl,
        JSWeakMapImpl::_markWeakImpl,
        JSWeakMapImpl::_mallocSizeImpl),
    JSWeakMapImpl::_getOwnIndexedRangeImpl,
    JSWeakMapImpl::_haveOwnIndexedImpl,
    JSWeakMapImpl::_getOwnIndexedPropertyFlagsImpl,
    JSWeakMapImpl::_getOwnIndexedImpl,
    JSWeakMapImpl::_setOwnIndexedImpl,
    JSWeakMapImpl::_deleteOwnIndexedImpl,
    JSWeakMapImpl::_checkAllOwnIndexedImpl,
};

template <CellKind C>
CallResult<HermesValue> JSWeakMapImpl<C>::create(
    Runtime *runtime,
    Handle<JSObject> protoHandle) {
  auto propStorage =
      JSObject::createPropStorage(runtime, NEEDED_PROPERTY_SLOTS);
  if (LLVM_UNLIKELY(propStorage == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto valueRes = BigStorage::create(runtime, 1);
  if (LLVM_UNLIKELY(valueRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto valueStorage = runtime->makeHandle<BigStorage>(*valueRes);

  void *mem = runtime->alloc(sizeof(JSWeakMapImpl<C>));
  return HermesValue::encodeObjectValue(new (mem) JSWeakMapImpl<C>(
      runtime,
      *protoHandle,
      runtime->getHiddenClassForPrototypeRaw(*protoHandle),
      **propStorage,
      *valueStorage));
}

template class JSWeakMapImpl<CellKind::WeakMapKind>;
template class JSWeakMapImpl<CellKind::WeakSetKind>;

} // namespace vm
} // namespace hermes
