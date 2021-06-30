/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSCALLABLEPROXY_H
#define HERMES_VM_JSCALLABLEPROXY_H

#include "hermes/VM/Callable.h"
#include "hermes/VM/JSProxy.h"

namespace hermes {
namespace vm {

/// JSCallableProxy acts like both a Proxy and like a NativeFunction.
/// See JSProxy for more discussion.
class JSCallableProxy : public NativeFunction {
 public:
  friend void CallableProxyBuildMeta(const GCCell *cell, Metadata::Builder &mb);
  friend detail::ProxySlots &detail::slots(JSObject *selfHandle);

  static const CallableVTable vt;
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::CallableProxyKind;
  }

  static PseudoHandle<JSCallableProxy> create(Runtime *runtime);

  static CallResult<HermesValue> create(
      Runtime *runtime,
      Handle<JSObject> prototype);

  void setTargetAndHandler(
      Runtime *runtime,
      Handle<JSObject> target,
      Handle<JSObject> handler);

  CallResult<bool> isConstructor(Runtime *runtime);

#ifdef HERMESVM_SERIALIZE
  explicit JSCallableProxy(Deserializer &d);

  friend void CallableProxySerialize(Serializer &s, const GCCell *cell);
  friend void CallableProxyDeserialize(Deserializer &d, CellKind kind);
#endif

  JSCallableProxy(
      Runtime *runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz)
      : NativeFunction(
            runtime,
            &vt.base.base,
            parent,
            clazz,
            nullptr /* context */,
            &JSCallableProxy::_proxyNativeCall) {}

 private:
  static CallResult<HermesValue>
  _proxyNativeCall(void *, Runtime *runtime, NativeArgs);

  static CallResult<PseudoHandle<JSObject>> _newObjectImpl(
      Handle<Callable> callable,
      Runtime *runtime,
      Handle<JSObject> protoHandle);

  detail::ProxySlots slots_;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JSCALLABLEPROXY_H
