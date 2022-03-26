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
void JSMathBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSMath>());
  JSObjectBuildMeta(cell, mb);
  mb.setVTable(&JSMath::vt);
}

void JSJSONBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSJSON>());
  JSObjectBuildMeta(cell, mb);
  mb.setVTable(&JSJSON::vt);
}

} // namespace vm
} // namespace hermes
