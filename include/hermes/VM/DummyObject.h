/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_DUMMYOBJECT_H
#define HERMES_VM_DUMMYOBJECT_H

#include "hermes/VM/GCCell.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/HermesValue.h"

namespace hermes {
namespace vm {
namespace testhelpers {

/// Multi-purpose dummy object type that is used for tests.
struct DummyObject final : public GCCell {
  static const VTable vt;
  GCPointer<DummyObject> other;
  const uint32_t x;
  const uint32_t y;
  GCHermesValue hvBool;
  GCHermesValue hvDouble;
  GCHermesValue hvUndefined;
  GCHermesValue hvEmpty;
  GCHermesValue hvNative;
  GCHermesValue hvNull;

  DummyObject(GC *gc);

  void setPointer(GC *gc, DummyObject *obj);
  static DummyObject *create(GC *gc);
  static bool classof(const GCCell *cell);
};

} // namespace testhelpers
} // namespace vm
} // namespace hermes

#endif // HERMES_VM_DUMMYOBJECT_H
