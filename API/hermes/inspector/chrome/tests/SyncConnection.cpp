/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SyncConnection.h"

#include <hermes/inspector/chrome/JSONValueInterfaces.h>
#include <functional>
#include <stdexcept>
#include "hermes/include/hermes/Parser/JSONParser.h"

#include <folly/json.h>
#include <glog/logging.h>
#include <hermes/inspector/RuntimeAdapter.h>
#include <hermes/inspector/chrome/MessageConverters.h>
#include <jsinspector/InspectorInterfaces.h>

namespace facebook {
namespace hermes {
namespace inspector {
namespace chrome {

using namespace std::placeholders;
using ::facebook::react::IRemoteConnection;

void ExecutorRuntimeAdapter::tickleJs() {
  runtime_.tickleJsAsync();
}

class SyncConnection::RemoteConnnection : public IRemoteConnection {
 public:
  RemoteConnnection(SyncConnection &conn) : conn_(conn) {}

  void onMessage(std::string message) override {
    conn_.onReply(message);
  }

  void onDisconnect() override {}

 private:
  SyncConnection &conn_;
};

SyncConnection::SyncConnection(
    AsyncHermesRuntime &runtime,
    bool waitForDebugger)
    : connection_(
          std::make_unique<ExecutorRuntimeAdapter>(runtime),
          "testConn",
          waitForDebugger) {
  connection_.connect(std::make_unique<RemoteConnnection>(*this));
}

void SyncConnection::send(const std::string &str) {
  LOG(INFO) << "SyncConnection::send sending " << str;

  connection_.sendMessage(str);
}

void SyncConnection::waitForResponse(
    std::function<void(const std::string &)> handler,
    std::chrono::milliseconds timeout) {
  std::string reply;

  {
    std::unique_lock<std::mutex> lock(mutex_);

    bool success = hasReply_.wait_for(
        lock, timeout, [this]() -> bool { return !replies_.empty(); });

    if (!success) {
      throw std::runtime_error("timed out waiting for reply");
    }

    reply = std::move(replies_.front());
    replies_.pop();
  }

  handler(reply);
}

void SyncConnection::waitForNotification(
    std::function<void(const std::string &)> handler,
    std::chrono::milliseconds timeout) {
  std::string notification;

  {
    std::unique_lock<std::mutex> lock(mutex_);

    bool success = hasNotification_.wait_for(
        lock, timeout, [this]() -> bool { return !notifications_.empty(); });

    if (!success) {
      throw std::runtime_error("timed out waiting for notification");
    }

    notification = std::move(notifications_.front());
    notifications_.pop();
  }

  handler(notification);
}

void SyncConnection::onReply(const std::string &message) {
  JSLexer::Allocator jsonAlloc;
  JSONFactory factory(jsonAlloc);
  JSONObject *obj = parseStrAsJsonObj(message, factory);
  LOG(INFO) << "SyncConnection::onReply got message: " << jsonValToStr(obj);

  std::lock_guard<std::mutex> lock(mutex_);
  if (obj->count("id")) {
    replies_.push(message);
    hasReply_.notify_one();
  } else {
    notifications_.push(message);
    hasNotification_.notify_one();
  }
}

} // namespace chrome
} // namespace inspector
} // namespace hermes
} // namespace facebook
