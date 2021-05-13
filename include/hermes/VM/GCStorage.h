/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
      GCBase::GCCallbacks *gcCallbacks,
      PointerBase *pointerBase,
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
    GCBase::HeapKind heapKind = (vmExperimentFlags & experiments::GenGC)
        ? GCBase::HeapKind::NCGEN
        : GCBase::HeapKind::HADES;
    GC *heap;
    switch (heapKind) {
      case GCBase::HeapKind::HADES:
        heap = new (storage_.buffer) HadesGC(
            gcCallbacks,
            pointerBase,
            gcConfig,
            crashMgr,
            provider,
            vmExperimentFlags);
        break;
      case GCBase::HeapKind::NCGEN:
        heap = new (storage_.buffer) GenGC(
            gcCallbacks,
            pointerBase,
            gcConfig,
            crashMgr,
            provider,
            vmExperimentFlags);
        break;
      case GCBase::HeapKind::MALLOC:
        llvm_unreachable(
            "MallocGC should not be used with the RuntimeGC build config");
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
    return reinterpret_cast<GC *>(storage_.buffer);
#else
    return &heap_;
#endif
  }

 private:
#ifdef HERMESVM_GC_RUNTIME
  llvh::AlignedCharArrayUnion<HadesGC, GenGC> storage_;
#else
  GC heap_;
#endif
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCSTORAGE_H
