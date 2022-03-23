/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SYMBOLREGISTRY_H
#define HERMES_VM_SYMBOLREGISTRY_H

#include "hermes/Support/OptValue.h"
#include "hermes/VM/CallResult.h"
#include "hermes/VM/Handle.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/SymbolID.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/DenseSet.h"

namespace hermes {
namespace vm {
struct SlotAcceptor;
class StringPrimitive;
class Runtime;

/// The global symbol registry, used to store information on which Symbols
/// have been created with Symbol.for and looked up with Symbol.keyFor.
///
/// Contains a mapping from the string keys for each symbol to the SymbolID
/// that was created with Symbol.for. This mapping is consulted every time
/// Symbol.for is called.
///
/// To get Symbol.keyFor, we also keep a set of the registered symbols.
/// Each symbol's description is the key string, so we use its
/// entry in the IdentifierTable to store the keyFor.
///
/// Note that we only know if a symbol in the IdentifierTable is a globally
/// registered symbol by asking the SymbolRegistry first, so the SymbolRegistry
/// must be asked whether a symbol is globally registered before retrieving its
/// description.
class SymbolRegistry {
  /// OrderedHashMap from the string key to the SymbolID.
  /// Uses PinnedHermesValue to avoid allocating a new handle whenever we want
  /// to use it.
  PinnedHermesValue stringMap_;

  /// The set of SymbolIDs that have been registered in the SymbolRegistry.
  /// Note that these are guaranteed to be values in stringMap_,
  /// and therefore their backing strings will be kept alive.
  llvh::DenseSet<SymbolID> registeredSymbols_{};

 public:
  explicit SymbolRegistry() {}

  /// Initialize the data structures of the SymbolRegistry.
  /// Must be performed before any other operations can be done.
  void init(Runtime &runtime);

  /// Mark the Strings and Symbols in the registry as roots.
  void markRoots(RootAcceptor &acceptor);

  /// Get the SymbolID for \p key, adding it if it doesn't exist.
  /// \param key the key for which to retrieve the SymbolID.
  /// \return the symbol associated with the key.
  /// Note: We return the raw SymbolID because the registry will necessarily
  /// keep the SymbolID alive.
  CallResult<SymbolID> getSymbolForKey(
      Runtime &runtime,
      Handle<StringPrimitive> key);

  /// \return true if \p symbol has been registered in the SymbolRegistry.
  bool hasSymbol(SymbolID symbol) const {
    return registeredSymbols_.find(symbol) != registeredSymbols_.end();
  }
};

} // namespace vm
} // namespace hermes

#endif
