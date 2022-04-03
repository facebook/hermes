/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSGenerator.h"

#include "hermes/VM/BuildMetadata.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSGenerator

const ObjectVTable JSGenerator::vt{
    VTable(CellKind::JSGeneratorKind, cellSize<JSGenerator>()),
    JSGenerator::_getOwnIndexedRangeImpl,
    JSGenerator::_haveOwnIndexedImpl,
    JSGenerator::_getOwnIndexedPropertyFlagsImpl,
    JSGenerator::_getOwnIndexedImpl,
    JSGenerator::_setOwnIndexedImpl,
    JSGenerator::_deleteOwnIndexedImpl,
    JSGenerator::_checkAllOwnIndexedImpl,
};

void JSGeneratorBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSGenerator>());
  JSObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSGenerator *>(cell);
  mb.setVTable(&JSGenerator::vt);
  mb.addField("innerFunction", &self->innerFunction_);
}

CallResult<PseudoHandle<JSGenerator>> JSGenerator::create(
    Runtime &runtime,
    Handle<GeneratorInnerFunction> innerFunction,
    Handle<JSObject> parentHandle) {
  auto *cell = runtime.makeAFixed<JSGenerator>(
      runtime,
      parentHandle,
      runtime.getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSGenerator>()));
  cell->innerFunction_.set(runtime, *innerFunction, runtime.getHeap());
  return JSObjectInit::initToPseudoHandle(runtime, cell);
}

} // namespace vm
} // namespace hermes
