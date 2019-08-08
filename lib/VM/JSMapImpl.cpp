/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/JSMapImpl.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/Operations.h"

#include "llvm/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSMapImpl

template <CellKind C>
void JSMapImpl<C>::MapOrSetBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSMapImpl<C> *>(cell);
  mb.addField("@storage", &self->storage_);
}

void MapBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSMapImpl<CellKind::MapKind>::MapOrSetBuildMeta(cell, mb);
}

void SetBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSMapImpl<CellKind::SetKind>::MapOrSetBuildMeta(cell, mb);
}

#ifdef HERMESVM_SERIALIZE
template <CellKind C>
void JSMapImpl<C>::serializeMapOrSetImpl(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const JSMapImpl<C>>(cell);
  JSObject::serializeObjectImpl(s, cell);
  s.writeRelocation(self->storage_.get(s.getRuntime()));
}

template <CellKind C>
JSMapImpl<C>::JSMapImpl(Deserializer &d, const VTable *vt) : JSObject(d, vt) {
  d.readRelocation(&storage_, RelocationKind::GCPointer);
}

void MapSerialize(Serializer &s, const GCCell *cell) {
  JSMap::serializeMapOrSetImpl(s, cell);
  s.endObject(cell);
}

void SetSerialize(Serializer &s, const GCCell *cell) {
  JSSet::serializeMapOrSetImpl(s, cell);
  s.endObject(cell);
}

void MapDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::MapKind && "Expected Map");
  void *mem = d.getRuntime()->alloc(sizeof(JSMap));
  auto *cell = new (mem) JSMap(d, &JSMap::vt.base);
  d.endObject(cell);
}

void SetDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::SetKind && "Expected Set");
  void *mem = d.getRuntime()->alloc(sizeof(JSSet));
  auto *cell = new (mem) JSSet(d, &JSSet::vt.base);
  d.endObject(cell);
}
#endif

template <CellKind C>
const ObjectVTable JSMapImpl<C>::vt{
    VTable(C, sizeof(JSMapImpl<C>)),
    JSMapImpl::_getOwnIndexedRangeImpl,
    JSMapImpl::_haveOwnIndexedImpl,
    JSMapImpl::_getOwnIndexedPropertyFlagsImpl,
    JSMapImpl::_getOwnIndexedImpl,
    JSMapImpl::_setOwnIndexedImpl,
    JSMapImpl::_deleteOwnIndexedImpl,
    JSMapImpl::_checkAllOwnIndexedImpl,
};

template <CellKind C>
CallResult<HermesValue> JSMapImpl<C>::create(
    Runtime *runtime,
    Handle<JSObject> parentHandle) {
  void *mem = runtime->alloc(sizeof(JSMapImpl));
  return HermesValue::encodeObjectValue(
      JSObject::allocateSmallPropStorage<NEEDED_PROPERTY_SLOTS>(
          new (mem) JSMapImpl(
              runtime,
              *parentHandle,
              runtime->getHiddenClassForPrototypeRaw(*parentHandle))));
}

template class JSMapImpl<CellKind::SetKind>;
template class JSMapImpl<CellKind::MapKind>;

//===----------------------------------------------------------------------===//
// class JSSetIterator

template <CellKind C>
void JSMapIteratorImpl<C>::MapOrSetIteratorBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSMapIteratorImpl<C> *>(cell);
  mb.addField("@data", &self->data_);
  mb.addField("@itr", &self->itr_);
}

void MapIteratorBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSMapIteratorImpl<CellKind::MapIteratorKind>::MapOrSetIteratorBuildMeta(
      cell, mb);
}

void SetIteratorBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSMapIteratorImpl<CellKind::SetIteratorKind>::MapOrSetIteratorBuildMeta(
      cell, mb);
}

#ifdef HERMESVM_SERIALIZE
void MapIteratorSerialize(Serializer &s, const GCCell *cell) {
  LLVM_DEBUG(
      llvm::dbgs() << "Serialize function not implemented for MapIterator\n");
}

void SetIteratorSerialize(Serializer &s, const GCCell *cell) {
  LLVM_DEBUG(
      llvm::dbgs() << "Serialize function not implemented for SetIterator\n");
}

void MapIteratorDeserialize(Deserializer &d, CellKind kind) {
  LLVM_DEBUG(
      llvm::dbgs() << "Deserialize function not implemented for MapIterator\n");
}

void SetIteratorDeserialize(Deserializer &d, CellKind kind) {
  LLVM_DEBUG(
      llvm::dbgs() << "Deserialize function not implemented for SetIterator\n");
}
#endif

template <CellKind C>
const ObjectVTable JSMapIteratorImpl<C>::vt = {
    VTable(C, sizeof(JSMapIteratorImpl<C>)),
    JSMapIteratorImpl::_getOwnIndexedRangeImpl,
    JSMapIteratorImpl::_haveOwnIndexedImpl,
    JSMapIteratorImpl::_getOwnIndexedPropertyFlagsImpl,
    JSMapIteratorImpl::_getOwnIndexedImpl,
    JSMapIteratorImpl::_setOwnIndexedImpl,
    JSMapIteratorImpl::_deleteOwnIndexedImpl,
    JSMapIteratorImpl::_checkAllOwnIndexedImpl,
};

template <CellKind C>
CallResult<HermesValue> JSMapIteratorImpl<C>::create(
    Runtime *runtime,
    Handle<JSObject> prototype) {
  void *mem = runtime->alloc(sizeof(JSMapIteratorImpl<C>));
  return HermesValue::encodeObjectValue(
      JSObject::allocateSmallPropStorage<NEEDED_PROPERTY_SLOTS>(
          new (mem) JSMapIteratorImpl<C>(
              runtime,
              *prototype,
              runtime->getHiddenClassForPrototypeRaw(*prototype))));
}

template class JSMapIteratorImpl<CellKind::MapIteratorKind>;
template class JSMapIteratorImpl<CellKind::SetIteratorKind>;

} // namespace vm
} // namespace hermes
