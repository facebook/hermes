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
  using Traits = HermesValueTraits<T>;
  explicit WeakRef(GC *gc, typename Traits::value_type value)
      : WeakRefBase(gc->allocWeakSlot(Traits::encode(value))) {
    HermesValueCast<T>::assertValid(slot_->value());
  }

  explicit WeakRef(GC *gc, Handle<T> handle) : WeakRef(gc, *handle) {}

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
  OptValue<typename Traits::value_type> unsafeGetOptional(GC *gc) const {
    if (!isValid()) {
      return OptValue<typename Traits::value_type>(llvh::None);
    }

    const HermesValue value = slot_->value();
    gc->weakRefReadBarrier(value);
    return Traits::decode(value);
  }

  /// Same as \c unsafeGetOptional, but without a read barrier to the GC.
  /// Do not use this unless it is within a signal handler or in the GC itself.
  /// If you call this in normal VM operations, the pointer might be garbage
  /// collected from underneath you at some time in the future, even if it's
  /// placed in a handle.
  OptValue<typename Traits::value_type> unsafeGetOptionalNoReadBarrier() const {
    if (!isValid()) {
      return OptValue<typename Traits::value_type>(llvh::None);
    }
    return Traits::decode(slot_->value());
  }

  /// This function returns the stored HermesValue and wraps it into a new
  /// handle, ensuring that it cannot be freed while the handle is alive.
  /// If the weak reference is not live, returns None.
  llvh::Optional<Handle<T>> get(HandleRootOwner &runtime, GC *gc) const {
    if (const auto optValue = unsafeGetOptional(gc)) {
      return Handle<T>::vmcast(runtime, Traits::encode(optValue.getValue()));
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
inline typename std::enable_if<!std::is_same<T, HermesValue>::value, T *>::type
getNoHandle(const WeakRef<T> &wr, GC *gc) {
  if (const auto hv = wr.unsafeGetOptional(gc)) {
    return ::hermes::vm::vmcast_or_null<T>(hv.getValue());
  }
  return nullptr;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_WEAKREF_H
