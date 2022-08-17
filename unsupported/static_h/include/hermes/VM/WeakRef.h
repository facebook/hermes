/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_WEAKREF_H
#define HERMES_VM_WEAKREF_H

#include "hermes/VM/GC.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/Handle.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/WeakRefSlot-inline.h"

namespace hermes {
namespace vm {

class HandleRootOwner;

/// This is a concrete base of \c WeakRef<T> that can be passed to concrete
/// functions in GC.
class WeakRefBase {
 protected:
  WeakRefSlot *slot_;
  WeakRefBase(WeakRefSlot *slot) : slot_(slot) {}

 public:
  /// \return true if the referenced object hasn't been freed.
  bool isValid() const {
    return isSlotValid(slot_);
  }

  /// \return true if the given slot stores a non-empty value.
  static bool isSlotValid(const WeakRefSlot *slot) {
    assert(slot && "slot must not be null");
    return slot->hasValue();
  }

  /// \return a pointer to the slot used by this WeakRef.
  /// Used primarily when populating a DenseMap with WeakRef keys.
  WeakRefSlot *unsafeGetSlot() {
    return slot_;
  }
  const WeakRefSlot *unsafeGetSlot() const {
    return slot_;
  }
};

/// This class encapsulates a weak reference - a reference that does not cause
/// the object it points to to be retained by the GC. The weak ref is considered
/// "valid" when the stored value is not an object reference at all (e.g. it is
/// a number), or when the referenced object has not been freed yet. When the
/// object is freed by the GC, the GC updates the reference to an "empty value",
/// making it "invalid".
template <class T>
class WeakRef : public WeakRefBase {
 public:
  explicit WeakRef(Runtime &runtime, Handle<T> handle)
      : WeakRef(runtime, runtime.getHeap(), *handle) {}

  explicit WeakRef(PointerBase &base, GC &gc, T *ptr)
      : WeakRefBase(gc.allocWeakSlot(CompressedPointer::encode(ptr, base))) {}

  explicit WeakRef(PointerBase &base, GC &gc, Handle<T> handle)
      : WeakRef(base, gc, *handle) {}

  /// Used only by hash tables to allow for special WeakRef creation.
  /// In particular, this makes tombstone and empty values in the hash table.
  explicit WeakRef(WeakRefSlot *slot) : WeakRefBase(slot) {}

  /// Convert between compatible types.
  template <
      typename U,
      typename =
          typename std::enable_if<IsHermesValueConvertible<U, T>::value>::type>
  WeakRef(const WeakRef<U> &other) : WeakRefBase(other) {}

  ~WeakRef() = default;

  WeakRef(const WeakRef &) = default;
  WeakRef &operator=(const WeakRef &) = default;

  bool operator==(const WeakRef &other) const {
    return slot_ == other.slot_;
  }

  /// \return the stored value if the referent is live, otherwise nullptr.
  T *get(Runtime &runtime) const {
    if (!isValid()) {
      return nullptr;
    }
    GCCell *value = slot_->get(runtime, runtime.getHeap());
    return static_cast<T *>(value);
  }

  /// Same as \c get, but without a read barrier to the GC.
  /// Do not use this unless it is within a signal handler or in the GC itself.
  /// If you call this in normal VM operations, the pointer might be garbage
  /// collected from underneath you at some time in the future, even if it's
  /// placed in a handle.
  T *getNoBarrierUnsafe(PointerBase &base) const {
    if (!isValid()) {
      return nullptr;
    }
    return static_cast<T *>(slot_->getNoBarrierUnsafe(base));
  }

  /// Clear the slot to which the WeakRef refers.
  void clear() {
    unsafeGetSlot()->clearPointer();
  }
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_WEAKREF_H
