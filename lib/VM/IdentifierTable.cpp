/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "identtable"
#include "hermes/VM/IdentifierTable.h"

#include "hermes/Support/UTF8.h"
#include "hermes/VM/CallResult.h"
#include "hermes/VM/Deserializer.h"
#include "hermes/VM/Serializer.h"
#include "hermes/VM/StringBuilder.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"

#include "llvh/Support/Debug.h"

namespace hermes {
namespace vm {

IdentifierTable::LookupEntry::LookupEntry(
    StringPrimitive *str,
    bool isNotUniqued)
    : strPrim_(str),
      isUTF16_(false),
      isNotUniqued_(isNotUniqued),
      marked_(true),
      num_(NON_LAZY_STRING_PRIM_TAG) {
  assert(str && "Invalid string primitive pointer");
  llvh::SmallVector<char16_t, 32> storage{};
  str->appendUTF16String(storage);
  hash_ = hermes::hashString(llvh::ArrayRef<char16_t>(storage));
}

#ifdef HERMESVM_SERIALIZE
enum class EntryKind { ASCII, UFT16, StrPrim, Free };

void IdentifierTable::LookupEntry::serialize(Serializer &s) {
  // Common for every type.
  s.writeInt<uint8_t>(isNotUniqued_);
  s.writeInt<uint32_t>(hash_);
  if (isStringPrim()) {
    // Actual StringPrimitive pointed to should be on the heap, will be
    // serialized later.
    // Write the following data for this entry:
    // [STRPRIM, num_, strPrim_(relocation)].
    s.writeInt<uint8_t>((uint8_t)EntryKind::StrPrim);
    s.writeInt<uint32_t>(num_);
    s.writeRelocation(strPrim_);
  } else if (isLazyASCII()) {
    // Pointer to an ASCII string literal.
    // Write the following data for this entry: [ASCII, num, str(offset)].
    s.writeInt<uint8_t>((uint8_t)EntryKind::ASCII);
    s.writeInt<uint32_t>(num_);
    s.writeCharStr(getLazyASCIIRef());
  } else if (isLazyUTF16()) {
    // Pointer to an UTF16 string literal.
    // Write the following data for this entry: [UTF16, num, str(offset)].
    s.writeInt<uint8_t>((uint8_t)EntryKind::UFT16);
    s.writeInt<uint32_t>(num_);
    s.writeChar16Str(getLazyUTF16Ref());
  } else {
    // A free entry. Write the following data for this entry: [FREE, num_].
    assert(isFreeSlot() && "invalid entry kind");
    s.writeInt<uint8_t>((uint8_t)EntryKind::Free);
    s.writeInt<uint32_t>(num_);
  }
}

void IdentifierTable::LookupEntry::deserialize(Deserializer &d) {
  isNotUniqued_ = d.readInt<uint8_t>();
  hash_ = d.readInt<uint32_t>();
  EntryKind kind = (EntryKind)d.readInt<uint8_t>();
  switch (kind) {
    case EntryKind::ASCII: {
      num_ = d.readInt<uint32_t>();
      asciiPtr_ = d.readCharStr();
      isUTF16_ = false;
      return;
    }
    case EntryKind::UFT16: {
      num_ = d.readInt<uint32_t>();
      utf16Ptr_ = d.readChar16Str();
      isUTF16_ = true;
      return;
    }
    case EntryKind::StrPrim: {
      num_ = d.readInt<uint32_t>();
      d.readRelocation(&strPrim_, RelocationKind::NativePointer);
      return;
    }
    case EntryKind::Free: {
      num_ = d.readInt<uint32_t>();
      return;
    }
  }
  llvm_unreachable("wrong EntryKind");
}
#endif

IdentifierTable::IdentifierTable() {
  hashTable_.setIdentifierTable(this);
}

CallResult<Handle<SymbolID>> IdentifierTable::getSymbolHandle(
    Runtime *runtime,
    UTF16Ref str,
    uint32_t hash) {
  auto cr = getOrCreateIdentifier(
      runtime, str, Runtime::makeNullHandle<StringPrimitive>(), hash);
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return runtime->makeHandle(*cr);
}

CallResult<Handle<SymbolID>> IdentifierTable::getSymbolHandle(
    Runtime *runtime,
    ASCIIRef str,
    uint32_t hash) {
  auto cr = getOrCreateIdentifier(
      runtime, str, Runtime::makeNullHandle<StringPrimitive>(), hash);
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return runtime->makeHandle(*cr);
}

SymbolID IdentifierTable::registerLazyIdentifier(ASCIIRef str) {
  return registerLazyIdentifierImpl(str, hermes::hashString(str));
}

SymbolID IdentifierTable::registerLazyIdentifier(ASCIIRef str, uint32_t hash) {
  return registerLazyIdentifierImpl(str, hash);
}

SymbolID IdentifierTable::registerLazyIdentifier(UTF16Ref str) {
  return registerLazyIdentifierImpl(str, hermes::hashString(str));
}

SymbolID IdentifierTable::registerLazyIdentifier(UTF16Ref str, uint32_t hash) {
  return registerLazyIdentifierImpl(str, hash);
}

CallResult<Handle<SymbolID>> IdentifierTable::getSymbolHandleFromPrimitive(
    Runtime *runtime,
    PseudoHandle<StringPrimitive> str) {
  assert(str && "null string primitive");
  if (str->isUniqued()) {
    // If the string was already uniqued, we can return directly.
    SymbolID id = str->getUniqueID();
    symbolReadBarrier(id.unsafeGetIndex());
    return runtime->makeHandle(id);
  }
  auto handle = runtime->makeHandle(std::move(str));
  // Force the string primitive to flatten if it's a rope.
  handle = StringPrimitive::ensureFlat(runtime, handle);
  auto cr = handle->isASCII()
      ? getOrCreateIdentifier(runtime, handle->castToASCIIRef(), handle)
      : getOrCreateIdentifier(runtime, handle->castToUTF16Ref(), handle);
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return runtime->makeHandle(*cr);
}

StringPrimitive *IdentifierTable::getStringPrim(Runtime *runtime, SymbolID id) {
  auto &entry = getLookupTableEntry(id);
  if (entry.isStringPrim()) {
    return entry.getStringPrimRef();
  }

  // This is a lazy identifier, since a string primitive is requested, we must
  // materialize it.
  return materializeLazyIdentifier(runtime, id);
}

StringView IdentifierTable::getStringView(Runtime *runtime, SymbolID id) const {
  auto &entry = getLookupTableEntry(id);
  if (entry.isStringPrim()) {
    // The const_cast is a mechanical requirement as it's not worth it to
    // add const version constructors for Handle.
    Handle<StringPrimitive> handle{
        runtime, const_cast<StringPrimitive *>(entry.getStringPrim())};
    // We know that this string already exists in the identifier table,
    // and hence it's safe to call the const version of getStringView.
    return StringPrimitive::createStringViewMustBeFlat(handle);
  }
  if (entry.isLazyASCII()) {
    return StringView(entry.getLazyASCIIRef());
  }
  return StringView(entry.getLazyUTF16Ref());
}

StringView IdentifierTable::getStringViewForDev(Runtime *runtime, SymbolID id)
    const {
  if (id == SymbolID::empty()) {
    assert(false && "getStringViewForDev on empty SymbolID");
    return "<<empty>>";
  }
  if (id == SymbolID::deleted()) {
    assert(false && "getStringViewForDev on deleted SymbolID");
    return "<<deleted>>";
  }
  if (id.isInvalid()) {
    assert(false && "getStringViewForDev on invalid SymbolID");
    return "<<invalid>>";
  }
  return getStringView(runtime, id);
}

std::string IdentifierTable::convertSymbolToUTF8(SymbolID id) {
  auto &vectorEntry = getLookupTableEntry(id);
  if (vectorEntry.isStringPrim()) {
    const StringPrimitive *stringPrim = vectorEntry.getStringPrim();
    llvh::SmallVector<char16_t, 16> tmp;
    stringPrim->appendUTF16String(tmp);
    std::string out;
    convertUTF16ToUTF8WithReplacements(out, UTF16Ref{tmp});
    return out;
  } else if (vectorEntry.isLazyASCII()) {
    auto asciiRef = vectorEntry.getLazyASCIIRef();
    return std::string{asciiRef.begin(), asciiRef.end()};
  } else if (vectorEntry.isLazyUTF16()) {
    auto ref = vectorEntry.getLazyUTF16Ref();
    std::string out;
    convertUTF16ToUTF8WithReplacements(out, ref);
    return out;
  } else {
    llvm_unreachable("Invalid symbol given");
  }
  // To avoid compiler warnings.
  return "";
}

void IdentifierTable::markIdentifiers(SlotAcceptor &acceptor, GC *gc) {
  for (auto &vectorEntry : lookupVector_) {
    if (!vectorEntry.isFreeSlot() && vectorEntry.isStringPrim()) {
#if defined(HERMESVM_GC_NONCONTIG_GENERATIONAL) || defined(HERMESVM_GC_HADES)
      assert(
          !gc->inYoungGen(vectorEntry.getStringPrimRef()) &&
          "Identifiers must be allocated in the old gen");
#endif
      acceptor.accept(
          reinterpret_cast<void *&>(vectorEntry.getStringPrimRef()));
    }
  }
}

void IdentifierTable::visitIdentifiers(
    const std::function<void(UTF16Ref, uint32_t)> &acceptor) {
  for (uint32_t i = 0; i < getSymbolsEnd(); ++i) {
    auto &vectorEntry = getLookupTableEntry(i);
    vm::SmallU16String<16> allocator;
    UTF16Ref ref;
    if (vectorEntry.isStringPrim()) {
      allocator.clear();
      auto stringPrim = vectorEntry.getStringPrim();
      stringPrim->appendUTF16String(allocator);
      ref = allocator.arrayRef();
    } else if (vectorEntry.isLazyASCII()) {
      allocator.clear();
      auto asciiRef = vectorEntry.getLazyASCIIRef();
      std::copy(
          asciiRef.begin(), asciiRef.end(), std::back_inserter(allocator));
      ref = allocator.arrayRef();
    } else if (vectorEntry.isLazyUTF16()) {
      ref = vectorEntry.getLazyUTF16Ref();
    } else {
      continue;
    }
    acceptor(ref, i);
  }
}

template <typename T, bool Unique>
CallResult<PseudoHandle<StringPrimitive>>
IdentifierTable::allocateDynamicString(
    Runtime *runtime,
    llvh::ArrayRef<T> str,
    Handle<StringPrimitive> primHandle) {
  size_t length = str.size();

  assert(
      (!primHandle || primHandle->isFlat()) && "StringPrimitive must be flat");

  GCScope gcScope(runtime);

  PseudoHandle<StringPrimitive> result;
  if (StringPrimitive::isExternalLength(length)) {
    if (LLVM_UNLIKELY(length > StringPrimitive::MAX_STRING_LENGTH)) {
      return runtime->raiseRangeError("String length exceeds limit");
    }
    std::basic_string<T> stdString(str.begin(), str.end());
    auto cr = ExternalStringPrimitive<T>::createLongLived(
        runtime, std::move(stdString));
    if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    result = createPseudoHandle(vmcast<StringPrimitive>(*cr));
  } else {
    void *mem = runtime->allocLongLived(
        DynamicStringPrimitive<T, Unique>::allocationSize((uint32_t)length));
    // Since we keep a raw pointer to mem, no more JS heap allocations after
    // this point.
    NoAllocScope _(runtime);
    if (primHandle) {
      str = primHandle->getStringRef<T>();
    }
    auto *tmp = new (mem) DynamicStringPrimitive<T, Unique>(runtime, length);
    std::copy(str.begin(), str.end(), tmp->getRawPointerForWrite());
    result = createPseudoHandle<StringPrimitive>(tmp);
  }

  return result;
}

void IdentifierTable::symbolReadBarrier(uint32_t id) {
  // Set the mark bool inside the symbol table entry so that this symbol isn't
  // garbage collected.
  // The reason this exists is that a Symbol can be retrieved via a string hash
  // that doesn't otherwise keep the symbol alive while in the middle of a GC.
  getLookupTableEntry(id).mark();
}

uint32_t IdentifierTable::allocIDAndInsert(
    uint32_t hashTableIndex,
    StringPrimitive *strPrim) {
  uint32_t nextId = allocNextID();
  SymbolID symbolId = SymbolID::unsafeCreate(nextId);
  assert(lookupVector_[nextId].isFreeSlot() && "Allocated a non-free slot");
  strPrim->convertToUniqued(symbolId);

  // We must assign strPrim to the lookupVector before inserting to
  // hashTable_, because inserting to hashTable_ could trigger a grow/rehash,
  // which requires accessing the newly inserted string primitive.
  new (&lookupVector_[nextId]) LookupEntry(strPrim);

  hashTable_.insert(hashTableIndex, symbolId);

  LLVM_DEBUG(
      llvh::dbgs() << "allocated symbol " << nextId << " '" << strPrim
                   << "'\n");

  return nextId;
}

template <typename T>
CallResult<SymbolID> IdentifierTable::getOrCreateIdentifier(
    Runtime *runtime,
    llvh::ArrayRef<T> str,
    Handle<StringPrimitive> maybeIncomingPrimHandle,
    uint32_t hash) {
  assert(
      !(maybeIncomingPrimHandle && maybeIncomingPrimHandle->isUniqued()) &&
      "Should not call getOrCreateIdentifier with a uniqued StrPrim");
  assert(
      (!maybeIncomingPrimHandle || maybeIncomingPrimHandle->isFlat()) &&
      "StringPrimitive must be flat");

  auto idx = hashTable_.lookupString(str, hash);
  if (hashTable_.isValid(idx)) {
    NoAllocScope scope{runtime};
    const auto id = hashTable_.get(idx);
    // Read barrier here because a symbol value is getting read out of the hash
    // map.
    symbolReadBarrier(id);
    return SymbolID::unsafeCreate(id);
  }

  // It is tempting here to check whether the incoming StringPrimitive can be
  // uniqued, and use it instead of allocating a new one. The problem is that
  // identifiers must always be allocated in "long-lived" memory, but we don't
  // (yet) have a way of checking whether that is the case. If in the future we
  // did get that GC API, the check would look something like this:
  // \code
  // if (maybeIncomingPrimHandle &&
  //     LLVM_UNLIKELY(maybeIncomingPrimHandle->canBeUniqued()) &&
  //     runtime->getHeap().isLongLived(*maybeIncomingPrimHandle)) {
  //   return SymbolID::unsafeCreate(
  //       allocIDAndInsert(idx, maybeIncomingPrimHandle.get()));
  // }
  // \endcode

  CallResult<PseudoHandle<StringPrimitive>> cr =
      allocateDynamicString(runtime, str, maybeIncomingPrimHandle);
  if (cr == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  // Allocate the id after we have performed memory allocations because a GC
  // would have freed id.
  return SymbolID::unsafeCreate(allocIDAndInsert(idx, cr->get()));
}

StringPrimitive *IdentifierTable::getExistingStringPrimitiveOrNull(
    Runtime *runtime,
    llvh::ArrayRef<char16_t> str) {
  auto idx = hashTable_.lookupString(str, hashString(str));
  if (!hashTable_.isValid(idx)) {
    return nullptr;
  }
  // Use a handle since getStringPrim may need to materialize the string.
  const auto id = hashTable_.get(idx);
  symbolReadBarrier(id);
  Handle<SymbolID> sym(runtime, SymbolID::unsafeCreate(id));
  return getStringPrim(runtime, *sym);
}

template <typename T>
SymbolID IdentifierTable::registerLazyIdentifierImpl(
    llvh::ArrayRef<T> str,
    uint32_t hash) {
  auto idx = hashTable_.lookupString(str, hash);
  if (hashTable_.isValid(idx)) {
    // If the string is already in the table, return it.
    const auto id = hashTable_.get(idx);
    symbolReadBarrier(id);
    return SymbolID::unsafeCreate(id);
  }
  uint32_t nextId = allocNextID();
  SymbolID symbolId = SymbolID::unsafeCreate(nextId);
  assert(lookupVector_[nextId].isFreeSlot() && "Allocated a non-free slot");
  new (&lookupVector_[nextId]) LookupEntry(str, hash);
  hashTable_.insert(idx, symbolId);
  LLVM_DEBUG(
      llvh::dbgs() << "Allocated lazy identifier: " << nextId << " " << str
                   << "\n");
  return symbolId;
}

StringPrimitive *IdentifierTable::materializeLazyIdentifier(
    Runtime *runtime,
    SymbolID id) {
  auto &entry = getLookupTableEntry(id);
  assert(
      (entry.isLazyASCII() || entry.isLazyUTF16()) && "identifier is not lazy");
  // This in theory can throw if running out of memory. However there are only
  // finite number of persistent identifiers, and we should always have enough
  // memory to hold them.
  PseudoHandle<StringPrimitive> strPrim = runtime->ignoreAllocationFailure(
      entry.isLazyASCII() ? allocateDynamicString(
                                runtime,
                                entry.getLazyASCIIRef(),
                                Runtime::makeNullHandle<StringPrimitive>())
                          : allocateDynamicString(
                                runtime,
                                entry.getLazyUTF16Ref(),
                                Runtime::makeNullHandle<StringPrimitive>()));
  if (id.isUniqued())
    strPrim->convertToUniqued(id);
  LLVM_DEBUG(llvh::dbgs() << "Materializing lazy identifier " << id << "\n");
  entry.materialize(strPrim.get());
  return strPrim.get();
}

void IdentifierTable::freeSymbol(uint32_t index) {
  assert(index < lookupVector_.size() && "Invalid symbol index to free");
  assert(
      lookupVector_[index].isNonLazyStringPrim() &&
      "The specified symbol cannot be freed");

  LLVM_DEBUG(
      llvh::dbgs() << "Freeing symbol index " << index << " '"
                   << lookupVector_[index].getStringPrim() << "'\n");

  if (LLVM_LIKELY(!lookupVector_[index].isNotUniqued())) {
    auto *strPrim =
        const_cast<StringPrimitive *>(lookupVector_[index].getStringPrim());
    strPrim->clearUniquedBit();
    hashTable_.remove(strPrim);
  }
  freeID(index);
}

uint32_t IdentifierTable::allocNextID() {
  // If the free list is empty, grow the array.
  if (firstFreeID_ == LookupEntry::FREE_LIST_END) {
    uint32_t newID = lookupVector_.size();
    if (LLVM_UNLIKELY(newID > LookupEntry::MAX_IDENTIFIER)) {
      hermes_fatal("Failed to allocate Identifier: IdentifierTable is full");
    }
    lookupVector_.emplace_back();
    LLVM_DEBUG(
        llvh::dbgs() << "Allocated new symbol id at end " << newID << "\n");
    // Don't need to tell the GC about this ID, it will assume any growth is
    // live.
    return newID;
  }

  // Allocate from the free list.
  uint32_t nextId = firstFreeID_;
  auto &entry = getLookupTableEntry(nextId);
  assert(entry.isFreeSlot() && "firstFreeID_ is not a free slot");
  firstFreeID_ = entry.getNextFreeSlot();
  LLVM_DEBUG(llvh::dbgs() << "Allocated freed symbol id " << nextId << "\n");
  return nextId;
}

void IdentifierTable::freeID(uint32_t id) {
  assert(
      lookupVector_[id].isStringPrim() &&
      "The specified symbol cannot be freed");

  // Add the freed index to the free list.
  lookupVector_[id].free(firstFreeID_);
  firstFreeID_ = id;
  LLVM_DEBUG(llvh::dbgs() << "Freeing ID " << id << "\n");
}

void IdentifierTable::unmarkSymbols() {
  for (auto &entry : lookupVector_) {
    entry.unmark();
  }
}

void IdentifierTable::freeUnmarkedSymbols(
    const std::vector<bool> &markedSymbols) {
  assert(
      markedSymbols.size() <= lookupVector_.size() &&
      "Size of markedSymbols must be less than the current lookupVector");
  for (uint32_t i = 0, e = markedSymbols.size(); i < e; ++i) {
    // We never free StringPrimitives that are materialized from a lazy
    // identifier.
    if (!markedSymbols[i] && !lookupVector_[i].isMarked() &&
        lookupVector_[i].isNonLazyStringPrim())
      freeSymbol(i);
    lookupVector_[i].unmark();
  }
  // Don't free any symbols that were newly created after the GC started.
  for (uint32_t i = markedSymbols.size(), e = lookupVector_.size(); i < e;
       ++i) {
    // Unmark any symbols allocated after the GC started.
    lookupVector_[i].unmark();
  }
}

#ifdef HERMES_SLOW_DEBUG
bool IdentifierTable::isSymbolLive(SymbolID id) const {
  auto &entry = getLookupTableEntry(id);
  // If the entry is not a free slot, then it is live.
  return !entry.isFreeSlot();
}
#endif

SymbolID IdentifierTable::createNotUniquedLazySymbol(ASCIIRef desc) {
  uint32_t nextID = allocNextID();
  new (&lookupVector_[nextID]) LookupEntry(desc, 0, true);
  return SymbolID::unsafeCreateNotUniqued(nextID);
}

CallResult<SymbolID> IdentifierTable::createNotUniquedSymbol(
    Runtime *runtime,
    Handle<StringPrimitive> desc) {
  uint32_t nextID = allocNextID();

#if defined(HERMESVM_GC_NONCONTIG_GENERATIONAL) || defined(HERMESVM_GC_HADES)
  if (runtime->getHeap().inYoungGen(desc.get())) {
    // Need to reallocate in the old gen if the description is in the young gen.
    CallResult<PseudoHandle<StringPrimitive>> longLivedStr = desc->isASCII()
        ? allocateDynamicString<char, /* Unique */ false>(
              runtime, desc->castToASCIIRef(), desc)
        : allocateDynamicString<char16_t, /* Unique */ false>(
              runtime, desc->castToUTF16Ref(), desc);
    // Since we keep a raw pointer to mem, no more JS heap allocations after
    // this point.
    NoAllocScope _(runtime);
    if (LLVM_UNLIKELY(longLivedStr == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    new (&lookupVector_[nextID]) LookupEntry(longLivedStr->get(), true);
  } else {
    // Description is already in the old gen, just point to it.
    new (&lookupVector_[nextID]) LookupEntry(*desc, true);
  }
#else
  // No concept of generations, so there's no need to allocLongLived.
  new (&lookupVector_[nextID]) LookupEntry(*desc, true);
#endif

  return SymbolID::unsafeCreateNotUniqued(nextID);
}

llvh::raw_ostream &operator<<(llvh::raw_ostream &OS, SymbolID symbolID) {
  if (symbolID.isInvalid())
    return OS << "SymbolID(INVALID)";
  else if (symbolID == SymbolID::deleted())
    return OS << "SymbolID(DELETED)";
  else
    return OS << "SymbolID("
              << (symbolID.isUniqued() ? "(Uniqued)" : "(Not Uniqued)")
              << symbolID.unsafeGetIndex() << ")";
}

#ifdef HERMESVM_SERIALIZE
void IdentifierTable::serialize(Serializer &s) {
  // Serialize uint32_t firstFreeID_;
  s.writeInt<uint32_t>(firstFreeID_);
  // Serialize IdentTableLookupVector lookupVector_;
  size_t size = lookupVector_.size();
  s.writeInt<uint32_t>(size);
  for (size_t i = 0; i < size; i++) {
    lookupVector_[i].serialize(s);
  }
  s.endObject(&lookupVector_);
  // Serialize detail::IdentifierHashTable hashTable_;
  hashTable_.serialize(s);
}

void IdentifierTable::deserialize(Deserializer &d) {
  firstFreeID_ = d.readInt<uint32_t>();
  size_t size = d.readInt<uint32_t>();
  lookupVector_.resize(size);
  for (auto &entry : lookupVector_) {
    entry.deserialize(d);
  }
  d.endObject(&lookupVector_);

  hashTable_.deserialize(d);
}
#endif

} // namespace vm
} // namespace hermes
