/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/DummyObject.h"

#include "hermes/VM/GC.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HermesValue-inline.h"

namespace hermes {
namespace vm {
namespace testhelpers {

const VTable DummyObject::vt{
    CellKind::DummyObjectKind,
    cellSize<DummyObject>(),
    _finalizeImpl,
    _markWeakImpl,
    _mallocSizeImpl,
    nullptr};

DummyObject::DummyObject(GC &gc) : other(), x(1), y(2) {
  hvBool.setNonPtr(HermesValue::encodeBoolValue(true), gc);
  hvDouble.setNonPtr(HermesValue::encodeNumberValue(3.14), gc);
  hvNative.setNonPtr(HermesValue::encodeNativeUInt32(0xE), gc);
  hvUndefined.setNonPtr(HermesValue::encodeUndefinedValue(), gc);
  hvEmpty.setNonPtr(HermesValue::encodeEmptyValue(), gc);
  hvNull.setNonPtr(HermesValue::encodeNullValue(), gc);
}

void DummyObject::acquireExtMem(GC &gc, uint32_t sz) {
  assert(externalBytes == 0);
  externalBytes = sz;
  gc.creditExternalMemory(this, sz);
}
void DummyObject::releaseExtMem(GC &gc) {
  gc.debitExternalMemory(this, externalBytes);
  externalBytes = 0;
}

void DummyObject::setPointer(GC &gc, DummyObject *obj) {
  other.set(gc.getPointerBase(), obj, gc);
}

/* static */ constexpr CellKind DummyObject::getCellKind() {
  return CellKind::DummyObjectKind;
}

DummyObject *DummyObject::create(GC &gc, PointerBase &base) {
  auto *cell = gc.makeAFixed<DummyObject, HasFinalizer::Yes>(gc);
  cell->weak.emplace(base, gc, cell);
  return cell;
}
DummyObject *DummyObject::createLongLived(GC &gc) {
  return gc.makeAFixed<DummyObject, HasFinalizer::Yes, LongLived::Yes>(gc);
}

bool DummyObject::classof(const GCCell *cell) {
  return cell->getKind() == CellKind::DummyObjectKind;
}

void DummyObject::_finalizeImpl(GCCell *cell, GC &gc) {
  auto *self = vmcast<DummyObject>(cell);
  if (self->finalizerCallback)
    (*self->finalizerCallback)();
  self->releaseExtMem(gc);
  self->~DummyObject();
}

size_t DummyObject::_mallocSizeImpl(GCCell *cell) {
  return vmcast<DummyObject>(cell)->extraBytes;
}

void DummyObject::_markWeakImpl(GCCell *cell, WeakRefAcceptor &acceptor) {
  auto *self = reinterpret_cast<DummyObject *>(cell);
  if (self->markWeakCallback)
    (*self->markWeakCallback)(cell, acceptor);
  if (self->weak)
    acceptor.accept(*self->weak);
}

} // namespace testhelpers

void DummyObjectBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const testhelpers::DummyObject *>(cell);
  mb.setVTable(&testhelpers::DummyObject::vt);
  mb.addField("HermesBool", &self->hvBool);
  mb.addField("HermesDouble", &self->hvDouble);
  mb.addField("HermesUndefined", &self->hvUndefined);
  mb.addField("HermesEmpty", &self->hvEmpty);
  mb.addField("HermesNative", &self->hvNative);
  mb.addField("HermesNull", &self->hvNull);
  mb.addField("other", &self->other);
}

} // namespace vm
} // namespace hermes
