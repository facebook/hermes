/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/VM/GCPointer.h"
#include "hermes/VM/SymbolID.h"
#include "hermes/VM/WeakRoot.h"
#include "hermes/VM/sh_mirror.h"

namespace hermes {
namespace vm {
using SlotIndex = uint32_t;

class JSObject;
class HiddenClass;

/// A cache entry for property writes.
/// If clazz is populated, then it's the class for the property write when
/// modifying an existing property.
/// If a property add is cached, it will be stored in _slotAndAddCacheIndex,
/// and the add cache index will be nonzero.
/// The property add start/result classes can be found in the RuntimeModule list
/// of AddPropertyCacheEntry.
struct WritePropertyCacheEntry {
  /// Cached class.
  WeakRoot<HiddenClass> clazz{nullptr};

  /// Cached property index and addCache index.
  /// Low 8 bits: slot.
  /// High 24 bits: addCache index. 0 is reserved and entry is never stored to.
  uint32_t _slotAndAddCacheIndex{0};

  /// Can store 8-bit slot.
  static constexpr SlotIndex kMaxSlot = 0xff;

  /// Can store 24-bit add cache index.
  static constexpr uint32_t kMaxAddCacheIndex = (1 << 24) - 1;

  /// Mask for the slot value at the bottom byte.
  static constexpr uint32_t kSlotMask = 0xff;

  /// \return the slot.
  SlotIndex getSlot() const {
    // Mask to one byte will result in a single-byte load from memory.
    return _slotAndAddCacheIndex & kSlotMask;
  }
  /// \return the add cache index.
  uint32_t getAddCacheIndex() const {
    // Clear out the top bit because that's not part of the index.
    return (_slotAndAddCacheIndex >> 8);
  }
  /// \return whether there is an associated addCacheIndex which holds valid
  /// cached data.
  bool hasAddCacheIndex() const {
    return getAddCacheIndex() != 0;
  }

  /// Set the slot. Does not affect the addCacheIndex.
  /// \pre slot <= kMaxSlot
  void setSlot(SlotIndex slot) {
    assert(slot <= kMaxSlot && "slot too large");
    // Clear everything that's not the index, then add the slot.
    // Masking slot even though we know it's a no-op due to the above assert
    // allows for better codegen (strb on ARM64).
    _slotAndAddCacheIndex =
        (_slotAndAddCacheIndex & ~kSlotMask) | (slot & 0xff);
  }

  /// Set the add cache index. Does not affect the slot.
  /// \pre slot <= kMaxSlot
  void setAddCacheIndex(uint32_t addCacheIndex) {
    assert(addCacheIndex <= kMaxAddCacheIndex && "index too large");
    _slotAndAddCacheIndex =
        (_slotAndAddCacheIndex & kSlotMask) | (addCacheIndex << 8);
  }
};

/// A cache entry for property reads.
struct ReadPropertyCacheEntry {
  /// Cached class: either for the object being read from,
  /// or it's prototype.
  WeakRoot<HiddenClass> clazz{nullptr};

  /// If \p clazz is a HiddenClass for the prototype, this is
  /// non-null and is the HiddenClass for the child object.  We call this
  /// a "negative match": the property was not found on an object
  /// with this HiddenClass, so if the object used in a subsequent
  /// read has the same HiddenClass, it also won't have the property.
  WeakRoot<HiddenClass> negMatchClazz{nullptr};

  /// Cached property index: in the object if \p clazz is the object's
  /// HiddenClass, or in the object's prototype if \p clazz is the
  /// prototype's HiddenClass.
  SlotIndex slot{0};
};

/// A cache entry used for all private name operations, e.g. reads, stores and
/// `in` checks using a private name.
struct PrivateNameCacheEntry {
  /// Cached class.
  WeakRoot<HiddenClass> clazz{nullptr};

  /// Cached symbol value.
  WeakRootSymbolID nameVal{};

  /// Cached property index. In the case of reads and stores, this is the offset
  /// where the property can be found. For existence checks, like the `in`
  /// operator, this is just a boolean.
  SlotIndex slot{0};
};

/// A cache entry for adding a property and transitioning the HiddenClass.
/// Stored in a list in RuntimeModule to save memory in WritePropertyCacheEntry.
/// In order to hit this cache entry:
/// * The startClazz must match the pre-PutById class of the object
/// * The parent must be the same as the cached parent
/// * The current Runtime parentCacheEpoch must be the same as the cached
///   parentEpoch field.
struct AddPropertyCacheEntry {
  /// Maximum parent epoch, it only holds 24 bits.
  static constexpr uint32_t kMaxParentEpoch = (1 << 24) - 1;

  /// The starting HiddenClass to transition from.
  WeakRoot<HiddenClass> startClazz{nullptr};

  /// The result HiddenClass to transition to.
  WeakRoot<HiddenClass> resultClazz{nullptr};

  /// The parent object pointer at the point the add was cached.
  WeakRoot<JSObject> parent{nullptr};

  /// Low 8 bits: slot.
  /// High 24 bits: The parent epoch at the point the add was cached.
  uint32_t _parentEpochAndSlot{0};

  /// \return the cached parentEpoch.
  uint32_t getParentEpoch() const {
    return _parentEpochAndSlot >> 8;
  }

  /// \return the cached slot.
  uint32_t getSlot() const {
    return _parentEpochAndSlot & 0xff;
  }

  /// Set both the parentEpoch and slot.
  void setParentEpochAndSlot(uint32_t parentEpoch, uint32_t slot) {
    assert(slot <= 0xff && "slot too large");
    _parentEpochAndSlot = (parentEpoch << 8) | (slot & 0xff);
  }
};

static_assert(
    sizeof(SHWritePropertyCacheEntry) == sizeof(WritePropertyCacheEntry));
static_assert(
    offsetof(SHWritePropertyCacheEntry, clazz) ==
    offsetof(WritePropertyCacheEntry, clazz));
static_assert(
    offsetof(SHWritePropertyCacheEntry, slotAndAddCacheIndex) ==
    offsetof(WritePropertyCacheEntry, _slotAndAddCacheIndex));
static_assert(
    sizeof(SHReadPropertyCacheEntry) == sizeof(ReadPropertyCacheEntry));
static_assert(
    offsetof(SHReadPropertyCacheEntry, clazz) ==
    offsetof(ReadPropertyCacheEntry, clazz));
static_assert(
    offsetof(SHReadPropertyCacheEntry, slot) ==
    offsetof(ReadPropertyCacheEntry, slot));
static_assert(sizeof(SHPrivateNameCacheEntry) == sizeof(PrivateNameCacheEntry));
static_assert(
    offsetof(SHPrivateNameCacheEntry, clazz) ==
    offsetof(PrivateNameCacheEntry, clazz));
static_assert(
    offsetof(SHPrivateNameCacheEntry, nameVal) ==
    offsetof(PrivateNameCacheEntry, nameVal));
static_assert(
    offsetof(SHPrivateNameCacheEntry, slot) ==
    offsetof(PrivateNameCacheEntry, slot));

} // namespace vm
} // namespace hermes
