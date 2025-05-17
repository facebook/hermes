/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ADT_TRANSPARENTOWNINGPTR_H
#define HERMES_ADT_TRANSPARENTOWNINGPTR_H

#include <cassert>
#include <cstddef>
#include <memory>

/// Transparent smart pointer that takes ownership of a raw pointer.
/// The pointer is stored at offset 0, so it is accessible directly with `&`.
/// The raw pointer is freed by calling the Deleter in the destructor.
/// Useful in the JIT because we can use this class as a pointer.
template <typename T, typename Deleter = std::default_delete<T>>
class TransparentOwningPtr {
 public:
  /// The owned pointer. May be nullptr.
  T *ptr;

  /// Construct a TransparentOwningPtr that owns nothing.
  explicit TransparentOwningPtr() : ptr(nullptr) {}
  /// Construct a TransparentOwningPtr that owns \p ptr.
  /// \pre ptr must be able to be freed with free().
  explicit TransparentOwningPtr(T *ptr) : ptr(ptr) {}

  TransparentOwningPtr(const TransparentOwningPtr &) = delete;
  TransparentOwningPtr &operator=(const TransparentOwningPtr &) = delete;

  TransparentOwningPtr(TransparentOwningPtr &&) = delete;
  TransparentOwningPtr &operator=(TransparentOwningPtr &&) = delete;

  /// Replace the managed pointer with \p newPtr.
  void reset(T *newPtr) {
    if (ptr) {
      Deleter deleter;
      deleter(ptr);
    }
    ptr = newPtr;
  }

  /// Destroy this TransparentOwningPtr, freeing the owned pointer.
  ~TransparentOwningPtr() {
    if (ptr) {
      Deleter deleter;
      deleter(ptr);
    }
  }

  /// \return the owned pointer, nullptr if no pointer is owned.
  T *get() const {
    return ptr;
  }

  /// \pre ptr must not be nullptr.
  /// \return the ith element of the array starting at the owned pointer.
  T &operator[](size_t i) const {
    assert(ptr != nullptr && "TransparentOwningPtr is null");
    return ptr[i];
  }
};

#endif
