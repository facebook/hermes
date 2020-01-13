/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_UTF16STREAM_H
#define HERMES_SUPPORT_UTF16STREAM_H

#include <cstdint>

#include "hermes/ADT/OwningArray.h"
#include "llvm/ADT/ArrayRef.h"

namespace hermes {

/// A stream of char16_t that can be constructed from either UTF16 or UTF8 data.
/// If the input is UTF8, it's converted in chunks, to reduce peak memory usage.
///
/// NOTE: This class is NOT GC-aware. Don't pass in data from GC-managed objects
/// unless you can somehow guarantee the data will not move.
class UTF16Stream {
 public:
  /// A stream that simply passes through the data in \p str.
  explicit UTF16Stream(llvm::ArrayRef<char16_t> str)
      : cur_(str.begin()),
        end_(str.end()),
        utf8Begin_(nullptr),
        utf8End_(nullptr) {}

  /// A stream that converts \p utf8 to UTF16. If the input is not valid UTF8,
  /// then the stream will end at the first malformed character.
  explicit UTF16Stream(llvm::ArrayRef<uint8_t> utf8);

  /// Movable but not copyable.
  UTF16Stream(UTF16Stream &&rhs) = default;
  UTF16Stream(UTF16Stream &rhs) = delete;
  UTF16Stream &operator=(UTF16Stream &rhs) = delete;

  /// Returns whether operator*/operator++ may be called.
  /// MUST be the first method called in any stream position.
  bool hasChar() {
    return cur_ != end_ || refill();
  }

  /// Returns the UTF16 unit at the current stream position.
  /// \pre hasChar returns true.
  char16_t operator*() const {
    assert(cur_ != end_ && "must check hasChar");
    return *cur_;
  }

  /// Advances the stream to the next UTF16 unit.
  /// \pre hasChar returns true.
  UTF16Stream &operator++() {
    assert(cur_ != end_ && "must check hasChar");
    ++cur_;
    return *this;
  }

 private:
  /// Tries to convert more data. Returns true if more data was converted.
  bool refill();

  /// The range of the input (if UTF16 input) or conversion buffer (if UTF8
  /// input) that is available to be consumed.
  const char16_t *cur_;
  const char16_t *end_;

  /// nullptr if there is no more UTF8 input left to convert.
  const uint8_t *utf8Begin_;
  const uint8_t *utf8End_;

  /// The conversion buffer (if UTF8 input).
  OwningArray<char16_t> storage_;
};

} // namespace hermes

#endif // HERMES_SUPPORT_UTF16STREAM_H
