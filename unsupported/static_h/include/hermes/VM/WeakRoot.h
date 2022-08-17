/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_WEAKROOT_H
#define HERMES_VM_WEAKROOT_H

#include "hermes/VM/CompressedPointer.h"
#include "hermes/VM/GCDecl.h"

namespace hermes {
namespace vm {

/// WeakRoot is used for weak pointers that are stored in roots, and therefore
/// do not need to take up a WeakRefSlot (since we always know where to update
/// them). Use protected inheritance to avoid callers casting this to its base
/// class and accidentally missing the read barrier.
class WeakRootBase : protected CompressedPointer {
 protected:
  explicit WeakRootBase() : CompressedPointer(nullptr) {}
  explicit WeakRootBase(std::nullptr_t) : CompressedPointer(nullptr) {}
  explicit WeakRootBase(GCCell *ptr, PointerBase &base)
      : CompressedPointer(CompressedPointer::encode(ptr, base)) {}

  inline GCCell *get(PointerBase &base, GC &gc) const;
  inline GCCell *getNonNull(PointerBase &base, GC &gc) const;

 public:
  using CompressedPointer::StorageType;
  using CompressedPointer::operator bool;
  using CompressedPointer::operator!=;
  using CompressedPointer::operator==;

  /// These functions should only be used in cases where it is known that no
  /// read barrier is necessary.
  GCCell *getNoBarrierUnsafe(PointerBase &base) const {
    return CompressedPointer::get(base);
  }
  GCCell *getNonNullNoBarrierUnsafe(PointerBase &base) const {
    return CompressedPointer::getNonNull(base);
  }

  CompressedPointer getNoBarrierUnsafe() const {
    return *this;
  }

  WeakRootBase &operator=(CompressedPointer ptr) {
    // No need for a write barrier on weak roots currently.
    setNoBarrier(ptr);
    return *this;
  }

  WeakRootBase &operator=(std::nullptr_t) {
    // No need for a write barrier on weak roots currently.
    setNoBarrier(CompressedPointer{nullptr});
    return *this;
  }
};

/// A wrapper around a pointer meant to be used as a weak root. It adds a read
/// barrier so that the GC is aware when the field is read.
template <typename T>
class WeakRoot final : public WeakRootBase {
 public:
  explicit WeakRoot() : WeakRootBase() {}
  explicit WeakRoot(std::nullptr_t) : WeakRootBase(nullptr) {}
  explicit WeakRoot(T *ptr, PointerBase &base) : WeakRootBase(ptr, base) {}

  inline T *get(PointerBase &base, GC &gc) const;
  inline T *getNonNull(PointerBase &base, GC &gc) const;

  CompressedPointer getNoBarrierUnsafe() const {
    return WeakRootBase::getNoBarrierUnsafe();
  }

  T *getNoBarrierUnsafe(PointerBase &base) const {
    return static_cast<T *>(WeakRootBase::getNoBarrierUnsafe(base));
  }
  T *getNonNullNoBarrierUnsafe(PointerBase &base) const {
    return static_cast<T *>(WeakRootBase::getNonNullNoBarrierUnsafe(base));
  }

  void set(PointerBase &base, T *ptr) {
    WeakRootBase::operator=(CompressedPointer::encode(ptr, base));
  }

  WeakRoot &operator=(CompressedPointer ptr) {
    WeakRootBase::operator=(ptr);
    return *this;
  }
};

} // namespace vm
} // namespace hermes

#endif
