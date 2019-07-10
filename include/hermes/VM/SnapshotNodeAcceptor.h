/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_SNAPSHOTNODEACCEPTOR_H
#define HERMES_VM_SNAPSHOTNODEACCEPTOR_H

#include "hermes/VM/GC.h"
#include "hermes/VM/SlotAcceptorDefault.h"

#include <cstdint>
#include <functional>

namespace hermes {
namespace vm {

struct SnapshotNodeAcceptor final : public SlotAcceptorWithNamesDefault {
  unsigned edgeCount = 0;

  SnapshotNodeAcceptor(GC &gc) : SlotAcceptorWithNamesDefault(gc) {}

  using SlotAcceptorWithNamesDefault::accept;

  void accept(void *&ptr, const char *name) override;
  void accept(HermesValue &hv, const char *name) override;
  void accept(SymbolID sym, const char *name) override;
  void accept(uint64_t value, const char *name);
  unsigned resetEdgeCount();
};

} // namespace vm
} // namespace hermes

#endif
