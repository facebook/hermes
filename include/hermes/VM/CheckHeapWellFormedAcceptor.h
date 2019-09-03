/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_CHECKHEAPWELLFORMEDACCEPTOR_H
#define HERMES_VM_CHECKHEAPWELLFORMEDACCEPTOR_H

#include "hermes/VM/HermesValue.h"
#include "hermes/VM/SlotAcceptorDefault.h"

namespace hermes {
namespace vm {

#ifdef HERMES_SLOW_DEBUG

/// An acceptor for checking that the heap is full of valid objects, with
/// pointers to valid objects.
struct CheckHeapWellFormedAcceptor final : public SlotAcceptorDefault,
                                           public WeakRootAcceptor {
  using SlotAcceptorDefault::accept;
  using SlotAcceptorDefault::SlotAcceptorDefault;

  void accept(void *&ptr) override;
  void acceptWeak(void *&ptr) override;
  void accept(HermesValue &hv) override;
  void accept(WeakRefBase &wr) override;
};

#endif

} // namespace vm
} // namespace hermes

#endif
