/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_WEAKREFHOLDER_H
#define HERMES_VM_WEAKREFHOLDER_H

#include "hermes/VM/Runtime.h"
#include "hermes/VM/WeakRef.h"

namespace hermes {
namespace vm {

/// RAII class to hold a reference to a WeakRef.
/// Allocates a new WeakRef slot in the GC when WeakRefHolder is constructed,
/// storing a WeakRef in the Runtime so that it will be marked during any GCs.
/// Upon destruction, will remove the WeakRef from the Runtime.
/// NOTE: This should only be allocated on the stack.
template <class T>
class WeakRefHolder {
  Runtime *runtime_;

  /// WeakRef that's being held by this holder.
  WeakRef<T> weakRef_;

 public:
  /// Allocate a new WeakRef and store it in the Runtime.
  explicit WeakRefHolder(Runtime *runtime, Handle<T> handle)
      : runtime_(runtime),
        weakRef_(WeakRef<T>::vmcast(
            runtime_->pushWeakRef(&runtime->getHeap(), Handle<>(handle)))) {}

  /// Remove the WeakRef from the Runtime.
  ~WeakRefHolder() {
    runtime_->popWeakRef(weakRef_);
  }

  WeakRef<T> &operator*() {
    return get();
  }
  WeakRef<T> *operator->() {
    return &get();
  }

  /// \return a copy of the WeakRef stored in the Runtime.
  WeakRef<T> &get() {
    return weakRef_;
  }
};

} // namespace vm
} // namespace hermes

#endif
