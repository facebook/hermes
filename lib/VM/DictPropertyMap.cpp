/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "vm"
#include "hermes/VM/DictPropertyMap.h"
#include "hermes/Support/Statistic.h"

HERMES_SLOW_STATISTIC(NumDictLookups, "Number of dictionary lookups");
HERMES_SLOW_STATISTIC(NumExtraHashProbes, "Number of extra hash probes");

namespace hermes {
namespace vm {

struct DictPropertyMap::detail {
  /// The upper bound of the search when trying to find the maximum capacity
  /// of this object, given GC::maxAllocationSize().
  /// It was chosen to be a value that is certain to not fit into an allocation;
  /// at the same time we want to make it smaller, so/ we have arbitrarily
  /// chosen to divide the max allocation size by two, which is still guaranteed
  /// not to fit.
  static constexpr uint32_t kSearchUpperBound = GC::maxAllocationSize() / 2;

  static_assert(
      !DictPropertyMap::constWouldFitAllocation(kSearchUpperBound),
      "kSearchUpperBound should not fit into an allocation");

  /// The maximum capacity of DictPropertyMap, given GC::maxAllocationSize().
  static constexpr uint32_t kMaxCapacity =
      DictPropertyMap::constFindMaxCapacity(0, kSearchUpperBound);

  // Double-check that kMaxCapacity is reasonable.
  static_assert(
      DictPropertyMap::constApproxAllocSize64(kMaxCapacity) <=
          GC::maxAllocationSize(),
      "invalid kMaxCapacity");

  // Ensure that it is safe to double capacities without checking for overflow
  // until we exceed kMaxCapacity.
  static_assert(
      kMaxCapacity < (1u << 31),
      "kMaxCapacity is unrealistically large");

  static_assert(
      DictPropertyMap::HashPair::canStore(kMaxCapacity),
      "too few bits to store max possible descriptor index");
};

VTable DictPropertyMap::vt{CellKind::DictPropertyMapKind, 0};

void DictPropertyMapBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const DictPropertyMap *>(cell);
  mb.addArray<Metadata::ArrayData::ArrayType::Symbol>(
      self->getDescriptorPairs(),
      &self->numDescriptors_,
      sizeof(DictPropertyMap::DescriptorPair));
}

DictPropertyMap::size_type DictPropertyMap::getMaxCapacity() {
  return detail::kMaxCapacity;
}

CallResult<PseudoHandle<DictPropertyMap>> DictPropertyMap::create(
    Runtime *runtime,
    size_type capacity) {
  if (LLVM_UNLIKELY(capacity > detail::kMaxCapacity)) {
    return runtime->raiseRangeError(
        TwineChar16("Property storage exceeds ") + detail::kMaxCapacity +
        " properties");
  }
  size_type hashCapacity = calcHashCapacity(capacity);
  void *mem = runtime->alloc</*fixedSize*/ false>(
      allocationSize(capacity, hashCapacity));
  return createPseudoHandle(
      new (mem) DictPropertyMap(runtime, capacity, hashCapacity));
}

#ifdef HERMESVM_SERIALIZE
void DictPropertyMapSerialize(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const DictPropertyMap>(cell);
  s.writeInt<uint32_t>(self->descriptorCapacity_);
  s.writeInt<uint32_t>(self->hashCapacity_);

  s.writeInt<uint32_t>(self->numDescriptors_);
  s.writeInt<uint32_t>(self->numProperties_);
  s.writeInt<uint32_t>(self->deletedListHead_);
  s.writeInt<uint32_t>(self->deletedListSize_);

  // No pointer in any of the arrays. let's do a memcpy of the storage
  // and write a size here for sanity check
  size_t size = DictPropertyMap::allocationSize(
                    self->descriptorCapacity_, self->hashCapacity_) -
      sizeof(DictPropertyMap);
  s.writeInt<uint32_t>(size);
  s.writeData(self->getDescriptorPairs(), size);
  s.endObject(cell);
}

void DictPropertyMapDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::DictPropertyMapKind && "Expected DictPropertyMap");
  uint32_t descriptorCapacity = d.readInt<uint32_t>();
  uint32_t hashCapacity = d.readInt<uint32_t>();

  if (LLVM_UNLIKELY(
          descriptorCapacity > DictPropertyMap::detail::kMaxCapacity)) {
    hermes_fatal("deserialized descriptorCapacity exceeds limit");
  }

#ifndef NDEBUG
  assert(
      DictPropertyMap::calcHashCapacity(descriptorCapacity) == hashCapacity &&
      "deserialized hash capacity does not match with calculated hashCapacity");
#endif

  void *mem = d.getRuntime()->alloc</*fixedSize*/ false>(
      DictPropertyMap::allocationSize(descriptorCapacity, hashCapacity));
  auto *cell = new (mem)
      DictPropertyMap(d.getRuntime(), descriptorCapacity, hashCapacity);

  cell->numDescriptors_ = d.readInt<uint32_t>();
  cell->numProperties_ = d.readInt<uint32_t>();
  cell->deletedListHead_ = d.readInt<uint32_t>();
  cell->deletedListSize_ = d.readInt<uint32_t>();

  // Read the whole storage into the trailing objects.
  size_t size = d.readInt<uint32_t>();
  assert(
      DictPropertyMap::allocationSize(
          cell->descriptorCapacity_, cell->hashCapacity_) ==
          size + sizeof(DictPropertyMap) &&
      "allocation size doesn't match");
  d.readData(cell->getDescriptorPairs(), size);

  d.endObject(cell);
}
#endif

std::pair<bool, DictPropertyMap::HashPair *> DictPropertyMap::lookupEntryFor(
    DictPropertyMap *self,
    SymbolID symbolID) {
  ++NumDictLookups;

  size_type const mask = self->hashCapacity_ - 1;
  size_type index = hash(symbolID) & mask;

  // Probing step.
  size_type step = 1;
  // Save the address of the start of the table to avoid recalculating it.
  HashPair *const tableStart = self->getHashPairs();
  // The first deleted entry we found.
  HashPair *deleted = nullptr;

  assert(symbolID.isValid() && "looking for an invalid SymbolID");

  for (;;) {
    HashPair *curEntry = tableStart + index;

    if (curEntry->isValid()) {
      if (self->isMatch(curEntry, symbolID))
        return {true, curEntry};
    } else if (curEntry->isEmpty()) {
      // If we encountered an empty pair, the search is over - we failed.
      // Return either this entry or a deleted one, if we encountered one.

      return {false, deleted ? deleted : curEntry};
    } else {
      assert(curEntry->isDeleted() && "unexpected HashPair state");
      // The first time we encounter a deleted entry, record it so we can
      // potentially reuse it for insertion.
      if (!deleted)
        deleted = curEntry;
    }

    ++NumExtraHashProbes;
    index = (index + step) & mask;
    ++step;
  }
}

ExecutionStatus DictPropertyMap::grow(
    MutableHandle<DictPropertyMap> &selfHandleRef,
    Runtime *runtime,
    size_type newCapacity) {
  auto res = create(runtime, newCapacity);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto *newSelf = res->get();
  auto *self = *selfHandleRef;

  selfHandleRef = newSelf;

  auto *dst = newSelf->getDescriptorPairs();
  size_type count = 0;

  for (auto *src = self->getDescriptorPairs(), *e = src + self->numDescriptors_;
       src != e;
       ++src) {
    if (src->first.isInvalid())
      continue;

    auto key = src->first;

    dst->first = key;
    dst->second = src->second;

    auto result = lookupEntryFor(newSelf, key);
    assert(!result.first && "found duplicate entry while growing");
    result.second->setDescIndex(count, key);

    ++dst;
    ++count;
  }

  assert(
      count == self->numProperties_ && "numProperties mismatch when growing");

  newSelf->numProperties_ = count;

  // Transfer the deleted list to the new instance.
  auto deletedIndex = self->deletedListHead_;
  if (deletedIndex != END_OF_LIST) {
    newSelf->deletedListHead_ = count;
    newSelf->deletedListSize_ = self->deletedListSize_;

    do {
      const auto *src = self->getDescriptorPairs() + deletedIndex;
      assert(
          src->first == SymbolID::deleted() &&
          "pair in the deleted list is not marked as deleted");

      dst->first = SymbolID::deleted();
      dst->second.slot = src->second.slot;

      deletedIndex = getNextDeletedIndex(src);
      setNextDeletedIndex(
          dst, deletedIndex != END_OF_LIST ? count + 1 : END_OF_LIST);

      ++dst;
      ++count;
    } while (deletedIndex != END_OF_LIST);
  }

  newSelf->numDescriptors_ = count;
  assert(count <= newSelf->descriptorCapacity_);
  return ExecutionStatus::RETURNED;
}

CallResult<std::pair<NamedPropertyDescriptor *, bool>>
DictPropertyMap::findOrAdd(
    MutableHandle<DictPropertyMap> &selfHandleRef,
    Runtime *runtime,
    SymbolID id) {
  auto *self = *selfHandleRef;
  auto found = lookupEntryFor(self, id);
  if (found.first) {
    return std::make_pair(
        &self->getDescriptorPairs()[found.second->getDescIndex()].second,
        false);
  }

  // We want to grow the hash table if the number of occupied hash entries
  // exceeds 75% of capacity or if the descriptor array is full. Since the
  // capacity of the table is 4/3 of the capacity of the descriptor array, it is
  // sufficient to only check for the latter.

  if (self->numDescriptors_ == self->descriptorCapacity_) {
    size_type newCapacity;
    if (self->numProperties_ == self->descriptorCapacity_) {
      // Double the new capacity, up to kMaxCapacity. However make sure that
      // we try to allocate at least one extra property. If we are already
      // exactly at kMaxCapacity, there is nothing we can do, so grow() will
      // simply fail.
      newCapacity = self->numProperties_ * 2;
      if (newCapacity > detail::kMaxCapacity)
        newCapacity =
            std::max(toRValue(detail::kMaxCapacity), self->numProperties_ + 1);
    } else {
      // Calculate the new capacity to be exactly as much as we need to
      // accomodate the deleted list plus one extra property. It it happens
      // to exceed kMaxCapacity, there is nothing we can do, so grow() will
      // raise an exception.
      newCapacity = self->numProperties_ + 1 + self->deletedListSize_;
    }

    if (LLVM_UNLIKELY(
            grow(selfHandleRef, runtime, newCapacity) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    self = *selfHandleRef;

    found = lookupEntryFor(self, id);
  }

  ++self->numProperties_;
  if (found.second->isDeleted())
    self->decDeletedHashCount();

  found.second->setDescIndex(self->numDescriptors_, id);

  auto *descPair = self->getDescriptorPairs() + self->numDescriptors_;

  descPair->first = id;
  ++self->numDescriptors_;

  return std::make_pair(&descPair->second, true);
}

void DictPropertyMap::erase(DictPropertyMap *self, PropertyPos pos) {
  auto *hashPair = self->getHashPairs() + pos.hashPairIndex;
  auto descIndex = hashPair->getDescIndex();
  assert(descIndex < self->numDescriptors_ && "descriptor index out of range");

  auto *descPair = self->getDescriptorPairs() + descIndex;
  assert(
      descPair->first != SymbolID::empty() &&
      "accessing deleted descriptor pair");

  hashPair->setDeleted();
  descPair->first = SymbolID::deleted();
  // Add the descriptor to the deleted list.
  setNextDeletedIndex(descPair, self->deletedListHead_);
  self->deletedListHead_ = descIndex;
  ++self->deletedListSize_;

  assert(self->numProperties_ != 0 && "num properties out of sync");
  --self->numProperties_;
  self->incDeletedHashCount();
}

SlotIndex DictPropertyMap::allocatePropertySlot(DictPropertyMap *self) {
  // If there are no deleted properties, the number of properties corresponds
  // exactly to the number of slots.
  if (self->deletedListHead_ == END_OF_LIST)
    return self->numProperties_;

  auto *deletedPair = self->getDescriptorPairs() + self->deletedListHead_;
  assert(
      deletedPair->first == SymbolID::deleted() &&
      "Head of deleted list is not deleted");

  // Remove the first element from the deleted list.
  self->deletedListHead_ = getNextDeletedIndex(deletedPair);
  --self->deletedListSize_;

  // Mark the pair as "invalid" instead of "deleted".
  deletedPair->first = SymbolID::empty();

  return deletedPair->second.slot;
}

void DictPropertyMap::dump() {
  auto &OS = llvm::errs();

  OS << "DictPropertyMap:" << getDebugAllocationId() << "\n";
  OS << "  HashPairs[" << hashCapacity_ << "]:\n";
  for (unsigned i = 0; i < hashCapacity_; ++i) {
    auto *pair = getHashPairs() + i;
    if (pair->isValid()) {
      OS << "    " << pair->getDescIndex() << "\n";
    } else if (pair->isEmpty()) {
      OS << "    (empty)\n";
    } else {
      assert(pair->isDeleted());
      OS << "    (deleted)\n";
    }
  }
  OS << "  Descriptors[" << descriptorCapacity_ << "]:\n";
  for (unsigned i = 0; i < descriptorCapacity_; ++i) {
    auto *pair = getDescriptorPairs() + i;
    OS << "    (" << pair->first << ", "
       << "(slot=" << pair->second.slot << "))\n";
  }
}

} // namespace vm
} // namespace hermes
