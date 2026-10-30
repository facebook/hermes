/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_REGEXPBYTECODE_H
#define HERMES_VM_REGEXPBYTECODE_H

#include "hermes/Regex/RegexBytecode.h"

#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/IntrusiveRefCntPtr.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace hermes {
namespace vm {

/// The bytecode of one compiled regexp. It is immutable after create() and
/// shared through llvh::IntrusiveRefCntPtr. The count is atomic because the
/// last reference may be dropped by a JSRegExp finalizer on the Hades
/// background thread. The object and its bytes are one malloc.
class RegExpBytecode final
    : public llvh::ThreadSafeRefCountedBase<RegExpBytecode> {
 public:
  /// Who is responsible for reporting this object's memory.
  enum class Ownership : uint8_t {
    /// Owned by exactly one JSRegExp; that JSRegExp reports it.
    Exclusive,
    /// Held by a literal cache (RuntimeModule or SHUnitExt) and shared by
    /// any number of JSRegExps; the cache reports it.
    Cached,
  };

  /// \return a new object holding a copy of \p bytecode, which must start
  /// with a regex::RegexBytecodeHeader.
  static llvh::IntrusiveRefCntPtr<RegExpBytecode> create(
      llvh::ArrayRef<uint8_t> bytecode,
      Ownership ownership);

  /// \return the bytecode.
  llvh::ArrayRef<uint8_t> bytecode() const {
    return {bytes(), size_};
  }

  /// \return the header at the start of the bytecode.
  const regex::RegexBytecodeHeader &header() const {
    return *reinterpret_cast<const regex::RegexBytecodeHeader *>(bytes());
  }

  /// \return who reports this object's memory.
  Ownership ownership() const {
    return ownership_;
  }

  /// \return the bytes of malloc'd memory, including this header.
  size_t mallocSize() const;

  /// Free the single allocation made by create(); called through the
  /// delete in ThreadSafeRefCountedBase::Release().
  static void operator delete(void *p) {
    free(p);
  }

 private:
  /// Construct the header; create() copies the bytes after it.
  RegExpBytecode(uint32_t size, Ownership ownership)
      : size_(size), ownership_(ownership) {}

  /// \return the offset of the bytes from the start of the allocation:
  /// the object size rounded up to malloc alignment.
  static constexpr size_t bytesOffset();

  /// \return the start of the bytes.
  inline const uint8_t *bytes() const;

  /// \return the start of the bytes.
  inline uint8_t *bytes();

  /// Number of bytes of bytecode.
  uint32_t size_;

  /// Who reports this object's memory.
  Ownership ownership_;
};

constexpr size_t RegExpBytecode::bytesOffset() {
  return (sizeof(RegExpBytecode) + alignof(std::max_align_t) - 1) /
      alignof(std::max_align_t) * alignof(std::max_align_t);
}

inline const uint8_t *RegExpBytecode::bytes() const {
  return reinterpret_cast<const uint8_t *>(this) + bytesOffset();
}

inline uint8_t *RegExpBytecode::bytes() {
  return reinterpret_cast<uint8_t *>(this) + bytesOffset();
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_REGEXPBYTECODE_H
