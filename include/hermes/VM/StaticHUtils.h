/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_STATICH_UTILS_H
#define HERMES_VM_STATICH_UTILS_H

#include "hermes/VM/Runtime.h"
#include "hermes/VM/static_h.h"

namespace hermes::vm {

struct AddPropertyCacheEntry;

inline Runtime &getRuntime(SHRuntime *shr) {
  return *static_cast<Runtime *>(shr);
}
inline SHRuntime *getSHRuntime(Runtime &runtime) {
  return static_cast<SHRuntime *>(&runtime);
}

inline PinnedHermesValue *toPHV(SHLegacyValue *shv) {
  return static_cast<PinnedHermesValue *>(shv);
}
inline const PinnedHermesValue *toPHV(const SHLegacyValue *shv) {
  return static_cast<const PinnedHermesValue *>(shv);
}

/// Free the \p unit, and all associated data.
void sh_unit_done(Runtime &runtime, SHUnit *unit);

/// Calculate the size of allocated memory not tracked by GC.
size_t sh_unit_additional_memory_size(const SHUnit *unit);

/// Mark the non-weak roots owned by this unit.
void sh_unit_mark_roots(
    SHUnit *unit,
    RootAcceptorWithNames &acceptor,
    bool markLongLived);

/// Mark the short lived weak roots owned by this unit.
void sh_unit_mark_weak_roots(
    SHUnit *unit,
    WeakRootAcceptor &acceptor,
    bool markLongLived);

} // namespace hermes::vm

#endif // HERMES_VM_STATICH_UTILS_H
