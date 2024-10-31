/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Sema/FlowContext.h"

#include "llvh/ADT/Hashing.h"
#include "llvh/ADT/STLExtras.h"
#include "llvh/ADT/ScopeExit.h"

namespace hermes {
namespace flow {

namespace {
/// Compare two sequences of types lexicographically.
template <typename It, typename Comp>
int lexicographicalComparison(
    It first1,
    It last1,
    It first2,
    It last2,
    Comp compare) {
  for (; first1 != last1; ++first1, ++first2) {
    // If sequence 2 is a prefix of sequence 1, sequence 1 is greater.
    if (first2 == last2)
      return 1;
    if (auto tmpRes = compare(*first1, *first2))
      return tmpRes;
  }

  return first2 == last2 ? 0 : -1;
}

template <typename It, typename F>
llvh::hash_code hashTypes(It first, It last, F hash) {
  llvh::hash_code hashCode = 0;
  for (; first != last; ++first) {
    hashCode = llvh::hash_combine(hashCode, hash(*first));
  }
  return hashCode;
}

/// A helper for comparing bools, returning -1, 0, +1.
inline int cmpHelperBool(bool a, bool b) {
  return (int)a - (int)b;
}

/// A helper for comparing nullable Type pointers, returning -1, 0, +1.
int cmpHelper(Type *a, Type *b) {
  return a ? b ? a->info->compare(b->info) : 1 : b ? -1 : 0;
}

/// Compare recursive lists of types on the left and right.
/// Runs in quadratic time, checks for each pair of (left, right) types.
/// \return true if every element of \p left has a corresponding entry on \p
///   right and vice versa.
static bool listsMatchSlow(
    llvh::ArrayRef<Type *> left,
    llvh::ArrayRef<Type *> right,
    TypeInfo::CompareState &state) {
  // matched[i] indicates whether right[i] has a match on the left.
  llvh::BitVector matched{};
  matched.resize(right.size(), false);

  // First check all left entries for matches on the right, marking the
  // matched bit.
  for (Type *l : left) {
    bool found = false;
    for (size_t j = 0, e = right.size(); j < e; ++j) {
      Type *r = right[j];
      if (l->info->equals(r->info, state)) {
        matched.set(j);
        found = true;
        // Don't break here, we want to check all entries on the right.
      }
    }
    // Missed one entry on the left, no point continuing.
    if (!found)
      return false;
  }

  // Only return true if all the right entries have been matched.
  return matched.all();
}

} // anonymous namespace

llvh::StringRef TypeInfo::getKindName() const {
  switch (kind_) {
    case TypeKind::Void:
      return "void";
    case TypeKind::Null:
      return "null";
    case TypeKind::Boolean:
      return "boolean";
    case TypeKind::String:
      return "string";
    case TypeKind::CPtr:
      return "c_ptr";
    case TypeKind::Number:
      return "number";
    case TypeKind::BigInt:
      return "bigint";
    case TypeKind::Any:
      return "any";
    case TypeKind::Mixed:
      return "mixed";
    case TypeKind::Union:
      return "union";
    case TypeKind::TypedFunction:
      return "function";
    case TypeKind::NativeFunction:
      return "native function";
    case TypeKind::UntypedFunction:
      return "untyped function";
    case TypeKind::Class:
      return "class";
    case TypeKind::ClassConstructor:
      return "class constructor";
    case TypeKind::Array:
      return "array";
    case TypeKind::Tuple:
      return "tuple";
    case TypeKind::ExactObject:
      return "object";
    case TypeKind::Generic:
      return "generic";
  }
  llvm_unreachable("invalid TypeKind");
}

int TypeInfo::compare(const TypeInfo *other) const {
  CompareState state{};
  return compare(other, state);
}
int TypeInfo::compare(const TypeInfo *other, CompareState &state) const {
  if (other == this)
    return 0;
  if (kind_ < other->kind_)
    return -1;
  else if (kind_ > other->kind_)
    return 1;

  auto cached = state.cache.find({this, other});
  if (cached != state.cache.end()) {
    return cached->second;
  }

  if (!state.visited.insert({this, other})) {
    // Failed to insert, already visited this pair.
    // Because we haven't diverged yet, they must be equal.
    return 0;
  }

  // Remove this entry in the visited list after we finish this call.
  auto popOnExit =
      llvh::make_scope_exit([&state]() { state.visited.pop_back(); });

  int result;
  switch (kind_) {
#define _HERMES_SEMA_FLOW_DEFKIND(name)                           \
  case TypeKind::name:                                            \
    result = static_cast<const name##Type *>(this)->_compareImpl( \
        static_cast<const name##Type *>(other), state);           \
    break;
    _HERMES_SEMA_FLOW_SINGLETONS
    _HERMES_SEMA_FLOW_COMPLEX_TYPES
#undef _HERMES_SEMA_FLOW_DEFKIND
    default:
      hermes_fatal("invalid TypeKind");
  }
  state.cache[{this, other}] = result;
  return result;
}

bool TypeInfo::equals(const TypeInfo *other) const {
  CompareState state{};
  return equals(other, state);
}
bool TypeInfo::equals(const TypeInfo *other, CompareState &state) const {
  if (other == this)
    return true;

  auto cached = state.cache.find({this, other});
  if (cached != state.cache.end()) {
    return cached->second;
  }

  if (!state.visited.insert({this, other})) {
    // Failed to insert, already visited this pair.
    // Because we haven't diverged yet, they must be equal.
    return true;
  }

  // Remove this entry in the visited list after we finish this call.
  auto popOnExit =
      llvh::make_scope_exit([&state]() { state.visited.pop_back(); });

  // Need to account for either left or right not being completed unions yet.
  bool thisIncomplete = false;
  if (auto *thisUnion = llvh::dyn_cast<UnionType>(this)) {
    if (thisUnion->getNumNonLoopingTypes() == -1) {
      thisIncomplete = true;
    }
  }
  bool otherIncomplete = false;
  if (auto *otherUnion = llvh::dyn_cast<UnionType>(other)) {
    if (otherUnion->getNumNonLoopingTypes() == -1) {
      otherIncomplete = true;
    }
  }

  // If the kinds are different, the types can still be the same if one of them
  // is an incomplete union type. This is because union arms can be
  // deduplicated, resulting in any other type.
  if (kind_ != other->kind_ && !thisIncomplete && !otherIncomplete) {
    state.cache[{this, other}] = false;
    return false;
  }

  // Handle two unions when at least one of them is incomplete.
  if ((thisIncomplete || otherIncomplete) && llvh::isa<UnionType>(this) &&
      llvh::isa<UnionType>(this)) {
    bool result = listsMatchSlow(
        llvh::cast<UnionType>(this)->getTypes(),
        llvh::cast<UnionType>(other)->getTypes(),
        state);
    state.cache[{this, other}] = result;
    return result;
  }
  // Handle the cases of one incomplete union and a non-union.
  if (thisIncomplete) {
    bool result = llvh::all_of(
        llvh::cast<UnionType>(this)->getTypes(),
        [&state, other](Type *t) { return t->info->equals(other, state); });
    state.cache[{this, other}] = result;
    return result;
  }
  if (otherIncomplete) {
    bool result = llvh::all_of(
        llvh::cast<UnionType>(other)->getTypes(),
        [&state, this](Type *t) { return t->info->equals(this, state); });
    state.cache[{this, other}] = result;
    return result;
  }

  bool result;
  switch (kind_) {
#define _HERMES_SEMA_FLOW_DEFKIND(name)                          \
  case TypeKind::name:                                           \
    result = static_cast<const name##Type *>(this)->_equalsImpl( \
        static_cast<const name##Type *>(other), state);          \
    break;
    _HERMES_SEMA_FLOW_SINGLETONS
    _HERMES_SEMA_FLOW_COMPLEX_TYPES
#undef _HERMES_SEMA_FLOW_DEFKIND
    default:
      hermes_fatal("invalid TypeKind");
  }
  state.cache[{this, other}] = result;
  return result;
}

unsigned TypeInfo::hash() const {
  if (LLVM_LIKELY(hashed_))
    return hashValue_;

  unsigned hv;
  switch (kind_) {
#define _HERMES_SEMA_FLOW_DEFKIND(name)                      \
  case TypeKind::name:                                       \
    hv = static_cast<const name##Type *>(this)->_hashImpl(); \
    break;

    _HERMES_SEMA_FLOW_SINGLETONS _HERMES_SEMA_FLOW_COMPLEX_TYPES

#undef _HERMES_SEMA_FLOW_DEFKIND
        ;
    default:
      llvm_unreachable("invalid TypeKind");
  }
  hashed_ = true;
  return hashValue_ = hv;
}

UnionType::UnionType(llvh::SmallVector<Type *, 4> &&uncanonicalizedTypes)
    : types_(std::move(uncanonicalizedTypes)) {
  setHasAnyMixed();
}

void UnionType::setCanonicalTypes(
    llvh::SmallVector<Type *, 4> &&canonicalTypes,
    llvh::SmallVector<Type *, 4> &&recursiveTypes) {
  numNonLoopingTypes_ = canonicalTypes.size();
  types_ = std::move(canonicalTypes);
  types_.append(recursiveTypes.begin(), recursiveTypes.end());
  recursiveTypes.clear();
  assert(types_.size() > 1 && " Single element unions are unsupported");
  setHasAnyMixed();
}

void UnionType::canonicalizeTypes(
    llvh::ArrayRef<Type *> types,
    llvh::SmallVectorImpl<Type *> &canonicalTypes,
    llvh::SmallVectorImpl<Type *> &loopingTypes) {
  // Copy the union types to canonicalized, but flatten nested unions. Note that
  // there can't be more than one level.
  // The types won't cycle back to the parent because that's checked in
  // DeclareScopeTypes and this function is intended to be used after union-only
  // cycles have been detected (and errored on) in FlowChecker.
  for (Type *elemType : types) {
    assert(elemType->info && "cannot canonicalize unknown type");
    if (UnionType *unionType = llvh::dyn_cast<UnionType>(elemType->info)) {
      assert(
          unionType->getNumNonLoopingTypes() >= 0 &&
          "canonicalizeTypes children must have nested unions collapsed "
          "prior to call");
      for (Type *nestedElem : unionType->getNonLoopingTypes()) {
        canonicalTypes.push_back(nestedElem);
      }
      for (Type *nestedElem : unionType->getLoopingTypes()) {
        loopingTypes.push_back(nestedElem);
      }
    } else {
      canonicalTypes.push_back(elemType);
    }
  }

  sortAndUniqueNonLoopingTypes(canonicalTypes);
  uniqueLoopingTypesSlow(loopingTypes);
}

void UnionType::sortAndUniqueNonLoopingTypes(
    llvh::SmallVectorImpl<Type *> &types) {
  // Sort for predictable order.
  std::sort(types.begin(), types.end(), [](Type *a, Type *b) {
    return a->info->compare(b->info) < 0;
  });

  // Remove identical union arms.
  types.erase(
      std::unique(
          types.begin(),
          types.end(),
          [](Type *a, Type *b) { return a->info->equals(b->info); }),
      types.end());
}

void UnionType::uniqueLoopingTypesSlow(llvh::SmallVectorImpl<Type *> &types) {
  // If any pair of types is equal, set the first to nullptr for easy
  // deletion.
  for (size_t i = 0, e = types.size(); i < e; ++i) {
    for (size_t j = i + 1; j < e; ++j) {
      if (types[i]->info->equals(types[j]->info)) {
        types[i] = nullptr;
        break;
      }
    }
  }

  // Remove the marked duplicates.
  llvh::erase_if(types, [](Type *type) { return type == nullptr; });
}

// Iterate types_ and set the hasAny_ and hasMixed_ flags.
void UnionType::setHasAnyMixed() {
  hasAny_ = false;
  hasMixed_ = false;
  for (Type *t : types_) {
    if (!t->info)
      continue;
    if (llvh::isa<AnyType>(t->info))
      hasAny_ = true;
    if (llvh::isa<MixedType>(t->info))
      hasMixed_ = true;
  }
}

int UnionType::_compareImpl(const UnionType *other, CompareState &state) const {
  assert(numNonLoopingTypes_ >= 0 && "uninitialized union");
  assert(other->numNonLoopingTypes_ >= 0 && "uninitialized union");
  assert(getLoopingTypes().empty() && "recursive unions are not comparable");
  assert(
      other->getLoopingTypes().empty() &&
      "recursive unions are not comparable");
  return lexicographicalComparison(
      getNonLoopingTypes().begin(),
      getNonLoopingTypes().end(),
      other->getNonLoopingTypes().begin(),
      other->getNonLoopingTypes().end(),
      [&state](Type *a, Type *b) { return a->info->compare(b->info, state); });
}

bool UnionType::_equalsImpl(const UnionType *other, CompareState &state) const {
  assert(
      numNonLoopingTypes_ >= 0 && other->numNonLoopingTypes_ >= 0 &&
      "uninitialized unions handled in TypeInfo::equals");

  if (numNonLoopingTypes_ != other->numNonLoopingTypes_)
    return false;

  if (lexicographicalComparison(
          getNonLoopingTypes().begin(),
          getNonLoopingTypes().end(),
          other->getNonLoopingTypes().begin(),
          other->getNonLoopingTypes().end(),
          [&state](Type *a, Type *b) {
            return a->info->compare(b->info, state);
          }))
    return false;

  return listsMatchSlow(getLoopingTypes(), other->getLoopingTypes(), state);
}

unsigned UnionType::_hashImpl() const {
  assert(numNonLoopingTypes_ >= 0 && "types haven't been canonicalized yet");
  return (unsigned)llvh::hash_combine(
      (unsigned)TypeKind::Union, numNonLoopingTypes_);
}

int ArrayType::_compareImpl(const ArrayType *other, CompareState &state) const {
  return element_->info->compare(other->element_->info, state);
}

bool ArrayType::_equalsImpl(const ArrayType *other, CompareState &state) const {
  return element_->info->equals(other->element_->info, state);
}

unsigned ArrayType::_hashImpl() const {
  return (unsigned)llvh::hash_combine((unsigned)TypeKind::Array);
}

int TupleType::_compareImpl(const TupleType *other, CompareState &state) const {
  return lexicographicalComparison(
      types_.begin(),
      types_.end(),
      other->types_.begin(),
      other->types_.end(),
      [&state](const Type *ta, const Type *tb) {
        return ta->info->compare(tb->info, state);
      });
}

bool TupleType::_equalsImpl(const TupleType *other, CompareState &state) const {
  if (types_.size() != other->types_.size())
    return false;
  for (size_t i = 0, e = types_.size(); i < e; ++i) {
    if (!types_[i]->info->equals(other->types_[i]->info, state))
      return false;
  }
  return true;
}

unsigned TupleType::_hashImpl() const {
  return (unsigned)llvh::hash_combine((unsigned)TypeKind::Tuple, types_.size());
}

hermes::OptValue<size_t> ExactObjectType::findField(Identifier id) const {
  auto it = fieldNameMap_.find(id);
  if (it == fieldNameMap_.end())
    return llvh::None;
  return it->second;
}

int ExactObjectType::_compareImpl(
    const ExactObjectType *other,
    CompareState &state) const {
  return lexicographicalComparison(
      fields_.begin(),
      fields_.end(),
      other->fields_.begin(),
      other->fields_.end(),
      [&state](const Field &ta, const Field &tb) {
        if (int tmp = ta.name.str().compare(tb.name.str()))
          return tmp;
        if (int tmp = ta.type->info->compare(tb.type->info, state))
          return tmp;
        return 0;
      });
}

bool ExactObjectType::_equalsImpl(
    const ExactObjectType *other,
    CompareState &state) const {
  if (fields_.size() != other->fields_.size())
    return false;
  for (size_t i = 0, e = fields_.size(); i < e; ++i) {
    if (fields_[i].name != other->fields_[i].name)
      return {};
    if (!fields_[i].type->info->equals(other->fields_[i].type->info, state))
      return false;
  }
  return true;
}

unsigned ExactObjectType::_hashImpl() const {
  return (unsigned)llvh::hash_combine(
      (unsigned)TypeKind::ExactObject, fields_.size());
}

/// Compare two instances of the same TypeKind.
int TypedFunctionType::_compareImpl(
    const TypedFunctionType *other,
    CompareState &state) const {
  if (auto tmp = cmpHelperBool(isAsync(), other->isAsync()))
    return tmp;
  if (auto tmp = cmpHelperBool(isGenerator(), other->isGenerator()))
    return tmp;
  if (auto tmp = cmpHelper(thisParam_, other->thisParam_))
    return tmp;
  if (auto tmp = lexicographicalComparison(
          params_.begin(),
          params_.end(),
          other->params_.begin(),
          other->params_.end(),
          [&state](const Param &pa, const Param &pb) {
            return pa.second->info->compare(pb.second->info, state);
          })) {
    return tmp;
  }
  return cmpHelper(return_, other->return_);
}

/// Compare two instances of the same TypeKind.
bool TypedFunctionType::_equalsImpl(
    const TypedFunctionType *other,
    CompareState &state) const {
  if (cmpHelperBool(isAsync(), other->isAsync()))
    return false;
  if (cmpHelperBool(isGenerator(), other->isGenerator()))
    return false;
  if (cmpHelper(thisParam_, other->thisParam_))
    return false;
  if (lexicographicalComparison(
          params_.begin(),
          params_.end(),
          other->params_.begin(),
          other->params_.end(),
          [&state](const Param &pa, const Param &pb) {
            return pa.second->info->compare(pb.second->info, state);
          })) {
    return false;
  }
  if (cmpHelper(return_, other->return_))
    return false;
  return true;
}

/// Calculate the type-specific hash.
unsigned TypedFunctionType::_hashImpl() const {
  return (unsigned)llvh::hash_combine(
      (unsigned)TypeKind::TypedFunction,
      isAsync(),
      isGenerator(),
      thisParam_ != nullptr,
      params_.size());
}

int NativeFunctionType::_compareImpl(
    const NativeFunctionType *other,
    CompareState &) const {
  // If the native signatures are different, there is no point comparing the
  // JS signatures. If the native signatures are the same, then by definition
  // the JS ones must also be the same.
  return signature_->compare(other->signature_);
}

bool NativeFunctionType::_equalsImpl(
    const NativeFunctionType *other,
    CompareState &) const {
  // If the native signatures are different, there is no point comparing the
  // JS signatures. If the native signatures are the same, then by definition
  // the JS ones must also be the same.
  return signature_ == other->signature_;
}

unsigned NativeFunctionType::_hashImpl() const {
  return (unsigned)llvh::hash_combine(
      (unsigned)TypeKind::NativeFunction, signature_->hash());
}

/// Compare two instances of the same TypeKind.
int UntypedFunctionType::_compareImpl(
    const UntypedFunctionType *other,
    CompareState &state) const {
  if (auto tmp = cmpHelperBool(isAsync(), other->isAsync()))
    return tmp;
  if (auto tmp = cmpHelperBool(isGenerator(), other->isGenerator()))
    return tmp;
  return 0;
}

/// Compare two instances of the same TypeKind.
bool UntypedFunctionType::_equalsImpl(
    const UntypedFunctionType *other,
    CompareState &state) const {
  if (cmpHelperBool(isAsync(), other->isAsync()))
    return false;
  if (cmpHelperBool(isGenerator(), other->isGenerator()))
    return false;
  return true;
}

/// Calculate the type-specific hash.
unsigned UntypedFunctionType::_hashImpl() const {
  return (unsigned)llvh::hash_combine(
      (unsigned)TypeKind::UntypedFunction, isAsync(), isGenerator());
}

unsigned TypeWithId::_hashImpl() const {
  return (unsigned)llvh::hash_combine((unsigned)getKind(), getId());
}

ClassType::ClassType(size_t id, Identifier className)
    : TypeWithId(TypeKind::Class, id), className_(className) {}

void ClassType::init(
    llvh::ArrayRef<Field> fields,
    Type *constructorType,
    Type *homeObjectType,
    Type *superClass) {
  fields_.reserve(fields.size());
  fields_.append(fields.begin(), fields.end());

  constructorType_ = constructorType;
  homeObjectType_ = homeObjectType;
  superClass_ = superClass;

  if (superClass) {
    auto *superClassInfo = llvh::cast_or_null<ClassType>(superClass_->info);
    // Copy the lookup table down from the superClass to avoid having to climb
    // the whole chain every time we want to typecheck a property access.
    fieldNameMap_.reserve(
        fields.size() + superClassInfo->getFieldNameMap().size());
    for (const auto &it : superClassInfo->getFieldNameMap()) {
      fieldNameMap_[it.first] = it.second;
    }
  } else {
    fieldNameMap_.reserve(fields.size());
  }

  // Override the fields which have been overridden.
  size_t index = 0;
  for (const auto &f : this->fields_) {
    fieldNameMap_[f.name] = FieldLookupEntry{this, index++};
  }

  markAsInitialized();
}

hermes::OptValue<ClassType::FieldLookupEntry> ClassType::findField(
    Identifier id) const {
  auto it = fieldNameMap_.find(id);
  if (it == fieldNameMap_.end())
    return llvh::None;
  return it->second;
}

FlowContext::FlowContext() = default;
FlowContext::~FlowContext() = default;

Type *FlowContext::findDeclType(const sema::Decl *decl) const {
  assert(decl && "nullptr sema::Decl");
  auto it = declTypes_.find(decl);
  return it != declTypes_.end() ? it->second : nullptr;
}

Type *FlowContext::getDeclType(const sema::Decl *decl) const {
  Type *res = findDeclType(decl);
  assert(res && "unresolved declaration");
  return res;
}

void FlowContext::setNodeType(const ESTree::Node *node, Type *type) {
  if (type)
    nodeTypes_[node] = type;
}

Type *FlowContext::findNodeType(const ESTree::Node *node) const {
  auto it = nodeTypes_.find(node);
  return it != nodeTypes_.end() ? it->second : nullptr;
}

Type *FlowContext::getNodeTypeOrAny(const ESTree::Node *node) const {
  auto it = nodeTypes_.find(node);
  return it != nodeTypes_.end() ? it->second : getAny();
}

Type *FlowContext::getSingletonType(TypeKind kind) const {
  switch (kind) {
#define _HERMES_SEMA_FLOW_DEFKIND(name) \
  case TypeKind::name:                  \
    return &name##Instance_;
    _HERMES_SEMA_FLOW_SINGLETONS
#undef _HERMES_SEMA_FLOW_DEFKIND
    default:
      llvm_unreachable("invalid singleton TypeKind");
      return nullptr;
  }
}

TypeInfo *FlowContext::maybeCreateUnion(
    llvh::ArrayRef<Type *> types,
    UnionExcludes excludes) {
  assert(!types.empty() && "types must not be empty");
  llvh::SmallVector<Type *, 4> canonicalTypes{};
  llvh::SmallVector<Type *, 4> recursiveTypes{};

  UnionType::canonicalizeTypes(types, canonicalTypes, recursiveTypes);

  if (excludes.excludeVoid || excludes.excludeNull) {
    // Only look at the first two elements and get the new ending.
    auto searchEnd = std::min(canonicalTypes.end(), canonicalTypes.begin() + 2);
    auto removedEnd =
        std::remove_if(canonicalTypes.begin(), searchEnd, [&excludes](Type *t) {
          return (llvh::isa<VoidType>(t->info) && excludes.excludeVoid) ||
              (llvh::isa<NullType>(t->info) && excludes.excludeNull);
        });
    // Erase the removed elements from the container.
    canonicalTypes.erase(removedEnd, searchEnd);
  }

  // The types collapsed to a single type, so return that.
  if (canonicalTypes.size() == 1 && recursiveTypes.empty())
    return canonicalTypes.front()->info;
  if (canonicalTypes.empty() && recursiveTypes.size() == 1)
    return recursiveTypes.front()->info;

  return createCanonicalizedUnion(
      std::move(canonicalTypes), std::move(recursiveTypes));
}

UnionType *FlowContext::createPopulatedNullable(Type *type) {
  llvh::SmallVector<Type *, 4> canonicalTypes{};
  llvh::SmallVector<Type *, 4> recursiveTypes{};
  UnionType::canonicalizeTypes(
      {getVoid(), getNull(), type}, canonicalTypes, recursiveTypes);

  // We know that there are at least 2 types, so it will be a union.
  return createCanonicalizedUnion(
      std::move(canonicalTypes), std::move(recursiveTypes));
}

} // namespace flow
} // namespace hermes
