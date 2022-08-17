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
#ifndef HERMESVM_GC_RUNTIME
      : heap_(
            gcCallbacks,
            pointerBase,
            gcConfig,
            crashMgr,
            provider,
            vmExperimentFlags) {
  }
#else
  {
    GCBase::HeapKind heapKind = GCBase::HeapKind::HadesGC;
    GC *heap;
    switch (heapKind) {
#define GC_KIND(kind)            \
  case GCBase::HeapKind::kind:   \
    heap = new (&storage_) kind( \
        gcCallbacks,             \
        pointerBase,             \
        gcConfig,                \
        crashMgr,                \
        provider,                \
        vmExperimentFlags);      \
    break;
      RUNTIME_GC_KINDS
#undef GC_KIND
      default:
        llvm_unreachable("No other valid GC for RuntimeGC");
    }
    assert(heap == get() && "Cannot safely cast buffer to GC");
    (void)heap;
  }

  ~GCStorage() {
    get()->~GC();
  }
#endif

  GC *get() {
#ifdef HERMESVM_GC_RUNTIME
    return reinterpret_cast<GC *>(&storage_);
#else
    return &heap_;
#endif
  }

 private:
#ifdef HERMESVM_GC_RUNTIME
  std::aligned_storage<std::max({
#define GC_KIND(kind) sizeof(kind),
      RUNTIME_GC_KINDS
#undef GC_KIND
  })>::type storage_;
#else
  GC heap_;
#endif
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCSTORAGE_H
