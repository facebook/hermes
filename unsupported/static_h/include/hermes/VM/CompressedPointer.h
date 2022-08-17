/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_COMPRESSEDPOINTER_H
#define HERMES_VM_COMPRESSEDPOINTER_H

#include "hermes/VM/PointerBase.h"

#include "llvh/Support/MathExtras.h"

namespace hermes {
namespace vm {

class GCCell;

class CompressedPointer {
 public:
#ifdef HERMESVM_COMPRESSED_POINTERS
  using StorageType = BasedPointer;
  using RawType = BasedPointer::StorageType;
#else
  using StorageType = GCCell *;
  using RawType = uintptr_t;
#endif

  explicit CompressedPointer() = default;
  constexpr explicit CompressedPointer(std::nullptr_t) : ptr_() {}

  static CompressedPointer fromRaw(RawType r) {
    return CompressedPointer(r);
  }

  static CompressedPointer encode(GCCell *ptr, PointerBase &base) {
    return CompressedPointer(pointerToStorageType(ptr, base));
  }
  static CompressedPointer encodeNonNull(GCCell *ptr, PointerBase &base) {
    return CompressedPointer(pointerToStorageTypeNonNull(ptr, base));
  }

  GCCell *get(PointerBase &base) const {
    return storageTypeToPointer(ptr_, base);
  }

  GCCell *getNonNull(PointerBase &base) const {
#ifdef HERMESVM_COMPRESSED_POINTERS
    return reinterpret_cast<GCCell *>(base.basedToPointerNonNull(ptr_));
#else
    (void)base;
    return ptr_;
#endif
  }

  void setInGC(CompressedPointer cp) {
    setNoBarrier(cp);
  }

  explicit operator bool() const {
    return static_cast<bool>(ptr_);
  }

  bool operator==(const CompressedPointer &other) const {
    return ptr_ == other.ptr_;
  }

  bool operator!=(const CompressedPointer &other) const {
    return !(*this == other);
  }

  RawType getRaw() const {
    return storageTypeToRaw(ptr_);
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
    ptr_ = cp.ptr_;
  }

 private:
  explicit CompressedPointer(RawType r) : ptr_(rawToStorageType(r)) {}
  explicit CompressedPointer(StorageType s) : ptr_(s) {}

#ifdef HERMESVM_COMPRESSED_POINTERS
  static BasedPointer::StorageType storageTypeToRaw(StorageType st) {
    return st.getRawValue();
  }
  static StorageType rawToStorageType(BasedPointer::StorageType raw) {
    return BasedPointer{raw};
  }
  static StorageType pointerToStorageType(GCCell *ptr, PointerBase &base) {
    return base.pointerToBased(ptr);
  }
  static StorageType pointerToStorageTypeNonNull(
      GCCell *ptr,
      PointerBase &base) {
    return base.pointerToBasedNonNull(ptr);
  }
  static GCCell *storageTypeToPointer(StorageType st, PointerBase &base) {
    return reinterpret_cast<GCCell *>(base.basedToPointer(st));
  }
#else
  static uintptr_t storageTypeToRaw(StorageType st) {
    return reinterpret_cast<uintptr_t>(st);
  }
  static StorageType rawToStorageType(uintptr_t st) {
    return reinterpret_cast<StorageType>(st);
  }
  static StorageType pointerToStorageType(GCCell *ptr, PointerBase &) {
    return ptr;
  }
  static StorageType pointerToStorageTypeNonNull(
      GCCell *ptr,
      PointerBase &base) {
    return ptr;
  }
  static GCCell *storageTypeToPointer(StorageType st, PointerBase &) {
    return st;
  }
#endif

  StorageType ptr_;
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
