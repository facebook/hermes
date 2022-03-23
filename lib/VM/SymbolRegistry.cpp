/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/SymbolRegistry.h"

#include "hermes/VM/Handle-inline.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/OrderedHashMap.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/SlotAcceptor.h"
#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

void SymbolRegistry::init(Runtime &runtime) {
  stringMap_ = OrderedHashMap::create(runtime)->getHermesValue();
}

/// Mark the Strings and Symbols in the registry as roots.
void SymbolRegistry::markRoots(RootAcceptor &acceptor) {
  acceptor.accept(stringMap_);
  // registeredSymbols_ doesn't need to be marked, because its contents are a
  // copy of the symbols present in the stringMap_.
}

CallResult<SymbolID> SymbolRegistry::getSymbolForKey(
    Runtime &runtime,
    Handle<StringPrimitive> key) {
  HashMapEntry *it = OrderedHashMap::find(
      Handle<OrderedHashMap>::vmcast(&stringMap_), runtime, key);
  if (it) {
    return it->value.getSymbol();
  }

  auto symbolRes =
      runtime.getIdentifierTable().createNotUniquedSymbol(runtime, key);
  if (LLVM_UNLIKELY(symbolRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<SymbolID> symbol = runtime.makeHandle(*symbolRes);

  if (LLVM_UNLIKELY(
          OrderedHashMap::insert(
              Handle<OrderedHashMap>::vmcast(&stringMap_),
              runtime,
              key,
              symbol) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  registeredSymbols_.insert(symbol.get());
  return symbol.get();
}

} // namespace vm
} // namespace hermes
