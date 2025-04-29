/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCSTORAGE_H
#define HERMES_VM_GCSTORAGE_H

#include "hermes/VM/GC.h"

namespace hermes {
namespace vm {

/// Helper class to store a GC object inline in Runtime. When combined with
/// StackRuntime, this is useful for preserving GC state on the stack.
class GCStorage {
 public:
  GCStorage(
      GCBase::GCCallbacks &gcCallbacks,
      PointerBase &pointerBase,
      const GCConfig &gcConfig,
      std::shared_ptr<CrashManager> crashMgr,
      std::shared_ptr<StorageProvider> provider,
      experiments::VMExperimentFlags vmExperimentFlags)
      : heap_(
            gcCallbacks,
            pointerBase,
            gcConfig,
            crashMgr,
            provider,
            vmExperimentFlags) {}

  GC *get() {
    return &heap_;
  }

 private:
  GC heap_;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCSTORAGE_H
