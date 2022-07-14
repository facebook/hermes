/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSWEAKREF
#define HERMES_VM_JSWEAKREF

#include "hermes/VM/CallResult.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/WeakRef.h"

namespace hermes {
namespace vm {

class JSWeakRef final : public JSObject {
 public:
  static const ObjectVTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::JSWeakRefKind;
  }

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::JSWeakRefKind;
  }

  /// Create a new WeakRef with prototype property \p parentHandle.
  static PseudoHandle<JSWeakRef> create(
      Runtime &runtime,
      Handle<JSObject> parentHandle);

  HermesValue deref(Runtime &runtime) const;

  // This allocates a new WeakRef pointing to the given target. This should only
  // be called once.
  void setTarget(Runtime &runtime, Handle<JSObject> target);

  friend void JSWeakRefBuildMeta(const GCCell *cell, Metadata::Builder &mb);

  static void _markWeakImpl(GCCell *cell, WeakRefAcceptor &acceptor);

  JSWeakRef(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz)
      : JSObject(runtime, *parent, *clazz), ref_(nullptr) {}

 private:
  WeakRef<JSObject> ref_;
};

} // namespace vm
} // namespace hermes
#endif // HERMES_VM_JSRWEAKREF
