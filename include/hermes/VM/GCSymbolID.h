/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCSYMBOLID_H
#define HERMES_VM_GCSYMBOLID_H

#include "hermes/VM/GC.h"
#include "hermes/VM/SymbolID.h"

namespace hermes {
namespace vm {

/// A SymbolID that lives on the JS heap. Hides the assignment operator, but
/// provides set operations that do a write barrier.
class GCSymbolID final : public SymbolID {
 public:
  constexpr GCSymbolID() : SymbolID() {}

  /// No write barrier needed for construction.
  explicit GCSymbolID(SymbolID id) : SymbolID(id) {}

  GCSymbolID(const GCSymbolID &) = default;
  ~GCSymbolID() = default;

  /// Deleted so a write barrier is used instead.
  GCSymbolID &operator=(const GCSymbolID &) = delete;

  /// Write a new value into this. Performs a write barrier for some GCs.
  GCSymbolID &set(SymbolID sym, GC *gc) {
    gc->writeBarrier(*this);
    id_ = sym.unsafeGetRaw();
    return *this;
  }
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCSYMBOLID_H
