/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSWeakMapImpl.h"

#include "hermes/VM/Casting.h"
#include "hermes/VM/Runtime-inline.h"

namespace hermes {
namespace vm {

void JSWeakMapImplBase::WeakMapImplBaseBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSWeakMapImplBase>());
  JSObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSWeakMapImplBase *>(cell);
  mb.addField("valueStorage", &self->valueStorage_);
}

/// Set a key/value, overwriting the previous value at that key,
/// or add a new key/value if the key doesn't exist.
ExecutionStatus JSWeakMapImplBase::setValue(
    Handle<JSWeakMapImplBase> self,
    Runtime &runtime,
    Handle<JSObject> key,
    Handle<> value) {
  {
    // No allocations should occur while a WeakRefKey is live.
    NoAllocScope noAlloc{runtime};
    WeakRefKey mapKey(
        WeakRef<JSObject>{runtime, key}, runtime.gcStableHashHermesValue(key));
    DenseMapT::iterator it = self->map_.find(mapKey);

    if (it != self->map_.end()) {
      // Key already exists, update existing value.
      assert(
          it->second < self->valueStorage_.getNonNull(runtime)->size(runtime) &&
          "invalid index");
      self->valueStorage_.getNonNull(runtime)->set(runtime, it->second, *value);
      return ExecutionStatus::RETURNED;
    }
  }

  // Index in valueStorage_ in which to place the new element.
  auto cr = getFreeValueStorageIndex(self, runtime);
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint32_t i = *cr;

  {
    // No allocations should occur while a WeakRefKey is live.
    NoAllocScope noAlloc{runtime};
    // Holding the WeakRefLock will prevent the weak ref from getting cleared.
    WeakRefLock lk{runtime.getHeap().weakRefMutex()};
    WeakRefKey mapKey(
        WeakRef<JSObject>{runtime, key}, runtime.gcStableHashHermesValue(key));
    auto result = self->map_.try_emplace(mapKey, i);
    (void)result;
    assert(result.second && "unable to add a new value to map");
  }

  self->valueStorage_.getNonNull(runtime)->set(runtime, i, *value);
  return ExecutionStatus::RETURNED;
}

/// Delete a key/value in the map.
/// \return true if the key/value existed and was removed.
bool JSWeakMapImplBase::deleteValue(
    Handle<JSWeakMapImplBase> self,
    Runtime &runtime,
    Handle<JSObject> key) {
  WeakRefLock lk{runtime.getHeap().weakRefMutex()};
  NoAllocScope noAlloc{runtime};
  WeakRefKey mapKey(
      WeakRef<JSObject>{runtime, key}, runtime.gcStableHashHermesValue(key));
  DenseMapT::iterator it = self->map_.find(mapKey);
  if (it == self->map_.end()) {
    return false;
  }
  self->deleteInternal(runtime, it);
  return true;
}

// Only during GC.
bool JSWeakMapImplBase::clearEntryDirect(GC &gc, const WeakRefKey &key) {
  assert(gc.calledByGC() && "Should only be used by the GC implementation.");
  DenseMapT::iterator it = map_.find(key);
  if (it == map_.end()) {
    return false;
  }
  it->first.ref.clear();
  valueStorage_.get(gc.getPointerBase())
      ->atRef(gc.getPointerBase(), it->second)
      .setInGC(HermesValue::encodeEmptyValue(), gc);
  return true;
}

GCHermesValue *JSWeakMapImplBase::getValueDirect(
    GC &gc,
    const WeakRefKey &key) {
  assert(gc.calledByGC() && "Should only be used by the GC implementation.");
  DenseMapT::iterator it = map_.find(key);
  if (it == map_.end()) {
    return nullptr;
  }
  return &valueStorage_.get(gc.getPointerBase())
              ->atRef(gc.getPointerBase(), it->second);
}

GCPointerBase &JSWeakMapImplBase::getValueStorageRef(GC &gc) {
  assert(gc.calledByGC() && "Should only be used by the GC implementation.");
  return valueStorage_;
}

/// \return true if the \p key exists in the map.
bool JSWeakMapImplBase::hasValue(
    Handle<JSWeakMapImplBase> self,
    Runtime &runtime,
    Handle<JSObject> key) {
  NoAllocScope noAlloc{runtime};
  WeakRefKey mapKey(
      WeakRef<JSObject>{runtime, key}, runtime.gcStableHashHermesValue(key));
  DenseMapT::iterator it = self->map_.find_as(mapKey);
  return it != self->map_.end();
}

HermesValue JSWeakMapImplBase::getValue(
    Handle<JSWeakMapImplBase> self,
    Runtime &runtime,
    Handle<JSObject> key) {
  NoAllocScope noAlloc{runtime};
  WeakRefKey mapKey(
      WeakRef<JSObject>{runtime, key}, runtime.gcStableHashHermesValue(key));
  DenseMapT::iterator it = self->map_.find(mapKey);
  if (it == self->map_.end()) {
    return HermesValue::encodeUndefinedValue();
  }
  return self->valueStorage_.getNonNull(runtime)->at(runtime, it->second);
}

uint32_t JSWeakMapImplBase::debugFreeSlotsAndGetSize(
    Runtime &runtime,
    JSWeakMapImplBase *self) {
  /// Free up any freeable slots, so the count is more accurate.
  if (self->hasFreeableSlots_) {
    // There are freeable slots: find and delete them.
    self->findAndDeleteFreeSlots(runtime);
  }
  return self->map_.size();
}

JSWeakMapImplBase::KeyIterator JSWeakMapImplBase::keys_begin() {
  return KeyIterator{map_.begin()};
}

JSWeakMapImplBase::KeyIterator JSWeakMapImplBase::keys_end() {
  return KeyIterator{map_.end()};
}

JSObject *detail::WeakRefKey::getObjectInGC(GC &gc) const {
  assert(gc.calledByGC() && "Should only be used by the GC implementation.");
  return ref.getNoBarrierUnsafe(gc.getPointerBase());
}

void JSWeakMapImplBase::_markWeakImpl(GCCell *cell, WeakRefAcceptor &acceptor) {
  auto *self = reinterpret_cast<JSWeakMapImplBase *>(cell);
  for (auto it = self->map_.begin(); it != self->map_.end(); ++it) {
    // We must mark the weak ref regardless of whether the ref is valid here,
    // because JSWeakMapImplBase still has a pointer from map_ into the
    // reference. If we were to skip marking this particular ref, it could be
    // freed before we have a chance to remove the pointer from map_.
    // Then, if the GC runs before we call deleteInternal on the ref,
    // we would attempt to call markWeakRef on a freed ref, which is a violation
    // of the markWeakRef contract.
    acceptor.accept(it->first.ref);
    if (!it->first.ref.isValid()) {
      // Set the hasFreeableSlots_ to indicate that this slot can be
      // cleaned up the next time we add an element to this map.
      self->hasFreeableSlots_ = true;
    }
  }
}

HeapSnapshot::NodeID JSWeakMapImplBase::getMapID(GC &gc) {
  assert(map_.size() && "Shouldn't call getMapID on an empty map");
  GCBase::IDTracker &tracker = gc.getIDTracker();
  const auto id = gc.getObjectID(this);
  auto &nativeIDList = tracker.getExtraNativeIDs(id);
  if (nativeIDList.empty()) {
    nativeIDList.push_back(tracker.nextNativeID());
  }
  return nativeIDList[0];
}

#ifdef HERMES_MEMORY_INSTRUMENTATION
void JSWeakMapImplBase::_snapshotAddEdgesImpl(
    GCCell *cell,
    GC &gc,
    HeapSnapshot &snap) {
  auto *const self = vmcast<JSWeakMapImplBase>(cell);
  JSObject::_snapshotAddEdgesImpl(self, gc, snap);
  if (self->map_.size()) {
    snap.addNamedEdge(
        HeapSnapshot::EdgeType::Internal, "map", self->getMapID(gc));
  }
}

void JSWeakMapImplBase::_snapshotAddNodesImpl(
    GCCell *cell,
    GC &gc,
    HeapSnapshot &snap) {
  auto *const self = vmcast<JSWeakMapImplBase>(cell);
  if (self->map_.size()) {
    snap.beginNode();
    snap.endNode(
        HeapSnapshot::NodeType::Native,
        "DenseMap",
        self->getMapID(gc),
        self->map_.getMemorySize(),
        0);
  }
}
#endif

/// Mark weak references and remove any invalid weak refs.
void JSWeakMapImplBase::findAndDeleteFreeSlots(Runtime &runtime) {
  WeakRefLock lk{runtime.getHeap().weakRefMutex()};
  for (auto it = map_.begin(); it != map_.end(); ++it) {
    if (!it->first.ref.isValid()) {
      // If invalid, clear the value and remove the key from the map.
      deleteInternal(runtime, it);
    }
  }
  hasFreeableSlots_ = false;
}

void JSWeakMapImplBase::deleteInternal(
    Runtime &runtime,
    JSWeakMapImplBase::DenseMapT::iterator it) {
  assert(it != map_.end() && "Invalid iterator to deleteInternal");
  valueStorage_.getNonNull(runtime)->setNonPtr(
      runtime, it->second, HermesValue::encodeNativeUInt32(freeListHead_));
  freeListHead_ = it->second;
  map_.erase(it);
}

CallResult<uint32_t> JSWeakMapImplBase::getFreeValueStorageIndex(
    Handle<JSWeakMapImplBase> self,
    Runtime &runtime) {
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
      return runtime.raiseRangeError("Out of space for elements in map");
    }
    useNextIndex = true;
  } else {
    // Free list has an element, use it.
    i = self->freeListHead_;
    useNextIndex = false;
  }

  auto storageHandle = runtime.makeMutableHandle(self->valueStorage_);
  if (i >= storageHandle->size(runtime)) {
    if (LLVM_UNLIKELY(
            BigStorage::resize(
                storageHandle,
                runtime,
                std::max(i + 1, storageHandle->size(runtime) * 2)) ==
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
    self->freeListHead_ = storageHandle->at(runtime, i).getNativeUInt32();
  }

  assert(i < storageHandle->size(runtime) && "invalid index");
  self->valueStorage_.setNonNull(runtime, *storageHandle, runtime.getHeap());

  return i;
}

template <CellKind C>
void JSWeakMapImpl<C>::WeakMapOrSetBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSWeakMapImpl<C>>());
  WeakMapImplBaseBuildMeta(cell, mb);
}

void JSWeakMapBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSWeakMap::WeakMapOrSetBuildMeta(cell, mb);
  mb.setVTable(&JSWeakMap::vt);
}

void JSWeakSetBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSWeakSet::WeakMapOrSetBuildMeta(cell, mb);
  mb.setVTable(&JSWeakSet::vt);
}

template <CellKind C>
const ObjectVTable JSWeakMapImpl<C>::vt{
    VTable(
        C,
        cellSize<JSWeakMapImpl>(),
        JSWeakMapImpl::_finalizeImpl,
        JSWeakMapImpl::_markWeakImpl,
        JSWeakMapImpl::_mallocSizeImpl,
        nullptr
#ifdef HERMES_MEMORY_INSTRUMENTATION
        ,
        VTable::HeapSnapshotMetadata {
          HeapSnapshot::NodeType::Object, nullptr, _snapshotAddEdgesImpl,
              _snapshotAddNodesImpl, nullptr
        }
#endif
        ),
    JSWeakMapImpl::_getOwnIndexedRangeImpl,
    JSWeakMapImpl::_haveOwnIndexedImpl,
    JSWeakMapImpl::_getOwnIndexedPropertyFlagsImpl,
    JSWeakMapImpl::_getOwnIndexedImpl,
    JSWeakMapImpl::_setOwnIndexedImpl,
    JSWeakMapImpl::_deleteOwnIndexedImpl,
    JSWeakMapImpl::_checkAllOwnIndexedImpl,
};

template <CellKind C>
CallResult<PseudoHandle<JSWeakMapImpl<C>>> JSWeakMapImpl<C>::create(
    Runtime &runtime,
    Handle<JSObject> parentHandle) {
  auto valueRes = BigStorage::create(runtime, 1);
  if (LLVM_UNLIKELY(valueRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto valueStorage = runtime.makeHandle<BigStorage>(std::move(*valueRes));

  auto *cell = runtime.makeAFixed<JSWeakMapImpl<C>, HasFinalizer::Yes>(
      runtime,
      parentHandle,
      runtime.getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSWeakMapImpl>()),
      valueStorage);
  return JSObjectInit::initToPseudoHandle(runtime, cell);
}

template class JSWeakMapImpl<CellKind::JSWeakMapKind>;
template class JSWeakMapImpl<CellKind::JSWeakSetKind>;

} // namespace vm
} // namespace hermes
