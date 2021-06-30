/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSGenerator.h"

#include "hermes/VM/BuildMetadata.h"

#include "llvh/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSGenerator

const ObjectVTable JSGenerator::vt{
    VTable(CellKind::GeneratorKind, cellSize<JSGenerator>()),
    JSGenerator::_getOwnIndexedRangeImpl,
    JSGenerator::_haveOwnIndexedImpl,
    JSGenerator::_getOwnIndexedPropertyFlagsImpl,
    JSGenerator::_getOwnIndexedImpl,
    JSGenerator::_setOwnIndexedImpl,
    JSGenerator::_deleteOwnIndexedImpl,
    JSGenerator::_checkAllOwnIndexedImpl,
};

void GeneratorBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSGenerator>());
  ObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSGenerator *>(cell);
  mb.setVTable(&JSGenerator::vt.base);
  mb.addField("innerFunction", &self->innerFunction_);
}

#ifdef HERMESVM_SERIALIZE
JSGenerator::JSGenerator(Deserializer &d) : JSObject(d, &vt.base) {
  d.readRelocation(&innerFunction_, RelocationKind::GCPointer);
}

void GeneratorSerialize(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const JSGenerator>(cell);
  JSObject::serializeObjectImpl(
      s, cell, JSObject::numOverlapSlots<JSGenerator>());
  s.writeRelocation(self->innerFunction_.get(s.getRuntime()));
  s.endObject(cell);
}

void GeneratorDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::GeneratorKind && "Expected Generator");
  auto *cell = d.getRuntime()->makeAFixed<JSGenerator>(d);
  d.endObject(cell);
}
#endif

CallResult<PseudoHandle<JSGenerator>> JSGenerator::create(
    Runtime *runtime,
    Handle<GeneratorInnerFunction> innerFunction,
    Handle<JSObject> parentHandle) {
  auto *cell = runtime->makeAFixed<JSGenerator>(
      runtime,
      parentHandle,
      runtime->getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSGenerator>()));
  cell->innerFunction_.set(runtime, *innerFunction, &runtime->getHeap());
  return JSObjectInit::initToPseudoHandle(runtime, cell);
}

} // namespace vm
} // namespace hermes

#undef DEBUG_TYPE
