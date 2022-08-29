/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_STRINGVIEW_H
#define HERMES_VM_STRINGVIEW_H

#include "SmallXString.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringRefUtils.h"
#include "hermes/VM/TwineChar16.h"
#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
namespace hermes {
namespace vm {

/// StringView is a view to the string content from StringPrimitive.
/// It hides the difference between ASCII string and UTF16 string, and hence
/// allow you to iterate through a string without worrying about the type.
/// Internally, it's a char pointer and a char16 pointer (only one is valid).
///
/// Performance: Iterating from StringView is slightly slower than normal
/// iterations: every operation has one extra conditional check on the type.
/// If you are in a extremely performance sensitive setting, consider getting
/// raw pointers directly out of StringPrimitive and explicitly duplicate code
/// to handle char and char16 strings separately.
///
/// Alternatively, if you know the string is very likely to be UTF16, or the
/// string is short, consider call getUTF16Ref (which may invoke a string copy
/// if it turns out to be an ASCII string).
class StringView {
  friend class StringPrimitive;
  friend class IdentifierTable;

  union {
    /// StringView can be used to represent a view to a non-GC-managed string,
    /// a.k.a persistent identifiers whose string content is from a static
    /// memory address (either C++ literal or from a persistent bytecode module.
    const void *nonManagedStringPtr_;

    /// Handle pointing to the actual string. We need a handle to allow a
    /// StringView to survive allocations, so that we can have multiple
    /// StringViews around at the same time. Note that the StringPrimitive
    /// must have been resolved if it's a rope, i.e. we should be able to obtain
    /// a char/char16 pointer directly from str_.
    ///
    /// NOTE: we are using \c llvh::AlignedCharArrayUnion to avoid constructing
    /// the handle (which doesn't have a default constructor).
    llvh::AlignedCharArrayUnion<Handle<StringPrimitive>> strPrim_;
  };

  /// Starting index in the StringPrimitive as the beginning of this view.
  uint32_t startIndex_ : 30;

  /// Whether we are storing a handle or a non-managed pointer.
  uint32_t isHandle_ : 1;

  /// Whether the string is ASCII.
  uint32_t isASCII_ : 1;

  /// Length of the string.
  uint32_t length_;

 public:
  /// Iterator for StringView. It's mostly standard except *operator does not
  /// return a reference, which disables certain things such as creating a
  /// reverse_iterator using std::reverse_iterator.
  class const_iterator {
    friend class StringView;

    /// Current pointer position if the underlying string is char string.
    const char *charPtr_{nullptr};

    /// Current pointer position if the underlying string is char16 string.
    const char16_t *char16Ptr_{nullptr};

    const_iterator(const char *charPtr, const char16_t *char16Ptr)
        : charPtr_(charPtr), char16Ptr_(char16Ptr) {
      assert(
          ((!charPtr_) ^ (!char16Ptr_)) &&
          "Must provide one of char or char16 pointer");
    }

    explicit const_iterator(const char *ptr) : const_iterator(ptr, nullptr) {}

    explicit const_iterator(const char16_t *ptr)
        : const_iterator(nullptr, ptr) {}

   public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = char16_t;
    using pointer = char16_t *;
    using difference_type = std::ptrdiff_t;
    using reference = char16_t;

    const_iterator() = default;

    /// Allows for copying.
    const_iterator(const const_iterator &other) = default;
    const_iterator &operator=(const const_iterator &other) = default;

    const_iterator &operator++() {
      if (charPtr_) {
        ++charPtr_;
      } else {
        ++char16Ptr_;
      }
      return *this;
    }
    const_iterator &operator--() {
      if (charPtr_) {
        --charPtr_;
      } else {
        --char16Ptr_;
      }
      return *this;
    }
    const_iterator &operator+=(difference_type rhs) {
      if (charPtr_) {
        charPtr_ += rhs;
      } else {
        char16Ptr_ += rhs;
      }
      return *this;
    }
    const_iterator &operator-=(difference_type rhs) {
      if (charPtr_) {
        charPtr_ -= rhs;
      } else {
        char16Ptr_ -= rhs;
      }
      return *this;
    }
    const_iterator operator++(int) {
      const_iterator tmp(charPtr_, char16Ptr_);
      if (charPtr_) {
        ++charPtr_;
      } else {
        ++char16Ptr_;
      }
      return tmp;
    }
    const_iterator operator--(int) {
      const_iterator tmp(charPtr_, char16Ptr_);
      if (charPtr_) {
        --charPtr_;
      } else {
        --char16Ptr_;
      }
      return tmp;
    }

    difference_type operator-(const const_iterator &rhs) const {
      if (charPtr_) {
        return charPtr_ - rhs.charPtr_;
      }
      return char16Ptr_ - rhs.char16Ptr_;
    }

    const_iterator operator-(difference_type rhs) const {
      if (charPtr_) {
        return const_iterator(charPtr_ - rhs, char16Ptr_);
      }
      return const_iterator(charPtr_, char16Ptr_ - rhs);
    }
    const_iterator operator+(difference_type rhs) const {
      if (charPtr_) {
        return const_iterator(charPtr_ + rhs, char16Ptr_);
      }
      return const_iterator(charPtr_, char16Ptr_ + rhs);
    }

    /// Const dereference. Note that we cannot return a reference here (without
    /// losing efficiency, and hence making this iterator non-standard.
    char16_t operator*() const {
      return charPtr_ ? *charPtr_ : *char16Ptr_;
    }

    /// Comparisons.
    bool operator==(const const_iterator &rhs) const {
      if (charPtr_) {
        return charPtr_ == rhs.charPtr_;
      }
      return char16Ptr_ == rhs.char16Ptr_;
    }
    bool operator!=(const const_iterator &rhs) const {
      return !(*this == rhs);
    }
    bool operator>(const const_iterator &rhs) const {
      if (charPtr_) {
        return charPtr_ > rhs.charPtr_;
      }
      return char16Ptr_ > rhs.char16Ptr_;
    }
    bool operator<(const const_iterator &rhs) const {
      if (charPtr_) {
        return charPtr_ < rhs.charPtr_;
      }
      return char16Ptr_ < rhs.char16Ptr_;
    }
    bool operator>=(const const_iterator &rhs) const {
      return !(*this < rhs);
    }
    bool operator<=(const const_iterator &rhs) const {
      return !(*this > rhs);
    }
  };

  /// Reverse iterator type.
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

// In debug mode the handle is non-trivial, which makes us non-trivial too and
// we need to invoke its copy constructor and destructor.
// We could also deal with this using templates, by inheriting from a different
// base class depending on std::is_trivially_copyable<>, but the complexity is
// probably not worth it.
#ifndef NDEBUG
  StringView(const StringView &other) {
    ::memcpy(this, &other, sizeof(*this));
    if (isHandle_)
      new (strPrim_.buffer) Handle<StringPrimitive>(other.strPrim());
  }

  StringView &operator=(const StringView &other) {
    if (this != &other) {
      if (isHandle_)
        strPrim().~Handle<StringPrimitive>();
      ::memcpy(this, &other, sizeof(*this));
      if (isHandle_)
        new (strPrim_.buffer) Handle<StringPrimitive>(other.strPrim());
    }
    return *this;
  }

  ~StringView() {
    if (isHandle_)
      strPrim().~Handle<StringPrimitive>();
  }
#else
  StringView(const StringView &other) = default;
  ~StringView() = default;
#endif

  StringView(const char *ptr) : StringView(ASCIIRef(ptr, strlen(ptr))) {}

  /// \return an iterator pointing at the beginning of the string.
  const_iterator begin() const {
    if (isASCII()) {
      return const_iterator(castToCharPtr());
    }
    return const_iterator(castToChar16Ptr());
  }

  /// \return an iterator pointing at one pass the end of the string.
  const_iterator end() const {
    if (isASCII()) {
      return const_iterator(castToCharPtr() + length_);
    }
    return const_iterator(castToChar16Ptr() + length_);
  }

  /// \return a reverse iterator pointing at the end of the string.
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }

  /// \return a reverse iterator pointing at one pass the begin of the string.
  const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }

  /// \return the length.
  size_t length() const {
    return length_;
  }

  /// \return whether this string is empty.
  bool empty() const {
    return !length_;
  }

  /// \return whether this is a char string.
  bool isASCII() const {
    return isASCII_;
  }

  /// Direct indexing, \return character at \p index.
  char16_t operator[](uint32_t index) const {
    assert(index < length_ && "Out of bound indexing");
    if (isASCII()) {
      return castToCharPtr()[index];
    }
    return castToChar16Ptr()[index];
  }

  /// \return a new StringView with the string sliced from \p start with
  /// length \p length.
  StringView slice(uint32_t start, uint32_t length) const {
    assert(start + length <= length_ && "Out of bound slicing");
    auto newStringView = *this;
    newStringView.startIndex_ += start;
    newStringView.length_ = length;
    return newStringView;
  }

  /// \return a new StringView with the string sliced from \p start till
  /// the end of the string.
  StringView slice(uint32_t start) const {
    assert(start <= length_ && "Out of bound slicing");
    return slice(start, length_ - start);
  }

  /// \return a new StringView with the string sliced between [first, last).
  StringView slice(const_iterator first, const_iterator last) const {
    return slice(first - begin(), last - first);
  }

  /// \return a UTF16Ref that pointing at the beginning of the string.
  /// If the string is already UTF16, we return the pointer directly;
  /// otherwise (it's ASCII) we copy the string into the end of \p allocator,
  /// and \return a pointer to the beginning of this string in the allocator.
  /// \pre allocator must be empty when passed in.
  UTF16Ref getUTF16Ref(llvh::SmallVectorImpl<char16_t> &allocator) const {
    assert(allocator.empty() && "Shouldn't use a non-empty allocator");
    return getUTF16Ref(allocator, false);
  }

  /// Append the string into \p allocator, even though the string may already be
  /// UTF16.
  void appendUTF16String(llvh::SmallVectorImpl<char16_t> &allocator) const {
    (void)getUTF16Ref(allocator, true);
  }

  /// Assuming the StringView represents a char string, \return the pointer.
  const char *castToCharPtr() const {
    assert(isASCII() && "Cannot cast char16_t pointer to char pointer");
    if (!isHandle_) {
      return static_cast<const char *>(nonManagedStringPtr_) + startIndex_;
    }
    assert(isHandle_ && "StringView does not contain a valid string");
    return (*strPrim())->castToASCIIPointer() + startIndex_;
  }

  /// Assuming the StringView represents a char16 string, \return the pointer.
  const char16_t *castToChar16Ptr() const {
    assert(!isASCII() && "Cannot cast char pointer to char16 pointer");
    if (!isHandle_) {
      return static_cast<const char16_t *>(nonManagedStringPtr_) + startIndex_;
    }
    assert(isHandle_ && "StringView does not contain a valid string");
    return (*strPrim())->castToUTF16Pointer() + startIndex_;
  }

  /// Check if two StringViews are equal.
  bool equals(const StringView &other) const {
    if (other.isASCII()) {
      return equals(ASCIIRef(other.castToCharPtr(), other.length()));
    }
    return equals(UTF16Ref(other.castToChar16Ptr(), other.length()));
  }

  /// Check if a StringView is equal to an ArrayRef.
  template <typename T>
  bool equals(const llvh::ArrayRef<T> &other) const {
    if (isASCII()) {
      return stringRefEquals(ASCIIRef(castToCharPtr(), length()), other);
    }
    return stringRefEquals(UTF16Ref(castToChar16Ptr(), length()), other);
  }

  TwineChar16 toTwine() const {
    if (isASCII()) {
      return TwineChar16(llvh::StringRef(castToCharPtr(), length()));
    }
    return TwineChar16(UTF16Ref(castToChar16Ptr(), length()));
  }

  operator TwineChar16() const {
    return toTwine();
  }

 private:
  /// These constructors should only be called from self or from
  /// StringPrimitive.

  // Create a StringView from a StringPrimitive
  explicit StringView(Handle<StringPrimitive> str)
      : startIndex_(0),
        isHandle_(true),
        isASCII_(str->isASCII()),
        length_(str->getStringLength()) {
    new (strPrim_.buffer) Handle<StringPrimitive>(str);
  }

  /// Create a StringView from lazy identifier.
  explicit StringView(ASCIIRef asciiRef)
      : nonManagedStringPtr_(asciiRef.data()),
        startIndex_(0),
        isHandle_(false),
        isASCII_(true),
        length_(asciiRef.size()) {}
  explicit StringView(UTF16Ref utf16Ref)
      : nonManagedStringPtr_(utf16Ref.data()),
        startIndex_(0),
        isHandle_(false),
        isASCII_(false),
        length_(utf16Ref.size()) {}

  /// Helper function for getUTF16Ref and copyUTF16String.
  UTF16Ref getUTF16Ref(
      llvh::SmallVectorImpl<char16_t> &allocator,
      bool alwaysCopy) const;

  Handle<StringPrimitive> &strPrim() {
    assert(isHandle_ && "must be a handle");
    // Need to go through a variable to placate gcc4.9.
    char *buffer = strPrim_.buffer;
    return *reinterpret_cast<Handle<StringPrimitive> *>(buffer);
  }
  const Handle<StringPrimitive> &strPrim() const {
    assert(isHandle_ && "must be a handle");
    // Need to go through a variable to placate gcc4.9.
    const char *buffer = strPrim_.buffer;
    return *reinterpret_cast<const Handle<StringPrimitive> *>(buffer);
  }
};

llvh::raw_ostream &operator<<(llvh::raw_ostream &os, const StringView &sv);

} // namespace vm
} // namespace hermes

#pragma GCC diagnostic pop
#endif // HERMES_VM_STRINGVIEW_H
