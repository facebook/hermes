/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_HEAPRUNTIME_H
#define HERMES_VM_HEAPRUNTIME_H

#include "hermes/VM/StorageProvider.h"

namespace hermes {
namespace vm {
/// Helper class to create a runtime object instantiated on the heap and manage
/// its lifetime via an aliased shared_ptr. It can be used with any
/// StorageProvider, and will consume the entirety of the next available segment
/// from the provider.
template <typename RT>
class HeapRuntime {
 public:
  ~HeapRuntime() {
    runtime_->~RT();
    sp_->deleteStorage(runtime_);
  }

  /// Allocate a segment and create an aliased shared_ptr that points to the
  /// start of the segment, but manages the lifetime of a HeapRuntime<RT>. The
  /// caller must instantiate an RT in the returned storage, and its destructor
  /// will automatically be called when the HeapRuntime is destroyed.
  static std::shared_ptr<RT> create(std::shared_ptr<StorageProvider> sp) {
    auto hrt = std::shared_ptr<HeapRuntime>(new HeapRuntime(std::move(sp)));
    return std::shared_ptr<RT>(hrt, hrt->runtime_);
  }

 private:
  HeapRuntime(std::shared_ptr<StorageProvider> sp) : sp_{std::move(sp)} {
    auto ptrOrError = sp_->newStorage("hermes-rt");
    if (!ptrOrError)
      hermes_fatal("Cannot initialize Runtime storage.", ptrOrError.getError());
    static_assert(sizeof(RT) < AlignedStorage::size(), "Segments too small.");
    runtime_ = static_cast<RT *>(*ptrOrError);
  }

  std::shared_ptr<StorageProvider> sp_;
  RT *runtime_;
};
} // namespace vm
} // namespace hermes
#endif // HERMES_VM_HEAPRUNTIME_H
