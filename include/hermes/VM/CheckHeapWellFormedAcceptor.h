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

namespace hermes {
namespace vm {

#ifdef HERMES_SLOW_DEBUG

/// An acceptor for checking that the heap is full of valid objects, with
/// pointers to valid objects.
struct CheckHeapWellFormedAcceptor final : public RootAndSlotAcceptor,
                                           public WeakRootAcceptor {
  explicit CheckHeapWellFormedAcceptor(GCBase &gc);

  /// Root acceptors.
  void accept(GCCell *&ptr) override;
  void accept(PinnedHermesValue &hv) override;
  void acceptNullable(PinnedHermesValue &hv) override;
  void accept(const RootSymbolID &sym) override;
  void acceptWeak(WeakRootBase &ptr) override;
  void acceptWeakSym(WeakRootSymbolID &ws) override;

  /// Heap acceptors.
  void accept(GCPointerBase &ptr) override;
  void accept(GCHermesValueBase &hv) override;
  void accept(GCSmallHermesValueBase &hv) override;
  void accept(const GCSymbolID &sym) override;

  /// Internal acceptors.
  void accept(const GCCell *ptr);
  void acceptWeak(GCCell *&ptr);
  void acceptHV(HermesValue &hv);
  void acceptSHV(SmallHermesValue &hv);
  void acceptSym(SymbolID sym);

  GCBase &gc;
};

#endif

} // namespace vm
} // namespace hermes

#endif
