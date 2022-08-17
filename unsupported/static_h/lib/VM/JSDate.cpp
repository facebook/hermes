/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSDate.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/Runtime-inline.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSDate

const ObjectVTable JSDate::vt{
    VTable(CellKind::JSDateKind, cellSize<JSDate>()),
    JSDate::_getOwnIndexedRangeImpl,
    JSDate::_haveOwnIndexedImpl,
    JSDate::_getOwnIndexedPropertyFlagsImpl,
    JSDate::_getOwnIndexedImpl,
    JSDate::_setOwnIndexedImpl,
    JSDate::_deleteOwnIndexedImpl,
    JSDate::_checkAllOwnIndexedImpl,
};

void JSDateBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSDate>());
  JSObjectBuildMeta(cell, mb);
  mb.setVTable(&JSDate::vt);
}

PseudoHandle<JSDate>
JSDate::create(Runtime &runtime, double value, Handle<JSObject> parentHandle) {
  auto *cell = runtime.makeAFixed<JSDate>(
      runtime,
      value,
      parentHandle,
      runtime.getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSDate>()));
  return JSObjectInit::initToPseudoHandle(runtime, cell);
}

} // namespace vm
} // namespace hermes
