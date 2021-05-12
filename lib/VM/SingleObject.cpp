/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/SingleObject.h"
#include "hermes/VM/BuildMetadata.h"

namespace hermes {
namespace vm {
void MathBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(
      JSObject::numOverlapSlots<SingleObject<CellKind::MathKind>>());
  ObjectBuildMeta(cell, mb);
  mb.setVTable(&JSMath::vt.base);
}

void JSONBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(
      JSObject::numOverlapSlots<SingleObject<CellKind::JSONKind>>());
  ObjectBuildMeta(cell, mb);
  mb.setVTable(&JSJSON::vt.base);
}

#ifdef HERMESVM_SERIALIZE
template <CellKind kind>
SingleObject<kind>::SingleObject(Deserializer &d, const VTable *vt)
    : JSObject(d, vt) {}

void MathSerialize(Serializer &s, const GCCell *cell) {
  JSObject::serializeObjectImpl(
      s, cell, JSObject::numOverlapSlots<SingleObject<CellKind::MathKind>>());
  s.endObject(cell);
}

void JSONSerialize(Serializer &s, const GCCell *cell) {
  JSObject::serializeObjectImpl(
      s, cell, JSObject::numOverlapSlots<SingleObject<CellKind::JSONKind>>());
  s.endObject(cell);
}

void JSONDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::JSONKind && "Expected JSON");
  auto *cell = d.getRuntime()->makeAFixed<JSJSON>(d, &JSJSON::vt.base);
  d.endObject(cell);
}

void MathDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::MathKind && "Expected Math");
  auto *cell = d.getRuntime()->makeAFixed<JSMath>(d, &JSMath::vt.base);
  d.endObject(cell);
}
#endif

} // namespace vm
} // namespace hermes
