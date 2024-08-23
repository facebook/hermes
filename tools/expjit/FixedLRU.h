/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cassert>
#include <vector>

using T = int;

template <typename T>
class FixedLRU {
  static constexpr uint32_t kMark = 0x764ABF12;

  struct Item {
#ifndef NDEBUG
    uint32_t marker;
#endif
    Item *prev, *next;
  };
  struct Node {
    Item item;
    T value;
  };

  std::vector<Node> nodes_{};
  std::vector<Node *> freed_{};
  Item head_;

 public:
  explicit FixedLRU() {
    head_.prev = head_.next = &head_;
  }
  explicit FixedLRU(size_t capacity) : FixedLRU() {
    nodes_.reserve(capacity);
    freed_.reserve(capacity);
  }

  explicit FixedLRU(const FixedLRU &) = delete;
  explicit FixedLRU(FixedLRU &&) = delete;
  void operator=(const FixedLRU &) = delete;
  void operator=(FixedLRU &&) = delete;

  bool empty() const {
    return head_.next == &head_;
  }

  /// Add a new value.
  T *add(const T &value) {
    Node *n;
    if (!freed_.empty()) {
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

  /// Mark a value as most recently used.
  void use(T *pValue) {
    Node *n = (Node *)((char *)pValue - offsetof(Node, value));
    assert(n->item.marker == kMark && "Invalid node pointer");
    if (n->item.prev != &head_) {
      listRemove(&n->item);
      listInsert(&head_, &n->item);
    }
  }

  /// Remove a value from the LRU.
  void remove(T *pValue) {
    Node *n = (Node *)((char *)pValue - offsetof(Node, value));
    assert(n->item.marker == kMark && "Invalid node pointer");
    listRemove(&n->item);
    freed_.push_back(n);
  }

  /// Return the least recently used one.
  T *leastRecent() {
    assert(!empty() && "LRU is empty");
    Node *n = (Node *)head_.prev;
    assert(n->item.marker == kMark && "Invalid node pointer");
    return &n->value;
  }

 private:
  void listInsert(Item *after, Item *elem) {
    assert(!elem->prev && !elem->next && "Item already in list");
    elem->next = after->next;
    elem->prev = after;
    after->next = elem;
    elem->next->prev = elem;
  }
  void listRemove(Item *elem) {
    assert(elem->prev && elem->next && "Item already removed");
    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;
#ifndef NDEBUG
    elem->prev = elem->next = nullptr;
#endif
  }
};
