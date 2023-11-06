/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// This file implements a scoped hash table similar to hermes::ScopedHashTable,
/// but scopes can be retained and reactivated after they have been popped from
/// the table.
//===----------------------------------------------------------------------===//

#ifndef HERMES_ADT_PERSISTENTSCOPEDHASHTABLE_H
#define HERMES_ADT_PERSISTENTSCOPEDHASHTABLE_H

#include "llvh/ADT/DenseMap.h"
#include "llvh/Support/RecyclingAllocator.h"

// hermes::PersistentScopedMap is a drop-in replacement for
// llvh::ScopedHashTable, but will allow us to export the current values in
// scope.

namespace hermes {
template <typename K, typename V>
class PersistentScopedMap;
template <typename K, typename V>
class PersistentScopedMapScope;

namespace detail {
template <typename K, typename V>
class PersistentScopedMapScopeData;
} // namespace detail

//===----------------------------------------------------------------------===//
// PersistentScopedMapScopePtr declaration

/// Smart pointer owning a referenced counted scope.
template <typename K, typename V>
class PersistentScopedMapScopePtr {
  friend class detail::PersistentScopedMapScopeData<K, V>;
  friend class PersistentScopedMap<K, V>;
  friend class PersistentScopedMapScope<K, V>;

  detail::PersistentScopedMapScopeData<K, V> *ptr_;

  explicit PersistentScopedMapScopePtr(
      detail::PersistentScopedMapScopeData<K, V> *scope);

 public:
  PersistentScopedMapScopePtr();
  PersistentScopedMapScopePtr(const PersistentScopedMapScopePtr &p);
  PersistentScopedMapScopePtr(PersistentScopedMapScopePtr &&other);

  PersistentScopedMapScopePtr &operator=(const PersistentScopedMapScopePtr &p);
  PersistentScopedMapScopePtr &operator=(PersistentScopedMapScopePtr &&other);

  ~PersistentScopedMapScopePtr();

  /// Free the scope and set the pointer to null.
  void reset();

  /// Return true if this pointer is not null.
  operator bool() const {
    return ptr_;
  }

  bool operator==(const PersistentScopedMapScopePtr &other) const {
    return ptr_ == other.ptr_;
  }

 private:
  detail::PersistentScopedMapScopeData<K, V> *get() const {
    return ptr_;
  }
  detail::PersistentScopedMapScopeData<K, V> *operator->() const {
    return ptr_;
  }

  void set(detail::PersistentScopedMapScopeData<K, V> *ptr);
};

namespace detail {

//===----------------------------------------------------------------------===//
// detail::PersistentScopedMapNode

/// A key/value pair in a scope. It points to the next pair in the same scope,
/// which allows us to pop all pairs in a scope at once.
/// Additionally it points to a "shadowed" pair with the same key in an upper
/// (surrounding) scope.
template <typename K, typename V>
class PersistentScopedMapNode : public std::pair<const K, V> {
 public:
  /// Shadowed value in an upper (surrounding) scope.
  PersistentScopedMapNode<K, V> *nextShadowed_{nullptr};
  /// Next node in the same scope. This is a linked list that allows us to
  /// pop a scope.
  PersistentScopedMapNode<K, V> *nextInScope_{nullptr};
  /// Current scope depth, starting from 0 for the outermost scope.
  uint32_t depth_;

  PersistentScopedMapNode(uint32_t depth, const K &key, const V &value)
      : std::pair<const K, V>(key, value), depth_(depth) {}
};

//===----------------------------------------------------------------------===//
// detail::PersistentScopedMapScopeData declaration

/// This is the data for a scope. It is reference counted. It contains the
/// head of the list of nodes, and a pointer to the parent scope.
template <typename K, typename V>
class PersistentScopedMapScopeData {
 public:
  using Node = detail::PersistentScopedMapNode<K, V>;

  /// The table we're creating a scope for.
  PersistentScopedMap<K, V> &base_;
  /// Start of the linked list of nodes in this scope. Owned by
  /// PersistentScopedMap.
  Node *head_{nullptr};
  /// The scope we're shadowing.
  PersistentScopedMapScopePtr<K, V> parentScope_;
  /// Scope depth, starting from 0 for the outermost one.
  uint32_t depth_;
  /// Reference counter.
  unsigned refCount_ = 0;
  /// Whether this scope is active in the map or it has been popped.
  bool active_;

  explicit PersistentScopedMapScopeData(PersistentScopedMap<K, V> &base);
  ~PersistentScopedMapScopeData();

  PersistentScopedMapScopeData(const PersistentScopedMapScopeData &) = delete;
  PersistentScopedMapScopeData(PersistentScopedMapScopeData &&) = delete;
  PersistentScopedMapScopeData &operator=(
      const PersistentScopedMapScopeData &) = delete;
  PersistentScopedMapScopeData &operator=(PersistentScopedMapScopeData &&) =
      delete;

  /// Pop this scope, which must be current one, from the map and mark is as
  /// not active.
  void pop();

  /// Increment the reference counter.
  void addRef();
  /// Decrease the reference counter. If it reaches zero, destroy the scope.
  void decRef();
};

} // namespace detail

/// RAII for creating and popping a scope.
template <typename K, typename V>
class PersistentScopedMapScope {
  PersistentScopedMapScopePtr<K, V> scope_;

 public:
  explicit PersistentScopedMapScope(PersistentScopedMap<K, V> &base)
      : scope_(new detail::PersistentScopedMapScopeData<K, V>(base)) {}

  PersistentScopedMapScope(const PersistentScopedMapScope &) = delete;
  PersistentScopedMapScope &operator=(const PersistentScopedMapScope &) =
      delete;
  PersistentScopedMapScope(PersistentScopedMapScope &&) = delete;
  PersistentScopedMapScope &operator=(PersistentScopedMapScope &&) = delete;

  ~PersistentScopedMapScope() {
    scope_->pop();
  }

  /// Return a persistent pointer that retains ownership of the scope so it can
  /// be reactivated after it has been popped.
  const PersistentScopedMapScopePtr<K, V> &ptr() const {
    return scope_;
  }
};

/// Scoped hash table similar to hermes::ScopedHashTable, but scopes can be
/// retained and reactivated after they have been popped from the table.
/// The type \c PersistentScopedMapScopePtr<K,V>, which technically is just an
/// intrusive reference counting smart pointer, is used to retain ownership of a
/// scope. The pointer can be used to reactivate the scope in the table using
/// \c PersistentScopedMap::activateScope().
///
/// Scopes can also be re-activated even if they are currently active but are
/// not the current scope. Note however that if there are active scopes in the
/// stack, in the end we must restore the state - the top-most scope in the
/// stack must be is active.
///
/// Example:
/// \code
///     PersistentScopedMapScopePtr<K, V> ptr;
///     PersistentScopedMap<const char *, const char *> table{};
///     PersistentScopedMapScope<K, V> A(table);
///     PersistentScopedMapScope<K, V> B(table);
///     // At this point, A and B are active scopes in the table.
///     // A(active)->B(active)
///     // We can reactivate A:
///     table.activateScope(A.ptr());
///     // Now the state is the same as it was when A was active initially.
///     table.activateScope(B.ptr());
///     // The state has been restored to "normal".
///     {
///         PersistentScopedMapScope<K, V> C(table);
///         // A(active)->B(active)->C(active)
///         // Save C for later.
///         ptr = C.ptr();
///     }
///     // A(active)->B(active)
///     PersistentScopedMapScope<K, V> D(table);
///     // A(active)->B(active)->D(active).
///     // Activate C again.
///     table.activateScope(ptr);
///     // A(active)->B(active)->C(active)
///     // Restore normal state.
///     table.activateScope(D.ptr());
///
/// \endcode
template <typename K, typename V>
class PersistentScopedMap {
 public:
  using value_type = std::pair<const K, V>;

 private:
  using Scope = detail::PersistentScopedMapScopeData<K, V>;
  using Node = detail::PersistentScopedMapNode<K, V>;

  /// Maps from keys to most current definition.
  llvh::DenseMap<K, Node *> map_{};
  /// The current scope.
  PersistentScopedMapScopePtr<K, V> scope_{};

  /// Unlinks the specified entry, which must be the innermost, and returns it.
  Node *popEntry(typename decltype(map_)::iterator it) {
    Node *entry = it->second;
    assert(entry && "Asked to pop an empty scope value");
    assert(
        entry->depth_ == scope_->depth_ &&
        "Asked to pop value not from innermost scope");
    auto *ret = entry;
    if (entry->nextShadowed_) {
      it->second = entry->nextShadowed_;
    } else {
      map_.erase(it);
    }
    return ret;
  }

  /// Unlinks the innermost Node for a key and returns it.
  Node *popEntry(const K &key) {
    return popEntry(map_.find(key));
  }

  /// Push the specified node to the top of the stack for the given map entry.
  ///
  /// \param scope the scope to push the entry into. Used only for debug checks.
  /// \param node the node to push.
  /// \param entry a pointer to the entry in the map for this key (which is a
  ///     Node *).
  void pushEntry(Scope *const scope, Node *node, Node **entry) {
    assert(
        (!*entry || (*entry)->depth_ < scope->depth_) &&
        "Can't insert values under existing names");
    node->nextShadowed_ = *entry;
    *entry = node;
  }

  /// Create a new node and insert it into the current scope.
  Node *insertNewNode(
      Scope *const scope,
      const K &key,
      const V &value,
      Node **entry) {
    // All Nodes allocated here.
    auto *update = new Node(scope->depth_, key, value);
    update->nextInScope_ = scope->head_;
    scope->head_ = update;
    pushEntry(scope, update, entry);
    return update;
  }

  /// Unlinks all nodes in the current scope from the hash map and mark it
  /// as inactive.
  void popScope(Scope *scope) {
    assert(scope && "Cannot pop a null scope");
    assert(scope->active_ && "Attempting to pop an inactive scope");
    assert(scope == scope_.get() && "Attempting to pop not current scope");

    for (Node *curNode = scope_->head_; curNode;
         curNode = curNode->nextInScope_) {
      assert(curNode->depth_ == scope_->depth_ && "Bad scope link");
      Node *popped = popEntry(curNode->first);
      assert(curNode == popped && "Unexpected innermost value for key");
      (void)popped;
    }
    scope_->active_ = false;
    scope_ = scope_->parentScope_;
  }

  /// Push the specified scope, which must be a child of the current scope,
  /// into the hash map and activate it.
  void pushChildScope(Scope *scope) {
    assert(!scope->active_ && "Attempting to push an active scope");
    assert(
        scope->parentScope_ == scope_ &&
        "Attempting to push a scope that isn't a child of the current one");
    for (Node *curNode = scope->head_; curNode;
         curNode = curNode->nextInScope_) {
      pushEntry(scope, curNode, &map_[curNode->first]);
    }
    scope->active_ = true;
    scope_.set(scope);
  }

  /// Attempt to insert an element into the specified scope.
  /// Semantics equivalent to std::map::try_emplace(). Returns a pair with an
  /// iterator to the K, V pair and a bool indicating whether the insertion took
  /// place.
  /// A key may not be inserted such that it would be shadowed by another scope
  /// currently in effect. Attempting to do so results in undefined behavior.
  std::pair<value_type *, bool>
  tryEmplaceIntoScope(Scope *const scope, const K &key, const V &value) {
    assert(scope_->active_ && "Attempting to modify an inactive scope");
    Node **entry = &map_[key];
    if (*entry && (*entry)->depth_ == scope->depth_) {
      // The key exists in the current scope.
      return {*entry, false};
    } else {
      // Otherwise, create a new node in the current scope.
      return {insertNewNode(scope, key, value, entry), true};
    }
  }

 public:
  PersistentScopedMap() = default;
  ~PersistentScopedMap() {
    assert(!scope_ && "Scopes remain when destructing PersistentScopedMap");
    assert(!map_.size() && "Elements remaining in map without scope!");
  }

  /// Return a pointer to the current scope. The pointer may be nullptr.
  const PersistentScopedMapScopePtr<K, V> &getCurrentScope() const {
    return scope_;
  }

  /// Attempt to insert an element into the specified scope.
  /// Semantics equivalent to std::map::try_emplace(). Returns a pair with an
  /// iterator to the K, V pair and a bool indicating whether the insertion took
  /// place.
  /// A key may not be inserted such that it would be shadowed by another scope
  /// currently in effect. Attempting to do so results in undefined behavior.
  std::pair<value_type *, bool> tryEmplaceIntoScope(
      const PersistentScopedMapScopePtr<K, V> &scope,
      const K &key,
      const V &value) {
    return tryEmplaceIntoScope(scope.get(), key, value);
  }

  /// Attempt to insert an element into the current scope.
  /// Semantics equivalent to std::map::try_emplace(). Returns a pair with an
  /// iterator to the K,V pair and a bool indicating whether the insertion took
  /// place.
  std::pair<value_type *, bool> try_emplace(const K &key, const V &value) {
    return tryEmplaceIntoScope(scope_.get(), key, value);
  }

  /// Insert or update a value in the specified scope.
  /// A key may not be inserted such that it would be shadowed by another scope
  /// currently in effect. Attempting to do so results in undefined behavior.
  void putInScope(
      const PersistentScopedMapScopePtr<K, V> &scope,
      const K &key,
      const V &value) {
    auto [it, inserted] = tryEmplaceIntoScope(scope, key, value);
    if (!inserted)
      it->second = value;
  }

  /// Insert or update an existing value in the current scope.
  void put(const K &key, const V &value) {
    putInScope(scope_, key, value);
  }

  /// Returns 1 if the value is defined, 0 if it's not.
  uint32_t count(const K &key) const {
    return map_.count(key);
  }

  /// Gets the innermost value for a key, or default value if none.
  V lookup(const K &key) const {
    auto result = map_.find(key);
    if (result == map_.end())
      return V();

    return result->second->second;
  }

  /// Return a pointer to the innermost value for a key, or nullptr if none.
  V *find(const K &key) {
    auto result = map_.find(key);
    if (result == map_.end())
      return nullptr;

    return &result->second->second;
  }

  /// \return a pointer to the value for a key if it exists in the current
  /// scope, or nullptr if none.
  V *findInCurrentScope(const K &key) {
    auto result = map_.find(key);
    if (result == map_.end())
      return nullptr;

    // Result is not in the current scope.
    if (result->second->depth_ != scope_->depth_)
      return nullptr;

    return &result->second->second;
  }

  void activateScope(const PersistentScopedMapScopePtr<K, V> &newScopePtr) {
    Scope *newScope = newScopePtr.get();
    // We need to find the closest active parent of newScope. Then we need to
    // deactivate and pop all scopes between the curren scope and that parent.
    // Finally, we need to push and activate all scopes between newScope and
    // the parent.

    // Keep track of scopes that need to be activated in reverse order.
    llvh::SmallVector<Scope *, 4> activateList{};
    Scope *activeParent = newScope;
    while (activeParent && !activeParent->active_) {
      activateList.push_back(activeParent);
      activeParent = activeParent->parentScope_.get();
    }

    // Deactivate and pop all scopes between scope_ and activeParent.
    while (scope_.get() != activeParent)
      popScope(scope_.get());

    // Push and activate the scopes in activateList in reverse order (starting
    // from the topmost).
    for (auto *scope : llvh::reverse(activateList))
      pushChildScope(scope);
  }

#ifdef UNIT_TEST
  // Gets all values currently in scope.
  std::unique_ptr<llvh::DenseMap<K, V>> test_flatten() const {
    std::unique_ptr<llvh::DenseMap<K, V>> result{
        new llvh::DenseMap<K, V>(map_.size())};
    for (auto &pair : map_) {
      assert(pair.second && "Node is null");
      (*result)[pair.first] = pair.second->second;
    }
    return result;
  }

  // Gets keys in each scope. This may correspond to a \p ScopeChain.
  // Shadowed keys are ignored. 0 is innermost.
  std::unique_ptr<std::vector<std::vector<K>>> test_getKeysByScope() const {
    assert(scope_ && "Missing scope");

    int size = scope_->depth_ + 1;
    std::unique_ptr<std::vector<std::vector<K>>> result;
    result.reset(new std::vector<std::vector<K>>());
    result->resize(size);

    for (auto &pair : map_) {
      auto *node = pair.second;
      assert(node && "Node is null");
      assert(node->depth_ <= scope_->depth_ && "Node at bad depth");
      result->at(size - node->depth_ - 1).push_back(pair.first);
    }
    return result;
  }
#endif // UNIT_TEST

  friend class detail::PersistentScopedMapScopeData<K, V>;
};

//===----------------------------------------------------------------------===//
// PersistentScopedMapScopePtr impl.

template <typename K, typename V>
PersistentScopedMapScopePtr<K, V>::PersistentScopedMapScopePtr(
    detail::PersistentScopedMapScopeData<K, V> *scope)
    : ptr_(scope) {
  if (ptr_)
    ptr_->addRef();
}

template <typename K, typename V>
PersistentScopedMapScopePtr<K, V>::PersistentScopedMapScopePtr()
    : ptr_(nullptr) {}

template <typename K, typename V>
PersistentScopedMapScopePtr<K, V>::PersistentScopedMapScopePtr(
    const PersistentScopedMapScopePtr &p)
    : ptr_(p.ptr_) {
  if (ptr_)
    ptr_->addRef();
}

template <typename K, typename V>
PersistentScopedMapScopePtr<K, V>::PersistentScopedMapScopePtr(
    PersistentScopedMapScopePtr &&other)
    : ptr_(other.ptr_) {
  other.ptr_ = nullptr;
}

template <typename K, typename V>
PersistentScopedMapScopePtr<K, V> &PersistentScopedMapScopePtr<K, V>::operator=(
    const PersistentScopedMapScopePtr &p) {
  set(p.ptr_);
  return *this;
}

template <typename K, typename V>
PersistentScopedMapScopePtr<K, V> &PersistentScopedMapScopePtr<K, V>::operator=(
    PersistentScopedMapScopePtr &&other) {
  if (ptr_)
    ptr_->decRef();
  ptr_ = other.ptr_;
  other.ptr_ = nullptr;
  return *this;
}

template <typename K, typename V>
PersistentScopedMapScopePtr<K, V>::~PersistentScopedMapScopePtr() {
  if (ptr_)
    ptr_->decRef();
}

template <typename K, typename V>
void PersistentScopedMapScopePtr<K, V>::reset() {
  if (ptr_)
    ptr_->decRef();
  ptr_ = nullptr;
}

template <typename K, typename V>
void PersistentScopedMapScopePtr<K, V>::set(
    detail::PersistentScopedMapScopeData<K, V> *ptr) {
  if (ptr_)
    ptr_->decRef();
  ptr_ = ptr;
  if (ptr_)
    ptr_->addRef();
}

namespace detail {
//===----------------------------------------------------------------------===//
// detail::PersistentScopedMapScopeData impl

template <typename K, typename V>
PersistentScopedMapScopeData<K, V>::PersistentScopedMapScopeData(
    PersistentScopedMap<K, V> &base)
    : base_(base),
      parentScope_(base.scope_),
      depth_(!parentScope_ ? 0 : parentScope_->depth_ + 1),
      active_(true) {
  base.scope_.set(this);
}

template <typename K, typename V>
PersistentScopedMapScopeData<K, V>::~PersistentScopedMapScopeData() {
  assert(refCount_ == 0 && "Scope destroyed with non-zero ref count");
  assert(!active_ && "Cannot destroy an active scope");

  // Delete all nodes.
  for (Node *curNode = head_; curNode;) {
    assert(curNode->depth_ == depth_ && "Bad scope link");
    Node *toDelete = curNode;
    curNode = curNode->nextInScope_;
    delete toDelete;
  }
}

template <typename K, typename V>
void PersistentScopedMapScopeData<K, V>::pop() {
  base_.popScope(this);
}

template <typename K, typename V>
void PersistentScopedMapScopeData<K, V>::addRef() {
  ++refCount_;
  assert(refCount_ != 0 && "refCount_ overflow!");
}

template <typename K, typename V>
void PersistentScopedMapScopeData<K, V>::decRef() {
  assert(refCount_ && "refCount_ underflow");
  if (--refCount_ == 0)
    delete this;
}

} // namespace detail

} // namespace hermes

#endif
