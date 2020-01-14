/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSWeakMapImpl.h"

#include "hermes/VM/Casting.h"
#include "hermes/VM/WeakRefHolder.h"

#include "llvm/Support/Debug.h"
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

// Only during GC.
bool JSWeakMapImplBase::clearEntryDirect(GC *gc, const WeakRefKey &key) {
  assert(gc->inGC() && "Should only be used by the GC implementation.");
  DenseMapT::iterator it = map_.find(key);
  if (it == map_.end()) {
    return false;
  }
  it->first.ref.clear();
  valueStorage_.get(gc->getPointerBase())
      ->at(it->second)
      .setNonPtr(HermesValue::encodeEmptyValue());
  return true;
}

GCHermesValue *JSWeakMapImplBase::getValueDirect(
    GC *gc,
    const WeakRefKey &key) {
  assert(gc->inGC() && "Should only be used by the GC implementation.");
  DenseMapT::iterator it = map_.find(key);
  if (it == map_.end()) {
    return nullptr;
  }
  return &valueStorage_.get(gc->getPointerBase())->at(it->second);
}

GCPointerBase::StorageType &JSWeakMapImplBase::getValueStorageRef(GC *gc) {
  assert(gc->inGC() && "Should only be used by the GC implementation.");
  return valueStorage_.getLoc(gc);
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

uint32_t JSWeakMapImplBase::debugFreeSlotsAndGetSize(
    PointerBase *base,
    JSWeakMapImplBase *self) {
  /// Free up any freeable slots, so the count is more accurate.
  if (self->hasFreeableSlots_) {
    // There are freeable slots: find and delete them.
    self->findAndDeleteFreeSlots(base);
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
  assert(gc->inGC() && "Should only be used by the GC implementation.");
  return vmcast_or_null<JSObject>(
      reinterpret_cast<GCCell *>(ref.unsafeGetSlot()->getPointer()));
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
  valueStorage_.getNonNull(base)
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
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSWeakMapImpl<C>>());
  WeakMapImplBaseBuildMeta(cell, mb);
}

void WeakMapBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSWeakMapImpl<CellKind::WeakMapKind>::WeakMapOrSetBuildMeta(cell, mb);
}

void WeakSetBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSWeakMapImpl<CellKind::WeakSetKind>::WeakMapOrSetBuildMeta(cell, mb);
}

#ifdef HERMESVM_SERIALIZE
void serializeJSWeakMapBase(Serializer &s, const GCCell *cell) {
  JSObject::serializeObjectImpl(
      s, cell, JSObject::numOverlapSlots<JSWeakMapImplBase>());
  auto *self = vmcast<const JSWeakMapImplBase>(cell);
  // Serialize llvm::DenseMap<WeakRefKey, uint32_t, detail::WeakRefInfo> map_.
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
  void *mem = d.getRuntime()->alloc</*fixedSize*/ true, HasFinalizer::Yes>(
      cellSize<JSWeakMap>());
  auto *cell = new (mem) JSWeakMap(d);
  d.endObject(cell);
}

void WeakSetDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::WeakSetKind && "Expected WeakSet.");
  void *mem = d.getRuntime()->alloc</*fixedSize*/ true, HasFinalizer::Yes>(
      cellSize<JSWeakSet>());
  auto *cell = new (mem) JSWeakSet(d);
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

  JSObjectAlloc<JSWeakMapImpl<C>, HasFinalizer::Yes> mem{runtime};
  return mem.initToHermesValue(new (mem) JSWeakMapImpl<C>(
      runtime,
      *parentHandle,
      runtime->getHiddenClassForPrototypeRaw(
          *parentHandle,
          numOverlapSlots<JSWeakMapImpl>() + ANONYMOUS_PROPERTY_SLOTS),
      *valueStorage));
}

template class JSWeakMapImpl<CellKind::WeakMapKind>;
template class JSWeakMapImpl<CellKind::WeakSetKind>;

} // namespace vm
} // namespace hermes
