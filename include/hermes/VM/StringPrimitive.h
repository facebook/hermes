/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
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

  /// Private implementation of createEfficient.
  /// Create a StringPrimitive containing the contents of \p str.
  /// If \p optStorage is provided, the implementation may acquire ownership of
  /// it.
  template <typename T>
  static CallResult<HermesValue> createEfficientImpl(
      Runtime *runtime,
      llvm::ArrayRef<T> str,
      std::basic_string<T> *optStorage = nullptr);

  /// Create a new DynamicASCIIStringPrimitive if str is all ASCII, otherwise
  /// create a new DynamicUTF16StringPrimitive.
  static CallResult<HermesValue> createDynamic(Runtime *runtime, UTF16Ref str);

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

  /// Internal helper to return a std::string from an ArrayRef.
  template <typename CharT>
  static std::basic_string<CharT> arrayToString(llvm::ArrayRef<CharT> arr) {
    return std::basic_string<CharT>{arr.begin(), arr.end()};
  }

 protected:
  /// Flag set in the \c length field to indicate that this string was
  /// "uniqued", that is, inserted into the identifier hash table and the
  /// associated SymbolID was stored in the string.
  /// Not all StringPrimitive subclasses support this and it usually happens on
  /// construction.
  /// This flag is automatically cleared by the identifier table when the
  /// associated SymbolID is garbage collected.
  static constexpr uint32_t LENGTH_FLAG_UNIQUED = uint32_t(1) << 31;

  /// Length of the string in 16-bit characters. The highest bit is set to 1
  /// if the string has been uniqued.
  uint32_t lengthAndUniquedFlag_;

  /// Super constructor to set the length properly.
  explicit StringPrimitive(
      Runtime *runtime,
      const VTable *vt,
      uint32_t cellSize,
      uint32_t length)
      : VariableSizeRuntimeCell(&runtime->getHeap(), vt, cellSize),
        lengthAndUniquedFlag_(length) {}

  /// Returns true if a string of the given \p length should be allocated as an
  /// external string, outside the JS heap. Note that some external strings may
  /// be shorter than this length.
  static bool isExternalLength(uint32_t length) {
    return length >= EXTERNAL_STRING_THRESHOLD;
  }

  /// Returns true if a string is safe to allocate externally (strings that are
  /// too small are not safe).
  static bool isSafeExternalLength(uint32_t length) {
    return length >= EXTERNAL_STRING_MIN_SIZE;
  }

 public:
  /// No string with length exceeding this constant can be allocated.
  static constexpr uint32_t MAX_STRING_LENGTH = 256 * 1024 * 1024;

  // Strings whose length is at least this size are always allocated as
  // "external" strings, outside the JS heap. Note that there may be external
  // strings smaller than this length.
  static constexpr uint32_t EXTERNAL_STRING_THRESHOLD = 64 * 1024;

  // Strings whose length is smaller than this will never be externally
  // allocated. This is to protect against a small string optimization which may
  // use interior pointers, which would break when memcpy'd by the GC.
  // This is also the size at which StringPrimitive will acquire an std::string
  // via ownership transfer. This is advantageous in that it reduces the amount
  // of copying from the malloc heap to the GC heap. However it should not be
  // too small, because the std::string itself imposes a space overhead.
  static constexpr uint32_t EXTERNAL_STRING_MIN_SIZE = 128;

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
  static CallResult<HermesValue> createEfficient(
      Runtime *runtime,
      ASCIIRef str);

  static CallResult<HermesValue> createEfficient(
      Runtime *runtime,
      UTF16Ref str);

  /// Versions of createEfficient that allow for ownership transfer of an
  /// std::string. Create a StringPrimitive from \p str. The implementation may
  /// choose to acquire ownership of \p str and use it to back the string.
  static CallResult<HermesValue> createEfficient(
      Runtime *runtime,
      std::basic_string<char> &&str);

  static CallResult<HermesValue> createEfficient(
      Runtime *runtime,
      std::basic_string<char16_t> &&str);

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

  /// \return the length of string in 16-bit characters.
  uint32_t getStringLength() const {
    return lengthAndUniquedFlag_ & ~LENGTH_FLAG_UNIQUED;
  }

  /// \return whether the string is uniqued.
  bool isUniqued() const {
    return (lengthAndUniquedFlag_ & LENGTH_FLAG_UNIQUED) != 0;
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
  ArrayRef<T> getStringRef() const {
    return ArrayRef<T>{castToPointer<T>(), getStringLength()};
  }

 private:
  /// Similar to copyUTF16String(SmallVectorImpl), copy the string into
  /// a raw pointer \p ptr. Since there is no size check, this function should
  /// only be called in rare cases carefully.
  void copyUTF16String(char16_t *ptr) const;

  /// Get a read-only raw char pointer, assert that this is ASCII string.
  const char *castToASCIIPointer() const;

  /// Get a read-only raw char16_t pointer, assert that this is UTF16 string.
  const char16_t *castToUTF16Pointer() const;

  /// Get a read-only pointer to char or char16_t. Assert this is a string of
  /// the correct type.
  template <typename T>
  inline const T *castToPointer() const;

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
  /// \return whether the StringPrimitive can be converted from non-uniqued to
  /// uniqued without reallocating.
  inline bool canBeUniqued() const;

  /// Convert a non-uniqued StringPrimitive to a unique one.
  /// \pre \c canBeUniqued() returns \c true.
  inline void convertToUniqued(SymbolID uniqueID);

  /// \return the unique id.
  /// This requires and asserts that the string is uniqued.
  SymbolID getUniqueID() const;

  /// Mark this string as not uniqued. This is used by IdentifierTable when
  /// the associated SymbolID is garbage collected.
  void clearUniquedBit() {
    lengthAndUniquedFlag_ &= ~LENGTH_FLAG_UNIQUED;
  }

  static std::string _snapshotNameImpl(GCCell *cell, GC *gc);
};

/// A subclass of StringPrimitive which stores a SymbolID.
/// Note that not all SymbolStringPrimitive are uniqued.
/// All ExternalStringPrimitives store a Symbol, but only those marked as
/// uniqued will use it.
class SymbolStringPrimitive : public StringPrimitive {
  /// The SymbolID that was assigned to this StringPrimitive when it was
  /// "uniqued" - inserted into the identifier table.
  /// This field is *only* valid if \c isUniqued() returns \c true. Otherwise
  /// its value must be ignored (even though the field itself might not be
  /// cleared).
  /// If the associated SymbolID is garbage collected, the uniqued flag will
  /// be cleared and this field becomes invalid (but won't actually be cleared,
  /// for performance reasons).
  SymbolID weakUniqueID_{};

 public:
  using StringPrimitive::StringPrimitive;

  static bool classof(const GCCell *cell) {
    static_assert(
        cellKindsContiguousAscending(
            CellKind::DynamicUniquedUTF16StringPrimitiveKind,
            CellKind::DynamicUniquedASCIIStringPrimitiveKind,
            CellKind::ExternalUTF16StringPrimitiveKind,
            CellKind::ExternalASCIIStringPrimitiveKind),
        "Unexpected CellKind ordering");
    return kindInRange(
        cell->getKind(),
        CellKind::DynamicUniquedUTF16StringPrimitiveKind,
        CellKind::ExternalASCIIStringPrimitiveKind);
  }

 protected:
  friend class StringPrimitive;

  /// Set the unique id. This should normally only be done immediately after
  /// construction.
  void updateUniqueID(SymbolID id) {
    assert(isUniqued() && "StringPrimitive is not uniqued");
    weakUniqueID_ = id;
  }

  /// \return the unique id.
  SymbolID getUniqueID() const {
    assert(isUniqued() && "StringPrimitive is not uniqued");
    return weakUniqueID_;
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
#ifdef HERMESVM_SERIALIZE
  template <typename, bool>
  friend void serializeDynamicStringImpl(Serializer &s, const GCCell *cell);

  template <typename, bool>
  friend void deserializeDynamicStringImpl(Deserializer &d);
#endif

  static bool classof(const GCCell *cell) {
    return cell->getKind() == DynamicStringPrimitive::getCellKind();
  }

 private:
  static const VTable vt;

  /// Construct from a DynamicStringPrimitive, perhaps with a SymbolID.
  /// If a non-empty SymbolID is provided, we must be a Uniqued string.
  explicit DynamicStringPrimitive(Runtime *runtime, uint32_t length)
      : OptSymbolStringPrimitive<Uniqued>(
            runtime,
            &vt,
            allocationSize(length),
            length) {
    assert(!isExternalLength(length) && "length should not be external");
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
};

/// An immutable JavaScript primitive string consisting of length and a pointer
/// to characters (either char or char16). The storage uses std::string or
/// std::u16string, and the object's finalizer deallocates the storage.
/// Note: while StringPrimitive extends VariableSizeRuntimeCell, these subtypes
/// are not actually variable-sized: we indicate that they are fixed-size in the
/// metadata.
template <typename T>
class ExternalStringPrimitive final : public SymbolStringPrimitive {
  friend class IdentifierTable;
  friend class StringBuilder;
  friend class StringPrimitive;

#ifdef UNIT_TEST
  // Test version needs access.
  friend class ExtStringForTest;
#endif

  using Ref = llvm::ArrayRef<T>;
  using StdString = std::basic_string<T>;

  /// \return the cell kind for this string.
  static constexpr CellKind getCellKind() {
    return std::is_same<T, char16_t>::value
        ? CellKind::ExternalUTF16StringPrimitiveKind
        : CellKind::ExternalASCIIStringPrimitiveKind;
  }

 public:
#ifdef HERMESVM_SERIALIZE
  template <typename>
  friend void serializeExternalStringImpl(Serializer &s, const GCCell *cell);

  template <typename>
  friend void deserializeExternalStringImpl(Deserializer &d);
#endif

  static bool classof(const GCCell *cell) {
    return cell->getKind() == ExternalStringPrimitive::getCellKind();
  }

 private:
  static const VTable vt;

  size_t calcExternalMemorySize() const {
    return contents_.capacity() * sizeof(T);
  }

  /// Construct an ExternalStringPrimitive from the given string \p contents,
  /// non-uniqued.
  ExternalStringPrimitive(Runtime *runtime, StdString &&contents);

  /// Destructor deallocates the contents_ string.
  ~ExternalStringPrimitive() = default;

  /// Transfer ownership of an std::string into a new StringPrim. Throw \c
  /// RangeError if the string is longer than \c MAX_STRING_LENGTH characters.
  static CallResult<HermesValue> create(Runtime *runtime, StdString &&str);

  /// Like the above, but the created StringPrimitive will be allocated in a
  /// "long-lived" area of the heap (if the GC supports that concept).  Note
  /// that this applies only to the object proper; the contents array is
  /// allocated outside the JS heap in either case.
  static CallResult<HermesValue> createLongLived(
      Runtime *runtime,
      StdString &&str);

  /// Create a StringPrim object with a specified capacity \p length in
  /// 16-bit characters. Throw \c RangeError if the string is longer than
  /// \c MAX_STRING_LENGTH characters. The new string is returned in
  /// \c CallResult<HermesValue>. This should only be used by StringBuilder.
  static CallResult<HermesValue> create(Runtime *runtime, uint32_t length);

  const T *getRawPointer() const {
    // C++11 defines this to be valid even if the string is empty.
    return &contents_[0];
  }

  /// In rare cases it is convenient to allocate an object without populating
  /// the string contents and then initialize it dynamically. It should not
  /// normally be done, but for those rare cases, this method gives access to
  /// the writable buffer.
  T *getRawPointerForWrite() {
    // C++11 defines this to be valid even if the string is empty.
    return &contents_[0];
  }

  // Finalizer to clean up the malloc'ed string.
  static void _finalizeImpl(GCCell *cell, GC *gc);

  /// \return the size of the external memory associated with \p cell, which is
  /// assumed to be an ExternalStringPrimitive.
  static size_t _mallocSizeImpl(GCCell *cell);

  /// \return the size of the external memory credited to the cell.
  static gcheapsize_t _externalMemorySizeImpl(const GCCell *cell);

  static void _snapshotAddEdgesImpl(GCCell *cell, GC *gc, HeapSnapshot &snap);
  static void _snapshotAddNodesImpl(GCCell *cell, GC *gc, HeapSnapshot &snap);

  /// The backing storage of this string. Note that the string's length is fixed
  /// and must always be equal to StringPrimitive::getStringLength().
  StdString contents_{};
};

template <typename T, bool Uniqued>
const VTable DynamicStringPrimitive<T, Uniqued>::vt = VTable(
    DynamicStringPrimitive<T, Uniqued>::getCellKind(),
    0,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    VTable::HeapSnapshotMetadata{
        HeapSnapshot::NodeType::String,
        DynamicStringPrimitive<T, Uniqued>::_snapshotNameImpl,
        nullptr,
        nullptr});

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
    ExternalStringPrimitive<T>::_mallocSizeImpl,
    nullptr,
    nullptr,
    ExternalStringPrimitive<T>::_externalMemorySizeImpl,
    VTable::HeapSnapshotMetadata{
        HeapSnapshot::NodeType::String,
        ExternalStringPrimitive<T>::_snapshotNameImpl,
        ExternalStringPrimitive<T>::_snapshotAddEdgesImpl,
        ExternalStringPrimitive<T>::_snapshotAddNodesImpl});

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
    return ExternalStringPrimitive<char>::create(runtime, arrayToString(str));
  }
}

inline CallResult<HermesValue> StringPrimitive::create(
    Runtime *runtime,
    UTF16Ref str) {
  static_assert(
      EXTERNAL_STRING_THRESHOLD < MAX_STRING_LENGTH,
      "External string threshold should be smaller than max string size.");
  if (LLVM_LIKELY(!isExternalLength(str.size()))) {
    return createDynamic(runtime, str);
  } else {
    return ExternalStringPrimitive<char16_t>::create(
        runtime, arrayToString(str));
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
    return ExternalStringPrimitive<char>::createLongLived(
        runtime, arrayToString(str));
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
    return ExternalStringPrimitive<char16_t>::createLongLived(
        runtime, arrayToString(str));
  }
}

inline bool StringPrimitive::canBeUniqued() const {
  return isa<SymbolStringPrimitive>(this);
}

inline void StringPrimitive::convertToUniqued(hermes::vm::SymbolID uniqueID) {
  assert(canBeUniqued() && "StringPrimitive cannot be uniqued");
  assert(!isUniqued() && "StringPrimitive is already uniqued");
  assert(
      uniqueID.isValid() && uniqueID.isUniqued() &&
      "uniqueID SymbolID is not valid and uniqued");
  this->lengthAndUniquedFlag_ |= LENGTH_FLAG_UNIQUED;
  vmcast<SymbolStringPrimitive>(this)->updateUniqueID(uniqueID);
}

template <>
inline const char *StringPrimitive::castToPointer<char>() const {
  return castToASCIIPointer();
}
template <>
inline const char16_t *StringPrimitive::castToPointer<char16_t>() const {
  return castToUTF16Pointer();
}

inline const char *StringPrimitive::castToASCIIPointer() const {
  if (LLVM_UNLIKELY(isExternal())) {
    return vmcast<ExternalASCIIStringPrimitive>(this)->getRawPointer();
  } else if (isa<DynamicUniquedASCIIStringPrimitive>(this)) {
    return vmcast<DynamicUniquedASCIIStringPrimitive>(this)->getRawPointer();
  } else {
    return vmcast<DynamicASCIIStringPrimitive>(this)->getRawPointer();
  }
}

inline const char16_t *StringPrimitive::castToUTF16Pointer() const {
  if (LLVM_UNLIKELY(isExternal())) {
    return vmcast<ExternalUTF16StringPrimitive>(this)->getRawPointer();
  } else if (isa<DynamicUniquedUTF16StringPrimitive>(this)) {
    return vmcast<DynamicUniquedUTF16StringPrimitive>(this)->getRawPointer();
  } else {
    return vmcast<DynamicUTF16StringPrimitive>(this)->getRawPointer();
  }
}

inline char *StringPrimitive::castToASCIIPointerForWrite() {
  if (LLVM_UNLIKELY(isExternal())) {
    return vmcast<ExternalASCIIStringPrimitive>(this)->getRawPointerForWrite();
  } else if (isa<DynamicUniquedASCIIStringPrimitive>(this)) {
    return vmcast<DynamicUniquedASCIIStringPrimitive>(this)
        ->getRawPointerForWrite();
  } else {
    return vmcast<DynamicASCIIStringPrimitive>(this)->getRawPointerForWrite();
  }
}

inline char16_t *StringPrimitive::castToUTF16PointerForWrite() {
  if (LLVM_UNLIKELY(isExternal())) {
    return vmcast<ExternalUTF16StringPrimitive>(this)->getRawPointerForWrite();
  } else if (isa<DynamicUniquedUTF16StringPrimitive>(this)) {
    return vmcast<DynamicUniquedUTF16StringPrimitive>(this)
        ->getRawPointerForWrite();
  } else {
    return vmcast<DynamicUTF16StringPrimitive>(this)->getRawPointerForWrite();
  }
}

inline SymbolID StringPrimitive::getUniqueID() const {
  assert(this->isUniqued() && "StringPrimitive is not uniqued");
  return vmcast<SymbolStringPrimitive>(this)->getUniqueID();
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

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_STRINGPRIMITIVE_H
