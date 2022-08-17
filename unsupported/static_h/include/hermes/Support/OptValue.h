/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// A simple replacement of llvh::Optional<> to be used with trivially copyable
/// types in performance-sensitive context because llvh::Optional<> is not
/// trivially copyable.
//===----------------------------------------------------------------------===//

#ifndef HERMES_SUPPORT_OPTVALUE_H
#define HERMES_SUPPORT_OPTVALUE_H

#include "hermes/Support/Compiler.h"

#include "llvh/ADT/None.h"

#include <cassert>
#include <type_traits>

namespace hermes {

/// A simple trivially copyable replacement for llvh::Optional<>. Trivially
/// copyable objects use a more efficient ABI passing convenient (can be passed
/// directly in registers) and simplify some situations involving goto-s due to
/// their trivial destructors.
template <typename T>
class OptValue {
  static_assert(
      std::is_trivially_copyable<T>::value,
      "OptValue<> can only be used with trivially copyable types");
  T value_{};
  bool hasValue_;

 public:
  typedef T value_type;

  OptValue(llvh::NoneType) : hasValue_(false) {}
  explicit OptValue() : hasValue_(false) {}
  OptValue(const T &v) : value_(v), hasValue_(true) {}

  OptValue(const OptValue &) = default;
  OptValue &operator=(const OptValue &) = default;
  ~OptValue() = default;

  bool hasValue() const {
    return hasValue_;
  }
  explicit operator bool() const {
    return hasValue_;
  }

  const T &getValue() const {
    assert(hasValue());
    return value_;
  }
  const T &operator*() const {
    return getValue();
  }

  const T *operator->() const {
    return &getValue();
  }
};
static_assert(
    std::is_trivially_copyable<OptValue<int>>::value,
    "OptValue<int> must be trivially copyable");

/// Specialization for bool that improves codegen by collapsing compares.
template <>
class OptValue<bool> {
  // -1 = none, 0 = false, 1 = true
  int value_;

 public:
  typedef bool value_type;
  OptValue(llvh::NoneType) : value_(-1) {}
  explicit OptValue() : value_(-1) {}
  OptValue(bool v) : value_(v ? 1 : 0) {}

  OptValue(const OptValue &) = default;
  OptValue &operator=(const OptValue &) = default;
  ~OptValue() = default;

  bool hasValue() const {
    return value_ >= 0;
  }
  explicit operator bool() const {
    return hasValue();
  }

  bool getValue() const {
    assert(hasValue());
    return value_ > 0;
  }

  bool operator*() const {
    return getValue();
  }
};
static_assert(
    std::is_trivially_copyable<OptValue<bool>>::value,
    "OptValue<bool> must be trivially copyable");

template <typename T, typename U>
bool operator==(const OptValue<T> &a, const OptValue<U> &b) {
  if (a && b)
    return *a == *b;
  return a.hasValue() == b.hasValue();
}

template <typename T, typename U>
bool operator!=(const OptValue<T> &a, const OptValue<U> &b) {
  return !(a == b);
}
} // namespace hermes

namespace llvh {
template <typename T>
struct isPodLike;
template <typename T>
struct isPodLike<hermes::OptValue<T>> {
  // An OptValue<T> is pod-like if T is.
  static const bool value = isPodLike<T>::value;
};
} // namespace llvh

#endif // HERMES_SUPPORT_OPTVALUE_H
