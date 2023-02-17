/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_COMPRESSEDPOINTER_H
#define HERMES_VM_COMPRESSEDPOINTER_H

#include "hermes/VM/AlignedStorage.h"
#include "hermes/VM/sh_runtime.h"

#include "llvh/Support/MathExtras.h"

namespace hermes {
namespace vm {

class GCCell;
using PointerBase = SHRuntime;

class CompressedPointer : private SHCompressedPointer {
 public:
  using RawType = decltype(raw);

  explicit CompressedPointer() = default;
  constexpr explicit CompressedPointer(std::nullptr_t)
      : SHCompressedPointer() {}

  static CompressedPointer fromRaw(RawType r) {
    return CompressedPointer(r);
  }

  static CompressedPointer encode(GCCell *ptr, PointerBase &base) {
    return {_sh_cp_encode((SHRuntime *)&base, ptr)};
  }
  static CompressedPointer encodeNonNull(GCCell *ptr, PointerBase &base) {
    assert(ptr && "Pointer must be non-null.");
    return {_sh_cp_encode_non_null((SHRuntime *)&base, ptr)};
  }

  GCCell *get(PointerBase &base) const {
    return (GCCell *)_sh_cp_decode((SHRuntime *)&base, *this);
  }

  GCCell *getNonNull(PointerBase &base) const {
    assert(*this && "Pointer must be non-null.");
    return (GCCell *)_sh_cp_decode_non_null((SHRuntime *)&base, *this);
  }

  void setInGC(CompressedPointer cp) {
    setNoBarrier(cp);
  }

  explicit operator bool() const {
    return static_cast<bool>(raw);
  }

  bool operator==(const CompressedPointer &other) const {
    return raw == other.raw;
  }

  bool operator!=(const CompressedPointer &other) const {
    return !(*this == other);
  }

  RawType getRaw() const {
    return raw;
  }

  CompressedPointer getSegmentStart() const {
    return fromRaw(
        getRaw() & llvh::maskTrailingZeros<RawType>(AlignedStorage::kLogSize));
  }

  CompressedPointer(const CompressedPointer &) = default;

  /// We delete the assignment operator, subclasses should determine how the
  /// value is assigned and then use setNoBarrier.
  /// (except in MSVC: this is to work around lack of CWG 1734.
  /// CompressedPointer will not be considered trivial otherwise.)
  /// TODO(T40821815) Consider removing this workaround when updating MSVC
#ifndef _MSC_VER
  CompressedPointer &operator=(const CompressedPointer &) = delete;
#else
  CompressedPointer &operator=(const CompressedPointer &) = default;
#endif

 protected:
  void setNoBarrier(CompressedPointer cp) {
    raw = cp.raw;
  }

 private:
  explicit CompressedPointer(RawType r) : SHCompressedPointer{r} {}
  CompressedPointer(SHCompressedPointer c) : CompressedPointer(c.raw) {}
};

static_assert(
    std::is_trivial<CompressedPointer>::value,
    "CompressedPointer must be trivial");

class AssignableCompressedPointer : public CompressedPointer {
 public:
  explicit AssignableCompressedPointer() = default;
  AssignableCompressedPointer(const AssignableCompressedPointer &) = default;

  constexpr explicit AssignableCompressedPointer(std::nullptr_t)
      : CompressedPointer(nullptr) {}
  constexpr explicit AssignableCompressedPointer(CompressedPointer cp)
      : CompressedPointer(cp) {}

  AssignableCompressedPointer &operator=(CompressedPointer ptr) {
    setNoBarrier(ptr);
    return *this;
  }
  AssignableCompressedPointer &operator=(AssignableCompressedPointer ptr) {
    setNoBarrier(ptr);
    return *this;
  }
  AssignableCompressedPointer &operator=(std::nullptr_t) {
    setNoBarrier(CompressedPointer(nullptr));
    return *this;
  }

  using CompressedPointer::get;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_COMPRESSEDPOINTER_H
