/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_CDP_DOMAINAGENT_H
#define HERMES_CDP_DOMAINAGENT_H

#include <mutex>
#include <string>

#include <hermes/inspector/chrome/MessageTypes.h>
#include <hermes/inspector/chrome/ThreadSafetyAnalysis.h>

namespace facebook {
namespace hermes {
namespace cdp {

namespace m = ::facebook::hermes::inspector_modern::chrome::message;

/// Fixed execution context ID because Hermes doesn't currently support realms
/// or Web Workers.
static constexpr int32_t kHermesExecutionContextId = 1;

/// A wrapper around std::function<void(...)> to make it safe to use from
/// multiple threads. The wrapper implements an invalidate function so that one
/// thread can clean up the underlying std::function in a thread-safe way.
template <typename... Args>
class SynchronizedCallback {
 public:
  SynchronizedCallback(std::function<void(Args...)> func)
      : funcContainer_(std::make_shared<FunctionContainer>(func)) {}

  /// Thread-safe version that calls the underlying std::function. If the
  /// underlying std::function is empty, this function is a no-op.
  void operator()(Args... args) const {
    std::lock_guard<std::mutex> lock(funcContainer_->mutex);
    if (funcContainer_->func) {
      funcContainer_->func(args...);
    }
  }

  /// Reset the underlying std::function so that future invocations of
  /// operator() would just be a no-op.
  void invalidate() {
    std::lock_guard<std::mutex> lock(funcContainer_->mutex);
    funcContainer_->func = std::function<void(Args...)>();
  }

 private:
  struct FunctionContainer {
    FunctionContainer(std::function<void(Args...)> func) : func(func) {}

    std::mutex mutex{};

    /// The actual std::function to be invoked by operator()
    std::function<void(Args...)> func TSA_GUARDED_BY(mutex);
  };
  std::shared_ptr<FunctionContainer> funcContainer_;
};

using SynchronizedOutboundCallback = SynchronizedCallback<const std::string &>;

class DomainAgent {
 protected:
  DomainAgent(SynchronizedOutboundCallback messageCallback)
      : messageCallback_(messageCallback) {}

  /// Sends the provided string back to the debug client
  void sendToClient(const std::string &str) {
    messageCallback_(str);
  }

  /// Sends the provided \p Response back to the debug client
  void sendResponseToClient(const m::Response &resp) {
    sendToClient(resp.toJsonStr());
  }

  /// Sends the provided \p Notification back to the debug client
  void sendNotificationToClient(const m::Notification &note) {
    sendToClient(note.toJsonStr());
  }

  /// Callback function to send CDP response back to the debug client
  SynchronizedOutboundCallback messageCallback_;
};

} // namespace cdp
} // namespace hermes
} // namespace facebook

#endif // HERMES_CDP_DOMAINAGENT_H
