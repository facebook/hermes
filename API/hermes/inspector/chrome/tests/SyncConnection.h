/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

#include <hermes/hermes.h>
#include <hermes/inspector/chrome/CDPHandler.h>

#include "AsyncHermesRuntime.h"

namespace facebook {
namespace hermes {
namespace inspector_modern {
namespace chrome {

class ExecutorRuntimeAdapter
    : public facebook::hermes::inspector_modern::RuntimeAdapter {
 public:
  explicit ExecutorRuntimeAdapter(AsyncHermesRuntime &runtime)
      : runtime_(runtime) {}

  virtual ~ExecutorRuntimeAdapter() override = default;

  HermesRuntime &getRuntime() override {
    return *runtime_.runtime();
  }

  void tickleJs() override;

 private:
  AsyncHermesRuntime &runtime_;
};

/**
 * SyncConnection provides a synchronous interface over Connection that is
 * useful in tests.
 */
class SyncConnection {
 public:
  explicit SyncConnection(
      AsyncHermesRuntime &runtime,
      bool waitForDebugger = false);
  ~SyncConnection() = default;

  /// sends a message to the debugger
  void send(const std::string &str);

  /// waits for the next response from the debugger. handler is called with the
  /// response. throws on timeout.
  void waitForResponse(
      std::function<void(const std::string &)> handler,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(2500));

  /// waits for the next notification from the debugger. handler is called with
  /// the notification. throws on timeout.
  void waitForNotification(
      std::function<void(const std::string &)> handler,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(2500));

  bool registerCallbacks();
  bool unregisterCallbacks();

  /// \return True if onUnregister was called in a previous unregisterCallbacks
  ///     call. A registerCallbacks call will reset the status.
  bool onUnregisterWasCalled();

 private:
  /// This function is given to the CDPHandler to receive replies in the form of
  /// CDP messages
  void onReply(const std::string &message);

  /// This function is given to the CDPHandler to be invoked upon
  /// unregisterCallbacks call
  void onUnregister();

  std::shared_ptr<CDPHandler> cdpHandler_;

  bool onUnregisterCalled_ = false;

  std::mutex mutex_;
  std::condition_variable hasReply_;
  std::queue<std::string> replies_;
  std::condition_variable hasNotification_;
  std::queue<std::string> notifications_;
};

} // namespace chrome
} // namespace inspector_modern
} // namespace hermes
} // namespace facebook
