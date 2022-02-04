/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ADT_SAFEINT_H
#define HERMES_ADT_SAFEINT_H

#include <cassert>
#include <cstdint>
#include <limits>

namespace hermes {

/// To avoid overflow, we introduce the SafeUInt32 which can be used
/// to increment a private counter with overflow checking.
/// When the user attempts to get the value out, an error will be thrown if the
/// value has overflowed.
class SafeUInt32 {
  /// The current value. Use a uint64_t to allow for overflow checking.
  uint64_t value_;

 public:
  explicit SafeUInt32(uint32_t value = 0) : value_(value) {}

  /// \return true iff the value has overflowed the max string length.
  bool isOverflowed() const {
    return value_ > std::numeric_limits<uint32_t>::max();
  }

  /// \return true iff the value is 0.
  bool isZero() const {
    return value_ == 0;
  }

  /// \return the value of the factory.
  /// \pre the value must not have overflowed.
  uint32_t get() const {
    assert(
        !isOverflowed() &&
        "attempt to retrieve value of overflowed SafeUInt32");
    return value_;
  };

  /// Add \param n characters to the value of the factory.
  void add(uint32_t n) {
    value_ += n;
  };

  uint32_t operator*() const {
    return get();
  }
};

} // namespace hermes

#endif
