/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_COMPRESSEDPOINTER_H
#define HERMES_VM_COMPRESSEDPOINTER_H

#include "hermes/VM/PointerBase.h"

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
  constexpr explicit CompressedPointer(StorageType ptr) : ptr_(ptr) {}
  CompressedPointer(PointerBase *base, GCCell *ptr)
      : ptr_(pointerToStorageType(ptr, base)) {}

  GCCell *get(PointerBase *base) const {
    return storageTypeToPointer(getStorageType(), base);
  }

  GCCell *getNonNull(PointerBase *base) const {
#ifdef HERMESVM_COMPRESSED_POINTERS
    return reinterpret_cast<GCCell *>(
        base->basedToPointerNonNull(getStorageType()));
#else
    (void)base;
    return getStorageType();
#endif
  }

  /// Get the underlying StorageType representation.
  StorageType getStorageType() const {
    return ptr_;
  }

  /// Get the location of the pointer. Should only be used within the
  /// implementation of garbage collection.
  StorageType &getLoc() {
    return ptr_;
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

  static GCCell *storageTypeToPointer(StorageType st, PointerBase *base) {
#ifdef HERMESVM_COMPRESSED_POINTERS
    return reinterpret_cast<GCCell *>(base->basedToPointer(st));
#else
    return st;
#endif
  }

  static StorageType pointerToStorageType(GCCell *ptr, PointerBase *base) {
#ifdef HERMESVM_COMPRESSED_POINTERS
    return base->pointerToBased(ptr);
#else
    return ptr;
#endif
  }

#ifdef HERMESVM_COMPRESSED_POINTERS
  static BasedPointer::StorageType storageTypeToRaw(StorageType st) {
    return st.getRawValue();
  }
  static StorageType rawToStorageType(BasedPointer::StorageType raw) {
    return BasedPointer{raw};
  }
  BasedPointer::StorageType getRaw() const {
    return storageTypeToRaw(ptr_);
  }
#else
  static uintptr_t storageTypeToRaw(StorageType st) {
    return reinterpret_cast<uintptr_t>(st);
  }
  static StorageType rawToStorageType(uintptr_t st) {
    return reinterpret_cast<StorageType>(st);
  }
  uintptr_t getRaw() const {
    return storageTypeToRaw(ptr_);
  }
#endif

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
