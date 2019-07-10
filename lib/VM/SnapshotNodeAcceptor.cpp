/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/SnapshotNodeAcceptor.h"

namespace hermes {
namespace vm {

void SnapshotNodeAcceptor::accept(void *&ptr, const char *name) {
  if (ptr) {
    edgeCount++;
  }
}

void SnapshotNodeAcceptor::accept(HermesValue &hv, const char *name) {
  if (hv.isPointer()) {
    auto ptr = hv.getPointer();
    accept(ptr, name);
  }
}

void SnapshotNodeAcceptor::accept(SymbolID sym, const char *name) {
  // snap->addSymbolIdToCurrentObject(name, sym.unsafeGetRaw());
}

void SnapshotNodeAcceptor::accept(uint64_t value, const char *name) {
  // snap->addInternalToCurrentObject(name, value);
}

unsigned SnapshotNodeAcceptor::resetEdgeCount() {
  auto count = edgeCount;
  edgeCount = 0;
  return count;
}

} // namespace vm
} // namespace hermes
