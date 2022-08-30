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
#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
namespace hermes {

/// To avoid overflow, we introduce the SafeUInt32 which can be used
/// to increment a private counter with overflow checking.
/// When the user attempts to get the value out, an error will be thrown if the
/// value has overflowed.
class SafeUInt32 {
  /// The current value. Use a uint64_t to allow for overflow checking.
  uint64_t value_;
  /// The bitwise OR of the top halves of all intermediate values.
  uint32_t ors_;

 public:
  explicit SafeUInt32(uint32_t value = 0) : value_(value), ors_(0) {}

  /// \return true iff the value has overflowed 32 bits.
  bool isOverflowed() const {
    return ors_ != 0;
  }

  /// \return true iff the value is 0.
  bool isZero() const {
    return value_ == 0 && ors_ == 0;
  }

  /// \return the value of the factory.
  /// \pre the value must not have overflowed.
  uint32_t get() const {
    assert(
        !isOverflowed() &&
        "attempt to retrieve value of overflowed SafeUInt32");
    return value_;
  };

  /// Add \param n to the value of the factory.
  void add(uint32_t n) {
    value_ += n;
    ors_ |= (value_ >> 32);
  };

  uint32_t operator*() const {
    return get();
  }
};

} // namespace hermes

#pragma GCC diagnostic pop
#endif
