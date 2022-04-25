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

  /// \return the stored value.
  /// The weak ref may be invalid, in which case an "empty" value is returned.
  /// This is an unsafe function since the referenced object may be freed any
  /// time that GC occurs.
  OptValue<T *> unsafeGetOptional(Runtime &runtime) const {
    return unsafeGetOptional(runtime, runtime.getHeap());
  }

  OptValue<T *> unsafeGetOptional(PointerBase &base, GC &gc) const {
    if (!isValid()) {
      return llvh::None;
    }
    GCCell *value = slot_->value(base);
    gc.weakRefReadBarrier(value);
    return static_cast<T *>(value);
  }

  /// Same as \c unsafeGetOptional, but without a read barrier to the GC.
  /// Do not use this unless it is within a signal handler or in the GC itself.
  /// If you call this in normal VM operations, the pointer might be garbage
  /// collected from underneath you at some time in the future, even if it's
  /// placed in a handle.
  OptValue<T *> unsafeGetOptionalNoReadBarrier(PointerBase &base) const {
    if (!isValid()) {
      return llvh::None;
    }
    return static_cast<T *>(slot_->value(base));
  }

  llvh::Optional<Handle<T>> get(Runtime &runtime) const {
    if (auto optValue = unsafeGetOptional(runtime)) {
      return runtime.makeHandle<T>(optValue.getValue());
    }
    return llvh::None;
  }
  /// Clear the slot to which the WeakRef refers.
  void clear() {
    unsafeGetSlot()->clearPointer();
  }
};

/// Only enabled if T is non-HV.
/// Defined as a free function to avoid template errors.
template <typename T>
inline T *getNoHandle(const WeakRef<T> &wr, PointerBase &base, GC &gc) {
  if (auto optVal = wr.unsafeGetOptional(base, gc)) {
    return optVal.getValue();
  }
  return nullptr;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_WEAKREF_H
