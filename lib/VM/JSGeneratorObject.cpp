/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSGeneratorObject.h"

#include "hermes/VM/BuildMetadata.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSGeneratorObject

const ObjectVTable JSGeneratorObject::vt{
    VTable(CellKind::JSGeneratorObjectKind, cellSize<JSGeneratorObject>()),
    JSGeneratorObject::_getOwnIndexedRangeImpl,
    JSGeneratorObject::_haveOwnIndexedImpl,
    JSGeneratorObject::_getOwnIndexedPropertyFlagsImpl,
    JSGeneratorObject::_getOwnIndexedImpl,
    JSGeneratorObject::_setOwnIndexedImpl,
    JSGeneratorObject::_deleteOwnIndexedImpl,
    JSGeneratorObject::_checkAllOwnIndexedImpl,
};

void JSGeneratorObjectBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSGeneratorObject>());
  JSObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSGeneratorObject *>(cell);
  mb.setVTable(&JSGeneratorObject::vt);
  mb.addField("innerFunction", &self->innerFunction_);
}

CallResult<PseudoHandle<JSGeneratorObject>> JSGeneratorObject::create(
    Runtime &runtime,
    Handle<Callable> innerFunction,
    Handle<JSObject> parentHandle) {
  auto *cell = runtime.makeAFixed<JSGeneratorObject>(
      runtime,
      parentHandle,
      runtime.getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSGeneratorObject>()));
  cell->innerFunction_.set(runtime, *innerFunction, runtime.getHeap());
  return JSObjectInit::initToPseudoHandle(runtime, cell);
}

} // namespace vm
} // namespace hermes
