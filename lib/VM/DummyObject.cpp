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
    cellSize<DummyObject>()};

DummyObject::DummyObject(GC *gc) : GCCell(gc, &vt), other(), x(1), y(2) {
  hvBool.setNonPtr(HermesValue::encodeBoolValue(true), gc);
  hvDouble.setNonPtr(HermesValue::encodeNumberValue(3.14), gc);
  hvNative.setNonPtr(HermesValue::encodeNativeUInt32(0xE), gc);
  hvUndefined.setNonPtr(HermesValue::encodeUndefinedValue(), gc);
  hvEmpty.setNonPtr(HermesValue::encodeEmptyValue(), gc);
  hvNull.setNonPtr(HermesValue::encodeNullValue(), gc);
}

void DummyObject::setPointer(GC *gc, DummyObject *obj) {
  other.set(gc->getPointerBase(), obj, gc);
}

DummyObject *DummyObject::create(GC *gc) {
  return gc->makeAFixed<DummyObject>(gc);
}

bool DummyObject::classof(const GCCell *cell) {
  return cell->getKind() == CellKind::DummyObjectKind;
}

} // namespace testhelpers

void DummyObjectBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const testhelpers::DummyObject *>(cell);
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
