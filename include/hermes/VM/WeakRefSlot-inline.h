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

GCCell *WeakRefSlot::get(PointerBase &base, GC &gc) const {
  // Cannot check state() here because it can race with marking code.
  assert(hasValue() && "tried to access collected referent");
  return value_.root.getNonNull(base, gc);
}

} // namespace vm
} // namespace hermes

#endif
