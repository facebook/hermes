/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

CallResult<HermesValue> PropertyAccessor::create(
    Runtime &runtime,
    Handle<Callable> getter,
    Handle<Callable> setter) {
  auto *cell = runtime.makeAFixed<PropertyAccessor>(runtime, getter, setter);
  return HermesValue::encodeObjectValue(cell);
}

} // namespace vm
} // namespace hermes
