/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cassert>
#include <deque>
#include <vector>

namespace hermes {

/// A container that tracks values in least-recently-used order. Values can be
/// added, removed, and marked as used. The least recently used value can be
/// retrieved efficiently.
///
/// Implementation: Values are stored in nodes that form a doubly-linked list
/// ordered by recency. The most recently used node is at the front (after the
/// sentinel head), and the least recently used is at the back (before the
/// head). Nodes are allocated from a deque to ensure pointer stability.
/// Removed nodes are cached in a free list for reuse.
template <typename T>
class SimpleLRU {
  /// Magic number used in debug builds to validate node pointers.
  static constexpr uint32_t kMark = 0x764ABF12;

  /// A node in the doubly-linked list, containing only the list pointers.
  /// This is separated from the value to allow the sentinel head to be just
  /// an Item without requiring a default-constructible T.
  struct Item {
#ifndef NDEBUG
    /// Debug marker to detect invalid pointer casts.
    uint32_t marker;
#endif
    /// Pointer to the previous item in the list (more recently used).
    Item *prev;
    /// Pointer to the next item in the list (less recently used).
    Item *next;
  };

  /// A complete node containing both list pointers and the stored value.
  struct Node {
    /// The list item, must be first for pointer arithmetic in leastRecent().
    Item item;
    /// The user-provided value.
    T value;
  };

  /// Storage for all allocated nodes. We use a deque so that insertions do
  /// not invalidate pointers to existing nodes.
  std::deque<Node> nodes_{};

  /// Cache of removed nodes available for reuse, avoiding repeated allocation.
  std::vector<Node *> freed_{};

  /// Number of pre-allocated nodes (from capacity constructor) not yet used.
  /// These are at indices [nodes_.size() - preAllocAvail_, nodes_.size()).
  size_t preAllocAvail_;

  /// Sentinel head of the doubly-linked list. head_.next points to the most
  /// recently used node, head_.prev points to the least recently used node.
  /// When empty, both point to &head_ itself.
  Item head_;

 public:
  /// Construct an empty LRU container.
  SimpleLRU() : preAllocAvail_(0) {
    head_.prev = head_.next = &head_;
  }

  /// Construct an empty LRU container with pre-allocated storage.
  /// \p capacity The number of elements to pre-allocate. Requires T to be
  ///   default-constructible.
  explicit SimpleLRU(size_t capacity) : preAllocAvail_(capacity) {
    head_.prev = head_.next = &head_;
    nodes_.resize(capacity);
    freed_.reserve(capacity);
#ifndef NDEBUG
    for (auto &node : nodes_)
      node.item.marker = kMark;
#endif
  }

  // Type is not copyable.
  SimpleLRU(const SimpleLRU &) = delete;
  SimpleLRU &operator=(const SimpleLRU &) = delete;

  // Type is not movable (due to self-referential head_ pointers).
  SimpleLRU(SimpleLRU &&) = delete;
  SimpleLRU &operator=(SimpleLRU &&) = delete;

  /// \return true if the container has no values.
  bool empty() const {
    return head_.next == &head_;
  }

  /// Add a new value to the container, marking it as most recently used.
  /// \p value The value to add.
  /// \return A pointer to the stored value. This pointer remains valid until
  ///   the value is removed.
  T *add(const T &value) {
    Node *n;
    if (preAllocAvail_ > 0) {
      // Use a pre-allocated node.
      n = &nodes_[nodes_.size() - preAllocAvail_];
      --preAllocAvail_;
    } else if (!freed_.empty()) {
      n = freed_.back();
      freed_.pop_back();
    } else {
      n = &nodes_.emplace_back();
#ifndef NDEBUG
      n->item.marker = kMark;
#endif
    }
    n->value = value;
    listInsert(&head_, &n->item);
    return &n->value;
  }

  /// Mark a value as most recently used, moving it to the front.
  /// \p pValue Pointer to a value previously returned by add().
  void use(T *pValue) {
    Node *n = (Node *)((char *)pValue - offsetof(Node, value));
    assert(n->item.marker == kMark && "Invalid node pointer");
    if (n->item.prev != &head_) {
      listRemove(&n->item);
      listInsert(&head_, &n->item);
    }
  }

  /// Remove a value from the container.
  /// \p pValue Pointer to a value previously returned by add(). The pointer
  ///   becomes invalid after this call.
  void remove(T *pValue) {
    Node *n = (Node *)((char *)pValue - offsetof(Node, value));
    assert(n->item.marker == kMark && "Invalid node pointer");
    listRemove(&n->item);
    freed_.push_back(n);
  }

  /// \return A pointer to the least recently used value.
  /// \pre !empty()
  T *leastRecent() {
    assert(!empty() && "LRU is empty");
    Node *n = (Node *)head_.prev;
    assert(n->item.marker == kMark && "Invalid node pointer");
    return &n->value;
  }

 private:
  /// Insert \p elem into the list immediately after \p after.
  /// \pre elem is not currently in any list.
  void listInsert(Item *after, Item *elem) {
    assert(!elem->prev && !elem->next && "Item already in list");
    elem->next = after->next;
    elem->prev = after;
    after->next = elem;
    elem->next->prev = elem;
  }

  /// Remove \p elem from the list.
  /// \pre elem is currently in a list.
  void listRemove(Item *elem) {
    assert(elem->prev && elem->next && "Item not in list");
    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;
#ifndef NDEBUG
    elem->prev = elem->next = nullptr;
#endif
  }
};

} // namespace hermes
