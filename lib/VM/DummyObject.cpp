/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/DummyObject.h"

#include "hermes/VM/Deserializer.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HermesValue-inline.h"
#include "hermes/VM/Serializer.h"

namespace hermes {
namespace vm {
namespace testhelpers {

const VTable DummyObject::vt{
    CellKind::DummyObjectKind,
    cellSize<DummyObject>(),
    _finalizeImpl,
    _markWeakImpl,
    _mallocSizeImpl,
    nullptr,
    _externalMemorySizeImpl};

DummyObject::DummyObject(GC *gc)
    : GCCell(gc, &vt), other(), x(1), y(2), weak(gc, this) {
  hvBool.setNonPtr(HermesValue::encodeBoolValue(true), gc);
  hvDouble.setNonPtr(HermesValue::encodeNumberValue(3.14), gc);
  hvNative.setNonPtr(HermesValue::encodeNativeUInt32(0xE), gc);
  hvUndefined.setNonPtr(HermesValue::encodeUndefinedValue(), gc);
  hvEmpty.setNonPtr(HermesValue::encodeEmptyValue(), gc);
  hvNull.setNonPtr(HermesValue::encodeNullValue(), gc);
}

void DummyObject::acquireExtMem(GC *gc, uint32_t sz) {
  assert(externalBytes == 0);
  externalBytes = sz;
  gc->creditExternalMemory(this, sz);
}
void DummyObject::releaseExtMem(GC *gc) {
  gc->debitExternalMemory(this, externalBytes);
  externalBytes = 0;
}

void DummyObject::setPointer(GC *gc, DummyObject *obj) {
  other.set(gc->getPointerBase(), obj, gc);
}

DummyObject *DummyObject::create(GC *gc) {
  return gc->makeAFixed<DummyObject, HasFinalizer::Yes>(gc);
}
DummyObject *DummyObject::createLongLived(GC *gc) {
  return gc->makeAFixed<DummyObject, HasFinalizer::Yes, LongLived::Yes>(gc);
}

bool DummyObject::classof(const GCCell *cell) {
  return cell->getKind() == CellKind::DummyObjectKind;
}

void DummyObject::_finalizeImpl(GCCell *cell, GC *gc) {
  auto *self = vmcast<DummyObject>(cell);
  if (self->finalizerCallback)
    (*self->finalizerCallback)();
  self->releaseExtMem(gc);
  self->~DummyObject();
}

gcheapsize_t DummyObject::_externalMemorySizeImpl(const GCCell *cell) {
  return vmcast<DummyObject>(cell)->externalBytes;
}

size_t DummyObject::_mallocSizeImpl(GCCell *cell) {
  return vmcast<DummyObject>(cell)->extraBytes;
}

void DummyObject::_markWeakImpl(GCCell *cell, WeakRefAcceptor &acceptor) {
  auto *self = reinterpret_cast<DummyObject *>(cell);
  if (self->markWeakCallback)
    (*self->markWeakCallback)(cell, acceptor);
  acceptor.accept(self->weak);
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

#ifdef HERMESVM_SERIALIZE
void DummyObjectSerialize(Serializer &s, const GCCell *cell) {
  llvm_unreachable("DummyObject cannot be serialized.");
}

void DummyObjectDeserialize(Deserializer &d, CellKind kind) {
  llvm_unreachable("DummyObject cannot be deserialized.");
}
#endif

} // namespace vm
} // namespace hermes
