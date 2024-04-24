/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SyncConnection.h"

#include <functional>
#include <stdexcept>

#include <glog/logging.h>

#include <hermes/Parser/JSONParser.h>
#include <hermes/inspector/RuntimeAdapter.h>
#include <hermes/inspector/chrome/JSONValueInterfaces.h>
#include <hermes/inspector/chrome/MessageConverters.h>

#include "TestHelpers.h"

namespace facebook {
namespace hermes {
namespace inspector {
namespace chrome {

using namespace std::placeholders;

void ExecutorRuntimeAdapter::tickleJs() {
  runtime_.tickleJsAsync();
}

SyncConnection::SyncConnection(
    AsyncHermesRuntime &runtime,
    bool waitForDebugger)
    : cdpHandler_(
          std::make_unique<ExecutorRuntimeAdapter>(runtime),
          "testConn",
          waitForDebugger) {
  registerCallback();
}

bool SyncConnection::registerCallback() {
  return cdpHandler_.registerCallback(
      std::bind(&SyncConnection::onReply, this, std::placeholders::_1));
}

void SyncConnection::unregisterCallback() {
  cdpHandler_.unregisterCallback();
}

void SyncConnection::send(const std::string &str) {
  LOG(INFO) << "SyncConnection::send sending " << str;

  cdpHandler_.handle(str);
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
  JSONObject *obj = mustParseStrAsJsonObj(message, factory);
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
