/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSMapImpl.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/Operations.h"

#include "llvh/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSMapImpl

template <CellKind C>
void JSMapImpl<C>::MapOrSetBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSMapImpl<C>>());
  ObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSMapImpl<C> *>(cell);
  mb.addField("storage", &self->storage_);
}

void MapBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSMapImpl<CellKind::MapKind>::MapOrSetBuildMeta(cell, mb);
  mb.setVTable(&JSMap::vt.base);
}

void SetBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSMapImpl<CellKind::SetKind>::MapOrSetBuildMeta(cell, mb);
  mb.setVTable(&JSSet::vt.base);
}

#ifdef HERMESVM_SERIALIZE
template <CellKind C>
void JSMapImpl<C>::serializeMapOrSetImpl(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const JSMapImpl<C>>(cell);
  JSObject::serializeObjectImpl(
      s, cell, JSObject::numOverlapSlots<JSMapImpl<C>>());
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
  auto *cell = d.getRuntime()->makeAFixed<JSMap>(d, &JSMap::vt.base);
  d.endObject(cell);
}

void SetDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::SetKind && "Expected Set");
  auto *cell = d.getRuntime()->makeAFixed<JSSet>(d, &JSSet::vt.base);
  d.endObject(cell);
}
#endif

template <CellKind C>
const ObjectVTable JSMapImpl<C>::vt{
    VTable(C, cellSize<JSMapImpl<C>>()),
    JSMapImpl::_getOwnIndexedRangeImpl,
    JSMapImpl::_haveOwnIndexedImpl,
    JSMapImpl::_getOwnIndexedPropertyFlagsImpl,
    JSMapImpl::_getOwnIndexedImpl,
    JSMapImpl::_setOwnIndexedImpl,
    JSMapImpl::_deleteOwnIndexedImpl,
    JSMapImpl::_checkAllOwnIndexedImpl,
};

template <CellKind C>
PseudoHandle<JSMapImpl<C>> JSMapImpl<C>::create(
    Runtime *runtime,
    Handle<JSObject> parentHandle) {
  auto *cell = runtime->makeAFixed<JSMapImpl>(
      runtime,
      parentHandle,
      runtime->getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSMapImpl>()));
  return JSObjectInit::initToPseudoHandle(runtime, cell);
}

template class JSMapImpl<CellKind::SetKind>;
template class JSMapImpl<CellKind::MapKind>;

//===----------------------------------------------------------------------===//
// class JSSetIterator

template <CellKind C>
void JSMapIteratorImpl<C>::MapOrSetIteratorBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSMapIteratorImpl<C>>());
  ObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSMapIteratorImpl<C> *>(cell);
  mb.addField("data", &self->data_);
  mb.addField("itr", &self->itr_);
}

void MapIteratorBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSMapIteratorImpl<CellKind::MapIteratorKind>::MapOrSetIteratorBuildMeta(
      cell, mb);
  mb.setVTable(&JSMapIterator::vt.base);
}

void SetIteratorBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSMapIteratorImpl<CellKind::SetIteratorKind>::MapOrSetIteratorBuildMeta(
      cell, mb);
  mb.setVTable(&JSSetIterator::vt.base);
}

#ifdef HERMESVM_SERIALIZE
template <CellKind C>
void serializeMapOrSetIteratorImpl(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const JSMapIteratorImpl<C>>(cell);
  JSObject::serializeObjectImpl(
      s, cell, JSObject::numOverlapSlots<JSMapIteratorImpl<C>>());
  s.writeRelocation(self->data_.get(s.getRuntime()));
  s.writeRelocation(self->itr_.get(s.getRuntime()));
  s.writeInt<uint8_t>((uint8_t)self->iterationKind_);
  s.writeInt<uint8_t>(self->iterationFinished_);
}

template <CellKind C>
JSMapIteratorImpl<C>::JSMapIteratorImpl(Deserializer &d)
    : JSObject(d, &vt.base) {
  d.readRelocation(&data_, RelocationKind::GCPointer);
  d.readRelocation(&itr_, RelocationKind::GCPointer);
  iterationKind_ = (IterationKind)d.readInt<uint8_t>();
  iterationFinished_ = d.readInt<uint8_t>();
}

void MapIteratorSerialize(Serializer &s, const GCCell *cell) {
  serializeMapOrSetIteratorImpl<CellKind::MapIteratorKind>(s, cell);
  s.endObject(cell);
}

void SetIteratorSerialize(Serializer &s, const GCCell *cell) {
  serializeMapOrSetIteratorImpl<CellKind::SetIteratorKind>(s, cell);
  s.endObject(cell);
}

void MapIteratorDeserialize(Deserializer &d, CellKind kind) {
  auto *cell = d.getRuntime()->makeAFixed<JSMapIterator>(d);
  d.endObject(cell);
}

void SetIteratorDeserialize(Deserializer &d, CellKind kind) {
  auto *cell = d.getRuntime()->makeAFixed<JSSetIterator>(d);
  d.endObject(cell);
}
#endif

template <CellKind C>
const ObjectVTable JSMapIteratorImpl<C>::vt = {
    VTable(C, cellSize<JSMapIteratorImpl<C>>()),
    JSMapIteratorImpl::_getOwnIndexedRangeImpl,
    JSMapIteratorImpl::_haveOwnIndexedImpl,
    JSMapIteratorImpl::_getOwnIndexedPropertyFlagsImpl,
    JSMapIteratorImpl::_getOwnIndexedImpl,
    JSMapIteratorImpl::_setOwnIndexedImpl,
    JSMapIteratorImpl::_deleteOwnIndexedImpl,
    JSMapIteratorImpl::_checkAllOwnIndexedImpl,
};

template <CellKind C>
PseudoHandle<JSMapIteratorImpl<C>> JSMapIteratorImpl<C>::create(
    Runtime *runtime,
    Handle<JSObject> prototype) {
  auto *cell = runtime->makeAFixed<JSMapIteratorImpl<C>>(
      runtime,
      prototype,
      runtime->getHiddenClassForPrototype(
          *prototype, numOverlapSlots<JSMapIteratorImpl>()));
  return JSObjectInit::initToPseudoHandle(runtime, cell);
}

template class JSMapIteratorImpl<CellKind::MapIteratorKind>;
template class JSMapIteratorImpl<CellKind::SetIteratorKind>;

} // namespace vm
} // namespace hermes

#undef DEBUG_TYPE
