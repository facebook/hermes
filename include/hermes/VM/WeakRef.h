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

  /// Free the underlying slot. This happens only when the WeakRef is destroyed
  /// on the mutator or in the finalizer on background thread.
  void releaseSlot() {
    slot_->free();
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

  /// Construct a WeakRef with slot pointer directly. This is only used in
  /// TransitionMap and JSWeakRef where we need to initialize a WeakRef with
  /// nullptr before creating an actual one.
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

  /// \return true if the underlying slot is null.
  bool isEmpty() const {
    return !slot_;
  }

  /// Whether the underlying slot is freed.
  bool isSlotFree() const {
    return slot_->isFree();
  }
};

/// This weak reference type is specifically for WeakMap/WeakSet. Each uniquely
/// owns a WeakMapEntrySlot, which holds both the key, the mapped value and
/// weak root to the owning map/set.
class WeakMapEntryRef {
 public:
  explicit WeakMapEntryRef(
      Runtime &runtime,
      JSObject *key,
      HermesValue value,
      JSWeakMapImplBase *ownerMapPtr)
      : slot_(
            runtime.getHeap().allocWeakMapEntrySlot(key, value, ownerMapPtr)) {}

  /// \return a pointer to the key object.
  GCCell *getKeyNoBarrierUnsafe(PointerBase &base) const {
    return slot_->key.getNoBarrierUnsafe(base);
  }

  /// \return The mapped value by the the key object.
  HermesValue getMappedValue(GC &gc) const {
    // During marking phase in Hades, mutator thread may read a mapped value A
    // and store it to a marked object B, then deletes the entry. In
    // completeMarking(), A may become unreachable and gets swept, leaving a
    // dangling reference in B. To address it, we add a read barrier on every
    // access to mapped value. Adding writer barrier should also work but this
    // is cleaner.
    gc.weakRefReadBarrier(slot_->mappedValue);
    return slot_->mappedValue;
  }

  /// Set mapped value in slot_ to \p value;
  void setMappedValue(HermesValue value) {
    slot_->mappedValue = value;
  }

  /// Methods used by the implementation of JSWeakMap.

  static WeakMapEntryRef createEmptyEntryRef() {
    return WeakMapEntryRef{reinterpret_cast<WeakMapEntrySlot *>(kEmptyKey)};
  }

  static WeakMapEntryRef createTombstoneEntryRef() {
    return WeakMapEntryRef{reinterpret_cast<WeakMapEntrySlot *>(kTombstoneKey)};
  }

  /// \return true if the WeakRoot to the key object is still alive.
  bool isKeyValid() const {
    return !!slot_->key;
  }

  /// \return true if the underlying slots are equal, or point to the same key
  /// object.
  bool isKeyEqual(const WeakMapEntryRef &other) const {
    // Comparing when the slots are equal should always succeed. This also
    // handles comparison with tombstone and empty.
    if (slot_ == other.slot_)
      return true;
    // If the slots don't match and either one is tombstone or empty,
    // the comparison has failed (so we won't dereference them below).
    if (reinterpret_cast<uintptr_t>(slot_) <= kTombstoneKey ||
        reinterpret_cast<uintptr_t>(other.slot_) <= kTombstoneKey)
      return false;

    // If either key has been garbage collected, it is impossible for them to
    // compare equal. Otherwise, check for reference equality between the keys.
    return slot_->key && other.slot_->key &&
        slot_->key.getNoBarrierUnsafe() ==
        other.slot_->key.getNoBarrierUnsafe();
  }

  /// \return true if the underlying slot points to the same key object as
  /// \p keyObject. This is only used by LookupKey in the implementation of
  // JSWeakMap, so \p keyObject should never be null.
  bool isKeyEqual(CompressedPointer keyObject) const {
    if (reinterpret_cast<uintptr_t>(slot_) <= kTombstoneKey)
      return false;
    return slot_->key.getNoBarrierUnsafe() == keyObject;
  }

  /// Free the WeakMapEntrySlot held by this reference.
  void releaseSlot() {
    slot_->free();
  }

  /// Whether the underlying slot is freed.
  bool isSlotFree() const {
    return slot_->isFree();
  }

 private:
  explicit WeakMapEntryRef(WeakMapEntrySlot *slot) : slot_(slot) {}

  WeakMapEntrySlot *slot_;

  /// Used to construct Empty/Tombstone WeakMapEntryRefs that are used in
  /// the implementation of JSWeakMap.
  static constexpr uint32_t kEmptyKey = 0;
  static constexpr uint32_t kTombstoneKey = 1;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_WEAKREF_H
