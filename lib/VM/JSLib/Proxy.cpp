/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// ES9 26.2 Proxy Objects
//===----------------------------------------------------------------------===//

#include "JSLibInternal.h"

namespace hermes {
namespace vm {

namespace {

CallResult<HermesValue>
proxyConstructor(void *, Runtime *runtime, NativeArgs args) {
  // 5. Let P be a newly created object.
  // 6. Set P’s essential internal methods (except for [[Call]] and
  // [[Construct]]) to the definitions specified in 9.5.
  PseudoHandle<JSProxy> proxy = JSProxy::create(runtime);

  // Return P.
  return proxy.getHermesValue();
}

} // namespace

Handle<JSObject> createProxyConstructor(Runtime *runtime) {
  Handle<NativeConstructor> cons = defineSystemConstructor<JSProxy>(
      runtime,
      Predefined::getSymbolID(Predefined::Proxy),
      proxyConstructor,
      runtime->makeNullHandle<JSObject>(),
      2,
      CellKind::ProxyKind);
  return cons;
}

} // namespace vm
} // namespace hermes
