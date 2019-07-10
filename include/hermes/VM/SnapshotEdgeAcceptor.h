/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_SNAPSHOTEDGEACCEPTOR_H
#define HERMES_VM_SNAPSHOTEDGEACCEPTOR_H

#include "hermes/VM/GC.h"
#include "hermes/VM/SlotAcceptorDefault.h"

#include <cstdint>
#include <functional>

namespace hermes {
namespace vm {

struct SnapshotEdgeAcceptor final : public SlotAcceptorWithNamesDefault {
  V8HeapSnapshot &snap;
  std::function<uintptr_t(const void *)> ptrToOffset;
  std::function<V8HeapSnapshot::StringID(llvm::StringRef str)> stringToID;

  SnapshotEdgeAcceptor(
      GC &gc,
      V8HeapSnapshot &snap,
      std::function<uintptr_t(const void *)> ptrToOffset,
      std::function<V8HeapSnapshot::StringID(llvm::StringRef str)> stringToID)
      : SlotAcceptorWithNamesDefault(gc),
        snap(snap),
        ptrToOffset(ptrToOffset),
        stringToID(stringToID) {}

  using SlotAcceptorWithNamesDefault::accept;

  void accept(void *&ptr, const char *name) override;
  void accept(HermesValue &hv, const char *name) override;
  void accept(SymbolID sym, const char *name) override;
  void accept(uint64_t value, const char *name);
};

} // namespace vm
} // namespace hermes

#endif
