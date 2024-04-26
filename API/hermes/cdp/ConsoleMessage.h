/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_CDP_CDPCONSOLEMESSAGESTORAGE_H
#define HERMES_CDP_CDPCONSOLEMESSAGESTORAGE_H

#include <queue>

#include <jsi/jsi.h>

namespace facebook {
namespace hermes {
namespace cdp {

/// Controls the max number of message to cached in \p consoleMessageCache_. The
/// value here is chosen to match what Chromium uses in their CDP
/// implementation.
static const int kMaxCachedConsoleMessages = 1000;

enum class ConsoleAPIType {
  kLog,
  kDebug,
  kInfo,
  kError,
  kWarning,
  kDir,
  kDirXML,
  kTable,
  kTrace,
  kStartGroup,
  kStartGroupCollapsed,
  kEndGroup,
  kClear,
  kAssert,
  kTimeEnd,
  kCount
};

struct ConsoleMessage {
  double timestamp;
  ConsoleAPIType type;
  std::vector<jsi::Value> args;

  ConsoleMessage(
      double timestamp,
      ConsoleAPIType type,
      std::vector<jsi::Value> args)
      : timestamp(timestamp), type(type), args(std::move(args)) {}
};

class ConsoleMessageStorage {
 public:
  ConsoleMessageStorage(size_t maxCachedMessages = kMaxCachedConsoleMessages);

  void addMessage(ConsoleMessage message);
  void clear();

  const std::deque<ConsoleMessage> &messages() const;
  size_t discarded() const;

 private:
  /// Maximum number of messages to cache.
  size_t maxCachedMessages_;
  /// Counts the number of console messages discarded when
  /// \p consoleMessageCache_ is full.
  size_t numConsoleMessagesDiscardedFromCache_ = 0;
  /// Cache for storing console messages. Earlier messages are discarded when
  /// the cache is full. The choice to use a std::deque is for fast operations
  /// at the beginning and the end, so that adding to the cache and discarding
  /// from the cache are fast.
  std::deque<ConsoleMessage> consoleMessageCache_{};
};

} // namespace cdp
} // namespace hermes
} // namespace facebook

#endif // HERMES_CDP_CDPCONSOLEMESSAGESTORAGE_H
