/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSWeakMapImpl.h"

#include "hermes/VM/Casting.h"
#include "hermes/VM/Runtime-inline.h"

#include "llvh/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

void JSWeakMapImplBase::WeakMapImplBaseBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSWeakMapImplBase>());
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
  {
    // No allocations should occur while a WeakRefKey is live.
    NoAllocScope noAlloc{runtime};
    WeakRefKey mapKey(
        WeakRef<JSObject>{&runtime->getHeap(), key},
        runtime->gcStableHashHermesValue(key));
    DenseMapT::iterator it = self->map_.find(mapKey);

    if (it != self->map_.end()) {
      // Key already exists, update existing value.
      assert(
          it->second < self->valueStorage_.get(runtime)->size() &&
          "invalid index");
      self->valueStorage_.get(runtime)->set(
          it->second, *value, &runtime->getHeap());
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
    WeakRefLock lk{runtime->getHeap().weakRefMutex()};
    WeakRefKey mapKey(
        WeakRef<JSObject>{&runtime->getHeap(), key},
        runtime->gcStableHashHermesValue(key));
    auto result = self->map_.try_emplace(mapKey, i);
    (void)result;
    assert(result.second && "unable to add a new value to map");
  }

  self->valueStorage_.get(runtime)->set(i, *value, &runtime->getHeap());
  return ExecutionStatus::RETURNED;
}

/// Delete a key/value in the map.
/// \return true if the key/value existed and was removed.
bool JSWeakMapImplBase::deleteValue(
    Handle<JSWeakMapImplBase> self,
    Runtime *runtime,
    Handle<JSObject> key) {
  WeakRefLock lk{runtime->getHeap().weakRefMutex()};
  NoAllocScope noAlloc{runtime};
  WeakRefKey mapKey(
      WeakRef<JSObject>{&runtime->getHeap(), key},
      runtime->gcStableHashHermesValue(key));
  DenseMapT::iterator it = self->map_.find(mapKey);
  if (it == self->map_.end()) {
    return false;
  }
  self->deleteInternal(runtime, &runtime->getHeap(), it);
  return true;
}

// Only during GC.
bool JSWeakMapImplBase::clearEntryDirect(GC *gc, const WeakRefKey &key) {
  assert(gc->calledByGC() && "Should only be used by the GC implementation.");
  DenseMapT::iterator it = map_.find(key);
  if (it == map_.end()) {
    return false;
  }
  it->first.ref.clear();
  valueStorage_.get(gc->getPointerBase())
      ->atRef(it->second)
      .setInGC(HermesValue::encodeEmptyValue(), gc);
  return true;
}

GCHermesValue *JSWeakMapImplBase::getValueDirect(
    GC *gc,
    const WeakRefKey &key) {
  assert(gc->calledByGC() && "Should only be used by the GC implementation.");
  DenseMapT::iterator it = map_.find(key);
  if (it == map_.end()) {
    return nullptr;
  }
  return &valueStorage_.get(gc->getPointerBase())->atRef(it->second);
}

GCPointerBase::StorageType &JSWeakMapImplBase::getValueStorageRef(GC *gc) {
  assert(gc->calledByGC() && "Should only be used by the GC implementation.");
  return valueStorage_.getLoc();
}

/// \return true if the \p key exists in the map.
bool JSWeakMapImplBase::hasValue(
    Handle<JSWeakMapImplBase> self,
    Runtime *runtime,
    Handle<JSObject> key) {
  NoAllocScope noAlloc{runtime};
  WeakRefKey mapKey(
      WeakRef<JSObject>{&runtime->getHeap(), key},
      runtime->gcStableHashHermesValue(key));
  DenseMapT::iterator it = self->map_.find_as(mapKey);
  return it != self->map_.end();
}

HermesValue JSWeakMapImplBase::getValue(
    Handle<JSWeakMapImplBase> self,
    Runtime *runtime,
    Handle<JSObject> key) {
  NoAllocScope noAlloc{runtime};
  WeakRefKey mapKey(
      WeakRef<JSObject>{&runtime->getHeap(), key},
      runtime->gcStableHashHermesValue(key));
  DenseMapT::iterator it = self->map_.find(mapKey);
  if (it == self->map_.end()) {
    return HermesValue::encodeUndefinedValue();
  }
  return self->valueStorage_.get(runtime)->at(it->second);
}

uint32_t JSWeakMapImplBase::debugFreeSlotsAndGetSize(
    PointerBase *base,
    GC *gc,
    JSWeakMapImplBase *self) {
  /// Free up any freeable slots, so the count is more accurate.
  if (self->hasFreeableSlots_) {
    // There are freeable slots: find and delete them.
    self->findAndDeleteFreeSlots(base, gc);
  }
  return self->map_.size();
}

JSWeakMapImplBase::KeyIterator JSWeakMapImplBase::keys_begin() {
  return KeyIterator{map_.begin()};
}

JSWeakMapImplBase::KeyIterator JSWeakMapImplBase::keys_end() {
  return KeyIterator{map_.end()};
}

JSObject *detail::WeakRefKey::getObject(GC *gc) const {
  assert(gc->calledByGC() && "Should only be used by the GC implementation.");
  return getNoHandle(ref, gc);
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

HeapSnapshot::NodeID JSWeakMapImplBase::getMapID(GC *gc) {
  assert(map_.size() && "Shouldn't call getMapID on an empty map");
  GCBase::IDTracker &tracker = gc->getIDTracker();
  const auto id = gc->getObjectID(this);
  auto &nativeIDList = tracker.getExtraNativeIDs(id);
  if (nativeIDList.empty()) {
    nativeIDList.push_back(tracker.nextNativeID());
  }
  return nativeIDList[0];
}

void JSWeakMapImplBase::_snapshotAddEdgesImpl(
    GCCell *cell,
    GC *gc,
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
    GC *gc,
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

/// Mark weak references and remove any invalid weak refs.
void JSWeakMapImplBase::findAndDeleteFreeSlots(PointerBase *base, GC *gc) {
  WeakRefLock lk{gc->weakRefMutex()};
  for (auto it = map_.begin(); it != map_.end(); ++it) {
    if (!it->first.ref.isValid()) {
      // If invalid, clear the value and remove the key from the map.
      deleteInternal(base, gc, it);
    }
  }
  hasFreeableSlots_ = false;
}

void JSWeakMapImplBase::deleteInternal(
    PointerBase *base,
    GC *gc,
    JSWeakMapImplBase::DenseMapT::iterator it) {
  assert(it != map_.end() && "Invalid iterator to deleteInternal");
  valueStorage_.getNonNull(base)->setNonPtr(
      it->second, HermesValue::encodeNativeUInt32(freeListHead_), gc);
  freeListHead_ = it->second;
  map_.erase(it);
}

CallResult<uint32_t> JSWeakMapImplBase::getFreeValueStorageIndex(
    Handle<JSWeakMapImplBase> self,
    Runtime *runtime) {
  if (self->freeListHead_ == kFreeListInvalid && self->hasFreeableSlots_) {
    // No elements in the free list and there are freeable slots.
    // Try to find some.
    self->findAndDeleteFreeSlots(runtime, &runtime->getHeap());
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
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSWeakMapImpl<C>>());
  WeakMapImplBaseBuildMeta(cell, mb);
}

void WeakMapBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSWeakMapImpl<CellKind::WeakMapKind>::WeakMapOrSetBuildMeta(cell, mb);
  mb.setVTable(&JSWeakMap::vt.base);
}

void WeakSetBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSWeakMapImpl<CellKind::WeakSetKind>::WeakMapOrSetBuildMeta(cell, mb);
  mb.setVTable(&JSWeakSet::vt.base);
}

#ifdef HERMESVM_SERIALIZE
void serializeJSWeakMapBase(Serializer &s, const GCCell *cell) {
  JSObject::serializeObjectImpl(
      s, cell, JSObject::numOverlapSlots<JSWeakMapImplBase>());
  auto *self = vmcast<const JSWeakMapImplBase>(cell);
  // Serialize llvh::DenseMap<WeakRefKey, uint32_t, detail::WeakRefInfo> map_.
  // We write all entries in Densemap. It's OK for us to serialize/deserialize
  // everything now because we serialize/deserialize every WeakRef too.
  // TODO: Ideally, we would want to compact the map and delete invalid entries
  // by calling findAndDeleteFreeSlots, but we can't do that now because \p cell
  // is \p const and also because compact the map would also change \p
  // BigStorage. See if there is a way to add an additional pass before
  // Serialize to compact all cells.
  s.writeInt<unsigned>(self->map_.size());
  for (auto it = self->map_.begin(); it != self->map_.end(); it++) {
    // Write it->first: WeakRefKey.
    s.writeRelocation(it->first.ref.unsafeGetSlot());
    s.writeInt<uint32_t>(it->first.hash);
    // Write it->second: uint32_t
    s.writeInt<uint32_t>(it->second);
  }

  // Serialize other fields.
  s.writeRelocation(self->valueStorage_.get(s.getRuntime()));
  s.writeInt<uint32_t>(self->freeListHead_);
  s.writeInt<uint32_t>(self->nextIndex_);
  s.writeInt<uint8_t>(self->hasFreeableSlots_);
}

JSWeakMapImplBase::JSWeakMapImplBase(Deserializer &d, const VTable *vt)
    : JSObject(d, vt) {
  unsigned size = d.readInt<unsigned>();
  for (unsigned i = 0; i < size; i++) {
    WeakRefSlot *slotPtr =
        (WeakRefSlot *)d.ptrRelocationOrNull(d.readInt<uint32_t>());
    assert(slotPtr && "WeakRef should have been deserialized.");
    // Note: it's ok to use \p key as DenseMap key because relocation has
    // finished for \p ref.
    WeakRefKey key(WeakRef<JSObject>(slotPtr), d.readInt<uint32_t>());
    auto res = map_.try_emplace(key, d.readInt<uint32_t>()).second;
    if (LLVM_UNLIKELY(!res)) {
      hermes_fatal("shouldn't fail to insert during deserialization");
    }
  }

  // Deserialize other fields.
  d.readRelocation(&valueStorage_, RelocationKind::GCPointer);
  freeListHead_ = d.readInt<uint32_t>();
  nextIndex_ = d.readInt<uint32_t>();
  hasFreeableSlots_ = d.readInt<uint8_t>();
}

void WeakMapSerialize(Serializer &s, const GCCell *cell) {
  serializeJSWeakMapBase(s, cell);
  s.endObject(cell);
}

void WeakSetSerialize(Serializer &s, const GCCell *cell) {
  serializeJSWeakMapBase(s, cell);
  s.endObject(cell);
}

template <CellKind C>
JSWeakMapImpl<C>::JSWeakMapImpl(Deserializer &d)
    : JSWeakMapImplBase(d, &vt.base) {}

void WeakMapDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::WeakMapKind && "Expected WeakMap.");
  auto *cell = d.getRuntime()->makeAFixed<JSWeakMap, HasFinalizer::Yes>(d);
  d.endObject(cell);
}

void WeakSetDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::WeakSetKind && "Expected WeakSet.");
  auto *cell = d.getRuntime()->makeAFixed<JSWeakSet, HasFinalizer::Yes>(d);
  d.endObject(cell);
}
#endif

template <CellKind C>
const ObjectVTable JSWeakMapImpl<C>::vt{
    VTable(
        C,
        cellSize<JSWeakMapImpl>(),
        JSWeakMapImpl::_finalizeImpl,
        JSWeakMapImpl::_markWeakImpl,
        JSWeakMapImpl::_mallocSizeImpl,
        nullptr,
        nullptr,
        VTable::HeapSnapshotMetadata{
            HeapSnapshot::NodeType::Object,
            nullptr,
            _snapshotAddEdgesImpl,
            _snapshotAddNodesImpl,
            nullptr}),
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
    Runtime *runtime,
    Handle<JSObject> parentHandle) {
  auto valueRes = BigStorage::create(runtime, 1);
  if (LLVM_UNLIKELY(valueRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto valueStorage = runtime->makeHandle<BigStorage>(std::move(*valueRes));

  auto *cell = runtime->makeAFixed<JSWeakMapImpl<C>, HasFinalizer::Yes>(
      runtime,
      parentHandle,
      runtime->getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSWeakMapImpl>()),
      valueStorage);
  return JSObjectInit::initToPseudoHandle(runtime, cell);
}

template class JSWeakMapImpl<CellKind::WeakMapKind>;
template class JSWeakMapImpl<CellKind::WeakSetKind>;

} // namespace vm
} // namespace hermes

#undef DEBUG_TYPE
