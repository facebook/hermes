/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
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

void MathSerialize(Serializer &s, const GCCell *cell) {}

void JSONSerialize(Serializer &s, const GCCell *cell) {}

void JSONDeserialize(Deserializer &d, CellKind kind) {}

void MathDeserialize(Deserializer &d, CellKind kind) {}

} // namespace vm
} // namespace hermes
