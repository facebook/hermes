/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
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

#include "llvm/Support/Debug.h"

namespace hermes {
namespace vm {

IdentifierTable::LookupEntry::LookupEntry(
    StringPrimitive *str,
    bool isNotUniqued)
    : strPrim_(str),
      isUTF16_(false),
      isNotUniqued_(isNotUniqued),
      num_(NON_LAZY_STRING_PRIM_TAG) {
  assert(str && "Invalid string primitive pointer");
  llvm::SmallVector<char16_t, 32> storage{};
  str->copyUTF16String(storage);
  hash_ = hermes::hashString(llvm::ArrayRef<char16_t>(storage));
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
      break;
    }
    case EntryKind::UFT16: {
      num_ = d.readInt<uint32_t>();
      utf16Ptr_ = d.readChar16Str();
      isUTF16_ = true;
      break;
    }
    case EntryKind::StrPrim: {
      num_ = d.readInt<uint32_t>();
      d.readRelocation(&strPrim_, RelocationKind::NativePointer);
      break;
    }
    case EntryKind::Free: {
      num_ = d.readInt<uint32_t>();
    }
    default:
      llvm_unreachable("wrong EntryKind");
  }
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
      runtime, str, runtime->makeNullHandle<StringPrimitive>(), hash);
  if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return runtime->makeHandle(*cr);
}

CallResult<Handle<SymbolID>> IdentifierTable::getSymbolHandle(
    Runtime *runtime,
    ASCIIRef str,
    uint32_t hash) {
  auto cr = getOrCreateIdentifier(
      runtime, str, runtime->makeNullHandle<StringPrimitive>(), hash);
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
    return runtime->makeHandle(str->getUniqueID());
  }
  auto handle = toHandle(runtime, std::move(str));
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

void IdentifierTable::debugGetSymbolName(
    Runtime *runtime,
    SymbolID id,
    llvm::SmallVectorImpl<char> &buffer) {
  GCScope gcScope{runtime};
  StringView view = getStringView(runtime, id);
  buffer.reserve(buffer.size() + view.length());
  for (char16_t ch : view) {
    if (ch > 32 && ch < 128)
      buffer.push_back((char)ch);
    else {
      char cvtbuf[8];
      ::snprintf(cvtbuf, sizeof(cvtbuf), "\\u%04x", ch);
      buffer.append(cvtbuf, cvtbuf + 6);
    }
  }
}

void IdentifierTable::markIdentifiers(SlotAcceptorWithNames &acceptor, GC *gc) {
  for (auto &vectorEntry : lookupVector_) {
    if (!vectorEntry.isFreeSlot() && vectorEntry.isStringPrim()) {
#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
      assert(
          !gc->inYoungGen(vectorEntry.getStringPrimRef()) &&
          "Identifiers must be allocated in the old gen");
#endif
      acceptor.accept(
          reinterpret_cast<void *&>(vectorEntry.getStringPrimRef()),
          "IdentifierTable_Normal_Identifier");
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
      stringPrim->copyUTF16String(allocator);
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
    llvm::ArrayRef<T> str,
    Handle<StringPrimitive> primHandle,
    SymbolID strId) {
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
        runtime, std::move(stdString), Unique ? strId : SymbolID::empty());
    if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    result = createPseudoHandle(vmcast<StringPrimitive>(*cr));
  } else {
    void *mem = runtime->allocLongLived(
        DynamicStringPrimitive<T, Unique>::allocationSize((uint32_t)length));
    // Since we keep a raw pointer to mem, no more JS heap allocations after
    // this point.
    if (primHandle) {
      str = primHandle->getStringRef<T>();
    }
    auto *tmp =
        new (mem) DynamicStringPrimitive<T, Unique>(runtime, length, strId);
    std::copy(str.begin(), str.end(), tmp->getRawPointerForWrite());
    result = createPseudoHandle<StringPrimitive>(tmp);
  }

  return std::move(result);
}

uint32_t IdentifierTable::allocIDAndInsert(
    uint32_t hashTableIndex,
    StringPrimitive *strPrim) {
  uint32_t nextId = allocNextID();
  SymbolID symbolId = SymbolID::unsafeCreate(nextId);
  assert(lookupVector_[nextId].isFreeSlot() && "Allocated a non-free slot");
  strPrim->updateUniqueID(symbolId);

  // We must assign strPrim to the lookupVector before inserting to
  // hashTable_, because inserting to hashTable_ could trigger a grow/rehash,
  // which requires accessing the newly inserted string primitive.
  new (&lookupVector_[nextId]) LookupEntry(strPrim);

  hashTable_.insert(hashTableIndex, symbolId);

  LLVM_DEBUG(
      llvm::dbgs() << "allocated symbol " << nextId << " '" << strPrim
                   << "'\n");

  return nextId;
}

template <typename T>
CallResult<SymbolID> IdentifierTable::getOrCreateIdentifier(
    Runtime *runtime,
    llvm::ArrayRef<T> str,
    Handle<StringPrimitive> maybeIncomingPrimHandle,
    uint32_t hash) {
  assert(
      !(maybeIncomingPrimHandle && maybeIncomingPrimHandle->isUniqued()) &&
      "Should not call getOrCreateIdentifier with a uniqued StrPrim");
  assert(
      (!maybeIncomingPrimHandle || maybeIncomingPrimHandle->isFlat()) &&
      "StringPrimitive must be flat");

  auto idx = hashTable_.lookupString(str, hash);
  if (hashTable_.at(idx).isValid()) {
    return SymbolID::unsafeCreate(hashTable_.at(idx).index);
  }

  CallResult<PseudoHandle<StringPrimitive>> cr = allocateDynamicString(
      runtime, str, maybeIncomingPrimHandle, SymbolID::empty());
  if (cr == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  // Allocate the id after we have performed memory allocations because a GC
  // would have freed id.
  return SymbolID::unsafeCreate(allocIDAndInsert(idx, cr->get()));
}

template <typename T>
SymbolID IdentifierTable::registerLazyIdentifierImpl(
    llvm::ArrayRef<T> str,
    uint32_t hash) {
  auto idx = hashTable_.lookupString(str, hash);
  if (hashTable_.at(idx).isValid()) {
    // If the string is already in the table, return it.
    return SymbolID::unsafeCreate(hashTable_.at(idx).index);
  }
  uint32_t nextId = allocNextID();
  SymbolID symbolId = SymbolID::unsafeCreate(nextId);
  assert(lookupVector_[nextId].isFreeSlot() && "Allocated a non-free slot");
  new (&lookupVector_[nextId]) LookupEntry(str, hash);
  hashTable_.insert(idx, symbolId);
  LLVM_DEBUG(
      llvm::dbgs() << "Allocated lazy identifier: " << nextId << " " << str
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
                                runtime->makeNullHandle<StringPrimitive>(),
                                id)
                          : allocateDynamicString(
                                runtime,
                                entry.getLazyUTF16Ref(),
                                runtime->makeNullHandle<StringPrimitive>(),
                                id));
  LLVM_DEBUG(llvm::dbgs() << "Materializing lazy identifier " << id << "\n");
  entry.materialize(strPrim.get());
  return strPrim.get();
}

void IdentifierTable::freeSymbol(uint32_t index) {
  assert(index < lookupVector_.size() && "Invalid symbol index to free");
  assert(
      lookupVector_[index].isNonLazyStringPrim() &&
      "The specified symbol cannot be freed");

  LLVM_DEBUG(
      llvm::dbgs() << "Freeing symbol index " << index << " '"
                   << lookupVector_[index].getStringPrim() << "'\n");

  if (LLVM_LIKELY(!lookupVector_[index].isNotUniqued())) {
    hashTable_.remove(lookupVector_[index].getStringPrim());
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
        llvm::dbgs() << "Allocated new symbol id at end " << newID << "\n");
    return newID;
  }

  // Allocate from the free list.
  uint32_t nextId = (uint32_t)firstFreeID_;
  const auto &entry = getLookupTableEntry(nextId);
  assert(entry.isFreeSlot() && "firstFreeID_ is not a free slot");
  firstFreeID_ = entry.getNextFreeSlot();
  LLVM_DEBUG(llvm::dbgs() << "Allocated freed symbol id " << nextId << "\n");
  return nextId;
}

void IdentifierTable::freeID(uint32_t id) {
  assert(
      lookupVector_[id].isStringPrim() &&
      "The specified symbol cannot be freed");

  // Add the freed index to the free list.
  lookupVector_[id].free(firstFreeID_);
  firstFreeID_ = id;
  LLVM_DEBUG(llvm::dbgs() << "Freeing ID " << id << "\n");
}

void IdentifierTable::freeUnmarkedSymbols(
    const std::vector<bool> &markedSymbols) {
  assert(
      markedSymbols.size() == lookupVector_.size() &&
      "Size of markedSymbols does not match lookupVector_");
  for (uint32_t i = 0, e = markedSymbols.size(); i < e; ++i) {
    // We never free StringPrimitives that are materialized from a lazy
    // identifier.
    if (!markedSymbols[i] && lookupVector_[i].isNonLazyStringPrim())
      freeSymbol(i);
  }
}

SymbolID IdentifierTable::createNotUniquedLazySymbol(ASCIIRef desc) {
  uint32_t nextID = allocNextID();
  new (&lookupVector_[nextID]) LookupEntry(desc, 0, true);
  return SymbolID::unsafeCreateNotUniqued(nextID);
}

CallResult<SymbolID> IdentifierTable::createNotUniquedSymbol(
    Runtime *runtime,
    Handle<StringPrimitive> desc) {
  uint32_t nextID = allocNextID();

#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
  if (runtime->getHeap().inYoungGen(desc.get())) {
    // Need to reallocate in the old gen if the description is in the young gen.
    CallResult<PseudoHandle<StringPrimitive>> longLivedStr = desc->isASCII()
        ? allocateDynamicString<char, /* Unique */ false>(
              runtime, desc->castToASCIIRef(), desc, SymbolID::empty())
        : allocateDynamicString<char16_t, /* Unique */ false>(
              runtime, desc->castToUTF16Ref(), desc, SymbolID::empty());
    // Since we keep a raw pointer to mem, no more JS heap allocations after
    // this point.
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

llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, SymbolID symbolID) {
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
