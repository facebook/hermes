/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// using include guards instead of #pragma once due to compile issues
// with MSVC and BUCK
#ifndef HERMES_INSPECTOR_CDPHANDLER_H
#define HERMES_INSPECTOR_CDPHANDLER_H

#include <functional>
#include <memory>
#include <string>

#include <hermes/hermes.h>
#include <hermes/inspector/RuntimeAdapter.h>

namespace facebook {
namespace hermes {
namespace inspector_modern {
namespace chrome {

using CDPMessageCallbackFunction = std::function<void(const std::string &)>;
using OnUnregisterFunction = std::function<void()>;

/// CDPHandler processes CDP messages between the client and the debugger.
/// It performs no networking or connection logic itself.
class INSPECTOR_EXPORT CDPHandler {
 public:
  /// CDPHandler constructor enables the debugger on the provided runtime. This
  /// should generally called before you start running any JS in the runtime.
  CDPHandler(
      std::unique_ptr<RuntimeAdapter> adapter,
      const std::string &title,
      bool waitForDebugger = false);
  ~CDPHandler();

  /// getRuntime returns the underlying runtime being debugged.
  HermesRuntime &getRuntime();

  /// getTitle returns the name of the friendly name of the runtime that's shown
  /// to users in the CDP frontend (e.g. Chrome DevTools).
  std::string getTitle() const;

  /// Provide a callback to receive replies and notifications from the debugger,
  /// and optionally provide a function to be called during
  /// unregisterCallbacks().
  /// \param msgCallback Function to receive replies and notifications from the
  ///     debugger
  /// \param onDisconnect Function that will be invoked upon calling
  ///     unregisterCallbacks
  /// \return true if there wasn't a previously registered callback
  bool registerCallbacks(
      CDPMessageCallbackFunction msgCallback,
      OnUnregisterFunction onUnregister);

  /// Unregister any previously registered callbacks.
  /// \return true if there were previously registered callbacks
  bool unregisterCallbacks();

  /// Process a JSON-encoded Chrome DevTools Protocol request.
  void handle(std::string str);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace chrome
} // namespace inspector_modern
} // namespace hermes
} // namespace facebook

#endif // HERMES_INSPECTOR_CDPHandler_H
