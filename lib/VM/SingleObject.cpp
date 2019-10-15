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
  ObjectBuildMeta(cell, mb);
}

void JSONBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
}

#ifdef HERMESVM_SERIALIZE
template <CellKind kind>
SingleObject<kind>::SingleObject(Deserializer &d, const VTable *vt)
    : JSObject(d, vt) {}

void MathSerialize(Serializer &s, const GCCell *cell) {
  JSObject::serializeObjectImpl(s, cell);
  s.endObject(cell);
}

void JSONSerialize(Serializer &s, const GCCell *cell) {
  JSObject::serializeObjectImpl(s, cell);
  s.endObject(cell);
}

void JSONDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::JSONKind && "Expected JSON");
  void *mem = d.getRuntime()->alloc(sizeof(JSJSON));
  auto *cell = new (mem) JSJSON(d, &JSJSON::vt.base);

  d.endObject(cell);
}

void MathDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::MathKind && "Expected Math");
  void *mem = d.getRuntime()->alloc(sizeof(JSMath));
  auto *cell = new (mem) JSMath(d, &JSMath::vt.base);
  d.endObject(cell);
}
#endif

} // namespace vm
} // namespace hermes
