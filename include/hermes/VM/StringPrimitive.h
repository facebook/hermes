/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_STRINGPRIMITIVE_H
#define HERMES_VM_STRINGPRIMITIVE_H

#include "hermes/Support/CheckedMalloc.h"
#include "hermes/Support/ErrorHandling.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCBase.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringRefUtils.h"

#include "llvm/Support/TrailingObjects.h"

namespace hermes {
namespace vm {

class StringView;

/// The base class for all types of StringPrimitives. In most of the cases,
/// use of strings should not need to worry about the precise type of
/// StringPrimitive, and use it directly in favor of the exact subclass.
class StringPrimitive : public VariableSizeRuntimeCell {
  friend class detail::IdentifierHashTable;
  friend class IdentifierTable;
  friend class StringBuilder;
  friend class StringView;

  friend llvm::raw_ostream &operator<<(
      llvm::raw_ostream &OS,
      const StringPrimitive *str);

  /// The following private overloads are to prevent creation of a
  /// StringPrimitive from a string literal. Naively allowing this would invoke
  /// ArrayRef's array template constructor, which would include the terminating
  /// NUL (as string literals are arrays that include the NUL). Use instead
  /// create() variants that are explicit about the length, e.g. ASCIIRef or
  /// UTF16Ref parameters.
  template <typename CharT, size_t N>
  static CallResult<HermesValue> create(Runtime *, const CharT (&Arr)[N]);

  /// As create(), but overloading createLongLived().
  template <typename CharT, size_t N>
  static CallResult<HermesValue> createLongLived(
      Runtime *,
      const CharT (&Arr)[N]);

 protected:
  /// Length of the string in 16-bit characters. The highest bit is set to 1
  /// if the string has been uniqued.
  uint32_t const length;

  /// Super constructor to set the length properly.
  explicit StringPrimitive(
      Runtime *runtime,
      const VTable *vt,
      uint32_t cellSize,
      uint32_t length)
      : VariableSizeRuntimeCell(&runtime->getHeap(), vt, cellSize),
        length(length) {}

 public:
  /// No string with length exceeding this constant can be allocated.
  static constexpr uint32_t MAX_STRING_LENGTH = 256 * 1024 * 1024;
  // Strings whose length is at least this size are allocated as "external"
  // strings, outside the JS heap.
  static constexpr uint32_t EXTERNAL_STRING_THRESHOLD = 64 * 1024;

  /// Returns true if a string of the given \p length should be allocated as an
  /// external string, outside the JS heap.
  static bool isExternalLength(uint32_t length) {
    return length >= EXTERNAL_STRING_THRESHOLD;
  }

  static bool classof(const GCCell *cell) {
    return kindInRange(
        cell->getKind(),
        CellKind::StringPrimitiveKind_first,
        CellKind::StringPrimitiveKind_last);
  }

  /// Proxy to {Dynamic,External}StringPrimitive<char>::create(runtime, length).
  /// The \p asciiNotUTF16 argument indicates whether the character type is char
  /// (if true) or char16_t (if false).
  static CallResult<HermesValue>
  create(Runtime *runtime, uint32_t length, bool asciiNotUTF16);

  /// Proxy to {Dynamic,External}StringPrimitive<char>::create(runtime, str).
  static CallResult<HermesValue> create(Runtime *runtime, ASCIIRef str);

  /// Proxy to {Dynamic,External}StringPrimitive<char16_t>::create(runtime,
  /// str).
  static CallResult<HermesValue> create(Runtime *runtime, UTF16Ref str);

  /// Create a StringPrimitive as efficiently as the contents of \p str allow.
  /// If it is the empty string or a single character string,
  /// return a string interned in the Runtime.
  /// If it is an ASCII string, create an ASCII string directly.
  /// Else, create a standard UTF-16 string.
  static CallResult<HermesValue> createEfficient(
      Runtime *runtime,
      UTF16Ref str);

  /// Like the above, but the created StringPrimitives will be
  /// allocated in a "long-lived" area of the heap (if the GC supports
  /// that concept).
  static CallResult<HermesValue>
  createLongLived(Runtime *runtime, uint32_t length, bool asciiNotUTF16);
  static CallResult<HermesValue> createLongLived(
      Runtime *runtime,
      ASCIIRef str);
  static CallResult<HermesValue> createLongLived(
      Runtime *runtime,
      UTF16Ref str);

  /// Copy a UTF-16 sequence into a new StringPrim without throwing.
  /// This function should only be used during VM initialization, where OOM
  /// should never happen.
  static inline Handle<StringPrimitive> createNoThrow(
      Runtime *runtime,
      UTF16Ref str);

  /// Copy a UTF-16 sequence into a new StringPrim without throwing.
  /// This function should only be used during VM initialization, where OOM
  /// should never happen.
  static inline Handle<StringPrimitive> createNoThrow(
      Runtime *runtime,
      llvm::StringRef ascii);

  /// Copy a std::u16string into a new StringPrim.
  static CallResult<HermesValue> create(
      Runtime *runtime,
      const std::u16string &str) {
    return create(runtime, UTF16Ref(str.data(), str.size()));
  }

  /// \return the length of string in 16-bit characters.
  uint32_t getStringLength() const {
    return length & ~(1u << 31);
  }

  /// \return whether the string is uniqued.
  bool isUniqued() const {
    return (length & (1u << 31)) != 0;
  }

  /// Compare a part of this string to \p other for equality.
  /// \return true if the section of this string from \p start of length \p
  /// length is equal to the string \p other.
  bool sliceEquals(
      uint32_t start,
      uint32_t length,
      const StringPrimitive *other) const;

  /// \return true if the other string is identical to this one.
  bool equals(const StringPrimitive *other) const;

  /// \return true if the other string view has identical content as self.
  bool equals(const StringView &other) const;

  /// Lexicographically compare the two strings.
  /// \return -1 if `this` is smaller, 0 if equal, +1 if `this` is greater.
  int compare(const StringPrimitive *other) const;

  /// Concatenate two StringPrimitives at \p xHandle and \p yHandle.
  /// \return pointer to a new StringPrimitive, representing the concatenation.
  static CallResult<HermesValue> concat(
      Runtime *runtime,
      Handle<StringPrimitive> xHandle,
      Handle<StringPrimitive> yHandle);

  /// Slice the StringPrimitive at \p str, \p length characters at \p start.
  /// \return new StringPrimitive, representing the sliced string.
  static CallResult<HermesValue> slice(
      Runtime *runtime,
      Handle<StringPrimitive> str,
      size_t start,
      size_t length);

  /// Flatten the string if it's a rope, possibly causing allocation/GC.
#ifndef HERMESVM_SANITIZE_HANDLES
  static Handle<StringPrimitive> ensureFlat(
      Runtime *,
      Handle<StringPrimitive> self) {
    return self;
  }
#else
  static Handle<StringPrimitive> ensureFlat(
      Runtime *runtime,
      Handle<StringPrimitive> self);
#endif

  /// \return true if the string is flat.
  bool isFlat() const {
    return true;
  }

  /// \return a StringView of this string. In the case of a rope, we will need
  /// to resolve the rope, which might involve object allocations.
  static StringView createStringView(
      Runtime *runtime,
      Handle<StringPrimitive> self);

  /// In the rare case (most likely for debugging, printing and etc), we just
  /// want to get a copy of the string in UTF16 form, without worrying about
  /// performance and efficiency. The string will be copied into \p str.
  void copyUTF16String(llvm::SmallVectorImpl<char16_t> &str) const;

  /// \return the character at \p index.
  /// Use it only when you cannot use a StringView.
  inline char16_t at(uint32_t index) const;

  /// Whether this is an ASCII string.
  inline bool isASCII() const;

#ifdef UNIT_TEST
  /// Allocate a StringPrimitive in the C heap for purposes for unit testing.
  static StringPrimitive *mallocPrimitive(UTF16Ref ref);
#endif

  /// If the given cell has is an ExternalStringPrimitive, returns the
  /// size of its associated external memory (in bytes), else zero.
  inline static uint32_t externalMemorySize(const GCCell *cell);

 private:
  /// Similar to copyUTF16String(SmallVectorImpl), copy the string into
  /// a raw pointer \p ptr. Since there is no size check, this function should
  /// only be called in rare cases carefully.
  void copyUTF16String(char16_t *ptr) const;

  /// Get a read-only raw char pointer, assert that this is ASCII string.
  const char *castToASCIIPointer() const;

  /// Get a read-only raw char16_t pointer, assert that this is UTF16 string.
  const char16_t *castToUTF16Pointer() const;

  /// Get a writable raw char pointer, assert that this is ASCII string.
  char *castToASCIIPointerForWrite();

  /// Get a writable raw char16_t pointer, assert that this is UTF16 string.
  char16_t *castToUTF16PointerForWrite();

  /// \return the UTF-16 ref starting at \p start of length \p length.
  UTF16Ref castToUTF16Ref(uint32_t start, uint32_t length) const {
    return UTF16Ref(castToUTF16Pointer() + start, length);
  }

  /// \return the ASCII ref starting at \p start of length \p length.
  ASCIIRef castToASCIIRef(uint32_t start, uint32_t length) const {
    return ASCIIRef(castToASCIIPointer() + start, length);
  }

  UTF16Ref castToUTF16Ref() const {
    return castToUTF16Ref(0, getStringLength());
  }

  ASCIIRef castToASCIIRef() const {
    return castToASCIIRef(0, getStringLength());
  }

  /// In cases when we know the String cannot be a rope (e.g. as Identifier),
  /// it is safe to call this function which guarantees to not trigger gc.
  static StringView createStringViewMustBeFlat(Handle<StringPrimitive> self);

  /// Set the unique id. This shouldn't usually be done.
  void updateUniqueID(SymbolID id);

  /// \return the unique id
  SymbolID getUniqueID() const;
};

/// A trait to map from StringPrimitive template types to CellKind.
template <bool external, typename T>
struct StringPrimTrait {};

template <>
struct StringPrimTrait</*external*/ false, char> {
  static constexpr CellKind kind = CellKind::DynamicASCIIStringPrimitiveKind;
};

template <>
struct StringPrimTrait</*external*/ false, char16_t> {
  static constexpr CellKind kind = CellKind::DynamicUTF16StringPrimitiveKind;
};

template <>
struct StringPrimTrait</*external*/ true, char> {
  static constexpr CellKind kind = CellKind::ExternalASCIIStringPrimitiveKind;
};

template <>
struct StringPrimTrait</*external*/ true, char16_t> {
  static constexpr CellKind kind = CellKind::ExternalUTF16StringPrimitiveKind;
};

/// An immutable JavaScript primitive string consisting of length and
/// characters (either char or char16).
/// The storage is allocated in and managed by the GC.
template <typename T>
class DynamicStringPrimitive final
    : public StringPrimitive,
      private llvm::TrailingObjects<DynamicStringPrimitive<T>, T, uint32_t> {
  friend class IdentifierTable;
  friend class llvm::TrailingObjects<DynamicStringPrimitive<T>, T, uint32_t>;
  friend class StringBuilder;
  friend class StringPrimitive;

  using Ref = llvm::ArrayRef<T>;

 public:
  static bool classof(const GCCell *cell) {
    return cell->getKind() == StringPrimTrait</*external*/ false, T>::kind;
  }

 protected:
  size_t numTrailingObjects(
      typename DynamicStringPrimitive::template OverloadToken<T>) const {
    return getStringLength();
  }

 private:
  static const VTable vt;

  explicit DynamicStringPrimitive(Runtime *runtime, uint32_t length)
      : StringPrimitive(runtime, &vt, allocationSize(length), length) {
    assert(!isExternalLength(length) && "length should not be external");
  }

  DynamicStringPrimitive(Runtime *runtime, uint32_t length, SymbolID uniqueID)
      : StringPrimitive(
            runtime,
            &vt,
            allocationSize(length, true),
            length | (1u << 31)) {
    assert(!isExternalLength(length) && "length should not be external");
    *(this->template getTrailingObjects<uint32_t>()) = uniqueID.unsafeGetRaw();
  }

  explicit DynamicStringPrimitive(Runtime *runtime, Ref src);

  /// Construct an object initializing it with a copy of the passed UTF16
  /// string and a specified ID.
  DynamicStringPrimitive(Runtime *runtime, Ref src, SymbolID uniqueID);

  /// Copy a UTF-16 sequence into a new StringPrim. Throw \c RangeError if the
  /// string is longer than \c MAX_STRING_LENGTH characters. The new string is
  static CallResult<HermesValue> create(Runtime *runtime, Ref str);

  /// Like the above, but the created StringPrimitive will be
  /// allocated in a "long-lived" area of the heap (if the GC supports
  /// that concept).
  static CallResult<HermesValue> createLongLived(Runtime *runtime, Ref str);

  /// Create a StringPrim object with a specified capacity \p length in
  /// 16-bit characters. Throw \c RangeError if the string is longer than
  /// \c MAX_STRING_LENGTH characters. The new string is returned in
  /// \c CallResult<HermesValue>. This should only be used by StringBuilder.
  static CallResult<HermesValue> create(Runtime *runtime, uint32_t length);

  /// Calculate the allocation size of a StringPrimitive given character length.
  static uint32_t allocationSize(uint32_t length, bool isUniqued = false) {
    return DynamicStringPrimitive::template totalSizeToAlloc<T, uint32_t>(
        length, (size_t)isUniqued);
  }

  const T *getRawPointer() const {
    return this->template getTrailingObjects<T>();
  }

  /// In rare cases it is convenient to allocate an object without populating
  /// the string contents and then initialize it dynamically. It should not
  /// normally be done, but for those rare cases, this method gives access to
  /// the writable buffer.
  T *getRawPointerForWrite() {
    return this->template getTrailingObjects<T>();
  }

  Ref getStringRef() const {
    return Ref(getRawPointer(), getStringLength());
  }
};

/// A common base class for the instantiations of ExternalStringPrimitive,
/// below, so these can access fields (currently just one) that do not depend
/// on the character type.
class ExternalStringPrimitiveBase : public StringPrimitive {
  friend class StringPrimitive;

 public:
  ExternalStringPrimitiveBase(
      Runtime *runtime,
      const VTable *vt,
      uint32_t cellSize,
      uint32_t length,
      SymbolID uniqueID = SymbolID::empty())
      : StringPrimitive(runtime, vt, cellSize, length), uniqueID_(uniqueID){};

  static bool classof(const GCCell *cell) {
    return kindInRange(
        cell->getKind(),
        CellKind::ExternalUTF16StringPrimitiveKind,
        CellKind::ExternalASCIIStringPrimitiveKind);
  }

 protected:
  SymbolID uniqueID_;
};

/// An immutable JavaScript primitive string consisting of length and a pointer
/// to characters (either char or char16).  The storage is malloced; the object
/// contains a pointer to that storage.  The object has a finalizer that
/// deallocates the storage.
/// Note: while StringPrimitive extends VariableSizeRuntimeCell, these subtypes
/// are not actually variable-sized: we indicate that they are fixed-size in the
/// metadata.
template <typename T>
class ExternalStringPrimitive final : public ExternalStringPrimitiveBase {
  friend class IdentifierTable;
  friend class StringBuilder;
  friend class StringPrimitive;

#ifdef UNIT_TEST
  // Test version needs access.
  friend class ExtStringForTest;
#endif

  using Ref = llvm::ArrayRef<T>;

 public:
  static bool classof(const GCCell *cell) {
    return cell->getKind() == StringPrimTrait</*external*/ true, T>::kind;
  }

 private:
  static const VTable vt;

  size_t getStringByteSize() const {
    return getStringLength() * sizeof(T);
  }

  ExternalStringPrimitive(Runtime *runtime, uint32_t length)
      : ExternalStringPrimitiveBase(
            runtime,
            &vt,
            sizeof(ExternalStringPrimitive<T>),
            length) {
    assert(isExternalLength(length) && "length should be external");
    contents_ = static_cast<T *>(checkedMalloc2(length, sizeof(T)));
  }

  ExternalStringPrimitive(Runtime *runtime, uint32_t length, SymbolID uniqueID)
      : ExternalStringPrimitiveBase(
            runtime,
            &vt,
            sizeof(ExternalStringPrimitive<T>),
            length | (1u << 31),
            uniqueID) {
    assert(isExternalLength(length) && "length should be external");
    contents_ = reinterpret_cast<T *>(checkedMalloc2(length, sizeof(T)));
  }

  ExternalStringPrimitive(Runtime *runtime, Ref src);

  /// Construct an object initializing it with a copy of the passed UTF16
  /// string and a specified ID.
  ExternalStringPrimitive(Runtime *runtime, Ref src, SymbolID uniqueID);

  /// Destructor deallocates the contents_ array.
  ~ExternalStringPrimitive() {
    free(contents_);
  }

  /// Copy a UTF-16 sequence into a new StringPrim. Throw \c RangeError if the
  /// string is longer than \c MAX_STRING_LENGTH characters.
  static CallResult<HermesValue> create(Runtime *runtime, Ref str);

  /// Like the above, but the created StringPrimitive will be allocated in a
  /// "long-lived" area of the heap (if the GC supports that concept).  Note
  /// that this applies only to the object proper; the contents array is
  /// allocated outside the JS heap in either case.
  static CallResult<HermesValue> createLongLived(Runtime *runtime, Ref str);

  /// Create a StringPrim object with a specified capacity \p length in
  /// 16-bit characters. Throw \c RangeError if the string is longer than
  /// \c MAX_STRING_LENGTH characters. The new string is returned in
  /// \c CallResult<HermesValue>. This should only be used by StringBuilder.
  static CallResult<HermesValue> create(Runtime *runtime, uint32_t length);

  const T *getRawPointer() const {
    return contents_;
  }

  /// In rare cases it is convenient to allocate an object without populating
  /// the string contents and then initialize it dynamically. It should not
  /// normally be done, but for those rare cases, this method gives access to
  /// the writable buffer.
  T *getRawPointerForWrite() {
    return contents_;
  }

  Ref getStringRef() const {
    return Ref(getRawPointer(), getStringLength());
  }

  // Finalizer to clean up the malloc'ed string.
  static void _finalizeImpl(GCCell *cell, GC *gc);

  /// \return the size of the external memory associated with \p cell, which is
  /// assumed to be an ExternalStringPrimitive.
  static size_t _mallocSizeImpl(GCCell *cell);

  T *contents_{nullptr};
};

template <typename T>
const VTable DynamicStringPrimitive<T>::vt =
    VTable(StringPrimTrait</*external*/ false, T>::kind, 0, nullptr, nullptr);

using DynamicUTF16StringPrimitive = DynamicStringPrimitive<char16_t>;
using DynamicASCIIStringPrimitive = DynamicStringPrimitive<char>;

template <typename T>
const VTable ExternalStringPrimitive<T>::vt = VTable(
    StringPrimTrait</*external*/ true, T>::kind,
    sizeof(ExternalStringPrimitive<T>),
    ExternalStringPrimitive<T>::_finalizeImpl,
    nullptr, // markWeak.
    ExternalStringPrimitive<T>::_mallocSizeImpl);

using ExternalUTF16StringPrimitive = ExternalStringPrimitive<char16_t>;
using ExternalASCIIStringPrimitive = ExternalStringPrimitive<char>;

//===----------------------------------------------------------------------===//
// StringPrimitive inline methods.

inline llvm::raw_ostream &operator<<(
    llvm::raw_ostream &OS,
    const StringPrimitive *str) {
  if (str->isASCII()) {
    return OS << str->castToASCIIRef();
  }
  return OS << str->castToUTF16Ref();
}

/*static*/ inline Handle<StringPrimitive> StringPrimitive::createNoThrow(
    Runtime *runtime,
    UTF16Ref str) {
  auto strRes = create(runtime, str);
  if (strRes == ExecutionStatus::EXCEPTION) {
    hermes_fatal("String allocation failed");
  }
  return runtime->makeHandle<StringPrimitive>(*strRes);
}

/*static*/ inline Handle<StringPrimitive> StringPrimitive::createNoThrow(
    Runtime *runtime,
    llvm::StringRef ascii) {
  auto strRes = create(runtime, ASCIIRef(ascii.data(), ascii.size()));
  if (strRes == ExecutionStatus::EXCEPTION) {
    hermes_fatal("String allocation failed");
  }
  return runtime->makeHandle<StringPrimitive>(*strRes);
}

inline CallResult<HermesValue>
StringPrimitive::create(Runtime *runtime, uint32_t length, bool asciiNotUTF16) {
  static_assert(
      EXTERNAL_STRING_THRESHOLD < MAX_STRING_LENGTH,
      "External string threshold should be smaller than max string size.");
  if (LLVM_LIKELY(!isExternalLength(length))) {
    if (asciiNotUTF16) {
      return DynamicStringPrimitive<char>::create(runtime, length);
    } else {
      return DynamicStringPrimitive<char16_t>::create(runtime, length);
    }
  } else {
    if (asciiNotUTF16) {
      return ExternalStringPrimitive<char>::create(runtime, length);
    } else {
      return ExternalStringPrimitive<char16_t>::create(runtime, length);
    }
  }
}

inline CallResult<HermesValue> StringPrimitive::create(
    Runtime *runtime,
    ASCIIRef str) {
  static_assert(
      EXTERNAL_STRING_THRESHOLD < MAX_STRING_LENGTH,
      "External string threshold should be smaller than max string size.");
  if (LLVM_LIKELY(!isExternalLength(str.size()))) {
    return DynamicStringPrimitive<char>::create(runtime, str);
  } else {
    return ExternalStringPrimitive<char>::create(runtime, str);
  }
}

inline CallResult<HermesValue> StringPrimitive::create(
    Runtime *runtime,
    UTF16Ref str) {
  static_assert(
      EXTERNAL_STRING_THRESHOLD < MAX_STRING_LENGTH,
      "External string threshold should be smaller than max string size.");
  if (LLVM_LIKELY(!isExternalLength(str.size()))) {
    return DynamicStringPrimitive<char16_t>::create(runtime, str);
  } else {
    return ExternalStringPrimitive<char16_t>::create(runtime, str);
  }
}

inline CallResult<HermesValue> StringPrimitive::createLongLived(
    Runtime *runtime,
    uint32_t length,
    bool asciiNotUTF16) {
  static_assert(
      EXTERNAL_STRING_THRESHOLD < MAX_STRING_LENGTH,
      "External string threshold should be smaller than max string size.");
  if (LLVM_LIKELY(!isExternalLength(length))) {
    if (asciiNotUTF16) {
      return DynamicStringPrimitive<char>::createLongLived(runtime, length);
    } else {
      return DynamicStringPrimitive<char16_t>::createLongLived(runtime, length);
    }
  } else {
    if (asciiNotUTF16) {
      return ExternalStringPrimitive<char>::createLongLived(runtime, length);
    } else {
      return ExternalStringPrimitive<char16_t>::createLongLived(
          runtime, length);
    }
  }
}

inline CallResult<HermesValue> StringPrimitive::createLongLived(
    Runtime *runtime,
    ASCIIRef str) {
  static_assert(
      EXTERNAL_STRING_THRESHOLD < MAX_STRING_LENGTH,
      "External string threshold should be smaller than max string size.");
  if (LLVM_LIKELY(!isExternalLength(str.size()))) {
    return DynamicStringPrimitive<char>::createLongLived(runtime, str);
  } else {
    return ExternalStringPrimitive<char>::createLongLived(runtime, str);
  }
}

inline CallResult<HermesValue> StringPrimitive::createLongLived(
    Runtime *runtime,
    UTF16Ref str) {
  static_assert(
      EXTERNAL_STRING_THRESHOLD < MAX_STRING_LENGTH,
      "External string threshold should be smaller than max string size.");
  if (LLVM_LIKELY(!isExternalLength(str.size()))) {
    return DynamicStringPrimitive<char16_t>::createLongLived(runtime, str);
  } else {
    return ExternalStringPrimitive<char16_t>::createLongLived(runtime, str);
  }
}

inline const char *StringPrimitive::castToASCIIPointer() const {
  if (LLVM_LIKELY(!isExternalLength(getStringLength()))) {
    return vmcast<DynamicASCIIStringPrimitive>(this)->getRawPointer();
  } else {
    return vmcast<ExternalASCIIStringPrimitive>(this)->getRawPointer();
  }
}

inline const char16_t *StringPrimitive::castToUTF16Pointer() const {
  if (LLVM_LIKELY(!isExternalLength(getStringLength()))) {
    return vmcast<DynamicUTF16StringPrimitive>(this)->getRawPointer();
  } else {
    return vmcast<ExternalUTF16StringPrimitive>(this)->getRawPointer();
  }
}

inline char *StringPrimitive::castToASCIIPointerForWrite() {
  if (LLVM_LIKELY(!isExternalLength(getStringLength()))) {
    return vmcast<DynamicASCIIStringPrimitive>(this)->getRawPointerForWrite();
  } else {
    return vmcast<ExternalASCIIStringPrimitive>(this)->getRawPointerForWrite();
  }
}

inline char16_t *StringPrimitive::castToUTF16PointerForWrite() {
  if (LLVM_LIKELY(!isExternalLength(getStringLength()))) {
    return vmcast<DynamicUTF16StringPrimitive>(this)->getRawPointerForWrite();
  } else {
    return vmcast<ExternalUTF16StringPrimitive>(this)->getRawPointerForWrite();
  }
}

inline SymbolID StringPrimitive::getUniqueID() const {
  assert(this->isUniqued() && "String is not uniqued");
  if (LLVM_LIKELY(!isExternalLength(getStringLength()))) {
    if (isASCII()) {
      return SymbolID::unsafeCreate(
          *vmcast<DynamicASCIIStringPrimitive>(this)
               ->template getTrailingObjects<uint32_t>());
    } else {
      return SymbolID::unsafeCreate(
          *vmcast<DynamicUTF16StringPrimitive>(this)
               ->template getTrailingObjects<uint32_t>());
    }
  } else {
    return vmcast<ExternalStringPrimitiveBase>(this)->uniqueID_;
  }
}

inline void StringPrimitive::updateUniqueID(SymbolID id) {
  assert(this->isUniqued() && "String is not uniqued");
  if (LLVM_LIKELY(!isExternalLength(getStringLength()))) {
    if (isASCII()) {
      *vmcast<DynamicASCIIStringPrimitive>(this)
           ->template getTrailingObjects<uint32_t>() = id.unsafeGetRaw();
    } else {
      *vmcast<DynamicUTF16StringPrimitive>(this)
           ->template getTrailingObjects<uint32_t>() = id.unsafeGetRaw();
    }
  } else {
    vmcast<ExternalStringPrimitiveBase>(this)->uniqueID_ = id;
  }
}

inline char16_t StringPrimitive::at(uint32_t index) const {
  assert(index < getStringLength() && "Index out of bound");
  if (isASCII()) {
    return *(castToASCIIPointer() + index);
  } else {
    return *(castToUTF16Pointer() + index);
  }
}

inline bool StringPrimitive::isASCII() const {
  // Abstractly, we're doing the following test:
  // return getKind() == CellKind::DynamicASCIIStringPrimitiveKind ||
  //        getKind() == CellKind::ExternalASCIIStringPrimitiveKind;
  // We speed this up by making the assumption that the string primitive kinds
  // are defined consecutively, in the order:
  // CELL_KIND(DynamicUTF16StringPrimitive, "")
  // CELL_KIND(DynamicASCIIStringPrimitive, "")
  // CELL_KIND(ExternalUTF16StringPrimitive, "")
  // CELL_KIND(ExternalASCIIStringPrimitive, "")
  // We verify this:
  static_assert(
      static_cast<unsigned>(CellKind::DynamicUTF16StringPrimitiveKind) + 1 ==
          static_cast<unsigned>(CellKind::DynamicASCIIStringPrimitiveKind),
      "kind order assumption");
  static_assert(
      static_cast<unsigned>(CellKind::DynamicASCIIStringPrimitiveKind) + 1 ==
          static_cast<unsigned>(CellKind::ExternalUTF16StringPrimitiveKind),
      "kind order assumption");
  static_assert(
      static_cast<unsigned>(CellKind::ExternalUTF16StringPrimitiveKind) + 1 ==
          static_cast<unsigned>(CellKind::ExternalASCIIStringPrimitiveKind),
      "kind order assumption");
  // Given this assumption, the ASCII versions are either both odd or both
  // even.
  return (static_cast<unsigned>(getKind()) & 0x1) ==
      (static_cast<unsigned>(CellKind::DynamicASCIIStringPrimitiveKind) & 0x1);
}

/*static*/
inline uint32_t StringPrimitive::externalMemorySize(const GCCell *cell) {
  // TODO (T27363944): a more general way of doing this, if we ever have more
  // gc kinds with external memory charges.
  if (const auto asExtAscii = dyn_vmcast<ExternalASCIIStringPrimitive>(cell)) {
    return asExtAscii->getStringByteSize();
  } else if (
      const auto asExtUTF16 = dyn_vmcast<ExternalUTF16StringPrimitive>(cell)) {
    return asExtUTF16->getStringByteSize();
  } else {
    return 0;
  }
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_STRINGPRIMITIVE_H
