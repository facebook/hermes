/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_CALLRESULT_H
#define HERMES_VM_CALLRESULT_H

#include "hermes/Support/Compiler.h"
#include "hermes/VM/Handle.h"

#include "llvh/Support/AlignOf.h"

#include <cassert>
#include <new>
#include <utility>

namespace hermes {
namespace vm {

class StringBuilder;

/// Describes the exit status of a JavaScript function: it either returned
/// normally or threw an exception.
enum class ExecutionStatus : uint32_t {
  EXCEPTION,
  RETURNED,
};

namespace detail {
enum class CallResultSpecialize {
  None,
  Bool,
  Trivial,
  Pointer,
  Handle,
  PseudoHandleHV,
  PseudoHandleCell,
};

template <typename T>
struct GetCallResultSpecialize {
  static constexpr CallResultSpecialize value = std::is_trivial<T>::value
      ? CallResultSpecialize::Trivial
      : CallResultSpecialize::None;
};

template <>
struct GetCallResultSpecialize<StringBuilder> {
  static constexpr CallResultSpecialize value = CallResultSpecialize::None;
};

template <>
struct GetCallResultSpecialize<bool> {
  static constexpr CallResultSpecialize value = CallResultSpecialize::Bool;
};

template <typename T>
struct GetCallResultSpecialize<T *> {
  static constexpr CallResultSpecialize value = CallResultSpecialize::Pointer;
};

template <typename T>
struct GetCallResultSpecialize<Handle<T>> {
  static constexpr CallResultSpecialize value = CallResultSpecialize::Handle;
};

template <>
struct GetCallResultSpecialize<PseudoHandle<HermesValue>> {
  static constexpr CallResultSpecialize value =
      CallResultSpecialize::PseudoHandleHV;
};

template <typename T>
struct GetCallResultSpecialize<PseudoHandle<T>> {
  static constexpr CallResultSpecialize value = HermesValueTraits<T>::is_cell
      ? CallResultSpecialize::PseudoHandleCell
      : CallResultSpecialize::None;
};

} // namespace detail

/// A tuple combining the result of a function which may have returned
/// successfully (ExecutionStatus::RETURNED) with a value, or thrown a VM
/// exception (ExceptionStatus::EXCEPTION).
/// This is used by some internal functions for convenience.
template <
    typename T,
    detail::CallResultSpecialize Specialize =
        detail::GetCallResultSpecialize<T>::value>
class CallResult {
  // Nontrivial implementation.
  // Trivial implementation provided by partial specialization below.
  // Here we identify a missing value as an exception.
  llvh::Optional<T> value_;

 public:
  CallResult(const CallResult &cr) = default;
  CallResult &operator=(const CallResult &cr) = default;
  CallResult(CallResult &&cr) = default;
  CallResult &operator=(CallResult &&cr) = default;

  /* implicit */ CallResult(const T &value) : value_(value) {}
  /* implicit */ CallResult(T &&value) : value_(std::move(value)) {}

  /* implicit */ CallResult(ExecutionStatus status) : value_(llvh::None) {
    assert(status != ExecutionStatus::RETURNED);
  }

  ~CallResult() = default;

  T &operator*() {
    return *value_;
  }
  const T &operator*() const {
    return *value_;
  }

  ExecutionStatus getStatus() const {
    return value_ ? ExecutionStatus::RETURNED : ExecutionStatus::EXCEPTION;
  }
  T &getValue() {
    assert(getStatus() == ExecutionStatus::RETURNED);
    return *value_;
  }
  const T &getValue() const {
    assert(getStatus() == ExecutionStatus::RETURNED);
    return *value_;
  }
  T *operator->() {
    return &getValue();
  }
  const T *operator->() const {
    return &getValue();
  }
} HERMES_ATTRIBUTE_WARN_UNUSED_RESULT_TYPE;

template <typename T>
class CallResult<T, detail::CallResultSpecialize::Trivial> {
  // Trivial implementation via partial specialization
  ExecutionStatus status_;
  T storage_;

  static_assert(std::is_trivial<T>::value, "T is not actually trivial");

  friend class CallResult<PseudoHandle<HermesValue>>;

 public:
  CallResult(const CallResult &cr) = default;
  CallResult &operator=(const CallResult &cr) {
    status_ = cr.status_;
    // GCC8 requires this no-op cast to avoid its -Wclass-memaccess warning.
    std::memcpy(reinterpret_cast<void *>(&storage_), &cr.storage_, sizeof(T));
    return *this;
  }
  ~CallResult() = default;

  /* implicit */ CallResult(T value) : status_(ExecutionStatus::RETURNED) {
    new (&storage_) T(value);
  }

  /* implicit */ CallResult(ExecutionStatus status) : status_(status) {
    assert(status != ExecutionStatus::RETURNED);
  }

  const T &operator*() const {
    return getValue();
  }

  ExecutionStatus getStatus() const {
    return status_;
  }
  const T &getValue() const {
    assert(status_ == ExecutionStatus::RETURNED);
    return *reinterpret_cast<const T *>(&storage_);
  }
  const T *operator->() const {
    return &getValue();
  }

 private:
  /// Private constructor meant to be used in converting
  /// CallResult<PseudoHandle<HermesValue>> to CallResult<HermesValue>.
  explicit CallResult(ExecutionStatus status, HermesValue value)
      : status_(status), storage_(value) {
    static_assert(
        std::is_same<T, HermesValue>::value,
        "private constructor is for constructing CallResult<HermesValue>");
  }
} HERMES_ATTRIBUTE_WARN_UNUSED_RESULT_TYPE;

/// Specialization for bool to fit in a single register.
template <>
class CallResult<bool, detail::CallResultSpecialize::Bool> {
  int status_ : 8;
  bool value_ : 1;

 public:
  CallResult(const CallResult &cr) = default;
  CallResult &operator=(const CallResult &cr) = default;
  ~CallResult() = default;

  /* implicit */ CallResult(bool value)
      : status_((int)ExecutionStatus::RETURNED), value_(value) {}

  /* implicit */ CallResult(ExecutionStatus status) : status_((int)status) {
    assert(status != ExecutionStatus::RETURNED);
  }

  bool operator*() const {
    return getValue();
  }

  ExecutionStatus getStatus() const {
    return (ExecutionStatus)status_;
  }
  bool getValue() const {
    assert(getStatus() == ExecutionStatus::RETURNED);
    return value_;
  }
} HERMES_ATTRIBUTE_WARN_UNUSED_RESULT_TYPE;

/// Specialization for pointer types.
template <typename T>
class CallResult<T *, detail::CallResultSpecialize::Pointer> {
  T *valueOrStatus_{};

 public:
  CallResult(const CallResult &cr) = default;
  CallResult &operator=(const CallResult &cr) = default;
  ~CallResult() = default;

  /* implicit */ CallResult(T *value) : valueOrStatus_(value) {}

  /* implicit */ CallResult(ExecutionStatus status)
      : valueOrStatus_(reinterpret_cast<T *>(-1)) {
    assert(status != ExecutionStatus::RETURNED);
  }

  T *&operator*() {
    return getValue();
  }
  T *operator->() {
    return getValue();
  }

  ExecutionStatus getStatus() const {
    return reinterpret_cast<intptr_t>(valueOrStatus_) == -1
        ? ExecutionStatus::EXCEPTION
        : ExecutionStatus::RETURNED;
  }
  T *&getValue() {
    assert(getStatus() == ExecutionStatus::RETURNED);
    return valueOrStatus_;
  }
} HERMES_ATTRIBUTE_WARN_UNUSED_RESULT_TYPE;

/// Specialization for Handle types.
template <typename T>
class CallResult<Handle<T>, detail::CallResultSpecialize::Handle> {
  Handle<T> valueOrStatus_;

 public:
  CallResult(const CallResult &cr) = default;
  CallResult &operator=(const CallResult &cr) = default;
  ~CallResult() = default;

  /* implicit */ CallResult(Handle<T> value) : valueOrStatus_(value) {}

  /* implicit */ CallResult(ExecutionStatus status)
      : valueOrStatus_(Handle<T>::unsafeCreate(
            reinterpret_cast<const PinnedHermesValue *>(-1))) {
    assert(status != ExecutionStatus::RETURNED);
  }

  Handle<T> &operator*() {
    return getValue();
  }
  Handle<T> *operator->() {
    assert(getStatus() == ExecutionStatus::RETURNED);
    return &getValue();
  }

  ExecutionStatus getStatus() const {
    return reinterpret_cast<intptr_t>(
               valueOrStatus_.unsafeGetPinnedHermesValue()) == -1
        ? ExecutionStatus::EXCEPTION
        : ExecutionStatus::RETURNED;
  }
  Handle<T> &getValue() {
    assert(getStatus() == ExecutionStatus::RETURNED);
    return valueOrStatus_;
  }
} HERMES_ATTRIBUTE_WARN_UNUSED_RESULT_TYPE;

/// Specialization for PseudoHandle<HermesValue>, which must use a separate
/// EXCEPTION representation, because -1 is a valid number HermesValue.
template <>
class CallResult<
    PseudoHandle<HermesValue>,
    detail::CallResultSpecialize::PseudoHandleHV> {
  // Storage method is similar to the Trivial implementation.
  ExecutionStatus status_;
  PseudoHandle<> storage_;

 public:
  CallResult(const CallResult &cr) = delete;
  CallResult &operator=(const CallResult &cr) = delete;
  CallResult(CallResult &&cr) = default;
  CallResult &operator=(CallResult &&cr) = default;
  ~CallResult() = default;

  /* implicit */ CallResult(PseudoHandle<> &&value)
      : status_(ExecutionStatus::RETURNED), storage_(std::move(value)) {}

  /* implicit */ CallResult(ExecutionStatus status) : status_(status) {
    assert(status != ExecutionStatus::RETURNED);
  }

  PseudoHandle<> &operator*() {
    return getValue();
  }
  const PseudoHandle<> &operator*() const {
    return getValue();
  }

  ExecutionStatus getStatus() const {
    return status_;
  }
  PseudoHandle<> &getValue() {
    assert(status_ == ExecutionStatus::RETURNED);
    return storage_;
  }
  const PseudoHandle<> &getValue() const {
    assert(status_ == ExecutionStatus::RETURNED);
    return storage_;
  }
  const PseudoHandle<> *operator->() const {
    return &getValue();
  }
  PseudoHandle<> *operator->() {
    return &getValue();
  }

  CallResult<HermesValue> toCallResultHermesValue() const {
    // Use a private constructor in CallResult<HermesValue> to convert
    // without having to branch.
    return CallResult<HermesValue>{status_, storage_.getHermesValue()};
  }
} HERMES_ATTRIBUTE_WARN_UNUSED_RESULT_TYPE;

/// Specialization for PseudoHandle which uses -1 as the EXCEPTION value.
template <typename T>
class CallResult<
    PseudoHandle<T>,
    detail::CallResultSpecialize::PseudoHandleCell> {
  PseudoHandle<T> valueOrStatus_;

#if defined(NDEBUG) && !defined(_WINDOWS)
  static_assert(
      std::is_trivially_copyable<PseudoHandle<T>>::value,
      "PseudoHandle<T> must be trivially copyable");
#endif

 public:
  CallResult(const CallResult &cr) = delete;
  CallResult &operator=(const CallResult &cr) = delete;
  ~CallResult() = default;

  /* implicit */ CallResult(CallResult &&value) = default;

  /* implicit */ CallResult(PseudoHandle<T> &&value)
      : valueOrStatus_(std::move(value)) {}

  template <
      typename U,
      typename = typename std::enable_if<
          std::is_convertible<PseudoHandle<U>, PseudoHandle<T>>::value>::type>
  /* implicit */ CallResult(CallResult<PseudoHandle<U>> &&other)
      : valueOrStatus_(std::move(other.unsafeGetValue())) {}

  /* implicit */ CallResult(ExecutionStatus status)
      : valueOrStatus_(PseudoHandle<T>::create(reinterpret_cast<T *>(-1))) {
    assert(status != ExecutionStatus::RETURNED);
  }

  PseudoHandle<T> &operator*() {
    return getValue();
  }
  PseudoHandle<T> *operator->() {
    assert(getStatus() == ExecutionStatus::RETURNED);
    return &valueOrStatus_;
  }

  ExecutionStatus getStatus() const {
    return reinterpret_cast<intptr_t>(valueOrStatus_.unsafeGetValue()) == -1
        ? ExecutionStatus::EXCEPTION
        : ExecutionStatus::RETURNED;
  }
  PseudoHandle<T> &getValue() {
    assert(getStatus() == ExecutionStatus::RETURNED);
    return valueOrStatus_;
  }
  PseudoHandle<T> &unsafeGetValue() {
    return valueOrStatus_;
  }
} HERMES_ATTRIBUTE_WARN_UNUSED_RESULT_TYPE;

template <typename T>
bool operator==(const CallResult<T> &lhs, ExecutionStatus rhs) {
  return lhs.getStatus() == rhs;
}
template <typename T>
bool operator!=(const CallResult<T> &lhs, ExecutionStatus rhs) {
  return lhs.getStatus() != rhs;
}
template <typename T>
bool operator==(ExecutionStatus lhs, const CallResult<T> &rhs) {
  return rhs == lhs;
}
template <typename T>
bool operator!=(ExecutionStatus lhs, const CallResult<T> &rhs) {
  return rhs != lhs;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_CALLRESULT_H
