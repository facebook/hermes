/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ADT_PTRORINT_H
#define HERMES_ADT_PTRORINT_H

#include <cassert>
#include <cstdint>

namespace hermes {

/// This class encapsulates a word containing either a pointer or an integer
/// index with one fewer bits than the width of a pointer. The lowest bit
/// in the word indicates whether it is a pointer or integer: 1 means integer,
/// 0 - pointer. (Obviously the pointer needs to be aligned to something more
/// than 1).
template <class T>
class PtrOrInt {
  static_assert(
      sizeof(int) <= sizeof(intptr_t),
      "int must not be larger than intptr_t");
  T *data_;

  /// Bitcast the pointer as int for manipulation. Note that this doesn't
  /// decode a previously encoded int value.
  intptr_t intbits() const {
    return reinterpret_cast<intptr_t>(data_);
  }

  /// Encode an int into a pointer by shifting and tagging it.
  static T *encodeInt(int x) {
    return reinterpret_cast<T *>(((intptr_t)x << 1) + 1);
  }

  /// Decode a previously encoded int.
  static int decodeInt(intptr_t x) {
    return (int)(x >> 1);
  }

 public:
  PtrOrInt(T *ptr) : data_(ptr) {}

  explicit PtrOrInt(int index) : data_(encodeInt(index)) {}

  bool isPointer() const {
    return (intbits() & 1) == 0;
  }

  bool isInt() const {
    return (intbits() & 1) != 0;
  }

  T *getPointer() const {
    assert(isPointer() && "value is not a pointer");
    return data_;
  }

  int getInt() const {
    assert(isInt() && "value is not an index");
    return decodeInt(intbits());
  }

  T *operator->() const {
    return getPointer();
  }

  T &operator*() const {
    return *getPointer();
  }

  /// Obtain a modifiable reference to the pointer. We need this so we can
  /// call gc->mark()
  T *&getPointerRef() {
    assert(isPointer() && "value is not a pointer");
    return data_;
  }

  /// Assign an integer value.
  PtrOrInt<T> &operator=(int x) {
    data_ = encodeInt(x);
    return *this;
  }

  /// Assign a pointer value.
  PtrOrInt<T> &operator=(T *ptr) {
    data_ = ptr;
    return *this;
  }
};

} // namespace hermes
#endif // HERMES_ADT_PTRORINT_H
