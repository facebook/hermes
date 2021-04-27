/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_WEAKREF_H
#define HERMES_VM_WEAKREF_H

#include "hermes/VM/Deserializer.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/Handle.h"

namespace hermes {
namespace vm {

class HandleRootOwner;

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
  explicit WeakRef(
      GC *gc,
      typename Traits::value_type value = Traits::defaultValue())
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

  /// This function returns the stored HermesValue and wraps it into a new
  /// handle, ensuring that it cannot be freed while the handle is alive.
  /// If the weak reference is not live, returns None.
  llvh::Optional<Handle<T>> get(HandleRootOwner *runtime, GC *gc) const {
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

class WeakRootBase {
 protected:
  explicit WeakRootBase() : ptr_() {}
  explicit WeakRootBase(std::nullptr_t) : ptr_() {}
  explicit WeakRootBase(GCPointerBase::StorageType ptr) : ptr_(ptr) {}
  explicit WeakRootBase(const WeakRootBase &that) : ptr_(that.ptr_) {}

  void *get(PointerBase *base, GC *gc) const {
    GCCell *ptr = GCPointerBase::storageTypeToPointer(ptr_, base);
    gc->weakRefReadBarrier(ptr);
    return ptr;
  }

 public:
  /// This function should only be used in cases where it is known that no read
  /// barrier is necessary.
  GCPointerBase::StorageType &getNoBarrierUnsafe() {
    return ptr_;
  }

  WeakRootBase &operator=(GCPointerBase::StorageType ptr) {
    // No need for a write barrier on weak roots currently.
    ptr_ = ptr;
    return *this;
  }

  bool operator==(GCPointerBase::StorageType that) const {
    // Checking for equality to another pointer does not change the possible
    // lifetime of the weak root, so there's no need for a read barrier
    // here.
    return ptr_ == that;
  }

  bool operator!=(GCPointerBase::StorageType that) const {
    // Checking for equality to another pointer does not change the possible
    // lifetime of the weak root, so there's no need for a read barrier
    // here.
    return !(*this == that);
  }

  explicit operator bool() const {
    // Checking for null doesn't change the lifetime of the weak root, so
    // there's no need for a read barrier here.
    // NOTE: There can be a race condition between calling this function and
    // calling get() normally, but the only time ptr_ is changed is during a
    // STW pause, so there's no way the mutator could observe it.
    return static_cast<bool>(ptr_);
  }

 private:
  GCPointerBase::StorageType ptr_;
};

/// A wrapper around a pointer meant to be used as a weak root. It adds a read
/// barrier so that the GC is aware when the field is read.
template <typename T>
class WeakRoot final : public WeakRootBase {
 public:
  explicit WeakRoot() : WeakRootBase() {}
  explicit WeakRoot(std::nullptr_t) : WeakRootBase(nullptr) {}

  T *get(PointerBase *base, GC *gc) const {
    return static_cast<T *>(WeakRootBase::get(base, gc));
  }

  void set(PointerBase *base, T *ptr) {
    WeakRootBase::operator=(GCPointerBase::pointerToStorageType(ptr, base));
  }

  WeakRoot &operator=(GCPointerBase::StorageType ptr) {
    WeakRootBase::operator=(ptr);
    return *this;
  }
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_WEAKREF_H
