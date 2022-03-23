/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_COPYABLEBASICSTRING_H
#define HERMES_VM_COPYABLEBASICSTRING_H

//===----------------------------------------------------------------------===//
// This file contains definitions that make std::basic_string<> safe to memcpy()
// by the GC. It is very VM-specific and doesn't have any usage outside. It
// basically legitimizes a hack that we should eventually work to remove.
//===----------------------------------------------------------------------===//

#include <cassert>
#include <string>

namespace hermes {
namespace vm {

// _ITERATOR_DEBUG_LEVEL enables checked iterators in Microsoft STL,
// which makes std::basic_string non-byte-copyable.
// HERMESVM_API_TRACE_ANDROID_REPLAY means that we're trying to simulate
// Android behavior on desktop linux; sizeof(std::basic_string<char>) is
// 8 bytes in an optimized 64-bit build on device, but 32 on desktop.
// So we use a pointer, which happens to be of the same size.  If
// the size were different, say 16 bytes on android, we would add
// a padding field to the CopyableBasicString class, under the
// HERMESVM_API_TRACE_ANDROID_REPLAY preprocessor variable.
#if !_ITERATOR_DEBUG_LEVEL && !defined(HERMESVM_API_TRACE_ANDROID_REPLAY)

/// Strings whose length is smaller than this should never be stored in a
/// CopyableBasicString. This is to protect against a small string optimization
/// which may use interior pointers, which would break when memcpy'd by the GC.
/// Additionally, storing strings that are too small in a CopyableBasicString
/// may be too costly, since it involves malloc() allocations and overhead from
/// the object itself.
static constexpr size_t COPYABLE_BASIC_STRING_MIN_LENGTH = 128;

/// The Hermes GC uses byte copy to move GC cells. After a cell is copied, the
/// old cell is no longer valid. This type provides the ability to store
/// std::basic_string in GC cells, which is ordinarily not possible because
/// arbitrary C++ objects cannot be safely copied with memcpy(). Whether an
/// object is safe to memcpy() depends on its precise implementation.
///
/// Depending on the implementation of std::basic_string, this type is either an
/// alias of \c std::basic_string, if std::basic_string is safe to mempcy for
/// lengths at least \c COPYABLE_BASIC_STRING_MIN_LENGTH, or a re-implementation
/// of a subset of the std::basic_string interface that is safe to copy.
///
/// The restriction for minimal length is needed to avoid small string
/// optimizations that may contain interior pointers, and also to avoid using
/// the native heap for strings that are too small (malloc() is expensive
/// compared to a GC allocation).
template <typename T>
using CopyableBasicString = std::basic_string<T>;

#else

/// Strings whose length is smaller than this should never be stored in a
/// CopyableBasicString. In other build configurations this is used to protect
/// against a small string optimization which may use interior pointers, which
/// would break when memcpy'd by the GC.
/// Additionally, storing strings that are too small in a CopyableBasicString
/// may be too costly, since it involves malloc() allocations and overhead from
/// the object itself.
static constexpr size_t COPYABLE_BASIC_STRING_MIN_LENGTH =
    sizeof(std::u16string) * 2;

/// The Hermes GC uses byte copy to move GC cells. After a cell is copied, the
/// old cell is no longer valid. This type provides the ability to store
/// std::basic_string in GC cells, which is ordinarily not possible because
/// arbitrary C++ objects cannot be safely copied with memcpy(). Whether an
/// object is safe to memcpy() depends on its precise implementation.
///
/// Depending on the implementation of std::basic_string, this type is either an
/// alias of \c std::basic_string, if std::basic_string is safe to mempcy for
/// lengths at least \c COPYABLE_BASIC_STRING_MIN_LENGTH, or a re-implementation
/// of a subset of the std::basic_string interface that is safe to copy.
///
/// The restriction for minimal length is needed to avoid small string
/// optimizations that may contain interior pointers, and also to avoid using
/// the native heap for strings that are too small (malloc() is expensive
/// compared to a GC allocation).
///
/// This specific class is a re-implementation that simply wraps a separately
/// allocated std::basic_string, which itself is not copied.
template <typename T>
class CopyableBasicString {
 public:
  using value_type = T;

  CopyableBasicString(const CopyableBasicString &) = delete;
  void operator=(const CopyableBasicString &) = delete;

  explicit CopyableBasicString() : str_(new std::basic_string<T>()) {}
  explicit CopyableBasicString(size_t count, char ch)
      : str_(new std::basic_string<T>(count, ch)) {}

  explicit CopyableBasicString(std::basic_string<T> &&o)
      : str_(new std::basic_string<T>(std::move(o))) {}

  CopyableBasicString(CopyableBasicString<T> &&o) : str_(o.str_) {
    o.str_ = nullptr;
  }

  ~CopyableBasicString() {
    delete str_;
  }

  size_t size() const {
    return get().size();
  }
  size_t capacity() const {
    return get().capacity();
  }

  void reserve(size_t new_cap) {
    get().reserve();
  }

  const T *data() const {
    return get().data();
  }

  template <class InputIt>
  CopyableBasicString<T> &append(InputIt first, InputIt last) {
    get().append(first, last);
    return *this;
  }

  T &operator[](size_t index) {
    return get()[index];
  }
  const T &operator[](size_t index) const {
    return get()[index];
  }

 private:
  /// Convenience getter of the underlying string, propagating constness.
  std::basic_string<T> &get() {
    assert(str_ && "accessing moved from string");
    return *str_;
  }
  /// Convenience getter of the underlying string, propagating constness.
  const std::basic_string<T> &get() const {
    assert(str_ && "accessing moved from string");
    return *const_cast<const std::basic_string<T> *>(str_);
  }

 private:
  /// The wrapped basic_string. Created on construction and deleted on
  /// destruction, unless it has been moved out.
  /// Moves simply copy the pointer and clear it in the source object. That
  /// could be viewed as a slight departure from the functionality of
  /// std::basic_string, but moved-from objects should not be used.
  std::basic_string<T> *str_;
};

#endif

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_COPYABLEBASICSTRING_H
