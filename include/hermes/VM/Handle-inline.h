/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_HANDLE_INLINE_H
#define HERMES_VM_HANDLE_INLINE_H

#include "hermes/VM/Handle.h"

#include "hermes/VM/Casting.h"
#include "hermes/VM/HandleRootOwner.h"

namespace hermes {
namespace vm {

template <typename T>
template <typename U>
inline PseudoHandle<T> PseudoHandle<T>::vmcast(PseudoHandle<U> &&other) {
  auto result = PseudoHandle<T>::create(hermes::vm::vmcast<T>(other.get()));
  other.invalidate();
  return result;
}

template <typename T>
template <typename U>
inline PseudoHandle<T> PseudoHandle<T>::dyn_vmcast(PseudoHandle<U> &&other) {
  auto result = PseudoHandle<T>::create(hermes::vm::dyn_vmcast<T>(other.get()));
  other.invalidate();
  return result;
}

/// Allocate a new handle in the current GCScope
inline HandleBase::HandleBase(HandleRootOwner &runtime, HermesValue value)
    : handle_(runtime.newPinnedHermesValue(value)) {
#ifndef NDEBUG
  gcScope_ = runtime.getTopGCScope();
  ++gcScope_->numActiveHandles_;
#endif
}

#ifndef NDEBUG
inline HandleBase::HandleBase(const HandleBase &sc) : handle_(sc.handle_) {
  gcScope_ = sc.gcScope_;
  if (gcScope_) {
    ++gcScope_->numActiveHandles_;
  }
}
#endif

#ifndef NDEBUG
inline HandleBase::~HandleBase() {
  if (gcScope_) {
    --gcScope_->numActiveHandles_;
  }
}
#endif

#ifndef NDEBUG
inline HandleBase &HandleBase::operator=(const HandleBase &other) {
  if (gcScope_)
    --gcScope_->numActiveHandles_;
  if ((gcScope_ = other.gcScope_) != nullptr)
    ++gcScope_->numActiveHandles_;
  handle_ = other.handle_;
  return *this;
}
#endif

template <typename T>
inline Handle<T> Handle<T>::dyn_vmcast(const HandleBase &other) {
  return vmisa<T>(other.getHermesValue())
      ? Handle<T>(other, true)
      : HandleRootOwner::makeNullHandle<T>();
}

} // namespace vm
} // namespace hermes

#endif
