/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_HANDLE_H
#define HERMES_VM_HANDLE_H

#include "hermes/VM/Casting.h"
#include "hermes/VM/GCBase.h"
#include "hermes/VM/GCDecl.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/HermesValueTraits.h"
#include "llvh/Support/type_traits.h"

#include <algorithm>

namespace hermes {
namespace vm {

// Forward declarations.
class GCScope;
class HandleRootOwner;
template <typename T = HermesValue>
class Handle;
template <typename T = HermesValue>
class MutableHandle;
template <typename T>
class Handle;
template <typename T>
class MutableHandle;

/// This class is used in performance-sensitive context in situations where we
/// want to encode in the function signature that allocations may be performed,
/// potentially moving the object, but we don't want to incur the cost of
/// always allocating a handle. The callee will allocate a handle internally if
/// it needs to.
///
/// For example:
/// \code
///   bool checkFlag(PseudoHandle<Foo> foo, Runtime &runtime) {
///     if (foo->cheapCheck())
///       return true;
///     auto fooHandle = runtime.makeHandle(std::move(foo));
///     return expensiveCheck(fooHandle, runtime);
///  }
/// \endcode
template <typename T = HermesValue>
class PseudoHandle {
  using traits_type = HermesValueTraits<T>;

 public:
  using value_type = typename traits_type::value_type;
  using arrow_type = typename traits_type::arrow_type;

 private:
  template <class U>
  friend class PseudoHandle;

  value_type value_;
#ifndef NDEBUG
  bool valid_{true};
#endif

  explicit PseudoHandle(value_type value) : value_(value) {}

 public:
  PseudoHandle(const PseudoHandle &) = delete;
  PseudoHandle &operator=(const PseudoHandle &) = delete;

#ifndef NDEBUG
  PseudoHandle(PseudoHandle &&hnd) : value_(hnd.value_), valid_(hnd.valid_) {
    hnd.valid_ = false;
  }
  PseudoHandle &operator=(PseudoHandle &&hnd) {
    value_ = std::move(hnd.value_);
    valid_ = hnd.valid_;
    hnd.valid_ = false;
    return *this;
  }
#else
  PseudoHandle(PseudoHandle &&) = default;
  PseudoHandle &operator=(PseudoHandle &&) = default;
#endif

  ~PseudoHandle() = default;

  constexpr PseudoHandle() : value_(traits_type::defaultValue()) {}
  PseudoHandle(Handle<T> handle) : value_(*handle) {}

  /// Conveniently construct PseudoHandle<HermesValue> from a PinnedHermesValue
  /// pointer.
  explicit PseudoHandle(PinnedHermesValue *pvalue) : value_(*pvalue) {
    static_assert(
        std::is_same<value_type, HermesValue>::value,
        "This constructor can only be used for PseudoHandle<HermesValue>");
  }

  PseudoHandle &operator=(Handle<T> handle) {
    value_ = *handle;
#ifndef NDEBUG
    valid_ = true;
#endif
    return *this;
  }

  /// Zero-cost conversion between compatible types.
  template <
      typename U,
      typename = typename std::enable_if<std::is_convertible<
          typename PseudoHandle<U>::value_type,
          typename PseudoHandle<T>::value_type>::value>::type>
  /* implicit */ PseudoHandle(PseudoHandle<U> &&other)
      : value_(static_cast<value_type>(other.get())) {
    other.invalidate();
  }

  void invalidate() {
#ifndef NDEBUG
    valid_ = false;
#endif
  }

  value_type get() const {
    assert(valid_ && "Pseudo handle has been invalidated");
    return value_;
  }

  arrow_type operator->() const {
    assert(valid_ && "Pseudo handle has been invalidated");
    return traits_type::arrow(value_);
  }

  explicit operator bool() const {
    assert(valid_ && "Pseudo handle has been invalidated");
    return value_ != nullptr;
  }

  /// \return the value encoded as HermesValue
  HermesValue getHermesValue() const {
    assert(valid_ && "Pseudo handle has been invalidated");
    return traits_type::encode(value_);
  }

  /// \return value_ directly without going through HermesValue.
  /// Used if the raw value_ was directly specified.
  /// In particular, used by CallResult to check if a value is exceptional.
  value_type unsafeGetValue() const {
    return value_;
  }

  /// Create a \c PseudoHandle from a value.
  static PseudoHandle<T> create(value_type value) {
    return PseudoHandle<T>(value);
  }

  template <typename U>
  static inline PseudoHandle vmcast(PseudoHandle<U> &&other);

  template <typename U>
  static inline PseudoHandle dyn_vmcast(PseudoHandle<U> &&other);
};

/// A HermesValue in the current GCScope which is trackable by the GC and will
/// be correctly marked and updated if objects are moved. The value is valid
/// while the owning GCScope object is alive.
/// This is a very lightweight class: copies are very cheap (just copying a
/// register); construction is also relatively cheap (in the common case a
/// comparison and increment).
/// This is the base object factoring out common code from the type-specific
/// versions.
class HandleBase {
#ifndef NDEBUG
  /// In debug mode we store the GCScope that created us so we can update its
  /// active handle count in our destructor.
  GCScope *gcScope_;
#endif

  /// Pointer to the location where the actual value is stored. That location
  /// is automatically updated by the garbage collector if necessary.
  PinnedHermesValue *handle_;

 protected:
#ifndef NDEBUG
  HandleBase &operator=(const HandleBase &other);
#else
  HandleBase &operator=(const HandleBase &other) = default;
#endif

  PinnedHermesValue *&handleRef() {
#ifdef HERMES_SLOW_DEBUG
    assert(!handle_->isInvalid() && "Reading from flushed handle");
#endif
    return handle_;
  }
  const PinnedHermesValue *handleRef() const {
#ifdef HERMES_SLOW_DEBUG
    assert(!handle_->isInvalid() && "Reading from flushed handle");
#endif
    return handle_;
  }

 public:
  /// Allocate a new handle in the current GCScope
  explicit HandleBase(
      HandleRootOwner &runtime,
      HermesValue value = HermesValue::encodeUndefinedValue());

  /// Create a Handle aliasing a non-movable HermesValue without
  /// allocating a handle.
  explicit HandleBase(const PinnedHermesValue *valueAddr)
      : handle_(const_cast<PinnedHermesValue *>(valueAddr)) {
#ifndef NDEBUG
    gcScope_ = nullptr;
#endif
  }

  /// Copy constructor.
#ifndef NDEBUG
  HandleBase(const HandleBase &sc);
#else
  HandleBase(const HandleBase &sc) = default;
#endif

#ifndef NDEBUG
  /// Move constructor.
  HandleBase(HandleBase &&sc) : handle_(sc.handle_) {
    gcScope_ = sc.gcScope_;
    sc.gcScope_ = nullptr;
  }
#endif

#ifndef NDEBUG
  ~HandleBase();
#else
  ~HandleBase() = default;
#endif

  const PinnedHermesValue *operator->() {
    return handleRef();
  }
  HermesValue operator*() {
    return *handleRef();
  }

  HermesValue getHermesValue() const {
    return *handleRef();
  }

  /// \return the underlying pointer to the PinnedHermesValue.
  /// Note that this gives up safety guarantees Handle provides,
  /// and should only be used if the Handle will be kept alive
  /// for the entire lifetime of the returned value.
  const PinnedHermesValue *unsafeGetPinnedHermesValue() const {
    // Don't call handleRef() because it creates HermesValues in debug mode.
    return handle_;
  }
} HERMES_ATTRIBUTE_WARN_UNUSED_VARIABLES;

#ifdef NDEBUG
static_assert(
    std::is_trivially_copyable<HandleBase>::value &&
        sizeof(HandleBase) == sizeof(void *),
    "Handle must fit in a register and be trivially copyable");
#endif

/// A helper type, specialized on whether we are casting to a cell or a
/// non-cell.
template <typename T, bool isCell = HermesValueTraits<T>::is_cell>
struct HermesValueCast {
  /// In debug mode, check that the value is of the correct type and assert
  /// if it isn't.
  static void assertValid(HermesValue x) {
#ifndef NDEBUG
    (void)vmcast<T>(x);
#endif
  }
};

template <typename T>
struct HermesValueCast<T, false> {
  /// In debug mode, check that the value is of the correct type and assert
  /// if it isn't.
  static void assertValid(HermesValue x) {
#ifndef NDEBUG
    (void)HermesValueTraits<T>::decode(x);
#endif
  }
};

/// A HermesValue in the current GCScope which is trackable by the GC and will
/// be correctly marked and updated if objects are moved. The value is valid
/// while the owning GCScope object is alive.
/// This is a very lightweight class: copies are very cheap (just copying a
/// register); construction is also relatively cheap (in the common case a
/// comparison and increment).
template <typename T>
class Handle : public HandleBase {
 protected:
  /// Create a Handle aliasing a non-movable HermesValue without
  /// allocating a handle.
  explicit Handle(const PinnedHermesValue *valueAddr, bool)
      : HandleBase(const_cast<PinnedHermesValue *>(valueAddr)) {}

  explicit Handle(const HandleBase &hb, bool) : HandleBase(hb) {}

  explicit Handle(HandleRootOwner &runtime, HermesValue hermesValue, bool)
      : HandleBase(runtime, hermesValue) {}

 public:
  using value_type = typename HermesValueTraits<T>::value_type;

  /// Allocate a new handle in the current GCScope
  explicit Handle(HandleRootOwner &runtime, value_type value)
      : HandleBase(runtime, HermesValueTraits<T>::encode(value)){};
  explicit Handle(GCScope *inScope, value_type value)
      : HandleBase(inScope, HermesValueTraits<T>::encode(value)){};

  /// Create a Handle aliasing a non-movable HermesValue without
  /// allocating a handle.
  explicit Handle(const PinnedHermesValue *valueAddr)
      : HandleBase(const_cast<PinnedHermesValue *>(valueAddr)) {
    static_assert(
        std::is_same<value_type, HermesValue>::value,
        "This constructor can only be used for Handle<HermesValue>");
  }

  /// Convert between compatible types.
  template <
      typename U,
      typename =
          typename std::enable_if<IsHermesValueConvertible<U, T>::value>::type>
  Handle(const Handle<U> &other) : HandleBase(other) {}

  Handle(const Handle<T> &) = default;
  Handle(Handle<T> &&) = default;

  template <
      typename U,
      typename =
          typename std::enable_if<IsHermesValueConvertible<U, T>::value>::type>
  Handle<T> &operator=(const Handle<U> &other) {
    HandleBase::operator=(other);
    return *this;
  }

#ifndef NDEBUG
  Handle<T> &operator=(const Handle<T> &other) {
    HandleBase::operator=(other);
    return *this;
  }
#else
  Handle<T> &operator=(const Handle<T> &other) = default;
#endif

  value_type get() const {
    return HermesValueTraits<T>::decode(getHermesValue());
  }

  /// \return true if it contains a non-null pointer.
  /// I couldn't find a way to disable this operator for non-pointers.
  explicit operator bool() const {
    return handleRef()->isPointer() && handleRef()->getPointer();
  }

  typename HermesValueTraits<T>::arrow_type operator->() const {
    return HermesValueTraits<T>::arrow(*handleRef());
  }

  value_type operator*() const {
    return get();
  }

  bool operator==(const Handle &value) const {
    return get() == value.get();
  }

  /// Allocate a Handle and initialize it with a HermesValue.
  /// Assert that value has the correct type.
  static Handle<T> vmcast(HandleRootOwner &runtime, HermesValue hermesValue) {
    HermesValueCast<T>::assertValid(hermesValue);
    return Handle<T>(runtime, hermesValue, true);
  }

  /// Create a Handle from aliasing a pinned HermesValue and assert that the
  /// value has the correct type.
  static Handle<T> vmcast(const PinnedHermesValue *valueAddr) {
    HermesValueCast<T>::assertValid(*valueAddr);
    return Handle<T>(valueAddr, true);
  }

  /// Create a Handle from aliasing a pinned HermesValue and assert that the
  /// value has the correct type.
  static Handle<T> vmcast_or_null(const PinnedHermesValue *valueAddr) {
    (void)hermes::vm::vmcast_or_null<T>(*valueAddr);
    return Handle<T>(valueAddr, true);
  }

  /// Cast the argument to the desired type and assert if it doesn't have the
  /// correct type.
  static Handle<T> vmcast(const HandleBase &other) {
    HermesValueCast<T>::assertValid(other.getHermesValue());
    return Handle<T>(other, true);
  }

  /// Cast the argument to the desired type and assert if it doesn't have the
  /// correct type.
  static Handle<T> vmcast_or_null(const HandleBase &other) {
    (void)hermes::vm::vmcast_or_null<T>(other.getHermesValue());
    return Handle<T>(other, true);
  }

  static Handle<T> dyn_vmcast(const HandleBase &other);

  /// Create a handle based on a potentially invalid address.
  /// For example, this is used in CallResult for exception values.
  static Handle<T> unsafeCreate(const PinnedHermesValue *valueAddr) {
    return Handle<T>(valueAddr, true);
  }
} HERMES_ATTRIBUTE_WARN_UNUSED_VARIABLES;

#ifdef NDEBUG
static_assert(
    std::is_trivially_copyable<Handle<>>::value &&
        sizeof(Handle<>) == sizeof(void *),
    "Handle must fit in a register and be trivially copyable");
#endif

/// Extend Handle to allow mutability. The vast majority of users must not
/// be able to modify the handle, but for those few that do, we need to be as
/// explicit as possible by using this class.
template <typename T>
class MutableHandle : public Handle<T> {
  MutableHandle(const MutableHandle &) = delete;
  void operator=(const MutableHandle &) = delete;

  /// Alias a MutableHandle with a non-movable HermesValue without any checks.
  explicit MutableHandle(PinnedHermesValue *valueAddr, bool dummy)
      : Handle<T>(valueAddr, dummy) {}

 public:
  using value_type = typename Handle<T>::value_type;

  /// Allocate a new handle in the current GCScope
  explicit MutableHandle(
      HandleRootOwner &runtime,
      value_type value = HermesValueTraits<T>::defaultValue())
      : Handle<T>(runtime, value) {}

  /// A move constructor.
  MutableHandle(MutableHandle &&sc) : Handle<T>(std::move(sc)) {}

  MutableHandle &operator=(MutableHandle &&sc) {
    *HandleBase::handleRef() = *sc.handleRef();
    return *this;
  }

  MutableHandle &operator=(PseudoHandle<T> &&other) {
    set(other.get());
    other.invalidate();
    return *this;
  }

  MutableHandle &operator=(value_type value) {
    set(value);
    return *this;
  }
  /// Clear the value of the handle so it doesn't prevent an object from GC.
  void clear() {
    set(HermesValueTraits<T>::defaultValue());
  }

  void set(value_type value) {
    *HandleBase::handleRef() = HermesValueTraits<T>::encode(value);
  }

  /// Create a MutableHandle from a pinned HermesValue and assert that the value
  /// has the correct type.
  static MutableHandle<T> vmcast(PinnedHermesValue *valueAddr) {
    HermesValueCast<T>::assertValid(*valueAddr);
    return MutableHandle<T>(valueAddr, true);
  }

  /// Alias a MutableHandle with a pinned HermesValue. This is only for cases
  /// where the MutableHandle<> is an output-only value, so we don't want to
  /// unnecessarily initialize it to a value compatible with its type.
  static MutableHandle<T> aliasForOutput(PinnedHermesValue *valueAddr) {
#ifdef HERMES_SLOW_DEBUG
    *valueAddr = HermesValue::encodeInvalidValue();
#endif
    return MutableHandle<T>(valueAddr, true);
  }
} HERMES_ATTRIBUTE_WARN_UNUSED_VARIABLES;

static_assert(
    sizeof(MutableHandle<>) == sizeof(HandleBase),
    "MutableHandle must be a thin wrapper on top of Handle");

/// Create a \c PseudoHandle from a pointer.
template <typename T>
inline PseudoHandle<T> createPseudoHandle(T *ptr) {
  return PseudoHandle<T>::create(ptr);
}

/// Create a \c PseudoHandle from a HermesValue.
inline PseudoHandle<> createPseudoHandle(HermesValue value) {
  return PseudoHandle<>::create(value);
}

} // namespace vm
} // namespace hermes

namespace std {
/// std::swap<MutableHandle<T>> specialization. This is needed as the default
/// implementation is not correct due MutableHandle<>::operator= definition.
template <typename T>
void swap(hermes::vm::MutableHandle<T> &a, hermes::vm::MutableHandle<T> &b) {
  typename hermes::vm::MutableHandle<T>::value_type tmp = a.get();
  a.set(b.get());
  b.set(tmp);
}
} // namespace std
/// When we updated LLVM most recently (2/2019), we had build failures.
/// Apparently, llvh::Optional<T> asks whether T "isPodLike", to
/// determine whether it can use an instantiation that invokes a copy
/// ctor of T.  isPodLike is implemented as isTriviallyCopyable.  The
/// inference by llvh::Optional is wrong, since isTriviallyCopyable<T> does
/// not imply that T has a copy ctor.  We get around this by explicitly
/// declaring that the particular instantiation that causes the failures
/// is *not* PodLike.
/// TODO(T40600161): when we next change LLVM version, see if this is
/// still necessary.
namespace llvh {

// Instantiating Optional with a T "isPodLike" will result in a specialized
// OptionalStorage class without move ctor and would only copy T. Since the
// PseudoHandle is not copyable we specialized the trait to be always false.
template <typename T>
struct isPodLike<hermes::vm::PseudoHandle<T>> {
  static const bool value = false;
};
} // namespace llvh

#endif // HERMES_VM_HANDLE_H
