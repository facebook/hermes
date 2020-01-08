/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSProxy.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSProxy

const ObjectVTable JSProxy::vt{
    VTable(CellKind::ProxyKind, cellSize<JSProxy>()),
    JSProxy::_getOwnIndexedRangeImpl,
    JSProxy::_haveOwnIndexedImpl,
    JSProxy::_getOwnIndexedPropertyFlagsImpl,
    JSProxy::_getOwnIndexedImpl,
    JSProxy::_setOwnIndexedImpl,
    JSProxy::_deleteOwnIndexedImpl,
    JSProxy::_checkAllOwnIndexedImpl,
};

void ProxyBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
}

#ifdef HERMESVM_SERIALIZE
JSProxy::JSProxy(Deserializer &d) : JSObject(d, &vt.base) {}

void ProxySerialize(Serializer &s, const GCCell *cell) {
  JSObject::serializeObjectImpl(s, cell);
  s.endObject(cell);
}

void ProxyDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::ProxyKind && "Expected Proxy");
  void *mem = d.getRuntime()->alloc(cellSize<JSProxy>());
  auto *cell = new (mem) JSProxy(d);
  d.endObject(cell);
}
#endif

PseudoHandle<JSProxy> JSProxy::create(Runtime *runtime) {
  void *mem = runtime->alloc(cellSize<JSProxy>());
  return createPseudoHandle(
      JSObject::allocateSmallPropStorage(new (mem) JSProxy(
          runtime,
          runtime->objectPrototypeRawPtr,
          runtime->getHiddenClassForPrototypeRaw(
              runtime->objectPrototypeRawPtr, ANONYMOUS_PROPERTY_SLOTS))));
}

CallResult<HermesValue> JSProxy::create(
    Runtime *runtime,
    Handle<JSObject> prototype) {
  assert(
      prototype.get() == runtime->objectPrototypeRawPtr &&
      "JSProxy::create() can only be used with object prototype");
  return create(runtime).getHermesValue();
}

} // namespace vm
} // namespace hermes
