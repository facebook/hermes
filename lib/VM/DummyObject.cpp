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
    _mallocSizeImpl,
    nullptr
#ifdef HERMES_MEMORY_INSTRUMENTATION
    ,
    VTable::HeapSnapshotMetadata{
        HeapSnapshot::NodeType::Object,
        nullptr,
        _snapshotAddEdgesImpl,
        nullptr,
        nullptr}
#endif
};

DummyObject::DummyObject(GC &gc) : other(), x(1), y(2) {
  hvBool.setNonPtr(HermesValue::encodeBoolValue(true), gc);
  hvDouble.setNonPtr(HermesValue::encodeUntrustedNumberValue(3.14), gc);
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
  cell->finalizerCallback.set(gc, nullptr);
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
  auto callback = self->finalizerCallback.get(gc);
  if (callback)
    (*callback)();
  if (self->weak)
    self->weak->releaseSlot();
  self->releaseExtMem(gc);

  // Callback is assumed to point to allocated memory
  delete callback;
  self->~DummyObject();
}

size_t DummyObject::_mallocSizeImpl(GCCell *cell) {
  return vmcast<DummyObject>(cell)->extraBytes;
}

#ifdef HERMES_MEMORY_INSTRUMENTATION
void DummyObject::_snapshotAddEdgesImpl(
    GCCell *cell,
    GC &gc,
    HeapSnapshot &snap) {
  auto *const self = vmcast<DummyObject>(cell);
  if (!self->weak)
    return;
  // Filter out empty refs from adding edges.
  if (!self->weak->isValid())
    return;
  // DummyObject has only one WeakRef field.
  snap.addNamedEdge(
      HeapSnapshot::EdgeType::Weak,
      "weak",
      gc.getObjectID(self->weak->getNoBarrierUnsafe(gc.getPointerBase())));
}
#endif

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
