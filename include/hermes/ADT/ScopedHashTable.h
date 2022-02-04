/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ADT_SCOPEDHASHTABLE_H
#define HERMES_ADT_SCOPEDHASHTABLE_H

#include "llvh/ADT/DenseMap.h"
#include "llvh/Support/Allocator.h"

// hermes::ScopedHashTable is a drop-in replacement for llvh::ScopedHashTable,
// but will allow us to export the current values in scope.

namespace hermes {
template <typename K, typename V>
class ScopedHashTableNode {
 public:
  K key_;
  V value_;

  ScopedHashTableNode<K, V> *nextShadowed_{nullptr};
  ScopedHashTableNode<K, V> *nextInScope_{nullptr};
  uint32_t depth_;
  ScopedHashTableNode(uint32_t depth, const K &key, const V &value)
      : key_(key), value_(value), depth_(depth) {}
};

template <typename K, typename V>
class ScopedHashTable;

template <typename K, typename V>
class ScopedHashTableScope {
 private:
  uint32_t depth_;
  /// Start of the linked list of nodes in this scope. Owned by ScopedHashTable.
  ScopedHashTableNode<K, V> *head_{nullptr};
  /// The scope we're shadowing. Owned by the user.
  ScopedHashTableScope<K, V> *previous_;
  /// The table we're creating a scope for.
  ScopedHashTable<K, V> &base_;

 public:
  ScopedHashTableScope(ScopedHashTable<K, V> &base);
  ~ScopedHashTableScope();

  ScopedHashTableScope(const ScopedHashTableScope &) = delete;
  ScopedHashTableScope(ScopedHashTableScope &&) = delete;
  ScopedHashTableScope &operator=(const ScopedHashTableScope &) = delete;
  ScopedHashTableScope &operator=(ScopedHashTableScope &&) = delete;

  friend class ScopedHashTable<K, V>;
};

template <typename K, typename V>
class ScopedHashTable {
 private:
  /// Maps from keys to most current definition. All Nodes are owned by this.
  /// They are allocated by insertIntoScope and deleted by clearCurrentScope
  llvh::DenseMap<K, ScopedHashTableNode<K, V> *> map_;
  // The current scope. Owned by the user.
  ScopedHashTableScope<K, V> *scope_{nullptr};

  /// Unlinks the innermost Node for a key and returns it.
  ScopedHashTableNode<K, V> *pop(const K &key) {
    ScopedHashTableNode<K, V> *&entry = map_[key];
    assert(entry && "Asked to pop an empty scope value");
    assert(
        entry->depth_ == scope_->depth_ &&
        "Asked to pop value not from innermost scope");
    auto *ret = entry;
    if (entry->nextShadowed_) {
      entry = entry->nextShadowed_;
    } else {
      map_.erase(key);
    }
    return ret;
  }

  /// Unlinks and deallocates all keys in the current scope.
  void clearCurrentScope() {
    assert(scope_ && "No current scope to clear");

    auto *current = scope_->head_;
    while (current) {
      assert(current->depth_ == scope_->depth_ && "Bad scope link");
      auto *popped = pop(current->key_);
      assert(current == popped && "Unexpected innermost value for key");
      current = current->nextInScope_;
      // All nodes deallocated here.
      delete popped;
    }
    scope_->head_ = nullptr;
  }

  /// Create a new node and insert it into the current scope.
  void insertNewNode(
      ScopedHashTableScope<K, V> *const scope,
      const K &key,
      const V &value,
      ScopedHashTableNode<K, V> *&entry) {
    // All Nodes allocated here.
    auto *update = new ScopedHashTableNode<K, V>(scope->depth_, key, value);
    assert(
        (!entry || entry->depth_ <= scope->depth_) &&
        "Can't insert values under existing names");
    update->nextShadowed_ = entry;
    update->nextInScope_ = scope->head_;
    scope->head_ = update;
    entry = update;
  }

 public:
  ScopedHashTable() : map_() {}
  ~ScopedHashTable() {
    assert(!scope_ && "Scopes remain when destructing ScopedHashTable");
    assert(!map_.size() && "Elements remaining in map without scope!");
  }

  /// Inserts a new key into the given scope. A key may not be inserted
  /// such that it would be shadowed by another scope currently in effect.
  void insertIntoScope(
      ScopedHashTableScope<K, V> *const scope,
      const K &key,
      const V &value) {
    assert(scope && "No currently defined scope");
    insertNewNode(scope, key, value, map_[key]);
  }

  /// Inserts a value into the current scope.
  void insert(const K &key, const V &value) {
    insertIntoScope(scope_, key, value);
  }

  /// If the key exists in the current scope, update its value to the specified
  /// one. Otherwise, create a new entry in current scope.
  void setInCurrentScope(const K &key, const V &value) {
    assert(scope_ && "No currently defined scope");
    ScopedHashTableNode<K, V> *&entry = map_[key];
    if (entry && entry->depth_ == scope_->depth_) {
      // If the key exists in the current scope, update the value.
      entry->value_ = value;
    } else {
      // Otherwise, create a new node in the current scope.
      insertNewNode(scope_, key, value, entry);
    }
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

    return result->second->value_;
  }

  // Gets all values currently in scope.
  std::unique_ptr<llvh::DenseMap<K, V>> flatten() const {
    std::unique_ptr<llvh::DenseMap<K, V>> result{
        new llvh::DenseMap<K, V>(map_.size())};
    for (auto &pair : map_) {
      assert(pair.second && "Node is null");
      (*result)[pair.first] = pair.second->value_;
    }
    return result;
  }

  // Gets keys in each scope. This may correspond to a \p ScopeChain.
  // Shadowed keys are ignored. 0 is innermost.
  std::unique_ptr<std::vector<std::vector<K>>> getKeysByScope() const {
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

  friend class ScopedHashTableScope<K, V>;
};

template <typename K, typename V>
ScopedHashTableScope<K, V>::ScopedHashTableScope(ScopedHashTable<K, V> &base)
    : base_(base) {
  previous_ = base.scope_;
  depth_ = !previous_ ? 0 : previous_->depth_ + 1;
  base.scope_ = this;
}

template <typename K, typename V>
ScopedHashTableScope<K, V>::~ScopedHashTableScope() {
  assert(this == base_.scope_ && "Deallocating scope that's not current!");
  base_.clearCurrentScope();
  base_.scope_ = previous_;
}

} // namespace hermes

#endif // HERMES_ADT_SCOPEDHASHTABLE_H
