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

#include <type_traits>

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

  /// Returns true if a string of the given \p length should be allocated as an
  /// external string, outside the JS heap. Note that some external strings may
  /// be shorter than this length.
  static bool isExternalLength(uint32_t length) {
    return length >= EXTERNAL_STRING_THRESHOLD;
  }

 public:
  /// No string with length exceeding this constant can be allocated.
  static constexpr uint32_t MAX_STRING_LENGTH = 256 * 1024 * 1024;

  // Strings whose length is at least this size are always allocated as
  // "external" strings, outside the JS heap. Note that there may be external
  // strings smaller than this length.
  static constexpr uint32_t EXTERNAL_STRING_THRESHOLD = 64 * 1024;

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
  static Handle<StringPrimitive> ensureFlat(
      Runtime *runtime,
      Handle<StringPrimitive> self) {
    // In the future, ensureFlat may trigger GC as it might allocate for
    // ropes. Move the heap here.
    runtime->potentiallyMoveHeap();
    return self;
    // TODO: Deal with different subclasses (e.g. rope)
  }

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

  /// Whether this is an external string.
  inline bool isExternal() const;

  /// Get a StringRef of T. T must be char or char16_t corresponding to whether
  /// this string is ASCII or UTF-16.
  template <typename T>
  inline ArrayRef<T> getStringRef() const;

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

 protected:
  /// Set the unique id. This should normally only be done immediately after
  /// construction. This requires and asserts that the string is uniqued.
  void updateUniqueID(SymbolID id);

  /// \return the unique id.
  /// This requires and asserts that the string is uniqued.
  SymbolID getUniqueID() const;
};

/// A subclass of StringPrimitive which stores a SymbolID.
class SymbolStringPrimitive : public StringPrimitive {
  SymbolID uniqueID_{};

  friend void symbolStringPrimitiveBuildMeta(
      const GCCell *cell,
      Metadata::Builder &mb);

 public:
  explicit SymbolStringPrimitive(
      Runtime *runtime,
      const VTable *vt,
      uint32_t cellSize,
      uint32_t length)
      : StringPrimitive(runtime, vt, cellSize, length | (1u << 31)) {}

  static bool classof(const GCCell *cell) {
    return kindInRange(
        cell->getKind(),
        CellKind::DynamicUniquedUTF16StringPrimitiveKind,
        CellKind::DynamicUniquedASCIIStringPrimitiveKind);
  }

  /// Set the unique id. This should normally only be done immediately after
  /// construction.
  void updateUniqueID(SymbolID id) {
    assert(isUniqued() && "StringPrimitive is not uniqued");
    uniqueID_ = id;
  }

  /// \return the unique id.
  SymbolID getUniqueID() const {
    assert(isUniqued() && "StringPrimitive is not uniqued");
    return uniqueID_;
  }
};

/// A helper typedef which is either SymbolStringPrimitive or StringPrimitive.
template <bool Uniqued>
using OptSymbolStringPrimitive = typename std::
    conditional<Uniqued, SymbolStringPrimitive, StringPrimitive>::type;

/// An immutable JavaScript primitive string consisting of length and
/// characters (either char or char16), optionally Uniqued.
/// The storage is allocated in and managed by the GC.
template <typename T, bool Uniqued>
class DynamicStringPrimitive final
    : public OptSymbolStringPrimitive<Uniqued>,
      private llvm::TrailingObjects<DynamicStringPrimitive<T, Uniqued>, T> {
  friend class IdentifierTable;
  friend class llvm::TrailingObjects<DynamicStringPrimitive<T, Uniqued>, T>;
  friend class StringBuilder;
  friend class StringPrimitive;
  using OptSymbolStringPrimitive<Uniqued>::isExternalLength;
  using OptSymbolStringPrimitive<Uniqued>::getStringLength;

  using Ref = llvm::ArrayRef<T>;

  /// \return the cell kind for this string.
  static constexpr CellKind getCellKind() {
    return std::is_same<T, char16_t>::value
        ? (Uniqued ? CellKind::DynamicUniquedUTF16StringPrimitiveKind
                   : CellKind::DynamicUTF16StringPrimitiveKind)
        : (Uniqued ? CellKind::DynamicUniquedASCIIStringPrimitiveKind
                   : CellKind::DynamicASCIIStringPrimitiveKind);
  }

 public:
  static bool classof(const GCCell *cell) {
    return cell->getKind() == DynamicStringPrimitive::getCellKind();
  }

 private:
  static const VTable vt;

  /// Construct from a DynamicStringPrimitive, perhaps with a SymbolID.
  /// If a non-empty SymbolID is provided, we must be a Uniqued string.
  explicit DynamicStringPrimitive(
      Runtime *runtime,
      uint32_t length,
      SymbolID id = {})
      : OptSymbolStringPrimitive<Uniqued>(
            runtime,
            &vt,
            allocationSize(length),
            length) {
    assert(!isExternalLength(length) && "length should not be external");
    if (Uniqued) {
      this->updateUniqueID(id);
    } else {
      assert(
          id == SymbolID::empty() &&
          "Non-Uniqued string passed a valid SymbolID");
    }
  }

  explicit DynamicStringPrimitive(Runtime *runtime, Ref src);

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

  /// Calculate the allocation size of a StringPrimitive given character
  /// length.
  static uint32_t allocationSize(uint32_t length) {
    return DynamicStringPrimitive::template totalSizeToAlloc<T>(length);
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

  /// \return the cell kind for this string.
  static constexpr CellKind getCellKind() {
    return std::is_same<T, char16_t>::value
        ? CellKind::ExternalUTF16StringPrimitiveKind
        : CellKind::ExternalASCIIStringPrimitiveKind;
  }

 public:
  static bool classof(const GCCell *cell) {
    return cell->getKind() == ExternalStringPrimitive::getCellKind();
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
    contents_ = static_cast<T *>(checkedMalloc2(length, sizeof(T)));
  }

  ExternalStringPrimitive(Runtime *runtime, uint32_t length, SymbolID uniqueID)
      : ExternalStringPrimitiveBase(
            runtime,
            &vt,
            sizeof(ExternalStringPrimitive<T>),
            length | (1u << 31),
            uniqueID) {
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

template <typename T, bool Uniqued>
const VTable DynamicStringPrimitive<T, Uniqued>::vt = VTable(
    DynamicStringPrimitive<T, Uniqued>::getCellKind(),
    0,
    nullptr,
    nullptr);

using DynamicUTF16StringPrimitive =
    DynamicStringPrimitive<char16_t, false /* not Uniqued */>;
using DynamicASCIIStringPrimitive =
    DynamicStringPrimitive<char, false /* not Uniqued */>;
using DynamicUniquedUTF16StringPrimitive =
    DynamicStringPrimitive<char16_t, true /* Uniqued */>;
using DynamicUniquedASCIIStringPrimitive =
    DynamicStringPrimitive<char, true /* Uniqued */>;

template <typename T>
const VTable ExternalStringPrimitive<T>::vt = VTable(
    ExternalStringPrimitive<T>::getCellKind(),
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
      return DynamicASCIIStringPrimitive::create(runtime, length);
    } else {
      return DynamicUTF16StringPrimitive::create(runtime, length);
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
    return DynamicASCIIStringPrimitive::create(runtime, str);
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
    return DynamicUTF16StringPrimitive::create(runtime, str);
  } else {
    return ExternalStringPrimitive<char16_t>::create(runtime, str);
  }
}

inline CallResult<HermesValue> StringPrimitive::createLongLived(
    Runtime *runtime,
    ASCIIRef str) {
  static_assert(
      EXTERNAL_STRING_THRESHOLD < MAX_STRING_LENGTH,
      "External string threshold should be smaller than max string size.");
  if (LLVM_LIKELY(!isExternalLength(str.size()))) {
    return DynamicASCIIStringPrimitive::createLongLived(runtime, str);
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
    return DynamicUTF16StringPrimitive::createLongLived(runtime, str);
  } else {
    return ExternalStringPrimitive<char16_t>::createLongLived(runtime, str);
  }
}

inline const char *StringPrimitive::castToASCIIPointer() const {
  if (LLVM_UNLIKELY(isExternal())) {
    return vmcast<ExternalASCIIStringPrimitive>(this)->getRawPointer();
  } else if (isUniqued()) {
    return vmcast<DynamicUniquedASCIIStringPrimitive>(this)->getRawPointer();
  } else {
    return vmcast<DynamicASCIIStringPrimitive>(this)->getRawPointer();
  }
}

inline const char16_t *StringPrimitive::castToUTF16Pointer() const {
  if (LLVM_UNLIKELY(isExternal())) {
    return vmcast<ExternalUTF16StringPrimitive>(this)->getRawPointer();
  } else if (isUniqued()) {
    return vmcast<DynamicUniquedUTF16StringPrimitive>(this)->getRawPointer();
  } else {
    return vmcast<DynamicUTF16StringPrimitive>(this)->getRawPointer();
  }
}

inline char *StringPrimitive::castToASCIIPointerForWrite() {
  if (LLVM_UNLIKELY(isExternal())) {
    return vmcast<ExternalASCIIStringPrimitive>(this)->getRawPointerForWrite();
  } else if (isUniqued()) {
    return vmcast<DynamicUniquedASCIIStringPrimitive>(this)
        ->getRawPointerForWrite();
  } else {
    return vmcast<DynamicASCIIStringPrimitive>(this)->getRawPointerForWrite();
  }
}

inline char16_t *StringPrimitive::castToUTF16PointerForWrite() {
  if (LLVM_UNLIKELY(isExternal())) {
    return vmcast<ExternalUTF16StringPrimitive>(this)->getRawPointerForWrite();
  } else if (isUniqued()) {
    return vmcast<DynamicUniquedUTF16StringPrimitive>(this)
        ->getRawPointerForWrite();
  } else {
    return vmcast<DynamicUTF16StringPrimitive>(this)->getRawPointerForWrite();
  }
}

inline SymbolID StringPrimitive::getUniqueID() const {
  assert(this->isUniqued() && "StringPrimitive is not uniqued");
  if (LLVM_LIKELY(!isExternal())) {
    return vmcast<SymbolStringPrimitive>(this)->getUniqueID();
  } else {
    return vmcast<ExternalStringPrimitiveBase>(this)->uniqueID_;
  }
}

inline void StringPrimitive::updateUniqueID(SymbolID id) {
  assert(this->isUniqued() && "String is not uniqued");
  if (LLVM_LIKELY(!isExternal())) {
    vmcast<SymbolStringPrimitive>(this)->updateUniqueID(id);
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
  //        getKind() == CellKind::DynamicUniquedASCIIStringPrimitiveKind ||
  //        getKind() == CellKind::ExternalASCIIStringPrimitiveKind;
  // We speed this up by making the assumption that the string primitive kinds
  // are defined consecutively, alternating between ASCII and UTF16.
  // We statically enforce this:
  static_assert(
      cellKindsContiguousAscending(
          CellKind::DynamicUTF16StringPrimitiveKind,
          CellKind::DynamicASCIIStringPrimitiveKind,
          CellKind::DynamicUniquedUTF16StringPrimitiveKind,
          CellKind::DynamicUniquedASCIIStringPrimitiveKind,
          CellKind::ExternalUTF16StringPrimitiveKind,
          CellKind::ExternalASCIIStringPrimitiveKind),
      "Cell kinds in unexpected order");
  // Given this assumption, the ASCII versions are either both odd or both
  // even.
  return (static_cast<uint32_t>(getKind()) & 1u) ==
      (static_cast<uint32_t>(CellKind::DynamicASCIIStringPrimitiveKind) & 1u);
}

inline bool StringPrimitive::isExternal() const {
  // We require that external cell kinds be larger than dynamic cell kinds.
  static_assert(
      cellKindsContiguousAscending(
          CellKind::DynamicUTF16StringPrimitiveKind,
          CellKind::DynamicASCIIStringPrimitiveKind,
          CellKind::DynamicUniquedUTF16StringPrimitiveKind,
          CellKind::DynamicUniquedASCIIStringPrimitiveKind,
          CellKind::ExternalUTF16StringPrimitiveKind,
          CellKind::ExternalASCIIStringPrimitiveKind),
      "Cell kinds in unexpected order");
  return getKind() >= CellKind::ExternalUTF16StringPrimitiveKind;
}

template <typename T>
inline ArrayRef<T> StringPrimitive::getStringRef() const {
  if (isExternal()) {
    return vmcast<ExternalStringPrimitive<T>>(this)->getStringRef();
  } else if (isUniqued()) {
    return vmcast<DynamicStringPrimitive<T, true /* Uniqued */>>(this)
        ->getStringRef();
  } else {
    return vmcast<DynamicStringPrimitive<T, false /* not Uniqued */>>(this)
        ->getStringRef();
  }
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
