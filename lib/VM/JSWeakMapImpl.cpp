/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/JSWeakMapImpl.h"

#include "hermes/VM/WeakRefHolder.h"

#include "llvm/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

void JSWeakMapImplBase::WeakMapImplBaseBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSWeakMapImplBase *>(cell);
  mb.addField("valueStorage", &self->valueStorage_);
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
    assert(
        it->second < self->valueStorage_.get(runtime)->size() &&
        "invalid index");
    self->valueStorage_.get(runtime)
        ->at(it->second)
        .set(*value, &runtime->getHeap());
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

  self->valueStorage_.get(runtime)->at(i).set(*value, &runtime->getHeap());
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
  self->deleteInternal(runtime, it);
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
  return self->valueStorage_.get(runtime)->at(it->second);
}

/// Mark weak references and remove any invalid weak refs.
void JSWeakMapImplBase::markWeakRefs(GC *gc) {
  for (auto it = map_.begin(); it != map_.end(); ++it) {
    // We must mark the weak ref regardless of whether the ref is valid here,
    // because JSWeakMapImplBase still has a pointer from map_ into the
    // reference. If we were to skip marking this particular ref, it could be
    // freed before we have a chance to remove the pointer from map_.
    // Then, if the GC runs before we call deleteInternal on the ref,
    // we would attempt to call markWeakRef on a freed ref, which is a violation
    // of the markWeakRef contract.
    gc->markWeakRef(it->first.ref);
    if (!it->first.ref.isValid()) {
      // Set the hasFreeableSlots_ to indicate that this slot can be
      // cleaned up the next time we add an element to this map.
      hasFreeableSlots_ = true;
    }
  }
}

/// Mark weak references and remove any invalid weak refs.
void JSWeakMapImplBase::findAndDeleteFreeSlots(PointerBase *base) {
  for (auto it = map_.begin(); it != map_.end(); ++it) {
    if (!it->first.ref.isValid()) {
      // If invalid, clear the value and remove the key from the map.
      deleteInternal(base, it);
    }
  }
  hasFreeableSlots_ = false;
}

void JSWeakMapImplBase::deleteInternal(
    PointerBase *base,
    JSWeakMapImplBase::DenseMapT::iterator it) {
  assert(it != map_.end() && "Invalid iterator to deleteInternal");
  valueStorage_.get(base)
      ->at(it->second)
      .setNonPtr(HermesValue::encodeNativeUInt32(freeListHead_));
  freeListHead_ = it->second;
  map_.erase(it);
}

CallResult<uint32_t> JSWeakMapImplBase::getFreeValueStorageIndex(
    Handle<JSWeakMapImplBase> self,
    Runtime *runtime) {
  if (self->freeListHead_ == kFreeListInvalid && self->hasFreeableSlots_) {
    // No elements in the free list and there are freeable slots.
    // Try to find some.
    self->findAndDeleteFreeSlots(runtime);
  }

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
    self->freeListHead_ = storageHandle->at(i).getNativeUInt32();
  }

  assert(i < storageHandle->size() && "invalid index");
  self->valueStorage_.set(runtime, *storageHandle, &runtime->getHeap());

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

#ifdef HERMESVM_SERIALIZE
void WeakMapSerialize(Serializer &s, const GCCell *cell) {
  LLVM_DEBUG(
      llvm::dbgs() << "Serialize function not implemented for WeakMap\n");
}

void WeakSetSerialize(Serializer &s, const GCCell *cell) {
  LLVM_DEBUG(
      llvm::dbgs() << "Serialize function not implemented for WeakSet\n");
}

void WeakMapDeserialize(Deserializer &d, CellKind kind) {
  LLVM_DEBUG(
      llvm::dbgs() << "Deserialize function not implemented for WeakMap\n");
}

void WeakSetDeserialize(Deserializer &d, CellKind kind) {
  LLVM_DEBUG(
      llvm::dbgs() << "Deserialize function not implemented for WeakSet\n");
}
#endif

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
    Handle<JSObject> parentHandle) {
  auto valueRes = BigStorage::create(runtime, 1);
  if (LLVM_UNLIKELY(valueRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto valueStorage = runtime->makeHandle<BigStorage>(*valueRes);

  void *mem = runtime->alloc</*fixedSize*/ true, HasFinalizer::Yes>(
      sizeof(JSWeakMapImpl<C>));
  return HermesValue::encodeObjectValue(
      JSObject::allocateSmallPropStorage<NEEDED_PROPERTY_SLOTS>(
          new (mem) JSWeakMapImpl<C>(
              runtime,
              *parentHandle,
              runtime->getHiddenClassForPrototypeRaw(*parentHandle),
              *valueStorage)));
}

template class JSWeakMapImpl<CellKind::WeakMapKind>;
template class JSWeakMapImpl<CellKind::WeakSetKind>;

} // namespace vm
} // namespace hermes
