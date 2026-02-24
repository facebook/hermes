/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_WEAKREFSLOT_INLINE_H
#define HERMES_VM_WEAKREFSLOT_INLINE_H

#include "hermes/VM/WeakRefSlot.h"
#include "hermes/VM/WeakRoot-inline.h"

namespace hermes {
namespace vm {

void WeakRefSlot::setPointer(CompressedPointer ptr) {
  // Cannot check state() here because it can race with marking code.
  value_.root.setObject(ptr);
}

GCCell *WeakRefSlot::getObject(PointerBase &base, GC &gc) const {
  // Cannot check state() here because it can race with marking code.
  assert(hasValue() && "tried to access collected referent");
  return value_.root.getObject(base, gc);
}

HermesValue WeakRefSlot::getValueNoBarrierUnsafe(PointerBase &base) const {
  // We don't use unboxToHV here because we know the exact possible types.
  if (value_.root.isObject())
    return HermesValue::encodeObjectValue(
        value_.root.getObjectNoBarrierUnsafe(base));
  return HermesValue::encodeSymbolValue(value_.root.getSymbolNoBarrierUnsafe());
}

} // namespace vm
} // namespace hermes

#endif
