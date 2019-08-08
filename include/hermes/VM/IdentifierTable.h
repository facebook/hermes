/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_IDENTIFIERTABLE_H
#define HERMES_VM_IDENTIFIERTABLE_H

#include "hermes/ADT/PtrOrInt.h"
#include "hermes/Support/HashString.h"
#include "hermes/VM/CallResult.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/Handle.h"
#include "hermes/VM/StringRefUtils.h"
#include "hermes/VM/SymbolID.h"
#include "hermes/VM/detail/IdentifierHashTable.h"

#include <functional>
#include <vector>

#include "llvm/ADT/DenseMap.h"

namespace hermes {
namespace vm {

class Runtime;
class RuntimeModule;
class StringPrimitive;
class StringView;

/// This class maintains a table of values used as keys in objects,
/// or as they are commonly referred throughout the source "identifiers" or
/// "symbols" (in ES6 parlance).
///
/// The sequential integer index of each string in the table is a 32-bit
/// integer value (SymbolID::RawType) that uniquely identifies the string.
/// For external type safety the 32-bit value is wrapped in a
/// lightweight class \c SymbolID which doesn't offer additional functionality
/// but restricts unsafe operations.
///
/// SymbolIDs are used in the VM to efficiently encode property names and string
/// literals.
///
/// The strings are kept in \c std::vector with a side hash table to perform
/// efficient lookups from a string to \c IdentifierID/SymbolID.
///
/// There are two types of SymbolIDs stored in the IdentifierTable.
/// - Uniqued SymbolIDs. They are stored in a hash table, with a lookup vector
///   which assigns SymbolIDs to strings. IdentifierTable provides a two-way
///   mapping between internal SymbolIDs and the strings which they represent.
/// - Not Uniqued SymbolIDs.  These are not placed in the hash table, but they
///   are placed in the lookup vector. Thus, we can map from a SymbolID
///   to the description string for the Symbol, but the mapping back is not
///   unique. These back the primitive JS type Symbol because \c Symbol('x') !==
///   Symbol('x').
///
/// SymbolIDs that are no longer used by the VM are detected by the garbage
/// collector which then invokes the \c freeUnmarkedSymbols() method of this
/// class. The memory for the unneeded symbols is released and their raw
/// SymbolIDs are made available for reuse. For that purpose a singly-
/// linked list of freed ids is maintained inside the index array.
///
/// SymbolIDs are guaranteed to be allocated sequentially from zero until the
/// first occasion when a symbol is freed.
class IdentifierTable {
  friend class detail::IdentifierHashTable;

 public:
  /// Initialize the identifier table.
  IdentifierTable();

  /// Given a UTF16 string \p str, retrieve a unique SymbolID for that
  /// string.
  /// If the SymbolID for a string has been previously requested,
  /// the same SymbolID will be returned on subsequent calls.
  /// Note it is not safe to pass pointers to potentially GC-heap allocated
  /// strings here. Only strings not managed by the GC may be passed here.
  CallResult<Handle<SymbolID>>
  getSymbolHandle(Runtime *runtime, UTF16Ref str, uint32_t hash);
  CallResult<Handle<SymbolID>> getSymbolHandle(Runtime *runtime, UTF16Ref str) {
    return getSymbolHandle(runtime, str, hermes::hashString(str));
  }

  /// Given a ASCII string \p str, retrieve a unique SymbolID for that
  /// string.
  CallResult<Handle<SymbolID>>
  getSymbolHandle(Runtime *runtime, ASCIIRef str, uint32_t hash);
  CallResult<Handle<SymbolID>> getSymbolHandle(Runtime *runtime, ASCIIRef str) {
    return getSymbolHandle(runtime, str, hermes::hashString(str));
  }

  /// Register a lazy ASCII identifier from a bytecode module or as predefined
  /// identifier.
  /// This function should only be called during initialization of a module.
  SymbolID registerLazyIdentifier(ASCIIRef str);
  SymbolID registerLazyIdentifier(ASCIIRef str, uint32_t hash);

  /// Register a lazy UTF16 identifier from a bytecode module or as predefined
  /// identifier.
  SymbolID registerLazyIdentifier(UTF16Ref str);
  SymbolID registerLazyIdentifier(UTF16Ref str, uint32_t hash);

  /// \return the SymbolID of the string primitive \p str.
  CallResult<Handle<SymbolID>> getSymbolHandleFromPrimitive(
      Runtime *runtime,
      PseudoHandle<StringPrimitive> str);

  /// Given a \c SymbolID \p id, get the unique string str such that
  /// getIdentifier(str) == id.
  StringPrimitive *getStringPrim(Runtime *runtime, SymbolID id);

  /// Given an \c SymbolID \p id, get a view of the unique string str such
  /// that getIdentifier(str) == id.
  StringView getStringView(Runtime *runtime, SymbolID id) const;

  /// Extract a symbol's name into an ASCII character buffer without performing
  /// any GC operations. This is used for debugging and logging only.
  void debugGetSymbolName(
      Runtime *runtime,
      SymbolID id,
      llvm::SmallVectorImpl<char> &buffer);

  /// Reserve enough space in the hash table to contain \p count identifiers.
  void reserve(uint32_t count) {
    hashTable_.reserve(count);
  }

  /// \return an estimate of the size of additional memory used by this
  /// IdentifierTable.
  size_t additionalMemorySize() const {
    return hashTable_.additionalMemorySize();
  }

  /// Mark all identifiers with the garbage collector \p gc.
  /// This is only supported in debug mode; in release mode identifiers are
  /// never allocated from the GC heap.
  void markIdentifiers(SlotAcceptorWithNames &acceptor, GC *gc);

  /// Visits every entry in the identifier table and calls acceptor with
  /// the entry and its id as arguments. This is intended to be used only for
  /// snapshots, as it is slow. The function passed as acceptor shouldn't
  /// perform any heap operations
  void visitIdentifiers(const std::function<void(UTF16Ref, uint32_t)> &lambda);

  /// \return one higher than the largest symbol in the identifier table. This
  /// enables the GC to size its internal structures for symbol marking.
  /// Optionally invoked at the beginning of a garbage collection.
  unsigned getSymbolsEnd() const {
    return lookupVector_.size();
  }

  /// Invoked at the end of a GC to free all unmarked symbols.
  void freeUnmarkedSymbols(const std::vector<bool> &markedSymbols);

  /// Allocate a new SymbolID without adding an entry to the
  /// IdentifierHashTable, so it is not uniqued; this function should only be
  /// used during Runtime initialization. \param desc the description of the
  /// not uniqued symbol.
  /// \return the newly allocated SymbolID.
  SymbolID createNotUniquedLazySymbol(ASCIIRef desc);

  /// Allocate a new SymbolID without adding an entry to the
  /// IdentifierHashTable, so it is not uniqued.
  /// \param desc the description of the not uniqued symbol.
  /// \return the newly allocated SymbolID.
  CallResult<SymbolID> createNotUniquedSymbol(
      Runtime *runtime,
      Handle<StringPrimitive> desc);

  void serialize(Serializer &s);

  void deserialize(Deserializer &d);

 private:
  /// Entry in the lookup vector. An entry can represent 5 different types:
  /// 1. A lazy ASCII identifier: asciiPtr_ will be set to a pointer to
  ///   some static char * memory, num_ will be the string length,
  ///   and isUTF16_ will be false.
  /// 2. A lazy UTF16 identifier: utf16Ptr_ will be set to a pointer to
  ///   some static char16_t * memory, num_ will be the string length,
  ///   and isUTF16_ will be true.
  /// 3. A StringPrimitive that's created dynamically during runtime:
  ///   strPrim_ will be set, and num_ will be NON_LAZY_STRING_PRIM_TAG.
  /// 4. A StringPrimitive that's transformed from a lazy identifier:
  ///   strPrim_ will be set, and num_ will be LAZY_STRING_PRIM_TAG. We
  ///   need to distinguish 3 and 4 since we never want to free 4, but we
  ///   could free 3 once it's not used.
  /// 5. A free slot. union will be nullptr, and num_ will be the index of
  ///   the next free slot (FREE_LIST_END means the end of free list).
  ///   Note that FREE_LIST_END is not a unique value, but it should only
  ///   be used when the union is nullptr, so that's not a problem.
  class LookupEntry {
    static constexpr uint32_t LAZY_STRING_PRIM_TAG = (uint32_t)(1 << 30) - 1;
    static constexpr uint32_t NON_LAZY_STRING_PRIM_TAG =
        LAZY_STRING_PRIM_TAG - 1;

   public:
    static constexpr uint32_t FREE_LIST_END = LAZY_STRING_PRIM_TAG;
    static constexpr uint32_t MAX_IDENTIFIER = NON_LAZY_STRING_PRIM_TAG - 1;

   private:
    union {
      StringPrimitive *strPrim_;
      const char *asciiPtr_;
      const char16_t *utf16Ptr_;
    };

    /// This flag is unused if the entry is empty (the union is nullptr)
    /// or if the union represents a StringPrimitive.
    /// If the union is ASCII or UTF16, this is set to true if UTF16.
    bool isUTF16_ : 1;

    /// Set to true if the Identifier hasn't been added to the hashtable.
    bool isNotUniqued_ : 1;

    uint32_t num_ : 30;

    /// Hash code for the represented string.
    uint32_t hash_{};

   public:
    explicit LookupEntry(ASCIIRef str, bool isNotUniqued = false)
        : asciiPtr_(str.data()),
          isUTF16_(false),
          isNotUniqued_(isNotUniqued),
          num_(str.size()),
          hash_(hermes::hashString(str)) {}
    explicit LookupEntry(ASCIIRef str, uint32_t hash, bool isNotUniqued = false)
        : asciiPtr_(str.data()),
          isUTF16_(false),
          isNotUniqued_(isNotUniqued),
          num_(str.size()),
          hash_(hash) {}
    explicit LookupEntry(UTF16Ref str)
        : utf16Ptr_(str.data()),
          isUTF16_(true),
          isNotUniqued_(false),
          num_(str.size()),
          hash_(hermes::hashString(str)) {}
    explicit LookupEntry(UTF16Ref str, uint32_t hash)
        : utf16Ptr_(str.data()),
          isUTF16_(true),
          isNotUniqued_(false),
          num_(str.size()),
          hash_(hash) {}
    explicit LookupEntry(StringPrimitive *str, bool isNotUniqued = false);
    explicit LookupEntry(
        StringPrimitive *str,
        uint32_t hash,
        bool isNotUniqued = false)
        : strPrim_(str),
          isUTF16_(true),
          isNotUniqued_(isNotUniqued),
          num_(NON_LAZY_STRING_PRIM_TAG),
          hash_(hash) {
      assert(str && "Invalid string primitive pointer");
    }

    /// Initialized as a free slot.
    LookupEntry() : strPrim_(nullptr), num_(FREE_LIST_END) {}

    bool isLazyASCII() const {
      return asciiPtr_ && !isUTF16_;
    }
    bool isLazyUTF16() const {
      return utf16Ptr_ && isUTF16_;
    }
    bool isStringPrim() const {
      return strPrim_ && num_ >= NON_LAZY_STRING_PRIM_TAG;
    }
    bool isLazyStringPrim() const {
      return strPrim_ && num_ == LAZY_STRING_PRIM_TAG;
    }
    bool isNonLazyStringPrim() const {
      return strPrim_ && num_ == NON_LAZY_STRING_PRIM_TAG;
    }
    bool isFreeSlot() const {
      // union is not necessarily a StringPrimitive*. However the purpose here
      // is a null check, and checking one pointer element is sufficient.
      return !strPrim_;
    }
    bool isNotUniqued() const {
      return isNotUniqued_;
    }

    /// Transform a lazy identifier into a StringPrimitive.
    void materialize(StringPrimitive *str) {
      assert(str && "Invalid string primitive pointer");
      strPrim_ = str;
      num_ = LAZY_STRING_PRIM_TAG;
    }

    /// Free this slot, only makes sense if it's a StringPrimitive.
    void free(int32_t nextFreeSlotID) {
      assert(isStringPrim() && "Cannot free this slot");
      strPrim_ = nullptr;
      num_ = nextFreeSlotID;
    }

    ASCIIRef getLazyASCIIRef() const {
      assert(isLazyASCII() && "Not an ASCII lazy identifier");
      return ASCIIRef(asciiPtr_, num_);
    }
    UTF16Ref getLazyUTF16Ref() const {
      assert(isLazyUTF16() && "Not a UTF16 lazy identifier");
      return UTF16Ref(utf16Ptr_, num_);
    }
    StringPrimitive *&getStringPrimRef() {
      assert(isStringPrim() && "Not a valid string primitive");
      return strPrim_;
    }
    const StringPrimitive *getStringPrim() const {
      assert(isStringPrim() && "Not a valid string primitive");
      return strPrim_;
    }
    int32_t getNextFreeSlot() const {
      assert(isFreeSlot() && "Not a free slot");
      return num_;
    }
    uint32_t getHash() const {
      return hash_;
    }

    void serialize(Serializer &s);

    void deserialize(Deserializer &d);
  };

  /// Stores all the entries referenced from the hash table, plus
  /// free slots.
  std::vector<LookupEntry> lookupVector_;

  /// The hash table.
  detail::IdentifierHashTable hashTable_{};

  /// Index of the first free index in lookupVector_.
  uint32_t firstFreeID_{LookupEntry::FREE_LIST_END};

  LookupEntry &getLookupTableEntry(SymbolID id) {
    return getLookupTableEntry(id.unsafeGetIndex());
  }

  const LookupEntry &getLookupTableEntry(SymbolID id) const {
    return getLookupTableEntry(id.unsafeGetIndex());
  }

  LookupEntry &getLookupTableEntry(uint32_t id) {
    assert(id < lookupVector_.size() && "Identifier ID out of bound");
    return lookupVector_[id];
  }

  const LookupEntry &getLookupTableEntry(uint32_t id) const {
    assert(id < lookupVector_.size() && "Identifier ID out of bound");
    return lookupVector_[id];
  }
  /// \param hash the hash of the string \p str.
  /// \return SymbolID from a string \str. If \p prim is not null, the
  /// primitive is assumed to be backing str, and is saved into a Handle and
  /// reloaded across allocation.
  template <typename T>
  CallResult<SymbolID> getOrCreateIdentifier(
      Runtime *runtime,
      llvm::ArrayRef<T> str,
      Handle<StringPrimitive> primHandle,
      uint32_t hash);
  template <typename T>
  CallResult<SymbolID> getOrCreateIdentifier(
      Runtime *runtime,
      llvm::ArrayRef<T> str,
      Handle<StringPrimitive> primHandle) {
    return getOrCreateIdentifier(
        runtime, str, primHandle, hermes::hashString(str));
  }

  /// Internal implementation of registerLazyIdentifier().
  template <typename T>
  SymbolID registerLazyIdentifierImpl(llvm::ArrayRef<T> str, uint32_t hash);

  /// Allocate a new SymbolID, and set it to \p str. Update the hash table
  /// location \p hashTableIndex with the ID. \return the new ID.
  uint32_t allocIDAndInsert(uint32_t hashTableIndex, StringPrimitive *str);

  /// Free the symbol with the specified index \p index.
  /// The specified symbol must be a valid one (not previously freed).
  void freeSymbol(uint32_t index);

  /// Allocate the next available id in \c lookupVector_.
  uint32_t allocNextID();

  /// Free an id in \c lookupVector_.
  void freeID(uint32_t id);

  /// Allocates a copy of the StringPrimitive \p prim or \p str, depending on
  /// whether \p primHandle is available, assigning it the given \p strId.
  /// \p Unique indicates that this string should be uniqued.
  /// \return the new allocated string.
  template <typename T, bool Unique = true>
  CallResult<PseudoHandle<StringPrimitive>> allocateDynamicString(
      Runtime *runtime,
      llvm::ArrayRef<T> str,
      Handle<StringPrimitive> primHandle,
      SymbolID strId);

  /// Turn an existing lazy identifier into a StringPrimitive.
  StringPrimitive *materializeLazyIdentifier(Runtime *runtime, SymbolID id);
};

} // end namespace vm
} // end namespace hermes

#endif
