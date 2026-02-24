/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSMapImpl.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/Operations.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSMapImpl

void JSMapBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSMap::buildMetadata(cell, mb);
  mb.addJSObjectOverlapSlots(
      JSObject::numOverlapSlots<JSMapImpl<CellKind::JSMapKind>>());
  JSObjectBuildMeta(cell, mb);
  mb.setVTable(&JSMap::vt);
}

void JSSetBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSSet::buildMetadata(cell, mb);
  mb.addJSObjectOverlapSlots(
      JSObject::numOverlapSlots<JSMapImpl<CellKind::JSSetKind>>());
  JSObjectBuildMeta(cell, mb);
  mb.setVTable(&JSSet::vt);
}

template <CellKind C>
const ObjectVTable JSMapImpl<C>::vt{
    VTable(
        C,
        cellSize<JSMapImpl<C>>(),
        false /* allowLargeAlloc */,
        JSMapImpl<C>::_finalizeImpl),
    JSMapImpl::_getOwnIndexedRangeImpl,
    JSMapImpl::_haveOwnIndexedImpl,
    JSMapImpl::_getOwnIndexedPropertyFlagsImpl,
    JSMapImpl::_getOwnIndexedImpl,
    JSMapImpl::_setOwnIndexedImpl,
    JSMapImpl::_deleteOwnIndexedImpl,
    JSMapImpl::_checkAllOwnIndexedImpl,
};

template <CellKind C>
void JSMapImpl<C>::_finalizeImpl(GCCell *cell, GC &gc) {
  auto *self = vmcast<JSMapImpl<C>>(cell);
  self->cleanUp(cell, gc);
  self->~JSMapImpl<C>();
}

template <CellKind C>
PseudoHandle<JSMapImpl<C>> JSMapImpl<C>::create(
    Runtime &runtime,
    Handle<JSObject> parentHandle) {
  auto *cell = runtime.makeAFixed<JSMapImpl, HasFinalizer::Yes>(
      runtime,
      parentHandle,
      runtime.getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSMapImpl>()));
  return JSObjectInit::initToPseudoHandle(runtime, cell);
}

template class JSMapImpl<CellKind::JSSetKind>;
template class JSMapImpl<CellKind::JSMapKind>;

//===----------------------------------------------------------------------===//
// class JSSetIterator

template <CellKind C>
void JSMapIteratorImpl<C>::MapOrSetIteratorBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSMapIteratorImpl<C>>());
  JSObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSMapIteratorImpl<C> *>(cell);
  mb.addField("data", &self->data_);
}

void JSMapIteratorBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSMapIterator::MapOrSetIteratorBuildMeta(cell, mb);
  mb.setVTable(&JSMapIterator::vt);
}

void JSSetIteratorBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  JSSetIterator::MapOrSetIteratorBuildMeta(cell, mb);
  mb.setVTable(&JSSetIterator::vt);
}

template <CellKind C>
const ObjectVTable JSMapIteratorImpl<C>::vt = {
    VTable(
        C,
        cellSize<JSMapIteratorImpl<C>>(),
        false /* allowLargeAlloc */,
        JSMapIteratorImpl<C>::_finalizeImpl),
    JSMapIteratorImpl::_getOwnIndexedRangeImpl,
    JSMapIteratorImpl::_haveOwnIndexedImpl,
    JSMapIteratorImpl::_getOwnIndexedPropertyFlagsImpl,
    JSMapIteratorImpl::_getOwnIndexedImpl,
    JSMapIteratorImpl::_setOwnIndexedImpl,
    JSMapIteratorImpl::_deleteOwnIndexedImpl,
    JSMapIteratorImpl::_checkAllOwnIndexedImpl,
};

template <CellKind C>
void JSMapIteratorImpl<C>::_finalizeImpl(GCCell *cell, GC &gc) {
  auto *self = vmcast<JSMapIteratorImpl<C>>(cell);
  self->~JSMapIteratorImpl();
}

template <CellKind C>
PseudoHandle<JSMapIteratorImpl<C>> JSMapIteratorImpl<C>::create(
    Runtime &runtime,
    Handle<JSObject> prototype) {
  auto *cell = runtime.makeAFixed<JSMapIteratorImpl<C>, HasFinalizer::Yes>(
      runtime,
      prototype,
      runtime.getHiddenClassForPrototype(
          *prototype, numOverlapSlots<JSMapIteratorImpl>()));
  return JSObjectInit::initToPseudoHandle(runtime, cell);
}

template class JSMapIteratorImpl<CellKind::JSMapIteratorKind>;
template class JSMapIteratorImpl<CellKind::JSSetIteratorKind>;

} // namespace vm
} // namespace hermes
