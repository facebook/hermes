/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SYMBOLID_H
#define HERMES_VM_SYMBOLID_H

#include "hermes/VM/GCDecl.h"

#include "llvh/ADT/DenseMap.h"

#include <cstdint>

namespace llvh {
class raw_ostream;
}

namespace hermes {
namespace vm {

/// This is an instance of a unique identifier index. It is guaranteed that
/// an instance of SymbolID always refers to a valid identifier index. To
/// enforce this guarantee, SymbolID instances cannot be created freely - they
/// are only created by "trusted" code.
///
/// This is a very lightweight class - with optimizations on, it is equivalent
/// to "uint32_t". It is supposed to be passed by value.
/// The top bits are reserved for the tag in a SmallHermesValue and only the
/// bottom NUM_BITS should be used.
class SymbolID {
 public:
  using RawType = uint32_t;

 private:
  /// The number of bits that we can actually use for a SymbolID, so that it
  /// still fits in a SmallHermesValue.
  static constexpr size_t NUM_BITS = 29;
  /// Extract only the bottom NUM_BITS bits from a given RawType.
  static constexpr RawType VALUE_MASK = (1 << NUM_BITS) - 1;

 public:
  /// Mask with the not uniqued bit (i.e. the top bit) set.
  static constexpr RawType NOT_UNIQUED_MASK = 1 << (NUM_BITS - 1);
  /// The largest symbol index allowed. That is, the largest possible SymbolID
  /// value if we ignore the not uniqued bit.
  static constexpr RawType LAST_INDEX = NOT_UNIQUED_MASK - 1;
  /// All values in the range [FIRST_INVALID_ID..LAST_INVALID_ID] are considered
  /// invalid. LAST_INVALID_ID is deliberately the largest possible integer, so
  /// we don't actually need to use it for range comparisons.
  static constexpr RawType LAST_INVALID_ID = VALUE_MASK;
  /// All values greater or equal than this one are considered invalid.
  static constexpr RawType FIRST_INVALID_ID = LAST_INVALID_ID - 1;

  /// An empty, uninitialized id, also used as an empty slot in the Identifier
  /// Table.
  static constexpr RawType EMPTY_ID = LAST_INVALID_ID - 0;

  /// Represents an ID that's deleted/garbage collected. Also used as a
  /// 'tombstone' key in llvh::DenseMap.
  static constexpr RawType DELETED_ID = LAST_INVALID_ID - 1;

  constexpr SymbolID() : id_(EMPTY_ID) {}

  SymbolID(const SymbolID &) = default;
  ~SymbolID() = default;
  SymbolID &operator=(const SymbolID &) = default;

  /// \return the index backing this SymbolID.
  /// Top bit will be cleared, so this doesn't distinguish between uniqued
  /// and not uniqued Symbols.
  constexpr uint32_t unsafeGetIndex() const {
    return id_ & (VALUE_MASK >> 1);
  }

  /// \return the raw 32-int number backing this SymbolID.
  /// This encodes both the lookup vector index and whether the symbol has
  /// been uniqued.
  RawType unsafeGetRaw() const {
    return id_;
  }

  bool isValid() const {
    return id_ < FIRST_INVALID_ID;
  }
  bool isInvalid() const {
    return id_ >= FIRST_INVALID_ID;
  }

  /// \return true if this SymbolID has not been uniqued through interning in
  /// IdentifierTable's hash table.
  constexpr bool isNotUniqued() const {
    return id_ & NOT_UNIQUED_MASK;
  }
  /// \return true if this SymbolID has been uniqued through interning in
  /// IdentifierTable's hash table.
  constexpr bool isUniqued() const {
    return !isNotUniqued();
  }

  bool operator==(SymbolID sym) const {
    return id_ == sym.id_;
  }
  bool operator!=(SymbolID sym) const {
    return id_ != sym.id_;
  }

  /// Create an instance of a \c SymbolID from an \c RawType. This
  /// function is unsafe: there is nothing preventing the garbage collector
  /// from freeing the referenced identifier. So, this function must only be
  /// used when we have ensured by other means that the identifier cannot be
  /// collected: e.g. it could be stored in a Handle<SymbolID> or in a GC root.
  static constexpr SymbolID unsafeCreate(RawType id) {
    assert(id <= LAST_INVALID_ID && "ID outside valid range");
    return SymbolID{id};
  }

  /// SymbolID::Empty and SymbolID::Deleted cannot be declared as constexpr
  /// member fields because those must be defined inline, and SymbolID is
  /// incomplete at the time of declaration.

  /// The "invalid" identifier id. Also marks an empty slot in hash tables.
  static constexpr SymbolID empty() {
    return SymbolID{EMPTY_ID};
  }

  /// Deleted identifier id. Marks deleted slots in hash tables.
  static constexpr SymbolID deleted() {
    return SymbolID{DELETED_ID};
  }

  /// Create an instance of a non-uniqued \c SymbolID given its index.
  /// This function is unsafe: there is nothing preventing the garbage collector
  /// from freeing the referenced identifier. So, this function must only be
  /// used when we have ensured by other means that the identifier cannot be
  /// collected: e.g. it could be stored in a Handle<SymbolID> or in a GC root.
  /// \param index the index of the given Symbol in the lookup vector.
  /// This will create a non-uniqued symbol (the top bit will be set).
  static constexpr SymbolID unsafeCreateNotUniqued(uint32_t index) {
    assert(index <= LAST_INDEX && "Index outside valid range");
    return SymbolID{index | NOT_UNIQUED_MASK};
  }

 protected:
  RawType id_;

  explicit constexpr SymbolID(RawType id) : id_(id) {}
};

llvh::raw_ostream &operator<<(llvh::raw_ostream &OS, SymbolID symbolID);

/// A SymbolID that lives on the JS heap. Hides the assignment operator, but
/// provides set operations that do a write barrier.
class GCSymbolID final : public SymbolID {
 public:
  constexpr GCSymbolID() : SymbolID() {}

  /// No write barrier needed for construction.
  explicit GCSymbolID(SymbolID id) : SymbolID(id) {}

  GCSymbolID(const GCSymbolID &) = default;
  ~GCSymbolID() = default;

  /// Deleted so a write barrier is used instead.
  GCSymbolID &operator=(const GCSymbolID &) = delete;

  /// Write a new value into this. Performs a write barrier for some GCs.
  inline GCSymbolID &set(SymbolID sym, GC &gc);
};

/// A SymbolID which is stored in non-moveable memory and is known to the
/// garbage collector.
class RootSymbolID final : public SymbolID {
 public:
  constexpr RootSymbolID() : SymbolID() {}

  explicit RootSymbolID(SymbolID id) : SymbolID(id) {}
};

} // namespace vm
} // namespace hermes

// Enable using SymbolID in DenseMap.
namespace llvh {

using namespace hermes::vm;

template <>
struct DenseMapInfo<SymbolID> {
  static inline SymbolID getEmptyKey() {
    return SymbolID::empty();
  }
  static inline SymbolID getTombstoneKey() {
    return SymbolID::deleted();
  }
  static inline unsigned getHashValue(SymbolID symbolID) {
    return DenseMapInfo<SymbolID::RawType>::getHashValue(
        symbolID.unsafeGetRaw());
  }
  static inline bool isEqual(SymbolID a, SymbolID b) {
    return a == b;
  }
};

} // namespace llvh

#endif // HERMES_VM_SYMBOLID_H
