/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/BoxedDouble.h"
#include "hermes/VM/Deserializer.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/Serializer.h"

namespace hermes {
namespace vm {

const VTable BoxedDouble::vt{
    CellKind::BoxedDoubleKind,
    cellSize<BoxedDouble>()};
void BoxedDoubleBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.setVTable(&BoxedDouble::vt);
}

#ifdef HERMESVM_SERIALIZE
void BoxedDoubleSerialize(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const BoxedDouble>(cell);
  s.writeHermesValue(HermesValue::encodeNumberValue(self->value_));
  s.endObject(cell);
}

void BoxedDoubleDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::BoxedDoubleKind && "Expected BoxedDouble");
  auto *cell = d.getRuntime()->makeAFixed<BoxedDouble>(d);
  d.endObject(cell);
}

BoxedDouble::BoxedDouble(Deserializer &d)
    : GCCell(&d.getRuntime()->getHeap(), &BoxedDouble::vt) {
  HermesValue hv;
  d.readHermesValue(&hv);
  value_ = hv.getNumber();
}
#endif

} // namespace vm
} // namespace hermes
