/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_CHECKHEAPWELLFORMEDACCEPTOR_H
#define HERMES_VM_CHECKHEAPWELLFORMEDACCEPTOR_H

#include "hermes/VM/HermesValue.h"
#include "hermes/VM/RootAndSlotAcceptorDefault.h"

namespace hermes {
namespace vm {

#ifdef HERMES_SLOW_DEBUG

/// An acceptor for checking that the heap is full of valid objects, with
/// pointers to valid objects.
struct CheckHeapWellFormedAcceptor final : public RootAndSlotAcceptorDefault,
                                           public WeakRootAcceptorDefault {
  using RootAndSlotAcceptorDefault::accept;
  using WeakRootAcceptorDefault::acceptWeak;

  CheckHeapWellFormedAcceptor(GC &gc);

  void accept(void *&ptr) override;
  void accept(const void *ptr);
  void acceptWeak(void *&ptr) override;
  void acceptHV(HermesValue &hv) override;
  void accept(WeakRefBase &wr) override;
  void acceptSym(SymbolID sym) override;
};

#endif

} // namespace vm
} // namespace hermes

#endif
