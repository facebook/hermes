/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSPROXY_H
#define HERMES_VM_JSPROXY_H

#include "hermes/VM/JSObject.h"

namespace hermes {
namespace vm {

class JSProxy : public JSObject {
 public:
  using Super = JSObject;
  friend void ProxyBuildMeta(const GCCell *cell, Metadata::Builder &mb);

  static const ObjectVTable vt;
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::ProxyKind;
  }

  static PseudoHandle<JSProxy> create(Runtime *runtime);

  static CallResult<HermesValue> create(
      Runtime *runtime,
      Handle<JSObject> prototype);

 private:
#ifdef HERMESVM_SERIALIZE
  explicit JSProxy(Deserializer &d);

  friend void ProxySerialize(Serializer &s, const GCCell *cell);
  friend void ProxyDeserialize(Deserializer &d, CellKind kind);
#endif

  JSProxy(Runtime *runtime, JSObject *parent, HiddenClass *clazz)
      : JSObject(runtime, &vt.base, parent, clazz) {}
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JSPROXY_H
