/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

} // namespace vm
} // namespace hermes
