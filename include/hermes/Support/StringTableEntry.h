/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_STRINGTABLEENTRY_H
#define HERMES_SUPPORT_STRINGTABLEENTRY_H

#include "llvh/ADT/ArrayRef.h"

#include <cstdint>

namespace hermes {

/// An entry in the string table inside the ConsecutiveStringStorage.
class StringTableEntry {
  static constexpr uint32_t UTF16_MASK = 1 << 31;

  /// The offset of this string in the string storage.
  uint32_t offset_;

  /// The length of this string. We use the most significant bit to represent
  /// whether it's a UTF16 string.
  uint32_t length_;

 public:
  using StringTableRefTy = llvh::ArrayRef<StringTableEntry>;
  using MutStringTableRefTy = llvh::MutableArrayRef<StringTableEntry>;
  using StringStorageRefTy = llvh::ArrayRef<unsigned char>;

  StringTableEntry(uint32_t offset, uint32_t length, bool isUTF16)
      : offset_(offset), length_(length) {
    if (isUTF16) {
      length_ |= UTF16_MASK;
    }
  }
  /// Default constructor needed for string table resizing.
  StringTableEntry() = default;

  /// \return the offset of this entry's string in its storage.
  uint32_t getOffset() const {
    return offset_;
  }

  /// \return the length of this entry's string.
  uint32_t getLength() const {
    return length_ & (~UTF16_MASK);
  }

  /// \return whether this entry is UTF16.
  bool isUTF16() const {
    return length_ & UTF16_MASK;
  }
};

} // namespace hermes

#endif // HERMES_SUPPORT_STRINGTABLEENTRY_H
