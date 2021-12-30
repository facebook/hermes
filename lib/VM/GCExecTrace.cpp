/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/GCExecTrace.h"

#include <hermes/Support/JSONEmitter.h>

namespace hermes {
namespace vm {

#ifdef HERMESVM_API_TRACE_DEBUG
void GCExecTrace::emit(::hermes::JSONEmitter &json) const {
  json.emitKey("gcExecTrace");
  json.openArray();
  for (const auto &gcRec : gcs_) {
    json.openDict();
    json.emitKeyValue("isYG", gcRec.isYG);
    json.emitKeyValue("ygUsedBefore", gcRec.ygUsedBefore);
    json.emitKeyValue("ygUsedAfter", gcRec.ygUsedAfter);
    json.emitKeyValue("ogUsedBefore", gcRec.ogUsedBefore);
    json.emitKeyValue("ogUsedAfter", gcRec.ogUsedAfter);
    if (gcRec.isYG) {
      json.emitKey("ygAllocs");
      json.openArray();
      for (auto &alloc : gcRec.allocSizes) {
        json.openDict();
        json.emitKeyValue("kind", cellKindStr(std::get<0>(alloc)));
        json.emitKeyValue("sz", std::get<1>(alloc));
        json.emitKeyValue("str", std::get<2>(alloc));
        json.closeDict();
      }
      json.closeArray();
    }
    json.closeDict();
  }
  json.closeArray();
}
#endif

} // namespace vm
} // namespace hermes
