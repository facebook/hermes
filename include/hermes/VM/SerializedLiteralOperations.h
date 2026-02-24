/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SERIALIZEDLITERALOPERATIONS_H
#define HERMES_VM_SERIALIZEDLITERALOPERATIONS_H

#include "hermes/BCGen/SerializedLiteralParser.h"
#include "hermes/VM/HiddenClass.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

/// Add the keys in \p buffer to the hidden class \p clazz, and return the
/// resulting hidden class.
/// \param buffer the key buffer starting at the first key to add.
/// \param numProps the number of keys to add.
/// \param rootClazz the root hidden class to add the keys to. It must not
///        already contain any of the keys in \p buffer.
/// \param getSym a function to convert a StringID to a SymbolID.
template <typename F>
HiddenClass *addBufferPropertiesToHiddenClass(
    Runtime &runtime,
    llvh::ArrayRef<uint8_t> buffer,
    uint32_t numProps,
    HiddenClass *rootClazz,
    F getSym) {
  struct : public Locals {
    PinnedValue<HiddenClass> clazz;
    PinnedValue<> tmpKey;
  } lv;
  LocalsRAII lraii(runtime, &lv);

  lv.clazz = rootClazz;

  GCScopeMarkerRAII marker{runtime};
  // Set up the visitor to populate keys in the hidden class.
  struct {
    void visitStringID(StringID id) {
      SymbolID sym = getSym(id);
      auto addResult = HiddenClass::addProperty(
          clazz, runtime, sym, PropertyFlags::defaultNewNamedPropertyFlags());
      clazz = addResult->first;
      marker.flush();
    }
    void visitNumber(double d) {
      tmpHandleKey = HermesValue::encodeTrustedNumberValue(d);
      // valueToSymbolID cannot fail because the key is known to be uint32.
      Handle<SymbolID> symHandle = *valueToSymbolID(runtime, tmpHandleKey);
      auto addResult = HiddenClass::addProperty(
          clazz,
          runtime,
          *symHandle,
          PropertyFlags::defaultNewNamedPropertyFlags());
      clazz = addResult->first;
      marker.flush();
    }
    void visitNull() {
      llvm_unreachable("Keys cannot be null");
    }
    void visitUndefined() {
      llvm_unreachable("Keys cannot be undefined");
    }
    void visitBool(bool b) {
      llvm_unreachable("Keys cannot be boolean");
    }

    PinnedValue<HiddenClass> &clazz;
    PinnedValue<> &tmpHandleKey;
    GCScopeMarkerRAII &marker;
    Runtime &runtime;
    F getSym;
  } v{lv.clazz, lv.tmpKey, marker, runtime, std::move(getSym)};

  // Visit each literal in the buffer and add it as a property.
  SerializedLiteralParser::parse(buffer, numProps, v);

  return *lv.clazz;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SERIALIZEDLITERALOPERATIONS_H
