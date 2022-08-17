/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SYMBOLID_INLINE_H
#define HERMES_VM_SYMBOLID_INLINE_H

#include "hermes/VM/SymbolID.h"

#include "hermes/VM/GC.h"

namespace hermes {
namespace vm {

inline GCSymbolID &GCSymbolID::set(SymbolID sym, GC &gc) {
  gc.snapshotWriteBarrier(this);
  id_ = sym.unsafeGetRaw();
  return *this;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SYMBOLID_INLINE_H
