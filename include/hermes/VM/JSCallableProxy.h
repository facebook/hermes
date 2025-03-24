/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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
  friend void JSCallableProxyBuildMeta(
      const GCCell *cell,
      Metadata::Builder &mb);
  friend detail::ProxySlots &detail::slots(JSObject *selfHandle);

  static const CallableVTable vt;
  static constexpr CellKind getCellKind() {
    return CellKind::JSCallableProxyKind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::JSCallableProxyKind;
  }

  static PseudoHandle<JSCallableProxy> create(Runtime &runtime);

  void setTargetAndHandler(
      Runtime &runtime,
      Handle<JSObject> target,
      Handle<JSObject> handler);

  JSCallableProxy(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz)
      : NativeFunction(
            runtime,
            parent,
            clazz,
            nullptr /* context */,
            &JSCallableProxy::_proxyNativeCall) {}

  /// \return the target function of this proxy. Can be nullptr (in the case of
  /// a revoked proxy.)
  Callable *getTarget(PointerBase &runtime) const {
    return vmcast_or_null<Callable>(slots_.target.get(runtime));
  }

 private:
  static CallResult<HermesValue>
  _proxyNativeCall(void *, Runtime &runtime, NativeArgs);

  detail::ProxySlots slots_;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JSCALLABLEPROXY_H
