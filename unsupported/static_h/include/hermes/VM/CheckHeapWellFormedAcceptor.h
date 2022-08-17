/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_CHECKHEAPWELLFORMEDACCEPTOR_H
#define HERMES_VM_CHECKHEAPWELLFORMEDACCEPTOR_H

#include "hermes/VM/GCBase.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/RootAndSlotAcceptorDefault.h"

namespace hermes {
namespace vm {

#ifdef HERMES_SLOW_DEBUG

/// An acceptor for checking that the heap is full of valid objects, with
/// pointers to valid objects.
struct CheckHeapWellFormedAcceptor final : public RootAndSlotAcceptorDefault,
                                           public WeakAcceptorDefault {
  using RootAndSlotAcceptorDefault::accept;
  using WeakAcceptorDefault::acceptWeak;

  explicit CheckHeapWellFormedAcceptor(GCBase &gc);

  void accept(GCCell *&ptr) override;
  void accept(const GCCell *ptr);
  void acceptWeak(GCCell *&ptr) override;
  void acceptHV(HermesValue &hv) override;
  void acceptSHV(SmallHermesValue &hv) override;
  void accept(WeakRefBase &wr) override;
  void acceptSym(SymbolID sym) override;

  GCBase &gc;
};

#endif

} // namespace vm
} // namespace hermes

#endif
