/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSDate.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/Runtime-inline.h"

#include "llvh/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSDate

const ObjectVTable JSDate::vt{
    VTable(CellKind::DateKind, cellSize<JSDate>()),
    JSDate::_getOwnIndexedRangeImpl,
    JSDate::_haveOwnIndexedImpl,
    JSDate::_getOwnIndexedPropertyFlagsImpl,
    JSDate::_getOwnIndexedImpl,
    JSDate::_setOwnIndexedImpl,
    JSDate::_deleteOwnIndexedImpl,
    JSDate::_checkAllOwnIndexedImpl,
};

void DateBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSDate>());
  ObjectBuildMeta(cell, mb);
  mb.setVTable(&JSDate::vt.base);
}

#ifdef HERMESVM_SERIALIZE
JSDate::JSDate(Deserializer &d) : JSObject(d, &vt.base) {
  HermesValue hv;
  d.readHermesValue(&hv);
  primitiveValue_ = hv.getNumber();
}

void DateSerialize(Serializer &s, const GCCell *cell) {
  JSObject::serializeObjectImpl(s, cell, JSObject::numOverlapSlots<JSDate>());
  const auto *self = static_cast<const JSDate *>(cell);
  s.writeHermesValue(HermesValue::encodeNumberValue(self->primitiveValue_));
  s.endObject(cell);
}

void DateDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::DateKind && "Expected Date");
  auto *cell = d.getRuntime()->makeAFixed<JSDate>(d);
  d.endObject(cell);
}
#endif

PseudoHandle<JSDate>
JSDate::create(Runtime *runtime, double value, Handle<JSObject> parentHandle) {
  auto *cell = runtime->makeAFixed<JSDate>(
      runtime,
      value,
      parentHandle,
      runtime->getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSDate>()));
  return JSObjectInit::initToPseudoHandle(runtime, cell);
}

} // namespace vm
} // namespace hermes

#undef DEBUG_TYPE
