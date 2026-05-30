/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/IR/TypeContext.h"
#include "hermes/Support/HermesSafeMath.h"

#include "hermes/IR/IR.h"

#include "llvh/Support/raw_ostream.h"

#include <algorithm>
#include <cassert>

namespace hermes {

namespace {

/// \return true if \p k is a number kind (including subtypes).
bool isNumberKind(TypeKind k) {
  return k == TypeKind::Number || k == TypeKind::Int32 ||
      k == TypeKind::Uint32 || k == TypeKind::UInt31;
}

/// \return true if \p k is a refined object kind that carries payload data.
[[maybe_unused]] bool isRefinedKind(TypeKind k) {
  return k >= TypeKind::_RefinedFirst && k <= TypeKind::_RefinedLast;
}

/// \return true if \p k is an object kind (including refinements).
bool isObjectKind(TypeKind k) {
  switch (k) {
    case TypeKind::Object:
    case TypeKind::ClassInstance:
    case TypeKind::Array:
    case TypeKind::Tuple:
    case TypeKind::Function:
    case TypeKind::ExactObject:
      return true;
    default:
      return false;
  }
}

/// \return true if \p k is a primitive kind (including number subtypes).
bool isPrimitiveKind(TypeKind k) {
  switch (k) {
    case TypeKind::Number:
    case TypeKind::Int32:
    case TypeKind::Uint32:
    case TypeKind::UInt31:
    case TypeKind::String:
    case TypeKind::BigInt:
    case TypeKind::Null:
    case TypeKind::Undefined:
    case TypeKind::Boolean:
    case TypeKind::Symbol:
      return true;
    default:
      return false;
  }
}

/// \return true if \p k is a non-pointer kind (including number subtypes).
bool isNonPtrKind(TypeKind k) {
  switch (k) {
    case TypeKind::Number:
    case TypeKind::Int32:
    case TypeKind::Uint32:
    case TypeKind::UInt31:
    case TypeKind::Boolean:
    case TypeKind::Null:
    case TypeKind::Undefined:
      return true;
    default:
      return false;
  }
}

/// \return true if leaf kind \p a is a subtype of leaf kind \p b.
bool isLeafSubtype(TypeKind a, TypeKind b) {
  // Refined kinds carry payloads and require structural comparison that is
  // not yet implemented.
  hermes_assert(
      !isRefinedKind(a) && !isRefinedKind(b),
      "refined object kinds not yet supported in subtype check");
  if (a == b)
    return true;
  // Int32, Uint32, UInt31 are subtypes of Number.
  if (b == TypeKind::Number &&
      (a == TypeKind::Int32 || a == TypeKind::Uint32 || a == TypeKind::UInt31))
    return true;
  // UInt31 is a subtype of both Int32 and Uint32.
  if (a == TypeKind::UInt31 && (b == TypeKind::Int32 || b == TypeKind::Uint32))
    return true;
  // Object refinements are subtypes of Object.
  if (b == TypeKind::Object) {
    switch (a) {
      case TypeKind::ClassInstance:
      case TypeKind::Array:
      case TypeKind::Tuple:
      case TypeKind::Function:
      case TypeKind::ExactObject:
        return true;
      default:
        break;
    }
  }
  return false;
}

/// \return true if leaf kinds \p a and \p b have no values in common.
bool areLeafKindsDisjoint(TypeKind a, TypeKind b) {
  // If one is a subtype of the other, not disjoint.
  if (isLeafSubtype(a, b) || isLeafSubtype(b, a))
    return false;
  // Number family (Number, Int32, Uint32, UInt31) overlap pairwise.
  if (isNumberKind(a) && isNumberKind(b))
    return false;
  // All other pairs of different kinds are disjoint.
  return true;
}

/// \return the display name of a leaf kind. Matches the strings used by
/// the old Type::print().
llvh::StringRef kindName(TypeKind k) {
  switch (k) {
    case TypeKind::NoType:
      return "notype";
    case TypeKind::Empty:
      return "empty";
    case TypeKind::Uninit:
      return "uninit";
    case TypeKind::Undefined:
      return "undefined";
    case TypeKind::Null:
      return "null";
    case TypeKind::Boolean:
      return "boolean";
    case TypeKind::Number:
      return "number";
    case TypeKind::BigInt:
      return "bigint";
    case TypeKind::String:
      return "string";
    case TypeKind::Symbol:
      return "symbol";
    case TypeKind::Environment:
      return "environment";
    case TypeKind::PrivateName:
      return "privateName";
    case TypeKind::FunctionCode:
      return "functionCode";
    case TypeKind::Object:
      return "object";
    case TypeKind::Bits32:
      return "bits32";
    case TypeKind::Int32:
      return "int32";
    case TypeKind::Uint32:
      return "uint32";
    case TypeKind::UInt31:
      return "uint31";
    case TypeKind::ClassInstance:
      return "classInstance";
    case TypeKind::Array:
      return "array";
    case TypeKind::Tuple:
      return "tuple";
    case TypeKind::Function:
      return "function";
    case TypeKind::ExactObject:
      return "exactObject";
    case TypeKind::Union:
      return "union";
  }
  llvm_unreachable("unknown TypeKind");
}

} // anonymous namespace

TypeKind TypeContext::getKind(Type t) const {
  assert(t.id_ < entries_.size() && "Type ID out of range");
  return entries_[t.id_].kind;
}

llvh::ArrayRef<Type> TypeContext::getUnionArms(Type t) const {
  assert(t.id_ < entries_.size() && "Type ID out of range");
  const auto &entry = entries_[t.id_];
  assert(entry.kind == TypeKind::Union && "Not a union type");
  return llvh::ArrayRef<Type>(
      typeArrays_.data() + entry.union_.armOffset, entry.union_.armCount);
}

bool TypeContext::isNoType(Type t) const {
  return entries_[t.id_].kind == TypeKind::NoType;
}

template <typename Pred>
bool TypeContext::containsMatchingKind(uint32_t id, Pred pred) const {
  assert(id < entries_.size() && "Type ID out of range");
  const auto &entry = entries_[id];
  if (entry.kind != TypeKind::Union)
    return pred(entry.kind);
  auto arms = getUnionArms(Type{id});
  for (Type arm : arms) {
    assert(entries_[arm.id_].kind != TypeKind::Union && "Nested unions");
    if (pred(entries_[arm.id_].kind))
      return true;
  }
  return false;
}

template <typename Pred>
bool TypeContext::allMatchKind(uint32_t id, Pred pred) const {
  assert(id < entries_.size() && "Type ID out of range");
  const auto &entry = entries_[id];
  if (entry.kind == TypeKind::NoType)
    return false;
  if (entry.kind != TypeKind::Union)
    return pred(entry.kind);
  auto arms = getUnionArms(Type{id});
  for (Type arm : arms) {
    if (!pred(entries_[arm.id_].kind))
      return false;
  }
  return true;
}

unsigned TypeContext::countKinds(Type t) const {
  assert(t.id_ < entries_.size() && "Type ID out of range");
  const auto &entry = entries_[t.id_];
  if (entry.kind == TypeKind::NoType)
    return 0;
  if (entry.kind == TypeKind::Union)
    return entry.union_.armCount;
  return 1;
}

TypeKind TypeContext::getFirstKind(Type t) const {
  assert(t.id_ < entries_.size() && "Type ID out of range");
  const auto &entry = entries_[t.id_];
  if (entry.kind != TypeKind::Union)
    return entry.kind;
  // For unions, return the kind of the first arm.
  auto arms = getUnionArms(t);
  assert(!arms.empty() && "Union with no arms");
  return entries_[arms[0].id_].kind;
}

void TypeContext::print(llvh::raw_ostream &OS, Type t) const {
  assert(t.id_ < entries_.size() && "Type ID out of range");
  const auto &entry = entries_[t.id_];

  if (entry.kind == TypeKind::NoType) {
    OS << "notype";
    return;
  }

  if (entry.kind != TypeKind::Union) {
    OS << kindName(entry.kind);
    return;
  }

  // Union: check for the "any" shorthand. If this union is a superset of
  // AnyType, print "any" plus any extra arms (empty, uninit).
  if (isSubsetOf(Type{kAnyTypeId}, t)) {
    OS << "any";
    auto arms = getUnionArms(t);
    for (Type arm : arms) {
      TypeKind k = entries_[arm.id_].kind;
      // Skip kinds that are part of AnyType.
      if (isSubsetOf(arm, Type{kAnyTypeId}))
        continue;
      OS << '|' << kindName(k);
    }
    return;
  }

  // General union: pipe-separated arms.
  auto arms = getUnionArms(t);
  for (size_t i = 0; i < arms.size(); ++i) {
    if (i != 0)
      OS << '|';
    OS << kindName(entries_[arms[i].id_].kind);
  }
}

bool TypeContext::isKnownPrimitiveType(Type t) const {
  return isPrimitive(t) && countKinds(t) == 1;
}

bool TypeContext::canBeAny(Type t) const {
  return isSubsetOf(Type{kAnyTypeId}, t);
}

bool TypeContext::canBeType(Type a, Type b) const {
  return isSubsetOf(b, a);
}

bool TypeContext::isProperSubsetOf(Type a, Type b) const {
  return a != b && isSubsetOf(a, b);
}

llvh::iterator_range<Type::iterator> TypeContext::arms(Type t) const {
  return {t.begin(*this), t.end(*this)};
}

bool TypeContext::canBeNumber(Type t) const {
  // Fast path: well-known IDs.
  if (t.id_ == kNumberId || t.id_ == kAnyTypeId || t.id_ == kNumericId ||
      t.id_ == kAnyEmptyUninitId)
    return true;
  return containsMatchingKind(t.id_, isNumberKind);
}

bool TypeContext::canBeString(Type t) const {
  if (t.id_ == kStringId || t.id_ == kAnyTypeId || t.id_ == kAnyEmptyUninitId)
    return true;
  return containsMatchingKind(
      t.id_, [](TypeKind k) { return k == TypeKind::String; });
}

bool TypeContext::canBeObject(Type t) const {
  if (t.id_ == kObjectId || t.id_ == kAnyTypeId || t.id_ == kAnyEmptyUninitId)
    return true;
  return containsMatchingKind(t.id_, isObjectKind);
}

bool TypeContext::canBeNull(Type t) const {
  if (t.id_ == kNullId || t.id_ == kAnyTypeId || t.id_ == kNullOrUndefId ||
      t.id_ == kAnyEmptyUninitId)
    return true;
  return containsMatchingKind(
      t.id_, [](TypeKind k) { return k == TypeKind::Null; });
}

bool TypeContext::canBeUndefined(Type t) const {
  if (t.id_ == kUndefinedId || t.id_ == kAnyTypeId || t.id_ == kNullOrUndefId ||
      t.id_ == kAnyEmptyUninitId)
    return true;
  return containsMatchingKind(
      t.id_, [](TypeKind k) { return k == TypeKind::Undefined; });
}

bool TypeContext::canBeEmpty(Type t) const {
  if (t.id_ == kEmptyId || t.id_ == kAnyEmptyUninitId)
    return true;
  return containsMatchingKind(
      t.id_, [](TypeKind k) { return k == TypeKind::Empty; });
}

bool TypeContext::canBeUninit(Type t) const {
  if (t.id_ == kUninitId || t.id_ == kAnyEmptyUninitId)
    return true;
  return containsMatchingKind(
      t.id_, [](TypeKind k) { return k == TypeKind::Uninit; });
}

bool TypeContext::canBeBigInt(Type t) const {
  if (t.id_ == kBigIntId || t.id_ == kAnyTypeId || t.id_ == kNumericId ||
      t.id_ == kAnyEmptyUninitId)
    return true;
  return containsMatchingKind(
      t.id_, [](TypeKind k) { return k == TypeKind::BigInt; });
}

bool TypeContext::canBeBoolean(Type t) const {
  if (t.id_ == kBooleanId || t.id_ == kAnyTypeId || t.id_ == kAnyEmptyUninitId)
    return true;
  return containsMatchingKind(
      t.id_, [](TypeKind k) { return k == TypeKind::Boolean; });
}

bool TypeContext::canBeSymbol(Type t) const {
  if (t.id_ == kSymbolId || t.id_ == kAnyTypeId || t.id_ == kAnyEmptyUninitId)
    return true;
  return containsMatchingKind(
      t.id_, [](TypeKind k) { return k == TypeKind::Symbol; });
}

bool TypeContext::isPrimitive(Type t) const {
  return allMatchKind(t.id_, isPrimitiveKind);
}

bool TypeContext::canBePrimitive(Type t) const {
  if (t.id_ == kNoTypeId)
    return false;
  return containsMatchingKind(t.id_, isPrimitiveKind);
}

bool TypeContext::isNonPtr(Type t) const {
  return allMatchKind(t.id_, isNonPtrKind);
}

bool TypeContext::isSubsetOf(Type a, Type b) const {
  if (a.id_ == b.id_)
    return true;
  if (a.id_ == kNoTypeId)
    return true;
  if (b.id_ == kNoTypeId)
    return false;

  TypeKind aKind = entries_[a.id_].kind;
  TypeKind bKind = entries_[b.id_].kind;

  // Union on left: all arms must be subsets of b.
  if (aKind == TypeKind::Union) {
    for (Type arm : getUnionArms(a)) {
      if (!isSubsetOf(arm, b))
        return false;
    }
    return true;
  }

  // Leaf on left, union on right: a must be subset of some arm.
  if (bKind == TypeKind::Union) {
    for (Type arm : getUnionArms(b)) {
      if (isSubsetOf(a, arm))
        return true;
    }
    return false;
  }

  // Both leaf types.
  return isLeafSubtype(aKind, bKind);
}

bool TypeContext::areDisjoint(Type a, Type b) const {
  if (a.id_ == kNoTypeId || b.id_ == kNoTypeId)
    return true;
  if (a.id_ == b.id_)
    return false;

  TypeKind aKind = entries_[a.id_].kind;
  TypeKind bKind = entries_[b.id_].kind;

  // Union on left: disjoint iff all arms are disjoint from b.
  if (aKind == TypeKind::Union) {
    for (Type arm : getUnionArms(a)) {
      if (!areDisjoint(arm, b))
        return false;
    }
    return true;
  }

  // Union on right: disjoint iff a is disjoint from all arms.
  if (bKind == TypeKind::Union) {
    for (Type arm : getUnionArms(b)) {
      if (!areDisjoint(a, arm))
        return false;
    }
    return true;
  }

  // Both leaf types.
  return areLeafKindsDisjoint(aKind, bKind);
}

uint32_t TypeContext::createUnionImpl(uint32_t a, uint32_t b) {
  // Collect sorted arm ID lists from both operands. Leaf types produce
  // single-element sequences; union arms are already sorted.
  auto collectArms = [this](uint32_t id, llvh::SmallVectorImpl<uint32_t> &out) {
    if (entries_[id].kind == TypeKind::Union) {
      for (Type arm : getUnionArms(Type{id}))
        out.push_back(arm.id_);
    } else {
      out.push_back(id);
    }
  };
  llvh::SmallVector<uint32_t, 8> aArms, bArms;
  collectArms(a, aArms);
  collectArms(b, bArms);

  // Remove cross-subsumed arms. Arms within each input are already
  // canonical (no internal subsumption), so we only check across inputs.
  auto isSubsumedBy = [this](uint32_t id, llvh::ArrayRef<uint32_t> others) {
    TypeKind k = entries_[id].kind;
    for (uint32_t o : others) {
      if (o != id && isLeafSubtype(k, entries_[o].kind))
        return true;
    }
    return false;
  };

  llvh::SmallVector<uint32_t, 8> aFiltered, bFiltered;
  for (uint32_t id : aArms) {
    if (!isSubsumedBy(id, bArms))
      aFiltered.push_back(id);
  }
  for (uint32_t id : bArms) {
    if (!isSubsumedBy(id, aArms))
      bFiltered.push_back(id);
  }

  // Merge both sorted sequences, skipping duplicates.
  llvh::SmallVector<uint32_t, 16> arms;
  size_t ai = 0, bi = 0;
  while (ai < aFiltered.size() && bi < bFiltered.size()) {
    if (aFiltered[ai] < bFiltered[bi]) {
      arms.push_back(aFiltered[ai++]);
    } else if (aFiltered[ai] > bFiltered[bi]) {
      arms.push_back(bFiltered[bi++]);
    } else {
      arms.push_back(aFiltered[ai]);
      ++ai;
      ++bi;
    }
  }
  while (ai < aFiltered.size())
    arms.push_back(aFiltered[ai++]);
  while (bi < bFiltered.size())
    arms.push_back(bFiltered[bi++]);

  if (arms.empty())
    return kNoTypeId;
  if (arms.size() == 1)
    return arms[0];

  // Intern: check if this union already exists.
  UnionInternKey key(arms);
  auto it = internTable_.find(key);
  if (it != internTable_.end())
    return it->second;

  // Create new union entry.
  uint32_t id = addUnionEntry(arms);
  internTable_[std::move(key)] = id;
  return id;
}

Type TypeContext::unionTy(Type a, Type b) {
  if (a.id_ == b.id_)
    return a;
  if (a.id_ == kNoTypeId)
    return b;
  if (b.id_ == kNoTypeId)
    return a;
  if (isSubsetOf(a, b))
    return b;
  if (isSubsetOf(b, a))
    return a;
  return Type{createUnionImpl(a.id_, b.id_)};
}

uint32_t TypeContext::intersectLeafTy(uint32_t a, uint32_t b) const {
  assert(a != kNoTypeId && b != kNoTypeId && "NoType must be handled first");
  TypeKind aKind = entries_[a].kind;
  TypeKind bKind = entries_[b].kind;
  assert(aKind != TypeKind::Union && bKind != TypeKind::Union);

  if (isLeafSubtype(aKind, bKind))
    return a;
  if (isLeafSubtype(bKind, aKind))
    return b;

  // The only non-subset overlap in the number family is Int32 ∩ Uint32 =
  // UInt31. All other number-family pairs are handled by the subtype checks
  // above.
  if (isNumberKind(aKind) && isNumberKind(bKind))
    return kUInt31Id;
  return kNoTypeId;
}

uint32_t TypeContext::createUnionFromLeafArms(llvh::ArrayRef<uint32_t> arms) {
  llvh::SmallVector<uint32_t, 16> normalized;
  normalized.reserve(arms.size());
  for (uint32_t arm : arms) {
    if (arm != kNoTypeId)
      normalized.push_back(arm);
  }

  if (normalized.empty())
    return kNoTypeId;

  std::sort(normalized.begin(), normalized.end());
  normalized.erase(
      std::unique(normalized.begin(), normalized.end()), normalized.end());

  llvh::SmallVector<uint32_t, 16> filtered;
  filtered.reserve(normalized.size());
  for (size_t i = 0; i < normalized.size(); ++i) {
    TypeKind candidate = entries_[normalized[i]].kind;
    bool subsumed = false;
    for (size_t j = 0; j < normalized.size(); ++j) {
      if (i == j)
        continue;
      if (isLeafSubtype(candidate, entries_[normalized[j]].kind)) {
        subsumed = true;
        break;
      }
    }
    if (!subsumed)
      filtered.push_back(normalized[i]);
  }

  if (filtered.empty())
    return kNoTypeId;
  if (filtered.size() == 1)
    return filtered[0];

  UnionInternKey key(filtered);
  auto it = internTable_.find(key);
  if (it != internTable_.end())
    return it->second;

  uint32_t id = addUnionEntry(filtered);
  internTable_[std::move(key)] = id;
  return id;
}

Type TypeContext::intersectTy(Type a, Type b) {
  if (a.id_ == b.id_)
    return a;
  if (a.id_ == kNoTypeId || b.id_ == kNoTypeId)
    return Type{kNoTypeId};
  TypeKind aKind = entries_[a.id_].kind;
  TypeKind bKind = entries_[b.id_].kind;

  if (aKind != TypeKind::Union && bKind != TypeKind::Union)
    return Type{intersectLeafTy(a.id_, b.id_)};

  llvh::SmallVector<uint32_t, 16> intersections;

  if (aKind == TypeKind::Union && bKind == TypeKind::Union) {
    auto aArms = getUnionArms(a);
    auto bArms = getUnionArms(b);
    intersections.reserve(aArms.size() * bArms.size());
    for (Type aArm : aArms) {
      for (Type bArm : bArms) {
        uint32_t intersection = intersectLeafTy(aArm.id_, bArm.id_);
        if (intersection != kNoTypeId)
          intersections.push_back(intersection);
      }
    }
    return Type{createUnionFromLeafArms(intersections)};
  }

  auto unionArms = aKind == TypeKind::Union ? getUnionArms(a) : getUnionArms(b);
  uint32_t leaf = aKind == TypeKind::Union ? b.id_ : a.id_;
  intersections.reserve(unionArms.size());
  for (Type arm : unionArms) {
    uint32_t intersection = intersectLeafTy(arm.id_, leaf);
    if (intersection != kNoTypeId)
      intersections.push_back(intersection);
  }
  return Type{createUnionFromLeafArms(intersections)};
}

Type TypeContext::subtractTy(Type a, Type b) {
  if (a.id_ == kNoTypeId)
    return Type{kNoTypeId};
  if (b.id_ == kNoTypeId)
    return a;
  if (isSubsetOf(a, b))
    return Type{kNoTypeId};
  if (areDisjoint(a, b))
    return a;

  // Distribute over unions in a. Copy arms first because unionTy may
  // reallocate typeArrays_, invalidating the ArrayRef from getUnionArms.
  if (entries_[a.id_].kind == TypeKind::Union) {
    llvh::SmallVector<Type, 16> arms;
    for (Type arm : getUnionArms(a))
      arms.push_back(arm);
    Type result{kNoTypeId};
    for (Type arm : arms)
      result = unionTy(result, subtractTy(arm, b));
    return result;
  }

  // a is a leaf, not subset of b, not disjoint from b.
  // Conservative: return a.
  return a;
}

uint32_t TypeContext::addUnionEntry(llvh::ArrayRef<uint32_t> arms) {
  assert(arms.size() >= 2 && "Union must have at least 2 arms");
  uint32_t offset = safePossiblyNarrowingCast<uint32_t>(
      typeArrays_.size(), "type array offset overflow");
  for (uint32_t armId : arms)
    typeArrays_.push_back(Type(armId));
  uint32_t id = entries_.size();
  entries_.push_back(
      TypeEntry::createUnion(
          offset,
          safePossiblyNarrowingCast<uint16_t>(
              arms.size(), "too many union arms")));
  return id;
}

TypeContext::TypeContext() {
  // Reserve space for well-known entries.
  entries_.reserve(kFirstDynamicId);

  // Pre-allocate leaf type entries. The order must match the well-known ID
  // constants exactly.

  // 0: NoType
  entries_.push_back(TypeEntry::createLeaf(TypeKind::NoType));
  assert(entries_.size() - 1 == kNoTypeId);

  // 1: Empty
  entries_.push_back(TypeEntry::createLeaf(TypeKind::Empty));
  assert(entries_.size() - 1 == kEmptyId);

  // 2: Uninit
  entries_.push_back(TypeEntry::createLeaf(TypeKind::Uninit));
  assert(entries_.size() - 1 == kUninitId);

  // 3: Undefined
  entries_.push_back(TypeEntry::createLeaf(TypeKind::Undefined));
  assert(entries_.size() - 1 == kUndefinedId);

  // 4: Null
  entries_.push_back(TypeEntry::createLeaf(TypeKind::Null));
  assert(entries_.size() - 1 == kNullId);

  // 5: Boolean
  entries_.push_back(TypeEntry::createLeaf(TypeKind::Boolean));
  assert(entries_.size() - 1 == kBooleanId);

  // 6: String
  entries_.push_back(TypeEntry::createLeaf(TypeKind::String));
  assert(entries_.size() - 1 == kStringId);

  // 7: Number
  entries_.push_back(TypeEntry::createLeaf(TypeKind::Number));
  assert(entries_.size() - 1 == kNumberId);

  // 8: BigInt
  entries_.push_back(TypeEntry::createLeaf(TypeKind::BigInt));
  assert(entries_.size() - 1 == kBigIntId);

  // 9: Symbol
  entries_.push_back(TypeEntry::createLeaf(TypeKind::Symbol));
  assert(entries_.size() - 1 == kSymbolId);

  // 10: Environment
  entries_.push_back(TypeEntry::createLeaf(TypeKind::Environment));
  assert(entries_.size() - 1 == kEnvironmentId);

  // 11: PrivateName
  entries_.push_back(TypeEntry::createLeaf(TypeKind::PrivateName));
  assert(entries_.size() - 1 == kPrivateNameId);

  // 12: FunctionCode
  entries_.push_back(TypeEntry::createLeaf(TypeKind::FunctionCode));
  assert(entries_.size() - 1 == kFunctionCodeId);

  // 13: Object
  entries_.push_back(TypeEntry::createLeaf(TypeKind::Object));
  assert(entries_.size() - 1 == kObjectId);

  // 14: Bits32
  entries_.push_back(TypeEntry::createLeaf(TypeKind::Bits32));
  assert(entries_.size() - 1 == kBits32Id);

  // 15: Int32
  entries_.push_back(TypeEntry::createLeaf(TypeKind::Int32));
  assert(entries_.size() - 1 == kInt32Id);

  // 16: Uint32
  entries_.push_back(TypeEntry::createLeaf(TypeKind::Uint32));
  assert(entries_.size() - 1 == kUint32Id);

  // 17: UInt31 (Int32 ∩ Uint32)
  entries_.push_back(TypeEntry::createLeaf(TypeKind::UInt31));
  assert(entries_.size() - 1 == kUInt31Id);

  // 18: AnyType — union of all JS-observable types (matching TYPE_ANY_MASK).
  // Primitives + Object, excludes Empty, Uninit, Environment, PrivateName,
  // FunctionCode, Bits32.
  {
    uint32_t anyArms[] = {
        kUndefinedId,
        kNullId,
        kBooleanId,
        kStringId,
        kNumberId,
        kBigIntId,
        kSymbolId,
        kObjectId};
    uint32_t id = addUnionEntry(anyArms);
    (void)id;
    assert(id == kAnyTypeId);
  }

  // 19: Numeric — Number | BigInt.
  {
    uint32_t numericArms[] = {kNumberId, kBigIntId};
    uint32_t id = addUnionEntry(numericArms);
    (void)id;
    assert(id == kNumericId);
  }

  // 20: AnyEmptyUninit — any | Empty | Uninit.
  // This is a union of all the AnyType arms plus Empty and Uninit.
  {
    uint32_t aeuArms[] = {
        kEmptyId,
        kUninitId,
        kUndefinedId,
        kNullId,
        kBooleanId,
        kStringId,
        kNumberId,
        kBigIntId,
        kSymbolId,
        kObjectId};
    uint32_t id = addUnionEntry(aeuArms);
    (void)id;
    assert(id == kAnyEmptyUninitId);
  }

  // 21: NullOrUndef — Null | Undefined.
  {
    uint32_t nuArms[] = {kUndefinedId, kNullId};
    uint32_t id = addUnionEntry(nuArms);
    (void)id;
    assert(id == kNullOrUndefId);
  }

  // 22: StringOrSymbol — String | Symbol.
  {
    uint32_t arms[] = {kStringId, kSymbolId};
    uint32_t id = addUnionEntry(arms);
    (void)id;
    assert(id == kStringOrSymbolId);
  }

  // 23: EmptyOrUninit — Empty | Uninit.
  {
    uint32_t arms[] = {kEmptyId, kUninitId};
    uint32_t id = addUnionEntry(arms);
    (void)id;
    assert(id == kEmptyOrUninitId);
  }

  // 24: ObjectOrNull — Null | Object (sorted by ID).
  {
    uint32_t arms[] = {kNullId, kObjectId};
    uint32_t id = addUnionEntry(arms);
    (void)id;
    assert(id == kObjectOrNullId);
  }

  // 25: ObjectOrUndef — Undefined | Object (sorted by ID).
  {
    uint32_t arms[] = {kUndefinedId, kObjectId};
    uint32_t id = addUnionEntry(arms);
    (void)id;
    assert(id == kObjectOrUndefId);
  }

  // Pre-populate intern table with well-known unions.
  // TypeContext is a friend of Type, so we can access Type::id_.
  for (uint32_t id = _kFirstUnionId; id != _kLastUnionId; ++id) {
    auto arms = getUnionArms(Type{id});
    UnionInternKey key;
    key.arms.reserve(arms.size());
    for (Type t : arms)
      key.arms.push_back(t.id_);
    internTable_[std::move(key)] = id;
  }

  // Pad remaining entries up to kFirstDynamicId with NoType placeholders.
  while (entries_.size() < kFirstDynamicId) {
    entries_.push_back(TypeEntry::createLeaf(TypeKind::NoType));
  }
  assert(entries_.size() == kFirstDynamicId);
}

// All Type operations now go through TypeContext explicitly. The only
// out-of-line Type method that remains here is the trivial print fallback,
// which is used by the operator<<(raw_ostream &, Type) overload in IR.cpp
// for diagnostic streams that do not have a TypeContext available.

void Type::print(llvh::raw_ostream &OS) const {
  OS << "type#" << id_;
}

Type Type::iterator::operator*() const {
  if (ctx_->entries_[type_.id_].kind != TypeKind::Union) {
    // Non-union: yield the type itself.
    return type_;
  }
  return ctx_->getUnionArms(type_)[index_];
}

Type::iterator Type::begin(const TypeContext &ctx) const {
  if (isNoType())
    return end(ctx);
  return iterator(ctx, *this, 0);
}

Type::iterator Type::end(const TypeContext &ctx) const {
  if (ctx.entries_[id_].kind == TypeKind::Union)
    return iterator(ctx, *this, ctx.getUnionArms(*this).size());
  return iterator(ctx, *this, isNoType() ? 0 : 1);
}

} // namespace hermes
