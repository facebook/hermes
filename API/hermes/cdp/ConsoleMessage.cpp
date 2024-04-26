/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ConsoleMessage.h"

namespace facebook {
namespace hermes {
namespace cdp {

ConsoleMessageStorage::ConsoleMessageStorage(size_t maxCachedMessages)
    : maxCachedMessages_(maxCachedMessages) {}

void ConsoleMessageStorage::addMessage(ConsoleMessage message) {
  assert(consoleMessageCache_.size() <= maxCachedMessages_);

  // There will be situations when we run out of storage due to limited cache
  // space. This message cache is used to collect console messages before the
  // first devtool connection. The cache is also used if the devtool re-opens
  // and needs restore the previous context as much as possible. This is why the
  // cache will discard earlier messages if max storage is reached. Keeping the
  // most recent messages makes sense when restoring the context.
  if (consoleMessageCache_.size() == maxCachedMessages_) {
    // Increment a counter so we can inform users how many messages were
    // discarded.
    numConsoleMessagesDiscardedFromCache_++;
    consoleMessageCache_.pop_front();
  }

  consoleMessageCache_.push_back(std::move(message));
}

void ConsoleMessageStorage::clear() {
  numConsoleMessagesDiscardedFromCache_ = 0;
  consoleMessageCache_.clear();
}

const std::deque<ConsoleMessage> &ConsoleMessageStorage::messages() const {
  return consoleMessageCache_;
}

size_t ConsoleMessageStorage::discarded() const {
  return numConsoleMessagesDiscardedFromCache_;
}

std::optional<double> ConsoleMessageStorage::oldestTimestamp() const {
  if (consoleMessageCache_.size() == 0) {
    return std::nullopt;
  }
  return consoleMessageCache_.front().timestamp;
}

} // namespace cdp
} // namespace hermes
} // namespace facebook
