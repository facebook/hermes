/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_WEAKREF_H
#define HERMES_VM_WEAKREF_H

#include "hermes/VM/Deserializer.h"
#include "hermes/VM/GC.h"
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
    HermesValueCast<T>::assertValid(slot_->value);
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
    return slot_ == other.unsafeGetSlot();
  }

  /// \return the stored value.
  /// The weak ref may be invalid, in which case an "empty" value is returned.
  /// This is an unsafe function since the referenced object may be freed any
  /// time that GC occurs.
  OptValue<typename Traits::value_type> unsafeGetOptional() const {
    return isValid() ? Traits::decode(slot_->value) : llvm::None;
  }

  /// Before calling this function the user must check whether weak reference is
  /// valid by calling \c isValid(). The function returns the stored HermesValue
  /// and wraps it into a new handle, ensuring that it cannot be freed while the
  /// handle is alive.
  Handle<T> get(HandleRootOwner *runtime) const {
    assert(isValid() && "this WeakRef references a freed object");
    return Handle<T>::vmcast(runtime, slot_->value);
  }

  /// If the reference is valid, returns a new handle protecting the object;
  /// otherwise returns an empty OptValue.
  OptValue<Handle<T>> getOptional(HandleRootOwner *runtime) {
    return isValid() ? get(runtime) : llvm::None;
  }

  template <class U>
  static WeakRef<T> vmcast(WeakRef<U> other) {
    assert(vmisa<T>(other.unsafeGetSlot()->value) && "invalid WeakRef cast");
    return WeakRef<T>(other.unsafeGetSlot());
  }
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_WEAKREF_H
