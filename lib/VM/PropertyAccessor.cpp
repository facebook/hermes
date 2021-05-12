/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/PropertyAccessor.h"

namespace hermes {
namespace vm {

const VTable PropertyAccessor::vt{
    CellKind::PropertyAccessorKind,
    cellSize<PropertyAccessor>()};

void PropertyAccessorBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const PropertyAccessor *>(cell);
  mb.setVTable(&PropertyAccessor::vt);
  mb.addField("getter", &self->getter);
  mb.addField("setter", &self->setter);
}

#ifdef HERMESVM_SERIALIZE
PropertyAccessor::PropertyAccessor(Deserializer &d)
    : GCCell(&d.getRuntime()->getHeap(), &vt) {
  d.readRelocation(&getter, RelocationKind::GCPointer);
  d.readRelocation(&setter, RelocationKind::GCPointer);
}

void PropertyAccessorSerialize(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const PropertyAccessor>(cell);
  s.writeRelocation(self->getter.get(s.getRuntime()));
  s.writeRelocation(self->setter.get(s.getRuntime()));
  s.endObject(cell);
}

void PropertyAccessorDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::PropertyAccessorKind && "Expected PropertyAccessor");
  auto *cell = d.getRuntime()->makeAFixed<PropertyAccessor>(d);
  d.endObject(cell);
}
#endif

CallResult<HermesValue> PropertyAccessor::create(
    Runtime *runtime,
    Handle<Callable> getter,
    Handle<Callable> setter) {
  auto *cell = runtime->makeAFixed<PropertyAccessor>(runtime, getter, setter);
  return HermesValue::encodeObjectValue(cell);
}

} // namespace vm
} // namespace hermes
