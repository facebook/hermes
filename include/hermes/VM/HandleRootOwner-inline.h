/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_HANDLEROOTOWNER_INLINE_H
#define HERMES_VM_HANDLEROOTOWNER_INLINE_H

#include "hermes/VM/Handle.h"
#include "hermes/VM/HandleRootOwner.h"

namespace hermes {
namespace vm {

inline Handle<HermesValue> HandleRootOwner::makeHandle(HermesValue value) {
  // This is the same as Handle<HermesValue>(getTopGCScope(), value), but it's
  // this way for historical reasons.  The source could be updated for clarity
  // if the generated code doesn't change.
  return Handle<HermesValue>(*this, value);
}
template <class T>
inline Handle<T> HandleRootOwner::makeHandle(T *p) {
  return Handle<T>(*this, p);
}
template <class T>
inline Handle<T> HandleRootOwner::makeHandle(HermesValue value) {
  return Handle<T>(*this, vmcast<T>(value));
}
inline Handle<SymbolID> HandleRootOwner::makeHandle(SymbolID value) {
  return Handle<SymbolID>(*this, value);
}
template <class T>
inline Handle<T> HandleRootOwner::makeHandle(PseudoHandle<T> &&pseudo) {
  Handle<T> res{*this, pseudo.get()};
  pseudo.invalidate();
  return res;
}
template <class T>
inline Handle<T> HandleRootOwner::makeHandle(
    PseudoHandle<HermesValue> &&pseudo) {
  Handle<T> res{*this, PseudoHandle<T>::vmcast(std::move(pseudo)).get()};
  return res;
}

inline MutableHandle<HermesValue> HandleRootOwner::makeMutableHandle(
    HermesValue value) {
  return MutableHandle<HermesValue>(*this, value);
}
template <class T>
inline MutableHandle<T> HandleRootOwner::makeMutableHandle(T *p) {
  return MutableHandle<T>(*this, p);
}

template <class T>
inline Handle<T> HandleRootOwner::makeNullHandle() {
  return Handle<T>::vmcast_or_null(&nullPointer_);
}

inline Handle<HermesValue> HandleRootOwner::getUndefinedValue() {
  return Handle<HermesValue>(&undefinedValue_);
}

inline Handle<HermesValue> HandleRootOwner::getNullValue() {
  return Handle<HermesValue>(&nullValue_);
}

inline Handle<HermesValue> HandleRootOwner::getEmptyValue() {
  return Handle<HermesValue>(&emptyValue_);
}

inline Handle<HermesValue> HandleRootOwner::getBoolValue(bool b) {
  return Handle<HermesValue>(b ? &trueValue_ : &falseValue_);
}

inline Handle<HermesValue> HandleRootOwner::getZeroValue() {
  return Handle<HermesValue>(&zeroValue_);
}

inline Handle<HermesValue> HandleRootOwner::getOneValue() {
  return Handle<HermesValue>(&oneValue_);
}

inline Handle<HermesValue> HandleRootOwner::getNegOneValue() {
  return Handle<HermesValue>(&negOneValue_);
}

inline GCScope *HandleRootOwner::getTopGCScope() {
  return topGCScope_;
}

inline PinnedHermesValue *HandleRootOwner::newPinnedHermesValue(
    GCScope *inScope,
    HermesValue value) {
  assert(inScope && "Target GCScope can't be null");
  return inScope->newPinnedHermesValue(value);
}

inline PinnedHermesValue *HandleRootOwner::newPinnedHermesValue(
    HermesValue value) {
  assert(topGCScope_ && "no active GCScope");
  return newPinnedHermesValue(topGCScope_, value);
}

} // namespace vm
} // namespace hermes

#endif
