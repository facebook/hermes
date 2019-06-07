/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_SNAPSHOTACCEPTOR_H
#define HERMES_VM_SNAPSHOTACCEPTOR_H

#include "hermes/VM/GC.h"
#include "hermes/VM/HeapSnapshot.h"
#include "hermes/VM/SlotAcceptorDefault.h"

#include <cstdint>
#include <functional>

namespace hermes {
namespace vm {

struct SnapshotRootAcceptor final : public SlotAcceptorWithNamesDefault {
  FacebookHeapSnapshot *snap;
  std::function<uintptr_t(const void *)> ptrToOffset;
  SnapshotRootAcceptor(
      GC &gc,
      FacebookHeapSnapshot *snap,
      std::function<uintptr_t(const void *)> ptrToOffset)
      : SlotAcceptorWithNamesDefault(gc),
        snap(snap),
        ptrToOffset(ptrToOffset) {}

  using SlotAcceptorWithNamesDefault::accept;

  void accept(void *&ptr, const char *name) override;
  void accept(HermesValue &hv, const char *name) override;
};

struct SnapshotAcceptor final : public SlotAcceptorWithNamesDefault {
  FacebookHeapSnapshot *snap;
  std::function<uintptr_t(const void *)> ptrToOffset;
  SnapshotAcceptor(
      GC &gc,
      FacebookHeapSnapshot *snap,
      std::function<uintptr_t(const void *)> ptrToOffset)
      : SlotAcceptorWithNamesDefault(gc),
        snap(snap),
        ptrToOffset(ptrToOffset) {}

  using SlotAcceptorWithNamesDefault::accept;

  void accept(void *&ptr, const char *name) override;
  void accept(HermesValue &hv, const char *name) override;
  void accept(SymbolID sym, const char *name) override;
  void accept(uint64_t value, const char *name);
};

} // namespace vm
} // namespace hermes

#endif
