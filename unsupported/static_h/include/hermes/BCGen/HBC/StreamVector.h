/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_STREAMVECTOR_H
#define HERMES_BCGEN_HBC_STREAMVECTOR_H

#include "llvh/ADT/ArrayRef.h"

namespace hermes {
namespace hbc {

/// When storing a stream of data, it is necessary to be able to construct it
/// in two different forms:
/// During serialization, we want to pass over individual data structures
/// without copying, and we cannot put them into large buffer because
/// we don't know their size and location;
/// during deserialization, we want to have the data to point to the input
/// memory buffer directly. We want to abstract out the difference and have
/// one unified interface.
template <typename T>
class StreamVector {
  StreamVector(const StreamVector &) = delete;
  void operator=(const StreamVector &) = delete;

  /// The vector is used when we need to pass over the whole array.
  std::vector<T> vec_{};

  /// The reference to the stream. Either pointing to bytesVec_
  /// or the input memory buffer directly.
  llvh::ArrayRef<T> ref_{vec_};

 public:
  /// Force the compiler to generate a move constructor.
  explicit StreamVector(StreamVector &&SV) = default;

  /// Used during deserialization.
  explicit StreamVector(const T *data, size_t size)
      : StreamVector(llvh::ArrayRef<T>(data, size)) {}

  /// Used during deserialization.
  explicit StreamVector(llvh::ArrayRef<T> ref) : ref_(ref) {}

  /// Used during serialization.
  explicit StreamVector(std::vector<T> &&data)
      : vec_(std::move(data)), ref_(vec_) {}

  explicit StreamVector() {}

  StreamVector &operator=(StreamVector &&that) = default;

  const llvh::ArrayRef<T> &getData() const {
    return ref_;
  }

  size_t size() const {
    return ref_.size();
  }

  const T &operator[](std::size_t index) const {
    assert(index < ref_.size() && "StreamVector access out of bound.");
    return ref_[index];
  }
};

} // namespace hbc
} // namespace hermes
#endif // HERMES_BCGEN_HBC_STREAMVECTOR_H
