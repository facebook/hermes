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
#include "hermes/VM/SmallHermesValue.h"
#include "hermes/VM/SymbolID.h"

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
///
/// The read barrier needs to execute if you are potentially reading the value
/// out and storing it in a strong reference. If you are just reading it to
/// compare it to something, or if it's just being moved around, there is no
/// need for a read barrier.
///
/// The purpose of the read barrier is to inform the GC that the value stored
/// inside the WeakRoot must now be treated as if it is alive, because we might
/// store it somewhere.
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

/// A SymbolID which is weakly held and is known to the GC.
class WeakRootSymbolID final : protected SymbolID {
 public:
  constexpr WeakRootSymbolID() : SymbolID() {}

  explicit WeakRootSymbolID(SymbolID id) : SymbolID(id) {}

  using SymbolID::operator=;
  using SymbolID::operator==;
  using SymbolID::isInvalid;

  inline SymbolID get(GC &gc);
  inline SymbolID getNoBarrierUnsafe() {
    return (SymbolID)(*this);
  }
};

/// A WeakSmallHermesValue can weakly hold Pointer (i.e., String, Object,
/// BoxedDouble, BigInt) and Symbol values, or any other primitive values that a
/// SmallHermesValue can hold (e.g., Bool, small integer, etc.).
/// Each value can have four states:
/// 1. Empty, which represents the invalidated state and is the default initial
/// value. When the pointed to object or symbol is dead, the state will
/// transition back to Empty again.
/// 2. Pointer, which must not be null.
/// 3. Symbol (user should check if it's a registered symbol).
/// 4. Any other values except the above three types (e.g., Bool).
/// During young gen collection, if the value is pointer, we update the pointer
/// value if the pointed object has been evacuated, or invalidate it if it's
/// dead. During old gen collection, we invalidate it if the pointed object is
/// dead or the symbol is invalid.
class WeakSmallHermesValue final : protected SmallHermesValue {
 public:
  WeakSmallHermesValue() : SmallHermesValue(encodeEmptyValue()) {}

  explicit WeakSmallHermesValue(SmallHermesValue val) : SmallHermesValue(val) {
    assert(
        (!val.isPointer() || val.getPointer()) &&
        "val cannot be invalid pointer");
  }

  using SmallHermesValue::isObject;
  using SmallHermesValue::isPointer;
  using SmallHermesValue::isSymbol;
  using SmallHermesValue::operator=;
  using SmallHermesValue::getRaw;

  inline SymbolID getSymbol(GC &gc) const;
  SymbolID getSymbolNoBarrierUnsafe() const {
    return SmallHermesValue::getSymbol();
  }

  inline GCCell *getPointer(PointerBase &base, GC &gc) const;
  GCCell *getPointerNoBarrierUnsafe(PointerBase &base) const {
    return SmallHermesValue::getPointer(base);
  }
  CompressedPointer getPointerNoBarrierUnsafe() const {
    return SmallHermesValue::getPointer();
  }
  inline GCCell *getObject(PointerBase &base, GC &gc) const;
  GCCell *getObjectNoBarrierUnsafe(PointerBase &base) const {
    return SmallHermesValue::getObject(base);
  }
  CompressedPointer getObjectNoBarrierUnsafe() const {
    return SmallHermesValue::getObject();
  }
  inline void setObject(CompressedPointer newVal);
  inline void setObject(PointerBase &base, GCCell *ptr);
  void setSymbol(SymbolID sym) {
    assert(sym.isValid() && "Must be valid symbol");
    setNoBarrier(encodeSymbolValue(sym));
  }
  void set(SmallHermesValue newVal) {
    assert(
        (!newVal.isPointer() || newVal.getPointer()) &&
        "newVal cannot be invalid pointer");
    setNoBarrier(newVal);
  }

  /// Set the value to dead.
  void invalidate() {
    setNoBarrier(encodeEmptyValue());
  }

  /// \return True if this weak value is invalid.
  bool isInvalid() const {
    return isEmpty();
  }

  explicit operator bool() const {
    return !isInvalid();
  }
};

} // namespace vm
} // namespace hermes

#endif
