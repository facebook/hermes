/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_PROPERTYACCESSOR_H
#define HERMES_VM_PROPERTYACCESSOR_H

#include "hermes/VM/Callable.h"

namespace hermes {
namespace vm {

/// This object is the value of a property which has a getter and/or setter.
class PropertyAccessor final : public GCCell {
 public:
  PropertyAccessor(
      Runtime *runtime,
      Handle<Callable> getter,
      Handle<Callable> setter)
      : GCCell(&runtime->getHeap(), &vt),
        getter(runtime, *getter, &runtime->getHeap()),
        setter(runtime, *setter, &runtime->getHeap()) {}

#ifdef HERMESVM_SERIALIZE
  /// Fast constructor used by deserialization. Don't do any GC allocation. Only
  /// calls super Constructor.
  PropertyAccessor(Deserializer &d);
#endif

  static const VTable vt;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::PropertyAccessorKind;
  }

  GCPointer<Callable> getter{};
  GCPointer<Callable> setter{};

  static CallResult<HermesValue>
  create(Runtime *runtime, Handle<Callable> getter, Handle<Callable> setter);
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_PROPERTYACCESSOR_H
