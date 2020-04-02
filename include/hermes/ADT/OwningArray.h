/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ADT_OWNINGARRAY_H
#define HERMES_ADT_OWNINGARRAY_H

#include "llvm/ADT/ArrayRef.h"

namespace hermes {

/// An array that owns a range of memory that doesn't grow or shrink.
/// This is needed to provide a correct move constructor, due to a bug in
/// LLVM's \c OwningArrayRef move constructor.
template <typename T>
class OwningArray : public llvm::OwningArrayRef<T> {
 public:
  using llvm::OwningArrayRef<T>::OwningArrayRef;
  /// Default constructors cannot be inherited with "using".
  OwningArray() = default;
  /// NOTE: this fixes a bug in llvm's OwningArrayRef move constructor, which
  /// does copy assignment instead of move assignment.
  OwningArray(OwningArray &&Other) {
    *this = std::move(Other);
  }
  OwningArray &operator=(OwningArray &&Other) {
    llvm::OwningArrayRef<T>::operator=(std::move(Other));
    return *this;
  }
};

} // namespace hermes

#endif
